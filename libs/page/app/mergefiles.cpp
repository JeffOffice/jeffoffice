#include "app/mergefiles.h"

#include "app/plaintext.h"
#include "app/uistyle.h"

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProgressDialog>
#include <QPushButton>
#include <QVBoxLayout>

namespace jp {

bool mergeFilesDialog(QWidget *p, const MergeSource &m, const QVector<int> &rows, const MergeFilesKinds &kinds, const QString &startFolder, MergeFilesChoice *choice)
{
    QDialog d(p);
    d.setObjectName(QStringLiteral("mergeFilesDialog"));
    d.setWindowTitle(QCoreApplication::translate("MergeFiles", "Save Each as a File"));
    auto *v = new QVBoxLayout(&d);
    auto *form = new QFormLayout();
    auto *folderRow = new QHBoxLayout();
    auto *folder = new QLineEdit(startFolder, &d);
    folder->setObjectName(QStringLiteral("mergeFilesFolder"));
    auto *browse = new QPushButton(QCoreApplication::translate("MergeFiles", "Choose…"), &d);
    folderRow->addWidget(folder, 1);
    folderRow->addWidget(browse);
    form->addRow(QCoreApplication::translate("MergeFiles", "Save in the folder"), folderRow);
    auto *pattern = new QLineEdit(defaultFileNamePattern(m), &d);
    pattern->setObjectName(QStringLiteral("mergeFilesPattern"));
    pattern->setToolTip(QCoreApplication::translate("MergeFiles", "Put the name of a column in braces, like {Last Name}, {First Name}."));
    form->addRow(QCoreApplication::translate("MergeFiles", "Name each file"), pattern);
    auto *kind = new QComboBox(&d);
    kind->setObjectName(QStringLiteral("mergeFilesKind"));
    kind->addItem(kinds.ownLabel);
    if (kinds.canPdf) kind->addItem(QCoreApplication::translate("MergeFiles", "PDF (.pdf)"));
    form->addRow(QCoreApplication::translate("MergeFiles", "Make each file as"), kind);
    v->addLayout(form);
    auto *help = new QLabel(QCoreApplication::translate("MergeFiles", "Put the name of a column in braces, like {Last Name}, {First Name}. A name that is used twice gets a number."), &d);
    help->setWordWrap(true);
    help->setTextFormat(Qt::PlainText);
    v->addWidget(help);
    auto *preview = new QLabel(&d);
    preview->setObjectName(QStringLiteral("mergeFilesPreview"));
    preview->setTextFormat(Qt::PlainText);
    preview->setWordWrap(true);
    v->addWidget(preview);
    auto *bb = new jp::ButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &d);
    v->addWidget(bb);
    QObject::connect(bb, &QDialogButtonBox::accepted, &d, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &d, &QDialog::reject);
    QObject::connect(browse, &QPushButton::clicked, &d, [&] {
        const QString dir = QFileDialog::getExistingDirectory(&d, QCoreApplication::translate("MergeFiles", "Save in the folder"), folder->text());
        if (!dir.isEmpty()) folder->setText(dir);
    });
    auto refresh = [&] {
        const QString ext = kind->currentIndex() == 1 ? QStringLiteral(".pdf") : kinds.ownExtension;
        // (only the first names are made for the preview: a list of a hundred thousand is not walked to show five)
        const QVector<int> first = rows.mid(0, 5);
        QStringList names = mergeFileNames(m, first, pattern->text(), ext);
        if (rows.size() > first.size()) names << QStringLiteral("…");
        preview->setText(QCoreApplication::translate("MergeFiles", "%n file will be made, for example:\n%1", nullptr, int(rows.size())).arg(names.join(QLatin1Char('\n'))));
        if (QPushButton *save = bb->button(QDialogButtonBox::Save)) save->setEnabled(!rows.isEmpty() && !folder->text().trimmed().isEmpty() && !pattern->text().trimmed().isEmpty());
    };
    QObject::connect(folder, &QLineEdit::textChanged, &d, [&](const QString &) { refresh(); });
    QObject::connect(pattern, &QLineEdit::textChanged, &d, [&](const QString &) { refresh(); });
    QObject::connect(kind, &QComboBox::currentIndexChanged, &d, [&](int) { refresh(); });
    refresh();
    if (d.exec() != QDialog::Accepted) return false;
    choice->folder = folder->text().trimmed();
    choice->pattern = pattern->text();
    choice->pdf = kind->currentIndex() == 1;
    return true;
}

bool confirmMergeFiles(QWidget *p, const QString &folder, const QStringList &names)
{
    const QString title = QCoreApplication::translate("MergeFiles", "Save Each as a File");
    if (names.size() > kMostMergeFiles) {
        jp::plainWarning(p, title, QCoreApplication::translate("MergeFiles", "A merge makes at most %n file at a time. Use a shorter list.", nullptr, kMostMergeFiles));
        return false;
    }
    if (names.size() > kAskAboveMergeFiles &&
        jp::plainQuestion(p, title, QCoreApplication::translate("MergeFiles", "This makes %n file in the folder %1. Make them?", nullptr, int(names.size())).arg(folder)) != QMessageBox::Yes)
        return false;
    const int existing = mergeFilesThatExist(folder, names);
    if (existing > 0 &&
        jp::plainQuestion(p, title, QCoreApplication::translate("MergeFiles", "%n file in that folder has the same name as one to be made. Replace it?", nullptr, existing)) != QMessageBox::Yes)
        return false;
    return true;
}

MergeFilesResult runMergeFiles(QWidget *p, const QString &folder, const QVector<int> &rows, const QStringList &names, const MakeMergeFile &make)
{
    QProgressDialog box(QCoreApplication::translate("MergeFiles", "Saving the files…"), QCoreApplication::translate("MergeFiles", "Cancel"), 0, int(rows.size()), p);
    box.setObjectName(QStringLiteral("mergeFilesBox"));
    box.setWindowModality(Qt::WindowModal);
    box.setMinimumDuration(400);
    box.setAutoClose(false);
    box.setAutoReset(false);
    const MergeFilesResult res = makeMergeFiles(folder, rows, names, make, [&](int done, int) {
        box.setValue(done);
        QCoreApplication::processEvents();
        return !box.wasCanceled();
    });
    box.close();
    const QString title = QCoreApplication::translate("MergeFiles", "Save Each as a File");
    if (!res.error.isEmpty()) jp::plainWarning(p, title, QCoreApplication::translate("MergeFiles", "%n file was saved before this stopped.", nullptr, res.written) + QLatin1Char('\n') + res.error);
    else if (res.stopped) jp::plainInformation(p, title, QCoreApplication::translate("MergeFiles", "Stopped. %n file was saved in the folder %1.", nullptr, res.written).arg(folder));
    else jp::plainInformation(p, title, QCoreApplication::translate("MergeFiles", "%n file was saved in the folder %1.", nullptr, res.written).arg(folder));
    return res;
}

} // namespace jp
