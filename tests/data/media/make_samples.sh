#!/bin/sh
# Makes the sample movies and sounds of this folder (JeffDeck's media tests play them; the folders are kept/, dropped/, and audio_dropped/).
# They are made here, from nothing: a gradient that moves, and sine tones, so there is no one else's file in them. Two seconds each, small pictures, little
# sound, to keep the whole folder under a megabyte.
#
#   sh make_samples.sh   (needs a full FFmpeg 7.x with its development files, and its libx264, libx265, libvpx, libmp3lame, libvorbis, libopus,
#                         libtheora, and libsvtav1; any FFmpeg of those majors will do, e.g. Debian's libav*-dev)
#
# kept/ plays in every build of JeffDeck; dropped/ is in formats the build of FFmpeg that is packaged leaves out (tools/build_ffmpeg.sh), and audio_dropped/
# holds a movie whose sound is in a format it leaves out. Which is which: third_party/README.md.
set -eu
cd "$(dirname "$0")"
# (MAKE_SAMPLE names a make_sample already built, which is used as it is)
G=${MAKE_SAMPLE:-/tmp/make_sample.$$}
if [ -z "${MAKE_SAMPLE:-}" ]; then
    cc -O1 -w make_sample.c -o "$G" $(pkg-config --cflags --libs libavformat libavcodec libavutil)
    trap 'rm -f $G' EXIT
fi
mkdir -p kept dropped audio_dropped
export W=64 H=48 FPS=10 CH=1
s() { out=$1; v=$2; a=$3; shift 3; env "$@" $G "$out" "$v" "$a" 2 >/dev/null 2>&1 || { echo "failed: $out" >&2; exit 1; }; }
cd kept
s t.wav - pcm_s16le RATE=8000;  s t_ms.wav - adpcm_ms RATE=8000;  s t_ima.wav - adpcm_ima_wav RATE=8000
s t.mp3 - libmp3lame RATE=22050;  s t.aac - aac RATE=22050;  s t.m4a - aac RATE=22050;  s alac.m4a - alac RATE=8000
s t.flac - flac RATE=8000;  s t.ogg - libvorbis RATE=22050;  s t.opus - libopus RATE=16000;  s t.aiff - pcm_s16be RATE=8000;  s t.wma - wmav2 RATE=22050
s ac3_only.mka - ac3 RATE=32000;  s eac3_only.mka - eac3 RATE=32000
s h264.mp4 libx264 aac RATE=22050;  s h264.mkv libx264 aac RATE=22050;  s h264.mov libx264 aac RATE=22050;  s h264.3gp libx264 aac RATE=22050
s h264.ts libx264 aac RATE=22050;  s hevc.mp4 libx265 aac RATE=22050;  s vp8.webm libvpx libvorbis RATE=22050;  s vp9.webm libvpx-vp9 libopus RATE=16000
s mpeg4.avi mpeg4 libmp3lame RATE=22050;  s msmpeg4.avi msmpeg4 libmp3lame RATE=22050;  s wmv1.wmv wmv1 wmav1 RATE=22050;  s wmv2.wmv wmv2 wmav2 RATE=22050
s mpeg2.mpg mpeg2video mp2 RATE=22050;  s mpeg2.ts mpeg2video mp2 RATE=22050;  s mjpeg.avi mjpeg pcm_s16le RATE=8000
s cinepak.avi cinepak pcm_s16le RATE=8000;  s msvideo1.avi msvideo1 pcm_s16le RATE=8000
s ac3.mkv libx264 ac3 RATE=32000;  s eac3.mkv libx264 eac3 RATE=32000;  s ac3.ts mpeg2video ac3 RATE=32000;  s ac3.mpg mpeg2video ac3 RATE=32000
s ac3.mp4 libx264 ac3 RATE=32000;  s eac3.mp4 libx264 eac3 RATE=32000
cd ../dropped
s flv1.flv flv libmp3lame RATE=22050;  s theora.ogv libtheora libvorbis RATE=22050;  s av1.mkv libsvtav1 aac RATE=22050 W=64 H=64
cd ../audio_dropped
s truehd.mkv libx264 truehd RATE=48000 STRICT=1
