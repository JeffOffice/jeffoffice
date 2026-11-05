# Names for cells and values
<!-- keywords: name, names, defined name, new name, named range, scope, comment, name from cells, insert name, apply names, Ctrl+F3, refers to, sort names, error name, warning-->

A **name** is a word that stands for a cell, a block of cells, a number, or a formula, so that =Price*Quantity can be written in place of =B2*C2. A name belongs to the whole workbook, or to one sheet. A name on a sheet hides a name of the workbook that is spelled the same, on that sheet.

## The rules for a name

- It begins with a letter, an underscore, or a backslash, and has only letters, digits, underscores, and dots after that. It has no spaces.
- It is at most 255 characters long.
- It is not the address of a cell, such as **A1** or **R2C3**.
- No other name of the same sheet (or of the workbook) is spelled the same, in capitals or not.

While you type, the dialog says what is wrong, and the **OK** button waits.

## Make a name

1. Select the cells, and click **New Name** on the **Formulas** tab.
2. Type the name, choose where it works, and say what it stands for. The cells you selected are filled in for you.

There is a quicker way for a name of the whole workbook. Select the cells, click the **Name box** (at the left of the formula bar), type the name, and press **Enter**. If no name is spelled like that yet, it is made for the cells you selected. If there is one, **Enter** goes to its cells.

**Name From Cells** makes many names at once from the labels round a block: with the headings in the top row and the numbers under them, each heading names the cells under it. Choose where the labels are (top row, left column, bottom row, or right column). A label that has spaces or marks a name can't have gets underscores in their place. A label that is empty, repeats another, or is a name the workbook has already is left out, and the status bar says how many.

## Look at all the names

Click **Names** on the **Formulas** tab, or press **Ctrl+F3**. The list shows each name, the value it has now, what it stands for, where it works, and its comment. Type in the filter to show only the names that fit. **New**, **Change**, and **Delete** make a name, change one, and take one away. Click a heading to sort the list by it, and click again to turn the order round. A name that points to an error, such as a place that was deleted, has a small warning sign beside it. A workbook with more than 20,000 names lists the first 20,000.

Changing the name of a name changes it in every formula that uses it, on every sheet, in one step. Taking a name away leaves the formulas that used it showing **#NAME?**. Each change can be undone.

## Put a name in a formula

While you type a formula, the names that fit show in the list under the cell. **Insert Name** on the **Formulas** tab lists the names to choose from.

## Put names in formulas that are already written

Say you made the name Rate for $A$1 after many formulas already read $A$1. Click **Apply Names** on the **Formulas** tab to write Rate in their place. Select the cells to look at first, or select one cell to look at the whole sheet. Choose the names to put in. A reference is changed only when it is exactly the cells of the name, with or without the dollar signs: A1:B2 is not changed to a name for A1. Only names for one block of cells on a sheet are offered, and only names of the whole workbook. The change is one step, so **Undo** takes it back.
