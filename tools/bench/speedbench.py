#!/usr/bin/env python3
"""Speed comparison of the JeffOffice command-line tools and apps against LibreOffice, on the same files.

    python3 tools/bench/speedbench.py [--quick] [--build DIR] [--out DIR] [--only m1,m2,m3,m4] [--runs N]

Measures (wall time, CPU time, and peak resident memory of the slowest process of each run):
  convert   headless open + save as the same format, and open + save as PDF
  startup   offscreen start to the first drawn window (ours) vs. a headless first-page render (LibreOffice),
            and, on a private X server, start to the first mapped window for both
  recalc    full recalculation of big workbooks with formulas
  Peak memory is recorded in every row of every measure.

Results go to /home/user/jo-scratch/speedbench/ (CSV of every run, CSV per file, Markdown summary), never into the repo.
The corpus at /media/user/volume2/jo-corpora is read only; nothing is copied out of it except into a temp dir that is removed.
"""
import argparse, csv, glob, os, random, re, shlex, shutil, signal, statistics, subprocess, sys, tempfile, threading, time, zipfile

CORPUS = "/media/user/volume2/jo-corpora"
SEED = 20261105
TIMEOUT = 120
LO = "/usr/bin/soffice"
X_DISPLAY = ":97"
X_UNIT = "jo-speedbench-x"
CPUS = ""        # --cpus: a core list (taskset -c) both tools are pinned to


def pinned(cmd):
    return ["taskset", "-c", CPUS] + list(cmd) if CPUS else list(cmd)

KINDS = {"docx": "write", "xlsx": "crunch", "pptx": "deck"}


# ---------------------------------------------------------------- sampling

_SCAN = {}


def list_files(ext, native):
    """(path, size) of the files with this extension: native = everything outside work/ (the converted copies live there)."""
    if not _SCAN:
        for root, dirs, files in os.walk(CORPUS):
            top = os.path.relpath(root, CORPUS).split(os.sep)[0]
            for f in files:
                e = f.rsplit(".", 1)[-1].lower()
                if e in KINDS:
                    p = os.path.join(root, f)
                    try:
                        _SCAN.setdefault((e, top != "work"), []).append((p, os.path.getsize(p)))
                    except OSError:
                        pass
        for v in _SCAN.values():
            v.sort()
    return list(_SCAN.get((ext, native), []))


def pick_sample(quick):
    rng = random.Random(SEED)
    counts = {"docx": 40, "xlsx": 40, "pptx": 25}
    if quick:
        counts = {"docx": 4, "xlsx": 4, "pptx": 2}
    sample, big = {}, {}
    for ext, n in counts.items():
        pool = [x for x in list_files(ext, True) if 1000 < x[1] < 20_000_000]
        sample[ext] = sorted(rng.sample(pool, min(n, len(pool))))
        if quick:
            big[ext] = []
            continue
        # big files: the three largest under 30 MB (the converted copies in work/ hold the biggest ones), plus the largest native
        cand = [x for x in list_files(ext, False) if 2_000_000 < x[1] < 30_000_000]
        cand += [x for x in list_files(ext, True) if x[1] > 500_000]
        cand = sorted(set(cand), key=lambda x: -x[1])[:3]
        big[ext] = cand
    return sample, big


def formula_count(path, limit=25_000_000):
    """Formula elements in the sheet parts (counted from the zip, no opening)."""
    n = 0
    try:
        with zipfile.ZipFile(path) as z:
            for i in z.infolist():
                if i.filename.startswith("xl/worksheets/sheet") and i.file_size < limit:
                    n += len(re.findall(rb"<f[ >/]", z.read(i)))
    except Exception:
        return 0
    return n


def pick_recalc(quick):
    if quick:
        return []
    rng = random.Random(SEED)
    cands = [x for x in list_files("xlsx", True) + list_files("xlsx", False) if 300_000 < x[1] < 25_000_000]
    cands = rng.sample(cands, min(120, len(cands)))
    scored = [(formula_count(p), p, s) for p, s in cands]
    scored = sorted([x for x in scored if x[0] >= 5000], reverse=True)[:6]
    return [(p, s, f) for f, p, s in scored]


# ---------------------------------------------------------------- running

