#!/usr/bin/env python3
"""Translates JeffPub's text with Claude, through the Anthropic API.

Two parts:
  ui    the program's strings, translations/jeffoffice_<language>.ts
  help  Help's topics, resources/help/<language>/<topic>.md

Only what is new or changed is sent. Every translation then goes through
automatic checks (placeholders, plural forms, links...) and a second pass
that translates it back into English and flags any whose meaning drifted;
the reviewer's correction is used when it passes the same checks.

translations/machine/<language>.json records what the machine wrote. A
translation that differs from the record was changed by a person: it is
never overwritten, and a Help topic whose English changed under a person's
version is listed in the report instead. Terms are kept the same everywhere
by translations/glossary/<language>.json (made on the first run; a person
may correct it, and a corrected term is never asked for again).

    python3 tools/translate/translate.py [--lang de,fr|all] [--part ui|help|all]
                                         [--dry-run] [--report FILE]

Without --lang it keeps up the languages already started (each has its
translations/jeffoffice_<language>.ts); name a language to start it.

Needs ANTHROPIC_API_KEY (except with --dry-run), the anthropic package
(pip install -r tools/translate/requirements.txt), and Qt's lconvert
(found through QT_ROOT_DIR, PATH, or ~/Qt).
"""
import argparse
import concurrent.futures as cf
import glob
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import threading
import xml.etree.ElementTree as ET

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TS_DIR = os.path.join(ROOT, "translations")
HELP_DIR = os.path.join(ROOT, "resources", "help")
HERE = os.path.dirname(os.path.abspath(__file__))

MODEL = "claude-opus-5-5"
WORKERS = 4

# The languages, in Qt's names (the .ts file's and Help folder's suffix).
# Each has two plural forms in Qt: the singular, then the plural.
LANGUAGES = {
    "de": "German",
    "es": "Spanish",
    "fr": "French",
    "it": "Italian",
    "nl": "Dutch",
    "pt_BR": "Brazilian Portuguese",
    "pt_PT": "European Portuguese (Portugal)",
}

# Dollars per million tokens: input, output, cache read, cache write (5 minutes).
PRICES = {
    "claude-opus-5-5": (4.0, 20.0, 0.20, 5.0),
    "claude-opus-5": (5.0, 25.0, 0.50, 6.25),
    "claude-opus-4-8": (5.0, 25.0, 0.50, 6.25),
}

BANNED = re.compile(r"\b(Microsoft|Publisher|Office|Word|Excel|PowerPoint|WordArt)\b")


def sha(text):
    return hashlib.sha1(text.encode("utf-8")).hexdigest()[:16]


# ---------- the API ----------

class Spend:
    """Tokens and dollars of every request."""

    def __init__(self):
        self.lock = threading.Lock()
        self.requests = 0
        self.tokens = [0, 0, 0, 0]   # input, output, cache read, cache write
        self.dollars = 0.0

    def add(self, model, usage):
        price = PRICES.get(model, PRICES[MODEL])
        t = [usage.input_tokens or 0, usage.output_tokens or 0,
             getattr(usage, "cache_read_input_tokens", 0) or 0,
             getattr(usage, "cache_creation_input_tokens", 0) or 0]
        with self.lock:
            self.requests += 1
            self.tokens = [a + b for a, b in zip(self.tokens, t)]
            self.dollars += sum(n * p for n, p in zip(t, price)) / 1e6

    def line(self):
        i, o, r, w = self.tokens
        return (f"{self.requests} requests; {i:,} input, {r:,} cached, {w:,} written to cache, "
                f"{o:,} output tokens; ${self.dollars:.2f}")


SPEND = Spend()


class Unusable(Exception):
    """A response that can't be used as it is (refused, cut short)."""


