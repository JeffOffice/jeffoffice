#include "app/appwindow.h"

#include <QApplication>
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
    static const QStringList prefixes = {QStringLiteral("file."), QStringLiteral("view."), QStringLiteral("zoom."), QStringLiteral("help."), QStringLiteral("show."), QStringLiteral("win."),
                                         QStringLiteral("window."), QStringLiteral("find."), QStringLiteral("nav.")};
    static const QStringList exact = {QStringLiteral("edit.copy"), QStringLiteral("edit.selectAll"), QStringLiteral("edit.find"), QStringLiteral("edit.goTo"), QStringLiteral("edit.selectObjects"),
                                      QStringLiteral("review.nextChange"), QStringLiteral("review.previousChange"), QStringLiteral("review.nextComment"), QStringLiteral("review.previousComment"),
                                      QStringLiteral("review.reviewingPane"), QStringLiteral("review.wordCount")};
    for (const QString &p : prefixes)
        if (id.startsWith(p)) return true;
    return exact.contains(id);
}

void AppWindow::refuseReadOnlyChange()
{
    statusBar()->showMessage(QCoreApplication::translate("AppWindow", "This file is open for reading only. Use Save a Copy, or Save As, to make a copy you can change."), 6000);
}

} // namespace jp
