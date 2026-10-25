#include "core/editsession.h"

#include "sheet/xlsxbase.h"

#include "grid/text.h"
#include "sheet/cellref.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QThread>
#include <QUndoCommand>

#include <algorithm>
#include <atomic>

namespace jp::crunch {

using namespace jp::office;
using namespace jp::office::sheet;
using jp::calc::Value;
using jp::grid::ValueStore;

namespace {

constexpr int kHoldWaitMs = 1000;   // how long an entry typed under a rule waits for the worker to be held, at most

bool sameMeaning(const Calculation::CellContent &a, const Calculation::CellContent &b)
{
    return a.exists == b.exists && a.isFormula == b.isFormula && a.formula == b.formula && a.value == b.value && a.style == b.style;
}

bool sameValue(const Value &a, const Value &b)
{
    if (a.isNumber() && b.isNumber()) return jp::calc::sameTo15Digits(a.number(), b.number());
    if ((a.isEmpty() && b.isNumber() && b.number() == 0) || (b.isEmpty() && a.isNumber() && a.number() == 0)) return true;
    return a == b;
}

SavedValue savedValue(const Value &v, qint32 sharedString)
{
    SavedValue s;
    switch (v.kind()) {
    case Value::Kind::Number:
        s.kind = SavedValue::Kind::Number;
        s.number = v.number();
        break;
    case Value::Kind::Text:
        s.kind = SavedValue::Kind::Text;
        s.text = v.text();
        s.sharedString = sharedString;
        break;
    case Value::Kind::Bool:
        s.kind = SavedValue::Kind::Bool;
        s.number = v.boolean() ? 1 : 0;
        break;
    case Value::Kind::Error:
        s.kind = SavedValue::Kind::Error;
        s.number = double(int(v.errorKind()));
        break;
    default: break;
    }
    return s;
}

} // namespace

// One step of the undo stack: cells put in the state they come to, and back.
qint64 contentBytes(const Calculation::CellContent &c)
{
    qint64 n = qint64(sizeof(c)) + qint64(c.formula.size()) * 2;
    if (c.value.isText()) n += qint64(c.value.text().size()) * 2;
    if (c.hasStored && c.stored.isText()) n += qint64(c.stored.text().size()) * 2;
    return n;
}

class ChangeCells : public QUndoCommand {
public:
    ChangeCells(EditSession *session, Step step, const QString &text, const std::shared_ptr<qint64> &total) : QUndoCommand(text), m_session(session), m_step(std::move(step)), m_total(total)
    {
        recount();
    }
    ~ChangeCells() override { *m_total -= m_bytes; }
    void redo() override
    {
        if (m_forgotten) return;
        if (!m_session->apply(&m_step, true)) {
            m_session->giveUpUndo(text(), true);
            return;
        }
        recount();
    }
    void undo() override
    {
        if (m_forgotten) return;
        if (!m_session->apply(&m_step, false)) {
            m_session->giveUpUndo(text(), false);
            return;
        }
        recount();
    }
    bool forgotten() const { return m_forgotten; }
    Step *step() { return &m_step; }
    // Whether the step inserted or deleted rows, columns, or cells.
    bool moves() const { return !m_forgotten && m_step.structure != nullptr; }
    // Whether the step added, copied, renamed, deleted, or moved a sheet (what the saved file's parts name was changed).
    bool sheetStructure() const
    {
        if (m_forgotten) return false;
        for (const SheetEdit &e : m_step.sheets)
            if (e.structural()) return true;
        return false;
    }
    // Whether the step put an object over the cells (one that, taken away and the file saved, could not be put back).
    bool adds() const
    {
        if (m_forgotten) return false;
        for (const ObjectEdit &e : m_step.objects)
            if (e.adds()) return true;
        return false;
    }
    // Whether the step deleted rows, columns, or cells.
    bool deletes() const
    {
        if (!m_forgotten)
            for (const ObjectEdit &e : m_step.objects)
                if (e.deletes()) return true;   // (an object taken away is not in the saved file: an Undo of it could not give it back to the file)
        if (m_forgotten || !m_step.structure) return false;
        using K = jp::office::sheet::StructEdit::Kind;
        const K k = m_step.structure->edit.kind;
        return k == K::DeleteRows || k == K::DeleteColumns || k == K::DeleteCellsUp || k == K::DeleteCellsLeft;
    }
    // The bytes the step holds (what a move of cells keeps is known when it is done).
    void recount()
    {
        qint64 n = 0;
        for (const CellChange &c : m_step.cells) n += qint64(sizeof(c)) + contentBytes(c.before) + contentBytes(c.after) + qint64(c.formatCode.size()) * 2;
        n += qint64(m_step.styles.size()) * qint64(sizeof(StyleCell));
        for (const TableRename &r : m_step.renames) n += qint64(sizeof(r)) + qint64(r.before.size() + r.after.size()) * 2;
        for (const MergeEdit &m : m_step.merges) n += qint64(sizeof(m)) + qint64(m.before.size() + m.after.size()) * qint64(sizeof(jp::office::sheet::CellRange));
        for (const SheetEdit &e : m_step.sheets) n += qint64(sizeof(e)) + e.rewrite.bytes() + qint64(e.orderBefore.size() + e.orderAfter.size()) * qint64(sizeof(int));
        for (const SizeEdit &e : m_step.sizes)
            n += qint64(sizeof(e)) + qint64(e.columnsBefore.size() + e.columnsAfter.size()) * qint64(sizeof(jp::office::sheet::ColumnSpan)) + qint64(e.rowsBefore.size() + e.rowsAfter.size()) * qint64(sizeof(jp::office::sheet::RowSpec));
        if (m_step.structure) n += m_step.structure->bytes();
        for (const ObjectEdit &e : m_step.objects) n += qint64(sizeof(e)) + approxBytes(e.before) + (e.after.chart == e.before.chart ? 0 : approxBytes(e.after));
        for (const PageEdit &e : m_step.pages) n += qint64(sizeof(e)) + qint64(e.before.rowBreaks.size() + e.before.colBreaks.size() + e.after.rowBreaks.size() + e.after.colBreaks.size()) * 16 + qint64(e.before.printArea.size() + e.after.printArea.size()) * qint64(sizeof(jp::office::sheet::CellRange)) + qint64(e.before.headerFooter.oddHeader.size() + e.after.headerFooter.oddHeader.size()) * 2;
        *m_total += n - m_bytes;
        m_bytes = n;
    }
    // Lets go of what the step holds: it can't be undone (nor done again) any more.
    void forget()
    {
        m_step = Step();
        *m_total -= m_bytes;
        m_bytes = 0;
        m_forgotten = true;
    }

private:
    EditSession *m_session;
    Step m_step;
    std::shared_ptr<qint64> m_total;
    qint64 m_bytes = 0;
    bool m_forgotten = false;
};

namespace {
// Every session has a name of its own on the clipboard.
qint64 nextToken()
{
    static std::atomic<qint64> next{1};
    return next++;
}
} // namespace

EditSession::EditSession(WorkbookLoader *loader, const std::shared_ptr<LoadedBook> &book, ValueStore *values, QObject *parent)
    : QObject(parent), m_loader(loader), m_book(book), m_values(values), m_locale(QLocale()), m_today(QDate::currentDate()), m_token(nextToken())
{
    m_undo.setUndoLimit(kUndoSteps);
    connect(&m_undo, &QUndoStack::cleanChanged, this, [this](bool clean) { cleanChanged(clean); });
    if (m_book && m_book->calc) m_pages.open(*m_book->calc);
}

EditSession::~EditSession()
{
    // (the stack tells the session as its steps go when it is destroyed, after the members it would use: it is let go of first)
    m_undo.disconnect(this);
}

namespace {

// The steps a command of the undo stack is: itself, or, for a macro (several steps a person sees as one: a dialog that changes the font and the borders), those it groups.
template <class F>
void eachStep(const QUndoCommand *command, const F &f)
{
    if (const auto *step = dynamic_cast<const ChangeCells *>(command)) f(const_cast<ChangeCells *>(step));
    for (int k = 0; k < command->childCount(); ++k) eachStep(command->child(k), f);
}

// A command is forgotten when it holds steps and none of them is held.
bool commandForgotten(const QUndoCommand *command)
{
    bool any = false, held = false;
    eachStep(command, [&](ChangeCells *step) {
        any = true;
        held = held || !step->forgotten();
    });
    return any && !held;
}

} // namespace

int EditSession::firstHeldStep() const
{
    int i = 0;
    while (i < m_undo.count() && commandForgotten(m_undo.command(i))) ++i;
    return i;
}

void EditSession::forgetDeletesInFile()
{
    // (the same for the sheets added, copied, renamed, deleted, and moved: the saved file has them as they are, and the parts of it that name a sheet were written with the names of the time)
    // (the saved file has lost what was in the rows, columns, and cells that were deleted: the parts of it that name cells were made smaller, and what was on the deleted cells (a note, a
    // link, a picture) is not in it, so an Undo of the delete could not give it back to the file. Undo goes back as far as the last delete saved, no further. Every other step, those that
    // move cells included, stays: Undo of it is made against the saved file.)
    int last = -1;
    for (int i = m_undo.index() - 1; i >= 0 && last < 0; --i)
        eachStep(m_undo.command(i), [&](ChangeCells *step) {
            if (step->deletes() || step->sheetStructure()) last = i;
        });
    // (an object put there that was undone and is not in the saved file could not be put there again by Redo: that step, and the ones after it, are given up)
    int firstUndoneAdd = -1;
    for (int i = m_undo.index(); i < m_undo.count() && firstUndoneAdd < 0; ++i)
        eachStep(m_undo.command(i), [&](ChangeCells *step) {
            if (step->adds()) firstUndoneAdd = i;
        });
    if (firstUndoneAdd >= 0)
        for (int i = firstUndoneAdd; i < m_undo.count(); ++i) eachStep(m_undo.command(i), [](ChangeCells *step) { step->forget(); });
    m_forgotBySave = 0;
    for (int i = 0; i <= last; ++i) {
        if (!commandForgotten(m_undo.command(i))) ++m_forgotBySave;
        eachStep(m_undo.command(i), [](ChangeCells *step) { step->forget(); });
    }
}

void EditSession::forgetStepsDone()
{
    for (int i = 0; i < m_undo.index(); ++i) eachStep(m_undo.command(i), [](ChangeCells *step) { step->forget(); });
}

void EditSession::giveUpUndo(const QString &stepText, bool redoing)
{
    // (the workbook is not as the stack says: no step is undone or done again from here)
    for (int i = 0; i < m_undo.count(); ++i) eachStep(m_undo.command(i), [](ChangeCells *step) { step->forget(); });
    Q_EMIT stepFailed(redoing ? tr("\"%1\" could not be done again, so Undo and Redo are not available any more for what came before it.").arg(stepText)
                              : tr("\"%1\" could not be undone, so Undo and Redo are not available any more for what came before it.").arg(stepText));
}

bool EditSession::canUndo() const { return m_undo.canUndo() && m_undo.index() > firstHeldStep(); }

void EditSession::setUndoByteLimit(qint64 bytes)
{
    m_undoByteLimit = std::max<qint64>(0, bytes);
    trimUndo();
}

void EditSession::trimUndo()
{
    // the oldest steps first; the newest that is done is never forgotten, nor are those that were undone (they come after it, and are not the oldest)
    for (int i = firstHeldStep(); *m_undoBytes > m_undoByteLimit && i < m_undo.index() - 1; ++i)
        eachStep(m_undo.command(i), [](ChangeCells *step) { step->forget(); });
}

void EditSession::cleanChanged(bool clean)
{
    if (!clean) return;
    // the workbook is as the file has it: what the file lacks is nothing. What the engine had for a formula before the first change is kept: the results that follow an Undo are still to
    // come, and they are compared with it (not with the values the store has, which are the edit's); it is let go when the worker has caught up (calculationFinished), or on a save.
    m_touched.clear();
    m_results.clear();
    m_columnsResized.clear();
    m_rowsResized.clear();
    m_mergesChanged.clear();
    m_formatsChanged.clear();
    m_sheetsListed = false;
    m_statesChanged.clear();
    m_tabColorsChanged.clear();
    m_sheetOps.clear();
    m_copies.clear();
}

void EditSession::calculationFinished()
{
    // everything the worker was to work out is told, and the workbook is as the file has it: no result is still to come that the engine's values before the edit would be needed for
    if (m_undo.isClean()) m_engineAtStart.clear();
}

bool EditSession::canEdit(QString *why) const
{
    auto refuse = [&](const QString &text) {
        if (why) *why = text;
        return false;
    };
    if (!m_book || !m_book->book || !m_book->calc || !m_loader->canHold()) return refuse(tr("The workbook is still being opened. It can be changed when it is open."));
    if (m_busy) return refuse(tr("The workbook is being saved. It can be changed when that is done."));
    if (!m_book->problem.isEmpty() || !m_book->book->sharedStringsError().isEmpty() || !m_book->book->stylesError().isEmpty())
        return refuse(tr("Some of this workbook could not be read whole, so JeffCrunch does not change it. You can look at it, and save nothing."));
    return true;
}

bool EditSession::canEditSheet(int sheet, QString *why) const
{
    auto refuse = [&](const QString &text) {
        if (why) *why = text;
        return false;
    };
    if (!m_book || !m_book->book || sheet < 0 || sheet >= m_book->book->info().sheets.size() || m_book->book->info().sheets[sheet].deleted) return refuse(tr("There is no such sheet."));
    if (m_book->book->info().sheets[sheet].kind != SheetKind::Worksheet) return refuse(tr("This sheet has no cells to change."));
    return true;
}

bool EditSession::canEditCell(int sheet, quint32 row, quint32 col, QString *why) const
{
    auto refuse = [&](const QString &text) {
        if (why) *why = text;
        return false;
    };
    if (!canEditSheet(sheet, why)) return false;
    if (row >= kMaxRows || col >= kMaxColumns) return refuse(tr("That cell is outside the sheet."));
    const Calculation::CellContent c = m_book->calc->content(sheet, row, col);
    if (!c.editable) return refuse(tr("You can't change part of an array or a data table."));
    if (isLocked(sheet, c.style)) return refuse(lockedMessage());
    return true;
}

bool EditSession::isLocked(int sheet, quint32 style) const
{
    if (!m_book || !m_book->calc || !m_book->book || sheet < 0 || sheet >= m_book->calc->sheetCount()) return false;
    return m_book->calc->sheet(sheet).isProtected && m_book->book->styles().format(style).locked;
}

QString EditSession::lockedMessage() const
{
    return tr("This sheet is protected, and the cell is locked, so it can't be changed. To change it, take the protection off in the program that put it on.");
}

QString EditSession::formatCodeOf(int sheet, quint32 row, quint32 col) const
{
    if (!m_book || !m_book->calc || !m_book->book) return QStringLiteral("General");
    const Calculation::CellContent c = m_book->calc->content(sheet, row, col);
    return m_book->book->styles().formatCode(c.style, m_locale);
}

EntryOptions EditSession::entryOptions(int, bool textFormat) const
{
    EntryOptions o;
    o.locale = m_locale;
    o.dateSystem = m_book->book->info().dateSystem;
    o.today = m_today;
    o.textFormat = textFormat;
    return o;
}

QString EditSession::editText(int sheet, quint32 row, quint32 col) const
{
    if (!m_book || !m_book->calc) return {};
    const Calculation::CellContent c = m_book->calc->content(sheet, row, col);
    if (!c.exists) return {};
    if (c.isFormula) return QLatin1Char('=') + formulaToLocale(jp::grid::displayFormula(c.formula), m_locale);
    const QString code = m_book->book->styles().formatCode(c.style, m_locale);
    return editTextOfValue(c.value, code, entryOptions(sheet, code == QLatin1String("@")));
}

QString EditSession::barText(int sheet, quint32 row, quint32 col, const QString &gridText) const
{
    // (a locale that writes the decimal point as a point has nothing to change: what the grid gives is what is typed)
    if (!m_book || !m_book->calc || m_locale.decimalPoint() != QLatin1String(",")) return gridText;
    const Calculation::CellContent c = m_book->calc->content(sheet, row, col);
    if (c.isFormula) {
        // "=formula" or "{=formula}" (an array's): the formula part in the locale's form
        const int at = gridText.indexOf(QLatin1Char('='));
        if (at < 0 || at > 1) return gridText;
        const bool braces = at == 1 && gridText.endsWith(QLatin1Char('}'));
        const QString formula = gridText.mid(at + 1, gridText.size() - at - 1 - (braces ? 1 : 0));
        return gridText.left(at + 1) + formulaToLocale(formula, m_locale) + (braces ? QStringLiteral("}") : QString());
    }
    // a number the grid gave as it is stored (a date and a percent are given as their format shows them) is written with the locale's decimal mark
    if (c.value.isNumber() && gridText == jp::calc::numberToText(c.value.number())) return editTextOfValue(c.value, QStringLiteral("General"), entryOptions(sheet, false));
    return gridText;
}

bool EditSession::typedChange(int sheet, quint32 row, quint32 col, const QString &typed, CellChange *out, bool *none, Outcome *problem)
{
    const Calculation *calc = m_book->calc.get();
    const Calculation::CellContent before = calc->content(sheet, row, col);
    // (a cell that holds nothing has its row's or its column's format, and a cell typed in takes it)
    const quint32 baseStyle = before.exists ? before.style : implicitStyle(sheet, row, col);
    const QString code = m_book->book->styles().formatCode(baseStyle, m_locale);
    const EntryResult r = readEntry(typed, entryOptions(sheet, code == QLatin1String("@")));
    if (!r.ok) {
        problem->ok = false;
        problem->problem = r.problem;
        problem->problemAt = r.problemAt;
        return false;
    }
    CellChange ch;
    ch.sheet = sheet;
    ch.row = row;
    ch.col = quint16(col);
    ch.before = before;
    ch.after = before;
    ch.after.exists = true;
    ch.after.isFormula = false;
    ch.after.formula.clear();
    ch.after.value = Value();
    ch.after.sharedString = -1;
    ch.after.style = baseStyle;
    const Entry &e = r.entry;
    switch (e.kind) {
    case Entry::Kind::Empty: break;
    case Entry::Kind::Number:
        ch.after.value = Value::number(e.number);
        // a cell that shows General takes the format that shows the entry as typed (one that has a format of its own keeps it)
        if (!e.formatCode.isEmpty() && (code == QLatin1String("General") || code.isEmpty())) ch.formatCode = e.formatCode;
        break;
    case Entry::Kind::Text:
        ch.after.value = Value::text(e.text);
        ch.textToShare = !m_book->book->sharedStringsPart().isEmpty();
        break;
    case Entry::Kind::Bool: ch.after.value = Value::boolean(e.number != 0); break;
    case Entry::Kind::Error: ch.after.value = Value::error(e.error); break;
    case Entry::Kind::Formula:
        ch.after.isFormula = true;
        ch.after.formula = e.text;
        break;
    }
    if (ch.after.value.isEmpty() && !ch.after.isFormula && (ch.after.style == 0 || !before.exists)) ch.after.exists = false;
    *none = sameMeaning(ch.before, ch.after) && ch.formatCode.isEmpty();   // (nothing to change)
    *out = std::move(ch);
    return true;
}

quint32 EditSession::implicitStyle(int sheet, quint32 row, quint32 col) const
{
    const Calculation *calc = m_book->calc.get();
    for (const RowSpec &r : calc->rowSpecsIn(sheet, row, row))
        if (r.row == row && r.customFormat && r.style != 0) return r.style;
    for (const ColumnSpan &c : calc->columnRuns(sheet, col, col))
        if (c.first <= col && col <= c.last) return c.style;
    return 0;
}

EditSession::Outcome EditSession::checkRule(int sheet, quint32 row, quint32 col, const Value &value, bool isFormula, const QString &typed) const
{
    Outcome out;
    const Calculation *calc = m_book->calc.get();
    const jp::office::sheet::Validation *rule = calc->validationAt(sheet, row, col);
    // (an entry that is empty, or a formula, is not held to a rule, nor is any when the file asks for no alert)
    if (!rule || rule->type == jp::office::sheet::Validation::Type::Any || !rule->showError || isFormula || value.isEmpty()) return out;
    {
        // (the rule is worked out with the worker held back: it reads the cells through the calculation, which the worker is working in)
        std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
        // (an entry is never taken unchecked: if the worker is not to be held for the moment, wait for it a little)
        for (int waited = 0; !hold && waited < kHoldWaitMs; waited += 10) {
            QThread::msleep(10);
            hold = m_loader->hold();
        }
        if (!hold) {
            out.ok = false;
            out.problem = tr("JeffCrunch could not check what you typed against the rule for this cell just now, so it was not entered. Try again in a moment.");
            return out;
        }
        if (hold->calculation()->entryAllowed(sheet, row, col, *rule, value, typed)) return out;
    }
    out.ok = false;
    out.rule = true;
    out.ruleStyle = int(rule->style);
    out.ruleTitle = rule->errorTitle.isEmpty() ? tr("Invalid Entry") : rule->errorTitle;
    if (!rule->error.isEmpty()) out.problem = rule->error;
    else if (rule->style == jp::office::sheet::Validation::Style::Stop) out.problem = tr("That isn't allowed in this cell. Enter a value that fits the rule for the cell.");
    else if (rule->style == jp::office::sheet::Validation::Style::Warning) out.problem = tr("That doesn't fit the rule for this cell. Do you want to keep it anyway?");
    else out.problem = tr("That doesn't fit the rule for this cell.");
    return out;
}

EditSession::Outcome EditSession::type(int sheet, quint32 row, quint32 col, const QString &typed, bool checkRule_)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || !canEditCell(sheet, row, col, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    // a cell of a table's header row names a column
    int column = -1;
    const int table = m_book->book->tableHeaderAt(sheet, row, col, &column);
    if (table >= 0) return typeInHeader(sheet, row, col, table, column, typed);
    CellChange ch;
    bool none = false;
    if (!typedChange(sheet, row, col, typed, &ch, &none, &out)) return out;
    if (none) return out;
    if (checkRule_) {
        const Outcome rule = checkRule(sheet, row, col, ch.after.value, ch.after.isFormula, typed.trimmed());
        if (!rule.ok) return rule;
    }
    Step step;
    step.cells.push_back(std::move(ch));
    pushStep(std::move(step), tr("Typing in %1").arg(cellName(row, col)));
    return out;
}

void EditSession::pushStep(Step step, const QString &text)
{
    m_undo.push(new ChangeCells(this, std::move(step), text, m_undoBytes));
    trimUndo();
}

EditSession::Outcome EditSession::typeInHeader(int sheet, quint32 row, quint32 col, int table, int column, const QString &typed)
{
    Outcome out;
    const Table &t = m_book->book->tables()[table];
    bool tooLong = false;
    const QString name = uniqueColumnName(typed, t.columns, column, &tooLong);
    if (name.isEmpty()) {
        out.ok = false;
        out.problem = tooLong ? tr("A column's name can have up to 255 characters. Shorten it, and try again.")
                              : tr("The first row of a table holds the names of its columns, so a cell there needs a name. Type one.");
        return out;
    }
    const QString oldName = t.columns[column];
    const Calculation::CellContent before = m_book->calc->content(sheet, row, col);
    if (name == oldName && before.value == Value::text(name)) return out;   // (nothing to change)
    std::vector<CellChange> changes;
    CellChange ch;
    ch.sheet = sheet;
    ch.row = row;
    ch.col = quint16(col);
    ch.before = before;
    ch.after = before;
    ch.after.exists = true;
    ch.after.isFormula = false;
    ch.after.formula.clear();
    ch.after.value = Value::text(name);
    ch.after.sharedString = -1;
    ch.textToShare = !m_book->book->sharedStringsPart().isEmpty();
    changes.push_back(std::move(ch));
    // the formulas that name the column follow
    if (name != oldName) {
        bool tooMany = false;
        {
            std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
            if (!hold) {
                out.ok = false;
                out.problem = tr("The workbook is busy. Try again in a moment.");
                return out;
            }
            const Calculation *calc = hold->calculation();
            calc->forEachFormulaText([&](int s, quint32 r, quint32 c, const QString &text) {
                if (tooMany || !text.contains(QLatin1Char('['))) return;
                const bool inside = s == t.sheet && r >= t.row1 + quint32(t.headerRows) && r <= t.row2 && c >= t.col1 && c <= t.col2;
                const QString renamed = renameColumnInFormula(text, t.name, oldName, name, inside);
                if (renamed == text) return;
                if (qint64(changes.size()) >= kMaxCellsPerEdit) {
                    tooMany = true;
                    return;
                }
                CellChange f;
                f.sheet = s;
                f.row = r;
                f.col = quint16(c);
                f.before = calc->content(s, r, c);
                f.after = f.before;
                f.after.formula = renamed;
                changes.push_back(std::move(f));
            });
        }
        if (tooMany) {
            out.ok = false;
            out.problem = tr("More formulas than JeffCrunch changes at once (%1) use this column's name.").arg(QLocale().toString(kMaxCellsPerEdit));
            return out;
        }
    }
    Step step;
    step.cells = std::move(changes);
    if (name != oldName) step.renames.push_back({table, column, oldName, name});
    pushStep(std::move(step), tr("Typing in %1").arg(cellName(row, col)));
    return out;
}

EditSession::Outcome EditSession::clear(int sheet, qint64 row1, qint64 col1, qint64 row2, qint64 col2)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    const Calculation *calc = m_book->calc.get();
    const Sheet &s = calc->sheet(sheet);
    std::vector<CellChange> changes;
    bool tooMany = false, array = false, locked = false, header = false;
    s.cells.forEachInRange(quint32(row1), quint32(col1), quint32(row2), quint32(col2), [&](const Cell &c) {
        const Calculation::CellContent before = calc->content(sheet, c.row, c.col);
        if (!before.editable) {
            array = true;
            return false;
        }
        if (!before.isFormula && before.value.isEmpty()) return true;   // (only a format: it stays)
        if (m_book->book->tableHeaderAt(sheet, c.row, c.col, nullptr) >= 0) {
            header = true;
            return false;
        }
        if (isLocked(sheet, before.style)) {
            locked = true;
            return false;
        }
        if (qint64(changes.size()) >= kMaxCellsPerEdit) {
            tooMany = true;
            return false;
        }
        CellChange ch;
        ch.sheet = sheet;
        ch.row = c.row;
        ch.col = c.col;
        ch.before = before;
        ch.after = before;
        ch.after.isFormula = false;
        ch.after.formula.clear();
        ch.after.value = Value();
        ch.after.sharedString = -1;
        if (ch.after.style == 0) ch.after.exists = false;
        changes.push_back(std::move(ch));
        return true;
    });
    if (array) {
        out.ok = false;
        out.problem = tr("You can't change part of an array or a data table.");
        return out;
    }
    if (locked) {
        out.ok = false;
        out.problem = lockedMessage();
        return out;
    }
    if (header) {
        out.ok = false;
        out.problem = tr("The first row of a table holds the names of its columns, so it can't be cleared. Select the cells below it, or type another name.");
        return out;
    }
    if (tooMany) {
        out.ok = false;
        out.problem = tr("That is more cells than JeffCrunch clears at once (%1). Select fewer cells.").arg(QLocale().toString(kMaxCellsPerEdit));
        return out;
    }
    if (changes.empty()) return out;
    const QString what = changes.size() == 1 ? tr("Clearing %1").arg(cellName(changes[0].row, changes[0].col))
                                             : QCoreApplication::translate("jp::crunch::EditSession", "Clearing %n cell(s)", nullptr, int(changes.size()));
    Step step;
    step.cells = std::move(changes);
    pushStep(std::move(step), what);
    return out;
}

