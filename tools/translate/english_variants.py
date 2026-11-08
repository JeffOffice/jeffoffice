#!/usr/bin/env python3
"""Makes the English (United Kingdom), English (Canada), and English (Australia)
texts from the American English ones, with a word list, not with a translator:
no API key, no network, the same result every run.

    python3 tools/translate/english_variants.py            # write the files
    python3 tools/translate/english_variants.py --check    # exit 1 if a file is out of date
    python3 tools/translate/english_variants.py --words    # list the words that changed

What it writes:
  translations/jeffoffice_en_<REGION>.ts   only the texts whose spelling differs
  resources/help/en_<REGION>/...           only the Help pages whose spelling differs
(the program falls back to American English for everything else, text by text
and page by page). Then run `python3 tools/help_files.py` when the set of Help
files changed (this script does that too).

A word changes only as a whole word, keeping its capitals. Placeholders (%1, %n),
markup, links, code, file names, and the letter after a & shortcut marker stay as
they are, and so does a Help line that ends with <!-- keep spelling -->. To add a spelling, add it to the lists at the top of this file (SHARED for all three, DOUBLE_L for the doubled L, and the -ize rule for British and Australian).
"""
import argparse
import os
import re
import subprocess
import sys
import xml.etree.ElementTree as ET
from xml.sax.saxutils import escape

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
REGIONS = ("GB", "CA", "AU")

# Words that are spelled the same in the three (colour, centre, grey, ...), with their forms.
SHARED = {}
for stem_us, stem_other, endings in [
        ("color", "colour", ["", "s", "ed", "ing", "ful", "less", "ation", "ations", "ize", "izes"]),
        ("center", "centre", ["", "s"]),
        ("centered", "centred", [""]),
        ("centering", "centring", [""]),
        ("gray", "grey", ["", "s", "ish", "scale", "scales"]),
        ("catalog", "catalogue", ["", "s"]),
        ("favorite", "favourite", ["", "s"]),
        ("behavior", "behaviour", ["", "s"]),
        ("neighbor", "neighbour", ["", "s"]),
        ("honor", "honour", ["", "s", "ed"]),
        ("flavor", "flavour", ["", "s"]),
        ("labor", "labour", ["", "s"]),
        ("humor", "humour", ["", "s"]),
        ("rumor", "rumour", ["", "s"]),
        ("vapor", "vapour", ["", "s"]),
        ("armor", "armour", ["", "s"]),
        ("odor", "odour", ["", "s"]),
        ("fiber", "fibre", ["", "s"]),
        ("meter", "metre", ["", "s"]),
        ("centimeter", "centimetre", ["", "s"]),
        ("millimeter", "millimetre", ["", "s"]),
        ("kilometer", "kilometre", ["", "s"]),
        ("liter", "litre", ["", "s"]),
        ("milliliter", "millilitre", ["", "s"]),
        ("theater", "theatre", ["", "s"]),
        ("defense", "defence", ["", "s"]),
        ("offense", "offence", ["", "s"]),
        ("pretense", "pretence", ["", "s"]),
        ("fulfill", "fulfil", ["", "s", "ed", "ing"]),
        ("skillful", "skilful", [""]),
        ("maneuver", "manoeuvre", ["", "s", "ed", "ing"]),
        ("skeptic", "sceptic", ["", "s", "al"]),
        ("artifact", "artefact", ["", "s"]),
        ("judgment", "judgement", ["", "s"]),
        ("acknowledgment", "acknowledgement", ["", "s"]),
        ("analog", "analogue", [""]),
        ("plow", "plough", ["", "s"]),
        ("mold", "mould", ["", "s"]),
        ("sulfur", "sulphur", [""]),
        ("aluminum", "aluminium", [""]),
        ("pajamas", "pyjamas", [""]),
        ("licenses", "licences", [""]),
        ("license", "licence", [""]),
        ("sizable", "sizeable", [""]),
        ("jewelry", "jewellery", [""]),
        ("ax", "axe", ["", "s"])]:
    for e in endings:
        SHARED[stem_us + e] = stem_other + e
