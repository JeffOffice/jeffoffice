#include "app/goalseekdialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace jp {

GoalSeekDialog::GoalSeekDialog(const QString &setCell, QWidget *parent) : QDialog(parent)
{
    setObjectName(QStringLiteral("goalSeekDialog"));
    setWindowTitle(tr("Goal Seek"));
    auto *layout = new QVBoxLayout(this);
    auto *about = new QLabel(tr("Finds the number to put in one cell so that a formula comes to the value you want."), this);
    about->setWordWrap(true);
    layout->addWidget(about);
    auto *form = new QFormLayout;
    m_set = new QLineEdit(setCell, this);
    m_set->setObjectName(QStringLiteral("goalSetCell"));
    m_set->setAccessibleName(tr("Cell with the formula"));
    m_value = new QLineEdit(this);
    m_value->setObjectName(QStringLiteral("goalToValue"));
    m_value->setAccessibleName(tr("The value you want"));
    m_change = new QLineEdit(this);
    m_change->setObjectName(QStringLiteral("goalChangeCell"));
    m_change->setAccessibleName(tr("Cell to change"));
    form->addRow(tr("Cell with the formula:"), m_set);
    form->addRow(tr("Value you want:"), m_value);
    form->addRow(tr("Cell to change:"), m_change);
    layout->addLayout(form);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
    auto *ok = buttons->button(QDialogButtonBox::Ok);
    auto refresh = [this, ok] { ok->setEnabled(!m_set->text().trimmed().isEmpty() && !m_value->text().trimmed().isEmpty() && !m_change->text().trimmed().isEmpty()); };
    connect(m_set, &QLineEdit::textChanged, this, refresh);
    connect(m_value, &QLineEdit::textChanged, this, refresh);
    connect(m_change, &QLineEdit::textChanged, this, refresh);
    refresh();
    m_value->setFocus();
}

QString GoalSeekDialog::setCell() const { return m_set->text().trimmed(); }
QString GoalSeekDialog::toValue() const { return m_value->text().trimmed(); }
QString GoalSeekDialog::changeCell() const { return m_change->text().trimmed(); }

} // namespace jp
