#pragma once
// The pieces File screen pages are made of, the same in every app: cards,
// headings, muted text, a page drawn as paper, a sidebar entry, a command
// shown as a card, and the list of the suite's apps.

#include "app/icons.h"

#include <QAbstractButton>
#include <QLayout>
#include <QList>
#include <QEvent>
#include <QFontMetrics>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QPolygonF>
#include <algorithm>

class QFrame;
class QLabel;

class QBoxLayout;

namespace jp {

inline bool dark() { return uiDark(); }
QString cardStyle();
QLabel *heading(const QString &t, QWidget *parent, double scale = 1.75);
QPixmap paperPixmap(const QImage &page, const QSize &box, qreal dpr);
QFrame *cardFrame(QWidget *parent);
QLabel *mutedLabel(const QString &t, QWidget *parent, double alpha = 0.62);
// Links to the additional license terms, the privacy notice, and the trademark policy, for the License and notices card.
QLabel *legalLinks(QWidget *parent);
QLabel *sectionLabel(const QString &t, QWidget *parent);
// One part of the program made by others, for the Third-party software list on About: its project, what it does
// here, and its license.
struct ThirdParty {
    QString name, url, use, license, licenseUrl;
};
// The "Third-party software" heading and table for the License and notices card (the table is named "thirdParty"):
// the parts every app has, then the app's own parts (JeffPub's .pub reader, for example).
void addThirdPartyList(QBoxLayout *card, QWidget *parent, const QFont &headingFont, const QList<ThirdParty> &extras = {});
void fitHeight(QLabel *l);

// One sidebar entry: icon and label, a rounded highlight when current, a
// softer one on hover, and a short accent bar at the left edge.
class NavItem : public QAbstractButton {
public:
    NavItem(const QString &iconName, const QString &text, QWidget *parent) : QAbstractButton(parent), m_icon(iconName)
    {
        setText(text);
        setCheckable(true);
        setCursor(Qt::PointingHandCursor);
        setFocusPolicy(Qt::TabFocus);
        setMinimumHeight(38);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setAttribute(Qt::WA_Hover);
    }
    QSize sizeHint() const override { return QSize(200, 38); }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const QRectF r = QRectF(rect()).adjusted(10, 2, -10, -2);
        const QColor accent = uiAccent();
        const bool on = isChecked(), hover = underMouse() && !on;
        if (on) {
            QColor bg = accent;
            bg.setAlphaF(dark() ? 0.28f : 0.12f);
            p.setPen(Qt::NoPen);
            p.setBrush(bg);
            p.drawRoundedRect(r, 8, 8);
            p.setBrush(accent);
            p.drawRoundedRect(QRectF(r.left() + 2, r.center().y() - 9, 3.5, 18), 1.75, 1.75);
        } else if (hover) {
            p.setPen(Qt::NoPen);
            p.setBrush(dark() ? QColor(255, 255, 255, 18) : QColor(0, 0, 0, 12));
            p.drawRoundedRect(r, 8, 8);
        }
        if (hasFocus() && !on) {
            p.setPen(QPen(accent, 1.5));
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(r.adjusted(0.75, 0.75, -0.75, -0.75), 8, 8);
        }
        const QColor fg = on ? (dark() ? accent.lighter(170) : accent) : uiText();
        // Tint the line icon to the label color.
        const int is = 18;
        QPixmap pm = jp::icon(m_icon).pixmap(QSize(is, is), devicePixelRatioF());
        QPainter tp(&pm);
        tp.setCompositionMode(QPainter::CompositionMode_SourceIn);
        tp.fillRect(pm.rect(), fg);
        tp.end();
        p.drawPixmap(QPointF(r.left() + 14, r.center().y() - is / 2.0), pm);
        QFont f = font();
        f.setWeight(on ? QFont::DemiBold : QFont::Normal);
        p.setFont(f);
        p.setPen(fg);
        p.drawText(r.adjusted(14 + is + 12, 0, -8, 0), Qt::AlignVCenter | Qt::AlignLeft, text());
    }

private:
    QString m_icon;
};

