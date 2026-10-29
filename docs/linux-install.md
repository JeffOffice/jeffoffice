# Installing JeffOffice on Linux

## Debian, Ubuntu, and their relatives: our apt repository

Add the repository once, and apt installs JeffOffice and keeps it up to date.

- **Needs:** Ubuntu 22.04 or newer, or Debian 12 or newer (any system with glibc 2.35 or newer), on a 64-bit Intel or AMD computer (amd64).
- **Size:** about 100 MB to download, about 320 MB once installed.
- **Hosting:** the repository is hosted on GitHub Pages, which suits early users. It serves the newest version only (older ones stay on the [Releases page](https://github.com/JeffOffice/jeffoffice/releases)), and it has a monthly traffic limit.

```sh
sudo apt install wget          # skip this if wget is already installed
sudo install -d -m 0755 /etc/apt/keyrings
sudo wget -O /etc/apt/keyrings/jeffoffice.gpg https://jeffoffice.github.io/apt/jeffoffice.gpg
sudo wget -O /etc/apt/sources.list.d/jeffoffice.sources https://jeffoffice.github.io/apt/jeffoffice.sources
sudo apt update
sudo apt install jeffoffice
```

`jeffoffice.gpg` is the public key that signs the repository, as a binary keyring (`jeffoffice.asc` is the same key as text). Check it before you trust it:

```sh
gpg --show-keys --with-fingerprint /etc/apt/keyrings/jeffoffice.gpg
```

The fingerprint is:

```
141F 304F 0AD9 E63E DB9D  4444 63DA A97B ACDF 2F2A
```

`jeffoffice.sources` is the repository entry:

```
Types: deb
URIs: https://jeffoffice.github.io/apt
Suites: stable
Components: main
Architectures: amd64
Signed-By: /etc/apt/keyrings/jeffoffice.gpg
```

`Signed-By` limits the key to this repository, so it can't vouch for packages from anywhere else. Later versions arrive with `sudo apt update && sudo apt upgrade`; the apps do not offer downloads of their own on an apt install.

### Remove it

```sh
sudo apt remove jeffoffice
sudo rm /etc/apt/sources.list.d/jeffoffice.sources /etc/apt/keyrings/jeffoffice.gpg
sudo apt update
```

## A single .deb or an AppImage

Each release page also has `jeffoffice_<version>_amd64.deb` (`sudo apt install ./jeffoffice_<version>_amd64.deb`) and one AppImage per app, for other distributions. These don't update through apt.

## For the maintainers: publishing

`tools/apt/publish.sh [-n KEEP] <repo dir> <file.deb>...` adds packages to the repository layout (`pool/`, `dists/stable/main/binary-amd64/`, and `by-hash/` copies of the indexes), keeps the newest KEEP versions (default 1), signs the indexes with the key in `APT_SIGNING_KEY` or `APT_SIGNING_KEY_FILE`, and writes the key, the `.sources` file, and the page at the top. The signing key is never stored in a repository.

The `apt` workflow runs it for a release tag. Run it only after that release's files are final: it takes the `.deb` attached to the tag at that moment. Running it again for a tag rebuilds the index from the release's current `.deb` and signs it again. The packages themselves are too big for git (over 100 MB), so `JeffOffice/apt` holds only the indexes, and its `deploy.yml` fetches the `.deb` from the release.
