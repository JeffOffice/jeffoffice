#!/bin/sh
# Decodes every sample file of tests/data/media/kept with the FFmpeg libraries a build made (PREFIX: its lib/, include/, and on Windows bin/), compiling
# tools/ffmpeg_decode_check.c against them; fails if any file decodes nothing. The files in dropped/ and audio_dropped/ are tried too and only reported.
#
#   tools/ffmpeg_decode_check.sh PREFIX [SAMPLEDIR]     (environment as for tools/ffmpeg_probe.sh: CC, PROBE_ARCH, PROBE_RUN)
set -eu
PREFIX=$(cd "${1:?PREFIX}" && pwd)
HERE=$(cd "$(dirname "$0")" && pwd)
SAMPLES=${2:-$HERE/../tests/data/media}
TMP=$(mktemp -d)
trap 'rm -rf "${TMP:?}"' EXIT
FLAGS=""
case "$(uname -s)" in
    Darwin) [ -n "${PROBE_ARCH:-}" ] && FLAGS="-arch $PROBE_ARCH"; RP="-Wl,-rpath,$PREFIX/lib" ;;
    MINGW* | MSYS* | CYGWIN*) PATH="$PREFIX/bin:$PATH"; export PATH; RP="" ;;
    *) RP="-Wl,-rpath,$PREFIX/lib" ;;
esac
# shellcheck disable=SC2086
${CC:-cc} -O1 -Wall $FLAGS -I"$PREFIX/include" "$HERE/ffmpeg_decode_check.c" -o "$TMP/check" -L"$PREFIX/lib" -lavformat -lavcodec -lavutil $RP
if [ -n "${PROBE_RUN:-}" ]; then
    # shellcheck disable=SC2086
    $PROBE_RUN true >/dev/null 2>&1 || { echo "ffmpeg_decode_check: this machine can't run '$PROBE_RUN'" >&2; exit 4; }
fi
bad=0
for f in "$SAMPLES"/kept/*; do
    # shellcheck disable=SC2086
    ${PROBE_RUN:-} "$TMP/check" "$f" || bad=1
done
for f in "$SAMPLES"/dropped/* "$SAMPLES"/audio_dropped/*; do
    # shellcheck disable=SC2086
    ${PROBE_RUN:-} "$TMP/check" "$f" || true
done
exit $bad
