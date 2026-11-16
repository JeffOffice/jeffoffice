#pragma once
// The outline of a tight or through wrap, as a drawing keeps it (DrawingML's wp:wrapPolygon: a start and a run of line-tos in units of 1/21600 of the frame) and as the
// layers above read it: shares of the frame, bounded (core/wrapoutline.h).

#include "wml/xmltree.h"

#include <QByteArray>
#include <QPolygonF>

namespace jp::office::wml {

// The outline of the wrap element `wrap` (a wp:wrapTight or wp:wrapThrough) of the tree; empty when it has none, or has fewer than three points. At most kMostWrapPoints points
// are read (what follows is not looked at), each clamped.
QPolygonF readWrapPolygon(const XmlTree &t, int wrap);
// The same for the bytes of a wrap element as written (its prefixes need not be declared).
QPolygonF readWrapPolygonXml(const QByteArray &wrapElement);
// A wp:wrapPolygon of the outline (shares of the frame; `edited` as the file says it), for a wrap element to hold.
QByteArray wrapPolygonXml(const QPolygonF &outline, bool edited = true);

} // namespace jp::office::wml
