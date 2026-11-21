#include "app/pasteoffer.h"

#include <QAction>
#include <QCoreApplication>
#include <QEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QToolButton>
#include <QWidget>
#include <algorithm>

namespace jp {

static int s_key = 0;
void PasteOffer::setKeyForTest(int key) { s_key = key; }
int PasteOffer::key()
{
    if (s_key) return s_key;
#ifdef Q_OS_MACOS
    return Qt::Key_Meta;   // (Qt's Key_Control is the Command key there: the key by that name on the keyboard is Key_Meta)
#else
    return Qt::Key_Control;
#endif
}

QString PasteOffer::keyName()
{
    return key() == Qt::Key_Meta ? QCoreApplication::translate("jp::PasteOffer", "Control") : QCoreApplication::translate("jp::PasteOffer", "Ctrl");
}

PasteOffer::PasteOffer(QWidget *host, QWidget *focus, const Choices &choices) : QObject(host), m_host(host), m_focus(focus), m_choices(choices)
{
}

PasteOffer::~PasteOffer()
{
    // (the window may be closing: nothing the button's menu holds calls back into it)
    if (m_button) {
        if (QMenu *menu = m_button->menu()) menu->disconnect();
        m_button->disconnect();
        delete m_button;
    }
}

// The choices, as actions of the menu; an action is let go later, since this runs from the step an action of the menu took.
void PasteOffer::fill()
{
    QMenu *menu = m_button->menu();
    for (QAction *old : menu->actions()) {
        menu->removeAction(old);
        old->deleteLater();
    }
    const QVector<PasteChoice> choices = m_choices ? m_choices() : QVector<PasteChoice>();
    for (const PasteChoice &c : choices) {
        if (c.separatorBefore && !menu->actions().isEmpty()) menu->addSeparator();
        QAction *a = menu->addAction(c.text);
        a->setObjectName(c.name);
        const std::function<void()> run = c.run;
        QPointer<QWidget> focus = m_focus;
        connect(a, &QAction::triggered, this, [run, focus] {
            if (run) run();
            if (focus) focus->setFocus(Qt::OtherFocusReason);
        });
    }
}

void PasteOffer::setDescription(const QString &text)
{
    m_description = text;
    if (m_button) m_button->setAccessibleDescription(text);
}

void PasteOffer::showAt(const QRect &anchor)
{
    if (!m_host) return;
    if (anchor.isNull()) {
        hide();
        return;
    }
    if (!m_button) {
        m_button = new QToolButton(m_host);
        m_button->setObjectName(QStringLiteral("pasteOptionsButton"));
        m_button->setText(QCoreApplication::translate("jp::PasteOffer", "Paste Options"));
        m_button->setAccessibleName(QCoreApplication::translate("jp::PasteOffer", "Paste Options"));
        m_button->setAccessibleDescription(m_description);
        m_button->setToolButtonStyle(Qt::ToolButtonTextOnly);
        m_button->setPopupMode(QToolButton::InstantPopup);
        m_button->setFocusPolicy(Qt::NoFocus);
        auto *menu = new QMenu(m_button);
        m_button->setMenu(menu);
        connect(menu, &QMenu::aboutToShow, this, [this] { fill(); });   // (built when it opens: what the choices can do is as of then)
        m_button->hide();
    }
    m_button->adjustSize();
    const QSize size = m_button->size();
    m_button->move(std::clamp(anchor.right() + 6, 0, std::max(0, m_host->width() - size.width())), std::clamp(anchor.bottom() + 4, 0, std::max(0, m_host->height() - size.height())));
    m_button->show();
    m_button->raise();
}

void PasteOffer::hide()
{
    m_tap = false;
    if (m_button) m_button->hide();
}

bool PasteOffer::isShown() const { return m_button && m_button->isVisible(); }

bool PasteOffer::openMenu()
{
    if (!isShown()) return false;
    // (popup, not exec: the menu is up while the key event is over, and the arrow keys and Enter choose)
    m_button->menu()->popup(m_button->mapToGlobal(QPoint(0, m_button->height())));
    Q_EMIT opened();
    return true;
}

void PasteOffer::keyPressed(const QKeyEvent *e, bool live)
{
    m_tap = e->key() == key() && !e->isAutoRepeat() && live;
    if (live && e->key() == Qt::Key_Escape && e->modifiers() == Qt::NoModifier) Q_EMIT escaped();
}

void PasteOffer::shortcutOverride(const QKeyEvent *e)
{
    const int k = e->key();
    if (k != Qt::Key_Control && k != Qt::Key_Meta && k != Qt::Key_Shift && k != Qt::Key_Alt) m_tap = false;
}

bool PasteOffer::wasTap(const QKeyEvent *e)
{
    const bool tap = m_tap && e->key() == key() && !e->isAutoRepeat();
    m_tap = false;
    return tap;
}

bool PasteOffer::keyReleased(const QKeyEvent *e) { return wasTap(e) && openMenu(); }

void PasteOffer::installOn(QWidget *focus)
{
    focus->installEventFilter(this);
}

bool PasteOffer::eventFilter(QObject *, QEvent *e)
{
    switch (e->type()) {
    case QEvent::WindowDeactivate:
    case QEvent::FocusOut:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick:
    case QEvent::Wheel:
        m_tap = false;
        break;
    case QEvent::ShortcutOverride:
        shortcutOverride(static_cast<QKeyEvent *>(e));
        break;
    case QEvent::KeyPress:
        keyPressed(static_cast<QKeyEvent *>(e), isShown());
        break;
    case QEvent::KeyRelease:
        if (keyReleased(static_cast<QKeyEvent *>(e))) return true;
        break;
    default:
        break;
    }
    return false;
}

} // namespace jp
