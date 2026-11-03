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

// Convert Text to Table: how many columns, and what separates the cells. False when cancelled.
bool convertTextToTableDialog(QWidget *parent, TextSeparator *separator, QChar *other, int *columns);
// Convert Table to Text: what goes between the cells' words. False when cancelled.
bool convertTableToTextDialog(QWidget *parent, TextSeparator *separator, QChar *other);
// Sort: up to three keys (a column, text, number, or date, up or down) and whether the first row is a header. `columns` is how many columns the table has. False when cancelled.
bool sortTableDialog(QWidget *parent, int columns, QVector<TableSortKey> *keys, bool *header);
// Formula: the formula (as the field's code has it, after the equals sign) and how the number is shown (a number picture such as 0.00; empty for as it comes). False when cancelled.
bool tableFormulaDialog(QWidget *parent, QString *formula, QString *picture);

} // namespace jp::write
