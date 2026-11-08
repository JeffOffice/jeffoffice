#include "app/versionhistory.h"

#include "app/plaintext.h"

#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace jp {

QString versionsNeedAFile(VersionHistoryHost::Kind kind)
{
    if (kind == VersionHistoryHost::Kind::Document) return QCoreApplication::translate("jp::VersionHistory", "Save the document first. A version is kept for a file, under the file's name.");
    if (kind == VersionHistoryHost::Kind::Workbook) return QCoreApplication::translate("jp::VersionHistory", "Save the workbook first. A version is kept for a file, under the file's name.");
    return kind == VersionHistoryHost::Kind::Publication ? QCoreApplication::translate("jp::VersionHistory", "Save the publication first. A version is kept for a file, under the file's name.")
                                                         : QCoreApplication::translate("jp::VersionHistory", "Save the presentation first. A version is kept for a file, under the file's name.");
}

void showVersionHistory(QWidget *parent, const VersionHistoryHost &host)
{
    const bool pub = host.kind == VersionHistoryHost::Kind::Publication;
    if (host.path.isEmpty()) {
        jp::plainInformation(parent, QCoreApplication::translate("jp::VersionHistory", "Version History"), versionsNeedAFile(host.kind));
        return;
    }
    QDialog dialog(parent);
    dialog.setObjectName(QStringLiteral("versionDialog"));
    dialog.setWindowTitle(QCoreApplication::translate("jp::VersionHistory", "Version History"));
    dialog.resize(560, 380);
    auto *layout = new QVBoxLayout(&dialog);
    const QString introText = host.kind == VersionHistoryHost::Kind::Document
        ? QCoreApplication::translate("jp::VersionHistory", "Versions are copies of this document that are kept on this computer. Keep one before a big change. Later you can compare it with the document, or go back to it.")
        : host.kind == VersionHistoryHost::Kind::Workbook
        ? QCoreApplication::translate("jp::VersionHistory", "Versions are copies of this workbook that are kept on this computer. Keep one before a big change. Later you can open it beside the workbook, or go back to it.")
        : pub ? QCoreApplication::translate("jp::VersionHistory", "Versions are copies of this publication that are kept on this computer. Keep one before a big change. Later you can open it, or go back to it.")
              : QCoreApplication::translate("jp::VersionHistory", "Versions are copies of this presentation that are kept on this computer. Keep one before a big change. Later you can open it, or go back to it.");
    auto *intro = new QLabel(introText, &dialog);
    intro->setTextFormat(Qt::PlainText);
    intro->setWordWrap(true);
    layout->addWidget(intro);
    auto *list = new QListWidget(&dialog);
    list->setObjectName(QStringLiteral("versionList"));
    layout->addWidget(list, 1);
    auto *row = new QHBoxLayout;
    auto *keep = new QPushButton(QCoreApplication::translate("jp::VersionHistory", "Keep a Version…"), &dialog);
    auto *open = new QPushButton(QCoreApplication::translate("jp::VersionHistory", "Open a Copy"), &dialog);
    auto *compare = host.compare ? new QPushButton(QCoreApplication::translate("jp::VersionHistory", "Compare"), &dialog) : nullptr;
    auto *restore = new QPushButton(QCoreApplication::translate("jp::VersionHistory", "Go Back to It"), &dialog);
    auto *drop = new QPushButton(QCoreApplication::translate("jp::VersionHistory", "Delete"), &dialog);
    for (QPushButton *b : {keep, open, compare, restore, drop})
        if (b) row->addWidget(b);
    layout->addLayout(row);
    auto *close = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    QObject::connect(close, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(close);
    QDialog *dlg = &dialog;
    const auto store = [&host] { return write::VersionStore(host.path, host.extension); };
    auto fill = [list, store] {
        list->clear();
        for (const write::DocVersion &v : store().list()) {
            auto *it = new QListWidgetItem(QCoreApplication::translate("jp::VersionHistory", "%1 (%2, %3 KB)").arg(v.name, QLocale().toString(v.saved, QLocale::ShortFormat)).arg(QLocale().toString(std::max<qint64>(1, v.size / 1024))));
            it->setData(Qt::UserRole, v.file);
            list->addItem(it);
        }
        if (list->count()) list->setCurrentRow(0);
    };
    auto chosen = [list] { return list->currentItem() ? list->currentItem()->data(Qt::UserRole).toString() : QString(); };
    auto sync = [=] {
        for (QPushButton *b : {open, compare, restore, drop})
            if (b) b->setEnabled(list->currentItem() != nullptr);
    };
    QObject::connect(list, &QListWidget::currentRowChanged, dlg, sync);
    QObject::connect(keep, &QPushButton::clicked, dlg, [=, &host] {
        bool ok = false;
        const QString name = QInputDialog::getText(dlg, QCoreApplication::translate("jp::VersionHistory", "Keep a Version"), QCoreApplication::translate("jp::VersionHistory", "Name for this version:"), QLineEdit::Normal, QCoreApplication::translate("jp::VersionHistory", "Version %1").arg(store().list().size() + 1), &ok);
        if (ok) host.keep(name, dlg, [=] {
                fill();
                sync();
            });
    });
    QObject::connect(open, &QPushButton::clicked, dlg, [=, &host] {
        const QString file = chosen();
        if (file.isEmpty()) return;
        dlg->accept();
        host.openCopy(store().folder() + QLatin1Char('/') + file);
    });
    if (compare)
        QObject::connect(compare, &QPushButton::clicked, dlg, [=, &host] {
            const QString file = chosen();
            if (file.isEmpty()) return;
            dlg->accept();
            host.compare(file);
        });
    QObject::connect(restore, &QPushButton::clicked, dlg, [=, &host] {
        const QString file = chosen();
        if (file.isEmpty()) return;
        const QString ask = host.kind == VersionHistoryHost::Kind::Document ? QCoreApplication::translate("jp::VersionHistory", "Go back to this version? The document as it is now is kept as a version first.")
                          : host.kind == VersionHistoryHost::Kind::Workbook ? QCoreApplication::translate("jp::VersionHistory", "Go back to this version? The workbook as it is now is kept as a version first.")
                          : pub ? QCoreApplication::translate("jp::VersionHistory", "Go back to this version? The publication as it is now is kept as a version first.")
                                : QCoreApplication::translate("jp::VersionHistory", "Go back to this version? The presentation as it is now is kept as a version first.");
        if (jp::plainQuestion(dlg, QCoreApplication::translate("jp::VersionHistory", "Go Back"), ask) != QMessageBox::Yes)
            return;
        dlg->accept();
        host.restore(file);
    });
    QObject::connect(drop, &QPushButton::clicked, dlg, [=] {
        const QString file = chosen();
        if (file.isEmpty()) return;
        store().remove(file);
        fill();
        sync();
    });
    fill();
    sync();
    dialog.exec();
}

} // namespace jp
