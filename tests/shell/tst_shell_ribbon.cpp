// The shared part of "Customize the ribbon": showing and hiding tabs and groups, tabs, groups, and commands of one's own, renaming, reordering, resetting, keeping it for a new window, KeyTips
// that do not clash, and a settings file that is not to be trusted. A small ribbon built from a file stands for an app's (each app's own test checks that its Ribbon page lists its tabs).
#include "app/appwindow.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/ribboncustom.h"
#include "app/ribbonpage.h"
#include "app/settings.h"
#include "../testrunner.h"

#include <QAction>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QToolButton>
#include <QTreeWidget>
#include <QtTest>

using namespace jp;

namespace {

const char *const kFile = R"({
  "fileKeytip": "F",
  "quickAccess": [ {"cmd": "file.save", "keytip": "1"} ],
  "tabs": [
    {"name": "Home", "keytip": "H", "groups": [
      {"name": "Clipboard", "keytip": "ZC", "items": [ {"large": "edit.paste", "keytip": "V"}, {"small": "edit.copy", "keytip": "C"} ]},
      {"name": "Font", "keytip": "ZF", "items": [ {"small": "format.bold", "keytip": "B"} ]} ]},
    {"name": "Insert", "keytip": "N", "groups": [
      {"name": "Pictures", "keytip": "ZP", "items": [ {"large": "insert.picture", "keytip": "P"} ]} ]},
    {"name": "View", "keytip": "W", "groups": [
      {"name": "Zoom", "keytip": "ZZ", "items": [ {"large": "view.zoom", "keytip": "Q"} ]} ]},
    {"name": "Chart Format", "keytip": "JC", "context": "Chart Tools", "color": "chart", "groups": [
      {"name": "Styles", "keytip": "ZS", "items": [ {"large": "chart.style", "keytip": "S"} ]} ]}
  ]})";

class RibbonWindow : public AppWindow {
public:
    RibbonWindow()
    {
        for (const char *id : {"file.save", "edit.paste", "edit.copy", "format.bold", "insert.picture", "view.zoom", "chart.style", "test.extra"}) {
            auto *a = new QAction(QString::fromLatin1(id).section(QLatin1Char('.'), 1).toUpper(), this);
            a->setObjectName(QString::fromLatin1(id));
            m_actions.insert(a->objectName(), a);
        }
        m_ribbon = new Ribbon(this);
        setMenuWidget(m_ribbon);
        RibbonParts parts;
        parts.action = [this](const QString &id) { return act(id); };
        parts.colors.insert(QStringLiteral("chart"), QColor(0x33, 0x66, 0x99));
        QString err;
        buildRibbon(m_ribbon, QByteArray(kFile), parts, this, &err);
        QVERIFY2(err.isEmpty(), qPrintable(err));
        RibbonCustom::apply(this);   // (as each app's window does, once its ribbon is built)
    }
    QAction *act(const QString &id) const override { return m_actions.value(id); }
    QStringList actionIds() const override
    {
        QStringList ids = m_actions.keys();
        ids.sort();
        return ids;
    }
    Ribbon *ribbon() const override { return m_ribbon; }

private:
    QHash<QString, QAction *> m_actions;
    Ribbon *m_ribbon = nullptr;
};

const QStringList kBuiltin = {QStringLiteral("Home"), QStringLiteral("Insert"), QStringLiteral("View")};

void setSaved(const QByteArray &json) { Settings::get().setValue(QStringLiteral("ribbon/custom"), QString::fromUtf8(json)); }
void clearSaved() { Settings::get().setValue(QStringLiteral("ribbon/custom"), QString()); }

int tabIndex(const Ribbon *r, const QString &name)
{
    for (int i = 0; i < r->tabCount(); ++i)
        if (r->tabName(i) == name) return i;
    return -1;
}

// The tabs that are always there, in the order the ribbon has them.
QStringList plainTabs(const Ribbon *r)
{
    QStringList out;
    for (int i = 0; i < r->tabCount(); ++i)
        if (r->tabContextGroup(i).isEmpty()) out << r->tabName(i);
    return out;
}

QStringList groupTitles(const RibbonTab *page)
{
    QStringList out;
    for (const RibbonGroup *g : page->groups()) out << g->title();
    return out;
}

RibbonGroup *groupNamed(Ribbon *r, const QString &tab, const QString &title)
{
    for (RibbonGroup *g : r->tab(tab)->groups())
        if (g->title() == title) return g;
    return nullptr;
}

