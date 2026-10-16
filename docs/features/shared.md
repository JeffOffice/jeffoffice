# Shared parts of JeffOffice

JeffOffice is six apps (JeffPub, JeffDeck, JeffWrite, JeffCrunch, JeffMail, and JeffBase) that are meant to share one body of code, so a feature built for one shows up in the others and a file one app saves, the others can read. Each of the six feature lists ([README](README.md)) ends its own "Shared with other JeffOffice apps" section with what that app builds on and what it expects to build for the others, and many items elsewhere in a list carry a "needed by" note. This page puts all of that in one place, so a developer can see where a shared piece lives, which app builds it first, and who uses it.

It was compiled from the six "Shared with other JeffOffice apps" sections, all 83 "needed by" notes, the "shared:" notes inside the lists' items, the libraries as they stand in the source tree, and the "How it is built" section of [docs/jeffdeck.md](../jeffdeck.md). Where a list and the code disagree about where something lives today, the code is taken as right. The six lists were not changed; the places where they disagree with each other are collected in section 4, with a recommended answer for each, for whoever edits the lists.

In the tables, a list is named by its app (`write` is [jeffwrite.md](jeffwrite.md), `deck` is [jeffdeck.md](jeffdeck.md), and so on), followed by the first words of the item. Rows are numbered (A1, B1, and so on) so that section 4 can point at them.

## 1. The layers, and the rule for where a piece lives

The libraries are stacked, lowest first, and each uses only the ones below it. `tools/check_layers.py` checks every `#include` against this order, and each library is linked alone on Linux to catch a call the include check cannot see.

| Layer | Library | What belongs in it |
|---|---|---|
| base | `libs/base` (core, io, render, text) | Data and files, with no placed objects: colors and color schemes, fonts and font files, barcodes, ZIP and compound-file containers, PDF pages, metafiles, the dictionaries and hyphenation, and the storage of rich text. New pure-data services (calculation, MIME, vCard, time zones) belong here. |
| draw | `libs/draw` (core, render) | Placed objects (frames of every kind, in points), fills, outlines, effects, AutoShapes, Text Art, SVG, and the style presets. |
| text | `libs/text` | The text engine, and the models that sit directly on text: fields now, and the flow, review, and math models when they are built. |
| office | `libs/office` | The file formats of the office kinds: packages (OPC), themes, DrawingML, charts, binary drawings, embedded fonts, and, when built, OpenDocument, the old binary readers, HTML, and data sources. |
| shell | `libs/shell` | The window and everything around a document that every app needs: ribbon, KeyTips, File screen, Help, translations, settings, AutoRecover, updates, statistics, crash reports, the spelling service, and widgets. Controls that need no page (the grid, the chart editor) belong here too. |
| page | `libs/page` (core, render, canvas, app) | Pages of placed objects and their editor, canvas, and dialogs. |
| apps | `apps/pub`, `apps/deck`, and later `apps/write`, `apps/crunch`, `apps/mail`, `apps/base` | Each app's window, commands, document model, and the readers and writers of formats only it opens. An app never uses another app. |

The build puts draw below text (the text engine uses the placed objects), which is the order above and the order in `docs/jeffdeck.md`.

**The rule.** A piece used by two or more apps lives in the lowest layer that has everything it needs. A piece used by one app stays in `apps/<app>` until a second app needs it. Three things follow:

- Because an app never uses another app, anything two apps both read or write (a `.xlsx` for a mail merge, a `.docx` for Insert File, a `.pptx` for a mail preview) has to live in a library, not in the app that wrote it first.
- A piece that needs no page and no window goes below the page layer, even if the first app to want it has pages. The calculation engine needs nothing but numbers and dates, so it goes in base; the grid control needs the text engine and widgets but no page, so it goes in shell.
- A piece is built in its shared layer from the start. It is not built inside one app and moved later, except for the pieces that already sit inside JeffPub or JeffDeck, which the first new app that needs them moves.

**Who builds first.** The app whose release needs the piece soonest. The order of need is JeffDeck for the four basics its Preview label waits on (Find and Replace, picture cropping, table tools, and slide master editing), then JeffWrite, JeffCrunch, JeffMail, and JeffBase. When a piece already exists inside JeffPub or JeffDeck, the app that builds it first also moves it and switches JeffPub and JeffDeck to the shared copy in the same change. Where every app lists a piece as "later", the app that already has working code goes first.

## 2. Already shared

These exist today in the libraries. "Used today" is JeffPub and JeffDeck, the two apps that exist; "will be used by" comes from the other four lists.

