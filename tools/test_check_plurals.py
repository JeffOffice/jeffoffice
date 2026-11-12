#!/usr/bin/env python3
"""python3 -I tools/test_check_plurals.py : checks that check_plurals.py finds a plural left half-translated or with the wrong number of forms."""
import os
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import check_plurals as c


def ts(lang, source, forms, numerus=True):
    body = "".join(f"<numerusform>{f}</numerusform>" for f in forms)
    attr = ' numerus="yes"' if numerus else ""
    return (f'<?xml version="1.0" encoding="utf-8"?>\n<!DOCTYPE TS>\n<TS version="2.1" language="{lang}">'
            f'<context><name>Ctx</name><message{attr}><source>{source}</source>'
            f'<translation type="unfinished">{body}</translation></message></context></TS>\n')


class Plurals(unittest.TestCase):
    def check(self, files):
        with tempfile.TemporaryDirectory() as d:
            for name, text in files.items():
                with open(os.path.join(d, name), "w", encoding="utf-8") as f:
                    f.write(text)
            return c.problems(d)

    def test_a_complete_russian_plural_passes(self):
        self.assertEqual(self.check({"jeffoffice.ts": ts("", "%n page(s)", ["", "", ""]), "jeffoffice_ru.ts": ts("ru", "%n page(s)", ["%n страница", "%n страницы", "%n страниц"])}), [])

    def test_an_untranslated_russian_plural_passes(self):
        self.assertEqual(self.check({"jeffoffice.ts": ts("", "%n page(s)", ["", "", ""]), "jeffoffice_ru.ts": ts("ru", "%n page(s)", ["", "", ""])}), [])

    def test_a_russian_form_left_empty_fails(self):
        found = self.check({"jeffoffice_ru.ts": ts("ru", "%n page(s)", ["%n страница", "%n страницы", ""])})
        self.assertEqual(len(found), 1)
        self.assertIn("empty", found[0])

    def test_russian_needs_three_forms(self):
        found = self.check({"jeffoffice_ru.ts": ts("ru", "%n page(s)", ["%n страница", "%n страниц"])})
        self.assertEqual(len(found), 1)
        self.assertIn("3 forms", found[0])

    def test_indonesian_has_one_form_and_german_two(self):
        self.assertEqual(self.check({"jeffoffice_id.ts": ts("id", "%n page(s)", ["%n halaman"]), "jeffoffice_de.ts": ts("de", "%n page(s)", ["%n Seite", "%n Seiten"])}), [])
        self.assertEqual(len(self.check({"jeffoffice_id.ts": ts("id", "%n page(s)", ["%n halaman", "%n halaman"])})), 1)

    def test_ukrainian_needs_three_forms(self):
        self.assertEqual(self.check({"jeffoffice_uk.ts": ts("uk", "%n page(s)", ["%n сторінка", "%n сторінки", "%n сторінок"])}), [])
        found = self.check({"jeffoffice_uk.ts": ts("uk", "%n page(s)", ["%n сторінка", "%n сторінок"])})
        self.assertEqual(len(found), 1)
        self.assertIn("3 forms", found[0])

    def test_a_source_with_n_must_be_a_plural(self):
        found = self.check({"jeffoffice.ts": ts("", "%n page(s)", [], numerus=False)})
        self.assertEqual(len(found), 1)
        self.assertIn("not marked as a plural", found[0])


if __name__ == "__main__":
    unittest.main()
