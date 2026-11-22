#include "render/pixelsnap.h"

#include <algorithm>
#include <cmath>

namespace jp {

namespace {

int nearestLine(double v) { return int(std::floor(v + 0.5 + 1e-6)); }

std::pair<int, int> snapSpan(double lo, double hi)
{
    const int from = nearestLine(lo);
    int to = nearestLine(hi);
    if (to <= from) to = from + 1;
    return {from, to};
}

} // namespace

bool paintsAxisAligned(const QPainter *p)
{
    const QTransform t = p->deviceTransform();
    return p->transform().isInvertible() && std::abs(t.m12()) <= 1e-9 && std::abs(t.m21()) <= 1e-9 && t.m11() > 0 && t.m22() > 0;
}

std::optional<QRect> wholePixelRuleBox(const QPainter *p, QPointF a, QPointF b, double width, bool fromFar)
{
    if (!paintsAxisAligned(p)) return std::nullopt;
    const QTransform t = p->deviceTransform();
    const bool horizontal = std::abs(a.y() - b.y()) < 1e-9, vertical = std::abs(a.x() - b.x()) < 1e-9;
    if (horizontal == vertical) return std::nullopt;
    const QPointF da = t.map(a), db = t.map(b);
    const double widthPx = width * (horizontal ? t.m22() : t.m11());
    const int thick = std::max(1, int(std::floor(widthPx + 0.5)));
    const double center = horizontal ? da.y() : da.x();
    const int from = fromFar ? nearestLine(center + widthPx / 2) - thick : nearestLine(center - widthPx / 2);
    const auto along = horizontal ? snapSpan(std::min(da.x(), db.x()), std::max(da.x(), db.x())) : snapSpan(std::min(da.y(), db.y()), std::max(da.y(), db.y()));
    return horizontal ? QRect(QPoint(along.first, from), QPoint(along.second - 1, from + thick - 1)) : QRect(QPoint(from, along.first), QPoint(from + thick - 1, along.second - 1));
}

std::optional<QRect> wholePixelRectBox(const QPainter *p, const QRectF &r)
{
    if (!paintsAxisAligned(p)) return std::nullopt;
    const QRectF d = p->deviceTransform().mapRect(r.normalized());
    const auto xs = snapSpan(d.left(), d.right()), ys = snapSpan(d.top(), d.bottom());
    return QRect(QPoint(xs.first, ys.first), QPoint(xs.second - 1, ys.second - 1));
}

void fillDeviceBox(QPainter *p, const QRect &box, const QBrush &brush)
{
    const QTransform toDevice = p->transform().inverted() * p->deviceTransform();
    p->save();
    p->setWorldTransform(toDevice.inverted());
    p->setRenderHint(QPainter::Antialiasing, false);
    p->fillRect(box, brush);
    p->restore();
}

bool fillWholePixelRule(QPainter *p, QPointF a, QPointF b, double width, const QBrush &brush, bool fromFar)
{
    const auto box = wholePixelRuleBox(p, a, b, width, fromFar);
    if (!box) return false;
    fillDeviceBox(p, *box, brush);
    return true;
}

bool fillWholePixelRect(QPainter *p, const QRectF &r, const QBrush &brush)
{
    const auto box = wholePixelRectBox(p, r);
    if (!box) return false;
    fillDeviceBox(p, *box, brush);
    return true;
}

bool strokeWholePixelRect(QPainter *p, const QRectF &r, double width, const QColor &color)
{
    if (!paintsAxisAligned(p) || r.width() <= 0 || r.height() <= 0) return false;
    const QRectF n = r.normalized();
    // (The top and bottom bars run the whole width, the side bars between them: a translucent color is not drawn twice at the corners.)
    const double half = width / 2;
    const auto top = wholePixelRuleBox(p, QPointF(n.left() - half, n.top()), QPointF(n.right() + half, n.top()), width);
    const auto bottom = wholePixelRuleBox(p, QPointF(n.left() - half, n.bottom()), QPointF(n.right() + half, n.bottom()), width, true);
    const auto left = wholePixelRuleBox(p, QPointF(n.left(), n.top()), QPointF(n.left(), n.bottom()), width);
    const auto right = wholePixelRuleBox(p, QPointF(n.right(), n.top()), QPointF(n.right(), n.bottom()), width, true);
    if (!top || !bottom || !left || !right) return false;
    // (The side bars go from below the top bar to above the bottom one, or to their own ends where the rectangle is too short for both.)
    const int sideFrom = top->bottom() + 1, sideTo = bottom->top() - 1;
    fillDeviceBox(p, *top, color);
    if (bottom->top() > top->bottom()) fillDeviceBox(p, *bottom, color);
    if (sideTo >= sideFrom) {
        fillDeviceBox(p, QRect(QPoint(left->left(), sideFrom), QPoint(left->right(), sideTo)), color);
        fillDeviceBox(p, QRect(QPoint(right->left(), sideFrom), QPoint(right->right(), sideTo)), color);
    }
    return true;
}

QRectF wholePixelRectF(const QPainter *p, const QRectF &r)
{
    const auto box = wholePixelRectBox(p, r);
    if (!box) return r;
    const QTransform inverse = p->deviceTransform().inverted();
    return inverse.mapRect(QRectF(box->left(), box->top(), box->width(), box->height()));
}

} // namespace jp
