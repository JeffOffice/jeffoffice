#!/usr/bin/env python3
"""python3 -I tools/test_check_third_party.py : checks that check_third_party.py reads an app's extra rows whole."""
import os
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import check_third_party as c

SOURCE = '''QWidget *App::buildAbout()
{
    AboutHost host;
    // The app's own parts.
    host.extras = {
        {QStringLiteral("First"), QStringLiteral("https://first.example/"), tr("Reads files; writes them too"), QStringLiteral("MIT"),
         QStringLiteral("https://first.example/license?a=1;b=2")},
        {QStringLiteral("Second"), QStringLiteral("https://second.example/"), tr("Draws"), QStringLiteral("MPL-2.0 or LGPL-2.1+"),
         QStringLiteral("https://second.example/license")},
    };
    host.tagline = tr("Not a row");
    return buildAboutPage(nullptr, this, host);
}
'''


class Extras(unittest.TestCase):
    def read(self, text):
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "app.cpp")
            with open(path, "w", encoding="utf-8") as f:
                f.write(text)
            return c.rows(path)

    def test_a_row_with_a_semicolon_is_not_cut_short(self):
        found = self.read(SOURCE)
        self.assertEqual([r["name"] for r in found], ["First", "Second"])
        self.assertEqual(found[0]["use"], "Reads files; writes them too")
        self.assertEqual(found[0]["licenseUrl"], "https://first.example/license?a=1;b=2")

    def test_the_old_call_form_still_reads(self):
        found = self.read(SOURCE.replace("host.extras = {", "addThirdPartyList(lv, legal, f, {").replace("    };\n    host.tagline", "    });\n    host.tagline"))
        self.assertEqual([r["name"] for r in found], ["First", "Second"])


if __name__ == "__main__":
    unittest.main()
