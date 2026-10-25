// The grid view's marks (cells a search found, tinted) and the buttons put on cells (a filter's drop-downs).

#include "grid/gridview.h"

#include "app/icons.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPolygonF>

#include <algorithm>
#include <cmath>

namespace jp::grid {

void GridView::setMarks(const std::vector<GridRange> &ranges)
{
    if (ranges.empty() && m_marks.empty()) return;
    m_marks = ranges;
    if (m_marks.size() > 10000) m_marks.resize(10000);
    viewport()->update();
}

void GridView::setCheckMarks(std::vector<GridRange> circles, std::vector<CheckArrow> arrows)
{
    if (circles.size() > kMaxCheckCircles) circles.resize(kMaxCheckCircles);
    if (arrows.size() > kMaxCheckArrows) arrows.resize(kMaxCheckArrows);
    if (circles.empty() && arrows.empty() && m_checkCircles.empty() && m_checkArrows.empty()) return;
    m_checkCircles = std::move(circles);
    m_checkArrows = std::move(arrows);
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
    if (m_marks.empty() && m_headerButtons.empty() && m_checkCircles.empty() && m_checkArrows.empty()) return;
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
    paintChecks(painter, inLayout);
    painter->restore();
}

// The visible part of a block, in the layout's pixels (empty when none of it shows).
static QRect visibleRangeRect(const GridLayout &L, const GridRange &r)
{
    QRect out;
    for (Pane p : {Pane::Corner, Pane::Top, Pane::Left, Pane::Body}) {
        const PaneView v = L.pane(p);
        if (v.isEmpty() || r.row2 < v.firstRow || r.row1 > v.lastRow || r.col2 < v.firstCol || r.col1 > v.lastCol) continue;
        out = out.united(L.rangeRectIn(p, std::max(r.row1, v.firstRow), std::max(r.col1, v.firstCol), std::min(r.row2, v.lastRow), std::min(r.col2, v.lastCol)) & v.rect);
    }
    return out;
}

// Circular cells outlined, and the arrows of a check of what formulas read. Called with the painter in the layout's pixels, antialiased.
void GridView::paintChecks(QPainter *painter, const QRect &clip)
{
    const GridLayout &L = m_sheet.layout();
    const bool dark = uiDark();
    const QColor red(dark ? 0xff : 0xcc, dark ? 0x6b : 0x22, dark ? 0x6b : 0x22);
    const QColor blue(dark ? 0x7d : 0x1f, dark ? 0xb0 : 0x5f, dark ? 0xff : 0xc8);
    const QColor ink = uiText();
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(red, 2, Qt::DashLine));
    for (const GridRange &r : m_checkCircles) {
        const QRect rect = visibleRangeRect(L, r);
        if (rect.isEmpty() || !rect.intersects(clip)) continue;
        painter->drawRect(QRectF(rect).adjusted(1, 1, -1, -1));
    }
    // the icon of a sheet: a page with a folded corner and a few lines
    auto drawSheetIcon = [&](const QRectF &r) {
        painter->save();
        painter->setPen(QPen(ink, 1));
        painter->setBrush(uiPanel());
        const QPolygonF page({r.topLeft(), QPointF(r.right() - 4, r.top()), QPointF(r.right(), r.top() + 4), r.bottomRight(), r.bottomLeft()});
        painter->drawPolygon(page);
        painter->drawLine(QPointF(r.right() - 4, r.top()), QPointF(r.right() - 4, r.top() + 4));
        painter->drawLine(QPointF(r.right() - 4, r.top() + 4), QPointF(r.right(), r.top() + 4));
        for (int i = 0; i < 3; ++i) painter->drawLine(QPointF(r.left() + 2, r.top() + 6 + i * 3), QPointF(r.right() - 2, r.top() + 6 + i * 3));
        painter->restore();
    };
    auto arrowHead = [&](const QPointF &tip, const QPointF &from, const QColor &color) {
        const QPointF d = tip - from;
        const double len = std::hypot(d.x(), d.y());
        if (len < 1) return;
        const QPointF u(d.x() / len, d.y() / len), n(-u.y(), u.x());
        painter->setPen(Qt::NoPen);
        painter->setBrush(color);
        painter->drawPolygon(QPolygonF({tip, tip - u * 9 + n * 4, tip - u * 9 - n * 4}));
    };
    for (const CheckArrow &a : m_checkArrows) {
        const QRect target = a.toOtherSheet ? QRect() : cellRectInLayout(a.toRow, a.toCol);
        const QRect source = a.fromOtherSheet ? QRect() : visibleRangeRect(L, a.from);
        QPointF p0, p1;
        bool dashed = false;
        QColor color = blue;
        if (a.fromOtherSheet) {
            // the icon is left of the cell it reads into
            if (target.isEmpty() || !target.adjusted(-40, -10, 10, 10).intersects(clip)) continue;
            const QRectF icon(std::max(2, target.left() - 30), target.center().y() - 8.0, 14, 16);
            drawSheetIcon(icon);
            p0 = icon.center() + QPointF(7, 0);
            p1 = QPointF(target.center());
            dashed = true;
            color = ink;
        } else if (a.toOtherSheet) {
            if (source.isEmpty() || !source.adjusted(-10, -10, 50, 10).intersects(clip)) continue;
            const QRectF icon(source.right() + 16.0, source.center().y() - 8.0, 14, 16);
            drawSheetIcon(icon);
            p0 = QPointF(source.center());
            p1 = icon.center() - QPointF(7, 0);
            dashed = true;
            color = ink;
        } else if (!source.isEmpty() && !target.isEmpty()) {
            if (!source.united(target).intersects(clip)) continue;
            // a block is outlined; the arrow starts at its first cell
            if (a.from.row1 != a.from.row2 || a.from.col1 != a.from.col2) {
                painter->setPen(QPen(color, 1.5));
                painter->setBrush(Qt::NoBrush);
                painter->drawRect(QRectF(source).adjusted(0.5, 0.5, -0.5, -0.5));
            }
            const QRect first = cellRectInLayout(a.from.row1, a.from.col1);
            p0 = QPointF((first.isEmpty() ? source : first).center());
            p1 = QPointF(target.center());
        } else if (!target.isEmpty() || !source.isEmpty()) {
            // one end is out of sight: a short arrow towards where the other is
            const QRect seen = target.isEmpty() ? source : target;
            if (!seen.intersects(clip)) continue;
            const qint64 otherRow = target.isEmpty() ? a.toRow : a.from.row1, otherCol = target.isEmpty() ? a.toCol : a.from.col1;
            const qint64 seenRow = target.isEmpty() ? a.from.row1 : a.toRow, seenCol = target.isEmpty() ? a.from.col1 : a.toCol;
            const QPointF c(seen.center());
            QPointF away(otherCol > seenCol ? 40 : otherCol < seenCol ? -40 : 0, otherRow > seenRow ? 40 : otherRow < seenRow ? -40 : 0);
            if (away.isNull()) continue;
            p0 = target.isEmpty() ? c + away : c;
            p1 = target.isEmpty() ? c : c + away;
        } else {
            continue;
        }
        painter->setPen(QPen(color, 1.5, dashed ? Qt::DashLine : Qt::SolidLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawLine(p0, p1);
        painter->setPen(Qt::NoPen);
        painter->setBrush(color);
        if (!a.fromOtherSheet) painter->drawEllipse(p0, 3.0, 3.0);
        arrowHead(p1, p0, color);
    }
}

} // namespace jp::grid
