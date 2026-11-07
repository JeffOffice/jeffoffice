// The split boxes of a grid: a small box at the top of the vertical scroll bar and at the left end of the horizontal one. Dragging one makes the bar that cuts the rows (or the columns), or moves it.
#include "grid/gridview.h"

#include <QBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>

#include <algorithm>

namespace jp::grid {

namespace {

constexpr int kBox = 8;   // the box's length along its scroll bar, in pixels

class SplitBox : public QWidget {
public:
    SplitBox(GridView *view, bool rows) : QWidget(view), m_view(view), m_rows(rows)
    {
        setObjectName(rows ? QStringLiteral("splitBoxRows") : QStringLiteral("splitBoxColumns"));
        setCursor(rows ? Qt::SplitVCursor : Qt::SplitHCursor);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.fillRect(rect(), palette().color(QPalette::Window).darker(112));
        p.setPen(palette().color(QPalette::Mid));
        p.drawRect(rect().adjusted(0, 0, -1, -1));
        // two lines across the box, as a grip
        p.setPen(palette().color(QPalette::Dark));
        if (m_rows) {
            p.drawLine(2, height() / 2 - 1, width() - 3, height() / 2 - 1);
            p.drawLine(2, height() / 2 + 1, width() - 3, height() / 2 + 1);
        } else {
            p.drawLine(width() / 2 - 1, 2, width() / 2 - 1, height() - 3);
            p.drawLine(width() / 2 + 1, 2, width() / 2 + 1, height() - 3);
        }
    }
    void mousePressEvent(QMouseEvent *e) override
    {
        if (e->button() != Qt::LeftButton) return;
        m_down = true;
        m_view->splitBoxPress(m_rows);
    }
    void mouseMoveEvent(QMouseEvent *e) override
    {
        if (m_down) m_view->splitBoxMove(m_rows, m_view->viewport()->mapFromGlobal(e->globalPosition().toPoint()));
    }
    void mouseReleaseEvent(QMouseEvent *e) override
    {
        if (e->button() != Qt::LeftButton || !m_down) return;
        m_down = false;
        m_view->splitBoxRelease();
    }

private:
    GridView *m_view;
    bool m_rows, m_down = false;
};

} // namespace

void GridView::setSplitBoxes(bool on)
{
    if (m_splitBoxes == on) return;
    if (on) {
        // (the area keeps each scroll bar in a container of its own with a box layout of one item: a box put in that layout takes the end of the bar, and goes with the bar when it is hidden)
        auto *rowsLayout = qobject_cast<QBoxLayout *>(verticalScrollBar()->parentWidget() ? verticalScrollBar()->parentWidget()->layout() : nullptr);
        auto *colsLayout = qobject_cast<QBoxLayout *>(horizontalScrollBar()->parentWidget() ? horizontalScrollBar()->parentWidget()->layout() : nullptr);
        if (!rowsLayout || !colsLayout) return;
        m_boxRows = new SplitBox(this, true);
        m_boxCols = new SplitBox(this, false);
        m_boxRows->setParent(verticalScrollBar()->parentWidget());
        m_boxCols->setParent(horizontalScrollBar()->parentWidget());
        m_boxRows->setFixedHeight(kBox);
        m_boxCols->setFixedWidth(kBox);
        m_boxRows->setToolTip(tr("Drag down to split the view into top and bottom parts"));
        m_boxCols->setToolTip(tr("Drag right to split the view into left and right parts"));
        rowsLayout->insertWidget(0, m_boxRows);
        colsLayout->insertWidget(0, m_boxCols);
        m_boxRows->show();
        m_boxCols->show();
    } else {
        delete m_boxRows;   // (a widget takes itself out of its layout)
        delete m_boxCols;
        m_boxRows = m_boxCols = nullptr;
    }
    m_splitBoxes = on;
}

void GridView::splitBoxPress(bool rows)
{
    if (!m_cellsEnabled || m_breakView) return;
    m_drag = Drag::SplitBar;
    m_splitDragRows = rows;
    m_splitDragCols = !rows;
}

void GridView::splitBoxMove(bool rows, const QPoint &viewportPos)
{
    if (m_drag != Drag::SplitBar) return;
    GridLayout &L = m_sheet.layout();
    if (!(rows ? L.splitHeight() : L.splitWidth())) {
        // no bar there yet: one is made at the second row (column) in view, and then follows the pointer (held rows and columns are let go first, as Split does)
        if (!L.isSplit() && (L.frozenRows() > L.skippedRows() || L.frozenCols() > L.skippedCols())) freezeAt(0, 0);
        const qint64 start = rows ? firstRowInView() : firstColInView();
        const qint64 at = rows ? L.rows.nextVisible(start) : L.cols.nextVisible(start);
        if (at < 0) return;
        constexpr qint64 kFar = 200000;
        const bool other = L.isSplit();   // (the bar the other box made stays)
        if (rows) {
            L.setSkipped(start, other ? L.skippedCols() : 0);
            L.setFrozen(at, other ? L.frozenCols() : 0);
            L.setSplit(L.splitWidth(), std::min(L.rows.position(at) - L.rows.position(start), kFar));
            L.setScroll(L.scrollX(), 0);
        } else {
            L.setSkipped(other ? L.skippedRows() : 0, start);
            L.setFrozen(other ? L.frozenRows() : 0, at);
            L.setSplit(std::min(L.cols.position(at) - L.cols.position(start), kFar), L.splitHeight());
            L.setScroll(0, L.scrollY());
        }
        layoutChanged();
    }
    moveSplitBar(mirror(viewportPos));
}

void GridView::splitBoxRelease()
{
    if (m_drag != Drag::SplitBar) return;
    m_drag = Drag::None;
    Q_EMIT splitChanged();
}

} // namespace jp::grid
