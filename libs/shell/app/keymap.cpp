#include "app/keymap.h"

#include "app/appwindow.h"
#include "app/plaintext.h"
#include "app/ribbon.h"
#include "app/settings.h"

#include <QAction>
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QSet>
#include <QShortcut>
#include <QToolButton>
#include <algorithm>
#include <functional>
#include <typeinfo>

namespace jp {

namespace {

const char *const kSetting = "keys/custom";
const char *const kDefaultKeys = "jpDefaultKeys";   // the command's own keys, noted before any custom key changes them
const char *const kTipOrig = "jpTipOrig";           // the tooltip as the window made it
const char *const kApplied = "jpKeysApplied";         // set on a window once it has applied the saved keys
const char *const kTipBase = "jpTipBase";           // the command's tooltip without the keys

QString commandText(const QAction *a)
{
    QString s = a->text();
    s.remove(QLatin1Char('&'));
    if (s.endsWith(QLatin1String("..."))) s.chop(3);
    if (s.endsWith(QChar(0x2026))) s.chop(1);
    return s.simplified();
}

QString portable(const QKeySequence &k) { return k.toString(QKeySequence::PortableText); }

// Notes a command's own keys and tooltip the first time it is seen.
void capture(QAction *a)
{
    if (a->property(kDefaultKeys).isValid()) return;
    QVariantList keys;
    for (const QKeySequence &k : a->shortcuts()) keys << QVariant::fromValue(k);
    a->setProperty(kDefaultKeys, keys);
    QString tip = a->toolTip();
    if (!a->shortcuts().isEmpty()) {
        // (the apps write the tooltip as "Name (Keys)", with the command's first key)
        const QString suffix = QStringLiteral(" (%1)").arg(a->shortcuts().first().toString(QKeySequence::NativeText));
        if (tip.endsWith(suffix)) tip.chop(suffix.size());
    }
    a->setProperty(kTipBase, tip);
    a->setProperty(kTipOrig, a->toolTip());
}

QList<QKeySequence> defaultsOf(const QAction *a)
{
    QList<QKeySequence> out;
    for (const QVariant &v : a->property(kDefaultKeys).toList()) out << v.value<QKeySequence>();
    return out;
}

// What a command shows as its tooltip: its text, then its first key in brackets.
void showKeysInTooltip(QAction *a, const QList<QKeySequence> &keys)
{
    const QString base = a->property(kTipBase).toString();
    if (keys == defaultsOf(a)) {
        // plain: reviewed: the tooltip the window made, given back
        a->setToolTip(a->property(kTipOrig).toString());
        return;
    }
    if (keys.isEmpty()) {
        // plain: reviewed: the tooltip the window made, without its key
        a->setToolTip(base);
        return;
    }
    const QString key = keys.first().toString(QKeySequence::NativeText);
    if (base.isEmpty()) a->setToolTip(plainToolTip(QCoreApplication::translate("KeyMap", "%1 (%2)").arg(commandText(a), key)));
    else a->setToolTip(plainToolTip(QCoreApplication::translate("KeyMap", "%1 (%2)").arg(QString(base).remove(QLatin1Char('&')), key)));
}

void setKeys(AppWindow *win, QAction *a, const QList<QKeySequence> &keys, const QSet<QAction *> &inWindow)
{
    capture(a);
    if (a->shortcuts() != keys) a->setShortcuts(keys);
    if (!keys.isEmpty()) {
        a->setShortcutContext(Qt::WindowShortcut);
        if (!inWindow.contains(a)) win->addAction(a);   // (a command that had no key was not added to the window)
    }
    showKeysInTooltip(a, keys);
}

QSet<QAction *> actionsOf(const AppWindow *win)
{
    const QList<QAction *> list = win->actions();
    return QSet<QAction *>(list.begin(), list.end());
}

// The saved text as the object it holds (empty when it is not one).
QJsonObject savedObject()
{
    const QByteArray json = Settings::get().value(QLatin1String(kSetting)).toString().left(1 << 20).toUtf8();
    if (json.isEmpty()) return {};
    const QJsonDocument doc = QJsonDocument::fromJson(json);
    return doc.isObject() ? doc.object() : QJsonObject();
}

QSet<QString> idsOf(const AppWindow *win)
{
    const QStringList ids = win->actionIds();
    return QSet<QString>(ids.begin(), ids.end());
}

// The saved differences: id -> keys, read with every check a file that is not to be trusted needs. `ids` are the commands the window has; entries beyond the cap are left.
QHash<QString, QList<QKeySequence>> readSaved(const QSet<QString> &ids)
{
    QHash<QString, QList<QKeySequence>> out;
    const QJsonObject o = savedObject();
    int seen = 0;
    for (auto it = o.begin(); it != o.end(); ++it) {
        if (++seen > KeyMap::kMaxEntries) break;
        if (!ids.contains(it.key()) || !it.value().isArray()) continue;
        QList<QKeySequence> keys;
        for (const QJsonValue &v : it.value().toArray()) {
            if (keys.size() >= KeyMap::kMaxKeysPerCommand) break;
            if (!v.isString()) continue;
            const QKeySequence k = KeyMap::parseKey(v.toString());
            if (!k.isEmpty() && !keys.contains(k)) keys << k;
        }
        // (an empty list is a command with its keys taken off; an entry with only bad keys is no entry)
        if (keys.isEmpty() && !it.value().toArray().isEmpty()) continue;
        out.insert(it.key(), keys);
    }
    return out;
}

QList<QKeySequence> reservedIn(const QList<QKeySequence> &keys)
{
    QList<QKeySequence> out;
    for (const QKeySequence &k : keys)
        if (KeyMap::isReserved(k)) out << k;
    return out;
}

// Which tab of the ribbon each command is on, in the order the ribbon shows them.
void ribbonPlaces(AppWindow *win, QHash<QString, QString> *tabOf, QHash<QString, int> *order)
{
    Ribbon *r = win->ribbon();
    if (!r) return;
    int n = 0;
    auto note = [&](QAction *a, const QString &tab) {
        if (!a || a->objectName().isEmpty() || tabOf->contains(a->objectName())) return;
        tabOf->insert(a->objectName(), tab);
        order->insert(a->objectName(), n++);
    };
    std::function<void(QMenu *, const QString &)> walkMenu = [&](QMenu *m, const QString &tab) {
        for (QAction *a : m->actions()) {
            if (a->menu()) walkMenu(a->menu(), tab);
            else note(a, tab);
        }
    };
    for (int i = 0; i < r->tabCount(); ++i) {
        RibbonTab *page = r->tabPage(i);
        if (!page) continue;
        const QString tab = r->tabTitle(i);
        for (RibbonGroup *g : page->groups())
            for (QToolButton *b : g->findChildren<QToolButton *>()) {
                if (b == g->launcher()) continue;
                note(b->defaultAction(), tab);
                if (QMenu *m = b->menu()) walkMenu(m, tab);
            }
    }
    for (QToolButton *b : r->quickAccessButtons()) note(b->defaultAction(), QCoreApplication::translate("KeyMap", "Quick Toolbar"));
}

} // namespace

KeyMap::KeyMap(AppWindow *win, bool withRibbon) : m_win(win)
{
    QHash<QString, QString> tabOf;
    QHash<QString, int> order;
    if (withRibbon) ribbonPlaces(win, &tabOf, &order);
    const QString other = QCoreApplication::translate("KeyMap", "Other");
    QSet<QString> done;
    for (const QString &id : win->actionIds()) {
        QAction *a = win->act(id);
        if (!a || done.contains(id)) continue;
        const QString text = commandText(a);
        if (a->menu() || text.isEmpty()) {
            // (a command the page does not list still has its keys in use)
            for (const QKeySequence &k : a->shortcuts()) m_foreign << k;
            continue;
        }
        done.insert(id);
        capture(a);
        KeyCommand c;
        c.id = id;
        c.text = text;
        c.group = tabOf.value(id, other);
        c.defaults = defaultsOf(a);
        c.keys = c.defaults;
        m_commands << c;
    }
    // The window's own shortcuts (F6 among the parts of the window).
    for (const QShortcut *sc : win->findChildren<QShortcut *>())
        if (!sc->key().isEmpty()) m_foreign << sc->key();
    // The ribbon's order; the commands on no tab last, by name.
    if (withRibbon) std::stable_sort(m_commands.begin(), m_commands.end(), [&](const KeyCommand &a, const KeyCommand &b) {
        const int oa = order.value(a.id, INT_MAX), ob = order.value(b.id, INT_MAX);
        if (oa != ob) return oa < ob;
        return a.text.localeAwareCompare(b.text) < 0;
    });
    for (int i = 0; i < m_commands.size(); ++i) m_index.insert(m_commands[i].id, i);
    // The saved differences over the commands' own keys.
    const auto saved = readSaved(QSet<QString>(m_index.keyBegin(), m_index.keyEnd()));
    QStringList ids = saved.keys();
    ids.sort();
    for (const QString &id : ids) {
        KeyCommand &c = m_commands[m_index[id]];
        QList<QKeySequence> keys;
        for (const QKeySequence &k : saved[id])
            if (!m_foreign.contains(k)) keys << k;   // (a key that is in use elsewhere in the window is not given away)
        // (a reserved key can be neither given nor taken away)
        if (reservedIn(keys) != reservedIn(c.defaults)) {
            QList<QKeySequence> fixed = reservedIn(c.defaults);
            for (const QKeySequence &k : keys)
                if (!isReserved(k)) fixed << k;
            c.keys = fixed.mid(0, kMaxKeysPerCommand);
        } else {
            c.keys = keys;
        }
    }
    // Two commands never share a key: a command with keys of its own wins over one that kept its defaults, and else the first by id.
    QSet<QString> changedIds(ids.begin(), ids.end());
    QHash<QKeySequence, QString> holder;
    auto settle = [&](bool changedFirst) {
        for (KeyCommand &c : m_commands) {
            if (changedIds.contains(c.id) != changedFirst) continue;
            for (const QKeySequence &k : QList<QKeySequence>(c.keys)) {
                auto h = holder.constFind(k);
                if (h != holder.constEnd() && h.value() != c.id) c.keys.removeAll(k);
                else holder.insert(k, c.id);
            }
        }
    };
    // (the unchanged commands are settled after, but must lose to a changed one: take the changed ones' keys off the others first)
    for (const QString &id : ids)
        for (const QKeySequence &k : m_commands[m_index[id]].keys) holder.insert(k, id);
    for (KeyCommand &c : m_commands) {
        if (changedIds.contains(c.id)) continue;
        for (const QKeySequence &k : QList<QKeySequence>(c.keys))
            if (holder.contains(k) && !isReserved(k)) c.keys.removeAll(k);
    }
    holder.clear();
    settle(true);
    settle(false);
}

const KeyCommand *KeyMap::command(const QString &id) const
{
    const auto it = m_index.constFind(id);
    return it == m_index.constEnd() ? nullptr : &m_commands[it.value()];
}

QString KeyMap::owner(const QKeySequence &key, const QString &exceptId) const
{
    for (const KeyCommand &c : m_commands)
        if (c.id != exceptId && c.keys.contains(key)) return c.id;
    return {};
}

KeyMap::Result KeyMap::check(const QString &id, const QKeySequence &key) const
{
    Result r;
    const auto it = m_index.constFind(id);
    if (it == m_index.constEnd() || key.isEmpty() || key.count() != 1) return r;
    const KeyCommand &c = m_commands[it.value()];
    if (isReserved(key)) {
        // (a reserved key a command has already is its own)
        r.status = c.keys.contains(key) ? Add::Already : Add::Reserved;
        return r;
    }
    if (c.keys.contains(key)) {
        r.status = Add::Already;
        return r;
    }
    if (c.keys.size() >= kMaxKeysPerCommand) return r;
    if (m_foreign.contains(key)) {
        r.status = Add::Reserved;
        return r;
    }
    const QString other = owner(key, id);
    if (!other.isEmpty()) {
        r.owner = other;
        r.ownerText = m_commands[m_index[other]].text;
        r.status = Add::Taken;
        return r;
    }
    r.status = Add::Added;
    return r;
}

KeyMap::Result KeyMap::add(const QString &id, const QKeySequence &key, bool takeOver)
{
    Result r = check(id, key);
    if (r.status == Add::Taken && takeOver) {
        m_commands[m_index[r.owner]].keys.removeAll(key);
        r.status = Add::Added;
    }
    if (r.status == Add::Added) m_commands[m_index[id]].keys.prepend(key);   // (the new key comes first: it is the one tooltips and menus show)
    return r;
}

bool KeyMap::remove(const QString &id, const QKeySequence &key)
{
    const auto it = m_index.constFind(id);
    if (it == m_index.constEnd() || isReserved(key)) return false;
    KeyCommand &c = m_commands[it.value()];
    // (Save, Undo, and Redo keep at least one key)
    static const QSet<QString> needKey = {QStringLiteral("file.save"), QStringLiteral("edit.undo"), QStringLiteral("edit.redo")};
    if (needKey.contains(id) && c.keys.contains(key) && c.keys.size() == 1) return false;
    return c.keys.removeAll(key) > 0;
}

void KeyMap::resetOne(const QString &id)
{
    const auto it = m_index.constFind(id);
    if (it == m_index.constEnd()) return;
    KeyCommand &c = m_commands[it.value()];
    // (the command's own keys come back, and leave the commands that took them over)
    for (const QKeySequence &k : c.defaults) {
        if (c.keys.contains(k)) continue;
        const QString other = owner(k, id);
        if (!other.isEmpty()) m_commands[m_index[other]].keys.removeAll(k);
    }
    const QList<QKeySequence> had = c.keys;
    c.keys = c.defaults;
    // (a key it had taken from another command, which is that command's own, goes back to it)
    for (const QKeySequence &k : had) {
        if (c.defaults.contains(k) || !owner(k).isEmpty()) continue;
        for (KeyCommand &o : m_commands)
            if (o.id != id && o.defaults.contains(k) && !o.keys.contains(k) && o.keys.size() < kMaxKeysPerCommand) {
                o.keys << k;
                break;
            }
    }
}

void KeyMap::resetAll()
{
    for (KeyCommand &c : m_commands) c.keys = c.defaults;
}

bool KeyMap::anyChanged() const
{
    return std::any_of(m_commands.begin(), m_commands.end(), [](const KeyCommand &c) { return c.changed(); });
}

void KeyMap::save() const
{
    // What is saved for commands this window does not have (another kind of window of the app) stays.
    QJsonObject o;
    const QJsonObject old = savedObject();
    for (auto it = old.begin(); it != old.end() && o.size() < kMaxEntries; ++it)
        if (!m_index.contains(it.key()) && it.value().isArray()) o.insert(it.key(), it.value());
    for (const KeyCommand &c : m_commands) {
        if (!c.changed()) continue;
        QJsonArray keys;
        for (const QKeySequence &k : c.keys) keys << portable(k);
        o.insert(c.id, keys);
    }
    Settings::get().setValue(QLatin1String(kSetting), o.isEmpty() ? QString() : QString::fromUtf8(QJsonDocument(o).toJson(QJsonDocument::Compact)));
}

void KeyMap::applyAll() const
{
    for (AppWindow *w : AppWindow::windows()) {
        const QSet<QString> has = idsOf(w);
        const QSet<QAction *> inWindow = actionsOf(w);
        for (const KeyCommand &c : m_commands) {
            if (!has.contains(c.id)) continue;
            QAction *a = w->act(c.id);
            // (only a command whose keys changed: the others keep what the window wrote in their tooltips)
            if (a && a->shortcuts() != c.keys) setKeys(w, a, c.keys, inWindow);
        }
    }
}

bool KeyMap::isReserved(const QKeySequence &key)
{
    if (key.count() != 1) return false;
    static const QList<QKeySequence> fixed = [] {
        QList<QKeySequence> v;
        v << QKeySequence(Qt::CTRL | Qt::Key_C) << QKeySequence(Qt::CTRL | Qt::Key_V) << QKeySequence(Qt::CTRL | Qt::Key_X) << QKeySequence(Qt::CTRL | Qt::Key_Z)
          << QKeySequence(Qt::CTRL | Qt::Key_A) << QKeySequence(Qt::CTRL | Qt::Key_Insert) << QKeySequence(Qt::SHIFT | Qt::Key_Insert) << QKeySequence(Qt::SHIFT | Qt::Key_Delete)
          << QKeySequence(Qt::ALT | Qt::Key_F4);
        return v;
    }();
    if (fixed.contains(key)) return true;
    const QKeyCombination c = key[0];
    const Qt::KeyboardModifiers mods = c.keyboardModifiers() & ~Qt::KeypadModifier;
    const int k = c.key();
    // The parts of the window (Help, the ribbon's collapse, F6 among the panes), Redo's two keys, and what a text box does with Ctrl (by word, to the ends, delete by word),
    // with or without Shift.
    if (mods == Qt::NoModifier && k == Qt::Key_F1) return true;
    if (mods == Qt::ControlModifier && (k == Qt::Key_F1 || k == Qt::Key_Y)) return true;
    if ((mods == Qt::NoModifier || mods == Qt::ShiftModifier) && k == Qt::Key_F6) return true;
    if (mods == (Qt::ControlModifier | Qt::ShiftModifier) && k == Qt::Key_Z) return true;
    if ((mods == Qt::ControlModifier || mods == (Qt::ControlModifier | Qt::ShiftModifier))
        && (k == Qt::Key_Left || k == Qt::Key_Right || k == Qt::Key_Home || k == Qt::Key_End || (mods == Qt::ControlModifier && (k == Qt::Key_Backspace || k == Qt::Key_Delete))))
        return true;
    // A key that only types a character (alone or with Shift).
    if (k > 0 && k < Qt::Key_Escape && !(mods & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier))) return true;
    // The keys that move the caret and edit, alone or with Shift, and the ones that confirm and leave.
    if (!(mods & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier))) {
        switch (k) {
        case Qt::Key_Tab: case Qt::Key_Backtab: case Qt::Key_Return: case Qt::Key_Enter: case Qt::Key_Backspace: case Qt::Key_Delete: case Qt::Key_Left: case Qt::Key_Right:
        case Qt::Key_Up: case Qt::Key_Down: case Qt::Key_Home: case Qt::Key_End: case Qt::Key_PageUp: case Qt::Key_PageDown: case Qt::Key_Space:
            return true;
        case Qt::Key_Escape:
            return mods == Qt::NoModifier;
        default:
            break;
        }
    }
    return false;
}

