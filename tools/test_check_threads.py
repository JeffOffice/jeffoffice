#!/usr/bin/env python3
"""python3 -I tools/test_check_threads.py : checks that check_threads.py reports a thread_local in code and not in a comment or a string."""
import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import check_threads as c

CODE = '''int f()
{
    static thread_local int calls = 0;
    return ++calls;
}
'''
QUIET = '''// a thread_local counter would crash at a thread's end
/* a thread_local
   in a block comment */
const char *s = "thread_local";
const char *r = R"x(thread_local " )x";
char q = '"'; int thread_locals_ok = 1;
'''


def about(rel, text):
    """What the check says of one file (it also says that the other listed files were not given)."""
    return [m for m in c.thread_locals({rel: text}) if m.startswith(rel + ":") or m.startswith(rel + " ")]


class ThreadLocal(unittest.TestCase):
    def test_reported_with_file_and_line(self):
        self.assertEqual(about("libs/zz/unlisted.cpp", CODE), ["libs/zz/unlisted.cpp:3: static thread_local int calls = 0;"])

    def test_comment_and_string_are_not_code(self):
        self.assertEqual(about("libs/zz/unlisted.cpp", QUIET), [])

    def test_code_after_a_literal_is_still_seen(self):
        text = 'const char *s = "a"; thread_local int n;\n'
        self.assertEqual(len(about("libs/zz/unlisted.cpp", text)), 1)

    def test_count_must_match_the_list(self):
        listed = next(iter(c.load_thread_local_allowed()))
        count = c.load_thread_local_allowed()[listed][0]
        msgs = about(listed, CODE * (count + 1))
        self.assertTrue(any("says" in m for m in msgs), msgs)


if __name__ == "__main__":
    unittest.main()
