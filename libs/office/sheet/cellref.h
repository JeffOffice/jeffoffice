#pragma once
// Cell addresses in A1 style, and the text of formulas that holds them.
//
// Everywhere in the spreadsheet code a row and a column are 0-based numbers
// ("A1" is row 0, column 0); text is 1-based, as people write it.

#include <QString>
#include <QStringList>
#include <QStringView>

namespace jp::office::sheet {

// The grid's size: 16,384 columns (A to XFD) and 1,048,576 rows.
constexpr quint32 kMaxColumns = 16384;
constexpr quint32 kMaxRows = 1048576;

// A block of cells, both ends in it. `valid` is false for none.
struct CellRange {
    quint32 row1 = 0, col1 = 0, row2 = 0, col2 = 0;
    bool valid = false;
    qint64 cellCount() const { return valid ? qint64(row2 - row1 + 1) * qint64(col2 - col1 + 1) : 0; }
    bool contains(quint32 row, quint32 col) const { return valid && row >= row1 && row <= row2 && col >= col1 && col <= col2; }
    bool operator==(const CellRange &o) const { return valid == o.valid && (!valid || (row1 == o.row1 && col1 == o.col1 && row2 == o.row2 && col2 == o.col2)); }
};

// "B3" (or "$B$3") as a row and a column; false if it isn't one inside the
// grid. Letters in either case.
bool parseCellRef(QStringView text, quint32 *row, quint32 *col);
// "A1:C3", or "A1" for a range of one cell; both ends inside the grid, in
// either order. False (and `out` not valid) if it is neither.
bool parseRangeRef(QStringView text, CellRange *out);
// 0 is "A", 25 "Z", 26 "AA", 16383 "XFD"; empty past the grid.
QString columnName(quint32 col);
// "A1" for row 0, column 0.
QString cellName(quint32 row, quint32 col);
QString rangeName(const CellRange &range);

// The formula, written for one cell, as it reads in a cell `rows` down and
// `cols` to the right of that one: each reference without a "$" before its
// column moves that many columns, and each without one before its row that
// many rows ("A1" to "B3", "$A1" to "$A3", "A$1:B$2" to "B$1:C$2", "C:D" to
// "D:E", "1:2" to "3:4"), whichever sheet it names. What isn't a reference stays
// as written: text in quotes, the names of sheets and tables and of
// defined names, function names (LOG10 and ATAN2 look like cells and are not),
// numbers (1E5), and errors (#REF!). A reference that would leave the grid
// becomes #REF!. The text has no leading "=".
QString shiftFormula(const QString &formula, int rows, int cols);

// The names of the functions a formula calls, upper-cased and in the order
// they appear, a name once for each call. A prefix a newer function carries
// in the file (_xlfn., _xlws.) is not part of its name. Text in quotes and
// what is in square brackets (table columns) holds none.
QStringList formulaFunctions(const QString &formula);

} // namespace jp::office::sheet
