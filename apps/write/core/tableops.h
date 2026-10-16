#pragma once
// What the editing commands do in a table: a paragraph belongs to a cell (its format has the cell's key), so typing, Enter, and the formatting commands work
// in a cell as they do anywhere; this adds what is particular to a table. Paragraphs of different cells are never joined; Tab goes from cell to cell (and
// adds a row after the last); a selection across cells empties them and takes with it the tables it holds whole; and rows and columns are added and taken
// away. Each is one step that undoes.

#include "core/writedoc.h"

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

} // namespace jp::write
