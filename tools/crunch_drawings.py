#!/usr/bin/env python3
"""Runs `jcrunchtool grid-drawings-one` on each workbook in a list, each in a child process, and writes a table of the results.

    python3 tools/crunch_drawings.py build/jcrunchtool keys.txt out-dir [jobs]
    JC_DRAWINGS_LIVE=1 python3 tools/crunch_drawings.py ...   reads each workbook whole, so that its charts read its cells

keys.txt has one path to a workbook on each line (paths may have spaces). Each workbook runs under `systemd-run --user --scope -p MemoryMax=4G`, with 120 seconds, so that one that
takes too much is killed and no other is touched. out-dir/drawings.tsv has a line for each workbook, in the order of the list: path, exit, seconds, peak MB, then what grid-drawings-one
says (anchors, charts, pictures, shapes, groups, connectors, objects that can be drawn, objects left out, hidden, anchors with nothing read, drawn, failed to draw, milliseconds to draw
them (sum and longest), chart sheets, the first sheet with something drawn, the references of charts and those found, and the reasons).
It prints the workbooks that have drawings, how many open with every object drawn and how many have some left out, and the reasons.
"""
import collections
import os
import shutil
import subprocess
import sys
import tempfile
import time
from concurrent.futures import ThreadPoolExecutor

LIMIT_SECONDS = 120
LIVE = bool(os.environ.get("JC_DRAWINGS_LIVE"))
HEADER = ("path\texit\tseconds\tpeak MB\tanchors\tcharts\tpictures\tshapes\tgroups\tconnectors\tcan be drawn\tleft out\thidden\tnothing read\tdrawn\tfailed\tdraw ms (sum)\tdraw ms (longest)"
          "\tchart sheets\tfirst sheet\tchart references\tfound\treasons")
FIELDS = 19


class SetupFailed(Exception):
    """What runs the workbooks (systemd-run, timeout, the program) failed, so no workbook's result would mean anything."""


def check_systemd_run():
    if shutil.which("systemd-run") is None:
        raise SetupFailed("systemd-run is not installed, which this sweep needs to cap each workbook's memory.")
    done = subprocess.run(["systemd-run", "--user", "--scope", "-p", "MemoryMax=4G", "-p", "MemorySwapMax=0", "-q", "true"], capture_output=True, text=True, timeout=60)
    if done.returncode != 0:
        raise SetupFailed("systemd-run could not start a memory-capped scope (%s): %s" % (done.returncode, (done.stderr or "").strip() or "no message"))


def run_one(tool, path):
    command = (["systemd-run", "--user", "--scope", "-p", "MemoryMax=4G", "-p", "MemorySwapMax=0", "-q", "timeout", "-k", "5", str(LIMIT_SECONDS), tool, "grid-drawings-one"]
               + (["--live"] if LIVE else []) + [path])
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
    if code in (126, 127):
        raise SetupFailed("the sweep could not run the program (exit %d): %s" % (code, message or "no message"))
    fields = output.decode("utf-8", "replace").rstrip("\n").split("\t")
    values = fields[2:2 + FIELDS] if code == 0 and len(fields) >= 2 + FIELDS else ["-"] * FIELDS
    return [path, str(code), f"{seconds:.2f}", f"{usage.ru_maxrss / 1024.0:.0f}"] + values, message


def main():
    if len(sys.argv) < 4:
        print(__doc__)
        return 2
    tool, keys, out_dir = sys.argv[1:4]
    try:
        check_systemd_run()
    except SetupFailed as why:
        print("crunch_drawings.py: %s" % why, file=sys.stderr)
        return 1
    jobs = int(sys.argv[4]) if len(sys.argv) > 4 else 8
    with open(keys, encoding="utf-8", errors="surrogateescape") as f:
        paths = [line for line in f.read().splitlines() if line]
    os.makedirs(out_dir, exist_ok=True)
    results = []
    try:
        with ThreadPoolExecutor(max_workers=jobs) as pool:
            for n, result in enumerate(pool.map(lambda p: run_one(tool, p), paths), 1):
                results.append(result)
                if n % 1000 == 0:
                    print(f"{n} of {len(paths)}", file=sys.stderr, flush=True)
    except SetupFailed as why:
        print("crunch_drawings.py: stopped after %d of %d workbooks: %s" % (len(results), len(paths), why), file=sys.stderr)
        return 1
    with open(os.path.join(out_dir, "drawings.tsv"), "w", encoding="utf-8", errors="surrogateescape") as f:
        f.write(HEADER + "\n")
        for row, _ in results:
            f.write("\t".join(row) + "\n")
    opened = [r for r, _ in results if r[1] == "0"]
    bad = [(r[1], r[0], m) for r, m in results if r[1] != "0"]
    with open(os.path.join(out_dir, "problems.txt"), "w", encoding="utf-8", errors="surrogateescape") as f:
        for code, path, message in bad:
            f.write(f"{code}\t{path}\t{message}\n")
    n = lambda r, i: int(r[i]) if r[i].lstrip("-").isdigit() else 0
    with_drawings = [r for r in opened if n(r, 4) > 0]
    kinds = {"charts": 5, "pictures": 6, "shapes": 7, "groups": 8, "connectors": 9}
    print(f"{len(paths)} workbooks: {len(opened)} opened, {len(bad)} did not (crashed or timed out: {sum(1 for c, _, _ in bad if int(c) >= 124)})")
    print(f"{len(with_drawings)} with a drawing; with: " + ", ".join(f"{k} {sum(1 for r in with_drawings if n(r, i) > 0)}" for k, i in kinds.items()) + f"; chart sheets {sum(1 for r in opened if n(r, 18) > 0)}")
    objects = sum(n(r, 5) + n(r, 6) + n(r, 7) + n(r, 8) + n(r, 9) for r in with_drawings)
    print(f"{objects} objects: " + ", ".join(f"{k} {sum(n(r, i) for r in with_drawings)}" for k, i in kinds.items()))
    whole = sum(1 for r in with_drawings if n(r, 11) == 0 and n(r, 15) == 0)
    print(f"{whole} of {len(with_drawings)} workbooks have every object that is not hidden drawn; {sum(1 for r in with_drawings if n(r, 11) > 0)} have some left out, {sum(1 for r in with_drawings if n(r, 15) > 0)} have one that drew nothing")
    print(f"objects drawn {sum(n(r, 14) for r in with_drawings)}, left out {sum(n(r, 11) for r in with_drawings)}, failed {sum(n(r, 15) for r in with_drawings)}, hidden {sum(n(r, 12) for r in with_drawings)}, anchors with nothing read {sum(n(r, 13) for r in with_drawings)}")
    refs, found = sum(n(r, 20) for r in with_drawings), sum(n(r, 21) for r in with_drawings)
    if LIVE:
        print(f"chart references: {refs}, found {found}")
    why = collections.Counter()
    for r in with_drawings:
        for reason in r[22].split("; ") if len(r) > 22 and r[22] not in ("", "-") else []:
            why[reason.split("(")[0].strip()] += 1
    for reason, count in why.most_common(12):
        print(f"  {count:5d}  {reason}")
    times = sorted(float(r[17]) for r in with_drawings if r[17] not in ("-", ""))
    if times:
        print(f"longest object draw: median {times[len(times) // 2]:.1f} ms, 95th percentile {times[int(0.95 * len(times))]:.1f} ms, longest {times[-1]:.1f} ms")
    return 0


if __name__ == "__main__":
    sys.exit(main())
