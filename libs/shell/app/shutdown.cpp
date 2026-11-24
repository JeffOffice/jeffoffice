#include "app/shutdown.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QThreadPool>

#include <algorithm>
#include <mutex>
#include <vector>

namespace jp {

namespace {

// (made with new and never deleted: a static's destructor must not run at the end of the program, see shutdown.h)
std::mutex &stepsLock() { static std::mutex *m = new std::mutex; return *m; }
std::vector<ShutdownStep> &steps() { static auto *v = new std::vector<ShutdownStep>; return *v; }

constexpr int kWholeMs = 6000;   // (all the steps together)
constexpr int kStepMs = 2000;    // (one step)

} // namespace

void addShutdownStep(const ShutdownStep &step)
{
    std::lock_guard<std::mutex> hold(stepsLock());
    steps().push_back(step);
}

int shutdownServices()
{
    std::vector<ShutdownStep> todo;
    {
        std::lock_guard<std::mutex> hold(stepsLock());
        todo.swap(steps());
    }
    QElapsedTimer clock;
    clock.start();
    auto budget = [&clock](int most) { return int(std::max<qint64>(0, std::min<qint64>(most, kWholeMs - clock.elapsed()))); };
    int left = 0;
    for (auto it = todo.rbegin(); it != todo.rend(); ++it) left += (*it)(budget(kStepMs));
    // (the pool every program shares: work still on it is waited for a moment, and what has not ended is let go of)
    if (QThreadPool *pool = QThreadPool::globalInstance())
        if (!pool->waitForDone(budget(kStepMs))) left += std::max(1, pool->activeThreadCount());
    const QByteArray report = qgetenv("JP_SHUTDOWN_REPORT");
    if (!report.isEmpty()) {
        QFile f(QString::fromLocal8Bit(report));
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) f.write("threads=" + QByteArray::number(left) + " steps=" + QByteArray::number(int(todo.size())) + "\n");
    }
    return left;
}

} // namespace jp
