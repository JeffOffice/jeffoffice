#include "app/ribbon.h"
#include "app/keyboardnav.h"
#include "app/theme.h"

#include "app/icons.h"
#include "core/appinfo.h"

#include <QAction>
#include <QAbstractSpinBox>
#include <QLineEdit>
#include <QComboBox>
#include <QGridLayout>
#include <QGuiApplication>
#include <QHash>
#include <QHBoxLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QApplication>
#include <QCoreApplication>
#include <QWidgetAction>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollArea>
#include <QStyle>
#include <QStyleOptionSpinBox>
#include <QStyleOptionToolButton>
#include <QStylePainter>
#include <QScrollBar>
#include <QStackedWidget>
#include <QStyleHints>
#include <QPointer>
#include <QTimer>
#include <QResizeEvent>
#include <QVBoxLayout>

#include <algorithm>
#include <climits>

namespace jp {

static bool dark() { return uiDark(); }
// In high contrast, the system's colors (from the palette) instead.
static QColor ribbonBg() { return uiHighContrast() ? QApplication::palette().color(QPalette::Window) : dark() ? QColor(0x26, 0x2B, 0x33) : QColor(0xFB, 0xFB, 0xFC); }
static QColor headerBg() { return uiHighContrast() ? QApplication::palette().color(QPalette::Window) : dark() ? QColor(0x1E, 0x22, 0x29) : QColor(0xF0, 0xF1, 0xF4); }
static QColor lineColor() { return uiHighContrast() ? QApplication::palette().color(QPalette::WindowText) : dark() ? QColor(0x3A, 0x41, 0x4C) : QColor(0xD9, 0xDC, 0xE2); }
static QColor mutedText() { return uiHighContrast() ? QApplication::palette().color(QPalette::WindowText) : dark() ? QColor(0xA9, 0xB0, 0xBC) : QColor(0x5E, 0x66, 0x73); }

int ribbonRowHeight(const QFontMetrics &fm) { return std::max(kMinRowHeight, fm.height() + 4); }

static QStringList largeLabelLines(const QToolButton *b)
{
    QString t = b->text();
    t.replace(QStringLiteral("&&"), QStringLiteral("\x01")).remove(QLatin1Char('&')).replace(QLatin1Char('\x01'), QLatin1Char('&'));
    t.remove(QStringLiteral("...")).remove(QChar(0x2026));
    const QFontMetrics fm(b->font());
    if (fm.horizontalAdvance(t) <= 70) return {t};
    // Break where the two lines balance best: at a space (dropped), or
    // after a hyphen inside a word (kept), as long compound words in some
    // languages need ("Pagina-achtergrond").
    int best = -1, bestDiff = INT_MAX;
    bool atSpace = true;
    for (int i = 0; i < t.size(); ++i) {
        const bool space = t[i] == ' ', hyphen = t[i] == '-' && i > 0 && i + 1 < t.size() && t[i - 1] != ' ' && t[i + 1] != ' ';
        if (!space && !hyphen) continue;
        const QString first = space ? t.left(i) : t.left(i + 1);
        const int d = std::abs(fm.horizontalAdvance(first) - fm.horizontalAdvance(t.mid(i + 1)));
        if (d < bestDiff) { bestDiff = d; best = i; atSpace = space; }
    }
    if (best < 0) return {t};
    return {atSpace ? t.left(best) : t.left(best + 1), t.mid(best + 1)};
}

// Icon, a label of up to two lines, and the margins: at least 64, more for a larger font.
static int largeRibbonButtonHeight(const QFontMetrics &fm, int iconHeight) { return std::max(64, iconHeight + 7 + 2 * (fm.height() - 1)); }

QStringList largeRibbonLabelLines(const QToolButton *b) { return largeLabelLines(b); }

QSize largeRibbonButtonHint(const QToolButton *b)
{
    const QFontMetrics fm(b->font());
    int w = b->iconSize().width() + 18;
    const QStringList lines = largeLabelLines(b);
    // A two-line label with a menu carries its chevron at the end of the
    // second line; there is no room for it underneath.
    const bool menu = b->menu() || b->popupMode() == QToolButton::InstantPopup;
    for (int i = 0; i < lines.size(); ++i)
        w = std::max(w, fm.horizontalAdvance(lines[i]) + 14 + (menu && i == 1 ? 12 : 0));
    // As wide as the label's longer line, so no word is cut short: a single
    // long word ("Onlineafbeeldingen") can't move to another line.
    return QSize(std::min(w, 200), largeRibbonButtonHeight(fm, b->iconSize().height()));
}

// Icon on top, label on up to two lines, a small chevron for menus, and soft
// rounded hover, press and on states. Qt's own tool buttons can only elide.
void paintLargeRibbonButton(QToolButton *b)
{
    QPainter p(b);
    p.setRenderHint(QPainter::Antialiasing);
    const QRectF r = QRectF(b->rect()).adjusted(1, 1, -1, -1);
    const bool down = b->isDown() || (b->menu() && b->menu()->isVisible()), on = b->isChecked(), hover = b->underMouse() && b->isEnabled();
    const QColor accent = uiAccent();
    if (on) {
        QColor bg = accent;
        bg.setAlphaF(dark() ? 0.30f : 0.14f);
        p.setPen(Qt::NoPen);
        p.setBrush(bg);
        p.drawRoundedRect(r, 7, 7);
    } else if (down || hover) {
        p.setPen(Qt::NoPen);
        p.setBrush(dark() ? QColor(255, 255, 255, down ? 34 : 20) : QColor(0, 0, 0, down ? 22 : 11));
        p.drawRoundedRect(r, 7, 7);
    }
    const QSize is = b->iconSize();
    const QRect ir(int(r.center().x() - is.width() / 2.0), int(r.top() + 4), is.width(), is.height());
    b->icon().paint(&p, ir, Qt::AlignCenter, b->isEnabled() ? QIcon::Normal : QIcon::Disabled);
    const QColor tc = b->isEnabled() ? uiText() : uiHighContrast() ? QApplication::palette().color(QPalette::Disabled, QPalette::WindowText) : QColor(0x9C, 0xA3, 0xAF);
    p.setPen(tc);
    p.setFont(b->font());
    const QFontMetrics fm(b->font());
    const bool menu = b->menu() || b->popupMode() == QToolButton::InstantPopup;
    const QStringList lines = largeLabelLines(b);
    auto chevron = [&](QPointF c) {
        p.setPen(QPen(tc, 1.3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.drawPolyline(QPolygonF({c + QPointF(-3.5, -1.5), c + QPointF(0, 2), c + QPointF(3.5, -1.5)}));
    };
    int y = ir.bottom() + 4;
    for (int i = 0; i < lines.size(); ++i) {
        const bool inlineChevron = menu && i == 1;
        const int avail = int(r.width()) - 4 - (inlineChevron ? 12 : 0);
        const QString text = fm.elidedText(lines[i], Qt::ElideRight, avail);
        if (inlineChevron) {
            // Center the text and its chevron together.
            const qreal tw = fm.horizontalAdvance(text), total = tw + 12;
            const qreal x = r.center().x() - total / 2;
            p.setPen(tc);
            p.drawText(QRectF(x, y, tw + 1, fm.height()), Qt::AlignLeft | Qt::AlignTop, text);
            chevron(QPointF(x + tw + 7.5, y + fm.ascent() - fm.xHeight() / 2.0));
        } else {
            p.setPen(tc);
            p.drawText(QRect(int(r.left()), y, int(r.width()), fm.height()), Qt::AlignHCenter | Qt::AlignTop, text);
        }
        y += fm.height() - 1;
    }
    if (menu && lines.size() < 2) chevron(QPointF(r.center().x(), std::min<double>(y + 5, r.bottom() - 4)));
}

namespace {
class LargeButton : public QToolButton {
public:
    using QToolButton::QToolButton;
    QSize sizeHint() const override { return largeRibbonButtonHint(this); }
    QSize minimumSizeHint() const override { return largeRibbonButtonHint(this); }

protected:
    void paintEvent(QPaintEvent *) override { paintLargeRibbonButton(this); }
    bool event(QEvent *e) override
    {
        if (e->type() == QEvent::HoverEnter || e->type() == QEvent::HoverLeave) update();
        return QToolButton::event(e);
    }
};
} // namespace

namespace {
// A small toggle (a switch with its words): icon and text start at the left, so a column of them reads straight and the buttons of the column can be one width.
class SmallToggle : public QToolButton {
public:
    using QToolButton::QToolButton;

protected:
    void paintEvent(QPaintEvent *e) override
    {
        if (menu() || popupMode() != QToolButton::DelayedPopup || toolButtonStyle() != Qt::ToolButtonTextBesideIcon) {
            QToolButton::paintEvent(e);
            return;
        }
        QStyleOptionToolButton opt;
        initStyleOption(&opt);
        QStylePainter p(this);
        QStyleOptionToolButton panel = opt;
        panel.features &= ~QStyleOptionToolButton::HasMenu;
        p.drawPrimitive(QStyle::PE_PanelButtonTool, panel);
        // (laid out from the leading edge: the left for left-to-right text, the right for right-to-left)
        const Qt::LayoutDirection dir = layoutDirection();
        const QRect r = opt.rect.adjusted(5, 0, -5, 0);
        const QSize is = iconSize();
        const QIcon::Mode mode = isEnabled() ? QIcon::Normal : QIcon::Disabled;
        int lead = 0;   // how far from the leading edge the text starts
        if (!icon().isNull()) {
            icon().paint(&p, QStyle::visualRect(dir, r, QRect(r.left(), (height() - is.height()) / 2, is.width(), is.height())), Qt::AlignCenter, mode, isChecked() ? QIcon::On : QIcon::Off);
            lead = is.width() + 4;
        }
        p.setPen(palette().color(isEnabled() ? QPalette::Active : QPalette::Disabled, QPalette::ButtonText));
        QString text = opt.text;
        text.replace(QStringLiteral("&&"), QStringLiteral("\x01")).remove(QLatin1Char('&')).replace(QLatin1Char('\x01'), QLatin1Char('&'));
        const QRect area(r.left() + lead, 0, r.width() - lead, height());
        p.drawText(QStyle::visualRect(dir, r, area), QStyle::visualAlignment(dir, Qt::AlignLeft) | Qt::AlignVCenter, fontMetrics().elidedText(text, dir == Qt::RightToLeft ? Qt::ElideLeft : Qt::ElideRight, area.width()));
        if (hasFocus()) {
            QStyleOptionFocusRect fo;
            fo.initFrom(this);
            fo.rect = rect().adjusted(1, 1, -1, -1);
            p.drawPrimitive(QStyle::PE_FrameFocusRect, fo);
        }
    }
};
} // namespace

bool isRibbonToggle(const QToolButton *b) { return dynamic_cast<const SmallToggle *>(b) != nullptr; }

QToolButton *ribbonButton(QAction *a, bool large, QWidget *parent)
{
    auto *b = large ? new LargeButton(parent) : !large && a && a->isCheckable() ? new SmallToggle(parent) : new QToolButton(parent);
    if (large) b->setAttribute(Qt::WA_Hover);
    b->setDefaultAction(a);
    b->setAutoRaise(true);
    // The keyboard reaches it (Tab, the arrow keys, KeyTips); a click
    // leaves the focus where it was, in the text being edited.
    b->setFocusPolicy(Qt::TabFocus);
    if (large) {
        b->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        b->setIconSize(QSize(kLargeIcon, kLargeIcon));
        b->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        b->setProperty("ribbonLarge", true);
    } else {
        b->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        b->setIconSize(QSize(16, 16));
        b->setProperty("ribbonRow", true);   // (its height follows the font: RibbonGroup::changeEvent)
        b->setFixedHeight(ribbonRowHeight(b->fontMetrics()));
    }
    return b;
}

namespace {
// A check box in a ribbon grid: the box alone (the heading above its column and the label at the left of its row name it), drawn as the other check boxes are.
class CheckButton : public QToolButton {
public:
    using QToolButton::QToolButton;
    QSize sizeHint() const override { return QSize(24, ribbonRowHeight(fontMetrics())); }
    QSize minimumSizeHint() const override { return sizeHint(); }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const QRectF box(width() / 2.0 - 7, height() / 2.0 - 7, 14, 14);
        const bool on = isChecked(), hover = underMouse() && isEnabled();
        const QColor accent = uiAccent();
        QColor line = on || hover ? accent : lineColor();
        if (!isEnabled()) line = lineColor();
        p.setPen(QPen(line, 1));
        p.setBrush(on ? (isEnabled() ? accent : lineColor()) : QBrush(ribbonBg()));
        p.drawRoundedRect(box.adjusted(0.5, 0.5, -0.5, -0.5), 4, 4);
        if (on) {
            p.setPen(QPen(Qt::white, 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            p.drawPolyline(QPolygonF({box.topLeft() + QPointF(3.5, 7.2), box.topLeft() + QPointF(6, 9.6), box.topLeft() + QPointF(10.5, 4.6)}));
        }
        if (hasFocus()) {
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(accent, 1.5));
            p.drawRoundedRect(box.adjusted(-2.5, -2.5, 2.5, 2.5), 6, 6);
        }
    }
    bool event(QEvent *e) override
    {
        if (e->type() == QEvent::HoverEnter || e->type() == QEvent::HoverLeave) update();
        return QToolButton::event(e);
    }
};
} // namespace

bool isRibbonCheck(const QToolButton *b) { return dynamic_cast<const CheckButton *>(b) != nullptr; }

QToolButton *ribbonCheck(QAction *a, QWidget *parent)
{
    auto *b = new CheckButton(parent);
    b->setDefaultAction(a);
    b->setToolButtonStyle(Qt::ToolButtonIconOnly);
    b->setAttribute(Qt::WA_Hover);
    b->setFocusPolicy(Qt::TabFocus);
    b->setProperty("ribbonRow", true);
    b->setFixedHeight(ribbonRowHeight(b->fontMetrics()));
    return b;
}

void setKeytip(QObject *w, const QString &k) { w->setProperty("keytip", k); }
QString keytip(const QObject *w) { return w->property("keytip").toString(); }

// ---------------- group ----------------
RibbonGroup::RibbonGroup(const QString &title, QWidget *parent) : QFrame(parent), m_title(title)
{
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(8, 19, 8, 4);   // title sits on top of the card
    outer->setSpacing(0);
    m_cols = new QHBoxLayout();
    m_cols->setSpacing(2);
    m_cols->setContentsMargins(0, 0, 0, 0);
    outer->addLayout(m_cols);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
}

QToolButton *RibbonGroup::addLarge(QAction *a, QMenu *menu, bool split)
{
    m_col = nullptr;
    QToolButton *b = ribbonButton(a, true, this);
    if (menu) {
        b->setMenu(menu);
        b->setPopupMode(split ? QToolButton::MenuButtonPopup : QToolButton::InstantPopup);
    }
    m_cols->addWidget(b);
    return b;
}

void RibbonGroup::beginColumn()
{
    m_col = new QVBoxLayout();
    m_col->setSpacing(1);
    m_col->setContentsMargins(0, 0, 0, 0);
    m_col->addStretch(1);
    m_cols->addLayout(m_col);
    m_rows = 0;
    m_rowLabels.clear();
    m_colToggles.clear();
}

QToolButton *RibbonGroup::addSmall(QAction *a, QMenu *menu, bool split, bool iconOnly)
{
    if (!m_col || m_rows >= 3) beginColumn();
    QToolButton *b = ribbonButton(a, false, this);
    if (iconOnly) b->setToolButtonStyle(Qt::ToolButtonIconOnly);
    if (menu) {
        b->setMenu(menu);
        b->setPopupMode(split ? QToolButton::MenuButtonPopup : QToolButton::InstantPopup);
    }
    m_col->insertWidget(m_col->count() - 1, b, 0, Qt::AlignLeft);
    ++m_rows;
    // The toggles of a column are one width (the longest), so their highlights line up.
    if (isRibbonToggle(b) && !menu && !iconOnly) {
        m_colToggles << b;
        int w = 0;
        for (QToolButton *t : std::as_const(m_colToggles)) w = std::max(w, t->sizeHint().width());
        for (QToolButton *t : std::as_const(m_colToggles)) t->setMinimumWidth(w);
    }
    return b;
}

// A number box in a row is as wide as the number it shows needs (and three digits more), at the font it is shown in: the style says how much room the box keeps around its text, and
// a box a caller gave a fixed width takes the new one, wider or narrower.
static void fitNumberBox(QWidget *w)
{
    auto *sb = qobject_cast<QAbstractSpinBox *>(w);
    if (!sb) return;
    sb->ensurePolished();
    QStyleOptionSpinBox opt;
    opt.initFrom(sb);
    opt.rect = QRect(0, 0, 400, ribbonRowHeight(sb->fontMetrics()));
    opt.frame = true;
    opt.buttonSymbols = sb->buttonSymbols();
    opt.subControls = QStyle::SC_All;
    const int around = 400 - sb->style()->subControlRect(QStyle::CC_SpinBox, &opt, QStyle::SC_SpinBoxEditField, sb).width();
    const QFontMetrics fm(sb->font());
    const int need = std::max(fm.horizontalAdvance(sb->text() + QStringLiteral("000")), fm.horizontalAdvance(QStringLiteral("0.000\""))) + around + (sb->findChild<QLineEdit *>() ? sb->findChild<QLineEdit *>()->textMargins().left() + sb->findChild<QLineEdit *>()->textMargins().right() : 0);
    if (sb->minimumWidth() == sb->maximumWidth() && sb->minimumWidth() > 0) {
        sb->setFixedWidth(need);
    } else {
        if (sb->minimumWidth() < need) sb->setMinimumWidth(need);
        if (sb->maximumWidth() < need) sb->setMaximumWidth(need);
    }
}

void RibbonGroup::addRow(const QList<QWidget *> &widgets)
{
    if (!m_col || m_rows >= 3) beginColumn();
    auto *row = new QHBoxLayout();
    row->setSpacing(1);
    row->setContentsMargins(0, 0, 0, 0);
    for (QWidget *w : widgets) {
        w->setParent(this);
        w->setProperty("ribbonRow", true);   // (the style sheet takes the padding off its fields)
        fitNumberBox(w);
        w->setFixedHeight(ribbonRowHeight(w->fontMetrics()));   // a row is as tall as a small button, so three fit the body
        row->addWidget(w);
    }
    row->addStretch(1);
    m_col->insertLayout(m_col->count() - 1, row);
    ++m_rows;
    // The labels at the start of a column's rows are as wide as the longest, so the fields beside them line up.
    if (auto *label = qobject_cast<QLabel *>(widgets.value(0))) {
        m_rowLabels << label;
        int w = 0;
        for (QLabel *l : std::as_const(m_rowLabels)) w = std::max(w, l->sizeHint().width());
        for (QLabel *l : std::as_const(m_rowLabels)) l->setMinimumWidth(w);
    }
}

void RibbonGroup::addGrid(const QList<QList<QWidget *>> &rows)
{
    auto *col = new QVBoxLayout();
    col->setSpacing(1);
    col->setContentsMargins(0, 0, 0, 0);
    auto *grid = new QGridLayout();
    grid->setHorizontalSpacing(6);
    grid->setVerticalSpacing(1);
    grid->setContentsMargins(0, 0, 0, 0);
    for (int r = 0; r < rows.size(); ++r)
        for (int c = 0; c < rows[r].size(); ++c) {
            QWidget *w = rows[r][c];
            w->setParent(this);
            w->setProperty("ribbonRow", true);
            fitNumberBox(w);
            w->setFixedHeight(ribbonRowHeight(w->fontMetrics()));
            // The first column names the rows, the first row the columns; check boxes and headings sit centered, fields fill their column.
            const bool field = qobject_cast<QComboBox *>(w) || qobject_cast<QAbstractSpinBox *>(w);
            const Qt::Alignment al = c == 0 ? Qt::AlignLeft | Qt::AlignVCenter : field ? Qt::Alignment() : Qt::AlignCenter;
            grid->addWidget(w, r, c, al);
        }
    col->addLayout(grid);
    col->addStretch(1);
    m_col = nullptr;
    m_cols->addLayout(col);
}

void RibbonGroup::addWidget(QWidget *w)
{
    m_col = nullptr;
    w->setParent(this);
    m_cols->addWidget(w);
}

void RibbonGroup::addSeparator()
{
    m_col = nullptr;
    auto *f = new QFrame(this);
    f->setObjectName(QStringLiteral("jpRibbonSeparator"));
    f->setFixedWidth(1);
    f->setStyleSheet(QStringLiteral("background:%1;").arg(lineColor().name()));
    m_cols->addSpacing(2);
    m_cols->addWidget(f);
    m_cols->addSpacing(2);
}

void RibbonGroup::setLauncher(const std::function<void()> &fn, const QString &tip)
{
    if (!m_launcher) {
        m_launcher = new QToolButton(this);
        m_launcher->setObjectName(QStringLiteral("launcher"));
        m_launcher->setAutoRaise(true);
        m_launcher->setFocusPolicy(Qt::TabFocus);
        m_launcher->setIcon(icon("ellipsis"));
        m_launcher->setIconSize(QSize(13, 13));
        m_launcher->setFixedSize(18, 15);
    }
    // plain: reviewed: the ribbon's own strings
    m_launcher->setToolTip(tip.isEmpty() ? tr("%1 Settings").arg(m_title) : tip);
    m_launcher->setAccessibleName(m_launcher->toolTip() + QStringLiteral("…"));
    m_launcher->setAccessibleDescription(tr("Opens the %1 dialog").arg(m_launcher->toolTip()));
    // Only our own click handler goes: cutting every connection from the
    // button also cut the one that tells Qt's screen reader support it's
    // gone, leaving a stale entry that crashed a later control's name.
    QObject::disconnect(m_launcherClicked);
    m_launcherClicked = connect(m_launcher, &QToolButton::clicked, this, [fn] { fn(); });
}

static void collectControls(const QLayout *l, QList<QWidget *> &out)
{
    for (int i = 0; i < l->count(); ++i) {
        QLayoutItem *it = l->itemAt(i);
        if (QWidget *w = it->widget()) out << w;
        else if (QLayout *sub = it->layout()) collectControls(sub, out);
    }
}

QList<QWidget *> RibbonGroup::controls() const
{
    QList<QWidget *> out;
    collectControls(layout(), out);
    return out;
}

QFont RibbonGroup::titleFont() const
{
    QFont f = font();
    f.setPointSizeF(f.pointSizeF() * 0.74);
    f.setWeight(QFont::DemiBold);
    f.setLetterSpacing(QFont::PercentageSpacing, 108);
    return f;
}

// The card is inset 3 on each side, and the title 9 from its left, beside
// room for the launcher (paintEvent).
int RibbonGroup::titleWidth() const
{
    return QFontMetrics(titleFont()).horizontalAdvance(m_title.toUpper()) + 6 + (m_launcher ? 30 : 18) + 1;
}

QSize RibbonGroup::sizeHint() const { return QFrame::sizeHint().expandedTo(QSize(titleWidth(), 0)); }
QSize RibbonGroup::minimumSizeHint() const { return QFrame::minimumSizeHint().expandedTo(QSize(titleWidth(), 0)); }

void RibbonGroup::paintEvent(QPaintEvent *e)
{
    QFrame::paintEvent(e);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    // A soft rounded card per group, titled at the top left in small capitals.
    const QRectF card = QRectF(rect()).adjusted(3, 3, -3, -2);
    p.setPen(uiHighContrast() ? QPen(lineColor(), 1) : Qt::NoPen);
    p.setBrush(uiHighContrast() ? QBrush(Qt::NoBrush) : dark() ? QBrush(QColor(255, 255, 255, 10)) : QBrush(QColor(0x1E, 0x29, 0x3B, 9)));
    p.drawRoundedRect(card, 9, 9);
    const QFont f = titleFont();
    p.setFont(f);
    p.setPen(mutedText());
    const QRectF label(card.left() + 9, card.top() + 3, card.width() - (m_launcher ? 30 : 18), 13);
    p.drawText(label, Qt::AlignLeft | Qt::AlignVCenter, QFontMetrics(f).elidedText(m_title.toUpper(), Qt::ElideRight, int(label.width())));
    if (m_launcher) m_launcher->move(int(card.right()) - 20, int(card.top()) + 2);
}

void RibbonGroup::changeEvent(QEvent *e)
{
    QFrame::changeEvent(e);
    if (e->type() != QEvent::FontChange) return;
    for (QWidget *c : controls())
        if (c->property("ribbonRow").toBool()) {
            c->setFixedHeight(ribbonRowHeight(c->fontMetrics()));
            c->updateGeometry();
        }
}

// ---------------- tab ----------------
RibbonTab::RibbonTab(QWidget *parent) : QWidget(parent)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(6, 3, 6, 3);
    m_layout->setSpacing(4);
}

RibbonGroup *RibbonTab::addGroup(const QString &title)
{
    auto *g = new RibbonGroup(title, this);
    // (a group added to a finished tab goes before its closing stretch)
    const int at = m_finished ? m_layout->count() - 1 : -1;
    m_layout->insertWidget(at, g);
    // Its collapsed form: one large button with the group's name.
    auto *a = new QAction(title, this);
    auto *b = ribbonButton(a, true, this);
    b->setPopupMode(QToolButton::InstantPopup);
    b->setMenu(new QMenu(b));   // shows the chevron; the dropdown itself is our own popup
    connect(b->menu(), &QMenu::aboutToShow, this, [this, g, b] {
        QTimer::singleShot(0, b->menu(), &QMenu::close);
        QTimer::singleShot(0, this, [this, g] { popUp(int(m_groups.indexOf(g))); });
    });
    b->hide();
    m_layout->insertWidget(at < 0 ? -1 : at + 1, b);
    m_groups << g;
    m_buttons << b;
    m_collapsed << false;
    return g;
}

void RibbonTab::removeGroup(RibbonGroup *g)
{
    const int i = int(m_groups.indexOf(g));
    if (i < 0 || m_popped >= 0) return;
    QToolButton *b = m_buttons[i];
    m_groups.remove(i);
    m_buttons.remove(i);
    m_collapsed.remove(i);
    if (QAction *a = b->defaultAction()) a->deleteLater();
    delete b;
    delete g;
    relayout();
}

void RibbonGroup::setUserHidden(bool hidden)
{
    if (m_userHidden == hidden) return;
    m_userHidden = hidden;
    if (hidden) setVisible(false);   // (its tab shows it again, or lays the others out, at its next layout)
}

void RibbonTab::ensureBuilt()
{
    if (m_attach) {
        const auto attach = std::move(m_attach);
        m_attach = nullptr;
        attach();
    }
    if (!m_builder) return;
    const auto fn = std::move(m_builder);
    m_builder = nullptr;
    fn(this);
    Q_EMIT built();
}

void RibbonTab::settle()
{
    if (QWidget *top = window())
        if (QLayout *l = top->layout()) l->activate();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    if (QWidget *view = parentWidget()) {
        if (QLayout *l = view->layout()) l->activate();
        if (QWidget *scroll = view->parentWidget()) QCoreApplication::sendPostedEvents(scroll, QEvent::LayoutRequest);
    }
    relayout();
}

void RibbonTab::finish()
{
    if (!m_finished) m_layout->addStretch(1);
    m_finished = true;
    // The collapsed button shows the group's first icon.
    for (int i = 0; i < m_groups.size(); ++i) {
        QIcon ic;
        for (QToolButton *tb : m_groups[i]->findChildren<QToolButton *>())
            if (!tb->icon().isNull() && !tb->text().isEmpty() && tb->toolButtonStyle() != Qt::ToolButtonIconOnly) { ic = tb->icon(); break; }
        if (QAction *a = m_buttons[i]->defaultAction()) a->setIcon(ic.isNull() ? icon("layout-grid") : ic);
    }
}

QSize RibbonTab::minimumSizeHint() const { return QSize(120, QWidget::minimumSizeHint().height()); }

int RibbonTab::collapsedCount() const { return int(std::count(m_collapsed.begin(), m_collapsed.end(), true)); }

void RibbonTab::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    relayout();
}

void RibbonTab::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    relayout();
}

