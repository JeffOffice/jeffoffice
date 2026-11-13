#!/usr/bin/env python3
"""Measures the grammar rules on real English text: flags per 10,000 words for each rule, and a random sample of flagged contexts to review.

Text sources (read only, never copied into the repo; the text goes to a scratch folder you name):
  - the practice halves of ~/doc-corpus (word/dev.txt, word2/dev2.txt, word3/dev3.txt): the paragraphs of each listed .docx. The held-out half (word/test.txt)
    is never opened: the script reads only the keys the dev lists name, and stops if one of them is in a test*.txt.
  - the .txt files inside the Govdocs1 zips of ~/jo-corpora/govdocs1 (hard-wrapped lines are joined into paragraphs).
Enron, EUSES and the other collections there are spreadsheets, which have no prose to check.

Usage: python3 -I tools/grammar/measure.py --scratch DIR --tool build/grammartool [--sample 40] [--govdocs-mb 150] [--seed 1] [--rule NAME]
Prints the table of rules; DIR/samples-docs.tsv and DIR/samples-govdocs.tsv hold the samples (rule<TAB>context).
"""
import argparse, glob, os, re, subprocess, sys, zipfile

CORPUS = os.path.expanduser("~/doc-corpus")
GOVDOCS = os.path.expanduser("~/jo-corpora/govdocs1")


def keys(listfile):
    return [l.split()[0] for l in open(listfile) if l.strip()]


def held_out():
    out = set()
    for f in glob.glob(os.path.join(CORPUS, "*", "test*.txt")):
        out |= set(keys(f))
    return out


def docx_paragraphs(path):
    with zipfile.ZipFile(path) as z:
        xml = z.read("word/document.xml").decode("utf-8", "replace")
    for p in re.findall(r"<w:p[ >].*?</w:p>", xml, re.S):
        parts = re.findall(r"<w:t(?: [^>]*)?>([^<]*)</w:t>|(<w:tab/>)|(<w:(?:br|cr)\b[^>]*/>)", p)
        text = "".join(t if t else ("\t" if tab else "\n") for t, tab, br in parts)
        text = (text.replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", '"').replace("&apos;", "'"))
        for line in text.split("\n"):
            if line.strip():
                yield line


def write_docs(out):
    held = held_out()
    n = 0
    with open(out, "w", encoding="utf-8") as o:
        for d, lst in (("word", "dev.txt"), ("word2", "dev2.txt"), ("word3", "dev3.txt")):
            for k in keys(os.path.join(CORPUS, d, lst)):
                if k in held and d == "word":
                    sys.exit("a dev key is in a held-out list: " + k)
                path = os.path.join(CORPUS, d, "in", k + ".docx")
                try:
                    for p in docx_paragraphs(path):
                        o.write(p.replace("\r", " ") + "\n")
                    n += 1
                except Exception as e:
                    print("skipped", k, e, file=sys.stderr)
    return n


def write_govdocs(out, mb):
    limit = mb * 1000000
    size = n = 0
    with open(out, "w", encoding="utf-8") as o:
        for zp in sorted(glob.glob(os.path.join(GOVDOCS, "*.zip"))):
            with zipfile.ZipFile(zp) as z:
                for info in z.infolist():
                    if not info.filename.lower().endswith(".txt") or info.file_size > 3000000:
                        continue
                    text = z.read(info).decode("utf-8", "replace")
                    n += 1
                    for para in re.split(r"\n\s*\n", text.replace("\r\n", "\n").replace("\r", "\n")):
                        line = " ".join(l.strip() for l in para.split("\n") if l.strip())
                        if len(line) > 20:
                            o.write(line + "\n")
                            size += len(line)
                    if size >= limit:
                        return n
    return n


def run(tool, files, sample, seed, rule):
    cmd = [tool, "--sample", str(sample), "--seed", str(seed)] + (["--rule", rule] if rule else []) + files
    r = subprocess.run(cmd, capture_output=True, text=True, env=dict(os.environ, QT_QPA_PLATFORM="offscreen"))
    words, rules, samples = 0, {}, []
    for l in r.stdout.splitlines():
        f = l.split("\t")
        if f[0] == "words":
            words = int(f[1])
        elif f[0] == "rule":
            rules[f[1]] = (int(f[2]), float(f[3]))
        elif f[0] == "sample":
            samples.append((f[1], f[2]))
    return words, rules, samples


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--scratch", required=True)
    ap.add_argument("--tool", required=True)
    ap.add_argument("--sample", type=int, default=40)
    ap.add_argument("--govdocs-mb", type=int, default=150)
    ap.add_argument("--seed", type=int, default=1)
    ap.add_argument("--rule")
    ap.add_argument("--reuse", action="store_true", help="use the text files already in the scratch folder")
    a = ap.parse_args()
    os.makedirs(a.scratch, exist_ok=True)
    docs, gov = os.path.join(a.scratch, "docs.txt"), os.path.join(a.scratch, "govdocs.txt")
    if not a.reuse:
        print("documents:", write_docs(docs), "govdocs text files:", write_govdocs(gov, a.govdocs_mb))
    res = {}
    for name, f in (("docs", docs), ("govdocs", gov)):
        res[name] = run(a.tool, [f], a.sample, a.seed, a.rule)
        with open(os.path.join(a.scratch, "samples-%s.tsv" % name), "w", encoding="utf-8") as o:
            for r, s in res[name][2]:
                o.write(r + "\t" + s + "\n")
    print("words: docs %d, govdocs %d" % (res["docs"][0], res["govdocs"][0]))
    print("%-20s %8s %9s %8s %9s" % ("rule", "docs", "/10k", "govdocs", "/10k"))
    for r in sorted(set(res["docs"][1]) | set(res["govdocs"][1])):
        d, g = res["docs"][1].get(r, (0, 0.0)), res["govdocs"][1].get(r, (0, 0.0))
        print("%-20s %8d %9.2f %8d %9.2f" % (r, d[0], d[1], g[0], g[1]))


main()
