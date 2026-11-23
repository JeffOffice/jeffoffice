#pragma once
// Border painting: the sides of table cells that a pointer's path covers. A border painter drags along the lines between cells (or the table's edge); each
// side of a cell that the path runs along, for at least half of the side's length, is one the pen's line goes on (or comes off, with the eraser). The geometry
// is here, apart from any table model, so every app that has cells with boxes can use it.

#include <QPointF>
#include <QRectF>
#include <QtGlobal>
#include <vector>

namespace jp::flow {

enum class CellSide : quint8 { Top, Left, Bottom, Right };

struct PaintCell {
    QRectF box;      // the cell's box, in the same space as the path
    int key = 0;     // the caller's name for the cell, given back with each side
    bool continued = false, continues = false;   // the cell goes on from the page above, or below the page's foot: that end of it is not an edge of the cell
};

struct PaintedSide {
    int key = 0;
    CellSide side = CellSide::Top;
};

// The most pieces (segments between two points) of a path that are looked at: a longer path is cut there, and the rest of it paints nothing.
inline constexpr int kMostPaintPieces = 4096;
// The most steps of looking (a square of the grid, a cell, a piece against a side) one path is given: the work of a drop is bounded by this, whatever the table's size.
inline constexpr qint64 kMostPaintComparisons = 1 << 20;

// The sides of `cells` that `path` runs along: the path comes within `reach` of the side's line over at least half the side. One point is a path too, with
// nothing to run along unless a side is shorter than `reach` twice. `work`, when given, is how many steps of looking were taken (never more than kMostPaintComparisons: the path is cut there).
std::vector<PaintedSide> sidesUnderPath(const std::vector<PaintCell> &cells, const std::vector<QPointF> &path, double reach, qint64 *work = nullptr);

} // namespace jp::flow
