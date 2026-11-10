#!/bin/sh
# Puts the FFmpeg libraries tools/build_ffmpeg.sh made in place of the full ones Qt ships, in a Qt installation (on a CI
# runner, where the Qt folder is a throwaway copy: never on a machine's own Qt), so that what CI's tests load and what
# windeployqt, macdeployqt, and linuxdeploy copy into the packages are ours. Qt's FFmpeg plugin binds to the library
# versions by name (libavcodec 61, libavformat 61, libavutil 59, libswresample 5, libswscale 8): a missing or different
# version stops here.
#
#   tools/use_ffmpeg.sh install QT_ROOT PREFIX [PREFIX_OF_THE_OTHER_PROCESSOR]   (two prefixes, on a Mac: joined with lipo)
#   tools/use_ffmpeg.sh check   DIR...      all five FFmpeg libraries are found under the folders, and each is ours (it carries our configuration)
set -eu

die() { echo "use_ffmpeg: $*" >&2; exit 1; }
FAMILIES="avcodec avformat avutil swresample swscale"
case "$(uname -s)" in
    Linux) KIND=so ;;
    Darwin) KIND=dylib ;;
    MINGW* | MSYS* | CYGWIN*) KIND=dll ;;
    *) die "not a system this script knows: $(uname -s)" ;;
esac

# The folder of a Qt installation that holds the libraries, and the files of a family in it (and the same in a build's prefix).
qt_libdir() { if [ "$KIND" = dll ]; then echo "$1/bin"; else echo "$1/lib"; fi; }
family_files() { # DIR NAME -> every file (and link) of the library
    case "$KIND" in
        so) ls "$1"/lib"$2".so "$1"/lib"$2".so.* 2>/dev/null ;;
        dylib) ls "$1"/lib"$2".dylib "$1"/lib"$2".*.dylib 2>/dev/null ;;
        dll) ls "$1/$2"-*.dll 2>/dev/null ;;
    esac
}
# The name of a library's own file by its major version: libavcodec.so.61, libavcodec.61.dylib, avcodec-61.dll.
major_name() { # FILE-NAME -> NAME.MAJOR as ours and Qt's both have it
    case "$KIND" in
        so) echo "$1" | sed -n 's/^\(lib[a-z]*\.so\.[0-9]*\)$/\1/p' ;;
        dylib) echo "$1" | sed -n 's/^\(lib[a-z]*\.[0-9]*\.dylib\)$/\1/p' ;;
        dll) echo "$1" | sed -n 's/^\([a-z]*-[0-9]*\.dll\)$/\1/p' ;;
    esac
}

mode=${1:-}
[ -n "$mode" ] || die "usage: install QT_ROOT PREFIX [PREFIX2] | check DIR..."
shift
case "$mode" in
install)
    [ $# -ge 2 ] || die "usage: install QT_ROOT PREFIX [PREFIX2]"
    QT=$1; shift
    DEST=$(qt_libdir "$QT")
    [ -d "$DEST" ] || die "no $DEST"
    if [ "$KIND" = dll ] && [ -f "$QT/plugins/multimedia/ffmpegmediaplugin.dll" ]; then
        echo "use_ffmpeg: Qt's own libraries, before they are replaced:"
        PY=python3; command -v python3 >/dev/null 2>&1 || PY=python
        "$PY" "$(dirname "$0")/check_bundle_deps.py" windows-imports "$QT/plugins/multimedia/ffmpegmediaplugin.dll" "$DEST" || true
    fi
    for name in $FAMILIES; do
        # Qt's library of this family, by the name its plugin binds to.
        had=""
        for f in $(family_files "$DEST" $name); do
            m=$(major_name "$(basename "$f")")
            [ -n "$m" ] && had=$m
        done
        [ -n "$had" ] || die "Qt has no $name library in $DEST"
        for p in "$@"; do
            src=$(qt_libdir "$p")
            [ -e "$src/$had" ] || die "this build's $name has no $had, the version Qt's plugin binds to (Qt's: $had; ours: $(for f in $(family_files "$src" $name); do basename "$f"; done | tr '\n' ' '))"
        done
        for f in $(family_files "$DEST" $name); do rm -f "$f"; done
        if [ $# -eq 1 ] || [ "$KIND" != dylib ]; then
            for f in $(family_files "$(qt_libdir "$1")" $name); do cp -a "$f" "$DEST/"; done
        else
            # Two processors: each real file is the two builds joined; the links are copied from the first.
            src1=$(qt_libdir "$1"); src2=$(qt_libdir "$2")
            for f in $(family_files "$src1" $name); do
                b=$(basename "$f")
                if [ -L "$f" ]; then cp -a "$f" "$DEST/"
                else lipo -create "$f" "$src2/$b" -output "$DEST/$b"; fi
            done
        fi
        if [ "$KIND" = dylib ]; then
            # (a library joined with lipo or copied is signed again, ad hoc: an arm64 Mac does not load one whose signature does not match)
            for f in $(family_files "$DEST" $name); do [ -L "$f" ] || codesign --force --sign - "$f"; done
        fi
        echo "use_ffmpeg: $name ($had) is ours in $DEST"
    done
    if [ "$KIND" = dll ] && [ -f "$QT/plugins/multimedia/ffmpegmediaplugin.dll" ]; then
        # (the same for Windows: each import is bound to a DLL by name, and one missing there stops the plugin from loading)
        PY=python3; command -v python3 >/dev/null 2>&1 || PY=python
        "$PY" "$(dirname "$0")/check_bundle_deps.py" windows-imports "$QT/plugins/multimedia/ffmpegmediaplugin.dll" "$DEST" || die "the plugin would not load with these libraries"
    fi
    if [ "$KIND" = dylib ] && [ -f "$QT/plugins/multimedia/libffmpegmediaplugin.dylib" ]; then
        # (every FFmpeg name the plugin imports must be found where the plugin looks: in the library it is bound to, or what that re-exports)
        python3 "$(dirname "$0")/check_macho_binds.py" "$QT/plugins/multimedia/libffmpegmediaplugin.dylib" "$DEST" || die "the plugin would not load with these libraries"
    fi
    ;;
check)
    [ $# -ge 1 ] || die "usage: check DIR..."
    # Every one of the five libraries found must be ours (each carries the configuration it was built with), and every one must be found.
    seen=""
    for dir in "$@"; do
        for name in $FAMILIES; do
            for f in $(find "$dir" \( -name "lib$name*" -o -name "$name-*.dll" \) -type f 2>/dev/null); do
                seen="$seen $name"
                grep -q -a -e '--disable-everything' "$f" || die "$f is not the FFmpeg this project builds (it has no minimal configuration)"
                grep -q -a -e 'enable-demuxer=mov' "$f" || die "$f is not the FFmpeg this project builds"
            done
        done
    done
    for name in $FAMILIES; do
        case " $seen " in *" $name "*) ;; *) die "no $name library found under $*" ;; esac
    done
    echo "use_ffmpeg: all five FFmpeg libraries found under $* are ours"
    ;;
*) die "unknown mode $mode" ;;
esac
