#include "app/pagenumbergallery.h"

#include "app/icons.h"

#include <QCoreApplication>
#include <QFont>
#include <QFontMetricsF>
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
        t.size = d.area == PageNumberArea::Cursor ? 0 : d.area == PageNumberArea::Margin ? 9 : 11;
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
        b.rect = QRectF(0, 0, 20, 20);
        return b;
    }
    if (d.area == PageNumberArea::Margin) {
        // (A box in the margin is centered in the strip it is in, and the box is as wide as the strip lets it be: a four-digit number in it still fits.)
        const double room = d.side == PageNumberSide::Left ? m.left() : m.right();
        if (d.look == PageNumberLook::Circle) w = h = std::clamp(room - 4, 22.0, 30.0);
        else if (d.look == PageNumberLook::Tab) {
            w = std::clamp(room - 4, 22.0, 34.0);
            h = d.turned ? 56 : 28;
        } else {   // a vertical bar
            w = std::clamp(room - 12, 22.0, 26.0);
            h = 120;
        }
        const double x = d.side == PageNumberSide::Left ? (room - w) / 2 : W - room + (room - w) / 2;
        b.rect = QRectF(std::max(0.0, x), H / 2 - h / 2, w, h);
        return b;
    }
    // (Wide enough for four digits at the size the number is set: a bold 11 point digit is about 7 points across.)
    if (d.look == PageNumberLook::Circle) w = h = 34;
    else if (d.look == PageNumberLook::RoundedBox) { w = 48; h = 24; }
    else if (d.look == PageNumberLook::Tab) { w = 54; h = 24; }
    else { w = std::max(24.0, W - m.left() - m.right()); h = 24; }
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
    // A small page, drawn as if it were 220 by 284 units across with margins of 22 and 36: the box a design has is its size on a larger page, drawn a little larger so that its number reads.
    const double ph = rc.height() * 0.94, pw = std::min(rc.width() * 0.9, ph * 0.775);
    const QRectF page(rc.center().x() - pw / 2, rc.center().y() - ph / 2, pw, ph);
    const double k = pw / 220.0;
    const QMarginsF margins(22, 44, 22, 44);
    constexpr int kLeastPixel = 8;   // (the smallest letters a picture draws: smaller ones can't be read)
    p->setPen(QPen(QColor(0x9A, 0xA0, 0xA8), 1));
    p->setBrush(Qt::white);
    p->drawRect(page);
    auto toPage = [&](const QRectF &r) { return QRectF(page.left() + r.left() * k, page.top() + r.top() * k, r.width() * k, r.height() * k); };
    // The text of the page, as gray lines.
    p->setPen(Qt::NoPen);
    p->setBrush(QColor(0xD2, 0xD6, 0xDB));
    const bool inline_ = d.area == PageNumberArea::Cursor;
    const double lineStep = 17;
    for (double y = 76; y < 284 - 70; y += lineStep) {
        if (inline_ && y > 120 && y < 154) continue;
        p->drawRect(toPage(QRectF(margins.left(), y, 220 - margins.left() - margins.right() - ((int(y) % 3) ? 0 : 30), 5)));
    }
    const PageNumberType type = pageNumberType(d);
    const QVector<PageNumberPiece> pieces = pageNumberPieces(d);
    QString text;
    for (const PageNumberPiece &piece : pieces) text += piece.kind == PageNumberPiece::Page ? QStringLiteral("1") : piece.kind == PageNumberPiece::Count ? QStringLiteral("9") : piece.text;
    QFont f = p->font();
    f.setPixelSize(type.size >= 20 ? 11 : kLeastPixel);
    f.setBold(type.bold);
    p->setFont(f);
    const QFontMetricsF fm(f);
    const QColor ink(0x30, 0x34, 0x3a);
    if (inline_) {
        const double cy = 135;
        const QRectF line = toPage(QRectF(margins.left(), cy - 14, 220 - margins.left() - margins.right(), 28));
        if (d.look == PageNumberLook::Circle) {
            const double dot = std::max(12.0, 20 * k);
            const QRectF r(line.center().x() - dot / 2, line.center().y() - dot / 2, dot, dot);
            p->setPen(Qt::NoPen);
            p->setBrush(accent);
            p->drawEllipse(r);
            p->setPen(Qt::white);
            p->drawText(r, Qt::AlignCenter, text);
        } else {
            p->setPen(ink);
            p->drawText(line, Qt::AlignCenter | Qt::TextDontClip, text);
        }
        p->restore();
        return;
    }
    if (pageNumberHasBox(d)) {
        const PageNumberBox box = pageNumberBox(d, QSizeF(220, 284), margins);
        QRectF r = toPage(box.rect);
        // (A short side is drawn larger, a long one as it is.)
        const double growW = box.rect.width() < 100 ? 1.5 : 1.0, growH = box.rect.height() < 100 ? 1.5 : 1.0;
        r = QRectF(r.center().x() - r.width() * growW / 2, r.center().y() - r.height() * growH / 2, r.width() * growW, r.height() * growH);
        p->setPen(Qt::NoPen);
        p->setBrush(accent);
        if (box.preset == QLatin1String("ellipse")) p->drawEllipse(r);
        else if (box.preset == QLatin1String("roundRect")) p->drawRoundedRect(r, r.height() * 0.3, r.height() * 0.3);
        else p->drawRect(r);
        p->setPen(Qt::white);
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
    const double textH = fm.height();
    const double rowH = std::max(26.0 * k, textH + 6);   // (the row is as tall as its letters need, and a little more)
    const double cy = top ? page.top() + margins.top() * k / 2 : page.bottom() - margins.bottom() * k / 2;
    const QRectF row(page.left() + margins.left() * k, cy - rowH / 2, (220 - margins.left() - margins.right()) * k, rowH);
    const Qt::Alignment al = d.side == PageNumberSide::Left ? Qt::AlignLeft : d.side == PageNumberSide::Right ? Qt::AlignRight : Qt::AlignHCenter;
    p->setPen(ink);
    p->drawText(row, al | Qt::AlignVCenter | Qt::TextDontClip, text);
    const PageNumberRule rule = pageNumberRule(d);
    if (rule != PageNumberRule::None) {
        // (The rule lies clear of the letters: the height of the letters and two pixels from the middle of the row.)
        p->setPen(QPen(ink, 1));
        const double y = rule == PageNumberRule::Above ? row.center().y() - textH / 2 - 2 : row.center().y() + textH / 2 + 2;
        p->drawLine(QPointF(row.left(), y), QPointF(row.right(), y));
    }
    p->restore();
}

QSize pageNumberItemSize() { return QSize(56, 72); }

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
