// What the few tests that still ask for a time share. The tests of cost assert counted work (steps, characters read, pictures made, ...); a clock is kept only as a ceiling
// that is ten times or more what the case takes alone, to keep a work budget's units honest or to stop a hang. A busy machine can make one run of a case several times slower
// than the same run on a quiet one, so such a test takes the fastest of several runs: the work itself, without the load the slow runs had.
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
