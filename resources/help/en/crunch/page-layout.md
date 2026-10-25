# Set up the pages
<!-- keywords: page layout, margins, orientation, portrait, landscape, paper, size, print area, page break, breaks, print titles, titles, repeat rows, scale to fit, fit to page, gridlines, headings, sheet options -->

The **Page Layout** tab decides how a sheet is cut into pages when it is printed or made into a PDF. Each sheet keeps its own choices, and they are saved in the workbook. Every change is one step you can undo.

## Margins, orientation, and size

- **Margins** sets the blank space around the cells. **Standard**, **Wide**, and **Narrow** are ready-made sets. **Custom Margins** opens the Page Setup dialog, where you can type each one, and say how far the header and footer sit from the edge of the paper.
- **Orientation** is **Portrait** (the paper is taller than wide) or **Landscape** (wider than tall).
- **Size** is the paper: Letter, Legal, A4, and others. **More Paper Sizes** lists them all.

## Print area

By default a sheet prints everything that has something in it: values, formulas, and cells with a visible border or fill, and any chart, picture, or shape. To print only part:

1. Select the cells you want.
2. Choose **Print Area > Set Print Area**.

**Add Selection to Print Area** puts a second block of cells in the print area. Each block prints on pages of its own. **Clear Print Area** goes back to printing everything. Rows or columns you insert or delete inside the print area change it along with the cells.

## Page breaks

Where one page ends and the next begins is worked out from the paper, the margins, and the sizes of your rows and columns: a row or column is never cut in half. To end a page earlier, select the cell below and to the right of where the break should go, and choose **Breaks > Insert Page Break**. With a whole row selected the break goes above it, and with a whole column selected it goes to its left. **Remove Page Break** takes away the break above or to the left of the selected cell, and **Reset All Page Breaks** takes away every break you set. Page breaks you set are ignored while a sheet is being shrunk to fit.

## The Page Breaks view

**View > Page Breaks** (or the button at the bottom right of the window) shows the sheet cut into the pages it prints on. The cells that print stay as they are, everything else is shaded, and each page has its number over its cells. A dashed line is a break JeffCrunch worked out, and a solid line is one you set.

- **Move a break.** Drag a line to another row or column. It becomes a break you set there, and the pages after it are cut again.
- **Remove a break.** Drag a line off the printed cells, onto the row numbers or column letters.
- **Change the print area.** Drag the blue edge of the printed cells. If you have not set a print area, the cells that print on their own become one, with that edge moved.

Each drag is one step of Undo. While a sheet is being shrunk to fit, its breaks are ignored, so the lines cannot be dragged. **Normal** brings back the cells as they were. The view each sheet was left in is saved with the workbook.

## The Page Layout view

**View > Page Layout** (or the middle button at the bottom right of the window) shows the sheet as the pages it prints on, one below the other, each as a sheet of paper with its margins and a gap between pages. Rulers run along the top and down the left. The header and footer are drawn where they print, and so are pictures in them.

- **Work in the cells.** Click a cell to choose it, drag to choose several, and use the arrow keys, typing, F2, and a double click as you do in the Normal view. What you type goes in a strip over the cell on its page.
- **Type a header or footer.** Move the pointer over the top or bottom margin of a page: its left, middle, and right parts light up. Click one and type. The **Header and Footer** tab appears, with buttons that put in the page number, the number of pages, the date, the time, the file's folder and name, the sheet's name, and a picture; its choices give the first page, and the odd and even pages, headers and footers of their own, and say whether the header grows with the cells and lines up with the margins. Press **Ctrl+Enter**, or click anywhere else, to finish; **Esc** gives it up. Each finished header or footer is one step of Undo.
- **Zoom** with the slider, or hold **Ctrl** and turn the mouse wheel.

Only the pages in view are drawn, so a sheet of thousands of pages opens at once. The view each sheet was left in is saved with the workbook.

## Print titles

**Print Titles** repeats rows at the top, and columns at the left, of every page after the one they are on, so that a heading row is on every page. Type the rows (such as **1:3**) and the columns (such as **A:B**) on the Sheet tab of the Page Setup dialog.

## Scale to fit

**Width** and **Height** shrink the cells until the sheet is no wider, or no taller, than that many pages. **Automatic** lets that direction take as many pages as it needs. **Scale** prints at a percent of full size, from 10 to 400, and works while Width and Height are both **Automatic**. If shrinking to fit would make the letters very small, the Print screen says so.

## Sheet options

**Show Gridlines** and **Show Headings** change the screen only. **Print Gridlines** puts the lines between cells on paper, and **Print Headings** puts the row numbers and column letters on paper.

## More choices

The small arrow at the corner of the **Page Setup** group opens the Page Setup dialog, which has all of these and more: [The Page Setup dialog](page-setup).
