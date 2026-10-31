#!/usr/bin/env python3
"""Checks that a save keeps everything a cell edit did not touch (the keep-on-save audit: docs/jeffcrunch.md, beta wave 1).

    python3 tools/crunch_keep_check.py BUILD_DIR LIST [--jobs N] [--max N] [--out DIR]

For each workbook in LIST (one path on a line) it runs `jcrunchtool roundtrip-one` with JC_ROUNDTRIP_KEEP, which types one number in a cell and saves, and compares the saved package
with the old one:
  - every part that is not a worksheet must be the old bytes, or be one the save is meant to rewrite (the styles, the shared strings, the calculation chain, the workbook part);
  - every worksheet part, with its cells taken out (`<c>` elements), must be the old bytes but for the `dimension` and the `spans` of rows; what is left is the skeleton: the rows, columns,
    merged ranges, outlines, validations, conditional formats, hyperlinks, drawings, form controls, and extension lists. A difference is named by the element it falls in.
Prints one summary: workbooks saved and compared, parts and skeletons compared and changed, and the commonest differences with an example path each.
"""
import collections, os, re, subprocess, sys, tempfile, zipfile
from concurrent.futures import ThreadPoolExecutor

REWRITTEN = re.compile(r"^(xl/(styles|sharedStrings|calcChain)\.xml|xl/workbook\.xml|docProps/(core|app)\.xml|\[Content_Types\]\.xml|xl/_rels/workbook\.xml\.rels)$")
SHEET = re.compile(r"^xl[/\\](worksheets|macrosheets|dialogsheets)[/\\][^/\\]+\.xml$", re.I)
CELL = re.compile(rb"<((?:[A-Za-z0-9]+:)?)c(?=[\s/>])[^>]*?/>|<((?:[A-Za-z0-9]+:)?)c(?=[\s/>])[^>]*>.*?</\2c>", re.S)
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


def skeleton(data):
    """The sheet with its cells out, and what a cell edit may rightly change out too: a row's spans, the sheet's dimension, the height of a row with no height of its own, a row written
    as <row/> or <row></row>, a row with nothing in it but its number (the edit made it), and <sheetData/> against an empty <sheetData></sheetData>."""
    text = CELL.sub(b"", data)
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
    out = os.path.join(tmp, "%d.xlsx" % k)
    env = dict(os.environ, JP_NO_TELEMETRY="1", QT_QPA_PLATFORM="offscreen", JC_ROUNDTRIP_KEEP=out)
    try:
        subprocess.run(["systemd-run", "--user", "--scope", "-p", "MemoryMax=6G", "-p", "MemorySwapMax=0", "-q", "timeout", "-k", "5", "300", build + "/jcrunchtool", "roundtrip-one", path],
                       env=env, capture_output=True, timeout=330)
    except Exception:
        return None
    if not os.path.exists(out):
        return None
    result = {"parts": 0, "partsChanged": [], "sheets": 0, "skeletonChanged": []}
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
    finally:
        os.remove(out)
    return result


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
        results = list(ex.map(lambda ik: (ik[1], one(build, ik[1], tmp, ik[0])), enumerate(paths)))
    ok = [(p, r) for p, r in results if r]
    failed = [p for p, r in results if not r]
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
    print("workbooks saved and compared: %d of %d (%d did not save)" % (len(ok), len(paths), len(failed)))
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
            for p, r in ok:
                for n, why in r["partsChanged"]: f.write("part\t%s\t%s\t%s\n" % (p, n, why))
                for n, tag, x, y in r["skeletonChanged"]: f.write("skeleton\t%s\t%s\t%s\n" % (p, n, tag))


if __name__ == "__main__":
    main()
