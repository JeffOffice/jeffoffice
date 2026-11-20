#include "app/previewhandles.h"

#include <QCoreApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QWidget>

#include <algorithm>

namespace jp {

// One handle: a thin line with a wider place to take hold of it.
class PreviewHandle : public QWidget {
public:
    PreviewHandle(PreviewHandles *owner, QWidget *surface) : QWidget(surface), m_owner(owner)
    {
        setFocusPolicy(Qt::StrongFocus);
        setMouseTracking(false);
    }
    PreviewHandles::Spec spec;
    // Puts the handle at its place in the page.
    void follow()
    {
        const QRectF page = m_owner->m_page;
        const double kx = page.width() / std::max(1.0, m_owner->m_points.width()), ky = page.height() / std::max(1.0, m_owner->m_points.height());
        const int grip = 9;
        if (vertical()) {
            const double x = page.left() + spec.position * kx;
            setGeometry(int(x) - grip / 2, int(page.top() + spec.from * ky), grip, std::max(1, int((spec.to - spec.from) * ky)));
            setCursor(Qt::SizeHorCursor);
        } else {
            const double y = page.top() + spec.position * ky;
            setGeometry(int(page.left() + spec.from * kx), int(y) - grip / 2, std::max(1, int((spec.to - spec.from) * kx)), grip);
            setCursor(Qt::SizeVerCursor);
        }
    }
    bool vertical() const { return spec.kind != PreviewHandles::Kind::MarginTop && spec.kind != PreviewHandles::Kind::MarginBottom; }
    bool isColumn() const { return spec.kind == PreviewHandles::Kind::ColumnRight; }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        QColor c = palette().color(QPalette::Highlight);
        const bool margin = !isColumn();
        c.setAlpha(hasFocus() || m_dragging ? 255 : margin ? 150 : 110);
        QPen pen(c, hasFocus() || m_dragging ? 2 : 1, margin ? Qt::DashLine : Qt::SolidLine);
        p.setPen(pen);
        if (vertical()) p.drawLine(width() / 2, 0, width() / 2, height());
        else p.drawLine(0, height() / 2, width(), height() / 2);
    }
    void mousePressEvent(QMouseEvent *e) override
    {
        if (e->button() != Qt::LeftButton) return;
        m_dragging = true;
        m_dragged = spec.position;
        // (where the pointer is on the screen, and where the line is in the preview: the line follows the pointer by what the pointer has moved, whatever the line's own place is by then)
        m_pressAt = vertical() ? e->globalPosition().x() : e->globalPosition().y();
        m_startPx = vertical() ? x() + width() / 2.0 : y() + height() / 2.0;
        setFocus(Qt::MouseFocusReason);
        raise();
        update();
    }
    void mouseMoveEvent(QMouseEvent *e) override
    {
        if (!m_dragging) return;
        setPositionPx(m_startPx + (vertical() ? e->globalPosition().x() : e->globalPosition().y()) - m_pressAt);
    }
    void mouseReleaseEvent(QMouseEvent *e) override
    {
        if (!m_dragging || e->button() != Qt::LeftButton) return;
        m_dragging = false;
        update();
        finish();
    }
    void keyPressEvent(QKeyEvent *e) override
    {
        const int back = vertical() ? Qt::Key_Left : Qt::Key_Up, forward = vertical() ? Qt::Key_Right : Qt::Key_Down;
        if (e->key() != back && e->key() != forward) {
            QWidget::keyPressEvent(e);
            return;
        }
        const double next = std::clamp(spec.position + (e->key() == forward ? 1 : -1) * PreviewHandles::kKeyStep, spec.lowest, spec.highest);
        if (next == spec.position) return;
        if (m_owner->moved) m_owner->moved(spec.kind, spec.column, next);
    }
    void focusInEvent(QFocusEvent *e) override
    {
        QWidget::focusInEvent(e);
        update();
    }
    void focusOutEvent(QFocusEvent *e) override
    {
        QWidget::focusOutEvent(e);
        update();
    }

private:
    // The line is at this pixel of the preview while it is dragged, kept inside the places it may be.
    void setPositionPx(double px)
    {
        const QRectF page = m_owner->m_page;
        const double k = vertical() ? page.width() / std::max(1.0, m_owner->m_points.width()) : page.height() / std::max(1.0, m_owner->m_points.height());
        const double points = std::clamp(((px - (vertical() ? page.left() : page.top())) / std::max(1e-6, k)), spec.lowest, spec.highest);
        m_dragged = points;
        const double at = (vertical() ? page.left() : page.top()) + points * k;
        if (vertical()) move(int(at) - width() / 2, y());
        else move(x(), int(at) - height() / 2);
    }
    void finish()
    {
        const double points = m_dragged;
        if (points == spec.position) return;
        if (m_owner->moved) m_owner->moved(spec.kind, spec.column, points);
    }
    PreviewHandles *m_owner;
    bool m_dragging = false;
    double m_pressAt = 0, m_startPx = 0, m_dragged = 0;
};

PreviewHandles::PreviewHandles(QWidget *surface) : QObject(surface), m_surface(surface) {}

int PreviewHandles::count() const { return int(m_handles.size()); }

void PreviewHandles::clear()
{
    for (PreviewHandle *h : m_handles) h->deleteLater();
    m_handles.clear();
}

void PreviewHandles::place(const QRectF &page, const QSizeF &points, const std::vector<Spec> &specs)
{
    m_page = page;
    m_points = points;
    std::vector<PreviewHandle *> kept;
    std::vector<PreviewHandle *> old = m_handles;
    for (const Spec &s : specs) {
        PreviewHandle *h = nullptr;
        for (auto it = old.begin(); it != old.end(); ++it)
            if ((*it)->spec.kind == s.kind && (*it)->spec.column == s.column) {
                h = *it;
                old.erase(it);
                break;
            }
        if (!h) {
            h = new PreviewHandle(this, m_surface);
            h->show();
        }
        h->spec = s;
        h->setAccessibleName(s.name);
        h->setToolTip(s.name);
        h->setAccessibleDescription(QCoreApplication::translate("jp::PreviewHandles", "Drag, or press the arrow keys, to move it."));
        h->follow();
        h->raise();
        kept.push_back(h);
    }
    for (PreviewHandle *h : old) {
        h->setParent(nullptr);
        h->deleteLater();
    }
    m_handles = kept;
}

} // namespace jp
