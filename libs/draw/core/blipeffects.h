#pragma once
// The color effects of a picture that DrawingML gives (the children of a:blip: a:lum, a:grayscl, a:biLevel, a:clrChange, a:clrRepl, a:duotone,
// a:hsl, a:tint), and how they change its pixels, as the presentation program draws them. They are kept beside the picture, never in its
// bytes: the picture is changed only where it is drawn, and written back as it was.

#include <QColor>
#include <QImage>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QVector>

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
    // ColorChange: the color that changes (color) and what it becomes (color2, whose alpha 0 makes it clear). ColorReplace: the color every
    // pixel becomes (color). Duotone: the colors black and white become (color, color2).
    QColor color, color2;

    bool operator==(const BlipEffect &o) const { return toJson() == o.toJson(); }
    QJsonObject toJson() const;
    static BlipEffect fromJson(const QJsonObject &o);
};

// The most effects one picture is given: real ones have at most three (a recolor, a brightness and contrast, a clear color), so a file that
// lists more has the rest left out.
constexpr int kMaxBlipEffects = 32;
// The most pixels a picture is changed in (the reference presentations' largest has 9 million, so this is 11 times that), and the most
// changes of one pixel each that one picture is given, its pixels times its effects (those have at most 27 million, so this is 14 times):
// past either, the picture is drawn as it is.
constexpr qint64 kMaxBlipPixels = 100LL * 1000 * 1000;
constexpr qint64 kMaxBlipWork = 400LL * 1000 * 1000;

QJsonArray blipEffectsToJson(const QVector<BlipEffect> &effects);
QVector<BlipEffect> blipEffectsFromJson(const QJsonArray &a);
// What `effects` come to, as text: the same for the same effects (a key for a picture kept changed).
QString blipEffectsKey(const QVector<BlipEffect> &effects);

// The number a contrast (-1 to 1) multiplies each color's distance from the middle by: 1 for none, more than 1 from 1 / (1 - contrast)
// above 0, and less from 1 + contrast below it.
double contrastFactor(double contrast);
// What each of the 256 values of a color becomes with `brightness` (-1 to 1) and a contrast that multiplies by `factor`: its distance from the
// middle times the factor, and the brightness moved by the mean of one and the factor (fitted to the presentation program's own pictures: a
// brightness of 21% with a contrast of 52% makes a 0 come to -56 (so, 0), and one of 14% alone adds 36 to each color).
QVector<uchar> luminanceTable(double brightness, double factor);

// Whether any of the effects can make a color clear (so a page that has the picture needs transparency to be drawn).
bool blipEffectsClear(const QVector<BlipEffect> &effects);

// `image` with the effects applied in order, as an image of 32 bits (it as it is if none of them changes a color, or it is too big).
QImage applyBlipEffects(const QImage &image, const QVector<BlipEffect> &all);
// The same, kept: the image of the same pixels asked for again, with the same effects, is not made again (to a budget of its own, as the
// copies of the pictures drawn are).
QImage blipAdjusted(const QImage &image, const QVector<BlipEffect> &effects);

} // namespace jp
