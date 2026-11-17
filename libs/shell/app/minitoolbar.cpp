#include "app/minitoolbar.h"
#include "app/uistyle.h"
#include "app/widgets.h"
#include <QAbstractScrollArea>
#include <QAction>
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QScrollBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <algorithm>

namespace jp {

MiniToolbar::MiniToolbar(QWidget *host, std::unique_ptr<MiniTarget> owned) : QFrame(host), m_target(std::move(owned))
{
    MiniTarget *target = m_target.get();
    setObjectName(QStringLiteral("miniToolbar"));
    jp::softPanel(this);
    setAutoFillBackground(true);
    auto *column = new QVBoxLayout(this);
    column->setContentsMargins(3, 2, 3, 2);
    column->setSpacing(1);
    // The font box on top, then the buttons and the font color (the same commands as the ribbon's, and they follow the selection as its boxes do).
    if (target->supportsFont()) {
        m_font = new FontCombo(this);
        connect(m_font, &FontCombo::familyChosen, this, [target](const QString &f) { target->chooseFont(f); });
        column->addWidget(m_font);
    }
    auto *row = new QHBoxLayout;
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(1);
    column->addLayout(row);
    using C = MiniTarget::Command;
    for (C c : {C::Bold, C::Italic, C::Underline, C::Shrink, C::Grow, C::AlignLeft, C::AlignCenter, C::AlignRight, C::Bullets, C::Numbering}) {
        QAction *a = target->action(c);
        if (!a) continue;
        auto *button = new QToolButton(this);
        button->setDefaultAction(a);
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
        button->setAutoRaise(true);
        button->setFocusPolicy(Qt::NoFocus);   // (the text keeps the keyboard)
        row->addWidget(button);
    }
    if (target->supportsColor()) {
        m_color = new ColorButton(QStringLiteral("baseline"), QCoreApplication::translate("MiniToolbar", "Font Color"), true, QCoreApplication::translate("MiniToolbar", "Automatic"), this);
        m_color->setCurrent(ColorRef::rgb(QColor(0xC0, 0, 0)));
        m_color->setExtraActions(target->colorExtras());
        connect(m_color, &ColorButton::colorPicked, this, [target](const ColorRef &c) { target->chooseColor(c); });
        row->addWidget(m_color);
    }
    hide();
}

void MiniToolbar::showBeside(const QRect &first, const QRect &last)
{
    const QSize size = sizeHint();
    const QRect area = parentWidget()->rect();
    int y = first.top() - size.height() - 6;
    if (y < 0) y = last.bottom() + 8;
    setGeometry(QRect(QPoint(std::clamp(first.left(), 0, std::max(0, area.width() - size.width())), std::clamp(y, 0, std::max(0, area.height() - size.height()))), size));
    show();
    raise();
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
