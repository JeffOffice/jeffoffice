#!/usr/bin/env python3
"""Checks that no object with static storage duration owns a thread, a file watcher, a timer, a media player, a network manager, or a child process.

When a program ends on Windows, ExitProcess ends every other thread first and then runs the destructors of the libraries' statics on the main thread. A destructor that waits for a
thread that is already gone (QThread::wait, QThreadPool, QFileSystemWatcher, a join), or for a lock that thread held, waits for ever, and the program can't be closed (Nov 23, 2026:
the speller's file watcher). The rule (libs/shell/app/shutdown.h, docs/shutdown.md): such an object is made with `new` and never deleted, or is stopped by a step of jp::shutdownServices().

Reported: a static or namespace-scope object, a static data member (in the class, or defined outside it as `QThreadPool X::pool;`), a `static T x(args);`, or a Q_GLOBAL_STATIC of one of the
types below, or of a struct or class that has a member of one of them (found in the same run, to any depth: `struct Holder { QThreadPool pool; }; static Holder h;`), unless it is a pointer or a reference (made with `new` or not: it destroys nothing), and unless tools/check_static_threads_allowed.txt lists the file with the exact number of uses ("path | count | reason"). A file whose count changes, up or down, is reported
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
# (a QTimer in a class does not make the class own a thread: it runs on the thread of its owner; a static of the class itself is still read for its timers, as before)
BASE_TYPES = TYPES.replace("|QTimer", "")
# A data member (not a pointer, not a reference, not static) of one of the types, inside a struct or class body: what owns one owns its thread.
BODY = re.compile(r"\b(?:struct|class)\s+(\w+)\s*(?:final\s*)?(?::[^{;]*)?\{")
MEMBER_TEMPLATE = r"(?:std::(?:unique_ptr|shared_ptr|optional|vector|map|list|deque)<)?"


def owner_types(files: dict) -> str:
    """The base types and every struct or class that has a member of one of them, to any depth (a fixed point over all the files)."""
    names = set()
    bodies = []
    for text in files.values():
        for m in BODY.finditer(text):
            depth, i = 1, m.end()
            while i < len(text) and depth:
                depth += (text[i] == "{") - (text[i] == "}")
                i += 1
            bodies.append((m.group(1), text[m.end():i - 1]))
    while True:
        alt = "|".join([BASE_TYPES] + sorted(re.escape(n) for n in names))
        member = re.compile(r"^\s*(?!static\b)%s(?:%s)\b[^;()*&<]*\s[\w]+\s*(?:[;={]|$)" % (MEMBER_TEMPLATE, alt), re.M)
        more = {name for name, body in bodies if name not in names and member.search(body)}
        if not more:
            return alt
        names |= more


def patterns(types: str) -> tuple:
    type_re = re.compile(r"\b(?:%s)\b" % types)
    glob = re.compile(r"\bQ_GLOBAL_STATIC(?:_WITH_ARGS)?\s*\(\s*(?:%s)\b" % types)
    # `static [const] T name`, `static T name{...}`, `static T name(args);`, `static std::unique_ptr<T> name`, and the like, on one line. A plain pointer or reference (T *name, T &name) is
    # never destroyed by the object it names: it is no use of this kind (what it points to is made somewhere else, and is looked at there).
    static = re.compile(r"^\s*(?:inline\s+)?static\s+(?!.*\bnew\b)(?:const\s+)?(?:std::(?:unique_ptr|shared_ptr|optional|vector|map)<[^;=(*&]*>|[\w:]+)\s+[\w:]+\s*(?:[;={]|\(([^()]*)\)\s*;|$)")
    # an object of the type at namespace scope, or a static data member defined outside its class (`QThreadPool X::pool;`): the line starts in column 0, has no parenthesis before the
    # name (a function), and ends in `;` or `{` or has its arguments in parentheses
    namespace = re.compile(r"^(?:std::(?:unique_ptr|shared_ptr)<)?(?:%s)\b[^;()*&]*\s[\w:]+\s*(?:[;={]|\([^()]*\)\s*;|$)" % types)
    return type_re, glob, static, namespace


def is_call(args) -> bool:
    """`static T f(int a);` declares a function: its parentheses hold a type and a name; an object's hold values."""
    return args is not None and re.search(r"[\w>&*]\s+[&*]*\w+\s*(?:=[^,]*)?(?:,|$)", args.strip()) is not None and bool(args.strip())


def allowed() -> dict:
    out = {}
    for line in (ROOT / "tools" / "check_static_threads_allowed.txt").read_text(encoding="utf-8").splitlines():
        if line.strip() and not line.startswith("#"):
            path, count, reason = (part.strip() for part in line.split(" | ", 2))
            out[path] = (int(count), reason)
    return out


def uses_in(path: pathlib.Path, text: str, pats: tuple) -> list:
    type_re, glob, static, namespace = pats
    found = []
    lines = text.split("\n")
    for number, line in enumerate(lines, 1):
        m = static.match(line)
        if glob.search(line):
            found.append((number, line.strip()))
        elif m and type_re.search(line.split("=", 1)[0].split("(", 1)[0]) and "new " not in line and not is_call(m.group(1)):
            found.append((number, line.strip()))
        elif path.suffix == ".cpp" and namespace.match(line) and "new " not in line:
            found.append((number, line.strip()))
    return found


def main() -> int:
    ok = allowed()
    problems = []
    seen = set()
    texts = {}
    for top in ("libs", "apps"):
        for path in sorted((ROOT / top).rglob("*")):
            if path.suffix in (".cpp", ".h", ".inc") and "third_party" not in path.parts:
                texts[path] = blanked(path.read_text(encoding="utf-8", errors="replace"))
    pats = patterns(owner_types(texts))
    for path, text in texts.items():
        rel = path.relative_to(ROOT).as_posix()
        found = uses_in(path, text, pats)
        if not found and rel not in ok:
            continue
        seen.add(rel)
        want = ok.get(rel, (0, ""))[0]
        if len(found) != want:
            for number, text in found:
                problems.append("%s:%d: static object that owns or waits for a thread: %s" % (rel, number, text))
            if not found or len(found) != want:
                problems.append("%s: tools/check_static_threads_allowed.txt says %d, the file has %d" % (rel, want, len(found)))
    # every program's main() ends the services on every way out: the guard right after the application object, and the run left through jp::runProgram
    for main_file in sorted((ROOT / "apps").glob("*/main.cpp")):
        text = main_file.read_text(encoding="utf-8", errors="replace")
        rel = main_file.relative_to(ROOT).as_posix()
        if "QApplication app(" in text and not (re.search(r"QApplication app\(argc, argv\);\s*\n\s*jp::ShutdownGuard\b", text) and "jp::runProgram(" in text):
            problems.append("%s: main() needs `jp::ShutdownGuard` on the line after the application object, and `return jp::runProgram(...)` (libs/shell/app/shutdown.h)" % rel)
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
