#pragma once
// What happens to the references of a workbook when cells move: rows and columns inserted or deleted, cells inserted or deleted with the cells after them shifted, a block of cells
// cut and pasted, and a formula copied to another cell (its relative references move). A reference follows the cells it names: one to a cell that moves moves with it, a range that has
// a row inserted inside it grows, a range loses the rows deleted from it, and a reference to cells that are deleted becomes #REF!, as the file format expects.
//
// Everything here works on text and ranges, not on the model: the same rules rewrite the formulas of every sheet, the defined names, the validations and conditional formats, the tables,
// the charts' ranges, and the hyperlinks, in the cells and in the file.

#include "calc/formula.h"
#include "sheet/cellref.h"

#include <QString>
#include <QStringList>
#include <QVector>

namespace jp::office::sheet {

struct StructEdit {
    enum class Kind : quint8 { InsertRows, DeleteRows, InsertColumns, DeleteColumns, InsertCellsDown, DeleteCellsUp, InsertCellsRight, DeleteCellsLeft, Move, Offset };
    Kind kind = Kind::InsertRows;
    int sheet = 0;                              // the sheet it happens on
    // Rows or columns: those from `first` (from 0), `count` of them.
    quint32 first = 0, count = 1;
    // Cells: the block that is made room for (inserted) or closed up (deleted); the cells below it (right of it) in its columns (rows) move. For a Move, the block that moves.
    quint32 row1 = 0, col1 = 0, row2 = 0, col2 = 0;
    // Move: where the block's first cell goes (the sheet, row, and column).
    int toSheet = 0;
    quint32 toRow = 0, toCol = 0;
    // Offset: how far a copied formula is from where it was written (only parts without "$" move).
    qint32 dRow = 0, dCol = 0;

    static StructEdit insertRows(int sheet, quint32 first, quint32 count);
    static StructEdit deleteRows(int sheet, quint32 first, quint32 count);
    static StructEdit insertColumns(int sheet, quint32 first, quint32 count);
    static StructEdit deleteColumns(int sheet, quint32 first, quint32 count);
    static StructEdit insertCellsDown(int sheet, const CellRange &block);
    static StructEdit deleteCellsUp(int sheet, const CellRange &block);
    static StructEdit insertCellsRight(int sheet, const CellRange &block);
    static StructEdit deleteCellsLeft(int sheet, const CellRange &block);
    static StructEdit move(int sheet, const CellRange &block, int toSheet, quint32 toRow, quint32 toCol);
    static StructEdit offset(qint32 dRow, qint32 dCol);
    // The edit that undoes this one for the cells (not for what was deleted): an insert for a delete and the other way round, a move back.
    StructEdit inverse() const;
    // Whether the edit moves cells of rows (or columns): what the rows' and columns' own settings (height, width, hidden) follow.
    bool changesRows() const { return kind == Kind::InsertRows || kind == Kind::DeleteRows; }
    bool changesColumns() const { return kind == Kind::InsertColumns || kind == Kind::DeleteColumns; }
};

enum class RefFate : quint8 { Same, Moved, Gone };

// Where the cells of a block (both ends in it) of `sheet` are after the edit: Same, Moved (the ends and the sheet in `out` are the new ones), or Gone (all of it was deleted, or it went off the
// edge of the grid). `wholeRows` and `wholeColumns` say that the block is whole rows or whole columns (a reference such as 3:5 or A:C), which an edit to the other kind does not touch.
struct Block {
    int sheet = 0;
    quint32 row1 = 0, col1 = 0, row2 = 0, col2 = 0;
    bool wholeRows = false, wholeColumns = false;
};
RefFate adjustBlock(const StructEdit &edit, const Block &in, Block *out);

// A range (a merged range, the cells of a validation) after the edit: false if it is gone.
bool adjustRange(const StructEdit &edit, int sheet, CellRange *range);
// A list of ranges as a file writes it ("A1:B2 D4 F6:F9"): what is left of them after the edit, "" if all are gone.
QString adjustSqref(const QString &sqref, const StructEdit &edit, int sheet);

// A formula as a file stores it (no "=" in front) with its references changed for the edit. `ownSheet` is the sheet the formula is on (a reference with no sheet names it), and
// `sheetNames` the names of the workbook's sheets in order. A reference to a sheet that is not in the list (another workbook) is left as it is. `changed`, if given, says whether the text
// is not the one given. `definedName` is for the text of a defined name.
QString adjustFormula(const QString &formula, int ownSheet, const QStringList &sheetNames, const StructEdit &edit, bool *changed = nullptr, bool definedName = false);
// The text of a formula for the cell that is `dRow` rows and `dCol` columns from the cell it was written for, as a copy of it has it (what Fill and Paste give); references without "$" move,
// those that would go off the grid are #REF!.
QString offsetFormula(const QString &formula, qint32 dRow, qint32 dCol, bool *changed = nullptr);

} // namespace jp::office::sheet
