// Merge Shapes: the shapes selected together are made one outline (or the pieces they cut one another into), as the page draws them.
#include "app/pageview.h"
#include "app/shapetools.h"
#include "core/items.h"

#include <QCoreApplication>
#include <QPointer>

namespace jp::write {

void PageView::cancelMerge()
{
    if (m_mergeCancel) *m_mergeCancel = true;
}

bool PageView::canMergeSelectedShapes()
{
    if (!m_doc || editsBlocked()) return false;
    const QVector<int> positions = selectedObjects();
    if (positions.size() < 2 || positions.size() > pathops::kMostShapes) return false;
    // (The sizes of the drawings are read first: a selection that is too big is off at once. The answer is kept for the same selection of the same document.)
    const int revision = m_doc->story() ? m_doc->story()->revision() : -1;
    if (m_mergeCheck.epoch == m_docEpoch && m_mergeCheck.revision == revision && m_mergeCheck.positions == positions) return m_mergeCheck.ok;
    m_mergeCheck.epoch = m_docEpoch;
    m_mergeCheck.revision = revision;
    m_mergeCheck.positions = positions;
    m_mergeCheck.ok = false;
    if (!mergeSelectionWithinLimits(*m_doc, positions)) return false;
    int page = -1;
    for (const int p : positions) {
        std::shared_ptr<Item> item;
        int at = 0;
        QPainterPath outline;
        if (!canMergeShape(*m_doc, p) || !floatingPictureItem(p, &item, &at) || (page >= 0 && at != page) || !shapeOutlineOnPage(*item, &outline)) return false;
        page = at;
    }
    m_mergeCheck.ok = true;
    return true;
}

int PageView::mergeWordsLost()
{
    if (!m_doc) return 0;
    const QVector<int> positions = selectedObjects();
    int n = 0;
    for (int i = 1; i < positions.size(); ++i) n += shapeHoldsWords(*m_doc, positions[i]) ? 1 : 0;
    return n;
}

bool PageView::mergeSelectedShapes(pathops::Op op, QString *why, int *made, int *wordsLost)
{
    if (made) *made = 0;
    if (wordsLost) *wordsLost = 0;
    auto refuse = [&](const QString &reason) {
        if (why) *why = reason;
        return false;
    };
    if (!m_doc || editsBlocked()) return refuse(QCoreApplication::translate("jp::Editor", "An object that is locked stays as it is."));
    const QVector<int> positions = selectedObjects();
    if (positions.size() < 2) return refuse(QCoreApplication::translate("jp::Editor", "Select two or more shapes first."));
    if (positions.size() > pathops::kMostShapes) return refuse(QCoreApplication::translate("jp::Editor", "Select no more than 64 shapes."));
    if (!mergeSelectionWithinLimits(*m_doc, positions)) return refuse(QCoreApplication::translate("jp::Editor", "The shapes are too complex to merge."));
    QVector<QPainterPath> outlines;
    QTransform firstTurn;
    int page = -1;
    for (const int p : positions) {
        std::shared_ptr<Item> item;
        int at = 0;
        QPainterPath outline;
        if (!canMergeShape(*m_doc, p) || !floatingPictureItem(p, &item, &at) || !shapeOutlineOnPage(*item, &outline)) return refuse(QCoreApplication::translate("jp::Editor", "Only shapes can be changed this way. Artwork and other objects stay as they are."));
        if (page >= 0 && at != page) return refuse(QCoreApplication::translate("jp::Editor", "Select shapes that are on the same page."));
        if (page < 0) firstTurn = item->transform();
        page = at;
        outlines << outline;
    }
    int refused = 0;
    // (The merge waits in a loop of its own: the window can close or show another document meanwhile, and then the result is dropped.)
    const QPointer<PageView> self(this);
    const auto hook = mergeStepHook;
    const int epoch = m_docEpoch, revision = m_doc->story()->revision();
    const QVector<QPainterPath> result = mergeBounded(
        op, outlines, &refused,
        [self](const QString &text, std::atomic<bool> *cancel) {
            if (!self) return;
            self->m_mergeCancel = cancel;
            Q_EMIT self->mergeBusy(text, cancel);
        },
        [self] {
            if (!self) return;
            self->m_mergeCancel = nullptr;
            Q_EMIT self->mergeBusyEnded();
        },
        hook);
    if (!self || epoch != m_docEpoch || !m_doc || !m_doc->story() || revision != m_doc->story()->revision()) return refuse(QCoreApplication::translate("jp::Editor", "Cancelled. The shapes are as they were."));
    if (result.isEmpty()) return refuse(mergeRefusal(refused));
    const int lost = mergeWordsLost();
    m_doc->pictureNotice.clear();
    QVector<int> where;
    if (!mergeShapeObjects(*m_doc, positions, result, firstTurn, &where)) return refuse(m_doc->pictureNotice.isEmpty() ? QCoreApplication::translate("jp::Editor", "The shapes are too complex to merge.") : m_doc->pictureNotice);
    selectObjects(where);
    Q_EMIT contentChanged();
    viewport()->update();
    Q_EMIT caretChanged();
    if (made) *made = int(where.size());
    if (wordsLost) *wordsLost = lost;
    return true;
}

} // namespace jp::write
