#!/usr/bin/env python3
"""Runs `jcrunchtool info` on each workbook in a list, each in a child process, and writes a table of the results.

    python3 tools/crunch_scan.py build/jcrunchtool keys.txt out-dir [jobs]

keys.txt has one path to a workbook on each line (paths may have spaces). Each workbook runs under
`systemd-run --user --scope -p MemoryMax=4G`, with 120 seconds to read, so that one that takes too much is killed
and no other is touched. out-dir/problems.txt says, for each workbook read other than whole, the exit and what the program said. out-dir/books.tsv has a line for each workbook, in
the order of the list:

    path, exit, seconds, peak MB, sheets, cells, formula cells, stored results, functions used

exit is that of `info` (0 read whole, 1 not opened, 2 opened with a sheet only partly read), 124 for a workbook that
took more than 120 seconds, and 128 and more for a program that was stopped (139 is a crash; 137 is a kill, which is the
memory cap if the peak is near 4 GB). The peak memory is the child's own, as the system counted it. The functions are
NAME:count pairs, the most used first.
"""
import os
import subprocess
import sys
import tempfile
import time
from concurrent.futures import ThreadPoolExecutor

LIMIT_SECONDS = 120
HEADER = "path\texit\tseconds\tpeak MB\tsheets\tcells\tformula cells\tstored results\tfunctions used"


def run_one(tool, path):
    command = ["systemd-run", "--user", "--scope", "-p", "MemoryMax=4G", "-p", "MemorySwapMax=0", "-q",
               "timeout", "-k", "5", str(LIMIT_SECONDS), tool, "info", "--tsv", path]
    started = time.time()
    env = dict(os.environ, JP_NO_TELEMETRY="1", QT_QPA_PLATFORM="offscreen")
    with tempfile.TemporaryFile() as errors:
        child = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=errors, env=env)
        output = child.stdout.read()
        # wait4 gives this child's own resource use (a program run by `timeout` counts in it).
        _, status, usage = os.wait4(child.pid, 0)
        errors.seek(0)
        message = errors.read(2000).decode("utf-8", "replace").strip().replace("\n", " | ")
    seconds = time.time() - started
    code = os.waitstatus_to_exitcode(status)
    if code < 0:
        code = 128 - code
    fields = output.decode("utf-8", "replace").rstrip("\n").split("\t")
    values = fields[:5] if code in (0, 2) and len(fields) >= 5 else ["-"] * 5
    problems = fields[5] if code == 2 and len(fields) > 5 else message
    peak = usage.ru_maxrss / 1024.0
    largest = fields[6:16] if code in (0, 2) and len(fields) >= 16 else ["-"] * 10
    return [path, str(code), f"{seconds:.2f}", f"{peak:.0f}"] + values[:4] + [values[4] or "-"], problems, [path] + largest


def main():
    if len(sys.argv) < 4:
        print(__doc__)
        return 2
    tool, keys, out_dir = sys.argv[1:4]
    jobs = int(sys.argv[4]) if len(sys.argv) > 4 else 8
    with open(keys, encoding="utf-8", errors="surrogateescape") as f:
        paths = [line for line in f.read().splitlines() if line]
    print(f"{len(paths)} keys", flush=True)
    os.makedirs(out_dir, exist_ok=True)
    results = []
    with ThreadPoolExecutor(max_workers=jobs) as pool:
        for n, (row, problem, largest) in enumerate(pool.map(lambda p: run_one(tool, p), paths), 1):
            results.append((row, problem, largest))
            if n % 500 == 0:
                print(f"{n} of {len(paths)}", file=sys.stderr, flush=True)
    with open(os.path.join(out_dir, "books.tsv"), "w", encoding="utf-8", errors="surrogateescape") as f:
        f.write(HEADER + "\n")
        for row, _, _ in results:
            f.write("\t".join(row) + "\n")
    # The most any one thing came to in each workbook, for choosing the limits of the readers.
    with open(os.path.join(out_dir, "limits.tsv"), "w", encoding="utf-8", errors="surrogateescape") as f:
        f.write("path\tcells in a sheet\tXML bytes of a sheet\tstring chars\tformula chars\tshared formulas\tshared range cells\texpanded formula chars\tshared strings\tshared string chars\tsheets\n")
        for _, _, largest in results:
            f.write("\t".join(largest) + "\n")
    # What went wrong, for the workbooks that were not read whole.
    with open(os.path.join(out_dir, "problems.txt"), "w", encoding="utf-8", errors="surrogateescape") as f:
        for row, problem, _ in results:
            if row[1] != "0":
                f.write(f"{row[1]}\t{row[0]}\t{problem}\n")
    print(f"wrote {os.path.join(out_dir, 'books.tsv')}", flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
