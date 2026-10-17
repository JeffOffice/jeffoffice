# JeffWrite

JeffWrite is the word processor of JeffOffice, modeled on the one in Office 2021. It is the third
JeffOffice app, built on the suite's shared engine. Its feature list is
[docs/features/jeffwrite.md](features/jeffwrite.md); the shared parts it builds and uses are in
[docs/features/shared.md](features/shared.md). This file is how it is built. Decided October 16, 2026.

**Ready for a first release when** the first-release items of its feature list work, the original
program opens every file JeffWrite saves, and JeffWrite's pages match that program's own PDFs about as
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
wave 3 (notes, headers and footers, page fields, anchored pictures) and wave 4 (tables), `.doc`, `.rtf`, and `.odt`, the ruler, and Word's own PDFs to measure the pages against.

Find and Replace and the Font dialog are built too. The Find and Replace pane gives libs/page's shared search (app/findreplace.h) the one story, through `FindWords`; the
search does not reach the headers, footers, and notes, which are stories the program does not edit yet. The Font dialog changes what the person changed, as one undo step, and every
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
(`wrapPolygon`: tight is square), objects that may overlap one another (`allowOverlap`) or sit in a table cell's layout (`layoutInCell`), the inside and outside of mirrored margins,
a drop cap in the margin (`dropCap="margin"`), `w:tblOverlap` (floating tables are drawn where they ask, over one another if they ask so), a floating table that goes over a page break
or moves to the next column rather than the next page, selecting text in a frame's box on the page, and editing the frame's properties.

## Checking it

- The Windows test machine has Word 2021: reference PDFs of documents, and every file JeffWrite saves
  opened and re-saved there.
- Documents: our own collection, the US government documents in Govdocs1 and Apache POI's test
  files, with a held-out half that no helper sees until the final check, as for JeffDeck.
- JeffPub's render-compare must stay at 0 differences after every change below libs/flow.
