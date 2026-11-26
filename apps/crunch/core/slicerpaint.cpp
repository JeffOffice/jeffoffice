#include "core/slicerpaint.h"

#include <QFont>
#include <QFontMetricsF>
#include <QPolygonF>

#include <algorithm>
#include <cmath>

namespace jp::crunch {

namespace {

const QColor kFrame(0xA6, 0xA6, 0xA6), kFace(Qt::white), kInk(0x26, 0x26, 0x26), kDim(0x9A, 0x9A, 0x9A), kOn(0x44, 0x72, 0xC4), kOnDim(0xB4, 0xC7, 0xE7), kOffEdge(0xD0, 0xD0, 0xD0);
constexpr double kHeader = 28, kPad = 4, kClearSide = 22, kDefaultButton = 24;

QFont fontOf(bool bold)
{
    QFont f;
    f.setPixelSize(12);
    f.setBold(bold);
    return f;
}

double buttonHeightOf(const SlicerFace &face)
{
    if (face.rowHeight <= 0) return kDefaultButton;
    return std::clamp(double(face.rowHeight) / 12700.0 * 96.0 / 72.0, 12.0, 200.0);
}

void words(QPainter *p, const QRectF &r, const QString &text, const QColor &ink, bool bold, Qt::Alignment align)
{
    if (text.isEmpty() || r.width() < 2) return;
    p->save();
    p->setClipRect(r);
    p->setFont(fontOf(bold));
    p->setPen(ink);
    const QRectF inner = r.adjusted(5, 0, -5, 0);
    p->drawText(inner, int(align | Qt::AlignVCenter), QFontMetricsF(p->font()).elidedText(text, Qt::ElideRight, inner.width()));
    p->restore();
}

// The clear-filter mark: a funnel, with a cross over it when a filter is on.
void funnel(QPainter *p, const QRectF &r, bool on)
{
    const QColor ink = on ? kInk : kOffEdge;
    const QPointF c = r.center();
    const double s = std::min(r.width(), r.height()) / 2 - 4;
    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);
    p->setPen(Qt::NoPen);
    p->setBrush(ink);
    QPolygonF t;
    t << QPointF(c.x() - s, c.y() - s) << QPointF(c.x() + s, c.y() - s) << QPointF(c.x() + s / 4, c.y()) << QPointF(c.x() + s / 4, c.y() + s) << QPointF(c.x() - s / 4, c.y() + s * 0.7)
      << QPointF(c.x() - s / 4, c.y());
    p->drawPolygon(t);
    if (on) {
        p->setPen(QPen(QColor(0xC0, 0x39, 0x2B), 1.6));
        p->drawLine(QPointF(c.x() + s / 3, c.y() + s / 6), QPointF(c.x() + s, c.y() + s));
        p->drawLine(QPointF(c.x() + s, c.y() + s / 6), QPointF(c.x() + s / 3, c.y() + s));
    }
    p->restore();
}

} // namespace

QRectF SlicerGeometry::button(int index) const
{
    const int row = index / std::max(1, columns), col = index % std::max(1, columns);
    return QRectF(body.left() + col * (buttonWidth + gap), body.top() + row * (buttonHeight + gap), buttonWidth, buttonHeight);
}

SlicerGeometry slicerGeometry(const QSizeF &size, const SlicerFace &face)
{
    SlicerGeometry g;
    g.columns = std::clamp(face.columns, 1, 100);
    const double top = face.showCaption ? kHeader : 0;
    g.header = QRectF(1, 1, std::max(0.0, size.width() - 2), face.showCaption ? kHeader - 1 : 0);
    g.clear = face.showCaption ? QRectF(size.width() - 1 - kClearSide - 2, 1 + (kHeader - 1 - kClearSide) / 2, kClearSide, kClearSide) : QRectF();
    g.body = QRectF(1 + kPad, top + kPad, std::max(0.0, size.width() - 2 - 2 * kPad), std::max(0.0, size.height() - top - 1 - 2 * kPad));
    g.buttonHeight = buttonHeightOf(face);
    g.buttonWidth = std::max(1.0, (g.body.width() - g.gap * (g.columns - 1)) / g.columns);
    g.rows = g.body.height() < g.buttonHeight ? 0 : int((g.body.height() + g.gap) / (g.buttonHeight + g.gap));
    return g;
}

SlicerHit slicerHit(const SlicerGeometry &g, const SlicerFace &face, const QPointF &at)
{
    SlicerHit hit;
    if (!g.clear.isNull() && g.clear.contains(at)) {
        hit.part = SlicerPart::Clear;
        return hit;
    }
    if (g.header.contains(at)) {
        hit.part = SlicerPart::Header;
        return hit;
    }
    const int shown = int(std::min<qint64>({qint64(face.buttons.size()), qint64(g.capacity()), qint64(kMaxSlicerButtons)}));
    if (g.columns <= 0 || !g.body.contains(at)) return hit;
    const int col = int((at.x() - g.body.left()) / (g.buttonWidth + g.gap)), row = int((at.y() - g.body.top()) / (g.buttonHeight + g.gap));
    const int index = row * g.columns + col;
    if (col < 0 || col >= g.columns || row < 0 || index >= shown) return hit;
    if (g.button(index).contains(at)) {
        hit.part = SlicerPart::Button;
        hit.index = index;
    }
    return hit;
}

void paintSlicer(QPainter *p, const QRectF &rect, const SlicerFace &face, bool usable)
{
    p->save();
    p->translate(rect.topLeft());
    p->setClipRect(QRectF(QPointF(0, 0), rect.size()));
    p->setPen(QPen(kFrame, 1));
    p->setBrush(kFace);
    p->drawRect(QRectF(0.5, 0.5, rect.width() - 1, rect.height() - 1));
    const SlicerGeometry g = slicerGeometry(rect.size(), face);
    if (face.showCaption) {
        words(p, g.header.adjusted(0, 0, -kClearSide - 4, 0), face.caption, kInk, true, Qt::AlignLeft);
        if (usable) funnel(p, g.clear, face.filtered);
        p->setPen(QPen(kOffEdge, 1));
        p->drawLine(QPointF(1, g.header.bottom()), QPointF(rect.width() - 1, g.header.bottom()));
    }
    const int shown = int(std::min<qint64>({qint64(face.buttons.size()), qint64(g.capacity()), qint64(kMaxSlicerButtons)}));
    for (int i = 0; i < shown; ++i) {
        const SlicerButton &b = face.buttons[size_t(i)];
        const QRectF r = g.button(i);
        p->setPen(QPen(b.selected ? kOn : kOffEdge, 1));
        p->setBrush(b.selected ? (b.noData ? kOnDim : kOn) : kFace);
        p->drawRect(r.adjusted(0.5, 0.5, -0.5, -0.5));
        words(p, r, b.text, b.selected ? Qt::white : (b.noData ? kDim : kInk), false, Qt::AlignCenter);
    }
    p->restore();
}

} // namespace jp::crunch
