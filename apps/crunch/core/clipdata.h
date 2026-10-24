#pragma once
// What Copy and Cut put on the clipboard, and what Paste reads from it. Besides the tab-separated text and the HTML table other programs take, the clipboard holds the cells whole (their
// values, formulas as they were written for the cell they came from, formats, and merged ranges) in a form of this program's own, so that a paste in the workbook they were copied from
// puts back everything, and one in another workbook what both can hold. Text and HTML from other programs are read into a table of texts, which are entered as if typed.

#include "sheet/calculation.h"

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QVector>

#include <vector>

namespace jp::crunch {

constexpr const char *kClipMime = "application/x-jeffcrunch-cells";
// Most cells made from what another program puts on the clipboard as text or an HTML table (a span of columns counts for all its cells): what is past it is not read, and the paste says
// that it is too much. A paste is at most a tenth of this.
constexpr qint64 kMaxClipCells = 1000000;

struct ClipCell {
    quint32 dRow = 0, dCol = 0;                          // from the block's first cell
    jp::office::sheet::Calculation::CellContent content; // formulas as written for the cell they were copied from
    QString formatCode;                                  // the number format's code ("General" for none): a workbook that does not have the formats takes this
    jp::calc::Value shown;                               // what the cell showed when it was copied: a formula's result
};

struct ClipData {
    qint64 token = 0;                                    // the workbook it was copied in (0 for none)
    qint64 serial = 0;                                   // and which copy it was there (a cut is pasted as a move only while it is the last copy)
    int sheet = 0;
    jp::office::sheet::CellRange source;                 // where the block was, as selected
    quint32 rows = 0, cols = 0;                          // the block that is copied: the selection, less the rows and columns past the last cell the sheet has
    bool cut = false;
    std::vector<ClipCell> cells;                         // those that exist
    std::vector<jp::office::sheet::CellRange> merges;    // inside the block, from its first cell
    bool valid() const { return rows > 0 && cols > 0; }
    QByteArray toBytes() const;
    static bool fromBytes(const QByteArray &bytes, ClipData *out);
};

// Text a program puts on the clipboard for a block of cells: rows on lines, cells apart by tabs, a cell with a tab, a line break, or a quote in it in quotes (a quote doubled); and an
// HTML table. The table's cells as read back are the rows of texts.
QVector<QStringList> parseTabSeparated(const QString &text);
QVector<QStringList> parseHtmlTable(const QString &html);

} // namespace jp::crunch
