#pragma once
// What the editing commands do in a table: a paragraph belongs to a cell (its format has the cell's key), so typing, Enter, and the formatting commands work
// in a cell as they do anywhere; this adds what is particular to a table. Paragraphs of different cells are never joined; Tab goes from cell to cell (and
// adds a row after the last); a selection across cells empties them and takes with it the tables it holds whole; and rows and columns are added and taken
// away. Each is one step that undoes.

#include "core/writedoc.h"

#include "flow/tabledata.h"

#include <QTextBlock>
#include <QDate>
#include <QSet>
#include <QTextCursor>
#include <QVector>
#include <optional>

namespace jp::write {

// The key of the cell a paragraph is in (0: it is not in a table).
int cellKeyOf(const QTextBlock &block);
inline bool inTable(const QTextBlock &block) { return cellKeyOf(block) != 0; }
// Whether the paragraph `second`, which follows `first`, may be joined with it (Delete and Backspace at their meeting): in the same cell, or neither in a table.
bool canJoinBlocks(const QTextBlock &first, const QTextBlock &second);

// Where the cursor is in the innermost table it is in.
struct CellPlace {
    bool inTable = false;
    int row = 0, column = 0;            // from 0, of the cell the cursor is in (its first column)
    int rows = 0, columns = 0;          // the table's
};
CellPlace cellPlace(WriteDoc &doc, const QTextCursor &c);

// A selection that has more than one cell in it, or a cell and what is outside it: deleted a cell at a time, the tables it holds whole taken out. False (nothing
// done) for a selection that is not like that.
bool removeSelectionAcrossCells(WriteDoc &doc, QTextCursor &c);
// Tab and Shift+Tab in a table go to the next and the previous cell, with its text selected; Tab in the last cell adds a row. False when not in a table.
bool tabThroughCells(WriteDoc &doc, QTextCursor &c, bool back);
// A paragraph pasted into a cell is in the cell, and one pasted outside a table is not in one.
void giveCellKey(QTextBlock first, QTextBlock last, int key);

// The rows and columns of the cells the selection is in (the cell the cursor is in, with no selection) get another beside them, or are taken away. A table
// that is left with no row or no column is taken away. All of these work on the innermost table of the cursor.
void insertTableRow(WriteDoc &doc, QTextCursor &c, bool below);
void insertTableColumn(WriteDoc &doc, QTextCursor &c, bool right);
void deleteTableRows(WriteDoc &doc, QTextCursor &c);
void deleteTableColumns(WriteDoc &doc, QTextCursor &c);
void deleteTable(WriteDoc &doc, QTextCursor &c);

// ---- Making and changing tables (each is one step that undoes, and is written to the file so that it opens the same)

// How a table is fitted to the text area: columns of a fixed width, widths that follow the cells' text, or the window (the text area's width).
enum class TableFit { Fixed, Contents, Window };

struct NewTable {
    int rows = 2, columns = 2;
    TableFit fit = TableFit::Fixed;
    QString style = QStringLiteral("TableGrid");
    double columnWidth = 0;             // points; 0: the columns share the text area's width
};
// A table at the cursor: before the paragraph the cursor is in (the paragraph is cut in two when the cursor is inside its text), with the cursor in its first cell.
// False (nothing done) in a table, or for a table of no rows or columns, or more than the file can hold.
bool insertTable(WriteDoc &doc, QTextCursor &c, const NewTable &table);
// Quick Tables: a table with its words in place, at the cursor as insertTable puts one (one step). A calendar has the month of `month` (its days under the days' names), a matrix a header row
// and first column, a list a header row with four columns, and a schedule a time in the first column and a day over each of the others.
enum class QuickTable { Calendar, Matrix, List, Schedule };
bool insertQuickTable(WriteDoc &doc, QTextCursor &c, QuickTable kind, const QDate &month = QDate::currentDate());
// The width in points of the text area the cursor's paragraph is set in.
double textWidthAt(WriteDoc &doc, int position);

// How many times the tables of a story have been looked at to find the one a place is in, and how many cells the commands that join cells have looked at, since the program
// began (cost tests: a caret that moves about a table looks once, and a join looks at the cells near it).
qint64 tableViewsBuiltForTest();
// How many cell records the story and the history of the document can reach (the records held beyond them are let go by the next table command).
int tableRecordsReachableForTest(WriteDoc &doc);
qint64 tableCellsScannedForTest();
// How many sets of a font's measures fitting columns to their cells has made.
qint64 tableFontMeasuresMadeForTest();

// The cell, row, column, or whole table the cursor is in selected.
enum class TableUnit { Cell, Row, Column, Table };
void selectInTable(WriteDoc &doc, QTextCursor &c, TableUnit unit);

// The cells the selection covers are made one (the text of each in turn; empty cells add no paragraph), or one cell is made columns by rows cells.
bool canMergeCells(WriteDoc &doc, const QTextCursor &c);
bool mergeCells(WriteDoc &doc, QTextCursor &c);
bool canSplitCell(WriteDoc &doc, const QTextCursor &c);
bool splitCells(WriteDoc &doc, QTextCursor &c, int columns, int rows);

// Borders: the sides of the selected cells as a whole (top, left, bottom, right), or between them (inside).
namespace BorderSide {
enum : int { Top = 1, Left = 2, Bottom = 4, Right = 8, InsideH = 16, InsideV = 32, Outside = 15, Inside = 48, All = 63 };
}
void setBorders(WriteDoc &doc, QTextCursor &c, int sides, const flow::TableLine &line);
// Shading of the selected cells (an invalid color: none).
void setShading(WriteDoc &doc, QTextCursor &c, const QColor &fill);
// The nine positions of text in a cell: across as the paragraphs' alignment, down as the cell's.
void setCellAlignment(WriteDoc &doc, QTextCursor &c, Qt::Alignment across, flow::CellAlign down);

// Size: the width of the selected columns, the height of the selected rows (0: as tall as the cells need), the columns or the rows made equal, and the fit.
void setColumnWidth(WriteDoc &doc, QTextCursor &c, double points);
void setRowHeight(WriteDoc &doc, QTextCursor &c, double points, bool exact);
// The whole table made that wide (points), every column in the same proportion.
void setTableWidth(WriteDoc &doc, QTextCursor &c, double points);
void distributeColumns(WriteDoc &doc, QTextCursor &c);
void distributeRows(WriteDoc &doc, QTextCursor &c);
void fitTable(WriteDoc &doc, QTextCursor &c, TableFit fit);

// Table Properties.
struct TableProps {
    // the table
    flow::TableData::Align align = flow::TableData::Align::Left;
    double indent = 0;
    enum class Width { Auto, Points, Percent } widthKind = Width::Auto;
    double width = 0;                   // points, or percent
    bool fixedLayout = false;
    QMarginsF margins;                  // default cell margins
    double spacing = 0;
    QString altTitle, altText;
    QString style;
    int look = 0;                       // 0x20 header row, 0x40 total row, 0x80 first column, 0x100 last column, 0x200 no banded rows, 0x400 no banded columns
    // the row the selection starts in
    double rowHeight = 0;
    bool rowExact = false, cantSplit = false, header = false;
    // the column and the cell
    double columnWidth = 0;
    flow::CellAlign valign = flow::CellAlign::Top;
    bool noWrap = false;
    bool operator==(const TableProps &o) const;
};
TableProps tablePropsAt(WriteDoc &doc, const QTextCursor &c);
// The properties that differ between `was` and `now` are applied to the table, the rows, the columns, and the cells the selection is in.
void setTableProps(WriteDoc &doc, QTextCursor &c, const TableProps &was, const TableProps &now);
// The table's style (a built-in one is added to the document when it lacks it), and the options: which of the look's bits are set.
void setTableStyle(WriteDoc &doc, QTextCursor &c, const QString &styleId);
// The tables that have one of these styles made again from the style sheet as it is (after a table style changed), as part of the step in progress.
void restyleTablesOfStyles(WriteDoc &doc, const QSet<QString> &styleIds);
void setTableLook(WriteDoc &doc, QTextCursor &c, int mask, bool on);

// ---- Convert, split, sort, and formulas

// How the text that becomes a table is cut into cells, and how a table's cells are told apart in the text it becomes.
enum class TextSeparator { Paragraphs, Tabs, Commas, Other };
// The paragraphs the selection touches (the cursor's paragraph, with no selection) become the rows of a table: each is cut into cells at the separator (`other` is the character for
// Other); with Paragraphs each paragraph is a cell and `columns` of them make a row. The table has as many columns as the most cells of a row (at least `columns`). The words keep their
// formatting. False (nothing done) for text that is in a table, or that has too many cells.
bool canConvertTextToTable(WriteDoc &doc, const QTextCursor &c);
bool convertTextToTable(WriteDoc &doc, QTextCursor &c, TextSeparator separator, QChar other, int columns);
// The table the cursor is in (not one that has a table in a cell) becomes paragraphs: each row is one paragraph with its cells' words between the separator (Paragraphs: each
// paragraph of the cells stays one).
bool convertTableToText(WriteDoc &doc, QTextCursor &c, TextSeparator separator, QChar other);
// The table is cut in two above the row the cursor is in, with an empty paragraph between them. False for the first row, and where a cell that is joined down goes across the cut.
bool splitTable(WriteDoc &doc, QTextCursor &c);
// Sort the rows by up to three columns. The selected rows are sorted when the selection takes more than one row, else the whole table (but the first row, with `header`). The
// text of the cells moves; what belongs to a cell (its shading and borders) stays where it is.
struct TableSortKey {
    enum class Kind { Text, Number, Date } kind = Kind::Text;
    int column = 0;                     // from 0
    bool ascending = true;
};
bool sortTableRows(WriteDoc &doc, QTextCursor &c, const QVector<TableSortKey> &keys, bool header);
// The numbers in the cells a name of a formula stands for in the table that the position is in: "A1" (column letters, row number), "A1:B3", or ABOVE, BELOW, LEFT, RIGHT (the
// cells that have numbers, up to the first empty one). None when the position is not in a table or the name is not one.
std::optional<QVector<double>> tableCellValues(WriteDoc &doc, int position, const QString &name);

} // namespace jp::write
