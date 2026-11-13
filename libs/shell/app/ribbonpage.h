#pragma once
// The Ribbon page of Options (Customize): the app's tabs and groups in a list, to show or hide, put in a new order, and add to with tabs and groups of one's own that hold any command.

#include "core/export.h"
#include "app/ribboncustom.h"

#include <QWidget>

class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;

namespace jp {

class AppWindow;

class JO_COMMON_EXPORT RibbonPage : public QWidget {
    Q_OBJECT
public:
    explicit RibbonPage(AppWindow *win, QWidget *parent = nullptr);
    // After OK: the changes are kept and every window of the app takes them.
    void apply();
    RibbonCustom &plan() { return m_plan; }

private:
    struct BuiltGroup {
        QString id, title;
    };
    struct BuiltTab {
        QString name, title;
        bool contextual = false;
        QList<BuiltGroup> groups;
    };
    struct Command {
        QString id, text, group;
    };
    void fillTree();
    void fillCommands();
    void filter(const QString &text);
    void changed();
    void showState();
    void say(const QString &text);
    // What the chosen row is: its kind ("tab", "group", or "cmd"), its id (a built-in group's is "Tab/Group"), the tab it is on, and for a command its place in the group.
    struct Chosen {
        QString kind, id, tab;
        int index = -1;
        bool own = false;
    };
    Chosen chosen() const;
    void choose(const Chosen &c);
    QStringList builtinOrder() const;
    const BuiltTab *builtTab(const QString &name) const;
    QString nameText(const QString &fallback) const;

    AppWindow *m_win;
    RibbonCustom m_plan;
    QList<BuiltTab> m_built;
    QList<Command> m_commands;
    QLineEdit *m_search, *m_name;
    QTreeWidget *m_tree;
    QListWidget *m_list;
    QPushButton *m_newTab, *m_newGroup, *m_addCommand, *m_rename, *m_up, *m_down, *m_remove, *m_resetOne, *m_resetAll;
    QLabel *m_message;
    bool m_filling = false;
};

} // namespace jp
