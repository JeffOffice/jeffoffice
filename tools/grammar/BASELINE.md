# Grammar rules: false alarms on real text

Measured November 13, 2026 with `tools/grammar/measure.py` (the numbers come from `grammartool`, which runs every rule over the text). A false alarm is a flag that a careful
proofreader would not want, or whose first suggestion is wrong. "/10k" is flags per 10,000 words; "FA/10k" is false alarms per 10,000 words (flags/10k times the false-alarm share).

## The text

- **docs**: the paragraphs of the 445 practice documents (word/dev.txt, word2/dev2.txt, word3/dev3.txt of the reference sets): 1,322,734 words. The held-out half (word/test.txt) is not read.
- **govdocs**: 818 of the plain-text files in the Govdocs1 zips, 20,037,102 words. They are rough: OCR errors, data tables, e-mail, slide text, and word lists, so they inflate the counts of
  the spacing and repetition rules. Enron, EUSES, POI, and LibreOffice hold spreadsheets and test files with no prose, and are not used.
- The files are read in place and the text is kept in a scratch folder, not in the repo.

## The 14 rules that were there (199 flags read: 100 from docs, 99 from govdocs)

| rule | docs flags | /10k | govdocs flags | /10k | read (docs, govdocs) | false (docs, govdocs) | false share, docs | FA/10k, docs |
|---|---|---|---|---|---|---|---|---|
| repeated-word | 23 | 0.17 | 337 | 0.17 | 14, 12 | 2, 11 | 14% | 0.02 |
| a-an | 32 | 0.24 | 212 | 0.11 | 14, 14 | 2, 11 | 14% | 0.03 |
| sentence-start | 29 | 0.22 | 1,765 | 0.88 | 18, 15 | 17, 9 | 94% | 0.21 |
| its | 3 | 0.02 | 9 | 0.00 | 3, 6 | 0, 0 | 0% | 0.00 |
| their-there | 0 | 0 | 0 | 0 | 0, 0 | - | - | 0 |
| your-youre | 1 | 0.01 | 0 | 0 | 1, 0 | 1, - | 100% | 0.01 |
| then-than | 3 | 0.02 | 8 | 0.00 | 3, 6 | 0, 0 | 0% | 0.00 |
| lose-loose | 0 | 0 | 0 | 0 | 0, 0 | - | - | 0 |
| could-of | 0 | 0 | 0 | 0 | 0, 0 | - | - | 0 |
| double-space | 3,115 | 23.55 | 632,791 | 315.81 | 20, 15 | 10, 15 | 50% | 11.8 |
| space-before-mark | 89 | 0.67 | 11,814 | 5.90 | 18, 14 | 6 (see below), 14 | 33% (0% below) | 0.22 (0 below) |
| missing-space | 9 | 0.07 | 12 | 0.01 | 9, 12 | 0, 7 | 0% | 0.00 |
| alot | 0 | 0 | 5 | 0.00 | 0, 5 | -, 0 | - | 0 |
| everyday | 0 | 0 | 0 | 0 | 0, 0 | - | - | 0 |

What the false alarms were:
- **double-space** (the one that matters most): half of the docs flags are alignment (columns of figures, "(2)  Small business", a label and its value, text pasted from a table), and every
  govdocs flag read was a plain-text table or justified text. 11.8 false alarms per 10,000 words in real documents.
- **sentence-start**: "org. code", "Dougl. ex Torr.", "PhDs. are" (a small letter after an abbreviation); 13 of the 18 docs samples are one document that says "org. code". Counted by
  document, the share is still more than half.
- **space-before-mark**: all 6 false alarms in docs are an equation that the text extraction left empty ("is , where G"), so a real document with the equation as an object would not show them;
  in govdocs the flags are data columns.
