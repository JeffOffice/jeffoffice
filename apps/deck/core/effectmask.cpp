#include "core/effectmask.h"

#include <QRectF>
#include <QMutex>
#include <QMutexLocker>
#include <QRegularExpression>
#include <list>
#include <algorithm>
#include <cmath>

namespace jp::deck {

namespace {

constexpr int kSlats = 6, kBars = 20, kSquares = 8, kDissolveCells = 20;

QPainterPath whole()
{
    QPainterPath all;
    all.addRect(QRectF(0, 0, 1, 1));
    return all;
}

// The same noise for the same cell, so a frame is a function of the progress alone.
quint32 noise(quint32 n, quint32 salt)
{
    quint32 h = n * 0x9E3779B1u + salt * 0x85EBCA6Bu;
    h ^= h >> 16;
    h *= 0x7FEB352Du;
    h ^= h >> 15;
    h *= 0x846CA68Bu;
    h ^= h >> 16;
    return h;
}

double noiseShare(quint32 n, quint32 salt) { return double(noise(n, salt) >> 8) / double(1 << 24); }

// Slats that open from one edge of each band.
QPainterPath blinds(bool horizontal, double p)
{
    QPainterPath out;
    for (int i = 0; i < kSlats; ++i) {
        const double a = double(i) / kSlats, size = 1.0 / kSlats;
        out.addRect(horizontal ? QRectF(0, a, 1, size * p) : QRectF(a, 0, size * p, 1));
    }
    return out;
}

// Squares that fill, one color of the board and then the other.
QPainterPath checkerboard(bool across, double p)
{
    QPainterPath out;
    const double side = 1.0 / kSquares;
    for (int j = 0; j < kSquares; ++j)
        for (int i = 0; i < kSquares; ++i) {
            const double share = std::clamp((i + j) % 2 == 0 ? p * 2 : p * 2 - 1, 0.0, 1.0);
            out.addRect(across ? QRectF(i * side, j * side, side * share, side) : QRectF(i * side, j * side, side, side * share));
        }
    return out;
}

// Thin bars that appear one by one in an order the noise gives.
QPainterPath randomBars(bool horizontal, double p)
{
    QPainterPath out;
    for (int i = 0; i < kBars; ++i) {
        if (p <= noiseShare(quint32(i), 7)) continue;
        const double a = double(i) / kBars, size = 1.0 / kBars;
        out.addRect(horizontal ? QRectF(0, a, 1, size) : QRectF(a, 0, size, 1));
    }
    return out;
}

QPainterPath dissolve(double p)
{
    QPainterPath out;
    const double side = 1.0 / kDissolveCells;
    for (int j = 0; j < kDissolveCells; ++j)
        for (int i = 0; i < kDissolveCells; ++i)
            if (noiseShare(quint32(j * kDissolveCells + i), 11) < p) out.addRect(QRectF(i * side, j * side, side, side));
    return out;
}

QPainterPath ellipseOf(double r)
{
    QPainterPath out;
    out.addEllipse(QPointF(0.5, 0.5), r, r);
    return out;
}

QPainterPath diamondOf(double q)
{
    QPainterPath out;
    out.addPolygon(QPolygonF({QPointF(0.5 - q, 0.5), QPointF(0.5, 0.5 - q), QPointF(0.5 + q, 0.5), QPointF(0.5, 0.5 + q)}));
    out.closeSubpath();
    return out;
}

QPainterPath plusOf(double q)
{
    const double a = 0.5 - q / 2, b = 0.5 + q / 2;
    QPainterPath out;
    out.addPolygon(QPolygonF({QPointF(a, 0), QPointF(b, 0), QPointF(b, a), QPointF(1, a), QPointF(1, b), QPointF(b, b), QPointF(b, 1), QPointF(a, 1), QPointF(a, b), QPointF(0, b), QPointF(0, a), QPointF(a, a)}));
    out.closeSubpath();
    return out;
}

QPainterPath boxOf(double q)
{
    QPainterPath out;
    out.addRect(QRectF(0.5 - q / 2, 0.5 - q / 2, q, q));
    return out;
}

// A pie whose slices begin at an angle (Qt's: degrees counterclockwise from 3 o'clock) and sweep by it.
void addSlice(QPainterPath &path, double radius, double start, double sweep)
{
    path.moveTo(0.5, 0.5);
    path.arcTo(QRectF(0.5 - radius, 0.5 - radius, 2 * radius, 2 * radius), start, sweep);
    path.closeSubpath();
}

QPainterPath wheel(int spokes, double p)
{
    QPainterPath out;
    for (int k = 0; k < spokes; ++k) addSlice(out, std::hypot(1.0, 1.0), 90 - 360.0 * k / spokes, -360.0 / spokes * p);
    return out;
}

// The shape of a form that opens from the middle ("out") or closes in from the edges ("in"), `p` of the way: what shows.
QPainterPath formed(bool in, double p, QPainterPath (*form)(double), double reach)
{
    if (!in) return form(p * reach);
    return whole().subtracted(form((1 - p) * reach));
}

// The last masks built, by filter, direction, and progress in 256ths: a frame asks for the same few again and again (every behavior of a
// slide, every frame of a run), and a dissolve is 400 rectangles. Bounded, so a file's odd filters cannot grow it.
constexpr size_t kCacheSize = 128;
constexpr int kSteps = 256;

struct Cache {
    QMutex lock;
    std::list<std::pair<QString, QPainterPath>> entries;   // the newest first
    int hits = 0, builds = 0;
};

Cache &cache()
{
    static Cache c;
    return c;
}

bool build(const QString &filter, double p, bool entering, QPainterPath *shown)
{
    static const QRegularExpression named(QStringLiteral("^(blinds|checkerboard|randombar|box|circle|diamond|plus|wheel)\\(([A-Za-z0-9]{1,12})\\)$"));
    QPainterPath path;
    if (filter == QLatin1String("dissolve")) {
        path = dissolve(p);
    } else if (const QRegularExpressionMatch m = named.match(filter); m.hasMatch()) {
        const QString what = m.captured(1), option = m.captured(2);
        const bool in = option == QLatin1String("in");
        if (what == QLatin1String("blinds")) path = blinds(option != QLatin1String("vertical"), p);
        else if (what == QLatin1String("checkerboard")) path = checkerboard(option != QLatin1String("down"), p);
        else if (what == QLatin1String("randombar")) path = randomBars(option != QLatin1String("vertical"), p);
        else if (what == QLatin1String("box")) path = formed(in, p, boxOf, 1);
        else if (what == QLatin1String("circle")) path = formed(in, p, [](double r) { return ellipseOf(r); }, std::hypot(1.0, 1.0) / 2);
        else if (what == QLatin1String("diamond")) path = formed(in, p, diamondOf, 1);
        else if (what == QLatin1String("plus")) path = formed(in, p, plusOf, 1);
        else path = wheel(std::clamp(option.toInt(), 1, 8), p);
    } else {
        return false;
    }
    *shown = entering ? path : whole().subtracted(path);
    return true;
}

} // namespace

bool effectMask(const QString &filter, double pIn, bool entering, QPainterPath *shown)
{
    if (filter.size() > 40) return false;   // none of the names is near this long
    const double p = std::isfinite(pIn) ? std::clamp(pIn, 0.0, 1.0) : 0.0;
    const int step = int(std::lround(p * kSteps));
    const QString key = filter + QLatin1Char(entering ? '+' : '-') + QString::number(step);
    Cache &c = cache();
    {
        QMutexLocker guard(&c.lock);
        for (auto it = c.entries.begin(); it != c.entries.end(); ++it)
            if (it->first == key) {
                c.entries.splice(c.entries.begin(), c.entries, it);
                *shown = it->second;
                ++c.hits;
                return true;
            }
    }
    if (!build(filter, double(step) / kSteps, entering, shown)) return false;
    QMutexLocker guard(&c.lock);
    ++c.builds;
    c.entries.emplace_front(key, *shown);
    if (c.entries.size() > kCacheSize) c.entries.pop_back();
    return true;
}

int effectMaskCacheHits()
{
    QMutexLocker guard(&cache().lock);
    return cache().hits;
}

int effectMaskBuilds()
{
    QMutexLocker guard(&cache().lock);
    return cache().builds;
}

int effectMaskCacheSize()
{
    QMutexLocker guard(&cache().lock);
    return int(cache().entries.size());
}

void clearEffectMaskCache()
{
    QMutexLocker guard(&cache().lock);
    cache().entries.clear();
    cache().hits = cache().builds = 0;
}

} // namespace jp::deck
