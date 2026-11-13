#!/usr/bin/env python3
"""python3 -I tools/test_translate_languages.py : checks that translate.py knows Russian and Ukrainian (three plural forms) and Indonesian (one)."""
import os
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "translate"))
import translate as T


class Languages(unittest.TestCase):
    def test_the_new_languages_are_known(self):
        self.assertEqual(T.LANGUAGES["ru"], "Russian")
        self.assertEqual(T.LANGUAGES["id"], "Indonesian")
        self.assertEqual(T.LANGUAGES["uk"], "Ukrainian")
        self.assertEqual([T.forms_in(l) for l in ("de", "ru", "id", "uk")], [2, 3, 1, 3])

    def test_a_plural_needs_its_languages_forms(self):
        self.assertEqual(T.check("%n page(s)", ["%n а", "%n б", "%n в"], True, 3), "")
        self.assertIn("exactly 3", T.check("%n page(s)", ["%n а", "%n б"], True, 3))
        self.assertEqual(T.check("%n page(s)", ["%n halaman"], True, 1), "")
        self.assertIn("exactly 1", T.check("%n page(s)", ["%n a", "%n b"], True, 1))
        # Only a two-form language may spell the number out in its singular.
        self.assertEqual(T.check("%n page(s)", ["one page", "%n pages"], True, 2), "")
        self.assertNotEqual(T.check("%n page(s)", ["одна страница", "%n страницы", "%n страниц"], True, 3), "")

    def test_released_languages_are_kept_up(self):
        # Russian, Indonesian, and Ukrainian were released in 0.27.1: the nightly run (no language named) keeps them up like the rest.
        for lang in ("ru", "id", "uk", "de"):
            self.assertTrue(os.path.exists(T.ts_path(lang)), lang)
            self.assertIn(lang, T.default_languages())
        self.assertFalse({"ru", "id", "uk"} & T.HELD)

    def test_a_held_language_is_not_translated_unless_named(self):
        # A language added later starts held: the nightly run must not start paying for it before it is approved.
        held = T.HELD
        try:
            T.HELD = {"ru"}
            self.assertNotIn("ru", T.default_languages())
            self.assertIn("de", T.default_languages())
        finally:
            T.HELD = held

    def test_the_program_holds_the_same_languages(self):
        root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        with open(os.path.join(root, "libs", "shell", "app", "i18n.cpp"), encoding="utf-8") as f:
            line = [l for l in f if "s_held{" in l][0]
        import re
        self.assertEqual(set(re.findall(r'QStringLiteral\("(\w+)"\)', line)), T.HELD)

    def test_the_prompts_name_the_forms(self):
        self.assertIn("exactly 3 of them", T.ui_system("ru", {}))
        self.assertIn("exactly 1 of them", T.ui_system("id", {}))
        self.assertIn("exactly 3 of them", T.ui_system("uk", {}))
        self.assertIn("exactly 2 of them", T.ui_system("de", {}))


if __name__ == "__main__":
    unittest.main()
