#!/usr/bin/env python3
"""Writes docs/features/public.json: the public, reworded, translated (English, Spanish, French) form of the feature
lists docs/features/{jeffpub,jeffdeck,jeffwrite,jeffcrunch}.md, which the website renders into its per-app feature pages
(the site's make_features.py reads this file; the site repo has no API code).

    python3 tools/features/public_features.py [--dry-run]       (needs ANTHROPIC_API_KEY unless --dry-run)

  1. Parses the lists. `- [x]` items are Features, `- [~]` items are "Coming next", `- [ ]` items are dropped, and so are
     tiers, `*(notes)*`, and the sections about testing, tiers, shared code, and open questions.
  2. Sends each new or changed item and heading to Claude (claude-sonnet-5-5), about 40 to a request, to be reworded as
     one or two plain public sentences and translated (Spanish "usted", French "vous"). Texts already in public.json are
     reused (keyed by the sha256 of the source text), so only what is new or changed is sent. It prints the count and an
     estimated cost first and stops if the estimate is over $10, and prints the actual tokens and cost afterwards
     (assumed $3 per million input tokens, $15 per million output tokens).
  3. Checks every reworded text in code for other products' and fonts' names (in any language); a text that fails is asked
     for once more, then the run stops and lists it.
  4. Writes public.json, without the texts whose source is gone.

Exit codes: 0 done; 1 stopped (some text broke the rules, or an error); 2 the service refused the key.
"""
import argparse
import concurrent.futures as cf
import hashlib
import json
import os
import re
import sys
import threading

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
FEATURES_DIR = os.path.join(ROOT, "docs", "features")
OUT = os.path.join(FEATURES_DIR, "public.json")
OVERRIDES = os.path.join(FEATURES_DIR, "public-overrides.json")

MODEL = "claude-sonnet-5-5"
PRICE_IN, PRICE_OUT = 3.0, 15.0     # dollars per million tokens (assumed Sonnet 5.5 pricing)
BATCH = 40
WORKERS = 4
MAX_ESTIMATE = 10.0
LANGS = ("en", "es", "fr")
APPS = {"jeffpub": "JeffPub", "jeffdeck": "JeffDeck", "jeffwrite": "JeffWrite", "jeffcrunch": "JeffCrunch"}
APPS = {k: {"name": v} for k, v in APPS.items()}

# ---------- the lists ----------

SKIP_H2 = ("The Preview bar", "The beta bar", "How the tiers", "Fidelity and testing", "Shared with other",
           "Left out on purpose", "Decided", "Open questions")
SKIP_H3 = ("What must survive a round trip",)
ITEM = re.compile(r"^- \[([x~ ])\] (.*)$")
SOON_TIERS = ("first release", "1.0")      # a [ ] item is "Coming soon" only with one of these tiers (not "later")
TIER = re.compile(r"^(?:\*\*(?:first release|1\.0|later)\*\*\s*)+")
ALT = re.compile(r"\*\*\(alt\)\*\*\s*")
ALT_NOTE = re.compile(r"\*\*\((?:alt:\s*)?(.*?)\)\*\*")
NEEDED = re.compile(r"\s*\*\*\(needed[^)]*\)\*\*", re.I)
NOTES = re.compile(r"\s*(?<!\*)\*\(.*?\)\*(?!\*)")


def sha(s):
    return hashlib.sha256(s.encode("utf-8")).hexdigest()


def item_note(raw):
    """The text of the line's *(...)* notes, joined, which says what is missing or how far a feature is."""
    notes = [m.group(0).strip()[2:-2].strip() for m in NOTES.finditer(NEEDED.sub("", raw))]
    return re.sub(r"\s+", " ", " | ".join(notes)).strip()


