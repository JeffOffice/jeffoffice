#!/usr/bin/env bash
# Builds the Linux packages from a finished build: a .deb of the suite
# (JeffPub, JeffDeck, and the previews that ship) and an AppImage of each app. They carry their own
# copy of Qt, so they run on any recent Debian or Ubuntu whatever Qt version
# the system has.
#
#   packaging/linux/build-packages.sh <build dir> <output dir>
#
# Needs QMAKE pointing at Qt's qmake, and linuxdeploy plus
# linuxdeploy-plugin-qt on PATH (or LINUXDEPLOY set).
set -euo pipefail
here=$(cd "$(dirname "$0")" && pwd)
src=$(cd "$here/../.." && pwd)
build=$(cd "$1" && pwd)
out=$(mkdir -p "$2" && cd "$2" && pwd)
ver=$(sed -n 's/^project(JeffPub VERSION \([0-9.]*\).*/\1/p' "$src/CMakeLists.txt")
arch=amd64
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
ld=${LINUXDEPLOY:-linuxdeploy}
export EXTRA_QT_MODULES="svg"
# Wayland desktops, and offscreen for command-line export without a display.
export EXTRA_PLATFORM_PLUGINS="libqoffscreen.so;libqwayland-egl.so;libqwayland-generic.so"

# The apps in the suite's package: JeffWrite too when SHIP_WRITE is "true", and JeffCrunch when SHIP_CRUNCH is (the owner decides when each ships).
suite=(JeffPub JeffDeck)
[ "${SHIP_WRITE:-}" = true ] && suite+=(JeffWrite)
[ "${SHIP_CRUNCH:-}" = true ] && suite+=(JeffCrunch)

# An AppDir with the given apps (JeffPub, JeffDeck, JeffWrite, JeffCrunch), the fonts and
# dictionaries they share, their desktop entries and icons, and the docs.
appdir() {
  local dir=$1; shift
  mkdir -p "$dir/usr/share/jeffoffice" "$dir/usr/share/doc/jeffoffice/pdfium"
  cp -r "$src/resources/fonts" "$src/resources/dict" "$dir/usr/share/jeffoffice/"
  { printf 'JeffOffice\nCopyright (c) 2026 JeffOffice LLC\nLicense: GPL-3.0 (below)\n\n'; cat "$src/LICENSE"; } > "$dir/usr/share/doc/jeffoffice/copyright"
  install -Dm644 "$src/README.md" "$dir/usr/share/doc/jeffoffice/README.md"
  # PDFium's license texts and those of the libraries built into it.
  cp "$src/third_party/pdfium/licenses/"* "$dir/usr/share/doc/jeffoffice/pdfium/"
  # FFmpeg's notice and license go with JeffDeck, the only app that carries FFmpeg.
  case " $* " in *" JeffDeck "*)
    mkdir -p "$dir/usr/share/doc/jeffoffice/ffmpeg"
    cp "$src/third_party/ffmpeg/licenses/"* "$dir/usr/share/doc/jeffoffice/ffmpeg/" ;;
  esac
  for name in "$@"; do
    local lower
    lower=$(echo "$name" | tr '[:upper:]' '[:lower:]')
    install -Dm755 "$build/$name" "$dir/usr/bin/$name"
    # The desktop entry runs the lower-case command.
    ln -s "$name" "$dir/usr/bin/$lower"
    install -Dm644 "$here/$lower.desktop" "$dir/usr/share/applications/$lower.desktop"
    # AppStream data: software centers and the snap listing read it.
    install -Dm644 "$here/com.jeffoffice.$name.metainfo.xml" "$dir/usr/share/metainfo/com.jeffoffice.$name.metainfo.xml"
  done
  case " $* " in *" JeffPub "*)
    install -Dm644 "$src/resources/app.png" "$dir/usr/share/icons/hicolor/256x256/apps/jeffpub.png"
    install -Dm644 "$here/jeffpub-mime.xml" "$dir/usr/share/mime/packages/jeffpub.xml" ;;
  esac
  case " $* " in *" JeffDeck "*)
    install -Dm644 "$src/apps/deck/resources/app.png" "$dir/usr/share/icons/hicolor/256x256/apps/jeffdeck.png" ;;
  esac
  case " $* " in *" JeffWrite "*)
    install -Dm644 "$src/apps/write/resources/app.png" "$dir/usr/share/icons/hicolor/256x256/apps/jeffwrite.png" ;;
  esac
  case " $* " in *" JeffCrunch "*)
    install -Dm644 "$src/apps/crunch/resources/app.png" "$dir/usr/share/icons/hicolor/256x256/apps/jeffcrunch.png" ;;
  esac
}

# linuxdeploy copies Qt, its plugins and the other libraries the programs
# need (PDFium among them) into an AppDir; with "appimage" it writes one too.
deploy() {
  local dir=$1 main=$2 output=$3; shift 3
  local exes=() modules="svg"
  for name in "$@"; do
    exes+=(--executable "$dir/usr/bin/$name")
    # JeffDeck plays movies and sounds through Qt Multimedia's FFmpeg
    # plugin: the plugin goes in before linuxdeploy runs, which then brings
    # the libraries it needs (FFmpeg's among them). JeffPub alone gets none.
    if [ "$name" = JeffDeck ]; then
      modules="svg;multimedia"
      mkdir -p "$dir/usr/plugins"
      cp -r "$("$QMAKE" -query QT_INSTALL_PLUGINS)/multimedia" "$dir/usr/plugins/"
    fi
  done
  local lower
  lower=$(echo "$main" | tr '[:upper:]' '[:lower:]')
  ( cd "$out" && EXTRA_QT_MODULES="$modules" LDAI_OUTPUT="$main-$ver-x86_64.AppImage" "$ld" --appdir "$dir" "${exes[@]}" \
      --library "$build/_deps/pdfium/lib/libpdfium.so" \
      --desktop-file "$dir/usr/share/applications/$lower.desktop" \
      --icon-file "$dir/usr/share/icons/hicolor/256x256/apps/$lower.png" \
      --plugin qt ${output:+--output "$output"} )
}

