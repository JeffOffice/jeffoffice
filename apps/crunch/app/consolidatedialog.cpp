#include "app/consolidatedialog.h"

#include "core/subtotal.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

namespace jp {

ConsolidateDialog::ConsolidateDialog(const QString &destination, QWidget *parent) : QDialog(parent)
{
    setObjectName(QStringLiteral("consolidateDialog"));
    setWindowTitle(tr("Consolidate"));
    auto *layout = new QVBoxLayout(this);
    auto *about = new QLabel(tr("Combines ranges from one or several sheets into one table."), this);
    about->setWordWrap(true);
    layout->addWidget(about);
    auto *form = new QFormLayout;
    m_function = new QComboBox(this);
    m_function->setObjectName(QStringLiteral("consolidateFunction"));
    m_function->setAccessibleName(tr("Function"));
    for (const crunch::SubtotalFunction &f : crunch::subtotalFunctions()) m_function->addItem(QCoreApplication::translate("jp::crunch::Subtotal", f.name), f.code);
    form->addRow(tr("Function:"), m_function);
    m_destination = new QLineEdit(destination, this);
    m_destination->setObjectName(QStringLiteral("consolidateDestination"));
    m_destination->setAccessibleName(tr("Put the result at"));
    form->addRow(tr("Put the result at:"), m_destination);
    layout->addLayout(form);
    auto *refRow = new QHBoxLayout;
    m_reference = new QLineEdit(this);
    m_reference->setObjectName(QStringLiteral("consolidateReference"));
    m_reference->setAccessibleName(tr("Range to add"));
    m_reference->setPlaceholderText(tr("Sheet2!A1:C10"));
    auto *add = new QPushButton(tr("Add"), this);
    add->setObjectName(QStringLiteral("consolidateAdd"));
    refRow->addWidget(m_reference, 1);
    refRow->addWidget(add);
    layout->addLayout(refRow);
    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("consolidateList"));
    m_list->setAccessibleName(tr("Ranges to combine"));
    layout->addWidget(m_list);
    auto *remove = new QPushButton(tr("Delete"), this);
    remove->setObjectName(QStringLiteral("consolidateDelete"));
    layout->addWidget(remove, 0, Qt::AlignRight);
    m_top = new QCheckBox(tr("Top row holds the names of columns"), this);
    m_top->setObjectName(QStringLiteral("consolidateTop"));
    m_left = new QCheckBox(tr("Left column holds the names of rows"), this);
    m_left->setObjectName(QStringLiteral("consolidateLeft"));
    m_links = new QCheckBox(tr("Create links to the source data"), this);
    m_links->setObjectName(QStringLiteral("consolidateLinks"));
    layout->addWidget(m_top);
    layout->addWidget(m_left);
    layout->addWidget(m_links);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
    auto *ok = buttons->button(QDialogButtonBox::Ok);
    auto refresh = [this, ok] { ok->setEnabled(m_list->count() > 0 && !m_destination->text().trimmed().isEmpty()); };
    connect(add, &QPushButton::clicked, this, [this, refresh] {
        const QString t = m_reference->text().trimmed();
        if (t.isEmpty()) return;
        m_list->addItem(t);
        m_reference->clear();
        refresh();
    });
    connect(m_reference, &QLineEdit::returnPressed, add, &QPushButton::click);
    connect(remove, &QPushButton::clicked, this, [this, refresh] {
        qDeleteAll(m_list->selectedItems());
        refresh();
    });
    connect(m_destination, &QLineEdit::textChanged, this, refresh);
    refresh();
    m_reference->setFocus();
}

QStringList ConsolidateDialog::references() const
{
    QStringList out;
    for (int i = 0; i < m_list->count(); ++i) out << m_list->item(i)->text();
    return out;
}
int ConsolidateDialog::function() const { return m_function->currentData().toInt(); }
bool ConsolidateDialog::topLabels() const { return m_top->isChecked(); }
bool ConsolidateDialog::leftLabels() const { return m_left->isChecked(); }
bool ConsolidateDialog::links() const { return m_links->isChecked(); }
QString ConsolidateDialog::destination() const { return m_destination->text().trimmed(); }

} // namespace jp