- **a-an**: a label ("Cat. A obligations") and "a onetime" (the word begins with a "w" sound); govdocs: OCR and German text.
- **repeated-word**: "When is enough enough?" and a rhetorical "listen, listen listen"; govdocs: word lists.
- **missing-space** in govdocs: the join between bullets in text made from slides.
- **your-youre**: a garbled heading. No false alarms were found in `its`, `then-than`, `alot`.
- Rules with no flags in 21 million words (their-there, lose-loose, could-of, everyday) are fine here; the tests are the only proof that they flag anything.

## After the fixes to double-space and sentence-start (same text, re-measured, flags re-read)

| rule | before: docs /10k, false share, FA/10k | after: docs /10k, false share, FA/10k | govdocs /10k before, after |
|---|---|---|---|
| double-space | 23.55, 50%, 11.8 | 6.68, about 10% (3 of 25 read), about 0.7 | 315.81, 1.15 |
| sentence-start | 0.22, 94%, 0.21 | 0.04, under 15% (6 flags, all read), under 0.01 | 0.88, 0.38 |

- double-space now leaves alone: a paragraph with a tab or a run of three spaces, a paragraph with "=" or a quoted-field table, a justified paragraph (five double spaces and more than one per eight words),
  a double space next to a number, a mark, or a one-letter symbol ("800 -  859", "X  xi", "(2)  Text", "-----  -----"), and the old ones (after a sentence end). What is left in docs is almost all real stray
  double spaces. Known false alarm left: a label and its value ("Originator  Name") in a table cell.
- sentence-start now leaves alone: a word of three letters or fewer before the period ("org. code"), a name with a capital inside ("PhDs."), a word that is not plain Latin letters, a capitalized word that stops
  in a cluster of consonants ("Dougl.", "Camp."), the end of an address ("name. net"), a colon after the word, a paragraph with ". . ." leaders, and a short list of connectors that go with author names ("ex", "auct", "nom", "comb").
  A shortened word of four letters ("conf.") is not covered, because "left. she" would go with it.
- Govdocs after: the double-space flags that remain are plain-text justification of a few spaces in a long line and are not read; sentence-start there is botanical and OCR lists (about half false).
- Every false-alarm sample read became a near-miss test in `tst_shell_grammar_rules`; the true flags still flag.

## The rules added in round 38

Every new error rule has under 1 false alarm per 10,000 words in docs and in govdocs. Flags in docs / govdocs per 10,000 words, and the flags read (all of them when there were 20 or fewer, a
random 10 to 14 of each source otherwise):

| rule | docs /10k | govdocs /10k | read | false alarms found |
|---|---|---|---|---|
| he-dont, they-was | 0, 0 | 0.0001, 0.0001 | 5 | 5: speech written in dialect in a history text (0.0003 per 10,000) |
| she-have, plural-has, plural-does, they-is, there-is-many, indefinite-are | 0 | 0 | 0 | none seen (no flags) |
| whos-whose, much-many, whether-weather, quite-quiet, advice-advise, to-much, too-to, breath-breathe | 0 | 0 | 0 | none seen (no flags) |
| idioms, between-you-and-i, me-and-him, double-comparative, modal-past, double-negative, your-welcome | 0 | 0 | 0 | none seen (no flags) |
| demonstrative-number | 0 | 0 | 0 | none (after the fix below) |
| affect-effect | 0 | 0.0002 | 4 | 0 |
| less-fewer | 0.01 | 0.00005 | 2 | 0 |
| irregardless, run-together | 0.01 | 0.0001 | 4 | 0 |
| chose-choose | 0.01 | 0.0004 | 10 | 0 |
| have-past | 0.01 | 0.00005 | 2 | 0 |
| comma-after-year | 0.04 | 0.0001 | 9 | 0 |
| comma-after-state | 0 | 0.0002 | 4 | 0 |
| compound-hyphen | 0.02 | 0.0007 | 17 | 0 |
| lowercase-i | 0.02 | 0.07 | 17 | 0 |
| day-month-names | 0 | 0.01 | 11 | 4 (word lists and data), 0.004 per 10,000 |
| comma-space | 0.17 | 0.09 | 22 | about 90% (lists of plant names and fields): 0.15 / 0.08 per 10,000 |
| doubled-mark | 0.08 | 0.09 | 18 | about 1 in 10 in docs, most in govdocs (dot leaders): 0.01 / 0.08 per 10,000 |

