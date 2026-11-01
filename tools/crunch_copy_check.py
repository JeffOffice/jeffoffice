#!/usr/bin/env python3
"""Runs `jcrunchtool copy-check` on each workbook in a list, each in a child process, and sums up how a copy in another file format went.

    python3 tools/crunch_copy_check.py build/jcrunchtool keys.txt out-dir xls|ods|xlsx [jobs]

keys.txt has one path to a workbook on each line (paths may have spaces). Each runs under `systemd-run --user --scope -p MemoryMax=4G` with 120 seconds, as in crunch_scan.py.
out-dir/copy-<format>.tsv has a line for each workbook: path, exit, the child's line. The summary is printed: how many were written and read back, and the sums over all of them: the values
that are the same of those the format can hold, the formulas kept, the number formats and the looks that are the same, the merged ranges, and what the copies do not have.
"""
import collections
import os
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor

LIMIT_SECONDS = 120


def run_one(tool, path, fmt):
    command = ["systemd-run", "--user", "--scope", "-p", "MemoryMax=4G", "-p", "MemorySwapMax=0", "-q", "timeout", "-k", "5", str(LIMIT_SECONDS), tool, "copy-check", path, fmt]
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
    return code, output.strip(), message


def main():
    if len(sys.argv) < 5:
        print(__doc__)
        return 2
    tool, keys, out_dir, fmt = sys.argv[1:5]
    jobs = int(sys.argv[5]) if len(sys.argv) > 5 else 8
    os.makedirs(out_dir, exist_ok=True)
    paths = [l.rstrip("\n") for l in open(keys) if l.strip()]
    results = [None] * len(paths)
    tsv = os.path.join(out_dir, "copy-%s.tsv" % fmt)
    if tool == "--summarize" and os.path.exists(tsv):
        # (the lines of an earlier run, summed up again)
        lines = [l for l in open(tsv).read().split("\n") if l.strip()]
        for i, line in enumerate(lines[: len(paths)]):
            parts = line.split("\t")
            if len(parts) < 3:
                results[i] = (1, "", "")
                continue
            results[i] = (int(parts[1]), parts[2].replace(" | ", "\t"), parts[3] if len(parts) > 3 else "")
        for i in range(len(lines), len(paths)):
            results[i] = (1, "", "")
    else:
        def work(i):
            results[i] = run_one(tool, paths[i], fmt)

        with ThreadPoolExecutor(jobs) as ex:
            list(ex.map(work, range(len(paths))))
    counts = collections.Counter()
    sums = collections.Counter()
    lost = collections.Counter()
    firsts = collections.Counter()
    reasons = collections.Counter()
    bad_files = []
    with open(tsv if tool != "--summarize" else os.devnull, "w") as f:
        for path, (code, output, message) in zip(paths, results):
            f.write("%s\t%d\t%s\t%s\n" % (path, code, output.replace("\t", " | "), message))
            fields = output.split("\t")
            fields += [""] * (23 - len(fields))   # (a line ends in empty fields that the run's strip took off)
            if code == 124:
                counts["timed out"] += 1
            elif code == 137:
                counts["killed by the memory cap"] += 1
            elif code >= 128 or code not in (0, 1, 3):
                counts["crashed"] += 1
            elif code == 1:
                counts["not opened (refused)"] += 1
            elif code == 3 or not fields[3]:
                counts["not written or not read back"] += 1
                firsts[fields[3] if len(fields) > 3 else "?"] += 1
            else:
                counts["written and read back"] += 1
                # fields: copy-check, path, fmt, result, bytes, sheets, compared, valueDiffs, formulas, formulasBack, formulasSame, asValues, fmt, fmtNum, fmtLook, lost, mergesA, mergesB, writeMs, readMs, notKept, first
                n = [int(x) for x in fields[4:20]]
                keys_ = ["bytes", "sheets", "values", "valueDiffs", "formulas", "formulasBack", "formulasSame", "asValues", "fmt", "fmtNum", "fmtLook", "lost", "mergesA", "mergesB", "writeMs", "readMs"]
                for k, v in zip(keys_, n):
                    sums[k] += v
                if n[3] == 0:
                    sums["filesValuesEqual"] += 1
                if fields[3] != "ok":
                    sums["filesDiffer"] += 1
                    bad_files.append(path)
                    if len(fields) > 21 and fields[21]:
                        firsts[fields[21].split(" at ")[0]] += 1
                if len(fields) > 22:
                    for item in fields[22].split("; "):
                        if item:
                            n_, _, why_ = item.partition(" x ")
                            reasons[why_] += int(n_)
                for item in fields[20].split("; "):
                    if item:
                        lost[item.split(" formulas")[0] if "formulas the format cannot hold" in item else item] += 1
    print("workbooks %d -> %s: %s" % (len(paths), fmt, "; ".join("%d %s" % (n, k) for k, n in sorted(counts.items(), key=lambda kv: -kv[1]))))
    v = sums["values"]
    if v:
        print("cells with a value %d: %d differ (%.4f%% equal); files with every value equal %d of %d" % (v, sums["valueDiffs"], 100.0 * (v - sums["valueDiffs"]) / v, sums["filesValuesEqual"], counts["written and read back"]))
        fo = sums["formulas"]
        if fo:
            print("formulas %d: %d kept as formulas (%.2f%%), %d with the same text; %d written as values (the format cannot hold them)" % (fo, sums["formulasBack"], 100.0 * sums["formulasBack"] / fo, sums["formulasSame"], sums["asValues"]))
        if sums["fmt"]:
            print("formats of %d cells: number format same %.2f%%, look same %.2f%%" % (sums["fmt"], 100.0 * sums["fmtNum"] / sums["fmt"], 100.0 * sums["fmtLook"] / sums["fmt"]))
        print("cells lost to the format's limits %d; merged ranges %d -> %d; bytes %d; write %.1f s, read %.1f s in all" % (sums["lost"], sums["mergesA"], sums["mergesB"], sums["bytes"], sums["writeMs"] / 1000.0, sums["readMs"] / 1000.0))
    if firsts:
        print("failures or first differences: " + "; ".join("%s (%d)" % (k, n) for k, n in firsts.most_common(10)))
    if lost:
        print("not in the copies: " + "; ".join("%s (%d files)" % (k, n) for k, n in lost.most_common(12)))
    if reasons:
        print("formulas written as values, by reason: " + "; ".join("%s (%d)" % (k, n) for k, n in reasons.most_common(8)))
    if bad_files:
        print("files that differ (first 5): " + " ".join(bad_files[:5]))
    return 0


if __name__ == "__main__":
    sys.exit(main())