def clean_item(raw):
    t = NEEDED.sub("", raw.strip())
    t = NOTES.sub("", t)                       # notes can sit anywhere in a line, not only at its end
    t = TIER.sub("", t.strip())
    t = ALT.sub("", t)
    t = ALT_NOTE.sub(r"(\1)", t)
    t = t.replace("**", "")
    t = re.sub(r"\s+", " ", t).strip()
    t = re.sub(r"\s+([,.;:])", r"\1", t)
    if "*(" in t or ")*" in t:
        sys.exit("note not removed cleanly: " + raw[:160])
    return t


def parse_list(path):
    """Returns tabs: [ {h: heading, direct: [(status, text, note)], groups: [ {h, items} ]} ]. Status is done ([x]), partly ([~]), or todo ([ ] with a first release or 1.0 tier; the others are left out)."""
    tabs, tab, group = [], None, None
    for line in open(path, encoding="utf-8"):
        line = line.rstrip("\n")
        m = re.match(r"(#{2,3}) (.*)", line)
        if m:
            level, text = len(m.group(1)), m.group(2).strip()
            if level == 2:
                group = None
                if text.startswith(SKIP_H2):
                    tab = None
                else:
                    tab = {"h": text, "direct": [], "groups": []}
                    tabs.append(tab)
            elif tab is not None:
                if text.startswith(SKIP_H3):
                    group = {"h": None, "items": []}     # collected and thrown away
                else:
                    group = {"h": text, "items": []}
                    tab["groups"].append(group)
            continue
        m = ITEM.match(line)
        if tab is None:
            continue
        if m:
            tier = re.match(r"\*\*(first release|1\.0|later)\*\*", m.group(2))
            if m.group(1) in "x~" or (tier and tier.group(1) in SOON_TIERS):
                status = {"x": "done", "~": "partly", " ": "todo"}[m.group(1)]
                entry = (status, clean_item(m.group(2)), item_note(m.group(2)))
                (group["items"] if group else tab["direct"]).append(entry)
        elif line.startswith("- ") or line.startswith("  - "):
            sys.exit("unparsed list line in %s: %s" % (path, line[:120]))
    for t in tabs:
        t["groups"] = [g for g in t["groups"] if g["h"] is not None]
    return tabs


def readme_state(feat_dir):
    """app -> (state, version) from the README table, e.g. ("beta", "0.20.0")."""
    out = {}
    for line in open(os.path.join(feat_dir, "README.md"), encoding="utf-8"):
        m = re.match(r"\|\s*(Jeff\w+)\s*\|.*\|\s*(\w[\w ]*?)\s*\((\d+\.\d+\.\d+)\)\s*\|", line)
        if m:
            out[m.group(1).lower()] = (m.group(2), m.group(3))
    return out


def reviewed(path):
    for line in open(path, encoding="utf-8"):
        m = re.match(r"Last reviewed (\w+) (\d+), (\d{4})\.", line)
        if m:
            return m.groups()
    return None


# ---------- the rules on the reworded text ----------

_ALWAYS = ["Microsoft", "PowerPoint", "Calibri", "Cambria", "Arial", "Times New Roman", "Courier New", "Segoe",
           r"PivotTables?", "AutoSum", "AutoFit", "Format Painter", "AutoRecover", "Quick Access Toolbar", "Quick Parts",
           "Outlook", "WordArt", "SmartArt", "Tell Me", "AutoFilter", "Flash Fill", "jpubtool", "jdecktool", "jwritetool", "writetool",
           "jcrunchtool", "fuzz\\w*", "repair message", "corpus", "corpora", "the original program", "original program", "the original"]
_ALWAYS_RE = re.compile(r"\b(?:" + "|".join(_ALWAYS) + r")\b", re.I)
_CASED_RE = re.compile(r"\b(Excel|Publisher|Office)\b")
_WORD_RE = re.compile(r"\bWord\b")
_ACCESS_RE = re.compile(r"\bAccess\b")
_WORD_OK_NEXT = {"wrap", "count", "spacing", "art", "list", "lists", "break", "breaks"}
_ACCESS_OK_NEXT = {"to", "the", "your", "a", "an", "any", "all", "every", "each", "and", "from", "with", "keys", "shortcuts"}
SUPERLATIVES = re.compile(r"\b(ultimate|blazing|amazing|seamless|revolutionary|world-class|unmatched|unrivaled|stunning|powerful|"
                          r"inigualable|impresionante|increíble|sin igual|incroyable|révolutionnaire|inégalé)\b", re.I)


