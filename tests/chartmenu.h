#pragma once
// For the tests of the chart editor's menus (the elements, layouts, and looks menus every program has from libs/page/app/chartdesign.h): opens a menu as a click would (its
// aboutToShow runs, which makes the entries for the chart that is selected) and fires the entry at the end of a path of menu texts. False when the path is not there.
#include <QAction>
#include <QMenu>
#include <QMetaObject>
#include <QStringList>

inline QAction *findChartMenuEntry(QMenu *menu, const QStringList &path)
{
    QMetaObject::invokeMethod(menu, "aboutToShow", Qt::DirectConnection);
    QMenu *m = menu;
    QAction *found = nullptr;
    for (int i = 0; i < path.size(); ++i) {
        found = nullptr;
        for (QAction *a : m->actions())
            if (a->text() == path[i]) found = a;
        if (!found) return nullptr;
        if (i + 1 < path.size()) m = found->menu();
        if (!m) return nullptr;
    }
    return found;
}

inline bool fireChartMenu(QMenu *menu, const QStringList &path)
{
    QAction *a = findChartMenuEntry(menu, path);
    if (!a) return false;
    a->trigger();
    return true;
}
