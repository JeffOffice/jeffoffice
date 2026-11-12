// JeffWrite's Review tab: Spelling, Track Changes, Accept and Reject, Display for Review, and comments, with the Reviewing pane.

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
#include <QApplication>
#include <QDockWidget>
#include <QInputDialog>
#include <QMenu>
#include <QStatusBar>
#include <QToolButton>
#include <QTreeWidget>

namespace jp {

void WriteWindow::createReviewActions()
{
    static const bool markupLoaded = (loadRevisionMarkup(), true);   // (the person's choices for the look of changes, read once, by the first window)
    Q_UNUSED(markupLoaded);
    mk("review.spelling", tr("Spelling"), "spell-check", QKeySequence(Qt::Key_F7), [this] { showSpelling(); });
    QAction *asType = mk("review.checkAsType", tr("Check Spelling as You Type"), "spell-check-2", QKeySequence(), [this] {
        const bool on = act("review.checkAsType")->isChecked();
        m_view->setSpellingEnabled(on);
        setSpellAsYouType(on);
    }, true);
    const bool on = spellAsYouType();
    asType->setChecked(on);
    m_view->setSpellingEnabled(on);
    // (Options, or another window, changes the one shared choice)
    connect(&SpellSettingNotifier::get(), &SpellSettingNotifier::changed, this, [this, asType](bool now) {
        asType->setChecked(now);
        m_view->setSpellingEnabled(now);
    });

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
        statusBar()->showMessage(n == 1 ? tr("1 change accepted.") : tr("%1 changes accepted.").arg(n), 5000);
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("review.acceptAllStop", tr("Accept All Changes and Stop Tracking"), "check-check", QKeySequence(), [this] {
        if (!m_doc) return;
        m_view->leaveHeaderFooter();
        m_view->leaveNote();
        const int n = write::acceptAll(*m_doc);
        m_doc->setTrackChanges(false);
        statusBar()->showMessage(n == 1 ? tr("1 change accepted. Track Changes is off.") : tr("%1 changes accepted. Track Changes is off.").arg(n), 5000);
        m_view->setFocus(Qt::OtherFocusReason);
        refreshReviewUi();
    });
    mk("review.rejectAll", tr("Reject All Changes"), "circle-x", QKeySequence(), [this] {
        if (!m_doc) return;
        m_view->leaveHeaderFooter();
        m_view->leaveNote();
        const int n = write::rejectAll(*m_doc);
        statusBar()->showMessage(n == 1 ? tr("1 change rejected.") : tr("%1 changes rejected.").arg(n), 5000);
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
    // (What Show Markup has on is each window's own, and all of it is on when a window begins.)
    auto showAction = [this](const char *id, const QString &text, bool *flag) {
        QAction *a = mk(QString::fromLatin1(id), text, "", QKeySequence(), [this, flag, id] {
            *flag = act(QString::fromLatin1(id))->isChecked();
            applyReviewView();
        }, true);
        *flag = true;
        a->setChecked(true);
        return a;
    };
    showAction("review.show.insdel", tr("Insertions and Deletions"), &m_showInsertionsAndDeletions);
    showAction("review.show.formatting", tr("Formatting"), &m_showFormatting);
    showAction("review.show.comments", tr("Comments"), &m_showComments);
    m_balloonGroup = new QActionGroup(this);
    auto balloonAction = [this](const char *id, const QString &text, int kind) {
        QAction *a = mk(QString::fromLatin1(id), text, "", QKeySequence(), [this, kind] {
            m_balloons = kind;
            applyReviewView();
        }, true);
        m_balloonGroup->addAction(a);
        return a;
    };
    balloonAction("review.balloons.inline", tr("Show All Revisions Inline"), 0);
    balloonAction("review.balloons.balloons", tr("Show Revisions in Balloons"), 1);
    balloonAction("review.balloons.comments", tr("Show Only Comments and Formatting in Balloons"), 2);
    m_balloons = 0;
    act("review.balloons.inline")->setChecked(true);
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
        statusBar()->showMessage(n == 1 ? tr("1 comment deleted.") : tr("%1 comments deleted.").arg(n), 5000);
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

namespace {
// (The names the person's choices are kept under, in the app's settings.)
const char *const kMarkupInts[] = {"insertion", "deletion", "moveFrom", "moveTo", "formatting", "bars"};
const char *const kMarkupColors[] = {"insertionColor", "deletionColor", "moveFromColor", "moveToColor", "formattingColor", "barColor"};
}

void WriteWindow::applyRevisionMarkup(const tp::RevisionMarkup &markup)
{
    const int generation = tp::revisionMarkup().generation + 1;
    tp::revisionMarkup() = markup;
    tp::revisionMarkup().generation = generation;
    tp::RevisionMarkup &m = tp::revisionMarkup();
    const int values[] = {m.insertion, m.deletion, m.moveFrom, m.moveTo, m.formatting, m.bars};
    const QColor colors[] = {m.insertionColor, m.deletionColor, m.moveFromColor, m.moveToColor, m.formattingColor, m.barColor};
    Settings &s = Settings::get();
    for (int i = 0; i < 6; ++i) {
        s.setValue(QStringLiteral("review/markup/") + QLatin1String(kMarkupInts[i]), values[i]);
        s.setValue(QStringLiteral("review/markup/") + QLatin1String(kMarkupColors[i]), colors[i].isValid() ? colors[i].name() : QString());
    }
    s.setValue(QStringLiteral("review/markup/trackMoves"), m.trackMoves);
    s.setValue(QStringLiteral("review/markup/trackFormatting"), m.trackFormatting);
    for (QWidget *w : QApplication::topLevelWidgets())
        if (auto *win = qobject_cast<WriteWindow *>(w)) {
            if (win->m_doc) win->m_doc->refreshRevisionMarkup();
            win->applyReviewView();
        }
}

// (Each choice that was kept replaces the one the program has; one that was never kept leaves it.)
void WriteWindow::loadRevisionMarkup()
{
    const Settings &s = Settings::get();
    tp::RevisionMarkup &m = tp::revisionMarkup();
    int *ints[] = {&m.insertion, &m.deletion, &m.moveFrom, &m.moveTo, &m.formatting, &m.bars};
    QColor *colors[] = {&m.insertionColor, &m.deletionColor, &m.moveFromColor, &m.moveToColor, &m.formattingColor, &m.barColor};
    const int most[] = {tp::RevisionMarkup::InsNone, tp::RevisionMarkup::DelColorOnly, tp::RevisionMarkup::DelColorOnly, tp::RevisionMarkup::InsNone, tp::RevisionMarkup::FmtNone, tp::RevisionMarkup::BarsNone};
    for (int i = 0; i < 6; ++i) {
        const QString ik = QStringLiteral("review/markup/") + QLatin1String(kMarkupInts[i]), ck = QStringLiteral("review/markup/") + QLatin1String(kMarkupColors[i]);
        if (const QVariant v = s.value(ik); v.isValid()) *ints[i] = std::clamp(v.toInt(), 0, most[i]);
        if (const QVariant v = s.value(ck); v.isValid()) *colors[i] = QColor(v.toString());   // (an empty or unreadable one is the author's own)
    }
    if (const QVariant v = s.value(QStringLiteral("review/markup/trackMoves")); v.isValid()) m.trackMoves = v.toBool();
    if (const QVariant v = s.value(QStringLiteral("review/markup/trackFormatting")); v.isValid()) m.trackFormatting = v.toBool();
    ++m.generation;
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
    const tp::RevisionMarkup &look = tp::revisionMarkup();
    switch (m_reviewView) {
    case ReviewView::AllMarkup:
        if (m_showInsertionsAndDeletions) {
            // (Deleted words are left out of the lines where they go in balloons, and where the person chose to have them hidden or marked by a caret; moved-away words stay unless balloons have them.)
            const bool leftOut = m_balloons == 1 || look.deletion == tp::RevisionMarkup::DelHidden || look.deletion == tp::RevisionMarkup::DelCaret;
            flags = tp::kRevisionMark | (!leftOut ? tp::kRevisionShowDeleted : m_balloons != 1 && look.trackMoves ? tp::kRevisionShowMoved : 0);
        }
        if (m_showFormatting && look.trackFormatting && flags) flags |= tp::kRevisionMarkFormat;
        marks.changeBars = m_showInsertionsAndDeletions;
        marks.formatBars = m_showFormatting;
        marks.deletions = m_showInsertionsAndDeletions && m_balloons == 1;
        marks.comments = m_showComments;
        marks.carets = marks.changeBars && m_balloons != 1 && look.deletion == tp::RevisionMarkup::DelCaret;
        break;
    case ReviewView::SimpleMarkup:
        marks.changeBars = m_showInsertionsAndDeletions;
        marks.formatBars = m_showFormatting;
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

// Whether the text has changes and comments, for the commands that have nothing to do without them.
void WriteWindow::updateReviewState()
{
    if (!m_doc) {
        m_hasRevisions = m_hasComments = false;
        m_reviewStateDoc = nullptr;
        return;
    }
    m_hasRevisions = write::revisionCount(m_doc->story()) > 0;
    m_hasComments = !write::comments(*m_doc).isEmpty();
    m_reviewStateRevision = m_doc->story()->revision();
    m_reviewStateDoc = m_doc.get();
}

void WriteWindow::refreshReviewUi()
{
    if (m_updatingReview) return;
    m_updatingReview = true;
    const bool has = m_doc != nullptr;
    if (has && (m_reviewStateDoc != m_doc.get() || m_reviewStateRevision != m_doc->story()->revision())) {
        // (A long document is looked through a moment after the last edit, not at each key.)
        if (m_doc->story()->blockCount() < 2000 || m_reviewStateDoc != m_doc.get()) {
            updateReviewState();
        } else if (!m_reviewStateTimer.isActive()) {
            m_reviewStateTimer.start();
        }
    }
    for (QAction *a : m_actions)
        if (a->objectName().startsWith(QLatin1String("review.")) && a->objectName() != QLatin1String("review.spelling") && a->objectName() != QLatin1String("review.checkAsType")) a->setEnabled(has);
    if (has) {
        act("review.trackChanges")->setChecked(m_doc->trackChanges());
        for (const char *id : {"review.accept", "review.reject", "review.acceptShown", "review.rejectShown", "review.acceptAll", "review.rejectAll", "review.acceptAllStop", "review.nextChange",
                               "review.previousChange"})
            act(QString::fromLatin1(id))->setEnabled(m_hasRevisions);
        for (const char *id : {"review.deleteComment", "review.deleteAllComments", "review.replyComment", "review.resolveComment", "review.nextComment", "review.previousComment"})
            act(QString::fromLatin1(id))->setEnabled(m_hasComments);
    }
    // A document whose tracking is locked on lets neither Track Changes be turned off nor a change be accepted or rejected.
    if (has && m_protectKind == QLatin1String("trackedChanges")) {
        for (const char *id : {"review.trackChanges", "review.accept", "review.reject", "review.acceptShown", "review.rejectShown", "review.acceptAll", "review.rejectAll", "review.acceptAllStop"})
            act(QString::fromLatin1(id))->setEnabled(false);
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
    statusBar()->showMessage(accept ? (n == 1 ? tr("1 change accepted.") : tr("%1 changes accepted.").arg(n)) : (n == 1 ? tr("1 change rejected.") : tr("%1 changes rejected.").arg(n)), 5000);
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
