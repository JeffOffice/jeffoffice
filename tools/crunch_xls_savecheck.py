#!/usr/bin/env python3
"""Runs `jcrunchtool xls-save-check` on each workbook in a list, each in a child process, and sums up how saving a workbook of the older binary form as the XML form went.

    python3 tools/crunch_xls_savecheck.py build/jcrunchtool keys.txt out-dir [jobs]

keys.txt has one path to a workbook on each line (paths may have spaces). Each runs under `systemd-run --user --scope -p MemoryMax=4G` with 120 seconds, as in crunch_scan.py.
out-dir/savecheck.tsv has a line for each workbook: path, exit, the child's line. The summary is printed: how many were written and read back the same, how many differed (and how), how
many could not be opened or saved, and how many crashed or took more than 120 seconds.
"""
import collections
import os
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor

LIMIT_SECONDS = 120


def run_one(tool, path):
    command = ["systemd-run", "--user", "--scope", "-p", "MemoryMax=4G", "-p", "MemorySwapMax=0", "-q", "timeout", "-k", "5", str(LIMIT_SECONDS), tool, "xls-save-check", path]
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
    os.makedirs(out_dir, exist_ok=True)
    paths = [l.rstrip("\n") for l in open(keys) if l.strip()]
    results = [None] * len(paths)

    def work(i):
        results[i] = run_one(tool, paths[i])

    with ThreadPoolExecutor(jobs) as ex:
        list(ex.map(work, range(len(paths))))
    counts = collections.Counter()
    kinds = collections.Counter()
    peak = 0.0
    slowest = 0
    with open(os.path.join(out_dir, "savecheck.tsv"), "w") as f:
        for path, (code, output, message, mb) in zip(paths, results):
            f.write("%s\t%d\t%s\t%s\n" % (path, code, output.replace("\t", " | "), message))
            peak = max(peak, mb)
            fields = output.split("\t")
            if len(fields) >= 8:
                slowest = max(slowest, int(fields[6]) + int(fields[7]))
            if code == 124:
                counts["timed out"] += 1
            elif code == 137:
                counts["killed by the memory cap"] += 1
            elif code >= 128 or code not in (0, 1, 3, 4):
                counts["crashed"] += 1
            elif code == 0:
                counts["written and read back the same"] += 1
            elif code == 1:
                counts["not opened (refused)"] += 1
            elif code == 3:
                counts["not saved"] += 1
                kinds[fields[2] if len(fields) > 2 else "?"] += 1
            else:
                counts["differ"] += 1
                for k in (fields[2] if len(fields) > 2 else "?").split(" | "):
                    kinds[k] += 1
    print("workbooks %d: %s" % (len(paths), "; ".join("%d %s" % (n, k) for k, n in sorted(counts.items(), key=lambda kv: -kv[1]))))
    if kinds:
        print("what differed or failed: " + "; ".join("%s (%d)" % (k, n) for k, n in kinds.most_common(12)))
    print("the slowest workbook took %.1f s for both writes; the most memory of any child %.0f MB" % (slowest / 1000.0, peak))
    return 0


if __name__ == "__main__":
    sys.exit(main())
