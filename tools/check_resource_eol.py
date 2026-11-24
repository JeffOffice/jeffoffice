#!/usr/bin/env python3
"""Checks that the text files tracked under resources/ keep LF endings and start without a byte order mark.

The tables a program reads from its compiled-in resources (resources/fontwidths.txt, fontkerning.txt, functionnames.txt, icon-tags.txt, the JSON files) are
read line by line; a Windows checkout may turn their LF into CRLF (Nov 24, 2026: a new table lost its last column that way), and the loaders cope with that
(jp::textResourceLines), but the files themselves must stay LF with no mark, or a diff and a checksum differ between machines.

A file that .gitattributes marks `-text` (the dictionaries of other projects, kept byte for byte) and a binary file (one with a NUL byte) is skipped.

    python3 tools/check_resource_eol.py

Prints each file with what is wrong and exits 1, or prints "resource eol ok" and exits 0.
"""
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
BOM = b"\xef\xbb\xbf"


def tracked(folder: str) -> list:
    out = subprocess.run(["git", "ls-files", "-z", "--", folder], cwd=ROOT, check=True, capture_output=True).stdout
    return [n for n in out.decode("utf-8").split("\0") if n]


def unset_text(names: list) -> set:
    """The files .gitattributes marks `-text`."""
    out = subprocess.run(["git", "check-attr", "-z", "--stdin", "text"], cwd=ROOT, check=True, capture_output=True,
                         input="\0".join(names).encode("utf-8")).stdout
    parts = out.decode("utf-8").split("\0")
    return {parts[i] for i in range(0, len(parts) - 2, 3) if parts[i + 2] == "unset"}


def problems(names: list) -> list:
    skip = unset_text(names)
    found = []
    for name in names:
        if name in skip:
            continue
        data = (ROOT / name).read_bytes()
        if b"\0" in data:
            continue
        if data.startswith(BOM):
            found.append(f"{name}: starts with a byte order mark")
        if b"\r" in data:
            count = data.count(b"\r")
            found.append(f"{name}: has {count} carriage return(s) (CRLF or CR line endings)")
    return found


def main() -> int:
    found = problems(tracked("resources"))
    for line in found:
        print(line)
    if found:
        return 1
    print("resource eol ok")
    return 0


if __name__ == "__main__":
    sys.exit(main())
