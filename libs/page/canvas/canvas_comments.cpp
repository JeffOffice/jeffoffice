// The comment markers on the canvas: drawn on the page being edited, and clicked or dragged there.

#include "app/commentspane.h"
#include "canvas/canvas.h"
#include "core/comments.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <algorithm>

namespace jp {

void Canvas::setCommentsShown(bool on)
{
    m_comments = on;
    m_commentDragId.clear();
    viewport()->update();
}

void Canvas::setActiveComment(const QString &id)
{
    m_activeComment = id;
    viewport()->update();
}

// A speech bubble with its tail at the lower left, its corner at the comment's place.
QRectF Canvas::commentMarker(const QPointF &page) const { return QRectF(pageToView(page), QSizeF(22, 21)); }

QString Canvas::commentAt(const QPointF &view) const
{
    const Document *d = m_ed->doc();
    const int page = m_ed->currentPage();
    if (!m_comments || !d || !m_ed->masterView().isEmpty() || page < 0 || page >= d->pages.size()) return QString();
    const QVector<comments::Comment> list = comments::commentsOf(*d->pages[page]);
    for (int i = list.size() - 1; i >= 0; --i)   // the one drawn last is on top
        if (commentMarker(list[i].id == m_commentDragId && m_commentDragMoved ? m_commentDragAt : comments::commentPlace(*d, list[i])).contains(view)) return list[i].id;
    return QString();
}

bool Canvas::commentPress(QMouseEvent *e)
{
    const QString id = e->button() == Qt::LeftButton ? commentAt(e->position()) : QString();
    if (id.isEmpty()) return false;
    const std::optional<comments::Comment> c = comments::findComment(*m_ed->doc()->pages[m_ed->currentPage()], id);
    m_commentDragId = id;
    m_commentDragMoved = false;
    m_commentDragFrom = e->position();
    m_commentDragOffset = c ? toPage(e->position()) - comments::commentPlace(*m_ed->doc(), *c) : QPointF();
    m_activeComment = id;
    Q_EMIT commentClicked(id);
    viewport()->update();
    e->accept();
    return true;
}

bool Canvas::commentMove(QMouseEvent *e)
{
    if (m_commentDragId.isEmpty()) return false;
    if (!m_commentDragMoved && (e->position() - m_commentDragFrom).manhattanLength() > 3) m_commentDragMoved = true;
    if (m_commentDragMoved) {
        const QSizeF size = m_ed->doc()->pageSize();
        const QPointF at = toPage(e->position()) - m_commentDragOffset;
        m_commentDragAt = QPointF(std::clamp(at.x(), 0.0, size.width() - 10), std::clamp(at.y(), 0.0, size.height() - 10));
        viewport()->update();
    }
    e->accept();
    return true;
}

bool Canvas::commentRelease(QMouseEvent *e)
{
    if (m_commentDragId.isEmpty()) return false;
    const QString id = m_commentDragId;
    const bool moved = m_commentDragMoved;
    m_commentDragId.clear();
    m_commentDragMoved = false;
    // (a comment attached to an object goes where the object goes; it is not dragged off it, and none is moved in a publication open for reading only)
    const std::optional<comments::Comment> c = comments::findComment(*m_ed->doc()->pages[m_ed->currentPage()], id);
    if (moved && c && c->itemId.isEmpty() && !m_ed->readOnly()) {
        m_ed->beginChange(tr("Move Comment"));
        comments::moveComment(*m_ed->doc()->pages[m_ed->currentPage()], id, m_commentDragAt);
        m_ed->endChange();
    }
    viewport()->update();
    e->accept();
    return true;
}

void Canvas::paintComments(QPainter &p)
{
    const Document *d = m_ed->doc();
    const int page = m_ed->currentPage();
    if (!m_comments || !d || !m_ed->masterView().isEmpty() || page < 0 || page >= d->pages.size()) return;
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    QFont initials = p.font();
    initials.setPixelSize(8);
    initials.setBold(true);
    for (const comments::Comment &c : comments::commentsOf(*d->pages[page])) {
        const bool dragged = c.id == m_commentDragId && m_commentDragMoved && c.itemId.isEmpty();
        const QRectF r = commentMarker(dragged ? m_commentDragAt : comments::commentPlace(*d, c));
        const QRectF bubble(r.topLeft(), QSizeF(22, 16));
        QPainterPath shape;
        shape.addRoundedRect(bubble, 4, 4);
        QPainterPath tail;
        tail.addPolygon(QPolygonF({r.topLeft() + QPointF(4, 14), r.topLeft() + QPointF(11, 14), r.topLeft() + QPointF(4, 21)}));
        shape = shape.united(tail);
        const bool active = c.id == m_activeComment;
        p.setPen(QPen(active ? QColor(0x20, 0x24, 0x2B) : Qt::white, active ? 2 : 1.5));
        p.setBrush(commentColor(c.author, c.done));
        p.drawPath(shape);
        p.setFont(initials);
        p.setPen(Qt::white);
        p.drawText(bubble, Qt::AlignCenter, c.initials.left(2));
    }
    p.restore();
}

} // namespace jp
