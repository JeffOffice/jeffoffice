#include "core/blipeffects.h"

#include <QCache>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMutex>
#include <QMutexLocker>
#include <algorithm>
#include <atomic>
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

uchar byteOf(float v) { return uchar(std::clamp(int(v * 255 + 0.5f), 0, 255)); }

// A color as hue (in sixths of a turn, 0 up to 6), saturation, and lightness, and back; floats, and without a branch that depends on the
// color, as a picture has millions of them and the colors of a photograph go every way.
void toHsl(float r, float g, float b, float *h, float *s, float *l)
{
    const float hi = std::max({r, g, b}), lo = std::min({r, g, b}), d = hi - lo;
    *l = (hi + lo) * 0.5f;
    if (d <= 0) {
        *h = *s = 0;
        return;
    }
    *s = std::min(1.0f, d / (1 - std::fabs(2 * *l - 1)));
    const float inverse = 1 / d, red = (g - b) * inverse, green = (b - r) * inverse + 2, blue = (r - g) * inverse + 4;
    const float k = hi == r ? red : hi == g ? green : blue;
    *h = k < 0 ? k + 6 : k;
}

// `h` in sixths of a turn, from -6 up to 12.
void fromHsl(float h, float s, float l, float *r, float *g, float *b)
{
    h = h < 0 ? h + 6 : h >= 6 ? h - 6 : h;
    const float c = (1 - std::fabs(2 * l - 1)) * s, m = l - c * 0.5f;
    *r = m + c * std::clamp(std::fabs(h - 3) - 1, 0.0f, 1.0f);
    *g = m + c * std::clamp(2 - std::fabs(h - 2), 0.0f, 1.0f);
    *b = m + c * std::clamp(2 - std::fabs(h - 4), 0.0f, 1.0f);
}

// Each pixel of `img` made what `change` gives for it.
template <class F>
void eachPixel(QImage &img, F change)
{
    for (int y = 0; y < img.height(); ++y) {
        auto *row = reinterpret_cast<QRgb *>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x) row[x] = change(row[x]);
    }
}

// An effect that leaves every color as it is (a:lum with nothing in it, as the reference presentations' masters have, or a tint of none).
bool changesNothing(const BlipEffect &e)
{
    switch (e.kind) {
    case BlipEffect::Luminance: return e.brightness == 0 && e.contrast == 0;
    case BlipEffect::Hsl: return std::fmod(e.hue, 360.0) == 0 && e.saturation == 0 && e.lightness == 0;
    case BlipEffect::Tint: return e.amount == 0;
    default: return false;
    }
}

QVector<BlipEffect> changing(const QVector<BlipEffect> &effects)
{
    QVector<BlipEffect> out;
    for (const BlipEffect &e : effects)
        if (!changesNothing(e)) out.append(e);
    return out;
}

// Moves `v` (0 to 1) toward 1 by the share `by` when it is above 0, and toward 0 when below.
float moved(float v, float by) { return by >= 0 ? v + (1 - v) * by : v * (1 + by); }

std::atomic<int> made{0};

} // namespace

bool BlipEffect::operator==(const BlipEffect &o) const
{
    return kind == o.kind && brightness == o.brightness && contrast == o.contrast && threshold == o.threshold && hue == o.hue && saturation == o.saturation &&
           lightness == o.lightness && amount == o.amount && color == o.color && color2 == o.color2 && useAlpha == o.useAlpha;
}

