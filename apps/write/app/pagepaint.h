#pragma once
// Drawing a page of the document: one page the paginator made, with the page layer's renderer. The page view, the print and PDF
// output, and the screenshots all draw through this.

#include "flow/paginator.h"
#include "render/renderer.h"

class QPainter;

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
};

// Paints the page in its own coordinates (points from its top left corner) on the painter.
void paintPage(QPainter *p, flow::MaterializedPage &page, LayoutCache *layouts, const PageDrawing &drawing);

} // namespace jp::write
