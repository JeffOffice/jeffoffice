// Home > Editing: Find and Replace (the pane on the right).

#include "app/writewindow.h"

#include "app/pageview.h"
#include "app/writefindpane.h"

#include <QAction>
#include <QDockWidget>

namespace jp {

void WriteWindow::createEditingActions()
{
    mk("edit.find", tr("Find"), "search", QKeySequence::Find, [this] { showFindPane(false); });
    mk("edit.replace", tr("Replace"), "replace", QKeySequence(Qt::CTRL | Qt::Key_H), [this] { showFindPane(true); });
}

void WriteWindow::showFindPane(bool replace)
{
    // (Find and Replace look through the text of the document, not its headers and footers: a header being edited is left first.)
    m_view->leaveHeaderFooter();
    m_view->leaveNote();
    if (!m_find) {
        m_findDock = new QDockWidget(tr("Find and Replace"), this);
        m_findDock->setObjectName(QStringLiteral("findDock"));
        m_findDock->setFeatures(QDockWidget::DockWidgetClosable);
        m_find = new WriteFindPane(this, m_findDock);
        m_findDock->setWidget(m_find);
        addDockWidget(Qt::RightDockWidgetArea, m_findDock);
        resizeDocks({m_findDock}, {300}, Qt::Horizontal);
    }
    m_findDock->show();
    m_findDock->raise();
    m_find->focusBox(replace);
}

} // namespace jp
