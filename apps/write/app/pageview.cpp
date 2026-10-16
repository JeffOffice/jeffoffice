#include "app/pageview.h"

#include "app/icons.h"
#include "app/pagepaint.h"
#include "core/editops.h"
#include "render/renderer.h"

#include <QApplication>
#include <QImage>
#include <QPaintEvent>
#include <QPainter>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextDocument>
#include <algorithm>
#include <cmath>

namespace jp::write {

namespace {
constexpr double kMargin = 24;      // around the pages, in pixels
constexpr double kGap = 14;         // between pages
constexpr int kKeptPages = 120;     // pages kept made (a window at 10% shows about 50)
constexpr double kPointToPixel = 96.0 / 72.0;
}

struct PageView::Rendered {
    std::unique_ptr<flow::MaterializedPage> page;   // null: the page has no lines
    LayoutCache layouts;
    QVector<TextItem *> boxes;
};

PageView::PageView(QWidget *parent) : QAbstractScrollArea(parent)
{
    setObjectName(QStringLiteral("pageView"));
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_InputMethodEnabled);
    viewport()->setCursor(Qt::IBeamCursor);
    viewport()->setMouseTracking(false);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    verticalScrollBar()->setSingleStep(48);
    horizontalScrollBar()->setSingleStep(48);
    setFrameShape(QFrame::NoFrame);
    m_pagination.setSingleShot(true);
    m_pagination.setInterval(0);
    connect(&m_pagination, &QTimer::timeout, this, &PageView::paginationStep);
    m_blink.setInterval(QApplication::cursorFlashTime() > 0 ? QApplication::cursorFlashTime() / 2 : 530);
    connect(&m_blink, &QTimer::timeout, this, &PageView::blink);
    m_autoscroll.setInterval(40);
    connect(&m_autoscroll, &QTimer::timeout, this, [this] {
        if (!m_dragging) {
            m_autoscroll.stop();
            return;
        }
        const QPoint pos = viewport()->mapFromGlobal(QCursor::pos());
        int dy = 0;
        if (pos.y() < 0) dy = pos.y() / 3 - 4;
        else if (pos.y() > viewport()->height()) dy = (pos.y() - viewport()->height()) / 3 + 4;
        if (dy == 0) return;
        verticalScrollBar()->setValue(verticalScrollBar()->value() + dy);
        const Hit hit = hitAt(QPoint(std::clamp(pos.x(), 0, viewport()->width()), dy < 0 ? 0 : viewport()->height() - 1));
        const int at = positionAt(hit);
        if (at >= 0) {
            m_cursor.setPosition(at, QTextCursor::KeepAnchor);
            caretMoved();
        }
    });
}

PageView::~PageView() = default;

void PageView::setDocument(WriteDoc *doc)
{
    if (m_connected) disconnect(m_connected, nullptr, this, nullptr);
    m_connected = nullptr;
    if (m_doc) disconnect(m_doc, nullptr, this, nullptr);
    m_rendered.clear();
    m_recent.clear();
    m_doc = doc;
    m_rects.clear();
    m_rowTops.clear();
    m_knownPages = 0;
    m_complete = false;
    m_wantedX = -1;
    if (!doc) {
        m_cursor = QTextCursor();
        setScrollRanges();
        viewport()->update();
        return;
    }
    m_cursor = QTextCursor(doc->story());
    m_connected = doc->story();
    connect(m_connected, &QTextDocument::contentsChange, this, &PageView::documentChanged);
    connect(doc, &WriteDoc::sectionsChanged, this, [this] {
        dropRendered(0);
        if (m_mode != ZoomMode::Percent) applyZoomMode();
        relayout();
        viewport()->update();
    });
    doc->pager().paginate(0);
    if (m_mode != ZoomMode::Percent) applyZoomMode();
    relayout();
    verticalScrollBar()->setValue(0);
    startPagination();
    viewport()->update();
    Q_EMIT pagesChanged();
    Q_EMIT caretChanged();
}

void PageView::setCursor(const QTextCursor &c)
{
    m_cursor = c;
    caretMoved();
}

// ---- pages

int PageView::pageCount() const { return m_knownPages; }
bool PageView::pagesComplete() const { return m_complete; }

int PageView::currentPage() const
{
    if (!m_doc) return 0;
    return std::clamp(m_doc->pager().pageOfPosition(m_cursor.position()), 0, std::max(0, m_knownPages - 1));
}

