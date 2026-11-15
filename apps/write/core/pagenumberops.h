#pragma once
// The Page Number gallery of JeffWrite (the designs are the shared gallery's, libs/shell/app/pagenumbergallery.h): a design put in the header or footer of the page the caret is on, a
// number in the margin, or a number at the caret; and Remove Page Numbers. A design is made of the PAGE and NUMPAGES fields, in the paragraph of the header (footer) with its
// alignment, rule, and size, or in boxes anchored to the page (a circle, rounded box, tab, or bar, and the margin designs) whose text is the field. Each is one step that undoes.

#include "app/pagenumbergallery.h"
#include "core/writedoc.h"

#include <QColor>
#include <QTextCursor>

namespace jp::write {

// What becomes of the text a header or footer already holds when a design goes in at the top or the bottom: it is replaced, or it is kept and the number is added to it.
enum class PageNumberText { Replace, Keep };

// The place a design goes on the page given (a number from 0): the section's header (footer) of the kind of page it is (the first page's, an odd page's, or an even page's). False for a
// design of the current position, or a page with none (a blank page).
struct PageNumberPlace {
    int section = 0;
    bool footer = false;
    int slot = 0;
};
bool pageNumberPlaceOf(WriteDoc &doc, int page, const PageNumberDesign &design, PageNumberPlace *place);

// Whether the header (footer) of the place holds anything but page numbers the gallery put in (marked ones), spaces, and a watermark: the text that Replace would take away. A page number
// field of the person's own is text.
bool pageNumberTargetHoldsText(WriteDoc &doc, const PageNumberPlace &place);

// Puts the design `id` (of the top, the bottom, or the margins) in the header or footer the page uses. What an earlier design of the same gallery left goes first. With `Replace` the text of
// the header goes (a watermark stays); with `Keep` it stays and the design is added (a paragraph after it, or boxes). The design is marked as gallery content: a content control (w:sdt) whose
// w:docPartObj names "Page Numbers (Top of Page)", "(Bottom of Page)", "(Margins)", or "(Current Position)". All or nothing: if a step fails, the whole step is taken back (and cannot be
// redone) and false is returned. One step that undoes. False too for another kind of design, or where the header can't be edited.
bool insertPageNumberDesign(WriteDoc &doc, int page, const QString &id, PageNumberText what);

// Puts the design `id` of the current position (plain, X of Y, bold, or circled) at the cursor, in the text or in a header or footer, marked as a gallery's; a selection is replaced. All or
// nothing, one step that undoes.
bool insertPageNumberAtCursor(WriteDoc &doc, QTextCursor &cursor, const QString &id);

// Takes out of every header and footer what is marked as page numbers of the gallery (by whoever made it: another program's files carry the same marks): the runs of a paragraph and the
// paragraphs that a mark of their own level holds, with the boxes anchored in them, and a paragraph the marks leave empty. Nothing else is touched: a PAGE or NUMPAGES field or a box that the
// person made stays, and so does a header with nothing marked (it is not even made editable). One step that undoes. Returns how many marks went.
int removePageNumbers(WriteDoc &doc);
// Whether any header or footer holds something marked as page numbers (Remove Page Numbers has something to do).
bool hasPageNumbers(WriteDoc &doc);

// For tests: how many times hasPageNumbers has looked through the headers (the window keeps its answer until the headers or sections change).
int pageNumberScansForTest();

// For tests: the `operation`th step (from 1) of the next designs fails, as a failure in the document would; 0 for none.
void setPageNumberFailureForTest(int operation);

// The accent of the document's theme (a plain blue where it has no theme): the color the pictures of the designs are drawn in.
QColor pageNumberAccent(const WriteDoc &doc);

} // namespace jp::write
