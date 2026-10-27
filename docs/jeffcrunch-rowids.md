# JeffCrunch: row and column ids

*Design note, October 26, 2026. A plan, not a change: no product code is touched by the commit that adds this note. Under `tools/` it adds the bench that took the numbers (`jcrunchtool edit-struct-bench` and `edit-save-struct-bench`) and the script that makes the bench's workbooks (`crunch_struct_bench_make.py`).*

Inserting or deleting a row, a column, or cells in JeffCrunch costs the size of the sheet. Every cell after the edit is moved, every formula that reads what moved is written again, every value the window holds is moved, and a record of all of it is kept for Undo. The preview bar for an edit is 16 ms on the window's thread. A person typing meets it (a number typed over a cell is 4 ms in the middle, 6 ms at the 99th percentile); a row inserted at the top of a sheet of 100,000 rows does not (0.2 s), and on a sheet of a million rows it takes a second, or eight for a column. The first release noted this as "the work that is left". This note says what that work is, in what order to do it, and how to know each step worked.

The goal: a single row or column inserted or deleted anywhere in a million-row sheet takes under 16 ms on the window's thread, Undo and Redo the same, the work that must still be done (formulas that really change) is done by the worker, and saving, memory, and every result the yardsticks give are unchanged.

In short:

- **Now** (100,000 rows by 21 columns): a row at the top 0.17 s, a column 0.6 s, saving after any such edit 4.7 s; (1,000,000 rows by 5): 1.1 s, 7.4 s, 19.7 s, and 1.5 GB of Undo records for twelve edits. Everything that is per cell or per formula is keyed by position, so an edit renumbers all of it.
- **Recommended**: every row and column gets an id equal to its file position, kept in a short list of runs (`LineMap`) that grows by two entries per edit. Cells, formula nodes, the graph, the window's values, and the session's marks hold ids and never move; the few things (rules, names, charts, notes, and the like) keep positions and the code they have. Formulas keep their file texts; references are read through the map. Undo is the inverse map operation and costs kilobytes.
- **Targets**: one row or column inserted or deleted anywhere in a million rows, under 16 ms on the window thread (the model and session part under 3 ms); Undo and Redo the same; saving no slower, and the same bytes for sheets with no structural edit; memory no more than +10% at rest.
- **Plan**: twelve rounds, about 21 days, each shippable with a number it moves; the first two (a model check against a naive reference, and `LineMap`) change no behavior.

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
| b2 | about 0.1 s (0.45 s) | about 0.1 s (0.45 s) | the formulas the graph names | smaller (no cell copies) | 3 |
| a, as run maps (4) | 5 to 10 ms (5 to 10 ms) | 5 to 10 ms (5 to 10 ms) | none; only formulas that really change | kilobytes | 4 |

What each changes, in brief:

| | What changes where | The saver | Undo | Main risks and memory |
|---|---|---|---|---|
| b1 | `calcedit.cpp` (candidates, groups), `graph.{h,cpp}` (columns) | none | none | low risk; memory as now |
| b2 | seven structures, each with a pending-shifts list: `sheetmodel` (blocks with offsets), `calculation` (nodes), `valuestore`, `graph`, formula texts, `editsession` marks | must translate through each list as it writes | records still copy cells and texts | seven lazy mechanisms with seven sets of corner cases; a long edit list slows every read until it is compacted, which is a rebase; hostile: a session of 10,000 edits |
| a, as run maps | `sheetmodel` (cells by id), `calculation` (nodes, registration, dirty questions), `graph`, `valuestore`, `loader` (batches), `refs`/`eval` (references through the map), `editstruct`/`editsession` (records, marks), `axis`/`gridsheet` (view) | replays the edit list once on a map, then looks rows up; same bytes | the inverse map operation and the changed lists; kilobytes | references after deletes (4.4), a sweep of about 30 files, threads; memory: 24 bytes a run, dead cells held until their step is forgotten |
| a, textbook (id table and tree) | the same | the same | the same | the same, plus 12 to 16 MB for every million rows whether edited or not |

The figures for b1, b2, and a are estimates from the profile above: (a) removes every phase that grows, leaving the floor of the view and the repaint; b2 keeps the visit of each candidate formula and the loop that rewrites it (273 and 178 ms of the 1,077 at a million rows).

## 4. Recommendation: ids made from file positions, kept in run maps

### 4.1 The idea

