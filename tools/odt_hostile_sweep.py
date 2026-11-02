#!/usr/bin/env python3
"""Runs damaged copies of .odt files through JeffWrite's reader: each file is cut short, has bytes of content.xml or styles.xml changed, cut out, or doubled (and is packed again), and
is read by `writetool text` under a timeout and a memory limit. A copy may be refused or opened; it may not crash, hang, or use more than the limit.

usage: odt_hostile_sweep.py <writetool> <odt-folder> <scratch-folder> [copies-per-file=3] [workers=6]
Prints counts only.
"""
import concurrent.futures, io, os, random, subprocess, sys, zipfile

tool, src, scratch = sys.argv[1:4]
copies = int(sys.argv[4]) if len(sys.argv) > 4 else 3
workers = int(sys.argv[5]) if len(sys.argv) > 5 else 6
env = dict(os.environ, JP_NO_TELEMETRY="1", QT_QPA_PLATFORM="offscreen")
os.makedirs(scratch, exist_ok=True)


def mutate(data, rnd, kind):
    if kind == 0:
        return data[: rnd.randrange(1, len(data))]
    z = zipfile.ZipFile(io.BytesIO(data))
    out = io.BytesIO()
    with zipfile.ZipFile(out, "w") as w:
        for info in z.infolist():
            body = z.read(info.filename)
            if info.filename in ("content.xml", "styles.xml") and len(body) > 10:
                for _ in range(rnd.randrange(1, 6)):
                    at = rnd.randrange(len(body))
                    if kind == 1:
                        body = body[:at] + bytes([rnd.randrange(256)]) + body[at + 1:]
                    elif kind == 2:
                        body = body[:at] + body[at + rnd.randrange(1, 200):]
                    else:
                        body = body[:at] + body[at: at + rnd.randrange(1, 400)] + body[at:]
            w.writestr(info.filename, body, compress_type=zipfile.ZIP_STORED if info.filename == "mimetype" else zipfile.ZIP_DEFLATED)
    return out.getvalue()


def one(args):
    name, i = args
    rnd = random.Random(hash((name, i)) & 0xFFFFFFFF)
    data = open(os.path.join(src, name), "rb").read()
    try:
        bad = mutate(data, rnd, i % 4)
    except Exception:
        return ("skipped", name, i)
    path = os.path.join(scratch, "%s.%d.odt" % (name[:-4], i))
    open(path, "wb").write(bad)
    try:
        p = subprocess.run(["bash", "-c", "ulimit -v 8000000; exec \"$@\"", "x", tool, "text", path], capture_output=True, timeout=60, env=env)
        verdict = "crash" if p.returncode < 0 or p.returncode >= 128 else ("opened" if p.returncode == 0 else "refused")
    except subprocess.TimeoutExpired:
        verdict = "hang"
    if verdict in ("opened", "refused"):
        os.remove(path)
    return (verdict, name, i)


jobs = [(n, i) for n in sorted(os.listdir(src)) if n.endswith(".odt") for i in range(copies)]
with concurrent.futures.ThreadPoolExecutor(workers) as ex:
    results = list(ex.map(one, jobs))
counts = {}
for v, _, _ in results:
    counts[v] = counts.get(v, 0) + 1
print("copies", len(results), counts)
for v, n, i in results:
    if v in ("crash", "hang"):
        print(v.upper(), n, i)
