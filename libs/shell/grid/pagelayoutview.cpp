#include "grid/pagelayoutview.h"

#include "app/icons.h"

#include <QCoreApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

namespace jp::grid {

namespace {

constexpr double kMinZoom = 0.1, kMaxZoom = 4.0;
// The most pages laid out (a sheet is cut into 50,000 pages at most; the view holds the sizes of that many).
constexpr int kMaxLaidOut = 50000;

} // namespace

PageLayoutView::PageLayoutView(QWidget *parent) : QAbstractScrollArea(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    viewport()->setMouseTracking(true);
    viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
    m_renderTimer.setSingleShot(true);
    m_renderTimer.setInterval(0);
    connect(&m_renderTimer, &QTimer::timeout, this, &PageLayoutView::renderStep);
    horizontalScrollBar()->setSingleStep(24);
    verticalScrollBar()->setSingleStep(40);
}

PageLayoutView::~PageLayoutView() = default;

void PageLayoutView::setSource(PageLayoutSource *source)
{
    m_source = source;
    sourceChanged();
}

void PageLayoutView::sourceChanged()
{
    m_stale.clear();
    m_cache.clear();
    m_order.clear();
    m_cachedPixels = 0;
    layoutPages();
    viewport()->update();
    scheduleDraw();
}

void PageLayoutView::contentsChanged()
{
    // (what is drawn stays on the screen until the page is drawn again, a page a turn)
    for (auto it = m_cache.constBegin(); it != m_cache.constEnd(); ++it) m_stale.insert(it.key());
    viewport()->update();
    scheduleDraw();
}

void PageLayoutView::setZoom(double zoom)
{
    zoom = std::clamp(zoom, kMinZoom, kMaxZoom);
    if (std::abs(zoom - m_zoom) < 1e-9) return;
    // (the page at the top of the view stays at the top)
    const int top = firstVisiblePage();
    const double into = top >= 0 && m_sizes.size() > size_t(top) ? double(verticalScrollBar()->value() - pageTop(top)) / std::max(1, pageHeightPx(top)) : 0;
    m_zoom = zoom;
    m_stale.clear();
    m_cache.clear();
    m_order.clear();
    m_cachedPixels = 0;
    layoutPages();
    if (top >= 0 && size_t(top) < m_sizes.size()) verticalScrollBar()->setValue(int(std::min<qint64>(pageTop(top) + qint64(into * pageHeightPx(top)), std::numeric_limits<int>::max())));
    viewport()->update();
    scheduleDraw();
    Q_EMIT zoomChanged(m_zoom);
}

void PageLayoutView::setMetric(bool metric)
{
    m_metric = metric;
    viewport()->update();
}

void PageLayoutView::layoutPages()
{
    m_tops.clear();
    m_sizes.clear();
    m_total = 0;
    m_widest = 0;
    if (m_source) {
        const int count = std::min(m_source->pageCount(), kMaxLaidOut);
        m_tops.reserve(size_t(count));
        m_sizes.reserve(size_t(count));
        qint64 y = kMargin + kRuler;
        for (int i = 0; i < count; ++i) {
            const QSizeF s = m_source->pageSize(i);
            const QSize px(std::max(1, int(std::lround(s.width() * pixelsPerPoint()))), std::max(1, int(std::lround(s.height() * pixelsPerPoint()))));
            m_tops.push_back(y);
            m_sizes.push_back(px);
            y += px.height() + kGap;
            m_widest = std::max(m_widest, px.width());
        }
        m_total = y - kGap + kMargin;
    }
    updateBars();
}

void PageLayoutView::updateBars()
{
    const QSize vp = viewport()->size();
    verticalScrollBar()->setPageStep(std::max(1, vp.height()));
    verticalScrollBar()->setRange(0, int(std::clamp<qint64>(m_total - vp.height(), 0, std::numeric_limits<int>::max())));
    const int content = m_widest + 2 * kMargin + kRuler;
    horizontalScrollBar()->setPageStep(std::max(1, vp.width()));
    horizontalScrollBar()->setRange(0, std::max(0, content - vp.width()));
}

void PageLayoutView::resizeEvent(QResizeEvent *e)
{
    QAbstractScrollArea::resizeEvent(e);
    updateBars();
    scheduleDraw();
}

qint64 PageLayoutView::pageTop(int page) const { return page >= 0 && size_t(page) < m_tops.size() ? m_tops[size_t(page)] : 0; }
int PageLayoutView::pageWidthPx(int page) const { return page >= 0 && size_t(page) < m_sizes.size() ? m_sizes[size_t(page)].width() : 0; }
int PageLayoutView::pageHeightPx(int page) const { return page >= 0 && size_t(page) < m_sizes.size() ? m_sizes[size_t(page)].height() : 0; }

int PageLayoutView::pageLeft(int page) const
{
    // (centered in the view when it is wider than the pages, else from the left margin, scrolled)
    const int room = viewport()->width() - kRuler;
    const int content = m_widest + 2 * kMargin;
    const int base = content <= room ? kRuler + (room - pageWidthPx(page)) / 2 : kRuler + kMargin + (m_widest - pageWidthPx(page)) / 2 - horizontalScrollBar()->value();
    return base;
}

QRect PageLayoutView::pageRect(int page) const
{
    if (page < 0 || size_t(page) >= m_sizes.size()) return QRect();
    const qint64 top = m_tops[size_t(page)] - verticalScrollBar()->value();
    if (top > viewport()->height() || top + m_sizes[size_t(page)].height() < 0) return QRect();
    return QRect(pageLeft(page), int(top), m_sizes[size_t(page)].width(), m_sizes[size_t(page)].height());
}

int PageLayoutView::firstVisiblePage() const
{
    if (m_tops.empty()) return -1;
    const qint64 y = verticalScrollBar()->value();
    auto it = std::upper_bound(m_tops.begin(), m_tops.end(), y);
    int i = int(it - m_tops.begin()) - 1;
    if (i < 0) i = 0;
    // (the page above it, if some of it is still in view)
    if (i > 0 && m_tops[size_t(i - 1)] + m_sizes[size_t(i - 1)].height() > y) --i;
    else if (m_tops[size_t(i)] + m_sizes[size_t(i)].height() <= y && size_t(i + 1) < m_tops.size()) ++i;
    return i;
}

int PageLayoutView::lastVisiblePage() const
{
    if (m_tops.empty()) return -1;
    const qint64 bottom = qint64(verticalScrollBar()->value()) + viewport()->height();
    auto it = std::upper_bound(m_tops.begin(), m_tops.end(), bottom);
    return std::max(0, int(it - m_tops.begin()) - 1);
}

int PageLayoutView::pageAt(const QPoint &p) const
{
    if (m_tops.empty()) return -1;
    const qint64 y = qint64(p.y()) + verticalScrollBar()->value();
    auto it = std::upper_bound(m_tops.begin(), m_tops.end(), y);
    const int i = int(it - m_tops.begin()) - 1;
    if (i < 0) return -1;
    return y < m_tops[size_t(i)] + m_sizes[size_t(i)].height() + kGap ? i : -1;
}

QPointF PageLayoutView::toPagePoint(int page, const QPoint &p) const
{
    const QRect r = pageRect(page);
    return QPointF((p.x() - r.left()) / pixelsPerPoint(), (p.y() - r.top()) / pixelsPerPoint());
}

QRect PageLayoutView::zoneRect(int page, bool header, int zone) const
{
    const QRect r = pageRect(page);
    if (r.isEmpty() || !m_source || zone < 0 || zone > 2) return QRect();
    const QRectF band = header ? m_source->headerBand(page) : m_source->footerBand(page);
    const double w = band.width() / 3;
    const double k = pixelsPerPoint();
    return QRect(r.left() + int(std::lround((band.left() + zone * w) * k)), r.top() + int(std::lround(band.top() * k)), int(std::lround(w * k)), int(std::lround(band.height() * k)));
}

QRect PageLayoutView::cellRect(qint64 row, qint64 col) const
{
    if (!m_source) return QRect();
    const int page = m_source->pageOfCell(row, col);
    if (page < 0) return QRect();
    const QRect pr = pageRect(page);
    QRectF c;
    if (pr.isEmpty() || !m_source->cellRect(page, row, col, &c)) return QRect();
    const double k = pixelsPerPoint();
    return QRect(pr.left() + int(std::lround(c.left() * k)), pr.top() + int(std::lround(c.top() * k)), std::max(1, int(std::lround(c.width() * k))), std::max(1, int(std::lround(c.height() * k))));
}

void PageLayoutView::setSelection(const GridRange &range, qint64 activeRow, qint64 activeCol)
{
    m_selection = range;
    m_activeRow = activeRow;
    m_activeCol = activeCol;
    viewport()->update();
}

bool PageLayoutView::ensureCellVisible(qint64 row, qint64 col)
{
    if (!m_source) return false;
    const int page = m_source->pageOfCell(row, col);
    if (page < 0 || size_t(page) >= m_tops.size()) return false;
    QRectF c;
    if (!m_source->cellRect(page, row, col, &c)) return false;
    const double k = pixelsPerPoint();
    const qint64 top = m_tops[size_t(page)] + qint64(c.top() * k), bottom = m_tops[size_t(page)] + qint64(std::ceil(c.bottom() * k));
    QScrollBar *v = verticalScrollBar();
    const int was = v->value();
    if (top < v->value()) v->setValue(int(std::max<qint64>(0, top - kMargin)));
    else if (bottom > qint64(v->value()) + viewport()->height()) v->setValue(int(std::min<qint64>(bottom - viewport()->height() + kMargin, std::numeric_limits<int>::max())));
    QScrollBar *h = horizontalScrollBar();
    const int hwas = h->value();
    const QRect now = cellRect(row, col);
    if (!now.isEmpty()) {
        if (now.left() < kRuler) h->setValue(h->value() - (kRuler - now.left()) - kMargin);
        else if (now.right() > viewport()->width()) h->setValue(h->value() + now.right() - viewport()->width() + kMargin);
    }
    return v->value() != was || h->value() != hwas;
}

void PageLayoutView::scrollToPage(int page)
{
    if (page >= 0 && size_t(page) < m_tops.size()) verticalScrollBar()->setValue(int(std::max<qint64>(0, m_tops[size_t(page)] - kMargin - kRuler)));
}

// ---- drawing

void PageLayoutView::drawPage(int page)
{
    if (!m_source || page < 0 || size_t(page) >= m_sizes.size() || (m_cache.contains(page) && !m_stale.contains(page))) return;
    if (m_cache.contains(page)) {
        // (drawn again: the old one goes)
        const QPixmap old = m_cache.take(page);
        m_cachedPixels -= qint64(old.width()) * old.height();
        m_order.remove(page);
        m_stale.remove(page);
    }
    const QSize px = m_sizes[size_t(page)];
    const qreal dpr = devicePixelRatioF();
    const qint64 pixels = qint64(std::ceil(px.width() * dpr)) * qint64(std::ceil(px.height() * dpr));
    if (pixels > kCacheBudgetPixels) return;   // (too large to keep: painted on the screen each time)
    QPixmap pm(int(std::ceil(px.width() * dpr)), int(std::ceil(px.height() * dpr)));
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::white);
    {
        QPainter p(&pm);
        p.setRenderHint(QPainter::TextAntialiasing, true);
        p.setRenderHint(QPainter::SmoothPixmapTransform, true);
        p.scale(pixelsPerPoint(), pixelsPerPoint());
        m_source->paintPage(&p, page);
    }
    ++m_drawn;
    m_cache.insert(page, pm);
    m_order.push_front(page);
    m_cachedPixels += pixels;
    trim();
}

