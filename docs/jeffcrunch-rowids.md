# JeffCrunch: row and column ids

*Design note, October 26, 2026. A plan, not a change: no product code is touched by the commit that adds this note. Under `tools/` it adds the bench that took the numbers (`jcrunchtool edit-struct-bench` and `edit-save-struct-bench`) and the script that makes the bench's workbooks (`crunch_struct_bench_make.py`).*

Inserting or deleting a row, a column, or cells in JeffCrunch costs the size of the sheet. Every cell after the edit is moved, every formula that reads what moved is written again, every value the window holds is moved, and a record of all of it is kept for Undo. The preview bar for an edit is 16 ms on the window's thread. A person typing meets it (a number typed over a cell is 4 ms in the middle, 6 ms at the 99th percentile); a row inserted at the top of a sheet of 100,000 rows does not (0.2 s), and on a sheet of a million rows it takes a second, or eight for a column. The first release noted this as "the work that is left". This note says what that work is, in what order to do it, and how to know each step worked.

The goal: a single row or column inserted or deleted anywhere in a million-row sheet takes under 16 ms on the window's thread, Undo and Redo the same, the work that must still be done (formulas that really change) is done by the worker, and saving, memory, and every result the yardsticks give are unchanged.

In short:

- **Now** (100,000 rows by 21 columns): a row at the top 0.17 s, a column 0.6 s, saving after any such edit 4.7 s; (1,000,000 rows by 5): 1.1 s, 7.4 s, 19.7 s, and 1.5 GB of Undo records for twelve edits. Everything that is per cell or per formula is keyed by position, so an edit renumbers all of it.
- **Recommended**: every row and column gets an id equal to its file position, kept in a short list of runs (`LineMap`) that grows by two entries per edit. Cells, formula nodes, the graph, the window's values, and the session's marks hold ids and never move; the few things (rules, names, charts, notes, and the like) keep positions and the code they have. Formulas keep their file texts; references are read through the map. Undo is the inverse map operation and costs kilobytes.
- **Targets**: one row or column inserted or deleted anywhere in a million rows, under 16 ms on the window thread (the model and session part under 3 ms); Undo and Redo the same; saving no slower, and the same bytes for sheets with no structural edit; memory no more than +10% at rest.
- **The spike** (section 3A): both designs built at their core on the real graph. A's edit is a microsecond at 100,000 and at 1,000,000 rows (0.06 ms after 1,000 random edits); B, the rival, is 4.6 to 9.9 ms at 100,000 rows and 43 to 80 ms at a million (the graph's pass), and a column 91 to 132 ms and 1.1 s. A recalculates 23 to 26% slower after 1,000 edits; B stays flat. Recommendation: A.
- **Plan**: thirteen rounds, about 41 days (8 weeks, range 6 to 10) after the review; rounds that change meaning ship dark behind a switch, so each ships alone; the first (a stronger model check with an independent oracle) and the second (`LineMap`) change no behavior.

## 1. Measurements now

### 1.1 How they were taken

- `jcrunchtool edit-struct-bench <file.xlsx> <label> [only]` opens a workbook in the window's model as the window does and waits until it is worked out. For each of twelve edits (one row inserted, and one deleted, at the top, the middle, and the end of the used rows; the same for columns; "end" is the second-to-last used row or column) it calls the editing session, repaints the grid's viewport, and records: the window thread's own processor time from the call to the repaint ("window thread"), the worker's processor time, the wall time until the worker has settled, the model's counted work (`Calculation::editWork()`: formulas looked at, cells moved, cells deleted), and the distinct formula texts the edit made. Then it undoes the edit (Undo timed the same way) so the next edit starts from the same sheet.
- `tools/crunch_struct_bench_make.py` makes the workbooks. Every sheet has `ROWS` rows of numbers and, after them, a column that adds each row up, written as one shared formula as the common programs write a filled-down column. Flags add what the brief asked for: `--cross` (a column that reads the row above, and six formulas that read whole columns: totals, `INDEX`, `COUNT(A:A)`), `--cf` (three conditional formats over the data), `--dv` (two data validations), `--table` (a table of 1,000 rows and 3 columns), `--chart` (a chart with a series of 200 rows and one that reads all 100,000), `--names` (50 defined names, some over the whole data), and `--all`. The sheets measured: 100,000 rows by 21 columns (20 numbers and the sum) in each variant, and 1,000,000 rows by 5 columns (4 numbers and the sum) plain and with everything (11 columns).
- `edit-save-struct-bench` saves the workbook as the session would (`takeSnapshot`, then `saveXlsx`) untouched, and after 1, 3, 20, and 200 structural edits (a row or a column inserted or deleted at a random place, none undone) in separate runs.
- This machine: Linux, 32 cores, offscreen window of 1,280 by 800, `JP_NO_TELEMETRY=1`, each run in `systemd-run --user --scope -p MemoryMax=16G` (24G for the largest). There is no `perf` here, so "where the time goes" is timers inside the edit (kept out of the commit) and the counters. Times vary by about 20% with other work on the machine; the tables are one run each. For the million-row runs the undo limit was raised to 8,000 MB (`JC_UNDO_MB`): at its normal 128 MB half of the edits there ask "This can't be undone" first and are not done (see 1.5).

### 1.2 Times, 100,000 rows by 21 columns, plain

| Edit | Window thread (ms) | Undo (ms) | Worker CPU (ms) | Settled by (ms) | Work counted | Texts made |
|---|--:|--:|--:|--:|--:|--:|
| insert row top | 174 | 166 | 173 | 342 | 2,200,000 | 1 |
| delete row top | 351 | 258 | 282 | 627 | 2,200,000 | 99,997 |
| insert row middle | 125 | 172 | 84.6 | 199 | 1,100,000 | 0 |
| delete row middle | 167 | 134 | 175 | 334 | 1,100,000 | 0 |
| insert row end | 10.9 | 6.51 | 0.10 | 11.5 | 44 | 0 |
| delete row end | 6.75 | 6.87 | 0.10 | 7.20 | 44 | 0 |
| insert column top | 601 | 639 | 218 | 814 | 2,200,000 | 0 |
| delete column top | 620 | 1,145 | 224 | 831 | 2,200,000 | 0 |
| insert column middle | 651 | 660 | 192 | 833 | 1,200,000 | 0 |
| delete column middle | 685 | 713 | 216 | 890 | 1,200,000 | 0 |
| insert column end | 617 | 659 | 178 | 784 | 300,000 | 0 |
| delete column end | 674 | 710 | 233 | 896 | 300,000 | 0 |

Reading it: "work counted" is the model's own count of formulas looked at plus cells moved plus cells deleted, so 2,200,000 is the 2.1 million cells of the sheet that move plus the 100,000 formulas that read them. The edit at the end of the sheet moves nothing (44 units of work) and still costs 7 to 11 ms: that is the floor today. A column costs three to four times a row. Deleting the first row is the worst row case (0.4 s) because the shared formula's text is held by the cell in that row: every one of the other 99,997 formulas is given a text of its own. Settling means the worker has recalculated what it was told depends on the cells that moved; no value changed, and it still took 0.2 to 0.9 s.

### 1.3 Times with formulas that read across the edit, rules, a table, a chart, and names (window thread, ms)

| Edit | base | cross | cfdv | table | chart | names | all |
|---|--:|--:|--:|--:|--:|--:|--:|
| insert row top | 174 | 502 | 175 | 166 | 201 | 174 | 551 |
| delete row top | 351 | 753 | 425 | n/a | 401 | 354 | n/a |
| insert row middle | 125 | 285 | 174 | 65.0 | 157 | 129 | 267 |
| delete row middle | 167 | 345 | 184 | 171 | 202 | 198 | 411 |
| insert row end | 10.9 | 21.2 | 14.0 | 10.1 | 26.4 | 11.2 | 39.0 |
| delete row end | 6.75 | 11.3 | 8.72 | 10.2 | 20.4 | 7.71 | 29.9 |
| insert column top | 601 | 1,275 | 721 | 413 | 750 | 656 | 1,068 |
| delete column top | 620 | 1,301 | 672 | 441 | 769 | 616 | 1,100 |
| insert column middle | 651 | 1,332 | 771 | 425 | 715 | 718 | 1,173 |
| delete column middle | 685 | 1,540 | 807 | 484 | 770 | 688 | 1,305 |
| insert column end | 617 | 163 | 745 | 31.9 | 701 | 641 | 85.1 |
| delete column end | 674 | 196 | 799 | 40.6 | 853 | 673 | 83.8 |

`cross` has 100,000 formulas that read the row above and six that read long ranges or a whole column; `cfdv` three conditional formats and two validations over the data; `table` a table of 1,000 rows at the right; `chart` a chart over the data; `names` fifty names; `all` all of them. The rules, the chart, and the names add almost nothing (they are few, and are followed by code that costs what they hold). What adds is formulas that read across the edit: another 100,000 formulas to write again (`cross`: 0.5 s for a row at the top, 1.3 s for a column), and the table, which makes the edit refuse to delete its header row (shown as n/a). In `table` and `all` the sheet is 27 columns wide (the table is at the right), so "end" is a column in front of the table and moves only the table's few cells; in `cross` it is the column just before the last two, which are the read-the-row-above formulas.

### 1.4 Times, 1,000,000 rows by 5 columns (window thread, ms)

| Edit | Window thread (ms) | Undo (ms) | Worker CPU (ms) | Settled by (ms) | Work counted | Texts made |
|---|--:|--:|--:|--:|--:|--:|
| insert row top | 1,077 | 1,174 | 880 | 1,958 | 6,000,000 | 1 |
| delete row top | 4,273 | 2,806 | 2,627 | 6,889 | 6,000,000 | 999,997 |
| insert row middle | 1,286 | 1,760 | 468 | 1,700 | 3,000,000 | 0 |
| delete row middle | 1,865 | 1,351 | 1,155 | 2,973 | 3,000,000 | 0 |
| insert row end | 53.9 | 16.2 | 0.10 | 54.5 | 12 | 0 |
| delete row end | 15.1 | 16.4 | 0.20 | 15.5 | 12 | 0 |
| insert column top | 7,363 | 7,401 | 1,078 | 8,429 | 6,000,000 | 0 |
| delete column top | 7,814 | 13,787 | 1,648 | 9,353 | 6,000,000 | 0 |
| insert column middle | 8,343 | 8,533 | 1,097 | 9,357 | 4,000,000 | 0 |
| delete column middle | 8,969 | 8,761 | 1,955 | 10,842 | 4,000,000 | 0 |
| insert column end | 7,668 | 7,846 | 1,036 | 8,604 | 3,000,000 | 0 |
| delete column end | 8,192 | 7,819 | 1,608 | 9,721 | 3,000,000 | 0 |

The same with everything in the sheet (11 columns: the table, the chart, the rules, the names, and the formulas that read the row above; window thread, ms):

| Edit | base | all |
|---|--:|--:|
| insert row top | 1,077 | 5,164 |
| delete row top | 4,273 | n/a |
| insert row middle | 1,286 | 1,908 |
| delete row middle | 1,865 | 4,132 |
| insert row end | 53.9 | 191 |
| delete row end | 15.1 | 77.7 |
| insert column top | 7,363 | 10,851 |
| delete column top | 7,814 | 11,769 |
| insert column middle | 8,343 | 1,718 |
| delete column middle | 8,969 | 2,195 |
| insert column end | 7,668 | 344 |
| delete column end | 8,192 | 319 |

Every term is six to twelve times larger than at 100,000 rows, for ten times the rows: it is work that grows with the sheet. A row at the top is 1.1 s (5.2 s with the rules, chart, table, names, and the read-the-row-above formulas), a column 7 to 11 s, and the worker takes 0.5 to 6 s more to settle.

### 1.5 Where the time goes

Phases of one edit (window thread, ms; from timers inside `applyStructure` and `Calculation::applyEdit`). "Candidates" is the formulas the graph says read the moved region, "text rewrite" is the loop that decides, for each, what its text becomes (shared families are rewritten once per distinct fate, but every candidate is still visited), "cells move" is `CellStore::shiftRows` or `shiftColumns` with its per-formula callback, "write formulas" is the loop that gives each rewritten formula its text and its node in the graph again, "session" is the marks the editing session keeps by cell, and "values" is the window's `ValueStore::relocate`.

100,000 rows by 21 columns:

