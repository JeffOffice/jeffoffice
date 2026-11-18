#pragma once
// Text boxes drawn with the mouse, and the direction of a box's words (see jeffwrite.md).

#include "core/pictureops.h"
#include "core/textdirection.h"

#include <QPointF>

namespace jp::write {

// A text box in the Simple design's look, drawn by dragging from one point of the page to another (points): the box is put in at the cursor between the two corners (put in order), floating
// in front of the text, as one step that undoes; with `keepProportions` (Shift) it is as wide as it is tall. A drag of less than 4 points either way gives the design's usual size from `from`; otherwise each side is at least 18 points.
PictureResult insertTextBoxAt(WriteDoc &doc, QTextCursor &c, const QPointF &from, const QPointF &to, bool keepProportions = false);
// The rectangle insertTextBoxAt puts the box in (its two corners).
std::pair<QPointF, QPointF> textBoxDragEnds(const QPointF &from, const QPointF &to, bool keepProportions);

// How a text box's (or shape's) words run: across, or turned a quarter either way.
using TextDirection = jp::TextDirection;
// The direction the drawn object after `position` says (Horizontal for anything else).
TextDirection textBoxDirection(const WriteDoc &doc, int position);
// Sets it, as one step that undoes: only the body's `vert` (a box of the older drawing format, its `layout-flow`) is written again, the rest of the drawing is kept; the frame is as it was. A
// box that is linked on keeps its link. False (and no step) when the object takes no words, is locked, or already runs so.
bool setTextBoxDirection(WriteDoc &doc, int position, TextDirection direction);

} // namespace jp::write
