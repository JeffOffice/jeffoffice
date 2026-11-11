#include "app/ribboncustom.h"

#include "app/appwindow.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/settings.h"

#include <QAction>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QPointer>
#include <QRegularExpression>
#include <QSet>
#include <QToolButton>
#include <algorithm>

namespace jp {

namespace {

const char *const kSetting = "ribbon/custom";
const char *const kUserGroup = "jpUserGroup";   // set on a group the person made
const char *const kHooked = "jpPlanHooked";     // set on a tab page that will take the customizing when it is made
const char *const kHolder = "jpRibbonPlan";

bool isUserId(const QString &id)
{
    static const QRegularExpression re(QStringLiteral("^user:[0-9]{1,6}$"));
    return re.match(id).hasMatch();
}

QString cleanName(const QString &s) { return s.simplified().left(RibbonCustom::kMaxName); }

QStringList readStrings(const QJsonValue &v, int maxCount, const std::function<bool(const QString &)> &ok)
{
    QStringList out;
    if (!v.isArray()) return out;
    for (const QJsonValue &e : v.toArray()) {
        if (out.size() >= maxCount) break;
        const QString s = e.toString();
        if (e.isString() && !s.isEmpty() && s.size() <= 200 && ok(s) && !out.contains(s)) out << s;
    }
    return out;
}

// What the settings say, as text (empty when it is not one, or too long to be ours).
QByteArray savedText()
{
    return Settings::get().value(QLatin1String(kSetting)).toString().left(1 << 20).toUtf8();
}

} // namespace

bool RibbonCustom::isEmpty() const
{
    return hiddenTabs.isEmpty() && hiddenGroups.isEmpty() && tabOrder.isEmpty() && tabs.isEmpty() && groups.isEmpty();
}

bool RibbonCustom::operator==(const RibbonCustom &o) const
{
    return toJson() == o.toJson();
}

QByteArray RibbonCustom::toJson() const
{
    QJsonObject o;
    auto strings = [](const QStringList &l) {
        QJsonArray a;
        for (const QString &s : l) a << s;
        return a;
    };
    if (!hiddenTabs.isEmpty()) o.insert(QStringLiteral("hiddenTabs"), strings(hiddenTabs));
    if (!hiddenGroups.isEmpty()) o.insert(QStringLiteral("hiddenGroups"), strings(hiddenGroups));
    if (!tabOrder.isEmpty()) o.insert(QStringLiteral("order"), strings(tabOrder));
    if (!tabs.isEmpty()) {
        QJsonArray a;
        for (const Tab &t : tabs) a << QJsonObject{{QStringLiteral("id"), t.id}, {QStringLiteral("name"), t.name}};
        o.insert(QStringLiteral("tabs"), a);
    }
    if (!groups.isEmpty()) {
        QJsonArray a;
        for (const Group &g : groups) {
            QJsonArray items;
            for (const Item &it : g.items) {
                QJsonObject io{{QStringLiteral("cmd"), it.cmd}};
                if (!it.name.isEmpty()) io.insert(QStringLiteral("name"), it.name);
                items << io;
            }
            a << QJsonObject{{QStringLiteral("id"), g.id}, {QStringLiteral("tab"), g.tab}, {QStringLiteral("name"), g.name}, {QStringLiteral("items"), items}};
        }
        o.insert(QStringLiteral("groups"), a);
    }
    return QJsonDocument(o).toJson(QJsonDocument::Compact);
}

RibbonCustom RibbonCustom::fromJson(const QByteArray &json)
{
    RibbonCustom c;
    if (json.isEmpty()) return c;
    const QJsonDocument doc = QJsonDocument::fromJson(json);
    if (!doc.isObject()) return c;
    const QJsonObject o = doc.object();
    c.hiddenTabs = readStrings(o.value(QLatin1String("hiddenTabs")), kMaxEntries, [](const QString &s) { return s != QLatin1String("Home") && !s.startsWith(QLatin1String("user:")); });
    c.hiddenGroups = readStrings(o.value(QLatin1String("hiddenGroups")), kMaxEntries, [](const QString &s) { return s.contains(QLatin1Char('/')) && !s.startsWith(QLatin1Char('/')); });
    c.tabOrder = readStrings(o.value(QLatin1String("order")), kMaxEntries, [](const QString &) { return true; });
    QSet<QString> ids;
    for (const QJsonValue &v : o.value(QLatin1String("tabs")).toArray()) {
        if (c.tabs.size() >= kMaxTabs) break;
        const QJsonObject t = v.toObject();
        const QString id = t.value(QLatin1String("id")).toString(), name = cleanName(t.value(QLatin1String("name")).toString());
        if (!isUserId(id) || ids.contains(id) || name.isEmpty()) continue;
        ids.insert(id);
        c.tabs.append({id, name});
    }
    QHash<QString, int> perTab;
    for (const QJsonValue &v : o.value(QLatin1String("groups")).toArray()) {
        const QJsonObject g = v.toObject();
        Group out;
        out.id = g.value(QLatin1String("id")).toString();
        out.tab = g.value(QLatin1String("tab")).toString();
        out.name = cleanName(g.value(QLatin1String("name")).toString());
        if (!isUserId(out.id) || ids.contains(out.id) || out.name.isEmpty() || out.tab.isEmpty() || out.tab.size() > 200) continue;
        if (out.tab.startsWith(QLatin1String("user:")) && !c.tab(out.tab)) continue;   // (a tab of the person's that is not there)
        if (perTab[out.tab] >= kMaxGroups || c.groups.size() >= kMaxTabs * kMaxGroups) continue;
        ids.insert(out.id);
        ++perTab[out.tab];
        for (const QJsonValue &iv : g.value(QLatin1String("items")).toArray()) {
            if (out.items.size() >= kMaxItems) break;
            const QJsonObject io = iv.toObject();
            const QString cmd = io.value(QLatin1String("cmd")).toString();
            if (cmd.isEmpty() || cmd.size() > 200) continue;
            out.items.append({cmd, cleanName(io.value(QLatin1String("name")).toString())});
        }
        c.groups.append(out);
    }
    return c;
}

RibbonCustom RibbonCustom::load()
{
    return fromJson(savedText());
}

void RibbonCustom::save() const
{
    Settings::get().setValue(QLatin1String(kSetting), isEmpty() ? QString() : QString::fromUtf8(toJson()));
}

const RibbonCustom::Tab *RibbonCustom::tab(const QString &id) const
{
    for (const Tab &t : tabs)
        if (t.id == id) return &t;
    return nullptr;
}

const RibbonCustom::Group *RibbonCustom::group(const QString &id) const
{
    for (const Group &g : groups)
        if (g.id == id) return &g;
    return nullptr;
}

RibbonCustom::Group *RibbonCustom::groupRef(const QString &id)
{
    for (Group &g : groups)
        if (g.id == id) return &g;
    return nullptr;
}

QList<const RibbonCustom::Group *> RibbonCustom::groupsOf(const QString &tab) const
{
    QList<const Group *> out;
    for (const Group &g : groups)
        if (g.tab == tab) out << &g;
    return out;
}

QString RibbonCustom::nextId() const
{
    int n = 0;
    auto note = [&n](const QString &id) { n = std::max(n, id.mid(5).toInt()); };
    for (const Tab &t : tabs) note(t.id);
    for (const Group &g : groups) note(g.id);
    return QStringLiteral("user:%1").arg(n + 1);
}

bool RibbonCustom::setTabHidden(const QString &tab, bool hidden)
{
    if (tab.isEmpty() || tab == QLatin1String("Home") || tab.startsWith(QLatin1String("user:"))) return false;
    if (hidden == hiddenTabs.contains(tab)) return false;
    if (hidden) {
        if (hiddenTabs.size() >= kMaxEntries) return false;
        hiddenTabs << tab;
    } else {
        hiddenTabs.removeAll(tab);
    }
    return true;
}

bool RibbonCustom::setGroupHidden(const QString &tab, const QString &group, bool hidden)
{
    const QString key = tab + QLatin1Char('/') + group;
    if (tab.isEmpty() || group.isEmpty() || hidden == hiddenGroups.contains(key)) return false;
    if (hidden) {
        if (hiddenGroups.size() >= kMaxEntries) return false;
        hiddenGroups << key;
    } else {
        hiddenGroups.removeAll(key);
    }
    return true;
}

QString RibbonCustom::addTab(const QString &name)
{
    const QString n = cleanName(name);
    if (n.isEmpty() || tabs.size() >= kMaxTabs) return QString();
    const QString id = nextId();
    tabs.append({id, n});
    return id;
}

QString RibbonCustom::addGroup(const QString &tab, const QString &name)
{
    const QString n = cleanName(name);
    if (n.isEmpty() || tab.isEmpty()) return QString();
    if (tab.startsWith(QLatin1String("user:")) && !this->tab(tab)) return QString();
    if (groupsOf(tab).size() >= kMaxGroups) return QString();
    const QString id = nextId();
    groups.append({id, tab, n, {}});
    return id;
}

bool RibbonCustom::addCommand(const QString &groupId, const QString &cmd)
{
    Group *g = groupRef(groupId);
    if (!g || cmd.isEmpty() || g->items.size() >= kMaxItems) return false;
    for (const Item &it : std::as_const(g->items))
        if (it.cmd == cmd) return false;
    g->items.append({cmd, QString()});
    return true;
}

bool RibbonCustom::rename(const QString &id, const QString &name)
{
    const QString n = cleanName(name);
    if (n.isEmpty()) return false;
    for (Tab &t : tabs)
        if (t.id == id) {
            t.name = n;
            return true;
        }
    if (Group *g = groupRef(id)) {
        g->name = n;
        return true;
    }
    return false;
}

bool RibbonCustom::renameCommand(const QString &groupId, int index, const QString &name)
{
    Group *g = groupRef(groupId);
    if (!g || index < 0 || index >= g->items.size()) return false;
    g->items[index].name = cleanName(name);
    return true;
}

QStringList RibbonCustom::effectiveOrder(const QStringList &builtin) const
{
    QStringList out;
    for (const QString &n : tabOrder)
        if ((builtin.contains(n) || tab(n)) && !out.contains(n)) out << n;
    for (const QString &n : builtin)
        if (!out.contains(n)) out << n;
    for (const Tab &t : tabs)
        if (!out.contains(t.id)) out << t.id;
    return out;
}

bool RibbonCustom::moveTab(const QString &id, int delta, const QStringList &builtin)
{
    QStringList order = effectiveOrder(builtin);
    const int at = int(order.indexOf(id)), to = at + delta;
    if (at < 0 || to < 0 || to >= order.size() || delta == 0) return false;
    order.move(at, to);
    QStringList plain = builtin;
    for (const Tab &t : tabs) plain << t.id;
    tabOrder = order == plain ? QStringList() : order;
    return true;
}

bool RibbonCustom::moveGroup(const QString &id, int delta)
{
    int at = -1;
    for (int i = 0; i < groups.size(); ++i)
        if (groups[i].id == id) at = i;
    if (at < 0 || delta == 0) return false;
    const int step = delta > 0 ? 1 : -1;
    for (int i = at + step; i >= 0 && i < groups.size(); i += step)
        if (groups[i].tab == groups[at].tab) {
            groups.swapItemsAt(at, i);
            return true;
        }
    return false;
}

bool RibbonCustom::moveCommand(const QString &groupId, int index, int delta)
{
    Group *g = groupRef(groupId);
    const int to = index + delta;
    if (!g || index < 0 || index >= g->items.size() || to < 0 || to >= g->items.size() || delta == 0) return false;
    g->items.move(index, to);
    return true;
}

bool RibbonCustom::remove(const QString &id)
{
    for (int i = 0; i < tabs.size(); ++i)
        if (tabs[i].id == id) {
            tabs.removeAt(i);
            groups.removeIf([&](const Group &g) { return g.tab == id; });
            tabOrder.removeAll(id);
            return true;
        }
    for (int i = 0; i < groups.size(); ++i)
        if (groups[i].id == id) {
            groups.removeAt(i);
            return true;
        }
    return false;
}

bool RibbonCustom::removeCommand(const QString &groupId, int index)
{
    Group *g = groupRef(groupId);
    if (!g || index < 0 || index >= g->items.size()) return false;
    g->items.removeAt(index);
    return true;
}

void RibbonCustom::resetTab(const QString &tab)
{
    if (tab.startsWith(QLatin1String("user:"))) {
        remove(tab);
        return;
    }
    hiddenTabs.removeAll(tab);
    hiddenGroups.removeIf([&](const QString &k) { return k.startsWith(tab + QLatin1Char('/')); });
    groups.removeIf([&](const Group &g) { return g.tab == tab; });
}

void RibbonCustom::resetAll()
{
    *this = RibbonCustom();
}

// ---- KeyTips

QString RibbonCustom::freeKeytip(const QString &name, const QStringList &used, int length)
{
    QString letters;
    for (const QChar c : name.toUpper())
        if (((c >= QLatin1Char('A') && c <= QLatin1Char('Z')) || (c >= QLatin1Char('0') && c <= QLatin1Char('9'))) && !letters.contains(c)) letters += c;
    QString all = letters;
    for (char c = 'A'; c <= 'Z'; ++c)
        if (!all.contains(QLatin1Char(c))) all += QLatin1Char(c);
    for (char c = '1'; c <= '9'; ++c)
        if (!all.contains(QLatin1Char(c))) all += QLatin1Char(c);
    auto free = [&](const QString &k) {
        for (const QString &u : used) {
            const QString up = u.toUpper();
            if (!up.isEmpty() && (up.startsWith(k) || k.startsWith(up))) return false;
        }
        return true;
    };
    if (length == 1) {
        for (const QChar c : all)
            if (free(QString(c))) return QString(c);
        return QString();
    }
    for (const QChar a : all)
        for (const QChar b : all)
            if (free(QString(a) + b)) return QString(a) + b;
    return QString();
}

// ---- applying

namespace {

struct Holder : QObject {
    explicit Holder(QObject *parent) : QObject(parent) { setObjectName(QLatin1String(kHolder)); }
    RibbonCustom plan;
    QPointer<AppWindow> win;
};

Holder *holderOf(Ribbon *r)
{
    return static_cast<Holder *>(r->findChild<QObject *>(QLatin1String(kHolder), Qt::FindDirectChildrenOnly));
}

// A command shown under a name of the person's: it does what the command does, and follows its state.
QAction *renamedCommand(QAction *orig, const QString &text, QObject *parent)
{
    auto *p = new QAction(text, parent);
    auto sync = [p, orig] {
        p->setIcon(orig->icon());
        p->setCheckable(orig->isCheckable());
        if (p->isChecked() != orig->isChecked()) p->setChecked(orig->isChecked());
        p->setEnabled(orig->isEnabled());
        p->setVisible(orig->isVisible());
        // plain: reviewed: the command's own tooltip and help, as it made them
        p->setToolTip(orig->toolTip());
        // plain: reviewed: the command's own help text
        p->setWhatsThis(orig->whatsThis());
    };
    sync();
    QObject::connect(orig, &QAction::changed, p, sync);
    QObject::connect(p, &QAction::triggered, orig, [orig] { orig->trigger(); });
    return p;
}

RibbonGroup *addUserGroup(RibbonTab *page, const RibbonCustom::Group &g, AppWindow *win)
{
    RibbonGroup *rg = page->addGroup(g.name);
    rg->setId(g.id);
    rg->setProperty(kUserGroup, true);
    for (const RibbonCustom::Item &it : g.items) {
        QAction *a = win->act(it.cmd);
        if (!a || a->menu()) continue;
        rg->addSmall(it.name.isEmpty() ? a : renamedCommand(a, it.name, rg));
    }
    describeForScreenReaders(rg);
    return rg;
}

QStringList keytipsIn(QWidget *root)
{
    QStringList out;
    for (QWidget *w : root->findChildren<QWidget *>())
        if (const QString k = keytip(w); !k.isEmpty()) out << k;
    return out;
}

// The letters of a tab's own groups and controls and those of the person's groups, which take free ones.
void placeUserGroups(RibbonTab *page, const QString &tabKey, const RibbonCustom &plan, AppWindow *win)
{
    QStringList used = keytipsIn(page);
    for (const RibbonCustom::Group *g : plan.groupsOf(tabKey)) {
        RibbonGroup *rg = addUserGroup(page, *g, win);
        const QString k = RibbonCustom::freeKeytip(g->name, used, 2);
        if (!k.isEmpty()) {
            setKeytip(rg, k);
            used << k;
        }
    }
}

void customizeBuiltIn(RibbonTab *page, const QString &tabName, const RibbonCustom &plan, AppWindow *win)
{
    QList<RibbonGroup *> mine;
    for (RibbonGroup *g : page->groups()) {
        if (g->property(kUserGroup).toBool()) mine << g;
        else g->setUserHidden(plan.hiddenGroups.contains(tabName + QLatin1Char('/') + g->id()));
    }
    for (RibbonGroup *g : mine) page->removeGroup(g);
    placeUserGroups(page, tabName, plan, win);
    page->finish();
    page->settle();
}

} // namespace

void RibbonCustom::apply(AppWindow *win)
{
    if (!win || !win->ribbon()) return;
    if (savedText().isEmpty() && !holderOf(win->ribbon())) return;
    load().applyTo(win);
}

void RibbonCustom::applyAll()
{
    const RibbonCustom plan = load();
    for (AppWindow *w : AppWindow::windows()) plan.applyTo(w);
}

void RibbonCustom::applyTo(AppWindow *win) const
{
    Ribbon *r = win ? win->ribbon() : nullptr;
    if (!r) return;
    Holder *h = holderOf(r);
    if (!h) {
        if (isEmpty()) return;
        h = new Holder(r);
    }
    h->plan = *this;
    h->win = win;
    const QString current = r->current() ? [&] {
        for (int i = 0; i < r->tabCount(); ++i)
            if (r->tabPage(i) == r->current()) return r->tabName(i);
        return QString();
    }() : QString();

    // The person's tabs are made again each time, in order.
    for (int i = r->tabCount() - 1; i >= 0; --i)
        if (r->tabName(i).startsWith(QLatin1String("user:"))) r->removeTab(r->tabName(i));
    QStringList used = {QStringLiteral("0")};
    used << (r->fileKeytip().isEmpty() ? QStringLiteral("F") : r->fileKeytip());
    int n = 0;
    for (const QToolButton *b : r->quickAccessButtons()) {
        ++n;
        used << (keytip(b).isEmpty() ? QString::number(n) : keytip(b));
    }
    for (int i = 0; i < r->tabCount(); ++i) used << r->tabKeytip(i);
    for (const Tab &t : tabs) {
        RibbonTab *page = r->addTab(t.id, QString(), QColor(), t.name, false);
        const QString k = freeKeytip(t.name, used, 1);
        if (!k.isEmpty()) {
            r->setTabKeytip(page, k);
            used << k;
        }
        placeUserGroups(page, t.id, *this, win);
        page->finish();
    }

    // The tabs the ribbon file made.
    for (int i = 0; i < r->tabCount(); ++i) {
        const QString name = r->tabName(i);
        if (name.startsWith(QLatin1String("user:"))) continue;
        r->setTabUserHidden(name, hiddenTabs.contains(name));
        RibbonTab *page = r->tabPage(i);
        if (!page) continue;
        if (page->isBuilt()) {
            customizeBuiltIn(page, name, *this, win);
        } else if (!page->property(kHooked).toBool()) {
            // (made when first shown: the customizing of the day it is made is what it gets)
            page->setProperty(kHooked, true);
            QObject::connect(page, &RibbonTab::built, h, [h, page, name] {
                if (h->win) customizeBuiltIn(page, name, h->plan, h->win);
            });
        }
    }
    r->arrangeTabs(tabOrder);
    if (!current.isEmpty() && r->tab(current) && r->current() != r->tab(current)) r->showTab(r->tab(current));
}

} // namespace jp
