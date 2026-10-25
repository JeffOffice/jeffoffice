#pragma once
// Checking formulas: what a formula reads (the blocks of cells its text names, and the ones the names in it stand for), and what reads a cell (the formulas that name it, found through the
// dependency graph). No GUI.

#include "sheet/calculation.h"

#include <vector>

namespace jp::crunch {

struct TraceBlock {
    int sheet = 0;
    jp::office::sheet::CellRange cells;
};
struct TraceCell {
    int sheet = 0;
    quint32 row = 0, col = 0;
};

// The most blocks one formula is followed to, the most names followed inside a formula, and how deep a name that names another is followed.
constexpr int kMaxTraceBlocks = 400;
constexpr int kMaxTraceNames = 50;
constexpr int kMaxTraceNameDepth = 4;
// The most formulas that read a cell that are listed (the number there are is told besides).
constexpr qint64 kMaxTraceReaders = 4000;

// The blocks of cells the formula in the cell reads, in the order its text names them (a reference to another workbook is left out; `truncated` says that blocks past the bound were). Empty
// for a cell with no formula.
std::vector<TraceBlock> cellsRead(const jp::office::sheet::Calculation &calc, int sheet, quint32 row, quint32 col, bool *truncated = nullptr);
// The formulas that read the cell (one that reads it in more than one way once), in the order of the sheets and cells, at most `cap`; `total` is how many there are.
std::vector<TraceCell> readersOf(const jp::office::sheet::Calculation &calc, int sheet, quint32 row, quint32 col, qint64 cap, qint64 *total = nullptr);

} // namespace jp::crunch
