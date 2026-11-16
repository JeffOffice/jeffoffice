#!/usr/bin/env python3
"""Every language's Help topic must link to the same targets as the English one.

Usage: check_help_links.py [help folder]   (default: resources/help)

A translation changes the words of a link, never its target (a topic name or a
web address), so the sorted targets of a topic must equal the English topic's.
Prints each topic that differs, with the targets that are missing or extra, and
exits 1; a topic with no English original is reported too. Topics a language
has not translated yet are not an error, and neither is one whose English has
changed since the translator last wrote it (translations/machine/<language>.json):
that one waits for the translator, or for a person to update it by hand, and
translate.py lists it.
"""
import hashlib
import json
import os
import re
import sys

LINK = re.compile(r"\]\(([^)\s]+)\)")


def topics(folder):
    for root, _, files in os.walk(folder):
        for name in files:
            if name.endswith(".md"):
                yield os.path.relpath(os.path.join(root, name), folder)


def targets(path):
    with open(path, encoding="utf-8", newline="") as f:
        return sorted(LINK.findall(f.read()))


def stale_topics(lang):
    """Topics of `lang` (es_ES follows es) whose recorded English is no longer the English."""
    path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "translations", "machine", ("es" if lang == "es_ES" else lang) + ".json")
    try:
        with open(path, encoding="utf-8") as f:
            return json.load(f).get("help", {})
    except OSError:
        return {}


def main():
    root = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "resources", "help")
    help_dir = os.path.normpath(sys.argv[1] if len(sys.argv) > 1 else root)
    english = os.path.join(help_dir, "en")
    bad = []
    for lang in sorted(os.listdir(help_dir)):
        folder = os.path.join(help_dir, lang)
        if lang == "en" or not os.path.isdir(folder):
            continue
        record = stale_topics(lang)
        for rel in sorted(topics(folder)):
            original = os.path.join(english, rel)
            if not os.path.exists(original):
                bad.append(f"{lang}/{rel}: no English topic of that name")
                continue
            with open(original, encoding="utf-8", newline="") as f:
                current = hashlib.sha1(f.read().replace("\r\n", "\n").encode("utf-8")).hexdigest()[:16]
            rec = record.get(rel[:-3].replace(os.sep, "/"))
            if rec and rec["en"] != current:
                continue
            want, got = targets(original), targets(os.path.join(folder, rel))
            if want != got:
                missing = [t for t in want if t not in got] or ["-"]
                extra = [t for t in got if t not in want] or ["-"]
                bad.append(f"{lang}/{rel}: links differ from English (missing: {', '.join(missing)}; extra: {', '.join(extra)})")
    for line in bad:
        print(line)
    if bad:
        print(f"{len(bad)} Help topic(s) with different links")
        return 1
    print("Help links match English in every language")
    return 0


if __name__ == "__main__":
    sys.exit(main())
