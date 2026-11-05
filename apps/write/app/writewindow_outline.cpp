// Outline view: View > Outline shows the paragraphs as a tree in place of the pages, and the Outlining tab (there while it is) promotes, demotes, and moves headings, and shows the document
// down to a level. The work on the document is core/outlineops.h.

#include "app/writewindow.h"

#include "app/writeoutlineview.h"
#include "app/pageview.h"
#include "app/ribbon.h"
#include "app/ruler.h"
#include "core/outlineops.h"

#include <QAction>
#include <QActionGroup>
#include <QStatusBar>

namespace jp {

void WriteWindow::createOutlineActions()
{
    mk("view.outline", tr("Outline"), "list-tree", QKeySequence(), [this] { setOutlineMode(act("view.outline")->isChecked()); }, true);
    // Print Layout is the other view: choosing it leaves the outline.
    disconnect(act("view.printLayout"), nullptr, nullptr, nullptr);
    connect(act("view.printLayout"), &QAction::triggered, this, [this] { setOutlineMode(false); });
    auto onRows = [this](const std::function<void(write::WriteDoc &, int, int)> &run) {
        return [this, run] {
            if (!m_doc || !m_outlineOn || isReadOnly()) return;
            const QPair<int, int> range = m_outline->selectedRange();
            if (range.first < 0) {
                statusBar()->showMessage(tr("Select a heading or a paragraph in the outline first."), 5000);
                return;
            }
            run(*m_doc, range.first, range.second);
            outlineEdited();
            m_outline->selectParagraphAt(range.first);
        };
    };
    mk("outline.promote", tr("Promote"), "arrow-left", QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_Left), onRows([this](write::WriteDoc &d, int a, int b) { write::outlineShift(d, a, b, -1, true); }));
    mk("outline.demote", tr("Demote"), "arrow-right", QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_Right), onRows([this](write::WriteDoc &d, int a, int b) { write::outlineShift(d, a, b, 1, true); }));
    mk("outline.toHeading1", tr("Promote to Heading 1"), "chevrons-left", QKeySequence(), onRows([](write::WriteDoc &d, int a, int b) { write::outlineSetLevel(d, a, b, 0); }));
    mk("outline.toBody", tr("Demote to Body Text"), "chevrons-right", QKeySequence(), onRows([](write::WriteDoc &d, int a, int b) { write::outlineSetLevel(d, a, b, write::kBodyLevel); }));
    auto move = [this](bool up) {
        return [this, up] {
            if (!m_doc || !m_outlineOn || isReadOnly()) return;
            const QPair<int, int> range = m_outline->selectedRange();
            if (range.first < 0) return;
            int at = -1;
            if (write::outlineMove(*m_doc, range.first, up, true, &at)) {
                outlineEdited();
                m_outline->selectParagraphAt(at);
            } else {
                statusBar()->showMessage(up ? tr("It can't move up from here.") : tr("It can't move down from here."), 5000);
            }
        };
    };
    mk("outline.moveUp", tr("Move Up"), "arrow-up", QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_Up), move(true));
    mk("outline.moveDown", tr("Move Down"), "arrow-down", QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_Down), move(false));
    mk("outline.expand", tr("Expand"), "plus", QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_Plus), [this] { m_outline->expandSelected(true); });
    mk("outline.collapse", tr("Collapse"), "minus", QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_Minus), [this] { m_outline->expandSelected(false); });
    auto *levels = new QActionGroup(this);
    for (int n = 1; n <= 9; ++n) {
        QAction *a = mk(QStringLiteral("outline.level.%1").arg(n), tr("Show Level %1").arg(n), "", QKeySequence(int(Qt::ALT | Qt::SHIFT) + int(Qt::Key_0) + n), [this, n] { m_outline->setShowLevel(n); }, true);
        levels->addAction(a);
    }
    QAction *all = mk("outline.level.all", tr("All Levels"), "", QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_A), [this] { m_outline->setShowLevel(0); }, true);
    levels->addAction(all);
    all->setChecked(true);
    mk("outline.showFormatting", tr("Show Text Formatting"), "text", QKeySequence(), [this] { m_outline->setShowFormatting(act("outline.showFormatting")->isChecked()); }, true)->setChecked(true);
    mk("outline.firstLineOnly", tr("Show First Line Only"), "text-quote", QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_L), [this] { m_outline->setFirstLineOnly(act("outline.firstLineOnly")->isChecked()); }, true);
    mk("outline.close", tr("Close Outline View"), "x", QKeySequence(), [this] { setOutlineMode(false); });
    // (They work in the outline only, and so are off, and their keys free, otherwise.)
    for (QAction *a : m_actions)
        if (a->objectName().startsWith(QLatin1String("outline."))) a->setEnabled(false);
    connect(m_outline, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem *item) {
        // (A double click on a row opens the text there in the pages; the arrow in front of a heading expands and collapses it.)
        if (item && item->childCount() == 0) {
            const int at = WriteOutlineView::positionOf(item);
            setOutlineMode(false);
            if (m_doc) {
                QTextCursor c(m_doc->story());
                c.setPosition(at);
                m_view->showCursor(c);
            }
        }
    });
}

// The text changed through the outline (or from Undo and Redo while it is shown): the rows are made again.
void WriteWindow::outlineEdited()
{
    m_outline->rebuild();
    refreshUi();
}

void WriteWindow::setOutlineMode(bool on)
{
    if (on == m_outlineOn) return;
    if (on && !m_doc) {
        act("view.outline")->setChecked(false);
        return;
    }
    m_outlineOn = on;
    act("view.outline")->setChecked(on);
    act("view.printLayout")->setChecked(!on);
    m_ribbon->setContextVisible(QStringLiteral("Outlining"), on, true);
    for (QAction *a : m_actions)
        if (a->objectName().startsWith(QLatin1String("outline."))) a->setEnabled(on);
    if (on) {
        // The pages and the ruler give way to the tree; it opens on the paragraph the caret is in.
        const int at = m_view->cursor().position();
        m_view->leaveHeaderFooter();
        m_view->leaveNote();
        m_ruler->hide();
        m_view->hide();
        m_outline->setDocument(m_doc.get());
        m_outline->show();
        m_outline->selectParagraphAt(at);
        m_outline->setFocus(Qt::OtherFocusReason);
    } else {
        // The caret goes to the start of the paragraph selected.
        const int at = m_outline->selectedRange().first;
        m_outline->hide();
        m_view->show();
        m_ruler->setVisible(act("view.ruler")->isChecked());
        if (at >= 0 && m_doc) {
            QTextCursor c(m_doc->story());
            c.setPosition(qMin(at, m_doc->story()->characterCount() - 1));
            m_view->showCursor(c);
        }
        m_outline->setDocument(nullptr);
        m_view->setFocus(Qt::OtherFocusReason);
    }
    refreshUi();
}

} // namespace jp
