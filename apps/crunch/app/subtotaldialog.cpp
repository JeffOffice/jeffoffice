#include "app/subtotaldialog.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

namespace jp {

SubtotalDialog::SubtotalDialog(const QStringList &names, QWidget *parent) : QDialog(parent)
{
    setObjectName(QStringLiteral("subtotalDialog"));
    setWindowTitle(tr("Subtotal"));
    auto *layout = new QVBoxLayout(this);
    auto *about = new QLabel(tr("Adds a row of totals under each run of equal entries in a column. Sort the list by that column first."), this);
    about->setWordWrap(true);
    layout->addWidget(about);
    auto *form = new QFormLayout;
    m_at = new QComboBox(this);
    m_at->setObjectName(QStringLiteral("subtotalAt"));
    m_at->setAccessibleName(tr("At each change in"));
    m_at->addItems(names);
    m_function = new QComboBox(this);
    m_function->setObjectName(QStringLiteral("subtotalFunction"));
    m_function->setAccessibleName(tr("Use function"));
    for (const crunch::SubtotalFunction &f : crunch::subtotalFunctions()) m_function->addItem(QCoreApplication::translate("jp::crunch::Subtotal", f.name), f.code);
    form->addRow(tr("At each change in:"), m_at);
    form->addRow(tr("Use function:"), m_function);
    layout->addLayout(form);
    layout->addWidget(new QLabel(tr("Add subtotal to:"), this));
    m_columns = new QListWidget(this);
    m_columns->setObjectName(QStringLiteral("subtotalColumns"));
    m_columns->setAccessibleName(tr("Add subtotal to"));
    for (int i = 0; i < names.size(); ++i) {
        auto *item = new QListWidgetItem(names[i], m_columns);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(i == names.size() - 1 && names.size() > 1 ? Qt::Checked : Qt::Unchecked);
    }
    layout->addWidget(m_columns);
    m_replace = new QCheckBox(tr("Replace current subtotals"), this);
    m_replace->setObjectName(QStringLiteral("subtotalReplace"));
    m_replace->setChecked(true);
    layout->addWidget(m_replace);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    auto *remove = buttons->addButton(tr("Remove All"), QDialogButtonBox::ResetRole);
    remove->setObjectName(QStringLiteral("subtotalRemoveAll"));
    connect(remove, &QPushButton::clicked, this, [this] {
        m_removeAll = true;
        accept();
    });
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

int SubtotalDialog::changeColumn() const { return m_at->currentIndex(); }
int SubtotalDialog::function() const { return m_function->currentData().toInt(); }
bool SubtotalDialog::replace() const { return m_replace->isChecked(); }

QList<int> SubtotalDialog::totalColumns() const
{
    QList<int> out;
    for (int i = 0; i < m_columns->count(); ++i)
        if (m_columns->item(i)->checkState() == Qt::Checked) out << i;
    return out;
}

} // namespace jp