QStringList KeyMap::reservedList()
{
    QStringList out;
    for (const QKeySequence &k : {QKeySequence(Qt::CTRL | Qt::Key_C), QKeySequence(Qt::CTRL | Qt::Key_X), QKeySequence(Qt::CTRL | Qt::Key_V), QKeySequence(Qt::CTRL | Qt::Key_Z),
                                  QKeySequence(Qt::CTRL | Qt::Key_A), QKeySequence(Qt::Key_Tab), QKeySequence(Qt::SHIFT | Qt::Key_Tab), QKeySequence(Qt::Key_Return),
                                  QKeySequence(Qt::Key_Escape), QKeySequence(Qt::Key_Backspace), QKeySequence(Qt::Key_Delete), QKeySequence(Qt::ALT | Qt::Key_F4)})
        out << k.toString(QKeySequence::NativeText);
    return out;
}

QKeySequence KeyMap::parseKey(const QString &text)
{
    const QString t = text.trimmed();
    if (t.isEmpty() || t.size() > 40) return {};
    const QKeySequence k = QKeySequence::fromString(t, QKeySequence::PortableText);
    if (k.count() != 1 || k[0].key() == Qt::Key_unknown || k[0].key() == 0) return {};
    // (a text that was read as something else than it says: "Ctrl+Nonsense+" is no key)
    if (QKeySequence::fromString(portable(k), QKeySequence::PortableText) != k) return {};
    return k;
}

