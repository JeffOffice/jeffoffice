#include "app/finddialog.h"

#include "sheet/cellref.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTabBar>
#include <QTreeWidget>
#include <QVBoxLayout>
#include "app/plaintext.h"

namespace jp {

namespace {
constexpr int kSheetRole = Qt::UserRole, kRowRole = Qt::UserRole + 1, kColRole = Qt::UserRole + 2;
}

FindDialog::FindDialog(QWidget *parent) : QDialog(parent)
{
    setObjectName(QStringLiteral("findDialog"));
    setWindowTitle(tr("Look For and Change"));
    setModal(false);
    auto *layout = new QVBoxLayout(this);
    m_tabs = new QTabBar;
    m_tabs->setObjectName(QStringLiteral("findTabs"));
    m_tabs->addTab(tr("Find"));
    m_tabs->addTab(tr("Change"));
    m_tabs->setExpanding(false);
    m_tabs->setAccessibleName(tr("Look for or change"));
    layout->addWidget(m_tabs);
    auto *form = new QFormLayout;
    m_find = new QLineEdit;
    m_find->setObjectName(QStringLiteral("findText"));
    m_find->setAccessibleName(tr("Look for"));
    m_find->setMaxLength(255);
    m_find->setClearButtonEnabled(true);
    form->addRow(tr("Look for:"), m_find);
    m_replace = new QLineEdit;
    m_replace->setObjectName(QStringLiteral("replaceText"));
    m_replace->setAccessibleName(tr("Change to"));
    m_replaceLabel = new QLabel(tr("Change to:"));
    m_replaceLabel->setBuddy(m_replace);
    form->addRow(m_replaceLabel, m_replace);
    layout->addLayout(form);
    auto *grid = new QGridLayout;
    m_within = new QComboBox;
    m_within->setObjectName(QStringLiteral("findWithin"));
    m_within->setAccessibleName(tr("Search one sheet or the whole workbook"));
    m_within->addItems({tr("Sheet"), tr("Workbook")});
    m_search = new QComboBox;
    m_search->setObjectName(QStringLiteral("findOrder"));
    m_search->setAccessibleName(tr("Order the cells are visited in: along rows or down columns"));
    m_search->addItems({tr("Along rows"), tr("Down columns")});
    m_lookIn = new QComboBox;
    m_lookIn->setObjectName(QStringLiteral("findLookIn"));
    m_lookIn->setAccessibleName(tr("What to search: formulas, values, or notes"));
    m_lookIn->addItems({tr("Formulas"), tr("Values"), tr("Notes")});
    auto addPair = [&](int row, int col, const QString &text, QWidget *w) {
        auto *label = plainLabel(new QLabel(text));
        label->setBuddy(w);
        grid->addWidget(label, row, col);
        grid->addWidget(w, row, col + 1);
    };
    addPair(0, 0, tr("Where:"), m_within);
    addPair(0, 2, tr("Order:"), m_search);
    addPair(1, 0, tr("Search in:"), m_lookIn);
    m_case = new QCheckBox(tr("Same capitals only"));
    m_case->setObjectName(QStringLiteral("findCase"));
    m_whole = new QCheckBox(tr("Whole cell only"));
    m_whole->setObjectName(QStringLiteral("findWhole"));
    grid->addWidget(m_case, 1, 2, 1, 2);
    grid->addWidget(m_whole, 2, 0, 1, 4);
    layout->addLayout(grid);
    auto *hint = new QLabel(tr("In the text to look for, ? stands for any one character and * for any stretch of characters. Type ~ in front of ? or * to look for that mark itself."));
    hint->setWordWrap(true);
    layout->addWidget(hint);
    auto *buttons = new QHBoxLayout;
    m_findAll = new QPushButton(tr("List All"));
    m_findAll->setObjectName(QStringLiteral("findAll"));
    m_findNext = new QPushButton(tr("Next"));
    m_findNext->setObjectName(QStringLiteral("findNext"));
    m_findNext->setDefault(true);
    m_replaceOne = new QPushButton(tr("Change"));
    m_replaceOne->setObjectName(QStringLiteral("replaceOne"));
    m_replaceAll = new QPushButton(tr("Change All"));
    m_replaceAll->setObjectName(QStringLiteral("replaceAll"));
    m_close = new QPushButton(tr("Close"));
    m_close->setObjectName(QStringLiteral("findClose"));
    buttons->addWidget(m_replaceAll);
    buttons->addWidget(m_replaceOne);
    buttons->addWidget(m_findAll);
    buttons->addWidget(m_findNext);
    buttons->addStretch(1);
    buttons->addWidget(m_close);
    layout->addLayout(buttons);
    m_results = new QTreeWidget;
    m_results->setObjectName(QStringLiteral("findResults"));
    m_results->setAccessibleName(tr("Matching cells"));
    m_results->setRootIsDecorated(false);
    m_results->setUniformRowHeights(true);
    m_results->setColumnCount(3);
    m_results->setHeaderLabels({tr("Sheet"), tr("Cell"), tr("Contents")});
    m_results->header()->setStretchLastSection(true);
    m_results->setMinimumHeight(120);
    m_results->hide();
    layout->addWidget(m_results, 1);
    m_status = plainLabel(new QLabel);
    m_status->setObjectName(QStringLiteral("findStatus"));
    m_status->setAccessibleName(tr("Outcome of the search"));
    layout->addWidget(m_status);

    connect(m_tabs, &QTabBar::currentChanged, this, [this] { updateTab(); });
    connect(m_findNext, &QPushButton::clicked, this, &FindDialog::findNextRequested);
    connect(m_findAll, &QPushButton::clicked, this, &FindDialog::findAllRequested);
    connect(m_replaceOne, &QPushButton::clicked, this, &FindDialog::replaceRequested);
    connect(m_replaceAll, &QPushButton::clicked, this, &FindDialog::replaceAllRequested);
    connect(m_close, &QPushButton::clicked, this, [this] {
        if (m_busy) Q_EMIT cancelRequested();
        else close();
    });
    connect(m_find, &QLineEdit::returnPressed, this, [this] {
        if (!m_busy) Q_EMIT findNextRequested();
    });
    connect(m_results, &QTreeWidget::currentItemChanged, this, [this](QTreeWidgetItem *item) {
        if (item) Q_EMIT resultChosen(item->data(0, kSheetRole).toInt(), item->data(0, kRowRole).toUInt(), item->data(0, kColRole).toUInt());
    });
    // (Notes can be looked in but a replace works on what the cells hold)
    connect(m_lookIn, &QComboBox::currentIndexChanged, this, [this] { updateTab(); });
    updateTab();
}

void FindDialog::updateTab()
{
    const bool replace = tab() == Tab::Replace;
    m_replace->setVisible(replace);
    m_replaceLabel->setVisible(replace);
    m_replaceOne->setVisible(replace);
    m_replaceAll->setVisible(replace);
    m_replaceOne->setEnabled(!m_busy);
    m_replaceAll->setEnabled(!m_busy);
    // a replace changes what the cells hold (their formulas and entries), whatever is looked in
    if (replace && m_lookIn->currentIndex() != 0) {
        const QSignalBlocker block(m_lookIn);
        m_lookIn->setCurrentIndex(0);
    }
    m_lookIn->setEnabled(!replace);
}

void FindDialog::setTab(Tab tab) { m_tabs->setCurrentIndex(tab == Tab::Replace ? 1 : 0); }
FindDialog::Tab FindDialog::tab() const { return m_tabs->currentIndex() == 1 ? Tab::Replace : Tab::Find; }
void FindDialog::setFindText(const QString &text) { m_find->setText(text); }
QString FindDialog::findText() const { return m_find->text(); }
QString FindDialog::replacement() const { return m_replace->text(); }

crunch::FindOptions FindDialog::options() const
{
    crunch::FindOptions o;
    o.pattern.text = m_find->text();
    o.pattern.matchCase = m_case->isChecked();
    o.pattern.wholeCell = m_whole->isChecked();
    o.scope = m_within->currentIndex() == 1 ? crunch::FindOptions::Scope::Workbook : crunch::FindOptions::Scope::Sheet;
    o.order = m_search->currentIndex() == 1 ? crunch::FindOptions::Order::ByColumns : crunch::FindOptions::Order::ByRows;
    o.lookIn = m_lookIn->currentIndex() == 2 ? crunch::FindOptions::LookIn::Notes : m_lookIn->currentIndex() == 1 ? crunch::FindOptions::LookIn::Values : crunch::FindOptions::LookIn::Formulas;
    return o;
}

void FindDialog::setOptions(const crunch::FindOptions &o)
{
    m_case->setChecked(o.pattern.matchCase);
    m_whole->setChecked(o.pattern.wholeCell);
    m_within->setCurrentIndex(o.scope == crunch::FindOptions::Scope::Workbook ? 1 : 0);
    m_search->setCurrentIndex(o.order == crunch::FindOptions::Order::ByColumns ? 1 : 0);
    m_lookIn->setCurrentIndex(o.lookIn == crunch::FindOptions::LookIn::Notes ? 2 : o.lookIn == crunch::FindOptions::LookIn::Values ? 1 : 0);
}

void FindDialog::setStatus(const QString &text) { m_status->setText(text); }

void FindDialog::clearResults()
{
    m_results->clear();
    m_results->hide();
}

void FindDialog::showResults(const std::vector<crunch::FoundCell> &hits, const QStringList &sheetNames, bool truncated)
{
    const QSignalBlocker block(m_results);
    m_results->clear();
    // (a long list is made of at most this many rows; the status says how many were found)
    const size_t shown = std::min<size_t>(hits.size(), 5000);
    QList<QTreeWidgetItem *> items;
    items.reserve(qsizetype(shown));
    for (size_t i = 0; i < shown; ++i) {
        const crunch::FoundCell &h = hits[i];
        auto *item = new QTreeWidgetItem({h.sheet >= 0 && h.sheet < sheetNames.size() ? sheetNames[h.sheet] : QString(), jp::office::sheet::cellName(h.row, h.col), h.text});
        item->setData(0, kSheetRole, h.sheet);
        item->setData(0, kRowRole, h.row);
        item->setData(0, kColRole, h.col);
        items << item;
    }
    m_results->addTopLevelItems(items);
    m_results->setVisible(!hits.empty());
    const int count = int(std::min<size_t>(hits.size(), 1000000));
    QString text = hits.empty() ? tr("No cell matches.") : tr("%n cell(s) match.", nullptr, count);
    if (truncated) text += QLatin1Char(' ') + tr("More cells match than the list can hold. Narrow the search.");
    else if (hits.size() > shown) text += QLatin1Char(' ') + tr("Only the first %1 are listed.").arg(shown);
    m_status->setText(text);
    if (!hits.empty()) {
        m_results->header()->resizeSection(0, 120);
        m_results->header()->resizeSection(1, 70);
    }
}

void FindDialog::setBusy(bool busy)
{
    m_busy = busy;
    m_findAll->setEnabled(!busy);
    m_findNext->setEnabled(!busy);
    m_close->setText(busy ? tr("Cancel") : tr("Close"));
    updateTab();
}

void FindDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape && m_busy) {
        Q_EMIT cancelRequested();
        return;
    }
    QDialog::keyPressEvent(e);
}

} // namespace jp
