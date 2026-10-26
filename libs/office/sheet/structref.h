#pragma once
// Structured references: a formula names the cells of a table by the table's name and the names of its columns, as in Sales[Amount], Sales[[#Headers],[Amount]], Sales[[Region]:[Amount]], and,
// inside the table, [@Amount] (the cell of the formula's own row). This reads the text of one into the cells it stands for.

#include "sheet/tables.h"

#include <QString>

namespace jp::office::sheet {

enum class StructResult : quint8 { Ok, Ref, Value };

// Splits the text of a reference into the table's name (empty when it has none, as in [@Amount]) and what is inside its outermost brackets; false if the text is no such reference.
bool splitStructuredRef(const QString &text, QString *table, QString *inside);
// The cells `inside` (see above) names in `table`, for a formula in row `formulaRow` (the row [#This Row] and @ mean; -1 for a formula outside the table's rows). Ok with `out` the cells (a
// block); Ref when a column is not the table's, or the parts asked for cannot be one block (headers with totals); Value when the formula's row is not a row of the table's data.
StructResult structuredArea(const Table &table, const QString &inside, qint64 formulaRow, CellRange *out);

// A reference to the cells of a table's column, as it is typed: "Sales[Amount]" (the name of the column escaped, and in double brackets when it has a mark in it).
QString structuredColumnRef(const QString &table, const QString &column);
// And to a column of the formula's own row: "[@Amount]" (when the formula is inside the table) or "Sales[[#This Row],[Amount]]" (the way the file writes it).
QString structuredRowRef(const QString &table, const QString &column, bool inside);
// The reference to a whole block of the table: "Sales[#Headers]", "Sales[#Totals]", "Sales[#All]", "Sales[[#Headers],[Amount]]" and the like.
QString structuredSpecialRef(const QString &table, const QString &item, const QString &column = QString());

// A formula as the file writes it, from one as it is typed: the short forms [@Amount] and Sales[@Amount] become Sales[[#This Row],[Amount]] (the table named by the one the cell is in when
// the name is left out). `tableAtCell` gives the name of the table the formula's cell is in (empty for none); a short form outside any table is left as it is.
QString fileFormOfStructuredRefs(const QString &formula, const QString &tableAtCell);
// And the other way, for showing: Sales[[#This Row],[Amount]] in a cell of the table Sales shows as [@Amount].
QString typedFormOfStructuredRefs(const QString &formula, const QString &tableAtCell);

// A formula with a table called by another name: Old[Amount] is New[Amount]. (A name in quotes or a sheet's name is not touched.)
QString renameTableInFormula(const QString &formula, const QString &oldName, const QString &newName);
// A formula with the references to a table's cells written as cell references (what a table turned back into plain cells leaves): Sales[Amount] is $B$2:$B$9, [@Amount] is B4. `insideTable`
// says the formula is in the table (a reference with no name is the table's). `formulaRow` is the formula's row, for the references to its own row (-1 when it is not known: those become
// #REF!); a reference that stands for no cells is #REF!. A formula on another sheet than the table's names the sheet.
QString structuredRefsToCells(const QString &formula, const Table &table, const QString &tableSheetName, const QString &formulaSheetName, qint64 formulaRow, bool insideTable);

// A formula with the references to a table that is gone written as #REF!.
QString structuredRefsToError(const QString &formula, const QString &tableName);

} // namespace jp::office::sheet
