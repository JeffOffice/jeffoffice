#include "core/snapguides.h"

#include <algorithm>
#include <cmath>

namespace jp {

namespace {

double gridNearest(double v, double spacing)
{
    const double g = std::max(kLeastSnapSpacing, spacing);
    return std::round(v / g) * g;
}

// The nearest of `places` to `v` within `within`.
std::optional<double> nearestWithin(const QVector<double> &places, double v, double within)
{
    std::optional<double> best;
    double least = within;
    for (const double p : places)
        if (std::abs(p - v) < least) {
            least = std::abs(p - v);
            best = p;
        }
    return best;
}

// How far `a` and `b` (the two edges) must move for the nearer of them to reach the grid.
double gridShift(double a, double b, double spacing)
{
    const double da = gridNearest(a, spacing) - a, db = gridNearest(b, spacing) - b;
    return std::abs(da) <= std::abs(db) ? da : db;
}

// One axis of a carried box: the shift, and the guide line if a guide took it.
void snapAxis(double from, double mid, double to, const QVector<double> *places, double gridSpacing, double within, double *shift, std::optional<double> *line)
{
    double best = within;
    bool hit = false;
    if (places)
        for (const double cand : {from, mid, to})
            for (const double p : *places)
                if (std::abs(p - cand) < best) {
                    best = std::abs(p - cand);
                    *shift = p - cand;
                    *line = p;
                    hit = true;
                }
    if (!hit && gridSpacing > 0) *shift = gridShift(from, to, gridSpacing);
}

} // namespace

void addPageTargets(SnapTargets &t, const QSizeF &page, const QMarginsF &margins)
{
    t.xs << 0 << page.width() << page.width() / 2 << margins.left() << page.width() - margins.right();
    t.ys << 0 << page.height() << page.height() / 2 << margins.top() << page.height() - margins.bottom();
}

void addBoxTargets(SnapTargets &t, const QRectF &box)
{
    t.xs << box.left() << box.center().x() << box.right();
    t.ys << box.top() << box.center().y() << box.bottom();
}

SnapResult snapMovedBox(const QRectF &box, const SnapTargets *targets, double gridSpacing, double within)
{
    SnapResult out;
    out.box = box;
    double dx = 0, dy = 0;
    snapAxis(box.left(), box.center().x(), box.right(), targets ? &targets->xs : nullptr, gridSpacing, within, &dx, &out.lineX);
    snapAxis(box.top(), box.center().y(), box.bottom(), targets ? &targets->ys : nullptr, gridSpacing, within, &dy, &out.lineY);
    out.box.translate(dx, dy);
    out.movedX = dx != 0 || out.lineX;
    out.movedY = dy != 0 || out.lineY;
    return out;
}

SnapResult snapSizedBox(const QRectF &box, bool left, bool right, bool top, bool bottom, const SnapTargets *targets, double gridSpacing, double within, double least)
{
    SnapResult out;
    out.box = box;
    // One moving side of an axis.
    auto side = [&](double v, const QVector<double> *places, std::optional<double> *line) {
        if (places)
            if (const auto p = nearestWithin(*places, v, within)) {
                *line = *p;
                return *p;
            }
        return gridSpacing > 0 ? gridNearest(v, gridSpacing) : v;
    };
    if (left) out.box.setLeft(std::min(side(box.left(), targets ? &targets->xs : nullptr, &out.lineX), box.right() - least));
    if (right) out.box.setRight(std::max(side(box.right(), targets ? &targets->xs : nullptr, &out.lineX), box.left() + least));
    if (top) out.box.setTop(std::min(side(box.top(), targets ? &targets->ys : nullptr, &out.lineY), box.bottom() - least));
    if (bottom) out.box.setBottom(std::max(side(box.bottom(), targets ? &targets->ys : nullptr, &out.lineY), box.top() + least));
    // (a grid that is farther than `within` still takes a side: that is what the grid is)
    out.movedX = out.box.left() != box.left() || out.box.right() != box.right();
    out.movedY = out.box.top() != box.top() || out.box.bottom() != box.bottom();
    if (out.lineX && !((left && out.box.left() == *out.lineX) || (right && out.box.right() == *out.lineX))) out.lineX.reset();
    if (out.lineY && !((top && out.box.top() == *out.lineY) || (bottom && out.box.bottom() == *out.lineY))) out.lineY.reset();
    return out;
}

} // namespace jp
