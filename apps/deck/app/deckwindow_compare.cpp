// JeffDeck's Review > Compare: another presentation is compared with this one (apps/deck/core/deckcompare.h), the changes it has are listed in the
// Reviewing Pane and marked on the slides, and each is accepted or rejected as one undo step. What is not decided stays in the file (io/pptxreview.h).

#include "app/deckcanvas.h"
#include "app/deckwindow.h"
#include "app/commentspane.h"
#include "app/editor.h"
#include "app/reviewpane_deck.h"
#include "app/settings.h"
#include "core/comments.h"
#include "core/deckcompare.h"
#include "core/reuse.h"
#include "io/pptxreview.h"
#include "io/presentationfile.h"
#include "office/package.h"
#include "app/outlineview.h"
#include "app/notespane.h"

#include <QAction>
#include <QDockWidget>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QHash>
#include <QSet>
#include <QSignalBlocker>
#include <QStatusBar>
#include <QTemporaryFile>

namespace jp {

namespace {
const QString kCompare = QStringLiteral("compare");
}

void DeckWindow::createReviewActions()
{
    mk("review.compare", tr("Compare…"), "git-compare-arrows", QKeySequence(), [this] {
        const QString path = compareFileChooser ? compareFileChooser()
                                                : QFileDialog::getOpenFileName(this, tr("Compare"), QString(),
                                                                               tr("Presentations (*.pptx *.pptm *.potx *.potm *.ppsx *.ppsm *.ppt *.pps *.pot *.odp *.otp)") + QStringLiteral(";;") + tr("All Files (*)"));
        if (path.isEmpty()) return;
        QString why;
        if (!compareWith(path, &why)) notice(tr("Compare"), why.isEmpty() ? tr("The presentation can't be compared.") : why, true);
    });
    mk("review.accept", tr("Accept"), "check", QKeySequence(), [this] {
        if (m_reviewPane && m_reviewPane->currentChange() >= 0) acceptChange(m_reviewPane->currentChange());
    });
    mk("review.reject", tr("Reject"), "x", QKeySequence(), [this] {
        if (m_reviewPane && m_reviewPane->currentChange() >= 0) rejectChange(m_reviewPane->currentChange());
    });
    mk("review.acceptAll", tr("Accept All"), "check-check", QKeySequence(), [this] { acceptAllChanges(); });
    mk("review.rejectAll", tr("Reject All"), "circle-x", QKeySequence(), [this] { rejectAllChanges(); });
    mk("review.prev", tr("Previous"), "arrow-left", QKeySequence(), [this] { stepChange(false); });
    mk("review.next", tr("Next"), "arrow-right", QKeySequence(), [this] { stepChange(true); });
    mk("review.pane", tr("Reviewing Pane"), "list-checks", QKeySequence(), [this] { showReviewPane(act("review.pane")->isChecked()); }, true);

    m_marksTimer.setSingleShot(true);
    m_marksTimer.setInterval(0);
    connect(&m_marksTimer, &QTimer::timeout, this, [this] { updateReviewMarks(); });
    // What the commands and the pane show follows the presentation, whatever changed it: an undo brings the states back with the slides.
    connect(m_ed, &Editor::changed, this, [this] { syncComparison(false); });
    connect(m_ed, &Editor::documentReplaced, this, [this] { syncComparison(true); });
    connect(m_ed, &Editor::pageChanged, this, [this] {
        if (m_cmp) m_marksTimer.start();
    });
    updateReviewActions();
}

void DeckWindow::updateReviewActions()
{
    const bool have = m_cmp && m_reviewPane;
    const int cur = have ? m_reviewPane->currentChange() : -1;
    const review::ChangeList *list = m_cmp ? &m_cmp->changes : nullptr;
    const bool one = cur >= 0 && list && cur < list->size();
    act("review.accept")->setEnabled(one && list->changes[cur].state != review::State::Accepted);
    act("review.reject")->setEnabled(one && list->changes[cur].state != review::State::Rejected);
    const bool pending = list && list->pending() > 0;
    act("review.acceptAll")->setEnabled(pending);
    act("review.rejectAll")->setEnabled(pending);
    act("review.prev")->setEnabled(pending);
    act("review.next")->setEnabled(pending);
}

void DeckWindow::showReviewPane(bool on)
{
    if (on && !m_reviewPane) {
        m_reviewDock = new QDockWidget(tr("Reviewing Pane"), this);
        m_reviewDock->setObjectName(QStringLiteral("reviewDock"));
        m_reviewDock->setFeatures(QDockWidget::DockWidgetClosable);
        m_reviewPane = new DeckReviewPane(m_reviewDock);
        m_reviewDock->setWidget(m_reviewPane);
        addDockWidget(Qt::RightDockWidgetArea, m_reviewDock);
        resizeDocks({m_reviewDock}, {340}, Qt::Horizontal);
        // (the action is the receiver: when the window goes, the action does, and the dock's hiding on the way out finds nothing to call)
        QAction *paneAction = act("review.pane");
        connect(m_reviewDock, &QDockWidget::visibilityChanged, paneAction, [paneAction](bool visible) { paneAction->setChecked(visible); });
        connect(m_reviewPane, &DeckReviewPane::acceptRequested, this, [this](int i) { acceptChange(i); });
        connect(m_reviewPane, &DeckReviewPane::rejectRequested, this, [this](int i) { rejectChange(i); });
        connect(m_reviewPane, &DeckReviewPane::acceptAllRequested, this, [this] { acceptAllChanges(); });
        connect(m_reviewPane, &DeckReviewPane::rejectAllRequested, this, [this] { rejectAllChanges(); });
        connect(m_reviewPane, &DeckReviewPane::changeChosen, this, [this](int i) {
            followChange(i);
            updateReviewActions();
        });
        connect(m_reviewPane, &DeckReviewPane::closeRequested, this, [this] { showReviewPane(false); });
        connect(m_reviewPane, &DeckReviewPane::commentChosen, this, [this](const QString &id, int slide) {
            if (slide >= 0 && slide < m_ed->doc()->pages.size()) m_ed->setCurrentPage(slide);
            showComments(true);
            m_comments->setCurrentComment(id);
        });
        connect(m_reviewPane, &DeckReviewPane::pageChanged, this, [this](int page) {
            if (page != 1 || !m_ed->doc()) return;
            QVector<DeckReviewPane::CommentRow> rows;
            const Document &d = *m_ed->doc();
            for (int s = 0; s < d.pages.size() && rows.size() < 2000; ++s)
                for (const comments::Comment &c : comments::commentsOf(*d.pages[s])) {
                    DeckReviewPane::CommentRow r;
                    r.slide = s;
                    r.id = c.id;
                    r.author = c.author;
                    r.text = c.text;
                    rows << r;
                }
            m_reviewPane->setComments(rows);
        });
    }
    if (m_reviewDock) m_reviewDock->setVisible(on);
    act("review.pane")->setChecked(on);
    if (on && m_reviewPane && m_reviewPane->sourceList() != (m_cmp ? &m_cmp->changes : nullptr)) {   // (a pane already showing this list is left as it is)
        DeckReviewPane::Source src;
        if (m_cmp) {
            src.list = &m_cmp->changes;
            auto slideOf = std::make_shared<QHash<QString, int>>();   // the slides' places by id, made once for a rebuild of the filters (it was a scan of the slides for each label)
            src.beginLabels = [slideOf] { slideOf->clear(); };
            src.slideLabel = [this, slideOf](const QString &container) {
                const Document *d = m_ed->doc();
                if (d && slideOf->isEmpty()) {   // (a place of this presentation's slide is 0 or more, one of the other presentation's is -1 less its place)
                    for (int i = 0; i < d->pages.size(); ++i)
                        if (!slideOf->contains(d->pages[i]->id)) slideOf->insert(d->pages[i]->id, i);
                    if (m_cmp && m_cmp->theirs)
                        for (int i = 0; i < m_cmp->theirs->pages.size(); ++i)
                            if (!slideOf->contains(m_cmp->theirs->pages[i]->id)) slideOf->insert(m_cmp->theirs->pages[i]->id, -1 - i);
                }
                const auto at = slideOf->constFind(container);
                if (d && at != slideOf->constEnd()) {
                    if (*at >= 0 && *at < d->pages.size()) return tr("%1. %2").arg(*at + 1).arg(deck::slideTitle(*d, *at));
                    const int t = -1 - *at;
                    if (m_cmp && m_cmp->theirs && t >= 0 && t < m_cmp->theirs->pages.size()) return tr("New: %1").arg(deck::slideTitle(*m_cmp->theirs, t));
                }
                return container;
            };
            src.texts = [this](int change, QString *before, QString *after) { return m_cmp && deck::textChangeOf(*m_cmp, change, before, after); };
        }
        m_reviewPane->setSource(src);
    }
    updateReviewActions();
}

void DeckWindow::dropComparison()
{
    m_cmp.reset();
    m_cmpKey.clear();
    if (m_reviewPane) m_reviewPane->setSource({});
    m_canvas->setReviewMarks({}, 0);
    updateReviewActions();
}

// The states of the changes into the presentation's appData (saved and undone with it), and the keys of the rejected ones for the file.
void DeckWindow::recordComparison()
{
    Document *d = m_ed->doc();
    if (!d || !m_cmp) return;
    QJsonObject states;
    QJsonArray rejected;
    for (const review::Change &c : std::as_const(m_cmp->changes.changes)) {
        if (c.state == review::State::Pending) continue;
        states[c.id] = review::stateKey(c.state);
        if (c.state == review::State::Rejected && rejected.size() < deck::kMaxReviewKeys) rejected.append(deck::changeKey(c).left(500));
    }
    QJsonObject o = d->appData.value(kCompare).toObject();
    o[QStringLiteral("theirs")] = m_cmpKey;
    o[QStringLiteral("states")] = states;
    o[QStringLiteral("rejected")] = rejected;
    o[QStringLiteral("pending")] = m_cmp->changes.pending();   // (the file keeps the comparison only while something is not decided)
    d->appData[kCompare] = o;
}

void DeckWindow::syncComparison(bool fresh)
{
    if (m_syncingReview) return;
    Document *d = m_ed->doc();
    QJsonObject c = d ? d->appData.value(kCompare).toObject() : QJsonObject();
    if (c.isEmpty()) {
        if (m_cmp) dropComparison();
        return;
    }
    // A comparison that cannot be made is let go of once, with one quiet word: nothing is tried again with every edit.
    auto giveUp = [&] {
        d->appData.remove(kCompare);
        if (m_cmp) dropComparison();
        statusBar()->showMessage(tr("The changes kept in this file can't be read, so they were left out."), 8000);
    };
    if (c.contains(QStringLiteral("raw"))) {   // (a part read with the file: made into the comparison now that the window needs it)
        if (!deck::decodeReviewPart(*d)) {
            giveUp();
            return;
        }
        c = d->appData.value(kCompare).toObject();
    }
    const QString key = c.value(QStringLiteral("theirs")).toString();
    if (fresh || !m_cmp || key != m_cmpKey) {
        // The other presentation comes from the store (it was put there when the file was read, or when the comparison was made), and the changes are found
        // again against it: what was accepted is no longer a difference, and what was rejected is told by its key.
        const QByteArray bytes = deck::storedReviewTheirs(key);
        std::unique_ptr<Document> theirs;
        if (!bytes.isEmpty()) {
            ++deck::reviewCounters().imports;
            theirs = deck::importAnyPresentationBytes(bytes);
        }
        if (!theirs) {
            giveUp();
            return;
        }
        deck::retainReviewTheirs(key, this);
        auto cmp = std::make_shared<deck::DeckComparison>(deck::compareDecks(*d, *theirs));
        QSet<QString> rejected;
        for (const QJsonValue &v : c.value(QStringLiteral("rejected")).toArray()) rejected.insert(v.toString());
        for (review::Change &ch : cmp->changes.changes)
            if (rejected.contains(deck::changeKey(ch).left(500))) ch.state = review::State::Rejected;
        if (m_reviewPane) m_reviewPane->setSource({});   // (it holds the old list until it is given the new one)
        m_cmp = cmp;
        m_cmpKey = key;
        recordComparison();   // (the ids are new ones: written straight, not a step of the history)
        showReviewPane(true);
        if (m_reviewPane) {
            QSignalBlocker stay(fresh ? m_reviewPane : nullptr);   // (a file that opens keeps the slide it opens on: the first change is chosen, not gone to)
            m_reviewPane->step(true);
        }
        m_marksTimer.start();
        updateReviewActions();
        return;
    }
    // The same comparison: the states follow appData (an undo or redo brought them back with the slides).
    const QJsonObject states = c.value(QStringLiteral("states")).toObject();
    bool differs = false;
    for (review::Change &ch : m_cmp->changes.changes) {
        const review::State want = review::stateFromKey(states.value(ch.id).toString());
        if (ch.state != want) {
            ch.state = want;
            differs = true;
        }
    }
    if (differs) {
        if (m_reviewPane) m_reviewPane->refresh();
        updateReviewActions();
    }
    if (differs) m_marksTimer.start();   // (an edit that decides nothing leaves the marks as they are; a change of slide makes them again)
}

bool DeckWindow::compareWith(const QString &path, QString *error)
{
    auto fail = [&](const QString &why) {
        if (error) *error = why;
        return false;
    };
    Document *d = m_ed->doc();
    if (!d) return fail(tr("There is no presentation to compare."));
    if (m_ed->readOnly()) return fail(tr("This presentation is open for reading only, so changes can't be taken into it."));
    m_notes->commit();
    if (m_outline) m_outline->commit();
    m_ed->endTextEdit();
    QByteArray bytes;
    QString why;
    if (!office::readWholeFile(path, &bytes, &why)) return fail(why.isEmpty() ? tr("The file can't be read.") : why);
    QString err;
    std::unique_ptr<Document> theirs = deck::importAnyPresentationBytes(bytes, &err);
    if (!theirs) return fail(err.isEmpty() ? tr("The file can't be read as a presentation.") : err);
    auto cmp = std::make_shared<deck::DeckComparison>(deck::compareDecks(*d, *theirs));
    const QString key = deck::storeReviewTheirs(bytes);
    deck::retainReviewTheirs(key, this);
    // (the copy of the comparison before this one is let go of: a window keeps the one it shows, not every one it ever made)
    if (!m_cmpKey.isEmpty() && m_cmpKey != key) deck::releaseReviewTheirs(m_cmpKey, this);
    // One step: undoing it takes the comparison away, and redoing it brings it back.
    if (m_reviewPane) m_reviewPane->setSource({});   // (it holds the old list until it is given the new one)
    const std::shared_ptr<deck::DeckComparison> older = m_cmp;
    m_syncingReview = true;
    m_cmp = cmp;
    m_cmpKey = key;
    m_ed->change(tr("Compare Presentations"), [&] {
        d->appData.remove(kCompare);
        recordComparison();
    });
    m_syncingReview = false;
    if (key.isEmpty()) notice(tr("Compare"), tr("The other presentation is too large to be kept in this file, so the changes not decided will not be kept when you save."));
    showReviewPane(true);
    if (m_reviewPane) m_reviewPane->step(true);
    m_marksTimer.start();
    updateReviewActions();
    const int n = m_cmp->changes.size();
    statusBar()->showMessage(n == 0 ? tr("The two presentations are the same.") : tr("Changes found: %1").arg(n), 6000);
    return true;
}

void DeckWindow::followChange(int index)
{
    if (!m_cmp || index < 0 || index >= m_cmp->changes.size()) return;
    const deck::ChangeMark mark = deck::markOf(*m_ed->doc(), *m_cmp, index);
    if (mark.slide >= 0 && mark.slide != m_ed->currentPage()) m_ed->setCurrentPage(mark.slide);
    m_marksTimer.start();
}

void DeckWindow::updateReviewMarks()
{
    Document *d = m_ed->doc();
    ++deck::reviewCounters().markRebuilds;
    if (!m_cmp || !d) {
        m_canvas->setReviewMarks({}, 0);
        return;
    }
    const int page = m_ed->currentPage();
    const int current = m_reviewPane ? m_reviewPane->currentChange() : -1;
    QVector<DeckCanvas::ReviewMark> marks;
    int level = 0;
    for (int i = 0; i < m_cmp->changes.size(); ++i) {
        const review::Change &ch = m_cmp->changes.changes[i];
        if (ch.state != review::State::Pending) continue;
        const deck::ChangeMark mk = deck::markOf(*d, *m_cmp, i);
        if (mk.slide != page) continue;
        if (!mk.hasRect) {
            ++level;
            continue;
        }
        DeckCanvas::ReviewMark m;
        m.rect = mk.rect;
        m.current = i == current;
        m.added = ch.kind == review::Kind::Added;
        marks << m;
    }
    m_canvas->setReviewMarks(marks, level);
}

bool DeckWindow::acceptChange(int index)
{
    if (!m_cmp || index < 0 || index >= m_cmp->changes.size() || m_ed->readOnly()) return false;
    m_notes->commit();
    if (m_outline) m_outline->commit();
    m_ed->endTextEdit();
    bool ok = false;
    m_syncingReview = true;
    m_ed->change(tr("Accept Change"), [&] {
        ok = deck::applyChange(*m_ed->doc(), *m_cmp, index);
        if (ok) recordComparison();
    });
    m_syncingReview = false;
    if (!ok) {
        statusBar()->showMessage(tr("This change can't be taken into the presentation."), 6000);
        return false;
    }
    if (m_reviewPane) {
        m_reviewPane->refreshChange(index);
        followChange(m_reviewPane->currentChange());
    }
    updateReviewActions();
    m_marksTimer.start();
    return true;
}

bool DeckWindow::rejectChange(int index)
{
    if (!m_cmp || index < 0 || index >= m_cmp->changes.size() || m_ed->readOnly()) return false;
    m_notes->commit();
    if (m_outline) m_outline->commit();
    m_ed->endTextEdit();
    bool ok = false;
    m_syncingReview = true;
    m_ed->change(tr("Reject Change"), [&] {
        ok = deck::rejectChange(*m_ed->doc(), *m_cmp, index);
        if (ok) recordComparison();
    });
    m_syncingReview = false;
    if (!ok) {
        statusBar()->showMessage(tr("This change can't be taken back."), 6000);
        return false;
    }
    if (m_reviewPane) {
        m_reviewPane->refreshChange(index);
        followChange(m_reviewPane->currentChange());
    }
    updateReviewActions();
    m_marksTimer.start();
    return true;
}

void DeckWindow::acceptAllChanges()
{
    if (!m_cmp || m_ed->readOnly()) return;
    m_notes->commit();
    if (m_outline) m_outline->commit();
    m_ed->endTextEdit();
    m_syncingReview = true;
    m_ed->change(tr("Accept All Changes"), [&] {
        deck::acceptAll(*m_ed->doc(), *m_cmp);
        recordComparison();
    });
    m_syncingReview = false;
    if (m_reviewPane) m_reviewPane->refresh();
    updateReviewActions();
    m_marksTimer.start();
}

void DeckWindow::rejectAllChanges()
{
    if (!m_cmp || m_ed->readOnly()) return;
    m_notes->commit();
    if (m_outline) m_outline->commit();
    m_ed->endTextEdit();
    m_syncingReview = true;
    m_ed->change(tr("Reject All Changes"), [&] {
        deck::rejectAll(*m_ed->doc(), *m_cmp);
        recordComparison();
    });
    m_syncingReview = false;
    if (m_reviewPane) m_reviewPane->refresh();
    updateReviewActions();
    m_marksTimer.start();
}

void DeckWindow::stepChange(bool forward)
{
    if (!m_cmp) return;
    showReviewPane(true);
    if (m_reviewPane) m_reviewPane->step(forward);
}

} // namespace jp
