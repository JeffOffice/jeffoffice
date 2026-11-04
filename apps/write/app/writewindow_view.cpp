// JeffWrite's View tab, the parts beyond the zoom: the Navigation pane and Focus mode.

#include "app/writewindow.h"

#include "app/navpane.h"
#include "app/pageview.h"
#include "app/ribbon.h"
#include "app/ruler.h"

#include <QAction>
#include <QDockWidget>
#include <QStatusBar>
#include <QTreeWidget>

namespace jp {

void WriteWindow::createViewExtraActions()
{
    mk("view.navigationPane", tr("Navigation Pane"), "panel-left", QKeySequence(), [this] { showNavigationPane(act("view.navigationPane")->isChecked()); }, true);
    mk("view.focus", tr("Focus"), "focus", QKeySequence(Qt::Key_F11), [this] { setFocusMode(!m_focusMode); });
}

void WriteWindow::showNavigationPane(bool show)
{
    if (show && !m_nav) {
        m_navDock = new QDockWidget(tr("Navigation"), this);
        m_navDock->setObjectName(QStringLiteral("navigationDock"));
        m_navDock->setFeatures(QDockWidget::DockWidgetClosable);
        m_nav = new NavigationPane(this, m_navDock);
        m_navDock->setWidget(m_nav);
        addDockWidget(Qt::LeftDockWidgetArea, m_navDock);
        resizeDocks({m_navDock}, {260}, Qt::Horizontal);
        connect(m_navDock, &QDockWidget::visibilityChanged, this, [this](bool visible) {
            act("view.navigationPane")->setChecked(visible);
            if (visible && m_nav) m_nav->refresh();
        });
    }
    if (!m_navDock) return;
    m_navDock->setVisible(show);
    act("view.navigationPane")->setChecked(show);
    if (show) {
        m_nav->refresh();
        m_nav->headings()->setFocus();
    } else {
        m_view->setFocus(Qt::OtherFocusReason);
    }
}

// Focus: the ribbon, the ruler, the status bar, and the panes out of the way, the window full screen; the same command (or Esc) brings them back.
void WriteWindow::setFocusMode(bool on)
{
    if (m_focusMode == on) return;
    m_focusMode = on;
    if (on) {
        m_focusWas.fullScreen = isFullScreen();
        m_focusWas.ribbon = m_ribbon->isVisible();
        m_focusWas.ruler = m_ruler->isVisible();
        m_focusWas.status = statusBar()->isVisible();
        m_focusWas.docks.clear();
        for (QDockWidget *d : findChildren<QDockWidget *>())
            if (d->isVisible()) {
                m_focusWas.docks << d;
                d->hide();
            }
        m_ribbon->hide();
        m_ruler->hide();
        statusBar()->hide();
        showFullScreen();
    } else {
        if (!m_focusWas.fullScreen) showNormal();
        m_ribbon->setVisible(m_focusWas.ribbon);
        m_ruler->setVisible(m_focusWas.ruler);
        statusBar()->setVisible(m_focusWas.status);
        for (const QPointer<QDockWidget> &d : std::as_const(m_focusWas.docks))
            if (d) d->show();
        m_focusWas.docks.clear();
    }
    act("view.focus")->setChecked(on);
    m_view->setFocus(Qt::OtherFocusReason);
}

void WriteWindow::refreshViewUi()
{
    if (m_nav && m_navDock && m_navDock->isVisible()) m_nav->refresh();
}

} // namespace jp
