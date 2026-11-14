#include "core/pathpoints.h"

#include <QLineF>
#include <algorithm>
#include <cmath>

// Edit Points and connection sites on an outline (a QPainterPath): shared by the apps that edit a shape's points and glue connectors to it. The point editing is the code JeffPub's canvas
// had, moved here as it was.

namespace jp::pts {

struct El { QPainterPath::ElementType t; QPointF p; };

static QVector<El> els(const QPainterPath &path)
{
    QVector<El> v;
    for (int i = 0; i < path.elementCount(); ++i) {
        const auto e = path.elementAt(i);
        v << El{e.type, QPointF(e.x, e.y)};
    }
    return v;
}

static QPainterPath build(const QVector<El> &v)
{
    QPainterPath p;
    p.setFillRule(Qt::WindingFill);
    for (int i = 0; i < v.size(); ++i) {
        switch (v[i].t) {
        case QPainterPath::MoveToElement: p.moveTo(v[i].p); break;
        case QPainterPath::LineToElement: p.lineTo(v[i].p); break;
        case QPainterPath::CurveToElement:
            if (i + 2 < v.size()) { p.cubicTo(v[i].p, v[i + 1].p, v[i + 2].p); i += 2; }
            break;
        default: break;
        }
    }
    return p;
}

static int subpathStart(const QVector<El> &v, int i)
{
    while (i > 0 && v[i].t != QPainterPath::MoveToElement) --i;
    return i;
}

static int subpathEnd(const QVector<El> &v, int i)   // last element index of the subpath holding i
{
    int j = i + 1;
    while (j < v.size() && v[j].t != QPainterPath::MoveToElement) ++j;
    return j - 1;
}

static bool isVertex(const QVector<El> &v, int i)
{
    const auto t = v[i].t;
    if (t == QPainterPath::MoveToElement) return true;
    if (t == QPainterPath::LineToElement || (t == QPainterPath::CurveToDataElement && (i + 1 >= v.size() || v[i + 1].t != QPainterPath::CurveToDataElement))) {
        // The closing point back onto the subpath's start is not a separate vertex.
        const int s = subpathStart(v, i);
        return !(i == subpathEnd(v, i) && QLineF(v[i].p, v[s].p).length() < 0.01);
    }
    return false;
}

QVector<int> vertices(const QPainterPath &path)
{
    const auto v = els(path);
    QVector<int> out;
    for (int i = 0; i < v.size(); ++i)
        if (isVertex(v, i)) out << i;
    return out;
}

// Moves vertex i by delta; curve handles next to it move along so curves keep their shape.
QPainterPath moved(const QPainterPath &path, int i, const QPointF &delta)
{
    auto v = els(path);
    if (i < 0 || i >= v.size()) return path;
    auto shift = [&](int k) { if (k >= 0 && k < v.size()) v[k].p += delta; };
    auto withHandles = [&](int k) {
        shift(k);
        if (k + 1 < v.size() && v[k + 1].t == QPainterPath::CurveToElement) shift(k + 1);
        if (v[k].t == QPainterPath::CurveToDataElement) shift(k - 1);
    };
    withHandles(i);
    if (v[i].t == QPainterPath::MoveToElement) {
        const int e = subpathEnd(v, i);
        if (e > i && QLineF(v[e].p - delta, v[i].p - delta).length() < 0.01) withHandles(e);
    }
    return build(v);
}

static int vertexCountInSubpath(const QVector<El> &v, int i)
{
    int n = 0;
    for (int k = subpathStart(v, i); k <= subpathEnd(v, i); ++k)
        if (isVertex(v, k)) ++n;
    return n;
}

// Deletes vertex i (keeping at least three points in its outline).
QPainterPath removed(const QPainterPath &path, int i)
{
    auto v = els(path);
    if (i < 0 || i >= v.size() || vertexCountInSubpath(v, i) <= 3) return path;
    if (v[i].t == QPainterPath::LineToElement) {
        v.remove(i);
    } else if (v[i].t == QPainterPath::CurveToDataElement) {
        v.remove(i - 2, 3);
    } else if (v[i].t == QPainterPath::MoveToElement) {
        // The next vertex becomes the start; the closing point follows it.
        const int e = subpathEnd(v, i);
        const bool closed = e > i && QLineF(v[e].p, v[i].p).length() < 0.01;
        int next = i + 1;
        if (v[next].t == QPainterPath::CurveToElement) { const QPointF np = v[next + 2].p; v.remove(next, 3); v[i].p = np; }
        else { v[i].p = v[next].p; v.remove(next); }
        if (closed) {
            const int e2 = subpathEnd(v, i);
            v[e2].p = v[i].p;
        }
    }
    return build(v);
}

// Inserts a vertex on the edge nearest to local point q (within tol); returns
// the new vertex's element index through *at.
QPainterPath inserted(const QPainterPath &path, const QPointF &q, double tol, int *at)
{
    auto v = els(path);
    double best = tol;
    int seg = -1;
    double bestT = 0;
    for (int i = 1; i < v.size(); ++i) {
        if (v[i].t == QPainterPath::LineToElement) {
            const QPointF a = v[i - 1].p, b = v[i].p, d = b - a;
            const double len2 = QPointF::dotProduct(d, d);
            const double t = len2 > 0 ? std::clamp(QPointF::dotProduct(q - a, d) / len2, 0.0, 1.0) : 0;
            const double dist = QLineF(a + d * t, q).length();
            if (dist < best && t > 0.02 && t < 0.98) { best = dist; seg = i; bestT = t; }
        } else if (v[i].t == QPainterPath::CurveToElement && i + 2 < v.size()) {
            const QPointF p0 = v[i - 1].p, p1 = v[i].p, p2 = v[i + 1].p, p3 = v[i + 2].p;
            for (int k = 1; k < 40; ++k) {
                const double t = k / 40.0, u = 1 - t;
                const QPointF pt = p0 * (u * u * u) + p1 * (3 * u * u * t) + p2 * (3 * u * t * t) + p3 * (t * t * t);
                const double dist = QLineF(pt, q).length();
                if (dist < best) { best = dist; seg = i; bestT = t; }
            }
        }
    }
    if (seg < 0) { if (at) *at = -1; return path; }
    if (v[seg].t == QPainterPath::LineToElement) {
        const QPointF a = v[seg - 1].p, b = v[seg].p;
        v.insert(seg, El{QPainterPath::LineToElement, a + (b - a) * bestT});
        if (at) *at = seg;
    } else {
        // Split the cubic at t (de Casteljau) into two curves.
        const QPointF p0 = v[seg - 1].p, p1 = v[seg].p, p2 = v[seg + 1].p, p3 = v[seg + 2].p;
        const double t = bestT;
        const QPointF a = p0 + (p1 - p0) * t, b = p1 + (p2 - p1) * t, c = p2 + (p3 - p2) * t;
        const QPointF d = a + (b - a) * t, e = b + (c - b) * t, m = d + (e - d) * t;
        v[seg].p = a; v[seg + 1].p = d; v[seg + 2].p = m;
        v.insert(seg + 3, El{QPainterPath::CurveToElement, e});
        v.insert(seg + 4, El{QPainterPath::CurveToDataElement, c});
        v.insert(seg + 5, El{QPainterPath::CurveToDataElement, p3});
        if (at) *at = seg + 2;
    }
    return build(v);
}

QVector<QPointF> vertexPoints(const QPainterPath &path)
{
    QVector<QPointF> out;
    for (const int i : vertices(path)) out << QPointF(path.elementAt(i).x, path.elementAt(i).y);
    return out;
}

bool withinCaps(const QPainterPath &path)
{
    return path.elementCount() <= kMostElements && int(vertices(path).size()) <= kMostPoints;
}

QVector<QPointF> sideSites(const QPainterPath &outline, const QSizeF &size)
{
    const QPointF c(size.width() / 2, size.height() / 2);
    QVector<QPointF> sides = {QPointF(c.x(), 0), QPointF(0, c.y()), QPointF(c.x(), size.height()), QPointF(size.width(), c.y())};
    if (outline.isEmpty() || size.width() <= 0 || size.height() <= 0) return sides;
    static const QPointF dirs[4] = {{0, -1}, {-1, 0}, {0, 1}, {1, 0}};
    const QList<QPolygonF> polygons = outline.toFillPolygons();
    for (int side = 0; side < 4; ++side) {
        double far = -1;
        for (const QPolygonF &poly : polygons) {
            for (qsizetype i = 0; i < poly.size(); ++i) {
                const QPointF a = poly[i], b = poly[(i + 1) % poly.size()], e = b - a;
                // The ray c + t * dir meets the edge a + u * e: t and u from the two crossing equations.
                const double den = dirs[side].x() * e.y() - dirs[side].y() * e.x();
                if (std::abs(den) < 1e-9) continue;
                const QPointF w = a - c;
                const double t = (w.x() * e.y() - w.y() * e.x()) / den, u = (w.x() * dirs[side].y() - w.y() * dirs[side].x()) / den;
                if (t > 0 && u >= -1e-9 && u <= 1 + 1e-9 && t > far) far = t;
            }
        }
        if (far > 0) sides[side] = c + dirs[side] * far;
    }
    return sides;
}

} // namespace jp::pts