# "colorize" etc. follow the -ize rule per region, so the Canadian one keeps -ize.
SHARED["colorize"] = "colourize"
SHARED["colorizes"] = "colourizes"
# "analog" only as the whole word, and "ax" is too short to trust: drop it.
SHARED.pop("ax", None)
SHARED.pop("axs", None)
SHARED.pop("analog", None)
# The noun "license"; the verb ("to license") is "license" in all three, so a verb phrase keeps it.
LICENSE_VERB_BEFORE = re.compile(r"(?:\bto|\bwill|\bcan|\bmay|\bwe|\byou|\bI|\bthey|\bnot)\s+$", re.I)

# Doubled consonant before an ending: British, Australian, and Canadian all write "cancelled".
DOUBLE_L = {}
for us, other in [("canceled", "cancelled"), ("canceling", "cancelling"), ("labeled", "labelled"), ("labeling", "labelling"),
                  ("modeled", "modelled"), ("modeling", "modelling"), ("traveled", "travelled"), ("traveling", "travelling"),
                  ("traveler", "traveller"), ("travelers", "travellers"), ("leveled", "levelled"), ("leveling", "levelling"),
                  ("fueled", "fuelled"), ("fueling", "fuelling"), ("signaled", "signalled"), ("signaling", "signalling"),
                  ("totaled", "totalled"), ("totaling", "totalling"), ("marveled", "marvelled"), ("channeled", "channelled"),
                  ("channeling", "channelling"), ("tunneled", "tunnelled"), ("tunneling", "tunnelling"), ("counselor", "counsellor"),
                  ("enrollment", "enrolment"), ("enroll", "enrol"), ("enrolls", "enrols"), ("installment", "instalment"),
                  ("installments", "instalments"), ("skillfully", "skilfully")]:
    DOUBLE_L[us] = other

# -ize -> -ise (British and Australian only), found by rule with these exceptions (words that merely end in "size").
IZE_KEEP = re.compile(r"(?:size|sizes|sized|sizing|prize|prizes|seize|seized|seizes|seizing|maize|capsize|capsized|baize|apologize_)$", re.I)
IZE = re.compile(r"^(?P<stem>[A-Za-z]{2,}?)(?P<z>iz|yz)(?P<end>e|es|ed|ing|er|ers|ation|ations|able)$", re.I)
# "-ize" words the English-speaking world spells -ize anyway, or that are not verbs of this kind.
IZE_ALWAYS = {"size", "sizes", "sized", "sizing", "resize", "resizes", "resized", "resizing", "autosize", "downsize", "upsize",
              "oversize", "oversized", "undersize", "undersized", "prize", "seize", "maize", "capsize", "baize", "zoom"}


def ise(word):
    """British -ise for an American -ize word, or None if it is not one."""
    low = word.lower()
    if low in IZE_ALWAYS or IZE_KEEP.search(low):
        return None
    m = IZE.match(word)
    if not m:
        return None
    z = m.group("z")
    out = m.group("stem") + ("is" if z.lower() == "iz" else "ys") + m.group("end")
    if z.isupper() and word.isupper():
        out = out.upper()
    return out


def match_case(src, new):
    if src.isupper() and len(src) > 1:
        return new.upper()
    if src[:1].isupper():
        return new[:1].upper() + new[1:]
    return new


def variant_word(word, region, before=""):
    """The spelling of one whole word in a region; None when it stays."""
    low = word.lower()
    new = SHARED.get(low)
    if new == "licence" or new == "licences":
        if LICENSE_VERB_BEFORE.search(before):
            new = None
    if new is None:
        new = DOUBLE_L.get(low)
    if new is None and region in ("GB", "AU"):
        z = ise(word)
        if z is not None:
            return z
    if new is None:
        return None
    return match_case(word, new)


# Pieces of a text that no word of ours may touch.
KEEP = re.compile(r"(<[^>]*>|&[A-Za-z]+;|&#\d+;|%L?\d+|%n|https?://\S+|`[^`]*`|\{[^}]*\})")
WORD_TOKEN = re.compile(r"[A-Za-z][A-Za-z&]*[A-Za-z]|[A-Za-z]")