| Edit | Candidates | Text rewrite | Cells move | Graph | Write formulas | Session | Values | Window thread total (ms) |
|---|--:|--:|--:|--:|--:|--:|--:|--:|
| insert row top | 3.8 | 51 | 60 | 2.3 | 26 | 9.1 | 23 | 188 |
| delete row top | 2.6 | 252 | 30 | 2.7 | 73 | 10 | 18 | 401 |
| insert row middle | 1.4 | 64 | 13 | 2.3 | 29 | 3.5 | 10 | 143 |
| insert row end | 0.0 | 0.0 | 0.0 | 2.0 | 0.1 | 0.0 | 0.1 | 12 |
| insert column top | 2.6 | 229 | 17 | 0.0 | 393 | 8.8 | 18 | 683 |
| delete column top | 2.9 | 232 | 20 | 0.2 | 338 | 9.9 | 24 | 697 |

1,000,000 rows by 5 columns:

| Edit | Candidates | Text rewrite | Cells move | Graph | Write formulas | Session | Values | Window thread total (ms) |
|---|--:|--:|--:|--:|--:|--:|--:|--:|
| insert row top | 13 | 273 | 142 | 12 | 178 | 119 | 297 | 1,077 |
| delete row top | 14 | 2,469 | 140 | 17 | 1098 | 149 | 322 | 4,273 |
| insert row middle | 6.7 | 559 | 42 | 12 | 324 | 53 | 160 | 1,286 |
| insert row end | 0.0 | 0.0 | 0.0 | 12 | 0.1 | 0.0 | 0.1 | 54 |
| insert column top | 14 | 2,425 | 47 | 0.0 | 4,324 | 124 | 347 | 7,363 |
| delete column top | 15 | 2,525 | 60 | 2.3 | 3,879 | 148 | 448 | 7,814 |

Counted work for the same edits (the record the model keeps):

(100,000 rows by 21 columns)

| Edit | Formulas the graph named | Cells moved | Cells deleted | Formulas written again | Formula places moved | Old texts kept for Undo |
|---|--:|--:|--:|--:|--:|--:|
| insert row top | 100,000 | 2,100,000 | 0 | 100,000 | 100,000 | 1 |
| delete row top | 100,000 | 2,099,979 | 21 | 99,999 | 99,999 | 99,999 |
| insert row middle | 50,000 | 1,050,000 | 0 | 50,000 | 50,000 | 50,000 |
| insert row end | 2 | 42 | 0 | 2 | 2 | 2 |
| insert column top | 100,000 | 2,100,000 | 0 | 100,000 | 100,000 | 100,000 |
| delete column top | 100,000 | 2,000,000 | 100,000 | 100,000 | 100,000 | 100,000 |

What this shows:

1. **Everything is linear in the sheet.** At 100,000 rows the phases that grow with cells or formulas are 90% of a row inserted at the top (text rewrite 51 ms, cells move 60, write formulas 26, values 23, session 9: 169 of 188); at 1,000,000 rows they are 96% (1,034 of 1,077).
2. **Columns lose their fast paths.** The graph is told rows were shifted (`DependencyGraph::shiftRows`, a pass over its entries), but for columns every candidate formula is dropped and made again: 393 ms where rows spend 26.
3. **The floor is not zero.** An edit that moves nothing, at the end of the 100,000-row sheet, is 7 to 11 ms: the graph's pass (2 ms; it runs for every row edit) and the view and the repaint (about 8 ms). At a million rows the same edit is 15 to 54 ms, of which the graph's pass is 12 to 17.
4. **The record of an edit is large, and Undo is as slow as the edit.** Twelve edits on the million-row sheet leave 1.5 GB in undo records (resident 465 MB after opening, 1,969 MB after the twelve; with everything in the sheet 1,196 MB and 2,457 MB, peak 3.0 GB): the estimate in the code is 320 bytes for each cell deleted and each formula written again, and the sheet's lists of rules are copied whole. At the normal limit (128 MB) six of the twelve edits on the million-row sheet are not done until the person has answered "This can't be undone, and neither can what you did before it. Do you want to go on?", and so are four of the twelve on the 100,000-row sheet that has formulas reading across the edit.
5. **Undo of a column delete is the slowest thing measured** (1.1 to 1.5 s at 100,000 rows, 13.8 s at a million): the deleted cells are put back one by one through the same call as a typed cell.
6. **The worker repeats the work.** After a row is inserted at the top of 100,000 rows the worker works out 100,000 formulas whose values cannot have changed (0.09 to 0.3 s of its time), because the graph can say only "this formula reads something near the edit"; for the 1,000,000-row sheet 0.9 to 4.6 s.
7. **Opening** the 1,000,000-row sheet takes 16 s and 465 MB resident (6 million cells at 24 bytes is 144 MB of it); with everything in it, 23 s and 1.2 GB. A formula costs about 100 bytes besides its cell (a `Formula` of 16, a node of 32, graph entries of 16 each, a value of 24).

### 1.6 Saving after structural edits

| Sheet | Structural edits made | Snapshot (ms) | Save (ms) | File (bytes) | Peak resident (MB) |
|---|--:|--:|--:|--:|--:|
| 100,000 by 21 | 0 | 0 | 4.8 | 10,161,070 | 148 |
| | 1 | 43 | 4,726 | 9,285,892 | 258 |
| | 3 | 169 | 5,320 | 9,250,688 | 332 |
| | 20 | 135 | 6,195 | 7,783,650 | 700 |
| | 200 | 123 | 3,975 | 1,903,493 | 3,847 |
| 100,000 by 27, everything | 0 | 0 | 5.7 | 11,355,452 | 281 |
| | 20 | 191 | 5,502 | 8,706,134 | 1,188 |
| | 200 | 74 | 3,897 | 3,007,401 | 1,725 |
| 1,000,000 by 5 | 0 | 0 | 12 | 26,536,852 | 493 |
| | 10 | 1,968 | 19,723 | 23,255,341 | 2,689 |

