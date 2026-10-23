#pragma once
// What one evaluation spends that the functions do not see one by one (internal to calc/): steps taken by rules that have no machine at hand (comparing two texts,
// matching a pattern), which Machine::charge takes into the work, and the cells of the arrays that are alive at once, which are refused past a budget. Both are per thread, as
// an evaluator belongs to one thread at a time.

#include <QtGlobal>

namespace jp::calc::budget {

// Steps to count with the next charge.
void addWork(qint64 steps);
qint64 takeWork();

// An array is alive from when it is made until the last copy of it is gone, and counts for its cells. An evaluation may have `limit` cells alive more than there were when
// it began (arrays kept by programs are of the ones before); Value::array gives no array, and refused() is true, once that would be passed.
void beginEvaluation(qint64 limit);
void endEvaluation();
bool refused();
// The most cells alive at once since the evaluation began, beyond those alive then.
qint64 peakCells();

// For Value: cells made alive, and no longer alive; and whether `cells` more may be made (if not, the refusal is noted).
void arrayMade(qint64 cells);
void arrayFreed(qint64 cells);
bool mayMake(qint64 cells);

} // namespace jp::calc::budget
