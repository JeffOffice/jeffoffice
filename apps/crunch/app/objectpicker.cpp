#include "app/objectpicker.h"

#include "grid/layout.h"

#include <QPainter>
#include <QPen>

#include <algorithm>

namespace jp {

ObjectPicker::ObjectPicker(grid::ObjectLayer *layer, QObject *parent) : QObject(parent), m_layer(layer)
{
    connect(layer, &grid::ObjectLayer::selectionChanged, this, [this](int id) {
        if (m_syncing) return;
        changed(id >= 0 ? std::vector<int>{id} : std::vector<int>());
    });
}

void ObjectPicker::changed(const std::vector<int> &ids)
{
    if (ids == m_picked) return;
    m_picked = ids;
    Q_EMIT pickedChanged();
    requestRepaint();
}

void ObjectPicker::setPicked(const std::vector<int> &ids)
{
    std::vector<int> clean;
    for (const int id : ids)
        if (id >= 0 && !m_layer->isDeleted(id) && std::find(clean.begin(), clean.end(), id) == clean.end()) clean.push_back(id);
    m_syncing = true;
    m_layer->select(clean.size() == 1 ? clean.front() : -1);
    m_syncing = false;
    changed(clean);
}

void ObjectPicker::setBoxMode(bool on)
{
    m_boxMode = on;
    if (!on && m_dragging) {
        m_dragging = false;
        requestRepaint();
    }
}

void ObjectPicker::paint(QPainter *painter, const grid::GridLayout &layout, grid::Pane pane, const QRect &clip)
{
    m_layer->paint(painter, layout, pane, clip);
    if (m_picked.size() > 1) {
        painter->save();
        QPen pen(QColor(0x2E, 0x75, 0xB6), 1, Qt::DashLine);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        for (const int id : m_picked) {
            const QRect r = m_layer->deviceRect(id, pane);
            if (!r.isEmpty()) painter->drawRect(r.adjusted(-2, -2, 2, 2));
        }
        painter->restore();
    }
    if (m_dragging && pane == grid::Pane::Body) {
        painter->save();
        painter->setPen(QPen(QColor(0x2E, 0x75, 0xB6), 1, Qt::DotLine));
        painter->setBrush(QColor(0x2E, 0x75, 0xB6, 40));
        painter->drawRect(QRect(m_from, m_to).normalized());
        painter->restore();
    }
}

bool ObjectPicker::press(const QPoint &point, const grid::GridLayout &layout, Qt::KeyboardModifiers modifiers)
{
    // (a handle of the one object picked is the layer's: Shift on a corner keeps the proportions)
    if ((modifiers & (Qt::ShiftModifier | Qt::ControlModifier)) && m_layer->handleAt(point) < 0) {
        const int hit = m_layer->objectAt(point);
        if (hit >= 0) {
            std::vector<int> ids = m_picked;
            const auto at = std::find(ids.begin(), ids.end(), hit);
            if (at != ids.end()) ids.erase(at);
            else ids.push_back(hit);
            setPicked(ids);
            return true;
        }
    }
    if (m_boxMode && m_layer->objectAt(point) < 0 && m_layer->handleAt(point) < 0) {
        m_dragging = true;
        m_from = m_to = point;
        requestRepaint();
        return true;
    }
    return m_layer->press(point, layout, modifiers);
}

bool ObjectPicker::mouseMove(const QPoint &point, const grid::GridLayout &layout, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
    if (m_dragging) {
        m_to = point;
        requestRepaint();
        return true;
    }
    return m_layer->mouseMove(point, layout, buttons, modifiers);
}

bool ObjectPicker::mouseRelease(const QPoint &point, const grid::GridLayout &layout, Qt::KeyboardModifiers modifiers)
{
    if (m_dragging) {
        m_dragging = false;
        m_to = point;
        const QRect box = QRect(m_from, m_to).normalized();
        std::vector<int> ids;
        // (the objects wholly inside the box, back to front)
        for (int i = 0; i < m_layer->count(); ++i) {
            const int id = m_layer->object(i).id;
            if (m_layer->isDeleted(id)) continue;
            const QRect r = m_layer->deviceRect(id, grid::Pane::Body);
            if (!r.isEmpty() && box.contains(r)) ids.push_back(id);
        }
        setPicked(ids);
        requestRepaint();
        return true;
    }
    return m_layer->mouseRelease(point, layout, modifiers);
}

std::optional<Qt::CursorShape> ObjectPicker::cursorAt(const QPoint &point, const grid::GridLayout &layout)
{
    if (m_boxMode && m_layer->objectAt(point) < 0) return Qt::CrossCursor;
    return m_layer->cursorAt(point, layout);
}

bool ObjectPicker::contextMenu(const QPoint &point, const QPoint &global, const grid::GridLayout &layout) { return m_layer->contextMenu(point, global, layout); }
bool ObjectPicker::doubleClick(const QPoint &point, const grid::GridLayout &layout) { return m_layer->doubleClick(point, layout); }

bool ObjectPicker::key(QKeyEvent *event)
{
    if (m_picked.size() > 1 && event->key() == Qt::Key_Delete) {
        Q_EMIT deleteRequested();
        return true;
    }
    if (m_dragging && event->key() == Qt::Key_Escape) {
        m_dragging = false;
        requestRepaint();
        return true;
    }
    return m_layer->key(event);
}

void ObjectPicker::layoutChanged() { m_layer->layoutChanged(); }
void ObjectPicker::setMirrored(bool mirrored) { m_layer->setMirrored(mirrored); }

} // namespace jp
