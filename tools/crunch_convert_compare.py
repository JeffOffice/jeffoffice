#!/usr/bin/env python3
"""The yardstick of the .ods and .csv readers: workbooks of the practice list converted by LibreOffice to .ods and to .csv (the first sheet), and what the readers get from each
compared with what the .xlsx reader gets from the original.

    python3 tools/crunch_convert_compare.py convert <keys.txt> <out-dir> <count> [jobs]
        takes every Nth path of keys.txt (one path to a line, paths may have spaces; N so that at least `count` are taken), converts each with LibreOffice (several at a time,
        each program with a profile of its own, a batch under a time limit) into out-dir/ods/files and out-dir/csv/files, and writes out-dir/ods/pairs.tsv and
        out-dir/csv/pairs.tsv (the converted file, a tab, the original). Nothing is written in the repository. The .csv holds the values as they are, not as shown (numbers are
        exact), with dates and times as the cell shows them.
    python3 tools/crunch_convert_compare.py compare ods|csv <jcrunchtool> <out-dir> [jobs]
        runs `jcrunchtool compare-one` for each pair (for .csv: the first sheet only, values only; in the locale en_US) and prints the shares that are the same and the commonest
        differences, as crunch_xls_compare.py does for .xls (it is the same summary: out-dir/ods/diffs.tsv and books.tsv, or out-dir/csv/...).
"""
import importlib.util
import os
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
BATCH = 20
CSV_FILTER = "csv:Text - txt - csv (StarCalc):44,34,76,1,,0,false,false,false,false,false,0"


def convert_batch(profile, kind, files, out_dir):
    """Converts the staged files with one LibreOffice process; returns how many outputs exist."""
    command = ["systemd-run", "--user", "--scope", "-p", "MemoryMax=8G", "-q", "timeout", "-k", "10", "900", "soffice", "--headless", "--norestore", "-env:UserInstallation=file://" + profile,
               "--convert-to", "ods" if kind == "ods" else CSV_FILTER, "--outdir", out_dir] + files
    subprocess.run(command, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, env=dict(os.environ, HOME=os.environ.get("HOME", "/tmp")))


def convert(keys_file, out_dir, count, jobs):
    keys = [l.rstrip("\n") for l in open(keys_file) if l.strip()]
    step = max(1, len(keys) // count)
    chosen = keys[::step]
    stage = os.path.join(out_dir, "stage")
    for sub in ("stage", "ods/files", "csv/files", "profiles"):
        os.makedirs(os.path.join(out_dir, sub), exist_ok=True)
    names = []
    for i, path in enumerate(chosen):
        name = "%05d" % i
        link = os.path.join(stage, name + ".xlsx")
        if not os.path.lexists(link):
            os.symlink(path, link)
        names.append(name)
    batches = [names[i:i + BATCH] for i in range(0, len(names), BATCH)]
    todo = [(kind, b) for kind in ("ods", "csv") for b in batches]

    def work(item):
        n, (kind, batch) = item
        profile = os.path.join(out_dir, "profiles", "p%d" % (n % jobs))
        files = [os.path.join(stage, name + ".xlsx") for name in batch]
        convert_batch(profile, kind, files, os.path.join(out_dir, kind, "files"))
        # a file the batch did not get to (the process died or ran out of time) is tried alone
        for name in batch:
            if not os.path.exists(os.path.join(out_dir, kind, "files", name + "." + kind)):
                convert_batch(profile, kind, [os.path.join(stage, name + ".xlsx")], os.path.join(out_dir, kind, "files"))

    # (a profile is used by one process at a time: the work is dealt out by the position in the list, `jobs` ways)
    lanes = [[] for _ in range(jobs)]
    for n, item in enumerate(todo):
        lanes[n % jobs].append((n, item))

    def lane(items):
        for item in items:
            work(item)

    with ThreadPoolExecutor(jobs) as ex:
        list(ex.map(lane, lanes))
    for kind in ("ods", "csv"):
        with open(os.path.join(out_dir, kind, "pairs.tsv"), "w") as f:
            for name, path in zip(names, chosen):
                f.write(os.path.join(out_dir, kind, "files", name + "." + kind) + "\t" + path + "\n")
        made = sum(1 for name in names if os.path.exists(os.path.join(out_dir, kind, "files", name + "." + kind)))
        print(kind, "converted", made, "of", len(names))


def compare(kind, tool, out_dir, jobs):
    spec = importlib.util.spec_from_file_location("crunch_xls_compare", os.path.join(HERE, "crunch_xls_compare.py"))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    flags = ["--first-sheet", "--values-only"] if kind == "csv" else []

    def run_one(tool, ours, theirs):
        command = ["systemd-run", "--user", "--scope", "-p", "MemoryMax=4G", "-p", "MemorySwapMax=0", "-q", "timeout", "-k", "5", str(module.LIMIT_SECONDS), tool, "compare-one"] + flags + [ours, theirs]
        env = dict(os.environ, JP_NO_TELEMETRY="1", QT_QPA_PLATFORM="offscreen", LANG="en_US.UTF-8", LC_ALL="en_US.UTF-8")
        with tempfile.TemporaryFile() as errors:
            child = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=errors, env=env)
            output = child.stdout.read().decode("utf-8", "replace")
            child.wait()
        return child.returncode, output

    module.run_one = run_one
    sys.argv = ["crunch_xls_compare.py", tool, os.path.join(out_dir, kind, "pairs.tsv"), os.path.join(out_dir, kind), str(jobs)]
    return module.main()


def main():
    if len(sys.argv) >= 5 and sys.argv[1] == "convert":
        convert(sys.argv[2], sys.argv[3], int(sys.argv[4]), int(sys.argv[5]) if len(sys.argv) > 5 else 6)
        return 0
    if len(sys.argv) >= 5 and sys.argv[1] == "compare" and sys.argv[2] in ("ods", "csv"):
        return compare(sys.argv[2], sys.argv[3], sys.argv[4], int(sys.argv[5]) if len(sys.argv) > 5 else 8)
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main())
