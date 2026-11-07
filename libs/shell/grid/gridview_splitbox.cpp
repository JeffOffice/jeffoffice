// The split boxes of a grid: a small box at the top of the vertical scroll bar and at the left end of the horizontal one. Dragging one makes the bar that cuts the rows (or the columns), or moves it.
#include "grid/gridview.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
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
    m_splitBoxes = on;
    if (on) {
        m_boxRows = new SplitBox(this, true);
        m_boxCols = new SplitBox(this, false);
        m_boxRows->setToolTip(tr("Drag down to split the view into top and bottom parts"));
        m_boxCols->setToolTip(tr("Drag right to split the view into left and right parts"));
        m_boxRows->hide();
        m_boxCols->hide();
        verticalScrollBar()->installEventFilter(this);
        horizontalScrollBar()->installEventFilter(this);
        placeSplitBoxes();
    } else {
        verticalScrollBar()->removeEventFilter(this);
        horizontalScrollBar()->removeEventFilter(this);
        delete m_boxRows;
        delete m_boxCols;
        m_boxRows = m_boxCols = nullptr;
        // (the scroll bars take their whole length again)
        if (!m_fullRows.isNull() && verticalScrollBar()->geometry().top() == m_fullRows.top() + kBox) verticalScrollBar()->setGeometry(m_fullRows);
        if (!m_fullCols.isNull() && horizontalScrollBar()->geometry().width() == m_fullCols.width() - kBox) horizontalScrollBar()->setGeometry(m_fullCols);
        m_fullRows = m_fullCols = QRect();
    }
}

bool GridView::eventFilter(QObject *watched, QEvent *event)
{
    if (m_splitBoxes && (watched == verticalScrollBar() || watched == horizontalScrollBar())) {
        switch (event->type()) {
        case QEvent::Resize:
        case QEvent::Move:
        case QEvent::Show:
        case QEvent::Hide: placeSplitBoxes(); break;
        default: break;
        }
    }
    return QAbstractScrollArea::eventFilter(watched, event);
}

// Puts each box where its scroll bar begins and shortens the bar by the box. The area lays the bars out itself whenever one comes or goes or the view is resized, so this runs after each of
// those: a bar that is as this left it (it begins where its box ends) is let be.
void GridView::placeSplitBoxes()
{
    if (!m_splitBoxes || m_placingBoxes || !m_boxRows) return;
    m_placingBoxes = true;
    struct Done {
        bool *flag;
        ~Done() { *flag = false; }
    } done{&m_placingBoxes};
    QScrollBar *v = verticalScrollBar(), *h = horizontalScrollBar();
    const bool rtl = layoutDirection() == Qt::RightToLeft;
    {
        const QRect g = v->geometry();
        if (v->isHidden() || g.height() < 5 * kBox) {
            m_boxRows->hide();
        } else if (g.top() != m_boxRows->y() + kBox || g.left() != m_boxRows->x()) {
            m_fullRows = g;
            m_boxRows->setGeometry(g.left(), g.top(), g.width(), kBox);
            v->setGeometry(g.left(), g.top() + kBox, g.width(), g.height() - kBox);
            m_boxRows->show();
            m_boxRows->raise();
        }
    }
    {
        const QRect g = h->geometry();
        if (h->isHidden() || g.width() < 5 * kBox) {
            m_boxCols->hide();
        } else if (g.top() != m_boxCols->y() || (rtl ? g.right() != m_boxCols->geometry().left() - 1 : g.left() != m_boxCols->x() + kBox)) {
            m_fullCols = g;
            if (rtl) {
                m_boxCols->setGeometry(g.right() - kBox + 1, g.top(), kBox, g.height());
                h->setGeometry(g.left(), g.top(), g.width() - kBox, g.height());
            } else {
                m_boxCols->setGeometry(g.left(), g.top(), kBox, g.height());
                h->setGeometry(g.left() + kBox, g.top(), g.width() - kBox, g.height());
            }
            m_boxCols->show();
            m_boxCols->raise();
        }
    }
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
