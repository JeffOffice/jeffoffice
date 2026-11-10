#!/usr/bin/env python3
"""Keeps the Third-party software list on each app's About page complete.

Every part made by others that the repository carries (third_party/*, PDFium's license texts, the dictionaries, the
fonts and icons, and the license files in packaging/licenses) must name a row of the list: the rows all four apps
share (libs/shell/app/filescreenparts.cpp, addThirdPartyList) or an app's own (the extras its About page passes, as `host.extras` of the AboutHost).
A part this script doesn't know fails the check, so a new library can't ship without its row: add the row, then
add the part to PARTS below.

It also writes docs/third-party.json, the list as data, for the website's Third-party software page:
    python3 tools/check_third_party.py --write
and fails when the committed file differs from the list, so the website's copy follows the apps.
"""
import json
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SHARED = "libs/shell/app/filescreenparts.cpp"
APPS = {"JeffPub": "apps/pub/app/backstage.cpp", "JeffDeck": "apps/deck/app/deckfilescreen.cpp",
        "JeffWrite": "apps/write/app/writefilescreen.cpp", "JeffCrunch": "apps/crunch/app/crunchfilescreen.cpp"}
JSON = "docs/third-party.json"

# Each part the repository carries, by where it is, and the row of the list (a name, or the start of one) it needs.
# None: our own code or a file that names no part.
PARTS = {
    "third_party/libmspub": "libmspub", "third_party/librevenge": "librevenge", "third_party/compat": None,
    "third_party/ffmpeg": "FFmpeg", "third_party/pdfium": "PDFium",
    "third_party/qpdf": "qpdf", "third_party/qpdfwrap": None, "third_party/libjpeg_turbo": "libjpeg-turbo",
    "pdfium/abseil": "Abseil", "pdfium/agg23": "Anti-Grain Geometry", "pdfium/dragonbox-Apache2-LLVM": "dragonbox",
    "pdfium/dragonbox-Boost": "dragonbox", "pdfium/fast_float": "fast_float", "pdfium/freetype": "FreeType",
    "pdfium/harfbuzz": "HarfBuzz", "pdfium/highway": "Highway", "pdfium/icu": "ICU", "pdfium/lcms": "Little CMS", "pdfium/libjpeg_turbo": "libjpeg-turbo",
    "pdfium/libopenjpeg": "OpenJPEG", "pdfium/libpng": "libpng", "pdfium/llvm-libc": "LLVM libc",
    "pdfium/pdfium-binaries": "pdfium-binaries", "pdfium/pdfium": "PDFium", "pdfium/simdutf": "simdutf", "pdfium/zlib": "zlib",
    "dict/de": "German dictionary", "dict/en": "English dictionaries", "dict/es": "Spanish dictionaries",
    "dict/fr_FR": "French dictionary", "dict/it_IT": "Italian dictionary", "dict/nl_NL": "Dutch dictionary",
    "dict/pt_BR": "Brazilian Portuguese dictionary", "dict/pt_PT": "European Portuguese dictionary",
    "packaging/GCC-RUNTIME-LIBRARY-EXCEPTION-3.1": "MinGW-w64 runtime", "packaging/LGPL-3.0": "Qt 6",
    "packaging/LUCIDE-ISC": "Lucide icons", "packaging/MPL-2.0": "libmspub", "packaging/MPL-NOTICE": "libmspub",
    "packaging/QT-NOTICE": "Qt 6", "packaging/README": None,
    "fonts": "Open fonts", "icons": "Lucide icons",
}


