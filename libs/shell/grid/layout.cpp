#include "grid/layout.h"

#include <algorithm>
#include <cmath>

namespace jp::grid {

namespace {
// The place on the screen of a place of the grid's: the same rounding for the end of a cell and the start of the next.
// (a place far outside any screen is kept to what an int holds, so that a sheet of a million tall rows at the largest zoom does not overflow)
int scaled(qint64 logical, double zoom) { return int(std::max(-1e9, std::min(1e9, std::round(double(logical) * zoom)))); }
qint64 clampTo(qint64 v, qint64 low, qint64 high) { return std::max(low, std::min(v, high)); }
} // namespace

GridLayout::GridLayout() : rows(1048576, 20), cols(16384, 64) {}

void GridLayout::setZoom(double zoom) { m_zoom = std::min(kMaxZoom, std::max(kMinZoom, zoom)); }

void GridLayout::setHeaders(bool shown, int rowHeaderWidth, int columnHeaderHeight)
{
    m_headerW = shown ? std::max(0, rowHeaderWidth) : 0;
    m_headerH = shown ? std::max(0, columnHeaderHeight) : 0;
}

void GridLayout::setFrozen(qint64 r, qint64 c)
{
    m_frozenRows = clampTo(r, 0, rows.count());
    m_frozenCols = clampTo(c, 0, cols.count());
    setScroll(m_scrollX, m_scrollY);
}

void GridLayout::setSkipped(qint64 r, qint64 c)
{
    rows.setSkipped(r);
    cols.setSkipped(c);
    setScroll(m_scrollX, m_scrollY);
}

void GridLayout::setSplit(qint64 width, qint64 height)
{
    m_splitW = std::max<qint64>(0, width);
    m_splitH = std::max<qint64>(0, height);
    setScroll(m_scrollX, m_scrollY);
}

qint64 GridLayout::minScrollX() const { return m_splitW > 0 ? -cols.position(m_frozenCols) : 0; }
qint64 GridLayout::minScrollY() const { return m_splitH > 0 ? -rows.position(m_frozenRows) : 0; }

bool GridLayout::scrollSplitStart(bool inRows, qint64 first)
{
    Axis &a = inRows ? rows : cols;
    const qint64 f = inRows ? m_frozenRows : m_frozenCols;
    qint64 &scroll = inRows ? m_scrollY : m_scrollX;
    if (!(inRows ? m_splitH : m_splitW)) return false;
    first = clampTo(first, 0, std::max<qint64>(0, a.lastVisible()));
    if (first == a.skipped()) return false;
    const qint64 absolute = a.position(f) + scroll + a.skippedExtent();
    a.setSkipped(first);
    scroll = absolute - a.skippedExtent() - a.position(f);
    setScroll(m_scrollX, m_scrollY);
    return true;
}

bool GridLayout::rowInTop(qint64 row) const
{
    if (m_splitH <= 0) return row < m_frozenRows;
    return row >= rows.skipped() && scaled(rows.position(row), m_zoom) < frozenHeight();
}

bool GridLayout::colInLeft(qint64 col) const
{
    if (m_splitW <= 0) return col < m_frozenCols;
    return col >= cols.skipped() && scaled(cols.position(col), m_zoom) < frozenWidth();
}

qint64 GridLayout::maxScrollX() const
{
    const qint64 last = cols.lastVisible();
    return last < m_frozenCols ? 0 : cols.position(last) - cols.position(m_frozenCols);
}

qint64 GridLayout::maxScrollY() const
{
    const qint64 last = rows.lastVisible();
    return last < m_frozenRows ? 0 : rows.position(last) - rows.position(m_frozenRows);
}

void GridLayout::setScroll(qint64 x, qint64 y)
{
    m_scrollX = clampTo(x, minScrollX(), std::max(minScrollX(), maxScrollX()));
    m_scrollY = clampTo(y, minScrollY(), std::max(minScrollY(), maxScrollY()));
}

qint64 GridLayout::bodyOriginX() const { return cols.position(m_frozenCols) + m_scrollX; }
qint64 GridLayout::bodyOriginY() const { return rows.position(m_frozenRows) + m_scrollY; }

int GridLayout::frozenWidth() const { return m_splitW > 0 ? scaled(m_splitW, m_zoom) : scaled(cols.position(m_frozenCols), m_zoom); }
int GridLayout::frozenHeight() const { return m_splitH > 0 ? scaled(m_splitH, m_zoom) : scaled(rows.position(m_frozenRows), m_zoom); }

int GridLayout::screenX(qint64 logical, bool frozen) const
{
    if (frozen) return m_headerW + scaled(logical, m_zoom);
    return m_headerW + frozenWidth() + scaled(logical - bodyOriginX(), m_zoom);
}

int GridLayout::screenY(qint64 logical, bool frozen) const
{
    if (frozen) return m_headerH + scaled(logical, m_zoom);
    return m_headerH + frozenHeight() + scaled(logical - bodyOriginY(), m_zoom);
}

int GridLayout::xOf(qint64 col) const { return screenX(cols.position(col), colInLeft(col)); }
int GridLayout::yOf(qint64 row) const { return screenY(rows.position(row), rowInTop(row)); }

int GridLayout::xEdge(qint64 col, bool frozen) const { return screenX(cols.position(col), frozen); }
int GridLayout::yEdge(qint64 row, bool frozen) const { return screenY(rows.position(row), frozen); }

QRect GridLayout::rangeRectIn(Pane pane, qint64 row1, qint64 col1, qint64 row2, qint64 col2) const
{
    const bool fr = pane == Pane::Corner || pane == Pane::Top, fc = pane == Pane::Corner || pane == Pane::Left;
    const int left = xEdge(col1, fc), right = xEdge(col2 + 1, fc), top = yEdge(row1, fr), bottom = yEdge(row2 + 1, fr);
    return QRect(left, top, std::max(0, right - left), std::max(0, bottom - top));
}

bool GridLayout::scrollToShow(qint64 row, qint64 col, qint64 *x, qint64 *y) const
{
    if (row < 0 || col < 0 || row >= rows.count() || col >= cols.count()) return false;
    const qint64 x0 = *x, y0 = *y;
    auto fit = [&](const Axis &axis, qint64 index, qint64 frozen, int total, int headers, qint64 *scroll, bool split, int splitDevice) {
        if (axis.isHidden(index)) return;
        const qint64 start0 = axis.position(index), end0 = axis.position(index + 1);
        if (split) {
            // a row (column) that the top (left) part shows whole needs no scroll; any other is brought into the part that scrolls, which may show what is above (left of) the split
            if (index >= axis.skipped() && scaled(end0, m_zoom) <= splitDevice) return;
        } else if (index < frozen) {
            return;
        }
        const qint64 frozenPos = axis.position(frozen);
        const qint64 start = start0, end = end0;
        const int fixed = split ? splitDevice : scaled(frozenPos, m_zoom);
        const qint64 shown = qint64(std::floor(double(std::max(0, total - headers - fixed)) / m_zoom));
        const qint64 origin = frozenPos + *scroll;
        if (start < origin || end - start > shown) *scroll = start - frozenPos;
        else if (end > origin + shown) *scroll = end - shown - frozenPos;
    };
    fit(rows, row, m_frozenRows, m_viewport.height(), m_headerH, y, m_splitH > 0, frozenHeight());
    fit(cols, col, m_frozenCols, m_viewport.width(), m_headerW, x, m_splitW > 0, frozenWidth());
    *x = clampTo(*x, minScrollX(), std::max(minScrollX(), maxScrollX()));
    *y = clampTo(*y, minScrollY(), std::max(minScrollY(), maxScrollY()));
    return *x != x0 || *y != y0;
}

bool GridLayout::ensureVisible(qint64 row, qint64 col)
{
    qint64 x = m_scrollX, y = m_scrollY;
    if (!scrollToShow(row, col, &x, &y)) return false;
    setScroll(x, y);
    return true;
}

QRect GridLayout::rangeRect(qint64 row1, qint64 col1, qint64 row2, qint64 col2) const
{
    // in the part of the range's first cell, to its last
    const bool fr = rowInTop(row1), fc = colInLeft(col1);
    const int left = screenX(cols.position(col1), fc), right = screenX(cols.position(col2 + 1), fc);
    const int top = screenY(rows.position(row1), fr), bottom = screenY(rows.position(row2 + 1), fr);
    return QRect(left, top, std::max(0, right - left), std::max(0, bottom - top));
}

QRect GridLayout::cellRect(qint64 row, qint64 col) const
{
    if (row < 0 || col < 0 || row >= rows.count() || col >= cols.count()) return QRect();
    const int m = merges.at(quint32(row), quint32(col));
    if (m >= 0) {
        const CellRange &r = merges.range(m);
        return rangeRect(r.row1, r.col1, r.row2, r.col2);
    }
    return rangeRect(row, col, row, col);
}

namespace {

// The row (or column) at a place of the screen, and whether it is one of the frozen: -1 if the place is on none.
struct Found {
    qint64 index = -1;
    bool frozen = false;
};

} // namespace

Hit GridLayout::hit(const QPoint &p) const
{
    Hit h;
    const int hx = m_headerW, hy = m_headerH;
    if (p.x() < 0 || p.y() < 0) return h;
    const bool inColumnHeader = p.y() < hy, inRowHeader = p.x() < hx;
    if (inColumnHeader && inRowHeader) {
        h.kind = Hit::Kind::Corner;
        return h;
    }
    // The row or column at a place in one direction. The plain division finds one that the rounding of the edges (as they are drawn) may put a row either side of, so its
    // edges are looked at and the one next to it tried.
    auto find = [&](int at, int headers, const Axis &axis, qint64 frozen, int frozenDevice, qint64 origin, bool split) {
        Found f;
        if (at < headers) return f;
        const int rel = at - headers;
        const bool fz = rel < frozenDevice;
        auto start = [&](qint64 i) { return headers + (fz ? scaled(axis.position(i), m_zoom) : frozenDevice + scaled(axis.position(i) - origin, m_zoom)); };
        qint64 i = axis.indexAt(fz ? qint64(std::floor(double(rel) / m_zoom)) : origin + qint64(std::floor(double(rel - frozenDevice) / m_zoom)));
        if (i < 0) return f;
        for (int k = 0; k < 2; ++k) {
            if (start(i) > at) {
                i = axis.previousVisible(i);
                if (i < 0) return f;
            } else {
                const qint64 next = axis.nextVisible(i);
                if (next < axis.count() && start(next) <= at) i = next;
            }
        }
        if ((!split && (i < frozen) != fz) || start(i) > at) return f;
        const qint64 next = axis.nextVisible(i);
        if (start(next < axis.count() ? next : axis.count()) <= at) return f;
        f.index = i;
        f.frozen = fz;
        return f;
    };
    const Found col = find(p.x(), hx, cols, m_frozenCols, frozenWidth(), bodyOriginX(), m_splitW > 0);
    const Found row = find(p.y(), hy, rows, m_frozenRows, frozenHeight(), bodyOriginY(), m_splitH > 0);
    if (inColumnHeader) {
        if (col.index < 0) return h;
        h.kind = Hit::Kind::ColumnHeader;
        h.col = col.index;
        h.pane = col.frozen ? Pane::Top : Pane::Body;
        return h;
    }
    if (inRowHeader) {
        if (row.index < 0) return h;
        h.kind = Hit::Kind::RowHeader;
        h.row = row.index;
        h.pane = row.frozen ? Pane::Left : Pane::Body;
        return h;
    }
    if (col.index < 0 || row.index < 0) return h;
    h.kind = Hit::Kind::Cell;
    h.row = row.index;
    h.col = col.index;
    h.pane = row.frozen ? (col.frozen ? Pane::Corner : Pane::Top) : (col.frozen ? Pane::Left : Pane::Body);
    h.merge = merges.at(quint32(h.row), quint32(h.col));
    if (h.merge >= 0) {
        h.row = merges.range(h.merge).row1;
        h.col = merges.range(h.merge).col1;
    }
    return h;
}

PaneView GridLayout::pane(Pane which) const
{
    PaneView v;
    const int ox = m_headerW, oy = m_headerH;
    const int spaceW = std::max(0, m_viewport.width() - ox), spaceH = std::max(0, m_viewport.height() - oy);
    const int fw = std::min(frozenWidth(), spaceW), fh = std::min(frozenHeight(), spaceH);
    const bool frozenRowsPart = which == Pane::Corner || which == Pane::Top, frozenColsPart = which == Pane::Corner || which == Pane::Left;
    v.rect = QRect(frozenColsPart ? ox : ox + fw, frozenRowsPart ? oy : oy + fh, frozenColsPart ? fw : spaceW - fw, frozenRowsPart ? fh : spaceH - fh);
    if (v.rect.width() <= 0 || v.rect.height() <= 0) {
        v.rect = QRect(v.rect.topLeft(), QSize(std::max(0, v.rect.width()), std::max(0, v.rect.height())));
        return v;
    }
    // the rows or columns an axis shows in a part: the frozen ones, or those from the scroll to the edge of the window
    auto span = [&](const Axis &axis, qint64 frozen, bool frozenPart, int device, qint64 origin, bool split, qint64 *first, qint64 *last) {
        qint64 a, b;
        if (frozenPart) {
            a = axis.firstVisible();
            b = split ? axis.lastVisible() : axis.previousVisible(frozen);
            if ((!split && a >= frozen) || b < a) {
                *first = 0;
                *last = -1;
                return;
            }
            // (a frozen part wider than the window shows those whose start is in it: found from the place, not by walking back over the rows that are frozen)
            const qint64 cap = axis.indexAt(qint64(std::ceil(double(device) / m_zoom)));
            if (cap >= 0 && cap < b) b = cap;
            while (b > a && scaled(axis.position(b), m_zoom) >= device) b = axis.previousVisible(b);
        } else {
            a = axis.indexAt(origin);
            if (a < 0) {
                *first = 0;
                *last = -1;
                return;
            }
            const qint64 end = origin + qint64(std::ceil(double(device) / m_zoom));
            b = axis.indexAt(std::min(end, axis.total() - 1));
            while (b > a && scaled(axis.position(b) - origin, m_zoom) >= device) b = axis.previousVisible(b);
        }
        *first = a;
        *last = b;
    };
    span(rows, m_frozenRows, frozenRowsPart, v.rect.height(), bodyOriginY(), m_splitH > 0, &v.firstRow, &v.lastRow);
    span(cols, m_frozenCols, frozenColsPart, v.rect.width(), bodyOriginX(), m_splitW > 0, &v.firstCol, &v.lastCol);
    return v;
}

QRect GridLayout::gridRect() const { return QRect(m_headerW, m_headerH, std::max(0, m_viewport.width() - m_headerW), std::max(0, m_viewport.height() - m_headerH)); }

} // namespace jp::grid
