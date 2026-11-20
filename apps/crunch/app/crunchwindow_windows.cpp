// View > Window: New Window (the workbook's file in a second window, for reading), Arrange All and Cascade (the windows set across the screen or in a stack), View Side by Side with
// Synchronous Scrolling (two windows each on half of the screen, scrolling together), Reset Window Position, and the list to Switch Windows by.

#include "app/crunchwindow.h"

#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "grid/gridview.h"

#include <QAction>
#include <QInputDialog>
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
    mk("win.hide", tr("Hide Window"), "eye-off", QKeySequence(), [this] { hideThisWindow(); });
    mk("win.unhide", tr("Unhide Window…"), "eye", QKeySequence(), [this] { unhideAWindow(); });
    mk("view.resetWindowPosition", tr("Reset Window Position"), "layout-panel-left", QKeySequence(), [this] {
        if (m_sidePartner) AppWindow::placeSideBySide(this, m_sidePartner);
        else setStatus(tr("Turn on View Side by Side first: this puts the two windows back on the halves of the screen."));
    });
}

// Hide Window: the window leaves the screen and the lists of windows, with its workbook open; Unhide Window brings one back. At least one other window stays in view (a program with every
// window hidden would be a program with nothing on the screen to ask for them), and a window tied to another side by side is let go first.
void CrunchWindow::hideThisWindow()
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active) return;
    CrunchWindow *next = nullptr;
    for (CrunchWindow *w : crunchWindows())
        if (w != this && !next) next = w;
    if (!next) {
        notice(tr("Hide Window"), tr("This is the only window that is open. Open or create another workbook first: a hidden window is brought back with Unhide Window in another one."));
        return;
    }
    if (m_sidePartner) setSideBySide(false);
    m_hiddenOnPurpose = true;
    hide();
    next->raise();
    next->activateWindow();
    next->setStatus(tr("The window \"%1\" is hidden. Unhide Window on the View tab brings it back.").arg(windowTitle()));
}

// The program is quitting and this window is out of view: when it has work that is not saved it comes back into view and asks as a closing window does (Cancel there cancels the quit).
bool CrunchWindow::readyToQuit()
{
    if (!m_hiddenOnPurpose || !m_session || (!m_session->isModified() && !m_saving && !m_protectionChanged)) return true;
    m_hiddenOnPurpose = false;
    show();
    raise();
    activateWindow();
    return close();
}

void CrunchWindow::unhideAWindow()
{
    const QList<CrunchWindow *> hidden = hiddenWindows();
    if (hidden.isEmpty()) {
        setStatus(tr("No window is hidden."));
        return;
    }
    // (two views of one file have one title: the second is numbered, so that each can be told from the other)
    QStringList titles;
    for (CrunchWindow *w : hidden) {
        QString title = w->windowTitle();
        for (int n = 2; titles.contains(title); ++n) title = tr("%1 (%2)").arg(w->windowTitle()).arg(n);
        titles << title;
    }
    int pick = 0;
    if (hidden.size() > 1) {
        if (m_windowPicker) {
            pick = m_windowPicker(titles);
        } else {
            bool ok = false;
            const QString chosen = QInputDialog::getItem(this, tr("Unhide Window"), tr("Window to show"), titles, 0, false, &ok);
            pick = ok ? int(titles.indexOf(chosen)) : -1;
        }
    }
    if (pick < 0 || pick >= hidden.size()) return;
    CrunchWindow *w = hidden[pick];
    w->m_hiddenOnPurpose = false;
    w->show();
    w->raise();
    w->activateWindow();
    w->setStatus(tr("The window \"%1\" is shown again.").arg(w->windowTitle()));
}

void CrunchWindow::setSynchronousScrolling(bool on)
{
    m_syncScroll = on;
    if (m_sidePartner) m_sidePartner->m_syncScroll = on;
    act(QStringLiteral("view.syncScroll"))->setChecked(on);
    setStatus(on ? tr("Synchronous Scrolling is on: the two windows scroll together.") : tr("Synchronous Scrolling is off: each window scrolls on its own."));
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
    AppWindow::placeSideBySide(this, partner);
}

void CrunchWindow::windowRibbonParts(RibbonParts &parts)
{
    parts.menus[QStringLiteral("switchWindowsMenu")] = [](QWidget *owner) { return AppWindow::switchWindowsMenu(owner, CrunchWindow::staticMetaObject); };
}

} // namespace jp
