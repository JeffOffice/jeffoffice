# How a program ends

A program that has background threads must end them on purpose. Nothing may be left for the end of the process to clean up.

## The rule

Anything that lives as long as the program and owns a thread, a pool of threads, a file watcher, a timer on another thread, a media player, a network manager, or a child process is one of these:

1. **Stopped by a step.** The owner calls `jp::addShutdownStep()` (libs/shell/app/shutdown.h) when it starts. Each program's `main()` calls `jp::shutdownServices()` when `exec()` has returned, before the application object goes. A step stops what it owns, waits a counted time at most, and lets go of what has not stopped. Steps run in the opposite order of their adding.
2. **Made with `new` and never deleted.** No destructor of a static runs it.

No static object (a namespace-scope object, a function-local static, a `Q_GLOBAL_STATIC`) has a destructor that waits for another thread, locks against one, or signals one.

## Why

On Windows, `ExitProcess` ends every other thread first and then runs the destructors of the libraries' statics on the main thread. A destructor that waits for a thread that is already gone (`QThread::wait`, a thread pool, `QFileSystemWatcher`, a join), or for a lock that thread held, waits for ever, and the program can't be closed. The speller's file watcher was the first one found. `QObject` and `QSettings` destructors take Qt's own global locks, so the program's long-lived `QObject` singletons and its settings are kept alive the same way.

## What is where

| Owner | How it ends |
| --- | --- |
| Settings (libs/shell/app/settings.cpp) | made with `new`; a step writes it |
| Setting notifiers (same file) | made with `new` |
| Spelling watch (libs/shell/app/spelling.cpp) | a step deletes it on the main thread |
| JeffCrunch's loader threads (apps/crunch/core/loader.cpp) | made with `new`; a step stops each and waits |
| Recovery copies (libs/shell/app/recovery.cpp) | `aboutToQuit` calls `endSession()`; the pool is made with `new` |
| Translator (libs/translate) | `aboutToQuit` calls `shutdownTranslation()`; the engine is made with `new` |
| The pool every program shares | `shutdownServices()` waits for it for a counted time |
| Header and footer pictures (libs/shell/grid/hfpicturecache.cpp) | made with `new` |

## Checks

- `python3 tools/check_static_threads.py` refuses a new static of a thread-owning type. A file that must have one is listed in tools/check_static_threads_allowed.txt with the exact count and the reason.
- tests/shell/tst_shell_exit.cpp (`exittests`) starts each program, closes its window the way a person does, and requires the process to be gone within a counted number of waits, twenty times for each program. A program started with `JP_CLOSE_AFTER_TURNS=N` closes its window after N turns of its event loop; `JP_SHUTDOWN_REPORT=<file>` makes `shutdownServices()` write `threads=N steps=K` there (N: threads let go of; K: steps that ran).
