#pragma once
// The commands of Insert > Chart and the Chart tab for a window that edits charts as objects of its pages (JeffPub, JeffDeck): the actions, the menus of the ribbon, and what the actions
// show for the chart that is selected. The window gives the way it makes an action (so the action is in its list, with its keys and its Help) and finds one; the changes are core/
// chartobject.h's, made through the editor as one step each.

#include "app/chartedit.h"

#include <QAction>
#include <QMenu>
#include <QObject>
#include <QString>
#include <functional>

namespace jp {

class ChartActions : public QObject {
public:
    struct Hooks {
        // Makes the action `id` (text, icon name, what it does, checkable) and returns it.
        std::function<QAction *(const QString &id, const QString &text, const QString &icon, const std::function<void()> &fn, bool checkable)> make;
        std::function<QAction *(const QString &id)> find;
        // Says something to the person (a data cell that is not a number); optional.
        std::function<void(const QString &)> notice;
        // Called after a command ran, to hand the keys back to the page.
        std::function<void()> done;
    };
    ChartActions(Editor *editor, QWidget *window, const Hooks &hooks, QObject *parent = nullptr);

    // Makes every action.
    void create();
    // Sets what every action shows (enabled, checked) for the selection; true when a chart is selected (the Chart tab shows).
    bool refresh();
    // The ribbon's menus, as the ribbon file names them (@insertChart, @chartType, @chartAxes, @chartGridlines, @chartStyles, @chartLegend).
    QMenu *insertMenu(QWidget *owner) const;
    QMenu *typeMenu(QWidget *owner) const;
    QMenu *axesMenu(QWidget *owner) const;
    QMenu *gridlinesMenu(QWidget *owner) const;
    QMenu *stylesMenu(QWidget *owner) const;
    QMenu *legendMenu(QWidget *owner) const;

private:
    void editData();
    void editTitle();
    void editAxisTitle(bool values);
    void change(const QString &label, const std::function<bool(office::Chart &)> &edit);

    Editor *m_ed;
    QWidget *m_window;
    Hooks m_hooks;
    QStringList m_ids;   // every action made, for refresh
};

} // namespace jp
