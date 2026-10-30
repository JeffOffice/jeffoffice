#!/usr/bin/env python3
"""Runs the edit script of the yardstick (jcrunchtool roundtrip-one) with two builds and compares the workbooks each saves after it, part by part (docs/jeffcrunch-rowids.md, round 7).

    python3 tools/crunch_save_compare.py LIST OUTDIR JOBS A_TOOL A_ENV B_TOOL B_ENV

LIST has a path to a workbook on each line. A is the reference (an older build, or the formulas held in places), B the new one; the ENV arguments are `NAME=value,NAME=value` (or empty), set for
that build, such as JC_ID_FRAME=0. Every part must be the same bytes, but for the cells of the sheets that have a formula, which may be written another way (whole from the window, or as the file
had them); for those the text of the formula and its value are compared. A line for each workbook that differs goes to OUTDIR/diffs.txt, and the counts, with an example of each, to OUTDIR/summary.txt.
"""
import collections, os, re, subprocess, sys, tempfile, zipfile, html
from concurrent.futures import ThreadPoolExecutor

LIMIT = 300
CELL = re.compile(rb'<c r="([A-Z]+[0-9]+)"[^>]*?(?:/>|>.*?</c>)', re.S)
ROWOPEN = re.compile(rb'<row [^>]*>')


def run(tool, env_extra, path, keep):
    env = dict(os.environ, JP_NO_TELEMETRY="1", QT_QPA_PLATFORM="offscreen", JC_ROUNDTRIP_KEEP_SCRIPT=keep)
    env.update(env_extra)
    cmd = ["systemd-run", "--user", "--scope", "-p", "MemoryMax=6G", "-p", "MemorySwapMax=0", "-q", "timeout", "-k", "5", str(LIMIT), tool, "roundtrip-one", path]
    try:
        subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, env=env, timeout=LIMIT + 30)
    except Exception:
        return False
    return os.path.exists(keep) and os.path.getsize(keep) > 0


def formula_and_value(cell):
    f = re.search(rb'<f[^>]*?(?:/>|>(.*?)</f>)', cell, re.S)
    v = re.search(rb'<v>(.*?)</v>|<v/>', cell, re.S)
    t = re.search(rb' t="([a-zA-Z]+)"', cell.split(b'>', 1)[0])
    return (f is not None, html.unescape(f.group(1).decode('utf-8', 'replace')) if f and f.group(1) is not None else None, (v.group(1) or b'') if v else None, t.group(1) if t else None)


def same_value(a, b):
    """The same value as a number (the digits written may differ: 6.425 and 6.4249999999999998) or the same bytes."""
    if a == b:
        return True
    try:
        return float(a) == float(b)
    except (TypeError, ValueError):
        return False


def close_value(a, b):
    """Numbers that agree to 1e-9 (a value another calculation gave: the sum added in another order)."""
    try:
        x, y = float(a), float(b)
    except (TypeError, ValueError):
        return False
    return abs(x - y) <= 1e-9 * max(abs(x), abs(y), 1e-300)


def compare(a_path, b_path):
    diffs = []
    with zipfile.ZipFile(a_path) as za, zipfile.ZipFile(b_path) as zb:
        na, nb = set(za.namelist()), set(zb.namelist())
        if na != nb:
            diffs.append(("parts differ in names", sorted(na ^ nb)[:3]))
        for name in sorted(na & nb):
            x, y = za.read(name), zb.read(name)
            if x == y:
                continue
            if not re.match(r"xl/worksheets/sheet\d+\.xml$", name):
                diffs.append(("part differs", name))
                continue
            # a sheet: everything but the cells with formulas must be the same
            xa, xb = CELL.sub(b"", x), CELL.sub(b"", y)
            if xa != xb:
                diffs.append(("sheet differs outside its cells", name))
                continue
            ca = {m.group(1): m.group(0) for m in CELL.finditer(x)}
            cb = {m.group(1): m.group(0) for m in CELL.finditer(y)}
            if set(ca) != set(cb):
                diffs.append(("cells differ in set", name))
                continue
            for ref in ca:
                if ca[ref] == cb[ref]:
                    continue
                where = name + " " + ref.decode()
                fa, fb = formula_and_value(ca[ref]), formula_and_value(cb[ref])
                if not fa[0] and not fb[0]:
                    diffs.append(("a cell without a formula differs", where))
                elif fa[0] != fb[0]:
                    diffs.append(("formula in one and not the other", where))
                elif fa[1] is None or fb[1] is None:
                    # (a member of a shared formula has no text of its own: the sheet is not compared by text there)
                    diffs.append(("formula shared in one, with its text in the other (not compared)", name))
                elif fa[1] != fb[1]:
                    diffs.append(("formula text differs", where + " " + repr(fa[1])[:60] + " / " + repr(fb[1])[:60]))
                elif not same_value(fa[2], fb[2]) or fa[3] != fb[3]:
                    diffs.append(("formula value differs" + ("" if not close_value(fa[2], fb[2]) or fa[3] != fb[3] else " in the last digits"), where + " " + repr(fa[2])[:30] + " / " + repr(fb[2])[:30]))
                else:
                    diffs.append(("formula cell written another way (same text and value)", name))
    return diffs


def one(args):
    path, a, b, tmp, idx = args
    ka, kb = os.path.join(tmp, "a%d.xlsx" % idx), os.path.join(tmp, "b%d.xlsx" % idx)
    oka, okb = run(a[0], a[1], path, ka), run(b[0], b[1], path, kb)
    try:
        if oka != okb:
            return path, [("one build kept a workbook and the other not", "%s %s" % (oka, okb))]
        if not oka:
            return path, []
        return path, compare(ka, kb)
    except Exception as e:
        return path, [("compare failed", repr(e))]
    finally:
        for k in (ka, kb):
            if os.path.exists(k):
                os.unlink(k)


def main():
    lst, out, jobs = sys.argv[1], sys.argv[2], int(sys.argv[3])
    a = (sys.argv[4], dict(kv.split("=") for kv in sys.argv[5].split(",") if kv))
    b = (sys.argv[6], dict(kv.split("=") for kv in sys.argv[7].split(",") if kv))
    os.makedirs(out, exist_ok=True)
    paths = [l.strip() for l in open(lst) if l.strip()]
    tmp = tempfile.mkdtemp(dir=out)
    counts = collections.Counter()
    examples = collections.defaultdict(list)
    files_with = 0
    done = 0
    with ThreadPoolExecutor(jobs) as pool, open(os.path.join(out, "diffs.txt"), "w") as log:
        for path, diffs in pool.map(one, [(p, a, b, tmp, i) for i, p in enumerate(paths)]):
            done += 1
            if diffs:
                files_with += 1
                kinds = collections.Counter(k for k, _ in diffs)
                for k, n in kinds.items():
                    counts[k] += n
                    if len(examples[k]) < 5:
                        examples[k].append((os.path.basename(path), [d for kk, d in diffs if kk == k][0]))
                log.write(path + "\t" + "; ".join("%s x%d" % kv for kv in kinds.items()) + "\n")
                log.flush()
            if done % 250 == 0:
                print("%d of %d" % (done, len(paths)), flush=True)
    with open(os.path.join(out, "summary.txt"), "w") as s:
        s.write("workbooks %d, with a difference %d\n" % (len(paths), files_with))
        for k, n in counts.most_common():
            s.write("%6d  %s   e.g. %s\n" % (n, k, examples[k][:3]))
    print(open(os.path.join(out, "summary.txt")).read())


main()
