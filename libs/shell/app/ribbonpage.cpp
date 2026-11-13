#include "app/ribbonpage.h"

#include "app/appwindow.h"
#include "app/keymap.h"
#include "app/plaintext.h"
#include "app/ribbon.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <algorithm>

namespace jp {

namespace {

enum Role { IdRole = Qt::UserRole, KindRole, TabRole, IndexRole, OwnRole };

} // namespace

RibbonPage::RibbonPage(AppWindow *win, QWidget *parent) : QWidget(parent), m_win(win), m_plan(RibbonCustom::load())
{
    setObjectName(QStringLiteral("ribbonPage"));
    // What the ribbon has, as the ribbon file made it.
    if (Ribbon *r = win->ribbon()) {
        r->buildAllTabs();
        QList<int> at;
        for (int i = 0; i < r->tabCount(); ++i)
            if (!r->tabName(i).startsWith(QLatin1String("user:"))) at << i;
        std::stable_sort(at.begin(), at.end(), [r](int a, int b) { return r->tabBase(a) < r->tabBase(b); });
        for (int i : std::as_const(at)) {
            BuiltTab t;
            t.name = r->tabName(i);
            t.title = r->tabTitle(i);
            t.contextual = !r->tabContextGroup(i).isEmpty();
            if (RibbonTab *page = r->tabPage(i))
                for (RibbonGroup *g : page->groups())
                    if (!g->property("jpUserGroup").toBool()) t.groups.append({g->id(), g->title()});
            m_built << t;
        }
    }
    QStringList known;
    for (const BuiltTab &t : std::as_const(m_built)) known << t.name;
    m_plan.dropUnknownTabs(known);   // (what another version's tab left is not kept)
    m_plan.resetQuick();   // (the quick toolbar is the Quick Toolbar page's)
    const KeyMap keys(win);
    for (const KeyCommand &c : keys.commands())
        if (!c.keyOnly) m_commands.append({c.id, c.text, c.group});

    auto *col = new QVBoxLayout(this);
    auto *about = plainLabel(new QLabel(tr("Customize the ribbon: clear the box of a tab or group to hide it, make your own tabs and groups, and put any command in them. Home and the File button always stay. Only what differs from the usual ribbon is kept."), this));
    about->setWordWrap(true);
    col->addWidget(about);

    auto *body = new QHBoxLayout;
    auto *left = new QVBoxLayout;
    left->addWidget(plainLabel(new QLabel(tr("Tabs and groups:"), this)));
    m_tree = new QTreeWidget(this);
    m_tree->setObjectName(QStringLiteral("ribbonTree"));
    m_tree->setHeaderHidden(true);
    m_tree->setUniformRowHeights(true);
    m_tree->setMinimumHeight(170);
    m_tree->setAccessibleName(tr("Ribbon tabs and groups"));
    left->addWidget(m_tree, 1);
    body->addLayout(left, 3);

    auto *right = new QVBoxLayout;
    right->addWidget(plainLabel(new QLabel(tr("Commands:"), this)));
    m_search = new QLineEdit(this);
    m_search->setObjectName(QStringLiteral("ribbonSearch"));
    m_search->setPlaceholderText(tr("Search commands"));
    m_search->setClearButtonEnabled(true);
    m_search->setAccessibleName(tr("Search commands"));
    right->addWidget(m_search);
    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("ribbonCommands"));
    m_list->setAccessibleName(tr("Commands to add"));
    right->addWidget(m_list, 1);
    m_addCommand = new QPushButton(tr("Add Command"), this);
    m_addCommand->setObjectName(QStringLiteral("ribbonAddCommand"));
    m_addCommand->setToolTip(tr("Puts the chosen command in the group you chose."));
    right->addWidget(m_addCommand);
    body->addLayout(right, 2);
    col->addLayout(body, 1);

    auto *nameRow = new QHBoxLayout;
    nameRow->addWidget(plainLabel(new QLabel(tr("Name:"), this)));
    m_name = new QLineEdit(this);
    m_name->setObjectName(QStringLiteral("ribbonName"));
    m_name->setAccessibleName(tr("Name of the tab, group, or command"));
    m_name->setMaxLength(RibbonCustom::kMaxName);
    nameRow->addWidget(m_name, 1);
    col->addLayout(nameRow);

