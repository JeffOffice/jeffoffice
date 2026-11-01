#pragma once
// The dialog boxes of tables: Insert Table, Split Cells, Table Properties, and Borders and Shading.

#include "core/tableops.h"

class QTextCursor;
class QWidget;

namespace jp::write {

class WriteDoc;

// Insert Table: the columns and rows, and how the columns are fitted. Returns the table asked for, with false when the dialog was cancelled. What was asked is remembered when
// the person says to.
bool insertTableDialog(QWidget *parent, NewTable *table);
// Split Cells: into how many columns and rows. False when cancelled.
bool splitCellsDialog(QWidget *parent, int *columns, int *rows, int mostRows);
// Table Properties for the table, rows, columns, and cells the cursor selects; what the person changed is applied as one step. False when cancelled.
bool tablePropertiesDialog(QWidget *parent, WriteDoc *doc, QTextCursor *cursor);
// Borders and Shading: the lines (their style, width, and color, and which sides) and the fill of the cells selected, or of the whole table.
bool bordersShadingDialog(QWidget *parent, WriteDoc *doc, QTextCursor *cursor);

} // namespace jp::write
