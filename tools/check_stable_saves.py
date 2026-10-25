#!/usr/bin/env python3
"""Saves each sample file several times, each in a child process of its own, and compares the saved parts.

Qt picks a new hash seed for each process (and refuses a forced one other than 0), so a save that depends on the order of a hash differs between runs; with three runs
an element with three or more attributes in a different order each time is seen with probability 5/6, and a file has hundreds.

    python3 tools/check_stable_saves.py BUILD_DIR [deck|crunch|write|pub ...] [--max N] [--runs N] [--jobs N] [--show]

The saves must match byte for byte, except for the save time in the core properties and the ids a format wants fresh (GUIDs, such as a date field's id).
Samples (dev lists only): deck ~/deck-corpus/fresh/dev.txt, crunch ~/crunch-corpus/dev-xlsx.txt, write ~/doc-corpus/word/dev.txt, pub tests/data/pub.
Prints one line per app: files compared, files that differ, files that could not be saved.
"""
import os, re, subprocess, sys, tempfile, zipfile, hashlib
from concurrent.futures import ThreadPoolExecutor

RUNS = 3
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
HOME = os.path.expanduser("~")
GUID = re.compile(rb"[{]?[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{12}[}]?")
TIME = re.compile(rb"(<(?:dcterms:(?:modified|created)|cp:lastModifiedBy|ap:TotalTime)[^>]*>)[^<]*")
STAMP = re.compile(rb"[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}(?:\.[0-9]+)?Z?")


def norm(data):
    return STAMP.sub(b"T", GUID.sub(b"G", TIME.sub(rb"\1", data)))


def parts(path):
    """name -> normalized bytes; a file that is not a zip is one part."""
    try:
        with zipfile.ZipFile(path) as z:
            return [(i.filename, norm(z.read(i))) for i in z.infolist()]
    except zipfile.BadZipFile:
        with open(path, "rb") as f:
            return [("", norm(f.read()))]


def listed(path, base=None):
    out = []
    for line in open(path):
        line = line.strip()
        if line and not line.startswith("#"):
            out.append(os.path.join(base, line) if base else line)
    return out


def deck_samples():
    d = os.path.join(HOME, "deck-corpus/fresh")
    ins = {os.path.splitext(f)[0]: os.path.join(d, "in", f) for f in os.listdir(os.path.join(d, "in"))}
    return [ins[i] for i in listed(os.path.join(d, "dev.txt")) if i in ins]


def write_samples():
    d = os.path.join(HOME, "doc-corpus/word")
    ins = {os.path.splitext(f)[0]: os.path.join(d, "in", f) for f in os.listdir(os.path.join(d, "in"))}
    return [ins[i] for i in listed(os.path.join(d, "dev.txt")) if i in ins]


def crunchxls_samples():
    return [p for p in listed(os.path.join(HOME, "crunch-corpus/dev-xls.txt")) if os.path.exists(p)]


def write2_samples():
    d = os.path.join(HOME, "doc-corpus/word2")
    ins = {os.path.splitext(f)[0]: os.path.join(d, "in", f) for f in os.listdir(os.path.join(d, "in"))}
    return [ins[i] for i in listed(os.path.join(d, "dev2.txt")) if i in ins]


def crunch_samples():
    return [p for p in listed(os.path.join(HOME, "crunch-corpus/dev-xlsx.txt")) if os.path.exists(p)]


def pub_samples():
    d = os.path.join(ROOT, "tests/data/pub")
    return sorted(os.path.join(d, f) for f in os.listdir(d) if f.endswith(".pub"))


def run(cmd, env_extra=None, cwd=None):
    env = dict(os.environ, JP_NO_TELEMETRY="1", QT_QPA_PLATFORM="offscreen")
    env.update(env_extra or {})
    # (a tool may exit with a code for what it found; whether it saved is whether the file is there)
    subprocess.run(["systemd-run", "--user", "--scope", "-p", "MemoryMax=16G", "-q"] + cmd, env=env, cwd=cwd, capture_output=True, timeout=600)
    return True


