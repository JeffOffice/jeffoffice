# Print and make a PDF
<!-- keywords: margins, drag margins, column width, drag column width, hyperlinks, links in a pdf, document properties, printer properties, open the pdf after saving, grayscale, black and white, last page first, reverse, pages on a sheet, several pages per sheet, n-up, odd pages, even pages, both sides, duplex, print, printer, preview, pdf, export, copies, collate, pages, page range, selection, entire workbook, scaling, fit, email, page breaks, dashed lines, save as pdf, scale to paper, paper size, sheet preview, password, encrypt, protect with a password, greyscale -->

Choose **File > Print** (or press **Ctrl+P**). The page opens with a preview of the pages that will come out, one at a time. Use the arrows under it to go from page to page, and **Fit page** or the zoom buttons to see more or less of a page. The preview is drawn the way the printer and the PDF are, so what you see is what you get.

## What to print

- **What**: the sheet you are on (or all the sheets whose tabs you picked with Ctrl or Shift, one after the other), the whole workbook (every sheet that is shown), or the selection. Hidden sheets are left out. A chart sheet is one page, with its chart as large as the page's room inside the margins.
- **Pages**: all of them, or some, such as **1-3, 5**.
- **Which pages**: **All pages in the range**, **Only odd pages**, or **Only even pages**. Use this to print on both sides of the paper by hand when your printer can't do it. First print the odd pages. When they are done, turn the stack over, put it back in the paper tray, and print again with **Only even pages**.
- **Sides**: **Print on one side**, **Print on both sides, flip on long edge**, **Print on both sides, flip on short edge**, or **Print on both sides by hand**. The long edge is for pages that turn like a book. The short edge is for pages that turn like a notepad. A printer that can't print on both sides has those two dimmed. Point at one to see why. **By hand** is for such a printer: JeffCrunch prints the first, third, fifth, and so on of the pages, or of the sheets if you print several pages on a sheet. Then a message asks you to turn the stack over and put it back in the paper tray. When you click **OK**, it prints the others, last page first, so they land on the backs. If you print only one page, there is nothing to turn over. All of them are dimmed when you save a PDF, because a file has no sides. JeffCrunch keeps your choice for next time.
- **Pages on a sheet**: print 1, 2, 4, 6, 9, or 16 pages, made smaller, on each sheet of paper. Each page gets a thin frame. The preview still shows one page at full size.
- **Copies** and **Collated** (a whole set at a time, 1 2 3 1 2 3, instead of each page's copies together).
- **Last page first** prints the pages in reverse order, so a stack comes out in order. With several pages on a sheet it is the sheets that come out last first, and on both sides of the paper a front stays with its back. It is dimmed when you print on both sides by hand, because JeffCrunch then sets the order itself.
- **Greyscale** prints every colour in shades of grey, on paper and in a PDF file. The preview is grey too.
- **Orientation**, **Paper**, and **Margins**, to use for this print instead of what each sheet says.
- **Scaling**: as each sheet says, full size, the whole sheet on one page, all the columns on one page, or all the rows on one page.
- **Ignore print area**: print everything that has something in it.
- **Page Setup** opens the dialog for the sheet you are on.

Under the preview, JeffCrunch says when something needs your attention: a sheet that needs more pages than it can make, a sheet shrunk so far that the letters are very small, or a sheet with nothing on it.

## Scale to a paper size

**Scale to paper** makes each page smaller or larger to fit the paper you pick, keeping its shape and putting it in the middle. Use it to print for a paper your printer does not have, such as a page set up for A4 on letter paper. **No scaling** prints each page on its own paper. The preview shows the sheet of paper that will come out.

## Several pages on a sheet

**Pages on a sheet** puts 2, 4, 6, 9, or 16 pages on each sheet of paper, each one smaller and in a frame. The preview shows the whole sheet, and the label under it says which sheet you are looking at, such as **Sheet 1 of 2**. Use **Next** and **Previous** to look at the other sheets.

## Move the margins and the column widths on the preview

On a page of the preview, dashed lines mark the four margins, and a thin line marks the right edge of each column that is on the page (the first 64 of them). Drag a margin line to move that margin, or drag a column's line to make the column wider or narrower. The sheet changes as you let go, as one step, so **Undo** takes it back, and the preview is made again. A line has a name that a screen reader says, such as "Width of column C, 3 of 12" or "Left margin, 0.75\"" (the margin's value in your units). Click a line, or tab to it, and press the arrow keys to move it by a small step; the new place is said when you let the key go, not at every repeat of a key held down. On a sheet that reads right to left the page is printed mirrored, and a column's line is at its left edge, which you drag away from the column to widen it. On a protected sheet a column's width can't be changed here, as it can't in the sheet; the margins can, as in Page Setup.

