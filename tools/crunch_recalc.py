#!/usr/bin/env python3
"""Runs `jcrunchtool recalc-one` on each workbook in a list, each in a child process, and compares every formula's worked-out value with the result the file stored.

    python3 tools/crunch_recalc.py build/jcrunchtool keys.txt out-dir [jobs]

keys.txt has one path to a workbook on each line (paths may have spaces). Each workbook runs under `systemd-run --user --scope -p MemoryMax=4G` with 120 seconds, as in
crunch_scan.py. A workbook stores the last result of every formula, so each formula is worked out once, reading every cell it refers to from the values the file stored,
and the answer is compared with the formula's own stored result (15 significant digits for a number; text, true or false, and the kind of an error exactly). This checks
each formula on its own; recalculating a whole workbook from its inputs is another thing.

Kept apart, as no mismatch: formulas that call a function the engine lacks, that call one that changes with no cell changing (NOW, TODAY, RAND, RANDBETWEEN), that refer to
another workbook or to the columns of a table, data tables, formulas that did not parse, formulas with no result stored, and those the work bound or the time stopped.
(OFFSET and INDIRECT are volatile too, but depend only on cells: they are worked out and counted apart.)

out-dir/cells.tsv has a line for each formula cell that did not match, a few of each kind that were kept apart, and a few that matched: workbook, sheet, cell, formula,
stored, ours, class (M match, X mismatch, U unknown function, V volatile, E external, T table columns, P not parsed, N no result, D data table, L work bound, S out of time, R a rate found by iteration).
out-dir/books.tsv has a line for each workbook; out-dir/unsupported.tsv lists the functions the engine lacks, with the cells that call each. The summary is printed: the cells and the share that matched, the workbooks that matched whole, how each function of the
most used fared, the commonest mismatches, and the commonest unsupported functions.
"""
import os
import re
import subprocess
import sys
import tempfile
import time
from collections import Counter
from concurrent.futures import ThreadPoolExecutor

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from crunch_scan import LIMIT_SECONDS, SetupFailed, check_systemd_run, need_linux  # noqa: E402

CLASSES = "MXUVETPNDLSR"


def unescape(text):
    return re.sub(r"\\([tnr\\])", lambda m: {"t": "\t", "n": "\n", "r": "\r", "\\": "\\"}[m.group(1)], text)


def escape(text):
    return text.replace("\\", "\\\\").replace("\t", "\\t").replace("\n", "\\n").replace("\r", "\\r")


def run_one(tool, path, matches, seconds):
    command = ["systemd-run", "--user", "--scope", "-p", "MemoryMax=4G", "-p", "MemorySwapMax=0", "-q",
               "timeout", "-k", "5", str(LIMIT_SECONDS), tool, "recalc-one", path, str(matches), str(seconds)]
    env = dict(os.environ, JP_NO_TELEMETRY="1", QT_QPA_PLATFORM="offscreen")
    with tempfile.TemporaryFile() as errors:
        child = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=errors, env=env)
        output = child.stdout.read()
        _, status, _ = os.wait4(child.pid, 0)
        errors.seek(0)
        message = errors.read(2000).decode("utf-8", "replace").strip().replace("\n", " | ")
    code = os.waitstatus_to_exitcode(status)
    if code < 0:
        code = 128 - code
    if code in (126, 127) or (code == 1 and message and not output and path not in message):
        raise SetupFailed("the run could not start the program (exit %d): %s" % (code, message or "no message"))
    rows, fns, unsup, pats, book = [], [], [], [], None
    for line in output.decode("utf-8", "replace").splitlines():
        f = line.split("\t")
        if f[0] == "row" and len(f) == 8:
            rows.append([unescape(x) for x in f[1:7]] + [f[7]])
        elif f[0] == "fn" and len(f) == 6:
            fns.append((f[1], [int(x) for x in f[2:6]]))
        elif f[0] == "unsup" and len(f) == 3:
            unsup.append((unescape(f[1]), int(f[2])))
        elif f[0] == "pat" and len(f) == 3:
            pats.append((unescape(f[1]), int(f[2])))
        elif f[0] == "book" and len(f) >= 19:
            book = f[1:]
    return path, code, rows, fns, unsup, pats, book, message


def pct(a, b):
    return "%.2f%%" % (100.0 * a / b) if b else "n/a"


