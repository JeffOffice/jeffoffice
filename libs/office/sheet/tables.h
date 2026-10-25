#pragma once
// Tables (xl/tables/tableN.xml): a range of cells whose first row holds the names of its columns. The names in the part and the texts of the header cells must be the same for the
// file to be valid, and formulas name a column as Table1[Column]. When a person types in a header cell the column is renamed here, in the formulas that name it, and in the part.
//
// What is read of a table is its name, where it is, and the names of its columns; the rest of the part is kept as it is, and written back with only the names changed.

#include "office/package.h"
#include "sheet/refshift.h"
#include "sheet/sheetmodel.h"

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QVector>

#include <map>
#include <memory>
#include <vector>

namespace jp::office::sheet {

struct WorkbookInfo;

// What a table says of one column besides its name.
struct TableColumnInfo {
    int id = 0;                     // the column's id in the part (unique in the table)
    int src = -1;                   // its place among the column elements of the part the table was read from; -1 for a column made here
    QString totalsFunction;         // what the totals row shows for it: empty for nothing, else the file's word (sum, min, max, average, count, countNums, stdDev, var, custom)
    QString totalsLabel;            // a text the totals row shows in the column instead
    QString totalsFormula;          // the formula of a "custom" total (no "=")
    QString calcFormula;            // the formula every cell of the column has (a calculated column), no "="; empty for none
    bool calcArray = false;
    bool operator==(const TableColumnInfo &o) const
    {
        return id == o.id && totalsFunction == o.totalsFunction && totalsLabel == o.totalsLabel && totalsFormula == o.totalsFormula && calcFormula == o.calcFormula && calcArray == o.calcArray;
    }
};

struct Table {
    QString part;                   // the table's part, as the package names it
    int sheet = -1;                 // the sheet it is on
    int id = 0;                     // the table's id in its part (unique in the workbook)
    QString name;                   // the name a formula calls it by (its displayName, else its name)
    quint32 row1 = 0, col1 = 0, row2 = 0, col2 = 0;   // the cells it covers, the header row first (when it has one)
    quint32 fileRow1 = 0, fileCol1 = 0, fileRow2 = 0, fileCol2 = 0;   // and as the file has them
    int headerRows = 1;             // 1, or 0 for a table with no header row
    int totalsRows = 0;             // 1 when the last row of the table is its row of totals, else 0 (the file's totalsRowCount, as it is now)
    int fileTotalsRows = 0;         // and as the file has it
    QStringList columns;            // the names of its columns as they are now
    QStringList fileColumns;        // and as the file has them
    QVector<TableColumnInfo> info;  // the rest of each column, as many as `columns`
    // Its look: the style's name, and the marks.
    QString styleName;
    bool showFirstColumn = false, showLastColumn = false, showRowStripes = true, showColumnStripes = false;
    // Whether the header row has its filter buttons, and the conditions of the filter (its `ref` is the table's, as it is written).
    bool filterButtons = true;
    AutoFilter filter;
    // What the file has, to tell whether the part must be written again: made when the table is read (empty for a table made here, which is always written).
    QByteArray fileSignature;
    // The first row of data, and the last (none when there is no data: first > last).
    quint32 dataFirst() const { return row1 + quint32(headerRows); }
    qint64 dataLast() const { return qint64(row2) - totalsRows; }
    bool hasData() const { return qint64(dataFirst()) <= dataLast(); }
    // The place in the table of a column of the sheet (-1 outside).
    int placeOf(quint32 col) const { return col >= col1 && col <= col2 ? int(col - col1) : -1; }
    int columnNamed(const QString &columnName) const
    {
        for (int i = 0; i < columns.size(); ++i)
            if (columns[i].compare(columnName, Qt::CaseInsensitive) == 0) return i;
        return -1;
    }
};

// Where a table is after rows, columns, or cells are inserted or deleted: Same, Moved (the whole table, or the data rows it has more or fewer of; `out` is where it is), or Refused (the edit
// would cut through the table's columns or remove its header row or all of it; `why` says so in a sentence). A table on another sheet is Same.
enum class TableFate : quint8 { Same, Moved, Refused };
TableFate tableAfter(const StructEdit &edit, const Table &table, CellRange *out, QString *why, int *totalsRows = nullptr);

// A table's place in the sheet, for putting it back.
struct TablePlace {
    std::shared_ptr<Table> whole;   // the table as it was, when the edit changed its columns (its range alone is put back otherwise)
    int table = 0;
    quint32 row1 = 0, col1 = 0, row2 = 0, col2 = 0;
    int totalsRows = 0;
};

// The tables of the workbook, in the order of the sheets and of each sheet's relationships. Tables whose part can't be read are left out (and a table with no columns).
QVector<Table> readTables(const Package &package, const WorkbookInfo &info);

// A formula with the column of a table renamed: Table1[Old], Table1[[#This Row],[Old]], Table1[[Old]:[Next]], and, for a formula inside the table (`insideTable`), [@Old] and
// [[#This Row],[Old]] with no table's name. Text in quotes and names of sheets are not touched, nor is a column of another table. A name that needs it is escaped (' before [ ] # and ')
// and a column that is named alone in single brackets and whose new name has a character that needs the double brackets is put in them.
QString renameColumnInFormula(const QString &formula, const QString &table, const QString &oldName, const QString &newName, bool insideTable);

// A table part with its columns named `names` (where they differ from `fileNames`, the names it has now), and the formulas it holds for its columns (calculatedColumnFormula and
// totalsRowFormula) following the renames; with a `range`, the cells it covers (its ref, and its filter's) are those; with `totalsRows` 0 a part that has a row of totals loses it (the count,
// and what its columns say about it). False if the part is not a table's.
bool renameTableXml(const QByteArray &xml, const QStringList &fileNames, const QStringList &names, const QString &table, QByteArray *out, const CellRange *range = nullptr, const int *totalsRows = nullptr);

// What a header cell's new text makes of the name of a column: the text as typed (no spaces at its ends), and one that another column of the table has (without regard to capitals)
// is given a number after it, as the programs people know do ("Total" is "Total2" when there is a "Total"). Empty if there is no name to be made (nothing typed, or past the 255 characters
// a column's name can have: `tooLong` says which).
QString uniqueColumnName(const QString &typed, const QStringList &columns, int column, bool *tooLong = nullptr);

// ---- the part of a table, read into the model and written from it

// What the part says beyond the name and cells (the style, the marks, the columns' totals and formulas, the filter), put into `table` (which has its name, range, and columns already). False if
// the part is not XML.
bool readTableModel(const QByteArray &xml, Table *table);
// The part for the table: the part it was read from (`original`, empty for a table made here) with the model put in it and everything else of it as it was. `ok` is false if the part given
// cannot be read as a table's.
QByteArray writeTablePart(const Table &table, const QByteArray &original, bool *ok = nullptr);
// The model as plain bytes, equal for equal tables: what is compared to tell whether a table changed from what the file has.
QByteArray tableSignature(const Table &table);
// A filter as the element the file has it (`<autoFilter ref=...>` with the conditions of its columns and its sort state), with the prefix of the element names; empty if it has no range.
QByteArray autoFilterElement(const AutoFilter &filter, const QByteArray &prefix);
// The filter an `autoFilter` element holds (from its bytes); false if it is not one.
bool parseAutoFilterElement(const QByteArray &xml, AutoFilter *out);

// A column's name as a reference writes it inside its brackets (a ' before [ ] # and '), the same read back, and whether the name has a mark that needs it in double brackets.
QString escapeTableColumn(const QString &name);
QString unescapeTableColumn(const QString &raw);
bool tableColumnNeedsDoubleBrackets(const QString &name);

// ---- names

constexpr int kMaxTableNameChars = 255;
// Whether a text can be the name of a table: letters, digits, underscores, periods and backslashes, beginning with a letter, an underscore, or a backslash, not a cell reference (A1, R1C1, R, C),
// and at most 255 characters. If not, `why` says in a sentence what is wrong.
bool validTableName(const QString &name, QString *why = nullptr);
// "Table1", "Table2", ... the first that none of `taken` has (without regard to capitals).
QString freshTableName(const QStringList &taken, const QString &stem = QString());

// "Column1", "Column2", ...: the first, counting from `from`, that none of `columns` has (without regard to capitals).
QString freshTableColumnName(const QStringList &columns, int from);

// The functions a totals row offers, in the order of the drop-down: the file's word, and the SUBTOTAL function number (101 to 111, the kind that leaves out hidden rows) it is worked out by.
struct TotalsFunction {
    const char *word;
    int number;
};
const std::vector<TotalsFunction> &totalsFunctions();
// The formula the cell of a total holds for a column: SUBTOTAL(109,Table1[Sales]) and the like (no "="); empty for none or for a label. The text a custom total holds is its own.
QString totalsFormulaOf(const Table &table, int column);

} // namespace jp::office::sheet
