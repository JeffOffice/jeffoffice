#!/usr/bin/env python3
"""Runs each self-checking function file (LibreOffice's, one function to a file) through `jcrunchtool recalc-full-one` and reads its result cell.

    python3 tools/crunch_function_files.py build/jcrunchtool /path/to/sc/qa/unit/data/functions out.tsv [jobs]

The folder is searched for .fods and .xlsx files; nothing is copied. A .fods function file has a row whose first cell reads "Result" and, to its right, one or more
cells that join the test rows with AND(): TRUE when every test row agrees. The whole workbook is recalculated from its inputs and each of those cells is compared with TRUE.
A file with no such row (the two .xlsx files lay their checks out otherwise) passes when every formula cell works out to what the file stored.
out.tsv has a line for each file: file, function (the file's name), result (pass, fail, error, timeout, or crash), seconds, and a detail (the result cell that failed, the
functions the engine lacks, the first formula whose own result differed, or the error message). Each file runs in a child process with 60 seconds and 4 GB (systemd-run), 4 at a time. Totals are printed.
"""
import os
import re
import subprocess
import sys
import tempfile
import time
import xml.etree.ElementTree as ET
from collections import Counter
from concurrent.futures import ThreadPoolExecutor

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from crunch_scan import SetupFailed, check_systemd_run, need_linux  # noqa: E402

LIMIT = 60
OUTPUT_CAP = 50 * 1024 * 1024
NS_TABLE = "{urn:oasis:names:tc:opendocument:xmlns:table:1.0}"
NS_TEXT = "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}"
NS_OFFICE = "{urn:oasis:names:tc:opendocument:xmlns:office:1.0}"


def run_capped(command, seconds=LIMIT, memory="4G"):
    """Runs a command in a memory-capped scope under a time limit. Returns (kind, exit code, seconds, stdout bytes, stderr text); kind is ok, timeout, or crash."""
    home = tempfile.mkdtemp(prefix="jcrunch-home-")
    env = dict(os.environ, HOME=home, XDG_CONFIG_HOME=home, JP_NO_TELEMETRY="1", QT_QPA_PLATFORM="offscreen")
    full = ["systemd-run", "--user", "--scope", "-p", "MemoryMax=" + memory, "-p", "MemorySwapMax=0", "-q",
            "timeout", "-k", "5", str(seconds)] + command
    started = time.time()
    try:
        with tempfile.TemporaryFile() as errors:
            child = subprocess.Popen(full, stdout=subprocess.PIPE, stderr=errors, env=env, stdin=subprocess.DEVNULL)
            data = child.stdout.read(OUTPUT_CAP)
            while child.stdout.read(1 << 20):   # (anything past the cap is read and dropped)
                pass
            _, status, _ = os.wait4(child.pid, 0)
            errors.seek(0)
            message = errors.read(2000).decode("utf-8", "replace").strip().replace("\n", " | ")
    finally:
        subprocess.run(["rm", "-rf", "--", home], check=False)
    took = time.time() - started
    code = os.waitstatus_to_exitcode(status)
    if code < 0:
        code = 128 - code
    if code in (126, 127):
        raise SetupFailed("the run could not start the program (exit %d): %s" % (code, message or "no message"))
    if code == 124 or (code >= 128 and took >= seconds):
        return "timeout", code, took, data, message
    if code >= 128:
        return "crash", code, took, data, message
    return "ok", code, took, data, message


def column_name(n):
    s = ""
    n += 1
    while n:
        n, r = divmod(n - 1, 26)
        s = chr(65 + r) + s
    return s


def result_cells(path):
    """The cells of a .fods file that sum up its test rows: [(sheet name, A1 reference)]; empty when there is no row labelled Result."""
    found = []
    row = 0
    sheet = ""
    for event, el in ET.iterparse(path, events=("start", "end")):
        if event == "start":
            if el.tag == NS_TABLE + "table":
                sheet = el.get(NS_TABLE + "name", "")
                row = 0
            continue
        if el.tag == NS_TABLE + "table-row":
            repeat = int(el.get(NS_TABLE + "number-rows-repeated", "1"))
            col = 0
            label = False
            for cell in el:
                if cell.tag not in (NS_TABLE + "table-cell", NS_TABLE + "covered-table-cell"):
                    continue
                crep = int(cell.get(NS_TABLE + "number-columns-repeated", "1"))
                if col == 0:
                    text = "".join(p.text or "" for p in cell.iter(NS_TEXT + "p")).strip()
                    label = text.lower().startswith("result") or text.lower() == "expected result"
                elif label and cell.get(NS_TABLE + "formula") and crep == 1 and cell.get(NS_OFFICE + "value-type") == "boolean":
                    found.append((sheet, column_name(col) + str(row + 1)))
                col += crep
            row += repeat
            el.clear()
    return found


