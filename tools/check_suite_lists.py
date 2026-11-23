#!/usr/bin/env python3
"""Checks that every test program CMake defines is run by the local gate and by CI on every system.

A test program is a name given to add_test(NAME ...) in a CMakeLists.txt, or to jo_app_checks(...) (the *_xcb variants are options of
those and not programs). Each one must be in the SUITES list of tools/gate.sh, and in the list of tools/ci_targets.sh for Linux, for
Windows (the two halves, 1/2 and 2/2, together), and for macOS (the same), whether as a test program or as an app-check program.
On Nov 23, 2026 three Write suites ran in the gate and in no CI list, and a fourth was in neither.

A program that is deliberately not run somewhere goes in EXCEPTIONS below, with the reason (a suite that needs a display only one
system has, for example).

    python3 tools/check_suite_lists.py

Prints each missing entry and exits 1, or prints "suite lists ok" and exits 0.
"""
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent

# (program, where) -> reason. where is "gate" or a system name: "Linux", "Windows", "macOS".
# writeobjecttests (tests/write/tst_write_objects.cpp, Nov 18) has been in no list since it was added. On Nov 23, 2026 it printed nothing and did not end
# in 9 minutes on this machine (offscreen, a warm build): until it is found and fixed it runs nowhere, and putting it in a list would hang the gate.
EXCEPTIONS = {(p, w): "hangs, see above" for p in ("writeobjecttests",) for w in ("gate", "Linux", "Windows", "macOS")}

SYSTEMS = {
    "Linux": [("", "")],
    "Windows": [("1/2", ""), ("2/2", "")],
    "macOS": [("1/2", ""), ("2/2", "")],
}


def cmake_programs() -> list:
    names = set()
    for path in ROOT.rglob("CMakeLists.txt"):
        if path.relative_to(ROOT).parts[0].startswith("build"):   # (build, build-asan, and the like: their _deps hold other projects' tests)
            continue
        text = path.read_text(encoding="utf-8", errors="replace")
        text = re.sub(r"#[^\n]*", "", text)
        for m in re.finditer(r"\badd_test\s*\(\s*NAME\s+([A-Za-z0-9_]+)", text):
            names.add(m.group(1))
        for m in re.finditer(r"^\s*jo_app_checks\s*\(\s*([A-Za-z0-9_]+)", text, re.M):
            names.add(m.group(1))
    return sorted(names)


def gate_suites() -> set:
    text = (ROOT / "tools" / "gate.sh").read_text(encoding="utf-8")
    m = re.search(r'^SUITES="([^"]*)"', text, re.M)
    if not m:
        sys.exit("check_suite_lists: no SUITES list in tools/gate.sh")
    return set(m.group(1).split())


def ci_programs(system: str) -> set:
    found = set()
    for shard, _ in SYSTEMS[system]:
        for part in ("tests", "checks"):
            args = ["bash", str(ROOT / "tools" / "ci_targets.sh"), part, system]
            args += [shard, "run"] if shard else ["", "run"]
            out = subprocess.run(args, capture_output=True, text=True)
            if out.returncode != 0:
                sys.exit("check_suite_lists: ci_targets.sh failed: " + " ".join(args[2:]) + ": " + out.stderr.strip())
            found.update(w.split(":")[0] for w in out.stdout.split())
    return found


def main() -> int:
    programs = cmake_programs()
    where = {"gate": gate_suites()}
    for system in SYSTEMS:
        where[system] = ci_programs(system)
    problems = []
    for prog in programs:
        for place, have in where.items():
            if prog not in have and (prog, place) not in EXCEPTIONS:
                problems.append(f"{prog}: not run by {'tools/gate.sh' if place == 'gate' else 'CI on ' + place + ' (tools/ci_targets.sh)'}")
    for key in EXCEPTIONS:
        if key[0] not in programs:
            problems.append(f"{key[0]}: listed in EXCEPTIONS but CMake does not define it")
    for p in problems:
        print(p)
    if problems:
        print(f"{len(problems)} problem(s)")
        return 1
    print("suite lists ok")
    return 0


if __name__ == "__main__":
    sys.exit(main())
