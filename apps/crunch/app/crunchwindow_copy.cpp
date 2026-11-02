// Saving a copy of the workbook as an .xls (the older binary form) or an .ods (OpenDocument) file. The copy is made from the same snapshot a save takes: the workbook is written as the
// .xlsx a save would write, on a thread of its own, that is read back as a workbook, and the copy is written from that (see sheet/oldsave.h). Before anything is put on disk the person is told
// what the file cannot hold or what was cut to fit, and may give it up or save an .xlsx instead. The workbook in the window is not changed, and is not marked saved: the copy is another file,
// as a .csv is, and the file the workbook was read from is never written.

#include "app/crunchwindow.h"

#include "app/oldformat.h"
#include "app/otherformats.h"
#include "app/telemetry.h"
#include "core/appinfo.h"
#include "core/loader.h"
#include "core/sheetobjects.h"
#include "sheet/oldsave.h"
#include "sheet/xlsxsave.h"

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
    const bool xls = crunch::copyKindOf(path) == QLatin1String("xls");
    m_saving = true;
    m_session->setBusy(true);
    updateEditActions();
    setStatus(tr("Saving %1…").arg(QFileInfo(path).fileName()));
    const std::shared_ptr<office::sheet::Workbook> book = m_loaded->book;
    const QPointer<CrunchWindow> self(this);
    QThread *thread = QThread::create([self, book, snapshot, path, xls] {
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
            if (ok) ok = xls ? office::sheet::saveXls(made, &copy, &copyReport, &message) : office::sheet::saveOds(made, &copy, &copyReport, &message);
        }
        QMetaObject::invokeMethod(qApp, [self, ok, message, path, copy, copyReport] {
            if (self) self->onCopyMade(ok, message, path, copy, copyReport.notKept, copyReport.trimmed);
        }, Qt::QueuedConnection);
    });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    m_saveThread = thread;
    thread->start();
    return true;
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
        const LossAnswer answer = askBeforeLosing(tr("Save"), warning, tr("Save Copy"), tr("Save as .xlsx…"));
        if (answer != LossAnswer::Continue) {
            if (answer == LossAnswer::SaveXlsx) {
                const bool closing = m_closeAfterSave;
                setStatus(tr("Not saved as .%1.").arg(crunch::copyKindOf(path)));
                act("file.saveAs")->trigger();
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