def main():
    if len(sys.argv) < 4:
        print(__doc__)
        return 2
    if not need_linux():
        return 2
    tool, keys, out_dir = sys.argv[1:4]
    try:
        check_systemd_run()
    except SetupFailed as why:
        print("crunch_recalc.py: %s" % why, file=sys.stderr)
        return 1
    jobs = int(sys.argv[4]) if len(sys.argv) > 4 else 8
    with open(keys, encoding="utf-8", errors="surrogateescape") as f:
        paths = [line for line in f.read().splitlines() if line]
    print(f"{len(paths)} keys", flush=True)
    os.makedirs(out_dir, exist_ok=True)
    started = time.time()
    results = []
    try:
        with ThreadPoolExecutor(max_workers=jobs) as pool:
            for n, result in enumerate(pool.map(lambda p: run_one(tool, p, 3, 100), paths), 1):
                results.append(result)
                if n % 500 == 0:
                    print(f"{n} of {len(paths)}", file=sys.stderr, flush=True)
    except SetupFailed as why:
        print("crunch_recalc.py: stopped after %d of %d workbooks: %s" % (len(results), len(paths), why), file=sys.stderr)
        pool.shutdown(wait=True, cancel_futures=True)
        return 1
    opened = crashed = timed_out = not_opened = 0
    totals = Counter()
    volatile_checked = volatile_matched = 0
    functions = {}
    unsupported = Counter()
    patterns = Counter()
    formula_books = whole_books = checked_books = 0
    manual_books = manual_mismatch = 0
    with open(os.path.join(out_dir, "cells.tsv"), "w", encoding="utf-8", errors="surrogateescape") as cells, \
            open(os.path.join(out_dir, "books.tsv"), "w", encoding="utf-8", errors="surrogateescape") as books:
        cells.write("workbook\tsheet\tcell\tformula\tstored\tours\tclass\n")
        books.write("workbook\texit\tformula cells\t" + "\t".join(CLASSES) + "\tvolatile checked\tvolatile matched\tmanual\twhole\tseconds\n")
        for path, code, rows, fns, unsup, pats, book, message in results:
            if code == 124:
                timed_out += 1
            elif code >= 128:
                crashed += 1
            elif code == 1:
                not_opened += 1
            else:
                opened += 1
            if book is None:
                books.write("%s\t%d\n" % (escape(path), code))
                continue
            counts = [int(x) for x in book[:13]]   # formula cells, then the 12 classes
            vc, vm, manual, whole, seconds = int(book[13]), int(book[14]), int(book[15]), int(book[16]), book[17]
            books.write("%s\t%d\t%s\t%d\t%d\t%d\t%d\t%s\n" % (escape(path), code, "\t".join(map(str, counts)), vc, vm, manual, whole, seconds))
            totals["formula cells"] += counts[0]
            for k, c in zip(CLASSES, counts[1:]):
                totals[k] += c
            volatile_checked += vc
            volatile_matched += vm
            if counts[0]:
                formula_books += 1
            matched, mismatched = counts[1], counts[2]
            if matched + mismatched:
                checked_books += 1
                if mismatched == 0:
                    whole_books += 1
                if manual:
                    manual_books += 1
                    if mismatched:
                        manual_mismatch += 1
            for name, v in fns:
                t = functions.setdefault(name, [0, 0, 0, 0])
                for i in range(4):
                    t[i] += v[i]
            for name, n in unsup:
                unsupported[name] += n
            for name, n in pats:
                patterns[name] += n
            for sheet, cell, cls, formula, stored, ours, fnlist in rows:
                cells.write("%s\t%s\t%s\t%s\t%s\t%s\t%s\n" % (escape(path), escape(sheet), cell, escape(formula), escape(stored), escape(ours), cls))
    checked = totals["M"] + totals["X"]
    top = sorted(functions.items(), key=lambda kv: -kv[1][0])[:50]
    print(f"recalc: workbooks {len(paths)} (opened {opened}, not opened {not_opened}, crashed {crashed}, timed out {timed_out}); formula cells {totals['formula cells']}, "
          f"checked {checked} of which matched {totals['M']} ({pct(totals['M'], checked)}); kept apart: unknown function {totals['U']}, volatile {totals['V']} "
          f"(OFFSET and INDIRECT worked out: {volatile_checked}, matched {volatile_matched}), another workbook {totals['E']}, table columns {totals['T']}, not parsed {totals['P']}, "
          f"no result stored {totals['N']}, data table {totals['D']}, work bound {totals['L']}, rate found by iteration {totals['R']}, out of time {totals['S']}; workbooks with formulas {formula_books}, "
          f"with something checked {checked_books}, matched whole {whole_books} ({pct(whole_books, checked_books)}); of those on manual calculation {manual_books}, {manual_mismatch} with mismatches; "
          f"{time.time() - started:.0f} s")
    print("functions by calls (calls, share of calls in cells that matched, cells, share of cells that matched):")
    for name, (calls, mcalls, ncells, mcells) in top:
        print("  %-14s %10d %9s %9d %9s" % (name, calls, pct(mcalls, calls), ncells, pct(mcells, ncells)))
    print(f"{len(patterns)} mismatch patterns, the 15 commonest:")
    for name, n in patterns.most_common(15):
        print("  %8d  %s" % (n, name))
    with open(os.path.join(out_dir, "unsupported.tsv"), "w", encoding="utf-8", errors="surrogateescape") as f:
        f.write("function\tcells\n")
        for name, n in unsupported.most_common():
            f.write("%s\t%d\n" % (name, n))
    print("unsupported functions (cells that call them):")
    for name, n in unsupported.most_common(40):
        print("  %8d  %s" % (n, name))
    print(f"wrote {os.path.join(out_dir, 'cells.tsv')}", flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
