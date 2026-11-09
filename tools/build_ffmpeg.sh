#!/bin/sh
# Builds the FFmpeg libraries JeffDeck plays movies and sounds with (Qt Multimedia's FFmpeg plugin loads them), from the
# official source of FFmpeg 7.1.5, with only what a movie or sound in a presentation needs. A presentation is a file
# from a stranger, so every demuxer and decoder compiled in is attack surface; Qt's own build has all of them.
#
#   tools/build_ffmpeg.sh PREFIX [WORKDIR]
#
# PREFIX gets lib/ (Windows: bin/ too, for the DLLs), include/, and share/ffmpeg-components.txt, the list of what was
# compiled in (third_party/ffmpeg-components.txt is its committed copy; tools/check_ffmpeg_components.py compares them).
# WORKDIR (default: ffmpeg-build next to PREFIX) is where the tarball, the unpacked source, and the build go; it is kept
# so that a second run is quick, and is safe to delete.
#
# The library versions are Qt's (libavcodec 61, libavformat 61, libavutil 59, libswresample 5, libswscale 8): Qt's
# plugin binds to them, so this stays on the 7.1 source (the snap's Qt binds to 60, 60, 58, 4, 7: FFMPEG_VERSION=6.1.6). LGPL 2.1 or later only: nothing GPL, nothing nonfree.
#
# Needs, on every system: a C compiler, make, tar with xz, curl (unless FFMPEG_TARBALL is given), and sha256sum or shasum.
#   Linux:    gcc or clang, make, nasm (x86_64; FFMPEG_NO_ASM=1 builds without assembly, several times slower to decode)
#   Windows:  MinGW-w64 gcc (the one Qt for Windows uses) on PATH, with MSYS's make, sh, and coreutils (Git for Windows's bash
#             or MSYS2's), and nasm; run it from that sh
#   macOS:    Xcode's clang, make, and nasm (x86_64 only: brew install nasm). FFMPEG_ARCH=arm64 or x86_64 (default: this Mac's);
#             the other one is cross-compiled. Libraries for both are joined with lipo by tools/use_ffmpeg.sh.
# Environment: FFMPEG_ARCH (macOS), FFMPEG_NO_ASM=1, FFMPEG_TARBALL=path of an already downloaded tarball (still checked),
# FFMPEG_JOBS (default 4), FFMPEG_VERSION (7.1.5 or 6.1.6).
set -eu

# The release to build (FFMPEG_VERSION): 7.1.5 for the libraries Qt 6.11's own packages bind to (libavcodec 61, libavutil 59), and 6.1.6 for the snap,
# whose Qt binds to libavcodec 60 and libavutil 58. Each has the SHA-256 of its official tarball here; any other release is refused.
VERSION=${FFMPEG_VERSION:-7.1.5}
case "$VERSION" in
    7.1.5) SHA256=de668509caf9e35e3cd162473441fdb29538c6d96ed080292b3cf9e6fc5d558f; AVCODEC_MAJOR=61 ;;
    6.1.6) SHA256=d4fcb164028dd3beee5d92c0ac72e46aac6973c75ea12dc14de07bf8f407370a; AVCODEC_MAJOR=60 ;;
    *) echo "build_ffmpeg: FFMPEG_VERSION $VERSION has no SHA-256 in this script" >&2; exit 1 ;;
esac
URL="https://ffmpeg.org/releases/ffmpeg-$VERSION.tar.xz"

# What a presentation's movies and sounds are: see third_party/README.md for the evidence. Containers (the matroska
# demuxer reads WebM too, mov reads mp4, m4a, and 3gp), what is in them, and the parsers those need to find frames.
DEMUXERS="
mov
matroska
avi
asf
mpegps
mpegts
mpegvideo
wav
mp3
aac
flac
ogg
aiff
"
DECODERS="
h264
hevc
mpeg4
h263
mpeg1video
mpeg2video
msmpeg4v1
msmpeg4v2
msmpeg4v3
wmv1
wmv2
wmv3
vc1
vp8
vp9
mjpeg
rawvideo
aac
ac3
eac3
mp2float
mp3float
wmav1
wmav2
flac
vorbis
opus
alac
pcm_s8
pcm_u8
pcm_s16le
pcm_s16be
pcm_s24le
pcm_s24be
pcm_s32le
pcm_s32be
pcm_f32le
pcm_f32be
pcm_f64le
pcm_f64be
pcm_alaw
pcm_mulaw
adpcm_ms
adpcm_ima_wav
adpcm_ima_qt
"
PARSERS="
h264
hevc
mpeg4video
h263
mpegvideo
vc1
vp8
vp9
mjpeg
aac
ac3
mpegaudio
flac
vorbis
opus
"
# Only a file on disk: no network, no pipe, no playlist reader, no data URL. (JeffDeck hands the player its bytes in memory.)
PROTOCOLS="
file
"

