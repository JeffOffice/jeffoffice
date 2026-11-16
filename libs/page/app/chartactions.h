#pragma once
// The commands of Insert > Chart and the Chart tab for a window that edits charts as objects of its pages (JeffPub, JeffDeck): the actions, the menus of the ribbon, and what the actions
// show for the chart that is selected. The window gives the way it makes an action (so the action is in its list, with its keys and its Help) and finds one; the changes are core/
// chartobject.h's, made through the editor as one step each.

#include "app/chartdesign.h"
#include "app/chartedit.h"

#include <QAction>
#include <QMenu>
#include <QObject>
#include <QString>
#include <QTimer>
#include <functional>
#include <memory>

namespace jp {

class Canvas;

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
    // The Chart Design tab: the editor every program shares (app/chartdesign.h). Its commands are chartdesign.switch, .selectData, .editData, .changeType, .axisH, and .axisV; its menus are the ribbon's
    // @chartElements, @chartLayouts, @chartColors, and @chartLooks.
    ChartDesign *design() const { return m_design; }
    QMenu *elementsMenu(QWidget *owner) const;
    QMenu *layoutsMenu(QWidget *owner) const;
    QMenu *colorsMenu(QWidget *owner) const;
    QMenu *looksMenu(QWidget *owner) const;
    // The three buttons beside a selected chart, on `canvas`; they follow the chart as it moves, sizes, and scrolls.
    void attachButtons(Canvas *canvas, const std::function<QIcon(const QString &)> &icon);
    ChartButtons *buttons() const { return m_buttons; }

private:
    void editData();
    void editTitle();
    void editAxisTitle(bool values);
    void change(const QString &label, const std::function<bool(office::Chart &)> &edit);

    void placeButtons();

    Editor *m_ed;
    QWidget *m_window;
    Hooks m_hooks;
    QStringList m_ids;   // every action made, for refresh
    std::unique_ptr<ChartDesignHost> m_host;
    ChartDesign *m_design = nullptr;
    ChartButtons *m_buttons = nullptr;
    Canvas *m_canvas = nullptr;
    std::function<QIcon(const QString &)> m_icon;
    QTimer m_buttonsTimer;
};

} // namespace jp
