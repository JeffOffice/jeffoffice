#include "app/appwindow.h"

#include <QApplication>

namespace jp {

QList<AppWindow *> AppWindow::windows()
{
    QList<AppWindow *> out;
    for (QWidget *w : QApplication::topLevelWidgets())
        if (auto *a = qobject_cast<AppWindow *>(w)) out << a;
    return out;
}

} // namespace jp