class Api:
    def __init__(self, effort):
        import anthropic
        self.client = anthropic.Anthropic(max_retries=6)
        self.effort = effort

    def ask(self, system, user, schema=None, max_tokens=16000):
        output = {"effort": self.effort}
        if schema:
            output["format"] = {"type": "json_schema", "schema": schema}
        # Streamed: a long Help topic can take longer than a plain request may.
        with self.client.beta.messages.stream(
            model=MODEL,
            max_tokens=max_tokens,
            system=[{"type": "text", "text": system, "cache_control": {"type": "ephemeral"}}],
            messages=[{"role": "user", "content": user}],
            output_config=output,
            betas=["server-side-fallback-2026-07-01"],
            fallbacks="default",
        ) as stream:
            r = stream.get_final_message()
        SPEND.add(r.model, r.usage)
        if r.stop_reason == "refusal":
            raise Unusable("declined")
        if r.stop_reason == "max_tokens":
            raise Unusable("cut short")
        text = "".join(b.text for b in r.content if b.type == "text")
        return json.loads(text) if schema else text


def strict(properties):
    """A JSON schema object whose every property is required."""
    return {"type": "object", "properties": properties, "required": list(properties), "additionalProperties": False}


STR = {"type": "string"}
STRS = {"type": "array", "items": STR}


# ---------- Qt's translation files ----------

def find_qt_tool(name):
    for d in [os.path.join(os.environ.get("QT_ROOT_DIR", ""), "bin")] + sorted(glob.glob(os.path.expanduser("~/Qt/*/*/bin")), reverse=True):
        p = os.path.join(d, name)
        if os.path.isfile(p):
            return p
    p = shutil.which(name) or shutil.which(name + "6")
    if p:
        return p
    sys.exit(f"Qt's {name} not found (set QT_ROOT_DIR)")


def lconvert(*args):
    subprocess.run([find_qt_tool("lconvert"), *args], check=True)


def ts_path(lang):
    return os.path.join(TS_DIR, f"jeffoffice_{lang}.ts")


class Message:
    def __init__(self, context, elem, filename):
        self.context = context
        self.elem = elem
        self.source = elem.findtext("source") or ""
        self.comment = elem.findtext("comment") or ""
        self.note = elem.findtext("extracomment") or ""
        self.filename = filename
        self.plural = elem.get("numerus") == "yes"
        self.key = sha(f"{context}\x1f{self.source}\x1f{self.comment}")
        tr = elem.find("translation")
        self.type = tr.get("type") if tr is not None else "unfinished"
        if self.plural:
            self.current = [f.text or "" for f in tr.findall("numerusform")] if tr is not None else []
        else:
            self.current = (tr.text or "") if tr is not None else ""

    def empty(self):
        return not any(self.current) if self.plural else not self.current

    def set(self, out):
        tr = self.elem.find("translation")
        if tr is None:
            tr = ET.SubElement(self.elem, "translation")
        for f in list(tr):
            tr.remove(f)
        tr.text = None
        if self.plural:
            for form in out:
                ET.SubElement(tr, "numerusform").text = form
        else:
            tr.text = out
        tr.set("type", "unfinished")   # a machine's: shown to reviewers as not yet checked
        self.current = out


def load_ts(lang, dry=False):
    path = ts_path(lang)
    if not os.path.exists(path):
        if dry:   # nothing written: count from the list of every string
            path = os.path.join(TS_DIR, "jeffoffice.ts")
        else:
            lconvert("-target-language", lang, "-i", os.path.join(TS_DIR, "jeffoffice.ts"), "-o", path)
    tree = ET.parse(path)
    messages = []
    for ctx in tree.getroot().findall("context"):
        name = ctx.findtext("name")
        filename = ""
        for m in ctx.findall("message"):
            for loc in m.findall("location"):
                if loc.get("filename"):
                    filename = loc.get("filename")
                    break
            messages.append(Message(name, m, os.path.basename(filename)))
    return tree, [m for m in messages if m.type not in ("vanished", "obsolete")]


def save_ts(tree, lang):
    with tempfile.NamedTemporaryFile("wb", suffix=".ts", delete=False) as f:
        tree.write(f, encoding="utf-8", xml_declaration=True)
    try:
        lconvert("-i", f.name, "-o", ts_path(lang))   # Qt's own layout, as lupdate writes it
    finally:
        os.unlink(f.name)


