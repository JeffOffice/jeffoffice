#pragma once
// AutoShapes. Each shape builds a QPainterPath for a w×h box from optional
// adjustment values (the yellow diamond handles), plus the rectangle where
// text sits when the shape holds text.

#include "core/presetgeom.h"

#include <QPainterPath>
#include <QString>
#include <QVector>

namespace jp {

// A yellow adjustment handle. A handle of the standard's (a preset shape's: `standard` is its place in the preset's list of handles) is where the
// preset's definition puts it and moves the adjustments it names: a point handle its x and y (`indexX`, `indexY`), a polar one its radius (`indexX`)
// and angle (`indexY`), each -1 where it moves none. Another handle slides along one axis; its position along that axis is adj * basis (measured
// from the far edge when fromEnd).
struct AdjHandle {
    int index = 0;         // the adjustment it moves (a standard handle's first one)
    int standard = -1;
    bool polar = false;
    int indexX = -1, indexY = -1;
    QString preset;
    int axis = 0;          // 0 = moves horizontally, 1 = vertically
    int basis = 2;         // 0 = width, 1 = height, 2 = min(width, height)
    bool fromEnd = false;
    double other = 0;      // fixed position on the other axis, as a fraction of that dimension
    double min = 0, max = 0.5;
    double limit = 0;      // above 0: the handle stays inside this fraction of the box along its axis (the shape's own range depends on the box's proportions)
};

using PathFn = QPainterPath (*)(double w, double h, const QVector<double> &a);
using RectFn = QRectF (*)(double w, double h, const QVector<double> &a);

struct ShapeDef {
    QString id;
    QString name;
    QString category;
    QString preset;              // the standard's name for it; its drawing, text rectangle, adjustments, and handles are the standard's (core/presetgeom.h)
    PathFn path = nullptr;       // a shape that is no preset of the standard's: its drawing
    QVector<double> defaults;
    QVector<AdjHandle> handles;
    RectFn textRect = nullptr;   // nullptr = inset by `inset`
    double inset = 0.0;
    bool open = false;           // an outline with no fill by default (arcs, brackets, braces)
    // An action button: only a presentation offers it, and a new one starts with the link it is for ("#next" and the like; none for the rest).
    bool slideAction = false;
    QString link;
};

const QVector<ShapeDef> &shapeLibrary();
const ShapeDef *shapeDef(const QString &id);

// What is stroked: every path of the shape's definition, or an open shape's outline paths only.
QPainterPath shapePath(const QString &id, const QSizeF &size, const QVector<double> &adj = {});
// The shape's paths each with how it is drawn: filled (or not, or lightened or darkened) and stroked (or not). A shape that is no preset has one.
QVector<GeometryPath> shapeParts(const QString &id, const QSizeF &size, const QVector<double> &adj = {});
QRectF shapeTextRect(const QString &id, const QSizeF &size, const QVector<double> &adj = {});
QVector<double> shapeAdj(const QString &id, const QVector<double> &adj);   // fills in defaults
// A save from before the library drew the standard's shapes (no "adjModel" in it) holds adjustments in the old meanings: a slant as a fraction of the width, a callout's tip
// from the top left, and the old defaults for what it left out. This gives the values that draw the same shape now. `size` is the shape's frame.
QVector<double> adjFromOldLibrary(const QString &id, const QSizeF &size, const QVector<double> &adj);
// A save of "adjModel" 2 (0.42): the arcs, pies, chords, block arcs, snipped and corner shapes, U-turn arrows, and arrows with heads of their own length held the old library's
// meanings; this gives the standard's that draw the same shape.
QVector<double> adjFromModel2Library(const QString &id, const QSizeF &size, const QVector<double> &adj);
// The library's shape for a preset of the standard's and the other way ("" for none).
QString shapeIdOfPreset(const QString &prst);
QString presetOfShapeId(const QString &shapeId);
QPointF adjHandlePos(const AdjHandle &h, const QSizeF &size, const QVector<double> &adj);
// Moves the handle to `local` (a point in the shape's frame): sets the adjustments it moves in `adj` (the library's, all of them) to what puts it there, held to the handle's range.
void adjDrag(const AdjHandle &h, const QSizeF &size, QVector<double> &adj, const QPointF &local);
// The handle's first adjustment for a drag to `local`; `adj` are the others (the defaults when not given).
double adjFromPoint(const AdjHandle &h, const QSizeF &size, const QPointF &local, const QVector<double> &adj = {});

} // namespace jp
