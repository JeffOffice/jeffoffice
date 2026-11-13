#!/usr/bin/env python3
"""Writes resources/translate/models.json, the list of translation models the Translate pane can download.

    python3 tools/make_translate_models.py records.json > resources/translate/models.json

records.json is the answer of Mozilla's Remote Settings for the collection of Firefox's translation models ({"data": [records]}); each record is one file of one
model (fileType model, lex, vocab, or srcvocab and trgvocab) with the SHA-256 and size of the file as it is served. For every pair of English and one of our
languages, in both directions, the newest release version (no "a" suffix: those are tests) that has all of the pair's files is listed. The addresses are relative to
"baseUrl": the whole list moves to another server by changing that one line (and the files are the same, byte for byte, under the same names).
"""
import json
import re
import sys

LANGUAGES = "es fr de it pt nl pl ru uk id ar hi zh ja ko vi tr el".split()
BASE = "https://firefox-settings-attachments.cdn.mozilla.net/"
# (Mozilla names Simplified Chinese zh-Hans; here it is "zh")
THEIRS = {"zh": "zh-Hans"}
ROLES = {"model": "model", "lex": "shortlist", "vocab": "vocab", "srcvocab": "srcvocab", "trgvocab": "trgvocab"}


def version_key(v):
    return tuple(int(x) for x in v.split("."))


def main():
    records = json.load(open(sys.argv[1]))["data"]
    by = {}
    for r in records:
        if not re.fullmatch(r"\d+\.\d+", r["version"]):
            continue
        by.setdefault((r["fromLang"], r["toLang"], r["version"]), {})[r["fileType"]] = r
    pairs = {}
    for lang in LANGUAGES:
        for a, b in (("en", lang), (lang, "en")):
            ours = (THEIRS.get(a, a), THEIRS.get(b, b))
            versions = sorted((v for (f, t, v) in by if (f, t) == ours), key=version_key, reverse=True)
            for v in versions:
                have = by[ours + (v,)]
                if "model" in have and "lex" in have and ("vocab" in have or ("srcvocab" in have and "trgvocab" in have)):
                    files = {}
                    for ft, rec in sorted(have.items()):
                        if ft not in ROLES:
                            continue
                        att = rec["attachment"]
                        files[ROLES[ft]] = {"url": att["location"], "size": att["size"], "sha256": att["hash"]}
                    pairs["%s-%s" % (a, b)] = {"version": v, "files": files}
                    break
            else:
                sys.exit("no complete release of %s-%s" % (a, b))
    out = {
        "about": "Translation models for the Translate pane (tools/make_translate_models.py): Mozilla's Firefox Translations models, MPL-2.0. A file's address is relative to baseUrl.",
        "baseUrl": BASE,
        "pairs": pairs,
    }
    json.dump(out, sys.stdout, indent=1)
    print()


main()
