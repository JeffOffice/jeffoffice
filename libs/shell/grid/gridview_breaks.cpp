// The grid view as the pages it prints on (see GridView::setBreakView): the cells outside the printed blocks shaded, each page numbered, and the lines where pages begin and the edges of the
// printed blocks dragged with the mouse. The view draws and tells what was dragged; the window changes the sheet's page setup.

#include "grid/gridview.h"

#include "app/icons.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QRegion>

#include <algorithm>
#include <cmath>

namespace jp::grid {

namespace {

constexpr int kGrab = 3;   // pixels either side of a line that take hold of it

// The most pages and blocks the view keeps (a sheet is cut into 50,000 pages at most, and has 1,024 print areas at most).
constexpr size_t kMaxBreakPages = 50000;
constexpr size_t kMaxBreakAreas = 1024;

} // namespace

void GridView::setBreakView(bool on)
{
    if (m_breakView == on) return;
    m_breakView = on;
    if (!on) {
        m_breakAreas.clear();
        m_breakPageList.clear();
        if (m_drag == Drag::BreakLine || m_drag == Drag::BreakEdge) m_drag = Drag::None;
    }
    viewport()->update();
}

void GridView::setBreakPages(std::vector<GridRange> areas, std::vector<BreakPage> pages, bool editable)
{
    if (areas.size() > kMaxBreakAreas) areas.resize(kMaxBreakAreas);
    if (pages.size() > kMaxBreakPages) pages.resize(kMaxBreakPages);
    // (the same pages and areas again, as a sheet shown again gives with none: nothing to draw)
    const auto samePage = [](const BreakPage &a, const BreakPage &b) { return a.row1 == b.row1 && a.row2 == b.row2 && a.col1 == b.col1 && a.col2 == b.col2 && a.number == b.number; };
    if (editable == m_breakEditable && areas == m_breakAreas && pages.size() == m_breakPageList.size() && std::equal(pages.begin(), pages.end(), m_breakPageList.begin(), samePage)) return;
    m_breakAreas = std::move(areas);
    m_breakPageList = std::move(pages);
    m_breakEditable = editable;
    viewport()->update();
}

GridView::BreakGrab GridView::breakGrabAt(const QPoint &vp) const
{
    BreakGrab g;
    if (!m_breakView || !m_breakEditable) return g;
    const QPoint p = mirror(vp);
    const GridLayout &L = m_sheet.layout();
    for (const Pane pane : {Pane::Corner, Pane::Top, Pane::Left, Pane::Body}) {
        const PaneView pv = L.pane(pane);
        if (pv.isEmpty() || !pv.rect.contains(p)) continue;
        const bool fr = pane == Pane::Corner || pane == Pane::Top, fc = pane == Pane::Corner || pane == Pane::Left;
        auto first = [](const std::vector<PageBreakLine> &v, qint64 at) {
            return std::lower_bound(v.begin(), v.end(), at, [](const PageBreakLine &l, qint64 value) { return l.before < value; });
        };
        // (the area a line is in: the one whose columns (rows) it runs across, and that it divides)
        auto areaOf = [&](bool column, const PageBreakLine &line) {
            for (size_t i = 0; i < m_breakAreas.size(); ++i) {
                const GridRange &a = m_breakAreas[i];
                if (column ? (a.row1 == line.from && a.row2 == line.to && line.before > a.col1 && line.before <= a.col2) : (a.col1 == line.from && a.col2 == line.to && line.before > a.row1 && line.before <= a.row2))
                    return int(i);
            }
            return -1;
        };
        for (auto it = first(m_breakRows, pv.firstRow); it != m_breakRows.end() && it->before <= pv.lastRow + 1; ++it) {
            const int y = L.yEdge(it->before, fr), x0 = L.xEdge(it->from, fc), x1 = L.xEdge(it->to + 1, fc);
            if (std::abs(p.y() - y) <= kGrab && p.x() >= x0 && p.x() <= x1) {
                g.kind = BreakGrab::Kind::Line;
                g.column = false;
                g.before = it->before;
                g.area = areaOf(false, *it);
                return g;
            }
        }
        for (auto it = first(m_breakCols, pv.firstCol); it != m_breakCols.end() && it->before <= pv.lastCol + 1; ++it) {
            const int x = L.xEdge(it->before, fc), y0 = L.yEdge(it->from, fr), y1 = L.yEdge(it->to + 1, fr);
            if (std::abs(p.x() - x) <= kGrab && p.y() >= y0 && p.y() <= y1) {
                g.kind = BreakGrab::Kind::Line;
                g.column = true;
                g.before = it->before;
                g.area = areaOf(true, *it);
                return g;
            }
        }
        for (size_t i = 0; i < m_breakAreas.size(); ++i) {
            const GridRange &a = m_breakAreas[i];
            const int x0 = L.xEdge(a.col1, fc), x1 = L.xEdge(a.col2 + 1, fc), y0 = L.yEdge(a.row1, fr), y1 = L.yEdge(a.row2 + 1, fr);
            const bool alongX = p.x() >= x0 - kGrab && p.x() <= x1 + kGrab, alongY = p.y() >= y0 - kGrab && p.y() <= y1 + kGrab;
            int side = -1;
            if (alongX && std::abs(p.y() - y0) <= kGrab) side = 0;
            else if (alongX && std::abs(p.y() - y1) <= kGrab) side = 1;
            else if (alongY && std::abs(p.x() - x0) <= kGrab) side = 2;
            else if (alongY && std::abs(p.x() - x1) <= kGrab) side = 3;
            if (side < 0) continue;
            g.kind = BreakGrab::Kind::Edge;
            g.area = int(i);
            g.side = side;
            return g;
        }
    }
    return g;
}

// The row (column) a point is nearest the start of: the boundary between two rows (columns) it is closest to. `inside` says whether the point is on a cell.
qint64 GridView::breakTargetAt(const QPoint &lp, bool column, bool *inside) const
{
    const GridLayout &L = m_sheet.layout();
    const Hit h = L.hit(lp);
    *inside = h.kind == Hit::Kind::Cell;
    if (!*inside) return -1;
    const QRect r = L.cellRect(h.row, h.col);
    if (column) return lp.x() * 2 >= r.left() + r.right() + 1 ? h.col + (r.width() > 0 ? 1 : 0) : h.col;
    return lp.y() * 2 >= r.top() + r.bottom() + 1 ? h.row + (r.height() > 0 ? 1 : 0) : h.row;
}

bool GridView::breakPress(const QPoint &lp)
{
    const BreakGrab g = breakGrabAt(mirror(lp));   // (breakGrabAt takes the viewport's point)
    if (g.kind == BreakGrab::Kind::None) return false;
    m_drag = g.kind == BreakGrab::Kind::Line ? Drag::BreakLine : Drag::BreakEdge;
    m_breakGrab = g;
    m_breakAt = lp;
    viewport()->update();
    return true;
}

bool GridView::breakMove(const QPoint &lp)
{
    if (m_drag != Drag::BreakLine && m_drag != Drag::BreakEdge) return false;
    m_breakAt = lp;
    viewport()->update();
    return true;
}

bool GridView::breakRelease()
{
    if (m_drag != Drag::BreakLine && m_drag != Drag::BreakEdge) return false;
    const bool line = m_drag == Drag::BreakLine;
    m_drag = Drag::None;
    viewport()->update();
    const BreakGrab g = m_breakGrab;
    if (g.area < 0 && !line) return true;
    const bool column = line ? g.column : g.side >= 2;
    bool inside = false;
    qint64 at = breakTargetAt(m_breakAt, column, &inside);
    if (line) {
        // (let go away from the printed cells, or at an end of the block the line divides: the line is to go)
        if (g.area < 0 || size_t(g.area) >= m_breakAreas.size()) return true;
        const GridRange &a = m_breakAreas[size_t(g.area)];
        const qint64 low = column ? a.col1 : a.row1, high = column ? a.col2 : a.row2;
        if (!inside || at <= low || at > high) at = -1;
        if (at != g.before) Q_EMIT breakMoveRequested(column, g.before, at);
        return true;
    }
    if (!inside || size_t(g.area) >= m_breakAreas.size()) return true;
    const GridRange &a = m_breakAreas[size_t(g.area)];
    qint64 value = at, old = 0;
    switch (g.side) {
    case 0: old = a.row1; value = at; if (value > a.row2) return true; break;
    case 1: old = a.row2; value = at - 1; if (value < a.row1) return true; break;
    case 2: old = a.col1; value = at; if (value > a.col2) return true; break;
    default: old = a.col2; value = at - 1; if (value < a.col1) return true; break;
    }
    if (value != old) Q_EMIT printEdgeRequested(g.area, g.side, value);
    return true;
}

// The shading outside the printed blocks, the numbers of the pages, the edges of the blocks, and the line being dragged. Found by the rows and columns each part of the grid shows.
void GridView::paintBreakView(QPainter *p, const QRect &clip)
{
    const GridLayout &L = m_sheet.layout();
    const bool dark = uiDark();
    const QColor shade = dark ? QColor(0, 0, 0, 140) : QColor(96, 100, 112, 120);
    const QColor mark = dark ? QColor(255, 255, 255, 120) : QColor(40, 44, 60, 130);
    const QColor ink = dark ? QColor(0x7F, 0xA8, 0xFF) : QColor(0x1F, 0x3F, 0xA8);
    p->save();
    p->setRenderHint(QPainter::Antialiasing, false);
    for (const Pane pane : {Pane::Body, Pane::Left, Pane::Top, Pane::Corner}) {
        const PaneView pv = L.pane(pane);
        const QRect area = pv.rect & clip;
        if (pv.isEmpty() || area.isEmpty()) continue;
        QRegion outside(area);
        for (const GridRange &a : m_breakAreas) {
            if (a.row2 < pv.firstRow || a.row1 > pv.lastRow || a.col2 < pv.firstCol || a.col1 > pv.lastCol) continue;
            outside -= L.rangeRectIn(pane, a.row1, a.col1, a.row2, a.col2);
        }
        p->setClipRect(area);
        p->setPen(Qt::NoPen);
        p->setBrush(shade);
        for (const QRect &r : outside) p->drawRect(r);
        // the numbers of the pages
        p->setBrush(Qt::NoBrush);
        p->setPen(mark);
        const QString format = QCoreApplication::translate("GridView", "Page %1");
        for (const BreakPage &page : m_breakPageList) {
            if (page.row2 < pv.firstRow || page.row1 > pv.lastRow || page.col2 < pv.firstCol || page.col1 > pv.lastCol) continue;
            const QRect box = L.rangeRectIn(pane, page.row1, page.col1, page.row2, page.col2) & pv.rect;
            if (box.width() < 12 || box.height() < 20) continue;
            // (a page of a few narrow columns is lettered over the shading beside it too, not further left than the part of the grid it is in)
            QFont f = p->font();
            f.setBold(true);
            f.setPixelSize(std::clamp(box.height() / 4, 12, 96));
            p->setFont(f);
            const QFontMetrics fm(f);
            const QString label = format.arg(page.number);
            const int w = fm.horizontalAdvance(label) + 4;
            const int x = std::max(area.left() + 4, box.center().x() - w / 2);
            p->drawText(QRect(x, box.center().y() - fm.height() / 2, w, fm.height()), Qt::AlignCenter | Qt::TextSingleLine, label);
        }
        // the edges of the blocks
        QPen edge(ink, 2);
        edge.setCosmetic(true);
        p->setPen(edge);
        for (const GridRange &a : m_breakAreas) {
            if (a.row2 < pv.firstRow || a.row1 > pv.lastRow || a.col2 < pv.firstCol || a.col1 > pv.lastCol) continue;
            p->drawRect(L.rangeRectIn(pane, a.row1, a.col1, a.row2, a.col2).adjusted(0, 0, -1, -1));
        }
    }
    // the line or edge being dragged
    if (m_drag == Drag::BreakLine || m_drag == Drag::BreakEdge) {
        const bool column = m_drag == Drag::BreakLine ? m_breakGrab.column : m_breakGrab.side >= 2;
        QPen ghost(QColor(0xE0, 0x6A, 0x10), 2, Qt::DashLine);
        ghost.setCosmetic(true);
        p->setClipRect(L.gridRect() & viewport()->rect());
        p->setPen(ghost);
        if (column) p->drawLine(m_breakAt.x(), 0, m_breakAt.x(), viewport()->height());
        else p->drawLine(0, m_breakAt.y(), viewport()->width(), m_breakAt.y());
    }
    p->restore();
}

} // namespace jp::grid
