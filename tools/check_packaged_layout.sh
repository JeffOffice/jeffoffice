#!/bin/bash
# The packaged layout on Linux: installs the build into a temporary prefix (cmake --install, no LTO needed), checks with ldd that every
# library each program needs resolves inside the prefix or to the system or Qt, launches each app from the prefix (offscreen), and runs
# jo_datacheck there (Help topic and a German translation load from the prefix's data folder).
# Usage: tools/check_packaged_layout.sh <build dir>      Prints one line, "packaged layout: OK" or what is wrong; exit status 1 when wrong.
set -u
B=$(cd "$1" && pwd)
P=$(mktemp -d)
trap 'rm -rf "$P"' EXIT
bad=""
note() { bad="$bad; $*"; }
qt=$(sed -n 's/^Qt6_DIR:PATH=\(.*\)\/lib\/cmake\/Qt6$/\1/p' "$B/CMakeCache.txt")
ninja -C "$B" jeffpub jeffdeck jeffwrite jeffcrunch jeffoffice-common jo_datacheck > "$P.build" 2>&1 || { echo "packaged layout: BUILD FAILED"; tail -5 "$P.build"; rm -f "$P.build"; exit 1; }
rm -f "$P.build"
cmake --install "$B" --prefix "$P" > /dev/null 2>&1 || note "cmake --install failed"
cmake --install "$B" --prefix "$P" --component check > /dev/null 2>&1 || note "install of jo_datacheck failed"
real=$(realpath "$P")
for f in "$P"/bin/*; do
  [ -x "$f" ] || continue
  out=$(LD_LIBRARY_PATH="$qt/lib" ldd "$f" 2>&1)
  echo "$out" | grep -q "not found" && note "$(basename "$f"): $(echo "$out" | grep 'not found' | head -2 | tr -s ' \t\n' ' ')"
  for lib in $(echo "$out" | awk '/=>/ {print $3}'); do
    r=$(realpath "$lib" 2>/dev/null) || continue
    case "$r" in "$real"/*|"$qt"/*|/lib/*|/usr/lib/*|/lib64/*) ;; *) note "$(basename "$f") resolves $(basename "$lib") to $r" ;; esac
  done
done
for app in JeffPub JeffDeck JeffWrite JeffCrunch; do
  [ -x "$P/bin/$app" ] || { note "$app was not installed"; continue; }
  l=$(LD_LIBRARY_PATH="$qt/lib" ldd "$P/bin/$app" | awk '/libjeffoffice-common/ {print $3}')
  [ "$(realpath "$l" 2>/dev/null)" = "$real/lib/jeffoffice/libjeffoffice-common.so" ] || note "$app does not use the installed library ($l)"
  H=$(mktemp -d)
  env -i HOME="$H" PATH="$PATH" LD_LIBRARY_PATH="$qt/lib" QT_QPA_PLATFORM=offscreen JP_NO_TELEMETRY=1 CUPS_SERVER=/nonexistent \
      timeout 120 "$P/bin/$app" --screenshot "$H/s.png" --size 800x600 > /dev/null 2>&1
  [ -s "$H/s.png" ] || note "$app did not start from the prefix"
  rm -rf "$H"
done
H=$(mktemp -d)
r=$(env -i HOME="$H" PATH="$PATH" LD_LIBRARY_PATH="$qt/lib" QT_QPA_PLATFORM=offscreen timeout 120 "$P/bin/jo_datacheck" 2>&1) || note "datacheck: $r"
rm -rf "$H"
case "$r" in *"data folder $real/share/jeffoffice/help"*) ;; *) note "datacheck did not use the prefix's data folder: $r" ;; esac
if [ -z "$bad" ]; then echo "packaged layout: OK"; else echo "packaged layout: FAILED$bad" | cut -c1-600; exit 1; fi
