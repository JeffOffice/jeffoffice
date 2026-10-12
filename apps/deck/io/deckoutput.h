#pragma once
// Printing a presentation and making its PDF: printed slides, notes pages,
// handouts of 1 to 9 slides, and the outline. Every layout is drawn by
// paintOutputPage onto any painter set up in points (a PDF, the printer, the
// print preview, a picture), so what is previewed is what is printed.
//
// Slides print one to a page, in the slide's own shape. Notes pages, handouts,
// and the outline print on the locale's paper (letter or A4), upright: a notes
// page has the slide framed above and its speaker notes below; a handout
// frames each slide in a grid, with a header and footer (the title and date,
// the page number), and three to a page leaves lines beside each slide to write
// on. Hidden slides are left out unless the options ask for them.

#include "core/document.h"
#include "render/renderer.h"

#include <QImage>
#include <QSizeF>
#include <QString>
#include <QVector>

class QPainter;

namespace jp::deck {

enum class PrintLayout { Slides, NotesPages, Handout1, Handout2, Handout3, Handout4, Handout6, Handout9, Outline };

struct OutputOptions {
    PrintLayout layout = PrintLayout::Slides;
    bool hiddenSlides = false;    // print the slides that are hidden too
    bool frameSlides = false;     // a thin frame around each printed slide (notes pages and handouts always frame them)
    bool grayscale = false;
    QVector<int> slides;          // 0-based slides to print, in this order; empty: all
    bool horizontalOrder = true;  // handouts of 4 or more: across, then down (else down, then across)
};

// The slides the options print, 0-based and in order: the ones asked for (all
// when none are), without the hidden ones unless asked for, and without any
// the presentation doesn't have.
QVector<int> outputSlides(const Document &d, const OutputOptions &o);
// A range of slides as typed, "1-3, 5" or "7-" or "-2": the slides' numbers
// and spans of them, counted from 1, separated by commas or semicolons, in
// the order typed. Returns the 0-based slides of a presentation of `count`;
// empty when the text is not a range or names no slide (a span runs to the
// last slide at most).
QVector<int> parseSlideRange(const QString &text, int count);
// Slides on one page of a handout layout; 0 for the others.
int slidesPerPage(PrintLayout layout);
// The locale's paper (letter or A4), upright, in points.
QSizeF localePaper();
// The size of a page of the output in points: the slide's for printed slides,
// else `paper` (the locale's when empty).
QSizeF outputPageSize(const Document &d, const OutputOptions &o, const QSizeF &paper = QSizeF());
// Pages of the output. The outline's depends on the paper (the locale's when empty).
int outputPageCount(const Document &d, const OutputOptions &o, const QSizeF &paper = QSizeF());
// The outline as text: each slide's number and title, then its body text, two
// spaces in for each level.
QString outputOutlineText(const Document &d, const OutputOptions &o);

// Paints page `page` (0-based) of the output onto `p`, which is set up in
// points for a page of `paper` size with its corner at the origin. A slide
// goes at its own size or, on other paper, as large as fits and centered. The
// page is painted as the painter's device wants it: the text as a PDF or the
// printer draws it, and, when grayscale is asked for, in gray (a PDF or printer
// set to gray converts the colors itself, anything else is painted as gray pixels).
void paintOutputPage(QPainter *p, const Document &d, LayoutCache *cache, const OutputOptions &o, int page, const QSizeF &paper);

// Writes the PDF: vector text with its fonts embedded, the title from the
// presentation's properties or the file's name. The old file stays whole
// until the new one is complete; on failure says why in `error`.
bool exportPdf(const Document &d, const QString &path, const OutputOptions &o, QString *error = nullptr);

// One slide as a picture `width` pixels across (the slide's shape), on white.
QImage slideImage(const Document &d, LayoutCache *cache, int slide, int width = 1920);

} // namespace jp::deck
