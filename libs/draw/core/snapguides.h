#pragma once
// Snapping for an object that a person drags or sizes by the mouse: to a drawing grid, and to alignment guides (the edges and centers of the other objects,
// the page's edges and center, and the margins). Plain geometry in the units of the caller (points on a page); the caller works out how far "near" is in
// its own units from the zoom, and draws the guide lines the result tells.

#include <QRectF>
#include <QVector>

#include <optional>

namespace jp {

struct SnapOptions {
    bool showGrid = false;      // the grid's dots are drawn
    bool snapGrid = false;      // edges go to the grid
    bool snapObjects = false;   // edges and centers go to the alignment guides
    double spacing = 6;         // the grid's spacing, in points
    bool operator==(const SnapOptions &o) const { return showGrid == o.showGrid && snapGrid == o.snapGrid && snapObjects == o.snapObjects && spacing == o.spacing; }
    bool snapping() const { return snapGrid || snapObjects; }
};

constexpr double kDefaultSnapSpacing = 6;
constexpr double kLeastSnapSpacing = 1;
constexpr double kMostSnapSpacing = 720;

// The places an edge or a center of a dragged object may go to: x for vertical guide lines, y for horizontal ones.
struct SnapTargets {
    QVector<double> xs, ys;
};

// The page's own: its edges and center, and the edges of its margins.
void addPageTargets(SnapTargets &t, const QSizeF &page, const QMarginsF &margins);
// The edges and the center of an object's box.
void addBoxTargets(SnapTargets &t, const QRectF &box);

// What a snap did: the box as moved, and the lines to show (the place of a vertical line, of a horizontal one) where an alignment guide took an edge or a center.
struct SnapResult {
    QRectF box;
    std::optional<double> lineX, lineY;
    bool movedX = false, movedY = false;   // an edge went to a guide or to the grid
};

// A box carried by `box`'s own edges and center: each axis goes to the nearest target within `within`, else (with a grid) to the grid by whichever of its two edges is nearer to it.
// `gridSpacing` 0 is no grid; `targets` null is no guides.
SnapResult snapMovedBox(const QRectF &box, const SnapTargets *targets, double gridSpacing, double within);

// The same for the sides that move in a size: `left`, `right`, `top`, and `bottom` say which. Each moving side goes to the nearest target within `within`, else to the grid.
// A side is never taken past the one opposite it (the box keeps `least` of size).
SnapResult snapSizedBox(const QRectF &box, bool left, bool right, bool top, bool bottom, const SnapTargets *targets, double gridSpacing, double within, double least);

} // namespace jp