int PageView::topPage() const
{
    const double y = verticalScrollBar()->value() + viewport()->height() / 3.0;
    int best = 0;
    for (int i = 0; i < m_rects.size(); ++i) {
        if (m_rects[i].top() <= y) best = i;
        else break;
    }
    return best;
}

void PageView::goToPage(int index)
{
    if (!m_doc || m_rects.isEmpty()) return;
    index = std::clamp(index, 0, int(m_rects.size()) - 1);
    verticalScrollBar()->setValue(int(m_rects[index].top() - kMargin));
}

QRect PageView::pageRect(int index) const
{
    if (index < 0 || index >= m_rects.size()) return QRect();
    return m_rects[index].translated(-horizontalScrollBar()->value(), -verticalScrollBar()->value()).toAlignedRect();
}

std::shared_ptr<PageView::Rendered> PageView::rendered(int index)
{
    if (!m_doc || index < 0) return nullptr;
    const auto found = m_rendered.constFind(index);
    if (found != m_rendered.constEnd()) {
        m_recent.removeOne(index);
        m_recent.prepend(index);
        return found.value();
    }
    flow::Paginator &pg = m_doc->pager();
    pg.paginate(index);
    if (index >= pg.pagesKnown()) return nullptr;
    auto r = std::make_shared<Rendered>();
    r->page = pg.materialize(index);
    if (r->page)
        for (const ItemPtr &item : r->page->doc->pages[0]->items)
            if (item->type() == ItemType::Text) r->boxes.push_back(static_cast<TextItem *>(item.get()));
    m_rendered.insert(index, r);
    m_recent.prepend(index);
    while (m_recent.size() > kKeptPages) m_rendered.remove(m_recent.takeLast());
    return r;
}

void PageView::dropRendered(int fromPage)
{
    for (auto it = m_rendered.begin(); it != m_rendered.end();) {
        if (it.key() >= fromPage) {
            m_recent.removeOne(it.key());
            it = m_rendered.erase(it);
        } else {
            ++it;
        }
    }
}

void PageView::relayout()
{
    if (m_relayouting) return;
    m_relayouting = true;
    if (!m_doc) {
        m_rects.clear();
        m_rowTops.clear();
        m_content = QSizeF();
        m_relayouting = false;
        return;
    }
    flow::Paginator &pg = m_doc->pager();
    const int known = pg.pagesKnown();
    m_complete = pg.complete();
    const double s = scale();
    const int cols = std::max(1, m_across);
    // The sizes of the pages: while edits wait to be laid out, those of pages not made yet are as they were.
    QVector<QSizeF> sizes(known);
    const QVector<QRectF> before = m_rects;
    const bool settled = pg.settled();
    for (int i = 0; i < known; ++i) {
        if (settled || i < 1) sizes[i] = pg.page(i).size * s;
        else if (i < before.size() && !m_rects[i].isEmpty()) sizes[i] = before[i].size();
        else sizes[i] = sizes[i - 1];
    }
    const int rows = (known + cols - 1) / cols;
    QVector<double> rowW(rows, 0.0), rowH(rows, 0.0);
    for (int i = 0; i < known; ++i) {
        const int r = i / cols;
        rowW[r] += sizes[i].width() + (i % cols ? kGap : 0);
        rowH[r] = std::max(rowH[r], sizes[i].height());
    }
    double widest = 0;
    for (double w : std::as_const(rowW)) widest = std::max(widest, w);
    const double area = std::max(widest, viewport()->width() - 2 * kMargin);
    m_rects.resize(known);
    m_rowTops.resize(rows);
    double y = kMargin;
    for (int r = 0; r < rows; ++r) {
        m_rowTops[r] = y;
        double x = kMargin + (area - rowW[r]) / 2;
        for (int c = 0; c < cols; ++c) {
            const int i = r * cols + c;
            if (i >= known) break;
            m_rects[i] = QRectF(QPointF(x, y), sizes[i]);
            x += sizes[i].width() + kGap;
        }
        y += rowH[r] + kGap;
    }
    m_content = QSizeF(area + 2 * kMargin, rows ? y - kGap + kMargin : 2 * kMargin);
    const bool changed = known != m_knownPages;
    m_knownPages = known;
    setScrollRanges();
    m_relayouting = false;
    if (changed) Q_EMIT pagesChanged();
}