    auto make = [this](const QString &object, const QString &text, const QString &tip) {
        auto *b = new QPushButton(text, this);
        b->setObjectName(object);
        // plain: reviewed: this page's own strings
        b->setToolTip(tip);
        return b;
    };
    auto *buttons = new QHBoxLayout;
    m_newTab = make(QStringLiteral("ribbonNewTab"), tr("New Tab"), tr("Makes a tab of your own, with the name above."));
    m_newGroup = make(QStringLiteral("ribbonNewGroup"), tr("New Group"), tr("Makes a group of your own on the chosen tab, with the name above."));
    m_rename = make(QStringLiteral("ribbonRename"), tr("Rename"), tr("Gives the chosen tab, group, or command of your own the name above."));
    m_up = make(QStringLiteral("ribbonUp"), tr("Move Up"), tr("Moves the chosen tab, group, or command up."));
    m_down = make(QStringLiteral("ribbonDown"), tr("Move Down"), tr("Moves the chosen tab, group, or command down."));
    m_remove = make(QStringLiteral("ribbonRemove"), tr("Remove"), tr("Takes away the chosen tab, group, or command of your own."));
    for (QPushButton *b : {m_newTab, m_newGroup, m_rename, m_up, m_down, m_remove}) buttons->addWidget(b);
    buttons->addStretch(1);
    col->addLayout(buttons);

    m_message = plainLabel(new QLabel(this));
    m_message->setObjectName(QStringLiteral("ribbonMessage"));
    m_message->setWordWrap(true);
    m_message->setAccessibleName(tr("Ribbon status"));
    col->addWidget(m_message);

    auto *resets = new QHBoxLayout;
    m_resetOne = make(QStringLiteral("ribbonResetOne"), tr("Reset One"), tr("Puts back the chosen tab as it usually is: shown, with its groups shown. A tab of your own is taken away."));
    m_resetAll = make(QStringLiteral("ribbonResetAll"), tr("Reset All"), tr("Puts back the whole ribbon as it usually is."));
    resets->addWidget(m_resetOne);
    resets->addWidget(m_resetAll);
    resets->addStretch(1);
    col->addLayout(resets);

