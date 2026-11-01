#pragma once
// What the editing commands do in a table: a paragraph belongs to a cell (its format has the cell's key), so typing, Enter, and the formatting commands work
// in a cell as they do anywhere; this adds what is particular to a table. Paragraphs of different cells are never joined; Tab goes from cell to cell (and
// adds a row after the last); a selection across cells empties them and takes with it the tables it holds whole; and rows and columns are added and taken
// away. Each is one step that undoes.

#include "core/writedoc.h"

#include "flow/tabledata.h"

#include <QTextBlock>
#include <QTextCursor>

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
// The width in points of the text area the cursor's paragraph is set in.
double textWidthAt(WriteDoc &doc, int position);

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
void setTableLook(WriteDoc &doc, QTextCursor &c, int mask, bool on);

} // namespace jp::write
