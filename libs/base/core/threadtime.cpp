#include "core/threadtime.h"

#if defined(Q_OS_WIN)
#include <windows.h>
#else
#include <time.h>
#endif

namespace jp {

qint64 threadCpuMicros()
{
#if defined(Q_OS_WIN)
    FILETIME created, exited, kernel, user;
    if (!GetThreadTimes(GetCurrentThread(), &created, &exited, &kernel, &user)) return 0;
    auto micros = [](const FILETIME &t) { return (qint64(t.dwHighDateTime) << 32 | t.dwLowDateTime) / 10; };   // (units of 100 nanoseconds)
    return micros(kernel) + micros(user);
#else
    timespec ts;
    if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts) != 0) return 0;
    return qint64(ts.tv_sec) * 1000000 + ts.tv_nsec / 1000;
#endif
}

} // namespace jp
