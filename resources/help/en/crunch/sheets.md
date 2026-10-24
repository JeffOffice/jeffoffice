# Work with sheets
<!-- keywords: sheet, sheets, tab, tabs, add, insert, new sheet, delete, remove, rename, name, move, copy, duplicate, reorder, drag, hide, unhide, show, tab color, color, select several sheets, Shift+F11, #REF!, 3-D reference, Move or Copy -->

A workbook is made of **sheets**, and the **tabs** at the bottom of the window are their names. Click a tab to show that sheet. Each change below is one step you can undo and is saved in the workbook.

## Add a sheet

- Click the **+** button after the last tab to add a sheet at the end.
- Press **Shift+F11**, or use **Home > Insert > Insert Sheet**, to add a sheet in front of the one you are looking at.

The new sheet is empty, has the next free name (Sheet4, Sheet5, and so on), and opens at once.

## Rename a sheet

**Double-click** the tab, type the new name, and press **Enter** (**Esc** gives it up). You can also use **Home > Format > Rename Sheet**.

A name can have up to 31 characters. It can't be empty, can't contain any of **: \ / ? * [ ]**, can't begin or end with an apostrophe, and can't be the same as another sheet's name (capital letters do not count as a difference). The name "History" is kept for the program. If the name is refused, JeffCrunch says why and the sheet keeps its name.

Every formula that names the sheet follows the new name. Example: **=Budget!B2** becomes **='Q1 Budget'!B2** when you rename Budget to Q1 Budget. Names that point at the sheet, the rules of conditional formats and validation, and charts follow it too.

## Delete a sheet

Right-click the tab and choose **Delete Sheet**, or use **Home > Delete > Delete Sheet**. If the sheet has anything on it, JeffCrunch asks first. Formulas on other sheets that read the deleted sheet show **#REF!**, which means that what they read is gone. **Undo** brings the sheet and those formulas back.

A workbook always keeps at least one sheet that is shown.

## Move or copy a sheet

- **Drag** a tab to a new place to move the sheet.
- Or use **Home > Format > Move or Copy Sheet**. Choose the sheet it should go in front of (or **move to end**), and check **Create a copy** to copy it instead of moving it. A copy is named like **Budget (2)** and opens.

Formulas keep reading the sheets they read, wherever the sheets are. A copy has the same cells, formulas, formats, and sizes as the sheet it came from. A reference with no sheet name (such as **A1**) reads the copy's own cell; a reference that names a sheet (such as **Sheet1!A1**) still names that sheet, even if it is the sheet the copy came from.

A copy does not have the pictures, charts, and notes of the original, because JeffCrunch does not show those yet; it tells you so when this happens. A sheet that has a table on it can't be copied or deleted yet.

## Hide and show a sheet

Right-click the tab and choose **Hide Sheet** (or use **Home > Format > Hide Sheet**). The sheet is not shown, but it is still in the workbook and formulas still read it. To show it again, right-click any tab and choose **Unhide Sheet**, then click its name in the list. The list is empty when no sheet is hidden.

## Color a tab

Right-click the tab and choose **Tab Color**, then pick a color, or choose **No Color**. The color shows as a line under the name. A color you pick from the theme's palette is saved as the plain color it is now.

## Several sheets at once

**Ctrl+click** tabs to pick more than one, or click one tab and **Shift+click** another to pick all the tabs between. The picked tabs are shaded. **Delete**, **Hide**, **Tab Color**, and **Move or Copy** then work on all of them as one step. Click a tab that is not picked to pick that one alone.

What you type or format while several sheets are picked changes only the sheet you are looking at.

## What can't be done

- A workbook that has a formula over a block of sheets, such as **=SUM(Sheet1:Sheet3!A1)**, can't have sheets added, deleted, renamed, or moved yet, because that would change which sheets the formula covers.
- After you save, **Undo** can't go back past a sheet you added, copied, deleted, renamed, or moved before the save. The saved file has them as they are. Hiding, showing, and the tab color can still be undone.
