// The outline of the rows and columns in the window: the margins with the brackets and buttons beside the rows and above the columns, the commands that group and ungroup the selected rows and
// columns, and what a click on a button does (hide or show a group's lines, or show the outline down to a level). The sheet is changed by the session, one step for each.

#include "app/crunchwindow.h"

#include "app/outlinegutter.h"
#include "core/editsession.h"
#include "grid/gridview.h"
#include "grid/pagelayoutview.h"

#include <QAction>
#include <QGridLayout>
#include <QUndoStack>

#include <algorithm>

namespace jp {

void CrunchWindow::createOutlineActions()
{
    mk("outline.groupRows", tr("Group Rows"), "list-tree", QKeySequence(), [this] { groupSelected(false, true); }, false, false);
    mk("outline.ungroupRows", tr("Ungroup Rows"), "list-x", QKeySequence(), [this] { groupSelected(false, false); }, false, false);
    mk("outline.groupColumns", tr("Group Columns"), "list-tree", QKeySequence(), [this] { groupSelected(true, true); }, false, false);
    mk("outline.ungroupColumns", tr("Ungroup Columns"), "list-x", QKeySequence(), [this] { groupSelected(true, false); }, false, false);
    mk("view.outline", tr("Outline Symbols"), "list-tree", QKeySequence(), [this] { refreshOutline(); }, true)->setChecked(true);
    for (const char *id : {"outline.groupRows", "outline.ungroupRows", "outline.groupColumns", "outline.ungroupColumns"}) m_opsActions << QString::fromLatin1(id);
}

// The widgets beside the rows and above the columns, with the grid in the middle of them.
QWidget *CrunchWindow::buildOutlineGutters(QWidget *parent)
{
    auto *host = new QWidget(parent);
    auto *layout = new QGridLayout(host);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    m_rowGutter = new OutlineGutter(false, m_view, host);
    m_columnGutter = new OutlineGutter(true, m_view, host);
    m_view->setParent(host);
    layout->addWidget(m_columnGutter, 0, 1);
    layout->addWidget(m_rowGutter, 1, 0);
    layout->addWidget(m_view, 1, 1);
    layout->setRowStretch(1, 1);
    layout->setColumnStretch(1, 1);
    auto clicked = [this](bool columns) {
        return [this, columns](quint32 first, quint32 last, int level, bool collapsed, qint64 summary) {
            crunch::Outline::Group g;
            g.first = first;
            g.last = last;
            g.level = level;
            g.collapsed = collapsed;
            g.summary = summary;
            outlineGroupClicked(columns, g);
        };
    };
    connect(m_rowGutter, &OutlineGutter::groupClicked, this, clicked(false));
    connect(m_columnGutter, &OutlineGutter::groupClicked, this, clicked(true));
    connect(m_rowGutter, &OutlineGutter::levelClicked, this, [this](int level) { outlineLevelClicked(false, level); });
    connect(m_columnGutter, &OutlineGutter::levelClicked, this, [this](int level) { outlineLevelClicked(true, level); });
    connect(m_view, &grid::GridView::viewChanged, this, [this] {
        if (m_rowGutter) m_rowGutter->update();
        if (m_columnGutter) m_columnGutter->update();
    });
    return host;
}

// The outline of the sheet in view, worked out again (after the sheet is shown, or a step changed its rows or columns).
void CrunchWindow::refreshOutline()
{
    if (!m_rowGutter || !m_columnGutter) return;
    const bool show = m_session && m_loaded && m_haveCalc && m_sheet >= 0 && act("view.outline")->isChecked() && !(m_pageView && m_pageView->isVisible());
    if (!show) {
        m_rowGutter->setOutline(nullptr);
        m_columnGutter->setOutline(nullptr);
        return;
    }
    m_rowGutter->setOutline(std::make_shared<const crunch::Outline>(m_session->outlineOf(m_sheet, false)));
    m_columnGutter->setOutline(std::make_shared<const crunch::Outline>(m_session->outlineOf(m_sheet, true)));
}

void CrunchWindow::outlineGroupClicked(bool columns, const crunch::Outline::Group &group)
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return;
    const crunch::Outline outline = m_session->outlineOf(m_sheet, columns);
    const auto changes = group.collapsed ? outline.expand(group) : outline.collapse(group);
    const crunch::EditSession::Outcome o = m_session->changeOutline(m_sheet, columns, changes, group.collapsed ? tr("Show Detail") : tr("Hide Detail"));
    if (!o.ok) notice(tr("Outline"), o.problem);
}

void CrunchWindow::outlineLevelClicked(bool columns, int level)
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return;
    const crunch::Outline outline = m_session->outlineOf(m_sheet, columns);
    const crunch::EditSession::Outcome o = m_session->changeOutline(m_sheet, columns, outline.showButton(level), tr("Show Level %1").arg(level));
    if (!o.ok) notice(tr("Outline"), o.problem);
}

// Group or Ungroup: the rows (columns) of the selection get a level more (less).
void CrunchWindow::groupSelected(bool columns, bool group)
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return;
    const grid::GridRange r = m_view->selection().range;
    const quint32 first = quint32(std::max<qint64>(0, columns ? r.col1 : r.row1)), last = quint32(std::max<qint64>(0, columns ? r.col2 : r.row2));
    const crunch::Outline outline = m_session->outlineOf(m_sheet, columns);
    const auto changes = group ? outline.group(first, last) : outline.ungroup(first, last);
    if (changes.empty()) {
        notice(tr("Outline"), group ? tr("These %1 are already in the deepest group of an outline, which has 7 levels.").arg(columns ? tr("columns") : tr("rows"))
                                    : tr("These %1 are not in a group.").arg(columns ? tr("columns") : tr("rows")));
        return;
    }
    const crunch::EditSession::Outcome o = m_session->changeOutline(m_sheet, columns, changes, group ? (columns ? tr("Group Columns") : tr("Group Rows")) : (columns ? tr("Ungroup Columns") : tr("Ungroup Rows")));
    if (!o.ok) notice(tr("Outline"), o.problem);
}

} // namespace jp