Style rules (off by default; flags per 10,000 words in docs): passive-voice 9.0, long-sentence (over 40 words) 26.8, repeated-sentence-start 3.4, wordy-phrases 4.7, in-order-to 2.0,
redundant-phrases 0.12, amount-number 0.03, try-and 0.01, could-care-less 0. They are suggestions and were not scored for false alarms.

### What the measuring changed

- `they-is`, `they-was`, `plural-has`, `plural-does` first had "you" as a subject; 8 of 8 flags read were "tell you is that" and "about you was your humor" (you as the object). "You" was taken out of all four.
- `demonstrative-number` flagged "this years program" (a missing apostrophe) and "these day trips": it now flags only where the noun ends the phrase.
- `to-much` flagged "amount to much" and "a boon to many": it is now only "is/are/was/seems ... to much" before the end of the clause.
- `comma-space` and `doubled-mark` flagged tables and CSV: three or more such commas in a paragraph, or runs of spaces, now silence them. `affect-effect` no longer flags "affect of" (the noun in psychology).

## French (round 38)

Measured November 13, 2026 with `tools/grammar/measure.py --language fr` (the numbers come from `grammartool --language fr`).

### The text

- **help**: our own French Help (`resources/help/fr`, 169 files, 150,378 words, with the markup taken out and code spans left as "x"). It is machine-translated text with the program's own labels in it,
  so it has typing and translation slips that real documents also have, and its quotation marks and colons are typed with plain spaces.
- **text**: the French program strings of six words or more (`translations/jeffoffice_fr.ts`) and the paragraphs of the French pages of the site: 107,837 words, in the same machine-translated voice.
- **govdocs**: Govdocs1 has no French prose worth reading: 3 files picked out by their common words gave 268 words, and the 2 flags there were data tables. LibreOffice and the other collections
  are spreadsheets. So the false-alarm count rests on the Help and the program text (258,000 words), and every flag of a rule with under 25 flags was read.
- The rules that have no flags in 150,000 words (a-accent, ou-accent, ce-se, contraction, ca, quant, hyphens, si-il, repeated-word) have only their tests as proof that they flag.

### Spaces, and why a no-break space

The spacing follows the country of the language tag. France and the rest (`fr`, `fr-FR`, `fr-BE`, `fr-CH`, `fr-LU`) take a no-break space before `; : ! ?` and inside `« »`. Canada (`fr-CA`, the convention of
the Office québécois de la langue française) takes one before the colon and inside `« »` only, so `;`, `!`, and `?` are never flagged there. The measurement below is for France.

French sets a space before `;` `:` `!` `?` and inside `« »`. Every word processor types a no-break space (U+00A0) there, and every font has it, so a suggestion puts that. The narrow no-break
space (U+202F) that careful typesetting uses is left alone, and so is any space that is not a plain one. There is no setting for it: one that took the narrow one would put a box in some fonts.
The flags there are the most frequent in French text (the Help types them with plain spaces): they are Punctuation, so they can be switched off with the kinds, like the others.

### The 14 rules of French (flags per 10,000 words of Help, then of program text; flags read; false alarms found)

