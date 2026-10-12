# Translations

JeffOffice's own text (JeffPub's, JeffDeck's, and the parts they share) is
written in American English; the apps share one set of translation files. It is translated
automatically into German, Spanish, French, Italian, Dutch, and Portuguese
(Brazil and Portugal) by `tools/translate/translate.py`, which sends only
new or changed text to Claude through the Anthropic API:

- Each night the Translate workflow (`.github/workflows/translate.yml`)
  lists the program's text again, translates what's new in the program's
  strings and in Help, and commits it as JeffOffice LLC. Before a release,
  run it by hand (`gh workflow run translate.yml`) or run the script
  locally with `ANTHROPIC_API_KEY` set.
- Each translation passes automatic checks (placeholders, plural forms,
  tags, line breaks, & markers, Help's links and headings), then a second
  pass translates it back into English and corrects any whose meaning
  drifted; the run's report lists every correction.
- `glossary/<language>.json` keeps terms the same everywhere. Correct a term
  there and it's never asked for again (rerun to apply it to new text).
- `machine/<language>.json` records what the machine wrote. A translation
  someone changed by hand differs from the record and is never overwritten;
  a Help topic whose English changed under a hand-edited translation is
  listed in the report for a person to update.
- Machine translations are marked unfinished, which Qt Linguist shows as
  not yet reviewed; the program uses them all the same.

A translation is a
`jeffoffice_<language>.ts` file here (for example `jeffoffice_de.ts` or
`jeffoffice_pt_BR.ts`), made and edited with Qt Linguist; the build turns each
into a `.qm` file inside the programs, and each app uses the one for the
system's language, or the one chosen in File > Options > General > Display
language.

- `jeffoffice.ts` lists every string marked for translation, with no language.
  Update it after changing the program's text:
  `cmake --build build --target jeffoffice_lupdate`.
- To start a language, copy `jeffoffice.ts` to `jeffoffice_<language>.ts`, set its
  language in Qt Linguist (Edit > Translation File Settings), translate,
  and rebuild.
- To see a translation in an app before it's built in, release it with
  `lrelease jeffoffice_<language>.ts` and start the app with
  `JEFFPUB_TRANSLATION=/path/to/jeffoffice_<language>.qm` (JeffDeck:
  `JEFFDECK_TRANSLATION`).
- English has `jeffoffice_en.ts` for plurals only ("%n page(s)"): lupdate adds
  each new plural there empty; give it its singular and plural forms.
- The ribbons' labels come from `resources/ribbon.json` (JeffPub's) and
  `apps/deck/resources/ribbon.json` (JeffDeck's); `tools/ribbon_strings.py`
  lists them for translators in `apps/pub/app/ribbon_strings.cpp` and
  `apps/deck/app/ribbon_strings.cpp`, which tests keep in step with the files.
- Help's topics are Markdown files in `resources/help/en/`, one per topic.
  To translate them, copy the folder to `resources/help/<language>/` (the
  same name as the `.ts` file's, such as `de` or `pt_BR`) and translate
  each file, keeping its name, its links, and its `<!-- keywords: -->`
  line (translate the keywords too). A topic left untranslated shows in
  English. Then run `python3 tools/help_files.py` to build them in.