bool EditSession::apply(Step *step, bool forward)
{
    for (const ObjectEdit &o : step->objects)
        if (m_objectHook) m_objectHook(o, forward);
    for (const PageEdit &e : step->pages) {
        m_pages.set(e.sheet, forward ? e.after : e.before);
        Q_EMIT pageSetupChanged(e.sheet);
    }
    // (a step of objects or of page setups alone changes no cell: the worker is not held for it)
    if ((!step->objects.empty() || !step->pages.empty()) && step->cells.empty() && step->renames.empty() && step->sizes.empty() && step->merges.empty() && !step->structure) return true;
    if (step->structure) return applyStructure(step, forward);
    QElapsedTimer waited;
    waited.start();
    std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
    m_lastHoldWait = waited.nsecsElapsed() / 1000;
    m_longestHoldWait = std::max(m_longestHoldWait, m_lastHoldWait);
    if (!hold) return false;
    Calculation *calc = hold->calculation();
    Workbook &book = *m_book->book;
    if (m_holdHook) m_holdHook(calc, false);
    std::map<int, std::vector<ValueStore::Item>> cells;
    std::vector<ValueStore::Item> formulas;
    {
        Calculation::BulkEdit bulk(calc);   // (what waits to be worked out is put right once, not at each of the cells)
        for (CellChange &ch : step->cells) {
            if (forward) {
                if (ch.textToShare) {
                    ch.after.sharedString = book.addSharedString(ch.after.value.text());
                    ch.textToShare = false;
                }
                if (!ch.formatCode.isEmpty()) {
                    ch.after.style = book.formatWithNumberFormat(ch.after.style, book.numberFormatId(ch.formatCode));
                    ch.formatCode.clear();
                }
            }
            const Calculation::CellContent &c = forward ? ch.after : ch.before;
            if (ch.styleOnly) {
                // only the cell's format changes: its value, its formula, and the array it may be part of are not touched
                calc->setStyle(ch.sheet, ch.row, ch.col, c.style);
                m_touched.insert(keyOf(ch.sheet, ch.row, ch.col));
                ValueStore::Item styled;
                styled.sheet = ch.sheet;
                styled.row = ch.row;
                styled.col = ch.col;
                cells[ch.sheet].push_back(styled);
                continue;
            }
            calc->put(ch.sheet, ch.row, ch.col, c, Calculation::Marking::Later);
            m_touched.insert(keyOf(ch.sheet, ch.row, ch.col));
            ValueStore::Item it;
            it.sheet = ch.sheet;
            it.row = ch.row;
            it.col = ch.col;
            cells[ch.sheet].push_back(it);
            if (c.isFormula) {
                // (a formula the engine keeps apart has the result it had, which the worker does not tell: the cell shows it at once)
                if (c.hasStored) it.value = c.stored;
                formulas.push_back(it);
            }
        }
    }
    {
        // the cells whose format alone changed: told one by one, or, when there are very many, as the whole sheet being drawn again
        const bool many = step->styles.size() > 20000;
        std::set<int> sheetsStyled;
        for (const StyleCell &sc : step->styles) {
            calc->setStyle(sc.sheet, sc.row, sc.col, forward ? sc.after : sc.before);
            m_touched.insert(keyOf(sc.sheet, sc.row, sc.col));
            sheetsStyled.insert(sc.sheet);
            if (many) continue;
            ValueStore::Item styled;
            styled.sheet = sc.sheet;
            styled.row = sc.row;
            styled.col = sc.col;
            cells[sc.sheet].push_back(styled);
        }
        if (many)
            for (const int sheet : sheetsStyled) m_sheetsRestyled.insert(sheet);
    }
    for (TableRename &r : step->renames) {
        book.setTableColumnName(r.table, r.column, forward ? r.after : r.before);
        // the defined names that name the column follow, and the file's other parts that do are told at a save
        const QString table = r.table >= 0 && r.table < book.tables().size() ? book.tables()[r.table].name : QString();
        const jp::office::sheet::ColumnRename was{table, r.before, r.after}, back{table, r.after, r.before};
        if (forward) {
            r.names = calc->renameColumnInNames(table, r.before, r.after);
            m_renames.push_back(was);
        } else {
            calc->restoreNames(r.names);
            if (!m_renames.empty() && m_renames.back() == was) m_renames.pop_back();
            else m_renames.push_back(back);
        }
    }
    applySizes(step, forward, calc);
    for (const MergeEdit &m : step->merges) {
        calc->setMerges(m.sheet, forward ? m.after : m.before, forward ? m.countAfter : m.countBefore);
        m_mergesChanged.insert(m.sheet);
    }
    for (const FormatEdit &f : step->formats) {
        calc->setSheetFormat(f.sheet, forward ? f.after : f.before);
        m_formatsChanged.insert(f.sheet);
    }
    if (!applySheetEdits(step, forward, calc, book)) return false;
    if (m_holdHook) m_holdHook(calc, true);
    // (a change of formats alone leaves nothing for the worker to work out)
    bool valuesChanged = !step->renames.empty();
    for (const CellChange &ch : step->cells) valuesChanged = valuesChanged || !ch.styleOnly;
    for (const SheetEdit &e : step->sheets) valuesChanged = valuesChanged || e.kind == SheetEdit::Kind::Insert || e.kind == SheetEdit::Kind::Copy || e.kind == SheetEdit::Kind::Delete || e.kind == SheetEdit::Kind::Rename;
    if (valuesChanged) hold->changed();
    hold.reset();   // (the worker has the model again, and works out what these changes make)
    m_tracking = true;
    // a formula put in a cell has no value until the worker gives it one: it shows nothing, not the value of what was there
    if (!formulas.empty()) m_values->apply(formulas);
    for (auto &entry : cells) Q_EMIT cellsChanged(entry.first, entry.second);
    for (const SizeEdit &e : step->sizes) Q_EMIT sizesChanged(e.sheet);
    for (const int sheet : m_sheetsRestyled) Q_EMIT sizesChanged(sheet);
    m_sheetsRestyled.clear();
    for (const MergeEdit &m : step->merges) Q_EMIT sizesChanged(m.sheet);
    for (const FormatEdit &f : step->formats) Q_EMIT sizesChanged(f.sheet);
    if (!step->sheets.empty()) Q_EMIT sheetsChanged();
    return true;
}

