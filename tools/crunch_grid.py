#!/usr/bin/env python3
"""Runs `jcrunchtool grid-one` on each workbook in a list, each in a child process, and writes a table of the results.

    python3 tools/crunch_grid.py build/jcrunchtool keys.txt out-dir [jobs]
    JC_GRID_FULL=1 python3 tools/crunch_grid.py ...   also reads each workbook whole and works its formulas out in slices, as the window does

keys.txt has one path to a workbook on each line (paths may have spaces). Each workbook runs under
`systemd-run --user --scope -p MemoryMax=4G`, with 120 seconds, so that one that takes too much is killed and no other is touched.
out-dir/grid.tsv has a line for each workbook, in the order of the list:

    path, exit, seconds, peak MB, then what grid-one says: milliseconds to the first screen drawn, to draw it again (the middle and the 95th percentile of 30), the rows and
    columns visited, cells looked at, runs of text, and whether a bound was reached; with JC_GRID_FULL also the milliseconds to read the whole workbook, to work its formulas
    out, the longest slice of that work (what the window would wait for), and the slices

exit is that of `grid-one` (0 drawn, 1 not opened, 2 opened with a sheet in part), 124 for a workbook that took more than 120 seconds, and 128 and more for a program that was
stopped (139 is a crash; 137 is a kill, which is the memory cap if the peak is near 4 GB). It prints how many crashed or timed out, and the times of the first screen.
"""
import os
import shutil
import subprocess
import sys
import tempfile
import time
from concurrent.futures import ThreadPoolExecutor

LIMIT_SECONDS = 120


class SetupFailed(Exception):
    """What runs the workbooks (systemd-run, timeout, the program) failed, so no workbook's result would mean anything."""


def check_systemd_run():
    if shutil.which("systemd-run") is None:
        raise SetupFailed("systemd-run is not installed, which this sweep needs to cap each workbook's memory.")
    try:
        done = subprocess.run(["systemd-run", "--user", "--scope", "-p", "MemoryMax=4G", "-p", "MemorySwapMax=0", "-q", "true"],
                              capture_output=True, text=True, timeout=60)
    except subprocess.TimeoutExpired:
        raise SetupFailed("systemd-run did not start a memory-capped scope in 60 seconds.")
    if done.returncode != 0:
        raise SetupFailed("systemd-run could not start a memory-capped scope (%s): %s" % (done.returncode, (done.stderr or "").strip() or "no message"))


FULL = bool(os.environ.get("JC_GRID_FULL"))
HEADER = ("path\texit\tseconds\tpeak MB\tfirst screen ms\tdraw again ms (middle)\tdraw again ms (95th)\trows\tcolumns\tcells looked at\truns of text\ttruncated"
          + ("\tread whole ms\tcalculate ms\tlongest slice ms\tslices" if FULL else ""))


def run_one(tool, path):
    command = ["systemd-run", "--user", "--scope", "-p", "MemoryMax=4G", "-p", "MemorySwapMax=0", "-q", "timeout", "-k", "5", str(LIMIT_SECONDS), tool, "grid-one"] + (["--full"] if FULL else []) + [path]
    started = time.time()
    env = dict(os.environ, JP_NO_TELEMETRY="1", QT_QPA_PLATFORM="offscreen")
    with tempfile.TemporaryFile() as errors:
        child = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=errors, env=env)
        output = child.stdout.read()
        _, status, usage = os.wait4(child.pid, 0)
        errors.seek(0)
        message = errors.read(2000).decode("utf-8", "replace").strip().replace("\n", " | ")
    seconds = time.time() - started
    code = os.waitstatus_to_exitcode(status)
    if code < 0:
        code = 128 - code
    if code in (126, 127) or (code == 1 and message and not output and path not in message):
        raise SetupFailed("the sweep could not run the program (exit %d): %s" % (code, message or "no message"))
    fields = output.decode("utf-8", "replace").rstrip("\n").split("\t")
    width = 12 if FULL else 8
    values = fields[2:2 + width] if code in (0, 2) and len(fields) >= 2 + width else ["-"] * width
    peak = usage.ru_maxrss / 1024.0
    return [path, str(code), f"{seconds:.2f}", f"{peak:.0f}"] + values, message