class Env:
    """A fresh home and config dir for one tool (and a fresh LibreOffice profile)."""
    def __init__(self, name):
        self.root = tempfile.mkdtemp(prefix="sb-" + name + "-")
        self.home = os.path.join(self.root, "home")
        self.profile = os.path.join(self.root, "lo-profile")
        for d in (self.home, os.path.join(self.home, ".config"), os.path.join(self.home, ".cache")):
            os.makedirs(d)

    def env(self, extra=None):
        e = {k: v for k, v in os.environ.items() if k in ("PATH", "LANG", "XDG_RUNTIME_DIR", "DBUS_SESSION_BUS_ADDRESS")}
        e.update(HOME=self.home, XDG_CONFIG_HOME=os.path.join(self.home, ".config"), XDG_CACHE_HOME=os.path.join(self.home, ".cache"),
                 JP_NO_TELEMETRY="1", QT_QPA_PLATFORM="offscreen")
        e.update(extra or {})
        return e

    def lo_args(self):
        return ["-env:UserInstallation=file://" + self.profile, "--headless", "--norestore", "--nolockcheck", "--nodefault"]

    def lo_always_recalc(self):
        d = os.path.join(self.profile, "user")
        os.makedirs(d, exist_ok=True)
        with open(os.path.join(d, "registrymodifications.xcu"), "w") as f:
            f.write('<?xml version="1.0" encoding="UTF-8"?>\n<oor:items xmlns:oor="http://openoffice.org/2001/registry" '
                    'xmlns:xs="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">\n'
                    '<item oor:path="/org.openoffice.Office.Calc/Formula/Load"><prop oor:name="OOXMLRecalcMode" oor:op="fuse"><value>0</value></prop></item>\n'
                    '<item oor:path="/org.openoffice.Office.Calc/Formula/Load"><prop oor:name="ODFRecalcMode" oor:op="fuse"><value>0</value></prop></item>\n'
                    '</oor:items>\n')

    def close(self):
        shutil.rmtree(self.root, ignore_errors=True)


def loadavg():
    return float(open("/proc/loadavg").read().split()[0])


def run(cmd, env, outputs=(), timeout=TIMEOUT, stdout_path=None):
    """One run: wall seconds, CPU seconds, peak RSS in MB, ok, note. Killed by process group at the timeout."""
    cmd = pinned(cmd)
    so = open(stdout_path, "wb") if stdout_path else subprocess.DEVNULL
    t0 = time.perf_counter()
    p = subprocess.Popen(cmd, env=env, stdout=so, stderr=subprocess.DEVNULL, stdin=subprocess.DEVNULL, start_new_session=True)
    timed_out = []

    def kill():
        timed_out.append(1)
        try:
            os.killpg(p.pid, signal.SIGKILL)
        except ProcessLookupError:
            pass
    timer = threading.Timer(timeout, kill)
    timer.start()
    _, status, ru = os.wait4(p.pid, 0)
    wall = time.perf_counter() - t0
    timer.cancel()
    if stdout_path:
        so.close()
    try:                                   # anything left in the group (the LibreOffice launcher's child)
        os.killpg(p.pid, signal.SIGKILL)
    except ProcessLookupError:
        pass
    rc = os.waitstatus_to_exitcode(status)
    ok = rc == 0 and not timed_out and all(os.path.exists(o) and os.path.getsize(o) > 0 for o in outputs)
    note = "timeout" if timed_out else ("rc=%d" % rc if rc else ("no output" if not ok else ""))
    return dict(wall=wall, cpu=ru.ru_utime + ru.ru_stime, rss=ru.ru_maxrss / 1024.0, ok=ok, note=note, load=loadavg())


COLS = ["measure", "kind", "tier", "file", "size", "tool", "ok", "note", "runs", "wall", "cpu", "rss", "load", "extra"]


class Rows(list):
    """The result rows; every row is also appended to a CSV at once, so a crash loses nothing."""
    def __init__(self, path):
        super().__init__()
        self.fh = open(path, "w", newline="")
        self.w = csv.DictWriter(self.fh, fieldnames=COLS, extrasaction="ignore")
        self.w.writeheader()

    def append(self, r):
        r.setdefault("extra", "")
        super().append(r)
        self.w.writerow({k: ("%.4f" % v if isinstance(v, float) else v) for k, v in r.items()})
        self.fh.flush()


def read_rows(path):
    rows = []
    for r in csv.DictReader(open(path)):
        for k in ("wall", "cpu", "rss", "load"):
            r[k] = float(r[k])
        r["ok"] = r["ok"] == "True"
        rows.append(r)
    return rows


def median(v):
    return statistics.median(v) if v else float("nan")