void KeyMap::apply(AppWindow *win)
{
    if (!win) return;
    win->setProperty(kApplied, true);
    if (Settings::get().value(QLatin1String(kSetting)).toString().isEmpty()) return;
    KeyMap map(win, false);
    const QSet<QAction *> inWindow = actionsOf(win);
    for (const KeyCommand &c : map.commands())
        if (QAction *a = win->act(c.id)) {
            if (c.keys == c.defaults) {
                capture(a);
                continue;
            }
            setKeys(win, a, c.keys, inWindow);
        }
}

void KeyMap::applyLater(AppWindow *win, QAction *a)
{
    // (before the window has applied the keys, its own apply() does it for every command)
    if (!win || !a || !win->property(kApplied).toBool() || a->objectName().isEmpty()) return;
    capture(a);
    const QJsonObject o = savedObject();
    const auto it = o.constFind(a->objectName());
    if (it == o.constEnd() || !it->isArray()) return;
    KeyMap map(win, false);
    if (const KeyCommand *c = map.command(a->objectName()); c && c->keys != c->defaults) setKeys(win, a, c->keys, actionsOf(win));
}

QString KeyMap::shownKey(const QAction *a)
{
    return a && !a->shortcuts().isEmpty() ? a->shortcuts().first().toString(QKeySequence::NativeText) : QString();
}

QString KeyMap::tipWithShownKey(const QString &tip, const QString &oldKey, const QAction *a)
{
    const QString suffix = QStringLiteral(" (%1)").arg(oldKey);
    if (!tip.endsWith(suffix)) return tip;
    const QString key = shownKey(a);
    return tip.left(tip.size() - suffix.size()) + (key.isEmpty() ? QString() : QStringLiteral(" (%1)").arg(key));
}

} // namespace jp
