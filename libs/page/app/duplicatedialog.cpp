#include "app/duplicatedialog.h"
#include "app/editor.h"
#include "app/uistyle.h"
#include "app/widgets.h"
#include <QCheckBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QVBoxLayout>

namespace jp {

bool duplicateStepsDialog(QWidget *p, Editor *ed)
{
    if (ed->selection().isEmpty()) return false;
    QDialog d(p);
    d.setObjectName(QStringLiteral("duplicateDialog"));
    d.setWindowTitle(QCoreApplication::translate("Dialogs", "Duplicate Several Times"));
    auto *v = new QVBoxLayout(&d);
    auto *form = new QFormLayout();
    auto *copies = new QSpinBox(&d);
    copies->setObjectName(QStringLiteral("duplicateCopies"));
    copies->setRange(1, Editor::kMaxDuplicateCopies);
    copies->setValue(3);
    auto measure = [&](const char *name, double pt) {
        auto *m = new MeasureSpin(&d);
        m->setObjectName(QLatin1String(name));
        m->setRange(-10000, 10000);
        m->setValue(pt);
        m->setFixedWidth(110);
        return m;
    };
    auto *dx = measure("duplicateOffsetX", 12), *dy = measure("duplicateOffsetY", 12);
    auto *grow = new DecimalSpin(&d);
    grow->setObjectName(QStringLiteral("duplicateGrow"));
    grow->setRange(-90, 1000);
    grow->setSuffix(QStringLiteral(" %"));
    grow->setValue(0);
    auto *step = new QCheckBox(QCoreApplication::translate("Dialogs", "Step the fill color to"), &d);
    step->setObjectName(QStringLiteral("duplicateStepColor"));
    auto *last = new ColorButton(QStringLiteral("square"), QString(), false, QString(), &d);
    last->setObjectName(QStringLiteral("duplicateLastColor"));
    last->setScheme(ed->doc()->colors);
    last->setCurrent(ColorRef::rgb(Qt::black));
    last->setPopupMode(QToolButton::InstantPopup);
    last->setWell(true);
    last->setEnabled(false);
    QObject::connect(last, &ColorButton::colorPicked, last, [last](const ColorRef &c) { last->setCurrent(c); });
    QObject::connect(step, &QCheckBox::toggled, last, &QWidget::setEnabled);
    form->addRow(QCoreApplication::translate("Dialogs", "Number of copies"), copies);
    form->addRow(QCoreApplication::translate("Dialogs", "Horizontal offset"), dx);
    form->addRow(QCoreApplication::translate("Dialogs", "Vertical offset"), dy);
    form->addRow(QCoreApplication::translate("Dialogs", "Size change for each copy"), grow);
    form->addRow(step, last);
    v->addLayout(form);
    auto *bb = new jp::ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &d);
    QObject::connect(bb, &QDialogButtonBox::accepted, &d, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &d, &QDialog::reject);
    v->addWidget(bb);
    if (d.exec() != QDialog::Accepted) return false;
    Editor::DuplicateSteps s;
    s.copies = copies->value();
    s.offset = QPointF(dx->value(), dy->value());
    s.growPercent = grow->value();
    s.stepColor = step->isChecked();
    s.lastColor = last->current().resolve(ed->doc()->colors);
    return ed->duplicateSteps(s);
}

} // namespace jp