def pct(v, q):
    if not v:
        return float("nan")
    v = sorted(v)
    return v[min(len(v) - 1, int(round(q * (len(v) - 1))))]


def measure(make_cmd, env, outputs_fn, runs, tmp, extra_env=None, stdout=False):
    """Run `runs` times (more when noisy); returns the aggregate of the successful runs."""
    rs = []
    n = runs
    i = 0
    while i < n:
        out_dir = tempfile.mkdtemp(dir=tmp)
        sp = os.path.join(out_dir, "stdout.txt") if stdout else None
        r = run(make_cmd(out_dir), env.env(extra_env), outputs_fn(out_dir), stdout_path=sp)
        r["stdout"] = open(sp).read() if sp and os.path.exists(sp) else ""
        shutil.rmtree(out_dir, ignore_errors=True)
        rs.append(r)
        i += 1
        if not r["ok"]:
            break                            # a failure is counted, not retried
        good = [x["wall"] for x in rs if x["ok"]]
        if i == n and runs > 1 and n == runs and max(good) > 1.3 * min(good) and max(good) > 0.3:
            n = runs + 2                     # noisy: two more runs, the median is taken over all
    good = [x for x in rs if x["ok"]]
    if not good:
        return dict(ok=False, note=rs[-1]["note"], runs=len(rs), wall=float("nan"), cpu=float("nan"), rss=float("nan"), load=rs[-1]["load"], stdout="")
    return dict(ok=True, note="", runs=len(rs), wall=median([x["wall"] for x in good]), cpu=median([x["cpu"] for x in good]),
                rss=max(x["rss"] for x in good), load=max(x["load"] for x in good), stdout=good[-1]["stdout"])


# ---------------------------------------------------------------- the tools

class Tools:
    def __init__(self, build, bench_bin):
        self.b = build
        self.bench = bench_bin

    def ours_convert(self, ext, src, dst_dir):
        out = os.path.join(dst_dir, "out." + ext)
        if ext == "docx":
            return [self.b + "/writetool", "docsave", src, out], out
        if ext == "xlsx":
            return [self.bench, src, out], out
        return [self.b + "/jdecktool", "save", src, out], out

    def ours_pdf(self, ext, src, dst_dir):
        out = os.path.join(dst_dir, "out.pdf")
        if ext == "docx":
            return [self.b + "/writetool", "pdf", src, out], out
        if ext == "xlsx":
            return [self.b + "/jcrunchtool", "print-pdf", src, out], out
        return [self.b + "/jdecktool", "pdf", src, out], out

    def lo_convert(self, lo_args, fmt, src, dst_dir):
        out = os.path.join(dst_dir, os.path.splitext(os.path.basename(src))[0] + "." + fmt)
        return [LO] + lo_args + ["--convert-to", fmt, "--outdir", dst_dir, src], out


def build_helper(build, out_bin):
    """Compile the xlsx open+save helper with the flags of jcrunchtool (taken from ninja)."""
    src = os.path.join(os.path.dirname(os.path.abspath(__file__)), "xlsxsave_bench.cpp")
    if os.path.exists(out_bin) and os.path.getmtime(out_bin) > os.path.getmtime(src) and os.path.getmtime(out_bin) > os.path.getmtime(build + "/jcrunchtool"):
        return True
    cmds = subprocess.run(["ninja", "-C", build, "-t", "commands", "jcrunchtool"], capture_output=True, text=True).stdout.strip().splitlines()
    comp = next(c for c in cmds if "jcrunchtool_filter.cpp.o" in c and " -c " in c)
    link = next(p for p in cmds[-1].split(" && ") if " -o " in p)
    toks = shlex.split(comp)
    o = toks.index("-o")
    toks[o + 1] = out_bin + ".o"
    toks[toks.index("-c") + 1] = src
    # drop the dependency-file options (they would write into the build tree)
    for flag in ("-MD", "-MT", "-MF"):
        while flag in toks:
            i = toks.index(flag)
            del toks[i:i + (1 if flag == "-MD" else 2)]
    # the compiler may be a launcher plus the compiler; run it from the build dir
    r = subprocess.run(toks, cwd=build, capture_output=True, text=True)
    if r.returncode:
        print("helper compile failed:", r.stderr[-500:])
        return False
    lt = shlex.split(link)
    o = lt.index("-o")
    lt[o + 1] = out_bin
    lt = [t for t in lt if not (t.startswith("CMakeFiles/jcrunchtool.dir/") and t.endswith(".o"))]
    lt.insert(lt.index("-o") + 2, out_bin + ".o")
    r = subprocess.run(lt, cwd=build, capture_output=True, text=True)
    if r.returncode:
        print("helper link failed:", r.stderr[-500:])
        return False
    return True


