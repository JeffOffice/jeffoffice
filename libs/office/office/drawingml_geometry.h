#pragma once
// The outline of a DrawingML shape (ECMA-376 20.1.9): guides, which are named
// values that formulas make from the shape's size and its adjustments, and
// the paths drawn with them. A custom shape (a:custGeom) carries its own
// definition; a preset shape (a:prstGeom) is one the standard names, and
// drawingml_geometry.cpp holds definitions in the same form for those the
// shape library (render/shapes) cannot draw as the standard does.

#include <QDomElement>
#include <QHash>
#include <QPainterPath>
#include <QRectF>
#include <QSet>
#include <QSizeF>
#include <QString>
#include <QVector>

namespace jp::office {

// Evaluates guide formulas, such as "*/ w adj1 100000" (x * y / z). The
// operators are */ +- +/ ?: abs at2 cat2 cos max min mod pin sat2 sin sqrt
// tan and val; angles are in 60,000ths of a degree. The names a formula can
// use are those defined, the shape's w and h (in EMU), and the built-in
// ones that follow from them (l t r b hc vc wd2 hd2 ss ls cd4 3cd4 ...).
// Every result is finite: a division by zero, a name that was never
// defined, a guide that depends on itself, and a chain of guides nested more
// deeply than a drawing needs each give 0.
class GuideEvaluator {
public:
    GuideEvaluator(double width, double height);
    // Defines a guide by its formula; a name defined already keeps its value.
    void define(const QString &name, const QString &formula);
    // Defines a name as a number.
    void set(const QString &name, double value);
    // The value of a name or of a number written out.
    double value(const QString &token);

private:
    double evaluate(const QString &formula);

    QHash<QString, double> m_values;
    QHash<QString, QString> m_formulas;
    QSet<QString> m_active;
    int m_depth = 0;
};

// One a:path, in points relative to the shape's frame.
struct GeometryPath {
    // How the path is filled: not at all, or as the shape's fill, or with that
    // fill lightened or darkened (the shaded faces of a cube).
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

// The adjustments a:avLst gives, by name, as numbers ("val 25000" is 25000).
QHash<QString, double> readAdjustments(const QDomElement &avLst);

// The geometry that an a:custGeom defines, for a frame of `size` points.
Geometry readCustomGeometry(const QDomElement &custGeom, const QSizeF &size);

// A preset's own definition, with `adjust` replacing its default adjustments;
// empty for a preset that has none here.
Geometry presetGeometry(const QString &prst, const QHash<QString, double> &adjust, const QSizeF &size);
bool hasPresetGeometry(const QString &prst);
// Whether the definition here is to be used even though the shape library
// has the preset too: where the library's shape differs from the standard's
// in a way that shows (a callout's body, an arc's direction).
bool prefersPresetGeometry(const QString &prst);

// The shape library's adjustments for a preset, from the standard's values:
// `given` by name ("adj", "adj1", ...), the standard's defaults where it has
// none, converted to the library's units (fractions of the frame's width,
// height, or shorter side). Empty for a preset the library has no
// adjustments for.
QVector<double> libraryAdjustments(const QString &prst, const QHash<QString, double> &given, const QSizeF &size);

} // namespace jp::office
