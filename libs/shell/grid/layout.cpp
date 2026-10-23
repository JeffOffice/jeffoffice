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
    m_scrollX = clampTo(x, 0, maxScrollX());
    m_scrollY = clampTo(y, 0, maxScrollY());
}

qint64 GridLayout::bodyOriginX() const { return cols.position(m_frozenCols) + m_scrollX; }
qint64 GridLayout::bodyOriginY() const { return rows.position(m_frozenRows) + m_scrollY; }

int GridLayout::frozenWidth() const { return scaled(cols.position(m_frozenCols), m_zoom); }
int GridLayout::frozenHeight() const { return scaled(rows.position(m_frozenRows), m_zoom); }

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

int GridLayout::xOf(qint64 col) const { return screenX(cols.position(col), col < m_frozenCols); }
int GridLayout::yOf(qint64 row) const { return screenY(rows.position(row), row < m_frozenRows); }

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
    auto fit = [&](const Axis &axis, qint64 index, qint64 frozen, int total, int headers, qint64 *scroll) {
        if (index < frozen || axis.isHidden(index)) return;
        const qint64 frozenPos = axis.position(frozen);
        const qint64 start = axis.position(index), end = axis.position(index + 1);
        const qint64 shown = qint64(std::floor(double(std::max(0, total - headers - scaled(frozenPos, m_zoom))) / m_zoom));
        const qint64 origin = frozenPos + *scroll;
        if (start < origin || end - start > shown) *scroll = start - frozenPos;
        else if (end > origin + shown) *scroll = end - shown - frozenPos;
    };
    fit(rows, row, m_frozenRows, m_viewport.height(), m_headerH, y);
    fit(cols, col, m_frozenCols, m_viewport.width(), m_headerW, x);
    *x = clampTo(*x, 0, maxScrollX());
    *y = clampTo(*y, 0, maxScrollY());
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
    const bool fr = row1 < m_frozenRows, fc = col1 < m_frozenCols;
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
    auto find = [&](int at, int headers, const Axis &axis, qint64 frozen, int frozenDevice, qint64 origin) {
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
        if ((i < frozen) != fz || start(i) > at) return f;
        const qint64 next = axis.nextVisible(i);
        if (start(next < axis.count() ? next : axis.count()) <= at) return f;
        f.index = i;
        f.frozen = fz;
        return f;
    };
    const Found col = find(p.x(), hx, cols, m_frozenCols, frozenWidth(), bodyOriginX());
    const Found row = find(p.y(), hy, rows, m_frozenRows, frozenHeight(), bodyOriginY());
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
    auto span = [&](const Axis &axis, qint64 frozen, bool frozenPart, int device, qint64 origin, qint64 *first, qint64 *last) {
        qint64 a, b;
        if (frozenPart) {
            a = axis.firstVisible();
            b = axis.previousVisible(frozen);
            if (a >= frozen || b < a) {
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
    span(rows, m_frozenRows, frozenRowsPart, v.rect.height(), bodyOriginY(), &v.firstRow, &v.lastRow);
    span(cols, m_frozenCols, frozenColsPart, v.rect.width(), bodyOriginX(), &v.firstCol, &v.lastCol);
    return v;
}

QRect GridLayout::gridRect() const { return QRect(m_headerW, m_headerH, std::max(0, m_viewport.width() - m_headerW), std::max(0, m_viewport.height() - m_headerH)); }

} // namespace jp::grid