To do it with the keyboard, press **Tab** until the line you want has the focus, then press the arrow keys, left and right for the sides and for the columns, up and down for the top and the bottom. A screen reader says the name of each line, such as **Left margin** or **Width of column B**. Each key press moves it a little and is a step of its own.

You can also use **Page Layout > Margins > Custom Margins** for the margins and **Home > Format > Column Width** for the columns. The lines are not drawn for the margins while you have chosen **Margins** for this print (Narrow, Standard, or Wide), because those are not the sheet's own, and they are not drawn while several pages share a sheet of paper.

## The printer

Pick a printer from the list, or **Save as PDF**. **Printer Properties** opens your system's own window for the printer, for choices such as paper trays. What you set there (the printer, the copies, collate, and colour) is put on this page. The button is hidden when **Save as PDF** is chosen, or when your computer has no such window. **Print** sends the pages.

With **Save as PDF** chosen, the box **Open the PDF after saving** opens the file in the program your computer uses for PDF files as soon as it is saved. It is off at first, and the choice is kept for the next time.

## Export to PDF

**File > Export** makes a PDF of the sheet you are on, or of the whole workbook, without choosing a printer. The letters stay letters (not pictures), so they can be searched and stay sharp when you zoom.

Each sheet gets a **bookmark**, an entry in the list that a PDF reader shows beside the pages. Click a bookmark to jump to the first page of its sheet.

A cell that has a link is a clickable box in the PDF, on every page the cell is on. A cell in a title row that repeats at the top of every page is a link on each of those pages. A link to a web address (http or https) or to an email address opens it. A link to a place in the workbook, such as Sheet2!A1, goes to that cell on its page, or to the first page of that sheet when the cell's page is not in the PDF. A link to a file, to any other kind of address, or to a sheet that has no page in the PDF is left out. A cell that a page break splits, such as a merged cell that runs over several pages, is a link on each of those pages, over the part of it that page shows. Links are limited: an address longer than 2048 characters is left out, and a PDF holds at most 20000 links, and the rest are not clickable. The export says so when that happens, and how many addresses were left out.

The PDF's own properties, the ones a PDF reader shows, are the title, author, subject, and keywords of the workbook (see **File > Info**). When the personal information is removed, or **Remove personal information** is on in the options, the PDF has no author, and no title made from the file's name. Links are not properties: an email (mailto) link is written to the PDF even when the personal information is removed, because it is a link that is in a cell. Take the cell's link off in the sheet if the address should not be in the PDF.

The box **Open the PDF after saving** on this page opens the PDF after it is saved. It is the same choice as the one on the Print page.

The box **Add document structure tags for accessibility to every PDF** is checked at first, and it is kept for the next time. It adds a map of each page for people who listen to a PDF with a screen reader. The map says that the cells of a page are a table, so the reader can say them row by row, from left to right. Backgrounds, lines, headings, headers, footers, and charts are left out of what is read. Uncheck the box if you don't want the map.

**Export the whole workbook as PDF/A** makes a PDF in the form made for keeping for years. Every font is put inside the file, so it reads the same long from now. Programs that archive documents ask for this form. It is the PDF/A-1b form, and it can be tagged too.

**Make an Email** makes a message file with your workbook in it, or a PDF of it. Nothing is sent. Open the file in your email program, add who it is for, and send it. A workbook that has a password stays locked in the message: whoever gets it needs the password to open it. The password itself is never put in the message. A PDF of the workbook has a password only if you click **Encrypt with a password…** in the box, which is the same as for a PDF you export. Tell the person the password some other way than in the message.

## Protect a PDF with a password

On **File > Export**, click **Encrypt with a password…** before you choose a PDF command. It is used for the next PDF only. Type the password twice. Anyone who opens the PDF then has to type it. The password really protects the file: the pages are scrambled with strong encryption (AES-256), and nobody can read them without it.

You can also check **Restrict printing, copying, and editing**, give a second password, and choose what is allowed. Those limits are only a request. Most PDF readers honour them, but they don't stop anyone who wants to get around them, so don't count on them to keep a file private.

A forgotten password can't be recovered, so keep it somewhere safe. The password is used for that one PDF and is never saved. The **Export the whole workbook as PDF/A** command is dimmed while a password is set, because a PDF/A file can't have one.

## Page breaks on the screen

After you open the Print screen, the sheet on your screen shows where the pages begin: a dashed line for a break JeffCrunch worked out, and a solid line for one you set. They stay up to date as you change the sheet. Turn them off with **View > Page Lines**. (The **Page Breaks** view, described in [Set up the pages](page-layout), always shows them.)

## What is not printed yet

Notes (comments) print after the sheet's last page when the sheet's Page Setup says **List them after the last page**; they are not printed where they are on the cells yet. Conditional formats are not shown on the screen yet, so they are not printed either.