def _sentence_start(text, pos):
    before = text[:pos].rstrip()
    return before == "" or before[-1] in ".!?:;\"“«¿¡"


def _next_word(text, end):
    m = re.match(r"\s+([A-Za-zÀ-ÿ]+)", text[end:])
    return m.group(1).lower() if m else ""


def violations(text):
    found = [m.group(0) for m in _ALWAYS_RE.finditer(text)]
    found += [m.group(0) for m in _CASED_RE.finditer(text)]
    for rx, ok in ((_WORD_RE, _WORD_OK_NEXT), (_ACCESS_RE, _ACCESS_OK_NEXT)):
        for m in rx.finditer(text):
            if not (_sentence_start(text, m.start()) and _next_word(text, m.end()) in ok):
                found.append(m.group(0))
    found += [m.group(0) for m in SUPERLATIVES.finditer(text)]
    return found


def _text_problems(d, label):
    if d.get("drop"):
        return []
    out = []
    for lang in LANGS:
        t = (d.get(lang) or "").strip()
        if not t:
            out.append("%s %sis empty" % (lang, label))
        for v in violations(t):
            out.append("%s %suses \"%s\"" % (lang, label, v))
        if "*(" in t or "**" in t or "\n" in t:
            out.append("%s %shas stray markup" % (lang, label))
    return out


def entry_problems(e):
    """e: {drop, en, es, fr, soon: {drop, en, es, fr}}: the text of what works today and of what is missing."""
    return _text_problems(e, "") + _text_problems(e.get("soon") or {"drop": True}, "soon text ")


PROMISES = re.compile(r"\b(coming soon|coming next|is coming|are coming|planned|will|in progress)\b", re.I)
OTHER_APPS = ("JeffPub", "JeffDeck", "JeffWrite", "JeffCrunch", "JeffMail", "JeffBase")


def scope_problems(e, wanted_entry):
    """What a reworded text may not say about its own entry: another app's name (unless the source has it) or a promise."""
    own = APPS[wanted_entry["app"]]["name"]
    source = wanted_entry["text"] + " " + wanted_entry.get("note", "")
    out = []
    for label, d in (("", e), ("soon text ", e.get("soon") or {"drop": True})):
        if d.get("drop"):
            continue
        for lang in LANGS:
            t = d.get(lang) or ""
            for name in OTHER_APPS:
                if name != own and name in t and name not in source:
                    out.append("%s %snames %s in %s's text" % (lang, label, name, own))
        m = PROMISES.search(d.get("en") or "")
        if m:
            out.append("en %spromises something (%s)" % (label, m.group(0)))
    return out


def load_overrides(path=None):
    """docs/features/public-overrides.json: [{app, source_startswith, works, soon, exact?}], English texts (or "drop") that replace
    the model's wording of a line whose source cannot say it right (the model only translates them)."""
    try:
        with open(path or OVERRIDES, encoding="utf-8") as f:
            return json.load(f)
    except FileNotFoundError:
        return []


def find_override(overrides, app, text):
    for o in overrides:
        if o["app"] == app and (text == o["source_startswith"] if o.get("exact") else text.startswith(o["source_startswith"])):
            return {k: o[k] for k in ("works", "soon") if k in o}
    return None


# ---------- the API ----------

