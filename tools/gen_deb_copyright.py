#!/usr/bin/env python3
"""Writes the .deb's /usr/share/doc/jeffoffice/copyright as a machine-readable Debian copyright file (DEP-5) from docs/third-party.json.

    python3 tools/gen_deb_copyright.py [out-file]        (standard output when no file is given)

One Files/License stanza for JeffOffice's own code, then one for each component of the list that ships on Linux (the rows only Windows installs are left out), and then each license's text
once, in a standalone License paragraph. The license of a row is free text ("MPL-1.1, GPL-2.0+ or LGPL-2.1+", "MPL-2.0; patterns LGPL"); it is turned into a license expression of
short names (`or` for the alternatives, `and` for the parts). A license whose text Debian keeps in /usr/share/common-licenses is pointed to there; for any other, the paragraph says
where its terms are (the row's license address, and the texts the package carries in /usr/share/doc/jeffoffice).
tools/check_deb_copyright.py checks the list and the output.
"""
import json
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LIST = os.path.join(ROOT, "docs", "third-party.json")
DOC = "usr/share/doc/jeffoffice"
SHARE = "usr/share/jeffoffice"
HEADER = [
    ("Format", "https://www.debian.org/doc/packaging-manuals/copyright-format/1.0/"),
    ("Upstream-Name", "JeffOffice"),
    ("Upstream-Contact", "JeffOffice LLC <info@jeffoffice.com>"),
    ("Source", "https://github.com/JeffOffice/jeffoffice"),
]

# Where the files of a component are in the package, by the start of its name in the list. A component that is built into the programs has no files of its own: it is given the programs.
FILES = [
    ("English", [SHARE + "/dict/en/*"]),
    ("Spanish", [SHARE + "/dict/es/*"]),
    ("French dictionary", [SHARE + "/dict/fr_FR/*"]),
    ("German dictionary", [SHARE + "/dict/de/*"]),
    ("Italian dictionary", [SHARE + "/dict/it_IT/*"]),
    ("Dutch dictionary", [SHARE + "/dict/nl_NL/*"]),
    ("Brazilian Portuguese", [SHARE + "/dict/pt_BR/*"]),
    ("European Portuguese", [SHARE + "/dict/pt_PT/*"]),
    ("Russian dictionary", [SHARE + "/dict/ru_RU/*"]),
    ("Indonesian dictionary", [SHARE + "/dict/id/*"]),
    ("Ukrainian dictionary", [SHARE + "/dict/uk_UA/*"]),
    ("Open fonts", [SHARE + "/fonts/*"]),
    ("qpdf", [DOC + "/qpdf/*"]),
    ("libjpeg-turbo (in qpdf)", [DOC + "/libjpeg-turbo/*"]),
    ("FFmpeg", [DOC + "/ffmpeg/*"]),
]
# The license text a part of PDFium has in the package (third_party/pdfium/licenses), by the start of its name in the list.
PDFIUM = {
    "PDFium": "pdfium", "pdfium-binaries": "pdfium-binaries", "FreeType": "freetype", "HarfBuzz": "harfbuzz", "Little CMS": "lcms", "libjpeg-turbo (in PDFium)": "libjpeg_turbo",
    "OpenJPEG": "libopenjpeg", "libpng": "libpng", "Abseil": "abseil", "Anti-Grain": "agg23", "dragonbox": "dragonbox-*", "fast_float": "fast_float", "simdutf": "simdutf",
    "Highway": "highway", "LLVM libc": "llvm-libc", "ICU": "icu", "zlib": "zlib",
}
# What Debian keeps in /usr/share/common-licenses, by short name (without a trailing +).
COMMON = {
    "GPL-2.0": ("GPL-2", "GNU General Public License version 2"), "GPL-3.0": ("GPL-3", "GNU General Public License version 3"),
    "LGPL-2.0": ("LGPL-2", "GNU Library General Public License version 2"), "LGPL-2.1": ("LGPL-2.1", "GNU Lesser General Public License version 2.1"),
    "LGPL-3.0": ("LGPL-3", "GNU Lesser General Public License version 3"), "Apache-2.0": ("Apache-2.0", "Apache License version 2.0"),
    "MPL-1.1": ("MPL-1.1", "Mozilla Public License version 1.1"), "MPL-2.0": ("MPL-2.0", "Mozilla Public License version 2.0"),
}


