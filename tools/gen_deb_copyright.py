#!/usr/bin/env python3
"""Writes the .deb's /usr/share/doc/jeffoffice/copyright as a machine-readable Debian copyright file (DEP-5) from docs/third-party.json.

    python3 tools/gen_deb_copyright.py [out-file]        (standard output when no file is given)

Files: in DEP-5 names paths of the source tree, so each component is given the real paths it has in this repository (COMPONENTS below); the first stanza is `Files: *` (JeffOffice, GPL-3+), and
later stanzas override it for what they name. A component with no path in the source (Qt, which is linked, and what is downloaded at build time) is named in the Comment of the first stanza. The
license of a stanza is written with Debian short names; `or` joins alternatives and `and` joins the licenses of different parts of one component, never a comma of the list's free text.
Each short name has one standalone License paragraph: a license Debian keeps in /usr/share/common-licenses is pointed to there; for any other, the paragraph says where its terms are.
A row of the list that COMPONENTS does not know, a row with no license, or a short name with no License paragraph is an error (tools/check_deb_copyright.py).
"""
import glob
import json
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LIST = os.path.join(ROOT, "docs", "third-party.json")
DOC = "usr/share/doc/jeffoffice"
HEADER = [
    ("Format", "https://www.debian.org/doc/packaging-manuals/copyright-format/1.0/"),
    ("Upstream-Name", "JeffOffice"),
    ("Upstream-Contact", "JeffOffice LLC <info@jeffoffice.com>"),
    ("Source", "https://github.com/JeffOffice/jeffoffice"),
]
D = "resources/dict/"
P = "third_party/pdfium/licenses/"
# The rows of docs/third-party.json by the start of their name: the stanzas each makes (source paths, then the license), or None for a component that has no path in the source (named in the main
# stanza's Comment). A path is a glob of DEP-5 (`*` and `?`); no two stanzas share one, and each matches something in the repository (the check looks).
COMPONENTS = [
    ("Qt 6", None, "LGPL-3"),
    ("Hunspell", None, "MPL-1.1 or GPL-2+ or LGPL-2.1+"),
    ("English dictionaries", [[D + "en/en_*", D + "en/README_en_*", D + "en/license.txt", D + "en/README_lightproof_en.txt"], "SCOWL and LGPL"]),
    ("English hyphenation", [[D + "en/hyph_en_*", D + "en/README_hyph_en_*"], "BSD-style"]),
    ("WordNet", [[D + "en/th_en_US_v2.dat", D + "en/WordNet_license.txt"], "WordNet"]),
    ("Spanish", [[D + "es/*"], "GPL-3+ or LGPL-3+ or MPL-1.1+"]),
    ("French", [[D + "fr_FR/*"], "MPL-2.0 and LGPL"]),
    ("German", [[D + "de/*"], "GPL-2 or GPL-3, and LGPL-2+"]),
    ("Italian", [[D + "it_IT/*"], "GPL-3 and LGPL"]),
    ("Dutch", [[D + "nl_NL/*"], "BSD-style or CC-BY-3.0"]),
    ("Brazilian", [[D + "pt_BR/*"], "LGPL-3 or MPL-1.1"]),
    ("European Portuguese", [[D + "pt_PT/*"], "GPL-2+"]),
    ("Russian", [[D + "ru_RU/*"], "BSD-style and LPPL-1.2+"]),
    ("Indonesian", [[D + "id/*"], "LGPL-3"]),
    ("Ukrainian", [[D + "uk_UA/*"], "MPL-1.1 and GPL-2+"]),
    ("Lucide", [["resources/icons/*"], "ISC"]),
    # (the first stanza of the fonts is for all of them; the stanzas after it, for the families that are under another license, override it)
    ("Open fonts", [["resources/fonts/*"], "OFL-1.1"],
     [["resources/fonts/RobotoSlab-*", "resources/fonts/PermanentMarker-*", "resources/fonts/Satisfy-*"], "Apache-2.0"],
     [["resources/fonts/TeXGyre*"], "GUST-Font-License"],
     [["resources/fonts/DejaVu*"], "Bitstream-Vera"],
     [["resources/fonts/LiberationSansNarrow-*"], "GPL-2 with Font-exception"]),
    ("zlib", [[P + "zlib.*"], "Zlib"]),
    ("PDFium", [[P + "pdfium.*"], "BSD-3-clause and Apache-2.0"]),
    ("qpdf", [["third_party/qpdf/*"], "Apache-2.0"]),
    ("libjpeg-turbo (in qpdf)", [["third_party/libjpeg_turbo/*"], "IJG and BSD-3-clause"]),
    ("pdfium-binaries", [[P + "pdfium-binaries.*"], "Expat"]),
    ("FreeType", [[P + "freetype.*"], "FTL"]),
    ("HarfBuzz", [[P + "harfbuzz.*"], "Old-MIT"]),
    ("Little CMS", [[P + "lcms.*"], "Expat"]),
    ("libjpeg-turbo (in PDFium)", [[P + "libjpeg_turbo.*"], "IJG and BSD-3-clause"]),
    ("OpenJPEG", [[P + "libopenjpeg.*"], "BSD-2-clause"]),
    ("libpng", [[P + "libpng.*"], "libpng-2.0"]),
    ("Abseil", [[P + "abseil.*"], "Apache-2.0"]),
    ("Anti-Grain", [[P + "agg23.*"], "AGG-2.3"]),
    ("dragonbox", [[P + "dragonbox-*"], "Apache-2.0 with LLVM-exception or BSL-1.0"]),
    ("fast_float", [[P + "fast_float.*"], "Expat"]),
    ("simdutf", [[P + "simdutf.*"], "Expat"]),
    ("Highway", [[P + "highway.*"], "Apache-2.0"]),
    ("LLVM libc", [[P + "llvm-libc.*"], "Apache-2.0 with LLVM-exception"]),
    ("ICU", [[P + "icu.*"], "Unicode-3.0"]),
    ("libmspub", [["third_party/libmspub/*"], "MPL-2.0"]),
    ("librevenge", [["third_party/librevenge/*"], "MPL-2.0 or LGPL-2.1+"]),
    ("LibreTranslate", None, "AGPL-3"),
    ("PSO Coated", None, "ECI-terms"),
    ("ISBN range", None, "ISBN-Agency-terms"),
    ("FFmpeg", [["third_party/ffmpeg/*"], "LGPL-2.1+"]),
]
# Debian keeps these in /usr/share/common-licenses, by short name without a trailing +.
COMMON = {
    "GPL-2": ("GPL-2", "GNU General Public License version 2"), "GPL-3": ("GPL-3", "GNU General Public License version 3"),
    "LGPL-2": ("LGPL-2", "GNU Library General Public License version 2"), "LGPL-2.1": ("LGPL-2.1", "GNU Lesser General Public License version 2.1"),
    "LGPL-3": ("LGPL-3", "GNU Lesser General Public License version 3"), "Apache-2.0": ("Apache-2.0", "Apache License version 2.0"),
    "MPL-1.1": ("MPL-1.1", "Mozilla Public License version 1.1"), "MPL-2.0": ("MPL-2.0", "Mozilla Public License version 2.0"),
}
# The others: where the terms are.
TERMS = {
    "OFL-1.1": "the SIL Open Font License version 1.1, in resources/fonts/OFL.txt", "GUST-Font-License": "the GUST Font License, in resources/fonts/GUST-FONT-LICENSE.txt",
    "Bitstream-Vera": "the Bitstream Vera and DejaVu license, in resources/fonts/DejaVu-LICENSE.txt",
    "Font-exception": "the font exception of the GNU General Public License (fonts that use these may be embedded in documents without those becoming covered by it), in resources/fonts/LiberationSansNarrow-LICENSE.txt",
    "SCOWL": "the SCOWL license, in resources/dict/en/license.txt", "WordNet": "the WordNet license, in resources/dict/en/WordNet_license.txt",
    "LGPL": "a version of the GNU Lesser General Public License that the README of the dictionary names (a copy of the versions is in /usr/share/common-licenses)",
    "BSD-style": "a BSD-style license, in the README of the dictionary or patterns (resources/dict/<language>/)", "CC-BY-3.0": "the Creative Commons Attribution 3.0 license, at https://creativecommons.org/licenses/by/3.0/",
    "LPPL-1.2+": "the LaTeX Project Public License version 1.2 or later, at https://www.latex-project.org/lppl/",
    "ISC": "the ISC license, in packaging/licenses/LUCIDE-ISC.txt", "Zlib": "the zlib license, in third_party/pdfium/licenses/zlib.txt",
    "BSD-3-clause": "the BSD 3-clause license, in the license texts of the component (third_party/pdfium/licenses, third_party/libjpeg_turbo/licenses)",
    "BSD-2-clause": "the BSD 2-clause license, in third_party/pdfium/licenses/libopenjpeg.txt", "Expat": "the Expat (MIT) license, in the license text of the component (third_party/pdfium/licenses)",
    "IJG": "the Independent JPEG Group license, in third_party/libjpeg_turbo/licenses/README.ijg", "FTL": "the FreeType License, in third_party/pdfium/licenses/freetype.txt",
    "Old-MIT": "the old MIT license of HarfBuzz, in third_party/pdfium/licenses/harfbuzz.txt", "libpng-2.0": "the libpng license version 2, in third_party/pdfium/licenses/libpng.txt",
    "AGG-2.3": "the Anti-Grain Geometry 2.3 license, in third_party/pdfium/licenses/agg23.txt", "LLVM-exception": "the LLVM exception to the Apache License 2.0, in third_party/pdfium/licenses/llvm-libc.txt",
    "BSL-1.0": "the Boost Software License 1.0, in third_party/pdfium/licenses/dragonbox-Boost.txt", "Unicode-3.0": "the Unicode License v3, in third_party/pdfium/licenses/icu.txt",
    "AGPL-3": "the GNU Affero General Public License version 3, at https://www.gnu.org/licenses/agpl-3.0.html",
    "ECI-terms": "the terms of the European Color Initiative for the PSO Coated v3 profile, at https://www.eci.org/",
    "ISBN-Agency-terms": "the terms of the International ISBN Agency for its range table, at https://www.isbn-international.org/range_file_generation",
}


