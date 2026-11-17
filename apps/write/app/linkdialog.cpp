// The Hyperlink dialog (the work on the document is core/linkops.h).

#include "app/linktargetpicker.h"
#include "app/uistyle.h"
#include "app/writedialogs.h"

#include "app/pageview.h"
#include "core/gotoops.h"
#include "core/linkops.h"
#include "core/refops.h"

#include <QFile>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
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
    auto *form = new QFormLayout;
    auto *display = new QLineEdit(&dlg);
    display->setObjectName(QStringLiteral("display"));
    display->setText(selecting ? c.selectedText().remove(QChar::ObjectReplacementCharacter) : existing.text);
    display->setEnabled(!selecting);
    form->addRow(QCoreApplication::translate("jp::write::LinkDialog", "Text to display"), display);
    v->addLayout(form);
    auto *picker = new LinkTargetPicker(&dlg);
    picker->setPlaceNames(QCoreApplication::translate("jp::write::LinkDialog", "Place in this document"), QCoreApplication::translate("jp::write::LinkDialog", "Place"));
    // The places: the headings, then the bookmarks a person made (the hidden ones belong to headings). The list stops being built a place past the most it holds, and the place the
    // link has now is always in it (first), so that changing the link's tip or words never changes where it goes.
    QVector<LinkPlace> places;
    const int enough = kMostLinkPlaces + 1;
    for (const RefTarget &t : crossReferenceTargets(*doc, RefKind::Heading)) {
        if (places.size() >= enough) break;
        places.push_back({QCoreApplication::translate("jp::write::LinkDialog", "Heading: %1").arg(t.label.left(120)), headingPlace(t.position)});
    }
    for (const BookmarkItem &b : bookmarkList(*doc, false, false)) {
        if (places.size() >= enough) break;
        places.push_back({QCoreApplication::translate("jp::write::LinkDialog", "Bookmark: %1").arg(b.name.left(120)), b.name});
    }
    LinkPlace current;
    const QString here = existing.anchor.isEmpty() ? QString() : placeOfAnchor(*doc, existing.anchor);
    if (!here.isEmpty()) {
        if (here == existing.anchor) {
            current = {QCoreApplication::translate("jp::write::LinkDialog", "Bookmark: %1").arg(existing.anchor.left(120)), existing.anchor};
        } else {
            QString label = doc->story()->findBlock(headingPositionOfPlace(here)).text();
            label.remove(QChar::ObjectReplacementCharacter);
            current = {QCoreApplication::translate("jp::write::LinkDialog", "Heading: %1").arg(label.simplified().left(120)), here};
        }
    }
    picker->setPlaces(places, current);
    picker->enableNewDocument(QCoreApplication::translate("jp::write::LinkDialog", "Documents (*.docx *.odt)"), {QStringLiteral("docx"), QStringLiteral("odt")}, [](const QString &path, QString *why) {
        const bool odt = path.endsWith(QLatin1String(".odt"), Qt::CaseInsensitive);
        const QByteArray bytes = odt ? WriteDoc::blank()->toOdt() : WriteDoc::blank()->toDocx();
        QFile f(path);
        if (bytes.isEmpty() || !f.open(QIODevice::WriteOnly | QIODevice::NewOnly) || f.write(bytes) != bytes.size()) {
            if (why) *why = QCoreApplication::translate("jp::write::LinkDialog", "The new document could not be written there.");
            return false;
        }
        return true;
    });
    if (!existing.anchor.isEmpty()) {
        picker->setKind(LinkTargetPicker::Place);
        picker->setPlaceValue(here);
    } else if (existing.address.startsWith(QLatin1String("mailto:"), Qt::CaseInsensitive)) {
        picker->setKind(LinkTargetPicker::Email);
        picker->setAddress(existing.address.mid(7));
    } else if (isFileLink(existing.address)) {
        picker->setKind(LinkTargetPicker::File);
        picker->setAddress(pathOfFileLink(existing.address));
    } else {
        picker->setAddress(existing.address);
    }
    v->addWidget(picker);
    auto *form2 = new QFormLayout;
    auto *tip = new QLineEdit(&dlg);
    tip->setObjectName(QStringLiteral("screenTip"));
    tip->setText(existing.screenTip);
    form2->addRow(QCoreApplication::translate("jp::write::LinkDialog", "Screen tip"), tip);
    v->addLayout(form2);
    auto *problem = new QLabel(&dlg);
    problem->setObjectName(QStringLiteral("problem"));
    problem->setWordWrap(true);
    problem->setTextFormat(Qt::PlainText);
    v->addWidget(problem);
    auto *bb = new jp::ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    QPushButton *remove = nullptr;
    if (existing.valid()) {
        remove = bb->addButton(QCoreApplication::translate("jp::write::LinkDialog", "Remove Link"), QDialogButtonBox::DestructiveRole);
        remove->setObjectName(QStringLiteral("remove"));
    }
    v->addWidget(bb);
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
        QString target, anchor;
        QString words = display->text();
        const LinkTargetPicker::Kind kind = picker->kind();
        if (kind == LinkTargetPicker::Place) {
            anchor = picker->placeValue();
            if (anchor.isEmpty()) why = QCoreApplication::translate("jp::write::LinkDialog", "Choose a place for the link to go to.");
        } else if (kind == LinkTargetPicker::File || kind == LinkTargetPicker::NewDocument) {
            if (kind == LinkTargetPicker::NewDocument) {
                if (picker->prepare(&why)) target = fileLinkOfPath(picker->target());
            } else {
                target = fileLinkOfPath(picker->address());
                if (target.isEmpty()) why = QCoreApplication::translate("jp::write::LinkDialog", "Type the whole path of a file on this computer, or press Browse. A link can't go to a program.");
            }
            if (!target.isEmpty() && words.isEmpty()) words = QFileInfo(pathOfFileLink(target)).fileName();
        } else {
            target = normalizedAddress(picker->target());
            if (target.isEmpty() && why.isEmpty()) why = QCoreApplication::translate("jp::write::LinkDialog", "Type an address for the link to go to.");
        }
        bool ok = false;
        if (why.isEmpty()) {
            if (existing.valid()) ok = changeHyperlink(*doc, existing.start, target, anchor, tip->text(), words, &why);
            else ok = insertHyperlink(*doc, c, target, anchor, tip->text(), words, &why);
            if (!ok && kind == LinkTargetPicker::NewDocument) picker->discardCreated();
        }
        if (!ok) problem->setText(why);
        return ok;
    };
    const int result = dlg.exec();
    if (removed && existing.valid()) return removeHyperlink(*doc, existing.start);
    return result == QDialog::Accepted;
}

} // namespace jp::write
