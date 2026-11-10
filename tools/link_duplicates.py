#!/usr/bin/env python3
"""Replaces each duplicate file under the given folders with a hard link to the first copy (same size, mode, and SHA-256), so a disk image made
from them stores the bytes once. Every folder still holds every file: copying one folder alone copies complete files. Symlinks and folders
are left alone, and the bytes do not change, so code signatures stay valid.

  tools/link_duplicates.py FOLDER...
"""
import hashlib
import os
import sys


def main(roots):
    by_size = {}
    for root in roots:
        for here, _dirs, names in os.walk(root):
            for name in names:
                path = os.path.join(here, name)
                if os.path.islink(path) or not os.path.isfile(path):
                    continue
                st = os.lstat(path)
                if st.st_size > 0:
                    by_size.setdefault((st.st_size, st.st_mode), []).append(path)
    first = {}
    linked = saved = 0
    for (size, mode), paths in by_size.items():
        if len(paths) < 2:
            continue
        for path in paths:
            with open(path, "rb") as f:
                digest = hashlib.sha256(f.read()).hexdigest()
            key = (size, mode, digest)
            keep = first.setdefault(key, path)
            if keep == path or os.path.samefile(keep, path):
                continue
            tmp = path + ".linktmp"
            os.link(keep, tmp)
            os.replace(tmp, path)
            linked += 1
            saved += size
    print("linked %d duplicate files, %.1f MB saved" % (linked, saved / 1048576))


if __name__ == "__main__":
    main(sys.argv[1:] or sys.exit("usage: link_duplicates.py FOLDER..."))
