// Go To for a page or a slide: a number to type, from 1 to how many there are.

#include "app/gotonumber.h"

#include "app/plaintext.h"

#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <algorithm>

namespace jp {

int goToNumberDialog(QWidget *parent, const QString &prompt, int count, int current)
{
    if (count < 1) return -1;
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("goToDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::GoToNumberDialog", "Go To"));
    auto *v = new QVBoxLayout(&dlg);
    auto *label = plainLabel(new QLabel(prompt, &dlg));
    v->addWidget(label);
    auto *number = new QSpinBox(&dlg);
    number->setObjectName(QStringLiteral("goToNumber"));
    number->setAccessibleName(prompt);
    number->setRange(1, count);
    number->setValue(std::clamp(current, 0, count - 1) + 1);
    label->setBuddy(number);
    v->addWidget(number);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Close, &dlg);
    buttons->button(QDialogButtonBox::Ok)->setText(QCoreApplication::translate("jp::GoToNumberDialog", "Go To"));
    v->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    number->setFocus();
    number->selectAll();
    return dlg.exec() == QDialog::Accepted ? number->value() - 1 : -1;
}

} // namespace jp