void PageView::setScrollRanges()
{
    const int vw = viewport()->width(), vh = viewport()->height();
    verticalScrollBar()->setRange(0, std::max(0, int(std::ceil(m_content.height())) - vh));
    verticalScrollBar()->setPageStep(vh);
    horizontalScrollBar()->setRange(0, std::max(0, int(std::ceil(m_content.width())) - vw));
    horizontalScrollBar()->setPageStep(vw);
}

void PageView::applyZoomMode()
{
    if (!m_doc || m_mode == ZoomMode::Percent) return;
    const QSizeF ps = m_doc->pager().page(std::clamp(topPage(), 0, std::max(0, m_doc->pager().pagesKnown() - 1))).size;
    if (ps.isEmpty()) return;
    const double vw = viewport()->width(), vh = viewport()->height();
    double z = m_zoom;
    const double w = ps.width() * kPointToPixel, h = ps.height() * kPointToPixel;
    if (m_mode == ZoomMode::PageWidth) {
        z = (vw - 2 * kMargin) / w;
    } else if (m_mode == ZoomMode::OnePage) {
        z = std::min((vw - 2 * kMargin) / w, (vh - 2 * kMargin) / h);
    } else {
        z = std::min((vw - 2 * kMargin - kGap * (m_across - 1)) / (m_across * w), (vh - 2 * kMargin - kGap * (m_down - 1)) / (m_down * h));
    }
    z = std::clamp(z, kLeastZoom, kMostZoom);
    if (std::abs(z - m_zoom) > 1e-6) {
        m_zoom = z;
        Q_EMIT zoomChanged(m_zoom);
    }
}

void PageView::setZoom(double zoom)
{
    zoom = std::clamp(zoom, kLeastZoom, kMostZoom);
    m_mode = ZoomMode::Percent;
    m_across = m_down = 1;
    if (std::abs(zoom - m_zoom) < 1e-9) return;
    // The point in the middle of the window stays in the middle.
    const QPointF center(horizontalScrollBar()->value() + viewport()->width() / 2.0, verticalScrollBar()->value() + viewport()->height() / 2.0);
    int page = -1;
    QPointF fraction;
    for (int i = 0; i < m_rects.size(); ++i)
        if (m_rects[i].adjusted(0, -kGap, 0, kGap).contains(center)) {
            page = i;
            fraction = QPointF((center.x() - m_rects[i].left()) / m_rects[i].width(), (center.y() - m_rects[i].top()) / m_rects[i].height());
            break;
        }
    m_zoom = zoom;
    relayout();
    if (page >= 0 && page < m_rects.size()) {
        const QRectF r = m_rects[page];
        horizontalScrollBar()->setValue(int(r.left() + fraction.x() * r.width() - viewport()->width() / 2.0));
        verticalScrollBar()->setValue(int(r.top() + fraction.y() * r.height() - viewport()->height() / 2.0));
    }
    viewport()->update();
    Q_EMIT zoomChanged(m_zoom);
}

void PageView::setZoomMode(ZoomMode mode, int across, int down)
{
    m_mode = mode;
    m_across = mode == ZoomMode::ManyPages ? std::clamp(across, 1, 8) : 1;
    m_down = mode == ZoomMode::ManyPages ? std::clamp(down, 1, 4) : 1;
    if (mode == ZoomMode::Percent) return;
    applyZoomMode();
    relayout();
    if (!m_rects.isEmpty()) {
        // Many pages across start at the top of the page the window was on.
        const int page = std::clamp(topPage(), 0, int(m_rects.size()) - 1);
        verticalScrollBar()->setValue(int(m_rects[page].top() - kMargin));
        horizontalScrollBar()->setValue(0);
    }
    viewport()->update();
    Q_EMIT zoomChanged(m_zoom);
}

void PageView::setShowMarks(bool on)
{
    m_marks = on;
    viewport()->update();
}

void PageView::paginateAll()
{
    if (!m_doc) return;
    m_doc->pager().paginate(-1);
    relayout();
    viewport()->update();
}

void PageView::refresh()
{
    dropRendered(0);
    if (m_mode != ZoomMode::Percent) applyZoomMode();
    relayout();
    viewport()->update();
}

// ---- the background

void PageView::startPagination()
{
    if (m_doc && !m_pagination.isActive() && !m_doc->pager().complete()) m_pagination.start();
}

