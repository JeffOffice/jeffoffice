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
    // ColorChange: the color that changes (color) and what it becomes (color2, whose alpha 0 makes it clear), and whether the pixels' own clearness
    // counts (a:clrChange's useA, kept as written, though a color is matched by its parts alone). ColorReplace: the color every pixel becomes
    // (color). Duotone: the colors black and white become (color, color2).
    QColor color, color2;
    bool useAlpha = true;

    bool operator==(const BlipEffect &o) const;
    QJsonObject toJson() const;
    static BlipEffect fromJson(const QJsonObject &o);
};

// The most effects one picture is given: real ones have at most three (a recolor, a brightness and contrast, a clear color), so a file that
// lists more has the rest left out.
constexpr int kMaxBlipEffects = 32;
// The most pixels a picture is changed in (the reference presentations' largest has 9 million, so this is 11 times that).
constexpr qint64 kMaxBlipPixels = 100LL * 1000 * 1000;
// What one picture's effects may cost (blipEffectsWork: its pixels times what each effect takes of one pixel, in nanoseconds as measured on a
// busy machine, so a bound on the seconds): about half a second. The reference presentations' largest picture (9 million pixels) with three
// effects of the plain kind is 216 million, and a 12 million pixel photograph with a hue turned 540 million; a picture and effects that cost
// more than this are drawn as the picture is (32 hue turns on a 12 million pixel photograph would take 17 seconds).
constexpr qint64 kMaxBlipWork = 600LL * 1000 * 1000;
// What a pixel takes of one effect: a table, a sum, or a color swapped; and a turn through hue, saturation, and lightness and back.
constexpr int kBlipPixelCost = 8;
constexpr int kBlipHslCost = 45;

QJsonArray blipEffectsToJson(const QVector<BlipEffect> &effects);
QVector<BlipEffect> blipEffectsFromJson(const QJsonArray &a);
// What `effects` come to, as text: the same for the same effects (a key for a picture kept changed).
QString blipEffectsKey(const QVector<BlipEffect> &effects);

// Whether any of the effects changes a color (one of a:lum with nothing in it does not).
bool blipEffectsChange(const QVector<BlipEffect> &effects);
// What `effects` cost for a picture of `pixels` (those that change nothing are free).
qint64 blipEffectsWork(qint64 pixels, const QVector<BlipEffect> &effects);
// How many pictures blipAdjusted has had to make, not found kept (for a test to see that a picture is made once).
int blipAdjustedMade();

// The number a contrast (-1 to 1) multiplies each color's distance from the middle by: 1 for none, more than 1 from 1 / (1 - contrast)
// above 0, and less from 1 + contrast below it.
double contrastFactor(double contrast);
// The contrast (-1 to 1) that multiplies by `factor`: contrastFactor's other way, for a file that gives the factor.
double contrastOfFactor(double factor);
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
