#!/usr/bin/env python3
"""Compares, for each workbook in a list, what the reader gets from the .xls with what it gets from the same workbook saved as .xlsx by another program, cell by cell.

    python3 tools/crunch_xls_compare.py build/jcrunchtool pairs.tsv out-dir [jobs]

pairs.tsv has one line for each workbook: the path of the .xls, a tab, and the path of the .xlsx (paths may have spaces; a line whose .xlsx is missing is counted as not
converted). Each pair runs `jcrunchtool compare-one` in a child process under `systemd-run --user --scope -p MemoryMax=4G` with 120 seconds. out-dir/diffs.tsv has a line for
each difference the tool printed (the first 60 of each kind in each workbook): workbook, sheet, cell, kind, ours, theirs; out-dir/books.tsv has the counts of each workbook.
The summary is printed: the share of cells whose values are the same (constants, formula cells, and both), of formulas whose text is the same, and of number formats that show
alike, and the commonest differences of each kind. Kinds: V a constant's value, W a formula's value, F a formula's text, N a number format, A a cell only ours has, B a cell
only theirs has.
"""
import os
import re
import subprocess
import sys
import tempfile
from collections import Counter
from concurrent.futures import ThreadPoolExecutor

LIMIT_SECONDS = 120
FIELDS = ["sheetsA", "sheetsB", "namesAgree", "inEither", "inBoth", "onlyOurs", "onlyTheirs", "constSame", "constNear", "constDiff", "fvalSame", "fvalNear", "fvalDiff", "ftextSame", "ftextDiff", "fmtSame", "fmtDiff"]


def unescape(text):
    return re.sub(r"\\([tnr\\])", lambda m: {"t": "\t", "n": "\n", "r": "\r", "\\": "\\"}[m.group(1)], text)


def run_one(tool, ours, theirs):
    command = ["systemd-run", "--user", "--scope", "-p", "MemoryMax=4G", "-p", "MemorySwapMax=0", "-q", "timeout", "-k", "5", str(LIMIT_SECONDS), tool, "compare-one", ours, theirs]
    env = dict(os.environ, JP_NO_TELEMETRY="1", QT_QPA_PLATFORM="offscreen")
    with tempfile.TemporaryFile() as errors:
        child = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=errors, env=env)
        output = child.stdout.read().decode("utf-8", "replace")
        child.wait()
    return child.returncode, output


def shape(value):
    """What a value looks like, not what it is: for counting the kinds of difference."""
    if value == "":
        return "(none)"
    if value.startswith("n:"):
        return "number"
    if value.startswith("s:"):
        return "text" if value != "s:" else "empty text"
    return value[:2] if value[:2] in ("b:", "e:") else value[:12]


def function_of_difference(ours, theirs):
    """The first function name that the two formulas do not both have, or the first characters where they part."""
    names = lambda f: re.findall(r"([A-Za-z_][A-Za-z0-9_.]*)\(", f)
    a, b = Counter(names(ours.upper())), Counter(names(theirs.upper()))
    extra = list((a - b).keys()) + list((b - a).keys())
    if extra:
        return "function " + extra[0]
    i = 0
    while i < min(len(ours), len(theirs)) and ours[i] == theirs[i]:
        i += 1
    return "text differs near: " + ours[max(0, i - 6):i + 10].replace(" ", "") + " | " + theirs[max(0, i - 6):i + 10].replace(" ", "")


