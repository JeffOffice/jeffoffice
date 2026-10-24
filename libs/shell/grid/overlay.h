#pragma once
// Something drawn over the cells of the grid, and what takes the mouse and keys before the cells do: the charts, pictures, and shapes of a sheet. The grid view gives it each part
// of the grid in turn (the frozen corner, the top, the left, and the body), with the part's rectangle and the clip that needs drawing, in the viewport's pixels; the overlay places
// what it holds by the layout, so it moves with the cells when they scroll and follows their sizes, the zoom, and the frozen panes.

#include <QKeyEvent>
#include <QPainter>
#include <QPoint>
#include <QRect>

#include <functional>

namespace jp::grid {

class GridLayout;
enum class Pane;

class GridOverlay {
public:
    virtual ~GridOverlay();
    // Draws over the cells of one part of the grid. The painter's clip is `clip` already (the part's rectangle with what needs drawing); it is left as it was.
    virtual void paint(QPainter *painter, const GridLayout &layout, Pane pane, const QRect &clip) = 0;
    // A mouse press at a point of the viewport, before the cells see it: true if the overlay has it (an object was under the point), false if the click goes on to the cells.
    virtual bool press(const QPoint &point, const GridLayout &layout, Qt::KeyboardModifiers modifiers)
    {
        Q_UNUSED(point);
        Q_UNUSED(layout);
        Q_UNUSED(modifiers);
        return false;
    }
    // A key before the grid's own: true if the overlay has it.
    virtual bool key(QKeyEvent *event)
    {
        Q_UNUSED(event);
        return false;
    }
    // The sizes of rows or columns, the frozen panes, or the sheet itself changed: what the overlay worked out from the layout is worked out again.
    virtual void layoutChanged() {}
    // How the overlay asks for part of the viewport to be drawn again (an empty rectangle: all of it); the view sets it.
    void setRepaintHook(std::function<void(const QRect &)> hook) { m_repaint = std::move(hook); }

protected:
    void requestRepaint(const QRect &area = QRect()) const
    {
        if (m_repaint) m_repaint(area);
    }

private:
    std::function<void(const QRect &)> m_repaint;
};

} // namespace jp::grid
