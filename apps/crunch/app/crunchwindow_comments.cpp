// JeffCrunch's comments (the newer kind of note, a thread with replies): the commands (New Comment, Reply, Resolve or Reopen, Delete, Previous and Next, Show Comments, and turning notes into
// comments and back), the thread that appears when the pointer rests on a cell, and the Comments pane. The threads are kept by the session (core/anchors.h, core/editcomments.cpp); this is
// how the window shows them and asks for them.

#include "app/crunchwindow.h"

#include "app/anchordialogs.h"
#include "app/threadspane.h"
#include "app/plaintext.h"
#include "core/anchors.h"

#include <QAction>
#include <QDateTime>
#include <QDockWidget>
#include <QLocale>
#include <QUndoStack>

#include <algorithm>

namespace jp {

using jp::office::sheet::CellRange;
using jp::office::sheet::NoteInfo;
using jp::office::sheet::ThreadComment;
using jp::office::sheet::noteKey;

namespace {

// The name the person is known by in the comments they write: the computer account's.
QString commentAuthor()
{
    QString name = qEnvironmentVariable("USERNAME");
    if (name.isEmpty()) name = qEnvironmentVariable("USER");
    return name.trimmed();
}

// A comment's time as a person reads it (the file's time is the form 2026-11-07T09:30:00.00).
QString shownTime(const QString &time)
{
    const QDateTime t = QDateTime::fromString(time.left(19), Qt::ISODate);
    return t.isValid() ? QLocale().toString(t, QLocale::ShortFormat) : time;
}

} // namespace

// What a thread reads like where the pointer rests: each comment with who wrote it and when, the replies set in, and a line that says it is resolved.
QString CrunchWindow::threadTipText(const NoteInfo &note)
{
    if (!note.thread) return note.text;
    QString out;
    if (jp::office::sheet::threadResolved(*note.thread)) out += tr("Resolved") + QStringLiteral("\n");
    int shown = 0;
    const std::vector<int> depths = jp::office::sheet::threadDepths(*note.thread);
    for (size_t i = 0; i < note.thread->size() && shown < CommentThreadsPane::kMaxShownComments; ++i, ++shown) {
        const ThreadComment &c = note.thread->at(i);
        const QString indent(2 * std::min(depths[i], 3), QLatin1Char(' '));
        const QString who = c.author.isEmpty() ? tr("Unknown") : c.author;
        out += indent + (c.time.isEmpty() ? who : tr("%1, %2").arg(who, shownTime(c.time))) + QLatin1Char('\n') + indent + c.text + QLatin1Char('\n');
    }
    if (note.thread->size() > size_t(shown)) out += tr("and %1 more").arg(qint64(note.thread->size()) - shown) + QLatin1Char('\n');
    return out.trimmed();
}

void CrunchWindow::createCommentActions()
{
    mk("comments.new", tr("New Comment"), "message-square-plus", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F2), [this] { newComment(); }, false, false);
    mk("comments.reply", tr("Reply"), "message-square", QKeySequence(), [this] { replyToThread(); }, false, false);
    mk("comments.resolve", tr("Resolve Thread"), "check", QKeySequence(), [this] { toggleThreadResolved(); }, false, false);
    mk("comments.delete", tr("Delete Comment"), "message-square-x", QKeySequence(), [this] { deleteSelectedThreads(); }, false, false);
    mk("comments.prev", tr("Previous Comment"), "chevron-left", QKeySequence(), [this] { goToComment(false); }, false, false);
    mk("comments.next", tr("Next Comment"), "chevron-right", QKeySequence(), [this] { goToComment(true); }, false, false);
    mk("comments.show", tr("Show Comments"), "messages-square", QKeySequence(), [this] { showCommentsPane(act("comments.show")->isChecked()); }, true, false);
    mk("comments.convert", tr("Convert Notes to Comments"), "message-square-plus", QKeySequence(), [this] { convertNotesToComments(); }, false, false);
    mk("comments.revert", tr("Convert Comments to Notes"), "sticky-note", QKeySequence(), [this] { convertCommentsToNotes(); }, false, false);
    connect(m_view, &grid::GridView::selectionChanged, this, [this] {
        if (m_commentsPane && m_commentsDock && m_commentsDock->isVisible()) fillComments();
    });
}

