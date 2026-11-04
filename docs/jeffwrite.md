# JeffWrite

JeffWrite is the word processor of JeffOffice. JeffOffice is an independent project, and JeffWrite aims to
work with Word files and the people who use them (compatibility and interoperability), and then to do
more, and do it better. It is the third JeffOffice app, built on the suite's shared engine. Its feature list is
[docs/features/jeffwrite.md](features/jeffwrite.md); the shared parts it builds and uses are in
[docs/features/shared.md](features/shared.md). This file is how it is built. Decided October 16, 2026.

**Ready for a first release when** the first-release items of its feature list work, every file JeffWrite saves
opens without a repair message, and JeffWrite's pages match the reference pages about as
closely as JeffPub's pages match Publisher's, measured on documents never used for tuning.

## The decision: a flow library above the page layer

A word processor's document is one long story that flows through pages made as the text fills them.
JeffPub already lays out a story across a chain of frames (libs/text, `StoryLayout`), with columns,
wrap, hyphenation, footnotes, widows and orphans, and fields, and its Autoflow adds pages until a story
fits. An investigation (October 16) measured what a word processor needs from that engine:

| Measured on JeffPub's engine | Result |
| --- | --- |
| Laying out 26,443 paragraphs (the longest document in our collection, 2,573 pages) | 8.0 s and 592 MB, all at once |
| One typed character, at the start or the end | the whole story again: 7.4 to 7.7 s |
| Undo | a snapshot of the whole document: 0.8 to 0.9 s at 20,000 paragraphs |
| Finding a page's frames in one long chain | grows with the square of the page count |
| Pages in one chain | wrong past about 2,340 letter pages (positions overflow) |
| Tables in the flow, pictures anchored to a paragraph, page references | not there |

Rebuilding that engine to be incremental would mean rewriting its most test-sensitive 1,000 lines,
which JeffPub's and JeffDeck's drawing depend on. Instead:

- **libs/flow**, a new library above libs/page (layers: base < draw < text < office < shell < page <
  flow < apps), holds the flowing document and its paginator. JeffWrite builds it; JeffBase's reports
  and JeffMail's composer use it later.
- **The paginator lays out one page at a time with the existing engine, unchanged.** Page k is a
  short StoryLayout run over a slice of the story, starting where page k-1 ended. A prototype laid
  out 465 pages in 2.2 s (4.7 ms a page) and matched the one-shot layout exactly for its first 44
  pages; later pages differed by single lines because a paragraph continued from the page before
  had lost its line count. So the paginator owns the rules that cross pages: widows and orphans,
  keep with next, keep lines together, and space before at the top of a page.
- **Incremental by construction.** An edit lays out again from the page holding the changed
  paragraph and stops when a page starts where it started before: about 10 to 30 ms for a typed
  character, whatever the document's length.
- **Painting and export reuse the page layer.** The paginator materializes the pages on screen (and,
  for printing or PDF, each page in turn) as ordinary Pages and TextItems with stable ids, so
  Renderer::paintPage, PDF export, and printing work unchanged.
