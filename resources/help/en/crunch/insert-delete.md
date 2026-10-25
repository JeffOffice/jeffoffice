# Insert and delete rows, columns, and cells
<!-- keywords: insert, delete, remove, row, column, cell, shift, move, add, blank, Ctrl+plus, Ctrl+minus, right-click, header, undo, reference, #REF!, formula, name, merged, table, array, edge -->

You can add empty rows, columns, or cells in the middle of your data, and take rows, columns, or cells out. Everything below moves over to make room or to close the gap, and the formulas that point at the cells that moved keep pointing at them.

## Insert

1. Select the row numbers, the column letters, or the cells where you want the new ones. To add three rows, select three rows.
2. Press **Ctrl and the plus key** (**Ctrl and the minus key** deletes), or use **Home > Insert**, or right-click the selection and choose **Insert**.

With whole rows or columns selected, the new ones go in at once, above the rows or to the left of the columns. With cells selected, a small window asks how the cells after them should move:

- **Shift cells right** or **Shift cells down** moves only the cells in the columns (or rows) of your selection, and leaves the rest of the sheet where it is.
- **Entire row** or **Entire column** adds whole rows or columns.

## Delete

Select what you want to take out and press **Ctrl and the minus key**, or use **Home > Delete**. With cells selected, the window asks whether to **shift cells left**, **shift cells up**, or delete the **entire row** or **entire column**. Deleting is one step, and **Undo** puts back everything it took out, including the formulas.

## What follows

A formula that points at cells that move points at them in their new places. Example: in B1 you have **=SUM(A1:A5)**. Insert a row above row 3, and the formula becomes **=SUM(A1:A6)**, because the range grew to hold the new row. If you delete row 3, it becomes **=SUM(A1:A4)**. A formula that pointed at a cell you deleted shows **#REF!**, which means that the cell it needs is gone. **Undo** brings the cell and the formula back.

These follow the cells too: formulas on every sheet, names (such as print areas and print titles), merged cells, the cells that rules of validation and conditional formats are for, tables, charts, pictures, notes, hyperlinks, and page breaks.

## What can't be done

- Cells can't be inserted or deleted through part of an **array formula** or a data table. Move them whole, or leave them alone. A cell that a data table reads for its input can't be deleted either.
- A **pivot table** can't be moved or cut through yet: rows, columns, or cells can't be inserted or deleted where that would move it or change it, and cells can't be cut out of it or onto it. The rest of the sheet can be changed as usual.
- A **table** can't be cut across: you can't insert a column inside it, and you can't delete its header row or all of its rows. Insert or delete rows inside it, or change the cells around it.
- A **protected sheet** can't have rows, columns, or cells inserted or deleted. To do that, choose Unprotect Sheet on the Review tab.
- Cells at the end of the sheet that hold something can't be pushed off the edge. Delete them first.
- After you save, **Undo** still goes back over what you did before, rows and columns you inserted included (the next save has them taken out again). The one thing it cannot give back is rows, columns, or cells you **deleted**: the saved file has lost what was in them, so **Undo** stops at the last delete you saved, and the line at the bottom of the window says so when you save.
- A step that moves or deletes so many cells that **Undo** could not hold it (more than about 128 MB) asks first, as the spreadsheet programs people know do: "This can't be undone." If you go on, it is done and **Undo** does not go back over it or what came before it.