void RibbonTab::relayout()
{
    if (m_inRelayout || m_groups.isEmpty()) return;
    m_inRelayout = true;
    const QMargins mg = m_layout->contentsMargins();
    int shown = 0;
    for (const RibbonGroup *g : std::as_const(m_groups)) shown += !g->userHidden();
    const int avail = width() - mg.left() - mg.right() - m_layout->spacing() * int(std::max(0, shown - 1));   // (the gaps between the groups too)
    QVector<int> full(m_groups.size()), small(m_groups.size());
    int total = 0;
    for (int i = 0; i < m_groups.size(); ++i) {
        if (m_groups[i]->userHidden()) continue;   // (no room, and nothing to fold)
        full[i] = m_groups[i]->sizeHint().width();
        small[i] = m_buttons[i]->sizeHint().width() + 8;
        total += full[i];
    }
    QVector<bool> col(m_groups.size(), false);
    for (int i = int(m_groups.size()) - 1; i >= 0 && total > avail; --i) {
        if (m_groups[i]->userHidden()) continue;
        total += small[i] - full[i];
        col[i] = true;
    }
    for (int i = 0; i < m_groups.size(); ++i) {
        const bool gone = m_groups[i]->userHidden();
        m_collapsed[i] = col[i];
        if (i != m_popped) m_groups[i]->setVisible(!col[i] && !gone);
        m_buttons[i]->setVisible(col[i] && !gone);
    }
    m_inRelayout = false;
}

