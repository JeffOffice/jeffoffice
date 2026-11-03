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
- [ ] **1.0** Planned thing *(not built)*
- [ ] **first release** Another thing

## Home tab

- [x] **1.0** Direct item

### Clipboard
- [x] **1.0** `AVERAGE` *(public 12%)*, `MAX` and `MIN` *(public 5% each)*, and `COUNT` *(3%)*
- [~] **1.0** Table text with **bold** inside

### What must survive a round trip
- [x] **1.0** thrown away

### Empty group
- [ ] **later** nothing

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
        self.assertEqual(beyond, [("done", "Free and open source", "shared: libs/x"), ("partly", "Online pictures", "note (with parens) | second note"),
                                  ("todo", "Planned thing", "not built"), ("todo", "Another thing", "")])

    def test_groups(self):
        home = self.tabs[1]
        self.assertEqual(home["direct"], [("done", "Direct item", "")])
        self.assertEqual([g["h"] for g in home["groups"]], ["Clipboard", "Empty group"])
        self.assertEqual(home["groups"][0]["items"][1], ("partly", "Table text with bold inside", ""))

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

    def test_note_text(self):
        self.assertEqual(pf.item_note("**1.0** A *(done: x (y))* and B *(missing z)* **(needed for the Preview)**"), "done: x (y) | missing z")
        self.assertEqual(pf.item_note("**1.0** No note here"), "")

    def test_key_differs_for_headings(self):
        self.assertNotEqual(pf.key_of("jeffpub", "heading", "Home tab"), pf.key_of("jeffpub", "item", "Home tab"))


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


class Keys(unittest.TestCase):
    def test_same_line_in_two_apps_is_two_keys(self):
        self.assertNotEqual(pf.key_of("jeffdeck", "item", "Installs without rights"), pf.key_of("jeffcrunch", "item", "Installs without rights"))

    def test_note_changes_the_key(self):
        self.assertNotEqual(pf.key_of("jeffpub", "item", "A", "works"), pf.key_of("jeffpub", "item", "A", "missing x"))

    def test_collect_sends_app_and_note(self):
        tabs = {"jeffdeck": [{"h": "H", "direct": [("partly", "Same", "n1")], "groups": []}],
                "jeffcrunch": [{"h": "H", "direct": [("partly", "Same", "n1"), ("todo", "Later one", "")], "groups": []}]}
        w = pf.collect(tabs)
        items = [v for v in w.values() if v["kind"] != "heading"]
        self.assertEqual(sorted(v["app"] for v in items), ["jeffcrunch", "jeffcrunch", "jeffdeck"])
        self.assertEqual({v["note"] for v in items}, {"n1", ""})
        self.assertEqual({v["kind"] for v in items}, {"partial", "todo"})


class Scope(unittest.TestCase):
    ENTRY = {"app": "jeffcrunch", "text": "Installs without administrator rights", "note": "", "kind": "feature"}

    def ok(self, **kw):
        return dict({"drop": False, "en": "Installs.", "es": "Se instala.", "fr": "S’installe.", "soon": {"drop": True, "en": "", "es": "", "fr": ""}}, **kw)

    def test_other_app_name(self):
        self.assertTrue(pf.scope_problems(self.ok(en="Install JeffDeck without rights."), self.ENTRY))
        self.assertFalse(pf.scope_problems(self.ok(en="Install JeffCrunch without rights."), self.ENTRY))

    def test_other_app_allowed_when_source_names_it(self):
        e = dict(self.ENTRY, text="Paste into JeffWrite")
        self.assertFalse(pf.scope_problems(self.ok(en="Paste cells into JeffWrite."), e))

    def test_promises(self):
        for t in ["This is coming soon.", "Planned for later.", "It will be added", "Work in progress"]:
            self.assertTrue(pf.scope_problems(self.ok(en=t), self.ENTRY), t)


