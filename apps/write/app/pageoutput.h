#pragma once
// Pages on a sheet: the one way JeffWrite puts its pages on paper, for the PDF export (a PDF writer) and for printing (a printer, to a printer or to a file).
// Both draw each page with the page view's painter (app/pagepaint.h), one page to a sheet, the sheet the size of the page's section and turned as it is.

#include "core/writedoc.h"

#include <QPagedPaintDevice>
#include <QString>
#include <QStringList>
#include <QVector>

class QComboBox;

namespace jp::write {

// Draws the pages (indexes from 0), in the order given, a sheet each, on the device. False when nothing could be drawn. `pageCount` is what the pages say they
// are of (NUMPAGES).
// With a layout, several pages share a sheet (2, 4, 6, 9, or 16, in rows from the top left, each scaled to fit its place and framed with a thin line), and/or each page is
// scaled to fit a paper size, centered on it; a sheet that holds two or six pages is turned to lie on its side.
struct SheetLayout {
    int perSheet = 1;
    QSizeF paper;         // the sheet's size in points, upright (empty: each page's own)
};
// The columns and rows that a number of pages on a sheet makes (1 for any number that isn't one of 1, 2, 4, 6, 9, 16).
QSize sheetGrid(int perSheet);
bool paintSheets(WriteDoc &doc, QPagedPaintDevice *device, const QVector<int> &pages, int pageCount, bool grayscale = false, const SheetLayout &layout = SheetLayout());

// The choices of the Print page's printer box: each printer, by its name, and last "Save as PDF". The entries are told apart by their role, not by their text, so a
// printer that is called "pdf" (or "Save as PDF") is a printer. `current` is the printer to start on.
void fillPrinterBox(QComboBox *box, const QStringList &printers, const QString &current);
struct PrinterChoice {
    QString printer;      // the printer's name (empty with a file)
    bool toFile = false;  // "Save as PDF"
};
PrinterChoice printerChoice(const QComboBox *box);

} // namespace jp::write
