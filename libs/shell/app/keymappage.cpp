#include "app/keymappage.h"

#include "app/appwindow.h"
#include "app/plaintext.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeySequenceEdit>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace jp {

namespace {

QString keysText(const QList<QKeySequence> &keys)
{
    QStringList s;
    for (const QKeySequence &k : keys) s << k.toString(QKeySequence::NativeText);
    return s.join(QStringLiteral(", "));
}

} // namespace

KeyboardPage::KeyboardPage(AppWindow *win, QWidget *parent) : QWidget(parent), m_map(new KeyMap(win))
{
    setObjectName(QStringLiteral("keyboardPage"));
    auto *col = new QVBoxLayout(this);
    auto *about = plainLabel(new QLabel(tr("Customize the keyboard: choose a command, press the keys you want for it, and choose Assign. Only the keys that differ from the usual ones are kept."), this));
    about->setWordWrap(true);
    col->addWidget(about);

    m_search = new QLineEdit(this);
    m_search->setObjectName(QStringLiteral("keySearch"));
    m_search->setPlaceholderText(tr("Search commands or keys"));
    m_search->setClearButtonEnabled(true);
    m_search->setAccessibleName(tr("Search commands or keys"));
    col->addWidget(m_search);

    m_tree = new QTreeWidget(this);
    m_tree->setObjectName(QStringLiteral("keyCommands"));
    m_tree->setHeaderLabels({tr("Command"), tr("Keys")});
    m_tree->setRootIsDecorated(true);
    m_tree->setUniformRowHeights(true);
    m_tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_tree->setAccessibleName(tr("Commands and their keys"));
    m_tree->setMinimumHeight(180);
    col->addWidget(m_tree, 1);

    auto *row = new QHBoxLayout;
    auto *keysCol = new QVBoxLayout;
    keysCol->addWidget(plainLabel(new QLabel(tr("Keys for this command:"), this)));
    m_keys = new QListWidget(this);
    m_keys->setObjectName(QStringLiteral("keyCurrent"));
    m_keys->setMaximumHeight(72);
    m_keys->setAccessibleName(tr("Keys for this command"));
    keysCol->addWidget(m_keys);
    m_remove = new QPushButton(tr("Remove"), this);
    m_remove->setObjectName(QStringLiteral("keyRemove"));
    keysCol->addWidget(m_remove);
    row->addLayout(keysCol, 1);

    auto *pressCol = new QVBoxLayout;
    pressCol->addWidget(plainLabel(new QLabel(tr("Press the new keys:"), this)));
    m_press = new QKeySequenceEdit(this);
    m_press->setObjectName(QStringLiteral("keyPress"));
    m_press->setMaximumSequenceLength(1);
    m_press->setAccessibleName(tr("Press the new keys"));
    pressCol->addWidget(m_press);
    auto *buttons = new QHBoxLayout;
    m_assign = new QPushButton(tr("Assign"), this);
    m_assign->setObjectName(QStringLiteral("keyAssign"));
    m_takeOver = new QPushButton(tr("Take Over"), this);
    m_takeOver->setObjectName(QStringLiteral("keyTakeOver"));
    buttons->addWidget(m_assign);
    buttons->addWidget(m_takeOver);
    buttons->addStretch(1);
    pressCol->addLayout(buttons);
    pressCol->addStretch(1);
    row->addLayout(pressCol, 1);
    col->addLayout(row);

    m_message = plainLabel(new QLabel(this));
    m_message->setObjectName(QStringLiteral("keyMessage"));
    m_message->setWordWrap(true);
    m_message->setAccessibleName(tr("Key status"));
    col->addWidget(m_message);

    auto *resets = new QHBoxLayout;
    m_resetOne = new QPushButton(tr("Reset One"), this);
    m_resetOne->setObjectName(QStringLiteral("keyResetOne"));
    m_resetOne->setToolTip(tr("Puts back the usual keys of the chosen command."));
    m_resetAll = new QPushButton(tr("Reset All"), this);
    m_resetAll->setObjectName(QStringLiteral("keyResetAll"));
    m_resetAll->setToolTip(tr("Puts back the usual keys of every command."));
    resets->addWidget(m_resetOne);
    resets->addWidget(m_resetAll);
    resets->addStretch(1);
    col->addLayout(resets);

    auto *reserved = plainLabel(new QLabel(tr("Reserved keys, which the system and typing need and which cannot be changed: %1").arg(KeyMap::reservedList().join(QStringLiteral(", "))), this));
    reserved->setWordWrap(true);
    col->addWidget(reserved);

    fillTree();
    connect(m_search, &QLineEdit::textChanged, this, &KeyboardPage::filter);
    connect(m_tree, &QTreeWidget::currentItemChanged, this, [this] {
        m_press->clear();
        say(QString());
        showCurrent();
    });
    connect(m_press, &QKeySequenceEdit::keySequenceChanged, this, &KeyboardPage::pressed);
    connect(m_assign, &QPushButton::clicked, this, [this] { assign(false); });
    connect(m_takeOver, &QPushButton::clicked, this, [this] { assign(true); });
    connect(m_remove, &QPushButton::clicked, this, [this] {
        const QString id = currentId();
        QListWidgetItem *item = m_keys->currentItem();
        if (id.isEmpty() || !item) return;
        const QKeySequence key = item->data(Qt::UserRole).value<QKeySequence>();
        if (!m_map->remove(id, key)) say(tr("%1 is reserved, and stays with this command.").arg(key.toString(QKeySequence::NativeText)));
        showRow(id);
        showCurrent();
    });
    connect(m_resetOne, &QPushButton::clicked, this, [this] {
        const QString id = currentId();
        if (id.isEmpty()) return;
        m_map->resetOne(id);
        for (const KeyCommand &c : m_map->commands()) showRow(c.id);   // (a command that had lost a key to this one gets it back)
        showCurrent();
        say(QString());
    });
    connect(m_resetAll, &QPushButton::clicked, this, [this] {
        m_map->resetAll();
        for (const KeyCommand &c : m_map->commands()) showRow(c.id);
        showCurrent();
        say(QString());
    });
    showCurrent();
}