def carried():
    """Every part the repository carries, as a key of PARTS."""
    keys = []
    for name in sorted(os.listdir(os.path.join(ROOT, "third_party"))):
        if os.path.isdir(os.path.join(ROOT, "third_party", name)):
            keys.append("third_party/" + name)
    for name in sorted(os.listdir(os.path.join(ROOT, "third_party/pdfium/licenses"))):
        keys.append("pdfium/" + os.path.splitext(name)[0])
    for name in sorted(os.listdir(os.path.join(ROOT, "resources/dict"))):
        if os.path.isdir(os.path.join(ROOT, "resources/dict", name)):
            keys.append("dict/" + name)
    for name in sorted(os.listdir(os.path.join(ROOT, "packaging/licenses"))):
        keys.append("packaging/" + os.path.splitext(name)[0])
    keys += ["fonts", "icons"]
    return sorted(set(keys))


STRING = r'"((?:[^"\\]|\\.)*)"'
ROW = re.compile(r'\{QStringLiteral\(' + STRING + r'\),\s*QStringLiteral\(' + STRING + r'\),\s*(?:tr|QCoreApplication::translate)\((?:"[^"]*",\s*)?'
                 + STRING + r'\),\s*QStringLiteral\(' + STRING + r'\),\s*QStringLiteral\(' + STRING + r'\)\}')


def statement(text, start):
    """The C++ statement that starts at `start`: up to the first ';' that is not inside a string or character literal."""
    i, quote = start, None
    while i < len(text):
        c = text[i]
        if quote:
            if c == "\\":
                i += 1
            elif c == quote:
                quote = None
        elif c in "\"'":
            quote = c
        elif c == ";":
            return text[start:i + 1]
        i += 1
    return text[start:]


def rows(path, windows_only_too=True):
    with open(os.path.join(ROOT, path), encoding="utf-8") as f:
        text = f.read()
    if path == SHARED:
        text = text[text.index("void addThirdPartyList"):]
    else:
        m = re.search(r"addThirdPartyList\(|host\.extras\s*=", text)
        text = statement(text, m.start()) if m else ""
    out = []
    for m in ROW.finditer(text):
        windows = "#ifdef Q_OS_WIN" in text[:m.start()] and "#endif" not in text[text.rindex("#ifdef Q_OS_WIN", 0, m.start()):m.start()] \
            if "#ifdef Q_OS_WIN" in text[:m.start()] else False
        name, url, use, lic, lic_url = (g.replace('\\"', '"') for g in m.groups())
        out.append({"name": name, "url": url, "use": use, "license": lic, "licenseUrl": lic_url, "windowsOnly": windows})
    return out


def listing():
    shared = rows(SHARED)
    if not shared:
        sys.exit("check_third_party.py: no rows found in %s; has addThirdPartyList changed shape?" % SHARED)
    out = [dict(r, apps=list(APPS)) for r in shared]
    for app, path in APPS.items():
        for r in rows(path):
            same = next((o for o in out if o["name"] == r["name"]), None)
            if same:
                same["apps"].append(app)
            else:
                out.append(dict(r, apps=[app]))
    return out


def main():
    lst = listing()
    names = [r["name"] for r in lst]
    problems = []
    for key in carried():
        if key not in PARTS:
            problems.append("%s is new: add its row to the About list (%s, or the app's own) and to PARTS in this script" % (key, SHARED))
        elif PARTS[key] and not any(n.startswith(PARTS[key]) for n in names):
            problems.append("%s is carried, but no row of the About list starts with %r" % (key, PARTS[key]))
    data = json.dumps({"about": "Third-party software in JeffOffice, from the apps' About pages (tools/check_third_party.py)",
                       "parts": lst}, indent=1, ensure_ascii=False) + "\n"
    path = os.path.join(ROOT, JSON)
    if "--write" in sys.argv:
        with open(path, "w", encoding="utf-8") as f:
            f.write(data)
    elif not os.path.exists(path) or open(path, encoding="utf-8").read() != data:
        problems.append("%s differs from the About list: run python3 tools/check_third_party.py --write" % JSON)
    for p in problems:
        print("check_third_party.py: " + p)
    print("third-party list: %d rows, %d problem(s)" % (len(lst), len(problems)))
    sys.exit(1 if problems else 0)


if __name__ == "__main__":
    main()
