// The sheets of a workbook as a person changes them: added, copied, renamed, deleted, moved, hidden and shown, and given a tab color. One step each, with the order of the tabs, the
// names, and what the formulas said kept for Undo. The calculation and the workbook do the work (Calculation::addSheet, rewriteSheetReferences; Workbook::setSheetName, ...), with the
// worker held back; this says what is allowed and what the step is.
#include "core/editsession.h"

#include <QRegularExpression>

#include <algorithm>

namespace jp::crunch {

using namespace jp::office::sheet;

namespace {

bool live(const SheetEntry &e) { return !e.deleted; }

int shownCount(const WorkbookInfo &info)
{
    int n = 0;
    for (const SheetEntry &e : info.sheets)
        if (live(e) && e.state == SheetState::Visible && e.kind == SheetKind::Worksheet) ++n;
    return n;
}

} // namespace

QVector<int> EditSession::sheetOrder() const { return m_book && m_book->book ? m_book->book->info().order : QVector<int>(); }

bool EditSession::validSheetName(const QString &name, int except, QString *why) const
{
    auto refuse = [&](const QString &text) {
        if (why) *why = text;
        return false;
    };
    if (name.trimmed().isEmpty()) return refuse(tr("A sheet's name can't be empty."));
    if (name.size() > 31) return refuse(tr("A sheet's name can have up to 31 characters."));
    static const QString bad = QStringLiteral(":\\/?*[]");
    for (const QChar c : name)
        if (bad.contains(c)) return refuse(tr("A sheet's name can't have any of these characters: %1").arg(QStringLiteral(": \\ / ? * [ ]")));
    if (name.startsWith(QLatin1Char('\'')) || name.endsWith(QLatin1Char('\''))) return refuse(tr("A sheet's name can't begin or end with an apostrophe."));
    if (name.compare(QLatin1String("History"), Qt::CaseInsensitive) == 0) return refuse(tr("\"History\" is a name the program keeps for itself."));
    if (!m_book || !m_book->book) return true;
    const auto &sheets = m_book->book->info().sheets;
    for (int i = 0; i < sheets.size(); ++i)
        if (i != except && live(sheets[i]) && sheets[i].name.compare(name, Qt::CaseInsensitive) == 0) return refuse(tr("There is a sheet named \"%1\" already. Give this one another name.").arg(sheets[i].name));
    return true;
}

QString EditSession::newSheetName() const
{
    int live_ = 0;
    const auto &sheets = m_book->book->info().sheets;
    for (const SheetEntry &e : sheets)
        if (live(e)) ++live_;
    for (int n = live_ + 1;; ++n) {
        const QString name = QStringLiteral("Sheet%1").arg(n);
        if (validSheetName(name, -1)) return name;
    }
}

QString EditSession::copyName(int sheet) const
{
    const auto &sheets = m_book->book->info().sheets;
    QString base = sheet >= 0 && sheet < sheets.size() ? sheets[sheet].name : QStringLiteral("Sheet");
    // ("Data (2)" copied is "Data (3)", not "Data (2) (2)")
    static const QRegularExpression suffix(QStringLiteral("^(.*) \\((\\d+)\\)$"));
    int n = 2;
    const auto m = suffix.match(base);
    if (m.hasMatch()) {
        base = m.captured(1);
        n = m.captured(2).toInt() + 1;
    }
    for (;; ++n) {
        const QString tail = QStringLiteral(" (%1)").arg(n);
        const QString name = base.left(31 - tail.size()) + tail;
        if (validSheetName(name, -1)) return name;
    }
}

bool EditSession::sheetHasCells(int sheet) const
{
    return m_book && m_book->calc && sheet >= 0 && sheet < m_book->calc->sheetCount() && m_book->calc->sheet(sheet).cells.cellCount() > 0;
}

// Whether a step that names, adds, or moves sheets is refused for what the workbook has: a formula over a block of sheets (Sheet1:Sheet3!A1) means what the order of the sheets makes it mean.
bool EditSession::sheetStepsRefused(QString *why)
{
    std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
    if (!hold) {
        if (why) *why = tr("The workbook is busy. Try again in a moment.");
        return true;
    }
    if (hold->calculation()->hasThreeDReferences()) {
        if (why) *why = tr("This workbook has a formula that adds up a block of sheets (such as Sheet1:Sheet3). JeffCrunch does not yet add, move, rename, or delete sheets in a workbook that has one.");
        return true;
    }
    return false;
}

EditSession::Outcome EditSession::pushSheetEdit(SheetEdit edit, const QString &text)
{
    Outcome out;
    Step step;
    step.sheets.push_back(std::move(edit));
    pushStep(std::move(step), text);
    return out;
}

namespace {

QVector<int> orderWith(QVector<int> order, int sheet, int before)
{
    order.removeAll(sheet);
    const int at = before >= 0 ? order.indexOf(before) : -1;
    if (at >= 0) order.insert(at, sheet);
    else order.append(sheet);
    return order;
}

} // namespace

namespace {

// The most sheets the program reads in a workbook (kMaxSheets): it does not write a file it would not read.
bool sheetCapReached(const jp::office::sheet::Workbook &book, QString *why)
{
    int live = 0;
    for (const SheetEntry &e : book.info().sheets)
        if (!e.deleted) ++live;
    if (live < jp::office::sheet::kMaxSheets) return false;
    *why = EditSession::tr("A workbook can have up to %1 sheets, and this one has that many. Delete a sheet, and try again.").arg(jp::office::sheet::kMaxSheets);
    return true;
}

} // namespace

const EditSession::CopyPoint *EditSession::copyPointOf(int sheet) const
{
    for (const CopyPoint &c : m_copies)
        if (c.sheet == sheet) return &c;
    return nullptr;
}

// What a copy of the sheet does not have, in a sentence for the person (empty if the copy has all of it): the pictures, charts, notes, and the like that the sheet's part is related to, the
// extensions of its file that the program keeps no model of, and the names that belong to the sheet alone.
QString EditSession::notCopiedNote(int sheet) const
{
    const Workbook &book = *m_book->book;
    QStringList left;
    auto add = [&left](const QString &what) {
        if (!left.contains(what)) left << what;
    };
    const QString part = book.info().sheets[sheet].part;
    if (!part.isEmpty()) {
        struct Kind {
            const char *type;
            QString what;
        };
        const Kind kinds[] = {{"/drawing", tr("pictures and charts")}, {"/image", tr("pictures and charts")}, {"/comments", tr("notes")}, {"/vmlDrawing", tr("notes")},
                              {"/legacyDrawing", tr("notes")}, {"/threadedComment", tr("notes")}, {"/pivotTable", tr("PivotTables")}, {"/queryTable", tr("connections to outside data")},
                              {"/ctrlProp", tr("form controls")}, {"/control", tr("form controls")}, {"/oleObject", tr("embedded objects")}, {"/package", tr("embedded objects")},
                              {"/slicer", tr("slicers")}, {"/timeline", tr("slicers")}};
        for (const Kind &k : kinds)
            if (!book.package().relationshipsOfType(part, QString::fromLatin1(k.type)).isEmpty()) add(k.what);
    }
    for (const QString &id : m_book->calc->sheet(sheet).notKept) {
        if (id == QLatin1String("sparklines")) add(tr("sparklines"));
        else if (id == QLatin1String("validationLists")) add(tr("drop-down lists that read other sheets"));
        else if (id == QLatin1String("extendedRules")) add(tr("rules of conditional formatting that read other sheets or use icons of their own"));
        else add(tr("some extra features"));
    }
    for (const DefinedName &n : book.info().definedNames)
        if (n.localSheet == sheet) {
            add(tr("names that belong to the sheet alone (a print area, say)"));
            break;
        }
    if (left.isEmpty()) return QString();
    return tr("The copy does not have the %1: those are not copied yet.").arg(QLocale().createSeparatedList(left));
}

EditSession::Outcome EditSession::insertSheet(int before, const QString &name, int *made)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || sheetStepsRefused(&why) || sheetCapReached(*m_book->book, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    const QString nm = name.isEmpty() ? newSheetName() : name;
    if (!validSheetName(nm, -1, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    SheetEdit e;
    e.kind = SheetEdit::Kind::Insert;
    e.sheet = m_book->calc->sheetCount();
    e.nameAfter = nm;
    e.orderBefore = sheetOrder();
    e.orderAfter = orderWith(e.orderBefore, e.sheet, before);
    if (made) *made = e.sheet;
    return pushSheetEdit(e, tr("Insert Sheet"));
}

EditSession::Outcome EditSession::copySheet(int sheet, int before, const QString &name, int *made)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why) || sheetStepsRefused(&why) || sheetCapReached(*m_book->book, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    const Workbook &book = *m_book->book;
    if (book.info().sheets[sheet].deleted) {
        out.ok = false;
        out.problem = tr("There is no such sheet.");
        return out;
    }
    for (const Table &t : book.tables())
        if (t.sheet == sheet) {
            out.ok = false;
            out.problem = tr("This sheet has a table, and JeffCrunch does not yet make a copy of a sheet that has one.");
            return out;
        }
    const QString nm = name.isEmpty() ? copyName(sheet) : name;
    if (!validSheetName(nm, -1, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    // what a copy leaves behind: said, with the copy made
    out.problem = notCopiedNote(sheet);
    SheetEdit e;
    e.kind = SheetEdit::Kind::Copy;
    e.sheet = m_book->calc->sheetCount();
    e.copyOf = sheet;
    e.nameAfter = nm;
    e.orderBefore = sheetOrder();
    e.orderAfter = orderWith(e.orderBefore, e.sheet, before);
    if (made) *made = e.sheet;
    const QString note = out.problem;
    out = pushSheetEdit(e, tr("Copy Sheet"));
    out.problem = note;
    return out;
}

EditSession::Outcome EditSession::deleteSheet(int sheet)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || sheetStepsRefused(&why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    const WorkbookInfo &info = m_book->book->info();
    if (sheet < 0 || sheet >= info.sheets.size() || info.sheets[sheet].deleted) {
        out.ok = false;
        out.problem = tr("There is no such sheet.");
        return out;
    }
    if (info.sheets[sheet].state == SheetState::Visible && info.sheets[sheet].kind == SheetKind::Worksheet && shownCount(info) <= 1) {
        out.ok = false;
        out.problem = tr("A workbook must have at least one sheet that is shown. Show another sheet, or add one, and then delete this one.");
        return out;
    }
    for (const Table &t : m_book->book->tables())
        if (t.sheet == sheet) {
            out.ok = false;
            out.problem = tr("This sheet has a table, and JeffCrunch does not yet delete a sheet that has one.");
            return out;
        }
    SheetEdit e;
    e.kind = SheetEdit::Kind::Delete;
    e.sheet = sheet;
    e.nameBefore = info.sheets[sheet].name;
    e.orderBefore = info.order;
    e.orderAfter = info.order;
    e.orderAfter.removeAll(sheet);
    return pushSheetEdit(e, tr("Delete Sheet"));
}

EditSession::Outcome EditSession::renameSheet(int sheet, const QString &name)
{
    Outcome out;
    QString why;
    if (!canEdit(&why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    const WorkbookInfo &info = m_book->book->info();
    if (sheet < 0 || sheet >= info.sheets.size() || info.sheets[sheet].deleted) {
        out.ok = false;
        out.problem = tr("There is no such sheet.");
        return out;
    }
    if (name == info.sheets[sheet].name) return out;   // (nothing to change)
    if (!validSheetName(name, sheet, &why) || sheetStepsRefused(&why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    SheetEdit e;
    e.kind = SheetEdit::Kind::Rename;
    e.sheet = sheet;
    e.nameBefore = info.sheets[sheet].name;
    e.nameAfter = name;
    return pushSheetEdit(e, tr("Rename Sheet"));
}

EditSession::Outcome EditSession::moveSheet(int sheet, int before)
{
    Outcome out;
    QString why;
    if (!canEdit(&why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    const WorkbookInfo &info = m_book->book->info();
    if (sheet < 0 || sheet >= info.sheets.size() || info.sheets[sheet].deleted) {
        out.ok = false;
        out.problem = tr("There is no such sheet.");
        return out;
    }
    const QVector<int> after = orderWith(info.order, sheet, before == sheet ? -2 : before);
    if (before == sheet || after == info.order) return out;   // (nothing to change)
    if (sheetStepsRefused(&why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    SheetEdit e;
    e.kind = SheetEdit::Kind::Move;
    e.sheet = sheet;
    e.orderBefore = info.order;
    e.orderAfter = after;
    return pushSheetEdit(e, tr("Move Sheet"));
}

EditSession::Outcome EditSession::setSheetHidden(int sheet, bool hidden)
{
    Outcome out;
    QString why;
    if (!canEdit(&why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    const WorkbookInfo &info = m_book->book->info();
    if (sheet < 0 || sheet >= info.sheets.size() || info.sheets[sheet].deleted) {
        out.ok = false;
        out.problem = tr("There is no such sheet.");
        return out;
    }
    const SheetState now = info.sheets[sheet].state, want = hidden ? SheetState::Hidden : SheetState::Visible;
    if ((now == SheetState::Visible) == !hidden) return out;   // (nothing to change)
    if (hidden && info.sheets[sheet].kind == SheetKind::Worksheet && shownCount(info) <= 1) {
        out.ok = false;
        out.problem = tr("A workbook must have at least one sheet that is shown.");
        return out;
    }
    SheetEdit e;
    e.kind = SheetEdit::Kind::State;
    e.sheet = sheet;
    e.stateBefore = now;
    e.stateAfter = want;
    return pushSheetEdit(e, hidden ? tr("Hide Sheet") : tr("Unhide Sheet"));
}

EditSession::Outcome EditSession::setTabColor(int sheet, const ColorSpec &color)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    const ColorSpec now = m_book->calc->sheet(sheet).tabColor;
    if (now == color) return out;   // (nothing to change)
    SheetEdit e;
    e.kind = SheetEdit::Kind::TabColor;
    e.sheet = sheet;
    e.colorBefore = now;
    e.colorAfter = color;
    return pushSheetEdit(e, tr("Tab Color"));
}

// A sheet that was added is taken away: the values, results, and marks kept by cell for it are let go of, so that a sheet added later at its place starts with none.
void EditSession::forgetSheet(int sheet)
{
    m_sheetRemoved = true;
    if (m_values) m_values->dropSheet(sheet);
    auto of = [sheet](quint64 key) { return int(key >> 40) == sheet; };
    for (auto it = m_touched.begin(); it != m_touched.end();) it = of(*it) ? m_touched.erase(it) : std::next(it);
    for (auto *map : {&m_results, &m_engineAtStart, &m_fileResults})
        for (auto it = map->begin(); it != map->end();) it = of(it->first) ? map->erase(it) : std::next(it);
}

void EditSession::dropRemovedSheets(std::vector<jp::grid::ValueStore::Item> *items) const
{
    if (!m_sheetRemoved || !m_book || !m_book->calc) return;
    const int count = m_book->calc->sheetCount();
    items->erase(std::remove_if(items->begin(), items->end(), [count](const jp::grid::ValueStore::Item &it) { return it.sheet < 0 || it.sheet >= count; }), items->end());
}

// Makes a step's changes of the sheets, or takes them back (with the worker held back).
bool EditSession::applySheetEdits(Step *step, bool forward, Calculation *calc, Workbook &book)
{
    for (SheetEdit &e : step->sheets) {
        switch (e.kind) {
        case SheetEdit::Kind::Insert:
        case SheetEdit::Kind::Copy:
            if (forward) {
                const int made = calc->addSheet(e.nameAfter, e.kind == SheetEdit::Kind::Copy ? e.copyOf : -1);
                const int entry = book.addSheetEntry(e.nameAfter);
                if (made != e.sheet || entry != e.sheet) return false;
                if (e.kind == SheetEdit::Kind::Copy) m_copies.push_back({made, e.copyOf, m_edits.size()});
                if (m_book->geometry.size() < size_t(made) + 1) m_book->geometry.resize(size_t(made) + 1);
                book.setSheetOrder(e.orderAfter);
            } else {
                if (calc->sheetCount() != e.sheet + 1) return false;   // (the sheet added is the last: what was done after it was undone first)
                calc->removeLastSheet();
                book.removeLastSheetEntry();
                if (!m_copies.empty() && m_copies.back().sheet == e.sheet) m_copies.pop_back();
                forgetSheet(e.sheet);
                if (m_book->geometry.size() > size_t(e.sheet)) m_book->geometry.resize(size_t(e.sheet));
                book.setSheetOrder(e.orderBefore);
            }
            break;
        case SheetEdit::Kind::Delete:
            if (forward) {
                calc->setSheetNames(e.sheet, e.nameBefore, QString());
                e.rewrite = Calculation::SheetRewrite();
                if (!calc->rewriteSheetReferences(e.nameBefore, QString(), &e.rewrite)) {
                    calc->setSheetNames(e.sheet, QString(), e.nameBefore);
                    return false;
                }
                book.setSheetName(e.sheet, QString());
                book.setSheetDeleted(e.sheet, true);
                book.setSheetOrder(e.orderAfter);
                for (const Calculation::EditRecord::Place &p : e.rewrite.rewritten)
                    if (!p.dataTable) m_touched.insert(keyOf(p.sheet, p.row, p.col));
                SheetOp op;
                op.kind = SheetOp::Kind::Delete;
                op.name = e.nameBefore;
                m_sheetOps.push_back(op);
            } else {
                book.setSheetName(e.sheet, e.nameBefore);
                book.setSheetDeleted(e.sheet, false);
                calc->setSheetNames(e.sheet, QString(), e.nameBefore);
                calc->undoSheetRewrite(e.rewrite);
                book.setSheetOrder(e.orderBefore);
                if (!m_sheetOps.empty() && m_sheetOps.back().kind == SheetOp::Kind::Delete && m_sheetOps.back().name == e.nameBefore) m_sheetOps.pop_back();
            }
            break;
        case SheetEdit::Kind::Rename:
            if (forward) {
                calc->setSheetNames(e.sheet, e.nameBefore, e.nameAfter);
                e.rewrite = Calculation::SheetRewrite();
                if (!calc->rewriteSheetReferences(e.nameBefore, e.nameAfter, &e.rewrite)) {
                    calc->setSheetNames(e.sheet, e.nameAfter, e.nameBefore);
                    return false;
                }
                book.setSheetName(e.sheet, e.nameAfter);
                for (const Calculation::EditRecord::Place &p : e.rewrite.rewritten)
                    if (!p.dataTable) m_touched.insert(keyOf(p.sheet, p.row, p.col));
                SheetOp op;
                op.kind = SheetOp::Kind::Rename;
                op.name = e.nameBefore;
                op.newName = e.nameAfter;
                m_sheetOps.push_back(op);
            } else {
                calc->setSheetNames(e.sheet, e.nameAfter, e.nameBefore);
                book.setSheetName(e.sheet, e.nameBefore);
                calc->undoSheetRewrite(e.rewrite);
                if (!m_sheetOps.empty() && m_sheetOps.back().kind == SheetOp::Kind::Rename && m_sheetOps.back().name == e.nameBefore && m_sheetOps.back().newName == e.nameAfter) m_sheetOps.pop_back();
            }
            break;
        case SheetEdit::Kind::Move: book.setSheetOrder(forward ? e.orderAfter : e.orderBefore); break;
        case SheetEdit::Kind::State:
            book.setSheetState(e.sheet, forward ? e.stateAfter : e.stateBefore);
            m_statesChanged.insert(e.sheet);
            break;
        case SheetEdit::Kind::TabColor:
            calc->setTabColor(e.sheet, forward ? e.colorAfter : e.colorBefore);
            m_tabColorsChanged.insert(e.sheet);
            break;
        }
        m_sheetsListed = true;
    }
    return true;
}

} // namespace jp::crunch