// The pages kept are those used last, within the budget (the page just drawn stays).
void PageLayoutView::trim()
{
    while (m_cachedPixels > kCacheBudgetPixels && m_order.size() > 1) {
        const int old = m_order.back();
        m_order.pop_back();
        const QPixmap pm = m_cache.take(old);
        m_stale.remove(old);
        m_cachedPixels -= qint64(pm.width()) * pm.height();
    }
}

void PageLayoutView::scheduleDraw()
{
    if (m_source && !m_renderTimer.isActive()) m_renderTimer.start();
}

void PageLayoutView::renderStep()
{
    if (!m_source) return;
    const int first = firstVisiblePage(), last = lastVisiblePage();
    for (int i = first; i >= 0 && i <= last; ++i) {
        if ((m_cache.contains(i) && !m_stale.contains(i)) || pageRect(i).isEmpty()) continue;
        drawPage(i);
        viewport()->update(pageRect(i));
        // (one page a turn)
        scheduleDraw();
        return;
    }
}

void PageLayoutView::drawPagesNow()
{
    const int first = firstVisiblePage(), last = lastVisiblePage();
    for (int i = first; i >= 0 && i <= last; ++i) drawPage(i);
    viewport()->update();
}

void PageLayoutView::paintEvent(QPaintEvent *e)
{
    QPainter p(viewport());
    const bool dark = uiDark();
    p.fillRect(e->rect(), dark ? QColor(0x26, 0x28, 0x2C) : QColor(0xC9, 0xCD, 0xD3));
    if (!m_source) return;
    const int first = firstVisiblePage(), last = lastVisiblePage();
    bool missing = false;
    for (int i = first; i >= 0 && i <= last; ++i) {
        const QRect r = pageRect(i);
        if (r.isEmpty() || !r.adjusted(0, 0, 4, 4).intersects(e->rect())) continue;
        p.fillRect(r.translated(3, 3), QColor(0, 0, 0, dark ? 90 : 50));
        const auto have = m_cache.constFind(i);
        if (have != m_cache.constEnd()) {
            p.drawPixmap(r.topLeft(), *have);
            m_order.remove(i);
            m_order.push_front(i);
            if (m_stale.contains(i)) missing = true;
        } else {
            p.fillRect(r, Qt::white);
            missing = true;
            const qint64 pixels = qint64(std::ceil(r.width() * devicePixelRatioF())) * qint64(std::ceil(r.height() * devicePixelRatioF()));
            if (pixels > kCacheBudgetPixels && m_source) {
                // (a page too large to keep is drawn here, clipped to what shows of it)
                p.save();
                p.setClipRect(r & viewport()->rect());
                p.translate(r.topLeft());
                p.scale(pixelsPerPoint(), pixelsPerPoint());
                m_source->paintPage(&p, i);
                p.restore();
            }
        }
        p.setPen(QColor(0, 0, 0, 70));
        p.drawRect(r.adjusted(0, 0, -1, -1));
        // the selection over the cells it is on
        const QColor accent = dark ? QColor(0x7F, 0xA8, 0xFF) : QColor(0x1F, 0x5F, 0xBF);
        QRectF sel;
        if (m_selection.row2 >= m_selection.row1 && m_source->rangeRect(i, m_selection, &sel)) {
            const double k = pixelsPerPoint();
            const QRect sr(r.left() + int(std::lround(sel.left() * k)), r.top() + int(std::lround(sel.top() * k)), int(std::lround(sel.width() * k)), int(std::lround(sel.height() * k)));
            QColor fill = accent;
            fill.setAlpha(40);
            p.fillRect(sr, fill);
            p.setPen(QPen(accent, 2));
            p.drawRect(sr.adjusted(0, 0, -1, -1));
        }
        QRectF act;
        if (m_activeRow >= 0 && m_source->cellRect(i, m_activeRow, m_activeCol, &act)) {
            const double k = pixelsPerPoint();
            const QRect ar(r.left() + int(std::lround(act.left() * k)), r.top() + int(std::lround(act.top() * k)), int(std::lround(act.width() * k)), int(std::lround(act.height() * k)));
            p.setPen(QPen(accent, 2));
            p.setBrush(Qt::NoBrush);
            p.drawRect(ar.adjusted(0, 0, -1, -1));
        }
        // the parts of the header and footer under the mouse
        if (i == m_hoverPage) {
            for (const bool header : {true, false}) {
                if (header != m_hoverHeader) continue;
                for (int z = 0; z < 3; ++z) {
                    const QRect zr = zoneRect(i, header, z);
                    QColor c = accent;
                    c.setAlpha(z == m_hoverZone ? 70 : 28);
                    p.fillRect(zr, c);
                    p.setPen(QPen(accent, 1, Qt::DotLine));
                    p.drawRect(zr.adjusted(0, 0, -1, -1));
                }
            }
        }
    }
    paintRulers(&p);
    if (missing) scheduleDraw();
}

