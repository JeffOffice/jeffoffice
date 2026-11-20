#!/usr/bin/env python3
"""Does a document that JeffWrite SAVES open again cleanly? One check for each document of the public collections, saved as .docx and as .odt (the formats JeffWrite saves).

    python3 tools/write_save_checks.py run <jwritetool> <out-dir> (--sample N | --all) [--hours H] [--jobs J] [--lo-per-kind N] [--only-list file]
    python3 tools/write_save_checks.py report <out-dir>

For each document, `jwritetool save-check` (tools/jwritetool.cpp) runs in a process of its own with 120 seconds and 4 GB: it opens the file as JeffWrite does, saves it as .docx and as .odt,
reads each saved file again with the program's own reader, and compares what a person would see: the text of every paragraph in order, the pages, and how many tables, pictures (drawn
objects), and notes there are. A crash, a hang (the time limit), a kill by the memory cap, and a refusal to open are recorded as such, never as a difference. If LibreOffice is installed
(soffice), each saved file is then opened there headless and the result ("loads" or "fails") is recorded; if not, that column stays empty. (The same test in the developer hooks of the test
suite, JW_ROUNDTRIP_IN and JW_STRICT_IN in tests/write/tst_write_03.cpp, checks the .docx parts' XML and what the file held, in one process for many files; this one is for the numbers of the
feature list.)

The documents are those listed in /home/user/jo-corpora/work/lists (Govdocs1 .doc, Apache POI's test .doc and .docx, the .rtf of Govdocs1, and the .docx that LibreOffice made from the Govdocs1
and POI .doc files, which stand for a .docx of another program). --sample N takes N documents of each kind (.docx, .doc, .rtf), chosen by a fixed seed (tools/save_check_common.py SEED),
each collection in turn; --all takes every document in the same order, until --hours of machine time have gone. A run skips the results it already has in
<out-dir>/write-results.jsonl; --only-list names a file of paths to run again (their results are replaced). --lo-per-kind N limits the LibreOffice check to the first N workbooks of each kind in that order, 600 by default, since it takes a second or more for each
file. Nothing of the collections is kept: the saved files are written in <out-dir>/work,
opened in LibreOffice, and deleted unless they differ. The report is <out-dir>/write-section.md, and REPORT.md when the other app's section is there too.
"""
import hashlib
import os
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import save_check_common as C  # noqa: E402

FORMATS = ["docx", "odt"]
SOURCES = ["docx", "doc", "rtf"]
EXPECTED = set()


def collections_by_source():
    out = {"docx": {}, "doc": {}, "rtf": {}}
    out["docx"]["poi-docx"] = C.read_list("poi-docx")
    out["docx"]["lo-made-docx (from Govdocs1 .doc)"] = C.read_list("conv-govdocs-doc")
    out["docx"]["lo-made-docx (from POI .doc)"] = C.read_list("conv-poi-doc")
    out["doc"]["govdocs-doc"] = C.read_list("govdocs-doc")
    out["doc"]["poi-doc"] = C.read_list("poi-doc")
    out["rtf"]["govdocs-rtf"] = C.find_files(C.CORPORA + "/work/x/govdocs", {"rtf"})
    return out


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
    results = C.Results(os.path.join(out_dir, "write-results.jsonl"))
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
            if only is not None or not all(results.has(path, f) for f in FORMATS):
                items.append((hashlib.sha1(path.encode()).hexdigest()[:12], col, src, path, rank))
    print("documents to check: %d" % len(items), flush=True)
    home = os.path.join(work, "home")
    os.makedirs(home, exist_ok=True)

    def run_item(item):
        idx, col, src, path, rank = item
        started = time.time()
        kind, code, out, err = C.run_capped([tool, "save-check", path, work],
                                            {"JW_SAVE_CHECK_NAME": idx, "HOME": home, "XDG_CONFIG_HOME": home + "/config", "XDG_DATA_HOME": home + "/data", "XDG_CACHE_HOME": home + "/cache"})
        seconds = round(time.time() - started, 2)
        lines = {}
        for l in out.splitlines():
            if l.startswith("save-check\t"):
                f = l.split("\t") + [""] * 12
                lines[f[2]] = f
        for fmt in FORMATS:
            rec = {"path": path, "collection": col, "src": src, "fmt": fmt, "status": "", "kinds": [], "first": "", "seconds": seconds, "rank": rank, "bytes": 0, "keep": os.path.join(work, "%s-%s.%s" % (idx, fmt, fmt)), "lo": ""}
            f = lines.get(fmt)
            if kind != "done" and not f:
                rec["status"], rec["first"] = kind, err[:200]
            elif not f:
                rec["status"], rec["first"] = "crash", err[:200] or "exit code %d" % code
            else:
                res = f[3]
                rec["bytes"] = int(f[4]) if f[4].isdigit() else 0
                if res == "not opened":
                    rec["status"], rec["first"] = "refused", f[4][:200]
                    rec["bytes"] = 0
                elif res == "not saved":
                    rec["status"], rec["first"] = "not saved", f[4][:200]
                    rec["bytes"] = 0
                elif res == "does not open again":
                    rec["status"], rec["first"] = "reopen failed", f[11][:200]
                else:
                    rec["status"] = res
                    rec["kinds"] = [k for k in f[10].split("; ") if k]
                    rec["first"] = f[11][:200]
            results.add(rec)

    ran = C.drive(items, run_item, jobs, deadline, "write")
    print("write documents done: %d of %d" % (ran, len(items)), flush=True)
    lo = C.LibreOffice(work, "txt:Text (encoded):UTF8")
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
    results = C.load_results(os.path.join(out_dir, "write-results.jsonl"))
    lo = C.soffice_path() is not None
    lines = C.report_app("JeffWrite", results, FORMATS, SOURCES, EXPECTED, lo,
                         ["How to read the table: a file is \"opened\" when the program reads it, \"saved\" when the saver makes a file, \"reopened\" when the program's own reader reads the saved file, and "
                          "\"the same\" when the text of every paragraph in order, the page count, and the counts of tables, pictures, and notes compare equal.", ""])
    open(os.path.join(out_dir, "write-section.md"), "w").write("\n".join(lines) + "\n")
    print("\n".join(l for l in lines if "|" in l and not l.startswith("---"))[:6000])
    C.assemble(out_dir)
    return 0


if __name__ == "__main__":
    sys.exit(main())