// A command shown as a card: tinted icon tile, bold title, muted description.
class ActionCard : public QAbstractButton {
public:
    ActionCard(const QString &iconName, const QString &title, const QString &desc, QWidget *parent)
        : QAbstractButton(parent), m_icon(iconName), m_desc(desc)
    {
        setText(title);
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_Hover);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        setMaximumWidth(820);
    }
    // The room the card gives its text: from the right of the icon tile to the left of the chevron, which keeps its own strip at the right so no word runs under it.
    static constexpr int kTextLeft = 68, kChevronStrip = 40, kPad = 12;
    int textWidth(int cardWidth) const { return std::max(60, cardWidth - kTextLeft - kChevronStrip); }
    // The part of the card where the title and the description are drawn (the whole of both, wrapped, at this width).
    QRect titleRect(int cardWidth) const { return QRect(kTextLeft, kPad, textWidth(cardWidth), titleHeight(cardWidth)); }
    QRect descriptionRect(int cardWidth) const { return QRect(kTextLeft, kPad + titleHeight(cardWidth) + 2, textWidth(cardWidth), descriptionHeight(cardWidth)); }
    // The chevron, which the text never reaches.
    QRect chevronRect(int cardWidth, int cardHeight) const { return QRect(cardWidth - 22, cardHeight / 2 - 6, 8, 12); }
    QSize sizeHint() const override { return QSize(300, heightForWidth(300)); }
    QSize minimumSizeHint() const override { return QSize(kTextLeft + kChevronStrip + 60, heightForWidth(kTextLeft + kChevronStrip + 60)); }   // (a row of cards stacks them before they get this narrow)
    bool hasHeightForWidth() const override { return true; }
    // (a card is never wider than its maximum, so a layout that asks at a wider width gets the height the card will really have)
    int heightForWidth(int w) const override
    {
        w = std::min(w, maximumWidth());
        return std::max(64, kPad + titleHeight(w) + 2 + descriptionHeight(w) + kPad);
    }
    const QString &description() const { return m_desc; }

protected:
    void resizeEvent(QResizeEvent *e) override
    {
        QAbstractButton::resizeEvent(e);
        // (a layout works out heights at widths of its own; the card is never shorter than its text at the width it really has)
        const int h = heightForWidth(width());
        if (h != minimumHeight()) setMinimumHeight(h);
    }
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
        const bool hover = underMouse(), down = isDown();
        const QColor accent = uiAccent();
        QColor bg = down ? uiCardPressed() : uiCard();
        QColor border = uiLine();
        if (hover || hasFocus()) border = accent;
        p.setPen(QPen(border, hover ? 1.4 : 1));
        p.setBrush(bg);
        p.drawRoundedRect(r, 10, 10);
        const QRectF tile(r.left() + 14, r.top() + 12, 40, 40);
        QColor tbg = accent;
        tbg.setAlphaF(dark() ? 0.25f : 0.10f);
        p.setPen(Qt::NoPen);
        p.setBrush(tbg);
        p.drawRoundedRect(tile, 9, 9);
        QPixmap pm = jp::icon(m_icon).pixmap(QSize(22, 22), devicePixelRatioF());
        {
            QPainter tp(&pm);
            tp.setCompositionMode(QPainter::CompositionMode_SourceIn);
            tp.fillRect(pm.rect(), dark() ? accent.lighter(150) : accent);
        }
        p.drawPixmap(QPointF(tile.center().x() - 11, tile.center().y() - 11), pm);
        QFont tf = font();
        tf.setWeight(QFont::DemiBold);
        p.setFont(tf);
        p.setPen(uiText());
        p.drawText(titleRect(width()), Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, text());
        p.setFont(font());
        QColor dc = uiText();
        dc.setAlphaF(0.68f);
        p.setPen(dc);
        p.drawText(descriptionRect(width()), Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignTop, m_desc);
        // Chevron on the right.
        p.setPen(QPen(hover ? accent : dc, 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        const QPointF c(chevronRect(width(), height()).center());
        p.drawPolyline(QPolygonF({c + QPointF(-3, -5), c + QPointF(2, 0), c + QPointF(-3, 5)}));
    }
    bool event(QEvent *e) override
    {
        if (e->type() == QEvent::HoverEnter || e->type() == QEvent::HoverLeave) update();
        return QAbstractButton::event(e);
    }

private:
    int titleHeight(int cardWidth) const
    {
        QFont tf = font();
        tf.setWeight(QFont::DemiBold);
        return std::max(QFontMetrics(tf).height() + 2, QFontMetrics(tf).boundingRect(QRect(0, 0, textWidth(cardWidth), 100000), Qt::TextWordWrap, text()).height());
    }
    int descriptionHeight(int cardWidth) const { return QFontMetrics(font()).boundingRect(QRect(0, 0, textWidth(cardWidth), 100000), Qt::TextWordWrap, m_desc).height(); }
    QString m_icon, m_desc;
};

