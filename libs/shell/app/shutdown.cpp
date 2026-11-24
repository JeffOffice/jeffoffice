#include "app/shutdown.h"

#include "app/settings.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QThread>
#include <QThreadPool>

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <mutex>

namespace jp {

namespace {

// (made with new and never deleted: a static's destructor must not run at the end of the program, see shutdown.h)
std::mutex &stepsLock() { static std::mutex *m = new std::mutex; return *m; }
std::vector<ShutdownStep> &steps() { static auto *v = new std::vector<ShutdownStep>; return *v; }
std::vector<ShutdownStep> &lastSteps() { static auto *v = new std::vector<ShutdownStep>; return *v; }
std::atomic<bool> &letGo() { static auto *b = new std::atomic<bool>(false); return *b; }

constexpr int kWholeMs = 6000;   // (all the steps together)
constexpr int kStepMs = 2000;    // (one step)

// Runs the steps last added first; each is given the time left (its own cap, or what remains of the whole, which of the two is less).
int runBackwards(const std::vector<ShutdownStep> &todo, const ShutdownLimits &limits, const std::function<qint64()> &nowMs, qint64 startedAt, std::vector<int> *given)
{
    int left = 0;
    for (auto it = todo.rbegin(); it != todo.rend(); ++it) {
        const int budget = int(std::max<qint64>(0, std::min<qint64>(limits.stepMs, limits.wholeMs - (nowMs() - startedAt))));
        if (given) given->push_back(budget);
        left += (*it)(budget);
    }
    return left;
}
} // namespace

void addShutdownStep(const ShutdownStep &step)
{
    std::lock_guard<std::mutex> hold(stepsLock());
    steps().push_back(step);
}

void addLastShutdownStep(const ShutdownStep &step)
{
    std::lock_guard<std::mutex> hold(stepsLock());
    lastSteps().push_back(step);
}

bool shutdownLetGo() { return letGo().load(); }

int runShutdownSteps(const std::vector<ShutdownStep> &todo, const ShutdownLimits &limits, const std::function<qint64()> &nowMs, std::vector<int> *given)
{
    return runBackwards(todo, limits, nowMs, nowMs(), given);
}

int shutdownServices()
{
    std::vector<ShutdownStep> todo, last;
    {
        std::lock_guard<std::mutex> hold(stepsLock());
        todo.swap(steps());
        last.swap(lastSteps());
    }
    QElapsedTimer clock;
    clock.start();
    const ShutdownLimits limits{kWholeMs, kStepMs};
    const std::function<qint64()> now = [&clock] { return clock.elapsed(); };
    int left = runBackwards(todo, limits, now, 0, nullptr);
    // (the pool every program shares: work still on it is waited for a moment, and what has not ended is let go of; before the settings are written, so that nothing is set after the last write)
    if (QThreadPool *pool = QThreadPool::globalInstance()) {
        const int budget = int(std::max<qint64>(0, std::min<qint64>(kStepMs, kWholeMs - clock.elapsed())));
        if (!pool->waitForDone(budget)) left += std::max(1, pool->activeThreadCount());
    }
    left += runBackwards(last, limits, now, 0, nullptr);
    if (left > 0) letGo().store(true);
    const QByteArray report = qgetenv("JP_SHUTDOWN_REPORT");
    if (!report.isEmpty() && settingsKeptApart()) {
        QFile f(QString::fromLocal8Bit(report));
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) f.write("threads=" + QByteArray::number(left) + " steps=" + QByteArray::number(int(todo.size() + last.size())) + "\n");
    }
    return left;
}

bool runWithinTime(const std::function<void()> &work, int budgetMs)
{
    QThread *thread = QThread::create(work);
    thread->start();
    if (!thread->wait(static_cast<unsigned long>(std::max(0, budgetMs)))) {
        std::lock_guard<std::mutex> hold(stepsLock());
        static auto *kept = new std::vector<QThread *>;   // (still running: never deleted, and not lost)
        kept->push_back(thread);
        return false;
    }
    delete thread;
    return true;
}

int runProgram(const std::function<int()> &program)
{
    const int code = program();
    if (shutdownLetGo()) {
        std::fflush(nullptr);
        std::_Exit(code);
    }
    return code;
}

} // namespace jp
