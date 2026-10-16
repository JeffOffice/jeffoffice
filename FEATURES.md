# JeffPub feature list

JeffPub is the desktop publishing app of JeffOffice, modeled on Publisher 2021: it opens and saves Publisher's `.pub` files, and it has its own open format, `.jpub`. It is in beta (version 0.8.0). Unlike the lists for JeffWrite, JeffCrunch, JeffMail, and JeffBase, this one describes a program that exists, so a box is checked only when the feature works in JeffPub, and every status was checked on October 16, 2026 against the code, the tests, the ribbon file (`resources/ribbon.json`), and the built-in Help. It is laid out the way the original is (File, then each ribbon tab, the tabs that appear only while something is selected, then what sits under the ribbon), and it was compared control by control with Microsoft's published list of Publisher's ribbon controls (Office 2019), so that the work can be planned and tracked and so that it can settle one question: when does JeffPub stop being a beta? Last reviewed October 16, 2026.

Legend: `[x]` works in JeffPub today · `[~]` partly there (the note says exactly what is missing, or what could not be confirmed) · `[ ]` not yet · **(alt)** the usual way to do this depends on a proprietary online service, so JeffPub provides an open replacement with the same purpose.

Every item carries a tier, measured against dropping the Beta label:

- **1.0**: needed before JeffPub leaves beta. A finished item is usually **1.0** too, since it counts toward the bar. The label comes off when every **1.0** item below is `[x]`, or has been moved to **later** by a decision written under Decided.
- **later**: used by few publications, needs a piece the suite doesn't have yet, or depends on something outside the program.

