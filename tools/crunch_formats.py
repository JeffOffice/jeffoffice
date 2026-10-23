#!/usr/bin/env python3
"""Runs `jcrunchtool formats-one` on each workbook in a list, each in a child process, and checks the number formats against what the files stored.

    python3 tools/crunch_formats.py build/jcrunchtool keys.txt out-dir [jobs]

keys.txt has one path to a workbook on each line (paths may have spaces). Each workbook runs under `systemd-run --user --scope -p MemoryMax=4G` with 120
seconds, as in crunch_scan.py. A workbook stores the last result of every formula, so a formula that is TEXT(x, "code"), FIXED(x, n, no_commas), or
DOLLAR(x, n) (alone, or joined with other text by & or CONCATENATE), where x is a number or a single cell that holds one and the code and decimals are in the
formula, has the text a format should make of x stored beside it. Each is a case: the engine formats x and the two texts are compared.

out-dir/formats.tsv has a line for each case: workbook, cell, function, value, code, stored, ours, match (1 or 0). out-dir/codes.tsv has a line for each
format code the cells of the workbooks use (the codes of their styles, the built-in ids resolved to their codes): the code, how many cells with a value use it,
and whether the engine parses it (1 or 0), the most used first. The summary is printed: the workbooks run, the cases, the share that match, and the 10 most common
patterns of those that do not (the function, the code, and the two texts with each digit as 9), then the 30 most used codes. The cases are of three
kinds: the results of TEXT, FIXED, and DOLLAR (the yardstick of the number formats); DATEVALUE of such a text, whose number is stored (a check that the text
reads back as the same day); and a join of text with a cell that holds a number, whose text is General's.
"""
import os
import re
import subprocess
import sys
import tempfile
import time
from collections import Counter
from concurrent.futures import ThreadPoolExecutor

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from crunch_scan import LIMIT_SECONDS, SetupFailed, check_systemd_run, need_linux  # noqa: E402


def unescape(text):
    return re.sub(r"\\([tnr\\])", lambda m: {"t": "\t", "n": "\n", "r": "\r", "\\": "\\"}[m.group(1)], text)


def run_one(tool, path):
    command = ["systemd-run", "--user", "--scope", "-p", "MemoryMax=4G", "-p", "MemorySwapMax=0", "-q",
               "timeout", "-k", "5", str(LIMIT_SECONDS), tool, "formats-one", path]
    env = dict(os.environ, JP_NO_TELEMETRY="1", QT_QPA_PLATFORM="offscreen")
    with tempfile.TemporaryFile() as errors:
        child = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=errors, env=env)
        output = child.stdout.read()
        _, status, _ = os.wait4(child.pid, 0)
        errors.seek(0)
        message = errors.read(2000).decode("utf-8", "replace").strip().replace("\n", " | ")
    code = os.waitstatus_to_exitcode(status)
    if code < 0:
        code = 128 - code
    if code in (126, 127) or (code == 1 and message and not output and path not in message):
        raise SetupFailed("the run could not start the program (exit %d): %s" % (code, message or "no message"))
    cases, uses = [], []
    for line in output.decode("utf-8", "replace").splitlines():
        fields = line.split("\t")
        if fields[0] == "case" and len(fields) == 8:
            cases.append([path] + [unescape(f) for f in fields[1:]])
        elif fields[0] == "use" and len(fields) == 4:
            uses.append((unescape(fields[1]), int(fields[2]), fields[3] == "1"))
    return path, code, cases, uses, message


def abstract(text):
    return re.sub(r"[0-9]", "9", text)


