#!/usr/bin/env python3
"""Every class with Q_OBJECT or Q_GADGET in libs/ must be declared with JO_COMMON_EXPORT (libs/base/core/export.h).

On Windows the shared library jeffoffice-common holds these classes; a program that takes the address of one of their signals or meta objects
gets the same address as the library only when the class is exported by the library and imported by its users.

An inline variable in a header of libs/ is a copy of its own in each program and in the library on Windows, so a program that reads it sees
its own copy, not what the library counted. Each one must say "library-only" in a comment on its line (read only by the library's own code,
or through an exported function). A static that changes, inside an inline function of a header, is the same: such a function goes out of line. Prints one line a problem
and a last line with the count of problems; exits 1 when there are any.
"""
import glob
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def main():
    problems = []
    seen = 0
    for f in sorted(glob.glob(os.path.join(ROOT, "libs", "**", "*.*"), recursive=True)):
        if not f.endswith((".h", ".cpp", ".inc")):
            continue
        t = open(f, encoding="utf8", errors="replace").read()
        t = re.sub(r"//[^\n]*", "", t)
        for m in re.finditer(r"\bQ_(OBJECT|GADGET)\b", t):
            decls = list(re.finditer(r"\b(class|struct)\s+((?:JO_COMMON_EXPORT\s+)?)(\w+)\s*(?:final\s*)?(?::[^{;]*)?\{", t[:m.start()]))
            if not decls:
                continue
            d = decls[-1]
            seen += 1
            if not d.group(2):
                line = t[:d.start()].count("\n") + 1
                problems.append(f"{os.path.relpath(f, ROOT)}:{line}: {d.group(3)} has {m.group(0)} but no JO_COMMON_EXPORT")
    for f in sorted(glob.glob(os.path.join(ROOT, "libs", "**", "*.h"), recursive=True)):
        for n, line in enumerate(open(f, encoding="utf8", errors="replace"), 1):
            if re.match(r"\s*inline\s+(?:thread_local\s+|static\s+|const\s+)*[\w:<>]+\s+\w+\s*(?:=|\{|;)", line) and "library-only" not in line:
                problems.append(f"{os.path.relpath(f, ROOT)}:{n}: an inline variable without a library-only note")
        t = open(f, encoding="utf8", errors="replace").read()
        for m in re.finditer(r"^(?:inline|template\s*<[^>]*>\s*inline)\b[^;{]*?\b(\w+)\s*\([^)]*\)\s*(?:const\s*)?\{", t, re.M):
            i, depth = m.end(), 1
            while depth and i < len(t):
                depth += t[i] == "{"
                depth -= t[i] == "}"
                i += 1
            body = re.sub(r"//[^\n]*", "", t[m.end():i])
            if re.search(r"\bstatic\s+(?!const\b|constexpr\b)", body):
                problems.append(f"{os.path.relpath(f, ROOT)}:{t[:m.start()].count(chr(10)) + 1}: {m.group(1)} is inline and holds a static that changes (move it out of line)")
    print("\n".join(problems))
    print(f"export macro: {seen} classes, {len(problems)} problem(s)")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
