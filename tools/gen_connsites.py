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


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    out = sys.argv[2] if len(sys.argv) > 2 else "libs/draw/core/connsites_presets.inc"
    root = ET.parse(sys.argv[1]).getroot()
    rows = []
    for preset in root:
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
    rows.sort()
    with open(out, "w") as f:
        f.write("// Generated from ECMA-376 Part 1 presetShapeDefinitions.xml by tools/gen_connsites.py; don't edit by hand.\n")
        f.write("// Per preset: its name, how many sites it has, its adjustments with their defaults (name and formula), its guides in order (name and formula),\n")
        f.write("// and its sites (angle, x, y: each a guide's name or a number).\n")
        for name, count, avs, gds, sites in rows:
            f.write('{"%s", %d,\nR"(%s)",\nR"(%s)",\nR"(%s)"},\n' % (name, count, lines(avs), lines(gds), lines(sites)))
    print("%d presets with connection sites" % len(rows))


main()
