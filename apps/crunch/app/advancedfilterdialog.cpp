#include "app/advancedfilterdialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

namespace jp {

AdvancedFilterDialog::AdvancedFilterDialog(const QString &list, QWidget *parent) : QDialog(parent)
{
    setObjectName(QStringLiteral("advancedFilterDialog"));
    setWindowTitle(tr("Advanced Filter"));
    auto *layout = new QVBoxLayout(this);
    auto *about = new QLabel(tr("Shows only the rows that meet the conditions you wrote on the sheet. The first row of the conditions repeats the headings of the columns to test; the rows under it hold the conditions."), this);
    about->setWordWrap(true);
    layout->addWidget(about);
    m_inPlace = new QRadioButton(tr("Hide the other rows, in the list"), this);
    m_inPlace->setObjectName(QStringLiteral("filterInPlace"));
    m_inPlace->setChecked(true);
    m_copy = new QRadioButton(tr("Copy the rows to another place"), this);
    m_copy->setObjectName(QStringLiteral("filterCopy"));
    layout->addWidget(m_inPlace);
    layout->addWidget(m_copy);
    auto *form = new QFormLayout;
    m_list = new QLineEdit(list, this);
    m_list->setObjectName(QStringLiteral("filterList"));
    m_list->setAccessibleName(tr("List range"));
    m_criteria = new QLineEdit(this);
    m_criteria->setObjectName(QStringLiteral("filterCriteria"));
    m_criteria->setAccessibleName(tr("Conditions range"));
    m_copyTo = new QLineEdit(this);
    m_copyTo->setObjectName(QStringLiteral("filterCopyTo"));
    m_copyTo->setAccessibleName(tr("Copy to"));
    m_copyTo->setEnabled(false);
    form->addRow(tr("List range:"), m_list);
    form->addRow(tr("Conditions range:"), m_criteria);
    form->addRow(tr("Copy to:"), m_copyTo);
    layout->addLayout(form);
    m_unique = new QCheckBox(tr("Leave out rows that repeat an earlier one"), this);
    m_unique->setObjectName(QStringLiteral("filterUnique"));
    layout->addWidget(m_unique);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
    connect(m_copy, &QRadioButton::toggled, m_copyTo, &QWidget::setEnabled);
    auto *ok = buttons->button(QDialogButtonBox::Ok);
    auto refresh = [this, ok] { ok->setEnabled(!m_list->text().trimmed().isEmpty() && !m_criteria->text().trimmed().isEmpty() && (!m_copy->isChecked() || !m_copyTo->text().trimmed().isEmpty())); };
    connect(m_list, &QLineEdit::textChanged, this, refresh);
    connect(m_criteria, &QLineEdit::textChanged, this, refresh);
    connect(m_copyTo, &QLineEdit::textChanged, this, refresh);
    connect(m_copy, &QRadioButton::toggled, this, refresh);
    refresh();
    m_criteria->setFocus();
}

QString AdvancedFilterDialog::list() const { return m_list->text().trimmed(); }
QString AdvancedFilterDialog::criteria() const { return m_criteria->text().trimmed(); }
QString AdvancedFilterDialog::copyTo() const { return m_copy->isChecked() ? m_copyTo->text().trimmed() : QString(); }
bool AdvancedFilterDialog::unique() const { return m_unique->isChecked(); }

} // namespace jp
