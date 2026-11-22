#pragma once
// The outline text goes round for a Tight or Through wrap (DrawingML's wrap polygon): its points are shares of the object's frame (0 to 1 across and down, a little outside
// it allowed), so the outline follows the frame when it is sized. A file's outline is bounded here, once, for every reader and for the editing commands.

#include <QImage>
#include <QList>
#include <QPainterPath>
#include <QPolygonF>
#include <QSizeF>
#include <QTransform>
#include <QVector>
#include <atomic>
#include <memory>
#include <mutex>

namespace jp {

// The most points an outline has (a file that has more has its first ones) and how far outside the frame a point may be, as a share of the frame.
constexpr int kMostWrapPoints = 128;
constexpr double kWrapOutlineReach = 0.5;
// The most room a wrap leaves round an object, in points (a distance from a file is held to 0 up to this, nan being 0).
constexpr double kMostWrapDistance = 1584;
// What a layer sets text around at most, the page layer and the paginator alike: a box's text goes round at most kMaxWrapObstacles objects, found among the next kMaxWrapCandidates (more would
// be a page where every box wraps around every other), and no more than kMaxWrapShapesPerPage outlines are made for a page's painting or a column's lines;
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
// What the room round an outline costs, in counted work (points flattened plus segments built; nothing is timed). The room is never made by the path clipper or the stroker: it is a
// list of short segments, and the text engine asks it one thing, which horizontal span of a line's band is blocked, from those segments alone (a band costs one pass over them).
// An outline of more than kMostWrapOutlineSegments points is read into at most kWrapProfileBands bands (the extent of each, a rectangle); a path of more than kMostWrapPathElements
// elements is its bounding rectangle; a room of more than kWrapRoomRowPieces pieces is read once into kWrapRoomRows rows (fewer, at least 32, when that would cost more than kWrapRoomRowWork segment tests), charged to the budget when it is made, and answers from them (a row's span: never less than the exact one, at most a row's height more). A page's budget (one pass, or one column of the
// paginator) and a call's budget (outside a pass) are WrapBudget; past it, an object is its rectangle.
constexpr int kMostWrapOutlineSegments = 512, kWrapProfileBands = 128, kMostWrapPathElements = 20000;
constexpr qint64 kMostWrapWorkPerPage = 400000, kMostWrapWorkPerCall = 200000;
constexpr int kWrapRoomRows = 256, kWrapRoomRowPieces = 64;
constexpr qint64 kWrapRoomRowWork = 60000;
constexpr int kMostWrapShapesPerCall = 64;
struct WrapBudget {
    qint64 work = 0;
    int shapes = 0;
    static WrapBudget forPage() { return {kMostWrapWorkPerPage, kMaxWrapShapesPerPage}; }
    static WrapBudget forCall() { return {kMostWrapWorkPerCall, kMostWrapShapesPerCall}; }
};

// The room round one outline placed on the page: the outline's inside and a band of `distance` round it (what the stroke of that width with bevel joins and the outline make
// together; a distance of 0 or less is the outline alone, its inside solid: one that crosses itself has no holes). Immutable once made, shared by the text boxes that it is in front of.
class WrapRoom {
public:
    // The room of the filled rings (each a closed outline; points that are not numbers dropped, coordinates held to 1e7) or, with `open`, of lines `halfWidth` thick (a line's
    // own thickness) with distance beyond that. Null when the budget has not enough left (the caller sets the object round its rectangle); an empty room (no ring of three points
    // or no length) answers no span. The budget is charged what was done.
    static std::shared_ptr<const WrapRoom> around(const QList<QPolygonF> &rings, double distance, WrapBudget &budget, bool open = false, double halfWidth = 0);
    // The room of a path: its flattened subpaths, as above (a path of more than kMostWrapPathElements elements is the rectangle of its control points).
    static std::shared_ptr<const WrapRoom> around(const QPainterPath &outline, double distance, WrapBudget &budget);
    // A rectangle's room, with the band.
    static std::shared_ptr<const WrapRoom> ofRect(const QRectF &rect);
    QRectF bounds() const { return m_bounds; }
    bool isEmpty() const { return m_pieces.isEmpty(); }
    // The span of the room within the band from y0 to y1 (the leftmost and rightmost it reaches there); false when it is not in the band. `work` is counted.
    bool spans(double y0, double y1, double *left, double *right) const;
    // Whether the room holds the whole of the rectangle (looked for along lines across it, at most 17 of them).
    bool covers(const QRectF &rect) const;
    // The room moved by `matrix`.
    std::shared_ptr<const WrapRoom> mapped(const QTransform &matrix) const;
    // A number that changes when the room's shape does, for a layout's key.
    double signature() const { return m_signature; }
    qint64 segments() const { return m_points.size(); }
    // How many segment tests the room has answered with (counted, for tests).
    qint64 queryWork() const { return m_queryWork.load(); }
    // How many rooms were made, and the work they were charged, since the program began (counted, for tests).
    static qint64 madeForTest();
    static qint64 workForTest();
    // The segment tests every room has answered on this thread, since it began (a page's layouts are one thread's: the difference over a page is the work of its questions).
    static qint64 queryWorkOnThread();

private:
    struct Piece { int first, count; double top, bottom; };
    WrapRoom() = default;
    void finish();
    void buildRows() const;
    int rowCount() const;
    qint64 rowWork(int rows) const;
    bool rowSpans(double y0, double y1, double *left, double *right) const;
    QVector<QPointF> m_points;   // the pieces' corners, one piece after the other (a piece of two points is one segment, of three or four a convex shape)
    QVector<Piece> m_pieces;
    QVector<QPointF> m_ring;     // the filled rings' points, one after the other
    QVector<int> m_ringEnds;
    QRectF m_bounds;
    double m_signature = 0;
    mutable std::atomic<qint64> m_queryWork{0};
    mutable std::once_flag m_rowsOnce;
    mutable QVector<double> m_rowLeft, m_rowRight;   // by row, from the bounds' top down (left > right: nothing in the row)
    mutable double m_rowStep = 0;
};

// One thing a text frame's lines keep out of: a polygon (a rectangle for a box, a wall) or a room, the room's span within a line's band being asked of it. Frame-local.
// `wall` (when it has points) goes with a room: the one-sided wall that keeps the text to one side of it, a rectangle (turned with the frame).
struct WrapObstacle {
    QPolygonF poly;
    std::shared_ptr<const WrapRoom> room;
    QPointF shift;
    QPolygonF wall;
    WrapObstacle() = default;
    WrapObstacle(const QPolygonF &p) : poly(p) {}
    WrapObstacle(std::shared_ptr<const WrapRoom> r, const QPolygonF &w = QPolygonF()) : room(std::move(r)), wall(w) {}
    QRectF boundingRect() const;
    void translate(double dx, double dy);
    // The span of this within the band from y0 to y1 (frame-local), false when there is none (a shape with no area is none).
    bool spans(double y0, double y1, double *left, double *right) const;
    bool isPlain() const;   // (a rectangle: four corners, or five with the first again)
    double signature() const;
};

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
