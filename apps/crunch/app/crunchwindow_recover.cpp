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

#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QLabel>
#include <QListWidget>
#include <QLocale>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QThread>
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
        return;
    }
    if (m_saving || m_recovering || m_trialMacros > 0 || m_session->isBusy()) return;
    // (a workbook protected with a password is not written plainly into the recovery folder)
    if (m_fileProtected) {
        dropRecoveryCopy();
        if (!m_recoveryNoted) {
            m_recoveryNoted = true;
            setStatus(tr("This workbook is protected with a password, so no recovery copy is kept of it. Save it to keep your work."));
        }
        return;
    }
    syncPanes();
    auto snapshot = std::make_shared<crunch::EditSession::Snapshot>();
    QString why;
    if (!m_session->takeSnapshot(snapshot.get(), &why)) return;
    if (m_objects) m_objects->addToSave(&snapshot->changes);
    if (m_editedProps) {
        snapshot->changes.setProperties = true;
        snapshot->changes.properties = *m_editedProps;
    }
    // Named for the workbook as it is now (Save As moves it).
    const QString copy = recovery::copyPath(m_path, displayName(), m_serial);
    const QString source = m_path.isEmpty() ? m_recoveredFrom : m_path;
    const QString title = displayName();
    const std::shared_ptr<office::sheet::Workbook> book = m_loaded->book;
    const QPointer<CrunchWindow> self(this);
    // What the file is made from must not be what the window goes on editing: the workbook object grows (strings, formats) and changes (sheet names) with every edit. The thread reads the file
    // again for itself and takes the list of sheets as it is now; the rest of what is saved is in the snapshot, a copy. Whatever holds a function that reads the window's models (a pivot
    // table or a chart made in this run), and a workbook read from an older form, is written here, where nothing can change meanwhile.
    const bool apart = !book->isConverted() && snapshot->changes.addedPivots.empty() && snapshot->changes.producedParts.empty();
    m_recovering = true;
    auto make = [snapshot, copy, source, title](const office::sheet::Workbook &from, const QByteArray &fileBytes, const office::sheet::WorkbookInfo *info) {
        return recovery::write(
            [&](const QString &file, QString *error) {
                QByteArray bytes;
                office::sheet::SaveReport report;
                if (!info) return office::sheet::saveXlsx(from, snapshot->changes, &bytes, error, &report) && office::sheet::writeFileAtomically(file, bytes, error);
                office::Package package;
                office::sheet::Workbook mine;
                QString why;
                if (!package.readLazy(fileBytes, &why) || !mine.open(package, &why)) {
                    if (error) *error = why;
                    return false;
                }
                mine.setInfo(*info);
                return office::sheet::saveXlsx(mine, snapshot->changes, &bytes, error, &report) && office::sheet::writeFileAtomically(file, bytes, error);
            },
            copy, source, title);
    };
    if (!apart) {
        recoveryFinished(make(*book, QByteArray(), nullptr), copy);
        return;
    }
    const QByteArray fileBytes = book->package().fileBytes();   // (shared, not copied)
    const auto info = std::make_shared<office::sheet::WorkbookInfo>(book->info());
    QThread *thread = QThread::create([self, make, fileBytes, info, copy] {
        office::sheet::Workbook none;
        const bool ok = make(none, fileBytes, info.get());
        QMetaObject::invokeMethod(qApp, [self, ok, copy] {
            if (!self) {
                if (ok) recovery::remove(copy);   // (the window went while its copy was being made)
                return;
            }
            self->recoveryFinished(ok, copy);
        }, Qt::QueuedConnection);
    });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
}

void CrunchWindow::recoveryFinished(bool ok, const QString &copy)
{
    const QString older = m_recoveryCopy;   // (kept until the new copy is written)
    m_recovering = false;
    if (ok) {
        m_recoveryCopy = copy;
        if (!older.isEmpty() && older != copy) recovery::remove(older);
        if (m_recoveredCopy) {
            recovery::discard(*m_recoveredCopy);   // (this run keeps its own copy before the old one goes)
            m_recoveredCopy.reset();
        }
    }
    Q_EMIT recoveryWritten(ok);
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
