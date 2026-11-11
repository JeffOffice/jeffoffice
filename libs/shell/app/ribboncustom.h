#pragma once
// Customize the ribbon: which tabs and groups of an app's ribbon show, the tabs and groups the person made (with any command in them), and the order of the tabs. It is kept in the app's
// settings ("ribbon/custom", only what differs from the ribbon file) and applied to a window once its ribbon is built, and again to every window when Options is closed with OK. A
// RibbonCustom is a working copy, as the Options page edits it: nothing changes in the windows until save() and applyAll().
//
// A built-in tab is named as the ribbon file names it, a built-in group as "Tab/Group". What the person made has an id "user:N". Anything the settings name that the ribbon does not have is
// ignored, and so is whatever is beyond the caps.

#include <QList>
#include <QString>
#include <QStringList>

class QAction;

namespace jp {

class AppWindow;
class Ribbon;

class RibbonCustom {
public:
    struct Item {
        QString cmd;    // a command's id
        QString name;   // what the person calls it here; empty for its own name
    };
    struct Group {
        QString id;     // "user:N"
        QString tab;    // the name of a built-in tab, or the id of one of the person's
        QString name;
        QList<Item> items;
    };
    struct Tab {
        QString id;     // "user:N"
        QString name;
    };

    QStringList hiddenTabs;     // built-in tabs turned off
    QStringList hiddenGroups;   // built-in groups turned off, "Tab/Group"
    QStringList tabOrder;       // the tabs that are always there, in the order the person wants (empty: as the ribbon file has them)
    QList<Tab> tabs;            // the person's own tabs
    QList<Group> groups;        // the person's own groups, on any tab
    bool quickSet = false;      // the person changed the quick toolbar: `quick` is it (otherwise the ribbon file's is)
    QStringList quick;          // a command's id, or an empty string for a separator

    bool isEmpty() const;
    bool operator==(const RibbonCustom &o) const;

    // The saved differences, read with every check a file that is not to be trusted needs (the caps below; unknown or malformed entries are left out). Nothing saved is an empty one.
    static RibbonCustom load();
    // Writes the differences to the app's settings (nothing, when there are none).
    void save() const;
    QByteArray toJson() const;
    static RibbonCustom fromJson(const QByteArray &json);

    // ---- editing (what the Options page does). Each says whether it changed anything.
    // Turn a built-in tab or group off or on (Home cannot be turned off).
    bool setTabHidden(const QString &tab, bool hidden);
    bool setGroupHidden(const QString &tab, const QString &group, bool hidden);
    // The id of the new tab (empty at the cap of kMaxTabs).
    QString addTab(const QString &name);
    // The id of the new group on a tab (empty at the cap of kMaxGroups on that tab, or for a tab that is not there).
    QString addGroup(const QString &tab, const QString &name);
    // False at the cap of kMaxItems, or for a command already in the group.
    bool addCommand(const QString &groupId, const QString &cmd);
    // The name of one of the person's tabs or groups; an empty name is refused.
    bool rename(const QString &id, const QString &name);
    // A command's name in a group; empty gives it its own back.
    bool renameCommand(const QString &groupId, int index, const QString &name);
    // Moves one of the person's tabs among the tabs that are always there (`builtin`: their names, as the ribbon file has them), a tab or a group among its neighbors, or a command in its group.
    bool moveTab(const QString &id, int delta, const QStringList &builtin);
    // (any tab, the person's or built in)
    bool moveGroup(const QString &id, int delta);
    bool moveCommand(const QString &groupId, int index, int delta);
    bool remove(const QString &id);   // one of the person's tabs (with its groups) or groups
    bool removeCommand(const QString &groupId, int index);
    // The order the tabs that are always there have: `tabOrder` over `builtin` (their names), then the person's tabs.
    QStringList effectiveOrder(const QStringList &builtin) const;
    // Forgets an order that is the ribbon file's (what is listed that this window does not have stays).
    void normalizeOrder(const QStringList &builtin);
    // A tab back as the ribbon file has it: shown, with its groups shown and the person's groups on it gone (one of the person's own is taken away).
    void resetTab(const QString &tab);
    void resetAll();
    // Takes away the groups on a tab that is neither one of `known` (the ribbon's own tabs) nor one of the person's: what is left of a tab another version had.
    void dropUnknownTabs(const QStringList &known);

    const Tab *tab(const QString &id) const;
    const Group *group(const QString &id) const;
    QList<const Group *> groupsOf(const QString &tab) const;

    // ---- the quick toolbar (`defaults`: the commands of the ribbon file's own, which is what it is until the person changes it). Each says whether it changed anything.
    QStringList effectiveQuick(const QStringList &defaults) const;
    // False at the cap of kMaxQuick, or for a command already there.
    bool addQuickCommand(const QString &cmd, const QStringList &defaults);
    bool addQuickSeparator(const QStringList &defaults);
    bool removeQuick(int index, const QStringList &defaults);
    bool moveQuick(int index, int delta, const QStringList &defaults);
    // A command back where the ribbon file has it (taken away, when the file does not have it).
    bool resetQuickCommand(const QString &cmd, const QStringList &defaults);
    // Whether a command is not where the ribbon file has it.
    bool quickCommandChanged(const QString &cmd, const QStringList &defaults) const;
    void resetQuick();

    // ---- applying
    // The saved customizing, on a window's ribbon (a window calls this once its ribbon is built; nothing is done when there is none).
    static void apply(AppWindow *win);
    // The saved customizing on the ribbon of every open window.
    static void applyAll();
    // What a window's ribbon has been given (empty for a window that was given nothing): the plan as applied, with the groups on tabs the ribbon does not have left out.
    static RibbonCustom appliedTo(const AppWindow *win);
    // This one on a window's ribbon (the tabs not made yet get it when they are made).
    void applyTo(AppWindow *win) const;

    // Letters for a KeyTip that no other control in `used` has or starts with, from the letters of `name`; `length` is one or two. Empty when none is free.
    static QString freeKeytip(const QString &name, const QStringList &used, int length);

    static constexpr int kMaxTabs = 20;
    static constexpr int kMaxGroups = 30;    // on one tab
    static constexpr int kMaxItems = 60;     // in one group
    static constexpr int kMaxAllItems = 600; // in all the groups together
    static constexpr int kMaxAllGroups = 1000;
    static constexpr int kMaxName = 60;
    static constexpr int kMaxQuick = 100;    // commands and separators in the quick toolbar
    static constexpr int kMaxEntries = 2000; // hidden tabs, hidden groups, and the order, each

private:
    void applyQuick(Ribbon *r, AppWindow *win) const;
    QString nextId() const;
    Group *groupRef(const QString &id);
};

} // namespace jp
