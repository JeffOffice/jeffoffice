#include "core/wrapoutline.h"

#include <QPainter>
#include <QPainterPathStroker>

#include <algorithm>
#include <atomic>
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

// The ring with each edge moved outward by `d` (a corner is mitered, no longer than five times d).
QVector<QPointF> offsetRing(const QVector<QPointF> &ring, double d)
{
    const int n = int(ring.size());
    double area = 0;
    for (int i = 0; i < n; ++i) area += ring[i].x() * ring[(i + 1) % n].y() - ring[(i + 1) % n].x() * ring[i].y();
    const double sign = area >= 0 ? 1.0 : -1.0;
    auto normal = [&](const QPointF &a, const QPointF &b) {
        const double len = std::hypot(b.x() - a.x(), b.y() - a.y());
        return len > 0 ? QPointF(sign * (b.y() - a.y()) / len, -sign * (b.x() - a.x()) / len) : QPointF();
    };
    QVector<QPointF> out;
    for (int i = 0; i < n; ++i) {
        const QPointF n1 = normal(ring[(i + n - 1) % n], ring[i]), n2 = normal(ring[i], ring[(i + 1) % n]);
        const double denom = std::max(0.2, 1.0 + n1.x() * n2.x() + n1.y() * n2.y());
        out << QPointF(ring[i].x() + (n1.x() + n2.x()) / denom * d, ring[i].y() + (n1.y() + n2.y()) / denom * d);
    }
    return out;
}

} // namespace

