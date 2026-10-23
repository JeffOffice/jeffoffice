#pragma once
// Drawing a page of the document: one page the paginator made, with the page layer's renderer. The page view, the print and PDF
// output, and the screenshots all draw through this.

#include "flow/paginator.h"
#include "render/renderer.h"

#include <functional>

class QPainter;
class QTextDocument;

namespace jp::write {

class SpellCheck;

struct PageDrawing {
    int selectionFrom = -1, selectionTo = -1;   // the selected text, as positions in the document
    bool marks = false;                          // paragraph marks, tabs, and spaces drawn
    bool output = false;                         // a printer or a PDF: no screen-only marks
    TextOutput textOutput = TextOutput::Picture;
    int pageCount = 1;
    bool grayscale = false;
    // The misspelled words get a red wave under them on the screen (never in a proof, a printer, or a PDF, which set `output`). `typingAt` is the caret
    // while a word is being typed: that word is left alone until the caret moves on.
    SpellCheck *spelling = nullptr;
    int typingAt = -1;
    // A header or footer is being edited: its story (the text of the page dims, and the boxes of the headers and footers do not), the selection in it, its misspelled words (its
    // own check, and the caret's place while a word is typed), and which of the two it is, for the label on its edge.
    const QTextDocument *editedStory = nullptr;
    int editSelFrom = -1, editSelTo = -1;
    SpellCheck *editSpelling = nullptr;
    int editTypingAt = -1;
    bool editFooter = false;
};

// Paints the page in its own coordinates (points from its top left corner) on the painter.
void paintPage(QPainter *p, flow::MaterializedPage &page, LayoutCache *layouts, const PageDrawing &drawing);

// A test's way to see which pages are drawn and how: called by paintPage with each page and its drawing (null to stop).
void setPaintObserver(const std::function<void(const flow::MaterializedPage &, const PageDrawing &)> &observer);

} // namespace jp::write
