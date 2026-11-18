// The threads of comments on cells: starting one, replying, editing, resolving, deleting, and turning notes into threads and back. One step of Undo each; the model they are kept in is
// core/anchors.h (a thread is the `thread` of the cell's NoteInfo).
#include "core/editcomments.h"
#include "core/editsession.h"
#include "core/threads.h"
#include "sheet/cellref.h"
#include <QCoreApplication>

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

qint64 g_bookCap = 0;

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
    if (const auto r = jp::threads::checkText(text, true); r != jp::threads::Refusal::None) return refused(jp::threads::refusalText(r));
    if (m_anchors.noteAt(sheet, row, col)) return refused(tr("This cell has a note or comments already. Reply to the comments, or delete them first."));
    if (qint64(m_anchors.notes(sheet).size()) >= jp::office::sheet::kMaxNotes) return refused(tr("A sheet can have up to %1 notes and comments.").arg(jp::office::sheet::kMaxNotes));
    if (threadCommentsIn(m_anchors) >= bookCommentCap()) return refused(bookCapText());
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
    if (const auto r = jp::threads::checkReply(text, int(was->thread->size()) - 1); r != jp::threads::Refusal::None) return refused(jp::threads::refusalText(r));
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
    if (const auto r = jp::threads::checkText(text, true); r != jp::threads::Refusal::None) return refused(jp::threads::refusalText(r));
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
    if (was->threadCut) return refused(tr("This thread has more comments than JeffCrunch changes (%1). It shows them, and keeps them, but doesn't change them.").arg(jp::office::sheet::kMaxThreadComments));
    SideEdit::Note n;
    n.row = row;
    n.col = col;
    n.before = *was;
    if (index > 0) {
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
    int keptCut = 0;
    for (const auto &c : cells) {
        const NoteInfo *was = m_anchors.noteAt(sheet, c.first, c.second);
        if (!was || !was->thread) continue;
        if (was->threadCut) {   // (Undo could not bring back the comments that were never read)
            ++keptCut;
            continue;
        }
        SideEdit::Note n;
        n.row = c.first;
        n.col = c.second;
        n.before = *was;
        notes.push_back(std::move(n));
    }
    if (notes.empty() && keptCut) return refused(tr("This thread has more comments than JeffCrunch changes (%1). It shows them, and keeps them, but doesn't delete them.").arg(jp::office::sheet::kMaxThreadComments));
    return pushNoteChanges(sheet, std::move(notes), stepText);
}

