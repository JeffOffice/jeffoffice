// The shared part of "Customize the ribbon": showing and hiding tabs and groups, tabs, groups, and commands of one's own, renaming, reordering, resetting, keeping it for a new window, KeyTips
// that do not clash, and a settings file that is not to be trusted. A small ribbon built from a file stands for an app's (each app's own test checks that its Ribbon page lists its tabs).
#include "app/appwindow.h"
#include "app/keymap.h"
#include "app/keytips.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/quicktoolbarpage.h"
#include "app/ribboncustom.h"
#include "app/ribbonpage.h"
#include "app/settings.h"
#include "../testrunner.h"

#include <QAction>
#include <QFrame>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QToolButton>
#include <QTreeWidget>
#include <QtTest>
#include <memory>

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
      {"name": "Pictures", "keytip": "ZP", "items": [ {"large": "insert.picture", "keytip": "P"}, {"large": "insert.table", "menu": ["insert.picture", "edit.copy"], "keytip": "T"} ]} ]},
    {"name": "View", "keytip": "W", "groups": [
      {"name": "Zoom", "keytip": "ZZ", "items": [ {"large": "view.zoom", "keytip": "Q"} ]} ]},
    {"name": "Chart Format", "keytip": "JC", "context": "Chart Tools", "color": "chart", "groups": [
      {"name": "Styles", "keytip": "ZS", "items": [ {"large": "chart.style", "keytip": "S"} ]} ]}
  ]})";

