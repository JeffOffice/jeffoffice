# Find, replace, and go to
<!-- keywords: find, replace, search, wildcard, star, question mark, tilde, match case, entire cell, go to, reference, name, special, blanks, constants, formulas, errors, current region, last cell, visible cells, conditional formats, validation, Ctrl+F, Ctrl+H, F5, notes -->

## Find

Press **Ctrl+F** (or choose **Find and Select** on the Home tab, then **Find**). Type the text and press **Enter** for the next cell that has it, or choose **Find All** to get a list of every cell that does. Click a line of the list to go to that cell.

| Option | What it does |
| --- | --- |
| **Within** | Look in this sheet, or in every sheet that shows. With more than one cell selected, a sheet search looks only in them. |
| **Search** | Go through the cells by rows (left to right, then down) or by columns (top to bottom, then right). |
| **Look in** | **Formulas** looks in what is typed in each cell, **Values** in what it shows, and **Notes** in the notes on cells. |
| **Match case** | Capital and small letters must be the same. |
| **Match entire cell contents** | The whole cell must be the text, not just a part of it. |

In the text you look for, **?** stands for any one character and **\*** for any run of characters (including none). Put **~** before ?, \*, or ~ to look for the mark itself: **a~\*b** finds a\*b.

A search runs a little at a time, so you can keep working. **Esc** (or **Cancel**) stops it, and so does changing a cell.

## Replace

Press **Ctrl+H**. **Replace** changes the cell with the cursor if it has the text, and goes on to the next one. **Replace All** changes every cell that has it, as one step you can undo. The new text is entered as if you typed it: a number becomes a number, and a formula that is changed stays a formula.

Nothing is changed, and you are told why, if a cell that would be changed is locked on a protected sheet, is part of an array or a data table, is in the first row of a table, or would not be a formula any more.

## Go to

Press **F5** (or **Ctrl+G**), or type in the Name box. You can give a cell such as B7, a range such as B3:D9, whole columns or rows (C:E, 4:9), a cell of another sheet (Sheet2!A1, or 'My sheet'!A1), or the name of a range.

## Go to Special

Choose **Go To Special** in the **Find and Select** menu to select cells by what they are:

- **Blank cells**, **Constants** (typed in), **Formulas**, and **Cells that show an error**. For constants and formulas you can leave out numbers, text, TRUE and FALSE, or errors.
- **The block around the cell with the cursor**, and **The last cell that is used**.
- **Cells that show** (not in hidden rows or columns), **Cells with conditional formatting**, and **Cells with a rule for what may be entered** (all of them, or only those with the same rule as the cell with the cursor).

The first block found is selected and any others are tinted; the tint goes away when you click a cell.