class Soon(unittest.TestCase):
    FLAT = {"id": 0, "works_drop": False, "en": "Opens files.", "es": "Abre archivos.", "fr": "Ouvre des fichiers.",
            "soon_drop": False, "soon_en": "Saving files", "soon_es": "Guardar archivos", "soon_fr": "Enregistrement de fichiers"}

    def test_partial_keeps_both(self):
        r = pf.normalize(self.FLAT, "partial")
        self.assertFalse(r["drop"])
        self.assertFalse(r["soon"]["drop"])
        self.assertEqual(pf.entry_problems(r), [])

    def test_feature_has_no_soon_text(self):
        r = pf.normalize(self.FLAT, "feature")
        self.assertTrue(r["soon"]["drop"])
        self.assertEqual(r["soon"]["en"], "")

    def test_todo_has_no_works_text(self):
        r = pf.normalize(self.FLAT, "todo")
        self.assertTrue(r["drop"])
        self.assertFalse(r["soon"]["drop"])

    def test_soon_text_is_checked(self):
        r = pf.normalize(dict(self.FLAT, soon_en="Export to Excel files"), "partial")
        self.assertTrue(pf.entry_problems(r))
        r = pf.normalize(dict(self.FLAT, soon_en="Saving files will be added"), "partial")
        self.assertTrue(pf.scope_problems(r, {"app": "jeffpub", "text": "x", "note": ""}))

    def test_old_entry_without_soon_is_sent_again(self):
        k = pf.key_of("jeffpub", "item", "A")
        wanted = {k: {"app": "jeffpub", "kind": "feature", "path": "p", "text": "A", "note": ""}}
        todo = {k: v for k, v in wanted.items() if k not in {k: {"drop": False}} or "soon" not in {k: {"drop": False}}[k]}
        self.assertEqual(list(todo), [k])


class Overrides(unittest.TestCase):
    OV = [{"app": "jeffpub", "source_startswith": "Runs on", "works": "JeffPub runs on Windows.", "soon": "drop"},
          {"app": "jeffpub", "source_startswith": "macOS app", "works": "Mac.", "soon": "drop", "exact": True}]

    def test_match_by_app_and_start(self):
        self.assertEqual(pf.find_override(self.OV, "jeffpub", "Runs on Linux"), {"works": "JeffPub runs on Windows.", "soon": "drop"})
        self.assertIsNone(pf.find_override(self.OV, "jeffdeck", "Runs on Linux"))

    def test_exact(self):
        self.assertIsNotNone(pf.find_override(self.OV, "jeffpub", "macOS app"))
        self.assertIsNone(pf.find_override(self.OV, "jeffpub", "macOS app (signed)"))

    def test_override_changes_the_key(self):
        tabs = {"jeffpub": [{"h": "H", "direct": [("partly", "Runs on Linux", "n")], "groups": []}]}
        with_ov = {k for k, v in pf.collect(tabs, self.OV).items() if v["kind"] != "heading"}
        without = {k for k, v in pf.collect(tabs).items() if v["kind"] != "heading"}
        self.assertNotEqual(with_ov, without)

    def test_override_wins_over_the_model(self):
        flat = {"id": 0, "works_drop": False, "en": "Model English", "es": "Modelo", "fr": "Modèle",
                "soon_drop": False, "soon_en": "Model soon", "soon_es": "x", "soon_fr": "x"}
        r = pf.normalize(flat, "partial", {"works": "Fixed English.", "soon": "drop"})
        self.assertEqual(r["en"], "Fixed English.")
        self.assertEqual(r["es"], "Modelo")           # translated by the model
        self.assertTrue(r["soon"]["drop"])
        self.assertEqual(r["soon"]["en"], "")

    def test_override_can_keep_the_models_other_part(self):
        flat = {"id": 0, "works_drop": False, "en": "Model English", "es": "a", "fr": "b",
                "soon_drop": False, "soon_en": "Model soon", "soon_es": "c", "soon_fr": "d"}
        r = pf.normalize(flat, "partial", {"soon": "Only the soon text"})
        self.assertEqual(r["en"], "Model English")
        self.assertEqual(r["soon"]["en"], "Only the soon text")

    def test_override_on_a_todo_keeps_the_translations(self):
        flat = {"id": 0, "works_drop": True, "en": "", "es": "Abre.", "fr": "Ouvre.", "soon_drop": False, "soon_en": "x", "soon_es": "y", "soon_fr": "z"}
        r = pf.normalize(flat, "todo", {"works": "Opens.", "soon": "drop"})
        self.assertFalse(r["drop"])
        self.assertEqual((r["en"], r["es"], r["fr"]), ("Opens.", "Abre.", "Ouvre."))
        self.assertEqual(pf.entry_problems(r), [])

    def test_the_shipped_overrides_load(self):
        for o in pf.load_overrides():
            self.assertTrue(o["app"] in pf.APPS and o["source_startswith"])
            for k in ("works", "soon"):
                self.assertNotIn("original", o.get(k, "").lower())
                self.assertEqual(pf.violations(o.get(k, "") if o.get(k) != "drop" else ""), [], o)


