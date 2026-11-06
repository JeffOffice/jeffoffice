#include "app/scenariodialogs.h"

#include "core/scenarios.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHash>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include <algorithm>

namespace jp {

ScenarioManagerDialog::ScenarioManagerDialog(QWidget *parent) : QDialog(parent)
{
    setObjectName(QStringLiteral("scenarioManagerDialog"));
    setWindowTitle(tr("Scenario Manager"));
    auto *layout = new QVBoxLayout(this);
    auto *intro = new QLabel(tr("A scenario is a named set of values for some cells. Keep several, and show the one you want to see."), this);
    intro->setWordWrap(true);
    layout->addWidget(intro);
    auto *row = new QHBoxLayout;
    auto *left = new QVBoxLayout;
    auto *label = new QLabel(tr("&Scenarios:"), this);
    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("scenarioList"));
    m_list->setAccessibleName(tr("Scenarios"));
    label->setBuddy(m_list);
    left->addWidget(label);
    left->addWidget(m_list, 1);
    row->addLayout(left, 1);
    auto *buttons = new QVBoxLayout;
    auto button = [&](const QString &text, const char *name) {
        auto *b = new QPushButton(text, this);
        b->setObjectName(QString::fromLatin1(name));
        buttons->addWidget(b);
        return b;
    };
    auto *add = button(tr("&Add…"), "scenarioAdd");
    m_delete = button(tr("&Delete"), "scenarioDelete");
    m_edit = button(tr("&Edit…"), "scenarioEdit");
    m_show = button(tr("S&how"), "scenarioShow");
    m_summary = button(tr("S&ummary…"), "scenarioSummary");
    buttons->addStretch(1);
    row->addLayout(buttons);
    layout->addLayout(row);
    m_about = new QLabel(this);
    m_about->setObjectName(QStringLiteral("scenarioAbout"));
    m_about->setWordWrap(true);
    m_about->setTextFormat(Qt::PlainText);
    layout->addWidget(m_about);
    auto *box = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(box);
    connect(add, &QPushButton::clicked, this, &ScenarioManagerDialog::addRequested);
    connect(m_edit, &QPushButton::clicked, this, [this] { Q_EMIT editRequested(currentIndex()); });
    connect(m_delete, &QPushButton::clicked, this, [this] { Q_EMIT deleteRequested(currentIndex()); });
    connect(m_show, &QPushButton::clicked, this, [this] { Q_EMIT showRequested(currentIndex()); });
    connect(m_summary, &QPushButton::clicked, this, &ScenarioManagerDialog::summaryRequested);
    connect(m_list, &QListWidget::currentRowChanged, this, [this] { refreshButtons(); });
    connect(m_list, &QListWidget::itemDoubleClicked, this, [this] { Q_EMIT showRequested(currentIndex()); });
    refreshButtons();
}

int ScenarioManagerDialog::currentIndex() const { return m_list->currentRow(); }

void ScenarioManagerDialog::setScenarios(const jp::office::sheet::ScenarioList &list)
{
    const int keep = m_list->currentRow();
    m_scenarios = list;
    m_list->clear();
    for (const jp::office::sheet::Scenario &s : list.list) m_list->addItem(s.name);
    if (m_list->count() > 0) m_list->setCurrentRow(std::max(0, std::min(keep, m_list->count() - 1)));
    refreshButtons();
}

void ScenarioManagerDialog::refreshButtons()
{
    const int i = m_list->currentRow();
    const bool any = i >= 0 && i < m_scenarios.list.size();
    m_edit->setEnabled(any);
    m_delete->setEnabled(any);
    m_show->setEnabled(any);
    m_summary->setEnabled(!m_scenarios.list.isEmpty());
    if (!any) {
        m_about->setText(m_scenarios.list.isEmpty() ? tr("No scenarios are set. Choose Add to make one.") : QString());
        return;
    }
    const jp::office::sheet::Scenario &s = m_scenarios.list[i];
    QStringList cells;
    for (const jp::office::sheet::ScenarioCell &c : s.cells) cells << c.ref;
    QString text = tr("Changing cells: %1").arg(cells.join(QStringLiteral(", ")));
    if (!s.comment.isEmpty()) text += QLatin1Char('\n') + tr("Comment: %1").arg(s.comment);
    m_about->setText(text);
}