# ---------------------------------------------------------------- measures

def fmt_row(rows, **kw):
    rows.append(kw)


def m_convert(rows, tools, files, runs, tmp, log):
    ours, lo = Env("ours"), Env("lo")
    try:
        for ext, tier_lists in files.items():
            tiny = min((x for t in tier_lists.values() for x in t), key=lambda x: x[1], default=None)
            if tiny is None:
                continue
            for what in ("save", "pdf"):
                # warm-up: the first run of each tool on a tiny file (starts the disk cache and the LibreOffice profile); not recorded
                for tool in ("ours", "lo"):
                    cmd, out = (tools.ours_convert if what == "save" else tools.ours_pdf)(ext, tiny[0], tmp) if tool == "ours" else \
                        tools.lo_convert(lo.lo_args(), ext if what == "save" else "pdf", tiny[0], tmp)
                    run(cmd, (ours if tool == "ours" else lo).env(), timeout=TIMEOUT)
                for tier, lst in tier_lists.items():
                    for path, size in lst:
                        with open(path, "rb") as f:
                            while f.read(1 << 22):
                                pass
                        for tool in ("ours", "lo"):
                            env = ours if tool == "ours" else lo
                            holder = {}

                            def mk(d, tool=tool, env=env):
                                if tool == "ours":
                                    cmd, out = (tools.ours_convert if what == "save" else tools.ours_pdf)(ext, path, d)
                                else:
                                    cmd, out = tools.lo_convert(env.lo_args(), ext if what == "save" else "pdf", path, d)
                                holder["out"] = out
                                return cmd
                            r = measure(mk, env, lambda d: [holder["out"]], runs, tmp, stdout=False)
                            rows.append(dict(measure="convert-" + what, kind=ext, tier=tier, file=os.path.relpath(path, CORPUS), size=size, tool=tool, **{k: r[k] for k in ("ok", "note", "runs", "wall", "cpu", "rss", "load")}))
                        log("convert-%s %s %s" % (what, ext, os.path.basename(path)))
    finally:
        ours.close(); lo.close()


def m_recalc(rows, tools, recalc_files, runs, tmp, log):
    ours, lo, lo_plain = Env("ours"), Env("lo"), Env("lop")
    lo.lo_always_recalc()
    try:
        if not recalc_files:
            return
        tiny = recalc_files[-1]
        for e, a in ((ours, [tools.b + "/jcrunchtool", "recalc-full-one", tiny[0], "0", "100"]),
                     (lo, [LO] + lo.lo_args() + ["--convert-to", "xlsx", "--outdir", tmp, tiny[0]]),
                     (lo_plain, [LO] + lo_plain.lo_args() + ["--convert-to", "xlsx", "--outdir", tmp, tiny[0]])):
            run(a, e.env())
        for path, size, nform in recalc_files:
            rel = os.path.relpath(path, CORPUS)
            with open(path, "rb") as f:
                while f.read(1 << 22):
                    pass
            r = measure(lambda d: [tools.b + "/jcrunchtool", "recalc-full-one", path, "0", "100"], ours, lambda d: [], runs, tmp, stdout=True)
            calc = float("nan")
            m = [l for l in r["stdout"].splitlines() if l.startswith("fullbook\t")]
            if m:
                f = m[-1].split("\t")
                try:
                    calc = float(f[27])
                except (IndexError, ValueError):
                    pass
            rows.append(dict(measure="recalc-total", kind="xlsx", tier="recalc", file=rel, size=size, tool="ours", **{k: r[k] for k in ("ok", "note", "runs", "wall", "cpu", "rss", "load")}, extra="formulas=%d" % nform))
            rows.append(dict(measure="recalc-only", kind="xlsx", tier="recalc", file=rel, size=size, tool="ours", ok=r["ok"] and calc == calc, note="", runs=r["runs"], wall=calc, cpu=float("nan"), rss=r["rss"], load=r["load"], extra="calc phase reported by the tool"))
            res = {}
            for tag, env in (("lo-recalc", lo), ("lo-plain", lo_plain)):
                holder = {}

                def mk(d, env=env):
                    c, o = tools.lo_convert(env.lo_args(), "xlsx", path, d)
                    holder["o"] = o
                    return c
                res[tag] = measure(mk, env, lambda d: [holder["o"]], runs, tmp)
            a, b = res["lo-recalc"], res["lo-plain"]
            rows.append(dict(measure="recalc-total", kind="xlsx", tier="recalc", file=rel, size=size, tool="lo", **{k: a[k] for k in ("ok", "note", "runs", "wall", "cpu", "rss", "load")}, extra="load+recalc+save, recalc forced"))
            d_ok = a["ok"] and b["ok"]
            rows.append(dict(measure="recalc-only", kind="xlsx", tier="recalc", file=rel, size=size, tool="lo", ok=d_ok, note="", runs=a["runs"], wall=(a["wall"] - b["wall"]) if d_ok else float("nan"),
                             cpu=(a["cpu"] - b["cpu"]) if d_ok else float("nan"), rss=a["rss"], load=a["load"], extra="forced minus default convert"))
            log("recalc %s" % os.path.basename(path))
    finally:
        ours.close(); lo.close(); lo_plain.close()


