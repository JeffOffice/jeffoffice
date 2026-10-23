#include "app/pageview.h"

#include "app/icons.h"
#include "app/pagepaint.h"
#include "core/editops.h"
#include "core/spellcheck.h"
#include "render/renderer.h"

#include <QApplication>
#include <QImage>
#include <QPaintEvent>
#include <QPainter>
#include <QMap>
#include <QScrollBar>
#include <QSet>
#include <QTimer>
#include <QTextBlock>
#include <QTextDocument>
#include <algorithm>
#include <cmath>
#include <limits>

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

struct PageView::FrameSpot {
    const TextItem *box = nullptr;
    LayoutCache::FrameLayout fl;
    int inFrame = 0;     // the position in the box's own text
    int from = 0;        // where the frame's text begins in the story
};

// Whether the place is on the first (dir < 0) or the last (dir > 0) line of the frame's text.
bool PageView::onEdgeLine(const FrameSpot &spot, int dir)
{
    // (The frame's end is a place of its own, after the last character: the last line is the one that has that character.)
    const int edge = dir < 0 ? spot.fl.layout->firstPosition(0) : std::max(spot.fl.layout->firstPosition(0), spot.fl.layout->lastPosition(0) - 1);
    return spot.fl.layout->lineStart(spot.inFrame) == spot.fl.layout->lineStart(edge);
}

