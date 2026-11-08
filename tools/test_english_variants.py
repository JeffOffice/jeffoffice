#!/usr/bin/env python3
"""python3 -I tools/test_english_variants.py : checks the English variants' word list and that the committed files are current."""
import os
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "translate"))
import english_variants as v


class Words(unittest.TestCase):
    def test_a_word_in_the_middle(self):
        self.assertEqual(v.respell_text("The color scheme", "GB"), "The colour scheme")

    def test_a_word_at_the_start_of_a_text(self):
        self.assertEqual(v.respell_text("Color scheme", "GB"), "Colour scheme")
        self.assertEqual(v.respell_text("Customize", "GB"), "Customise")
        self.assertEqual(v.respell_text("Customize", "CA"), "Customize")
        self.assertEqual(v.respell_text("Gray", "AU"), "Grey")

    def test_names_of_files_stay(self):
        self.assertEqual(v.respell_text("Open gray.png and color_scheme", "GB"), "Open gray.png and color_scheme")
        self.assertEqual(v.respell_text(".color and a/color", "GB"), ".color and a/color")


class Files(unittest.TestCase):
    def test_the_files_are_current(self):
        for region in v.REGIONS:
            text, _ = v.ui_files(region)
            with open(os.path.join(v.ROOT, "translations", "jeffoffice_en_%s.ts" % region), encoding="utf-8", newline="") as f:
                self.assertEqual(f.read(), text, region)
            folder = os.path.join(v.ROOT, "resources", "help", "en_%s" % region)
            for rel, page in v.help_files(region).items():
                with open(os.path.join(folder, rel), encoding="utf-8", newline="") as f:
                    self.assertEqual(f.read(), page, region + "/" + rel)


if __name__ == "__main__":
    unittest.main()
