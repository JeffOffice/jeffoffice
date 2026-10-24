#!/usr/bin/env python3
"""Runs `jcrunchtool roundtrip-one` on each workbook in a list, each in a child process, and writes a table of the results (the yardstick for saving: docs/jeffcrunch.md).

    python3 tools/crunch_roundtrip.py build/jcrunchtool keys.txt out-dir [jobs]

keys.txt has one path to a workbook on each line (paths may have spaces). Each workbook runs under `systemd-run --user --scope -p MemoryMax=6G`, with 300 seconds, so that one that takes too
much is killed and no other is touched. out-dir/roundtrip.tsv has a line for each workbook, in the order of the list (the fields `roundtrip-one` prints), and out-dir/problems.txt the workbooks
that crashed, timed out, or did not read back the same, with what differed.

The summary: how many workbooks were saved and read back, of them how many read back the same untouched (every part the same to the byte, and the same cells, formulas, and formats) and with one
cell edited (that cell has what was typed, every formula that depends on it has its new result, nothing else differs); how many of the parts that were not meant to change did not; and the
commonest differences. A workbook that is opened in part is one the window does not change or save, and is counted apart.
"""
import collections
import os
import shutil
import subprocess
import sys
import tempfile
import time
from concurrent.futures import ThreadPoolExecutor

LIMIT_SECONDS = 300
MEMORY = "6G"

HEADER = ("path\texit\tseconds\tpeak MB\tparts\tparts the same\tuntouched read back the same\tuntouched differences\tedited cell\tformulas that depend on it\tresults that changed\t"
          "unexpected differences\tparts not meant to change\tof them the same\tcell right\tedit differences\tuntouched ms\tedit ms")


class SetupFailed(Exception):
    """What runs the workbooks (systemd-run, timeout, the program) failed, so no workbook's result would mean anything."""


def check_systemd_run():
    if shutil.which("systemd-run") is None:
        raise SetupFailed("systemd-run is not installed, which this sweep needs to cap each workbook's memory.")
    try:
        done = subprocess.run(["systemd-run", "--user", "--scope", "-p", "MemoryMax=" + MEMORY, "-p", "MemorySwapMax=0", "-q", "true"], capture_output=True, text=True, timeout=60)
    except subprocess.TimeoutExpired:
        raise SetupFailed("systemd-run did not start a memory-capped scope in 60 seconds.")
    if done.returncode != 0:
        raise SetupFailed("systemd-run could not start a memory-capped scope (%s): %s" % (done.returncode, (done.stderr or "").strip() or "no message"))


def run_one(tool, path):
    command = ["systemd-run", "--user", "--scope", "-p", "MemoryMax=" + MEMORY, "-p", "MemorySwapMax=0", "-q", "timeout", "-k", "5", str(LIMIT_SECONDS), tool, "roundtrip-one", path]
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
    width = len(HEADER.split("\t")) - 4
    values = fields[4:4 + width] if code in (0, 2) and len(fields) >= 4 + width else ["-"] * width
    return [path, str(code), f"{seconds:.2f}", f"{usage.ru_maxrss / 1024.0:.0f}"] + values, message


def percentile(values, p):
    values = sorted(values)
    return values[min(len(values) - 1, int(p * len(values)))] if values else 0