void RibbonTab::popUp(int i)
{
    if (i < 0 || i >= m_groups.size() || m_popped >= 0) return;
    RibbonGroup *g = m_groups[i];
    QToolButton *b = m_buttons[i];
    auto *pop = new QFrame(this, Qt::Popup);
    pop->setAttribute(Qt::WA_DeleteOnClose);
    pop->setObjectName(QStringLiteral("jpRibbonPop"));
    pop->setStyleSheet(QStringLiteral("#jpRibbonPop{background:%1; border:1px solid %2; border-radius:8px;}").arg(ribbonBg().name(), lineColor().name()));
    auto *pl = new QVBoxLayout(pop);
    pl->setContentsMargins(6, 6, 6, 4);
    m_popped = i;
    g->setParent(pop);
    pl->addWidget(g);
    g->show();
    // A command chosen in the dropdown closes it.
    for (QToolButton *tb : g->findChildren<QToolButton *>())
        if (!tb->menu() && !isRibbonCheck(tb)) connect(tb, &QToolButton::clicked, pop, &QWidget::close);   // (a check box is ticked, not chosen: the dropdown stays)
    connect(pop, &QObject::destroyed, this, [this, g, b] {
        // Put the group back beside its button, hidden while collapsed.
        g->setParent(this);
        m_layout->insertWidget(m_layout->indexOf(b), g);
        m_popped = -1;
        relayout();
    });
    pop->adjustSize();
    QPoint at = b->mapToGlobal(QPoint(0, b->height()));
    pop->move(at);
    pop->show();
}

