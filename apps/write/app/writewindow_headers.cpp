// Insert > Header and Footer, and the Header & Footer tab that is there while one is edited: Close, Insert Page Number, and the options (a different first page, different odd and
// even pages, and linking to the section before's).

#include "app/writewindow.h"

#include "app/pageview.h"
#include "app/ribbon.h"
#include "core/editops.h"

#include <QAction>

namespace jp {

void WriteWindow::createHeaderFooterActions()
{
    write::PageView *v = m_view;
    mk("insert.header", tr("Header"), "panel-top", QKeySequence(), [this] {
        if (m_doc) m_view->enterHeaderFooterAtCaret(false);
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("insert.footer", tr("Footer"), "panel-bottom", QKeySequence(), [this] {
        if (m_doc) m_view->enterHeaderFooterAtCaret(true);
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("hf.close", tr("Close Header and Footer"), "circle-x", QKeySequence(), [v] {
        v->leaveHeaderFooter();
        v->setFocus(Qt::OtherFocusReason);
    });
    mk("hf.pageNumber", tr("Page Number"), "hash", QKeySequence(), [this] {
        if (m_doc && m_view->inHeaderFooter()) write::insertPageNumber(*m_doc, m_view->cursor());
        m_view->setFocus(Qt::OtherFocusReason);
        m_view->viewport()->update();
    });
    mk("hf.differentFirst", tr("Different First Page"), "file-digit", QKeySequence(), [this] {
        if (!m_doc || !m_view->inHeaderFooter()) return;
        m_doc->setDifferentFirstPage(m_view->headerFooterEdit().section, act("hf.differentFirst")->isChecked());
        m_view->refreshHeaderFooter(true);
        m_view->setFocus(Qt::OtherFocusReason);
    }, true);
    mk("hf.differentOddEven", tr("Different Odd && Even Pages"), "file-stack", QKeySequence(), [this] {
        if (!m_doc || !m_view->inHeaderFooter()) return;
        m_doc->setDifferentOddEven(act("hf.differentOddEven")->isChecked());
        m_view->refreshHeaderFooter(true);
        m_view->setFocus(Qt::OtherFocusReason);
    }, true);
    mk("hf.linkPrevious", tr("Link to Previous"), "link", QKeySequence(), [this] {
        if (!m_doc || !m_view->inHeaderFooter()) return;
        const write::PageView::HeaderFooterEdit e = m_view->headerFooterEdit();
        const bool linked = act("hf.linkPrevious")->isChecked();
        m_doc->setLinkedToPrevious(e.section, e.footer, e.slot, linked);
        // (Linked, the section shows the section before's, or nothing if that has none; unlinked, the copy it has now.)
        m_view->refreshHeaderFooter(!linked);
        m_view->setFocus(Qt::OtherFocusReason);
    }, true);
}

// A header or footer is edited now, or none is any more: the tab that holds its controls comes and goes.
void WriteWindow::headerFooterModeChanged()
{
    const bool editing = m_view->inHeaderFooter();
    m_ribbon->setContextVisible(QStringLiteral("Header & Footer"), editing, true);
    refreshUi();
}

} // namespace jp
