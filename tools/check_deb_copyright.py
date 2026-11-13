#!/usr/bin/env python3
"""Checks what the .deb's copyright file is made from: every row of docs/third-party.json names a license, the file tools/gen_deb_copyright.py writes from it is
shaped as a Debian copyright file (DEP-5: a header, a Files stanza with a License for each component, and each license's text once), and the parser test passes.
Run by the gate (tools/gate.sh); nothing is installed or downloaded.
"""
import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gen_deb_copyright as gen  # noqa: E402


def main():
    problems = []
    parts = gen.load()
    for p in parts:
        if not str(p.get("license", "")).strip():
            problems.append("%s in docs/third-party.json has no license" % p.get("name", "(no name)"))
    if not problems:
        problems += gen.problems(gen.generate(parts))
    import test_gen_deb_copyright as tests
    result = unittest.TextTestRunner(stream=open(os.devnull, "w"), verbosity=0).run(unittest.defaultTestLoader.loadTestsFromModule(tests))
    if not result.wasSuccessful():
        problems.append("tools/test_gen_deb_copyright.py: %d failed" % (len(result.failures) + len(result.errors)))
    for p in problems:
        print("check_deb_copyright.py: " + p)
    print("deb copyright: %d rows, %d problem(s)" % (len(parts), len(problems)))
    sys.exit(1 if problems else 0)


if __name__ == "__main__":
    main()
