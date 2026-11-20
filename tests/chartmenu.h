#pragma once
// For the tests of the chart editor's menus (the elements menu every program has from libs/page/app/chartdesign.h): opens a menu as a click would (its aboutToShow runs, which makes the
// entries for the chart that is selected) and finds an entry by its object name, which is its menu's name, a dot, and the entry's English words ("chartTitleMenu.Above"), never by the text it
// shows, which is in the language of the program.
#include <QAction>
#include <QComboBox>
#include <QToolButton>
#include <QWidget>
#include <QMenu>
#include <QMetaObject>
#include <QString>

inline QAction *findChartMenuEntry(QMenu *menu, const QString &objectName)
{
    QMetaObject::invokeMethod(menu, "aboutToShow", Qt::DirectConnection);
    // (a submenu of the last time is not deleted with its entries: the last found is the new one)
    const QList<QAction *> found = menu->findChildren<QAction *>(objectName);
    return found.isEmpty() ? nullptr : found.last();
}

inline bool fireChartMenu(QMenu *menu, const QString &objectName)
{
    QAction *a = findChartMenuEntry(menu, objectName);
    if (!a) return false;
    a->trigger();
    return true;
}

// The Current Selection box of a ribbon tab (found by its object name), and a pick in it as a click makes it.
inline QComboBox *chartPartBoxOn(QWidget *tab) { return tab ? tab->findChild<QComboBox *>(QStringLiteral("chartPartBox")) : nullptr; }

inline void pickChartPart(QComboBox *box, int index)
{
    box->setCurrentIndex(index);
    Q_EMIT box->activated(index);
}

// Whether a button of the tab runs the action.
inline bool tabHasButtonFor(QWidget *tab, const QAction *action)
{
    for (QToolButton *b : tab->findChildren<QToolButton *>())
        if (b->defaultAction() == action) return true;
    return false;
}
