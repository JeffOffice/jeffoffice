#!/usr/bin/env python3
"""Does a workbook that JeffCrunch SAVES open again cleanly? One check for each workbook of the public collections and each format JeffCrunch saves (.xlsx, .xls, .ods).

    python3 tools/crunch_save_checks.py run <jcrunchtool> <out-dir> (--sample N | --all) [--hours H] [--jobs J] [--lo-per-kind N] [--only-list file]
    python3 tools/crunch_save_checks.py report <out-dir>

For each workbook and format, `jcrunchtool copy-check` (tools/jcrunchtool_copycheck.cpp) runs in a process of its own with 120 seconds and 4 GB: it opens the workbook as the window does,
saves it in the format, reads the saved file again with the program's own readers, and compares every cell's value and formula text, the sheet names, the extents, the sizes set by hand of
columns and rows, and what a person sees of each cell's format; it prints the kinds of difference. A crash, a hang (the time limit), a kill by the memory cap, and a refusal to open are
recorded as such, never as a difference. If LibreOffice is installed (soffice), each saved file is then opened there headless and the result ("loads" or "fails") is recorded; if not, that
column stays empty.

The workbooks are those listed in /home/user/jo-corpora/work/lists (Enron, EUSES, Govdocs1, Apache POI's test files, LibreOffice's test files: .xlsx and .xls) and the .ods files of
LibreOffice's test folder. --sample N takes N workbooks of each kind (.xlsx, .xls, .ods), chosen by a fixed seed (tools/save_check_common.py SEED), each collection in turn so that all are in it;
--all takes every workbook in the same order, until --hours of machine time have gone (a workbook that was begun is finished). A run skips the results it already has in
<out-dir>/crunch-results.jsonl, so --sample 200 and then --all go on from where they were; --only-list names a file of paths to run again (the results of those are replaced).
--lo-per-kind N limits the LibreOffice check to the first N workbooks of each kind in that order, 600 by default, since it takes a second or more for each
file. Nothing of the collections is kept: the copies are written in <out-dir>/work, opened in LibreOffice, and deleted unless they differ. The report is <out-dir>/crunch-section.md, and
REPORT.md when the other app's section is there too.
"""
import hashlib
import os
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import save_check_common as C  # noqa: E402

FORMATS = ["xlsx", "xls", "ods"]
SOURCES = ["xlsx", "xls", "ods"]
EXPECTED = {"formula written as a value"}   # (what the older forms cannot hold: not a fault of the save)


def collections_by_source():
    out = {"xlsx": {}, "xls": {}, "ods": {}}
    for name in ("enron-xlsx", "poi-xlsx", "lo-xlsx"):
        out["xlsx"][name] = C.read_list(name)
    for name in ("enron-xls", "euses-xls", "govdocs-xls", "poi-xls", "lo-xls"):
        out["xls"][name] = C.read_list(name)
    out["ods"]["libreoffice-ods"] = C.find_files(C.CORPORA + "/libreoffice", {"ods"}) + C.find_files(C.CORPORA + "/poi", {"ods"})
    return out


def parse(line):
    f = line.rstrip("\n").split("\t")
    f += [""] * (24 - len(f))
    return f


