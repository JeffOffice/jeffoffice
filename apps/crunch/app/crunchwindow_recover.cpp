// Automatic recovery in JeffCrunch's window: a copy of a workbook with changes that are not saved, made in the background at the interval Options > Save sets, kept in the folder of this run
// (libs/shell recovery) in the file form the workbook is saved in; the copy goes once the work is saved or the window is closed, and copies a crashed run left are offered at the next start.

#include "app/crunchwindow.h"

#include "app/plaintext.h"
#include "app/icons.h"
#include "app/recovery.h"
#include "app/settings.h"
#include "core/appinfo.h"
#include "core/sheetobjects.h"
#include "sheet/xlsxsave.h"

#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QElapsedTimer>
#include <QLabel>
#include <QListWidget>
#include <QLocale>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QThread>
#include <atomic>
#include <array>
#include <QVBoxLayout>

namespace jp {

void CrunchWindow::createRecovery()
{
    static quint64 s_windows = 0;
    m_serial = ++s_windows;
    m_recoverTimer.setInterval(std::max(1, Settings::get().autoRecoverMinutes()) * 60 * 1000);
    connect(&m_recoverTimer, &QTimer::timeout, this, &CrunchWindow::autoRecover);
    m_recoverTimer.start();
}

void CrunchWindow::autoRecover()
{
    // Options may have changed the interval since the timer started.
    const int ms = std::max(1, Settings::get().autoRecoverMinutes()) * 60 * 1000;
    if (m_recoverTimer.interval() != ms) m_recoverTimer.start(ms);
    if (!m_session || !m_loaded || m_loading) return;
    // A copy is only for work that is not saved: it goes when the workbook is as its file is.
    if (!m_session->isModified() && !m_protectionChanged) {
        dropRecoveryCopy();
        m_recoveryStamp = {-1, -1, -1, -1};
        return;
    }
    if (m_saving || m_recovering || m_trialMacros > 0 || m_session->isBusy()) return;
    // (a workbook protected with a password of a person's is not written plainly into the recovery folder; one protected only with the password every program knows is)
    if (hasUserPassword()) {
        dropRecoveryCopy();
        if (!m_recoveryNoted) {
            m_recoveryNoted = true;
            setStatus(tr("This workbook is protected with a password, so no recovery copy is kept of it. Save it to keep your work."));
        }
        return;
    }
    // Nothing has changed since the last copy was begun: it is still the workbook.
    const std::array<qint64, 4> stamp{m_loader.changesMade(), m_session->undoStack()->index(), m_session->undoStack()->count(), m_protectionChanged ? 1 : 0};
    if (stamp == m_recoveryStamp && !m_recoveryCopy.isEmpty()) return;
    syncPanes();
    auto snapshot = std::make_shared<crunch::EditSession::Snapshot>();
    QString why;
    if (!m_session->takeSnapshot(snapshot.get(), &why)) return;
    if (m_objects) m_objects->addToSave(&snapshot->changes);
    if (m_editedProps) {
        snapshot->changes.setProperties = true;
        snapshot->changes.properties = *m_editedProps;
    }
    // (the rules the patch of a sheet takes away are written to a map the window's save reads: this copy has a map of its own, which nothing reads)
    for (auto &entry : snapshot->changes.sheets)
        if (entry.second.condRawOut) entry.second.condRawOut = std::make_shared<decltype(entry.second.condRawOut)::element_type>();
    // Named for the workbook as it is now (Save As moves it).
    const QString copy = recovery::copyPath(m_path, displayName(), m_serial);
    const QString source = m_path.isEmpty() ? m_recoveredFrom : m_path;
    const QString title = displayName();
    const std::shared_ptr<office::sheet::Workbook> book = m_loaded->book;
    const quint64 generation = m_recoveryGeneration;
    // What the file is made from must not be what the window goes on editing: the workbook object grows (strings, formats) and changes (sheet names) with every edit. A workbook read from a file
    // is read again by the thread for itself, with the list of sheets as it is now; a workbook read from another form (an older file, a text file) has no file to read again, and the thread
    // is given a copy of the object (its large parts are shared, not copied). The rest of what is saved is in the snapshot, a copy. A snapshot that holds a function that reads the window's
    // models (a pivot table made in this run, or a chart part made from a model) is written here, where nothing can change meanwhile.
    const bool apart = snapshot->changes.addedPivots.empty() && snapshot->changes.producedParts.empty();
    const bool converted = book->isConverted();
    QElapsedTimer copying;
    copying.start();
    const std::shared_ptr<office::sheet::Workbook> copyOfBook = apart && converted ? std::make_shared<office::sheet::Workbook>(*book) : nullptr;
    m_recoveryCopyMs = copying.nsecsElapsed() / 1e6;
    m_recovering = true;
    m_recoveryStamp = stamp;
    m_recoveryJobCopy = copy;
    m_recoveryCancel = std::make_shared<std::atomic<bool>>(false);
    const std::shared_ptr<std::atomic<bool>> cancel = m_recoveryCancel;
    const std::function<void()> hook = m_recoveryHook;
    auto make = [snapshot, copy, source, title, cancel, hook, copyOfBook](const office::sheet::Workbook &from, const QByteArray &fileBytes, const office::sheet::WorkbookInfo *info, QString *error) {
        return recovery::write(
            [&](const QString &file, QString *err) {
                QByteArray bytes;
                office::sheet::SaveReport report;
                if (copyOfBook) {
                    if (hook) hook();
                    return office::sheet::saveXlsx(*copyOfBook, snapshot->changes, &bytes, err, &report, cancel.get()) && office::sheet::writeFileAtomically(file, bytes, err);
                }
                if (!info) {
                    if (hook) hook();
                }
                if (!info) return office::sheet::saveXlsx(from, snapshot->changes, &bytes, err, &report, cancel.get()) && office::sheet::writeFileAtomically(file, bytes, err);
                office::Package package;
                office::sheet::Workbook mine;
                QString why;
                office::sheet::Limits limits;
                limits.richText = true;   // (as the window reads: the runs of formatting inside texts are kept)
                if (!package.readLazy(fileBytes, &why) || !mine.open(package, &why, limits)) {
                    if (err) *err = tr("The workbook could not be read again for its copy: %1").arg(why);
                    return false;
                }
                mine.setInfo(*info);
                if (hook) hook();   // (a test holds the thread here, while the window goes on)
                return office::sheet::saveXlsx(mine, snapshot->changes, &bytes, err, &report, cancel.get()) && office::sheet::writeFileAtomically(file, bytes, err);
            },
            copy, source, title, error);
    };
    if (!apart) {
        QString error;
        const bool ok = make(*book, QByteArray(), nullptr, &error);
        recoveryFinished(ok, copy, generation, error);
        return;
    }
    const QByteArray fileBytes = converted ? QByteArray() : book->package().fileBytes();   // (shared, not copied)
    const auto info = std::make_shared<office::sheet::WorkbookInfo>(book->info());
    QThread *thread = QThread::create([this, make, fileBytes, info, copy, generation] {
        office::sheet::Workbook none;
        QString error;
        const bool ok = make(none, fileBytes, info.get(), &error);
        // (the window waits for this thread before it goes: cancelRecovery)
        QMetaObject::invokeMethod(this, [this, ok, copy, generation, error] { recoveryFinished(ok, copy, generation, error); }, Qt::QueuedConnection);
    });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    m_recoveryThread = thread;
    thread->start(QThread::LowPriority);
}

void CrunchWindow::recoveryFinished(bool ok, const QString &copy, quint64 generation, const QString &error)
{
    const QString older = m_recoveryCopy;   // (kept until the new copy is written)
    m_recovering = false;
    m_recoveryJobCopy.clear();
    // A copy begun before the workbook was saved, replaced, closed, or given a password is not of the workbook that is here now: it goes.
    if (generation != m_recoveryGeneration) {
        if (ok) recovery::remove(copy);
        Q_EMIT recoveryWritten(false);
        return;
    }
    if (ok) {
        m_recoveryCopy = copy;
        if (!older.isEmpty() && older != copy) recovery::remove(older);
        if (m_recoveredCopy) {
            recovery::discard(*m_recoveredCopy);   // (this run keeps its own copy before the old one goes)
            m_recoveredCopy.reset();
        }
    } else {
        m_recoveryStamp = {-1, -1, -1, -1};   // (tried again at the next tick)
        qWarning("JeffCrunch: the recovery copy could not be made: %s", qPrintable(error));
        if (!m_recoveryFailNoted) {
            m_recoveryFailNoted = true;
            setStatus(tr("A copy of your unsaved work could not be kept for recovery. %1").arg(error));
        }
    }
    Q_EMIT recoveryWritten(ok);
}

// The copy being made is given up (the window is closing, or the program is quitting): the thread is asked to stop and waited for, and what it made is removed.
void CrunchWindow::cancelRecovery()
{
    if (m_recoveryCancel) m_recoveryCancel->store(true);
    if (m_recoveryThread) m_recoveryThread->wait();
    if (m_recovering) recovery::remove(m_recoveryJobCopy);
    m_recovering = false;
    m_recoveryJobCopy.clear();
    ++m_recoveryGeneration;   // (a finish that is on its way finds the copy out of date)
}

void CrunchWindow::finishRecoveries()
{
    for (QWidget *top : QApplication::topLevelWidgets())
        if (auto *w = qobject_cast<CrunchWindow *>(top)) w->cancelRecovery();
}

// The copy no longer matches the workbook (it was saved, closed, replaced, or given a password): the one kept goes, and one that is being made is not kept when it is done.
void CrunchWindow::invalidateRecovery()
{
    ++m_recoveryGeneration;
    dropRecoveryCopy();
    m_recoveryStamp = {-1, -1, -1, -1};
}

void CrunchWindow::dropRecoveryCopy()
{
    recovery::remove(m_recoveryCopy);
    m_recoveryCopy.clear();
}

void CrunchWindow::offerRecovery(bool askIfNone)
{
    const QVector<recovery::Recovered> found = recovery::orphans();
    if (found.isEmpty()) {
        if (askIfNone) jp::plainInformation(this, tr("Recover Unsaved Work"), tr("There's no unsaved work to recover."));
        return;
    }
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Recover Unsaved Work"));
    auto *v = new QVBoxLayout(&dlg);
    auto *intro = plainLabel(new QLabel(tr("%1 didn't close normally, and saved copies of your unsaved work. Choose which to open; you can save each one where you like.").arg(appInfo().name), &dlg));
    intro->setWordWrap(true);
    v->addWidget(intro);
    auto *list = new QListWidget(&dlg);
    list->setAccessibleName(tr("Recovered workbooks"));
    const QLocale loc;
    for (const auto &r : found) {
        const QString where = r.source.isEmpty() ? tr("never saved") : QDir::toNativeSeparators(r.source);
        auto *it = new QListWidgetItem(tr("%1\n%2, saved %3").arg(r.title, where, loc.toString(r.saved, QLocale::ShortFormat)), list);
        it->setFlags(it->flags() | Qt::ItemIsUserCheckable);
        it->setCheckState(Qt::Checked);
        it->setIcon(icon(QStringLiteral("life-buoy")));
    }
    list->setMinimumSize(480, 180);
    v->addWidget(list);
    auto *buttons = new QDialogButtonBox(&dlg);
    QPushButton *open = buttons->addButton(tr("Open"), QDialogButtonBox::AcceptRole);
    QPushButton *del = buttons->addButton(tr("Delete"), QDialogButtonBox::DestructiveRole);
    buttons->addButton(tr("Not Now"), QDialogButtonBox::RejectRole);
    v->addWidget(buttons);
    open->setDefault(true);
    int choice = 0;
    connect(open, &QPushButton::clicked, &dlg, [&] { choice = 1; dlg.accept(); });
    connect(del, &QPushButton::clicked, &dlg, [&] {
        if (jp::plainQuestion(&dlg, tr("Delete"), tr("Delete the checked copies? Their unsaved work can't be brought back.")) != QMessageBox::Yes)
            return;
        choice = 2;
        dlg.accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted) return;
    for (int i = 0; i < found.size(); ++i) {
        if (list->item(i)->checkState() != Qt::Checked) continue;
        const recovery::Recovered &r = found[i];
        if (choice == 2) {
            recovery::discard(r);
            continue;
        }
        QString err;
        if (!openRecovered(r, &err))
            jp::plainWarning(this, tr("Recover Unsaved Work"), tr("%1 can't open the copy of \"%2\".\n%3").arg(appInfo().name, r.title, err));
    }
}

CrunchWindow *CrunchWindow::openRecovered(const recovery::Recovered &r, QString *error)
{
    if (!QFileInfo::exists(r.file)) {
        if (error) *error = tr("The copy isn't there.");
        return nullptr;
    }
    CrunchWindow *target = this;
    if ((m_loaded || m_loading) && !isUntouchedBlank()) {
        target = new CrunchWindow();
        target->setAttribute(Qt::WA_DeleteOnClose);
        target->setClosesIfOpenFails(true);
        target->show();
    }
    target->m_recoveredCopy = r;
    if (!target->openFile(r.file, OpenHow::Copy)) {
        target->m_recoveredCopy.reset();
        if (error) *error = tr("The copy can't be read.");
        if (target != this) target->close();
        return nullptr;
    }
    return target;
}

} // namespace jp
