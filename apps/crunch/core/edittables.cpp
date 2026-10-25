// Tables in an open workbook (see sheet/tables.h): making one from cells, turning it back into cells, naming it, its marks and style, its row of names and row of totals, resizing it,
// and the ways it grows by itself (a cell typed under it or beside it; a formula typed in an empty column). Each is one step of Undo (or, for what follows a typed entry, a step of its own
// that Undo takes away first); a table is put in the workbook, replaced, or taken out whole by a step, so the steps name tables by their places and nothing else.
#include "core/editsession.h"

#include "sheet/cellref.h"
#include "sheet/structref.h"
#include "sheet/tablestyle.h"

#include <QCoreApplication>

#include <algorithm>

namespace jp::crunch {

using namespace jp::office;
using namespace jp::office::sheet;
using jp::calc::Value;

namespace {

constexpr int kMaxTablesInWorkbook = 20000;

bool inRange(const Table &t, quint32 row, quint32 col) { return row >= t.row1 && row <= t.row2 && col >= t.col1 && col <= t.col2; }

bool overlaps(const Table &t, const CellRange &r) { return !(t.row2 < r.row1 || t.row1 > r.row2 || t.col2 < r.col1 || t.col1 > r.col2); }

CellRange rangeOf(const Table &t)
{
    CellRange r;
    r.valid = true;
    r.row1 = t.row1;
    r.col1 = t.col1;
    r.row2 = t.row2;
    r.col2 = t.col2;
    return r;
}

QString freshColumnName(const QStringList &columns, int from) { return freshTableColumnName(columns, from); }

int nextColumnId(const Table &t)
{
    int id = 0;
    for (const TableColumnInfo &c : t.info) id = std::max(id, c.id);
    return id + 1;
}

} // namespace

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// asking

int EditSession::tableAt(int sheet, quint32 row, quint32 col) const
{
    if (!m_book || !m_book->book) return -1;
    const QVector<Table> &all = m_book->book->tables();
    for (int i = 0; i < all.size(); ++i)
        if (all[i].sheet == sheet && inRange(all[i], row, col)) return i;
    return -1;
}

QStringList EditSession::tableNames(int limit) const
{
    QStringList out;
    if (!m_book || !m_book->book) return out;
    for (const Table &t : m_book->book->tables()) {
        if (m_book->book->info().sheets[t.sheet].deleted) continue;
        out << t.name;
        if (out.size() >= limit) break;
    }
    return out;
}

QString EditSession::tableNameProblem(const QString &name, int except) const
{
    QString why;
    if (!validTableName(name, &why)) return why;
    const QVector<Table> &all = m_book->book->tables();
    for (int i = 0; i < all.size(); ++i)
        if (i != except && all[i].name.compare(name, Qt::CaseInsensitive) == 0 && !m_book->book->info().sheets[all[i].sheet].deleted)
            return tr("Another table has that name. Pick another.");
    for (int id = 0; id < m_nameTable.slots(); ++id)
        if (m_nameTable.at(id).alive && m_nameTable.at(id).name.compare(name, Qt::CaseInsensitive) == 0) return tr("A name in the workbook is called that. Pick another.");
    return {};
}

QString EditSession::calculatedFormulaFor(const Table &t, int column, quint32 row) const
{
    if (column < 0 || column >= t.info.size() || t.info[column].calcFormula.isEmpty()) return {};
    return shiftFormula(t.info[column].calcFormula, int(qint64(row) - qint64(t.dataFirst())), 0);
}

bool EditSession::cellsEmpty(int sheet, const CellRange &range) const
{
    bool empty = true;
    m_book->calc->sheet(sheet).cells.forEachInRange(range.row1, range.col1, range.row2, range.col2, [&](const Cell &c) {
        if (c.hasFormula() || c.type != CellType::Empty) {
            empty = false;
            return false;
        }
        return true;
    });
    return empty;
}

bool EditSession::tableFree(int sheet, const CellRange &range, int except, QString *why) const
{
    const QVector<Table> &all = m_book->book->tables();
    for (int i = 0; i < all.size(); ++i)
        if (i != except && all[i].sheet == sheet && overlaps(all[i], range)) {
            if (why) *why = tr("Those cells are in another table already.");
            return false;
        }
    for (const CellRange &pivot : m_book->book->pivotTablesOn(sheet))
        if (!(pivot.row2 < range.row1 || pivot.row1 > range.row2 || pivot.col2 < range.col1 || pivot.col1 > range.col2)) {
            if (why) *why = tr("Those cells are in a pivot table, so they can't be a table.");
            return false;
        }
    const AutoFilter &f = m_book->calc->autoFilterOf(sheet);
    if (f.ref.valid && !(f.ref.row2 < range.row1 || f.ref.row1 > range.row2 || f.ref.col2 < range.col1 || f.ref.col1 > range.col2)) {
        if (why) *why = tr("The sheet's filter covers some of those cells. Turn it off first, since a table has a filter of its own.");
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// changes of cells

CellChange EditSession::textCell(int sheet, quint32 row, quint32 col, const QString &text)
{
    CellChange ch;
    ch.sheet = sheet;
    ch.row = row;
    ch.col = quint16(col);
    ch.before = m_book->calc->content(sheet, row, col);
    ch.after = ch.before;
    ch.after.exists = true;
    ch.after.isFormula = false;
    ch.after.formula.clear();
    ch.after.value = Value::text(text);
    ch.after.sharedString = -1;
    ch.after.hasStored = false;
    ch.textToShare = !m_book->book->sharedStringsPart().isEmpty();
    return ch;
}

CellChange EditSession::formulaCell(int sheet, quint32 row, quint32 col, const QString &formula)
{
    CellChange ch;
    ch.sheet = sheet;
    ch.row = row;
    ch.col = quint16(col);
    ch.before = m_book->calc->content(sheet, row, col);
    ch.after = ch.before;
    ch.after.exists = true;
    ch.after.isFormula = true;
    ch.after.formula = formula;
    ch.after.value = Value();
    ch.after.sharedString = -1;
    ch.after.hasStored = false;
    return ch;
}

CellChange EditSession::clearedCell(int sheet, quint32 row, quint32 col)
{
    CellChange ch;
    ch.sheet = sheet;
    ch.row = row;
    ch.col = quint16(col);
    ch.before = m_book->calc->content(sheet, row, col);
    ch.after = ch.before;
    ch.after.isFormula = false;
    ch.after.formula.clear();
    ch.after.value = Value();
    ch.after.sharedString = -1;
    ch.after.hasStored = false;
    if (ch.after.style == 0) ch.after.exists = false;
    return ch;
}

EditSession::Outcome EditSession::pushTableStep(Step step, const QString &text)
{
    Outcome out;
    if (step.empty()) return out;
    if (qint64(step.cells.size()) > kMaxCellsPerEdit) {
        out.ok = false;
        out.problem = tr("That changes more cells than JeffCrunch changes at once (%1).").arg(QLocale().toString(kMaxCellsPerEdit));
        return out;
    }
    pushStep(std::move(step), text);
    return out;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// applying: the steps' tables

void EditSession::applyTableEdits(Step *step, bool forward, Calculation *calc, Workbook &book, std::set<int> *sheets)
{
    auto sheetNameOf = [&](int sheet) { return sheet >= 0 && sheet < book.info().sheets.size() ? book.info().sheets[sheet].name : QString(); };
    auto doOne = [&](TableEdit &e, bool fwd) {
        const Table &now = fwd ? e.after : e.before;
        const Table &was = fwd ? e.before : e.after;
        const bool erase = (e.kind == TableEdit::Kind::Insert && !fwd) || (e.kind == TableEdit::Kind::Erase && fwd);
        const bool insert = (e.kind == TableEdit::Kind::Insert && fwd) || (e.kind == TableEdit::Kind::Erase && !fwd);
        if (erase) {
            book.eraseTable(e.index);
            sheets->insert(was.sheet);
        } else if (insert) {
            book.insertTable(e.index, now);
            sheets->insert(now.sheet);
        } else {
            book.setTableAt(e.index, now);
            sheets->insert(now.sheet);
            sheets->insert(was.sheet);
        }
        if (e.rewrite == TableEdit::Rewrite::None) return;
        // the defined names that name the table follow it, and the file's other parts are told when it is saved
        if (fwd) {
            if (e.rewrite == TableEdit::Rewrite::Rename) {
                e.names = calc->rewriteNames([&](const QString &text) { return renameTableInFormula(text, e.before.name, e.after.name); });
                TableRewrite r;
                r.kind = TableRewrite::Kind::Rename;
                r.oldName = e.before.name;
                r.newName = e.after.name;
                r.seq = ++m_opSeq;
                m_tableRewrites.push_back(r);
            } else {
                const QString sheetName = sheetNameOf(e.before.sheet);
                e.names = calc->rewriteNames([&](const QString &text) { return structuredRefsToCells(text, e.before, sheetName, QStringLiteral("\x01"), -1, false); });
                TableRewrite r;
                r.kind = TableRewrite::Kind::ToRange;
                r.oldName = e.before.name;
                r.table = e.before;
                r.sheetName = sheetName;
                r.seq = ++m_opSeq;
                m_tableRewrites.push_back(r);
            }
        } else {
            calc->restoreNames(e.names);
            TableRewrite r;
            r.kind = e.rewrite == TableEdit::Rewrite::Rename ? TableRewrite::Kind::Rename : TableRewrite::Kind::ToRange;
            if (!m_tableRewrites.empty() && m_tableRewrites.back().kind == r.kind && m_tableRewrites.back().oldName == e.before.name && (r.kind != TableRewrite::Kind::Rename || m_tableRewrites.back().newName == e.after.name)) {
                m_tableRewrites.pop_back();
            } else if (e.rewrite == TableEdit::Rewrite::Rename) {
                // (undone against a file that was saved with the new name: the old name is written back)
                r.oldName = e.after.name;
                r.newName = e.before.name;
                r.seq = ++m_opSeq;
                m_tableRewrites.push_back(r);
            }
        }
    };
    if (forward) {
        for (TableEdit &e : step->tables) doOne(e, true);
    } else {
        for (auto it = step->tables.rbegin(); it != step->tables.rend(); ++it) doOne(*it, false);
    }
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// making

EditSession::Outcome EditSession::makeTable(int sheet, const CellRange &range, bool hasHeaders, const QString &style, int *made)
{
    Outcome out;
    auto refuse = [&](const QString &why) {
        out.ok = false;
        out.problem = why;
        return out;
    };
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) return refuse(why);
    Workbook &book = *m_book->book;
    if (!range.valid || range.col2 < range.col1 || range.row2 < range.row1) return refuse(tr("Pick the cells to make a table of."));
    if (book.tables().size() >= kMaxTablesInWorkbook) return refuse(tr("The workbook has as many tables as JeffCrunch works with."));
    if (qint64(range.col2) - range.col1 + 1 > 16384) return refuse(tr("A table can have up to 16,384 columns."));
    if (m_book->calc->sheet(sheet).isProtected) return refuse(tr("This sheet is protected, so a table can't be made on it."));
    CellRange r = range;
    // a table needs a row of data besides the row of names: the cells under a single row are used when they are empty
    if (hasHeaders && r.row1 == r.row2) {
        if (r.row2 + 1 >= kMaxRows) return refuse(tr("A table needs a row of data under its names."));
        r.row2 += 1;
    }
    if (!tableFree(sheet, r, -1, &why)) return refuse(why);
    for (const CellRange &m : m_book->calc->mergesOf(sheet))
        if (!(m.row2 < r.row1 || m.row1 > r.row2 || m.col2 < r.col1 || m.col1 > r.col2)) return refuse(tr("Merged cells can't be in a table. Unmerge them first."));
    bool arrayInside = false;
    m_book->calc->sheet(sheet).cells.forEachInRange(r.row1, r.col1, r.row2, r.col2, [&](const Cell &c) {
        if (c.flags & (ArrayMember | ArrayFormula | DataTable)) {
            arrayInside = true;
            return false;
        }
        return true;
    });
    if (arrayInside) return refuse(tr("An array formula or a data table is in those cells, so they can't be a table."));
    const quint32 columns = r.col2 - r.col1 + 1;
    // the names of the columns
    Table t;
    t.sheet = sheet;
    t.row1 = r.row1;
    t.col1 = r.col1;
    t.row2 = r.row2;
    t.col2 = r.col2;
    t.headerRows = hasHeaders ? 1 : 0;
    Step step;
    QStringList names;
    for (quint32 i = 0; i < columns; ++i) {
        QString name;
        if (hasHeaders) {
            const Calculation::CellContent c = m_book->calc->content(sheet, r.row1, r.col1 + i);
            QString shown;
            if (c.exists && (c.isFormula || !c.value.isEmpty())) shown = displayText(sheet, r.row1, r.col1 + i);
            name = shown.trimmed();
            if (name.size() > 255) name = name.left(255);
            if (name.isEmpty()) name = freshColumnName(names, int(i) + 1);
            // (two of a name: the second is numbered, as a name typed in the row would be)
            const QString unique = uniqueColumnName(name, names + QStringList(QString()), int(names.size()));
            name = unique.isEmpty() ? freshColumnName(names, int(i) + 1) : unique;
            const bool plainText = c.exists && !c.isFormula && c.value.isText() && c.value.text() == name;
            if (!plainText) {
                if (!c.editable) return refuse(tr("An array formula or a data table is in those cells, so they can't be a table."));
                step.cells.push_back(textCell(sheet, r.row1, r.col1 + i, name));
            }
        } else {
            name = freshColumnName(names, int(i) + 1);
        }
        names << name;
    }
    t.columns = names;
    t.info.resize(names.size());
    for (int i = 0; i < t.info.size(); ++i) t.info[i].id = i + 1;
    QStringList taken = tableNames(kMaxTablesInWorkbook);
    for (int id = 0; id < m_nameTable.slots(); ++id)
        if (m_nameTable.at(id).alive) taken << m_nameTable.at(id).name;
    t.name = freshTableName(taken);
    t.part = book.freshTablePart();
    t.id = book.freshTableId();
    t.styleName = style.isEmpty() ? QStringLiteral("TableStyleMedium2") : style;
    t.showRowStripes = true;
    t.filterButtons = hasHeaders;
    TableEdit e;
    e.kind = TableEdit::Kind::Insert;
    e.index = int(book.tables().size());
    e.after = t;
    step.tables.push_back(std::move(e));
    out = pushTableStep(std::move(step), tr("Making a table"));
    if (out.ok && made) *made = int(book.tables().size()) - 1;
    return out;
}

EditSession::Outcome EditSession::tableToRange(int table)
{
    Outcome out;
    QString why;
    if (!canEdit(&why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    Workbook &book = *m_book->book;
    if (table < 0 || table >= book.tables().size()) {
        out.ok = false;
        out.problem = tr("There is no such table.");
        return out;
    }
    const Table t = book.tables()[table];
    if (m_book->calc->sheet(t.sheet).isProtected) {
        out.ok = false;
        out.problem = lockedMessage();
        return out;
    }
    Step step;
    const QString tableSheet = book.info().sheets[t.sheet].name;
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
            const bool inside = s == t.sheet && inRange(t, r, c);
            const QString converted = structuredRefsToCells(text, t, tableSheet, book.info().sheets[s].name, qint64(r), inside);
            if (converted == text) return;
            if (qint64(step.cells.size()) >= kMaxCellsPerEdit) {
                tooMany = true;
                return;
            }
            CellChange f;
            f.sheet = s;
            f.row = r;
            f.col = quint16(c);
            f.before = calc->content(s, r, c);
            f.after = f.before;
            f.after.formula = converted;
            f.after.hasStored = false;
            step.cells.push_back(std::move(f));
        });
    }
    if (tooMany) {
        out.ok = false;
        out.problem = tr("More formulas than JeffCrunch changes at once (%1) use this table.").arg(QLocale().toString(kMaxCellsPerEdit));
        return out;
    }
    TableEdit e;
    e.kind = TableEdit::Kind::Erase;
    e.rewrite = TableEdit::Rewrite::ToRange;
    e.index = table;
    e.before = t;
    step.tables.push_back(std::move(e));
    // other tables' formulas for their columns that name this one
    const QVector<Table> &all = book.tables();
    for (int i = 0; i < all.size(); ++i) {
        if (i == table) continue;
        Table other = all[i];
        bool changed = false;
        for (TableColumnInfo &c : other.info) {
            const QString a = structuredRefsToCells(c.calcFormula, t, tableSheet, book.info().sheets[other.sheet].name, -1, false);
            const QString b = structuredRefsToCells(c.totalsFormula, t, tableSheet, book.info().sheets[other.sheet].name, -1, false);
            changed = changed || a != c.calcFormula || b != c.totalsFormula;
            c.calcFormula = a;
            c.totalsFormula = b;
        }
        if (!changed) continue;
        TableEdit o;
        o.kind = TableEdit::Kind::Replace;
        o.index = i > table ? i - 1 : i;   // (the table before it is taken out first)
        o.before = all[i];
        o.after = other;
        step.tables.push_back(std::move(o));
    }
    return pushTableStep(std::move(step), tr("Turning a table into cells"));
}

EditSession::Outcome EditSession::renameTable(int table, const QString &name)
{
    Outcome out;
    QString why;
    if (!canEdit(&why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    Workbook &book = *m_book->book;
    if (table < 0 || table >= book.tables().size()) {
        out.ok = false;
        out.problem = tr("There is no such table.");
        return out;
    }
    const QString typed = name.trimmed();
    const Table t = book.tables()[table];
    if (typed == t.name) return out;
    const QString problem = tableNameProblem(typed, table);
    if (!problem.isEmpty()) {
        out.ok = false;
        out.problem = problem;
        return out;
    }
    Step step;
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
            const QString renamed = renameTableInFormula(text, t.name, typed);
            if (renamed == text) return;
            if (qint64(step.cells.size()) >= kMaxCellsPerEdit) {
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
            f.after.hasStored = false;
            step.cells.push_back(std::move(f));
        });
    }
    if (tooMany) {
        out.ok = false;
        out.problem = tr("More formulas than JeffCrunch changes at once (%1) use this table's name.").arg(QLocale().toString(kMaxCellsPerEdit));
        return out;
    }
    // the table itself (and the formulas its columns hold), and the other tables' formulas that name it
    const QVector<Table> &all = book.tables();
    for (int i = 0; i < all.size(); ++i) {
        Table after = all[i];
        if (i == table) after.name = typed;
        for (TableColumnInfo &c : after.info) {
            c.calcFormula = renameTableInFormula(c.calcFormula, t.name, typed);
            c.totalsFormula = renameTableInFormula(c.totalsFormula, t.name, typed);
        }
        if (i != table && tableSignature(after) == tableSignature(all[i])) continue;
        TableEdit e;
        e.kind = TableEdit::Kind::Replace;
        e.index = i;
        e.before = all[i];
        e.after = after;
        if (i == table) e.rewrite = TableEdit::Rewrite::Rename;
        step.tables.push_back(std::move(e));
    }
    return pushTableStep(std::move(step), tr("Naming a table"));
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// the look, the row of names, the row of totals

EditSession::Outcome EditSession::setTableMarks(int table, const TableMarks &marks)
{
    Outcome out;
    QString why;
    if (!canEdit(&why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    Workbook &book = *m_book->book;
    if (table < 0 || table >= book.tables().size()) {
        out.ok = false;
        out.problem = tr("There is no such table.");
        return out;
    }
    Table after = book.tables()[table];
    after.styleName = marks.style;
    after.showFirstColumn = marks.firstColumn;
    after.showLastColumn = marks.lastColumn;
    after.showRowStripes = marks.rowStripes;
    after.showColumnStripes = marks.columnStripes;
    after.filterButtons = marks.filterButtons;
    if (tableSignature(after) == tableSignature(book.tables()[table])) return out;
    // a table's filter that is on is not left with no buttons: the rows it hides would stay hidden
    if (!after.filterButtons && tableFilter(table).active()) {
        out.ok = false;
        out.problem = tr("The table is filtered. Clear its filter first, so the rows it hides are shown, and then take the buttons off.");
        return out;
    }
    Step step;
    TableEdit e;
    e.index = table;
    e.before = book.tables()[table];
    e.after = after;
    step.tables.push_back(std::move(e));
    return pushTableStep(std::move(step), tr("Changing a table's look"));
}

EditSession::Outcome EditSession::setTableHeaderRow(int table, bool on)
{
    Outcome out;
    auto refuse = [&](const QString &why) {
        out.ok = false;
        out.problem = why;
        return out;
    };
    QString why;
    if (!canEdit(&why)) return refuse(why);
    Workbook &book = *m_book->book;
    if (table < 0 || table >= book.tables().size()) return refuse(tr("There is no such table."));
    const Table t = book.tables()[table];
    if ((t.headerRows > 0) == on) return out;
    if (m_book->calc->sheet(t.sheet).isProtected) return refuse(lockedMessage());
    Table after = t;
    Step step;
    if (on) {
        if (t.row1 == 0) return refuse(tr("There is no row above the table to hold the names of its columns."));
        CellRange above;
        above.valid = true;
        above.row1 = above.row2 = t.row1 - 1;
        above.col1 = t.col1;
        above.col2 = t.col2;
        if (!cellsEmpty(t.sheet, above)) return refuse(tr("The row above the table has something in it. Clear it, or make room, and try again."));
        if (!tableFree(t.sheet, above, table, &why)) return refuse(why);
        after.row1 = t.row1 - 1;
        after.headerRows = 1;
        after.filterButtons = true;
        for (int i = 0; i < t.columns.size(); ++i) step.cells.push_back(textCell(t.sheet, after.row1, t.col1 + quint32(i), t.columns[i]));
    } else {
        if (!t.hasData() || t.dataLast() - qint64(t.dataFirst()) < 0) return refuse(tr("A table needs a row of data."));
        for (int i = 0; i < t.columns.size(); ++i) step.cells.push_back(clearedCell(t.sheet, t.row1, t.col1 + quint32(i)));
        after.row1 = t.row1 + 1;
        after.headerRows = 0;
        after.filter.columns.clear();
    }
    TableEdit e;
    e.index = table;
    e.before = t;
    e.after = after;
    step.tables.push_back(std::move(e));
    return pushTableStep(std::move(step), on ? tr("Showing a table's names") : tr("Hiding a table's names"));
}

EditSession::Outcome EditSession::setTableTotalsRow(int table, bool on)
{
    Outcome out;
    auto refuse = [&](const QString &why) {
        out.ok = false;
        out.problem = why;
        return out;
    };
    QString why;
    if (!canEdit(&why)) return refuse(why);
    Workbook &book = *m_book->book;
    if (table < 0 || table >= book.tables().size()) return refuse(tr("There is no such table."));
    const Table t = book.tables()[table];
    if ((t.totalsRows > 0) == on) return out;
    if (m_book->calc->sheet(t.sheet).isProtected) return refuse(lockedMessage());
    Table after = t;
    Step step;
    if (on) {
        if (t.row2 + 1 >= kMaxRows) return refuse(tr("There is no row under the table."));
        CellRange below;
        below.valid = true;
        below.row1 = below.row2 = t.row2 + 1;
        below.col1 = t.col1;
        below.col2 = t.col2;
        if (!cellsEmpty(t.sheet, below)) return refuse(tr("The row under the table has something in it. Clear it, or make room, and try again."));
        if (!tableFree(t.sheet, below, table, &why)) return refuse(why);
        after.row2 = t.row2 + 1;
        after.totalsRows = 1;
        for (TableColumnInfo &c : after.info) {
            c.totalsFunction.clear();
            c.totalsLabel.clear();
            c.totalsFormula.clear();
        }
        const int last = int(after.columns.size()) - 1;
        if (last > 0) {
            after.info[0].totalsLabel = tr("Total");
            step.cells.push_back(textCell(t.sheet, after.row2, t.col1, after.info[0].totalsLabel));
        }
        // the last column adds up, or, if it holds no numbers, counts
        bool numbers = false;
        if (t.hasData())
            m_book->calc->sheet(t.sheet).cells.forEachInRange(t.dataFirst(), t.col2, quint32(t.dataLast()), t.col2, [&](const Cell &c) {
                if (c.type == CellType::Number || c.type == CellType::Date || (c.hasFormula() && valueAt(t.sheet, c.row, c.col).isNumber())) {
                    numbers = true;
                    return false;
                }
                return true;
            });
        after.info[last].totalsFunction = numbers ? QStringLiteral("sum") : QStringLiteral("count");
        step.cells.push_back(formulaCell(t.sheet, after.row2, t.col2, totalsFormulaOf(after, last)));
    } else {
        for (int i = 0; i < t.columns.size(); ++i) {
            const Calculation::CellContent c = m_book->calc->content(t.sheet, t.row2, t.col1 + quint32(i));
            if (c.exists && (c.isFormula || !c.value.isEmpty())) step.cells.push_back(clearedCell(t.sheet, t.row2, t.col1 + quint32(i)));
        }
        after.row2 = t.row2 - 1;
        after.totalsRows = 0;
    }
    TableEdit e;
    e.index = table;
    e.before = t;
    e.after = after;
    step.tables.push_back(std::move(e));
    return pushTableStep(std::move(step), on ? tr("Showing a table's totals") : tr("Hiding a table's totals"));
}

EditSession::Outcome EditSession::setTableTotal(int table, int column, const QString &function, const QString &custom)
{
    Outcome out;
    auto refuse = [&](const QString &why) {
        out.ok = false;
        out.problem = why;
        return out;
    };
    QString why;
    if (!canEdit(&why)) return refuse(why);
    Workbook &book = *m_book->book;
    if (table < 0 || table >= book.tables().size()) return refuse(tr("There is no such table."));
    const Table t = book.tables()[table];
    if (t.totalsRows == 0) return refuse(tr("The table has no row of totals."));
    if (column < 0 || column >= t.columns.size()) return refuse(tr("There is no such column."));
    if (m_book->calc->sheet(t.sheet).isProtected) return refuse(lockedMessage());
    Table after = t;
    TableColumnInfo &info = after.info[column];
    const quint32 row = t.row2, col = t.col1 + quint32(column);
    Step step;
    if (function.isEmpty() || function == QLatin1String("none")) {
        info.totalsFunction.clear();
        info.totalsFormula.clear();
        step.cells.push_back(info.totalsLabel.isEmpty() ? clearedCell(t.sheet, row, col) : textCell(t.sheet, row, col, info.totalsLabel));
    } else if (function == QLatin1String("custom")) {
        QString text = custom.trimmed();
        if (text.startsWith(QLatin1Char('='))) text = text.mid(1);
        if (text.isEmpty()) return refuse(tr("Type the formula for the total."));
        CellChange typed;
        bool none = false;
        if (!typedChange(t.sheet, row, col, QLatin1Char('=') + text, &typed, &none, &out)) return out;
        if (!typed.after.isFormula) return refuse(tr("That is not a formula."));
        info.totalsFunction = QStringLiteral("custom");
        info.totalsFormula = fileFormOfStructuredRefs(typed.after.formula, t.name);
        info.totalsLabel.clear();
        step.cells.push_back(formulaCell(t.sheet, row, col, info.totalsFormula));
    } else {
        const auto &known = totalsFunctions();
        if (std::none_of(known.begin(), known.end(), [&](const TotalsFunction &f) { return function == QLatin1String(f.word); })) return refuse(tr("JeffCrunch does not know that kind of total."));
        info.totalsFunction = function;
        info.totalsFormula.clear();
        info.totalsLabel.clear();
        step.cells.push_back(formulaCell(t.sheet, row, col, totalsFormulaOf(after, column)));
    }
    TableEdit e;
    e.index = table;
    e.before = t;
    e.after = after;
    step.tables.push_back(std::move(e));
    return pushTableStep(std::move(step), tr("Changing a table's total"));
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// resizing

EditSession::Outcome EditSession::resizeTable(int table, const CellRange &range)
{
    Outcome out;
    auto refuse = [&](const QString &why) {
        out.ok = false;
        out.problem = why;
        return out;
    };
    QString why;
    if (!canEdit(&why)) return refuse(why);
    Workbook &book = *m_book->book;
    if (table < 0 || table >= book.tables().size()) return refuse(tr("There is no such table."));
    const Table t = book.tables()[table];
    if (m_book->calc->sheet(t.sheet).isProtected) return refuse(lockedMessage());
    if (!range.valid || range.row1 != t.row1 || range.col1 != t.col1) return refuse(tr("The table keeps its first cell. Pick cells that begin where it does."));
    if (range.col2 < range.col1 || qint64(range.col2) - range.col1 + 1 > 16384 || range.row2 >= kMaxRows) return refuse(tr("The table can't be that size."));
    if (qint64(range.row2) - range.row1 + 1 < qint64(t.headerRows + t.totalsRows + 1)) return refuse(tr("A table needs a row of data besides its names and its totals."));
    if (range.row2 == t.row2 && range.col2 == t.col2) return out;
    if (!tableFree(t.sheet, range, table, &why)) return refuse(why);
    for (const CellRange &m : m_book->calc->mergesOf(t.sheet))
        if (!(m.row2 < range.row1 || m.row1 > range.row2 || m.col2 < range.col1 || m.col1 > range.col2)) return refuse(tr("Merged cells can't be in a table. Unmerge them first."));
    Table after = t;
    after.row2 = range.row2;
    after.col2 = range.col2;
    Step step;
    // columns taken off the right, or added to it
    const int newColumns = int(qint64(range.col2) - range.col1 + 1);
    if (newColumns < t.columns.size()) {
        after.columns = t.columns.mid(0, newColumns);
        after.info = t.info.mid(0, newColumns);
        QVector<FilterColumn> kept;
        for (const FilterColumn &c : after.filter.columns)
            if (int(c.column) < newColumns) kept.append(c);
        after.filter.columns = kept;
    }
    for (int i = int(t.columns.size()); i < newColumns; ++i) {
        const quint32 col = t.col1 + quint32(i);
        QString name;
        if (t.headerRows > 0) {
            const Calculation::CellContent c = m_book->calc->content(t.sheet, t.row1, col);
            if (c.exists && (c.isFormula || !c.value.isEmpty())) name = displayText(t.sheet, t.row1, col).trimmed();
        }
        if (name.isEmpty()) name = freshColumnName(after.columns, int(after.columns.size()) + 1);
        const QString unique = uniqueColumnName(name, after.columns + QStringList(QString()), int(after.columns.size()));
        name = unique.isEmpty() ? freshColumnName(after.columns, int(after.columns.size()) + 1) : unique;
        if (t.headerRows > 0) {
            const Calculation::CellContent c = m_book->calc->content(t.sheet, t.row1, col);
            if (!(c.exists && !c.isFormula && c.value.isText() && c.value.text() == name)) step.cells.push_back(textCell(t.sheet, t.row1, col, name));
        }
        after.columns << name;
        TableColumnInfo info;
        info.id = nextColumnId(after);
        after.info.append(info);
    }
    // the rows: a row of totals goes to the new last row; rows added to the data get the columns' formulas where they are empty
    const qint64 oldDataLast = t.dataLast();
    const qint64 newDataLast = qint64(after.row2) - after.totalsRows;
    if (t.totalsRows > 0 && after.row2 != t.row2) {
        const int common = int(std::min<qint64>(t.columns.size(), after.columns.size()));
        for (int i = 0; i < common; ++i) {
            const quint32 col = t.col1 + quint32(i);
            const Calculation::CellContent was = m_book->calc->content(t.sheet, t.row2, col);
            if (!was.exists || (!was.isFormula && was.value.isEmpty())) continue;
            // (the cell the totals go to: the old totals row is a row of data now when the table is longer, and is cleared)
            CellChange moved;
            moved.sheet = t.sheet;
            moved.row = after.row2;
            moved.col = quint16(col);
            moved.before = m_book->calc->content(t.sheet, after.row2, col);
            moved.after = was;
            moved.after.style = moved.before.exists ? moved.before.style : was.style;
            moved.textToShare = false;
            step.cells.push_back(std::move(moved));
            step.cells.push_back(clearedCell(t.sheet, t.row2, col));
        }
        // a column added under the totals has none
    }
    for (qint64 row = oldDataLast + 1; row <= newDataLast; ++row)
        for (int i = 0; i < after.columns.size(); ++i) {
            const quint32 r = quint32(row), col = t.col1 + quint32(i);
            const QString formula = calculatedFormulaFor(after, i, r);
            if (formula.isEmpty()) continue;
            // (a cell the old totals row held is cleared above, and is filled here)
            const Calculation::CellContent now = m_book->calc->content(t.sheet, r, col);
            const bool wasTotals = t.totalsRows > 0 && r == t.row2;
            if (now.exists && (now.isFormula || !now.value.isEmpty()) && !wasTotals) continue;
            CellChange f = formulaCell(t.sheet, r, col, formula);
            if (wasTotals) f.before = m_book->calc->content(t.sheet, r, col);
            step.cells.push_back(std::move(f));
        }
    // (a change of one cell twice in a step: the last stands, with what the cell was before the step)
    {
        std::map<quint64, size_t> at;
        std::vector<CellChange> merged;
        for (CellChange &c : step.cells) {
            const quint64 key = keyOf(c.sheet, c.row, c.col);
            const auto it = at.find(key);
            if (it == at.end()) {
                at[key] = merged.size();
                merged.push_back(std::move(c));
            } else {
                CellChange &first = merged[it->second];
                first.after = c.after;
                first.textToShare = c.textToShare;
            }
        }
        step.cells = std::move(merged);
    }
    TableEdit e;
    e.index = table;
    e.before = t;
    e.after = after;
    step.tables.push_back(std::move(e));
    return pushTableStep(std::move(step), tr("Resizing a table"));
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// growing by itself

EditSession::TableGrowth EditSession::growthByTyping(int sheet, quint32 row, quint32 col) const
{
    TableGrowth g;
    if (!m_book || !m_book->book) return g;
    const QVector<Table> &all = m_book->book->tables();
    for (int i = 0; i < all.size(); ++i) {
        const Table &t = all[i];
        if (t.sheet != sheet || t.totalsRows > 0) continue;
        if (row == t.row2 + 1 && col >= t.col1 && col <= t.col2 && t.row2 + 1 < kMaxRows) {
            g.table = i;
            g.row = true;
            return g;
        }
        if (col == t.col2 + 1 && row >= t.row1 && row <= t.row2 && t.col2 + 1 < kMaxColumns && t.col2 + 1 - t.col1 < 16384) {
            g.table = i;
            g.column = true;
            return g;
        }
    }
    return g;
}

void EditSession::growTableAfterEntry(int sheet, quint32 row, quint32 col, const CellChange &entry)
{
    // only a cell that now holds something grows a table
    if (!entry.after.exists || (!entry.after.isFormula && entry.after.value.isEmpty())) return;
    const TableGrowth g = growthByTyping(sheet, row, col);
    if (g.table < 0) return;
    Workbook &book = *m_book->book;
    const Table t = book.tables()[g.table];
    if (m_book->calc->sheet(sheet).isProtected) return;
    Table after = t;
    Step step;
    if (g.row) {
        after.row2 = t.row2 + 1;
        // the row must be free of another table, and the sheet's filter, and holds nothing the table's rule of columns would wipe: the cells already in it stay
        CellRange r = rangeOf(after);
        if (!tableFree(sheet, r, g.table, nullptr)) return;
        for (int i = 0; i < t.columns.size(); ++i) {
            const quint32 c = t.col1 + quint32(i);
            if (c == col) continue;
            const QString formula = calculatedFormulaFor(after, i, after.row2);
            if (formula.isEmpty()) continue;
            const Calculation::CellContent now = m_book->calc->content(sheet, after.row2, c);
            if (now.exists && (now.isFormula || !now.value.isEmpty())) continue;
            step.cells.push_back(formulaCell(sheet, after.row2, c, formula));
        }
    } else {
        const quint32 newCol = t.col2 + 1;
        after.col2 = newCol;
        if (!tableFree(sheet, rangeOf(after), g.table, nullptr)) return;
        QString name;
        if (t.headerRows > 0) {
            if (row == t.row1) {
                name = displayText(sheet, row, col).trimmed();
            } else {
                const Calculation::CellContent head = m_book->calc->content(sheet, t.row1, newCol);
                if (head.exists && (head.isFormula || !head.value.isEmpty())) name = displayText(sheet, t.row1, newCol).trimmed();
            }
        }
        if (name.size() > 255) name = name.left(255);
        if (name.isEmpty()) name = freshColumnName(t.columns, int(t.columns.size()) + 1);
        const QString unique = uniqueColumnName(name, t.columns + QStringList(QString()), int(t.columns.size()));
        name = unique.isEmpty() ? freshColumnName(t.columns, int(t.columns.size()) + 1) : unique;
        if (t.headerRows > 0) {
            const Calculation::CellContent head = m_book->calc->content(sheet, t.row1, newCol);
            const bool isName = head.exists && !head.isFormula && head.value.isText() && head.value.text() == name;
            if (!isName) step.cells.push_back(textCell(sheet, t.row1, newCol, name));
        }
        after.columns << name;
        TableColumnInfo info;
        info.id = nextColumnId(after);
        after.info.append(info);
    }
    TableEdit e;
    e.index = g.table;
    e.before = t;
    e.after = after;
    step.tables.push_back(std::move(e));
    pushStep(std::move(step), g.row ? tr("Growing a table by a row") : tr("Growing a table by a column"));
}

void EditSession::calculatedColumnAfterEntry(int sheet, quint32 row, quint32 col, const CellChange &entry)
{
    if (!entry.after.exists || !entry.after.isFormula) return;
    const int index = tableAt(sheet, row, col);
    if (index < 0) return;
    Workbook &book = *m_book->book;
    const Table t = book.tables()[index];
    if (!t.hasData() || qint64(row) < qint64(t.dataFirst()) || qint64(row) > t.dataLast() || m_book->calc->sheet(sheet).isProtected) return;
    const int column = int(col - t.col1);
    if (column < 0 || column >= t.info.size()) return;
    const int delta = int(qint64(t.dataFirst()) - qint64(row));
    const QString first = shiftFormula(entry.after.formula, delta, 0);   // the formula as the first row of data would have it
    if (first.contains(QLatin1String("#REF!"))) return;
    const QString was = t.info[column].calcFormula;
    if (first == was) return;
    const qint64 rows = t.dataLast() - qint64(t.dataFirst()) + 1;
    if (rows - 1 > kMaxCellsPerEdit) return;   // (too many rows to fill at once: the cell stays as it was typed)
    // the other cells of the column: empty (the formula is new), or the old formula (it is changed); anything else, and the cell is an exception that stands alone
    std::vector<CellChange> fills;
    bool consistent = true;
    for (qint64 r = qint64(t.dataFirst()); r <= t.dataLast() && consistent; ++r) {
        if (quint32(r) == row) continue;
        const Calculation::CellContent now = m_book->calc->content(sheet, quint32(r), col);
        const bool empty = !now.exists || (!now.isFormula && now.value.isEmpty());
        const bool old = !was.isEmpty() && now.isFormula && now.formula == shiftFormula(was, int(r - qint64(t.dataFirst())), 0);
        if (!empty && !old) {
            consistent = false;
            break;
        }
        fills.push_back(formulaCell(sheet, quint32(r), col, shiftFormula(first, int(r - qint64(t.dataFirst())), 0)));
    }
    if (!consistent) return;
    Table after = t;
    after.info[column].calcFormula = first;
    Step step;
    step.cells = std::move(fills);
    TableEdit e;
    e.index = index;
    e.before = t;
    e.after = after;
    step.tables.push_back(std::move(e));
    pushStep(std::move(step), tr("Filling a calculated column"));
    m_lastCalculated = true;
    m_calculatedIndex = m_undo.index();
}

// Rows inserted among a table's rows take the formulas of its calculated columns where their cells are empty; columns inserted among its columns are named in its row of names.
void EditSession::fillTableAfterInsert(int sheet, bool rows, quint32 first, quint32 count)
{
    Workbook &book = *m_book->book;
    const QVector<Table> &all = book.tables();
    for (int i = 0; i < all.size(); ++i) {
        const Table &t = all[i];
        if (t.sheet != sheet) continue;
        Step step;
        if (rows) {
            if (first <= t.row1 || first > t.row2 || qint64(count) * t.columns.size() > kMaxCellsPerEdit) continue;
            // (the table has grown: the new rows are data rows, but for one inserted at the row of totals, which is under them)
            for (quint32 r = first; r < first + count; ++r)
                for (int c = 0; c < t.columns.size(); ++c) {
                    const QString formula = calculatedFormulaFor(t, c, r);
                    if (formula.isEmpty()) continue;
                    const Calculation::CellContent now = m_book->calc->content(sheet, r, t.col1 + quint32(c));
                    if (now.exists && (now.isFormula || !now.value.isEmpty())) continue;
                    step.cells.push_back(formulaCell(sheet, r, t.col1 + quint32(c), formula));
                }
        } else {
            if (first <= t.col1 || first > t.col2 || t.headerRows == 0) continue;
            for (quint32 k = 0; k < count; ++k) {
                const quint32 col = first + k;
                const int at = int(col - t.col1);
                if (at < 0 || at >= t.columns.size()) continue;
                step.cells.push_back(textCell(sheet, t.row1, col, t.columns[at]));
            }
        }
        if (!step.cells.empty()) pushStep(std::move(step), rows ? tr("Filling a calculated column") : tr("Naming the new columns"));
    }
}

EditSession::Outcome EditSession::undoCalculatedColumn()
{
    Outcome out;
    if (!m_lastCalculated || m_undo.index() != m_calculatedIndex) {
        out.ok = false;
        out.problem = tr("There is nothing to undo.");
        return out;
    }
    m_undo.undo();
    return out;
}

} // namespace jp::crunch
