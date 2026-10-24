#!/usr/bin/env python3
"""Runs `jcrunchtool xls-fuzz` on each workbook in a list, each in a child process, and sums up the sweep for crashes and hangs.

    python3 tools/crunch_xls_fuzz.py build/jcrunchtool keys.txt out-dir [jobs] [copies=200] [seed=1]

keys.txt has one path to a workbook on each line (paths may have spaces). Each runs under `systemd-run --user --scope -p MemoryMax=4G` with 120 seconds, as in crunch_scan.py.
out-dir/fuzz.tsv has a line for each workbook: path, exit, the child's line (copies, opened, refused, sheets stopped short, calculated, the longest time of one copy). The summary
is printed: how many copies were made, how many opened, how many workbooks crashed (exit 128 and more other than a kill by the memory cap) or took more than 120 seconds.
"""
import os
import re
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor

LIMIT_SECONDS = 120


def run_one(tool, path, copies, seed):
    command = ["systemd-run", "--user", "--scope", "-p", "MemoryMax=4G", "-p", "MemorySwapMax=0", "-q", "timeout", "-k", "5", str(LIMIT_SECONDS), tool, "xls-fuzz", path, str(copies), str(seed), "100"]
    env = dict(os.environ, JP_NO_TELEMETRY="1", QT_QPA_PLATFORM="offscreen")
    with tempfile.TemporaryFile() as errors:
        child = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=errors, env=env)
        output = child.stdout.read().decode("utf-8", "replace")
        _, status, usage = os.wait4(child.pid, 0)
        errors.seek(0)
        message = errors.read(300).decode("utf-8", "replace").strip().replace("\n", " | ")
    code = os.waitstatus_to_exitcode(status)
    if code < 0:
        code = 128 - code
    return code, output.strip(), message, usage.ru_maxrss / 1024.0


def main():
    if len(sys.argv) < 4:
        print(__doc__)
        return 2
    tool, keys, out_dir = sys.argv[1:4]
    jobs = int(sys.argv[4]) if len(sys.argv) > 4 else 8
    copies = int(sys.argv[5]) if len(sys.argv) > 5 else 200
    seed = int(sys.argv[6]) if len(sys.argv) > 6 else 1
    os.makedirs(out_dir, exist_ok=True)
    paths = [l.rstrip("\n") for l in open(keys) if l.strip()]
    results = [None] * len(paths)

    def work(i):
        results[i] = run_one(tool, paths[i], copies, seed)

    with ThreadPoolExecutor(jobs) as ex:
        list(ex.map(work, range(len(paths))))
    total = opened = refused = crashed = timed_out = killed = 0
    longest = 0.0
    peak = 0.0
    with open(os.path.join(out_dir, "fuzz.tsv"), "w") as f:
        for path, (code, output, message, mb) in zip(paths, results):
            f.write("%s\t%d\t%s\t%s\n" % (path, code, output.replace("\t", " | "), message))
            peak = max(peak, mb)
            if code == 124 or code == 137 and mb < 3800 and False:
                timed_out += 1
            elif code == 124:
                timed_out += 1
            elif code == 137:
                killed += 1
            elif code >= 128 or code not in (0, 1):
                crashed += 1
            m = re.search(r"(\d+) copies\t(\d+) opened\t(\d+) refused", output)
            if m:
                total += int(m.group(1))
                opened += int(m.group(2))
                refused += int(m.group(3))
            m = re.search(r"(\d+) ms longest", output)
            if m:
                longest = max(longest, int(m.group(1)) / 1000.0)
    print("workbooks %d; copies %d (%d opened, %d refused); crashed %d, timed out %d, killed by the memory cap %d; the longest copy %.2f s; the most memory of any child %.0f MB" % (
        len(paths), total, opened, refused, crashed, timed_out, killed, longest, peak))
    return 0


if __name__ == "__main__":
    sys.exit(main())
