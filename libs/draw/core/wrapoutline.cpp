#include "core/wrapoutline.h"

#include <QPainterPathStroker>

#include <algorithm>
#include <cmath>

namespace jp {

QPolygonF boundedWrapOutline(const QVector<QPointF> &points)
{
    QPolygonF out;
    for (const QPointF &p : points) {
        if (out.size() >= kMostWrapPoints) break;
        if (!std::isfinite(p.x()) || !std::isfinite(p.y())) continue;
        out << QPointF(std::clamp(p.x(), -kWrapOutlineReach, 1.0 + kWrapOutlineReach), std::clamp(p.y(), -kWrapOutlineReach, 1.0 + kWrapOutlineReach));
    }
    return out.size() < 3 ? QPolygonF() : out;
}

namespace {

double distanceToChord(const QPointF &p, const QPointF &a, const QPointF &b)
{
    const QPointF ab = b - a;
    const double len2 = ab.x() * ab.x() + ab.y() * ab.y();
    if (len2 <= 0) return std::hypot(p.x() - a.x(), p.y() - a.y());
    return std::abs((p.x() - a.x()) * ab.y() - (p.y() - a.y()) * ab.x()) / std::sqrt(len2);
}

// Douglas-Peucker on the closed ring: the points of `ring` kept so that every other one is within `tolerance` of the outline.
QVector<QPointF> simplifiedRing(const QVector<QPointF> &ring, double tolerance)
{
    const int n = int(ring.size());
    int far = 0;
    double best = -1;
    for (int i = 1; i < n; ++i) {
        const double d = std::hypot(ring[i].x() - ring[0].x(), ring[i].y() - ring[0].y());
        if (d > best) { best = d; far = i; }
    }
    QVector<bool> keep(n, false);
    keep[0] = keep[far] = true;
    QVector<std::pair<int, int>> todo;
    todo << std::make_pair(0, far) << std::make_pair(far, n);   // (n is point 0 again)
    while (!todo.isEmpty()) {
        const auto [a, b] = todo.takeLast();
        int at = -1;
        double worst = tolerance;
        for (int i = a + 1; i < b; ++i) {
            const double d = distanceToChord(ring[i], ring[a], ring[b % n]);
            if (d > worst) { worst = d; at = i; }
        }
        if (at < 0) continue;
        keep[at] = true;
        todo << std::make_pair(a, at) << std::make_pair(at, b);
    }
    QVector<QPointF> out;
    for (int i = 0; i < n; ++i)
        if (keep[i]) out << ring[i];
    return out;
}

} // namespace

QPolygonF wrapOutlineOfPath(const QPainterPath &path, const QSizeF &frame)
{
    if (path.isEmpty() || path.elementCount() > 20000 || !(frame.width() > 0) || !(frame.height() > 0)) return QPolygonF();
    QPainterPath solid = path;
    solid.setFillRule(Qt::WindingFill);
    const QPolygonF flat = solid.simplified().toFillPolygon();
    QVector<QPointF> ring;
    for (const QPointF &p : flat)
        if (std::isfinite(p.x()) && std::isfinite(p.y()) && (ring.isEmpty() || p != ring.last())) ring << p;
    while (ring.size() > 1 && ring.first() == ring.last()) ring.removeLast();
    if (ring.size() < 3) return QPolygonF();
    // (The tolerance starts at a half of a percent of the frame and doubles until the outline is short enough.)
    double tolerance = std::max(frame.width(), frame.height()) * 0.005;
    QVector<QPointF> kept = simplifiedRing(ring, tolerance);
    while (kept.size() > kMostPathOutlinePoints) {
        tolerance *= 1.5;
        kept = simplifiedRing(ring, tolerance);
    }
    for (QPointF &p : kept) p = QPointF(p.x() / frame.width(), p.y() / frame.height());
    return boundedWrapOutline(kept);
}

QPolygonF wrapOutlineOfAlpha(const QImage &image, int *scanned)
{
    if (scanned) *scanned = 0;
    if (image.isNull() || !image.hasAlphaChannel()) return QPolygonF();
    const int w = image.width(), h = image.height();
    const int step = std::max(1, (std::max(w, h) + kAlphaOutlineSide - 1) / kAlphaOutlineSide);
    const int sw = (w + step - 1) / step, sh = (h + step - 1) / step;
    if (scanned) *scanned = sw * sh;
    const int bands = std::min(kAlphaOutlineBands, sh);
    QVector<int> left(bands, -1), right(bands, -1);
    for (int sy = 0; sy < sh; ++sy) {
        const int band = std::min(bands - 1, int(qint64(sy) * bands / sh));
        const int y = std::min(h - 1, sy * step + step / 2);
        for (int sx = 0; sx < sw; ++sx) {
            if (qAlpha(image.pixel(std::min(w - 1, sx * step + step / 2), y)) <= 16) continue;
            if (left[band] < 0 || sx < left[band]) left[band] = sx;
            if (sx > right[band]) right[band] = sx;
        }
    }
    QVector<int> used;
    bool solid = true;
    for (int b = 0; b < bands; ++b) {
        if (left[b] < 0) { solid = false; continue; }
        used << b;
        solid = solid && left[b] == 0 && right[b] == sw - 1;
    }
    // (Nothing in it, or every band across: the picture keeps its frame.)
    if (used.size() < 2 || (solid && used.size() == bands)) return QPolygonF();
    auto yOf = [&](int b) { return b == used.first() ? double(b) / bands : b == used.last() ? double(b + 1) / bands : (b + 0.5) / bands; };
    QVector<QPointF> points;
    for (int b : used) points << QPointF(double(left[b]) / sw, yOf(b));
    for (int i = int(used.size()) - 1; i >= 0; --i) points << QPointF(double(right[used[i]] + 1) / sw, yOf(used[i]));
    return boundedWrapOutline(points);
}

double boundedWrapDistance(double distance)
{
    return std::isfinite(distance) ? std::clamp(distance, 0.0, kMostWrapDistance) : (distance > 0 ? kMostWrapDistance : 0.0);
}

QPainterPath wrapRoomAround(const QPainterPath &outline, double distance)
{
    // (Filled by winding, so that the middle of a star is inside it as the stroke's band is: odd-even leaves it a hole.)
    QPainterPath solid = outline;
    solid.setFillRule(Qt::WindingFill);
    distance = boundedWrapDistance(distance);
    if (!(distance > 0)) return solid.simplified();
    QPainterPathStroker st;
    st.setWidth(distance * 2);
    return solid.united(st.createStroke(solid));
}

QPolygonF wrapOutlineMoved(const QPolygonF &outline, int index, const QPointF &to)
{
    if (index < 0 || index >= outline.size()) return outline;
    QVector<QPointF> points(outline.begin(), outline.end());
    points[index] = to;
    // (Only the point that moved is made to keep inside the reach; the others are as they were.)
    const QPolygonF kept = boundedWrapOutline(points);
    return kept.size() == outline.size() ? kept : outline;
}

QPolygonF wrapOutlineWithPoint(const QPolygonF &outline, const QPointF &near, double tolerance, const QSizeF &frame, int *inserted)
{
    if (inserted) *inserted = -1;
    if (outline.size() < 3 || outline.size() >= kMostWrapPoints || !(frame.width() > 0) || !(frame.height() > 0)) return outline;
    int best = -1;
    double bestDistance = tolerance;
    QPointF at;
    for (int i = 0; i < outline.size(); ++i) {
        const QPointF a(outline[i].x() * frame.width(), outline[i].y() * frame.height());
        const QPointF b(outline[(i + 1) % outline.size()].x() * frame.width(), outline[(i + 1) % outline.size()].y() * frame.height());
        const QPointF p(near.x() * frame.width(), near.y() * frame.height());
        const QPointF ab = b - a;
        const double len2 = ab.x() * ab.x() + ab.y() * ab.y();
        const double t = len2 > 0 ? std::clamp(((p.x() - a.x()) * ab.x() + (p.y() - a.y()) * ab.y()) / len2, 0.0, 1.0) : 0.0;
        const QPointF on = a + ab * t;
        const double d = std::hypot(p.x() - on.x(), p.y() - on.y());
        if (d <= bestDistance) {
            bestDistance = d;
            best = i;
            at = QPointF(on.x() / frame.width(), on.y() / frame.height());
        }
    }
    if (best < 0) return outline;
    QPolygonF out = outline;
    out.insert(best + 1, at);
    if (inserted) *inserted = best + 1;
    return out;
}

QPolygonF wrapOutlineWithout(const QPolygonF &outline, int index)
{
    if (outline.size() <= 3 || index < 0 || index >= outline.size()) return outline;
    QPolygonF out = outline;
    out.remove(index);
    return out;
}

} // namespace jp
