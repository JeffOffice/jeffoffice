#!/usr/bin/env python3
"""Checks .odt files written by JeffWrite against what LibreOffice itself writes.

usage: odt_check.py <folder of files LibreOffice wrote> <folder of files to check>

Each file to check must have the mimetype first and stored, every part well formed XML with its namespaces declared, a manifest that lists every part (but itself and the
mimetype), and every file its manifest names. The elements and attributes (by namespace and name) that the files use are compared with those the LibreOffice files use: a name
that no LibreOffice file has is printed with how many files have it. Counts only; never any text.
"""
import collections, os, sys, zipfile
import xml.etree.ElementTree as ET

ref_dir, chk_dir = sys.argv[1:3]
NS_MANIFEST = "urn:oasis:names:tc:opendocument:xmlns:manifest:1.0"


def vocabulary(z):
    elements, attributes = set(), set()
    for name in ("content.xml", "styles.xml", "meta.xml"):
        if name not in z.namelist():
            continue
        for _, e in ET.iterparse(z.open(name), events=("end",)):
            elements.add(e.tag)
            for a in e.attrib:
                attributes.add((e.tag, a))
    return elements, attributes


known_e, known_a = set(), set()
for f in os.listdir(ref_dir):
    if f.endswith(".odt"):
        try:
            e, a = vocabulary(zipfile.ZipFile(os.path.join(ref_dir, f)))
            known_e |= e
            known_a |= a
        except Exception:
            pass
problems = collections.Counter()
unknown_e, unknown_a = collections.Counter(), collections.Counter()
files = [f for f in sorted(os.listdir(chk_dir)) if f.endswith(".odt")]
for f in files:
    path = os.path.join(chk_dir, f)
    try:
        z = zipfile.ZipFile(path)
        infos = z.infolist()
        if infos[0].filename != "mimetype" or infos[0].compress_type != 0:
            problems["mimetype not first and stored"] += 1
        if z.read("mimetype") != b"application/vnd.oasis.opendocument.text":
            problems["wrong mimetype"] += 1
        for name in ("content.xml", "styles.xml", "meta.xml", "META-INF/manifest.xml"):
            ET.fromstring(z.read(name))
        manifest = ET.fromstring(z.read("META-INF/manifest.xml"))
        listed = {e.attrib["{%s}full-path" % NS_MANIFEST] for e in manifest}
        for i in infos:
            if i.filename in ("mimetype", "META-INF/manifest.xml") or i.filename.endswith("/"):
                continue
            if i.filename not in listed:
                problems["part not in the manifest"] += 1
        for l in listed:
            if l != "/" and l not in z.namelist():
                problems["manifest names a missing part"] += 1
        e, a = vocabulary(z)
        for x in e - known_e:
            unknown_e[x] += 1
        for x in a - known_a:
            unknown_a[x] += 1
    except Exception as ex:
        problems["unreadable: " + type(ex).__name__] += 1
print("files", len(files), "problems", dict(problems) if problems else 0)
for k, v in unknown_e.most_common(30):
    print("element not in any LibreOffice file:", k, "in", v, "files")
for k, v in unknown_a.most_common(40):
    print("attribute not in any LibreOffice file:", k, "in", v, "files")
