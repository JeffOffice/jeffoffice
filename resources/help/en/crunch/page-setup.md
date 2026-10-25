# The Page Setup dialog
<!-- keywords: page setup, dialog, header, footer, page numbers, date, first page, odd even, black and white, draft, notes, errors, page order, center on page, start page numbers, header and footer codes -->

Open it from the small arrow at the corner of the **Page Setup** group on the **Page Layout** tab. It has four tabs. **OK** changes the sheet you are on, as one step you can undo.

## Page

- **Which way the paper turns**: tall or wide.
- **How big the cells print**: a percent of full size, or **Shrink until it fits**, with how many pages across and down. "Any number of" lets that direction take as many pages as it needs.
- **Paper**: the size of the paper.
- **Start the page numbers at**: the number the first page gets. Without it, the first page is 1, and a workbook printed whole numbers its pages on from one sheet to the next.

## Margins

Type the space around the cells, and how far the header and footer sit from the top and bottom edges of the paper. The boxes use centimeters where your computer is set to a metric region, and inches elsewhere. **Across the page** and **Down the page** put the cells in the middle of the page.

## Header and Footer

A **header** is a line of words at the top of every page, and a **footer** is one at the bottom. Pick one from the list, or choose **Edit header** or **Edit footer** to write your own. Each has a left, a middle, and a right part, and buttons that put in the page number, the number of pages, the date, the time, the file name, the folder, and the sheet name. These change from page to page, so they are worked out when the page prints. To put a picture in a part, click in that part and choose **Picture…**, then pick a PNG, JPEG, GIF, or BMP file. The picture is made no more than two inches on its longest side, and it prints where the code `&G` stands. **Remove Picture** takes it out. A part holds one picture, and the first page and the even pages can each have pictures of their own.

You can also set a different header and footer for the first page, and for odd and even pages, and choose whether the letters grow and shrink with the cells and whether the header and footer line up with the margins.

Behind the buttons are short codes that the file keeps, and you can type them yourself: **&P** is the page number, **&N** the number of pages, **&D** the date, **&T** the time, **&F** the file name, **&A** the sheet name, and **&&** is an ampersand. **&B**, **&I**, and **&U** turn bold, italic, and underline on and off, **&12** sets the size to 12 points, and **&"Arial,Bold"** picks a font. **&L**, **&C**, and **&R** begin the left, middle, and right parts.

## Sheet

- **Cells to print**: the print area, as blocks such as **A1:D20, F1:G9**. Leave it empty to print everything.
- **Repeat these rows at the top** and **Repeat these columns at the left**: the print titles, such as **1:3** and **A:B**.
- **Also print**: the lines between cells, the row numbers and column letters, black and white only (no colors or fills), and a quick draft (no lines between cells, and no pictures, charts, or shapes).
- **Notes** and **Cells that show an error**: what to do with them. The setting for errors changes what the page shows: as the cell shows it, nothing, two dashes, or #N/A. Notes are kept in the file for other programs, but JeffCrunch does not read or print them yet.
- **Which page comes next**: down the sheet and then across, or across and then down.
