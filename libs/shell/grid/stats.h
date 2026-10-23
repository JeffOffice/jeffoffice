#pragma once
// The numbers the status bar shows of a selection: how many cells hold something, and the sum, average, smallest, and largest of those that hold numbers. Rows and columns that
// are hidden are left out, a merged range is one cell, and a formula is counted at the value it shows. A selection of a whole column of a million rows is not added up at once:
// a StatsWalk does a few thousand rows at a time and says when it is done.

#include "grid/gridsheet.h"
#include "grid/selection.h"

#include <QString>

namespace jp::grid {

struct Stats {
    qint64 count = 0;       // cells that hold something (text, numbers, true or false, errors)
    qint64 numbers = 0;     // of them, numbers
    double sum = 0;
    double min = 0, max = 0;
    bool hasNumbers() const { return numbers > 0; }
    double average() const { return numbers > 0 ? sum / double(numbers) : 0; }
};

class StatsWalk {
public:
    // Rows looked at in each look at the cells (a few thousand: a fraction of a millisecond).
    static constexpr qint64 kRowsPerSlice = 4096;
    StatsWalk(const GridSheet *sheet, const GridRange &range);
    // Looks at rows until about `budget` have been looked at (0 for all of them); true when every row has been.
    bool step(qint64 budget);
    const Stats &stats() const { return m_stats; }
    // Rows looked at so far (not counting those that are hidden), and the cells stepped over: what a test counts.
    qint64 rowsLooked() const { return m_rowsLooked; }

private:
    const GridSheet *m_sheet;
    GridRange m_range;
    qint64 m_next;
    Stats m_stats;
    qint64 m_rowsLooked = 0;
};

// All of it at once.
Stats computeStats(const GridSheet &sheet, const GridRange &range);
// "Average: 3.5    Count: 6    Sum: 21", or "Count: 2", or nothing, as the numbers call for.
QString statsText(const Stats &stats);

} // namespace jp::grid
