#include "app/minitoolbar.h"
#include "app/uistyle.h"
#include "app/settings.h"
#include "app/widgets.h"
#include <QAbstractScrollArea>
#include <QAction>
#include <QCoreApplication>
#include <QApplication>
#include <QCursor>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMenu>
#include <QScrollBar>
#include <QShowEvent>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>

namespace jp {

namespace {
const QSize kCell(28, 26);   // a button of an aligned bar
std::function<QPoint()> pointerSource;   // (a test's own pointer)

// A button that gives its action to the target to run, when the target wants it (for one that asks a question: not from inside a menu that is about to close).
class BarButton : public QToolButton {
public:
    BarButton(QWidget *parent, std::function<bool(QAction *)> takeOver, std::function<void()> ran) : QToolButton(parent), m_takeOver(std::move(takeOver)), m_ran(std::move(ran)) {}

protected:
    void nextCheckState() override
    {
        QAction *a = defaultAction();
        if (a && m_takeOver && m_takeOver(a)) return;
        QToolButton::nextCheckState();
        if (m_ran) m_ran();
    }

private:
    std::function<bool(QAction *)> m_takeOver;
    std::function<void()> m_ran;
};
}

MiniToolbar::MiniToolbar(QWidget *host, std::unique_ptr<MiniTarget> owned) : QFrame(host), m_target(std::move(owned))
{
    MiniTarget *target = m_target.get();
    setObjectName(QStringLiteral("miniToolbar"));
    // (Options: Show Mini Toolbar on selection, turned off, takes a bar that shows away at once)
    connect(&MiniToolbarSettingNotifier::get(), &MiniToolbarSettingNotifier::changed, this, [this](bool on) {
        if (!on) hide();
    });
    jp::softPanel(this);
    setAutoFillBackground(true);
    auto *column = new QVBoxLayout(this);
    column->setContentsMargins(3, 2, 3, 2);
    column->setSpacing(1);
    // The font box on top, then the buttons and the font color (the same commands as the ribbon's, and they follow the selection as its boxes do).
    if (target->supportsFont()) {
        m_font = new FontCombo(this);
        connect(m_font, &FontCombo::familyChosen, this, [this, target](const QString &f) {
            target->chooseFont(f);
            Q_EMIT commandRan();
        });
        if (target->supportsSize()) {
            auto *top = new QHBoxLayout;
            top->setContentsMargins(0, 0, 0, 0);
            top->setSpacing(2);
            m_size = new SizeCombo(this);
            m_size->setFocusPolicy(Qt::NoFocus);
            m_font->setFocusPolicy(Qt::NoFocus);   // (a click on either box leaves the selection where it is: the entry keeps the keyboard)
            connect(m_size, &SizeCombo::sizeChosen, this, [this, target](double pt) {
                target->chooseSize(pt);
                Q_EMIT commandRan();
            });
            top->addWidget(m_font, 1, target->alignRows() ? Qt::AlignLeft : Qt::Alignment());   // (an aligned bar starts the font box where its icon rows start)
            top->addWidget(m_size);
            column->addLayout(top);
        } else {
            column->addWidget(m_font);
        }
    }
    auto *row = new QHBoxLayout;
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(1);
    column->addLayout(row);
    using C = MiniTarget::Command;
    for (C c : {C::Bold, C::Italic, C::Underline, C::Shrink, C::Grow, C::AlignLeft, C::AlignCenter, C::AlignRight, C::Bullets, C::Numbering}) {
        QAction *a = target->action(c);
        if (!a) continue;
        auto *button = new BarButton(this, [target](QAction *x) { return target->takeOver(x); }, [this] { Q_EMIT commandRan(); });
        button->setDefaultAction(a);
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
        button->setAutoRaise(true);
        button->setFocusPolicy(Qt::NoFocus);   // (the text keeps the keyboard)
        if (target->alignRows()) button->setFixedSize(kCell);
        row->addWidget(button);
    }
    if (target->supportsFill()) {
        m_fill = new ColorButton(QStringLiteral("paint-bucket"), QCoreApplication::translate("MiniToolbar", "Fill Color"), true, QCoreApplication::translate("MiniToolbar", "No Fill"), this);
        m_fill->setCurrent(ColorRef::rgb(QColor(0xFF, 0xFF, 0x00)));
        connect(m_fill, &ColorButton::colorPicked, this, [this, target](const ColorRef &c) {
            m_fill->setCurrent(c);
            target->chooseFill(c);
            Q_EMIT commandRan();
        });
        row->addWidget(m_fill);
    }
    if (target->supportsColor()) {
        m_color = new ColorButton(QStringLiteral("baseline"), QCoreApplication::translate("MiniToolbar", "Font Color"), true, QCoreApplication::translate("MiniToolbar", "Automatic"), this);
        m_color->setCurrent(ColorRef::rgb(QColor(0xC0, 0, 0)));
        m_color->setExtraActions(target->colorExtras());
        connect(m_color, &ColorButton::colorPicked, this, [this, target](const ColorRef &c) {
            target->chooseColor(c);
            Q_EMIT commandRan();
        });
        row->addWidget(m_color);
    }
    if (target->alignRows()) row->addStretch(1);   // (a row starts at the left edge and keeps its buttons' size)
    const QList<QAction *> more = target->moreActions();
    if (!more.isEmpty()) {
        auto *extra = new QHBoxLayout;
        extra->setContentsMargins(0, 0, 0, 0);
        extra->setSpacing(1);
        column->addLayout(extra);
        for (QAction *a : more) {
            auto *button = new BarButton(this, [target](QAction *x) { return target->takeOver(x); }, [this] { Q_EMIT commandRan(); });
            button->setDefaultAction(a);
            button->setToolButtonStyle(Qt::ToolButtonIconOnly);
            button->setAutoRaise(true);
            button->setFocusPolicy(Qt::NoFocus);
            if (a->menu()) button->setPopupMode(QToolButton::InstantPopup);
            if (target->alignRows()) button->setFixedSize(kCell);
            extra->addWidget(button);
        }
        extra->addStretch(1);
    }
    hide();
}

void MiniToolbar::showBeside(const QRect &first, const QRect &last)
{
    if (!miniToolbarOnSelection()) {   // (Options: Show Mini Toolbar on selection)
        hide();
        return;
    }
    const QSize size = sizeHint();
    const QRect area = parentWidget()->rect();
    int y = first.top() - size.height() - 6;
    if (y < 0) y = last.bottom() + 8;
    setGeometry(QRect(QPoint(std::clamp(first.left(), 0, std::max(0, area.width() - size.width())), std::clamp(y, 0, std::max(0, area.height() - size.height()))), size));
    show();
    raise();
}

void MiniToolbar::fadeAwayFromPointer(bool on)
{
    if (!on) {
        delete m_fadeTimer;
        m_fadeTimer = nullptr;
        if (m_fade) m_fade->setOpacity(1);
        return;
    }
    if (m_fadeTimer) return;
    if (!m_fade) {
        m_fade = new QGraphicsOpacityEffect(this);
        setGraphicsEffect(m_fade);
    }
    m_fadeTimer = new QTimer(this);
    m_fadeTimer->setInterval(60);
    // (the pointer is not believed for the first few looks: it may still be where it was before the bar came)
    connect(m_fadeTimer, &QTimer::timeout, this, [this] {
        if (++m_ticks <= kGraceTicks) return;
        ++m_looks;
        fadeTo(pointerSource ? pointerSource() : QCursor::pos());
    });
    if (isVisible()) m_fadeTimer->start();
}

void MiniToolbar::setPointerSource(const std::function<QPoint()> &source) { pointerSource = source; }

// Fully opaque within 40 pixels of the bar, fading out by 180, and gone past that.
double MiniToolbar::opacityFor(const QRect &bar, const QPoint &p)
{
    const int dx = std::max({bar.left() - p.x(), 0, p.x() - bar.right()}), dy = std::max({bar.top() - p.y(), 0, p.y() - bar.bottom()});
    const double d = std::hypot(dx, dy);
    return std::clamp(1.0 - (d - 40.0) / 140.0, 0.0, 1.0);
}

// A bar with a list of its own open (a font list, a palette, the borders) is left as it is, however far the pointer: the list is part of it, and goes with it (the pointer is for the list
// then, and a bar that went would take it down while a choice is being made).
void MiniToolbar::fadeTo(const QPoint &global)
{
    if (QWidget *popup = QApplication::activePopupWidget(); popup && popup->isVisible()) return;
    const double o = opacityFor(QRect(mapToGlobal(QPoint(0, 0)), size()), global);
    if (o <= 0) {
        hide();
        return;
    }
    if (m_fade) m_fade->setOpacity(o);
}

void MiniToolbar::showEvent(QShowEvent *event)
{
    QFrame::showEvent(event);
    if (m_fade) m_fade->setOpacity(1);
    m_ticks = 0;
    m_looks = 0;
    if (m_fadeTimer) m_fadeTimer->start();
}

void MiniToolbar::hideEvent(QHideEvent *event)
{
    QFrame::hideEvent(event);
    if (m_fadeTimer) m_fadeTimer->stop();
}

void MiniToolbar::hideOnTypingAndScrolling(QAbstractScrollArea *area)
{
    area->installEventFilter(this);
    area->viewport()->installEventFilter(this);
    for (QScrollBar *bar : {area->horizontalScrollBar(), area->verticalScrollBar()})
        connect(bar, &QScrollBar::valueChanged, this, [this] { hide(); });
}

bool MiniToolbar::eventFilter(QObject *, QEvent *event)
{
    if (!isVisible()) return false;
    if (event->type() == QEvent::Wheel || event->type() == QEvent::Resize) {
        hide();
    } else if (event->type() == QEvent::KeyPress) {
        const auto *k = static_cast<QKeyEvent *>(event);
        const bool command = k->modifiers() & (Qt::ControlModifier | Qt::MetaModifier);
        const bool typed = !command && !k->text().isEmpty() && k->text().at(0).isPrint();
        if (k->key() == Qt::Key_Escape || typed || k->key() == Qt::Key_Backspace || k->key() == Qt::Key_Delete || k->key() == Qt::Key_Return || k->key() == Qt::Key_Enter || k->key() == Qt::Key_Tab) hide();
    }
    return false;
}

} // namespace jp