// The commands of a group, as its buttons show them (text) and the command each one does.
QList<QToolButton *> buttonsOf(RibbonGroup *g)
{
    QList<QToolButton *> out;
    for (QWidget *w : g->controls())
        if (auto *b = qobject_cast<QToolButton *>(w)) out << b;
    return out;
}

bool clash(const QString &a, const QString &b)
{
    return !a.isEmpty() && !b.isEmpty() && (a.startsWith(b) || b.startsWith(a));
}

struct LazyTabs {
    LazyTabs() { setLazyRibbonTabs(true); }
    ~LazyTabs() { setLazyRibbonTabs(false); }
};

} // namespace

class ShellRibbonTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void init() { clearSaved(); }
    void cleanup() { clearSaved(); }

    void hidingAndShowingTabsAndGroups()
    {
        RibbonWindow w;
        Ribbon *r = w.ribbon();
        QVERIFY(r->tabVisible(tabIndex(r, QStringLiteral("Insert"))));
        RibbonCustom c;
        QVERIFY(c.setTabHidden(QStringLiteral("Insert"), true));
        QVERIFY(c.setGroupHidden(QStringLiteral("Home"), QStringLiteral("Font"), true));
        c.save();
        RibbonCustom::applyAll();
        QVERIFY(!r->tabVisible(tabIndex(r, QStringLiteral("Insert"))));
        QVERIFY(r->tabVisible(tabIndex(r, QStringLiteral("View"))));
        QVERIFY(groupNamed(r, QStringLiteral("Home"), QStringLiteral("Font"))->userHidden());
        QVERIFY(!groupNamed(r, QStringLiteral("Home"), QStringLiteral("Clipboard"))->userHidden());
        // shown again
        QVERIFY(c.setTabHidden(QStringLiteral("Insert"), false));
        QVERIFY(c.setGroupHidden(QStringLiteral("Home"), QStringLiteral("Font"), false));
        c.save();
        RibbonCustom::applyAll();
        QVERIFY(r->tabVisible(tabIndex(r, QStringLiteral("Insert"))));
        QVERIFY(!groupNamed(r, QStringLiteral("Home"), QStringLiteral("Font"))->userHidden());
        // Home cannot be turned off, nor a tab that comes and goes
        QVERIFY(!c.setTabHidden(QStringLiteral("Home"), true));
        QVERIFY(c.isEmpty());
        c.hiddenTabs << QStringLiteral("Home") << QStringLiteral("Chart Format");
        c.save();
        RibbonCustom::applyAll();
        QVERIFY(r->tabVisible(tabIndex(r, QStringLiteral("Home"))));
        QVERIFY(!r->tabUserHidden(tabIndex(r, QStringLiteral("Chart Format"))));
        // the program's own showing of a tab goes on beside the person's choice
        c = RibbonCustom();
        c.setTabHidden(QStringLiteral("View"), true);
        c.save();
        RibbonCustom::applyAll();
        r->setTabVisible(QStringLiteral("View"), false);
        r->setTabVisible(QStringLiteral("View"), true);
        QVERIFY(!r->tabVisible(tabIndex(r, QStringLiteral("View"))));
    }

    void ownTabsGroupsAndCommands()
    {
        RibbonWindow w;
        Ribbon *r = w.ribbon();
        RibbonCustom c;
        const QString tab = c.addTab(QStringLiteral("Mine"));
        QVERIFY(!tab.isEmpty());
        const QString group = c.addGroup(tab, QStringLiteral("Stuff"));
        QVERIFY(!group.isEmpty());
        QVERIFY(c.addCommand(group, QStringLiteral("format.bold")));
        QVERIFY(c.addCommand(group, QStringLiteral("test.extra")));   // (a command that is on no tab)
        QVERIFY(!c.addCommand(group, QStringLiteral("format.bold")));   // (once)
        QVERIFY(c.addCommand(group, QStringLiteral("no.such.command")));
        const QString onHome = c.addGroup(QStringLiteral("Home"), QStringLiteral("Mine too"));
        QVERIFY(c.addCommand(onHome, QStringLiteral("view.zoom")));
        const QString onChart = c.addGroup(QStringLiteral("Chart Format"), QStringLiteral("Chart extras"));
        QVERIFY(c.addCommand(onChart, QStringLiteral("edit.copy")));
        c.save();
        RibbonCustom::applyAll();
        const int at = tabIndex(r, tab);
        QVERIFY(at >= 0);
        QCOMPARE(r->tabTitle(at), QStringLiteral("Mine"));
        QCOMPARE(groupTitles(r->tabPage(at)), QStringList{QStringLiteral("Stuff")});
        RibbonGroup *g = groupNamed(r, tab, QStringLiteral("Stuff"));
        QList<QToolButton *> buttons = buttonsOf(g);
        QCOMPARE(buttons.size(), 2);   // (the command that is not there is left out)
        QCOMPARE(buttons[0]->defaultAction(), w.act(QStringLiteral("format.bold")));
        QCOMPARE(buttons[1]->defaultAction(), w.act(QStringLiteral("test.extra")));
        // on a tab of the ribbon file, after its own groups; on a tab that comes and goes
        QCOMPARE(groupTitles(r->tab(QStringLiteral("Home"))), (QStringList{QStringLiteral("Clipboard"), QStringLiteral("Font"), QStringLiteral("Mine too")}));
        QCOMPARE(groupTitles(r->tab(QStringLiteral("Chart Format"))), (QStringList{QStringLiteral("Styles"), QStringLiteral("Chart extras")}));
        QCOMPARE(buttonsOf(groupNamed(r, QStringLiteral("Home"), QStringLiteral("Mine too"))).value(0)->defaultAction(), w.act(QStringLiteral("view.zoom")));
        // a click does what the command does
        int bold = 0;
        connect(w.act(QStringLiteral("format.bold")), &QAction::triggered, this, [&bold] { ++bold; });
        buttons[0]->click();
        QCOMPARE(bold, 1);
        // applying again makes no second copy
        RibbonCustom::applyAll();
        QCOMPARE(groupTitles(r->tab(QStringLiteral("Home"))).size(), 3);
        QCOMPARE(plainTabs(r).count(tab), 1);
        // taking them away
        QVERIFY(c.remove(group));
        QVERIFY(c.remove(onHome));
        QVERIFY(c.remove(tab));
        c.save();
        RibbonCustom::applyAll();
        QCOMPARE(tabIndex(r, tab), -1);
        QCOMPARE(groupTitles(r->tab(QStringLiteral("Home"))).size(), 2);
    }

    void renaming()
    {
        RibbonWindow w;
        Ribbon *r = w.ribbon();
        RibbonCustom c;
        const QString tab = c.addTab(QStringLiteral("Mine"));
        const QString group = c.addGroup(tab, QStringLiteral("Stuff"));
        c.addCommand(group, QStringLiteral("format.bold"));
        QVERIFY(c.rename(tab, QStringLiteral("  Better   Tab ")));
        QVERIFY(c.rename(group, QStringLiteral("Better Group")));
        QVERIFY(c.renameCommand(group, 0, QStringLiteral("Heavy")));
        QVERIFY(!c.rename(tab, QStringLiteral("   ")));
        c.save();
        RibbonCustom::applyAll();
        QCOMPARE(r->tabTitle(tabIndex(r, tab)), QStringLiteral("Better Tab"));
        RibbonGroup *g = groupNamed(r, tab, QStringLiteral("Better Group"));
        QVERIFY(g);
        QToolButton *b = buttonsOf(g).value(0);
        QVERIFY(b);
        QCOMPARE(b->text(), QStringLiteral("Heavy"));
        // it does what the command does, and follows its state
        QAction *bold = w.act(QStringLiteral("format.bold"));
        int n = 0;
        connect(bold, &QAction::triggered, this, [&n] { ++n; });
        b->click();
        QCOMPARE(n, 1);
        bold->setEnabled(false);
        QVERIFY(!b->isEnabled());
        bold->setEnabled(true);
        // its own name again
        QVERIFY(c.renameCommand(group, 0, QString()));
        c.save();
        RibbonCustom::applyAll();
        QCOMPARE(buttonsOf(groupNamed(r, tab, QStringLiteral("Better Group"))).value(0)->defaultAction(), bold);
    }

    void reordering()
    {
        RibbonWindow w;
        Ribbon *r = w.ribbon();
        QCOMPARE(plainTabs(r), (QStringList{QStringLiteral("Home"), QStringLiteral("Insert"), QStringLiteral("View")}));
        RibbonCustom c;
        QVERIFY(c.moveTab(QStringLiteral("Insert"), 1, kBuiltin));
        c.save();
        RibbonCustom::applyAll();
        QCOMPARE(plainTabs(r), (QStringList{QStringLiteral("Home"), QStringLiteral("View"), QStringLiteral("Insert")}));
        // the tab that comes and goes keeps its place
        const int chart = tabIndex(r, QStringLiteral("Chart Format"));
        QCOMPARE(chart, 3);
        // a tab of one's own joins at the end, and moves in among the others
        const QString tab = c.addTab(QStringLiteral("Mine"));
        c.save();
        RibbonCustom::applyAll();
        QCOMPARE(plainTabs(r), (QStringList{QStringLiteral("Home"), QStringLiteral("View"), QStringLiteral("Insert"), tab}));
        QVERIFY(c.moveTab(tab, -2, kBuiltin));
        QVERIFY(!c.moveTab(tab, -5, kBuiltin));
        c.save();
        RibbonCustom::applyAll();
        QCOMPARE(plainTabs(r), (QStringList{QStringLiteral("Home"), tab, QStringLiteral("View"), QStringLiteral("Insert")}));
        QCOMPARE(tabIndex(r, QStringLiteral("Chart Format")), chart);
        // back in the file's order, the setting says nothing about it
        QVERIFY(c.moveTab(tab, 2, kBuiltin));
        QVERIFY(c.moveTab(QStringLiteral("Insert"), -1, kBuiltin));
        QVERIFY(c.tabOrder.isEmpty());
        // groups and commands of one's own
        const QString g1 = c.addGroup(QStringLiteral("Home"), QStringLiteral("One"));
        const QString g2 = c.addGroup(QStringLiteral("Home"), QStringLiteral("Two"));
        c.addCommand(g1, QStringLiteral("format.bold"));
        c.addCommand(g1, QStringLiteral("edit.copy"));
        QVERIFY(c.moveGroup(g2, -1));
        QVERIFY(!c.moveGroup(g2, -1));
        QVERIFY(c.moveCommand(g1, 0, 1));
        QVERIFY(!c.moveCommand(g1, 1, 1));
        c.save();
        RibbonCustom::applyAll();
        QCOMPARE(groupTitles(r->tab(QStringLiteral("Home"))), (QStringList{QStringLiteral("Clipboard"), QStringLiteral("Font"), QStringLiteral("Two"), QStringLiteral("One")}));
        QCOMPARE(buttonsOf(groupNamed(r, QStringLiteral("Home"), QStringLiteral("One"))).value(0)->defaultAction(), w.act(QStringLiteral("edit.copy")));
        // the ribbon's own groups keep their order
        QVERIFY(!c.moveGroup(QStringLiteral("Home/Font"), -1));
    }

    void resettingOneTabAndAll()
    {
        RibbonWindow w;
        Ribbon *r = w.ribbon();
        RibbonCustom c;
        c.setTabHidden(QStringLiteral("Insert"), true);
        c.setGroupHidden(QStringLiteral("Insert"), QStringLiteral("Pictures"), true);
        c.setGroupHidden(QStringLiteral("Home"), QStringLiteral("Font"), true);
        c.addGroup(QStringLiteral("Insert"), QStringLiteral("Extra"));
        c.moveTab(QStringLiteral("Insert"), 1, kBuiltin);
        const QString tab = c.addTab(QStringLiteral("Mine"));
        c.save();
        RibbonCustom::applyAll();
        QVERIFY(!r->tabVisible(tabIndex(r, QStringLiteral("Insert"))));
        QCOMPARE(groupTitles(r->tab(QStringLiteral("Insert"))), (QStringList{QStringLiteral("Pictures"), QStringLiteral("Extra")}));
        // One: only that tab goes back
        c.resetTab(QStringLiteral("Insert"));
        c.save();
        RibbonCustom::applyAll();
        QVERIFY(r->tabVisible(tabIndex(r, QStringLiteral("Insert"))));
        QVERIFY(!groupNamed(r, QStringLiteral("Insert"), QStringLiteral("Pictures"))->userHidden());
        QCOMPARE(groupTitles(r->tab(QStringLiteral("Insert"))), QStringList{QStringLiteral("Pictures")});
        QVERIFY(groupNamed(r, QStringLiteral("Home"), QStringLiteral("Font"))->userHidden());
        QVERIFY(tabIndex(r, tab) >= 0);
        // a tab of one's own is taken away by it
        c.resetTab(tab);
        c.save();
        RibbonCustom::applyAll();
        QCOMPARE(tabIndex(r, tab), -1);
        // All
        c.resetAll();
        QVERIFY(c.isEmpty());
        c.save();
        RibbonCustom::applyAll();
        QVERIFY(!groupNamed(r, QStringLiteral("Home"), QStringLiteral("Font"))->userHidden());
        QCOMPARE(plainTabs(r), kBuiltin);
        QVERIFY(Settings::get().value(QStringLiteral("ribbon/custom")).toString().isEmpty());   // (nothing is kept for what is as the file has it)
    }

    void theCapsOfWhatCanBeMade()
    {
        RibbonCustom c;
        QStringList tabs;
        for (int i = 0; i < RibbonCustom::kMaxTabs; ++i) tabs << c.addTab(QStringLiteral("T%1").arg(i));
        QVERIFY(!tabs.contains(QString()));
        QVERIFY(c.addTab(QStringLiteral("One too many")).isEmpty());
        for (int i = 0; i < RibbonCustom::kMaxGroups; ++i) QVERIFY(!c.addGroup(tabs[0], QStringLiteral("G%1").arg(i)).isEmpty());
        QVERIFY(c.addGroup(tabs[0], QStringLiteral("One too many")).isEmpty());
        QVERIFY(c.addGroup(QStringLiteral("user:999"), QStringLiteral("On no tab")).isEmpty());   // (a tab of one's own that is not there)
        const QString g = c.groups.first().id;
        for (int i = 0; i < RibbonCustom::kMaxItems; ++i) QVERIFY(c.addCommand(g, QStringLiteral("cmd.%1").arg(i)));
        QVERIFY(!c.addCommand(g, QStringLiteral("cmd.extra")));
    }

    void aNewWindowHasThem()
    {
        RibbonCustom c;
        c.setTabHidden(QStringLiteral("Insert"), true);
        const QString tab = c.addTab(QStringLiteral("Mine"));
        const QString group = c.addGroup(tab, QStringLiteral("Stuff"));
        c.addCommand(group, QStringLiteral("format.bold"));
        c.moveTab(QStringLiteral("View"), -1, kBuiltin);
        c.save();
        RibbonWindow later;
        Ribbon *r = later.ribbon();
        QVERIFY(!r->tabVisible(tabIndex(r, QStringLiteral("Insert"))));
        QCOMPARE(plainTabs(r), (QStringList{QStringLiteral("Home"), QStringLiteral("View"), QStringLiteral("Insert"), tab}));
        QCOMPARE(buttonsOf(groupNamed(r, tab, QStringLiteral("Stuff"))).value(0)->defaultAction(), later.act(QStringLiteral("format.bold")));
    }

    void tabsMadeLateTakeThemWhenMade()
    {
        LazyTabs lazy;
        RibbonCustom c;
        c.setGroupHidden(QStringLiteral("Insert"), QStringLiteral("Pictures"), true);
        const QString extra = c.addGroup(QStringLiteral("Insert"), QStringLiteral("Extra"));
        c.addCommand(extra, QStringLiteral("format.bold"));
        c.addGroup(QStringLiteral("Chart Format"), QStringLiteral("Chart extras"));
        c.save();
        RibbonWindow w;
        Ribbon *r = w.ribbon();
        RibbonTab *insert = r->tab(QStringLiteral("Insert"));
        QVERIFY(!insert->isBuilt());   // (not made yet: nothing was done to it)
        QCOMPARE(groupTitles(insert), (QStringList{QStringLiteral("Pictures"), QStringLiteral("Extra")}));
        QVERIFY(groupNamed(r, QStringLiteral("Insert"), QStringLiteral("Pictures"))->userHidden());
        QCOMPARE(buttonsOf(groupNamed(r, QStringLiteral("Insert"), QStringLiteral("Extra"))).size(), 1);
        QCOMPARE(groupTitles(r->tab(QStringLiteral("Chart Format"))), (QStringList{QStringLiteral("Styles"), QStringLiteral("Chart extras")}));
        // a change after the tab was made is applied to it
        c.setGroupHidden(QStringLiteral("Insert"), QStringLiteral("Pictures"), false);
        c.save();
        RibbonCustom::applyAll();
        QVERIFY(!groupNamed(r, QStringLiteral("Insert"), QStringLiteral("Pictures"))->userHidden());
        // a change made while a tab was not made yet is applied when it is
        RibbonWindow second;
        RibbonTab *view = second.ribbon()->tab(QStringLiteral("View"));
        QVERIFY(!view->isBuilt());
        c.setGroupHidden(QStringLiteral("View"), QStringLiteral("Zoom"), true);
        c.save();
        RibbonCustom::applyAll();
        QVERIFY(!view->isBuilt());
        QVERIFY(groupNamed(second.ribbon(), QStringLiteral("View"), QStringLiteral("Zoom"))->userHidden());
    }

    void keyTipsOfWhatIsMadeDoNotClash()
    {
        RibbonCustom c;
        // names that want letters the ribbon has
        const QStringList names = {QStringLiteral("Home"), QStringLiteral("Insert"), QStringLiteral("Wonderful"), QStringLiteral("Mine"), QStringLiteral("Zebra"), QStringLiteral("Home"), QStringLiteral("1")};
        for (const QString &n : names) {
            const QString t = c.addTab(n);
            const QString g1 = c.addGroup(t, n);
            Q_UNUSED(g1);
        }
        for (const QString &n : {QStringLiteral("Zebra"), QStringLiteral("Clip"), QStringLiteral("Font"), QStringLiteral("Zoo"), QStringLiteral("ZC")}) c.addGroup(QStringLiteral("Home"), n);
        c.save();
        RibbonWindow w;
        Ribbon *r = w.ribbon();
        QStringList tips = {r->fileKeytip(), QStringLiteral("1"), QStringLiteral("0")};
        int own = 0;
        for (int i = 0; i < r->tabCount(); ++i) {
            tips << r->tabKeytip(i);
            if (r->tabName(i).startsWith(QLatin1String("user:"))) {
                ++own;
                QVERIFY2(!r->tabKeytip(i).isEmpty(), qPrintable(r->tabName(i)));
            }
        }
        QCOMPARE(own, names.size());
        for (int i = 0; i < tips.size(); ++i)
            for (int j = i + 1; j < tips.size(); ++j) QVERIFY2(!clash(tips[i], tips[j]), qPrintable(tips[i] + QLatin1Char('/') + tips[j]));
        // on a tab: the groups and their controls
        RibbonTab *home = r->tab(QStringLiteral("Home"));
        QStringList inTab;
        for (QWidget *wd : home->findChildren<QWidget *>())
            if (const QString k = keytip(wd); !k.isEmpty()) inTab << k;
        int madeGroups = 0;
        for (const RibbonGroup *g : home->groups())
            if (g->id().startsWith(QLatin1String("user:"))) {
                ++madeGroups;
                QVERIFY2(!keytip(g).isEmpty(), qPrintable(g->title()));
            }
        QCOMPARE(madeGroups, 5);
        for (int i = 0; i < inTab.size(); ++i)
            for (int j = i + 1; j < inTab.size(); ++j) QVERIFY2(!clash(inTab[i], inTab[j]), qPrintable(inTab[i] + QLatin1Char('/') + inTab[j]));
        // the letters are chosen from the name first
        QCOMPARE(RibbonCustom::freeKeytip(QStringLiteral("Mine"), {}, 1), QStringLiteral("M"));
        QCOMPARE(RibbonCustom::freeKeytip(QStringLiteral("Mine"), {QStringLiteral("M")}, 1), QStringLiteral("I"));
        QCOMPARE(RibbonCustom::freeKeytip(QStringLiteral("Mine"), {QStringLiteral("MI")}, 2), QStringLiteral("MM"));
        QVERIFY(RibbonCustom::freeKeytip(QStringLiteral("Mine"), {QStringLiteral("M"), QStringLiteral("I")}, 2) != QStringLiteral("MI"));
    }

    void aSettingsFileThatIsNotToBeTrusted()
    {
        // not a file we wrote
        for (const char *bad : {"", "{", "[1,2,3]", "42", "\"text\"", "null", "{\"tabs\":5,\"groups\":\"x\",\"hiddenTabs\":{\"a\":1}}"}) {
            setSaved(bad);
            RibbonWindow w;
            QCOMPARE(plainTabs(w.ribbon()), kBuiltin);
            QVERIFY(RibbonCustom::load().isEmpty());
        }
        // names that are not there, ids that are not ours, entries of the wrong kind, and too many of everything
        QJsonObject o;
        o.insert(QStringLiteral("hiddenTabs"), QJsonArray{QStringLiteral("Home"), QStringLiteral("Nowhere"), 7, QJsonValue(), QStringLiteral("Chart Format"), QStringLiteral("user:1")});
        o.insert(QStringLiteral("hiddenGroups"), QJsonArray{QStringLiteral("Home/Nowhere"), QStringLiteral("Nowhere/Font"), QStringLiteral("nonsense"), QStringLiteral("Home/Font")});
        o.insert(QStringLiteral("order"), QJsonArray{QStringLiteral("View"), QStringLiteral("Ghost"), QStringLiteral("View"), QStringLiteral("Home"), 5, QStringLiteral("Chart Format")});
        QJsonArray tabs, groups;
        for (int i = 0; i < 25; ++i) tabs << QJsonObject{{QStringLiteral("id"), QStringLiteral("user:%1").arg(i + 1)}, {QStringLiteral("name"), QStringLiteral("Tab %1").arg(i)}};
        tabs << QJsonObject{{QStringLiteral("id"), QStringLiteral("user:1")}, {QStringLiteral("name"), QStringLiteral("Twin")}};
        tabs << QJsonObject{{QStringLiteral("id"), QStringLiteral("Home")}, {QStringLiteral("name"), QStringLiteral("Not ours")}};
        tabs << 12 << QJsonArray();
        QJsonArray items;
        for (int i = 0; i < 100; ++i) items << QJsonObject{{QStringLiteral("cmd"), QStringLiteral("format.bold")}, {QStringLiteral("name"), QString(100, QLatin1Char('x'))}};
        items << QStringLiteral("format.bold") << QJsonObject{{QStringLiteral("cmd"), 5}} << QJsonObject{{QStringLiteral("cmd"), QStringLiteral("")}};
        for (int i = 0; i < 40; ++i)
            groups << QJsonObject{{QStringLiteral("id"), QStringLiteral("user:%1").arg(100 + i)}, {QStringLiteral("tab"), QStringLiteral("user:2")}, {QStringLiteral("name"), QStringLiteral("G%1").arg(i)}, {QStringLiteral("items"), items}};
        groups << QJsonObject{{QStringLiteral("id"), QStringLiteral("user:300")}, {QStringLiteral("tab"), QStringLiteral("user:99")}, {QStringLiteral("name"), QStringLiteral("Orphan")}}   // (a tab that is not there)
               << QJsonObject{{QStringLiteral("id"), QStringLiteral("x")}, {QStringLiteral("tab"), QStringLiteral("Home")}, {QStringLiteral("name"), QStringLiteral("Bad id")}}
               << QJsonObject{{QStringLiteral("id"), QStringLiteral("user:301")}, {QStringLiteral("tab"), QStringLiteral("Home")}, {QStringLiteral("name"), QStringLiteral("On Home")}, {QStringLiteral("items"), QJsonArray{QJsonObject{{QStringLiteral("cmd"), QStringLiteral("nothing.here")}}}}}
               << QJsonObject{{QStringLiteral("id"), QStringLiteral("user:302")}, {QStringLiteral("tab"), QStringLiteral("Mystery")}, {QStringLiteral("name"), QStringLiteral("On a tab that is not there")}};
        o.insert(QStringLiteral("tabs"), tabs);
        o.insert(QStringLiteral("groups"), groups);
        setSaved(QJsonDocument(o).toJson(QJsonDocument::Compact));
        const RibbonCustom loaded = RibbonCustom::load();
        QCOMPARE(loaded.tabs.size(), RibbonCustom::kMaxTabs);
        QCOMPARE(int(loaded.groupsOf(QStringLiteral("user:2")).size()), RibbonCustom::kMaxGroups);
        QCOMPARE(int(loaded.groupsOf(QStringLiteral("user:2")).first()->items.size()), RibbonCustom::kMaxItems);
        QVERIFY(loaded.tabs.first().name.size() <= RibbonCustom::kMaxName);
        QVERIFY(!loaded.hiddenTabs.contains(QStringLiteral("Home")));
        RibbonWindow w;
        Ribbon *r = w.ribbon();
        QVERIFY(r->tabVisible(tabIndex(r, QStringLiteral("Home"))));
        QVERIFY(!r->tabUserHidden(tabIndex(r, QStringLiteral("Chart Format"))));
        QVERIFY(groupNamed(r, QStringLiteral("Home"), QStringLiteral("Font"))->userHidden());
        QVERIFY(!groupNamed(r, QStringLiteral("Home"), QStringLiteral("Clipboard"))->userHidden());
        QCOMPARE(groupTitles(r->tab(QStringLiteral("Home"))).size(), 3);   // (the one with a command that is not there is still a group)
        int own = 0;
        for (int i = 0; i < r->tabCount(); ++i) own += r->tabName(i).startsWith(QLatin1String("user:"));
        QCOMPARE(own, RibbonCustom::kMaxTabs);
        QCOMPARE(groupTitles(r->tab(QStringLiteral("user:2"))).size(), RibbonCustom::kMaxGroups);
        QCOMPARE(buttonsOf(r->tab(QStringLiteral("user:2"))->groups().first()).size(), RibbonCustom::kMaxItems);
        QCOMPARE(r->tabTitle(tabIndex(r, QStringLiteral("user:2"))), QStringLiteral("Tab 1"));
        // the order lists what is there, once each; what it does not list follows
        QCOMPARE(plainTabs(r).mid(0, 3), (QStringList{QStringLiteral("View"), QStringLiteral("Home"), QStringLiteral("Insert")}));
        QCOMPARE(tabIndex(r, QStringLiteral("Chart Format")), 3);
        // saved again from the loaded copy, it is no larger
        QVERIFY(loaded.toJson().size() < QJsonDocument(o).toJson(QJsonDocument::Compact).size());
        QCOMPARE(RibbonCustom::fromJson(loaded.toJson()), loaded);
    }

    void theRibbonPage()
    {
        RibbonWindow w;
        Ribbon *r = w.ribbon();
        RibbonPage page(&w);
        auto *tree = page.findChild<QTreeWidget *>(QStringLiteral("ribbonTree"));
        auto *search = page.findChild<QLineEdit *>(QStringLiteral("ribbonSearch"));
        auto *list = page.findChild<QListWidget *>(QStringLiteral("ribbonCommands"));
        auto *name = page.findChild<QLineEdit *>(QStringLiteral("ribbonName"));
        QVERIFY(tree && search && list && name);
        auto button = [&](const char *object) { return page.findChild<QPushButton *>(QString::fromLatin1(object)); };
        auto topLevel = [&] {
            QStringList out;
            for (int i = 0; i < tree->topLevelItemCount(); ++i) out << tree->topLevelItem(i)->text(0);
            return out;
        };
        // the tabs and their groups; Home has no box to clear, and the others do; the tab that comes and goes can only have groups added
        QCOMPARE(topLevel().mid(0, 3), (QStringList{QStringLiteral("Home"), QStringLiteral("Insert"), QStringLiteral("View")}));
        QVERIFY(topLevel().last().contains(QLatin1String("Chart Format")));
        QTreeWidgetItem *home = tree->topLevelItem(0), *insert = tree->topLevelItem(1);
        QVERIFY(!(home->flags() & Qt::ItemIsUserCheckable));
        QVERIFY(insert->flags() & Qt::ItemIsUserCheckable);
        QVERIFY(!(tree->topLevelItem(tree->topLevelItemCount() - 1)->flags() & Qt::ItemIsUserCheckable));
        QCOMPARE(home->childCount(), 2);
        QCOMPARE(list->count(), 8);
        // the command list searches
        search->setText(QStringLiteral("zoom"));
        int shown = 0;
        for (int i = 0; i < list->count(); ++i) shown += !list->item(i)->isHidden();
        QCOMPARE(shown, 1);
        // hide a tab and a group
        insert->setCheckState(0, Qt::Unchecked);
        QVERIFY(page.plan().hiddenTabs.contains(QStringLiteral("Insert")));
        tree->setCurrentItem(tree->topLevelItem(0)->child(1));
        QVERIFY(button("ribbonResetOne")->isEnabled() == false);
        tree->topLevelItem(0)->child(1)->setCheckState(0, Qt::Unchecked);
        QVERIFY(page.plan().hiddenGroups.contains(QStringLiteral("Home/Font")));
        QVERIFY(button("ribbonResetOne")->isEnabled());
        // a tab of one's own, a group in it, and commands in that
        name->setText(QStringLiteral("Mine"));
        button("ribbonNewTab")->click();
        QCOMPARE(page.plan().tabs.size(), 1);
        QCOMPARE(page.plan().tabs.first().name, QStringLiteral("Mine"));
        name->setText(QStringLiteral("Stuff"));
        button("ribbonNewGroup")->click();
        QCOMPARE(page.plan().groups.size(), 1);
        search->setText(QStringLiteral("bold"));
        for (int i = 0; i < list->count(); ++i)
            if (!list->item(i)->isHidden()) list->setCurrentRow(i);
        button("ribbonAddCommand")->click();
        QCOMPARE(page.plan().groups.first().items.size(), 1);
        QCOMPARE(page.plan().groups.first().items.first().cmd, QStringLiteral("format.bold"));
        // rename the command, then the group, and move the tab up
        name->setText(QStringLiteral("Heavy"));
        button("ribbonRename")->click();
        QCOMPARE(page.plan().groups.first().items.first().name, QStringLiteral("Heavy"));
        // nothing has changed in the window until OK
        QVERIFY(r->tabVisible(tabIndex(r, QStringLiteral("Insert"))));
        QCOMPARE(tabIndex(r, QStringLiteral("user:1")), -1);
        page.apply();
        QVERIFY(!r->tabVisible(tabIndex(r, QStringLiteral("Insert"))));
        QVERIFY(tabIndex(r, QStringLiteral("user:1")) >= 0);
        QCOMPARE(buttonsOf(groupNamed(r, QStringLiteral("user:1"), QStringLiteral("Stuff"))).value(0)->text(), QStringLiteral("Heavy"));
        // Reset All
        QVERIFY(button("ribbonResetAll")->isEnabled());
        button("ribbonResetAll")->click();
        QVERIFY(page.plan().isEmpty());
        page.apply();
        QCOMPARE(plainTabs(r), kBuiltin);
        QVERIFY(r->tabVisible(tabIndex(r, QStringLiteral("Insert"))));
    }
};

JP_TEST_CLASS(ShellRibbonTests)
#include "tst_shell_ribbon.moc"
