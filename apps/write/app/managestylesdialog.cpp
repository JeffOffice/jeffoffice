// Manage Styles: every style the document has in a list, with whether it is recommended (in the gallery), whether it is hidden, and its order in the lists (core/styleops.h setStyleListing).

#include "app/writedialogs.h"

#include "app/plaintext.h"
#include "core/styleops.h"
#include "core/writedoc.h"
#include "office/package.h"
#include "wml/wml.h"

#include <QCheckBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QPushButton>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMap>
#include <QMessageBox>
#include <QSet>
#include <QSpinBox>
#include <QVBoxLayout>

namespace jp::write {

namespace {

struct Listing {
    bool gallery = false, hidden = false, locked = false;
    int priority = 99;
    bool operator==(const Listing &o) const { return gallery == o.gallery && hidden == o.hidden && priority == o.priority && locked == o.locked; }
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
    auto *locked = new QCheckBox(QCoreApplication::translate("jp::write::Dialogs", "Locked: not to be used while formatting is limited"), &dlg);
    locked->setObjectName(QStringLiteral("manageLocked"));
    auto *priority = new QSpinBox(&dlg);
    priority->setObjectName(QStringLiteral("managePriority"));
    priority->setRange(0, 99);
    priority->setAccessibleName(QCoreApplication::translate("jp::write::Dialogs", "Order in the lists"));
    auto *form = new QFormLayout;
    form->addRow(QCoreApplication::translate("jp::write::Dialogs", "Order in the lists (lower comes first):"), priority);
    side->addWidget(gallery);
    side->addWidget(hidden);
    side->addWidget(locked);
    side->addLayout(form);
    auto *del = new QPushButton(QCoreApplication::translate("jp::write::Dialogs", "Delete"), &dlg);
    del->setObjectName(QStringLiteral("manageDelete"));
    del->setToolTip(QCoreApplication::translate("jp::write::Dialogs", "Deletes a style you made. The styles that come with the program can't be deleted."));
    auto *import = new QPushButton(QCoreApplication::translate("jp::write::Dialogs", "Import from file..."), &dlg);
    import->setObjectName(QStringLiteral("manageImport"));
    auto *exportButton = new QPushButton(QCoreApplication::translate("jp::write::Dialogs", "Export to file..."), &dlg);
    exportButton->setObjectName(QStringLiteral("manageExport"));
    exportButton->setToolTip(QCoreApplication::translate("jp::write::Dialogs", "Saves the styles you choose to a template file that other documents can import them from."));
    side->addWidget(del);
    side->addWidget(import);
    side->addWidget(exportButton);
    // Limiting formatting is the document's: only styles that are not locked can be used, and text can't be formatted by hand.
    auto *limit = new QCheckBox(QCoreApplication::translate("jp::write::Dialogs", "Limit formatting to the styles that are not locked"), &dlg);
    limit->setObjectName(QStringLiteral("manageLimitFormatting"));
    const std::optional<office::wml::DocumentProtection> protectionBefore = doc->protection();
    limit->setChecked(doc->formattingLimited());
    if (protectionBefore && protectionBefore->enforced && protectionBefore->hasPassword()) {
        limit->setEnabled(false);
        limit->setToolTip(QCoreApplication::translate("jp::write::Dialogs", "This document is protected with a password. Stop the protection in Review > Restrict Editing first."));
    }
    side->addWidget(limit);
    side->addStretch(1);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    side->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    QMap<QString, Listing> before, now;
    QSet<QString> deletable;   // the styles the person made, which the document has
    auto addRow = [&](const ListedStyle &e) {
        const std::optional<office::wml::Style> s = styleOfDocument(*doc, e.id);
        if (!s) return;
        Listing l;
        l.gallery = s->qFormat;
        l.hidden = s->hidden;
        l.priority = s->uiPriority.value_or(99);
        l.locked = office::wml::styleLateLocked(s->late);
        before.insert(e.id, l);
        now.insert(e.id, l);
        if (s->customStyle && !s->isDefault && doc->styleSheet().find(e.id)) deletable.insert(e.id);
        auto *item = new QListWidgetItem(e.name, list);
        item->setData(Qt::UserRole, e.id);
    };
    for (const ListedStyle &e : listStyles(*doc, StyleFilter::All)) addRow(e);
    QString shown;
    auto keep = [&] {
        if (!shown.isEmpty()) now[shown] = Listing{gallery->isChecked(), hidden->isChecked(), locked->isChecked(), priority->value()};
    };
    QObject::connect(list, &QListWidget::currentItemChanged, &dlg, [&](QListWidgetItem *item) {
        keep();
        shown.clear();
        gallery->setEnabled(item);
        hidden->setEnabled(item);
        locked->setEnabled(item);
        priority->setEnabled(item);
        del->setEnabled(item && deletable.contains(item->data(Qt::UserRole).toString()));
        if (!item) return;
        const QString id = item->data(Qt::UserRole).toString();
        const Listing l = now.value(id);
        gallery->setChecked(l.gallery);
        hidden->setChecked(l.hidden);
        locked->setChecked(l.locked);
        priority->setValue(l.priority);
        shown = id;
    });
    // Delete takes the style away at once, as a step of its own (Undo puts it back); the choices made in this box for other styles wait for OK.
    QObject::connect(del, &QPushButton::clicked, &dlg, [&] {
        QListWidgetItem *item = list->currentItem();
        if (!item) return;
        const QString id = item->data(Qt::UserRole).toString();
        QString problem;
        if (!deletable.contains(id) || !deleteStyle(*doc, id, &problem)) return;
        shown.clear();
        before.remove(id);
        now.remove(id);
        deletable.remove(id);
        delete list->takeItem(list->row(item));
        if (list->count() > 0 && !list->currentItem()) list->setCurrentRow(0);
    });
    // Import copies styles from another file, and the list gains the ones that were new.
    QObject::connect(import, &QPushButton::clicked, &dlg, [&] {
        const QString path = QFileDialog::getOpenFileName(&dlg, QCoreApplication::translate("jp::write::Dialogs", "Import Styles"), QString(),
                                                          QCoreApplication::translate("jp::write::Dialogs", "Documents (*.docx *.dotx *.docm *.dotm *.doc *.dot *.rtf *.odt *.ott)"));
        if (path.isEmpty()) return;
        QByteArray bytes;
        QString err;
        std::unique_ptr<WriteDoc> other;
        if (office::readWholeFile(path, &bytes, &err) && !bytes.isEmpty()) other = WriteDoc::fromFile(bytes, &err, nullptr);
        if (!other) {
            jp::plainWarning(&dlg, QCoreApplication::translate("jp::write::Dialogs", "Import Styles"), QCoreApplication::translate("jp::write::Dialogs", "The styles of that file can't be read."));
            return;
        }
        const QSet<QString> had(before.keyBegin(), before.keyEnd());
        keep();
        if (!importStylesDialog(&dlg, doc, *other)) return;
        for (const ListedStyle &e : listStyles(*doc, StyleFilter::All))
            if (!had.contains(e.id)) addRow(e);
        list->sortItems();
    });
    // Export asks which styles, then where to put the file.
    QObject::connect(exportButton, &QPushButton::clicked, &dlg, [&] {
        keep();
        exportStylesDialog(&dlg, doc);
    });
    if (list->count() > 0) list->setCurrentRow(0);
    else {
        gallery->setEnabled(false);
        hidden->setEnabled(false);
        locked->setEnabled(false);
        priority->setEnabled(false);
    }
    if (dlg.exec() != QDialog::Accepted) return;
    keep();
    WriteDoc::StepGroup group(*doc);
    for (auto it = now.cbegin(); it != now.cend(); ++it) {
        const Listing was = before.value(it.key());
        if (it.value().gallery != was.gallery || it.value().hidden != was.hidden || it.value().priority != was.priority) setStyleListing(*doc, it.key(), it.value().gallery, it.value().hidden, it.value().priority);
        if (it.value().locked != was.locked) setStyleLocked(*doc, it.key(), it.value().locked);
    }
    if (limit->isChecked() != doc->formattingLimited()) {
        const office::wml::DocumentProtection with = office::wml::protectionWithFormatting(protectionBefore, limit->isChecked());
        // (Turned off, a protection that said nothing else goes altogether.)
        if (!limit->isChecked() && (with.edit.isEmpty() || with.edit == QLatin1String("none")) && !with.hasPassword()) doc->setProtection(std::nullopt);
        else doc->setProtection(with);
    }
}

// Export Styles: a list to check the styles to write, then the file to write them to.
bool exportStylesDialog(QWidget *parent, WriteDoc *doc)
{
    if (!doc) return false;
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("exportStylesDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::write::Dialogs", "Export Styles"));
    auto *column = new QVBoxLayout(&dlg);
    column->addWidget(new QLabel(QCoreApplication::translate("jp::write::Dialogs", "Check the styles to save in a template file:"), &dlg));
    auto *list = new QListWidget(&dlg);
    list->setObjectName(QStringLiteral("exportStylesList"));
    list->setAccessibleName(QCoreApplication::translate("jp::write::Dialogs", "Styles to export"));
    list->setMinimumHeight(220);
    column->addWidget(list, 1);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    column->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    // The styles the document has itself (a built-in style it never changed is in every document already).
    for (const ListedStyle &e : listStyles(*doc, StyleFilter::All)) {
        const office::wml::Style *s = doc->styleSheet().find(e.id);
        if (!e.inDocument || !s || (s->type != QLatin1String("paragraph") && s->type != QLatin1String("character"))) continue;
        auto *item = new QListWidgetItem(e.name, list);
        item->setData(Qt::UserRole, e.id);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
    }
    if (list->count() == 0) {
        jp::plainInformation(parent, QCoreApplication::translate("jp::write::Dialogs", "Export Styles"), QCoreApplication::translate("jp::write::Dialogs", "This document has no styles to export."));
        return false;
    }
    if (dlg.exec() != QDialog::Accepted) return false;
    QStringList ids;
    for (int i = 0; i < list->count(); ++i)
        if (list->item(i)->checkState() == Qt::Checked) ids << list->item(i)->data(Qt::UserRole).toString();
    if (ids.isEmpty()) return false;
    QString path = QFileDialog::getSaveFileName(parent, QCoreApplication::translate("jp::write::Dialogs", "Export Styles"), QString(), QCoreApplication::translate("jp::write::Dialogs", "Templates (*.dotx)"));
    if (path.isEmpty()) return false;
    if (QFileInfo(path).suffix().isEmpty()) path += QStringLiteral(".dotx");
    QString problem;
    if (!exportStyles(*doc, ids, path, &problem)) {
        jp::plainWarning(parent, QCoreApplication::translate("jp::write::Dialogs", "Export Styles"), problem);
        return false;
    }
    return true;
}

bool importStylesDialog(QWidget *parent, WriteDoc *doc, const WriteDoc &other)
{
    if (!doc) return false;
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("importStylesDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::write::Dialogs", "Import Styles"));
    auto *column = new QVBoxLayout(&dlg);
    column->addWidget(new QLabel(QCoreApplication::translate("jp::write::Dialogs", "Check the styles to copy into this document:"), &dlg));
    auto *list = new QListWidget(&dlg);
    list->setObjectName(QStringLiteral("importStylesList"));
    list->setAccessibleName(QCoreApplication::translate("jp::write::Dialogs", "Styles to import"));
    list->setMinimumHeight(220);
    column->addWidget(list, 1);
    auto *replace = new QCheckBox(QCoreApplication::translate("jp::write::Dialogs", "Replace styles that have the same name"), &dlg);
    replace->setObjectName(QStringLiteral("importReplace"));
    column->addWidget(replace);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    column->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    // The styles the other document defines that people make or use (its own built-in ones it has not changed are not listed).
    for (const ListedStyle &e : listStyles(const_cast<WriteDoc &>(other), StyleFilter::All)) {
        const office::wml::Style *s = other.styleSheet().find(e.id);
        if (!e.inDocument || !s || !s->customStyle) continue;
        auto *item = new QListWidgetItem(e.name, list);
        item->setData(Qt::UserRole, e.id);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
    }
    if (list->count() == 0) {
        jp::plainInformation(parent, QCoreApplication::translate("jp::write::Dialogs", "Import Styles"), QCoreApplication::translate("jp::write::Dialogs", "That file has no styles of its own to import."));
        return false;
    }
    if (dlg.exec() != QDialog::Accepted) return false;
    QStringList ids;
    for (int i = 0; i < list->count(); ++i)
        if (list->item(i)->checkState() == Qt::Checked) ids << list->item(i)->data(Qt::UserRole).toString();
    if (ids.isEmpty()) return false;
    QString problem;
    const int n = importStyles(*doc, other, ids, replace->isChecked(), &problem);
    if (n == 0 && !problem.isEmpty()) jp::plainWarning(parent, QCoreApplication::translate("jp::write::Dialogs", "Import Styles"), problem);
    return n > 0;
}

} // namespace jp::write