def respell(token, region, before):
    """One token such as "Colo&r" (a & before the shortcut letter is allowed inside)."""
    if "&" in token:
        plain = token.replace("&", "")
        idx = token.index("&")
        new = variant_word(plain, region, before)
        if new is None:
            return token
        letter = plain[idx] if idx < len(plain) else ""
        # The shortcut keeps its letter: the same one nearest to where it was.
        spots = [i for i, c in enumerate(new) if c == letter]
        if not spots:
            return new
        at = min(spots, key=lambda i: (abs(i - idx), i))
        return new[:at] + "&" + new[at:]
    return variant_word(token, region, before) or token


def respell_plain(text, region, changed=None):
    out = []
    pos = 0
    for m in WORD_TOKEN.finditer(text):
        tok = m.group()
        start, end = m.start(), m.end()
        before = text[max(0, start - 12):start]
        prev = text[start - 1] if start else ""
        nxt = text[end] if end < len(text) else ""
        after2 = text[end:end + 2]
        # File names, extensions, identifiers, and paths stay: gray.png, .color, color_scheme, a/color.
        skip = (prev and prev in "._/\\@#$") or nxt == "_" or (nxt == "." and after2[1:2].isalnum())
        new = tok if skip else respell(tok, region, before)
        if new != tok and changed is not None:
            changed.setdefault((tok.replace("&", ""), new.replace("&", "")), 0)
            changed[(tok.replace("&", ""), new.replace("&", ""))] += 1
        out.append(text[pos:start])
        out.append(new)
        pos = end
    out.append(text[pos:])
    return "".join(out)


def respell_text(text, region, changed=None):
    """A text with markup, placeholders, and code kept as they are."""
    parts = KEEP.split(text)
    # (KEEP.split keeps what matched at the odd places)
    for i in range(0, len(parts), 2):
        parts[i] = respell_plain(parts[i], region, changed)
    return "".join(parts)


# ----- the program's text -----

def read_ts(path):
    """[(context, source, comment, numerus, [translations])] from a .ts file."""
    out = []
    for ctx in ET.parse(path).getroot().iter("context"):
        name = ctx.findtext("name") or ""
        for m in ctx.iter("message"):
            tr = m.find("translation")
            if tr is not None and tr.get("type") in ("vanished", "obsolete"):
                continue
            forms = [f.text or "" for f in tr.iter("numerusform")] if tr is not None and tr.find("numerusform") is not None else [(tr.text or "") if tr is not None else ""]
            out.append((name, m.findtext("source") or "", m.findtext("comment") or "", m.get("numerus") == "yes", forms))
    return out


def ui_files(region, changed=None):
    master = read_ts(os.path.join(ROOT, "translations", "jeffoffice.ts"))
    plurals = {(c, s, k): f for c, s, k, n, f in read_ts(os.path.join(ROOT, "translations", "jeffoffice_en.ts"))}
    contexts = {}
    count = 0
    for ctx, source, comment, numerus, _ in master:
        if numerus:
            forms = plurals.get((ctx, source, comment))
            if not forms:
                continue
            new = [respell_text(f, region, changed) for f in forms]
            if new == forms:
                continue
        else:
            text = respell_text(source, region, changed)
            if text == source:
                continue
            new = [text]
        contexts.setdefault(ctx, []).append((source, comment, numerus, new))
        count += 1
    lines = ['<?xml version="1.0" encoding="utf-8"?>', "<!DOCTYPE TS>", '<TS version="2.1" language="en_%s">' % region]
    for ctx in sorted(contexts):
        lines += ["<context>", "    <name>%s</name>" % escape(ctx)]
        for source, comment, numerus, forms in contexts[ctx]:
            lines.append('    <message numerus="yes">' if numerus else "    <message>")
            lines.append("        <source>%s</source>" % escape(source))
            if comment:
                lines.append("        <comment>%s</comment>" % escape(comment))
            if numerus:
                lines.append("        <translation>")
                lines += ["            <numerusform>%s</numerusform>" % escape(f) for f in forms]
                lines.append("        </translation>")
            else:
                lines.append("        <translation>%s</translation>" % escape(forms[0]))
            lines.append("    </message>")
        lines.append("</context>")
    lines.append("</TS>")
    return "\n".join(lines) + "\n", count


# ----- Help -----

KEEP_LINE = "<!-- keep spelling -->"   # a Help line that ends with this is left as written (it shows the spellings themselves)
FENCE = re.compile(r"^(```|~~~)")
LINK_TARGET = re.compile(r"(\]\([^)]*\))")
KEYWORDS = re.compile(r"(<!--\s*keywords:)(.*?)(-->)", re.S)