# ---------- checks a translation must pass ----------

PLACEHOLDER = re.compile(r"%L?\d+|%n")
# A keyboard shortcut marker: an & right before the letter it underlines.
# "Header & Footer" has none: that & is the word "and".
MARKER = re.compile(r"(?<!&)&(?=[^\s&])")
TAG = re.compile(r"</?[A-Za-z][^>]*>")


def check(source, out, plural):
    """What's wrong with a translation of `source`, or an empty string."""
    forms = out if plural else [out]
    if plural and len(forms) != 2:
        return "give exactly 2 forms: singular, then plural"
    if any(not f.strip() for f in forms):
        return "empty translation"
    want = sorted(PLACEHOLDER.findall(source))
    for i, f in enumerate(forms):
        have = sorted(PLACEHOLDER.findall(f))
        # A singular form may spell out the number instead of %n.
        if plural and i == 0 and have == sorted(p for p in want if p != "%n"):
            continue
        if have != want:
            return f"placeholders must be {' '.join(want) or 'none'}, found {' '.join(have) or 'none'}"
        if sorted(TAG.findall(f)) != sorted(TAG.findall(source)):
            return "keep the HTML tags exactly"
        if f.count("\n") != source.count("\n"):
            return "keep the line breaks"
        if len(MARKER.findall(source)) != len(MARKER.findall(f)):
            return "keep exactly the & shortcut markers the English has (an & right before a letter)"
        if (source[:1] == " ") != (f[:1] == " ") or (source[-1:] == " ") != (f[-1:] == " "):
            return "keep leading and trailing spaces"
        if source.rstrip().endswith(("…", "...")) and not f.rstrip().endswith(("…", "...")):
            return "keep the ellipsis at the end"
        if source.rstrip().endswith(":") and not f.rstrip().endswith(":"):
            return "keep the colon at the end"
        if m := BANNED.search(f):
            if not BANNED.search(source):
                return f"don't name {m.group(1)}"
    return ""


# ---------- the glossary ----------

def term_list():
    terms = []
    with open(os.path.join(HERE, "terms.txt"), encoding="utf-8") as f:
        terms = [line.strip() for line in f if line.strip() and not line.startswith("#")]
    with open(os.path.join(ROOT, "resources", "ribbon.json"), encoding="utf-8") as f:
        ribbon = json.load(f)
    groups = list(ribbon.get("templates", {}).values())
    for tab in ribbon["tabs"]:
        terms.append(tab["name"])
        groups += [g for g in tab["groups"] if "name" in g]
    terms += [g["name"] for g in groups]
    seen, out = set(), []
    for t in terms:
        if t.lower() not in seen:
            seen.add(t.lower())
            out.append(t)
    return out


def glossary(lang, api, dry):
    path = os.path.join(TS_DIR, "glossary", f"{lang}.json")
    have = {}
    if os.path.exists(path):
        with open(path, encoding="utf-8") as f:
            have = json.load(f)
    missing = [t for t in term_list() if t not in have]
    if missing and not dry:
        system = (f"You name the standard {LANGUAGES[lang]} terms used in the menus, ribbon tabs, and dialog boxes of "
                  f"office and desktop publishing software. For each English term, give the term a {LANGUAGES[lang]} "
                  "user sees in such programs, as it would appear in a menu or on a ribbon tab (keep its capitalization "
                  "style for that language). Product names stay as they are; never use the name Microsoft, Publisher, "
                  "Office, or WordArt. Text Art is this program's styled-text feature: translate it as a descriptive "
                  "name, not a trademark.")
        schema = strict({"terms": {"type": "array", "items": strict({"en": STR, "tr": STR})}})
        got = api.ask(system, "Terms:\n" + "\n".join(missing), schema)
        for t in got["terms"]:
            if t["en"] in missing and t["tr"].strip():
                have[t["en"]] = t["tr"].strip()
        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, "w", encoding="utf-8") as f:
            json.dump(dict(sorted(have.items(), key=lambda kv: kv[0].lower())), f, ensure_ascii=False, indent=1)
            f.write("\n")
    return have


