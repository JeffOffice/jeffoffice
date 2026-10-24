// The grid view's part in an overlay (grid/overlay.h): it is given each part of the grid to draw over, and the clicks that land on an object of it.

#include "grid/gridview.h"

#include <QMouseEvent>
#include <QPointer>

namespace jp::grid {

void GridView::setOverlay(GridOverlay *overlay)
{
    if (m_overlay) m_overlay->setRepaintHook(nullptr);
    m_overlay = overlay;
    if (!m_overlay) {
        viewport()->update();
        return;
    }
    // (the viewport goes with the view: a hook the overlay outlives does nothing)
    const QPointer<QWidget> vp = viewport();
    m_overlay->setRepaintHook([vp](const QRect &area) {
        if (!vp) return;
        if (area.isEmpty()) vp->update();
        else vp->update(area);
    });
    viewport()->update();
}

void GridView::paintOverlay(QPainter *painter, const QRect &clip)
{
    if (!m_overlay) return;
    const GridLayout &L = m_sheet.layout();
    for (const Pane pane : {Pane::Corner, Pane::Top, Pane::Left, Pane::Body}) {
        const QRect area = L.pane(pane).rect.intersected(clip);
        if (!area.isEmpty()) m_overlay->paint(painter, L, pane, area);
    }
}

bool GridView::overlayPress(QMouseEvent *e)
{
    if (!m_overlay || e->button() != Qt::LeftButton) return false;
    return m_overlay->press(e->pos(), m_sheet.layout(), e->modifiers());
}

} // namespace jp::grid
