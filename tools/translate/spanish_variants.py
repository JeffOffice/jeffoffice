#!/usr/bin/env python3
"""Makes the Spanish (Spain) texts from the Spanish ones (the Latin American
neutral Spanish of translations/jeffoffice_es.ts and resources/help/es), with a
word list, not with a translator: no API key, no network, the same result every
run.

    python3 tools/translate/spanish_variants.py            # write the files
    python3 tools/translate/spanish_variants.py --check    # exit 1 if a file is out of date
    python3 tools/translate/spanish_variants.py --words    # list the words that changed, and how often

What it writes:
  translations/jeffoffice_es_ES.ts   only the texts whose words differ
  resources/help/es_ES/...           only the Help pages whose words differ
(the program falls back to the neutral Spanish for everything else, text by text
and page by page, as the English variants fall back to American English). Then
run `python3 tools/help_files.py` when the set of Help files changed (this
script does that too).

A word changes only as a whole word, keeping its capitals, number, and gender.
Placeholders (%1, %n), markup, links, code, file names, and the letter after a &
shortcut marker stay as they are, and so does a Help line that ends with
<!-- keep spelling -->. To add a word, add it to WORDS below (and, for a word
both versions share, to SAME so that nobody adds it by mistake).

The File menu: "Archivo" as the name of the File tab, menu, or screen stays
"Archivo", as it is in Spain's software. Any other use of the word is a file
(a fichero): "el archivo", "los archivos", "Archivos recientes".
"""
import argparse
import collections
import os
import re
import subprocess
import sys
from xml.sax.saxutils import escape
import xml.etree.ElementTree as ET

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from english_variants import FENCE, KEEP, KEEP_LINE, KEYWORDS, LINK_TARGET, ROOT, match_case, write_if_changed  # noqa: E402

REGION = "ES"
LANG = "es_ES"

# Words that differ in Spain, with their number forms (the lower-case spelling; a capital or all capitals is kept).
WORDS = {
    "archivo": "fichero", "archivos": "ficheros",
    "computador": "ordenador", "computadores": "ordenadores",
    "celular": "móvil", "celulares": "móviles",
    "mouse": "ratón", "mouses": "ratones",
}
# "computadora" is feminine and "ordenador" is not: the article before it changes too.
FEMININE = {"computadora": "ordenador", "computadoras": "ordenadores"}
ARTICLE = {"la": "el", "las": "los", "una": "un", "unas": "unos", "esta": "este", "estas": "estos", "esa": "ese", "esas": "esos",
           "otra": "otro", "otras": "otros", "toda": "todo", "todas": "todos", "nuestra": "nuestro", "nuestras": "nuestros"}
CONTRACTION = {"de": "del", "a": "al"}   # de la computadora -> del ordenador
# What may stand before or after "computadora" without an adjective that would need to change gender with it.
NO_AGREEMENT = {"a", "al", "de", "del", "en", "con", "sin", "por", "para", "y", "o", "e", "u", "que", "se", "no", "su", "sus", "tu", "tus", "mi", "mis",
                "cada", "como", "desde", "hasta", "sobre", "entre", "ya", "es", "está", "están", "son", "puede", "pueden", "tiene", "tienen", "tenga",
                "local", "locales", "personal", "personales", "portátil", "portátiles", "otra", "otras", "esta", "estas", "esa", "esas", "la", "las",
                "una", "unas", "toda", "todas", "nuestra", "nuestras", "hasta", "cuando", "donde", "si", "ni", "según", "durante", "al", "lo", "le",
                "les", "me", "te", "nos", "os", "mientras", "antes", "después", "también", "solo", "sólo", "más", "menos", "muy", "pero", "sino"}
# Verb forms of "presionar" (to press a key or button) are "pulsar" in Spain: presione -> pulse. "presión" and "presiones" (pressure) stay.
PRESS = re.compile(r"^presion(?=(?:a|an|e|en|ar|ará|arán|ando|ado|ada|ados|adas|arla|arlo|arlas|arlos)$)", re.I)

# Words both versions write the same. They are never changed; a test checks that none is in WORDS.
SAME = {"carpeta", "carpetas", "pantalla", "pantallas", "guardar", "clic", "haga", "teclado", "impresora", "contraseña", "diapositiva",
        "vídeo", "hipervínculo", "vínculo", "tabulación", "ubicación", "copia", "seguridad", "teléfono", "ratón", "pulse", "archivado", "archivar"}

# "Archivo" is the File tab, menu, or screen when it is the whole text, bold or linked, follows one of these words, or leads a menu path.
MENU_BEFORE = re.compile(r"(?:pantalla|página|pestaña|ficha|menú|botón|vista|cinta|comando)s?\s+(?:de\s+(?:la\s+)?)?$", re.I)
MENU_AFTER = re.compile(r"^(?:\*\*|\])?\s*(?:>|›|→|»|&gt;)")

