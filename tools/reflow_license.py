#!/usr/bin/env python3
"""Reflows a license text that is hard-wrapped (the GNU GPL breaks every ~76 characters) into paragraphs.

    tools/reflow_license.py LICENSE out.txt          plain text, one line per paragraph
    tools/reflow_license.py --rtf LICENSE out.rtf    rich text: centered and bold headings, spaced paragraphs
    tools/reflow_license.py --rtf LICENSE out.rtf --append ADDITIONAL-TERMS.md
                                                     the Markdown terms follow, under the heading "Additional Terms under section 7"

The Windows installer's license page shows the result, so its box wraps each paragraph once, at its own width.
Kept as they are: blank lines between blocks, centered headings, numbered section titles, lettered list items
(a), b), ...), and indented blocks such as the sample notices at the end (their lines stay separate).
"""
import re
import sys

CENTERED = 10   # a line indented this far is a centered heading
INDENTED = 4    # a block indented this far is an item or a sample


def classify(lines):
    """One blank-line-separated block -> (kind, payload). Kinds: head (list of lines), title, item, sample, para."""
    indents = [len(l) - len(l.lstrip()) for l in lines]
    if all(i >= CENTERED for i in indents):
        return "head", [l.strip() for l in lines]
    if len(lines) == 1 and re.match(r"^\s*\d+\.\s+\S.{0,60}$", lines[0]):
        return "title", lines[0].strip()
    if indents[0] >= INDENTED and re.match(r"^\s*(?:[a-z]|\d+)\)\s", lines[0]) and min(indents) >= INDENTED:
        return "item", " ".join(lines)
    if min(indents) >= INDENTED:
        return "sample", [l.rstrip() for l in lines]
    return "para", " ".join(lines)


def parse(text):
    blocks, cur = [], []
    for line in text.replace("\r\n", "\n").split("\n"):
        if line.strip():
            cur.append(line.rstrip())
        elif cur:
            blocks.append(cur)
            cur = []
    if cur:
        blocks.append(cur)
    out = []
    for b in blocks:
        kind, payload = classify(b)
        if kind in ("item", "para"):
            payload = re.sub(r"\s+", " ", payload).strip()
        out.append((kind, payload))
    return out


def parse_markdown(text):
    """ADDITIONAL-TERMS.md (one line per paragraph, no hard wraps) as blocks: its title is dropped (the caller adds its own
    heading), "## " lines and whole-bold lines are titles, "- " lines are items with a bullet, "> " lines are indented."""
    out = []
    for line in text.replace("\r\n", "\n").split("\n"):
        line = line.strip()
        if not line or line.startswith("# "):
            continue
        plain = line.replace("**", "")
        if line.startswith("## "):
            out.append(("title", plain[3:]))
        elif line.startswith("**") and line.endswith("**"):
            out.append(("title", plain))
        elif line.startswith("- "):
            out.append(("item", "- " + plain[2:]))
        elif line.startswith("> "):
            out.append(("item", plain[2:]))
        else:
            out.append(("para", plain))
    return out


def to_text(blocks):
    parts = []
    for kind, p in blocks:
        if kind == "head":
            parts.append("\n".join(p))
        elif kind == "item":
            parts.append("    " + p)
        elif kind == "sample":
            parts.append("\n".join(p))
        else:
            parts.append(p)
    return "\n\n".join(parts) + "\n"


def esc(s):
    out = []
    for ch in s:
        if ch in "\\{}":
            out.append("\\" + ch)
        elif ord(ch) > 126:
            out.append("\\u%d?" % (ord(ch) - (65536 if ord(ch) > 32767 else 0)))
        else:
            out.append(ch)
    return "".join(out)


def to_rtf(blocks):
    r = [r"{\rtf1\ansi\ansicpg1252\deff0{\fonttbl{\f0\fswiss Segoe UI;}{\f1\fmodern Consolas;}}\viewkind4\uc1\f0\fs20"]
    for kind, p in blocks:
        if kind == "head":
            r.append(r"\pard\qc\sb120\sa120\keepn\b\fs24 " + r"\line ".join(esc(x) for x in p) + r"\b0\fs20\par")
        elif kind == "title":
            r.append(r"\pard\sb200\sa100\keepn\b\fs22 " + esc(p) + r"\b0\fs20\par")
        elif kind == "item":
            r.append(r"\pard\li500\sa120 " + esc(p) + r"\par")
        elif kind == "sample":
            r.append(r"\pard\li500\sa120\f1\fs18 " + r"\line ".join(esc(x[INDENTED:]) for x in p) + r"\f0\fs20\par")
        else:
            r.append(r"\pard\sa120 " + esc(p) + r"\par")
    r.append("}")
    return "\n".join(r) + "\n"


def main(argv):
    rtf = "--rtf" in argv
    args = [a for a in argv[1:] if a != "--rtf"]
    extra = None
    if "--append" in args:
        i = args.index("--append")
        extra = args[i + 1]
        del args[i:i + 2]
    if len(args) != 2:
        sys.exit(__doc__)
    with open(args[0], encoding="utf-8") as f:
        blocks = parse(f.read())
    if extra:
        with open(extra, encoding="utf-8") as f:
            blocks += [("head", ["Additional Terms under section 7"])] + parse_markdown(f.read())
    with open(args[1], "w", encoding="ascii", newline="\r\n" if not rtf else "\n") as f:
        f.write(to_rtf(blocks) if rtf else to_text(blocks))


if __name__ == "__main__":
    main(sys.argv)
