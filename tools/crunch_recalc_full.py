#!/usr/bin/env python3
"""Runs `jcrunchtool recalc-full-one` on each workbook in a list, each in a child process, and compares every formula's value, worked out from the workbook's constants in
dependency order, with the result the file stored.

    python3 tools/crunch_recalc_full.py build/jcrunchtool keys.txt out-dir [jobs]

keys.txt has one path to a workbook on each line (paths may have spaces). Each workbook runs under `systemd-run --user --scope -p MemoryMax=6G` with 120 seconds, as in
crunch_scan.py. Unlike crunch_recalc.py, which reads what a formula refers to from the values the file stored, this works every formula out from the constants: a formula
that is wrong makes the formulas that read it wrong, as it would in a program that recalculates a workbook. What the engine cannot work out keeps the result the file stored
(a function it lacks, one that changes by itself, another workbook, a table's columns, a data table, a formula that did not parse or has no result stored, one past the
bounds, a circle in a workbook that asks for iterative calculation), and every formula that reads one of those, directly or through others, is tainted: its value is told,
but it is not counted in the share. (OFFSET and INDIRECT depend on cells only: they are worked out and counted apart.)

out-dir/cells.tsv has a line for each formula cell that did not match, a few of each kind that were kept apart, and a few that matched: workbook, sheet, cell, formula,
stored, ours, class (M match, X mismatch, U unknown function, V volatile, E external, T table columns, P not parsed, N no result, D data table, L work bound, I circle in an
iterative workbook, Z tainted, S out of time). out-dir/books.tsv has a line for each workbook, with its seconds and peak memory; out-dir/unsupported.tsv lists the functions the
engine lacks, with the cells that call each. The summary is printed: the cells and the share that matched, the workbooks that matched whole (against the bars of
docs/jeffcrunch.md), how each of the commonest 50 functions fared, the commonest mismatches, the time and memory, and the commonest unsupported functions.
"""
import os
import re
import statistics
import subprocess
import sys
import tempfile
import time
from collections import Counter
from concurrent.futures import ThreadPoolExecutor

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from crunch_scan import LIMIT_SECONDS, SetupFailed, check_systemd_run, need_linux  # noqa: E402

CLASSES = "MXUVETPNDLIZS"
MEMORY = "6G"


def unescape(text):
    return re.sub(r"\\([tnr\\])", lambda m: {"t": "\t", "n": "\n", "r": "\r", "\\": "\\"}[m.group(1)], text)


def escape(text):
    return text.replace("\\", "\\\\").replace("\t", "\\t").replace("\n", "\\n").replace("\r", "\\r")