bool EditSession::widenLastStep(int index, int sheet, quint32 col, double width)
{
    auto *cmd = const_cast<ChangeCells *>(dynamic_cast<const ChangeCells *>(m_undo.command(index)));
    if (!cmd || cmd->forgotten() || cmd->step()->structure || cmd->step()->cells.empty() || col >= kMaxColumns) return false;
    bool inColumn = false;
    for (const CellChange &c : cmd->step()->cells) inColumn = inColumn || (c.sheet == sheet && c.col == col);
    if (!inColumn) return false;
    SizeEdit e;
    e.sheet = sheet;
    e.columns = true;
    e.first = e.last = col;
    e.columnsBefore = m_book->calc->columnRuns(sheet, col, col);
    ColumnSpan c = e.columnsBefore.empty() ? ColumnSpan() : e.columnsBefore.front();
    if (!e.columnsBefore.empty() && (c.customWidth || c.hidden || (c.width >= 0 && c.width >= width))) return false;   // (a width a person set stays, and a column wide enough is left)
    c.first = c.last = col;
    c.width = width;
    c.customWidth = false;
    e.columnsAfter.push_back(c);
    std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
    if (!hold) return false;
    hold->calculation()->setColumnRuns(sheet, col, col, e.columnsAfter);
    hold.reset();
    m_columnsResized.insert(keyOf(sheet, 0, col));
    cmd->step()->sizes.push_back(e);
    cmd->recount();
    Q_EMIT sizesChanged(sheet);
    return true;
}