Saving a sheet nobody changed copies its part as it is (milliseconds). The first structural edit makes the saver stream the whole part and write every row and cell again, renumbered (a shared formula is written out for each cell that uses it, so that none loses its text): 4.7 s at 100,000 rows (2.1 million cells) and 19.7 s at a million rows (6 million cells, besides 2 s for the session's snapshot), which is 2 to 3 microseconds a cell, and it does not grow with the number of edits (1 edit 4.7 s, 3 edits 5.3 s, 20 edits 6.2 s; the 200-edit row is faster because random deletes had taken most of the cells out). Memory goes up with it: 148 MB untouched, 258 MB after one edit. So the saver needs two things from this work: to be no slower, and, as an addition to it, a faster way to renumber (4.8). The replaying of the edit list is not what dominates it: 3 edits cost 5.3 s and 200 cost 4.0 s.

## 2. How an edit works today

`EditSession::insertRows` (and the other five calls) goes to `structure()`, then `applyStructure` in `apps/crunch/core/editstruct.cpp`, which holds the worker back (`WorkbookLoader::hold`) and calls `Calculation::applyEdit` (`libs/office/sheet/calcedit.cpp`). In that order:

1. The graph is asked which formulas read the region the edit moves (`dependentsOfArea`), and the formulas that read where they are told (`m_special`) are added.
2. Each such formula gets its new text: shared families once per distinct fate (`TextRewriter`), the rest through `adjustFormula`.
3. The cells that a delete removes are copied into the record.
4. `CellStore::shiftRows` or `shiftColumns` moves every cell from the edit on (cells carry their row and column) and tells a callback about each formula cell that moved, which updates its node and appends to the record.
5. The graph is told rows were shifted. For columns, no.
6. The sheet's own lists follow (arrays and data tables, merged ranges, the filter, row and column sizes, the extent), then validations, conditional formats, and defined names.
7. Each formula that was rewritten is given its text (`retext`) and made again in the graph (`dropNode`, `addNode`), and everything that read the region is queued for the worker.

Back in the session: tables move (`Workbook::moveTables`), the marks kept by cell move (`followEdit`), page breaks and print areas follow (`PageSettings`), notes and links follow (`CellAnchors`), the window's formula values are moved (`ValueStore::relocate`), the edit is appended to the lists the saver and the worker's batches use (`m_edits`, `m_log`), and the window rebuilds the sheet's view and repaints. Charts and drawings catch up from the list of edits when they are next used (`SheetObjects`).

What is keyed by position, so what an edit must move or rewrite:

| What | Held as | Followed today by | Costs, per edit |
|---|---|---|---|
| Cells | `Cell.row`, `Cell.col`, in blocks of 64 rows | `CellStore::shiftRows`, `shiftColumns` | every cell from the edit on (columns: every cell) |
| Formula nodes | `Node.row`, `Node.col` | the move callback | each formula that moved |
| Formula texts | A1 text of a shared family plus `dRow`, `dCol` for each cell | `TextRewriter`, `adjustFormula`, `retext` | each formula that reads the region |
| Dependency graph | row intervals in a tree over 16,384 columns | `shiftRows` (rows); drop and add (columns) | every interval of the sheet |
| Window's formula values | hash by sheet, row, column | `ValueStore::relocate` | each formula that moved |
| Worker batches | items by position, with a generation | `EditSession::moveToNow` over `m_log` | each item of a batch in flight |
| Session marks | `m_touched`, `m_results`, `m_engineAtStart`, `m_fileResults`, `m_rowsResized`, `m_columnsResized` | `followEdit` | every entry, and every formula written again is marked |
| Undo record | `EditRecord`: copies of deleted cells, old texts, old lists | `Calculation::undoEdit` runs the inverse edit and puts the copies back | cells deleted plus formulas written again, about 320 bytes each |
| Row and column sizes | `RowSpec.row` (all rows with a size, hidden state, or outline level), `ColumnSpan` | rewritten as new vectors, old ones copied for Undo | every spec (a million rows with heights: about 40 MB copied) |
| Merged ranges, arrays, data tables, filter | `CellRange` | `adjustRange` | each |
| Validations, conditional formats | ranges and formula text for the first cell | `adjustValidations`, `adjustConditionalFormats` | each rule (the whole lists are copied for Undo) |
| Defined names, print area and titles | text with references | `adjustFormula` | each name |
| Tables | `Table` ranges; formulas that name their columns | `tableAfter`, `tablesChanged()` | each table; every formula that names one is registered again |
| Charts, drawings, notes, links, page breaks | series text, anchors, a map by cell, `PageBreak.before` | `SheetObjects`, `CellAnchors`, `pagefollow` | each |
| The file | the list `m_edits` | `structsave.cpp` replays it over every part of the old file | per row (and per cell, for columns) times edits |
| The view | selection, scroll, frozen panes | the window | constant |

The first five rows and the undo record are the ones that grow with the sheet. The rest grow with how many rules, names, objects, and notes there are, which is small except on purpose (a hundred thousand notes), and the plan leaves them alone for now.

Edits that are not whole rows or columns (insert or delete cells, shifting the rest of their columns or rows; cut and paste of a block) move only some of a sheet's cells and are rarer. They keep the code they have; see 4.10.

## 3. The options

**(a) Stable ids for every row and column, with a position map.** Rows and columns get permanent ids; a position map (an order-statistic tree, or blocked arrays) turns a position into an id and back; cells, formulas, and everything else that is per cell or per formula hold ids, so nothing moves when a row is inserted. This is what the first estimate described: 3 to 4 weeks. In its textbook form it pays for the map in every sheet whether it was ever edited (a table of 4-byte ids and a tree over a million rows: 12 to 16 MB, and a cell store that must find a row's cells through it).

**(b) A cheaper middle path.** Positions stay the key; each structure gets its own lazy shift. Blocked cell storage so an insert moves pointers (blocks hold their first row and rows are counted from it), formulas keep their positions plus the list of edits and are rewritten when read, the graph answers queries through the list, the window's values are keyed by the formula's place in the sheet's list instead of by cell. It has two forms:

- *b1, the step of about 700 lines estimated before:* use the graph's shifted path for columns as for rows, visit shared families once instead of once per formula, and mark fewer formulas. By the profile above it brings a column to the cost of a row (100,000 rows: 0.6 s to about 0.17 s) and does nothing for rows.
- *b2, every structure lazy on its own:* cells (blocks with offsets), nodes, values, graph, texts, session marks, each with its own "pending shifts" list. Each of them composes a list of edits into a piecewise map from old position to new, which is the position map of (a) written seven times. It would reach about 0.1 s for a row at the top of 100,000 rows and 0.4 to 0.5 s at a million (what is left is the visit of every candidate formula and the rewrite loop), and it is the same work as (a) with a worse invariant: no single place says where a row is.

**(c) What else was considered.**

- *Do the same work on the worker, show a transformed view until it finishes.* The window thread gets cheap but the work and the records stay, a second edit has to wait for the first, and Undo is as slow as now. Rejected; the "transformed view" half is the position map of (a).
- *Run maps instead of id tables.* A row's id is its position in the file; a sheet nobody has inserted into has the identity map and holds nothing. An insert makes ids that are new; a delete takes ids out. The order is kept as a short list of runs (first id, count) with running counts, which has one entry for each edit made, not for each row. **This is (a) in the form to build, and it is the recommendation (4).**
- *Copy-on-write blocks* (cells in blocks shared between snapshots), which make Undo of cell edits free. Not needed once cells do not move; kept as a later option.
- *Renumbering ids to positions in the background* ("rebasing") so the run list never grows. Not needed at first (the list has two entries per edit); it is the relief valve if a very long session needs it (4.12).
- *Positions as order labels with spaces between* (so an inserted row gets a label between its neighbors). Needs relabeling when a gap runs out, and relabeling touches everything keyed by label. Rejected for the run map.

Comparison, for a row inserted at the top (window thread; the 100,000-row sheet and, in brackets, the 1,000,000-row sheet), a column near the left, and what each costs to build:

| | Row at the top | Column | Work that still grows with the sheet | Undo record | Weeks |
|---|--:|--:|---|---|--:|
| Now | 0.17 s (1.1 s) | 0.6 s (7.4 s) | everything | 320 bytes a cell or formula | |
| b1 | 0.17 s (1.1 s) | 0.17 s (1.1 s) | everything | same | 0.5 |
| b2 | about 0.1 s (0.45 s) | about 0.1 s (0.45 s) | the formulas the graph names | smaller (no cell copies) | 5 |
| a, as run maps (4) | the core 0.001 ms (0.001 ms), measured in 3A; with the floor under 16 ms | the same | none; only formulas that really change | kilobytes | 8 |

What each changes, in brief:

| | What changes where | The saver | Undo | Main risks and memory |
|---|---|---|---|---|
| b1 | `calcedit.cpp` (candidates, groups), `graph.{h,cpp}` (columns) | none | none | low risk; memory as now |
| b2 | seven structures, each with a pending-shifts list: `sheetmodel` (blocks with offsets), `calculation` (nodes), `valuestore`, `graph`, formula texts, `editsession` marks | must translate through each list as it writes | records still copy cells and texts | seven lazy mechanisms with seven sets of corner cases; a long edit list slows every read until it is compacted, which is a rebase; hostile: a session of 10,000 edits |
| a, as run maps | `sheetmodel` (cells by id), `calculation` (nodes, registration, dirty questions), `graph`, `valuestore`, `loader` (batches), `refs`/`eval` (references through the map), `editstruct`/`editsession` (records, marks), `axis`/`gridsheet` (view) | replays the edit list once on a map, then looks rows up; same bytes | the inverse map operation and the changed lists; kilobytes | references after deletes (4.4), a sweep of about 30 files, threads; memory: 24 bytes a run, dead cells held until their step is forgotten |
| a, textbook (id table and tree) | the same | the same | the same | the same, plus 12 to 16 MB for every million rows whether edited or not |

The figures for b1, b2, and a are estimates from the profile above: (a) removes every phase that grows, leaving the floor of the view and the repaint; b2 keeps the visit of each candidate formula and the loop that rewrites it (273 and 178 ms of the 1,077 at a million rows).

## 3A. The spike: both designs, measured

The review of this note asked for the core of the recommendation and of its simpler rival to be built and measured before anyone commits weeks to it. Two days went into `tools/spike/rowid_spike.cpp` (measurement only: it is not in the build, and no product file uses it; build line in the file's header). Both designs run on the real `DependencyGraph` (`libs/base/calc/graph.cpp`), with the same workload: rows of numbers, a shared-formula column that adds each row up (reads its own row), a column that reads the row above (a difference), six formulas that read the whole column of sums, a `SUM(A:A)`, and, after the edits, a formula filled down a new column and a block pasted. 100,000 rows by 21 columns and 1,000,000 rows by 5. Both give the same values after 400 random inserts and deletes (the checksum test: `rowid_spike A|B ROWS COLS 400 check`), so they compute the same thing.

- **A, the recommendation.** `LineMap` for rows and for columns (runs, running counts, dead records with their neighbors); cells held by row id and column id in blocks of 64 ids; formulas in the id frame (a relative part is an id distance, so the sum formula reads its own row by id, the difference reads the id before it, the totals read ids at both ends and are resolved to positions at each calculation, with a cache of position range to id runs per epoch); the graph holds ids and is never shifted. Undo is the inverse map operation.
- **B, the rival.** Formulas stored relative to their own cell with the absolute parts apart (so a rigid move rewrites no text; the absolute parts are rewritten once per family, and the one formula that straddles an edit is given the row it read, as an exception); a block-offset row map for the cell store only (blocks of 64 to 128 rows, a Fenwick tree over their row counts, and a token for each row so that formula nodes and values do not move when a block does); positions everywhere else, and the graph's existing `shiftRows` pass. B has no column machinery (the cell store is by column position and the graph has no column pass), so a column edit loops over every cell and tells every formula to the graph again; "B with a column pass" below is an estimate, not a measurement.

What the spike leaves out, so its numbers are floors and not forecasts: the sparse lists (merged ranges, rules, names, tables, charts, notes), the session's marks, the view and the repaint (about 8 ms at 100,000 rows today, which both designs still pay), the undo records of those lists, the saver, and the full evaluator (three kinds of formula, a whole-column read, and an absolute range). Column ids that the graph's tree (16,384 columns) can hold were used for A's new columns (the real change widens the tree to 65,536 leaves). The machine was otherwise idle.

Window thread, milliseconds, edit / undo (the map, the cell store, and the graph work; one row or column at a time):

| Edit (core only) | A 100k | B 100k | A 1M | B 1M |
|---|--:|--:|--:|--:|
| insert row top | 0.0012 / 0.0006 | 5.9 / 5.2 | 0.0013 / 0.0005 | 63.1 / 50.6 |
| delete row top | 0.0006 / 0.0007 | 9.9 / 7.0 | 0.0005 / 0.0003 | 80.4 / 77.1 |
| insert row middle | 0.0008 / 0.0005 | 4.7 / 6.3 | 0.0007 / 0.0004 | 46.8 / 67.9 |
| delete row middle | 0.0005 / 0.0003 | 5.5 / 6.2 | 0.0005 / 0.0002 | 57.1 / 61.7 |
| insert row end | 0.0006 / 0.0004 | 4.6 / 4.5 | 0.0006 / 0.0003 | 42.8 / 45.8 |
| delete 10% of rows (middle) | 0.0008 / 0.0008 | 12.2 / 39.7 | 0.0008 / 0.0016 | 78.4 / 526.4 |
| insert column near left | 0.0010 / 0.0007 | 131.8 / 90.8 | 0.0010 / 0.0021 | 1166.0 / 1074.3 |

Steady state (a state of the sheet after random single-row inserts and deletes; the same machine and the same sheets):

| Steady state | A 100k | B 100k | A 1M | B 1M |
|---|--:|--:|--:|--:|
| after 1 edits: full recalculation (ms) | 15.6 | 11.6 | 57.2 | 51.2 |
| after 1 edits: paint of 50x30 cells (ms) | 0.041 | 0.014 | 0.065 | 0.009 |
| after 1 edits: 1,000 formula-bar reads (ms) | 0.02 | 0.04 | 0.05 | 0.14 |
| after 1 edits: Find over every formula (ms) | 2 | 1 | 23 | 18 |
| after 1 edits: resident (MB) | 75 | 102 | 298 | 445 |
| after 1000 edits: full recalculation (ms) | 17.5 | 12.8 | 62.8 | 59.1 |
| after 1000 edits: paint of 50x30 cells (ms) | 0.045 | 0.013 | 0.067 | 0.009 |
| after 1000 edits: 1,000 formula-bar reads (ms) | 0.10 | 0.04 | 0.13 | 0.13 |
| after 1000 edits: Find over every formula (ms) | 5 | 1 | 46 | 18 |
| after 1000 edits: resident (MB) | 75 | 95 | 298 | 445 |
| edit median / 99th after 1 (ms) | 0.0008 / 0.0008 | 5.2157 / 5.2157 | 0.0010 / 0.0010 | 55.6027 / 55.6027 |
| edit median / 99th after 1000 (ms) | 0.0618 / 0.1859 | 4.9306 / 6.1690 | 0.0595 / 0.1809 | 54.7131 / 64.9878 |
| at rest: resident (MB) / recalculation (ms) | 75 / 14.2 | 78 / 11.9 | 298 / 49.8 | 321 / 47.5 |
| fill 1 formula column (ms) | 84 | 65 | 509 | 652 |
| paste 100,000 x 10 values (ms) | 49 | 33 | 22 | 13 |

Long session at 100,000 rows (10,000 random edits):

| Long session at 100k | A | B |
|---|--:|--:|
| after 10000 edits: full recalculation (ms) | 24.4 | 16.4 |
| after 10000 edits: paint (ms) | 0.047 | 0.015 |
| after 10000 edits: Find (ms) | 10 | 1 |
| after 10000 edits: map entries | 14050 | 1716 |
| edit median / 99th over the 10,000 (ms) | 0.9312 / 2.6411 | 5.8901 / 7.6852 |

Reading the spike:

1. **A makes the edit itself free, and B cannot at a million rows.** A's row edits are a microsecond at both sizes (the map has a few hundred runs; after 1,000 random edits 1,500 runs and 60 microseconds, 0.19 ms at the 99th). B is 4.6 to 9.9 ms at 100,000 rows and 43 to 80 ms at a million rows, and the time is the graph's pass over its entries (4 million of them here; `shiftRows` visits every one); the cell store's Fenwick tree costs well under a millisecond. The bar is 16 ms for the whole edit with the view's floor still to add, so **B reaches it for none of the million-row edits and, once the floor is added, is at or over it at 100,000 rows; A leaves the whole 16 ms for everything else** (the sparse lists, the session, the view, and the repaint, about 10 ms together today).
2. **Columns are where B loses most.** 91 to 132 ms at 100,000 rows and 1.1 to 1.2 s at a million for one column (edit and undo), because the graph has no column pass and B re-tells every formula (a column pass added to the graph would remove most of that: the cell loop that remains is about 15 ms at 100,000 rows and 50 ms at a million in the product today, so a B with a column pass would be near its row figures, still over the bar at a million). A's column insert is the same microsecond as a row.
3. **Undo of a delete is cheap in A and not in B.** Deleting 10% of the rows (100,000 at a million rows): A 0.001 ms and back in 0.002 ms; B 78 ms and 526 ms (it has to put the rows back one at a time and tell the formulas in them to the graph again, because the pass dropped their intervals).
4. **Steady state costs A a little and B almost nothing.** After 1,000 edits A's recalculation is 23 to 26% slower than at rest, because ranges are resolved through the map (the cache of position range to id runs per epoch is what keeps it there), and after 10,000 edits (14,000 runs) 70% slower and an edit is 0.9 ms (the spike rebuilds the map's index at every edit; an incremental update is the fix and is in round 1). B stays flat. Paint, formula-bar reads, and Find over every formula are small in both: a formula-bar read is 0.1 to 0.2 microseconds in A.
5. **Memory favors A a little.** At rest 75 MB against 78 MB at 100,000 rows and 298 against 321 MB at a million; after edits B grows (95 to 102 MB at 100,000 rows, 445 MB at a million: blocks that split keep their slack, and every row has a token and a location, 16 bytes) while A stays at its resting size, since an edit adds a run, not data.
6. **Fill and paste cost the same order in both** (84 against 65 ms at 100,000 rows, 509 against 652 ms at a million for a filled column of formulas; a pasted block of 100,000 by 10 is 49 against 33 ms and 22 against 13), so ids do not make filling a column or pasting a block slower in a way a person would see; A pays a position-to-id step per row (about 20 nanoseconds with the hint).
7. **The worker's questions (A).** The question after an insert is a few milliseconds (4 ms at 100,000 rows, for the readers of the rows around the gap; most of them are the whole-column formulas). A column inserted inside a range that every row-sum reads would, asked naively, return every one of those 1,000,000 formulas (23 ms to ask): that is the case for the "blank-insensitive" rule of 4.6 (a sum does not change when an empty column is put in it), without which an insert in the middle of a wide sheet would recalculate every row sum.

**Recommendation after the spike: A.** The graph has to stop being shifted for a million-row edit to fit in 16 ms, and the only way to stop shifting it is to key it by something that does not move, which is a row id. Once rows have ids the run map is lighter and simpler than B's blocks and tokens (no splits that renumber slots, no per-row table, columns for free), and it also gives Undo for nothing. B's idea that is worth keeping is its relative storage of formulas: A's id frame is the same thing (a relative part is a distance in ids), and B's per-family exception for the formula that straddles an edit is how the id frame treats a formula that reads across an edit. No mix is recommended; B is the fallback if A's formula round (5) cannot be finished: B's cell store and relative storage would give 5 to 10 ms at 100,000 rows and 43 to 80 ms at a million, short of the bar there.

What each reaches against the product target (under 16 ms for one insert or delete at a million rows on the window thread): **A: yes**, the core is a microsecond and the rest is the floor (sparse lists, session, view; about 10 ms, with the view's part to be brought down in round 8). **B: no** (43 to 80 ms for rows, 1.1 s for a column, and 0.5 s for the Undo of a large delete).

## 4. Recommendation: ids made from file positions, kept in run maps

### 4.1 The idea

Every row and every column of a sheet has an id for as long as the workbook is open (never saved: files store positions). At load, the id of a row is its position in the file (0 to 1,048,575) and the id of a column is its column number (0 to 16,383). **Ids and positions are equal until the first insert or delete, and a sheet that has none has no map at all** (one flag, "identity"): cells, formulas, graph, and values behave, byte for byte, as they do now. An insert makes new ids (rows from 2^20 up, columns from 2^14 up); a delete takes ids out of the order and keeps a record of them, so Undo puts back the same ids. The order of the ids is a **`LineMap`**: a list of runs, each a first id and a count of consecutive ids, in the order of position, with the running count of live rows before each run. It has one run to start with and two more for each edit.

Everything that is **per cell or per formula** holds ids and never has to move: the cells, the formula nodes, the graph's entries, the window's values, the worker's batches, the session's marks. Everything that is **few** (merged ranges, rules, names, tables, charts, drawings, notes, links, breaks, print areas, arrays) keeps positions and is followed by the code it has now: an edit that costs a millisecond for those is fine, and the code that is correct today stays in use.

A reference in a formula is an id too, so a formula never needs its text written again when a row or column moves (references follow cells, as they do now): the text is read through the map when a position is wanted (to evaluate it, to show it, to save it). **Every formula is held this way**, including those typed or filled into inserted rows and columns, totals rows, and a computed column inserted and filled down (an earlier version of this note kept those as position texts, which would have brought the old cost back for the most common work after an insert; 4.4 says how they are held instead).

### 4.2 `LineMap` (`libs/base/calc/linemap.h`, new)

One for rows and one for columns of each sheet (`Sheet::rows`, `Sheet::cols`). Operations, with their cost in runs R (a few dozen after a session of edits, thousands after a long one):

- `position(id)` and `id(position)`: a binary search over the runs, O(log R); with a one-entry hint for the sequential reads of a recalculation, O(1). Identity: no search.
- `insert(position, count)` makes a run of new ids and splits the run it falls in: O(R) to move entries (a vector of 24-byte entries: 10,000 runs is a 240 KB move, 15 µs).
- `erase(position, count)` takes the ids at those positions out and returns the runs taken, which the caller keeps as the undo record: O(R + runs taken). It also records, for each run taken, the live ids on either side at that time (`before`, `after`).
- `revive(runs)` puts the same ids back in the same place (the neighbors named in the record are alive again because Undo is last in, first out).
- Maintenance is incremental: an edit changes the runs it touches and the running counts after them, and the index by first id is updated in place, not built again (the spike rebuilt it at every edit, which is why its edit grows to 0.9 ms at 14,000 runs). A version number is bumped by every change; a hint kept by a reader (the last run it used) is valid only for the version it saw.
- `isLive(id)`, `walk(position range)` (the runs a range of positions covers, in order, as id ranges: what the cell store's range walk is built on), and `resolveStart(id)`, `resolveEnd(id)`, which answer for an id that is not live (4.4).
- Caps, each refusing the edit with "This sheet has been edited too many times. Save it and open it again.": one million runs; 49,152 columns inserted in a session (a column id is 16 bits); about 15.7 million rows inserted in a session (the session keeps cells in 24 bits of row). All are far beyond anything a person does, and each is tested.

Positions past the last row (1,048,576 rows are live at most) are handled as now: an insert that would push a cell off the sheet is refused; empty rows pushed off simply die.

### 4.3 What is held by id, and what by position

| Held by id | How |
|---|---|
| Cells | `Cell` keeps its 24 bytes; `row` is the row id, `col` the column id (16 bits). Blocks are keyed by row id / 64; cells within a block are in order of (row id, column id). Nothing moves on an insert or a delete. |
| Formula nodes | `Node.row`, `Node.col` are ids. |
| Formula references, defined names, and tables' ends | in the id frame (4.4, 4.14). |
| Graph | row intervals are ids (4.6). The tree over columns grows from 16,384 to 65,536 leaves. |
| Values the window holds | `ValueStore` is keyed by (sheet, row id, column id). |
| Worker batches | items carry ids; the generation and `moveToNow` go. |
| Session marks | the marks and results by cell are keyed by ids; the saver turns them into positions when it is asked. |
| Row sizes and hidden state | `RowSpec.row` becomes an id; the sorted list is walked in the order of the map. |
| **Held by position** | merged ranges; arrays and data tables (`Sheet::ranges`); the filter; validations and conditional formats; defined names; tables; charts; drawings; notes and links; page breaks, print area and titles; column runs (at most 16,384); the view (selection, scroll, frozen panes). |

An id-or-position confusion would be silent (an id is a plausible number), so the sweep that changes `Cell` renames its fields (`row` to `rowId`, `col` to `colId`) and gives the cell store a positional interface in which every callback is handed the position it is called for; the compiler then lists every reader (about 30 files, 160 calls into the cell store). The `LineMap` interface takes and returns small wrapper types (`RowId`, `RowPos`), not bare integers.

### 4.4 Formulas: the id frame

A formula is a text plus, for each cell, `dRow` and `dCol`: the cell's distance from the cell the text was written for; a reference without `$` moves with the cell (`RefLit` in `calc/formula.h`). Today distances and numbers are rows and columns. **They become ids.** For everything read from a file the two are the same numbers, so loaded formulas, shared families, compiled programs, and the sheet's texts need no change. Resolving a reference to what it reads is the one new step: the numbers `resolveReferences` already computes are ids; the rows and columns of the area are `position(id)`. A range needs only its two ends; a row inserted inside a range is inside it by order, with no change to the reference. A relative reference is an id distance, so it follows the cell it named, as a rewritten text does today.

**Texts hold large ids.** An inserted row's id is above 1,048,575, which an A1 text cannot hold. The text stored in `Sheet::formulaTexts` therefore uses the same grammar with wider limits: rows to 2^31 - 1 and columns to 65,535 (column letters to `CRXO`). The parser takes a `ParseLimits` argument (the grid's limits for what a person types, the wide ones for stored texts), as do `shiftFormula` and the rewriter. This is a contained change (one parameter through the tokenizer and the printer) and it is what lets a formula typed into an inserted row, a totals row under a table that grew, or `=A5` pointing at an inserted row be held by id like every other. The spike held such references as plain numbers in a structure; the stored form is the text only because the sheet's formula cache, the shared families, and the saver already work on texts. What stays in positions is small and named: **3-D references** (`Sheet1:Sheet3!A5` names the same cell on every sheet, and an edit on one sheet must not change that: the formula keeps a position text, is listed per sheet, and is rewritten as today, which costs the number of such formulas), and the cells' formulas of **array formulas and data tables** (their ranges, in `Sheet::ranges`, are positions; the formula text is in the id frame like any other).

**Whole rows and whole columns** (`$A:$A`, `5:5`, `A:C`) are flags in `RefLit` (`kRefWholeCols`, `kRefWholeRows`), not ids. They resolve at every read to the first and the last live position, and **never go through the rules for dead ids**. The graph registers them as an interval to the largest id, so rows with ids above the original ones are inside. An insert at position 0, at the last original row, or past it falls inside `SUM(A:A)` by construction; the model check has these three cases by name.

**Dead references.** Rules for a reference whose id is no longer live (a row was deleted), which keep the answers the common programs give:

- A single cell whose id is dead is `#REF!`.
- A range's start that is dead resolves to the live row that followed the deleted rows **when they were deleted** (the `after` the `LineMap` recorded), its end to the live row that preceded them (`before`). If that row was itself deleted later, the answer is looked up again, and so on. Rows inserted afterwards next to the deleted ones are therefore outside the range, as in the programs people know. If the start comes after the end, or neither is live, the range is `#REF!`.
- Undo brings the same ids back and every reference reads them again; nothing is restored for formulas.

*The invariant, and the order of Undo and Redo.* (1) An id is made once and is live or dead. (2) A dead record is written when its ids die and removed only by the revival of the same step; a step stores the map's version as it was just after the kill and revives only if the version is the same, so a revival out of order does not happen: the step is dropped instead and Undo says so (this cannot arise in last-in-first-out order). (3) `before` and `after` were live when the record was written, and each hop of a chain goes to an id that is live or died later than the one it came from, so a chain ends. (4) Records are never edited, so what a reference resolves to depends on the order of kills and revivals, never on what was inserted next to the dead rows afterwards. Undo and Redo are last in, first out; Redo after Undo kills the same ids again and writes the same record; trimming old steps (the byte limit, "Undo stops at the last delete saved") never revives anything and leaves the records where they are; a new edit after Undo drops the redo steps, whose ids stay dead (the ids of a dropped *insert* go back to the counter, 4.16). The sequence in the model check, with a formula `=SUM(A2:A5)` over rows with ids a, b, c, d: delete a (start resolves to b), delete b (start follows to c), Undo (b is live: start is b), Redo (a dies again with the same record), Undo twice, then redo both and insert a row between the live neighbors of the deleted rows: the new row is outside the range (start is c, the new row is before it).

*Positional and id formulas agree.* The eager engine of today (rewrite the text, re-tell the graph) stays in the code, behind a per-workbook switch, until the last round; it is the oracle. The model check feeds the same typed text and the same edits to a workbook in each mode and requires equal values, equal formula text as shown, and equal saved files; 3-D formulas, which are positional in both, are in the same run.

Writing a text for a person or a file (the formula bar, Find in formulas, a saved cell): `Sheet::formulaText` turns ids into positions through the maps (a single pass over the text's references, the existing rewriter with a different mapping; identity: the text as it is). Typing, pasting, and filling go the other way: the text the person writes is in positions, and is given ids with the same pass. A shared family made by a fill is split where the rows or columns it covers or reads are not consecutive ids (a run boundary), so each part still has one text and a distance.

### 4.5 Reading cells

`CellStore::find(row, col)` takes positions, as now, and maps them (`id(position)` twice; a one-entry hint on each side). Its range walks (`forEachInRange`, `forEach`, `forEachRowTail`) take the `LineMap`'s runs for the rows and, inside a row, for the columns: the cells of a block are in order of id, and for the usual map of a few runs the walk visits each run's cells in turn, which is also position order. A callback gets the position beside the cell. Cells of dead rows and columns stay where they are (they are the undo record of a delete) and are never reached by a walk, because the walk follows the live runs; for the cells of dead columns inside live rows, a 65,536-bit table says which column ids are live. `cellCount()` is kept for the live cells.

A delete is therefore: take the ids out of the map, count and note the cells for the statistics (a walk of the deleted rows' blocks: time in proportion to what was deleted, not to the sheet), and keep nothing else. When the step falls off the undo stack, the dead cells are let go of in the idle time, in slices.

### 4.6 The graph, and what is worked out again

The graph keeps its structure (sorted runs in nodes of a tree over columns, now 65,536 leaves) and holds ids. Two rules make the intervals right without any rewriting:

- Ids of original rows and columns are in the order of position, so an interval of ids covers exactly the original rows between its ends, however many rows were inserted or deleted among them.
- **Inserted rows are found by the gap they sit in.** A change in a cell of an inserted row is asked as "the original rows just before and after it": the intervals that overlap that pair. A reference whose end is an inserted row is told to the graph as the original row at that end, the interval a little too wide. Both err in one direction only (a formula is worked out when it need not be, never the other way). A whole row or column is told as an interval to the largest id.

`DependencyGraph::shiftRows` and the column fallback are removed. After an edit the worker is asked, once (so the window thread does not wait for a result of 100,000 formulas), for four sets, and works out only those:

1. **Shape-sensitive formulas whose range holds the gap.** A formula is *blank-insensitive* if none of its calls can notice an empty row or column put inside a range it reads: `SUM`, `SUMIF` and `SUMIFS` with ranges of one shape, `MIN`, `MAX`, `COUNT`, `COUNTA`, `AVERAGE`, `PRODUCT`, and the like, on whole ranges. Everything else is *shape-sensitive* (`COUNTBLANK`, `ROWS`, `COLUMNS`, `INDEX` and `MATCH` with a position, lookups, array arithmetic, a range given to a function that is not on the list). The default is sensitive, so a function nobody classified is worked out again, never skipped. A row-sum over a range into which a column is inserted is blank-insensitive and is not asked again: that is what keeps a column inserted inside 1,000,000 row-sums from recalculating them all (the spike's naive question returned all of them).
2. **Formulas that observe a position.** The trait is wider than "reads its own place": a formula *observes position* when any call can see the position of its own cell or of an argument's cell or range: `ROW` and `COLUMN` with or without an argument (`ROW(B5)`, `COLUMN()`, `SUMPRODUCT(ROW(A1:A10))`), `ADDRESS`, `CELL`, `AREAS`, `OFFSET`, `INDIRECT`, and any formula that uses a defined name or a table that contains one. The program is scanned for these calls at every nesting depth when it is compiled. Each such node keeps, in a per-sheet list, the positions it last saw (its own, and the first position of each area it passed to such a call); after an edit the worker compares them with the positions now and works out the nodes where they differ. The list is the size of the formulas that have the trait, not of the sheet.
3. **Formulas that read the deleted ids** from outside them (a question to the graph for the readers of the deleted runs; formulas that are themselves in dead rows are skipped).
4. **Formulas that are kept apart or read where they are told** (as `m_special` now).

Rigid moves (the formula and what it reads both after the edit, or both before, and no call that observes position) ask for nothing.

*Results of an older shape are not used.* Each sheet has a structural epoch, increased by every edit that is not rigid for some formula. A worker result carries the epoch it was computed under; a node dirtied by the questions above is stamped with the epoch of the edit that dirtied it; the window discards a result whose epoch is older than its node's stamp, and the node is worked out again. A result computed before a rigid edit is still good (its ids still name the same cells) and is not discarded.

### 4.7 Undo

A structural step holds the `LineMap` change (the runs inserted, or the runs taken out with their neighbors), the before-images of the position-keyed lists that the edit changed (and only those: a copy made when an edit actually changes it, not always), the positional formulas' old texts, and the notes' and links' delta. No cell is copied and no formula text is saved for the others. Undo is the inverse `LineMap` operation and the lists put back: time in proportion to what the step touched, and the record is kilobytes. The "This can't be undone" question for rows and columns goes away, which is a visible change; the question stays for the edits that still make large records (cut and paste of huge blocks, shifting cells).

*Dead cells and the nodes in them.* The nodes of formulas in dead rows stay registered in the graph and keep their values, flagged dead: the worker does not work them out and the questions skip them, so a delete of 500,000 rows does no work per node on the window thread. Its cost is the map operation, a walk of the 8,000 blocks of 64 rows to count what was deleted (a few milliseconds), and one question to the worker for the readers outside the rows (the spike asks 100,000 deleted rows in 4.8 ms at a million rows; 500,000 is five times that, on the worker). **When the rows are revived** by Undo, their formulas are marked dirty in one worker job (their values were kept, so the grid shows them at once; while the rows were dead the cells they read may have changed, so they are worked out again; a later refinement compares the epoch they were last worked out under). The nodes and graph entries of dead rows are let go of when their step is forgotten, in idle slices, with the cells.

### 4.8 Saving

Files store positions, so nothing about ids is saved, and **the saver must write the new text of every formula that moved**: a formula's text in the file has positions, and after a row is inserted at the top every formula whose references are at or after the edit has a different text there. Today the model lists the formulas it rewrote and the saver writes those whole; with ids nothing is listed, so the saver does the rewriting from the old file's text and the final maps.

- The saver replays the session's list of edits once on a fresh `LineMap` per sheet (the list says which sheet, where, and how many) and then asks it for each row of the old file: `position(id = the file's row)`; a row that is not live is gone. Cells in the old file are renumbered by the same two lookups. That replaces the loop over the whole list for every row and cell, with the same arithmetic, so the same numbers.
- **A family-level rewriter** (`TextRewriter` with a mapping from the maps) gives each *distinct* formula text its new text once: a shared master, or a formula that is alone. A family whose members all move alike keeps its shared form with a new master text and a new `ref` (its members stay `<f t="shared" si="n"/>`); a family cut by an edit is split at the cut, each part given a master; a formula alone is rewritten alone, at about a microsecond. For 1,000,000 single formulas that is a second at most, inside the 2 to 3 microseconds a cell that a save takes today (1.6); for a sheet of shared families it is the families.
- The cells the person changed are written whole as now. The part of a file for a sheet with no structural edit is not read.
- The earlier stretch goal (a save under 1 s) is dropped: the 2 to 3 microseconds a cell are copies of bytes and strings per part of a cell, which is a separate improvement (write the renumbered row and cell in place) and is not needed by this work. The gate is: no slower than now after 1, 20, and 200 edits, and the same bytes for sheets with no structural edit. The old saver stays in the tree, selected by the same switch as the formulas, until the last round, and the new one is compared with it on the dev list's edit script.

### 4.9 Threads and the window

The `LineMap` changes only on the window's thread, under the hold, as every other model change does; readers (the window drawing, the worker evaluating) read it without a lock, as they read the cells. `WorkbookLoader::hold()` makes the worker leave the formula it is on before it returns (the evaluator looks at a flag as it steps), so no reader is inside a map read while the map changes; a reader's hint is valid only for the map version it saw, so a hint left from before an edit is never used after it. A test makes edits through a hold hook while the worker is inside a long `SUM`, and fails if a read sees a map between versions. After an edit: the grid's row sizes (`Axis`) change in place instead of the view being built again (the sizes in the view are positions; an insert at p shifts the entries after it, a memory move), the selection and scroll position follow, and the viewport repaints. The worker gets the questions of 4.6 and works out only what they name; its results carry the sheet's structural epoch and are dropped if the epoch of the edit that dirtied their node is newer; the grid shows every value it has at once, in its new place, and a value that is being worked out again shows the old one until the worker has it, as for any edit.

### 4.10 What does not use ids

Inserting or deleting cells (shifting the rest of some columns or rows) and cut-and-paste move some cells, not whole lines. They keep their code (`reshuffle`, `applyMove`) and the position-based rewriting of the formulas that read the cells they move, through the cell store's positional interface. A formula the rewrite touches is written back in the id frame by the pass that gives a typed text its ids. Their cost is what they move, as now, and the "big step" question remains for them.

### 4.11 Memory

`Cell` stays 24 bytes, `Node` 32, a graph interval 16, `Formula` 16. A `LineMap` is 24 bytes per run, two runs per edit: 100 edits is under 5 KB. The graph's tree over 65,536 columns is made by use as now. The undo record shrinks from about 320 bytes per cell deleted and formula rewritten to a few runs. Dead cells stay in the store until their step is forgotten, at the 24 bytes each already has, against about 320 bytes a cell in the record today. The sheet at rest is unchanged, the target is no more than 10% (expected 0%).

### 4.12 Hostile inputs and limits

A file cannot make a `LineMap` (maps are made by edits, not read); only a long session can. The caps in 4.2 stop it; the idle purge of dead cells runs in slices with the work counted, not timed. An edit's work beyond the window thread is the worker's, which looks at a stop flag as it steps. Rebasing the ids to positions (so the run list returns to one entry) is the relief valve for a session that reaches a cap; it is built, capped, in the last round (4.16).

### 4.13 Targets

| | Today | Target |
|---|--:|--:|
| One row or column inserted or deleted anywhere in 1,000,000 rows (window thread, processor time, with the repaint) | 1.1 to 11.8 s | **under 16 ms at the 99th percentile; the model and session part under 3 ms** |
| Counted work for that edit | 6,000,000 | no more than the formulas that really change (rigid moves: under 100, whatever the sheet's size, tested at two sizes) |
| Undo and Redo of it | as long as the edit, 1.5 GB held for twelve | the same as the edit; the record in kilobytes |
| A delete of 500,000 rows | not measured (one row costs 1.9 to 4.3 s at a million rows) | under 50 ms (time in proportion to the blocks of cells deleted) |
| The worker after a rigid edit | 0.1 to 6 s | under 20 ms, then only what the questions of 4.6 name |
| Saving after a structural edit | 4.7 s at 100,000 rows; 19.7 s at a million | not slower after 1, 20, or 200 edits; sheets with none: the same bytes and the same time |
| Resident memory after opening | 465 MB (1,000,000 rows by 5) | not above +10% (expected: the same) |
| Resident memory after 12 structural edits (1,000,000 rows) | +323% (465 to 1,969 MB) | under +5% |
| Typing a number in a cell (the 4b-1 yardstick) | 4.3 ms middle, 6.1 ms 99th | not slower |

### 4.14 Names and tables

Defined names (and with them the print area and titles) are formulas without a cell: they are held in the id frame like any other, parsed once to ids, evaluated through the maps. An edit therefore does not rewrite a name's text (the O(names) pass of today goes for row and column edits), and the formulas that use a name are not told to the graph again when a name's cells only move. Their position text, for the Names dialog and the saver, is made when asked. Tables keep the ids of their first and last row and of their columns, and the header row; the structured references (`Sales[Amount]`) resolve from those, through the maps, at registration and at evaluation. A row inserted in a table, a table that moves, and a table that grows by typing below it change positions and not ids, so nothing that names the table is registered again; only an edit that deletes the table's first or last row (or its columns) changes what a table stands for. **What is registered again is counted by name and by table**, not per formula: `editStats()` reports "formulas registered again because a name changed", "... because a table changed", and the test asserts each is at most the number of users of the names and tables whose ids changed (zero for an insert or a delete that touches neither end). The bench grows two cases for this: 100,000 formulas that name one defined name over the data, and 100,000 structured references into a table of 1,000 rows, an insert above and one inside each.

### 4.15 Fill, copy, paste, cut, shift cells, and sort

- **Fill and paste of whole columns** go through the cell store's positional interface (`put` by position), which is a position-to-id step per row (a hint makes it about 20 nanoseconds). Measured in the spike after 1,000 edits: a formula filled down 100,000 rows 84 ms (65 ms in the rival), 1,000,000 rows 509 ms (652); a block of 100,000 by 10 values 49 ms (33) at 100,000 rows. Filling splits a shared family where the rows are not consecutive ids (a run boundary), at most one family per run the column crosses. Round 5 repeats the measurement on the real cell store.
- **Copy, paste, and cut-and-paste of a block** (`applyMove`) and **inserting or deleting cells** (shifting the rest of some columns or rows) move cells between rows. They keep their code and its position-based rewriting of the formulas that read the cells they move, reached through the positional interface and the pass that turns a text into ids.
- **Sort** moves cell contents among rows; the rows (their ids, heights, hidden state, outline levels) stay where they are, as the common programs keep a row's height with its place. A formula that moves with its cell is re-based as a copy would be: when the old row, the new row, and everything the formula reads are in one run (the usual case), its `dRow` changes by the distance and nothing else; otherwise it goes through the text pass (positions, shift, ids). Notes and links move with the cell through `CellAnchors::permute`, which is by position and is unchanged. Sort is in the model check, with formulas that read their own row, the row above, and a fixed cell.

### 4.16 Ids that run out, and rebase

Ids are never reused while anything can name them. An insert takes the next ids; its Undo kills them (Redo revives the same ones, so Undo and Redo use no ids); a new edit after Undo drops the redo steps, and when a dropped insert step holds the highest ids ever taken, the counter goes back to where it was before that step (nothing can name those ids: whatever referred to them was undone first). A session that inserts and undoes in a loop therefore does not use ids up. Columns have 49,152 spare ids and rows 2^24 - 2^20; at 80% of either, or at 100,000 runs, the sheet is **rebased** in the last round: ids are renumbered to positions in one pass over every structure that holds them (cells, nodes, graph, values, id-frame texts, marks, records), with the window held, with a status message, and with the undo history dropped (the person is asked first, as for any step Undo cannot hold). The pass costs about what one edit costs today (0.2 to 1.2 s at 100,000 rows, 1 to 8 s at a million), is rare by construction, and is capped: it is refused with a message if the sheet is still over a cap after a rebase. Its test runs it after random edits on a small sheet and compares every observable with the sheet before.

### 4.17 Long sessions and walks over many runs

A range read (`SUM` over a column, a paint, Find, a copy) is walked as id runs. The walk of a position range is cached per sheet and per version of the map (the cache of the spike): the same range read by 1,000,000 formulas is turned into runs once. After 10,000 edits (14,000 runs) the spike's recalculation was 70% over its resting time and an edit 0.9 ms (the full rebuild of the index at each edit, which is replaced by an incremental update); the test for round 1 and round 4 runs a session of 20,000 runs and asserts counted steps (comparisons per position lookup, runs visited per range walk), not time.

## 5. The build plan

Rounds of one or two days for one helper, re-estimated after the review and the spike. **Each round ships alone**: rounds that change the meaning of data are gated by a per-workbook switch (`idFrame`, a constant in `CalcOptions` that tests flip), off until the round that completes the path and removed in the last round; the other rounds are correct with the switch in either place. Each has a number it moves (the bench, or a counter that a test asserts), and its tests are written first. Counters are asserted, never time ("count work, never time it"); the bench's times are for the report. Estimates in the last column are for the window thread at 100,000 rows by 21 columns, plain, a row inserted at the top (0.17 s today); at a million rows they are about ten times larger until the end of round 5.

| Round | Days | What | Ships as | Moves |
|---|--:|---|---|---|
| 0 | 3 | The harness: the stronger model check, golden digests, counters, bench | tests and tools; counters are a few lines | nothing; the baseline and the safety net |
| 1 | 2 | `LineMap`: incremental, versioned, dead records, caches | a library no one calls yet | microseconds per operation; 20,000 runs |
| 2a | 4 | The cell store's positional interface; `Cell` fields renamed | no behavior change | nothing visible; byte-identical saves |
| 2b | 2.5 | Cells held by id (needs 2a) | on; ids equal positions until an edit; the eager rewrite still runs for formulas | cells moved: 2,100,000 to 0; 174 to about 140 ms |
| 3 | 3 | Nodes, values, worker batches, and session marks by id (needs 2b); structural epoch | on; the eager rewrite still runs | about 100 ms; values and session phases to 0 |
| 4 | 4 | The graph by id (65,536 columns, whole rows and columns to the largest id, gap and dead-id questions, blank-insensitive and position-observing traits, results by epoch) | on; the formulas still rewritten register their ids | about 75 ms; columns reach rows' cost (600 to 75); the worker's settle to what the questions name |
| 5a | 3.5 | Formulas in the id frame: wide parser limits, text to ids and back, families split by run, `idFrame` off | dark behind `idFrame` | nothing until on |
| 5b | 3 | Names and tables in ids; dead references; 3-D references positional; whole rows and columns (needs 5a) | dark | nothing until on |
| 5c | 2.5 | Sort, paste, cut, and shift cells through the id frame; `idFrame` turned on after the gate | on | formula visits 100,000 to a handful; about 15 ms; texts made 99,997 to 0 |
| 6 | 3 | Records, Undo and Redo, row sizes by id, the purge of dead cells (needs 5c) | with `idFrame` on | Undo and its record; the "can't be undone" question gone for rows and columns |
| 7 | 4 | The saver: replay once on maps, family-level rewriter (needs 5c) | with `idFrame` on, the old saver kept for comparison | save after an edit no slower than 4.7 s at 100,000 rows |
| 8 | 2.5 | The view in place; epochs and `hold()` verified | on | the floor: about 8 ms to under 5 |
| 9 | 4 | Hardening, capped rebase, the old paths and the switch removed, final numbers | on | the final numbers; the old paths gone |

About 41 days, or 8 weeks of one helper (range: 6 to 10). The first estimate, 21 days, did not have the saver's text rewriting, names and tables, the stronger model check, the id-frame text, rebase, or the honest size of the two sweeps. The best stopping points are after round 4 (cells, nodes, values, and the graph by id, formulas still rewritten: columns now as cheap as rows, about 75 ms at 100,000 rows, and a smaller Undo record) and after round 5c (the bar is met for rows and columns; Undo, the saver, and the view still to do).

### Round 0: the harness (3 days)

Tests first, and nothing else changes.

- **A stronger model check.** `tst_crunch_structmodel.cpp` has a naive reference model: a map from (sheet, row, column) to a constant or a formula whose references are absolute positions, with the things an edit must carry: merged ranges, a conditional format and a validation (ranges and formulas), defined names (cells, rows, a table column), a table, a chart series, a drawing anchor, a note, a link, a page break, the print area, row heights, column widths, and a filter. The model implements insert and delete of rows and columns by renumbering every item, in its own code (it shares nothing with `adjustRange`). **Two sheets**, formulas that read across them, **3-D references**, **structured references**, `OFFSET` and `INDIRECT`, `ROW(range)` inside `SUMPRODUCT`, `COLUMN()`, `ADDRESS`, whole-column and whole-row references (with inserts at position 0, at the last original row, and past it), and **shared families made by reopening a saved file**. A seeded generator makes steps of: insert and delete of 1 to 3 rows or columns, cell edits, **fill, copy and paste, sort, cut and paste (Move), insert and delete of cells shifting the rest**, Undo and Redo (the sequence delete A, delete its neighbor, Undo, Redo, insert between is a named case), and a save and reopen.
- **After every step**, on a small sheet (40 by 12 cells, two sheets): the workbook is saved to memory, opened again, and **recalculated from scratch**, and every value must equal the incremental one (an independent oracle: the reader and the calculation that do not use the edit code); the displayed text of every formula, every value, and each kind above must equal the model's; and **a graph-superset check**: for every formula, the set of cells it actually read is inside what the graph holds for it, and the dependents the graph gives for each cell include every formula that read it (the existing "ask every formula" test, run after each step).
- **The two modes.** Once rounds 5 and later exist, the same typed text and the same steps are run with `idFrame` off (the eager engine) and on, and the values, shown text, and saved files must be equal.
- **Golden digests** from today's engine: the same steps on the unchanged code, a 64-bit digest of the observable state after each step, in the repository (`tests/crunch/structgolden.inc`, a few kilobytes). Where a later round means to differ (a dead-reference edge case), the file changes in a commit that says why.
- **Counters** (`Calculation::editStats()`): cells moved or touched, formulas visited, formulas rewritten, texts made, graph entries touched, values moved, session entries followed, formulas registered again because a name or a table changed, formulas asked by the worker's questions (by set), and the same for Undo. A few lines of product code, so tests can assert on them.
- The bench and the generator of this note, plus the two cases for names and tables (4.14), and the steady-state benches of the spike on the real engine: recalculation, paint, Find, and formula-bar reads after 1 and 1,000 edits.
- The rule for later rounds: a round's tests assert its counters at two sheet sizes (1,000 and 100,000 rows) and require them equal; every test breaks the code once to see it fail. Budget: 3,000 steps with the oracle for the normal suite (under 30 s), 100,000 for the nightly.

**As built (October 26, 2026).** The check is `tests/crunch/structmodel.h` and `.cpp` (the workbook it begins with is `structmodel_book.inc`: three sheets of about 40 rows and 12 columns with a formula of every kind of reference, shared families, a table, a chart, notes, links, merged cells, rules, names, a print area, breaks, and a filter), run by `CrunchStructModelTests` (about 20 s) and by `jcrunchtool model-check <seeds> <steps> [first seed] [every=n] [nograph] [noshrink]` for as long as wanted (`model-replay <file>` runs a failure's steps again, `model-dump`, `model-book`, and `model-golden` print the state, the workbook, and the digests). The steps (24 kinds: rows, columns, and cells inserted and deleted, typing of formulas of every kind, fill, copy and paste, cut and paste, sort, Undo and Redo, merge, sizes, notes, links, names, page setup, a save to memory read again, a real save) come from a generator of its own, so a seed is the same on every machine. After every step: the window's values are the calculation's; a snapshot of the save is read again in a window of its own, every formula is worked out again, and the values, the formula text, and each thing that follows the cells must equal the session's; every formula is worked out again over a source that writes down what it reads, and the graph must hold each of those reads; a step that was refused changed nothing; and, after a plain insert or delete of rows or columns, a second opinion of the state (ranges renumbered, references in formula text found and renumbered by code that shares nothing with the engine's) must equal the session's. A failure prints the seed and the fewest steps (dropped while it still fails). The digests of 8 seeds of 40 steps are in `tests/crunch/structmodel_golden.txt` (and the digest of the parts a save writes after them). The counters are `Calculation::editStats`, `EditSession::marksFollowed`, `ValueStore::relocated`, and `GridSheet::viewCellsRebuiltForTest`; `edit-struct-bench` prints them after its old columns.

Six defects of today's engine were found by the first runs and fixed (each with a test): the links of a sheet lose their address when an edit moves none of them; a formula that is `ROW()` or `COLUMN()` keeps its old value after an edit moves it; Undo of an edit that wrote a shared family again puts the family's text in cells that had been deleted and put back, without their offsets; a link on one sheet whose place is on another (`Calc!A1`) was not followed by the session (only by the saver); a page break before the first row or column was kept and not saved (now refused); and the saver did not move with a sort a link on a block of cells that an edit had left one cell.

### Round 1: `LineMap` (2 days)

`libs/base/calc/linemap.h` with `RowId`, `RowPos`, `ColId`, `ColPos` wrapper types and the operations of 4.2, maintained incrementally, with versions and hints. Tests first (`tests/sheet/tst_sheet_linemap.cpp`): a property test against a naive `std::vector<int>` of ids (random insert, erase, revive in last-in-first-out order, then every id's position and every position's id, 20,000 steps per seed); the dead-reference answers of 4.4 and the invariant, including the named sequence, against a naive recomputation; a session of 20,000 runs with counted comparisons per lookup; the caps; identity does no allocation; a micro bench of 1,000,000 rows and 10,000 edits. Gate: microseconds per operation, 24 bytes per run.

### Round 2a: the cell store's positional interface (4 days)

No behavior change. `CellStore` gets the interface that does not expose positions in `Cell`: `find`, `findMutable`, `set`, `erase`, and the walks give the position with the cell (`f(row, col, cell)` or a view); `Cell` keeps its fields but renamed `rowId`/`colId`, still equal to positions. The compiler lists the readers (about 300 mentions of `Cell.row` and `Cell.col`, 160 calls into the store, 30 files, including importers, exporters, the grid's painter and conditional formats, find, clipboard, sort, tables, filter, and the tools); each is read and changed to take the position from the call. The honest size is 3 to 4 days for one helper: the edits are mechanical and the review of each is not, and the suites of three libraries must be green at the end. It can be split by directory between two people. Gate: the suite, `jcrunchtool roundtrip` on the dev list byte-identical to before, the grid sweep.

### Round 2b: cells held by id (2.5 days)

`Sheet::rows`, `Sheet::cols` (`LineMap`s, identity); the cell store maps positions to ids on the way in and out; `shiftRows`, `shiftColumns` become `LineMap` operations plus the (still eager) update of node positions, record, and callbacks for formulas; dead rows and columns keep their cells; walks follow the live runs. Tests first: the model check passes with counters showing cells moved = 0; a delete followed by Undo leaves `cellCount` and every cell as before; a walk across five runs visits the cells in position order; sort, fill, paste, and sheet copy on a sheet that has had structural edits. Gate: 174 to about 140 ms for the row at the top.

**As built (October 27, 2026).** On, not dark: ids equal places until the first edit of a sheet's rows or columns, and the sheet then walks the live runs. The two `LineMap`s are the cell store's (`CellStore::rows()`, `cols()`, and `Sheet::rows()` and `cols()` that give them), so that a copy of a sheet copies its order with its cells. Calls take places and turn them into ids; every walk gives the place beside the cell. A walk of a sheet with no edit is the old code; after the first edit it goes along the runs of the rows and, inside a row, the runs of the columns (`CellStore::spans`, `walkRows`, `visitRow`; the walk from the end, in slices, keeps its cursor as a place). `insertLines`, `deleteLines`, `undoLines`, and `redoLines` change the order and move no cell; `formulaCells` finds the formulas and array members that change places (before the edit) and `placeOf` says where each is (after it), which is what the node positions and `movedFormulas` are made from; `Calculation::applyEdit` does the rest as it did. What turned out different from the plan:

- **A delete adds rows (columns) at the end.** The `LineMap` shrinks when lines are erased, and the sheet must stay as long as it was or a cell could not be put in its last row. A delete is an erase and an insert at the end (`LineEdits` holds both); Undo undoes both. An insert so near the end that it does not fit (the `LineMap` refuses `at + count` past the capacity) is the same delete: everything from there on is pushed off.
- **A dead cell holds no formula.** The nodes of the formulas in deleted rows are still dropped at once (round 3 keeps them), so the cell is made a plain value when its row or column dies (`HasFormula` and the other formula flags cleared, `formula` -1). Undo brings the same cells back and then writes the cells of the record over them, as it always did, so the formulas are made again; the record is not smaller yet (round 6).
- **Redo brings back the same rows.** The session gives the record of an undone edit back to `applyEdit`, which asks the order to redo it (`LineMap::redo`) when the record is for the same edit and the order is in the state it left; else it inserts or deletes anew. Undo asks the order to undo (`applyEditImpl`), and falls back to the inverse edit if the step is not the last. Undo and Redo of an edit therefore use no ids.
- **`cellCount()` leaves out dead cells**, which are counted when a row or column dies and when it comes back (a cell in a dead row and a dead column once).
- **Dead cells are not let go of** (the purge is round 6): a session that deletes many rows holds their cells, 24 bytes each, until it is closed.
- **Records are let go of, and the order is compacted (the review of 2b).** Every record of an edit of rows or columns goes to the session's `LineDrops` when its step is dropped (a new edit after Undo, the history trimmed, a step forgotten); the session hands them to `Calculation::forgetLines` the next time it holds the workbook, which purges the cells only that step could bring back, drops the records of those lines, gives the ids back when it can, and makes the order identity again when nothing is left (`LineMapCore::dropDead`, `tryIdentity`). An order near its limits (80% of its ids, `rebaseRuns` runs or dead records, or too few ids for the edit) is rebased before the edit: **every step that edited that sheet's rows (columns) is forgotten, the done ones up to the last such step and the undone ones from the first** (the chosen way of clearing the history; Undo goes on to work for steps made after), then the cells are renumbered to places and the dead ones let go of. A sheet copy is rebased as it is made. New edits are the only ones that take budget: `canApply` no longer checks ids (Undo and Redo of an edit take none), and `applyEdit` checks for a new edit only. An Undo or Redo the order does not take fails with a warning and makes no new rows or columns.
- **A walk from the end keeps a place**, so rows or columns edited between two slices, or the order made one run again, go on from where it was. Walks along runs of columns keep to one pass along each row (a bit for each run, at most 64 runs, else a sort); the runs of a range are the order's cached ones; formula moves are streamed. Cells pushed off the end are recorded in `gone` (a formula in them could not be: the edit is refused first).
- `jcrunchtool edit-walk-bench <file> [row edits] [column edits]`: on 1,000,000 rows by 20 columns of numbers, a full walk and a sliced walk from the end take 53 and 139 ms before and 51 and 131 ms now (no edit), and after 10 column and 1,000 row edits (which took 94 s and 3.4 s) 48 and 128 ms before, 126 and 224 ms now: the order has about 2,000 runs of rows and 21 of columns. Memory after the edits 933 MB before, 820 MB now.

Measured on this machine, `edit-struct-bench` on 100,000 rows by 21 columns, plain, the same load for both (two runs each): insert a row at the top 391 and 402 ms before, 317 and 322 after; delete at the top 343 and 354, 316 and 331; insert in the middle 139 and 161, 118 and 120; at the end 9 and 11, 7. Cells moved 2,100,000 to 0; the counted work of the row at the top 2,200,000 to 100,000. The rest of the top row's time is the 100,000 formulas written again (rounds 3 to 7). A sheet's memory after the twelve edits and their Undo is lower (about 250 MB against 348), and at rest it is the same (192 MB against 193).

### Round 3: nodes, values, batches, and marks by id (3 days; needs 2b)

`Node.row` and `Node.col` are ids; `ValueStore` is keyed by ids; the `CalcBatch` items carry ids and the sheet's structural epoch (the generation, `m_log`, and `moveToNow` are removed); the session's marks and results (`m_touched` and the others) are keyed by ids and turned to positions only when the saver or a command asks; the move callback is gone, `movedFormulas` is gone from the record. The eager rewrite of formulas still runs (positions in the graph are converted at registration). Tests first: a batch in flight when an edit is made lands on the right cell and a result older than its node's stamp is discarded (today's `moveToNow` test, rewritten); touched cells survive 200 random edits and save the same file. Gate: the "values" and "session" phases to 0 (32 ms of 188 at 100,000 rows; 416 of 1,077 at a million).

**As built (October 27, 2026).** On, not dark. A node, a value, a result, and a mark name a cell by the ids of its row and column, which are the ones its `Cell` holds (`rowId`, `colId`); an edit of whole rows or columns changes none of them. What turned out different from the plan:

- **Nothing is told to the nodes and nothing is followed.** The callback that told each moved formula its new place is gone for whole rows and columns (the walk it made, `forEachMoved`, too), `movedFormulas` is gone from the record, `ValueStore::relocate` is not called for them, and `followSet`, `followMap`, and `followLines` do nothing for them. Cells shifted one by one (insert and delete cells) still get new ids as they go, so for them the record has `shifted` (the ids a formula went between, which the window's value follows), and a Move has `landed` (where its formulas went, for Undo). `EditRecord::Place` and `GoneCell` carry both the place and the ids; the ids are for use at once (marking, letting go of values), and the places are what Undo goes by (a place comes back as it was, an id is renumbered by a compaction, so a record that is kept holds places).
- **Results in flight.** `Item::epoch` (in the padding of the item) is the sheet's structural epoch when the value was worked out. Every edit of rows, columns, or cells makes the sheet's epoch one more. A formula that an edit of cells (shifted or moved) put in another cell is stamped with the epoch of the edit, at the place it left and the place it came to, and worked out again; the window (`EditSession::dropStale`) lets go of a value older than its stamp, and a last batch (`CalcBatch::epochs`) lets go of the stamps nothing older can be on its way for. **Formulas an edit of whole rows or columns wrote again are not stamped:** a value worked out for their old text lands on the right cell and is replaced by the new one that is already queued behind it, and a stamp for each of 100,000 formulas costs what the rest of the round saves. The generation, `m_log`, `moveToNow`, and the loader's `sealStale` are removed.
- **A compaction renumbers everything by ids.** `compactLines` gives the table the cells were renumbered by; the nodes, the window's values (`ValueStore::renumber`), every mark and result of the session, and the keys the undo steps hold are renumbered with it, the stamps of the sheet are let go of, and every formula of the sheet is worked out again (what is in flight is named by the old ids; the epoch says so).
- **Marks.** The marks and results by cell are keyed by (sheet, row id, column id) (`EditSession::keyOf`, `markOf`) and are turned into places by `placeOfMark` when the file is made (cells in rows or columns that are deleted are not in it, and the cells and results are sorted by place as they were). A deleted row's marks stay with its ids (Undo brings them back, so `touchedGone` and the like are only for cells shifted one by one); `sweepMarks` lets go of those on ids that nothing can name any more, when the records that held them are let go of and before the ids are given again. The set of the cells the person changed is a flat table (`KeySet`), so that marking the 100,000 formulas an edit wrote again is one pass.
- **Not done:** the nodes of formulas in deleted rows are still dropped at once (the plan keeps them dead; that needs the graph's questions to skip them, so it goes with round 4), and the saver still takes the formulas that were written again from the marks.

Measured on this machine, `edit-struct-bench` on 100,000 rows by 21 columns, plain, the same load for both (two runs each, the window thread's own time): insert a row at the top 277 and 277 ms before, 254 and 254 after; delete at the top 294 and 295, 275 and 275; insert in the middle 107 and 108, 136 and 97; delete in the middle 146 and 148, 141 and 137; at the end 6.5 and 6.6, 6.8 and 7.8; insert a column at the top 595 and 597, 600 and 605 (the graph's pass and the rewrite still cost what they did: rounds 4 to 7). The values the window moved for the row at the top 100,000 to 0 (and for the other edits of rows 99,999, 50,000, and 49,999 to 0, for columns 100,000 to 0), the marks followed by a column edit after earlier edits 199,800 to 200,000 to 0. Timed by hand inside `applyStructure`, the "values" phase of the table in 1.5 was 23 ms and is 0, and the "session" phase was 9.7 ms and is 4.1: what is left is marking the 100,000 formulas that were written again, which the saver still needs until round 7. A sheet's memory at rest is the same (194 MB against 192 and 193), and after the twelve edits and their Undo it is 330 to 340 MB against 335 to 355. `edit-walk-bench` on 1,000,000 rows by 20 columns of numbers: a full walk and a sliced walk from the end 53 and 134 ms before the edits and 53 and 135 now, 134 and 233 after them and 129 and 234 now (10 column and 1,000 row edits 3,167 ms before, 3,232 now). The 8 golden digests are unchanged, the long model check (200 seeds of 300 steps) has no failure, and the dev list's yardstick is the same (8,754,109 formula cells, 7,065,202 checked, 7,063,488 matched, 99.98%) with `cells.tsv` byte for byte the same.

### Round 4: the graph by id, and the worker's questions (4 days)

Intervals are ids; the column tree has 65,536 leaves; whole rows and columns are intervals to the largest id; `shiftRows` and the column fallback are removed; registering a formula converts positions to ids (an inserted end becomes the original row at that end); after an edit the worker is asked the four sets of 4.6 (with the blank-insensitive list and the position-observing trait scanned at every depth of a program) instead of the formulas near the edit being queued. Tests first: the random graph test (it asks every formula) extended with random inserts and deletes between queries (a query after an edit must return a superset of the exact answer, equal for edits that touch no inserted row); a change in an inserted row reaches a formula that reads a range around it; `ROW(B5)`, `COLUMN()`, and `SUMPRODUCT(ROW(A1:A10))` formulas are worked out again when their positions change and `A1+1` formulas are not (counted); a column inserted inside 1,000 row-sums asks for none of them; `COUNTBLANK` over the same range asks for all. Gate: the graph phase and "write formulas" phase to the questions' cost; the worker's settle after a rigid edit under 20 ms at 100,000 rows.

### Round 5a: formulas in the id frame (3.5 days; dark)

The parser's wide limits, `Sheet::formulaText`, the entry path (typing, paste, fill, import of a formula text), `resolveReferences` and the evaluator's reads through the maps, families split by run. Switch off by default: tests turn it on. Tests first: references to a row, a column, a range, a whole column, a row range across 200 random edits (the model check, both modes); a formula typed into an inserted row, a totals row, a computed column inserted and filled down, and `=A5` pointing at an inserted row, all id-held (a counter says no formula is positional but the 3-D ones); the formula bar and Find show positions; a saved file reads back to the same formulas.

### Round 5b: names, tables, dead references, 3-D, whole rows and columns (3 days; dark; needs 5a)

4.14 and the rules of 4.4, checked against the goldens and the naive model, with the named sequence. Tests first: the counters of 4.14; a deleted start and end; `#REF!` for a range whose rows are all gone; Undo bringing it back; `SUM(A:A)` with an insert at 0, at the last original row, and past it; 3-D formulas in both modes. Where the golden digests differ, each difference is understood and fixed or recorded as deliberate.

### Round 5c: sort, paste, cut, shift cells; the switch on (2.5 days; needs 5b)

4.15, and the model check's other steps in id mode. Gate for turning `idFrame` on by default: the model check in both modes at the nightly budget, the dev list's roundtrip with the edit script, `recalc-full` on the dev list unchanged (section 6), and the bench: formula visits 100,000 to a handful at 1,000 and at 100,000 rows; "text rewrite" and "write formulas" phases to 0; deleting the first row makes no texts.

### Round 6: records, Undo, sizes (3 days; needs 5c)

`EditRecord` holds the `LineMap` change and only the before-images the edit changed; Undo and Redo run the inverse, with the version stamp of 4.4; `RowSpec` is keyed by id; the byte estimate and the "can't be undone" question no longer apply to rows and columns; revived formulas are marked dirty in one worker job (4.7); the purge of dead cells and nodes when a step is forgotten runs in idle slices. Tests first: twelve edits at 100,000 rows hold under 64 KB (counted); a delete of 500,000 rows and its Undo (counted work, and the values shown at once); redo after undo after redo; Undo across a save (`m_saveGen` rules unchanged); a table edit with fill inside a structural step; notes and links with Undo. Gate: Undo time and memory in the table of 4.13.

### Round 7: the saver (4 days; needs 5c)

The list of edits is replayed once on `LineMap`s and the old file's rows and cells are mapped by lookups; the family-level rewriter of 4.8 gives each distinct formula text its new text; the touched set is turned from ids to positions when the snapshot is taken. The old saver stays, chosen by the switch. Tests first: the output of the dev list's edit script (`jcrunchtool roundtrip`) equal to the old saver's in every part but the formulas the old one wrote whole (those equal the old file's bytes renumbered, or the rewritten text); `tests/sheet/tst_sheet_structsave.cpp` unchanged; a test of 500 random edits on a small sheet saved and reopened against the window's state; shared families cut by an insert, a delete of the master's row, and a family that moves as one. Gate: the table of 1.6, no slower after 1, 20, and 200 edits.

### Round 8: the view in place (2.5 days)

`Axis` gets `insertPlaces` and `erasePlaces` (the sizes shift in a memory move; no rebuild), `showSheet(…, true)` after a structural edit is replaced by the in-place update, the selection and scroll position follow, and the viewport repaints only the changed band when the edit is at the bottom of the view. The floor at a million rows, which has not been split out yet, is measured first. The test of `hold()` of 4.9. Tests first: an axis insert and erase equal a rebuild (random); the view after an edit equals a view made again; a paint count after an edit at the end of the sheet. Gate: the floor, 8 ms to under 5; the whole bar of 4.13 on `edit-struct-bench` at both sizes.

### Round 9: hardening and clean-up (4 days)

The hostile tests: a million runs refused with the message, column ids exhausted and a rebase (4.16), a delete of the whole sheet and its Undo, ten thousand edits at the top and a save, a sheet with 100,000 rows inserted and filled (all id-held: formula visits counted), a session of random edits under the memory cap, 20,000 runs. The capped rebase. The yardsticks of section 6, `check_stable_saves.py`, and the bench at both sizes with the final numbers written into this note. The old eager paths and the switch are deleted, `docs/jeffcrunch.md` and `docs/features/jeffcrunch.md` (the "speed" line of the first release) are updated, and the report.

## 6. What must not change

- **Files.** The output for a part that was not edited is the old file's bytes; for a workbook with no structural edit it is exactly what it is now (`jcrunchtool roundtrip` on the dev list: 8,371 of 8,371 saved untouched, all parts the same; `tools/check_stable_saves.py` over the crunch dev list). For a workbook with edits, the saved parts equal what the current saver writes for the same steps, except that formulas that only moved are no longer written whole (they are the old file's bytes, renumbered), which the round 7 test pins down.
- **The calculation yardstick.** `jcrunchtool recalc-full dev-xlsx.txt out 14` on the dev list gives the same counts as before the work (7,208,571 formulas checked, 7,201,105 match, 99.90%), and the formula-by-formula check the same. The evaluator is not changed except for how a reference is turned into an area, which for a sheet without structural edits is the same arithmetic.
- **The round trip tools** (`roundtrip-one`, `roundtrip` with its edit script, the names and tables round trips), and the grid sweep (8,371 drawn).
- **Behavior.** Every refusal and its words (a table's header row, a data table's input cell, an array formula cut through, a protected sheet, cells pushed off the sheet), the rule that an edit that cannot be undone asks first (still for the edits that make large records), Undo after a save, the formula text a person sees after an edit (positions, `#REF!` where the common programs show it), and which cell is selected after an insert.
- **Typing** (4.3 ms middle, 6.1 ms at the 99th) and the open time and memory of a sheet nobody edited.
- **Tests.** Every existing test passes unchanged except those that assert something the redesign removes on purpose (the "can't be undone" question for a row or column, the generation of a batch); each such change is named in the commit.

## 7. Risks

1. **Meaning of references to deleted rows.** The invariant of 4.4 gives what the common programs give in the cases checked; in a long sequence they could differ in a corner. The model check, the goldens, and the two modes are there to find these; round 5b budgets deciding each difference.
2. **The id-frame text** touches the tokenizer, the printer, `shiftFormula`, and the rewriters (one parameter), and every stored text then holds large numbers. The risk is a place that assumes 1,048,576 (a clamp, a "gone" test); the compiler cannot list those, so round 5a greps for every use of the grid's limits and the model check feeds inserted-row formulas from its first seed.
3. **Two sweeps.** Round 2a (300 mentions, 30 files) and the formula round (5a to 5c, 9 days) are the likeliest to overrun; each ships alone or dark.
4. **Threads.** The `LineMap` is changed under the hold, and the window and the worker read it without a lock; versions on hints and the test of 4.9 are the check; if the machine's compiler has the thread sanitizer, a run of the crunch tests under it is part of round 9 (not checked here).
5. **The graph's gap rule** costs some extra recalculations (never a missed one). The superset check after every step in the model is the test; a counter reports the extra.
6. **Blank-insensitive and position-observing lists** are lists of functions: a function put on the first by mistake is skipped when it should not be. The default is sensitive, the list is short and each entry has a test, and the oracle of Round 0 (recalculate from scratch) catches a wrong skip.
7. **The saver** must write new text for every moved formula; a family cut by an edit is the hard case; the comparison with the old saver on the dev list's edit script is the check.
8. **Dead cells and nodes held** until the step is forgotten. Counted in the undo byte limit, purged in slices; a delete of 500,000 rows holds about 12 MB of cells until then.
9. **Caps and rebase.** Rows 2^24 ids, columns 49,152, runs 100,000: rebase in round 9 with a message and no undo; the refusal before it.
10. **The floor at a million rows is not split out yet.** An edit at the end of the million-row sheet took 15 to 54 ms, of which 12 to 17 are the graph's pass; the rest is the view and the repaint. Round 8 starts by measuring it, and the 16 ms bar depends on it. The spike measured the core only.
11. **The spike's limits.** Three formula kinds, no sparse lists, no session, no view, no saver, new columns' ids kept inside the graph's old 16,384. It says that the core is a microsecond and that the rival's graph pass is 40 to 80 ms at a million rows; it does not say the rest is easy.
12. **Estimates.** 41 days is for a helper who knows these files, with a range of 6 to 10 weeks; the plan can stop after rounds 4 and 5c with a product that is better than now and no worse.

## 8. Decisions to confirm

1. Option A (ids in run maps) over B (blocks and relative storage with the graph's pass). Recommended: A, because the graph pass alone keeps B above 16 ms at a million rows.
2. Every formula held by id, with the stored text allowed to hold ids up to 2^31 and 65,535 (4.4), instead of position texts for formulas in inserted rows. Recommended: yes; 3-D references alone stay positional.
3. The "can't be undone" question goes away for rows and columns (4.7). Recommended: yes.
4. No quick step first (the graph's column pass, half a week): round 4 makes it unnecessary. Recommended: no quick step.
5. Caps of 100,000 runs, 49,152 inserted columns, and 2^24 row ids per session, with a rebase that drops the undo history. Recommended: yes.
6. 41 days (8 weeks) rather than 21, with the stopping points after rounds 4 and 5c. Recommended: commit to round 0 to 4 first (about 18 days), and decide the rest on the real numbers after round 4.

## 9. How the review's findings are handled

| # | Finding | Where and how |
|--:|---|---|
| 1 | Whole rows and columns must resolve to first..last live position | 4.4: flags in `RefLit`, positions at every read, never the dead-id rules; the graph interval to the largest id; inserts at 0, at the last original row, and past it are named tests (round 5b) |
| 2 | "Reads its own place" is too narrow | 4.6 set 2: `observesPosition` for `ROW`, `COLUMN`, `ADDRESS`, `CELL`, `AREAS`, `OFFSET`, `INDIRECT` at any depth, with or without arguments; positions last seen are compared after each edit; round 4 tests `ROW(B5)`, `COLUMN()`, `SUMPRODUCT(ROW(A1:A10))` |
| 3 | Dead-end memory, Undo and Redo order | 4.4: the invariant (four statements), version stamp on steps, the named sequence (delete A, delete neighbor, Undo, Redo, insert between) in rounds 1 and 5b |
| 4 | Positional and id formulas must agree | 4.4 and Round 0: the eager engine stays as the oracle behind `idFrame`; the same text and steps in both modes, equal values, shown text, and saved files |
| 5 | Names and tables rewritten by position | 4.14: names and tables held in ids; registered again by name and by table, with counters and two bench cases |
| 6 | Dead cells: nodes, cost, revive | 4.7: nodes kept and flagged dead, no per-node work on the window thread, a 500,000-row delete costs the map, a block walk, and one worker question (4.8 ms for 100,000 rows in the spike); revive dirties the formulas in one worker job |
| 7 | Paste and fill of whole columns | Measured in the spike (3A, 4.15): fill 84 ms (rival 65) at 100,000 rows, 509 (652) at a million; paste 49 (33) and 22 (13) |
| 8 | Sort | 4.15: contents move, row ids stay, relative parts re-based by `dRow` or through the text pass; notes and links by `CellAnchors::permute`; sort is in the model check |
| 9 | Typical use makes many formulas positional | 4.4 and 4.1: every formula is id-held, the stored text accepts ids to 2^31 and 65,535; only 3-D references stay positional; round 5a counts positional formulas |
| 10 | The saver must rewrite moved formulas' text | 4.8: family-level rewriter, 4 days in round 7; the save stretch goal is dropped (the gate is no slower than now) |
| 11 | Steady-state benches after 1 and 1,000 edits | 3A: recalculation, paint, Find, formula-bar reads, memory after 1, 1,000, and 10,000 edits, for both designs; Round 0 repeats them on the real engine |
| 12 | Results before a non-rigid edit; `hold()` | 4.6 and 4.9: structural epoch per sheet, results stamped, older ones dropped; `hold()` makes the worker leave its formula, hints are versioned, a test edits under the hold during a long `SUM` |
| 13 | Walks over many runs | 4.17: cache of position range to id runs per epoch; a session of 20,000 runs in rounds 1 and 4 with counted steps; the spike ran 14,000 |
| 14 | Id exhaustion | 4.16: ids returned when a dropped insert held the highest, Undo and Redo use none, caps, capped rebase in round 9 |
| 15 | Each round must ship alone | Section 5: the `idFrame` switch, dark rounds 5a and 5b, 3 needs 2b, 6 and 7 need 5c, the old paths kept until round 9 |
| 16 | A stronger model check | Round 0: two sheets, 3-D, structured references, `OFFSET`, `INDIRECT`, `ROW(range)`, shared families from reopen, fill, paste, sort, cut, shift cells; after every step save, reopen, and recalculate from scratch against the incremental values; a graph-superset check |
| 17 | Re-estimate 2a and 5a | 2a: 4 days; the formula round: 9 days in three; the whole: 41 days, 8 weeks (6 to 10) |
| 18 | The spike | 3A, and `tools/spike/rowid_spike.cpp`; recommendation A |

## Appendix: repeating the measurements

```
python3 tools/crunch_struct_bench_make.py w.xlsx 100000 20 --all        # ROWS and NUMERIC_COLS; flags --cross --cf --dv --table --chart --names --all --plain
JP_NO_TELEMETRY=1 systemd-run --user --scope -p MemoryMax=16G build/jcrunchtool edit-struct-bench w.xlsx 100k-all
JC_UNDO_MB=8000 ... edit-struct-bench w.xlsx 1m-all "insert row"        # a larger undo limit; the last argument keeps the edits whose name contains it
JC_SAVE_EDITS=1,20,200 ... edit-save-struct-bench w.xlsx 100k-all       # saves after that many structural edits
```

The phase times of 1.5 come from timers placed by hand in `EditSession::applyStructure`, `Calculation::applyEdit`, and `CrunchWindow::onCellsMoved`; they are not in the commit (they are product code), and round 0's counters replace them. The tables are the output of the bench; the machine's other work moved single numbers by up to 20%.
