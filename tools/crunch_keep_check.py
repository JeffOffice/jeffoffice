#!/usr/bin/env python3
"""Checks that a save keeps everything a cell edit did not touch (the keep-on-save audit: docs/jeffcrunch.md, beta wave 1).

    python3 tools/crunch_keep_check.py BUILD_DIR LIST [--jobs N] [--max N] [--out FILE]

For each workbook in LIST (one path on a line) it runs `jcrunchtool roundtrip-one` with JC_ROUNDTRIP_KEEP, which types one number in a cell and saves, and compares the saved package
with the old one:
  - every part that is not a worksheet must be the old bytes, or be one the save is meant to rewrite (the styles, the shared strings, the calculation chain, the workbook part);
  - every worksheet part, with its cells taken out (`<c>` elements), must be the old bytes but for the `dimension` and the `spans` of rows; what is left is the skeleton: the rows, columns,
    merged ranges, outlines, validations, conditional formats, hyperlinks, drawings, form controls, and extension lists. A difference is named by the element it falls in.
A workbook whose package cannot be read is counted as unreadable, one over 200 MB (a part over a quarter of that) as too large, a run that did not start or did not finish in its time (systemd-run or the tool
missing or failing, or the time out) as a runner failure, and one that ran and wrote no file as not saved; none stops the run.
Prints one summary: workbooks saved and compared, parts and skeletons compared and changed, and the commonest differences with an example path each.
"""
import collections, os, re, subprocess, sys, tempfile, zipfile
from concurrent.futures import ThreadPoolExecutor

REWRITTEN = re.compile(r"^(xl/(styles|sharedStrings|calcChain)\.xml|xl/workbook\.xml|docProps/(core|app)\.xml|\[Content_Types\]\.xml|xl/_rels/workbook\.xml\.rels)$")
SHEET = re.compile(r"^xl[/\\](worksheets|macrosheets|dialogsheets)[/\\][^/\\]+\.xml$", re.I)
CELL_OPEN = re.compile(rb"<((?:[A-Za-z0-9]+:)?)c(?=[\s/>])[^>]*>")
MAX_BYTES = 200 * 1024 * 1024
VOLATILE = re.compile(rb'(<(?:[A-Za-z0-9]+:)?dimension ref=")[^"]*|( spans=")[^"]*')
TAG = re.compile(rb"<([A-Za-z0-9:_]+)")


SELF_ROW = re.compile(rb"<row ([^>]*?)/>")
EMPTY_ROW = re.compile(rb'<row r="[0-9]+"></row>')
PLAIN_HEIGHT = re.compile(rb"<row [^>]*>")
BETWEEN_TAGS = re.compile(rb">\s+<")
ROOT = re.compile(rb"<worksheet\s[^>]*>")


def row_without_auto_height(m):
    tag = m.group(0)
    return tag if b"customHeight" in tag else re.sub(rb' ht="[^"]*"', b"", tag)


def strip_cells(data):
    """The sheet without its `<c>` elements. One pass over the bytes: each opening tag is found by a pattern that cannot run past its own `>`, and its end by a plain search for the closing
    tag (cells do not nest), so the cost is in the size of the sheet and no pattern is tried again from the same place."""
    out = []
    at = 0
    while True:
        m = CELL_OPEN.search(data, at)
        if not m:
            out.append(data[at:])
            break
        out.append(data[at:m.start()])
        if m.group(0).endswith(b"/>"):
            at = m.end()
            continue
        close = data.find(b"</" + m.group(1) + b"c>", m.end())
        if close < 0:   # (a cell that is never closed: the rest of the sheet is left as it is)
            out.append(data[m.start():])
            break
        at = close + len(m.group(1)) + 4
    return b"".join(out)


def skeleton(data):
    """The sheet with its cells out, and what a cell edit may rightly change out too: a row's spans, the sheet's dimension, the height of a row with no height of its own, a row written
    as <row/> or <row></row>, a row with nothing in it but its number (the edit made it), and <sheetData/> against an empty <sheetData></sheetData>."""
    text = strip_cells(data)
    text = BETWEEN_TAGS.sub(b"><", text)
    text = ROOT.sub(lambda m: re.sub(rb"\s+", b" ", m.group(0)), text)
    text = VOLATILE.sub(lambda m: (m.group(1) or m.group(2)), text)
    text = SELF_ROW.sub(rb"<row \1></row>", text)
    text = PLAIN_HEIGHT.sub(row_without_auto_height, text)
    text = EMPTY_ROW.sub(b"", text)
    return text.replace(b"<sheetData/>", b"<sheetData></sheetData>")


def first_difference(a, b):
    n = min(len(a), len(b))
    i = next((k for k in range(n) if a[k] != b[k]), n)
    start = a.rfind(b"<", 0, i + 1)
    m = TAG.match(a, start if start >= 0 else 0)
    return (m.group(1).decode() if m else "?"), i