void PageView::paginationStep()
{
    if (!m_doc) return;
    flow::Paginator &pg = m_doc->pager();
    const bool done = pg.paginateFor(12);
    const int known = pg.pagesKnown();
    if (known != m_knownPages || done != m_complete || (done && !pg.settled())) {
        if (m_mode != ZoomMode::Percent && known != m_knownPages) applyZoomMode();
        relayout();
        viewport()->update();
    }
    if (!done) m_pagination.start();
}

void PageView::documentChanged(int position, int, int)
{
    if (!m_doc) return;
    const int page = m_doc->pager().pageOfPosition(std::clamp(position, 0, std::max(0, m_doc->story()->characterCount() - 1)));
    dropRendered(std::max(0, page - 1));
    relayout();
    startPagination();
    viewport()->update();
    Q_EMIT contentChanged();
}

// ---- drawing

void PageView::paintEvent(QPaintEvent *e)
{
    QPainter p(viewport());
    const bool dark = uiDark();
    p.fillRect(e->rect(), dark ? QColor(0x26, 0x26, 0x26) : QColor(0xE0, 0xE0, 0xE0));
    if (!m_doc) return;
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);
    const double sx = horizontalScrollBar()->value(), sy = verticalScrollBar()->value();
    const QRectF visible(sx + e->rect().left(), sy + e->rect().top(), e->rect().width(), e->rect().height());
    const double s = scale();
    flow::Paginator &pg = m_doc->pager();
    const int cols = std::max(1, m_across);
    // The first row that reaches into the window.
    int firstRow = int(std::upper_bound(m_rowTops.begin(), m_rowTops.end(), visible.top()) - m_rowTops.begin()) - 1;
    firstRow = std::max(0, firstRow);
    if (firstRow > 0) --firstRow;   // (a row above may be taller than its top says)
    bool recount = false;
    for (int i = firstRow * cols; i < m_rects.size(); ++i) {
        const QRectF r = m_rects[i];
        if (r.top() > visible.bottom()) break;
        if (!r.intersects(visible)) continue;
        const QRectF dev = r.translated(-sx, -sy);
        // The page's shadow and its paper.
        for (int k = 4; k >= 1; --k) p.fillRect(dev.adjusted(k, k, k, k), QColor(0, 0, 0, dark ? 28 : 14));
        p.fillRect(dev, Qt::white);
        const auto page = rendered(i);
        if (!page) {
            recount = true;
            continue;
        }
        if (page->page) {
            p.save();
            p.setClipRect(dev);
            p.translate(dev.topLeft());
            p.scale(s, s);
            PageDrawing d;
            d.marks = m_marks;
            d.pageCount = std::max(1, m_knownPages);
            if (m_cursor.hasSelection()) {
                d.selectionFrom = m_cursor.selectionStart();
                d.selectionTo = m_cursor.selectionEnd();
            }
            paintPage(&p, *page->page, &page->layouts, d);
            p.restore();
        }
        p.setPen(QPen(QColor(0, 0, 0, dark ? 90 : 50), 1));
        p.setBrush(Qt::NoBrush);
        p.drawRect(dev.adjusted(0.5, 0.5, -0.5, -0.5));
    }
    // The caret.
    if (m_caretOn && (hasFocus() || m_caretWithoutFocus) && !m_cursor.hasSelection()) {
        QRect r;
        if (caretRect(&r) && r.intersects(e->rect())) {
            p.setRenderHint(QPainter::Antialiasing, false);
            p.fillRect(QRect(r.x(), r.y(), std::max(1, int(std::lround(devicePixelRatioF() > 1 ? 1.5 : 1))), r.height()), QColor(0, 0, 0));
        }
    }
    if (recount || pg.pagesKnown() != m_knownPages) QTimer::singleShot(0, this, [this] {
        relayout();
        viewport()->update();
    });
}

void PageView::resizeEvent(QResizeEvent *e)
{
    QAbstractScrollArea::resizeEvent(e);
    if (m_mode != ZoomMode::Percent) applyZoomMode();
    relayout();
}

void PageView::scrollContentsBy(int, int) { viewport()->update(); }

QImage PageView::pageImage(int index, int width)
{
    const auto page = rendered(index);
    if (!page || !page->page) return QImage();
    const QSizeF size = page->page->size;
    QImage img(QSize(width, std::max(1, int(std::lround(width * size.height() / size.width())))), QImage::Format_RGB32);
    img.fill(Qt::white);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.scale(img.width() / size.width(), img.height() / size.height());
    PageDrawing d;
    d.pageCount = std::max(1, m_knownPages);
    d.output = true;
    paintPage(&p, *page->page, &page->layouts, d);
    return img;
}

