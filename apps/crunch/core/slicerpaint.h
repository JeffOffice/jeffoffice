#pragma once
// How a slicer looks on a sheet (core/sheetslicers.h): a frame with a caption and a clear-filter button over a grid of buttons, drawn in the grid's units (pixels at 100%) inside the rectangle its
// anchor gives, and where a point of it is (what a click on it means). One plain look for now: a white face, a blue button for an item the table shows, a pale one for an item no record has.

#include "core/slicerface.h"

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
    QRectF button(int index) const;     // where the index'th button shown is (from 0, row by row)
};
SlicerGeometry slicerGeometry(const QSizeF &size, const SlicerFace &face);

enum class SlicerPart : quint8 { None, Header, Clear, Button };
struct SlicerHit {
    SlicerPart part = SlicerPart::None;
    int index = 0;   // for a button: its place among those shown (from 0)
};
SlicerHit slicerHit(const SlicerGeometry &geometry, const SlicerFace &face, const QPointF &at);

// Draws the slicer in `rect`. `usable` false: the slicer's field is not one that can be read, so it is drawn as an empty frame with its name.
void paintSlicer(QPainter *painter, const QRectF &rect, const SlicerFace &face, bool usable);

} // namespace jp::crunch