def one(build, path, tmp, k):
    """The result for one workbook: a dict with "status" ok, nosave (ran, wrote no file), runner (did not run), unreadable (the package or its saved copy is not a zip), or toolarge."""
    try:
        if os.path.getsize(path) > MAX_BYTES:
            return {"status": "toolarge"}
    except OSError:
        return {"status": "unreadable"}
    out = os.path.join(tmp, "%d.xlsx" % k)
    env = dict(os.environ, JP_NO_TELEMETRY="1", QT_QPA_PLATFORM="offscreen", JC_ROUNDTRIP_KEEP=out)
    try:
        run = subprocess.run(["systemd-run", "--user", "--scope", "-p", "MemoryMax=6G", "-p", "MemorySwapMax=0", "-q", "timeout", "-k", "5", "300", build + "/jcrunchtool", "roundtrip-one", path],
                             env=env, capture_output=True, timeout=330)
    except (subprocess.TimeoutExpired, OSError):   # (a run that did not finish in its time did not run to the end: it is a failure of the runner)
        return {"status": "runner"}
    if not os.path.exists(out):
        # (systemd-run itself failing says so on its stderr and never started the tool)
        if run.returncode != 0 and (b"Failed to" in run.stderr or b"systemd-run" in run.stderr or b"Running scope" in run.stderr):
            return {"status": "runner"}
        return {"status": "nosave"}
    result = {"status": "ok", "parts": 0, "partsChanged": [], "sheets": 0, "skeletonChanged": []}
    try:
        with zipfile.ZipFile(path) as old, zipfile.ZipFile(out) as new:
            names = {i.filename for i in new.infolist()}
            for info in old.infolist():
                n = info.filename
                if n.endswith("/"):
                    continue
                if n not in names:
                    result["partsChanged"].append((n, "gone" if not n.endswith("calcChain.xml") else "calcChain dropped (by design: the formulas' order is worked out again)"))
                    continue
                if info.file_size > MAX_BYTES // 4:   # (a part this large is not read: it is not compared, and not counted)
                    continue
                a, b = old.read(n), new.read(n)
                if SHEET.match(n):
                    result["sheets"] += 1
                    sa, sb = skeleton(a), skeleton(b)
                    if sa != sb:
                        tag, at = first_difference(sa, sb)
                        result["skeletonChanged"].append((n, tag, sa[max(0, at - 60):at + 80], sb[max(0, at - 60):at + 80]))
                elif REWRITTEN.match(n):
                    result.setdefault("rewritten", collections.Counter())[n if not n.startswith("xl/_") else "rels"] += 1 if a != b else 0
                else:
                    result["parts"] += 1
                    if a != b:
                        result["partsChanged"].append((n, "bytes"))
            for n in names - {i.filename for i in old.infolist()}:
                result["partsChanged"].append((n, "new"))
    except Exception:   # (a bad zip, a bad entry, a part that will not inflate: this workbook is counted, the run goes on)
        return {"status": "unreadable"}
    finally:
        if os.path.exists(out):
            os.remove(out)
    return result


def safe_one(build, path, tmp, k):
    try:
        return one(build, path, tmp, k)
    except Exception:
        return {"status": "unreadable"}


def main():
    args = sys.argv[1:]
    build, lst = os.path.abspath(args.pop(0)), args.pop(0)
    jobs, mx, show = 6, 10**9, None
    while args:
        a = args.pop(0)
        if a == "--jobs": jobs = int(args.pop(0))
        elif a == "--max": mx = int(args.pop(0))
        elif a == "--out": show = args.pop(0)
    paths = [l.strip() for l in open(lst) if l.strip() and not l.startswith("#")][:mx]
    tmp = tempfile.mkdtemp()
    with ThreadPoolExecutor(jobs) as ex:
        results = list(ex.map(lambda ik: (ik[1], safe_one(build, ik[1], tmp, ik[0])), enumerate(paths)))
    ok = [(p, r) for p, r in results if r["status"] == "ok"]
    failed = [p for p, r in results if r["status"] == "nosave"]
    by_status = collections.Counter(r["status"] for _, r in results)
    parts = sum(r["parts"] for _, r in ok)
    sheets = sum(r["sheets"] for _, r in ok)
    pc = collections.Counter()
    sc = collections.Counter()
    ex_p, ex_s = {}, {}
    badp = bads = 0
    for p, r in ok:
        if r["partsChanged"]: badp += 1
        if r["skeletonChanged"]: bads += 1
        for n, why in r["partsChanged"]:
            key = (re.sub(r"[0-9]+", "N", n), why)
            pc[key] += 1
            ex_p.setdefault(key, p)
        for n, tag, x, y in r["skeletonChanged"]:
            key = tag
            sc[key] += 1
            ex_s.setdefault(key, (p, n, x, y))
    rw = collections.Counter()
    for _, r in ok:
        rw.update(r.get("rewritten", {}))
    print("rewritten parts the save may change (workbooks where the bytes differ):", dict(rw))
    print("workbooks saved and compared: %d of %d (%d did not save, %d unreadable, %d too large, %d runner failures)"
          % (len(ok), len(paths), len(failed), by_status["unreadable"], by_status["toolarge"], by_status["runner"]))
    print("parts that are not sheets and not meant to change: %d compared, in %d workbooks some differ" % (parts, badp))
    print("worksheet parts compared by skeleton (cells taken out): %d, in %d workbooks some differ" % (sheets, bads))
    for key, n in pc.most_common(15):
        print("  part %s %s: %d (e.g. %s)" % (key[0], key[1], n, ex_p[key]))
    for key, n in sc.most_common(15):
        p, n2, x, y = ex_s[key]
        print("  skeleton in <%s>: %d (e.g. %s %s)\n      old %r\n      new %r" % (key, n, p, n2, x, y))
    if show:
        with open(show, "w") as f:
            for p in failed: f.write("nosave\t%s\n" % p)
            for p, r in results:
                if r["status"] in ("unreadable", "toolarge", "runner"): f.write("%s\t%s\n" % (r["status"], p))
            for p, r in ok:
                for n, why in r["partsChanged"]: f.write("part\t%s\t%s\t%s\n" % (p, n, why))
                for n, tag, x, y in r["skeletonChanged"]: f.write("skeleton\t%s\t%s\t%s\n" % (p, n, tag))


if __name__ == "__main__":
    main()
