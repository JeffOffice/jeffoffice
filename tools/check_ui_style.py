#!/usr/bin/env python3
"""Checks the code against the one dialog style (docs/design/STYLE.md).

    python3 -I tools/check_ui_style.py [-v] [--all] [--only crunch,write,shell,deck,pub,page]

Counts, per app (apps/crunch, apps/write, apps/deck, apps/pub) and per library (libs/shell, libs/page):
  groupbox   a QGroupBox with a title (use jp::Section or jp::sectionHeading)
  colon      a label (QLabel, a form row's label) whose text ends in ":" (labels have no trailing colon)
  buttons    a QDialogButtonBox built by hand (use jp::ButtonBox)
  msgbox     a QMessageBox built or called outside jp::plainBox (jp::plainBox, plainWarning, plainQuestion, plainInformation, jp::MessageBox)
A line that carries "style: reviewed" is not counted (only that line); the count of such lines is printed. The default is crunch, write, and shell (--all for every area).
The exit status is 1 when anything is counted.
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
DEFAULT_AREAS = list(AREAS)   # (every app and library)
MARK = "style: reviewed"
LIT = r'"(?:[^"\\]|\\.)*"'
# the implementation of the shared pieces themselves
OWN = ("libs/shell/app/uistyle.cpp", "libs/shell/app/uistyle.h", "libs/shell/app/plaintext.h")

# a titled box: new QGroupBox(title), new QGroupBox(title, parent), or a variable for the title (a parent is &x, this, parent, or nothing)
GROUPBOX = re.compile(r"\bnew\s+QGroupBox\s*\(\s*(?!\)|&|this\b|parent\b|parentWidget\b|nullptr\b)|\bQGroupBox\s+\w+\s*\(\s*(?:tr\(|QCoreApplication::translate|\")")
# a text, in tr(...), QT_TR_NOOP(...), or translate(ctx, ...), that ends in a colon (with an optional disambiguation or comment argument)
COLON_TEXT = re.compile(r"(?:\btr|\bQT_TR_NOOP|\btranslate\s*\(\s*" + LIT + r"\s*,)\s*\(?\s*\"(?:[^\"\\]|\\.)*:\s*\"\s*(?:,\s*" + LIT + r"\s*)*[,)]")
# ... that a label shows: a QLabel, a form row, or setText; not a question a QInputDialog asks, a prompt, or a message box
LABELISH = re.compile(r"QLabel|addRow\(|setText\(|\bnew\s+QCheckBox|->addWidget\(new")
NOT_A_LABEL = re.compile(r"QInputDialog|setLabelText|QMessageBox|MessageBox|plain(?:Box|Warning|Information|Question)|\bnotice\(|setToolTip|setStatusTip|setPlaceholderText|setWhatsThis")
BUTTONS = re.compile(r"\bnew\s+QDialogButtonBox\b|\bQDialogButtonBox\s+\w+\s*[({]|\bQDialogButtonBox\s*\*\s*\w+\s*=\s*new\s+QDialogButtonBox\b")
MSGBOX = re.compile(r"\bQMessageBox\s+\w+\s*[({]|\bnew\s+QMessageBox\b|\bQMessageBox::(?:warning|information|question|critical|about|aboutQt)\s*\(")


def files(area):
    base = os.path.join(ROOT, AREAS[area])
    for d, _, names in os.walk(base):
        for n in names:
            if n.endswith((".cpp", ".h", ".inc")):
                yield os.path.join(d, n)


def statement(text, pos):
    """The statement around `pos`: back to the previous ; { or }, forward to the next ;"""
    a = max(text.rfind(";", 0, pos), text.rfind("{", 0, pos), text.rfind("}", 0, pos)) + 1
    b = text.find(";", pos)
    return text[a:(b if b >= 0 else len(text))]


def check(path):
    """(rel, kind, line, text) for each break of the rules, and the number of reviewed lines."""
    rel = os.path.relpath(path, ROOT).replace(os.sep, "/")
    if rel in OWN:
        return [], 0
    with open(path, encoding="utf-8", errors="replace") as f:
        text = f.read()
    lines = text.split("\n")
    out, reviewed = [], 0
    seen = set()

    def add(kind, line):
        nonlocal reviewed
        s = lines[line - 1].strip()
        if s.startswith(("//", "*", "/*")):
            return
        if MARK in lines[line - 1]:
            reviewed += 1
            return
        if (kind, line) not in seen:
            seen.add((kind, line))
            out.append((rel, kind, line, s))

    for i, line in enumerate(lines):
        if GROUPBOX.search(line):
            add("groupbox", i + 1)
        if BUTTONS.search(line):
            add("buttons", i + 1)
        if MSGBOX.search(line):
            add("msgbox", i + 1)
    for m in COLON_TEXT.finditer(text):
        stmt = statement(text, m.start())
        if LABELISH.search(stmt) and not NOT_A_LABEL.search(stmt):
            add("colon", text.count("\n", 0, m.start()) + 1)
    return out, reviewed


def main(argv):
    verbose = "-v" in argv
    only = DEFAULT_AREAS
    if "--all" in argv:
        only = list(AREAS)
    if "--only" in argv:
        only = argv[argv.index("--only") + 1].split(",")
    bad = 0
    reviewed_total = 0
    for area in only:
        counts = {"groupbox": 0, "colon": 0, "buttons": 0, "msgbox": 0}
        for p in sorted(files(area)):
            found, reviewed = check(p)
            reviewed_total += reviewed
            for rel, kind, line, text in found:
                counts[kind] += 1
                if verbose:
                    print(f"{rel}:{line}: {kind}: {text[:120]}")
        total = sum(counts.values())
        bad += total
        print(f"{area}: " + " ".join(f"{k}={v}" for k, v in counts.items()) + f" total={total}")
    print(f"ui style: {bad} problem(s), {reviewed_total} reviewed line(s) ({','.join(only)})")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
