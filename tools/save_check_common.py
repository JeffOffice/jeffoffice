#!/usr/bin/env python3
"""What the two save-check scripts share (tools/crunch_save_checks.py and tools/write_save_checks.py): the lists of the public collections, the seeded sample, one
check in a process of its own under a time limit and a memory cap, the check that LibreOffice (if it is installed) opens each saved file, the results file, and the report.

Nothing from the collections is copied anywhere that is kept: the files the programs save are written under the work folder, opened in LibreOffice, and deleted (the ones that
differ or fail to open are kept until the next run, for a look). The report names files by their names only.

Results are lines of JSON in <out-dir>/<app>-results.jsonl, one for each (file, saved format); a run skips what is already there, so a run that is stopped goes on where it was.
"""
import collections
import concurrent.futures
import glob
import json
import os
import random
import shutil
import signal
import subprocess
import sys
import tempfile
import threading
import time

CORPORA = "/home/user/jo-corpora"
LISTS = CORPORA + "/work/lists"
SEED = "jeffoffice-save-checks-1"
LIMIT_SECONDS = 120          # for each file and format
MEMORY = "4G"
LO_BATCH = 8                 # files to a LibreOffice run
LO_BATCH_SECONDS = 240
LO_ONE_SECONDS = 90


def read_list(name):
    path = os.path.join(LISTS, name)
    if not os.path.exists(path):
        return []
    return [l.rstrip("\n") for l in open(path) if l.strip() and os.path.exists(l.rstrip("\n"))]


def find_files(root, exts):
    out = []
    for dirpath, _, names in os.walk(root):
        for n in names:
            if n.lower().rsplit(".", 1)[-1] in exts:
                out.append(os.path.join(dirpath, n))
    return sorted(out)


def ordered(collections_):
    """{collection: [paths]} -> one list of (collection, path): each collection shuffled with the fixed seed, then taken in turn (so a prefix of any length has every collection in it,
    the larger ones more often at the end), the same list every run."""
    lists = []
    for name in sorted(collections_):
        paths = sorted(collections_[name])
        random.Random(SEED + ":" + name).shuffle(paths)
        lists.append([(name, p) for p in paths])
    out = []
    i = 0
    while any(i < len(l) for l in lists):
        for l in lists:
            if i < len(l):
                out.append(l[i])
        i += 1
    return out


def run_capped(command, env_extra=None, limit=LIMIT_SECONDS, memory=MEMORY):
    """Runs the command in a scope with a memory cap and a time limit. Returns (kind, exit code, stdout, stderr): kind is "done", "hang" (the limit), "memory" (killed by the cap),
    or "crash" (killed by a signal, or an exit code the tool does not use)."""
    env = dict(os.environ, JP_NO_TELEMETRY="1", QT_QPA_PLATFORM="offscreen")
    env.update(env_extra or {})
    full = ["systemd-run", "--user", "--scope", "-p", "MemoryMax=" + memory, "-p", "MemorySwapMax=0", "-q", "timeout", "-k", "5", str(limit)] + command
    with tempfile.TemporaryFile() as errors, tempfile.TemporaryFile() as outs:
        child = subprocess.Popen(full, stdout=outs, stderr=errors, env=env, stdin=subprocess.DEVNULL)
        _, status, _ = os.wait4(child.pid, 0)
        outs.seek(0)
        errors.seek(0)
        out = outs.read(200000).decode("utf-8", "replace")
        err = errors.read(1000).decode("utf-8", "replace").strip().replace("\n", " | ")
    code = os.waitstatus_to_exitcode(status)
    if code < 0:
        code = 128 - code
    if code == 124:
        return "hang", code, out, err
    if code == 137:
        return "memory", code, out, err
    if code >= 128:
        return "crash", code, out, err
    return "done", code, out, err


def soffice_path():
    return shutil.which("soffice") or shutil.which("libreoffice")