def glossary_text(lang, terms):
    return "\n".join(f"{en} = {tr}" for en, tr in sorted(terms.items(), key=lambda kv: kv[0].lower()))


# ---------- the program's strings ----------

def ui_system(lang, terms):
    name = LANGUAGES[lang]
    return f"""You translate the user interface of JeffPub, a free desktop publishing program for Windows, Linux, and macOS, from American English into {name}.

How to translate:
- Use the words a {name} user expects from mainstream office and desktop publishing software in {name}: the usual terms for commands, menus, tabs, and dialog boxes. Prefer the established software term over a literal translation.
- Use the glossary below for every term it lists, in every string, so the program stays consistent.
- Keep each string's role. A command or button label stays short, in the grammatical form {name} software uses for such labels. A sentence stays a sentence; a title stays a title. Capitalize as {name} interfaces do (English Title Case does not carry over).
- Keep every placeholder exactly as written (%1, %2, %L1, %n); you may move them. Keep HTML tags and entities, line breaks (\\n), and leading and trailing spaces. Keep a trailing colon or ellipsis.
- An "&" right before a letter ("&Open") marks a keyboard shortcut: keep the same number of them, each before a letter of the translated word. An "&" with spaces around it ("Header & Footer") is just the word "and": write it as {name} interfaces do, and never turn it into a shortcut marker.
- Product and format names stay as they are: JeffPub, JeffOffice, PDF, PDF/A, PDF/X, EPUB, SVG, PNG, JPEG, ISBN, EAN, UPC, CMYK, RGB, HSL, PANTONE, OpenType, Unicode, Markdown. Never name Microsoft, Publisher, Office, Word, Excel, PowerPoint, or WordArt.
- Key names: as printed on {name} keyboards and written in {name} documentation (German, for example, writes Strg for Ctrl and Umschalt for Shift).
- An item with "plural": true contains %n. Put its forms in "forms": first the singular, then the plural; leave "text" empty. For every other item, put the translation in "text" and leave "forms" empty.
- Each item says where it appears ("context" is the part of the program, "file" its source file) and may carry a developer's "note". Use them to pick the right meaning. Strings from "Templates" are sample text in built-in designs (business names, slogans, placeholder text): translate them naturally for {name} readers.
- Return every item, with its id.

Glossary (English = {name}):
{glossary_text(lang, terms)}
"""


UI_SCHEMA = strict({"items": {"type": "array", "items": strict({"id": STR, "text": STR, "forms": STRS})}})
REVIEW_SCHEMA = strict({"items": {"type": "array", "items": strict(
    {"id": STR, "back": STR, "ok": {"type": "boolean"}, "problem": STR, "fix": STR, "fix_forms": STRS})}})


def review_system(lang, terms):
    name = LANGUAGES[lang]
    return f"""You check translations of JeffPub's user interface (a desktop publishing program) from American English into {name}.

For each item:
- "back": translate the {name} text back into English, literally, without looking at the original.
- "ok": true when the translation means what the English means in that place in the program, reads as natural {name} interface text, and uses the glossary's terms. Do not object to word order, capitalization, or wording choices that are correct.
- When "ok" is false: "problem" says what is wrong in one sentence, and "fix" (or "fix_forms" for a plural: singular, then plural) gives the corrected translation, keeping placeholders (%1, %n...), HTML tags, line breaks, "&" shortcut markers (an & right before a letter; an & between spaces is just "and"), and leading and trailing spaces. When "ok" is true, leave "problem", "fix", and "fix_forms" empty.
- Return every item, with its id.

Glossary (English = {name}):
{glossary_text(lang, terms)}
"""


def chunks(messages, size=70, chars=7000):
    out, cur, n = [], [], 0
    for m in sorted(messages, key=lambda m: m.context):
        if cur and (len(cur) >= size or n + len(m.source) > chars):
            out.append(cur)
            cur, n = [], 0
        cur.append(m)
        n += len(m.source) + len(m.note)
    if cur:
        out.append(cur)
    return out