die() { echo "build_ffmpeg: $*" >&2; exit 1; }

[ $# -ge 1 ] || die "usage: tools/build_ffmpeg.sh PREFIX [WORKDIR]"
mkdir -p "$1"
PREFIX=$(cd "$1" && pwd)
WORK=${2:-"$PREFIX.build"}
mkdir -p "$WORK"
WORK=$(cd "$WORK" && pwd)
JOBS=${FFMPEG_JOBS:-4}

sha256_of() {
    if command -v sha256sum >/dev/null 2>&1; then sha256sum "$1" | cut -d' ' -f1
    else shasum -a 256 "$1" | cut -d' ' -f1; fi
}

# ---- the source: its own folder, checked before it is unpacked ----
TARBALL=${FFMPEG_TARBALL:-}
if [ -z "$TARBALL" ]; then
    mkdir -p "$WORK/download"
    TARBALL="$WORK/download/ffmpeg-$VERSION.tar.xz"
    [ -f "$TARBALL" ] || curl -fsSL --retry 3 -o "$TARBALL" "$URL"
fi
[ "$(sha256_of "$TARBALL")" = "$SHA256" ] || die "$TARBALL is not FFmpeg $VERSION (its SHA-256 differs from the one in this script)"
SRC="$WORK/src/ffmpeg-$VERSION"
if [ ! -f "$SRC/configure" ]; then
    rm -rf "${WORK:?}/src"
    mkdir -p "$WORK/src"
    tar -xJf "$TARBALL" -C "$WORK/src"
fi

grep -q "LIBAVCODEC_VERSION_MAJOR  $AVCODEC_MAJOR\$" "$SRC/libavcodec/version_major.h" || die "FFmpeg $VERSION does not have libavcodec $AVCODEC_MAJOR"

# ---- the system ----
OS=$(uname -s)
ARCH=$(uname -m)
SYSFLAGS=""
CFLAGS_EXTRA="-fstack-protector-strong"
LDFLAGS_EXTRA=""
case "$OS" in
    Linux)
        SYSFLAGS="--enable-pthreads"
        CFLAGS_EXTRA="$CFLAGS_EXTRA -D_FORTIFY_SOURCE=2"
        # (the libraries find each other beside themselves, as Qt's do)
        LDFLAGS_EXTRA="-Wl,-z,relro -Wl,-z,now -Wl,-rpath,@@ORIGIN@@"
        ;;
    MINGW* | MSYS* | CYGWIN*)
        SYSFLAGS="--target-os=mingw32 --disable-pthreads --enable-w32threads"
        # (the DLLs need no GCC runtime of their own; MinGW's stack protector needs a library of its own, so none here)
        CFLAGS_EXTRA=""
        LDFLAGS_EXTRA="-static-libgcc -Wl,--nxcompat -Wl,--dynamicbase"
        ;;
    Darwin)
        TARGET=${FFMPEG_ARCH:-$ARCH}
        SYSFLAGS="--enable-pthreads --install-name-dir=@rpath"
        CFLAGS_EXTRA="$CFLAGS_EXTRA -mmacosx-version-min=12.0 -arch $TARGET"
        # (the libraries find each other beside themselves, as Qt's do)
        LDFLAGS_EXTRA="-mmacosx-version-min=12.0 -arch $TARGET -Wl,-rpath,@loader_path"
        if [ "$TARGET" != "$ARCH" ]; then
            SYSFLAGS="$SYSFLAGS --enable-cross-compile --arch=$TARGET --target-os=darwin"
        fi
        ;;
    *) die "not a system this script knows: $OS" ;;
