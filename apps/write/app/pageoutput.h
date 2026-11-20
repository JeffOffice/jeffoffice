#pragma once
// Pages on a sheet: the one way JeffWrite puts its pages on paper, for the PDF export (a PDF writer) and for printing (a printer, to a printer or to a file).
// Both draw each page with the page view's painter (app/pagepaint.h), one page to a sheet, the sheet the size of the page's section and turned as it is.

#include "app/pagepaint.h"
#include "core/printing.h"
#include "core/writedoc.h"
#include "app/printpage.h"
#include "io/pdfbookmarks.h"

#include <QPagedPaintDevice>
#include <QString>
#include <QStringList>
#include <QVector>
#include <functional>

namespace jp::write {

class WriteTagger;

// What the marks over a page drew: bars (a line a change touches), comment balloons, deletion balloons, and carets.
struct MarkupCounts {
    int bars = 0, comments = 0, deletions = 0, carets = 0;
    int more = 0;          // balloons that did not fit (a margin too narrow, or no room left down the page): a note at the foot of the page says how many
    double lowest = 0;     // the lowest point (in the page's points from its top) a balloon or the note reaches
};
// Draws the marks of review over a page (as the page view draws them on the screen) in the page's own coordinates; returns what it drew. Nothing for a page with no body text.
// A balloon never goes past the foot of the page: the ones that do not fit are counted in a "+N more" note at the page's foot.
MarkupCounts paintMarkupMarks(QPainter *p, flow::MaterializedPage &page, LayoutCache *layouts, WriteDoc &doc, const MarkupMarks &marks);
// A test's way to see the marks each page drew (called by paintSheets once for each page it draws marks on; null to stop).
void setMarkupObserver(const std::function<void(int pageNumber, const MarkupCounts &)> &observer);

// Draws the pages (indexes from 0), in the order given, a sheet each, on the device. False when nothing could be drawn. `pageCount` is what the pages say they
// are of (NUMPAGES).
// With a layout, several pages share a sheet (2, 4, 6, 9, or 16, in rows from the top left, each scaled to fit its place and framed with a thin line), and/or each page is
// scaled to fit a paper size, centered on it; a sheet that holds two or six pages is turned to lie on its side.
struct SheetLayout {
    int perSheet = 1;
    QSizeF paper;         // the sheet's size in points, upright (empty: each page's own)
    PrintTurn turn = PrintTurn::AsSet;   // tall or wide: the sheet is turned that way, whichever way its page is, and the page is scaled to fit it
};
// The columns and rows that a number of pages on a sheet makes (1 for any number that isn't one of 1, 2, 4, 6, 9, 16).
QSize sheetGrid(int perSheet);
// `tagger`: the pages are marked for a tagged PDF (the pages of a sheet in the order they are laid out on it, which is the order they are read in).
// `maxImageDpi`: pictures are downsampled to that many pixels to the inch (a PDF of minimum size; 0 leaves them as they are).
// `look`: what prints besides the text (core/printing.h): its marks are drawn over each page, and its background says whether the page's color is on the paper (unset: a PDF has it, a printer is not sent it).
bool paintSheets(WriteDoc &doc, QPagedPaintDevice *device, const QVector<int> &pages, int pageCount, bool grayscale = false, const SheetLayout &layout = SheetLayout(), double maxImageDpi = 0, WriteTagger *tagger = nullptr, const PrintLook *look = nullptr);

// The sheet (from 0) of the PDF that each of the pages is on, as paintSheets makes them (-1: a page it leaves out); the links and the bookmarks go by it.
QVector<int> sheetOfPages(WriteDoc &doc, const QVector<int> &pages, const SheetLayout &layout);

// The links of the pages as a PDF holds them (see core/linkops.h): over the words of each link on the page. `sheets` are the pages the PDF holds, in order (paintSheets' `pages`); with a
// `layout` the areas and the spots are where the page's place on its sheet puts them (several pages to a sheet, or scaled to a paper); a link to a page the PDF leaves out is left out.
QVector<PdfLink> sheetLinks(WriteDoc &doc, const QVector<int> &sheets, const SheetLayout &layout = SheetLayout());

} // namespace jp::write