// The rulers: along the top and down the left, counting from the corner of the page in view (the page at the middle of the view), in inches or centimeters.
void PageLayoutView::paintRulers(QPainter *p)
{
    const bool dark = uiDark();
    const QColor bg = dark ? QColor(0x3A, 0x3D, 0x42) : QColor(0xEE, 0xEF, 0xF1), ink = dark ? QColor(0xD0, 0xD3, 0xD8) : QColor(0x40, 0x44, 0x4C);
    const int mid = std::max(0, firstVisiblePage());
    int page = pageAt(QPoint(viewport()->width() / 2, viewport()->height() / 2));
    if (page < 0) page = mid;
    const QRect r = pageRect(page);
    p->fillRect(QRect(0, 0, viewport()->width(), kRuler), bg);
    p->fillRect(QRect(0, 0, kRuler, viewport()->height()), bg);
    p->setPen(QColor(0, 0, 0, 60));
    p->drawLine(0, kRuler - 1, viewport()->width(), kRuler - 1);
    p->drawLine(kRuler - 1, 0, kRuler - 1, viewport()->height());
    if (r.width() <= 0 || !m_source || page < 0) return;
    const double k = pixelsPerPoint();
    const double unitPoints = m_metric ? 72.0 / 2.54 : 72.0;          // a centimeter or an inch
    const int subdivisions = m_metric ? 2 : 8;                          // halves of a centimeter, eighths of an inch
    p->setPen(ink);
    QFont f = p->font();
    f.setPixelSize(9);
    p->setFont(f);
    const QSizeF size = m_source->pageSize(page);
    const double step = unitPoints / subdivisions * k;
    const bool minor = step >= 3.5;
    auto ticks = [&](bool horizontal) {
        const double length = horizontal ? size.width() : size.height();
        const int count = int(length / (unitPoints / subdivisions));
        for (int i = 0; i <= count; ++i) {
            const double at = i * unitPoints / subdivisions * k;
            const bool whole = i % subdivisions == 0, half = i % (subdivisions / 2) == 0;
            if (!whole && !half && !minor) continue;
            const int len = whole ? 9 : half ? 6 : 3;
            if (horizontal) {
                const int x = r.left() + int(std::lround(at));
                if (x < kRuler || x > viewport()->width()) continue;
                p->drawLine(x, kRuler - 1, x, kRuler - 1 - len);
                if (whole && i > 0 && step * subdivisions > 14) p->drawText(QRect(x + 2, 0, 24, kRuler - 8), Qt::AlignLeft | Qt::AlignVCenter, QString::number(i / subdivisions));
            } else {
                const int y = r.top() + int(std::lround(at));
                if (y < kRuler || y > viewport()->height()) continue;
                p->drawLine(kRuler - 1, y, kRuler - 1 - len, y);
                if (whole && i > 0 && step * subdivisions > 14) p->drawText(QRect(0, y + 2, kRuler - 8, 12), Qt::AlignRight | Qt::AlignTop, QString::number(i / subdivisions));
            }
        }
    };
    p->save();
    p->setClipRect(QRect(kRuler, 0, viewport()->width() - kRuler, kRuler));
    ticks(true);
    p->restore();
    p->save();
    p->setClipRect(QRect(0, kRuler, kRuler, viewport()->height() - kRuler));
    ticks(false);
    p->restore();
    // the margins on the rulers: the stretch between them is the cells' room
    p->fillRect(QRect(kRuler, 0, 0, 0), bg);
}

