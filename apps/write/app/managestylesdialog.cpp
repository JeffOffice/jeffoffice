// Manage Styles: every style the document has in a list, with whether it is recommended (in the gallery), whether it is hidden, and its order in the lists (core/styleops.h setStyleListing).

#include "app/writedialogs.h"

#include "core/styleops.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMap>
#include <QSpinBox>
#include <QVBoxLayout>

namespace jp::write {

namespace {

struct Listing {
    bool gallery = false, hidden = false;
    int priority = 99;
    bool operator==(const Listing &o) const { return gallery == o.gallery && hidden == o.hidden && priority == o.priority; }
};

}

void manageStylesDialog(QWidget *parent, WriteDoc *doc)
{
    if (!doc) return;
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("manageStylesDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::write::Dialogs", "Manage Styles"));
    auto *row = new QHBoxLayout(&dlg);
    auto *list = new QListWidget(&dlg);
    list->setObjectName(QStringLiteral("manageStylesList"));
    list->setAccessibleName(QCoreApplication::translate("jp::write::Dialogs", "Styles"));
    list->setMinimumWidth(220);
    row->addWidget(list, 1);
    auto *side = new QVBoxLayout;
    row->addLayout(side);
    auto *gallery = new QCheckBox(QCoreApplication::translate("jp::write::Dialogs", "Recommended: show in the gallery"), &dlg);
    gallery->setObjectName(QStringLiteral("manageInGallery"));
    auto *hidden = new QCheckBox(QCoreApplication::translate("jp::write::Dialogs", "Hidden: leave out of every list"), &dlg);
    hidden->setObjectName(QStringLiteral("manageHidden"));
    auto *priority = new QSpinBox(&dlg);
    priority->setObjectName(QStringLiteral("managePriority"));
    priority->setRange(0, 99);
    priority->setAccessibleName(QCoreApplication::translate("jp::write::Dialogs", "Order in the lists"));
    auto *form = new QFormLayout;
    form->addRow(QCoreApplication::translate("jp::write::Dialogs", "Order in the lists (lower comes first):"), priority);
    side->addWidget(gallery);
    side->addWidget(hidden);
    side->addLayout(form);
    side->addStretch(1);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    side->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    QMap<QString, Listing> before, now;
    for (const ListedStyle &e : listStyles(*doc, StyleFilter::All)) {
        const std::optional<office::wml::Style> s = styleOfDocument(*doc, e.id);
        if (!s) continue;
        Listing l;
        l.gallery = s->qFormat;
        l.hidden = s->hidden;
        l.priority = s->uiPriority.value_or(99);
        before.insert(e.id, l);
        now.insert(e.id, l);
        auto *item = new QListWidgetItem(e.name, list);
        item->setData(Qt::UserRole, e.id);
    }
    QString shown;
    auto keep = [&] {
        if (!shown.isEmpty()) now[shown] = Listing{gallery->isChecked(), hidden->isChecked(), priority->value()};
    };
    QObject::connect(list, &QListWidget::currentItemChanged, &dlg, [&](QListWidgetItem *item) {
        keep();
        shown.clear();
        gallery->setEnabled(item);
        hidden->setEnabled(item);
        priority->setEnabled(item);
        if (!item) return;
        const QString id = item->data(Qt::UserRole).toString();
        const Listing l = now.value(id);
        gallery->setChecked(l.gallery);
        hidden->setChecked(l.hidden);
        priority->setValue(l.priority);
        shown = id;
    });
    if (list->count() > 0) list->setCurrentRow(0);
    else {
        gallery->setEnabled(false);
        hidden->setEnabled(false);
        priority->setEnabled(false);
    }
    if (dlg.exec() != QDialog::Accepted) return;
    keep();
    WriteDoc::StepGroup group(*doc);
    for (auto it = now.cbegin(); it != now.cend(); ++it)
        if (!(it.value() == before.value(it.key()))) setStyleListing(*doc, it.key(), it.value().gallery, it.value().hidden, it.value().priority);
}

} // namespace jp::write
