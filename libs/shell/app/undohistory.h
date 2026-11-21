#pragma once
// The drop-down list of steps on the Undo and Redo buttons: the steps newest first, a click on one does it and every step between, and the hover marks how many
// that is. The list reads the app's steps through a function (newest first, as names), and does them through another, so an app with its own undo rules keeps them.

#include "core/export.h"
#include <QMenu>
#include <QStringList>
#include <functional>

class QLabel;
class QListWidget;

namespace jp {

class JO_COMMON_EXPORT UndoHistoryMenu : public QMenu {
    Q_OBJECT
public:
    // The most steps a list shows (the rest are still there, and are reached by the button).
    static constexpr int kMaxShown = 100;

    UndoHistoryMenu(bool redo, std::function<QStringList()> names, std::function<void(int count)> run, QWidget *parent = nullptr);

    // Marks the first `count` steps (as a hover does) and says what a click would do.
    void markSteps(int count);
    int marked() const { return m_marked; }
    // Does the first `count` steps through the app's function and closes the list.
    void choose(int count);
    int shownSteps() const;

private:
    void fill();
    bool m_redo;
    std::function<QStringList()> m_names;
    std::function<void(int)> m_run;
    QListWidget *m_list = nullptr;
    QLabel *m_footer = nullptr;
    int m_marked = 0;
};

} // namespace jp
