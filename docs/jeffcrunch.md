# JeffCrunch

JeffCrunch is the spreadsheet app of JeffOffice. JeffOffice is an independent project: JeffCrunch aims to work with Excel workbooks and the people who use them (compatibility and interoperability), and then to do more, and do it better. Its feature checklist is [features/jeffcrunch.md](features/jeffcrunch.md); this note is the design, the order it is built in, and how each step is measured.

## The decisions (October 22, 2026)

- **Our own calculation engine**, in C++ (`libs/base/calc`). A spreadsheet engine is not a part you call once: every cell read, edit, recalculation, and undo goes through it, so it has to own its storage, recalculate only what an edit changed, and share undo with the app. It also serves JeffWrite's fields, JeffBase, chart axes, and `TEXT()`. IronCalc, an open-source engine, is run once through the same yardstick when the yardstick exists, so the choice is scored rather than argued.
- **About 110 functions first.** In the public workbooks, the 50 most used functions cover 91% of the workbooks that use functions and the 100 most used cover 99%. A formula with a function JeffCrunch doesn't have yet keeps the result the file stored, and shows `#NAME?` only once something it depends on changes.
- **Macros are kept, never run.** A workbook's macro project is read, kept untouched, and saved back. Converting macros to a JeffOffice script (translated to JavaScript, reviewed by you, run only when you allow it) comes after the preview, as its own project.
- **Formats.** `.xlsx` is the default. The first release opens `.xls` (Excel 5 to 2003), `.ods`, and `.csv`; saving `.xls` and `.ods` follows once `.xlsx` is solid. Whatever JeffCrunch can't edit yet is kept and written back as it was.

## The preview bar

JeffCrunch ships as a preview when, on public workbooks never used for tuning, it meets every line below. Each is measured by a program, with no other spreadsheet involved: a workbook stores the last result of every formula, so recalculating every formula and comparing with what the file stored checks the engine against thousands of real answers.

| Measure | Preview bar |
|---|---|
| Opening | Every file opens with no crash or hang; 99% of undamaged files keep their own cell count |
| Recalculating every formula against its stored result (to 15 digits; text, true/false, and error kind exactly; volatile, external, and unsupported formulas listed apart) | 90% of formula workbooks match fully; 98% of formula cells match; each of the 50 most used functions matches 99% of the time |
| Function test files from the public LibreOffice test suite | All for the first 110 functions; 80% of the rest |
| Number formats, checked against stored `TEXT()`, `FIXED`, and `DOLLAR` results | 98% |
| Saving | No crashes; 99.5% of workbooks read back the same; 100% of parts JeffCrunch didn't touch written back identical |
| `.xls` | Every file opens; 98% of values equal those of the same workbook as `.xlsx` |
| Damaged and deliberately broken files, plus 5,000 generated mutations | No crash or hang within 4 GB and 120 seconds |
| Speed: a 100,000-row by 20-column sheet, and the largest public sheet (151 MB) | First screen under 1 and 3 seconds; painting under 8 ms for 95% of frames; an edit repainted within 16 ms for 99% of edits |

Version 1.0 raises the recalculation bar to 98% of workbooks and 99.9% of cells, and the function test files to 95%.

**The workbooks.** The public collections (business models from the Enron case, school and lab workbooks, and workbooks from government sites) are split in two by each workbook's own name, so that copies of one workbook stay together: a practice half for tuning, and a held-out half that is only ever scored, in totals. The LibreOffice and POI test files are never tuned on; they are used for crash and hang sweeps.

## The model

- `libs/base/calc`: values (number, text, true/false, error, empty, and arrays from the start), number formats and dates (both date systems), the formula tokenizer and parser to bytecode, the evaluator over an abstract cell source, the dependency graph, and the function registry. No GUI.
- `libs/office/sheet`: the workbook model (sparse cells in row blocks, shared strings, styles, merges, names, tables, and the parts kept as read), the `.xlsx` reader and writer, then `.xls` and `.ods`.
- `libs/shell/grid`: the grid view on a grid model. Row heights are a default plus overrides with running sums; only the cells in view are painted, from a cache of their display text.
- `apps/crunch`: the window, commands, undo, sort, filter, and pages for printing.

**Recalculation is incremental.** A cell lists the cells that depend on it; a range is kept as a range in an index per sheet, never expanded to its cells. An edit marks what depends on it and recalculates in order with an explicit stack (a running balance 100,000 rows deep would overflow the system's stack). Volatile functions (`NOW`, `RAND`, `INDIRECT`, `OFFSET`) are re-registered on each run. Recalculation runs on the main thread in short slices, so typing never waits, and Esc stops it. Loading, saving, AutoRecover, sorting, and CSV parsing run on worker threads from a snapshot. The arithmetic follows the format's established behavior, oddities included (February 29, 1900 exists; sums that should be zero come out as zero), because the results files store depend on it.

**Undo** keeps changes, not copies: a command records the cells, styles, sort order, or inserted rows it changed, and undo applies the inputs again and recalculates.

**Hostile files.** Every count, size, and depth a file controls is bounded, at least ten times above what real workbooks use, and formula nesting is limited. The package reader inflates a part only when it is read, and sheets are read by a streaming scanner, so one huge sheet never has to fit in memory at once.

## Waves

Each wave ends with its own gate: the tests, the layer check, and the yardstick numbers it moves.

1. **The yardstick and the readers it needs.** `jcrunchtool`, which runs a list of workbooks under memory and time limits and writes a table of results; the package reader inflating parts only when read; a streaming sheet reader with shared strings; and the first scores (opening, cell counts, the formulas' stored results read).
2. **Number formats.** The whole format language (sections and conditions, colors, text, fractions, elapsed time, both date systems, locales), checked against stored formatted results.
3. **Formulas.** The tokenizer, parser, and evaluator, the dependency graph, and the first 110 functions, scored by recalculating every formula.
4. **The grid and editing.** The window, the grid, selection, editing, undo, formatting, and saving `.xlsx`, scored for speed and for saving.
5. **Toward the preview.** `.xls` and `.ods` reading, printing, charts that follow their cells, and the remaining preview items in the checklist, then the hostile sweep and the held-out scores.
