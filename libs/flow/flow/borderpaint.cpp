#include "flow/borderpaint.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace jp::flow {

namespace {

// The part of the segment p to q that is within `reach` of the line at `at` (across: the line is horizontal, so its distance is the y difference; else vertical).
// Gives the range along the line (x for a horizontal one) the clipped part covers; false when none of it is near.
bool clipped(const QPointF &p, const QPointF &q, bool horizontal, double at, double reach, double *lo, double *hi)
{
    const double pa = horizontal ? p.y() : p.x(), qa = horizontal ? q.y() : q.x();
    const double pb = horizontal ? p.x() : p.y(), qb = horizontal ? q.x() : q.y();
    double t0 = 0, t1 = 1;
    const double d = qa - pa;
    if (d == 0) {
        if (std::abs(pa - at) > reach) return false;
    } else {
        double a = (at - reach - pa) / d, b = (at + reach - pa) / d;
        if (a > b) std::swap(a, b);
        t0 = std::max(t0, a);
        t1 = std::min(t1, b);
        if (t0 > t1) return false;
    }
    const double u = pb + (qb - pb) * t0, v = pb + (qb - pb) * t1;
    *lo = std::min(u, v);
    *hi = std::max(u, v);
    return true;
}

} // namespace

std::vector<PaintedSide> sidesUnderPath(const std::vector<PaintCell> &cells, const std::vector<QPointF> &path, double reach, qint64 *work)
{
    std::vector<PaintedSide> out;
    if (path.empty() || cells.empty()) return out;
    // The pieces: a single point is a piece of no length.
    std::vector<std::pair<QPointF, QPointF>> pieces;
    if (path.size() == 1) pieces.push_back({path[0], path[0]});
    for (size_t i = 1; i < path.size() && int(pieces.size()) < kMostPaintPieces; ++i) pieces.push_back({path[i - 1], path[i]});
    QRectF around;
    for (const auto &pc : pieces) around = around.united(QRectF(pc.first, pc.second).normalized());
    around.adjust(-reach, -reach, reach, reach);
    std::vector<std::pair<double, double>> spans;
    for (const PaintCell &cell : cells) {
        if (!cell.box.intersects(around) && !around.contains(cell.box)) continue;
        for (const CellSide side : {CellSide::Top, CellSide::Left, CellSide::Bottom, CellSide::Right}) {
            const bool horizontal = side == CellSide::Top || side == CellSide::Bottom;
            const double at = side == CellSide::Top ? cell.box.top() : side == CellSide::Bottom ? cell.box.bottom() : side == CellSide::Left ? cell.box.left() : cell.box.right();
            const double from = horizontal ? cell.box.left() : cell.box.top(), to = horizontal ? cell.box.right() : cell.box.bottom();
            const double length = to - from;
            if (length <= 0) continue;
            spans.clear();
            for (const auto &pc : pieces) {
                if (work) ++*work;
                double lo = 0, hi = 0;
                if (clipped(pc.first, pc.second, horizontal, at, reach, &lo, &hi)) spans.push_back({std::max(lo, from), std::min(hi, to)});
            }
            std::sort(spans.begin(), spans.end());
            double covered = 0, end = from;
            for (const auto &sp : spans) {
                if (sp.second <= sp.first && !(length <= 2 * reach)) continue;
                const double start = std::max(sp.first, end);
                if (sp.second > start) {
                    covered += sp.second - start;
                    end = sp.second;
                }
            }
            // A side no longer than the reach is covered by touching it at all.
            const bool touched = length <= 2 * reach && std::any_of(spans.begin(), spans.end(), [](const auto &sp) { return sp.second >= sp.first; });
            if (touched || covered * 2 >= length) out.push_back({cell.key, side});
        }
    }
    return out;
}

} // namespace jp::flow