def item(i, m):
    d = {"id": str(i), "context": m.context, "file": m.filename, "text": m.source}
    if m.plural:
        d["plural"] = True
    if m.comment or m.note:
        d["note"] = " ".join(x for x in (m.comment, m.note) if x)
    return d


def translate_chunk(api, system, chunk, report):
    """{message: translation} for the messages it could translate."""
    done, notes = {}, {}
    for attempt in range(2):
        todo = [m for m in chunk if m not in done]
        if not todo:
            break
        items = []
        for i, m in enumerate(todo):
            d = item(i, m)
            if m in notes:
                d["fix_this"] = notes[m]
            items.append(d)
        try:
            got = api.ask(system, "Translate these items:\n" + json.dumps(items, ensure_ascii=False, indent=0), UI_SCHEMA)
        except Exception as e:   # the rest of the run goes on; these stay untranslated
            report.problem(f"{todo[0].context}: {len(todo)} strings not translated ({e})")
            return done
        by_id = {g["id"]: g for g in got["items"]}
        for i, m in enumerate(todo):
            g = by_id.get(str(i))
            if g is None:
                notes[m] = "missing from the last answer"
                continue
            out = g["forms"] if m.plural else g["text"]
            why = check(m.source, out, m.plural)
            if why:
                notes[m] = f"the last translation {json.dumps(out, ensure_ascii=False)} was rejected: {why}"
            else:
                done[m] = out
    for m in chunk:
        if m not in done:
            report.problem(f"not translated: [{m.context}] {m.source!r}: {notes.get(m, '')}")
    return done


def review_chunk(api, system, pairs, report):
    """Corrections the reviewer made that pass the checks: {message: translation}."""
    items = []
    for i, (m, out) in enumerate(pairs):
        d = item(i, m)
        d["translation"] = out
        items.append(d)
    try:
        got = api.ask(system, "Check these translations:\n" + json.dumps(items, ensure_ascii=False, indent=0), REVIEW_SCHEMA)
    except Exception as e:
        report.problem(f"{pairs[0][0].context}: {len(pairs)} strings not reviewed ({e})")
        return {}
    fixes = {}
    for g in got["items"]:
        if not g["id"].isdigit() or int(g["id"]) >= len(pairs) or g["ok"]:
            continue
        m, out = pairs[int(g["id"])]
        fix = g["fix_forms"] if m.plural else g["fix"]
        used = bool(fix) and not check(m.source, fix, m.plural)
        if used:
            fixes[m] = fix
        report.flag(m, out, g["back"], g["problem"], fix if used else None)
    return fixes


def run_ui(lang, api, dry, report):
    terms = glossary(lang, api, dry)
    tree, messages = load_ts(lang, dry)
    state = load_state(lang)
    record = state.setdefault("ui", {})
    todo = []
    for m in messages:
        recorded = record.get(m.key)
        if m.empty():
            todo.append(m)
        elif recorded is not None:
            if recorded != sha(json.dumps(m.current, ensure_ascii=False)):
                report.human += 1   # a person's correction: kept
        elif m.type == "unfinished":
            todo.append(m)   # lupdate's guess from similar text, or nothing a person approved
    report.note(f"Program strings: {len(messages)}, to translate: {len(todo)} ({sum(len(m.source.split()) for m in todo):,} words)")
    if dry or not todo:
        return
    system, rsystem = ui_system(lang, terms), review_system(lang, terms)
    results = {}
    with cf.ThreadPoolExecutor(WORKERS) as pool:
        for done in pool.map(lambda c: translate_chunk(api, system, c, report), chunks(todo)):
            results.update(done)
        pairs = [(m, results[m]) for m in todo if m in results]
        batches = [pairs[i:i + 70] for i in range(0, len(pairs), 70)]
        for fixes in pool.map(lambda b: review_chunk(api, rsystem, b, report), batches):
            results.update(fixes)
    for m, out in results.items():
        m.set(out)
        record[m.key] = sha(json.dumps(out, ensure_ascii=False))
    report.translated += len(results)
    save_ts(tree, lang)
    save_state(lang, state)


