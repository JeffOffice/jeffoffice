# Quick Analysis
<!-- keywords: quick analysis, analyze, analysis, instant chart, quick chart, quick totals, totals, running total, percent of total, data bars, color scale, icon set, sparklines, table, pivot table, Ctrl+Q, preview -->

**Quick Analysis** puts the most common ways to look at a block of numbers in one small box: color the cells, chart them, total them, make them a table, or give each row a tiny chart. You pick what you want, and you see it on your sheet before you decide.

## Open it

1. Select two or more cells that have something in them. A small button appears just below and to the right of the last selected cell.
2. Click the button, or press **Ctrl+Q** (on a Mac, **Control+Q**), or click **Home > Editing > Quick Analysis**.
3. Pick a tab, and then a choice.

The button never takes the keyboard from the sheet. It goes away when you select one cell, or cells with nothing in them, or when the sheet is protected in a way that does not let you do anything the box offers.

## Look before you choose

Point at a choice, or press the **Left** and **Right** arrow keys to move over the choices of a tab (the **Up** and **Down** arrow keys move between the tabs). The sheet shows what the choice would do, but nothing is changed: there is no step to undo, and a file saved at that moment has none of it. Press **Esc** or click away, and the look is gone. Click a choice, or press **Enter**, to make it. Each choice you make is one step of **Undo**.

## The tabs

- **Formatting**: **Data Bars**, **Color Scale**, and **Icon Set** show how big each number is. **Greater Than** colors the cells whose number is greater than the average of all the numbers. **Top 10%** colors the highest tenth. **Clear Format** takes the conditional formatting off the selected cells. These are the same rules as in **Home > Conditional Formatting**, and you can change them there.
- **Charts**: up to four kinds of chart that suit the shape of your cells, the same ones **Insert > Recommended Charts** picks. A look shows the chart in the corner of the sheet. Choosing one puts the chart on the sheet.
- **Totals**: **Sum**, **Average**, **Count**, **% of Total**, and **Running Total**, written as ordinary formulas. The first row of the choices puts one total under each column of numbers. The second row (**Row Sum**, and so on) puts one total to the right of each row. A first row or first column that holds only words is taken to be names and is left out. **% of Total** shows each column's (or row's) share of all the numbers, as a percent. **Running Total** adds up as it goes, in new cells beside (or under) your cells. If any of the cells where the totals go already hold something, you are asked before they are replaced.
- **Tables**: **Table** makes the cells a table, with buttons to sort and filter. **Pivot Table** makes a pivot table that totals the data, on a new sheet; the first row should hold the names of the columns.
- **Sparklines**: **Line**, **Column**, and **Win/Loss** draw a small chart of each row in the cell to the right of it.

## Limits

Quick Analysis looks at up to 250,000 cells for totals and sparklines. If you select more, it says so and changes nothing. A choice that would not fit (no room under the cells, for example) says so too.
