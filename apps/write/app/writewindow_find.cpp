// Home > Editing: Find and Replace (the pane on the right).

#include "app/writewindow.h"

#include "app/pageview.h"
#include "app/writefindpane.h"
#include "core/fontops.h"

#include <QAction>
#include <QDockWidget>

namespace jp {

void WriteWindow::createEditingActions()
{
    mk("edit.find", tr("Find"), "search", QKeySequence::Find, [this] { showFindPane(false); });
    mk("edit.replace", tr("Replace"), "replace", QKeySequence(Qt::CTRL | Qt::Key_H), [this] { showFindPane(true); });
    mk("edit.replaceFonts", tr("Replace Fonts…"), "a-large-small", QKeySequence(), [this] { replaceFonts(); });
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

void WriteWindow::replaceFonts()
{
    if (!m_doc || refuseIfReadOnly()) return;
    m_view->leaveHeaderFooter();
    m_view->leaveNote();
    ReplaceFontsHost host;
    host.fontsUsed = [this] { return write::fontsUsed(*m_doc); };
    host.replace = [this](const QString &from, const QString &to) { return write::replaceFont(*m_doc, from, to); };
    const ReplaceFontsResult done = askReplaceFonts(this, host);
    statusBar()->showMessage(done.uses > 0 ? done.message() : tr("No fonts were replaced."), 6000);
    m_view->setFocus(Qt::OtherFocusReason);
}

} // namespace jp
