#!/usr/bin/env python3
"""Tests for public_features.py that need no API: parsing, note stripping, the banned-name checker.
Run: python3 -m unittest tools/features/test_public_features.py  (or python3 tools/features/test_public_features.py)"""
import os
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import public_features as pf

LIST = """# JeffX feature list

Last reviewed November 2, 2026.

## How the tiers were chosen

- [x] **1.0** This section is skipped.

## Beyond Excel

- [x] **first release** Free and open source *(shared: libs/x)* **(needed for the Preview)**
- [~] **1.0** **(alt)** Online pictures *(note (with parens))* *(second note)*
- [ ] **later** Not there yet

## Home tab

- [x] **1.0** Direct item

### Clipboard
- [x] **1.0** `AVERAGE` *(public 12%)*, `MAX` and `MIN` *(public 5% each)*, and `COUNT` *(3%)*
- [~] **1.0** Table text with **bold** inside

### What must survive a round trip
- [x] **1.0** thrown away

### Empty group
- [ ] **1.0** nothing

## Fidelity and testing

- [x] **1.0** Skipped section
"""


class Parse(unittest.TestCase):
    def setUp(self):
        d = tempfile.mkdtemp()
        self.path = os.path.join(d, "jeffx.md")
        open(self.path, "w", encoding="utf-8").write(LIST)
        self.tabs = pf.parse_list(self.path)

    def test_sections_kept_and_skipped(self):
        self.assertEqual([t["h"] for t in self.tabs], ["Beyond Excel", "Home tab"])

    def test_statuses_and_dropped_items(self):
        beyond = self.tabs[0]["direct"]
        self.assertEqual(beyond, [("done", "Free and open source"), ("next", "Online pictures")])

    def test_groups(self):
        home = self.tabs[1]
        self.assertEqual(home["direct"], [("done", "Direct item")])
        self.assertEqual([g["h"] for g in home["groups"]], ["Clipboard", "Empty group"])
        self.assertEqual(home["groups"][0]["items"][1], ("next", "Table text with bold inside"))

    def test_unparsed_list_line_stops(self):
        open(self.path, "a", encoding="utf-8").write("\n## Tab\n- not a checkbox\n")
        with self.assertRaises(SystemExit):
            pf.parse_list(self.path)


class Clean(unittest.TestCase):
    def test_notes_anywhere_in_the_line(self):
        raw = "**1.0** `AVERAGE` *(public 12%)*, `MAX` *(5%)*, and `COUNT` *(3%)* *(done: x (y))*"
        self.assertEqual(pf.clean_item(raw), "`AVERAGE`, `MAX`, and `COUNT`")

    def test_tier_alt_needed(self):
        self.assertEqual(pf.clean_item("**first release** **1.0** **(alt)** Email **(needed for the Preview)**"), "Email")

    def test_alt_note_kept_as_text(self):
        self.assertEqual(pf.clean_item("**1.0** Pictures **(alt: Openverse search)**"), "Pictures (Openverse search)")

    def test_star_in_code_survives(self):
        self.assertIn("`*`", pf.clean_item("**1.0** wildcards `*` and `?` *(note)*"))

    def test_key_differs_for_headings(self):
        self.assertNotEqual(pf.key_of("heading", "Home tab"), pf.key_of("item", "Home tab"))
        self.assertEqual(pf.key_of("item", "x"), pf.sha("x"))


class Banned(unittest.TestCase):
    def bad(self, text):
        return bool(pf.violations(text))

    def test_names(self):
        for t in ["Opens Microsoft files", "Like Excel does", "Works with PowerPoint", "Publisher files", "In Outlook",
                  "Uses Calibri", "Cambria and Arial", "Times New Roman", "Courier New", "Segoe UI", "PivotTables",
                  "pivottable", "AutoSum", "autofit", "Format Painter", "AutoRecover", "Quick Access Toolbar",
                  "Quick Parts", "the Office suite", "a Word document", "open Access databases", "microsoft"]:
            self.assertTrue(self.bad(t), t)

    def test_other_languages(self):
        self.assertTrue(self.bad("Abre archivos de Excel"))
        self.assertTrue(self.bad("Ouvre les documents Word"))

    def test_allowed(self):
        for t in ["JeffOffice is free", "Opens `.docx`, `.xlsx`, and `.pptx` files", "Word wrap around pictures",
                  "Access to every setting", "Other office suites", "pivot tables", "Opens `.pub` files",
                  "Tablas dinámicas y hojas de cálculo", "It excels at nothing special"]:
            self.assertFalse(self.bad(t), t)

    def test_superlatives(self):
        self.assertTrue(self.bad("A seamless experience"))
        self.assertTrue(self.bad("The ultimate tool"))

    def test_entry_problems(self):
        ok = {"drop": False, "en": "Opens files.", "es": "Abre archivos.", "fr": "Ouvre des fichiers."}
        self.assertEqual(pf.entry_problems(ok), [])
        self.assertTrue(pf.entry_problems(dict(ok, fr="")))
        self.assertTrue(pf.entry_problems(dict(ok, es="Abre archivos de Excel.")))
        self.assertTrue(pf.entry_problems(dict(ok, en="**bold**")))
        self.assertEqual(pf.entry_problems({"drop": True, "en": "", "es": "", "fr": ""}), [])


class Build(unittest.TestCase):
    def test_structure_and_stale_texts_dropped(self):
        tabs = {"jeffpub": [{"h": "Home tab", "direct": [("done", "A")], "groups": [{"h": "G", "items": [("next", "B")]}]}]}
        k = pf.key_of
        texts = {k("heading", "Home tab"): {"drop": False, "en": "h", "es": "h", "fr": "h"},
                 k("item", "A"): {"drop": False, "en": "a", "es": "a", "fr": "a"},
                 k("item", "gone"): {"drop": False, "en": "x", "es": "x", "fr": "x"}}
        data = pf.build_public(tabs, {"jeffpub": ("beta", "0.20.0")}, {"jeffpub": ("November", "2", "2026")}, texts)
        self.assertNotIn(k("item", "gone"), data["texts"])
        tab = data["apps"]["jeffpub"]["tabs"][0]
        self.assertEqual(tab["groups"][0]["items"][0]["status"], "next")
        self.assertEqual(data["apps"]["jeffpub"]["version"], "0.20.0")


if __name__ == "__main__":
    unittest.main()
