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
};

struct PaintedSide {
    int key = 0;
    CellSide side = CellSide::Top;
};

// The most pieces (segments between two points) of a path that are looked at: a longer path is cut there, and the rest of it paints nothing.
inline constexpr int kMostPaintPieces = 4096;

// The sides of `cells` that `path` runs along: the path comes within `reach` of the side's line over at least half the side. One point is a path too, with
// nothing to run along unless a side is shorter than `reach` twice. `work`, when given, is how many (piece, side) pairs were compared.
std::vector<PaintedSide> sidesUnderPath(const std::vector<PaintCell> &cells, const std::vector<QPointF> &path, double reach, qint64 *work = nullptr);

} // namespace jp::flow
