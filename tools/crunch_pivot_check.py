#!/usr/bin/env python3
"""The yardstick of the pivot tables: works out every pivot table of the workbooks in a list from its cache and compares the cells with the cells the file stored.

  crunch_pivot_check.py <jcrunchtool> <list.txt> <result.tsv> [jobs]

The list has a path on each line; only the workbooks that have a pivot table are run through `jcrunchtool pivot-check`. The result has a line for each pivot
table (see tools/jcrunchtool_pivot.cpp); the summary printed is the share of tables that match exactly, and the most common first differences.
"""
import collections
import concurrent.futures
import re
import subprocess
import sys
import zipfile


def has_pivot(path):
    try:
        with zipfile.ZipFile(path) as z:
            return any(n.startswith('xl/pivotTables/pivotTable') and n.endswith('.xml') for n in z.namelist())
    except Exception:
        return False


def run(tool, path):
    try:
        p = subprocess.run([tool, 'pivot-check', path], capture_output=True, text=True, timeout=600, stdin=subprocess.DEVNULL)
        return [l for l in p.stdout.splitlines() if l.startswith('pivot-check\t')] or ['pivot-check\t%s\t\t\tno output (exit %d)\t0\t0\t' % (path, p.returncode)]
    except subprocess.TimeoutExpired:
        return ['pivot-check\t%s\t\t\ttimeout\t0\t0\t' % path]


def main():
    tool, lst, result = sys.argv[1:4]
    jobs = int(sys.argv[4]) if len(sys.argv) > 4 else 6
    paths = [l.rstrip('\n') for l in open(lst) if l.strip()]
    with concurrent.futures.ThreadPoolExecutor(jobs) as ex:
        withp = [p for p, h in zip(paths, ex.map(has_pivot, paths)) if h]
        lines = []
        for r in ex.map(lambda p: run(tool, p), withp):
            lines += r
    with open(result, 'w') as f:
        f.write('\n'.join(lines) + '\n')
    kinds = collections.Counter()
    causes = collections.Counter()
    for l in lines:
        c = l.split('\t')
        res = c[4] if len(c) > 4 else '?'
        k = 'ok' if res.startswith('ok') else ('differs' if res == 'differs' else 'unsupported')
        kinds[k] += 1
        if k != 'ok':
            first = c[7] if len(c) > 7 else ''
            causes[(res if k == 'unsupported' else re.sub(r'[A-Z]+\d+: ', 'cell: ', re.sub(r'(stored|worked out) .*', '', first)) )[:90]] += 1
    n = sum(kinds.values())
    print('workbooks with pivots: %d, pivot tables: %d' % (len(withp), n))
    print('ok %d (%.1f%%), differs %d, unsupported %d' % (kinds['ok'], 100.0 * kinds['ok'] / max(n, 1), kinds['differs'], kinds['unsupported']))
    for cause, k in causes.most_common(8):
        print('  %4d  %s' % (k, cause))


if __name__ == '__main__':
    main()
