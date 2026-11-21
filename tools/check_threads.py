#!/usr/bin/env python3
"""Checks that libs/ and apps/ start no thread through the C++ library, and ask the system for no locale outside jp::systemLocale.

std::thread, std::jthread, and std::async start a thread that, on Windows (MinGW), frees the
storage of its thread_local objects before their destructors run: a worker that had used one
crashed as it ended (libs/base/core/workthread.h has the story). Our threads are QThreads:
jp::WorkThread and jp::runDetached of that header, QThreadPool, or QtConcurrent. The number of
processors is QThread::idealThreadCount(). The test programs follow the same rule (tests/).

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

# The system's locale, by any route.
LOCALE = re.compile(
    r"QLocale::system\s*\(|\bQSystemLocale\b|Qt::(SystemLocale\w*|LocalDate|DefaultLocale\w*|TextDate)\b|\bQLocale\s*\(\s*\)|\bQLocale\s+\w+\s*(;|\{\s*\})"
    r"|\blocaleAwareCompare\b|\bQCollator\s+\w+\s*(;|\{\s*\})|\bQCollator\s*\(\s*\)|\buiLanguages\s*\(|\bQLocale::setDefault\b|%L\d"
    r"|\.toString\(\s*(QStringLiteral\()?\"[^\"]*(yy|MM|dd|HH|hh|mm|ss)")
# A file that starts a thread of its own is never excused by a folder below: what the thread's body calls is in it.
STARTS = re.compile(r"QThread::create|\bWorkThread\b|\brunDetached\b|QThreadPool|QtConcurrent::")

# Where the system's locale may be used, each with the reason. A path is a glob from the root; a folder's files are all in it.
ALLOWED = [
    ("libs/base/core/syslocale.*", "the one place that asks the system, on the main thread"),
    # Widget code: QWidget classes, dialogs, and window controllers run on the main thread (Qt's own rule), and show the person's own formats and signs.
    ("apps/*/app/*", "window and dialog code of an app: main thread"),
    ("libs/*/app/*", "window and dialog code of a shared layer: main thread"),
    ("libs/shell/grid/*", "the grid widget and its painting: main thread"),
    # An app's own main thread work that is not widget code: what a window's edit session says about an edit it is asked to make (its refusals name the limits in the person's number
    # format) and reads from what the person types (their own decimal and date formats). The session lives on the main thread; the calculation worker gets the book, not the session.
    ("apps/crunch/core/edit*.cpp", "edit session: plans and refusals made on the main thread"),
    ("apps/crunch/core/editsession.h", "the session's own locale, set on the main thread"),
    ("libs/office/sheet/csv.h", "a member the opener fills from the copy of jp::systemLocale()"),
    ("apps/crunch/core/dataform.cpp", "data form: main thread, reads what the person types"),
    ("apps/crunch/core/validationedit.cpp", "validation editing: main thread, reads what the person types"),
    ("apps/crunch/core/errorcheck.cpp", "error checking of typed text: main thread"),
    ("apps/crunch/core/printjob.cpp", "the print job's labels and tags: made on the main thread"),
    ("apps/crunch/core/spellwalk.cpp", "the proofing language, asked on the main thread before the walk"),
    ("apps/crunch/core/sheetobjects.cpp", "refusal texts of the object edits: main thread"),
    ("apps/crunch/core/advancedfilter.h", "the person's locale as the default of a parse made on the main thread"),
    ("apps/write/core/*ops.cpp", "document edits the window makes: main thread"),
    ("apps/write/core/field*.cpp", "fields are updated by the window: main thread"),
    ("apps/write/core/field*.h", "fields are updated by the window: main thread"),
    ("apps/write/core/templates.cpp", "page size of a new document: main thread"),
    ("apps/write/core/textexport.cpp", "export from the window: main thread"),
    ("apps/write/core/printing.cpp", "printing is made by the window: main thread"),
    ("apps/write/core/writedoc.cpp", "page size of a new document: main thread"),
    ("apps/pub/templates/*", "templates are built by the window: main thread (the threads there only paint pictures)"),
    ("apps/pub/io/importers.cpp", "an import started from the window: main thread"),
    ("apps/deck/io/deckoutput.cpp", "slide output made by the window: main thread"),
    ("libs/office/sheet/entry.*", "typed entry: the main thread's own parse, in the person's own signs"),
    ("libs/office/sheet/headerfooter.h", "member that a print job fills from its main thread"),
    ("libs/page/core/*", "pages and merge sources are made by the window: main thread"),
    ("libs/translate/translate/languages.cpp", "the list of languages for the pane: main thread"),
    ("libs/translate/translate/translator.cpp", "a message made on the main thread"),
    ("libs/shell/app/*", "shell window code: main thread"),
]


def code_of(line: str) -> str:
    """The line without a // comment (a string with // in it is rare here and only hides a use)."""
    return line.split("//", 1)[0]


def allowed(rel: str, text: str) -> bool:
    if STARTS.search(text):
        return rel.endswith(("syslocale.cpp", "syslocale.h"))
    return any(fnmatch.fnmatchcase(rel, pattern) for pattern, _ in ALLOWED)


def main() -> int:
    bad, locale_bad, held = [], [], 0
    for top in ("libs", "apps", "tests"):
        for path in sorted((ROOT / top).rglob("*")):
            if path.suffix not in (".cpp", ".h", ".mm", ".inc") or "third_party" in path.parts:
                continue
            text = path.read_text(encoding="utf-8", errors="replace")
            rel = str(path.relative_to(ROOT))
            ok_locale = top == "tests" or allowed(rel, text)
            for number, line in enumerate(text.splitlines(), 1):
                code = code_of(line)
                if USE.search(code):
                    bad.append(f"{rel}:{number}: {line.strip()[:120]}")
                elif LOCALE.search(code):
                    if ok_locale:
                        held += 1
                    else:
                        locale_bad.append(f"{rel}:{number}: {line.strip()[:120]}")
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