class LibreOffice:
    """Opens saved files headless (a convert to the plain form of the first sheet or the text), several at a time in a run with a profile of its own, under a time limit. A file
    loads when the run makes its output; a run that stops or does not finish leaves its files to be tried one at a time."""

    def __init__(self, work, target):
        self.exe = soffice_path()
        self.work = work
        self.target = target          # "csv" for a workbook, "txt:Text (encoded):UTF8" for a document
        self.profiles = queue_of(os.path.join(work, "lo-profile-%d-%d" % (os.getpid(), i)) for i in range(2))

    def available(self):
        return self.exe is not None

    def _run(self, files, seconds):
        profile = self.profiles.get()
        out_dir = tempfile.mkdtemp(prefix="lo-out-", dir=self.work)
        try:
            command = [self.exe, "-env:UserInstallation=file://" + profile, "--headless", "--norestore", "--nolockcheck", "--convert-to", self.target, "--outdir", out_dir] + files
            child = subprocess.Popen(command, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, stdin=subprocess.DEVNULL, start_new_session=True)
            try:
                child.wait(seconds)
                timed_out = False
            except subprocess.TimeoutExpired:
                timed_out = True
                try:
                    os.killpg(child.pid, signal.SIGKILL)   # (the group this run started, no other)
                except ProcessLookupError:
                    pass
                child.wait()
            made = set(os.listdir(out_dir))
            return made, timed_out
        finally:
            shutil.rmtree(out_dir, ignore_errors=True)
            self.profiles.put(profile)

    def check(self, files):
        """{file: "loads" | "fails" | "timeout"}"""
        result = {}
        if not self.available():
            return {f: "" for f in files}
        made, timed_out = self._run(files, LO_BATCH_SECONDS)
        retry = []
        for f in files:
            stem = os.path.splitext(os.path.basename(f))[0]
            if any(m.startswith(stem + ".") for m in made):
                result[f] = "loads"
            elif timed_out or len(files) > 1:
                retry.append(f)
            else:
                result[f] = "fails"
        for f in retry:   # (one at a time: a file that stopped the batch is told from the files after it)
            made, timed_out = self._run([f], LO_ONE_SECONDS)
            stem = os.path.splitext(os.path.basename(f))[0]
            result[f] = "loads" if any(m.startswith(stem + ".") for m in made) else ("timeout" if timed_out else "fails")
        return result


def queue_of(items):
    import queue
    q = queue.Queue()
    for i in items:
        os.makedirs(i, exist_ok=True)
        q.put(i)
    return q


def load_results(path):
    done = {}
    if os.path.exists(path):
        for l in open(path):
            try:
                r = json.loads(l)
                done[(r["path"], r["fmt"])] = r
            except Exception:
                pass
    return done


class Results:
    def __init__(self, path):
        self.path = path
        self.lock = threading.Lock()
        self.done = load_results(path)

    def has(self, p, fmt):
        return (p, fmt) in self.done

    def add(self, rec):
        with self.lock:
            self.done[(rec["path"], rec["fmt"])] = rec
            with open(self.path, "a") as f:
                f.write(json.dumps(rec) + "\n")


def drive(items, run_item, jobs, deadline, label):
    """Runs run_item(item) for each item on `jobs` threads, starting no new one after the deadline (a time). Prints a summary line every 200 items."""
    count = [0]
    lock = threading.Lock()
    started = time.time()

    def work(item):
        if time.time() > deadline:
            return False
        run_item(item)
        with lock:
            count[0] += 1
            if count[0] % 200 == 0:
                print("%s: %d done in %.0f s" % (label, count[0], time.time() - started), flush=True)
        return True

    with concurrent.futures.ThreadPoolExecutor(jobs) as ex:
        ran = list(ex.map(work, items))
    return sum(1 for r in ran if r)


# ---- the report

def pct(a, b):
    return "%.1f%%" % (100.0 * a / b) if b else "-"


