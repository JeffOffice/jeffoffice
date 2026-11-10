#include "core/threadcheck.h"

#include <QCoreApplication>
#include <QThread>

namespace jp::threadcheck {

std::atomic<bool> g_watching{false};
static std::atomic<int> g_offThread{0};

void noteOffThread()
{
    if (QCoreApplication::instance() && QThread::currentThread() != QCoreApplication::instance()->thread()) ++g_offThread;
}

void watch(bool on) { g_watching = on; }
int offThreadCalls() { return g_offThread; }
void reset() { g_offThread = 0; }

} // namespace jp::threadcheck
