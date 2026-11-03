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
TIER = re.compile(r"^(?:\*\*(?:first release|1\.0|later)\*\*\s*)+")
ALT = re.compile(r"\*\*\(alt\)\*\*\s*")
ALT_NOTE = re.compile(r"\*\*\((?:alt:\s*)?(.*?)\)\*\*")
NEEDED = re.compile(r"\s*\*\*\(needed[^)]*\)\*\*", re.I)
NOTES = re.compile(r"\s*(?<!\*)\*\(.*?\)\*(?!\*)")


def sha(s):
    return hashlib.sha256(s.encode("utf-8")).hexdigest()


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
    """Returns tabs: [ {h: heading, direct: [(status, text)], groups: [ {h, items} ]} ]."""
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
            if m.group(1) in "x~":
                entry = ("done" if m.group(1) == "x" else "next", clean_item(m.group(2)))
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
           "Outlook", "WordArt"]
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


def entry_problems(e):
    if e.get("drop"):
        return []
    out = []
    for lang in LANGS:
        t = (e.get(lang) or "").strip()
        if not t:
            out.append("%s is empty" % lang)
        for v in violations(t):
            out.append("%s uses \"%s\"" % (lang, v))
        if "*(" in t or "**" in t or "\n" in t:
            out.append("%s has stray markup" % lang)
    return out


# ---------- the API ----------

SYSTEM = """You write the public feature pages of JeffOffice, a free, open-source office suite for Windows, macOS, and Linux with four apps: JeffPub (desktop publishing), JeffDeck (presentations), JeffWrite (documents), and JeffCrunch (spreadsheets). JeffMail and JeffBase are planned.

You receive entries from an internal checklist, as JSON: id, kind ("heading", "feature", or "next"), path (the app, tab, and group the entry sits in), and text. Return, for each entry, a public version in English, Spanish, and French.

For a "feature" or "next" entry, write one or two plain sentences that say what the person can do, in factual words a visitor and a search engine can both use. A "next" entry is a feature that is only partly there; describe the capability in the same way (the page shows it under "Coming next", so do not add "coming" or promise dates). Rules:
- American English, with the Oxford comma ("A, B, and C"). No marketing superlatives or hype: no "best", "powerful", "seamless", "ultimate", and the like. State facts.
- Keep file extensions exactly (.pub, .docx, .xlsx, .pptx, .odt, .ods, .odp, .csv, and so on), each in backticks. Also put keyboard keys and function names in backticks.
- Never name another company's product, program, font, or feature. Do not write Microsoft, Excel, Word (as a product), PowerPoint, Publisher, Office (except inside "JeffOffice"), Outlook, Access, Calibri, Cambria, Arial, Times New Roman, Courier New, Segoe, PivotTable, AutoSum, AutoFit, Format Painter, AutoRecover, Quick Access Toolbar, or Quick Parts, in any language. Say "the original program", "other office suites", or "other programs" where a product would be named; for fonts say "common fonts" or "stand-ins with the same letter widths"; for a product's own feature name use a plain description ("pivot table" is fine as a generic term). File formats are named by extension and, where useful, by plain description (for example "binary spreadsheet files from older programs").
- Remove everything internal: tiers, status, version and release history, dates, percentages, counts of files or documents tested, the names of test collections or corpora, benchmark results, code paths and module names (libs/..., tools/...), and how the work was measured or tested. Keep a number only when it is a real limit or capability a user would care about (for example "up to 1,048,576 rows").
- Use "drop": true (and empty strings) for an entry that is only about testing method, our own collection or corpus, internal code structure or build tooling, or that cannot be said without naming another product or only makes sense as a comparison with one. Do not drop a real user-facing feature just because it is hard to word.
- Do not invent features: say only what the source says. If a source line is vague, stay vague.

For a "heading" entry, write a short plain title (under 9 words) in the three languages. Keep the structure and meaning. If a heading names another product, replace the name with neutral wording: "Beyond Excel" becomes "Beyond the usual spreadsheet", "Beyond Publisher" becomes "Beyond the usual desktop publisher", "Beyond Word" becomes "Beyond the usual word processor", "Beyond PowerPoint" becomes "Beyond the usual presentation program"; "Web tab (Publisher's web page tools)" becomes "Web tab (web page tools)"; "Under the ribbon: X" becomes "Behind the scenes: X". Tab names such as "Home tab" and "Insert tab" stay, using the natural Spanish and French words for "tab" ("pestaña", "onglet"). Never drop a heading.

Languages: Spanish addresses the reader as "usted" and avoids regional slang ("computadora", "archivo", "hoja de cálculo", "cinta de opciones" for ribbon). French addresses the reader as "vous" ("ruban", "tableur", "classeur", "feuille de calcul"). Keep the names JeffOffice, JeffPub, JeffDeck, JeffWrite, JeffCrunch, JeffMail, and JeffBase untranslated. Spanish and French translate your English wording, not the source. Use the typographic apostrophe (’) in French.

Output only JSON matching the schema: one result per entry, with the same id."""

