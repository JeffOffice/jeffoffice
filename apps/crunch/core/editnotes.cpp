// The notes and the links of the cells: putting them on, changing them, and taking them off. One step of Undo each; the model they are kept in is core/anchors.h.
#include "core/editsession.h"

#include <algorithm>

namespace jp::crunch {

using jp::office::sheet::CellRange;
using jp::office::sheet::LinkSpec;
using jp::office::sheet::NoteInfo;

namespace {

// The most characters of a note a cell can hold.
constexpr int kMaxNoteLength = 32767;

EditSession::Outcome refused(const QString &why)
{
    EditSession::Outcome o;
    o.ok = false;
    o.problem = why;
    return o;
}

} // namespace

bool EditSession::canEditNotes(int sheet, QString *why) const
{
    auto refuse = [&](const QString &text) {
        if (why) *why = text;
        return false;
    };
    if (!canEdit(why) || !canEditSheet(sheet, why)) return false;
    if (m_book->book->isConverted())
        return refuse(tr("Notes and links of a workbook opened from this kind of file are kept when it is saved as an .xlsx workbook. Save it as one, and then add them."));
    if (m_anchors.truncated(sheet)) return refuse(tr("This sheet has more notes than JeffCrunch changes (%1). It shows them, and keeps them, but doesn't put notes on it or take them off.").arg(jp::office::sheet::kMaxModelNotes));
    return true;
}

EditSession::Outcome EditSession::setNote(int sheet, quint32 row, quint32 col, const QString &text, const QString &author, bool shown, const QString &stepText)
{
    QString why;
    if (!canEditNotes(sheet, &why)) return refused(why);
    if (row >= jp::office::sheet::kMaxRows || col >= jp::office::sheet::kMaxColumns) return refused(tr("That cell is outside the sheet."));
    const jp::office::sheet::Sheet &model = m_book->calc->sheet(sheet);
    if (model.isProtected && model.protection.objects) return refused(tr("This sheet is protected, so its notes can't be changed. Take the protection off first."));
    if (text.isEmpty()) return refused(tr("A note needs some words. To take a note away, delete it."));
    if (text.size() > kMaxNoteLength) return refused(tr("A note can have up to %1 characters. Shorten it, and try again.").arg(kMaxNoteLength));
    const NoteInfo *was = m_anchors.noteAt(sheet, row, col);
    if (was && was->threaded) return refused(tr("This note is a conversation of comments. JeffCrunch shows it, but doesn't change it."));
    if (!was && qint64(m_anchors.notes(sheet).size()) >= jp::office::sheet::kMaxNotes) return refused(tr("A sheet can have up to %1 notes.").arg(jp::office::sheet::kMaxNotes));
    NoteInfo now;
    now.text = text;
    now.shown = shown;
    now.author = was ? was->author : author;
    if (was && *was == now) return Outcome();   // (nothing to change)
    SideEdit edit;
    edit.sheet = sheet;
    SideEdit::Note n;
    n.row = row;
    n.col = col;
    if (was) n.before = *was;
    n.after = now;
    edit.notes.push_back(std::move(n));
    Step step;
    step.sides.push_back(std::move(edit));
    pushStep(std::move(step), stepText);
    return Outcome();
}

EditSession::Outcome EditSession::deleteNotes(int sheet, const std::vector<std::pair<quint32, quint32>> &cells, const QString &stepText)
{
    QString why;
    if (!canEditNotes(sheet, &why)) return refused(why);
    const jp::office::sheet::Sheet &model = m_book->calc->sheet(sheet);
    if (model.isProtected && model.protection.objects) return refused(tr("This sheet is protected, so its notes can't be changed. Take the protection off first."));
    SideEdit edit;
    edit.sheet = sheet;
    bool threaded = false;
    for (const auto &c : cells) {
        const NoteInfo *was = m_anchors.noteAt(sheet, c.first, c.second);
        if (!was) continue;
        if (was->threaded) {
            threaded = true;
            continue;
        }
        SideEdit::Note n;
        n.row = c.first;
        n.col = c.second;
        n.before = *was;
        edit.notes.push_back(std::move(n));
    }
    if (edit.notes.empty()) return threaded ? refused(tr("This note is a conversation of comments. JeffCrunch shows it, but doesn't change it.")) : Outcome();
    Step step;
    step.sides.push_back(std::move(edit));
    pushStep(std::move(step), stepText);
    return Outcome();
}

EditSession::Outcome EditSession::showNotes(int sheet, const std::vector<std::pair<quint32, quint32>> &cells, bool shown, const QString &stepText)
{
    QString why;
    if (!canEditNotes(sheet, &why)) return refused(why);
    SideEdit edit;
    edit.sheet = sheet;
    for (const auto &c : cells) {
        const NoteInfo *was = m_anchors.noteAt(sheet, c.first, c.second);
        if (!was || was->shown == shown) continue;
        SideEdit::Note n;
        n.row = c.first;
        n.col = c.second;
        n.before = *was;
        NoteInfo now = *was;
        now.shown = shown;
        n.after = now;
        edit.notes.push_back(std::move(n));
    }
    if (edit.notes.empty()) return Outcome();
    Step step;
    step.sides.push_back(std::move(edit));
    pushStep(std::move(step), stepText);
    return Outcome();
}

EditSession::Outcome EditSession::checkLink(int sheet, const CellRange &range, const LinkSpec &link) const
{
    QString why;
    if (!canEditNotes(sheet, &why)) return refused(why);
    if (!range.valid || range.row2 >= jp::office::sheet::kMaxRows || range.col2 >= jp::office::sheet::kMaxColumns) return refused(tr("That cell is outside the sheet."));
    if (link.target.isEmpty() && link.location.isEmpty()) return refused(tr("A link needs an address or a place to go to."));
    if (link.target.size() > jp::office::sheet::kMaxLinkTarget || link.location.size() > jp::office::sheet::kMaxLinkTarget)
        return refused(tr("That address is longer than a link can hold (%1 characters).").arg(jp::office::sheet::kMaxLinkTarget));
    if (link.tip.size() > jp::office::sheet::kMaxLinkText || link.display.size() > jp::office::sheet::kMaxLinkText) return refused(tr("That text is too long for a link (up to %1 characters).").arg(jp::office::sheet::kMaxLinkText));
    if (jp::office::sheet::linkKind(link) == jp::office::sheet::LinkKind::Unsafe) return refused(tr("That kind of address runs a script or carries a page inside itself, so it can't be made a link."));
    const jp::office::sheet::Sheet &model = m_book->calc->sheet(sheet);
    if (model.isProtected) {
        if (!model.protection.allowInsertLinks) return refused(tr("This sheet is protected, so links can't be put on it. Take the protection off first (Review, Unprotect Sheet)."));
        return lockedCellsIn(sheet, range);
    }
    return Outcome();
}

// Not ok if a cell of the range is locked on a protected sheet (a cell that holds nothing has the lock of its row or its column), or if the range is more cells than are looked at.
EditSession::Outcome EditSession::lockedCellsIn(int sheet, const CellRange &range) const
{
    if (range.cellCount() > kMaxCellsLockLooked) return refused(tr("That is more cells (%1) than are looked at to see that none is locked. Select fewer cells.").arg(QLocale().toString(qint64(kMaxCellsLockLooked))));
    const jp::office::sheet::Sheet &model = m_book->calc->sheet(sheet);
    for (quint32 r = range.row1; r <= range.row2; ++r)
        for (quint32 c = range.col1; c <= range.col2; ++c) {
            const jp::office::sheet::Cell *cell = model.cells.find(r, c);
            if (isLocked(sheet, cell ? cell->style : implicitStyle(sheet, r, c))) return refused(lockedMessage());
        }
    return Outcome();
}

EditSession::Outcome EditSession::setLink(int sheet, const CellRange &range, const LinkSpec &link, const QString &stepText)
{
    const Outcome checked = checkLink(sheet, range, link);
    if (!checked.ok) return checked;
    SideEdit edit;
    edit.sheet = sheet;
    for (const LinkSpec &l : m_anchors.links(sheet))
        if (l.range.row1 >= range.row1 && l.range.row2 <= range.row2 && l.range.col1 >= range.col1 && l.range.col2 <= range.col2) edit.linksRemoved.push_back(l);
    if (qint64(m_anchors.links(sheet).size()) - qint64(edit.linksRemoved.size()) >= jp::office::sheet::kMaxLinks) return refused(tr("A sheet can have up to %1 links.").arg(jp::office::sheet::kMaxLinks));
    LinkSpec now = link;
    now.range = range;
    now.relId.clear();
    if (edit.linksRemoved.size() == 1 && edit.linksRemoved[0] == now) return Outcome();   // (nothing to change)
    edit.linksAdded.push_back(std::move(now));
    Step step;
    step.sides.push_back(std::move(edit));
    pushStep(std::move(step), stepText);
    return Outcome();
}

EditSession::Outcome EditSession::removeLinks(int sheet, const CellRange &range, const QString &stepText)
{
    QString why;
    if (!canEditNotes(sheet, &why)) return refused(why);
    const jp::office::sheet::Sheet &model = m_book->calc->sheet(sheet);
    if (model.isProtected) {
        if (!model.protection.allowInsertLinks) return refused(tr("This sheet is protected, so its links can't be changed. Take the protection off first (Review, Unprotect Sheet)."));
        // (the links on the cells it may not change stay: every cell of the range is looked at)
        const Outcome locked = lockedCellsIn(sheet, range);
        if (!locked.ok) return locked;
    }
    SideEdit edit;
    edit.sheet = sheet;
    for (const LinkSpec &l : m_anchors.links(sheet))
        if (l.range.row1 <= range.row2 && l.range.row2 >= range.row1 && l.range.col1 <= range.col2 && l.range.col2 >= range.col1) edit.linksRemoved.push_back(l);
    if (edit.linksRemoved.empty()) return Outcome();
    Step step;
    step.sides.push_back(std::move(edit));
    pushStep(std::move(step), stepText);
    return Outcome();
}

SideEdit EditSession::notesAndLinksIn(int sheet, const CellRange &range) const
{
    SideEdit edit;
    edit.sheet = sheet;
    for (const auto &n : m_anchors.notes(sheet)) {
        const quint32 row = quint32(n.first >> 32), col = quint32(n.first & 0xFFFFFFFFu);
        if (!range.contains(row, col) || n.second.threaded) continue;
        SideEdit::Note note;
        note.row = row;
        note.col = col;
        note.before = n.second;
        edit.notes.push_back(std::move(note));
    }
    for (const LinkSpec &l : m_anchors.links(sheet))
        if (l.range.row1 <= range.row2 && l.range.row2 >= range.row1 && l.range.col1 <= range.col2 && l.range.col2 >= range.col1) edit.linksRemoved.push_back(l);
    return edit;
}

EditSession::Outcome EditSession::clearNotesAndLinks(int sheet, const CellRange &range, const QString &stepText)
{
    QString why;
    if (!canEditNotes(sheet, &why)) return refused(why);
    SideEdit edit = notesAndLinksIn(sheet, range);
    if (edit.empty()) return Outcome();
    Step step;
    step.sides.push_back(std::move(edit));
    pushStep(std::move(step), stepText);
    return Outcome();
}

} // namespace jp::crunch
