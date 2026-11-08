#!/usr/bin/env python3
"""Opens each file in a folder of spreadsheet test files with `jcrunchtool info --tsv` and records whether it opened and how many cells it loaded.

    python3 tools/crunch_format_files.py build/jcrunchtool /path/to/sc/qa/unit/data out.tsv [jobs]

Built for LibreOffice's Calc test folder (kept on the machine, never in the repository). The folder is searched below, by extension: .xlsx, .ods, .xls, .xlsb, .xlsm, .csv,
and .fods, and files in other forms (.slk, .wks, .wb2, .dif, .dbf, .123, .gnumeric, .html) with the later formats. Nothing is copied.
out.tsv has a line for each file: file, kind (the extension), result (opened, error, timeout, or crash; error includes a file that opened but a sheet of it could not be read whole, with the cells that were read), cells, seconds, and a detail (the error message, or the problems
a sheet reported while it was read). Each file runs in a child process with 60 seconds and 4 GB (systemd-run), 4 at a time. Totals are printed for each kind.
"""
import os
import sys
from collections import Counter, defaultdict
from concurrent.futures import ThreadPoolExecutor

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from crunch_function_files import run_capped  # noqa: E402
from crunch_scan import SetupFailed, check_systemd_run, need_linux  # noqa: E402

EXTENSIONS = (".xlsx", ".ods", ".xls", ".xlsb", ".xlsm", ".csv", ".fods", ".slk", ".wks", ".wb2", ".dif", ".dbf", ".123", ".gnumeric", ".html")


def check_file(tool, path):
    kind, code, took, data, message = run_capped([tool, "info", "--tsv", path])
    if kind != "ok":
        return path, kind, 0, took, ("exit %d %s" % (code, message))[:300]
    f = data.decode("utf-8", "replace").strip().split("\t")
    if len(f) >= 6 and code == 2:   # (it opened, and a sheet could not be read whole: the cells that were read are counted)
        return path, "error", int(f[1]), took, f[5][:300]
    if code != 0 or len(f) < 6:
        return path, "error", 0, took, (message or "exit %d" % code).replace(path, "")[:300].strip(": ")
    return path, "opened", int(f[1]), took, f[5]


def main():
    if len(sys.argv) < 4:
        print(__doc__)
        return 2
    if not need_linux():
        return 2
    tool, folder, out = sys.argv[1:4]
    jobs = int(sys.argv[4]) if len(sys.argv) > 4 else 4
    try:
        check_systemd_run()
    except SetupFailed as why:
        print("crunch_format_files.py: %s" % why, file=sys.stderr)
        return 1
    paths = sorted(os.path.join(d, f) for d, _, fs in os.walk(folder) for f in fs if f.lower().endswith(EXTENSIONS))
    print("%d files" % len(paths), flush=True)
    results = []
    try:
        with ThreadPoolExecutor(max_workers=jobs) as pool:
            for n, r in enumerate(pool.map(lambda p: check_file(tool, p), paths), 1):
                results.append(r)
                if n % 200 == 0:
                    print("%d of %d" % (n, len(paths)), file=sys.stderr, flush=True)
    except SetupFailed as why:
        print("crunch_format_files.py: stopped after %d of %d files: %s" % (len(results), len(paths), why), file=sys.stderr)
        return 1
    by_kind = defaultdict(Counter)
    with open(out, "w", encoding="utf-8") as f:
        f.write("file\tkind\tresult\tcells\tseconds\tdetail\n")
        for path, result, cells, took, detail in results:
            ext = os.path.splitext(path)[1].lower()
            by_kind[ext][result] += 1
            f.write("%s\t%s\t%s\t%d\t%.2f\t%s\n" % (os.path.relpath(path, folder), ext, result, cells, took, detail.replace("\t", " ").replace("\n", " ")))
    for ext in sorted(by_kind):
        c = by_kind[ext]
        print("%-9s %4d: %s" % (ext, sum(c.values()), ", ".join("%s %d" % (k, c[k]) for k in ("opened", "error", "timeout", "crash") if c[k])))
    total = Counter()
    for c in by_kind.values():
        total.update(c)
    print("format files: %d; %s" % (len(results), ", ".join("%s %d" % (k, total[k]) for k in ("opened", "error", "timeout", "crash"))))
    print("wrote %s" % out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