void KeyboardPage::fillTree()
{
    QHash<QString, QTreeWidgetItem *> groups;
    for (const KeyCommand &c : m_map->commands()) {
        QTreeWidgetItem *&g = groups[c.group];
        if (!g) {
            g = new QTreeWidgetItem(m_tree, QStringList{c.group});
            g->setFlags(Qt::ItemIsEnabled);
            QFont f = g->font(0);
            f.setBold(true);
            g->setFont(0, f);
            g->setExpanded(true);
        }
        auto *item = new QTreeWidgetItem(g, QStringList{c.text, keysText(c.keys)});
        item->setData(0, Qt::UserRole, c.id);
        item->setToolTip(0, c.text);
    }
}

void KeyboardPage::filter(const QString &text)
{
    const QStringList words = text.simplified().toLower().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *g = m_tree->topLevelItem(i);
        int shown = 0;
        for (int j = 0; j < g->childCount(); ++j) {
            QTreeWidgetItem *c = g->child(j);
            const QString hay = (c->text(0) + QLatin1Char(' ') + c->text(1) + QLatin1Char(' ') + g->text(0)).toLower();
            bool all = true;
            for (const QString &w : words) all = all && hay.contains(w);
            c->setHidden(!all);
            shown += all;
        }
        g->setHidden(shown == 0);
        if (shown && !words.isEmpty()) g->setExpanded(true);
    }
}

QString KeyboardPage::currentId() const
{
    const QTreeWidgetItem *item = m_tree->currentItem();
    return item ? item->data(0, Qt::UserRole).toString() : QString();
}

void KeyboardPage::showRow(const QString &id)
{
    const KeyCommand *c = m_map->command(id);
    if (!c) return;
    for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *g = m_tree->topLevelItem(i);
        for (int j = 0; j < g->childCount(); ++j)
            if (g->child(j)->data(0, Qt::UserRole).toString() == id) {
                g->child(j)->setText(1, keysText(c->keys));
                QFont f = g->child(j)->font(1);
                f.setBold(c->changed());
                g->child(j)->setFont(1, f);
                return;
            }
    }
}

void KeyboardPage::showCurrent()
{
    m_keys->clear();
    const KeyCommand *c = m_map->command(currentId());
    if (c)
        for (const QKeySequence &k : c->keys) {
            auto *item = new QListWidgetItem(k.toString(QKeySequence::NativeText), m_keys);
            item->setData(Qt::UserRole, QVariant::fromValue(k));
        }
    if (m_keys->count()) m_keys->setCurrentRow(0);
    m_remove->setEnabled(c && m_keys->count());
    m_resetOne->setEnabled(c && c->changed());
    m_resetAll->setEnabled(m_map->anyChanged());
    m_press->setEnabled(c);
    m_assign->setEnabled(false);
    m_takeOver->hide();
}

void KeyboardPage::say(const QString &text)
{
    m_message->setText(text);
}

void KeyboardPage::pressed()
{
    m_takeOver->hide();
    m_assign->setEnabled(false);
    const QKeySequence key = m_press->keySequence();
    const QString id = currentId();
    if (key.isEmpty() || id.isEmpty()) {
        say(QString());
        return;
    }
    const QString name = key.toString(QKeySequence::NativeText);
    const KeyMap::Result r = m_map->check(id, key);
    switch (r.status) {
    case KeyMap::Add::Added:
        say(tr("%1 is not used by another command.").arg(name));
        m_assign->setEnabled(true);
        break;
    case KeyMap::Add::Already:
        say(tr("%1 is already a key of this command.").arg(name));
        break;
    case KeyMap::Add::Reserved:
        say(tr("%1 is reserved (the system or typing needs it) and cannot be assigned.").arg(name));
        break;
    case KeyMap::Add::Taken:
        say(tr("%1 is used by \"%2\". Choose Take Over to give it to this command instead.").arg(name, r.ownerText));
        m_takeOver->show();
        break;
    case KeyMap::Add::Invalid:
        say(tr("This command cannot have more keys, or that is not a key it can have. Remove one first."));
        break;
    }
}

void KeyboardPage::assign(bool takeOver)
{
    const QString id = currentId();
    const QKeySequence key = m_press->keySequence();
    if (id.isEmpty() || key.isEmpty()) return;
    const KeyMap::Result r = m_map->add(id, key, takeOver);
    if (r.status != KeyMap::Add::Added) return;
    showRow(id);
    if (!r.owner.isEmpty()) showRow(r.owner);
    for (const KeyCommand &c : m_map->commands()) showRow(c.id);
    m_press->clear();
    showCurrent();
    say(tr("%1 is now a key of \"%2\".").arg(key.toString(QKeySequence::NativeText), m_map->command(id)->text));
}

void KeyboardPage::apply()
{
    m_map->save();
    m_map->applyAll();
}

} // namespace jp
