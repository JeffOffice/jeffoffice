#pragma once
// How a slicer looks on a sheet (core/sheetslicers.h): a frame with a caption and a clear-filter button over a grid of buttons, drawn in the grid's units (pixels at 100%) inside the rectangle its
// anchor gives, and where a point of it is (what a click on it means). One plain look for now, in the colors of the light or the dark window: a button for an item the table shows is blue, one for an
// item no record has is dimmed. Buttons that do not fit are not drawn: the last row says how many more there are (and how many of those are chosen), and a click on it opens the Slicers pane.

#include "core/slicerface.h"

#include <QColor>
#include <QPainter>
#include <QPointF>
#include <QRectF>
#include <QSizeF>

namespace jp::crunch {

// The most buttons one slicer draws (a field has up to a million items): the rest are not made, drawn, or clicked.
constexpr int kMaxSlicerButtons = 400;

// Where the parts of a slicer are, in units from its top left.
struct SlicerGeometry {
    QRectF header, clear, body;
    int columns = 1;
    double buttonWidth = 0, buttonHeight = 0, gap = 3;
    int rows = 0;                       // the rows of buttons that fit
    int capacity() const { return rows * columns; }   // the buttons that fit
    int shown = 0;                      // the buttons drawn and clicked (those of the face that fit, one row less when some are left out, and at most kMaxSlicerButtons)
    QRectF moreLine;                    // where the line that says buttons are left out is (null when none are)
    QRectF button(int index) const;     // where the index'th button shown is (from 0, row by row)
};
SlicerGeometry slicerGeometry(const QSizeF &size, const SlicerFace &face);

enum class SlicerPart : quint8 { None, Header, Clear, Button, More };
struct SlicerHit {
    SlicerPart part = SlicerPart::None;
    int index = 0;   // for a button: its place among those shown (from 0)
};
SlicerHit slicerHit(const SlicerGeometry &geometry, const SlicerFace &face, const QPointF &at);

// The line that says how many items are not drawn ("52 more items", or "52 more items, 3 chosen" when some of them are chosen), and the sentence of a slicer that cannot be used here.
QString slicerMoreText(const SlicerFace &face, const SlicerGeometry &geometry);
QString slicerUnusableText(SlicerWhy why);

// The colors of a slicer in the light or the dark window: the button of each state is a face and an ink that read at 4.5 to 1 or better.
struct SlicerColors {
    QColor frame, face, ink, dim, edge, on, onInk, onDim, onDimInk, offDim, offDimInk, rule;
};
SlicerColors slicerColors(bool dark);

// Draws the slicer in `rect`; a slicer that cannot be used (face.unusable) is drawn with its caption and the sentence that says so.
void paintSlicer(QPainter *painter, const QRectF &rect, const SlicerFace &face);

} // namespace jp::crunch
