#include "app/uistyle.h"
#include "app/autocorrectdialog.h"

#include "app/autocorrectconfig.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QLocale>
#include <QPushButton>
#include <QTabWidget>
#include <QTableWidget>
#include <functional>
#include <QVBoxLayout>

namespace jp {

namespace {

// A list of words to keep, with a box to type one and the buttons to add it and to take the selected one away.
QListWidget *exceptionList(QWidget *tab, QVBoxLayout *layout, const QString &name, const QString &label, QLineEdit **edit, QObject *owner, const std::function<void()> &changed)
{
    auto *title = new QLabel(label, tab);
    title->setTextFormat(Qt::PlainText);
    layout->addWidget(title);
    auto *row = new QHBoxLayout();
    *edit = new QLineEdit(tab);
    (*edit)->setObjectName(name + QStringLiteral("Edit"));
    auto *add = new QPushButton(QCoreApplication::translate("AutoCorrectDialog", "Add"), tab);
    add->setObjectName(name + QStringLiteral("Add"));
    auto *del = new QPushButton(QCoreApplication::translate("AutoCorrectDialog", "Delete"), tab);
    del->setObjectName(name + QStringLiteral("Delete"));
    row->addWidget(*edit, 1);
    row->addWidget(add);
    row->addWidget(del);
    layout->addLayout(row);
    auto *list = new QListWidget(tab);
    list->setObjectName(name);
    layout->addWidget(list, 1);
    title->setBuddy(*edit);
    QLineEdit *box = *edit;
    QObject::connect(add, &QPushButton::clicked, owner, [list, box, changed] {
        const QString word = box->text().trimmed();
        if (word.isEmpty()) return;
        if (list->findItems(word, Qt::MatchExactly).isEmpty()) list->addItem(word);
        box->clear();
        changed();
    });
    QObject::connect(box, &QLineEdit::returnPressed, add, &QPushButton::click);
    QObject::connect(del, &QPushButton::clicked, owner, [list, changed] {
        qDeleteAll(list->selectedItems());
        changed();
    });
    return list;
}

} // namespace

AutoCorrectPage::AutoCorrectPage(const QString &language, QWidget *parent) : QWidget(parent)
{
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    auto *top = new QHBoxLayout();
    auto *languageLabel = new QLabel(QCoreApplication::translate("AutoCorrectDialog", "Language"), this);
    m_languages = new QComboBox(this);
    m_languages->setObjectName(QStringLiteral("autoCorrectLanguage"));
    languageLabel->setBuddy(m_languages);
    top->addWidget(languageLabel);
    top->addWidget(m_languages, 1);
    outer->addLayout(top);
    QStringList codes = {QStringLiteral("en"), QStringLiteral("fr"), QStringLiteral("de"), QStringLiteral("es"), QStringLiteral("it"), QStringLiteral("pt"), QStringLiteral("nl")};
    const QString base = autotext::baseLanguage(language);
    if (!codes.contains(base)) codes << base;
    for (const QString &code : codes) m_languages->addItem(QLocale(code).nativeLanguageName(), code);
    auto *tabs = new QTabWidget(this);
    auto *replaceTab = new QWidget(tabs);
    auto *rl = new QVBoxLayout(replaceTab);
    auto box = [&](const char *text, const char *name) {
        auto *b = new QCheckBox(QCoreApplication::translate("AutoCorrectDialog", text), replaceTab);
        b->setObjectName(QString::fromLatin1(name));
        rl->addWidget(b);
        return b;
    };
    m_sentences = box(QT_TRANSLATE_NOOP("AutoCorrectDialog", "Capitalize first letter of sentences"), "capSentences");
    m_cells = box(QT_TRANSLATE_NOOP("AutoCorrectDialog", "Capitalize first letter of table cells"), "capCells");
    m_days = box(QT_TRANSLATE_NOOP("AutoCorrectDialog", "Capitalize names of days"), "capDays");
    m_twoInitials = box(QT_TRANSLATE_NOOP("AutoCorrectDialog", "Correct TWo INitial CApitals"), "capTwoInitials");
    m_capsLock = box(QT_TRANSLATE_NOOP("AutoCorrectDialog", "Correct accidental usage of cAPS LOCK key"), "capCapsLock");
    m_replace = box(QT_TRANSLATE_NOOP("AutoCorrectDialog", "Replace text as you type"), "replaceText");
    auto *fields = new QHBoxLayout();
    m_from = new QLineEdit(replaceTab);
    m_from->setObjectName(QStringLiteral("replaceFrom"));
    m_from->setPlaceholderText(QCoreApplication::translate("AutoCorrectDialog", "Replace"));
    m_with = new QLineEdit(replaceTab);
    m_with->setObjectName(QStringLiteral("replaceWith"));
    m_with->setPlaceholderText(QCoreApplication::translate("AutoCorrectDialog", "With"));
    auto *add = new QPushButton(QCoreApplication::translate("AutoCorrectDialog", "Add"), replaceTab);
    add->setObjectName(QStringLiteral("addReplacement"));
    add->setToolTip(QCoreApplication::translate("AutoCorrectDialog", "Adds the entry, or changes the \"With\" text of the entry that has the same \"Replace\" text."));
    auto *del = new QPushButton(QCoreApplication::translate("AutoCorrectDialog", "Delete"), replaceTab);
    del->setObjectName(QStringLiteral("deleteReplacement"));
    fields->addWidget(m_from, 1);
    fields->addWidget(m_with, 1);
    fields->addWidget(add);
    fields->addWidget(del);
    rl->addLayout(fields);
    m_table = new QTableWidget(0, 2, replaceTab);
    m_table->setObjectName(QStringLiteral("replacements"));
    m_table->setHorizontalHeaderLabels({QCoreApplication::translate("AutoCorrectDialog", "Replace"), QCoreApplication::translate("AutoCorrectDialog", "With")});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    rl->addWidget(m_table, 1);
    auto *restore = new QPushButton(QCoreApplication::translate("AutoCorrectDialog", "Restore Defaults"), replaceTab);
    restore->setObjectName(QStringLiteral("restoreDefaults"));
    rl->addWidget(restore, 0, Qt::AlignLeft);
    tabs->addTab(replaceTab, QCoreApplication::translate("AutoCorrectDialog", "AutoCorrect"));
    auto *exceptionTab = new QWidget(tabs);
    auto *el = new QVBoxLayout(exceptionTab);
    m_exceptions = exceptionList(exceptionTab, el, QStringLiteral("exceptionList"), QCoreApplication::translate("AutoCorrectDialog", "Do not capitalize after these abbreviations:"), &m_exceptionEdit, this, [] {});
    m_initials = exceptionList(exceptionTab, el, QStringLiteral("initialList"), QCoreApplication::translate("AutoCorrectDialog", "Do not correct these words with two initial capitals:"), &m_initialEdit, this, [] {});
    tabs->addTab(exceptionTab, QCoreApplication::translate("AutoCorrectDialog", "Exceptions"));
    outer->addWidget(tabs, 1);
    connect(add, &QPushButton::clicked, this, &AutoCorrectPage::addReplacement);
    connect(m_with, &QLineEdit::returnPressed, add, &QPushButton::click);
    connect(del, &QPushButton::clicked, this, &AutoCorrectPage::deleteReplacement);
    connect(restore, &QPushButton::clicked, this, &AutoCorrectPage::restoreDefaults);
    connect(m_table, &QTableWidget::currentCellChanged, this, [this](int row) {
        if (m_filling || row < 0) return;
        m_from->setText(m_table->item(row, 0)->text());
        m_with->setText(m_table->item(row, 1)->text());
    });
    connect(m_languages, &QComboBox::currentIndexChanged, this, [this] {
        if (m_filling) return;
        store();
        showLanguage(m_languages->currentData().toString());
    });
    m_filling = true;
    m_languages->setCurrentIndex(m_languages->findData(base));
    m_filling = false;
    showLanguage(base);
}

void AutoCorrectPage::showLanguage(const QString &language)
{
    m_language = autotext::baseLanguage(language);
    if (!m_configs.contains(m_language)) m_configs.insert(m_language, autoCorrectConfig(m_language));
    const autotext::Config &c = m_configs[m_language];
    // (the switches are the same for every language: they are set once)
    if (!m_switchesSet) {
        m_replace->setChecked(c.replace);
        m_sentences->setChecked(c.sentences);
        m_cells->setChecked(c.cells);
        m_days->setChecked(c.days);
        m_twoInitials->setChecked(c.twoInitials);
        m_capsLock->setChecked(c.capsLock);
        m_switchesSet = true;
    }
    fillReplacements();
    fillExceptions();
}

void AutoCorrectPage::fillReplacements()
{
    m_filling = true;
    const autotext::Config &c = m_configs[m_language];
    m_table->setRowCount(0);
    for (const auto &entry : c.replacements) {
        const int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setItem(row, 0, new QTableWidgetItem(entry.first));
        m_table->setItem(row, 1, new QTableWidgetItem(entry.second));
    }
    m_from->clear();
    m_with->clear();
    m_filling = false;
}

void AutoCorrectPage::fillExceptions()
{
    const autotext::Config &c = m_configs[m_language];
    m_exceptions->clear();
    m_exceptions->addItems(c.exceptions);
    m_initials->clear();
    m_initials->addItems(c.initialExceptions);
}

// What the widgets show goes into the configuration of the language shown.
void AutoCorrectPage::store()
{
    autotext::Config &c = m_configs[m_language];
    c.language = m_language;
    c.replace = m_replace->isChecked();
    c.sentences = m_sentences->isChecked();
    c.cells = m_cells->isChecked();
    c.days = m_days->isChecked();
    c.twoInitials = m_twoInitials->isChecked();
    c.capsLock = m_capsLock->isChecked();
    c.replacements.clear();
    for (int row = 0; row < m_table->rowCount(); ++row) c.replacements.append({m_table->item(row, 0)->text(), m_table->item(row, 1)->text()});
    c.exceptions.clear();
    for (int i = 0; i < m_exceptions->count(); ++i) c.exceptions << m_exceptions->item(i)->text();
    c.initialExceptions.clear();
    for (int i = 0; i < m_initials->count(); ++i) c.initialExceptions << m_initials->item(i)->text();
}

autotext::Config AutoCorrectPage::config()
{
    store();
    return m_configs[m_language];
}

void AutoCorrectPage::addReplacement()
{
    const QString from = m_from->text();
    if (from.trimmed().isEmpty()) return;
    int row = -1;
    for (int i = 0; i < m_table->rowCount() && row < 0; ++i)
        if (m_table->item(i, 0)->text() == from) row = i;
    if (row < 0) {
        row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setItem(row, 0, new QTableWidgetItem(from));
        m_table->setItem(row, 1, new QTableWidgetItem());
    }
    m_table->item(row, 1)->setText(m_with->text());
    m_filling = true;
    m_table->setCurrentCell(row, 0);
    m_filling = false;
    m_table->scrollToItem(m_table->item(row, 0));
}

void AutoCorrectPage::deleteReplacement()
{
    const int row = m_table->currentRow();
    if (row < 0) return;
    m_filling = true;
    m_table->removeRow(row);
    m_filling = false;
    m_from->clear();
    m_with->clear();
}

// The program's own lists take the place of this language's.
void AutoCorrectPage::restoreDefaults()
{
    autotext::Config &c = m_configs[m_language];
    c.replacements = autotext::defaultReplacements(m_language);
    c.exceptions = autotext::defaultExceptions(m_language);
    c.initialExceptions = autotext::defaultInitialExceptions(m_language);
    fillReplacements();
    fillExceptions();
}

void AutoCorrectPage::apply()
{
    store();
    const autotext::Config shown = m_configs[m_language];
    for (autotext::Config &c : m_configs) {
        c.replace = shown.replace;
        c.sentences = shown.sentences;
        c.cells = shown.cells;
        c.days = shown.days;
        c.twoInitials = shown.twoInitials;
        c.capsLock = shown.capsLock;
        saveAutoCorrectConfig(c);
    }
}

bool autoCorrectDialog(QWidget *parent, const QString &language)
{
    QDialog d(parent);
    d.setObjectName(QStringLiteral("autoCorrectDialog"));
    d.setWindowTitle(QCoreApplication::translate("AutoCorrectDialog", "AutoCorrect Options"));
    auto *v = new QVBoxLayout(&d);
    auto *page = new AutoCorrectPage(language, &d);
    page->setObjectName(QStringLiteral("autoCorrectPage"));
    v->addWidget(page, 1);
    auto *bb = new jp::ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &d);
    v->addWidget(bb);
    QObject::connect(bb, &QDialogButtonBox::accepted, &d, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &d, &QDialog::reject);
    d.setMinimumSize(520, 520);
    if (d.exec() != QDialog::Accepted) return false;
    page->apply();
    return true;
}

} // namespace jp