APPS = {
    "deck": (deck_samples, lambda b, i, o: [b + "/jdecktool", "save", i, o], ".pptx", None),
    "deckodp": (deck_samples, lambda b, i, o: [b + "/jdecktool", "save", i, o], ".odp", None),
    "write": (write_samples, lambda b, i, o: [b + "/writetool", "docsave", i, o], ".docx", None),
    "write2": (write2_samples, lambda b, i, o: [b + "/writetool", "docsave", i, o], ".docx", None),
    "crunch": (crunch_samples, lambda b, i, o: [b + "/jcrunchtool", "roundtrip-one", i], ".xlsx", "JC_ROUNDTRIP_KEEP"),
    "crunchxls": (crunchxls_samples, lambda b, i, o: [b + "/jcrunchtool", "xls-save-check", i], ".xlsx", "JC_XLS_SAVE_KEEP"),
    "crunchnotes": (crunch_samples, lambda b, i, o: [b + "/jcrunchtool", "side-roundtrip", i, o], ".xlsx", None),
    "crunchfilter": (crunch_samples, lambda b, i, o: [b + "/jcrunchtool", "filter-roundtrip", i, o], ".xlsx", None),
    "pub": (pub_samples, lambda b, i, o: [b + "/jpubtool", "convert", i, o], ".jpub", None),
    "pubfile": (pub_samples, lambda b, i, o: [b + "/jpubtool", "topub", i, o], ".pub", None),
}


def one(app, build, sample, tmp, k):
    _, cmd, ext, keep = APPS[app]
    if app in ("pub", "pubfile"):
        # (an import gives each item an id of its own, which is meant: the publication is imported once and the saves are of that)
        first = os.path.join(tmp, "%d-base.jpub" % k)
        run([build + "/jpubtool", "convert", sample, first])
        if not os.path.exists(first):
            return "fail", None
        sample = first
    outs = []
    for n in range(RUNS):
        # (the same name in a folder of its own for each run: a saved file may carry its own name)
        os.makedirs(os.path.join(tmp, "%d-%d" % (k, n)), exist_ok=True)
        here = os.path.join(tmp, "%d-%d" % (k, n))
        o = os.path.join(here, "saved" + ext)
        env = {keep: o} if keep else None
        # (run in its own folder with the name given plainly: a saved file may carry the path it was given)
        run(cmd(build, sample, "saved" + ext) if not keep else cmd(build, sample, o), env, here)
        if not os.path.exists(o):
            return "fail", None
        outs.append(o)
    a = parts(outs[0])
    bad = []
    for o in outs[1:]:
        b = parts(o)
        if [n for n, _ in a] != [n for n, _ in b]:
            bad.append("part list")
        else:
            bad += [n for (n, x), (_, y) in zip(a, b) if x != y and n not in bad]
    for o in outs:
        os.remove(o)
    return ("differ", bad) if bad else ("same", None)


def main():
    args = sys.argv[1:]
    build = os.path.abspath(args.pop(0))
    global RUNS
    mx, jobs, show = 50, 6, False
    names = []
    while args:
        a = args.pop(0)
        if a == "--runs": RUNS = int(args.pop(0))
        elif a == "--max": mx = int(args.pop(0))
        elif a == "--jobs": jobs = int(args.pop(0))
        elif a == "--show": show = True
        else: names.append(a)
    names = names or ["deck", "deckodp", "write", "write2", "crunch", "crunchxls", "pub", "pubfile"]
    total = 0
    with tempfile.TemporaryDirectory() as tmp:
        for app in names:
            samples = APPS[app][0]()[:mx]
            with ThreadPoolExecutor(jobs) as ex:
                res = list(ex.map(lambda ks: one(app, build, ks[1], tmp, ks[0]), enumerate(samples)))
            same = sum(r[0] == "same" for r in res)
            diff = sum(r[0] == "differ" for r in res)
            fail = sum(r[0] == "fail" for r in res)
            total += diff
            print("%s: %d files, %d same, %d differ, %d not saved" % (app, len(samples), same, diff, fail))
            if show:
                for s, r in zip(samples, res):
                    if r[0] != "same": print("   ", r[0], os.path.basename(s), (r[1] or [])[:6])
    return 1 if total else 0


if __name__ == "__main__":
    sys.exit(main())
