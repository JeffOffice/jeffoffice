# How formulas are shown and calculated
<!-- keywords: formula, formulas, calculate, calculation, recalculate, function, functions, value, result, stored, F9, circular, show formulas, formula bar, error, point, pointing, click a cell, reference, F4, error checking, ignore error, evaluate formula, step, watch window, watch, let, name a value, cell function, info function, filterxml, xml, xpath-->

A **formula** is a cell that works out its value from other cells, such as =SUM(B3:B8). The cell shows the value, and the **formula bar** shows the formula.

## What JeffCrunch does with them

When you open a workbook, the file holds the last value of every formula, so the sheet can be shown at once. Then JeffCrunch works every formula out again, a few at a time, so the window stays quick. The status bar says **Calculating** with a percent while it does. When it is done, the cells hold JeffCrunch's own results. To work everything out again, click **Recalculate All** on the **Formulas** tab, or press **F9**.

## When JeffCrunch can't work a formula out

JeffCrunch has more than 400 functions, and it is still growing. A formula that uses a function JeffCrunch doesn't have yet keeps the value the file stored for it. So does one that refers to another workbook. Cells that depend on those are worked out from the stored values.

## Functions that need a few notes

- **Bonds and loans.** PRICE, YIELD, DURATION, ODDFPRICE, and the other bond functions count days the way the **basis** you give says (0 is 30 days to a month in a 360-day year, 1 is the real days, and so on). YIELD, ODDFYIELD, ODDLYIELD, and RATE find their answer by trying values until the price or payment fits. They stop when the answer is right to about 15 digits, so the last digits may differ from another program's.
- **Distributions and tests.** The functions that end in .DIST, .INV, .TEST, and the older names such as NORMDIST and TDIST, give the same answers: an older name is kept so that old files still work. The *.INV functions are found by trying values too.
- **Lists that fill several cells.** SORT, SORTBY, FILTER, UNIQUE, SEQUENCE, RANDARRAY, XLOOKUP, MMULT, MINVERSE, and LINEST give a block of values. Enter them as an array formula (**Ctrl+Shift+Enter**) over the cells they should fill. Letting a formula spill into the cells next to it is not done yet.
- **Texts in other scripts.** LENB, LEFTB, MIDB, RIGHTB, FINDB, SEARCHB, and REPLACEB count each character as one, as they do on a computer that is not set up for double-byte text. PHONETIC gives the text of the cell, because JeffCrunch does not keep the readings of Japanese text.
- **CONVERT.** Units are written as text, with capital and small letters as shown in the function's list: "kg", "lbm", "mi", "F". A metric prefix can go in front of a unit that takes one, such as "km" or "mL".

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

Each address in the formula is coloured in the text, and the cells it names are outlined on the sheet in the same colour, so you can see at a glance what the formula reads. A click after a whole address, such as =A1, ends the entry and selects the cell you clicked, as it does for any other text.

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

## Error Checking

Choose **Error Checking** in the **Check** group of the Formulas tab to look through the sheet for cells that may be wrong. It looks for two things: a formula that comes to an error (such as #DIV/0!), and a number that is kept as text, which formulas may skip.

The dialog shows one cell at a time, goes to that cell on the sheet, and says in a sentence what is the matter. **Next** and **Previous** move through the cells. A small green triangle in the upper-left corner of a cell marks it on the sheet. **Ignore Error** takes the cell off the list, takes its triangle away, and is saved in the file. One step of Undo brings it back. If nothing is wrong, you are told so.

## Evaluate Formula

Use this to see how a formula comes to its value. Select a cell that holds a formula and choose **Evaluate Formula**. The formula shows with the next part underlined. Press **Evaluate** to work that part out; its value takes its place. The parts go in this order: each single cell from the left, then each function with its brackets, starting with the innermost, then what is left. **Restart** begins again. If a part can't be worked out, the dialog says why and leaves the formula as it was.

## Watch Window

The **Watch Window** is a small window that stays in view while you work. It lists cells you want to keep an eye on, with their sheet, cell, value, and formula, even when they are on another sheet. Select cells and press **Add Watch**. Select rows in the list and press **Delete Watch**. Double-click a row to go to the cell. The values change when the sheet is worked out again. The list is kept while the workbook is open.

## Give names to values inside a formula

**LET** names a value in the middle of a formula, so you write it once and use it again. The pairs of a name and a value come first, and the calculation comes last. In =LET(price, B2, tax, price*0.08, price+tax) the name **price** stands for the value in B2, **tax** is worked out from it, and the last part adds them. A name can use the names before it. A name works only inside its own LET, and a LET inside another may use a name again. A name can stand for a block of cells too, as in =LET(rows, B2:B9, SUM(rows)/COUNT(rows)).

## Ask about a cell or the workbook

**CELL** tells something about a cell. Its first argument says what: "address", "row", "col", "contents", "type", "format" (how numbers show, such as "F2" for two decimals, "P0" for a percent, or "D1" for a date), "colour" (1 when negative numbers show in colour), "parentheses" (1 when positive numbers show in brackets), "prefix" (the mark of a text that is aligned left, right, or centre), "protect" (1 when the cell is locked), "width" (the column's width in characters), and "filename" (the folder, the file, and the sheet, such as /home/me/[Budget.xlsx]Sheet1; empty until the workbook is saved). **INFO** tells about the program and the workbook: "directory", "numfile" (how many sheets show), "origin", "osversion", "recalc" (Automatic or Manual), "release", and "system".

## Pick values out of XML

**FILTERXML** takes text that holds XML and an XPath, which is a short way to say which parts you want, and gives the values it finds. =FILTERXML(A1, "//item/price") gives every price in the text in A1, one under another. Use "/shop/item[2]/name" for the second item, "//item[@id='b']/name" for the item whose id is b, or "count(//item)" for how many there are. If the text is not valid XML, or the path is not one, the answer is #VALUE!. If nothing matches, it is #N/A. XML that has a document type is refused.
