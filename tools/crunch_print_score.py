#!/usr/bin/env python3
"""Scores JeffCrunch's printed pages against reference pages made by the common spreadsheet program, sheet by sheet (the yardstick of docs/jeffcrunch.md, the printing section).

    python3 tools/crunch_print_score.py JCRUNCHTOOL [--ref DIR] [--out DIR] [--jobs N] [--limit N] [--pages N]

DIR (default ~/crunch-corpus/printref) holds index.tsv, with a line for each sheet: key, path, sheet (counted from 1, in the workbook's order), the sheet's name, status, and pages;
and <key>/sheet-<NN>.pdf for each sheet whose status is ok. For each workbook the tool asks jcrunchtool for our pages of those sheets (print-sheets: a PDF for each, and where each page is
cut), and compares:
  * the page count of each sheet: the percent that match exactly;
  * for the sheets whose counts match, each page's picture, by the measure of the yardstick for slides: both pages resized to the page's inches at 40 dots to the inch, blurred 2 pixels,
    the mean gray difference (0 is the same page);
  * where the pages are cut: our first and last row and column of each page (from the pagination), and, from the text of the reference page (pdftotext -bbox), whether its first and last
    words and its words as a whole are those of our page (the words say which cells are on it, whatever the type is);
and lists the ten sheets that match worst, with what is different. Two things are made to match the reference's own run: each workbook is printed from a link named as the reference's copy
was (<key>.<ext>, so the file name fields show the same name), and the date and time fields show the moment the reference PDF was made (its CreationDate, given to the tool as JP_NOW). A
reference that is one blank page (an empty sheet) is compared by page count only, and counted on its own line. Only the first --pages (default 20) pages of a sheet are compared picture by picture. Prints summary lines; writes
OUT/scores.tsv (a line for each page) and OUT/sheets.tsv (a line for each sheet).
"""
import collections, csv, glob, os, re, shutil, statistics, subprocess, sys, tempfile
from concurrent.futures import ThreadPoolExecutor
from html import unescape

from PIL import Image, ImageChops, ImageFilter, ImageStat

DPI = 40


def sh(cmd, timeout=600, **kw):
    return subprocess.run(cmd, capture_output=True, text=True, timeout=timeout, **kw)


def page_info(pdf):
    """Pages and the first page's size in points, from pdfinfo."""
    try:
        out = sh(["pdfinfo", pdf], timeout=120).stdout
    except Exception:
        return 0, (0, 0)
    pages = re.search(r"^Pages:\s+(\d+)", out, re.M)
    size = re.search(r"^Page size:\s+([\d.]+) x ([\d.]+) pts", out, re.M)
    return (int(pages.group(1)) if pages else 0), ((float(size.group(1)), float(size.group(2))) if size else (0, 0))


def ref_now(pdf):
    """The moment the reference was made (its PDF's creation date, as ISO 8601 with its offset), so the date and time fields show the same: JP_NOW for the tool; None if it has none."""
    try:
        out = sh(["pdfinfo", "-isodates", pdf], timeout=60).stdout
    except Exception:
        return None
    m = re.search(r"^CreationDate:\s+(\S+)", out, re.M)
    if not m:
        return None
    v = m.group(1)
    return v + ":00" if re.search(r"[+-]\d\d$", v) else v   # (pdfinfo gives -04; the tool reads -04:00)


def is_blank(pdf):
    """Whether the reference is the one blank page an empty sheet comes out as: a page, a few kilobytes, and no words."""
    try:
        if os.path.getsize(pdf) > 12000 or page_info(pdf)[0] != 1:
            return False
    except OSError:
        return False
    pages = words_by_page(pdf, 1)
    return not (pages and pages[0])


def words_by_page(pdf, limit):
    """For each page (up to `limit`), the words in reading order."""
    try:
        out = sh(["pdftotext", "-bbox", "-l", str(limit), pdf, "-"], timeout=300).stdout
    except Exception:
        return []
    pages = []
    for page in out.split("<page ")[1:]:
        ws = [(float(m.group(2)), float(m.group(1)), unescape(m.group(3))) for m in re.finditer(r'<word xMin="([\d.]+)" yMin="([\d.]+)"[^>]*>(.*?)</word>', page)]
        ws.sort(key=lambda w: (round(w[0] / 3), w[1]))
        pages.append([w[2] for w in ws])
    return pages


