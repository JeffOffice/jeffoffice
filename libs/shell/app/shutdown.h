#pragma once
// How a program ends, and what may be left running when it does (developer note: docs/shutdown.md).
//
// THE RULE for anything that lives as long as the program and owns a thread, a pool of threads, a file watcher, a timer on another thread, a media player, a network manager,
// or a child process (the five kinds of thing that wait for another thread when they are destroyed):
//   (a) it is stopped on purpose, by a step added with addShutdownStep(), which each program's main runs through shutdownServices() when exec() has returned and before the
//       application object goes; the step waits for a counted time at most and then lets go of what has not stopped; or
//   (b) it is made with `new` and never deleted, so that no destructor of a static ever runs it.
// No static (a namespace-scope object, a function-local static, a Q_GLOBAL_STATIC) has a destructor that waits for another thread, locks against one, or signals one.
// tools/check_static_threads.py keeps new ones out. The reason: when a program ends on Windows, ExitProcess ends every other thread first and then runs the destructors of the
// libraries' statics on the main thread; a destructor that waits for a thread that is already gone, or for a lock it held, waits for ever and the program can't be closed.

#include "core/export.h"

#include <functional>

namespace jp {

// A step stops what it owns and waits at most `budgetMs` milliseconds for it; it returns how many of its threads it had to let go of (0: all stopped).
using ShutdownStep = std::function<int(int budgetMs)>;

// Steps run in the opposite order of their adding, so what was made last is stopped first. Safe to call from any thread.
JO_COMMON_EXPORT void addShutdownStep(const ShutdownStep &step);

// Runs every step, then the program's own: the pool of threads every program shares, then the settings are written. Called by main() when exec() has returned, on the main thread.
// Every wait is bounded: the whole takes a few seconds at the most. Returns how many threads had to be let go of. When JP_SHUTDOWN_REPORT names a file, the result is written to it
// ("threads=N steps=K", one line: the threads let go of, and the steps that ran) for the tests of the programs.
JO_COMMON_EXPORT int shutdownServices();

} // namespace jp
