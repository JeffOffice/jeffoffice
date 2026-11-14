#pragma once
// Edit Points and connection sites on a shape's outline, for every app: the points of a QPainterPath (a corner, or where a curve ends), moving one (its curve handles go with it),
// taking one away, putting one in; and where a connector meets the outline. All in the outline's own points.

#include <QPainterPath>
#include <QPointF>
#include <QSizeF>
#include <QVector>

namespace jp::pts {

// What an outline may hold to be edited: more elements than this, or a custom geometry of more bytes (DrawingML's own text), is left as it is and never parsed for its points.
constexpr int kMostPoints = 2000;
constexpr int kMostElements = kMostPoints * 3 + 8;
constexpr qsizetype kMostGeometryBytes = 300000;

// The elements of the path that are points (a move-to, a line-to, or where a curve ends); a subpath's closing line back to its start is not one.
QVector<int> vertices(const QPainterPath &path);
// The same points, as places.
QVector<QPointF> vertexPoints(const QPainterPath &path);
// Whether the path is small enough to be edited point by point.
bool withinCaps(const QPainterPath &path);
// The path with the point at element `i` moved by `delta` (curve handles next to it move along, so curves keep their shape).
QPainterPath moved(const QPainterPath &path, int i, const QPointF &delta);
// The path without the point at element `i`; the path itself when its subpath would be left with fewer than three points.
QPainterPath removed(const QPainterPath &path, int i);
// The path with a point put on the edge nearest to `q` (within `tol`), and in `*at` the element of the new point; the path itself and -1 when no edge is near.
QPainterPath inserted(const QPainterPath &path, const QPointF &q, double tol, int *at);

// Where a connector meets an outline of a frame of `size`: the places a line from the middle of the frame meets the outline going up, left, down, and right (the middle of the side where it
// meets nothing). On a rectangle or a diamond they are the middles of the sides; on a star or a heart they are on the shape and not on the box round it.
QVector<QPointF> sideSites(const QPainterPath &outline, const QSizeF &size);

} // namespace jp::pts
