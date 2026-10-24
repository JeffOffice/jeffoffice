# Conditional formats
<!-- keywords: conditional formatting, conditional format, rule, highlight, color scale, data bar, icon set, icons, top, bottom, above average, below average, duplicate, unique, formula, stop if true, priority, bars, arrows, traffic lights, gradient, differential format -->

A **conditional format** is a rule that changes how a cell looks when what it holds meets a condition. Example: a rule says "color a cell red if it is below 100". A cell holding 80 is red, a cell holding 120 is not, and when you change 80 to 150 the red goes away by itself. The cell's own format is not touched: the rule only decides what is drawn.

## What JeffCrunch shows

JeffCrunch shows the rules a workbook has:

- **Highlight rules**: a rule that changes the font, the fill, the border, or the number format of the cells that meet it. The condition can be a comparison with a value (greater than, between, equal to, and so on), a formula, text that contains, begins with, or ends with some words, a date such as "last week", cells that are blank or have an error, values that are among the highest or lowest (a number or a percent), values above or below the average, and values that are duplicated or unique.
- **Color scales**: each cell is filled with a color that moves between two or three colors as its number goes from low to high. A scale from white to green makes the biggest numbers the greenest.
- **Data bars**: a bar in the cell as long as the number is large. Bars can be a solid color or fade out, can have a border, can have an axis, and run the other way, in another color, for negative numbers.
- **Icon sets**: a small picture in the cell that says which part of the range the number is in, such as an up arrow, a sideways arrow, or a down arrow; or a red, yellow, or green light. There are twenty sets.

When two rules say something about the same cell, the one that comes first in the workbook's order wins for what they both set, and a rule marked "stop if true" ends the search. The numbers in a color scale, a data bar, or an icon set are counted over the rule's own cells, and they are counted again when one of those cells changes.

## What stays the same

- Rules follow the cells they belong to. Insert or delete rows or columns, cut and paste, and the rules still cover the right cells; **Undo** puts them back.
- A rule JeffCrunch does not draw is kept in the file just as it was.
- Saving writes the rules the workbook had, unchanged.

## What is not done yet

**You can't make, change, or delete a rule yet.** JeffCrunch has no **Conditional Formatting** command: it shows what the workbook already has, and keeps it right through your edits. Rule editing comes in a later version. Until then, make or change rules in the program that made the workbook.
