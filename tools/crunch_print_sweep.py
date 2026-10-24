#!/usr/bin/env python3
"""Prints every workbook of a list to a PDF, each in a process of its own under a memory cap and a time limit, and reports what happened.

    python3 tools/crunch_print_sweep.py JCRUNCHTOOL LIST.txt [LIST2.txt ...] [--jobs N] [--out DIR]

For each workbook it runs `jcrunchtool print-pdf <file> <pdf>` under `systemd-run --user --scope -p MemoryMax=4G` and `timeout 120`, and deletes the PDF. The answers it counts:
ok (pages written), nothing (the workbook has nothing to print), refused (the file could not be opened: a message, not a crash), failed, timed out, and crashed (a signal).
It prints the pages per sheet (median, 95th percentile, most), the milliseconds per workbook, and writes DIR/sweep.tsv with a line for each workbook: path, answer, pages, sheets, ms.
Only the practice lists (dev-*.txt) are for this.
"""
import os, statistics, subprocess, sys, tempfile
from concurrent.futures import ThreadPoolExecutor


def run(args):
    tool, path, out = args
    pdf = os.path.join(out, "w%d.pdf" % (abs(hash(path)) % 10**12))
    cmd = ["systemd-run", "--user", "--scope", "-q", "-p", "MemoryMax=4G", "timeout", "120", tool, "print-pdf", path, pdf]
    env = dict(os.environ, QT_QPA_PLATFORM="offscreen", JP_NO_TELEMETRY="1")
    try:
        r = subprocess.run(cmd, capture_output=True, text=True, env=env, timeout=150)
    except subprocess.TimeoutExpired:
        return path, "timed out", 0, 0, 120000, []
    finally:
        try:
            os.unlink(pdf)
        except OSError:
            pass
    sheets_pages = []
    pages = sheets = ms = 0
    lines = r.stdout.splitlines()
    if r.returncode == 0 and lines:
        f = lines[0].split("\t")
        pages, sheets, ms = int(f[0]), int(f[1]), int(f[2])
        for l in lines[1:]:
            p = l.split("\t")
            if p[0] == "S":
                sheets_pages.append(int(p[2]))
        return path, "ok", pages, sheets, ms, sheets_pages
    if r.returncode == 124:
        return path, "timed out", 0, 0, 120000, []
    if r.returncode in (1, 2):
        return path, "refused", 0, 0, 0, []
    if r.returncode == 3:
        return path, "nothing", 0, 0, 0, []
    if r.returncode == 4:
        return path, "failed", 0, 0, 0, []
    return path, "crashed (%d)" % r.returncode, 0, 0, 0, []


def pct(v, p):
    v = sorted(v)
    return v[min(len(v) - 1, int(p * len(v)))] if v else 0


def main():
    args = sys.argv[1:]
    jobs, out = 12, tempfile.mkdtemp(prefix="crunch-print-sweep-")
    if "--jobs" in args:
        i = args.index("--jobs")
        jobs = int(args[i + 1])
        del args[i:i + 2]
    keep = None
    if "--out" in args:
        i = args.index("--out")
        keep = args[i + 1]
        os.makedirs(keep, exist_ok=True)
        del args[i:i + 2]
    tool, lists = args[0], args[1:]
    paths = []
    for l in lists:
        paths += open(l, encoding="utf-8").read().splitlines()
    paths = [p for p in paths if p]
    with ThreadPoolExecutor(jobs) as ex:
        results = list(ex.map(run, [(tool, p, out) for p in paths]))
    if keep:
        with open(os.path.join(keep, "sweep.tsv"), "w", encoding="utf-8") as f:
            f.write("path\tanswer\tpages\tsheets\tms\n")
            for p, a, pg, sh, ms, _ in results:
                f.write("%s\t%s\t%d\t%d\t%d\n" % (p, a, pg, sh, ms))
    counts = {}
    for _, a, *_ in results:
        counts[a.split(" ")[0] if a.startswith("crashed") else a] = counts.get(a.split(" ")[0] if a.startswith("crashed") else a, 0) + 1
    per_sheet = [n for *_, sp in results for n in sp]
    times = [ms for _, a, pg, sh, ms, _ in results if a == "ok"]
    print("workbooks %d: %s" % (len(results), ", ".join("%s %d" % (k, counts[k]) for k in sorted(counts))))
    print("sheets with pages %d: pages per sheet median %g, 95th percentile %d, most %d" % (len(per_sheet), statistics.median(per_sheet) if per_sheet else 0, pct(per_sheet, 0.95), max(per_sheet) if per_sheet else 0))
    print("milliseconds per workbook: median %g, 95th percentile %d, most %d" % (statistics.median(times) if times else 0, pct(times, 0.95), max(times) if times else 0))
    bad = [(p, a) for p, a, *_ in results if a.startswith("crashed") or a in ("timed out", "failed")]
    for p, a in bad[:20]:
        print("  %s: %s" % (a, p))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
