// Version History (File > Info): named copies of the document kept on this computer, to compare with and to go back to. The copies are kept by core/versionstore.cpp.

#include "app/writewindow.h"

#include "app/pageview.h"
#include "app/plaintext.h"
#include "app/settings.h"
#include "core/appinfo.h"

#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QStatusBar>
#include <QTemporaryDir>
#include <QVBoxLayout>

namespace jp {

QVector<write::DocVersion> WriteWindow::versions() const
{
    if (m_path.isEmpty()) return {};
    return write::VersionStore(m_path).list();
}

bool WriteWindow::saveVersion(const QString &name)
{
    if (!m_doc) return false;
    if (m_path.isEmpty()) {
        notice(tr("Version History"), tr("Save the document first. A version is kept for a file, under the file's name."), true);
        return false;
    }
    QString err;
    QByteArray bytes = m_doc->toDocx(&err);
    if (bytes.isEmpty()) {
        notice(tr("Version History"), tr("The version couldn't be kept.\n%1").arg(err), true);
        return false;
    }
    write::VersionStore store(m_path);
    if (!store.add(name.trimmed().isEmpty() ? tr("Version %1").arg(store.list().size() + 1) : name, bytes, &err)) {
        notice(tr("Version History"), tr("The version couldn't be kept.\n%1").arg(err), true);
        return false;
    }
    statusBar()->showMessage(tr("Kept a version of %1.").arg(displayName()), 4000);
    return true;
}

bool WriteWindow::restoreVersion(const QString &file)
{
    if (!m_doc || m_path.isEmpty()) return false;
    write::VersionStore store(m_path);
    const QByteArray bytes = store.read(file);
    QString err;
    std::unique_ptr<write::WriteDoc> doc;
    if (!bytes.isEmpty()) doc = write::WriteDoc::fromDocx(bytes, &err, nullptr);
    if (!doc) {
        notice(tr("Version History"), tr("That version can't be opened.\n%1").arg(err), true);
        return false;
    }
    QString which = file;
    for (const write::DocVersion &v : store.list())
        if (v.file == file) which = v.name;
    // What the document is now is kept first: going back loses nothing.
    if (!saveVersion(tr("Before restoring \"%1\"").arg(which))) return false;
    setDocument(std::move(doc), m_path);
    m_doc->setModified(true);
    statusBar()->showMessage(tr("Went back to \"%1\". Save the document to keep it.").arg(which), 8000);
    return true;
}

int WriteWindow::compareWithVersion(const QString &file)
{
    if (!m_doc || m_path.isEmpty()) return -1;
    const QByteArray bytes = write::VersionStore(m_path).read(file);
    if (bytes.isEmpty()) return -1;
    // (the compare reads a file: the version is put in a folder that goes with this call)
    QTemporaryDir dir;
    if (!dir.isValid()) return -1;
    const QString path = dir.filePath(QFileInfo(m_path).completeBaseName() + QStringLiteral(" (version).docx"));
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly) || f.write(bytes) != bytes.size()) return -1;
    f.close();
    write::CompareOptions options;
    options.author = Settings::get().userName();
    return compareWith(path, options);
}

void WriteWindow::versionHistory()
{
    if (!m_doc) return;
    if (m_path.isEmpty()) {
        notice(tr("Version History"), tr("Save the document first. A version is kept for a file, under the file's name."), true);
        return;
    }
    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("versionDialog"));
    dialog.setWindowTitle(tr("Version History"));
    dialog.resize(560, 380);
    auto *layout = new QVBoxLayout(&dialog);
    auto *intro = new QLabel(tr("Versions are copies of this document that are kept on this computer. Keep one before a big change. Later you can compare it with the document, or go back to it."), &dialog);
    intro->setWordWrap(true);
    layout->addWidget(intro);
    auto *list = new QListWidget(&dialog);
    list->setObjectName(QStringLiteral("versionList"));
    layout->addWidget(list, 1);
    auto *row = new QHBoxLayout();
    auto *keep = new QPushButton(tr("Keep a Version…"), &dialog);
    auto *open = new QPushButton(tr("Open a Copy"), &dialog);
    auto *compare = new QPushButton(tr("Compare"), &dialog);
    auto *restore = new QPushButton(tr("Go Back to It"), &dialog);
    auto *drop = new QPushButton(tr("Delete"), &dialog);
    for (QPushButton *b : {keep, open, compare, restore, drop}) row->addWidget(b);
    layout->addLayout(row);
    auto *close = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    connect(close, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(close);
    QDialog *dlg = &dialog;
    auto fill = [this, list] {
        list->clear();
        for (const write::DocVersion &v : versions()) {
            auto *it = new QListWidgetItem(tr("%1 (%2, %3 KB)").arg(v.name, QLocale().toString(v.saved, QLocale::ShortFormat)).arg(QLocale().toString(std::max<qint64>(1, v.size / 1024))));
            it->setData(Qt::UserRole, v.file);
            list->addItem(it);
        }
        if (list->count()) list->setCurrentRow(0);
    };
    auto chosen = [list] { return list->currentItem() ? list->currentItem()->data(Qt::UserRole).toString() : QString(); };
    auto sync = [=] { for (QPushButton *b : {open, compare, restore, drop}) b->setEnabled(list->currentItem() != nullptr); };
    connect(list, &QListWidget::currentRowChanged, dlg, sync);
    connect(keep, &QPushButton::clicked, dlg, [=, this] {
        bool ok = false;
        const QString name = QInputDialog::getText(dlg, tr("Keep a Version"), tr("Name for this version:"), QLineEdit::Normal, tr("Version %1").arg(versions().size() + 1), &ok);
        if (ok && saveVersion(name)) {
            fill();
            sync();
        }
    });
    connect(open, &QPushButton::clicked, dlg, [=, this] {
        const QString file = chosen();
        if (file.isEmpty()) return;
        dlg->accept();
        openFile(write::VersionStore(m_path).folder() + QLatin1Char('/') + file, OpenHow::Copy);
    });
    connect(compare, &QPushButton::clicked, dlg, [=, this] {
        const QString file = chosen();
        if (file.isEmpty()) return;
        dlg->accept();
        compareWithVersion(file);
    });
    connect(restore, &QPushButton::clicked, dlg, [=, this] {
        const QString file = chosen();
        if (file.isEmpty()) return;
        if (jp::plainQuestion(dlg, tr("Go Back"), tr("Go back to this version? The document as it is now is kept as a version first.")) != QMessageBox::Yes) return;
        dlg->accept();
        restoreVersion(file);
    });
    connect(drop, &QPushButton::clicked, dlg, [=, this] {
        const QString file = chosen();
        if (file.isEmpty()) return;
        write::VersionStore(m_path).remove(file);
        fill();
        sync();
    });
    fill();
    sync();
    dialog.exec();
}

} // namespace jp
