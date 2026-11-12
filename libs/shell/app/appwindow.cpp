#include "app/appwindow.h"

#include "app/ribbon.h"

#include <QAction>
#include <QApplication>
#include <QEvent>
#include <QAbstractSpinBox>
#include <QComboBox>
#include <QKeyEvent>
#include <QWindowStateChangeEvent>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QScreen>
#include <QSet>
#include <QMenu>
#include <QToolButton>
#include <QStatusBar>

namespace jp {

QList<AppWindow *> AppWindow::windows()
{
    QList<AppWindow *> out;
    for (QWidget *w : QApplication::topLevelWidgets())
        if (auto *a = qobject_cast<AppWindow *>(w)) out << a;
    return out;
}

void AppWindow::openOptions(OptionsStart start)
{
    QAction *options = act(QStringLiteral("file.options"));
    if (!options || !options->isEnabled()) return;
    m_optionsStart = start;
    options->trigger();
    m_optionsStart = OptionsStart::Default;
}

AppWindow::OptionsStart AppWindow::takeOptionsStart()
{
    const OptionsStart start = m_optionsStart;
    m_optionsStart = OptionsStart::Default;
    return start;
}

void AppWindow::toggleFullScreen()
{
    m_fullScreenWanted = !m_fullScreenWanted;
    if (m_fullScreenWanted) {
        if (!isFullScreen()) m_wasMaximized = isMaximized();   // (kept to put it back as it was)
        showFullScreen();
    } else if (m_wasMaximized) {
        showMaximized();
    } else {
        showNormal();
    }
    if (QAction *a = act(QStringLiteral("view.fullScreen"))) a->setChecked(m_fullScreenWanted);
}

void AppWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    if (e->type() != QEvent::WindowStateChange) return;
    // (The window system's own button or key can change it too.)
    if (isFullScreen() != m_fullScreenWanted) {
        if (isFullScreen()) m_wasMaximized = static_cast<QWindowStateChangeEvent *>(e)->oldState().testFlag(Qt::WindowMaximized);
        m_fullScreenWanted = isFullScreen();
    }
    if (QAction *a = act(QStringLiteral("view.fullScreen"))) a->setChecked(m_fullScreenWanted);
}

bool AppWindow::escapeLeavesFullScreen(QObject *target, QEvent *event, bool otherWantsIt)
{
    if (event->type() != QEvent::KeyPress || static_cast<QKeyEvent *>(event)->key() != Qt::Key_Escape || !isFullScreen() || otherWantsIt) return false;
    if (QApplication::activePopupWidget() || QApplication::activeModalWidget()) return false;
    auto *w = qobject_cast<QWidget *>(target);
    if (!w || w->window() != this) return false;
    // (Esc in a box that is being typed in, or a list that is open, is that box's.)
    for (const QWidget *p = w; p && p != this; p = p->parentWidget())
        if (qobject_cast<const Ribbon *>(p) || qobject_cast<const QLineEdit *>(p) || qobject_cast<const QTextEdit *>(p) || qobject_cast<const QPlainTextEdit *>(p) || qobject_cast<const QAbstractSpinBox *>(p)
            || qobject_cast<const QComboBox *>(p))
            return false;
    toggleFullScreen();
    return true;
}

QMenu *AppWindow::switchWindowsMenu(QWidget *owner, const QMetaObject &kind)
{
    auto *switchMenu = new QMenu(owner);
    QObject::connect(switchMenu, &QMenu::aboutToShow, switchMenu, [switchMenu, &kind] {
        switchMenu->clear();
        for (AppWindow *w : AppWindow::windows()) {
            if (!w->isVisible() || !kind.cast(w)) continue;
            QAction *a = switchMenu->addAction(w->windowTitle());
            QObject::connect(a, &QAction::triggered, w, [w] {
                w->raise();
                w->activateWindow();
            });
        }
    });
    return switchMenu;
}

void AppWindow::arrangeInColumns(const QList<AppWindow *> &wins)
{
    if (wins.isEmpty()) return;
    const QRect area = wins.first()->screen()->availableGeometry();
    const int each = area.width() / int(wins.size());
    for (int i = 0; i < wins.size(); ++i) {
        wins[i]->showNormal();
        wins[i]->setGeometry(area.left() + i * each, area.top(), each, area.height());
    }
}

void AppWindow::cascade(const QList<AppWindow *> &wins)
{
    int k = 0;
    for (AppWindow *w : wins) {
        w->showNormal();
        w->move(40 + k * 30, 40 + k * 30);
        w->raise();
        ++k;
    }
}

void AppWindow::placeSideBySide(AppWindow *left, AppWindow *right)
{
    const QRect area = left->screen()->availableGeometry();
    left->showNormal();
    right->showNormal();
    left->setGeometry(area.left(), area.top(), area.width() / 2, area.height());
    right->setGeometry(area.left() + area.width() / 2, area.top(), area.width() / 2, area.height());
}

