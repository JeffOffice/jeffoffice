// Saving a copy of the workbook as an .xls (the older binary form) or an .ods (OpenDocument) file. The copy is made from the same snapshot a save takes: the workbook is written as the
// .xlsx a save would write, on a thread of its own, that is read back as a workbook, and the copy is written from that (see sheet/oldsave.h). Before anything is put on disk the person is told
// what the file cannot hold or what was cut to fit, and may give it up or save an .xlsx instead. The workbook in the window is not changed, and is not marked saved: the copy is another file,
// as a .csv is, and the file the workbook was read from is never written.

#include "app/crunchwindow.h"

#include "app/compatdialog.h"
#include "app/lossywarning.h"
#include "app/oldformat.h"
#include "app/otherformats.h"
#include "app/telemetry.h"
#include "core/appinfo.h"
#include "core/loader.h"
#include "core/sheetobjects.h"
#include "io/crypto.h"
#include "odt/odttemplate.h"
#include "office/odfcrypt.h"
#include "sheet/oldsave.h"
#include "sheet/xlsxsave.h"

#include <memory>

#include <QAction>
#include <QFileInfo>
#include <QPointer>
#include <QThread>

namespace jp {

bool CrunchWindow::exportCopy(const QString &path)
{
    if (!m_session || !m_loaded || !m_loaded->book || m_saving) return false;
    if (!commitEdit()) return false;
    if (m_loading || !m_haveCalc) {
        m_closeAfterSave = false;
        notice(tr("Save"), tr("The workbook is still being read. Save it when the status bar says Ready."));
        return false;
    }
    // the file the workbook was read from, and the file it was last saved to, are not written by a copy
    if (isTheOriginal(path) || crunch::sameFile(path, m_path)) {
        m_closeAfterSave = false;
        notice(tr("Save"), crunch::originalNotWrittenNotice(QFileInfo(isTheOriginal(path) ? m_originalPath : m_path).fileName()));
        return false;
    }
    // the copy has what the formulas are worth: a calculation that is going is waited for
    if (m_calculating) {
        m_saveAfterCalculation = path;
        setStatus(tr("Saving when the calculation is done…"));
        return true;
    }
    syncPanes();
    auto snapshot = std::make_shared<crunch::EditSession::Snapshot>();
    QString why;
    if (!m_session->takeSnapshot(snapshot.get(), &why, false)) {
        m_closeAfterSave = false;
        notice(tr("Save"), why);
        return false;
    }
    if (m_objects) m_objects->addToSave(&snapshot->changes);
    const bool xls = crunch::copyKindOf(path) == QLatin1String("xls"), ots = crunch::copyKindOf(path) == QLatin1String("ots");
    // (an .ods or .ots copy keeps the embedded objects of other kinds the file held; an .xls has no place for them)
    snapshot->changes.keepOdfObjects = !xls;
    snapshot->changes.keepOdfNotes = true;   // (what an .xlsx cannot hold of a note goes through it: the .ods copy has the date and the box as the file had them, and the .xls copy says which dates it loses)
    m_saving = true;
    m_session->setBusy(true);
    updateEditActions();
    setStatus(tr("Saving %1…").arg(QFileInfo(path).fileName()));
    const std::shared_ptr<office::sheet::Workbook> book = m_loaded->book;
    const QPointer<CrunchWindow> self(this);
    // (an .xls, .ods, or .ots copy of a workbook that has a password is protected with it, enciphered on the thread that makes the copy; the copy of the password goes with the thread and is wiped)
    const auto secret = std::make_shared<QString>(hasUserPassword() ? m_filePassword : QString());
    QThread *thread = QThread::create([self, book, snapshot, path, xls, ots, secret] {
        QByteArray xlsx, copy;
        QString message;
        office::sheet::SaveReport report;
        office::sheet::CopyReport copyReport;
        bool ok = office::sheet::saveXlsx(*book, snapshot->changes, &xlsx, &message, &report);
        if (ok) {
            // (what is written is the workbook as a save would write it, read back: the edits are in it, and the writers read one kind of model)
            office::Package package;
            office::sheet::Workbook made;
            ok = package.readLazy(xlsx, &message) && made.open(package, &message);
            if (ok) ok = xls ? office::sheet::saveXls(made, &copy, &copyReport, &message, nullptr, *secret) : office::sheet::saveOds(made, &copy, &copyReport, &message);
            if (ok && ots) copy = office::odt::odfAsTemplate(copy);
            if (ok && !xls && !secret->isEmpty()) {
                copy = office::encryptOdf(copy, *secret, office::kMaxOpenFileBytes);
                if (copy.isEmpty()) {
                    ok = false;
                    message = QCoreApplication::translate("jp::CrunchWindow", "The workbook could not be protected with the password.");
                }
            }
        }
        jp::crypto::wipe(*secret);
        QMetaObject::invokeMethod(qApp, [self, ok, message, path, copy, copyReport] {
            if (self) self->onCopyMade(ok, message, path, copy, copyReport.notKept, copyReport.trimmed);
        }, Qt::QueuedConnection);
    });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    m_saveThread = thread;
    thread->start();
    return true;
}

// What an .xls or an .ods copy would leave out of the workbook: the copy is made in memory as a save would make it (nothing is written), on another thread; `done` is called with the list.
void CrunchWindow::findCopyLosses(bool xls, const CompatDone &done)
{
    if (const QString why = cannotCheckNow(); !why.isEmpty()) {
        done(CompatAnswer::failed(why));
        return;
    }
    if (!commitEdit()) {
        done(CompatAnswer::failed(tr("The cell being edited could not be accepted, so the workbook could not be checked. Finish or cancel the edit, and try again.")));
        return;
    }
    syncPanes();
    auto snapshot = std::make_shared<crunch::EditSession::Snapshot>();
    QString why;
    if (!m_session->takeSnapshot(snapshot.get(), &why, false)) {
        done(CompatAnswer::failed(why));
        return;
    }
    if (m_objects) m_objects->addToSave(&snapshot->changes);
    snapshot->changes.keepOdfObjects = !xls;
    snapshot->changes.keepOdfNotes = true;
    setStatus(xls ? tr("Checking what an .xls file can hold…") : tr("Checking what an .ods file can hold…"));
    const std::shared_ptr<office::sheet::Workbook> book = m_loaded->book;
    const QPointer<CrunchWindow> self(this);
    QThread *thread = QThread::create([self, book, snapshot, xls, done] {
        QByteArray xlsx, copy;
        QString message;
        office::sheet::SaveReport report;
        office::sheet::CopyReport copyReport;
        bool ok = office::sheet::saveXlsx(*book, snapshot->changes, &xlsx, &message, &report);
        if (ok) {
            office::Package package;
            office::sheet::Workbook made;
            ok = package.readLazy(xlsx, &message) && made.open(package, &message);
            if (ok) ok = xls ? office::sheet::saveXls(made, &copy, &copyReport, &message) : office::sheet::saveOds(made, &copy, &copyReport, &message);
        }
        QMetaObject::invokeMethod(qApp, [self, ok, message, copyReport, done] {
            if (!self) return;
            self->setStatus(tr("Ready"));
            if (!ok) return done(CompatAnswer::failed(message));
            QStringList lost = copyReport.notKept;
            for (const QString &what : std::as_const(copyReport.trimmed)) lost << tr("Cut or changed to fit: %1").arg(what);
            done(lost);
        }, Qt::QueuedConnection);
    });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
}

// Why the workbook can't be checked just now, or empty when it can.
QString CrunchWindow::cannotCheckNow() const
{
    if (!m_session || !m_loaded || !m_loaded->book) return tr("There is no workbook to check.");
    if (m_saving) return tr("The workbook is being saved or copied right now. Check it when that is done.");
    if (m_loading || !m_haveCalc) return tr("The workbook is still being read. Check it when the status bar says Ready.");
    return QString();
}

// The kinds Check Compatibility offers: .xls and .ods (a copy made in memory), and .csv (the sheet in view).
jp::CompatHost CrunchWindow::compatHost()
{
    CompatHost host;
    host.markedFinal = [this] { return markedAsFinal(); };
    host.alwaysReadOnly = [this] { return alwaysOpensReadOnly(); };
    const QString what = tr("workbook");
    const QPointer<CrunchWindow> self(this);
    host.formats.push_back({tr("Older spreadsheet (.xls)"), QStringLiteral(".xls"), QStringLiteral(".xlsx"), what, [self](const CompatDone &done) {
        if (self) self->findCopyLosses(true, done);
    }});
    host.formats.push_back({tr("OpenDocument spreadsheet (.ods)"), QStringLiteral(".ods"), QStringLiteral(".xlsx"), what, [self](const CompatDone &done) {
        if (self) self->findCopyLosses(false, done);
    }});
    host.formats.push_back({tr("Text with commas (.csv)"), QStringLiteral(".csv"), QStringLiteral(".xlsx"), what, [self](const CompatDone &done) {
        if (!self) return;
        if (const QString why = self->cannotCheckNow(); !why.isEmpty()) return done(CompatAnswer::failed(why));
        done(self->csvFound());
    }});
    return host;
}

void CrunchWindow::checkCompatibility(int format)
{
    std::function<void(const QString &, const QString &, bool)> say;
    if (m_notifier) say = [this](const QString &title, const QString &text, bool warn) { notice(title, text, warn); };
    showCompatibility(this, compatHost(), format, say);
}

void CrunchWindow::onCopyMade(bool ok, const QString &message, const QString &path, const QByteArray &bytes, const QStringList &notKept, const QStringList &trimmed)
{
    m_saving = false;
    if (m_session) m_session->setBusy(false);
    updateEditActions();
    auto fail = [&](const QString &text) {
        m_closeAfterSave = false;
        notice(tr("Save"), tr("%1 couldn't save \"%2\".\n%3\nThe file that was there is as it was.").arg(appInfo().name, QFileInfo(path).fileName(), text));
        setStatus(tr("Ready"));
        Q_EMIT copyExported(path, false);
    };
    if (!ok) {
        fail(message);
        return;
    }
    // what the file cannot hold: the person may go on, save an .xlsx instead, or give it up
    const QString warning = crunch::copyWarning(crunch::copyKindOf(path), notKept, trimmed);
    if (!warning.isEmpty()) {
        // (the warning every app gives, libs/shell/app/lossywarning.h; a test stands in for the older question below, as before)
        LossAnswer answer = LossAnswer::Continue;
        if (m_lossAsker) {
            answer = askBeforeLosing(tr("Save"), warning, tr("Save Copy"), tr("Save as .xlsx…"));
        } else {
            QStringList lost = notKept;
            for (const QString &what : trimmed) lost << tr("Cut or changed to fit: %1").arg(what);
            const LossyChoice choice = askLossy(this, {path, tr("workbook"), QStringLiteral(".") + crunch::copyKindOf(path), QStringLiteral(".xlsx"), lost});
            answer = choice == LossyChoice::SaveAnyway ? LossAnswer::Continue : choice == LossyChoice::SaveOwnFormat ? LossAnswer::SaveXlsx : LossAnswer::Cancel;
        }
        if (answer != LossAnswer::Continue) {
            if (answer == LossAnswer::SaveXlsx) {
                const bool closing = m_closeAfterSave;
                setStatus(tr("Not saved as .%1.").arg(crunch::copyKindOf(path)));
                saveAs(lossyOwnFormatPath(path, QStringLiteral(".xlsx")));   // (beside the file, as the other apps do)
                m_closeAfterSave = closing && (m_saving || !m_saveAfterCalculation.isNull());
            } else {
                m_closeAfterSave = false;
                setStatus(tr("Ready"));
            }
            Q_EMIT copyExported(path, false);
            return;
        }
    }
    QString why;
    if (!office::sheet::writeFileAtomically(path, bytes, &why)) {
        fail(why);
        return;
    }
    telemetry::count(QStringLiteral("file.save.") + crunch::copyKindOf(path));
    setStatus(tr("Saved a copy as %1").arg(QFileInfo(path).fileName()));
    Q_EMIT copyExported(path, true);
    // (the window was being closed: a copy written from that question was the answer to it, and the window closes)
    if (m_closeAfterSave) {
        m_closeAfterSave = false;
        m_closing = true;
        close();
        m_closing = false;
    }
}

} // namespace jp
