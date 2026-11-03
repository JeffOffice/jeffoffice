#pragma once
// What every JeffOffice app's main window offers the shared parts (Help,
// the updater): its commands by id, its ribbon, and the list of open windows.

#include <QMainWindow>
#include <QStringList>

class QAction;

namespace jp {

class Ribbon;

class AppWindow : public QMainWindow {
    Q_OBJECT
public:
    using QMainWindow::QMainWindow;
    virtual QAction *act(const QString &id) const = 0;
    virtual QStringList actionIds() const = 0;   // sorted
    virtual Ribbon *ribbon() const = 0;
    // Every app window that is open, in the order the system lists them.
    static QList<AppWindow *> windows();

    // A file opened read-only: nothing in it can be changed here, and Save asks for a new name (which makes the window an ordinary one). The app's window overrides
    // setReadOnly to show it (the title, what it will not let be typed or dragged), and calls the base.
    bool isReadOnly() const { return m_readOnly; }
    virtual void setReadOnly(bool on) { m_readOnly = on; }
    // Whether a command (by its id) still works in a read-only document: the ones that only look, find, copy, or leave. Every other is refused.
    static bool commandAllowedReadOnly(const QString &id);
    // Says, in the status bar, why a change was refused.
    void refuseReadOnlyChange();

protected:
    bool m_readOnly = false;
};

} // namespace jp