# --- startup on offscreen and on a private X server

def start_x():
    if os.path.exists("/tmp/.X11-unix/X97"):
        return True
    subprocess.run(["systemd-run", "--user", "--unit=" + X_UNIT, "-p", "MemoryMax=4G", "-q", "/usr/lib/xorg/Xorg", X_DISPLAY, "-noreset", "-nolisten", "tcp",
                    "-config", "xrdp/xorg.conf", "-logfile", "/tmp/speedbench-xorg.log"], capture_output=True)
    for _ in range(50):
        if os.path.exists("/tmp/.X11-unix/X97"):
            time.sleep(0.5)
            return True
        time.sleep(0.2)
    return False


def stop_x():
    subprocess.run(["systemctl", "--user", "stop", X_UNIT], capture_output=True)


def first_window_time(cmd, env, timeout=60):
    """Seconds from launch until a top-level window is mapped on the X server (None if none appears)."""
    import ctypes
    from ctypes import c_int, c_uint, c_ulong, c_void_p, byref, POINTER, Structure
    x = ctypes.CDLL("libX11.so.6")
    x.XOpenDisplay.restype = c_void_p
    x.XOpenDisplay.argtypes = [ctypes.c_char_p]
    x.XDefaultRootWindow.restype = c_ulong
    x.XDefaultRootWindow.argtypes = [c_void_p]
    x.XQueryTree.argtypes = [c_void_p, c_ulong, POINTER(c_ulong), POINTER(c_ulong), POINTER(POINTER(c_ulong)), POINTER(c_uint)]
    x.XFree.argtypes = [c_void_p]

    class Attr(Structure):
        _fields_ = [("x", c_int), ("y", c_int), ("w", c_int), ("h", c_int), ("border", c_int), ("depth", c_int), ("visual", c_void_p), ("root", c_ulong),
                    ("cls", c_int), ("bit_gravity", c_int), ("win_gravity", c_int), ("backing_store", c_int), ("backing_planes", c_ulong), ("backing_pixel", c_ulong),
                    ("save_under", c_int), ("colormap", c_ulong), ("map_installed", c_int), ("map_state", c_int), ("all_ev", ctypes.c_long), ("your_ev", ctypes.c_long),
                    ("dont_prop", ctypes.c_long), ("override", c_int), ("screen", c_void_p)]
    x.XGetWindowAttributes.argtypes = [c_void_p, c_ulong, POINTER(Attr)]
    handler = ctypes.CFUNCTYPE(c_int, c_void_p, c_void_p)(lambda d, e: 0)    # a window that vanishes between two calls is not an error
    x.XSetErrorHandler.argtypes = [ctypes.c_void_p]
    x.XSetErrorHandler(ctypes.cast(handler, ctypes.c_void_p))
    dpy = x.XOpenDisplay(X_DISPLAY.encode())
    if not dpy:
        return None, None, 0
    root = x.XDefaultRootWindow(dpy)
    r0, par0, ch0, n0 = c_ulong(), c_ulong(), POINTER(c_ulong)(), c_uint()
    x.XQueryTree(dpy, root, byref(r0), byref(par0), byref(ch0), byref(n0))
    before = {ch0[i] for i in range(n0.value)}      # windows that exist before the program starts
    if ch0:
        x.XFree(ch0)
    t0 = time.perf_counter()
    p = subprocess.Popen(pinned(cmd), env=dict(env, DISPLAY=X_DISPLAY), stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, stdin=subprocess.DEVNULL, start_new_session=True)
    found = None
    large = None
    while time.perf_counter() - t0 < timeout and p.poll() is None:
        r, par, ch, n = c_ulong(), c_ulong(), POINTER(c_ulong)(), c_uint()
        x.XQueryTree(dpy, root, byref(r), byref(par), byref(ch), byref(n))
        for i in range(n.value):
            a = Attr()
            if ch[i] not in before and x.XGetWindowAttributes(dpy, ch[i], byref(a)) and a.map_state == 2 and a.w > 100 and a.h > 100:
                now = time.perf_counter() - t0
                if found is None:
                    found = now
                if a.w >= 600 and a.h >= 400 and large is None:
                    large = now
        if ch:
            x.XFree(ch)
        if large is not None:
            break
        time.sleep(0.005)
    try:
        os.killpg(p.pid, signal.SIGKILL)
    except ProcessLookupError:
        pass
    try:
        _, _, ru = os.wait4(p.pid, 0)
        rss = ru.ru_maxrss / 1024.0
    except ChildProcessError:
        rss = 0
    return found, large, rss


