// JeffCrunch's protection: Protect Sheet and Protect Workbook (the structure), the password asked for to take either off, and the cells a protected sheet won't let the cursor rest on. The
// settings and the hash are libs/office/sheet/protection.h; the session holds them (core/editprotect.cpp). A password is hashed, and checked, off the window's thread (it can take a
// while for a file that asks for many repeats: a note with a Cancel button shows if it does), and never kept as text. Undo and Redo of a step that would take a password's protection off
// ask for the password too, so that a password can't be got round by undoing.

#include "app/crunchwindow.h"

#include "app/protectdialogs.h"
#include "sheet/protection.h"

#include <QAction>
#include <QInputDialog>
#include <QLineEdit>
#include <QProgressDialog>
#include <QThread>
#include <QUndoStack>

#include <atomic>
#include <memory>

namespace jp {

using jp::office::sheet::PasswordHash;
using jp::office::sheet::PasswordResult;
using jp::office::sheet::SheetProtection;
using jp::office::sheet::WorkbookProtection;

namespace {
constexpr int kSkipLimit = 2000;   // (the most cells passed over to find one the cursor may rest on)
}

// A piece of work on a thread of its own, whose answer is taken on the window's thread when it is over (no loop of events is run to wait for it). The window can't be edited meanwhile.
struct CrunchWindow::PasswordJob {
    std::atomic<bool> stop{false};
    QPointer<QProgressDialog> note;
    QThread *thread = nullptr;
};

void CrunchWindow::runPasswordJob(const QString &text, const std::function<int(const std::atomic<bool> *)> &work, const std::function<void(int)> &done)
{
    if (m_passwordJob) return;   // (one at a time)
    auto job = std::make_shared<PasswordJob>();
    m_passwordJob = job;
    m_session->setBusy(true);
    updateEditActions();
    QThread *thread = QThread::create([job, work, self = QPointer<CrunchWindow>(this), done] {
        const int result = work(&job->stop);
        if (!self) return;
        QMetaObject::invokeMethod(self.data(), [self, done, result, job] {
            if (!self) return;
            if (job->note) job->note->close();
            self->m_passwordJob.reset();
            if (self->m_session) self->m_session->setBusy(false);
            self->updateEditActions();
            done(result);
        }, Qt::QueuedConnection);
    });
    job->thread = thread;
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    // a note with a Cancel button, if it takes longer than a moment
    QTimer::singleShot(300, this, [this, job, text] {
        if (m_passwordJob != job) return;
        auto *note = new QProgressDialog(text, tr("Cancel"), 0, 0, this);
        note->setObjectName(QStringLiteral("passwordNote"));
        note->setWindowModality(Qt::NonModal);
        note->setAutoClose(false);
        note->setAutoReset(false);
        note->setAttribute(Qt::WA_DeleteOnClose);
        connect(note, &QProgressDialog::canceled, this, [job] { job->stop = true; });
        job->note = note;
        note->show();
    });
    thread->start();
}

void CrunchWindow::stopPasswordJob()
{
    if (!m_passwordJob) return;
    m_passwordJob->stop = true;
    if (m_passwordJob->thread) m_passwordJob->thread->wait();
    m_passwordJob.reset();
}

void CrunchWindow::createProtectActions()
{
    mk("protect.sheet", tr("Protect Sheet"), "lock", QKeySequence(), [this] { protectSheet(); }, false, false);
    mk("protect.workbook", tr("Protect Workbook"), "shield", QKeySequence(), [this] { protectWorkbook(); }, false, false);
    connect(m_view, &grid::GridView::selectionChanged, this, &CrunchWindow::guardSelection);
}

void CrunchWindow::connectProtection()
{
    connect(m_session.get(), &crunch::EditSession::protectionChanged, this, [this](int) {
        updateProtectActions();
        updateNoteActions();
        updateEditActions();
        guardSelection();
    });
    connect(m_session->undoStack(), &QUndoStack::indexChanged, this, [this] { updateProtectActions(); });
    updateProtectActions();
}

void CrunchWindow::updateProtectActions()
{
    const bool open = m_session && !m_saving && !m_passwordJob && m_sheet >= 0 && !m_chartSheetShown && m_loaded && m_loaded->calc;
    const bool on = open && m_session->protectionOf(m_sheet).enabled;
    act("protect.sheet")->setText(on ? tr("Unprotect Sheet") : tr("Protect Sheet"));
    act("protect.sheet")->setEnabled(open);
    const bool locked = m_session && m_session->workbookProtection().any();
    act("protect.workbook")->setText(locked ? tr("Unprotect Workbook") : tr("Protect Workbook"));
    act("protect.workbook")->setEnabled(m_session && !m_saving && !m_passwordJob);
}

// Asks for the password of `hash`, checks it, and, if it is the right one, goes on with `onRight` (the person is told why not otherwise). A hash with no password asks nothing. The check is
// made off the window's thread; `onRight` is called when it is over.
void CrunchWindow::askPassword(const QString &what, const PasswordHash &hash, const std::function<void()> &onRight)
{
    if (!hash.isSet()) {
        onRight();
        return;
    }
    PasswordRequest req;
    req.what = what;
    if (m_anchorHooks.passwordAsker) {
        if (!m_anchorHooks.passwordAsker(&req)) return;
    } else {
        bool ok = false;
        req.password = QInputDialog::getText(this, tr("Password"), tr("Type the password for %1:").arg(what), QLineEdit::Password, QString(), &ok);
        if (!ok) return;
    }
    const QString typed = req.password;
    req.password.clear();
    runPasswordJob(tr("Checking the password…"), [hash, typed](const std::atomic<bool> *stop) { return int(jp::office::sheet::checkPassword(hash, typed, stop)); },
                   [this, onRight](int result) {
                       switch (PasswordResult(result)) {
                       case PasswordResult::Match: onRight(); return;
                       case PasswordResult::Wrong: notice(tr("Password"), tr("That is not the right password."), true); return;
                       case PasswordResult::Unsupported:
                           notice(tr("Password"), tr("This file's password is kept in a form JeffCrunch can't check, so the protection can't be taken off here."), true);
                           return;
                       case PasswordResult::TooSlow:
                           notice(tr("Password"),
                                  tr("This file asks for its password to be checked far more times than any program does when it makes one (more than %1), so JeffCrunch doesn't check it.")
                                      .arg(QLocale().toString(jp::office::sheet::kMaxSpinCount)),
                                  true);
                           return;
                       case PasswordResult::Stopped: setStatus(tr("The check of the password was stopped.")); return;
                       }
                   });
}

// A new password made into its hash off the window's thread, and then `done` with it.
void CrunchWindow::hashPassword(const QString &typed, const std::function<void(const PasswordHash &)> &done)
{
    auto made = std::make_shared<PasswordHash>();
    runPasswordJob(tr("Making the password safe…"), [made, typed](const std::atomic<bool> *stop) { return jp::office::sheet::makePasswordHash(typed, made.get(), stop) ? 1 : 0; },
                   [made, done](int ok) {
                       if (ok) done(*made);
                   });
}

void CrunchWindow::protectSheet()
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!m_session || m_sheet < 0 || m_passwordJob) return;
    QString why;
    if (!m_session->canEdit(&why) || !m_session->canEditSheet(m_sheet, &why)) {
        notice(tr("Protect Sheet"), why);
        return;
    }
    const int sheet = m_sheet;
    const SheetProtection now = m_session->protectionOf(sheet);
    if (now.enabled) {
        askPassword(m_loaded->book->info().sheets[sheet].name, now.password, [this, sheet] {
            const crunch::EditSession::Outcome o = m_session->setSheetProtection(sheet, SheetProtection(), tr("Unprotect Sheet"));
            if (!o.ok) notice(tr("Protect Sheet"), o.problem);
        });
        return;
    }
    ProtectRequest req;
    req.settings.enabled = true;
    req.settings.allowSelectLocked = req.settings.allowSelectUnlocked = true;
    if (m_anchorHooks.protectAsker) {
        if (!m_anchorHooks.protectAsker(&req)) return;
    } else {
        ProtectSheetDialog dialog(req, this);
        for (;;) {
            if (dialog.exec() != QDialog::Accepted) return;
            QString problem;
            if (dialog.result(&req, &problem)) break;
        }
    }
    SheetProtection p = req.settings;
    p.enabled = true;
    p.password = PasswordHash();
    auto apply = [this, sheet](const SheetProtection &chosen) {
        const crunch::EditSession::Outcome o = m_session->setSheetProtection(sheet, chosen, tr("Protect Sheet"));
        if (!o.ok) notice(tr("Protect Sheet"), o.problem);
    };
    if (req.password.isEmpty()) {
        apply(p);
        return;
    }
    const QString typed = req.password;
    req.password.clear();
    hashPassword(typed, [p, apply](const PasswordHash &hash) mutable {
        p.password = hash;
        apply(p);
    });
}

