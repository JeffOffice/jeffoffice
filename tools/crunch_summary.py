#!/usr/bin/env python3
"""Sums up the table `jcrunchtool scan` wrote, in one line.

    python3 tools/crunch_summary.py out-dir

Workbooks, how many opened (read whole, or opened with a sheet only partly read), crashed (a program stopped, exit 128 and
more), and timed out (exit 124), the cells and the formula cells, the share of formula cells with a result stored, and the
25 most used functions with how many formulas call them (a share of all the calls).
"""
import os
import sys
from collections import Counter


def main():
    if len(sys.argv) != 2:
        print(__doc__)
        return 2
    books = opened = partly = crashed = timed_out = not_opened = 0
    cells = formulas = stored = 0
    functions = Counter()
    with open(os.path.join(sys.argv[1], "books.tsv"), encoding="utf-8", errors="surrogateescape") as f:
        for line in f.read().splitlines()[1:]:
            fields = line.split("\t")
            if len(fields) < 9:
                continue
            books += 1
            code = int(fields[1])
            if code == 124:
                timed_out += 1
            elif code >= 128:
                crashed += 1
            elif code == 1:
                not_opened += 1
            else:
                opened += 1
                partly += code == 2
                cells += int(fields[5])
                formulas += int(fields[6])
                stored += int(fields[7])
                if fields[8] != "-":
                    for pair in fields[8].split(","):
                        name, _, count = pair.rpartition(":")
                        functions[name] += int(count)
    calls = sum(functions.values())
    share = f"{100.0 * stored / formulas:.2f}%" if formulas else "n/a"
    top = ", ".join(f"{n} {c} ({100.0 * c / calls:.1f}%)" for n, c in functions.most_common(25))
    print(f"workbooks {books}, opened {opened}, crashed {crashed}, timed out {timed_out}, cells {cells}, formula cells {formulas}, "
          f"formula cells with a stored result {share}, top 25 functions: {top}; "
          f"[{not_opened} not opened, {partly} of the opened only partly read]")
    return 0


if __name__ == "__main__":
    sys.exit(main())
