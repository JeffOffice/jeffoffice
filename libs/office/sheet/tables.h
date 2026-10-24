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

namespace jp::office::sheet {

struct WorkbookInfo;

struct Table {
    QString part;                   // the table's part, as the package names it
    int sheet = -1;                 // the sheet it is on
    QString name;                   // the name a formula calls it by (its displayName, else its name)
    quint32 row1 = 0, col1 = 0, row2 = 0, col2 = 0;   // the cells it covers, the header row first (when it has one)
    quint32 fileRow1 = 0, fileCol1 = 0, fileRow2 = 0, fileCol2 = 0;   // and as the file has them
    int headerRows = 1;             // 1, or 0 for a table with no header row
    QStringList columns;            // the names of its columns as they are now
    QStringList fileColumns;        // and as the file has them
};

// Where a table is after rows, columns, or cells are inserted or deleted: Same, Moved (the whole table, or the data rows it has more or fewer of; `out` is where it is), or Refused (the edit
// would cut through the table's columns or remove its header row or all of it; `why` says so in a sentence). A table on another sheet is Same.
enum class TableFate : quint8 { Same, Moved, Refused };
TableFate tableAfter(const StructEdit &edit, const Table &table, CellRange *out, QString *why);

// A table's place in the sheet, for putting it back.
struct TablePlace {
    int table = 0;
    quint32 row1 = 0, col1 = 0, row2 = 0, col2 = 0;
};

// The tables of the workbook, in the order of the sheets and of each sheet's relationships. Tables whose part can't be read are left out (and a table with no columns).
QVector<Table> readTables(const Package &package, const WorkbookInfo &info);

// A formula with the column of a table renamed: Table1[Old], Table1[[#This Row],[Old]], Table1[[Old]:[Next]], and, for a formula inside the table (`insideTable`), [@Old] and
// [[#This Row],[Old]] with no table's name. Text in quotes and names of sheets are not touched, nor is a column of another table. A name that needs it is escaped (' before [ ] # and ')
// and a column that is named alone in single brackets and whose new name has a character that needs the double brackets is put in them.
QString renameColumnInFormula(const QString &formula, const QString &table, const QString &oldName, const QString &newName, bool insideTable);

// A table part with its columns named `names` (where they differ from `fileNames`, the names it has now), and the formulas it holds for its columns (calculatedColumnFormula and
// totalsRowFormula) following the renames; with a `range`, the cells it covers (its ref, and its filter's) are those. False if the part is not a table's.
bool renameTableXml(const QByteArray &xml, const QStringList &fileNames, const QStringList &names, const QString &table, QByteArray *out, const CellRange *range = nullptr);

// What a header cell's new text makes of the name of a column: the text as typed (no spaces at its ends), and one that another column of the table has (without regard to capitals)
// is given a number after it, as the programs people know do ("Total" is "Total2" when there is a "Total"). Empty if there is no name to be made (nothing typed, or past the 255 characters
// a column's name can have: `tooLong` says which).
QString uniqueColumnName(const QString &typed, const QStringList &columns, int column, bool *tooLong = nullptr);

} // namespace jp::office::sheet
