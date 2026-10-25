# JeffCrunch feature list

JeffCrunch is the spreadsheet app of JeffOffice. JeffOffice is an independent project: JeffCrunch aims to work with Excel workbooks and the people who use them (compatibility and interoperability), and then to do more, and do it better. It saves `.xlsx` by default. Its first release also opens `.xls` (Excel 5 to 2003), `.ods`, and `.csv`; saving in `.xls` and `.ods` follows once `.xlsx` is solid (decided October 11, 2026). It is not built yet: nothing exists except what the suite already does for every app, so `[x]` and `[~]` count only that. This list is the whole job, a checklist of features: everything people rely on so their workbooks open, calculate the same numbers, look right, and keep working, and then more, grouped the way the app's ribbon is (File, then each ribbon tab, the tabs that appear only while something is selected, then what sits under the ribbon), so the work can be planned and tracked.

Last reviewed October 22, 2026.

Legend: `[x]` done · `[~]` partly there (the note says what's missing, or which shared JeffOffice part it builds on) · `[ ]` not yet · **(alt)** the usual way depends on a proprietary online service, so the app offers an open replacement.

Every item carries a tier:

- **first release**: needed in JeffCrunch's first public release.
- **1.0**: needed before JeffCrunch is called finished (version 1.0).
- **later**: wanted, but not before 1.0.

## How the tiers were chosen

**First release** is the first public build (a preview, as JeffDeck 0.7.0 is). It must open, edit, print, and save the kinds of workbooks in our collection and the public collections without damaging them, and its formulas must come out with the numbers the workbooks stored. It should be good enough for a budget, a ledger, a price list, a report, or a small model. For the advanced objects (pivot tables, slicers, charts the editor can't draw yet, query definitions), "first release" means open, show, keep, and write back unchanged; building and editing them is 1.0. **1.0** is what someone coming from Excel expects day to day. Version 1.0 is ready to ship when every file JeffCrunch saves opens without a repair message, JeffCrunch's recalculated numbers equal the numbers stored in the public workbooks, and JeffCrunch's printed pages match the reference pages about as closely as JeffPub's match Publisher's. An item is **later** when it is used by few workbooks, needs a piece the suite doesn't have yet, or depends on something outside the program.

How the evidence reads. Where the counts measure a feature, its item carries how many real workbooks use it, as "ours" and "public". **Ours** is the share of our own 1,481 working spreadsheets (1,335 `.xlsx` and 146 `.xls`); we also hold 1,267 `.csv` files, described under File formats. **Public** is the average of three public collections, each counted alone and then averaged so that the biggest doesn't drown the others: **Enron** (15,871 `.xlsx` business models and reports; the figure for one collection reads "Enron 18%"), **EUSES** (4,648 `.xls` of many kinds, from schools to labs), and **government** (1,304 `.xls` from U.S. government sites). Edge cases count lightly and are named when quoted: Apache POI's 366 test `.xlsx` ("POI") and LibreOffice's 364 test `.xlsx` ("LibreOffice"). The old `.xls` files were converted to `.xlsx` by LibreOffice to be counted. A count is "how many workbooks use it", so 3% means about 1 in 33. Five cautions:

- The conversion writes some things into every sheet, so they read near 100% in the converted old files and are not real: page headers and footers, print options, row and column grouping, and "cell styles beyond Normal". Where the figure matters, the item quotes the real one (Enron, or our own `.xlsx`).
- The conversion also hides some real uses: hidden rows and columns, array formulas, shared formulas, and what-if tables read 0% in the converted files because LibreOffice writes them in another form than the count looks for. Only the native `.xlsx` figures (Enron, ours, POI, LibreOffice) are trusted for those. The same goes for `TRUE` and `FALSE` showing as "functions" in the converted files. The conversion also drops macro projects, so those are counted in the old files themselves (8% of EUSES and 12% of government).
- The public collections were gathered between 2001 (Enron) and 2009 (government) from files made before Excel 2007 (Excel wrote the Enron `.xlsx` later; the files name it as the program), so features newer than Excel 2003 (tables, sparklines, data bars, color scales, icon sets, slicers, timelines, dynamic arrays) read 0% in all three public collections. Evidence for them comes from POI and LibreOffice (edge cases) and from our own files, and is thin; the items say so.
- Function counts are counts of workbooks, taken from workbooks that have at least one formula ("of formula workbooks"). A shared formula (one formula filled down a column) names its functions only in the first cell, so how many times a function is used is undercounted; how many workbooks use it is right.
- Our own `.xlsx` are mostly small data exports: the middle one holds 55 cells, and 4% have any formula (the `.xls` are bigger: the middle one holds 2,915 cells, and 33% have formulas). So ours is weak evidence about formulas and models. The owner expects files more advanced than ours, which is why the public collections weigh most.

Two things were found while counting. First, the first counting run read only the 58 Enron files saved as `.xls` and skipped the 15,871 saved as `.xlsx`; the figures here include all of them (counted again on October 15, 2026). Second, the 417 POI `.xls` and 304 LibreOffice `.xls` files did not convert (their output folders are empty), so they are not counted; they are the damaged and unusual files, and the best test of the `.xls` reader.

## Beyond Excel

What JeffCrunch offers that Excel doesn't, then the plans that go past Excel.

- [~] **first release** Runs on Linux and Windows, where Excel ran only on Windows and Mac, and on macOS as the rest of the suite does *(shared: the suite's builds, installers, and AppImage; the macOS app is still a test build)*
- [x] **first release** Free and open source (GPL-3.0): no license fee, no subscription, no account or sign-in, nothing that has to be online
- [~] **first release** Installs without administrator rights, or runs with no installation (Windows `.zip`, Linux AppImage) *(shared: the suite's packaging)*
- [ ] **first release** OpenDocument spreadsheets (`.ods`, `.ots`) are opened with the same care as `.xlsx`, not as an afterthought, and saved once `.xlsx` is solid (details under File formats)
- [ ] **first release** Nothing in a file is lost: whatever JeffCrunch can't edit yet (pivot caches, slicers, timelines, the data model, query definitions, custom XML, the macro project, ActiveX controls, 3-D models, and unknown elements) is kept with the workbook and written back as it was
- [ ] **first release** Every formula keeps its last result: a function JeffCrunch doesn't have yet still shows the number the file stored and is written back as it was, never turned into a `#NAME?` error
- [ ] **first release** Numbers match the original's. The calculation engine is checked by recalculating tens of thousands of public workbooks and comparing every formula with the answer the workbook stored (see Fidelity and testing)
- [x] **first release** Bundled open fonts, with stand-ins that keep the widths of Calibri, Cambria, Arial, Times New Roman, and Courier New, so column widths and wrapped text fall where the original puts them on a computer without Microsoft's fonts
- [~] **first release** Spelling works offline, in 12 languages, with no cloud service *(shared: the bundled dictionaries in libs/base/text)*
- [ ] **first release** Text and CSV files without surprises: the import preview shows every column's type before anything changes, and the original's habits that damage data are off unless you ask. A part number like `00123` keeps its zeros, a 16-digit account number keeps its last digits, and `1-2` or a gene name like `MARCH1` is not turned into a date. The window remembers the file's separator and quote choices, and a `.csv` you didn't edit is written back as it was read *(ours: 1,267 `.csv` files)*
- [ ] **1.0** **(alt)** Getting data without Microsoft's online services (the original uses Power Query for this): read a text or CSV file, a workbook, JSON, XML, a table from a web page, a whole folder of files, a SQLite file, or a database over ODBC, PostgreSQL, or MySQL, as a list of steps you can see, change, and refresh; a Power Query saved in a workbook is kept untouched (see the open questions)
- [ ] **1.0** `jcrunchtool`, a command-line tool that converts between formats, recalculates and compares a workbook with the answers it stored, lists a file's parts, extracts a range as CSV, and runs the fidelity tests, as `jpubtool` and `jdecktool` do for the other apps *(shared: the pattern of those tools)*
- [~] **1.0** PDF with a bookmark for each sheet, tags for screen readers, and PDF/A *(shared: JeffPub's PDF export)*
- [~] **1.0** Insert > Icons: 1,539 open vector icons, searched by name *(shared: JeffPub's icon picker)*
- [~] **1.0** **(alt)** Insert > Pictures > Online Pictures searches Openverse and Wikimedia Commons, keeping the license and credit, in place of Bing *(shared: JeffPub's online pictures)*
- [~] **1.0** Help that works offline, with a page for every function (what it does, each argument, and a worked example) and a search that also finds commands and runs them *(shared: libs/shell help; the function pages are new)*
- [ ] **later** Copy a range as a Markdown table or an HTML table, and paste either back as cells
- [~] **later** Insert > Barcode: a column of codes turned into barcodes for labels (EAN, UPC, Code 128, Code 39, QR), drawn as vector bars *(shared: libs/base/core barcode)*

## File (backstage)

### Info
- [~] **first release** Workbook properties: title, author, subject, keywords, categories, comments, company, and manager, with the dates created, modified, and printed *(shared: JeffPub's publication properties)*
- [ ] **1.0** Custom properties (a name, a type, and a value), kept as saved
- [ ] **1.0** Protect Workbook menu: Mark as Final (the workbook opens read-only with a note), Encrypt with Password (open and save a password-protected `.xlsx` with the standard's strong encryption, and `.xls` with its older one), Protect Current Sheet, and Protect Workbook Structure (see Review) *(workbook protection: ours 0%, public <1%)*
- [ ] **1.0** Inspect Workbook: find and remove comments, personal information, hidden sheets, rows, and columns, custom XML, headers and footers, external links, and invisible objects before sharing *(hidden sheets: ours 1%, public 4%)*
- [ ] **1.0** Check Compatibility: lists what an older format (`.xls`, `.ods`, `.csv`) can't hold, such as rows past 65,536, columns past 256, newer functions, newer conditional formats, sparklines, tables, and slicers, before saving to it
- [ ] **1.0** Check Accessibility (see Review)
- [ ] **1.0** **(alt)** Version history without a cloud service: named local copies of a workbook, compared and restored from Info
- [~] **1.0** Recover unsaved workbooks *(shared: AutoRecover in libs/shell)*
- [ ] **1.0** Show a workbook's folder and copy its path
- [ ] **later** Add a digital signature (an invisible one, and the signature line from Insert) with a certificate on your computer
- [ ] **later** Links to Files: see Edit Links under Data

### New
- [ ] **first release** Blank workbook, using the defaults of the Normal template (one sheet, Calibri 11, the Office theme)
- [~] **first release** Template gallery with live previews and categories, starting with a few built-in templates (budget, invoice, expense report, loan schedule, calendar, and timesheet) written by JeffOffice and open-licensed *(shared: JeffPub's New page)*
- [ ] **1.0** More built-in templates: checkbook register, grade book, inventory list, mileage log, project plan, and sales report
- [ ] **1.0** Save as a template (`.xltx`, `.ots`) and reuse your own under My Templates; a template can hold formulas, styles, and sheets
- [ ] **1.0** The default workbook and sheet: set the font, size, number of sheets, and view of every new workbook or new sheet (`Book.xltx` and `Sheet.xltx` in the original)

### Open, Save, Save As, and Close
- [~] **first release** Open `.xlsx`, `.xls`, `.ods`, `.csv`, and `.txt` (and the templates `.xltx`, `.xlt`, and `.ots`), from the Recent list with pinning, from Browse, by drag-and-drop, and from the command line *(shared: the recent list and file pages in libs/shell)*
- [ ] **first release** Save as `.xlsx` by default; Save As lists every format JeffCrunch writes (`.xlsx`, `.csv`, and `.txt` at first); Ctrl+S, Ctrl+Shift+S, and Close with a prompt to save. Save on a workbook opened as `.xls` or `.ods` asks for an `.xlsx` name, never writing over the old file (decided October 11, 2026)
- [ ] **first release** A file that can't be read gives a clear message and never crashes the program or loses the other open workbooks *(unreadable or encrypted: ours 0%, POI 4%)*
- [~] **first release** AutoRecover (unsaved work offered again after a crash, cleaned up once saved) *(shared: libs/shell recovery)*
- [~] **1.0** Backup copy kept on every save *(shared: JeffPub's option)*
- [ ] **1.0** Save a Copy (keeps working in the original) and Save as Template
- [ ] **1.0** A warning before saving to a format that can't hold something in the workbook, naming what would change, with the choice to save an `.xlsx` as well
- [ ] **1.0** Open and Repair: recover the cell values, formulas, and styles that can be read from a damaged `.xlsx` or `.xls`, and say what was dropped
- [ ] **1.0** Open read-only, open as a copy, and open in a new window
- [ ] **1.0** Open an encrypted workbook by asking for its password
- [ ] **1.0** Save `.xls` and `.ods` (follows once `.xlsx` is solid; see File formats)
- [ ] **first release** Open `.xlsm` and `.xltm` (macro-enabled), keeping the macro project untouched and never running it; save them back the same way *(macro projects: ours 1% (2 of 146 old files), public 10% of the old files (EUSES 8%, government 12%); none can sit in a plain `.xlsx`; POI 2%, LibreOffice 1%)*
- [ ] **1.0** Convert a workbook's macros to a JeffOffice script: the macros are translated to JavaScript, shown to you to review, and run only when you allow it, in a sandbox with no files or network unless you grant them; anything that can't be translated is marked, never dropped *(decided October 22, 2026: after the preview, as its own project)*
- [ ] **later** Open and save the binary workbook `.xlsb`, and open `.xlam` add-ins as data
- [ ] **later** Protected View: files from the Internet open for reading only until you choose to edit them
- [ ] **later** Open from a web address (HTTP and WebDAV) and save back

### Print
- [~] **first release** Print with printer choice, copies, collate, what to print (the active sheets, the whole workbook, or the selection), page ranges, one- or two-sided, orientation, paper size, margins, and scaling, and printer properties *(shared: JeffPub's print; done October 24, 2026: File > Print with the system's printers and Save as PDF, copies, collate, this sheet, the whole workbook, or the selection, page ranges, orientation, paper, margins, and scaling for the one print, and the system's own dialog for two-sided and the other settings of a printer; not done: several selected sheets printed at once)*
- [~] **first release** Print preview with page navigation, zoom, a margins view that drags the margins and column widths, and a shrink-to-fit check *(shared: JeffPub's print preview; done: the live preview drawn as the printer and the PDF are, next and previous, page n of m, fit page and zoom, and a note under it when a sheet is shrunk very small, needs more pages than can be made, or has nothing on it; not done: dragging the margins and the column widths on the preview)*
- [x] **first release** Print what the page setup says (see Page Layout): the print area, titles repeated on every page, page breaks, scaling to a number of pages wide and tall, headers and footers, gridlines, and headings *(done October 24, 2026; notes are not printed, and conditional formats are not shown on the screen yet so they are not printed)*
- [ ] **1.0** Print several pages on a sheet, scaled to a paper size, collated or not, and a table or chart alone
- [ ] **1.0** Duplex printing by hand: print the odd pages, then the even ones, with a prompt to turn the stack over

### Share and Export
- [~] **first release** Save as PDF: the active sheets, the whole workbook, or the selection, page range, fonts embedded, document properties, and a PDF opened after saving *(shared: JeffPub's PDF export)*
- [ ] **first release** Export to CSV and text: the active sheet to `.csv` (comma), `.csv` (semicolon, for countries where the comma is the decimal mark), tab-delimited `.txt`, and a space- or custom-separated file, in the encoding chosen (UTF-8 with or without a mark, UTF-16, and Windows code pages), with cells written as shown or as stored
- [~] **1.0** PDF/A, tagged PDF, PDF with a password *(shared: JeffPub's PDF export)*
- [~] **1.0** Save as a single-file web page, and a selection or chart as an HTML table or picture *(shared: JeffPub's web export)*
- [~] **1.0** **(alt)** Email: make a ready-to-send `.eml` with the workbook as an attachment, or a PDF, instead of sending through an online account *(shared: JeffPub's .eml export)*
- [ ] **1.0** **(alt)** Share without a cloud account: a package with the workbook and its pictures for another computer
- [ ] **later** Save as `.xlsm` and `.xltm`, keeping macros untouched and never running them
- [ ] **later** Export a table to a SQLite file or to SQL `INSERT` statements

### Options
- [~] **first release** General: user name and initials, light or dark interface, display language, the font and size of new workbooks, number of sheets, usage statistics (opt in), and update checks *(shared: libs/shell settings)*
- [~] **first release** Formulas: calculation (automatic, automatic except data tables, or manual, and recalculate before saving), iterative calculation (maximum iterations and change), the R1C1 style, formula autocomplete, error-checking rules, and the workbook's 1904 date system *(manual calculation: ours 0%, public 4%, Enron 11%; iterative calculation: ours 0%, public 2%, Enron 5%; 1904 dates: ours <1%, public <1%, POI 5%)* *(done October 25, 2026: the three calculation modes, iterative calculation, and formula autocomplete are in the Formulas tab; the options page is not yet)*
- [~] **first release** Display: show gridlines and their color, formulas and zeros, headings, sheet tabs, scroll bars, row and column headers, outline symbols, and page breaks; comments shown as indicators, hover, or always; what to show for objects *(shared: JeffPub's display options)*
- [~] **first release** Proofing: AutoCorrect (replacement list, capitalization rules, smart quotes), spelling, ignoring words in capitals or with numbers, and custom dictionaries *(shared: JeffPub's Proofing options)*
- [~] **first release** Save: default format, AutoRecover interval, and the default folder *(shared: JeffPub's Save options)*
- [ ] **first release** Advanced editing: press Enter to move selection (down, right, up, or left), edit directly in the cell, extend data range formats and formulas, automatic decimal places, the fill handle and drag-and-drop, AutoComplete for cell values, and cut, copy, and paste choices
- [ ] **1.0** Advanced display for workbooks and sheets: show formulas, page breaks, zeros, outline symbols, objects, the gridline color, and right-to-left sheets; set the default zoom and the standard width
- [ ] **1.0** Advanced calculation and compatibility: precision as displayed, update links to other workbooks on open, save external link values, ignore other programs that use DDE, and the use of system separators (decimal, thousands, and list separators)
- [ ] **1.0** Language: editing languages and the display language; the list separator and decimal mark follow the system, with a choice
- [ ] **1.0** Customize the Ribbon and the Quick Access Toolbar
- [ ] **1.0** Customize the keyboard: assign keys to commands, with Excel's shortcuts the starting set
- [ ] **1.0** Trust Center equivalents: what happens with external links, data connections, macros (kept, never run), and files from outside; no add-ins from the Store
- [~] **1.0** Privacy: remove personal information on save, and the choice about what is sent anywhere (statistics, crash reports) *(shared: libs/shell telemetry)*
- [ ] **1.0** Accessibility options: sound and animation feedback, high contrast, and keyboard focus

## Home tab

### Clipboard
- [~] **first release** Cut, Copy, and Paste of cells: a copied range shows a moving border until Esc; pasted formulas shift their relative references and keep the absolute ones; text from other programs (plain, tab-separated, HTML tables, and RTF) lands in cells *(shared: JeffPub's clipboard)* *(done in 4b-2a: values, formulas with their relative references moved, formats, and merged cells, within a workbook and between workbooks (values, formulas, and number formats); a cut moves the cells and what refers to them; tab-separated text and HTML tables in and out; RTF not yet)*
- [~] **first release** Paste Special: all, formulas, values, formats, comments and notes, validation, all using the source theme, all except borders, column widths, formulas with number formats, and values with number formats; with an operation (add, subtract, multiply, or divide the pasted numbers into the cells under them), skip blanks, and transpose (rows become columns) *(done in 4b-2a: all, formulas, values, formats, the four operations, skip blanks, and transpose; the others in the list not yet)*
- [ ] **1.0** The paste gallery with a live preview, and paste as a picture, as a linked picture that follows the range, as a link to the cells, as a hyperlink, with values and source formatting, and merging conditional formatting
- [ ] **1.0** Paste text through the Text Import wizard (split pasted text into columns by separator or fixed width), and Set Default Paste
- [x] **first release** Format Painter: one use, or locked with a double-click, for cell formatting *(shared: JeffPub's Format Painter)* *(done in 4b-2b)*
- [ ] **1.0** Copy as Picture (as shown on screen or as printed), pasted into JeffWrite or JeffDeck as a picture *(shared with JeffWrite and JeffDeck)*
- [ ] **1.0** Cells pasted into JeffWrite or JeffDeck land as a table, keeping the fonts, fills, borders, and number formats as shown *(shared with JeffWrite and JeffDeck)*
- [ ] **later** Clipboard task pane holding the last 24 items

### Font
- [~] **first release** Font name (theme fonts first, recent fonts, a preview in the list) and size (list, typed, and Grow and Shrink) *(shared: JeffPub's Font group)* *(done in 4b-2b: the font and size boxes, with Increase and Decrease Font Size)*
- [~] **first release** Bold, italic, underline (single and double, plus the accounting underlines that run the width of the cell), strikethrough, subscript, and superscript *(shared: JeffPub's Font group)* *(done in 4b-2b: bold, italic, underline (single and double), and strikethrough on the Home tab, and the accounting underlines, subscript, and superscript in Format Cells; all are drawn)*
- [~] **first release** Font color (theme colors with tints, standard colors, more colors) and cell fill color, with No Fill *(cells formatted beyond Normal: ours .xlsx 8%, Enron 95%; the converted old files read 98%, not real)* *(done in 4b-2b: theme colors with tints, standard colors, More Colors, and No Color, each one step of Undo)*
- [~] **first release** Borders: bottom, top, left, right, none, all, outside, thick outside, double bottom, top and bottom, and the others in the gallery, with line style and color, Draw Border, Draw Border Grid, Erase Border, and diagonal borders in the Format Cells dialog *(done in 4b-2b: the Borders menu with line color and style, and the diagonals in Format Cells; Draw Border, Draw Border Grid, and Erase Border not yet)*
- [~] **first release** The Format Cells dialog with its Font, Border, and Fill tabs: pattern fills with two colors, gradient fills, and a live preview *(done in 4b-2b: the three tabs with two-color patterns and a preview; gradient fills are shown but not made)*
- [ ] **first release** Formatting part of a cell's text (a bold word, a colored phrase, a superscript) is shown, kept, and edited in the cell and in the formula bar *(rich text kept inside a shared string; inline rich text: ours 0%, POI <1%)*
- [ ] **later** Phonetic guide (furigana for Japanese text), shown and edited

### Alignment
- [~] **first release** Horizontal alignment: general (numbers right, text left), left, center, right, justify, repeat to fill (the cell's text repeated across it), center over the selected cells, and spread evenly; vertical alignment: top, center, bottom, justify, and distributed *(done in 4b-2b: every choice, on the Home tab and in Format Cells, and all of them drawn)*
- [~] **first release** Wrap Text, and row heights that follow wrapped text (automatic height) as the original sets them *(done in 4b-2b: Wrap Text on and off; a row's height does not follow the text by itself yet (AutoFit Row Height fits it))*
- [~] **first release** Merge & Center, Merge Across, Merge Cells, and Unmerge Cells; a merged area keeps the top-left cell's value, warns before discarding the others, and takes part in sort, fill, filter, and print as the original lets it *(merged cells: ours 3% (.xls 23%), public 47%, Enron 31%, EUSES 52%, government 57%)* *(done in 4b-2b: the four commands, with a question before other cells' values are emptied; sort, fill, filter, and print later)*
- [x] **first release** Indent (increase and decrease, in steps of the character width) and Shrink to Fit *(done in 4b-2b)*
- [x] **first release** Orientation: angle counterclockwise or clockwise, vertical text, rotate up, and rotate down, from -90 to +90 degrees *(done in 4b-2b: the menu, and degrees in Format Cells; drawn at its angle by the cell's alignment)*
- [~] **1.0** Text direction: by the text, left to right, and right to left, with right-to-left sheets (the whole sheet runs right to left) *(right-to-left sheets: ours 0%, public 0%)* *(done in 4b-2b: the choices in Format Cells, and a sheet that reads right to left is drawn mirrored as the file says; no command turns it on)*
- [x] **first release** The Format Cells dialog's Alignment tab with every choice above

### Number
- [x] **first release** The number format box: General, Number, Currency, Accounting, Short Date, Long Date, Time, Percentage, Fraction, Scientific, Text, and More Number Formats *(custom formats: the middle workbook holds 2 to 3 of its own, the most in one file is 1,736 (Enron); see Number formats)* *(done in 4b-2b)*
- [~] **first release** Accounting Number Format with a currency chosen from a list, Percent Style, Comma Style, Increase Decimal, and Decrease Decimal *(done in 4b-2b: Currency (the language's symbol), Percent Style, Comma Style, Increase Decimal, and Decrease Decimal; a list of currencies on the button not yet (Format Cells has one))*
- [x] **first release** The Format Cells dialog's Number tab: every category above plus Special (ZIP code, ZIP+4, phone number, social security number), with decimal places, the thousands separator, negative number styles (red, parentheses), a live sample, and Custom with its format-code language *(done in 4b-2b: twelve categories with their options and a sample in the person's language)*
- [~] **1.0** Formats follow the locale: dates, times, currencies, and separators for the workbook's region and the system's, with a locale tag in a format (`[$-409]`, `[$-407]`) *(done in 4b-2b: numbers are shown in the marks of the system's language, and Format Cells lists its ready-made dates, times, and currencies; the file keeps the neutral form)*

### Styles
- [~] **first release** Conditional formatting shows as the file says, for every rule kind it can hold (cell value, text, dates, top and bottom, above and below average, duplicates and unique, formula, data bars, color scales, and icon sets), including rules on tables and pivot tables *(rules: cell-value ours 0%, public 2%, Enron 3%; formula ours <1%, public <1%, government 1%; the newer kinds (data bars, color scales, icon sets): ours <1%, LibreOffice 2%, POI <1%)* *(done in 4b-2b: every kind in the list is read, follows edits, is saved as it was, and is drawn for the cells that show, in the ordinary form of the file: the twenty icon sets by their names, color scales, and data bars (with the 2010 extension's look). What only the 2010 extension holds is kept in the file but not drawn: an icon set with icons of its own, and a rule whose formula reads another sheet when the file stores it there (see the item below). The rules cannot be made or changed yet)*
- [ ] **first release** Conditional Formatting menu: Highlight Cells Rules (greater than, less than, between, equal to, text that contains, a date occurring, duplicate values) and New Rule with a formula, with the format chosen (font, border, fill, number), Manage Rules (order, Stop If True, the range each rule applies to, edit and delete), and Clear Rules from the selection, the sheet, a table, or a pivot table *(4b-2b shows the rules a workbook has; making and changing them is not done)*
- [ ] **1.0** Top/Bottom Rules (top 10 items, top 10 percent, bottom, above average, below average), Data Bars (gradient and solid, negative values, axis, direction), Color Scales (two and three colors, with their types), and Icon Sets (the 20 sets, thresholds, reverse order, show icon only) *(drawn in 4b-2b when a workbook has them; the menu to make them is not done)*
- [~] **1.0** Conditional formats that point at another sheet or another workbook's cells, and the 2010 extension forms of data bars and icon sets *(2010-style forms: ours <1%, LibreOffice 5%, POI <1%)* *(done in 4b-2b: the 2010 extension form of a data bar is read, drawn, and written into a copy of the sheet, and a rule that reads another sheet is drawn when the file stores it in the ordinary form. Not done: the rules the extension itself holds (the form used for a formula that reads another sheet, and icon sets with icons of their own) are kept in the file as they were but are not drawn, and a copy of the sheet does not have them (the copy says so); another workbook's cells are not read)*
- [ ] **1.0** Format as Table: the gallery of table styles (light, medium, and dark), with header row, banded rows, and filter buttons turned on; the selection becomes a table (see Table Design) *(tables: ours 1%, public 0% (they came after the Enron files), POI 11%, LibreOffice 8%)*
- [~] **first release** Cell Styles: the gallery (Normal, Good, Bad, Neutral, Calculation, Check Cell, Explanatory Text, Input, Linked Cell, Note, Output, Warning Text, Heading 1 to 4, Title, Total, the Accent styles with tints, Comma, Currency, Percent), apply, modify (what a style sets: number, alignment, font, border, fill, protection), and Normal as the base of the workbook *(done in 4b-2b: the gallery and applying a style, with the built-in styles made when used; modify not yet)*
- [ ] **1.0** New Cell Style, Merge Styles from another open workbook, and delete a style, with cells reverting to Normal
- [ ] **later** New PivotTable Style and new Table Style (see the contextual tabs)

### Cells
- [~] **first release** Insert and Delete: cells (shifting the others right or down, or left or up), rows, columns, and sheets; the formulas that point there follow, and a formula pointing at a deleted cell shows `#REF!` as the original does *(done in 4b-2a: cells, rows, and columns, one undo step, every reference follows, in the file too; sheets not yet)* *(done in 4b-2b: sheets too (Insert Sheet, the New Sheet button, Delete Sheet), with #REF! in what read a deleted sheet)*
- [x] **first release** Format menu: Row Height and AutoFit Row Height, Column Width, AutoFit Column Width, and Default Width (the unit is the width of a zero in the default font, so the standard width of 8.43 is 64 pixels in Calibri 11, as in the original) *(done in 4b-2a: Row Height, AutoFit Row Height, Column Width, AutoFit Column Width, drag a border, double-click to fit, and a number typed or pasted widens a column that has its first width; Default Width not yet)* *(done in 4b-2b: Default Width and Default Row Height too)*
- [x] **first release** Hide and Unhide rows, columns, and sheets, with Very Hidden sheets (hidden so that the Unhide list doesn't offer them) kept as saved *(hidden sheets: ours 1%, public 4%, Enron 6%; very hidden: ours 0%, Enron 3%; hidden columns: ours <1%, Enron 21%; hidden rows: ours <1%, Enron 16%)* *(done in 4b-2a: rows and columns, Ctrl+0 and Ctrl+9; sheets not yet)* *(done in 4b-2b: sheets, with several picked at once)*
- [~] **first release** Rename Sheet, Move or Copy Sheet (within the workbook, to another open workbook, or to a new one), and Tab Color *(done in 4b-2b: rename (every formula that names the sheet follows, a name quoted by the file format's rules), move or copy within the workbook (or drag a tab), and Tab Color; to another open workbook or a new one not yet. A copy has the sheet's protection, filter, links, page setup, page breaks, row and column groups, and data bar looks, and says it leaves out pictures, charts, notes, PivotTables, sparklines, and the names that belong to the sheet alone. A workbook holds up to 1,000 sheets, the most the reader takes)*
- [ ] **first release** Protect Sheet and Lock Cell: locked cells (all cells are locked until the sheet is protected) and hidden formulas (the Format Cells Protection tab), the list of what a protected sheet still allows, and a password that is kept and checked as the file stores it (see Review) *(sheet protection: ours 0%, public 4%, EUSES 7%)*
- [ ] **1.0** Insert and Delete table rows and columns (above, below, left, right)
- [ ] **1.0** Move a selection by dragging its border (with Alt to drop onto another sheet's tab and Ctrl to copy), and insert copied cells, shifting the rest

### Editing
- [~] **first release** AutoSum menu (Sum, Average, Count Numbers, Max, Min, More Functions) and Alt+= inserting the sum of the cells above or to the left, guessing the range from the data *(done in 4b-2a: Sum, Average, Count Numbers, Max, and Min, Alt+= (Cmd+Shift+T on a Mac); More Functions not yet)*
- [~] **first release** Fill: Down (Ctrl+D), Right (Ctrl+R), Up, Left, across worksheets, Series (linear, growth, date by day, weekday, month, or year, autofill trend, step and stop values), and Justify; the fill handle (dragging the corner of a selection) with its options (copy cells, fill series, formatting only, without formatting, fill days, weekdays, months, or years) *(done in 4b-2a: Down, Right, Up, Left, Series, and the fill handle with Ctrl to copy; across worksheets, Justify, and the handle's options menu not yet)*
- [ ] **first release** Custom lists for fills (Sunday to Saturday, January to December, and your own, kept in Options)
- [ ] **1.0** Flash Fill (Ctrl+E): fills a column from a pattern you typed in the first cells of the next one, such as splitting `Smith, Ann` into a first and a last name
- [ ] **first release** Clear: All, Formats, Contents, Comments and Notes, and Hyperlinks (and Remove Hyperlinks)
- [x] **first release** Sort & Filter menu: Sort A to Z, Sort Z to A, Custom Sort, Filter, Clear, and Reapply (details under Data) *(filters: ours 2% (.xls 13%), public 3%)* *(done in 4b-2c)*
- [~] **first release** Find & Select: Find and Replace (look in formulas, values, or comments; match case, match the whole cell; by row or by column; in the sheet or the workbook; with wildcards `*`, `?`, and `~`; and by format), Go To (a reference, a name, or the Special dialog: comments, constants, formulas (numbers, text, logicals, errors), blanks, current region and array, objects, row and column differences, precedents, dependents, last cell, visible cells only, conditional formats, and data validation), Select Objects, and the Selection Pane *(shared: the Find and Replace dialogs in libs/page; the cell search is new)* *(done in 4b-2c: Find, Find All (a list to click), Replace, and Replace All (one step, refused with a reason for locked, array, and table-header cells) in the sheet or the workbook, by rows or columns, in formulas, values, or notes, with match case, match entire cell, and the wildcards; Go To by reference (also another sheet's, whole rows or columns) or name; Go To Special: blanks, constants and formulas by kind of result, errors, current region, last cell, visible cells, conditional formats, and data validation (the first block is selected and the others tinted). Not yet: find by format, replace in notes, the current array, objects, row and column differences, precedents and dependents, Select Objects, and the Selection Pane)*
- [ ] **later** Ideas (Analyze Data): not offered, since it sends a range to an online service (see Left out on purpose)

## Insert tab

### Tables
- [ ] **first release** PivotTables and PivotCharts in a workbook are shown as saved (the numbers in the cells, the field list, the filters) and kept whole, with the data behind them (the pivot cache), written back as they were *(pivot tables: ours 0%, public <1%, Enron 2% (250 workbooks), POI 2%, LibreOffice 19%)*
- [ ] **1.0** PivotTable: insert from a range, a table, or another workbook's range, onto a new sheet or an existing one, with the field list (see PivotTable Analyze)
- [ ] **1.0** Recommended PivotTables: a few ready layouts picked from the data's shape (counts, sums by a text column, sums by a date column)
- [ ] **1.0** Table (Ctrl+T or Ctrl+L): turn a range into a table with a header row, a name, calculated columns that fill themselves, structured references in formulas (`Sales[Amount]`), a totals row, and automatic growth when you type below it *(tables: ours 1%, public 0%, POI 11%, LibreOffice 8%)*
- [ ] **later** Forms: not offered, since it builds an online form (see Left out on purpose); a data entry form for a table (the old Form command) comes at 1.0 under Developer

### Illustrations
- [~] **first release** Pictures from a file (PNG, JPEG, GIF, SVG, BMP, TIFF, WMF, and EMF), shown, moved with the cell or fixed, and kept, with alt text *(pictures: ours <1%, public 9%, Enron 13%; shared: JeffPub's picture insert)*
- [~] **first release** Drawing objects that sit on a sheet (lines, rectangles, text boxes, arrows, and the other shapes) are shown, kept, and written back, in front of or behind the cells *(drawing layer, which also holds pictures and charts: ours 2% (.xls 16%), public 23%, Enron 26%; shared: libs/draw and DrawingML in libs/office)*
- [~] **1.0** Shapes: lines, rectangles, basic shapes, block arrows, flowchart, stars and banners, and callouts, with text inside them, connectors, and edit points *(shared: JeffPub's shapes)*
- [~] **1.0** Icons *(shared: JeffPub's icon picker)* and **(alt)** Online Pictures from Openverse and Wikimedia Commons *(shared: JeffPub's online pictures)*
- [ ] **1.0** SmartArt diagrams are shown as saved and kept; editing follows *(SmartArt: ours 0%, public 0%)*
- [ ] **later** Screenshot and Screen Clipping
- [~] **later** 3-D Models are kept as saved, with the picture they show *(shared: JeffPub keeps them as pictures)*

### Charts
- [~] **first release** Charts are shown as saved, drawn from the numbers in the cells they point at, in all the kinds the original has; those the editor can't draw yet are kept and drawn from their stored values *(charts: ours <1%, public 8%, Enron 11% (1,685 workbooks), EUSES 7%, government 6%; chart sheets: Enron 3%; shared: libs/office chart reads and draws column, bar, line, pie, doughnut, area, and scatter from stored values; here they follow the cells)*
- [~] **first release** New Chart for the common kinds: clustered, stacked, and 100% column and bar; line (with or without markers); pie; area; and scatter (markers only, straight lines, and smooth lines), from the selected cells, with the series in rows or columns by the shape of the data *(line charts: ours <1%, public 4%, Enron 7%; column and bar: public 3%; scatter: public 2%, government 4%; pie: public <1%; area: public <1%; done (October 25, 2026): the Insert tab's Charts group and a New Chart dialog (More Charts) with a picture of each kind drawn from the cells selected; column and bar (clustered, stacked, 100%), line (plain, with markers, stacked), pie, doughnut, area (plain, stacked), scatter (points, joined points), and combo, from the first row and column as names and labels, the series down columns or along rows by the shape of the data (or switched); scatter with smooth lines is not offered yet)*
- [~] **1.0** More kinds: doughnut, bubble, radar (and filled radar), stock (high-low-close, open-high-low-close, and the volume forms), combo (a column and a line together, a secondary axis), 3-D column, bar, line, pie, and area, pie of pie and bar of pie, and surface (3-D and contour) *(3-D bar and pie: public <1%; radar, stock, surface: public <1%; doughnut and a combo of columns and a line against a second axis are made and changed here)*
- [ ] **1.0** Recommended Charts: a few charts picked for the selected data
- [ ] **later** The charts added in 2016, each kept as saved and drawn from stored values first: treemap, sunburst, histogram, box and whisker, waterfall, funnel, and Pareto *(charts of the newer parts: ours 0%, public 0%, POI 0%)*
- [ ] **later** **(alt)** Map charts (filled maps by country, state, or postal code): not offered with Bing's maps; an open-map version comes later, and 3-D Maps (Power Map) is left out
- [ ] **1.0** PivotChart: a chart that follows a pivot table (see PivotTable Analyze), with field buttons and filters

### Sparklines
- [ ] **1.0** Sparklines (a tiny chart inside one cell): Line, Column, and Win/Loss, made from a row or column of values into one cell or a group of cells, shown as saved in first release *(sparklines: ours 0%, public 0%, POI <1%, LibreOffice <1%; a 2010 feature)*

### Filters
- [ ] **1.0** Slicer (a set of buttons that filter a table or pivot table by clicking, one button per value): insert, connect to several pivot tables, style, and multi-select; shown as saved in first release *(slicers: ours 0%, public 0%; a 2010 feature)*
- [ ] **1.0** Timeline (a slider that filters a pivot table by year, quarter, month, or day): insert, style, and move; shown as saved in first release *(timelines: ours 0%, public 0%; a 2013 feature)*

### Links
- [ ] **first release** Link (Ctrl+K): to a web address, an email address, a file, a place in this workbook (a cell or a name), or a new workbook, with the screen tip; follow with Ctrl+click; edit and remove *(hyperlinks: ours 4% (.xls 25%), public 8%, government 14%, EUSES 9%)*
- [ ] **1.0** Insert Link gallery (recent files, browsed pages, and bookmarks inside the workbook)

### Comments
- [ ] **1.0** Comment (the newer kind, with replies; see Review). The original's older comments, called notes, are in the first release (also under Review)

### Text
- [~] **first release** Text Box and Text Art on a sheet, shown and kept as saved; editing at 1.0 *(shared: JeffPub's text boxes and Text Art)*
- [ ] **first release** Header & Footer: opens the page layout view to edit the header and footer (see Header & Footer tab) *(headers and footers: ours <1% (.xlsx), Enron 95%; the converted collections read 100%, not real)*
- [ ] **1.0** Object: an embedded object from another program (a JeffWrite page, a JeffDeck slide, a picture, or a drawing), shown as the picture it saved and opened in its own program *(embedded objects: ours 0%, public 2%, Enron 5% (723 workbooks))*
- [ ] **later** Signature Line (see Info; a digital signature)

### Symbols
- [ ] **1.0** Equation: the Office Math editor, with the built-in equations, writing by typing, and ink; shown as saved in first release *(equations: ours 0%, public 0%; shared: the equation engine to build for JeffWrite and JeffDeck)*
- [~] **first release** Symbol: any character from any font, recent symbols, and special characters, with shortcut keys *(shared: JeffPub's Symbol dialog)*

## Draw tab

- [ ] **later** Ink on a sheet: pens, pencil, and highlighter with colors and thickness, an eraser, lasso select, Ink to Shape, Ink to Math, and Ink Replay; ink already in a workbook is kept and shown *(ink: ours 0%, public 0%; shared: the ink layer to build on the slide show's pen, needed by JeffWrite and JeffDeck too)*
- [ ] **later** Draw with Touch, for touch screens and pens

## Page Layout tab

### Themes
- [~] **first release** A workbook's theme (its colors, fonts, and effects) is read and kept, and every color or font a cell takes from the theme resolves to it, as the cell shows in the original *(shared: libs/office theme)*
- [~] **1.0** Themes gallery, Colors, Fonts, and Effects: a new theme restyles every cell that uses theme colors and theme fonts; save the current theme; create your own color and font sets; browse for a theme from another file *(shared: JeffDeck's theme gallery and the themes of libs/office)*

### Page Setup
- [~] **first release** Margins (Normal, Wide, Narrow, and custom, with the header and footer margins), Orientation, and Size (every paper size the printer offers, plus More Paper Sizes), and Center on Page (horizontally and vertically) *(shared: JeffPub's page setup; done October 24, 2026 except that the sizes are a fixed list of the common ones and More Paper Sizes is the Page Setup dialog's size box, not the printer's own list)*
- [x] **first release** Print Area: Set, Add to, and Clear; several areas on one sheet print on separate pages *(print areas: ours <1% (.xls 3%), public 36%, Enron 47% (7,422 workbooks), EUSES 35%, government 27%; done October 24, 2026, read from `.xlsx` and `.xls`, kept through saving, and moved by inserting and deleting rows and columns; and with a sheet through insert, copy, delete, rename, and move, and Undo of each)*
- [~] **first release** Breaks: Insert, Remove, and Reset All Page Breaks (horizontal and vertical), and automatic breaks that fall where the original puts them for the same fonts and printer *(manual page breaks: ours <1%, public 14%, EUSES 17%, government 15%, Enron 10%; done October 24, 2026: the commands, the breaks read and saved and moved by edits, and dashed lines for the automatic ones on the sheet; the automatic breaks: the automatic breaks agree for 84.6% of the 1,283 reference sheets by count of pages, and the first word of a page is the same on 72.4% of pages)*
- [x] **first release** Print Titles: rows repeated at the top and columns repeated at the left of every printed page *(print titles: ours <1%, public 18%, government 20%, EUSES 19%, Enron 16%; done October 24, 2026, read from `.xlsx` and `.xls`, kept through saving, and moved by edits)*
- [ ] **1.0** Background: a picture tiled behind a sheet's cells (shown, not printed)
- [~] **first release** The Page Setup dialog with its four tabs: Page (orientation, scale or fit, paper size, quality, first page number), Margins, Header/Footer (see Header & Footer tab), and Sheet (print area, titles, gridlines, black and white, draft quality, row and column headings, comments at the end or as displayed, errors printed as shown or as blank, `--`, or `#N/A`, and page order: down then over, or over then down) *(headers and footers: Enron 95%; the converted collections read 100%, not real; done October 24, 2026 with the four tabs and the presets and the custom editor for the header and footer; the print quality is not on the Page tab, and the comments setting is kept and saved but notes are not printed yet)*

### Scale to Fit
- [x] **first release** Fit the sheet to a number of pages wide and tall (or automatic), or a percent from 10 to 400, with a shrink warning when the type becomes too small to read *(done October 24, 2026; a fit never enlarges, as in the original)*
- [x] **first release** Scaling stays with the sheet and is kept as saved, so the same pages come out on the same paper *(done October 24, 2026: the scale, the fit, and the paper are read, changed, and saved by patching only what differs)*

### Sheet Options
- [~] **first release** Gridlines and Headings: view and print each separately *(printing each is done October 24, 2026, and the view part was there before)*
- [ ] **first release** Right-to-left sheet (Page Layout > Sheet Right-to-Left) *(right-to-left sheets: ours 0%, public 0%)*

### Arrange
- [~] **1.0** Bring Forward and Send Backward (to front and to back), Selection Pane, Align (left, center, right, top, middle, bottom) and Distribute, Group and Ungroup, Rotate and Flip, Snap to Grid and Snap to Shape *(shared: JeffPub's Arrange group)*

## Formulas tab

### Function Library
- [~] **first release** Insert Function (the fx button on the formula bar): search by what a function does, browse by category, show each argument's help, and the live result of the arguments typed so far (Function Arguments dialog) *(done in 4b-2c: a dialog with a search of the names and what the functions do, the kinds, and for each of the engine's functions its arguments and a sentence; it begins the formula in the cell or at the cursor, and AutoSum's More Functions opens it. Not yet: the Function Arguments dialog with the live result)*
- [ ] **first release** AutoSum and its menu; Recently Used; and a gallery for each category: Financial, Logical, Text, Date & Time, Lookup & Reference, Math & Trig, and More Functions (Statistical, Engineering, Cube, Information, Compatibility, and Web) (see Functions)
- [~] **first release** Typing a formula: function names completed as you type (with the argument names shown), arguments told apart by commas or semicolons as the language wants, references colored in the formula and in the sheet, drag a colored border to change a reference, and F4 cycling between relative and absolute (`A1`, `$A$1`, `A$1`, `$A1`) *(done October 25, 2026: after `=` and letters a list of the functions, names, and tables that begin so (at most 12, from an index capped at 20,000 names), Tab takes the one marked, the arrow keys then Enter or Tab; the arguments of the function being typed show under the cell with the current one in bold, with the separator the language uses. Not yet: colored references, dragging a border, and F4)*
- [ ] **first release** Formulas that name another sheet or workbook (`Sheet2!A1`, `'My Sheet'!A1:B4`, `[Budget.xlsx]Sheet1!A1`), a 3-D reference over several sheets (`Sheet1:Sheet5!A1`), a whole row or column (`A:A`, `3:3`), and a name

### Defined Names
- [~] **first release** Defined names: a name for a cell, a range, a constant, or a formula, whose scope is the workbook or one sheet; the Name Box (jump to or create a name); built-in names for print areas, print titles, and filters kept as saved *(names defined: the middle workbook holds 1, and the most in one is 4,427 (government) and 3,295 (Enron))* *(done October 25, 2026: names are read with their comments and hidden marks, made and changed and taken away (each one step of Undo, saved into the file's list with the names that were not touched left byte for byte), checked for the rules a name keeps with the reason said as it is typed, and Go To follows them. Not yet: creating a name by typing it in the Name Box, and formulas that use a name written for a block of several sheets)*
- [~] **first release** Name Manager: add, edit, delete, filter, and sort names; hidden names kept; a name that points to an error flagged *(done October 25, 2026 as the Names dialog (Ctrl+F3): the list with name, value, what it stands for, where it works, and comment, a filter, New, Change, and Delete; a name that is renamed is renamed in every formula, validation, conditional format, and other name that uses it, and in the file. Not yet: sorting by a column, and flagging a name that points to an error)*
- [~] **1.0** Create from Selection (names from the row and column labels), Use in Formula, Paste Names (F3), and Apply Names (replace references with names) *(done October 25, 2026: Name From Cells (top, left, bottom, or right labels, in one step) and Insert Name (a menu on the Formulas tab; F3 is Find Next here, so it has no key). Not yet: Apply Names)*

### Formula Auditing
- [~] **1.0** Show What Feeds This Cell and Show What Uses This Cell (arrows to the cells a formula uses and to the cells that use it), Clear Arrows, and Trace Error *(done October 25, 2026: arrows from what a formula reads (names followed) and to the formulas that read a cell, one step more with each click, a sheet icon for a cell on another sheet, at most 2,000 arrows with a note when there were more, and Clear Arrows; any change to the workbook takes them away. Not yet: Trace Error)*
- [ ] **first release** Show Formulas (Ctrl plus the grave accent key, left of 1): the sheet shows formulas instead of results, with widened columns
- [ ] **1.0** Error Checking: the rules (a formula that gives an error, an inconsistent formula, a formula that leaves out neighboring cells, a number stored as text, a two-digit year, a formula to unlocked cells, and others), a green corner marker on the cell with its menu, and "ignore error" kept as saved *(errors ignored: ours <1%, public <1%, POI <1%)*
- [ ] **1.0** Evaluate Formula: step through a formula one part at a time and see each part's value
- [ ] **1.0** Watch Window: a list of cells and their values that stays in view across sheets and workbooks
- [~] **first release** Circular References: a warning when a formula depends on itself, the cell named, and the status bar's note (see Calculation engine) *(done October 25, 2026: the cells are named once when a circle begins, outlined on the sheet, noted in the status bar, and listed (double-click to go there) under Circular References. Evaluate Formula is not done)*

### Calculation
- [~] **first release** How Formulas Are Worked Out: Automatic, Tables by Hand, and Manual (with Calculate Before Save), kept in the file as saved *(manual calculation: ours 0%, public 4%, Enron 11% (1,804 workbooks))* *(done October 25, 2026: the three modes, one step of Undo, saved in the file; a Manual workbook opens with the results its file holds and the status bar says when a recalculation is due; iterative calculation (most rounds, smallest change) is gone round by the engine, bounded, and saved. Data tables are still not worked out in any mode, and Calculate Before Save is not offered)*
- [~] **first release** Calculate Now (F9), Calculate Sheet (Shift+F9), recalculate everything (Ctrl+Alt+F9), and rebuild the dependency list and recalculate (Ctrl+Shift+Alt+F9) *(done October 25, 2026: F9 and Shift+F9. Calculate Sheet works out the formulas of the sheet and whatever else was waiting; the other two keys are not done)*

## Data tab

### Get & Transform Data and Connections
- [ ] **first release** Data > From Text/CSV: opens a text file through the import preview (file origin and encoding, delimiter, how many rows to check to guess types, and the column types), then loads it into a sheet *(ours: 1,267 `.csv` files)*
- [ ] **1.0** **(alt)** From Table/Range, From Web (the tables on a page), From Workbook, From XML, From JSON, From Folder, and From PDF: each a list of steps that can be changed and refreshed; loaded to a sheet or a table *(connections: ours 0%, public <1%, Enron <1% (114 workbooks))*
- [ ] **1.0** From a database: SQLite, ODBC, PostgreSQL, MySQL and MariaDB, with a saved connection (no password in the file unless you choose) and a refresh *(query tables: ours 0%, public <1%, Enron <1% (111 workbooks))*
- [ ] **1.0** **(alt)** Query steps editor, an open editor in place of Power Query: choose columns, remove or keep rows, filter, sort, change a column's type, split a column, replace values, group and total, pivot and unpivot, merge two queries (join), append, and load to the sheet or only keep the connection
- [ ] **first release** Data connections, query tables, and the Power Query definitions in a workbook are kept and written back as they were, and their last results show in the sheet *(data connections: ours 0%, public <1%; custom XML and the query's data mashup: kept)*
- [ ] **later** Power Query's M language: read, edit, and run saved queries (see the open questions)
- [ ] **later** Power Pivot and the data model (tables with relationships and measures in DAX): kept as saved; building one is not planned *(data model: ours 0%, public 0%, LibreOffice <1%)*
- [ ] **1.0** Queries & Connections pane: the list of queries and connections, Refresh All (with background refresh), Refresh on open, refresh every N minutes, and Connection Properties
- [ ] **1.0** Edit Links: the external workbooks a workbook points to, with Update Values, Change Source, Open Source, Break Link, and Check Status; a link is shown with the values the file stored until you update it *(external links: ours <1%, public 8%, Enron 18% (2,786 workbooks), EUSES 3%, government 3%)*
- [ ] **later** Data Types (Stocks and Geography) are an online Microsoft service: cells that hold one are kept and show their stored text; no lookup is offered (see Left out on purpose and the open questions)

### Sort & Filter
- [~] **first release** Sort A to Z and Z to A on a column (the whole list moves together, the header row is found by itself), and Custom Sort: several levels, on values, cell color, font color, or icon, ascending, descending, or by a custom list (months, weekdays), match case, and top to bottom or left to right *(sort state: ours <1%, public 0%, POI 2%)* *(done in 4b-2c: both on the block around the cell (a header is found), and Custom Sort with up to 8 levels on values, cell color, or font color, ascending, descending, or by a list (days, months, or one typed), header, match case, and left to right; one step, formulas written for their new rows, merged cells of one size moved with their rows and other merges refused, the rows a filter hides left where they are. Not yet: sort by icon, and writing the file's sort state)*
- [~] **first release** Filter (AutoFilter): drop-down buttons in the header row; check boxes for values with a search box; text filters (equals, begins with, ends with, contains, and their opposites), number filters (greater than, between, top 10 items or percent, above and below average), date filters (today, this week, next month, this quarter, year to date, and all dates in a period), filter by color, and a custom filter of two conditions joined by And or Or; the status bar says how many rows matched; hidden rows keep their row numbers *(filters: ours 2% (.xls 13%), public 3%, Enron 4% (621 workbooks), EUSES 2%, government 3%)* *(done in 4b-2c: buttons, value lists with search and (Select All), a date column listed by year, month, and day, the number, text, and date filters, top 10, above and below average, the periods and a quarter or a month of any year, color filters, a custom filter of two conditions, and sort from the button; the filter and the rows it hides are read and saved, and told from rows hidden by hand. Not yet: the status bar's count of rows, filters of tables, and filter by icon)*
- [x] **first release** Clear and Reapply: the filter state is kept as saved, and rows hidden by a filter stay hidden when the file reopens *(workbooks that store a filter range: ours 2%, public 4%)* *(done in 4b-2c)*
- [ ] **1.0** Advanced Filter: a criteria range you write on the sheet (And across, Or down), filter in place or copy the results to another place, and unique records only
- [ ] **1.0** Filters and sorts on tables and pivot tables (a table keeps its own filter; a sort on a pivot table sorts its items, not the cells)

### Data Tools
- [ ] **1.0** Text to Columns: split a column's text by a separator (comma, semicolon, space, tab, or your own) or at fixed widths, with each column's type and where the results go
- [ ] **1.0** Remove Duplicates: choose the columns that must match, and say how many rows were removed and how many remain
- [~] **first release** Data Validation shows and keeps its rules as saved (a cell showing its drop-down arrow, its input message on selection, and its error alert on a bad entry) *(data validation: ours <1%, public 1%, EUSES 2%, government 2%, Enron <1% (86 workbooks), POI 4%; by kind: list public 1%, whole number and text length <1%, decimal, date, time, and custom <1%)* *(done in 4b-2a: the rules are read, kept, followed through edits, and applied to typed and pasted values with their own style, title, and message, and a list is offered from a button on the cell; the input message and the newer extension-list form are not shown)* *(done in 4b-2c: the input message shows beside the cell, and the rules the extension list holds (lists that read other sheets) are read, offered as a list, and kept through a save and through inserted rows and columns)*
- [ ] **1.0** Data Validation editing: Any value, Whole number, Decimal, List (typed or from a range, in-cell drop-down), Date, Time, Text length, and Custom (a formula), with between, not between, equal, and the others, Ignore blank, an input message, and an error alert of Stop, Warning, or Information; Circle Invalid Data and Clear Validation Circles; validation by a list on another sheet or a name; the 2010 extension form *(2010-style rules: ours 0%, POI <1%, LibreOffice 1%)*
- [ ] **1.0** Consolidate: combine ranges from several sheets or workbooks by position or by label with a function (sum, count, average, and the others), with links to the source data
- [ ] **later** Relationships and Manage Data Model (see above)

### Forecast (What-If)
- [ ] **1.0** Goal Seek: change one cell until a formula's result reaches a target (for example, the interest rate that makes a payment $500)
- [ ] **1.0** Data Table (a what-if table): one- and two-input tables that run a formula for many values of one or two cells, recalculated with the sheet or set to calculate only on request *(what-if tables: ours 0%, public <1%, Enron <1% (16 workbooks), LibreOffice <1%)*
- [ ] **1.0** Scenario Manager: named sets of input values, shown, merged from other workbooks, and summarized as a report; scenarios in a file are kept and shown *(scenarios: ours 0%, public <1%, EUSES <1% (12 workbooks), LibreOffice <1%)*
- [ ] **1.0** Forecast Sheet: a chart and table of future values from a timeline and a series (the ETS algorithm), with a confidence band and seasonality, using FORECAST.ETS
- [ ] **later** Solver, an add-in of the original, for optimizing with limits: a linear, nonlinear, or evolutionary solver *(LibreOffice's tests hold solver files)*

### Outline
- [ ] **first release** Row and column outlines are kept and work: the plus and minus buttons and level numbers 1 to 8 in the margin, collapsed groups staying collapsed *(grouping: ours <1%, Enron 4%; the converted collections read 100%, not real)*
- [ ] **1.0** Group and Ungroup rows and columns, Auto Outline, Clear Outline, Show Detail and Hide Detail, and the Outline settings (summary rows below detail, summary columns right of detail, automatic styles)
- [ ] **1.0** Subtotal: insert a SUBTOTAL row at each change in a sorted column (with the function, the columns to total, replace current subtotals, a page break between groups, and the summary below the data), and Remove All

## Review tab

### Proofing and Accessibility
- [~] **first release** Spelling (F7): check the sheet or the selection, with the cell's text shown in context, ignore, change, add, and the dictionaries *(shared: the spelling service in libs/shell)*
- [ ] **1.0** Thesaurus, from an open-licensed word list in the languages that have one
- [ ] **1.0** Workbook Statistics: the numbers of sheets, cells with data, formulas, charts, and pivot tables
- [ ] **later** Speak Cells: read cells aloud as they are entered, by rows or by columns, with the system's speech
- [ ] **1.0** Check Accessibility: missing alt text, merged cells, blank cells used for layout, duplicate sheet names, a header row not marked, low color contrast, and a reading order

### Language
- [ ] **later** **(alt)** Translate: not Microsoft Translator; an offline translator may follow (see the open questions)
- [ ] **later** Hangul and Hanja conversion, and Chinese simplified and traditional conversion

### Comments and Notes
- [ ] **first release** Notes (the original's older comments, a text box that shows on hover): New Note, Edit, Delete, Previous, Next, Show/Hide, Show All Notes, resize and move, and the author name; saved as the original does, both the note and its drawing *(notes: ours <1%, public 7%, Enron 10% (1,540 workbooks), EUSES 6%, government 6%, POI 7%)*
- [ ] **1.0** Comments (the newer, threaded kind): New Comment, replies, resolve, delete, previous and next, and Show Comments; "Convert to Comments" and back; shown as saved first *(threaded comments: ours 0%, public 0%, POI <1%; shared: the review model to build for JeffWrite)*
- [ ] **later** Show Ink and Hide Ink (the ink of a review)

### Changes and Protect
- [ ] **first release** Protect Sheet (with a password, and the choices of what stays allowed: select locked or unlocked cells, format cells, rows, or columns, insert rows, columns, or hyperlinks, delete rows or columns, sort, use filters and pivot tables, edit objects and scenarios) and Unprotect Sheet; enforced as the original does, and the password check kept as the file stores it *(sheet protection: ours 0%, public 4%, EUSES 7%, government 3%, Enron 2%)*
- [ ] **1.0** Protect Workbook: lock the structure (no new, deleted, moved, renamed, or hidden sheets) and windows, with a password *(workbook protection: ours 0%, public <1%, POI 2%)*
- [ ] **1.0** Allow Edit Ranges: ranges that can be edited on a protected sheet, each with its own password or with named users on one computer
- [ ] **later** Track Changes (the older kind: Highlight Changes, Accept or Reject Changes, and a history sheet) and Share Workbook (the older shared-workbook mode); the files' change records are kept as saved *(shared workbooks and change logs: ours 0%, public 0%)*
- [ ] **later** Collaborate and co-authoring: left out (see Left out on purpose)

## View tab

### Workbook Views
- [~] **first release** Normal, Page Break Preview (dragging the blue page lines, with "Page 1" watermark on each page and the print area in white), and Page Layout (each page drawn with margins, header and footer, and the rulers) *(Normal only; after a preview or print, dashed lines on the Normal view show where the pages begin and View > Page Breaks turns them off; Page Break Preview and the Page Layout view are not built)*
- [ ] **1.0** Custom Views: named combinations of the view, filters, hidden rows and columns, print settings, and window position, shown and kept *(custom views: ours 0%, public <1%, Enron <1% (138 workbooks))*
- [ ] **later** Sheet View (the 365 feature: private filter and sort views of a shared sheet)

### Show, Zoom, and Window
- [~] **first release** Gridlines, Formula Bar, and Headings (and the ruler in Page Layout), turned on and off for each sheet and kept as saved *(shared: JeffPub's View tab)*
- [~] **first release** Zoom (10% to 400%, and fit to the selection), 100%, and Zoom to Selection, kept for each sheet *(shared: JeffPub's zoom)*
- [~] **first release** Freeze Panes (rows above and columns left of the selection stay in view), Freeze Top Row, and Freeze First Column; Split (four panes that scroll apart) *(frozen panes: ours 1%, public 12%, Enron 19% (2,969 workbooks), government 12%, EUSES 6%; split panes: ours 0%, public <1%, EUSES 1%, government 1%)* *(wave 4a: Freeze Panes, Top Row, First Column, and Unfreeze work, and a file's frozen panes are shown; not saved yet, no Split, and held panes begin at the first row and column))*
- [ ] **1.0** New Window (a second view of the same workbook), Arrange All (tiled, horizontal, vertical, cascade), View Side by Side with Synchronous Scrolling, Reset Window Position, Hide and Unhide a window, and Switch Windows
- [ ] **1.0** Full Screen, and the Ribbon hidden or shown; the status bar choices (the average, count, numerical count, minimum, maximum, and sum of the selection; the zoom slider; the calculation mode)
- [ ] **later** Macros: View Macros and Record Macro (see Developer)

## Developer tab (hidden until turned on)

- [ ] **first release** Form controls on a sheet (button, combo box, check box, spin button, list box, option button, group box, label, and scroll bar) are shown, kept, and written back, with the cell or range each is linked to *(form controls: ours <1%, public 3%, Enron 7% (1,037 workbooks), EUSES 2%, POI <1%, LibreOffice 3%)*
- [ ] **1.0** Insert and use form controls: they work without macros when linked to a cell (a check box writes TRUE or FALSE, a spin button changes a number, a combo box writes the position chosen)
- [ ] **1.0** Data entry form for a table or list: one record at a time, with next, previous, new, delete, and find by criteria
- [ ] **later** ActiveX controls are kept and drawn as the picture they saved; never run *(ActiveX controls: ours 0%, public <1%, Enron 1% (230 workbooks))*
- [ ] **later** Macros are kept and never run; recording or writing them needs a scripting language first (see the open questions) *(macro projects in the old `.xls`: ours 1%, EUSES 8% (375 of 4,652), government 12% (151 of 1,304); counted in the files themselves, since the conversion drops them)*
- [ ] **later** XML maps: bind cells to an XML schema, then import and export XML, with refresh, map properties, and expansion packs *(custom XML: ours <1%, POI 3%, LibreOffice 4%)*
- [ ] **later** Add-ins and COM add-ins: not offered (see Left out on purpose)

## Help tab

- [~] **first release** Help (F1): a built-in, offline pane of plain-language topics with a contents page, links, Back and Forward, and a search that finds topics, commands, and functions *(shared: libs/shell help)*
- [~] **first release** Keyboard Shortcuts (a table made from the commands themselves), What's New, Feedback (a GitHub page with the version filled in), and About with Check for Updates *(shared: libs/shell)*
- [~] **1.0** A search box on the ribbon that finds a command by what it does and runs it *(shared: the help search)*
- [ ] **1.0** Function help from the formula bar: the function you are typing opens its page (the same pages as the Insert Function dialog)

## Contextual tabs

The tabs that appear only while something is selected or being edited: a table, a pivot table, a chart, a sparkline, a slicer, a picture, a shape, a header or footer, an equation, a diagram, a 3-D model, or ink.

### Table Design tab (while a table is selected)

- [ ] **first release** A table in a file is opened with its name, range, header row, totals row, banded rows and columns, first and last column marks, filter buttons, and style, drawn as the style says (the built-in light, medium, and dark styles and your own), and kept; its calculated columns and structured references work *(tables: ours 1%, public 0% (they came after the Enron files), POI 11%, LibreOffice 8%)*
- [ ] **1.0** Table Name and Resize Table (change the range), Header Row, Total Row (with a function for each column), First Column, Last Column, Banded Rows, Banded Columns, and Filter Button
- [ ] **1.0** Table Styles gallery, Clear, and New Table Style (a style you define for each part of a table); the Quick Styles with live preview
- [ ] **1.0** Summarize with PivotTable, Remove Duplicates, Convert to Range (keeping the style as plain formatting), and Insert Slicer for a table
- [ ] **1.0** A table grows when you type under it or beside it, a formula typed in one cell of a column fills the whole column (a calculated column), and a formula can point at a table's column by name (`=SUM(Sales[Amount])`, `[@Price]*[@Qty]`)
- [ ] **later** External Table Data: Export to a SharePoint list or Visio is not offered; Refresh and Unlink for a table loaded by a query (see Data)

### PivotTable Analyze and Design tabs (while a pivot table is selected)

A pivot table is a report that groups and totals a list for you: put a list of sales in it, drag "Region" to the rows and "Month" to the columns and "Amount" to the values, and it fills in the total for each region and month. *(pivot tables: ours 0%, public <1%, Enron 2% (250 workbooks), EUSES <1% (17), government <1% (9); POI 2% and LibreOffice 19% are test files)*

- [ ] **1.0** The field list pane: the fields of the source, and four boxes (Filters, Columns, Rows, and Values) that fields are dragged between or checked into; Defer Layout Update; and a search box
- [ ] **1.0** Value Field Settings: how to total (sum, count, count numbers, average, max, min, product, standard deviation, variance, and their population forms), a custom name, a number format, and Show Values As (percent of grand, column, row, or parent total; difference from; running total; rank; index)
- [ ] **1.0** Field Settings: a custom name, subtotals (automatic, custom, or none), layout and print choices (repeat item labels, blank row after each item, show items with no data), and sort and filter by label, by value, and by top 10
- [ ] **1.0** Group Selection and Group Field: dates by seconds, minutes, hours, days, months, quarters, and years; numbers by a start, an end, and a step; text items into groups you name; Ungroup
- [ ] **1.0** Expand and Collapse (the plus and minus buttons), Drill Down and Drill Up, double-click a total to see the rows behind it on a new sheet
- [ ] **1.0** Calculated Field (a field computed from others), Calculated Item, Solve Order, and List Formulas
- [ ] **1.0** Refresh and Refresh All (rebuild from the source, keeping the layout and column widths), Change Data Source, refresh when the file opens, and Clear (all, or filters only); Move PivotTable; Select (labels, values, or the whole table)
- [ ] **1.0** PivotTable Options: name, layout and format (merge cells with labels, "for error values show," "for empty cells show," autofit column widths on update, preserve cell formatting), totals and filters (grand totals for rows and columns, filter items in several fields), display, printing, and data (save the source data with the file, refresh on open)
- [ ] **1.0** Design tab: Subtotals (do not show, show at the top, show at the bottom), Grand Totals (off, on for rows and columns, rows only, or columns only), Report Layout (compact, outline, or tabular; repeat or don't repeat item labels), Blank Rows, row and column headers, banded rows and columns, and the PivotTable Styles gallery with New PivotTable Style
- [ ] **1.0** Report Filter pages: show each value of a filter as its own sheet
- [ ] **1.0** GETPIVOTDATA: a formula that reads a number out of a pivot table by field names; it is written for you when you click a pivot table cell while typing a formula (Generate GetPivotData turns that on or off)
- [ ] **1.0** Slicers and timelines connected to a pivot table; Filter Connections (see Slicer and Timeline)
- [ ] **later** Pivot tables over the data model (several related tables), measures in DAX, named sets, and OLAP cubes: kept as saved, not built *(data model: ours 0%, public 0%)*

### PivotChart tabs (while a pivot chart is selected)

- [ ] **1.0** PivotChart Analyze: show or hide field buttons (axis, legend, report filter, value), expand and collapse buttons, and the field list, plus the pivot table's refresh, change source, and clear commands; the PivotChart changes with its pivot table, and its formatting stays after a refresh
- [ ] **1.0** PivotChart tab and Format: the same as the Chart tab and Chart Format (below)

### Chart and Chart Format tabs (while a chart is selected)

- [~] **first release** Chart elements for the common kinds: Chart Title, Axis Titles, Legend (top, bottom, left, right, or overlay), Data Labels, Axes (primary and secondary, horizontal and vertical), and Gridlines (major and minor), shown, kept, and changed; the Chart Parts, Looks, and Chart Filters buttons beside a selected chart *(done: Chart Title (above or over), Axes, Axis Titles, Gridlines (major and minor), Legend (right, top, left, bottom), and Data Labels (values or percentages, with a position) of the first axes, from the + button beside the chart and from Chart Parts on the Chart tab; a brush button gives its colors and looks; the elements of a second axis, the legend over the plot, and the Chart Filters button are not offered yet. A chart is changed only when a chart written from the model loses nothing of it (no trend lines, picture fills, or 3-D; gradients, labels of single points, and date axes are kept); those can still be moved, sized, ordered, and deleted)*
- [x] **first release** Chart Data: the list of series (each with a name, values, and X values from cells), the horizontal axis labels, Swap Rows and Columns, Edit and Add and Remove series, and Hidden and Empty Cells (show gaps, zero, or connect with a line; include hidden rows and columns)
- [~] **first release** Chart Kind, including a different type for each series (a combo chart), and Move Chart: onto its own chart sheet or into any sheet, kept as a chart sheet when the file has one *(chart sheets: Enron 3%; done: the whole chart or each series as a kind, a second axis for a series; Move Chart onto a chart sheet is not offered)*
- [x] **1.0** Layouts (the ready arrangements of title, legend, labels, and axes) and Colors (a color set for the series); the Looks gallery *(four layouts, eight sets of colors, and four looks of our own)*
- [ ] **1.0** Format panes for an axis (bounds, major and minor units, a log scale, reverse order, where the other axis crosses, display units, number format, tick marks, label position, text direction), for a series (gap width, overlap, secondary axis, smooth line, marker shape and size, fill, border, explosion of a pie slice, angle of the first slice, hole size), and for a point
- [ ] **1.0** Data labels: value, category name, series name, percentage, legend key, a separator, position (inside, outside, center, best fit), a number format, leader lines, and "value from cells" (labels read from a range)
- [ ] **1.0** Trendlines (linear, exponential, logarithmic, polynomial of order 2 to 6, power, and moving average; forecast forward and backward; set the intercept; show the equation and R-squared), Error Bars (fixed value, percentage, standard deviation, standard error, and custom), Data Table (the values under the chart), Drop Lines, High-Low Lines, Series Lines, and Up/Down Bars
- [~] **1.0** Format tab: Current Selection (a list of the chart's parts and the Format Selection pane, Reset to Match Style), Shape Styles, WordArt Styles, Insert Shapes, text boxes and pictures drawn on the chart, and Size and Arrange *(shared: JeffPub's shape and Text Art groups, and the chart format panes of JeffDeck)*
- [~] **1.0** Chart area and plot area fills (solid, gradient, picture, pattern), borders, shadow, glow, soft edges, 3-D format and rotation for the 3-D kinds, and wall, floor, and side-wall choices *(shared: libs/draw effects)*
- [ ] **1.0** A chart saved as a template (`.crtx`) and applied again; Save as Picture
- [ ] **1.0** Chart text: titles and labels that link to a cell (type `=Sheet1!A1` in the formula bar), rich text in titles, and axis labels that wrap and rotate
- [~] **1.0** A chart scales and moves with its cells as the file says (move and size with cells, move but don't size, or don't move or size with cells), and prints or hides as set *(the Properties of an object choose and save it, for a chart, picture, or shape; printing or hiding the object is not offered)*
- [~] **first release** A chart's look comes from its style, theme colors, and fonts as saved; the same chart draws the same way in JeffCrunch, JeffDeck, and JeffWrite *(shared: libs/office chart)*

### Sparkline tab (while a sparkline is selected)

- [ ] **1.0** Edit Data (the location and the source range of one sparkline or a group), Type (Line, Column, or Win/Loss), and Show (High Point, Low Point, Negative Points, First Point, Last Point, and Markers)
- [ ] **1.0** Style and Sparkline Color, Marker Color (for each kind of point), and line weight
- [ ] **1.0** Axis: date axis, show axis, plot data right to left, minimum and maximum (automatic, the same for all, or custom), and Hidden and Empty Cells; Group, Ungroup, and Clear

### Slicer and Timeline tabs

- [ ] **1.0** Slicer tab: caption, Report Connections (the tables and pivot tables it filters), settings (sort ascending or descending or by a custom list, hide items with no data, show items deleted from the source), Slicer Styles and New Slicer Style, the number of columns, and button and slicer sizes
- [ ] **1.0** Timeline tab: caption, Report Connections, Timeline Styles, and what shows (header, scroll bar, selection label, time level) with a level of years, quarters, months, or days

### Picture Format tab (while a picture is selected)

- [~] **1.0** Corrections (sharpen and soften, brightness and contrast), Color (saturation, tone, recolor), Artistic Effects, Transparency, Compress Pictures, Change Picture, Reset Picture, and Remove Background *(shared: libs/draw picture effects, JeffPub's Picture Tools)*
- [~] **1.0** Picture Styles, Picture Border, Picture Effects (shadow, reflection, glow, soft edges, bevel, 3-D rotation), and Picture Layout *(shared: JeffPub's Picture Tools)*
- [~] **1.0** Crop (to a shape, to an aspect ratio, fill, and fit), Size, Arrange, and Alt Text *(shared: JeffPub's Picture Tools)*
- [~] **1.0** A picture's properties: move and size with cells, move but don't size, or don't move or size with cells; locked; print object; the picture as a cell's fill or as the sheet's background *(the choice is made in Properties and saved; locked, print object, and a picture as a cell's fill or the sheet's background are not offered)*

### Shape Format, Text Box, and WordArt tabs

- [~] **1.0** Insert Shapes, Edit Shape (change shape, edit points, reroute connectors), Shape Styles, Shape Fill, Shape Outline, and Shape Effects *(shared: JeffPub's Drawing Tools)*
- [~] **1.0** WordArt Styles, Text Fill, Text Outline, and Text Effects; Align, Group, Rotate, Bring Forward, Send Backward, and the Selection Pane *(shared: JeffPub's Text Art Tools and Arrange group)*
- [~] **1.0** Text in a shape or text box: margins, vertical alignment, autofit and wrap, columns, text direction, and alt text *(shared: JeffPub's Text Box Tools)*
- [ ] **1.0** A shape can show a cell's value: type `=A1` in the formula bar with a shape selected (and in a text box), and the text follows the cell

### Header & Footer tab (while editing a header or footer)

- [ ] **first release** The header and footer each have a left, a center, and a right part, edited in the Page Layout view or in the Page Setup dialog, with the codes the original saves (`&P` page, `&N` pages, `&D` date, `&T` time, `&F` file, `&Z` path, `&A` sheet name, `&G` picture, `&B` bold, `&I` italic, `&U` underline, `&"font"`, `&12` size, `&K` color, and `&&` for an ampersand) *(headers and footers: Enron 95%, ours <1% (.xlsx); the converted collections read 100%, not real)*
- [ ] **1.0** Header and Footer galleries of built-in forms (Page 1, Page 1 of ?, the sheet name, the file name, the author and date, and others); the insert buttons (Page Number, Number of Pages, Current Date, Current Time, File Path, File Name, Sheet Name, Picture) with Format Picture; Go to Header and Go to Footer
- [ ] **1.0** Different First Page, Different Odd & Even Pages, Scale with Document, and Align with Page Margins

### Equation, SmartArt, 3-D Model, and Ink tabs

- [ ] **1.0** Equation tab: structures (fraction, script, radical, integral, large operator, bracket, function, accent, limit, operator, matrix), symbols, professional and linear forms, and ink equations *(shared: the equation engine to build for JeffWrite and JeffDeck)*
- [~] **later** SmartArt Design and Format: choose a layout, change colors and styles, add and demote shapes, and edit text in the text pane *(shared: JeffDeck's SmartArt)*
- [ ] **later** 3-D Model Format: views, rotate, and scene; a model is kept as saved and drawn as its stored picture
- [ ] **later** Ink Tools and Background Removal tabs (see Draw and Picture Format)

## Functions

Every worksheet function a workbook can use, by category: 482 names, and 36 more that exist only in Microsoft 365 (518 in all). "Classic" means the function was in Excel 2007; "newer" means 2010 or later. Public workbooks use 230 of the 518 names (and a few old macro-sheet calls); 105 are used by ten or more workbooks. The list is long on purpose: a workbook that uses one function JeffCrunch lacks gets a wrong answer, as soon as anything is recalculated, in every cell that depends on it. LibreOffice's test suite holds one self-checking file for each of 447 of the 482 (each file ends in a cell that reads TRUE when every test row passes); the 35 without a file include the functions that change on every recalculation (NOW, TODAY, RAND, and RANDBETWEEN), the cube functions, and the web function, which need checks of our own.

The evidence for a function reads "of formula workbooks": the share of workbooks that have at least one formula and use that function.

- [ ] **first release** Math and trigonometry, the classic 60: SUM, ROUND, SUBTOTAL, SUMIF, ABS, SUMPRODUCT, SQRT, LN, INT, EXP, ROUNDUP, PI, RAND, PRODUCT, MMULT, POWER, ROUNDDOWN, LOG, SUMSQ, COS, SIN, MINVERSE, MOD, CEILING, TRUNC, LOG10, RADIANS, ATAN, COMBIN, FLOOR, SIGN, ACOS, ASIN, ATAN2, TAN, DEGREES, FACT, EVEN, MDETERM, QUOTIENT, RANDBETWEEN, ACOSH, ASINH, ATANH, COSH, FACTDOUBLE, GCD, LCM, MROUND, MULTINOMIAL, ODD, ROMAN, SERIESSUM, SINH, SQRTPI, SUMIFS, SUMX2MY2, SUMX2PY2, SUMXMY2, TANH *(public workbooks use 41 of these; the most used are called out below)*
- [ ] **1.0** Math and trigonometry, the newer 21 (from Excel 2010 and later): ACOT, ACOTH, AGGREGATE, ARABIC, BASE, CEILING.MATH, CEILING.PRECISE, COMBINA, COT, COTH, CSC, CSCH, DECIMAL, FLOOR.MATH, FLOOR.PRECISE, ISO.CEILING, MUNIT, RANDARRAY, SEC, SECH, SEQUENCE; SEQUENCE and RANDARRAY are dynamic-array functions (see below)
- [ ] **first release** Statistical, the classic 45: AVERAGE, MIN, MAX, COUNTIF, COUNTA, COUNT, AVERAGEA, MEDIAN, CORREL, FREQUENCY, LINEST, SMALL, INTERCEPT, SLOPE, LARGE, COUNTBLANK, MINA, FORECAST, KURT, MAXA, SKEW, GEOMEAN, RSQ, AVEDEV, STDEVPA, PEARSON, STDEVA, STEYX, AVERAGEIF, AVERAGEIFS, COUNTIFS, DEVSQ, FISHER, FISHERINV, GAMMALN, GROWTH, HARMEAN, LOGEST, PERMUT, PROB, STANDARDIZE, TREND, TRIMMEAN, VARA, VARPA
- [ ] **first release** Compatibility, the 38 older names Excel still reads and writes (the new ones replaced them in 2010): STDEV, NORMSDIST, NORMDIST, RANK, VAR, MODE, NORMINV, NORMSINV, STDEVP, TINV, QUARTILE, COVAR, CONFIDENCE, TDIST, BINOMDIST, CHIDIST, PERCENTILE, CHIINV, VARP, BETAINV, CHITEST, FDIST, FINV, GAMMADIST, LOGINV, PERCENTRANK, BETADIST, CRITBINOM, EXPONDIST, FTEST, GAMMAINV, HYPGEOMDIST, LOGNORMDIST, NEGBINOMDIST, POISSON, TTEST, WEIBULL, ZTEST
- [ ] **1.0** Statistical, the newer 66 (Excel 2010 and later), including the `.DIST`, `.INV`, `.TEST`, `.S`, and `.P` forms of the distributions, the forecasting functions, and MAXIFS and MINIFS: BETA.DIST, BETA.INV, BINOM.DIST, BINOM.DIST.RANGE, BINOM.INV, CHISQ.DIST, CHISQ.DIST.RT, CHISQ.INV, CHISQ.INV.RT, CHISQ.TEST, CONFIDENCE.NORM, CONFIDENCE.T, COVARIANCE.P, COVARIANCE.S, EXPON.DIST, F.DIST, F.DIST.RT, F.INV, F.INV.RT, F.TEST, FORECAST.ETS, FORECAST.ETS.CONFINT, FORECAST.ETS.SEASONALITY, FORECAST.ETS.STAT, FORECAST.LINEAR, GAMMA, GAMMA.DIST, GAMMA.INV, GAMMALN.PRECISE, GAUSS, HYPGEOM.DIST, LOGNORM.DIST, LOGNORM.INV, MAXIFS, MINIFS, MODE.MULT, MODE.SNGL, NEGBINOM.DIST, NORM.DIST, NORM.INV, NORM.S.DIST, NORM.S.INV, PERCENTILE.EXC, PERCENTILE.INC, PERCENTRANK.EXC, PERCENTRANK.INC, PERMUTATIONA, PHI, POISSON.DIST, QUARTILE.EXC, QUARTILE.INC, RANK.AVG, RANK.EQ, SKEW.P, STDEV.P, STDEV.S, T.DIST, T.DIST.2T, T.DIST.RT, T.INV, T.INV.2T, T.TEST, VAR.P, VAR.S, WEIBULL.DIST, Z.TEST
- [ ] **first release** Logical, the classic 7: IF, AND, OR, FALSE, TRUE, NOT, IFERROR
- [ ] **1.0** Logical, the newer 5: IFNA, IFS, LET, SWITCH, XOR (LET gives names to values inside one formula)
- [ ] **first release** Lookup and reference, the classic 16: VLOOKUP, MATCH, INDEX, HLOOKUP, OFFSET, LOOKUP, CHOOSE, ROWS, INDIRECT, TRANSPOSE, ROW, ADDRESS, HYPERLINK, COLUMN, AREAS, COLUMNS
- [ ] **1.0** Lookup and reference, the newer: GETPIVOTDATA, FILTER, FORMULATEXT, SORT, SORTBY, UNIQUE, XLOOKUP, XMATCH (FILTER, SORT, SORTBY, UNIQUE, XLOOKUP, and XMATCH are dynamic-array functions or their companions; GETPIVOTDATA needs pivot tables)
- [ ] **later** RTD (real-time data from a program that serves it through COM): kept as written, shows the stored value
- [ ] **first release** Text, the classic 23: CONCATENATE, TEXT, LEFT, VALUE, MID, FIND, LEN, TRIM, RIGHT, UPPER, REPT, CHAR, FIXED, REPLACE, EXACT, PROPER, T, DOLLAR, SEARCH, SUBSTITUTE, CLEAN, CODE, LOWER
- [ ] **1.0** Text, the newer: CONCAT, NUMBERVALUE, TEXTJOIN, UNICHAR, UNICODE; and the double-byte and byte-counting forms for East Asian text: ASC, BAHTTEXT, DBCS, FINDB, LEFTB, LENB, MIDB, PHONETIC, REPLACEB, RIGHTB, SEARCHB
- [ ] **first release** Date and time, the classic 21: NOW, TODAY, MONTH, YEAR, WEEKDAY, EOMONTH, DATE, WORKDAY, DAY, DATEVALUE, EDATE, YEARFRAC, DAYS360, HOUR, NETWORKDAYS, TIME, DATEDIF, MINUTE, WEEKNUM, SECOND, TIMEVALUE
- [ ] **1.0** Date and time, the newer: DAYS, ISOWEEKNUM, NETWORKDAYS.INTL, WORKDAY.INTL
- [ ] **first release** Financial, the classic 53 (the loan and investment functions, the depreciation functions, and the bond and Treasury bill functions): NPV, PMT, XIRR, IRR, XNPV, PV, PPMT, SLN, IPMT, MDURATION, PRICE, VDB, YIELD, RATE, FV, ACCRINT, ACCRINTM, AMORDEGRC, AMORLINC, COUPDAYBS, COUPDAYS, COUPDAYSNC, COUPNCD, COUPNUM, COUPPCD, CUMIPMT, CUMPRINC, DB, DDB, DISC, DOLLARDE, DOLLARFR, DURATION, EFFECT, FVSCHEDULE, INTRATE, ISPMT, MIRR, NOMINAL, NPER, ODDFPRICE, ODDFYIELD, ODDLPRICE, ODDLYIELD, PRICEDISC, PRICEMAT, RECEIVED, SYD, TBILLEQ, TBILLPRICE, TBILLYIELD, YIELDDISC, YIELDMAT
- [ ] **1.0** Financial, the newer: PDURATION, RRI
- [ ] **first release** Information, the classic 17: CELL, ISNUMBER, NA, ISERROR, ISNA, ISTEXT, ISBLANK, ISERR, N, ERROR.TYPE, INFO, ISEVEN, ISLOGICAL, ISNONTEXT, ISODD, ISREF, TYPE
- [ ] **1.0** Information, the newer: ISFORMULA, SHEET, SHEETS
- [ ] **first release** Database, all 12 (a database here is a list with a header row and a criteria range): DAVERAGE, DSUM, DCOUNT, DCOUNTA, DMAX, DMIN, DSTDEV, DSTDEVP, DGET, DPRODUCT, DVAR, DVARP
- [ ] **1.0** Engineering, all 54: base conversions (BIN2DEC and the rest), bit functions, complex-number functions (IMSUM and the rest), Bessel functions, ERF, CONVERT, DELTA, and GESTEP: BESSELI, BESSELJ, BESSELK, BESSELY, BIN2DEC, BIN2HEX, BIN2OCT, BITAND, BITLSHIFT, BITOR, BITRSHIFT, BITXOR, COMPLEX, CONVERT, DEC2BIN, DEC2HEX, DEC2OCT, DELTA, ERF, ERF.PRECISE, ERFC, ERFC.PRECISE, GESTEP, HEX2BIN, HEX2DEC, HEX2OCT, IMABS, IMAGINARY, IMARGUMENT, IMCONJUGATE, IMCOS, IMCOSH, IMCOT, IMCSC, IMCSCH, IMDIV, IMEXP, IMLN, IMLOG10, IMLOG2, IMPOWER, IMPRODUCT, IMREAL, IMSEC, IMSECH, IMSIN, IMSINH, IMSQRT, IMSUB, IMSUM, IMTAN, OCT2BIN, OCT2DEC, OCT2HEX *(engineering functions: public workbooks 0, ours 0; POI and LibreOffice test them)*
- [ ] **1.0** Web: ENCODEURL and FILTERXML (FILTERXML reads a value out of XML text with an XPath expression)
- [ ] **later** Web: WEBSERVICE (fetches a web address; needs a network, and the original offers it only on Windows). A workbook that holds it keeps the formula and the stored result
- [ ] **later** Cube, all 7: CUBEKPIMEMBER, CUBEMEMBER, CUBEMEMBERPROPERTY, CUBERANKEDMEMBER, CUBESET, CUBESETCOUNT, CUBEVALUE (read values from an OLAP cube or the data model; kept as written, showing the stored values)
- [ ] **later** Microsoft 365 only, 36 functions: LAMBDA (a function you write in a formula) and its helpers MAP, REDUCE, SCAN, BYROW, BYCOL, MAKEARRAY, and ISOMITTED; the array shapers CHOOSECOLS, CHOOSEROWS, DROP, TAKE, EXPAND, HSTACK, VSTACK, TOCOL, TOROW, WRAPCOLS, and WRAPROWS; the text splitters TEXTBEFORE, TEXTAFTER, TEXTSPLIT, VALUETOTEXT, and ARRAYTOTEXT; the newest GROUPBY, PIVOTBY, PERCENTOF, TRIMRANGE, and the regular-expression trio REGEXTEST, REGEXEXTRACT, and REGEXREPLACE; and the online ones (IMAGE, STOCKHISTORY, FIELDVALUE, TRANSLATE, DETECTLANGUAGE). The files that hold them are read, kept, and shown with their stored results; LibreOffice's test files cover 23 of the 36, which makes them cheap to add when wanted. *(none in the public workbooks, which predate them)*
- [ ] **later** Functions of other spreadsheet programs found in `.ods` files (LibreOffice's EASTERSUNDAY, DAYSINMONTH, ISLEAPYEAR, WEEKS, MONTHS, YEARS, ROUNDSIG, ROT13, FOURIER, REGEX, and the others): kept as written, shown with their stored results, and calculated once the open question on them is settled
- [ ] **first release** Old macro-sheet calls in `.xls` files (CALL, REGISTER.ID, DDE, and the data table's TABLE) are kept as written and show the stored result *(CALL and REGISTER.ID: 5 public workbooks; TABLE: 15)*

### The most used, one by one
- [ ] **first release** `SUM`: adds numbers in cells, ranges, 3-D ranges, and a list of arguments; skips text and blanks in a range, but counts a number typed as an argument and text typed as a number *(of formula workbooks: public 65%, Enron 73%, EUSES 69%, government 54%; ours 49%)*
- [ ] **first release** `IF` (nested up to 64 deep, with the untaken branch never run) *(public 24%, Enron 21%, EUSES 22%, government 28%; ours 6%)*, with `AND` *(public 5%, government 8%)*, `OR` *(public 3%)*, and `NOT`
- [ ] **first release** `AVERAGE` *(public 12%, Enron 11%, EUSES 13%, government 12%; ours 5%)*, `MAX` and `MIN` *(public 5% each)*, `COUNT` *(public 3%)*, and `COUNTA` *(public 2%, ours 8%)*
- [ ] **first release** `VLOOKUP` (exact match, approximate match on sorted data, and wildcards in an exact match) *(public 7%, Enron 9%, EUSES 5%, government 8%; ours 6%)*, with `HLOOKUP` *(public 1%)*, `LOOKUP`, `INDEX` *(public 2%)*, and `MATCH` *(public 2%)*
- [ ] **first release** `NOW` *(public 6%, Enron 16% (about 1 in 6))* and `TODAY` *(public 3%, Enron 6%)*: they change every time the workbook calculates, so a workbook that holds one is always "changed" and asks to be saved
- [ ] **first release** `ROUND` *(public 5%, Enron 7%)*, `ROUNDUP`, `ROUNDDOWN`, `INT`, `TRUNC`, `CEILING`, `FLOOR`, and `MROUND`, rounding as the original does (half away from zero, with the same results at the edge of floating-point error, such as `ROUND(2.675, 2)` giving 2.68 and `INT(-0.5)` giving -1)
- [ ] **first release** `SUBTOTAL` (the functions 1 to 11, which count rows hidden by hand, and 101 to 111, which skip them; all skip rows a filter hides and any SUBTOTAL inside the range) *(public 3%, Enron 4%, government 4%)*, `SUMIF` *(public 2%, Enron 4%)*, `COUNTIF` *(public 2%, ours 7%)*, and `SUMPRODUCT` *(public 2%)*; the criteria text language (`">5"`, `"<>"`, `"a*"`, `"?b"`, and `"~*"`) matches the original's
- [ ] **first release** `SUMIFS`, `COUNTIFS`, `AVERAGEIF`, `AVERAGEIFS`, and `IFERROR`: 2007 additions *(ours 1% each; public 0%, since the public files predate them)*
- [ ] **first release** `SQRT` *(public 2%)*, `ABS` *(public 2%)*, `LN` *(public 1%)*, `EXP`, `MOD`, `POWER`, `LOG`, `PI`, and the trigonometry set, with errors (`#NUM!` for the square root of a negative, `#DIV/0!` for a division by zero) as the original gives them
- [ ] **first release** `MONTH`, `YEAR`, `DAY`, `DATE`, `WEEKDAY`, `EOMONTH`, `EDATE`, `WORKDAY`, and `DATEDIF` *(MONTH and YEAR: public 2%, Enron 3% to 4%; DATE 1%; WEEKDAY 2%; EOMONTH: Enron 2%)*, using serial numbers (a date is a whole number of days; the time is the fraction) with the 1900 leap-year quirk kept and the 1904 option
- [ ] **first release** `CONCATENATE` and the `&` operator *(public 2%)*, `LEFT`, `MID` *(public 2%, government 4%)*, `RIGHT`, `LEN`, `FIND`, `SEARCH`, `TRIM`, `UPPER`, `LOWER`, `PROPER`, `SUBSTITUTE`, `REPT`, and `VALUE` *(ours: LEFT and RIGHT 6% each)*
- [ ] **first release** `TEXT` (turns a number into text with a format code, so it needs the whole number-format language; public 1%, Enron 2%)
- [ ] **first release** `ISNUMBER` *(public 2%)*, `ISERROR` *(public 1%)*, `ISNA`, `ISBLANK`, `NA`, and `IFERROR`, with each of the error values (`#N/A`, `#VALUE!`, `#REF!`, `#DIV/0!`, `#NUM!`, `#NAME?`, `#NULL!`, and `#SPILL!`) behaving as in the original
- [ ] **first release** `OFFSET` *(public 1%)* and `INDIRECT`: they build a reference from numbers or text, so they recalculate on every change and cannot be traced by the dependency list alone (see the engine); `CHOOSE`, `ROW`, `COLUMN`, `ADDRESS`, and `TRANSPOSE` too
- [ ] **first release** `CELL` *(public 2%, Enron 4%)* and `INFO`: the file name, sheet, format, width, and type of a cell; `CELL("filename")` gives the path and sheet name as the original writes it
- [ ] **first release** `HYPERLINK` *(ours 35%: 34 of our 98 workbooks that have any formula; public <1%)*: a clickable link built by a formula
- [ ] **first release** The loan and investment functions `NPV`, `IRR`, `XIRR`, `XNPV`, `PMT`, `PV`, `FV`, `RATE`, `NPER`, `SLN`, `VDB`, and `DB` *(each under 1% of public workbooks; XIRR and XNPV in the Enron models; ours: PMT 1%)*, which solve for a value by repeating a guess, so they must stop at the same answer to the same number of digits
- [ ] **first release** `STDEV` *(public 2%, EUSES 3%)*, `MEDIAN`, `CORREL`, `SLOPE`, `INTERCEPT`, `LINEST`, `FREQUENCY`, `LARGE`, `SMALL`, and `RANK`
- [ ] **first release** Array functions `MMULT`, `MINVERSE`, `MDETERM`, `TRANSPOSE`, `FREQUENCY`, `LINEST`, `LOGEST`, `TREND`, and `GROWTH`, entered as an array formula (a result that fills a block of cells, in braces `{=...}`) *(array formulas: ours 0%, public <1%, Enron 1%; the converted collections read 0%, not real)*
- [ ] **1.0** Dynamic-array functions, the newest kind: `FILTER` (rows that meet a test), `SORT` and `SORTBY`, `UNIQUE`, `SEQUENCE` (a grid of numbers), `RANDARRAY`, `XLOOKUP` (finds a value in any direction, with a "not found" answer and a choice of match), and `XMATCH`; and `LET` (names inside a formula). A formula whose result is a list fills the cells below and beside it (a "spill"), and every file that holds one is read and shown as saved already in the first release *(dynamic array results: ours 0%, public 0%, POI <1%, LibreOffice 2%)*
- [ ] **1.0** Function names in the language of the interface (SUMME, SOMME, SUMA, SOMMA, SOM, and SOMA, and the rest, for the seven languages of the interface), with the English names always stored in the file; argument separators follow the language (`;` where the comma is the decimal mark)
- [ ] **1.0** Calling a function that does not exist in this version shows `#NAME?`, keeps the typed text, and (for an unknown function in a file) keeps the stored result and writes the formula back as it was

## Shortcut menus (right-click)

What a right-click offers depends on what is under the pointer.

- [ ] **first release** Shortcut menus for a cell, a row heading, a column heading, a sheet tab, a selected object, a chart and each of its parts, a table, and a pivot table, with the commands the original offers there
- [ ] **1.0** The Mini toolbar that floats beside a selected cell's text, and the Quick Analysis button that appears under a selected range (formatting, charts, totals, tables, and sparklines in one pane)

## Under the ribbon: the calculation engine

How big workbooks are: the middle one in each public collection holds about 1,000 cells (government 1,700); one in ten holds more than 27,000 (Enron), 6,900 (EUSES), or 63,000 (government); the largest holds 2.0 million (Enron), 3.3 million (EUSES), or 2.2 million cells (government), and one government workbook has 163 sheets. The Enron `.xlsx` alone hold 20.7 million formulas, EUSES 1.1 million, and the government files 1.9 million.

### Dependency tracking and recalculation
- [ ] **first release** A dependency graph of what each formula reads (cells, ranges, names, table columns, other sheets, and other workbooks), so a change recalculates only the formulas that depend on it, in an order that gives the original's answers, with a big range tracked as a range rather than as a million cells
- [ ] **first release** Calculation on open: formulas keep the results the file stored, and are recalculated only when the file asks (it was saved by an older version or by another program, or it is flagged to calculate fully on load) or when a volatile function is in it; the same recalculation runs when you press F9 (see Fidelity and testing) *(in 600 Enron workbooks picked at random, all written by Excel, 19% carry the "calculate fully on load" flag)*
- [ ] **first release** Volatile functions (NOW, TODAY, RAND, RANDBETWEEN, OFFSET, INDIRECT, CELL, and INFO) recalculate at every change, and anything that depends on them with them; a workbook holding one is marked as changed *(NOW and TODAY: public 6% and 3% of formula workbooks, Enron 16% and 6%; OFFSET: public 1%; INDIRECT: public <1%)*
- [ ] **first release** Calculation modes (automatic, automatic except data tables, manual) and Calculate Before Save, kept as saved; in manual mode the status bar says "Calculate" when results are out of date *(manual: ours 0%, public 4%, Enron 11% (1,804 workbooks))*
- [ ] **first release** Iterative calculation: when switched on, a circular reference (a formula that depends on itself) repeats up to the chosen number of times (100 by default) until the change is under the chosen limit (0.001), and gives the same result as the original; when off, a warning names the cell and the formula shows the last value or 0 as the original does *(iterative calculation: ours 0%, public 2%, Enron 5% (736 workbooks))*
- [ ] **1.0** Recalculation on all cores for big workbooks, with the number of threads in Options, giving the same answers as one thread
- [ ] **first release** One engine for everything that holds a formula: cell formulas, conditional-format rules, data-validation rules, defined names, chart series, text-box links, and form-control links
- [ ] **first release** Stored results kept with every formula, with their type (number, text, true or false, or an error), so a function JeffCrunch doesn't have, a link to a closed workbook, or a query still shows the number the file held, and a saved file carries each formula's current result

### Numbers, dates, and text
- [ ] **first release** Numbers are 64-bit floating point with 15 significant digits kept and shown, as the original does, including its rules at the edges: an addition or subtraction whose result is almost zero next to the numbers it started from is shown as exactly zero (with the original's exceptions), a whole number typed with more than 15 digits keeps only the first 15 and zeros after them, and the largest and smallest numbers are the same
- [ ] **1.0** Set precision as displayed: stored values are rounded to the shown decimals (a one-way choice with a warning)
- [ ] **first release** The two date systems: the 1900 system, in which Excel counts a day that never existed, February 29, 1900 (serial number 60), and the 1904 system that Mac files used; a serial number shows as a date in the file's own system and converts when copied between workbooks of different systems *(1904 dates: ours <1%, public <1%, Enron <1% (33 workbooks), POI 5%)*
- [ ] **first release** The error values `#NULL!`, `#DIV/0!`, `#VALUE!`, `#REF!`, `#NAME?`, `#NUM!`, `#N/A`, and `#SPILL!`, read and written as stored, shown as typed, passed through arithmetic and functions as the original does (the first error in an expression wins, in the original's order), and `#GETTING_DATA`, `#CALC!`, `#FIELD!`, `#BLOCKED!`, `#UNKNOWN!`, and `#CONNECT!` kept when a file holds them. Errors are common in real workbooks: of 1.5 million formula results in 1,500 workbooks picked at random from the Enron files, 9% were errors, and more than half of those were `#NAME?` (a name or add-in function that was missing), then `#N/A` (27%), `#REF!` (13%), `#DIV/0!` (4%), and `#VALUE!` (1%).
- [ ] **first release** Operators: `+ - * / ^ &`, percent, negation, comparison (text compares without regard to case; text sorts after numbers, and true after text), and the reference operators colon (a range), space (the cells two ranges share), and comma (a union); precedence as the original has it (negation binds tighter than the power, so `=-2^2` is 4)
- [ ] **first release** Conversions the original does silently: text that looks like a number becomes a number in arithmetic (`"3"+"4"` is 7), true and false become 1 and 0, an empty cell is 0 in arithmetic and empty text in joining, and the text a user typed is parsed by the locale (`1,5` in German)
- [ ] **first release** Locale: the decimal mark, thousands mark, list separator, date order, and currency follow the system (or the workbook's choice), while formulas are stored with English function names and periods for decimals, so a workbook moves between countries without change
- [ ] **first release** Text up to 32,767 characters in a cell (the cell shows the first 1,024 in the original's grid; the formula bar shows all); formulas up to 8,192 characters, nested 64 deep, and up to 255 arguments; the original's limits are the limits, so no workbook overflows

### References, arrays, and links
- [ ] **first release** A1 and R1C1 references, relative, absolute, and mixed; whole-row and whole-column references; 3-D references across sheets (`Sheet1:Sheet9!B2`); names; sheet names with spaces and quotes handled as the original writes them
- [ ] **first release** Structured references to tables (`Table1[Sales]`, `Table1[[#Totals],[Sales]]`, `[@Price]`, `Table1[[Q1]:[Q4]]`) evaluated and kept as written
- [ ] **first release** Array formulas entered with Ctrl+Shift+Enter (shown in braces, a result that fills a block, and edited as one unit) and array evaluation inside functions such as SUMPRODUCT and IF *(array formulas: ours 0%, public <1%, Enron 1% (185 workbooks), POI 2%, LibreOffice 3%; the converted collections read 0%, not real)*
- [ ] **1.0** Dynamic arrays and spill: a formula whose result is a list fills the cells below and beside it, the block follows the formula, `A1#` points to the whole block, and `#SPILL!` appears when something is in the way; a file from an older version is read with the implicit-intersection mark `@` where the old rule applied *(dynamic array results: ours 0%, public 0%, POI <1%, LibreOffice 2%)*
- [ ] **first release** External references (`[Prices.xlsx]Sheet1!$A$1`, or a defined name in another workbook): the values the file stored for them are shown without opening the other workbook, and Edit Links updates them on request *(external links: ours <1%, public 8%, Enron 18% (2,786 workbooks), EUSES 3%, government 3%)*
- [ ] **first release** Shared formulas (one formula stored once for a block of cells, in the file) are read as one formula for each cell, and written as the original writes them *(shared formulas: ours 2%, Enron 35% (5,622 of 15,871); the converted collections read 0%, not real)*
- [~] **first release** The Name Box, defined names, and tables resolve when sheets are renamed, moved, or deleted, and a formula that points to a deleted sheet shows `#REF!` as the original does *(done in 4b-2b: formulas, defined names (those of one sheet too), rules, hyperlinks, and chart series follow a renamed or moved sheet, and what read a deleted sheet shows #REF!)*
- [ ] **later** Python in the sheet, the 365 feature, is not planned (see Left out on purpose)

## Under the ribbon: grid and editing

### The grid
- [~] **first release** The grid is drawn sharp at every zoom and on high-resolution screens: cell text, borders (including the double and the dashed kinds), diagonal borders, wrap and rotated text, overflow of text into empty cells next to it, and `####` when a number doesn't fit *(wave 4a: drawn at 10% to 400% with borders of all 13 styles, diagonals, wrapped and turned text, overflow, and #### when a number does not fit; a scroll at a zoom other than 100% draws the whole window))*
- [~] **first release** Column widths and row heights in the original's units, rounded as it rounds, so a wrapped paragraph breaks at the same word *(shared: the font-width tables of libs/text)*
- [~] **first release** Selecting: click, drag, Shift and Ctrl for several ranges, whole rows and columns, Ctrl+A (the current region, then the sheet), Ctrl+arrows to jump to the edge of data, End mode, Shift+Space and Ctrl+Space, F5 and the Name Box; the active cell stays shown in the Name Box with the selection's size *(wave 4a: click, drag, Shift, whole rows and columns, Ctrl+A, Ctrl+arrows, Shift+Space and Ctrl+Space, and the Name Box (type an address to go there) work; not yet Ctrl for several ranges, the current region, End mode, or F5))*
- [ ] **first release** Entering data: type in the cell or the formula bar; Enter, Tab, and arrow keys accept and move (Enter's direction is a choice); Esc cancels; `'` forces text; Alt+Enter breaks a line; Ctrl+Enter fills the selection with what you typed; Ctrl+; and Ctrl+Shift+; type today's date and the time; Alt+Down lists the column's values
- [ ] **first release** What a typed entry becomes, as the original decides: a number, a percent (`5%`), a currency (`$5`), a date (`3/4`, `4-Mar`, `March 2026`), a time, a fraction (`1 1/2`), scientific (`1E5`), a formula (starts with `=`, `+`, or `-`), or text, and the number format the cell takes with it; a text that only looks like a number or a date (a part number, a gene name) can be kept as text (see Beyond Excel)
- [~] **first release** AutoComplete for text in a column, and AutoCorrect as you type (replace text, fix two initial capitals, capitalize the first letter of sentences and day names, turn web addresses into links) *(shared: JeffPub's AutoCorrect)*
- [ ] **first release** Edit mode (F2 or double-click): the formula text with colored references, point mode (clicking or arrowing to a cell puts its address in the formula), F4 to cycle absolute and relative, Ctrl+Shift+Enter for an array formula, and the expanded formula bar (Ctrl+Shift+U) for long formulas
- [~] **first release** Undo and Redo with a history list, and Repeat (F4 or Ctrl+Y) *(shared: the editor's undo)*
- [ ] **first release** Drag a selection's border to move it (Ctrl to copy, Shift to insert, Alt to move to another sheet), the fill handle to copy or extend a series, and a double-click on the handle to fill down as far as the data to its left
- [~] **first release** Row and column insert, delete, move, and copy keep everything that points at them right: formulas, names, conditional formats, data validation, charts, merged cells, filters, pivot tables, print areas, hyperlinks, and notes *(done in 4b-2a for formulas, names, merged cells, tables, conditional formats, data validation, hyperlinks, page breaks, filters, charts, drawings, notes, and print areas and titles; pivot tables are not followed)*
- [ ] **first release** Sort and filter behave as the original: numbers before text before logicals before errors before blanks (blanks last in both directions), text compared by the language's collation without regard to case unless asked, a stable sort, formulas adjusting as the rows move, and a header row recognized by itself
- [ ] **1.0** Copy and paste of only the visible cells (Alt+;) in a filtered or hidden range, and pasting a copied range onto a filtered one fills only the visible rows
- [ ] **1.0** Select a block of cells by the type of content with Go To Special (also in Home), and select all objects; the Selection Pane lists the objects on a sheet and hides them one at a time
- [~] **first release** The status bar: Ready, Edit, Enter, and Point; Calculate; the sum, count, average, minimum, and maximum of the selection (a choice of which); the filter count ("5 of 200 records found"); and the zoom slider *(wave 4a: Ready, Calculating, the average, count, and sum of the selection, and the zoom slider; no Edit, Enter, Point, or filter count yet, and the choice of statistics is not offered))*
- [~] **first release** Everything works from the keyboard, with the original's shortcuts (Ctrl+Arrow, Ctrl+Shift+Arrow, Ctrl+Home, Ctrl+End, Ctrl+1 for Format Cells, Ctrl+D, Ctrl+R, Ctrl+Z, F2, F4, F5, F7, F9, Alt+=, and the rest), no two commands on one key, all reassignable in Options *(shared: JeffPub's shortcut table, KeyTips)*
- [ ] **1.0** Accessible to screen readers: the cell's address, value, formula, and format are read as the selection moves; sheets, tables, and charts have names and alt text; high contrast; no color-only meaning
- [ ] **later** Touch and pen input

## Under the ribbon: number formats

The format a cell shows its value in. Workbooks keep their own: the middle one in each public collection has 2 or 3 of its own, 1 in 10 has more than 7 to 9, and the largest holds 1,736 (Enron).

- [ ] **first release** The format-code language: the four sections (positive; negative; zero; text), digit placeholders (`0`, `#`, `?`), the decimal point, the thousands separator, and a trailing comma that divides by a thousand each, percent, scientific (`0.00E+00`), fractions (`# ?/?`, `# ??/??`, or a fixed denominator like `?/8`), quoted text and escapes, `*x` to repeat a character across the cell, `_x` to leave the width of a character, `@` for the cell's text, and conditions (`[>=1000]`) and colors (`[Red]`, `[Color 5]`) in brackets
- [ ] **first release** Dates and times: `d dd ddd dddd`, `m mm mmm mmmm mmmmm`, `yy yyyy`, `h hh`, `mm`, `ss`, `AM/PM`, `[h]:mm:ss` for elapsed time (hours beyond 24), and tenths and hundredths of a second; `m` means month or minute by where it sits, as in the original
- [ ] **first release** The built-in format numbers (0 to 49, with 14 to 22, 27 to 36, and 50 to 58 standing for the system's date and time formats) read from `.xls` and `.xlsx` and written back as they were; custom formats numbered from 164
- [ ] **first release** How a value is shown: General (up to 11 characters, then scientific), the rounding for display only (the stored value keeps its 15 digits), `####` when a number is wider than the column (and for a negative date), text overflowing into empty neighbors, and the minus sign, parentheses, and zero rules of each format
- [ ] **first release** Accounting and currency: symbol at the left edge, numbers aligned on the decimal, negatives in parentheses or red, the symbol per currency and country, and the euro with either order
- [ ] **1.0** Locale tags in a format (`[$-409]`, `[$-F800]` for the long system date, `[$€-407]`), the calendars (Hijri, Thai Buddhist, Japanese eras with `gg`, `e`, and `ggge`), and the East Asian forms (`[DBNum1]`, `[DBNum2]` for numerals written in Chinese or Korean characters)
- [ ] **first release** The same codes drive TEXT, FIXED, DOLLAR, chart axes and data labels, pivot values, and CSV export "as shown"

## Under the ribbon: printing and page layout

- [~] **first release** Pagination: the sheet is cut into pages by the paper, margins, scale, print area, repeated titles, manual breaks, and the page order, with the breaks falling where the original puts them for the same fonts; a page shows the cells whole (a row is not split) and wide text is clipped as in the original *(done October 24, 2026 as a library with bounds and tests; where the breaks fall against the original's: the page count is the same for 84.6% of 1,283 sheets (87.5% of those we print), the first word of a page for 72.4%)*
- [~] **first release** Header and footer fields (page, pages, date, time, file, path, sheet, picture) updated at print time, with separate first-page and even-page forms, and the margins they sit in *(done October 24, 2026 except the picture, which draws nothing)*
- [~] **first release** Gridlines, row and column headings, notes (at the end of the sheet or as displayed), errors as blank, black and white, and draft quality, printed as chosen *(done October 24, 2026 except notes, which are not read yet)*
- [x] **first release** Charts, pictures, and shapes print in place with their cells, or not, as each one's "print object" says *(done October 24, 2026; a draft leaves them out; a chart sheet prints as one page with its chart over the page's room)*
- [ ] **1.0** Print the pages a pivot table needs (repeat item labels, a page for each filter value), and print a selected table or chart alone

## Fidelity and testing

The collections are tests as well as evidence. Every saved workbook stores the last result of each formula next to the formula, so opening a workbook, recalculating every formula, and comparing with what the file stored checks the calculation engine against the stored answers, with no one needing to work out the right answer by hand. Together the collections hold about 23.7 million formulas (Enron's `.xlsx` 20.7 million, EUSES 1.1 million, government 1.9 million).

- [ ] **first release** Recalculate-and-compare: for each workbook, load it, recalculate every formula from the cells, and compare each result with the stored one: numbers to 15 significant digits, text and true or false exactly, and error values exactly (which error, not just "an error"). The report counts mismatches by function, by workbook, and by cell, and `jcrunchtool` runs it on a folder. Cells that depend on a volatile function (NOW, TODAY, RAND), on another workbook, or on a data connection are compared only where the stored result can still be reproduced; they are listed apart
- [ ] **first release** Compare against the originals, not the conversions: the stored results in the converted EUSES and government files are LibreOffice's, not Excel's, so those tests read the `.xls` files themselves through the new `.xls` reader (which keeps Excel's own stored results); the Enron `.xlsx` were all written by Excel, so they test directly *(600 sampled Enron workbooks: all name Excel as the program that wrote them)*
- [ ] **first release** Errors are tested as results: 9% of the formula results in the Enron files are errors (1.5 million formulas in 1,500 workbooks picked at random: `#NAME?` more than half, then `#N/A`, `#REF!`, `#DIV/0!`, and `#VALUE!`), so the kind of error and the way it passes through functions are checked as often as the numbers
- [ ] **first release** LibreOffice's function files: 509 files, one for each function, each self-checking (a cell reads TRUE when all its test rows agree); every one is opened and its result cell is checked. They cover 447 of the 482 functions and 23 of the 36 in Microsoft 365, and test odd cases (text where a number goes, errors in arguments, arrays, empty cells, wildcards). Where LibreOffice and Excel disagree, the answer Excel stored in a workbook decides, and the test notes it *(the 35 functions without a file: the volatile ones, the cube functions, WEBSERVICE, FILTERXML, PHONETIC, and about 20 more, each needing a test of our own)*
- [ ] **first release** Reference answers: recalculated values and the "as shown" text of every cell (a CSV saved from a workbook writes each cell as shown) give the answers for number formats, rounding, dates, and anything the stored results don't settle
- [ ] **first release** Round trip: open, save, and open again in JeffCrunch and compare the model; check that the saved `.xlsx` opens without a repair message and that every cell matches. Run over all `.xlsx` (Enron 15,871, ours 1,335, POI 366, LibreOffice 364) and over the `.xls` and `.ods` files after conversion to `.xlsx`
- [ ] **first release** LibreOffice's Calc test folder as format tests: 312 `.xlsx`, 305 `.ods`, 80 `.xls`, 8 `.xlsb`, 6 `.xlsm`, 9 `.csv`, 18 `.fods`, and the Lotus, Quattro Pro, Works, SYLK, DIF, dBASE, and Gnumeric files for the later formats; each opens and shows what the test file's name says it holds
- [~] **first release** Pages against reference pages: a set of workbooks has reference PDF pages; JeffCrunch's pages are compared page by page (page breaks, column widths, wrapped text, fills, borders, headers and footers, charts) with the same tools used for JeffPub and JeffDeck. The set is picked from the collections by features: print areas, titles, scale to fit, merged cells, wrapped text, charts, and notes *(`tools/crunch_print_score.py` against 360 practice workbooks: page count 84.6% of 1,283 sheets, picture difference median 3.28 (60% of pages under 4), first word of the page 72.4%, nothing printed where the original printed nothing 251 of 251)*
- [ ] **1.0** Charts: the 1,685 Enron, 340 EUSES, 81 government, and 28 POI workbooks with charts are drawn and compared with a reference picture of each, as JeffDeck's charts are
- [ ] **1.0** Pivot tables: the 250 Enron, 17 EUSES, 9 government, and 70 LibreOffice workbooks with pivot tables. First release: round trip keeps the table and its cache whole. 1.0: refresh each pivot table and compare its cells with the stored ones
- [ ] **1.0** Number formats: every format in the collections applied to its cell's stored value, compared with the "as shown" text of the reference answers
- [ ] **first release** Speed and memory: the largest workbooks (3.3, 2.2, and 2.0 million cells) open, scroll, recalculate, sort, and save within a memory cap (large jobs run under a cap) and a time limit that is recorded for each release
- [ ] **first release** CSV: all 1,267 of our `.csv` files and LibreOffice's 9 open, and save back the same bytes when unedited; the dialect guesser is scored on them
- [ ] **first release** Workbooks held out from tuning are scored apart from the ones used for tuning, so the numbers are not flattered by the files the engine and the layout were fitted to

## Safety and limits

Suite-wide rules decided October 11, 2026 (see Decided): nothing in a file runs or reaches out until the person allows it, every file reader is fuzzed and held to limits on memory and time, and a damaged file opens as far as it can without crashing.

- [~] **first release** The grid holds 1,048,576 rows by 16,384 columns (A1 to XFD1048576), stored sparsely, so memory follows the cells that hold something; a workbook of 3 million cells opens in the background while the first screen shows, and scrolling and typing never wait for the rest *(wave 4a: stored sparsely, and a workbook opens in the background while the first screen shows: a workbook of 2 million cells shows its first screen in under 100 ms, and the 151 MB sheet in 85 ms; typing is not there yet))*
- [~] **first release** A file that lies about itself (a part that inflates to gigabytes, a shared-strings table with a billion entries, a sheet with a dimension of millions of rows, a formula nested 10,000 deep) is stopped with a message, never a crash or a frozen window *(shared: the zip reader in libs/base/io)*
- [ ] **first release** External links do not update, data connections do not run, and macros do not run when a file opens, unless the person says so; a cell in an opened `.csv` that starts with `=`, `+`, `-`, or `@` is kept as text unless the person asks for formulas, so a hostile file can't run a formula or a command (the original runs them) *(cells starting with `=` in our `.csv`: 1 file of 1,267)*
- [ ] **first release** Recalculation and sorting of a large range can be cancelled with Esc, and a function that fails inside a formula (running out of memory, a bad argument) gives an error value, not a crash
- [ ] **first release** Damaged and unusual files: Apache POI's test files (366 `.xlsx`, of which 16 (4%) can't be opened as a zip package, being damaged or encrypted, and 417 `.xls`) and LibreOffice's (364 `.xlsx` and 304 `.xls`) must never crash the program; each opens with what can be read, or says why not. A hostile-file sweep (truncated, bomb-sized, and mutated files from these collections) runs before each release with a time and memory cap, as it does for JeffDeck
- [ ] **later** Fuzzing: files from the collections with random bytes changed, run against the readers

## File formats

What each format needs, to open, to save, and to come back unchanged. "Round trip" means open, save, and open again without a repair message or a visible change; for a spreadsheet it also means that every formula, and every number it had stored, is still there.

### Excel `.xlsx` and `.xltx` (Open XML)
- [ ] **first release** Open `.xlsx` from Excel 2007 to 2021 and Microsoft 365 (Transitional): sheets and their order, visibility, and tab colors; cell values of every type (number, shared string with rich-text runs, inline string, true or false, error, and date); formulas (ordinary, shared, array, and what-if table) with their stored results; styles (fonts, fills, borders, number formats, named cell styles, and the differential styles that conditional formats and tables use), the theme and its tints, the color palette; row heights, column widths and groups, merged cells, hyperlinks, notes with their drawings, and threaded comments; defined names with scope; data validation; conditional formatting (the 2007 form and the 2010 extension form); autofilter and sort state; tables; pivot tables and their caches; slicers, timelines, and sparklines (the 2010 extension forms); drawings (DrawingML shapes, pictures, charts, and SmartArt) and the older VML drawing; external links with their stored values; connections and query tables; custom views and scenarios; sheet and workbook protection; sheet views (zoom, selection, frozen and split panes, gridlines off); page setup, headers and footers, page breaks, and print areas; form controls; custom XML; and the document properties *(ours: 1,335 files; Enron 15,871; POI 366; LibreOffice 364)*
- [ ] **first release** Read big sheets as a stream, without building the whole page of XML in memory, so a sheet of 3 million cells and a shared-strings table of millions of entries open within the memory cap and in seconds, not minutes *(the largest public sheet: 3.3 million cells)*
- [ ] **first release** Save `.xlsx` so that every file opens without a repair message, checked on a large collection as JeffPub's `.pub` files were, with the same cell values, results, and formatting, and the pages printing the same
- [ ] **first release** Round trip keeps what JeffCrunch can't edit: pivot tables and caches, slicers, timelines, sparklines, the data model, Power Query definitions and their data mashup, custom XML and XML maps, form-control and ActiveX parts, the macro project of a macro-enabled file, rich data, printer settings, the metadata that marks dynamic arrays, the `x14`, `x15`, and `x16` extension lists, the `mc:AlternateContent` choices, and every part or attribute JeffCrunch doesn't know, written back as they were, and untouched parts kept byte for byte
- [ ] **first release** What a saved formula carries: the formula text in the file's own syntax (English names, periods, `_xlfn.` prefixes for newer functions), its stored result with the right type, shared formulas where the original would write them, and array formulas with their ranges; the file says to calculate fully on load whenever a stored result can't be trusted (for example, a function JeffCrunch doesn't have) *(workbooks that already say so: 19% of 600 Enron files sampled)*
- [ ] **first release** Open and save templates (`.xltx`) with their styles and sheets
- [ ] **first release** Open and save macro-enabled files (`.xlsm`, `.xltm`), keeping the macro project untouched and never running it; Save As offers `.xlsm` when the workbook came from a file with macros, so they aren't dropped without a word *(old files that hold macros: ours 1%, EUSES 8%, government 12%)*
- [ ] **1.0** Open encrypted files and save with a password (agile encryption, AES)
- [ ] **later** Open and save the binary workbook `.xlsb`, the Strict variant of Open XML, and Flat OPC (`.xml`) files *(`.xlsb` in LibreOffice's tests: 8 files)*

### Excel 97 to 2003 `.xls` (binary)
- [~] **first release** Open `.xls` from Excel 97 to 2003 (the "Workbook" stream of a compound file): cells (numbers in all their packed forms, shared-string text, rich text, true or false, errors, blanks, formulas with their stored results, array and shared formulas, what-if tables); formats, fonts, fills, borders, and the palette; styles; merged cells; column widths and row heights; outlines; defined names including print areas and titles; hyperlinks; notes; data validation; conditional formats; autofilter; page setup, headers and footers, page breaks; panes and views; sheet and workbook protection; scenarios; external links with stored values; the drawing layer (shapes, pictures, text boxes); embedded charts and chart sheets; pivot tables, kept; and form controls *(public: EUSES 4,334 of 4,652 (93%), government 1,195 of 1,304 (92%), all 58 Enron `.xls`; ours 133 of 146 (91%); shared: the compound-file reader in libs/base/io, the Escher drawing reader in libs/office; the BIFF record reader is new)*
- [ ] **first release** Open `.xls` from Excel 5.0 and 95 (a compound file with a "Book" stream, the older BIFF5 records and the 8-bit text of its code page) *(EUSES 6% (266), government 3% (40), ours 8% (12))*
- [ ] **first release** Charts in `.xls` (the chart records of a chart sheet or an embedded chart) are read into the same chart model as the `.xlsx` ones, and written to `.xlsx` as real charts
- [ ] **first release** Macro projects and OLE objects in an `.xls` are noticed and kept with the workbook, and Save as `.xlsx` says that the macros won't be kept, as the original does; keeping them in an `.xlsm` is 1.0 *(old files with a macro project: ours 1%, EUSES 8%, government 12%)*
- [ ] **first release** A file that is not a spreadsheet, or is damaged (a `.xls` with no workbook stream: 8 in EUSES, 18 in government, 1 of ours; a web page or text saved with a `.xls` name: 13 in EUSES) is told apart by its content, opened as what it is when that is a spreadsheet, a web page, or text, and otherwise refused with a message
- [ ] **1.0** Open the plain (not compound) Excel 2.0 to 4.0 worksheet and macro files, including Excel 4.0 macro sheets as sheets of formulas kept as written *(government 4% (51), EUSES <1% (27), ours 0%)*
- [ ] **1.0** Open encrypted `.xls` (the RC4 forms and the older XOR scheme)
- [ ] **1.0** Save `.xls` so Excel opens it, with Check Compatibility naming what is lost (rows past 65,536, columns past 256, newer functions, styles beyond the cell-format limit, newer conditional formats, tables, sparklines, and slicers); until then, "Save As" offers a new `.xlsx` name instead, as JeffDeck does for `.ppt` (decided October 11, 2026)
- [ ] **later** Open Lotus 1-2-3 (`.wk1`, `.wks`), Quattro Pro, Microsoft Works, dBASE (`.dbf`), SYLK, DIF, and Gnumeric files *(shared: librevenge, already in third_party, for the Works and Lotus ones; LibreOffice's tests hold 3 Quattro Pro, 3 Works, 3 SYLK, 3 dBASE, 2 DIF, and 2 Gnumeric files)*

### OpenDocument `.ods`, `.ots`
- [~] **first release** Open `.ods` and `.ots` (ODF 1.1 to 1.3, including LibreOffice's and Apache OpenOffice's): cells of every value type (number, percentage, currency, date, time, true or false, text), formulas written in the OpenFormula syntax with their stored results, repeated rows and columns, covered cells and merges, number styles turned into formats, cell and automatic styles, named ranges, database ranges (filters and sorts), data validation, conditional formatting, notes, hyperlinks, embedded charts, drawing shapes and pictures, pivot tables (kept), print ranges, page styles with headers and footers, protection, and the view settings (cursor, zoom, frozen panes) *(LibreOffice's tests: 305 `.ods`; shared: JeffDeck's ODF reader for styles, shapes, and charts)*
- [~] **1.0** Save `.ods` and `.ots` as LibreOffice writes them, with every property explicit so LibreOffice, Apache OpenOffice, and Excel's importer open it as it looks here, with formulas in OpenFormula and results stored, checked on a collection; follows once `.xlsx` is solid (decided October 11, 2026) *(shared: JeffDeck's ODF writer)*
- [ ] **later** Flat OpenDocument (`.fods`) *(LibreOffice's tests: 18, besides the 509 function files)*

### CSV and text
- [ ] **first release** Open `.csv`, `.tsv`, and `.txt` through the import preview: the separator (comma, semicolon, tab, space, pipe, or your own, several in a row counted as one on request), the quote character (a quote inside quotes written twice), line breaks inside quotes, line ends (CRLF, LF, or CR), the encoding (UTF-8 with or without a mark, UTF-16 with its mark, the Windows and ISO 8859 code pages, Mac Roman, and the East Asian ones: Shift JIS, GBK, Big5, and EUC-KR; chosen by a guess the person can override), the first row to read, rows of different lengths, the decimal and thousands marks, the date order, trailing minus signs, and a type for each column (General, Text, a date in a chosen order, or skipped); into a new sheet, the current one, or a new workbook
- [ ] **first release** A file with more than 1,048,576 rows loads the first million and says so, or continues into further sheets by choice (see the open questions)
- [ ] **first release** Save `.csv` (comma; and a semicolon form for countries where the comma is the decimal mark), `.tsv`, and Unicode text (UTF-16, tab-separated): the active sheet only, with a warning that other sheets aren't saved; cells as shown (formatted) or as stored (full precision, dates as ISO text), chosen; text quoted when it holds the separator, a quote, a line break, or leading or trailing spaces; line ends and the byte-order mark chosen; formula cells written as their results
- [ ] **first release** CSV round trip: a `.csv` opened and saved without edits comes out the same file (same separator, quotes, line ends, encoding, and cell text), because values are kept as the text read until they're edited *(ours: 1,267 `.csv`)*
- [ ] **1.0** Paste and drop delimited text from any program through the same import window; open a `.csv` from the command line with the dialect on the command line
- [ ] **later** Fixed-width text (`.prn`) and the Text Import wizard's fixed columns when saving

What our own `.csv` files look like, to set the defaults (counts only): 1,267 files, the middle one 853 bytes and the largest 1.1 MB (one over 1 MB); 99.6% separated by commas (2 by semicolons, 2 with one column, 1 empty); 99% end lines with CRLF; 99.6% start with a row of words, a header; 77% quote some fields; 32% hold letters beyond ASCII, all valid UTF-8 (one file has a UTF-8 mark, one is UTF-16); 1% have rows of different lengths; and a few hold dates written m/d/y (15 files), ISO dates (6), dollar amounts with commas (5), codes with leading zeros (5), or digit strings of 12 or more digits (4). They are mostly small exports from other programs, so the first test is that they open and save back the same.

### Other formats and parts
- [~] **first release** PDF export (see Share and Export) *(shared: JeffPub)*
- [~] **1.0** Open an HTML page or a saved web page's tables as a workbook (Excel opens its own "web page" saves this way), and save a sheet as HTML *(web pages saved with a `.xls` name: 13 in EUSES; shared: JeffPub's web export)*
- [ ] **later** Open and save XML Spreadsheet 2003 (`.xml`), the older Excel-as-XML form
- [~] **first release** Pictures in workbooks: PNG, JPEG, GIF, BMP, TIFF, EMF, WMF, and SVG (with its PNG stand-in) in `.xlsx`, and the picture records of `.xls`, read and written *(shared: libs/base EMF and WMF, libs/draw SVG)*
- [~] **first release** Fonts: a workbook names fonts without embedding them; a missing one is drawn with a stand-in of the same width so columns and wraps hold *(shared: the bundled fonts and width-keeping stand-ins)*
- [ ] **later** Copy and paste of cells to and from other programs in their clipboard forms (HTML, RTF, tab-separated text, and the binary spreadsheet form)
- [ ] **later** XPS: not offered, since the format has no use outside Windows

### What must survive a round trip
- [ ] **first release** A checklist for every format that JeffCrunch saves: formulas as written, stored results with their types, number formats, styles and theme colors, column widths and row heights, merged cells, hidden rows, columns, and sheets, defined names and their scopes, print setup with headers and footers, notes (text, size, and position), data validation and conditional formats, tables and their styles, pivot tables, charts (series formulas and formatting, extension elements included), drawings and their anchors (move and size with cells, or not), hyperlinks, outline levels, sheet views, protection, workbook settings (calculation, iteration, date system), external-link caches, and custom properties; anything JeffCrunch can't hold in a format is named by Check Compatibility before the file is written

## Shared with other JeffOffice apps

The suite-wide map of shared parts, [shared.md](shared.md), says where each lives, which app builds it first, and who uses it; where this list disagrees with it, the map holds.

What JeffCrunch builds on, and the pieces it should build so that JeffWrite, JeffDeck, JeffMail, and JeffBase can use them.

### Already there
- [~] **first release** libs/office: Open XML packages, themes, DrawingML shapes, text, and tables (read and written), charts (read from their stored values and drawn as SVG: column, bar, line, pie, doughnut, area, and scatter), the Escher drawing reader for `.xls` drawings, and the embedded-font readers; JeffCrunch adds the spreadsheet parts (`xl/`) beside them
- [~] **first release** libs/base: the compound-file reader and writer (libs/base/io cfb, which holds the `.xls` workbook stream), the zip reader and writer, colors, fonts, barcodes, EMF and WMF, PDF, and the dictionaries; and libs/draw: shapes, fills, effects, text art, SVG, and picture effects
- [~] **first release** libs/shell: the window, ribbon, KeyTips, file pages, Help, translations, AutoRecover, settings, spelling, updates, and statistics
- [~] **first release** libs/text, the text engine, for the text inside cells, text boxes, and chart labels; libs/page's editor pieces, Find and Replace, options, and online pictures. The grid of cells and its editing are new: libs/page is a page of placed objects, and a sheet is not one
- [~] **first release** JeffPub's merge-source reader opens `.csv` and `.xlsx` for mail merge (values only); it gives way to JeffCrunch's readers, which keep everything

### To build once, for several apps
- [ ] **first release** A calculation engine as a library: formula parser (A1 and R1C1, structured references, names, arrays), dependency graph, function library, date system, error values, and the number-format language, shared by JeffCrunch, the table formulas of JeffWrite and JeffDeck, and JeffBase's expressions *(needed by: JeffCrunch, JeffWrite, JeffDeck, JeffBase)*
- [ ] **first release** The number-format language (the codes above) as a library, used also for fields in JeffWrite, mail-merge formats, JeffBase column formats, and chart axes *(needed by: all)*
- [~] **first release** A grid control: virtual rows and columns, frozen and split panes, selection, in-cell editing, and fast scrolling over millions of cells, also used for JeffBase's table view and for the data grid of chart editing in JeffWrite and JeffDeck *(needed by: JeffCrunch, JeffBase, JeffWrite, JeffDeck)* *(wave 4a: libs/shell/grid: virtual rows and columns, frozen panes, selection, and fast scrolling; no in-cell editing or split panes yet))*
- [ ] **1.0** Chart editing on top of libs/office chart: the data grid, the element menus, types, and styles; a chart in JeffWrite or JeffDeck keeps its data in a small embedded sheet edited with JeffCrunch's grid, and a chart in a sheet follows its cells *(needed by: JeffCrunch, JeffWrite, JeffDeck)*
- [ ] **1.0** Embedded worksheets: Insert > Table > Excel Spreadsheet in JeffWrite and Insert > Object in JeffDeck show the saved picture and open in JeffCrunch's grid in place; cells copied from JeffCrunch land in JeffWrite and JeffDeck as a table, with fonts, fills, borders, and number formats *(needed by: JeffCrunch, JeffWrite, JeffDeck)*
- [ ] **1.0** A merge-source library: a JeffWrite mail merge, JeffPub catalog merge, or JeffMail send reads a JeffCrunch workbook's table or named range directly (with the cell values as shown or as stored), a `.csv`, or a JeffBase table, with filter, sort, duplicate, and address checks; JeffCrunch's text import is the front end of the same code *(needed by: JeffPub, JeffWrite, JeffMail, JeffBase, JeffCrunch)*
- [ ] **1.0** A review model for notes and comments with replies, so JeffWrite's tracked changes, JeffDeck's comments, and JeffCrunch's notes follow it *(needed by: JeffWrite, JeffDeck, JeffCrunch)*
- [ ] **1.0** An equation editor and layout for Office Math and MathML *(needed by: JeffWrite, JeffDeck, JeffCrunch)*
- [ ] **1.0** A data-source layer (text, CSV, JSON, XML, HTML tables, SQLite, ODBC, PostgreSQL, and MySQL) with the editable list of steps, which JeffBase can use for its own imports and queries *(needed by: JeffCrunch, JeffBase)*
- [ ] **1.0** Themes, table styles, and cell styles in one format and gallery, so a JeffDeck theme can color a JeffCrunch workbook *(needed by: all)*
- [ ] **later** An ink layer built on the slide show's pen *(needed by: JeffWrite, JeffDeck, JeffCrunch)*

## Left out on purpose

Not planned, because they only work with an online service or a product JeffOffice doesn't have: Office add-ins and the Store, Ideas (Analyze Data) and Insights, the Stocks and Geography data types and STOCKHISTORY, Microsoft Forms, 3-D Maps (Power Map) and Bing map charts, Python in Excel and Copilot, OneDrive and SharePoint (co-authoring, cloud AutoSave and version history, workflows, publishing to Excel Services or a SharePoint list, Visio data visualizer), Power BI, Microsoft Translator, sensitivity labels and information rights management, the Dynamics, Salesforce, Azure, Analysis Services, and Facebook connectors, fax, and XPS. What a file holds of these is kept and written back.

## Decided

- Old and open formats (October 11, 2026): the first release opens `.xls`, `.ods`, and `.csv`; saving in `.xls` and `.ods` follows once `.xlsx` is solid, as JeffDeck did with `.ppt` and `.odp`. Until then, Save on such a file asks for an `.xlsx` name.

## Open questions

1. Power Query: keep saved queries untouched and offer our own editor of steps, or also run Microsoft's M language so that saved queries can refresh? M has a public specification, but it is large.
2. Macros: keep them untouched and never run them, or give JeffCrunch an open scripting language later (and which one, and should it read VBA)? About 1 in 10 of the old public `.xls` hold macros.
3. Should creating a pivot table move up to the first release? Public use is small (Enron 2%, about 250 of 15,871 workbooks), but it is the best-known advanced feature of a spreadsheet, and the file parts are needed either way.
4. Should the dynamic-array functions (FILTER, SORT, UNIQUE, SEQUENCE, XLOOKUP, and LET) move up to the first release? No public workbook uses them (the files predate them), but anyone who types a modern formula will.
5. CSV with more than a million rows: stop at the original's limit with a warning, or continue into further sheets, or open it as a read-only table that scrolls?
6. Should an option turn off the original's arithmetic quirks (the false February 29, 1900, and the rule that shows almost-zero sums as zero) for new workbooks, or are they always on?
7. LibreOffice's extra functions (EASTERSUNDAY, ISLEAPYEAR, and the others found in `.ods` files): calculate them, or keep them as text with the stored result?
8. An open replacement for the Stocks and Geography data types (for example from Wikidata), or leave them out for good?
9. Translate in the Review tab: is an offline translation model (size and license to check) worth shipping, or is it dropped?
10. Solver (the add-in of the original): include a simple one in 1.0, or leave it for later?
11. Protected View (files from the Internet open read-only until you choose to edit): add it, or is a plain read-only open enough?
12. East Asian features (the double-byte text functions, phonetic guides, and the Japanese era and Hijri calendars): stay at 1.0 and later, or does anyone who will use JeffCrunch need them for the first release?
13. Which built-in templates come first: budget, invoice, expense report, loan schedule, calendar, or timesheet?
