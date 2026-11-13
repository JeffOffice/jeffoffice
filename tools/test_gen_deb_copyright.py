"""Tests of tools/gen_deb_copyright.py: the parser that checks the structure of a Debian copyright file, and the file made from docs/third-party.json."""
import copy
import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gen_deb_copyright as gen  # noqa: E402

GOOD = """Format: https://www.debian.org/doc/packaging-manuals/copyright-format/1.0/
Upstream-Name: X
Source: https://example.com/

Files: *
Copyright: 2026 X
License: MIT

License: MIT
 Permission is hereby granted.
 .
 Second paragraph of the text.
"""


class ParserTests(unittest.TestCase):
    def test_a_good_file_parses_and_has_no_problems(self):
        paras = gen.parse(GOOD)
        self.assertEqual(len(paras), 3)
        self.assertEqual(paras[2][0][0], "License")
        self.assertIn("Second paragraph", paras[2][0][1])
        self.assertEqual(gen.problems(GOOD), [])

    def test_a_license_without_its_text_is_found(self):
        bad = GOOD.split("License: MIT\n Permission")[0]
        self.assertEqual(gen.problems(bad), ["the license MIT has no text"])

    def test_a_text_that_nothing_uses_is_found(self):
        self.assertEqual(gen.problems(GOOD + "\nLicense: ISC\n Text.\n"), ["the text of the license ISC is not used"])

    def test_a_license_text_twice_is_found(self):
        self.assertIn("the license MIT has its text twice", gen.problems(GOOD + "\nLicense: MIT\n Again.\n"))

    def test_a_stanza_without_files_copyright_or_license_is_found(self):
        self.assertTrue(gen.problems(GOOD.replace("Copyright: 2026 X\n", "")))

    def test_text_that_is_no_paragraph_is_found(self):
        self.assertTrue(gen.problems("just words\n"))
        self.assertTrue(gen.problems(GOOD.replace("Format:", "Formats:")))


class GeneratedFileTests(unittest.TestCase):
    parts = gen.load()

    def test_the_file_made_from_the_list_is_well_formed(self):
        self.assertEqual(gen.problems(gen.generate(self.parts)), [])

    def test_each_component_that_ships_on_linux_has_a_stanza_and_the_rest_do_not(self):
        paras = gen.parse(gen.generate(self.parts))
        stanzas = [p for p in paras if p[0][0] == "Files"]
        self.assertEqual(len(stanzas), 1 + sum(1 for p in self.parts if not p.get("windowsOnly")))
        text = gen.generate(self.parts)
        for p in self.parts:
            self.assertEqual(("Copyright: %s, see" % p["name"]) in text, not p.get("windowsOnly"), p["name"])

    def test_each_license_text_is_there_once(self):
        paras = gen.parse(gen.generate(self.parts))
        names = [dict(p)["License"].split("\n")[0] for p in paras if p[0][0] == "License"]
        self.assertEqual(len(names), len(set(names)))
        self.assertIn("GPL-3.0", names)

    def test_alternatives_and_parts_are_told_apart(self):
        self.assertEqual(gen.expression("MPL-1.1, GPL-2.0+ or LGPL-2.1+")[0], "MPL-1.1 or GPL-2.0+ or LGPL-2.1+")
        self.assertEqual(gen.expression("MPL-2.0; patterns LGPL")[0], "MPL-2.0, and LGPL")
        self.assertEqual(gen.expression("IJG and BSD-3-Clause")[0], "IJG and BSD-3-Clause")

    def test_a_row_without_a_license_is_refused(self):
        rows = copy.deepcopy(self.parts)
        rows[3]["license"] = " "
        with self.assertRaises(ValueError):
            gen.generate(rows)
        del rows[3]["license"]
        with self.assertRaises(ValueError):
            gen.generate(rows)


if __name__ == "__main__":
    unittest.main()