def report_app(title, results, formats, source_exts, expected_kinds, lo_available, notes):
    """The section of one app: a table of numbers for each saved format by the kind of file that was opened, then the differences by kind with three example names each, the ten
    most common differences, the crashes and the hangs."""
    lines = ["## %s" % title, ""]
    recs = [r for r in results.values() if r["fmt"] in formats]
    if not recs:
        return lines + ["(no results)", ""]
    lines.append("Saved as | Opened as | Tried | Opened | Saved | Reopened | Reopened the same | Same, format limits aside | Crashed | Hung | Over memory | Opens in LibreOffice" + ("" if lo_available else " (not installed: empty)"))
    lines.append("--- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---")
    groups = collections.defaultdict(list)
    for r in recs:
        groups[(r["fmt"], r["src"])].append(r)
        groups[(r["fmt"], "all")].append(r)
    for fmt in formats:
        for src in list(source_exts) + ["all"]:
            g = groups.get((fmt, src), [])
            if not g:
                continue
            tried = len(g)
            crashed = sum(1 for r in g if r["status"] == "crash")
            hung = sum(1 for r in g if r["status"] == "hang")
            mem = sum(1 for r in g if r["status"] == "memory")
            opened = sum(1 for r in g if r["status"] in ("ok", "differs", "not saved", "reopen failed"))
            saved = sum(1 for r in g if r["status"] in ("ok", "differs", "reopen failed"))
            reopened = sum(1 for r in g if r["status"] in ("ok", "differs"))
            same = sum(1 for r in g if r["status"] == "ok")
            same_ex = sum(1 for r in g if r["status"] == "ok" or (r["status"] == "differs" and set(r.get("kinds", [])) <= expected_kinds))
            lo = [r.get("lo", "") for r in g if r["status"] in ("ok", "differs")]
            lo_loads = sum(1 for x in lo if x == "loads")
            lo_text = "%d of %d" % (lo_loads, sum(1 for x in lo if x)) if any(lo) else ""
            lines.append("%s | %s | %d | %d | %d | %d | %d (%s) | %d (%s) | %d | %d | %d | %s" % (fmt, src, tried, opened, saved, reopened, same, pct(same, reopened), same_ex, pct(same_ex, reopened), crashed, hung, mem, lo_text))
    lines.append("")
    # refusals and failures to save or reopen, by reason
    for fmt in formats:
        g = [r for r in recs if r["fmt"] == fmt]
        refused = collections.Counter()
        names = collections.defaultdict(list)
        for r in g:
            if r["status"] in ("refused", "not saved", "reopen failed"):
                k = "%s: %s" % (r["status"], r.get("first", "")[:70])
                refused[k] += 1
                names[k].append(os.path.basename(r["path"]))
        if refused:
            lines.append("Not opened, not saved, or not reopened, saved as %s (the commonest reasons):" % fmt)
            lines.append("")
            for k, n in refused.most_common(6):
                lines.append("- %d x %s (e.g. %s)" % (n, k, ", ".join(names[k][:3])))
            lines.append("")
    # differences by kind
    for fmt in formats:
        kinds = collections.Counter()
        ex = collections.defaultdict(list)
        for r in recs:
            if r["fmt"] == fmt and r["status"] == "differs":
                for k in r.get("kinds", []):
                    kinds[k] += 1
                    if len(ex[k]) < 3:
                        ex[k].append(os.path.basename(r["path"]))
        if kinds:
            lines.append("Differences, saved as %s, by kind (files with each; three example names):" % fmt)
            lines.append("")
            for k, n in kinds.most_common():
                lines.append("- %s: %d (%s)" % (k, n, ", ".join(ex[k])))
            lines.append("")
    # the ten commonest differences over all formats
    kinds = collections.Counter()
    for r in recs:
        if r["status"] == "differs":
            for k in r.get("kinds", []):
                kinds[(k, r["fmt"])] += 1
    if kinds:
        lines.append("The ten most common differences (kind, saved as, files):")
        lines.append("")
        for i, ((k, fmt), n) in enumerate(kinds.most_common(10), 1):
            lines.append("%d. %s, %s: %d files" % (i, k, fmt, n))
        lines.append("")
    # crashes and hangs
    bad = [r for r in recs if r["status"] in ("crash", "hang", "memory")]
    if bad:
        lines.append("Crashes, hangs, and files over the memory cap (names only):")
        lines.append("")
        by = collections.defaultdict(list)
        for r in bad:
            by[(r["status"], r["fmt"])].append(os.path.basename(r["path"]))
        for (s, fmt), names_ in sorted(by.items()):
            lines.append("- %s, saved as %s: %d (%s)" % (s, fmt, len(names_), ", ".join(names_[:5])))
        lines.append("")
    # LibreOffice
    fails = [r for r in recs if r.get("lo") in ("fails", "timeout")]
    if fails:
        lines.append("Saved files LibreOffice did not load:")
        lines.append("")
        by = collections.defaultdict(list)
        for r in fails:
            by[(r["fmt"], r["lo"])].append(os.path.basename(r["path"]))
        for (fmt, s), names_ in sorted(by.items()):
            lines.append("- saved as %s, %s: %d (%s)" % (fmt, s, len(names_), ", ".join(names_[:3])))
        lines.append("")
    secs = sorted(r.get("seconds", 0) for r in recs)
    if secs:
        lines.append("Time for a file and format (open, save, reopen): middle %.2f s, 95th percentile %.2f s, longest %.1f s." % (secs[len(secs) // 2], secs[int(len(secs) * 0.95)], secs[-1]))
        lines.append("")
    lines += notes
    return lines


def collection_counts(results):
    c = collections.Counter()
    for r in results.values():
        c[(r["fmt"], r["collection"])] += 1
    return c


def assemble(out_dir):
    """REPORT.md from the apps' sections (and report-notes.md, which a person writes) that are there."""
    parts = ["# Save checks: does a file the program saves open again cleanly?", ""]
    for name in ("report-notes.md", "crunch-section.md", "write-section.md"):
        p = os.path.join(out_dir, name)
        if os.path.exists(p):
            parts.append(open(p).read())
    open(os.path.join(out_dir, "REPORT.md"), "w").write("\n".join(parts))