class NewBannedWords(unittest.TestCase):
    def test_words(self):
        for t in ["SmartArt diagrams", "WordArt styles", "Tell Me box", "AutoFilter buttons", "Flash Fill", "the original program",
                  "as the original writes it", "jcrunchtool converts", "jwritetool", "fuzzed readers", "no repair message",
                  "a corpus of files", "Texto de WordArt", "the original"]:
            self.assertTrue(pf.violations(t), t)

    def test_still_allowed(self):
        for t in ["Diagrams are kept", "Text styles", "Command search", "Filter buttons", "Original pictures are kept", "A toolbar"]:
            self.assertEqual(pf.violations(t), [], t)


class SoonDedupe(unittest.TestCase):
    def test_duplicates_within_an_app_are_hidden(self):
        k = pf.key_of
        tabs = {"jeffdeck": [{"h": "T", "direct": [("partly", "A", ""), ("partly", "B", ""), ("todo", "C", ""), ("done", "D", "")], "groups": []}],
                "jeffpub": [{"h": "T", "direct": [("partly", "E", "")], "groups": []}]}
        soon = lambda en: {"drop": False, "en": en, "es": en, "fr": en}
        texts = {}
        for app, x, en in [("jeffdeck", "A", "PDF export: bookmarks."), ("jeffdeck", "B", "PDF export - Bookmarks"), ("jeffdeck", "C", "Other"),
                           ("jeffdeck", "D", "Other"), ("jeffpub", "E", "PDF export: bookmarks.")]:
            texts[k(app, "item", x)] = {"drop": False, "en": "w", "es": "w", "fr": "w", "soon": soon(en)}
        data = pf.build_public(tabs, {"jeffdeck": ("beta", "1"), "jeffpub": ("beta", "1")}, {}, texts)
        deck = data["apps"]["jeffdeck"]["tabs"][0]["items"]
        self.assertEqual([bool(i.get("hide_soon")) for i in deck], [False, True, False, True])   # B repeats A; D is done
        self.assertFalse(data["apps"]["jeffpub"]["tabs"][0]["items"][0].get("hide_soon"))        # another app: not a duplicate

    def test_norm_line(self):
        self.assertEqual(pf.norm_line("PDF export: Bookmarks!"), pf.norm_line("pdf export - bookmarks"))


class Build(unittest.TestCase):
    def test_structure_and_stale_texts_dropped(self):
        tabs = {"jeffpub": [{"h": "Home tab", "direct": [("done", "A", "")], "groups": [{"h": "G", "items": [("partly", "B", "n")]}]}]}
        k = pf.key_of
        texts = {k("jeffpub", "heading", "Home tab"): {"drop": False, "en": "h", "es": "h", "fr": "h"},
                 k("jeffpub", "item", "A"): {"drop": False, "en": "a", "es": "a", "fr": "a"},
                 k("jeffpub", "item", "gone"): {"drop": False, "en": "x", "es": "x", "fr": "x"}}
        data = pf.build_public(tabs, {"jeffpub": ("beta", "0.20.0")}, {"jeffpub": ("November", "2", "2026")}, texts)
        self.assertNotIn(k("jeffpub", "item", "gone"), data["texts"])
        tab = data["apps"]["jeffpub"]["tabs"][0]
        self.assertEqual(tab["groups"][0]["items"][0]["status"], "partly")
        self.assertEqual(tab["groups"][0]["items"][0]["text"], k("jeffpub", "item", "B", "n"))
        self.assertEqual(data["apps"]["jeffpub"]["version"], "0.20.0")

    def test_prompt_rules(self):
        self.assertIn("no \"will\"", pf.SYSTEM)
        self.assertIn("only what works today", pf.SYSTEM)


if __name__ == "__main__":
    unittest.main()