// ---------------- header ----------------
// One control in the ribbon's top row: the File button, a tab, or the
// chevron that collapses the ribbon, or Help's "?". Drawn as pills, but real controls:
// the keyboard reaches them and screen readers see them. Only the current
// tab takes the focus with Tab; the arrow keys move along the tabs and
// switch to each (as tab lists do).
class HeaderButton : public QAbstractButton {
public:
    enum Kind { File, Tab, Collapse, Help };
    HeaderButton(Kind k, Ribbon *r, int tab, QWidget *parent) : QAbstractButton(parent), m_kind(k), m_r(r), m_tab(tab)
    {
        setAttribute(Qt::WA_Hover);
        setFocusPolicy(k == Tab ? Qt::NoFocus : Qt::TabFocus);
        setObjectName(k == File ? QStringLiteral("jpRibbonFile") : k == Tab ? QStringLiteral("jpRibbonTab") : k == Help ? QStringLiteral("jpRibbonHelp") : QStringLiteral("jpRibbonCollapse"));
        if (k == Tab) setProperty("jpOwnArrows", true);   // they move along the tabs
        if (k == File) {
            setText(QCoreApplication::translate("Ribbon", "File"));
            setAccessibleDescription(QCoreApplication::translate("Ribbon", "Opens the File page: new, open, save, print, share, export, and options."));
            // plain: reviewed: the control's own strings
            setWhatsThis(accessibleDescription());
        }
        connect(this, &QAbstractButton::clicked, this, [this] {
            if (m_kind == File) Q_EMIT m_r->fileClicked();
            else if (m_kind == Collapse) m_r->setMinimized(!m_r->m_minimized);
            else if (m_kind == Help) Q_EMIT m_r->helpClicked();
            else {
                if (m_r->m_minimized) m_r->setMinimized(false);
                m_r->showTab(m_r->m_tabs[m_tab].page);
            }
        });
    }
    Kind kind() const { return m_kind; }
    int tab() const { return m_tab; }
    void setTab(int t) { m_tab = t; }
    bool isCurrent() const { return m_kind == Tab && m_tab == m_r->m_current && !m_r->m_minimized; }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
        const bool hot = underMouse();
        if (m_kind == File) {
            // One pill holding the app's badge, the label and a chevron, so it
            // reads as a single control that opens the File screen.
            QColor bg = uiAccent();
            bg.setAlphaF(dark() ? (hot ? 0.34f : 0.22f) : (hot ? 0.18f : 0.10f));
            QColor edge = uiAccent();
            edge.setAlphaF(dark() ? 0.55f : 0.35f);
            p.setPen(QPen(edge, 1));
            p.setBrush(bg);
            p.drawRoundedRect(r, r.height() / 2, r.height() / 2);
            const QRectF badge(r.left() + 4, r.center().y() - 9, 18, 18);
            p.setPen(Qt::NoPen);
            p.setBrush(uiAccent());
            p.drawEllipse(badge);
            QFont bf = font();
            bf.setBold(true);
            bf.setPixelSize(9);
            p.setFont(bf);
            p.setPen(uiHighContrast() ? palette().color(QPalette::HighlightedText) : QColor(Qt::white));
            p.drawText(badge, Qt::AlignCenter, appInfo().badge);
            QFont lf = font();
            lf.setWeight(QFont::DemiBold);
            p.setFont(lf);
            const QColor fg = uiHighContrast() ? uiText() : dark() ? uiAccent().lighter(140) : uiAccent();
            p.setPen(fg);
            p.drawText(QRectF(badge.right() + 7, r.top(), r.right() - badge.right() - 25, r.height()), Qt::AlignVCenter | Qt::AlignLeft, text());
            const QPointF c(r.right() - 12, r.center().y() + 0.5);
            p.setPen(QPen(fg, 1.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            p.drawPolyline(QPolygonF({c + QPointF(-3, -1.5), c + QPointF(0, 1.5), c + QPointF(3, -1.5)}));
            return;
        }
        if (m_kind == Help) {
            if (hot) {
                p.setPen(Qt::NoPen);
                p.setBrush(dark() ? QColor(255, 255, 255, 18) : QColor(0, 0, 0, 12));
                p.drawRoundedRect(r, 6, 6);
            }
            const QRectF c(width() / 2.0 - 8, height() / 2.0 - 8, 16, 16);
            p.setPen(QPen(mutedText(), 1.4));
            p.setBrush(Qt::NoBrush);
            p.drawEllipse(c);
            QFont qf = font();
            qf.setBold(true);
            qf.setPixelSize(11);
            p.setFont(qf);
            p.drawText(c, Qt::AlignCenter, QStringLiteral("?"));
            return;
        }
        if (m_kind == Collapse) {
            if (hot) {
                p.setPen(Qt::NoPen);
                p.setBrush(dark() ? QColor(255, 255, 255, 18) : QColor(0, 0, 0, 12));
                p.drawRoundedRect(r, 6, 6);
            }
            p.setPen(QPen(mutedText(), 1.5));
            const QRect cr(width() / 2 - 8, height() / 2 - 8, 16, 16);
            if (m_r->m_minimized) { p.drawLine(cr.left() + 3, cr.top() + 6, cr.center().x(), cr.top() + 11); p.drawLine(cr.center().x(), cr.top() + 11, cr.right() - 3, cr.top() + 6); }
            else { p.drawLine(cr.left() + 3, cr.top() + 11, cr.center().x(), cr.top() + 6); p.drawLine(cr.center().x(), cr.top() + 6, cr.right() - 3, cr.top() + 11); }
            return;
        }
        // Tabs are pills: the current one filled, others lit on hover.
        // Contextual tabs (Text Box, Picture...) carry a colored dot.
        const auto &t = m_r->m_tabs[m_tab];
        const bool cur = isCurrent();
        const QColor base = t.group.isEmpty() || uiHighContrast() ? uiAccent() : t.color;
        if (cur) {
            QColor bg = base;
            bg.setAlphaF(uiHighContrast() ? 1.0f : dark() ? 0.30f : 0.13f);
            p.setPen(Qt::NoPen);
            p.setBrush(bg);
            p.drawRoundedRect(r, r.height() / 2, r.height() / 2);
        } else if (hot) {
            p.setPen(uiHighContrast() ? QPen(uiText(), 1) : Qt::NoPen);
            p.setBrush(uiHighContrast() ? Qt::NoBrush : dark() ? QBrush(QColor(255, 255, 255, 18)) : QBrush(QColor(0, 0, 0, 12)));
            p.drawRoundedRect(r, r.height() / 2, r.height() / 2);
        }
        QRectF tr = r;
        if (!t.group.isEmpty()) {
            p.setPen(Qt::NoPen);
            p.setBrush(uiHighContrast() ? (cur ? palette().color(QPalette::HighlightedText) : uiText()) : t.color);
            p.drawEllipse(QPointF(r.left() + 13, r.center().y()), 3.5, 3.5);
            tr.setLeft(r.left() + 12);
        }
        QFont tf = font();
        if (cur) tf.setWeight(QFont::DemiBold);
        p.setFont(tf);
        if (uiHighContrast()) p.setPen(cur ? palette().color(QPalette::HighlightedText) : uiText());
        else p.setPen(cur ? (dark() ? base.lighter(140) : base.darker(t.group.isEmpty() ? 100 : 130)) : uiText());
        p.drawText(tr, Qt::AlignCenter, t.title);
    }

    void mouseDoubleClickEvent(QMouseEvent *e) override
    {
        if (m_kind == Tab) m_r->setMinimized(!m_r->m_minimized);
        else QAbstractButton::mouseDoubleClickEvent(e);
    }

    void keyPressEvent(QKeyEvent *e) override
    {
        if (m_kind != Tab || (e->modifiers() & ~Qt::KeypadModifier)) return QAbstractButton::keyPressEvent(e);
        switch (e->key()) {
        case Qt::Key_Left:
        case Qt::Key_Right: {
            const int dir = e->key() == Qt::Key_Right ? 1 : -1;
            for (int i = m_tab + dir; i >= 0 && i < m_r->m_tabs.size(); i += dir)
                if (m_r->m_tabs[i].visible && !m_r->m_tabs[i].userHidden) {
                    m_r->showTab(m_r->m_tabs[i].page);
                    if (QWidget *b = m_r->tabButton(i)) b->setFocus(Qt::TabFocusReason);
                    return;
                }
            // Past the ends: the File button, or the More button (when tabs overflowed) or the collapse chevron.
            if (dir > 0)
                if (QWidget *more = m_r->moreButton()) { more->setFocus(Qt::OtherFocusReason); return; }
            if (QWidget *w = m_r->findChild<QWidget *>(dir < 0 ? QStringLiteral("jpRibbonFile") : QStringLiteral("jpRibbonCollapse"))) w->setFocus(Qt::TabFocusReason);
            return;
        }
        case Qt::Key_Down: {
            // Into the tab's controls, the nearest below.
            QList<QWidget *> controls;
            if (RibbonTab *page = m_r->current())
                for (QWidget *w : page->findChildren<QWidget *>())
                    if (w->isVisible() && w->isEnabled() && (w->focusPolicy() & Qt::TabFocus)) controls << w;
            if (QWidget *w = nearestInDirection(this, Qt::Key_Down, controls)) w->setFocus(Qt::TabFocusReason);
            return;
        }
        case Qt::Key_Return:
        case Qt::Key_Enter:
        case Qt::Key_Space:
            click();
            return;
        default:
            QAbstractButton::keyPressEvent(e);
        }
    }

private:
    Kind m_kind;
    Ribbon *m_r;
    int m_tab;
};

// The top row: the quick toolbar, the File button, the tabs and the
// collapse chevron as controls, with the brand mark drawn behind them.
class RibbonHeader : public QWidget {
public:
    explicit RibbonHeader(Ribbon *r) : QWidget(r), m_r(r)
    {
        setObjectName(QStringLiteral("jpRibbonTabs"));
        setAccessibleName(QCoreApplication::translate("Ribbon", "Ribbon tabs"));
        updateMetrics();
        m_file = new HeaderButton(HeaderButton::File, r, -1, this);
        m_collapse = new HeaderButton(HeaderButton::Collapse, r, -1, this);
        m_help = new HeaderButton(HeaderButton::Help, r, -1, this);
        m_help->setAccessibleName(QCoreApplication::translate("Ribbon", "Help"));
        m_help->setAccessibleDescription(QCoreApplication::translate("Ribbon", "Opens Help: how to do things in %1, and a search that finds commands too.").arg(appInfo().name));
        // plain: reviewed: the ribbon's own strings
        m_help->setWhatsThis(m_help->accessibleDescription());
        m_help->setToolTip(QCoreApplication::translate("Ribbon", "Help (F1)"));
        m_more = new QToolButton(this);
        m_more->setObjectName(QStringLiteral("jpRibbonMore"));
        m_more->setAutoRaise(true);
        m_more->setFocusPolicy(Qt::TabFocus);   // the keyboard reaches it (Tab, the arrow keys, KeyTips)
        m_more->setIcon(icon("chevron-down"));
        m_more->setPopupMode(QToolButton::InstantPopup);
        m_more->setStyleSheet(QStringLiteral("QToolButton::menu-indicator { image: none; width: 0px; }"));   // (the chevron is the whole picture)
        m_more->setMenu(new QMenu(m_more));
        m_more->setToolTip(QCoreApplication::translate("Ribbon", "More tabs"));
        m_more->setAccessibleName(QCoreApplication::translate("Ribbon", "More tabs"));
        setKeytip(m_more, QStringLiteral("0"));
        connect(m_more->menu(), &QMenu::aboutToHide, this, [this] { QTimer::singleShot(0, this, [this] { layoutButtons(); }); });   // (a change that came while it was open)
        m_more->hide();
    }
    // Where the program's name and the four dots at the right of the top row go: as wide as the name needs at this font (the row's tabs leave this corner free).
    struct Brand {
        QFont font;
        QRect text, dots;
    };
    Brand brand() const
    {
        Brand b;
        b.font = font();
        b.font.setBold(true);
        b.font.setPointSizeF(b.font.pointSizeF() * 0.85);
        const QFontMetrics bm(b.font);
        const int nameW = std::max(74, bm.horizontalAdvance(appInfo().name) + 6);
        b.text = QRect(width() - nameW - 6, 1, nameW, std::max(16, bm.height()));
        b.dots = QRect(b.text.left() - 43, 6, 31, 6);
        return b;
    }
    // The height of the pills (tabs, File, Help, the chevrons) and of the row: 26 and 50 for the usual font, more for a larger one.
    int pillHeight() const { return std::max(26, fontMetrics().height() + 10); }
    // The pills sit below the brand's line (19 down for the usual font, lower for a larger one).
    int top() const { return std::max(19, QFontMetrics(brand().font).height() + 3); }
    void updateMetrics() { setFixedHeight(std::max(50, top() + pillHeight() + 5)); }
    QVector<QToolButton *> qatButtons;
    QVector<HeaderButton *> tabButtons;
    HeaderButton *m_file, *m_collapse, *m_help;
    QToolButton *m_more = nullptr;   // the tabs that don't fit the row
    QStringList m_moreShown;         // the tabs its menu lists now (pages, as text)

