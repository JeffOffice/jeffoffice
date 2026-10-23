#pragma once
// Where a parsed formula reads, found without working it out: the blocks of cells its references name, for the cell it is in (a shared formula's references move with the
// cell). What the dependency graph is first told about a formula. What a formula reaches through a defined name, INDIRECT, or OFFSET is not here: it is told when the
// formula has been worked out (EvalContext::touched). No GUI.

#include "calc/area.h"
#include "calc/eval.h"
#include "calc/formula.h"

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
    // Whether where it reads is told as it is worked out, besides what its references name.
    bool readsWhereItIsTold() const { return usesNames || dynamic || reachesBeyond; }
};
ProgramTraits traitsOf(const Program &program);

// The place of each of the program's sheet names in the source (-1 for a name it has no sheet of).
std::vector<qint32> bindSheetNames(const Program &program, const CellSource &source);

// The blocks the program's references name, for the cell (sheet, row, col) that is shiftRows and shiftCols from the cell the text was written for. A reference to a sheet
// that is not there, or off the edge of the grid, names nothing. A 3-D reference is one block of several sheets. The blocks are added to `out`.
void resolveReferences(const Program &program, const std::vector<qint32> &boundSheets, int sheetCount, int sheet, int shiftRows, int shiftCols, std::vector<CellArea> *out);

} // namespace jp::calc
