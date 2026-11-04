#include "app/appwindow.h"

#include "app/ribbon.h"

#include <QApplication>
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

bool AppWindow::commandAllowedReadOnly(const QString &id)
{
    // The commands that only look, find, copy, open, save elsewhere, or leave: each named, so a command that is added later is refused until it is put here. (Save and Save As are here
    // because each of them handles a document open for reading only itself: a new name, never the file's own.) The commands a person can reach in a read-only document and that
    // change it are not here: Replace, the slide show's set-up, its timings, its custom shows, the file's properties, freezing panes.
    static const QSet<QString> allowed = [] {
        QSet<QString> ids;
        for (const char *id : {
                 "file.blank", "file.close", "file.exportHtml", "file.exportImages", "file.exportPdf", "file.exportPicture", "file.new", "file.open", "file.open.jpub", "file.open.pub",
                 "file.openDialog", "file.openText", "file.options", "file.print", "file.printNow", "file.save", "file.save.jpub", "file.save.pub", "file.saveAs", "file.saveAsPub",
                 "file.saveCopy", "file.saveTemplate",
                 "find.find", "find.goto", "find.gotoSpecial", "find.next",
                 "help.feedback", "help.shortcuts", "help.show", "help.whatsNew", "view.pageMovement.sideToSide", "view.pageMovement.vertical",
                 "show.fromCurrent", "show.fromStart", "show.presenterView",
                 "view.baselines", "view.boundaries", "view.collapseRibbon", "view.fields", "view.fit", "view.formulaBar", "view.formulas", "view.graphics", "view.gridlines", "view.guides",
                 "view.headings", "view.marks", "view.master", "view.measurement", "view.normal", "view.notes", "view.outline", "view.pageBreakView", "view.pageBreaks", "view.pageLayout",
                 "view.navigationPane", "view.focus", "view.pageNav", "view.printLayout", "view.ruler", "view.rulers", "view.scratch", "view.single", "view.slideShow", "view.sorter", "view.spread",
                 "win.arrange", "win.cascade", "win.new",
                 "zoom.10", "zoom.100", "zoom.150", "zoom.200", "zoom.25", "zoom.300", "zoom.33", "zoom.400", "zoom.50", "zoom.66", "zoom.75", "zoom.dialog", "zoom.in", "zoom.multiplePages",
                 "zoom.onePage", "zoom.out", "zoom.page", "zoom.pageWidth", "zoom.selection", "zoom.width",
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
        if (!tab) continue;
        for (RibbonGroup *g : tab->groups())
            for (QWidget *c : g->controls()) {
                auto *button = qobject_cast<QToolButton *>(c);
                bool safe = true;
                if (button && button->menu()) safe = menuAllowedReadOnly(button->menu());
                else if (button && button->defaultAction()) safe = true;   // (a button for a command: the command is refused, or off, by its id)
                else safe = ribbonControlAllowedReadOnly(c->property("ribbonWidget").toString());
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
}

void AppWindow::refuseReadOnlyChange()
{
    statusBar()->showMessage(QCoreApplication::translate("AppWindow", "This file is open for reading only. Use Save a Copy, or Save As, to make a copy you can change."), 6000);
}

} // namespace jp
