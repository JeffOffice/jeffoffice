#!/usr/bin/env python3
"""Checks, on a Mac, that every FFmpeg name Qt's multimedia plugin imports resolves where the plugin looks for it.

A Mac's linker is two-level: the plugin records, for each name it imports, the one library that name comes from (nm -m
shows it: "(undefined) external _av_malloc (from libavformat.61)"), and dyld, when it loads the plugin, looks for the name
in that library and in the libraries it re-exports (LC_REEXPORT_DYLIB), and nowhere else. A name that is missing stops the
plugin from loading at all ("Symbol not found"), and Qt then has no FFmpeg backend. So for each import of an FFmpeg library
this finds the file the plugin means in the library folder, follows its re-exports, and fails if the name is not defined.

    python3 tools/check_macho_binds.py PLUGIN.dylib LIBDIR
"""
import glob
import os
import re
import subprocess
import sys

IMPORT = re.compile(r"\(undefined\) external (\S+) \(from ([^)]+)\)")
REEXPORT = re.compile(r"cmd LC_REEXPORT_DYLIB.*?name (\S+) \(offset", re.S)
FFMPEG = re.compile(r"^lib(av|sw)")


def run(*cmd):
    return subprocess.run(cmd, capture_output=True, text=True, check=False).stdout


def library_file(libdir, name):
    """The file in libdir that a "from" name (libavformat.61, libswresample) or an install name's last part means."""
    base = name[:-6] if name.endswith(".dylib") else name
    for candidate in (base + ".dylib",) + tuple(sorted(glob.glob(os.path.join(libdir, base + ".*dylib")))):
        path = candidate if os.path.isabs(candidate) else os.path.join(libdir, candidate)
        if os.path.exists(path):
            return os.path.realpath(path)
    return None


def defined(path, arch, cache):
    key = (path, arch)
    if key not in cache:
        names = set()
        for line in run("nm", "-gU", "-arch", arch, path).splitlines():
            parts = line.split()
            if len(parts) >= 2 and parts[-1].startswith("_"):
                names.add(parts[-1])
        cache[key] = names
    return cache[key]


def reexports(path, libdir):
    out = []
    for block in run("otool", "-l", path).split("Load command")[1:]:
        m = REEXPORT.search(block)
        if m:
            found = library_file(libdir, os.path.basename(m.group(1)))
            if found:
                out.append(found)
    return out


def reachable(path, arch, libdir, cache, seen=None):
    """Every name defined by the library and by what it re-exports, however deep."""
    seen = seen if seen is not None else set()
    if path in seen:
        return set()
    seen.add(path)
    names = set(defined(path, arch, cache))
    for other in reexports(path, libdir):
        names |= reachable(other, arch, libdir, cache, seen)
    return names


def check(plugin, libdir):
    problems, count = [], 0
    probe = library_file(libdir, "libavformat.61") or ""
    archs = run("lipo", "-archs", probe).split() if probe else []
    if not archs:
        return ["no libavformat in %s" % libdir], 0
    cache = {}
    for arch in archs:
        for line in run("nm", "-m", "-arch", arch, plugin).splitlines():
            m = IMPORT.search(line)
            if not m or not FFMPEG.match(m.group(2)):
                continue
            name, source = m.group(1), m.group(2)
            count += 1
            path = library_file(libdir, source)
            if not path:
                problems.append("%s: %s is bound to %s, which is not in %s" % (arch, name, source, libdir))
            elif name not in reachable(path, arch, libdir, cache):
                problems.append("%s: %s is bound to %s, which does not define or re-export it" % (arch, name, source))
    return problems, count


def main(argv):
    if len(argv) != 2:
        print(__doc__)
        return 2
    problems, count = check(argv[0], argv[1])
    for p in problems:
        print("bind check: " + p)
    if problems:
        return 1
    print("bind check: %d FFmpeg imports of the plugin all resolve" % count)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
