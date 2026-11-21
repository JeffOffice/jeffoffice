#include "core/pathops.h"

#include <QPainterPathStroker>
#include <QPolygonF>
#include <algorithm>
#include <cmath>

namespace jp::pathops {

namespace {

QPainterPath withWinding(QPainterPath p)
{
    p.setFillRule(Qt::WindingFill);
    return p;
}

bool empty(const QPainterPath &p)
{
    return p.isEmpty() || p.boundingRect().isEmpty() || p.elementCount() < 3;
}

// What both cover, and what the first covers but the second does not.
QPainterPath both(const QPainterPath &a, const QPainterPath &b) { return withWinding(a).intersected(withWinding(b)); }
QPainterPath minus(const QPainterPath &a, const QPainterPath &b) { return withWinding(a).subtracted(withWinding(b)); }
QPainterPath either(const QPainterPath &a, const QPainterPath &b) { return withWinding(a).united(withWinding(b)); }

bool tooBig(const QVector<QPainterPath> &parts)
{
    qint64 total = 0;
    for (const QPainterPath &p : parts) total += p.elementCount();
    return total > kMostResultElements;
}

} // namespace

QVector<QPainterPath> merge(Op op, const QVector<QPainterPath> &outlines, int *refused)
{
    auto refuse = [&](int why) {
        if (refused) *refused = why;
        return QVector<QPainterPath>();
    };
    if (outlines.size() < 2 || outlines.size() > kMostShapes) return refuse(1);
    qint64 total = 0;
    for (const QPainterPath &p : outlines) {
        if (p.elementCount() > kMostElements) return refuse(1);
        total += p.elementCount();
    }
    if (total > kMostTotalElements) return refuse(1);
    QVector<QPainterPath> out;
    if (op == Op::Union) {
        QPainterPath acc = outlines[0];
        for (int i = 1; i < outlines.size(); ++i) {
            acc = either(acc, outlines[i]);
            if (acc.elementCount() > kMostResultElements) return refuse(1);
        }
        out << acc;
    } else if (op == Op::Subtract) {
        QPainterPath acc = outlines[0];
        for (int i = 1; i < outlines.size(); ++i) acc = minus(acc, outlines[i]);
        out << acc;
    } else if (op == Op::Intersect) {
        QPainterPath acc = outlines[0];
        for (int i = 1; i < outlines.size(); ++i) acc = both(acc, outlines[i]);
        out << acc;
    } else if (op == Op::Combine) {
        // (what an odd number of the shapes cover: each next shape flips the area it overlaps)
        QPainterPath acc = outlines[0];
        for (int i = 1; i < outlines.size(); ++i) {
            acc = minus(either(acc, outlines[i]), both(acc, outlines[i]));
            if (acc.elementCount() > kMostResultElements) return refuse(1);
        }
        out << acc;
    } else {
        // Each shape cuts every piece so far into what it covers and what it does not, and what it covers alone is a piece of its own.
        QVector<QPainterPath> pieces;
        QPainterPath covered;
        for (int i = 0; i < outlines.size(); ++i) {
            QVector<QPainterPath> next;
            for (const QPainterPath &piece : pieces) {
                const QPainterPath in = both(piece, outlines[i]), outside = minus(piece, outlines[i]);
                if (!empty(in)) next << in;
                if (!empty(outside)) next << outside;
            }
            const QPainterPath alone = i == 0 ? withWinding(outlines[i]) : minus(outlines[i], covered);
            if (!empty(alone)) next << alone;
            covered = i == 0 ? withWinding(outlines[i]) : either(covered, outlines[i]);
            if (next.size() > kMostPieces || tooBig(next)) return refuse(2);
            pieces = next;
        }
        for (const QPainterPath &p : pieces) {
            const QVector<QPainterPath> parts = islands(p);
            if (parts.isEmpty()) return refuse(2);
            out += parts;
            if (out.size() > kMostPieces) return refuse(2);
        }
    }
    QVector<QPainterPath> kept;
    for (const QPainterPath &p : out)
        if (!empty(p)) kept << p;
    if (kept.isEmpty() || tooBig(kept)) return refuse(kept.isEmpty() ? 0 : 1);
    return kept;
}

QVector<QPainterPath> subpaths(const QPainterPath &p)
{
    QVector<QPainterPath> out;
    QPainterPath cur;
    auto flush = [&] {
        if (cur.elementCount() > 1 && out.size() <= kMostPieces) {
            cur.setFillRule(p.fillRule());
            out << cur;
        }
        cur = QPainterPath();
    };
    for (int i = 0; i < p.elementCount(); ++i) {
        const QPainterPath::Element e = p.elementAt(i);
        if (e.type == QPainterPath::MoveToElement) {
            flush();
            cur.moveTo(e.x, e.y);
        } else if (e.type == QPainterPath::LineToElement) {
            cur.lineTo(e.x, e.y);
        } else if (e.type == QPainterPath::CurveToElement && i + 2 < p.elementCount()) {
            cur.cubicTo(e.x, e.y, p.elementAt(i + 1).x, p.elementAt(i + 1).y, p.elementAt(i + 2).x, p.elementAt(i + 2).y);
            i += 2;
        }
    }
    flush();
    return out;
}

QVector<QPainterPath> islands(const QPainterPath &path)
{
    const QVector<QPainterPath> subs = subpaths(path);
    if (subs.size() > kMostPieces) return {};
    const int n = subs.size();
    QVector<QPointF> probe(n);
    QVector<double> area(n);
    for (int i = 0; i < n; ++i) {
        probe[i] = subs[i].elementAt(0);
        const QRectF r = subs[i].boundingRect();
        area[i] = r.width() * r.height();
    }
    // How many other subpaths lie round each one: an even number is an outer outline, an odd number a hole.
    QVector<int> depth(n, 0), parent(n, -1);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j) {
            if (i == j || area[j] < area[i] || !subs[j].boundingRect().contains(subs[i].boundingRect()) || !subs[j].contains(probe[i])) continue;
            ++depth[i];
            if (parent[i] < 0 || area[j] < area[parent[i]]) parent[i] = j;
        }
    QVector<QPainterPath> out;
    QVector<int> at(n, -1);
    for (int i = 0; i < n; ++i)
        if (depth[i] % 2 == 0) {
            at[i] = out.size();
            QPainterPath one = subs[i];
            one.setFillRule(Qt::OddEvenFill);
            out << one;
        }
    for (int i = 0; i < n; ++i) {
        if (depth[i] % 2 == 0) continue;
        int outer = parent[i];
        while (outer >= 0 && at[outer] < 0) outer = parent[outer];
        if (outer >= 0) out[at[outer]].addPath(subs[i]);
        else {
            at[i] = out.size();
            out << subs[i];
        }
    }
    return out;
}

QPainterPath polygon(const QPainterPath &path, double tolerance)
{
    QPainterPath out;
    out.setFillRule(path.fillRule());
    const double flat = std::max(0.01, tolerance);
    // (toSubpathPolygons flattens to a fixed pixel size: a matrix scaled up by 1 / tolerance makes its flatness that many points)
    const QTransform up = QTransform::fromScale(1 / flat, 1 / flat), down = QTransform::fromScale(flat, flat);
    for (const QPolygonF &poly : path.toSubpathPolygons(up)) {
        if (poly.size() < 2) continue;
        const QPolygonF back = down.map(poly);
        out.moveTo(back[0]);
        for (int i = 1; i < back.size(); ++i) out.lineTo(back[i]);
    }
    return out;
}

QPainterPath contour(const QPainterPath &path, double width)
{
    QPainterPathStroker stroker;
    stroker.setWidth(std::clamp(width, 0.1, 1000.0));
    stroker.setJoinStyle(Qt::MiterJoin);
    return stroker.createStroke(path);
}

} // namespace jp::pathops