TOKEN = re.compile(r"[A-Za-zÁÉÍÓÚÜÑáéíóúüñ][A-Za-zÁÉÍÓÚÜÑáéíóúüñ&]*[A-Za-zÁÉÍÓÚÜÑáéíóúüñ]|[A-Za-zÁÉÍÓÚÜÑáéíóúüñ]")


def is_menu(before, after, whole):
    """Whether "Archivo" here names the File tab (checked on the plain text around it)."""
    if whole:
        return True
    if before.endswith("**") and after.startswith("**") or before.endswith("[") and (after == "" or after.startswith("]")):
        return True
    return bool(MENU_BEFORE.search(before) or MENU_AFTER.match(after))


def respell_plain(text, kinds, whole, changed=None, left=None, pre="", post=""):
    """One run of plain text (no markup in it). `kinds` is "UI" or "Help", for the counts."""
    toks = list(TOKEN.finditer(text))
    plain = [m.group().replace("&", "") for m in toks]
    new = [None] * len(toks)   # None: the word stays
    for i, m in enumerate(toks):
        prev = text[m.start() - 1] if m.start() else ""
        nxt = text[m.end()] if m.end() < len(text) else ""
        # File names, extensions, identifiers, and paths stay: archivo.png, mouse_speed, a/archivo.
        if (prev and prev in "._/\\@#$") or nxt == "_" or (nxt == "." and text[m.end() + 1:m.end() + 2].isalnum()):
            continue
        low = plain[i].lower()
        if low == "archivo" and plain[i][:1].isupper() and is_menu(pre + text[:m.start()], text[m.end():] + post, whole):
            if changed is not None:
                changed[(kinds, "Archivo", "(File tab, kept)")] += 1
        elif low in WORDS:
            new[i] = match_case(plain[i], WORDS[low])
        elif low in FEMININE:
            new[i] = match_case(plain[i], FEMININE[low])
        elif PRESS.match(low):
            new[i] = match_case(plain[i], PRESS.sub("puls", low))
    gaps = [text[(toks[i - 1].end() if i else 0):toks[i].start()] for i in range(len(toks))]
    for i in range(len(toks)):
        if new[i] is None or plain[i].lower() not in FEMININE:
            continue
        # Agreement: the article before changes with the noun (la computadora, el ordenador); an adjective beside it we cannot change, so the word stays.
        before = plain[i - 1].lower() if i and gaps[i] == " " else ""
        after = plain[i + 1].lower() if i + 1 < len(toks) and gaps[i + 1] == " " else ""
        if (before and before not in ARTICLE and before not in NO_AGREEMENT and before.endswith(("a", "as"))) or \
                (after and after not in NO_AGREEMENT and after.endswith(("a", "as", "o", "os"))):
            new[i] = None
            if left is not None:
                left.append((kinds, text[max(0, toks[i].start() - 20):toks[i].end() + 20]))
            continue
        if before in ARTICLE:
            new[i - 1] = match_case(plain[i - 1], ARTICLE[before])
            if before in ("la", "las") and i >= 2 and gaps[i - 1] == " " and plain[i - 2].lower() in CONTRACTION and before == "la":
                new[i - 2] = match_case(plain[i - 2], CONTRACTION[plain[i - 2].lower()])   # de la -> del, a la -> al
                new[i - 1] = ""
                gaps[i] = ""
    out = []
    for i, m in enumerate(toks):
        out.append(gaps[i])
        word = m.group() if new[i] is None else new[i]
        if new[i] and "&" in m.group():
            # The shortcut keeps its letter: the same one nearest to where it was.
            idx = m.group().index("&")
            letter = plain[i][idx] if idx < len(plain[i]) else ""
            spots = [j for j, c in enumerate(word) if c == letter]
            if spots:
                at = min(spots, key=lambda j: (abs(j - idx), j))
                word = word[:at] + "&" + word[at:]
        out.append(word)
        if changed is not None and new[i]:
            changed[(kinds, plain[i], new[i])] += 1
    out.append(text[(toks[-1].end() if toks else 0):])
    return "".join(out)


def respell_text(text, kinds, changed=None, left=None):
    """A text with markup, placeholders, and code kept as they are."""
    whole = re.sub(r"[&\s]", "", text).lower() == "archivo"
    parts = KEEP.split(text)
    # (KEEP.split keeps what matched at the odd places; a bold "**" or a "[" is not in KEEP, so the neighbors see them)
    for i in range(0, len(parts), 2):
        # (what stands around the run, markup included, tells whether "Archivo" is the File tab: "Archivo &gt; Opciones")
        parts[i] = respell_plain(parts[i], kinds, whole, changed, left, "".join(parts[:i])[-40:], "".join(parts[i + 1:])[:40])
    return "".join(parts)


# ----- the program's text -----