| rule | help /10k | text /10k | read | false alarms |
|---|---|---|---|---|
| fr-space-before-mark | 89.04 | 126.58 | 2,704, about 70 read (every flag without a space before the mark) | 1 ("taxe;" in a formula, 0.01/10k); "#REF!", "B2;" and "mailto:" were found and are left alone now |
| fr-guillemets | 31.19 | 49.15 | 999, about 30 read | 0 |
| fr-elision | 0.47 | 0.18 | 9 of 9 | 2 ("De indique" and "de indique": a label, then a verb), 0.08/10k; "de et" ("from and"), "de y" (a value), and "que en" (a list of codes) were found and are left alone now |
| fr-sentence-start | 0.07 | 0 | 1 of 1 | 1 ("ppp signifie"), 0.04/10k; "p2s3 est" and "_xlfn. pour" (code) were found and are left alone now |
| fr-repeated-word | 0 | 0 | the one flag found was a word mentioned between guillemets, now left alone | 0 |
| fr-wordy-phrases (style) | 0.07 | 0 | 1 | not scored |
| the other 8 | 0 | 0 | | |

Reused from English, measured on the same text: double-space 0 (the paragraphs here have no double spaces once the markup is out), comma-space 0.07 (1 flag: `&"Arial,Gras"`, a false alarm,
0.07/10k), doubled-mark 0. Style rules (off by default): long-sentence 11.44, repeated-sentence-start 2.26 per 10,000 words.

Every error rule is under 1 false alarm per 10,000 words.

### What was narrowed or left out

- **ces / ses**: no mechanical test tells them apart ("ces" before a noun is right whichever it means). Only "ces sont" for "ce sont" is flagged.
- **ce / se**: only after il, ils, or on ("il ce lève"), never after elle (where "pour elle ce n'est pas" is right), and "se que / se qui / se dont" for "ce"; "il c'est passé" for "s'est".
- **a / à**: "il à mangé" only before a past participle, and the phrases that begin with "à" ("a partir de"). "Il à la maison" is left alone (it can be a clause with its verb left out).
- **ou / où**: "Ou est la gare ?" at the start of a sentence, "d'ou", "jusqu'ou", and "oui où non". "Ou" in the middle of a sentence is left alone.
- **elision** leaves "le onze", "la une", "de un à dix", "de et à", and words with a capital ("Le Ami") alone; "le / de" before "h" is never flagged (some of them take the apostrophe and some do not).
- **de le / à le**: skipped before an infinitive ("de le voir") and before a pronoun, so it misses a noun that ends like an infinitive ("de le quartier").
- **quand à**: only before a pronoun or a possessive ("quand à lui").
- **est-ce**: only "qu'est ce que / qui", "n'est ce pas", and "c'est à dire" before a comma or "que". "où est ce que" is left alone ("où est ce que je cherche" is right).
- **malgré que** and the other pleonasms are Style (off by default).
- Not checked: "peut être" and "plus tôt", which are right as often as wrong.

## German (round 38)

Measured November 13, 2026 with `tools/grammar/measure.py --language de`.

### The text

- **help**: our own German Help (`resources/help/de`, 169 files, 138,054 words, markup taken out); **text**: the German program strings of six words or more and the paragraphs of the German pages
  of the site (90,615 words). Both are machine-translated, so they hold few of the mistakes people make (a "dass" that is "das"), and the flags below were mostly about labels. Govdocs1 has no
  German prose (0 files found by their common words), and LibreOffice and the other collections are spreadsheets. This is the weakest measurement of the three languages: a rule with no flags
  in 228,000 words has only its tests as proof that it flags, and its false-alarm rate is only known to be under about 0.13 per 10,000 words.

### The 11 rules of German (flags per 10,000 words of Help, then of program text)

| rule | help /10k | text /10k | read | false alarms |
|---|---|---|---|---|
| de-repeated-word | 0.22 | 0.11 | 4 of 4 | 4 ("Vertikal verteilen verteilen", "Sortieren nach nach", "Gehe zu zu": the name of a command, then the sentence about it), 0.17/10k |
| de-comma-before-conjunction | 0 | 0 | 3 flags found first, all "etwa weil" (a comma stands before "etwa"): "etwa", "beispielsweise", "wohl", "eher", "bereits", "nämlich", "speziell", and "unter" are left alone now | 0 now |
| de-sentence-start | 0 | 0 | 4 found first, all "p2s3 ist", "_xlnm. beginnen" (code): now left alone | 0 now |
| the other 8 (das-dass, seid-seit, wider-wieder, noun-capital, als-wie, run-together, abbreviation-space, wordy-phrases) | 0 | 0 | | |

