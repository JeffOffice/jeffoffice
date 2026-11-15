#include "app/pagenumbergallery.h"

#include "app/icons.h"

#include <QCoreApplication>
#include <QFont>
#include <QPainter>
#include <QPainterPath>
#include <algorithm>

namespace jp {

namespace {

QString lookName(PageNumberLook look)
{
    switch (look) {
    case PageNumberLook::Plain: return QCoreApplication::translate("jp::PageNumbers", "Plain number");
    case PageNumberLook::OfCount: return QCoreApplication::translate("jp::PageNumbers", "Page X of Y");
    case PageNumberLook::Bold: return QCoreApplication::translate("jp::PageNumbers", "Bold large number");
    case PageNumberLook::RuleAbove: return QCoreApplication::translate("jp::PageNumbers", "Number with a rule above");
    case PageNumberLook::RuleBelow: return QCoreApplication::translate("jp::PageNumbers", "Number with a rule below");
    case PageNumberLook::Circle: return QCoreApplication::translate("jp::PageNumbers", "Number in a circle");
    case PageNumberLook::RoundedBox: return QCoreApplication::translate("jp::PageNumbers", "Number in a rounded box");
    case PageNumberLook::Tab: return QCoreApplication::translate("jp::PageNumbers", "Number on a tab");
    case PageNumberLook::Bar: return QCoreApplication::translate("jp::PageNumbers", "Number on a bar");
    case PageNumberLook::PageLine: return QCoreApplication::translate("jp::PageNumbers", "Page X with a thin line");
    }
    return {};
}

QString sideName(PageNumberSide side, bool margin)
{
    if (margin) return side == PageNumberSide::Left ? QCoreApplication::translate("jp::PageNumbers", "left margin") : QCoreApplication::translate("jp::PageNumbers", "right margin");
    switch (side) {
    case PageNumberSide::Left: return QCoreApplication::translate("jp::PageNumbers", "left");
    case PageNumberSide::Center: return QCoreApplication::translate("jp::PageNumbers", "center");
    case PageNumberSide::Right: return QCoreApplication::translate("jp::PageNumbers", "right");
    }
    return {};
}

const char *areaWord(PageNumberArea a)
{
    switch (a) {
    case PageNumberArea::Top: return "top";
    case PageNumberArea::Bottom: return "bottom";
    case PageNumberArea::Margin: return "margin";
    case PageNumberArea::Cursor: return "cursor";
    }
    return "";
}

const char *lookWord(PageNumberLook l)
{
    switch (l) {
    case PageNumberLook::Plain: return "plain";
    case PageNumberLook::OfCount: return "ofCount";
    case PageNumberLook::Bold: return "bold";
    case PageNumberLook::RuleAbove: return "ruleAbove";
    case PageNumberLook::RuleBelow: return "ruleBelow";
    case PageNumberLook::Circle: return "circle";
    case PageNumberLook::RoundedBox: return "roundedBox";
    case PageNumberLook::Tab: return "tab";
    case PageNumberLook::Bar: return "bar";
    case PageNumberLook::PageLine: return "pageLine";
    }
    return "";
}

const char *sideWord(PageNumberSide s) { return s == PageNumberSide::Left ? "left" : s == PageNumberSide::Right ? "right" : "center"; }

PageNumberDesign make(PageNumberArea area, PageNumberLook look, PageNumberSide side, bool turned = false)
{
    PageNumberDesign d;
    d.area = area;
    d.look = look;
    d.side = side;
    d.turned = turned;
    if (area == PageNumberArea::Margin) {
        d.id = QStringLiteral("margin.%1.%2").arg(QLatin1String(sideWord(side)), QLatin1String(lookWord(look)));
        d.label = QCoreApplication::translate("jp::PageNumbers", "%1, %2").arg(lookName(look), sideName(side, true));
        if (turned) d.label = QCoreApplication::translate("jp::PageNumbers", "%1, turned").arg(d.label);
    } else if (area == PageNumberArea::Cursor) {
        d.id = QStringLiteral("cursor.%1").arg(QLatin1String(lookWord(look)));
        d.label = lookName(look);
    } else {
        d.id = QStringLiteral("%1.%2.%3").arg(QLatin1String(areaWord(area)), QLatin1String(lookWord(look)), QLatin1String(sideWord(side)));
        d.label = QCoreApplication::translate("jp::PageNumbers", "%1, %2").arg(lookName(look), sideName(side, false));
    }
    return d;
}

} // namespace

QString pageNumberAreaName(PageNumberArea area)
{
    switch (area) {
    case PageNumberArea::Top: return QCoreApplication::translate("jp::PageNumbers", "Top of Page");
    case PageNumberArea::Bottom: return QCoreApplication::translate("jp::PageNumbers", "Bottom of Page");
    case PageNumberArea::Margin: return QCoreApplication::translate("jp::PageNumbers", "Page Margins");
    case PageNumberArea::Cursor: return QCoreApplication::translate("jp::PageNumbers", "Current Position");
    }
    return {};
}

QVector<PageNumberDesign> pageNumberDesigns()
{
    using A = PageNumberArea;
    using L = PageNumberLook;
    using S = PageNumberSide;
    QVector<PageNumberDesign> out;
    for (const A area : {A::Top, A::Bottom}) {
        for (const L look : {L::Plain, L::OfCount, L::Bold})
            for (const S side : {S::Left, S::Center, S::Right}) out << make(area, look, side);
        out << make(area, L::RuleAbove, S::Center) << make(area, L::RuleBelow, S::Center) << make(area, L::PageLine, S::Left) << make(area, L::Circle, S::Right)
            << make(area, L::RoundedBox, S::Center) << make(area, L::Tab, S::Right) << make(area, L::Bar, S::Center);
    }
    out << make(A::Margin, L::Circle, S::Left) << make(A::Margin, L::Circle, S::Right) << make(A::Margin, L::Tab, S::Left) << make(A::Margin, L::Tab, S::Right, true)
        << make(A::Margin, L::Bar, S::Left, true) << make(A::Margin, L::Bar, S::Right, true);
    for (const L look : {L::Plain, L::OfCount, L::Bold, L::Circle}) out << make(A::Cursor, look, S::Center);
    return out;
}

std::optional<PageNumberDesign> pageNumberDesignById(const QString &id)
{
    for (const PageNumberDesign &d : pageNumberDesigns())
        if (d.id == id) return d;
    return std::nullopt;
}

QVector<PageNumberPiece> pageNumberPieces(const PageNumberDesign &d)
{
    using P = PageNumberPiece;
    if (d.look == PageNumberLook::OfCount)
        return {P{P::Text, QCoreApplication::translate("jp::PageNumbers", "Page") + QLatin1Char(' ')}, P{P::Page, {}},
                P{P::Text, QLatin1Char(' ') + QCoreApplication::translate("jp::PageNumbers", "of") + QLatin1Char(' ')}, P{P::Count, {}}};
    if (d.look == PageNumberLook::PageLine) return {P{P::Text, QCoreApplication::translate("jp::PageNumbers", "Page") + QLatin1Char(' ')}, P{P::Page, {}}};
    return {P{P::Page, {}}};
}

PageNumberType pageNumberType(const PageNumberDesign &d)
{
    PageNumberType t;
    switch (d.look) {
    case PageNumberLook::Bold:
        t.bold = true;
        t.size = d.area == PageNumberArea::Cursor ? 0 : 22;
        break;
    case PageNumberLook::Circle:
    case PageNumberLook::RoundedBox:
    case PageNumberLook::Tab:
    case PageNumberLook::Bar:
        t.bold = true;
        t.size = d.area == PageNumberArea::Cursor ? 0 : 11;
        t.onAccent = true;
        break;
    default: break;
    }
    return t;
}

PageNumberRule pageNumberRule(const PageNumberDesign &d)
{
    if (d.look == PageNumberLook::RuleAbove) return PageNumberRule::Above;
    if (d.look == PageNumberLook::RuleBelow || d.look == PageNumberLook::PageLine) return PageNumberRule::Below;
    return PageNumberRule::None;
}

bool pageNumberHasBox(const PageNumberDesign &d)
{
    return d.look == PageNumberLook::Circle || d.look == PageNumberLook::RoundedBox || d.look == PageNumberLook::Tab || d.look == PageNumberLook::Bar;
}

PageNumberBox pageNumberBox(const PageNumberDesign &d, const QSizeF &page, const QMarginsF &m)
{
    PageNumberBox b;
    if (!pageNumberHasBox(d)) return b;
    b.turned = d.turned;
    b.preset = d.look == PageNumberLook::Circle ? QStringLiteral("ellipse") : d.look == PageNumberLook::RoundedBox ? QStringLiteral("roundRect") : QStringLiteral("rect");
    const double W = page.width(), H = page.height();
    double w = 0, h = 0;
    if (d.area == PageNumberArea::Cursor) {
        b.rect = QRectF(0, 0, 18, 18);
        return b;
    }
    if (d.area == PageNumberArea::Margin) {
        const double room = d.side == PageNumberSide::Left ? m.left() : m.right();
        if (d.look == PageNumberLook::Circle) w = h = std::clamp(room - 8, 14.0, 26.0);
        else if (d.look == PageNumberLook::Tab) {
            w = d.turned ? std::clamp(room - 4, 14.0, 26.0) : std::clamp(room - 4, 16.0, 34.0);
            h = d.turned ? 54 : 26;
        } else {   // a vertical bar
            w = std::clamp(room - 20, 12.0, 20.0);
            h = 120;
        }
        double x = d.side == PageNumberSide::Left ? (room - w) / 2 : W - room + (room - w) / 2;
        if (d.look == PageNumberLook::Tab) x = d.side == PageNumberSide::Left ? 0 : W - w;
        b.rect = QRectF(std::max(0.0, x), H / 2 - h / 2, w, h);
        return b;
    }
    if (d.look == PageNumberLook::Circle) w = h = 26;
    else if (d.look == PageNumberLook::RoundedBox) { w = 40; h = 22; }
    else if (d.look == PageNumberLook::Tab) { w = 46; h = 22; }
    else { w = std::max(24.0, W - m.left() - m.right()); h = 22; }
    const bool top = d.area == PageNumberArea::Top;
    const double y = top ? std::max(10.0, m.top() / 2 - h / 2) : std::min(H - h - 10, H - m.bottom() / 2 - h / 2);
    double x = (W - w) / 2;
    if (d.look != PageNumberLook::Bar) {
        if (d.side == PageNumberSide::Left) x = d.look == PageNumberLook::Tab ? 0 : m.left();
        else if (d.side == PageNumberSide::Right) x = d.look == PageNumberLook::Tab ? W - w : W - m.right() - w;
    } else {
        x = m.left();
    }
    b.rect = QRectF(x, y, w, h);
    return b;
}

bool pageNumberIsTextOnly(const PageNumberDesign &d)
{
    if (d.area == PageNumberArea::Margin) return false;
    return d.look == PageNumberLook::Plain || d.look == PageNumberLook::OfCount || d.look == PageNumberLook::Bold;
}

void paintPageNumberDesign(QPainter *p, const QRectF &rc, const PageNumberDesign &d, const QColor &accent)
{
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    // A small page, drawn as if it were 220 by 284 units across with margins of 30 and 36: the box a design has is its size on a larger page, so it reads in the picture.
    const double ph = rc.height() * 0.94, pw = std::min(rc.width() * 0.84, ph * 0.775);
    const QRectF page(rc.center().x() - pw / 2, rc.center().y() - ph / 2, pw, ph);
    const double k = pw / 220.0;
    const QMarginsF margins(30, 36, 30, 36);
    p->setPen(QPen(QColor(0x9A, 0xA0, 0xA8), 1));
    p->setBrush(Qt::white);
    p->drawRect(page);
    auto toPage = [&](const QRectF &r) { return QRectF(page.left() + r.left() * k, page.top() + r.top() * k, r.width() * k, r.height() * k); };
    // The text of the page, as gray lines.
    p->setPen(Qt::NoPen);
    p->setBrush(QColor(0xD2, 0xD6, 0xDB));
    const bool inline_ = d.area == PageNumberArea::Cursor;
    const double lineStep = 17;
    for (double y = 60; y < 284 - 56; y += lineStep) {
        if (inline_ && y > 120 && y < 150) continue;
        p->drawRect(toPage(QRectF(margins.left(), y, 220 - margins.left() - margins.right() - ((int(y) % 3) ? 0 : 30), 5)));
    }
    const PageNumberType type = pageNumberType(d);
    const QVector<PageNumberPiece> pieces = pageNumberPieces(d);
    QString text;
    for (const PageNumberPiece &piece : pieces) text += piece.kind == PageNumberPiece::Page ? QStringLiteral("1") : piece.kind == PageNumberPiece::Count ? QStringLiteral("9") : piece.text;
    QFont f = p->font();
    f.setPixelSize(std::max(5, int(std::lround((type.size > 0 ? type.size * 0.62 : 15) * k))));
    f.setBold(type.bold);
    p->setFont(f);
    const QColor ink(0x30, 0x34, 0x3a);
    if (inline_) {
        const double cy = 135;
        const QRectF line = toPage(QRectF(margins.left(), cy - 11, 220 - margins.left() - margins.right(), 22));
        if (d.look == PageNumberLook::Circle) {
            const QRectF dot = toPage(QRectF(110 - 10, cy - 10, 20, 20));
            p->setPen(Qt::NoPen);
            p->setBrush(accent);
            p->drawEllipse(dot);
            p->setPen(Qt::white);
            p->setFont(f);
            p->drawText(dot, Qt::AlignCenter, text);
        } else {
            p->setPen(ink);
            p->drawText(line, Qt::AlignCenter, text);
        }
        p->restore();
        return;
    }
    if (pageNumberHasBox(d)) {
        const PageNumberBox box = pageNumberBox(d, QSizeF(220, 284), margins);
        const QRectF r = toPage(box.rect);
        p->setPen(Qt::NoPen);
        p->setBrush(accent);
        if (box.preset == QLatin1String("ellipse")) p->drawEllipse(r);
        else if (box.preset == QLatin1String("roundRect")) p->drawRoundedRect(r, r.height() * 0.3, r.height() * 0.3);
        else p->drawRect(r);
        p->setPen(Qt::white);
        QFont bf = f;
        bf.setPixelSize(std::max(5, int(std::lround(std::min(r.width(), r.height()) * 0.5))));
        p->setFont(bf);
        if (box.turned) {
            p->translate(r.center());
            p->rotate(-90);
            p->drawText(QRectF(-r.height() / 2, -r.width() / 2, r.height(), r.width()), Qt::AlignCenter, text);
        } else {
            p->drawText(r, Qt::AlignCenter, text);
        }
        p->restore();
        return;
    }
    const bool top = d.area == PageNumberArea::Top;
    const double rowH = type.size > 0 ? std::max(20.0, type.size * 1.1) : 20;
    const double cy = top ? margins.top() / 2 : 284 - margins.bottom() / 2;
    const QRectF row = toPage(QRectF(margins.left(), cy - rowH / 2, 220 - margins.left() - margins.right(), rowH));
    const Qt::Alignment al = d.side == PageNumberSide::Left ? Qt::AlignLeft : d.side == PageNumberSide::Right ? Qt::AlignRight : Qt::AlignHCenter;
    p->setPen(ink);
    p->drawText(row, al | Qt::AlignVCenter, text);
    const PageNumberRule rule = pageNumberRule(d);
    if (rule != PageNumberRule::None) {
        p->setPen(QPen(ink, std::max(1.0, k * 0.9)));
        const double y = rule == PageNumberRule::Above ? row.top() : row.bottom();
        p->drawLine(QPointF(row.left(), y), QPointF(row.right(), y));
    }
    p->restore();
}

QSize pageNumberItemSize() { return QSize(52, 64); }

QVector<GalleryItem> pageNumberGalleryItems(const QColor &accent, const std::function<bool(const PageNumberDesign &)> &accepts)
{
    QVector<GalleryItem> items;
    for (const PageNumberDesign &d : pageNumberDesigns()) {
        if (accepts && !accepts(d)) continue;
        items << GalleryItem{d.id, QCoreApplication::translate("jp::PageNumbers", "%1: %2").arg(pageNumberAreaName(d.area), d.label), drawnIcon([d, accent](QPainter *p, const QRectF &rc) { paintPageNumberDesign(p, rc, d, accent); }), pageNumberAreaName(d.area)};
    }
    return items;
}

} // namespace jp
