// The threads of comments on cells: starting one, replying, editing, resolving, deleting, and turning notes into threads and back. One step of Undo each; the model they are kept in is
// core/anchors.h (a thread is the `thread` of the cell's NoteInfo).
#include "core/editsession.h"

namespace jp::crunch {

using jp::office::sheet::NoteInfo;
using jp::office::sheet::ThreadComment;
using jp::office::sheet::ThreadList;

namespace {

EditSession::Outcome refused(const QString &why)
{
    EditSession::Outcome o;
    o.ok = false;
    o.problem = why;
    return o;
}

// The most notes looked through for a person of the same name.
constexpr int kMaxLookedForPerson = 20000;

// A comment by `author`, with the person the workbook's threads already have of that name (else a new one).
ThreadComment commentBy(const CellAnchors &anchors, const QString &text, const QString &author, const QString &parentId)
{
    ThreadComment c;
    c.id = jp::office::sheet::newThreadId();
    c.parentId = parentId;
    c.author = author.trimmed().isEmpty() ? QCoreApplication::translate("jp::crunch::EditSession", "Unknown") : author.trimmed();
    c.time = jp::office::sheet::threadTimeNow();
    c.text = text;
    int looked = 0;
    for (int s = 0; s < anchors.sheetCount() && c.personId.isEmpty(); ++s)
        for (const auto &n : anchors.notes(s)) {
            if (++looked > kMaxLookedForPerson) break;
            if (!n.second.thread) continue;
            bool found = false;
            for (const ThreadComment &o : *n.second.thread)
                if (!o.personId.isEmpty() && o.author == c.author) {
                    c.personId = o.personId;
                    c.userId = o.userId;
                    c.providerId = o.providerId;
                    found = true;
                    break;
                }
            if (found) break;
        }
    if (c.personId.isEmpty()) {
        c.personId = jp::office::sheet::newThreadId();
        c.userId = c.author;
        c.providerId = QStringLiteral("None");
    }
    return c;
}

NoteInfo withThread(const NoteInfo &from, ThreadList list)
{
    NoteInfo n = from;
    n.threaded = true;
    n.threadEdited = true;
    n.author.clear();
    n.text = jp::office::sheet::threadWords(list);
    n.thread = std::make_shared<const ThreadList>(std::move(list));
    return n;
}

} // namespace

EditSession::Outcome EditSession::checkThreadCell(int sheet, quint32 row, quint32 col) const
{
    QString why;
    if (!canEditNotes(sheet, &why)) return refused(why);
    if (row >= jp::office::sheet::kMaxRows || col >= jp::office::sheet::kMaxColumns) return refused(tr("That cell is outside the sheet."));
    const jp::office::sheet::Sheet &model = m_book->calc->sheet(sheet);
    if (model.isProtected && model.protection.objects) return refused(tr("This sheet is protected, so its comments can't be changed. Take the protection off first."));
    return Outcome();
}

EditSession::Outcome EditSession::pushNoteChanges(int sheet, std::vector<SideEdit::Note> notes, const QString &stepText)
{
    if (notes.empty()) return Outcome();
    SideEdit edit;
    edit.sheet = sheet;
    edit.notes = std::move(notes);
    Step step;
    step.sides.push_back(std::move(edit));
    pushStep(std::move(step), stepText);
    return Outcome();
}

EditSession::Outcome EditSession::newComment(int sheet, quint32 row, quint32 col, const QString &text, const QString &author, const QString &stepText)
{
    const Outcome checked = checkThreadCell(sheet, row, col);
    if (!checked.ok) return checked;
    if (text.trimmed().isEmpty()) return refused(tr("A comment needs some words."));
    if (text.size() > jp::office::sheet::kMaxThreadCommentChars) return refused(tr("A comment can have up to %1 characters. Shorten it, and try again.").arg(jp::office::sheet::kMaxThreadCommentChars));
    if (m_anchors.noteAt(sheet, row, col)) return refused(tr("This cell has a note or comments already. Reply to the comments, or delete them first."));
    if (qint64(m_anchors.notes(sheet).size()) >= jp::office::sheet::kMaxNotes) return refused(tr("A sheet can have up to %1 notes and comments.").arg(jp::office::sheet::kMaxNotes));
    SideEdit::Note n;
    n.row = row;
    n.col = col;
    n.after = withThread(NoteInfo(), {commentBy(m_anchors, text, author, QString())});
    return pushNoteChanges(sheet, {std::move(n)}, stepText);
}

EditSession::Outcome EditSession::replyToComment(int sheet, quint32 row, quint32 col, const QString &text, const QString &author, const QString &stepText)
{
    const Outcome checked = checkThreadCell(sheet, row, col);
    if (!checked.ok) return checked;
    const NoteInfo *was = m_anchors.noteAt(sheet, row, col);
    if (!was || !was->thread || was->thread->empty()) return refused(tr("There are no comments on this cell to reply to."));
    if (was->threadCut || was->thread->size() >= size_t(jp::office::sheet::kMaxThreadComments)) return refused(tr("This thread has more comments than JeffCrunch changes (%1). It shows them, and keeps them, but doesn't add to them.").arg(jp::office::sheet::kMaxThreadComments));
    if (text.trimmed().isEmpty()) return refused(tr("A reply needs some words."));
    if (text.size() > jp::office::sheet::kMaxThreadCommentChars) return refused(tr("A comment can have up to %1 characters. Shorten it, and try again.").arg(jp::office::sheet::kMaxThreadCommentChars));
    ThreadList list = *was->thread;
    list.push_back(commentBy(m_anchors, text, author, list.front().id));
    SideEdit::Note n;
    n.row = row;
    n.col = col;
    n.before = *was;
    n.after = withThread(*was, std::move(list));
    return pushNoteChanges(sheet, {std::move(n)}, stepText);
}

EditSession::Outcome EditSession::editComment(int sheet, quint32 row, quint32 col, int index, const QString &text, const QString &stepText)
{
    const Outcome checked = checkThreadCell(sheet, row, col);
    if (!checked.ok) return checked;
    const NoteInfo *was = m_anchors.noteAt(sheet, row, col);
    if (!was || !was->thread || index < 0 || size_t(index) >= was->thread->size()) return refused(tr("There is no such comment."));
    if (was->threadCut) return refused(tr("This thread has more comments than JeffCrunch changes (%1). It shows them, and keeps them, but doesn't change them.").arg(jp::office::sheet::kMaxThreadComments));
    if (text.trimmed().isEmpty()) return refused(tr("A comment needs some words."));
    if (text.size() > jp::office::sheet::kMaxThreadCommentChars) return refused(tr("A comment can have up to %1 characters. Shorten it, and try again.").arg(jp::office::sheet::kMaxThreadCommentChars));
    if (was->thread->at(size_t(index)).text == text) return Outcome();
    ThreadList list = *was->thread;
    list[size_t(index)].text = text;
    list[size_t(index)].extra.clear();   // (who was mentioned is by place in the words)
    SideEdit::Note n;
    n.row = row;
    n.col = col;
    n.before = *was;
    n.after = withThread(*was, std::move(list));
    return pushNoteChanges(sheet, {std::move(n)}, stepText);
}

EditSession::Outcome EditSession::resolveThread(int sheet, quint32 row, quint32 col, bool resolved, const QString &stepText)
{
    const Outcome checked = checkThreadCell(sheet, row, col);
    if (!checked.ok) return checked;
    const NoteInfo *was = m_anchors.noteAt(sheet, row, col);
    if (!was || !was->thread || was->thread->empty()) return refused(tr("There are no comments on this cell."));
    if (was->threadCut) return refused(tr("This thread has more comments than JeffCrunch changes (%1). It shows them, and keeps them, but doesn't change them.").arg(jp::office::sheet::kMaxThreadComments));
    if (was->thread->front().done == resolved) return Outcome();
    ThreadList list = *was->thread;
    list.front().done = resolved;
    SideEdit::Note n;
    n.row = row;
    n.col = col;
    n.before = *was;
    n.after = withThread(*was, std::move(list));
    return pushNoteChanges(sheet, {std::move(n)}, stepText);
}

EditSession::Outcome EditSession::deleteComment(int sheet, quint32 row, quint32 col, int index, const QString &stepText)
{
    const Outcome checked = checkThreadCell(sheet, row, col);
    if (!checked.ok) return checked;
    const NoteInfo *was = m_anchors.noteAt(sheet, row, col);
    if (!was || !was->thread || index < 0 || size_t(index) >= was->thread->size()) return refused(tr("There is no such comment."));
    SideEdit::Note n;
    n.row = row;
    n.col = col;
    n.before = *was;
    if (index > 0 && !was->threadCut) {
        // (a reply goes; the replies to it are now replies to the first comment)
        ThreadList list = *was->thread;
        const QString gone = list[size_t(index)].id;
        list.erase(list.begin() + index);
        for (ThreadComment &c : list)
            if (c.parentId == gone) c.parentId = list.front().id;
        n.after = withThread(*was, std::move(list));
    }   // (the first comment goes, and the thread with it)
    return pushNoteChanges(sheet, {std::move(n)}, stepText);
}

EditSession::Outcome EditSession::deleteThreads(int sheet, const std::vector<std::pair<quint32, quint32>> &cells, const QString &stepText)
{
    QString why;
    if (!canEditNotes(sheet, &why)) return refused(why);
    const jp::office::sheet::Sheet &model = m_book->calc->sheet(sheet);
    if (model.isProtected && model.protection.objects) return refused(tr("This sheet is protected, so its comments can't be changed. Take the protection off first."));
    std::vector<SideEdit::Note> notes;
    for (const auto &c : cells) {
        const NoteInfo *was = m_anchors.noteAt(sheet, c.first, c.second);
        if (!was || !was->thread) continue;
        SideEdit::Note n;
        n.row = c.first;
        n.col = c.second;
        n.before = *was;
        notes.push_back(std::move(n));
    }
    return pushNoteChanges(sheet, std::move(notes), stepText);
}

EditSession::Outcome EditSession::convertNotesToComments(int sheet, const std::vector<std::pair<quint32, quint32>> &cells, const QString &author, const QString &stepText)
{
    QString why;
    if (!canEditNotes(sheet, &why)) return refused(why);
    const jp::office::sheet::Sheet &model = m_book->calc->sheet(sheet);
    if (model.isProtected && model.protection.objects) return refused(tr("This sheet is protected, so its comments can't be changed. Take the protection off first."));
    std::vector<SideEdit::Note> notes;
    for (const auto &c : cells) {
        const NoteInfo *was = m_anchors.noteAt(sheet, c.first, c.second);
        if (!was || was->thread || was->text.isEmpty()) continue;
        QString text = was->text;
        if (text.size() > jp::office::sheet::kMaxThreadCommentChars) text.truncate(jp::office::sheet::kMaxThreadCommentChars);
        SideEdit::Note n;
        n.row = c.first;
        n.col = c.second;
        n.before = *was;
        n.after = withThread(*was, {commentBy(m_anchors, text, was->author.isEmpty() ? author : was->author, QString())});
        notes.push_back(std::move(n));
    }
    return pushNoteChanges(sheet, std::move(notes), stepText);
}

EditSession::Outcome EditSession::convertCommentsToNotes(int sheet, const std::vector<std::pair<quint32, quint32>> &cells, const QString &stepText)
{
    QString why;
    if (!canEditNotes(sheet, &why)) return refused(why);
    const jp::office::sheet::Sheet &model = m_book->calc->sheet(sheet);
    if (model.isProtected && model.protection.objects) return refused(tr("This sheet is protected, so its comments can't be changed. Take the protection off first."));
    std::vector<SideEdit::Note> notes;
    for (const auto &c : cells) {
        const NoteInfo *was = m_anchors.noteAt(sheet, c.first, c.second);
        if (!was || !was->thread || was->thread->empty() || was->threadCut) continue;
        NoteInfo now;
        now.shown = was->shown;
        // (the note keeps the first comment's author and all the words, each comment under the other)
        now.author = was->thread->front().author;
        QString text;
        for (const ThreadComment &t : *was->thread) {
            if (!text.isEmpty()) text += QStringLiteral("\n\n");
            text += t.text;
        }
        now.text = text.left(32767);
        SideEdit::Note n;
        n.row = c.first;
        n.col = c.second;
        n.before = *was;
        n.after = now;
        notes.push_back(std::move(n));
    }
    return pushNoteChanges(sheet, std::move(notes), stepText);
}

} // namespace jp::crunch