void EditSession::applySizes(Step *step, bool forward, Calculation *calc)
{
    for (const SizeEdit &e : step->sizes) {
        if (e.columns) {
            calc->setColumnRuns(e.sheet, e.first, e.last, forward ? e.columnsAfter : e.columnsBefore);
            for (quint32 c = e.first; c <= e.last; ++c) m_columnsResized.insert(keyOf(e.sheet, 0, c));
        } else {
            calc->setRowSpecs(e.sheet, e.first, e.last, forward ? e.rowsAfter : e.rowsBefore);
            for (quint32 r = e.first; r <= e.last; ++r) m_rowsResized.insert(keyOf(e.sheet, r, 0));
        }
    }
}

bool EditSession::storedEquals(int sheet, quint32 row, quint32 col, const Value &value) const
{
    const Sheet &s = m_book->calc->sheet(sheet);
    const Cell *c = s.cells.find(row, col);
    if (!c) return false;
    switch (c->type) {
    case CellType::Empty: return value.isEmpty();
    case CellType::Number:
    case CellType::Date: return sameValue(Value::number(c->number), value);
    case CellType::SharedString: return value.isText() && value.text() == m_book->book->sharedStrings().at(c->textIndex());
    case CellType::InlineString:
    case CellType::FormulaString: return value.isText() && value.text() == s.texts.value(c->textIndex());
    case CellType::Bool: return value.isBool() && value.boolean() == (c->number != 0);
    case CellType::Error: return value.isError() && int(value.errorKind()) == int(c->number);
    }
    return false;
}

