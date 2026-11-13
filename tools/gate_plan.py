#!/usr/bin/env python3
"""Which of the gate's parts runs each item (a test class, or a test function of a one-class suite), balanced by the time it took in tools/gate_times.json.
    python3 tools/gate_plan.py <suite> <kind: classes|functions> <parts> <part k> item...      prints the items of part k, one per line
The slowest item goes first, each into the part that is lightest so far. An item with no time is placed as the gate did before (round robin by its place among those with none). With no
times at all that is the old split.
"""
import json
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def plan(times, items, n):
    """The part (0 to n-1) of each item, by index."""
    part = [0] * len(items)
    load = [0] * n
    timed = sorted((i for i, x in enumerate(items) if x in times), key=lambda i: (-times[items[i]], items[i]))
    for i in timed:
        k = min(range(n), key=lambda p: (load[p], p))
        part[i] = k
        load[k] += times[items[i]]
    untimed = [i for i, x in enumerate(items) if x not in times]
    for j, i in enumerate(untimed):
        part[i] = (j + 1) % n   # (as the gate's awk did: item number j+1 of the list goes to part (j+1) % n + 1... counted from 0)
    return part


def main():
    suite, kind, n, k = sys.argv[1], sys.argv[2], int(sys.argv[3]), int(sys.argv[4])
    items = sys.argv[5:]
    try:
        data = json.load(open(os.path.join(ROOT, "tools", "gate_times.json")))
    except (OSError, ValueError):
        data = {}
    raw = data.get(kind, {}).get(suite, {})
    # (a function is keyed Class::function for a suite of several classes; the gate's list names the function only)
    times = {}
    for key, ms in raw.items():
        times[key.split("::")[-1] if kind == "functions" else key] = ms
    for i, p in enumerate(plan(times, items, n)):
        if p == k % n:
            print(items[i])


if __name__ == "__main__":
    main()