def main():
    args = sys.argv[1:]
    if len(args) >= 2 and args[0] == "report":
        return report(args[1])
    if len(args) < 4 or args[0] != "run":
        print(__doc__)
        return 2
    tool, out_dir = os.path.abspath(args[1]), os.path.abspath(args[2])
    opts = args[3:]
    sample = int(opts[opts.index("--sample") + 1]) if "--sample" in opts else None
    hours = float(opts[opts.index("--hours") + 1]) if "--hours" in opts else 2.0
    jobs = int(opts[opts.index("--jobs") + 1]) if "--jobs" in opts else 6
    lo_per_kind = int(opts[opts.index("--lo-per-kind") + 1]) if "--lo-per-kind" in opts else 600
    only = set(l.strip() for l in open(opts[opts.index("--only-list") + 1])) if "--only-list" in opts else None
    if sample is None and "--all" not in opts:
        print(__doc__)
        return 2
    work = os.path.join(out_dir, "work")
    os.makedirs(work, exist_ok=True)
    results = C.Results(os.path.join(out_dir, "crunch-results.jsonl"))
    deadline = time.time() + hours * 3600
    items = []
    n = 0
    for src, cols in collections_by_source().items():
        order = C.ordered(cols)
        if sample is not None:
            order = order[:sample]
        for rank, (col, path) in enumerate(order):
            n += 1
            if only is not None and path not in only:
                continue
            for fmt in FORMATS:
                if only is not None or not results.has(path, fmt):
                    items.append((hashlib.sha1(path.encode()).hexdigest()[:12], col, src, path, fmt, rank))
    print("workbooks to check: %d files, %d runs" % (n, len(items)), flush=True)

    def run_item(item):
        idx, col, src, path, fmt, rank = item
        keep = os.path.join(work, "%s-%s.%s" % (idx, fmt, fmt))
        started = time.time()
        kind, code, out, err = C.run_capped([tool, "copy-check", path, fmt], {"COPY_CHECK_KEEP": keep})
        rec = {"path": path, "collection": col, "src": src, "fmt": fmt, "status": "", "kinds": [], "first": "", "seconds": round(time.time() - started, 2), "bytes": 0, "keep": keep, "rank": rank, "lo": ""}
        line = next((l for l in out.splitlines() if l.startswith("copy-check\t")), "")
        if kind != "done" or not line:
            rec["status"] = kind if kind != "done" else "crash"
            rec["first"] = err[:200]
        else:
            f = parse(line)
            res = f[3]
            rec["bytes"] = int(f[4]) if f[4].isdigit() else 0
            if res.startswith("not opened"):
                rec["status"], rec["first"] = "refused", res[len("not opened: "):]
            elif res.startswith("save failed"):
                rec["status"], rec["first"] = "not saved", res[len("save failed: "):]
            elif res.startswith("the copy does not open"):
                rec["status"], rec["first"] = "reopen failed", res[len("the copy does not open: "):]
            elif res in ("ok", "differs"):
                rec["status"] = res
                rec["kinds"] = [k for k in f[23].split("; ") if k]
                rec["first"] = f[21][:200]
            else:
                rec["status"], rec["first"] = "crash", res[:200]
        results.add(rec)

    ran = C.drive(items, run_item, jobs, deadline, "crunch")
    print("crunch runs done: %d of %d" % (ran, len(items)), flush=True)
    # LibreOffice, on the copies that were saved
    lo = C.LibreOffice(work, "csv")
    todo = [r for r in results.done.values() if r["status"] in ("ok", "differs", "reopen failed") and not r["lo"] and os.path.exists(r.get("keep", "")) and r.get("rank", 0) < lo_per_kind]
    if lo.available():
        batches = [todo[i:i + C.LO_BATCH] for i in range(0, len(todo), C.LO_BATCH)]

        def run_batch(batch):
            res = lo.check([r["keep"] for r in batch])
            for r in batch:
                r = dict(r, lo=res.get(r["keep"], ""))
                results.add(r)
                if r["lo"] == "loads" and r["status"] == "ok":
                    try:
                        os.unlink(r["keep"])
                    except OSError:
                        pass

        C.drive(batches, run_batch, 2, time.time() + 4 * 3600, "libreoffice")
    else:
        print("LibreOffice (soffice) is not installed: its column stays empty", flush=True)
    return report(out_dir)


def report(out_dir):
    results = C.load_results(os.path.join(out_dir, "crunch-results.jsonl"))
    lo = C.soffice_path() is not None
    lines = C.report_app("JeffCrunch", results, FORMATS, SOURCES, EXPECTED, lo,
                         ["How to read the table: a file is \"opened\" when the program reads it, \"saved\" when the saver makes a file, \"reopened\" when the program's own reader reads the saved file, and "
                          "\"the same\" when every cell's value and formula text, the sheet names, the extents, the sizes set by hand, and the cell formats compare equal. \"Format limits aside\" also counts "
                          "files whose only difference is a formula the older form cannot hold, saved as its value (named in the program's own message before saving).", ""])
    open(os.path.join(out_dir, "crunch-section.md"), "w").write("\n".join(lines) + "\n")
    print("\n".join(l for l in lines if "|" in l and not l.startswith("---"))[:6000])
    C.assemble(out_dir)
    return 0


if __name__ == "__main__":
    sys.exit(main())
