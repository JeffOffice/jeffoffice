# Print and make a PDF
<!-- keywords: pages on a sheet, several pages per sheet, n-up, odd pages, even pages, both sides, duplex, print, printer, preview, pdf, export, copies, collate, pages, page range, selection, entire workbook, scaling, fit, email, page breaks, dashed lines, save as pdf, scale to paper, paper size, sheet preview, password, encrypt, protect with a password -->

Choose **File > Print** (or press **Ctrl+P**). The page opens with a preview of the pages that will come out, one at a time. Use the arrows under it to go from page to page, and **Fit page** or the zoom buttons to see more or less of a page. The preview is drawn the way the printer and the PDF are, so what you see is what you get.

## What to print

- **What**: the sheet you are on (or all the sheets whose tabs you picked with Ctrl or Shift, one after the other), the whole workbook (every sheet that is shown), or the selection. Hidden sheets are left out. A chart sheet is one page, with its chart as large as the page's room inside the margins.
- **Pages**: all of them, or some, such as **1-3, 5**.
- **Which pages**: **All the pages**, **Odd pages only**, or **Even pages only**. Use this to print on both sides of the paper by hand when your printer can't do it. First print the odd pages. When they are done, turn the stack over, put it back in the paper tray, and print again with **Even pages only**.
- **Sides**: **Print on one side**, **Print on both sides, flip on long edge**, **Print on both sides, flip on short edge**, or **Print on both sides by hand**. The long edge is for pages that turn like a book. The short edge is for pages that turn like a notepad. A printer that can't print on both sides has those two dimmed. Point at one to see why. **By hand** is for such a printer: JeffCrunch prints the first, third, fifth, and so on of the pages, or of the sheets if you print several pages on a sheet. Then a message asks you to turn the stack over and put it back in the paper tray. When you click **OK**, it prints the others, last page first, so they land on the backs. If you print only one page, there is nothing to turn over. All of them are dimmed when you save a PDF, because a file has no sides. JeffCrunch keeps your choice for next time.
- **Pages on a sheet**: print 1, 2, 4, 6, 9, or 16 pages, made smaller, on each sheet of paper. Each page gets a thin frame. The preview still shows one page at full size.
- **Copies** and **Collated** (a whole set at a time, 1 2 3 1 2 3, instead of each page's copies together).
- **Orientation**, **Paper**, and **Margins**, to use for this print instead of what each sheet says.
- **Scaling**: as each sheet says, full size, the whole sheet on one page, all the columns on one page, or all the rows on one page.
- **Ignore print area**: print everything that has something in it.
- **Page Setup** opens the dialog for the sheet you are on.

Under the preview, JeffCrunch says when something needs your attention: a sheet that needs more pages than it can make, a sheet shrunk so far that the letters are very small, or a sheet with nothing on it.

## Scale to a paper size

**Scale to paper** makes each page smaller or larger to fit the paper you pick, keeping its shape and putting it in the middle. Use it to print for a paper your printer does not have, such as a page set up for A4 on letter paper. **No scaling** prints each page on its own paper. The preview shows the sheet of paper that will come out.

## Several pages on a sheet

**Pages on a sheet** puts 2, 4, 6, 9, or 16 pages on each sheet of paper, each one smaller and in a frame. The preview shows the whole sheet, and the label under it says which sheet you are looking at, such as **Sheet 1 of 2**. Use **Next** and **Previous** to look at the other sheets.

## The printer

Pick a printer from the list, or **Save as PDF**. **Printer settings** opens your system's own dialog for the printer's other choices, such as paper trays. **Print** sends the pages.

## Export to PDF

**File > Export** makes a PDF of the sheet you are on, or of the whole workbook, without choosing a printer. The letters stay letters (not pictures), so they can be searched and stay sharp when you zoom.

**Export the whole workbook as PDF/A** makes a PDF in the form made for keeping for years. Every font is put inside the file, so it reads the same long from now. Programs that archive documents ask for this form.

**Make an Email** makes a message file with your workbook in it, or a PDF of it. Nothing is sent. Open the file in your email program, add who it is for, and send it. A workbook that has a password stays locked in the message: whoever gets it needs the password to open it. The password itself is never put in the message. A PDF of the workbook has no password.

## Protect a PDF with a password

On **File > Export**, click **Encrypt with a password…** before you choose a PDF command. It is used for the next PDF only. Type the password twice. Anyone who opens the PDF then has to type it. The password really protects the file: the pages are scrambled with strong encryption (AES-256), and nobody can read them without it.

You can also check **Restrict printing, copying, and editing**, give a second password, and choose what is allowed. Those limits are only a request. Most PDF readers honor them, but they don't stop anyone who wants to get around them, so don't count on them to keep a file private.

A forgotten password can't be recovered, so keep it somewhere safe. The password is used for that one PDF and is never saved. The **Export the whole workbook as PDF/A** command is dimmed while a password is set, because a PDF/A file can't have one.

## Page breaks on the screen

After you open the Print screen, the sheet on your screen shows where the pages begin: a dashed line for a break JeffCrunch worked out, and a solid line for one you set. They stay up to date as you change the sheet. Turn them off with **View > Page Lines**. (The **Page Breaks** view, described in [Set up the pages](page-layout), always shows them.)

## What is not printed yet

Notes (comments) print after the sheet's last page when the sheet's Page Setup says **List them after the last page**; they are not printed where they are on the cells yet. Conditional formats are not shown on the screen yet, so they are not printed either.
