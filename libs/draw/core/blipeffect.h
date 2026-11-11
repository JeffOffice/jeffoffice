#pragma once
// One color effect of a picture (see core/blipeffects.h, which has what is done with them), kept apart so the styles that hold a list of them need not include the rest.

#include <QColor>
#include <QJsonObject>
#include <QString>

namespace jp {

struct BlipEffect {
    enum Kind { Luminance, Grayscale, BiLevel, ColorChange, ColorReplace, Duotone, Hsl, Tint };
    Kind kind = Luminance;
    // Luminance: the brightness and contrast, each from -1 to 1 (a:lum's bright and contrast).
    double brightness = 0, contrast = 0;
    // BiLevel: the share of the range from which a pixel is white (a:biLevel's thresh).
    double threshold = 0.5;
    // Hsl: degrees the hue turns, and how much the saturation and the lightness move, from -1 to 1. Tint: the hue it moves toward (degrees) and
    // how much, from -1 to 1 (a:tint's hue and amt).
    double hue = 0, saturation = 0, lightness = 0, amount = 0;
    // ColorChange: the color that changes (color) and what it becomes (color2, whose alpha 0 makes it clear), and whether the pixels' own clearness
    // counts (a:clrChange's useA, kept as written, though a color is matched by its parts alone). ColorReplace: the color every pixel becomes
    // (color). Duotone: the colors black and white become (color, color2).
    QColor color, color2;
    bool useAlpha = true;

    bool operator==(const BlipEffect &o) const;
    QJsonObject toJson() const;
    static BlipEffect fromJson(const QJsonObject &o);
};

} // namespace jp
