#include "calc/budget.h"

#include <algorithm>
#include <limits>

namespace jp::calc::budget {

namespace {

struct State {
    qint64 pendingWork = 0;
    qint64 live = 0;      // cells of arrays alive (a thread that frees what another made goes below 0: only the difference from `base` is used)
    qint64 base = 0;
    qint64 limit = std::numeric_limits<qint64>::max() / 4;   // cells beyond base
    qint64 peak = 0;
    bool refused = false;
};

thread_local State g;

} // namespace

void addWork(qint64 steps) { g.pendingWork += steps; }

qint64 takeWork()
{
    const qint64 n = g.pendingWork;
    g.pendingWork = 0;
    return n;
}

void beginEvaluation(qint64 limit)
{
    g.pendingWork = 0;
    g.base = g.live;
    g.limit = limit;
    g.peak = g.live;
    g.refused = false;
}

void endEvaluation() { g.limit = std::numeric_limits<qint64>::max() / 4; }

bool refused() { return g.refused; }

qint64 peakCells() { return std::max<qint64>(0, g.peak - g.base); }

void arrayMade(qint64 cells)
{
    g.live += cells;
    if (g.live > g.peak) g.peak = g.live;
}

void arrayFreed(qint64 cells) { g.live -= cells; }

bool mayMake(qint64 cells)
{
    if (g.live - g.base + cells > g.limit) {
        g.refused = true;
        return false;
    }
    return true;
}

} // namespace jp::calc::budget
