#include "core/controlpaint.h"

#include <QFont>
#include <QFontMetricsF>
#include <QPolygonF>

#include <algorithm>
#include <cmath>

namespace jp::crunch {

using namespace jp::office::sheet;

namespace {

const QColor kEdge(0x70, 0x70, 0x70), kFace(0xE1, 0xE1, 0xE1), kFaceEdge(0xAD, 0xAD, 0xAD), kInk(0x1C, 0x1C, 0x1C), kField(Qt::white), kTrack(0xF0, 0xF0, 0xF0), kThumb(0xCD, 0xCD, 0xCD),
    kChosen(0x00, 0x78, 0xD7);

QFont fontOf(const FormControl &c)
{
    QFont f;
    if (!c.fontFace.isEmpty()) f.setFamily(c.fontFace);
    f.setPixelSize(std::max(6, int(std::lround((c.fontPt > 0 ? c.fontPt : 9.0) * 96.0 / 72.0))));
    return f;
}

QColor inkOf(const FormControl &c) { return QColor::fromRgba(c.color); }

// The size of the scroll bar's arrows, its track, and its thumb, along its length.
struct BarGeometry {
    bool vertical = true;
    double length = 0, thickness = 0, arrow = 0, trackStart = 0, trackLength = 0, thumbStart = 0, thumbLength = 0;
};

BarGeometry barOf(const FormControl &c, const QSizeF &size)
{
    BarGeometry g;
    g.vertical = c.horizontal ? false : size.height() >= size.width();
    g.length = g.vertical ? size.height() : size.width();
    g.thickness = g.vertical ? size.width() : size.height();
    g.arrow = std::min(g.thickness, g.length / 2.5);
    g.trackStart = g.arrow;
    g.trackLength = std::max(0.0, g.length - 2 * g.arrow);
    const double range = std::max(1, c.max - c.min);
    g.thumbLength = std::min(g.trackLength, std::max(8.0, g.trackLength * std::max(1, c.page) / (range + std::max(1, c.page))));
    const double at = std::clamp(double(c.val - c.min) / range, 0.0, 1.0);
    g.thumbStart = g.trackStart + at * (g.trackLength - g.thumbLength);
    return g;
}

void triangle(QPainter *p, const QRectF &r, int direction)
{
    // 0 up, 1 down, 2 left, 3 right
    const double s = std::max(2.0, std::min(r.width(), r.height()) / 4.0);
    const QPointF c = r.center();
    QPolygonF t;
    switch (direction) {
    case 0: t << QPointF(c.x() - s, c.y() + s / 2) << QPointF(c.x() + s, c.y() + s / 2) << QPointF(c.x(), c.y() - s / 2); break;
    case 1: t << QPointF(c.x() - s, c.y() - s / 2) << QPointF(c.x() + s, c.y() - s / 2) << QPointF(c.x(), c.y() + s / 2); break;
    case 2: t << QPointF(c.x() + s / 2, c.y() - s) << QPointF(c.x() + s / 2, c.y() + s) << QPointF(c.x() - s / 2, c.y()); break;
    default: t << QPointF(c.x() - s / 2, c.y() - s) << QPointF(c.x() - s / 2, c.y() + s) << QPointF(c.x() + s / 2, c.y()); break;
    }
    p->setPen(Qt::NoPen);
    p->setBrush(kInk);
    p->drawPolygon(t);
}

void faceButton(QPainter *p, const QRectF &r)
{
    p->setPen(QPen(kFaceEdge, 1));
    p->setBrush(kFace);
    p->drawRect(r.adjusted(0.5, 0.5, -0.5, -0.5));
}

void words(QPainter *p, const QRectF &r, const FormControl &c, const QString &text, Qt::Alignment align)
{
    if (text.isEmpty()) return;
    p->save();
    p->setClipRect(r);
    p->setFont(fontOf(c));
    p->setPen(inkOf(c));
    p->drawText(r, int(align | Qt::AlignVCenter | Qt::TextWordWrap), text);
    p->restore();
}

} // namespace

void paintControl(QPainter *p, const QRectF &rect, const FormControl &c, const QStringList &items)
{
    if (rect.width() < 1 || rect.height() < 1) return;
    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);
    const QRectF r = rect;
    switch (c.kind) {
    case ControlKind::CheckBox: {
        const double s = std::min(13.0, std::min(r.width(), r.height()));
        const QRectF box(r.left() + 1, r.center().y() - s / 2, s, s);
        p->setPen(QPen(kInk, 1));
        p->setBrush(c.checked == 2 ? QColor(0xC0, 0xC0, 0xC0) : kField);
        p->drawRect(box.adjusted(0.5, 0.5, -0.5, -0.5));
        if (c.checked) {
            p->setPen(QPen(kInk, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            p->drawPolyline(QPolygonF() << QPointF(box.left() + s * 0.22, box.top() + s * 0.52) << QPointF(box.left() + s * 0.42, box.top() + s * 0.72) << QPointF(box.left() + s * 0.78, box.top() + s * 0.28));
        }
        words(p, QRectF(box.right() + 4, r.top(), std::max(0.0, r.right() - box.right() - 4), r.height()), c, c.text, Qt::AlignLeft);
        break;
    }
    case ControlKind::Radio: {
        const double s = std::min(13.0, std::min(r.width(), r.height()));
        const QRectF box(r.left() + 1, r.center().y() - s / 2, s, s);
        p->setPen(QPen(kInk, 1));
        p->setBrush(kField);
        p->drawEllipse(box.adjusted(0.5, 0.5, -0.5, -0.5));
        if (c.checked) {
            p->setPen(Qt::NoPen);
            p->setBrush(kInk);
            p->drawEllipse(box.center(), s * 0.22, s * 0.22);
        }
        words(p, QRectF(box.right() + 4, r.top(), std::max(0.0, r.right() - box.right() - 4), r.height()), c, c.text, Qt::AlignLeft);
        break;
    }
    case ControlKind::Button: {
        p->setPen(QPen(kFaceEdge, 1));
        p->setBrush(kFace);
        p->drawRoundedRect(r.adjusted(0.5, 0.5, -0.5, -0.5), 2, 2);
        words(p, r.adjusted(3, 0, -3, 0), c, c.text, Qt::AlignHCenter);
        break;
    }
    case ControlKind::GroupBox: {
        const QFontMetricsF fm(fontOf(c));
        const double top = r.top() + fm.height() / 2;
        const double w = c.text.isEmpty() ? 0 : fm.horizontalAdvance(c.text) + 6;
        p->setPen(QPen(QColor(0xA0, 0xA0, 0xA0), 1));
        p->setBrush(Qt::NoBrush);
        p->drawRect(QRectF(r.left() + 0.5, top, r.width() - 1, r.bottom() - top - 0.5));
        if (w > 0) {
            p->setPen(Qt::NoPen);
            p->setBrush(kField);
            p->drawRect(QRectF(r.left() + 6, r.top(), std::min(w, r.width() - 12), fm.height()));
            words(p, QRectF(r.left() + 9, r.top(), std::max(0.0, r.width() - 12), fm.height()), c, c.text, Qt::AlignLeft);
        }
        break;
    }
    case ControlKind::Label: words(p, r, c, c.text, Qt::AlignLeft); break;
    case ControlKind::Drop: {
        p->setPen(QPen(kEdge, 1));
        p->setBrush(kField);
        p->drawRect(r.adjusted(0.5, 0.5, -0.5, -0.5));
        const double bw = std::min(kButtonWidth, r.width() / 2);
        const QRectF button(r.right() - bw - 1, r.top() + 1, bw, r.height() - 2);
        faceButton(p, button);
        triangle(p, button, 1);
        if (c.sel >= 1 && c.sel <= items.size()) words(p, QRectF(r.left() + 3, r.top(), std::max(0.0, button.left() - r.left() - 4), r.height()), c, items.at(c.sel - 1), Qt::AlignLeft);
        break;
    }
    case ControlKind::List: {
        p->setPen(QPen(kEdge, 1));
        p->setBrush(kField);
        p->drawRect(r.adjusted(0.5, 0.5, -0.5, -0.5));
        const int visible = std::max(1, int((r.height() - 2) / kListLine));
        const int top = c.sel > visible ? c.sel - visible : 0;
        for (int i = 0; i < visible && top + i < items.size(); ++i) {
            const QRectF line(r.left() + 1, r.top() + 1 + i * kListLine, r.width() - 2, kListLine);
            const bool chosen = top + i + 1 == c.sel;
            if (chosen) {
                p->setPen(Qt::NoPen);
                p->setBrush(kChosen);
                p->drawRect(line);
            }
            p->save();
            p->setClipRect(line);
            p->setFont(fontOf(c));
            p->setPen(chosen ? QColor(Qt::white) : inkOf(c));
            p->drawText(line.adjusted(3, 0, -2, 0), int(Qt::AlignLeft | Qt::AlignVCenter), items.at(top + i));
            p->restore();
        }
        break;
    }
    case ControlKind::Spin: {
        const bool horizontal = c.horizontal || r.width() > r.height() * 1.5;
        const QRectF a = horizontal ? QRectF(r.left(), r.top(), r.width() / 2, r.height()) : QRectF(r.left(), r.top(), r.width(), r.height() / 2);
        const QRectF b = horizontal ? QRectF(r.center().x(), r.top(), r.width() / 2, r.height()) : QRectF(r.left(), r.center().y(), r.width(), r.height() / 2);
        faceButton(p, a);
        faceButton(p, b);
        triangle(p, a, horizontal ? 2 : 0);
        triangle(p, b, horizontal ? 3 : 1);
        break;
    }
    case ControlKind::Scroll: {
        const BarGeometry g = barOf(c, r.size());
        p->setPen(Qt::NoPen);
        p->setBrush(kTrack);
        p->drawRect(r);
        const bool v = g.vertical;
        auto along = [&](double from, double len) { return v ? QRectF(r.left(), r.top() + from, r.width(), len) : QRectF(r.left() + from, r.top(), len, r.height()); };
        const QRectF first = along(0, g.arrow), last = along(g.length - g.arrow, g.arrow), thumb = along(g.thumbStart, g.thumbLength);
        faceButton(p, first);
        faceButton(p, last);
        triangle(p, first, v ? 0 : 2);
        triangle(p, last, v ? 1 : 3);
        if (g.trackLength > 0) {
            p->setPen(QPen(kFaceEdge, 1));
            p->setBrush(kThumb);
            p->drawRect(thumb.adjusted(0.5, 0.5, -0.5, -0.5));
        }
        break;
    }
    }
    p->restore();
}

ControlHit controlHit(const FormControl &c, const QSizeF &size, const QPointF &at, int lineCount)
{
    ControlHit hit;
    if (at.x() < 0 || at.y() < 0 || at.x() > size.width() || at.y() > size.height()) return hit;
    hit.part = ControlPart::Body;
    switch (c.kind) {
    case ControlKind::Spin: {
        const bool horizontal = c.horizontal || size.width() > size.height() * 1.5;
        const bool first = horizontal ? at.x() < size.width() / 2 : at.y() < size.height() / 2;
        // (the upper half steps toward the largest: up; for one laid out left to right, the right half does)
        hit.part = horizontal ? (first ? ControlPart::Down : ControlPart::Up) : (first ? ControlPart::Up : ControlPart::Down);
        break;
    }
    case ControlKind::Scroll: {
        const BarGeometry g = barOf(c, size);
        const double pos = g.vertical ? at.y() : at.x();
        if (pos < g.arrow) hit.part = ControlPart::Up;
        else if (pos > g.length - g.arrow) hit.part = ControlPart::Down;
        else if (pos < g.thumbStart) hit.part = ControlPart::PageUp;
        else if (pos > g.thumbStart + g.thumbLength) hit.part = ControlPart::PageDown;
        else hit.part = ControlPart::Thumb;
        break;
    }
    case ControlKind::List: {
        const int visible = std::max(1, int((size.height() - 2) / kListLine));
        const int top = c.sel > visible ? c.sel - visible : 0;
        const int line = top + int(std::floor((at.y() - 1) / kListLine)) + 1;
        if (at.y() >= 1 && line >= 1 && line <= lineCount && line <= top + visible) {
            hit.part = ControlPart::Line;
            hit.line = line;
        }
        break;
    }
    default: break;
    }
    return hit;
}

} // namespace jp::crunch
