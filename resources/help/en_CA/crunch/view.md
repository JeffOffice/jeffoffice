# Change the view
<!-- keywords: outline, group, ungroup, collapse, expand, detail, level, view, zoom, right-to-left, right to left, Arabic, Hebrew, mirrored, freeze, freeze panes, unfreeze, gridlines, headings, formula bar, collapse the ribbon, full screen, status bar, minimum, maximum, show formulas, sheet tabs, hidden sheets, headers -->

The **View** tab changes how the sheet looks on your screen. It never changes the workbook.

## Views

The **Views** group of the View tab (and the three buttons at the bottom right of the window) change what the sheet shows. **Normal** is the cells as they are. **Page Layout** shows the pages as they print, with their header and footer. **Page Breaks** shades what does not print, numbers the pages, and lets you drag the lines where pages begin. See [Set up the pages](page-layout) for the last two. Each sheet remembers its view, and the workbook saves it.

## Zoom

Use the slider at the bottom right, the **Zoom** button on the View tab, or hold **Ctrl** and turn the mouse wheel. You can zoom from 10% to 400%. **100%** puts it back to normal size.

## Show and hide

In the **Show** group, turn the **Gridlines** (the pale lines between cells), the **Headings** (the column letters and row numbers), and the **Formula Bar** on or off. A workbook can ask for its gridlines or headings to be off; this choice is yours to change.

Two more choices are in the same group. **Collapse the Ribbon** (**Ctrl+F1**) folds the ribbon up so that it shows only the tab names, which leaves more room for the cells. Click it again, or click a tab, to bring it back. **Full Screen** (**Ctrl+Shift+F1**) makes the window fill the whole screen. Click it again to go back.

## The numbers in the status bar

When you select two or more cells, the status bar shows numbers about them. **Right-click** the numbers to choose which are shown: **Average**, **Count** (the cells that hold anything), **Numerical Count** (only the cells that hold numbers), **Minimum** (the smallest number), **Maximum** (the largest number), and **Sum**. A check mark shows the ones that are on. At least one stays on. JeffCrunch remembers your choice.

## Freeze rows and columns

**Frozen** rows and columns stay in place while the rest of the sheet scrolls, so a heading row is always in view. On the **View** tab, click **Freeze Panes**:

- **Freeze Panes** freezes the rows above the selected cell and the columns to its left, from the first row and column you can see. Rows and columns that you scrolled out of sight stay out of sight until you unfreeze.
- **Freeze Top Row** freezes the top row you can see.
- **Freeze First Column** freezes the first column you can see.
- **Unfreeze Panes** puts them back, and the sheet stays where it is.

The frozen rows and columns are saved with the workbook, so they are held again when it is opened.

A workbook that was saved with frozen panes opens with them.

## Sheets

The tabs at the bottom are the sheets of the workbook. Click a tab to show that sheet. Each sheet remembers where you were in it. Sheets that are hidden are not shown. A coloured line under a tab is the colour of the sheet's tab. To add, rename, move, copy, hide, delete, or colour a sheet, see [Work with sheets](sheets).

## Sheets that read from right to left

A sheet made for a language such as Arabic or Hebrew reads from right to left, and JeffCrunch shows it that way: column A is at the right edge, the row numbers are at the right, and the horizontal scroll bar runs the other way. The arrow keys follow the sheet, so the Left arrow goes to the next column. Text in a cell starts from the side its language reads from. The workbook says which way a sheet reads, and you can change it: on the **Page Layout** tab, click **Sheet Right-to-Left**. Click it again to turn it off. The choice is saved with the sheet.

## Group rows and columns (outline)

On the **Data** tab, **Group Rows** gives the selected rows a level more, and **Ungroup Rows** a level less; **Group Columns** and **Ungroup Columns** do the same for columns. Rows can be grouped inside groups, down to seven levels. Beside the rows (and above the columns) the sheet shows a bracket for each group and, on the line that sums the group up (the line below the rows, or to the right of the columns, unless your file puts them above or to the left), a button: **minus** hides the group's lines, and **plus** shows them again. The numbered buttons at the corner show the outline down to a level: **1** shows only the lines that are in no group, **2** the lines of the first level too, and so on. A group inside a hidden group stays hidden when the outer group is shown, until its own button is clicked. Each of these is one step of Undo, and the hidden lines and the groups are saved with the workbook. Ungrouping a hidden group shows its lines.

**View > Outline Symbols** turns the brackets and buttons off and on.

## Show formulas

On the **Formulas** tab, **Show Formulas** (Ctrl+`; on a Mac, the Control key, not the Command key) shows the formula in every cell that has one, in place of its value. The columns that hold formulas are made wider, up to a limit, so that the formulas show whole, and go back to their widths when you turn it off; the widths in your file are not changed.
