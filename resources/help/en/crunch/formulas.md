# How formulas are shown and calculated
<!-- keywords: formula, formulas, calculate, calculation, recalculate, function, functions, value, result, stored, F9, circular, show formulas, formula bar, error, point, pointing, click a cell, reference, F4 -->

A **formula** is a cell that works out its value from other cells, such as =SUM(B3:B8). The cell shows the value, and the **formula bar** shows the formula.

## What JeffCrunch does with them

When you open a workbook, the file holds the last value of every formula, so the sheet can be shown at once. Then JeffCrunch works every formula out again, a few at a time, so the window stays quick. The status bar says **Calculating** with a percent while it does. When it is done, the cells hold JeffCrunch's own results. To work everything out again, click **Recalculate All** on the **Formulas** tab, or press **F9**.

## When JeffCrunch can't work a formula out

JeffCrunch has more than 200 functions, and it is still growing. A formula that uses a function JeffCrunch doesn't have yet keeps the value the file stored for it. So does one that refers to another workbook. Cells that depend on those are worked out from the stored values.

## MOD: the remainder

**MOD(n, d)** gives what is left of **n** after the most whole copies of **d** are taken out: it is n - d * INT(n / d). Example: **=MOD(10, 3)** is 1, and **=MOD(-10, 3)** is 2, because INT(-10 / 3) is -4 and -10 - 3 * -4 is 2. The answer has the sign of **d**. If **d** is 0, the answer is #DIV/0!.

## Show the formulas

Click **Show Formulas** on the **Formulas** tab, or press **Ctrl+`** (on a Mac, the Control key, not the Command key), to see the formula in each cell in place of its value. The columns that hold formulas are made wider, up to a limit, so that the formulas show whole; they go back to their widths when you turn it off, and the widths in your file are not changed.

## Changing formulas

To change a formula, select its cell, press **F2**, edit it, and press **Enter**. You can also edit it in the formula bar. When you change a cell, the formulas that depend on it are worked out again in the background. See [Type in cells](edit-cells).

## Writing a formula

Type **=** and the first letters of a function, and a short list shows the functions and [names](names) that begin that way, with a sentence about each. Press **Tab** to take the one that is marked. Use the **Down** and **Up** arrows to mark another, and then **Tab** or **Enter**. **Esc** closes the list. Once you are inside the brackets of a function, its arguments show under the cell, with the one you are typing in bold. The list and the arguments are the same ones **Pick a Function** shows. Arguments are separated by commas, or by semicolons where your language writes a comma for the decimal mark.

## Pointing at cells

Instead of typing a cell's address, you can point at it. While you type a formula, put the cursor where a cell can go (right after **=**, an operator such as **+**, an opening bracket, or a comma) and click the cell. Its address is written for you. Drag over cells to write a block such as B2:D5. Click again before you type anything else, and the new cell replaces the one just written. Hold **Ctrl** and click to add another address after a comma. Click a column letter to write a whole column such as A:A, or a row number to write a whole row such as 3:3, and drag along the letters or numbers for B:D or 2:5.

The arrow keys work too. Press an arrow and a pointer moves away from the cell you are typing in, and its address is written; press more arrows to move on, or hold **Shift** to stretch the pointer over a block. The status bar says **Point** while the arrow keys point, **Enter** while you type, and **Edit** while the arrow keys move the cursor in the text. Press **F2** to switch between pointing and editing the text.

To point at a cell on another sheet, click that sheet's tab while the cursor is where a cell can go, and then click the cell. The sheet's name is written with it, in quotes when it needs them, as in 'My Sheet'!B3. **Enter** takes you back to the sheet you started on and puts the formula in its cell. **Esc** gives everything up, the change of sheet too.

You can also drag the border of an outlined block on the sheet to move that address, or drag its corner square to stretch it; the formula changes as you drag.

Press **F4** with the cursor in or just after an address to fix parts of it with a dollar sign: A1 becomes $A$1, then A$1, then $A1, and then A1 again.

Each address in the formula is colored in the text, and the cells it names are outlined on the sheet in the same color, so you can see at a glance what the formula reads. A click after a whole address, such as =A1, ends the entry and selects the cell you clicked, as it does for any other text.

## Data tables

A data table (what-if table) shows what a formula comes to for many values of one or two cells. Open a workbook that has one, and JeffCrunch works its cells out: for a table with two inputs, the formula is in the cell at the table's top left corner, the values to try across go into the row input cell, and those down go into the column input cell. For a table of one input, the values go down the column at its left (and the formulas are in the row above it) or across the row above it (and the formulas are in the column at its left). Each cell of the table is the result of the formula, and of every formula that depends on the input cell, with the input set to that value; nothing else in the workbook is touched. In **Automatic** mode the tables are worked out with the formulas, and the formulas that read a table's cells follow. Making a new data table is not offered yet. A table that cannot be worked out (a formula that depends on the input uses a function JeffCrunch does not have, or changes by itself, such as RAND or NOW) keeps the values its file holds.

## Calculating by hand

Click **Calculation Mode** on the **Formulas** tab to choose how the workbook works its formulas out:

- **Automatic** works them out as you change cells.
- **Automatic, Tables by Hand** does the same, but leaves the cells of data tables as they are until you press **F9**.
- **Manual** works nothing out, data tables included, until you ask. The status bar says **Out of date** after a change. Press **F9** (**Recalculate All**) to work everything out, or **Shift+F9** (**Recalculate This Sheet**) to work out the formulas of the sheet you are on, and whatever else was waiting. A workbook that is set to Manual opens with the results its file holds.

The choice is saved with the workbook, and it is one step you can undo.

## Formulas that read themselves

A formula that reads its own result, directly or through other cells, can't be worked out in the usual way. JeffCrunch tells you once, names the cells, and outlines them on the sheet; they keep the values they had. Click **Circular References** on the **Formulas** tab for the list, and double-click a cell in it to go there.

Some models need such a circle on purpose. Choose **Go Round Circular Formulas** in the **Calculation Mode** menu, and the formulas are worked out again and again, each reading the latest values of the others, until no value changes by more than the amount you give or the number of rounds you allow is used.

## See what a formula reads

Select a cell and click **Show What Feeds This Cell** to draw arrows from the cells its formula reads. Click again to go one step further back. **Show What Uses This Cell** draws arrows to the formulas that read the cell, and each click goes one step on. A small page stands for a cell on another sheet. At most 2,000 arrows are drawn, and the status bar says when there were more. **Clear Arrows** clears them, and so does any change to the workbook.
