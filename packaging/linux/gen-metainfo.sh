#!/usr/bin/env bash
# Writes an AppStream file with its <releases> made from the project's version and today's date (or SOURCE_DATE_EPOCH's day):
#
#   packaging/linux/gen-metainfo.sh <source metainfo.xml> <output metainfo.xml>
#
# The packages (build-packages.sh, snap/snapcraft.yaml) call this, so a release never ships last release's number.
set -euo pipefail
here=$(cd "$(dirname "$0")" && pwd)
ver=$(sed -n 's/^project(JeffPub VERSION \([0-9.]*\).*/\1/p' "$here/../../CMakeLists.txt")
[ -n "$ver" ] || { echo "no version in CMakeLists.txt" >&2; exit 1; }
day=$(date -u ${SOURCE_DATE_EPOCH:+-d "@$SOURCE_DATE_EPOCH"} +%F)
mkdir -p "$(dirname "$2")"
awk -v v="$ver" -v d="$day" '
  /<releases>/ { print; print "    <release version=\"" v "\" date=\"" d "\">"; print "      <url type=\"details\">https://github.com/JeffOffice/jeffoffice/releases/tag/v" v "</url>"; print "    </release>"; skip = 1; next }
  /<\/releases>/ { skip = 0 }
  !skip { print }
' "$1" > "$2"
