// JeffDeck's Help: F1 and the ribbon's "?" open the Help pane on the right of
// the window, on the topic for what is being done (resources/help/en/deck/context.json).

#include "app/deckwindow.h"

#include "app/animationpane.h"
#include "app/commentspane.h"
#include "app/deckfilescreen.h"
#include "app/editor.h"
#include "app/deckfindpane.h"
#include "app/help.h"
#include "app/notespane.h"
#include "app/onlinepictures.h"
#include "app/outlineview.h"
#include "app/reusepane.h"
#include "app/ribbon.h"
#include "app/slidesorter.h"
#include "app/slidespane.h"

#include <QAction>
#include <QApplication>
#include <QDockWidget>

namespace jp {

void DeckWindow::createHelpActions()
{
    mk("help.show", tr("Help"), "circle-help", QKeySequence::HelpContents, [this] { showHelp(helpContext()); });
    connect(m_ribbon, &Ribbon::helpClicked, this, [this] { act("help.show")->trigger(); });
}

void DeckWindow::showHelp(const QString &topicIn)
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

HelpWindow *DeckWindow::showHelpOver(const QString &topic)
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

// The topic for where the keyboard is: the File screen's page, the ribbon's
// tab, a pane or a view, else the kind of object selected. Empty when none
// has one.
QString DeckWindow::helpContext() const
{
    QWidget *f = QApplication::focusWidget();
    auto inside = [f](const QWidget *w) { return f && w && (f == w || w->isAncestorOf(f)); };
    QString key;
    if (m_fileScreen->isVisible()) key = QStringLiteral("file:") + m_fileScreen->currentPage();
    else if (inside(m_ribbon)) {
        for (int i = 0; i < m_ribbon->tabCount(); ++i)
            if (m_ribbon->tabPage(i) == m_ribbon->current()) key = QStringLiteral("tab:") + m_ribbon->tabName(i);
    } else if (inside(m_notes)) key = QStringLiteral("pane:notes");
    else if (inside(m_comments)) key = QStringLiteral("pane:comments");
    else if (inside(m_reuse)) key = QStringLiteral("pane:reuse");
    else if (inside(m_online)) key = QStringLiteral("pane:online");
    else if (inside(m_find)) key = QStringLiteral("pane:find");
    else if (inside(m_animPane)) key = QStringLiteral("pane:animation");
    else if (inside(m_slides)) key = QStringLiteral("pane:slides");
    else if (inside(m_outline)) key = QStringLiteral("view:outline");
    else if (inside(m_sorter)) key = QStringLiteral("view:sorter");
    else if (m_view == View::Master) key = QStringLiteral("view:master");
    else if (!m_ed->selectionKind().isEmpty()) key = QStringLiteral("item:") + m_ed->selectionKind();
    return key.isEmpty() ? QString() : help::contextTopic(key);
}

} // namespace jp
