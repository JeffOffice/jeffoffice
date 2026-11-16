#pragma once
// For the tests of the chart editor's menus (the elements menu every program has from libs/page/app/chartdesign.h): opens a menu as a click would (its aboutToShow runs, which makes the
// entries for the chart that is selected) and finds an entry by its object name, which is its menu's name, a dot, and the entry's English words ("chartTitleMenu.Above"), never by the text it
// shows, which is in the language of the program.
#include <QAction>
#include <QMenu>
#include <QMetaObject>
#include <QString>

inline QAction *findChartMenuEntry(QMenu *menu, const QString &objectName)
{
    QMetaObject::invokeMethod(menu, "aboutToShow", Qt::DirectConnection);
    return menu->findChild<QAction *>(objectName);
}

inline bool fireChartMenu(QMenu *menu, const QString &objectName)
{
    QAction *a = findChartMenuEntry(menu, objectName);
    if (!a) return false;
    a->trigger();
    return true;
}
