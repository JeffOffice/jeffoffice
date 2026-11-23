#include "flow/borderpaint.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <utility>
#include <QHash>

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
    // The cells by the squares of a grid they reach (with the reach added), so that a piece looks at the cells near it, not at all of them.
    // A square's number is whole and kept within +-kMostSquare (about +-3.4e10 points), so that far or odd coordinates (negative, huge, infinite, not a number)
    // never overflow; the key adds the same amount to each number, so no two squares inside that range share a key. Past the range squares are merged,
    // which only makes a cell a candidate more often: every candidate is still tested against the piece's box below.
    constexpr double kSquare = 32;
    constexpr qint64 kMostSquare = qint64(1) << 30, kMostSquaresOfACell = 1024;
    auto squareOf = [](double v) -> qint64 {
        if (std::isnan(v)) return 0;
        return qint64(std::clamp(std::floor(v / kSquare), -double(kMostSquare), double(kMostSquare)));
    };
    auto key = [](qint64 x, qint64 y) { return (quint64(x + kMostSquare) << 32) | quint64(y + kMostSquare); };
    QHash<quint64, std::vector<int>> grid;
    std::vector<int> large;   // cells that reach more squares than kMostSquaresOfACell: each is looked at for every piece, not entered in every square
    for (size_t i = 0; i < cells.size(); ++i) {
        const QRectF b = cells[i].box.adjusted(-reach, -reach, reach, reach);
        if (!(b.width() >= 0) || !(b.height() >= 0)) continue;
        const qint64 x0 = squareOf(b.left()), x1 = squareOf(b.right()), y0 = squareOf(b.top()), y1 = squareOf(b.bottom());
        if ((x1 - x0 + 1) * (y1 - y0 + 1) > kMostSquaresOfACell) {
            large.push_back(int(i));
            continue;
        }
        for (qint64 x = x0; x <= x1; ++x)
            for (qint64 y = y0; y <= y1; ++y) grid[key(x, y)].push_back(int(i));
    }
    qint64 done = 0;
    qint64 &count = work ? *work : done;
    count = 0;
    // For each side that a piece comes near: the spans of it that the pieces cover (a side is its cell's index times 4, and the side).
    std::map<int, std::vector<std::pair<double, double>>> spansOf;
    std::vector<int> seen(cells.size(), -1);
    bool cut = false;
    for (size_t k = 0; k < pieces.size() && !cut; ++k) {
        const QRectF box = QRectF(pieces[k].first, pieces[k].second).normalized().adjusted(-reach, -reach, reach, reach);
        // The cells in one list, each looked at once for this piece; true when the count is used up.
        auto lookAt = [&](const std::vector<int> &list) {
            for (const int ci : list) {
                if (seen[size_t(ci)] == int(k)) continue;
                seen[size_t(ci)] = int(k);
                if (count + 5 > kMostPaintComparisons) return true;
                ++count;   // (a cell)
                const PaintCell &cell = cells[size_t(ci)];
                if (!cell.box.intersects(box)) continue;
                for (const CellSide side : {CellSide::Top, CellSide::Left, CellSide::Bottom, CellSide::Right}) {
                    if ((side == CellSide::Top && cell.continued) || (side == CellSide::Bottom && cell.continues)) continue;
                    ++count;   // (a piece against a side)
                    const bool horizontal = side == CellSide::Top || side == CellSide::Bottom;
                    const double at = side == CellSide::Top ? cell.box.top() : side == CellSide::Bottom ? cell.box.bottom() : side == CellSide::Left ? cell.box.left() : cell.box.right();
                    const double from = horizontal ? cell.box.left() : cell.box.top(), to = horizontal ? cell.box.right() : cell.box.bottom();
                    if (to - from <= 0) continue;
                    double lo = 0, hi = 0;
                    if (clipped(pieces[k].first, pieces[k].second, horizontal, at, reach, &lo, &hi)) spansOf[ci * 4 + int(side)].push_back({std::max(lo, from), std::min(hi, to)});
                }
            }
            return false;
        };
        cut = lookAt(large);
        const qint64 x1 = squareOf(box.right()), y1 = squareOf(box.bottom());
        for (qint64 x = squareOf(box.left()); x <= x1 && !cut; ++x)
            for (qint64 y = squareOf(box.top()); y <= y1 && !cut; ++y) {
                if (count >= kMostPaintComparisons) {
                    cut = true;
                    break;
                }
                ++count;   // (a square looked into)
                const auto found = grid.constFind(key(x, y));
                if (found == grid.constEnd()) continue;
                cut = lookAt(found.value());
            }
    }
    for (auto &entry : spansOf) {
        const PaintCell &cell = cells[size_t(entry.first / 4)];
        const CellSide side = CellSide(entry.first % 4);
        const bool horizontal = side == CellSide::Top || side == CellSide::Bottom;
        const double from = horizontal ? cell.box.left() : cell.box.top(), to = horizontal ? cell.box.right() : cell.box.bottom();
        const double length = to - from;
        std::vector<std::pair<double, double>> &spans = entry.second;
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
    return out;
}

} // namespace jp::flow
