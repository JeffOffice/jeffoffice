#pragma once
// The outline of a DrawingML shape (ECMA-376 20.1.9): guides, which are named
// values that formulas make from the shape's size and its adjustments, and
// the paths drawn with them. A custom shape (a:custGeom) carries its own
// definition; a preset shape (a:prstGeom) is one the standard names, and
// drawingml_geometry.cpp holds definitions in the same form for those the
// shape library (render/shapes) cannot draw as the standard does.

#include "core/guideevaluator.h"

#include <QByteArray>
#include <QDomElement>
#include <QHash>
#include <QPair>
#include <QPainterPath>
#include <QRectF>
#include <QSet>
#include <QSizeF>
#include <QString>
#include <QVector>

namespace jp::office {

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
// The counterpart of libraryAdjustments, for writing: the standard's
// adjustments ("adj", "adj1", ... with their values in 100,000ths) that a
// shape of the library with `adj` is. All of the preset's, or, when every
// one is the standard's own default, none: the presentation program refuses
// a file whose shape gives some and not the others. Empty for a preset the
// library has none for.
QVector<QPair<QString, double>> presetAdjustments(const QString &prst, const QVector<double> &adj, const QSizeF &size);

// An a:custGeom for an outline (points in the frame of `size`, in points): one path in EMU, a subpath closed where it comes back to its start, the text rectangle (`textRect`, the frame's whole
// when null), and, when `sites` has four places (up, left, down, right), the connection sites. Written so that readCustomGeometry reads the same outline.
QByteArray customGeometryXml(const QPainterPath &outline, const QSizeF &size, const QRectF &textRect, const QVector<QPointF> &sites);

} // namespace jp::office