| ID | Piece | Library and main files | What it does | Used today | Will be used by |
|---|---|---|---|---|---|
| A1 | Scheme-aware colors | `libs/base/core`: color.h | A color is a fixed RGB value or one of eight color-scheme slots, lightened or darkened; switching the scheme recolors every reference. | JeffPub, JeffDeck | All |
| A2 | Fonts | `libs/base/core`: fonts.h, fontfiles.h, sfnt.h; `libs/text/text`: fontaverages.inc; `resources/fonts` | Bundled open fonts, same-width stand-ins for common proprietary fonts that are missing, the font files a document names (to copy into a file), and a font-file reader that checks a font is sound. | JeffPub, JeffDeck | All; JeffCrunch relies on the stand-ins to keep column widths and wraps |
| A3 | Barcodes | `libs/base/core`: barcode.h | Book ISBNs, EAN-13, UPC-A, and EAN-8 with add-ons, Code 128, and Code 39, as exact bars in points. | JeffPub (JeffDeck has no command yet) | JeffWrite, JeffCrunch, JeffBase (labels) |
| A4 | ZIP and compound-file containers | `libs/base/io`: zip.h, cfb.h | Read and write ZIP packages; read every storage and stream of a compound (OLE2) file, and write a valid one back. | JeffPub (`.jpub`, EPUB, `.pub`), JeffDeck (`.pptx`, `.ppt`) | JeffWrite (`.docx`, `.doc`), JeffCrunch (`.xlsx`, `.xls`), JeffMail (`.msg`), JeffBase |
| A5 | PDF pages and Qt's PDFs | `libs/base/render`: pdfpage.h; `libs/base/io`: qtpdf.h, pdfspots.h | A PDF page as a picture (through PDFium), and reopening the PDFs Qt writes to add spot colors and PDF/X. | JeffPub, JeffDeck | JeffMail (previews), all (export; see B28) |
| A6 | Windows metafiles | `libs/base/render`: metafile.h | EMF and WMF drawn as vectors through QPainter, so they stay sharp on screen, in print, and in PDF. | JeffPub, JeffDeck | JeffWrite, JeffCrunch, JeffMail, JeffBase |
| A7 | Dictionaries | `libs/base/text`: dictionaries.h, hyphenation.h; `resources/dict` | Spelling and hyphenation for 12 languages (English for the United States, Canada, Australia, and the United Kingdom; Spanish for Spain and Mexico; French; German; Italian; Dutch; and Portuguese for Brazil and Portugal) and an English thesaurus, chosen by the language tag on the text. | JeffPub, JeffDeck | All |
| A8 | Story storage and text properties | `libs/base/text`: storyio.h, textprops.h | Lossless JSON of a text story, and the custom text properties (fields, list kinds, slide line spacing) that Qt's own formats drop. | JeffPub, JeffDeck | JeffWrite, JeffMail, JeffBase |
| A9 | Placed objects and drawing | `libs/draw/core`: items.h, style.h, presets.h, shapetypes.h, svg.h, texteffects.h, blipeffects.h; `libs/draw/render`: shapes.h, textart.h | Frames of every kind (text, picture, shape, line, table, group); AutoShapes; Text Art; an SVG reader; shadow, glow, and reflection on text; picture color effects; presets for table formats, shape styles, and picture styles. | JeffPub, JeffDeck | JeffWrite (floating objects), JeffCrunch (drawings on a sheet), JeffBase (report and form decoration) |
| A10 | Text engine | `libs/text/text`: textengine.h | Lays out a story across a chain of frames: paragraphs, tabs with leaders, columns, insets, wrap obstacles, hyphenation, footnote layout, right-to-left text, and the effects on a run. | JeffPub, JeffDeck | JeffWrite (extends it from frames to pages), JeffCrunch (text in cells, text boxes, and chart labels), JeffMail, JeffBase |
| A11 | Field model | `libs/text/text`: textengine.h (FieldSource, FieldContext); `libs/base/text`: textprops.h | A field is a code in a run of text. The document answers page numbers and counts, business information, merge fields, and note numbers; the engine draws the answer, or the «Field» code when no record is chosen. | JeffPub | All, through the field engine (B9) |
| A12 | Open packages and DrawingML | `libs/office/office`: package.h, xml.h, theme.h, drawingml*.h | Read and write the ZIP packages of parts that `.pptx`, `.docx`, and `.xlsx` are, the theme part, and DrawingML shapes, pictures, connectors, text, tables, and colors. | JeffDeck (JeffPub links it and uses none of it) | JeffWrite, JeffCrunch, JeffBase, JeffMail |
| A13 | Charts, read and drawn | `libs/office/office`: chart.h, chart_svg.cpp | Reads a chart part into a model from the values it stores, and draws it as SVG (column, bar, line, pie, doughnut, area, and scatter). | JeffDeck | JeffWrite, JeffCrunch, JeffBase |
| A14 | OfficeArt drawing reader | `libs/office/office`: odraw*.h | Reads the binary drawing records inside `.ppt`, `.doc`, and `.xls` files into placed objects. | JeffDeck (`.ppt`) | JeffWrite (`.doc`), JeffCrunch (`.xls`) |
| A15 | Embedded-font readers | `libs/office/office`: eot.h, mtx.h; `libs/base/core`: sfnt.h | Take a font out of a presentation file (Embedded OpenType and MicroType Express), treating every number in it as input from a stranger. | JeffDeck | JeffWrite, JeffCrunch (saving fonts is not shared yet; see B16) |
| A16 | Window, ribbon, and KeyTips | `libs/shell/app`: appwindow, ribbon, ribbonbuilder, keytips, keyboardnav, focusring, widgets, icons, theme | The main window; the ribbon built from `resources/ribbon.json`; KeyTips; arrow-key movement among controls; the focus ring; color pickers, galleries, font and size boxes, measurement spin boxes, and the Insert Table grid; the Lucide icon set; the light and dark looks. | JeffPub, JeffDeck | All |
| A17 | File screen and file dialogs | `libs/shell/app`: filescreen, filescreenparts, filedialogs | The File screen's frame (sidebar, back button, commands, and pages), its cards and paper-page previews, the suite's app list, and the Open, Save, and Insert Picture dialogs. | JeffPub, JeffDeck | All |
| A18 | Help and translations | `libs/shell/app`: help.h, helpwiki.cpp, i18n.h; `resources/help` | Help topics in Markdown with F1 context and search; one translation set for the suite, with Qt's own dialogs. | JeffPub, JeffDeck | All |
| A19 | Settings, AutoRecover, updates, statistics, and crash reports | `libs/shell/app`: settings, recovery, updater, telemetry, crash; `libs/base/core`: appinfo.h | Preferences and measurement units; copies of unsaved work; the update check with Stable and Beta channels; anonymous usage statistics; crash reports. Each app tells the libraries who it is once, at startup. | JeffPub, JeffDeck | All |
| A20 | Spelling service | `libs/shell/app`: spelling.h | The misspelled ranges of a story, the person's own words, and the session's ignore list, for checking as you type and for the dialogs. | JeffPub, JeffDeck | All |
| A21 | Document of placed objects, and its renderer | `libs/page/core`: document.h; `libs/page/render`: renderer.h, svgexport.h | The Document (pages, master pages, text stories, pictures, styles, color and font schemes) and one painter-based renderer for the screen, thumbnails, PDF, pictures, the printer, and SVG. | JeffPub (publications), JeffDeck (slides and layouts) | JeffWrite (floating objects); JeffCrunch and JeffBase use the editor pieces, not the page |
| A22 | Editor and canvas | `libs/page/app`: editor.h, editor_text.cpp; `libs/page/canvas`: canvas.h, canvasaccessible.h | Selection, text editing, AutoCorrect, the clipboard, undo as one step, align and distribute, connectors, Format Painter, crop mode, rulers, guides, and a screen-reader view of a page. | JeffPub, JeffDeck | JeffWrite (its flowing editor grows from it), JeffCrunch (undo, drawings), JeffBase |
| A23 | Dialogs for apps with pages | `libs/page/app`: textdialogs, proofingdialogs, optionsdialog, onlinepictures, iconpicker, textartgallery | Spelling and Thesaurus; text, list, symbol, link, fill, and table dialogs; Options (the suite's General page and Proofing, then the app's own); Online Pictures from Openverse and Wikimedia Commons, with credits; the icon picker (1,539 icons); the Text Art gallery. | JeffPub, JeffDeck | JeffWrite, JeffCrunch, JeffMail, JeffBase |
| A24 | Table object | `libs/draw/core`: items.h (TableItem), presets.h; `libs/office/office`: drawingml_table.h; `libs/shell/app`: widgets.h (the Insert Table grid) | A table as a placed object with merged cells, per-cell fills and borders, and table formats; read and written as DrawingML. The editing tools are not shared yet (B43). | JeffPub, JeffDeck | JeffWrite, JeffMail |
| A25 | Hostile-input guards and tests | `libs/office/office`: package.cpp (limits); `tests/office`, `tests/tests.cpp`; `tools/jdecktool.cpp`, `tools/jpubtool.cpp` | Limits on the size of packages, fonts, metafiles, and merge sources, and tests that feed them damaged and hostile files. | JeffPub, JeffDeck | All (extended by B35) |

