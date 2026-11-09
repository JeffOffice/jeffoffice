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
    python3 tools/check_ffmpeg_components.py --probe FILE [--allow-hwdevice NAME] [--write]   FILE is what tools/ffmpeg_probe.sh found in the built libraries
                                                                    (run by CI on every system's build): fails on anything the script does not allow
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
                r"--enable-bzlib", r"--enable-lzma", r"--enable-iconv", r"--enable-libx", r"--enable-lib[a-z0-9]+"]
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
        built = os.path.abspath(built)
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


def golden_of(probe_text):
    """What the libraries report, as the file third_party/ffmpeg-runtime-VERSION.txt keeps it: sorted, without the version line or the hardware devices."""
    lines = [l.strip() for l in probe_text.splitlines() if l.strip() and not l.startswith(("version", "hwdevice"))]
    return "\n".join(sorted(lines)) + "\n"


def probe_version(probe_text):
    m = re.match(r"version (\S+)", probe_text)
    return m.group(1) if m else None


def tokens(name):
    return set(name.split(","))


def check_probe(probe_text, golden_text, script_path, allow_hw=()):
    """Compares what tools/ffmpeg_probe.c found in the built libraries with what the script allows. The runtime names are not the configure names (the
    demuxer "mov" says "mov,mp4,m4a,3gp,3g2,mj2", the decoder msmpeg4v3 says "msmpeg4"), so the comparison is with the file that keeps what a good build
    reports (the same on every system), and the file is held to the script: as many demuxers, decoders, parsers as it enables, no muxer, no encoder, only the
    file protocol, no hardware device but those the system gives (allow_hw), no bitstream filter but the allowed, and nothing banned."""
    lists = script_lists(read(script_path))
    found = {}
    parsers = []
    for line in probe_text.splitlines():
        parts = line.split()
        if not parts or parts[0] in ("version", "decoder-codec"):
            continue
        if parts[0] == "parser":
            parsers.append(" ".join(parts[1:]))
        elif len(parts) == 2:
            found.setdefault(parts[0], set()).add(parts[1])
        else:
            return ["not a line of the probe's list: " + line]
    if not found.get("demuxer"):
        return ["the probe found no demuxers: the libraries did not load, or the probe did not run"]
    problems = []
    if golden_of(probe_text) != golden_text:
        have = set(golden_of(probe_text).splitlines())
        keep = set(golden_text.splitlines())
        for l in sorted(have - keep):
            problems.append("the built libraries have '%s', which the good list does not" % l)
        for l in sorted(keep - have):
            problems.append("the built libraries lack '%s', which the good list has" % l)
    for kind in ("demuxer", "decoder"):
        if len(found.get(kind, set())) != len(lists[kind]):
            problems.append("the built libraries have %d %ss; %s enables %d" % (len(found.get(kind, set())), kind, script_path, len(lists[kind])))
    if len(parsers) != len(lists["parser"]):
        problems.append("the built libraries have %d parsers; %s enables %d" % (len(parsers), script_path, len(lists["parser"])))
    for kind in ("muxer", "encoder"):
        for name in sorted(found.get(kind, set())):
            problems.append("the built libraries have the %s %s: none are built" % (kind, name))
    for kind in ("protocol-in", "protocol-out"):
        for name in sorted(found.get(kind, set()) - lists["protocol"]):
            problems.append("the built libraries have the protocol %s (%s)" % (name, kind))
    if found.get("protocol-in", set()) != lists["protocol"]:
        problems.append("the built libraries' input protocols are %s, not %s" % (sorted(found.get("protocol-in", set())), sorted(lists["protocol"])))
    for name in sorted(found.get("bsf", set()) - ALLOWED_BSF):
        problems.append("the built libraries have the bitstream filter %s" % name)
    for name in sorted(found.get("hwdevice", set()) - set(allow_hw)):
        problems.append("the built libraries have the hardware device type %s" % name)
    for kind, banned in BANNED.items():
        for name in sorted(found.get(kind, set())):
            if tokens(name) & banned:
                problems.append("the built libraries have the banned %s %s" % (kind, name))
    for name in sorted(found.get("protocol-in", set()) | found.get("protocol-out", set())):
        if name in BANNED["protocol"]:
            problems.append("the built libraries have the banned protocol %s" % name)
    for name in sorted(found.get("muxer", set()) | found.get("encoder", set())):
        if name in ("spdif", "adx", "libx264"):
            problems.append("the built libraries have the banned %s" % name)
    return problems


def main(argv):
    built, write, probe, allow_hw = None, False, None, []
    script, lst = SCRIPT, LIST
    args = list(argv)
    while args:
        a = args.pop(0)
        if a == "--built":
            built = args.pop(0)
        elif a == "--write":
            write = True
        elif a == "--probe":
            probe = args.pop(0)
        elif a == "--allow-hwdevice":
            allow_hw.append(args.pop(0))
        elif a == "--script":
            script = args.pop(0)
        elif a == "--components":
            lst = args.pop(0)
        else:
            print("usage: check_ffmpeg_components.py [--built PREFIX [--write]]", file=sys.stderr)
            return 2
    problems = []
    if probe:
        with open(probe, encoding="utf-8") as f:
            text = f.read()
        version = probe_version(text)
        golden_path = os.path.join(ROOT, "third_party/ffmpeg-runtime-%s.txt" % version)
        if write:
            with open(golden_path, "w", encoding="utf-8") as f:
                f.write(golden_of(text))
        if not os.path.exists(golden_path):
            print("FFmpeg libraries: there is no %s for FFmpeg %s" % (os.path.relpath(golden_path, ROOT), version))
            return 1
        with open(golden_path, encoding="utf-8") as f:
            problems = check_probe(text, f.read(), script, allow_hw)
        for p in problems:
            print("FFmpeg libraries: " + p)
        if problems:
            return 1
        print("FFmpeg libraries OK: what the built libraries hold is what the script allows")
        return 0
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