def load(path=LIST):
    with open(path, encoding="utf-8") as f:
        return json.load(f)["parts"]


def entry_of(name):
    for entry in COMPONENTS:
        if name.startswith(entry[0]):
            return entry
    return None


def stanzas_of(entry):
    """The (globs, license) stanzas of a COMPONENTS entry: none for a component with no path in the source."""
    if entry[1] is None:
        return []
    return [e for e in entry[1:] if isinstance(e, list)]


def names_of(expression):
    """The short names a license expression uses: `or`, `and`, `with`, and commas join them."""
    return [n for n in re.split(r"[\s,]+", re.sub(r"\b(and|or|with)\b", " ", expression)) if n]


def field(name, value):
    lines = value.split("\n")
    out = "%s: %s" % (name, lines[0])
    for line in lines[1:]:
        out += "\n " + (line if line.strip() else ".")
    return out


def paragraph(fields):
    return "\n".join(field(k, v) for k, v in fields)


def generate(parts):
    """The copyright file as text. `parts` is the list of docs/third-party.json: each needs a license and a COMPONENTS entry."""
    own, paras, loose, used = "GPL-3+", [], [], []
    stanzas = []
    for p in parts:
        if not str(p.get("license", "")).strip():
            raise ValueError("%s has no license" % p.get("name", "(no name)"))
        if p.get("windowsOnly"):
            continue
        entry = entry_of(p["name"])
        if entry is None:
            raise ValueError("%s is not in COMPONENTS of tools/gen_deb_copyright.py: give it its source paths and license" % p["name"])
        if entry[1] is None:
            loose.append("%s (%s; %s)" % (p["name"], entry[2], p["use"]))
            used.append(entry[2])
            continue
        for k, (globs, expr) in enumerate(stanzas_of(entry)):
            note = "%s. Listed as: %s." % (p["use"], p["license"]) if k == 0 else "A family of %s under another license." % p["name"]
            stanzas.append(paragraph([("Files", "\n".join(globs)), ("Copyright", "%s, see %s" % (p["name"], p["url"])), ("License", expr), ("Comment", note)]))
            used.append(expr)
    comment = "JeffOffice is free software under the GNU General Public License version 3 or later, with the Additional Terms in /%s/ADDITIONAL-TERMS.md. Components made by others are the stanzas below; later stanzas override this one for the files they name." % DOC
    if loose:
        comment += "\nNot in the source tree (linked, or fetched when it is built): " + "; ".join(loose) + "."
    paras = [paragraph(HEADER), paragraph([("Files", "*"), ("Copyright", "2026 JeffOffice LLC"), ("License", own), ("Comment", comment)])] + stanzas
    order = []
    for expr in [own] + used:
        for n in names_of(expr):
            if n not in order:
                order.append(n)
    for n in order:
        base = n.rstrip("+")
        if base in COMMON:
            file, title = COMMON[base]
            later = " (or, where the license says so, a later version)" if n.endswith("+") else ""
            text = "On Debian systems, the text of the %s%s is in /usr/share/common-licenses/%s." % (title, later, file)
            if base == "GPL-3":
                text += "\nThe text is also in /%s/LICENSE." % DOC
        elif n in TERMS:
            text = "The terms are %s." % TERMS[n]
        elif base in TERMS:
            text = "The terms are %s." % TERMS[base]
        else:
            raise ValueError("the license %s has no text in TERMS or COMMON of tools/gen_deb_copyright.py" % n)
        paras.append(paragraph([("License", "%s\n%s" % (n, text))]))
    return "\n\n".join(paras) + "\n"


