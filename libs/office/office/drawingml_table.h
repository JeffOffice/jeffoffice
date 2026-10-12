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

#include <QDomElement>
#include <QXmlStreamWriter>

namespace jp::office {

// Whether the graphic frame (p:graphicFrame) holds a table: its
// a:graphicData names the table's URI and has an a:tbl in it.
bool isTableFrame(const QDomElement &graphicFrame);

// The table in a graphic frame; null if the frame holds none, or one that
// can't be a table here (no columns or rows, more cells than a table is
// kept to). `styles` is as for readTextBody: the list styles the cells' text
// inherits. The table styles are `tableStyles` (the root, a:tblStyleLst, of
// the presentation's tableStyles part) if given, else read from the part the
// package's main part links to; a style with no definition there is
// approximated, see tableStyleOf in drawingml_table.cpp.
ItemPtr readTable(const QDomElement &graphicFrame, const DrawingContext &ctx, const TextStyleStack &styles = {},
                  const QDomElement &tableStyles = QDomElement());

// The table as a p:graphicFrame, with the shape number ctx.nextShapeId (then
// counted up). The cells carry their own fills, borders, margins, and
// anchors; the style named in TableItem::format is named in a:tblPr.
void writeTable(QXmlStreamWriter &w, const TableItem &t, DrawingWriteContext &ctx);

} // namespace jp::office
