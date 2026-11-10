#!/usr/bin/env python3
"""Checks that, in the JeffOffice snap, Qt's FFmpeg media plugin (the KDE content snap's libffmpegmediaplugin.so) loads OUR minimal FFmpeg.

    python3 tools/snap_ffmpeg_audit.py SNAP_FILE CONTENT_SNAP_FILE
    python3 tools/snap_ffmpeg_audit.py --unpacked SNAP_ROOT CONTENT_ROOT      (folders already unpacked; the content snap's files are $SNAP/kf6)

(Unpacks both with unsquashfs. The content snap comes from `snap download kf6-core24`; the snap workflow does both.) It reads the FFmpeg libraries the
plugin needs (its NEEDED entries), and for each of the snap's apps resolves every one of them with the app's LD_LIBRARY_PATH order (meta/snap.yaml, with
the content snap's runtime folders where the variables stand), as the loader does. Exits 1 unless, for every app, every one resolves to the snap's own
usr/lib/jeffoffice-ffmpeg (a folder that holds nothing but FFmpeg's libraries), and what is in that folder passes the runtime probe: our minimal build
(tools/check_ffmpeg_components.py --probe).
"""
import glob
import os
import re
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
LIB = re.compile(r"/lib(avcodec|avformat|avutil|swresample|swscale)\.so\.\d+(\.\d+)*$")
FFNAME = re.compile(r"^lib(avcodec|avformat|avutil|swresample|swscale)\.so\.\d+$")
OURS = "usr/lib/jeffoffice-ffmpeg"
PLUGIN = "libffmpegmediaplugin.so"
ARCH = "x86_64-linux-gnu"


def listing(snap_file):
    """Every path inside a snap, from unsquashfs's listing (no unpacking)."""
    out = subprocess.run(["timeout", "300", "unsquashfs", "-q", "-ll", snap_file], capture_output=True, text=True, check=True).stdout
    paths = []
    for line in out.splitlines():
        m = re.search(r"\s(squashfs-root/\S+)", line)
        if m:
            paths.append(m.group(1)[len("squashfs-root/"):])
    return paths


def extract(snap_file, dest, wanted):
    """Unpacks only the named paths (and the folders above them)."""
    if wanted:
        subprocess.run(["timeout", "600", "unsquashfs", "-q", "-f", "-d", dest, snap_file] + sorted(set(wanted)), check=True)


def libs(root):
    found = []
    for path in glob.glob(os.path.join(root, "**", "lib*.so.*"), recursive=True):
        if LIB.search(path) and (os.path.isfile(path) or os.path.islink(path)):
            found.append(path)
    return sorted(found)


def needed_ffmpeg(plugin):
    """The FFmpeg libraries a plugin binds to: its NEEDED entries."""
    out = subprocess.run(["readelf", "-d", plugin], capture_output=True, text=True, check=True).stdout
    return sorted(n for n in re.findall(r"\(NEEDED\)\s+Shared library: \[([^\]]+)\]", out) if FFNAME.match(n))


def app_library_paths(snap_root):
    """Each app's LD_LIBRARY_PATH value, as meta/snap.yaml has it (the app's environment, else the snap's), by app name."""
    import yaml
    with open(os.path.join(snap_root, "meta/snap.yaml"), encoding="utf-8") as f:
        meta = yaml.safe_load(f)
    base = (meta.get("environment") or {}).get("LD_LIBRARY_PATH")
    return {name: ((app.get("environment") or {}).get("LD_LIBRARY_PATH") or base) for name, app in sorted((meta.get("apps") or {}).items())}


def runtime_folders():
    """What stands for ${SNAP_LIBRARY_PATH} and ${LD_LIBRARY_PATH} in a value: the content snap's runtime folders, which the snap's launcher adds (it only appends)."""
    return ["$SNAP/kf6/usr/lib/" + ARCH, "$SNAP/kf6/usr/lib", "$SNAP/kf6/lib/" + ARCH, "$SNAP/kf6/lib"]


def ordered(value):
    """The folders of an LD_LIBRARY_PATH value, in order, as $SNAP/... paths (a variable is the content snap's runtime folders)."""
    out = []
    # (the value has "${SNAP_LIBRARY_PATH}${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}", whose colon is not a separator)
    value = re.sub(r"\$\{(SNAP_LIBRARY_PATH|LD_LIBRARY_PATH)[^}]*\}|\$LD_LIBRARY_PATH", "@RUNTIME@", value.replace("${SNAP}", "$SNAP"))
    for part in value.split(":"):
        part = part.strip()
        if "@RUNTIME@" in part:
            out += runtime_folders()
            part = part.replace("@RUNTIME@", "").strip()
        if part:
            out.append(part)
    # the launcher appends the content snap's folders after all of those
    out += runtime_folders()
    seen = []
    for d in out:
        if d not in seen:
            seen.append(d)
    return seen


