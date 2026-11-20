#pragma once
// The outline text goes round for a Tight or Through wrap (DrawingML's wrap polygon): its points are shares of the object's frame (0 to 1 across and down, a little outside
// it allowed), so the outline follows the frame when it is sized. A file's outline is bounded here, once, for every reader and for the editing commands.

#include <QImage>
#include <QPainterPath>
#include <QPolygonF>
#include <QSizeF>
#include <QVector>

namespace jp {

// The most points an outline has (a file that has more has its first ones) and how far outside the frame a point may be, as a share of the frame.
constexpr int kMostWrapPoints = 128;
constexpr double kWrapOutlineReach = 0.5;
// The most room a wrap leaves round an object, in points (a distance from a file is held to 0 up to this, nan being 0).
constexpr double kMostWrapDistance = 1584;
// What a layer sets text around at most, the page layer and the paginator alike: a box's text goes round at most kMaxWrapObstacles objects, found among the next kMaxWrapCandidates (more would
// be a page where every box wraps around every other), and no more than kMaxWrapShapesPerPage outlines are made (each costs about a millisecond) for a page's painting or a column's lines;
// the objects after that budget wrap around their rectangles.
constexpr int kMaxWrapObstacles = 32, kMaxWrapCandidates = 512, kMaxWrapShapesPerPage = 1024;
// The unit DrawingML gives a polygon's coordinates: the frame's width and height are each this many.
constexpr double kWrapPolygonUnits = 21600.0;

// The points kept as an outline: at most kMostWrapPoints (the first), a point that is not a number dropped, each clamped to the reach; fewer than three left makes no outline (empty).
QPolygonF boundedWrapOutline(const QVector<QPointF> &points);
// The most points an outline made from a shape has, and the most an outline made from a picture's transparency has; and the picture's longer side and its bands as it is scanned.
constexpr int kMostPathOutlinePoints = 24, kMostAlphaOutlinePoints = 64, kAlphaOutlineSide = 256, kAlphaOutlineBands = 32;
// The outline of a path drawn in a frame of `frame` (the path's own unit, points), as shares of the frame: its flattened edge with the corners kept and no more than kMostPathOutlinePoints,
// moved outward by the tolerance the points were simplified with (so that the text stays off the curve, not on the chords inside it), bounded as above. A path of more than one part, or with
// a hole, is drawn small and read as a picture is (kMostAlphaOutlinePoints at most; a hole is closed). Empty (the frame itself) for an empty path, a frame with no size, or a path of more than 20000 elements.
QPolygonF wrapOutlineOfPath(const QPainterPath &path, const QSizeF &frame);
// The outline of what a picture shows: the pixels with an alpha above 16, the largest alpha of each cell of a grid of at most kAlphaOutlineSide cells on the longer side (so that a line of one
// pixel counts), in kAlphaOutlineBands bands, the left and right edge of each joined into one outline of at most kMostAlphaOutlinePoints points. A picture of more than kMostAlphaScanPixels
// is read at every few pixels (the work is bounded); `scanned` gets how many pixels were looked at. Empty (the frame itself) for a picture with no transparency, with nothing in it, or with
// no alpha at all.
constexpr qint64 kMostAlphaScanPixels = 16000000;
QPolygonF wrapOutlineOfAlpha(const QImage &image, int *scanned = nullptr);
// The room the text leaves round an outline already placed on the page: the outline and a band of `distance` round it (a distance of 0 or less is the outline alone, its inside solid: an outline that crosses itself has no holes). The page layer
// and the paginator both take this, so that a line meets the same edge in both.
QPainterPath wrapRoomAround(const QPainterPath &outline, double distance);
// The wall of a rectangular room (the object and its distances) that keeps the text to one side: the one rectangle from the room's edge on the side the text is not on, out to
// 1e5 points that way, as tall as the room (never a join of two, which came out as a sliver for some places). The text is on the left only (`textLeft`), on the right only, or on the
// larger side of the room within `frame` (`largest`: the left when it is as wide as the right). The page layer and the paginator both take this.
QRectF wrapSideWall(const QRectF &room, const QRectF &frame, bool textLeft, bool largest);
// A wrap distance as the layers use it: 0 for a number that is not one or is below 0, at most kMostWrapDistance.
double boundedWrapDistance(double distance);

// The outline with point `index` moved to `to` (shares of the frame; kept inside the reach); unchanged for a number that is not one of its points.
QPolygonF wrapOutlineMoved(const QPolygonF &outline, int index, const QPointF &to);
// The outline with a point put on its edge nearest `near` (shares of the frame), where that edge is within `tolerance` of it, both measured in the unit of `frame`, the frame's size; unchanged
// when no edge is near enough or the outline has kMostWrapPoints already. `inserted` gets the new point's number, or -1.
QPolygonF wrapOutlineWithPoint(const QPolygonF &outline, const QPointF &near, double tolerance, const QSizeF &frame, int *inserted = nullptr);
// The outline without point `index`; unchanged when that would leave fewer than three.
QPolygonF wrapOutlineWithout(const QPolygonF &outline, int index);

} // namespace jp
