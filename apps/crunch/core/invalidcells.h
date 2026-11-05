#pragma once
// Circle Invalid Data: the cells with data that a rule of validation does not allow (a rule only holds what is typed; a cell can hold what was there before the rule, or what a formula gives).

#include "sheet/calculation.h"

#include <vector>

namespace jp::crunch {

constexpr size_t kMaxInvalidCells = 10000;

// The cells of the sheet that hold a value the rule that covers them does not allow, in order of row and then column; at most kMaxInvalidCells (`truncated` says more were left out). Empty
// cells are not looked at. Call with the worker held back.
std::vector<jp::office::sheet::CellRange> invalidCells(const jp::office::sheet::Calculation &calc, int sheet, bool *truncated = nullptr);

} // namespace jp::crunch
