// Protecting sheets and the workbook's structure: what a protected sheet allows and the message when it doesn't, and the commands that protect and unprotect (one step of Undo each). The
// settings and the hash of a password are libs/office/sheet/protection.h; the window makes the hash off its own thread and gives the session the result.
#include "core/editsession.h"

namespace jp::crunch {

using jp::office::sheet::SheetKind;
using jp::office::sheet::SheetProtection;
using jp::office::sheet::WorkbookProtection;

const SheetProtection &EditSession::protectionOf(int sheet) const
{
    static const SheetProtection none;
    if (!m_book || !m_book->calc || sheet < 0 || sheet >= m_book->calc->sheetCount()) return none;
    return m_book->calc->sheet(sheet).protection;
}

const WorkbookProtection &EditSession::workbookProtection() const
{
    static const WorkbookProtection none;
    return m_book && m_book->book ? m_book->book->info().protection : none;
}

bool EditSession::cellIsLocked(int sheet, quint32 row, quint32 col) const
{
    if (!m_book || !m_book->calc || !m_book->book || sheet < 0 || sheet >= m_book->calc->sheetCount()) return false;
    const jp::office::sheet::Calculation::CellContent c = m_book->calc->content(sheet, row, col);
    return m_book->book->styles().format(c.exists ? c.style : implicitStyle(sheet, row, col)).locked;
}

bool EditSession::allows(int sheet, Allow what, QString *why) const
{
    if (!m_book || !m_book->calc || sheet < 0 || sheet >= m_book->calc->sheetCount()) return true;
    const jp::office::sheet::Sheet &s = m_book->calc->sheet(sheet);
    if (!s.isProtected) return true;
    const SheetProtection &p = s.protection;
    bool ok = false;
    QString what_;
    switch (what) {
    case Allow::SelectLocked: ok = p.allowSelectLocked; what_ = tr("selecting locked cells"); break;
    case Allow::SelectUnlocked: ok = p.allowSelectUnlocked; what_ = tr("selecting unlocked cells"); break;
    case Allow::FormatCells: ok = p.allowFormatCells; what_ = tr("formatting cells"); break;
    case Allow::FormatColumns: ok = p.allowFormatColumns; what_ = tr("changing columns' width or hiding them"); break;
    case Allow::FormatRows: ok = p.allowFormatRows; what_ = tr("changing rows' height or hiding them"); break;
    case Allow::InsertColumns: ok = p.allowInsertColumns; what_ = tr("inserting columns"); break;
    case Allow::InsertRows: ok = p.allowInsertRows; what_ = tr("inserting rows"); break;
    case Allow::InsertLinks: ok = p.allowInsertLinks; what_ = tr("putting in links"); break;
    case Allow::DeleteColumns: ok = p.allowDeleteColumns; what_ = tr("deleting columns"); break;
    case Allow::DeleteRows: ok = p.allowDeleteRows; what_ = tr("deleting rows"); break;
    case Allow::Sort: ok = p.allowSort; what_ = tr("sorting"); break;
    case Allow::Filter: ok = p.allowFilter; what_ = tr("using filters"); break;
    case Allow::Pivot: ok = p.allowPivot; what_ = tr("using PivotTables"); break;
    case Allow::Objects: ok = !p.objects; what_ = tr("changing drawing objects and notes"); break;
    }
    if (!ok && why) *why = tr("This sheet is protected, and its protection doesn't allow %1. Take the protection off first (Review, Unprotect Sheet).").arg(what_);
    return ok;
}

bool EditSession::structureOpen(QString *why) const
{
    if (!workbookProtection().lockStructure) return true;
    if (why) *why = tr("The workbook's sheets are locked, so a sheet can't be added, deleted, renamed, moved, hidden, or shown. Take the protection off first (Review, Unprotect Workbook).");
    return false;
}

EditSession::Outcome EditSession::setSheetProtection(int sheet, const SheetProtection &protection, const QString &stepText)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    if (m_book->book->info().sheets[sheet].kind != SheetKind::Worksheet) {
        out.ok = false;
        out.problem = tr("This sheet has no cells to protect.");
        return out;
    }
    if (m_book->calc->sheet(sheet).protection == protection) return out;   // (nothing to change)
    ProtectEdit e;
    e.sheet = sheet;
    e.sheetBefore = m_book->calc->sheet(sheet).protection;
    e.sheetAfter = protection;
    Step step;
    step.protections.push_back(std::move(e));
    pushStep(std::move(step), stepText);
    return out;
}

EditSession::Outcome EditSession::setWorkbookProtection(const WorkbookProtection &protection, const QString &stepText)
{
    Outcome out;
    QString why;
    if (!canEdit(&why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    if (workbookProtection() == protection) return out;
    ProtectEdit e;
    e.sheet = -1;
    e.bookBefore = workbookProtection();
    e.bookAfter = protection;
    Step step;
    step.protections.push_back(std::move(e));
    pushStep(std::move(step), stepText);
    return out;
}

} // namespace jp::crunch
