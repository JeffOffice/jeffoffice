#pragma once
// AutoShapes. Each shape builds a QPainterPath for a w×h box from optional
// adjustment values (the yellow diamond handles), plus the rectangle where
// text sits when the shape holds text.

#include <QPainterPath>
#include <QString>
#include <QVector>

namespace jp {

// A yellow adjustment handle. The handle slides along one axis; its position
// along that axis is adj * basis (measured from the far edge when fromEnd).
struct AdjHandle {
    int index = 0;
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
    PathFn path = nullptr;
    QVector<double> defaults;
    QVector<AdjHandle> handles;
    RectFn textRect = nullptr;   // nullptr = inset by `inset`
    double inset = 0.0;
    bool open = false;           // stroke only (arcs, brackets)
    // An action button: only a presentation offers it, and a new one starts with the link it is for ("#next" and the like; none for the rest).
    bool slideAction = false;
    QString link;
};

const QVector<ShapeDef> &shapeLibrary();
const ShapeDef *shapeDef(const QString &id);

QPainterPath shapePath(const QString &id, const QSizeF &size, const QVector<double> &adj = {});
QRectF shapeTextRect(const QString &id, const QSizeF &size, const QVector<double> &adj = {});
QVector<double> shapeAdj(const QString &id, const QVector<double> &adj);   // fills in defaults
// A save from before the library drew the standard's shapes (no "adjModel" in it) holds adjustments in the old meanings: a slant as a fraction of the width, a callout's tip
// from the top left, and the old defaults for what it left out. This gives the values that draw the same shape now. `size` is the shape's frame.
QVector<double> adjFromOldLibrary(const QString &id, const QSizeF &size, const QVector<double> &adj);
QPointF adjHandlePos(const AdjHandle &h, const QSizeF &size, const QVector<double> &adj);
double adjFromPoint(const AdjHandle &h, const QSizeF &size, const QPointF &local);

} // namespace jp