def help_page(md, region, changed=None):
    out = []
    in_code = False
    for line in md.split("\n"):
        if FENCE.match(line):
            in_code = not in_code
            out.append(line)
            continue
        if in_code or line.startswith("    ") or KEEP_LINE in line:
            out.append(line)
            continue
        if "<!-- keywords:" in line:
            # Both spellings find the page: the American words stay, the region's are added.
            def both(m):
                words = m.group(2)
                extra = respell_text(words, region, changed)
                if extra == words:
                    return m.group(0)
                mine = [w.strip() for w in words.split(",")]
                add_ = [w.strip() for w in extra.split(",") if w.strip() not in mine]
                return m.group(1) + words.rstrip() + ", " + ", ".join(add_) + " " + m.group(3)
            out.append(KEYWORDS.sub(both, line))
            continue
        pieces = LINK_TARGET.split(line)
        for i in range(0, len(pieces), 2):
            pieces[i] = respell_text(pieces[i], region, changed)
        out.append("".join(pieces))
    return "\n".join(out)


def help_files(region, changed=None):
    """{relative path: text} for the pages that differ."""
    base = os.path.join(ROOT, "resources", "help", "en")
    out = {}
    for d, _, files in os.walk(base):
        for f in sorted(files):
            if not f.endswith(".md"):
                continue
            path = os.path.join(d, f)
            with open(path, encoding="utf-8") as fh:
                md = fh.read()
            new = help_page(md, region, changed)
            if new != md:
                out[os.path.relpath(path, base).replace(os.sep, "/")] = new
    return out


def write_if_changed(path, text):
    old = None
    if os.path.exists(path):
        with open(path, encoding="utf-8", newline="") as f:
            old = f.read()
    if old == text:
        return False
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8", newline="") as f:
        f.write(text)
    return True


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--check", action="store_true", help="change nothing; exit 1 if a file is out of date")
    ap.add_argument("--words", action="store_true", help="list each changed word and how often")
    ap.add_argument("--ui-only", action="store_true")
    ap.add_argument("--help-only", action="store_true")
    args = ap.parse_args()
    stale = []
    listing = {}
    for region in REGIONS:
        changed = listing if args.words else None
        if not args.help_only:
            text, count = ui_files(region, changed)
            path = os.path.join(ROOT, "translations", "jeffoffice_en_%s.ts" % region)
            if args.check:
                if not os.path.exists(path) or open(path, encoding="utf-8", newline="").read() != text:
                    stale.append(path)
            elif write_if_changed(path, text):
                print("%s: %d texts" % (os.path.relpath(path, ROOT), count))
        if not args.ui_only:
            pages = help_files(region, changed)
            folder = os.path.join(ROOT, "resources", "help", "en_%s" % region)
            have = set()
            if os.path.isdir(folder):
                for d, _, files in os.walk(folder):
                    for f in files:
                        have.add(os.path.relpath(os.path.join(d, f), folder).replace(os.sep, "/"))
            if args.check:
                for rel, text in pages.items():
                    p = os.path.join(folder, rel)
                    if not os.path.exists(p) or open(p, encoding="utf-8", newline="").read() != text:
                        stale.append(p)
                stale += [os.path.join(folder, r) for r in sorted(have - set(pages))]
            else:
                n = sum(write_if_changed(os.path.join(folder, rel), text) for rel, text in pages.items())
                for rel in sorted(have - set(pages)):
                    os.remove(os.path.join(folder, rel))
                    n += 1
                print("resources/help/en_%s: %d pages%s" % (region, len(pages), " (%d written)" % n if n else ""))
    if args.words:
        for (us, other), n in sorted(listing.items(), key=lambda kv: kv[0]):
            print("%5d  %s -> %s" % (n, us, other))
    if args.check:
        if stale:
            print("out of date (run tools/translate/english_variants.py):\n  " + "\n  ".join(stale))
            return 1
        print("English variants are current.")
        return 0
    if not args.ui_only and not args.help_only:
        subprocess.run([sys.executable, os.path.join(ROOT, "tools", "help_files.py")], check=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