def m_startup(rows, tools, files, runs, tmp, log, use_x):
    ours, lo = Env("ours"), Env("lo")
    apps = {"docx": ("JeffWrite", "--writer"), "xlsx": ("JeffCrunch", "--calc"), "pptx": ("JeffDeck", "--impress")}
    try:
        for ext, (app, lo_flag) in apps.items():
            all_files = [x for t in files[ext].values() for x in t]
            if not all_files:
                continue
            tiny = min(all_files, key=lambda x: x[1])
            big = max(all_files, key=lambda x: x[1])
            png_holder = {}
            # warm-up (not recorded)
            run([tools.b + "/" + app, "--screenshot", os.path.join(tmp, "w.png")], ours.env())
            run([LO] + lo.lo_args() + ["--convert-to", "png", "--outdir", tmp, tiny[0]], lo.env())
            cases = [("empty", None, None), ("tiny", tiny, "tiny"), ("big", big, "big")]
            for label, f, _ in cases:
                path = f[0] if f else None
                size = f[1] if f else 0
                rel = os.path.relpath(path, CORPUS) if path else "(empty document)"
                r = measure(lambda d: [tools.b + "/" + app, "--screenshot", os.path.join(d, "s.png")] + ([path] if path else []), ours, lambda d: [os.path.join(d, "s.png")], runs, tmp)
                rows.append(dict(measure="startup-offscreen-first-draw", kind=ext, tier=label, file=rel, size=size, tool="ours", **{k: r[k] for k in ("ok", "note", "runs", "wall", "cpu", "rss", "load")}))
                if path:
                    r = measure(lambda d: [LO] + lo.lo_args() + ["--convert-to", "png", "--outdir", d, path], lo, lambda d: [os.path.join(d, os.path.splitext(os.path.basename(path))[0] + ".png")], runs, tmp)
                else:
                    r = measure(lambda d: [LO] + lo.lo_args() + ["--terminate_after_init"], lo, lambda d: [], runs, tmp)
                rows.append(dict(measure="startup-offscreen-first-draw", kind=ext, tier=label, file=rel, size=size, tool="lo", **{k: r[k] for k in ("ok", "note", "runs", "wall", "cpu", "rss", "load")},
                                 extra="headless first-page render" if path else "terminate_after_init: start without a window"))
                if use_x:
                    for tool in ("ours", "lo"):
                        ts, tl, rss = [], [], 0
                        for _ in range(runs):
                            if tool == "ours":
                                cmd = [tools.b + "/" + app] + ([path] if path else [])
                                env = ours.env(dict(QT_QPA_PLATFORM="xcb"))
                            else:
                                cmd = [LO, "-env:UserInstallation=file://" + lo.profile, "--norestore", "--nologo", "--nolockcheck", path if path else lo_flag]
                                env = lo.env(dict(QT_QPA_PLATFORM=""))
                                env.pop("QT_QPA_PLATFORM", None)
                            t, tb, rs_ = first_window_time(cmd, env)
                            time.sleep(0.5)
                            if t is not None:
                                ts.append(t)
                            if tb is not None:
                                tl.append(tb)
                            rss = max(rss, rs_) if tool == "ours" else float("nan")
                        for name, v, note in (("startup-x-first-window", ts, "first mapped window, over 100x100; ours is the splash" if tool == "ours" else "first mapped window, over 100x100"),
                                              ("startup-x-main-window", tl, "first mapped window of 600x400 or more (the 800x600 screen caps the size)")):
                            rows.append(dict(measure=name, kind=ext, tier=label, file=rel, size=size, tool=tool, ok=bool(v), note="" if v else "no window", runs=runs,
                                             wall=median(v), cpu=float("nan"), rss=rss, load=loadavg(), extra=note))
                log("startup %s %s" % (ext, label))
    finally:
        ours.close(); lo.close()


