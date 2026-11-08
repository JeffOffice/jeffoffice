#!/usr/bin/env python3
"""Splits the Spanish regional dictionaries of the RLA-ES project (https://github.com/sbosio/rla-es, release 2.9)
into the files resources/dict/es keeps, so the nearly identical word lists are stored once.

    python3 tools/split_spanish_dicts.py SRC_DIR [OUT_DIR]

SRC_DIR holds the release's es_XX.dic and es_XX.aff for every region (unzip each es_XX.oxt from the release in a new
empty folder and copy the pairs into one folder). OUT_DIR defaults to resources/dict/es. It writes:

  es_common.dic   the words that every region's .dic has, with the same flags in each
  es_XX.dic       only the other words of region XX, loaded on top of es_common.dic (Hunspell::add_dic)
  es_XX.aff       one file for each distinct .aff (named for the first region that has it); the regions that share
                  one are listed on stdout, and libs/base/text/dictionaries.cpp names the file each region uses

Together the two .dic files hold exactly the lines of the original, so Hunspell accepts and rejects the same words
(tests/pub/tst_pub_find.cpp checks regional words; the load order does not matter). Next release: download, split,
and update the affix names in dictionaries.cpp from the printed list.
"""
import glob
import os
import shutil
import sys


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    src = sys.argv[1]
    out = sys.argv[2] if len(sys.argv) > 2 else os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "resources", "dict", "es")
    words = {}
    for f in sorted(glob.glob(os.path.join(src, "es_??.dic"))):
        with open(f, encoding="utf-8", newline="") as fh:
            lines = fh.read().split("\n")
        words[os.path.basename(f)[3:5]] = [l for l in lines[1:] if l]   # (the first line is the count)
    if not words:
        sys.exit("no es_XX.dic in " + src)
    # A word goes to the common list only when every region has exactly the same lines for it: Hunspell looks an affixed
    # form up in the first dictionary that has the word, so the same word with other flags in a region's own list would
    # lose forms. A word that differs anywhere keeps all its lines in each region's own list.
    def by_word(lines):
        d = {}
        for l in lines:
            d.setdefault(l.split("/")[0], []).append(l)
        return d
    per = {r: by_word(v) for r, v in words.items()}
    first = next(iter(per.values()))
    shared = {w for w, ls in first.items() if all(p.get(w) == ls for p in per.values())}
    order = [l for w in first if w in shared for l in first[w]]
    common = shared
    def write(path, lines):
        with open(path, "w", encoding="utf-8", newline="") as fh:
            fh.write("%d\n%s\n" % (len(lines), "\n".join(lines)))
    write(os.path.join(out, "es_common.dic"), order)
    for r, v in words.items():
        write(os.path.join(out, "es_%s.dic" % r), [l for l in v if l.split("/")[0] not in common])
    groups = {}
    for f in sorted(glob.glob(os.path.join(src, "es_??.aff"))):
        with open(f, "rb") as fh:
            groups.setdefault(fh.read(), []).append(os.path.basename(f)[3:5])
    for data, regions in groups.items():
        with open(os.path.join(out, "es_%s.aff" % regions[0]), "wb") as fh:
            fh.write(data)
        print("es_%s.aff: %s" % (regions[0], " ".join(regions)))
    print("common: %d lines" % len(order))
    return 0


if __name__ == "__main__":
    sys.exit(main())
