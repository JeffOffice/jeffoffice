#include "core/pathops.h"

#include <QPainterPathStroker>
#include <QPolygonF>
#include <algorithm>
#include <cmath>
#include <functional>

namespace jp::pathops {

namespace {

// Each outline is made once into a simple path (no part of it over another) under ITS OWN fill rule, and every step after that works on odd-even paths, so a hole
// stays a hole: the result of one step is such a path too, whatever the operands' rules were.
QPainterPath normal(const QPainterPath &p)
{
    QPainterPath s = p.simplified();
    s.setFillRule(Qt::OddEvenFill);
    return s;
}

// The path with every subpath closed (the boolean results are areas; a stroke of one joins at the start of each outline).
QPainterPath closedAreas(const QPainterPath &p)
{
    QPainterPath out;
    out.setFillRule(Qt::OddEvenFill);
    bool open = false;
    for (int i = 0; i < p.elementCount(); ++i) {
        const QPainterPath::Element e = p.elementAt(i);
        if (e.type == QPainterPath::MoveToElement) {
            if (open) out.closeSubpath();
            out.moveTo(e.x, e.y);
            open = true;
        } else if (e.type == QPainterPath::LineToElement) {
            out.lineTo(e.x, e.y);
        } else if (e.type == QPainterPath::CurveToElement && i + 2 < p.elementCount()) {
            out.cubicTo(e.x, e.y, p.elementAt(i + 1).x, p.elementAt(i + 1).y, p.elementAt(i + 2).x, p.elementAt(i + 2).y);
            i += 2;
        }
    }
    if (open) out.closeSubpath();
    return out;
}

bool empty(const QPainterPath &p)
{
    return p.isEmpty() || p.boundingRect().isEmpty() || p.elementCount() < 3;
}

// What both cover, and what the first covers but the second does not. (Each result is made simple again: the clipper gives a shape inside the hole of another twice over, which the
// odd-even rule makes nothing, and a piece that is nothing must be seen to be.)
QPainterPath both(const QPainterPath &a, const QPainterPath &b) { return normal(a.intersected(b)); }
QPainterPath minus(const QPainterPath &a, const QPainterPath &b) { return normal(a.subtracted(b)); }
QPainterPath either(const QPainterPath &a, const QPainterPath &b) { return normal(a.united(b)); }

bool tooBig(const QVector<QPainterPath> &parts)
{
    qint64 total = 0;
    for (const QPainterPath &p : parts) total += p.elementCount();
    return total > kMostResultElements;
}

struct Box { double l, r, t, b, x1, y1, x2, y2; };

// The boxes of the pieces of the outline: a line is one piece and a curve eight (the clipper makes lines of curves).
void addBoxes(QVector<Box> &out, const QPainterPath &p)
{
    QPointF cur;
    auto add = [&](const QPointF &a, const QPointF &b) { out.push_back(Box{std::min(a.x(), b.x()), std::max(a.x(), b.x()), std::min(a.y(), b.y()), std::max(a.y(), b.y()), a.x(), a.y(), b.x(), b.y()}); };
    for (int i = 0; i < p.elementCount(); ++i) {
        const QPainterPath::Element e = p.elementAt(i);
        if (e.type == QPainterPath::MoveToElement) {
            cur = e;
        } else if (e.type == QPainterPath::LineToElement) {
            add(cur, e);
            cur = e;
        } else if (e.type == QPainterPath::CurveToElement && i + 2 < p.elementCount()) {
            const QPointF c1 = e, c2 = p.elementAt(i + 1), end = p.elementAt(i + 2);
            QPointF prev = cur;
            for (int k = 1; k <= 8; ++k) {
                const double t = k / 8.0, u = 1 - t;
                const QPointF at = cur * (u * u * u) + c1 * (3 * u * u * t) + c2 * (3 * u * t * t) + end * (t * t * t);
                add(prev, at);
                prev = at;
            }
            cur = end;
            i += 2;
        }
    }
}

double side(const Box &s, double x, double y) { return (s.x2 - s.x1) * (y - s.y1) - (s.y2 - s.y1) * (x - s.x1); }

// How many pairs of pieces cross or touch (the pairs of neighbors in an outline touch, so the vertices are counted too): the clipper's work grows with this. Only pairs
// whose boxes overlap are looked at. At most cap + 1; the count stops once it is past cap.
qint64 countPairs(QVector<Box> boxes, qint64 cap)
{
    std::sort(boxes.begin(), boxes.end(), [](const Box &a, const Box &b) { return a.l < b.l; });
    QVector<int> active;
    qint64 pairs = 0, steps = 0;
    for (int i = 0; i < boxes.size(); ++i) {
        const Box &x = boxes[i];
        int keep = 0;
        for (int k = 0; k < active.size(); ++k) {
            const Box &y = boxes[active[k]];
            if (y.r < x.l) continue;
            active[keep++] = active[k];
            if (y.t > x.b || x.t > y.b) continue;
            if (side(x, y.x1, y.y1) * side(x, y.x2, y.y2) <= 0 && side(y, x.x1, x.y1) * side(y, x.x2, x.y2) <= 0 && ++pairs > cap) return cap + 1;
        }
        steps += active.size();
        if (steps > cap * 8 + 1000) return cap + 1;
        active.resize(keep);
        active.push_back(i);
    }
    return pairs;
}

qint64 pairsOf(const QVector<QPainterPath> &paths, qint64 cap)
{
    QVector<Box> boxes;
    for (const QPainterPath &p : paths) addBoxes(boxes, p);
    return countPairs(boxes, cap);
}

int multiplier(Op op) { return op == Op::Combine ? 3 : op == Op::Fragment ? 2 : 1; }

} // namespace

qint64 estimateWork(Op op, const QVector<QPainterPath> &outlines)
{
    return std::min<qint64>(pairsOf(outlines, kMostWork) * multiplier(op), kMostWork + 1);
}

QVector<QPainterPath> merge(Op op, const QVector<QPainterPath> &outlines, int *refused, const std::function<bool(int, int)> &progress, qint64 *workDone)
{
    auto refuse = [&](int why) {
        if (refused) *refused = why;
        return QVector<QPainterPath>();
    };
    if (workDone) *workDone = 0;
    if (outlines.size() < 2 || outlines.size() > kMostShapes) return refuse(1);
    qint64 total = 0;
    for (const QPainterPath &p : outlines) {
        if (p.elementCount() > kMostElements) return refuse(1);
        total += p.elementCount();
    }
    if (total > kMostTotalElements) return refuse(1);
    // What the work will be, counted before any of it is done; then each step is counted as it comes, and the steps together are held to kMostSteps.
    if (estimateWork(op, outlines) > kMostWork) return refuse(3);
    qint64 spent = 0;
    const int steps = int(outlines.size()) * 2;
    int done = 0;
    auto go = [&] { return !progress || progress(done++, steps); };
    // The cost of the step on `a` and `b`: false (the work is past the budget) when it does not fit.
    auto afford = [&](const QPainterPath &a, const QPainterPath &b) {
        const qint64 left = std::max<qint64>(0, kMostSteps - spent);
        const qint64 c = pairsOf({a, b}, left) * multiplier(op);
        spent += c;
        if (workDone) *workDone = spent;
        return spent <= kMostSteps;
    };
    QVector<QPainterPath> shapes;
    for (const QPainterPath &p : outlines) {
        if (!go()) return refuse(4);
        shapes << normal(p);
    }
    QVector<QPainterPath> out;
    if (op != Op::Fragment) {
        QPainterPath acc = shapes[0];
        for (int i = 1; i < shapes.size(); ++i) {
            if (!go()) return refuse(4);
            if (!afford(acc, shapes[i])) return refuse(3);
            if (op == Op::Union) acc = either(acc, shapes[i]);
            else if (op == Op::Subtract) acc = minus(acc, shapes[i]);
            else if (op == Op::Intersect) acc = both(acc, shapes[i]);
            else acc = minus(either(acc, shapes[i]), both(acc, shapes[i]));   // (Combine: what an odd number of the shapes cover: each next shape flips the area it overlaps)
            if (acc.elementCount() > kMostResultElements) return refuse(1);
            if (op == Op::Intersect && empty(acc)) return refuse(0);
        }
        out << acc;
    } else {
        // Each shape cuts every piece so far into what it covers and what it does not, and what it covers alone is a piece of its own.
        QVector<QPainterPath> pieces;
        QPainterPath covered;
        for (int i = 0; i < shapes.size(); ++i) {
            if (!go()) return refuse(4);
            QVector<QPainterPath> next;
            for (const QPainterPath &piece : pieces) {
                if (!afford(piece, shapes[i])) return refuse(3);
                const QPainterPath in = both(piece, shapes[i]), outside = minus(piece, shapes[i]);
                if (!empty(in)) next << in;
                if (!empty(outside)) next << outside;
            }
            if (i > 0 && !afford(shapes[i], covered)) return refuse(3);
            const QPainterPath alone = i == 0 ? shapes[i] : minus(shapes[i], covered);
            if (!empty(alone)) next << alone;
            covered = i == 0 ? shapes[i] : either(covered, shapes[i]);
            if (covered.elementCount() > kMostResultElements) return refuse(1);
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
        if (!empty(p)) kept << closedAreas(p);
    if (kept.isEmpty() || tooBig(kept)) return refuse(kept.isEmpty() ? 0 : 1);
    return kept;
}

QVector<QPainterPath> subpaths(const QPainterPath &p)
{
    QVector<QPainterPath> out;
    QPainterPath cur;
    auto flush = [&] {
        if (cur.elementCount() > 1 && out.size() <= kMostPieces) {
            if (cur.elementCount() > 2 && cur.currentPosition() == QPointF(cur.elementAt(0))) cur.closeSubpath();   // (it was closed: a stroke of it joins at the start)
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
            out.last().setFillRule(Qt::OddEvenFill);
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
        if (back.size() > 2 && back.first() == back.last()) out.closeSubpath();   // (a closed outline: a stroke of it joins at the start)
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