// ---- the caret and the mouse

bool PageView::caretLine(int position, int *pageOut, QLineF *line)
{
    if (!m_doc) return false;
    flow::Paginator &pg = m_doc->pager();
    const int home = pg.pageOfPosition(std::clamp(position, 0, std::max(0, m_doc->story()->characterCount() - 1)));
    for (int page : {home, home - 1, home + 1}) {
        if (page < 0) continue;
        const auto r = rendered(page);
        if (!r || !r->page || r->boxes.isEmpty()) continue;
        const int inPage = position - r->page->storyFrom;
        if (inPage < 0) continue;
        const LayoutCache::FrameLayout fl = r->layouts.textFrame(*r->page->doc, *r->boxes.first(), r->page->number, RenderOptions());
        if (!fl.layout) continue;
        int frame = -1;
        QRectF rect;
        if (!fl.layout->caretRect(inPage, &frame, &rect) || frame < 0 || frame >= r->boxes.size()) continue;
        const TextItem *box = r->boxes[frame];
        const double x = box->rect.left() + rect.x();
        *line = QLineF(x, box->rect.top() + rect.top(), x, box->rect.top() + rect.bottom());
        *pageOut = page;
        return true;
    }
    return false;
}

bool PageView::caretRect(QRect *rect)
{
    int page = 0;
    QLineF line;
    if (!caretLine(m_cursor.position(), &page, &line) || page >= m_rects.size()) return false;
    const double s = scale();
    const QRectF pr = m_rects[page].translated(-horizontalScrollBar()->value(), -verticalScrollBar()->value());
    *rect = QRectF(pr.left() + line.x1() * s, pr.top() + line.y1() * s, 1, (line.y2() - line.y1()) * s).toAlignedRect();
    return true;
}

PageView::Hit PageView::hitAt(const QPoint &vp)
{
    Hit hit;
    if (m_rects.isEmpty()) return hit;
    const QPointF c(vp.x() + horizontalScrollBar()->value(), vp.y() + verticalScrollBar()->value());
    int best = 0;
    double bestDistance = 1e18;
    for (int i = 0; i < m_rects.size(); ++i) {
        const QRectF &r = m_rects[i];
        const double dx = c.x() < r.left() ? r.left() - c.x() : c.x() > r.right() ? c.x() - r.right() : 0;
        const double dy = c.y() < r.top() ? r.top() - c.y() : c.y() > r.bottom() ? c.y() - r.bottom() : 0;
        const double d = dx * dx + dy * dy * 4;
        if (d < bestDistance) {
            bestDistance = d;
            best = i;
        }
        if (d == 0) break;
    }
    hit.page = best;
    hit.point = QPointF((c.x() - m_rects[best].left()) / scale(), (c.y() - m_rects[best].top()) / scale());
    return hit;
}

int PageView::positionAt(const Hit &hit)
{
    if (hit.page < 0) return -1;
    const auto r = rendered(hit.page);
    const int last = std::max(0, m_doc->story()->characterCount() - 1);
    if (!r || !r->page || r->boxes.isEmpty()) return std::clamp(m_doc->pager().page(hit.page).start.pos, 0, last);
    // The column the point is in (else the nearest).
    int column = 0;
    double bestGap = 1e18;
    for (int i = 0; i < r->boxes.size(); ++i) {
        const QRectF &b = r->boxes[i]->rect;
        const double gap = hit.point.x() < b.left() ? b.left() - hit.point.x() : hit.point.x() > b.right() ? hit.point.x() - b.right() : 0;
        if (gap < bestGap) {
            bestGap = gap;
            column = i;
        }
    }
    const TextItem *box = r->boxes[column];
    const LayoutCache::FrameLayout fl = r->layouts.textFrame(*r->page->doc, *r->boxes.first(), r->page->number, RenderOptions());
    if (!fl.layout) return -1;
    const QVector<QRectF> lines = fl.layout->lineRects(column);
    if (lines.isEmpty()) return std::clamp(r->page->storyFrom + std::max(0, fl.layout->firstPosition(column)), 0, last);
    // The line nearest the point, then the place in it.
    const double y = hit.point.y() - box->rect.top();
    int line = 0;
    double bestDistance = 1e18;
    for (int i = 0; i < lines.size(); ++i) {
        const double d = y < lines[i].top() ? lines[i].top() - y : y > lines[i].bottom() ? y - lines[i].bottom() : 0;
        if (d < bestDistance) {
            bestDistance = d;
            line = i;
        }
    }
    const QPointF local(hit.point.x() - box->rect.left(), lines[line].center().y());
    int pos = fl.layout->hitTest(column, local);
    if (pos < 0) pos = hit.point.x() < box->rect.left() ? fl.layout->lineStart(fl.layout->firstPosition(column)) : fl.layout->lastPosition(column);
    return std::clamp(pos + r->page->storyFrom, 0, last);
}