esac
ASM=""
if [ "${FFMPEG_NO_ASM:-}" = 1 ]; then
    ASM="--disable-asm"
else
    case "$OS:${FFMPEG_ARCH:-$ARCH}" in
        *:x86_64 | *:amd64 | *:i686)
            command -v nasm >/dev/null 2>&1 || die "nasm is needed for the x86 assembly (FFMPEG_NO_ASM=1 builds without it, and decodes several times slower)" ;;
    esac
fi

# ---- configure ----
ENABLES=""
for d in $DEMUXERS; do ENABLES="$ENABLES --enable-demuxer=$d"; done
for d in $DECODERS; do ENABLES="$ENABLES --enable-decoder=$d"; done
for d in $PARSERS; do ENABLES="$ENABLES --enable-parser=$d"; done
for d in $PROTOCOLS; do ENABLES="$ENABLES --enable-protocol=$d"; done

BUILD="$WORK/build"
rm -rf "${BUILD:?}"
mkdir -p "$BUILD"
cd "$BUILD"
# shellcheck disable=SC2086
"$SRC/configure" --prefix="$PREFIX" \
    --disable-everything --disable-autodetect --disable-network --disable-programs --disable-doc --disable-debug \
    --disable-avdevice --disable-avfilter --disable-postproc \
    --disable-hwaccels --disable-iconv --disable-zlib --disable-bzlib --disable-lzma \
    --enable-shared --disable-static --enable-pic \
    --enable-avcodec --enable-avformat --enable-avutil --enable-swresample --enable-swscale \
    $SYSFLAGS $ASM $ENABLES \
    --extra-cflags="$CFLAGS_EXTRA" --extra-ldflags="$LDFLAGS_EXTRA" >configure.log 2>&1 || { tail -30 configure.log >&2; die "configure failed (see $BUILD/configure.log)"; }

# (configure would expand the dollar sign itself, and the makefiles expand it twice: the libraries' run path is "$ORIGIN", written so the shell sees it quoted)
sed "s/@@ORIGIN@@/'\$\$\$\$ORIGIN'/" ffbuild/config.mak >ffbuild/config.mak.new && mv ffbuild/config.mak.new ffbuild/config.mak

# The license must come out LGPL: a GPL or nonfree component would say so here.
grep -q '^#define FFMPEG_LICENSE "LGPL version 2.1 or later"' config.h || die "the build is not LGPL 2.1 or later"

make -j"$JOBS" >make.log 2>&1 || { tail -30 make.log >&2; die "make failed (see $BUILD/make.log)"; }
rm -rf "${PREFIX:?}/lib" "${PREFIX:?}/include" "${PREFIX:?}/bin"
make install >install.log 2>&1 || die "make install failed (see $BUILD/install.log)"

# ---- what was compiled in: the lists the libraries themselves are made from ----
mkdir -p "$PREFIX/share"
LIST="$PREFIX/share/ffmpeg-components.txt"
{
    echo "# FFmpeg $VERSION as tools/build_ffmpeg.sh configures it: every component compiled in, one per line (kind name)."
    echo "# Made by that script from the lists the libraries are built from; tools/check_ffmpeg_components.py reads it."
    for pair in "demuxer libavformat/demuxer_list.c" "muxer libavformat/muxer_list.c" "protocol libavformat/protocol_list.c" \
        "decoder libavcodec/codec_list.c" "encoder libavcodec/codec_list.c" "parser libavcodec/parser_list.c" "bsf libavcodec/bsf_list.c"; do
        kind=${pair%% *}
        file=${pair#* }
        sed -n 's/^[ \t]*&ff_\(.*\)_\([a-z0-9]*\),*$/\1 \2/p' "$file" | awk -v k="$kind" '$2 == k { print k, $1 }'
    done | LC_ALL=C sort
    echo "count hwaccel $(grep -c '^#define CONFIG_[A-Z0-9_]*_HWACCEL 1' config_components.h || true)"
} >"$LIST"
echo "build_ffmpeg: FFmpeg $VERSION is in $PREFIX; components: $LIST"
