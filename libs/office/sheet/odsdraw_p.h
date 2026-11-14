#pragma once
// What the two halves of the drawing conversion (odsdraw_read.cpp, odsdraw_write.cpp) share.

#include <QByteArray>
#include <QSizeF>
#include <QString>

namespace jp::office::sheet::odsdraw {

// The pair of names a preset shape and a fontwork shape have in the standard.
struct NamePair {
    const char *drawingml;
    const char *odf;
};
// The shapes the standard has a name for (a preset it has none for is "ooxml-" and its name), and the fontwork shapes the text warps stand for.
QString odfTypeOfPreset(const QString &prst);
QString presetOfOdfType(const QString &type);
QString fontworkOfWarp(const QString &warp);   // empty for a warp the standard has no fontwork shape for
QString warpOfFontwork(const QString &type);   // "textPlain" for one that is not known

// The format of picture bytes by their first bytes ("png", "jpeg", "gif", "bmp", "tiff", "svg", "emf", "wmf"; the ODT code's own test); empty if they are none of these.
QString imageFormatOf(const QByteArray &bytes);
// The size a picture has of itself, in points (its pixels at its own resolution, or at 96 to the inch if it says none); empty if it can't be told.
QSizeF naturalSizePoints(const QByteArray &bytes);
// Text or an attribute value written into XML (the ODT writer's own escaping: what XML has no place for is left out).
QString xmlText(const QString &text);

} // namespace jp::office::sheet::odsdraw