void CrunchWindow::protectWorkbook()
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!m_session || m_passwordJob) return;
    QString why;
    if (!m_session->canEdit(&why)) {
        notice(tr("Protect Workbook"), why);
        return;
    }
    const WorkbookProtection now = m_session->workbookProtection();
    if (now.any()) {
        askPassword(tr("the workbook"), now.password, [this] {
            const crunch::EditSession::Outcome o = m_session->setWorkbookProtection(WorkbookProtection(), tr("Unprotect Workbook"));
            if (!o.ok) notice(tr("Protect Workbook"), o.problem);
        });
        return;
    }
    WorkbookProtectRequest req;
    if (m_anchorHooks.workbookProtectAsker) {
        if (!m_anchorHooks.workbookProtectAsker(&req)) return;
    } else {
        ProtectWorkbookDialog dialog(req, this);
        for (;;) {
            if (dialog.exec() != QDialog::Accepted) return;
            QString problem;
            if (dialog.result(&req, &problem)) break;
        }
    }
    WorkbookProtection p;
    p.lockStructure = req.lockStructure;
    p.lockWindows = req.lockWindows;
    auto apply = [this](const WorkbookProtection &chosen) {
        const crunch::EditSession::Outcome o = m_session->setWorkbookProtection(chosen, tr("Protect Workbook"));
        if (!o.ok) notice(tr("Protect Workbook"), o.problem);
    };
    if (req.password.isEmpty()) {
        apply(p);
        return;
    }
    const QString typed = req.password;
    req.password.clear();
    hashPassword(typed, [p, apply](const PasswordHash &hash) mutable {
        p.password = hash;
        apply(p);
    });
}

