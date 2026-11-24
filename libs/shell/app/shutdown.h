#pragma once
// How a program ends, and what may be left running when it does (developer note: docs/shutdown.md).
//
// THE RULE for anything that lives as long as the program and owns a thread, a pool of threads, a file watcher, a timer on another thread, a media player, a network manager,
// or a child process (the five kinds of thing that wait for another thread when they are destroyed):
//   (a) it is stopped on purpose, by a step added with addShutdownStep(), which each program's main runs through shutdownServices() (by a ShutdownGuard, on every way out of main(),
//       before the application object goes); the step waits for a counted time at most and then lets go of what has not stopped; or
//   (b) it is made with `new` and never deleted, so that no destructor of a static ever runs it.
// No static (a namespace-scope object, a function-local static, a Q_GLOBAL_STATIC) has a destructor that waits for another thread, locks against one, or signals one.
// tools/check_static_threads.py keeps new ones out. The reason: when a program ends on Windows, ExitProcess ends every other thread first and then runs the destructors of the
// libraries' statics on the main thread; a destructor that waits for a thread that is already gone, or for a lock it held, waits for ever and the program can't be closed.

#include "core/export.h"

#include <QtGlobal>

#include <functional>
#include <vector>

namespace jp {

// A step stops what it owns and waits at most `budgetMs` milliseconds for it; it returns how many of its threads it had to let go of (0: all stopped).
using ShutdownStep = std::function<int(int budgetMs)>;

// Steps run in the opposite order of their adding, so what was made last is stopped first. Safe to call from any thread.
JO_COMMON_EXPORT void addShutdownStep(const ShutdownStep &step);

// Steps that must come after the pool every program shares has been waited for: what writes the settings, so that nothing is set after the last write. Run in the opposite order too.
JO_COMMON_EXPORT void addLastShutdownStep(const ShutdownStep &step);

// Runs every step, then the pool of threads every program shares is waited for, then the last steps (the settings are written). Called by ShutdownGuard (below) when main is left, on the
// main thread, and safe to call twice (a step runs once). Every wait is bounded by a step's own cap (2 s) and a cap for the whole (6 s), except that a step is only ever given the time
// that is left: a step that overruns is let go of, and a thread let go of is counted in the answer. Returns how many threads had to be let go of. When JP_SHUTDOWN_REPORT names a file
// and this run keeps its settings apart (a test run: settingsKeptApart()), the result is written to it ("threads=N steps=K", one line: the threads let go of, and the steps that ran) for
// the tests of the programs; a person's run never writes it.
JO_COMMON_EXPORT int shutdownServices();

// Runs `work` on a thread of its own and waits for it at most `budgetMs`; true when it finished. A thread that has not finished is left running (and its object kept: never deleted
// while it runs), for the step that used it to count as one let go of.
JO_COMMON_EXPORT bool runWithinTime(const std::function<void()> &work, int budgetMs);

// Ends the program's services however main() is left, by any return: declare it right after the application object (it runs before the application object goes).
struct ShutdownGuard {
    ShutdownGuard() = default;
    ShutdownGuard(const ShutdownGuard &) = delete;
    ShutdownGuard &operator=(const ShutdownGuard &) = delete;
    ~ShutdownGuard() { shutdownServices(); }
};

// Runs the program (what main used to be) and leaves it. When any step had to let go of a thread, no static object is destroyed: a thread let go of may still be at work, and a
// static a library destroys at the end could be used by it (and on Windows a destructor that waits for it is stuck for ever). What must be written has been (the settings by the last
// step, the recovery copies when the event loop ended, the clipboard when the application object went), so the program leaves at once with its own code, after the output is flushed.
JO_COMMON_EXPORT int runProgram(const std::function<int()> &program);

// For the tests of this machinery: runs `steps` (in the order they were added) the way shutdownServices() does, with a clock that is counted by the caller (milliseconds) so that no
// test waits. A step is given the time left: at most `stepMs`, and at most what remains of `wholeMs`. `given`, when not null, gets each budget given. Returns the threads let go of.
struct ShutdownLimits {
    int wholeMs = 6000;
    int stepMs = 2000;
};
JO_COMMON_EXPORT int runShutdownSteps(const std::vector<ShutdownStep> &steps, const ShutdownLimits &limits, const std::function<qint64()> &nowMs, std::vector<int> *given = nullptr);

// Whether any step has let go of a thread so far in this run.
JO_COMMON_EXPORT bool shutdownLetGo();

} // namespace jp
