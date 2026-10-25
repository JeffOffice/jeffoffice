// The grid view's marks (cells a search found, tinted) and the buttons put on cells (a filter's drop-downs).

#include "grid/gridview.h"

#include "app/icons.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPolygonF>

#include <algorithm>

namespace jp::grid {

void GridView::setMarks(const std::vector<GridRange> &ranges)
{
    if (ranges.empty() && m_marks.empty()) return;
    m_marks = ranges;
    if (m_marks.size() > 10000) m_marks.resize(10000);
    viewport()->update();
}

void GridView::setHeaderButtons(std::vector<HeaderButton> buttons)
{
    if (buttons.size() > kMaxHeaderButtons) buttons.resize(kMaxHeaderButtons);
    m_headerButtons = std::move(buttons);
    viewport()->update();
}

QRect GridView::headerButtonRectInLayout(qint64 row, qint64 col) const
{
    const QRect cell = cellRectInLayout(row, col);
    if (cell.isEmpty() || cell.width() < 22 || cell.height() < 12) return QRect();
    const int size = std::min(cell.height() - 2, 17);
    return QRect(cell.right() - size, cell.bottom() - size, size, size);
}

QRect GridView::headerButtonRect(qint64 row, qint64 col) const { return mirror(headerButtonRectInLayout(row, col)); }

bool GridView::headerButtonPress(const QPoint &layoutPoint)
{
    for (const HeaderButton &b : m_headerButtons) {
        if (!headerButtonRectInLayout(b.row, b.col).contains(layoutPoint)) continue;
        Q_EMIT headerButtonClicked(b.row, b.col);
        return true;
    }
    return false;
}

void GridView::paintMarks(QPainter *painter, const QRect &clip)
{
    if (m_marks.empty() && m_headerButtons.empty()) return;
    const GridLayout &L = m_sheet.layout();
    const QRect inLayout = mirror(clip);
    painter->save();
    if (mirrored()) {
        painter->translate(viewport()->width(), 0);
        painter->scale(-1, 1);
    }
    QColor tint = uiAccent();
    tint.setAlpha(uiDark() ? 70 : 45);
    for (const GridRange &r : m_marks) {
        for (Pane p : {Pane::Corner, Pane::Top, Pane::Left, Pane::Body}) {
            const PaneView v = L.pane(p);
            if (v.isEmpty() || r.row2 < v.firstRow || r.row1 > v.lastRow || r.col2 < v.firstCol || r.col1 > v.lastCol) continue;
            const QRect rect = L.rangeRectIn(p, std::max(r.row1, v.firstRow), std::max(r.col1, v.firstCol), std::min(r.row2, v.lastRow), std::min(r.col2, v.lastCol)) & v.rect & inLayout;
            if (!rect.isEmpty()) painter->fillRect(rect, tint);
        }
    }
    painter->setRenderHint(QPainter::Antialiasing, true);
    // (a button is looked at only if its cell is in a part of the grid that shows: there may be thousands of them, and a paint should cost the ones in view)
    PaneView panes[4];
    int pane = 0;
    for (Pane p : {Pane::Corner, Pane::Top, Pane::Left, Pane::Body}) panes[pane++] = L.pane(p);
    auto inView = [&](qint64 row, qint64 col) {
        for (const PaneView &v : panes)
            if (!v.isEmpty() && row >= v.firstRow && row <= v.lastRow && col >= v.firstCol && col <= v.lastCol) return true;
        return false;
    };
    for (const HeaderButton &b : m_headerButtons) {
        if (!inView(b.row, b.col)) continue;
        const QRect r = headerButtonRectInLayout(b.row, b.col);
        if (r.isEmpty() || !r.intersects(inLayout)) continue;
        painter->setPen(QPen(uiLine(), 1));
        painter->setBrush(b.filtered ? uiAccent().lighter(uiDark() ? 100 : 170) : uiPanel());
        painter->drawRect(r.adjusted(0, 0, -1, -1));
        painter->setPen(Qt::NoPen);
        painter->setBrush(b.filtered ? uiAccent().darker(uiDark() ? 100 : 130) : uiText());
        const QPointF c = QRectF(r).center();
        if (b.filtered) {
            // a funnel
            const QPolygonF funnel({QPointF(c.x() - 4.5, c.y() - 3.5), QPointF(c.x() + 4.5, c.y() - 3.5), QPointF(c.x() + 1, c.y() + 0.5), QPointF(c.x() + 1, c.y() + 4), QPointF(c.x() - 1, c.y() + 3),
                                    QPointF(c.x() - 1, c.y() + 0.5)});
            painter->drawPolygon(funnel);
        } else if (b.sort != 0) {
            const double up = b.sort > 0 ? -1 : 1;
            const QPolygonF arrow({QPointF(c.x() - 4, c.y() + up * 2), QPointF(c.x() + 4, c.y() + up * 2), QPointF(c.x(), c.y() - up * 3)});
            painter->drawPolygon(arrow);
        } else {
            const QPolygonF arrow({QPointF(c.x() - 4, c.y() - 2), QPointF(c.x() + 4, c.y() - 2), QPointF(c.x(), c.y() + 3)});
            painter->drawPolygon(arrow);
        }
    }
    painter->restore();
}

} // namespace jp::grid
