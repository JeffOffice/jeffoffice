#!/usr/bin/env python3
"""Checks that every library only uses the ones below it.

The shared libraries, lowest first, are libs/base, libs/draw, libs/text,
libs/office, libs/shell, libs/page, and libs/flow; each app in apps/ sits on top of them
all and never uses another app. Every quoted #include is resolved to the file
it names (each library's folder is an include root, so "core/color.h" is
libs/base/core/color.h), and an include of a file in a higher layer, or in
another app, is an error.

    python3 tools/check_layers.py

Prints each wrong include as file:line and exits 1 if there is any.
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LIBS = ["libs/base", "libs/draw", "libs/text", "libs/office", "libs/shell", "libs/page", "libs/flow"]
INCLUDE = re.compile(r'\s*#\s*include\s+"([^"]+)"')


def main():
    apps = sorted("apps/" + a for a in os.listdir(os.path.join(ROOT, "apps")) if os.path.isdir(os.path.join(ROOT, "apps", a)))
    roots = LIBS + apps
    rank = {r: i for i, r in enumerate(LIBS)}
    rank.update({a: len(LIBS) for a in apps})
    problems = 0
    for root in roots:
        for d, _, files in os.walk(os.path.join(ROOT, root)):
            for f in files:
                if not f.endswith((".cpp", ".h", ".inc")):
                    continue
                path = os.path.join(d, f)
                with open(path, encoding="utf-8") as src:
                    for n, line in enumerate(src, 1):
                        m = INCLUDE.match(line)
                        if not m:
                            continue
                        name = m.group(1)
                        if os.path.exists(os.path.join(d, name)):
                            continue  # beside the file, in the same library
                        owner = next((r for r in roots if os.path.exists(os.path.join(ROOT, r, name))), None)
                        if owner is None:
                            continue  # not ours (Qt, third_party, generated)
                        if rank[owner] > rank[root] or (owner in apps and owner != root):
                            print(f"{os.path.relpath(path, ROOT)}:{n}: {root} includes {name} from {owner}")
                            problems += 1
    print(f"{problems} wrong includes" if problems else "layers OK")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
