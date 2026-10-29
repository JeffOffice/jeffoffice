# Installing JeffOffice on Linux

## Debian, Ubuntu, and their relatives: our apt repository

Add the repository once, and apt installs JeffOffice and keeps it up to date. It is for 64-bit Intel and AMD computers (amd64).

```sh
sudo install -d -m 0755 /etc/apt/keyrings
sudo curl -fsSL https://jeffoffice.github.io/apt/jeffoffice.gpg -o /etc/apt/keyrings/jeffoffice.gpg
sudo curl -fsSL https://jeffoffice.github.io/apt/jeffoffice.sources -o /etc/apt/sources.list.d/jeffoffice.sources
sudo apt update
sudo apt install jeffoffice
```

`jeffoffice.gpg` is the public key that signs the repository, as a binary keyring (`jeffoffice.asc` is the same key as text). `jeffoffice.sources` is the repository entry:

```
Types: deb
URIs: https://jeffoffice.github.io/apt
Suites: stable
Components: main
Architectures: amd64
Signed-By: /etc/apt/keyrings/jeffoffice.gpg
```

`Signed-By` limits the key to this repository, so it can't vouch for packages from anywhere else. Later versions arrive with `sudo apt update && sudo apt upgrade`.

To remove it: `sudo apt remove jeffoffice`, then delete `/etc/apt/sources.list.d/jeffoffice.sources` and `/etc/apt/keyrings/jeffoffice.gpg`.

## A single .deb or an AppImage

Each release page also has `jeffoffice_<version>_amd64.deb` (`sudo apt install ./jeffoffice_<version>_amd64.deb`) and one AppImage per app, for other distributions.

## For the maintainers: publishing

`tools/apt/publish.sh [-n KEEP] <repo dir> <file.deb>...` adds packages to the repository layout (`pool/`, `dists/stable/main/binary-amd64/`), keeps the newest KEEP versions (default 3), signs the indexes with the key in `APT_SIGNING_KEY` or `APT_SIGNING_KEY_FILE`, and writes the key, the `.sources` file, and the page at the top. The `apt` workflow runs it for a release tag. The signing key is never stored in a repository.