// A sheet that does not let locked (or unlocked) cells be selected keeps the cursor off them: a cell it may not rest on is passed over, in the way the cursor was going (the next one it may rest
// on in that direction), and when there is none the cursor goes back to where it was.
void CrunchWindow::guardSelection()
{
    if (!m_session || m_sheet < 0 || m_revertingSelection || m_chartSheetShown || !m_loaded || !m_loaded->calc) return;
    const SheetProtection &p = m_session->protectionOf(m_sheet);
    const grid::Selection &sel = m_view->selection();
    if (!p.enabled || (p.allowSelectLocked && p.allowSelectUnlocked) || sel.activeRow < 0) {
        m_lastSelectable = sel;
        return;
    }
    auto may = [&](qint64 row, qint64 col) { return m_session->cellIsLocked(m_sheet, quint32(row), quint32(col)) ? p.allowSelectLocked : p.allowSelectUnlocked; };
    if (may(sel.activeRow, sel.activeCol)) {
        m_lastSelectable = sel;
        return;
    }
    const bool locked = m_session->cellIsLocked(m_sheet, quint32(sel.activeRow), quint32(sel.activeCol));
    m_revertingSelection = true;
    bool moved = false;
    if (m_lastSelectable.activeRow >= 0) {
        const int dRow = sel.activeRow > m_lastSelectable.activeRow ? 1 : sel.activeRow < m_lastSelectable.activeRow ? -1 : 0;
        const int dCol = sel.activeCol > m_lastSelectable.activeCol ? 1 : sel.activeCol < m_lastSelectable.activeCol ? -1 : 0;
        if (dRow || dCol) {
            qint64 row = sel.activeRow, col = sel.activeCol;
            for (int step = 0; step < kSkipLimit && !moved; ++step) {
                row += dRow;
                col += dCol;
                if (row < 0 || col < 0 || row >= qint64(jp::office::sheet::kMaxRows) || col >= qint64(jp::office::sheet::kMaxColumns)) break;
                if (!may(row, col)) continue;
                m_view->selectCell(row, col);
                m_view->ensureVisible(row, col);
                moved = true;
            }
        }
        if (!moved) m_view->setSelection(m_lastSelectable);
    }
    m_revertingSelection = false;
    m_lastSelectable = m_view->selection();
    if (!moved) setStatus(locked ? tr("This sheet is protected, and doesn't let locked cells be selected.") : tr("This sheet is protected, and doesn't let unlocked cells be selected."));
}

} // namespace jp
