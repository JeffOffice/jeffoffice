#pragma once
// The fill of a frame: none, a color, a gradient, a picture, a texture, or a pattern (the rest of a frame's look is in core/style.h).

#include "core/blipeffect.h"
#include "core/color.h"

#include <QBrush>
#include <QJsonObject>
#include <QRectF>
#include <QVector>
#include <functional>

class QImage;

namespace jp {

// The most stops a gradient is given: real ones have a handful, so a file that lists more has the rest left out.
constexpr int kMaxGradientStops = 256;

struct GradientStop {
    double pos = 0;
    ColorRef color;
    double transparency = 0;
};

// Looks up a picture by id (used by picture/texture fills).
using ImageLookup = std::function<QImage(const QString &)>;

struct Fill {
    enum Type { NoFill, Solid, Gradient, Picture, Texture, Pattern };
    enum GradType { Linear, Radial, Rectangular, PathGrad };

    Type type = NoFill;
    ColorRef color;
    double transparency = 0;       // 0 opaque .. 1 clear

    GradType gradType = Linear;
    double angle = 90;             // degrees, 90 = top to bottom
    QVector<GradientStop> stops;   // when empty, color -> color2

    ColorRef color2;               // gradient end, pattern background
    QString imageId;               // picture and texture fills
    // What a picture fill's colors go through as it is drawn (see core/blipeffects.h): the picture's bytes stay as they are.
    QVector<BlipEffect> blipEffects;
    bool tile = false;
    double tileScale = 1.0;
    int pattern = 0;               // index into patternBrushes()

    static Fill none() { return Fill(); }
    static Fill solid(const ColorRef &c, double t = 0) { Fill f; f.type = Solid; f.color = c; f.transparency = t; return f; }
    static Fill gradient(const ColorRef &a, const ColorRef &b, double angle = 90) {
        Fill f; f.type = Gradient; f.color = a; f.color2 = b; f.angle = angle; return f;
    }
    bool isNone() const { return type == NoFill || (type == Solid && color.isNone()); }

    QBrush brush(const QRectF &r, const ColorScheme &s, const ImageLookup &img = {}) const;
    QImage patternTile(const ColorScheme &s) const;   // 8 x 8, the background clear when it has none
    QJsonObject toJson() const;
    static Fill fromJson(const QJsonObject &o);
    bool operator==(const Fill &o) const { return toJson() == o.toJson(); }
};

} // namespace jp
