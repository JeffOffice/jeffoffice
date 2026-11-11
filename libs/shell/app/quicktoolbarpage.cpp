#include "app/quicktoolbarpage.h"

#include "app/appwindow.h"
#include "app/keymap.h"
#include "app/plaintext.h"
#include "app/ribbon.h"

#include <QAction>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

namespace jp {

namespace {

enum Role { IdRole = Qt::UserRole };

} // namespace

QuickToolbarPage::QuickToolbarPage(AppWindow *win, QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("quickToolbarPage"));
    const RibbonCustom saved = RibbonCustom::load();
    m_plan.quickSet = saved.quickSet;
    m_plan.quick = saved.quick;
    if (Ribbon *r = win->ribbon())
        for (const Ribbon::QuickEntry &e : r->defaultQuickAccess())
            if (e.action) {
                m_defaults << e.action->objectName();
                m_locked = m_locked || e.action->objectName().isEmpty();   // (a command with no id cannot be kept in the list: nothing is changed that would lose it)
            }
    const KeyMap keys(win);
    for (const KeyCommand &c : keys.commands()) m_commands.append({c.id, c.text, c.group});

    auto *col = new QVBoxLayout(this);
    auto *about = plainLabel(new QLabel(tr("Customize the quick toolbar, the small toolbar at the top left of the window: put any command on it, add separators, and change the order. Only what differs from the usual quick toolbar is kept."), this));
    about->setWordWrap(true);
    col->addWidget(about);

    auto *body = new QHBoxLayout;
    auto *left = new QVBoxLayout;
    left->addWidget(plainLabel(new QLabel(tr("Quick toolbar:"), this)));
    m_toolbar = new QListWidget(this);
    m_toolbar->setObjectName(QStringLiteral("quickList"));
    m_toolbar->setMinimumHeight(170);
    m_toolbar->setAccessibleName(tr("Quick toolbar commands"));
    left->addWidget(m_toolbar, 1);
    body->addLayout(left, 3);

    auto *right = new QVBoxLayout;
    right->addWidget(plainLabel(new QLabel(tr("Commands:"), this)));
    m_search = new QLineEdit(this);
    m_search->setObjectName(QStringLiteral("quickSearch"));
    m_search->setPlaceholderText(tr("Search commands"));
    m_search->setClearButtonEnabled(true);
    m_search->setAccessibleName(tr("Search commands"));
    right->addWidget(m_search);
    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("quickCommands"));
    m_list->setAccessibleName(tr("Commands to add"));
    right->addWidget(m_list, 1);
    m_add = new QPushButton(tr("Add Command"), this);
    m_add->setObjectName(QStringLiteral("quickAdd"));
    m_add->setToolTip(tr("Puts the chosen command at the end of the quick toolbar."));
    right->addWidget(m_add);
    body->addLayout(right, 2);
    col->addLayout(body, 1);

    auto make = [this](const QString &object, const QString &text, const QString &tip) {
        auto *b = new QPushButton(text, this);
        b->setObjectName(object);
        // plain: reviewed: this page's own strings
        b->setToolTip(tip);
        return b;
    };
    auto *buttons = new QHBoxLayout;
    m_separator = make(QStringLiteral("quickSeparator"), tr("Add Separator"), tr("Puts a separator at the end of the quick toolbar."));
    m_up = make(QStringLiteral("quickUp"), tr("Move Up"), tr("Moves the chosen command up."));
    m_down = make(QStringLiteral("quickDown"), tr("Move Down"), tr("Moves the chosen command down."));
    m_remove = make(QStringLiteral("quickRemove"), tr("Remove"), tr("Takes the chosen command or separator off the quick toolbar."));
    for (QPushButton *b : {m_separator, m_up, m_down, m_remove}) buttons->addWidget(b);
    buttons->addStretch(1);
    col->addLayout(buttons);

    m_message = plainLabel(new QLabel(this));
    m_message->setObjectName(QStringLiteral("quickMessage"));
    m_message->setWordWrap(true);
    m_message->setAccessibleName(tr("Quick toolbar status"));
    col->addWidget(m_message);

    auto *resets = new QHBoxLayout;
    m_resetOne = make(QStringLiteral("quickResetOne"), tr("Reset One"), tr("Puts the chosen command back as the quick toolbar usually has it: where it is usually, or off it when it usually is not there."));
    m_resetAll = make(QStringLiteral("quickResetAll"), tr("Reset All"), tr("Puts back the whole quick toolbar as it usually is."));
    resets->addWidget(m_resetOne);
    resets->addWidget(m_resetAll);
    resets->addStretch(1);
    col->addLayout(resets);

    fillToolbar(-1);
    fillCommands();
    connect(m_search, &QLineEdit::textChanged, this, &QuickToolbarPage::filter);
    connect(m_toolbar, &QListWidget::currentItemChanged, this, [this] {
        if (m_filling) return;
        m_fromCommands = false;
        say(QString());
        showState();
    });
    connect(m_list, &QListWidget::currentItemChanged, this, [this] {
        m_fromCommands = true;
        showState();
    });
    connect(m_add, &QPushButton::clicked, this, [this] {
        QListWidgetItem *cmd = m_list->currentItem();
        if (!cmd) return say(tr("Choose a command to put on the quick toolbar."));
        if (!m_plan.addQuickCommand(cmd->data(IdRole).toString(), m_defaults)) return say(tr("The quick toolbar has that command already, or has no room for another (%1 at most).").arg(RibbonCustom::kMaxQuick));
        fillToolbar(int(m_plan.effectiveQuick(m_defaults).size()) - 1);
        say(QString());
    });
    connect(m_separator, &QPushButton::clicked, this, [this] {
        if (!m_plan.addQuickSeparator(m_defaults)) return say(tr("The quick toolbar has no room for another (%1 at most).").arg(RibbonCustom::kMaxQuick));
        fillToolbar(int(m_plan.effectiveQuick(m_defaults).size()) - 1);
        say(QString());
    });
    auto move = [this](int delta) {
        const int row = m_toolbar->currentRow();
        if (!m_plan.moveQuick(row, delta, m_defaults)) return;
        fillToolbar(row + delta);
    };
    connect(m_up, &QPushButton::clicked, this, [move] { move(-1); });
    connect(m_down, &QPushButton::clicked, this, [move] { move(1); });
    connect(m_remove, &QPushButton::clicked, this, [this] {
        const int row = m_toolbar->currentRow();
        if (!m_plan.removeQuick(row, m_defaults)) return;
        fillToolbar(std::min(row, int(m_plan.effectiveQuick(m_defaults).size()) - 1));
        say(QString());
    });
    connect(m_resetOne, &QPushButton::clicked, this, [this] {
        const int row = m_toolbar->currentRow();
        if (!m_plan.resetQuickCommand(resetTarget(), m_defaults)) return;
        fillToolbar(row);
        say(QString());
    });
    connect(m_resetAll, &QPushButton::clicked, this, [this] {
        m_plan.resetQuick();
        fillToolbar(-1);
        say(QString());
    });
    showState();
}