def read_es(path):
    """[(context, source, comment, numerus, type, [translations])] for the translated texts of a .ts file."""
    out = []
    for ctx in ET.parse(path).getroot().iter("context"):
        name = ctx.findtext("name") or ""
        for m in ctx.iter("message"):
            tr = m.find("translation")
            if tr is None or tr.get("type") in ("vanished", "obsolete"):
                continue
            numerus = m.get("numerus") == "yes"
            forms = [f.text or "" for f in tr.iter("numerusform")] if numerus else [tr.text or ""]
            if not "".join(forms):
                continue
            out.append((name, m.findtext("source") or "", m.findtext("comment") or "", numerus, tr.get("type"), forms))
    return out


def ui_file(changed=None, left=None):
    contexts = {}
    count = 0
    for ctx, source, comment, numerus, kind, forms in read_es(os.path.join(ROOT, "translations", "jeffoffice_es.ts")):
        new = [respell_text(f, "UI", changed, left) for f in forms]
        if new == forms:
            continue
        contexts.setdefault(ctx, []).append((source, comment, numerus, kind, new))
        count += 1
    lines = ['<?xml version="1.0" encoding="utf-8"?>', "<!DOCTYPE TS>", '<TS version="2.1" language="%s">' % LANG]
    for ctx in sorted(contexts):
        lines += ["<context>", "    <name>%s</name>" % escape(ctx)]
        for source, comment, numerus, kind, forms in contexts[ctx]:
            lines.append('    <message numerus="yes">' if numerus else "    <message>")
            lines.append("        <source>%s</source>" % escape(source))
            if comment:
                lines.append("        <comment>%s</comment>" % escape(comment))
            attr = ' type="%s"' % kind if kind else ""
            if numerus:
                lines.append("        <translation%s>" % attr)
                lines += ["            <numerusform>%s</numerusform>" % escape(f) for f in forms]
                lines.append("        </translation>")
            else:
                lines.append("        <translation%s>%s</translation>" % (attr, escape(forms[0])))
            lines.append("    </message>")
        lines.append("</context>")
    lines.append("</TS>")
    return "\n".join(lines) + "\n", count


# ----- Help -----

def help_page(md, changed=None, left=None):
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
            # Both words find the page: the neutral ones stay, Spain's are added.
            def both(m):
                words = m.group(2)
                extra = respell_text(words, "Help", changed, left)
                if extra == words:
                    return m.group(0)
                mine = [w.strip() for w in words.split(",")]
                add_ = [w.strip() for w in extra.split(",") if w.strip() not in mine]
                return m.group(1) + words.rstrip() + ", " + ", ".join(add_) + " " + m.group(3)
            out.append(KEYWORDS.sub(both, line))
            continue
        pieces = LINK_TARGET.split(line)
        for i in range(0, len(pieces), 2):
            pieces[i] = respell_text(pieces[i], "Help", changed, left)
        out.append("".join(pieces))
    return "\n".join(out)


def help_files(changed=None, left=None):
    """{relative path: text} for the pages that differ."""
    base = os.path.join(ROOT, "resources", "help", "es")
    out = {}
    for d, _, files in os.walk(base):
        for f in sorted(files):
            if not f.endswith(".md"):
                continue
            path = os.path.join(d, f)
            with open(path, encoding="utf-8") as fh:
                md = fh.read()
            new = help_page(md, changed, left)
            if new != md:
                out[os.path.relpath(path, base).replace(os.sep, "/")] = new
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--check", action="store_true", help="change nothing; exit 1 if a file is out of date")
    ap.add_argument("--words", action="store_true", help="list each changed word and how often")
    ap.add_argument("--ui-only", action="store_true")
    ap.add_argument("--help-only", action="store_true")
    args = ap.parse_args()
    stale = []
    changed = collections.Counter()
    left = []
    if not args.help_only:
        text, count = ui_file(changed, left)
        path = os.path.join(ROOT, "translations", "jeffoffice_%s.ts" % LANG)
        if args.check:
            if not os.path.exists(path) or open(path, encoding="utf-8", newline="").read() != text:
                stale.append(path)
        elif write_if_changed(path, text):
            print("%s: %d texts" % (os.path.relpath(path, ROOT), count))
    if not args.ui_only:
        pages = help_files(changed, left)
        folder = os.path.join(ROOT, "resources", "help", LANG)
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
            print("resources/help/%s: %d pages%s" % (LANG, len(pages), " (%d written)" % n if n else ""))
    if args.words:
        rows = collections.defaultdict(lambda: {"UI": 0, "Help": 0})
        for (kind, old, new), n in changed.items():
            rows[(old.lower(), new.lower())][kind] += n
        for (old, new), n in sorted(rows.items()):
            print("%5d UI %5d Help  %s -> %s" % (n["UI"], n["Help"], old, new))
        for kind, context in left:
            print("left alone (agreement), %s: ...%s..." % (kind, context))
    if args.check:
        if stale:
            print("out of date (run tools/translate/spanish_variants.py):\n  " + "\n  ".join(stale))
            return 1
        print("Spanish (Spain) is current.")
        return 0
    if not args.ui_only and not args.help_only:
        subprocess.run([sys.executable, os.path.join(ROOT, "tools", "help_files.py")], check=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
