#pragma once
// The Quick Toolbar page of Options (Customize): the commands of the small toolbar at the top left of the window, in a list, with any command of the app to add (the same list, and search, as
// the Ribbon and Keyboard pages have), a separator to add, and the commands to put in a new order, take away, and put back as the app has them. Only the quick toolbar is kept from this page:
// what the Ribbon page keeps is left as it is.

#include "core/export.h"
#include "app/ribboncustom.h"

#include <QWidget>

class QCheckBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;

namespace jp {

class AppWindow;

class JO_COMMON_EXPORT QuickToolbarPage : public QWidget {
    Q_OBJECT
public:
    explicit QuickToolbarPage(AppWindow *win, QWidget *parent = nullptr);
    // After OK: the changes are kept and every window of the app takes them.
    void apply();
    // (only its quick toolbar is the page's)
    RibbonCustom &plan() { return m_plan; }

private:
    struct Command {
        QString id, text, group;
    };
    void fillToolbar(int select);
    void fillCommands();
    void filter(const QString &text);
    void showState();
    void say(const QString &text);
    QString commandText(const QString &id) const;
    // The command Reset One acts on: the one chosen in the list that was chosen from last.
    QString resetTarget() const;

    RibbonCustom m_plan;
    QStringList m_defaults;
    QList<Command> m_commands;
    QLineEdit *m_search;
    QListWidget *m_toolbar, *m_list;
    QPushButton *m_add, *m_separator, *m_up, *m_down, *m_remove, *m_resetOne, *m_resetAll;
    QLabel *m_message;
    QCheckBox *m_below;
    bool m_belowTouched = false;
    bool m_fromCommands = false;
    bool m_filling = false;
    bool m_locked = false;
};

} // namespace jp
