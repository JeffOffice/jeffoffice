#pragma once
// How much work a slice of a long job (a search, the values of a column, a filter, the keys of a sort) does between two events: the job is done a slice at a time on the window's thread,
// and the number of things a slice does is tuned by how long the last one took, so that a slice is a few milliseconds on any machine and for any kind of cell.

#include <QElapsedTimer>

#include <algorithm>

namespace jp {

class SliceTuner {
public:
    // The most microseconds a slice should take, and the least and most things it does.
    static constexpr qint64 kTargetMicros = 5000;
    static constexpr qint64 kLeast = 50, kMost = 200000;
    qint64 budget() const { return m_fixed > 0 ? m_fixed : m_budget; }
    // A test makes every slice the same size (so that how many slices a job takes is known).
    void setFixed(qint64 things) { m_fixed = things; }
    void begin() { m_timer.start(); }
    // The slice did `done` things (the budget, unless the job ended): the next does as many as fit in the target at this pace.
    void end(qint64 done)
    {
        const qint64 micros = std::max<qint64>(1, m_timer.nsecsElapsed() / 1000);
        if (done <= 0) return;
        const qint64 fit = std::clamp<qint64>(done * kTargetMicros / micros, kLeast, kMost);
        m_budget = std::clamp<qint64>((m_budget + fit) / 2, kLeast, kMost);   // (half way: one slow slice does not make the next tiny)
    }
    void reset() { m_budget = 1000; }

private:
    qint64 m_budget = 1000;
    qint64 m_fixed = 0;
    QElapsedTimer m_timer;
};

} // namespace jp