def score(mine, ref, w_pt, h_pt):
    a, b = Image.open(mine).convert("L"), Image.open(ref).convert("L")
    size = (max(1, round(w_pt / 72 * DPI)), max(1, round(h_pt / 72 * DPI)))
    a, b = a.resize(size, Image.LANCZOS), b.resize(size, Image.LANCZOS)
    return ImageStat.Stat(ImageChops.difference(a.filter(ImageFilter.GaussianBlur(2)), b.filter(ImageFilter.GaussianBlur(2)))).mean[0]


def rasterize(pdf, prefix, limit):
    sh(["pdftoppm", "-r", "80", "-l", str(limit), "-png", pdf, prefix], timeout=600)
    return sorted(glob.glob(prefix + "-*.png"), key=lambda p: int(re.search(r"-(\d+)\.png$", p).group(1)))


def one_workbook(job):
    tool, key, path, sheets, ref_dir, out_dir, max_pages = job
    target = os.path.join(out_dir, key)
    os.makedirs(target, exist_ok=True)
    # the workbook as the reference printed it: a copy named <key>.<ext> (a link here), and the moment the reference was made (the sheets of one workbook are made within seconds of each
    # other: one run of the tool for each minute there is)
    linkdir = tempfile.mkdtemp(prefix="crunch-print-link-")
    link = os.path.join(linkdir, key + (os.path.splitext(path)[1] or ".xlsx"))
    try:
        os.symlink(os.path.abspath(os.path.expanduser(path)), link)
    except OSError:
        link = path
    groups = collections.OrderedDict()
    for n, name in sheets:
        moment = ref_now(os.path.join(ref_dir, key, "sheet-%02d.pdf" % n))
        groups.setdefault(re.sub(r"(T\d\d:\d\d):\d\d", r"\1", moment) if moment else "", (moment, []))[1].append(n)
    where = {}
    cuts = collections.defaultdict(list)
    for gi, (moment, ns) in enumerate(groups.values()):
        gdir = os.path.join(target, "run%d" % gi)
        os.makedirs(gdir, exist_ok=True)
        env = dict(os.environ, QT_QPA_PLATFORM="offscreen", JP_NO_TELEMETRY="1")
        env.pop("JP_NOW", None)
        if moment:
            env["JP_NOW"] = moment
        cmd = ["systemd-run", "--user", "--scope", "-q", "-p", "MemoryMax=4G", "timeout", "300", tool, "print-sheets", link, gdir] + [str(n) for n in ns]
        try:
            sh(cmd, timeout=360, env=env)
        except Exception:
            pass
        for n in ns:
            where[n] = gdir
        try:
            for line in open(os.path.join(gdir, "cuts.tsv"), encoding="utf-8"):
                f = line.rstrip("\n").split("\t")
                if len(f) >= 12 and int(f[0]) in ns:
                    cuts[int(f[0])].append(f)
        except OSError:
            pass
    shutil.rmtree(linkdir, ignore_errors=True)
    results = []
    for n, name in sheets:
        mine = os.path.join(where.get(n, target), "sheet-%02d.pdf" % n)
        ref = os.path.join(ref_dir, key, "sheet-%02d.pdf" % n)
        row = {"key": key, "sheet": n, "name": name, "ref_pages": 0, "our_pages": 0, "status": "", "pages": [], "cause": "", "blank": False}
        if not os.path.exists(ref):
            row["status"] = "no reference"
        elif not os.path.exists(mine):
            # nothing printed: a failure of ours, unless the reference is the blank page of an empty sheet (the tool says "no cells to print" for those, and they are counted on their own)
            row["ref_pages"] = page_info(ref)[0]
            row["blank"] = is_blank(ref)
            row["status"] = "empty" if row["blank"] else "count differs"
            row["cause"] = "" if row["blank"] else "not printed; reference %d pages" % row["ref_pages"]
        else:
            rp, rsize = page_info(ref)
            op, osize = page_info(mine)
            row["ref_pages"], row["our_pages"] = rp, op
            row["status"] = "match" if rp == op else "count differs"
            row["blank"] = is_blank(ref)
            cause = []
            if rp != op:
                cause.append("pages %d, reference %d" % (op, rp))
            if abs(rsize[0] - osize[0]) > 2 or abs(rsize[1] - osize[1]) > 2:
                cause.append("paper %.0fx%.0f, reference %.0fx%.0f" % (osize[0], osize[1], rsize[0], rsize[1]))
            if cuts.get(n):
                c0 = cuts[n][0]
                cause.append("scale %s" % c0[2])
            row["cause"] = "; ".join(cause)
            limit = 0 if row["blank"] else max_pages   # (a blank reference: the page count is all there is to compare)
            rw, ow = (words_by_page(ref, limit), words_by_page(mine, limit)) if limit else ([], [])
            for i in range(min(rp, op, limit)):
                p = {"page": i + 1, "score": None, "jaccard": None, "first": None, "last": None, "cut": ""}
                if i < len(cuts.get(n, [])):
                    c = cuts[n][i]
                    p["cut"] = "rows %s-%s cols %s-%s" % (c[4], c[5], c[6], c[7])
                if i < len(rw) and i < len(ow):
                    a, b = collections.Counter(rw[i]), collections.Counter(ow[i])
                    union = sum((a | b).values())
                    p["jaccard"] = (sum((a & b).values()) / union) if union else 1.0
                    p["first"] = (rw[i][:1] == ow[i][:1])
                    p["last"] = (rw[i][-1:] == ow[i][-1:])
                row["pages"].append(p)
            if rp == op and limit > 0:
                with tempfile.TemporaryDirectory() as tmp:
                    a = rasterize(mine, os.path.join(tmp, "m"), limit)
                    b = rasterize(ref, os.path.join(tmp, "r"), limit)
                    for i, (x, y) in enumerate(zip(a, b)):
                        if i < len(row["pages"]):
                            row["pages"][i]["score"] = score(x, y, rsize[0] or osize[0], rsize[1] or osize[1])
        results.append(row)
    return results


