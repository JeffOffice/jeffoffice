// JeffPub's comments: the Review tab's Comments group, the pane beside the page, and the markers on it. The comments themselves, the pane, and the
// markers are the shared ones (libs/page): a page keeps its comments in its page props, so Undo and the .jpub file cover them.

#include "app/commentspane.h"
#include "app/mainwindow.h"
#include "canvas/canvas.h"
#include "core/comments.h"

#include <QAction>
#include <QSplitter>
#include <algorithm>

namespace jp {

// The Comments pane stands to the right of the page. Show Comments shows it with the markers on the page, and hides both.
void MainWindow::createCommentActions()
{
    m_comments = new CommentsPane(m_ed, this);
    m_comments->hide();
    m_comments->setEmptyText(tr("There are no comments on this page."));
    m_comments->setAnchorToObjects(true);
    m_split->insertWidget(m_split->indexOf(m_canvas) + 1, m_comments);

    mk("comment.new", tr("New Comment"), "message-square-plus", QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_M), [this] { newComment(); });
    mk("comment.delete", tr("Delete"), "message-square-x", QKeySequence(), [this] { m_comments->deleteCurrent(); });
    mk("comment.deletePage", tr("Delete All Comments on This Page"), "message-square-x", QKeySequence(), [this] { m_comments->deleteSlideComments(); });
    mk("comment.deleteAll", tr("Delete All Comments in This Publication"), "message-square-x", QKeySequence(), [this] { m_comments->deleteAllComments(); });
    mk("comment.prev", tr("Previous"), "arrow-left", QKeySequence(), [this] {
        showComments(true);
        m_comments->visit(-1);
    });
    mk("comment.next", tr("Next"), "arrow-right", QKeySequence(), [this] {
        showComments(true);
        m_comments->visit(1);
    });
    mk("comment.show", tr("Show Comments"), "messages-square", QKeySequence(), [this] { showComments(act("comment.show")->isChecked()); }, true);

    // What the commands can do follows the comments, whatever changed them.
    auto update = [this] {
        const Document *d = m_ed->doc();
        const int i = m_ed->currentPage();
        const bool any = d && !comments::allComments(*d).isEmpty();
        act("comment.delete")->setEnabled(!m_comments->currentComment().isEmpty());
        act("comment.deletePage")->setEnabled(d && i >= 0 && i < d->pages.size() && comments::commentCount(*d->pages[i]) > 0);
        act("comment.deleteAll")->setEnabled(any);
        act("comment.prev")->setEnabled(any);
        act("comment.next")->setEnabled(any);
    };
    connect(m_ed, &Editor::changed, this, update);
    connect(m_ed, &Editor::pageChanged, this, update);
    connect(m_comments, &CommentsPane::currentCommentChanged, this, [this, update](const QString &id) {
        m_canvas->setActiveComment(id);
        if (!id.isEmpty() && !m_canvas->commentsShown()) showComments(true);
        // The page scrolls to the comment's marker.
        const Document *d = m_ed->doc();
        const int i = m_ed->currentPage();
        if (!id.isEmpty() && d && i >= 0 && i < d->pages.size())
            if (const auto c = comments::findComment(*d->pages[i], id)) m_canvas->ensureVisible(QRectF(comments::commentPlace(*d, *c), QSizeF(24, 24)));
        update();
    });
    connect(m_comments, &CommentsPane::closeRequested, this, [this] { showComments(false); });
    // A marker that is clicked opens its comment in the pane.
    connect(m_canvas, &Canvas::commentClicked, this, [this](const QString &id) { m_comments->setCurrentComment(id); });
    // A publication that comes with comments shows them.
    connect(m_ed, &Editor::documentReplaced, this, [this, update] {
        if (!comments::allComments(*m_ed->doc()).isEmpty()) showComments(true);
        update();
    });
    update();
}

void MainWindow::showComments(bool on)
{
    act("comment.show")->setChecked(on);
    m_comments->setVisible(on);
    m_canvas->setCommentsShown(on);
    if (on) m_comments->reload();
    if (on) {
        // The pane takes a quarter of the room at most, not the whole of what it asks for.
        QList<int> sizes = m_split->sizes();
        const int at = m_split->indexOf(m_comments), canvas = m_split->indexOf(m_canvas);
        if (sizes.value(at) < 240) {
            const int pane = std::clamp(sizes.value(canvas) / 4, 260, 360);
            if (sizes.value(canvas) > pane + 200) {
                sizes[canvas] -= pane;
                sizes[at] = pane;
                m_split->setSizes(sizes);
            }
        }
    }
}

void MainWindow::newComment()
{
    if (!askCommentAuthor(this)) return;
    showComments(true);
    m_comments->startNewComment();
}

} // namespace jp
