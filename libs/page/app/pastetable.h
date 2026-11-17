#pragma once
// A table on the clipboard as HTML (cells copied from JeffCrunch, a table copied from a document, or a table from another program's page) read into a table that JeffPub and JeffDeck
// hold: the text of each cell with its fonts, sizes, bold, italic, lines, and colors; the fill; the four borders; how the text sits; merged cells; and the width of each column when the page
// gives one. Nothing is fetched. A clipboard is held to bounds, so a hostile one costs a bounded amount: kPasteMaxHtmlBytes of HTML, kPasteMaxRows rows, kPasteMaxColumns columns, and
// kPasteMaxCells cells (and, in the HTML itself, a span of no more than kPasteMaxColumns columns or kPasteMaxRows rows, which is looked at before the page is parsed).

#include "core/items.h"

#include <QColor>
#include <QString>
#include <QTextCharFormat>
#include <QVector>

namespace jp {

class Document;

constexpr int kPasteMaxRows = 1000, kPasteMaxColumns = 64, kPasteMaxCells = 100000, kPasteMaxHtmlBytes = 4 * 1024 * 1024, kPasteMaxCellChars = 4000;

struct PastedRun {
    QString text;
    QTextCharFormat format;   // only what the HTML said: family, size, weight, italic, underline, strike-through, color
};
struct PastedParagraph {
    QVector<PastedRun> runs;
    Qt::Alignment align = Qt::AlignLeft;
};
struct PastedCell {
    QVector<PastedParagraph> paragraphs;
    bool origin = false;        // a cell of its own (not one a neighbor spans over, not a hole)
    int rowSpan = 1, colSpan = 1;
    bool hasFill = false;
    QColor fill;
    CellBorder border;
    VAlign valign = VAlign::Top;
    bool hasValign = false;
};
struct PastedTable {
    int rows = 0, cols = 0;
    QVector<PastedCell> cells;       // row-major
    QVector<double> colWidths;       // points; 0 where the page said nothing
    bool truncated = false;          // a cell's text was cut to kPasteMaxCellChars
    PastedCell &cell(int r, int c) { return cells[r * cols + c]; }
    const PastedCell &cell(int r, int c) const { return cells[r * cols + c]; }
};

enum class PastedTableResult {
    NotATable,   // no HTML, no table, more than one, words around the table, or fewer than two cells: paste as before
    Ok,
    TooBig,      // past a bound: the clipboard is pasted as plain text
};
// The one table in the HTML (and nothing else in it but blank lines).
PastedTableResult readPastedTable(const QString &html, PastedTable *out);

// A table of the pasted cells for a page of `page` size (its own look: no table style), centered in the upper part of the page, with the column widths the page gave or the text needs, scaled to
// fit the page's width. Its stories are made in `doc`.
std::shared_ptr<TableItem> makePastedTable(Document *doc, const PastedTable &pasted, const QSizeF &page);
// One pasted cell written over a cell of a table (its text and look; the target's merge stays as it is).
void writePastedCell(Document *doc, TableCell &target, const PastedCell &source);

} // namespace jp