ScenarioEditDialog::ScenarioEditDialog(bool adding, std::function<QString(const QString &)> currentText, QWidget *parent) : QDialog(parent), m_adding(adding), m_currentText(std::move(currentText))
{
    setObjectName(QStringLiteral("scenarioEditDialog"));
    setWindowTitle(adding ? tr("Add Scenario") : tr("Edit Scenario"));
    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout;
    m_name = new QLineEdit(this);
    m_name->setObjectName(QStringLiteral("scenarioName"));
    m_name->setAccessibleName(tr("Scenario name"));
    m_name->setMaxLength(255);
    m_cells = new QLineEdit(this);
    m_cells->setObjectName(QStringLiteral("scenarioCells"));
    m_cells->setAccessibleName(tr("Changing cells"));
    m_cells->setReadOnly(!adding);
    m_comment = new QLineEdit(this);
    m_comment->setObjectName(QStringLiteral("scenarioComment"));
    m_comment->setAccessibleName(tr("Comment"));
    m_comment->setMaxLength(255);
    form->addRow(tr("Scenario &name:"), m_name);
    form->addRow(tr("&Changing cells:"), m_cells);
    form->addRow(tr("C&omment:"), m_comment);
    layout->addLayout(form);
    m_problem = new QLabel(this);
    m_problem->setObjectName(QStringLiteral("scenarioProblem"));
    m_problem->setTextFormat(Qt::PlainText);
    m_problem->setWordWrap(true);
    layout->addWidget(m_problem);
    m_table = new QTableWidget(0, 2, this);
    m_table->setObjectName(QStringLiteral("scenarioValues"));
    m_table->setAccessibleName(tr("Values of the changing cells"));
    m_table->setHorizontalHeaderLabels({tr("Cell"), tr("Value")});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->verticalHeader()->hide();
    m_table->setMinimumHeight(160);
    layout->addWidget(m_table);
    auto *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_ok = box->button(QDialogButtonBox::Ok);
    connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(box);
    connect(m_name, &QLineEdit::textChanged, this, [this] { refreshOk(); });
    connect(m_cells, &QLineEdit::textChanged, this, [this] { if (m_adding) cellsChanged(); });
    refreshOk();
    m_name->setFocus();
}

void ScenarioEditDialog::setName(const QString &name) { m_name->setText(name); }
void ScenarioEditDialog::setComment(const QString &comment) { m_comment->setText(comment); }
QString ScenarioEditDialog::name() const { return m_name->text().trimmed(); }
QString ScenarioEditDialog::cellsText() const { return m_cells->text().trimmed(); }
QString ScenarioEditDialog::comment() const { return m_comment->text().trimmed(); }

QStringList ScenarioEditDialog::values() const
{
    QStringList out;
    for (int r = 0; r < m_table->rowCount(); ++r) out << (m_table->item(r, 1) ? m_table->item(r, 1)->text() : QString());
    return out;
}

void ScenarioEditDialog::setCells(const QString &text, const QStringList &values)
{
    m_filling = true;
    m_cells->setText(text);
    QStringList cells;
    jp::crunch::parseScenarioCells(text, &cells);
    m_table->setRowCount(int(cells.size()));
    for (int r = 0; r < cells.size(); ++r) {
        auto *cell = new QTableWidgetItem(cells[r]);
        cell->setFlags(Qt::ItemIsEnabled);
        m_table->setItem(r, 0, cell);
        m_table->setItem(r, 1, new QTableWidgetItem(values.value(r)));
    }
    m_filling = false;
    refreshOk();
}

void ScenarioEditDialog::cellsChanged()
{
    if (m_filling) return;
    QStringList cells;
    QString problem;
    QHash<QString, QString> typed;
    for (int r = 0; r < m_table->rowCount(); ++r)
        if (m_table->item(r, 0) && m_table->item(r, 1)) typed.insert(m_table->item(r, 0)->text(), m_table->item(r, 1)->text());
    if (!jp::crunch::parseScenarioCells(m_cells->text(), &cells, &problem)) {
        m_problem->setText(m_cells->text().trimmed().isEmpty() ? QString() : problem);
        m_table->setRowCount(0);
        refreshOk();
        return;
    }
    m_problem->clear();
    m_filling = true;
    m_table->setRowCount(int(cells.size()));
    for (int r = 0; r < cells.size(); ++r) {
        auto *cell = new QTableWidgetItem(cells[r]);
        cell->setFlags(Qt::ItemIsEnabled);
        m_table->setItem(r, 0, cell);
        m_table->setItem(r, 1, new QTableWidgetItem(typed.contains(cells[r]) ? typed.value(cells[r]) : (m_currentText ? m_currentText(cells[r]) : QString())));
    }
    m_filling = false;
    refreshOk();
}

void ScenarioEditDialog::refreshOk() { m_ok->setEnabled(!name().isEmpty() && m_table->rowCount() > 0); }

} // namespace jp
