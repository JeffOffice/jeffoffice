#include "app/deckcanvas.h"

#include "core/deck.h"

#include <QPainter>

namespace jp {

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
}

} // namespace jp
