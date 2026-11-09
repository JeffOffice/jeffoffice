#!/usr/bin/env python3
"""Keeps the FFmpeg JeffDeck ships down to what a movie or sound in a presentation needs.

tools/build_ffmpeg.sh builds FFmpeg 7.1.5 with only the demuxers, decoders, parsers, and protocol it lists.
This check fails when:
  - the script configures something it must not (a GPL or nonfree build, a network, a muxer, an encoder, a device,
    hardware acceleration, a protocol besides the file one, or a component on the banned list below);
  - a banned component (the ones named in the flaws that affect Qt's full build: the VobSub demuxer, NVDEC, the
    S/PDIF muxer, the RTP, RTSP, and SDP demuxers, and the ADX decoder, and kin) is in third_party/ffmpeg-components.txt;
  - that file does not list exactly the components the script enables (and the few bitstream filters a decoder pulls in).
    python3 tools/check_ffmpeg_components.py                        checks every third_party/ffmpeg-components*.txt
    python3 tools/check_ffmpeg_components.py --built PREFIX [--components FILE]   also compares PREFIX/share/ffmpeg-components.txt
                                                                    (what a build really compiled in) with the file
    python3 tools/check_ffmpeg_components.py --built PREFIX --write  replaces the file with it
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SCRIPT = "tools/build_ffmpeg.sh"
LIST = "third_party/ffmpeg-components.txt"

# Components that are never compiled in. Kinds: demuxer, decoder, muxer, protocol, parser, bsf, encoder.
BANNED = {
    "demuxer": {"vobsub", "rtp", "rtsp", "sdp", "adx", "hls", "dash", "webm_dash_manifest", "concat", "image2", "image2pipe", "lavfi",
                "rpl", "sap", "mpegtsraw", "live_flv", "tty", "ffmetadata", "srt", "mpl2", "ass"},
    "decoder": {"adpcm_adx", "libdav1d", "libx264", "libx265", "libopenh264"},
    "protocol": {"rtp", "rtsp", "sdp", "tcp", "udp", "udplite", "http", "https", "tls", "hls", "concat", "concatf", "subfile", "pipe",
                 "fd", "data", "crypto", "async", "cache", "ftp", "gopher", "md5", "prompeg", "srtp", "rtmp", "unix"},
}
# What the script's flags may not contain at all.
BANNED_FLAGS = [r"--enable-gpl", r"--enable-nonfree", r"--enable-version3", r"--enable-network", r"--enable-programs",
                r"--enable-autodetect", r"--enable-hwaccels?", r"--enable-nvdec", r"--enable-cuvid", r"--enable-nvenc",
                r"--enable-vaapi", r"--enable-vdpau", r"--enable-libx26", r"--enable-libdav1d", r"--enable-openssl",
                r"--enable-gnutls", r"--enable-muxer", r"--enable-encoder", r"--enable-indev", r"--enable-outdev",
                r"--enable-avdevice", r"--enable-avfilter", r"--enable-filter", r"--enable-bsf", r"--enable-static",
                r"--enable-zlib", r"--enable-bzlib", r"--enable-lzma", r"--enable-iconv", r"--enable-libx", r"--enable-lib[a-z0-9]+"]
# What it must contain.
REQUIRED_FLAGS = ["--disable-everything", "--disable-autodetect", "--disable-network", "--disable-programs", "--disable-doc",
                  "--disable-avdevice", "--disable-hwaccels", "--enable-shared", "--disable-static"]
# The bitstream filters a decoder may pull in by itself (configure selects them).
ALLOWED_BSF = {"vp9_superframe_split"}


def read(path):
    with open(os.path.join(ROOT, path) if not os.path.isabs(path) else path, encoding="utf-8") as f:
        return f.read()


def script_lists(text):
    """The allow-lists the script holds: VAR="\\nname\\nname\\n" blocks."""
    out = {}
    for var, kind in (("DEMUXERS", "demuxer"), ("DECODERS", "decoder"), ("PARSERS", "parser"), ("PROTOCOLS", "protocol")):
        m = re.search(r'^%s="([^"]*)"' % var, text, re.M)
        out[kind] = set(m.group(1).split()) if m else None
    return out


def list_file(text):
    """(kind, name) pairs and the counts of a components file."""
    pairs, counts = set(), {}
    for line in text.splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split()
        if parts[0] == "count" and len(parts) == 3:
            counts[parts[1]] = int(parts[2])
        elif len(parts) == 2:
            pairs.add((parts[0], parts[1]))
        else:
            raise ValueError("not a line of the components file: " + line)
    return pairs, counts


def check(script_path, list_path, built=None, write=False):
    problems = []
    script = read(script_path)
    lists = script_lists(script)
    for kind, names in lists.items():
        if names is None:
            problems.append("%s has no list of %ss" % (script_path, kind))
    if problems:
        return problems
    # The flags, in the script's own configure call (its comments don't count).
    code = "\n".join(l for l in script.splitlines() if not l.lstrip().startswith("#"))
    for flag in REQUIRED_FLAGS:
        if flag not in code:
            problems.append("%s no longer configures %s" % (script_path, flag))
    for pattern in BANNED_FLAGS:
        for m in re.finditer(pattern + r"(?![A-Za-z0-9_-])", code):
            problems.append("%s configures %s" % (script_path, m.group(0)))
    for pattern in (r"--enable-(?:muxer|encoder|filter|bsf|hwaccel|indev|outdev)=",):
        if re.search(pattern, code):
            problems.append("%s enables a component kind it must not" % script_path)
    for kind, banned in BANNED.items():
        for name in sorted(lists.get(kind, set()) & banned):
            problems.append("%s enables the %s %s, which is banned" % (script_path, kind, name))
    if lists["protocol"] != {"file"}:
        problems.append("%s: the only protocol is file, not %s" % (script_path, sorted(lists["protocol"])))
    if "FFMPEG_LICENSE" not in code or "LGPL version 2.1 or later" not in code:
        problems.append("%s no longer checks that the build is LGPL 2.1 or later" % script_path)
    # The committed list.
    if built:
        built_text = read(os.path.join(built, "share/ffmpeg-components.txt"))
        if write:
            with open(os.path.join(ROOT, list_path), "w", encoding="utf-8") as f:
                f.write(built_text)
        elif built_text != read(list_path):
            problems.append("%s is not what the build at %s compiled in (run with --write to replace it)" % (list_path, built))
    pairs, counts = list_file(read(list_path))
    for kind, name in sorted(pairs):
        if name in BANNED.get(kind, ()):
            problems.append("%s has the %s %s, which is banned" % (list_path, kind, name))
        if kind in ("muxer", "encoder"):
            problems.append("%s has the %s %s: none are built" % (list_path, kind, name))
        if kind == "bsf" and name not in ALLOWED_BSF:
            problems.append("%s has the bitstream filter %s, which no decoder here needs" % (list_path, name))
    for kind in ("hwaccel", "filter", "indev", "outdev"):
        if counts.get(kind, 0) != 0:
            problems.append("%s counts %d %ss: none are built" % (list_path, counts[kind], kind))
    for kind, names in lists.items():
        have = {n for k, n in pairs if k == kind}
        for name in sorted(names - have):
            problems.append("%s enables the %s %s but %s does not list it" % (script_path, kind, name, list_path))
        for name in sorted(have - names):
            problems.append("%s lists the %s %s, which %s does not enable" % (list_path, kind, name, script_path))
    return problems


def main(argv):
    built, write = None, False
    script, lst = SCRIPT, LIST
    args = list(argv)
    while args:
        a = args.pop(0)
        if a == "--built":
            built = args.pop(0)
        elif a == "--write":
            write = True
        elif a == "--script":
            script = args.pop(0)
        elif a == "--components":
            lst = args.pop(0)
        else:
            print("usage: check_ffmpeg_components.py [--built PREFIX [--write]]", file=sys.stderr)
            return 2
    problems = []
    if lst == LIST and not built:
        # (one list for each release the script builds: ffmpeg-components.txt for 7.1.5, ffmpeg-components-6.1.6.txt for the snap's)
        import glob
        for path in sorted(glob.glob(os.path.join(ROOT, "third_party/ffmpeg-components*.txt"))):
            problems += check(script, os.path.relpath(path, ROOT), None, False)
    else:
        problems = check(script, lst, built, write)
    for p in problems:
        print("FFmpeg components: " + p)
    if problems:
        return 1
    print("FFmpeg components OK")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