EditSession::Outcome EditSession::convertNotesToComments(int sheet, const std::vector<std::pair<quint32, quint32>> &cells, const QString &author, const QString &stepText)
{
    QString why;
    if (!canEditNotes(sheet, &why)) return refused(why);
    const jp::office::sheet::Sheet &model = m_book->calc->sheet(sheet);
    if (model.isProtected && model.protection.objects) return refused(tr("This sheet is protected, so its comments can't be changed. Take the protection off first."));
    std::vector<SideEdit::Note> notes;
    qint64 total = threadCommentsIn(m_anchors);
    bool stopped = false;
    for (const auto &c : cells) {
        const NoteInfo *was = m_anchors.noteAt(sheet, c.first, c.second);
        if (!was || was->thread || was->text.isEmpty()) continue;
        if (total >= bookCommentCap()) {
            stopped = true;
            break;
        }
        ++total;
        QString text = was->text;
        if (text.size() > jp::office::sheet::kMaxThreadCommentChars) text.truncate(jp::office::sheet::kMaxThreadCommentChars);
        SideEdit::Note n;
        n.row = c.first;
        n.col = c.second;
        n.before = *was;
        n.after = withThread(*was, {commentBy(m_anchors, text, was->author.isEmpty() ? author : was->author, QString())});
        notes.push_back(std::move(n));
    }
    if (stopped && notes.empty()) return refused(bookCapText());
    Outcome out = pushNoteChanges(sheet, std::move(notes), stepText);
    if (stopped && out.ok) out.problem = tr("%1 The notes that are left stay notes.").arg(bookCapText());
    return out;
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

// ---- one set of rules with the other apps (core/threads.h), and the Reviewing Pane's model

namespace {

struct ThreadAt {
    int sheet;
    quint32 row, col;
};

QVector<ThreadAt> threadCells(const CellAnchors &anchors)
{
    QVector<ThreadAt> out;
    for (int s = 0; s < anchors.sheetCount(); ++s)
        for (const auto &n : anchors.notes(s))
            if (n.second.thread && !n.second.thread->empty()) out.push_back({s, quint32(n.first >> 32), quint32(n.first & 0xffffffffu)});
    return out;
}

} // namespace

bool stepThread(const CellAnchors &anchors, int sheet, quint32 row, quint32 col, bool forward, int *toSheet, quint32 *toRow, quint32 *toCol)
{
    const QVector<ThreadAt> cells = threadCells(anchors);
    QVector<qint64> keys;
    for (const ThreadAt &t : cells) keys.push_back(jp::review::crunchPosition(t.sheet, int(t.row), int(t.col)));
    const int at = jp::threads::stepIndex(keys, jp::review::crunchPosition(sheet, int(row), int(col)), forward);
    if (at < 0) return false;
    *toSheet = cells[at].sheet;
    *toRow = cells[at].row;
    *toCol = cells[at].col;
    return true;
}

void setBookCommentCapForTest(qint64 most) { g_bookCap = most; }
qint64 bookCommentCap() { return g_bookCap > 0 ? g_bookCap : jp::office::sheet::kMaxThreadCommentsPerBook; }
QString bookCapText() { return QCoreApplication::translate("jp::crunch::EditSession", "A workbook can have up to %1 comments in threads.").arg(bookCommentCap()); }
qint64 threadCommentsIn(const CellAnchors &anchors)
{
    qint64 n = 0;
    for (int s = 0; s < anchors.sheetCount(); ++s)
        for (const auto &note : anchors.notes(s))
            if (note.second.thread) n += qint64(note.second.thread->size());
    return n;
}

jp::review::ChangeList threadChanges(const CellAnchors &anchors, const std::function<QString(int)> &sheetName)
{
    jp::review::ChangeList out;
    for (const ThreadAt &t : threadCells(anchors)) {
        if (out.changes.size() >= jp::review::kMaxChanges) {
            out.tooMany = true;
            break;
        }
        const NoteInfo *note = anchors.noteAt(t.sheet, t.row, t.col);
        if (!note || !note->thread) continue;
        const ThreadList &list = *note->thread;
        jp::review::Change c;
        const QString cell = jp::office::sheet::cellName(t.row, t.col);
        const QString sheetLabel = sheetName ? sheetName(t.sheet) : QString::number(t.sheet);
        c.id = sheetLabel + QLatin1Char('!') + cell;
        c.kind = jp::review::Kind::Commented;
        c.author = list.front().author;
        c.when = QDateTime::fromString(list.front().time, Qt::ISODate);
        c.target = {sheetLabel, cell, QStringLiteral("thread")};
        c.summary = list.front().text.left(120).replace(QLatin1Char('\n'), QLatin1Char(' '));
        c.state = list.front().done ? jp::review::State::Accepted : jp::review::State::Pending;
        c.position = jp::review::crunchPosition(t.sheet, int(t.row), int(t.col));
        for (size_t i = 1; i < list.size() && c.replies.size() < jp::review::kMaxReplies; ++i)
            c.replies.push_back({list[i].author, QDateTime::fromString(list[i].time, Qt::ISODate), list[i].text});
        c.data.insert(QStringLiteral("resolved"), list.front().done);
        out.changes.push_back(c);
    }
    std::stable_sort(out.changes.begin(), out.changes.end(), [](const jp::review::Change &a, const jp::review::Change &b) { return a.position < b.position; });
    return out;
}

} // namespace jp::crunch
