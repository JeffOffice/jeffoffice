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

// Whether the header (footer) of the place holds anything but page numbers (their fields, the words "Page" and "of" round them, and boxes of a design): the text that would be replaced.
bool pageNumberTargetHoldsText(WriteDoc &doc, const PageNumberPlace &place);

// Puts the design `id` (of the top, the bottom, or the margins) in the header or footer the page uses. A box of an earlier design goes first. With `Replace` the text of the header goes
// (a watermark stays); with `Keep` it stays and the design is added (a paragraph after it, or a box). One step that undoes. False for another kind of design, or where the header
// can't be edited.
bool insertPageNumberDesign(WriteDoc &doc, int page, const QString &id, PageNumberText what);

// Puts the design `id` of the current position (plain, X of Y, bold, or circled) at the cursor, in the text or in a header or footer; a selection is replaced. One step that undoes.
bool insertPageNumberAtCursor(WriteDoc &doc, QTextCursor &cursor, const QString &id);

// Takes the PAGE and NUMPAGES fields out of every header and footer, the words of a design's "Page X of Y" that are left alone in their paragraph, the rule of a design, and the boxes of a
// design; every other text of the headers and footers stays. One step that undoes. Returns how many fields and boxes went.
int removePageNumbers(WriteDoc &doc);

// The accent of the document's theme (a plain blue where it has no theme): the color the pictures of the designs are drawn in.
QColor pageNumberAccent(const WriteDoc &doc);

} // namespace jp::write