void EditSession::noteResults(const std::vector<ValueStore::Item> &items)
{
    if (!m_tracking || !m_book || !m_book->calc) return;
    for (const ValueStore::Item &it : items) {
        const quint64 key = keyOf(it.sheet, it.row, it.col);
        if (m_touched.count(key)) continue;
        const Sheet &s = m_book->calc->sheet(it.sheet);
        const Cell *c = s.cells.find(it.row, it.col);
        if (!c || !(c->hasFormula() || (c->flags & ArrayMember))) continue;
        // What the engine had for the cell before this (the store is told after): a result that is the same is not a change, and the file's own value stays (the engine's differences from the
        // program that wrote the file are not made into changes by a formula being worked out again). A result that comes back to the value the engine had before the first change is the file's again.
        Value before;
        const bool hadBefore = m_values->find(it.sheet, it.row, it.col, &before);
        const auto base = m_engineAtStart.find(key);
        if (base != m_engineAtStart.end()) {
            if (sameValue(base->second, it.value)) {
                m_results.erase(key);
                continue;
            }
        } else if (hadBefore && sameValue(before, it.value)) {
            continue;
        } else if (hadBefore) {
            m_engineAtStart[key] = before;
        }
        const auto file = m_fileResults.find(key);
        const bool same = file != m_fileResults.end() ? sameValue(file->second, it.value) : storedEquals(it.sheet, it.row, it.col, it.value);
        if (same) m_results.erase(key);
        else m_results[key] = it.value;
    }
}