bool AppWindow::commandAllowedReadOnly(const QString &id)
{
    // The commands that only look, find, copy, open, save elsewhere, or leave: each named, so a command that is added later is refused until it is put here. (Save and Save As are here
    // because each of them handles a document open for reading only itself: a new name, never the file's own.) The commands a person can reach in a read-only document and that
    // change it are not here: Replace, the slide show's set-up, its timings, its custom shows, freezing panes. (Pub's Info command only opens the Info page, which shows a read-only file's facts.)
    static const QSet<QString> allowed = [] {
        QSet<QString> ids;
        for (const char *id : {
                 "file.blank", "file.close", "file.exportHtml", "file.exportImages", "file.exportPdf", "file.exportPicture", "file.new", "file.open", "file.open.jpub", "file.open.pub",
                 "file.openDialog", "file.openText", "file.options", "file.properties", "file.print", "file.printNow", "file.save", "file.save.jpub", "file.save.pub", "file.saveAs", "file.saveAsPub",
                 "file.saveCopy", "file.saveTemplate",
                 "find.find", "find.goto", "find.gotoSpecial", "find.next",
                 "help.feedback", "help.shortcuts", "help.show", "help.whatsNew", "view.pageMovement.sideToSide", "view.pageMovement.vertical", "view.readMode", "view.draft", "view.webLayout", "view.split",
                 "show.fromCurrent", "show.fromStart", "show.presenterView",
                 "view.baselines", "view.boundaries", "view.collapseRibbon", "view.fullScreen", "view.fields", "view.fit", "view.formulaBar", "view.formulas", "view.graphics", "view.gridlines", "view.guides",
                 "view.headings", "view.marks", "view.master", "view.measurement", "view.normal", "view.notes", "view.outline", "view.pageBreakView", "view.pageBreaks", "view.pageLayout",
                 "view.navigationPane", "view.focus", "view.pageNav", "view.printLayout", "view.ruler", "view.rulers", "view.scratch", "view.single", "view.slideShow", "view.sorter", "view.spread",
                 "win.arrange", "win.cascade", "win.new",
                 "zoom.10", "zoom.100", "zoom.150", "zoom.200", "zoom.25", "zoom.300", "zoom.33", "zoom.400", "zoom.50", "zoom.66", "zoom.75", "zoom.dialog", "zoom.in", "zoom.multiplePages",
                 "zoom.onePage", "zoom.out", "zoom.page", "zoom.pageWidth", "zoom.selection", "zoom.width",
                 "comment.next", "comment.prev", "comment.show",
                 "edit.copy", "edit.selectAll", "edit.find", "edit.goTo", "edit.selectObjects",
                 "review.nextChange", "review.previousChange", "review.nextComment", "review.previousComment", "review.reviewingPane", "review.wordCount", "review.accessibility", "review.translate", "review.compare"})
            ids.insert(QLatin1String(id));
        return ids;
    }();
    return allowed.contains(id);
}

namespace {

// Whether every command in a menu (and its submenus) may be used in a read-only document. A command made in code with no id is not known to be safe.
bool menuAllowedReadOnly(const QMenu *menu)
{
    if (menu->actions().isEmpty()) return false;   // (a menu that is filled when it opens is not known to be safe)
    for (const QAction *a : menu->actions()) {
        if (a->isSeparator()) continue;
        if (const QMenu *sub = a->menu()) {
            if (!menuAllowedReadOnly(sub)) return false;
        } else if (!AppWindow::commandAllowedReadOnly(a->objectName())) {
            return false;
        }
    }
    return true;
}

// Turns the controls of a tab that are not safe in a read-only document off (or back on).
void applyReadOnlyToTab(RibbonTab *tab, bool on)
{
    for (RibbonGroup *g : tab->groups())
        for (QWidget *c : g->controls()) {
            auto *button = qobject_cast<QToolButton *>(c);
            bool safe = true;
            if (button && button->menu()) safe = menuAllowedReadOnly(button->menu());
            else if (button && button->defaultAction()) safe = true;   // (a button for a command: the command is refused, or off, by its id)
            else safe = AppWindow::ribbonControlAllowedReadOnly(c->property("ribbonWidget").toString());
            if (safe) continue;
            if (on) {
                if (c->isEnabled()) {
                    c->setProperty("readOnlyOff", true);
                    c->setEnabled(false);
                }
            } else if (c->property("readOnlyOff").toBool()) {
                c->setProperty("readOnlyOff", false);
                c->setEnabled(true);
            }
        }
}

} // namespace

bool AppWindow::ribbonControlAllowedReadOnly(const QString &name)
{
    // The controls of the ribbon that are not commands and only change how the document is shown.
    static const QSet<QString> allowed = {QStringLiteral("zoomBox"), QStringLiteral("gridlinesCheck"), QStringLiteral("guidesCheck"), QStringLiteral("rulerCheck")};
    return allowed.contains(name);
}

void AppWindow::setReadOnly(bool on)
{
    m_readOnly = on;
    // The ribbon's controls that are not commands (the font box, the size box, color buttons, galleries, number boxes) and its menus of commands made in code are turned off in a
    // document open for reading only, and on again after; what a command's own button does is decided by the command's id.
    Ribbon *r = ribbon();
    if (!r) return;
    for (int i = 0; i < r->tabCount(); ++i) {
        RibbonTab *tab = r->tabAt(i);
        if (tab && tab->isBuilt()) applyReadOnlyToTab(tab, on);   // (a tab made later is turned off when it is made, if the document is still read-only then)
    }
}

void AppWindow::keepRibbonReadOnly()
{
    Ribbon *r = ribbon();
    if (!m_readOnly || !r) return;
    for (int i = 0; i < r->tabCount(); ++i)
        if (RibbonTab *tab = r->tabAt(i); tab && tab->isBuilt()) applyReadOnlyToTab(tab, true);
}

void AppWindow::ribbonTabMade(RibbonTab *tab)
{
    if (m_readOnly && tab) applyReadOnlyToTab(tab, true);
}

void AppWindow::refuseReadOnlyChange()
{
    statusBar()->showMessage(QCoreApplication::translate("AppWindow", "This file is open for reading only. Use Save a Copy, or Save As, to make a copy you can change."), 6000);
}

} // namespace jp
