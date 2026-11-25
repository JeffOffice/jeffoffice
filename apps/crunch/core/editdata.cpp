// Get Data in an open workbook: the rows read from a file are put on the sheet as a table that remembers where they came from, and read again by Refresh. Each is one step of Undo, and nothing
// is changed when it is not ok.
#include "core/editsession.h"

#include "sheet/datasource.h"

#include <QCoreApplication>
#include <QSet>

namespace jp::crunch {

using namespace jp::office;
using namespace jp::office::sheet;
using jp::calc::Value;

namespace {

constexpr int kMaxDataTablesInWorkbook = 20000;

QStringList uniqueNames(const QStringList &header)
{
    QStringList out;
    QSet<QString> used;
    for (int i = 0; i < header.size(); ++i) {
        QString name = header[i].trimmed().left(255);
        if (name.isEmpty()) name = QStringLiteral("Column%1").arg(i + 1);
        QString candidate = name;
        for (int n = 2; used.contains(candidate.toLower()); ++n) candidate = name.left(250) + QString::number(n);
        used.insert(candidate.toLower());
        out << candidate;
    }
    return out;
}

}   // namespace

QStringList EditSession::dataTableNames(const QStringList &header) const { return uniqueNames(header); }

bool EditSession::cellsHoldData(int sheet, const CellRange &range) const { return !cellsEmpty(sheet, range); }

// One cell of loaded data: text stays text (a value that begins with = is never a formula), a number, true or false, or a date is put in as that.
CellChange EditSession::dataCell(int sheet, quint32 row, quint32 col, const DataCell &cell, char kind)
{
    if (cell.kind == 0 && cell.text.isEmpty()) {
        const Calculation::CellContent before = m_book->calc->content(sheet, row, col);
        return before.exists ? clearedCell(sheet, row, col) : textCell(sheet, row, col, QString());
    }
    if (kind == 'n') {
        bool ok = false;
        const double d = cell.text.toDouble(&ok);
        if (ok) {
            CellChange ch = textCell(sheet, row, col, QString());
            ch.after.value = Value::number(d);
            ch.textToShare = false;
            return ch;
        }
    } else if (kind == 'b') {
        CellChange ch = textCell(sheet, row, col, QString());
        ch.after.value = Value::boolean(cell.text.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0);
        ch.textToShare = false;
        return ch;
    } else if (kind == 'd') {
        CellChange typed;
        bool none = false;
        Outcome problem;
        if (typedChange(sheet, row, col, cell.text, &typed, &none, &problem) && !none && !typed.after.isFormula && typed.after.value.isNumber()) return typed;
    }
    return textCell(sheet, row, col, cell.text);
}

EditSession::Outcome EditSession::placeDataTable(int sheet, quint32 row, quint32 col, const DataRows &data, const QVector<char> &kinds, const QString &source, bool replace, int *made)
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
    if (data.header.isEmpty() || data.rows.isEmpty()) return refuse(tr("There are no rows to load."));
    if (m_book->calc->sheet(sheet).isProtected) return refuse(tr("This sheet is protected, so a table can't be put on it."));
    if (book.tables().size() >= kMaxDataTablesInWorkbook) return refuse(tr("The workbook has as many tables as JeffCrunch works with."));
    const qint64 columns = data.header.size(), rows = qint64(data.rows.size()) + 1;
    if (qint64(col) + columns > 16384 || qint64(row) + rows > 1048576) return refuse(tr("There is no room for the table there: it needs %1 rows and %2 columns from the chosen cell.").arg(rows).arg(columns));
    CellRange r;
    r.valid = true;
    r.row1 = row;
    r.col1 = col;
    r.row2 = quint32(row + rows - 1);
    r.col2 = quint32(col + columns - 1);
    if (!tableFree(sheet, r, -1, &why)) return refuse(why);
    for (const CellRange &m : m_book->calc->mergesOf(sheet))
        if (!(m.row2 < r.row1 || m.row1 > r.row2 || m.col2 < r.col1 || m.col1 > r.col2)) return refuse(tr("Merged cells can't be in a table. Unmerge them first."));
    bool arrayInside = false;
    m_book->calc->sheet(sheet).cells.forEachInRange(r.row1, r.col1, r.row2, r.col2, [&](quint32, quint32, const Cell &c) {
        if (c.flags & (ArrayMember | ArrayFormula | DataTable)) {
            arrayInside = true;
            return false;
        }
        return true;
    });
    if (arrayInside) return refuse(tr("An array formula or a data table is in those cells, so they can't be a table."));
    if (!replace && !cellsEmpty(sheet, r)) return refuse(tr("Those cells are not empty."));
    const QStringList names = uniqueNames(data.header);
    Step step;
    for (qint64 c = 0; c < columns; ++c) step.cells.push_back(textCell(sheet, row, quint32(col + c), names[int(c)]));
    for (int i = 0; i < data.rows.size(); ++i)
        for (qint64 c = 0; c < columns; ++c) step.cells.push_back(dataCell(sheet, quint32(row + 1 + i), quint32(col + c), data.rows[i].value(int(c)), kinds.value(int(c), 't')));
    Table t;
    t.sheet = sheet;
    t.row1 = r.row1;
    t.col1 = r.col1;
    t.row2 = r.row2;
    t.col2 = r.col2;
    t.headerRows = 1;
    t.columns = names;
    t.info.resize(names.size());
    for (int i = 0; i < t.info.size(); ++i) t.info[i].id = i + 1;
    QStringList taken = tableNames(kMaxDataTablesInWorkbook);
    for (int id = 0; id < m_nameTable.slots(); ++id)
        if (m_nameTable.at(id).alive) taken << m_nameTable.at(id).name;
    t.name = freshTableName(taken);
    t.part = book.freshTablePart();
    t.id = book.freshTableId();
    t.styleName = QStringLiteral("TableStyleMedium2");
    t.showRowStripes = true;
    t.filterButtons = true;
    t.dataSource = source;
    TableEdit e;
    e.kind = TableEdit::Kind::Insert;
    e.index = int(book.tables().size());
    e.after = t;
    step.tables.push_back(std::move(e));
    out = pushTableStep(std::move(step), tr("Getting data"));
    if (out.ok && made) *made = int(book.tables().size()) - 1;
    return out;
}