void CrunchWindow::updateCommentActions()
{
    const bool open = m_session && !m_saving && m_sheet >= 0 && !m_chartSheetShown && m_loaded && m_loaded->calc;
    const bool can = open && m_session->canEditNotes(m_sheet);
    const grid::Selection &sel = m_view->selection();
    const quint32 row = quint32(std::max<qint64>(0, sel.activeRow)), col = quint32(std::max<qint64>(0, sel.activeCol));
    const NoteInfo *here = open ? m_session->anchors().noteAt(m_sheet, row, col) : nullptr;
    const bool thread = here && here->thread && !here->thread->empty();
    act("comments.new")->setEnabled(can && !here);
    act("comments.reply")->setEnabled(can && thread && !here->threadCut);
    QAction *resolve = act("comments.resolve");
    resolve->setEnabled(can && thread && !here->threadCut);
    resolve->setText(thread && jp::office::sheet::threadResolved(*here->thread) ? tr("Reopen Thread") : tr("Resolve Thread"));
    bool anyThreads = false, anyPlain = false, anyInSelection = false, anyConvertible = false;
    if (open) {
        const CellRange r = selectedRange();
        for (const auto &n : m_session->anchors().notes(m_sheet)) {
            const bool isThread = n.second.thread && !n.second.thread->empty();
            anyThreads = anyThreads || isThread;
            anyPlain = anyPlain || (!isThread && !n.second.text.isEmpty());
            anyConvertible = anyConvertible || (isThread && !n.second.threadCut);
            if (isThread && r.contains(quint32(n.first >> 32), quint32(n.first & 0xFFFFFFFFu))) anyInSelection = true;
        }
    }
    act("comments.delete")->setEnabled(can && anyInSelection);
    act("comments.prev")->setEnabled(anyThreads);
    act("comments.next")->setEnabled(anyThreads);
    act("comments.show")->setEnabled(open);
    act("comments.convert")->setEnabled(can && anyPlain);
    act("comments.revert")->setEnabled(can && anyConvertible);
}

bool CrunchWindow::askCommentText(const QString &title, const QString &where, QString *text)
{
    NoteRequest req;
    req.where = where;
    req.title = title;
    req.author = commentAuthor();
    if (m_anchorHooks.noteAsker) {
        if (!m_anchorHooks.noteAsker(&req)) return false;
    } else {
        NoteDialog dialog(req, this);
        if (dialog.exec() != QDialog::Accepted) return false;
        req.text = dialog.text();
    }
    *text = req.text;
    return !text->trimmed().isEmpty();
}

void CrunchWindow::newComment()
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!m_session || m_sheet < 0) return;
    const grid::Selection &sel = m_view->selection();
    const quint32 row = quint32(std::max<qint64>(0, sel.activeRow)), col = quint32(std::max<qint64>(0, sel.activeCol));
    QString why;
    if (!m_session->canEditNotes(m_sheet, &why)) {
        notice(tr("Comment"), why);
        return;
    }
    QString text;
    if (!askCommentText(tr("New comment on %1").arg(jp::office::sheet::cellName(row, col)), jp::office::sheet::cellName(row, col), &text)) return;
    const crunch::EditSession::Outcome o = m_session->newComment(m_sheet, row, col, text, commentAuthor(), tr("New Comment"));
    if (!o.ok) notice(tr("Comment"), o.problem);
    else showCommentsPane(true);
    m_view->setFocus();
}

void CrunchWindow::replyToThreadAt(quint32 row, quint32 col)
{
    if (!m_session || m_sheet < 0) return;
    QString text;
    if (!askCommentText(tr("Reply to the comments on %1").arg(jp::office::sheet::cellName(row, col)), jp::office::sheet::cellName(row, col), &text)) return;
    const crunch::EditSession::Outcome o = m_session->replyToComment(m_sheet, row, col, text, commentAuthor(), tr("Reply"));
    if (!o.ok) notice(tr("Comment"), o.problem);
}

