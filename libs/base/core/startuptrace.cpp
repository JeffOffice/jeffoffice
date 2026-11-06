#include "core/startuptrace.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>

namespace jp {

namespace {
const char *g_t0 = std::getenv("JP_STARTUP_T0");
const bool g_on = std::getenv("JP_STARTUP_TRACE") != nullptr;
struct First { First() { startupMark("static initializers"); } } g_first;
} // namespace

void startupMark(const char *what)
{
    if (!g_on) return;
    const long long now = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    const long long t0 = g_t0 ? std::atoll(g_t0) : 0;
    timespec cpu{};
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cpu);
    std::fprintf(stderr, "T %8.1f ms  %s  [cpu %.1f]\n", (now - t0) / 1e6, what, cpu.tv_sec * 1e3 + cpu.tv_nsec / 1e6);
}

} // namespace jp