int PageView::lineEdge(int position, bool end)
{
    int page = 0;
    QLineF line;
    if (!caretLine(position, &page, &line)) return position;
    const auto r = rendered(page);
    const LayoutCache::FrameLayout fl = r->layouts.textFrame(*r->page->doc, *r->boxes.first(), r->page->number, RenderOptions());
    if (!fl.layout) return position;
    const int inPage = position - r->page->storyFrom;
    return (end ? fl.layout->lineEnd(inPage) : fl.layout->lineStart(inPage)) + r->page->storyFrom;
}

int PageView::moveVertical(int position, int direction)
{
    int page = 0;
    QLineF line;
    if (!caretLine(position, &page, &line)) return position;
    const auto r = rendered(page);
    const LayoutCache::FrameLayout fl = r->layouts.textFrame(*r->page->doc, *r->boxes.first(), r->page->number, RenderOptions());
    if (!fl.layout) return position;
    const int inPage = position - r->page->storyFrom;
    if (m_wantedX < 0) {
        int frame = 0;
        QRectF rect;
        if (fl.layout->caretRect(inPage, &frame, &rect)) m_wantedX = rect.x();
    }
    const int moved = fl.layout->moveVertical(inPage, direction, m_wantedX);
    if (moved != inPage) return moved + r->page->storyFrom;
    // At the first or last line of the page: on to the neighbor.
    const auto next = rendered(page + direction);
    if (!next || !next->page || next->boxes.isEmpty()) return position;
    const LayoutCache::FrameLayout nl = next->layouts.textFrame(*next->page->doc, *next->boxes.first(), next->page->number, RenderOptions());
    if (!nl.layout) return position;
    const int column = direction > 0 ? 0 : int(next->boxes.size()) - 1;
    const QVector<QRectF> lines = nl.layout->lineRects(column);
    if (lines.isEmpty()) return position;
    const QRectF target = direction > 0 ? lines.first() : lines.last();
    const int at = nl.layout->hitTest(column, QPointF(m_wantedX, target.center().y()));
    return at < 0 ? position : at + next->page->storyFrom;
}

void PageView::ensureCaretVisible()
{
    QRect r;
    if (!caretRect(&r)) return;
    const int margin = 36;
    if (r.top() < margin) verticalScrollBar()->setValue(verticalScrollBar()->value() + r.top() - margin);
    else if (r.bottom() > viewport()->height() - margin) verticalScrollBar()->setValue(verticalScrollBar()->value() + r.bottom() - viewport()->height() + margin);
    if (r.left() < margin) horizontalScrollBar()->setValue(horizontalScrollBar()->value() + r.left() - margin);
    else if (r.right() > viewport()->width() - margin) horizontalScrollBar()->setValue(horizontalScrollBar()->value() + r.right() - viewport()->width() + margin);
}

void PageView::caretMoved(bool keepWanted)
{
    if (!keepWanted) m_wantedX = -1;
    m_caretOn = true;
    if (hasFocus()) m_blink.start();
    ensureCaretVisible();
    viewport()->update();
    Q_EMIT caretChanged();
}

void PageView::blink()
{
    m_caretOn = !m_caretOn;
    QRect r;
    if (caretRect(&r)) viewport()->update(r.adjusted(-2, -2, 2, 2));
}

void PageView::focusInEvent(QFocusEvent *e)
{
    QAbstractScrollArea::focusInEvent(e);
    m_caretOn = true;
    m_blink.start();
    viewport()->update();
}

void PageView::focusOutEvent(QFocusEvent *e)
{
    QAbstractScrollArea::focusOutEvent(e);
    m_blink.stop();
    viewport()->update();
}

bool PageView::focusNextPrevChild(bool) { return false; }

} // namespace jp::write