def load(path=LIST):
    with open(path, encoding="utf-8") as f:
        return json.load(f)["parts"]


def short_name(text):
    """One short name from a license as the list words it: no spaces, no commas, no brackets ("Apache-2.0 (LLVM)" is Apache-2.0-LLVM, "patterns LGPL" is LGPL)."""
    text = re.sub(r"^(patterns|parts)\s+", "", text.strip())
    text = re.sub(r"[()']", " ", text.replace(" + ", " with "))
    text = re.sub(r"\s+with\s+", "-with-", text)
    return re.sub(r"[\s,]+", "-", text.strip()).strip("-")


def expression(license_text):
    """The license expression of a license as the list words it, and its short names in order. `;` joins parts (and); `,` and `or` join alternatives; `and` stays."""
    names, exprs = [], []
    for part in [p for p in license_text.split(";") if p.strip()]:
        alternatives = []
        for alt in re.split(r",\s*|\s+or\s+", part):
            ands = [short_name(x) for x in re.split(r"\s+and\s+", alt) if x.strip()]
            names += [n for n in ands if n and n not in names]
            if ands:
                alternatives.append(" and ".join(ands))
        exprs.append(" or ".join(alternatives))
    return ", and ".join(exprs), names


def files_of(part):
    name = part["name"]
    for start, globs in FILES:
        if name.startswith(start):
            return globs
    for start, stem in PDFIUM.items():
        if name.startswith(start):
            return ["%s/pdfium/%s" % (DOC, stem) + ("" if stem.endswith("*") else ".*")]
    return ["usr/bin/*"]   # (built into the programs)


def field(name, value):
    lines = value.split("\n")
    out = "%s: %s" % (name, lines[0])
    for line in lines[1:]:
        out += "\n " + (line if line.strip() else ".")
    return out


def paragraph(fields):
    return "\n".join(field(k, v) for k, v in fields)


def generate(parts):
    """The copyright file as text. `parts` is the list of docs/third-party.json (each needs a license; a part without one is a ValueError)."""
    for p in parts:
        if not str(p.get("license", "")).strip():
            raise ValueError("%s has no license" % p.get("name", "(no name)"))
    paras = [paragraph(HEADER)]
    own = "GPL-3.0"
    paras.append(paragraph([("Files", "*"), ("Copyright", "2026 JeffOffice LLC"), ("License", own),
                            ("Comment", "JeffOffice is free software under the GNU General Public License version 3, with the Additional Terms in /%s/ADDITIONAL-TERMS.md." % DOC)]))
    order, where = [own], {own: ""}
    for p in parts:
        if p.get("windowsOnly"):
            continue
        expr, names = expression(p["license"])
        paras.append(paragraph([("Files", "\n".join(files_of(p))), ("Copyright", "%s, see %s" % (p["name"], p["url"])), ("License", expr),
                                ("Comment", "%s. Licensed as %s." % (p["use"], p["license"]))]))
        for n in names:
            if n not in where:
                order.append(n)
                where[n] = p.get("licenseUrl", "")
    for n in order:
        base = n.rstrip("+")
        if base in COMMON:
            file, title = COMMON[base]
            later = " (or, where the license says so, a later version)" if n.endswith("+") else ""
            text = "On Debian systems, the text of the %s%s is in /usr/share/common-licenses/%s.%s" % (title, later, file, "\nThe text is also in /%s/LICENSE." % DOC if base == "GPL-3.0" else "")
        else:
            text = "The terms of this license are at %s.\nThe texts that go with the package are in /%s." % (where[n] or "the component's own web page", DOC) if where[n] else \
                "The terms of this license are in the texts that go with the package, in /%s." % DOC
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


def problems(text):
    """What is wrong with a copyright file as DEP-5 wants it: the header, a Files and License in each file stanza, a License text (a standalone paragraph) for every name used once."""
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
    used, defined = [], []
    for i, para in enumerate(paras[1:], 2):
        keys = [k for k, _ in para]
        if len(set(keys)) != len(keys):
            out.append("paragraph %d repeats a field" % i)
        d = dict(para)
        if "Files" in d:
            if not d["Files"].strip() or not d.get("Copyright", "").strip() or not d.get("License", "").strip():
                out.append("paragraph %d: a Files stanza needs Files, Copyright, and License" % i)
            used += [n for n in re.split(r"[\s,]+", re.sub(r"\b(and|or)\b", " ", d.get("License", "").split("\n")[0])) if n]
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