Every row and every column of a sheet has an id for as long as the workbook is open (never saved: files store positions). At load, the id of a row is its position in the file (0 to 1,048,575) and the id of a column is its column number (0 to 16,383). **Ids and positions are equal until the first insert or delete, and a sheet that has none has no map at all** (one flag, "identity"): cells, formulas, graph, and values behave, byte for byte, as they do now. An insert makes new ids (rows from 2^20 up, columns from 2^14 up); a delete takes ids out of the order and keeps a record of them, so Undo puts back the same ids. The order of the ids is a **`LineMap`**: a list of runs, each a first id and a count of consecutive ids, in the order of position, with the running count of live rows before each run. It has one run to start with and two more for each edit.

Everything that is **per cell or per formula** holds ids and never has to move: the cells, the formula nodes, the graph's entries, the window's values, the worker's batches, the session's marks. Everything that is **few** (merged ranges, rules, names, tables, charts, drawings, notes, links, breaks, print areas, arrays) keeps positions and is followed by the code it has now: an edit that costs a millisecond for those is fine, and the code that is correct today stays in use.

A reference in a formula is an id too, so a formula never needs its text written again when a row or column moves (references follow cells, as they do now): the text is read through the map when a position is wanted (to evaluate it, to show it, to save it).

### 4.2 `LineMap` (`libs/base/calc/linemap.h`, new)

One for rows and one for columns of each sheet (`Sheet::rows`, `Sheet::cols`). Operations, with their cost in runs R (a few dozen after a session of edits, thousands after a long one):