# 1. An AppImage of each app.
appdir "$work/JeffPub.AppDir" JeffPub
deploy "$work/JeffPub.AppDir" JeffPub appimage JeffPub
appdir "$work/JeffDeck.AppDir" JeffDeck
deploy "$work/JeffDeck.AppDir" JeffDeck appimage JeffDeck
if [ "${SHIP_WRITE:-}" = true ]; then
  appdir "$work/JeffWrite.AppDir" JeffWrite
  deploy "$work/JeffWrite.AppDir" JeffWrite appimage JeffWrite
fi
if [ "${SHIP_CRUNCH:-}" = true ]; then
  appdir "$work/JeffCrunch.AppDir" JeffCrunch
  deploy "$work/JeffCrunch.AppDir" JeffCrunch appimage JeffCrunch
fi

# 2. The .deb: both apps under /opt/jeffoffice, their commands in /usr/bin,
#    and the desktop entries, icons and file type where the desktop finds them.
appdir "$work/suite" "${suite[@]}"
deploy "$work/suite" JeffPub "" "${suite[@]}"
deb=$work/deb
mkdir -p "$deb/opt/jeffoffice" "$deb/usr/bin" "$deb/DEBIAN"
cp -a "$work/suite/usr/." "$deb/opt/jeffoffice/"
rm -f "$deb/opt/jeffoffice/bin/jeffpub" "$deb/opt/jeffoffice/bin/jeffdeck" "$deb/opt/jeffoffice/bin/jeffwrite" "$deb/opt/jeffoffice/bin/jeffcrunch"
mkdir -p "$deb/usr/share"
mv "$deb/opt/jeffoffice/share/applications" "$deb/opt/jeffoffice/share/icons" "$deb/opt/jeffoffice/share/mime" "$deb/opt/jeffoffice/share/metainfo" "$deb/usr/share/"
mkdir -p "$deb/usr/share/doc"
mv "$deb/opt/jeffoffice/share/doc/jeffoffice" "$deb/usr/share/doc/"
ln -s /opt/jeffoffice/bin/JeffPub "$deb/usr/bin/jeffpub"
ln -s /opt/jeffoffice/bin/JeffDeck "$deb/usr/bin/jeffdeck"
[ "${SHIP_WRITE:-}" = true ] && ln -s /opt/jeffoffice/bin/JeffWrite "$deb/usr/bin/jeffwrite"
[ "${SHIP_CRUNCH:-}" = true ] && ln -s /opt/jeffoffice/bin/JeffCrunch "$deb/usr/bin/jeffcrunch"
size=$(du -sk "$deb" | cut -f1)
# The suite's package replaces JeffPub's own (jeffpub, through 0.6.11), and
# that one's before it (jeffpub79, through 0.5.0).
# JeffDeck's movies and sounds: Qt Multimedia needs PulseAudio's client library (libpulse0), and its FFmpeg plugin the X11 client
# libraries it links (libxext6, libxrandr2, libxrender1) and libdrm2, which linuxdeploy leaves to the system (as it does the other X11
# and graphics libraries above). FFmpeg's own libraries, Qt's Quick and Qml (the plugin links them), and the rest are bundled.
cat > "$deb/DEBIAN/control" <<CONTROL
Package: jeffoffice
Version: $ver
Section: editors
Priority: optional
Architecture: $arch
Installed-Size: $size
Depends: libc6 (>= 2.35), libstdc++6, libgl1, libegl1, libfontconfig1, libfreetype6, libdbus-1-3, libxkbcommon0, libxkbcommon-x11-0, libx11-6, libx11-xcb1, libxcb1, libxcb-cursor0, libxcb-icccm4, libxcb-image0, libxcb-keysyms1, libxcb-randr0, libxcb-render-util0, libxcb-shape0, libxcb-xinerama0, libxcb-xkb1, libwayland-client0, libwayland-cursor0, libwayland-egl1, libglib2.0-0 | libglib2.0-0t64, libpulse0, libxext6, libxrandr2, libxrender1, libdrm2
Conflicts: jeffpub, jeffpub79
Replaces: jeffpub, jeffpub79
Provides: jeffpub, jeffpub79
Maintainer: JeffOffice LLC <https://github.com/JeffOffice/jeffoffice>
Homepage: https://github.com/JeffOffice/jeffoffice
Description: Free, open-source office suite
 JeffOffice is a free, open-source office suite. JeffPub lays out flyers,
 newsletters, brochures and cards, with templates, mail merge and PDF
 export, and opens .pub files. JeffDeck makes and shows presentations, and
 opens .pptx, .ppt and .odp files. JeffWrite (a preview) writes documents,
 and opens .docx and .doc files. JeffCrunch (a preview) makes spreadsheets,
 and opens .xlsx, .xls, .ods and .csv files.
CONTROL
dpkg-deb --build --root-owner-group "$deb" "$out/jeffoffice_${ver}_${arch}.deb"
ls -la "$out"
