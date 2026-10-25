# How formulas are shown and calculated
<!-- keywords: formula, formulas, calculate, calculation, recalculate, function, functions, value, result, stored, F9, circular, show formulas, formula bar, error -->

A **formula** is a cell that works out its value from other cells, such as =SUM(B3:B8). The cell shows the value, and the **formula bar** shows the formula.

## What JeffCrunch does with them

When you open a workbook, the file holds the last value of every formula, so the sheet can be shown at once. Then JeffCrunch works every formula out again, a few at a time, so the window stays quick. The status bar says **Calculating** with a percent while it does. When it is done, the cells hold JeffCrunch's own results. To work everything out again, click **Recalculate All** on the **Formulas** tab, or press **F9**.

## When JeffCrunch can't work a formula out

JeffCrunch has more than 200 functions, and it is still growing. A formula that uses a function JeffCrunch doesn't have yet keeps the value the file stored for it. So does one that refers to another workbook. Cells that depend on those are worked out from the stored values.

## Show the formulas

Click **Show Formulas** on the **Formulas** tab, or press **Ctrl+`** (on a Mac, the Control key, not the Command key), to see the formula in each cell in place of its value.

## Changing formulas

To change a formula, select its cell, press **F2**, edit it, and press **Enter**. You can also edit it in the formula bar. When you change a cell, the formulas that depend on it are worked out again in the background. See [Type in cells](edit-cells).

## Writing a formula

Type **=** and the first letters of a function, and a short list shows the functions and [names](names) that begin that way, with a sentence about each. Press **Tab** to take the one that is marked. Use the **Down** and **Up** arrows to mark another, and then **Tab** or **Enter**. **Esc** closes the list. Once you are inside the brackets of a function, its arguments show under the cell, with the one you are typing in bold. The list and the arguments are the same ones **Pick a Function** shows. Arguments are separated by commas, or by semicolons where your language writes a comma for the decimal mark.

## Calculating by hand

Click **Calculation Mode** on the **Formulas** tab to choose how the workbook works its formulas out:

- **Automatic** works them out as you change cells.
- **Automatic, Tables by Hand** does the same, and is kept in the file for programs that treat data tables apart.
- **Manual** works nothing out until you ask. The status bar says **Out of date** after a change. Press **F9** (**Recalculate All**) to work everything out, or **Shift+F9** (**Recalculate This Sheet**) to work out the formulas of the sheet you are on, and whatever else was waiting. A workbook that is set to Manual opens with the results its file holds.

The choice is saved with the workbook, and it is one step you can undo.

## Formulas that read themselves

A formula that reads its own result, directly or through other cells, can't be worked out in the usual way. JeffCrunch tells you once, names the cells, and outlines them on the sheet; they keep the values they had. Click **Circular References** on the **Formulas** tab for the list, and double-click a cell in it to go there.

Some models need such a circle on purpose. Choose **Go Round Circular Formulas** in the **Calculation Mode** menu, and the formulas are worked out again and again, each reading the latest values of the others, until no value changes by more than the amount you give or the number of rounds you allow is used.

## See what a formula reads

Select a cell and click **Show What Feeds This Cell** to draw arrows from the cells its formula reads. Click again to go one step further back. **Show What Uses This Cell** draws arrows to the formulas that read the cell, and each click goes one step on. A small page stands for a cell on another sheet. At most 2,000 arrows are drawn, and the status bar says when there were more. **Clear Arrows** clears them, and so does any change to the workbook.
