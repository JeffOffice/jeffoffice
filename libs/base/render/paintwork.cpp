#include "render/paintwork.h"

#include <QPaintDevice>
#include <QPaintEngine>

#include <algorithm>
#include <cmath>
#include <vector>

namespace jp {

namespace {

bool drawsPixels(const QPainter *p)
{
    return p->paintEngine() && p->paintEngine()->type() == QPaintEngine::Raster;
}

} // namespace

double areaWork(const QPainter *p, const QRectF &box)
{
    if (!drawsPixels(p)) return 5;
    const QRectF device(QPointF(0, 0), QSizeF(p->device()->width(), p->device()->height()));
    const QRectF mapped = p->transform().mapRect(box).intersected(device);
    return 5 + std::max(0.0, mapped.width()) * std::max(0.0, mapped.height()) / 2000;
}

double fillWork(const QPainter *p, const QPainterPath &path)
{
    const int elements = path.elementCount();
    if (!drawsPixels(p) || elements < 2) return 0;
    const QTransform t = p->transform();
    const int rows = p->device()->height();
    if (rows <= 0) return 0;
    auto rowOf = [&](double x, double y) { return t.m12() * x + t.m22() * y + t.dy(); };
    // The rows the path reaches (only those are counted, so a small path costs a little however tall the image is).
    double top = rows, bottom = 0;
    for (int i = 0; i < elements; ++i) {
        const QPainterPath::Element e = path.elementAt(i);
        const double y = rowOf(e.x, e.y);
        top = std::min(top, y);
        bottom = std::max(bottom, y);
    }
    const int first = int(std::floor(std::max(0.0, top))), last = int(std::ceil(std::min(double(rows), bottom)));
    if (last < first) return 0;
    std::vector<int> crossing(size_t(last - first) + 2, 0);
    auto edge = [&](const QPointF &a, const QPointF &b) {
        const double ya = rowOf(a.x(), a.y()), yb = rowOf(b.x(), b.y());
        const double from = std::max(double(first), std::min(ya, yb)), to = std::min(double(last), std::max(ya, yb));
        if (to < from) return;
        ++crossing[size_t(std::floor(from) - first)];
        --crossing[size_t(std::floor(to) - first) + 1];
    };
    QPointF start, previous;
    for (int i = 0; i < elements; ++i) {
        const QPainterPath::Element e = path.elementAt(i);
        const QPointF at(e.x, e.y);
        if (e.type == QPainterPath::MoveToElement) {
            if (i > 0 && previous != start) edge(previous, start);
            start = at;
        } else {
            edge(previous, at);
        }
        previous = at;
    }
    if (previous != start) edge(previous, start);
    double total = 0;
    int active = 0;
    for (int y = 0; y <= last - first; ++y) {
        active += crossing[size_t(y)];
        total += active * (0.5 + 0.004 * std::min(active, 2200));
    }
    return total;
}

} // namespace jp
