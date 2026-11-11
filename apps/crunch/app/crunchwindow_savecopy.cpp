// Save a Copy in JeffCrunch's window: the workbook is written to another file as Save As would write it, and the window goes on with the original, which keeps its name and the changes
// it has not saved (the session is not told it was saved). A .csv, .xls, or .ods copy is made as those always are (see crunchwindow_copy.cpp and crunchwindow_csv.cpp).

#include "app/crunchwindow.h"

#include "app/filetasks.h"
#include "app/oldformat.h"
#include "app/otherformats.h"
#include "app/settings.h"
#include "app/telemetry.h"
#include "core/appinfo.h"
#include "core/loader.h"
#include "core/sheetobjects.h"
#include "io/crypto.h"
#include "office/officecrypt.h"
#include "app/privacy.h"
#include "office/privacy.h"
#include "sheet/xlsxsave.h"

#include <QAction>
#include <QApplication>
#include <QFileInfo>
#include <QPointer>
#include <QStandardPaths>
#include <utility>
#include <QThread>

namespace jp {

bool CrunchWindow::saveCopy()
{
    if (!m_loaded || !m_session || m_saving || !commitEdit()) return false;
    const QString kind = saveKind();
    const QString dir = m_path.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) : QFileInfo(m_path).absolutePath();
    const QString name = tr("%1 copy").arg(m_path.isEmpty() ? displayName() : QFileInfo(m_path).completeBaseName());
    const QString start = dir + QLatin1Char('/') + name + QLatin1Char('.') + kind;
    const QString filter = crunch::workbookKindFilter(kind) + QStringLiteral(";;") + crunch::copyFilters() + QStringLiteral(";;") + crunch::csvFilter() + QStringLiteral(";;") + crunch::tabFilter();
    const QString p = askForSavePath(tr("Save a Copy"), start, filter);
    return !p.isEmpty() && saveAs(p, true);
}

bool CrunchWindow::saveAsTemplate()
{
    if (!m_loaded || !m_session || m_saving || !commitEdit()) return false;
    const QString path = askMyTemplatePath(this, displayName(), QStringLiteral(".xltx"));
    if (path.isEmpty()) return false;
    // A template is a plain one (.xltx): a workbook with macros in it is made one without them, and the person is told first, as Save As tells.
    if (crunch::workbookKindHoldsMacros(saveKind()) && !m_loaded->book->package().relationshipsOfType(m_loaded->book->mainPart(), QStringLiteral("/vbaProject")).isEmpty()) {
        const LossAnswer answer = askBeforeLosing(tr("Save"), tr("A .xltx file can't hold macros. The macros of this workbook will not be in the copy you are saving. The workbook open here keeps them."),
                                                  tr("Save Without Macros"), QString());
        if (answer != LossAnswer::Continue) return false;
    }
    // (the picture of the sheet in view is kept beside the template, once the template is written)
    const QImage picture = m_view->grab().toImage().scaledToWidth(240, Qt::SmoothTransformation);
    auto *once = new QObject(this);
    connect(this, &CrunchWindow::copyExported, once, [this, once, path, picture](const QString &made, bool ok) {
        if (!crunch::sameFile(made, path)) return;
        once->deleteLater();
        if (!ok) return;
        saveTemplateThumbnail(path, picture);
        setStatus(tr("Saved \"%1\" in My Templates.").arg(QFileInfo(path).completeBaseName()));
    });
    m_copyAsKind = QStringLiteral("xltx");
    if (startCopy(path)) return true;
    m_copyAsKind.clear();
    delete once;
    return false;
}

