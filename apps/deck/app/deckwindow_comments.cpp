// JeffDeck's comments: the Review tab's Comments group, the pane beside the
// slide, and the markers on it.

#include "app/commentspane.h"
#include "app/deckcanvas.h"
#include "app/deckwindow.h"
#include "app/editor.h"
#include "core/comments.h"

#include <QAction>
#include <QSplitter>
#include <algorithm>

namespace jp {

// The Comments pane stands to the right of the slide, above the notes. Show
// Comments shows it with the markers on the slide, and hides both.
void DeckWindow::createCommentActions()
{
    m_comments = new CommentsPane(m_ed, this);
    m_comments->hide();
    m_commentSplit = new QSplitter(Qt::Horizontal, this);
    m_rightSplit->insertWidget(0, m_commentSplit);
    m_commentSplit->addWidget(m_canvas);
    m_commentSplit->addWidget(m_comments);
    m_commentSplit->setStretchFactor(0, 1);
    m_commentSplit->setCollapsible(0, false);
    m_rightSplit->setStretchFactor(0, 1);
    m_rightSplit->setCollapsible(0, false);
    m_rightSplit->setSizes({600, 90});

    mk("comment.new", tr("New Comment"), "message-square-plus", QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_M), [this] { newComment(); });
    mk("comment.delete", tr("Delete"), "message-square-x", QKeySequence(), [this] { m_comments->deleteCurrent(); });
    mk("comment.deleteSlide", tr("Delete All Comments on This Slide"), "message-square-x", QKeySequence(), [this] { m_comments->deleteSlideComments(); });
    mk("comment.deleteAll", tr("Delete All Comments in This Presentation"), "message-square-x", QKeySequence(), [this] { m_comments->deleteAllComments(); });
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
        const bool any = d && !deck::allComments(*d).isEmpty();
        act("comment.delete")->setEnabled(!m_comments->currentComment().isEmpty());
        act("comment.deleteSlide")->setEnabled(d && i >= 0 && i < d->pages.size() && deck::commentCount(*d->pages[i]) > 0);
        act("comment.deleteAll")->setEnabled(any);
        act("comment.prev")->setEnabled(any);
        act("comment.next")->setEnabled(any);
    };
    connect(m_ed, &Editor::changed, this, update);
    connect(m_ed, &Editor::pageChanged, this, update);
    connect(m_comments, &CommentsPane::currentCommentChanged, this, [this, update](const QString &id) {
        m_canvas->setActiveComment(id);
        if (!id.isEmpty() && !m_canvas->commentsShown()) showComments(true);
        update();
    });
    connect(m_comments, &CommentsPane::closeRequested, this, [this] { showComments(false); });
    // A marker that is clicked opens its comment in the pane.
    connect(m_canvas, &DeckCanvas::commentClicked, this, [this](const QString &id) { m_comments->setCurrentComment(id); });
    // A presentation that comes with comments shows them.
    connect(m_ed, &Editor::documentReplaced, this, [this, update] {
        if (!deck::allComments(*m_ed->doc()).isEmpty()) showComments(true);
        update();
    });
    update();
}

void DeckWindow::showComments(bool on)
{
    act("comment.show")->setChecked(on);
    m_comments->setVisible(on);
    m_canvas->setCommentsShown(on);
    if (on) m_comments->reload();
    if (on && m_commentSplit->width() > 0) {
        // The pane takes a third of the room at most, not the whole of what it asks for.
        const int pane = std::clamp(m_commentSplit->width() / 4, 260, 360);
        m_commentSplit->setSizes({m_commentSplit->width() - pane, pane});
    }
}

void DeckWindow::newComment()
{
    if (!askCommentAuthor(this)) return;
    showComments(true);
    m_comments->startNewComment();
}

} // namespace jp
