#!/usr/bin/env python3
"""Checks that the program still compiles with the translator left out (the CMake option JO_TRANSLATE off), without a second build tree.

    python3 tools/check_translate_off.py [build folder]      (default: build)

The files that differ between the two are the ones that say `#ifdef JO_TRANSLATE_ENGINE`. Each is compiled again, syntax only, with the compile command of the build
folder and that one definition taken away. Prints one line and exits 1 if one fails.
"""
import os
import re
import shlex
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def main():
    build = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, "build"))
    files = []
    for d, _, names in os.walk(os.path.join(ROOT, "libs")):
        for n in names:
            if n.endswith(".cpp"):
                path = os.path.join(d, n)
                with open(path, encoding="utf-8") as f:
                    if "JO_TRANSLATE_ENGINE" in f.read():
                        files.append(os.path.relpath(path, ROOT))
    if not files:
        print("translate off: no file to check")
        return 1
    bad = 0
    for rel in files:
        target = "CMakeFiles/jo_translate.dir/" + rel + ".o"
        out = subprocess.run(["ninja", "-C", build, "-t", "commands", target], capture_output=True, text=True).stdout.strip().splitlines()
        if not out:
            print("translate off: no compile command for " + rel)
            bad += 1
            continue
        args = shlex.split(out[-1])
        # (without the compiler cache in front, the object and dependency outputs, and the definition; the precompiled header is not used)
        if args and os.path.basename(args[0]).startswith("ccache"):
            args = args[1:]
        keep, skip = [], 0
        for a in args:
            if skip:
                skip -= 1
            elif a in ("-o", "-MF", "-MT"):
                skip = 1
            elif a in ("-MD", "-MQ") or a == "-DJO_TRANSLATE_ENGINE=1" or a.startswith("-include") or a.endswith("cmake_pch.hxx") or "cmake_pch" in a:
                continue
            else:
                keep.append(a)
        r = subprocess.run(keep + ["-fsyntax-only"], cwd=build, capture_output=True, text=True)
        if r.returncode != 0:
            print("translate off: " + rel + " does not compile:\n" + "\n".join(r.stderr.splitlines()[:8]))
            bad += 1
    print("translate off: %d file(s), %d problem(s)" % (len(files), bad))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
