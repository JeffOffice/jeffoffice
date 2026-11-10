#!/bin/sh
# Prints the FFmpeg release (for tools/build_ffmpeg.sh's FFMPEG_VERSION) whose libraries a Qt FFmpeg media plugin binds to, from the plugin's NEEDED entries:
#
#   tools/ffmpeg_for_plugin.sh PLUGIN.so      -> 7.1.5 or 6.1.6; exit 1, with a message, for any other set of library versions
#
# The snap builds its own FFmpeg for the plugin of the KDE content snap's SDK, whatever FFmpeg major that Qt was built with.
set -eu
plugin=${1:?plugin}
[ -f "$plugin" ] || { echo "ffmpeg_for_plugin: no $plugin" >&2; exit 1; }
needed=$(readelf -d "$plugin" | sed -n 's/.*(NEEDED).*\[\(lib\(avformat\|avcodec\|avutil\|swresample\|swscale\)\.so\.[0-9]*\)\].*/\1/p' | LC_ALL=C sort | tr '\n' ' ')
case "$needed" in
    "libavcodec.so.61 libavformat.so.61 libavutil.so.59 libswresample.so.5 libswscale.so.8 ") echo 7.1.5 ;;
    "libavcodec.so.60 libavformat.so.60 libavutil.so.58 libswresample.so.4 libswscale.so.7 ") echo 6.1.6 ;;
    *) echo "ffmpeg_for_plugin: $plugin needs FFmpeg libraries this build has no release for: ${needed:-none} (known: avcodec 61/avformat 61/avutil 59/swresample 5/swscale 8, which is 7.1.5; and 60/60/58/4/7, which is 6.1.6)" >&2; exit 1 ;;
esac
