#include "grid/sparklinepaint.h"

#include <QPen>
#include <QPointF>
#include <QPolygonF>

#include <algorithm>
#include <cmath>

namespace jp::grid {

using jp::office::sheet::SparklineGroup;

void paintSparkline(QPainter *p, const QRect &cell, const SparklineGroup &g, const SparklineColors &c, const QVector<double> &in, double groupLow, double groupHigh, double scale)
{
    const int n = int(in.size());
    if (n == 0 || cell.width() < 4 || cell.height() < 4) return;
    // what an empty cell is: a zero, a gap, or a gap that the line goes across
    QVector<double> v = in;
    for (double &x : v)
        if (!std::isfinite(x) && g.empty == SparklineGroup::Empty::Zero) x = 0;
    double lo = std::numeric_limits<double>::infinity(), hi = -lo;
    int firstAt = -1, lastAt = -1, highAt = -1, lowAt = -1;
    for (int i = 0; i < n; ++i) {
        if (!std::isfinite(v[i])) continue;
        if (firstAt < 0) firstAt = i;
        lastAt = i;
        if (v[i] < lo) {
            lo = v[i];
            lowAt = i;
        }
        if (v[i] > hi) {
            hi = v[i];
            highAt = i;
        }
    }
    if (firstAt < 0) return;
    const bool bars = g.type != SparklineGroup::Type::Line;
    double axisLow = lo, axisHigh = hi;
    if (g.minAxis == SparklineGroup::Axis::Group && std::isfinite(groupLow)) axisLow = groupLow;
    if (g.maxAxis == SparklineGroup::Axis::Group && std::isfinite(groupHigh)) axisHigh = groupHigh;
    if (g.minAxis == SparklineGroup::Axis::Custom) axisLow = g.manualMin;
    if (g.maxAxis == SparklineGroup::Axis::Custom) axisHigh = g.manualMax;
    if (bars && g.type == SparklineGroup::Type::Column) {
        if (g.minAxis != SparklineGroup::Axis::Custom) axisLow = std::min(axisLow, 0.0);
        if (g.maxAxis != SparklineGroup::Axis::Custom) axisHigh = std::max(axisHigh, 0.0);
    }
    if (axisHigh <= axisLow) {
        axisLow -= 1;
        axisHigh += 1;
    }
    const double pad = std::max(2.0, 2.0 * scale);
    const QRectF area = QRectF(cell).adjusted(pad, pad, -pad, -pad);
    if (area.width() < 1 || area.height() < 1) return;
    auto xAt = [&](int i) {
        double f = bars ? (i + 0.5) / n : (n == 1 ? 0.5 : double(i) / (n - 1));
        if (g.rightToLeft) f = 1 - f;
        return area.left() + f * area.width();
    };
    auto yAt = [&](double value) {
        const double f = std::clamp((value - axisLow) / (axisHigh - axisLow), 0.0, 1.0);
        return area.bottom() - f * area.height();
    };
    const bool winLoss = g.type == SparklineGroup::Type::WinLoss;
    const double zeroY = winLoss ? area.center().y() : yAt(0);

    p->save();
    p->setClipRect(cell);
    p->setRenderHint(QPainter::Antialiasing, true);
    if (g.showAxis && (winLoss || (axisLow <= 0 && axisHigh >= 0))) {
        QPen axis(c.axis);
        axis.setWidthF(1);
        p->setPen(axis);
        p->drawLine(QPointF(area.left(), std::round(zeroY) + 0.5), QPointF(area.right(), std::round(zeroY) + 0.5));
    }
    auto pointColor = [&](int i, const QColor &plain) {
        if (g.high && i == highAt) return c.high;
        if (g.low && i == lowAt) return c.low;
        if (g.first && i == firstAt) return c.first;
        if (g.last && i == lastAt) return c.last;
        if (g.negative && v[i] < 0) return c.negative;
        return plain;
    };
    if (bars) {
        const double slot = area.width() / n;
        const double width = std::max(1.0, slot * (n > 1 ? 0.8 : 0.5));
        p->setPen(Qt::NoPen);
        for (int i = 0; i < n; ++i) {
            if (!std::isfinite(v[i])) continue;
            const QColor color = pointColor(i, c.series);
            double top, bottom;
            if (winLoss) {
                if (v[i] == 0) continue;
                const double half = area.height() * 0.45;
                top = v[i] > 0 ? zeroY - half : zeroY;
                bottom = v[i] > 0 ? zeroY : zeroY + half;
            } else {
                top = std::min(yAt(v[i]), zeroY);
                bottom = std::max(yAt(v[i]), zeroY);
                if (bottom - top < 1) {
                    // a value at the axis is a thin mark
                    if (v[i] == 0) continue;
                    bottom = top + 1;
                }
            }
            p->setBrush(color);
            const double left = xAt(i) - width / 2;
            p->drawRect(QRectF(left, top, width, bottom - top));
        }
    } else {
        QPen pen(c.series);
        pen.setWidthF(std::max(1.0, g.lineWeight * 96.0 / 72.0 * scale));
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        p->setPen(pen);
        p->setBrush(Qt::NoBrush);
        QPolygonF run;
        auto flush = [&] {
            if (run.size() > 1) p->drawPolyline(run);
            run.clear();
        };
        for (int i = 0; i < n; ++i) {
            if (!std::isfinite(v[i])) {
                if (g.empty != SparklineGroup::Empty::Span) flush();
                continue;
            }
            run << QPointF(xAt(i), yAt(v[i]));
        }
        flush();
        // a lone point still shows
        const double dot = std::max(3.0, pen.widthF() * 2.2);
        p->setPen(Qt::NoPen);
        for (int i = 0; i < n; ++i) {
            if (!std::isfinite(v[i])) continue;
            const QColor special = pointColor(i, QColor());
            const bool marked = special.isValid() || g.markers;
            if (!marked) continue;
            p->setBrush(special.isValid() ? special : c.markers);
            p->drawEllipse(QPointF(xAt(i), yAt(v[i])), dot / 2, dot / 2);
        }
    }
    p->restore();
}

} // namespace jp::grid
