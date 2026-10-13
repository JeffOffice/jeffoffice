#include "core/blipeffects.h"

#include <QCache>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMutex>
#include <QMutexLocker>
#include <algorithm>
#include <cmath>

namespace jp {

namespace {

const char *kindNames[] = {"lum", "gray", "bilevel", "clrchange", "clrrepl", "duotone", "hsl", "tint"};

QJsonObject colorJson(const QColor &c) { return QJsonObject{{"rgb", c.name()}, {"a", c.alphaF()}}; }

QColor colorFrom(const QJsonObject &o)
{
    QColor c(o["rgb"].toString());
    if (!c.isValid()) c = QColor(Qt::black);
    c.setAlphaF(std::clamp(o["a"].toDouble(1.0), 0.0, 1.0));
    return c;
}

uchar byteOf(double v) { return uchar(std::clamp(int(v * 255 + 0.5), 0, 255)); }

// A color as hue (degrees), saturation, and lightness, and back.
void toHsl(double r, double g, double b, double *h, double *s, double *l)
{
    const double hi = std::max({r, g, b}), lo = std::min({r, g, b}), d = hi - lo;
    *l = (hi + lo) / 2;
    *s = d <= 0 ? 0 : d / (1 - std::abs(2 * *l - 1));
    if (d <= 0) *h = 0;
    else if (hi == r) *h = 60 * std::fmod((g - b) / d + 6, 6.0);
    else if (hi == g) *h = 60 * ((b - r) / d + 2);
    else *h = 60 * ((r - g) / d + 4);
}

void fromHsl(double h, double s, double l, double *r, double *g, double *b)
{
    const double c = (1 - std::abs(2 * l - 1)) * s, hp = std::fmod(std::fmod(h, 360.0) + 360, 360.0) / 60;
    const double x = c * (1 - std::abs(std::fmod(hp, 2.0) - 1)), m = l - c / 2;
    double rr = 0, gg = 0, bb = 0;
    if (hp < 1) { rr = c; gg = x; }
    else if (hp < 2) { rr = x; gg = c; }
    else if (hp < 3) { gg = c; bb = x; }
    else if (hp < 4) { gg = x; bb = c; }
    else if (hp < 5) { rr = x; bb = c; }
    else { rr = c; bb = x; }
    *r = rr + m;
    *g = gg + m;
    *b = bb + m;
}

// An effect that leaves every color as it is (a:lum with nothing in it, as the reference presentations' masters have).
bool changesNothing(const BlipEffect &e) { return e.kind == BlipEffect::Luminance && e.brightness == 0 && e.contrast == 0; }

QVector<BlipEffect> changing(const QVector<BlipEffect> &effects)
{
    QVector<BlipEffect> out;
    for (const BlipEffect &e : effects)
        if (!changesNothing(e)) out.append(e);
    return out;
}

// Moves `v` (0 to 1) toward 1 by the share `by` when it is above 0, and toward 0 when below.
double moved(double v, double by) { return by >= 0 ? v + (1 - v) * by : v * (1 + by); }

} // namespace

QJsonObject BlipEffect::toJson() const
{
    QJsonObject o{{"k", QString::fromLatin1(kindNames[kind])}};
    switch (kind) {
    case Luminance: o["bright"] = brightness; o["contrast"] = contrast; break;
    case Grayscale: break;
    case BiLevel: o["thresh"] = threshold; break;
    case ColorChange: o["from"] = colorJson(color); o["to"] = colorJson(color2); break;
    case ColorReplace: o["color"] = colorJson(color); break;
    case Duotone: o["dark"] = colorJson(color); o["light"] = colorJson(color2); break;
    case Hsl: o["hue"] = hue; o["sat"] = saturation; o["lum"] = lightness; break;
    case Tint: o["hue"] = hue; o["amt"] = amount; break;
    }
    return o;
}

BlipEffect BlipEffect::fromJson(const QJsonObject &o)
{
    BlipEffect e;
    const QString name = o["k"].toString();
    for (int i = 0; i <= Tint; ++i)
        if (name == QLatin1String(kindNames[i])) e.kind = Kind(i);
    switch (e.kind) {
    case Luminance: e.brightness = o["bright"].toDouble(); e.contrast = o["contrast"].toDouble(); break;
    case Grayscale: break;
    case BiLevel: e.threshold = o["thresh"].toDouble(0.5); break;
    case ColorChange: e.color = colorFrom(o["from"].toObject()); e.color2 = colorFrom(o["to"].toObject()); break;
    case ColorReplace: e.color = colorFrom(o["color"].toObject()); break;
    case Duotone: e.color = colorFrom(o["dark"].toObject()); e.color2 = colorFrom(o["light"].toObject()); break;
    case Hsl: e.hue = o["hue"].toDouble(); e.saturation = o["sat"].toDouble(); e.lightness = o["lum"].toDouble(); break;
    case Tint: e.hue = o["hue"].toDouble(); e.amount = o["amt"].toDouble(); break;
    }
    return e;
}

QJsonArray blipEffectsToJson(const QVector<BlipEffect> &effects)
{
    QJsonArray a;
    for (const BlipEffect &e : effects) a.append(e.toJson());
    return a;
}

QVector<BlipEffect> blipEffectsFromJson(const QJsonArray &a)
{
    QVector<BlipEffect> out;
    for (const QJsonValue &v : a) {
        if (out.size() >= kMaxBlipEffects) break;
        out.append(BlipEffect::fromJson(v.toObject()));
    }
    return out;
}

QString blipEffectsKey(const QVector<BlipEffect> &effects)
{
    return QString::fromUtf8(QJsonDocument(blipEffectsToJson(effects)).toJson(QJsonDocument::Compact));
}

bool blipEffectsClear(const QVector<BlipEffect> &effects)
{
    for (const BlipEffect &e : effects)
        if ((e.kind == BlipEffect::ColorChange && e.color2.alpha() < 255) || (e.kind == BlipEffect::ColorReplace && e.color.alpha() < 255)) return true;
    return false;
}

double contrastFactor(double contrast)
{
    const double c = std::clamp(contrast, -1.0, 1.0);
    // At the most, the colors are cut at the middle: a factor no one sees past.
    return c >= 0 ? 1 / std::max(1 - c, 1.0 / 1024) : 1 + c;
}

QVector<uchar> luminanceTable(double brightness, double factor)
{
    QVector<uchar> table(256);
    for (int v = 0; v < 256; ++v)
        table[v] = uchar(std::clamp(int(std::lround(((v / 255.0 - 0.5) * factor + 0.5 + brightness * (1 + factor) / 2) * 255)), 0, 255));
    return table;
}

QImage applyBlipEffects(const QImage &image, const QVector<BlipEffect> &all)
{
    if (image.isNull() || all.isEmpty() || qint64(image.width()) * image.height() > kMaxBlipPixels) return image;
    const QVector<BlipEffect> effects = changing(all);
    if (effects.isEmpty() || qint64(image.width()) * image.height() * effects.size() > kMaxBlipWork) return image;
    QImage img = image.convertToFormat(QImage::Format_ARGB32);
    for (const BlipEffect &e : effects) {
        QVector<uchar> table;
        if (e.kind == BlipEffect::Luminance) table = luminanceTable(std::clamp(e.brightness, -1.0, 1.0), contrastFactor(e.contrast));
        const double threshold = std::clamp(e.threshold, 0.0, 1.0) * 256;
        const QColor from = e.color, to = e.color2;
        for (int y = 0; y < img.height(); ++y) {
            auto *row = reinterpret_cast<QRgb *>(img.scanLine(y));
            for (int x = 0; x < img.width(); ++x) {
                const QRgb c = row[x];
                const int r = qRed(c), g = qGreen(c), b = qBlue(c), a = qAlpha(c);
                switch (e.kind) {
                case BlipEffect::Luminance: row[x] = qRgba(table[r], table[g], table[b], a); break;
                case BlipEffect::Grayscale: {
                    const int gray = int(0.299 * r + 0.587 * g + 0.114 * b);
                    row[x] = qRgba(gray, gray, gray, a);
                    break;
                }
                case BlipEffect::BiLevel: {
                    const int level = 0.299 * r + 0.587 * g + 0.114 * b >= threshold ? 255 : 0;
                    row[x] = qRgba(level, level, level, a);
                    break;
                }
                case BlipEffect::ColorChange:
                    // A color is the one when each of its parts is within nine of it, as a gif's or a jpeg's edges are not.
                    if (std::abs(r - from.red()) < 10 && std::abs(g - from.green()) < 10 && std::abs(b - from.blue()) < 10)
                        row[x] = qRgba(to.red(), to.green(), to.blue(), int(a * to.alphaF() + 0.5));
                    break;
                case BlipEffect::ColorReplace: row[x] = qRgba(from.red(), from.green(), from.blue(), int(a * from.alphaF() + 0.5)); break;
                case BlipEffect::Duotone: {
                    const double t = qGray(c) / 255.0;
                    row[x] = qRgba(int(from.red() + (to.red() - from.red()) * t + 0.5), int(from.green() + (to.green() - from.green()) * t + 0.5),
                                   int(from.blue() + (to.blue() - from.blue()) * t + 0.5), a);
                    break;
                }
                case BlipEffect::Hsl:
                case BlipEffect::Tint: {
                    double h, s, l, rr, gg, bb;
                    toHsl(r / 255.0, g / 255.0, b / 255.0, &h, &s, &l);
                    if (e.kind == BlipEffect::Hsl) {
                        h += e.hue;
                        s = moved(s, std::clamp(e.saturation, -1.0, 1.0));
                        l = moved(l, std::clamp(e.lightness, -1.0, 1.0));
                    } else {
                        // Every color turned to the hue, and made more or less of it: a gray picture takes the hue's color.
                        h = e.hue;
                        s = std::clamp(s + e.amount, 0.0, 1.0);
                    }
                    fromHsl(h, s, l, &rr, &gg, &bb);
                    row[x] = qRgba(byteOf(rr), byteOf(gg), byteOf(bb), a);
                    break;
                }
                }
            }
        }
    }
    return img;
}

QImage blipAdjusted(const QImage &image, const QVector<BlipEffect> &effects)
{
    if (image.isNull() || changing(effects).isEmpty()) return image;
    static QMutex lock;
    static QCache<QString, QImage> cache(256 * 1024);   // KB
    const QString key = QString::number(image.cacheKey()) + QLatin1Char('|') + blipEffectsKey(effects);
    {
        QMutexLocker locked(&lock);
        if (const QImage *kept = cache.object(key)) return *kept;
    }
    const QImage made = applyBlipEffects(image, effects);
    QMutexLocker locked(&lock);
    cache.insert(key, new QImage(made), std::max<qsizetype>(1, made.sizeInBytes() / 1024));
    return made;
}

} // namespace jp