Reused from English, measured on the same text: comma-space 0.07 (1 flag, `&"Arial,Fett"`, a false alarm, 0.04/10k), space-before-mark 0 (it flagged 7 times a button named "?" first: "die Schaltfläche ? am
rechten Rand"; a "?" or "!" with a small letter after it is left alone now), double-space 0, doubled-mark 0. Style rules (off by default): long-sentence 5.87 and 5.52, repeated-sentence-start 2.03 and 0.11.

Every error rule is under 1 false alarm per 10,000 words.

### What was narrowed or left out

- **das / dass**: only ", das er" after a verb of thinking or saying ("Ich glaube, das er kommt": there is no noun for "das" to point back to), and "Dass ist" at the start of a sentence. A relative
  clause ("das er baute") looks the same as the other mistake, so "das" is never changed after a noun. "dass" is never changed to "das" in the middle of a sentence.
- **seid / seit**: "seid" before a length of time ("seid zwei Jahren") and "ihr seit" before a plain predicate ("ihr seit nicht"). "Ihr seid gestern gekommen" is right, so "seid gestern" is left alone.
- **wider / wieder**: "wider mal", "immer wider", and the like; "wieder Willen", "wieder Erwarten". "Hin und wider" is left alone.
- **comma before dass, weil, obwohl, ob**: not after und, oder, aber, sondern, nur, auch, so, als, ohne, kaum, and the words that go with the conjunction ("nur weil", "als ob", "ohne dass"); "ob" only before a
  pronoun (it is also a word that governs a noun, as in "ob seiner Leistung"). "wenn", "damit", "bevor", and "nachdem" are not checked (too many legitimate forms with no comma).
- **capital after an article**: only a list of about 120 nouns that are never a verb, adjective, or adverb in small letters ("essen", "leben", "kosten", "recht", "macht", "weg" are not in it).
- **als / wie**: after a comparative, not after "mehr" or "weiter" ("weiter wie bisher" is right).
- **ß and ss** are not a rule, and neither are the spelling reforms.
- **z. B. and the like** is Style (off by default): "z.B." is common and widely accepted; the rule only offers the spaced form.

## Spanish (round 38)

Measured November 13, 2026 with `tools/grammar/measure.py --language es`.

### The text

- **help**: our own Spanish Help (`resources/help/es`, 169 files, 147,999 words, markup taken out); **text**: the Spanish program strings of six words or more and the paragraphs of the Spanish pages of
  the site (104,663 words); **rules**: the Puerto Rico court rules in Spanish that courtrules.net imports (`~/court-sources/pr_import_cache/*.txt`, text taken from PDFs, 405,362 words, read in place and
  not copied). The rules are real legal Spanish written by people, which the other two are not: they have page numbers, tables of contents, and line breaks that the PDF left in. 658,000 words in all.
  Govdocs1 has no Spanish prose that the common-word test could find (0 files).

### The 10 rules of Spanish (flags per 10,000 words in the help, program text, and court rules)

| rule | help | text | rules | read | false alarms |
|---|---|---|---|---|---|
| es-repeated-word | 0 | 0 | 0.02 | 8 found first, 1 left | 6 first, all gone but the 1 left: "está está" (the relative clause "la ventana en la que está está vacía"), "Acerca de de" and "Después de de" (the name of a command, then the sentence); the 1 left is "de de" in a table of contents (0.015/10k) |
| es-de-que | 0 | 0 | 0.02 | 1 of 1 | 0 ("se da cuenta que": a real queísmo) |
| es-hay-ahi-ay | 0 | 0 | 0 | 2 found first | 2 ("De ahí que": hence; left alone now) |
| es-del-al | 0 | 0 | 0 | 1 found first | 1 ("&A el nombre": a header code; left alone now) |
| es-sentence-start | 0.07 | 0 | 0 | 1 of 1 | 1 ("ppp significa", 0.015/10k) |
| es-inverted-marks, es-question-accent, es-a-ver, es-hecho | 0 | 0 | 0 | | |
| es-wordy-phrases (style) | 0.07 | 0 | 1.92 | not read | not scored ("con el fin de", "en base a") |

Reused from English, measured on the same text: space-before-mark 0.07, 0, 0.05 (4 flags: two a label with its text left out, two page-number lines in the PDF text; not scored), comma-space 0.07 (1 flag,
`&"Arial,Bold"`, a false alarm, 0.015/10k), double-space and doubled-mark 0. Style rules (off by default): long-sentence 11.01, 13.19, and 78.55 (legal sentences), repeated-sentence-start 2.43, 0.19, and 0.84.

Every error rule is under 1 false alarm per 10,000 words. The inverted marks found nothing in 658,000 words of text that was written with care, which is what it should do; the tests are the only proof
that it flags.

### What was narrowed or left out

- **¿ and ¡**: a "?" or "!" after a word, at the end of the text or before a space, in a sentence that has no "¿" or "¡" in it. A small letter after it ("Yahoo! es") goes on with the sentence, and a
  period after a short form ("Sr.", "EE. UU.", "pág.") does not end it. With a comma before the mark the question may begin after the comma, so the mark is only pointed at, with nothing to put in place.
- **accents on the question words**: only the first word after "¿" (or after "y", "pero", "entonces"): "¿Que?", "¿Como?", "¿Porque?". "¿Que no vienes?" is left alone (it is not a question word there),
  and so are "¡Que viva!" and any question word in the middle of a question. The exclamation marks are not checked for accents.
- **a ver / haber**: "haber si" at the start of a clause, and "a ver" after puede, debe, podría, and the like. "Va a ver" and "va a haber" are right and left alone.
- **hay / ahí / ay**: "ahí" before a quantity or "que" (not after "de": "de ahí que"), "ay" in the middle of a sentence before a quantity or "que", and "de hay", "por hay", "hasta hay", "desde hay".
- **del / al**: "de el" and "a el" before a plain noun; not before a word like "mismo" or "que", which makes "él" without its accent likelier.
- **de que**: "pienso de que" and the like (creer, pensar, opinar, suponer), "me alegro que", "me acuerdo que", and "dar cuenta que". Other verbs take "de que" in some uses and not in others.
- **echo / hecho**: "ha echo" gets two suggestions, "hecho" and "echado". Not checked: "mas" and "más", "si" and "sí", "tu" and "tú", "sino" and "si no" (each is right in a sentence of its own), and "si" before the
  conditional.

## Where human-written text is missing (re-measure when we have some)

Most of the text above is our own machine-translated Help, program strings, and site pages, which holds few of the mistakes people make and many labels. Re-measure these when human-written text arrives:

- **French**: all of it. There is no human-written French at all (Govdocs1 gave 268 words). The rules with no flags (a-accent, ou-accent, ce-se, contraction, ca, quant, hyphens, si-il, repeated-word) are
  proved by their tests only. fr-CA has not been measured separately (it only turns off three marks).
- **German**: all of it. The weakest of the three: 229,000 words, no human-written text.
- **Spanish**: only the court rules of Puerto Rico (405,000 words) are human-written, and they are legal text from PDFs with no questions or exclamations in them. So es-inverted-marks, es-question-accent,
  es-a-ver, es-hay-ahi-ay, and es-hecho have never seen ordinary prose; the rest were checked against the court rules.