There is no "first release" tier here, because JeffPub already shipped. Version 1.0 is ready when Publisher opens every file JeffPub saves (met so far for all 284 publications of the owner's collection) and JeffPub's pages match Publisher's own pictures of them closely enough (the scores are under File, in Open, Save, and Close, and the bar is an open question).

## How the tiers were chosen

There are no usage counts for Publisher files yet. The owner's collection is 284 real publications (375 pages), and the fidelity scores are measured on it and on 78 template pages. The only public `.pub` files are 26 small test files from other open-source projects (in `tests/data/pub`); they exercise the reader and say nothing about how often a feature is used. So the tiers rest on what the collection shows through those checks, on what a person moving from Publisher meets in daily work, and on what a replacement must do. Four cautions:

- The collection is one person's, so a feature the owner never used reads as "not needed" here without being known to be rare. A count of features across the collection's `.pub` files would turn these judgments into figures (see the open questions).
- The ribbon below was checked against Microsoft's published list of Publisher's controls (about 1,350 rows: the tabs and groups, the shortcut menus, the controls that are not on the ribbon, and the File page). Every tab and group was read; each control with no match was added as an item or put under Left out on purpose, except a few minor ones that are not listed (a printed sample of the color scheme, Picture Display, Ruled Lines, snapping to ruler marks, and the More Windows list).
- Where the code, the tests, and Help could not show that a feature works as written, the item is `[~]` and says what could not be confirmed.
- Publisher is installed on the Windows test machine, so every fidelity number compares JeffPub with the original's own output, never with a guess.

## Beyond Publisher

Features JeffPub has that Publisher never did, then the ones planned before JeffPub leaves beta.

- [x] **1.0** Runs on Linux as well as Windows; Publisher only ever ran on Windows
- [x] **1.0** Free and open source (GPL-3.0): no license fee, subscription, or account
- [x] **1.0** Installs without administrator rights, or runs with no installation at all (the Windows `.zip` and the Linux AppImage)
- [x] **1.0** An open file format: a `.jpub` publication is a ZIP of readable JSON and the original pictures, [fully documented](docs/jpub-format.md), alongside opening and saving `.pub`
- [x] **1.0** Barcodes: book ISBNs with or without a price add-on (hyphenated as the ISBN agency assigns them, for every country), EAN-13, UPC-A and EAN-8 with add-ons, and Code 128 and Code 39, drawn as vector bars and editable after placing
- [x] **1.0** WebP and SVG pictures
- [x] **1.0** **(alt)** PDF pages as pictures: Insert Picture takes a PDF (choose the page from pictures of them all); it shows as PDFium draws it, and prints and exports to PDF and SVG as sharp vectors, its text as letter outlines *(each page's vector drawing is checked against PDFium's own, and wherever they differ (hidden layers, blending, a letter whose outline can't be found) that part comes from a 300 dpi picture instead; saved to `.pub` as a picture, since Publisher takes no PDFs)*
- [~] **1.0** macOS app (Apple silicon and Intel) *(test builds; not yet signed, so macOS asks before opening it the first time; see the open questions)*
- [x] **1.0** PDF/X export for commercial printers, with a print check before export *(PDF/X-1a:2001 for U.S. web coated (SWOP) or European coated (FOGRA39) printing, and PDF/X-4 (transparency kept) for premium coated paper (FOGRA51, with ECI's PSO Coated v3 profile, downloaded from the European Color Initiative the first time, after asking, since its terms don't allow passing it on) or with your printer's own color profile; checked for low-resolution pictures and missing fonts first; black overprinting as the publication's Commercial Print settings say: black text below a size, lines, fills, from a darkness you choose (Publisher's defaults: text below 24 pt and lines))*
- [x] **1.0** Automatic table of contents from the Heading 1-3 styles, with page numbers taken from the layout and dot leaders, updated with one click (Insert > References)
- [x] **1.0** Footnotes and endnotes: numbered references (Insert > References), footnotes at the bottom of the column their number lands in under a short rule (a line moves on with its note when both don't fit; a note too tall for a whole column is split: the line with the number stays with the first part, and the rest goes on at the bottom of the next column or linked text box, above that column's own notes, under a rule as wide as the column, through as many columns as it needs), endnotes under "Notes" after the story *(note text is edited in a dialog)*
- [x] **1.0** Insert > Icons: 1,539 vector icons from the open-source Lucide set, searched by name or keyword; each becomes an editable shape in the color scheme's main color (Line Color recolors it, Edit Points reshapes it) whose line weight grows and shrinks with it, sharp in print and PDF, and saved to `.pub` as a drawn shape *(shared: the picker is in libs/page, and JeffDeck uses it too)*
- [x] **1.0** SVG: pages saved as SVG drawings (Save as Picture), with letters as outlines so they look the same without the fonts; SVG pictures print and export to PDF as sharp vectors, cut by the drawing's clip paths on screen, in print, in PDFs, and in saved SVG pages; and Convert to Shapes (Picture Format, or right-click) turns an SVG picture into editable shapes, cut the same way *(text and pictures inside the drawing are left out, with a note saying so; a clip path made of text, or inside a symbol, pattern, or marker, is ignored, and in a very large drawing with very many clipped parts the last ones show uncut)*
- [x] **1.0** EPUB export for e-book readers and stores (File > Export > Save as E-book): text that reflows to any screen, chapters at each Heading 1, contents from Heading 1-3, pictures and tables in place, footnotes that pop up and endnotes at the back, the first page as the cover; or page for page (fixed layout): each page exactly as designed, as picture books, magazines, and comics are sold, with its words underneath for search and screen readers, the contents at the pages the headings are on, and facing pages for booklets
- [x] **1.0** A published specification of the `.jpub` format ([docs/jpub-format.md](docs/jpub-format.md)), kept true by a test that opens its example
- [x] **1.0** Merge to PDF: a mail merge saves one PDF with every recipient's pages (Mailings > Finish), a kind of output Publisher has no button for
- [x] **1.0** One set of choices for the suite: the theme, display language, user name and initials, update choices, and statistics choice are kept once, so changing one in any JeffOffice app changes it in all of them

## File (backstage)

### Info
- [x] **1.0** Business Information: create, edit, and switch between named sets (name, tagline, contact person, title, address, phone/fax/email, logo)
- [x] **1.0** Commercial Print Information: color model (RGB, process CMYK, spot colors, process + spot) and embedded-font management *(spot colors are named inks: they print on their own separation plates and are Separation colors in PDFs; a process-color publication makes a CMYK PDF, with colors entered as CMYK kept exactly)*
- [x] **1.0** Run Design Checker from Info
- [x] **1.0** Publication properties (title, author, subject, keywords, comments)

### New
- [x] **1.0** Template gallery with live previews and categories: Award Certificates, Banners, Brochures, Business Cards, Business Forms, Calendars, Catalogs, Email, Envelopes, Flyers, Gift Certificates, Greeting Cards, Invitation Cards, Labels, Letterhead, Menus, Newsletters, Paper Folding Projects, Postcards, Programs, Resumes, Signs, With Compliments Cards *(48 designs, at least two in every category)*
- [x] **1.0** Customize a template before creating it: color scheme, font scheme, business information, and template options (for example "include logo" or "mailing address") *(the New page shows the options each template has: "Include logo" on 15 templates, which places the business logo and makes room for it, and "Include mailing address" on postcards)*
- [x] **1.0** Blank page sizes, More Blank Page Sizes, Create New Page Size
- [x] **1.0** Save as a template and reuse your own templates ("My Templates")

### Open / Save / Save As / Close
- [x] **1.0** Open JeffPub publications (`.jpub`) and recent publications, with pinning
- [~] **1.0** Open `.pub` files (every version since 1998), including linked text boxes, pictures, shapes, and tables *(partial: text, pictures, shapes, and tables open; Publisher's shapes come back as JeffPub's own where they match, and picture crops, transparency, brightness, contrast, and recoloring are kept; checked against PDFs of the same files, most covers and pages now match closely; fonts stored inside the file and the page numbering of any section after the first are not brought in, and opening a file says so; see File formats)*
- [x] **1.0** Save as `.pub` *(each part checked by opening the saved file in another .pub program. Saved: pages and master pages, text boxes and linked text boxes, character formatting (font, size, bold, italic, underline, strikethrough, small and all caps, superscript and subscript, color, letter spacing, and tracking), paragraphs (alignment, indents, spacing, line spacing, tab stops with leaders, bullets and numbering with their fonts, right-to-left direction), every shape (as Publisher's own where it matches, otherwise an exact outline), lines with dashes and arrowheads, text in shapes, rotation and flips, pictures with cropping, transparency, brightness, contrast, recoloring, and a clear color, pictures cut to a shape, tables with merged cells, fills (solid, gradient, picture, texture, and pattern) with transparency, borders with transparency, shadows, Text Art, named paragraph styles (each paragraph keeps its style), process (CMYK) colors. Saving reports objects it leaves out)*
- [~] **1.0** Checked against Publisher itself: Publisher opened and exported all 284 publications of a real collection, and opened every one of them after JeffPub saved it as `.pub`. Against Publisher's own pictures of the 375 pages of those publications, JeffPub's pages differ by a median blurred-pixel difference of 0.96 and a mean of 1.74, with 73% under 2 (0 is identical; measured with pages scaled to 40 dots per inch, the scale used to compare every JeffOffice app with its original, so these figures differ a little from earlier ones taken at another scale); on 78 template pages the median is 0.62 and 79% are under 2; and 95% of their text boxes break into the same words *(the rest differ by a word or two at a box's end; the text-box figure was measured earlier and not repeated for this review; the bar for leaving beta is an open question)*
- [x] **1.0** Booklets, folded cards, and envelopes in `.pub`: saved as Publisher saves them (a booklet's or folded card's spreads and two-page masters, and each layout type), and Publisher's own open with all their pages *(Publisher read JeffPub's booklets as booklets, with the same spreads and masters, and pictured them the same; it kept JeffPub's folded cards and envelopes as folded cards and envelopes when saving them again)*
- [x] **1.0** Objects in text (Publisher's "inline" pictures, shapes, and text boxes): Wrap Text > In Line with Text moves an object over a text box into its text, and choosing another wrap with the object selected in the text moves it back out; opened from and saved to `.pub`, and kept in `.jpub`; each sits on its line's baseline inside its wrap distances and a tall one makes its line taller (a line with nothing but objects is exactly as tall as they are, with no text above or below), as in Publisher *(measured against Publisher to the pixel; Publisher showed a JeffPub-saved copy of its sample exactly as its own file)*
- [x] **1.0** Fields and hyperlinks in `.pub` text: page numbers (this page's, and the next or previous linked box's), dates and times in Publisher's 17 formats (they update, as in Publisher), and hyperlinks are read and saved as Publisher's own *(Publisher counted the same fields and links in JeffPub's copies of its samples as in its own files, and pictured them the same)*
- [x] **1.0** Anonymous usage statistics, with a choice in Windows setup and when JeffPub first starts, and a switch in File > Options: once a day (and on the day of an update), the version, operating system, language, and how often each command is used; never files or their contents; if JeffPub closes unexpectedly, a crash report is sent the next time it starts, under the same choice, and says where in the program it stopped, never what was in the publication (see README, Privacy)
- [x] **1.0** Save, Save As, AutoRecover (copies of unsaved work offered again after a crash, and cleaned up once saved), and backup on save
- [x] **1.0** Close and prompt to save changes

### Print
- [x] **1.0** Print with printer selection, copies, page ranges (all, current, custom), and printer properties
- [x] **1.0** Layouts: one page per sheet, multiple pages per sheet, multiple copies per sheet, booklet (side-fold and top-fold), and tiled printing for banners and posters
- [x] **1.0** Paper size, one-sided or two-sided (flip on long or short edge), color, grayscale, or composite
- [x] **1.0** Print preview with ruler, page navigation, front/back view, and "show numbers"
- [x] **1.0** Crop marks, bleed marks, registration marks, density bars, color bars, job information, and "allow bleeds"
- [x] **1.0** Separations (CMYK and spot plates): a plate per process ink and per spot color; tints of a spot color print as lighter ink on its plate, and spot colors are knocked out of the process plates
- [x] **1.0** Mail merge and catalog merge printing

### Share and Export
- [x] **1.0** Create PDF (standard, minimum size, high quality, commercial press; page ranges; document properties; PDF/A; PDF/X-1a; open the PDF after saving it; a booklet as its printed sheets, two pages to a side in folding order, as Publisher makes it, or as single pages)
- [x] **1.0** Save pages as pictures: PNG, JPEG, GIF, TIFF, and BMP at a chosen resolution
- [x] **1.0** Web page export (single-file HTML): each page is a picture, so its words cannot be selected
- [x] **1.0** Pack and Go: Save for a Commercial Printer, Save for Another Computer (bundle fonts and linked pictures)
- [x] **1.0** Save for a Photo Printer
- [x] **1.0** **(alt)** Email: send the current page as a picture, or the publication as an attachment. JeffPub creates a ready-to-send `.eml`, PDF, or HTML file instead of sending through a mail program.
- [ ] **1.0** Links in a PDF: a hyperlink in the publication is a link you can click in the PDF *(no link is written today; the PDF/X path also removes any annotations)*
- [ ] **later** Bookmarks in a PDF, from page names or Heading 1-3 *(the e-book export already builds its contents from the headings)*
- [ ] **later** A tagged PDF for screen readers (headings, reading order, and alt text), and a PDF with a password

### Options
- [x] **1.0** General (user name and initials, light or dark interface, display language, usage statistics, update checks), Proofing (spelling as you type, ignoring words in capitals or with numbers, AutoCorrect of common misspellings, symbols, sentence capitals, and two initial capitals; smart quotes; AutoFormat as you type), Save (AutoRecover interval, backup copies), and Advanced (auto-select entire word, drag-and-drop text, hyphenation in new text boxes, measurement units, nudge amount, recent publication count)
- [x] **1.0** Updates: Check for Updates now or each time JeffPub starts, with Update Now, Later, or Skip This Version, and a Get beta versions choice for early builds (two channels, Stable and Beta); the update choices are shared with the other JeffOffice apps
- [ ] **later** Customize the ribbon and the Quick Access Toolbar *(the toolbar holds Save, Undo, Redo, and Print, and cannot be changed)*

## Home tab
- [x] **1.0** Clipboard: Paste (with its source formatting), Keep Text Only, Paste Special, Cut, Copy, Format Painter (single use and locked)
- [x] **1.0** Font: font face with scheme fonts first, size, Grow Font, Shrink Font, Clear All Formatting, Bold, Italic, Underline (styles), Strikethrough, Subscript, Superscript, Change Case, Character Spacing, Font Color (scheme colors, tints, more colors, eyedropper), and the Font dialog
- [x] **1.0** Paragraph: Bullets (gallery and custom bullet character), Numbering (formats and starting number), Decrease and Increase Indent, Special Characters (show ¶), Align Left, Center, Right, Justify, and Distribute, Line Spacing, Paragraph Spacing, and the Paragraph dialog (indents, spacing, line breaks, tabs, baseline alignment)
- [x] **1.0** Styles: gallery, New Style, Modify Style, Import Styles (from a publication), and Styles by example
- [x] **1.0** Objects: Draw Text Box, Table, Pictures, Shapes
- [x] **1.0** Arrange: Wrap Text, Bring Forward or to Front, Send Backward or to Back, Group, Ungroup, Regroup, Align and Distribute (relative to margin guides or objects), Rotate and Flip
- [x] **1.0** Editing: Find, Replace (match case, whole word, search direction), Select (Select All Text in Text Box, Select All, Select Objects, Select Object by Type)
- [x] **1.0** Go to Page (F5), a page number typed into a window *(a key and a command, not a ribbon button)*
- [ ] **later** Show Clipboard: a pane that lists what was copied lately, to paste any of it
- [ ] **later** East Asian text layout: phonetic guides above characters, enclosed characters, horizontal text inside a vertical line, East Asian justification, and editing marks

## Insert tab
- [x] **1.0** Pages: Insert Blank Page, Insert Duplicate Page, Insert Page dialog (count, before/after, blank, duplicate, or one text box per page), Catalog Pages (a catalog area repeated for each item of a product list, in rows and columns, with preview and merging to a new publication, PDF, or printer)
- [x] **1.0** Tables: grid picker and Insert Table dialog
- [x] **1.0** Illustrations: Pictures (from file), Online Pictures **(alt: Openverse and Wikimedia Commons search with license info)**, Shapes (full gallery), Icons, Picture Placeholder, Barcode (book ISBN from ISBN-10 or -13, hyphenated as the International ISBN Agency assigns ranges for every country, with a price add-on in US, Canadian, Australian, or New Zealand dollars or pounds, or no suggested price; EAN-13, UPC-A, and EAN-8 with 2- or 5-digit add-ons; Code 128; Code 39; magnification and bar height; vector bars and digits on a white quiet zone)
- [x] **1.0** Building Blocks: Page Parts (headings, pull quotes, sidebars, stories, tables of contents), Calendars (by month and year, with options), Borders & Accents, Advertisements, and saving your own (Save as Building Block), which Page Parts lists under My Building Blocks
- [x] **1.0** Text: Draw Text Box, Business Information fields, Text Art, Insert File (text, RTF, HTML, and `.docx` documents into a text box), Symbol (recently used symbols, kept between sessions, in the Symbol drop-down and the dialog, and the full character map), Date & Time (formats, update automatically), Object **(alt: places a PDF page, an SVG drawing, or a picture; documents from other programs can't be embedded)**
- [x] **1.0** Links: Hyperlink (web, email, place in this document, new document), Bookmark
- [x] **1.0** Header & Footer: Header, Footer (open the master page), Page Number (position, format, start number from 1 to 1000, as in Publisher) *(the first page number and the style are saved in `.pub` files and read back; a publication has one numbering, so a later section of a Publisher file that numbers its pages on its own isn't kept, and opening one says so)*
- [ ] **later** Draw Vertical Text Box: a text box whose text is turned from the start *(today a text box is drawn, then turned with Text Direction on the Text Box tab)*
- [ ] **later** Sections: start page numbering again at a chosen page (Insert Section, Merge with Previous Section, and a page number format for each section), with the sections shown in the Pages list *(a Publisher file with several sections opens with the first section's numbering only; see the open questions)*
- [ ] **later** Embed a live document from another program (a spreadsheet or a chart) and edit it in place *(needs JeffCrunch; see Shared)*

## Page Design tab
- [x] **1.0** Template: Change Template (the gallery in a mode of its own, with the color scheme and font scheme starting at the publication's own; applies another design to this publication as one undo step, or makes a new publication with your text and graphics and leaves this one alone; each story goes into the new design's box with the same role, your pictures replace its placeholder pictures in reading order, and the page size and pages follow the design) and Extra Content *(a pane for the stories and pictures with no place in the new design: place on the page by button, double-click, or drag, or discard; it is saved with the publication; tables, shapes, lines, Text Art, and groups stay behind only when they are the old design's own and unchanged, and any other goes to Extra Content whole; no pages are added for leftovers; items on master pages and page backgrounds still stay with the old design)*
- [x] **1.0** Page Setup: Margins (margin guide presets, and Custom Margins opening the Margin Guides settings), Orientation, Size (presets, the user's saved custom sizes, Create New Page Size, and editing or deleting saved sizes), and the Page Setup dialog (layout type: one page per sheet, booklet, envelope, folded card, multiple pages per sheet, labels; for several pages per sheet, the sheet's side and top margins and the gaps between pages, kept apart from the margin guides, with how many fit). A new page size, from the gallery or the dialog, scales nothing: every object moves by half the change so it keeps its distance from the page center, ruler guides keep their share of the page, objects left wholly off a smaller page go to the scratch area, and Undo restores it all *(as Publisher does; checked in Publisher)*
- [x] **1.0** Printing several pages per sheet (business cards, labels) places them by the publication's own side and top margins and gaps
- [x] **1.0** Guides: built-in guide layouts, Grid and Baseline Guides dialog (columns, rows, spacing, center guide, baseline spacing and offset), Add Horizontal and Vertical Ruler Guides (each new one beside the last), Ruler Guides dialog (add, move, and remove guides by position, or add a series at even spacing), Clear All Ruler Guides
- [x] **1.0** Layout: Align To Guides, Align To Objects
- [x] **1.0** Pages: Delete, Move, Rename, Master Pages (apply or none)
- [x] **1.0** Schemes: color scheme gallery (90+ schemes), Create New Color Scheme, font schemes, Create New Font Scheme, Update Font Scheme
- [x] **1.0** Page Background: solid and gradient presets and More Backgrounds (gradient, texture, pattern, picture, tint), Apply Image as Background
- [ ] **later** Send to Master Page: move the selected objects from a page onto its master page, and back

## Mailings tab
- [x] **1.0** Mail Merge and Email Merge
- [x] **1.0** Step-by-Step Mail Merge Wizard (task pane)
- [x] **1.0** Select Recipients: Type a New List, Use an Existing List (CSV, tab-delimited, and `.xlsx` spreadsheets, and vCard contacts), Select from Contacts **(alt: any address book's contacts saved as vCard `.vcf` files)**
- [x] **1.0** Edit Recipient List: sort, filter, find duplicates, validate addresses, include or exclude records
- [x] **1.0** Address Block, Greeting Line, Insert Merge Field, Picture Field
- [x] **1.0** Preview Results with record navigation and Find Recipient
- [x] **1.0** Finish & Merge: Merge to Printer, Merge to New Publication, Merge to Email **(alt: one `.eml` per recipient)**, Export recipient list
- [x] **1.0** Catalog merge (repeating product layouts from a data source)
- [ ] **later** Edit Address Block and Edit Greeting Line: name order, company and country lines, and the text to use when a name is empty *(today both are fixed)*
- [ ] **later** Format a merge field's date, time, number, or currency (1234.5 shown as $1,234.50) *(the value comes through as the list holds it)*
- [ ] **later** Merge to an existing publication: add the merged pages to a publication you pick
- [ ] **later** Personalized hyperlink: a link whose address includes a field from the list
- [ ] **later** Catalog merge: choose across or down for the order items fill the cells, pick a catalog page layout, and use a Catalog Merge tab of its own *(a pane does it today: Rows and Columns, product fields, picture fields, and preview)*

## Review tab
- [x] **1.0** Spelling (check as you type, dialog with suggestions, ignore, add to dictionary; check as you type is switched on and off from the ribbon)
- [x] **1.0** Thesaurus
- [x] **1.0** Research **(alt: an offline thesaurus: synonyms and related words, without going online)**
- [x] **1.0** Translate **(alt: opens the selected text in LibreTranslate, an open-source translation service)**
- [x] **1.0** Language: set the proofing language for selected text; spelling and automatic hyphenation follow it (English for the US, UK, Canada, and Australia; Spanish for Spain and Mexico; French; German; Italian; Dutch; Portuguese for Brazil and Portugal)
- [x] **1.0** Word Count: pages, words, characters, paragraphs, text boxes, and pictures
- [x] **1.0** Check group: Run Design Checker (also under File > Properties) and Hyphenation (automatic hyphenation, Insert Optional Hyphen, Add Optional Hyphens) beside Word Count
- [ ] **later** Accessibility checks beside the Design Checker: missing alt text, reading order, and low contrast *(needs the suite's accessibility checker; see Shared)*
- [ ] **later** Chinese conversion (Simplified and Traditional), Hangul and Hanja conversion, and an IME dictionary update

## View tab
- [x] **1.0** Views: Normal, Master Page
- [x] **1.0** Layout: Single Page, Two-Page Spread
- [x] **1.0** Show: Boundaries, Guides, Fields, Rulers, Page Navigation, Scratch Area, Baselines, Graphics Manager
- [x] **1.0** Zoom: zoom box, 100%, Whole Page, Page Width, Selected Objects, and Ctrl+mouse-wheel
- [x] **1.0** Window: New Window, Arrange All, Cascade, Switch Windows
- [x] **1.0** Collapse the Ribbon with Ctrl+F1, as well as with its chevron or a double-click on a tab

## Help tab
- [x] **1.0** Help (F1, or the "?" beside the ribbon's collapse button) **(alt: built in and working offline, instead of web pages)**: a Help pane of 51 topics in plain language, with a contents page, links between topics, Back and Forward, and a search that finds topics and also commands, which it runs; F1 opens the topic for what you're doing (the ribbon tab, the task pane, the File page, the dialog, or the selected object), over a dialog in a window of its own; the same topics are published to the project's GitHub wiki with every push to main (`JeffPub --export-help-wiki`, see docs/help-wiki.md)
- [x] **1.0** Feedback **(alt: a JeffPub GitHub page for reporting a problem or suggesting an idea, with the version filled in)**
- [x] **1.0** Keyboard Shortcuts (a table of every command's keys, made from the commands themselves) and What's New (this version's release notes)

## Master Page tab (while editing master pages)
- [x] **1.0** Add Master Page, Duplicate, Rename, Delete, Two-Page Master, Apply To, Close Master Page
- [x] **1.0** Header and footer editing with Show Header/Footer, Insert Page Number, Insert Date, and Insert Time (Alt+Shift+P, D, and T); the Mailings tab hides while a master page is open, as there
- [x] **1.0** The Master Page tab comes first, right after File, and is the one showing when master view opens; the other contextual tabs keep their places after Help

## Drawing Tools / Shape Format tab
- [x] **1.0** Insert Shapes gallery, Edit Shape (Change Shape and Edit Points), Draw Text Box
- [x] **1.0** Curve, Freeform: Shape, and Freeform: Scribble (Shapes > Lines): click points for a smooth curve or straight-sided freeform (drag to draw by hand), double-click or Enter to end, click the first point to close it into a filled shape; a scribble is one stroke. Each becomes a shape whose points can be edited
- [x] **1.0** Connectors (Shapes > Lines): elbow and curved connectors, plain or with one or two arrowheads; any line's ends attach to the middle of a side of a shape, text box, picture, or table (the sites show as you draw) and follow it when it moves, resizes, or turns; a yellow handle moves an elbow's bend; dragging a connector away on its own detaches it. Opened from and saved to `.pub` attached, as Publisher's own connectors *(checked by moving a shape in Publisher: the connectors followed it)*
- [x] **1.0** Shape Styles gallery, Shape Fill (colors, picture, gradient, texture, pattern, no fill), Shape Outline (color, weight, dashes, arrows, pattern), Change Shape
- [x] **1.0** Shape Effects: Shadow, Reflection, Glow, Soft Edges, Bevel, 3-D Rotation
- [x] **1.0** Shadow Options (color, transparency, blur, distance, and angle) under Shape Effects > Shadow, with 3-D Rotation and Bevel beside it *(the classic Shadow Effects and 3-D Effects menus are on the Text Art tab only, and no nudge buttons were found)*
- [x] **1.0** Arrange group and Size group (height and width)
- [x] **1.0** Add text inside shapes
- [x] **1.0** Shape adjustment handles (yellow diamonds)
- [ ] **later** Reroute Connectors: choose the shortest path for connectors attached to a shape

## Text Box Tools Format tab
- [x] **1.0** Text: Text Fit (Best Fit, Shrink Text On Overflow, Grow Text Box to Fit, Do Not Autofit), Text Direction (vertical), Hyphenation (automatic and manual)
- [x] **1.0** Alignment: nine-way alignment, Columns (count and spacing), Margins (presets and custom)
- [x] **1.0** Linking: Create Link (pitcher cursor), Break, Previous, Next, overflow indicator, autoflow
- [x] **1.0** Font group mirrored from Home
- [~] **1.0** Effects: Shadow, Outline, Emboss, Engrave, Text Fill (solid and gradient), Text Outline, and Text Glow in a color you pick *(reflection and bevel on letters are not on the ribbon, and Help lists only these seven)*
- [x] **1.0** Typography: Drop Cap (gallery and custom: lines, size, font, color), Number Style (lining, old-style, proportional, tabular), Ligatures, Stylistic Sets, Stylistic Alternates, Swash, Small Caps, True Small Caps *(True Small Caps uses the font's own small capitals when every letter of a paragraph asks for them; where only some runs do, those runs get small capitals made of capitals, because Qt takes a font feature for a whole paragraph)*
- [x] **1.0** Arrange and Size
- [ ] **later** Reflection and Bevel for letters (the Text Effects menu), as shapes already have
- [ ] **later** A Text Styles gallery: ready-made combinations of fill, outline, and effects for letters

## Picture Tools Format tab
- [x] **1.0** Insert: Change Picture, Remove Picture, Swap (between picture frames), Arrange Thumbnails
- [x] **1.0** Adjust: Corrections (brightness and contrast presets), Recolor (presets, more variations, set transparent color), Compress Pictures, Reset Picture
- [x] **1.0** Picture Styles gallery, Picture Border, Picture Effects, Caption gallery, Picture Shape (any closed shape)
- [x] **1.0** Crop (handles, drag picture inside frame), Crop to Shape, Fit, Fill, Clear Crop, Pan picture
- [x] **1.0** Arrange and Size
- [x] **1.0** Picture placeholders with click-to-insert
- [x] **1.0** Scratch-area picture tray behavior (pictures parked on the scratch area): inserting several pictures at once puts them beside the page as thumbnails, Arrange Thumbnails tidies them, and Swap trades two pictures
- [x] **1.0** Graphics Manager pane (every picture with its page, format, size, original file, status, and resolution; go to it, save a copy, replace it) and pictures linked to their files *(Insert Picture offers Insert, Link to File, and Insert and Link. A link keeps the file's full and relative paths, a SHA-1 of its bytes (Modified means other contents, not another date), and a preview of at most 512 pixels, the relative path winning when the publication and its pictures are moved together; the page draws from the file, print, PDF, and e-book use it in full, and a missing file leaves the preview. Only a regular file of up to 256 MB is read, never a pipe or a device. When a publication opens, a link is followed only if its file is in the publication's own folder or below it (`..` and symbolic links resolved); any other, including network and device paths, is never touched, shows its stored copy or preview as Not updated, and is followed only by Update Link or Change Link on that picture, for that session, with the saved path unchanged; the mail merge's picture files follow the same rule, until the recipient list is chosen again in the session, which allows the pictures in its folder and below for that session only; a recovered AutoRecover copy is read as if in its original file's folder; jpubtool and the command line never follow outside links. Insert and Link keeps the whole picture and refreshes it from a changed file when the publication opens. The pane shows Embedded, Linked, Missing, Modified, or Not updated with Update Link, Change Link, and Embed Picture; the Design Checker reports "Picture is linked, not embedded", "Linked picture is missing", and "Linked picture shows only its preview until Update Link". Saving as .pub, Pack and Go, and e-mail store each picture whole, from what the publication holds. Saved in `.jpub`; a `.pub` file opens with embedded pictures, since the .pub reader reports no link paths. A file changed on disk while the publication is open shows as Modified (by size and date) but is read again only by Update Link)*

## Table Tools Design and Layout tabs
- [x] **1.0** Table Formats gallery, Fill, Borders (style, weight, color, which borders), Diagonals *(.pub files keep a diagonal's line as Publisher's diagonal border; see Merge Cells, Split Cells, Diagonals)*
- [x] **1.0** Insert rows and columns above, below, left, and right; Delete rows, columns, and table; Select cell, row, column, and table, or several cells at once (drag across them, or Shift+click; a merged cell is always whole) so that fills, borders, diagonals, alignment, margins, text formatting, Merge Cells, and Delete Rows and Columns act on all of them in one undo step; View Gridlines
- [x] **1.0** Merge Cells, Split Cells, Diagonals *(a cell divided by a diagonal has separate text in each half, as in Publisher: the text wraps inside its triangle, a click or Tab goes to the half it reaches, and No Division joins the second half's text to the first's; .pub files keep both halves' text, the direction, and the line along it, or none, as Publisher prints a line only for a cell with a diagonal border. Middle and bottom alignment of a long text in a half only approximate Publisher's)*
- [x] **1.0** Cell alignment (nine-way), Cell Margins, Text Direction, and Hyphenation for table cells *(.pub files keep each cell's vertical alignment, Text Direction, and margins, and one Hyphenation setting (on or off, and the zone) for the whole table, as Publisher does: a table's text is one story, so the Hyphenation command sets every cell)*
- [x] **1.0** Size: Height and Width boxes for the whole table (typed in your units; the columns or rows scale in proportion), Grow to Fit Text as a check box, Format Table (Colors and Lines, Size with Lock aspect ratio, Layout, Cell Properties: vertical alignment, margins, and turned text for the selected cells; from the Alignment group's launcher and the right-click menu), column widths and row heights by dragging, and Distribute Rows and Columns *(Lock aspect ratio is saved in JeffPub files; .pub files don't carry it yet)*
- [x] **1.0** Tab moves between cells, and Tab in the last cell adds a row

## Text Art Tools Format tab
- [x] **1.0** Edit Text dialog (text, font, size, bold, italic)
- [x] **1.0** Spacing, Even Height, Vertical Text, Align Text
- [x] **1.0** Text Art Styles gallery (30 classic plus modern styles), Shape Fill, Shape Outline, Change Shape (Text Art transforms: arch, circle, button, wave, slant, inflate, deflate, fade, triangle, chevron, cascade, can, and others)
- [x] **1.0** Shadow Effects (presets and Shadow Options) and 3-D Effects (rotations and bevels) *(menus of presets; no nudge buttons were found)*

## Object behavior and canvas
- [x] **1.0** Frames: text box, picture, shape, line, arrow, table, Text Art, group, embedded object
- [x] **1.0** Select (click, Shift or Ctrl-click, marquee, Tab cycling, select through groups)
- [x] **1.0** Move, resize (aspect lock with Shift, from center with Ctrl), rotate (15° steps with Shift), flip, nudge with arrows and Alt+arrows, and Ctrl-drag to copy
- [x] **1.0** Snap to margin and grid guides, ruler guides, and other objects, with visual alignment guides
- [x] **1.0** Layering, grouping, and locking (lock position and size)
- [x] **1.0** Text wrapping: None, Square, Tight, Through, Top and Bottom, Edit Wrap Points, and wrap distances
- [x] **1.0** Format Object dialog: Colors and Lines, Size (with rotation and scale), Layout (position, wrap, distances), Picture, Text Box (margins, autofit, columns, vertical alignment), and Alt Text
- [x] **1.0** Measurement toolbar: x, y, width, height, rotation, tracking (percent), text scaling, kerning (points), and line spacing
- [x] **1.0** Measurements to three decimal places in every unit (8.125", 1.234 cm, 10.125 pt), shown without trailing zeros and kept without rounding
- [x] **1.0** Scratch area shared across pages
- [x] **1.0** Rulers in in, cm, mm, pt, and pica, with drag-out guides and a zero point you can move: Shift and the right mouse button on a ruler, or a drag from the box where the rulers meet; double-click a ruler (or the box) to put it back. The rulers and the status bar count from it, and it is saved with the publication *(in `.jpub`; `.pub` files open with it at the page's corner)*
- [x] **1.0** Context menus for every object type
- [x] **1.0** Undo and redo with full history
- [x] **1.0** Two-page spreads and facing pages
- [x] **1.0** The window: a Pages list (drag to move a page, right-click to add, copy, delete, rename, or move), a status bar (page buttons, pointer position, selected size, view buttons, and zoom), and task panes on the right (Find and Replace, Design Checker, Mail Merge, Research, Graphics Manager, Online Pictures, Catalog Merge, and Help)

## Text engine
- [x] **1.0** Rich text: fonts, sizes, color (scheme-aware), highlight, underline styles, strike, sub and superscript, all caps, small caps, tracking (a percentage, with Publisher's Very Tight to Very Loose presets) and kerning (points) together, automatic pair kerning (for fonts 14 pt and above unless set otherwise), and scaling
- [x] **1.0** Paragraphs: alignment, indents (left, right, first line, hanging), spacing before and after, line spacing (single, multiple, exact, at least), keep with next, keep lines together, widow and orphan control, start in next text box
- [x] **1.0** Tabs with leaders (left, center, right, decimal) *(a publication's default tab stops, 0.5 in unless it says otherwise, are set in the Tabs dialog, from 1 to 1,584 pt, as one undo step, and are read from and saved to .pub files and kept in JeffPub's own)*
- [x] **1.0** Bullets and numbering, multilevel (Increase Indent nests list items: 1. then a. then i., or a dot, a circle, then a square)
- [~] **1.0** Text styles (paragraph and character) with Based On and Style for Following Paragraph *(Based On is saved and a style takes what its base sets when it is applied, but text already styled does not follow a later change to the base style; Help says so)*
- [x] **1.0** Linked text frames across pages, columns inside frames, text wrap around objects, vertical alignment, autofit
- [x] **1.0** Continued-on and continued-from notices
- [x] **1.0** Fields: page number, page count, date and time, business information, and mail merge
- [~] **1.0** Hyphenation (automatic or manual, by language) and optional hyphens (Insert Optional Hyphen, and Add Optional Hyphens for a whole word) *(nonbreaking spaces and hyphens are respected by the line breaker when they are in the text, and the Symbol window can insert them; no command or key was found that inserts them, so that part could not be confirmed)*
- [x] **1.0** Find and Replace across all stories
- [x] **1.0** AutoCorrect (replacement list, capitalization rules) and AutoFormat-as-you-type
- [x] **1.0** Right-to-left and complex script text via Qt's shaping engine, with a right-to-left paragraph direction (Home tab and the Paragraph dialog)
- [x] **1.0** Align text to baseline guides, a paragraph choice that sits lines on the page's baseline guides

## Color and graphics
- [x] **1.0** Color schemes with 8 slots (Main, Accents 1–5, Hyperlink, Followed Hyperlink) and tints and shades of every scheme color
- [x] **1.0** Custom colors (RGB, HSL, CMYK, PANTONE-style spot color entry), recent colors, eyedropper, and transparency on fills, lines, and text *(spot colors are entered by name and value, since the PANTONE library itself is proprietary; colors entered as CMYK keep their exact ink amounts)*
- [x] **1.0** Fill effects: gradient (linear, radial, rectangular, path, with presets), texture, pattern, picture (stretch or tile), and tint
- [x] **1.0** Picture formats: PNG, JPEG, GIF, BMP, TIFF, WebP, SVG, and EMF/WMF **(JeffPub plays EMF and WMF itself, so they look the same on every system, except that text in Wingdings or Webdings shows only some of its symbols on a computer without those fonts, which Linux is; EMF drawing that exists only as EMF+ records, gradient fills, and see-through bitmaps are not drawn)**

## Platform and packaging
- [x] **1.0** Builds and runs on Linux (x86_64) and Windows 10/11 (x86_64)
- [x] **1.0** Windows installer and portable zip
- [x] **1.0** Linux packages: `.deb` (Debian, Ubuntu) and AppImage
- [~] **1.0** macOS app (Apple silicon and Intel) in a `.dmg`, signed and notarized so it opens without warnings *(the release job builds a `.dmg` that holds both apps; it is not signed or notarized)*
- [x] **1.0** File associations for `.jpub`; `.pub` files list JeffPub under "Open with" without making it the default
- [x] **1.0** Light and dark interface themes, switched in File > Options at once (no restart), and following the system's setting while set to
- [x] **1.0** Windows high contrast mode: the system's colors throughout, applied as it's turned on or off
- [x] **1.0** Standard desktop publishing keyboard shortcuts (Ctrl+B, Ctrl+Shift+>, F9, Ctrl+Shift+L, Ctrl+M for the master page, Ctrl+Shift+C and Ctrl+Shift+V to copy and paste formatting, Ctrl+Shift+N, and others), no two commands sharing keys
- [x] **1.0** KeyTips: press Alt and letters appear on the ribbon (the same letters as Microsoft Publisher's for the tabs and the File button); type a tab's letter, then a command's, to use it without the mouse
- [x] **1.0** The ribbon by keyboard: Tab and the arrow keys move among its tabs and controls, with a visible focus ring; Escape returns to the page; F6 moves between the ribbon, the page thumbnails, and the page
- [x] **1.0** Screen readers (Narrator and others): every control in the window (the ribbon's tabs, buttons, galleries, and color buttons, the page list, the page, the task panes, and the status bar) has a name and a description, checked by tests for every control and with Windows' own screen reader interface; the objects on the page are read as well, each by its kind and its text or alt text with where it is and how big ("Text box: Spring sale", "Picture: a red barn", "Picture, no alt text"), the selected one as the focus, and the text being typed with its cursor and selection
- [x] **1.0** In seven languages: English, German, Spanish, French, Italian, Dutch, and Portuguese (Brazil and Portugal), the program and its Help, following the system's language or the choice in File > Options; the translations are machine-made and checked automatically, and can be corrected with Qt Linguist (see translations/README.md)
- [x] **1.0** Bundled open fonts, including metric-compatible replacements for Calibri, Cambria, Arial, Times New Roman, and Courier New; a missing stock Windows or Office font (336 faces measured) keeps each letter's own width, and from 14 pt its own pair kerning (none where the original has none), on its stand-in, so lines break where they do in Microsoft Publisher; where such a font has no italic face (Impact, Garamond's bold, Script MT Bold), its upright stand-in is slanted by a third, and bold on a font with no bold face (Franklin Gothic Demi, Forte) is drawn by hand as Publisher does, each place its own way: widened 0.017 em on the screen and in pictures (a whole pixel for small letters at 96 to 216 dpi), stroked 1/35 em in a PDF export and 0.01 em on paper (Arial Black and Rockwell Extra Bold are stroked in a PDF and on paper, bold or not, and plain on the screen), from 216 pt as outlines
- [x] **1.0** The Quick Access Toolbar: Save, Undo, Redo, and Print, with KeyTips 1 to 4

## Web tab (Publisher's web page tools)

Publisher 2019 still has a Web tab for making a web page from a publication. JeffPub exports a single HTML file of page pictures, and none of these exist yet; whether to build them is an open question.

- [ ] **later** Hotspots and a Navigation Bar: clickable areas on a picture and a bar of links across pages
- [ ] **later** Web form controls: text box, text area, check box, option button, list box, and Submit button
- [ ] **later** HTML Code Fragment (a piece of your own HTML placed in a page), Background Sound, and Web Page Options
- [ ] **later** Web Page Preview in the browser

## File formats

What each format needs, to open, to save, and to come back unchanged. "Round trip" means open, save, and open again in Publisher without a repair message or a visible change. Publisher 2021 on the Windows test machine is the judge.

### Publisher `.pub`
- [~] **1.0** Open `.pub` files from Publisher 98 on: pages, master pages, text and its formats, fields, linked text boxes, shapes, lines, pictures, tables, booklets, folded cards, envelopes, and objects in text *(Help says every version since 1998; all 284 publications of the owner's collection open and save again; the 26 public test files are read in the tests, three of them damaged on purpose; fonts stored inside a file, any section's page numbering after the first, and pictures the program cannot draw are reported when opening)*
- [x] **1.0** Save `.pub` as a complete file in the Publisher 2003-and-later layout, with the parts listed under Open, Save, and Close; Publisher opened all 284 saved copies and pictured them as it pictured the originals
- [x] **1.0** Save reports what it leaves out, and those objects stay in the publication and are kept in `.jpub`
- [ ] **1.0** Score `.pub` files JeffPub was never tuned on (a held-out set, drawn from the same kinds of publications), so the bar for leaving beta is measured on fresh files *(see the open questions)*
- [ ] **later** Read the fonts stored inside a `.pub` file, and use them while that publication is open
- [ ] **later** Save the earlier Publisher file versions (98, 2000, and 2002 to 2003) that Publisher's Change File Type offers *(which versions it can write could not be confirmed; JeffPub writes the 2003-and-later layout only)*
- [ ] **later** Keep a `.pub` file's macro project untouched on save, and never run it *(the writer adds an empty macro storage; whether a macro project in an opened file is kept could not be confirmed)*

### JeffPub `.jpub`
- [x] **1.0** Open and save `.jpub`, a ZIP of readable JSON and the original pictures, with a published specification kept true by a test; the same kind, saved to the templates folder, is a template
- [x] **1.0** AutoRecover copies of unsaved work, offered again after a crash and cleaned up once saved
- [ ] **later** Fonts inside a `.jpub`: the file keeps the names of the fonts a publication used, not the fonts *(see the open questions; JeffDeck embeds the bundled fonts its text uses)*

### PDF
- [x] **1.0** Export PDF: standard, minimum size, high quality, and commercial press, with page ranges, document properties, a booklet as printed sheets, and fonts embedded
- [x] **1.0** Export PDF/A (ISO 19005, PDF/A-1b) for archives, PDF/X-1a:2001 and PDF/X-4 for printers, with a print check first
- [x] **1.0** Place a page of a PDF as a picture, shown as PDFium draws it and printed and exported as vectors
- [ ] **later** Open a PDF as a publication, each page rebuilt as text boxes and pictures *(not a Publisher feature; a PDF is read as a picture today)*

The gaps in an exported PDF (links, bookmarks, tags, and a password) are listed under Share and Export.

### E-books, web pages, and pictures out
- [x] **1.0** Export EPUB: reflowable (chapters at Heading 1, contents from Heading 1-3) or page for page (fixed layout)
- [x] **1.0** Export a single-file web page (HTML), and pages as SVG drawings
- [x] **1.0** Export pages as pictures: PNG, JPEG, GIF, TIFF, and BMP at a chosen resolution

A web site of several linked pages is not offered; see the Web tab section.

### Pictures in
- [x] **1.0** Pictures in publications: PNG, JPEG, GIF, BMP, TIFF, WebP, SVG, WMF, EMF, ICO, and PDF, up to 256 MB a file, embedded or linked
- [ ] **later** Modern photo formats (HEIC and AVIF) *(not in the picture filter; whether Qt's image plugins in the packages would read them could not be confirmed)*

### Text, lists, and contacts
- [x] **1.0** Insert File: `.txt`, `.rtf`, `.html`, `.htm`, `.docx`, and `.md` into a text box
- [ ] **later** Insert File from `.doc` and `.odt` *(needs the readers JeffWrite will build)*
- [x] **1.0** Recipient lists for a mail merge: CSV, tab-delimited, `.xlsx` (values only), and vCard `.vcf`; out: a recipient list as CSV, one `.eml` per recipient, and one PDF
- [ ] **later** Lists in `.xls` and `.ods`, and contacts written back as vCard

### Printing and color
- [x] **1.0** Print to any printer, with separations for CMYK and spot plates; ICC color profiles (`.icc` and `.icm`) for PDF/X
- [ ] **later** XPS: not offered, since the format has no use outside Windows *(Publisher saves XPS)*

### What must survive a round trip
- [~] **1.0** A checklist for every format JeffPub saves: pages and master pages, text with its runs, styles, fields, and links, shapes, pictures with their crops and effects, tables with merged cells, booklets and cards, Text Art, building blocks, alt text, and the publication's properties *(each part has a test for the formats `.jpub` and `.pub`; no single checklist exists, and macro projects and stored fonts are not carried through `.pub`)*

## Shared with other JeffOffice apps

What JeffPub is built on, what its work gave the other apps, and the pieces to build once so that JeffDeck, JeffWrite, JeffCrunch, JeffMail, and JeffBase can use them too. Since October 11, JeffPub's source has been split into shared libraries and the app.

### Already there
- [x] **1.0** libs/page: the document of placed objects (pages, master pages, frames), its editor (undo, selection, text editing, align and distribute), canvas, renderer, proofing dialogs (Spelling and Thesaurus), text dialogs, options, Online Pictures, the icon picker, and the Text Art gallery and its Edit Text window, moved out of JeffPub so that JeffDeck uses them too
- [x] **1.0** libs/shell: the window, ribbon, KeyTips, File screen frame, Help, translations, AutoRecover, settings, spelling, updates, usage statistics, and crash reports
- [x] **1.0** libs/draw, libs/text, and libs/base: shapes, fills, effects, text art, SVG, picture effects, the text engine, colors, fonts, barcodes, zip and compound files, EMF and WMF, PDF, and the dictionaries *(the text engine now draws a run's effect list of shadows, glow, and reflection, from JeffDeck's work; JeffPub's ribbon still sets its own Shadow and Glow only)*
- [~] **1.0** libs/office: Open XML packages, DrawingML, themes, charts, and the embedded-font readers, built for JeffDeck; JeffPub links it but uses none of it yet *(its `.docx` reader for Insert File sits in apps/pub/io)*
- [x] **1.0** JeffPub's own pieces in apps/pub: the `.pub` reader and writer, `.jpub`, EPUB, PDF/X, templates and Change Template, mail merge and its list readers, the Design Checker, Find and Replace, building blocks, Pack and Go, and the File pages

### To build once, for several apps
- [ ] **later** Move the rest of JeffPub's finished tools into libs/page and libs/shell: Find and Replace (a pane in apps/pub today), the crop tool's ribbon controls, and the paste choices *(needed by: JeffDeck, JeffPub, JeffWrite)*
- [ ] **later** A Format pane and a Selection Pane, built once *(needed by: JeffDeck, JeffPub, JeffWrite; Publisher itself has the Format Object window and no Selection Pane)*
- [ ] **later** Shape styles, picture styles, and text effects menus on the effects the engine already draws, which would give letters Reflection and Bevel *(needed by: JeffDeck, JeffPub, JeffWrite)*
- [ ] **later** A table engine for every app, starting from JeffPub's tables (merged cells, borders, diagonals, sizes, and the Table Design and Table Layout tabs) *(needed by: JeffWrite, JeffMail, JeffCrunch, JeffDeck)*
- [ ] **later** A merge-source library (CSV, tab-delimited, `.xlsx`, `.ods`, vCard, and later JeffBase tables and JeffCrunch ranges) with filter, sort, duplicate, and address checks, moved out of libs/page/core and apps/pub *(needed by: JeffPub, JeffWrite, JeffMail, JeffBase)*
- [ ] **later** An iCalendar and vCard library, moved out of JeffPub's merge code *(needed by: JeffMail, JeffPub, JeffWrite, JeffBase)*
- [ ] **later** A MIME library, so that JeffPub's `.eml` files and every app's "send by email" are written the same way *(needed by: JeffMail, JeffPub, JeffWrite, JeffDeck, JeffCrunch)*
- [ ] **later** Share by email from every app: File > Share > Email opens a JeffMail message with the page, the publication, or a PDF attached, and falls back to the ready-to-send `.eml` file JeffPub makes today when JeffMail isn't installed *(needed by: all)*
- [ ] **later** A field engine on the field model of libs/text for page numbers, dates, document properties, and merge fields *(needed by: all)*
- [ ] **later** Building blocks and templates in one format and gallery, with JeffPub's Page Parts, Calendars, Borders & Accents, Advertisements, and templates as the model *(needed by: all)*
- [ ] **later** Themes, table styles, and cell styles in one format and gallery, so JeffPub's color and font schemes and a JeffDeck theme can color each other's files *(needed by: all)*
- [ ] **later** An accessibility checker (alt text, titles and headings, reading order, tables, and contrast) with a task pane, beside the Design Checker *(needed by: all)*
- [ ] **later** Keeping a macro project untouched in the kinds that can hold one, never running it *(needed by: JeffWrite, JeffCrunch, JeffDeck, JeffPub)*
- [ ] **later** Embedded worksheets: Insert > Object in JeffPub shows the saved picture and opens in JeffCrunch's grid in place *(needed by: JeffCrunch, JeffWrite, JeffDeck, JeffPub)*

## Left out on purpose

Not planned, because they only work with an online service or a product JeffOffice doesn't have: Office add-ins and the Store; Smart Lookup (Look Up) and Researcher; Microsoft's libraries of stock images and clip art (Online Pictures and the bundled icons stand in); Microsoft Translator (Translate opens LibreTranslate); sign-in, OneDrive, SharePoint, online recent places, and Shared with Me; the template and building-block communities Microsoft hosts; Send a Smile and Send a Frown (the Feedback button stands in); the Insider program; and the Fix It and contact-support links. Running a macro in a file is also left out, since nothing in a file runs on its own (see Decided); the Developer tab (Visual Basic, Macros, Macro Security, and the Add-In Manager) goes with it.

## Decided

- Old and open formats (October 11, 2026, for all the apps): the first release of an app opens the old and the open kinds, and saving them follows once the main kind is solid. JeffPub already opens and saves its main kind, `.pub`, so this changes nothing here.
- Reference program: Publisher is installed on the Windows test machine (Office LTSC Professional Plus 2021), as Word, Excel, and PowerPoint are. Every fidelity number in this list compares JeffPub with Publisher's own output.
- Security (October 11, 2026, decided for the whole suite): nothing in a file runs or reaches out until the person allows it, and every file reader is fuzzed and held to limits on memory and time. JeffPub's `.pub` reader is tested with damaged and fuzzed files.
- Linked pictures (October 10, 2026): a publication opened from a file follows a link only if the picture is in the publication's own folder or below it; any other link, network paths and devices included, is never touched, and shows its stored copy as Not updated until the person runs Update Link or Change Link on it.
- Email: JeffPub never sends a message itself. It writes a ready-to-send `.eml`, PDF, or HTML file (and one `.eml` per recipient for an email merge) to open in a mail program.
- No Microsoft code or artwork: JeffPub contains no source code, program files, icons, logos, templates, clip art, or fonts from Microsoft; its templates and sample text are original (README, Legal notices).
- Test files (October 8, 2026): no Publisher file of anyone's collection goes in the repository, and neither do generated designs or font pages; tests build `.pub` files in temporary folders, and the public samples in `tests/data/pub` are unchanged copies of other projects' test files.
- Embedded fonts (October 11, 2026, decided for JeffDeck and followed by JeffWrite): saving embeds, by default, the fonts JeffOffice brings that other computers may lack; fonts drawn in place of common ones are saved under the common name. JeffPub's PDF export embeds fonts already; whether `.jpub` does too is an open question.

## Open questions

1. The fidelity bar for dropping Beta: JeffPub's pages now differ from Publisher's by a median of 0.96 and a mean of 1.74 on the owner's 375 pages, with 73% under 2 (79% of 78 template pages). What share under 2 on files it was never tuned on lets the label come off?
2. Does leaving beta wait for a signed and notarized Mac app, which needs an Apple developer account, or does the unsigned test build ship as it is?
3. Which PDF features make 1.0: links only (as tiered here), or bookmarks and tags too, since a PDF is how most publications leave JeffPub?
4. Should a `.jpub` embed the bundled fonts a publication uses, as JeffDeck's `.pptx` does, so that it looks the same on any computer, or only Pack and Go and PDF?
5. Sections: a Publisher file whose later sections number their pages on their own opens with the first section's numbering only. Is that acceptable for 1.0, or do sections (and per-section page number formats) have to be built? A count of the owner's files that have them would settle it.
6. Does a macro project in an opened `.pub` file have to survive a save, untouched and never run, or may it be dropped with a message?
7. Web pages: build Publisher's Web tab (hotspots, navigation bars, form controls, HTML fragments), or leave web design out and keep the single-file web page?
8. East Asian layout (phonetic guides, enclosed characters, horizontal text in vertical lines, and conversion between Chinese forms): 1.0 for the people who need it, or later?
9. Letters get Reflection, Bevel, and a Text Styles gallery from the shared effects menus the JeffDeck list plans, or does JeffPub add them to its own ribbon first?
