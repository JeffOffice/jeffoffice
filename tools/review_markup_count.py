#!/usr/bin/env python3
"""Counts, over .docx files, how many documents contain each kind of tracked-change markup, and how many of
those the reader keeps when the document is read and saved again.

    review_markup_count.py WRITETOOL DIR [DIR ...] [--jobs N] [--list]

Read-only: every file is only read; the saved copy of a file that has markup is made in a temporary folder
that is removed. For each markup element (by local name) and each kind of part (body: word/document.xml;
header or footer: word/header*.xml and footer*.xml; notes: word/footnotes.xml and endnotes.xml) the table has
the documents that contain it, and the documents whose saved copy still has the same number of them (kept).
Documents are counted once each, by content hash, however many folders hold them.
"""
import hashlib, os, re, subprocess, sys, tempfile, zipfile
from collections import Counter
from concurrent.futures import ThreadPoolExecutor

NAMES = ["ins", "del", "moveFrom", "moveTo", "moveFromRangeStart", "moveToRangeStart", "rPrChange", "pPrChange", "sectPrChange",
         "tblPrChange", "tblGridChange", "trPrChange", "tcPrChange", "tblPrExChange", "numberingChange", "cellIns", "cellDel", "cellMerge"]
TAG = re.compile(rb"<(?:[A-Za-z0-9]+:)?(" + "|".join(NAMES).encode() + rb")[\s/>]")
PARTS = [("body", re.compile(r"^word/document\.xml$")), ("header/footer", re.compile(r"^word/(header|footer)\d*\.xml$")),
         ("notes", re.compile(r"^word/(footnotes|endnotes)\.xml$"))]


def count(path):
    c = Counter()
    try:
        with zipfile.ZipFile(path) as z:
            for n in z.namelist():
                for kind, rx in PARTS:
                    if rx.match(n):
                        if z.getinfo(n).file_size > 200_000_000:
                            continue
                        for m in TAG.finditer(z.read(n)):
                            c[(kind, m.group(1).decode())] += 1
    except (zipfile.BadZipFile, OSError, KeyError, RuntimeError):
        return None
    return c


def saved(tool, path, tmp):
    out = os.path.join(tmp, "out.docx")
    try:
        subprocess.run(["timeout", "-k", "5", "120", tool, "docsave", path, out], capture_output=True, timeout=130,
                       env=dict(os.environ, QT_QPA_PLATFORM="offscreen", JP_NO_TELEMETRY="1"))
    except subprocess.TimeoutExpired:
        return None
    return count(out) if os.path.exists(out) else None


def main():
    args = sys.argv[1:]
    jobs = 8
    if "--jobs" in args:
        i = args.index("--jobs"); jobs = int(args[i + 1]); del args[i:i + 2]
    show = "--list" in args
    args = [a for a in args if a != "--list"]
    if len(args) < 2:
        print(__doc__); return 2
    tool, dirs = args[0], args[1:]
    files, seen = [], set()
    for d in dirs:
        for root, _, names in os.walk(d):
            for n in names:
                if n.lower().endswith((".docx", ".docm")):
                    p = os.path.join(root, n)
                    try:
                        h = hashlib.sha1(open(p, "rb").read()).hexdigest()
                    except OSError:
                        continue
                    if h not in seen:
                        seen.add(h); files.append(p)
    with ThreadPoolExecutor(jobs) as ex:
        before = list(ex.map(count, files))
    todo = [(f, b) for f, b in zip(files, before) if b]
    def one(item):
        with tempfile.TemporaryDirectory() as tmp:
            return saved(tool, item[0], tmp)
    with ThreadPoolExecutor(jobs) as ex:
        after = list(ex.map(one, todo))
    have, kept = Counter(), Counter()
    for (f, b), a in zip(todo, after):
        for k, n in b.items():
            have[k] += 1
            if a is not None and a.get(k, 0) >= n:
                kept[k] += 1
            elif show:
                print("dropped", k, f, n, None if a is None else a.get(k, 0))
    ok = sum(1 for b in before if b is not None)
    print("documents %d (read %d), with any markup %d" % (len(files), ok, len(todo)))
    print("%-14s %-20s %6s %6s" % ("part", "element", "docs", "kept"))
    for k in sorted(have, key=lambda k: (k[0], -have[k])):
        print("%-14s %-20s %6d %6d" % (k[0], k[1], have[k], kept[k]))
    return 0


if __name__ == "__main__":
    sys.exit(main())
