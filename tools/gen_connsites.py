#!/usr/bin/env python3
"""Writes libs/draw/core/connsites_presets.inc: the connection sites of the
standard's preset shapes, read from presetShapeDefinitions.xml.

Usage: python3 -I tools/gen_connsites.py <presetShapeDefinitions.xml> [out.inc]
"""
import sys
import xml.etree.ElementTree as ET


def local(tag):
    return tag.rsplit('}', 1)[-1]


def child(e, name):
    for c in e:
        if local(c.tag) == name:
            return c
    return None


def lines(items):
    return "\n".join(items)


# The standard's file in use here names upDownArrow twice (the same entry both times) and has no upArrow. A duplicate that is a copy of the first is dropped;
# any other duplicate stops the run. The up arrow is the down arrow turned over: its head ends at y2 from the top, and its sites are the tip, the head's
# left end, the foot's middle, and the head's right end (the down arrow's list, from the other side).
KNOWN_COPIES = {"upDownArrow"}
UP_ARROW = ("upArrow", ["adj1 val 50000", "adj2 val 50000"],
            ["maxAdj2 */ 100000 h ss", "a1 pin 0 adj1 100000", "a2 pin 0 adj2 maxAdj2", "dy2 */ ss a2 100000", "y2 +- t dy2 0"],
            ["3cd4 hc t", "cd2 l y2", "cd4 hc b", "0 r y2"])


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    out = sys.argv[2] if len(sys.argv) > 2 else "libs/draw/core/connsites_presets.inc"
    root = ET.parse(sys.argv[1]).getroot()
    rows = []
    seen = {}
    for preset in root:
        name = local(preset.tag)
        text = ET.tostring(preset)
        if name in seen:
            if name not in KNOWN_COPIES or seen[name] != text:
                sys.exit("the standard's file names %s twice, and the second is not a copy of the first" % name)
            continue
        seen[name] = text
        cxn = child(preset, "cxnLst")
        if cxn is None:
            continue
        av = child(preset, "avLst")
        gd = child(preset, "gdLst")
        avs = ["%s %s" % (g.get("name"), " ".join(g.get("fmla").split())) for g in (av if av is not None else [])]
        gds = ["%s %s" % (g.get("name"), " ".join(g.get("fmla").split())) for g in (gd if gd is not None else [])]
        sites = []
        for c in cxn:
            pos = child(c, "pos")
            sites.append("%s %s %s" % (c.get("ang"), pos.get("x"), pos.get("y")))
        rows.append((local(preset.tag), len(sites), avs, gds, sites))
    if "upArrow" in seen:
        sys.exit("the standard's file has upArrow now: drop the supplied one")
    name, avs, gds, sites = UP_ARROW
    rows.append((name, len(sites), avs, gds, sites))
    rows.sort()
    if len({r[0] for r in rows}) != len(rows):
        sys.exit("a preset is listed twice")
    with open(out, "w") as f:
        f.write("// Generated from ECMA-376 Part 1 presetShapeDefinitions.xml by tools/gen_connsites.py; don't edit by hand.\n")
        f.write("// Per preset: its name, how many sites it has, its adjustments with their defaults (name and formula), its guides in order (name and formula),\n")
        f.write("// and its sites (angle, x, y: each a guide's name or a number).\n")
        for name, count, avs, gds, sites in rows:
            f.write('{"%s", %d,\nR"(%s)",\nR"(%s)",\nR"(%s)"},\n' % (name, count, lines(avs), lines(gds), lines(sites)))
    print("%d presets with connection sites" % len(rows))


main()
