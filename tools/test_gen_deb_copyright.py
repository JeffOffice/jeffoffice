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
        self.assertEqual(gen.problems(gen.generate(self.parts), gen.ROOT), [])

    def test_each_component_with_a_path_has_its_stanzas_and_the_rest_are_in_the_main_comment(self):
        paras = gen.parse(gen.generate(self.parts))
        text = gen.generate(self.parts)
        want = 1
        for p in self.parts:
            if p.get("windowsOnly"):
                self.assertNotIn(p["name"], text)
                continue
            entry = gen.entry_of(p["name"])
            want += len(gen.stanzas_of(entry))
            self.assertEqual(entry[1] is None, p["name"] + " (" in dict(paras[1])["Comment"], p["name"])
        self.assertEqual(len([p for p in paras if p[0][0] == "Files"]), want)

    def test_each_license_text_is_there_once(self):
        paras = gen.parse(gen.generate(self.parts))
        names = [dict(p)["License"].split("\n")[0] for p in paras if p[0][0] == "License"]
        self.assertEqual(len(names), len(set(names)))
        self.assertIn("GPL-3", names)

    def test_the_list_text_is_never_turned_into_alternatives(self):
        text = gen.generate(self.parts)
        stanzas = {dict(p)["Files"].split("\n")[0]: dict(p)["License"] for p in gen.parse(text) if p[0][0] == "Files"}
        # (a row of several licenses is a stanza for each family, under its own license)
        self.assertEqual(stanzas["resources/fonts/*"], "OFL-1.1")
        self.assertEqual(stanzas["resources/fonts/RobotoSlab-*"], "Apache-2.0")
        self.assertEqual(stanzas["resources/fonts/TeXGyre*"], "GUST-Font-License")
        self.assertEqual(stanzas["resources/fonts/DejaVu*"], "Bitstream-Vera")
        self.assertEqual(stanzas["resources/fonts/LiberationSansNarrow-*"], "GPL-2 with Font-exception")
        # (the parts of a dictionary are `and`, and the free text's `;` and commas are not read as anything)
        self.assertEqual(stanzas["resources/dict/fr_FR/*"], "MPL-2.0 and LGPL")
        self.assertEqual(stanzas["resources/dict/en/en_*"], "SCOWL and LGPL")

    def test_the_names_are_debian_short_names_or_have_their_own_text(self):
        text = gen.generate(self.parts)
        for bad in ("ECI-s-terms", "The-agency-s-terms", "GPL-3.0", "GPL-2.0+", "Apache-2.0-LLVM", "BSD-3-Clause"):
            self.assertNotIn("License: " + bad, text)
        names = [dict(p)["License"].split("\n")[0] for p in gen.parse(text) if p[0][0] == "License"]
        for n in ("GPL-3", "GPL-3+", "OFL-1.1", "Expat", "ECI-terms", "ISBN-Agency-terms", "GUST-Font-License"):
            self.assertTrue(n in names or n.rstrip("+") in names, n)

    def test_the_main_stanza_is_files_star_for_gpl3_and_later(self):
        paras = gen.parse(gen.generate(self.parts))
        self.assertEqual(dict(paras[1])["Files"], "*")
        self.assertEqual(dict(paras[1])["License"], "GPL-3+")
        self.assertIn("Qt 6", dict(paras[1])["Comment"])   # (it has no path in the source)

    def test_the_paths_are_source_paths_that_exist_and_no_two_stanzas_share_one(self):
        text = gen.generate(self.parts)
        self.assertEqual(gen.problems(text, gen.ROOT), [])
        self.assertNotIn("usr/bin", text)
        self.assertTrue(any("is not a path of the source tree" in x for x in gen.problems(text.replace("resources/icons/*", "usr/bin/*"), gen.ROOT)))
        self.assertTrue(any("matches nothing" in x for x in gen.problems(text.replace("resources/icons/*", "resources/nothing/*"), gen.ROOT)))
        twice = text.replace("third_party/qpdf/*", "third_party/ffmpeg/*")
        self.assertTrue(any("is in two stanzas" in x for x in gen.problems(twice, gen.ROOT)))

    def test_a_name_without_a_license_paragraph_is_refused(self):
        saved = list(gen.COMPONENTS)
        try:
            gen.COMPONENTS[gen.COMPONENTS.index(next(e for e in gen.COMPONENTS if e[0] == "Lucide"))] = ("Lucide", [["resources/icons/*"], "Invented-Terms"])
            with self.assertRaises(ValueError):
                gen.generate(self.parts)
        finally:
            gen.COMPONENTS[:] = saved
        text = gen.generate(self.parts).replace("License: ISC\n", "License: ISC-Invented\n", 1)
        self.assertIn("the license ISC-Invented has no text", gen.problems(text))

    def test_a_row_the_table_does_not_know_is_refused(self):
        rows = copy.deepcopy(self.parts) + [{"name": "New library", "license": "MIT", "windowsOnly": False, "use": "x", "url": "u"}]
        with self.assertRaises(ValueError):
            gen.generate(rows)

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