SYSTEM = """You write the public feature pages of JeffOffice, a free, open-source office suite for Windows, macOS, and Linux with four apps: JeffPub (desktop publishing), JeffDeck (presentations), JeffWrite (documents), and JeffCrunch (spreadsheets). JeffMail and JeffBase are planned.

You receive entries from an internal checklist, as JSON: id, kind ("heading", "feature", "partial", or "todo"), app (the app the entry is about), path (the app, tab, and group the entry sits in), text, and for items a note. For each entry return two public texts, each in English, Spanish, and French: "works" (what the app does today) and "soon" (what is not there yet).

The note is internal: it says how far the feature is, what works, and what is missing. Per kind:
- "feature" (done): works = one or two plain sentences saying what the person can do in that app, in factual words a visitor and a search engine can both use; use the note only to avoid saying more than is true. soon = dropped.
- "partial" (mostly works today): works = only what works today, leaving out what the note says is missing (if the note says nothing usable works yet, drop works). soon = the part the note names as missing, as one short noun-phrase line naming the capability in the present tense ("Conditional format rules: create and edit"); drop soon if the note names nothing concrete.
- "todo" (not there yet): works = dropped. soon = one short plain line naming the feature the same way, from the text.
- "heading": works = the heading; soon = dropped.
To drop a text, set its drop flag (works_drop or soon_drop) true and its three strings empty. Rules:
- Wording of "soon" lines: plain present-tense names of the capability, no dates, no "will", no "coming soon", "planned", or "in progress" (the page's heading says it).
- A note that says "shared: <another app>'s X" means X is built in that other app and is NOT in this app, unless the note itself says this app has it. Do not claim X for this app; for a "partial" or "todo" entry it belongs to "soon".
- An entry may carry "fixed": English text for "works" and/or "soon" that is already correct and final. Return it unchanged in the English field (works: en; soon: soon_en), with its drop flag false (or true when the fixed value is "drop"), and translate it into Spanish and French. For a part that is not in "fixed", follow the rules as usual.
- Never write "the original", "the original program", or "original" to mean another product. Do not write SmartArt (say "diagrams"), WordArt or Text Art (say "text styles"), Tell Me (say "command search"), AutoFilter (say "filter"), or Flash Fill. Never mention command-line or developer tools (names ending in "tool"), fuzzing, "repair message", test collections, or counts of files tested; drop those entries.
- The app is the one named in the entry's "app" field; whenever the sentence needs the app's name, use that name, and never another app's name. Do not mention other JeffOffice apps at all unless the source text itself does.
- Never add a fact that is not in the source text or note. In "works" texts never say anything is "coming", "coming soon", "planned", "in progress", or "will" happen; say only what works now.
- American English, with the Oxford comma ("A, B, and C"). No marketing superlatives or hype: no "best", "powerful", "seamless", "ultimate", and the like. State facts.
- Keep file extensions exactly (.pub, .docx, .xlsx, .pptx, .odt, .ods, .odp, .csv, and so on), each in backticks. Also put keyboard keys and function names in backticks.
- Never name another company's product, program, font, or feature. Do not write Microsoft, Excel, Word (as a product), PowerPoint, Publisher, Office (except inside "JeffOffice"), Outlook, Access, Calibri, Cambria, Arial, Times New Roman, Courier New, Segoe, PivotTable, AutoSum, AutoFit, Format Painter, AutoRecover, Quick Access Toolbar, or Quick Parts, in any language. Say "other office suites" or "other programs" where a product would be named (never "the original program"); for fonts say "common fonts" or "stand-ins with the same letter widths"; for a product's own feature name use a plain description ("pivot table" is fine as a generic term). File formats are named by extension and, where useful, by plain description (for example "binary spreadsheet files from older programs").
- Remove everything internal: tiers, status, version and release history, dates, percentages, counts of files or documents tested, the names of test collections or corpora, benchmark results, code paths and module names (libs/..., tools/...), and how the work was measured or tested. Keep a number only when it is a real limit or capability a user would care about (for example "up to 1,048,576 rows").
- Use the drop flags for a text that is only about testing method, our own collection or corpus, internal code structure or build tooling, or that cannot be said without naming another product or only makes sense as a comparison with one. Do not drop a real user-facing feature just because it is hard to word.
- Do not invent features: say only what the source says. If a source line is vague, stay vague.

For a "heading" entry (works), write a short plain title (under 9 words) in the three languages. Keep the structure and meaning. If a heading names another product, replace the name with neutral wording: "Beyond Excel" becomes "Beyond the usual spreadsheet", "Beyond Publisher" becomes "Beyond the usual desktop publisher", "Beyond Word" becomes "Beyond the usual word processor", "Beyond PowerPoint" becomes "Beyond the usual presentation program"; "Web tab (Publisher's web page tools)" becomes "Web tab (web page tools)"; "Under the ribbon: X" becomes "Behind the scenes: X". Tab names such as "Home tab" and "Insert tab" stay, using the natural Spanish and French words for "tab" ("pestaña", "onglet"). Never drop a heading.

Languages: Spanish addresses the reader as "usted" and avoids regional slang ("computadora", "archivo", "hoja de cálculo", "cinta de opciones" for ribbon). French addresses the reader as "vous" ("ruban", "tableur", "classeur", "feuille de calcul"). Keep the names JeffOffice, JeffPub, JeffDeck, JeffWrite, JeffCrunch, JeffMail, and JeffBase untranslated. Spanish and French translate your English wording, not the source. Use the typographic apostrophe (’) in French.

Output only JSON matching the schema: one result per entry, with the same id."""

