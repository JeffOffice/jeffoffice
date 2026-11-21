#!/usr/bin/env python3
"""Checks that libs/ and apps/ (and tests/) make no wildcard disconnect on a QObject.

The application's style sheet connects to each widget's destroyed signal. A disconnect that names no receiver
(`w->disconnect()`, `QObject::disconnect(w, nullptr, nullptr, nullptr)`) also cuts that connection, so the next
app-wide setStyleSheet reads a freed widget (Nov 21, 2026: PasteOffer's destructor held up release 0.44). Disconnect
only what you mean: `w->disconnect(this)`, or name the signal and the receiver.

A call is flagged when it names no receiver and no signal (every argument missing or null), or when it names the
destroyed signal and no receiver. A disconnect of one signal (`disconnect(a, &QAction::triggered, nullptr, nullptr)`),
of a connection handle (`disconnect(conn)`), or of a receiver (`w->disconnect(this)`) is fine.

A line that ends the call may carry `// disconnect-all: <reason>` for an object that is never a widget and never styled.

    python3 tools/check_disconnect.py

Prints each use with its file and line and exits 1, or prints "disconnect ok" and exits 0.
"""
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
CALL = re.compile(r"\bdisconnect\s*\(")
OPTOUT = re.compile(r"//\s*disconnect-all:\s*\S")
NULLS = {"", "nullptr", "0", "NULL", "Q_NULLPTR"}


def mask(text: str) -> str:
    """The text with comments and string contents blanked (same length, same lines), so a call in a comment is not read."""
    out, i, n = [], 0, len(text)
    while i < n:
        two = text[i:i + 2]
        if two == "//":
            j = text.find("\n", i)
            j = n if j < 0 else j
            out.append(" " * (j - i))
            i = j
        elif two == "/*":
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append("".join(c if c == "\n" else " " for c in text[i:j]))
            i = j
        elif text[i] == '"':
            j = i + 1
            while j < n and text[j] != '"' and text[j] != "\n":
                j += 2 if text[j] == "\\" else 1
            out.append('"' + "_" * (j - i - 1) + ('"' if j < n and text[j] == '"' else ""))
            i = j + 1
        else:
            out.append(text[i])
            i += 1
    return "".join(out)


def split_args(body: str) -> list:
    args, depth, cur = [], 0, []
    for c in body:
        if c in "([{":
            depth += 1
        elif c in ")]}":
            depth -= 1
        if c == "," and depth == 0:
            args.append("".join(cur).strip())
            cur = []
        else:
            cur.append(c)
    args.append("".join(cur).strip())
    return [] if args == [""] else args


def is_wildcard(call_text: str, args: list, member: bool) -> bool:
    """member: w->disconnect(signal, receiver, method) or (receiver, method); else disconnect(sender, signal, receiver, method)."""
    if not member and len(args) >= 2:
        args = args[1:]   # drop the sender: what is left reads the same as a member call's
    elif not member and len(args) == 1:
        return args[0] in NULLS   # a connection handle (or a lone null)
    if all(a in NULLS for a in args):
        return True
    names_destroyed = "destroyed" in args[0] if args else False
    # a signal (first) with no receiver after it: signal, receiver, method
    if names_destroyed and (len(args) < 2 or args[1] in NULLS):
        return True
    return False


def scan(path: pathlib.Path) -> list:
    raw = path.read_text(encoding="utf-8", errors="replace")
    text = mask(raw)
    lines = raw.splitlines()
    bad = []
    for m in CALL.finditer(text):
        start, depth, i = m.end(), 1, m.end()
        while i < len(text) and depth:
            depth += {"(": 1, ")": -1}.get(text[i], 0)
            i += 1
        if depth:
            continue
        args = split_args(text[start:i - 1])
        before = text[max(0, m.start() - 2):m.start()]
        member = before in ("->",) or before.endswith(".")
        if not is_wildcard(text[m.start():i], args, member):
            continue
        first = text.count("\n", 0, m.start())
        last = text.count("\n", 0, i)
        if any(OPTOUT.search(lines[n]) for n in range(first, min(last + 1, len(lines)))):
            continue
        bad.append(f"{path.relative_to(ROOT)}:{first + 1}: {lines[first].strip()[:120]}")
    return bad


def main() -> int:
    bad = []
    for top in ("libs", "apps", "tests"):
        for path in sorted((ROOT / top).rglob("*")):
            if path.suffix in (".cpp", ".h", ".mm", ".inc") and "third_party" not in path.parts:
                bad += scan(path)
    if bad:
        print("\n".join(bad))
        print(f"{len(bad)} problem(s): a wildcard disconnect also cuts the style sheet's hold on a widget: use w->disconnect(this), or name the signal and the receiver (or `// disconnect-all: <reason>` for an object never styled)")
        return 1
    print("disconnect ok")
    return 0


if __name__ == "__main__":
    sys.exit(main())
