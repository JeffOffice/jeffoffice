// Saving in JeffCrunch's window: Save, Save As, and the question when a window with changes that are not saved is closed. The file is made on a thread of its own from what the
// EditSession took when the save began (the window can be looked at and scrolled meanwhile, but nothing is changed until it is done), and is written to a file of its own that then takes the
// place of the old one, so a save that fails leaves the old file as it was.

#include "app/crunchwindow.h"
#include "app/plaintext.h"

#include "app/oldformat.h"
#include "app/otherformats.h"
#include "app/settings.h"
#include "app/telemetry.h"
#include "core/sheetobjects.h"
#include "core/appinfo.h"
#include "core/loader.h"
#include "sheet/xlsxsave.h"

#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>
#include <QPointer>
#include <QThread>

#include <cstring>

namespace jp {

using jp::office::sheet::SaveReport;

// The kind of file this workbook is, from the type of its main part, which is in the file: a workbook (.xlsx), one with macros in it (.xlsm), a template (.xltx), a template with macros
// (.xltm), or an add-in (.xlam).
QString CrunchWindow::saveKind() const
{
    if (!m_loaded || !m_loaded->book) return QStringLiteral("xlsx");
    const office::Package &p = m_loaded->book->package();
    return crunch::workbookKindOfType(p.contentType(m_loaded->book->mainPart()));
}

bool CrunchWindow::save()
{
    if (!m_session || !m_loaded) return false;
    if (m_path.isEmpty()) {
        act("file.saveAs")->trigger();
        return m_saving || !m_saveAfterCalculation.isNull();   // (a new workbook has no file to save to: Save asks for one, as Save As does)
    }
    // A workbook read from the older binary form is saved under a new name, as .xlsx: the person is told, and asked for the name (the file it came from is never written).
    if (savesAsANewFile()) {
        if (!commitEdit()) return false;
        const QString path = pathToSave();
        return !path.isEmpty() && saveAs(path);
    }
    return startSave(m_path);
}

bool CrunchWindow::snapshotBytesForTest(QByteArray *bytes, QString *why)
{
    if (!m_session || !m_loaded) return false;
    crunch::EditSession::Snapshot snapshot;
    if (!m_session->takeSnapshot(&snapshot, why)) return false;
    if (m_objects) m_objects->addToSave(&snapshot.changes);
    SaveReport report;
    return office::sheet::saveXlsx(*m_loaded->book, snapshot.changes, bytes, why, &report);
}

bool CrunchWindow::saveAs(const QString &pathIn)
{
    if (!m_session || !m_loaded) return false;
    if (crunch::isCsvName(pathIn)) return startSave(pathIn);
    QString path = pathIn;
    const QString kind = saveKind();
    const QString ending = QFileInfo(path).suffix().toLower();
    if (ending.isEmpty()) path += QLatin1Char('.') + kind;
    else if (!crunch::workbookKindsSavedAs(kind).contains(ending)) {
        notice(tr("Save"), crunch::workbookKindNotice(kind, appInfo().name));
        return false;
    } else if (crunch::workbookKindHoldsMacros(kind) && !crunch::workbookKindHoldsMacros(ending)
               && !m_loaded->book->package().relationshipsOfType(m_loaded->book->mainPart(), QStringLiteral("/vbaProject")).isEmpty()) {
        // (the kind chosen holds no macros: the person is told that the copy will not have them, before anything is written)
        const LossAnswer answer = askBeforeLosing(tr("Save"), tr("A .%1 file can't hold macros. The macros of this workbook will not be in the copy you are saving. The workbook open here keeps them.").arg(ending),
                                                  tr("Save Without Macros"), QString());
        if (answer != LossAnswer::Continue) {
            m_closeAfterSave = false;
            return false;
        }
    }
    return startSave(path);
}

// Whether the file the workbook was read from (or last written to) is not as this program has it: its bytes are compared with the ones it holds (a file that is only touched, or
// copied over itself, is not changed). A file that is gone is not: saving makes it again.
bool CrunchWindow::fileChangedOnDisk() const
{
    if (!m_loaded || !m_loaded->book || m_path.isEmpty()) return false;
    const QByteArray mine = m_loaded->book->package().fileBytes();
    QFile f(m_path);
    if (!f.exists()) return false;
    if (f.size() != mine.size()) return true;
    if (!f.open(QIODevice::ReadOnly)) return true;
    constexpr qint64 kChunk = 1 << 20;
    QByteArray chunk;
    for (qint64 at = 0; at < mine.size(); at += kChunk) {
        const qint64 n = std::min<qint64>(kChunk, mine.size() - at);
        chunk = f.read(n);
        if (chunk.size() != n || std::memcmp(chunk.constData(), mine.constData() + at, size_t(n)) != 0) return true;
    }
    return false;
}

bool CrunchWindow::startSave(const QString &path)
{
    if (!m_session || m_saving) return false;
    if (crunch::isCsvName(path)) return exportCsv(path);
    if (!commitEdit()) return false;
    // a workbook read from a file of another form is never written over the file it came from, whatever that file is called, and for as long as the window has the workbook (after the first
    // save the window's file is the .xlsx, and the original is still not its own)
    if (isTheOriginal(path)) {
        notice(tr("Save"), crunch::originalNotWrittenNotice(QFileInfo(m_originalPath).fileName()));
        return false;
    }
    const bool samePath = crunch::sameFile(path, m_path);
    // nothing to save over the file it was read from
    if (samePath && !m_session->isModified() && !m_session->viewsChanged()) {
        setStatus(tr("Nothing to save: %1 is as it was.").arg(QFileInfo(path).fileName()));
        Q_EMIT saved(path, true);
        return true;
    }
    // the file has what the formulas are worth: a calculation that is going is waited for
    if (m_calculating && m_session->isModified()) {
        m_saveAfterCalculation = path;
        setStatus(tr("Saving when the calculation is done…"));
        return true;
    }
    // The new file is the one that was read with what changed put in: if another program changed the file since, saving over it would take that work out. The person is asked.
    if (samePath && fileChangedOnDisk()) {
        const QString name = QFileInfo(path).fileName();
        ChangedAnswer answer = ChangedAnswer::Cancel;
        if (m_changedAsker) {
            answer = m_changedAsker(name);
        } else {
            QMessageBox box(QMessageBox::Warning, appInfo().name,
                            tr("Another program changed \"%1\" after you opened it. If you save here, what it changed is lost.").arg(name), QMessageBox::NoButton, this);
            box.setTextFormat(Qt::PlainText);
            box.setInformativeText(tr("You can save your work as a copy and keep both, or save over the file."));
            QPushButton *copyButton = box.addButton(tr("Save a Copy…"), QMessageBox::AcceptRole);
            QPushButton *overwriteButton = box.addButton(tr("Overwrite"), QMessageBox::DestructiveRole);
            box.addButton(tr("Cancel"), QMessageBox::RejectRole);
            box.setDefaultButton(copyButton);
            box.exec();
            QAbstractButton *clicked = box.clickedButton();
            answer = clicked == copyButton ? ChangedAnswer::SaveCopy : clicked == overwriteButton ? ChangedAnswer::Overwrite : ChangedAnswer::Cancel;
        }
        if (answer != ChangedAnswer::Overwrite) {
            m_closeAfterSave = false;
            if (answer == ChangedAnswer::SaveCopy) {
                act("file.saveAs")->trigger();
            } else {
                setStatus(tr("Not saved: %1 was changed by another program.").arg(name));
            }
            return false;
        }
    }
    auto snapshot = std::make_shared<crunch::EditSession::Snapshot>();
    QString why;
    if (!m_session->takeSnapshot(snapshot.get(), &why)) {
        notice(tr("Save"), why);
        return false;
    }
    // (the kind of the file follows the ending chosen: a template may be saved as a workbook, and an add-in or a macro template as a workbook with macros)
    {
        const QString ending = QFileInfo(path).suffix().toLower();
        if (ending != saveKind() && crunch::workbookKindsSavedAs(saveKind()).contains(ending)) snapshot->changes.targetKind = ending;
    }
    if (m_objects) m_objects->addToSave(&snapshot->changes);
    m_saving = true;
    m_session->setBusy(true);
    updateEditActions();
    setStatus(tr("Saving %1…").arg(QFileInfo(path).fileName()));
    const std::shared_ptr<office::sheet::Workbook> book = m_loaded->book;
    const QPointer<CrunchWindow> self(this);
    QThread *thread = QThread::create([self, book, snapshot, path] {
        QByteArray bytes;
        QString message;
        SaveReport report;
        bool ok = office::sheet::saveXlsx(*book, snapshot->changes, &bytes, &message, &report);
        snapshot->headerDrawings = report.headerDrawings;   // (read by the window after this thread has told it the save is over)
        if (ok) ok = office::sheet::writeFileAtomically(path, bytes, &message);
        QMetaObject::invokeMethod(qApp, [self, ok, message, path, bytes, snapshot] {
            if (self) self->onSaved(ok, message, path, bytes, snapshot);
        }, Qt::QueuedConnection);
    });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    m_saveThread = thread;
    thread->start();
    return true;
}

void CrunchWindow::onSaved(bool ok, const QString &message, const QString &path, const QByteArray &bytes, const std::shared_ptr<crunch::EditSession::Snapshot> &snapshot)
{
    m_saving = false;
    m_session->setBusy(false);
    if (ok) {
        office::Package package;
        QString why;
        int forgotten = 0;   // the steps of Undo this save made it give up
        if (!package.readLazy(bytes, &why)) {
            // (the file is written; this program cannot read what it wrote: it keeps the old base, and says so)
            notice(tr("Save"), tr("%1 saved \"%2\", but could not read it back: %3").arg(appInfo().name, QFileInfo(path).fileName(), why));
        } else {
            m_session->saved(*snapshot, package);
            if (m_objects) m_objects->saved();
            forgotten = m_session->stepsForgottenBySave();
        }
        if (!crunch::sameFile(path, m_path)) {
            m_path = path;
            m_blank = false;
            updateTitle();
        }
        Settings::get().addRecentFile(path);
        telemetry::count(QStringLiteral("file.save.") + crunch::fileKind(path));
        // (a save is the point after which what it saved of rows, columns, or cells deleted is not given back by Undo: that is said)
        if (forgotten > 0) setStatus(tr("Saved %1. Rows, columns, and cells deleted before this save can't be undone now, nor can what you did before that.").arg(QFileInfo(path).fileName()));
        else setStatus(tr("Saved %1").arg(QFileInfo(path).fileName()));
    } else {
        m_closeAfterSave = false;
        notice(tr("Save"), tr("%1 couldn't save \"%2\".\n%3\nThe file that was there is as it was.").arg(appInfo().name, QFileInfo(path).fileName(), message));
        setStatus(tr("Ready"));
    }
    updateEditActions();
    Q_EMIT saved(path, ok);
    if (ok && m_closeAfterSave) {
        m_closeAfterSave = false;
        close();
    }
}

// Whether the window may be closed: the changes that are not saved are asked about. Save saves them first (and the window closes when that is done).
bool CrunchWindow::askAboutChanges()
{
    if (!m_session || (!m_session->isModified() && !m_saving)) return true;
    // (the person chose to write the values of the sheet as a .csv from this question: that was the answer, and the window closes when it is written)
    if (m_closing) return true;
    if (m_saving) {
        m_closeAfterSave = true;
        return false;
    }
    const QString name = m_path.isEmpty() ? displayName() : QFileInfo(m_path).fileName();
    const QString title = appInfo().name;
    const QString text = tr("Do you want to save the changes you made to \"%1\"?").arg(name);
    Answer answer = Answer::Cancel;
    if (m_asker) {
        answer = m_asker(title, text);
    } else {
        QMessageBox box(QMessageBox::Warning, title, text, QMessageBox::NoButton, this);
        box.setTextFormat(Qt::PlainText);
        QPushButton *saveButton = box.addButton(tr("Save"), QMessageBox::AcceptRole);
        QPushButton *discardButton = box.addButton(tr("Don't Save"), QMessageBox::DestructiveRole);
        box.addButton(tr("Cancel"), QMessageBox::RejectRole);
        box.setDefaultButton(saveButton);
        box.exec();
        QAbstractButton *clicked = box.clickedButton();
        answer = clicked == saveButton ? Answer::Save : clicked == discardButton ? Answer::Discard : Answer::Cancel;
    }
    if (answer == Answer::Discard) return true;
    if (answer == Answer::Save) {
        m_closeAfterSave = true;
        // (a save that says no may still have handed the person on to another one, as the .csv question does; the close waits for that, and is given up if none was started)
        if (!save() && !m_saving && m_saveAfterCalculation.isNull()) m_closeAfterSave = false;
    }
    return false;
}

} // namespace jp
