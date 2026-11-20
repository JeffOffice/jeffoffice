#!/usr/bin/env python3
"""Counts, over .docx files, how many documents contain each kind of tracked-change markup, and how many of
those the reader keeps when the document is read and saved again.

    review_markup_count.py WRITETOOL DIR [DIR ...] [--jobs N] [--list]

Read-only: every file is only read; the saved copy of a file that has markup is made in a temporary folder
that is removed. For each markup element (by local name) and each kind of part (body: word/document.xml;
header or footer: word/header*.xml and footer*.xml; notes: word/footnotes.xml and endnotes.xml) the table has
the documents that contain it (the word-processing namespace only: a drawing's path command moveTo is not a move), and the documents whose saved copy still has the same number of them (kept). Then, for each kind of part, the documents whose changes `review-list` lists.
Documents are counted once each, by content hash, however many folders hold them.
"""
import hashlib, io, os, re, subprocess, sys, tempfile, zipfile
import xml.etree.ElementTree as ET
from collections import Counter
from concurrent.futures import ThreadPoolExecutor

NAMES = ["ins", "del", "moveFrom", "moveTo", "moveFromRangeStart", "moveToRangeStart", "rPrChange", "pPrChange", "sectPrChange",
         "tblPrChange", "tblGridChange", "trPrChange", "tcPrChange", "tblPrExChange", "numberingChange", "cellIns", "cellDel", "cellMerge"]
# A cheap first look (any prefix); what is counted is only an element of the word-processing namespace, found with the parser (a drawing's path command is also called moveTo).
TAG = re.compile(rb"<(?:[A-Za-z0-9]+:)?(" + "|".join(NAMES).encode() + rb")[\s/>]")
WORD_NS = ("http://schemas.openxmlformats.org/wordprocessingml/2006/main", "http://purl.oclc.org/ooxml/wordprocessingml/main")
WANTED = set(NAMES)
PARTS = [("body", re.compile(r"^word/document\.xml$")), ("header/footer", re.compile(r"^word/(header|footer)\d*\.xml$")),
         ("notes", re.compile(r"^word/(footnotes|endnotes)\.xml$"))]


def word_elements(data):
    """The elements of one part that are in the word-processing namespace and are one of NAMES, as a Counter by local name."""
    found = Counter()
    try:
        for _, el in ET.iterparse(io.BytesIO(data), events=("start",)):
            tag = el.tag
            if tag.startswith("{"):
                ns, _, local = tag[1:].partition("}")
                if ns in WORD_NS and local in WANTED:
                    found[local] += 1
    except ET.ParseError:
        # A part that is not well formed: what the parser had read before the error is not counted, and the part is counted by the
        # prefixes its own declarations bind to the namespace (an unparsable part is rare and the reader is as strict as the parser).
        prefixes = {m.decode() for ns in WORD_NS for m in re.findall(rb'xmlns:([A-Za-z0-9_.-]+)="' + re.escape(ns.encode()) + rb'"', data)}
        found = Counter()
        for m in TAG.finditer(data):
            pre = re.match(rb"<([A-Za-z0-9]+):", m.group(0))
            if pre and pre.group(1).decode() in prefixes:
                found[m.group(1).decode()] += 1
    return found


def count(path):
    c = Counter()
    try:
        with zipfile.ZipFile(path) as z:
            for n in z.namelist():
                for kind, rx in PARTS:
                    if rx.match(n):
                        if z.getinfo(n).file_size > 200_000_000:
                            continue
                        data = z.read(n)
                        if not TAG.search(data):
                            continue
                        for local, k in word_elements(data).items():
                            c[(kind, local)] += k
    except (zipfile.BadZipFile, OSError, KeyError, RuntimeError):
        return None
    return c


def listed(tool, path):
    """The parts (body, header/footer, notes) in which the tool lists tracked changes of the document, with how many it lists in each."""
    try:
        p = subprocess.run(["timeout", "-k", "5", "120", tool, "review-list", path], capture_output=True, timeout=130,
                           env=dict(os.environ, QT_QPA_PLATFORM="offscreen", JP_NO_TELEMETRY="1"))
    except subprocess.TimeoutExpired:
        return None
    n = Counter()
    for line in p.stdout.decode("utf8", "replace").split("\n"):
        f = line.split("\t")
        if len(f) < 7:
            continue
        where = f[-1]
        n["header/footer" if where.startswith(("header", "footer")) else "notes" if where.startswith("footnote") else "body"] += 1
    return n


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
    with ThreadPoolExecutor(jobs) as ex:
        lists = list(ex.map(lambda item: listed(tool, item[0]), todo))
    have, kept = Counter(), Counter()
    for (f, b), a in zip(todo, after):
        for k, n in b.items():
            have[k] += 1
            if a is not None and a.get(k, 0) >= n:
                kept[k] += 1
            elif show:
                print("dropped", k, f, n, None if a is None else a.get(k, 0))
    shown, shownCount = Counter(), Counter()
    for (f, b), l in zip(todo, lists):
        for part in {k[0] for k in b}:
            shown[part] += 1 if l and l.get(part, 0) > 0 else 0
            shownCount[part] += l.get(part, 0) if l else 0
    ok = sum(1 for b in before if b is not None)
    print("documents %d (read %d), with any markup %d" % (len(files), ok, len(todo)))
    print("%-14s %-20s %6s %6s" % ("part", "element", "docs", "kept"))
    for k in sorted(have, key=lambda k: (k[0], -have[k])):
        print("%-14s %-20s %6d %6d" % (k[0], k[1], have[k], kept[k]))
    print("documents whose changes in a part are listed (Next Change, the Reviewing pane), and how many changes are listed:")
    for part in ("body", "header/footer", "notes"):
        have_part = sum(1 for b in (b for _, b in todo) if any(k[0] == part for k in b))
        print("%-14s with markup %4d   listed in %4d   changes listed %6d" % (part, have_part, shown[part], shownCount[part]))
    return 0


if __name__ == "__main__":
    sys.exit(main())