void CrunchWindow::replyToThread()
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    const grid::Selection &sel = m_view->selection();
    replyToThreadAt(quint32(std::max<qint64>(0, sel.activeRow)), quint32(std::max<qint64>(0, sel.activeCol)));
    m_view->setFocus();
}

void CrunchWindow::setThreadResolvedAt(quint32 row, quint32 col, bool resolved)
{
    if (!m_session || m_sheet < 0) return;
    const crunch::EditSession::Outcome o = m_session->resolveThread(m_sheet, row, col, resolved, resolved ? tr("Resolve Thread") : tr("Reopen Thread"));
    if (!o.ok) notice(tr("Comment"), o.problem);
}

void CrunchWindow::toggleThreadResolved()
{
    if (!m_session || m_sheet < 0) return;
    const grid::Selection &sel = m_view->selection();
    const quint32 row = quint32(std::max<qint64>(0, sel.activeRow)), col = quint32(std::max<qint64>(0, sel.activeCol));
    const NoteInfo *here = m_session->anchors().noteAt(m_sheet, row, col);
    if (!here || !here->thread || here->thread->empty()) return;
    setThreadResolvedAt(row, col, !jp::office::sheet::threadResolved(*here->thread));
}

void CrunchWindow::deleteThreadsAt(const std::vector<std::pair<quint32, quint32>> &cells)
{
    if (!m_session || m_sheet < 0 || cells.empty()) return;
    const crunch::EditSession::Outcome o = m_session->deleteThreads(m_sheet, cells, cells.size() == 1 ? tr("Delete Comment") : tr("Delete Comments"));
    if (!o.ok) notice(tr("Comment"), o.problem);
}

void CrunchWindow::deleteSelectedThreads()
{
    if (!m_session || m_sheet < 0) return;
    std::vector<std::pair<quint32, quint32>> cells;
    const CellRange r = selectedRange();
    const auto &notes = m_session->anchors().notes(m_sheet);
    for (auto it = notes.lower_bound(noteKey(r.row1, 0)); it != notes.end() && quint32(it->first >> 32) <= r.row2; ++it) {
        const quint32 c = quint32(it->first & 0xFFFFFFFFu);
        if (c >= r.col1 && c <= r.col2 && it->second.thread) cells.emplace_back(quint32(it->first >> 32), c);
    }
    deleteThreadsAt(cells);
}

// The next (or previous) cell with a thread, going by rows and wrapping round.
void CrunchWindow::goToComment(bool next)
{
    if (!m_session || m_sheet < 0) return;
    std::vector<quint64> keys;
    for (const auto &n : m_session->anchors().notes(m_sheet))
        if (n.second.thread) keys.push_back(n.first);
    if (keys.empty()) return;
    const grid::Selection &sel = m_view->selection();
    const quint64 here = noteKey(quint32(std::max<qint64>(0, sel.activeRow)), quint32(std::max<qint64>(0, sel.activeCol)));
    quint64 key;
    if (next) {
        const auto it = std::upper_bound(keys.begin(), keys.end(), here);
        key = it == keys.end() ? keys.front() : *it;
    } else {
        const auto it = std::lower_bound(keys.begin(), keys.end(), here);
        key = it == keys.begin() ? keys.back() : *std::prev(it);
    }
    m_view->selectCell(qint64(key >> 32), qint64(key & 0xFFFFFFFFu));
    m_view->ensureVisible(qint64(key >> 32), qint64(key & 0xFFFFFFFFu));
}

void CrunchWindow::convertNotesToComments()
{
    if (!m_session || m_sheet < 0) return;
    std::vector<std::pair<quint32, quint32>> cells;
    for (const auto &n : m_session->anchors().notes(m_sheet))
        if (!n.second.thread) cells.emplace_back(quint32(n.first >> 32), quint32(n.first & 0xFFFFFFFFu));
    const crunch::EditSession::Outcome o = m_session->convertNotesToComments(m_sheet, cells, commentAuthor(), tr("Convert Notes to Comments"));
    if (!o.ok) notice(tr("Comment"), o.problem);
}

