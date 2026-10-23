# JeffWrite feature list

JeffWrite is the word processor of JeffOffice. JeffOffice is an independent project: JeffWrite aims to work with Word files and the people who use them (compatibility and interoperability), and then to do more, and do it better. It saves `.docx`, and opens the older `.doc` too; `.rtf` and `.odt` are planned. Its first public version, 0.9.0 (a Preview, released October 19, 2026), is a window with the ribbon, the page view, editing, `.docx` and `.doc` files, tables, headers and footers, notes, and pictures and text boxes as files have them, spelling, and printing; each box says what that version does of an item (`[x]` all of it, `[~]` part, with the note saying what is missing, and `[ ]` none). This list is the whole job, a checklist of features: everything people rely on so their documents open, look right, and keep working, and then more, grouped the way the app's ribbon is (File, then each ribbon tab, the tabs that appear only while something is selected, then what sits under the ribbon), so the work can be planned and tracked.

Last reviewed October 22, 2026.

Legend: `[x]` works in JeffWrite 0.9.0 · `[~]` partly works in 0.9.0 (the note says what is missing) · `[ ]` not in 0.9.0 (a *(shared: ...)* note names the part of the suite it builds on) · **(alt)** the usual way depends on a proprietary online service, so the app offers an open replacement.

Every item carries a tier:

- **first release**: needed in JeffWrite's first public release.
- **1.0**: needed before JeffWrite is called finished (version 1.0).
- **later**: wanted, but not before 1.0.

## The Preview bar (decided October 17, 2026)

JeffWrite ships as a Preview when two things hold. First, its pages come close enough to the reference pages, measured against reference PDFs on documents never used for tuning (the reserved half of the yardstick, 95 documents): at least 70% of their pages scoring under 2, and every word a reference page shows also in JeffWrite's pages (differences only in how the two PDFs give their text don't count). A page's score is the mean difference in gray between JeffWrite's picture of it and the reference picture, where 0 is identical. On October 17 the reserved documents stood at 37% of pages under 2 (40 of 95 documents averaging under 2) and 2.2% of words missing; on October 19 they stood at 73% of pages under 2 and 0.16% of words missing, all five basics worked, and JeffWrite shipped as the Preview in 0.9.0. Second, five basics work: Find and Replace, the Font dialog, spell check as you type, opening old `.doc` files, and printing. The items that make up those basics are marked "(needed for the Preview)".

## How the tiers were chosen

**First release** is the first public build (a preview, as JeffDeck 0.7.0 was). It must open, edit, print, and save the kinds of documents in our collection and the public collections without damaging them, and be good enough for a letter, a report, a brief, or a book chapter. **1.0** is what someone coming from Word expects to find day to day; version 1.0 is ready to ship when every file JeffWrite saves opens without a repair message and JeffWrite's pages match the reference pages about as closely as JeffPub's match Publisher's. An item is **later** when few documents use it, when it needs a piece the suite doesn't have yet, or when it depends on something outside the program.

How the evidence reads. Where the counts measure a feature, its item carries how many real documents use it, as "ours" and "public": **ours** is the share of our own collection of 2,082 working documents (1,056 `.docx`, plus 1,026 old `.doc` and `.rtf` files, about 3% of them `.rtf`), and **public** is the share of 1,527 U.S. government `.doc` files. The old files were converted to `.docx` to be counted. Where the two kinds of our files differ a lot, the item says so. Four cautions:

- The conversion adds "automatic hyphenation" and "vertical text direction" to every old file, so their counts use only our `.docx`.
- Footnote and endnote *parts* exist in 71% of our `.docx` because Word writes empty ones into most files. The real use is the count of footnote references, and those figures are the ones quoted below.
- The public sample is all `.doc`; the only public `.docx` are Apache POI's 134 test files, which are edge cases and count lightly (the POI `.doc` folder held no files). Features that exist only in the newer format (content controls, table styles, embedded fonts) therefore have thin public evidence.
- "Page number formats" reads 99% in the converted `.doc` files, which looks like the conversion writing the element into every section; only our `.docx` figure (38%) is trusted.

## Beyond Word

What JeffWrite offers that the original doesn't, then the plans that go past Word.

