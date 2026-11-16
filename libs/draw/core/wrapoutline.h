#pragma once
// The outline text goes round for a Tight or Through wrap (DrawingML's wrap polygon): its points are shares of the object's frame (0 to 1 across and down, a little outside
// it allowed), so the outline follows the frame when it is sized. A file's outline is bounded here, once, for every reader and for the editing commands.

#include <QPainterPath>
#include <QPolygonF>
#include <QSizeF>
#include <QVector>

namespace jp {

// The most points an outline has (a file that has more has its first ones) and how far outside the frame a point may be, as a share of the frame.
constexpr int kMostWrapPoints = 128;
constexpr double kWrapOutlineReach = 0.5;
// The unit DrawingML gives a polygon's coordinates: the frame's width and height are each this many.
constexpr double kWrapPolygonUnits = 21600.0;

// The points kept as an outline: at most kMostWrapPoints (the first), a point that is not a number dropped, each clamped to the reach; fewer than three left makes no outline (empty).
QPolygonF boundedWrapOutline(const QVector<QPointF> &points);
// The room the text leaves round an outline already placed on the page: the outline and a band of `distance` round it (a distance of 0 or less is the outline alone). The page layer
// and the paginator both take this, so that a line meets the same edge in both.
QPainterPath wrapRoomAround(const QPainterPath &outline, double distance);

// The outline with point `index` moved to `to` (shares of the frame; kept inside the reach); unchanged for a number that is not one of its points.
QPolygonF wrapOutlineMoved(const QPolygonF &outline, int index, const QPointF &to);
// The outline with a point put on its edge nearest `near` (shares of the frame), where that edge is within `tolerance` of it, both measured in the unit of `frame`, the frame's size; unchanged
// when no edge is near enough or the outline has kMostWrapPoints already. `inserted` gets the new point's number, or -1.
QPolygonF wrapOutlineWithPoint(const QPolygonF &outline, const QPointF &near, double tolerance, const QSizeF &frame, int *inserted = nullptr);
// The outline without point `index`; unchanged when that would leave fewer than three.
QPolygonF wrapOutlineWithout(const QPolygonF &outline, int index);

} // namespace jp
