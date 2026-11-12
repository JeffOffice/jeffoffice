#!/bin/sh
# sh tools/test_deb_repo_scripts.sh : the .deb's postinst adds the apt repository once, leaves an existing or
# refused one alone, and its postrm takes away only what it added. Runs against a scratch DPKG_ROOT.
set -e
here=$(cd "$(dirname "$0")/.." && pwd)
inst=$here/packaging/linux/deb-postinst rm_=$here/packaging/linux/deb-postrm
t=$(mktemp -d); trap 'rm -rf "$t"' EXIT
fresh() { rm -rf "$t/r"; mkdir -p "$t/r/etc/apt/sources.list.d" "$t/r/opt/jeffoffice/share"; cp "$here/packaging/linux/jeffoffice-apt.gpg" "$t/r/opt/jeffoffice/share/jeffoffice-apt.gpg"; }
fail() { echo "FAIL: $*"; exit 1; }
S=$t/r/etc/apt/sources.list.d/jeffoffice.sources K=$t/r/etc/apt/keyrings/jeffoffice.gpg
# a new install adds both files, and a second run changes nothing
fresh; DPKG_ROOT=$t/r sh "$inst" configure ""
[ -f "$S" ] && [ -f "$K" ] || fail "nothing added"
grep -q '^URIs: https://jeffoffice.github.io/apt$' "$S" || fail "wrong entry"
cmp -s "$K" "$here/packaging/linux/jeffoffice-apt.gpg" || fail "wrong key"
cp "$S" "$t/before"; DPKG_ROOT=$t/r sh "$inst" configure 0.25.1; cmp -s "$S" "$t/before" || fail "second run changed the entry"
# only configure does anything
fresh; DPKG_ROOT=$t/r sh "$inst" abort-upgrade 0.26.0; [ ! -e "$S" ] || fail "added on abort-upgrade"
# one written by hand (the guide's, or an old-style .list) is left alone
fresh; printf 'Types: deb\nURIs: https://jeffoffice.github.io/apt\n' > "$S"; cp "$S" "$t/hand"
DPKG_ROOT=$t/r sh "$inst" configure ""; cmp -s "$S" "$t/hand" || fail "hand-written entry changed"; [ ! -e "$K" ] || fail "key added beside a hand-written entry"
fresh; echo 'deb [signed-by=/etc/apt/keyrings/jeffoffice.gpg] https://jeffoffice.github.io/apt stable main' > "$t/r/etc/apt/sources.list.d/jo.list"
DPKG_ROOT=$t/r sh "$inst" configure ""; [ ! -e "$S" ] || fail "added beside a .list entry"
# refused in /etc/default/jeffoffice
fresh; mkdir -p "$t/r/etc/default"; echo 'add_repository=false' > "$t/r/etc/default/jeffoffice"
DPKG_ROOT=$t/r sh "$inst" configure ""; [ ! -e "$S" ] || fail "added though refused"
# no apt at all: nothing
rm -rf "$t/r"; mkdir -p "$t/r/opt/jeffoffice/share"; DPKG_ROOT=$t/r sh "$inst" configure ""; [ ! -e "$t/r/etc" ] || fail "made /etc/apt on a system without apt"
# purge takes away what was added, remove keeps it, and a hand-written entry survives purge
fresh; DPKG_ROOT=$t/r sh "$inst" configure ""; DPKG_ROOT=$t/r sh "$rm_" remove; [ -f "$S" ] || fail "remove took the entry"
DPKG_ROOT=$t/r sh "$rm_" purge; [ ! -e "$S" ] && [ ! -e "$K" ] || fail "purge left files"
fresh; printf 'Types: deb\nURIs: https://jeffoffice.github.io/apt\nSigned-By: /etc/apt/keyrings/jeffoffice.gpg\n' > "$S"; mkdir -p "$t/r/etc/apt/keyrings"; cp "$here/packaging/linux/jeffoffice-apt.gpg" "$K"
DPKG_ROOT=$t/r sh "$rm_" purge; [ -f "$S" ] && [ -f "$K" ] || fail "purge took a hand-written entry"
echo "deb repo scripts: all checks passed"
