#!/usr/bin/env python3
"""Checks that text which may come from a file, the clipboard, or the network is never handed to a widget that may take it for markup.

    python3 tools/check_plain_text.py [folder ...]      (default: apps libs)

A QLabel, a tooltip, a What's This text, a status tip, and a message box guess whether their text is rich text, and a guess of "yes" on text such as
<img src="\\\\host\\share\\x.png"> makes Qt read that path (on Windows a UNC path hands the person's credentials to the host) with no click.
So the text given to these calls must be one of

  * a string literal, tr(...), translate(...), QStringLiteral(...), or QString(), with (when .arg(...) is used) arguments that are also such text or numbers;
  * wrapped in jp::plainToolTip(...) (libs/shell/app/plaintext.h), or ending in .toHtmlEscaped();
  * for a QLabel: given to a label that also gets setTextFormat(Qt::PlainText) (or jp::plainLabel);
  * for a message box: shown through jp::plainInformation, jp::plainWarning, jp::plainQuestion, or jp::plainBox, never QMessageBox's own static functions,
    and a QMessageBox that is built by hand needs setTextFormat(Qt::PlainText) in the same file.

Anything else is reported as file:line, and the exit status is 1. A reviewed exception (text that is ours alone, say a number or a fixed name) carries the marker
    // plain: reviewed
on its line or the line before, with the reason after it.
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MARK = "plain: reviewed"
LIT = r'"(?:[^"\\]|\\.)*"'


def split_args(text, start):
    """The top-level arguments of the call whose '(' ends just before `start`; returns (args, end)."""
    depth, i, in_string, in_char = 1, start, False, False
    args, cur = [], start
    while i < len(text) and i - start < 4000:
        c = text[i]
        if in_string:
            if c == '\\':
                i += 1
            elif c == '"':
                in_string = False
        elif in_char:
            if c == '\\':
                i += 1
            elif c == "'":
                in_char = False
        elif c == '"':
            in_string = True
        elif c == "'" and i + 1 < len(text) and (text[i + 1] == '\\' or (i + 2 < len(text) and text[i + 2] == "'")):
            in_char = True
        elif c in '([{':
            depth += 1
        elif c in ')]}':
            depth -= 1
            if depth == 0:
                args.append(text[cur:i].strip())
                return args, i
        elif c == ',' and depth == 1:
            args.append(text[cur:i].strip())
            cur = i + 1
        i += 1
    return None, i


def top_level(expr, ch):
    """Index of the first top-level `ch` (not inside brackets or strings), or -1."""
    depth, in_string, i = 0, False, 0
    while i < len(expr):
        c = expr[i]
        if in_string:
            if c == '\\':
                i += 1
            elif c == '"':
                in_string = False
        elif c == '"':
            in_string = True
        elif c in '([{':
            depth += 1
        elif c in ')]}':
            depth -= 1
        elif depth == 0 and c == ch:
            if ch == '?' or ch == ':' and expr[i - 1:i + 1] != '::' and expr[i:i + 2] != '::':
                return i
            if ch == '+':
                return i
        i += 1
    return -1


def parts(expr, ch):
    out, depth, in_string, cur, i = [], 0, False, 0, 0
    while i < len(expr):
        c = expr[i]
        if in_string:
            if c == '\\':
                i += 1
            elif c == '"':
                in_string = False
        elif c == '"':
            in_string = True
        elif c in '([{':
            depth += 1
        elif c in ')]}':
            depth -= 1
        elif depth == 0 and c == ch and expr[i:i + 2] not in ('++', '::'):
            out.append(expr[cur:i])
            cur = i + 1
        i += 1
    out.append(expr[cur:])
    return out


SAFE_CALL = re.compile(r'^(?:[\w:]*::)?(?:tr|translate|QStringLiteral|QLatin1String|QLatin1StringView|QString::number|plainToolTip|jp::plainToolTip|QObject::tr|QString::fromLatin1)\s*\(')
NUMBER = re.compile(r'^-?[\d.]+[a-zA-Z]*$')


def safe(expr):
    e = expr.strip()
    if not e or e in ('QString()', 'QString("")', 'nullptr'):
        return True
    while e.startswith('(') and e.endswith(')') and len(parts(e[1:-1], '?')) >= 1 and balanced(e[1:-1]):
        e = e[1:-1].strip()
    q = top_level(e, '?')
    if q >= 0:
        rest = e[q + 1:]
        c = top_level(rest, ':')
        if c < 0:
            return False
        return safe(rest[:c]) and safe(rest[c + 1:])
    plus = parts(e, '+')
    if len(plus) > 1:
        return all(safe(p) for p in plus)
    if re.fullmatch(LIT, e) or NUMBER.match(e):
        return True
    if e.endswith('.toHtmlEscaped()') or e.endswith('.toHtmlEscaped().toUtf8()'):
        return True
    if e.startswith('QChar') or e.startswith("'"):
        return True
    # a safe call with .arg(...) chained on: every argument must be safe too
    m = SAFE_CALL.match(e)
    if m:
        args, end = split_args(e, m.end())
        if args is None:
            return False
        rest = e[end + 1:]
        # (the arguments of tr/translate/plainToolTip are context and count, not text to judge)
        while rest:
            r = re.match(r'^\s*\.(arg|toUpper|toLower|trimmed|simplified|left|mid|right|replace|chopped)\s*\(', rest)
            if not r:
                return False
            a, end = split_args(rest, r.end())
            if a is None:
                return False
            if r.group(1) == 'arg' and not all(safe(x) for x in a):
                return False
            rest = rest[end + 1:]
        return True
    if re.fullmatch(r'\w+(?:\(\))?', e) and e in ('tip', 'about'):
        return False
    # (a literal with .arg(...) chained on)
    m = re.match(LIT + r'\s*\.arg\s*\(', e)
    if m:
        return False
    return False


def balanced(s):
    d = 0
    for c in s:
        if c == '(':
            d += 1
        elif c == ')':
            d -= 1
            if d < 0:
                return False
    return d == 0


CALLS = [
    # (regex of the call up to its '(', index of the text argument (negative: from the end) )
    (re.compile(r'\bsetToolTip\s*\('), -1, 'tooltip'),
    (re.compile(r'\bsetStatusTip\s*\('), 0, 'status tip'),
    (re.compile(r'\bsetWhatsThis\s*\('), 0, "What's This"),
    (re.compile(r'QToolTip::showText\s*\('), 1, 'tooltip'),
    (re.compile(r'QMessageBox::(?:information|warning|critical|question|about)\s*\('), 2, 'message box'),
    (re.compile(r'\bsetInformativeText\s*\('), 0, 'message box'),
    (re.compile(r'\bsetDetailedText\s*\('), 0, 'message box'),
    (re.compile(r'QMessageBox\s+\w+\s*\('), 2, 'message box'),
]


def main():
    folders = sys.argv[1:] or ['apps', 'libs']
    sources = []
    for f in folders:
        for d, _, files in os.walk(os.path.join(ROOT, f)):
            for n in files:
                if n.endswith(('.cpp', '.h', '.inc')):
                    sources.append(os.path.join(d, n))
    texts = {}
    labels, plain = {}, {}

    def family(path):
        # (a class's members are declared in its header and used in files named like it_*.cpp: they share their names)
        return os.path.dirname(path), os.path.splitext(os.path.basename(path))[0].split('_')[0]

    for p in sources:
        with open(p, encoding='utf-8') as fh:
            texts[p] = fh.read()
        k = family(p)
        for m in re.finditer(r'\bQLabel\s*\*([^;(){}]*);', texts[p]):   # (QLabel *a = nullptr, *b; lists several)
            for n in re.finditer(r'(?:^|,)\s*\*?\s*(\w+)', m.group(1)):
                labels.setdefault(k, set()).add(n.group(1))
        for m in re.finditer(r'\bQLabel\s*\*\s*(\w+)', texts[p]):
            labels.setdefault(k, set()).add(m.group(1))
        for m in re.finditer(r'\bQLabel\s+(\w+)\s*[;(]', texts[p]):
            labels.setdefault(k, set()).add(m.group(1))
        for m in re.finditer(r'(\w+)\s*=\s*plainLabel\(|(\w+)(?:->|\.)setTextFormat\(Qt::PlainText\)|plainLabel\(\s*(\w+)\s*\)', texts[p]):
            plain.setdefault(k, set()).add(m.group(1) or m.group(2) or m.group(3))
    problems = 0
    parents = {'this', 'parent', 'p', 'w', 'page', 'dlg', 'box', 'id', 'card', 'd', 'host', 'frame', 'row', 'body', 'dialog', 'panel', 'pane', 'content', 'bar', 'root'}
    for p in sorted(sources):
        if p.endswith(os.path.join('shell', 'app', 'plaintext.h')):
            continue
        text = texts[p]
        lines = text.split('\n')
        offsets = [0]
        for l in lines:
            offsets.append(offsets[-1] + len(l) + 1)

        def line_of(pos):
            lo, hi = 0, len(lines)
            while lo < hi - 1:
                mid = (lo + hi) // 2
                if offsets[mid] <= pos:
                    lo = mid
                else:
                    hi = mid
            return lo

        def reviewed(ln):
            return MARK in lines[ln] or (ln > 0 and MARK in lines[ln - 1])

        def report(ln, what):
            nonlocal problems
            if reviewed(ln):
                return
            rel = os.path.relpath(p, ROOT)
            print(f"{rel}:{ln + 1}: {what}: {lines[ln].strip()[:110]}")
            problems += 1

        for rx, idx, what in CALLS:
            for m in rx.finditer(text):
                args, _ = split_args(text, m.end())
                ln = line_of(m.start())
                if args is None:
                    report(ln, what + ' (call not read)')
                    continue
                if what == 'message box' and rx.pattern.startswith('QMessageBox::'):
                    report(ln, 'use jp::plainInformation/plainWarning/plainQuestion')
                    continue
                if what == 'message box' and rx.pattern.startswith('QMessageBox\\s'):
                    if 'setTextFormat(Qt::PlainText)' not in text:
                        report(ln, 'a message box built by hand needs setTextFormat(Qt::PlainText)')
                    continue
                if what == 'message box':
                    if not safe(args[0]) and 'setTextFormat(Qt::PlainText)' not in text:
                        report(ln, what)
                    continue
                if not args or args == ['']:
                    continue
                arg = args[idx] if (idx >= 0 and len(args) > idx) or (idx < 0 and args) else None
                if arg is None:
                    continue
                if not safe(arg):
                    report(ln, what)
        # a tooltip given through a model or an item: plainToolTip must be near
        for m in re.finditer(r'Qt::ToolTipRole', text):
            ln = line_of(m.start())
            near = '\n'.join(lines[max(0, ln - 12):ln + 13])
            if 'plainToolTip' in near or reviewed(ln) or re.search(r'\btr\(|translate\(', lines[ln]):
                continue
            report(ln, 'tooltip role')
        # labels
        for m in re.finditer(r'new\s+QLabel\s*\(', text):
            args, _ = split_args(text, m.end())
            ln = line_of(m.start())
            if args is None or not args or args == ['']:
                continue
            if safe(args[0]) or (len(args) == 1 and (args[0] in parents or args[0].startswith('&'))):
                continue
            before = text[max(0, m.start() - 120):m.start()]
            if 'plainLabel' in before:
                continue
            nm = re.search(r'(\w+)\s*=\s*$', before)
            if nm and nm.group(1) in plain.get(family(p), ()):
                continue
            report(ln, 'QLabel')
        for m in re.finditer(r'\b(\w+)\s*(?:->|\.)\s*setText\s*\(', text):
            if m.group(1) not in labels.get(family(p), ()):
                continue
            args, _ = split_args(text, m.end())
            ln = line_of(m.start())
            if not args or args == [''] or safe(args[0]):
                continue
            if m.group(1) in plain.get(family(p), ()):
                continue
            report(ln, 'QLabel::setText')
    print(f"{problems} places to check" if problems else "plain text OK")
    return 1 if problems else 0


if __name__ == '__main__':
    sys.exit(main())