bool EditSession::takeSnapshot(Snapshot *out, QString *why)
{
    auto refuse = [&](const QString &text) {
        if (why) *why = text;
        return false;
    };
    if (!m_book || !m_book->book || !m_book->calc) return refuse(tr("There is no workbook to save."));
    if (!m_book->problem.isEmpty() || !m_book->book->sharedStringsError().isEmpty() || !m_book->book->stylesError().isEmpty())
        return refuse(tr("Some of this workbook could not be read whole, so JeffCrunch does not save it."));
    // (a workbook read from the older binary form has no package yet: the first save makes one)
    if (!m_book->book->package().isLazy() && !m_book->book->isConverted()) return refuse(tr("This workbook was not opened from a file JeffCrunch can save over."));
    const Calculation *calc = m_book->calc.get();
    Workbook &book = *m_book->book;
    *out = Snapshot();
    SaveChanges &changes = out->changes;
    const bool hasShared = !book.sharedStringsPart().isEmpty();
    const bool hasStyles = !book.stylesPart().isEmpty();
    bool resultMissing = false;
    std::vector<quint64> touchedInOrder(m_touched.begin(), m_touched.end());
    std::sort(touchedInOrder.begin(), touchedInOrder.end());
    // (a sheet that was added is made whole from the model, and one that was deleted is not in the file: the marks of their cells are not what is put in)
    const WorkbookInfo &listed = book.info();
    auto written = [&](int sheet) { return sheet >= 0 && sheet < listed.sheets.size() && !listed.sheets[sheet].deleted && !listed.sheets[sheet].isNew; };
    auto addCell = [&](int sheet, quint32 row, quint32 col) {
        const quint64 key = keyOf(sheet, row, col);
        const Calculation::CellContent c = calc->content(sheet, row, col);
        SavedCell cell;
        cell.row = row;
        cell.col = quint16(col);
        if (c.exists) {
            cell.style = c.style;
            cell.isFormula = c.isFormula;
            if (c.isFormula) {
                cell.formula = c.formula;
                if (c.arrayRange.valid) cell.arrayRef = c.arrayRange.cellCount() > 1 ? rangeName(c.arrayRange) : cellName(c.arrayRange.row1, c.arrayRange.col1);
                Value v;
                // (a formula the engine keeps apart has the result it had, which the worker does not tell)
                const bool known = m_values->find(sheet, row, col, &v) || (c.hasStored && ((v = c.stored), true));
                if (known) {
                    cell.value = savedValue(v, -1);
                    out->touchedFormulas.push_back(key);
                    out->touchedResults[key] = v;
                } else {
                    cell.hasResult = false;
                    resultMissing = true;
                }
            } else {
                cell.value = savedValue(c.value, hasShared ? c.sharedString : -1);
            }
        }
        changes.sheets[sheet].cells.push_back(cell);
    };
    for (const quint64 key : touchedInOrder) {
        const int sheet = int(key >> 40);
        if (!written(sheet)) continue;
        addCell(sheet, quint32((key >> 16) & 0xFFFFFF), quint32(key & 0xFFFF));
    }
    // the sheets a person added: every cell, and what the sheet has besides
    bool newSheets = false;
    if (m_sheetsListed) {
        changes.sheetsChanged = true;
        changes.sheetOps = m_sheetOps;
        for (int sheet = 0; sheet < listed.sheets.size(); ++sheet) {
            if (listed.sheets[sheet].deleted || !listed.sheets[sheet].isNew) continue;
            book.planSheetPart(sheet);
            newSheets = true;
            const Sheet &model = calc->sheet(sheet);
            changes.sheets[sheet];   // (made, though the sheet is empty)
            model.cells.forEach([&](const Cell &c) { addCell(sheet, c.row, c.col); });
            SheetChanges &sc = changes.sheets[sheet];
            sc.skeleton = newSheetSkeleton(model);
            sc.isProtected = model.isProtected;
            if (const CopyPoint *copy = copyPointOf(sheet)) {
                // a copy: its file begins as the file of the sheet it was copied from (the first sheet of the chain of copies that has a part), moved by the edits to rows and columns made to
                // the cells it came from before the copy, each as an edit to the copy
                struct Span {
                    int sheet;
                    size_t from, to;
                };
                std::vector<Span> spans;
                int source = copy->source;
                size_t to = copy->edits;
                for (;;) {
                    const CopyPoint *earlier = copyPointOf(source);
                    spans.push_back({source, earlier ? earlier->edits : 0, to});
                    if (!earlier) break;
                    to = earlier->edits;
                    source = earlier->source;
                }
                if (source >= 0 && source < listed.sheets.size() && !listed.sheets[source].isNew && !listed.sheets[source].part.isEmpty()) {
                    sc.copyFromPart = listed.sheets[source].part;
                    for (auto span = spans.rbegin(); span != spans.rend(); ++span)
                        for (size_t i = span->from; i < span->to && i < m_edits.size(); ++i) {
                            StructEdit edit = m_edits[i];
                            if (edit.kind == StructEdit::Kind::Offset) continue;
                            bool touches = false;
                            if (edit.sheet == span->sheet) {
                                edit.sheet = sheet;
                                touches = true;
                            }
                            if (edit.kind == StructEdit::Kind::Move && edit.toSheet == span->sheet) {
                                edit.toSheet = sheet;
                                touches = true;
                            }
                            if (touches) sc.copyEdits.push_back(edit);
                        }
                }
            }
            sc.replaceColumns = true;
            for (const ColumnSpan &c : calc->columnRuns(sheet, 0, kMaxColumns - 1)) sc.columns.push_back(c);
            if (calc->mergeCountOf(sheet) == qint64(calc->mergesOf(sheet).size())) {
                sc.replaceMerges = true;
                for (const CellRange &m : calc->mergesOf(sheet)) sc.merges.push_back(m);
            }
            for (const RowSpec &spec : calc->rowSpecsIn(sheet, 0, kMaxRows - 1)) {
                RowProps p;
                p.row = spec.row;
                p.height = spec.height;
                p.hidden = spec.hidden;
                p.customHeight = spec.customHeight;
                p.styleKnown = true;
                p.customFormat = spec.customFormat;
                p.style = spec.style;
                p.outlineKnown = true;
                p.outline = spec.outline;
                p.collapsed = spec.collapsed;
                sc.rows.push_back(p);
            }
        }
        for (const int sheet : m_tabColorsChanged) {
            if (!written(sheet)) continue;
            SheetChanges &sc = changes.sheets[sheet];
            sc.replaceTabColor = true;
            sc.tabColor = calc->sheet(sheet).tabColor;
        }
        // the sheet that is open has its tab selected, and a sheet that is hidden, or was the one that was open and is not now, does not (the workbook says which tab is open: where the
        // file said, or the first that is shown if that sheet is hidden or gone)
        {
            const int wasAt = listed.activeTab >= 0 && listed.activeTab < listed.fileOrder.size() ? listed.fileOrder[listed.activeTab] : -1;
            const int nowTab = book.activeTabNow();
            const int nowAt = nowTab >= 0 && nowTab < listed.order.size() ? listed.order[nowTab] : -1;
            for (int sheet = 0; sheet < listed.sheets.size(); ++sheet) {
                const SheetEntry &entry = listed.sheets[sheet];
                if (entry.deleted) continue;
                int want = -1;
                if (sheet == nowAt && nowAt != wasAt) want = 1;
                else if ((entry.state != SheetState::Visible && m_statesChanged.count(sheet)) || (sheet == wasAt && sheet != nowAt)) want = 0;
                if (want < 0 || (entry.isNew && want == 0)) continue;
                changes.sheets[sheet].tabSelected = want;
            }
        }
    }
    // the runs of columns of the sheets whose columns were sized or inserted or deleted, as the sheet has them now; and the sizes of the rows a step changed
    {
        std::set<int> sheets;
        for (const quint64 key : m_columnsResized) sheets.insert(int(key >> 40));
        for (const StructEdit &e : m_edits)
            if (e.kind == StructEdit::Kind::InsertColumns || e.kind == StructEdit::Kind::DeleteColumns) sheets.insert(e.sheet);
        for (const int sheet : sheets) {
            if (!written(sheet)) continue;
            SheetChanges &sc = changes.sheets[sheet];
            sc.replaceColumns = true;
            quint32 next = 0;   // (where the last run ended; a run that overlaps one before it keeps what that one left)
            for (jp::office::sheet::ColumnSpan c : calc->columnRuns(sheet, 0, kMaxColumns - 1)) {
                c.first = std::max(c.first, next);
                if (c.first > c.last) continue;
                next = c.last + 1;
                sc.columns.push_back(c);
            }
        }
    }
    for (const int sheet : m_formatsChanged) {
        if (!written(sheet)) continue;
        SheetChanges &sc = changes.sheets[sheet];
        sc.replaceFormat = true;
        sc.format = calc->sheetFormatOf(sheet);
    }
    for (const int sheet : m_mergesChanged) {
        if (!written(sheet)) continue;
        // (only when all the sheet's merged ranges are held: past the limit the file's are left as they are)
        if (calc->mergeCountOf(sheet) != qint64(calc->mergesOf(sheet).size())) continue;
        SheetChanges &sc = changes.sheets[sheet];
        sc.replaceMerges = true;
        for (const jp::office::sheet::CellRange &m : calc->mergesOf(sheet)) sc.merges.push_back(m);
    }
    for (const quint64 key : m_rowsResized) {
        const int sheet = int(key >> 40);
        if (!written(sheet)) continue;
        const quint32 row = quint32((key >> 16) & 0xFFFFFF);
        const std::vector<jp::office::sheet::RowSpec> specs = calc->rowSpecsIn(sheet, row, row);
        RowProps p;
        p.row = row;
        p.styleKnown = true;
        if (specs.empty()) {
            // (a row that is back to the sheet's own height, shown: the file that has it taller, or hidden, from an earlier save is told so)
            p.height = calc->sheet(sheet).format.defaultRowHeight;
        } else {
            p.height = specs.front().height;
            p.hidden = specs.front().hidden;
            p.customHeight = specs.front().customHeight;
            p.customFormat = specs.front().customFormat;
            p.style = specs.front().style;
        }
        changes.sheets[sheet].rows.push_back(p);
    }
    for (const auto &[key, v] : m_results) {
        if (m_touched.count(key)) continue;
        const int sheet = int(key >> 40);
        if (!written(sheet)) continue;
        SavedResult r;
        r.row = quint32((key >> 16) & 0xFFFFFF);
        r.col = quint16(key & 0xFFFF);
        r.value = savedValue(v, -1);
        changes.sheets[sheet].results.push_back(r);
        out->results[key] = v;
    }
    if (hasShared) {
        changes.newSharedStrings = book.addedSharedStrings();
        changes.sharedStringsBefore = book.sharedStringsInFile();
    }
    changes.replacedParts = book.changedTableParts();
    {
        QStringList names;
        for (const jp::office::sheet::SheetEntry &se : book.info().sheets) names << (se.deleted ? QString() : se.name);
        m_pages.addToSave(&changes, names, book.info().order);
    }
    changes.edits = m_edits;
    changes.renames = m_renames;
    out->strings = int(changes.newSharedStrings.size());
    if (hasStyles) {
        for (const Workbook::AddedNumberFormat &f : book.addedNumberFormats()) changes.newNumberFormats.append({f.id, f.code});
        for (const Workbook::AddedCellFormat &f : book.addedCellFormats()) changes.newCellFormats.append({f.base, f.format.numFmtId, true, f.format});
        changes.cellFormatsBefore = book.cellFormatsInFile();
        changes.newFonts = book.addedFonts();
        changes.fontsBefore = book.fontsInFile();
        changes.newFills = book.addedFills();
        changes.fillsBefore = book.fillsInFile();
        changes.newBorders = book.addedBorders();
        changes.bordersBefore = book.bordersInFile();
        for (const Workbook::AddedStyle &s : book.addedStyles()) changes.newStyles.append({s.format, s.name});
        changes.styleXfsBefore = book.styleXfsInFile();
    }
    out->numberFormats = int(changes.newNumberFormats.size());
    out->cellFormats = int(changes.newCellFormats.size());
    out->fonts = int(changes.newFonts.size());
    out->fills = int(changes.newFills.size());
    out->borders = int(changes.newBorders.size());
    out->styles = int(changes.newStyles.size());
    // The other program is asked to work every formula out again when this program could not: a formula it keeps apart that may read what changed, a result it did not get to, or a
    // formula it gave no result.
    changes.fullCalcOnLoad = (!m_touched.empty() || !m_results.empty() || newSheets) && (calc->isDirty() || resultMissing || calc->keptCount() > 0);
    return true;
}

