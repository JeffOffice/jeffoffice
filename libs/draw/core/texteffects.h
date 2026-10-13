#pragma once
// What a:effectLst says of a run of text: shadows, a glow, a reflection, soft
// edges. A run holds them as a string (tp::TextFx), so they travel with its
// format; the text engine draws them under the letters.

#include <QColor>
#include <QImage>
#include <QString>

namespace jp {

struct TextShadow {
    bool on = false;
    QColor color = Qt::black;   // with its alpha
    double blur = 0;            // points (blurRad)
    double distance = 0;        // points
    double angle = 0;           // degrees clockwise from the right: 45 is down and right
    double sx = 1, sy = 1;      // scale, 1 is 100%
    double kx = 0, ky = 0;      // skew, degrees
    QString align = QStringLiteral("b");   // the point the scale and skew hold still
};

struct TextGlow {
    bool on = false;
    QColor color = Qt::black;
    double radius = 0;          // points
};

struct TextReflection {
    bool on = false;
    double blur = 0;            // points
    double startAlpha = 1, endAlpha = 0;   // opacity where the fade starts and ends (1 is opaque)
    double startPos = 0, endPos = 1;       // where, as a share of the reflection's height
    double distance = 0;        // points
    double angle = 90;          // degrees
    double fadeAngle = 90;      // degrees: the way the fade runs
    double sx = 1, sy = 1;      // 1 is 100%; a reflection that is turned over says -1
    double kx = 0, ky = 0;
    QString align = QStringLiteral("b");
};

struct TextEffects {
    TextShadow outer, inner;
    TextGlow glow;
    double softEdge = 0;        // points
    TextReflection reflection;
    // The list as the file had it, to be written back as it was ("" when
    // the effects were made here).
    QString xml;

    bool any() const { return outer.on || inner.on || glow.on || softEdge > 0 || reflection.on; }
    QString toJson() const;
    static TextEffects fromJson(const QString &json);
    // Parsed, and kept by its string for the next time.
    static const TextEffects &cached(const QString &json);
};

// The text effects every shape and Text Art made here has until it says otherwise: a soft gray shadow.
TextEffects defaultTextShadow();

// How far a shadow's blur spreads, as the standard deviation of a Gaussian: the presentation program's blur radius is the width of a
// box that blurs the edge, which a Gaussian of this spread matches.
inline double blurSigma(double blurRadius) { return blurRadius * 0.2887; }   // 1 / sqrt(12)

// The program's own pictures put a shadow on words a little nearer to them and a little softer than the file's distance and radius say:
// measured on the stems of its letters against the same stems drawn here, 0.88 of the distance and 1.08 of the spread; these are the
// rounded numbers.
inline constexpr double kShadowNear = 0.9, kShadowSoft = 1.1;

// Blurs the alpha channel of an Alpha8 image with a Gaussian of the given spread (pixels), three passes of boxes.
void blurAlpha(QImage &alpha8, double sigma);

} // namespace jp