// Cards side by side that stack when the window is too narrow to give each a readable line: as many to a row as leave each at least kWide pixels, down to one to a row. A row is
// as tall as its tallest card (each card taller than its own text is made as tall as the row), and every card shows all its text.
class CardRow : public QLayout {
public:
    static constexpr int kWide = 230;
    explicit CardRow(int spacing = 12) : m_spacing(spacing) {}
    ~CardRow() override { qDeleteAll(m_items); }
    void addItem(QLayoutItem *item) override { m_items << item; }
    int count() const override { return int(m_items.size()); }
    QLayoutItem *itemAt(int i) const override { return m_items.value(i); }
    QLayoutItem *takeAt(int i) override { return i >= 0 && i < m_items.size() ? m_items.takeAt(i) : nullptr; }
    Qt::Orientations expandingDirections() const override { return Qt::Horizontal; }
    bool hasHeightForWidth() const override { return true; }
    int heightForWidth(int w) const override { return arrange(QRect(0, 0, w, 0), false); }
    QSize minimumSize() const override
    {
        int w = 0;
        for (const QLayoutItem *i : m_items) w = std::max(w, i->minimumSize().width());
        return QSize(w, heightForWidth(w));
    }
    QSize sizeHint() const override
    {
        int w = 0;
        for (const QLayoutItem *i : m_items) w += i->sizeHint().width() + m_spacing;
        w = std::max(0, w - m_spacing);
        return QSize(w, heightForWidth(w));
    }
    void setGeometry(const QRect &r) override
    {
        QLayout::setGeometry(r);
        arrange(r, true);
    }

private:
    int columnsFor(int width) const
    {
        const int n = std::max(1, int(m_items.size()));
        for (int k = n; k > 1; --k)
            if ((width - (k - 1) * m_spacing) / k >= kWide) return k;
        return 1;
    }
    // Lays the cards out in the rectangle (or only measures, when `place` is false) and returns the height used.
    int arrange(const QRect &r, bool place) const
    {
        const int k = columnsFor(r.width());
        const int each = (r.width() - (k - 1) * m_spacing) / k;
        int y = r.y(), done = 0;
        while (done < m_items.size()) {
            const int inRow = std::min<int>(k, int(m_items.size()) - done);
            int rowHeight = 0;
            for (int j = 0; j < inRow; ++j) {
                QLayoutItem *item = m_items[done + j];
                const int w = std::min(each, item->maximumSize().width());
                rowHeight = std::max(rowHeight, item->hasHeightForWidth() ? item->heightForWidth(w) : item->sizeHint().height());
            }
            if (place)
                for (int j = 0; j < inRow; ++j) {
                    QLayoutItem *item = m_items[done + j];
                    item->setGeometry(QRect(r.x() + j * (each + m_spacing), y, std::min(each, item->maximumSize().width()), rowHeight));
                }
            y += rowHeight + m_spacing;
            done += inRow;
        }
        return std::max(0, y - m_spacing - r.y());
    }
    QList<QLayoutItem *> m_items;
    int m_spacing;
};

// The app's badge: its two letters in a rounded square in its color, drawn for
// the screen of `on`.
QPixmap appBadge(const QWidget *on, int size, int pixelSize, qreal radius);

// The suite the app belongs to, as a card: the JeffOffice logo, then each app
// and where it stands. `screen` is the File screen the card is for.
QWidget *suiteCard(QWidget *parent, const QWidget *screen);

} // namespace jp