    fillTree();
    fillCommands();
    connect(m_search, &QLineEdit::textChanged, this, &RibbonPage::filter);
    connect(m_tree, &QTreeWidget::currentItemChanged, this, [this] {
        const Chosen c = chosen();
        if (c.own && c.kind == QLatin1String("cmd")) {
            if (const RibbonCustom::Group *g = m_plan.group(c.id)) m_name->setText(g->items.value(c.index).name);
        } else if (c.own) {
            const QString id = c.id;
            if (const RibbonCustom::Tab *t = m_plan.tab(id)) m_name->setText(t->name);
            else if (const RibbonCustom::Group *g = m_plan.group(id)) m_name->setText(g->name);
        }
        say(QString());
        showState();
    });
    connect(m_list, &QListWidget::currentItemChanged, this, [this] { showState(); });
    connect(m_name, &QLineEdit::textChanged, this, [this] { showState(); });
    connect(m_tree, &QTreeWidget::itemChanged, this, [this](QTreeWidgetItem *item, int) {
        if (m_filling) return;
        const QString kind = item->data(0, KindRole).toString(), id = item->data(0, IdRole).toString();
        const bool hidden = item->checkState(0) != Qt::Checked;
        if (kind == QLatin1String("tab")) {
            m_plan.setTabHidden(id, hidden);
        } else if (kind == QLatin1String("group")) {
            const QString tab = item->data(0, TabRole).toString();
            m_plan.setGroupHidden(tab, id.mid(tab.size() + 1), hidden);
        }
        showState();   // (the row stays as it is: it is the one that was clicked)
    });
    connect(m_newTab, &QPushButton::clicked, this, [this] {
        const QString id = m_plan.addTab(nameText(tr("New Tab")));
        if (id.isEmpty()) return say(tr("There is room for %1 tabs of your own.").arg(RibbonCustom::kMaxTabs));
        changed();
        choose({QStringLiteral("tab"), id, id, -1, true});
        say(QString());
    });
    connect(m_newGroup, &QPushButton::clicked, this, [this] {
        const Chosen c = chosen();
        if (c.tab.isEmpty()) return;
        const QString id = m_plan.addGroup(c.tab, nameText(tr("New Group")));
        if (id.isEmpty()) return say(tr("There is room for %1 groups on a tab.").arg(RibbonCustom::kMaxGroups));
        changed();
        choose({QStringLiteral("group"), id, c.tab, -1, true});
        say(QString());
    });
    connect(m_addCommand, &QPushButton::clicked, this, [this] {
        const Chosen c = chosen();
        QListWidgetItem *cmd = m_list->currentItem();
        if (!c.own || c.kind == QLatin1String("tab") || !cmd) return say(tr("Choose a group of your own, and a command to put in it."));
        if (!m_plan.addCommand(c.id, cmd->data(IdRole).toString())) return say(tr("This group has that command already, or has no room for another (%1 at most).").arg(RibbonCustom::kMaxItems));
        changed();
        const RibbonCustom::Group *g = m_plan.group(c.id);
        choose({QStringLiteral("cmd"), c.id, c.tab, g ? int(g->items.size()) - 1 : 0, true});
        say(QString());
    });
    connect(m_rename, &QPushButton::clicked, this, [this] {
        const Chosen c = chosen();
        if (!c.own) return;
        const bool ok = c.kind == QLatin1String("cmd") ? m_plan.renameCommand(c.id, c.index, m_name->text()) : m_plan.rename(c.id, m_name->text());
        if (!ok) return say(tr("Type a name first."));
        changed();
        choose(c);
        say(QString());
    });
    auto move = [this](int delta) {
        const Chosen c = chosen();
        if (c.kind == QLatin1String("tab")) {
            if (!m_plan.moveTab(c.id, delta, builtinOrder())) return;
        } else if (c.kind == QLatin1String("group") && c.own) {
            if (!m_plan.moveGroup(c.id, delta)) return;
        } else if (c.kind == QLatin1String("cmd") && c.own) {
            if (!m_plan.moveCommand(c.id, c.index, delta)) return;
        } else {
            return;
        }
        Chosen next = c;
        if (c.kind == QLatin1String("cmd")) next.index += delta;
        changed();
        choose(next);
    };
    connect(m_up, &QPushButton::clicked, this, [move] { move(-1); });
    connect(m_down, &QPushButton::clicked, this, [move] { move(1); });
    connect(m_remove, &QPushButton::clicked, this, [this] {
        const Chosen c = chosen();
        if (!c.own) return;
        const bool ok = c.kind == QLatin1String("cmd") ? m_plan.removeCommand(c.id, c.index) : m_plan.remove(c.id);
        if (!ok) return;
        changed();
        Chosen next;
        if (c.kind == QLatin1String("cmd")) next = {QStringLiteral("group"), c.id, c.tab, -1, true};
        else if (c.kind == QLatin1String("group")) next = {QStringLiteral("tab"), c.tab, c.tab, -1, c.tab.startsWith(QLatin1String("user:"))};
        choose(next);
    });
    connect(m_resetOne, &QPushButton::clicked, this, [this] {
        const Chosen c = chosen();
        if (c.tab.isEmpty()) return;
        m_plan.resetTab(c.tab);
        m_plan.normalizeOrder(builtinOrder());
        changed();
        say(QString());
    });
    connect(m_resetAll, &QPushButton::clicked, this, [this] {
        m_plan.resetAll();
        changed();
        say(QString());
    });
    showState();
}

QString RibbonPage::nameText(const QString &fallback) const
{
    const QString n = m_name->text().simplified();
    return n.isEmpty() ? fallback : n;
}

QStringList RibbonPage::builtinOrder() const
{
    QStringList out;
    for (const BuiltTab &t : m_built)
        if (!t.contextual) out << t.name;
    return out;
}

