# How formulas are shown and calculated
<!-- keywords: formula, formulas, calculate, calculation, recalculate, function, functions, value, result, stored, F9, circular, show formulas, formula bar, error -->

A **formula** is a cell that works out its value from other cells, such as =SUM(B3:B8). The cell shows the value, and the **formula bar** shows the formula.

## What JeffCrunch does with them

When you open a workbook, the file holds the last value of every formula, so the sheet can be shown at once. Then JeffCrunch works every formula out again, a few at a time, so the window stays quick. The status bar says **Calculating** with a percent while it does. When it is done, the cells hold JeffCrunch's own results. To work everything out again, click **Calculate Now** on the **Formulas** tab, or press **F9**.

## When JeffCrunch can't work a formula out

JeffCrunch has more than 200 functions, and it is still growing. A formula that uses a function JeffCrunch doesn't have yet keeps the value the file stored for it. So does a formula that goes round in a circle, and one that refers to another workbook. Cells that depend on those are worked out from the stored values.

## Show the formulas

Click **Show Formulas** on the **Formulas** tab, or press **Ctrl+`** (on a Mac, the Control key, not the Command key), to see the formula in each cell in place of its value.

## Changing formulas

Typing in cells and changing formulas come in a later version. Right now JeffCrunch shows formulas, and the formula bar is for reading.
