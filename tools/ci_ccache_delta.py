#!/usr/bin/env python3
"""The part of a compiler cache that one CI job adds to the shared one (.github/workflows/build.yml, "Restore the compiler cache").

    tools/ci_ccache_delta.py snap   <cache dir> <snapshot file>
    tools/ci_ccache_delta.py pack   <cache dir> <snapshot file> <delta file>
    tools/ci_ccache_delta.py unpack <delta file> <cache dir>

snap lists the entries (path and size) the shared cache brought; pack, after the build, keeps the entries that are not in that list (or
changed size) and were made or used since the snapshot (ccache sets an entry's time when it hits), so a delta holds what this job needs
beyond the shared cache and drops what it stopped using; unpack puts a delta into a cache dir. Python's own tar module, because the
three systems' tar programs differ. ccache's counters, tmp, and config are left out."""
import json, os, sys, tarfile


def entries(root):
    for d, dirs, files in os.walk(root):
        dirs[:] = [x for x in dirs if x != 'tmp']
        for f in files:
            if f in ('stats', 'CACHEDIR.TAG', 'ccache.conf') or f.endswith('.lock'):
                continue
            p = os.path.join(d, f)
            yield os.path.relpath(p, root).replace(os.sep, '/'), p


def main(a):
    if len(a) == 4 and a[1] == 'snap':
        sizes = {r: os.path.getsize(p) for r, p in entries(a[2])}
        with open(a[3], 'w') as f:
            json.dump(sizes, f)
        print(f"base: {len(sizes)} entries, {sum(sizes.values()) / 1e6:.0f} MB")
    elif len(a) == 5 and a[1] == 'pack':
        root, snap, out = a[2:]
        since = os.path.getmtime(snap)
        base = json.load(open(snap))
        n = new = size = 0
        with tarfile.open(out, 'w') as t:
            for r, p in entries(root):
                s = os.path.getsize(p)
                if base.get(r) == s or os.path.getmtime(p) < since - 1:
                    continue
                t.add(p, arcname=r)
                n += 1; size += s; new += r not in base
        print(f"delta: {n} entries ({new} not in the base), {size / 1e6:.0f} MB")
    elif len(a) == 4 and a[1] == 'unpack':
        n = 0
        with tarfile.open(a[2]) as t:
            for m in t:
                if not m.isfile() or m.name.startswith(('/', '..')) or '/../' in m.name:
                    continue
                t.extract(m, a[3], **({'filter': 'data'} if hasattr(tarfile, 'data_filter') else {})); n += 1
        print(f"delta: {n} entries added")
    else:
        sys.exit(__doc__)


main(sys.argv)