# ---------------------------------------------------------------- report

def summarize(rows, out_md, meta):
    groups = {}
    for r in rows:
        groups.setdefault((r["measure"], r["kind"], r["tier"]), {}).setdefault(r["file"], {})[r["tool"]] = r
    lines = ["# Speed bench: JeffOffice vs LibreOffice", "", meta, ""]
    lines.append("Wall seconds unless noted. Median and p90 are over files; ratio is the median of ours/LibreOffice over files both tools finished (above 1.0 means we are slower). RSS is the peak of one process; fails count files that errored, timed out (%d s), or wrote nothing." % TIMEOUT)
    lines.append("")
    lines.append("Notes: startup-offscreen compares our offscreen start to a first drawn window (--screenshot) with LibreOffice's headless first-page render (the nearest thing it has; for the empty case it is a start with no window, so not comparable and left out of the losses list). startup-x is the time to a mapped window on a private X server; ours shows a splash first (first-window) and the main window later (main-window); LibreOffice here starts without a splash. convert-save for xlsx uses a helper that reads every sheet and writes the file; the writer copies unchanged parts, so it is cheaper than re-serializing.")
    lines.append("")
    lines.append("| measure | kind | tier | files | ours med | ours p90 | LO med | LO p90 | ratio med | ratio worst | ours cpu med | LO cpu med | ours RSS MB med/max | LO RSS MB med/max | ours fails | LO fails |")
    lines.append("|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|")
    losses = []
    for (m, k, t), files in sorted(groups.items()):
        o = [v["ours"] for v in files.values() if "ours" in v]
        l = [v["lo"] for v in files.values() if "lo" in v]
        ow = [x["wall"] for x in o if x["ok"]]
        lw = [x["wall"] for x in l if x["ok"]]
        oc = [x["cpu"] for x in o if x["ok"] and x["cpu"] == x["cpu"]]
        lc = [x["cpu"] for x in l if x["ok"] and x["cpu"] == x["cpu"]]
        orss = [x["rss"] for x in o if x["ok"]]
        lrss = [x["rss"] for x in l if x["ok"]]
        ratios = []
        for f, v in files.items():
            if "ours" in v and "lo" in v and v["ours"]["ok"] and v["lo"]["ok"] and v["lo"]["wall"] > 0.001 and v["ours"]["wall"] == v["ours"]["wall"]:
                ratios.append((v["ours"]["wall"] / v["lo"]["wall"], f, v["ours"]["wall"], v["lo"]["wall"]))
                if not (m.startswith("startup-offscreen") and t == "empty"):      # (LibreOffice's empty case has no window: not comparable)
                    losses.append((ratios[-1][0], m, k, f, v["ours"]["wall"], v["lo"]["wall"]))
        rr = [x[0] for x in ratios]
        fm = lambda v: "%.2f" % v if v == v else "-"
        lines.append("| %s | %s | %s | %d | %s | %s | %s | %s | %s | %s | %s | %s | %s/%s | %s/%s | %d | %d |" % (
            m, k, t, len(files), fm(median(ow)), fm(pct(ow, .9)), fm(median(lw)), fm(pct(lw, .9)), fm(median(rr)), fm(max(rr) if rr else float("nan")),
            fm(median(oc)), fm(median(lc)), "%.0f" % median(orss) if orss else "-", "%.0f" % max(orss) if orss else "-", "%.0f" % median(lrss) if lrss else "-", "%.0f" % max(lrss) if lrss else "-",
            sum(1 for x in o if not x["ok"]), sum(1 for x in l if not x["ok"])))
    lines += ["", "## Largest losses (ours/LibreOffice wall ratio; only runs where ours takes over 0.3 s)", ""]
    for ratio, m, k, f, ow, lw in sorted([x for x in losses if x[4] > 0.3], reverse=True)[:12]:
        lines.append("- %.1fx  %s %s  %s  (ours %.2f s, LO %.2f s)" % (ratio, m, k, f, ow, lw))
    with open(out_md, "w") as fh:
        fh.write("\n".join(lines) + "\n")


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    repo = os.path.abspath(os.path.join(here, "..", ".."))
    ap = argparse.ArgumentParser()
    ap.add_argument("--quick", action="store_true")
    ap.add_argument("--build", default=os.path.join(repo, "build"))
    ap.add_argument("--out", default="/home/user/jo-scratch/speedbench")
    ap.add_argument("--only", default="convert,startup,recalc")
    ap.add_argument("--runs", type=int, default=0)
    ap.add_argument("--summarize", help="only write the summary from this results CSV")
    ap.add_argument("--cpus", default="", help="pin both tools to these cores, e.g. 0-7 (taskset -c); recorded in the summary")
    ap.add_argument("--no-x", action="store_true", help="skip the private X server measurements")
    a = ap.parse_args()
    global CPUS
    CPUS = a.cpus
    out = os.path.abspath(a.out)
    if a.summarize:
        summarize(read_rows(a.summarize), a.summarize.replace("results-", "summary-").replace(".csv", ".md"), "Summary rebuilt from " + a.summarize)
        return
    if os.path.commonpath([out, repo]) == repo:
        sys.exit("the output directory must be outside the repo")
    os.makedirs(out, exist_ok=True)
    runs = a.runs or (1 if a.quick else 3)
    tmp = tempfile.mkdtemp(prefix="sb-work-")
    stamp = time.strftime("%Y%m%d-%H%M%S") + ("-quick" if a.quick else "")

    # one scope for the whole run: 8 GB cap, for both tools alike
    if os.environ.get("SPEEDBENCH_SCOPE") != "1" and shutil.which("systemd-run"):
        os.environ["SPEEDBENCH_SCOPE"] = "1"
        os.execvp("systemd-run", ["systemd-run", "--user", "--scope", "-q", "-p", "MemoryMax=8G", sys.executable] + sys.argv)

    log = lambda s: print(time.strftime("%H:%M:%S"), s, flush=True)
    bench_bin = os.path.join(out, "xlsxsave_bench")
    if not build_helper(a.build, bench_bin):
        sys.exit("helper build failed")
    tools = Tools(a.build, bench_bin)
    sample, big = pick_sample(a.quick)
    files = {e: {"sample": sample[e], "big": big[e]} for e in sample}
    recalc_files = pick_recalc(a.quick)
    with open(os.path.join(out, "sample-%s.txt" % stamp), "w") as f:
        for e in files:
            for t, lst in files[e].items():
                for p, s in lst:
                    f.write("%s\t%s\t%d\t%s\n" % (e, t, s, os.path.relpath(p, CORPUS)))
        for p, s, n in recalc_files:
            f.write("recalc\txlsx\t%d\t%s\tformulas=%d\n" % (s, os.path.relpath(p, CORPUS), n))
    only = a.only.split(",")
    csv_path = os.path.join(out, "results-%s.csv" % stamp)
    rows = Rows(csv_path)
    t0 = time.time()
    la0 = loadavg()
    use_x = False
    try:
        if "startup" in only:
            use_x = not a.no_x and start_x()
            m_startup(rows, tools, files, runs, tmp, log, use_x)
        if "recalc" in only:
            m_recalc(rows, tools, recalc_files, runs, tmp, log)
        if "convert" in only:
            m_convert(rows, tools, files, runs, tmp, log)
    finally:
        if use_x:
            stop_x()
        shutil.rmtree(tmp, ignore_errors=True)
    rows.fh.close()
    try:
        lo_ver = subprocess.run([LO, "--version"], capture_output=True, text=True).stdout.strip()
    except Exception:
        lo_ver = "?"
    meta = "Run %s, %s. Build: %s. Runs per file: %d (median; two extra when the first three differ by over 30%%). Load average %.1f at start, %.1f at end. X server measures: %s. Warm-up run per tool and measure excluded. Cores: %s. The load average of every run is in the CSV (column load)." % (
        stamp, lo_ver, a.build, runs, la0, loadavg(), "on" if use_x else "off", CPUS or "all (not pinned)")
    md = os.path.join(out, "summary-%s.md" % stamp)
    summarize(rows, md, meta)
    shutil.copy(md, os.path.join(out, "summary-latest.md"))
    shutil.copy(csv_path, os.path.join(out, "results-latest.csv"))
    log("done in %.0f s: %s" % (time.time() - t0, md))


if __name__ == "__main__":
    main()
