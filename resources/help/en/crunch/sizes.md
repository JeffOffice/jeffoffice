# Change the width of columns and the height of rows
<!-- keywords: default width, default row height, standard width, standard height, width, height, column, row, size, wider, taller, narrower, shorter, fit, autofit, hide, unhide, drag, border, double click, characters, points, Ctrl+0, Ctrl+9, pixels, number too wide, hashes -->

A **column's width** is counted in characters: how many digits fit across it. A **row's height** is counted in points, a unit used for type (72 points make an inch). The usual column holds about 8 digits, and the usual row is 15 points high.

## Drag a border

Point at the line between two column letters (or two row numbers). The pointer changes to arrows, and you can drag the line. A dashed line shows where the border will go, and nothing changes until you let go. Drag it all the way to the other border to hide the column or row.

If whole columns (or rows) are selected and you drag the border of one of them, they all get the same size.

## Fit what is inside

**Double-click** the border to make the column just wide enough for the longest thing in it, or the row just tall enough for its tallest line. This also works from **Home > Format > AutoFit Column Width** and **AutoFit Row Height**, for all the selected columns or rows at once. A column or row with nothing in it keeps its size.

## Type a size

Select the columns or rows, then use **Home > Format > Column Width** or **Row Height**, and type the size. A size of 0 hides them.

## Change the standard size

Columns and rows that have no size of their own take the **standard** size: 8.43 characters wide and 15 points high, unless the workbook says otherwise. To change it for the whole sheet, use **Home > Format > Default Width** or **Default Row Height**, type the size, and click **OK**. Every column (or row) that has not been given a size of its own changes at once; the ones you sized yourself keep their sizes. This is one step you can undo, and the workbook keeps the new standard.

## Hide and show

**Ctrl+0** hides the selected columns, and **Ctrl+9** hides the selected rows. To show hidden ones, select the columns or rows on both sides of them (or a block that covers them) and use **Home > Format > Unhide Columns** or **Unhide Rows**. The right-click menu on the column letters and row numbers has these commands too.

## A number too wide for its cell

When you type or paste a number that does not fit its column, the column widens to show it, if the column still has the width it began with. A column whose width you set yourself keeps it, and a number that does not fit it shows as **#** marks until you widen the column. Typing and the widening are one step, so **Undo** takes back both.

Every change is one step you can undo, and the file keeps the sizes. For a sense of scale: a column 8.43 characters wide is about 64 pixels, and a row 15 points high is 20 pixels, at 100% zoom.
