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

QPainterPath wrapRoomAround(const QPainterPath &outline, double distance)
{
    if (!(distance > 0)) return outline;
    QPainterPathStroker st;
    st.setWidth(distance * 2);
    return outline.united(st.createStroke(outline));
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
