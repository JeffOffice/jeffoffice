// The style area of the pages (Draft view): which paragraphs begin on the pages on show and where, and the strip that names their styles.

#include "app/pageview.h"

#include "core/styleinspect.h"

#include <QScrollBar>
#include <QSet>
#include <QTextBlock>
#include <algorithm>
#include <cmath>

namespace jp::write {

namespace {
constexpr int kMostLooked = 5000;   // paragraphs looked at in one go (a page of one-character paragraphs holds far fewer)
}

void PageView::setStyleAreaWidth(int width)
{
    m_styleWidth = std::max(0, width);
    if (!m_styleArea && m_styleWidth == 0) return;
    applyStyleArea();
}

void PageView::setStyleAreaSuppressed(bool on)
{
    if (m_styleSuppressed == on) return;
    m_styleSuppressed = on;
    if (m_styleArea) applyStyleArea();
}

// The strip is there while there is a width, in Draft view, and Read Mode is not on; it is made the first time it is wanted, and costs nothing while it is hidden.
void PageView::applyStyleArea()
{
    const bool wanted = m_styleWidth > 0 && m_draft && !m_web && !m_styleSuppressed && m_doc;
    if (wanted && !m_styleArea) {
        m_styleArea = new StyleArea(this, [this] { return styleRows(); }, [this](int w) { isRightToLeft() ? setViewportMargins(0, 0, w, 0) : setViewportMargins(w, 0, 0, 0); });
        connect(m_styleArea, &StyleArea::rowClicked, this, &PageView::selectParagraphAt);
        connect(m_styleArea, &StyleArea::rowDoubleClicked, this, [this](int position) {
            selectParagraphAt(position);
            Q_EMIT styleNameActivated(position);
        });
        connect(m_styleArea, &StyleArea::widthDragged, this, [this](int width) {
            m_styleWidth = width;
            Q_EMIT styleAreaWidthDragged(width);
        });
        connect(this, &PageView::contentChanged, m_styleArea, &StyleArea::scheduleRefresh);
        connect(this, &PageView::zoomChanged, m_styleArea, &StyleArea::scheduleRefresh);
        connect(this, &PageView::pagesChanged, m_styleArea, &StyleArea::scheduleRefresh);
    }
    if (!m_styleArea) return;
    m_styleArea->setAreaWidth(wanted ? m_styleWidth : 0);
    // (The pages fit the room the strip leaves.)
    if (m_mode != ZoomMode::Percent) applyZoomMode();
    relayout();
    viewport()->update();
}

QVector<StyleRow> PageView::styleRows()
{
    QVector<StyleRow> out;
    if (!m_doc || m_hf.active || m_note.active || m_rects.isEmpty()) return out;
    flow::Paginator &pg = m_doc->pager();
    QTextDocument *story = m_doc->story();
    const double sy = verticalScrollBar()->value(), viewH = viewport()->height();
    const double s = scale();
    const int cols = m_sideToSide ? std::max(1, m_knownPages) : std::max(1, m_across);
    int firstRow = int(std::upper_bound(m_rowTops.begin(), m_rowTops.end(), sy) - m_rowTops.begin()) - 1;
    firstRow = std::max(0, firstRow);
    if (firstRow > 0) --firstRow;   // (a row above may be taller than its top says)
    QHash<QString, QString> names;
    QSet<int> tops;
    int looked = 0;
    for (int i = firstRow * cols; i < m_rects.size() && looked < kMostLooked; ++i) {
        if (m_rects[i].top() > sy + viewH) break;
        if (m_rects[i].bottom() < sy) continue;
        // The paragraphs that begin on the page: from the one its first line is in (not that one when it began on the page before) to the one the next page's first line is in
        // (that one too when the next page begins in the middle of it).
        const flow::Paginator::PageStart start = pg.page(i).start;
        int endAt = story->characterCount();
        bool endMid = false;
        if (i + 1 < m_knownPages) {
            const flow::Paginator::PageStart next = pg.page(i + 1).start;
            endAt = next.pos;
            endMid = next.skip > 0;
        }
        QTextBlock b = story->findBlock(std::clamp(start.pos, 0, std::max(0, story->characterCount() - 1)));
        if (b.isValid() && (start.skip > 0 || b.position() < start.pos)) b = b.next();
        for (; b.isValid() && looked < kMostLooked && (b.position() < endAt || (b.position() == endAt && endMid)); b = b.next()) {
            ++looked;
            ++m_styleLooked;
            int page = 0;
            QLineF line;
            if (!caretLine(b.position(), &page, &line) || page < 0 || page >= m_rects.size()) continue;
            const double y = m_rects[page].top() + line.y1() * s - sy, h = (line.y2() - line.y1()) * s;
            if (y + h < 0 || y > viewH) continue;
            // (Paragraphs side by side, in the cells of a table's row, have their first lines at the one place: the name of the first is shown.)
            const int key = int(std::lround(y));
            bool taken = false;
            for (int d = -2; d <= 2 && !taken; ++d) taken = tops.contains(key + d);
            if (taken) continue;
            tops.insert(key);
            StyleRow row;
            row.top = int(std::floor(y));
            row.height = std::max(1, int(h));
            row.position = b.position();
            row.name = paragraphStyleNameOf(*m_doc, b, &names);
            out << row;
        }
    }
    return out;
}

void PageView::selectParagraphAt(int position)
{
    if (!m_doc) return;
    const QTextBlock b = m_doc->story()->findBlock(position);
    if (!b.isValid()) return;
    QTextCursor c(m_doc->story());
    c.setPosition(b.position());
    c.setPosition(b.position() + std::max(0, b.length() - 1), QTextCursor::KeepAnchor);
    setCursor(c);
    setFocus(Qt::MouseFocusReason);
}

} // namespace jp::write