bool CrunchWindow::startCopy(const QString &path)
{
    if (!m_session || !m_loaded || !m_loaded->book || m_saving) return false;
    if (!commitEdit()) return false;
    if (m_loading || !m_haveCalc) {
        notice(tr("Save"), tr("The workbook is still being read. Save a copy when the status bar says Ready."));
        return false;
    }
    // the workbook's own file, and the file it was read from, are not written by a copy
    if (isTheOriginal(path) || crunch::sameFile(path, m_path)) {
        notice(tr("Save"), tr("A copy can't be saved over \"%1\". Choose another name.").arg(QFileInfo(path).fileName()));
        return false;
    }
    // the copy has what the formulas are worth: a calculation that is going is waited for
    if (m_calculating) {
        m_saveAfterCalculation = path;
        m_copyAfterCalculation = true;
        setStatus(tr("Saving a copy when the calculation is done…"));
        return true;
    }
    syncPanes();
    auto snapshot = std::make_shared<crunch::EditSession::Snapshot>();
    QString why;
    if (!m_session->takeSnapshot(snapshot.get(), &why)) {
        notice(tr("Save"), why);
        return false;
    }
    {
        const QString ending = QFileInfo(path).suffix().toLower();
        const QString forced = std::exchange(m_copyAsKind, QString());
        if (!forced.isEmpty() && forced != saveKind()) snapshot->changes.targetKind = forced;
        else if (ending != saveKind() && crunch::workbookKindsSavedAs(saveKind()).contains(ending)) snapshot->changes.targetKind = ending;
    }
    if (m_objects) m_objects->addToSave(&snapshot->changes);
    if (!backupBeforeSave(this, path, hasUserPassword() ? m_filePassword : QString())) return false;
    m_saving = true;
    m_session->setBusy(true);
    updateEditActions();
    setStatus(tr("Saving a copy as %1…").arg(QFileInfo(path).fileName()));
    const std::shared_ptr<office::sheet::Workbook> book = m_loaded->book;
    const QPointer<CrunchWindow> self(this);
    // (a protected workbook's copy is protected with the same password)
    const bool protect = m_fileProtected;
    const auto secret = std::make_shared<QString>(m_filePassword.isEmpty() ? office::defaultPassword() : m_filePassword);
    const bool namesOut = removePersonalOnSave();   // (Options > Privacy: a copy has no names either)
    const auto namesProblems = std::make_shared<QStringList>();
    QThread *thread = QThread::create([self, book, snapshot, path, protect, secret, namesOut, namesProblems] {
        QByteArray bytes;
        QString message;
        office::sheet::SaveReport report;
        bool ok = office::sheet::saveXlsx(*book, snapshot->changes, &bytes, &message, &report);
        if (ok && namesOut) bytes = office::removePersonalFromPackage(bytes, nullptr, namesProblems.get());
        if (ok) {
            QByteArray file = bytes;
            if (protect) {
                file = office::encryptPackage(bytes, *secret);
                if (file.isEmpty()) {
                    ok = false;
                    message = QCoreApplication::translate("jp::CrunchWindow", "The workbook could not be protected with the password.");
                }
            }
            if (ok) ok = office::sheet::writeFileAtomically(path, file, &message);
        }
        jp::crypto::wipe(*secret);
        QMetaObject::invokeMethod(qApp, [self, ok, message, path, namesProblems] {
            if (!self) return;
            self->onWorkbookCopyMade(ok, message, path);
            if (ok && !namesProblems->isEmpty()) self->notice(self->tr("Save"), privacy::notTakenOutMessage(*namesProblems), true);
        }, Qt::QueuedConnection);
    });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    m_saveThread = thread;
    thread->start();
    return true;
}

void CrunchWindow::onWorkbookCopyMade(bool ok, const QString &message, const QString &path)
{
    m_saving = false;
    if (m_session) m_session->setBusy(false);
    updateEditActions();
    if (ok) {
        Settings::get().addRecentFile(path);
        telemetry::count(QStringLiteral("file.save.") + crunch::fileKind(path));
        setStatus(tr("Saved a copy as %1").arg(QFileInfo(path).fileName()));
    } else {
        notice(tr("Save"), tr("%1 couldn't save \"%2\".\n%3\nThe file that was there is as it was.").arg(appInfo().name, QFileInfo(path).fileName(), message));
        setStatus(tr("Ready"));
    }
    Q_EMIT copyExported(path, ok);
}

} // namespace jp
