// Home > Editing: Find and Replace (the pane on the right) and Select Objects.

#include "app/deckwindow.h"

#include "app/editor.h"
#include "app/deckfindpane.h"

#include <QAction>
#include <QDockWidget>

namespace jp {

void DeckWindow::createEditingActions()
{
    Editor *ed = m_ed;
    mk("edit.find", tr("Find"), "search", QKeySequence::Find, [this] { showFindPane(false); });
    mk("edit.replace", tr("Replace"), "replace", QKeySequence(Qt::CTRL | Qt::Key_H), [this] { showFindPane(true); });
    mk("edit.selectObjects", tr("Select Objects"), "mouse-pointer-2", QKeySequence(), [ed] {
        ed->setTool(Tool::Select);
        ed->endTextEdit();
    });
}

void DeckWindow::showFindPane(bool replace)
{
    if (!m_find) {
        m_findDock = new QDockWidget(tr("Find and Replace"), this);
        m_findDock->setObjectName(QStringLiteral("findDock"));
        m_findDock->setFeatures(QDockWidget::DockWidgetClosable);
        m_find = new DeckFindPane(this, m_findDock);
        m_findDock->setWidget(m_find);
        addDockWidget(Qt::RightDockWidgetArea, m_findDock);
        resizeDocks({m_findDock}, {300}, Qt::Horizontal);
    }
    m_findDock->show();
    m_findDock->raise();
    m_find->focusBox(replace);
}

} // namespace jp
