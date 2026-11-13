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
