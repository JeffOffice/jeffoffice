// The shared part of "Customize the keyboard": assigning, taking a key over, resetting, keeping the keys for a new window, the reserved keys, a settings file that is not
// to be trusted, and the tooltips. A small window of a few commands stands for an app's window (each app's own test checks that its Keyboard page lists its commands).
#include "app/appwindow.h"
#include "app/keymap.h"
#include "app/plaintext.h"
#include "app/settings.h"
#include "../testrunner.h"

#include <QAction>
#include <QJsonDocument>
#include <QJsonObject>
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
        KeyMap::apply(this);
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
    void mk(const QString &id, const QString &text, const QKeySequence &key)
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
    }
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
        QCOMPARE(keysOf(w.act(QStringLiteral("test.bold"))), (QStringList{QStringLiteral("Ctrl+B"), QStringLiteral("Ctrl+Shift+B")}));
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
        QCOMPARE(keysOf(second.act(QStringLiteral("test.two"))), (QStringList{QStringLiteral("Ctrl+2"), QStringLiteral("Ctrl+Shift+2")}));
        // an open window takes a later change too
        KeyMap map(&second);
        map.add(QStringLiteral("test.italic"), k("Ctrl+Shift+J"));
        map.save();
        map.applyAll();
        QCOMPARE(keysOf(first.act(QStringLiteral("test.italic"))), (QStringList{QStringLiteral("Ctrl+I"), QStringLiteral("Ctrl+Shift+J")}));
        QCOMPARE(keysOf(second.act(QStringLiteral("test.italic"))), (QStringList{QStringLiteral("Ctrl+I"), QStringLiteral("Ctrl+Shift+J")}));
    }

    void reservedKeysAreRefused()
    {
        for (const char *key : {"Ctrl+C", "Ctrl+V", "Ctrl+X", "Ctrl+Z", "Ctrl+A", "Tab", "Shift+Tab", "Return", "Escape", "Backspace", "Delete", "Left", "Shift+End", "Space", "B", "Shift+7"})
            QVERIFY2(KeyMap::isReserved(k(key)), key);
        for (const char *key : {"Ctrl+B", "Ctrl+Shift+Z", "F5", "Alt+Left", "Ctrl+Left", "Ctrl+Alt+7", "Ctrl+Return"})
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
            QCOMPARE_NE(keysOf(w.act(QStringLiteral("test.bold"))), QStringList{QStringLiteral("nonsense")});
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
        // too many entries: the extra are left, and the window still starts
        QJsonObject many;
        for (int i = 0; i < KeyMap::kMaxEntries + 500; ++i) many.insert(QStringLiteral("junk.%1").arg(i), QJsonArray{QStringLiteral("Ctrl+Shift+Q")});
        many.insert(QStringLiteral("zzz.last"), QJsonArray{QStringLiteral("Ctrl+Shift+P")});
        setSaved(QJsonDocument(many).toJson(QJsonDocument::Compact));
        KeyWindow w;
        QCOMPARE(keysOf(w.act(QStringLiteral("test.bold"))), QStringList{QStringLiteral("Ctrl+B")});
        // too many keys for one command
        setSaved(R"({"test.plain": ["Ctrl+Shift+1","Ctrl+Shift+2","Ctrl+Shift+3","Ctrl+Shift+4","Ctrl+Shift+5","Ctrl+Shift+6"]})");
        KeyWindow capped;
        QCOMPARE(capped.act(QStringLiteral("test.plain"))->shortcuts().size(), KeyMap::kMaxKeysPerCommand);
        // a good entry among bad ones still counts
        setSaved(R"({"junk": ["Ctrl+Shift+Q"], "test.plain": ["Ctrl+Shift+P", "x y z"]})");
        KeyWindow mixed;
        QCOMPARE(keysOf(mixed.act(QStringLiteral("test.plain"))), QStringList{QStringLiteral("Ctrl+Shift+P")});
    }

    void tooltipsShowTheNewKey()
    {
        KeyWindow w;
        QAction *bold = w.act(QStringLiteral("test.bold")), *plain = w.act(QStringLiteral("test.plain"));
        QCOMPARE(bold->toolTip(), QStringLiteral("Bold (%1)").arg(native(k("Ctrl+B"))));
        QVERIFY(plain->toolTip().isEmpty() || !plain->toolTip().contains(QLatin1Char('(')));
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
};

JP_TEST_CLASS(ShellKeymapTests)
#include "tst_shell_keymap.moc"