EditSession::Outcome EditSession::refreshDataTable(int table, const DataRows &data, const QVector<char> &kinds)
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
    if (table < 0 || table >= book.tables().size()) return refuse(tr("That table is not there."));
    const Table was = book.tables()[table];
    if (!canEditSheet(was.sheet, &why)) return refuse(why);
    if (m_book->calc->sheet(was.sheet).isProtected) return refuse(tr("This sheet is protected, so the table can't be changed."));
    if (was.dataSource.isEmpty()) return refuse(tr("This table was not made by Get Data, so it has no file to read again."));
    if (was.headerRows != 1 || was.totalsRows != 0) return refuse(tr("The table has a row of totals or no row of names. Take the row of totals off first, and refresh again."));
    const QStringList names = uniqueNames(data.header);
    if (names.size() != was.columns.size()) return refuse(tr("The columns in the file are not the table's (the file has %1 and the table has %2), so nothing was changed.").arg(names.size()).arg(was.columns.size()));
    for (int i = 0; i < names.size(); ++i)
        if (names[i].compare(was.columns[i], Qt::CaseInsensitive) != 0) return refuse(tr("The columns in the file are not the table's, so nothing was changed. The first one that differs is \"%1\".").arg(names[i]));
    if (data.rows.isEmpty()) return refuse(tr("The file now has no rows, so nothing was changed."));
    const qint64 newLast = qint64(was.row1) + data.rows.size();
    if (newLast >= 1048576) return refuse(tr("The rows would go past the end of the sheet, so nothing was changed."));
    Table now = was;
    now.row2 = quint32(newLast);
    now.fileRow2 = now.row2;
    CellRange grown;
    grown.valid = true;
    grown.row1 = was.row2 + 1;
    grown.row2 = now.row2;
    grown.col1 = was.col1;
    grown.col2 = was.col2;
    if (now.row2 > was.row2) {
        if (!cellsEmpty(was.sheet, grown) || !tableFree(was.sheet, [&] { CellRange all = grown; all.row1 = was.row1; return all; }(), table, nullptr)) return refuse(tr("There is no room under the table for the new rows, so nothing was changed."));
        for (const CellRange &m : m_book->calc->mergesOf(was.sheet))
            if (!(m.row2 < grown.row1 || m.row1 > grown.row2 || m.col2 < grown.col1 || m.col1 > grown.col2)) return refuse(tr("Merged cells are under the table, so nothing was changed."));
    }
    Step step;
    for (int i = 0; i < data.rows.size(); ++i)
        for (int c = 0; c < names.size(); ++c) step.cells.push_back(dataCell(was.sheet, quint32(was.row1 + 1 + i), quint32(was.col1 + c), data.rows[i].value(c), kinds.value(c, 't')));
    // (rows the table had and the file has no more of)
    for (quint32 rr = now.row2 + 1; rr <= was.row2; ++rr)
        for (quint32 cc = was.col1; cc <= was.col2; ++cc) {
            const Calculation::CellContent before = m_book->calc->content(was.sheet, rr, cc);
            if (before.exists) step.cells.push_back(clearedCell(was.sheet, rr, cc));
        }
    TableEdit e;
    e.kind = TableEdit::Kind::Replace;
    e.index = table;
    e.before = was;
    e.after = now;
    step.tables.push_back(std::move(e));
    out = pushTableStep(std::move(step), tr("Refreshing data"));
    return out;
}

// Takes the link to a file off every table that has one (Inspect): the tables and their cells stay. One step of Undo.
EditSession::Outcome EditSession::removeDataLinks(int *removed)
{
    Outcome out;
    QString why;
    if (!canEdit(&why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    Step step;
    const QVector<Table> &all = m_book->book->tables();
    for (int i = 0; i < all.size(); ++i) {
        if (all[i].dataSource.isEmpty()) continue;
        TableEdit e;
        e.kind = TableEdit::Kind::Replace;
        e.index = i;
        e.before = all[i];
        e.after = all[i];
        e.after.dataSource.clear();
        step.tables.push_back(std::move(e));
    }
    if (removed) *removed = int(step.tables.size());
    return pushTableStep(std::move(step), tr("Removing links to files"));
}

int EditSession::dataLinkCount() const
{
    int n = 0;
    for (const Table &t : m_book->book->tables())
        if (!t.dataSource.isEmpty()) ++n;
    return n;
}

}   // namespace jp::crunch
