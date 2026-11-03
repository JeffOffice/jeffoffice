#!/usr/bin/env python3
"""python3 -I tools/test_reflow_license.py : checks the license reflow on the repository's LICENSE."""
import os
import re
import sys
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import reflow_license as r

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def words(s):
    return re.findall(r"\S+", s)


class Reflow(unittest.TestCase):
    def setUp(self):
        with open(os.path.join(ROOT, "LICENSE"), encoding="utf-8") as f:
            self.src = f.read()
        self.blocks = r.parse(self.src)
        self.txt = r.to_text(self.blocks)

    def test_no_word_is_lost_or_changed(self):
        self.assertEqual(words(self.src), words(self.txt))

    def test_warranty_sentence_is_one_line(self):
        line = [l for l in self.txt.split("\n") if "WITHOUT WARRANTY OF ANY KIND" in l]
        self.assertEqual(len(line), 1)
        self.assertGreater(len(line[0]), 150)  # the whole paragraph, not 76 characters of it

    def test_structure_is_kept(self):
        kinds = [k for k, _ in self.blocks]
        self.assertEqual(kinds[0], "head")
        self.assertIn("title", kinds)
        self.assertIn("item", kinds)
        self.assertIn("sample", kinds)
        self.assertIn("END OF TERMS AND CONDITIONS", [x for k, p in self.blocks if k == "head" for x in p])
        # samples keep one line per source line, with their relative indent
        self.assertIn("\n    <program>  Copyright (C) <year>  <name of author>\n", self.txt)

    def test_additional_terms_follow(self):
        with open(os.path.join(ROOT, "ADDITIONAL-TERMS.md"), encoding="utf-8") as f:
            md = r.parse_markdown(f.read())
        txt = r.to_text(self.blocks + [("head", ["Additional Terms under section 7"])] + md)
        self.assertGreater(txt.index("Additional Terms under section 7"), txt.index("END OF TERMS AND CONDITIONS"))
        self.assertNotIn("**", txt)
        self.assertNotIn("## ", txt)
        self.assertIn("    - the copyright notice", txt)
        self.assertIn("5. Indemnification (section 7(f)).", txt)

    def test_rtf_is_balanced(self):
        t = r.to_rtf(self.blocks)
        self.assertEqual(t.count("{") - t.count(r"\{"), t.count("}") - t.count(r"\}"))
        self.assertTrue(t.isascii())


if __name__ == "__main__":
    unittest.main()