RESULT_SCHEMA = {
    "type": "object",
    "properties": {"results": {"type": "array", "items": {
        "type": "object",
        "properties": {"id": {"type": "integer"}, "works_drop": {"type": "boolean"},
                       "en": {"type": "string"}, "es": {"type": "string"}, "fr": {"type": "string"},
                       "soon_drop": {"type": "boolean"},
                       "soon_en": {"type": "string"}, "soon_es": {"type": "string"}, "soon_fr": {"type": "string"}},
        "required": ["id", "works_drop", "en", "es", "fr", "soon_drop", "soon_en", "soon_es", "soon_fr"], "additionalProperties": False}}},
    "required": ["results"], "additionalProperties": False,
}


class Spend:
    def __init__(self):
        self.lock = threading.Lock()
        self.requests = 0
        self.tokens_in = 0
        self.tokens_out = 0
        self.models = set()

    def add(self, r):
        u = r.usage
        with self.lock:
            self.requests += 1
            self.tokens_in += (u.input_tokens or 0) + (getattr(u, "cache_creation_input_tokens", 0) or 0) + (getattr(u, "cache_read_input_tokens", 0) or 0)
            self.tokens_out += u.output_tokens or 0
            self.models.add(r.model)

    @property
    def dollars(self):
        return (self.tokens_in * PRICE_IN + self.tokens_out * PRICE_OUT) / 1e6


SPEND = Spend()
_client = None
_client_lock = threading.Lock()


def ask(entries, feedback=None):
    """entries: [{id, kind, path, text}]; returns the parsed results list."""
    global _client
    import anthropic
    with _client_lock:
        if _client is None:
            _client = anthropic.Anthropic(max_retries=6)
    user = "Entries:\n" + json.dumps(entries, ensure_ascii=False, indent=1)
    if feedback:
        user += "\n\nYour earlier answer for some entries used words that are not allowed. Write these entries again without them (problems by id):\n" + \
                json.dumps(feedback, ensure_ascii=False)
    try:
        with _client.beta.messages.stream(
            model=MODEL, max_tokens=24000,
            system=SYSTEM, messages=[{"role": "user", "content": user}],
            output_config={"effort": "low", "format": {"type": "json_schema", "schema": RESULT_SCHEMA}},
            betas=["server-side-fallback-2026-07-01"], fallbacks="default",
        ) as stream:
            r = stream.get_final_message()
    except (anthropic.AuthenticationError, anthropic.PermissionDeniedError) as e:
        print("The service refused the key (ANTHROPIC_API_KEY): %s" % e, file=sys.stderr)
        os._exit(2)
    SPEND.add(r)
    if r.stop_reason in ("refusal", "max_tokens"):
        raise RuntimeError("response unusable: " + r.stop_reason)
    return json.loads("".join(b.text for b in r.content if b.type == "text"))["results"]


