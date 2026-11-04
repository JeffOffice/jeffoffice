#!/usr/bin/env python3
"""Compares a cheaper model's translations with the ones already made.

Picks program strings and Help topics whose current translation is the
machine's own (the record matches it, and the English has not changed
since), translates them again with each model named, through the same
prompts, checks and review as translate.py, and writes a side-by-side
report. Nothing in translations/ or resources/help/ is changed.

    python3 tools/translate/trial.py --lang fr --models claude-haiku-5-5,claude-sonnet-5-5 --out trial
"""

import argparse
import concurrent.futures as cf
import glob
import json
import os
import random
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import translate as T  # noqa: E402


def sample_ui(lang, n, rng):
    _, messages = T.load_ts(lang)
    record = T.load_state(lang).get("ui", {})
    mine = [m for m in messages if not m.empty() and record.get(m.key) == T.sha(json.dumps(m.current, ensure_ascii=False))]
    return rng.sample(mine, min(n, len(mine)))


def sample_help(lang, per_app, rng):
    record = T.load_state(lang).get("help", {})
    picked = []
    for folder, app, kind in T.HELP_APPS:
        if folder not in ("write", "crunch"):
            continue
        ok = []
        for path in sorted(glob.glob(os.path.join(T.HELP_DIR, "en", folder, "*.md"))):
            tid = os.path.basename(path)[:-3]
            rec = record.get(T.help_key(folder, tid))
            target = os.path.join(T.HELP_DIR, lang, folder, tid + ".md")
            if not rec or not os.path.exists(target):
                continue
            with open(path, encoding="utf-8") as f:
                en = f.read()
            with open(target, encoding="utf-8") as f:
                old = f.read()
            # Mid-sized topics: long enough to judge, short enough to read.
            if rec["en"] == T.sha(en) and rec["out"] == T.sha(old) and 300 <= len(en.split()) <= 1500:
                ok.append((folder, app, kind, tid, en, old))
        picked += rng.sample(ok, min(per_app, len(ok)))
    return picked


def run_model(model, lang, ui, topics, out_dir):
    T.MODEL = model
    T.SPEND = T.Spend()
    api = T.Api("low")
    report = T.Report(lang)
    terms = T.glossary(lang, api, True)   # (the terms already chosen; none added)
    system, rsystem = T.ui_system(lang, terms), T.review_system(lang, terms)
    results = {}
    with cf.ThreadPoolExecutor(T.WORKERS) as pool:
        for done in pool.map(lambda c: T.translate_chunk(api, system, c, report), T.chunks(ui)):
            results.update(done)
        pairs = [(m, results[m]) for m in ui if m in results]
        batches = [pairs[i:i + 70] for i in range(0, len(pairs), 70)]
        for fixes in pool.map(lambda b: T.review_chunk(api, rsystem, b, report), batches):
            results.update(fixes)
    table = T.ui_table(lang)
    help_out = {}
    for folder, app, kind, tid, en, old in topics:
        hs, hr = T.help_system(lang, terms, table, app, kind), T.help_review_system(lang, terms, table, app, kind)
        out = T.translate_topic(api, hs, hr, T.help_key(folder, tid), en, report)
        help_out[(folder, tid)] = out
        if out is not None:
            d = os.path.join(out_dir, model, folder)
            os.makedirs(d, exist_ok=True)
            with open(os.path.join(d, tid + ".md"), "w", encoding="utf-8") as f:
                f.write(out)
    return results, help_out, report, T.SPEND.line()


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--lang", default="fr")
    ap.add_argument("--models", default="claude-haiku-5-5,claude-sonnet-5-5")
    ap.add_argument("--strings", type=int, default=200)
    ap.add_argument("--topics-per-app", type=int, default=2)
    ap.add_argument("--seed", type=int, default=21)
    ap.add_argument("--out", default="trial")
    args = ap.parse_args()
    if not os.environ.get("ANTHROPIC_API_KEY"):
        sys.exit("ANTHROPIC_API_KEY is not set")
    rng = random.Random(args.seed)
    ui = sample_ui(args.lang, args.strings, rng)
    topics = sample_help(args.lang, args.topics_per_app, rng)
    os.makedirs(args.out, exist_ok=True)
    for folder, app, kind, tid, en, old in topics:
        d = os.path.join(args.out, "current", folder)
        os.makedirs(d, exist_ok=True)
        with open(os.path.join(d, tid + ".md"), "w", encoding="utf-8") as f:
            f.write(old)
    runs = {}
    for model in [m for m in args.models.split(",") if m]:
        runs[model] = run_model(model, args.lang, ui, topics, args.out)
        print(f"{model}: {runs[model][3]}", flush=True)

    cell = lambda s: (" / ".join(s) if isinstance(s, list) else (s or "")).replace("|", "\\|").replace("\n", " ")
    models = list(runs)
    md = [f"# Translation trial: {T.LANGUAGES[args.lang]}", "",
          f"{len(ui)} program strings and {len(topics)} Help topics whose current translation is the machine's, translated again.", ""]
    for model in models:
        results, help_out, report, spend = runs[model]
        same = sum(1 for m in ui if m in results and results[m] == m.current)
        md += [f"## {model}", "", f"- {spend}",
               f"- Strings translated: {len(results)} of {len(ui)}; identical to the current translation: {same}",
               f"- Strings the review flagged: {len(report.flags)}; Help passages still flagged: {len(report.help_flags)}; problems: {len(report.problems)}", ""]
        md += [f"  - {p}" for p in report.problems[:20]]
    md += ["", "## Strings that differ", "", "| English | Current | " + " | ".join(models) + " |", "| --- | --- | " + " | ".join("---" for _ in models) + " |"]
    for m in sorted(ui, key=lambda m: m.context):
        outs = [runs[x][0].get(m) for x in models]
        if all(o == m.current for o in outs):
            continue
        md.append(f"| {cell(m.source)} | {cell(m.current)} | " + " | ".join(cell(o) if o is not None else "(none)" for o in outs) + " |")
    md += ["", "## Help topics", "", "Side by side in current/ and in each model's folder."]
    for folder, app, kind, tid, en, old in topics:
        md.append(f"- {folder}/{tid} ({len(en.split())} words)")
    with open(os.path.join(args.out, "report.md"), "w", encoding="utf-8") as f:
        f.write("\n".join(md) + "\n")
    for model in models:
        with open(os.path.join(args.out, f"review-{model}.md"), "w", encoding="utf-8") as f:
            f.write(runs[model][2].markdown())


if __name__ == "__main__":
    try:
        sys.exit(main())
    except T.KeyRefused as e:
        print(f"The translation service refused the key: {e}", file=sys.stderr)
        sys.exit(2)
