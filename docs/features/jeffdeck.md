# JeffDeck feature list

JeffDeck is the presentation app of JeffOffice, modeled on PowerPoint in Office 2021: it saves `.pptx` by default, opens `.pptx`, `.potx`, `.ppsx`, `.ppt`, `.pps`, `.pot`, `.odp`, and `.otp`, and also saves `.potx`, `.ppsx`, `.odp`, and `.otp`. It is a preview (it shipped in 0.7.0 and again in 0.8.0), so unlike the lists for the apps not built yet, this one describes a program that exists, a box is checked only when the feature works in JeffDeck today, and every item carries a status checked against the code, the tests, the ribbon file, and Help on October 15, 2026 (see [docs/jeffdeck.md](../jeffdeck.md) for the design and status notes, which this list does not replace). It is laid out the way the original is (File, then each ribbon tab, the slide show, the tabs that appear only while something is selected, then what sits under the ribbon), so that the work can be planned and tracked, and so that it can settle one question: when does JeffDeck stop being a preview?

Last reviewed October 16, 2026.

Legend: `[x]` done · `[~]` partly there (the note says what's missing, or which shared JeffOffice part it builds on) · `[ ]` not yet · **(alt)** the usual way depends on a proprietary online service, so the app offers an open replacement.

Every item carries a tier:

- **1.0**: needed before JeffDeck is called finished (for JeffDeck, the items beyond what it needs to drop its Preview label).
- **later**: wanted, but not before 1.0.

## How the tiers were chosen

Dropping the Preview label needs the old `.ppt` fidelity gaps fixed (the first four gaps under Fidelity and testing, measured on decks never used for tuning) plus five basics: a right-click menu on slides, Find and Replace, picture cropping, table editing tools, and slide master editing (decided October 16, 2026). The items that make up those five basics are marked "(needed to drop Preview)".

There is no "first release" tier here, because JeffDeck already shipped. Version 1.0 asks for more: it is ready when PowerPoint opens every file JeffDeck saves (met so far for all 64 reference presentations) and JeffDeck's slides match PowerPoint's own pictures about as closely as JeffPub's pages match Publisher's (see Fidelity and testing, where JeffDeck is close on average and not yet on the median). A finished item is usually **1.0** too, since it counts toward the bar for version 1.0. An item is **later** when few presentations use it, when it needs a piece the suite doesn't have yet, or when it depends on something outside the program.

How the evidence reads. A count reads "ours 4 of 19": the number of decks among our 19 real `.pptx` files that use the feature. Our reference collection is 54 real presentations (1,028 slides: 35 old `.ppt` and 19 `.pptx`) plus PowerPoint's 10 built-in designs; the 64 are the ones every score below is measured on. Only the `.pptx` were counted, by reading their XML (390 slides); the `.ppt` are binary and were not, and the held-out test set was not opened. Nothing is counted yet for public presentations: the 40 old `.ppt` files from the U.S. government collection Govdocs1 were used only to measure fidelity, so **public** figures do not exist yet. Four cautions:

- The collection is old. 52 of the 54 real presentations are 4:3 (the other two are 16:9), 12 of the 19 `.pptx` have speaker notes, and almost none use the newer features (SmartArt, equations, sections, comments, custom shows, embedded fonts, and macros all read 0 of 19). A zero here means "not in the sample", not "nobody uses it".
- Counts of a feature are decks, not occurrences: "ours 3 of 19" means 3 decks, and the item says how many uses when that helps. Where a count could be thrown off (for example, an old `.ppt` that PowerPoint converted writes empty timing parts), the item says which figure is trusted.
- Slides, not presentations, are the unit of the fidelity scores; the two are kept apart below.
- The README's "Not yet in the preview" list is older than the code in places: it still names text shrunk to fit its box, a picture's brightness and contrast, and some text shadows as gaps, and recent work on all three is in the tests (see Fidelity and testing). This list follows the code.

## Beyond PowerPoint

What JeffDeck offers that the original doesn't, then the plans that go past PowerPoint.

- [~] **1.0** Runs on Linux and Windows, where PowerPoint 2021 ran only on Windows and Mac, and on macOS as the rest of the suite does *(shared: the suite's builds, installers, and AppImage; the macOS app is still a test build)*
- [x] **1.0** Free and open source (GPL-3.0): no license fee, no subscription, no account or sign-in, nothing that has to be online
- [~] **1.0** Installs without administrator rights, or runs with no installation (Windows `.zip`, Linux AppImage) *(shared: the suite's packaging)*
- [x] **1.0** Nothing in a file is lost: what JeffDeck can't edit yet (charts, SmartArt diagrams, equations, animation effects it has no form for, transition effects it has no form for, and parts it doesn't know) is kept with its slide and written back whole; a duplicated slide keeps them too
- [x] **1.0** Bundled open fonts, with stand-ins that keep the widths of Calibri, Arial, and the other common fonts, so a slide wraps where the original wraps on a computer without Microsoft's fonts
- [x] **1.0** Embedded fonts that cannot clash: a saved `.pptx` carries the bundled fonts its text uses (never a font everyone has, never one whose license forbids it), and the fonts a file carries are used only while that presentation is open, under private names, so they never replace another presentation's font
- [x] **1.0** Nine original themes (JeffOffice, Harbor, Ember, Almanac, Confetti, Granite, Orchard, Marquee, and Tideline), each with four variants, drawn by JeffOffice, with no Microsoft artwork
- [x] **1.0** **(alt)** Insert > Pictures > Online Pictures searches Openverse and Wikimedia Commons, keeps the title as alt text and adds a credit line when asked, in place of Microsoft's stock images and Bing; nothing from the presentation is sent
- [x] **1.0** Insert > Icons: more than 1,500 open vector icons, searched by name, drawn in the theme's text color, and saved both as a picture and as an SVG
- [x] **1.0** Kept ink: pen and highlighter strokes from a slide show become ordinary freeform shapes on their slides (one Ctrl+Z removes them all), so other programs show them and they can be edited like any shape
- [x] **1.0** Help and the interface offline, in English, German, Spanish, French, Italian, Dutch, and two forms of Portuguese; usage statistics and crash reports only for people who opted in, and commands counted by id, never by name
- [x] **1.0** `jdecktool`, a command-line tool that draws each slide to a picture, lists a file's masters, layouts, and slides, reads and saves a file again while listing what was left out, prints to PDF, lists a slide's animation steps, and draws a morph at five points along the way, as `jpubtool` does for JeffPub
- [~] **1.0** Spelling works offline in the bundled dictionaries, with no cloud service *(shared: the dictionaries in resources/dict; JeffDeck has the dialog and spelling as you type, no Set Language command)*
- [~] **1.0** PDF with real text and embedded fonts; bookmarks for each slide title, tags for screen readers, and PDF/A are not there yet *(shared: JeffPub's PDF export)*
- [ ] **later** Slides from a plain-text or Markdown outline (paste or open one, get slides), and an outline written back as Markdown, beside the Outline view that already exists
- [ ] **later** A self-contained web page of the slides, with the transitions and click steps, that plays in any browser
- [ ] **later** **(alt)** Present on the local network: the audience opens a link in a browser on the same network and follows the show, with no account, in place of Present Online and Broadcast (see the open questions)
- [ ] **later** **(alt)** Live captions in the show from speech recognition that runs on your own computer, in place of the original's online subtitles (model size and license to check)
- [~] **later** A QR code or barcode put on a slide as a vector picture *(shared: libs/base/core barcode; no command in JeffDeck yet)*

## File (backstage)

JeffDeck's File screen has New, Open, Save, Save As, Print, Export, Close, Options, and About. It has no Info, Share, or Publish page yet.

### Info
- [~] **1.0** Presentation properties: title, author, subject, keywords, category, comments, company, and manager, with the dates created, modified, and printed *(the title is read and used for the PDF; no window shows or edits any of them)*
- [ ] **1.0** Custom properties, kept as saved *(ours 2 of 19)*
- [ ] **1.0** Protect Presentation: Mark as Final (opens read-only with a note), Encrypt with Password (open and save a password-protected `.pptx` with the standard's strong encryption), and Always Open Read-Only
- [ ] **1.0** Inspect Presentation: find and remove comments, notes, personal information, hidden slides, content off the slide, and custom XML before sharing *(hidden slides: ours 0 of 54; notes: ours 12 of 19)*
- [ ] **1.0** Check Accessibility (see Review)
- [~] **1.0** Check Compatibility: names what an older or other format can't hold before saving *(after saving an `.odp`, a message lists what was left out; nothing is checked ahead of time, and nothing for `.ppt`)*
- [ ] **1.0** **(alt)** Version history without a cloud service: named local copies of a presentation, compared and restored from Info
- [ ] **1.0** Show a presentation's folder and copy its path
- [ ] **later** Compress Media and Optimize Media Compatibility
- [ ] **later** Add a digital signature with a certificate on your computer

### New
- [x] **1.0** Blank Presentation (16:9, the first theme, nine layouts)
- [ ] **1.0** Template gallery with live previews: the nine built-in themes, each with its variants, shown as places to start (they are only in the Design tab now), and your own templates under My Templates
- [x] **1.0** A presentation opened from a template (`.potx`, `.otp`) is saved as a new presentation, so the template stays as it was
- [ ] **1.0** More built-in templates written by JeffOffice and open-licensed: a lesson, a report, a pitch, a photo album, and a training deck
- [ ] **1.0** The default for new presentations: set the theme, fonts, and slide size every new presentation starts with
- [ ] **later** Photo Album (Insert > Photo Album) and templates found online

### Open, Save, Save As, and Close
- [x] **1.0** Open with Ctrl+O, Browse, the Pinned and Recent lists (pin, unpin, remove; a file that can't be found is gray); a file opens in the current window when it holds an untouched new presentation, and in a window of its own otherwise
- [x] **1.0** Save (Ctrl+S) and Save As (F12) with a kind chosen by ending (`.pptx`, `.potx`, `.ppsx`, `.odp`, `.otp`); a star in the title bar marks unsaved changes; Close asks first; the old file is kept when the new one can't be written
- [x] **1.0** A file that is not entirely readable opens with the rest and lists what it left out; a file that can't be read at all says why
- [x] **1.0** AutoRecover every 10 minutes for each presentation with unsaved changes, offered again after a crash, and Recover Unsaved Work on the Open page
- [~] **1.0** Open and Repair: recover what can be read from a damaged file *(a damaged `.pptx` or `.odp` opens with what can be read, as above; a badly damaged `.ppt` is refused; there is no separate Repair command)*
- [ ] **1.0** Save a Copy (keeps working in the original) and Save as Template from the File screen
- [ ] **1.0** Open read-only, open as a copy, and open in a new window
- [ ] **1.0** Open an encrypted presentation by asking for its password
- [ ] **1.0** Open and save the macro-enabled kinds (`.pptm`, `.ppsm`, `.potm`), keeping the macro project untouched and never running it *(macro projects: ours 0 of 19)*
- [ ] **1.0** Save As lists `.ppt`, `.pps`, and `.pot`, with a warning of what they can't hold; until then Save on an old file asks for a new name (see File formats)
- [ ] **1.0** A warning before saving to a kind that can't hold something in the presentation, naming what would change, with the choice to save a `.pptx` as well
- [ ] **later** Backup copy kept on every save
- [ ] **later** Protected View: files from the Internet open for reading only until you choose to edit them
- [ ] **later** Open from a web address and save back

### Print
- [x] **1.0** Print page with a preview drawn the way the page is printed, and with page navigation; choices are remembered; Ctrl+P opens it
- [x] **1.0** Print what: Slides (one per page, in the slide's shape), Notes Pages, Outline, and Handouts with 1, 2, 3, 4, 6, or 9 slides per page (lines beside each slide for 3), with the order across or down for 4 or more
- [x] **1.0** Range: all, the current slide, or slide numbers and ranges such as 1-3, 5; hidden slides only when asked; a thin frame around each slide; grayscale
- [~] **1.0** Printer, copies, collate, and one- or two-sided *(done in the system's print window, which JeffDeck opens; collate and two-sided are not on the Print page)*
- [ ] **1.0** Pure black and white, scale to fit paper, high quality, and printing comments and ink
- [ ] **1.0** Edit the header and footer of notes pages and handouts from the Print page *(the handout's header and footer hold the title, the date, and the page number, and can't be changed)*
- [ ] **later** Print several slides in a custom grid, and 4 slides per page with an Outline beside them
- [ ] **later** Duplex printing by hand: print the odd pages, then the even ones, with a prompt to turn the stack over

### Export and Share
- [x] **1.0** Export to PDF with the same Print what, Order, and Range choices, real text with fonts inside, the presentation's title as the PDF's title, and the page count shown before saving
- [~] **1.0** Export a slide as a picture: the current slide to PNG or JPEG, 1920 pixels across, on white *(not all slides at once, not other sizes, not GIF, TIFF, BMP, or SVG)*
- [ ] **1.0** Export all slides as pictures into a folder, and as a presentation of pictures
- [ ] **1.0** Create a Video: MP4 of the show with transitions, animations, rehearsed timings, and narration, in a size chosen *(the design notes put it after the first release; it is the one big output the preview lacks)*
- [ ] **1.0** PDF extras: bookmarks for each slide title, tags for screen readers, PDF/A, and PDF with a password *(shared: JeffPub's PDF export)*
- [ ] **later** Package the presentation: copy it with its linked movies and sounds and its fonts to a folder or `.zip` (the original packs a CD)
- [ ] **later** **(alt)** Email: a ready-to-send `.eml` with the presentation, or a PDF, attached, instead of sending through an online account *(shared: JeffPub's .eml export)*
- [ ] **later** Create handouts in a word processor file (`.docx`, `.odt`) once JeffWrite exists
- [ ] **later** Save as a single-file web page, and an outline as `.rtf` or Markdown

### Options
- [x] **1.0** General: user name and initials (used on comments), interface theme (system, light, or dark), display language, update checks and beta builds, and usage statistics (opt in) *(shared: the choices belong to every JeffOffice app)*
- [x] **1.0** Proofing: spelling as you type, ignore words in capitals or with numbers, AutoCorrect, smart quotes, and AutoFormat as you type (dashes, fractions, ordinals, automatic lists)
- [ ] **1.0** AutoCorrect Options window: the replacement list, exceptions, capitalization rules, and math symbols *(only check boxes now)*
- [x] **1.0** View: show the ruler, and snap objects to other objects
- [x] **1.0** Save: embed the bundled fonts the slides use, on by default
- [ ] **1.0** Save options: default format, AutoRecover interval, and default folder
- [ ] **1.0** Advanced: undo steps, the default slide show choices, print choices, cut and paste choices, and picture quality
- [ ] **1.0** Customize the Quick Access Toolbar *(fixed at Undo, Redo, and From Beginning today)* and the ribbon
- [ ] **1.0** Trust Center equivalents: what happens with linked files, linked media, and macros (kept, never run)
- [ ] **later** Customize the keyboard: assign keys to commands, with the original's shortcuts as the starting set
- [~] **later** Accessibility options: high contrast, sound and animation feedback, and keyboard focus *(follows the system theme; the ribbon has KeyTips)*

### About
- [x] **1.0** About: JeffDeck's and Qt's versions, the other JeffOffice apps, links to the source, downloads, and problem reports, and the license and notices

## Home tab

### Clipboard
- [x] **1.0** Paste, Cut, and Copy of text and objects, inside JeffDeck and to and from other programs (text, formatted text, and pictures), by the usual keys
- [~] **1.0** Paste options: keep source formatting, use the destination theme, paste as a picture, and keep text only, with Paste Special for the list of forms *(the editor can paste text only, but no command or key reaches it; formatted text from a web page keeps its formatting)*
- [x] **1.0** Format Painter for text and objects, once or, with Shift on the button, many times
- [ ] **1.0** Cut, copy, and paste whole slides in the slides pane and the Slide Sorter, with the choice to keep the source formatting or use the destination theme *(Duplicate Slide and Ctrl+D in the sorter exist; no slide clipboard was found)*
- [ ] **later** The Clipboard pane that keeps the last 24 items

### Slides
- [x] **1.0** New Slide (Ctrl+M) after the current slide with its layout, and a layout gallery on its arrow showing the nine built-in layouts (Title Slide, Title and Content, Section Header, Two Content, Comparison, Title Only, Blank, Content with Caption, and Picture with Caption)
- [x] **1.0** Layout: change a slide's layout; text stays, and each placeholder moves to the place the new layout gives its kind
- [x] **1.0** Reset: put the placeholders back where the layout has them, keeping their text
- [x] **1.0** Delete Slide, Duplicate Slide (with its notes, transition, and animations), and Hide Slide (skipped in the show, not printed unless asked; struck through in the pane) *(hidden slides: ours 0 of 54)*
- [x] **1.0** Sections: add, rename, remove (the heading only), remove with its slides, move up and down, collapse and expand all, and remove all; shown in the slides pane and the Slide Sorter, and saved in the file *(sections: ours 0 of 19)*
- [x] **1.0** Reuse Slides: a pane that shows another presentation's slides, one click adds a copy after the current slide, with or without the source's formatting
- [ ] **later** Slides from Outline: add slides from a text, `.rtf`, `.docx`, or `.odt` outline file

### Font
- [x] **1.0** Font and size boxes, Increase and Decrease Font Size, and Clear All Formatting
- [x] **1.0** Bold, Italic, Underline, Strikethrough, and Shadow (the shadow button gives a run the effect the original gives, and a second click takes it off) *(underline: ours 10 of 19)*
- [x] **1.0** Character Spacing from Very Tight to Very Loose, with More Spacing for a number of your own *(ours 7 of 19)*
- [x] **1.0** Change Case: sentence case, lowercase, UPPERCASE, Capitalize Each Word, and tOGGLE cASE
- [x] **1.0** Font Color: the theme's colors, more colors, and an eyedropper that picks a color from the screen
- [x] **1.0** The Font window (Ctrl+T, or the corner button): superscript, subscript, small caps, all caps, and the rest of the character choices in one place *(superscript: ours 4 of 19; subscript 0; small or all caps 1)*
- [~] **1.0** Text Effects: shadow, glow, reflection, inner shadow, and outline on letters *(shown, kept, and drawn as the original draws them, and the Shadow button sets a shadow; there is no Text Effects menu to choose the others; ours: effects on letters 9 of 19, an outline on letters 6)*
- [ ] **later** Text Highlight Color *(ours 0 of 19)*
- [ ] **later** Double strikethrough, equalized character height, and the other rare Font-window choices

### Paragraph
- [x] **1.0** Bullets (disc, circle, square, diamond, arrow, check mark, star, and dash) and Numbering (1. 2. 3., a. b. c., A. B. C., i. ii. iii., I. II. III., 1) 2) 3), and (1) (2) (3)), each with its Bullets and Numbering window *(bullets: ours 13 of 19; numbering 1)*
- [ ] **later** Picture bullets and your own bullet character *(picture bullets: ours 0 of 19)*
- [x] **1.0** Decrease and Increase List Level, taking the bullet, indent, and size the layout gives the new level
- [x] **1.0** Line Spacing (1, 1.15, 1.5, 2, 2.5, and 3) and the Paragraph window (indents, spacing before and after, and tab stops) *(spacing as a percentage: ours 12 of 19)*
- [x] **1.0** Align Left, Center, Align Right, and Justify, with Ctrl+L, Ctrl+E, Ctrl+R, and Ctrl+J
- [ ] **later** Distributed and the other justification forms
- [x] **1.0** Columns: one, two, or three in a text box *(ours 0 of 19)*
- [ ] **later** More Columns: a number of your own and the space between
- [~] **1.0** Text Direction: horizontal and rotated 90 degrees *(not rotated 270, not stacked; ours 0 of 19)*
- [ ] **later** Left-to-right and right-to-left paragraph buttons *(the shared editor can set a paragraph's reading direction; the ribbon has no button; ours 0 of 19)*
- [x] **1.0** Align Text to the top, middle, or bottom of its box
- [~] **1.0** Text fit: text too long for its placeholder shrinks to fit, and a file's saved fit is drawn exactly as the original draws it *(shrink on overflow: ours 14 of 19, and a shape that grows to its text 16 of 19; there is no choice between Do Not Autofit, Shrink Text on Overflow, and Resize Shape to Fit Text, and no text margins or wrap choice)*
- [ ] **later** Convert to SmartArt

### Drawing
- [x] **1.0** The Shapes gallery: lines, arrows, elbow and curved connectors, curves, freehand drawing, and shapes sorted by kind (also on Insert) *(preset shapes: ours 19 of 19; freeform 3; connectors 8)*
- [x] **1.0** Connectors that stay attached: an end snaps to a connection site on a shape and follows it
- [x] **1.0** Arrange: Bring to Front, Bring Forward, Send Backward, Send to Back, Group (Ctrl+Shift+G), Ungroup, Rotate Right 90, Rotate Left 90, Flip Horizontal, and Flip Vertical *(groups: ours 3 of 19)*
- [~] **1.0** Align objects (left, center, right, top, middle, bottom, and distribute) *(the shared editor has align and distribute; JeffDeck's Arrange menu doesn't offer them)*
- [ ] **1.0** Regroup, the Selection Pane (names, order, and hide, with a place to name an object so a Morph can pair it), and More Rotation Options
- [~] **1.0** Shape Fill: theme colors, more colors, the eyedropper, and No Fill *(a solid color only: no gradient, picture, texture, or pattern fill, which the engine draws; gradients: ours 3 of 19, a picture fill 1, patterns 0)*
- [~] **1.0** Shape Outline: color and No Outline *(no weight, dashes, or arrowheads, which are drawn; dashes: ours 2 of 19, arrowheads 3)*
- [ ] **1.0** Quick Styles for shapes and text boxes, drawn from the theme
- [ ] **1.0** Shape Effects: shadow, reflection, glow, and soft edges *(shadows are drawn, blurred as the original blurs them; ours: shadows 10 of 19, glow, reflection, or soft edges 2)*
- [ ] **later** Shape Effects: bevel and 3-D rotation, which are not drawn yet *(ours 1 of 19)*

### Editing
- [ ] **1.0** (needed to drop Preview) Find (Ctrl+F) in slide text, tables, and notes, with whole word and match case *(shared: JeffPub has a Find and Replace pane in apps/pub; JeffDeck has none)*
- [ ] **1.0** (needed to drop Preview) Replace (Ctrl+H) and Replace Fonts
- [x] **1.0** Select All (Ctrl+A) in an object, on a slide, and in the Slide Sorter
- [ ] **1.0** Select Objects and the Selection Pane (see Drawing)
- [x] **1.0** Undo and Redo: every change is one step that restores the whole presentation
- [ ] **later** Dictate

## Insert tab

### Slides
- [x] **1.0** New Slide with the layout gallery, and Reuse Slides (see Home)

### Tables
- [x] **1.0** Table: slide across a grid to pick rows and columns, or Insert Table with the numbers typed; Tab moves to the next cell, Shift+Tab goes back, and Tab in the last cell adds a row *(tables: ours 3 of 19, 6 in all)*
- [ ] **later** Draw Table (draw the rows and columns with a pen)
- [ ] **later** Excel Spreadsheet: a worksheet embedded in a slide that opens in JeffCrunch *(needs JeffCrunch)*

### Images
- [x] **1.0** Pictures from this device: one or more files, put in the middle of the slide no larger than most of it, and kept inside the presentation; dropping picture files on a slide puts each where it lands; PNG, JPEG, GIF, BMP, TIFF, WebP, SVG, WMF, EMF, ICO, and PDF are read *(pictures: ours 19 of 19, 728 in all)*
- [x] **1.0** **(alt)** Online Pictures (see Beyond PowerPoint)
- [ ] **later** Screenshot and Screen Clipping
- [ ] **later** Photo Album: a presentation made from a folder of pictures, with layouts and captions

### Illustrations
- [x] **1.0** Shapes (see Home), and Icons (see Beyond PowerPoint)
- [ ] **1.0** Chart: the gallery of types (column, bar, line, pie, doughnut, area, scatter, and the rest) and a small data grid that fills it *(a chart in a file is drawn from its saved values and kept whole; creating and editing one is not there; charts: ours 1 of 19, 5 charts)*
- [ ] **later** SmartArt: the gallery of layouts and a text pane, with the diagram laid out again as the text changes *(a diagram in a file is drawn from the drawing it saved and kept whole; ours 0 of 19; see the open questions)*
- [ ] **later** 3D Models: a model kept and shown, and turned in the show *(ours 0 of 19)*

### Links
- [x] **1.0** Link (Ctrl+K): turn selected text or an object into a link to a web page, to a place in the presentation, or to an email address *(links: ours 7 of 19 decks, 99 links, 81 of them to web addresses; none in the sample jumps to a slide or a file)*
- [ ] **1.0** Link to an existing file or a new document, and the Action choices: go to the next, previous, first, last, or last viewed slide, a slide, a custom show, a file, or a web address, on click or on pointing, and run a program or play a sound *(a link's targets other than a web page or email address do nothing in the show yet)*
- [ ] **later** Zoom: a Summary Zoom, Section Zoom, or Slide Zoom that jumps to a slide with an animated zoom

### Comments
- [x] **1.0** New Comment (also on Review)

### Text
- [x] **1.0** Text Box: drag a box on the slide and type; move, resize, and turn it
- [ ] **later** Vertical Text Box
- [x] **1.0** Header and Footer for slides: date and time, slide number, and footer text, with Don't show on title slide, Apply, and Apply to All (new slides get it too) *(footer placeholders, slide numbers, and dates in slides: ours 0 of 19)*
- [ ] **1.0** Header and Footer for notes and handouts: the Notes and Handouts tab with header, date, page number, and footer
- [~] **1.0** Text Art: a gallery of styles for fancy words, with the words changed in a box, shown and saved with its shadow *(JeffPub's gallery; not the original's WordArt styles for any selected text; no way yet to change the style or warp of Text Art once it is on the slide; WordArt warp in files: ours 0 of 19)*
- [x] **1.0** Date and Time: the date or the time put into the text being edited as a field
- [~] **1.0** Slide Number as a field in a text box *(the Header and Footer window sets slide numbers; no command puts the field into a box)*
- [~] **later** Object: an embedded object from another program *(an embedded object in a file is kept as a frame and shows its saved picture, as far as could be confirmed; ours 1 of 19, 2 objects; nothing opens it)*

### Symbols
- [~] **later** Equation: type or write math and see it set in type *(an equation in a file is shown as the picture the file keeps, in a box with other words too, and kept whole; it can't be changed; ours 0 of 19; follows the shared equation editor)*
- [x] **1.0** Symbol: a character that isn't on the keyboard

### Media
- [x] **1.0** Video on This Device and Audio on This Device: the file is kept inside the presentation, a movie shows its first picture, and a sound shows a speaker; JeffDeck says why before reading a file it can't play *(movies: ours 4 of 19, 10 in all; sounds 2 of 19, 10 in all)*
- [ ] **later** Online Video: a movie from a web page played in the show
- [ ] **later** Record Audio, and Screen Recording

## Draw tab

The Draw tab of Office 2021 is for ink on slides while editing. JeffDeck has ink only in the slide show (see Slide show), kept as shapes.

- [ ] **later** Draw with a mouse, pen, or touch on a slide while editing, with a Draw with Touch switch
- [ ] **later** Pens (pen, pencil, highlighter), their colors and thicknesses, and your own favorite pens
- [ ] **later** Erasers (stroke, point small, point medium) and Lasso Select *(the show's eraser takes a whole line)*
- [ ] **later** Ink to Shape, Ink to Text, and Ink to Math
- [ ] **later** Ink Replay: play back how a stroke was drawn
- [ ] **later** The ruler for drawing straight lines
- [ ] **later** Ink saved as ink in the file (the original keeps strokes as ink, not as shapes) *(shared: the ink layer; ours: ink in files 0 of 19)*

## Design tab

### Themes and Variants
- [x] **1.0** Themes gallery: nine themes shown as pictures of their title slides; a click applies one to every slide in one step; each slide moves to the layout of its own kind, text you set yourself keeps its look, and a color that would be hard to read moves just far enough to read (below 4.5 to 1)
- [x] **1.0** Right-click a theme: Apply to All Slides, Apply to Selected Slides (those slides get a slide master of their own; the same choice later reuses it), and Browse for Themes (take the theme of a `.pptx`, `.potx`, `.ppsx`, `.ppt`, `.pps`, or `.pot` file)
- [x] **1.0** Variants: four other looks of a theme, with Colors (the color schemes; each old color becomes its match everywhere), Fonts (heading and body pairs), and Background Styles (twelve: four colors, each flat and with two gradients), light or dark text following the background and links kept readable
- [ ] **1.0** Variants > Effects (the theme's effect styles), Create New Theme Colors and Fonts, and Reset
- [ ] **1.0** Save Current Theme and open `.thmx` theme files *(a theme comes today from a presentation or template file; ours: more than one master 2 of 19)*

### Customize
- [~] **1.0** Slide Size: Standard (4:3, 10 by 7.5 inches) and Widescreen (16:9, 13.333 by 7.5), with Ensure Fit scaling text, line weights, and layouts *(no Maximize; a file's other size is kept but can't be chosen; 4:3: ours 17 of 19 .pptx and 52 of 54 in all)*
- [ ] **1.0** Custom Slide Size: width, height, orientation (including portrait slides), the first slide number, and the orientation of notes and handouts
- [~] **1.0** Format Background: a solid color for this slide or all, applied to slides and layouts that had their own *(no gradient, picture, texture, pattern, or Hide Background Graphics, and no Reset Background; backgrounds are drawn)*
- [ ] **1.0** The Format Background pane (see Shape Format)

## Transitions tab

- [x] **1.0** Preview: plays the transition into the current slide; Esc stops it
- [x] **1.0** The gallery: None, Morph, Fade, Push, Wipe, Split, Reveal, Cover, Uncover, Zoom, and Cut (a cut goes through black), shown on the slide in the pane with a star *(transitions: ours 3 of 19, 67 in all; where an effect is named it is fade, wipe, dissolve, or cut)*
- [x] **1.0** Effect Options: the direction (from right, left, bottom, or top) for Push, Wipe, Reveal, Cover, and Uncover; Out or In for Split and Zoom; Objects, Words, or Characters for Morph
- [x] **1.0** Morph as the 2019 program writes it, with a fade for older programs: pairs the objects of two slides (a name starting with `!!`, the same name, the same text, the same picture, the same kind of shape, the nearest first), moves each pair's place, size, turn, fill, and line, fades what has no pair, and moves the words or the characters two texts share; a text over 16,000 characters, a turned or flipped box, and a slide of more than 1,000 objects play as Objects or a fade; a group and a table move whole and dissolve rather than change shape
- [ ] **1.0** Name an object (the Selection Pane) so that a `!!` name pairs two objects for Morph *(a `!!` name only comes from a file today)*
- [x] **1.0** Timing: Duration (0.01 to 59 seconds; 0.7 by default, 2 for Morph), On Mouse Click, After (seconds, and both together moves on at whichever comes first), and Apply To All
- [x] **1.0** A transition JeffDeck has no form for plays as a fade and is saved as it was *(an unnamed type in a file is kept)*
- [ ] **1.0** The rest of the Subtle set, the common ones from the 2007 program: Dissolve, Blinds, Checkerboard, Comb, Random Bars, Circle, Diamond, Plus, Wedge, Wheel, Newsflash, Strips, and the Shape and Uncover variants *(Dissolve is in 1 of 19 decks, 5 uses)*
- [ ] **later** The Exciting and Dynamic sets (Vortex, Ripple, Honeycomb, Glitter, Shred, Switch, Flip, Gallery, Cube, Doors, Box, Ferris Wheel, Conveyor, Rotate, Window, Orbit, Fly Through, Prism, Pan, Curtains, Wind, Airplane, Origami, Fracture, Crush, Peel Off, Page Curl, Fall Over, Drape, and Flash), each played in the show as the original plays it
- [ ] **later** Transition sound, and loop until the next sound
- [x] **1.0** A transition is one undo step with its timing; the apply-to-all form is one step too

## Animations tab

- [x] **1.0** Preview plays the slide's animations over the slide; Esc stops it
- [x] **1.0** The gallery: None; 8 Entrance effects (Appear, Fade, Fly In, Float In, Split, Wipe, Grow & Turn, and Zoom); 4 Emphasis effects (Pulse, Teeter, Spin, and Grow/Shrink); 8 Exit effects (Disappear, Fade, Fly Out, Float Out, Split, Wipe, Shrink & Turn, and Zoom); and 64 Motion Paths in 6 groups (Lines, Arcs, Turns, Shapes, Loops, and Curves) *(animations: ours 4 of 19, 126 effects, all Entrance and 109 of them Appear; no Emphasis, Exit, or path)*
- [x] **1.0** Effect Options: the direction (From Left, Clockwise, and the rest), the amount (Grow/Shrink from Tiny to Huge, and Both, Horizontal, or Vertical; Spin from a quarter to two spins), and the sequence (As One Object or By Paragraph, an indented paragraph going with the one above it) *(builds by paragraph: ours 0 of 19)*
- [x] **1.0** Add Animation gives a shape a second effect, started on a click
- [x] **1.0** The Animation Pane: each effect's click number or bracket, picture, shape name, start, and a timeline bar; pick one or several rows with the shape picked on the slide; drag to reorder; Play All and Play From; right-click for On Click, With Previous, After Previous, Effect Options, Timing, and Remove; numbers shown on the slide's shapes
- [x] **1.0** Animation Painter copies a shape's effects onto the next shape clicked
- [x] **1.0** Timing: Start (On Click, With Previous, or After Previous), Duration, Delay, and Move Earlier and Move Later
- [x] **1.0** Additional Effect Options: smooth start and end, Repeat, Rewind when done playing, Text Animation (All at once or By paragraph), and Hide after the effect
- [x] **1.0** Effects are saved as the original writes them, so it lists each by its own name; an effect JeffDeck has no kind for is kept as Custom (it can be moved, removed, and started another way, but its options can't be changed), and plays as a fade, a move, a turn, or a change of size, whichever it does
- [ ] **1.0** The rest of the common 2007 effects: Entrance (Bounce, Swivel, Wheel, Random Bars, Shape, Checkerboard, Blinds, Box, Diamond, Plus, Peek In, Dissolve In, Flash Once, Stretch, Expand, and Spiral In), Emphasis (Transparency, Darken, Lighten, Bold Flash, Flicker, Wave, Grow with Color, and Blast), and the matching Exit effects *(the reference decks' 126 entrance effects are of 8 kinds: 109 are Appear, and the other 17 are Fade, Circle, Wheel, Random Bars, Grow & Turn, and two forms of Zoom; Circle, Wheel, and Random Bars are not among JeffDeck's eight, and play as a fade)*
- [ ] **1.0** Triggers: start an effect when a named shape is clicked, and when a movie bookmark is reached *(read and kept; not played, not made; ours: triggers 0 of 19)*
- [ ] **later** Effects that change a color (Object Color, Complementary Color, Line Color, Fill Color, Font Color, and the color choices of Effect Options): kept and not played
- [ ] **later** Sounds on effects, After Animation (dim, hide on the next click), and Animate Text by word or by letter *(sounds and color changes are kept as the file has them)*
- [ ] **later** Build chart elements by series or category, and SmartArt by level
- [ ] **later** Effect Options > Change Animation and Build Order changes after the first choice

## Slide Show tab

- [x] **1.0** From Beginning (F5, and a button on the Quick Access Toolbar) and From Current Slide (Shift+F5, and a button on the status bar)
- [x] **1.0** Custom Slide Show: a list of your custom shows to play, and the Custom Shows window to make (New, with slides added, reordered, and removed), edit, copy, and delete them; a slide may be in a show twice; saved in the file *(custom shows: ours 0 of 19)*
- [x] **1.0** Set Up Slide Show: Presented by a speaker (full screen), Browsed by an individual (window), and Browsed at a kiosk (full screen, always loops, moves on only by timings); loop until Esc; show without animation; pen color and laser color; all slides, a range, or a custom show; advance manually or by timings; saved in the file *(kiosk or loop: ours 0 of 19; slides that advance by a time: 0 of 19)*
- [x] **1.0** Hide Slide, Rehearse Timings (a bar with Next, Pause, this slide's time, Repeat, and the total; offers to keep the times at the end), and Use Timings
- [x] **1.0** Monitors: Use Presenter View, and Presenter View (Alt+F5) to rehearse on one screen with the show in a small window
- [ ] **1.0** Show Presentation On: choose which screen shows the slides and which holds Presenter View, and swap them *(the screens are picked by where the window is)*
- [ ] **later** Resolution: the display size the show uses
- [ ] **later** Record Slide Show (see Record)
- [ ] **later** Play Narrations
- [ ] **later** **(alt)** Present Online (see the open questions)
- [ ] **later** Subtitles: show captions under the slides, in a language and place chosen

## Record tab

Office 2021 has a Record tab that records a show with the speaker's voice, ink, laser pointer, and, if wanted, the speaker on camera. JeffDeck has the rehearsal of timings (see Slide Show), and none of the rest.

- [ ] **later** Record Slide Show from the beginning or the current slide: narration, rehearsed timings, ink, and laser pointer saved on each slide, with the speaker's camera *(needs capture from the microphone and the camera)*
- [ ] **later** Clear Timings and Narration on the current slide or all slides
- [ ] **later** Play recorded narration and timings in the show
- [ ] **later** Record a video of the show to a file *(see Create a Video under File)*
- [ ] **later** Screen Recording and Record Audio on a slide
- [ ] **later** Export a recorded show as a video or as a show (`.ppsx`)

## Review tab

- [x] **1.0** Spelling (F7): a dialog that finds each misspelled word and offers suggestions, plus spelling as you type, ignoring words in capitals and with numbers, and AutoCorrect, in the bundled dictionaries *(shared: libs/page proofing dialogs and the dictionaries)*
- [ ] **1.0** Right-click a word marked as misspelled for suggestions, Ignore, and Add to Dictionary *(the slide has no right-click menu at all; see Shortcut menus)*
- [x] **1.0** Thesaurus (Shift+F7)
- [x] **1.0** Word Count: words on the slides, and in the selection when typing
- [ ] **1.0** Check Accessibility: a list of what a screen reader or a low-vision reader can't use (a missing alt text, a missing slide title, reading order, a table without a header row, low contrast, a movie without captions), with a task pane, and fixes by one click *(alt text exists in 14 of 19 decks; the canvas exposes its objects to screen readers)*
- [ ] **1.0** Set Proofing Language for the selected text or the whole presentation, and Language preferences *(the dictionaries are bundled for English, German, Spanish, French, Italian, Dutch, and Portuguese; it could not be confirmed how a text gets its language)*
- [ ] **later** **(alt)** Translate the selected text, a slide, or the presentation on your own computer, with an offline translation model (see Left out on purpose)
- [ ] **later** Smart Lookup and Insights
- [x] **1.0** New Comment (Ctrl+Alt+M): a pane beside the slide with a box for the text, a marker on the slide you can drag to move the comment, replies, Resolve and Reopen, edit and delete buttons, your name and initials asked for once
- [x] **1.0** Delete: the open comment, all comments on this slide, or all in the presentation; Previous and Next visit each comment on its slide; Show Comments hides or shows the pane and the markers; a presentation that comes with comments opens with them shown; Ctrl+Z takes any of it back *(comments: ours 0 of 19)*
- [x] **1.0** Comments saved in the `.pptx` with their replies, and read from both the older and the newer comment forms
- [ ] **1.0** Compare: merge another version of the presentation into this one and accept or reject each change, with a Reviewing Pane *(shared: the review model)*
- [ ] **later** Show Markup, Start Inking, and Hide Ink

## View tab

- [x] **1.0** Normal, Outline View, and Slide Sorter (the same buttons are on the status bar); each switch keeps the current slide
- [ ] **1.0** Notes Page: work on the notes in the shape of a printed page *(on the tab, not available)*
- [ ] **1.0** Reading View: the show in a window with a bar, for reading a presentation *(on the tab, not available; the Browsed by an individual show type exists)*
- [ ] **1.0** Slide Master, Handout Master, and Notes Master views (see the Slide Master tab)
- [x] **1.0** Show: Ruler, Gridlines, and Guides check boxes; Notes shows or hides the notes box under the slide
- [~] **1.0** Grid and guide settings: spacing, snap to grid, drag guides, and add or remove a guide *(Guides can be shown and objects snap to others; whether a guide can be dragged out of the ruler could not be confirmed)*
- [x] **1.0** Zoom: a menu from 400% to 10%, Fit to Window, a slider and buttons on the status bar, and Ctrl with the mouse wheel; in the Slide Sorter the same controls size the pictures
- [ ] **1.0** Color, Grayscale, and Pure Black and White views, and the choice of how each object prints in them *(grayscale applies to printing and PDF only)*
- [ ] **1.0** New Window (a second window of the same presentation), Arrange All, Cascade, Move Split, and Switch Windows *(each presentation opens in its own window; the system switches between them)*
- [ ] **later** Right-to-left slide view direction
- [ ] **later** Macros

## Help tab

- [x] **1.0** Help (F1, and the question-mark button): topics for each tab, pane, and dialog open on the page for what you are doing, offline, and translated
- [ ] **1.0** Tell Me: a box on the ribbon that finds a command and runs it *(Help topics have keywords; there is no command search)*
- [ ] **later** What's New, a link to a community page, and Report a Problem from the Help tab *(About has links to the source, downloads, and problem reports)*

## The slide show and Presenter View

- [x] **1.0** Full-screen show drawn by the same code as the editor, the print, and the PDF, so a slide shows as it prints; after the last slide a black screen says the show is over; hidden slides are skipped; the window returns on the last slide the show showed
- [x] **1.0** Move on by click or by Right, Down, Space, Enter, Page Down, or N; back by Left, Up, Backspace, Page Up, or P (one animation step back, or the slide before); Home and End; a slide's number then Enter; B or the period for black, W or the comma for white; Esc ends the show
- [x] **1.0** The right-click menu: Next, Previous, Go to Slide, Pointer Options (the tool and the ink color), Screen (black or white), and End Show
- [ ] **1.0** More of the show menu: See All Slides, Last Viewed, Show Presenter View, Zoom into the slide, Arrow Options (hide the pointer), Speaker Notes, and Subtitle Settings
- [x] **1.0** Animations play click by click: a click while a step plays finishes it and does no more; steps that start by themselves start when the slide appears; the next click plays the next step before the show moves on; Slide Show without animation shows slides whole
- [x] **1.0** Transitions play on the way forward onto a slide, Morph among them; Esc during one stops it
- [x] **1.0** Ink: Pen (Ctrl+P), Highlighter (Ctrl+I), Laser Pointer (Ctrl+L, or hold Ctrl and the mouse button), and Eraser (Ctrl+E), E to erase all the ink on the slide, Ctrl+A or Esc for the arrow, a color for each tool; ink stays on its slide while the show runs; at the end Keep puts each stroke on its slide as a shape and Discard leaves the slides as they were
- [x] **1.0** Movies and sounds play as the file says (in the click sequence, by themselves, or when clicked on; volume, trim, loop, rewind, full screen, hidden while stopped, across slides); the control bar shows when you point at a movie, and a click plays or pauses it
- [x] **1.0** A click on a link opens a web page or an email address
- [ ] **1.0** A click on a link to a slide, a custom show, another presentation, or a file, and on an action button (see Insert > Links)
- [ ] **1.0** Animations that wait for a click on a shape (triggers), and sounds with effects *(see Animations)*
- [x] **1.0** Presenter View on a second screen: the slide the audience sees, large (a click moves on); the next slide, or the same slide when steps remain; the notes, with A+ and A- for their size; a timer from the start with Pause (then Resume) and Reset; the time of day; the slide number; Previous and Next; Pen and Pointer; Black Screen; See All Slides (a grid; click one to go there); and End Slide Show; closing it ends the show
- [x] **1.0** Ink drawn in Presenter View appears on the audience's screen, and so does the laser dot
- [ ] **1.0** Edit the notes in Presenter View *(read only there; edited in the main window)*
- [ ] **later** Presenter View extras: zoom into the slide, swap the two screens, show the taskbar, and a strip of slide thumbnails
- [x] **1.0** The show of a presentation being edited: a Browsed by an individual window stays on its slide, follows the edits, and ends if the document is replaced
- [ ] **later** Run the show from a phone or another device as a remote (next, back, laser, and notes)
- [ ] **later** Live captions and the speaker's camera in a corner of the show *(see Beyond PowerPoint and Record)*

## Contextual tabs

These tabs are shown while something is selected. JeffDeck has one: Playback, while a single movie or sound is selected. The rest below do not exist yet. Most of their commands act on pieces the shared drawing engine already draws, so a good share of the work is a way to reach them (a tab, a menu, or a pane) rather than a new renderer; the notes say where that holds.

### Shape Format (shapes, text boxes, lines, and connectors)
- [ ] **1.0** The Shape Format tab itself, and a Format Shape pane with Fill & Line (solid, gradient with stops, picture or texture, and pattern fills; line color, width, compound type, dash, cap, join, and arrowheads), Effects (shadow, reflection, glow, soft edges), and Size & Properties (size, position, rotation, lock aspect ratio, and, for text, margins, wrap, columns, and autofit) *(the engine draws all of these; today only a solid fill color, an outline color, and the Shadow button are reachable)*
- [ ] **1.0** Insert Shapes and Change Shape (keep the text and formatting, swap the geometry)
- [ ] **1.0** Shape Styles: the Quick Styles gallery drawn from the theme, with Shape Fill, Shape Outline, and Shape Effects
- [ ] **1.0** WordArt Styles for the text in a shape: Text Fill, Text Outline, and Text Effects (shadow, reflection, glow, and transform) *(effects on letters are drawn and kept; only the Shadow button reaches them; WordArt warps in files: ours 0 of 19)*
- [ ] **1.0** Alt Text for any object, and mark as decorative *(an online picture gets its title as alt text; no way to see or change one; alt text in files: ours 14 of 19)*
- [ ] **1.0** Size: exact Height and Width, and the Size and Position window (position from the corner or center, rotation to an angle, lock aspect ratio) *(drag, arrow keys, and the rotation handle work)*
- [ ] **later** Edit Points for a freeform shape, and Merge Shapes (union, combine, fragment, intersect, and subtract)
- [ ] **later** Set as Default Shape, Text Box, or Line

### Picture Format (pictures, icons, and screenshots)
- [ ] **1.0** The Picture Format tab itself, and a Format Picture pane
- [~] **1.0** (needed to drop Preview) Crop with handles, Crop to Shape, Aspect Ratio, and Fill and Fit *(a crop in a file is drawn and saved, and a picture dropped in a picture placeholder is cropped to fill it from the middle; no tool crops, though JeffPub has one in apps/pub; ours: crops 7 of 19 decks, 227 pictures)*
- [~] **1.0** Corrections, Color, and Transparency: brightness and contrast, sharpen and soften, recolor, saturation, temperature, set transparent color, and picture transparency *(a brightness and contrast, gray, black and white, a color made clear, a duotone, a turned hue, and a tint are read, drawn as the original draws them, and saved as listed, with the picture's bytes untouched; none can be made; ours: any in 1 of 19, 4 uses)*
- [ ] **1.0** Change Picture (keeping size, place, and formatting) and Reset Picture
- [ ] **1.0** Picture Styles (frames, shadow, reflection, and rounded corners), Picture Border, and Picture Effects
- [ ] **1.0** Alt Text, Arrange, and Size (see Shape Format)
- [ ] **later** Artistic Effects (blur, pencil sketch, paint, and the rest) *(a picture's blur and fill overlay are neither drawn nor saved yet)*
- [ ] **later** Compress Pictures: a resolution for screen, print, or email, and delete the cropped areas
- [ ] **later** Remove Background: mark areas to keep and remove
- [ ] **later** Picture Layout: turn pictures into SmartArt
- [ ] **later** Graphics Format for icons and SVG pictures: Graphics Fill, Outline, and Effects, Change Graphic, and Convert to Shape *(an icon takes the theme's text color today)*
- [x] **1.0** Resize a picture at a corner to keep its proportions, Shift to stretch it freely, turn it by its handle, and drop one picture on another to swap them (Ctrl to copy)

### Table Design and Table Layout (tables)
- [~] **1.0** Tables from a file show and save as made, with rows that grow to their text *(the editing tools below are not there; rows that grow with typing could not be confirmed)*
- [ ] **1.0** Table Style Options (Header Row, Total Row, Banded Rows, First Column, Last Column, and Banded Columns) and the Table Styles gallery (Light, Medium, and Dark, from the theme) *(a table style id in files: ours 1 of 19)*
- [ ] **1.0** (needed to drop Preview) Shading, Borders (all, outside, inside, top, bottom, left, and right, none), and Draw Borders (pen style, weight, and color; Draw Table; Eraser)
- [ ] **1.0** (needed to drop Preview) Select (table, column, row, and cell), View Gridlines, Delete, and Insert Above, Below, Left, and Right
- [ ] **1.0** (needed to drop Preview) Merge Cells and Split Cells *(merged cells in files: ours 0 of 19)*
- [ ] **1.0** (needed to drop Preview) Cell Size: row height and column width by number and by dragging the borders, Distribute Rows and Columns
- [ ] **1.0** (needed to drop Preview) Alignment: align left, center, right, top, middle, and bottom, text direction, and cell margins
- [ ] **1.0** Table Size, Arrange, and Table Effects (shadow, reflection, cell bevel)
- [ ] **1.0** Cells copied from JeffCrunch or a document land as a table with fonts, fills, borders, and number formats *(shared: the table engine; see Shared)*

### Chart Design and Chart Format (charts)
- [x] **1.0** A chart in a file is drawn from the values saved with it, as a vector, and saved whole with all its parts; a duplicated slide keeps it; a copy pasted on a slide is only a picture *(charts: ours 1 of 19; types read: column, bar, line, pie, doughnut, area, and scatter; in an `.odp` a chart is read too)*
- [ ] **1.0** Edit Data and Select Data: a data grid for the chart's numbers, opened in place, with Switch Row/Column *(shared: JeffCrunch's grid and the chart editing library)*
- [ ] **1.0** Add Chart Element (axes, axis titles, chart title, data labels, data table, error bars, gridlines, legend, lines, and trendline) and Quick Layout
- [ ] **1.0** Change Colors, Chart Styles, and Change Chart Type (column, bar, line, pie, area, and scatter first)
- [ ] **1.0** Chart Format: Current Selection, Format Selection, Reset to Match Style, and the shape and text styles of the elements
- [ ] **later** Combination charts, 3-D charts, stock, surface, radar, treemap, sunburst, histogram, box and whisker, waterfall, and funnel charts *(read and kept; drawing the rarer types and editing them follow)*
- [ ] **later** Chart Elements, Chart Styles, and Chart Filters buttons beside the chart

### SmartArt Design and Format (diagrams)
- [x] **1.0** A SmartArt diagram in a file is drawn from the drawing it saved, treated as one object (selected, moved, resized, copied, and deleted as a whole), printed as vectors, and saved whole with all its parts, so the program that made it can still edit it *(ours 0 of 19)*
- [ ] **later** Edit the text of a diagram, and add, remove, promote, and demote shapes, with the diagram laid out again *(needs a layout engine for the layouts SmartArt uses; see the open questions)*
- [ ] **later** Layouts, Change Colors, SmartArt Styles, Reset Graphic, and Convert to Text or Shapes
- [ ] **later** A diagram with no saved drawing is drawn *(left unread today, since the file gave nothing to draw; the layout engine would draw it)*

### Video, Audio, and Playback (movies and sounds)
- [x] **1.0** The Playback tab (Media Tools): Play and Pause to preview, Trim (a start and an end time, the rest of the file staying inside), Volume (Low, Medium, High, or Mute), Start (In Click Sequence, Automatically, or When Clicked On), Play Full Screen, Hide While Not Playing, Loop until Stopped, and Rewind after Playing; each change is one step
- [~] **1.0** Play Across Slides *(a file's across-slides setting plays and is saved; no command sets it)*
- [ ] **later** Fade In and Fade Out durations, Add and Remove Bookmark (for triggers), and Closed Captions
- [ ] **later** Video Format: Poster Frame, Corrections, Color, Video Styles, Video Shape, Border, and Effects, and Reset Design *(a movie shows its first picture as its poster)*
- [ ] **1.0** Alt Text, Arrange, and Size for movies and sounds (see Shape Format)
- [ ] **later** Compress Media

### Equation, Ink, and Zoom tools
- [ ] **later** Equation tab: structures (fractions, scripts, radicals, integrals, large operators, brackets, functions, accents, limits, and matrices), symbols, and conversions between linear and professional forms *(shared: the equation editor)*
- [ ] **later** Ink Tools (Pens) while a stroke is selected, and Ink to Math
- [ ] **later** 3-D Model tools: Reset, Views, Pan and Zoom, and 3-D Model Views
- [ ] **later** Zoom tools: choose the Zoom image and Return to Zoom

### Slide Master, Handout Master, and Notes Master views
- [ ] **1.0** (needed to drop Preview) Slide Master view: the master and each layout opened and edited like slides, with a pane of them *(you can't edit slide masters or layouts yet; they are read, kept, written, and applied)*
- [ ] **1.0** (needed to drop Preview) Edit Master: Insert Slide Master, Insert Layout, Delete, Rename, and Preserve; more than one master per file *(more than one master: ours 2 of 19; 234 layouts in 21 masters across the 19 .pptx)*
- [ ] **1.0** (needed to drop Preview) Master Layout and Insert Placeholder: which placeholders a layout has and where (title, text, content, picture, chart, table, diagram, media, date, footer, and slide number), and Title and Footers check boxes
- [ ] **1.0** (needed to drop Preview) Edit the master's text styles: each level's bullet, font, size, color, indent, and spacing
- [ ] **1.0** (needed to drop Preview) Draw shapes and pictures on a master or a layout (a logo, a band, a line), and Hide Background Graphics on a slide
- [ ] **1.0** Edit Theme and Background groups in master view: Themes, Colors, Fonts, Effects, Background Styles, and Slide Size *(see Design)*
- [ ] **1.0** (needed to drop Preview) Close Master View
- [ ] **1.0** Notes Master view: the notes page's slide image, notes body, header, date, footer, and page number, its text styles, and its background *(ours: notes master in 12 of 19)*
- [ ] **1.0** Handout Master view: slides per page, orientation, header, date, footer, and page number, and its background *(ours: handout master in 3 of 19)*
- [~] **1.0** Notes with formatting: bold, italic, lists, links, and pictures in speaker notes *(notes are plain text; what happens to a formatted note in a file when it is edited could not be confirmed; notes with text: ours 12 of 19, 138 slides)*

## Shortcut menus (right-click)

JeffDeck has right-click menus in the slides pane, the Slide Sorter, and the show, but not yet on the slide itself.

- [x] **1.0** In the slides pane and the Slide Sorter: New Slide, Duplicate Slide, Delete Slide, Hide Slide, and Add Section; on a section heading: Rename, Remove, Move Up and Down, Collapse All, and Expand All; a theme, an Animation Pane row, and a file in the Open list have menus of their own, and so does the show
- [ ] **1.0** (needed to drop Preview) On an object on the slide: cut, copy, paste options, group and order, edit text, hyperlink, save as picture, size and position, format, alt text, and new comment *(the canvas offers the request and JeffDeck has no handler, so a right-click on a slide shows nothing)*
- [ ] **1.0** On text being edited: spelling suggestions, Ignore, and Add to Dictionary, with font, paragraph, bullets, link, and Look Up
- [ ] **1.0** On the slide's background: paste options, grid and guides, ruler, Format Background, Layout, and Reset
- [ ] **later** The mini toolbar that floats by selected text, with the font, size, bold, italic, color, alignment, and bullet buttons

## Under the ribbon: the rendering engine

- [x] **1.0** One drawing path for the editing window, the slide show, the slide pictures, and the print and PDF; vector shapes, text, and pictures all the way to the PDF *(shared: libs/page render, with the shape and text code of libs/draw and libs/text)*
- [x] **1.0** The standard's preset shapes with their adjust values, and freeform paths; connectors with elbow and curved routes *(preset shapes: ours 19 of 19, 2,257 in all; freeform 3)*
- [x] **1.0** Fills: solid, gradient (linear, radial, rectangular, and path) with stops, picture, texture, and pattern fills, with transparency; a shape may fill with the slide's background; an empty placeholder shows its layout's picture fill
- [x] **1.0** Lines: width, dashes, caps, joins, and arrowheads
- [x] **1.0** Shadows blurred as the original blurs them (a Gaussian a third as wide as the radius), the same on screen, in the show, and in a PDF, where a blurred effect becomes a 300 dpi picture
- [ ] **later** Soft edges, bevels, and 3-D rotation, which are not drawn *(bevel or 3-D in files: ours 1 of 19; soft edges, glow, or reflection on shapes 2 of 19)*
- [x] **1.0** Pictures: PNG, JPEG, GIF, BMP, TIFF, WebP, SVG, WMF, EMF, ICO, and PDF; large and hostile pictures held to budgets; identical pictures are found by their contents and kept once *(WMF or EMF in files: ours 1 of 19, 11 pictures; TIFF, BMP, or GIF 6 of 19)*
- [x] **1.0** Picture color changes drawn as the original draws them, in the order a file lists them (brightness and contrast, gray, black and white from a threshold, a color made clear or changed, one color for all, a duotone, a turned hue, and a tint), on pictures, picture fills, and backgrounds
- [~] **1.0** Cropped pictures, flips, and turns *(drawn and saved; a picture turned over is written as a half turn and a mirror in an `.odp`)*
- [x] **1.0** Charts drawn from their saved values as vectors (column, bar, line, pie, doughnut, area, and scatter), with a limit on points a picture can hold that says so when it is passed
- [x] **1.0** SmartArt drawn from the drawing a file keeps, once for every frame that shows it, with a limit on how many are drawn
- [x] **1.0** Equations shown as the picture the file keeps beside them; embedded objects shown as their saved picture
- [~] **1.0** Tables: borders, fills, and text drawn from the file, and rows grown to their text *(table styles by id could not be confirmed; ours 3 of 19)*
- [x] **1.0** Effects on letters (outer and inner shadows, a glow, and a reflection) drawn under the letters, read from the run and from the list styles, masters, and layouts it inherits from, and saved as listed
- [ ] **later** Text on a path and 3-D text *(Text Art keeps a curved shape; the original's transform warps on ordinary text are not drawn: ours 0 of 19)*
- [x] **1.0** A picture of each slide for the slide pane, the sorter, and the animation pane, redrawn as the slide changes

## Under the ribbon: the text engine on slides

- [x] **1.0** Text set as on a slide: a single line is 1.2 times the text size whatever the font, a wider line's extra space goes above it, the paragraph at the top of a box takes no space before it, an empty paragraph shows no bullet, text doesn't wrap around objects, and what doesn't fit runs out of its box instead of hiding; a baseline sits where the original puts it for spaced lines
- [x] **1.0** Text shrunk to fit (`normAutofit`) drawn as the original draws it: whole points, 1.2 times the size less the saved line reduction, indents, tab stops, letter spacing, exact line spacing, and space in points left as written, and space given as a share of the line following the size the text is drawn at; the same on screen, in the show, in a PDF, and in an `.odp`
- [~] **1.0** Text that fits a placeholder by shrinking as you type *(it shrinks; there is no menu choice among not fitting, shrinking, and resizing the shape)*
- [x] **1.0** Lists with the layout's bullets and levels: a new slide's text place takes its layout's first paragraph, and Increase and Decrease List Level take the layout's paragraph for the new level; an empty paragraph shows no bullet; bullet colors follow the colors
- [x] **1.0** Tab stops, columns, vertical rotation, justification, character spacing, kerning, small and all caps, super- and subscript, and underline, set and drawn
- [~] **1.0** Fields: the date, the time, and the slide number follow the slide *(the shared field engine is a planned piece; see Shared)*
- [ ] **later** Right-to-left and complex-script text with its buttons and settings *(the shared text engine does right-to-left; the ribbon has no buttons; ours: right-to-left paragraphs 0 of 19; East Asian or complex-script fonts named in 14 of 19 decks)*
- [x] **1.0** Hyperlinks inside text drawn in the theme's link colors, kept readable on any background, including on the bands and panels some layouts draw behind text; the words of a link stay inside the link when saved to an `.odp`
- [x] **1.0** Spelling marks as you type in a text being edited *(shared: libs/page editor with the bundled dictionaries)*

## Under the ribbon: themes and masters

- [x] **1.0** A theme is a slide master, its layouts, and a theme part; saved as theme, slide master, and layout parts, and read back as they were, in `.pptx` and `.odp` *(layouts: 234 across 19 decks)*
- [x] **1.0** Applying a theme copies its master and layouts in, moves each slide to the layout of its own kind, and sets text that followed the old layout in the new layout's look; picture placeholders keep their pictures (cropped to fill a picture place, fitted in a content place); a theme made for another slide shape is stretched, with circles and squares kept round and square
- [x] **1.0** Colors are plain colors on the objects, as files hold them; saving finds the theme's colors in them again; a change of colors swaps each old color for the new scheme's; text and shapes on a dark background use the four dark and light colors, so they trade places when the background turns light or dark; dark masters are saved with the dark color map
- [x] **1.0** Text on a picture, a table cell, or a shape with its own color does not turn light or dark with the slide's background; a color that would read below 4.5 to 1 moves toward light or dark just far enough
- [ ] **1.0** Theme effects, custom color and font schemes, and theme files (see Design)
- [ ] **1.0** (needed to drop Preview) Edit masters and layouts (see the Slide Master view)
- [x] **1.0** Footers: the date, slide number, and footer text placed from the Header and Footer window, applied to all slides and to new ones
- [x] **1.0** Slide size changes scale text, line weights, layouts, and placeholders to the new shape, and other objects keep their own shape and stay centered
- [ ] **later** Section Zoom and Slide Zoom targets, and the first slide number

## Under the ribbon: the animation and transition engines

- [x] **1.0** The timing tree of a slide is read, kept, and written as the original writes it, so it lists each effect by its own name; clicks, with, and after starts, delays, durations, repeats, smooth start and end, rewind, and hide after are all kept *(a timing part is in 17 of 19 decks, and real effects in 4 of 19)*
- [x] **1.0** Effects play as a fade, a wipe, a split, a move, a turn, a change of size, a pulse, a motion path, and a build by paragraph; others play as a fade; a damaged or hostile timing tree is read without harm and held to limits
- [x] **1.0** Edits write the tree again without an empty main sequence, keep the media and the numbers they name, keep effects on charts, tables, and frames, drop effects of shapes that are gone, and keep a deep or unknown effect as it was
- [~] **1.0** Interactive sequences (triggers) *(read and kept; not played, not made)*
- [ ] **later** Color effects and sounds played
- [x] **1.0** The transition engine draws the ten effects as frames between two whole slides, Morph included, with the click and timing rules of the show
- [x] **1.0** The Morph engine pairs objects, then moves, scales, turns, fades, and recolors them; the texts of a pair move by word or by character
- [ ] **later** The rest of the transitions, drawn frame by frame as the original draws them
- [x] **1.0** `jdecktool animate-demo` makes a presentation with every effect JeffDeck offers, and `steps` and `morph` draw the frames, so effects can be compared with the original's

## Under the ribbon: media playback

- [x] **1.0** Movies and sounds are played through Qt Multimedia (JeffDeck's packages carry the FFmpeg 7.1 libraries it uses), in place while editing and in the show: `.mp4`, `.mov`, `.avi`, `.wmv`, `.mkv`, `.webm`, `.mp3`, `.wav`, `.m4a`, `.wma`, `.ogg`, and `.flac`
- [x] **1.0** A file is never decoded until you ask: opening, saving, and printing use only the poster; a file the computer can't play says why, and the rest of the program carries on
- [x] **1.0** The play list of a slide (what plays, when, and how) is read from the timing tree and written back as the original writes it, whatever the edits
- [ ] **1.0** A movie or sound that the file only links to (a path on a disk or an address on the web) is played when the file can be found *(not played today; the link and poster are kept)*
- [ ] **later** Online video from a web page
- [ ] **later** Fades, bookmarks, captions, a chosen poster frame, and Compress Media
- [ ] **later** Record audio and screen recordings placed on a slide

## Fidelity and testing

JeffDeck is measured against the original's own pictures of each slide. PowerPoint on the Windows test machine draws every slide of the reference presentations; JeffDeck draws the same slides, and a score for each slide says how far apart the two pictures are (the average difference of the two blurred pictures in gray, at a small size, the scale JeffPub's comparison uses: lower is closer, and 2 is the line the percentages below use). The reference pictures stay on the build machine and never go in the repository.

| Set | Slides or files | Mean | Median | Under 2 |
| --- | --- | --- | --- | --- |
| Our 64 reference presentations (54 real ones, 35 of them old `.ppt`, and PowerPoint's 10 built-in designs) | 1,172 slides | 1.53 | 1.27 | 81% |
| JeffPub's own pages against Publisher's, for comparison | | 1.74 | 0.96 | 73% |
| 40 old `.ppt` files from the U.S. government collection Govdocs1, which JeffDeck was never tuned on | 40 files | | 1.77 | 57% |
| Newer `.pptx` files | 2 files only | | 1.08 | |

Reading it: on its own decks JeffDeck's mean and its share under 2 are already better than JeffPub's, and its median is not (1.27 against 0.96). On files it was never tuned on, the share under 2 drops from 81% to 57%, so the 64 decks flatter it. The newer files score well, and two files prove little. The gaps behind the Govdocs1 numbers are listed first, each one a 1.0 item.

- [ ] **1.0** Pictures that go missing on slides of old `.ppt` files (a fix is in progress)
- [ ] **1.0** A picture's white backdrop drawn see-through where the original draws it white
- [ ] **1.0** Wrong background colors on old `.ppt` slides
- [ ] **1.0** Title bars drawn with a fill where the original draws none
- [ ] **1.0** Score the Govdocs1 set again after those four fixes and write the new numbers here *(the bar is an open question)*
- [ ] **1.0** Test many more newer `.pptx` files, from PowerPoint 2010 through 2021 and from other programs, 16:9 ones among them, before trusting the 1.08 *(52 of the 54 real reference decks are 4:3, and so are 17 of the 19 real `.pptx`)*
- [~] **1.0** The bar on our own decks: a mean and a share under 2 at least as good as JeffPub's, and a median that is too *(mean 1.53 against 1.74 and 81% against 73% are met; the median, 1.27 against 0.96, is not)*
- [~] **1.0** Text the original shrank to fit its box, drawn as it draws it *(done for the reference decks and tested down to whole points and line reductions; the README still lists it as a gap; check again on the Govdocs1 set)*
- [~] **1.0** A picture's brightness and contrast, drawn as the original draws them *(the formulas are fitted to its own pictures and tested; the README still lists it as a gap; a picture's blur, alpha effects, and fill overlay are neither drawn nor saved yet)*
- [~] **1.0** Text shadows, glows, and reflections *(drawn as the original draws them and tested; the README still lists shadows as a gap; soft edges on words, bevels, and 3-D turns are not drawn)*
- [ ] **1.0** Charts compared with PowerPoint's picture of each, as JeffCrunch's will be *(charts: ours 1 of 19 decks; the check finds how many charts the 64 hold)*
- [ ] **1.0** Tables, SmartArt diagrams, and equations compared with PowerPoint's pictures the same way *(tables: ours 3 of 19; SmartArt 0; equations 0)*
- [ ] **1.0** Notes pages, handouts, and the outline printed to PDF compared with PowerPoint's own PDF of them *(only slides are measured now)*
- [ ] **1.0** Animations and transitions compared with PowerPoint's own frames *(the score looks at a still of each slide; effects are checked by the tests, not against the original)*
- [ ] **1.0** A repeatable slide comparison anyone can run: the scoring of the 64 was done on the build machine, and `tools/fidelity.py` in this repository scores `.pub` files only; add the slide version, made from `jdecktool render` and PowerPoint's pictures, and write the numbers into each release's notes
- [ ] **1.0** A list of the lowest-scoring tenth of slides after each release, each with its cause, kept until the median is under 1
- [x] **1.0** Fonts do not move text: bundled stand-ins keep the widths of Calibri, Arial, and the other common fonts, and a file's own embedded fonts are used while it is open
- [x] **1.0** Files JeffDeck saves open in PowerPoint on the Windows test machine (Office 2021) without a repair message *(met so far for all 64 reference presentations)*
- [~] **1.0** Decks held out from tuning are scored apart from the ones used for tuning *(the 40 old `.ppt` files from Govdocs1 are; the held-out test set of our own decks has not been opened)*
- [ ] **1.0** The public test collections that hold presentations (Apache POI's slideshow test files, and the rest of Govdocs1's `.ppt` files) run through the readers and the save-and-reopen check, each file that does not open or does not save back listed with its cause

## Safety and limits

Suite-wide rules decided October 11, 2026 (see Decided): nothing in a file runs or reaches out until the person allows it, every file reader is fuzzed and held to limits on memory and time, and a damaged file opens as far as it can without crashing.

- [x] **1.0** A damaged or hostile file never crashes the program: it opens what it can and lists what it left out, checked with random damage on `.ppt` and with hostile `.pptx` and `.odp` files
- [~] **1.0** Nothing in a presentation runs or reaches out until the person allows it: macros, links, linked media, and remote content *(macros are kept and never run, and a link opens only when it is clicked in the show; what happens with linked files and linked media is the Trust Center item in Options)*
- [~] **1.0** Every file reader is fuzzed and held to limits on memory and time *(random damage on `.ppt`, hostile `.pptx` and `.odp` files, and pictures held to budgets are done; limits on memory and time for every reader could not be confirmed)*

## File formats

What each format needs, to open, to save, and to come back unchanged. "Round trip" means open, save, and open again in the original program without a repair message or a visible change.

### PowerPoint `.pptx`, `.potx`, and `.ppsx` (Open XML)
- [x] **1.0** Open `.pptx` from PowerPoint 2007 to 2021 and Microsoft 365 (Transitional, and the Strict form too): slide sizes, masters, layouts, slides, notes, themes, text and its lists, tables, pictures, shapes, charts, SmartArt drawings, equations, comments, sections, transitions, animations, movies and sounds, and fonts that a file carries (plain, or squeezed the way PowerPoint squeezes them), with limits on how much one file may ask for
- [x] **1.0** Save `.pptx` as a complete package with its elements in the standard's order, the master's text styles and theme, and the effects the editor makes; PowerPoint opens all 64 reference presentations JeffDeck saved *(the 54 real ones, old `.ppt` among them, and the 10 designs)*
- [x] **1.0** `.potx` and `.ppsx` open and save as the same format in its template and show forms; a file opened from a template is saved under a new name
- [x] **1.0** Round trip keeps what JeffDeck can't edit: charts, SmartArt (links kept working when ids change), equations (both alternatives), animation effects it has no kind for, transition effects it has no form for, unread frames, and compatibility markup
- [~] **1.0** Round trip keeps custom XML, tags, drawing and control parts, the VBA project, and printer settings *(could not be confirmed one by one; custom XML in files: ours 1 of 19, drawing parts 1, tags 0, VBA 0)*
- [x] **1.0** Fonts in saved files: the bundled fonts the text uses, all characters, as an uncompressed Embedded OpenType part, on by default; fonts drawn in place of common ones are written as the common name; a font a file carries that JeffDeck doesn't have is written back while the slides still use it
- [ ] **1.0** Open and save `.pptm`, `.ppsm`, and `.potm`, keeping the macro project untouched and never running it *(ours 0 of 19)*
- [ ] **1.0** Open encrypted files and save with a password
- [ ] **1.0** A round-trip check on a large collection, as JeffPub's `.pub` files had: PowerPoint opens every saved file with no repair message, and the slides come back unchanged *(64 files so far; the Govdocs1 files and newer `.pptx` should join)*
- [ ] **later** Save the Strict variant, and Flat OPC (`.xml`) files
- [ ] **later** Open and save `.thmx` theme files (see Design)

### Old PowerPoint `.ppt`, `.pps`, and `.pot` (binary)
- [x] **1.0** Open PowerPoint 97 to 2003 files: masters and title masters, layouts, slides, notes, text and its formats, fields, links, ruler, transitions, shapes (with the drawing group's defaults, textures, shadows, and the writer's own XML), and pictures found without scanning the whole store; limits on huge text, the number of shapes, and the entries read
- [x] **1.0** A damaged file never crashes the program: random damage is survived, and a file that can't be read says why
- [x] **1.0** Save on an old file asks for a new name and saves a `.pptx` (a `.ppsx` for a `.pps`), never writing over the original
- [~] **1.0** Animations, sounds, movies, charts, and embedded objects of an old file *(transitions are read and tested; whether the binary animation records, linked or embedded media, and OLE charts are read could not be confirmed)*
- [ ] **1.0** Fonts embedded in an old file *(not read yet)*
- [ ] **later** Save `.ppt`, `.pps`, and `.pot` so PowerPoint 2003 opens them, with Check Compatibility naming what is lost (decided October 11, 2026, for the other apps: until then Save As offers a new `.pptx` name)
- [ ] **later** Open PowerPoint 4.0 and 95 files *(could not confirm that the reader takes them; the tests cover 97 to 2003)*

### OpenDocument `.odp` and `.otp`
- [x] **1.0** Open `.odp` and `.otp` (ODF 1.1 to 1.3, LibreOffice's and Apache OpenOffice's): sizes, masters, layouts, text and lists, objects, pictures, tables, charts (categories, series, scatter domains), and notes, with limits for hostile files
- [ ] **1.0** Read back everything JeffDeck writes: animations, comments, custom shows, and the show settings are in the file but listed as left out on opening *(they are written and LibreOffice plays them)*
- [x] **1.0** Save `.odp` and `.otp` as LibreOffice writes its own: a master page for each master and layout with graphics or a background, every slide with its objects, text with every property explicit, pictures, movies and sounds kept in the file, tables, notes, comments, transitions, animations, text shrunk to fit at the size it was shrunk to, footers, custom shows, and the show settings; LibreOffice opens all 64 reference presentations saved this way; a file opened as `.odp` is saved as `.odp`
- [~] **1.0** What an `.odp` has no place for: charts, SmartArt diagrams, and equations are saved as the pictures JeffDeck shows; sections, reflections, bevels, inner shadows, glows and reflections on words, effects that wait for a click on an object (a movie's are kept), and color effects are left out, and the message after the save lists them
- [ ] **later** Flat OpenDocument (`.fodp`)

### PDF, pictures, video, and web
- [x] **1.0** Export PDF of slides, notes pages, handouts, and the outline, with real text and fonts inside
- [ ] **1.0** PDF/A, tagged PDF, bookmarks from slide titles, links, and PDF with a password
- [~] **1.0** Export a slide as a picture (PNG or JPEG, 1920 pixels across) *(one slide at a time; no other kinds or sizes)*
- [ ] **1.0** Export all slides as pictures, GIF, TIFF, BMP, and SVG
- [ ] **1.0** Export a video (MP4) with the show's timings and narration
- [ ] **later** Export an animated GIF of the show
- [ ] **later** Open a PDF as slides, one page each *(a PDF is read as a picture on a slide today)*
- [ ] **later** Save as a web page, and the outline as `.rtf` or Markdown
- [ ] **later** XPS: not offered, since the format has no use outside Windows

### Pictures, media, and parts
- [x] **1.0** Pictures in presentations: PNG, JPEG, GIF, BMP, TIFF, WebP, SVG (with its PNG stand-in), WMF, EMF, ICO, and PDF, read and written; icons saved as a PNG and the SVG
- [x] **1.0** Movies and sounds kept inside the file with the calls that play them, in `.pptx` and `.odp`; linked ones keep their address and poster
- [ ] **later** Copy and paste of slides and objects to and from other programs in their clipboard forms

### What must survive a round trip
- [~] **1.0** A checklist for every format JeffDeck saves: slide size, masters, layouts, and themes; each slide's placeholders, text with its runs, lists, and fields; shapes, pictures, and their effects; tables, charts, SmartArt, and equations; notes and their master; comments with replies; sections; hidden slides; transitions and animations with their timing; movies and sounds with their trim and volume; custom shows and show settings; headers and footers; fonts; alt text; and the document's properties *(most are tested one by one; properties, alt text, and a notes master could not be confirmed)*

## Shared with other JeffOffice apps

The suite-wide map of shared parts, [shared.md](shared.md), says where each lives, which app builds it first, and who uses it; where this list disagrees with it, the map holds.

What JeffDeck builds on, and the pieces to build once so that JeffWrite, JeffCrunch, JeffMail, and JeffBase can use them too.

### Already there
- [~] **1.0** libs/page: the document of placed objects (each slide is a page, each layout a master page), its editor (undo as one step, selection, text editing, align and distribute, connectors that stay attached), canvas, proofing dialogs, options, Online Pictures, the icon picker, and the Text Art gallery, shared with JeffPub; Find and Replace and the crop tool still sit in JeffPub's own code
- [~] **1.0** libs/office: Open XML packages, themes, DrawingML shapes, text, and tables (read and written), charts (read from their stored values and drawn), the Escher drawing reader for `.ppt`, and the embedded-font readers; JeffDeck adds the presentation parts (apps/deck/io)
- [~] **1.0** libs/draw, libs/text, and libs/base: shapes, fills, effects, text art, SVG, picture effects, the text engine, colors, fonts, barcodes, zip and compound files, EMF and WMF, PDF, and the dictionaries
- [~] **1.0** libs/shell: the window, ribbon, KeyTips, file pages, Help, translations, AutoRecover, settings, spelling, updates, and statistics
- [x] **1.0** JeffDeck's own pieces in apps/deck/core: themes and their application, the animation model and its editing, sections, comments, slide reuse, ink, show settings, media, and embedded fonts

### To build once, for several apps
- [ ] **1.0** Move JeffPub's finished tools into libs/page and libs/shell so JeffDeck gets them: Find and Replace, the crop tool, and the paste choices (Keep Text Only) *(needed by: JeffDeck, JeffPub, JeffWrite)*
- [ ] **1.0** A Format pane (fill, line, effects, size and position, and text options) and the Selection Pane, built once *(needed by: JeffDeck, JeffPub, JeffWrite)*
- [ ] **1.0** Shape styles, picture styles, and text effects menus on the effects the engine already draws *(needed by: JeffDeck, JeffPub, JeffWrite)*
- [ ] **1.0** A table engine with merged cells, styles, borders, and sizes, and its Table Design and Table Layout tabs *(needed by: JeffWrite, JeffMail, JeffCrunch, JeffDeck)*
- [ ] **1.0** Chart editing on top of libs/office chart: the data grid, the element menus, types, and styles; a chart in JeffDeck keeps its data in a small embedded sheet edited with JeffCrunch's grid *(needed by: JeffCrunch, JeffWrite, JeffDeck)*
- [ ] **1.0** A review model for comments with replies, tracked changes, and comparison, so JeffDeck's comments, JeffWrite's tracked changes, and JeffCrunch's notes follow it, and Compare works in a presentation *(needed by: JeffWrite, JeffDeck, JeffCrunch)*
- [ ] **1.0** A field engine for slide numbers, dates, and document properties *(needed by: all)*
- [ ] **1.0** Themes, table styles, and cell styles in one format and gallery, so a JeffDeck theme can color a JeffCrunch workbook and a JeffWrite document *(needed by: all)*
- [ ] **1.0** An accessibility checker (alt text, titles and headings, reading order, tables, and contrast) with a task pane *(needed by: all)*
- [ ] **1.0** Properties, Inspect, Mark as Final, and Encrypt with Password on the file screen, and the Open XML encryption (open and save) *(needed by: JeffWrite, JeffCrunch, JeffDeck)*
- [ ] **1.0** Keeping a macro project untouched in the macro-enabled kinds, never running it *(needed by: JeffWrite, JeffCrunch, JeffDeck)*
- [ ] **later** An equation editor and layout for Office Math and MathML *(needed by: JeffWrite, JeffDeck, JeffCrunch)*
- [ ] **later** Embedded worksheets: Insert > Object in JeffDeck shows the saved picture and opens in JeffCrunch's grid in place *(needed by: JeffCrunch, JeffWrite, JeffDeck)*
- [ ] **later** An ink layer (pens, highlighter, eraser, ink to shape) built on the slide show's pen *(needed by: JeffWrite, JeffDeck, JeffCrunch)*
- [ ] **later** A SmartArt layout engine, to edit a diagram and to draw one a file saved without a drawing *(needed by: JeffDeck, JeffWrite)*
- [ ] **later** Capture from the microphone, the camera, and the screen, for Record Slide Show, Record Audio, and Screen Recording *(needed by: JeffDeck)*
- [ ] **later** A movie encoder for video export *(needed by: JeffDeck; see the open questions)*

## Left out on purpose

Not planned, because they only work with an online service or a product JeffOffice doesn't have: Office add-ins and the Store, Designer and Design Ideas, Insights, Smart Lookup, and Researcher, Microsoft's libraries of stock images, icons, people, videos, and 3-D models (Online Pictures and the bundled icons stand in), Microsoft Translator, online dictation, Present Online and Broadcast through Microsoft's service, OneDrive and SharePoint (online version history, co-authoring, check-in, and workflows), Stream upload, Microsoft Forms and Quizzes, Cameo, Presenter Coach, the Developer tab and VBA (macros are kept and never run), sensitivity labels, fax, and XPS.

## Decided

- Old and open formats (October 11, 2026, for all the apps): the first release opens the old and the open kinds, and saving them follows once the main kind is solid. Until then, Save on an old file asks for a new name in the new kind. JeffDeck has since added saving `.odp` and `.otp`; saving `.ppt`, `.pps`, and `.pot` is **later**.
- Embedded fonts (October 11, 2026): saving embeds, by default, the fonts JeffOffice brings that other computers may lack; fonts drawn in place of common ones are saved under the common name.
- Reference pictures: PowerPoint is installed on the Windows test machine (Office LTSC Professional Plus 2021), as Word, Excel, and Publisher are.
- Dropping the Preview label (October 16, 2026): it needs the old `.ppt` fidelity gaps fixed, measured on decks never used for tuning, plus five basics: a right-click menu on slides, Find and Replace, picture cropping, table editing tools, and slide master editing. The items that make them up carry "(needed to drop Preview)"; every other **1.0** item is needed before JeffDeck is called finished.

## Open questions

1. SmartArt: does editing a diagram's text have to be in 1.0, or are showing, keeping, and moving one enough to drop the Preview label? Editing needs a layout engine for hundreds of layouts, and none of our 19 real `.pptx` has a diagram.
2. Charts: does JeffDeck wait for the shared chart editor and JeffCrunch's grid, or ship a smaller chart editor of its own first?
3. The fidelity bar for dropping Preview: JeffPub's numbers on the 64 reference decks, and what share under 2 on files it was never tuned on (57% now on the 40 old `.ppt` files)?
4. Which transitions and effects make 1.0: the 2007 set (about 30 transitions and 3 dozen effects), or all of the original's roughly 50 transitions and 150 effects?
5. Video export: which encoder can ship, given that the FFmpeg libraries in the packages are the LGPL build (which may lack the usual MP4 video encoder), and is WebM beside or instead of MP4 acceptable?
6. Record Slide Show (voice, camera, ink, and laser pointer saved on each slide): 1.0 or later, since Office 2021 puts it on its own tab?
7. Present Online: leave it out, or build viewing on the local network in a browser, with no account?
8. Compare and merge: 1.0, or later with the shared review model?
9. Translate: is an offline translation model (size and license to check) worth shipping, or is Translate left out?
10. Should the Notes Page and Reading View buttons, which are on the View tab and disabled, come off the ribbon until they work?
