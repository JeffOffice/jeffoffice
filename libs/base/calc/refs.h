#pragma once
// Where a parsed formula reads, found without working it out: the blocks of cells its references name, for the cell it is in (a shared formula's references move with the
// cell). What the dependency graph is first told about a formula. What a formula reaches through a defined name, INDIRECT, or OFFSET is not here: it is told when the
// formula has been worked out (EvalContext::touched). No GUI.

#include "calc/area.h"
#include "calc/eval.h"
#include "calc/formula.h"

#include <QHash>
#include <QString>

#include <vector>

namespace jp::calc {

// What a program is made of that matters to where it reads and when it must be worked out again.
struct ProgramTraits {
    bool usesNames = false;       // a defined name stands for a block of cells or a formula: where it reads is found as it is worked out
    bool dynamic = false;         // calls INDIRECT or OFFSET: where it reads depends on values
    bool changesByItself = false; // calls NOW, TODAY, RAND, or RANDBETWEEN: its value changes with no cell changing
    bool iterative = false;       // calls a function found by trying numbers (IRR, XIRR): the digits are those of one program's iteration
    bool reachesBeyond = false;   // a block it reads is made as it is worked out: from references that functions give (the range operator, A1:INDEX(C:C,3)), or of the shape of another
                                  // (the sum range of SUMIF and AVERAGEIF is read as large as the range), so that the cells in it may be named by no reference of the formula
    bool observesPlace = false;   // calls ROW or COLUMN with no argument: its value is its own row or column, which an edit that moves the cell changes with no reference of the formula in it
    // Whether where it reads is told as it is worked out, besides what its references name.
    bool readsWhereItIsTold() const { return usesNames || dynamic || reachesBeyond || observesPlace; }
    // Edits of whole rows and columns change the places of cells and nothing the cells hold (docs/jeffcrunch-rowids.md, 4.6); two traits say which formulas can notice:
    bool observesPosition = false;   // a call can see the position of its own cell or of a cell or block it is given: ROW and COLUMN (with or without an argument), ADDRESS, CELL, OFFSET, INDIRECT, at any depth;
                                     // and a block that is taken for one value of it, the one in the row or column of the formula's cell (implicit intersection), which a cell that moves alone picks another of
    bool blankInsensitive = false;   // no call can notice an empty row or column put inside a block it reads: every block of more than a cell is an argument of a function on the list (blankInsensitiveFunction)
                                     // and goes nowhere else. The default is false: a function nobody classified, a block under an operator, and a name make a formula sensitive.
};
// Whether the function (by name, upper case) gives the same value for a block with a blank row or column put inside it, whatever the block holds: the functions that skip the empty cells they
// are given and do not count where they are. A function that counts cells (COUNTBLANK, ROWS, COLUMNS), that gives a place (MATCH, INDEX, the lookups), that pairs cells of two blocks by
// their place (SUMPRODUCT, SUMIF with blocks of other shapes), or that makes an array is not on the list. Nothing is on it by default.
bool blankInsensitiveFunction(const char *name);
ProgramTraits traitsOf(const Program &program);

// What the traits are for what the program is, and what every defined name it uses stands for, and every name those use (a name that calls RAND makes every formula that uses it
// change by itself). `boundSheets` says which sheet each of the program's sheet names is, `sheet` is the sheet of the cell it is in; `cache`, if given, keeps what was found for
// each name, by its scope and name.
using NameTraitsCache = QHash<QString, ProgramTraits>;
ProgramTraits traitsFollowingNames(const Program &program, const std::vector<qint32> &boundSheets, int sheet, const CellSource &source, NameTraitsCache *cache = nullptr);

// The place of each of the program's sheet names in the source (-1 for a name it has no sheet of).
std::vector<qint32> bindSheetNames(const Program &program, const CellSource &source);

// The blocks the program's references name, for the cell (sheet, row, col) that is shiftRows and shiftCols from the cell the text was written for. A reference to a sheet
// that is not there, or off the edge of the grid, names nothing. A 3-D reference is one block of several sheets. The blocks are added to `out`.
// `places`, for a formula held in the id frame (EvalContext::idFrame): its references are ids, and shiftRows and shiftCols are distances in ids; the blocks are of the places the ids have now (a
// whole row or column is every place, and a reference to a row or column that was deleted names nothing).
void resolveReferences(const Program &program, const std::vector<qint32> &boundSheets, int sheetCount, int sheet, int shiftRows, int shiftCols, std::vector<CellArea> *out, const CellSource *places = nullptr);

} // namespace jp::calc