def run_one(tool, path, matches, seconds):
    command = ["systemd-run", "--user", "--scope", "-p", "MemoryMax=" + MEMORY, "-p", "MemorySwapMax=0", "-q",
               "timeout", "-k", "5", str(LIMIT_SECONDS), tool, "recalc-full-one", path, str(matches), str(seconds)]
    env = dict(os.environ, JP_NO_TELEMETRY="1", QT_QPA_PLATFORM="offscreen")
    started = time.time()
    with tempfile.TemporaryFile() as errors:
        child = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=errors, env=env)
        output = child.stdout.read()
        _, status, usage = os.wait4(child.pid, 0)
        errors.seek(0)
        message = errors.read(2000).decode("utf-8", "replace").strip().replace("\n", " | ")
    wall = time.time() - started
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
        elif f[0] == "fullbook" and len(f) >= 29:
            book = f[1:]
    return path, code, rows, fns, unsup, pats, book, message, wall


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
        print("crunch_recalc_full.py: %s" % why, file=sys.stderr)
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
        print("crunch_recalc_full.py: stopped after %d of %d workbooks: %s" % (len(results), len(paths), why), file=sys.stderr)
        pool.shutdown(wait=True, cancel_futures=True)
        return 1
    opened = crashed = timed_out = not_opened = 0
    totals = Counter()
    volatile_checked = volatile_matched = tainted_matched = circular = root_mismatches = 0
    functions = {}
    unsupported = Counter()
    patterns = Counter()
    formula_books = whole_books = checked_books = 0
    manual_books = manual_mismatch = 0
    timing = []   # (seconds, peak MB, sheet MB, formula cells, path)
    lost = []
    with open(os.path.join(out_dir, "cells.tsv"), "w", encoding="utf-8", errors="surrogateescape") as cells, \
            open(os.path.join(out_dir, "books.tsv"), "w", encoding="utf-8", errors="surrogateescape") as books:
        cells.write("workbook\tsheet\tcell\tformula\tstored\tours\tclass\n")
        books.write("workbook\texit\tformula cells\t" + "\t".join(CLASSES) + "\tvolatile checked\tvolatile matched\ttainted matched\tcircular\troots\tmanual\twhole\tseconds\tload\tcalc\tpeak MB\tsheet MB\tgraph entries\tevaluations\twall\n")
        for path, code, rows, fns, unsup, pats, book, message, wall in results:
            if code == 124:
                timed_out += 1
                lost.append((path, "timed out"))
            elif code >= 128:
                crashed += 1
                lost.append((path, "crashed (exit %d)" % code))
            elif code == 1:
                not_opened += 1
            else:
                opened += 1
            if book is None:
                books.write("%s\t%d\t%s%.1f\n" % (escape(path), code, "\t" * 28, wall))
                continue
            counts = [int(x) for x in book[:14]]   # formula cells, then the 13 classes
            vc, vm, tm, circ, roots = int(book[14]), int(book[15]), int(book[16]), int(book[17]), int(book[18])
            manual, whole = int(book[19]), int(book[20])
            seconds, load, calc, peak, sheet_mb = float(book[21]), book[22], book[23], float(book[24]), float(book[25])
            graph_entries, evaluations = int(book[26]), int(book[27])
            books.write("%s\t%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%.2f\t%s\t%s\t%.0f\t%.1f\t%d\t%d\t%.1f\n" % (escape(path), code, "\t".join(map(str, counts)), vc, vm, tm, circ, roots, manual, whole,
                                                                                                         seconds, load, calc, peak, sheet_mb, graph_entries, evaluations, wall))
            timing.append((max(seconds, wall), peak, sheet_mb, counts[0], path))
            totals["formula cells"] += counts[0]
            for k, c in zip(CLASSES, counts[1:]):
                totals[k] += c
            volatile_checked += vc
            volatile_matched += vm
            tainted_matched += tm
            circular += circ
            root_mismatches += roots
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
    cells_ok = 100.0 * totals["M"] / checked if checked else 0
    books_ok = 100.0 * whole_books / checked_books if checked_books else 0
    print(f"recalc-full: workbooks {len(paths)} (opened {opened}, not opened {not_opened}, crashed {crashed}, timed out {timed_out}); formula cells {totals['formula cells']}, "
          f"checked {checked} of which matched {totals['M']} ({pct(totals['M'], checked)}); kept apart: unknown function {totals['U']}, volatile {totals['V']} "
          f"(OFFSET and INDIRECT worked out: {volatile_checked}, matched {volatile_matched}), another workbook {totals['E']}, table columns {totals['T']}, not parsed {totals['P']}, "
          f"no result stored {totals['N']}, data table {totals['D']}, work bound {totals['L']}, circle in an iterative workbook {totals['I']}, out of time {totals['S']}; "
          f"mismatches that follow another {totals['X'] - root_mismatches}; tainted (read a formula kept apart) {totals['Z']} (of which match the file {tainted_matched}); circular cells {circular}; workbooks with formulas {formula_books}, "
          f"with something checked {checked_books}, matched whole {whole_books} ({pct(whole_books, checked_books)}); of those on manual calculation {manual_books}, {manual_mismatch} with "
          f"mismatches; {time.time() - started:.0f} s")
    print("preview bar: cells %s (98%% asked), workbooks %s (90%% asked): %s" % (pct(totals["M"], checked), pct(whole_books, checked_books),
                                                                                "met" if cells_ok >= 98 and books_ok >= 90 else "not met"))
    print("functions by calls (calls, share of calls in cells that matched, cells, share of cells that matched):")
    for name, (calls, mcalls, ncells, mcells) in top:
        print("  %-14s %10d %9s %9d %9s" % (name, calls, pct(mcalls, calls), ncells, pct(mcells, ncells)))
    print(f"{len(patterns)} patterns of the mismatches that follow no other, the 15 commonest:")
    for name, n in patterns.most_common(15):
        print("  %8d  %s" % (n, name))
    if timing:
        secs = sorted(t[0] for t in timing)
        peaks = sorted(t[1] for t in timing)
        print("time per workbook: median %.2f s, 95th %.2f s, 99th %.2f s, longest %.1f s, all together %.0f s (cpu); peak memory: median %.0f MB, 95th %.0f MB, largest %.0f MB" % (
            statistics.median(secs), secs[int(len(secs) * 0.95)], secs[int(len(secs) * 0.99)], secs[-1], sum(secs), statistics.median(peaks), peaks[int(len(peaks) * 0.95)], peaks[-1]))
        print("slowest workbooks (seconds, peak MB, largest sheet MB, formula cells):")
        for sec, peak, sheet_mb, n, path in sorted(timing, reverse=True)[:8]:
            print("  %7.1f %7.0f %7.1f %9d  %s" % (sec, peak, sheet_mb, n, os.path.basename(path)))
        print("biggest in memory:")
        for sec, peak, sheet_mb, n, path in sorted(timing, key=lambda t: -t[1])[:8]:
            print("  %7.1f %7.0f %7.1f %9d  %s" % (sec, peak, sheet_mb, n, os.path.basename(path)))
        print("largest sheet:")
        for sec, peak, sheet_mb, n, path in sorted(timing, key=lambda t: -t[2])[:3]:
            print("  %7.1f %7.0f %7.1f %9d  %s" % (sec, peak, sheet_mb, n, os.path.basename(path)))
    for path, why in lost[:20]:
        print("  lost: %s %s" % (why, path))
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