def parse_rows(data):
    rows = {}
    summary = None
    unsupported = []
    for line in data.decode("utf-8", "replace").splitlines():
        f = line.split("\t")
        if f[0] == "row" and len(f) == 8:
            rows[(f[1], f[2])] = f[3:]
        elif f[0] == "unsup" and len(f) == 3:
            unsupported.append(f[1])
        elif f[0] == "fullbook":
            summary = f[1:]
    return rows, unsupported, summary


def check_file(tool, path):
    name = os.path.splitext(os.path.basename(path))[0]
    try:
        cells = result_cells(path) if path.endswith(".fods") else []
    except ET.ParseError as why:
        return path, name, "error", 0.0, "xml: %s" % why
    kind, code, took, data, message = run_capped([tool, "recalc-full-one", path, "100000000", "50"])
    if kind != "ok":
        return path, name, kind, took, "exit %d %s" % (code, message[:200])
    rows, unsupported, summary = parse_rows(data)
    if summary is None:
        return path, name, "error", took, (message or "no result from the tool")[:300]
    detail = []
    if unsupported:
        detail.append("unsupported: " + ",".join(sorted(set(unsupported))))
    if not cells:
        # (no result row: every formula must work out to what the file stored)
        bad = [r for r in rows.values() if r[0] not in ("M", "V", "E", "N")]
        if bad:
            detail.append("%d formula cells differ, e.g. class %s %s" % (len(bad), bad[0][0], bad[0][1][:60]))
        return path, name, "fail" if bad else "pass", took, "; ".join(["no Result row"] + detail)
    failed = 0
    roots = [r for r in rows.values() if r[0] == "X"]   # (the mismatches no other mismatch led to)
    if roots:
        detail.append("%d root mismatches, e.g. [%s] %s stored %s ours %s" % (len(roots), roots[0][4], roots[0][1][:60], roots[0][2][:20], roots[0][3][:20]))
    for key in cells:
        r = rows.get(key)
        ours = r[3] if r else None
        if r is None or r[0] != "M":
            failed += 1
            detail.append("%s!%s class %s ours %s" % (key[0], key[1], r[0] if r else "-", (ours if ours is not None else "none")[:40]))
    return path, name, "fail" if failed else "pass", took, "; ".join(detail)[:400]


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
        print("crunch_function_files.py: %s" % why, file=sys.stderr)
        return 1
    paths = sorted(os.path.join(d, f) for d, _, fs in os.walk(folder) for f in fs if f.endswith((".fods", ".xlsx")))
    print("%d function files" % len(paths), flush=True)
    results = []
    try:
        with ThreadPoolExecutor(max_workers=jobs) as pool:
            for n, r in enumerate(pool.map(lambda p: check_file(tool, p), paths), 1):
                results.append(r)
                if n % 100 == 0:
                    print("%d of %d" % (n, len(paths)), file=sys.stderr, flush=True)
    except SetupFailed as why:
        print("crunch_function_files.py: stopped after %d of %d files: %s" % (len(results), len(paths), why), file=sys.stderr)
        return 1
    totals = Counter()
    with open(out, "w", encoding="utf-8") as f:
        f.write("file\tfunction\tresult\tseconds\tdetail\n")
        for path, name, result, took, detail in results:
            totals[result] += 1
            f.write("%s\t%s\t%s\t%.2f\t%s\n" % (os.path.relpath(path, folder), name, result, took, detail.replace("\t", " ").replace("\n", " ")))
    print("function files: %d; %s" % (len(results), ", ".join("%s %d" % (k, totals[k]) for k in ("pass", "fail", "error", "timeout", "crash"))))
    print("wrote %s" % out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
