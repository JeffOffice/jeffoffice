#!/usr/bin/env python3
"""Lists every FFmpeg library inside the JeffOffice snap and inside the KDE content snap it runs on, works out which copy the multimedia plugin will load
first (the snap's LD_LIBRARY_PATH order), and runs the runtime probe on that copy: it must be our minimal build (tools/check_ffmpeg_components.py --probe).

    python3 tools/snap_ffmpeg_audit.py SNAP_FILE CONTENT_SNAP_FILE

(Unpacks both with unsquashfs. The content snap comes from `snap download kf6-core24`; the snap workflow does both.) Prints the libraries of each,
the library paths the snap's apps set, the folder that wins, and the probe's verdict; exits 1 if the winning copy is not ours or any app can reach a
library of another build ahead of it.
"""
import glob
import os
import re
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
LIB = re.compile(r"/lib(avcodec|avformat|avutil|swresample|swscale)\.so\.(\d+)$")


def listing(snap_file):
    """Every path inside a snap, from unsquashfs's listing (no unpacking): the files that are FFmpeg libraries, and the whole list."""
    out = subprocess.run(["timeout", "300", "unsquashfs", "-q", "-ll", snap_file], capture_output=True, text=True, check=True).stdout
    paths = []
    for line in out.splitlines():
        m = re.search(r"\s(squashfs-root/\S+)", line)
        if m:
            paths.append(m.group(1)[len("squashfs-root/"):])
    return paths


def extract(snap_file, dest, wanted):
    """Unpacks only the named paths (and the folders above them)."""
    if not wanted:
        return
    subprocess.run(["timeout", "600", "unsquashfs", "-q", "-f", "-d", dest, snap_file] + sorted(set(wanted)), check=True)


def libs(root):
    found = []
    for path in glob.glob(os.path.join(root, "**", "lib*.so.*"), recursive=True):
        if os.path.isfile(path) and LIB.search(path):
            found.append(path)
    return sorted(found)


def library_paths(snap_root):
    """The LD_LIBRARY_PATH entries each app of the snap sets, in order, from meta/snap.yaml and the launcher scripts the extension adds."""
    texts = []
    with open(os.path.join(snap_root, "meta/snap.yaml"), encoding="utf-8") as f:
        texts.append(f.read())
    for script in glob.glob(os.path.join(snap_root, "snap/command-chain/*")) + glob.glob(os.path.join(snap_root, "bin/*launch*")):
        if os.path.isfile(script):
            try:
                with open(script, encoding="utf-8") as f:
                    texts.append(f.read())
            except UnicodeDecodeError:
                pass
    entries = []
    for text in texts:
        for m in re.finditer(r"LD_LIBRARY_PATH[=:]\s*\"?([^\n\"]+)", text):
            for part in m.group(1).split(":"):
                part = part.strip().strip("'\"")
                if part and part not in entries:
                    entries.append(part)
    return entries


def resolve(entry, snap_root, content_root):
    """A library path of the snap, as it is in the unpacked folders: $SNAP is the snap; the content snap is mounted under $SNAP/kf6 (or named by a variable)."""
    e = entry.replace("${SNAP}", "$SNAP")
    e = re.sub(r"\$SNAP_DESKTOP_RUNTIME|\$\{SNAP_DESKTOP_RUNTIME\}", "$SNAP/kf6", e)
    e = e.replace("$SNAP/kf6", content_root)
    e = e.replace("$SNAP", snap_root)
    e = re.sub(r"\$\{?SNAP_LIBRARY_PATH\}?|\$\{?LD_LIBRARY_PATH\}?|\$\{?SNAP_ARCH\}?", "", e)
    return e if e.startswith("/") and "$" not in e else None


def main(argv):
    if len(argv) != 2:
        print(__doc__)
        return 2
    snap_file, content_file = argv
    work = tempfile.mkdtemp()
    snap_root, content_root = os.path.join(work, "snap"), os.path.join(work, "content")
    # (only what is needed is unpacked: the FFmpeg libraries, the snap's metadata, and its launcher scripts; the content snap is a gigabyte)
    for label, src, dst, extra in (("snap", snap_file, snap_root, True), ("content snap", content_file, content_root, False)):
        print("listing the %s ..." % label, flush=True)
        paths = listing(src)
        wanted = [p for p in paths if LIB.search("/" + p)]
        if extra:
            wanted += [p for p in paths if p == "meta/snap.yaml" or p.startswith("snap/command-chain/") or "launch" in p.split("/")[-1]]
        print("  %d paths, %d to unpack" % (len(paths), len(wanted)), flush=True)
        extract(src, dst, wanted)
    problems = []
    print("FFmpeg libraries in the snap:")
    for p in libs(snap_root):
        print("  " + os.path.relpath(p, snap_root))
    print("FFmpeg libraries in the content snap:")
    for p in libs(content_root):
        print("  " + os.path.relpath(p, content_root))
    entries = library_paths(snap_root)
    print("library paths the snap's apps set, in order:")
    for e in entries:
        print("  " + e)
    ordered = []
    for e in entries:
        r = resolve(e, snap_root, content_root)
        if r and os.path.isdir(r) and r not in ordered:
            ordered.append(r)
    # The system's own folders come last, after everything the snap names (the loader's default path is the base snap's).
    winner = None
    for d in ordered:
        if glob.glob(os.path.join(d, "libavcodec.so.*")):
            winner = d
            break
    if winner is None:
        winner = os.path.join(snap_root, "usr/lib/x86_64-linux-gnu")
        print("no library path of the snap holds libavcodec; the snap's own usr/lib/x86_64-linux-gnu is probed")
    print("the folder the plugin loads FFmpeg from first: " + winner)
    ours = os.path.join(snap_root, "usr/lib/x86_64-linux-gnu")
    if os.path.realpath(winner) != os.path.realpath(ours):
        problems.append("the first copy of FFmpeg on the snap's library path is in %s, not in the snap's own folder" % winner)
    out = os.path.join(work, "probe.txt")
    subprocess.run(["sh", os.path.join(HERE, "ffmpeg_probe.sh"), winner, out], check=True)
    verdict = subprocess.run([sys.executable, os.path.join(HERE, "check_ffmpeg_components.py"), "--probe", out], capture_output=True, text=True)
    print(verdict.stdout.strip())
    if verdict.returncode != 0:
        problems.append("the copy the plugin loads first is not our minimal build")
    for p in problems:
        print("snap FFmpeg audit: " + p)
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
