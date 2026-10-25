// JeffCrunch's protection: Protect Sheet and Protect Workbook (the structure), the password asked for to take either off, and the cells a protected sheet won't let the cursor rest on. The
// settings and the hash are libs/office/sheet/protection.h; the session holds them (core/editprotect.cpp). A password is hashed, and checked, off the window's thread, and never kept as text.
#include "app/crunchwindow.h"

#include "app/protectdialogs.h"
#include "sheet/protection.h"

#include <QAction>
#include <QEventLoop>
#include <QInputDialog>
#include <QLineEdit>
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

// Runs `work` on a thread of its own and returns its result when it is done, the window's events going on (not the person's keys and clicks) while it works.
template <class T, class F>
T offThread(F work)
{
    struct Box {
        T value{};
    };
    auto box = std::make_shared<Box>();
    QThread *thread = QThread::create([box, work] { box->value = work(); });
    QEventLoop loop;
    QObject::connect(thread, &QThread::finished, &loop, &QEventLoop::quit);
    thread->start();
    loop.exec(QEventLoop::ExcludeUserInputEvents);
    thread->wait();
    delete thread;
    return box->value;
}

} // namespace

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
    const bool open = m_session && !m_saving && m_sheet >= 0 && !m_chartSheetShown && m_loaded && m_loaded->calc;
    const bool on = open && m_session->protectionOf(m_sheet).enabled;
    act("protect.sheet")->setText(on ? tr("Unprotect Sheet") : tr("Protect Sheet"));
    act("protect.sheet")->setEnabled(open);
    const bool locked = m_session && m_session->workbookProtection().any();
    act("protect.workbook")->setText(locked ? tr("Unprotect Workbook") : tr("Protect Workbook"));
    act("protect.workbook")->setEnabled(m_session && !m_saving);
}

// Asks for the password of `hash` and says whether it is the right one; the person is told why not otherwise. (Nothing is asked of a hash with no password.)
bool CrunchWindow::askPassword(const QString &what, const PasswordHash &hash)
{
    if (!hash.isSet()) return true;
    PasswordRequest req;
    req.what = what;
    if (m_anchorHooks.passwordAsker) {
        if (!m_anchorHooks.passwordAsker(&req)) return false;
    } else {
        bool ok = false;
        req.password = QInputDialog::getText(this, tr("Password"), tr("Type the password for %1:").arg(what), QLineEdit::Password, QString(), &ok);
        if (!ok) return false;
    }
    const QString typed = req.password;
    req.password.clear();
    const PasswordResult r = offThread<PasswordResult>([hash, typed] { return jp::office::sheet::checkPassword(hash, typed); });
    switch (r) {
    case PasswordResult::Match: return true;
    case PasswordResult::Wrong: notice(tr("Password"), tr("That is not the right password."), true); return false;
    case PasswordResult::Unsupported:
        notice(tr("Password"), tr("This file's password is kept in a form JeffCrunch can't check, so the protection can't be taken off here."), true);
        return false;
    case PasswordResult::TooSlow:
        notice(tr("Password"), tr("This file asks for its password to be checked far more times than any program does when it makes one (more than %1), so JeffCrunch doesn't check it.").arg(QLocale().toString(jp::office::sheet::kMaxSpinCount)), true);
        return false;
    case PasswordResult::Stopped: return false;
    }
    return false;
}

void CrunchWindow::protectSheet()
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!m_session || m_sheet < 0) return;
    QString why;
    if (!m_session->canEdit(&why) || !m_session->canEditSheet(m_sheet, &why)) {
        notice(tr("Protect Sheet"), why);
        return;
    }
    const SheetProtection now = m_session->protectionOf(m_sheet);
    if (now.enabled) {
        if (!askPassword(m_loaded->book->info().sheets[m_sheet].name, now.password)) return;
        const crunch::EditSession::Outcome o = m_session->setSheetProtection(m_sheet, SheetProtection(), tr("Unprotect Sheet"));
        if (!o.ok) notice(tr("Protect Sheet"), o.problem);
        return;
    }
    ProtectRequest req;
    req.settings.enabled = true;
    // (what people allow most often)
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
    if (!req.password.isEmpty()) {
        const QString typed = req.password;
        req.password.clear();
        PasswordHash made;
        const bool ok = offThread<bool>([&made, typed] { return jp::office::sheet::makePasswordHash(typed, &made); });
        if (!ok) return;
        p.password = made;
    }
    const crunch::EditSession::Outcome o = m_session->setSheetProtection(m_sheet, p, tr("Protect Sheet"));
    if (!o.ok) notice(tr("Protect Sheet"), o.problem);
}

void CrunchWindow::protectWorkbook()
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!m_session) return;
    QString why;
    if (!m_session->canEdit(&why)) {
        notice(tr("Protect Workbook"), why);
        return;
    }
    const WorkbookProtection now = m_session->workbookProtection();
    if (now.any()) {
        if (!askPassword(tr("the workbook"), now.password)) return;
        const crunch::EditSession::Outcome o = m_session->setWorkbookProtection(WorkbookProtection(), tr("Unprotect Workbook"));
        if (!o.ok) notice(tr("Protect Workbook"), o.problem);
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
    if (!req.password.isEmpty()) {
        const QString typed = req.password;
        req.password.clear();
        PasswordHash made;
        const bool ok = offThread<bool>([&made, typed] { return jp::office::sheet::makePasswordHash(typed, &made); });
        if (!ok) return;
        p.password = made;
    }
    const crunch::EditSession::Outcome o = m_session->setWorkbookProtection(p, tr("Protect Workbook"));
    if (!o.ok) notice(tr("Protect Workbook"), o.problem);
}

// A sheet that does not let locked (or unlocked) cells be selected keeps the cursor off them: it goes back to where it was.
void CrunchWindow::guardSelection()
{
    if (!m_session || m_sheet < 0 || m_revertingSelection || m_chartSheetShown || !m_loaded || !m_loaded->calc) return;
    const SheetProtection &p = m_session->protectionOf(m_sheet);
    const grid::Selection &sel = m_view->selection();
    if (!p.enabled || (p.allowSelectLocked && p.allowSelectUnlocked) || sel.activeRow < 0) {
        m_lastSelectable = sel;
        return;
    }
    const bool locked = m_session->cellIsLocked(m_sheet, quint32(sel.activeRow), quint32(sel.activeCol));
    if (locked ? p.allowSelectLocked : p.allowSelectUnlocked) {
        m_lastSelectable = sel;
        return;
    }
    m_revertingSelection = true;
    if (m_lastSelectable.activeRow >= 0) m_view->setSelection(m_lastSelectable);
    m_revertingSelection = false;
    setStatus(locked ? tr("This sheet is protected, and doesn't let locked cells be selected.") : tr("This sheet is protected, and doesn't let unlocked cells be selected."));
}

} // namespace jp
