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
};

} // namespace jp