const RibbonPage::BuiltTab *RibbonPage::builtTab(const QString &name) const
{
    for (const BuiltTab &t : m_built)
        if (t.name == name) return &t;
    return nullptr;
}

void RibbonPage::fillTree()
{
    m_filling = true;
    m_tree->clear();
    auto bold = [](QTreeWidgetItem *item) {
        QFont f = item->font(0);
        f.setBold(true);
        item->setFont(0, f);
    };
    auto addGroups = [&](QTreeWidgetItem *parent, const QString &tab, const BuiltTab *built) {
        if (built)
            for (const BuiltGroup &g : built->groups) {
                auto *item = new QTreeWidgetItem(parent, QStringList{g.title});
                item->setData(0, IdRole, tab + QLatin1Char('/') + g.id);
                item->setData(0, KindRole, QStringLiteral("group"));
                item->setData(0, TabRole, tab);
                item->setData(0, OwnRole, false);
                item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
                item->setCheckState(0, m_plan.hiddenGroups.contains(tab + QLatin1Char('/') + g.id) ? Qt::Unchecked : Qt::Checked);
            }
        for (const RibbonCustom::Group *g : m_plan.groupsOf(tab)) {
            auto *item = new QTreeWidgetItem(parent, QStringList{g->name});
            item->setData(0, IdRole, g->id);
            item->setData(0, KindRole, QStringLiteral("group"));
            item->setData(0, TabRole, tab);
            item->setData(0, OwnRole, true);
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            item->setToolTip(0, tr("A group of your own"));
            bold(item);
            for (int i = 0; i < g->items.size(); ++i) {
                QString text = g->items[i].cmd;
                for (const Command &c : std::as_const(m_commands))
                    if (c.id == g->items[i].cmd) text = c.text;
                auto *leaf = new QTreeWidgetItem(item, QStringList{g->items[i].name.isEmpty() ? text : g->items[i].name});
                leaf->setData(0, IdRole, g->id);
                leaf->setData(0, KindRole, QStringLiteral("cmd"));
                leaf->setData(0, TabRole, tab);
                leaf->setData(0, IndexRole, i);
                leaf->setData(0, OwnRole, true);
                leaf->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                leaf->setToolTip(0, plainToolTip(text));
            }
            item->setExpanded(true);
        }
    };
    const QStringList order = m_plan.effectiveOrder(builtinOrder());
    for (const QString &id : order) {
        const bool own = id.startsWith(QLatin1String("user:"));
        const BuiltTab *built = own ? nullptr : builtTab(id);
        const RibbonCustom::Tab *mine = own ? m_plan.tab(id) : nullptr;
        if (!built && !mine) continue;
        auto *item = new QTreeWidgetItem(m_tree, QStringList{own ? mine->name : built->title});
        item->setData(0, IdRole, id);
        item->setData(0, KindRole, QStringLiteral("tab"));
        item->setData(0, TabRole, id);
        item->setData(0, OwnRole, own);
        bold(item);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | (own || id == QLatin1String("Home") ? Qt::ItemFlags() : Qt::ItemIsUserCheckable));
        if (!own && id != QLatin1String("Home")) item->setCheckState(0, m_plan.hiddenTabs.contains(id) ? Qt::Unchecked : Qt::Checked);
        addGroups(item, id, built);
        item->setExpanded(true);
    }
    // The tabs that come and go with what is selected.
    for (const BuiltTab &t : m_built) {
        if (!t.contextual) continue;
        auto *item = new QTreeWidgetItem(m_tree, QStringList{tr("%1 (appears when needed)").arg(t.title)});
        item->setData(0, IdRole, t.name);
        item->setData(0, KindRole, QStringLiteral("tab"));
        item->setData(0, TabRole, t.name);
        item->setData(0, OwnRole, false);
        bold(item);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        addGroups(item, t.name, &t);
        item->setExpanded(false);
    }
    m_filling = false;
}

