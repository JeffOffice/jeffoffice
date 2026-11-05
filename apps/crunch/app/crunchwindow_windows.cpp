// View > Window: New Window (the workbook's file in a second window, for reading), Arrange All and Cascade (the windows set across the screen or in a stack), View Side by Side with
// Synchronous Scrolling (two windows each on half of the screen, scrolling together), Reset Window Position, and the list to Switch Windows by.

#include "app/crunchwindow.h"

#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "grid/gridview.h"

#include <QAction>
#include <QMenu>
#include <QScrollBar>
#include <QStatusBar>

#include <memory>

namespace jp {

void CrunchWindow::createWindowActions()
{
    mk("win.new", tr("New Window"), "app-window", QKeySequence(), [this] {
        if (m_path.isEmpty() || !m_loaded) {
            setStatus(tr("Save the workbook first: a new window shows the file."));
            return;
        }
        auto *w = new CrunchWindow();
        w->setAttribute(Qt::WA_DeleteOnClose);
        w->m_closeIfOpenFails = true;
        w->setNotifier(m_notifier);
        w->m_promptForText = m_promptForText;
        w->m_anchorHooks = m_anchorHooks;
        w->m_secondView = true;
        w->show();
        if (!w->openFile(m_path, OpenHow::ReadOnly)) w->close();
    });
    mk("win.arrange", tr("Arrange All"), "layout-panel-left", QKeySequence(), [] {
        QList<AppWindow *> wins;
        for (CrunchWindow *w : crunchWindows()) wins << w;
        AppWindow::arrangeInColumns(wins);
    });
    mk("win.cascade", tr("Cascade"), "layers-2", QKeySequence(), [] {
        QList<AppWindow *> wins;
        for (CrunchWindow *w : crunchWindows()) wins << w;
        AppWindow::cascade(wins);
    });
    mk("view.sideBySide", tr("View Side by Side"), "columns-2", QKeySequence(), [this] { setSideBySide(act("view.sideBySide")->isChecked()); }, true);
    mk("view.syncScroll", tr("Synchronous Scrolling"), "arrow-down-up", QKeySequence(), [this] { setSynchronousScrolling(act("view.syncScroll")->isChecked()); }, true);
    act("view.syncScroll")->setChecked(true);
    mk("view.resetWindowPosition", tr("Reset Window Position"), "layout-panel-left", QKeySequence(), [this] {
        if (m_sidePartner) AppWindow::placeSideBySide(this, m_sidePartner);
    });
    // (both do something only while two windows are side by side: they are dimmed otherwise)
    act("view.syncScroll")->setEnabled(false);
    act("view.resetWindowPosition")->setEnabled(false);
}

// Synchronous Scrolling and Reset Window Position are on while this window has a partner.
void CrunchWindow::updateWindowActions()
{
    for (const char *id : {"view.syncScroll", "view.resetWindowPosition"}) act(QString::fromLatin1(id))->setEnabled(m_sidePartner != nullptr);
}

void CrunchWindow::setSynchronousScrolling(bool on)
{
    m_syncScroll = on;
    if (m_sidePartner) m_sidePartner->m_syncScroll = on;
    act(QStringLiteral("view.syncScroll"))->setChecked(on);
    if (m_sidePartner) m_sidePartner->act(QStringLiteral("view.syncScroll"))->setChecked(on);
}

void CrunchWindow::setSideBySide(bool on)
{
    // The windows that were tied are let go first; a window is tied to one other.
    auto untie = [](CrunchWindow *w) {
        if (!w) return;
        for (QMetaObject::Connection &c : w->m_sideScroll) QObject::disconnect(c);
        w->m_sidePartner = nullptr;
        if (w->act(QStringLiteral("view.sideBySide"))->isChecked()) w->act(QStringLiteral("view.sideBySide"))->setChecked(false);
        w->updateWindowActions();
    };
    if (!on) {
        CrunchWindow *other = m_sidePartner;
        untie(this);
        untie(other);
        return;
    }
    CrunchWindow *partner = nullptr;
    for (CrunchWindow *w : crunchWindows())
        if (w != this) {
            partner = w;
            break;
        }
    if (!partner) {
        act(QStringLiteral("view.sideBySide"))->setChecked(false);
        setStatus(tr("Open a second window to view two workbooks side by side."));
        return;
    }
    untie(m_sidePartner);
    untie(partner->m_sidePartner);
    untie(this);
    act(QStringLiteral("view.sideBySide"))->setChecked(true);
    partner->act(QStringLiteral("view.sideBySide"))->setChecked(true);
    m_sidePartner = partner;
    partner->m_sidePartner = this;
    // Each window's scrolling moves the other's by the same distance (while Synchronous Scrolling is on).
    auto follow = [](CrunchWindow *from, CrunchWindow *to, int which) {
        auto guard = std::make_shared<bool>(false);
        const std::pair<QScrollBar *, QScrollBar *> bars[2] = {{from->m_view->verticalScrollBar(), to->m_view->verticalScrollBar()},
                                                              {from->m_view->horizontalScrollBar(), to->m_view->horizontalScrollBar()}};
        for (int i = 0; i < 2; ++i) {
            QScrollBar *a = bars[i].first, *b = bars[i].second;
            from->m_sideScroll[which * 2 + i] = QObject::connect(a, &QScrollBar::valueChanged, to, [from, a, b, guard](int value) {
                if (*guard || !from->m_syncScroll) return;
                *guard = true;
                b->setValue(value);
                *guard = false;
            });
        }
    };
    follow(this, partner, 0);
    follow(partner, this, 1);
    updateWindowActions();
    partner->updateWindowActions();
    AppWindow::placeSideBySide(this, partner);
}

void CrunchWindow::windowRibbonParts(RibbonParts &parts)
{
    parts.menus[QStringLiteral("switchWindowsMenu")] = [](QWidget *owner) {
        auto *switchMenu = new QMenu(owner);
        QObject::connect(switchMenu, &QMenu::aboutToShow, switchMenu, [switchMenu] {
            switchMenu->clear();
            for (CrunchWindow *w : CrunchWindow::crunchWindows()) {
                QAction *a = switchMenu->addAction(w->windowTitle());
                QObject::connect(a, &QAction::triggered, w, [w] {
                    w->raise();
                    w->activateWindow();
                });
            }
        });
        return switchMenu;
    };
}

} // namespace jp
