// Home > Editing: Find and Replace (the pane on the right), Go To, and Select Objects.

#include "app/deckwindow.h"

#include "app/editor.h"
#include "app/gotodialog.h"
#include "app/deckfindpane.h"
#include "app/pagefonts.h"
#include "app/replacefonts.h"
#include "core/themes.h"

#include <QAction>
#include <QStatusBar>
#include <QDockWidget>

namespace jp {

void DeckWindow::createEditingActions()
{
    Editor *ed = m_ed;
    mk("edit.find", tr("Find"), "search", QKeySequence::Find, [this] { showFindPane(false); });
    mk("edit.replace", tr("Replace"), "replace", QKeySequence(Qt::CTRL | Qt::Key_H), [this] { showFindPane(true); });
    mk("edit.replaceFonts", tr("Replace Fonts…"), "a-large-small", QKeySequence(), [this] { replaceFonts(); });
    mk("edit.goTo", tr("Go To…"), "locate-fixed", QKeySequence(Qt::CTRL | Qt::Key_G), [this] { showGoToDialog(this); });
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

void DeckWindow::replaceFonts()
{
    // The theme's own font: the presentation's font scheme changes (as Design > Fonts does), and text set in it follows.
    const auto theme = [](Document &d, const QString &key, const QString &to) {
        deck::FontSet scheme = deck::fontSetOf(d);
        (key == QLatin1String("+Heading") ? scheme.heading : scheme.body) = to;
        int n = 0;
        for (const FontUse &u : pageFontsUsed(d))
            if (u.key == key) n = u.uses;
        deck::applyFonts(d, scheme);
        return n;
    };
    const ReplaceFontsResult done = askReplacePageFonts(this, m_ed, theme);
    statusBar()->showMessage(done.uses > 0 ? done.message() : tr("No fonts were replaced."), 6000);
}

} // namespace jp