def main():
    if len(sys.argv) < 4:
        print(__doc__)
        return 2
    if sys.platform != "linux":
        print("crunch_roundtrip.py runs on Linux only: it caps each workbook's memory with systemd-run. `jcrunchtool roundtrip-one <file>` works on every system.", file=sys.stderr)
        return 2
    tool, keys, out_dir = sys.argv[1:4]
    try:
        check_systemd_run()
    except SetupFailed as why:
        print("crunch_roundtrip.py: %s" % why, file=sys.stderr)
        return 1
    jobs = int(sys.argv[4]) if len(sys.argv) > 4 else 6
    with open(keys, encoding="utf-8", errors="surrogateescape") as f:
        paths = [line for line in f.read().splitlines() if line]
    print(f"{len(paths)} keys", flush=True)
    os.makedirs(out_dir, exist_ok=True)
    results = []
    try:
        with ThreadPoolExecutor(max_workers=jobs) as pool:
            for n, (row, message) in enumerate(pool.map(lambda p: run_one(tool, p), paths), 1):
                results.append((row, message))
                if n % 250 == 0:
                    print(f"{n} of {len(paths)}", file=sys.stderr, flush=True)
    except SetupFailed as why:
        print("crunch_roundtrip.py: stopped after %d of %d workbooks: %s" % (len(results), len(paths), why), file=sys.stderr)
        pool.shutdown(wait=True, cancel_futures=True)
        return 1
    with open(os.path.join(out_dir, "roundtrip.tsv"), "w", encoding="utf-8", errors="surrogateescape") as f:
        f.write(HEADER + "\n")
        for row, _ in results:
            f.write("\t".join(row) + "\n")
    done = [r for r, _ in results if r[1] == "0" and r[4] != "-"]
    partial = [r for r, _ in results if r[1] == "2"]
    not_opened = [r for r, _ in results if r[1] == "1"]
    crashed = [r for r, _ in results if r[1].isdigit() and (int(r[1]) >= 128)]
    hung = [r for r, _ in results if r[1] == "124"]
    untouched_ok = [r for r in done if r[6] == "1"]
    edit_ok = [r for r in done if r[11] == "0" and r[14] == "1" and not r[15] and (r[8] != "")]
    both = [r for r in done if r[6] == "1" and r[11] == "0" and r[14] == "1" and not r[15] and r[12] == r[13]]
    parts_total = sum(int(r[4]) for r in done)
    parts_same = sum(int(r[5]) for r in done)
    edit_parts = sum(int(r[12]) for r in done)
    edit_parts_same = sum(int(r[13]) for r in done)
    kinds = collections.Counter()
    for r in done:
        for field in (r[7], r[15]):
            for kind in [k.strip() for k in field.split(";") if k.strip()]:
                kinds[kind] += 1
    with open(os.path.join(out_dir, "problems.txt"), "w", encoding="utf-8", errors="surrogateescape") as f:
        for row, message in results:
            if row[1] not in ("0", "1", "2") or (row[1] == "0" and row not in both):
                f.write(f"{row[1]}\t{row[0]}\t{row[7] if len(row) > 7 else ''}\t{row[15] if len(row) > 15 else ''}\t{message}\n")
    print(f"{len(results)} workbooks: {len(done)} saved and read back, {len(partial)} opened in part (not changed or saved), {len(not_opened)} not opened, {len(crashed)} crashed or killed, {len(hung)} timed out")
    if done:
        print("saved untouched and read back the same: %d of %d (%.2f%%)" % (len(untouched_ok), len(done), 100.0 * len(untouched_ok) / len(done)))
        print("parts the same when saved untouched: %d of %d (%.4f%%)" % (parts_same, parts_total, 100.0 * parts_same / max(1, parts_total)))
        print("with one cell edited, read back right (the cell is what was typed, what depends on it has its new result, nothing else differs): %d of %d (%.2f%%)" % (len(edit_ok), len(done), 100.0 * len(edit_ok) / len(done)))
        print("parts not meant to change, and the same: %d of %d (%.4f%%)" % (edit_parts_same, edit_parts, 100.0 * edit_parts_same / max(1, edit_parts)))
        print("both: %d of %d (%.2f%%)" % (len(both), len(done), 100.0 * len(both) / len(done)))
        deps = sum(int(r[9]) for r in done if r[9].isdigit())
        changed = sum(int(r[10]) for r in done if r[10].isdigit())
        print("formulas that read the edited cell: %d; formulas whose result changed: %d; unexpected differences: %d" % (deps, changed, sum(int(r[11]) for r in done if r[11].isdigit())))
        if kinds:
            print("commonest differences:")
            for kind, n in kinds.most_common(10):
                print("  %6d  %s" % (n, kind))
        t = [float(r[2]) for r in done]
        print("whole process (s): middle %.2f, 95th percentile %.2f, longest %.2f" % (percentile(t, 0.5), percentile(t, 0.95), max(t)))
    print(f"wrote {os.path.join(out_dir, 'roundtrip.tsv')}", flush=True)
    return 0 if not crashed and not hung else 1


if __name__ == "__main__":
    sys.exit(main())