def key_of(app, kind, text, note=""):
    """The key of a reworded text: per app (the same line in two lists is two texts), and per note (the note changes what is true)."""
    return sha(app + "\n" + ("heading\n" if kind == "heading" else "") + text + (("\n" + note) if note else ""))


def item_key(app, text, note, ov):
    """An item's key also holds its override, so changing an override sends the item again."""
    return key_of(app, "item", text, note + (("\n" + json.dumps(ov, sort_keys=True, ensure_ascii=False)) if ov else ""))


def norm_line(t):
    return re.sub(r"[^a-z0-9]+", " ", t.lower()).strip()


def collect(apps_tabs, overrides=()):
    """Every distinct thing to reword: {key: {app, kind, path, text, note}} in page order."""
    wanted = {}

    def item(app, status, text, note, path):
        ov = find_override(overrides, app, text)
        k = item_key(app, text, note, ov)
        wanted.setdefault(k, {"app": app, "kind": {"done": "feature", "partly": "partial", "todo": "todo"}[status], "path": path, "text": text, "note": note, "override": ov})

    for app, tabs in apps_tabs.items():
        base = APPS[app]["name"]
        for tab in tabs:
            wanted.setdefault(key_of(app, "heading", tab["h"]), {"app": app, "kind": "heading", "path": base, "text": tab["h"], "note": ""})
            for status, text, note in tab["direct"]:
                item(app, status, text, note, base + " > " + tab["h"])
            for g in tab["groups"]:
                if not g["items"]:
                    continue
                wanted.setdefault(key_of(app, "heading", g["h"]), {"app": app, "kind": "heading", "path": base + " > " + tab["h"], "text": g["h"], "note": ""})
                for status, text, note in g["items"]:
                    item(app, status, text, note, base + " > " + tab["h"] + " > " + g["h"])
    return wanted


