#!/usr/bin/env python3
"""Compares the text JeffWrite leaves after Accept All and Reject All with reference texts.

    review_compare.py WRITETOOL REFDIR [--only REGEX] [--show N] [--jobs N]

REFDIR holds in/<key>.docx, and out/<key>.accept.txt and out/<key>.reject.txt: the body text of the
original program after its Accept All and Reject All (UTF-8, a paragraph ends in \\r). The texts come
from `writetool review-text <file> accept|reject`. Before comparing, BOTH texts are normalized, and
only so:

  - line endings: "\\r\\n" and "\\n" become "\\r" (the reference writes a paragraph end as \\r);
  - cell and row end marks: the control character \\x07 is dropped (the reference's end-of-cell and
    end-of-row marks; the \\r in front of each stays, so a cell or a row still ends its paragraph);
  - field marks: \\x13 (field begin), \\x14 (separator), and \\x15 (end) are dropped.

Nothing else is changed: every other character (tabs, \\x0b line breaks, \\x0c page and section
breaks, \\x01 pictures, \\x02 note references, \\x1e and \\x1f hyphens, non-breaking spaces) has to match.
Prints one line per text, a table of matches, and for each text that differs where it first does.
"""
import os, re, subprocess, sys
from concurrent.futures import ThreadPoolExecutor

MARKS = re.compile("[\x07\x13\x14\x15]")


def normalize(text):
    return MARKS.sub("", text.replace("\r\n", "\r").replace("\n", "\r"))


def run(tool, path, mode):
    p = subprocess.run([tool, "review-text", path, mode], capture_output=True, timeout=600,
                       env=dict(os.environ, QT_QPA_PLATFORM="offscreen", JP_NO_TELEMETRY="1"))
    return p.stdout.decode("utf8", "replace"), p.returncode


def main():
    args = sys.argv[1:]
    if len(args) < 2:
        print(__doc__)
        return 2
    tool, ref = args[0], args[1]
    only = re.compile(args[args.index("--only") + 1]) if "--only" in args else None
    show = int(args[args.index("--show") + 1]) if "--show" in args else 60
    jobs = int(args[args.index("--jobs") + 1]) if "--jobs" in args else 6
    keys = sorted(f[:-5] for f in os.listdir(os.path.join(ref, "in")) if f.endswith(".docx"))
    if only:
        keys = [k for k in keys if only.search(k)]
    work = [(k, m) for k in keys for m in ("accept", "reject")]

    def one(item):
        k, m = item
        want_path = os.path.join(ref, "out", "%s.%s.txt" % (k, m))
        with open(want_path, encoding="utf8", newline="") as f:
            want = normalize(f.read())
        got, code = run(tool, os.path.join(ref, "in", k + ".docx"), m)
        got = normalize(got)
        return k, m, want, got, code

    ok = 0
    bad = []
    with ThreadPoolExecutor(jobs) as ex:
        for k, m, want, got, code in ex.map(one, work):
            if want == got and code == 0:
                ok += 1
                print("%s %-6s match" % (k, m))
            else:
                n = next((i for i in range(min(len(want), len(got))) if want[i] != got[i]), min(len(want), len(got)))
                bad.append((k, m, n, want, got, code))
                print("%s %-6s DIFFERS at %d (reference %d characters, ours %d, exit %d)" % (k, m, n, len(want), len(got), code))
    print("\nmatches: %d of %d" % (ok, len(work)))
    for k, m, n, want, got, code in bad:
        print("\n%s %s at %d:\n  reference: %r\n  ours:      %r" % (k, m, n, want[max(0, n - 30):n + show], got[max(0, n - 30):n + show]))
    return 0 if not bad else 1


if __name__ == "__main__":
    sys.exit(main())