    int qatWidth() const
    {
        int w = 6;
        for (auto *b : qatButtons) w += b->width() + 2;
        return w + 8;
    }
    void layoutQat()
    {
        int x = 6;
        for (auto *b : qatButtons) {
            b->setGeometry(x, top() - 1, 26, 26);
            x += 28;
        }
        layoutButtons();
    }
    void addTabButton(int i)
    {
        auto *b = new HeaderButton(HeaderButton::Tab, m_r, i, this);
        const auto &t = m_r->m_tabs[i];
        b->setText(t.title);
        b->setAccessibleName(t.title);
        tabButtons << b;
        b->show();
        layoutButtons();
    }
    // The tabs the More menu lists: those that don't fit the row (not while it is open: it is made again when it closes).
    void fillMoreMenu()
    {
        QMenu *menu = m_more->menu();
        if (menu->isVisible()) return;
        menu->clear();
        for (HeaderButton *b : tabButtons) {
            const auto &t = m_r->m_tabs[b->tab()];
            if (!t.visible || t.userHidden || !m_moreShown.contains(QString::number(reinterpret_cast<quintptr>(t.page)))) continue;
            QString title = t.title;
            menu->addAction(title.replace(QLatin1Char('&'), QStringLiteral("&&")), m_r, [r = m_r, page = t.page] { r->showTab(page); });   // (no mnemonic: the & is the word's)
        }
    }
    // Places the File button, the visible tabs and the chevron; the current
    // tab is the one Tab reaches, and every tab button says which it is.
    void layoutButtons()
    {
        QFontMetrics fm(font());
        updateMetrics();
        const int pillH = pillHeight();
        int x = qatWidth();
        m_file->setGeometry(x + 2, top(), fm.horizontalAdvance(m_file->text()) + 58, pillH);
        x += 6 + m_file->width() + 4;
        // The tabs sit in the room between File and Help. Each is its text with 26 of padding (not under 14, so the labels stay apart); when they still don't fit, the last ones go into
        // a "more tabs" menu (the current tab always stays in the row).
        QVector<HeaderButton *> shown;
        int text = 0;
        for (HeaderButton *b : tabButtons) {
            const auto &t = m_r->m_tabs[b->tab()];
            const QString description = t.group.isEmpty() ? QCoreApplication::translate("Ribbon", "Ribbon tab") : QCoreApplication::translate("Ribbon", "Ribbon tab, %1").arg(t.group);
            if (b->accessibleDescription() != description) {
                b->setAccessibleDescription(description);
                // plain: reviewed: the ribbon's own strings
                b->setWhatsThis(description);   // Windows' screen readers read this one
            }
            if (b->text() != t.title) {
                b->setText(t.title);
                b->setAccessibleName(t.title);
            }
            b->setFocusPolicy(b->tab() == m_r->m_current ? Qt::TabFocus : Qt::NoFocus);
            if (!t.visible || t.userHidden) continue;
            shown << b;
            text += fm.horizontalAdvance(t.title) + (t.group.isEmpty() ? 0 : 12) + 4;
        }
        const int room = width() - 64 - x;
        const int pad = shown.isEmpty() ? 26 : std::clamp((room - text) / int(shown.size()), 14, 26);
        auto widthOf = [&](HeaderButton *b) { return fm.horizontalAdvance(m_r->m_tabs[b->tab()].title) + pad + (m_r->m_tabs[b->tab()].group.isEmpty() ? 0 : 12) + 4; };
        int total = 0;
        for (HeaderButton *b : shown) total += widthOf(b);
        QVector<HeaderButton *> hidden;
        if (total > room) {
            const int moreW = 32;
            HeaderButton *current = nullptr;
            for (HeaderButton *b : shown)
                if (b->tab() == m_r->m_current) current = b;
            int used = current ? widthOf(current) : 0;
            QVector<HeaderButton *> keep;
            for (HeaderButton *b : shown) {
                if (b == current) { keep << b; continue; }
                if (used + widthOf(b) <= room - moreW) { keep << b; used += widthOf(b); }
                else hidden << b;
            }
            shown = keep;
        }
        // A button is shown or hidden only when that changes (no flicker as the window is dragged wider or narrower).
        for (HeaderButton *b : tabButtons) {
            const bool want = shown.contains(b);
            if (b->isHidden() == want) b->setVisible(want);
        }
        QWidget *last = m_file;
        for (HeaderButton *b : shown) {
            const int w = widthOf(b) - 4;
            const QRect g(x, top(), w, pillH);
            if (b->geometry() != g) b->setGeometry(g);
            x += w + 4;
            QWidget::setTabOrder(last, b);
            last = b;
        }
        if (m_more->isHidden() == !hidden.isEmpty()) m_more->setVisible(!hidden.isEmpty());
        if (!hidden.isEmpty()) {
            m_more->setGeometry(x, top(), 28, pillH);
            QWidget::setTabOrder(last, m_more);   // (the keyboard goes on from the last tab to it)
            QStringList pages;
            for (HeaderButton *b : hidden) pages << QString::number(reinterpret_cast<quintptr>(m_r->m_tabs[b->tab()].page));
            // The menu is made again only when the tabs in it change, and never at once: the change may come from the menu's own action.
            if (pages != m_moreShown) {
                m_moreShown = pages;
                QPointer<RibbonHeader> self(this);
                QTimer::singleShot(0, this, [self] { if (self) self->fillMoreMenu(); });
            }
        } else {
            m_moreShown.clear();
        }
        m_collapse->setGeometry(width() - 30, top() - 1, 28, pillH);
        m_help->setGeometry(width() - 60, top() - 1, 28, pillH);
        m_collapse->setToolTip(m_r->m_minimized ? QCoreApplication::translate("Ribbon", "Expand the Ribbon (Ctrl+F1)") : QCoreApplication::translate("Ribbon", "Collapse the Ribbon (Ctrl+F1)"));
        m_collapse->setAccessibleName(m_r->m_minimized ? QCoreApplication::translate("Ribbon", "Expand the Ribbon") : QCoreApplication::translate("Ribbon", "Collapse the Ribbon"));
        m_collapse->setAccessibleDescription(m_r->m_minimized ? QCoreApplication::translate("Ribbon", "Shows the ribbon's commands again.")
                                                              : QCoreApplication::translate("Ribbon", "Shows only the tab names, leaving more room for the page."));
        // plain: reviewed: the ribbon's own strings
        m_collapse->setWhatsThis(m_collapse->accessibleDescription());
        m_collapse->update();
        m_file->update();
    }

protected:
    void resizeEvent(QResizeEvent *) override { layoutButtons(); }

    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(), headerBg());
        // Brand mark: four CMYK dots, and the program's name.
        const Brand br = brand();
        const QColor dots[4] = {QColor(0, 163, 224), QColor(229, 0, 126), QColor(255, 212, 0), dark() ? QColor(220, 220, 220) : QColor(28, 35, 48)};
        for (int i = 0; i < 4; ++i) {
            p.setPen(Qt::NoPen);
            p.setBrush(dots[i]);
            p.drawEllipse(QPointF(br.dots.left() + 3 + i * 9, 9), 3, 3);
        }
        p.setFont(br.font);
        p.setPen(mutedText());
        p.drawText(br.text, Qt::AlignLeft | Qt::AlignVCenter, appInfo().name);
        p.setPen(lineColor());
        p.drawLine(0, height() - 1, width(), height() - 1);
    }

    void wheelEvent(QWheelEvent *e) override
    {
        // The mouse wheel scrolls through tabs.
        int i = m_r->m_current;
        const int n = m_r->m_tabs.size();
        for (int k = 0; k < n; ++k) {
            i = (i + (e->angleDelta().y() < 0 ? 1 : n - 1)) % n;
            if (m_r->m_tabs[i].visible && !m_r->m_tabs[i].userHidden) break;
        }
        m_r->showTab(m_r->m_tabs[i].page);
    }