- **Changes below libs/flow are additive only**, and each must leave JeffPub drawing every page
  identically and JeffDeck's slides unchanged:
  - an id index in Document (item lookup is a linear scan today);
  - the current time out of the layout cache key unless a story holds a date or time field (today a
    long story is laid out again every minute it's on screen);
  - small additions to StoryLayout for the paginator: line offsets in its line information, and a
    starting state for a continued paragraph and for footnotes carried from the page before.

## The model

- **FlowDocument**: an ordered list of **sections**, each with page size, orientation, margins,
  columns, and headers and footers (first page, odd, even), and one **main story**, a QTextDocument
  using the suite's text properties (libs/text), as JeffPub's stories do.
- **Blocks in the story**: paragraphs, and **tables** as block-level items in the flow (rows that
  split across pages and repeat their header rows), and **anchored objects** (pictures, shapes, text
  boxes) tied to a paragraph, with wrap. Inline objects use JeffPub's inline-object property.
- **Notes**: footnotes and endnotes as stories referenced from the main story, laid out at the foot
  of the page that holds the reference (libs/text already reserves and carries them within a frame).
- **Fields**: page-dependent fields (PAGE, NUMPAGES, SECTIONPAGES, STYLEREF, PAGEREF, TOC) are
  evaluated after pagination, repeating until page breaks stop moving (bounded).
- **References**: the fields that make paragraphs (TOC and its table of figures, INDEX, TOA, BIBLIOGRAPHY) are
  worked out in one pass (`apps/write/core/fieldops.cpp`) from the marks the text holds (headings, TC, SEQ, XE, TA, CITATION)
  and rebuilt as the paragraphs between the field's separator and its end (`tocops.cpp`, `indexops.cpp`, `citeops.cpp`). Sources
  are the file's bibliography part (a custom XML part, `writedoc_sources.cpp`), kept as read until they are changed. How notes are
  numbered is the settings part's `footnotePr` and `endnotePr` (`writedoc_notesettings.cpp`); footnote marks take their format from
  `FieldSource::noteMarkText`, and the endnotes are made again when the document is read from what it would save. Convert Notes is `convertNotes` (`editops.cpp`: a reference is replaced by one of the other kind, and `WriteDoc::convertedNote` makes the note with its styles); the separator notes are held as blocks, set by `setNoteSeparator` and written into the notes part by `notesPartXml`'s `separators`.
- **Undo** records commands (what changed in which block), not whole-document snapshots.

## Waves

1. **Flow core** (one helper): the additive lower-layer changes above; libs/flow with FlowDocument,
   sections, the paginator (widows and orphans, keeps, page and section breaks, columns), and
   materialized pages; benchmarks as tests (26,443 paragraphs paginate in under 5 s in the
   background with the first page shown in under 0.3 s; a typed character repaginates in under 50 ms).
2. **The app shell** (one helper, once wave 1's interfaces exist): apps/write with the window, ribbon,
   KeyTips, File screen, Help, and settings from libs/shell; a continuous view of pages with zoom;
   editing through libs/page's Editor holding the story; .docx reading and writing for text, styles,
   numbering, sections, headers and footers.
3. **Pages that books need**: footnotes and endnotes per page, headers and footers per section,
   page-dependent fields and the table of contents, anchored pictures with wrap.
4. **Tables in the flow**, then the rest of the first-release tier, in the order of the feature list.

## Where it stands

Wave 2 is built (October 16, 2026): `apps/write` with the window, ribbon, File screen, Help, and settings of the suite; the continuous page
view with zoom, drawing only the pages in view while the rest is laid out in the background; editing on the story's own undo stack (typing,
deleting, moving, selecting, the clipboard, character and paragraph formatting, lists, styles, page and section breaks, page setup);
`.docx` reading and writing in `libs/office/wml` (the XML tree, the model, styles with inheritance, numbering, sections, and everything the
program doesn't edit kept as XML or as whole parts and written back where it was); and packaging behind a switch that is off. What
JeffWrite shows of a table, a picture, a field, a header or footer, or a note is a stand-in or nothing; it keeps them. Not yet: the pieces of
wave 3 (notes, headers and footers, page fields, anchored pictures) and wave 4 (tables), `.rtf` and `.odt`, the ruler, and reference pages to measure the pages against.

Find and Replace and the Font dialog are built too. The Find and Replace pane gives libs/page's shared search (app/findreplace.h) the one story, through `FindWords`; the
search looks through the text only, not the headers, footers, and notes, which are stories of their own. The Font dialog changes what the person changed, as one undo step, and every
effect it sets (and every `w:u` value) is read from and written to `.docx`.

Spelling as you type and printing came after wave 4 as well (October 17, 2026). Spelling is a red wave under misspelled words, suggestions on right-click, and the Spelling pane on F7, checked a
paragraph at a time for the pages in view; printing is the File screen's Print page, with the printer, copies, collate, ranges, and a preview, and printing and the PDF export draw every page
through one function.

Floating content came after the drift round (October 17, 2026): the page is laid out with the objects its lines anchor as walls the text keeps out of (square, tight as square, and
top and bottom; both sides, the left, the right, or the larger), again until the objects and the lines agree, with at most 24 walls to a page, and the page layer sets the text of each
box round the same walls, so what is drawn is what was laid out. The header and the footer are the page's lowest layer (drawn before the body's objects, and no wall of the body's
reaches them). Frames (`w:framePr` in the body) are the paragraphs next to one another that share their properties, set out of the text (`tp::OutOfLayout`: no lines, no room) and
drawn as a text box where the file asks, at the paragraph after them; a table with `w:tblpPr` has a place, no lines among the text, and its rows drawn there, if it fits a page of its
own section (and moves with its paragraph to the next page when it would go over the foot). Every frame and floating table is somewhere: the frames and tables right before a paragraph
(or before the first row of a table) are set from it, the nearest 16 of them, and a frame of more than 300 paragraphs, the rest past 16, and those at the end of the story are laid out
as the text's own paragraphs, so that nothing of a file is left off its pages. Editing next to a frame keeps it apart (Backspace and Delete never join a frame's paragraph and the text's;
a paragraph style leaves a paragraph in its frame), and the caret, a click, Home, End, and Up and Down work in a frame's text. Not yet: the outlines of tight and through wraps
(`wrapPolygon`: tight is square), objects that may overlap one another (`allowOverlap`) or sit in a table cell's layout (`layoutInCell`), a drop cap in the margin (`dropCap="margin"`), `w:tblOverlap` (floating tables are drawn where they ask, over one another if they ask so), a floating table that goes over a page break
or moves to the next column rather than the next page, selecting text in a frame's box on the page, and editing the frame's properties.

