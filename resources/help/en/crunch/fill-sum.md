# Fill cells and add them up
<!-- keywords: fill, series, drag, handle, AutoFill, Ctrl+D, Ctrl+R, down, right, up, left, number, date, weekday, month, year, step, stop, copy, Sum, sum, average, count, max, min, Alt+equals, total -->

## Fill

Fill continues what you started. Select the cells you filled in, then drag the small square at the corner of the selection (the **fill handle**) along a column or a row, and let go. A dashed border shows how far it will go.

| You start with | The fill gives you |
| --- | --- |
| 1, 2 | 3, 4, 5, and so on: the step between them continues |
| A single number such as 5 | 5, 5, 5: copies (hold Ctrl while dragging for the opposite choice) |
| A date | The next days (use the **Fill Options** button, or **Fill > Series**, for weekdays, months, or years) |
| Mon, or January | Tue, Wed, ... or February, March, ... in your language |
| Item 1 | Item 2, Item 3, ... |
| A formula | The formula copied, with its references moved as in a paste |

**Fill > Down** (**Ctrl+D**) and **Fill > Right** (**Ctrl+R**) copy the first row or column of the selection over the rest. **Fill > Up** and **Fill > Left** work from the last. **Fill > Series** asks for the kind (linear, growth, date), the unit of a date, the step, and where to stop. Every filled cell takes the format of the cell it follows. A cell that can't be changed (part of an array, a locked cell, or a table's header) stops the fill, and nothing changes.

After you drag the fill handle, the small **Fill Options** button appears at the corner of what was filled. Its menu does the fill again another way, and the fill stays one step of Undo: **Copy Cells**, **Fill Series**, **Fill Formatting Only** (the cells keep what they hold and take the format), **Fill Without Formatting** (the cells keep their own format), and, for dates, **Fill Days**, **Fill Weekdays**, **Fill Months**, and **Fill Years**. The button goes away when you do anything else.

A fill does not go into or out of merged cells, because it would leave values in cells the merged area hides. Unmerge them first.

**Fill > Justify** puts the text of the selected cells of one column into lines as wide as the column, in the cells from the first one down; leftover cells of the selection are emptied. If the text needs more cells than you selected, JeffCrunch asks whether to use the cells below. It works only on text.

**Fill > Across Worksheets** copies the selected cells to the same cells of the sheets you check in a list: everything, just the contents, or just the formats. It is one step of Undo.

## Sum

**Home > Sum** (**Alt and the equals sign**) puts a total in the cell you select. It looks at the numbers above the cell; if there are none, at the numbers to its left. The formula is put in the cell for you to check, and **Enter** keeps it. The arrow next to Sum gives the other totals: **Average**, **How Many Numbers**, **Max**, and **Min**.

Select a block first, and each column of the block gets its own formula in the empty cells under it (or, for a single row, to its right).
