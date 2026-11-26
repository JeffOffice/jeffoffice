#include "core/slicerpaint.h"

#include "app/icons.h"

#include <QCoreApplication>
#include <QFont>
#include <QFontMetricsF>
#include <QPolygonF>

#include <algorithm>
#include <cmath>

namespace jp::crunch {

namespace {

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
void funnel(QPainter *p, const QRectF &r, bool on, const SlicerColors &col)
{
    const QColor ink = on ? col.ink : col.edge;
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
    if (face.unusable != SlicerWhy::None) return g;
    const qint64 total = std::max<qint64>(face.itemCount, qint64(face.buttons.size()));
    const qint64 made = std::min<qint64>(qint64(face.buttons.size()), kMaxSlicerButtons);
    if (total <= std::min<qint64>(g.capacity(), made)) {
        g.shown = int(total);
        return g;
    }
    // (some buttons are left out: the last row that fits tells how many)
    const int rowsOfButtons = std::max(0, g.rows - 1);
    g.shown = int(std::min<qint64>(made, qint64(rowsOfButtons) * g.columns));
    const int moreRow = (g.shown + g.columns - 1) / g.columns;
    if (g.rows > 0 || g.body.height() >= 14)
        g.moreLine = QRectF(g.body.left(), g.body.top() + moreRow * (g.buttonHeight + g.gap), g.body.width(), std::min(g.buttonHeight, g.body.height()));
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
    if (!g.moreLine.isNull() && g.moreLine.contains(at)) {
        hit.part = SlicerPart::More;
        return hit;
    }
    const int shown = g.shown;
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

QString slicerMoreText(const SlicerFace &face, const SlicerGeometry &g)
{
    const qint64 total = std::max<qint64>(face.itemCount, qint64(face.buttons.size()));
    const int left = int(std::min<qint64>(total - g.shown, 1000000000));
    qint64 chosen = 0;
    if (face.filtered) {
        qint64 inView = 0;
        for (int i = 0; i < g.shown && i < int(face.buttons.size()); ++i) inView += face.buttons[size_t(i)].selected ? 1 : 0;
        chosen = std::max<qint64>(0, face.chosenCount - inView);
    }
    if (chosen > 0) return QCoreApplication::translate("jp::crunch::Slicer", "More items: %1 (%2 chosen)").arg(left).arg(chosen);
    return QCoreApplication::translate("jp::crunch::Slicer", "More items: %1").arg(left);
}

QString slicerUnusableText(SlicerWhy why)
{
    if (why == SlicerWhy::Table) return QCoreApplication::translate("jp::crunch::Slicer", "This slicer filters a table; JeffCrunch shows it but cannot use it yet.");
    if (why == SlicerWhy::Missing) return QCoreApplication::translate("jp::crunch::Slicer", "What this slicer filters is not in the file, so JeffCrunch shows it but cannot use it.");
    return QString();
}

SlicerColors slicerColors(bool dark)
{
    SlicerColors c;
    if (dark) {
        c.frame = QColor(0x6A, 0x6A, 0x6A);
        c.face = QColor(0x2B, 0x2B, 0x2B);
        c.ink = QColor(0xF0, 0xF0, 0xF0);
        c.edge = QColor(0x55, 0x55, 0x55);
        c.on = QColor(0x3D, 0x6B, 0xC4);
        c.onInk = Qt::white;
        c.onDim = QColor(0x2E, 0x3F, 0x66);
        c.onDimInk = QColor(0xE4, 0xEB, 0xFA);
        c.offDim = QColor(0x22, 0x22, 0x22);
        c.offDimInk = QColor(0xA8, 0xA8, 0xA8);
    } else {
        c.frame = QColor(0xA6, 0xA6, 0xA6);
        c.face = Qt::white;
        c.ink = QColor(0x26, 0x26, 0x26);
        c.edge = QColor(0xD0, 0xD0, 0xD0);
        c.on = QColor(0x2F, 0x55, 0x97);
        c.onInk = Qt::white;
        c.onDim = QColor(0xC9, 0xD7, 0xEE);
        c.onDimInk = QColor(0x1F, 0x2D, 0x4A);
        c.offDim = QColor(0xED, 0xED, 0xED);
        c.offDimInk = QColor(0x59, 0x59, 0x59);
    }
    c.dim = c.offDimInk;
    c.rule = c.edge;
    return c;
}

void paintSlicer(QPainter *p, const QRectF &rect, const SlicerFace &face)
{
    const SlicerColors c = slicerColors(uiDark());
    const bool usable = face.unusable == SlicerWhy::None;
    p->save();
    p->translate(rect.topLeft());
    p->setClipRect(QRectF(QPointF(0, 0), rect.size()));
    p->setPen(QPen(c.frame, 1));
    p->setBrush(c.face);
    p->drawRect(QRectF(0.5, 0.5, rect.width() - 1, rect.height() - 1));
    const SlicerGeometry g = slicerGeometry(rect.size(), face);
    if (face.showCaption) {
        words(p, g.header.adjusted(0, 0, -kClearSide - 4, 0), face.caption, c.ink, true, Qt::AlignLeft);
        if (usable) funnel(p, g.clear, face.filtered, c);
        p->setPen(QPen(c.rule, 1));
        p->drawLine(QPointF(1, g.header.bottom()), QPointF(rect.width() - 1, g.header.bottom()));
    }
    if (!usable) {
        // (the sentence wraps in the body, and is cut off at its bottom)
        p->setClipRect(g.body);
        p->setFont(fontOf(false));
        p->setPen(c.ink);
        p->drawText(g.body.adjusted(2, 0, -2, 0), int(Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap), slicerUnusableText(face.unusable));
        p->restore();
        return;
    }
    for (int i = 0; i < g.shown && i < int(face.buttons.size()); ++i) {
        const SlicerButton &b = face.buttons[size_t(i)];
        const QRectF r = g.button(i);
        const QColor fill = b.selected ? (b.noData ? c.onDim : c.on) : (b.noData ? c.offDim : c.face);
        const QColor ink = b.selected ? (b.noData ? c.onDimInk : c.onInk) : (b.noData ? c.offDimInk : c.ink);
        p->setPen(QPen(b.selected ? c.on : c.edge, 1));
        p->setBrush(fill);
        p->drawRect(r.adjusted(0.5, 0.5, -0.5, -0.5));
        words(p, r, b.text, ink, false, Qt::AlignCenter);
    }
    if (!g.moreLine.isNull()) words(p, g.moreLine, slicerMoreText(face, g), c.ink, false, Qt::AlignLeft);
    p->restore();
}

} // namespace jp::crunch
