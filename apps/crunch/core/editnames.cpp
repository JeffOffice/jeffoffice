// The defined names of an open workbook, and how it calculates, changed by a person: each change is one step of Undo (see EditSession::defineName and core/nametable.h).
#include "core/editsession.h"

#include "calc/eval.h"
#include "core/namesfromcells.h"
#include "sheet/namerefs.h"
#include "sheet/refshift.h"

#include <QCoreApplication>

#include <algorithm>

namespace jp::crunch {

using namespace jp::office::sheet;
using jp::calc::Value;

namespace {

// What a name stands for as a file writes it, from what a person typed ("=" in front is not part of it).
QString fileText(const QString &typed, const QLocale &locale)
{
    QString t = typed.trimmed();
    if (t.startsWith(QLatin1Char('='))) t.remove(0, 1);
    return formulaFromLocale(t.trimmed(), locale);
}

}

bool EditSession::nameListed(int id) const
{
    if (id < 0 || id >= m_nameTable.slots()) return false;
    const NameEntry &e = m_nameTable.at(id);
    if (!e.alive) return false;
    if (e.scope < 0) return true;
    const WorkbookInfo &info = m_book->book->info();
    return e.scope < info.sheets.size() && !info.sheets[e.scope].deleted;
}

QString EditSession::nameTextOf(int id) const
{
    if (id < 0 || id >= m_nameTable.slots() || !m_book || !m_book->calc) return QString();
    const NameEntry &e = m_nameTable.at(id);
    QString text;
    if (!m_book->calc->nameText(e.scope, e.name, &text)) return QString();
    return QLatin1Char('=') + formulaToLocale(text, m_locale);
}

QString EditSession::nameValueOf(int id) const
{
    if (id < 0 || id >= m_nameTable.slots() || !m_book || !m_book->calc) return QString();
    const NameEntry &e = m_nameTable.at(id);
    QString text;
    if (!m_book->calc->nameText(e.scope, e.name, &text)) return QString();
    // a block of cells is shown as a block, not as its first cell
    CellRange range;
    const int bang = int(text.lastIndexOf(QLatin1Char('!')));
    if (!text.contains(QLatin1Char('(')) && parseRangeRef(QStringView(text).mid(bang + 1), &range) && range.cellCount() > 1) return QStringLiteral("{…}");
    jp::calc::ParseOptions po;
    po.definedName = true;
    const jp::calc::ParseResult parsed = jp::calc::parseFormula(text, po);
    if (!parsed.ok()) return QString();
    jp::calc::Evaluator ev;
    ev.setWorkLimit(20000);   // (a name is shown in a list: one that takes much working out is not worked out there)
    jp::calc::EvalContext ctx;
    ctx.source = m_book->calc.get();
    ctx.sheet = std::max(0, e.scope);
    ctx.dateSystem = m_book->book->info().dateSystem;
    const Value v = ev.evaluate(*parsed.program, ctx);
    if (ev.status() != jp::calc::EvalStatus::Ok) return QStringLiteral("…");
    if (v.isArray()) return QStringLiteral("{…}");
    if (v.isNumber()) return m_locale.toString(v.number(), 'g', 10);
    if (v.isText()) return v.text();
    if (v.isBool()) return v.boolean() ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
    if (v.isError()) return v.debug().mid(2);
    return QString();
}

QString EditSession::nameProblem(const NameDraft &draft, int id) const
{
    const QString name = draft.name.trimmed();
    if (const QString why = jp::office::sheet::nameProblem(name); !why.isEmpty()) return why;
    if (draft.scope >= 0) {
        QString why;
        if (!canEditSheet(draft.scope, &why)) return why;
    }
    const int same = m_nameTable.find(draft.scope, name);
    if (same >= 0 && same != id) {
        if (draft.scope < 0) return QCoreApplication::translate("jp::crunch::EditSession", "There is already a name like that in this workbook. Choose another, or change that one.");
        return QCoreApplication::translate("jp::crunch::EditSession", "There is already a name like that on this sheet. Choose another, or change that one.");
    }
    if (tableNamed(name)) return QCoreApplication::translate("jp::crunch::EditSession", "A table in this workbook is called that. Choose another name.");
    if (id < 0 && m_nameTable.aliveCount() >= int(jp::office::sheet::kMaxDefinedNames)) return QCoreApplication::translate("jp::crunch::EditSession", "This workbook has as many names as it can have.");
    if (draft.comment.size() > 255) return QCoreApplication::translate("jp::crunch::EditSession", "A comment can have at most 255 characters.");
    const QString text = fileText(draft.text, m_locale);
    if (text.isEmpty())
        return QCoreApplication::translate("jp::crunch::EditSession", "Say what the name stands for: a cell or block of cells such as Sheet1!$A$1:$A$9, a number, or a formula.");
    jp::calc::ParseOptions po;
    po.definedName = true;
    const jp::calc::ParseResult parsed = jp::calc::parseFormula(text, po);
    if (!parsed.ok()) return QCoreApplication::translate("jp::crunch::EditSession", "What the name stands for is not a formula JeffCrunch can read: %1").arg(parsed.error);
    return QString();
}

std::vector<int> EditSession::shadowingSheets(const QString &name) const
{
    std::vector<int> out;
    for (int i = 0; i < m_nameTable.slots(); ++i) {
        const NameEntry &e = m_nameTable.at(i);
        if (e.alive && e.scope >= 0 && e.name.compare(name, Qt::CaseInsensitive) == 0) out.push_back(e.scope);
    }
    return out;
}

EditSession::Outcome EditSession::defineName(const NameDraft &draft, int *made)
{
    Outcome out;
    QString why;
    if (!canEdit(&why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    if (const QString problem = nameProblem(draft, -1); !problem.isEmpty()) {
        out.ok = false;
        out.problem = problem;
        return out;
    }
    NameEdit e;
    e.id = m_nameTable.slots();   // (the place it will have: Step's redo makes it)
    e.after.name = draft.name.trimmed();
    e.after.scope = draft.scope;
    e.after.comment = draft.comment;
    e.after.hidden = draft.hidden;
    e.before = e.after;
    e.before.alive = false;
    e.textAfter = fileText(draft.text, m_locale);
    Step step;
    step.names.push_back(std::move(e));
    if (made) *made = m_nameTable.slots();
    pushStep(std::move(step), tr("New Name"));
    return out;
}

EditSession::Outcome EditSession::changeName(int id, const NameDraft &draft)
{
    Outcome out;
    QString why;
    if (!canEdit(&why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    if (!nameListed(id)) {
        out.ok = false;
        out.problem = tr("There is no such name.");
        return out;
    }
    const NameEntry &was = m_nameTable.at(id);
    NameDraft d = draft;
    d.scope = was.scope;   // (a name stays with the sheet it belongs to: to move it, make it again)
    if (const QString problem = nameProblem(d, id); !problem.isEmpty()) {
        out.ok = false;
        out.problem = problem;
        return out;
    }
    // a name given the text of one that hides it, or that it would hide, is read by some formulas as the other: refused
    if (was.name.compare(d.name.trimmed(), Qt::CaseInsensitive) != 0) {
        if (was.scope < 0 && !shadowingSheets(d.name.trimmed()).empty()) {
            out.ok = false;
            out.problem = tr("A sheet has a name of its own like that. Renaming would change which name some formulas on that sheet read. Choose another name.");
            return out;
        }
        if (was.scope >= 0 && m_nameTable.find(-1, d.name.trimmed()) >= 0) {
            out.ok = false;
            out.problem = tr("The workbook has a name like that. Renaming would change which name formulas on this sheet read. Choose another name.");
            return out;
        }
    }
    NameEdit e;
    e.id = id;
    e.before = was;
    e.after = was;
    e.after.name = d.name.trimmed();
    e.after.comment = d.comment;
    e.after.hidden = d.hidden;
    m_book->calc->nameText(was.scope, was.name, &e.textBefore);
    e.textAfter = fileText(d.text, m_locale);
    if (e.before == e.after && e.textBefore == e.textAfter) return out;   // (nothing to change)
    Step step;
    step.names.push_back(std::move(e));
    pushStep(std::move(step), tr("Change Name"));
    return out;
}

EditSession::Outcome EditSession::removeName(int id)
{
    Outcome out;
    QString why;
    if (!canEdit(&why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    if (!nameListed(id)) {
        out.ok = false;
        out.problem = tr("There is no such name.");
        return out;
    }
    NameEdit e;
    e.id = id;
    e.before = m_nameTable.at(id);
    e.after = e.before;
    e.after.alive = false;
    m_book->calc->nameText(e.before.scope, e.before.name, &e.textBefore);
    e.textAfter = e.textBefore;
    Step step;
    step.names.push_back(std::move(e));
    pushStep(std::move(step), tr("Delete Name"));
    return out;
}

EditSession::Outcome EditSession::defineNamesFromLabels(int sheet, const CellRange &block, int edges, int *made, int *skipped)
{
    Outcome out;
    if (made) *made = 0;
    if (skipped) *skipped = 0;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    if (block.cellCount() > kMaxCellsPerEdit) {
        out.ok = false;
        out.problem = tr("That is too many cells to make names from. Select fewer.");
        return out;
    }
    const Calculation *calc = m_book->calc.get();
    int left = 0;
    const std::vector<LabelName> found = namesFromLabels(block, edges, [&](quint32 r, quint32 c) {
        const Value v = calc->cell(sheet, r, c);
        return v.isText() ? v.text() : v.isNumber() ? QString::number(v.number(), 'g', 15) : QString();
    }, &left);
    const QString prefix = sheetNamePrefix(m_book->book->info().sheets[sheet].name);
    Step step;
    int slot = m_nameTable.slots();
    for (const LabelName &n : found) {
        if (m_nameTable.find(-1, n.name) >= 0 || tableNamed(n.name) || int(m_nameTable.aliveCount()) + int(step.names.size()) >= int(jp::office::sheet::kMaxDefinedNames)) {
            ++left;
            continue;
        }
        NameEdit e;
        e.id = slot++;
        e.after.name = n.name;
        e.before = e.after;
        e.before.alive = false;
        e.textAfter = prefix + QLatin1Char('$') + columnName(n.cells.col1) + QLatin1Char('$') + QString::number(n.cells.row1 + 1);
        if (n.cells.cellCount() > 1) e.textAfter += QStringLiteral(":$") + columnName(n.cells.col2) + QLatin1Char('$') + QString::number(n.cells.row2 + 1);
        step.names.push_back(std::move(e));
    }
    if (skipped) *skipped = left;
    if (step.names.empty()) return out;
    if (made) *made = int(step.names.size());
    pushStep(std::move(step), tr("Name From Cells"));
    return out;
}

// Makes a step's names, or takes them back (with the worker held back).
bool EditSession::applyNameEdits(Step *step, bool forward, Calculation *calc)
{
    for (NameEdit &e : step->names) {
        const NameEntry &from = forward ? e.before : e.after, &to = forward ? e.after : e.before;
        const QString &textTo = forward ? e.textAfter : e.textBefore;
        if (e.id == m_nameTable.slots() && !e.before.alive && forward) {
            m_nameTable.add(e.before);   // (the place a name made by this step is to have)
        }
        if (e.id >= m_nameTable.slots()) return false;
        if (from.alive && !to.alive) {
            calc->setNameText(from.scope, from.name, QString());
        } else if (!from.alive && to.alive) {
            calc->setNameText(to.scope, to.name, textTo);
        } else if (from.alive && to.alive) {
            if (forward) {
                if (e.renamed()) {
                    e.rewrite = Calculation::SheetRewrite();
                    NameRename r{e.before.name, e.after.name, e.before.scope, e.before.scope < 0 ? shadowingSheets(e.before.name) : std::vector<int>()};
                    calc->renameName(e.before.scope, e.before.name, e.after.name, &e.rewrite);
                    for (const Calculation::EditRecord::Place &p : e.rewrite.rewritten)
                        if (!p.dataTable) markTouched(p.sheet, p.row, p.col);
                    m_nameRenames.push_back(std::move(r));
                }
                if (e.textBefore != e.textAfter) calc->setNameText(to.scope, to.name, e.textAfter);
            } else {
                if (e.textBefore != e.textAfter) calc->setNameText(from.scope, from.name, e.textBefore);
                if (e.renamed()) {
                    NameRename inverse{e.after.name, e.before.name, e.after.scope, e.after.scope < 0 ? shadowingSheets(e.after.name) : std::vector<int>()};
                    calc->undoRenameName(e.before.scope, e.before.name, e.after.name, e.rewrite);
                    for (const Calculation::EditRecord::Place &p : e.rewrite.rewritten)
                        if (!p.dataTable) markTouched(p.sheet, p.row, p.col);
                    // (what the file is to follow: the rename taken off the list if it is the last, else a rename the other way)
                    NameRename was{e.before.name, e.after.name, e.before.scope, std::vector<int>()};
                    if (!m_nameRenames.empty() && m_nameRenames.back().oldName == was.oldName && m_nameRenames.back().newName == was.newName && m_nameRenames.back().scope == was.scope) m_nameRenames.pop_back();
                    else m_nameRenames.push_back(std::move(inverse));
                }
            }
        }
        m_nameTable.put(e.id, to);
    }
    return true;
}

// What the file's list of names is to be changed by: those made, changed, and taken away since it was read or saved.
void EditSession::nameChangesForSave(std::vector<NameChange> *out) const
{
    const Calculation *calc = m_book->calc.get();
    const WorkbookInfo &info = m_book->book->info();
    auto usable = [&](int scope) { return scope < 0 || (scope < info.sheets.size() && !info.sheets[scope].deleted); };
    const std::vector<NameBase> &base = m_nameTable.base();
    std::vector<int> entryOf(base.size(), -1);
    for (int i = 0; i < m_nameTable.slots(); ++i) {
        const NameEntry &e = m_nameTable.at(i);
        if (e.alive && e.base >= 0 && size_t(e.base) < base.size()) entryOf[size_t(e.base)] = i;
    }
    for (size_t b = 0; b < base.size(); ++b) {
        const NameBase &was = base[b];
        if (!usable(was.scope)) continue;   // (it went with its sheet)
        NameChange c;
        c.hadOld = true;
        c.oldName = was.name;
        c.oldPlace = was.scope;
        if (entryOf[b] < 0) {
            c.remove = true;
            out->push_back(c);
            continue;
        }
        const NameEntry &e = m_nameTable.at(entryOf[b]);
        QString text;
        calc->nameText(e.scope, e.name, &text);
        if (e.name == was.name && e.comment == was.comment && e.hidden == was.hidden && text == was.text) continue;
        c.name = e.name;
        c.place = e.scope;
        c.text = text;
        c.comment = e.comment;
        c.hidden = e.hidden;
        out->push_back(c);
    }
    for (int i = 0; i < m_nameTable.slots(); ++i) {
        const NameEntry &e = m_nameTable.at(i);
        if (!e.alive || e.base >= 0 || !usable(e.scope)) continue;
        NameChange c;
        c.name = e.name;
        c.place = e.scope;
        calc->nameText(e.scope, e.name, &c.text);
        c.comment = e.comment;
        c.hidden = e.hidden;
        out->push_back(c);
    }
}

// ---- how the workbook calculates

EditSession::Outcome EditSession::setCalcSettings(const CalcSettings &settings)
{
    Outcome out;
    QString why;
    if (!canEdit(&why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    CalcSettings now = settings;
    now.count = std::clamp(now.count, 1, jp::office::sheet::Calculation::kMaxIterations);
    if (!(now.delta >= 0) || now.delta > 1e9) now.delta = m_calcNow.delta;
    if (now == m_calcNow) return out;   // (nothing to change)
    CalcEdit e;
    e.before = m_calcNow;
    e.after = now;
    Step step;
    step.calcs.push_back(e);
    pushStep(std::move(step), tr("How Formulas Are Worked Out"));
    return out;
}

void EditSession::applyCalcEdits(Step *step, bool forward, Calculation *calc)
{
    for (const CalcEdit &e : step->calcs) {
        const CalcSettings &to = forward ? e.after : e.before;
        calc->setIteration(to.iterate, to.count, to.delta);
        m_calcNow = to;
    }
}

} // namespace jp::crunch
