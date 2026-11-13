#!/usr/bin/env python3
"""Measures the grammar rules on real English text: flags per 10,000 words for each rule, and a random sample of flagged contexts to review.

Text sources (read only, never copied into the repo; the text goes to a scratch folder you name):
  - the practice halves of ~/doc-corpus (word/dev.txt, word2/dev2.txt, word3/dev3.txt): the paragraphs of each listed .docx. The held-out half (word/test.txt)
    is never opened: the script reads only the keys the dev lists name, and stops if one of them is in a test*.txt.
  - the .txt files inside the Govdocs1 zips of ~/jo-corpora/govdocs1 (hard-wrapped lines are joined into paragraphs).
Enron, EUSES and the other collections there are spreadsheets, which have no prose to check.

Usage: python3 -I tools/grammar/measure.py --scratch DIR --tool build/grammartool [--sample 40] [--govdocs-mb 150] [--seed 1] [--rule NAME]
Prints the table of rules; DIR/samples-docs.tsv and DIR/samples-govdocs.tsv hold the samples (rule<TAB>context).

For French, German, or Spanish: --language fr|de|es [--help-dir DIR] [--text-dir DIR]. The sources are then the program's own Help in that language (--help-dir, the markdown
folder of the language; the markup is stripped), the .txt files of --text-dir (hard-wrapped lines are joined into paragraphs), and the paragraphs of the Govdocs1 text files that
are in that language (found by their common words; this is only to choose the text to measure, the program never guesses a language).
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


COMMON = {
    "en": "the and of to is in that it for with was on as are be this by".split(),
    "fr": "le la les de des du et est un une que qui dans pour pas sur au aux avec ce cette il elle nous vous ils sont ne se".split(),
    "de": "der die das und ist nicht ein eine mit von zu den dem auf für ich sie es im auch sich wird sind".split(),
    "es": "el la los las de del y es un una que en por con para no se su al lo como más".split(),
}


def share(words, lang):
    if not words:
        return 0.0
    common = set(COMMON[lang])
    if len({w for w in words if w in common}) < 6:   # (a table of codes has a few of the short words, over and over)
        return 0.0
    return sum(1 for w in words if w in common) / len(words)


def is_language(text, lang):
    words = re.findall(r"[^\W\d_]+", text.lower())
    return len(words) >= 10 and share(words, lang) >= 0.25 and share(words, "en") <= 0.05


def markdown_paragraphs(path):
    text = open(path, encoding="utf-8").read()
    text = re.sub(r"<!--.*?-->", "", text, flags=re.S)
    text = re.sub(r"```.*?```", "", text, flags=re.S)
    for para in re.split(r"\n\s*\n", text):
        for line in para.split("\n"):
            line = line.strip()
            if not line or line.startswith("|") or re.fullmatch(r"[-=*_ ]+", line):
                continue
            line = re.sub(r"^#+\s*", "", line)
            line = re.sub(r"^([-*+]|\d+\.)\s+", "", line)
            line = re.sub(r"!?\[([^\]]*)\]\([^)]*\)", r"\1", line)
            line = re.sub(r"`[^`]*`", "x", line)
            line = line.replace("**", "").replace("__", "")
            if len(line) > 20:
                yield line


def write_help(out, folder):
    n = 0
    with open(out, "w", encoding="utf-8") as o:
        for dp, _, fs in os.walk(folder):
            for f in sorted(fs):
                if f.endswith(".md"):
                    n += 1
                    for line in markdown_paragraphs(os.path.join(dp, f)):
                        o.write(line + "\n")
    return n


def write_text_dir(out, folder):
    n = 0
    with open(out, "w", encoding="utf-8") as o:
        for f in sorted(glob.glob(os.path.join(folder, "*.txt"))):
            n += 1
            text = open(f, encoding="utf-8", errors="replace").read().replace("\r\n", "\n")
            for para in re.split(r"\n\s*\n", text):
                line = " ".join(l.strip() for l in para.split("\n") if l.strip())
                if len(line) > 20:
                    o.write(line + "\n")
    return n


def write_govdocs_language(out, lang, mb):
    limit = mb * 1000000
    size = n = 0
    with open(out, "w", encoding="utf-8") as o:
        for zp in sorted(glob.glob(os.path.join(GOVDOCS, "*.zip"))):
            with zipfile.ZipFile(zp) as z:
                for info in z.infolist():
                    if not info.filename.lower().endswith(".txt") or info.file_size > 3000000:
                        continue
                    text = z.read(info).decode("utf-8", "replace")
                    head = re.findall(r"[^\W\d_]+", text[:20000].lower())
                    if share(head, lang) < 0.15:
                        continue
                    n += 1
                    for para in re.split(r"\n\s*\n", text.replace("\r\n", "\n").replace("\r", "\n")):
                        line = " ".join(l.strip() for l in para.split("\n") if l.strip())
                        if len(line) > 20 and is_language(line, lang):
                            o.write(line + "\n")
                            size += len(line)
                    if size >= limit:
                        return n
    return n


def run(tool, files, sample, seed, rule, lang="en"):
    cmd = [tool, "--language", lang, "--sample", str(sample), "--seed", str(seed)] + (["--rule", rule] if rule else []) + files
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
    ap.add_argument("--language", default="en", choices=["en", "fr", "de", "es"])
    ap.add_argument("--help-dir", help="the program's Help in the language (markdown folder)")
    ap.add_argument("--text-dir", help="a folder of .txt files in the language")
    a = ap.parse_args()
    os.makedirs(a.scratch, exist_ok=True)
    if a.language != "en":
        sources = []
        if not a.reuse:
            if a.help_dir:
                print("help files:", write_help(os.path.join(a.scratch, "help.txt"), a.help_dir))
            if a.text_dir:
                print("text files:", write_text_dir(os.path.join(a.scratch, "text.txt"), a.text_dir))
            print("govdocs text files in the language:", write_govdocs_language(os.path.join(a.scratch, "govdocs.txt"), a.language, a.govdocs_mb))
        names = [n for n in ("help", "text", "govdocs") if os.path.exists(os.path.join(a.scratch, n + ".txt"))]
        res = {}
        for n in names:
            res[n] = run(a.tool, [os.path.join(a.scratch, n + ".txt")], a.sample, a.seed, a.rule, a.language)
            with open(os.path.join(a.scratch, "samples-%s.tsv" % n), "w", encoding="utf-8") as o:
                for r, smp in res[n][2]:
                    o.write(r + "\t" + smp + "\n")
        print("words: " + ", ".join("%s %d" % (n, res[n][0]) for n in names))
        print("%-26s" % "rule" + "".join(" %8s %9s" % (n, "/10k") for n in names))
        for r in sorted(set().union(*[set(res[n][1]) for n in names])):
            print("%-26s" % r + "".join(" %8d %9.2f" % res[n][1].get(r, (0, 0.0)) for n in names))
        return
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
