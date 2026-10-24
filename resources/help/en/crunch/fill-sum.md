# Fill cells and add them up
<!-- keywords: fill, series, drag, handle, AutoFill, Ctrl+D, Ctrl+R, down, right, up, left, number, date, weekday, month, year, step, stop, copy, AutoSum, sum, average, count, max, min, Alt+equals, total -->

## Fill

Fill continues what you started. Select the cells you filled in, then drag the small square at the corner of the selection (the **fill handle**) along a column or a row, and let go. A dashed border shows how far it will go.

| You start with | The fill gives you |
| --- | --- |
| 1, 2 | 3, 4, 5, and so on: the step between them continues |
| A single number such as 5 | 5, 5, 5: copies (hold Ctrl while dragging for the opposite choice) |
| A date | The next days (use **Fill > Series** for weekdays, months, or years) |
| Mon, or January | Tue, Wed, ... or February, March, ... in your language |
| Item 1 | Item 2, Item 3, ... |
| A formula | The formula copied, with its references moved as in a paste |

**Fill > Down** (**Ctrl+D**) and **Fill > Right** (**Ctrl+R**) copy the first row or column of the selection over the rest. **Fill > Up** and **Fill > Left** work from the last. **Fill > Series** asks for the kind (linear, growth, date), the unit of a date, the step, and where to stop. Every filled cell takes the format of the cell it follows. A cell that can't be changed (part of an array, a locked cell, or a table's header) stops the fill, and nothing changes.

## AutoSum

**Home > AutoSum** (**Alt and the equals sign**) puts a total in the cell you select. It looks at the numbers above the cell; if there are none, at the numbers to its left. The formula is put in the cell for you to check, and **Enter** keeps it. The arrow next to AutoSum gives the other totals: **Average**, **Count Numbers**, **Max**, and **Min**.

Select a block first, and each column of the block gets its own formula in the empty cells under it (or, for a single row, to its right).
