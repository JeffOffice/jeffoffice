#pragma once
// Table commands that need no window: what a table's rows, columns, cells, and borders do when a ribbon,
// a menu, or a key asks. Each command works on the cells the editor says it is for (the block of
// selected cells, the cell the text cursor is in, or with only the table selected, the whole table),
// and is one step that undoes. A command that can't be done says why in the editor's status bar.

#include "app/editor.h"
#include "core/document.h"
#include "office/drawingml_table.h"
#include "office/theme.h"

#include <QString>

namespace jp {

// The table the selection is in: the one being typed in, else the selected object when it is a table.
TableItem *selectedTable(Editor *ed);

// Takes away a cell's division by a diagonal: the second half's text joins the first's (the first half's last
// paragraph goes on with the second's first), and the cell is one again.
void joinHalves(Document *d, TableCell &cell);
// Drops the stories no object uses any more, as deleting objects does.
void dropUnusedStories(Document *d);

// Structure edits that keep stories, spans, and borders consistent. A new row or column through a merged cell
// makes the merged cell bigger, and the new cells under it are covered; deleted rows and columns shorten the
// merged cells that reach into them (a table keeps at least one of each).
void tableInsertRow(Editor *ed, TableItem *t, int at);
void tableInsertCol(Editor *ed, TableItem *t, int at);
void tableDeleteRows(TableItem *t, int r0, int r1);
void tableDeleteCols(TableItem *t, int c0, int c1);

// Rows and columns. Insert puts a new one beside the rows (columns) the selected cells are in, or at the end
// of the table with nothing but the table selected.
// Each says what it did (count 0 when nothing).
Editor::TableEdit insertTableRows(Editor *ed, bool below);
Editor::TableEdit insertTableColumns(Editor *ed, bool right);
// Nothing is deleted, with the reason in the status bar, when no cells are named or the table would be left with none.
Editor::TableEdit deleteTableRows(Editor *ed);
Editor::TableEdit deleteTableColumns(Editor *ed);
void deleteTable(Editor *ed);

// Merge Cells joins the selected cells into one, whose text is all of theirs (one cell on its own joins the
// cell on its right). Split Cells makes each merged cell in the selection its separate cells again.
void mergeTableCells(Editor *ed);
void splitTableCells(Editor *ed);

// Select Table, Row, Column, and Cell.
void selectTable(Editor *ed);
void selectTableRow(Editor *ed);
void selectTableColumn(Editor *ed);
void selectTableCell(Editor *ed);

// Distribute Rows and Columns make the table's rows (columns) all the same size, keeping the table's size.
void distributeTableRows(Editor *ed);
void distributeTableColumns(Editor *ed);

// The height of the rows (the width of the columns) the selected cells are in, in points.
void setTableRowHeight(Editor *ed, double points);
void setTableColumnWidth(Editor *ed, double points);

// Shading, alignment, text direction, and margins of the cells the commands above work on. Text Direction turns the text of every one of the cells
// a quarter turn, or back if they all are turned.
void setTableShading(Editor *ed, const ColorRef &color);   // no color: no fill
void setTableCellAlignment(Editor *ed, VAlign align);
void toggleTableTextDirection(Editor *ed);
void setTableCellMargins(Editor *ed, const QMarginsF &margins);

// Which edges Borders draws the line on, in the cells it works on.
enum class BorderSides { All, Outside, Inside, None, Top, Bottom, Left, Right, InsideHorizontal, InsideVertical };
void setTableBorders(Editor *ed, BorderSides sides, const Stroke &line);

// A presentation's table styles (office/tablestyles.h). Which parts of the style show: Header Row, Total Row, First Column, Last
// Column, Banded Rows, and Banded Columns (the table's header, totalRow, firstCol, lastCol, banded, and bandedCols).
struct TableStyleParts {
    bool header = true, totalRow = false, firstCol = false, lastCol = false, banded = true, bandedCols = false;
    static TableStyleParts of(const TableItem &t) { return {t.header, t.totalRow, t.firstCol, t.lastCol, t.banded, t.bandedCols}; }
};

// Where a table's styles come from: the colors of the theme, and the definitions the presentation's file carried (which win over this suite's own
// for the same id, as office::tableStyleEffect says).
struct TableStyles {
    office::Theme theme;
    office::TableStyleDefs defs;
};

// Gives the table a style and the parts of it that show (the colors are the theme's), as one step that undoes: every cell takes the fill,
// the lines, and the bold, italic, and color of its text that the style gives it. What the old style did not give is the person's own
// and stays (a shaded cell, a border drawn by hand, a word set bold), unless `reset`, which makes every cell as the style has it. A
// style the program does not know by id (or a table whose style was one of JeffPub's) is named and its parts set, and nothing else.
void setTableStyle(Editor *ed, TableItem *t, const QString &styleId, const TableStyleParts &parts, const TableStyles &styles, bool reset);
// A table whose style is known keeps it through rows or columns coming and going: after the edit, each cell has the place in the style that
// its row and column now give it (the banding goes on, a new row has the look of its neighbors in the style, a header row stays the first
// row), except what the person did to it. Editor::tableEdited calls this for JeffDeck.
void keepTableStyle(Editor *ed, TableItem *t, const Editor::TableEdit &edit, const TableStyles &styles);
// The same without the step: for a table that is being made (not yet on the page) or inside a change of its own.
void giveTableStyle(Editor *ed, TableItem *t, const QString &styleId, const TableStyleParts &parts, const TableStyles &styles, bool reset);

} // namespace jp
