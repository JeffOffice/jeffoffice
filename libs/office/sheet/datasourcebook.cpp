#include "sheet/datasourcebook.h"

#include "calc/dates.h"
#include "sheet/cellref.h"
#include "sheet/sheetreader.h"

#include <QCoreApplication>
#include <QFileInfo>

#include <algorithm>

namespace jp::office::sheet {

namespace {

QString numberText(double d)
{
    if (d == double(qint64(d)) && std::abs(d) < 1e15) return QString::number(qint64(d));
    return QString::number(d, 'g', 15);
}

bool openBook(const QString &path, const DataLimits &limits, Workbook *book, QString *why)
{
    auto fail = [&](const QString &w) {
        if (why) *why = w;
        return false;
    };
    if (dataPathIsRemote(path)) return fail(QCoreApplication::translate("jp::office::sheet::DataSource", "Only a file on this computer can be loaded, not a web address or a file on another computer."));
    const QFileInfo info(path);
    if (!info.exists()) return fail(QCoreApplication::translate("jp::office::sheet::DataSource", "The file is not there."));
    if (!info.isFile()) return fail(QCoreApplication::translate("jp::office::sheet::DataSource", "That is not a file."));
    if (info.size() > limits.maxBytes) return fail(QCoreApplication::translate("jp::office::sheet::DataSource", "The file is larger than %1 MB, which is more than this program reads as data.").arg(limits.maxBytes >> 20));
    QString error;
    if (!book->openFile(path, &error)) return fail(error.isEmpty() ? QCoreApplication::translate("jp::office::sheet::DataSource", "The workbook can't be opened.") : error);
    return true;
}

// "Sheet1!$A$1:$C$9" or "'My Sheet'!A1:B2": the sheet and the cells; false for anything else (a formula, a name of names).
bool splitReference(const QString &ref, QString *sheet, CellRange *range)
{
    const int bang = int(ref.lastIndexOf(QLatin1Char('!')));
    if (bang <= 0) return false;
    QString name = ref.left(bang);
    if (name.startsWith(QLatin1Char('\'')) && name.endsWith(QLatin1Char('\'')) && name.size() >= 2) name = name.mid(1, name.size() - 2).replace(QLatin1String("''"), QLatin1String("'"));
    QString cells = ref.mid(bang + 1);
    cells.remove(QLatin1Char('$'));
    CellRange r;
    if (!parseRangeRef(cells, &r) || !r.valid) {
        quint32 row = 0, col = 0;
        if (!parseCellRef(cells, &row, &col)) return false;
        r.valid = true;
        r.row1 = r.row2 = row;
        r.col1 = r.col2 = col;
    }
    *sheet = name;
    *range = r;
    return true;
}

int sheetPlace(const Workbook &book, const QString &name)
{
    const WorkbookInfo &info = book.info();
    for (int i = 0; i < info.sheets.size(); ++i)
        if (!info.sheets[i].deleted && info.sheets[i].kind == SheetKind::Worksheet && info.sheets[i].name.compare(name, Qt::CaseInsensitive) == 0) return i;
    return -1;
}

DataCell cellOf(const Workbook &book, const Sheet &sheet, const Cell &c)
{
    switch (c.type) {
    case CellType::Empty: return DataCell();
    case CellType::Number: return DataCell{numberText(c.number), 'n'};
    case CellType::SharedString:
    case CellType::InlineString:
    case CellType::FormulaString: return DataCell{sheet.text(c, book.sharedStrings()), 's'};
    case CellType::Bool: return DataCell{c.number != 0 ? QStringLiteral("TRUE") : QStringLiteral("FALSE"), 'b'};
    case CellType::Error: return DataCell{errorText(CellError(int(c.number))), 's'};
    case CellType::Date: {
        int y = 0, m = 0, d = 0;
        const double whole = std::floor(c.number);
        if (!jp::calc::serialToDate(whole, book.info().dateSystem, &y, &m, &d)) return DataCell{numberText(c.number), 'n'};
        QString text = QStringLiteral("%1-%2-%3").arg(y, 4, 10, QLatin1Char('0')).arg(m, 2, 10, QLatin1Char('0')).arg(d, 2, 10, QLatin1Char('0'));
        // (a time of day: the date alone would lose it, so it is text)
        if (c.number != whole) {
            const int secs = int(std::lround((c.number - whole) * 86400.0));
            text += QStringLiteral(" %1:%2:%3").arg(secs / 3600 % 24, 2, 10, QLatin1Char('0')).arg(secs / 60 % 60, 2, 10, QLatin1Char('0')).arg(secs % 60, 2, 10, QLatin1Char('0'));
        }
        return DataCell{text, 's'};
    }
    }
    return DataCell();
}

}   // namespace

QStringList dataBookLists(const QString &path, const DataLimits &limits, const std::function<bool()> &cancelled, QString *why)
{
    Workbook book;
    if (!openBook(path, limits, &book, why)) return {};
    if (cancelled && cancelled()) {
        if (why) *why = QCoreApplication::translate("jp::office::sheet::DataSource", "The read was stopped.");
        return {};
    }
    QStringList out;
    const WorkbookInfo &info = book.info();
    for (int place : info.order) {
        if (place < 0 || place >= info.sheets.size() || info.sheets[place].deleted || info.sheets[place].kind != SheetKind::Worksheet) continue;
        if (out.size() < limits.maxLists) out << QStringLiteral("sheet:") + info.sheets[place].name;
    }
    for (const Table &t : book.tables())
        if (out.size() < limits.maxLists && !t.name.isEmpty()) out << QStringLiteral("table:") + t.name;
    for (const DefinedName &n : info.definedNames) {
        QString sheet;
        CellRange r;
        if (out.size() < limits.maxLists && !n.hidden && n.localSheet < 0 && splitReference(n.ref, &sheet, &r)) out << QStringLiteral("name:") + n.name;
    }
    if (out.isEmpty() && why) *why = QCoreApplication::translate("jp::office::sheet::DataSource", "The workbook has no sheet, table, or named range with cells to load.");
    return out;
}

bool dataReadBook(const QString &path, const QString &list, int maxRows, const DataLimits &limits, const std::function<bool()> &cancelled, DataRows *out, QString *why)
{
    auto fail = [&](const QString &w) {
        if (why) *why = w;
        return false;
    };
    Workbook book;
    if (!openBook(path, limits, &book, why)) return false;
    const int colon = int(list.indexOf(QLatin1Char(':')));
    const QString what = colon < 0 ? QString() : list.left(colon), name = colon < 0 ? QString() : list.mid(colon + 1);
    int place = -1;
    CellRange range;
    bool headerRow = true;
    if (what == QLatin1String("sheet")) {
        place = sheetPlace(book, name);
        if (place < 0) return fail(QCoreApplication::translate("jp::office::sheet::DataSource", "That sheet is not in the workbook."));
    } else if (what == QLatin1String("table")) {
        for (const Table &t : book.tables())
            if (t.name.compare(name, Qt::CaseInsensitive) == 0) {
                place = t.sheet;
                range.valid = true;
                range.row1 = t.row1;
                range.col1 = t.col1;
                range.row2 = quint32(t.dataLast() >= qint64(t.row1) ? t.dataLast() : t.row2);
                range.col2 = t.col2;
                headerRow = t.headerRows > 0;
                break;
            }
        if (place < 0) return fail(QCoreApplication::translate("jp::office::sheet::DataSource", "That table is not in the workbook."));
    } else if (what == QLatin1String("name")) {
        for (const DefinedName &n : book.info().definedNames) {
            QString sheet;
            if (n.localSheet < 0 && n.name.compare(name, Qt::CaseInsensitive) == 0 && splitReference(n.ref, &sheet, &range)) {
                place = sheetPlace(book, sheet);
                break;
            }
        }
        if (place < 0) return fail(QCoreApplication::translate("jp::office::sheet::DataSource", "That named range is not in the workbook, or is not a block of cells."));
    } else {
        return fail(QCoreApplication::translate("jp::office::sheet::DataSource", "That is not a sheet, table, or named range."));
    }
    if (place < 0 || place >= book.info().sheets.size()) return fail(QCoreApplication::translate("jp::office::sheet::DataSource", "That sheet is not in the workbook."));
    const int cap = maxRows > 0 ? std::min(maxRows, limits.maxRows) : limits.maxRows;
    Sheet sheet;
    // (a preview reads only the rows it needs)
    const qint64 stopAfter = range.valid && maxRows > 0 ? qint64(range.row1) + cap + 1 : (maxRows > 0 ? qint64(cap) + 1 : -1);
    const bool whole = book.readSheet(place, &sheet, [&](qint64) { return !(cancelled && cancelled()); }, stopAfter);
    if (cancelled && cancelled()) return fail(QCoreApplication::translate("jp::office::sheet::DataSource", "The read was stopped."));
    if (!whole && !sheet.partial && !sheet.error.isEmpty()) return fail(sheet.error);
    if (!range.valid) {
        if (sheet.extentRows == 0 || sheet.extentCols == 0) return fail(QCoreApplication::translate("jp::office::sheet::DataSource", "The sheet is empty."));
        range.valid = true;
        range.row1 = 0;
        range.col1 = 0;
        range.row2 = sheet.extentRows - 1;
        range.col2 = sheet.extentCols - 1;
    }
    // (the size comes from the cells that are there, never from the area asked for: one cell at the far corner is one cell)
    bool columnsCut = false;
    {
        quint32 lastRow = range.row1, lastCol = range.col1;
        const quint32 colLimit = quint32(std::min<qint64>(qint64(range.col1) + limits.maxCols - 1, qint64(range.col2)));
        sheet.cells.forEachInRange(range.row1, range.col1, range.row2, range.col2, [&](quint32 row, quint32 col, const Cell &c) {
            const DataCell cell = cellOf(book, sheet, c);
            if (cell.kind == 0 && cell.text.isEmpty()) return true;
            if (col > colLimit) {
                columnsCut = true;
                return true;
            }
            lastRow = std::max(lastRow, row);
            lastCol = std::max(lastCol, col);
            return true;
        });
        range.row2 = lastRow;
        range.col2 = lastCol;
    }
    const quint32 width = range.col2 - range.col1 + 1;
    // the cells of the rows wanted: the header, then the data
    const quint32 firstData = headerRow ? range.row1 + 1 : range.row1;
    const qint64 lastWanted = std::min<qint64>(qint64(range.row2), qint64(firstData) + cap - 1);
    *out = DataRows();
    out->header.resize(int(width));
    QVector<QVector<DataCell>> rows;
    if (range.row2 >= firstData) rows.resize(int(std::max<qint64>(0, lastWanted - qint64(firstData) + 1)));
    for (QVector<DataCell> &r : rows) r.resize(int(width));
    sheet.cells.forEachInRange(range.row1, range.col1, quint32(std::min<qint64>(lastWanted, range.row2)), range.col2, [&](quint32 row, quint32 col, const Cell &c) {
        const DataCell cell = cellOf(book, sheet, c);
        if (headerRow && row == range.row1) out->header[int(col - range.col1)] = cell.text;
        else if (row >= firstData && row - firstData < quint32(rows.size())) rows[int(row - firstData)][int(col - range.col1)] = cell;
        return true;
    });
    if (!headerRow) {
        for (int c = 0; c < int(width); ++c) out->header[c] = QStringLiteral("Column%1").arg(c + 1);
    } else {
        for (int c = 0; c < int(width); ++c)
            if (out->header[c].isEmpty()) out->header[c] = QStringLiteral("Column%1").arg(c + 1);
    }
    // (rows with nothing at the end of a sheet's used area are not rows)
    while (!rows.isEmpty()) {
        bool any = false;
        for (const DataCell &c : rows.last()) any = any || c.kind != 0 || !c.text.isEmpty();
        if (any) break;
        rows.removeLast();
    }
    out->rows = rows;
    if (columnsCut) out->notes << QCoreApplication::translate("jp::office::sheet::DataSource", "Only the first %1 columns are loaded; the rest are left out.").arg(limits.maxCols);
    if (maxRows <= 0 && qint64(range.row2) > lastWanted) out->notes << QCoreApplication::translate("jp::office::sheet::DataSource", "Only the first %1 rows are loaded; the rest are left out.").arg(cap);
    if (out->rows.isEmpty()) return fail(QCoreApplication::translate("jp::office::sheet::DataSource", "That holds no rows under its names."));
    return true;
}

}   // namespace jp::office::sheet
