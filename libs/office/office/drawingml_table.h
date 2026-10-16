#pragma once
// Tables in DrawingML (ECMA-376 part 1, 20.1.4): the p:graphicFrame that holds
// an a:tbl becomes a placed table (jp::TableItem) and back. Each cell is
// read whole: its text is a story, its fill, borders, margins, and anchor are
// the cell's own, and what the table's style (a:tableStyleId, defined in the
// presentation's tableStyles part) gives the cell is put into those, and into
// the text's formatting, so the table is drawn right without knowing styles.
// The style's id is kept in TableItem::format to name it again when written.

#include "office/drawingml.h"
#include "office/drawingml_write.h"

#include <QColor>
#include <QDomElement>
#include <QXmlStreamWriter>
#include <optional>
#include <vector>

namespace jp::office {

// What a table style gives each cell of a table: the fill and the text's bold, italic, and color (an invalid color or no value:
// the style says nothing), and the line on each edge. Rows and columns count from 0; `above(row, col)` is the line along the top of
// a cell (`above(rows, col)` along the bottom of the last row) and `leftOf(row, col)` along its left side.
struct TableStyleEffect {
    int rows = 0, cols = 0;
    std::vector<Fill> fill;
    std::vector<std::optional<bool>> bold, italic;
    std::vector<QColor> color;
    std::vector<Stroke> across, down;   // (rows + 1) * cols lines, and rows * (cols + 1)
    Stroke &above(int row, int col) { return across[size_t(row) * cols + col]; }
    Stroke &leftOf(int row, int col) { return down[size_t(row) * (cols + 1) + col]; }
    const Stroke &above(int row, int col) const { return across[size_t(row) * cols + col]; }
    const Stroke &leftOf(int row, int col) const { return down[size_t(row) * (cols + 1) + col]; }
};

// What the style with that id (one of tablestyles.h's) gives the cells of `table`, with the parts it has turned on (header row, total
// row, first and last column, banded rows and columns) and the colors of `theme`. False when the id is not one of those styles.
bool tableStyleEffect(const QString &styleId, const TableItem &table, const Theme &theme, TableStyleEffect *out);

// Whether the graphic frame (p:graphicFrame) holds a table: its
// a:graphicData names the table's URI and has an a:tbl in it.
bool isTableFrame(const QDomElement &graphicFrame);

// The table in a graphic frame; null if the frame holds none, or one that
// can't be a table here (no columns or rows, more cells than a table is
// kept to, or more than ctx.budget has cells left). `styles` is as for
// readTextBody: the list styles the cells' text inherits. The table styles
// are `tableStyles` (the root, a:tblStyleLst, of the presentation's
// tableStyles part) if given, else read from the part the package's main part
// links to; a style with no definition there is approximated by the usual
// one (kBuiltinStyles in drawingml_table.cpp).
ItemPtr readTable(const QDomElement &graphicFrame, const DrawingContext &ctx, const TextStyleStack &styles = {},
                  const QDomElement &tableStyles = QDomElement());

// The table as a p:graphicFrame, with the shape number ctx.nextShapeId (then
// counted up). The cells carry their own fills, borders, margins, and
// anchors; the style named in TableItem::format is named in a:tblPr. Nothing,
// and no shape number used, for a table with no rows or no columns.
void writeTable(QXmlStreamWriter &w, const TableItem &t, DrawingWriteContext &ctx);

} // namespace jp::office
