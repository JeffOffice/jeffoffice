// Version History (File > Info) for a workbook: copies kept on this computer, to open beside the workbook and to go back to. The copies are kept by libs/shell/app/versionstore.cpp.

#include "app/crunchwindow.h"

#include "app/plaintext.h"
#include "app/otherformats.h"
#include "core/appinfo.h"
#include "core/sheetobjects.h"
#include "sheet/oldsave.h"
#include "sheet/xlsxsave.h"

#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QLocale>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QSaveFile>
#include <QStandardPaths>
#include <QStatusBar>
#include <QThread>
#include <QVBoxLayout>

namespace jp {

namespace {
const QString kVersionEnding = QStringLiteral(".xlsx");
}

QVector<write::DocVersion> CrunchWindow::versions() const
{
    if (m_path.isEmpty()) return {};
    return write::VersionStore(m_path, kVersionEnding).list();
}

bool CrunchWindow::saveVersion(const QString &name)
{
    if (!m_session || !m_loaded || !m_loaded->book || m_saving || !commitEdit()) return false;
    if (m_path.isEmpty()) {
        notice(tr("Version History"), tr("Save the workbook first. A version is kept for a file, under the file's name."), true);
        return false;
    }
    if (m_loading || !m_haveCalc) {
        notice(tr("Version History"), tr("The workbook is still being read. Keep a version when the status bar says Ready."), true);
        return false;
    }
    syncPanes();
    auto snapshot = std::make_shared<crunch::EditSession::Snapshot>();
    QString why;
    if (!m_session->takeSnapshot(snapshot.get(), &why, false)) {
        notice(tr("Version History"), why, true);
        return false;
    }
    if (m_objects) m_objects->addToSave(&snapshot->changes);
    const QString path = m_path;
    write::VersionStore probe(path, kVersionEnding);
    const QString label = name.trimmed().isEmpty() ? tr("Version %1").arg(probe.list().size() + 1) : name;
    setStatus(tr("Keeping a version…"));
    const std::shared_ptr<office::sheet::Workbook> book = m_loaded->book;
    const QPointer<CrunchWindow> self(this);
    QThread *thread = QThread::create([self, book, snapshot, path, label] {
        QByteArray bytes;
        QString message;
        office::sheet::SaveReport report;
        bool ok = office::sheet::saveXlsx(*book, snapshot->changes, &bytes, &message, &report);
        if (ok) ok = write::VersionStore(path, kVersionEnding).add(label, bytes, &message);
        QMetaObject::invokeMethod(qApp, [self, ok, message] {
            if (!self) return;
            self->setStatus(ok ? tr("Kept a version of %1.").arg(self->displayName()) : tr("Ready"));
            if (!ok) self->notice(tr("Version History"), tr("The version couldn't be kept.\n%1").arg(message), true);
            Q_EMIT self->versionKept(ok);
        }, Qt::QueuedConnection);
    });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
    return true;
}

bool CrunchWindow::restoreVersion(const QString &file)
{
    if (!m_session || m_path.isEmpty() || m_saving) return false;
    // (the version is put in the workbook's own file, so that is a file of the usual kind)
    if (QFileInfo(m_path).suffix().compare(QLatin1String("xlsx"), Qt::CaseInsensitive) != 0 || savesAsANewFile()) {
        notice(tr("Version History"), tr("Going back works for a workbook saved as an .xlsx file. Open the version as a copy instead."), true);
        return false;
    }
    write::VersionStore store(m_path, kVersionEnding);
    const QByteArray bytes = store.read(file);
    if (bytes.isEmpty()) {
        notice(tr("Version History"), tr("That version can't be opened."), true);
        return false;
    }
    QString which = file;
    for (const write::DocVersion &v : store.list())
        if (v.file == file) which = v.name;
    const QString path = m_path;
    // What the workbook is now is kept first: going back loses nothing.
    auto link = std::make_shared<QMetaObject::Connection>();
    *link = connect(this, &CrunchWindow::versionKept, this, [this, link, bytes, path, which](bool ok) {
        disconnect(*link);
        if (!ok) return;
        QString why;
        if (!office::sheet::writeFileAtomically(path, bytes, &why)) {
            notice(tr("Version History"), tr("The version couldn't be put back.\n%1").arg(why), true);
            return;
        }
        m_reopening = true;
        openFile(path, OpenHow::Normal);
        m_reopening = false;
        setStatus(tr("Went back to \"%1\".").arg(which));
    });
    if (!saveVersion(tr("Before restoring \"%1\"").arg(which))) {
        disconnect(*link);
        return false;
    }
    return true;
}

bool CrunchWindow::openVersionCopy(const QString &file)
{
    if (m_path.isEmpty()) return false;
    const QByteArray bytes = write::VersionStore(m_path, kVersionEnding).read(file);
    if (bytes.isEmpty()) return false;
    // (the copy is read from a file that stays while its window is open: one of the program's own, older ones go)
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QStringLiteral("/versions");
    QDir().mkpath(dir);
    for (const QFileInfo &old : QDir(dir).entryInfoList(QDir::Files))
        if (old.lastModified().daysTo(QDateTime::currentDateTime()) > 7) QFile::remove(old.absoluteFilePath());
    const QString path = QStringLiteral("%1/%2 (version %3).xlsx").arg(dir, QFileInfo(m_path).completeBaseName(), QString::number(QDateTime::currentMSecsSinceEpoch()));
    QSaveFile out(path);
    if (!out.open(QIODevice::WriteOnly) || out.write(bytes) != bytes.size() || !out.commit()) return false;
    return openFile(path, OpenHow::Copy);
}

void CrunchWindow::versionHistory()
{
    if (!m_session) return;
    if (m_path.isEmpty()) {
        notice(tr("Version History"), tr("Save the workbook first. A version is kept for a file, under the file's name."), true);
        return;
    }
    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("versionDialog"));
    dialog.setWindowTitle(tr("Version History"));
    dialog.resize(560, 380);
    auto *layout = new QVBoxLayout(&dialog);
    auto *intro = new QLabel(tr("Versions are copies of this workbook that are kept on this computer. Keep one before a big change. Later you can open it beside the workbook, or go back to it."), &dialog);
    intro->setWordWrap(true);
    intro->setTextFormat(Qt::PlainText);
    layout->addWidget(intro);
    auto *list = new QListWidget(&dialog);
    list->setObjectName(QStringLiteral("versionList"));
    layout->addWidget(list, 1);
    auto *row = new QHBoxLayout();
    auto *keep = new QPushButton(tr("Keep a Version…"), &dialog);
    auto *open = new QPushButton(tr("Open a Copy"), &dialog);
    auto *restore = new QPushButton(tr("Go Back to It"), &dialog);
    auto *drop = new QPushButton(tr("Delete"), &dialog);
    for (QPushButton *b : {keep, open, restore, drop}) row->addWidget(b);
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
    auto sync = [=] { for (QPushButton *b : {open, restore, drop}) b->setEnabled(list->currentItem() != nullptr); };
    connect(list, &QListWidget::currentRowChanged, dlg, sync);
    connect(keep, &QPushButton::clicked, dlg, [=, this] {
        bool ok = false;
        const QString name = QInputDialog::getText(dlg, tr("Keep a Version"), tr("Name for this version:"), QLineEdit::Normal, tr("Version %1").arg(versions().size() + 1), &ok);
        if (!ok) return;
        auto link = std::make_shared<QMetaObject::Connection>();
        *link = connect(this, &CrunchWindow::versionKept, dlg, [=](bool) {
            disconnect(*link);
            fill();
            sync();
        });
        if (!saveVersion(name)) disconnect(*link);
    });
    connect(open, &QPushButton::clicked, dlg, [=, this] {
        const QString file = chosen();
        if (file.isEmpty()) return;
        dlg->accept();
        openVersionCopy(file);
    });
    connect(restore, &QPushButton::clicked, dlg, [=, this] {
        const QString file = chosen();
        if (file.isEmpty()) return;
        if (jp::plainQuestion(dlg, tr("Go Back"), tr("Go back to this version? The workbook as it is now is kept as a version first.")) != QMessageBox::Yes) return;
        dlg->accept();
        restoreVersion(file);
    });
    connect(drop, &QPushButton::clicked, dlg, [=, this] {
        const QString file = chosen();
        if (file.isEmpty()) return;
        write::VersionStore(m_path, kVersionEnding).remove(file);
        fill();
        sync();
    });
    fill();
    sync();
    dialog.exec();
}

} // namespace jp