QString QuickToolbarPage::commandText(const QString &id) const
{
    for (const Command &c : m_commands)
        if (c.id == id) return c.text;
    return QString();
}

QString QuickToolbarPage::resetTarget() const
{
    if (m_fromCommands) return m_list->currentItem() ? m_list->currentItem()->data(IdRole).toString() : QString();
    return m_toolbar->currentItem() ? m_toolbar->currentItem()->data(IdRole).toString() : QString();
}

void QuickToolbarPage::fillToolbar(int select)
{
    m_filling = true;
    m_toolbar->clear();
    for (const QString &id : m_plan.effectiveQuick(m_defaults)) {
        QString text = id.isEmpty() ? tr("Separator") : commandText(id);
        if (text.isEmpty()) text = tr("%1 (not available in this version)").arg(id);   // (kept in the list: it comes back where the command is)
        auto *item = new QListWidgetItem(id.isEmpty() ? QStringLiteral("--- %1 ---").arg(text) : text, m_toolbar);
        item->setData(IdRole, id);
        item->setToolTip(plainToolTip(text));
    }
    m_filling = false;
    m_toolbar->setCurrentRow(select);
    showState();
}

void QuickToolbarPage::fillCommands()
{
    m_list->clear();
    for (const Command &c : std::as_const(m_commands)) {
        auto *item = new QListWidgetItem(tr("%1 (%2)").arg(c.text, c.group), m_list);
        item->setData(IdRole, c.id);
        item->setToolTip(plainToolTip(c.text));
    }
    filter(m_search->text());
}

void QuickToolbarPage::filter(const QString &text)
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

void QuickToolbarPage::showState()
{
    const int row = m_toolbar->currentRow(), count = m_toolbar->count();
    if (m_locked) {
        for (QPushButton *b : {m_add, m_separator, m_up, m_down, m_remove, m_resetOne, m_resetAll}) b->setEnabled(false);
        say(tr("This quick toolbar has a command that cannot be kept in a list, so it cannot be changed here."));
        return;
    }
    m_add->setEnabled(m_list->currentItem() != nullptr);
    m_separator->setEnabled(count < RibbonCustom::kMaxQuick);
    m_up->setEnabled(row > 0);
    m_down->setEnabled(row >= 0 && row < count - 1);
    m_remove->setEnabled(row >= 0);
    const QString target = resetTarget();
    m_resetOne->setEnabled(!target.isEmpty() && m_plan.quickCommandChanged(target, m_defaults));
    m_resetAll->setEnabled(!m_plan.isEmpty());
}

void QuickToolbarPage::say(const QString &text)
{
    m_message->setText(text);
}

void QuickToolbarPage::apply()
{
    RibbonCustom out = RibbonCustom::load();   // (what the Ribbon page keeps stays as it is)
    out.quickSet = m_plan.quickSet;
    out.quick = m_plan.quick;
    out.save();
    RibbonCustom::applyAll();
}

} // namespace jp