`.doc` (the binary files of 1997 to 2003) opens, in `libs/office/doc` (October 17, 2026): a reader turns the compound file (its text pieces,
the bins of character and paragraph properties, the styles, lists, sections, headers and footers, notes, fields, tables, pictures, and the
drawing's shapes) into the parts of a `.docx` package that the `.docx` reader then reads, so layout, editing, and saving as `.docx` are the same
code; saving asks for a `.docx` name. On the practice half of the yardstick (the 75 government documents whose originals are `.doc`) the
`.doc` renders score as well as the same documents' `.docx` counterparts (median page 1.19 for both).

The words round (October 18, 2026) went through the words that the reference pages show and JeffWrite's lack. Page numbers after a continuous section that restarts them count the page it
begins on as its first; a header's or footer's STYLEREF shows the page's paragraph of the style (the first on the page, the last with `\l`, else the nearest before, with the number
switches giving its number); text without `xml:space="preserve"` has no blanks at its ends; rows and cells held by content controls (nested too) are the table's; sections' line numbers
(`w:lnNumType`: again at each page, at each section, or running on; `w:start` is the numbers skipped) are drawn in the margin; ruby text shows after its base text in brackets; a page that three sections or more begin on, one under another, is laid out
in a part for each (columns made even where another follows); and a page's copy of the text cuts runs of joined paragraphs where the document does. Not yet: ruby set above its base.

The second part of the words round (October 18, 2026): the endnotes the text refers to are set after its last text, after the file's separator for them, each with its number (they are
paragraphs of the story that the file does not have; their words are edited where they stand (see "Notes are edited" below), and saving writes the notes from them; at most 5,000 are set); an absolute position
tab (`w:ptab`) goes to the left, middle, or right of the margin or the indent, with its leader, whatever the paragraph's stops (`tp::AlignTab`); a page number carries the chapter's when the
section says `w:chapStyle` (the last heading of that level on the page, else the nearest before it, and `w:chapSep`); a first section of even pages numbers its first page 2; an item at a
deeper level than the list has had counts the levels above it as begun, so that the next item at one of them is one more; a numbered paragraph that begins with a page break has its number
after the break; a column break (`w:br w:type="column"`) sends the text after it to the next column, or the next page from the last column; the shapes of a group in a group of a drawing are
read once; the line of a drawn box takes no room from its text; and a justified line with no space keeps its letters together. The text the person has ends where the endnotes begin (`editableEnd`): select-all, a copy, a search, and a spelling check go as far as it does, and the endnotes are changed only inside one note's own words; a note's numbered paragraphs count on their own, and the file's separator is a rule 144 points long. A column break where there is no next column (a table's cell, a text box) is a line break, and a paragraph with an absolute position tab and ordinary tabs keeps its own stops for the ordinary ones. A reference to a bookmark (`PAGEREF`) shows the number of the page the bookmark is on, in its section's format and with the chapter's number where the section carries it, for the pages that are laid out (the rest show the file's last result); words are hyphenated where the settings ask (`w:autoHyphenation`, `w:hyphenationZone`, and a paragraph's `w:suppressAutoHyphens`). Not yet: footnotes cited in a table's cell, and the numbers in the chapter's own format when the heading's numbering text has words in it.

The third part of the words round (October 19, 2026) drew what the pages left out: an equation (`m:oMath`, `m:oMathPara`) shows every character in order as a line on the baseline, in the size and face of its first run
(fractions as numerator and denominator, scripts after their base, delimiters, n-ary operators with their limits, matrices row by row; an equation of its own paragraph is centered), bounded in depth, length, and
total, and written back as it was; letters of the mathematical alphanumeric block show as the letters they are styles of (italic or bold through a layout-only display style, `tp::DisplayStyle`), where text that has
them keeps them as atoms and writes them back as they were; a legacy form field shows its box (checked or not), its default text where its result has none, or the entry it chose, in the text and in boxes, headers,
footers, and notes; the endnotes of a section that puts them at its end (`w:pos` `sectEnd`, in the section or the settings) are set after its last text, its break moves onto the last of them, the export puts it back on
the paragraph before them, and select-all, copy, search, and spell check do not reach them (all the tails together are at most the most the program sets); a stand-in drawn with a stroke by hand keeps
each word in one piece, which a PDF text reader needs; East Asian characters that a run's font lacks fall back to any installed face that has them (nothing is bundled); and facing pages (`w:mirrorMargins`) swap the margins' sides on pages with an even number (the
text, the headers and footers, the page borders, and what is set from the margin; an object set in the inside or outside strip of the margin is still set as on a page with an odd number).