# ---------- Help ----------

LINK = re.compile(r"\]\(([^)\s]+)\)")
HEADING = re.compile(r"^#{1,6} ", re.M)
COMMENT = re.compile(r"<!--\s*(\w+)")


def check_help(en, out):
    if out.strip().startswith("```"):
        return "the answer is wrapped in a code block"
    if not out.startswith("# "):
        return "the first line must be '# ' and the title"
    if sorted(LINK.findall(en)) != sorted(LINK.findall(out)):
        return "link targets must stay exactly the same"
    if len(HEADING.findall(en)) != len(HEADING.findall(out)):
        return "keep the same headings"
    if sorted(COMMENT.findall(en)) != sorted(COMMENT.findall(out)):
        return "keep the <!-- --> comments"
    if m := BANNED.search(out):
        return f"don't name {m.group(1)}"
    return ""


def ui_table(lang):
    """The program's own text, so Help names each command as the program shows it."""
    _, messages = load_ts(lang)
    rows = {}
    for m in sorted(messages, key=lambda m: (m.context != "Ribbon", m.context)):
        if m.plural or m.empty() or "%" in m.source or len(m.source.split()) > 8 or "\n" in m.source:
            continue
        tr = rows.setdefault(m.source.strip(), [])
        if m.current.strip() not in tr:
            tr.append(m.current.strip())
    # An English word the program translates two ways (a command and a
    # noun: "Group") lists both, for the Help to pick by context.
    return "\n".join(f"{en} = {' | '.join(tr)}" for en, tr in sorted(rows.items(), key=lambda kv: kv[0].lower()))


def help_system(lang, terms, table):
    name = LANGUAGES[lang]
    return f"""You translate the Help of JeffPub, a free desktop publishing program, from American English into {name}. Each request is one Help topic, a Markdown file.

How to translate:
- Answer with the translated file only: nothing before or after it, and no code block around it.
- Keep the structure exactly: the same headings, paragraphs, lists, tables, bold and italic text, and blank lines.
- The first line is "# " and the topic's title: translate the title.
- "<!-- keywords: a, b, c -->": keep the markers, and translate the keywords into words a {name} user would search for (add common {name} synonyms if useful).
- Keep "<!-- shortcuts -->" and every other <!-- --> comment exactly as it is.
- Links: in [text](target), translate the text and never change the target (topic names such as linked-text-boxes, and web addresses).
- When the text names a command, tab, group, button, box, or dialog, use exactly the {name} label the program shows, from "Program text" below (where it lists two, separated by |, pick the one that fits: the command or the noun); for one not listed, translate consistently with the glossary.
- Keyboard shortcuts: write key names as {name} keyboards print them (German, for example, writes Strg+C), keeping the + between keys.
- Measurements: {name} readers measure in centimeters, and JeffPub shows them centimeters unless they choose otherwise. Give lengths, sizes, and distances in centimeters (millimeters for small ones), converting from inches (1 inch = 2.54 cm) and rounding as {name} documentation would (2 inches becomes 5 cm; a quarter inch, 0.6 cm). Where an example depends on US paper sizes (Letter, half-letter), use A4 and A5 instead when the example works the same way. Units that name a measure stay as they are: points, picas, ppi and dpi (pixels or dots per inch), and a point being 1/72 inch.
- Keep file names, file extensions, code, and web addresses as they are. Product and format names stay as they are (JeffPub, PDF, EPUB, CMYK...). Never name Microsoft, Publisher, Office, Word, Excel, PowerPoint, or WordArt.
- Write plainly, as good {name} software documentation does, addressing the reader the way {name} software documentation usually does.

Glossary (English = {name}):
{glossary_text(lang, terms)}

Program text (English = {name}):
{table}
"""


HELP_REVIEW_SCHEMA = strict({"ok": {"type": "boolean"}, "problems": {"type": "array", "items": strict(
    {"english": STR, "translated": STR, "back": STR, "problem": STR})}})


