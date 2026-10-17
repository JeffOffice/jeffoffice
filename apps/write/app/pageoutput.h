#pragma once
// Pages on a sheet: the one way JeffWrite puts its pages on paper, for the PDF export (a PDF writer) and for printing (a printer, to a printer or to a file).
// Both draw each page with the page view's painter (app/pagepaint.h), one page to a sheet, the sheet the size of the page's section and turned as it is.

#include "core/writedoc.h"

#include <QPagedPaintDevice>
#include <QVector>

namespace jp::write {

// Draws the pages (indexes from 0), in the order given, a sheet each, on the device. False when nothing could be drawn. `pageCount` is what the pages say they
// are of (NUMPAGES).
bool paintSheets(WriteDoc &doc, QPagedPaintDevice *device, const QVector<int> &pages, int pageCount, bool grayscale = false);

} // namespace jp::write