void RibbonPage::fillCommands()
{
    m_list->clear();
    for (const Command &c : std::as_const(m_commands)) {
        auto *item = new QListWidgetItem(tr("%1 (%2)").arg(c.text, c.group), m_list);
        item->setData(IdRole, c.id);
        item->setData(Qt::UserRole + 1, c.text);
        item->setToolTip(plainToolTip(c.text));
    }
    filter(m_search->text());
}

void RibbonPage::filter(const QString &text)
{
    const QStringList words = text.simplified().toLower().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    for (int i = 0; i < m_list->count(); ++i) {
        QListWidgetItem *item = m_list->item(i);
        const QString hay = item->text().toLower();
        bool all = true;
        for (const QString &w : words) all = all && hay.contains(w);
        item->setHidden(!all);
    }
}

RibbonPage::Chosen RibbonPage::chosen() const
{
    Chosen c;
    const QTreeWidgetItem *item = m_tree->currentItem();
    if (!item) return c;
    c.kind = item->data(0, KindRole).toString();
    c.id = item->data(0, IdRole).toString();
    c.tab = item->data(0, TabRole).toString();
    c.index = item->data(0, IndexRole).isValid() ? item->data(0, IndexRole).toInt() : -1;
    c.own = item->data(0, OwnRole).toBool();
    return c;
}

void RibbonPage::choose(const Chosen &c)
{
    QTreeWidgetItemIterator it(m_tree);
    for (; *it; ++it) {
        const QTreeWidgetItem *item = *it;
        if (item->data(0, KindRole).toString() != c.kind || item->data(0, IdRole).toString() != c.id) continue;
        if (c.kind == QLatin1String("cmd") && item->data(0, IndexRole).toInt() != c.index) continue;
        m_tree->setCurrentItem(*it);
        return;
    }
    m_tree->setCurrentItem(nullptr);
    showState();
}

void RibbonPage::changed()
{
    const Chosen c = chosen();
    fillTree();
    choose(c);
    showState();
}

void RibbonPage::showState()
{
    const Chosen c = chosen();
    const bool tab = c.kind == QLatin1String("tab"), group = c.kind == QLatin1String("group"), cmd = c.kind == QLatin1String("cmd");
    const QString tabName = c.tab;
    bool tabChanged = false;
    if (!tabName.isEmpty()) {
        const QString prefix = tabName + QLatin1Char('/');
        // (a tab that is not where the ribbon file has it has changed too)
        QStringList base = builtinOrder();
        for (const RibbonCustom::Tab &t : m_plan.tabs) base << t.id;
        const QStringList now = m_plan.effectiveOrder(builtinOrder());
        tabChanged = (base.contains(tabName) && base.indexOf(tabName) != now.indexOf(tabName)) || m_plan.hiddenTabs.contains(tabName) || tabName.startsWith(QLatin1String("user:")) || !m_plan.groupsOf(tabName).isEmpty();
        for (const QString &k : m_plan.hiddenGroups) tabChanged = tabChanged || k.startsWith(prefix);
    }
    m_newTab->setEnabled(m_plan.tabs.size() < RibbonCustom::kMaxTabs);
    m_newGroup->setEnabled(!c.tab.isEmpty() && m_plan.groupsOf(c.tab).size() < RibbonCustom::kMaxGroups);
    m_addCommand->setEnabled(c.own && (group || cmd) && m_list->currentItem());
    m_rename->setEnabled(c.own && (!m_name->text().simplified().isEmpty() || cmd));
    m_up->setEnabled(tab || (c.own && (group || cmd)));
    m_down->setEnabled(tab || (c.own && (group || cmd)));
    m_remove->setEnabled(c.own);
    m_resetOne->setEnabled(tabChanged);
    m_resetAll->setEnabled(!m_plan.isEmpty());
}

void RibbonPage::say(const QString &text)
{
    m_message->setText(text);
}

void RibbonPage::apply()
{
    RibbonCustom out = m_plan;
    const RibbonCustom saved = RibbonCustom::load();
    out.quickSet = saved.quickSet;
    out.quick = saved.quick;
    out.quickBelow = saved.quickBelow;
    out.save();
    RibbonCustom::applyAll();
}

} // namespace jp
