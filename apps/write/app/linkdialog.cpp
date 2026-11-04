// The Hyperlink dialog (the work on the document is core/linkops.h).

#include "app/writedialogs.h"

#include "app/pageview.h"
#include "core/gotoops.h"
#include "core/linkops.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>
#include <functional>

namespace jp::write {

namespace {

// A dialog whose OK is refused (the dialog stays open) until what it holds can be used.
class CheckedDialog : public QDialog {
public:
    using QDialog::QDialog;
    std::function<bool()> check;
    void accept() override
    {
        if (!check || check()) QDialog::accept();
    }
};

}

bool linkDialog(QWidget *parent, WriteDoc *doc, PageView *view)
{
    if (!doc || !view) return false;
    QTextCursor &c = view->cursor();
    const Hyperlink existing = c.hasSelection() ? Hyperlink() : hyperlinkAt(*doc, c.position());
    const bool selecting = c.hasSelection();
    CheckedDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("linkDialog"));
    dlg.setWindowTitle(existing.valid() ? QCoreApplication::translate("jp::write::LinkDialog", "Edit Hyperlink") : QCoreApplication::translate("jp::write::LinkDialog", "Insert Hyperlink"));
    auto *v = new QVBoxLayout(&dlg);
    auto *web = new QRadioButton(QCoreApplication::translate("jp::write::LinkDialog", "Web or email address"), &dlg);
    web->setObjectName(QStringLiteral("kindWeb"));
    auto *place = new QRadioButton(QCoreApplication::translate("jp::write::LinkDialog", "Place in this document"), &dlg);
    place->setObjectName(QStringLiteral("kindPlace"));
    v->addWidget(web);
    v->addWidget(place);
    auto *form = new QFormLayout;
    auto *display = new QLineEdit(&dlg);
    display->setObjectName(QStringLiteral("display"));
    display->setText(selecting ? c.selectedText().remove(QChar::ObjectReplacementCharacter) : existing.text);
    display->setEnabled(!selecting);
    auto *address = new QLineEdit(&dlg);
    address->setObjectName(QStringLiteral("address"));
    address->setText(existing.address);
    address->setPlaceholderText(QCoreApplication::translate("jp::write::LinkDialog", "https://example.com"));
    auto *bookmark = new QComboBox(&dlg);
    bookmark->setObjectName(QStringLiteral("place"));
    for (const GoToItem &b : goToItems(*doc, GoToKind::Bookmark)) bookmark->addItem(b.label);
    bookmark->setCurrentText(existing.anchor);
    auto *tip = new QLineEdit(&dlg);
    tip->setObjectName(QStringLiteral("screenTip"));
    tip->setText(existing.screenTip);
    form->addRow(QCoreApplication::translate("jp::write::LinkDialog", "Text to display:"), display);
    form->addRow(QCoreApplication::translate("jp::write::LinkDialog", "Address:"), address);
    form->addRow(QCoreApplication::translate("jp::write::LinkDialog", "Bookmark:"), bookmark);
    form->addRow(QCoreApplication::translate("jp::write::LinkDialog", "Screen tip:"), tip);
    v->addLayout(form);
    auto *problem = new QLabel(&dlg);
    problem->setObjectName(QStringLiteral("problem"));
    problem->setWordWrap(true);
    problem->setTextFormat(Qt::PlainText);
    v->addWidget(problem);
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    QPushButton *remove = nullptr;
    if (existing.valid()) {
        remove = bb->addButton(QCoreApplication::translate("jp::write::LinkDialog", "Remove Link"), QDialogButtonBox::DestructiveRole);
        remove->setObjectName(QStringLiteral("remove"));
    }
    v->addWidget(bb);
    auto enable = [=] {
        address->setEnabled(web->isChecked());
        bookmark->setEnabled(place->isChecked());
    };
    (existing.anchor.isEmpty() ? web : place)->setChecked(true);
    QObject::connect(web, &QRadioButton::toggled, &dlg, enable);
    enable();
    bool removed = false;
    if (remove)
        QObject::connect(remove, &QPushButton::clicked, &dlg, [&] {
            removed = true;
            dlg.check = nullptr;
            dlg.accept();
        });
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    // OK stays open, saying why, until the link can be made.
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    dlg.check = [&] {
        QString why;
        const QString target = web->isChecked() ? normalizedAddress(address->text()) : QString();
        const QString anchor = place->isChecked() ? bookmark->currentText() : QString();
        bool ok = false;
        if (existing.valid()) ok = changeHyperlink(*doc, existing.start, target, anchor, tip->text(), display->text(), &why);
        else ok = insertHyperlink(*doc, c, target, anchor, tip->text(), display->text(), &why);
        if (web->isChecked() && target.isEmpty() && why.isEmpty()) why = QCoreApplication::translate("jp::write::LinkDialog", "Type an address for the link to go to.");
        if (!ok) problem->setText(why);
        return ok;
    };
    const int result = dlg.exec();
    if (removed && existing.valid()) return removeHyperlink(*doc, existing.start);
    return result == QDialog::Accepted;
}

} // namespace jp::write
