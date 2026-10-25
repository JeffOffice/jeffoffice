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
    const QPointer<GridView> self(this);
    m_overlay->setRepaintHook([self](const QRect &area) {
        if (!self) return;
        if (area.isEmpty()) self->viewport()->update();
        else self->viewport()->update(self->mirror(area));   // (the overlay speaks in the layout's places)
    });
    viewport()->update();
}

void GridView::setCellsEnabled(bool enabled)
{
    if (m_cellsEnabled == enabled) return;
    m_cellsEnabled = enabled;
    viewport()->update();
}

void GridView::paintOverlay(QPainter *painter, const QRect &clip)
{
    if (!m_overlay) return;
    const GridLayout &L = m_sheet.layout();
    // (a sheet that reads right to left is laid out as ever and drawn mirrored: the overlay draws in the layout's places, through a mirrored painter, and turns its pictures back)
    const QRect inLayout = mirror(clip);
    m_overlay->setMirrored(mirrored());
    painter->save();
    if (mirrored()) {
        painter->translate(viewport()->width(), 0);
        painter->scale(-1, 1);
    }
    for (const Pane pane : {Pane::Corner, Pane::Top, Pane::Left, Pane::Body}) {
        const QRect area = L.pane(pane).rect.intersected(inLayout);
        if (!area.isEmpty()) m_overlay->paint(painter, L, pane, area);
    }
    painter->restore();
}

bool GridView::overlayPress(QMouseEvent *e)
{
    if (!m_overlay || e->button() != Qt::LeftButton) return false;
    return m_overlay->press(mirror(e->pos()), m_sheet.layout(), e->modifiers());   // (in the layout's places)
}

bool GridView::overlayMove(QMouseEvent *e)
{
    if (!m_overlay) return false;
    const QPoint lp = mirror(e->pos());
    if (m_overlay->mouseMove(lp, m_sheet.layout(), e->buttons(), e->modifiers())) return true;
    if (!(e->buttons() & Qt::LeftButton))
        if (const std::optional<Qt::CursorShape> shape = m_overlay->cursorAt(lp, m_sheet.layout())) {
            viewport()->setCursor(*shape);
            return true;
        }
    return false;
}

bool GridView::overlayRelease(QMouseEvent *e)
{
    if (!m_overlay || e->button() != Qt::LeftButton) return false;
    return m_overlay->mouseRelease(mirror(e->pos()), m_sheet.layout(), e->modifiers());
}

bool GridView::overlayContext(QMouseEvent *e)
{
    if (!m_overlay || e->button() != Qt::RightButton) return false;
    return m_overlay->contextMenu(mirror(e->pos()), e->globalPosition().toPoint(), m_sheet.layout());
}

bool GridView::overlayDoubleClick(QMouseEvent *e)
{
    if (!m_overlay || e->button() != Qt::LeftButton) return false;
    return m_overlay->doubleClick(mirror(e->pos()), m_sheet.layout());
}

} // namespace jp::grid
