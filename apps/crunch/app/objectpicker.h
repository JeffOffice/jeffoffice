#pragma once
// The objects of a sheet picked together. The object layer picks one object; this is the overlay the grid view has in its place, which gives the layer the mouse and keys except where a person
// picks more: Shift or Ctrl and a click adds an object to the picked ones (or takes it off), and, with Select Objects on, a drag over empty cells draws a box and picks the objects inside it.
// With one object picked the layer draws its handles; with more, each picked object gets a dashed frame here.

#include "grid/objectlayer.h"

#include <QObject>

#include <vector>

namespace jp {

class ObjectPicker : public QObject, public grid::GridOverlay {
    Q_OBJECT
public:
    explicit ObjectPicker(grid::ObjectLayer *layer, QObject *parent = nullptr);

    // The ids picked, in the order they were picked (one when the layer has one picked; none when the layer has none).
    const std::vector<int> &picked() const { return m_picked; }
    void setPicked(const std::vector<int> &ids);
    // Select Objects: a drag over cells with no object under it draws a box (and the cells do not see the mouse).
    void setBoxMode(bool on);
    bool boxMode() const { return m_boxMode; }

    void paint(QPainter *painter, const grid::GridLayout &layout, grid::Pane pane, const QRect &clip) override;
    bool press(const QPoint &point, const grid::GridLayout &layout, Qt::KeyboardModifiers modifiers) override;
    bool mouseMove(const QPoint &point, const grid::GridLayout &layout, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers) override;
    bool mouseRelease(const QPoint &point, const grid::GridLayout &layout, Qt::KeyboardModifiers modifiers) override;
    std::optional<Qt::CursorShape> cursorAt(const QPoint &point, const grid::GridLayout &layout) override;
    bool contextMenu(const QPoint &point, const QPoint &global, const grid::GridLayout &layout) override;
    bool doubleClick(const QPoint &point, const grid::GridLayout &layout) override;
    bool key(QKeyEvent *event) override;
    void layoutChanged() override;
    void setMirrored(bool mirrored) override;

Q_SIGNALS:
    void pickedChanged();
    // Delete was pressed with more than one object picked.
    void deleteRequested();

private:
    void changed(const std::vector<int> &ids);
    grid::ObjectLayer *m_layer;
    std::vector<int> m_picked;
    bool m_boxMode = false, m_syncing = false;
    bool m_dragging = false;
    QPoint m_from, m_to;
};

} // namespace jp