class RibbonWindow : public AppWindow {
public:
    RibbonWindow()
    {
        for (const char *id : {"file.save", "edit.paste", "edit.copy", "format.bold", "insert.picture", "insert.table", "view.zoom", "chart.style", "test.extra"}) {
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
    // A command of the test's own (with an icon, so that its button is as wide whatever the font), and its end.
    QAction *addCommand(const QString &id, const QString &text, bool icon = true)
    {
        auto *a = new QAction(text, this);
        a->setObjectName(id);
        if (icon) {
            QPixmap pix(16, 16);
            pix.fill(Qt::darkGray);
            a->setIcon(QIcon(pix));
        }
        m_actions.insert(id, a);
        return a;
    }
    void dropCommand(const QString &id) { delete m_actions.take(id); }

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

// The quick toolbar's commands, as the buttons have them (an empty string for a separator is not among them: the buttons only).
QStringList quickIds(const Ribbon *r)
{
    QStringList out;
    for (const QToolButton *b : r->quickAccessButtons()) out << (b->defaultAction() ? b->defaultAction()->objectName() : QString());
    return out;
}

const QStringList kQuickDefaults = {QStringLiteral("file.save")};

QByteArray quickJson(int commands)
{
    QJsonArray a;
    for (int i = 0; i < commands; ++i) a << QJsonObject{{QStringLiteral("cmd"), QStringLiteral("test.cmd%1").arg(i)}};
    return QJsonDocument(QJsonObject{{QStringLiteral("quick"), a}}).toJson(QJsonDocument::Compact);
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
        // (and it is back in its place)
        QCOMPARE(plainTabs(r), (QStringList{QStringLiteral("Home"), QStringLiteral("Insert"), QStringLiteral("View"), tab}));
        QVERIFY(!c.tabOrder.contains(QStringLiteral("Insert")));
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
        c.addGroup(QStringLiteral("Insert"), QStringLiteral("Extra"));   // (a group with a command makes every tab, to find the menus: see the test of menus)
        c.addGroup(QStringLiteral("Chart Format"), QStringLiteral("Chart extras"));
        c.save();
        RibbonWindow w;
        Ribbon *r = w.ribbon();
        RibbonTab *insert = r->tab(QStringLiteral("Insert"));
        QVERIFY(!insert->isBuilt());   // (not made yet: nothing was done to it)
        QCOMPARE(groupTitles(insert), (QStringList{QStringLiteral("Pictures"), QStringLiteral("Extra")}));
        QVERIFY(groupNamed(r, QStringLiteral("Insert"), QStringLiteral("Pictures"))->userHidden());
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
        // the order lists what is there, once each; a tab it does not list stays where the file has it
        QCOMPARE(plainTabs(r).mid(0, 3), (QStringList{QStringLiteral("View"), QStringLiteral("Insert"), QStringLiteral("Home")}));
        QCOMPARE(tabIndex(r, QStringLiteral("Chart Format")), 3);
        // saved again from the loaded copy, it is no larger
        QVERIFY(loaded.toJson().size() < QJsonDocument(o).toJson(QJsonDocument::Compact).size());
        QCOMPARE(RibbonCustom::fromJson(loaded.toJson()), loaded);
    }

    void aCommandThatOpensAMenuKeepsItsMenuInAGroupOfOurs()
    {
        for (int lazy = 0; lazy < 2; ++lazy) {
            std::unique_ptr<LazyTabs> late(lazy ? new LazyTabs : nullptr);
            RibbonCustom c;
            const QString tab = c.addTab(QStringLiteral("Mine"));
            const QString group = c.addGroup(tab, QStringLiteral("Menus"));
            QVERIFY(c.addCommand(group, QStringLiteral("insert.table")));
            c.save();
            RibbonWindow w;
            QToolButton *b = buttonsOf(groupNamed(w.ribbon(), tab, QStringLiteral("Menus"))).value(0);
            QVERIFY(b);
            QVERIFY2(b->menu(), "the command's menu is not on its button");
            QVERIFY(b->menu()->actions().contains(w.act(QStringLiteral("insert.picture"))));
            QVERIFY(b->menu()->actions().contains(w.act(QStringLiteral("edit.copy"))));
            clearSaved();
        }
    }

    void aRenamedToggleMirrorsTheCommand()
    {
        RibbonWindow w;
        QAction *a = w.act(QStringLiteral("test.extra"));
        a->setCheckable(true);
        a->setChecked(true);
        a->setIcon(QIcon(QPixmap(8, 8)));
        a->setStatusTip(QStringLiteral("Status"));
        a->setIconText(QStringLiteral("Icon text"));
        a->setData(42);
        RibbonCustom c;
        const QString tab = c.addTab(QStringLiteral("Mine"));
        const QString group = c.addGroup(tab, QStringLiteral("Stuff"));
        c.addCommand(group, QStringLiteral("test.extra"));
        c.renameCommand(group, 0, QStringLiteral("Mark"));
        c.save();
        RibbonCustom::applyAll();
        QToolButton *b = buttonsOf(groupNamed(w.ribbon(), tab, QStringLiteral("Stuff"))).value(0);
        QVERIFY(b);
        QAction *p = b->defaultAction();
        QVERIFY(p != a);
        // as it is when made
        QVERIFY(p->isCheckable() && p->isChecked());
        QVERIFY(!p->icon().isNull());
        QCOMPARE(p->statusTip(), QStringLiteral("Status"));
        QCOMPARE(p->iconText(), QStringLiteral("Icon text"));
        QCOMPARE(p->data().toInt(), 42);
        // and as it changes
        a->setChecked(false);
        QVERIFY(!p->isChecked());
        a->setIcon(QIcon());
        QVERIFY(p->icon().isNull());
        a->setVisible(false);
        QVERIFY(!p->isVisible());
        a->setStatusTip(QStringLiteral("Other"));
        a->setIconText(QStringLiteral("Other text"));
        a->setData(7);
        QCOMPARE(p->statusTip(), QStringLiteral("Other"));
        QCOMPARE(p->iconText(), QStringLiteral("Other text"));
        QCOMPARE(p->data().toInt(), 7);
        // a click on it toggles the command
        a->setVisible(true);
        b->click();
        QVERIFY(a->isChecked());
        QVERIFY(p->isChecked());
    }

    void movingATabKeepsTheOrderNamesThisWindowLacks()
    {
        RibbonCustom c;
        c.tabOrder = {QStringLiteral("Ghost"), QStringLiteral("View"), QStringLiteral("Home")};
        QVERIFY(c.moveTab(QStringLiteral("Insert"), -1, kBuiltin));
        QVERIFY(c.tabOrder.contains(QStringLiteral("Ghost")));
        QCOMPARE(c.effectiveOrder(kBuiltin), (QStringList{QStringLiteral("Insert"), QStringLiteral("View"), QStringLiteral("Home")}));
        // back to the file's order: only what this window does not know stays
        QVERIFY(c.moveTab(QStringLiteral("Insert"), 2, kBuiltin));
        QVERIFY(c.moveTab(QStringLiteral("View"), 1, kBuiltin));
        QVERIFY(c.moveTab(QStringLiteral("Insert"), -1, kBuiltin));
        QCOMPARE(c.effectiveOrder(kBuiltin), kBuiltin);
        QCOMPARE(c.tabOrder, QStringList{QStringLiteral("Ghost")});
    }

    void anIdIsAlwaysOneThatLoadAccepts()
    {
        RibbonCustom c;
        for (int i = 0; i < RibbonCustom::kMaxTabs; ++i) c.addTab(QStringLiteral("T%1").arg(i));
        for (int i = 0; i < 2000; ++i) {
            const QString id = c.addGroup(c.tabs[i % c.tabs.size()].id, QStringLiteral("G"));
            if (id.isEmpty()) {
                // full: take one away and make another
                QVERIFY(c.remove(c.groups[(i * 7) % c.groups.size()].id));
                continue;
            }
            if (i % 3 == 0) c.remove(id);
        }
        for (const RibbonCustom::Group &g : std::as_const(c.groups)) QVERIFY2(g.id.mid(5).toInt() <= RibbonCustom::kMaxTabs + RibbonCustom::kMaxAllGroups, qPrintable(g.id));
        // every id survives a save and a load
        const RibbonCustom again = RibbonCustom::fromJson(c.toJson());
        QCOMPARE(again.groups.size(), c.groups.size());
        QCOMPARE(again.tabs.size(), c.tabs.size());
        // removing and making tabs over and over
        for (int i = 0; i < 3000; ++i) {
            const QString t = c.tabs.first().id;
            c.remove(t);
            QVERIFY(!c.addTab(QStringLiteral("N")).isEmpty());
        }
        QCOMPARE(RibbonCustom::fromJson(c.toJson()).tabs.size(), c.tabs.size());
        const RibbonCustom full = RibbonCustom::fromJson(c.toJson());
        QCOMPARE(full.groups.size(), c.groups.size());
    }

    void theRibbonPageDropsGroupsOnTabsThereAreNone()
    {
        RibbonCustom c;
        const QString onHome = c.addGroup(QStringLiteral("Home"), QStringLiteral("Mine"));
        c.addGroup(QStringLiteral("Mystery"), QStringLiteral("On a tab that is not there"));
        c.save();
        RibbonWindow w;
        RibbonPage page(&w);
        QCOMPARE(page.plan().groups.size(), 1);
        QCOMPARE(page.plan().groups.first().id, onHome);
        page.apply();
        QVERIFY(!Settings::get().value(QStringLiteral("ribbon/custom")).toString().contains(QLatin1String("Mystery")));
    }

    void theTotalOfCommandsIsCapped()
    {
        RibbonCustom c;
        const QString tab = c.addTab(QStringLiteral("Mine"));
        int made = 0;
        for (int g = 0; g < RibbonCustom::kMaxGroups; ++g) {
            const QString id = c.addGroup(tab, QStringLiteral("G%1").arg(g));
            QVERIFY(!id.isEmpty());
            for (int i = 0; i < RibbonCustom::kMaxItems; ++i)
                if (c.addCommand(id, QStringLiteral("cmd.%1").arg(i))) ++made;
        }
        QCOMPARE(made, RibbonCustom::kMaxAllItems);
        // the same from a file
        int total = 0;
        for (const RibbonCustom::Group &g : RibbonCustom::fromJson(c.toJson()).groups) total += int(g.items.size());
        QCOMPARE(total, RibbonCustom::kMaxAllItems);
        QJsonArray groups;
        for (int g = 0; g < 30; ++g) {
            QJsonArray items;
            for (int i = 0; i < 100; ++i) items << QJsonObject{{QStringLiteral("cmd"), QStringLiteral("c%1").arg(i)}};
            groups << QJsonObject{{QStringLiteral("id"), QStringLiteral("user:%1").arg(g + 1)}, {QStringLiteral("tab"), QStringLiteral("Home")}, {QStringLiteral("name"), QStringLiteral("G")}, {QStringLiteral("items"), items}};
        }
        total = 0;
        for (const RibbonCustom::Group &g : RibbonCustom::fromJson(QJsonDocument(QJsonObject{{QStringLiteral("groups"), groups}}).toJson(QJsonDocument::Compact)).groups) total += int(g.items.size());
        QCOMPARE(total, RibbonCustom::kMaxAllItems);
    }

    void applyingAgainWithATabThatComesAndGoesShown()
    {
        RibbonWindow w;
        Ribbon *r = w.ribbon();
        r->setContextVisible(QStringLiteral("Chart Tools"), true);
        RibbonCustom c;
        const QString a = c.addTab(QStringLiteral("Alpha"));
        const QString b = c.addTab(QStringLiteral("Beta"));
        c.moveTab(QStringLiteral("View"), -1, kBuiltin);
        c.save();
        for (int round = 0; round < 3; ++round) {
            RibbonCustom::applyAll();
            QCOMPARE(plainTabs(r), (QStringList{QStringLiteral("Home"), QStringLiteral("View"), QStringLiteral("Insert"), a, b}));
            QCOMPARE(tabIndex(r, QStringLiteral("Chart Format")), 3);
            QVERIFY(r->tabVisible(3));
        }
        c.remove(a);
        c.save();
        RibbonCustom::applyAll();
        QCOMPARE(plainTabs(r), (QStringList{QStringLiteral("Home"), QStringLiteral("View"), QStringLiteral("Insert"), b}));
        QCOMPARE(tabIndex(r, QStringLiteral("Chart Format")), 3);
        const QString d = c.addTab(QStringLiteral("Delta"));
        c.save();
        RibbonCustom::applyAll();
        QCOMPARE(plainTabs(r), (QStringList{QStringLiteral("Home"), QStringLiteral("View"), QStringLiteral("Insert"), b, d}));
        QCOMPARE(tabIndex(r, QStringLiteral("Chart Format")), 3);
        QVERIFY(r->tabVisible(3));
    }

    void theRibbonPageEnablesResetOneForATabThatMoved()
    {
        RibbonWindow w;
        RibbonPage page(&w);
        auto *tree = page.findChild<QTreeWidget *>(QStringLiteral("ribbonTree"));
        auto button = [&](const char *object) { return page.findChild<QPushButton *>(QString::fromLatin1(object)); };
        tree->setCurrentItem(tree->topLevelItem(1));   // Insert
        QVERIFY(!button("ribbonResetOne")->isEnabled());
        button("ribbonDown")->click();
        QCOMPARE(tree->topLevelItem(2)->text(0), QStringLiteral("Insert"));
        QVERIFY(button("ribbonResetOne")->isEnabled());
        button("ribbonResetOne")->click();
        QCOMPARE(tree->topLevelItem(1)->text(0), QStringLiteral("Insert"));
        QVERIFY(!page.plan().tabOrder.contains(QStringLiteral("Insert")));
    }

    void aTabNobodyTouchedStaysUnmadeAfterApplying()
    {
        LazyTabs lazy;
        // commands in a group on Home, and a group hidden on View's neighbor: nothing else is made
        RibbonCustom c;
        const QString g = c.addGroup(QStringLiteral("Home"), QStringLiteral("Mine"));
        c.addCommand(g, QStringLiteral("format.bold"));
        c.addCommand(g, QStringLiteral("test.extra"));
        c.save();
        RibbonWindow w;
        Ribbon *r = w.ribbon();
        QVERIFY(!r->tab(QStringLiteral("Insert"))->isBuilt());
        QVERIFY(!r->tab(QStringLiteral("View"))->isBuilt());
        QVERIFY(!r->tab(QStringLiteral("Chart Format"))->isBuilt());
        QCOMPARE(buttonsOf(groupNamed(r, QStringLiteral("Home"), QStringLiteral("Mine"))).size(), 2);
        // a command that opens a menu from its button makes only the tab that button is on
        c.addCommand(g, QStringLiteral("insert.table"));
        c.save();
        RibbonCustom::applyAll();
        QVERIFY(r->tab(QStringLiteral("Insert"))->isBuilt());
        QVERIFY(!r->tab(QStringLiteral("View"))->isBuilt());
        QVERIFY(!r->tab(QStringLiteral("Chart Format"))->isBuilt());
        QVERIFY(buttonsOf(groupNamed(r, QStringLiteral("Home"), QStringLiteral("Mine"))).value(2)->menu());
    }

    void addingRemovingAndMovingQuickCommands()
    {
        RibbonCustom c;
        QVERIFY(c.addQuickCommand(QStringLiteral("format.bold"), kQuickDefaults));
        QVERIFY(c.addQuickCommand(QStringLiteral("edit.copy"), kQuickDefaults));
        QVERIFY(!c.addQuickCommand(QStringLiteral("edit.copy"), kQuickDefaults));   // (already there)
        QVERIFY(!c.addQuickCommand(QString(), kQuickDefaults));
        QCOMPARE(c.effectiveQuick(kQuickDefaults), (QStringList{QStringLiteral("file.save"), QStringLiteral("format.bold"), QStringLiteral("edit.copy")}));
        QVERIFY(!c.isEmpty());
        QVERIFY(c.moveQuick(2, -1, kQuickDefaults));
        QVERIFY(!c.moveQuick(0, -1, kQuickDefaults));
        QVERIFY(!c.moveQuick(2, 1, kQuickDefaults));
        QCOMPARE(c.quick, (QStringList{QStringLiteral("file.save"), QStringLiteral("edit.copy"), QStringLiteral("format.bold")}));
        QVERIFY(c.removeQuick(0, kQuickDefaults));
        QVERIFY(!c.removeQuick(5, kQuickDefaults));
        QCOMPARE(c.quick, (QStringList{QStringLiteral("edit.copy"), QStringLiteral("format.bold")}));
        // a window shows them, in order
        c.save();
        RibbonWindow w;
        QCOMPARE(quickIds(w.ribbon()), (QStringList{QStringLiteral("edit.copy"), QStringLiteral("format.bold")}));
        QCOMPARE(w.ribbon()->quickAccessButtons().value(0)->defaultAction(), w.act(QStringLiteral("edit.copy")));
    }

    void aQuickToolbarSeparator()
    {
        RibbonCustom c;
        QVERIFY(c.addQuickSeparator(kQuickDefaults));
        QVERIFY(c.addQuickCommand(QStringLiteral("format.bold"), kQuickDefaults));
        QCOMPARE(c.quick, (QStringList{QStringLiteral("file.save"), QString(), QStringLiteral("format.bold")}));
        c.save();
        RibbonWindow w;
        QVERIFY(w.ribbon()->describe().contains(QLatin1String("quick access: ")));
        QVERIFY2(w.ribbon()->describe().contains(QLatin1String(", |, ")), "the separator is not on the quick toolbar");
        QCOMPARE(quickIds(w.ribbon()), (QStringList{QStringLiteral("file.save"), QStringLiteral("format.bold")}));
        QCOMPARE(w.ribbon()->findChildren<QFrame *>(QStringLiteral("jpQuickSeparator")).size(), 1);
    }

    void resettingOneQuickCommandAndAll()
    {
        RibbonCustom c;
        c.addQuickCommand(QStringLiteral("format.bold"), kQuickDefaults);
        c.addQuickCommand(QStringLiteral("edit.copy"), kQuickDefaults);
        c.removeQuick(0, kQuickDefaults);   // Save is gone
        QVERIFY(c.quickCommandChanged(QStringLiteral("file.save"), kQuickDefaults));
        QVERIFY(c.quickCommandChanged(QStringLiteral("edit.copy"), kQuickDefaults));
        QVERIFY(!c.quickCommandChanged(QStringLiteral("view.zoom"), kQuickDefaults));
        QVERIFY(c.resetQuickCommand(QStringLiteral("file.save"), kQuickDefaults));   // back, where the file has it
        QCOMPARE(c.quick.value(0), QStringLiteral("file.save"));
        QVERIFY(c.resetQuickCommand(QStringLiteral("edit.copy"), kQuickDefaults));   // not in the file: gone
        QCOMPARE(c.quick, (QStringList{QStringLiteral("file.save"), QStringLiteral("format.bold")}));
        QVERIFY(c.resetQuickCommand(QStringLiteral("format.bold"), kQuickDefaults));
        QVERIFY2(c.isEmpty(), "a quick toolbar that is the file's is not kept");
        c.addQuickCommand(QStringLiteral("format.bold"), kQuickDefaults);
        c.resetQuick();
        QVERIFY(c.isEmpty());
        QCOMPARE(c.effectiveQuick(kQuickDefaults), kQuickDefaults);
        // a window takes the file's back
        c.addQuickCommand(QStringLiteral("format.bold"), kQuickDefaults);
        c.save();
        RibbonWindow w;
        QCOMPARE(quickIds(w.ribbon()), (QStringList{QStringLiteral("file.save"), QStringLiteral("format.bold")}));
        clearSaved();
        RibbonCustom::applyAll();
        QCOMPARE(quickIds(w.ribbon()), kQuickDefaults);
    }

    void aSavedQuickToolbarIsThereInANewWindow()
    {
        RibbonCustom c;
        c.removeQuick(0, kQuickDefaults);
        c.addQuickCommand(QStringLiteral("view.zoom"), kQuickDefaults);
        c.addQuickSeparator(kQuickDefaults);
        c.addQuickCommand(QStringLiteral("format.bold"), kQuickDefaults);
        c.save();
        const RibbonCustom back = RibbonCustom::load();
        QVERIFY(back.quickSet);
        QCOMPARE(back.quick, c.quick);
        RibbonWindow later;
        QCOMPARE(quickIds(later.ribbon()), (QStringList{QStringLiteral("view.zoom"), QStringLiteral("format.bold")}));
        QCOMPARE(later.ribbon()->defaultQuickAccess().value(0).action, later.act(QStringLiteral("file.save")));   // (the ribbon file's is still known)
        QCOMPARE(int(later.ribbon()->defaultQuickAccess().size()), 1);
        QCOMPARE(RibbonCustom::appliedTo(&later).quick, c.quick);
        // taking everything out is kept too
        RibbonCustom none;
        none.removeQuick(0, kQuickDefaults);
        QVERIFY(none.quickSet);
        QVERIFY(none.quick.isEmpty());
        QVERIFY(!none.isEmpty());
        none.save();
        RibbonWindow empty;
        QVERIFY(quickIds(empty.ribbon()).isEmpty());
    }

    void aCommandTheAppNoLongerHasIsKeptButNotShown()
    {
        setSaved(R"({"quick":[{"cmd":"file.save"},{"cmd":"gone.command"},{"sep":true},{"cmd":"format.bold"}]})");
        RibbonWindow w;
        QCOMPARE(quickIds(w.ribbon()), (QStringList{QStringLiteral("file.save"), QStringLiteral("format.bold")}));
        const RibbonCustom c = RibbonCustom::load();
        QCOMPARE(c.quick, (QStringList{QStringLiteral("file.save"), QStringLiteral("gone.command"), QString(), QStringLiteral("format.bold")}));
        // the person changes something else: the command is still in the saved list
        RibbonCustom edit = c;
        edit.addQuickCommand(QStringLiteral("edit.copy"), kQuickDefaults);
        edit.save();
        QVERIFY(RibbonCustom::load().quick.contains(QStringLiteral("gone.command")));
        // a separator left with nothing to separate is not drawn twice
        setSaved(R"({"quick":[{"sep":true},{"cmd":"file.save"},{"cmd":"gone.command"},{"sep":true},{"sep":true},{"cmd":"format.bold"},{"sep":true}]})");
        RibbonCustom::applyAll();
        QCOMPARE(w.ribbon()->findChildren<QFrame *>(QStringLiteral("jpQuickSeparator")).size(), 1);
        QVERIFY(!w.ribbon()->describe().contains(QLatin1String("|, |")));
    }

    void theQuickToolbarIsCappedOnLoadAndOnAdd()
    {
        const RibbonCustom big = RibbonCustom::fromJson(quickJson(250));
        QCOMPARE(int(big.quick.size()), RibbonCustom::kMaxQuick);
        QCOMPARE(big.quick.last(), QStringLiteral("test.cmd99"));
        RibbonCustom c;
        c.quickSet = true;
        for (int i = 0; i < RibbonCustom::kMaxQuick; ++i) QVERIFY(c.addQuickCommand(QStringLiteral("test.cmd%1").arg(i), kQuickDefaults));
        QVERIFY(!c.addQuickCommand(QStringLiteral("format.bold"), kQuickDefaults));
        QVERIFY(!c.addQuickSeparator(kQuickDefaults));
        QCOMPARE(int(c.quick.size()), RibbonCustom::kMaxQuick);
        QVERIFY(c.removeQuick(0, kQuickDefaults));
        QVERIFY(c.addQuickCommand(QStringLiteral("format.bold"), kQuickDefaults));
        // none of these commands is in the window: nothing shows, and what was saved is kept
        setSaved(quickJson(250));
        RibbonWindow w;
        QVERIFY(quickIds(w.ribbon()).isEmpty());
        QCOMPARE(int(RibbonCustom::load().quick.size()), RibbonCustom::kMaxQuick);
        // a file with doubles and bad entries
        const RibbonCustom odd = RibbonCustom::fromJson(R"({"quick":[{"cmd":"a"},{"cmd":"a"},{"cmd":""},5,{"nothing":1},{"cmd":"b"}]})");
        QCOMPARE(odd.quick, (QStringList{QStringLiteral("a"), QStringLiteral("b")}));
    }

    void noTabIsMadeAtStartupForAQuickToolbar()
    {
        LazyTabs lazy;
        RibbonCustom c;
        c.addQuickCommand(QStringLiteral("format.bold"), kQuickDefaults);
        c.addQuickCommand(QStringLiteral("view.zoom"), kQuickDefaults);
        c.addQuickSeparator(kQuickDefaults);
        c.save();
        RibbonWindow w;
        Ribbon *r = w.ribbon();
        QCOMPARE(quickIds(r), (QStringList{QStringLiteral("file.save"), QStringLiteral("format.bold"), QStringLiteral("view.zoom")}));
        for (const QString &name : {QStringLiteral("Insert"), QStringLiteral("View"), QStringLiteral("Chart Format")}) QVERIFY2(!r->tab(name)->isBuilt(), qPrintable(name + QLatin1String(" was made")));
    }

    void aQuickCommandWithAMenuKeepsItsMenu()
    {
        for (int lazy = 0; lazy < 2; ++lazy) {
            std::unique_ptr<LazyTabs> late(lazy ? new LazyTabs : nullptr);
            RibbonCustom c;
            QVERIFY(c.addQuickCommand(QStringLiteral("insert.table"), kQuickDefaults));
            c.save();
            RibbonWindow w;
            Ribbon *r = w.ribbon();
            QToolButton *b = r->quickAccessButtons().value(1);
            QVERIFY(b);
            QCOMPARE(b->defaultAction(), w.act(QStringLiteral("insert.table")));
            QVERIFY2(b->menu(), "the command's menu is not on its button");
            QVERIFY(b->menu()->actions().contains(w.act(QStringLiteral("insert.picture"))));
            QVERIFY(b->menu()->actions().contains(w.act(QStringLiteral("edit.copy"))));
            if (lazy) {
                QVERIFY(r->tab(QStringLiteral("Insert"))->isBuilt());   // (the tab the menu is on, and no other)
                QVERIFY(!r->tab(QStringLiteral("View"))->isBuilt());
                QVERIFY(!r->tab(QStringLiteral("Chart Format"))->isBuilt());
            }
            clearSaved();
        }
    }

    // How many buttons the quick toolbar shows when each is `button` wide: the room is a third of the row (not under 100), the » button and the gaps come out of it.
    static int quickRoomFor(int rowWidth, int button, int gap = 2, int more = 22) { return std::max(0, (std::max(100, rowWidth / 3) - more - 6) / (button + gap)); }

    // A window whose quick toolbar has more commands than fit: `count` commands with icons (so that the buttons are the same width whatever the font), the one at `menuAt` being the command with a menu.
    static QStringList overflowing(RibbonWindow &w, int count, int menuAt = -1)
    {
        QStringList ids;
        for (int i = 0; i < count; ++i) {
            const QString id = QStringLiteral("test.many%1").arg(i);
            w.addCommand(id, QStringLiteral("Many %1").arg(i));
            ids << id;
        }
        if (menuAt >= 0) ids[menuAt] = QStringLiteral("insert.table");
        QPixmap pix(16, 16);
        pix.fill(Qt::darkGray);
        w.act(QStringLiteral("insert.table"))->setIcon(QIcon(pix));
        RibbonCustom c;
        c.quickSet = true;
        c.quick = ids;
        c.save();
        RibbonCustom::applyAll();
        w.show();
        return ids;
    }

    void aLongQuickToolbarGoesIntoAnOverflowButton()
    {
        RibbonWindow w;
        overflowing(w, 60);
        Ribbon *r = w.ribbon();
        auto shownNow = [r] {
            int n = 0;
            for (const QToolButton *b : r->quickAccessButtons()) n += !b->isHidden();
            return n;
        };
        // (the room is the row's own width, so no window size is fought with)
        QTRY_COMPARE(shownNow(), quickRoomFor(r->width(), 26));
        QVERIFY(shownNow() < 60);
        const int hidden = int(r->quickAccessHidden().size());
        QCOMPARE(hidden, 60 - shownNow());
        QToolButton *more = r->quickAccessMore();
        QVERIFY(more && !more->isHidden());
        QCOMPARE(int(more->menu()->actions().size()), hidden);
        QVERIFY(more->geometry().right() <= std::max(100, r->width() / 3));   // (the title and the tabs keep their room)
        // a short toolbar shows them all, and the » button goes
        RibbonCustom c;
        c.quickSet = true;
        c.quick = {QStringLiteral("test.many0"), QStringLiteral("test.many1")};   // (with icons: the same width in any font)
        c.save();
        RibbonCustom::applyAll();
        QTRY_VERIFY(r->quickAccessHidden().isEmpty());
        QVERIFY(more->isHidden());
    }

    void keyTipsOfAQuickToolbarAreNumbersThenLettersAndNeverClash()
    {
        RibbonWindow w;
        QStringList ids;
        for (int i = 0; i < 14; ++i) ids << w.addCommand(QStringLiteral("test.tip%1").arg(i), QStringLiteral("Tip %1").arg(i))->objectName();
        RibbonCustom c;
        c.quickSet = true;
        c.quick = ids;
        c.save();
        RibbonCustom::applyAll();
        Ribbon *r = w.ribbon();
        QStringList tips;
        for (const QToolButton *b : r->quickAccessButtons()) tips << keytip(b);
        QCOMPARE(int(tips.size()), 14);
        for (int i = 0; i < 9; ++i) QCOMPARE(tips[i], QString::number(i + 1));
        for (int i = 9; i < 14; ++i) QVERIFY2(tips[i].size() == 2 && !tips[i].at(0).isDigit(), qPrintable(tips[i]));
        QVERIFY2(!keytip(r->quickAccessMore()).isEmpty(), "the » button has no KeyTip");
        tips << keytip(r->quickAccessMore()) << r->fileKeytip() << QStringLiteral("0");
        for (int i = 0; i < r->tabCount(); ++i) tips << r->tabKeytip(i);
        // (a tab of the person's, made after, takes letters that are free too)
        RibbonCustom d = RibbonCustom::load();
        d.addTab(QStringLiteral("Tipsy"));
        d.save();
        RibbonCustom::applyAll();
        tips = {};
        for (const QToolButton *b : r->quickAccessButtons()) tips << keytip(b);
        tips << keytip(r->quickAccessMore()) << r->fileKeytip() << QStringLiteral("0");
        for (int i = 0; i < r->tabCount(); ++i) tips << r->tabKeytip(i);
        for (int i = 0; i < tips.size(); ++i)
            for (int j = i + 1; j < tips.size(); ++j) QVERIFY2(!clash(tips[i], tips[j]), qPrintable(tips[i] + QLatin1Char('/') + tips[j]));
    }

    static void typeKeyTip(QWidget *w, const QString &letters)
    {
        for (const QChar ch : letters) {
            const int key = ch.isDigit() ? Qt::Key_0 + ch.digitValue() : Qt::Key_A + (ch.toUpper().unicode() - 'A');
            QKeyEvent press(QEvent::KeyPress, key, Qt::NoModifier, QString(ch));
            qApp->sendEvent(w, &press);
        }
    }

    void theOverflowButtonHasKeyTipsAndTheCommandsInItAreReachable()
    {
        RibbonWindow w;
        const QStringList ids = overflowing(w, 40, 39);   // (the last is the command with a menu)
        QVERIFY(QTest::qWaitForWindowActive(&w));
        Ribbon *r = w.ribbon();
        QTRY_VERIFY(!r->quickAccessHidden().isEmpty());
        QToolButton *more = r->quickAccessMore();
        KeyTips tips(&w, r, [] {});
        tips.showTop();
        QStringList shown;
        QWidget *moreTarget = nullptr;
        for (const auto &t : tips.shown()) {
            shown << t.first;
            if (t.second == more) moreTarget = more;
        }
        QVERIFY2(moreTarget, "the » button shows no KeyTip");
        for (const auto &t : tips.shown()) QVERIFY2(!r->quickAccessHidden().contains(qobject_cast<QToolButton *>(t.second)), "a KeyTip is shown on a button that is not there");
        // typing a command's letters from the menu runs it: a command that is in the » menu
        QToolButton *hiddenPlain = nullptr;
        for (QToolButton *b : r->quickAccessHidden())
            if (!b->menu()) hiddenPlain = b;
        QVERIFY(hiddenPlain);
        int ran = 0;
        QObject::connect(hiddenPlain->defaultAction(), &QAction::triggered, &w, [&] { ++ran; });
        typeKeyTip(&w, keytip(hiddenPlain));
        QCOMPARE(ran, 1);
        // the » button's letters open its menu
        tips.showTop();
        bool opened = false;
        QTimer::singleShot(200, &w, [&] {
            opened = more->menu()->isVisible();
            more->menu()->close();
        });
        typeKeyTip(&w, keytip(more));
        QVERIFY2(opened, "the » button's KeyTip does not open its menu");
        Q_UNUSED(ids);
    }

    void aCommandWithAMenuInTheOverflowMenuIsTheCommand()
    {
        RibbonWindow w;
        overflowing(w, 40, 39);
        Ribbon *r = w.ribbon();
        QTRY_VERIFY(!r->quickAccessHidden().isEmpty());
        QAction *table = w.act(QStringLiteral("insert.table")), *plain = w.act(QStringLiteral("test.many38"));
        QToolButton *more = r->quickAccessMore();
        auto entry = [&](const QString &text) -> QAction * {
            for (QAction *a : more->menu()->actions())
                if (a->text() == text) return a;
            return nullptr;
        };
        QAction *plainEntry = entry(plain->text()), *tableEntry = entry(table->text());
        QVERIFY(plainEntry && tableEntry);
        QVERIFY2(plainEntry == plain || plainEntry->menu() == nullptr, "a plain command in the menu is not the command");
        QVERIFY(tableEntry->menu());
        QVERIFY(tableEntry->menu()->actions().contains(w.act(QStringLiteral("edit.copy"))));
        // it follows the command: text, enabled, and checked
        table->setEnabled(false);
        QTRY_VERIFY(!entry(table->text())->isEnabled());
        table->setEnabled(true);
        QTRY_VERIFY(entry(table->text())->isEnabled());
        table->setText(QStringLiteral("Grid &Thing"));
        QTRY_VERIFY(entry(QStringLiteral("Grid &Thing")));
        table->setCheckable(true);
        table->setChecked(true);
        QTRY_VERIFY(entry(QStringLiteral("Grid &Thing")) && entry(QStringLiteral("Grid &Thing"))->isChecked());
        plain->setEnabled(false);
        QVERIFY(!entry(plain->text())->isEnabled());
        // it does what the command does
        int ran = 0;
        QObject::connect(table, &QAction::triggered, &w, [&] { ++ran; });
        entry(QStringLiteral("Grid &Thing"))->trigger();
        QVERIFY2(ran == 1 || entry(QStringLiteral("Grid &Thing"))->menu(), "the entry does not run the command");
    }

    void aQuickToolbarOfNoMoreThanItsRoomLaysOutOnceWhenSet()
    {
        RibbonWindow w;
        Ribbon *r = w.ribbon();
        QList<Ribbon::QuickEntry> entries;
        for (int i = 0; i < 100; ++i) entries.append({w.addCommand(QStringLiteral("test.lay%1").arg(i), QStringLiteral("L %1").arg(i)), nullptr, false, QString()});
        const int before = r->quickAccessLayoutCount();
        r->setQuickAccess(entries);
        QCOMPARE(r->quickAccessLayoutCount() - before, 1);
        QCOMPARE(int(r->quickAccessButtons().size()), 100);
    }

    void aButtonsWidthIsMeasuredWithoutItsMnemonic()
    {
        RibbonWindow w;
        w.addCommand(QStringLiteral("test.mnem"), QStringLiteral("&Zoomer"), false);
        RibbonCustom c;
        c.quickSet = true;
        c.quick = {QStringLiteral("test.mnem")};
        c.save();
        RibbonCustom::applyAll();
        QToolButton *b = w.ribbon()->quickAccessButtons().value(0);
        QVERIFY(b);
        QCOMPARE(b->width(), b->fontMetrics().horizontalAdvance(QStringLiteral("Zoomer")) + 14);
    }

    void anActionHiddenAfterItsButtonWasMadeHidesTheButton()
    {
        RibbonCustom c;
        c.addQuickCommand(QStringLiteral("format.bold"), kQuickDefaults);
        c.save();
        RibbonWindow w;
        Ribbon *r = w.ribbon();
        QToolButton *b = r->quickAccessButtons().value(1);
        QVERIFY(b);
        QVERIFY(!b->isHidden());
        w.act(QStringLiteral("format.bold"))->setVisible(false);
        QTRY_VERIFY(b->isHidden());
        QVERIFY2(r->quickAccessHidden().isEmpty(), "a command that is hidden is not one that did not fit");
        w.act(QStringLiteral("format.bold"))->setVisible(true);
        QTRY_VERIFY(!b->isHidden());
    }

    void aDefaultCommandThatIsGoneIsLeftOutOfTheReset()
    {
        RibbonCustom c;
        c.addQuickCommand(QStringLiteral("format.bold"), kQuickDefaults);
        c.save();
        RibbonWindow w;
        w.dropCommand(QStringLiteral("file.save"));
        QCOMPARE(int(w.ribbon()->defaultQuickAccess().size()), 0);
        clearSaved();
        RibbonCustom::applyAll();   // (back to the ribbon file's: of which nothing is left)
        QCOMPARE(int(w.ribbon()->quickAccessButtons().size()), 0);
    }

    void theOverflowMenuIsMadeAgainWhenItClosesAfterAChangeWhileOpen()
    {
        RibbonWindow w;
        overflowing(w, 40);
        Ribbon *r = w.ribbon();
        QTRY_VERIFY(!r->quickAccessHidden().isEmpty());
        QToolButton *more = r->quickAccessMore();
        more->menu()->popup(w.mapToGlobal(QPoint(10, 10)));
        QTRY_VERIFY(more->menu()->isVisible());
        const int before = int(more->menu()->actions().size());
        RibbonCustom c;
        c.quickSet = true;
        for (int i = 0; i < 30; ++i) c.quick << QStringLiteral("test.many%1").arg(i);
        c.save();
        RibbonCustom::applyAll();
        more->menu()->close();
        QTRY_COMPARE(int(more->menu()->actions().size()), int(r->quickAccessHidden().size()));
        QVERIFY(int(more->menu()->actions().size()) != before);
    }

    void theQuickToolbarPage()
    {
        RibbonWindow w;
        Ribbon *r = w.ribbon();
        QuickToolbarPage page(&w);
        auto *toolbar = page.findChild<QListWidget *>(QStringLiteral("quickList"));
        auto *search = page.findChild<QLineEdit *>(QStringLiteral("quickSearch"));
        auto *list = page.findChild<QListWidget *>(QStringLiteral("quickCommands"));
        QVERIFY(toolbar && search && list);
        auto button = [&](const char *object) { return page.findChild<QPushButton *>(QString::fromLatin1(object)); };
        QCOMPARE(toolbar->count(), 1);
        QCOMPARE(int(list->count()), int(KeyMap(&w).commands().size()));
        QVERIFY(list->count() > 0);
        QVERIFY(!button("quickResetAll")->isEnabled());
        // the command list searches
        search->setText(QStringLiteral("zoom"));
        int shown = 0;
        for (int i = 0; i < list->count(); ++i) shown += !list->item(i)->isHidden();
        QCOMPARE(shown, 1);
        for (int i = 0; i < list->count(); ++i)
            if (!list->item(i)->isHidden()) list->setCurrentRow(i);
        button("quickAdd")->click();
        QCOMPARE(page.plan().quick, (QStringList{QStringLiteral("file.save"), QStringLiteral("view.zoom")}));
        QCOMPARE(toolbar->count(), 2);
        button("quickSeparator")->click();
        QCOMPARE(toolbar->count(), 3);
        QVERIFY(button("quickResetAll")->isEnabled());
        toolbar->setCurrentRow(1);
        button("quickUp")->click();
        QCOMPARE(page.plan().quick.value(0), QStringLiteral("view.zoom"));
        button("quickDown")->click();
        QCOMPARE(page.plan().quick.value(1), QStringLiteral("view.zoom"));
        toolbar->setCurrentRow(2);
        button("quickRemove")->click();
        QCOMPARE(int(page.plan().quick.size()), 2);
        // nothing has changed in the window until OK
        QCOMPARE(quickIds(r), kQuickDefaults);
        page.apply();
        QCOMPARE(quickIds(r), (QStringList{QStringLiteral("file.save"), QStringLiteral("view.zoom")}));
        QCOMPARE(RibbonCustom::load().quick, page.plan().quick);
        // Reset One on a command of the file's, then Reset All
        toolbar->setCurrentRow(0);
        button("quickRemove")->click();
        toolbar->setCurrentRow(-1);
        QVERIFY(button("quickResetOne")->isEnabled() == false);   // (nothing chosen)
        search->clear();
        for (int i = 0; i < list->count(); ++i)
            if (list->item(i)->data(Qt::UserRole).toString() == QLatin1String("file.save")) list->setCurrentRow(i);
        QVERIFY(button("quickResetOne")->isEnabled());
        button("quickResetOne")->click();
        QCOMPARE(page.plan().quick.value(0), QStringLiteral("file.save"));
        button("quickResetAll")->click();
        QVERIFY(page.plan().isEmpty());
        page.apply();
        QCOMPARE(quickIds(r), kQuickDefaults);
    }

    void theQuickToolbarPageLeavesTheRibbonPagesWorkAlone()
    {
        for (int order = 0; order < 2; ++order) {
            clearSaved();
            RibbonCustom c;
            c.setTabHidden(QStringLiteral("Insert"), true);
            c.save();
            RibbonWindow w;
            QuickToolbarPage quickPage(&w);
            RibbonPage ribbonPage(&w);
            ribbonPage.plan().setTabHidden(QStringLiteral("View"), true);
            quickPage.plan().addQuickCommand(QStringLiteral("format.bold"), kQuickDefaults);
            if (order == 0) {
                quickPage.apply();
                ribbonPage.apply();
            } else {
                ribbonPage.apply();
                quickPage.apply();
            }
            const RibbonCustom saved = RibbonCustom::load();
            QVERIFY2(saved.hiddenTabs.contains(QStringLiteral("View")) && saved.hiddenTabs.contains(QStringLiteral("Insert")), qPrintable(QString::number(order)));
            QCOMPARE(saved.quick, (QStringList{QStringLiteral("file.save"), QStringLiteral("format.bold")}));
        }
        // Reset All on the Ribbon page keeps the quick toolbar, and the other way round
        clearSaved();
        RibbonCustom c;
        c.setTabHidden(QStringLiteral("Insert"), true);
        c.addQuickCommand(QStringLiteral("format.bold"), kQuickDefaults);
        c.save();
        RibbonWindow w;
        {
            RibbonPage ribbonPage(&w);
            ribbonPage.findChild<QPushButton *>(QStringLiteral("ribbonResetAll"))->click();
            ribbonPage.apply();
            const RibbonCustom saved = RibbonCustom::load();
            QVERIFY(saved.hiddenTabs.isEmpty());
            QCOMPARE(saved.quick, (QStringList{QStringLiteral("file.save"), QStringLiteral("format.bold")}));
        }
        c.setTabHidden(QStringLiteral("Insert"), true);
        c.save();
        {
            QuickToolbarPage quickPage(&w);
            quickPage.findChild<QPushButton *>(QStringLiteral("quickResetAll"))->click();
            quickPage.apply();
            const RibbonCustom saved = RibbonCustom::load();
            QVERIFY(!saved.quickSet);
            QVERIFY(saved.hiddenTabs.contains(QStringLiteral("Insert")));
        }
    }

    void everyTabHasItsOwnPlace()
    {
        RibbonWindow w;
        Ribbon *r = w.ribbon();
        r->setContextVisible(QStringLiteral("Chart Tools"), true);
        auto unique = [&] {
            QSet<int> homes, bases;
            for (int i = 0; i < r->tabCount(); ++i) {
                QVERIFY2(!homes.contains(r->tabHome(i)), qPrintable(QStringLiteral("home %1 of %2").arg(r->tabHome(i)).arg(r->tabName(i))));
                QVERIFY2(!bases.contains(r->tabBase(i)), qPrintable(QStringLiteral("base %1 of %2").arg(r->tabBase(i)).arg(r->tabName(i))));
                homes.insert(r->tabHome(i));
                bases.insert(r->tabBase(i));
            }
        };
        RibbonCustom c;
        QStringList ids;
        for (int round = 0; round < 6; ++round) {
            ids << c.addTab(QStringLiteral("T%1").arg(round));
            if (round % 2) c.remove(ids[round - 1]);
            c.moveTab(QStringLiteral("Insert"), round % 2 ? 1 : -1, kBuiltin);
            c.save();
            RibbonCustom::applyAll();
            unique();
            if (QTest::currentTestFailed()) return;
        }
        c.resetAll();
        c.save();
        RibbonCustom::applyAll();
        unique();
        QCOMPARE(r->tabCount(), 4);
    }

    void applyingLeavesOutGroupsOnTabsThereAreNone()
    {
        RibbonCustom c;
        c.addGroup(QStringLiteral("Home"), QStringLiteral("Mine"));
        c.addGroup(QStringLiteral("Mystery"), QStringLiteral("On a tab that is not there"));
        c.save();
        RibbonWindow w;
        const RibbonCustom applied = RibbonCustom::appliedTo(&w);
        QCOMPARE(applied.groups.size(), 1);
        QCOMPARE(applied.groups.first().tab, QStringLiteral("Home"));
        // (what is saved is left as it is: the page drops it when the person goes on)
        QVERIFY(Settings::get().value(QStringLiteral("ribbon/custom")).toString().contains(QLatin1String("Mystery")));
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
        QCOMPARE(list->count(), 9);
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
