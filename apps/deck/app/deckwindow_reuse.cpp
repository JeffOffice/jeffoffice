// JeffDeck's Reuse Slides command: the pane on the right of the window, and
// the copy of a slide it asks for, one step that undoes.

#include "app/deckwindow.h"

#include "app/editor.h"
#include "app/notespane.h"
#include "app/outlineview.h"
#include "app/reusepane.h"
#include "core/reuse.h"

#include <QAction>
#include <QDockWidget>

namespace jp {

void DeckWindow::createReuseActions()
{
    mk("slide.reuse", tr("Reuse Slides…"), "import", QKeySequence(), [this] { showReusePane(); });
}

void DeckWindow::showReusePane()
{
    if (!m_reuse) {
        m_reuseDock = new QDockWidget(tr("Reuse Slides"), this);
        m_reuseDock->setObjectName(QStringLiteral("reuseDock"));
        m_reuseDock->setFeatures(QDockWidget::DockWidgetClosable);
        m_reuse = new ReusePane(m_reuseDock);
        m_reuseDock->setWidget(m_reuse);
        addDockWidget(Qt::RightDockWidgetArea, m_reuseDock);
        resizeDocks({m_reuseDock}, {280}, Qt::Horizontal);
        connect(m_reuse, &ReusePane::slideChosen, this, &DeckWindow::reuseSlide);
    }
    m_reuseDock->show();
    m_reuseDock->raise();
}

void DeckWindow::reuseSlide(int slide, bool keepFormatting)
{
    const Document *from = m_reuse ? m_reuse->source() : nullptr;
    if (!from || slide < 0 || slide >= from->pages.size()) return;
    m_notes->commit();
    if (m_outline) m_outline->commit();
    m_ed->endTextEdit();
    m_ed->beginChange(tr("Reuse Slide"));
    const int index = deck::insertSlideFrom(*m_ed->doc(), m_ed->currentPage() + 1, *from, slide, keepFormatting);
    if (index >= 0) m_ed->setCurrentPage(index);
    m_ed->endChange();
}

} // namespace jp
