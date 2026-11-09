#!/bin/sh
# Builds tools/ffmpeg_probe.c and runs it on the FFmpeg libraries in LIBDIR (on Windows, the folder with the DLLs), writing its list to OUTFILE:
#
#   tools/ffmpeg_probe.sh LIBDIR OUTFILE
#
# Environment: CC (default cc); PROBE_ARCH (macOS: arm64 or x86_64, the processor of the libraries); PROBE_RUN (a command put before the probe,
# "arch -x86_64" to run Intel libraries on an Apple processor). Exit 4: this machine can't run that processor's programs (the caller says so).
set -eu
LIBDIR=$(cd "${1:?LIBDIR}" && pwd)
OUT=${2:?OUTFILE}
HERE=$(cd "$(dirname "$0")" && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "${TMP:?}"' EXIT
FLAGS=""
LIBS=""
case "$(uname -s)" in
    Darwin) [ -n "${PROBE_ARCH:-}" ] && FLAGS="-arch $PROBE_ARCH" ;;
    MINGW* | MSYS* | CYGWIN*) PATH="$LIBDIR:$PATH"; export PATH ;;
    *) LIBS="-ldl" ;;
esac
# shellcheck disable=SC2086
${CC:-cc} -O1 -Wall $FLAGS "$HERE/ffmpeg_probe.c" -o "$TMP/probe" $LIBS
if [ -n "${PROBE_RUN:-}" ]; then
    # shellcheck disable=SC2086
    $PROBE_RUN true >/dev/null 2>&1 || { echo "ffmpeg_probe: this machine can't run '$PROBE_RUN'" >&2; exit 4; }
fi
# shellcheck disable=SC2086
${PROBE_RUN:-} "$TMP/probe" "$LIBDIR" >"$OUT"
