#pragma once
// Customize the keyboard: the keys of every command of an app's window, changed by the person and kept in the app's settings ("keys/custom", only what differs from the
// commands' own keys). A KeyMap is a working copy, as the Options page edits it: nothing changes in the windows until save() and applyAll().

#include <QHash>
#include <QKeySequence>
#include <QList>
#include <QString>
#include <QStringList>

class QAction;

namespace jp {

class AppWindow;

// One command as the keyboard page lists it.
struct KeyCommand {
    QString id;
    QString text;   // the command's name, as the ribbon shows it
    QString group;  // the ribbon tab it is on ("Other" for one that is on none)
    QList<QKeySequence> keys;
    QList<QKeySequence> defaults;
    bool keyOnly = false;   // a command an app makes only for its key (a style, a symbol): listed here, but not offered for the ribbon or the quick toolbar
    bool changed() const { return keys != defaults; }
};

class KeyMap {
public:
    // The commands of `win` (every one with a name), with the saved custom keys over their own. `win` must outlive the map. Where the commands are on the ribbon is read only
    // when `withRibbon` (it makes every tab of the ribbon).
    explicit KeyMap(AppWindow *win, bool withRibbon = true);
    // Whether the page lists a command: it has a name and is not a menu (a command the page does not list still has its keys in use). The one test the map and its callers share.
    static bool listsCommand(const QAction *a);

    // A command that is only for a key sets this property on its action to the name of its group on the page (and is listed last, after the ribbon's tabs).
    static constexpr const char *kGroupProperty = "jpKeyGroup";
    // The most commands of one kind (ids with one prefix) read from the settings, and the ids the settings hold that begin with `prefix` (sorted, up to the most).
    static constexpr int kMaxExtraIds = 200;
    static QStringList savedIds(const QString &prefix);
    // A command made after the map was (the page adds one): taken from the window's command of that id; false when the window has none, or the map has it already.
    bool addCommand(const QString &id);

    // In ribbon order, by tab; the ones on no tab last.
    const QList<KeyCommand> &commands() const { return m_commands; }
    const KeyCommand *command(const QString &id) const;
    // The command that has `key` (another than `exceptId`), or an empty id.
    QString owner(const QKeySequence &key, const QString &exceptId = QString()) const;

    enum class Add { Added, Already, Reserved, Taken, Invalid };
    struct Result {
        Add status = Add::Invalid;
        QString owner;       // for Taken: the id of the command that has the key
        QString ownerText;   // and its name
    };
    // What add() would do, changing nothing.
    Result check(const QString &id, const QKeySequence &key) const;
    // Gives `id` the key. A key another command has is refused (Taken) unless `takeOver`, which removes it from that command. A reserved key is always refused.
    Result add(const QString &id, const QKeySequence &key, bool takeOver = false);
    // False for a key that is reserved (the command keeps it).
    bool remove(const QString &id, const QKeySequence &key);
    void resetOne(const QString &id);
    void resetAll();
    bool anyChanged() const;

    // Writes the differences to the app's settings.
    void save() const;
    // Gives every open window of the same app (the same kind as `win`) the keys of this map.
    void applyAll() const;

    // The keys a person may not give to a command: the ones the system and text editing need (Copy, Cut, Paste, Undo, Select All, Tab, Enter, Escape, the arrow and
    // editing keys, and a key that only types a character).
    static bool isReserved(const QKeySequence &key);
    static QStringList reservedList();   // for showing, in the native text
    // A key typed or read from a file: one key, in the form `QKeySequence::PortableText` writes; empty when it is malformed.
    static QKeySequence parseKey(const QString &text);

    // Sets the keys of a window from the settings (a window calls this once its commands are made), the first time also noting each command's own keys. Unknown ids,
    // malformed keys, reserved keys, and more than the cap of entries in the settings are ignored.
    static void apply(AppWindow *win);

    // A command made after the window was up (a part of the window built when first used): it takes its saved keys. Call it with each new command.
    static void applyLater(AppWindow *win, QAction *a);

    // The key a command shows now, as text (its first key; empty when it has none): for a tooltip a window writes at each change ("Undo typing (Ctrl+Z)").
    static QString shownKey(const QAction *a);
    // `tip` written for the command's own key `oldKey`, with the key it has now: the "(oldKey)" at the end of the tip is made that key's, or taken off when it has none.
    static QString tipWithShownKey(const QString &tip, const QString &oldKey, const QAction *a);

    static constexpr int kMaxEntries = 2000;
    static constexpr int kMaxKeysPerCommand = 4;

private:
    AppWindow *m_win;
    QList<KeyCommand> m_commands;
    QHash<QString, int> m_index;
    QList<QKeySequence> m_foreign;   // keys in use that the page does not list: the window's own shortcuts and commands with no name
};

} // namespace jp