def to_disk(folder, snap_root, content_root):
    return folder.replace("$SNAP/kf6", content_root).replace("$SNAP", snap_root)


def resolution_problems(snap_root, content_root, needed, values, our_files):
    """For each app: every needed library must resolve to our folder. our_files: the paths in our folder of the snap (it holds only FFmpeg's libraries)."""
    problems = []
    ours = os.path.realpath(os.path.join(snap_root, OURS))
    stray = [p for p in our_files if not FFNAME.match(os.path.basename(p))]
    if stray:
        problems.append("%s holds more than FFmpeg's libraries: %s" % (OURS, ", ".join(stray)))
    for name in needed:
        if not os.path.lexists(os.path.join(snap_root, OURS, name)):
            problems.append("%s has no %s, which the plugin needs" % (OURS, name))
    for app, value in values.items():
        if not value:
            problems.append("app %s sets no LD_LIBRARY_PATH" % app)
            continue
        folders = ordered(value)
        print("  %s: %s" % (app, " : ".join(folders)))
        for name in needed:
            where = None
            for f in folders:
                d = to_disk(f, snap_root, content_root)
                if os.path.lexists(os.path.join(d, name)):
                    where = f
                    break
            real = os.path.realpath(os.path.join(to_disk(where, snap_root, content_root), name)) if where else None
            verdict = "NOT FOUND (the plugin does not load)" if not where else where
            print("    %-22s -> %s" % (name, verdict))
            if not where:
                problems.append("app %s: %s resolves nowhere on the snap's library path" % (app, name))
            elif os.path.dirname(real) != ours:
                problems.append("app %s: %s resolves to %s, not to %s" % (app, name, where, "$SNAP/" + OURS))
    return problems


def audit(snap_root, content_root, work, probe=True):
    plugins = glob.glob(os.path.join(content_root, "**", PLUGIN), recursive=True)
    if not plugins:
        return ["the content snap has no %s" % PLUGIN]
    plugin = plugins[0]
    print("the plugin: " + os.path.relpath(plugin, content_root))
    needed = needed_ffmpeg(plugin)
    print("FFmpeg libraries it needs: " + " ".join(needed))
    if not needed:
        return ["the plugin needs no FFmpeg libraries: nothing to audit, which cannot be right"]
    print("FFmpeg libraries in the content snap:")
    for p in libs(content_root):
        print("  " + os.path.relpath(p, content_root))
    print("FFmpeg libraries in the snap:")
    for p in libs(snap_root):
        print("  " + os.path.relpath(p, snap_root))
    our_dir = os.path.join(snap_root, OURS)
    our_files = sorted(os.listdir(our_dir)) if os.path.isdir(our_dir) else []
    print("library path of each app, and where each needed library resolves:")
    problems = resolution_problems(snap_root, content_root, needed, app_library_paths(snap_root), our_files)
    if probe and os.path.isdir(our_dir):
        out = os.path.join(work, "probe.txt")
        subprocess.run(["sh", os.path.join(HERE, "ffmpeg_probe.sh"), our_dir, out], check=True)
        verdict = subprocess.run([sys.executable, os.path.join(HERE, "check_ffmpeg_components.py"), "--probe", out], capture_output=True, text=True)
        print(verdict.stdout.strip())
        if verdict.returncode != 0:
            problems.append("what is in %s is not our minimal build" % OURS)
    return problems


def main(argv):
    if len(argv) == 3 and argv[0] == "--unpacked":
        snap_root, content_root = argv[1], argv[2]
    elif len(argv) == 2:
        snap_file, content_file = argv
        work = tempfile.mkdtemp()
        snap_root, content_root = os.path.join(work, "snap"), os.path.join(work, "content")
        # (only what is needed is unpacked: the FFmpeg libraries, the plugin, and the snap's metadata; the content snap is a gigabyte)
        for label, src, dst, extra in (("snap", snap_file, snap_root, True), ("content snap", content_file, content_root, False)):
            print("listing the %s ..." % label, flush=True)
            paths = listing(src)
            wanted = [p for p in paths if LIB.search("/" + p) or p.split("/")[-1] == PLUGIN]
            if extra:
                wanted += [p for p in paths if p == "meta/snap.yaml" or p.startswith(OURS + "/")]
            print("  %d paths, %d to unpack" % (len(paths), len(wanted)), flush=True)
            extract(src, dst, wanted)
    else:
        print(__doc__)
        return 2
    work = tempfile.mkdtemp()
    problems = audit(snap_root, content_root, work)
    for p in problems:
        print("snap FFmpeg audit: " + p)
    if not problems:
        print("snap FFmpeg audit OK: Qt's FFmpeg plugin loads our FFmpeg in every app")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