def percentile(values, p):
    values = sorted(values)
    return values[min(len(values) - 1, int(p * len(values)))] if values else 0


def main():
    if len(sys.argv) < 4:
        print(__doc__)
        return 2
    if sys.platform != "linux":
        print("crunch_grid.py runs on Linux only: it caps each workbook's memory with systemd-run. `jcrunchtool grid-one <file>` works on every system.", file=sys.stderr)
        return 2
    tool, keys, out_dir = sys.argv[1:4]
    try:
        check_systemd_run()
    except SetupFailed as why:
        print("crunch_grid.py: %s" % why, file=sys.stderr)
        return 1
    jobs = int(sys.argv[4]) if len(sys.argv) > 4 else 8
    with open(keys, encoding="utf-8", errors="surrogateescape") as f:
        paths = [line for line in f.read().splitlines() if line]
    print(f"{len(paths)} keys", flush=True)
    os.makedirs(out_dir, exist_ok=True)
    results = []
    try:
        with ThreadPoolExecutor(max_workers=jobs) as pool:
            for n, (row, message) in enumerate(pool.map(lambda p: run_one(tool, p), paths), 1):
                results.append((row, message))
                if n % 500 == 0:
                    print(f"{n} of {len(paths)}", file=sys.stderr, flush=True)
    except SetupFailed as why:
        print("crunch_grid.py: stopped after %d of %d workbooks: %s" % (len(results), len(paths), why), file=sys.stderr)
        pool.shutdown(wait=True, cancel_futures=True)
        return 1
    with open(os.path.join(out_dir, "grid.tsv"), "w", encoding="utf-8", errors="surrogateescape") as f:
        f.write(HEADER + "\n")
        for row, _ in results:
            f.write("\t".join(row) + "\n")
    with open(os.path.join(out_dir, "problems.txt"), "w", encoding="utf-8", errors="surrogateescape") as f:
        for row, message in results:
            if row[1] != "0":
                f.write(f"{row[1]}\t{row[0]}\t{message}\n")
    codes = [r[1] for r, _ in results]
    crashed = sum(1 for c in codes if c.isdigit() and int(c) >= 128 and int(c) != 137 or c == "137")
    hung = sum(1 for c in codes if c == "124")
    refused = sum(1 for c in codes if c == "1")
    first = [float(r[4]) for r, _ in results if r[1] in ("0", "2") and r[4] != "-"]
    again = [float(r[6]) for r, _ in results if r[1] in ("0", "2") and r[6] != "-"]
    wall = [float(r[2]) for r, _ in results]
    slices = [float(r[14]) for r, _ in results if FULL and r[1] in ("0", "2") and r[14] != "-"]
    print(f"{len(results)} workbooks: {codes.count('0')} drawn, {codes.count('2')} drawn with a sheet in part, {refused} not opened, {crashed} crashed or killed, {hung} timed out")
    if first:
        print("first screen (ms): middle %.1f, 95th percentile %.1f, 99th %.1f, longest %.1f" % (percentile(first, 0.5), percentile(first, 0.95), percentile(first, 0.99), max(first)))
        print("drawing the screen again (ms, the 95th percentile of each workbook's 30): middle %.2f, 95th percentile %.2f, longest %.2f" % (percentile(again, 0.5), percentile(again, 0.95), max(again)))
        print("whole process (s): middle %.2f, 95th percentile %.2f, longest %.2f" % (percentile(wall, 0.5), percentile(wall, 0.95), max(wall)))
    if slices:
        print("the longest slice of calculation in each workbook (ms): middle %.2f, 95th percentile %.2f, 99th %.2f, longest %.2f" % (percentile(slices, 0.5), percentile(slices, 0.95), percentile(slices, 0.99), max(slices)))
    print(f"wrote {os.path.join(out_dir, 'grid.tsv')}", flush=True)
    return 0 if crashed == 0 and hung == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