private:
    Ribbon *m_r;
};

// ---------------- ribbon ----------------
namespace {
// The tabs' pages, one showing. Its size is that of the page showing: a stacked layout asks every page, which polished and measured the controls of every tab at
// the first show (the pages are all one height, and the width of the ribbon is the window's).
class RibbonBody : public QStackedWidget {
public:
    explicit RibbonBody(QWidget *parent) : QStackedWidget(parent) { connect(this, &QStackedWidget::currentChanged, this, [this] { updateGeometry(); }); }
    QSize sizeHint() const override { return currentWidget() ? currentWidget()->sizeHint() : QSize(); }
    QSize minimumSizeHint() const override { return currentWidget() ? currentWidget()->minimumSizeHint() : QSize(); }
};
} // namespace

Ribbon::Ribbon(QWidget *parent) : QWidget(parent)
{
    auto *v = new QVBoxLayout(this);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(0);
    m_header = new RibbonHeader(this);
    m_stack = new RibbonBody(this);
    m_pending = new QWidget(this);
    m_pending->hide();
    updateBodyHeight();
    v->addWidget(m_header);
    v->addWidget(m_stack);
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, ribbonBg());
    setPalette(pal);
    m_stack->setAutoFillBackground(true);
    m_stack->setPalette(pal);
    setAccessibleName(tr("Ribbon"));
    installArrowNavigation(this);
    connect(UiTheme::instance(), &UiTheme::changed, this, &Ribbon::restyle);   // the look switched in Options
}

