#!/usr/bin/env bash
# Adds .deb files to JeffOffice's apt repository and signs it.
#
#   tools/apt/publish.sh [-n KEEP] <repo dir> <file.deb>...
#
# <repo dir> is a checkout of JeffOffice/apt (an empty folder starts a new
# repository). The script puts the packages in pool/, keeps the newest KEEP
# versions of each package (default 3), rebuilds dists/stable/main/binary-amd64
# (Packages, Packages.gz) and dists/stable (Release, InRelease, Release.gpg),
# and writes the public key, the .sources file, and index.html at the top.
# Needs apt-ftparchive, dpkg-deb, and gpg. Nothing else.
#
# The signing key is a private key, never part of the repository. Give it as
#   APT_SIGNING_KEY       the key's text (ASCII-armored), as a CI secret, or
#   APT_SIGNING_KEY_FILE  the path of a file holding it.
# APT_SIGNING_PASSPHRASE is its passphrase, if it has one. It is imported into
# a throwaway GNUPGHOME that is deleted when the script ends.
# APT_BASE_URL is where the site will be served (default
# https://jeffoffice.github.io/apt); it goes into index.html and the .sources file.
set -euo pipefail

keep=3
while getopts n: opt; do
  case $opt in
    n) keep=$OPTARG ;;
    *) echo "usage: $0 [-n KEEP] <repo dir> <file.deb>..." >&2; exit 2 ;;
  esac
done
shift $((OPTIND - 1))
[ $# -ge 2 ] || { echo "usage: $0 [-n KEEP] <repo dir> <file.deb>..." >&2; exit 2; }
case $keep in ''|*[!0-9]*|0) echo "KEEP must be a positive number" >&2; exit 2 ;; esac