def estimate(todo):
    chars = sum(len(v["text"]) + len(v["path"]) + len(v["note"]) for v in todo.values())
    batches = -(-len(todo) // BATCH)
    # (scaled by what the first real run measured: input 1.26x and output 1.75x the plain estimate)
    tokens_in = 1.26 * (chars / 3.5 + 45 * len(todo) + batches * (len(SYSTEM) / 3.5 + 300))
    two = lambda v: 1.5 if v["kind"] in ("partial", "todo") else 1.0     # a second text for what is missing
    tokens_out = 1.75 * (sum(max(len(v["text"]) * 0.75, 40) * two(v) for v in todo.values()) * 3 / 3.2 + 30 * len(todo))
    return tokens_in, tokens_out, (tokens_in * PRICE_IN + tokens_out * PRICE_OUT) / 1e6


def normalize(r, kind, override=None):
    """The model's flat result as stored: works text plus a "soon" text; a kind that has only one of them drops the other."""
    works = {"drop": bool(r["works_drop"]) or kind == "todo", "en": r["en"].strip(), "es": r["es"].strip(), "fr": r["fr"].strip()}
    soon = {"drop": bool(r["soon_drop"]) or kind in ("feature", "heading"), "en": r["soon_en"].strip(), "es": r["soon_es"].strip(), "fr": r["soon_fr"].strip()}
    for d in (works, soon):
        if d["drop"]:
            d.update(en="", es="", fr="")
    for name, d in (("works", works), ("soon", soon)):
        v = (override or {}).get(name)
        if v == "drop":
            d.update(drop=True, en="", es="", fr="")
        elif v:
            d["drop"] = False
            d["en"] = v.strip()          # the English is the override's own words; the model only translated it
    works["soon"] = soon
    return works


def reword(wanted, cache, dry_run, checkpoint):
    todo = {k: v for k, v in wanted.items() if k not in cache or "soon" not in cache[k] or entry_problems(cache[k]) or scope_problems(cache[k], v)}
    print("%d distinct items and headings; %d cached; %d to send." % (len(wanted), len(wanted) - len(todo), len(todo)))
    if not todo:
        return
    ti, to, cost = estimate(todo)
    print("Estimated: ~%d input tokens, ~%d output tokens, about $%.2f (assumed $%g/M in, $%g/M out; %s)." % (ti, to, cost, PRICE_IN, PRICE_OUT, MODEL))
    if cost > MAX_ESTIMATE:
        sys.exit("Estimate is over $%g: stopping." % MAX_ESTIMATE)
    if dry_run:
        return
    if not os.environ.get("ANTHROPIC_API_KEY"):
        sys.exit("ANTHROPIC_API_KEY is not set")
    keys = list(todo)
    batches = [keys[i:i + BATCH] for i in range(0, len(keys), BATCH)]
    lock = threading.Lock()
    failed = {}

    def run(batch_keys):
        entries = [dict({"id": i, "kind": todo[k]["kind"], "app": APPS[todo[k]["app"]]["name"], "path": todo[k]["path"], "text": todo[k]["text"]},
                        **({"note": todo[k]["note"]} if todo[k]["kind"] != "heading" else {}),
                        **({"fixed": todo[k]["override"]} if todo[k].get("override") else {})) for i, k in enumerate(batch_keys)]
        by_id = dict(enumerate(batch_keys))
        accepted, bad = {}, {}
        for attempt in (1, 2):
            if attempt == 1:
                results = ask(entries)
            else:
                results = ask([e for e in entries if e["id"] in bad], {str(i): p for i, p in bad.items()})
            bad = {}
            seen = set()
            for r in results:
                i = r.get("id")
                if i not in by_id or i in seen or i in accepted:
                    continue
                seen.add(i)
                r = normalize(r, todo[by_id[i]]["kind"], todo[by_id[i]].get("override"))
                problems = entry_problems(r) + scope_problems(r, todo[by_id[i]])
                if problems:
                    bad[i] = problems
                else:
                    accepted[i] = r
            for e in entries:
                if e["id"] not in accepted and e["id"] not in seen:
                    bad[e["id"]] = ["no result returned"]
            if not bad:
                break
        with lock:
            for i, e in accepted.items():
                cache[by_id[i]] = e
            for i, p in bad.items():
                failed[by_id[i]] = p
            checkpoint()
            print("batch done: %d ok, %d failed (running total $%.2f)" % (len(accepted), len(bad), SPEND.dollars), flush=True)

    with cf.ThreadPoolExecutor(WORKERS) as pool:
        for fut in [pool.submit(run, b) for b in batches]:
            fut.result()
    print("Actual: %d requests, %d input tokens, %d output tokens, $%.2f (at $%g/M in, $%g/M out; models seen: %s)." % (
        SPEND.requests, SPEND.tokens_in, SPEND.tokens_out, SPEND.dollars, PRICE_IN, PRICE_OUT, ", ".join(sorted(SPEND.models))))
    if failed:
        print("\nThese still break the rules after a second try; the build stops:", file=sys.stderr)
        for k, p in failed.items():
            print("  [%s] %s\n    %s" % (todo[k]["kind"], todo[k]["text"][:200], "; ".join(p)), file=sys.stderr)
        sys.exit(1)



# ---------- public.json ----------

def load_texts():
    try:
        with open(OUT, encoding="utf-8") as f:
            return json.load(f).get("texts", {})
    except FileNotFoundError:
        return {}


def build_public(apps_tabs, states, rev, texts, overrides=()):
    """The file's content: the structure of each list (keys into texts) and the texts still in use. Within an app, a Coming soon
    line whose English repeats an earlier one (same words, ignoring case and punctuation) is marked hide_soon."""
    used = set()

    def ref(app, kind, text, note=""):
        k = key_of(app, kind, text, note)
        used.add(k)
        return k

    def items(app, lst):
        out = []
        for s, x, n in lst:
            k = item_key(app, x, n, find_override(overrides, app, x))
            used.add(k)
            out.append({"status": s, "text": k})
        return out

    apps = {}
    for app, tabs in apps_tabs.items():
        out = []
        seen = set()
        for tab in tabs:
            t = {"heading": ref(app, "heading", tab["h"]), "items": items(app, tab["direct"]),
                 "groups": [{"heading": ref(app, "heading", g["h"]), "items": items(app, g["items"])} for g in tab["groups"] if g["items"]]}
            for it in t["items"] + [i for g in t["groups"] for i in g["items"]]:
                if it["status"] == "done":
                    it["hide_soon"] = True       # a done line has nothing missing, whatever an older text says
                    continue
                soon = (texts.get(it["text"]) or {}).get("soon")
                if soon and not soon["drop"]:
                    n = norm_line(soon["en"])
                    if n in seen:
                        it["hide_soon"] = True
                    seen.add(n)
            out.append(t)
        r = rev.get(app)
        apps[app] = {"name": APPS[app]["name"], "state": states[app][0], "version": states[app][1],
                     "reviewed": {"month": r[0], "day": r[1], "year": r[2]} if r else None, "tabs": out}
    return {"version": 1, "model": MODEL, "apps": apps, "texts": {k: texts[k] for k in sorted(used) if k in texts}}


def write_public(data):
    tmp = OUT + ".tmp"
    with open(tmp, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=1, sort_keys=True)
        f.write("\n")
    os.replace(tmp, OUT)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dry-run", action="store_true", help="count and estimate; call nothing, write nothing")
    ap.add_argument("--features", default=FEATURES_DIR, help="the docs/features directory")
    args = ap.parse_args()

    states = readme_state(args.features)
    apps_tabs, rev = {}, {}
    for app in APPS:
        path = os.path.join(args.features, app + ".md")
        apps_tabs[app] = parse_list(path)
        rev[app] = reviewed(path)
        if app not in states:
            sys.exit("no state for %s in README.md" % app)
        kept = sum(len(t["direct"]) + sum(len(g["items"]) for g in t["groups"]) for t in apps_tabs[app])
        print("%-10s %d tabs, %d items kept from the list" % (app, len(apps_tabs[app]), kept))
    texts = load_texts()
    overrides = load_overrides()
    wanted = collect(apps_tabs, overrides)
    for o in overrides:
        if not any(find_override([o], app, x) is not None for app, tabs in apps_tabs.items() for t in tabs for _, x, _ in t["direct"] + [i for g in t["groups"] for i in g["items"]]):
            print("warning: an override matches nothing: %s %r" % (o["app"], o["source_startswith"][:50]))
    state = (apps_tabs, states, rev)
    reword(wanted, texts, args.dry_run, lambda: write_public(build_public(*state, texts, overrides)))
    if args.dry_run:
        return
    bad = [(k, entry_problems(texts[k]) + scope_problems(texts[k], wanted[k])) for k in wanted if entry_problems(texts[k]) + scope_problems(texts[k], wanted[k])]
    if bad:
        for k, p in bad:
            print("  %s: %s" % (wanted[k]["text"][:120], "; ".join(p)), file=sys.stderr)
        sys.exit("stored text breaks the rules (delete those keys from public.json to redo them)")
    data = build_public(*state, texts, overrides)
    write_public(data)
    print("wrote %s: %d texts" % (os.path.relpath(OUT, ROOT), len(data["texts"])))


if __name__ == "__main__":
    main()
