#!/usr/bin/env python3
"""Converts TeX hyphenation patterns (the \\patterns{} and \\hyphenation{} blocks of a hyph-utf8 file, such as
hyph-ru.tex) into the libhyphen .dic format that libs/base/text/hyphenation.cpp loads.

    python3 tools/dict/tex_to_hyph.py hyph-ru.tex resources/dict/ru_RU/hyph_ru_RU.dic [LEFTMIN RIGHTMIN]

The minimums default to the file's own "hyphenmins" header (left, right), else 2 and 2.

The loader (like libhyphen) follows one chain of pattern starts through the word and applies only the longest start that
ends at each letter, so the output is written the way libhyphen's substrings.pl writes it: one line for every start of
every pattern, holding the digits of all the patterns that end the same way (those that are tails of that start).

The patterns are used as TeX uses them (the highest digit at each gap wins, odd breaks). Two things differ:
  - Patterns containing a hyphen ("а8б8-") only keep TeX from breaking next to a hyphen; the loader already splits
    words at hyphens and never breaks next to one, so they are dropped.
  - An exception word ("по-мру", "помни") becomes one pattern for the whole word, with 9 at its break points and 8 at
    every other gap, which beats any pattern digit but a stray 9 (checked below; a word that disagrees is reported).
The output is sorted, so the same input always gives the same bytes.
"""
import re
import sys


def parse_pattern(p):
    letters, digits = "", [0]
    for c in p:
        if c.isdigit():
            digits[-1] = int(c)
        else:
            letters += c
            digits.append(0)
    return letters, digits


def block(text, name):
    m = re.search(r"\\" + name + r"\s*\{(.*?)\}", text, re.S)
    if not m:
        return []
    lines = []
    for line in m.group(1).splitlines():
        line = line.split("%")[0].strip()
        lines += line.split()
    return lines


def convert(text):
    pats = {}
    for p in block(text, "patterns"):
        if "-" in p:
            continue
        letters, digits = parse_pattern(p)
        old = pats.get(letters)
        pats[letters] = digits if old is None else [max(a, b) for a, b in zip(old, digits)]
    exceptions = {}
    for w in block(text, "hyphenation"):
        letters = w.replace("-", "")
        d, j = [0] * (len(letters) + 3), 0
        for c in w:
            if c == "-":
                d[j + 1] = 9
            else:
                j += 1
        for k in range(2, len(letters) + 1):   # the gaps between letters: 8 where TeX has no hyphen
            d[k] = d[k] or 8
        exceptions["." + letters + "."] = d
    # every start of every pattern is a state; its digits merge the patterns that are tails of it
    states = set()
    for s in list(pats) + list(exceptions):
        for n in range(1, len(s) + 1):
            states.add(s[:n])
    out = {}
    for s in states:
        d = [0] * (len(s) + 1)
        for o in range(len(s)):
            for e in range(o + 1, len(s) + 1):
                v = pats.get(s[o:e])
                if v and e == len(s):
                    for k, x in enumerate(v):
                        d[o + k] = max(d[o + k], x)
        out[s] = d
    for s, d in exceptions.items():
        out[s] = d[:len(s) + 1]
    return out, pats, exceptions


def apply(out, word):
    w = "." + word + "."
    maxlen = max(map(len, out))
    levels = [0] * (len(w) + 1)
    for end in range(1, len(w) + 1):
        for ln in range(min(end, maxlen), 0, -1):
            v = out.get(w[end - ln:end])
            if v is not None:
                for k, x in enumerate(v):
                    levels[end - ln + k] = max(levels[end - ln + k], x)
                break
    return levels


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    text = open(sys.argv[1], encoding="utf-8").read()
    m = re.search(r"hyphenmins:\s*typesetting:\s*left:\s*(\d+)\s*right:\s*(\d+)", re.sub(r"%", "", text))
    left, right = (m.group(1), m.group(2)) if m else ("2", "2")
    if len(sys.argv) > 4:
        left, right = sys.argv[3], sys.argv[4]
    out, pats, exceptions = convert(text)
    bad = 0
    for s, d in exceptions.items():
        lv = apply(out, s[1:-1])
        if [x % 2 for x in lv[2:len(s) - 1]] != [x % 2 for x in d[2:len(s) - 1]]:
            bad += 1
            print("exception not reproduced:", s, file=sys.stderr)
    lines = ["UTF-8", "LEFTHYPHENMIN " + left, "RIGHTHYPHENMIN " + right]
    for s in sorted(out):
        d = out[s]
        line = "".join((str(d[i]) if d[i] else "") + (s[i] if i < len(s) else "") for i in range(len(s) + 1))
        lines.append(line)
    with open(sys.argv[2], "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines) + "\n")
    print("%d patterns, %d exceptions (%d not reproduced), %d lines" % (len(pats), len(exceptions), bad, len(lines)))


if __name__ == "__main__":
    main()
