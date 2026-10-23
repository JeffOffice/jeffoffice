// JeffWrite's Review tab: Spelling & Grammar, the pane that steps through the misspellings, and checking spelling as you type.

#include "app/writewindow.h"

#include "app/pageview.h"
#include "app/settings.h"
#include "app/spellpane.h"
#include "app/writefilescreen.h"

#include <QAction>
#include <QDockWidget>

namespace jp {

void WriteWindow::createReviewActions()
{
    mk("review.spelling", tr("Spelling & Grammar"), "spell-check", QKeySequence(Qt::Key_F7), [this] { showSpelling(); });
    QAction *asType = mk("review.checkAsType", tr("Check Spelling as You Type"), "spell-check-2", QKeySequence(), [this] {
        const bool on = act("review.checkAsType")->isChecked();
        m_view->setSpellingEnabled(on);
        Settings::get().setValue(QStringLiteral("proof/asYouType"), on);
    }, true);
    const bool on = Settings::get().value(QStringLiteral("proof/asYouType"), true).toBool();
    asType->setChecked(on);
    m_view->setSpellingEnabled(on);
}

void WriteWindow::showSpelling()
{
    if (!m_doc) return;
    m_view->leaveHeaderFooter();   // (the Spelling pane goes through the text of the document, not its headers and footers)
    m_view->leaveNote();
    if (m_fileScreen->isVisible()) hideFileScreen();
    if (!m_spellDock) {
        m_spellDock = new QDockWidget(tr("Spelling"), this);
        m_spellDock->setObjectName(QStringLiteral("spellDock"));
        m_spellDock->setFeatures(QDockWidget::DockWidgetClosable);
        m_spellPane = new write::SpellPane(m_view, m_spellDock);
        m_spellDock->setWidget(m_spellPane);
        addDockWidget(Qt::RightDockWidgetArea, m_spellDock);
        resizeDocks({m_spellDock}, {320}, Qt::Horizontal);
    }
    m_spellDock->show();
    m_spellDock->raise();
    m_spellPane->start();
}

} // namespace jp
