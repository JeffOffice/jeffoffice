#!/usr/bin/env python3
"""Checks the calls that mark text for translation, so that the nightly lupdate (.github/workflows/translate.yml) sees all of it and finishes.

    python3 tools/check_translate_calls.py [folder ...]      (default: apps libs)

Four things are reported, and the exit status is 1 if there is any:

  * a tr() or translate() call with a template argument (or a comparison) after its text, such as
        QCoreApplication::translate("Ctx", "%n cell(s)", nullptr, int(std::min<qint64>(a, b)))
    which makes lupdate loop for ever on the file. Compute the number first and pass the variable.
  * a function or lambda that only forwards its text to translate(), such as
        QString say(const char *text) { return QCoreApplication::translate("Ctx", text); }
    whose callers' texts lupdate never sees (or sees under another context). Call
    QCoreApplication::translate("Ctx", "text") with the text in the call, or mark the text where it is written with QT_TRANSLATE_NOOP("Ctx", "text").
  * in a header, an enum with its own underlying type (enum class Kind : quint8 { ... }) above a class with Q_OBJECT. Qt 6.8's lupdate
    misreads it and files the class's tr() texts under its bare name ("EditSession", not "jp::crunch::EditSession"), where the program
    never looks: they show in English in every language. Drop the underlying type, or move the enum into the class.
  * a QT_TR_NOOP outside a member function, such as in a table of names at the top of a file. lupdate files it under the namespace
    ("jp"), not the class whose tr() reads it. Write QT_TRANSLATE_NOOP("jp::CrunchWindow", "text") with the context the reader uses.
"""
import os
import re
import sys

CALL = re.compile(r'\b(?:translate|tr)\s*\(')
LIT = r'"(?:[^"\\]|\\.)*"'
TYPED_ENUM = re.compile(r'^\s*enum\b[^;{(]*:\s*[\w:]+\s*\{', re.M)
TOP_LINE = re.compile(r'^[A-Za-z_~]')
MEMBER = re.compile(r'\b\w+::~?\w+\s*\(')
WRAPPER = re.compile(r'(?:QString\s+(\w+)\(const char \*(\w+)\)\s*\{|auto\s+(\w+)\s*=\s*\[\]\(const char \*(\w+)\)\s*\{)\s*return\s+QCoreApplication::translate\(\s*' + LIT + r'\s*,\s*(\w+)\s*\)')


def call_args(text, start):
    depth, i, in_string = 1, start, False
    while i < len(text) and depth and i - start < 3000:
        c = text[i]
        if in_string:
            if c == '\\':
                i += 1
            elif c == '"':
                in_string = False
        elif c == '"':
            in_string = True
        elif c == '(':
            depth += 1
        elif c == ')':
            depth -= 1
        i += 1
    return text[start:i - 1]


def main():
    folders = sys.argv[1:] or ['apps', 'libs']
    problems = 0
    for folder in folders:
        for here, _, names in os.walk(folder):
            for name in names:
                if not name.endswith(('.cpp', '.h')):
                    continue
                path = os.path.join(here, name)
                text = open(path, encoding='utf-8', errors='replace').read()
                for m in CALL.finditer(text):
                    bare = re.sub(LIT, '""', call_args(text, m.end()))
                    bare = bare.replace('->', '')
                    if '<' in bare or '>' in bare:
                        print('%s:%d: a template argument or comparison in a translate() call (lupdate loops on it)' % (path, text.count('\n', 0, m.start()) + 1))
                        problems += 1
                for m in WRAPPER.finditer(text):
                    function, parameter = (m.group(1), m.group(2)) if m.group(1) else (m.group(3), m.group(4))
                    if m.group(5) != parameter:
                        continue
                    # (only a caller that writes a bare text hides it: one that marks it with QT_TRANSLATE_NOOP, as a table of names does, is gathered where it is marked)
                    bare = re.search(r'(?<![\w.>:~])' + re.escape(function) + r'\(\s*"', text)
                    if bare:
                        print('%s:%d: %s() only forwards its text to translate(), so lupdate does not see the texts of its callers' % (path, text.count('\n', 0, bare.start()) + 1, function))
                        problems += 1
                if name.endswith('.h') and 'Q_OBJECT' in text:
                    for m in TYPED_ENUM.finditer(text, 0, text.find('Q_OBJECT')):
                        print('%s:%d: an enum with its own type above a Q_OBJECT class (lupdate then files the class\'s texts under a name the program never looks up)' % (path, text.count('\n', 0, m.start()) + 1))
                        problems += 1
                lines = text.split('\n')
                for i, line in enumerate(lines):
                    if 'QT_TR_NOOP(' not in line:
                        continue
                    # (the declaration it is in starts at the last line that starts at the margin: a member function's says Class::name()
                    j = i
                    while j >= 0 and not (TOP_LINE.match(lines[j]) and not lines[j].startswith(('namespace', 'using', '#'))):
                        j -= 1
                    if j < 0 or not MEMBER.search(lines[j]):
                        print('%s:%d: QT_TR_NOOP outside a member function (lupdate files it under the namespace: use QT_TRANSLATE_NOOP with the reader\'s context)' % (path, i + 1))
                        problems += 1
    print('%d problem(s)' % problems)
    return 1 if problems else 0


if __name__ == '__main__':
    sys.exit(main())