def parse(text):
    """The paragraphs of a DEP-5 file as lists of (field, value); ValueError for text that is not shaped as one."""
    paras = []
    for block in re.split(r"\n\s*\n", text.strip("\n")):
        fields = []
        for line in block.split("\n"):
            if line.startswith("#"):
                continue
            if line[:1] in (" ", "\t"):
                if not fields:
                    raise ValueError("a continuation line starts a paragraph: %r" % line)
                fields[-1][1] += "\n" + line[1:]
            else:
                m = re.match(r"^([A-Za-z][A-Za-z0-9-]*):[ \t]*(.*)$", line)
                if not m:
                    raise ValueError("not a field: %r" % line)
                fields.append([m.group(1), m.group(2)])
        paras.append([(k, v) for k, v in fields])
    return paras


def problems(text, root=None):
    """What is wrong with a copyright file as DEP-5 wants it: the header, `Files: *` first, a Files, Copyright, and License in each file stanza, no glob in two stanzas, a License paragraph (once) for every
    short name used and none for a name that is not, and (with `root`, the source tree) a path that matches nothing in it."""
    out = []
    try:
        paras = parse(text)
    except ValueError as e:
        return [str(e)]
    if not paras or paras[0][0][0] != "Format":
        return ["the first paragraph is not the header (Format first)"]
    header = dict(paras[0])
    for key in ("Format", "Upstream-Name", "Source"):
        if not header.get(key):
            out.append("the header has no %s" % key)
    if len(paras) < 2 or dict(paras[1]).get("Files", "").strip() != "*":
        out.append("the first stanza after the header is not `Files: *`")
    used, defined, globs = [], [], {}
    # (the components that have no path in the source are named in the main stanza's Comment as "Name (license; use)": the licenses are used there)
    for m in re.finditer(r"; ([^;]*?) \(([^;()]+); ", "; " + dict(paras[1]).get("Comment", "")) if len(paras) > 1 else []:
        used += names_of(m.group(2))
    for i, para in enumerate(paras[1:], 2):
        keys = [k for k, _ in para]
        if len(set(keys)) != len(keys):
            out.append("paragraph %d repeats a field" % i)
        d = dict(para)
        if "Files" in d:
            if not d["Files"].strip() or not d.get("Copyright", "").strip() or not d.get("License", "").strip():
                out.append("paragraph %d: a Files stanza needs Files, Copyright, and License" % i)
            used += names_of(d.get("License", "").split("\n")[0])
            for g in d["Files"].split():
                if g in globs:
                    out.append("the path %s is in two stanzas (%d and %d)" % (g, globs[g], i))
                globs[g] = i
                if g.startswith("usr/") or g.startswith("/"):
                    out.append("the path %s is not a path of the source tree" % g)
                elif root and g != "*" and not glob.glob(os.path.join(root, g)):
                    out.append("the path %s matches nothing in the source tree" % g)
        elif keys[:1] == ["License"]:
            name, _, body = d["License"].partition("\n")
            if not name.strip() or " " in name.strip() or not body.strip():
                out.append("paragraph %d: a License paragraph is a short name, then its text" % i)
            if name in defined:
                out.append("the license %s has its text twice" % name)
            defined.append(name)
        else:
            out.append("paragraph %d is neither a Files stanza nor a License paragraph" % i)
    for n in sorted(set(used) - set(defined)):
        out.append("the license %s has no text" % n)
    for n in sorted(set(defined) - set(used)):
        out.append("the text of the license %s is not used" % n)
    return out


def main():
    text = generate(load())
    if len(sys.argv) > 1:
        with open(sys.argv[1], "w", encoding="utf-8") as f:
            f.write(text)
    else:
        sys.stdout.write(text)


if __name__ == "__main__":
    main()
