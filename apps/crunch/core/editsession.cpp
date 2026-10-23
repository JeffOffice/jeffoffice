#include "core/editsession.h"

#include "grid/text.h"
#include "sheet/cellref.h"

#include <QCoreApplication>
#include <QUndoCommand>

#include <algorithm>

namespace jp::crunch {

using namespace jp::office;
using namespace jp::office::sheet;
using jp::calc::Value;
using jp::grid::ValueStore;

namespace {

QString tr(const char *text) { return QCoreApplication::translate("jp::crunch::EditSession", text); }

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
class ChangeCells : public QUndoCommand {
public:
    ChangeCells(EditSession *session, std::vector<CellChange> changes, const QString &text) : QUndoCommand(text), m_session(session), m_changes(std::move(changes)) {}
    void redo() override { m_session->apply(&m_changes, true); }
    void undo() override { m_session->apply(&m_changes, false); }

private:
    EditSession *m_session;
    std::vector<CellChange> m_changes;
};

EditSession::EditSession(WorkbookLoader *loader, const std::shared_ptr<LoadedBook> &book, ValueStore *values, QObject *parent)
    : QObject(parent), m_loader(loader), m_book(book), m_values(values), m_locale(QLocale()), m_today(QDate::currentDate())
{
    m_undo.setUndoLimit(kUndoSteps);
    connect(&m_undo, &QUndoStack::cleanChanged, this, [this](bool clean) { cleanChanged(clean); });
}

EditSession::~EditSession() = default;

void EditSession::cleanChanged(bool clean)
{
    if (!clean) return;
    // the workbook is as the file has it: what the file lacks is nothing
    m_touched.clear();
    m_results.clear();
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
    if (!m_book || !m_book->book || sheet < 0 || sheet >= m_book->book->info().sheets.size()) return refuse(tr("There is no such sheet."));
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
    if (!m_book->calc->content(sheet, row, col).editable) return refuse(tr("You can't change part of an array or a data table."));
    return true;
}

QString EditSession::formatCodeOf(int sheet, quint32 row, quint32 col) const
{
    if (!m_book || !m_book->calc || !m_book->book) return QStringLiteral("General");
    const Calculation::CellContent c = m_book->calc->content(sheet, row, col);
    return m_book->book->styles().formatCode(c.style);
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
    if (c.isFormula) return QLatin1Char('=') + jp::grid::displayFormula(c.formula);
    const QString code = m_book->book->styles().formatCode(c.style);
    return editTextOfValue(c.value, code, entryOptions(sheet, code == QLatin1String("@")));
}

EditSession::Outcome EditSession::type(int sheet, quint32 row, quint32 col, const QString &typed)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || !canEditCell(sheet, row, col, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    const Calculation *calc = m_book->calc.get();
    const Calculation::CellContent before = calc->content(sheet, row, col);
    const QString code = m_book->book->styles().formatCode(before.style);
    const EntryResult r = readEntry(typed, entryOptions(sheet, code == QLatin1String("@")));
    if (!r.ok) {
        out.ok = false;
        out.problem = r.problem;
        out.problemAt = r.problemAt;
        return out;
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
    if (ch.after.value.isEmpty() && !ch.after.isFormula && ch.after.style == 0) ch.after.exists = false;
    if (sameMeaning(ch.before, ch.after) && ch.formatCode.isEmpty()) return out;   // (nothing to change)
    std::vector<CellChange> changes;
    changes.push_back(std::move(ch));
    m_undo.push(new ChangeCells(this, std::move(changes), tr("Typing in %1").arg(cellName(row, col))));
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
    bool tooMany = false, array = false;
    s.cells.forEachInRange(quint32(row1), quint32(col1), quint32(row2), quint32(col2), [&](const Cell &c) {
        const Calculation::CellContent before = calc->content(sheet, c.row, c.col);
        if (!before.editable) {
            array = true;
            return false;
        }
        if (!before.isFormula && before.value.isEmpty()) return true;   // (only a format: it stays)
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
    if (tooMany) {
        out.ok = false;
        out.problem = tr("That is more cells than JeffCrunch clears at once (%1). Select fewer cells.").arg(QLocale().toString(kMaxCellsPerEdit));
        return out;
    }
    if (changes.empty()) return out;
    const QString what = changes.size() == 1 ? tr("Clearing %1").arg(cellName(changes[0].row, changes[0].col))
                                             : QCoreApplication::translate("jp::crunch::EditSession", "Clearing %n cell(s)", nullptr, int(changes.size()));
    m_undo.push(new ChangeCells(this, std::move(changes), what));
    return out;
}

bool EditSession::apply(std::vector<CellChange> *changes, bool forward)
{
    std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
    if (!hold) return false;
    Calculation *calc = hold->calculation();
    Workbook &book = *m_book->book;
    std::map<int, std::vector<ValueStore::Item>> cells;
    std::vector<ValueStore::Item> formulas;
    for (CellChange &ch : *changes) {
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
        calc->put(ch.sheet, ch.row, ch.col, c, Calculation::Marking::Later);
        m_touched.insert(keyOf(ch.sheet, ch.row, ch.col));
        ValueStore::Item it;
        it.sheet = ch.sheet;
        it.row = ch.row;
        it.col = ch.col;
        cells[ch.sheet].push_back(it);
        if (c.isFormula) formulas.push_back(it);
    }
    hold->changed();
    hold.reset();   // (the worker has the model again, and works out what these changes make)
    m_tracking = true;
    // a formula put in a cell has no value until the worker gives it one: it shows nothing, not the value of what was there
    if (!formulas.empty()) m_values->apply(formulas);
    for (auto &entry : cells) Q_EMIT cellsChanged(entry.first, entry.second);
    return true;
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
    if (!m_book->book->package().isLazy()) return refuse(tr("This workbook was not opened from a file JeffCrunch can save over."));
    const Calculation *calc = m_book->calc.get();
    Workbook &book = *m_book->book;
    *out = Snapshot();
    SaveChanges &changes = out->changes;
    const bool hasShared = !book.sharedStringsPart().isEmpty();
    const bool hasStyles = !book.stylesPart().isEmpty();
    bool resultMissing = false;
    for (const quint64 key : m_touched) {
        const int sheet = int(key >> 40);
        const quint32 row = quint32((key >> 16) & 0xFFFFFF), col = quint32(key & 0xFFFF);
        const Calculation::CellContent c = calc->content(sheet, row, col);
        SavedCell cell;
        cell.row = row;
        cell.col = quint16(col);
        if (c.exists) {
            cell.style = c.style;
            cell.isFormula = c.isFormula;
            if (c.isFormula) {
                cell.formula = c.formula;
                Value v;
                if (m_values->find(sheet, row, col, &v)) {
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
    }
    for (const auto &[key, v] : m_results) {
        if (m_touched.count(key)) continue;
        const int sheet = int(key >> 40);
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
    out->strings = int(changes.newSharedStrings.size());
    if (hasStyles) {
        for (const Workbook::AddedNumberFormat &f : book.addedNumberFormats()) changes.newNumberFormats.append({f.id, f.code});
        for (const Workbook::AddedCellFormat &f : book.addedCellFormats()) changes.newCellFormats.append({f.base, f.numFmtId});
        changes.cellFormatsBefore = book.cellFormatsInFile();
    }
    out->numberFormats = int(changes.newNumberFormats.size());
    out->cellFormats = int(changes.newCellFormats.size());
    // The other program is asked to work every formula out again when this program could not: a formula it keeps apart that may read what changed, a result it did not get to, or a
    // formula it gave no result.
    changes.fullCalcOnLoad = (!m_touched.empty() || !m_results.empty()) && (calc->isDirty() || resultMissing || calc->keptCount() > 0);
    return true;
}

void EditSession::saved(const Snapshot &snapshot, const Package &package)
{
    m_book->book->savedTo(package, snapshot.strings, snapshot.numberFormats, snapshot.cellFormats);
    for (const auto &[key, v] : snapshot.results) m_fileResults[key] = v;
    for (const auto &[key, v] : snapshot.touchedResults) m_fileResults[key] = v;
    m_touched.clear();
    // what differs from the file is what differs from the file just written
    for (auto it = m_results.begin(); it != m_results.end();) {
        const auto file = m_fileResults.find(it->first);
        if (file != m_fileResults.end() && sameValue(file->second, it->second)) it = m_results.erase(it);
        else ++it;
    }
    m_undo.setClean();
}

} // namespace jp::crunch
