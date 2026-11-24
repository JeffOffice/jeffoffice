#!/usr/bin/env python3
"""Checks that no object with static storage duration owns a thread, a file watcher, a timer, a media player, a network manager, or a child process.

When a program ends on Windows, ExitProcess ends every other thread first and then runs the destructors of the libraries' statics on the main thread. A destructor that waits for a
thread that is already gone (QThread::wait, QThreadPool, QFileSystemWatcher, a join), or for a lock that thread held, waits for ever, and the program can't be closed (Nov 23, 2026:
the speller's file watcher). The rule (libs/shell/app/shutdown.h, docs/shutdown.md): such an object is made with `new` and never deleted, or is stopped by a step of jp::shutdownServices().

Reported: a static or namespace-scope object, a static data member, or a Q_GLOBAL_STATIC of one of the types below, unless it is a pointer or a reference (made with `new` or not: it destroys nothing), and unless tools/check_static_threads_allowed.txt lists the file with the exact number of uses ("path | count | reason"). A file whose count changes, up or down, is reported
until its line is corrected.

    python3 tools/check_static_threads.py

Prints each use with its file and line and exits 1, or prints "static threads ok" and exits 0.
"""
import pathlib
import re
import sys

sys.dont_write_bytecode = True
sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
from check_threads import blanked  # noqa: E402  (the comment and string blanking)

ROOT = pathlib.Path(__file__).resolve().parent.parent
# What owns a thread or waits for one when it is destroyed, and our own classes found to do so (Speller: its watcher; SpellingWatch; Janitor: JeffCrunch's loader threads).
TYPES = r"QThreadPool|QFileSystemWatcher|QThread|jp::WorkThread|WorkThread|QTimer|QMediaPlayer|QNetworkAccessManager|QProcess|std::thread|std::jthread|std::future|QFuture|Speller|SpellingWatch|Janitor|GrammarCheck"
TYPE = re.compile(r"\b(?:%s)\b" % TYPES)
GLOBAL = re.compile(r"\bQ_GLOBAL_STATIC(?:_WITH_ARGS)?\s*\(\s*(?:%s)\b" % TYPES)
# `static [const] T name`, `static T name{...}`, `static std::unique_ptr<T> name`, and the like, on one line. A plain pointer or reference (T *name, T &name) is never destroyed by the
# object it names: it is no use of this kind (what it points to is made somewhere else, and is looked at there).
STATIC = re.compile(r"^\s*(?:inline\s+)?static\s+(?!.*\bnew\b)(?:const\s+)?(?:std::(?:unique_ptr|shared_ptr|optional|vector|map)<[^;=(*&]*>|[\w:]+)\s+[\w:]+\s*(?:[;={]|$)")
# an object of the type at namespace scope: the line starts in column 0, has no parenthesis before the name (a function), and ends in `;` or `{`
NAMESPACE = re.compile(r"^(?:std::(?:unique_ptr|shared_ptr)<)?(?:%s)\b[^;()*&]*\s[\w]+\s*(?:[;={]|$)" % TYPES)


def allowed() -> dict:
    out = {}
    for line in (ROOT / "tools" / "check_static_threads_allowed.txt").read_text(encoding="utf-8").splitlines():
        if line.strip() and not line.startswith("#"):
            path, count, reason = (part.strip() for part in line.split(" | ", 2))
            out[path] = (int(count), reason)
    return out


def uses_in(path: pathlib.Path) -> list:
    found = []
    lines = blanked(path.read_text(encoding="utf-8", errors="replace")).split("\n")
    for number, line in enumerate(lines, 1):
        if GLOBAL.search(line):
            found.append((number, line.strip()))
        elif STATIC.match(line) and TYPE.search(line.split("=", 1)[0].split("(", 1)[0]) and "new " not in line:
            found.append((number, line.strip()))
        elif path.suffix == ".cpp" and NAMESPACE.match(line) and "new " not in line:
            found.append((number, line.strip()))
    return found


def main() -> int:
    ok = allowed()
    problems = []
    seen = set()
    for top in ("libs", "apps"):
        for path in sorted((ROOT / top).rglob("*")):
            if path.suffix not in (".cpp", ".h", ".inc") or "third_party" in path.parts:
                continue
            rel = path.relative_to(ROOT).as_posix()
            found = uses_in(path)
            if not found and rel not in ok:
                continue
            seen.add(rel)
            want = ok.get(rel, (0, ""))[0]
            if len(found) != want:
                for number, text in found:
                    problems.append("%s:%d: static object that owns or waits for a thread: %s" % (rel, number, text))
                if not found or len(found) != want:
                    problems.append("%s: tools/check_static_threads_allowed.txt says %d, the file has %d" % (rel, want, len(found)))
    for rel in ok:
        if rel not in seen:
            problems.append("%s: listed in tools/check_static_threads_allowed.txt, but not found" % rel)
    if problems:
        print("\n".join(problems))
        print("A static must not own a thread: make it with new and never delete it, or end it in a step of jp::shutdownServices() (libs/shell/app/shutdown.h).")
        return 1
    print("static threads ok")
    return 0


if __name__ == "__main__":
    sys.exit(main())