QJsonObject BlipEffect::toJson() const
{
    QJsonObject o{{"k", QString::fromLatin1(kindNames[kind])}};
    switch (kind) {
    case Luminance: o["bright"] = brightness; o["contrast"] = contrast; break;
    case Grayscale: break;
    case BiLevel: o["thresh"] = threshold; break;
    case ColorChange:
        o["from"] = colorJson(color);
        o["to"] = colorJson(color2);
        if (!useAlpha) o["useA"] = false;
        break;
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
    case ColorChange:
        e.color = colorFrom(o["from"].toObject());
        e.color2 = colorFrom(o["to"].toObject());
        e.useAlpha = o["useA"].toBool(true);
        break;
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

bool blipEffectsChange(const QVector<BlipEffect> &effects) { return !changing(effects).isEmpty(); }

qint64 blipEffectsWork(qint64 pixels, const QVector<BlipEffect> &effects)
{
    qint64 each = 0;
    for (const BlipEffect &e : effects) {
        if (changesNothing(e)) continue;
        each += e.kind == BlipEffect::Hsl || e.kind == BlipEffect::Tint ? kBlipHslCost : kBlipPixelCost;
    }
    return pixels * each;
}

int blipAdjustedMade() { return made; }

double contrastFactor(double contrast)
{
    const double c = std::clamp(contrast, -1.0, 1.0);
    // At the most, the colors are cut at the middle: a factor no one sees past.
    return c >= 0 ? 1 / std::max(1 - c, 1.0 / 1024) : 1 + c;
}

double contrastOfFactor(double factor)
{
    return factor > 1 ? 1 - 1 / factor : factor - 1;
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
    if (image.isNull() || all.isEmpty()) return image;
    const qint64 pixels = qint64(image.width()) * image.height();
    const QVector<BlipEffect> effects = changing(all);
    if (effects.isEmpty() || pixels > kMaxBlipPixels || blipEffectsWork(pixels, effects) > kMaxBlipWork) return image;
    QImage img = image.convertToFormat(QImage::Format_ARGB32);
    for (const BlipEffect &e : effects) {
        const QColor from = e.color, to = e.color2;
        switch (e.kind) {
        case BlipEffect::Luminance: {
            const QVector<uchar> table = luminanceTable(std::clamp(e.brightness, -1.0, 1.0), contrastFactor(e.contrast));
            const uchar *level = table.constData();
            eachPixel(img, [level](QRgb c) { return qRgba(level[qRed(c)], level[qGreen(c)], level[qBlue(c)], qAlpha(c)); });
            break;
        }
        case BlipEffect::Grayscale:
            eachPixel(img, [](QRgb c) {
                const int gray = int(0.299 * qRed(c) + 0.587 * qGreen(c) + 0.114 * qBlue(c) + 0.5);
                return qRgba(gray, gray, gray, qAlpha(c));
            });
            break;
        case BlipEffect::BiLevel: {
            const double threshold = std::clamp(e.threshold, 0.0, 1.0) * 256;
            // (The sum of the weights is a hair under 1 in a double: a gray of 128 is not 127.99999999999999.)
            eachPixel(img, [threshold](QRgb c) {
                const int level = 0.299 * qRed(c) + 0.587 * qGreen(c) + 0.114 * qBlue(c) + 1e-9 >= threshold ? 255 : 0;
                return qRgba(level, level, level, qAlpha(c));
            });
            break;
        }
        case BlipEffect::ColorChange: {
            // A color is the one when each of its parts is within nine of it, as a gif's or a jpeg's edges are not.
            const int fr = from.red(), fg = from.green(), fb = from.blue(), tr = to.red(), tg = to.green(), tb = to.blue();
            const double clearness = to.alphaF();
            eachPixel(img, [=](QRgb c) {
                if (std::abs(qRed(c) - fr) < 10 && std::abs(qGreen(c) - fg) < 10 && std::abs(qBlue(c) - fb) < 10) return qRgba(tr, tg, tb, int(qAlpha(c) * clearness + 0.5));
                return c;
            });
            break;
        }
        case BlipEffect::ColorReplace: {
            const int cr = from.red(), cg = from.green(), cb = from.blue();
            const double clearness = from.alphaF();
            eachPixel(img, [=](QRgb c) { return qRgba(cr, cg, cb, int(qAlpha(c) * clearness + 0.5)); });
            break;
        }
        case BlipEffect::Duotone: {
            // What each of the 256 grays comes to, in each of the colors.
            QRgb ramp[256];
            for (int v = 0; v < 256; ++v) {
                const double t = v / 255.0;
                ramp[v] = qRgb(int(from.red() + (to.red() - from.red()) * t + 0.5), int(from.green() + (to.green() - from.green()) * t + 0.5),
                               int(from.blue() + (to.blue() - from.blue()) * t + 0.5));
            }
            eachPixel(img, [&ramp](QRgb c) { return (ramp[qGray(c)] & 0x00FFFFFF) | (c & 0xFF000000); });
            break;
        }
        case BlipEffect::Hsl: {
            // The hue turn in sixths of a turn, and how far the saturation and the lightness move.
            const float turn = float(std::fmod(e.hue, 360.0) / 60), satBy = float(std::clamp(e.saturation, -1.0, 1.0)), lumBy = float(std::clamp(e.lightness, -1.0, 1.0));
            eachPixel(img, [=](QRgb c) {
                float h, s, l, r, g, b;
                toHsl(qRed(c) * (1.0f / 255), qGreen(c) * (1.0f / 255), qBlue(c) * (1.0f / 255), &h, &s, &l);
                fromHsl(h + turn, moved(s, satBy), moved(l, lumBy), &r, &g, &b);
                return qRgba(byteOf(r), byteOf(g), byteOf(b), qAlpha(c));
            });
            break;
        }
        case BlipEffect::Tint: {
            // Each color moves toward the hue by the amount (a gray one has none and takes it), and is made more or less of a color by it;
            // an amount of none leaves a color as it was.
            const float turn = float(std::fmod(e.hue, 360.0) / 60), by = float(std::clamp(e.amount, -1.0, 1.0)), hue = turn < 0 ? turn + 6 : turn;
            eachPixel(img, [=](QRgb c) {
                float h, s, l, r, g, b;
                toHsl(qRed(c) * (1.0f / 255), qGreen(c) * (1.0f / 255), qBlue(c) * (1.0f / 255), &h, &s, &l);
                if (s <= 0) {
                    h = hue;
                } else {
                    float d = hue - h;
                    if (d > 3) d -= 6;
                    else if (d < -3) d += 6;
                    h += d * std::max(by, 0.0f);
                }
                fromHsl(h, std::clamp(s + by, 0.0f, 1.0f), l, &r, &g, &b);
                return qRgba(byteOf(r), byteOf(g), byteOf(b), qAlpha(c));
            });
            break;
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
    const QImage result = applyBlipEffects(image, effects);
    // A picture left as it is (too big, or too much to do) has nothing to keep.
    if (result.cacheKey() == image.cacheKey()) return result;
    ++made;
    QMutexLocker locked(&lock);
    cache.insert(key, new QImage(result), std::max<qsizetype>(1, result.sizeInBytes() / 1024));
    return result;
}

} // namespace jp
