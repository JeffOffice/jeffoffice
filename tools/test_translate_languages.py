#!/usr/bin/env python3
"""python3 -I tools/test_translate_languages.py : checks that translate.py knows Russian (three plural forms) and Indonesian (one)."""
import os
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "translate"))
import translate as T


class Languages(unittest.TestCase):
    def test_the_new_languages_are_known(self):
        self.assertEqual(T.LANGUAGES["ru"], "Russian")
        self.assertEqual(T.LANGUAGES["id"], "Indonesian")
        self.assertEqual([T.forms_in(l) for l in ("de", "ru", "id")], [2, 3, 1])

    def test_a_plural_needs_its_languages_forms(self):
        self.assertEqual(T.check("%n page(s)", ["%n а", "%n б", "%n в"], True, 3), "")
        self.assertIn("exactly 3", T.check("%n page(s)", ["%n а", "%n б"], True, 3))
        self.assertEqual(T.check("%n page(s)", ["%n halaman"], True, 1), "")
        self.assertIn("exactly 1", T.check("%n page(s)", ["%n a", "%n b"], True, 1))
        # Only a two-form language may spell the number out in its singular.
        self.assertEqual(T.check("%n page(s)", ["one page", "%n pages"], True, 2), "")
        self.assertNotEqual(T.check("%n page(s)", ["одна страница", "%n страницы", "%n страниц"], True, 3), "")

    def test_the_prompts_name_the_forms(self):
        self.assertIn("exactly 3 of them", T.ui_system("ru", {}))
        self.assertIn("exactly 1 of them", T.ui_system("id", {}))
        self.assertIn("exactly 2 of them", T.ui_system("de", {}))


if __name__ == "__main__":
    unittest.main()
