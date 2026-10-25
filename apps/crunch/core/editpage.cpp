// How the sheets of an open workbook print, changed by the page commands: each change is one step of Undo (see EditSession::setPageSetup and core/pagesettings.h).
#include "core/editsession.h"

namespace jp::crunch {

using jp::office::sheet::PageSetup;

EditSession::Outcome EditSession::setPageSetup(int sheet, const PageSetup &setup, const QString &text)
{
    Outcome out;
    QString why;
    if (!canEdit(&why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    if (sheet < 0 || sheet >= m_pages.sheetCount() || m_book->book->info().sheets[sheet].kind != jp::office::sheet::SheetKind::Worksheet) {
        out.ok = false;
        out.problem = tr("That sheet can't be set up for printing.");
        return out;
    }
    if (m_pages.sheet(sheet) == setup) return out;   // (nothing to change)
    Step step;
    PageEdit e;
    e.sheet = sheet;
    e.before = m_pages.sheet(sheet);
    e.after = setup;
    step.pages.push_back(std::move(e));
    pushStep(std::move(step), text);
    return out;
}

} // namespace jp::crunch
