#!/usr/bin/env python3
"""Checks that every plural in the translation files can be filled in every form its language has.

    python3 tools/check_plurals.py [translations folder]      (default: translations)

Reported, with exit status 1 if there is any:

  * a string with %n in translations/jeffoffice.ts that is not marked as a plural (numerus): its call passes no count, so a
    translation could give it only one form. Russian has three forms ("1 страница", "2 страницы", "5 страниц"), Indonesian one.
  * a plural in a language's file with the wrong number of forms for that language (Russian 3, Indonesian 1, the others 2).
  * a plural with some of its forms filled in and others empty: the empty form shows nothing for the numbers that use it.
    (A plural with every form empty is not translated yet and shows in English.)
"""
import glob
import os
import sys
import xml.etree.ElementTree as ET

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# Forms per plural in Qt, for the languages that don't have two.
FORMS = {"ru": 3, "id": 1}


def messages(path):
    root = ET.parse(path).getroot()
    for ctx in root.findall("context"):
        for m in ctx.findall("message"):
            tr = m.find("translation")
            if tr is not None and tr.get("type") in ("vanished", "obsolete"):
                continue
            yield root.get("language", ""), ctx.findtext("name"), m, tr


def problems(folder):
    out = []
    for path in sorted(glob.glob(os.path.join(folder, "jeffoffice*.ts"))):
        name = os.path.basename(path)
        for lang, ctx, m, tr in messages(path):
            source = m.findtext("source") or ""
            where = f"{name} [{ctx}] {source!r}"
            plural = m.get("numerus") == "yes"
            if "%n" in source and not plural:
                out.append(f"{where}: has %n but is not marked as a plural (pass the count to tr())")
            if not plural or tr is None or not lang:
                continue
            forms = [f.text or "" for f in tr.findall("numerusform")]
            want = FORMS.get(lang.split("_")[0], 2)
            if len(forms) != want:
                out.append(f"{where}: {lang} needs {want} forms, has {len(forms)}")
            elif any(forms) and not all(f.strip() for f in forms):
                out.append(f"{where}: a plural form is empty")
    return out


def main():
    folder = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, "translations")
    found = problems(folder)
    for p in found:
        print(p)
    print(f"plurals: {len(found)} problems" if found else "plurals: ok")
    return 1 if found else 0


if __name__ == "__main__":
    sys.exit(main())
