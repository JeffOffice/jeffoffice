#!/usr/bin/env python3
"""Checks that a package carries everything JeffDeck's FFmpeg media plugin needs to load, so that a missing library fails CI, not a person's first movie.

For the multimedia plugin and each FFmpeg library of the package, lists the libraries it needs and fails if one is neither in the package nor the system's:

    python3 tools/check_bundle_deps.py windows PACKAGE_DIR    (the folder of the program: the plugin is in multimedia/, the DLLs beside the program; reads imports with objdump)
    python3 tools/check_bundle_deps.py mac APP_BUNDLE         (JeffDeck.app: the plugin in Contents/PlugIns/multimedia, libraries in Contents/Frameworks; reads them with otool)
    python3 tools/check_bundle_deps.py windows-imports PLUGIN.dll DLLDIR   every function the plugin imports from an FFmpeg DLL is exported by that DLL in DLLDIR
                                                              (Windows binds an import to a DLL by name; one missing, and the plugin does not load, and Qt 6.11 goes on to another backend)
    python3 tools/check_bundle_deps.py windows-closure PLUGIN.dll DIR...  every DLL the plugin needs, and those need, in turn, is in one of the folders or is the system's

(Linux packages are checked with ldd where they are made, .github/workflows/build.yml.)
"""
import glob
import os
import re
import subprocess
import sys


def run(*cmd):
    return subprocess.run(cmd, capture_output=True, text=True, check=False).stdout


def windows(package):
    plugin = os.path.join(package, "multimedia", "ffmpegmediaplugin.dll")
    files = [plugin] + sorted(glob.glob(os.path.join(package, "av*-*.dll")) + glob.glob(os.path.join(package, "sw*-*.dll")))
    if not os.path.exists(plugin):
        return ["the multimedia plugin is not in the package: %s" % plugin], 0
    system = os.path.join(os.environ.get("SystemRoot", r"C:\Windows"), "System32")
    here = {os.path.basename(p).lower() for p in glob.glob(os.path.join(package, "**", "*.dll"), recursive=True)}
    problems, seen = [], 0
    for f in files:
        for name in re.findall(r"DLL Name: (\S+)", run("objdump", "-p", f)):
            seen += 1
            low = name.lower()
            if low in here or os.path.exists(os.path.join(system, name)) or low.startswith(("api-ms-win-", "ext-ms-win-")):
                continue
            problems.append("%s needs %s, which is neither in the package nor a system library" % (os.path.relpath(f, package), name))
    return problems, seen


def mac(app):
    plugin = os.path.join(app, "Contents/PlugIns/multimedia/libffmpegmediaplugin.dylib")
    frameworks = os.path.join(app, "Contents/Frameworks")
    if not os.path.exists(plugin):
        return ["the multimedia plugin is not in the bundle: %s" % plugin], 0
    files = [plugin] + sorted(glob.glob(os.path.join(frameworks, "libav*.dylib")) + glob.glob(os.path.join(frameworks, "libsw*.dylib")))
    problems, seen = [], 0
    for f in files:
        if os.path.islink(f):
            continue
        for line in run("otool", "-L", f).splitlines()[1:]:
            m = re.match(r"\s+(\S+) \(", line)
            if not m:
                continue
            ref = m.group(1)
            seen += 1
            if ref.startswith(("/usr/lib/", "/System/")):
                continue
            if ref.startswith("@rpath/"):
                rest = ref[len("@rpath/"):]
                if os.path.exists(os.path.join(frameworks, rest)) or os.path.exists(os.path.join(app, "Contents/PlugIns", rest)):
                    continue
            elif ref.startswith("@loader_path/") and os.path.exists(os.path.normpath(os.path.join(os.path.dirname(f), ref[len("@loader_path/"):]))):
                continue
            problems.append("%s needs %s, which is not in the bundle" % (os.path.relpath(f, app), ref))
    return problems, seen


def pe_imports(path):
    """{dll name: [function names]} from objdump -p."""
    out, current = {}, None
    for line in run("objdump", "-p", path).splitlines():
        m = re.match(r"\s*DLL Name: (\S+)", line)
        if m:
            current = m.group(1)
            out.setdefault(current, [])
            continue
        m = re.match(r"\s+[0-9a-fA-F]+\s+(\d+)\s+(\S+)\s*$", line)
        if m and current:
            out[current].append(m.group(2))
    return out


def pe_exports(path):
    names = set()
    for line in run("objdump", "-p", path).splitlines():
        m = re.match(r"\s+\[\s*\d+\]\s+(\S+)\s*$", line)
        if m:
            names.add(m.group(1))
    return names


def windows_imports(plugin, dlldir):
    problems, total = [], 0
    imports = pe_imports(plugin)
    for dll, names in sorted(imports.items()):
        if not re.match(r"(av|sw)[a-z]*-\d+\.dll$", dll, re.I):
            continue
        path = os.path.join(dlldir, dll)
        have = pe_exports(path) if os.path.exists(path) else None
        print("bind check: the plugin imports %d names from %s (%s)" % (len(names), dll, "found, exports %d" % len(have) if have is not None else "NOT FOUND in " + dlldir))
        total += len(names)
        if have is None:
            problems.append("%s is not in %s" % (dll, dlldir))
            continue
        missing = [n for n in names if n not in have]
        if missing:
            problems.append("%s lacks %d names the plugin imports from it: %s" % (dll, len(missing), " ".join(missing[:12]) + (" ..." if len(missing) > 12 else "")))
    return problems, total


def windows_closure(plugin, dirs):
    system = os.path.join(os.environ.get("SystemRoot", r"C:\Windows"), "System32")
    problems, seen, todo, done, exports_of = [], 0, [plugin], set(), {}
    while todo:
        f = todo.pop()
        if f.lower() in done:
            continue
        done.add(f.lower())
        imports = pe_imports(f)
        for name in imports:
            low = name.lower()
            if low.startswith(("api-ms-win-", "ext-ms-win-")):
                continue
            found = next((os.path.join(d, name) for d in dirs if os.path.exists(os.path.join(d, name))), None)
            if found:
                seen += 1
                todo.append(found)
            elif os.path.exists(os.path.join(system, name)):
                seen += 1
                found = os.path.join(system, name)
            if found:
                # (and every function this one imports from that DLL is one it exports: "The specified procedure could not be found" is this)
                key = found.lower()
                if key not in exports_of:
                    exports_of[key] = pe_exports(found)
                have = exports_of[key]
                if have:
                    gone = [n for n in imports[name] if n not in have]
                    if gone:
                        problems.append("%s imports %d names that %s does not export: %s" % (os.path.basename(f), len(gone), name, " ".join(gone[:10])))
                continue
            problems.append("%s needs %s, which is in none of %s and is not a system library" % (os.path.basename(f), name, ", ".join(dirs)))
    return problems, seen


def main(argv):
    if argv and argv[0] == "windows-closure" and len(argv) >= 3:
        problems, seen = windows_closure(argv[1], argv[2:])
    elif argv and argv[0] == "windows-imports" and len(argv) == 3:
        problems, seen = windows_imports(argv[1], argv[2])
    elif len(argv) == 2 and argv[0] in ("windows", "mac"):
        problems, seen = (windows if argv[0] == "windows" else mac)(argv[1])
    else:
        print(__doc__)
        return 2
    for p in problems:
        print("bundle check: " + p)
    if problems:
        return 1
    print("bundle check: the %d libraries the FFmpeg plugin and its libraries need are all in the package or the system's" % seen)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
