#!/usr/bin/env python3
"""Checks that libs/ and apps/ start no thread through the C++ library, use no thread_local that is not listed, and ask the system for no locale outside jp::systemLocale.

std::thread, std::jthread, and std::async start a thread that, on Windows (MinGW), frees the
storage of its thread_local objects before their destructors run: a worker that had used one
crashed as it ended (libs/base/core/workthread.h has the story). Our threads are QThreads:
jp::WorkThread and jp::runDetached of that header, QThreadPool, or QtConcurrent. The number of
processors is QThread::idealThreadCount(). The test programs follow the same rule (tests/).

thread_local is reported wherever it is written in our own code (comments and strings aside), unless tools/check_thread_local_allowed.txt lists the file with the exact number of uses
("path | count | reason"): an object with a destructor (a cache, a std::function, a QString) is what crashed at a worker's end on Windows, so new code passes a value down or keeps the
object in the caller. A file whose count changes, up or down, is reported until its line is corrected.

The system's locale (QLocale::system(), a default-constructed QLocale() while the default is the system's, QDate::toString(Qt::TextDate), a date pattern given to
QDate/QTime/QDateTime::toString, localeAwareCompare, a default QCollator, %L in a string, QLocale::uiLanguages) is asked of the operating system at each use, and that is not safe on a worker
thread beside another (on Windows it crashed inside QSystemLocale::query, Nov 20, 2026). Code that can run on a worker takes the person's locale from jp::systemLocale()
(libs/base/core/syslocale.h, asked once on the main thread) and writes dates of a fixed shape with jp::fixedText. Only the main thread's widget code may use Qt's own: see ALLOWED below.

    python3 tools/check_threads.py

Prints each use with its file and line and exits 1, or prints "threads ok" and exits 0.
"""
import fnmatch
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
USE = re.compile(r"\bstd::(thread|jthread|async)\b")
TLS = re.compile(r"\bthread_local\b")

# The system's locale, by any route.
LOCALE = re.compile(
    r"QLocale::system\s*\(|\bQSystemLocale\b|Qt::(SystemLocale\w*|LocalDate|DefaultLocale\w*|TextDate)\b|\bQLocale\s*\(\s*\)|\bQLocale\s+(?!m_)\w+(?<!_)\s*(;|\{\s*\})"
    r"|\blocaleAwareCompare\b|\bQCollator\s+(?!m_)\w+(?<!_)\s*(;|\{\s*\})|\bQCollator\s*\(\s*\)|\buiLanguages\s*\(|\bQLocale::setDefault\b|%L\d"
    r"|\.toString\(\s*(QStringLiteral\()?\"[^\"]*(yy|MM|dd|HH|hh|mm|ss)")
# A date, a time, or a date and time written with no format, or with one that is not a literal, is written in the system's locale: the receiver's name says what it is (a heuristic: names that
# hold a date or a time; QString::toString and the like are not matched).
DATEISH = re.compile(r"\b\w*(?:[Dd]ate|[Tt]ime|[Dd]ay|[Ww]hen|[Ss]tamp|[Mm]odified|[Cc]reated)\w*(?:\(\))?\.toString\(\s*(?:\)|(?!Qt::|QStringLiteral|QLatin1|\"|QLocale|loc|locale)[A-Za-z_])")
# A file that starts a thread of its own is never excused by a folder below: what the thread's body calls is in it.
STARTS = re.compile(r"QThread::create|\bWorkThread\b|\brunDetached\b|QThreadPool|QtConcurrent::")

# Where the system's locale may be used: one line for each FILE in tools/check_threads_allowed.txt ("path | reason"), never a folder.
# A file that a thread-starting file includes (the thread's body may call into it) must say "main thread only, checked: <how>". The heuristic: the stem of the file's name equals the stem of
# a header that a file matching STARTS includes, or of that file itself.
INCLUDE = re.compile(r'#include\s+"([^"]+)"')


def load_allowed() -> dict:
    out = {}
    for line in (ROOT / "tools" / "check_threads_allowed.txt").read_text(encoding="utf-8").splitlines():
        if line.strip() and not line.startswith("#"):
            path, _, reason = line.partition(" | ")
            out[path.strip()] = reason.strip()
    return out


def code_of(line: str) -> str:
    """The line without a // comment (a string with // in it is rare here and only hides a use)."""
    return line.split("//", 1)[0]


def load_thread_local_allowed() -> dict:
    """path -> (count, reason) from tools/check_thread_local_allowed.txt."""
    out = {}
    for line in (ROOT / "tools" / "check_thread_local_allowed.txt").read_text(encoding="utf-8").splitlines():
        if line.strip() and not line.startswith("#"):
            path, count, reason = (part.strip() for part in line.split(" | ", 2))
            out[path] = (int(count), reason)
    return out


RAW = re.compile(r'R"([^()\\ ]{0,16})\(')


