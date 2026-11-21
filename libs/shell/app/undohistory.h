#pragma once
// The drop-down list of steps on the Undo and Redo buttons: the steps newest first, a click on one does it and every step between, and the hover marks how many
// that is. The list reads the app's steps through a function (newest first, as names), and does them through another, so an app with its own undo rules keeps them.

#include "core/export.h"
#include <QMenu>
#include <QStringList>
#include <functional>

class QKeyEvent;
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
    // The rows as the list holds them: the text shown (elided to the width of the list), and the whole name, which is the tip and what a screen reader reads.
    QString rowText(int row) const;
    QString rowName(int row) const;
    QString footerText() const;
    // The most width the list takes, however long a step's name is.
    static constexpr int kMaxWidth = 360;

protected:
    // The keys of a menu: Up and Down move the marked steps (as the pointer does), Home and End mark the first and the last, PageUp and PageDown move by a page, Enter does the marked
    // steps, and Escape closes the list.
    void keyPressEvent(QKeyEvent *e) override;

private:
    void fill();
    void announce();
    int page() const;
    bool m_redo;
    std::function<QStringList()> m_names;
    std::function<void(int)> m_run;
    QListWidget *m_list = nullptr;
    QLabel *m_footer = nullptr;
    int m_marked = 0;
};

} // namespace jp