- [~] **first release** Runs on Linux and Windows, where Word ran only on Windows and Mac, and on macOS as the rest of the suite does *(shared: the suite's builds, installers, and AppImage; the macOS app is still a test build)*
- [x] **first release** Free and open source (GPL-3.0): no license fee, no subscription, no account or sign-in, nothing that has to be online
- [x] **first release** Installs without administrator rights, or runs with no installation (Windows `.zip`, Linux AppImage) *(shared: the suite's packaging)*
- [ ] **first release** OpenDocument text (`.odt`, `.ott`) is opened and saved with the same care as `.docx`, not as an afterthought (details under File formats)
- [~] **first release** Nothing in a file is lost: whatever JeffWrite can't edit yet (SmartArt, content-control bindings, custom XML, unknown elements, macros, embedded objects) is kept with the document and written back as it was
- [~] **first release** Lines and pages break where the reference pages break them in the sample documents, judged against reference PDFs of them *(shared: the measuring tools and the metric-compatible fonts built for JeffPub)* *(0.9.0: on the 95 documents held out from tuning, 73% of the pages score under 2, and all but 0.16% of the words appear on JeffWrite's pages)*
- [x] **first release** Bundled open fonts, with stand-ins that keep the widths of Calibri, Cambria, Arial, Times New Roman, and Courier New, so a document lays out the same on a computer without Microsoft's fonts
- [~] **first release** Spelling and hyphenation work offline, in 12 languages, with no cloud service *(spelling works in JeffWrite, as you type and in the Spelling pane; hyphenation works where a file's settings ask for it, and no menu or dialog sets it)* *(shared: the bundled dictionaries in libs/base/text)*
- [ ] **first release** **(alt)** Grammar and style checking that runs on your own computer, instead of Word's online Editor: Harper (open source, Apache-2.0), English first, other languages later (decided October 11, 2026)
- [ ] **1.0** EPUB export for e-book readers and stores, with chapters at each Heading 1, the contents from the headings, footnotes that pop up, and pictures and tables in place *(shared: JeffPub's EPUB writer in apps/pub/io)*
- [ ] **1.0** PDF with bookmarks from headings, tags for screen readers, and PDF/A; PDF/X-1a and PDF/X-4 for commercial printers, with a print check *(shared: JeffPub's PDF export)*
- [ ] **1.0** Barcodes in a document or on mail-merge labels: book ISBNs with a price add-on, EAN, UPC, Code 128, and Code 39, drawn as vector bars *(shared: libs/base/core barcode)*
- [ ] **1.0** **(alt)** Insert > Pictures > Online Pictures searches Openverse and Wikimedia Commons, keeping the license and credit, in place of Bing *(shared: JeffPub's online pictures)*
- [ ] **1.0** Insert > Icons: 1,539 open vector icons, searched by name *(shared: JeffPub's icon picker)*
- [x] **1.0** Help that works offline, with a search that also finds commands and runs them *(shared: libs/shell help)*
- [~] **1.0** `jwritetool`, a command-line tool that converts between formats, extracts text, checks a file's round trip, and runs the fidelity tests, as `jpubtool` and `jdecktool` do for the other apps *(shared: the pattern of those tools)* *(0.9.0: `writetool` renders a file to pictures or a PDF, lists its pages, and turns a `.doc` into a `.docx` package; it is a development tool, and it does not yet convert between formats or check a round trip)*

## File (backstage)

### Info
- [ ] **first release** Document properties: title, author, subject, keywords, categories, comments, company, and manager, with the dates and the counts of pages, words, characters, lines, and paragraphs *(shared: JeffPub's publication properties)*
- [ ] **1.0** Custom document properties (a name, a type, and a value) that DOCPROPERTY fields can show
- [ ] **1.0** Mark as Final (the document opens read-only with a note)
- [ ] **1.0** Encrypt with a password: open and save password-protected `.docx` (the standard's strong encryption) and `.odt`
- [ ] **1.0** Inspect Document: find and remove comments, tracked changes, personal information, hidden text, custom XML, and headers and footers before sharing
- [ ] **1.0** Check Compatibility: lists what an older format (`.doc`, `.rtf`, `.odt`) can't hold before saving to it
- [ ] **1.0** **(alt)** Version history without a cloud service: named local copies of a document, compared and restored from Info
- [x] **1.0** Recover unsaved documents *(shared: AutoRecover in libs/shell)* *(0.9.0: Open > Recover Unsaved Work offers the copies that AutoRecover kept)*
- [ ] **1.0** Show a document's folder and copy its path
- [ ] **later** Add a digital signature (an invisible one, and the signature line from Insert) with a certificate on your computer
- [ ] **later** Links to Files: update or break links to other files (LINK and INCLUDETEXT fields)

### New
- [x] **first release** Blank document, using the defaults of the Normal template
- [ ] **first release** Template gallery with live previews and categories, starting with a few built-in templates (letter, report, brief, manuscript, newsletter) written by JeffOffice and open-licensed *(shared: JeffPub's New page)*
- [ ] **1.0** More built-in templates: resumes, cover letters, invoices, flyers, brochures, meeting notes, legal pleadings, and book interiors
- [ ] **1.0** Save as a template (`.dotx`, `.ott`) and reuse your own under My Templates
- [ ] **1.0** The Normal template: the default styles, margins, and page size that every new blank document starts from, changed with Set as Default in the Font, Paragraph, and Page Setup dialogs
- [ ] **1.0** Attach a different template to a document (Developer tab > Document Template) and copy styles and building blocks between a document and its template with the Organizer *(ours 2% attached a template, public 0%; POI 3%)*
- [ ] **later** Customize a template's color scheme and font scheme before creating the document from it *(shared: the idea in JeffPub's template page)*

### Open, Save, Save As, and Close
- [~] **first release** Open `.docx`, `.doc`, `.rtf`, and `.odt` (and `.dotx`, `.dot`, `.ott` templates), from the Recent list with pinning, from Browse, by drag-and-drop, and from the command line *(shared: the recent list and file pages in libs/shell; `.doc` and `.dot` open since `libs/office/doc` reads the binary file into the model the `.docx` reader makes, text and styles and lists and sections and headers and notes and fields and tables and pictures and drawings; comments, form-field state, and macros are left out and the person is told, and Save asks for a `.docx` name)* *(0.9.0: opens `.docx`, `.docm`, `.dotx`, `.dotm`, `.doc`, and `.dot`, from the Recent list, from Browse, and from the command line; pinned files on the Open page; not `.rtf`, `.odt`, `.ott`, or drag-and-drop)*
- [~] **first release** Save as `.docx` by default; Save As lists every format JeffWrite writes; Ctrl+S, Ctrl+Shift+S, and Close with a prompt to save *(0.9.0: Save As offers `.docx` and `.dotx`; Ctrl+S, Ctrl+Shift+S, and a prompt to save on Close; a file is saved beside the old one and replaces it only when whole)*
- [x] **first release** AutoRecover (unsaved work offered again after a crash, cleaned up once saved) *(shared: libs/shell recovery)*
- [ ] **1.0** Backup copy kept on every save *(shared: JeffPub's option)*
- [ ] **1.0** Save a Copy (keeps working in the original) and Save as Template
- [ ] **1.0** A warning before saving to a format that can't hold something in the document, naming what would change, with the choice to save a `.docx` as well
- [ ] **1.0** Open and Repair: recover what text and pictures can be read from a damaged `.docx` or `.doc`
- [ ] **1.0** Open read-only, open as a copy, and open in a new window
- [ ] **1.0** Open an encrypted document by asking for its password
- [ ] **1.0** Return to where you stopped editing when a document is reopened (Shift+F5 jumps among the last three editing places)
- [ ] **later** Protected View: files from the Internet open for reading only until you choose to edit them
- [ ] **later** Open from a web address (HTTP and WebDAV) and save back
- [ ] **later** Recover Text from Any File: open an unknown file and keep its readable text

### Print
- [~] **first release** Print with printer choice, copies, collate, page ranges (all, current page, selection, custom, including Word's section syntax such as p2s3), one- or two-sided, orientation, paper size, margins, and printer properties *(done: the printer or a PDF file, copies, collate, all pages, the current page, and custom ranges such as 1-3, 5, 8-, and -2, with each page on the paper and the way up its section sets; missing: selection, the p2s3 syntax, one- or two-sided, margins, and printer properties)* *(shared: JeffPub's print)* **(needed for the Preview)**
- [~] **first release** Print preview with page navigation, zoom, one or many pages, and a shrink-to-fit-one-page check *(done: each page as it will print, one at a time, with arrows; missing: zoom, many pages, and the check)* *(shared: JeffPub's print preview)* **(needed for the Preview)**
- [ ] **1.0** Print only odd or even pages, in reverse order, and several pages on a sheet or scaled to a paper size
- [ ] **1.0** What prints: markup (tracked changes and comments) or without it, hidden text, background colors and pictures, document properties, field codes, drawings, and a list of the markup; fields updated before printing
- [ ] **1.0** Book fold printing: a booklet in folding order, from the page-setup choice under Layout *(shared: JeffPub's booklet printing)*
- [ ] **1.0** Duplex printing by hand: print the odd pages, then the even ones, with a prompt to turn the stack over

### Share and Export
- [~] **first release** Save as PDF: standard and minimum size, page range, fonts embedded, document properties, bookmarks from headings, and a PDF opened after saving *(shared: JeffPub's PDF export)* *(0.9.0: Export to PDF on the File screen, and a PDF file as a printer in Print; no document properties or bookmarks)*
- [ ] **1.0** PDF/A, tagged PDF, PDF with a password, and PDF/X-1a and PDF/X-4 for printers *(shared: JeffPub's PDF export)*
- [ ] **1.0** Save as plain text (choose the encoding and where line breaks fall) and as filtered HTML
- [ ] **1.0** Save as a single-file web page *(shared: JeffPub's web export)*
- [ ] **1.0** **(alt)** Email: make a ready-to-send `.eml` with the document as an attachment, a PDF, or the pages as HTML, instead of sending through an online account *(shared: JeffPub's .eml export)*
- [ ] **1.0** **(alt)** Share without a cloud account: a package with the document, the fonts, and the pictures (Pack and Go, as JeffPub has it) for another computer or a printer
- [ ] **later** Save as `.docm` and `.dotm`, keeping macros untouched and never running them

### Options
- [~] **first release** General: user name and initials, light or dark interface, display language, usage statistics (opt in), and update checks *(shared: libs/shell settings)* *(0.9.0: Options has the General and Proofing pages)*
- [ ] **first release** Display: show white space between pages, formatting marks, hidden text, object anchors, optional hyphens, and gridlines; print options *(shared: JeffPub's display options)*
- [~] **first release** Proofing: AutoCorrect (replacement list, capitalization rules, smart quotes), AutoFormat as you type, spelling as you type, ignoring words in capitals or with numbers, and custom dictionaries *(in JeffWrite the choices for spelling as you type and for skipping capitals and numbers work; those for AutoCorrect and AutoFormat do nothing yet)* *(shared: JeffPub's Proofing options)*
- [ ] **1.0** Proofing exceptions (words AutoCorrect must leave alone), grammar settings, and the main dictionary for each language
- [ ] **first release** Save: default format, AutoRecover interval, default folder, and Embed fonts in the file *(shared: JeffPub's Save options)*
- [ ] **1.0** Language: editing languages and the display language, with the default language for new text
- [ ] **1.0** Advanced editing: typing replaces the selection, drag-and-drop text, Ctrl+click to follow a link, smart paragraph selection, select the whole word, overtype mode, and smart cut and paste
- [ ] **1.0** Advanced cut, copy, and paste: the default for each way of pasting (within and between documents, from other programs, and from lists and tables)
- [ ] **1.0** Advanced display and layout compatibility options: measurement units, show pixels for HTML features, and a per-document list of layout quirks kept for old files (see Compatibility)
- [ ] **1.0** Font substitution: which font replaces a missing one, and a note when a document needs one
- [ ] **1.0** Customize the Ribbon: show, hide, rename, and reorder tabs and groups, and add your own, plus the Quick Access Toolbar
- [ ] **1.0** Customize the keyboard: assign keys to commands, styles, symbols, and building blocks, and reset them
- [ ] **1.0** Privacy: remove personal information on save, and the choice about what is sent anywhere (statistics, crash reports) *(shared: libs/shell telemetry)*

## Home tab

### Clipboard
- [~] **first release** Cut, Copy, and Paste with the paste choices: keep source formatting, merge formatting, keep text only, and picture; Paste Special for formatted text (RTF and HTML), unformatted text, and pictures *(shared: JeffPub's clipboard)* *(0.9.0: Cut, Copy, Paste, and Select All, which keep the formats of the text; no paste choices, Paste Special, or pictures)*
- [ ] **first release** Format Painter: one use, or locked with a double-click; copies the paragraph's formatting when its mark is selected *(shared: JeffPub's Format Painter)*
- [ ] **1.0** Paste choices for lists and tables: continue, restart, or merge a list; paste spreadsheet cells as a table in the destination's style, in the source's, as a nested table, as new rows, or as a picture; Set Default Paste
- [ ] **1.0** Smart cut and paste (spaces and punctuation fixed around moved words) and the Paste Options button that appears after pasting
- [ ] **later** Clipboard task pane holding the last 24 items

### Font
- [~] **first release** Font name (theme fonts first, recent fonts, a preview in the list) and size (list, typed, Grow and Shrink, and the Ctrl+[ and Ctrl+] keys) *(shared: JeffPub's Font group)* *(0.9.0: the font box, the size box, and Grow and Shrink on the ribbon)*
- [x] **first release** Bold, italic, underline (single, words only, double, thick, dotted, dashed, wavy, each with its own color), strikethrough, and double strikethrough *(the Font box chooses the underline and its color, and double strikethrough; the Underline button on the ribbon is a single line; each is read from and written to `.docx` as `w:u` with its val and color, `w:strike`, and `w:dstrike`)*
- [x] **first release** Subscript and superscript *(ours 26%, public 35%; the buttons, Ctrl+= and Ctrl+Shift+=, and the Font box; `w:vertAlign`)*
- [x] **first release** Small caps and all caps *(small caps: ours 10%, public 9%; all caps: ours 10%, public 9%; in the Font box, not on the ribbon; `w:smallCaps` and `w:caps`, also against a style that has one)*
- [~] **first release** Character spacing: expanded or condensed by an amount, raised or lowered, scaled, and pair kerning from a chosen size *(ours 10%, public 15%)*
- [~] **first release** Font color (theme colors with tints, standard colors, more colors, eyedropper) and highlight color, with Stop Highlighting *(highlight: ours 2%, public 8%)*
- [~] **first release** Hidden text: kept, not shown or printed unless asked for *(Hidden in the Font box, kept as `w:vanish`, not shown, printed, or found by Find; not yet shown when the paragraph marks are)*
- [x] **first release** Clear All Formatting
- [~] **first release** The Font dialog with every effect above, plus the live preview, and Set as Default *(Ctrl+D, the Font group's launcher, and the right-click menu: font, style, size, color, underline and its color, the seven effects, and the preview, applied as one step to what was changed; a box is blank where the selection differs, and what is set then goes to all of it; no Advanced tab (spacing, raised and lowered) and no Set as Default yet)*
- [ ] **1.0** Change Case: sentence case, lowercase, UPPERCASE, capitalize each word, and toggle (Shift+F3 cycles)
- [ ] **1.0** Character shading and character border
- [ ] **1.0** OpenType choices: ligatures, number spacing and forms, stylistic sets, and contextual alternates
- [ ] **1.0** The Mini toolbar that floats beside a selection
- [~] **later** Text effects on letters (outline, shadow, reflection, glow, gradient fill): kept and drawn first, edited later *(shared: libs/draw text effects)*
- [ ] **later** Classic effects (emboss, engrave, outline, shadow), phonetic guide (ruby, ours 0%, POI 1%), and enclosed characters

### Paragraph
- [~] **first release** Alignment: left, center, right, and justified; the Paragraph dialog with indents (left, right, first line, hanging), the ruler's markers, Increase and Decrease Indent, and mirror indents *(shared: JeffPub's paragraph formatting)*
- [~] **first release** Line spacing (single, 1.15, double, multiple, at least, exactly) and space before and after, with "don't add space between paragraphs of the same style"
- [~] **first release** Tab stops: left, center, right, decimal, and bar, with dot, dash, or line leaders, set on the ruler or in the Tabs dialog, with default stops and Clear All *(ours 37%, public 67%)* *(0.9.0: a file's tab stops, with their leaders and the default stops, are laid out; none can be set yet)*
- [~] **first release** Bulleted lists: the gallery, a symbol or picture of your own, and level changes *(lists: ours 44%, public 52%; 59% of his `.docx`)*
- [~] **first release** Numbered lists: the number formats (1., 1), A., a., I., i., and others), start at, continue, restart, and Set Numbering Value *(same lists evidence)*
- [~] **first release** Multilevel lists, nine levels: each level's number format, alignment, indent, tab or space after, restart rule, legal style, and link to a heading style, saved as list styles *(lists: ours 44%, public 52%; the books and briefs depend on it)* *(0.9.0: a file's nine levels, with each level's number format, indent, tab or space after, restart, and link to a style, are read, laid out, and kept; the choices in the item are not offered, and a picture bullet is not drawn)*
- [~] **first release** Show or hide formatting marks: paragraph marks, tabs, spaces, hidden text, optional hyphens, and object anchors *(shared: JeffPub's Show Special Characters)*
- [~] **first release** Paragraph shading *(shading: ours 6%, public 17%)* and borders on any side, with every style, width, and color and the distance from the text *(paragraph borders: ours 3%, public 17%)*
- [~] **first release** Line and Page Breaks in the Paragraph dialog: widow and orphan control, keep with next, keep lines together, page break before, suppress line numbers, and don't hyphenate; outline level (body text or 1 to 9)
- [~] **first release** Right-to-left paragraphs and text, with the Left-to-Right and Right-to-Left buttons and mixed-direction runs *(ours 4%, public 10%; shared: Qt shaping in libs/text)*
- [ ] **1.0** Sort paragraphs, list items, or table rows by text, number, or date, on up to three keys
- [ ] **1.0** Horizontal line from the Borders menu
- [ ] **later** Distributed and Thai justification, and East Asian typography: vertical in horizontal, combine characters, two lines in one, fit text, character scaling, and the line-breaking rules for Chinese, Japanese, and Korean *(vertical text direction: ours 1% of his `.docx`, POI 7%)*

### Styles
- [~] **first release** Style gallery with live preview (Normal, No Spacing, Heading 1 to 9, Title, Subtitle, Emphasis, Strong, Quote, Intense Quote, References, Book Title, List Paragraph), applied by click, by typing the name, or by key *(shared: JeffPub's styles)*
- [ ] **first release** Styles pane: list with previews, which styles to show, and sorting
- [ ] **first release** Create a style from a selection and Modify Style: name, type (paragraph, character, linked, table, list), based on, style for the following paragraph, formatting, add to the gallery, update automatically, and this document or new documents *(a typical `.docx` of our defines 16 styles and one in ten more than 33; a typical government file 30)*
- [~] **first release** Update a style to match the selection, Select Text with Similar Formatting, and Clear Formatting
- [ ] **1.0** Manage Styles: edit, recommend, restrict, set defaults, and import or export styles with the Organizer
- [ ] **1.0** Style Inspector and Reveal Formatting (Shift+F1)
- [ ] **later** Style separator (end a paragraph's visible text and start another style in the same paragraph, for run-in heads)

### Editing
- [~] **first release** Find with the Navigation pane's results (by heading, by page, as text), the match count, and every match highlighted *(shared: JeffPub's Find; Ctrl+F opens the Find and Replace pane, and Find Next selects each match in the text and tables, with match case and whole words, scrolls the pages to it, and goes round; no Navigation pane, count, or highlighting yet; hidden text is skipped, and a bookmark or field code inside words does not part them)*
- [~] **first release** Replace and Replace All, with match case, whole words, and direction, across the body, notes, headers and footers, and text boxes *(shared: JeffPub's Replace; Ctrl+H, Replace, and Replace All in the body and tables, Replace All as one step that says how many; the new words look like the first of the old; no direction, and headers, footers, and notes are not searched because JeffWrite cannot edit them yet)*
- [ ] **first release** Go To: page, section, line, bookmark, comment, footnote, endnote, field, table, graphic, equation, and heading
- [ ] **1.0** Advanced Find and Replace: wildcards with groups, sounds like, all forms of a word, ignore punctuation and spaces, find by format (font, paragraph, tabs, language, frame, style, highlight), Special (paragraph mark, tab, any digit, section break, field, footnote mark, and the rest), and replace with the clipboard or the found text
- [ ] **1.0** Select All, Select Objects, and the Selection Pane (objects by name, shown or hidden, in order)

### Dictate
- [ ] **later** **(alt)** Dictate: speech to text by an open model on your own computer, instead of Office's online dictation

## Insert tab

Header, Footer, and Page Number are listed under Header & Footer tools, and Comment under Review.

### Pages
- [~] **first release** Page Break (Ctrl+Enter) and Blank Page *(page breaks: ours 26%, public 38%)*
- [ ] **1.0** Cover Page gallery of built-in designs, Remove Current Cover Page, and saving your own

### Tables
- [ ] **first release** Insert Table from the grid or the dialog (columns, rows, fixed or auto width, remember the size), and Draw Table; everything else is under Table tools *(tables: ours 20%, public 42%)* *(shared: JeffPub's tables)*
- [ ] **1.0** Convert Text to Table and Table to Text (by paragraphs, commas, tabs, or another character)
- [ ] **1.0** Quick Tables gallery (calendars, matrices, tabular lists, with subheadings) and saving your own
- [ ] **later** Excel Spreadsheet: an embedded worksheet, edited in place by JeffCrunch once it exists *(shared with JeffCrunch)*

### Illustrations
- [x] **first release** Picture formats: PNG, JPEG, GIF, BMP, TIFF, WebP, SVG, and EMF and WMF are read and drawn, the vector ones staying vectors in print and PDF *(shared: libs/base and libs/draw)* *(pictures: ours 25%, public 38%)*
- [ ] **first release** Insert Picture from a file, linked or embedded, from the clipboard, or by dragging in, with its alt text *(shared: JeffPub's Insert Picture)*
- [ ] **1.0** **(alt)** Online Pictures and Stock Images from Openverse and Wikimedia Commons, with license and credit kept as a caption *(shared: JeffPub's online pictures)*
- [~] **first release** Shapes: lines, rectangles, basic shapes, block arrows, equation shapes, flowchart, stars and banners, and callouts, with text inside them, connectors, and edit points *(drawing objects of any kind, pictures among them: ours 32% (.docx 18%, .doc and .rtf 45%), public 59%; the old-style (VML) ones: ours 17%, public 50%; shared: libs/draw)* *(0.9.0: a file's shapes and text boxes (DrawingML and the old VML ones) are drawn and kept; none can be inserted or edited)*
- [ ] **1.0** Drawing canvas: a frame that holds shapes and pictures together *(shared: libs/draw groups)*
- [ ] **1.0** Icons, drawn as shapes whose color and line weight can change *(shared: JeffPub's icon picker)*
- [ ] **1.0** Chart: insert, edit its data in a grid, and change its type (column, bar, line, pie, area, scatter, and the rest), elements, layout, style, and colors *(charts: ours 0%, public 0%, POI 1%; shared: libs/office chart, also used by JeffDeck and later JeffCrunch)*
- [ ] **later** SmartArt: shown from the drawing the file keeps and written back as it was (ours <1%, public 0%); editing the diagram follows *(shared: JeffDeck's SmartArt drawing)*
- [ ] **later** 3-D models: shown as the picture the file keeps and written back; rotating them in 3-D is not planned
- [ ] **later** Screenshot and screen clipping (the system's screenshot tool works in the meantime)

### Media
- [ ] **later** Online video: a poster picture with a link that plays the video in the browser

### Links
- [~] **first release** Hyperlink: a web address, email address, place in this document (heading or bookmark), or new document, with screen tip, edit, remove, and Ctrl+click to follow *(hyperlinks: ours 32%, public 37%)* *(shared: JeffPub's hyperlinks)*
- [~] **first release** Bookmark: add, go to, delete, sort by name or location, and hidden bookmarks *(bookmarks: ours 40% (.docx 77%, .doc and .rtf 1%), public 25%; most of them are the ones Word makes for contents and cross-references)* *(shared: JeffPub's bookmarks)* *(0.9.0: a file's bookmarks are kept, and what refers to them (page references) finds them; none can be added or used)*

### Text
- [~] **first release** Text Box: draw one, in a horizontal or vertical direction, with fill, outline, insets, wrap, and linking boxes so text continues from one to the next *(text boxes: ours 12%, public 44%)* *(shared: JeffPub's text boxes and linked frames)* *(0.9.0: a file's text boxes are drawn and kept, with their text and tables; none can be drawn)*
- [ ] **1.0** Text box gallery of built-in designs (simple, sidebars, pull quotes) and saving your own
- [ ] **1.0** Quick Parts and building blocks: AutoText, Document Property, Field, the Building Blocks Organizer, and saving a selection to a gallery, as the galleries for cover pages, headers, footers, equations, tables, and watermarks use *(glossary parts: ours <1%, public 0%; POI 9%)*
- [~] **1.0** WordArt: styles, fill, outline, and effects, and text shapes (warps) *(shared: libs/draw text art)*
- [ ] **1.0** Drop Cap: dropped or in the margin, lines high, distance from text, and font
- [ ] **1.0** Date & Time: formats and languages, updated automatically or fixed *(DATE field: ours 1%, public 2%)*
- [ ] **1.0** Text from File: insert another document's text (`.docx`, `.doc`, `.rtf`, `.odt`, or plain text), or a bookmarked part of it *(shared: JeffPub's Insert File)*
- [ ] **later** Signature Line: a drawn line for a name and title, signed digitally when digital signatures exist
- [ ] **later** Object: embed or link an object made by another program (OLE on Windows; on other systems the picture and the data are kept and written back) *(OLE objects: ours 1%, public 10%)*

### Symbols
- [ ] **first release** Symbol: recently used symbols, the full character map by font and Unicode block, a shortcut key for a symbol, and an AutoCorrect entry for it *(symbols: ours 1%, public 13%)* *(shared: JeffPub's Symbol dialog)*
- [~] **first release** Special characters: nonbreaking space and hyphen, optional hyphen, en and em dash and space, quotation marks, line break (Shift+Enter), and no-width optional break *(shared: JeffPub)*

## Draw tab

- [ ] **first release** Ink drawn in a file is kept and written back as it was
- [ ] **later** Pens, pencil, and highlighter with their colors, thicknesses, and favorites; the eraser and the lasso; drawing with a mouse, pen, or finger
- [ ] **later** Ink to Shape and Ink to Math (handwriting becomes a clean shape or an equation), and Ink Replay
- [ ] **later** Ink Editor gestures (scratch out to delete, circle to select) and inking comments

## Design tab

- [ ] **1.0** Themes gallery: apply, save the current one, browse for one, and reset to the template's; a theme is its colors, fonts, and effects *(shared: libs/office theme, JeffDeck's themes)*
- [ ] **1.0** Theme Colors and Theme Fonts galleries (heading and body pairs) and Create New for each; changing them restyles the whole document *(shared: JeffPub's color and font schemes)*
- [ ] **1.0** Style sets gallery (a coordinated set of every style's look), paragraph spacing presets, Set as Default, and Reset to the template's set
- [ ] **1.0** Watermark: built-in text (Confidential, Draft, and others), custom text or picture with size, color, layout, and transparency, and Remove *(watermark-like header text: ours <1%, public 1%)*
- [~] **1.0** Page Color: theme colors, more colors, and fill effects (gradient, texture, pattern, picture); background colors print only if asked *(page background: ours 2%, public 1%)* *(0.9.0: a file's page color is drawn on the pages and in a PDF, and not printed on paper; none can be set)*
- [~] **1.0** Page Borders: box, shadow, 3-D, and custom; every line style, width, color, and art border; for the whole document, a section, or the first page; measured from the text or the page edge *(0.9.0: a file's page borders are drawn; none can be set)*

## Layout tab

### Page Setup
- [~] **first release** Margins: the presets (Normal, Narrow, Moderate, Wide, Mirrored), custom margins, gutter on the left or top, and "Mirror margins" so inside and outside margins swap on facing pages; multiple pages per sheet: normal, mirror margins, 2 pages per sheet, book fold, and reverse book fold; applied to the document, this section, or from this point on *(mirror margins: ours 20% (.docx 40%, .doc and .rtf <1%), public 1%)* *(shared: JeffPub's page setup)* *(0.9.0: the presets Normal, Narrow, Moderate, and Wide on the ribbon; a file's mirror margins and gutter are used)*
- [~] **first release** Orientation and Size: Letter, Legal, A4, A5, B5, Executive, envelopes, and the rest, a custom size, and More Paper Sizes; each section its own orientation *(landscape sections: ours 1%, public 7%)* *(0.9.0: portrait and landscape, and the paper sizes, on the ribbon)*
- [~] **first release** Columns: one to many, presets (two, three, left, right), custom widths and spacing, equal widths, a line between, and "from this point forward"; Column Break *(columns: ours 9% (.docx 16%, .doc and .rtf 2%), public 7%; column breaks: ours 1%, public 2%)* *(0.9.0: one, two, and three columns on the ribbon; a file's other column settings are laid out)*
- [~] **first release** Breaks: page, column, and text wrapping; section breaks: next page, continuous, even page, and odd page *(more than one section: ours 24% (.docx 42%, .doc and .rtf 7%), public 28%)* *(0.9.0: Insert > Page Break (Ctrl+Enter) and a next-page section break; a file's column breaks, continuous, even-page, and odd-page sections are laid out)*
- [~] **first release** Hyphenation: none, automatic, or manual, with "hyphenate words in capitals," the hyphenation zone, and a limit on hyphens in a row, in 12 languages; per-paragraph "don't hyphenate" *(automatic hyphenation: ours 2% of his .docx, public n/a (the conversion adds it); POI 6%; shared: libs/base/text hyphenation)* *(0.9.0: a file's automatic hyphenation, hyphenation zone, and per-paragraph "don't hyphenate" are used, with the bundled dictionary of the text's language; no menu sets it)*
- [ ] **first release** The Page Setup dialog: margins and gutter; paper size and source; layout tab with section start, different odd and even pages, different first page, header and footer distance from the edge, vertical alignment of the page (top, center, justified, bottom), line numbers, and borders; applied to a part of the document
- [~] **1.0** Line Numbers: continuous, restart each page or section, count by, start at, distance from text, and suppress for a paragraph *(ours 0%, public 1%)* *(0.9.0: a section's line numbers (each page, each section, or running on, counted by, started at, and skipped for a paragraph) are drawn in the margin when the file says so; none can be set)*
- [ ] **later** Text direction for a section (vertical text pages), gutter at the top, and the East Asian document grid (characters per line, lines per page)

### Paragraph
- [ ] **1.0** Indent left and right, and spacing before and after, as boxes on the tab

### Arrange
- [~] **first release** Wrap Text: in line with text, square, tight, through, top and bottom, behind text, and in front of text, for pictures, shapes, and text boxes *(pictures: ours 25% (.docx 12%, .doc and .rtf 39%), public 38%; shared: JeffPub's wrap engine)* *(files' square, tight, and top-and-bottom wraps are laid out, on both sides, the left, the right, or the larger one, with their distances; tight goes round the rectangle until the outlines are read; through, and wrap points, are not read; `allowOverlap`, `layoutInCell`, and mirrored margins' inside and outside are not done)*
- [~] **first release** Position: anchored to a paragraph, character, line, margin, page, or column, by offset or alignment; move with text, lock the anchor, and allow overlap *(files' positions from the page, margin, the strips beside the margins, column, paragraph, and line are laid out; from a character is from its line's start)*
- [ ] **1.0** Edit Wrap Points (the outline tight wrapping follows), distance from text, wrap on one or both sides, and Set as Default Layout
- [ ] **1.0** Bring Forward and Send Backward (also in front of and behind text), Align (to the page, margin, or each other, and distribute), Group, Ungroup, Regroup, and Rotate or Flip *(shared: JeffPub's Arrange)*
- [ ] **1.0** Gridlines, snap to the grid, alignment guides, and grid settings *(shared: JeffPub's guides)*

## References tab

### Table of Contents
- [~] **first release** Table of Contents: the gallery (Automatic Table 1 and 2, Manual Table) and the custom dialog: page numbers shown and right-aligned, tab leader, the built-in formats, levels 1 to 9, built from heading styles, outline levels, or table entry fields, and TOC 1 to 9 styles to modify *(TOC field: ours 19% (.docx 38%, .doc and .rtf <1%), public 7%; shared: JeffPub's automatic table of contents)* *(0.9.0: a file's table of contents is laid out as saved, its page numbers worked out from the pages; none can be made or updated)*
- [ ] **first release** Update Table (page numbers only, or the whole table), F9, Remove Table, and Add Text (mark a paragraph for a level) *(PAGEREF fields, which contents lines are made of: ours 19% (.docx 38%, .doc and .rtf <1%), public <1%)*
- [ ] **1.0** Mark entries for the table by hand (TC fields, Alt+Shift+O) *(TC field: ours <1%, public 2%)*

### Footnotes
- [~] **first release** Insert Footnote and Insert Endnote (Ctrl+Alt+F and Ctrl+Alt+D) with their dialog: number format (1, a, i, symbols, or a mark you type), start at, restart each section or page, location (bottom of page, below text, end of section, end of document), and notes in columns *(footnote references: ours 2% (.docx 3%, .doc and .rtf 0%), public 17%; endnote references: ours <1%, public <1%; shared: libs/text footnotes)* *(0.9.0: a file's footnotes and endnotes are set at the foot of the pages that cite them (or after the text, or at a section's end, as the file says), with its numbering, marks, separators, and continuation notices; none can be inserted, and their text cannot be edited)*
- [ ] **first release** Next and Previous Footnote or Endnote, Show Notes, double-click a mark to jump to its note, and the notes pane in Draft view
- [ ] **1.0** Convert footnotes to endnotes and back (all, or one), and swap them
- [ ] **1.0** Edit the separator, the continuation separator, and the continuation notice for footnotes and endnotes

### Citations and Bibliography
- [ ] **1.0** Insert Citation and Manage Sources (a master list shared among documents, beside the document's own): book, article, report, website, case, interview, and the other source types, placeholders, and the style (APA, MLA, Chicago, and the rest) *(in a sample of 399 of our `.docx`, none held a single source; the custom XML in 56% of them is Word's empty source list)*
- [ ] **1.0** Bibliography, Works Cited, and References galleries, and Update Citations and Bibliography
- [ ] **later** **(alt)** Citation styles in the open CSL format, and sources imported from BibTeX, RIS, and Zotero exports, in place of Word's own style files

### Captions
- [ ] **1.0** Insert Caption: label (Figure, Table, Equation, or your own), above or below, numbering with the chapter number, exclude the label, and AutoCaption *(SEQ field: ours <1%, public 3%)*
- [ ] **1.0** Insert Table of Figures from a caption label or a style, and update it
- [ ] **first release** Cross-reference to a heading, numbered item, bookmark, footnote, endnote, caption, or page, as its text, number, page, or "above" and "below," inserted as a link *(also the Insert > Links command; PAGEREF: ours 19% (.docx 38%, .doc and .rtf <1%), public <1%)*

### Index
- [ ] **1.0** Mark Entry and Mark All (main entry, subentry, cross-reference such as "see," a page range from a bookmark, bold or italic page number) *(XE field: ours 0%, public 1%; legal and book indexes depend on it)*
- [ ] **1.0** Insert Index: formats, columns, right-aligned page numbers, tab leader, and sorting by language, with Update and AutoMark from a list of words

### Table of Authorities
- [ ] **1.0** Mark Citation (long and short form, category) and Insert Table of Authorities (cases, statutes, rules, treatises, regulations, constitutional provisions, and other categories; "passim"; original formatting kept), with Update *(not measured by the counts; legal briefs use it)*

## Mailings tab

- [x] **first release** Documents that already hold merge fields keep them on a round trip (MERGEFIELD, NEXT, IF, ASK, and the rest), and their data-source link is kept *(MERGEFIELD: ours 0%, public <1%; ADDRESSBLOCK: ours <1%, public 0%)*
- [ ] **1.0** Start Mail Merge: letters, email messages, envelopes, labels, and directory, the Step-by-Step Wizard, and Clear Merge Type *(shared: JeffPub's mail merge and the merge source in libs/page/core)*
- [ ] **1.0** Select Recipients: type a new list, use an existing one (CSV, tab-delimited, `.xlsx`, `.ods`, vCard contacts), or choose contacts **(alt: any address book saved as `.vcf`)** *(shared: JeffPub)*
- [ ] **1.0** Edit Recipient List: sort, filter, find duplicates, validate addresses, and include or exclude records *(shared: JeffPub)*
- [ ] **1.0** Insert fields: Address Block and Greeting Line (formats, country rules, preview), any merge field, Match Fields, Highlight Merge Fields, and Update Labels *(shared: JeffPub)*
- [ ] **1.0** Rules: If...Then...Else, Ask, Fill-in, Merge Record #, Merge Sequence #, Next Record, Next Record If, Set Bookmark, and Skip Record If
- [ ] **1.0** Preview Results with record navigation, Find Recipient, and Check for Errors *(shared: JeffPub)*
- [ ] **1.0** Finish & Merge: Edit Individual Documents (a new document), Print Documents, and Send Email Messages **(alt: one `.eml` file per recipient)** *(shared: JeffPub)*
- [ ] **1.0** Envelopes: delivery and return addresses, the common envelope sizes, feed direction, and print or add to the document
- [ ] **1.0** Labels: a full page of one label or a single label, the common vendors' sheet sizes and your own, and print or save as a document *(shared: JeffPub's several-to-a-sheet printing)*
- [ ] **later** Merge from a JeffBase table or a JeffCrunch worksheet directly, without exporting a file *(shared with JeffBase and JeffCrunch)*
- [ ] **later** Japanese greetings and postcards, Chinese envelopes, and the Japanese postal barcode

## Review tab

### Proofing
- [~] **first release** Spelling as you type (wavy underlines, suggestions on right-click, Ignore All, Add to Dictionary, AutoCorrect), the Spelling pane, custom and exclusion dictionaries, and "ignore words in capitals or with numbers" *(done: red wavy underlines, suggestions on right-click, Ignore All, Add to Dictionary, the Spelling pane (F7, Review > Spelling & Grammar) with Change, Change All, Ignore, Ignore All, and Add, and skipping capitals and words with numbers; missing: AutoCorrect, exclusion dictionaries and editing the custom one, and underlines in tables, headers, footers, and notes)* *(shared: libs/shell spelling and libs/base/text dictionaries, 12 languages)* **(needed for the Preview)**
- [~] **first release** Set the proofing language of selected text, mark text "do not check spelling," and detect the language as you type; the language shows in the status bar *(done: the language of each run and "do not check spelling" are read from the file and spelling follows them; missing: setting them, detecting the language as you type, and the status bar)* *(shared: JeffPub's language setting)*
- [~] **first release** Word Count: pages, words, characters with and without spaces, paragraphs, and lines, with or without text boxes, footnotes, and endnotes; the count of a selection in the status bar *(0.9.0: the status bar shows the page and the words)*
- [ ] **1.0** **(alt)** Thesaurus and Research: synonyms and related words offline, in place of Microsoft's online Researcher and Insights *(shared: JeffPub's thesaurus)*
- [ ] **1.0** Read Aloud with the system's voices: from the cursor, with the current word highlighted, and a speed and voice choice
- [ ] **1.0** Check Accessibility and its pane (also reached from Info): missing alt text, tables without header rows, blank cells, merged cells, heading order, hard-to-read colors, floating objects, and unhelpful link text

### Language
- [ ] **1.0** **(alt)** Translate: the selected text or the whole document opened in an open translation service (LibreTranslate, with a server address you can change), instead of Microsoft Translator *(shared: JeffPub's Translate)*
- [ ] **later** Chinese conversion between simplified and traditional, Hangul and Hanja conversion, the Japanese consistency checker, and IME dictionary tools

### Comments
- [~] **first release** Comments: new, edit, reply in threads, resolve and reopen, delete (one, all shown, or all), Previous and Next, in balloons in the margin or in the Reviewing pane, with author and initials; read and written back with their replies *(comments: ours <1%, public 1%; kept for a document that comes back from an editor)* *(0.9.0: a file's comments, with their ranges and replies, are kept and written back; none is shown, and none can be added)*
- [ ] **later** Ink comments and @mentions

### Tracking and changes
- [ ] **first release** Track Changes on and off (Ctrl+Shift+E): inserted and deleted text, formatting changes, moves, and changes to paragraphs, tables, and numbering, each with author and date *(a document that comes back from an editor must open with its markup; tracked insertions: ours 1%, public 5%; deletions: ours 1%, public 3%; tracking left on: ours 1%, public 4%)*
- [ ] **first release** Accept and Reject: the current change, then move on; all shown; all in the document; and accept all and stop tracking
- [ ] **first release** Display for Review (Simple Markup, All Markup, No Markup, Original), Show Markup by kind and by person, balloons (changes in balloons, inline, or only comments and formatting), Highlight Updates, and the Reviewing pane (vertical or horizontal)
- [ ] **1.0** Track Changes Options: colors by author and kind, change bars, moved text, and formatting changes; change the user name; Lock Tracking with a password

### Compare
- [ ] **1.0** Compare two versions into one document of tracked changes (by word or character; ignoring case, formatting, white space, tables, headers and footers, notes, text boxes, fields, and moves), and Combine revisions from several authors; Show Source Documents *(legal publishing leans on it; see the open questions)*

### Protect
- [ ] **1.0** Restrict Editing: limit formatting to chosen styles, and allow only tracked changes, comments, or filling in forms, with an optional password *(document protection: ours <1%, public 1%)*
- [ ] **later** Co-authoring locks and Block Authors: not planned (they depend on a cloud service)

## View tab

### Views
- [~] **first release** Print Layout (pages as they print), Draft (one continuous column, with a style area), and Outline *(shared: JeffPub's page view)* *(0.9.0: Print Layout only)*
- [ ] **1.0** Web Layout and Read Mode (the page fitted to the window, side to side, with column width, page color, and layout choices)
- [ ] **1.0** Focus mode: the ribbon and window parts out of the way
- [ ] **later** Immersive Reader tools: column width, page color (none, sepia, inverse), text spacing, syllables, Read Aloud, and Line Focus (one, three, or five lines lit at a time)

### Show and Zoom
- [ ] **first release** Ruler (horizontal and vertical, with indents, tab stops, and margins that can be dragged) and Gridlines *(shared: JeffPub's rulers)*
- [ ] **first release** Navigation pane: Headings (an outline you can click, drag to reorder, promote, demote, and collapse), Pages (thumbnails), and Results
- [ ] **1.0** Collapse and expand headings in the document itself
- [x] **first release** Zoom: a dialog (percent, page width, text width, whole page, many pages), 100%, One Page, Multiple Pages, Page Width, a slider in the status bar, and Ctrl+wheel *(shared: JeffPub's zoom)*
- [ ] **1.0** Page movement: vertical or side to side
- [~] **first release** Status bar: page x of y, word count, language, proofing errors, tracking, section, line, and column, with the view buttons and the zoom slider *(shared: JeffPub's and JeffDeck's status bars)* *(0.9.0: the page, the word count, and the zoom slider)*

### Window
- [ ] **1.0** New Window, Arrange All, Switch Windows, View Side by Side with synchronous scrolling, Reset Window Position, and Split (two panes onto one document) *(shared: JeffPub's windows, apart from Split)*
- [x] **first release** Collapse the ribbon with Ctrl+F1, a double-click on a tab, or its chevron *(shared: JeffPub)*

## Developer tab (hidden until turned on)

- [x] **first release** Content controls are shown and kept as saved, including the table of contents' own wrapper *(content controls: ours 21% (.docx 42%, .doc and .rtf <1%), public 7%; in a sample of 400 of our `.docx`, 70 had a control in the body and 69 of those were the contents' own wrapper; headers and footers were not checked)*
- [ ] **1.0** Insert and edit content controls: rich text, plain text, picture, building-block gallery, check box, combo box, drop-down list, date picker, and repeating section, with title, tag, locking, and Design Mode; Group and Ungroup
- [~] **1.0** Legacy form fields (text, check box, drop-down) and Restrict Editing to filling in forms *(legacy form fields: ours <1%, public 2%; check box field: ours <1%, public 2%)* *(0.9.0: a file's form fields show their box (checked or not), their default text, or the entry chosen; none can be inserted or filled in)*
- [~] **later** Macros are kept and never run; recording or writing them needs a scripting language first (see the open questions) *(macros: ours 0%, public 0%; POI 2%)* *(0.9.0: a `.docm` opens and its macros are never run; saving it as `.docx` leaves them out and says so)*
- [ ] **later** XML mapping: bind content controls to custom XML, and the Schema, Transform, and Expansion Packs commands
- [ ] **later** ActiveX and web controls on a form

## Help tab

- [~] **first release** Help (F1): a built-in, offline pane of plain-language topics with a contents page, links, Back and Forward, and a search that finds topics and commands *(shared: libs/shell help)*
- [~] **first release** Keyboard Shortcuts (a table made from the commands themselves), What's New, Feedback (a GitHub page with the version filled in), and About with Check for Updates *(shared: libs/shell)*
- [~] **1.0** A search box on the ribbon that finds a command by what it does and runs it *(shared: the help search)*

## Outline view and master documents

- [ ] **1.0** Outline view: promote, demote, promote to heading, demote to body text, move up and down, expand and collapse, show a level (1 to 9 or all), show text formatting, show first line only, and drag a heading's symbol to move its section
- [ ] **later** Master Document: show subdocuments, create, insert, unlink, merge, split, lock, and expand or collapse them; a file that has them opens as a single document with its parts kept

## Contextual tabs

The tabs that appear only while something is selected or being edited: a table, a picture, a shape or text box, a header or footer, an equation, and a chart or diagram.

### Table Design and Layout tabs

- [ ] **first release** Table styles gallery (grid, list, and plain styles), the style options (header row, total row, banded rows, first and last column, banded columns), Modify Table Style, New Table Style, and Clear *(table styles: ours 12% (.docx 23%, .doc and .rtf 0%), public 0%; shared: JeffPub's Table Formats gallery)*
- [ ] **first release** Shading and borders: every border style, width, and color on any side or inside, the border painter, the Borders and Shading dialog, and View Gridlines *(shading: ours 6%, public 17%)*
- [~] **first release** Select a cell, row, column, or table; insert rows above or below and columns left or right; delete cells, rows, columns, or the table; the Tab key at the last cell adds a row *(0.9.0: the Tab key at the last cell adds a row, and the editing functions add and take away rows and columns and keep merged cells whole; no commands or menus offer them yet)*
- [ ] **first release** Merge Cells and Split Cells (across and down), and Split Table *(merged across: ours 4%, public 20%; merged down: ours 1%, public 8%; shared: JeffPub's merged cells)*
- [~] **first release** Nested tables, a table inside a cell *(nested: ours 2%, public 2%)* *(0.9.0: nested tables are laid out down to a limit, and past it the text of the tables stays as plain paragraphs)*
- [ ] **first release** Cell size: row height (at least or exactly), column width (points or percent), AutoFit to contents, to the window, or fixed, and Distribute Rows and Columns *(shared: JeffPub)*
- [ ] **first release** Table Properties: alignment, indent, text wrapping around the table, preferred width, default cell margins, space between cells, "allow row to break across pages," **Repeat as header row at the top of each page**, vertical alignment, no wrap, fit text, and alt text *(repeating header rows: ours 1%, public 7%)*
- [ ] **first release** Cell alignment in nine positions, and text direction in a cell
- [ ] **1.0** Draw Table and Eraser, with the pen style, weight, and color
- [ ] **1.0** Formula in a cell (=SUM(ABOVE), =AVERAGE(LEFT), and the functions, with number formats), updated with F9 *(shared: a calculation engine with JeffCrunch)*
- [ ] **1.0** Table handles: a move handle at the corner, a resize handle, and dragging borders
- [~] **later** Floating tables that text wraps around, and diagonal borders in a cell *(files' floating tables are laid out where they ask, with the text round them, if they fit a page of their section, and move to the next page when they would go over the foot; `tblOverlap` is not used; not edited)*

### Picture Format tab

- [~] **first release** Crop (to a shape, to an aspect ratio, fill, fit), size (height and width, locked aspect, scale), rotate, and Reset Picture; Compress Pictures (resolution, delete cropped areas) *(shared: JeffPub's picture formatting)* *(0.9.0: a file's crops, transparent colors, and borders are drawn; none can be set)*
- [ ] **first release** Alt text (the pane) and Mark as Decorative, for pictures, shapes, charts, and tables
- [ ] **1.0** Corrections (sharpen and soften, brightness and contrast), Color (saturation, tone, recolor, set transparent color), and artistic effects *(shared: libs/draw blip effects, as in JeffDeck)*
- [ ] **1.0** Picture styles gallery, border (color, weight, dashes), and effects (preset, shadow, reflection, glow, soft edges, bevel, 3-D rotation)
- [ ] **1.0** Change Picture (from a file, online, or the clipboard) and the Format Picture pane
- [ ] **later** Remove Background by marking areas to keep and remove, and Picture Layout (turn pictures into SmartArt)

### Shape Format, Text Box, and WordArt tabs

- [~] **first release** Shape styles gallery; fill (solid, gradient, picture, texture, pattern); outline (color, weight, dashes, arrow ends); effects (shadow, reflection, glow, soft edges, bevel, 3-D rotation); and the Format Shape pane *(shared: libs/draw)* *(0.9.0: a file's fills and outlines are drawn; none can be set)*
- [~] **first release** Text in shapes and text boxes: direction, alignment, margins, wrap, autofit, columns, and Create and Break Link between text boxes *(shared: JeffPub)* *(0.9.0: a file's text in shapes and text boxes is laid out in the box, with its tables; none can be edited yet)*
- [ ] **1.0** Edit Shape: change the shape, Edit Points, and Reroute Connectors; Insert Shapes on the tab
- [~] **1.0** WordArt styles on text: fill, outline, effects, and warps *(shared: libs/draw text art)*
- [ ] **later** The older text box and WordArt tabs (3-D extrusion with tilt, depth, and lighting, shadow nudges), for documents made in older versions *(shared: libs/draw)*

### Header & Footer tab

- [~] **first release** Header and Footer: Edit, Remove, and the gallery of built-in designs (blank, three columns, and the others); Page Number at the top, bottom, in the margins, or at the cursor *(shared: JeffPub's master page header and footer)* *(0.9.0: a file's headers and footers are shown on its pages, with their tables, pictures, and fields; none can be edited, added, or removed)*
- [~] **first release** Format Page Numbers: 1 2 3, a b c, A B C, i ii iii, I II III, dashed, with the chapter number (heading style and separator), continue from the previous section, or start at *(page number formats: 38% of our `.docx`, the only trustworthy count; PAGE field: ours 38% (.docx 64%, .doc and .rtf 11%), public 54%; NUMPAGES: ours 5%, public 8%)* *(0.9.0: a section's number format, chapter number, and start are read, kept, and used; none can be set)*
- [~] **first release** Different First Page and Different Odd & Even Pages *(first page: ours 6%, public 30%; odd and even headers: ours 20% (.docx 38%, .doc and .rtf <1%), public 5%)* *(0.9.0: a file's first-page, odd, and even headers and footers are used; none can be set)*
- [~] **first release** Link to Previous for each header and footer of each section; Previous and Next section; Go to Header and Go to Footer; and the kinds of header and footer (first, odd, and even) for every section *(headers: ours 35%, public 46%; footers: ours 35%, public 58%)* *(0.9.0: a section without a header or footer of its own takes the one before it, as the file says; no commands)*
- [~] **first release** Insert in a header or footer: Date and Time, Document Info (author, file name, path, title), Quick Parts and fields, **STYLEREF so a running head shows the current chapter or heading**, and pictures *(STYLEREF: ours 15% (.docx 29%, .doc and .rtf <1%), public <1%)* *(0.9.0: STYLEREF shows the page's paragraph of the style (the first, the last with \l, or the nearest before it), and pictures and fields a file has are shown; nothing can be inserted)*
- [~] **first release** Header from top and Footer from bottom distances, Insert Alignment Tab (left, center, right, relative to the margin or the indents, with a leader), and Show Document Text *(0.9.0: a file's distances and absolute position tabs are used; none can be set)*

### Equation tab

- [ ] **1.0** Insert Equation (Alt+=): the built-in equations, writing by typing (`\alpha` and the rest turn into symbols), and saving to the gallery *(equations: ours 0%, public 2%; shared: an equation engine to build for JeffDeck and JeffCrunch)*
- [ ] **1.0** Structures: fractions, scripts, radicals, integrals, large operators, brackets, functions, accents, limits, operators, and matrices; and the symbol galleries (basic math, Greek, letter-like, operators, arrows, negated, scripts, geometry)
- [ ] **1.0** Professional and Linear forms with Unicode or LaTeX input, Normal Text inside an equation, and equation options (math AutoCorrect, math regions)
- [~] **1.0** Equations read and saved as Office Math in `.docx` and `.doc`, and as MathML in `.odt`, drawn on screen, in print, and in PDF *(0.9.0: an equation is read from `.docx`, drawn as a line of text in its order, and written back as it was; it is not laid out as printed)*
- [ ] **later** Ink Equation

### Chart, SmartArt, and graphics tabs

- [ ] **1.0** Chart Design and Format: add or remove elements (axes, titles, data labels, data table, error bars, gridlines, legend, lines, trendline, up/down bars), quick layouts, colors, styles, switch row and column, select and edit data, change chart type, and format each part *(shared: libs/office chart)*
- [ ] **1.0** Graphics tab for SVG pictures: change the graphic, Convert to Shape, styles, fill, outline, and effects *(shared: JeffPub's SVG to shapes)*
- [ ] **later** SmartArt Design and Format: add shapes, promote and demote, the text pane, layouts, colors, styles, reset, and organization charts *(shared: JeffDeck's SmartArt drawing)*
- [ ] **later** 3-D Model tab: scene views and reset

## Shortcut menus (right-click)

What a right-click offers depends on what is under the pointer. The commands are the ones the ribbon already holds, put where the original puts them.

- [~] **first release** On text: Cut, Copy, and Paste Options (keep the source formatting, merge formatting, or keep text only), Font, Paragraph, Bullets and Numbering, Hyperlink, and New Comment *(Cut, Copy, Paste, Font, Paragraph, and Select All; the paste choices, lists, link, and comment are not there yet)*
- [~] **first release** On a word the spelling check flags: suggestions, Ignore All, Add to Dictionary, and the AutoCorrect options *(done: suggestions, Ignore All, and Add to Dictionary; missing: the AutoCorrect options)* **(needed for the Preview)**
- [ ] **first release** In a table: Insert and Delete for rows, columns, and cells, Select, Merge Cells, Split Cells, Borders and Shading, AutoFit, and Table Properties
- [ ] **1.0** On a picture, shape, or text box: Cut, Copy, Paste, Wrap Text, Bring to Front and Send to Back, Group, Edit Alt Text, Size and Position, and Format
- [ ] **1.0** On a field, a link, a footnote, or a tracked change: Update Field, Edit Field, and Toggle Field Codes; Open, Edit, and Remove Hyperlink; Go to a note; and Accept or Reject a change
- [ ] **1.0** On a comment: Reply, Resolve, and Delete

## Under the ribbon: the text and page engine

### Editing basics
- [~] **first release** Typing and selecting: move by character, word, sentence, line, paragraph, and page; extend with Shift, F8, or the mouse; select a column with Alt+drag; select several places with Ctrl; double- and triple-click; Click and Type *(shared: the text editing in libs/page/app)*
- [~] **first release** Undo and Redo with a history list, and Repeat (F4 or Ctrl+Y) *(0.9.0: Undo and Redo, with typing and each command one step; no history list or Repeat)*
- [ ] **first release** Drag-and-drop to move or copy text, with automatic scrolling
- [ ] **first release** AutoCorrect and AutoFormat as you type: smart quotes, lists started by typing "1." or "*", borders from a row of dashes, ordinals, fractions, dashes, web addresses turned into links, and *bold* and _underline_ marks turned into formatting *(shared: JeffPub's AutoCorrect)*
- [ ] **1.0** Spike (collect several cuts and paste them together), Overtype, and the Insert key
- [~] **first release** Word's keyboard shortcuts (Ctrl+B, Ctrl+Alt+1, F5, F7, Alt+F9, Shift+F3, and the others), with no two commands on one key, and all of them reassignable in Options *(shared: JeffPub's shortcut table)*
- [~] **first release** Text input for every script: input methods, dead keys, and the system's emoji and symbol pickers

### Pagination and sections
- [x] **first release** Pages flow from the text: a document is a run of paragraphs and tables laid out into pages by size, margins, columns, floating objects, footnotes, and the keep rules; pages are laid out as needed and in the background, so a long document opens at once and typing never waits for the last page *(the largest of our `.docx` has 26,443 paragraphs and the largest government file 45,013; one in ten of our `.docx` has more than 2,200 paragraphs and one in ten government files more than 1,400)*
- [~] **first release** Line and page breaks match Word's for the same fonts, in the way JeffPub's match Publisher's *(shared: libs/text metrics and the font-width tables)* *(0.9.0: on the 95 documents held out from tuning, 73% of the pages score under 2 and all but 0.16% of the words appear on JeffWrite's pages; a line or a page still breaks differently in places)*
- [~] **first release** Sections: each with its own page size and orientation, margins, columns, headers and footers, page-number format and start, vertical alignment, line numbering, and footnote numbering; start types next page, continuous, even page, and odd page *(more than one section: ours 24% (.docx 42%, .doc and .rtf 7%), public 28%; landscape sections: ours 1%, public 7%)*
- [~] **first release** Facing pages and mirrored margins: left and right pages swap inside and outside margins and gutter, and a section that starts on an odd or even page gets the blank page it needs *(mirror margins: ours 20% (.docx 40%, .doc and .rtf <1%), public 1%; odd and even headers: ours 20% (.docx 38%, .doc and .rtf <1%), public 5%)* *(0.9.0: mirrored margins swap the sides on pages with an even number, in the text, the headers and footers, and the page borders, and a section that starts on an odd or even page gets the blank page it needs; an object set in the inside or outside strip of a margin is still set as on an odd page)*
- [~] **first release** Columns: equal or unequal widths, a line between, balanced by a continuous section break, a column break, and columns inside text boxes and table cells *(columns: ours 9% (.docx 16%, .doc and .rtf 2%), public 7%; shared: columns in libs/text)*
- [~] **first release** Floating pictures, shapes, and text boxes: text wraps around their shape, they are anchored to a paragraph and move with it, and they never push the text out of the page's headers, footers, or notes *(pictures: ours 25% (.docx 12%, .doc and .rtf 39%), public 38%)*
- [~] **first release** Frames (paragraphs positioned on the page, which older files use for drop caps, side notes, and captions): laid out where the file puts them and kept, with the Frame dialog to edit them at 1.0 *(frames: ours 1%, public <1%)* *(laid out and kept; the caret, a click, Home, End, and Up and Down work in their text, and no edit joins a frame to the text round it; the Frame dialog, and a selection's highlight in the frame's box, are not there yet)*
- [~] **first release** Tables across pages: rows split or kept whole, header rows repeated, nested tables, and tables wrapped by text; a table in a column or a text box *(0.9.0: a table's rows split across pages or stay whole as the file says, header rows repeat, nested tables and tables in a column, a text box, or a header are laid out, and a table set at a place is laid out there)*
- [~] **first release** Widow and orphan control, keep with next, keep lines together, and page break before, as Word applies them (including the cases where it gives up)
- [ ] **1.0** Document grid and East Asian layout: characters per line, lines per page, and vertical pages

### Notes
- [~] **first release** Footnotes at the bottom of the page their mark lands on, under a separator; a note too long for the page continues on the next under a continuation separator and notice; numbering across the document, by section, or by page; custom marks *(shared: footnote layout in libs/text, extended from columns to pages)* *(0.9.0: done as the item says, with the file's numbering, custom marks, separators, and notices; no commands insert one)*
- [~] **first release** Endnotes at the end of the section or document, with their own numbering, and placed before or after the last page's text
- [~] **1.0** Notes for text in a column layout, in a table, and in a text box, handled as Word handles them *(0.9.0: a note cited in a continuous two-column section or a table's cell is set at the page's foot, and a note longer than the page continues)*

### Fields
- [~] **first release** The field model: a field is a code and a result, both kept; show codes or results (Alt+F9 and per field), update (F9, all, before printing), lock, unlock, and unlink (Ctrl+Shift+F9), nested fields, shading, and the switches for formats (\* MERGEFORMAT, \* Upper, \# number pictures, \@ date pictures) *(0.9.0: every field is kept with its code and its result and written back as it was, and the page, page count, section pages, style reference, and page reference fields are worked out from the laid-out pages; there is no show codes, update, lock, or unlink, and the switches for formats are read only for the page number)*
- [~] **first release** Fields that must work: PAGE, NUMPAGES, SECTIONPAGES, SECTION, STYLEREF, REF, PAGEREF, NOTEREF, TOC, HYPERLINK, and SEQ *(PAGE: ours 38%, public 54%; PAGEREF: ours 19%, public <1%; STYLEREF: ours 15%, public <1%; TOC: ours 19%, public 7%; HYPERLINK: ours 2%, public 3%; SEQ: ours <1%, public 3%)* *(0.9.0: PAGE (with its number formats and chapter number), NUMPAGES, SECTIONPAGES, STYLEREF, and PAGEREF are worked out; the others (REF, NOTEREF, TOC, HYPERLINK, SEQ, and SECTION) show the result the file has)*
- [ ] **1.0** Fields that follow: DATE, TIME, CREATEDATE, SAVEDATE, PRINTDATE, FILENAME, AUTHOR, TITLE, SUBJECT, DOCPROPERTY, IF, COMPARE, SET, ASK, FILLIN, QUOTE, LISTNUM, SYMBOL, TC, XE, INDEX, TA, TOA, and formula (=) *(DATE: ours 1%, public 2%; FILENAME: ours 0%, public 2%; DOCPROPERTY: ours 0%, public <1%; INDEX: ours 0%, public 1%)*
- [x] **later** Fields that rarely appear: DATABASE, INCLUDETEXT, INCLUDEPICTURE, LINK, EQ, ADVANCE, GOTOBUTTON, MACROBUTTON, AUTOTEXTLIST, and ADDIN; they are kept and shown as saved *(EQ: ours 0%, public <1%; GOTOBUTTON: ours 0%, public <1%)*
- [ ] **1.0** The Field dialog: choose a category and a field, set its properties and options, and see the code it writes

### Styles
- [~] **first release** The style model: paragraph, character, linked, table, and list styles; based-on chains; the next-style rule; the document's defaults; latent styles; and Word's order for combining them (table, paragraph, character, then direct formatting), with toggle properties such as bold working as the standard says *(shared: JeffPub's style engine)*
- [x] **first release** Direct formatting kept as the file stores it, not folded into styles, so that a file saved without edits changes nothing
- [~] **1.0** Theme references: fonts and colors that follow the theme (heading and body fonts, accent colors with tints), so a theme change restyles the document *(shared: libs/office theme)* *(0.9.0: a file's theme fonts and colors are read and used; no command changes a theme)*

### Numbering
- [~] **first release** The numbering model: abstract definitions and the lists that use them, nine levels, level overrides, restart and continue rules, "start at," legal numbering (1.1.1), picture and symbol bullets, the text and tab after a number, and numbering linked to heading styles *(lists: ours 44% (.docx 59%, .doc and .rtf 27%), public 52%; the 627 `.docx` of our that use lists hold 361,894 numbered paragraphs between them)* *(0.9.0: definitions, nine levels, overrides, restarts and continuing, start at, legal numbering, symbol bullets, the text and tab after a number, and links to styles are read, laid out, and kept; a picture bullet is not drawn)*
- [ ] **1.0** Number formats of other scripts and systems: Arabic-Indic, Hebrew, Thai, Chinese and Japanese counting, ordinal text ("first," "second"), and chapter-style numbers

### Review model
- [~] **first release** Tracked changes as the file keeps them: insertions, deletions, moves from and to, and property changes for runs, paragraphs, sections, tables, rows, cells, and numbering, with author and date; accepting and rejecting give the same text Word's do *(0.9.0: insertions and deletions are read and kept with their authors and dates (also from `.doc`), deleted text is hidden and takes no room, and the markup is written back as it was; no markup is shown, and none can be accepted, rejected, or made)*
- [x] **first release** Comments with their ranges, replies, resolved state, authors, and initials, written back whole even when they are not shown
- [ ] **1.0** Document comparison that finds moves and formatting differences, not only insertions and deletions

### Compatibility with older layouts
- [~] **first release** Compatibility mode: the file's compatibility setting (Word 97 to 2003, 2007, 2010, and 2013 or later) picks the layout rules used, as Word does, and shows "Compatibility Mode" in the title with a Convert command *(0.9.0: the settings that change layout (the space at the top of a page after a break, line spacing, punctuation that hangs, East Asian layout, hyphenation, mirrored margins) are read and honored one by one; there is no "Compatibility Mode" title or Convert command)*
- [~] **first release** Documents opened from `.doc`, `.rtf`, and older `.docx` lay out as they did in the program that made them, with the options that change layout (spacing at the top of a page, table cell margins, justification, line height) honored one by one *(0.9.0: `.doc` and older `.docx` files lay out by the options their files hold; `.rtf` is not opened)*
- [~] **1.0** The per-document list of compatibility options, kept and written back, with those that never change layout (such as "don't use HTML paragraph auto spacing") ignored on screen *(0.9.0: the settings are kept and written back, and the ones that change layout are used)*
- [~] **first release** Fonts: embedded fonts in a file (the obfuscated `.odttf` parts) are read and used for that document, and a missing font is replaced by one with the same widths so lines break where they did *(embedded fonts: ours 18% (.docx 35%, .doc and .rtf 0%), public 0%; shared: libs/office font readers and the width tables)* *(0.9.0: a missing font is replaced by one with the same widths, so that lines break where they did (a table of the stand-ins and the widths of the common fonts); embedded fonts are not read)*
- [~] **1.0** Embed fonts when saving, only where each font's own license bits allow it, with "only the characters used" and "don't embed common fonts" *(shared: libs/base sfnt)*

### Text and languages
- [~] **first release** Complex scripts and bidirectional text (Arabic, Hebrew, Indic, Thai), symbol fonts (Wingdings, Symbol) mapped to their letters, and a font fallback when a letter is missing *(right-to-left: ours 4%, public 10%; shared: Qt shaping in libs/text)*
- [~] **first release** Each run carries its language; spelling and hyphenation follow it, and "no proofing" is kept *(done: spelling follows the run's language, else its style's, else the document's default, and "no proofing" is kept; hyphenation follows the run's own language, and text that states none is hyphenated as US English)* *(shared: libs/base/text)*
- [ ] **later** East Asian line breaking (kinsoku), ruby, vertical text, and half- and full-width forms

### Speed
- [ ] **1.0** Large pictures and many of them are decoded as needed, and scaled copies are cached, so scrolling stays smooth

### Platform and accessibility
- [x] **first release** Light and dark interface, following the system's setting, and Windows high contrast *(shared: libs/shell theme)*
- [x] **first release** KeyTips and the ribbon by keyboard (Alt, then letters; Tab and arrows; Escape to the page) with a visible focus ring *(shared: libs/shell)*
- [~] **first release** Screen readers: every control named and described, and the document read by paragraph and line, with objects by kind, alt text, and place *(shared: libs/shell and the canvas's accessible interface; the flowing text's own is new)*
- [x] **first release** The program and its Help in seven languages (English, German, Spanish, French, Italian, Dutch, Portuguese), following the system or the Options choice *(shared: libs/shell i18n; JeffWrite's own strings are new)*
- [x] **first release** Update check at start, and opt-in statistics and crash reports, with the one choice shared across the suite *(shared: libs/shell updater, telemetry, and crash)*
- [~] **first release** File associations for `.docx`, `.doc`, `.rtf`, `.odt`, `.dotx`, and `.ott`, and installers for Windows, Linux (`.deb` and AppImage), and macOS *(shared: the suite's packaging)* *(0.9.0: associated: `.docx`, `.dotx`, and `.doc`, with installers for Windows, Linux, and macOS)*
- [ ] **1.0** Touch and pen input, and high-DPI scaling on every system
- [ ] **later** A right-to-left interface for Arabic and Hebrew

## Fidelity and testing

JeffWrite will be checked against reference pages, not against a guess. JeffWrite's pages are scored against reference PDFs and pictures, on the scale JeffPub's comparison uses (a page drawn small, blurred, and compared in gray, where 0 is identical). Reference pictures stay on the build machine and never go in the repository.

- [~] **first release** Every file JeffWrite saves (`.docx`, and `.doc`, `.rtf`, and `.odt` too) opens without a repair message, checked on a large collection *(0.9.0: `.docx` is the only format saved; the tests read back what is saved, strictly, and compare it with what was opened)*
- [~] **first release** Our own collection of 2,082 working documents is the main test set: each document is opened, drawn, saved, and opened again, and its pages are scored against the reference PDF of it *(0.9.0: the yardstick is a set of documents with reference PDFs and pictures, a half to tune on and a half held out, and a rendering of each is scored page by page)*
- [x] **first release** Documents held out from tuning are scored apart from the ones used for tuning, so the numbers are not flattered by the files the layout was fitted to
- [~] **1.0** The public test collections: Govdocs1's government `.doc` files and Apache POI's Word test files (edge cases and damaged files) run through the readers and the save-and-reopen check, each file that does not open or does not save back listed with its cause *(0.9.0: the release sweep ran JeffWrite over 2,289 files, among them hundreds of damaged or deliberately broken ones; none made it close or run out of memory)*
- [ ] **1.0** A repeatable comparison anyone can run, made from `jwritetool` and reference PDFs, with the numbers written into each release's notes

## Safety and limits

Suite-wide rules decided October 11, 2026 (see Decided): nothing in a file runs or reaches out until the person allows it, every file reader is fuzzed and held to limits on memory and time, and a damaged file opens as far as it can without crashing.

- [~] **first release** A damaged or hostile file (a zip bomb, deep nesting, a huge table, a loop of linked parts) is opened within limits on time and memory, or refused with a clear message; a sweep of hostile files runs before each release *(shared: the hostile-file sweep of the suite)* *(0.9.0: a file over 1 GB is refused before it is read, and the files of the release sweep (hundreds of them damaged or built to break it) neither closed the program nor ran it out of memory; a few built to be enormous are still laid out in the background)*
- [x] **first release** Opening a file never changes it; saving writes a new file beside the old and replaces it only when complete
- [~] **first release** Nothing in a document runs or reaches out until the person allows it: macros, external links, linked pictures, remote templates, and remote content *(the rule is decided for the whole suite; macros are kept and never run, see the Developer tab)*
- [~] **first release** Every file reader (`.docx`, `.doc`, `.rtf`, `.odt`, and the pictures inside them) is fuzzed and held to limits on memory and time *(the rule is decided and the suite's hostile-file sweep exists; JeffWrite's `.docx` and `.doc` readers are held to limits by tests of hostile files and fuzzed by random editing; there are no `.rtf` or `.odt` readers yet)*
- [~] **first release** A damaged file opens as far as it can, lists what it left out, and never crashes the program *(0.9.0: a file that is not a document says so and opens nothing, a file that could not be read whole is not saved over its original, and what an old `.doc` holds that cannot be shown is listed)*

## File formats

What each format needs, to open, to save, and to come back unchanged. "Round trip" means open, save, and open again without a repair message or a visible change.

### Word `.docx` and `.dotx` (Open XML)
- [~] **first release** Open `.docx` from Word 2007 to 2021 (Transitional): body text and every kind of formatting, styles, numbering, sections and page setup, headers and footers, notes, fields with their results, bookmarks, links, tables, pictures, DrawingML shapes and text boxes, legacy VML shapes, equations, charts, content controls, comments, tracked changes, properties, themes, settings, and embedded fonts *(our own `.docx`: 1,056 files)*
- [~] **first release** Save `.docx` so that every file opens without a repair message, checked on a large collection as JeffPub's `.pub` files were, and the layout matches the reference pages
- [x] **first release** Round trip keeps what JeffWrite can't edit: SmartArt, 3-D models, custom XML, document variables, the glossary document of building blocks, people and comment extensions, `mc:AlternateContent` choices, and the extension elements of newer Word versions (`w14`, `w15`, `w16`), written back as they were, and untouched parts kept byte for byte
- [x] **first release** Open and save templates (`.dotx`) with their styles and building blocks
- [ ] **1.0** Open and save macro-enabled files (`.docm`, `.dotm`), keeping the macro project untouched and never running it
- [ ] **1.0** Open encrypted files and save with a password
- [ ] **later** Open and save the Strict variant of Open XML, and Flat OPC (`.xml`) files

### Word 97 to 2003 `.doc` and `.dot` (binary)
- [~] **first release** Open `.doc` and `.dot`: text and every property of characters, paragraphs, sections, and tables; styles; lists; fields; footnotes and endnotes; headers and footers; bookmarks; pictures and drawings (Escher shapes and text boxes); embedded objects; comments; tracked changes; form fields; and fast-saved and complex files *(public: all 1,527 files; our: about a thousand of his 2,082; shared: the compound-file reader in libs/base/io and the drawing reader in libs/office)*
- [ ] **1.0** Save `.doc` so Word opens it, with Check Compatibility naming what is lost; if this stays out of 1.0, "Save As" offers a new `.docx` name instead, as JeffDeck does for `.ppt` *(see the open questions)* *(0.9.0: Save As on an opened `.doc` asks for a `.docx` name)*
- [ ] **1.0** Open encrypted `.doc` (RC4 and the older XOR scheme)
- [ ] **later** Open Word 6.0 and Word 95 `.doc`, and Word 2003 XML (`.xml`)

### Rich Text `.rtf`
- [ ] **first release** Open `.rtf`: character and paragraph formatting, styles, lists, tables, pictures (WMF, EMF, PNG, JPEG), fields, footnotes, headers and footers, sections, bookmarks, and all Unicode text and code pages *(about 3% of our old files are `.rtf`)*
- [ ] **1.0** Save `.rtf` that Word and other editors open with the same text, styles, lists, tables, and pictures
- [ ] **1.0** Paste and drop RTF from other programs through the same reader *(shared: JeffPub's RTF insert)*

### OpenDocument `.odt`, `.ott`
- [ ] **first release** Open `.odt` and `.ott` (ODF 1.1 to 1.3, including LibreOffice's and Apache OpenOffice's): styles (automatic and named, paragraph, character, list, table, page), master pages as sections, headers and footers, lists, tables, frames and images, footnotes and endnotes, fields (page number, count, date, document properties, cross-references), bookmarks, tables of contents and indexes, tracked changes, comments, equations (MathML), embedded charts, drawing shapes, and properties *(shared: JeffDeck's ODF reader for styles, shapes, and charts)*
- [ ] **first release** Save `.odt` and `.ott` as LibreOffice writes them, with every property explicit so LibreOffice, Apache OpenOffice, and Word open it as it looks here, checked on a collection *(shared: JeffDeck's ODF writer)*
- [ ] **later** Flat OpenDocument (`.fodt`)

### Other formats
- [x] **first release** PDF export (see Share and Export) *(shared: JeffPub)*
- [ ] **first release** Open and save plain text with the encoding chosen (UTF-8, UTF-16, Windows code pages, ISO 8859, and the East Asian ones) and the line ends chosen
- [ ] **1.0** Open an HTML page as a document, and save a document as HTML (single file and filtered) *(shared: JeffPub's web export)*
- [ ] **1.0** EPUB export (see Beyond Word) *(shared: JeffPub's EPUB writer)*
- [ ] **later** Open a PDF as editable text and pictures (Word's PDF reflow)
- [ ] **later** **(alt)** Open and save Markdown (`.md`), which Word cannot
- [ ] **later** Open WordPerfect (`.wpd`) and Microsoft Works (`.wps`) documents *(shared: librevenge, already in third_party)*
- [ ] **later** XPS: not offered, since the format has no use outside Windows

## Shared with other JeffOffice apps

The suite-wide map of shared parts, [shared.md](shared.md), says where each lives, which app builds it first, and who uses it; where this list disagrees with it, the map holds.

What JeffWrite builds on, and the pieces it should build so that JeffCrunch, JeffDeck, JeffMail, and JeffBase can use them.

### Already there
- [x] **first release** libs/text, the text engine: paragraph layout, tabs with leaders, columns, hyphenation, footnote layout, fields, and right-to-left; JeffWrite adds pages that grow from the text instead of frames that hold it
- [x] **first release** libs/page, the document of placed objects, with its editor, canvas, Find and Replace, proofing dialogs, options, and online pictures; JeffWrite's flowing document, libs/flow, sits beside it
- [x] **first release** libs/office: Open XML packages, themes, DrawingML shapes, text, and tables (read and written), charts, the Escher drawing reader for `.doc`, and the embedded-font readers
- [x] **first release** libs/draw and libs/base: shapes, fills, effects, text art, SVG, picture effects, colors, fonts, barcodes, zip and compound files, EMF and WMF, PDF, and the dictionaries
- [x] **first release** libs/shell: the window, ribbon, KeyTips, file pages, Help, translations, AutoRecover, settings, spelling, updates, and statistics

### To build once, for several apps
- [~] **first release** A flow-document model and its page layout (sections, pages growing from paragraphs and tables, floating objects, notes, and headers and footers), which JeffMail's composer and JeffBase's reports can use *(needed by: JeffWrite, JeffMail, JeffBase)* *(0.9.0: built in libs/flow and used by JeffWrite; JeffMail's composer and JeffBase's reports do not use it yet)*
- [ ] **first release** A field engine on the field model of libs/text: page and date fields, document properties, cross-references, merge fields, and formula, shared with JeffPub, JeffDeck (slide numbers), and JeffMail *(needed by: all)*
- [ ] **1.0** An equation editor and layout for Office Math and MathML, drawn in print and PDF *(needed by: JeffWrite, JeffDeck, JeffCrunch)*
- [ ] **1.0** Chart editing on top of libs/office chart: the data grid, the element menus, types, and styles *(needed by: JeffWrite, JeffDeck, JeffCrunch)*
- [ ] **1.0** A merge-source library (CSV, tab-delimited, `.xlsx`, `.ods`, vCard, and later JeffBase tables and JeffCrunch ranges) with filter, sort, duplicate, and address checks, moved out of libs/page/core *(needed by: JeffPub, JeffWrite, JeffMail, JeffBase)*
- [~] **1.0** A table engine with merged cells, styles, nested tables, and formulas that run on a calculation engine shared with JeffCrunch *(needed by: JeffWrite, JeffMail, JeffCrunch)* *(0.9.0: tables with merged cells, nested tables, and rows that split across pages are laid out and edited in JeffWrite; table styles, formulas, and the shared calculation engine are not)*
- [ ] **1.0** A review model for tracked changes, comments with replies, and comparison, so JeffDeck's comments and JeffCrunch's notes can follow it *(needed by: JeffWrite, JeffDeck, JeffCrunch)*
- [ ] **first release** A grammar and style check behind the spelling service: Harper, linked as a native library *(needed by: JeffWrite, JeffMail)*
- [ ] **1.0** Building blocks and templates in one format and gallery (Quick Parts, page parts, slide layouts) *(needed by: all)*
- [ ] **later** An ink layer (pens, highlighter, eraser, ink to shape) built on the slide show's pen *(needed by: JeffWrite, JeffDeck)*

## Left out on purpose

Not planned, because they only work with an online service or a product JeffOffice doesn't have: Office add-ins and the Store, Designer and Design Ideas, Insights and Researcher, the LinkedIn Resume Assistant, OneDrive and SharePoint (check-in, workflows, co-authoring locks, information rights management), Microsoft Translator, online dictation, sensitivity labels, blog posting, fax, and XPS.

## Decided

- Old and open formats (October 11, 2026): the first release opens `.doc`, `.rtf`, and `.odt`; saving in them follows once `.docx` is solid, as JeffDeck did with `.ppt` and `.odp`. Until then, Save on such a file asks for a `.docx` name.
- Grammar checking (October 11, 2026): Harper, English in the first release.
- Embedded fonts: follow JeffDeck's rule. Saving embeds, by default, the fonts JeffOffice brings that other computers may lack; fonts drawn in place of common ones are saved under the common name.
- How text flows across pages (a new library or an extension of libs/page) is an engineering choice, made when building starts.

## Open questions

1. Should Compare Documents (and Combine) move up to the first release, since legal publishing runs on redlines?
2. Should the Table of Authorities and Index marking move up to the first release for briefs and books, though the counts don't measure them?
3. Macros: keep them untouched and never run them, or give JeffWrite an open scripting language later (and which one)?
4. Dictation: is an offline speech model (size and license to check) worth shipping, or is it dropped?
5. Citations: support Word's own source list only, or CSL styles and BibTeX or Zotero import as well?
6. Should Protected View (read-only until you choose to edit) exist, or is a plain read-only open enough?
7. Which built-in templates come first: letter, report, brief, manuscript, newsletter, or others?
8. East Asian typography (vertical text, kinsoku, ruby): stay at later, or does anyone who will use JeffWrite need it for 1.0?
