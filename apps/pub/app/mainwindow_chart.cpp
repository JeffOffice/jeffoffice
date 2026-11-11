// Insert > Chart and the Chart Format tab. A chart is an object of the page (a picture drawn from its chart part: libs/page's core/chartobject.h), moved, sized, and turned like any other
// object; the commands are the ones JeffDeck's chart tools use too (app/chartactions.h), each one step that undoes. The tab shows while a chart is selected.

#include "app/mainwindow.h"

#include "app/chartactions.h"
#include "app/editor.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "canvas/canvas.h"

#include <QAction>
#include <QStatusBar>

namespace jp {

void MainWindow::createChartActions()
{
    ChartActions::Hooks hooks;
    hooks.make = [this](const QString &id, const QString &text, const QString &icon, const std::function<void()> &fn, bool checkable) { return mk(id, text, icon, QKeySequence(), fn, checkable); };
    hooks.find = [this](const QString &id) { return act(id); };
    hooks.notice = [this](const QString &text) { statusBar()->showMessage(text, 8000); };
    hooks.done = [this] { m_canvas->setFocus(Qt::OtherFocusReason); };
    m_chartActions = new ChartActions(m_ed, this, hooks, this);
    m_chartActions->create();
    auto sync = [this] { refreshChart(); };
    connect(m_ed, &Editor::selectionChanged, this, sync);
    connect(m_ed, &Editor::changed, this, sync);
    connect(m_ed, &Editor::pageChanged, this, sync);
    connect(m_ed, &Editor::documentReplaced, this, sync);
    refreshChart();
}

void MainWindow::addChartParts(RibbonParts &parts)
{
    parts.colors[QStringLiteral("chart")] = QColor(0x8A, 0x5A, 0xA8);
    parts.menus[QStringLiteral("insertChart")] = [this](QWidget *owner) { return m_chartActions->insertMenu(owner); };
    parts.menus[QStringLiteral("chartType")] = [this](QWidget *owner) { return m_chartActions->typeMenu(owner); };
    parts.menus[QStringLiteral("chartAxes")] = [this](QWidget *owner) { return m_chartActions->axesMenu(owner); };
    parts.menus[QStringLiteral("chartGridlines")] = [this](QWidget *owner) { return m_chartActions->gridlinesMenu(owner); };
    parts.menus[QStringLiteral("chartStyles")] = [this](QWidget *owner) { return m_chartActions->stylesMenu(owner); };
    parts.menus[QStringLiteral("chartLegend")] = [this](QWidget *owner) { return m_chartActions->legendMenu(owner); };
}

void MainWindow::refreshChart()
{
    if (!m_chartActions) return;
    const bool isChart = m_chartActions->refresh();
    if (m_ribbon) m_ribbon->setContextVisible(QStringLiteral("Chart Tools"), isChart);
}

} // namespace jp
