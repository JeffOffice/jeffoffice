# Change the view
<!-- keywords: outline, group, ungroup, collapse, expand, detail, level, view, zoom, right-to-left, right to left, Arabic, Hebrew, mirrored, freeze, freeze panes, unfreeze, split, split panes, panes, split bar, gridlines, headings, formula bar, collapse the ribbon, full screen, status bar, minimum, maximum, show formulas, sheet tabs, hidden sheets, headers, new window, arrange all, side by side, synchronous scrolling, switch windows, custom views, saved view, named view -->

The **View** tab changes how the sheet looks on your screen. It never changes the workbook.

## Views

The **Views** group of the View tab (and the three buttons at the bottom right of the window) change what the sheet shows. **Normal** is the cells as they are. **Page Layout** shows the pages as they print, with their header and footer. **Page Breaks** shades what does not print, numbers the pages, and lets you drag the lines where pages begin. See [Set up the pages](page-layout) for the last two. Each sheet remembers its view, and the workbook saves it.

## Custom views

A **custom view** is a name for the way the workbook looks right now. It keeps which sheet is in front, which rows and columns are hidden, the zoom, the frozen rows and columns, the cell chosen in each sheet, the gridlines and headings, and the size of the window. You can have a view for reading and a view for checking, and switch between them in one click.

On the **View** tab, click **Custom Views**. To keep the look you have now, type a name and click **Add**. If a view already has that name, the new look takes its place. To use a view, click its name and then **Show**. Showing a view is one step, so **Undo** puts everything back: the rows and columns, the sheet in front, the zoom, the frozen panes, the cell, the gridlines, the headings, the formula bar, and the size of the window. Rows that a filter hides are left to the filter. To remove a view, click it and then **Delete**.

The views are saved with the workbook. A view does not keep a filter, only the rows you hid by hand. It does not move when you insert or delete rows or columns, so make the view again after you do. If the workbook has more runs of hidden rows and columns than a view can keep (20,000), **Add** tells you so and keeps nothing. When you rename a sheet the views follow it, and when you delete one it leaves the views. Views that other programs saved are listed, and they show the sheet and the window size.

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

## Split the view into panes

**Split** cuts the sheet into two or four **panes**. A pane is a part of the window that shows the sheet. Each pane scrolls on its own, so you can look at the top of a long list and the bottom of it at the same time.

1. Select the cell where the cut should be. The bars are put at the top and the left of that cell. A cell in the first row you can see gives only a vertical bar. A cell in the first column you can see gives only a horizontal bar.
2. On the **View** tab, click **Split**.

The wheel of the mouse moves the pane under the pointer. The scroll bars move the pane at the lower right. Drag a bar to move it. Double-click a bar to take that bar away, or double-click where the two bars cross to take both away. Click **Split** again to take the bars away.

Panes that are frozen and panes that are split are different, so a sheet has one kind at a time. When you click **Freeze Panes** on a split sheet, the split goes away first. When you click **Split** on a frozen sheet, the frozen panes are let go first.

The bars are saved with the workbook, so the split is there again when the workbook is opened. The pane at the lower right can scroll back over the rows and columns that come before the bars. It can not go back further than the first row and column that the pane at the top left shows.

## Sheets

The tabs at the bottom are the sheets of the workbook. Click a tab to show that sheet. Each sheet remembers where you were in it. Sheets that are hidden are not shown. A colored line under a tab is the color of the sheet's tab. To add, rename, move, copy, hide, delete, or color a sheet, see [Work with sheets](sheets).

## Windows

On the **View** tab, in the **Window** group:

- **New Window** opens the workbook's file in a second window, for reading. You can look at two parts of one workbook at once. The second window cannot change the file, so the two never disagree. Save the workbook first, so the new window shows what you have now.
- **Arrange All** puts every open JeffCrunch window side by side, each in its own column of the screen.
- **View Side by Side** puts this window and another one on the two halves of the screen. While **Synchronous Scrolling** is on, scrolling one window scrolls the other the same distance. Turn that off to scroll them apart. Click **View Side by Side** again to let them go. **Reset Window Position** puts the two halves back if you moved the windows.
- **Switch Windows** lists the open windows. Click one to bring it to the front.

## Sheets that read from right to left

A sheet made for a language such as Arabic or Hebrew reads from right to left, and JeffCrunch shows it that way: column A is at the right edge, the row numbers are at the right, and the horizontal scroll bar runs the other way. The arrow keys follow the sheet, so the Left arrow goes to the next column. Text in a cell starts from the side its language reads from. The workbook says which way a sheet reads, and you can change it: on the **Page Layout** tab, click **Sheet Right-to-Left**. Click it again to turn it off. The choice is saved with the sheet.

## Group rows and columns (outline)

On the **Data** tab, **Group Rows** gives the selected rows a level more, and **Ungroup Rows** a level less; **Group Columns** and **Ungroup Columns** do the same for columns. Rows can be grouped inside groups, down to seven levels. Beside the rows (and above the columns) the sheet shows a bracket for each group and, on the line that sums the group up (the line below the rows, or to the right of the columns, unless your file puts them above or to the left), a button: **minus** hides the group's lines, and **plus** shows them again. The numbered buttons at the corner show the outline down to a level: **1** shows only the lines that are in no group, **2** the lines of the first level too, and so on. A group inside a hidden group stays hidden when the outer group is shown, until its own button is clicked. Each of these is one step of Undo, and the hidden lines and the groups are saved with the workbook. Ungrouping a hidden group shows its lines.

**View > Outline Symbols** turns the brackets and buttons off and on.

## Show formulas

On the **Formulas** tab, **Show Formulas** (Ctrl+`; on a Mac, the Control key, not the Command key) shows the formula in every cell that has one, in place of its value. The columns that hold formulas are made wider, up to a limit, so that the formulas show whole, and go back to their widths when you turn it off; the widths in your file are not changed.
