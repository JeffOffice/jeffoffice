// Version History (File > Info): named copies of the document kept on this computer, to compare with and to go back to. The copies are kept by app/versionstore.cpp.

#include "app/writewindow.h"

#include "app/pageview.h"
#include "app/plaintext.h"
#include "app/settings.h"
#include "app/versionhistory.h"
#include "office/officecrypt.h"
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
    // (a version of a document that asks for a password asks for it too)
    const QString kept = m_versions.forNew(m_fileProtected ? m_filePassword : QString());
    if (!kept.isEmpty()) bytes = office::encryptPackage(bytes, kept);
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
    QByteArray bytes;
    QString err;
    std::unique_ptr<write::WriteDoc> doc;
    if (m_versions.open(store.read(file), m_fileProtected ? m_filePassword : QString(), &bytes)) doc = write::WriteDoc::fromDocx(bytes, &err, nullptr);
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
    QByteArray bytes;
    if (!m_versions.open(write::VersionStore(m_path).read(file), m_fileProtected ? m_filePassword : QString(), &bytes)) return -1;
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
        notice(tr("Version History"), versionsNeedAFile(VersionHistoryHost::Kind::Document), true);
        return;
    }
    VersionHistoryHost host;
    host.kind = VersionHistoryHost::Kind::Document;
    host.path = m_path;
    host.extension = QStringLiteral(".docx");
    host.keep = [this](const QString &name, QObject *, const std::function<void()> &refresh) {
        if (!saveVersion(name)) return false;
        refresh();
        return true;
    };
    host.openCopy = [this](const QString &copy) { openFile(copy, OpenHow::Copy); };
    host.compare = [this](const QString &file) { compareWithVersion(file); };
    host.restore = [this](const QString &file) { return restoreVersion(file); };
    showVersionHistory(this, host);
}

} // namespace jp