def summ(name, v):
    if not v:
        print("%-34s n=0" % name)
        return
    v = sorted(v)
    print("%-34s n=%5d mean %5.2f med %5.2f p90 %5.2f max %6.2f | <1 %3.0f%% <2 %3.0f%% <4 %3.0f%%" % (
        name, len(v), statistics.mean(v), statistics.median(v), v[int(len(v) * .9)], v[-1],
        100 * sum(x < 1 for x in v) / len(v), 100 * sum(x < 2 for x in v) / len(v), 100 * sum(x < 4 for x in v) / len(v)))


def main():
    args = sys.argv[1:]
    opt = {"--ref": os.path.expanduser("~/crunch-corpus/printref"), "--out": None, "--jobs": "8", "--limit": "0", "--pages": "20"}
    rest = []
    i = 0
    while i < len(args):
        if args[i] in opt:
            opt[args[i]] = args[i + 1]
            i += 2
        else:
            rest.append(args[i])
            i += 1
    tool = rest[0]
    ref_dir = opt["--ref"]
    out_dir = opt["--out"] or tempfile.mkdtemp(prefix="crunch-print-score-")
    os.makedirs(out_dir, exist_ok=True)
    rows = list(csv.reader(open(os.path.join(ref_dir, "index.tsv"), encoding="utf-8-sig"), delimiter="\t"))
    if rows and not (len(rows[0]) > 2 and rows[0][2].strip().isdigit()):
        rows = rows[1:]
    groups = collections.OrderedDict()
    skipped = collections.Counter()
    for r in rows:
        if len(r) < 5:
            continue
        key, path, n, name, status = r[0], r[1], int(r[2]), r[3], r[4]
        if status.strip() != "ok":
            skipped[status.strip()] += 1
            continue
        groups.setdefault((key, path), []).append((n, name))
    jobs = [(tool, k, p, s, ref_dir, out_dir, int(opt["--pages"])) for (k, p), s in groups.items()]
    if int(opt["--limit"]) > 0:
        jobs = jobs[:int(opt["--limit"])]
    with ThreadPoolExecutor(int(opt["--jobs"])) as ex:
        results = [r for part in ex.map(one_workbook, jobs) for r in part]
    with open(os.path.join(out_dir, "sheets.tsv"), "w", encoding="utf-8") as f:
        f.write("key\tsheet\tname\tour_pages\tref_pages\tstatus\tcause\n")
        for r in results:
            f.write("%s\t%d\t%s\t%d\t%d\t%s\t%s\n" % (r["key"], r["sheet"], r["name"], r["our_pages"], r["ref_pages"], r["status"], r["cause"]))
    with open(os.path.join(out_dir, "scores.tsv"), "w", encoding="utf-8") as f:
        f.write("key\tsheet\tpage\tscore\tjaccard\tfirst\tlast\tour cut\n")
        for r in results:
            for p in r["pages"]:
                f.write("%s\t%d\t%d\t%s\t%s\t%s\t%s\t%s\n" % (r["key"], r["sheet"], p["page"], "" if p["score"] is None else "%.2f" % p["score"], "" if p["jaccard"] is None else "%.3f" % p["jaccard"], p["first"], p["last"], p["cut"]))
    scored = [r for r in results if r["status"] in ("match", "count differs")]
    exact = [r for r in scored if r["status"] == "match"]
    blank_unprinted = [r for r in results if r["status"] == "empty"]
    print("sheets %d from %d workbooks (skipped by the reference: %s; no reference page %d; not printed by us %d)" % (
        len(results), len(groups), ", ".join("%s %d" % kv for kv in skipped.items()) or "none", sum(r["status"] == "no reference" for r in results), sum(r["cause"].startswith("not printed") for r in results)))
    if scored:
        print("page count: %d of %d sheets match exactly (%.1f%%)" % (len(exact), len(scored), 100 * len(exact) / len(scored)))
        blanks = [r for r in scored if r["blank"]]
        if blanks:
            print("  %d of them have a blank one-page reference (compared by count only)" % len(blanks))
    if blank_unprinted:
        print("empty sheets: %d have a blank one-page reference and we print nothing for them (the sheet has no cells to print; a message says so); not counted above; with them counted as misses the match is %.1f%%" % (len(blank_unprinted), 100 * len(exact) / max(1, len(scored) + len(blank_unprinted))))
    if scored:
        both = [p for r in exact for p in r["pages"]]
        summ("page picture difference (matched)", [p["score"] for p in both if p["score"] is not None])
        jac = [p["jaccard"] for r in scored for p in r["pages"] if p["jaccard"] is not None]
        if jac:
            print("same words on the page: mean %.3f; %.1f%% of pages at 0.95 or more (all compared pages)" % (statistics.mean(jac), 100 * sum(x >= 0.95 for x in jac) / len(jac)))
        cuts = [p for r in scored for p in r["pages"] if p["first"] is not None]
        if cuts:
            print("page cut: first word agrees on %.1f%% of pages, last word on %.1f%%" % (100 * sum(p["first"] for p in cuts) / len(cuts), 100 * sum(p["last"] for p in cuts) / len(cuts)))
    def badness(r):
        s = [p["score"] for p in r["pages"] if p["score"] is not None]
        return (abs(r["our_pages"] - r["ref_pages"]) * 100 + (statistics.mean(s) if s else 0)) if r["status"] in ("match", "count differs") else -1
    worst = sorted(scored, key=badness, reverse=True)[:10]
    print("the ten worst sheets:")
    for r in worst:
        s = [p["score"] for p in r["pages"] if p["score"] is not None]
        first_cut = r["pages"][0]["cut"] if r["pages"] else ""
        print("  %s sheet %d (%s): pages %d, reference %d, picture %.1f; %s; first page %s" % (r["key"], r["sheet"], r["name"][:24], r["our_pages"], r["ref_pages"], statistics.mean(s) if s else -1, r["cause"], first_cut))
    return 0


if __name__ == "__main__":
    sys.exit(main())
