# Find, replace, and go to
<!-- keywords: find, replace, change, search, wildcard, star, question mark, tilde, same capitals, whole cell, go to, reference, name, select by kind, blanks, constants, formulas, errors, current region, last cell, visible cells, conditional formats, validation, Ctrl+F, Ctrl+H, F5, notes -->

## Find

Press **Ctrl+F** (or choose **Find** on the Home tab, then **Find** again). Type the text and press **Enter** for the next cell that has it, or choose **List All** to get a list of every cell that matches. Click a line of the list to go to that cell.

| Option | What it does |
| --- | --- |
| **Where** | Search this sheet, or every sheet that shows. With more than one cell selected, a search of one sheet looks only in them. |
| **Order** | Visit the cells along rows (left to right, then down) or down columns (top to bottom, then right). **Next** stops at the first match in that order. |
| **Search in** | **Formulas** looks in what is typed in each cell, **Values** in what it shows, and **Notes** in the notes on cells. |
| **Same capitals only** | Capital and small letters must be the same. |
| **Whole cell only** | The whole cell must be the text, not just a part of it. |

In the text you look for, **?** stands for any one character and **\*** for any stretch of characters (including none). Type **~** in front of ?, \*, or ~ to look for the mark itself: **a~\*b** matches a\*b.

A search runs a little at a time, so you can keep working. **Esc** (or **Cancel**) stops it, and so does changing a cell. A very long text costs more of each little step than a short one, so a sheet of long texts still leaves the window free.

Notes on cells follow the cells: after a sort, or after rows or columns are inserted or deleted, a search of notes finds each note at the cell it is on now. A note on a cell that was deleted is gone from the search (and is back after Undo). If a workbook has been changed in so many ways that the notes can't be followed through all of them, you are asked to save it first; a workbook opened from an older kind of file or from a text file has its notes in a form JeffCrunch does not search, and you are told so.

## Replace

Press **Ctrl+H**. **Change** puts the new text in the cell with the cursor if it has the text, and goes on to the next one. **Change All** does it in every cell that has it, as one step you can undo; a large change is put in a little at a time. The new text is entered as if you typed it: a number becomes a number, and a formula that is changed stays a formula.

Nothing is changed, and you are told why, if a cell that would be changed is locked on a protected sheet, is part of an array or a data table, is in the first row of a table, or would not be a formula any more.

## Go to

Press **F5** (or **Ctrl+G**), or type in the Name box. You can give a cell such as B7, a range such as B3:D9, whole columns or rows (C:E, 4:9), a cell of another sheet (Sheet2!A1, or 'My sheet'!A1), or the name of a range.

## Select by kind

Choose **Select by Kind** in the **Find** menu to select cells by what they are:

- **Blank cells**, **Constants** (typed in), **Formulas**, and **Cells that show an error**. For constants and formulas you can leave out numbers, text, TRUE and FALSE, or errors.
- **The block around the cell with the cursor**, and **The last cell that is used**.
- **Cells that show** (not in hidden rows or columns), **Cells with conditional formatting**, and **Cells with a rule for what may be entered** (all of them, or only those with the same rule as the cell with the cursor).

The first block found is selected and any others are tinted; the tint goes away when you click a cell.