def main():
    if len(sys.argv) < 4:
        print(__doc__)
        return 2
    tool, pairs_file, out_dir = sys.argv[1:4]
    jobs = int(sys.argv[4]) if len(sys.argv) > 4 else 8
    os.makedirs(out_dir, exist_ok=True)
    pairs = [l.rstrip("\n").split("\t") for l in open(pairs_file) if l.strip()]
    results = [None] * len(pairs)

    def work(i):
        ours, theirs = pairs[i]
        if not os.path.exists(theirs):
            results[i] = ("missing", "")
            return
        code, output = run_one(tool, ours, theirs)
        results[i] = (code, output)

    with ThreadPoolExecutor(jobs) as ex:
        list(ex.map(work, range(len(pairs))))
    totals = Counter()
    diffs = open(os.path.join(out_dir, "diffs.tsv"), "w")
    books = open(os.path.join(out_dir, "books.tsv"), "w")
    kinds = {k: Counter() for k in "VWFNAB"}
    not_opened = Counter()
    books_total = books_equal = books_same_values = 0
    for (ours, theirs), (code, output) in zip(pairs, results):
        if code == "missing":
            totals["not converted"] += 1
            continue
        lines = output.split("\n")
        summary = None
        for line in lines:
            if line.startswith("s\t"):
                summary = [int(x) for x in line.split("\t")[1:]]
            elif line.startswith("d\t"):
                f = line.split("\t")
                if len(f) >= 6:
                    diffs.write(ours + "\t" + "\t".join(f[1:]) + "\n")
                    kind = f[3]
                    o, t = unescape(f[4]), unescape(f[5])
                    if kind in "VW":
                        kinds[kind][shape(o) + " -> " + shape(t)] += 1
                    elif kind == "F":
                        kinds[kind][function_of_difference(o, t)] += 1
                    elif kind == "N":
                        kinds[kind][o + " | " + t] += 1
                    else:
                        kinds[kind][shape(o if kind == "A" else t)] += 1
            elif line.startswith("!"):
                not_opened[line.split("\t")[0] + ": " + (line.split("\t")[1][:60] if "\t" in line else "")] += 1
        if summary is None:
            totals["crashed or timed out"] += 1
            books.write(ours + "\t" + str(code) + "\n")
            continue
        books_total += 1
        d = dict(zip(FIELDS, summary))
        for k, v in d.items():
            totals[k] += v
        books.write(ours + "\t" + str(code) + "\t" + "\t".join(str(x) for x in summary) + "\n")
        if d["onlyOurs"] + d["onlyTheirs"] + d["constDiff"] + d["fvalDiff"] == 0:
            books_same_values += 1
    diffs.close()
    books.close()
    t = totals
    pct = lambda a, b: "%.2f%%" % (100.0 * a / b) if b else "n/a"
    print("workbooks", len(pairs), "compared", books_total, "not converted", t["not converted"], "crashed or timed out", t["crashed or timed out"], "not opened", sum(not_opened.values()))
    for k, v in not_opened.most_common(6):
        print("   ", v, k)
    print("cells with a value or a formula in either file:", t["inEither"], "; in both:", t["inBoth"], "; only ours:", t["onlyOurs"], "; only theirs:", t["onlyTheirs"])
    const_total = t["constSame"] + t["constNear"] + t["constDiff"]
    formula_total = t["fvalSame"] + t["fvalNear"] + t["fvalDiff"]
    exact = t["constSame"] + t["fvalSame"]
    near = exact + t["constNear"] + t["fvalNear"]
    print("values the same to 15 digits: constants %s of %d, formula cells %s of %d, both kinds %s of the %d cells in both, %s of the %d cells in either" % (
        pct(t["constSame"], const_total), const_total, pct(t["fvalSame"], formula_total), formula_total, pct(exact, t["inBoth"]), t["inBoth"], pct(exact, t["inEither"]), t["inEither"]))
    print("... or within two units of the 15th digit (the XML form written by LibreOffice holds 15 digits, rounded its own way): constants %s, formula cells %s, both kinds %s of the cells in both, %s of the cells in either (%d constants and %d formula cells are of that kind)" % (
        pct(t["constSame"] + t["constNear"], const_total), pct(t["fvalSame"] + t["fvalNear"], formula_total), pct(near, t["inBoth"]), pct(near, t["inEither"]), t["constNear"], t["fvalNear"]))
    print("formula text the same: %s of %d; number formats that show alike: %s of %d" % (pct(t["ftextSame"], t["ftextSame"] + t["ftextDiff"]), t["ftextSame"] + t["ftextDiff"],
                                                                                         pct(t["fmtSame"], t["fmtSame"] + t["fmtDiff"]), t["fmtSame"] + t["fmtDiff"]))
    print("workbooks whose cells and values are all the same:", books_same_values, "of", books_total, pct(books_same_values, books_total))
    names = {"V": "constants with a different value", "W": "formula cells with a different value", "F": "formulas with different text", "N": "number formats that show otherwise",
             "A": "cells only ours has", "B": "cells only theirs has"}
    for kind in "VWFNAB":
        total = sum(kinds[kind].values())
        print("commonest %s (among the %d printed):" % (names[kind], total))
        for k, v in kinds[kind].most_common(8):
            print("   %6d  %s" % (v, k))
    return 0


if __name__ == "__main__":
    sys.exit(main())