- `position(id)` and `id(position)`: a binary search over the runs, O(log R); with a one-entry hint for the sequential reads of a recalculation, O(1). Identity: no search.
- `insert(position, count)` makes a run of new ids and splits the run it falls in: O(R) to move entries (a vector of 24-byte entries: 10,000 runs is a 240 KB move, 15 µs).
- `erase(position, count)` takes the ids at those positions out and returns the runs taken, which the caller keeps as the undo record: O(R + runs taken). It also records, for each run taken, the live ids on either side at that time (`before`, `after`).
- `revive(runs)` puts the same ids back in the same place (the neighbors named in the record are alive again because Undo is last in, first out).
- `isLive(id)`, `walk(position range)` (the runs a range of positions covers, in order, as id ranges: what the cell store's range walk is built on), and `resolveStart(id)`, `resolveEnd(id)`, which answer for an id that is not live (4.4).
- Caps, each refusing the edit with "This sheet has been edited too many times. Save it and open it again.": one million runs; 49,152 columns inserted in a session (a column id is 16 bits); about 15.7 million rows inserted in a session (the session keeps cells in 24 bits of row). All are far beyond anything a person does, and each is tested.

Positions past the last row (1,048,576 rows are live at most) are handled as now: an insert that would push a cell off the sheet is refused; empty rows pushed off simply die.

### 4.3 What is held by id, and what by position

| Held by id | How |
|---|---|
| Cells | `Cell` keeps its 24 bytes; `row` is the row id, `col` the column id (16 bits). Blocks are keyed by row id / 64; cells within a block are in order of (row id, column id). Nothing moves on an insert or a delete. |
| Formula nodes | `Node.row`, `Node.col` are ids. |
| Formula references | in the id frame (4.4). |
| Graph | row intervals are ids (4.6). The tree over columns grows from 16,384 to 65,536 leaves. |
| Values the window holds | `ValueStore` is keyed by (sheet, row id, column id). |
| Worker batches | items carry ids; the generation and `moveToNow` go. |
| Session marks | the marks and results by cell are keyed by ids; the saver turns them into positions when it is asked. |
| Row sizes and hidden state | `RowSpec.row` becomes an id; the sorted list is walked in the order of the map. |
| **Held by position** | merged ranges; arrays and data tables (`Sheet::ranges`); the filter; validations and conditional formats; defined names; tables; charts; drawings; notes and links; page breaks, print area and titles; column runs (at most 16,384); the view (selection, scroll, frozen panes). |

An id-or-position confusion would be silent (an id is a plausible number), so the sweep that changes `Cell` renames its fields (`row` to `rowId`, `col` to `colId`) and gives the cell store a positional interface in which every callback is handed the position it is called for; the compiler then lists every reader (about 30 files, 160 calls into the cell store). The `LineMap` interface takes and returns small wrapper types (`RowId`, `RowPos`), not bare integers.

### 4.4 Formulas: the id frame

A formula is a text plus, for each cell, `dRow` and `dCol`: the cell's distance from the cell the text was written for; a reference without `$` moves with the cell (`RefLit` in `calc/formula.h`). Today distances and numbers are rows and columns. **They become ids.** For everything read from a file the two are the same numbers, so loaded formulas, shared families, compiled programs, and the sheet's texts need no change. Resolving a reference to what it reads is the one new step: the numbers `resolveReferences` already computes are ids; the rows and columns of the area are `position(id)`. A range needs only its two ends; a row inserted inside a range is inside it by order, with no change to the reference.

Rules for a reference whose id is no longer live (a row was deleted), which keep the answers the common programs give:

- A single cell whose id is dead is `#REF!`.
- A range's start that is dead resolves to the live row that followed the deleted rows **when they were deleted** (the `after` the `LineMap` recorded), its end to the live row that preceded them (`before`). If that row was itself deleted later, the answer is looked up again, and so on. Rows inserted afterwards next to the deleted ones are therefore outside the range, as in the programs people know. If the start comes after the end, or neither is live, the range is `#REF!`.
- Undo brings the same ids back and every reference reads them again; nothing is restored for formulas.

Writing a text for a person or a file (the formula bar, Find in formulas, a saved cell): `Sheet::formulaText` turns ids into positions through the maps (a single pass over the text's references, the existing rewriter with a different mapping; identity: the text as it is). Typing, pasting, and filling go the other way: the text the person writes is in positions, and is given ids with the same pass. A shared family made by a fill is split where the rows or columns it covers or reads are not consecutive ids (a run boundary), so each part still has one text and a distance.

**What stays in positions: the formulas that cannot be written in the id frame.** A text holds row numbers up to 1,048,576, so a formula whose cell, or whose reference's end, is an inserted row or column (ids above those) cannot be written as an id-frame text. Formulas that read a block of several sheets (a 3-D reference) are positional too: they name the same cell on every sheet, and an edit on one sheet must not change that. These formulas stay *positional*: they keep position texts and are rewritten by an edit as today, by the code that is in use, and are listed in a per-sheet set so the edit visits them without searching. They are the formulas typed into rows or columns that were inserted, which is few: a person inserts a row, types in it, and is done. A test counts them, so that a workbook in which they are many (a hundred thousand rows inserted and filled with formulas) is seen; there it costs what an edit costs today, no more. The alternative, which makes every formula id-frame, is a text that accepts row numbers up to 2^31 and columns up to 65,535 (a change in the formula parser's limits, and in the rewriter that serves it); it is a later step if the count says it is worth it.

### 4.5 Reading cells

`CellStore::find(row, col)` takes positions, as now, and maps them (`id(position)` twice; a one-entry hint on each side). Its range walks (`forEachInRange`, `forEach`, `forEachRowTail`) take the `LineMap`'s runs for the rows and, inside a row, for the columns: the cells of a block are in order of id, and for the usual map of a few runs the walk visits each run's cells in turn, which is also position order. A callback gets the position beside the cell. Cells of dead rows and columns stay where they are (they are the undo record of a delete) and are never reached by a walk, because the walk follows the live runs; for the cells of dead columns inside live rows, a 65,536-bit table says which column ids are live. `cellCount()` is kept for the live cells.

A delete is therefore: take the ids out of the map, count and note the cells for the statistics (a walk of the deleted rows' blocks: time in proportion to what was deleted, not to the sheet), and keep nothing else. When the step falls off the undo stack, the dead cells are let go of in the idle time, in slices.

### 4.6 The graph

The graph keeps its structure (sorted runs in nodes of a tree over columns) and holds ids. Two rules make the intervals right without any rewriting:

- Ids of original rows and columns are in the order of position, so an interval of ids covers exactly the original rows between its ends, however many rows were inserted or deleted among them.
- **Inserted rows are found by the gap they sit in.** A change in a cell of an inserted row is asked as "the original rows just before and after it": the intervals that overlap that pair. A reference whose end is an inserted row (these formulas are the positional ones of 4.4) is told to the graph as the original row at that end, the interval a little too wide. Both err in one direction only (a formula is worked out when it need not be, never the other way).

`DependencyGraph::shiftRows` and the column fallback are removed. `ProgramTraits` gets one more flag, "reads its own place", set by the functions that answer with a position (`ROW`, `COLUMN`, `ADDRESS`, `CELL`, and the like) and by `OFFSET` and `INDIRECT`; the nodes with it are listed per sheet. The queries after an edit are asked once, of the worker (so the window thread does not wait for a result of 100,000 formulas): the formulas whose range holds the gap (they may now read more cells), the formulas that read the deleted ids, and the formulas that read where they are (`ROW()`, `COLUMN()`, `OFFSET`, `CELL`, and the like) whose cell moved. Rigid moves (the formula and what it reads both after the edit, or both before) ask for nothing.

### 4.7 Undo

A structural step holds the `LineMap` change (the runs inserted, or the runs taken out with their neighbors), the before-images of the position-keyed lists that the edit changed (and only those: a copy made when an edit actually changes it, not always), the positional formulas' old texts, and the notes' and links' delta. No cell is copied and no formula text is saved for the others. Undo is the inverse `LineMap` operation and the lists put back: time in proportion to what the step touched, and the record is kilobytes. The "This can't be undone" question for rows and columns goes away, which is a visible change; the question stays for the edits that still make large records (cut and paste of huge blocks, shifting cells).

### 4.8 Saving

Files store positions, so nothing about ids is saved. The saver today replays the list of edits for every row and cell it writes. It will instead replay the same list once on a fresh `LineMap` (the edit list already says which sheet, where, and how many) and ask it for each row of the old file: `position(id = the file's row)`; a row that is not live is gone; cells in the old file are renumbered by the same two lookups. That is the same mathematics as the loop it replaces, so the output is the same bytes, in time that no longer grows with the number of edits. The cells the person changed and the formulas whose text changed are written whole as now; with id-frame formulas that is only those whose text really changed (not every formula that merely moved), which makes the saved file smaller and the save faster. The part of a file for a sheet with no structural edit is not read.

The replay is not where the time goes (1.6): it is the writing of each cell again, 2 to 3 microseconds, with a copy of its bytes made for each of its parts and its address built as text. Round 7 therefore also writes the renumbered rows and cells in place (new address, same bytes) and keeps a shared formula shared when its family moves as one; the table of 1.6 is the check. That is a saving this redesign makes possible (a formula that only moved no longer has to be written out), not one it needs.

### 4.9 Threads and the window

The `LineMap` changes only on the window's thread, under the hold, as every other model change does; readers (the window drawing, the worker evaluating) read it without a lock, as they read the cells. After an edit: the grid's row sizes (`Axis`) change in place instead of the view being built again (the sizes in the view are positions; an insert at p shifts the entries after it, a memory move), the selection and scroll position follow, and the viewport repaints. The worker gets the questions of 4.6 and works out only what they name; the grid shows every value it has at once, in its new place, and a value that is being worked out again shows the old one until the worker has it, as for any edit.

### 4.10 What does not use ids

Inserting or deleting cells (shifting the rest of some columns or rows) and cut-and-paste move some cells, not whole lines. They keep their code (`reshuffle`, `applyMove`) and the position-based rewriting of the formulas that read the cells they move, through the cell store's positional interface. A formula the rewrite touches is written back in the id frame by the pass that gives a typed text its ids. Their cost is what they move, as now, and the "big step" question remains for them.

### 4.11 Memory

`Cell` stays 24 bytes, `Node` 32, a graph interval 16, `Formula` 16. A `LineMap` is 24 bytes per run, two runs per edit: 100 edits is under 5 KB. The graph's tree over 65,536 columns is made by use as now. The undo record shrinks from about 320 bytes per cell deleted and formula rewritten to a few runs. Dead cells stay in the store until their step is forgotten, at the 24 bytes each already has, against about 320 bytes a cell in the record today. The sheet at rest is unchanged, the target is no more than 10% (expected 0%).

### 4.12 Hostile inputs and limits

A file cannot make a `LineMap` (maps are made by edits, not read); only a long session can. The caps in 4.2 stop it; the idle purge of dead cells runs in slices with the work counted, not timed. An edit's work beyond the window thread is the worker's, which looks at a stop flag as it steps. Rebasing the ids to positions (so the run list returns to one entry) is not planned for the first version; it is an O(sheet) pass that could be done on the worker if a session ever needs it.

### 4.13 Targets

| | Today | Target |
|---|--:|--:|
| One row or column inserted or deleted anywhere in 1,000,000 rows (window thread, processor time, with the repaint) | 1.1 to 11.8 s | **under 16 ms at the 99th percentile; the model and session part under 3 ms** |
| Counted work for that edit | 6,000,000 | no more than the formulas that really change (rigid moves: under 100, whatever the sheet's size, tested at two sizes) |
| Undo and Redo of it | as long as the edit, 1.5 GB held for twelve | the same as the edit; the record in kilobytes |
| A delete of 500,000 rows | not measured (one row costs 1.9 to 4.3 s at a million rows) | under 50 ms (time in proportion to the blocks of cells deleted) |
| The worker after a rigid edit | 0.1 to 6 s | under 20 ms, then only what the questions of 4.6 name |
| Saving after a structural edit | 4.7 s at 100,000 rows; 19.7 s at a million | not slower after 1, 20, or 200 edits (stretch: under 1 s at 100,000 rows); sheets with none: the same bytes and the same time |
| Resident memory after opening | 465 MB (1,000,000 rows by 5) | not above +10% (expected: the same) |
| Resident memory after 12 structural edits (1,000,000 rows) | +323% (465 to 1,969 MB) | under +5% |
| Typing a number in a cell (the 4b-1 yardstick) | 4.3 ms middle, 6.1 ms 99th | not slower |

## 5. The build plan

Rounds of one or two days for one helper. Each ships with the full suite green, has a number it moves (the bench, or a counter that a test asserts), and writes its tests first. Counters are asserted, never time ("count work, never time it"); the bench's times are for the report. Estimates in the last column are for the window thread at 100,000 rows by 21 columns, plain, a row inserted at the top (0.17 s today); at a million rows they are about ten times larger until round 5a.

| Round | Days | What | Owns | Moves |
|---|--:|---|---|---|
| 0 | 1.5 | The harness | `tests/crunch/tst_crunch_structmodel.cpp`, `tests/crunch/structmodel.h`, `tools/jcrunchtool_edit.cpp`, `tools/crunch_struct_bench_make.py`, `calculation.{h,cpp}` (counters only), `CMakeLists.txt` | nothing; the baseline and the safety net |
| 1 | 1.5 | `LineMap` | `libs/base/calc/linemap.{h,cpp}`, `tests/sheet/tst_sheet_linemap.cpp`, `CMakeLists.txt` | microseconds per operation |
| 2a | 2 | The cell store's positional interface | `sheetmodel.{h,cpp}` and every reader of `Cell` (about 30 files) | nothing visible; byte-identical saves, same results |
| 2b | 1.5 | Cells held by id | `sheetmodel.{h,cpp}`, `calcedit.cpp`, `calcmove.cpp` | cells moved: 2,100,000 to 0; 174 to about 140 ms |
| 3 | 2 | Nodes, values, batches, and marks by id | `calculation.{h,cpp}`, `valuestore.{h,cpp}`, `loader.{h,cpp}`, `editsession.{h,cpp}`, `editstruct.cpp`, `crunchwindow_file.cpp` | about 100 ms; session and values phases to 0 |
| 4 | 2 | The graph by id; what the worker is asked | `graph.{h,cpp}`, `refs.{h,cpp}`, `calculation.cpp`, `calcedit.cpp` | about 75 ms; columns reach rows' cost (600 to 75); the worker's settle to what the questions name |
| 5a | 2 | Formulas in the id frame | `refs.cpp`, `eval.cpp`, `formula.h`, `calculation.cpp`, `calcedit.cpp`, `sheetmodel.cpp`, `entry.cpp`, `editpaste.cpp`, `editsession.cpp` | formula visits: 100,000 to the few; about 15 ms |
| 5b | 1.5 | Dead references, positional formulas, the old rewrite for rows and columns removed | `calcedit.cpp`, `refshift.cpp` | texts made: 99,997 to 0 |
| 6 | 2 | Records, Undo, sizes by id | `calcedit.cpp`, `calculation.h`, `editstruct.cpp`, `editsession.cpp`, `anchors.cpp`, `pagesettings.cpp`, `sheetobjects.cpp`, `tables.cpp`, `sheetmodel.h` | Undo and its record; the "can't be undone" question gone for rows and columns |
| 7 | 2 | The saver | `structsave.cpp`, `sheetpatch.cpp`, `xlsxsave.cpp`, `editsession.cpp` | save time after an edit (4.7 s at 100,000 rows) |
| 8 | 1.5 | The view in place | `axis.cpp`, `gridsheet.cpp`, `gridview*.cpp`, `crunchwindow_cells.cpp` | the floor: about 8 ms to under 5 |
| 9 | 1.5 | Hardening and clean-up | all | the final numbers; the old paths gone |

About 21 days; none of the rounds depends on a later one for correctness, so the work can stop after any round with a sheet that is faster than now and no worse in any other way. If the work stops early the best stopping points are after round 4 (the work moved to ids except formulas) and after 5b.

### Round 0: the harness (1.5 days)

Tests first, and nothing else changes.

- **A randomized model check.** `tst_crunch_structmodel.cpp` has a naive reference model: a map from (row, column) to a constant or a formula whose references are absolute positions, with the kinds of things an edit must carry: merged ranges, a conditional format and a validation (ranges and formulas), a defined name, a table, a chart series, a drawing anchor, a note, a link, a page break, the print area, a row height, a column width, and a filter. The model implements insert and delete of rows and columns by renumbering every item, in about 300 lines of its own (it shares no code with `adjustRange`). A seeded generator makes thousands of steps (insert and delete of 1 to 3 rows or columns at random places, cell edits, Undo and Redo, a save and reopen every 50 steps) on a small workbook of 40 by 12 cells; after every step it compares, over the whole sheet, the displayed text of every formula, every value after the calculation settles, and each kind above, with the window's session. Seeds 1 to 20 with 150 steps (3,000 steps, a few seconds each seed) run in the normal suite; `CRUNCH_SEEDS` and `CRUNCH_STEPS` raise them for the nightly. Every test needs a mutation: break the new code once (a row not moved in the chart's series) and see it fail.
- **Golden digests from today's engine.** The same steps are run on the unchanged code and a 64-bit digest of the observable state after each step is stored in the repository (`tests/crunch/structgolden.inc`, a few kilobytes). Round 9 removes nothing from it; a later change that is meant to differ (the dead-endpoint rules, if they differ in an edge case) changes the file in a commit that says why.
- **Counters** (`Calculation::editStats()`): cells moved or touched, formulas visited, formulas rewritten, texts made, graph entries touched, values moved, session entries followed, and the same for Undo. They are product code (a few lines) so tests can assert on them.
- The bench and the generator of this note.
- The rule for later rounds: a round's tests assert its counters at two sheet sizes (1,000 and 100,000 rows) and require them equal.

### Round 1: `LineMap` (1.5 days)

`libs/base/calc/linemap.h`: the operations of 4.2, with `RowId`, `RowPos`, `ColId`, `ColPos` wrapper types. Tests first (`tests/sheet/tst_sheet_linemap.cpp`): a property test against a naive `std::vector<int>` of ids (random insert, erase, revive in last-in-first-out order, then every id's position and every position's id, 20,000 steps per seed); the dead-reference answers of 4.4 against a naive recomputation; the caps; identity does no allocation; a micro bench of 1,000,000 rows and 10,000 edits. Gate: microseconds per operation, 24 bytes per run, all in a test that counts comparisons.

### Round 2a: the cell store's positional interface (2 days)

No behavior change. `CellStore` gets the interface that does not expose positions in `Cell`: `find`, `findMutable`, `set`, `erase`, and the walks give the position with the cell (`f(row, col, cell)` or a view); `Cell` keeps its fields but renamed `rowId`/`colId`, still equal to positions. The compiler lists the readers; each is changed to take the position from the call, not the cell. Owns every file the compiler lists. Gate: the suite, `jcrunchtool roundtrip` on the dev list byte-identical to before (untouched saves 100%), the grid sweep. This round is the biggest sweep of the plan and can be done by two people on separate directories (`libs/office` and `libs/shell` plus `apps`); if it overruns, it still ships alone.

### Round 2b: cells held by id (1.5 days)

`Sheet::rows`, `Sheet::cols` (`LineMap`s, identity); the cell store maps positions to ids on the way in and out; `shiftRows`, `shiftColumns` become `LineMap` operations plus the (still eager) update of node positions, record, and callbacks for formulas; dead rows and columns keep their cells. Tests first: the model check passes with counters showing cells moved = 0; a delete followed by Undo leaves `cellCount` and every cell as before; the dead-column table; a walk across five runs visits the cells in position order; sort, fill, paste, and sheet copy on a sheet that has had structural edits (these use the positional interface and must not notice). Gate: 174 to about 140 ms for the row at the top; columns lose the cells term (15 ms of 640 today, so little visible: the gain of columns waits for round 4).

### Round 3: nodes, values, batches, and marks by id (2 days)

`Node.row` and `Node.col` are ids; `ValueStore` is keyed by ids; the `CalcBatch` items carry ids (the generation, `m_log`, and `moveToNow` are removed); the session's marks and results (`m_touched` and the others) are keyed by ids and turned to positions only when the saver or a command asks; the move callback is gone, `movedFormulas` is gone from the record. Tests first: a batch in flight when an edit is made lands on the right cell (a test with a hold hook that edits between a batch's making and its taking: today's `moveToNow` test, rewritten); touched cells survive 200 random edits and save the same file. Gate: the "values" and "session" phases to 0 (32 ms of 188 at 100,000 rows; 416 of 1,077 at a million).

### Round 4: the graph by id, and the worker's questions (2 days)

Intervals are ids; the column tree has 65,536 leaves; `shiftRows` and the column fallback are removed; registering a formula converts positions to ids (an inserted end becomes the original row at that end); after an edit the worker is asked the three questions of 4.6 instead of the formulas near the edit being queued. Tests first: the existing random graph test (it asks every formula) extended with random inserts and deletes between queries (a query after an edit must return a superset of the exact answer, and equal for edits that touch no inserted row); a change in an inserted row reaches a formula that reads a range around it; `ROW()` formulas below an edit are worked out again and `A1+1` formulas are not (counted). Gate: the graph phase and "write formulas" phase to the questions' cost; the worker's settle after a rigid edit under 20 ms at 100,000 rows.

### Round 5a: formulas in the id frame (2 days)

`resolveReferences`, the evaluator's reads, `Sheet::formulaText`, and the entry path (typing, paste, fill, import of a formula text) work through the maps as in 4.4; the positional set holds the formulas that cannot be id-frame. Texts of formulas that are not changed by an edit are no longer written again, so `rewritten` shrinks to the positional formulas and the dead-reference cases. Tests first: references to a row, a column, a range, a whole column, a row range, and a 3-D block across 200 random edits (the model check); `ROW()`, `COLUMN()`, `OFFSET`, `INDIRECT`, and a defined name that reads a cell; a fill after inserts splits into runs and shares text within a run; the formula bar and Find show positions; a saved file reads back to the same formulas. Gate: formulas visited from 100,000 to a handful at 100,000 and at 1,000; the "text rewrite" and "write formulas" phases to 0.

### Round 5b: dead references and the old rewrite removed (1.5 days)

The rules of 4.4 (dead start and end, `#REF!`, Undo) are checked against the goldens and the naive model; where the golden digests differ, each difference is understood and either fixed or recorded as deliberate in the digest file's commit. The old row and column rewrite in `applyEdit` (groups, `retext`, `was`) is deleted; it stays only for cell shifting and Move through the positional pass. Gate: "texts made" by deleting the first row: 99,997 to 0 (350 ms to a few); the deleted-row cases.

### Round 6: records, Undo, sizes (2 days)

`EditRecord` holds the `LineMap` change and the changed before-images only; Undo and Redo run the inverse; `RowSpec` is keyed by id; the lists copied for Undo are copied only when the edit changed them; the byte estimate (`estimateUndoBytes`) and the "can't be undone" question no longer apply to rows and columns; the purge of dead cells when a step is forgotten runs in idle slices. Tests first: twelve edits at 100,000 rows hold under 64 KB (counted); a delete of 500,000 rows and its Undo; redo after undo after redo; Undo across a save (`m_saveGen` rules: unchanged); a table edit with fill inside a structural step; notes and links with Undo. Gate: Undo time and memory in the table of 4.13.

### Round 7: the saver (2 days)

The list of edits is replayed once on `LineMap`s and the old file's rows and cells are mapped by lookups (4.8); the touched set is turned from ids to positions when the snapshot is taken; rows and cells are renumbered in place and a family of shared formulas that moved as one stays shared. Tests first: the output of the dev list's edit script (`jcrunchtool roundtrip`) byte-identical to the old saver's in every part until the in-place writer is switched on, then identical in every part but the formulas that only moved (those are the old file's bytes); `tests/sheet/tst_sheet_structsave.cpp` unchanged; and a new test of 500 random edits on a small sheet saved and reopened against the window's state. Gate: the table of 1.6, with a save after one edit no slower than now, and the stretch of under 1 s at 100,000 rows (a count of bytes copied in the test, the time in the report).

### Round 8: the view in place (1.5 days)

`Axis` gets `insertPlaces` and `erasePlaces` (the sizes shift in a memory move; no rebuild), `showSheet(…, true)` after a structural edit is replaced by the in-place update (the geometry made ahead stays, merges are re-read from the sheet's list), the selection and scroll position follow, and the viewport repaints only the changed band when the edit is at the bottom of the view. Tests first: an axis insert and erase equal a rebuild (random); the view after an edit equals a view made again; a paint count after an edit at the end of the sheet. Gate: the floor, 8 ms to under 5; the whole bar of 4.13 on `edit-struct-bench` at both sizes.

### Round 9: hardening and clean-up (1.5 days)

The hostile tests: a million runs refused with the message, column ids exhausted, a delete of the whole sheet and its Undo, ten thousand edits at the top and a save, a sheet with 100,000 rows inserted and filled (the positional count), a session of random edits under the memory cap. The yardsticks of section 6, `check_stable_saves.py`, and the bench at both sizes with the final numbers written into this note. The old eager paths are deleted, `docs/jeffcrunch.md` and `docs/features/jeffcrunch.md` (the "speed" line of the first release) are updated, and the report.

## 6. What must not change

- **Files.** The output for a part that was not edited is the old file's bytes; for a workbook with no structural edit it is exactly what it is now (`jcrunchtool roundtrip` on the dev list: 8,371 of 8,371 saved untouched, all parts the same; `tools/check_stable_saves.py` over the crunch dev list). For a workbook with edits, the saved parts equal what the current saver writes for the same steps, except that formulas that only moved are no longer written whole (they are the old file's bytes, renumbered), which the round 7 test pins down.
- **The calculation yardstick.** `jcrunchtool recalc-full dev-xlsx.txt out 14` on the dev list gives the same counts as before the work (7,208,571 formulas checked, 7,201,105 match, 99.90%), and the formula-by-formula check the same. The evaluator is not changed except for how a reference is turned into an area, which for a sheet without structural edits is the same arithmetic.
- **The round trip tools** (`roundtrip-one`, `roundtrip` with its edit script, the names and tables round trips), and the grid sweep (8,371 drawn).
- **Behavior.** Every refusal and its words (a table's header row, a data table's input cell, an array formula cut through, a protected sheet, cells pushed off the sheet), the rule that an edit that cannot be undone asks first (still for the edits that make large records), Undo after a save, the formula text a person sees after an edit (positions, `#REF!` where the common programs show it), and which cell is selected after an insert.
- **Typing** (4.3 ms middle, 6.1 ms at the 99th) and the open time and memory of a sheet nobody edited.
- **Tests.** Every existing test passes unchanged except those that assert something the redesign removes on purpose (the "can't be undone" question for a row or column, the generation of a batch); each such change is named in the commit.

## 7. Risks

1. **Meaning of references to deleted rows.** The rules of 4.4 give what the common programs give in the cases listed; in a long sequence (delete, insert next to it, delete again, undo) they could differ in a corner. The model check and the goldens are there to find these; the round 5b budget includes deciding each difference.
2. **The sweep of round 2a.** About 160 calls and 300 mentions of `Cell.row`; renaming makes the compiler list them, so none is missed, but a reader that decided "row" from a cell may have meant something else. Each is read, not replaced by script.
3. **Threads.** The `LineMap` is changed under the hold, and the window and the worker read it without a lock. A test makes edits while the worker is busy (the hold hook), and, if the machine's compiler has the thread sanitizer, a run of the crunch tests under it is part of round 9 (not checked here).
4. **The graph's gap rule** costs some extra recalculations (never a missed one). A property test against "ask every formula" is the check; a counter reports the extra.
5. **Positional formulas.** Where they are many (a hundred thousand inserted rows filled with formulas) an edit costs what it does today. The count is reported; the later step (a text that accepts large ids) is described in 4.4.
6. **Column ids are 16 bits** (49,152 inserts per session). Refusal with a message, tested; widening `Cell` is not worth 4 bytes on every cell.
7. **Dead cells held** until the step is forgotten. Counted in the undo byte limit, purged in slices.
8. **Saver equality** for edited workbooks is the hard-to-see risk; round 7's byte comparison against the old saver on the dev list's edit script is the check, and the old saver stays in the tree until round 9 for the comparison.
9. **The floor at a million rows is not split out yet.** An edit at the end of the million-row sheet took 15 to 54 ms, of which 12 to 17 are the graph's pass; the rest is the view and the repaint (and, once, a count of formula texts the first edit makes). Round 8 starts by measuring it, and the 16 ms bar depends on it.
10. **Estimates.** The round sizes are for a helper who knows these files; rounds 2a and 5a are the likeliest to overrun. Each ships alone, and the plan can stop after any round with a faster, no-worse product.

## 8. Decisions to confirm

1. Formulas in inserted rows and columns stay positional (4.4) for the first delivery, with a count, instead of widening the formula parser to ids above 2^20. Recommended: yes; revisit with the count.
2. The "can't be undone" question goes away for rows and columns (4.7). Recommended: yes.
3. A quick step first (b1, half a week, brings a column to the cost of a row)? Recommended: no, round 4 makes it unnecessary and it would be thrown away.
4. The caps: a million runs and 49,152 inserted columns per session, refused with a message. Recommended: yes.

## Appendix: repeating the measurements

```
python3 tools/crunch_struct_bench_make.py w.xlsx 100000 20 --all        # ROWS and NUMERIC_COLS; flags --cross --cf --dv --table --chart --names --all --plain
JP_NO_TELEMETRY=1 systemd-run --user --scope -p MemoryMax=16G build/jcrunchtool edit-struct-bench w.xlsx 100k-all
JC_UNDO_MB=8000 ... edit-struct-bench w.xlsx 1m-all "insert row"        # a larger undo limit; the last argument keeps the edits whose name contains it
JC_SAVE_EDITS=1,20,200 ... edit-save-struct-bench w.xlsx 100k-all       # saves after that many structural edits
```

The phase times of 1.5 come from timers placed by hand in `EditSession::applyStructure`, `Calculation::applyEdit`, and `CrunchWindow::onCellsMoved`; they are not in the commit (they are product code), and round 0's counters replace them. The tables are the output of the bench; the machine's other work moved single numbers by up to 20%.
