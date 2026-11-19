#!/usr/bin/env python3
"""Checks that libs/ and apps/ start no thread through the C++ library.

std::thread, std::jthread, and std::async start a thread that, on Windows (MinGW), frees the
storage of its thread_local objects before their destructors run: a worker that had used one
crashed as it ended (libs/base/core/workthread.h has the story). Our threads are QThreads:
jp::WorkThread and jp::runDetached of that header, QThreadPool, or QtConcurrent. The number of
processors is QThread::idealThreadCount().

    python3 tools/check_threads.py

Prints each use with its file and line and exits 1, or prints "threads ok" and exits 0.
"""
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
USE = re.compile(r"\bstd::(thread|jthread|async)\b")


def code_of(line: str) -> str:
    """The line without a // comment (a string with // in it is rare here and only hides a use)."""
    return line.split("//", 1)[0]


def main() -> int:
    bad = []
    for top in ("libs", "apps"):
        for path in sorted((ROOT / top).rglob("*")):
            if path.suffix not in (".cpp", ".h", ".mm", ".inc") or "third_party" in path.parts:
                continue
            for number, line in enumerate(path.read_text(encoding="utf-8", errors="replace").splitlines(), 1):
                if USE.search(code_of(line)):
                    bad.append(f"{path.relative_to(ROOT)}:{number}: {line.strip()[:120]}")
    if bad:
        print("\n".join(bad))
        print(f"{len(bad)} use(s) of the C++ library's threads: use jp::WorkThread or jp::runDetached (libs/base/core/workthread.h)")
        return 1
    print("threads ok")
    return 0


if __name__ == "__main__":
    sys.exit(main())