void EditSession::saved(const Snapshot &snapshot, const Package &package)
{
    Workbook::SavedCounts counts;
    counts.strings = snapshot.strings;
    counts.numberFormats = snapshot.numberFormats;
    counts.cellFormats = snapshot.cellFormats;
    counts.fonts = snapshot.fonts;
    counts.fills = snapshot.fills;
    counts.borders = snapshot.borders;
    counts.styles = snapshot.styles;
    m_book->book->savedTo(package, counts);
    for (const auto &[key, v] : snapshot.results) m_fileResults[key] = v;
    for (const auto &[key, v] : snapshot.touchedResults) m_fileResults[key] = v;
    m_touched.clear();
    m_columnsResized.clear();
    m_rowsResized.clear();
    m_mergesChanged.clear();
    m_formatsChanged.clear();
    m_sheetsListed = false;
    m_statesChanged.clear();
    m_tabColorsChanged.clear();
    m_sheetOps.clear();
    m_copies.clear();
    m_engineAtStart.clear();
    m_edits.clear();
    m_renames.clear();
    ++m_saveGen;
    m_pages.saved();
    forgetDeletesInFile();
    // what differs from the file is what differs from the file just written
    for (auto it = m_results.begin(); it != m_results.end();) {
        const auto file = m_fileResults.find(it->first);
        if (file != m_fileResults.end() && sameValue(file->second, it->second)) it = m_results.erase(it);
        else ++it;
    }
    m_undo.setClean();
}

} // namespace jp::crunch
