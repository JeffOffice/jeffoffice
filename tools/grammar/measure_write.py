#!/usr/bin/env python3
"""Measures the grammar rules on the JeffWrite DEVELOPMENT documents, through the program's own reader and the check the window uses: every story of each
document (text, table cells, endnotes, headers, footers, footnotes, text boxes), not only the body paragraphs.

  python3 -I tools/grammar/measure_write.py run --tool build/writetool --scratch DIR [--sample 30] [--seed 1] [--jobs 8]
      writes DIR/flags.tsv (every flag: key, story kind, rule, text, context, suggestion), DIR/words.tsv (words by kind of story), and DIR/table.tsv + DIR/samples.tsv
      (a random sample of --sample flags of each rule, for reading: id, rule, kind, context, suggestion, and an empty column for the verdict).
  python3 -I tools/grammar/measure_write.py gate --scratch DIR --verdicts FILE [--need 0.9]
      FILE has one line for each flag read: the sample id (from samples.tsv), a tab, and "right" or "wrong". Prints, for each rule, the flags per 10,000 words, the flags read, and the
      share right, and says SHIP when the share is at least --need (and at least one flag was read), else NARROW; exits 1 if a rule that has flags was not read.

Only the keys of the development lists of ~/doc-corpus (word/dev.txt, word2/dev2.txt, word3/dev3.txt) are opened. The held-out lists (test*.txt) are never read, and the script stops
if a development key is in one. Nothing is copied into the repo. A rule with no flags has no sample: it is shown with its word count, and only its tests prove that it flags.
"""
import argparse, concurrent.futures, glob, os, random, subprocess, sys

CORPUS = os.path.expanduser("~/doc-corpus")


def keys(listfile):
    return [l.split()[0] for l in open(listfile) if l.strip()]


def dev_files():
    held = set()
    for f in glob.glob(os.path.join(CORPUS, "*", "test*.txt")):
        held |= set(keys(f))
    out = []
    for d, lst in (("word", "dev.txt"), ("word2", "dev2.txt"), ("word3", "dev3.txt")):
        for k in keys(os.path.join(CORPUS, d, lst)):
            if k in held:
                sys.exit("a development key is in a held-out list: " + k)
            out.append((k, os.path.join(CORPUS, d, "in", k + ".docx")))
    return out


def one(tool, item):
    key, path = item
    try:
        r = subprocess.run([tool, "grammar", path], capture_output=True, timeout=300, env=dict(os.environ, QT_QPA_PLATFORM="offscreen"))
    except Exception as e:
        return key, [], str(e)
    return key, r.stdout.decode("utf-8", "replace").splitlines(), "" if r.returncode == 0 else "exit %d" % r.returncode


def run(a):
    os.makedirs(a.scratch, exist_ok=True)
    files = dev_files()
    flags, words, failed = [], {}, 0
    with concurrent.futures.ThreadPoolExecutor(a.jobs) as ex:
        for key, lines, err in ex.map(lambda it: one(a.tool, it), files):
            if err:
                failed += 1
            for l in lines:
                p = l.split("\t")
                if p[0] == "words" and len(p) >= 4:
                    w = words.setdefault(p[1], [0, 0])
                    w[0] += int(p[2]); w[1] += int(p[3])
                elif p[0] == "flag" and len(p) >= 6:
                    flags.append([key] + p[1:6])
    with open(os.path.join(a.scratch, "flags.tsv"), "w", encoding="utf-8") as o:
        for f in flags:
            o.write("\t".join(f) + "\n")
    with open(os.path.join(a.scratch, "words.tsv"), "w") as o:
        for k, (w, p) in sorted(words.items()):
            o.write("%s\t%d\t%d\n" % (k, w, p))
    total = sum(w for w, _ in words.values())
    rng = random.Random(a.seed)
    by = {}
    for f in flags:
        by.setdefault(f[2], []).append(f)
    with open(os.path.join(a.scratch, "samples.tsv"), "w", encoding="utf-8") as s, open(os.path.join(a.scratch, "table.tsv"), "w") as t:
        for rule in sorted(by):
            fl = by[rule]
            pick = fl if len(fl) <= a.sample else rng.sample(fl, a.sample)
            kinds = {}
            for f in fl:
                kinds[f[1]] = kinds.get(f[1], 0) + 1
            t.write("%s\t%d\t%.3f\t%d\t%s\n" % (rule, len(fl), len(fl) * 10000.0 / max(total, 1), len(pick), ",".join("%s=%d" % kv for kv in sorted(kinds.items()))))
            for i, f in enumerate(pick):
                s.write("%s#%d\t%s\t%s\t%s\t%s\t\n" % (rule, i + 1, rule, f[1], f[4].replace("\n", " "), f[5]))
    print("documents %d (failed %d), words %d, flags %d, rules with flags %d" % (len(files), failed, total, len(flags), len(by)))
    print("words by story kind:", ", ".join("%s %d" % (k, w) for k, (w, _) in sorted(words.items())))


def gate(a):
    verdict = {}
    for l in open(a.verdicts):
        p = l.strip().split("\t")
        if len(p) >= 2 and p[1] in ("right", "wrong"):
            verdict[p[0]] = p[1] == "right"
    total = sum(int(l.split("\t")[1]) for l in open(os.path.join(a.scratch, "words.tsv")))
    bad = 0
    print("rule\tflags\t/10k\tread\tright\tshare\tverdict")
    for l in open(os.path.join(a.scratch, "table.tsv")):
        rule, n, per, pick, _ = l.rstrip("\n").split("\t")
        read = [v for k, v in verdict.items() if k.rsplit("#", 1)[0] == rule]
        share = sum(read) / len(read) if read else 0.0
        ok = len(read) == int(pick) and share >= a.need
        if len(read) != int(pick):
            bad = 1
        print("%s\t%s\t%s\t%d\t%d\t%.0f%%\t%s" % (rule, n, per, len(read), sum(read), share * 100, "SHIP" if ok else "NARROW"))
    sys.exit(bad)


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("mode", choices=["run", "gate"])
    ap.add_argument("--tool"); ap.add_argument("--scratch", required=True); ap.add_argument("--sample", type=int, default=30)
    ap.add_argument("--seed", type=int, default=1); ap.add_argument("--jobs", type=int, default=8)
    ap.add_argument("--verdicts"); ap.add_argument("--need", type=float, default=0.9)
    a = ap.parse_args()
    run(a) if a.mode == "run" else gate(a)
