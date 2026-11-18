#pragma once
// A shape's outline as the standard defines it (ECMA-376 20.1.9): adjustments and guides (named values that formulas make from the shape's size), the paths drawn with
// them, the rectangle for text, and the handles that change the adjustments. The preset shapes' definitions are a generated table (presetgeom_presets.inc, made by
// tools/gen_presetgeom.py); an a:custGeom of a file is read into the same form (libs/office/office/drawingml_geometry.cpp). This is the one place that evaluates either.

#include <QHash>
#include <QPainterPath>
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QString>
#include <QStringList>
#include <QVector>

namespace jp {

// One a:path, in points relative to the shape's frame.
struct GeometryPath {
    // How the path is filled: not at all, or as the shape's fill, or with that fill lightened or darkened (the shaded faces of a cube).
    enum Fill { Normal, None, Lighten, LightenLess, Darken, DarkenLess };
    QPainterPath path;
    Fill fill = Normal;
    bool stroke = true;
};

struct Geometry {
    QVector<GeometryPath> paths;
    // Where the shape's text sits when the geometry says so; null for the whole frame.
    QRectF textRect;
    bool isEmpty() const { return paths.isEmpty(); }
};

// A definition before any value is known.
struct GeomGuide {
    QString name;
    QStringList words;   // the formula, split: "*/", "w", "adj1", "100000"
};

struct GeomCommand {
    enum Kind { Move, Line, Arc, Quad, Cubic, Close } kind = Close;
    QString arg[6];
};

struct GeomPath {
    QString width, height;
    GeometryPath::Fill fill = GeometryPath::Normal;
    bool stroke = true;
    bool extrusionOk = true;
    QVector<GeomCommand> commands;
};

// What a handle moves along one axis: the adjustment's name and its range (numbers or guides' names).
struct GeomAxis {
    QString ref, min, max;
};

// A handle: a point handle (ahXY) moves the adjustments `first` (along x) and `second` (along y); a polar one (ahPolar) its radius (`first`) and its angle (`second`).
struct GeomHandle {
    bool polar = false;
    GeomAxis first, second;
    QString posX, posY;
};

struct GeomSpec {
    QVector<GeomGuide> adjust, guides;
    QString textRect[4];
    bool hasTextRect = false;
    QVector<GeomPath> paths;
    QVector<GeomHandle> handles;
};

// The geometry of a definition for a frame of `size` points. `adjust` holds adjustments by name, in the standard's units (25000 is a quarter); one the definition does not
// have is ignored, and one it has that is not given has its own value.
Geometry buildGeometry(const GeomSpec &spec, const QHash<QString, double> &adjust, const QSizeF &size);

// Where a handle is and what it may reach, for these adjustments: `min` and `max` hold the ranges of its two adjustments (the standard's units).
struct HandleGeometry {
    QPointF at;
    double min[2] = {0, 0}, max[2] = {0, 0};
};
bool evaluateHandle(const GeomSpec &spec, int handle, const QHash<QString, double> &adjust, const QSizeF &size, HandleGeometry *out);

// The standard's presets (the generated table), by name; null for one it does not have.
const GeomSpec *presetSpec(const QString &prst);
// The adjustments' names in the order the preset defines them, with their defaults (the standard's units).
QStringList presetAdjustmentList(const QString &prst);
QVector<double> presetAdjustmentDefaults(const QString &prst);
// By how much a library shape's adjustment `index` is smaller than the standard's: 100,000ths of a size, and 21,600,000ths of a turn for an angle (the library keeps
// these as fractions).
double presetAdjustmentScale(const QString &prst, int index);
// The preset's adjustments for a frame of `size`: `given` by name over the defaults, each of those a handle moves held to the handle's range; in the preset's order and units.
QVector<double> presetAdjustmentValues(const QString &prst, const QHash<QString, double> &given, const QSizeF &size);

} // namespace jp
