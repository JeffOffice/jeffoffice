#!/usr/bin/env python3
"""Checks the code against the one dialog style (docs/design/STYLE.md).

    python3 -I tools/check_ui_style.py [-v] [--only crunch,write,shell,deck,pub,page]

Counts, per app (apps/crunch, apps/write, apps/deck, apps/pub) and per library (libs/shell, libs/page):
  groupbox   a QGroupBox with a title (use jp::Section or jp::sectionHeading)
  colon      a label (QLabel, a form row's label) whose text ends in ":" (labels have no trailing colon)
  buttons    a QDialogButtonBox built by hand (use jp::ButtonBox)
  msgbox     a QMessageBox built or called outside jp::plainBox (jp::plainBox, plainWarning, plainQuestion, plainInformation, jp::MessageBox)
A line that carries "style: reviewed" (or follows one that does) is not counted. The exit status is 1 when anything is counted in the places asked for
(default: all of them).
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
AREAS = {
    "crunch": "apps/crunch",
    "write": "apps/write",
    "deck": "apps/deck",
    "pub": "apps/pub",
    "shell": "libs/shell",
    "page": "libs/page",
}
MARK = "style: reviewed"
LIT = r'"(?:[^"\\]|\\.)*"'
# the implementation of the shared pieces themselves
OWN = ("libs/shell/app/uistyle.cpp", "libs/shell/app/uistyle.h", "libs/shell/app/plaintext.h")

GROUPBOX = re.compile(r"\bnew\s+QGroupBox\s*\(\s*(?!\)|&|this\b|parent\b|\w+\s*\))|\bQGroupBox\s+\w+\s*\(\s*(?:tr\(|QCoreApplication::translate|\")")
# a label's text: QLabel(...), a form row (addRow), or setText on a label, given a tr()/translate()/literal that ends in a colon
COLON_TEXT = re.compile(r"(?:\btr|\bQT_TR_NOOP|\btranslate\s*\(\s*" + LIT + r"\s*,)\s*\(?\s*\"(?:[^\"\\]|\\.)*:\s*\"\s*\)")
LABELISH = re.compile(r"QLabel|addRow\(|label|Label|->addWidget\(new")
BUTTONS = re.compile(r"\bnew\s+QDialogButtonBox\b|\bQDialogButtonBox\s+\w+\s*[({]|\bQDialogButtonBox\s*\*\s*\w+\s*=\s*new\s+QDialogButtonBox\b")
MSGBOX = re.compile(r"\bQMessageBox\s+\w+\s*[({]|\bnew\s+QMessageBox\b|\bQMessageBox::(?:warning|information|question|critical|about|aboutQt)\s*\(")


def files(area):
    base = os.path.join(ROOT, AREAS[area])
    for d, _, names in os.walk(base):
        for n in names:
            if n.endswith((".cpp", ".h", ".inc")):
                yield os.path.join(d, n)


def check(path):
    rel = os.path.relpath(path, ROOT).replace(os.sep, "/")
    if rel in OWN:
        return []
    out = []
    with open(path, encoding="utf-8", errors="replace") as f:
        lines = f.read().split("\n")
    for i, line in enumerate(lines):
        code = line
        if MARK in line or (i and MARK in lines[i - 1]):
            continue
        s = line.strip()
        if s.startswith(("//", "*", "/*")):
            continue
        if GROUPBOX.search(code):
            out.append(("groupbox", i + 1, s))
        if COLON_TEXT.search(code) and LABELISH.search(code):
            out.append(("colon", i + 1, s))
        if BUTTONS.search(code):
            out.append(("buttons", i + 1, s))
        if MSGBOX.search(code):
            out.append(("msgbox", i + 1, s))
    return [(rel,) + o for o in out]


def main(argv):
    verbose = "-v" in argv
    only = list(AREAS)
    if "--only" in argv:
        only = argv[argv.index("--only") + 1].split(",")
    bad = 0
    for area in only:
        counts = {"groupbox": 0, "colon": 0, "buttons": 0, "msgbox": 0}
        for p in sorted(files(area)):
            for rel, kind, line, text in check(p):
                counts[kind] += 1
                if verbose:
                    print(f"{rel}:{line}: {kind}: {text[:120]}")
        total = sum(counts.values())
        bad += total
        print(f"{area}: " + " ".join(f"{k}={v}" for k, v in counts.items()) + f" total={total}")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
