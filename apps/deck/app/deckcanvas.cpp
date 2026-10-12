#include "app/deckcanvas.h"

#include "app/commentspane.h"
#include "app/editormedia.h"
#include "core/comments.h"
#include "core/deck.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <algorithm>

namespace jp {

DeckCanvas::DeckCanvas(Editor *ed, QWidget *parent) : Canvas(ed, parent) { m_media = new EditorMedia(this); }

bool DeckCanvas::viewportEvent(QEvent *e)
{
    if (e->type() == QEvent::Leave) m_media->leave();
    return Canvas::viewportEvent(e);
}

void DeckCanvas::setGuides(bool on)
{
    m_guides = on;
    viewport()->update();
}

void DeckCanvas::setGridlines(bool on)
{
    m_grid = on;
    viewport()->update();
}

void DeckCanvas::setCommentsShown(bool on)
{
    m_comments = on;
    m_dragId.clear();
    viewport()->update();
}

void DeckCanvas::setActiveComment(const QString &id)
{
    m_activeComment = id;
    viewport()->update();
}

// A speech bubble with its tail at the lower left, its corner at the comment's place.
QRectF DeckCanvas::commentMarker(const QPointF &page) const { return QRectF(pageToView(page), QSizeF(22, 21)); }

QString DeckCanvas::commentAt(const QPointF &view) const
{
    const Document *d = editor()->doc();
    const int page = editor()->currentPage();
    if (!m_comments || !d || !editor()->masterView().isEmpty() || page < 0 || page >= d->pages.size()) return QString();
    const QVector<deck::Comment> list = deck::commentsOf(*d->pages[page]);
    for (int i = list.size() - 1; i >= 0; --i)   // the one drawn last is on top
        if (commentMarker(list[i].id == m_dragId && m_dragMoved ? m_dragAt : list[i].at).contains(view)) return list[i].id;
    return QString();
}

void DeckCanvas::mousePressEvent(QMouseEvent *e)
{
    if (m_media->press(e)) return;
    const QString id = e->button() == Qt::LeftButton ? commentAt(e->position()) : QString();
    if (id.isEmpty()) {
        Canvas::mousePressEvent(e);
        return;
    }
    const std::optional<deck::Comment> c = deck::findComment(*editor()->doc()->pages[editor()->currentPage()], id);
    m_dragId = id;
    m_dragMoved = false;
    m_dragFrom = e->position();
    m_dragOffset = c ? toPage(e->position()) - c->at : QPointF();
    m_activeComment = id;
    Q_EMIT commentClicked(id);
    viewport()->update();
    e->accept();
}

void DeckCanvas::mouseMoveEvent(QMouseEvent *e)
{
    if (m_media->move(e)) return;
    if (m_dragId.isEmpty()) {
        Canvas::mouseMoveEvent(e);
        return;
    }
    if (!m_dragMoved && (e->position() - m_dragFrom).manhattanLength() > 3) m_dragMoved = true;
    if (m_dragMoved) {
        const QSizeF size = editor()->doc()->pageSize();
        const QPointF at = toPage(e->position()) - m_dragOffset;
        m_dragAt = QPointF(std::clamp(at.x(), 0.0, size.width() - 10), std::clamp(at.y(), 0.0, size.height() - 10));
        viewport()->update();
    }
    e->accept();
}

void DeckCanvas::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_media->release(e)) return;
    if (m_dragId.isEmpty()) {
        Canvas::mouseReleaseEvent(e);
        return;
    }
    const QString id = m_dragId;
    const bool moved = m_dragMoved;
    m_dragId.clear();
    m_dragMoved = false;
    if (moved) {
        editor()->beginChange(tr("Move Comment"));
        deck::moveComment(*editor()->doc()->pages[editor()->currentPage()], id, m_dragAt);
        editor()->endChange();
    }
    viewport()->update();
    e->accept();
}

void DeckCanvas::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape && !m_dragId.isEmpty()) {
        m_dragId.clear();
        m_dragMoved = false;
        viewport()->update();
        return;
    }
    Canvas::keyPressEvent(e);
}

void DeckCanvas::paintEvent(QPaintEvent *e)
{
    Canvas::paintEvent(e);
    const Document *d = editor()->doc();
    const int page = editor()->currentPage();
    if (!d || !editor()->masterView().isEmpty() || page < 0 || page >= d->pages.size()) return;
    QPainter p(viewport());
    const QSizeF ps = d->pageSize();
    const QRectF slide(pageToView(QPointF(0, 0)), pageToView(QPointF(ps.width(), ps.height())));
    if (m_grid) {
        // A twelfth of an inch, or a multiple of it where that would crowd.
        double step = 6;
        while (step * ppp() < 7) step *= 2;
        p.setPen(QColor(0x80, 0x84, 0x8C, 150));
        for (double y = step; y < ps.height(); y += step)
            for (double x = step; x < ps.width(); x += step) p.drawPoint(pageToView(QPointF(x, y)).toPoint());
    }
    if (m_guides) {
        QPen dash(QColor(0x80, 0x84, 0x8C), 1, Qt::DashLine);
        p.setPen(dash);
        const QPointF mid = slide.center();
        p.drawLine(QPointF(mid.x(), slide.top()), QPointF(mid.x(), slide.bottom()));
        p.drawLine(QPointF(slide.left(), mid.y()), QPointF(slide.right(), mid.y()));
    }
    // Placeholders: dotted while empty, selected, or typed in; a text box only while typed in. Other objects show no outline.
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor(0x80, 0x84, 0x8C), 1, Qt::DotLine));
    p.setBrush(Qt::NoBrush);
    const QStringList selected = editor()->selection();
    for (const auto &it : d->pages[page]->items) {
        if (it->type() != ItemType::Text) continue;
        const auto *t = static_cast<const TextItem *>(it.get());
        const QTextDocument *story = d->storyDoc(t->storyId);
        const bool typing = editor()->isEditingText() && editor()->textTarget().itemId == t->id;
        const bool placeholder = !deck::placeholderType(*it).isEmpty();
        if (!(typing || (placeholder && (selected.contains(t->id) || !story || story->isEmpty())))) continue;
        QPolygonF poly;
        for (const QPointF &pt : t->outline()) poly << pageToView(pt);
        p.drawPolygon(poly);
    }
    m_media->paint(&p);
    if (!m_comments) return;
    QFont initials = p.font();
    initials.setPixelSize(8);
    initials.setBold(true);
    for (const deck::Comment &c : deck::commentsOf(*d->pages[page])) {
        const QRectF r = commentMarker(c.id == m_dragId && m_dragMoved ? m_dragAt : c.at);
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
}

} // namespace jp
