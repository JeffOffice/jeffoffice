#include "core/texteffects.h"

#include <QCache>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>
#include <cmath>
#include <vector>

namespace jp {

namespace {

QJsonObject shadowJson(const TextShadow &s)
{
    QJsonObject o{{"c", s.color.name(QColor::HexArgb)}, {"b", s.blur}, {"d", s.distance}, {"a", s.angle}};
    if (s.sx != 1) o["sx"] = s.sx;
    if (s.sy != 1) o["sy"] = s.sy;
    if (s.kx != 0) o["kx"] = s.kx;
    if (s.ky != 0) o["ky"] = s.ky;
    if (s.align != QLatin1String("b")) o["al"] = s.align;
    return o;
}

TextShadow shadowFrom(const QJsonObject &o)
{
    TextShadow s;
    s.on = true;
    s.color = QColor(o["c"].toString());
    if (!s.color.isValid()) s.color = Qt::black;
    s.blur = std::clamp(o["b"].toDouble(), 0.0, 1584.0);
    s.distance = std::clamp(o["d"].toDouble(), 0.0, 1584.0);
    s.angle = o["a"].toDouble();
    s.sx = std::clamp(o["sx"].toDouble(1), -20.0, 20.0);
    s.sy = std::clamp(o["sy"].toDouble(1), -20.0, 20.0);
    s.kx = std::clamp(o["kx"].toDouble(), -90.0, 90.0);
    s.ky = std::clamp(o["ky"].toDouble(), -90.0, 90.0);
    s.align = o["al"].toString(QStringLiteral("b"));
    return s;
}

} // namespace

QString TextEffects::toJson() const
{
    QJsonObject o;
    if (outer.on) o["o"] = shadowJson(outer);
    if (inner.on) o["i"] = shadowJson(inner);
    if (glow.on) o["g"] = QJsonObject{{"c", glow.color.name(QColor::HexArgb)}, {"r", glow.radius}};
    if (softEdge > 0) o["s"] = softEdge;
    if (reflection.on) {
        const TextReflection &r = reflection;
        QJsonObject ro{{"b", r.blur}, {"sa", r.startAlpha}, {"ea", r.endAlpha}, {"sp", r.startPos}, {"ep", r.endPos},
                       {"d", r.distance}, {"a", r.angle}, {"fa", r.fadeAngle}, {"sx", r.sx}, {"sy", r.sy}};
        if (r.kx != 0) ro["kx"] = r.kx;
        if (r.ky != 0) ro["ky"] = r.ky;
        if (r.align != QLatin1String("b")) ro["al"] = r.align;
        o["r"] = ro;
    }
    if (!xml.isEmpty()) o["x"] = xml;
    return QString::fromUtf8(QJsonDocument(o).toJson(QJsonDocument::Compact));
}

TextEffects TextEffects::fromJson(const QString &json)
{
    TextEffects e;
    const QJsonObject o = QJsonDocument::fromJson(json.toUtf8()).object();
    if (o.contains("o")) e.outer = shadowFrom(o["o"].toObject());
    if (o.contains("i")) e.inner = shadowFrom(o["i"].toObject());
    if (o.contains("g")) {
        const QJsonObject g = o["g"].toObject();
        e.glow.on = true;
        e.glow.color = QColor(g["c"].toString());
        if (!e.glow.color.isValid()) e.glow.color = Qt::black;
        e.glow.radius = std::clamp(g["r"].toDouble(), 0.0, 1584.0);
    }
    e.softEdge = std::clamp(o["s"].toDouble(), 0.0, 1584.0);
    if (o.contains("r")) {
        const QJsonObject r = o["r"].toObject();
        TextReflection &t = e.reflection;
        t.on = true;
        t.blur = std::clamp(r["b"].toDouble(), 0.0, 1584.0);
        t.startAlpha = std::clamp(r["sa"].toDouble(1), 0.0, 1.0);
        t.endAlpha = std::clamp(r["ea"].toDouble(0), 0.0, 1.0);
        t.startPos = std::clamp(r["sp"].toDouble(0), 0.0, 1.0);
        t.endPos = std::clamp(r["ep"].toDouble(1), 0.0, 1.0);
        t.distance = std::clamp(r["d"].toDouble(), 0.0, 1584.0);
        t.angle = r["a"].toDouble(90);
        t.fadeAngle = r["fa"].toDouble(90);
        t.sx = std::clamp(r["sx"].toDouble(1), -20.0, 20.0);
        t.sy = std::clamp(r["sy"].toDouble(1), -20.0, 20.0);
        t.kx = std::clamp(r["kx"].toDouble(), -90.0, 90.0);
        t.ky = std::clamp(r["ky"].toDouble(), -90.0, 90.0);
        t.align = r["al"].toString(QStringLiteral("b"));
    }
    e.xml = o["x"].toString();
    return e;
}

const TextEffects &TextEffects::cached(const QString &json)
{
    // A run's effects are looked at on every paint; the same few strings come back.
    static thread_local QCache<QString, TextEffects> known(256);
    if (const TextEffects *e = known.object(json)) return *e;
    auto *parsed = new TextEffects(fromJson(json));
    known.insert(json, parsed);
    return *parsed;
}

TextEffects defaultTextShadow()
{
    TextEffects e;
    e.outer.on = true;
    e.outer.color = QColor(0, 0, 0, 110);
    e.outer.blur = 3;
    e.outer.distance = 3;
    e.outer.angle = 45;
    return e;
}

namespace {

// One box blur of an Alpha8 image along its rows or its columns, the edges
// counting as clear. The sum slides along: a pixel in, a pixel out.
void boxPass(QImage &img, int radius, bool horizontal)
{
    if (radius < 1) return;
    const int w = img.width(), h = img.height();
    const int n = horizontal ? w : h, lines = horizontal ? h : w;
    const int step = horizontal ? 1 : int(img.bytesPerLine());
    std::vector<uchar> in(static_cast<size_t>(n));
    const int window = radius * 2 + 1;
    for (int l = 0; l < lines; ++l) {
        uchar *base = horizontal ? img.scanLine(l) : img.scanLine(0) + l;
        bool any = false;
        for (int i = 0; i < n; ++i) {
            in[size_t(i)] = base[i * step];
            any |= in[size_t(i)] != 0;
        }
        if (!any) continue;
        int sum = 0;
        for (int i = 0; i < std::min(radius, n); ++i) sum += in[size_t(i)];
        for (int i = 0; i < n; ++i) {
            if (i + radius < n) sum += in[size_t(i + radius)];
            if (i - radius - 1 >= 0) sum -= in[size_t(i - radius - 1)];
            base[i * step] = uchar((sum + window / 2) / window);
        }
    }
}

} // namespace

void blurAlpha(QImage &alpha8, double sigma)
{
    if (alpha8.isNull() || alpha8.format() != QImage::Format_Alpha8 || !(sigma >= 0.3)) return;
    // Three boxes make a Gaussian (the widths that give the spread: Kovesi's).
    const int passes = 3;
    const double ideal = std::sqrt(12 * sigma * sigma / passes + 1);
    int lower = int(std::floor(ideal));
    if (lower % 2 == 0) --lower;
    const int upper = lower + 2;
    const double mid = (12 * sigma * sigma - passes * lower * lower - 4 * passes * lower - 3 * passes) / (-4.0 * lower - 4);
    const int smaller = int(std::lround(mid));
    for (int k = 0; k < passes; ++k) {
        const int size = k < smaller ? lower : upper;
        boxPass(alpha8, (size - 1) / 2, true);
        boxPass(alpha8, (size - 1) / 2, false);
    }
}

} // namespace jp
