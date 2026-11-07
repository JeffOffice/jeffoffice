#!/usr/bin/env python3
"""Checks that the Done, Partly, and Not yet counts in docs/features/README.md match each app's feature list.

    python3 tools/check_feature_counts.py [--fix]

A count is the number of checklist lines in the app's list file: list items that start with [x], [~], or [ ]. The counts are
typed by hand when a list changes, and they drift; --fix writes the right ones into the table. Anything that differs is reported,
and the exit status is 1.
"""
import os, re, sys

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DOCS = os.path.join(HERE, "docs", "features")
ROW = re.compile(r"^(\|[^|]*\|[^|]*\|[^|]*\| )(\d+) \| (\d+) \| (\d+)( \| \[[^\]]*\]\(([^)]+)\) \|)\s*$")
ITEM = re.compile(r"^\s*[-*] \[([x~ ])\]")


def tally(path):
    found = {"x": 0, "~": 0, " ": 0}
    with open(path, encoding="utf-8") as f:
        for line in f:
            m = ITEM.match(line)
            if m:
                found[m.group(1)] += 1
    return found["x"], found["~"], found[" "]


def main():
    fix = "--fix" in sys.argv[1:]
    readme = os.path.join(DOCS, "README.md")
    lines = open(readme, encoding="utf-8").read().split("\n")
    rows = problems = 0
    for i, line in enumerate(lines):
        m = ROW.match(line)
        if not m:
            continue
        rows += 1
        listed = tuple(int(m.group(n)) for n in (2, 3, 4))
        counted = tally(os.path.join(DOCS, m.group(6)))
        if listed != counted:
            problems += 1
            print(f"docs/features/README.md:{i + 1}: {m.group(6)} says {listed[0]} | {listed[1]} | {listed[2]}, its list has {counted[0]} | {counted[1]} | {counted[2]}")
            lines[i] = f"{m.group(1)}{counted[0]} | {counted[1]} | {counted[2]}{m.group(5)}"
    if fix and problems:
        with open(readme, "w", encoding="utf-8") as f:
            f.write("\n".join(lines))
    print(f"feature counts: {rows} rows, {0 if fix else problems} problem(s)")
    return 1 if problems and not fix else 0


if __name__ == "__main__":
    sys.exit(main())
