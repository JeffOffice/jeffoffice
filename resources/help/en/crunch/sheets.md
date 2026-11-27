# Work with sheets
<!-- keywords: sheet, sheets, tab, tabs, add, insert, new sheet, delete, remove, rename, name, move, copy, duplicate, reorder, drag, hide, unhide, show, tab color, color, select several sheets, Shift+F11, #REF!, 3-D reference, Move or Copy -->

A workbook is made of **sheets**, and the **tabs** at the bottom of the window are their names. Click a tab to show that sheet. Each change below is one step you can undo and is saved in the workbook.

## Add a sheet

- Click the **+** button after the last tab to add a sheet at the end.
- Press **Shift+F11**, or use **Home > Insert > Insert Sheet**, to add a sheet in front of the one you are looking at.

The new sheet is empty, has the next free name (Sheet4, Sheet5, and so on), and opens at once. A workbook can have up to 1,000 sheets; past that, JeffCrunch says so and adds none, because a file with more is not one it opens.

## Rename a sheet

**Double-click** the tab, type the new name, and press **Enter** (**Esc** gives it up). You can also use **Home > Format > Rename Sheet**.

A name can have up to 31 characters. It can't be empty, can't contain any of **: \ / ? * [ ]**, can't begin or end with an apostrophe, and can't be the same as another sheet's name (capital letters do not count as a difference). The name "History" is kept for the program. If the name is refused, JeffCrunch says why and the sheet keeps its name.

Every formula that names the sheet follows the new name. Example: **=Budget!B2** becomes **='Q1 Budget'!B2** when you rename Budget to Q1 Budget. Names that point at the sheet, the rules of conditional formats and validation, and charts follow it too.

## Delete a sheet

Right-click the tab and choose **Delete Sheet**, or use **Home > Delete > Delete Sheet**. If the sheet has anything on it, JeffCrunch asks first. Formulas on other sheets that read the deleted sheet show **#REF!**, which means that what they read is gone. **Undo** brings the sheet and those formulas back.

A workbook always keeps at least one sheet that is shown.

## Move or copy a sheet

- **Drag** a tab to a new place to move the sheet.
- Or use **Home > Format > Move or Copy Sheet**. Choose the sheet the chosen sheet should go in front of (or **(after the last sheet)**), and check **Copy them, and leave the originals where they are** to copy it instead of moving it. A copy is named like **Budget (2)** and opens.

Formulas keep reading the sheets they read, wherever the sheets are. A copy has the same cells, formulas, formats, and sizes as the sheet it came from. A reference with no sheet name (such as **A1**) reads the copy's own cell; a reference that names a sheet (such as **Sheet1!A1**) still names that sheet, even if it is the sheet the copy came from.

A copy also has the original's protection, filter, links, page setup and page breaks, the groups of rows and columns, and the look of its data bars; links follow the rows you inserted or deleted, in the original before the copy and in the copy after it. It does not have the pictures, charts, notes, pivot tables, sparklines, or the names that belong to the original alone, because JeffCrunch does not copy those yet; it tells you which when this happens. A sheet that has tables is copied with copies of them (each with a name, an id, and a part of its own, and the copy's formulas name its own tables). Deleting a sheet takes its tables with it; a formula on another sheet that named one has #REF! then, and Undo brings it back.

To copy a sheet into **another workbook** that is open, choose it in **To workbook** in the same box. The box lists what the copy will not have (for now, the pictures, charts, conditional formats, tables, notes, and how the sheet prints) and says so before anything is copied. Formulas that read other sheets of the first workbook become links to its saved file, so the first workbook needs to have been saved as an .xlsx file at least once (changes made since are fine: the copy shows what the open workbook shows now, and the links see them after the next save); formulas that read the copied sheet itself stay on the copy. Cells that use names, tables, an array, or a spill, or that read a third workbook, come as the values they show, and the box says how many. How the sheet is shown (held rows and columns, zoom, gridlines, and the way it reads) and its default row height and column width come with it. The copy has the same look: formats, fonts, and colors are added to the other workbook (a color from the first workbook's theme is kept as the color it shows when the two themes differ). Both workbooks must count dates from the same year (1900 or 1904). **Undo** in the other workbook takes the copy away; the first workbook is not changed. Moving a sheet to another workbook is not done yet.

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
