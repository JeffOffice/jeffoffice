#pragma once
// The pieces File screen pages are made of, the same in every app: cards,
// headings, muted text, a page drawn as paper, a sidebar entry, a command
// shown as a card, and the list of the suite's apps.

#include "app/icons.h"

#include <QAbstractButton>
#include <QEvent>
#include <QFontMetrics>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QPolygonF>
#include <algorithm>

class QFrame;
class QLabel;

namespace jp {

inline bool dark() { return uiDark(); }
QString cardStyle();
QLabel *heading(const QString &t, QWidget *parent, double scale = 1.75);
QPixmap paperPixmap(const QImage &page, const QSize &box, qreal dpr);
QFrame *cardFrame(QWidget *parent);
QLabel *mutedLabel(const QString &t, QWidget *parent, double alpha = 0.62);
QLabel *sectionLabel(const QString &t, QWidget *parent);
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
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setMaximumWidth(820);
    }
    QSize sizeHint() const override
    {
        const QFontMetrics fm(font());
        const int textW = std::max(200, width() - 92);
        const int dh = fm.boundingRect(QRect(0, 0, textW, 1000), Qt::TextWordWrap, m_desc).height();
        return QSize(520, std::max(64, 22 + fm.height() + 4 + dh));
    }
    bool hasHeightForWidth() const override { return true; }
    int heightForWidth(int w) const override
    {
        const QFontMetrics fm(font());
        const int dh = fm.boundingRect(QRect(0, 0, std::max(200, w - 92), 1000), Qt::TextWordWrap, m_desc).height();
        return std::max(64, 22 + fm.height() + 4 + dh);
    }

protected:
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
        const QFontMetrics fm(font());
        const QRectF tr(tile.right() + 14, r.top() + 11, r.right() - tile.right() - 28, fm.height() + 2);
        p.drawText(tr, Qt::AlignLeft | Qt::AlignVCenter, text());
        p.setFont(font());
        QColor dc = uiText();
        dc.setAlphaF(0.68f);
        p.setPen(dc);
        p.drawText(QRectF(tr.left(), tr.bottom() + 2, tr.width(), r.bottom() - tr.bottom() - 8), Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignTop, m_desc);
        // Chevron on the right.
        p.setPen(QPen(hover ? accent : dc, 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        const QPointF c(r.right() - 18, r.center().y());
        p.drawPolyline(QPolygonF({c + QPointF(-3, -5), c + QPointF(2, 0), c + QPointF(-3, 5)}));
    }
    bool event(QEvent *e) override
    {
        if (e->type() == QEvent::HoverEnter || e->type() == QEvent::HoverLeave) update();
        return QAbstractButton::event(e);
    }

private:
    QString m_icon, m_desc;
};

// The app's badge: its two letters in a rounded square in its color, drawn for
// the screen of `on`.
QPixmap appBadge(const QWidget *on, int size, int pixelSize, qreal radius);

// The suite the app belongs to, as a card: the JeffOffice logo, then each app
// and where it stands. `screen` is the File screen the card is for.
QWidget *suiteCard(QWidget *parent, const QWidget *screen);

} // namespace jp