def help_review_system(lang, terms, table):
    name = LANGUAGES[lang]
    return f"""You check a {name} translation of a Help topic of JeffPub, a desktop publishing program, against its American English original.

List each place where the translation's meaning drifted: something said differently, left out, or added; a command, tab, button, or dialog named differently from "Program text" below; a link target changed. For each, quote the English and the {name}, give "back" (the {name} translated literally back into English), and say what's wrong in one sentence. Do not list matters of style or word choice that are correct. "ok" is true when there is nothing to list.

Glossary (English = {name}):
{glossary_text(lang, terms)}

Program text (English = {name}):
{table}
"""


def translate_topic(api, system, rsystem, tid, en, report):
    notes = ""
    for attempt in range(2):
        try:
            out = api.ask(system, f"Topic \"{tid}\":\n\n{en}" + (f"\n\nA reviewer found these problems in your last translation; avoid them:\n{notes}" if notes else ""), max_tokens=32000)
        except Exception as e:
            report.problem(f"Help topic {tid}: not translated ({e})")
            return None
        out = out.strip() + "\n"
        why = check_help(en, out)
        if why:
            notes = why
            continue
        try:
            review = api.ask(rsystem, f"English:\n\n{en}\n\n{LANGUAGES_LABEL}:\n\n{out}", HELP_REVIEW_SCHEMA)
        except Exception as e:
            report.problem(f"Help topic {tid}: not reviewed ({e})")
            return out
        if review["ok"] or not review["problems"]:
            return out
        if attempt == 1:
            for p in review["problems"]:
                report.help_flags.append((tid, p))
            return out
        notes = "\n".join(f"- \"{p['translated']}\" (for \"{p['english']}\"): {p['problem']}" for p in review["problems"])
    report.problem(f"Help topic {tid}: not translated ({notes})")
    return None


LANGUAGES_LABEL = "Translation"


def run_help(lang, api, dry, report):
    terms = glossary(lang, api, dry)
    state = load_state(lang)
    record = state.setdefault("help", {})
    out_dir = os.path.join(HELP_DIR, lang)
    todo = []
    for path in sorted(glob.glob(os.path.join(HELP_DIR, "en", "*.md"))):
        tid = os.path.basename(path)[:-3]
        with open(path, encoding="utf-8") as f:
            en = f.read()
        target = os.path.join(out_dir, tid + ".md")
        rec = record.get(tid)
        if not os.path.exists(target):
            todo.append((tid, en))
            continue
        with open(target, encoding="utf-8") as f:
            mine = sha(f.read())
        if rec and rec["en"] == sha(en):
            continue
        if rec and rec["out"] == mine:
            todo.append((tid, en))   # the English changed under the machine's version
        else:
            report.problem(f"Help topic {tid}: the English changed, but a person edited the {LANGUAGES[lang]}; update it by hand")
    report.note(f"Help topics: {len(todo)} to translate ({sum(len(en.split()) for _, en in todo):,} words)")
    if dry or not todo:
        return
    table = ui_table(lang)
    system, rsystem = help_system(lang, terms, table), help_review_system(lang, terms, table)
    os.makedirs(out_dir, exist_ok=True)
    with cf.ThreadPoolExecutor(WORKERS) as pool:
        for (tid, en), out in zip(todo, pool.map(lambda t: translate_topic(api, system, rsystem, t[0], t[1], report), todo)):
            if out is None:
                continue
            with open(os.path.join(out_dir, tid + ".md"), "w", encoding="utf-8") as f:
                f.write(out)
            record[tid] = {"en": sha(en), "out": sha(out)}
            report.topics += 1
    save_state(lang, state)
    subprocess.run([sys.executable, os.path.join(ROOT, "tools", "help_files.py")], check=True, stdout=subprocess.DEVNULL)


# ---------- state and report ----------

def state_path(lang):
    return os.path.join(TS_DIR, "machine", f"{lang}.json")


def load_state(lang):
    if os.path.exists(state_path(lang)):
        with open(state_path(lang), encoding="utf-8") as f:
            return json.load(f)
    return {}


