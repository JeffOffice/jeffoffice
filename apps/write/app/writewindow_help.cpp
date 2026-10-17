// JeffWrite's Help: F1 and the ribbon's "?" open the Help pane on the right of the window, on the topic for what is being done
// (resources/help/en/write/context.json).

#include "app/writewindow.h"

#include "app/help.h"
#include "app/pageview.h"
#include "app/ribbon.h"
#include "app/spellpane.h"
#include "app/writefilescreen.h"

#include <QAction>
#include <QApplication>
#include <QDockWidget>

namespace jp {

void WriteWindow::createHelpActions()
{
    mk("help.show", tr("Help"), "circle-help", QKeySequence::HelpContents, [this] { showHelp(helpContext()); });
    connect(m_ribbon, &Ribbon::helpClicked, this, [this] { act("help.show")->trigger(); });
}

void WriteWindow::showHelp(const QString &topicIn)
{
    const QString topic = topicIn.isEmpty() ? QStringLiteral("index") : topicIn;
    // The File screen covers the pane: Help opens in a window of its own.
    if (m_fileScreen->isVisible()) {
        showHelpOver(topic);
        return;
    }
    if (!m_help) {
        m_helpDock = new QDockWidget(tr("Help"), this);
        m_helpDock->setObjectName(QStringLiteral("helpDock"));
        m_helpDock->setFeatures(QDockWidget::DockWidgetClosable);
        m_help = new HelpView(this, m_helpDock);
        m_helpDock->setWidget(m_help);
        addDockWidget(Qt::RightDockWidgetArea, m_helpDock);
        resizeDocks({m_helpDock}, {360}, Qt::Horizontal);
    }
    m_helpDock->show();
    m_helpDock->raise();
    m_help->showTopic(topic);
    m_help->focusTopic();
}

HelpWindow *WriteWindow::showHelpOver(const QString &topic)
{
    // One, reused while it's open.
    auto *w = findChild<HelpWindow *>(QString(), Qt::FindDirectChildrenOnly);
    if (!w) w = new HelpWindow(this, this);
    w->view()->showTopic(topic);
    w->show();
    w->raise();
    w->activateWindow();
    w->view()->focusTopic();
    return w;
}

// The topic for where the keyboard is: the File screen's page, the ribbon's tab, or the text. Empty when none has one.
QString WriteWindow::helpContext() const
{
    QWidget *f = QApplication::focusWidget();
    auto inside = [f](const QWidget *w) { return f && w && (f == w || w->isAncestorOf(f)); };
    QString key;
    if (m_fileScreen->isVisible()) {
        key = QStringLiteral("file:") + m_fileScreen->currentPage();
    } else if (m_spellPane && inside(m_spellPane)) {
        key = QStringLiteral("pane:spelling");
    } else if (inside(m_ribbon)) {
        for (int i = 0; i < m_ribbon->tabCount(); ++i)
            if (m_ribbon->tabPage(i) == m_ribbon->current()) key = QStringLiteral("tab:") + m_ribbon->tabName(i);
    }
    return key.isEmpty() ? QString() : help::contextTopic(key);
}

} // namespace jp