void CrunchWindow::convertCommentsToNotes()
{
    if (!m_session || m_sheet < 0) return;
    std::vector<std::pair<quint32, quint32>> cells;
    bool replies = false;
    for (const auto &n : m_session->anchors().notes(m_sheet))
        if (n.second.thread && !n.second.threadCut) {
            cells.emplace_back(quint32(n.first >> 32), quint32(n.first & 0xFFFFFFFFu));
            replies = replies || n.second.thread->size() > 1;
        }
    if (replies && !confirm(tr("Convert Comments to Notes"), tr("A note has no replies, and doesn't say who wrote each comment or when. The comments of a thread become one note with all their words. Convert them?"))) return;
    const crunch::EditSession::Outcome o = m_session->convertCommentsToNotes(m_sheet, cells, tr("Convert Comments to Notes"));
    if (!o.ok) notice(tr("Comment"), o.problem);
}

void CrunchWindow::showCommentsPane(bool show)
{
    if (show && !m_commentsPane) {
        m_commentsDock = new QDockWidget(tr("Comments"), this);
        m_commentsDock->setObjectName(QStringLiteral("commentsDock"));
        m_commentsDock->setFeatures(QDockWidget::DockWidgetClosable);
        m_commentsPane = new CommentThreadsPane(m_commentsDock);
        m_commentsDock->setWidget(m_commentsPane);
        addDockWidget(Qt::RightDockWidgetArea, m_commentsDock.data());
        resizeDocks({m_commentsDock.data()}, {300}, Qt::Horizontal);
        connect(m_commentsPane.data(), &CommentThreadsPane::threadChosen, this, [this](quint32 row, quint32 col) {
            m_view->selectCell(row, col);
            m_view->ensureVisible(row, col);
        });
        connect(m_commentsPane.data(), &CommentThreadsPane::replyRequested, this, [this](quint32 row, quint32 col) { replyToThreadAt(row, col); });
        connect(m_commentsPane.data(), &CommentThreadsPane::resolveRequested, this, [this](quint32 row, quint32 col, bool resolved) { setThreadResolvedAt(row, col, resolved); });
        connect(m_commentsPane.data(), &CommentThreadsPane::deleteRequested, this, [this](quint32 row, quint32 col) { deleteThreadsAt({{row, col}}); });
        connect(m_commentsDock.data(), &QDockWidget::visibilityChanged, this, [this](bool visible) {
            if (QAction *a = act(QStringLiteral("comments.show"))) {
                const QSignalBlocker block(a);
                a->setChecked(visible);
            }
            if (visible) fillComments();
        });
    }
    if (!m_commentsDock) return;
    m_commentsDock->setVisible(show);
    if (QAction *a = act(QStringLiteral("comments.show"))) {
        const QSignalBlocker block(a);
        a->setChecked(show);
    }
    if (show) fillComments();
}

// The cards of the pane: the sheet's threads in the order of their cells.
void CrunchWindow::fillComments()
{
    if (!m_commentsPane) return;
    std::vector<CommentThreadsPane::Thread> threads;
    int more = 0;
    const grid::Selection &sel = m_view->selection();
    if (m_session && m_sheet >= 0 && !m_chartSheetShown)
        for (const auto &n : m_session->anchors().notes(m_sheet)) {
            if (!n.second.thread || n.second.thread->empty()) continue;
            if (threads.size() >= size_t(CommentThreadsPane::kMaxCards)) {
                ++more;
                continue;
            }
            CommentThreadsPane::Thread t;
            t.row = quint32(n.first >> 32);
            t.col = quint32(n.first & 0xFFFFFFFFu);
            t.cell = jp::office::sheet::cellName(t.row, t.col);
            t.resolved = jp::office::sheet::threadResolved(*n.second.thread);
            t.locked = n.second.threadCut;
            const std::vector<int> depths = jp::office::sheet::threadDepths(*n.second.thread);
            for (size_t i = 0; i < n.second.thread->size(); ++i) {
                if (i >= size_t(CommentThreadsPane::kMaxShownComments)) {
                    t.more = int(n.second.thread->size() - i);
                    break;
                }
                const ThreadComment &c = n.second.thread->at(i);
                t.comments.push_back({c.author, shownTime(c.time), c.text, depths[i]});
            }
            threads.push_back(std::move(t));
        }
    m_commentsPane->setThreads(threads, sel.activeRow, sel.activeCol, more);
}

} // namespace jp