// The body holds a group's title (19), its margin (4), the tab's (6), and either the tallest large button or three rows: 100 for the usual font, more for a larger one.
void Ribbon::updateBodyHeight()
{
    const QFontMetrics fm(font());
    const int h = std::max({100, 29 + largeRibbonButtonHeight(fm, kLargeIcon), 31 + 3 * ribbonRowHeight(fm)});
    if (m_stack->height() != h || m_stack->minimumHeight() != h) m_stack->setFixedHeight(h);
    m_header->updateMetrics();
    m_header->layoutButtons();
}

bool Ribbon::event(QEvent *e)
{
    const bool r = QWidget::event(e);
    switch (e->type()) {
    case QEvent::FontChange:
    case QEvent::ApplicationFontChange:
    case QEvent::ScreenChangeInternal:
    case QEvent::DevicePixelRatioChange:
        updateBodyHeight();
        break;
    default:
        break;
    }
    return r;
}

// The colors set when the ribbon was built, again for the current look.
void Ribbon::restyle()
{
    auto paint = [](QWidget *w) {
        QPalette pal = w->palette();
        pal.setColor(QPalette::Window, ribbonBg());
        w->setPalette(pal);
    };
    paint(this);
    paint(m_stack);
    for (const Tab &t : m_tabs) {
        if (t.scroll) paint(t.scroll);
        paint(t.page);
    }
    for (QFrame *f : findChildren<QFrame *>(QStringLiteral("jpRibbonSeparator"))) f->setStyleSheet(QStringLiteral("background:%1;").arg(lineColor().name()));
    for (QWidget *w : findChildren<QWidget *>()) w->update();
}

void Ribbon::keyPressEvent(QKeyEvent *e)
{
    // Escape from a control (that didn't use it) goes back to the page.
    if (e->key() == Qt::Key_Escape) {
        Q_EMIT leaveRequested();
        return;
    }
    QWidget::keyPressEvent(e);
}

void Ribbon::focusCurrentTab()
{
    if (m_minimized) setMinimized(false);
    if (QWidget *b = tabButton(m_current)) b->setFocus(Qt::TabFocusReason);
}

QScrollArea *Ribbon::makeScroll(RibbonTab *page)
{
    auto *scroll = new QScrollArea();
    scroll->setFocusPolicy(Qt::NoFocus);   // only holds the controls; Tab goes to them
    scroll->setWidget(page);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->horizontalScrollBar()->setStyleSheet(QStringLiteral("QScrollBar:horizontal{height:6px;}"));
    QPalette pal = scroll->palette();
    pal.setColor(QPalette::Window, ribbonBg());
    scroll->setPalette(pal);
    m_stack->addWidget(scroll);
    return scroll;
}

void Ribbon::attachPage(RibbonTab *page)
{
    for (Tab &t : m_tabs)
        if (t.page == page && !t.scroll) t.scroll = makeScroll(page);
}

RibbonTab *Ribbon::addTab(const QString &name, const QString &contextGroup, const QColor &color, const QString &title, bool deferred)
{
    auto *page = new RibbonTab(m_pending);   // (a deferred tab's page waits in a hidden holder until it is made: never a window of its own)
    QPalette pal = page->palette();
    pal.setColor(QPalette::Window, ribbonBg());
    page->setAutoFillBackground(true);
    page->setPalette(pal);
    // The scroll area that holds the page costs several widgets: a tab to be made late gets it with its controls.
    QScrollArea *scroll = deferred ? nullptr : makeScroll(page);
    if (deferred) page->m_attach = [this, page] { attachPage(page); };
    connect(page, &RibbonTab::built, this, [this, page] {
        Q_EMIT tabBuilt(page);
        if (m_builtQueued) return;
        m_builtQueued = true;
        QMetaObject::invokeMethod(this, [this] {
            m_builtQueued = false;
            Q_EMIT tabsBuilt();
        }, Qt::QueuedConnection);
    });
    m_tabs.push_back(Tab{name, contextGroup, color, page, scroll, contextGroup.isEmpty(), title.isEmpty() ? name : title, QString(), int(m_tabs.size()), false, false, int(m_tabs.size())});
    m_header->addTabButton(int(m_tabs.size()) - 1);
    return page;
}

void Ribbon::setTabKeytip(RibbonTab *t, const QString &k)
{
    for (Tab &tab : m_tabs)
        if (tab.page == t) tab.keytip = k;
}

void Ribbon::setTabVisible(const QString &name, bool visible)
{
    for (int i = 0; i < m_tabs.size(); ++i)
        if (m_tabs[i].name == name && m_tabs[i].visible != visible) {
            m_tabs[i].visible = visible;
            if (!visible && i == m_current)
                for (int k = 0; k < m_tabs.size(); ++k)
                    if (m_tabs[k].name == QLatin1String("Home")) { showTab(m_tabs[k].page); break; }
            m_header->layoutButtons();
        }
}

bool Ribbon::setTabUserHidden(const QString &name, bool hidden)
{
    for (int i = 0; i < m_tabs.size(); ++i)
        if (m_tabs[i].name == name) {
            if (hidden && (name == QLatin1String("Home") || !m_tabs[i].group.isEmpty())) return false;
            if (m_tabs[i].userHidden == hidden) return true;
            m_tabs[i].userHidden = hidden;
            if (hidden && i == m_current)
                for (int k = 0; k < m_tabs.size(); ++k)
                    if (m_tabs[k].name == QLatin1String("Home")) { showTab(m_tabs[k].page); break; }
            m_header->layoutButtons();
            return true;
        }
    return false;
}

void Ribbon::setTabTitle(const QString &name, const QString &title)
{
    for (Tab &t : m_tabs)
        if (t.name == name && t.title != title) {
            t.title = title;
            m_header->layoutButtons();
        }
}

void Ribbon::arrangeTabs(const QStringList &names)
{
    QList<const Tab *> plain;
    for (const Tab &t : m_tabs)
        if (t.group.isEmpty()) plain << &t;
    QList<int> places;
    for (const Tab *t : plain) places << t->home;
    std::sort(places.begin(), places.end());
    // The wanted order: the names given (each once, only tabs there are), then the rest as built.
    QStringList order;
    for (const QString &n : names)
        for (const Tab *t : plain)
            if (t->name == n && !order.contains(n)) order << n;
    QList<const Tab *> rest = plain;
    std::sort(rest.begin(), rest.end(), [](const Tab *a, const Tab *b) { return a->base < b->base; });
    if (names.isEmpty()) order.clear();
    for (const Tab *t : rest)
        if (!order.contains(t->name)) order << t->name;
    for (int k = 0; k < order.size(); ++k)
        for (Tab &t : m_tabs)
            if (t.group.isEmpty() && t.name == order[k]) t.home = places[k];
    sortTabs();
    m_header->layoutButtons();
}

void Ribbon::removeTab(const QString &name)
{
    for (int i = 0; i < m_tabs.size(); ++i) {
        if (m_tabs[i].name != name) continue;
        const bool wasCurrent = i == m_current;
        for (int k = int(m_header->tabButtons.size()) - 1; k >= 0; --k) {
            HeaderButton *b = m_header->tabButtons[k];
            if (b->tab() == i) {
                m_header->tabButtons.remove(k);
                delete b;
            } else if (b->tab() > i) {
                b->setTab(b->tab() - 1);
            }
        }
        QScrollArea *scroll = m_tabs[i].scroll;
        RibbonTab *page = m_tabs[i].page;
        m_tabs.remove(i);
        if (scroll) delete scroll;   // (it owns the page)
        else delete page;
        if (m_current > i) --m_current;
        if (wasCurrent) {
            m_current = 0;
            for (int k = 0; k < m_tabs.size(); ++k)
                if (m_tabs[k].name == QLatin1String("Home")) m_current = k;
            if (!m_tabs.isEmpty()) showTab(m_tabs[m_current].page);
        } else {
            m_header->layoutButtons();
        }
        return;
    }
}

