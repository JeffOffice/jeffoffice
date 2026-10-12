// Crashes on purpose, for the test of crash reports (tests.cpp,
// crashLeavesAReport):
//   jpcrashtest <report> segv|abort
// Prints where the function that crashes is (its module and offset, as a
// report gives them), then crashes in it.

#include "app/crash.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

int *volatile g_nowhere = nullptr;

// One function for each, so the compiler doesn't move the rarely taken
// path to another part of the program.
[[gnu::noinline]] void crashBySegv() { *g_nowhere = 1; }
[[gnu::noinline]] void crashByAbort() { std::abort(); }

} // namespace

int main(int argc, char **argv)
{
#ifdef _WIN32
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);   // no "stopped working" window
#endif
    if (argc < 3) return 2;
    jp::crash::install(QString::fromLocal8Bit(argv[1]));
    void (*crash)() = std::strcmp(argv[2], "abort") == 0 ? crashByAbort : crashBySegv;
    QString module;
    quint64 offset = 0;
    if (!jp::crash::locate(reinterpret_cast<const void *>(crash), &module, &offset)) return 3;
    std::printf("%s %llx\n", module.toLatin1().constData(), static_cast<unsigned long long>(offset));
    std::fflush(stdout);
    crash();
    return 0;
}