The 0.9.1 round (October 22, 2026) made the fixes people notice: the status bar says "1 word" and "652 words" (every counted text on screen has its English singular and plural forms, and counts that said "(s)" are counted by Qt's plural forms, so that each language gives its own); spelling accepts the suite's own names (JeffWrite, JeffPub, JeffDeck, JeffOffice, JeffCrunch, JeffMail, JeffBase, and LLC) in every language; and a paragraph of very many runs costs each page its own text and no more: a paragraph of letters with no place to break is laid out as breaking anywhere (the same lines, where Qt read on to the end of the word for each line), the default tab stops of a line are listed as stops for text that ends at a delimiter the paragraph has none of (Qt takes a tab to their position, as to a listed stop, and does not read the rest of the paragraph for each tab that a left stop answered; the stops it keeps for itself pass over a stop that a tab starts a fraction of a point before, and drift when the distance is not whole), where a page's runs begin, where its tabs are, and where its notes were counted are found from where the page before left off, and the look of a format, its fine form, and a width's tab stops are made once (500,000 runs of one letter: from over two minutes to 17 seconds). A style sheet's styles that carry no id are found by their names, and the blank result of a reference field takes no part in its line's height (blanks typed after it count as any typed do), which gave two practice documents (V141 and W157) the page counts of their reference pages. Spelling takes a name in capitals with its possessive in capitals ("JEFFWRITE'S"), and JeffWrite's English Help says what this version shows, keeps, and cannot yet edit.

Headers and footers are edited (October 23, 2026). A header or footer is a story of its own, in the form the text is edited in: a field is one character that shows its result (the page's number, the count, a
paragraph style's text, or what the file had), each element the program does not edit (a picture or shape, a bookmark, a link, a content control, a table) is a piece kept as written and put back where it
was, and a part with more paragraphs than are shown (400, those of a table's cells counted; a table is cut between its rows) keeps the rest, in the table where it was; the page and the
form the part is edited in show the same of it, and a paragraph a tracked change deleted is out of both and is written back. Double-clicking a page's header or footer area, or Insert > Header and Footer, enters it for the page's section and slot (the first page's,
the even pages', or the ordinary one); the text dims, the Header & Footer tab shows, and Esc, a double click in the text, or Close leaves. Everything in `editops.h` works there (the cursor is in the story), a
page number is `insertPageNumber`, and Different First Page (a section's `w:titlePg`), Different Odd & Even Pages (the document's `w:evenAndOddHeaders`, carried by the last section's record so that it undoes
with the text), and Link to Previous (a section with no reference of its own has the section before's; off, it has a copy with a part and relationship of its own) are steps of the history. A double click in the margin of a page whose section has none of its own shows an empty header or footer to type in, and makes nothing in the file until something is put in it
(`createHeaderFooter` with `lazy`; leaving it as it is leaves the document as it was, and Insert > Header and Footer makes it at once); the blank page that an odd-page or even-page section
break leaves has none to enter. A header or footer takes no page, column, or section break, and no reference to a note or a comment: the commands are grayed there, typing and pasting leave them
out, and copied text has a field as the text it shows (the number of the page being edited, the count of pages, the date as the file had it). There is one
history (`WriteDoc::undo` and `redo`): the steps of the text and of each header and footer in the order they were made (a story's next edit after another story's is not joined to its last step; an edit that joins the block of the one before it is one step, not two), and the view
follows a step into the header it was in. Saving writes a header or footer from its story only if the history has steps in it (a part that was not edited, or whose edits are all taken back, is written back as the bytes it was read as; so are the
styles, when the Header and Footer styles that making one added have no paragraph left), new parts get a name, a
relationship, a content type, and the section's reference, and a part no section names any more is not written. A part that cannot be read (not XML), one that is past the 64 parts read, and a reference
that names an address outside the file are never replaced. Find and Replace and the Spelling pane look through the text only (opening them leaves the header, and so does any cursor in the text that the view is given), and misspelled words are marked in the header
being edited. Not yet: editing a header's pictures and shapes, and its tables' rows and columns (their text is edited), the gallery of built-in designs, Go to Header and Go to Footer, and Date and Time.

Notes are edited (October 23, 2026). A footnote is a story of its own, in the form the text is edited in, like a header's (`WriteDoc::editableFootnote`; one is made the first time a page lays out a note that cites it, up to 5,000 for a document, and `isSideStory` is true of them). The pages show the notes that a page carries as `MaterializedPage::noteBoxes`, each saying which story it shows, and `Paginator::noteEdited` sets again the pages
that cite a note whose words changed (the note's cached height is dropped, and the pages from the first that cites it to the one after the last are laid out again). Clicking in the words of a note on a page puts the caret there
(`PageView::m_note`; the hf functions work for a note as for a header, through `sideStory()`), and everything in `editops.h` works in it; the number that begins the note and the separator lines are not in the story's words, so no edit
reaches them, and a note takes no page, column, or section break and no reference to another note. Esc, Up at the first line, Down at the last, or a click in the text leaves the note for the text just after its reference. There is one
history with the text and the headers and footers (the steps carry the story they were made in), and an undo moves the view to the note it changed. A note is the document's while the text cites it (`footnoteReferences`, an index that an
edit of the text drops): Insert > Footnote (References tab, Ctrl+Alt+F; `insertFootnote`) puts a reference at the caret (after a selection's end, replacing nothing) and a new note (`makeFootnote`: one more than any Id the file or its
references use, with its number mark and a blank) and puts the caret in the note; the notes after it are numbered again by the pages; deleting the last reference to a note takes the note out of the document, and an undo brings back both;
cutting and pasting a reference moves its note, and pasting a copy of one cites a copy of the note (`duplicateFootnote`); two references to one note, as some files have, share it until the last is deleted. Saving writes the notes part
from the stories: a note whose story has no history steps is written back as the bytes it was read as (`notesPartXml` copies every child it was not asked to change), a note that was edited is written from its story with its mark, its
atoms, and its other parts, and the separator, continuation separator, and continuation notice notes are kept as the file has them; a notes part the file lacks is made with a content type, a relationship, the separators, and the
settings' `w:footnotePr` references. A note that cannot be read is never replaced, and a hostile file (thousands of notes, a note of thousands of paragraphs, references to notes that are not there, a note cited twice, a note in a header
or a table) is edited and saved within the bounds above and the 400-paragraph bound a header has.

The endnotes are edited too, though they are paragraphs of the body story (set after the text, or after each section that puts them at its end), stamped `EndnoteBlock` with the copy they belong to (`EndnoteNote`: the first copy of a note
is the one that is edited, the others, for a note cited again, are shown) and the style they were read in (`EndnoteStyle`). `isLocked` is true of every one of them but the paragraphs of the note an edit is inside of (`NoteScopeGuard`), so
an edit inside one note's words works as it does in the text, and one across two notes, into the separator, or into a number mark is refused (`holdInEndnote`; a table the note holds is kept as it was and is locked, so the words before
and after it are two places to type). The notes an edit touched are tracked by Id (`touchEndnotes`); `captureEndnotes` takes them back as the blocks the notes are made of when the document is saved or the tails are set again, and the save
replaces a note of `endnotes.xml` only when its blocks differ from those it was read as (`blocksFingerprint`), writing the rest byte for byte. After every edit `settleEndnotes` compares the notes the references name with the notes the tails
have, and when they differ (a reference was added, deleted, cut, or pasted, or a section break moved) `rebuildEndnoteTails` takes the notes back, sets the tails and the numbers again, and joins all of it to the edit's undo step.
Insert > Endnote (Ctrl+Alt+D; `insertEndnote`), deleting a reference, and moving or copying one work as they do for footnotes. Find, spelling, select-all, and copy keep to the text the person has (`editableEnd`).

Notes, after the review of October 23, 2026. A paragraph of an endnote keeps its note under every style and look command (`applyStyleToBlock` copies `EndnoteBlock`, `EndnoteNote`, and sets `EndnoteStyle`, which `paragraphStyleAt` reads); a header and a note
are never edited together (`PageView::leaveSideModeFor` is the one place the modes meet, `restoreBodyCursor` the one that gives the caret back); the page a note is edited on is the page of the copy being edited
that has the caret's place (`partsOfEditedNote`, a part for each page the note goes on over, a copy for each citation; `syncNotePage` follows the caret, and Up and Down at the edge of a part go on into
the next or the earlier part); a new note takes an Id above every Id of its part (`readNotes` says the highest, the separators and the notes past the most that are read included), and the writer matches a note by its Id
and its type; a note is cited while the text, a header or footer, a text box or drawing kept as XML, or another note cites it (`notesCitedBeyondTheText`); the most notes is counted by the notes the text cites, a note that
no step of the history can bring back is let go with its slot (`reclaimDeadNotes`: its history step is in neither log), and at the most both commands are off with the reason in the status bar; and settling the endnotes
follows the references as the story changes (`followEndnoteReferences`), so an edit that touches none of them costs nothing more than it does without an endnotes part.

## Editing tables (beta wave B, October 31, 2026)

A table is changed by changing the bytes of its parts and making the records of its cells again from them (`core/tableops.cpp`, "Making and changing tables"). The start of the table (its
`w:tblPr` and grid), of each row (`w:trPr`), and of each cell (`w:tcPr`) are kept as the file's bytes in `TablePart`, `RowPart`, and `CellPart` (`core/tablemodel.h`); a command patches the
properties it sets (`patchPr` puts an element among the others in the schema's order, and merges the sides of a border group), and `remake` feeds the parts to the same `TableBuilder` that reads a
file, with the marks read from the bytes by `readTableMark` (libs/office/wml). So the records, the table style's parts, and the paragraphs' look come out as they will when the saved file is opened,
and the tests compare a table with the same file opened again (`tablesSignature`). The paragraphs of the cells are then given the keys of the new records in the same step (`giveKeys`), so one undo
takes a command back. Merge Cells and Split Cells also move paragraphs; they work from the last place of the story up, so the places of the layout they started from stay good. Insert Table makes
the parts by hand and takes its look from the table style (Table Grid, added to the document when it lacks it: `builtinTableStyleIds` in libs/office/wml/wmlnew.cpp has the gallery's styles).
Kept pieces between rows and cells go through `remake` in their places. Not yet: Split Table, deleting a single cell, a column's width in percent, text direction, text wrapping around a table,
Modify Table Style and New Table Style, the border painter, and drawing a table.


## Pictures, page setup, and the ruler (beta wave C, November 1, 2026)

A picture is a character of the story that holds the `w:drawing` of its run as XML (`core/pictureops.cpp`). Every command writes that element again from a small model of it (`Model`: the
extent, the wrap, the place, the alt text, and the graphic kept as it was, with the picture's size patched in) and gives the character the form the pages show (`WriteDoc::makeDrawnFormat`,
which is what reading a file does), in one change of the character's format, so each is one step of the history and the file saved says what the pages show. Only a drawing that is one plain
picture (`wp:inline` or `wp:anchor` holding a picture) can be changed (`PictureInfo::editable`); any other picture is selected and kept. Insert Picture reads the bytes with `QImageReader`
before decoding (a side past 30,000 pixels, more than 40 million pixels, or more than 48 MB is refused), keeps PNG, JPEG, GIF, BMP, and TIFF as they are and any other form it reads as PNG,
and reuses a part that holds the same bytes (`WriteDoc::addMedia`). The forms the pages draw from (`FlowDocument::objects` and `images`) only grow, because a step of the history may bring
a character that names one back; at the most the document can show (2,000 pictures, 20,000 objects) the command is refused and says so (`WriteDoc::pictureNotice`), and nothing is let go.
A drag with the mouse (`PageView`: the handles, `pageview_pictures.cpp`) changes the size and the place on release: a picture set at a place is put on the page (`positionH` and `positionV`
from the page), and one in line goes to the place in the text where it is dropped. A picture on the clipboard (or a picture file dropped on a page) is inserted the same way.

Page Setup (`core/pagesetup.cpp`) applies what the dialog changed (and only that) to the section or to every section in one pass over the story (`WriteDoc::changeSections`, which makes the
sections again once) and one step of the history; the values are held to what a page can hold (`normalized`: paper from 0.5 to 30 inches, margins that leave half an inch of text).
Not yet: vertical alignment of the page, line numbers, borders, mirrored margins and several pages to a sheet, a gutter at the top, and "from this point on".

The ruler (`app/ruler.cpp`) is above the pages and reads the paragraph at the start of the selection and the section the caret is in. Its markers are dragged with the mouse and each
drag is one change on release: the indents are changed by the amount dragged in every paragraph the selection touches, the margins through Page Setup's code, and the tab stops
(`core/rulerops.cpp`: at most 64 for a paragraph, at most 22 inches in, each with its leader) through the paragraph's format. Not yet: the vertical ruler, the column of the caret in a section of
several columns (the ruler shows the whole text area), mirrored pages, the default tab stops, and a double click on the ruler for the Tabs dialog.

## Styles, Copy Formatting, Go To, and dragging text (beta wave D, November 1, 2026)

A change to the styles is a change to the style sheet and to the text that has the styles, and is one step of the one history. `core/styleops.cpp` makes the change inside one edit block of the
story: `WriteDoc::putStyle` puts the style in the sheet and keeps what it was and is (`StyleChange`) for the step the edit becomes (`stepAdded` takes it, by the step's serial, into
`m_styleSteps`); a count on the first paragraph (`prop::StylesRev`) is raised so that the edit is a step of the story's undo stack whatever paragraphs it touches; `WriteDoc::undo` and `redo` put the
sheet as it was or is for each step they move (`applyStyleChanges`, which also gives the sheet's "changed" mark back, so that a change and its undo write `styles.xml` as the bytes it was read as). What
a step holds of the styles is the one style it changed, never a copy of the sheet, and it is let go with the step (a new edit ends what could be redone, and with it the changes those steps held).
Modify Style restyles the text in one pass (`restyleText`): each paragraph of the style, or of one based on it (`descendantsOf`, every style looked at once), takes the properties of the new look in
place of those of the old, for the properties it still has as the old look gave them, and so does each run of the style or of a character style based on it; what the paragraph or run says itself stays. The
old look is worked out with the old sheet put back for the moment (`swapStyleSheet`), once for each style met. A definition that comes to the style as it is (compared as the bytes `styleXml` writes)
changes nothing. A style written from its fields keeps `w:aliases`, `w:autoRedefine`, `w:hidden`, and the elements after `w:qFormat` (`Style::aliases`, `autoRedefine`, `hidden`, `late`).
The bounds: a name is 255 characters at most (listed names are cut to that), Create Style adds no style to a document of 20,000, the Styles pane lists 5,000 at most, a basis that would make a loop is
refused (a loop a file has is followed no further than the resolvers' 32 steps, and the style being changed is looked at once), and a following style is followed once. A style that updates itself
(`w:autoRedefine`) takes what a paragraph of it was given by hand (`changeBlockFormat`, and `changeCharFormat` on a selection) as part of that command's step (`followAutoRedefine`). Not yet:
linked, table, and list styles, the paragraphs of headers, footers, and notes follow a changed style only when they are made again, and a style whose numbering changed leaves the lists as they are.

The Copy Formatting (`copyFormat`, `pasteFormat`) holds the look of the first character of the selection and, when the selection takes in a paragraph mark or is a caret, the paragraph's format; the
window (`WriteWindow::setPainterMode`) paints it when the view says a selection was made with the mouse (`PageView::selectionMade`), once or until Esc. What belongs to a paragraph in the file
(`paragraphKeepsOwn`: its list, section, frame, and the XML kept beside it) is never painted. Go To (`core/gotoops.cpp`) finds a place and changes nothing: a page is the paginator's page (laid out as far
as it), a line is counted from the top of the document, and the lists it offers are held to 5,000. Dragging text (`moveSelectedText`, and `PageView::prepareTextDrag` and `dropText`) copies the
selection as a clip, cuts it, and pastes it at the drop (the place is moved back by what was cut) in one edit block.

## Review: tracked changes, comments, and Display for Review (beta wave E, November 1, 2026)

**How a change is kept.** The story keeps a tracked change as the file does, so that a step of the history and a save need nothing else. An insertion or a deletion (`w:ins`, `w:del`, `w:moveFrom`,
`w:moveTo`) is a pair of invisible pieces kept as XML round the text it holds, as a hyperlink's are; the words a deletion holds (`w:delText`) are text of the paragraph between the pieces
(the reader turns them into text, and the writer puts `w:delText` back wherever the text is inside a deletion), hidden from the lines (`tp::HiddenText`, with `prop::TrackedDeleted` saying why). A change of a
run's properties (`w:rPrChange`) is in the run's kept properties, one of a paragraph (`w:pPrChange`) in the paragraph's, and a paragraph mark that was inserted or deleted says so in the mark's
kept properties. What the lines mark (`tp::Revision`, `tp::RevisionAuthor`, `tp::RevisionFormat`) is a look put on the text from those pieces (`refreshRevisionLook`), never read back. The records of changes
to tables, rows, cells, sections, and numbering are kept in the XML they are in and are not made or resolved.

**`core/reviewops.*`** finds changes (`revisionsIn`, `revisionAt`, `nextRevision`, `previousRevision`, at most 2,000,000, and a pane lists 2,000), and accepts or rejects them (`resolveRevisions`) in one edit block:
the blocks from the last to the first, in each the changes from the last to the first with the end of each moved by what the changes inside it did (so that 100,000 changes cost one pass), the paragraph
mark last (a mark that is taken away joins the paragraph to the next, which keeps its own look; a paragraph with nothing left goes). Rejecting a change of properties gives the old properties back
by putting the difference between what the run says now and what its record says on the format (`restoredFormat`), so that what is not a run's property (a piece kept as XML, a field) stays. Accept and
Reject at the caret take the innermost change there (or every change in the selection) and go on to the next. The text engine and the page paint the views (`LayoutEnv::revisionView`, set by
`WriteDoc::setRevisionView`, and the story's `tp::kRevisionViewProperty` for the places that have no environment): deleted text takes its place (and a paragraph whose mark was deleted stays a paragraph of
its own), is struck through, and inserted text is underlined, in the color of each person; Original leaves out what was inserted. A view is never an edit: it changes no format of the story, so it
is no step of the history and no change of the file.

**Making changes** (`editops.cpp`, "tracked changes"): with `WriteDoc::trackChanges` (the file's `w:trackRevisions`, written when it differs from what the file had) on, `insertText`, `insertParagraph`,
`removeSelection`, `deleteBack`, `deleteForward`, `changeCharFormat`, `changeBlockFormat`, and `applyParagraphStyle` record what they do, each in the edit block that was the edit, as one step. Typing goes
inside the reviewer's own insertion when the caret is in one or just after one, else between a new pair of pieces; deleting what the reviewer typed removes it (and a pair left empty), and anything
else is marked deleted (joined to a deletion of the reviewer's beside it) and stays; a selection typed over is a deletion and an insertion; the mark of a paragraph the reviewer made (Enter) is taken away for good by Backspace. The author is the user name of the settings (the window gives it to the document with
`setReviewer`), the Id is above every one the document has (`nextRevisionId`), and the time is now. `UntrackedEdit` makes the commands record nothing while Accept and Reject use them. Not tracked yet: table commands, lists, sections, and
what is pasted or dragged; paragraph marks inside a table's cells.

**Comments** (`core/commentops.*`): the file's parts (comments, their extensions for replies and resolved, the durable Ids, and the people) are read once (`CommentSource`) and the text has three marks of each comment (the two ends of its range,
and the reference). A comment made or changed here keeps what it says in the reference mark (`Inline::look`), so that a step of the history takes it with it and Undo needs nothing else; the parts are written
again at a save only when a comment was made, changed, or taken out (`writeCommentParts`): the file's own comments stay the bytes they were, new ones are generated, the extension part gets one entry for each comment (the
paragraph id of its last paragraph, its parent's, and whether it is done), the Ids part keeps its entries and gets one for each new comment, and the people part gets the new authors. A reply is a comment with its own
range marks next to its parent's. A reply to a comment that is not there, or a loop of replies, is a comment of its own; a comment with no range is at its reference. Everything is held to 100,000 comments.

**The window**: Review has Track Changes (Ctrl+Shift+E), Accept and Reject with their menus, Previous and Next, Display for Review, Show Markup, the Reviewing pane (`ReviewPane`: comments in threads and
changes, with their buttons), and the comment commands (New Comment is Ctrl+Alt+M); the status bar says whether tracking is on. `PageView::setReviewMarks` has the page view paint a bar in the margin
by each line a change touches, and a balloon for each comment (and each deletion, in balloons), over the pages from the page's own line positions; they are in a picture of a page too, never in print or PDF. A document with changes opens with All Markup. What Show Markup has on is each window's own (all of it on when a window begins); a long
document is looked through for changes and comments a moment after the last edit, not at each key.

**Checking it**: `writetool review-text FILE accept|reject|none` writes the body's text as the original program reports it (a paragraph ends in `\r`, a cell and a row in `\r\a`, hidden text and the code of a field are left out, text in
capitals is in capitals, a symbol is `(`, a picture in the line is `\x01`, a note's reference `\x02`), and `tools/review_compare.py WRITETOOL REFDIR` compares it with reference texts, after taking out only `\a` and the field marks
(`\x13`, `\x14`, `\x15`) of both and making every line ending `\r`. On the 27 documents with tracked changes of the dev set and practice set 2 it matches Accept All and Reject All on all 54 texts. `writetool review-list` lists the changes
of a file and `review-save` saves the file with all accepted or rejected. The tests are `writereviewtests` (one class, `tests/write/tst_write_review.cpp`): the hostile cases are 20,000 changes in a paragraph (Undo of Accept All gives the
exact file), nested changes as deep as the reader goes, comments with no range, and a loop of replies (at 100,000 changes, paragraphs with deleted marks, 100,000 comments with loops, and a reply chain 50,000 deep, Accept All and Reject All take 2 to 8 seconds).

## Insert, Layout, Design, and View (round 9, November 4, 2026)

Symbols, special characters, a blank page, hyperlinks, text from another file, the breaks, columns, hyphenation, line numbers, mirrored margins, the page's color and borders, the Navigation pane, and Focus.

- **Symbols** reuse JeffPub's Symbol dialog (`libs/page/app/textdialogs.cpp`): `symbolDialog(parent, insertOne)` takes the function that puts a symbol in, so JeffWrite's `core/insertops.cpp` puts it in a font of its own (one step through `WriteDoc::StepGroup`). The special characters are plain characters of the text; the two hyphens are `w:noBreakHyphen` and `w:softHyphen` in the file.
- **Document-level settings** (mirrored margins, automatic hyphenation, the page color) ride in the record of the last section, as the different odd and even headers do, so an undo brings them back; they are written to `word/settings.xml` (`settingsWithMirrorMargins`, `settingsWithAutoHyphenation`, `settingsWithDisplayBackgroundShape`) and to `w:background` only when they changed.
- **Columns, line numbers, and page borders** are set on a section's properties by `core/layoutops.cpp` (the line numbers and borders are kept elements of `w:sectPr`, written in the schema's order). The Columns dialog sets unequal widths (`w:equalWidth="0"` and `w:col`) and a line between (`w:sep`, drawn by `Paginator::addColumnLines`).
- **Hyperlinks** are a pair of `w:hyperlink` tags round the words, an address outside the file being a relationship of the main part (`core/linkops.cpp`); a Ctrl+click follows web, email, FTP, and telephone addresses only, and no link starts a program or opens a file.
- **Text from File** opens the other file as a document of its own and pastes its text with its looks, pictures, and links through the same copy a paste between two windows uses.
- **The Navigation pane** (`app/navpane.cpp`) lists the paragraphs that have an outline level (the same list as Go To), draws small pictures of the first 60 pages with `PageView::pageImage`, and finds words.
- **What an `.odt` leaves out**: the notes of the save now name page borders, line numbers, unequal columns, the page color, mirrored margins, and automatic hyphenation.

## Checking it

- Reference pages of documents, and every file JeffWrite saves opened and re-saved without a repair
  message.
- Documents: our own collection, the US government documents in Govdocs1 and Apache POI's test
  files, with a held-out half that no helper sees until the final check, as for JeffDeck.
- JeffPub's render-compare must stay at 0 differences after every change below libs/flow.

### Scoring the pages (beta wave A, October 31, 2026)

Run `sh ~/jo-scratch/wscore.sh BUILD/writetool LABEL dev` (the 95 tuning documents) or `... word2` (practice set 2, 200 documents); each prints one line (pages, median, share
under 2, page counts that match, words missing) and leaves `scores.tsv` (a row for each page) and `pages.tsv` (a row for each document) in `~/jo-scratch/score-SET-LABEL/`. The
held-out half is not scored or looked at until a gate.

| | Practice set 2 before | after | Dev before | after |
|---|---|---|---|---|
| Pages under 2 | 76.2% (1,513 of 1,986) | 82.1% | 91.0% | 91.8% |
| Page counts that match | 188 of 200 | 190 of 200 | 93 of 95 | 93 of 95 |
| Words missing | 0.18% | 0.18% | 0.15% | 0.15% |

Triage of the 473 pages of practice set 2 that scored 2 or more before the wave (78 documents), by cause, with what the wave did:

| Cause | Documents (pages) | Done |
|---|---|---|
| Space after the last note at the foot, cuts of notes (widow and orphan control), no break after a slash in a link's address, spaces at a paragraph's end that wrap | V106 (27), V073 (22), V041, V181 | V106 8, V073 4 pages left |
| Empty header and footer paragraphs of a file with no Header style are the default size, not Normal's | V179 (35) | 9 left |
| The thin stand-in for Courier New is lighter than the reference letters (pictures only) | V070 (43), V095 (5), V166 (2), V182 (1) | 0 left |
| A numbering level that follows its number with a space or nothing | V059 (4), V056 (12, some), V084 | V059 1 left |
| A cover of continuous sections with unequal columns and a picture whose wrap cycles | V014 (47) | not done |
| Tables and frames that go over a page, and list columns in a continuous section | V107 (24), V011 (16), V113 (10), V024 (9) | not done |
| Pictures, drawn text, WordArt, and fields the reference page printed at another time, not line or page breaks | V158 (9), V183 (8), V017 (7), V083 (6), V084 (6), V099, V105, V185 (4 each) | not done |
| Heading and paragraph spacing of a few documents, and the rest | V056, V063, V088, V153, and 40 more documents of 1 to 6 pages each | not done |

What is left is under 1% of the pages for each cause except the cover of V014 (2.4%), and tables and frames over a page (V107, V011, V113, V024: 2.8% together).