// ---- the mouse and keys

bool PageLayoutView::zoneAt(const QPoint &pt, int *page, bool *header, int *zone) const
{
    const int pg = pageAt(pt);
    if (pg < 0 || !m_source || !pageRect(pg).contains(pt)) return false;
    for (const bool h : {true, false})
        for (int z = 0; z < 3; ++z)
            if (zoneRect(pg, h, z).contains(pt)) {
                *page = pg;
                *header = h;
                *zone = z;
                return true;
            }
    return false;
}

void PageLayoutView::mousePressEvent(QMouseEvent *e)
{
    setFocus(Qt::MouseFocusReason);
    if (e->button() != Qt::LeftButton || !m_source || e->pos().x() < kRuler || e->pos().y() < kRuler) return;
    int page = -1, zone = -1;
    bool header = false;
    if (zoneAt(e->pos(), &page, &header, &zone)) {
        Q_EMIT zoneClicked(page, header, zone);
        return;
    }
    page = pageAt(e->pos());
    if (page < 0) return;
    qint64 row = 0, col = 0;
    if (!m_source->cellAt(page, toPagePoint(page, e->pos()), &row, &col)) return;
    m_dragging = true;
    Q_EMIT cellPressed(page, row, col, e->modifiers());
}

void PageLayoutView::mouseMoveEvent(QMouseEvent *e)
{
    if (m_dragging && (e->buttons() & Qt::LeftButton) && m_source) {
        const int page = pageAt(e->pos());
        qint64 row = 0, col = 0;
        if (page >= 0 && m_source->cellAt(page, toPagePoint(page, e->pos()), &row, &col)) Q_EMIT cellDragged(page, row, col);
        return;
    }
    int page = -1, zone = -1;
    bool header = false;
    const bool on = e->pos().x() >= kRuler && e->pos().y() >= kRuler && zoneAt(e->pos(), &page, &header, &zone);
    const int hp = on ? page : -1, hz = on ? zone : -1;
    const bool hh = on && header;
    if (hp != m_hoverPage || hz != m_hoverZone || (on && hh != m_hoverHeader)) {
        m_hoverPage = hp;
        m_hoverZone = hz;
        m_hoverHeader = hh;
        viewport()->update();
    }
    viewport()->setCursor(on ? Qt::IBeamCursor : Qt::ArrowCursor);
}

void PageLayoutView::leaveEvent(QEvent *e)
{
    QAbstractScrollArea::leaveEvent(e);
    if (m_hoverPage >= 0) {
        m_hoverPage = -1;
        m_hoverZone = -1;
        viewport()->update();
    }
}

void PageLayoutView::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    if (m_dragging) {
        m_dragging = false;
        Q_EMIT cellReleased();
    }
}

void PageLayoutView::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton || !m_source) return;
    const int page = pageAt(e->pos());
    qint64 row = 0, col = 0;
    if (page >= 0 && m_source->cellAt(page, toPagePoint(page, e->pos()), &row, &col)) Q_EMIT cellDoubleClicked(page, row, col);
}

void PageLayoutView::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() & Qt::ControlModifier) {
        const int d = e->angleDelta().y();
        if (d != 0) setZoom(m_zoom * std::pow(1.1, d / 120.0));
        e->accept();
        return;
    }
    QAbstractScrollArea::wheelEvent(e);
}

void PageLayoutView::keyPressEvent(QKeyEvent *e)
{
    Q_EMIT keyPressed(e);
    if (!e->isAccepted()) QAbstractScrollArea::keyPressEvent(e);
}

} // namespace jp::grid
