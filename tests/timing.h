// What the tests that compare speeds share. A busy machine can make one run of a case several times slower than the same run on a quiet
// one, so a test that asserts a time takes the fastest of several runs: the work itself, without the load the slow runs had.
#pragma once

#include <QElapsedTimer>
#include <algorithm>
#include <functional>
#include <limits>

// The fastest of n runs of run(), in nanoseconds. When before() is given it runs before each run, outside the clock, to set up what the run
// needs (a fresh document, a cursor, the text to replace).
inline qint64 bestNanoseconds(const std::function<void()> &run, int n = 5, const std::function<void()> &before = nullptr)
{
    qint64 best = std::numeric_limits<qint64>::max();
    for (int i = 0; i < n; ++i) {
        if (before) before();
        QElapsedTimer timer;
        timer.start();
        run();
        best = std::min(best, timer.nsecsElapsed());
    }
    return best;
}