def blanked(text: str) -> str:
    """The text with comments and the insides of string and character literals replaced by spaces (new lines kept), so that a word in them is not code."""
    out, i, n = [], 0, len(text)
    keep_lines = lambda part: "".join("\n" if ch == "\n" else " " for ch in part)
    while i < n:
        c = text[i]
        two = text[i:i + 2]
        raw = RAW.match(text, i) if c == "R" and (i == 0 or not (text[i - 1].isalnum() or text[i - 1] == "_")) else None
        if two == "//":
            j = text.find("\n", i)
            j = n if j < 0 else j
            out.append(" " * (j - i))
            i = j
        elif two == "/*":
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append(keep_lines(text[i:j]))
            i = j
        elif raw:
            j = text.find(")" + raw.group(1) + '"', i)
            j = n if j < 0 else j + len(raw.group(1)) + 2
            out.append(keep_lines(text[i:j]))
            i = j
        elif c in "\"'" and not (c == "'" and i > 0 and text[i - 1].isalnum()):   # (a digit separator, 1'000, is no literal)
            j = i + 1
            while j < n and text[j] != c and text[j] != "\n":
                j += 2 if text[j] == "\\" else 1
            j = min(j + 1, n)
            out.append(keep_lines(text[i:j]))
            i = j
        else:
            out.append(c)
            i += 1
    return "".join(out)


def thread_locals(texts: dict) -> list:
    """Messages for each thread_local in our own code that tools/check_thread_local_allowed.txt does not account for."""
    table, found, msgs = load_thread_local_allowed(), {}, []
    for rel, text in texts.items():
        lines = text.splitlines()
        for number, line in enumerate(blanked(text).splitlines(), 1):
            if TLS.search(line):
                found.setdefault(rel, []).append(f"{rel}:{number}: {lines[number - 1].strip()[:120]}")
    for rel, uses in found.items():
        if rel not in table:
            msgs += uses
        elif table[rel][0] != len(uses):
            msgs += [f"{rel}: {len(uses)} thread_local, tools/check_thread_local_allowed.txt says {table[rel][0]}: correct the line (a new use must go, not be listed)"] + uses
    msgs += [f"{rel}: listed in tools/check_thread_local_allowed.txt but has no thread_local: take the line out" for rel in sorted(set(table) - set(found))]
    return msgs


def main() -> int:
    bad, locale_bad, held = [], [], 0
    table = load_allowed()
    used = set()
    texts = {}
    for top in ("libs", "apps"):
        for path in sorted((ROOT / top).rglob("*")):
            if path.suffix in (".cpp", ".h", ".mm", ".inc") and "third_party" not in path.parts:
                texts[str(path.relative_to(ROOT))] = path.read_text(encoding="utf-8", errors="replace")
    tl_texts = {}
    reached = set()   # stems of files that a thread-starting file is, or includes
    for rel, text in texts.items():
        if STARTS.search(text) and not rel.endswith(("syslocale.cpp", "syslocale.h")):
            reached.add(pathlib.PurePosixPath(rel).stem)
            reached.update(pathlib.PurePosixPath(m.group(1)).stem for m in INCLUDE.finditer(text))
    for top in ("libs", "apps", "tests"):
        for path in sorted((ROOT / top).rglob("*")):
            if path.suffix not in (".cpp", ".h", ".mm", ".inc") or "third_party" in path.parts:
                continue
            text = path.read_text(encoding="utf-8", errors="replace")
            rel = str(path.relative_to(ROOT))
            tl_texts[rel] = text
            starts = bool(STARTS.search(text)) and not rel.endswith(("syslocale.cpp", "syslocale.h"))
            if rel.endswith(("syslocale.cpp", "syslocale.h")):
                ok_locale = True
            elif top == "tests":
                ok_locale = True
            elif starts:
                ok_locale = False   # (a file that starts a thread is never excused: what the thread's body calls is in it)
            elif rel in table:
                ok_locale = True
                if pathlib.PurePosixPath(rel).stem in reached and not table[rel].startswith("main thread only, checked:"):
                    locale_bad.append(f"{rel}: called from a file that starts a thread: its line in tools/check_threads_allowed.txt must begin \"main thread only, checked: <how>\"")
            else:
                ok_locale = False
            for number, line in enumerate(text.splitlines(), 1):
                code = code_of(line)
                if USE.search(code):
                    bad.append(f"{rel}:{number}: {line.strip()[:120]}")
                elif LOCALE.search(code) or DATEISH.search(code):
                    if ok_locale:
                        held += 1
                        used.add(rel)
                    else:
                        locale_bad.append(f"{rel}:{number}: {line.strip()[:120]}")
    locale_bad += [f"{rel}: listed in tools/check_threads_allowed.txt but uses the system's locale nowhere: take the line out" for rel in sorted(set(table) - used)]
    tl_bad = thread_locals(tl_texts)
    if tl_bad:
        print("\n".join(tl_bad))
        print(f"{len(tl_bad)} problem(s): thread_local (an object with a destructor crashes at a worker's end on Windows, libs/base/core/workthread.h): pass the value down, or keep it in the caller")
        return 1
    if bad:
        print("\n".join(bad))
        print(f"{len(bad)} problem(s): the C++ library's threads: use jp::WorkThread or jp::runDetached (libs/base/core/workthread.h)")
        return 1
    if locale_bad:
        print("\n".join(locale_bad))
        print(f"{len(locale_bad)} problem(s): the system's locale outside the main thread's widget code: use jp::systemLocale() or jp::fixedText (libs/base/core/syslocale.h)")
        return 1
    print(f"threads ok ({held} uses of the system's locale in main thread code, listed in ALLOWED)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
