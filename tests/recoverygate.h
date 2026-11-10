#pragma once
// For tests of a recovery copy made on a worker thread: a hook that holds the worker until the test lets it go. The state is shared (not a reference into the test), so a test that fails
// halfway leaves no worker touching a stack that is gone, and the test's guard lets the worker go however the test ends.

#include <QScopeGuard>
#include <QSemaphore>
#include <QThread>
#include <atomic>
#include <functional>
#include <memory>

struct RecoveryGate {
    QSemaphore release;
    std::atomic<QThread *> madeOn{nullptr};
    std::atomic<int> made{0};
    // The hook: counts the call, notes the thread, and waits to be let go (a minute at most, so that a stuck test ends).
    static std::function<void()> hook(const std::shared_ptr<RecoveryGate> &gate)
    {
        return [gate] {
            gate->madeOn = QThread::currentThread();
            ++gate->made;
            gate->release.tryAcquire(1, 60000);
        };
    }
    void open() { release.release(1000); }
};
