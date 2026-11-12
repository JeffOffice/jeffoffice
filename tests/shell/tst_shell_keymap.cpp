// The shared part of "Customize the keyboard": assigning, taking a key over, resetting, keeping the keys for a new window, the reserved keys, a settings file that is not
// to be trusted, and the tooltips. A small window of a few commands stands for an app's window (each app's own test checks that its Keyboard page lists its commands).
#include "app/appwindow.h"
#include "app/keymap.h"
#include "app/plaintext.h"
#include "app/settings.h"
#include "../testrunner.h"

#include <QAction>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QShortcut>
#include <QtTest>

using namespace jp;

namespace {

class KeyWindow : public AppWindow {
public:
    KeyWindow()
    {
        mk(QStringLiteral("test.bold"), QStringLiteral("Bold"), QKeySequence(Qt::CTRL | Qt::Key_B));
        mk(QStringLiteral("test.italic"), QStringLiteral("Italic"), QKeySequence(Qt::CTRL | Qt::Key_I));
        mk(QStringLiteral("test.plain"), QStringLiteral("Plain"), QKeySequence());
        mk(QStringLiteral("test.two"), QStringLiteral("Two Keys"), QKeySequence(Qt::CTRL | Qt::Key_2));
        act(QStringLiteral("test.two"))->setShortcuts({QKeySequence(Qt::CTRL | Qt::Key_2), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_2)});
        mk(QStringLiteral("edit.undo"), QStringLiteral("Undo"), QKeySequence(Qt::CTRL | Qt::Key_Z));
        mk(QStringLiteral("file.save"), QStringLiteral("Save"), QKeySequence(Qt::CTRL | Qt::Key_S));
        // a key with no name to list it by, and a key the window's own shortcut takes
        mk(QStringLiteral("test.unnamed"), QString(), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_9));
        new QShortcut(QKeySequence(Qt::Key_F6), this);
        KeyMap::apply(this);
    }
    // A command made after the window is up, as a lazily built part of an app does.
    void addLater(const QString &id, const QString &text)
    {
        KeyMap::applyLater(this, mk(id, text, QKeySequence()));
    }
    QAction *act(const QString &id) const override { return m_actions.value(id); }
    QStringList actionIds() const override
    {
        QStringList ids = m_actions.keys();
        ids.sort();
        return ids;
    }
    Ribbon *ribbon() const override { return nullptr; }

private:
    // As the apps make a command: the key, and the tooltip that names it.
    QAction *mk(const QString &id, const QString &text, const QKeySequence &key)
    {
        auto *a = new QAction(text, this);
        a->setObjectName(id);
        if (!key.isEmpty()) {
            a->setShortcut(key);
            a->setShortcutContext(Qt::WindowShortcut);
            a->setToolTip(plainToolTip(QStringLiteral("%1 (%2)").arg(text, key.toString(QKeySequence::NativeText))));
            addAction(a);
        }
        m_actions.insert(id, a);
        return a;
    }
    QHash<QString, QAction *> m_actions;
};

// A window of the same app that has only some of the commands.
class SmallWindow : public AppWindow {
public:
    SmallWindow()
    {
        m_bold = new QAction(QStringLiteral("Bold"), this);
        m_bold->setObjectName(QStringLiteral("test.bold"));
        m_bold->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));
        addAction(m_bold);
        KeyMap::apply(this);
    }
    QAction *act(const QString &id) const override { return id == QLatin1String("test.bold") ? m_bold : nullptr; }
    QStringList actionIds() const override { return {QStringLiteral("test.bold")}; }
    Ribbon *ribbon() const override { return nullptr; }

private:
    QAction *m_bold;
};

// A window with commands that are only for a key (a style, a symbol), as an app that offers them makes them: from the settings when it starts, and by id later.
class ExtraWindow : public AppWindow {
public:
    explicit ExtraWindow(const QStringList &extras = {})
    {
        auto *bold = new QAction(QStringLiteral("Bold"), this);
        bold->setObjectName(QStringLiteral("test.bold"));
        bold->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));
        addAction(bold);
        m_actions.insert(bold->objectName(), bold);
        for (const QString &id : extras) makeKeyCommand(id);
        KeyMap::apply(this);
    }
    QAction *makeKeyCommand(const QString &id) override
    {
        if (!id.startsWith(QLatin1String("extra."))) return nullptr;
        if (QAction *have = m_actions.value(id)) return have;
        auto *a = new QAction(id.mid(6), this);
        a->setObjectName(id);
        a->setProperty(KeyMap::kGroupProperty, QStringLiteral("Symbols"));
        m_actions.insert(id, a);
        KeyMap::applyLater(this, a);
        return a;
    }
    QAction *act(const QString &id) const override { return m_actions.value(id); }
    QStringList actionIds() const override
    {
        QStringList ids = m_actions.keys();
        ids.sort();
        return ids;
    }
    Ribbon *ribbon() const override { return nullptr; }

