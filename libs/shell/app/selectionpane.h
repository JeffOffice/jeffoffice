#pragma once
// The Selection Pane (selectionpane.cpp): a list of the objects of a page, a slide, or a sheet by name, the one in front first. A row picks the object; its box shows or hides it; its
// name is edited in place; the buttons (and a drag) move it forward or back; Show All and Hide All set every box. What the list is of, and what each action does to it, is the app's:
// it gives the pane a Source of functions, and the pane keeps no copy of the objects (a Source left unset takes that part away, so a sheet that cannot hide objects shows no boxes).

#include "core/export.h"
#include <QDialog>
#include <QString>
#include <QVector>
#include <functional>

class QListWidget;
class QPushButton;

namespace jp {

class JO_COMMON_EXPORT SelectionPane : public QDialog {
    Q_OBJECT
public:
    struct Row {
        QString name;
        bool shown = true;
        bool picked = false;
    };
    struct Source {
        std::function<QVector<Row>()> rows;                         // front first
        std::function<void(int row)> pick;
        std::function<void(int row, const QString &name)> rename;   // unset: the names are fixed
        std::function<void(int row, bool shown)> show;              // unset: no boxes
        std::function<void(bool shown)> showAll;                    // unset: no Show All and Hide All
        std::function<void(int from, int to)> move;                 // a row to another place in the list
        // The words of a pane that is not the Selection Pane itself (the Reading Order list is the same list read from the back); empty: the Selection Pane's own.
        QString title, listName, forwardText, backwardText;
        QString note;   // a line of explanation above the list (empty: none)
    };
    SelectionPane(QWidget *parent, const Source &source);
    // The list again from the Source (the app calls it when its objects or its pick change).
    void refresh();
    // The list again, but not at once and not while the pane is hidden: one refresh, at the end of the event-loop turn, however many changes asked for it (a page of thousands of objects
    // changes in thousands of ways in a turn); a hidden pane is made current when it is shown.
    void requestRefresh();

protected:
    void showEvent(QShowEvent *e) override;

private:
    void moveCurrent(int by);
    Source m_source;
    QListWidget *m_list = nullptr;
    QPushButton *m_showAll = nullptr, *m_hideAll = nullptr;
    // Set while the list is giving a signal out (a pick, a box, a name, a drag): a refresh asked for then waits until the signal is done, so that no row is taken away under it.
    bool m_busy = false, m_again = false, m_queued = false, m_stale = false;
    bool m_filling = false;
};

} // namespace jp