bool PageView::frameSpotAt(Rendered &r, int position, FrameSpot *spot)
{
    if (!r.page) return false;
    for (const auto &fb : r.page->frames) {
        if (position < fb.from || position >= fb.to) continue;
        for (const auto &it : r.page->doc->pages[0]->items) {
            if (it->id != fb.itemId || it->type() != ItemType::Text) continue;
            spot->box = static_cast<const TextItem *>(it.get());
            spot->fl = r.layouts.textFrame(*r.page->doc, *spot->box, r.page->number, RenderOptions());
            spot->from = fb.from;
            spot->inFrame = position - fb.from;
            return spot->fl.layout != nullptr;
        }
    }
    return false;
}

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
    if (m_note.active) {
        QObject::disconnect(m_hfEdit);
        m_note = NoteEdit();
        m_hfSpell.reset();
        m_bodyCursor = QTextCursor();
        Q_EMIT noteModeChanged();
    }
    if (m_hf.active) {
        QObject::disconnect(m_hfEdit);
        if (m_doc) m_doc->pager().setEditedStory(nullptr);
        m_hf = HeaderFooterEdit();
        m_hfSpell.reset();
        Q_EMIT headerFooterModeChanged();
    }
    if (m_connected) disconnect(m_connected, nullptr, this, nullptr);
    m_connected = nullptr;
    if (m_doc) disconnect(m_doc, nullptr, this, nullptr);
    if (m_doc) m_doc->discardPendingHeaderFooter();   // (the empty header or footer that was shown to be typed in is not kept)
    m_spell.reset();
    m_editEnd = -1;
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
    m_spell = std::make_unique<SpellCheck>(doc->story());
    m_spell->setDefaultLanguage(doc->proofingLanguage());
    m_spell->setEnabled(m_spellOn);
    connect(m_spell.get(), &SpellCheck::changed, viewport(), qOverload<>(&QWidget::update));
    connect(m_connected, &QTextDocument::contentsChange, this, &PageView::documentChanged);
    connect(doc, &WriteDoc::sectionsChanged, this, [this] {
        dropRendered(0);
        if (m_mode != ZoomMode::Percent) applyZoomMode();
        relayout();
        viewport()->update();
        // (The header being edited may be another now, or none: after the change is done.)
        if (m_hf.active) QTimer::singleShot(0, this, [this] { refreshHeaderFooter(false); });
    });
    connect(doc, &WriteDoc::headerFooterChanged, this, [this](QTextDocument *) {
        dropRendered(0);
        relayout();
        startPagination();
        viewport()->update();
        Q_EMIT contentChanged();
    });
    connect(doc, &WriteDoc::noteChanged, this, [this](QTextDocument *) {
        dropRendered(0);
        relayout();
        startPagination();
        viewport()->update();
        settleNote();
        Q_EMIT contentChanged();
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

bool PageView::takeCursorOf(const QTextCursor &c)
{
    QTextDocument *d = c.document();
    if (!m_doc || !d) return true;
    if (m_hf.active && d == m_hf.story) return true;
    if (m_note.active && d == m_note.story) return true;
    if (int id = 0; m_doc->footnoteIdOf(d, &id)) {
        // A cursor in a note's story is the view's if the note is on a page: the note is edited there.
        const int page = pageShowingNote(d);
        return page >= 0 && enterNote(d, id, page);
    }
    if (d != m_doc->story()) return false;
    if (m_hf.active) leaveHeaderFooter();
    if (m_note.active) leaveNote();
    return true;
}

void PageView::setCursor(const QTextCursor &c)
{
    if (!takeCursorOf(c)) return;
    m_cursor = c;
    caretMoved();
}

void PageView::showCursor(const QTextCursor &c)
{
    if (!m_doc || !takeCursorOf(c)) return;
    // A page is known once the one before it is laid out: lay out pages until the one with the position on it is.
    flow::Paginator &pg = m_doc->pager();
    while (!pg.complete()) {
        const int known = pg.pagesKnown();
        if (known > 0 && c.position() < pg.page(known - 1).start.pos) break;
        pg.paginate(known);
        if (pg.pagesKnown() == known) break;
    }
    relayout();
    m_cursor = c;
    QRect r;
    if (caretRect(&r) && (r.top() < 0 || r.bottom() > viewport()->height())) verticalScrollBar()->setValue(verticalScrollBar()->value() + r.center().y() - viewport()->height() / 2);
    caretMoved();
}

// ---- pages

int PageView::pageCount() const { return m_knownPages; }
bool PageView::pagesComplete() const { return m_complete; }

int PageView::currentPage() const
{
    if (!m_doc) return 0;
    if (m_hf.active || m_note.active) return std::clamp(sidePage(), 0, std::max(0, m_knownPages - 1));
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
    // (The page's text boxes, a column each, in order: a header, a footer, a note, and the numbers of the lines are text items too, and none of the text's columns.)
    if (r->page)
        for (const QString &id : r->page->boxes)
            if (Item *item = r->page->doc->item(id))
                if (item->type() == ItemType::Text) r->boxes.push_back(static_cast<TextItem *>(item));
    m_rendered.insert(index, r);
    m_recent.prepend(index);
    while (m_recent.size() > kKeptPages) m_rendered.remove(m_recent.takeLast());
    return r;
}

int PageView::shownTotal(int index) const
{
    const auto found = m_rendered.constFind(index);
    if (found == m_rendered.constEnd() || !found.value()->page || !found.value()->page->showsTotal) return -1;
    return found.value()->page->pageTotal;
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
    // (A page that shows the number of pages was made with the number there was: with another, it is made again. So is one that shows the pages bookmarks are on, as far as the pages were
    // laid out when it was made.)
    if (changed)
        for (auto it = m_rendered.begin(); it != m_rendered.end();) {
            if (it.value()->page && (it.value()->page->showsTotal || it.value()->page->showsRefs)) {
                m_recent.removeOne(it.key());
                it = m_rendered.erase(it);
            } else {
                ++it;
            }
        }
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

void PageView::setSpellingEnabled(bool on)
{
    m_spellOn = on;
    if (m_spell) m_spell->setEnabled(on);
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

void PageView::documentChanged(int position, int, int added)
{
    if (!m_doc) return;
    m_editEnd = position + added;
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
    // (The misspelled words of the pages are found within a few milliseconds; what is left waits for idle time.)
    SpellCheck::Pass spellingTime(*m_spell, 6);
    std::unique_ptr<SpellCheck::Pass> headerSpellingTime;
    if (m_hfSpell) headerSpellingTime = std::make_unique<SpellCheck::Pass>(*m_hfSpell, 6);
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
            if (m_hf.active) {
                d.editedStory = m_hf.story;
                d.editFooter = m_hf.footer;
                d.editSpelling = m_hfSpell.get();
                d.editTypingAt = !m_cursor.hasSelection() && m_cursor.position() == m_editEnd ? m_cursor.position() : -1;
                if (m_cursor.hasSelection()) {
                    d.editSelFrom = m_cursor.selectionStart();
                    d.editSelTo = m_cursor.selectionEnd();
                }
            } else if (m_note.active) {
                // (A note is edited where it stands: the text of the page is not dimmed, and its own misspelled words show.)
                d.spelling = m_spell.get();
                d.editedStory = m_note.story;
                d.editIsNote = true;
                d.editSpelling = m_hfSpell.get();
                d.editTypingAt = !m_cursor.hasSelection() && m_cursor.position() == m_editEnd ? m_cursor.position() : -1;
                if (m_cursor.hasSelection()) {
                    d.editSelFrom = m_cursor.selectionStart();
                    d.editSelTo = m_cursor.selectionEnd();
                }
            } else {
                d.spelling = m_spell.get();
                d.typingAt = !m_cursor.hasSelection() && m_cursor.position() == m_editEnd ? m_cursor.position() : -1;
                if (m_cursor.hasSelection()) {
                    d.selectionFrom = m_cursor.selectionStart();
                    d.selectionTo = m_cursor.selectionEnd();
                }
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
    if (m_hf.active || m_note.active) return hfCaretLine(position, pageOut, line);
    flow::Paginator &pg = m_doc->pager();
    const int home = pg.pageOfPosition(std::clamp(position, 0, std::max(0, m_doc->story()->characterCount() - 1)));
    QVector<int> candidates;
    // (A row of a table that goes over page breaks is on every page that starts in it: the page that has the part with the position's line is the one to look on first.)
    const int direct = pg.pageOfTablePosition(std::clamp(position, 0, std::max(0, m_doc->story()->characterCount() - 1)));
    if (direct >= 0) candidates << direct;
    candidates << home << home - 1 << home + 1;
    for (int page : candidates) {
        if (page < 0) continue;
        const auto r = rendered(page);
        if (!r || !r->page) continue;
        // A position in a cell of a table is where the table's own layout of it puts it.
        for (const auto &fragment : r->page->tables) {
            QLineF cellLine;
            if (fragment->caretLine(position, &cellLine)) {
                *line = cellLine;
                *pageOut = page;
                return true;
            }
        }
        // A position in a frame's text is where the frame's box draws it.
        if (FrameSpot spot; frameSpotAt(*r, position, &spot)) {
            int frame = -1;
            QRectF rect;
            if (spot.fl.layout->caretRect(spot.inFrame, &frame, &rect)) {
                const double x = spot.box->rect.left() + rect.x();
                *line = QLineF(x, spot.box->rect.top() + rect.top(), x, spot.box->rect.top() + rect.bottom());
                *pageOut = page;
                return true;
            }
        }
        if (r->boxes.isEmpty()) continue;
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
    if (m_hf.active || m_note.active) return hfPositionAt(hit);
    const auto r = rendered(hit.page);
    const int last = std::max(0, m_doc->story()->characterCount() - 1);
    // A point in the rows of a table (beside them, too) is in the nearest of the cells.
    if (r && r->page)
        for (const auto &fragment : r->page->tables) {
            const QRectF fr = fragment->rect();
            if (hit.point.y() < fr.top() - 1 || hit.point.y() > fr.bottom() + 1) continue;
            const int at = fragment->positionAt(hit.point);
            if (at >= 0) return std::clamp(at, 0, last);
        }
    // A point in a frame's box is in the frame's text (its paragraphs have no lines in the page's).
    if (r && r->page)
        for (const auto &fb : r->page->frames) {
            for (const auto &it : r->page->doc->pages[0]->items) {
                if (it->id != fb.itemId || it->type() != ItemType::Text || !it->rect.contains(hit.point)) continue;
                const auto *box = static_cast<const TextItem *>(it.get());
                const LayoutCache::FrameLayout fl = r->layouts.textFrame(*r->page->doc, *box, r->page->number, RenderOptions());
                if (!fl.layout) continue;
                int at = fl.layout->hitTest(0, QPointF(hit.point.x() - box->rect.left(), hit.point.y() - box->rect.top()));
                if (at < 0) at = fl.layout->lastPosition(0);
                return std::clamp(fb.from + at, fb.from, std::min(last, fb.to - 1));
            }
        }
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
    if (m_hf.active || m_note.active) return hfLineEdge(position, end);
    int page = 0;
    QLineF line;
    if (!caretLine(position, &page, &line)) return position;
    const auto r = rendered(page);
    for (const auto &fragment : r->page->tables) {
        int edge = 0;
        if (fragment->lineEdge(position, end, &edge)) return edge;
    }
    if (FrameSpot spot; frameSpotAt(*r, position, &spot)) return spot.from + (end ? spot.fl.layout->lineEnd(spot.inFrame) : spot.fl.layout->lineStart(spot.inFrame));
    const LayoutCache::FrameLayout fl = r->layouts.textFrame(*r->page->doc, *r->boxes.first(), r->page->number, RenderOptions());
    if (!fl.layout) return position;
    const int inPage = position - r->page->storyFrom;
    return (end ? fl.layout->lineEnd(inPage) : fl.layout->lineStart(inPage)) + r->page->storyFrom;
}

int PageView::moveVertical(int position, int direction)
{
    if (m_hf.active || m_note.active) return hfMoveVertical(position, direction);
    int page = 0;
    QLineF line;
    if (!caretLine(position, &page, &line)) return position;
    const auto r = rendered(page);
    for (const auto &fragment : r->page->tables)
        if (fragment->contains(position)) {
            const int moved = fragment->moveVertical(position, direction, &m_wantedX);
            if (moved != position) return moved;
            break;
        }
    // In a frame's text: by the lines of the frame's box, and no further (the frame is no part of the page's lines).
    if (FrameSpot spot; frameSpotAt(*r, position, &spot)) {
        if (m_wantedX < 0) {
            int frame = 0;
            QRectF rect;
            if (spot.fl.layout->caretRect(spot.inFrame, &frame, &rect)) m_wantedX = rect.x();
        }
        return spot.from + spot.fl.layout->moveVertical(spot.inFrame, direction, m_wantedX);
    }
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
    // (The endnotes the pages set are not the person's: no caret or selection reaches them. A caret that has gone on past those at the end of a section is at the start of the next
    // section's text, one that has gone back at the end of the last section's, and a click in them is at the nearer place.)
    int direction = 0;
    const int movedFrom = m_movedFrom;
    if (m_movedFrom >= 0 && m_cursor.document()) direction = m_cursor.position() > m_movedFrom ? 1 : m_cursor.position() < m_movedFrom ? -1 : 0;
    m_movedFrom = -1;
    if (m_doc && m_cursor.document() && !holdInEndnote(movedFrom)) holdInBody(m_cursor, direction);
    syncNotePage();
    m_contextAt = -1;
    if (!keepWanted) m_wantedX = -1;
    m_caretOn = true;
    if (hasFocus()) m_blink.start();
    ensureCaretVisible();
    viewport()->update();
    Q_EMIT caretChanged();
}

bool PageView::holdInEndnote(int movedFrom)
{
    QTextDocument *d = m_cursor.document();
    if (!d || !m_doc || d != m_doc->story()) return false;
    // Where it began: a key or a drag went from a place (in a note or not), a click began where its anchor is. Only what begins in a note's words is held in them; a key that goes from
    // the text into the notes is not (it skips them, as ever).
    const EndnoteSpan s = endnoteSpanAt(d, movedFrom >= 0 ? movedFrom : m_cursor.anchor());
    if (!(s.valid && s.editable)) return false;
    const int began = movedFrom >= 0 ? movedFrom : m_cursor.anchor();
    if (began < s.blockFrom || began > s.textTo) return false;
    const int position = std::clamp(m_cursor.position(), s.textFrom, s.textTo), anchor = std::clamp(m_cursor.anchor(), s.textFrom, s.textTo);
    if (position != m_cursor.position() || anchor != m_cursor.anchor()) {
        m_cursor.setPosition(anchor);
        m_cursor.setPosition(position, QTextCursor::KeepAnchor);
    }
    return true;
}

void PageView::leaveEndnote(int id)
{
    if (!m_doc) return;
    const QVector<int> refs = m_doc->endnoteReferences(id);
    if (refs.isEmpty()) return;
    m_cursor.setPosition(std::min(refs.first() + 1, m_doc->story()->characterCount() - 1));
    m_movedFrom = -1;   // (it went from a place in the note: the text is not held by the note)
    caretMoved();
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

// ---- headers and footers

PageView::Area PageView::areaOf(const Hit &hit)
{
    if (hit.page < 0) return Area::Body;
    const auto r = rendered(hit.page);
    if (!r || !r->page) return Area::Body;
    // (Only a point on the page itself is in a header's or a footer's area.)
    if (!QRectF(QPointF(0, 0), r->page->size).contains(hit.point)) return Area::Body;
    const QRectF body = r->page->bodyArea;
    if (hit.point.y() < body.top()) return Area::Header;
    if (hit.point.y() > body.bottom()) return Area::Footer;
    return Area::Body;
}

PageView::Area PageView::areaAt(const QPoint &viewportPos) { return areaOf(hitAt(viewportPos)); }

bool PageView::enterHeaderFooter(int page, bool footer, bool makeNow)
{
    if (!m_doc) return false;
    flow::Paginator &pg = m_doc->pager();
    pg.paginate(page);
    if (page < 0 || page >= pg.pagesKnown()) return false;
    const flow::Paginator::Page p = pg.page(page);
    if (p.start.blank) return false;   // (a page with nothing on it: no header or footer is shown on it, so there is nothing to edit)
    const int section = p.start.section;
    return enterHeaderFooterFor(section, footer, m_doc->headerSlotOf(section, page <= 0 || p.start.firstOfSection, p.number), page, makeNow);
}

bool PageView::enterHeaderFooterAtCaret(bool footer) { return enterHeaderFooter(currentPage(), footer, true); }

bool PageView::enterHeaderFooterFor(int section, bool footer, int slot, int page, bool makeNow)
{
    if (!m_doc) return false;
    leaveSideModeFor(false);
    if (m_hf.active && m_hf.section == section && m_hf.footer == footer && m_hf.slot == slot && m_doc->headerFooterStory(section, footer, slot) == m_hf.story) {
        m_hf.page = page;
        if (makeNow) m_doc->createHeaderFooter(section, footer, slot);
        viewport()->update();
        return true;
    }
    // (The empty one that was shown to be typed in goes when another is entered.)
    if (m_hf.active && m_doc->isPendingHeaderFooter(m_hf.story)) leaveHeaderFooter();
    if (!m_doc->createHeaderFooter(section, footer, slot, !makeNow)) return false;
    QTextDocument *story = m_doc->editableHeaderFooter(section, footer, slot);
    if (!story) return false;
    const bool was = m_hf.active;
    if (!was) m_bodyCursor = m_cursor;
    else QObject::disconnect(m_hfEdit);
    m_hf = HeaderFooterEdit{true, section, slot, page, footer, story};
    m_doc->pager().setEditedStory(story);
    m_hfSpell = std::make_unique<SpellCheck>(story);
    m_hfSpell->setDefaultLanguage(m_doc->proofingLanguage());
    m_hfSpell->setEnabled(m_spellOn);
    connect(m_hfSpell.get(), &SpellCheck::changed, viewport(), qOverload<>(&QWidget::update));
    m_hfEdit = connect(story, &QTextDocument::contentsChange, this, [this](int position, int, int added) { m_editEnd = position + added; });
    m_cursor = QTextCursor(story);
    m_cursor.setPosition(0);
    m_wantedX = -1;
    m_movedFrom = -1;
    m_dragging = false;
    m_editEnd = -1;
    dropRendered(0);
    relayout();
    viewport()->update();
    Q_EMIT headerFooterModeChanged();
    caretMoved();
    return true;
}

void PageView::leaveSideModeFor(bool noteMode)
{
    if (noteMode) {
        if (m_hf.active) leaveHeaderFooter();
    } else if (m_note.active) {
        leaveNote();
    }
}

void PageView::restoreBodyCursor()
{
    if (m_doc && m_bodyCursor.document() == m_doc->story()) {
        m_cursor = m_bodyCursor;
    } else if (m_doc) {
        m_cursor = QTextCursor(m_doc->story());
        m_cursor.setPosition(0);
    }
    m_bodyCursor = QTextCursor();
}

void PageView::leaveHeaderFooter()
{
    if (!m_hf.active) return;
    QObject::disconnect(m_hfEdit);
    if (m_doc) m_doc->pager().setEditedStory(nullptr);
    m_hf = HeaderFooterEdit();
    m_hfSpell.reset();
    m_dragging = false;
    m_editEnd = -1;
    restoreBodyCursor();
    if (m_doc) m_doc->discardPendingHeaderFooter();   // (an empty one that nothing was put in is not made)
    dropRendered(0);
    relayout();
    viewport()->update();
    Q_EMIT headerFooterModeChanged();
    caretMoved();
}

void PageView::refreshHeaderFooter(bool create)
{
    if (!m_hf.active || !m_doc) return;
    if (m_hf.section >= m_doc->sectionCount()) {
        leaveHeaderFooter();
        return;
    }
    flow::Paginator &pg = m_doc->pager();
    pg.paginate(m_hf.page);
    if (pg.pagesKnown() == 0) return;
    const int page = std::clamp(m_hf.page, 0, pg.pagesKnown() - 1);
    const flow::Paginator::Page p = pg.page(page);
    if (p.start.blank) {
        leaveHeaderFooter();
        return;
    }
    const int section = p.start.section, slot = m_doc->headerSlotOf(section, page <= 0 || p.start.firstOfSection, p.number);
    QTextDocument *now = m_doc->headerFooterStory(section, m_hf.footer, slot);
    if (now == m_hf.story && section == m_hf.section && slot == m_hf.slot) return;
    if ((!now && !create) || !enterHeaderFooterFor(section, m_hf.footer, slot, page)) leaveHeaderFooter();
}

int PageView::pageShowing(int section, bool footer, int slot)
{
    Q_UNUSED(footer);
    flow::Paginator &pg = m_doc->pager();
    const int known = pg.pagesKnown();
    if (known <= 0) return -1;
    const int from = std::clamp(topPage(), 0, known - 1);
    for (int step = 0; step < known && step < 4000; ++step) {
        const int i = (from + step) % known;
        const flow::Paginator::Page p = pg.page(i);
        if (!p.start.blank && p.start.section == section && m_doc->headerSlotOf(section, i <= 0 || p.start.firstOfSection, p.number) == slot) return i;
    }
    return -1;
}

bool PageView::takeFurnitureAt(const Hit &hit, bool footer)
{
    if (hit.page < 0) return false;
    flow::Paginator &pg = m_doc->pager();
    pg.paginate(hit.page);
    if (hit.page >= pg.pagesKnown()) return false;
    const flow::Paginator::Page p = pg.page(hit.page);
    if (p.start.blank) return false;
    const int section = p.start.section, slot = m_doc->headerSlotOf(section, hit.page <= 0 || p.start.firstOfSection, p.number);
    if (m_hf.active && m_hf.section == section && m_hf.footer == footer && m_hf.slot == slot) {
        if (m_hf.page != hit.page) {
            m_hf.page = hit.page;
            viewport()->update();
        }
        return true;
    }
    return enterHeaderFooterFor(section, footer, slot, hit.page);
}

void PageView::followHistory(QTextDocument *story, const QTextCursor &at)
{
    if (story == m_doc->story()) {
        if (m_hf.active) leaveHeaderFooter();
        if (m_note.active) leaveNote();
        m_cursor = at;
    } else if ((m_hf.active && m_hf.story == story) || (m_note.active && m_note.story == story)) {
        m_cursor = at;
    } else if (int id = 0; m_doc->footnoteIdOf(story, &id)) {
        // (A note that is on no page, its reference gone, is left alone: the view stays where it is.)
        const int page = pageShowingNote(story);
        if (page >= 0 && enterNote(story, id, page)) m_cursor = at;
    } else {
        int section = 0, slot = 0;
        bool footer = false;
        if (m_doc->headerFooterSlotOf(story, &section, &footer, &slot)) {
            if (m_note.active) leaveNote();
            const int page = pageShowing(section, footer, slot);
            if (page >= 0 && enterHeaderFooterFor(section, footer, slot, page)) m_cursor = at;
        }
    }
    if (m_hf.active) refreshHeaderFooter(false);
    caretMoved();
}

static const QVector<flow::MaterializedPage::FurnitureBox> &boxesOf(const flow::MaterializedPage &page, bool note) { return note ? page.noteBoxes : page.furniture; }

// Where the boxes of the edited story on the page being edited put a position: the last box that begins at or before it (a story with tables is a box for each stretch of text).
bool PageView::hfSpotAt(Rendered &r, int position, FrameSpot *spot)
{
    if (!r.page) return false;
    const flow::MaterializedPage::FurnitureBox *best = nullptr;
    // (A page can set two copies of a note, and a part of one that goes on from the page before beside the start of another: the boxes of the copy being edited are the ones to look at.)
    const auto &boxes = boxesOf(*r.page, m_note.active);
    const bool ofCopy = m_note.active && m_note.number != 0 && std::any_of(boxes.cbegin(), boxes.cend(), [&](const auto &fb) { return fb.source == sideStory() && fb.number == m_note.number; });
    for (const auto &fb : boxes)
        if (fb.source == sideStory() && (!ofCopy || fb.number == m_note.number) && fb.from <= position && (!best || fb.from >= best->from)) best = &fb;
    if (!best) return false;
    for (const auto &it : r.page->doc->pages[0]->items) {
        if (it->id != best->itemId || it->type() != ItemType::Text) continue;
        spot->box = static_cast<const TextItem *>(it.get());
        spot->fl = r.layouts.textFrame(*r.page->doc, *spot->box, r.page->number, RenderOptions());
        spot->from = best->from;
        spot->inFrame = position - best->from;
        return spot->fl.layout != nullptr;
    }
    return false;
}

bool PageView::hfCaretLine(int position, int *pageOut, QLineF *line)
{
    const auto r = rendered(sidePage());
    if (!r || !r->page) return false;
    if (m_hf.active)
        for (int i = 0; i < r->page->furnitureTables.size(); ++i) {
            if (r->page->furnitureTableSources.value(i) != sideStory()) continue;
            QLineF cellLine;
            if (r->page->furnitureTables[i]->caretLine(position, &cellLine)) {
                *line = cellLine;
                *pageOut = sidePage();
                return true;
            }
        }
    FrameSpot spot;
    if (!hfSpotAt(*r, position, &spot)) return false;
    int frame = -1;
    QRectF rect;
    if (!spot.fl.layout->caretRect(spot.inFrame, &frame, &rect)) return false;
    const double x = spot.box->rect.left() + rect.x();
    *line = QLineF(x, spot.box->rect.top() + rect.top(), x, spot.box->rect.top() + rect.bottom());
    *pageOut = sidePage();
    return true;
}

int PageView::hfPositionAt(const Hit &hit)
{
    const auto r = rendered(hit.page);
    const int last = std::max(0, sideStory()->characterCount() - 1);
    if (!r || !r->page) return -1;
    if (m_hf.active)
        for (int i = 0; i < r->page->furnitureTables.size(); ++i) {
            if (r->page->furnitureTableSources.value(i) != sideStory()) continue;
            const QRectF fr = r->page->furnitureTables[i]->rect();
            if (hit.point.y() < fr.top() - 1 || hit.point.y() > fr.bottom() + 1) continue;
            const int at = r->page->furnitureTables[i]->positionAt(hit.point);
            if (at >= 0) return std::clamp(at, 0, last);
        }
    // The box of the edited story nearest the point (above or below it).
    const TextItem *nearest = nullptr;
    const flow::MaterializedPage::FurnitureBox *nearestBox = nullptr;
    double bestGap = 1e18;
    for (const auto &fb : boxesOf(*r->page, m_note.active)) {
        if (fb.source != sideStory()) continue;
        for (const auto &it : r->page->doc->pages[0]->items) {
            if (it->id != fb.itemId || it->type() != ItemType::Text) continue;
            const QRectF &b = it->rect;
            const double gap = hit.point.y() < b.top() ? b.top() - hit.point.y() : hit.point.y() > b.bottom() ? hit.point.y() - b.bottom() : 0;
            if (gap < bestGap) {
                bestGap = gap;
                nearest = static_cast<const TextItem *>(it.get());
                nearestBox = &fb;
            }
        }
    }
    if (!nearest) return -1;
    const LayoutCache::FrameLayout fl = r->layouts.textFrame(*r->page->doc, *nearest, r->page->number, RenderOptions());
    if (!fl.layout) return -1;
    const QVector<QRectF> lines = fl.layout->lineRects(0);
    if (lines.isEmpty()) return std::clamp(nearestBox->from + std::max(0, fl.layout->firstPosition(0)), 0, last);
    const double y = hit.point.y() - nearest->rect.top();
    int line = 0;
    double bestDistance = 1e18;
    for (int i = 0; i < lines.size(); ++i) {
        const double d = y < lines[i].top() ? lines[i].top() - y : y > lines[i].bottom() ? y - lines[i].bottom() : 0;
        if (d < bestDistance) {
            bestDistance = d;
            line = i;
        }
    }
    const QPointF local(hit.point.x() - nearest->rect.left(), lines[line].center().y());
    int pos = fl.layout->hitTest(0, local);
    if (pos < 0) pos = hit.point.x() < nearest->rect.left() ? fl.layout->lineStart(fl.layout->firstPosition(0)) : fl.layout->lastPosition(0);
    return std::clamp(pos + nearestBox->from, 0, last);
}

int PageView::hfLineEdge(int position, bool end)
{
    const auto r = rendered(sidePage());
    if (!r || !r->page) return position;
    if (m_hf.active)
        for (int i = 0; i < r->page->furnitureTables.size(); ++i) {
            if (r->page->furnitureTableSources.value(i) != sideStory()) continue;
            int edge = 0;
            if (r->page->furnitureTables[i]->lineEdge(position, end, &edge)) return edge;
        }
    FrameSpot spot;
    if (!hfSpotAt(*r, position, &spot)) return position;
    return spot.from + (end ? spot.fl.layout->lineEnd(spot.inFrame) : spot.fl.layout->lineStart(spot.inFrame));
}

int PageView::hfMoveVertical(int position, int direction)
{
    const auto r = rendered(sidePage());
    if (!r || !r->page) return position;
    if (m_hf.active)
        for (int i = 0; i < r->page->furnitureTables.size(); ++i) {
            if (r->page->furnitureTableSources.value(i) != sideStory() || !r->page->furnitureTables[i]->contains(position)) continue;
            const int moved = r->page->furnitureTables[i]->moveVertical(position, direction, &m_wantedX);
            if (moved != position) return moved;
            break;
        }
    FrameSpot spot;
    if (!hfSpotAt(*r, position, &spot)) return position;
    if (m_wantedX < 0) {
        int frame = 0;
        QRectF rect;
        if (spot.fl.layout->caretRect(spot.inFrame, &frame, &rect)) m_wantedX = rect.x();
    }
    // A note's line at the edge of a part that goes on goes on into the next part (the first line of it) or back to the last line of the part before.
    if (m_note.active) {
        if (onEdgeLine(spot, direction)) {
            const QVector<NotePart> parts = partsOfEditedNote(m_note.page);
            const int at = partIndexAt(parts, position);
            if (direction > 0 && at + 1 < parts.size()) return parts[at + 1].from;
            if (direction < 0 && at > 0) return std::max(0, parts[at].from - 1);
        }
    }
    return spot.from + spot.fl.layout->moveVertical(spot.inFrame, direction, m_wantedX);
}

// ---- footnotes

bool PageView::noteAt(const Hit &hit, QTextDocument **story, int *id, int *number)
{
    if (hit.page < 0) return false;
    const auto r = rendered(hit.page);
    if (!r || !r->page) return false;
    for (const auto &nb : r->page->noteBoxes) {
        const Item *item = r->page->doc->item(nb.itemId);
        if (!item || !item->rect.contains(hit.point)) continue;
        int noteId = 0;
        if (!m_doc->footnoteIdOf(nb.source, &noteId)) continue;
        if (story) *story = m_doc->editableFootnote(noteId);
        if (id) *id = noteId;
        if (number) *number = nb.number;
        return story ? *story != nullptr : true;
    }
    return false;
}

int PageView::pageShowingNote(const QTextDocument *story)
{
    int id = 0;
    if (!m_doc || !m_doc->footnoteIdOf(story, &id)) return -1;
    flow::Paginator &pg = m_doc->pager();
    const QVector<int> refs = m_doc->footnoteReferences(id);
    for (int k = 0; k < refs.size() && k < 8; ++k) {
        const int home = pg.pageOfPosition(std::clamp(refs[k], 0, std::max(0, m_doc->story()->characterCount() - 1)));
        for (int page = home; page <= home + 2; ++page) {
            const auto r = rendered(page);
            if (!r || !r->page) continue;
            for (const auto &nb : r->page->noteBoxes)
                if (nb.source == story) return page;
        }
    }
    return -1;
}

bool PageView::enterNote(QTextDocument *story, int id, int page, int number)
{
    if (!m_doc || !story) return false;
    if (m_note.active && m_note.story == story) {
        m_note.page = page;
        chooseCopy(page, number);
        viewport()->update();
        return true;
    }
    leaveSideModeFor(true);
    const bool was = m_note.active;
    if (!was) m_bodyCursor = m_cursor;
    else QObject::disconnect(m_hfEdit);
    m_note = NoteEdit{true, id, page, story};
    chooseCopy(page, number);
    m_hfSpell = std::make_unique<SpellCheck>(story);
    m_hfSpell->setDefaultLanguage(m_doc->proofingLanguage());
    m_hfSpell->setEnabled(m_spellOn);
    connect(m_hfSpell.get(), &SpellCheck::changed, viewport(), qOverload<>(&QWidget::update));
    m_hfEdit = connect(story, &QTextDocument::contentsChange, this, [this](int position, int, int added) { m_editEnd = position + added; });
    m_cursor = QTextCursor(story);
    m_cursor.setPosition(noteTextStart(story));
    m_wantedX = -1;
    m_movedFrom = -1;
    m_dragging = false;
    m_editEnd = -1;
    viewport()->update();
    Q_EMIT noteModeChanged();
    m_notePinned = true;   // (the caret is at the start of the note's text, which is not on the page that was clicked, if the note goes on from page to page)
    caretMoved();
    m_notePinned = false;
    return true;
}

bool PageView::enterFootnote(int id)
{
    if (!m_doc) return false;
    QTextDocument *story = m_doc->editableFootnote(id);
    if (!story) return false;
    const int page = pageShowingNote(story);
    return page >= 0 && enterNote(story, id, page);
}

void PageView::leaveNote(bool atReference)
{
    if (!m_note.active) return;
    QObject::disconnect(m_hfEdit);
    const int id = m_note.id, page = m_note.page;
    m_note = NoteEdit();
    m_hfSpell.reset();
    m_dragging = false;
    m_editEnd = -1;
    restoreBodyCursor();
    if (atReference && m_doc) {
        // Just after the reference (the one on the page the note was edited on, else the first).
        const QVector<int> refs = m_doc->footnoteReferences(id);
        int at = refs.isEmpty() ? -1 : refs.first();
        for (const int ref : refs)
            if (m_doc->pager().pageOfPosition(ref) == page) {
                at = ref;
                break;
            }
        if (at >= 0) m_cursor.setPosition(std::min(at + 1, m_doc->story()->characterCount() - 1));
    }
    viewport()->update();
    Q_EMIT noteModeChanged();
    m_movedFrom = -1;   // (a place in the note is no place of the text's)
    caretMoved();
}

QVector<QVector<PageView::NotePart>> PageView::copiesOfEditedNote()
{
    QVector<NotePart> all;
    if (!m_note.active || !m_doc) return {};
    flow::Paginator &pg = m_doc->pager();
    const QVector<int> refs = m_doc->footnoteReferences(m_note.id);
    auto partsOn = [&](int page) {
        QVector<NotePart> found;
        const auto r = rendered(page);
        if (r && r->page)
            for (const auto &nb : r->page->noteBoxes)
                if (nb.source == m_note.story) found.push_back({page, nb.from, nb.number});
        return found;
    };
    // (Each citation: the page that has its note's first part is its own page or one of the next two; the rest of the note is on the pages that follow, one after another.)
    QSet<int> done;
    for (int k = 0; k < refs.size() && k < 8; ++k) {
        const int home = pg.pageOfPosition(std::clamp(refs[k], 0, std::max(0, m_doc->story()->characterCount() - 1)));
        int start = -1;
        for (int page = home; page <= home + 2 && start < 0; ++page)
            if (done.contains(page) || !partsOn(page).isEmpty()) start = page;
        if (start < 0) continue;
        for (int page = start; page < start + 64 && !done.contains(page); ++page) {
            const QVector<NotePart> found = partsOn(page);
            if (found.isEmpty()) break;
            done.insert(page);
            all += found;
        }
    }
    // The copies: the boxes of one number, in the order of the pages; one that begins no later in the note than the box before it, or that is not on the page after it, is another copy that has
    // the number (the numbers begin again on each page when the settings say so).
    QMap<int, QVector<NotePart>> byNumber;
    for (const NotePart &part : std::as_const(all)) byNumber[part.number].push_back(part);
    QVector<QVector<NotePart>> copies;
    for (auto it = byNumber.begin(); it != byNumber.end(); ++it) {
        QVector<NotePart> &parts = it.value();
        std::sort(parts.begin(), parts.end(), [](const NotePart &a, const NotePart &b) { return a.page != b.page ? a.page < b.page : a.from < b.from; });
        for (const NotePart &part : std::as_const(parts)) {
            if (copies.isEmpty() || copies.last().first().number != part.number || part.from <= copies.last().last().from || part.page - copies.last().last().page > 1) copies.push_back({});
            copies.last().push_back(part);
        }
    }
    std::sort(copies.begin(), copies.end(), [](const QVector<NotePart> &a, const QVector<NotePart> &b) { return a.first().page != b.first().page ? a.first().page < b.first().page : a.first().from < b.first().from; });
    return copies;
}

int PageView::editedCopyIndex(const QVector<QVector<NotePart>> &copies, int hintPage) const
{
    if (copies.isEmpty()) return -1;
    if (m_note.copy >= 0 && m_note.copy < copies.size())
        for (const NotePart &part : copies[m_note.copy])
            if (part.page == hintPage) return m_note.copy;
    int best = 0, bestDistance = std::numeric_limits<int>::max();
    for (int i = 0; i < copies.size(); ++i) {
        int distance = std::numeric_limits<int>::max();
        for (const NotePart &part : copies[i]) distance = std::min(distance, std::abs(part.page - hintPage));
        if (distance < bestDistance) {
            bestDistance = distance;
            best = i;
        }
    }
    return best;
}

QVector<PageView::NotePart> PageView::partsOfEditedNote(int hintPage)
{
    const QVector<QVector<NotePart>> copies = copiesOfEditedNote();
    const int index = editedCopyIndex(copies, hintPage);
    return index < 0 ? QVector<NotePart>() : copies[index];
}

void PageView::chooseCopy(int page, int number)
{
    const QVector<QVector<NotePart>> copies = copiesOfEditedNote();
    for (int i = 0; i < copies.size(); ++i)
        for (const NotePart &part : copies[i])
            if (part.page == page && (number == 0 || part.number == number)) {
                m_note.copy = i;
                m_note.number = part.number;
                return;
            }
}

int PageView::partIndexAt(const QVector<NotePart> &parts, int position)
{
    int at = 0;
    for (int i = 0; i < parts.size(); ++i)
        if (parts[i].from <= position) at = i;
    return at;
}

int PageView::pageOfNotePosition(int position, int *number, int *copy)
{
    const QVector<QVector<NotePart>> copies = copiesOfEditedNote();
    const int index = editedCopyIndex(copies, m_note.page);
    if (index < 0) return -1;
    const NotePart &part = copies[index][partIndexAt(copies[index], position)];
    if (number) *number = part.number;
    if (copy) *copy = index;
    return part.page;
}

void PageView::syncNotePage()
{
    if (!m_note.active || m_notePinned || m_cursor.document() != m_note.story) return;
    int number = 0, copy = 0;
    const int page = pageOfNotePosition(m_cursor.position(), &number, &copy);
    if (page < 0) return;
    m_note.page = page;
    m_note.number = number;
    m_note.copy = copy;
}

void PageView::settleNote()
{
    if (!m_note.active || !m_doc) return;
    int number = 0, copy = 0;
    const int page = pageOfNotePosition(m_cursor.document() == m_note.story ? m_cursor.position() : 0, &number, &copy);
    if (page < 0) {
        leaveNote();
    } else {
        m_note.page = page;
        m_note.number = number;
        m_note.copy = copy;
    }
}

bool PageView::noteLineAtEdge(int dir)
{
    if (!m_note.active) return false;
    const auto r = rendered(m_note.page);
    FrameSpot spot;
    if (!r || !r->page || !hfSpotAt(*r, m_cursor.position(), &spot)) return true;
    if (!onEdgeLine(spot, dir)) return false;
    // (The first line of a part that goes on from the page before, or the last of one that goes on to the next page, is not the note's edge.)
    const QVector<NotePart> parts = partsOfEditedNote(m_note.page);
    const int at = partIndexAt(parts, m_cursor.position());
    return dir < 0 ? at <= 0 : at + 1 >= parts.size();
}

int PageView::cursorSection()
{
    if (!m_doc) return 0;
    if (m_hf.active) return m_hf.section;
    if (m_note.active) {
        flow::Paginator &pg = m_doc->pager();
        pg.paginate(m_note.page);
        return pg.pagesKnown() > 0 ? pg.page(std::clamp(m_note.page, 0, pg.pagesKnown() - 1)).start.section : 0;
    }
    return m_doc->sectionAt(m_cursor.position());
}

} // namespace jp::write