def save_state(lang, state):
    os.makedirs(os.path.dirname(state_path(lang)), exist_ok=True)
    state = {part: dict(sorted(v.items())) for part, v in sorted(state.items())}
    with open(state_path(lang), "w", encoding="utf-8") as f:
        json.dump(state, f, ensure_ascii=False, indent=0, sort_keys=True)
        f.write("\n")


class Report:
    def __init__(self, lang):
        self.lang = lang
        self.lock = threading.Lock()
        self.lines, self.problems, self.flags, self.help_flags = [], [], [], []
        self.translated = self.topics = self.human = 0

    def note(self, line):
        print(f"[{self.lang}] {line}", flush=True)
        self.lines.append(line)

    def problem(self, line):
        with self.lock:
            print(f"[{self.lang}] {line}", flush=True)
            self.problems.append(line)

    def flag(self, m, out, back, problem, fix):
        with self.lock:
            self.flags.append((m, out, back, problem, fix))

    def markdown(self):
        name = LANGUAGES[self.lang]
        md = [f"## {name} ({self.lang})", ""] + [f"- {line}" for line in self.lines]
        md += [f"- Translated {self.translated} strings and {self.topics} Help topics; "
               f"{self.human} strings keep a person's correction."]
        if self.flags:
            md += ["", f"### Strings the review flagged ({len(self.flags)})", "",
                   "| English | First translation | Back into English | Problem | Used instead |", "| --- | --- | --- | --- | --- |"]
            cell = lambda s: (" / ".join(s) if isinstance(s, list) else (s or "")).replace("|", "\\|").replace("\n", " ")
            for m, out, back, problem, fix in self.flags:
                md.append(f"| {cell(m.source)} | {cell(out)} | {cell(back)} | {cell(problem)} | {cell(fix) if fix else 'kept the first'} |")
        if self.help_flags:
            md += ["", f"### Help passages still flagged after a second try ({len(self.help_flags)})", ""]
            md += [f"- {tid}: \"{p['translated']}\" (back: \"{p['back']}\"): {p['problem']}" for tid, p in self.help_flags]
        if self.problems:
            md += ["", "### Problems", ""] + [f"- {p}" for p in self.problems]
        return "\n".join(md) + "\n"


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--lang", help="languages, comma-separated, or \"all\" (default: the languages already "
                    "started, each with its translations/jeffoffice_<language>.ts; start a new one by naming it)")
    ap.add_argument("--part", choices=["ui", "help", "all"], default="all")
    ap.add_argument("--dry-run", action="store_true", help="count what would be sent; call nothing")
    ap.add_argument("--effort", default="low", choices=["low", "medium", "high"])
    ap.add_argument("--report", help="write a Markdown report here")
    args = ap.parse_args()
    if not args.lang:
        langs = [l for l in LANGUAGES if os.path.exists(ts_path(l))]
    elif args.lang == "all":
        langs = list(LANGUAGES)
    else:
        langs = [l for l in args.lang.split(",") if l]
    for l in langs:
        if l not in LANGUAGES:
            sys.exit(f"unknown language {l}; known: {', '.join(LANGUAGES)}")
    if not args.dry_run and not os.environ.get("ANTHROPIC_API_KEY"):
        sys.exit("ANTHROPIC_API_KEY is not set")
    api = None if args.dry_run else Api(args.effort)
    reports = []
    for lang in langs:
        report = Report(lang)
        reports.append(report)
        if args.part in ("ui", "all"):
            run_ui(lang, api, args.dry_run, report)
        if args.part in ("help", "all"):
            run_help(lang, api, args.dry_run, report)
    total = "" if args.dry_run else SPEND.line()
    if total:
        print(total)
    if args.report:
        with open(args.report, "w", encoding="utf-8") as f:
            f.write("# Translation run\n\n" + (f"{total}\n\n" if total else "") + "\n".join(r.markdown() for r in reports))
    return 1 if any(r.problems for r in reports) else 0


if __name__ == "__main__":
    sys.exit(main())
