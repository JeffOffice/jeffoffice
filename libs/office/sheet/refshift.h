#pragma once
// What happens to the references of a workbook when cells move: rows and columns inserted or deleted, cells inserted or deleted with the cells after them shifted, a block of cells
// cut and pasted, and a formula copied to another cell (its relative references move). A reference follows the cells it names: one to a cell that moves moves with it, a range that has
// a row inserted inside it grows, a range loses the rows deleted from it, and a reference to cells that are deleted becomes #REF!, as the file format expects.
//
// Everything here works on text and ranges, not on the model: the same rules rewrite the formulas of every sheet, the defined names, the validations and conditional formats, the tables,
// the charts' ranges, and the hyperlinks, in the cells and in the file.

#include "sheet/cellref.h"

#include <QString>
#include <QStringList>
#include <QVector>

#include <memory>

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
    // Move, for the formulas of the cells that move: what they read that is written over by the move is still read (it is the formulas of other cells that read it that have #REF!).
    bool keepsWhatIsWrittenOver = false;

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

// Where a cell is after the edit (what a stored value, a mark on a cell, follows): Same, Moved (`row` and `col` are the new place; for a Move the sheet too, in `sheet`), or Gone (deleted,
// pushed off the edge, or written over by a Move).
RefFate adjustCell(const StructEdit &edit, int *sheet, quint32 *row, quint32 *col);

// A run of columns [first, last] (the width a file gives to columns, say) after columns are inserted or deleted: false if all of it was deleted or went off the grid. A run that new columns
// are inserted inside of grows to hold them.
bool adjustColumnRun(const StructEdit &edit, quint32 *first, quint32 *last);

// A range (a merged range, the cells of a validation) after the edit: false if it is gone.
bool adjustRange(const StructEdit &edit, int sheet, CellRange *range);
// A list of ranges as a file writes it ("A1:B2 D4 F6:F9"): what is left of them after the edit, "" if all are gone.
QString adjustSqref(const QString &sqref, const StructEdit &edit, int sheet);

// For tests: whether adjustFormula makes the common case (a formula of plain cell references, numbers, and operators, for rows inserted or deleted) without reading it as a formula; the
// result is the same either way (a test compares them).
void setFastRowEdits(bool on);

// A formula as a file stores it (no "=" in front) with its references changed for the edit. `ownSheet` is the sheet the formula is on (a reference with no sheet names it), and
// `sheetNames` the names of the workbook's sheets in order. A reference to a sheet that is not in the list (another workbook) is left as it is. `changed`, if given, says whether the text
// is not the one given. `definedName` is for the text of a defined name.
QString adjustFormula(const QString &formula, int ownSheet, const QStringList &sheetNames, const StructEdit &edit, bool *changed = nullptr, bool definedName = false);
// The references of one formula text, read once: what an edit of rows or columns does to the references of the cells that share the text (a shared formula's text is written for its first
// cell, and each cell is a distance from it) is worked out for each cell from what happens to its own references, and the text it comes to is made once for all the cells that the edit
// treats alike. It is made so that the text for a cell that is `dRow` rows and `dCol` columns from the one it was written for, offsetFormula(apply(key), dRow, dCol), is the same as
// adjustFormula(offsetFormula(text, dRow, dCol), edit): a test compares them.
class TextRewriter {
public:
    static constexpr int kMaxRefs = 6;
    // What happens to each reference: unchanged, gone, or its four ends moved by so much (the same for every cell of the group).
    struct Key {
        qint32 v[kMaxRefs * 5] = {0};
        bool operator==(const Key &o) const;
        quint64 hash() const;
    };
    enum class Fate { Same, Changed, Unsupported };
    TextRewriter(const QString &text, int ownSheet, const QStringList &sheetNames);
    ~TextRewriter();
    TextRewriter(TextRewriter &&) noexcept;
    TextRewriter &operator=(TextRewriter &&) noexcept;
    TextRewriter(const TextRewriter &) = delete;
    // Whether the text can be handled this way (it is read, and has no more references than a key holds).
    bool usable() const;
    // What the edit (rows or columns inserted or deleted) does to the references of the cell at (dRow, dCol) from the cell the text was written for. Unsupported: leave it to adjustFormula
    // (a reference that goes off the grid for the cell, or ends that cross, say).
    Fate fate(const StructEdit &edit, qint32 dRow, qint32 dCol, Key *key) const;
    // The text for the cells of the key (written for the same cell as before); empty if it can't be made (a reference that would be off the grid in it).
    QString apply(const Key &key) const;

private:
    struct Impl;
    std::unique_ptr<Impl> d;
};

// The formula of a cell that a cut moves (a Move edit): the references to cells that move with it follow them, and when the cell goes to another sheet the references to cells that stay
// name the sheet it came from (the formula reads what it read), the others the sheet it is on now.
QString adjustMovedFormula(const QString &formula, const QStringList &sheetNames, const StructEdit &move, bool *changed = nullptr);

// The formula of a rule (a validation, a conditional format) after the edit. Such a formula is written for the first cell of the rule's first range (`firstRange`, as it was before the
// edit): a reference in it that is not fixed with "$" is to the cell that far from that one. After the edit it is written for the first cell that is left of the range, so it is read as
// written for the cell that is there now (the one that was the first cell left), and what it names follows its cells: ISNUMBER($A2) for B2:B10 is ISNUMBER($A2) for B2:B9 when row 2 is
// deleted, not ISNUMBER(#REF!). `ownSheet` is the sheet the rule is on.
QString adjustRuleFormula(const QString &formula, int ownSheet, const QStringList &sheetNames, const StructEdit &edit, const CellRange &firstRange, bool *changed = nullptr);

// The text of a formula for the cell that is `dRow` rows and `dCol` columns from the cell it was written for, as a copy of it has it (what Fill and Paste give); references without "$" move,
// those that would go off the grid are #REF!.
QString offsetFormula(const QString &formula, qint32 dRow, qint32 dCol, bool *changed = nullptr);

// The sheets themselves: a formula as a file stores it with a sheet renamed (each reference that names `oldName` names `newName`, quoted as a formula must when it has anything but letters,
// digits, and underscores), and as it is when a sheet is deleted (each reference to its cells is #REF!). A name is matched without regard to case. `definedName` is for the text of a defined
// name. Both leave a text as it is when nothing in it names the sheet.
QString renameSheetInFormula(const QString &formula, const QString &oldName, const QString &newName, bool *changed = nullptr, bool definedName = false);
QString deleteSheetInFormula(const QString &formula, const QString &name, bool *changed = nullptr, bool definedName = false);
// Whether a formula has a reference through several sheets ("Sheet1:Sheet3!A1"): what moving or inserting a sheet would change the meaning of.
bool formulaHasThreeD(const QString &formula, bool definedName = false);

} // namespace jp::office::sheet