def main():
    if len(sys.argv) < 4:
        print(__doc__)
        return 2
    if not need_linux():
        return 2
    tool, keys, out_dir = sys.argv[1:4]
    try:
        check_systemd_run()
    except SetupFailed as why:
        print("crunch_formats.py: %s" % why, file=sys.stderr)
        return 1
    jobs = int(sys.argv[4]) if len(sys.argv) > 4 else 8
    with open(keys, encoding="utf-8", errors="surrogateescape") as f:
        paths = [line for line in f.read().splitlines() if line]
    print(f"{len(paths)} keys", flush=True)
    os.makedirs(out_dir, exist_ok=True)
    started = time.time()
    results = []
    try:
        with ThreadPoolExecutor(max_workers=jobs) as pool:
            for n, result in enumerate(pool.map(lambda p: run_one(tool, p), paths), 1):
                results.append(result)
                if n % 500 == 0:
                    print(f"{n} of {len(paths)}", file=sys.stderr, flush=True)
    except SetupFailed as why:
        print("crunch_formats.py: stopped after %d of %d workbooks: %s" % (len(results), len(paths), why), file=sys.stderr)
        pool.shutdown(wait=True, cancel_futures=True)
        return 1
    opened = crashed = timed_out = not_opened = 0
    cases = []
    code_cells = Counter()
    code_parses = {}
    books_with_cases = set()
    for path, code, book_cases, uses, message in results:
        if code == 124:
            timed_out += 1
        elif code >= 128:
            crashed += 1
        elif code == 1:
            not_opened += 1
        else:
            opened += 1
        if book_cases:
            books_with_cases.add(path)
        cases.extend(book_cases)
        for text, count, parses in uses:
            code_cells[text] += count
            code_parses[text] = parses
    with open(os.path.join(out_dir, "formats.tsv"), "w", encoding="utf-8", errors="surrogateescape") as f:
        f.write("workbook\tcell\tfunction\tvalue\tcode\tstored\tours\tmatch\n")
        for c in cases:
            f.write("\t".join(x.replace("\t", "\\t").replace("\n", "\\n").replace("\r", "\\r") for x in c) + "\n")
    with open(os.path.join(out_dir, "codes.tsv"), "w", encoding="utf-8", errors="surrogateescape") as f:
        f.write("code\tcells\tparses\n")
        for text, count in code_cells.most_common():
            f.write("%s\t%d\t%d\n" % (text.replace("\t", "\\t").replace("\n", "\\n"), count, 1 if code_parses[text] else 0))
    def group(c):
        return "wrapped" if c[2].startswith("DATEVALUE(") else "join" if c[2] == "&" else "direct"
    totals, matches = Counter(group(c) for c in cases), Counter(group(c) for c in cases if c[7] == "1")
    by_function = Counter(c[2] for c in cases)
    by_function_match = Counter(c[2] for c in cases if c[7] == "1")
    patterns = Counter((c[2], c[4], abstract(c[6]), abstract(c[5])) for c in cases if c[7] != "1")

    def share(name):
        return f"{matches[name]}/{totals[name]} ({100.0 * matches[name] / totals[name]:.2f}%)" if totals[name] else "0/0"
    parts = ", ".join(f"{fn} {by_function_match[fn]}/{n}" for fn, n in sorted(by_function.items()))
    top = "; ".join("%d x %s %r: ours %r, stored %r" % (n, fn, code, ours, stored) for (fn, code, ours, stored), n in patterns.most_common(10))
    print(f"formats: workbooks {len(paths)} (opened {opened}, not opened {not_opened}, crashed {crashed}, timed out {timed_out}); cases in {len(books_with_cases)} workbooks: "
          f"TEXT/FIXED/DOLLAR results {share('direct')}, DATEVALUE of them {share('wrapped')}, numbers joined as General {share('join')}; by function [{parts}]; "
          f"{len(patterns)} mismatch patterns, top 10: {top or 'none'}; {time.time() - started:.0f} s")
    print("top 30 format codes by cells with a value:")
    for text, count in code_cells.most_common(30):
        print("  %10d  %s  %s" % (count, "parses" if code_parses[text] else "NOT PARSED", text))
    unparsed = [t for t in code_cells if not code_parses[t]]
    print(f"{len(code_cells)} distinct codes, {len(unparsed)} that the engine does not parse, used by {sum(code_cells[t] for t in unparsed)} cells")
    print(f"wrote {os.path.join(out_dir, 'formats.tsv')}", flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