QPolygonF wrapOutlineOfPath(const QPainterPath &path, const QSizeF &frame)
{
    if (path.isEmpty() || path.elementCount() > 20000 || !(frame.width() > 0) || !(frame.height() > 0)) return QPolygonF();
    QPainterPath solid = path;
    solid.setFillRule(Qt::WindingFill);
    solid = solid.simplified();
    const QList<QPolygonF> parts = solid.toSubpathPolygons();
    if (parts.size() > 1) {
        // (Several parts, or a hole: drawn small, and read as a picture is.)
        const double k = double(kAlphaOutlineSide) / std::max(frame.width(), frame.height());
        QImage drawn(std::max(1, int(std::lround(frame.width() * k))), std::max(1, int(std::lround(frame.height() * k))), QImage::Format_ARGB32_Premultiplied);
        drawn.fill(Qt::transparent);
        {
            QPainter painter(&drawn);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.scale(k, k);
            painter.fillPath(solid, Qt::black);
        }
        return wrapOutlineOfAlpha(drawn);
    }
    if (parts.isEmpty()) return QPolygonF();
    QVector<QPointF> ring;
    for (const QPointF &p : parts[0])
        if (std::isfinite(p.x()) && std::isfinite(p.y()) && (ring.isEmpty() || p != ring.last())) ring << p;
    while (ring.size() > 1 && ring.first() == ring.last()) ring.removeLast();
    if (ring.size() < 3) return QPolygonF();
    // (The tolerance starts at a half of a percent of the frame and grows until the outline is short enough.)
    double tolerance = std::max(frame.width(), frame.height()) * 0.005;
    QVector<QPointF> kept = simplifiedRing(ring, tolerance);
    while (kept.size() > kMostPathOutlinePoints) {
        tolerance *= 1.5;
        kept = simplifiedRing(ring, tolerance);
    }
    kept = offsetRing(kept, tolerance);
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
    // (A picture of more pixels than the most is read at every stride-th one.)
    const double all = double(w) * h;
    const int stride = all > double(kMostAlphaScanPixels) ? int(std::ceil(std::sqrt(all / double(kMostAlphaScanPixels)))) : 1;
    const int bands = std::min(kAlphaOutlineBands, sh);
    QVector<int> left(bands, -1), right(bands, -1);
    qint64 looked = 0;
    for (int y = 0; y < h; y += stride) {
        const int band = std::min(bands - 1, int(qint64(y / step) * bands / sh));
        for (int x = 0; x < w; x += stride) {
            ++looked;
            if (qAlpha(image.pixel(x, y)) <= 16) continue;
            const int sx = x / step;
            if (left[band] < 0 || sx < left[band]) left[band] = sx;
            if (sx > right[band]) right[band] = sx;
        }
    }
    if (scanned) *scanned = int(std::min<qint64>(looked, 2000000000));
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

QRectF wrapSideWall(const QRectF &room, const QRectF &frame, bool textLeft, bool largest)
{
    const bool onLeft = textLeft || (largest && room.left() - frame.left() >= frame.right() - room.right());
    QRectF wall;
    if (onLeft) wall.setCoords(room.left(), room.top(), 1e5, room.bottom());
    else wall.setCoords(-1e5, room.top(), room.right(), room.bottom());
    return wall;
}

namespace {

std::atomic<qint64> g_clipperCalls{0}, g_clipperElements{0};

// Douglas-Peucker on an open run, without recursion (a zig-zag would make the depth as long as the run): the indexes kept in `keep`.
void simplifyRunFlat(const QPolygonF &in, double tol, QVector<bool> &keep)
{
    QVector<QPair<int, int>> todo;
    todo << qMakePair(0, int(in.size()) - 1);
    while (!todo.isEmpty()) {
        const auto [a, b] = todo.takeLast();
        if (b <= a + 1) continue;
        const QPointF p = in[a], q = in[b];
        double best = -1;
        int at = -1;
        for (int i = a + 1; i < b; ++i) {
            const double d = distanceToChord(in[i], p, q);
            if (d > best) { best = d; at = i; }
        }
        if (best > tol) {
            keep[at] = true;
            todo << qMakePair(a, at) << qMakePair(at, b);
        }
    }
}

QPolygonF hullOf(QPolygonF pts)
{
    std::sort(pts.begin(), pts.end(), [](const QPointF &a, const QPointF &b) { return a.x() < b.x() || (a.x() == b.x() && a.y() < b.y()); });
    if (pts.size() < 3) return pts;
    auto cross = [](const QPointF &o, const QPointF &a, const QPointF &b) { return (a.x() - o.x()) * (b.y() - o.y()) - (a.y() - o.y()) * (b.x() - o.x()); };
    QPolygonF h(2 * pts.size());
    int k = 0;
    for (int i = 0; i < pts.size(); ++i) {
        while (k >= 2 && cross(h[k - 2], h[k - 1], pts[i]) <= 0) --k;
        h[k++] = pts[i];
    }
    for (int i = int(pts.size()) - 2, t = k + 1; i >= 0; --i) {
        while (k >= t && cross(h[k - 2], h[k - 1], pts[i]) <= 0) --k;
        h[k++] = pts[i];
    }
    h.resize(k - 1);
    return h;
}

} // namespace

QPainterPath wrapPathBounded(const QPainterPath &path)
{
    if (path.elementCount() <= kMostWrapPathElements) return path;
    const QRectF box = path.controlPointRect();
    if (!(box.width() >= 0) || !(box.height() >= 0) || !std::isfinite(box.width()) || !std::isfinite(box.height())) return QPainterPath();
    const double diag = std::hypot(box.width(), box.height());
    QList<QPolygonF> rings = path.toFillPolygons();
    qint64 total = 0;
    for (const QPolygonF &r : rings) total += r.size();
    auto fromRings = [&](const QList<QPolygonF> &rs) {
        QPainterPath out;
        out.setFillRule(Qt::WindingFill);
        for (const QPolygonF &r : rs) out.addPolygon(r), out.closeSubpath();
        return out;
    };
    // (The longest run the simplifier is given is bounded first: a point close to the one kept before it is dropped, which costs one step each.)
    const double tol0 = std::max(diag * kWrapSimplifyShare, 0.02);
    QList<QPolygonF> thin;
    qint64 thinTotal = 0;
    for (const QPolygonF &r : rings) {
        QPolygonF t;
        for (const QPointF &p : r) {
            if (!std::isfinite(p.x()) || !std::isfinite(p.y())) continue;
            if (t.isEmpty() || std::hypot(p.x() - t.last().x(), p.y() - t.last().y()) >= tol0 * 0.5) t << p;
        }
        if (t.size() >= 3) { thin << t; thinTotal += t.size(); }
    }
    if (thin.isEmpty()) return QPainterPath();
    if (thinTotal <= kMostWrapSimplifyInput) {
        for (double tol = tol0; tol <= diag; tol *= 1.6) {
            QList<QPolygonF> out;
            qint64 n = 0;
            for (const QPolygonF &r : thin) {
                QPolygonF closed = r;
                closed << r.first();
                QVector<bool> keep(closed.size(), false);
                keep.first() = keep.last() = true;
                simplifyRunFlat(closed, tol, keep);
                QPolygonF o;
                for (int i = 0; i + 1 < closed.size(); ++i)
                    if (keep[i]) o << closed[i];
                if (o.size() >= 3) { out << o; n += o.size(); }
            }
            if (n <= kMostWrapPathElements) return out.isEmpty() ? QPainterPath() : fromRings(out);
        }
    }
    // (Too many points to keep the shape in the budget: the convex hull, and a rectangle when that has too many.)
    QPolygonF all;
    for (const QPolygonF &r : thin) all += r;
    QPolygonF hull = hullOf(all);
    if (hull.size() >= 3 && hull.size() <= kMostWrapPathElements) return fromRings({hull});
    QPainterPath rect;
    rect.addRect(box);
    return rect;
}

QPainterPath wrapRoomAround(const QPainterPath &outline, double distance)
{
    // (Filled by winding, so that the middle of a star is inside it as the stroke's band is: odd-even leaves it a hole.)
    QPainterPath solid = wrapPathBounded(outline);
    solid.setFillRule(Qt::WindingFill);
    distance = boundedWrapDistance(distance);
    ++g_clipperCalls;
    g_clipperElements += solid.elementCount();
    if (!(distance > 0)) return solid.simplified();
    QPainterPathStroker st;
    st.setWidth(distance * 2);
    return solid.united(st.createStroke(solid));
}

qint64 wrapClipperCallsForTest() { return g_clipperCalls.load(); }
qint64 wrapClipperElementsForTest() { return g_clipperElements.load(); }

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
