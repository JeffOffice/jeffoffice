#pragma once
// What every JeffOffice app's main window offers the shared parts (Help,
// the updater): its commands by id, its ribbon, and the list of open windows.

#include <QMainWindow>
#include <QStringList>

class QAction;

namespace jp {

class Ribbon;
class RibbonTab;

class AppWindow : public QMainWindow {
    Q_OBJECT
public:
    using QMainWindow::QMainWindow;
    virtual QAction *act(const QString &id) const = 0;
    virtual QStringList actionIds() const = 0;   // sorted
    virtual Ribbon *ribbon() const = 0;
    // Every app window that is open, in the order the system lists them.
    static QList<AppWindow *> windows();
    // The View > Window group's placing, for the windows an app lists (its own kind): side by side in columns across the screen, stacked with each a little down and right from the one
    // before, and two windows each filling half of the screen. Each window that is placed is shown normal (not maximized or minimized).
    static void arrangeInColumns(const QList<AppWindow *> &wins);
    static void cascade(const QList<AppWindow *> &wins);
    static void placeSideBySide(AppWindow *left, AppWindow *right);

    // A file opened read-only: nothing in it can be changed here, and Save asks for a new name (which makes the window an ordinary one). The app's window overrides
    // setReadOnly to show it (the title, what it will not let be typed or dragged), and calls the base.
    bool isReadOnly() const { return m_readOnly; }
    virtual void setReadOnly(bool on);
    // Whether a command (by its id) still works in a read-only document: the ones that only look, find, copy, or leave. Every other is refused.
    static bool commandAllowedReadOnly(const QString &id);
    // Whether a control of the ribbon that is not a command (named as the ribbon file names its widgets) may be used in a read-only document.
    static bool ribbonControlAllowedReadOnly(const QString &name);
    // Says, in the status bar, why a change was refused.
    void refuseReadOnlyChange();
    // True, after saying why, when a change is asked for in a read-only document: for a control that changes the document without being a command (a box, a gallery, a menu made in code).
    bool refuseIfReadOnly()
    {
        if (!m_readOnly) return false;
        refuseReadOnlyChange();
        return true;
    }

protected:
    // A ribbon tab made when first shown is turned off, as the others are, if the document is read-only (the windows call this from the ribbon's tabBuilt).
    void ribbonTabMade(RibbonTab *tab);
    bool m_readOnly = false;
};

} // namespace jp