void Ribbon::setContextVisible(const QString &group, bool visible, bool first)
{
    bool changed = false, currentHidden = false, reorder = false;
    RibbonTab *appeared = nullptr;
    for (int i = 0; i < m_tabs.size(); ++i) {
        auto &t = m_tabs[i];
        if (t.group != group || t.visible == visible) continue;
        t.visible = visible;
        if (t.first != (visible && first)) {
            t.first = visible && first;
            reorder = true;
        }
        changed = true;
        if (!visible && i == m_current) currentHidden = true;
        if (t.first && !appeared) appeared = t.page;
    }
    if (reorder) sortTabs();
    if (currentHidden) {
        for (int i = 0; i < m_tabs.size(); ++i)
            if (m_tabs[i].name == QLatin1String("Home")) { showTab(m_tabs[i].page); break; }
    }
    if (appeared) showTab(appeared);
    if (changed) m_header->layoutButtons();
}

// The tabs in order: those placed first, then the rest as built. Every tab
// button follows its tab to its new number.
void Ribbon::sortTabs()
{
    RibbonTab *current = m_tabs.value(m_current).page;
    QHash<HeaderButton *, RibbonTab *> pages;
    for (HeaderButton *b : m_header->tabButtons) pages[b] = m_tabs[b->tab()].page;
    std::stable_sort(m_tabs.begin(), m_tabs.end(), [](const Tab &a, const Tab &b) { return a.first != b.first ? a.first : a.home < b.home; });
    for (int i = 0; i < m_tabs.size(); ++i) {
        if (m_tabs[i].page == current) m_current = i;
        for (HeaderButton *b : m_header->tabButtons)
            if (pages[b] == m_tabs[i].page) b->setTab(i);
    }
    std::sort(m_header->tabButtons.begin(), m_header->tabButtons.end(), [](const HeaderButton *a, const HeaderButton *b) { return a->tab() < b->tab(); });
    for (HeaderButton *b : m_header->tabButtons) b->raise();   // screen readers go through them in the order the parent holds them
}

void Ribbon::showTab(RibbonTab *t)
{
    if (t) t->ensureBuilt();
    for (int i = 0; i < m_tabs.size(); ++i)
        if (m_tabs[i].page == t) {
            m_current = i;
            m_stack->setCurrentWidget(m_tabs[i].scroll);
        }
    m_header->layoutButtons();
    Q_EMIT tabChanged();
}

RibbonTab *Ribbon::current() const { return m_tabs.value(m_current).page; }

RibbonTab *Ribbon::tab(const QString &name) const
{
    for (const auto &t : m_tabs)
        if (t.name == name) return t.page;
    return nullptr;
}

QToolButton *Ribbon::addQuickAccess(QAction *a)
{
    auto *b = new QToolButton(m_header);
    b->setDefaultAction(a);
    b->setAutoRaise(true);
    b->setIconSize(QSize(16, 16));
    b->setToolButtonStyle(Qt::ToolButtonIconOnly);
    b->setFocusPolicy(Qt::TabFocus);   // the keyboard reaches it; a click leaves the focus in the text
    m_header->qatButtons << b;
    m_header->layoutQat();
    m_header->update();
    return b;
}

QWidget *Ribbon::tabButton(int i) const
{
    for (HeaderButton *b : m_header->tabButtons)
        if (b->tab() == i) return b;
    return nullptr;
}

QRect Ribbon::brandRect(QFont *font) const
{
    const auto b = m_header->brand();
    if (font) *font = b.font;
    return b.text.translated(m_header->pos());
}

QWidget *Ribbon::moreButton() const { return m_header->m_more && m_header->m_more->isVisible() ? m_header->m_more : nullptr; }

QWidget *Ribbon::fileButton() const { return m_header->m_file; }

QList<QToolButton *> Ribbon::quickAccessButtons() const { return QList<QToolButton *>(m_header->qatButtons.begin(), m_header->qatButtons.end()); }

void Ribbon::setMinimized(bool m)
{
    m_minimized = m;
    m_stack->setVisible(!m);
    m_header->layoutButtons();
}

// ---------- description (tests) ----------
namespace {
QString describeAction(const QAction *a)
{
    if (!a) return QStringLiteral("(none)");
    if (a->isSeparator()) return QStringLiteral("-");
    if (auto *wa = qobject_cast<const QWidgetAction *>(a)) return QStringLiteral("widget:") + QString::fromLatin1(wa->defaultWidget() ? wa->defaultWidget()->metaObject()->className() : "?");
    QString s = a->objectName().isEmpty() ? QStringLiteral("\"%1\"").arg(a->text()) : a->objectName();
    if (a->menu()) s += QStringLiteral(" >");
    return s;
}

QString describeMenu(const QMenu *m, int depth = 0)
{
    if (!m) return QString();
    QStringList parts;
    for (const QAction *a : m->actions()) {
        QString s = describeAction(a);
        if (a->menu() && depth < 3) s += QStringLiteral(" {") + describeMenu(a->menu(), depth + 1) + QLatin1Char('}');
        parts << s;
    }
    return parts.join(QStringLiteral(", "));
}

QString keytipSuffix(const QObject *o, bool on, const char *label = "keytip")
{
    const QString k = on && o ? keytip(o) : QString();
    return k.isEmpty() ? QString() : QStringLiteral(" %1=%2").arg(QLatin1String(label), k);
}

QString describeWidget(const QWidget *w, bool keytips)
{
    QString s = QString::fromLatin1(w->metaObject()->className());
    if (auto *b = qobject_cast<const QToolButton *>(w)) {
        s += QLatin1Char(' ') + (b->defaultAction() ? describeAction(b->defaultAction()) : QStringLiteral("\"%1\"").arg(b->text()));
        static const char *styles[] = {"icon", "text", "beside", "under", "follow"};
        s += QStringLiteral(" style=%1 icon=%2").arg(QLatin1String(styles[b->toolButtonStyle()])).arg(b->iconSize().width());
        if (b->menu()) {
            static const char *modes[] = {"delayed", "split", "instant"};
            s += QStringLiteral(" popup=%1 menu=[%2]").arg(QLatin1String(modes[b->popupMode()]), describeMenu(b->menu()));
        }
    } else if (auto *l = qobject_cast<const QLabel *>(w)) {
        s += QStringLiteral(" \"%1\"").arg(l->text());
    }
    if (!w->toolTip().isEmpty()) s += QStringLiteral(" tip=\"%1\"").arg(w->toolTip());
    return s + keytipSuffix(w, keytips);
}

void describeLayout(const QLayout *l, int depth, QStringList &out, bool keytips)
{
    for (int i = 0; i < l->count(); ++i) {
        QLayoutItem *it = l->itemAt(i);
        const QString pad(depth * 2, QLatin1Char(' '));
        if (QWidget *w = it->widget()) out << pad + describeWidget(w, keytips);
        else if (QLayout *sub = it->layout()) {
            out << pad + QString::fromLatin1(qobject_cast<QHBoxLayout *>(sub) ? "row" : "column");
            describeLayout(sub, depth + 1, out, keytips);
        }
    }
}
} // namespace

void Ribbon::buildAllTabs()
{
    for (const Tab &t : m_tabs) t.page->ensureBuilt();
}

QString Ribbon::describe(bool keytips) const
{
    const_cast<Ribbon *>(this)->buildAllTabs();
    QStringList out;
    if (keytips && !m_fileKeytip.isEmpty()) out << QStringLiteral("file keytip=") + m_fileKeytip;
    QStringList qat;
    for (const QToolButton *b : m_header->qatButtons) qat << describeAction(b->defaultAction()) + keytipSuffix(b, keytips);
    out << QStringLiteral("quick access: ") + qat.join(QStringLiteral(", "));
    for (const Tab &t : m_tabs) {
        QString line = QStringLiteral("tab %1%2").arg(t.name, t.group.isEmpty() ? QString() : QStringLiteral(" (%1, %2)").arg(t.group, t.color.name()));
        if (keytips && !t.keytip.isEmpty()) line += QStringLiteral(" keytip=") + t.keytip;
        out << line;
        for (const RibbonGroup *g : t.page->findChildren<RibbonGroup *>(Qt::FindDirectChildrenOnly)) {
            QString head = QStringLiteral("  group %1").arg(g->title());
            const QToolButton *l = g->findChild<QToolButton *>(QStringLiteral("launcher"), Qt::FindDirectChildrenOnly);
            if (l) head += QStringLiteral(" launcher=\"%1\"").arg(l->toolTip());
            head += keytipSuffix(g, keytips) + keytipSuffix(l, keytips, "launcher-keytip");
            out << head;
            describeLayout(g->layout(), 2, out, keytips);
        }
    }
    return out.join(QLatin1Char('\n')) + QLatin1Char('\n');
}

} // namespace jp
