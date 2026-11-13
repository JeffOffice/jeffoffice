#!/usr/bin/env python3
"""Reads the QtTest XML results a gate run wrote (JO_GATE_XML=1 tools/gate.sh ...: <log>.<part>.xml) and writes tools/gate_times.json: the milliseconds each test class and test function took,
by suite, for tools/gate_plan.py to balance the parts by. Prints the 15 slowest test functions.
    python3 tools/gate_times.py <log> [--write]
A function that runs on every part of a sharded app-check suite counts the longest of its parts.
"""
import glob
import json
import os
import re
import sys
import xml.etree.ElementTree as ET

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, "tools", "gate_times.json")


def main():
    log = sys.argv[1]
    classes, functions, slow = {}, {}, []
    for path in sorted(glob.glob(log + ".*.xml")):
        suite = re.sub(r"\.(part\d+|rest)$", "", os.path.basename(path)[len(os.path.basename(log)) + 1:-4])
        text = open(path, encoding="utf-8", errors="replace").read()
        # (the runner appends one document for each class)
        for doc in re.findall(r"<TestCase .*?</TestCase>", text, re.S):
            try:
                case = ET.fromstring(doc)
            except ET.ParseError:
                continue
            name = case.get("name")
            total = 0.0
            for fn in case.findall("TestFunction"):
                d = fn.find("Duration")
                ms = float(d.get("msecs")) if d is not None else 0.0
                if fn.get("name") in ("initTestCase", "cleanupTestCase"):
                    continue
                total += ms
                slow.append((ms, suite, name, fn.get("name"), path))
                key = fn.get("name")
                if suite.endswith("checks"):
                    functions.setdefault(suite, {})[key] = max(functions.get(suite, {}).get(key, 0), round(ms))
                else:
                    functions.setdefault(suite, {})[name + "::" + key] = round(ms)
            if not suite.endswith("checks"):
                classes.setdefault(suite, {})[name] = classes.get(suite, {}).get(name, 0) + round(total)
    slow.sort(reverse=True)
    for ms, suite, name, fn, path in slow[:15]:
        print("%8.1f s  %s %s::%s (%s)" % (ms / 1000, suite, name, fn, os.path.basename(path)))
    if "--write" in sys.argv:
        # (only what the plan reads: the classes of the suites split by class, the functions of those split by function)
        by_class, by_function = ("crunchtests", "sheettests", "jdtests", "jptests"), ("writetests", "flowtests")
        # (and only what took 200 ms or more: the rest is placed as the gate did before, and weighs nothing)
        classes = {k: {n: ms for n, ms in v.items() if ms >= 200} for k, v in classes.items() if k in by_class}
        functions = {k: {n: ms for n, ms in v.items() if ms >= 200} for k, v in functions.items() if k in by_function}
        with open(OUT, "w", encoding="utf-8") as f:
            json.dump({"about": "Milliseconds of each test class and function in one full gate run (tools/gate_times.py); tools/gate_plan.py balances the gate's parts by them",
                       "classes": classes, "functions": functions}, f, indent=1, sort_keys=True)
            f.write("\n")


if __name__ == "__main__":
    main()