private:
    QHash<QString, QAction *> m_actions;
};

QKeySequence k(const char *portable) { return QKeySequence::fromString(QString::fromLatin1(portable), QKeySequence::PortableText); }
QString native(const QKeySequence &key) { return key.toString(QKeySequence::NativeText); }
QStringList keysOf(const QAction *a)
{
    QStringList out;
    for (const QKeySequence &s : a->shortcuts()) out << s.toString(QKeySequence::PortableText);
    return out;
}
void setSaved(const QByteArray &json) { Settings::get().setValue(QStringLiteral("keys/custom"), QString::fromUtf8(json)); }
void clearSaved() { Settings::get().setValue(QStringLiteral("keys/custom"), QString()); }

} // namespace

class ShellKeymapTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void init() { clearSaved(); }
    void cleanup() { clearSaved(); }

    void assigningAKeyChangesTheCommandAtOnce()
    {
        KeyWindow w;
        KeyMap map(&w);
        const auto r = map.add(QStringLiteral("test.plain"), k("Ctrl+Shift+P"));
        QCOMPARE(int(r.status), int(KeyMap::Add::Added));
        map.save();
        map.applyAll();
        QCOMPARE(keysOf(w.act(QStringLiteral("test.plain"))), QStringList{QStringLiteral("Ctrl+Shift+P")});
        // (the command's own keys stay, and a second key is added beside the first)
        QVERIFY(map.add(QStringLiteral("test.bold"), k("Ctrl+Shift+B")).status == KeyMap::Add::Added);
        map.save();
        map.applyAll();
        QCOMPARE(keysOf(w.act(QStringLiteral("test.bold"))), (QStringList{QStringLiteral("Ctrl+Shift+B"), QStringLiteral("Ctrl+B")}));
        // a key it has already
        QCOMPARE(int(map.add(QStringLiteral("test.bold"), k("Ctrl+B")).status), int(KeyMap::Add::Already));
        // removing one
        QVERIFY(map.remove(QStringLiteral("test.bold"), k("Ctrl+B")));
        map.save();
        map.applyAll();
        QCOMPARE(keysOf(w.act(QStringLiteral("test.bold"))), QStringList{QStringLiteral("Ctrl+Shift+B")});
        // only the differences are kept
        const QJsonObject saved = QJsonDocument::fromJson(Settings::get().value(QStringLiteral("keys/custom")).toString().toUtf8()).object();
        QCOMPARE(saved.keys(), (QStringList{QStringLiteral("test.bold"), QStringLiteral("test.plain")}));
    }

    void aKeyInUseSaysWhoHasItAndCanBeTakenOver()
    {
        KeyWindow w;
        KeyMap map(&w);
        const auto r = map.add(QStringLiteral("test.plain"), k("Ctrl+I"));
        QCOMPARE(int(r.status), int(KeyMap::Add::Taken));
        QCOMPARE(r.owner, QStringLiteral("test.italic"));
        QCOMPARE(r.ownerText, QStringLiteral("Italic"));
        QVERIFY(map.command(QStringLiteral("test.plain"))->keys.isEmpty());   // (nothing changed)
        QCOMPARE(int(map.add(QStringLiteral("test.plain"), k("Ctrl+I"), true).status), int(KeyMap::Add::Added));
        QVERIFY(map.command(QStringLiteral("test.italic"))->keys.isEmpty());
        QCOMPARE(map.owner(k("Ctrl+I")), QStringLiteral("test.plain"));
        map.save();
        map.applyAll();
        QCOMPARE(keysOf(w.act(QStringLiteral("test.plain"))), QStringList{QStringLiteral("Ctrl+I")});
        QVERIFY(w.act(QStringLiteral("test.italic"))->shortcuts().isEmpty());
    }

    void resetOneAndResetAll()
    {
        KeyWindow w;
        KeyMap map(&w);
        map.add(QStringLiteral("test.plain"), k("Ctrl+Shift+P"));
        map.add(QStringLiteral("test.bold"), k("Ctrl+I"), true);
        QVERIFY(map.anyChanged());
        map.resetOne(QStringLiteral("test.bold"));
        QCOMPARE(map.command(QStringLiteral("test.bold"))->keys, QList<QKeySequence>{k("Ctrl+B")});
        // (the key it had taken from Italic goes back to Italic)
        QCOMPARE(map.command(QStringLiteral("test.italic"))->keys, QList<QKeySequence>{k("Ctrl+I")});
        QCOMPARE(map.command(QStringLiteral("test.plain"))->keys, QList<QKeySequence>{k("Ctrl+Shift+P")});
        map.resetAll();
        QVERIFY(!map.anyChanged());
        QVERIFY(map.command(QStringLiteral("test.plain"))->keys.isEmpty());
        QCOMPARE(map.command(QStringLiteral("test.italic"))->keys, QList<QKeySequence>{k("Ctrl+I")});
        map.save();
        QVERIFY(Settings::get().value(QStringLiteral("keys/custom")).toString().isEmpty() || Settings::get().value(QStringLiteral("keys/custom")).toString() == QLatin1String("{}"));
        map.applyAll();
        QCOMPARE(keysOf(w.act(QStringLiteral("test.italic"))), QStringList{QStringLiteral("Ctrl+I")});
        QVERIFY(w.act(QStringLiteral("test.plain"))->shortcuts().isEmpty());
    }

    void theKeysHoldForANewWindowAndEveryOpenOne()
    {
        KeyWindow first;
        {
            KeyMap map(&first);
            map.add(QStringLiteral("test.plain"), k("Ctrl+Shift+P"));
            map.add(QStringLiteral("test.two"), k("Ctrl+Shift+2"));
            map.remove(QStringLiteral("test.two"), k("Ctrl+Alt+2"));
            map.save();
            map.applyAll();
        }
        KeyWindow second;   // (applies the saved keys as it starts)
        QCOMPARE(keysOf(second.act(QStringLiteral("test.plain"))), QStringList{QStringLiteral("Ctrl+Shift+P")});
        QCOMPARE(keysOf(second.act(QStringLiteral("test.two"))), (QStringList{QStringLiteral("Ctrl+Shift+2"), QStringLiteral("Ctrl+2")}));
        // an open window takes a later change too
        KeyMap map(&second);
        map.add(QStringLiteral("test.italic"), k("Ctrl+Shift+J"));
        map.save();
        map.applyAll();
        QCOMPARE(keysOf(first.act(QStringLiteral("test.italic"))), (QStringList{QStringLiteral("Ctrl+Shift+J"), QStringLiteral("Ctrl+I")}));
        QCOMPARE(keysOf(second.act(QStringLiteral("test.italic"))), (QStringList{QStringLiteral("Ctrl+Shift+J"), QStringLiteral("Ctrl+I")}));
    }

    void reservedKeysAreRefused()
    {
        for (const char *key : {"Ctrl+C", "Ctrl+V", "Ctrl+X", "Ctrl+Z", "Ctrl+A", "Tab", "Shift+Tab", "Return", "Escape", "Backspace", "Delete", "Left", "Shift+End", "Space", "B", "Shift+7", "Ctrl+Left", "Ctrl+Shift+Right", "Ctrl+Home", "Ctrl+Shift+End", "Ctrl+Backspace", "Ctrl+Delete", "Ctrl+Y", "Ctrl+Shift+Z", "F6", "Shift+F6", "F1"})
            QVERIFY2(KeyMap::isReserved(k(key)), key);
        for (const char *key : {"Ctrl+B", "Ctrl+Shift+Y", "F5", "Alt+Left", "Ctrl+Up", "Ctrl+Alt+7", "Ctrl+Return"})
            QVERIFY2(!KeyMap::isReserved(k(key)), key);
        QVERIFY(!KeyMap::reservedList().isEmpty());
        KeyWindow w;
        KeyMap map(&w);
        for (const char *key : {"Ctrl+C", "Ctrl+A", "Tab", "Escape", "Q"}) {
            const auto r = map.add(QStringLiteral("test.plain"), k(key), true);
            QVERIFY2(r.status == KeyMap::Add::Reserved, key);
        }
        QVERIFY(map.command(QStringLiteral("test.plain"))->keys.isEmpty());
        // a reserved key a command has by default stays with it
        QVERIFY(!map.remove(QStringLiteral("edit.undo"), k("Ctrl+Z")));
        QCOMPARE(map.command(QStringLiteral("edit.undo"))->keys, QList<QKeySequence>{k("Ctrl+Z")});
        // and a settings file cannot give one away
        setSaved(R"({"test.plain": ["Ctrl+C"], "edit.undo": []})");
        KeyWindow other;
        QVERIFY(other.act(QStringLiteral("test.plain"))->shortcuts().isEmpty());
        QCOMPARE(keysOf(other.act(QStringLiteral("edit.undo"))), QStringList{QStringLiteral("Ctrl+Z")});
    }

    void aMalformedSettingsFileIsIgnored()
    {
        const QList<QByteArray> hostile = {
            "not json at all", "[1, 2, 3]", "\"text\"", "{\"test.bold\": 5}", "{\"test.bold\": [5, null, {}]}", "{\"test.bold\": [\"Ctrl+Nonsense+\"]}",
            "{\"test.bold\": [\"\"]}", "{\"test.bold\": [\"Ctrl+B, Ctrl+K\"]}", "{\"no.such.command\": [\"Ctrl+Shift+Q\"]}",
            "{\"test.bold\": [\"Ctrl+Shift+B\", \"Ctrl+Shift+B\"], \"test.italic\": [\"Ctrl+Shift+B\"]}"};
        for (const QByteArray &json : hostile) {
            setSaved(json);
            KeyWindow w;   // (must not crash, warn, or take a hostile file's word)
            const QString what = QString::fromUtf8(json);
            QVERIFY2(!keysOf(w.act(QStringLiteral("test.bold"))).contains(QStringLiteral("Ctrl+Nonsense+")), qPrintable(what));
            QVERIFY2(w.act(QStringLiteral("test.bold"))->shortcuts().size() <= KeyMap::kMaxKeysPerCommand, qPrintable(what));
            QVERIFY2(w.act(QStringLiteral("no.such")) == nullptr, qPrintable(what));
            // (two commands never share a key)
            QSet<QString> seen;
            for (const QString &id : w.actionIds())
                for (const QString &key : keysOf(w.act(id))) {
                    QVERIFY2(!seen.contains(key), qPrintable(what + QLatin1Char(' ') + key));
                    seen.insert(key);
                }
        }
        // the ones that are wholly bad leave every command as it was
        for (const QByteArray &json : QList<QByteArray>{"not json at all", "[1, 2, 3]", "{\"test.bold\": 5}", "{\"test.bold\": [\"Ctrl+Nonsense+\"]}", "{\"no.such.command\": [\"Ctrl+Shift+Q\"]}"}) {
            setSaved(json);
            KeyWindow w;
            QCOMPARE(keysOf(w.act(QStringLiteral("test.bold"))), QStringList{QStringLiteral("Ctrl+B")});
            QCOMPARE(keysOf(w.act(QStringLiteral("test.italic"))), QStringList{QStringLiteral("Ctrl+I")});
        }
        // too many entries: the ones past the cap are left, and the window still starts
        QJsonObject many;
        for (int i = 0; i < KeyMap::kMaxEntries; ++i) many.insert(QStringLiteral("a.junk.%1").arg(i, 5, 10, QLatin1Char('0')), QJsonArray{QStringLiteral("Ctrl+Shift+Q")});
        many.insert(QStringLiteral("test.bold"), QJsonArray{QStringLiteral("Ctrl+Shift+B")});   // (the entry after the last that is counted)
        setSaved(QJsonDocument(many).toJson(QJsonDocument::Compact));
        KeyWindow w;
        QCOMPARE(keysOf(w.act(QStringLiteral("test.bold"))), QStringList{QStringLiteral("Ctrl+B")});
        // (and one inside the cap is taken)
        many = QJsonObject();
        for (int i = 0; i < KeyMap::kMaxEntries - 1; ++i) many.insert(QStringLiteral("a.junk.%1").arg(i, 5, 10, QLatin1Char('0')), QJsonArray{QStringLiteral("Ctrl+Shift+Q")});
        many.insert(QStringLiteral("test.bold"), QJsonArray{QStringLiteral("Ctrl+Shift+B")});
        setSaved(QJsonDocument(many).toJson(QJsonDocument::Compact));
        KeyWindow inside;
        QCOMPARE(keysOf(inside.act(QStringLiteral("test.bold"))), QStringList{QStringLiteral("Ctrl+Shift+B")});
        // too many keys for one command
        setSaved(R"({"test.plain": ["Ctrl+Shift+1","Ctrl+Shift+2","Ctrl+Shift+3","Ctrl+Shift+4","Ctrl+Shift+5","Ctrl+Shift+6"]})");
        KeyWindow capped;
        QCOMPARE(capped.act(QStringLiteral("test.plain"))->shortcuts().size(), KeyMap::kMaxKeysPerCommand);
        // a good entry among bad ones still counts
        setSaved(R"({"junk": ["Ctrl+Shift+Q"], "test.plain": ["Ctrl+Shift+P", "x y z"]})");
        KeyWindow mixed;
        QCOMPARE(keysOf(mixed.act(QStringLiteral("test.plain"))), QStringList{QStringLiteral("Ctrl+Shift+P")});
    }

    void saveKeepsTheKeysOfCommandsThisWindowLacks()
    {
        setSaved(R"({"other.command": ["Ctrl+Shift+Q"], "test.italic": ["Ctrl+Shift+J"]})");
        KeyWindow w;
        KeyMap map(&w);
        map.add(QStringLiteral("test.plain"), k("Ctrl+Shift+P"));
        map.resetOne(QStringLiteral("test.italic"));
        map.save();
        const QJsonObject saved = QJsonDocument::fromJson(Settings::get().value(QStringLiteral("keys/custom")).toString().toUtf8()).object();
        QCOMPARE(saved.keys(), (QStringList{QStringLiteral("other.command"), QStringLiteral("test.plain")}));
        QCOMPARE(saved[QStringLiteral("other.command")].toArray().at(0).toString(), QStringLiteral("Ctrl+Shift+Q"));
    }

    void everyWindowOfTheAppTakesTheKeysWhateverItsClass()
    {
        KeyWindow w;
        SmallWindow small;   // (another kind of window, with one of the commands)
        KeyMap map(&w);
        map.remove(QStringLiteral("test.bold"), k("Ctrl+B"));
        map.add(QStringLiteral("test.bold"), k("Ctrl+Shift+B"));
        map.add(QStringLiteral("test.italic"), k("Ctrl+Shift+J"));
        map.save();
        map.applyAll();
        QCOMPARE(keysOf(small.act(QStringLiteral("test.bold"))), QStringList{QStringLiteral("Ctrl+Shift+B")});
        QCOMPARE(keysOf(w.act(QStringLiteral("test.bold"))), QStringList{QStringLiteral("Ctrl+Shift+B")});
    }

    void aTooltipTheWindowWritesItselfIsLeftAlone()
    {
        KeyWindow w;
        QAction *undo = w.act(QStringLiteral("edit.undo"));
        undo->setToolTip(QStringLiteral("Undo typing (Ctrl+Z)"));   // (what a window writes at each change)
        KeyMap map(&w);
        map.add(QStringLiteral("test.plain"), k("Ctrl+Shift+P"));
        map.save();
        map.applyAll();
        QCOMPARE(undo->toolTip(), QStringLiteral("Undo typing (Ctrl+Z)"));
        QCOMPARE(KeyMap::shownKey(w.act(QStringLiteral("test.bold"))), native(k("Ctrl+B")));
        QCOMPARE(KeyMap::shownKey(w.act(QStringLiteral("test.plain"))), native(k("Ctrl+Shift+P")));
        const QString old = QStringLiteral("Redo x (Ctrl+Y)");
        QCOMPARE(KeyMap::tipWithShownKey(old, QStringLiteral("Ctrl+Y"), w.act(QStringLiteral("test.bold"))), QStringLiteral("Redo x (%1)").arg(native(k("Ctrl+B"))));
        // (a command with no key shows none; a tip that does not end in the key is left as it is)
        clearSaved();
        KeyWindow bare;
        QCOMPARE(KeyMap::tipWithShownKey(old, QStringLiteral("Ctrl+Y"), bare.act(QStringLiteral("test.plain"))), QStringLiteral("Redo x"));
        QCOMPARE(KeyMap::tipWithShownKey(QStringLiteral("Redo x"), QStringLiteral("Ctrl+Y"), bare.act(QStringLiteral("test.bold"))), QStringLiteral("Redo x"));
    }

    void keysTheMapDoesNotKnowAreInUse()
    {
        KeyWindow w;
        KeyMap map(&w);
        // the window's own shortcut (F6), and a command with no name to list it by
        QCOMPARE(int(map.add(QStringLiteral("test.plain"), k("F6"), true).status), int(KeyMap::Add::Reserved));
        QCOMPARE(int(map.add(QStringLiteral("test.plain"), k("Ctrl+Shift+9"), true).status), int(KeyMap::Add::Reserved));
        QVERIFY(map.command(QStringLiteral("test.plain"))->keys.isEmpty());
        // and a settings file cannot give them away
        setSaved(R"({"test.plain": ["F6", "Ctrl+Shift+9"]})");
        KeyWindow other;
        QVERIFY(other.act(QStringLiteral("test.plain"))->shortcuts().isEmpty());
    }

    void saveUndoAndRedoKeepAKey()
    {
        KeyWindow w;
        KeyMap map(&w);
        QVERIFY(map.add(QStringLiteral("file.save"), k("Ctrl+Shift+S")).status == KeyMap::Add::Added);
        QVERIFY(map.remove(QStringLiteral("file.save"), k("Ctrl+S")));   // (another key is left)
        QVERIFY(!map.remove(QStringLiteral("file.save"), k("Ctrl+Shift+S")));   // (the last is kept)
        QCOMPARE(map.command(QStringLiteral("file.save"))->keys, QList<QKeySequence>{k("Ctrl+Shift+S")});
        QVERIFY(!map.remove(QStringLiteral("edit.undo"), k("Ctrl+Z")));
    }

    void aCommandMadeLaterTakesTheSavedKeys()
    {
        setSaved(R"({"test.late": ["Ctrl+Shift+L"]})");
        KeyWindow w;
        QVERIFY(w.act(QStringLiteral("test.late")) == nullptr);
        w.addLater(QStringLiteral("test.late"), QStringLiteral("Late"));
        QCOMPARE(keysOf(w.act(QStringLiteral("test.late"))), QStringList{QStringLiteral("Ctrl+Shift+L")});
        QCOMPARE(w.act(QStringLiteral("test.late"))->toolTip(), QStringLiteral("Late (%1)").arg(native(k("Ctrl+Shift+L"))));
    }

    void tooltipsShowTheNewKey()
    {
        KeyWindow w;
        QAction *bold = w.act(QStringLiteral("test.bold")), *plain = w.act(QStringLiteral("test.plain"));
        QCOMPARE(bold->toolTip(), QStringLiteral("Bold (%1)").arg(native(k("Ctrl+B"))));
        QCOMPARE(plain->toolTip(), QStringLiteral("Plain"));   // (no key: the name alone)
        KeyMap map(&w);
        map.add(QStringLiteral("test.plain"), k("Ctrl+Shift+P"));
        map.remove(QStringLiteral("test.bold"), k("Ctrl+B"));
        map.add(QStringLiteral("test.bold"), k("Ctrl+Shift+B"));
        map.save();
        map.applyAll();
        QCOMPARE(bold->toolTip(), QStringLiteral("Bold (%1)").arg(native(k("Ctrl+Shift+B"))));
        QCOMPARE(plain->toolTip(), QStringLiteral("Plain (%1)").arg(native(k("Ctrl+Shift+P"))));
        // with no key the tooltip is the name alone, and a reset brings the old one back
        map.remove(QStringLiteral("test.bold"), k("Ctrl+Shift+B"));
        map.save();
        map.applyAll();
        QVERIFY2(!bold->toolTip().contains(QLatin1Char('(')), qPrintable(bold->toolTip()));
        map.resetAll();
        map.save();
        map.applyAll();
        QCOMPARE(bold->toolTip(), QStringLiteral("Bold (%1)").arg(native(k("Ctrl+B"))));
        QVERIFY(!plain->toolTip().contains(QLatin1Char('(')));
        // a new window shows the saved key
        map.add(QStringLiteral("test.italic"), k("Ctrl+Shift+J"), false);
        map.remove(QStringLiteral("test.italic"), k("Ctrl+I"));
        map.save();
        KeyWindow later;
        QCOMPARE(later.act(QStringLiteral("test.italic"))->toolTip(), QStringLiteral("Italic (%1)").arg(native(k("Ctrl+Shift+J"))));
    }

    void aCommandOnlyForAKeyHasItsOwnGroupAndComesLast()
    {
        ExtraWindow w({QStringLiteral("extra.alpha")});
        KeyMap map(&w);
        const KeyCommand *extra = map.command(QStringLiteral("extra.alpha"));
        QVERIFY(extra);
        QCOMPARE(extra->group, QStringLiteral("Symbols"));
        QVERIFY(extra->keyOnly);
        QVERIFY(!map.command(QStringLiteral("test.bold"))->keyOnly);
        QCOMPARE(map.commands().last().id, QStringLiteral("extra.alpha"));
    }

    void theSettingsNameAtMostTheCapOfCommandsOfOneKind()
    {
        QJsonObject o;
        for (int i = 0; i < 500; ++i) o.insert(QStringLiteral("extra.%1").arg(i, 4, 10, QLatin1Char('0')), QJsonArray{QStringLiteral("Ctrl+Shift+J")});
        o.insert(QStringLiteral("other.one"), QJsonArray{QStringLiteral("Ctrl+Shift+K")});
        o.insert(QStringLiteral("extra.notAnArray"), QStringLiteral("Ctrl+Shift+L"));
        setSaved(QJsonDocument(o).toJson(QJsonDocument::Compact));
        const QStringList ids = KeyMap::savedIds(QStringLiteral("extra."));
        QCOMPARE(ids.size(), KeyMap::kMaxExtraIds);
        QCOMPARE(ids.first(), QStringLiteral("extra.0000"));
        QVERIFY(!ids.contains(QStringLiteral("extra.notAnArray")) && !ids.contains(QStringLiteral("other.one")));
        QVERIFY(KeyMap::savedIds(QStringLiteral("nothing.")).isEmpty());
    }

    void aCommandAddedAfterTheMapIsListedAndSaved()
    {
        ExtraWindow w;
        KeyMap map(&w);
        QVERIFY(!map.addCommand(QStringLiteral("extra.late")));   // (the window has none yet)
        QVERIFY(w.makeKeyCommand(QStringLiteral("extra.late")));
        QVERIFY(map.addCommand(QStringLiteral("extra.late")));
        QVERIFY(!map.addCommand(QStringLiteral("extra.late")));
        QCOMPARE(map.command(QStringLiteral("extra.late"))->group, QStringLiteral("Symbols"));
        QCOMPARE(map.add(QStringLiteral("extra.late"), k("Ctrl+Shift+Q")).status, KeyMap::Add::Added);
        QCOMPARE(map.add(QStringLiteral("test.bold"), k("Ctrl+Shift+Q")).status, KeyMap::Add::Taken);   // (the conflict checks see it)
        map.save();
        ExtraWindow again(KeyMap::savedIds(QStringLiteral("extra.")));
        QCOMPARE(keysOf(again.act(QStringLiteral("extra.late"))), QStringList{QStringLiteral("Ctrl+Shift+Q")});
        KeyMap fresh(&again);
        fresh.resetOne(QStringLiteral("extra.late"));
        QVERIFY(fresh.command(QStringLiteral("extra.late"))->keys.isEmpty());
    }

    void everyOpenWindowMakesACommandOnlyForAKeyThatItLacks()
    {
        ExtraWindow a({QStringLiteral("extra.beta")});
        ExtraWindow b;
        QVERIFY(!b.act(QStringLiteral("extra.beta")));
        KeyMap map(&a);
        map.add(QStringLiteral("extra.beta"), k("Ctrl+Shift+B"));
        map.save();
        map.applyAll();
        QVERIFY(b.act(QStringLiteral("extra.beta")));
        QCOMPARE(keysOf(b.act(QStringLiteral("extra.beta"))), QStringList{QStringLiteral("Ctrl+Shift+B")});
        QCOMPARE(keysOf(a.act(QStringLiteral("extra.beta"))), QStringList{QStringLiteral("Ctrl+Shift+B")});
    }
};

JP_TEST_CLASS(ShellKeymapTests)
#include "tst_shell_keymap.moc"
