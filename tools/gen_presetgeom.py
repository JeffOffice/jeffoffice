#!/usr/bin/env python3
"""Writes libs/draw/core/presetgeom_presets.inc: the standard's preset shapes
(adjustments, guides, adjustment handles, text rectangle, and paths), read from
presetShapeDefinitions.xml. The table is read by core/presetgeom.cpp, the one
place that evaluates a preset's outline.

Usage: python3 -I tools/gen_presetgeom.py <presetShapeDefinitions.xml> [out.inc]

Per preset, in this order, each part a raw string:
  adjustments  "name formula" per line
  guides       "name formula" per line, in order
  handles      "XY refX minX maxX refY minY maxY posX posY" or
               "PO refR minR maxR refAng minAng maxAng posX posY" per line ("-" for none)
  text rect    "l t r b"
  paths        per line "P w h fill stroke extrusionOk" (w and h "-" when the path has none;
               fill the standard's word, "norm" for the default; stroke and extrusionOk 0 or 1)
               followed by its commands, ";"-separated: M x y, L x y, A wR hR stAng swAng,
               Q x1 y1 x2 y2, C x1 y1 x2 y2 x3 y3, Z
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


def fm(s):
    return " ".join(s.split())


KNOWN_COPIES = {"upDownArrow"}

# The standard's file in use here has no upArrow: it is the down arrow turned over (its head ends at y1 from the top).
UP_ARROW = dict(
    name="upArrow",
    av=["adj1 val 50000", "adj2 val 50000"],
    gd=["maxAdj2 */ 100000 h ss", "a1 pin 0 adj1 100000", "a2 pin 0 adj2 maxAdj2", "dy1 */ ss a2 100000", "y1 +- t dy1 0",
        "dx1 */ w a1 200000", "x1 +- hc 0 dx1", "x2 +- hc dx1 0", "dy2 */ x1 dy1 wd2", "y2 +- y1 0 dy2"],
    ah=["XY adj1 0 100000 - - - x1 b", "XY - - - adj2 0 maxAdj2 l y1"],
    rect="x1 y2 x2 b",
    paths=["P - - norm 1 1 M l y1;L hc t;L r y1;L x2 y1;L x2 b;L x1 b;L x1 y1;Z"])


def dash(v):
    return v if v is not None else "-"


def read(preset):
    av = child(preset, "avLst")
    gd = child(preset, "gdLst")
    ah = child(preset, "ahLst")
    rect = child(preset, "rect")
    pl = child(preset, "pathLst")
    out = dict(name=local(preset.tag))
    out["av"] = ["%s %s" % (g.get("name"), fm(g.get("fmla"))) for g in (av if av is not None else [])]
    out["gd"] = ["%s %s" % (g.get("name"), fm(g.get("fmla"))) for g in (gd if gd is not None else [])]
    hs = []
    for h in (ah if ah is not None else []):
        pos = child(h, "pos")
        if local(h.tag) == "ahXY":
            hs.append("XY " + " ".join([dash(h.get(k)) for k in ("gdRefX", "minX", "maxX", "gdRefY", "minY", "maxY")] + [pos.get("x"), pos.get("y")]))
        else:
            hs.append("PO " + " ".join([dash(h.get(k)) for k in ("gdRefR", "minR", "maxR", "gdRefAng", "minAng", "maxAng")] + [pos.get("x"), pos.get("y")]))
    out["ah"] = hs
    out["rect"] = "%s %s %s %s" % tuple(rect.get(k) for k in "ltrb") if rect is not None else "l t r b"
    paths = []
    for p in (pl if pl is not None else []):
        cmds = []
        for c in p:
            t = local(c.tag)
            pts = [(pt.get("x"), pt.get("y")) for pt in c if local(pt.tag) == "pt"]
            flat = " ".join("%s %s" % q for q in pts)
            if t == "moveTo": cmds.append("M " + flat)
            elif t == "lnTo": cmds.append("L " + flat)
            elif t == "quadBezTo": cmds.append("Q " + flat)
            elif t == "cubicBezTo": cmds.append("C " + flat)
            elif t == "close": cmds.append("Z")
            elif t == "arcTo": cmds.append("A %s %s %s %s" % (c.get("wR"), c.get("hR"), c.get("stAng"), c.get("swAng")))
            else: sys.exit("unknown path command " + t)
        stroke = "0" if p.get("stroke") in ("false", "0") else "1"
        extr = "0" if p.get("extrusionOk") in ("false", "0") else "1"
        paths.append("P %s %s %s %s %s %s" % (dash(p.get("w")), dash(p.get("h")), p.get("fill") or "norm", stroke, extr, ";".join(cmds)))
    out["paths"] = paths
    return out


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    out = sys.argv[2] if len(sys.argv) > 2 else "libs/draw/core/presetgeom_presets.inc"
    root = ET.parse(sys.argv[1]).getroot()
    rows = {}
    seen = {}
    for preset in root:
        name = local(preset.tag)
        text = ET.tostring(preset)
        if name in seen:
            if name not in KNOWN_COPIES or seen[name] != text:
                sys.exit("the standard's file names %s twice, and the second is not a copy of the first" % name)
            continue
        seen[name] = text
        rows[name] = read(preset)
    if "upArrow" in rows:
        sys.exit("the standard's file has upArrow now: drop the supplied one")
    rows["upArrow"] = UP_ARROW
    with open(out, "w") as f:
        f.write("// Generated from ECMA-376 Part 1 presetShapeDefinitions.xml by tools/gen_presetgeom.py; don't edit by hand.\n")
        f.write("// Per preset: name, adjustments, guides, handles, text rectangle, paths (the layout is described in the tool).\n")
        for name in sorted(rows):
            r = rows[name]
            f.write('{"%s",\nR"(%s)",\nR"(%s)",\nR"(%s)",\nR"(%s)",\nR"(%s)"},\n' % (
                name, "\n".join(r["av"]), "\n".join(r["gd"]), "\n".join(r["ah"]), r["rect"], "\n".join(r["paths"])))
    print("%d presets" % len(rows))


main()
