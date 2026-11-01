// JeffWrite's Review tab: Spelling & Grammar, Track Changes, Accept and Reject, Display for Review, and comments, with the Reviewing pane.

#include "app/writewindow.h"

#include "app/pageview.h"
#include "app/reviewpane.h"
#include "app/settings.h"
#include "app/spellpane.h"
#include "app/writefilescreen.h"
#include "core/commentops.h"
#include "core/editops.h"
#include "core/reviewops.h"
#include "text/textprops.h"

#include <QAction>
#include <QActionGroup>
#include <QDockWidget>
#include <QInputDialog>
#include <QMenu>
#include <QStatusBar>
#include <QToolButton>
#include <QTreeWidget>

namespace jp {

namespace {

// The user name and initials of the settings: who makes changes and comments.
QString userInitials()
{
    return Settings::get().value(QStringLiteral("user/initials")).toString();
}

} // namespace

void WriteWindow::createReviewActions()
{
    mk("review.spelling", tr("Spelling & Grammar"), "spell-check", QKeySequence(Qt::Key_F7), [this] { showSpelling(); });
    QAction *asType = mk("review.checkAsType", tr("Check Spelling as You Type"), "spell-check-2", QKeySequence(), [this] {
        const bool on = act("review.checkAsType")->isChecked();
        m_view->setSpellingEnabled(on);
        Settings::get().setValue(QStringLiteral("proof/asYouType"), on);
    }, true);
    const bool on = Settings::get().value(QStringLiteral("proof/asYouType"), true).toBool();
    asType->setChecked(on);
    m_view->setSpellingEnabled(on);

    // ---- tracking
    mk("review.trackChanges", tr("Track Changes"), "pencil-line", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_E), [this] {
        if (!m_doc) return;
        m_doc->setTrackChanges(act("review.trackChanges")->isChecked());
        statusBar()->showMessage(m_doc->trackChanges() ? tr("Track Changes is on.") : tr("Track Changes is off."), 4000);
        m_view->setFocus(Qt::OtherFocusReason);
        refreshReviewUi();
    }, true);
    mk("review.accept", tr("Accept"), "check", QKeySequence(), [this] {
        if (!m_doc) return;
        m_view->leaveHeaderFooter();
        m_view->leaveNote();
        QTextCursor c = m_view->cursor();
        write::resolveAtCursor(*m_doc, c, true);
        m_view->showCursor(c);
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("review.reject", tr("Reject"), "x", QKeySequence(), [this] {
        if (!m_doc) return;
        m_view->leaveHeaderFooter();
        m_view->leaveNote();
        QTextCursor c = m_view->cursor();
        write::resolveAtCursor(*m_doc, c, false);
        m_view->showCursor(c);
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("review.acceptShown", tr("Accept All Changes Shown"), "check-check", QKeySequence(), [this] { resolveShown(true); });
    mk("review.rejectShown", tr("Reject All Changes Shown"), "circle-x", QKeySequence(), [this] { resolveShown(false); });
    mk("review.acceptAll", tr("Accept All Changes"), "check-check", QKeySequence(), [this] {
        if (!m_doc) return;
        m_view->leaveHeaderFooter();
        m_view->leaveNote();
        const int n = write::acceptAll(*m_doc);
        statusBar()->showMessage(tr("%n change(s) accepted.", nullptr, n), 5000);
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("review.acceptAllStop", tr("Accept All Changes and Stop Tracking"), "check-check", QKeySequence(), [this] {
        if (!m_doc) return;
        m_view->leaveHeaderFooter();
        m_view->leaveNote();
        const int n = write::acceptAll(*m_doc);
        m_doc->setTrackChanges(false);
        statusBar()->showMessage(tr("%n change(s) accepted. Track Changes is off.", nullptr, n), 5000);
        m_view->setFocus(Qt::OtherFocusReason);
        refreshReviewUi();
    });
    mk("review.rejectAll", tr("Reject All Changes"), "circle-x", QKeySequence(), [this] {
        if (!m_doc) return;
        m_view->leaveHeaderFooter();
        m_view->leaveNote();
        const int n = write::rejectAll(*m_doc);
        statusBar()->showMessage(tr("%n change(s) rejected.", nullptr, n), 5000);
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("review.nextChange", tr("Next Change"), "arrow-down", QKeySequence(), [this] { gotoChange(true); });
    mk("review.previousChange", tr("Previous Change"), "arrow-up", QKeySequence(), [this] { gotoChange(false); });

    // ---- Display for Review
    m_viewGroup = new QActionGroup(this);
    auto viewAction = [this](const char *id, const QString &text, ReviewView view) {
        QAction *a = mk(QString::fromLatin1(id), text, "", QKeySequence(), [this, view] { setReviewView(view); }, true);
        m_viewGroup->addAction(a);
        return a;
    };
    viewAction("review.view.simple", tr("Simple Markup"), ReviewView::SimpleMarkup);
    viewAction("review.view.all", tr("All Markup"), ReviewView::AllMarkup)->setChecked(true);
    viewAction("review.view.none", tr("No Markup"), ReviewView::NoMarkup);
    viewAction("review.view.original", tr("Original"), ReviewView::Original);
    auto showAction = [this](const char *id, const QString &text, bool *flag, const char *key) {
        QAction *a = mk(QString::fromLatin1(id), text, "", QKeySequence(), [this, flag, key, id] {
            *flag = act(QString::fromLatin1(id))->isChecked();
            Settings::get().setValue(QString::fromLatin1(key), *flag);
            applyReviewView();
        }, true);
        *flag = Settings::get().value(QString::fromLatin1(key), true).toBool();
        a->setChecked(*flag);
        return a;
    };
    showAction("review.show.insdel", tr("Insertions and Deletions"), &m_showInsertionsAndDeletions, "review/showInsertionsAndDeletions");
    showAction("review.show.formatting", tr("Formatting"), &m_showFormatting, "review/showFormatting");
    showAction("review.show.comments", tr("Comments"), &m_showComments, "review/showComments");
    m_balloonGroup = new QActionGroup(this);
    auto balloonAction = [this](const char *id, const QString &text, int kind) {
        QAction *a = mk(QString::fromLatin1(id), text, "", QKeySequence(), [this, kind] {
            m_balloons = kind;
            Settings::get().setValue(QStringLiteral("review/balloons"), kind);
            applyReviewView();
        }, true);
        m_balloonGroup->addAction(a);
        return a;
    };
    balloonAction("review.balloons.inline", tr("Show All Revisions Inline"), 0);
    balloonAction("review.balloons.balloons", tr("Show Revisions in Balloons"), 1);
    balloonAction("review.balloons.comments", tr("Show Only Comments and Formatting in Balloons"), 2);
    m_balloons = std::clamp(Settings::get().value(QStringLiteral("review/balloons"), 0).toInt(), 0, 2);
    act(m_balloons == 1 ? "review.balloons.balloons" : m_balloons == 2 ? "review.balloons.comments" : "review.balloons.inline")->setChecked(true);
    mk("review.reviewingPane", tr("Reviewing Pane"), "panel-right-open", QKeySequence(), [this] { showReviewPane(true); });

    // ---- comments
    mk("review.newComment", tr("New Comment"), "message-square-plus", QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_M), [this] { newComment(); });
    mk("review.deleteComment", tr("Delete Comment"), "message-square-x", QKeySequence(), [this] {
        if (!m_doc) return;
        const std::optional<write::Comment> c = write::commentAt(*m_doc, m_view->cursor().position());
        if (!c) {
            statusBar()->showMessage(tr("The caret is not in a comment."), 4000);
            return;
        }
        write::deleteComment(*m_doc, c->parent >= 0 ? c->parent : c->id);
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("review.deleteAllComments", tr("Delete All Comments in Document"), "message-square-x", QKeySequence(), [this] {
        if (!m_doc) return;
        const int n = write::deleteAllComments(*m_doc);
        statusBar()->showMessage(tr("%n comment(s) deleted.", nullptr, n), 5000);
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("review.nextComment", tr("Next Comment"), "arrow-down", QKeySequence(), [this] { gotoComment(true); });
    mk("review.previousComment", tr("Previous Comment"), "arrow-up", QKeySequence(), [this] { gotoComment(false); });
    mk("review.replyComment", tr("Reply to Comment"), "message-square-reply", QKeySequence(), [this] {
        if (!m_doc) return;
        const std::optional<write::Comment> c = write::commentAt(*m_doc, m_view->cursor().position());
        if (!c) {
            statusBar()->showMessage(tr("The caret is not in a comment."), 4000);
            return;
        }
        showReviewPane(false);
        if (m_review) {
            m_review->refresh();
            for (int i = 0; i < m_review->commentList()->topLevelItemCount(); ++i)
                if (m_review->commentList()->topLevelItem(i)->data(0, Qt::UserRole).toInt() == c->id) m_review->commentList()->topLevelItem(i)->setSelected(true);
            m_review->replyField()->setFocus();
        }
    });
    mk("review.resolveComment", tr("Resolve Comment"), "check", QKeySequence(), [this] {
        if (!m_doc) return;
        const std::optional<write::Comment> c = write::commentAt(*m_doc, m_view->cursor().position());
        if (!c) {
            statusBar()->showMessage(tr("The caret is not in a comment."), 4000);
            return;
        }
        const int root = c->parent >= 0 ? c->parent : c->id;
        write::setCommentDone(*m_doc, root, !c->done);
        m_view->setFocus(Qt::OtherFocusReason);
    });
    connect(m_viewGroup, &QActionGroup::triggered, this, [this] { m_view->viewport()->update(); });
}

void WriteWindow::showSpelling()
{
    if (!m_doc) return;
    m_view->leaveHeaderFooter();   // (the Spelling pane goes through the text of the document, not its headers and footers)
    m_view->leaveNote();
    if (m_fileScreen->isVisible()) hideFileScreen();
    if (!m_spellDock) {
        m_spellDock = new QDockWidget(tr("Spelling"), this);
        m_spellDock->setObjectName(QStringLiteral("spellDock"));
        m_spellDock->setFeatures(QDockWidget::DockWidgetClosable);
        m_spellPane = new write::SpellPane(m_view, m_spellDock);
        m_spellDock->setWidget(m_spellPane);
        addDockWidget(Qt::RightDockWidgetArea, m_spellDock);
        resizeDocks({m_spellDock}, {320}, Qt::Horizontal);
    }
    m_spellDock->show();
    m_spellDock->raise();
    m_spellPane->start();
}

void WriteWindow::showReviewPane(bool changes)
{
    if (!m_doc) return;
    if (m_fileScreen->isVisible()) hideFileScreen();
    if (!m_review) {
        m_reviewDock = new QDockWidget(tr("Reviewing"), this);
        m_reviewDock->setObjectName(QStringLiteral("reviewDock"));
        m_reviewDock->setFeatures(QDockWidget::DockWidgetClosable);
        m_review = new ReviewPane(this, m_reviewDock);
        m_reviewDock->setWidget(m_review);
        addDockWidget(Qt::RightDockWidgetArea, m_reviewDock);
        resizeDocks({m_reviewDock}, {340}, Qt::Horizontal);
        connect(m_view, &write::PageView::contentChanged, m_review, [this] { QTimer::singleShot(0, m_review, [this] { if (m_review->isVisible()) m_review->refresh(); }); });
        connect(m_doc.get(), &write::WriteDoc::historyChanged, m_review, [this] { QTimer::singleShot(0, m_review, [this] { if (m_review->isVisible()) m_review->refresh(); }); });
    }
    m_reviewDock->show();
    m_reviewDock->raise();
    m_review->showPage(changes ? ReviewPane::Changes : ReviewPane::Comments);
    m_review->refresh();
}

// How the document shows its changes and comments now.
void WriteWindow::setReviewView(ReviewView view)
{
    m_reviewView = view;
    const char *id = view == ReviewView::SimpleMarkup ? "review.view.simple" : view == ReviewView::AllMarkup ? "review.view.all" : view == ReviewView::NoMarkup ? "review.view.none" : "review.view.original";
    if (QAction *a = act(QString::fromLatin1(id))) a->setChecked(true);
    applyReviewView();
}

void WriteWindow::applyReviewView()
{
    if (!m_doc) return;
    int flags = 0;
    write::PageView::ReviewMarks marks;
    switch (m_reviewView) {
    case ReviewView::AllMarkup:
        if (m_showInsertionsAndDeletions) flags = m_balloons == 1 ? tp::kRevisionMark : (tp::kRevisionShowDeleted | tp::kRevisionMark);
        marks.changeBars = m_showInsertionsAndDeletions || m_showFormatting;
        marks.deletions = m_showInsertionsAndDeletions && m_balloons == 1;
        marks.comments = m_showComments;
        break;
    case ReviewView::SimpleMarkup:
        marks.changeBars = true;
        marks.comments = m_showComments;
        break;
    case ReviewView::NoMarkup:
        break;
    case ReviewView::Original:
        flags = tp::kRevisionShowDeleted | tp::kRevisionHideInserted;
        break;
    }
    m_doc->setRevisionView(flags);
    m_view->setReviewMarks(marks);
    m_view->viewport()->update();
    refreshReviewUi();
}

void WriteWindow::refreshReviewUi()
{
    if (m_updatingReview) return;
    m_updatingReview = true;
    const bool has = m_doc != nullptr;
    for (QAction *a : m_actions)
        if (a->objectName().startsWith(QLatin1String("review.")) && a->objectName() != QLatin1String("review.spelling") && a->objectName() != QLatin1String("review.checkAsType")) a->setEnabled(has);
    if (has) {
        act("review.trackChanges")->setChecked(m_doc->trackChanges());
        QTextCursor &c = m_view->cursor();
        const bool any = write::revisionAt(m_doc->story(), c.position()).has_value() || c.hasSelection();
        act("review.accept")->setEnabled(any || write::nextRevision(m_doc->story(), c.position()).has_value());
        act("review.reject")->setEnabled(any || write::nextRevision(m_doc->story(), c.position()).has_value());
        act("review.deleteComment")->setEnabled(write::commentAt(*m_doc, c.position()).has_value());
        act("review.replyComment")->setEnabled(write::commentAt(*m_doc, c.position()).has_value());
        act("review.resolveComment")->setEnabled(write::commentAt(*m_doc, c.position()).has_value());
    }
    if (m_trackLabel) {
        m_trackLabel->setVisible(has);
        m_trackLabel->setText(has && m_doc->trackChanges() ? tr("Track Changes: On") : tr("Track Changes: Off"));
    }
    m_updatingReview = false;
}

void WriteWindow::newComment()
{
    if (!m_doc) return;
    m_view->leaveHeaderFooter();
    m_view->leaveNote();
    bool ok = false;
    const QString text = QInputDialog::getMultiLineText(this, tr("New Comment"), tr("Comment:"), QString(), &ok);
    if (!ok || text.trimmed().isEmpty()) return;
    QTextCursor c = m_view->cursor();
    const int id = write::addComment(*m_doc, c, text);
    if (id < 0) {
        statusBar()->showMessage(tr("A comment can't be added here."), 5000);
        return;
    }
    m_view->setCursor(c);
    if (!m_showComments) act("review.show.comments")->trigger();
    showReviewPane(false);
    m_view->setFocus(Qt::OtherFocusReason);
}

void WriteWindow::gotoChange(bool forward)
{
    if (!m_doc) return;
    m_view->leaveHeaderFooter();
    m_view->leaveNote();
    QTextDocument *s = m_doc->story();
    const QTextCursor &cur = m_view->cursor();
    const std::optional<write::Revision> r = forward ? write::nextRevision(s, cur.selectionEnd()) : write::previousRevision(s, cur.selectionStart());
    if (!r) {
        statusBar()->showMessage(tr("There are no more changes."), 4000);
        return;
    }
    QTextCursor c(s);
    c.setPosition(std::min(r->from, s->characterCount() - 1));
    c.setPosition(std::min(std::max(r->from, r->to), s->characterCount() - 1), QTextCursor::KeepAnchor);
    m_view->showCursor(c);
    m_view->setFocus(Qt::OtherFocusReason);
}

void WriteWindow::gotoComment(bool forward)
{
    if (!m_doc) return;
    m_view->leaveHeaderFooter();
    m_view->leaveNote();
    const QTextCursor &cur = m_view->cursor();
    const std::optional<write::Comment> c = forward ? write::nextComment(*m_doc, cur.selectionEnd()) : write::previousComment(*m_doc, cur.selectionStart());
    if (!c) {
        statusBar()->showMessage(tr("There are no more comments."), 4000);
        return;
    }
    QTextDocument *s = m_doc->story();
    QTextCursor t(s);
    t.setPosition(std::min(c->from, s->characterCount() - 1));
    t.setPosition(std::min(std::max(c->from, c->to + 1), s->characterCount() - 1), QTextCursor::KeepAnchor);
    m_view->showCursor(t);
    m_view->setFocus(Qt::OtherFocusReason);
}

// The changes the way they are shown now covers (the kinds Show Markup has on; with the Reviewing pane open, the reviewer chosen in it).
QVector<write::Revision> WriteWindow::shownRevisions() const
{
    QVector<write::Revision> out;
    if (!m_doc) return out;
    const QString person = m_review && m_reviewDock && m_reviewDock->isVisible() ? m_review->chosenPerson() : QString();
    for (const write::Revision &r : write::revisionsIn(m_doc->story())) {
        const bool formatting = r.kind == write::Revision::Format || r.kind == write::Revision::ParagraphFormat;
        if (formatting ? !m_showFormatting : !m_showInsertionsAndDeletions) continue;
        if (m_reviewView == ReviewView::NoMarkup) continue;
        if (!person.isEmpty() && r.author != person) continue;
        out.push_back(r);
    }
    return out;
}

int WriteWindow::resolveShown(bool accept)
{
    if (!m_doc) return 0;
    m_view->leaveHeaderFooter();
    m_view->leaveNote();
    const int n = write::resolveRevisions(*m_doc, m_doc->story(), shownRevisions(), accept);
    statusBar()->showMessage(accept ? tr("%n change(s) accepted.", nullptr, n) : tr("%n change(s) rejected.", nullptr, n), 5000);
    m_view->setFocus(Qt::OtherFocusReason);
    return n;
}

void WriteWindow::addReviewContextMenu(QMenu &menu)
{
    if (!m_doc || m_view->inHeaderFooter() || m_view->inNote()) return;
    menu.addSeparator();
    const int at = m_view->contextPosition();
    if (write::revisionAt(m_doc->story(), at)) {
        menu.addAction(act("review.accept"));
        menu.addAction(act("review.reject"));
    }
    if (write::commentAt(*m_doc, at)) {
        menu.addAction(act("review.replyComment"));
        menu.addAction(act("review.resolveComment"));
        menu.addAction(act("review.deleteComment"));
    } else {
        menu.addAction(act("review.newComment"));
    }
    menu.addAction(act("review.trackChanges"));
}

} // namespace jp