RESULT_SCHEMA = {
    "type": "object",
    "properties": {"results": {"type": "array", "items": {
        "type": "object",
        "properties": {"id": {"type": "integer"}, "drop": {"type": "boolean"},
                       "en": {"type": "string"}, "es": {"type": "string"}, "fr": {"type": "string"}},
        "required": ["id", "drop", "en", "es", "fr"], "additionalProperties": False}}},
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


def key_of(kind, text):
    return sha(("heading\n" if kind == "heading" else "") + text)


def collect(apps_tabs):
    """Every distinct thing to reword: {key: {kind, path, text}} in page order."""
    wanted = {}
    for app, tabs in apps_tabs.items():
        for tab in tabs:
            base = APPS[app]["name"]
            wanted.setdefault(key_of("heading", tab["h"]), {"kind": "heading", "path": base, "text": tab["h"]})
            for status, text in tab["direct"]:
                wanted.setdefault(key_of("item", text), {"kind": "feature" if status == "done" else "next", "path": base + " > " + tab["h"], "text": text})
            for g in tab["groups"]:
                if not g["items"]:
                    continue
                wanted.setdefault(key_of("heading", g["h"]), {"kind": "heading", "path": base + " > " + tab["h"], "text": g["h"]})
                for status, text in g["items"]:
                    wanted.setdefault(key_of("item", text), {"kind": "feature" if status == "done" else "next", "path": base + " > " + tab["h"] + " > " + g["h"], "text": text})
    return wanted


def estimate(todo):
    chars = sum(len(v["text"]) + len(v["path"]) for v in todo.values())
    batches = -(-len(todo) // BATCH)
    tokens_in = chars / 3.5 + 45 * len(todo) + batches * (len(SYSTEM) / 3.5 + 300)
    tokens_out = sum(max(len(v["text"]) * 0.75, 40) for v in todo.values()) * 3 / 3.2 + 30 * len(todo)
    return tokens_in, tokens_out, (tokens_in * PRICE_IN + tokens_out * PRICE_OUT) / 1e6


def reword(wanted, cache, dry_run, checkpoint):
    todo = {k: v for k, v in wanted.items() if k not in cache}
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
        entries = [{"id": i, "kind": todo[k]["kind"], "path": todo[k]["path"], "text": todo[k]["text"]} for i, k in enumerate(batch_keys)]
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
                problems = entry_problems(r)
                if problems:
                    bad[i] = problems
                else:
                    accepted[i] = {"drop": bool(r["drop"]), "en": r["en"].strip(), "es": r["es"].strip(), "fr": r["fr"].strip()}
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


def build_public(apps_tabs, states, rev, texts):
    """The file's content: the structure of each list (keys into texts) and the texts still in use."""
    used = set()

    def ref(kind, text):
        k = key_of(kind, text)
        used.add(k)
        return k

    apps = {}
    for app, tabs in apps_tabs.items():
        out = []
        for tab in tabs:
            out.append({
                "heading": ref("heading", tab["h"]),
                "items": [{"status": s, "text": ref("item", x)} for s, x in tab["direct"]],
                "groups": [{"heading": ref("heading", g["h"]), "items": [{"status": s, "text": ref("item", x)} for s, x in g["items"]]}
                           for g in tab["groups"] if g["items"]],
            })
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
    wanted = collect(apps_tabs)
    state = (apps_tabs, states, rev)
    reword(wanted, texts, args.dry_run, lambda: write_public(build_public(*state, texts)))
    if args.dry_run:
        return
    bad = [(k, entry_problems(texts[k])) for k in wanted if entry_problems(texts[k])]
    if bad:
        for k, p in bad:
            print("  %s: %s" % (wanted[k]["text"][:120], "; ".join(p)), file=sys.stderr)
        sys.exit("stored text breaks the rules (delete those keys from public.json to redo them)")
    data = build_public(*state, texts)
    write_public(data)
    print("wrote %s: %d texts" % (os.path.relpath(OUT, ROOT), len(data["texts"])))


if __name__ == "__main__":
    main()