repo=$(mkdir -p "$1" && cd "$1" && pwd); shift
base=${APT_BASE_URL:-https://jeffoffice.github.io/apt}
base=${base%/}
suite=stable
comp=main
arch=amd64

for deb in "$@"; do
  [ -f "$deb" ] || { echo "no such file: $deb" >&2; exit 1; }
  [ "$(dpkg-deb -f "$deb" Architecture)" = "$arch" ] || { echo "$deb is not an $arch package" >&2; exit 1; }
done

# The signing key goes into a private, temporary keyring.
gnupg=$(mktemp -d)
chmod 700 "$gnupg"
trap 'gpgconf --homedir "$gnupg" --kill all >/dev/null 2>&1 || true; rm -rf "$gnupg"' EXIT
export GNUPGHOME=$gnupg
if [ -n "${APT_SIGNING_KEY:-}" ]; then
  printf '%s\n' "$APT_SIGNING_KEY" | gpg --batch --quiet --import
elif [ -n "${APT_SIGNING_KEY_FILE:-}" ]; then
  gpg --batch --quiet --import "$APT_SIGNING_KEY_FILE"
else
  echo "Set APT_SIGNING_KEY or APT_SIGNING_KEY_FILE to the private signing key." >&2
  exit 1
fi
fpr=$(gpg --batch --with-colons --list-secret-keys | awk -F: '/^fpr:/ {print $10; exit}')
[ -n "$fpr" ] || { echo "The key has no secret part." >&2; exit 1; }
sign=(gpg --batch --yes --pinentry-mode loopback --default-key "$fpr")
if [ -n "${APT_SIGNING_PASSPHRASE:-}" ]; then
  pass=$gnupg/pass
  ( umask 077; printf '%s' "$APT_SIGNING_PASSPHRASE" > "$pass" )
  sign+=(--passphrase-file "$pass")
else
  sign+=(--passphrase '')
fi

# 1. The packages: pool/main/<first letter>/<name>/<name>_<version>_<arch>.deb
for deb in "$@"; do
  name=$(dpkg-deb -f "$deb" Package)
  dir=$repo/pool/$comp/${name:0:1}/$name
  mkdir -p "$dir"
  cp -f "$deb" "$dir/${name}_$(dpkg-deb -f "$deb" Version | tr ':' '.')_${arch}.deb"
done

# 2. Keep only the newest versions of each package.
while IFS= read -r pkgdir; do
  # newest first; the version is read from the package, not the file name
  mapfile -t old < <(
    for f in "$pkgdir"/*.deb; do printf '%s\t%s\n' "$(dpkg-deb -f "$f" Version)" "$f"; done |
      sort -t$'\t' -k1,1Vr | tail -n +"$((keep + 1))" | cut -f2)
  for f in "${old[@]}"; do [ -n "$f" ] && rm -f "$f" && echo "removed old $(basename "$f")"; done
done < <(find "$repo/pool" -mindepth 3 -maxdepth 3 -type d 2>/dev/null)

# 3. The index of the packages.
bin=$repo/dists/$suite/$comp/binary-$arch
mkdir -p "$bin"
rm -f "$repo/dists/$suite/Release" "$repo/dists/$suite/InRelease" "$repo/dists/$suite/Release.gpg"
( cd "$repo" && apt-ftparchive --arch "$arch" packages pool > "dists/$suite/$comp/binary-$arch/Packages" )
gzip -9nc "$bin/Packages" > "$bin/Packages.gz"

# 4. The signed Release: its checksums cover the indexes above.
( cd "$repo" && apt-ftparchive \
    -o APT::FTPArchive::Release::Origin=JeffOffice \
    -o APT::FTPArchive::Release::Label=JeffOffice \
    -o APT::FTPArchive::Release::Suite=$suite \
    -o APT::FTPArchive::Release::Codename=$suite \
    -o APT::FTPArchive::Release::Architectures=$arch \
    -o APT::FTPArchive::Release::Components=$comp \
    -o "APT::FTPArchive::Release::Description=JeffOffice, a free, open-source office suite" \
    release "dists/$suite" > "$gnupg/Release" )
mv "$gnupg/Release" "$repo/dists/$suite/Release"
"${sign[@]}" --clearsign -o "$repo/dists/$suite/InRelease" "$repo/dists/$suite/Release"
"${sign[@]}" --armor --detach-sign -o "$repo/dists/$suite/Release.gpg" "$repo/dists/$suite/Release"

# 5. The public key (a binary keyring for Signed-By, and the same as text),
#    the .sources file, and the page that explains both.
gpg --batch --export "$fpr" > "$repo/jeffoffice.gpg"
gpg --batch --armor --export "$fpr" > "$repo/jeffoffice.asc"
cat > "$repo/jeffoffice.sources" <<SOURCES
Types: deb
URIs: $base
Suites: $suite
Components: $comp
Architectures: $arch
Signed-By: /etc/apt/keyrings/jeffoffice.gpg
SOURCES
touch "$repo/.nojekyll"

latest=$(for f in "$repo"/pool/$comp/*/*/*.deb; do dpkg-deb -f "$f" Version; done | sort -V | tail -n 1)
cat > "$repo/index.html" <<HTML
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>JeffOffice for Debian and Ubuntu</title>
<style>
  body { font: 16px/1.5 system-ui, sans-serif; max-width: 46rem; margin: 2rem auto; padding: 0 1rem; color: #1b1b1b; background: #fff; }
  pre { background: #f2f2f2; padding: .8rem 1rem; overflow-x: auto; border-radius: 4px; }
  @media (prefers-color-scheme: dark) { body { color: #e8e8e8; background: #161616; } pre { background: #262626; } a { color: #8ab4f8; } }
</style>
</head>
<body>
<h1>JeffOffice for Debian and Ubuntu</h1>
<p>JeffOffice is a free, open-source office suite: JeffPub, JeffDeck, JeffWrite, and JeffCrunch. Add this repository once, and apt installs it and keeps it up to date. The current version is $latest. It is for 64-bit Intel and AMD computers (amd64).</p>
<h2>Add the repository</h2>
<pre>sudo install -d -m 0755 /etc/apt/keyrings
sudo curl -fsSL $base/jeffoffice.gpg -o /etc/apt/keyrings/jeffoffice.gpg
sudo curl -fsSL $base/jeffoffice.sources -o /etc/apt/sources.list.d/jeffoffice.sources
sudo apt update</pre>
<h2>Install JeffOffice</h2>
<pre>sudo apt install jeffoffice</pre>
<p>Later versions arrive with your other updates (<code>sudo apt update &amp;&amp; sudo apt upgrade</code>).</p>
<h2>What the files are</h2>
<ul>
<li><a href="jeffoffice.gpg">jeffoffice.gpg</a>: the public key that signs this repository, as a binary keyring (<a href="jeffoffice.asc">jeffoffice.asc</a> is the same key as text). Its fingerprint is <code>$fpr</code>.</li>
<li><a href="jeffoffice.sources">jeffoffice.sources</a>: the repository entry. It says that only this key may vouch for these packages.</li>
</ul>
<h2>Remove it</h2>
<pre>sudo apt remove jeffoffice
sudo rm /etc/apt/sources.list.d/jeffoffice.sources /etc/apt/keyrings/jeffoffice.gpg</pre>
<p><a href="https://github.com/JeffOffice/jeffoffice">JeffOffice on GitHub</a></p>
</body>
</html>
HTML

echo "Published: $(find "$repo/pool" -name '*.deb' | wc -l) package file(s), signed by $fpr"
