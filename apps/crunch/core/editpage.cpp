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
    {
        // a text a person made longer than the file may hold is refused (one the file came with is let be, for the sheet's other settings)
        const jp::office::sheet::HeaderFooter &now = m_pages.sheet(sheet).headerFooter, &want = setup.headerFooter;
        const std::pair<const QString &, const QString &> texts[] = {{now.oddHeader, want.oddHeader}, {now.oddFooter, want.oddFooter}, {now.evenHeader, want.evenHeader},
                                                                    {now.evenFooter, want.evenFooter}, {now.firstHeader, want.firstHeader}, {now.firstFooter, want.firstFooter}};
        for (const auto &t : texts)
            if (t.second.size() > jp::office::sheet::kMaxHeaderFooterFileChars && t.second != t.first) {
                out.ok = false;
                out.problem = tr("A header or footer can have up to %1 characters, counting the codes in it.").arg(jp::office::sheet::kMaxHeaderFooterFileChars);
                return out;
            }
    }
    if ((setup.rowBreaks.size() > jp::office::sheet::kMaxPageBreaks && setup.rowBreaks.size() > m_pages.sheet(sheet).rowBreaks.size())
        || (setup.colBreaks.size() > jp::office::sheet::kMaxPageBreaks && setup.colBreaks.size() > m_pages.sheet(sheet).colBreaks.size())) {
        out.ok = false;
        out.problem = tr("A sheet can have up to %1 page breaks of each kind.").arg(jp::office::sheet::kMaxPageBreaks);
        return out;
    }
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
