#!/usr/bin/env python3
"""Sweeps a folder of .odt files through JeffWrite's reader and compares the text with the .docx or .doc each was made from.

usage: odt_sweep.py <writetool> <odt-folder> <source-folder> [workers=6] [round-trip-folder]

For each X.odt: `writetool text` on it and on the source named X.* (docx or doc) in the source folder, under a timeout and a memory limit; the words of the two
(letters and digits, lower case) are matched as multisets, and a file passes when 99% of the larger list is matched. With a round-trip folder, each file is also read,
saved as .odt there (`writetool odtsave`), and the saved file read again and compared with the first reading. Prints counts, never text.
"""
import collections, concurrent.futures, os, re, subprocess, sys

tool, odt_dir, src_dir = sys.argv[1:4]
workers = int(sys.argv[4]) if len(sys.argv) > 4 else 6
rt_dir = sys.argv[5] if len(sys.argv) > 5 else None
env = dict(os.environ, JP_NO_TELEMETRY="1", QT_QPA_PLATFORM="offscreen")


def run(args, timeout=90):
    try:
        p = subprocess.run(["bash", "-c", "ulimit -v 8000000; exec \"$@\"", "x", tool] + args, capture_output=True, timeout=timeout, env=env)
        return p.returncode, p.stdout.decode("utf-8", "replace"), p.stderr.decode("utf-8", "replace")
    except subprocess.TimeoutExpired:
        return -9, "", "timeout"


def words(text):
    return collections.Counter(re.findall(r"\w+", "\n".join(l for l in text.split("\n") if not l.startswith("#note")).lower()))


def match(a, b):
    both = sum((a & b).values())
    size = max(sum(a.values()), sum(b.values()))
    return 1.0 if size == 0 else both / size


def one(name):
    base = name[:-4]
    out = {"name": name}
    sources = [f for f in os.listdir(src_dir) if f.rsplit(".", 1)[0] == base]
    code, text, err = run(["text", os.path.join(odt_dir, name)])
    out["code"] = code
    out["err"] = err[-200:]
    if code != 0:
        return out
    out["notes"] = [l[6:] for l in text.split("\n") if l.startswith("#note")]
    if sources:
        c2, t2, _ = run(["text", os.path.join(src_dir, sources[0])])
        if c2 == 0:
            out["match"] = match(words(text), words(t2))
            out["words"] = sum(words(t2).values())
    if rt_dir:
        saved = os.path.join(rt_dir, name)
        c3, t3, e3 = run(["odtsave", os.path.join(odt_dir, name), saved])
        out["save"] = c3
        out["saveerr"] = (t3 + e3)[-200:]
        if c3 == 0:
            c4, t4, _ = run(["text", saved])
            out["reopen"] = c4
            if c4 == 0:
                out["rtmatch"] = match(words(text), words(t4))
    return out


names = sorted(f for f in os.listdir(odt_dir) if f.endswith(".odt"))
if rt_dir:
    os.makedirs(rt_dir, exist_ok=True)
results = []
with concurrent.futures.ThreadPoolExecutor(workers) as ex:
    results = list(ex.map(one, names))
ok = [r for r in results if r["code"] == 0]
print("files", len(results), "opened", len(ok), "failed", len(results) - len(ok))
for r in results:
    if r["code"] != 0:
        print("FAIL", r["name"], r["code"], r["err"].replace("\n", " ")[:150])
m = [r for r in ok if "match" in r]
print("compared", len(m), "at>=99%", sum(1 for r in m if r["match"] >= 0.99), "at>=90%", sum(1 for r in m if r["match"] >= 0.90))
for r in sorted(m, key=lambda r: r["match"])[:15]:
    print("LOW %.3f" % r["match"], r["name"], r.get("words"))
if rt_dir:
    s = [r for r in ok if "save" in r]
    print("saved", sum(1 for r in s if r["save"] == 0), "of", len(s), "reopened", sum(1 for r in s if r.get("reopen") == 0))
    rt = [r for r in s if "rtmatch" in r]
    print("round-trip text at>=99%", sum(1 for r in rt if r["rtmatch"] >= 0.99), "of", len(rt))
    for r in s:
        if r["save"] != 0 or r.get("reopen", 0) != 0:
            print("RTFAIL", r["name"], r["save"], r.get("reopen"), r["saveerr"].replace("\n", " ")[:120])
    for r in sorted(rt, key=lambda r: r["rtmatch"])[:10]:
        print("RTLOW %.3f" % r["rtmatch"], r["name"])