## 3. To build once

These do not exist yet, or exist only inside one app and must move. The library and layer are recommendations, chosen by the rule in section 1; where the lists name a different place, section 4 says so. Rows run from the lowest layer to the highest.

| ID | Piece | Library and layer (recommended) | Builds first | Used by | Feature-list items it serves |
|---|---|---|---|---|---|
| B1 | Number-format language (the codes for numbers, dates, text, colors, and conditions, for formatting and for reading) | `libs/base` (new `calc/`): needs only colors and dates | JeffCrunch (first release); see D8 for why its date and number parts are wanted earlier | JeffCrunch, JeffWrite (field and table-cell formats), JeffPub, JeffWrite, and JeffMail (merge formats), JeffBase (column formats and Format codes), chart axes in JeffDeck, JeffWrite, and JeffCrunch (`libs/office/office/chart.cpp` has a private formatter today) | crunch: "The number-format language (the codes above) as a library"; base: "The calculation engine and the number-format language" |
| B2 | Calculation engine (formula parser, dependency graph, function library, date system, error values) | `libs/base` (new `calc/`) | JeffCrunch (first release) | JeffCrunch, JeffWrite (table and field formulas), JeffBase (expressions in queries, validation rules, and defaults); JeffDeck only through the sheet behind a chart | crunch: "A calculation engine as a library"; write: "A table engine with merged cells, styles, nested tables, and formulas"; base: "The calculation engine and the number-format language" |
| B3 | MIME library (parse and build messages, headers, character sets, encoded words) | `libs/base/io` (new `mime`); the header encoding and `.eml` writer in `apps/pub/app/sharing.cpp` move into it | JeffMail (first release) | JeffMail, JeffPub, JeffWrite, JeffDeck, JeffCrunch, and JeffBase (through Share by email) | pub: "A MIME library"; mail: "A MIME library: parse and build messages" |
| B4 | iCalendar and vCard library, with a round trip that keeps what it does not understand | `libs/base/io` (new `vcard`, `ical`); the vCard reader moves out of `apps/pub/io/importers.cpp`, and iCalendar is new | JeffMail (first release) | JeffMail, JeffPub, JeffWrite, JeffBase | pub, mail, and base: "An iCalendar and vCard library" |
| B5 | Date, time-zone, and recurrence rules, and the controls for dates, times, recurrence, and reminders | Rules and the time-zone database in `libs/base` (new `time/`); the controls in `libs/shell` | JeffMail (1.0) | JeffMail, JeffCrunch (date cells), JeffBase (date fields) | mail: "Date, time, recurrence, reminder, and time-zone controls" |
| B6 | Secret store and OAuth sign-in (the system keychain; browser sign-in with a loopback address) | `libs/base/io`: `libs/base` already links Qt Network | JeffMail (1.0) | JeffMail, JeffCrunch (online sources), JeffBase (saved server connections; not yet named in its list) | mail: "A secret store and OAuth sign-in service" |
| B7 | Ink layer (pens, highlighter, eraser, ink to shape) | Strokes as freeform shapes in `libs/draw` (new `ink`); the tools in `libs/page`; the slide show's pen is `apps/deck/core/ink.h` today | JeffDeck (later; the working code is there) | JeffDeck, JeffWrite, JeffCrunch, JeffMail | deck, write, crunch, and mail: "An ink layer" |
| B8 | Flowing-page engine: the flow-document model (sections, paragraphs, tables, floating objects, notes, and headers and footers) and the page layout that makes pages as text fills them | `libs/text` (new `flow/`): needs the text engine and placed objects but no widgets, so it can run without a window | JeffWrite (first release) | JeffWrite, JeffBase (reports: sections that grow with their rows, breaks at group ends, and repeating headers and footers), JeffMail (the composer, and HTML in and out) | write: "A flow-document model and its page layout"; base: the same item and "Report pages use the flow-document layout"; mail: "A mail profile of the flow-document editor" |
| B9 | Field engine: page and date fields, document properties, cross-references, merge fields, and formula | `libs/text` (new `fields/`), on the field model (A11); formula fields call the calculation engine (B2) | JeffWrite (first release) | JeffWrite, JeffPub (page numbers, dates, properties), JeffDeck (slide numbers), JeffMail (dates, merge fields) | pub, write, and mail: "A field engine on the field model of libs/text"; deck: "A field engine for slide numbers, dates, and document properties" |
| B10 | Equation editor and layout for Office Math and MathML | Layout in `libs/text` (new `math/`); the Office Math and MathML readers and writers in `libs/office`; the entry window in `libs/page` | JeffWrite (1.0) | JeffWrite, JeffDeck, JeffCrunch | write, deck, and crunch: "An equation editor and layout for Office Math and MathML" |
| B11 | Review model: comments with replies, tracked changes, and comparison | `libs/text` (new `review/`): the records (author, date, replies, resolved, kind of change) and comparing two stories; the app supplies the anchor (a text range, an object, or a cell); the panes in `libs/shell` | JeffWrite (1.0) | JeffWrite, JeffDeck (its comments are in `apps/deck/core/comments.h` today), JeffCrunch (notes) | write: "A review model for tracked changes, comments with replies, and comparison"; deck: "A review model for comments with replies"; crunch: "A review model for notes and comments with replies" |
| B12 | OpenDocument readers and writers (package and manifest, styles, shapes, charts, text, tables) | `libs/office` (new `odf/`); the working code is in `apps/deck/io` (odpimport*.cpp, odpexport*.cpp) | JeffWrite (first release: `.odt`), by moving the shared parts out of JeffDeck | JeffDeck (`.odp`, `.otp`), JeffWrite (`.odt`, `.ott`), JeffCrunch (`.ods`, `.ots`), JeffPub (Insert File from `.odt`) | write: "Open `.odt` and `.ott`" and "Save `.odt` and `.ott`"; crunch: "Open `.ods` and `.ots`" and "Save `.ods` and `.ots`" (both "shared: JeffDeck's ODF reader or writer") |
| B13 | Readers of the office documents that other apps call: `.doc`, `.xls`, and `.ppt` (the binary kinds), `.docx`, `.rtf`, `.xlsx`, `.csv`, and `.pptx` | `libs/office` (new `word/`, `sheet/`, `slides/`). Compound files (A4) and OfficeArt (A14) are shared already; to share: OLE property sets, embedded OLE objects, and the record-reader guards. JeffDeck's `.ppt` reader is `apps/deck/io/pptimport.*` today | JeffWrite (`.doc`, first release), then JeffCrunch (`.xls`); JeffDeck's `.ppt` and `.pptx` readers move last, and only as far as previews need (see D14) | Each app for its own formats; JeffMail (attachment previews and search), JeffPub (Insert File from `.doc`), JeffBase (spreadsheet import), and the merge-source library (B18) | write: "Open `.doc` and `.dot`"; crunch: "Open `.xls` from Excel 97 to 2003"; mail: "Preview Word, Excel, and PowerPoint files"; base: "Excel and spreadsheet import"; pub: "Insert File from `.doc` and `.odt`" |
| B14 | Password protection of files: Open XML encryption (open and save), and the older RC4 and XOR schemes of `.doc` and `.xls` | `libs/office` (new `crypto/`) | JeffWrite (1.0) | JeffWrite, JeffCrunch, JeffDeck | deck: "Properties, Inspect, Mark as Final, and Encrypt with Password ... and the Open XML encryption"; write: "Encrypt with a password"; crunch: "Open encrypted files and save with a password" |
| B15 | Macro projects kept untouched in the kinds that hold one, and never run | `libs/office` package code: the project parts and their content types survive a save | JeffDeck (1.0; its round trip is meant to keep the parts it cannot edit) | JeffDeck, JeffWrite, JeffCrunch, JeffPub | pub and deck: "Keeping a macro project untouched" |
| B16 | Saving embedded fonts | `libs/office` (new `fonts/`): choose the bundled fonts a file needs and write them; the code is `apps/deck/core/embeddedfonts.cpp` and `apps/deck/io/pptxfonts.cpp` today | JeffWrite (decided: "follow JeffDeck's rule") | JeffDeck, JeffWrite | write, Decided: "Embedded fonts: follow JeffDeck's rule" |
| B17 | Safe HTML and style renderer for untrusted content (no network, no scripts, and bounded memory and time), and HTML import and export | `libs/office` (new `html/`): reads HTML into the flow model (B8); writes clean HTML with a plain-text twin | JeffMail (first release) | JeffMail, JeffWrite (open and save HTML), JeffCrunch (HTML tables) | mail: "A safe HTML and style renderer for untrusted content"; write: "Open an HTML page as a document"; crunch: "Open an HTML page or a saved web page's tables as a workbook" |
| B18 | Merge-source library: fields and rows from CSV and tab-delimited text, vCard, spreadsheets (values as shown or as stored), JeffBase tables and queries, and JeffMail's address books; filter, sort, duplicate, and address checks | `libs/office` (new `data/`). The `MergeSource` struct is in `libs/page/core/document.h` and the readers (`loadMergeSource`, `parseDelimited`) in `apps/pub/io/importers.cpp` today | JeffWrite (1.0: Mailings) | JeffPub, JeffWrite, JeffMail (Merge to Email), JeffBase, JeffCrunch (its text import is the front end) | pub, write, crunch, mail, and base: "A merge-source library" |
| B19 | Data-source layer (text, CSV, JSON, XML, HTML tables, SQLite, ODBC, PostgreSQL, and MySQL) with the editable list of steps | `libs/office/data`, beside B18, with one record-source interface under both | JeffCrunch (1.0) | JeffCrunch, JeffBase | crunch and base: "A data-source layer" |
| B20 | SQLite layer: the safety settings, the Access-to-SQLite SQL translator, and the Access functions registered as SQLite functions | `libs/office/data` (new `sqlite`) | JeffBase (first release) | JeffBase, JeffCrunch ("from a database") | base: "A SQLite layer with the safety settings above"; crunch: "From a database" |
| B21 | SmartArt layout engine (edit a diagram, and draw one a file saved without a drawing) | `libs/office` (new `diagram/`); JeffDeck's reader is `apps/deck/io/pptxdiagram.*` | JeffDeck (later) | JeffDeck, JeffWrite, JeffCrunch | deck: "A SmartArt layout engine"; write: "SmartArt Design and Format"; crunch: "(shared: JeffDeck's SmartArt)" |
| B22 | Themes, table styles, and cell styles in one format and one gallery | The format in `libs/office` (the theme part is A12) with the built-in themes moved out of `apps/deck/core/themes.cpp` and `themeapply.cpp`; the galleries in `libs/shell`; applying them to placed objects in `libs/page` | JeffWrite (1.0: Design tab); JeffDeck holds the working code | All | pub, deck, crunch, and base: "Themes, table styles, and cell styles in one format and gallery"; write: "Themes gallery" |
| B23 | Grid control (virtual rows and columns, frozen and split panes, selection, in-cell editing, fast scrolling) | `libs/shell` (new `grid/`) | JeffCrunch (first release) | JeffCrunch, JeffBase (the datasheet), JeffWrite and JeffDeck (the data grid of chart editing) | crunch and base: "A grid control" |
| B24 | Chart editor: the data grid, the element menus, types, and styles | `libs/shell` (new `chart/`), on the grid (B23) and the chart model (A13) | JeffCrunch (1.0) | JeffCrunch, JeffWrite, JeffDeck, JeffBase (data picked from a query) | deck, write, crunch, and base: "Chart editing on top of libs/office chart" |
| B25 | Embedded worksheets, and cells pasted between apps as a table with fonts, fills, borders, and number formats | `libs/shell` hosts the grid in place; `libs/office` keeps the embedded package and its stand-in picture | JeffCrunch (1.0) | JeffCrunch, JeffWrite, JeffDeck, JeffPub | pub, deck, and crunch: "Embedded worksheets"; deck: "Cells copied from JeffCrunch or a document land as a table" |
| B26 | Grammar and style check (Harper, decided October 11, 2026; English first) | The spelling service in `libs/shell` (A20), with Harper linked as a native library from `third_party` | JeffWrite (first release) | JeffWrite, JeffMail | write: "A grammar and style check behind the spelling service"; mail: "The grammar and style check behind the spelling service (Harper)" |
| B27 | Spelling and proofing on any text source: the Spelling and Grammar dialog, Thesaurus, AutoCorrect, the language setting, and word count | `libs/shell`, through a small "text to proof" interface; the Editor version in `libs/page` (A23) becomes an adapter | JeffWrite (first release) | JeffWrite, JeffMail, JeffCrunch (cells), JeffBase (controls) | crunch: "the spelling service in libs/shell"; mail: "the Spelling and Grammar dialog (F7)" and "JeffWrite's count" |
| B28 | Print, Print Preview, Page Setup, and PDF export (PDF/A, tagged PDF, bookmarks, and a password) | `libs/shell` (new `print/`) over a "pages to paint" interface; the layouts (booklet, several to a sheet, printer's marks) stay with the page layer. JeffPub's code is in `apps/pub`, and JeffDeck has its own preview and export in `apps/deck/app` | JeffWrite (first release) | All six | write, crunch, base, mail, and deck: "(shared: JeffPub's print preview, page setup, PDF export, or printing)" |
| B29 | Share by email: File > Share > Email opens a JeffMail message, and falls back to a ready-to-send `.eml` | `libs/shell`, on the MIME library (B3) | JeffMail (1.0) | All six | pub: "Share by email from every app"; mail: the same |
| B30 | Accessibility checker and its pane | `libs/shell`: the issue list and pane, with each app registering rules for its own kinds of object | JeffWrite (1.0) | All six | pub, deck, mail, and base: "An accessibility checker"; write: "Check Accessibility and its pane"; crunch: "Check Accessibility" |
| B31 | File screen pages: Properties, Inspect, Mark as Final, and Protect | `libs/shell` (the File screen, A17); JeffPub's Properties page in `apps/pub/app/backstage.cpp` is the model | JeffWrite (first release) | JeffWrite, JeffCrunch, JeffDeck | deck: "Properties, Inspect, Mark as Final, and Encrypt with Password on the file screen"; write and crunch: "(shared: JeffPub's publication properties)" |
| B32 | Building blocks and templates in one format and one gallery | The gallery in `libs/shell`; the content in `libs/page` and in the flow editor (B37); JeffPub's Building Blocks (`apps/pub`) are the model | JeffWrite (1.0: Quick Parts); JeffMail (1.0) adds signatures | JeffPub, JeffWrite, JeffMail; the other lists say "all" | pub: "Building blocks and templates in one format and gallery"; write: the same (Quick Parts, page parts, slide layouts); mail: "Building blocks in one format and gallery" |
| B33 | Read Aloud (text to speech) and Dictate | `libs/shell` (new `speech/`), with the system's voices; Dictate only if an offline model is chosen (write, open question 4) | JeffWrite (1.0: Read Aloud) | JeffWrite, JeffMail, JeffCrunch (Speak Cells) | mail: "Text to speech for Read Aloud" and "A Dictate button"; write: "Read Aloud with the system's voices"; crunch: "Speak Cells" |
| B34 | Capture from the screen, the camera, and the microphone, including Screenshot and Screen Clipping | `libs/shell` (new `capture/`) | JeffDeck (later: Record Slide Show) | JeffDeck, JeffWrite, JeffCrunch, JeffMail | deck: "Capture from the microphone, the camera, and the screen"; deck, write, crunch, and mail: "Screenshot and Screen Clipping" |
| B35 | Fuzzing harness and resource limits for readers | Memory and time budgets in `libs/base`; hostile-file tests in `tests/` and a tool for each app (`tools/jdecktool.cpp` and `tools/jpubtool.cpp` today) | JeffMail (first release), extending what JeffDeck and JeffPub did | All | mail: "The fuzzing harness and the resource limits for readers"; write and deck: "Every file reader is fuzzed and held to limits" |
| B36 | Dialogs in `libs/page/app` that need no page, moved down: Online Pictures (needs only appinfo.h) and the Options frame with its General and Proofing pages (the page Editor is used for one setting, spelling as you type) | `libs/shell` | JeffCrunch (first release) | JeffCrunch, JeffMail, JeffBase, which have no page | crunch, mail, and base, Already there: "libs/page's editor pieces ... options, and online pictures" |
| B37 | Flow editor and canvas, and its mail profile: caret, selection, input, undo as one step, pages drawn from B8 with placed objects drawn by the page renderer; the profile switches off what email cannot carry and adds the plain-text twin and the import that cleans received HTML | `libs/page` (new `flow/`) | JeffWrite (first release); JeffMail adds the profile | JeffWrite, JeffMail (the composer) | mail: "A mail profile of the flow-document editor"; write, Decided: "How text flows across pages" |
| B38 | Format pane and Selection pane | The pane frame and the property editors (color, fill, line, number, font) in `libs/shell`; the pages (fill and line, effects, size and position, text options, picture) in `libs/page` | JeffWrite (first release: its Format Shape pane); deck says 1.0 | JeffWrite, JeffDeck, JeffPub, JeffCrunch (Selection Pane, chart Format Selection pane) | pub and deck: "A Format pane and a Selection Pane, built once"; write: "the Format Shape pane"; crunch: "Selection Pane" |
| B39 | Shape styles, picture styles, and text-effects menus on the effects the engine already draws | `libs/page` menus on the `libs/draw` presets (A9) | JeffWrite (first release: shape styles) | JeffDeck, JeffPub, JeffWrite, JeffCrunch | pub and deck: "Shape styles, picture styles, and text effects menus"; write: "Shape styles gallery" |
| B40 | Find and Replace | `libs/page` (searches an Editor's stories, notes, and tables) with a small "searchable" interface in `libs/shell` so the grid, the flow editor, and JeffBase controls plug in. It is a pane in `apps/pub/app/taskpane.cpp` today | JeffDeck (Preview) | JeffDeck, JeffPub, JeffWrite, JeffCrunch (cells, formulas, comments), JeffMail, JeffBase | deck: "Find (Ctrl+F) in slide text, tables, and notes" (needed to drop Preview); pub: "Move the rest of JeffPub's finished tools" |
| B41 | Picture cropping: Crop with handles, Crop to Shape, Aspect Ratio, Fill, and Fit | `libs/page`: crop mode is in the canvas and editor already; the ribbon controls come out of `apps/pub/app/mainwindow_ribbon.cpp` | JeffDeck (Preview) | JeffDeck, JeffPub, JeffWrite, JeffCrunch (picture tools), JeffMail | deck: "Crop with handles, Crop to Shape, Aspect Ratio, and Fill and Fit" (needed to drop Preview); pub: "the crop tool's ribbon controls" |
| B42 | Paste choices (Keep Source Formatting, Merge Formatting, Keep Text Only) | `libs/page` editor (the clipboard code) | JeffDeck (with B40) | JeffDeck, JeffPub, JeffWrite | pub and deck: "the paste choices" |
| B43 | Table editing tools and the Table Design and Table Layout tabs: insert and delete rows and columns, merge and split, borders and shading, sizes, distribute, alignment, and style options | `libs/page` commands on the table object (A24); the two tabs in the ribbon file. The code is in `apps/pub/app/mainwindow_actions.cpp` and `mainwindow_ribbon.cpp` today | JeffDeck (Preview) | JeffDeck, JeffPub, JeffWrite, JeffMail; see D4 about JeffCrunch | pub: "A table engine for every app"; deck: "A table engine with merged cells, styles, borders, and sizes" and the Table Layout items "needed to drop Preview"; write and mail: "A table engine" |

### Named in a list as shared, but used by one app

These stay in `apps/<app>` until a second app asks for them.

| Piece | App | Note |
|---|---|---|
| Jet and ACE database page reader and writer (`.mdb` and `.accdb`) | JeffBase | base: "A reader and writer for Jet and ACE database pages". Its list says JeffCrunch's database import could read `.accdb` through it; if that is wanted, it moves to `libs/office/data`. |
| Movie encoder for video export | JeffDeck | deck: "A movie encoder for video export", needed by JeffDeck only. |
| Property sheet control (a tabbed list of named properties with editors) | JeffBase | base: "A property sheet control". Its editors are the same as the Format pane's; see D10. |

## 4. Disagreements found

Where the six lists disagree with each other, or with the code. Each item gives what the lists say (by app and item), and a recommended answer.

**Decided October 16, 2026:** every recommended answer below is adopted, except D3: the flowing-page engine lives in a new library, **libs/flow**, above libs/page (layers: base < draw < text < office < shell < page < flow < apps), and drives the text engine page by page without changing it; JeffWrite builds it, and JeffBase's reports and JeffMail's composer use it. The reasons and measurements are in [docs/jeffwrite.md](../jeffwrite.md). Where a feature list disagrees with this page, this page holds; each list is brought in line as its app is built.

**Names.** The same piece is called different things. This page uses the left-hand name.

| Name used here | Names in the lists |
|---|---|
| Flowing-page engine (B8) | "A flow-document model and its page layout" (write, base); "the flow-document layout" (base); "a flowing document" beside libs/page (write, Already there) |
| Flow editor and its mail profile (B37) | "A mail profile of the flow-document editor" and "JeffWrite's flowing editor" (mail) |
| Chart editor (B24) | "Chart editing on top of libs/office chart" (deck, write, crunch, base); "the chart editing library" (deck, Chart Design); "the data grid" (write) |
| Table editing tools (B43) | "A table engine for every app" (pub); "A table engine with merged cells" (deck, write, mail); "the Table Design and Table Layout tabs" (deck) |
| Merge-source library (B18) | "A merge-source library" (pub, write, crunch, mail, base); "JeffPub's merge-source reader" (crunch, base); "JeffCrunch's readers" (base); "A data-source layer" (crunch, base) |
| Themes (B22) | "Themes, table styles, and cell styles in one format and gallery" (pub, deck, crunch, base); "libs/office theme" and "JeffDeck's themes" (write); "the themes format planned for the suite" (base) |
| Building blocks (B32) | "Building blocks and templates" (pub, write); "Building blocks" and "the building-blocks gallery" (mail); "Quick Parts" (write, mail) |
| Share by email (B29) | "Share by email from every app" (pub, mail); "email-as-attachment sharing" (mail); "JeffPub's `.eml` export" (write, crunch, deck) |

### D1. Where Find and Replace lives today

- pub, "Move the rest of JeffPub's finished tools": "Find and Replace (a pane in apps/pub today)". deck, same item, and the Find item: "JeffPub has a Find and Replace pane in apps/pub; JeffDeck has none".
- write, Already there: libs/page has "Find and Replace". crunch, Already there: "libs/page's editor pieces, Find and Replace", and "the find and replace dialogs in libs/page; the cell search is new". base, Already there: the same, and "the find and replace dialog in libs/page/app".
- The code: the only Find and Replace is `apps/pub/app/taskpane.cpp`. Nothing in `libs/` has it.
- Tiers: pub says later, deck says 1.0 and needed to drop Preview.

Recommendation: write, crunch, and base correct their Already there items to say it is not shared yet. JeffDeck builds B40 first, because its Preview waits on it, and Pub's "later" is changed to say JeffPub adopts the shared copy.

### D2. Print, Print Preview, Page Setup, and PDF export are called shared by five lists and listed as shared by none

- write (items for Print, Print Preview, page setup, PDF export, booklet printing), crunch (the same), base ("Already there: JeffPub's print and print preview, PDF export"), mail ("JeffPub's printing"), and deck ("PDF extras ... shared: JeffPub's PDF export") each rely on JeffPub's. That is about 25 "shared:" notes.
- No list has an item for it in its "Shared with other JeffOffice apps" section. pub keeps its printing in apps/pub, and the code agrees. JeffDeck has its own print preview and PDF export in `apps/deck/app`, so there are already two copies.

Recommendation: add B28. JeffWrite builds it first, because its first release prints, moving JeffPub's and JeffDeck's code onto it.

### D3. The flow-document layer: names, library, the editor, and what JeffMail's composer uses

- write, Already there: "JeffWrite needs a flowing document beside [libs/page] (see the open questions)", and, Decided: "How text flows across pages (a new library or an extension of libs/page) is an engineering choice".
- write, To build once: "A flow-document model and its page layout", with no item for the editor. base says the same, and "which JeffWrite builds first". mail adds the editor: "A mail profile of the flow-document editor" (needed by JeffWrite, JeffMail).
- mail, Already there: libs/text and libs/page hold "the text engine and editor that JeffWrite's flowing editor grows from, and the one JeffMail's composer uses". That conflicts with mail's own To build once item, where the composer is the flow editor.

Recommendation: split it into B8 (model and layout, in `libs/text/flow`, no widgets, so reports, HTML import, and tests can use it without a window) and B37 (editor and canvas, in `libs/page/flow`). JeffWrite builds both; write names B37 as a shared piece; mail's Already there says the composer uses the flow editor with the mail profile.

### D4. Table engine: tier, first builder, scope, and JeffCrunch as a user

- Tier: pub later; deck, write, and mail 1.0.
- First builder: deck needs Table Layout now (five items "needed to drop Preview"), so JeffDeck is first; write adds nested tables and formulas, so it would say JeffWrite.
- Scope: pub and deck mean tables placed on a page (the table object in `libs/draw`, A24); write and mail mean tables inside flowing text.
- pub, deck, write, and mail all name JeffCrunch as a user. crunch has no table engine item.

Recommendation: JeffDeck builds B43 first, as commands on the table object in `libs/page`. In-flow tables in B8 use the same cell records (fill, borders, merge, style), and formulas go through the calculation engine (B2). Drop JeffCrunch from the needed-by lists: its Excel-style tables are list objects with their own styles, which come from B22. Align the tiers to 1.0.

### D5. Chart editing: no one is named as the builder, and the grid has to exist first

- deck, write, crunch, and base each list "Chart editing on top of libs/office chart" at 1.0, and none says who builds it. deck says a chart's data lives in a small embedded sheet "edited with JeffCrunch's grid", which crunch lists at first release.
- The needed-by lists differ: deck, write, and crunch omit JeffBase; base includes itself.

Recommendation: JeffCrunch builds B24 on B23. JeffDeck's and JeffWrite's chart editing wait for it. Add JeffBase to the other three needed-by lists.

### D6. Review model: three descriptions of one piece

- deck: "comments with replies, tracked changes, and comparison"; write: "tracked changes, comments with replies, and comparison"; crunch: "notes and comments with replies" (no tracked changes or comparison). crunch also says "the review model to build for JeffWrite".
- JeffDeck already has comments, in `apps/deck/core/comments.h`.

Recommendation: one model (B11) with two parts, comments with replies (every app) and revisions with comparison (JeffWrite, JeffDeck). JeffWrite builds it, and JeffDeck's comments move onto it.

### D7. Merge-source library, data-source layer, and "JeffCrunch's readers"

- Tier: pub later; write, crunch, mail, and base 1.0.
- Today's code: pub says "moved out of libs/page/core and apps/pub"; write says only "libs/page/core". The code has the `MergeSource` struct in libs/page/core and the readers in apps/pub/io/importers.cpp.
- crunch: "JeffCrunch's text import is the front end of the same code" and its merge-source reader "gives way to JeffCrunch's readers". base: "JeffCrunch's readers, which JeffPub's merge-source reader gives way to". That makes JeffCrunch's readers something JeffWrite, JeffPub, and JeffMail call, which the layering forbids when they sit in `apps/crunch`.
- crunch and base also each list a "data-source layer" (text, CSV, JSON, XML, HTML tables, SQLite, ODBC, and more) that overlaps with the merge-source library.

Recommendation: one place, `libs/office/data`, with one record-source interface (fields and rows) under both the merge-source library (B18) and the data-source layer (B19). The spreadsheet readers it needs are in `libs/office` (see D14). JeffWrite builds B18 first, JeffCrunch builds B19 and the step list, and tiers are 1.0.

### D8. Calculation engine and number-format language: one item or two, and when

- crunch lists two items: the calculation engine (whose description includes "the number-format language", needed by JeffCrunch, JeffWrite, JeffDeck, and JeffBase) and the number-format language (needed by all). base joins them in one item. write mentions the engine only inside its table engine item.
- crunch and base name JeffDeck as a user of the engine ("the table formulas of JeffWrite and JeffDeck"), but deck has no formula item. The only JeffDeck use is the sheet behind a chart.
- Timing: write's field engine (first release) includes formula fields, and its merge and date fields need date and number formats, but JeffCrunch builds the language and comes second. A private formatter already sits in `libs/office/office/chart.cpp`.

Recommendation: keep two rows (B1 and B2) in `libs/base`. JeffCrunch owns both, but B1 (formats, dates, parsing) is built at the start of JeffWrite's field work and the rest of the engine follows with JeffCrunch. Drop JeffDeck from the engine's needed-by list or have deck add the item. The chart formatter is replaced by B1.

### D9. Field engine: tier and scope

- Tier: pub later; deck 1.0; write and mail first release.
- Scope: write includes cross-references and formula; mail only dates, merge fields, and properties; deck slide numbers, dates, and properties. All say "needed by: all".
- write, Already there, counts "fields" under libs/text; the code has the field model (A11) but no engine for cross-references or formulas.

Recommendation: JeffWrite builds B9 in `libs/text`, with its first-release set as the base. JeffDeck's slide-number fields and JeffMail's subset use it. crunch (the `&P` header and footer codes) and base (`=[Page]` and `=Now()` in report headers) also need page and date codes, and neither names the field engine.

### D10. Format pane, Selection pane, and JeffBase's property sheet control

- pub: later; JeffPub has a Format Object dialog that works. deck: 1.0, a Format Shape pane. write: the Format Shape pane at first release and the Format Picture pane at 1.0, in its Contextual tabs, but not in its shared section. crunch: a Selection Pane and chart Format Selection pane items, also not in its shared section.
- base: "A property sheet control (a tabbed list of named properties with editors for colors, fonts, expressions, and lists)" needed by JeffBase only, and "JeffPub's and JeffDeck's panes can adopt it". That is a second pane with the same editors, with a different first builder.

Recommendation: one pane frame and one set of property editors in `libs/shell`, with the pages in `libs/page` (B38). JeffWrite builds it first, because its first release has the pane. JeffBase's property sheet is the same editors in a tabbed list and does not get its own copy. Add B38 to write's and crunch's shared sections.

### D11. Accessibility checker: who builds, and who needs it

- pub: later, "needed by: all"; deck, mail, and base: 1.0. mail says "(shared: JeffWrite's checker)", so it expects JeffWrite to build it; deck says "built once".
- write (Check Accessibility and its pane) and crunch (Check Accessibility) both have the feature at 1.0 but no shared-section item.
- Needed-by lists differ: mail lists four apps, base lists five, pub and deck say "all".

Recommendation: JeffWrite builds B30, in `libs/shell`, with each app registering rules for its own objects. Add it to write's and crunch's shared sections, and use "all six" everywhere.

### D12. File protection, macro projects, and embedded fonts

- deck lists "Properties, Inspect, Mark as Final, and Encrypt with Password ... and the Open XML encryption" and "Keeping a macro project untouched" as shared (needed by JeffWrite, JeffCrunch, and JeffDeck). pub lists the macro item at later.
- write and crunch each have their own items for encrypted files (write: `.docx`, `.odt`, and `.doc` with RC4 and XOR; crunch: agile AES and `.xls` RC4 and XOR) and macro-enabled files, and none is in their shared sections.
- Inside write: "Open and save macro-enabled files (`.docm`, `.dotm`)" is 1.0, but "Save as `.docm` and `.dotm`" is later. Inside crunch: open `.xlsm` and "save them back the same way" is 1.0, but "Save as `.xlsm` and `.xltm`" is later.
- write, Decided: "Embedded fonts: follow JeffDeck's rule". No list has a shared item for saving embedded fonts, and the code is in `apps/deck`.

Recommendation: B14, B15, and B16 in `libs/office`. JeffWrite builds B14 and B16, JeffDeck builds B15. Add all three to the shared sections of write and crunch, and make the two tiers inside write and inside crunch agree.

### D13. OpenDocument readers and writers

- write ("shared: JeffDeck's ODF reader for styles, shapes, and charts" and "JeffDeck's ODF writer") and crunch (the same two notes) treat JeffDeck's ODF code as shared.
- deck's shared section does not list it, and says `.odp` is done. The code is in `apps/deck/io` (odpimport*.cpp, odpexport*.cpp), which JeffWrite and JeffCrunch cannot link.
- Tiers: write first release (open `.odt`); crunch first release (open `.ods`), 1.0 (save); deck done.

Recommendation: B12. JeffWrite builds it first by moving the style, shape, and chart parts into `libs/office/odf`. Add an item to deck's shared section saying it will move.

### D14. Readers used across apps live in apps today

- mail: previews and attachment search use "libs/office and the apps' file readers" (Preview Word, Excel, and PowerPoint files). base: spreadsheet import is "JeffCrunch's readers" and export is "JeffCrunch's workbook writer". pub: "Insert File from `.doc` and `.odt` (needs the readers JeffWrite will build)". crunch, Already there: "JeffCrunch adds the spreadsheet parts (`xl/`) beside [libs/office]" with no place named. deck: the presentation parts are in `apps/deck/io`.
- An app never uses another app. Today JeffPub has its own `.docx` reader for Insert File in `apps/pub/io`.

Recommendation: B13. Each file family gets a folder in `libs/office`, and a reader there makes either the shared model (the flow model, the placed-object Document, a cell table) or a read-only content view (text, values, pictures). Apps keep their windows, commands, and editing models. JeffMail's previews, JeffBase's import, and JeffPub's Insert File call the library.

### D15. HTML: "JeffPub's web export" is a set of pictures

- write ("Open an HTML page as a document, and save a document as HTML") and crunch ("Open an HTML page or a saved web page's tables as a workbook, and save a sheet as HTML") both say "shared: JeffPub's web export".
- pub: the web export is "a single HTML file" in which "each page is a picture, so its words cannot be selected".
- mail plans a clean HTML export with a plain-text twin and a safe HTML renderer (first release), and says JeffWrite will use the renderer for HTML import.

Recommendation: B17 in `libs/office/html`, built first by JeffMail. JeffWrite and JeffCrunch use it for their HTML items. Remove "JeffPub's web export" from their notes. JeffPub's picture-based export stays in `apps/pub`.

### D16. MIME, Share by email, and who is "all"

- pub (later) and mail (first release and 1.0) both list the MIME library and Share by email, with the same builder (JeffMail).
- The MIME library's needed-by list is JeffMail, JeffPub, JeffWrite, JeffDeck, and JeffCrunch; Share by email says "all", which includes JeffBase.
- write, crunch, and deck each cite "shared: JeffPub's `.eml` export" as shared, but that code is `apps/pub/app/sharing.cpp`.

Recommendation: JeffMail builds B3, then B29. Both are needed by all six. The three notes in write, crunch, and deck point at B3 and B29 instead of JeffPub's code.

### D17. Proofing dialogs are bound to the page Editor

- write, deck, and base list the libs/page proofing dialogs as shared; crunch uses "the spelling service in libs/shell"; mail plans a Spelling and Grammar dialog (F7) and a word count that it takes from JeffWrite.
- `libs/page/app/proofingdialogs.h` says its dialogs "work on an Editor". The grid (JeffCrunch), the flow editor (JeffWrite, JeffMail), and JeffBase controls have no page Editor.

Recommendation: B27. JeffWrite builds the "text to proof" interface in `libs/shell`, and the page Editor's version becomes an adapter. Harper (B26) sits behind the same spelling service.

### D18. Themes and building blocks: working code in JeffDeck and JeffPub, no builder named

- Themes: pub later; deck, crunch, and base 1.0; write has theme items at 1.0 ("shared: libs/office theme, JeffDeck's themes") but no shared-section item. deck's themes are done, in `apps/deck/core`.
- Building blocks: pub later (its own Building Blocks work today); write and mail 1.0; deck, crunch, and base have no item, though deck has slide layouts and write's item names them.

Recommendation: B22 and B32, built first by JeffWrite (its Design tab and Quick Parts), moving JeffDeck's and JeffPub's code. Add themes to write's shared section.

### D19. Small pieces whose needed-by lists and tiers disagree

- Ink layer: deck lists JeffWrite, JeffDeck, and JeffCrunch; write lists JeffWrite and JeffDeck; crunch lists JeffWrite, JeffDeck, and JeffCrunch; mail lists JeffWrite, JeffDeck, and JeffMail. All say later. The union is four apps.
- Equation editor: deck later; write and crunch 1.0. crunch says "to build for JeffWrite and JeffDeck".
- Text to speech: mail says later and needed by JeffWrite, JeffDeck, and JeffMail; write has Read Aloud at 1.0; deck has no Read Aloud item; crunch has Speak Cells (later).
- Capture: deck lists microphone, camera, and screen capture as needed by JeffDeck only; write, crunch, mail, and deck each list Screenshot and Screen Clipping (later).
- Secret store: mail lists JeffMail and JeffCrunch; JeffBase's list has server connections (ODBC, PostgreSQL, and MySQL) that need stored passwords, and JeffBase is not named.

Recommendation: B7 goes to JeffDeck (the pen exists there) with all four as users. B10 goes to JeffWrite. B33 goes to JeffWrite (earliest need), with JeffCrunch added and JeffDeck dropped from mail's list unless deck adds Read Aloud. B34 lists all four apps as users. B6 adds JeffBase.

### D20. SQLite access and the data-source layer

- base: "A SQLite layer with the safety settings above, the Access-to-SQLite SQL translator, and the Access functions registered as SQLite functions" (first release, needed by JeffBase and JeffCrunch).
- crunch: the data-source layer includes SQLite (1.0), and its "From a database" import is separate.
- Both would write the first SQLite wrapper.

Recommendation: one SQLite connection in `libs/office/data` (B20), built by JeffBase, since the safety settings are the hard part. JeffCrunch's data-source layer (B19) calls it. If JeffCrunch ships first, it builds the read-only part and JeffBase adds the translator on top.

## 5. Keeping the map current

When a piece in section 3 lands, move its row to section 2, change the box in each list item it served, and update the counts in the README. When a list's shared section changes, change the row it refers to.

Last reviewed October 16, 2026.
