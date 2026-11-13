#pragma once
// The Keyboard page of Options (Customize): every command of the app's window, grouped by ribbon tab and searchable, with its keys; keys are added by pressing them, and taken off.

#include "core/export.h"
#include "app/keymap.h"

#include <QHash>
#include <QWidget>
#include <memory>

class QKeySequenceEdit;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;

namespace jp {

class AppWindow;

class JO_COMMON_EXPORT KeyboardPage : public QWidget {
    Q_OBJECT
public:
    explicit KeyboardPage(AppWindow *win, QWidget *parent = nullptr);
    // After OK: the keys are kept and every window of the app takes them.
    void apply();
    KeyMap &map() { return *m_map; }

private:
    void fillTree();
    QTreeWidgetItem *addItem(const KeyCommand &c);
    void addExtra(AppWindow *win, const QString &id);
    void filter(const QString &text);
    QString currentId() const;
    void showCurrent();
    void showRow(const QString &id);
    void pressed();
    void assign(bool takeOver);
    void say(const QString &text);

    std::unique_ptr<KeyMap> m_map;
    QHash<QString, QTreeWidgetItem *> m_groups;
    QLineEdit *m_search;
    QTreeWidget *m_tree;
    QListWidget *m_keys;
    QKeySequenceEdit *m_press;
    QPushButton *m_assign, *m_takeOver, *m_remove, *m_resetOne, *m_resetAll;
    QLabel *m_message;
};

} // namespace jp
