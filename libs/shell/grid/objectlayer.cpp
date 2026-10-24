#include "grid/objectlayer.h"

#include <QElapsedTimer>
#include <QMetaObject>
#include <QPalette>

#include <algorithm>
#include <cmath>

namespace jp::grid {

using jp::office::sheet::AnchorKind;
using jp::office::sheet::AnchorPoint;
using jp::office::sheet::DrawingAnchor;
using jp::office::sheet::kEmuPerPixel;

GridOverlay::~GridOverlay() = default;

namespace {

constexpr Pane kPanes[] = {Pane::Corner, Pane::Top, Pane::Left, Pane::Body};
constexpr int kPicturesPerStep = 8;        // (a step also ends once this much time has passed)
constexpr int kStepMillis = 8;
constexpr int kNearDelayMillis = 30;      // the objects near the screen are made this long after the grid stops moving
constexpr int kStaleDelayMillis = 120;     // a picture whose cells changed is made again this long after (a calculation tells many changes)
constexpr qint64 kFarthest = qint64(1) << 29;   // a place on the screen is kept this near, so that a rectangle's arithmetic fits an int
constexpr double kBudgetShare = 0.6;       // the pictures of what is on the screen are made small enough to take this much of the budget
constexpr double kCell = 256;              // a square of the index is this many units on a side
constexpr qint64 kMostSquares = 64;        // an object that touches more squares than this is kept in a list of its own
constexpr qint64 kMostQueried = 1 << 16;   // an area that reaches more squares than this (a window zoomed far out) looks at every object instead

qint64 squareOf(double v)
{
    return qint64(std::floor(std::max(-1e12, std::min(1e12, v)) / kCell));
}
quint64 keyOf(qint64 sx, qint64 sy) { return (quint64(sx + (qint64(1) << 31)) << 32) | quint64(sy + (qint64(1) << 31)); }

// A place along an axis: the start of the row or column, and how far into it (EMU), no further than it is high or wide.
double placeOf(const Axis &axis, quint32 index, qint64 offsetEmu)
{
    if (qint64(index) >= axis.count()) return double(axis.total()) + double(offsetEmu) / double(kEmuPerPixel);
    const double into = std::min(double(offsetEmu) / double(kEmuPerPixel), double(axis.size(qint64(index))));
    return double(axis.position(qint64(index))) + into;
}

int clampPlace(qint64 v) { return int(std::max(-kFarthest, std::min(kFarthest, v))); }

void pointIn(const Axis &axis, double at, quint32 *index, qint64 *offsetEmu)
{
    const double total = double(axis.total());
    if (axis.count() <= 0) {
        *index = 0;
        *offsetEmu = 0;
        return;
    }
    if (at >= total) {
        const qint64 last = axis.previousVisible(axis.count());
        *index = quint32(std::max<qint64>(0, last));
        *offsetEmu = qint64(double(axis.size(qint64(*index))) * double(kEmuPerPixel));
        return;
    }
    qint64 i = axis.indexAt(qint64(std::floor(std::max(0.0, at))));
    if (i < 0) i = std::max<qint64>(0, axis.nextVisible(-1));
    *index = quint32(i);
    *offsetEmu = qint64(std::max(0.0, std::min(at - double(axis.position(i)), double(axis.size(i)))) * double(kEmuPerPixel));
}

} // namespace

ObjectBox anchorBox(const DrawingAnchor &a, const Axis &rows, const Axis &cols)
{
    ObjectBox b;
    if (a.kind == AnchorKind::Absolute) {
        b.x = double(a.x) / double(kEmuPerPixel);
        b.y = double(a.y) / double(kEmuPerPixel);
        b.w = double(a.cx) / double(kEmuPerPixel);
        b.h = double(a.cy) / double(kEmuPerPixel);
        return b;
    }
    b.x = placeOf(cols, a.from.col, a.from.colOff);
    b.y = placeOf(rows, a.from.row, a.from.rowOff);
    if (a.kind == AnchorKind::OneCell) {
        b.w = double(a.cx) / double(kEmuPerPixel);
        b.h = double(a.cy) / double(kEmuPerPixel);
        return b;
    }
    b.w = std::max(0.0, placeOf(cols, a.to.col, a.to.colOff) - b.x);
    b.h = std::max(0.0, placeOf(rows, a.to.row, a.to.rowOff) - b.y);
    return b;
}

AnchorPoint anchorPointAt(const Axis &rows, const Axis &cols, double x, double y)
{
    AnchorPoint p;
    pointIn(cols, x, &p.col, &p.colOff);
    pointIn(rows, y, &p.row, &p.rowOff);
    return p;
}

ObjectLayer::ObjectLayer(QObject *parent) : QObject(parent)
{
    // (the charts are made one at a time: what they share (fonts, number formats) is then used by one thread besides the window's)
    m_pool.setMaxThreadCount(1);
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, [this] { step(); });
}

ObjectLayer::~ObjectLayer()
{
    m_timer.stop();
    m_pool.waitForDone();   // (the jobs post what they made here: none is left to)
}

void ObjectLayer::setObjects(std::vector<LayerObject> objects)
{
    m_timer.stop();
    ++m_generation;   // (a job still running for the objects there were is not heard)
    m_entries.clear();
    m_bytes = 0;
    m_reserved = 0;
    m_inflight = 0;
    m_selected = -1;
    m_boxesValid = false;
    m_indexValid = false;
    m_firstLive = -2;
    m_maxMargin = 0;
    m_shrink = 1.0;
    m_byId.clear();
    m_painting = false;
    const size_t n = std::min<size_t>(objects.size(), size_t(kMaxObjects));
    m_entries.reserve(n);
    m_byId.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        Entry e;
        e.object = std::move(objects[i]);
        m_maxMargin = std::max(m_maxMargin, e.object.margin);
        m_byId.emplace(e.object.id, i);
        m_entries.push_back(std::move(e));
    }
    m_seen.assign(n, 0);
    m_query = 0;
    m_nearKey.clear();
    requestRepaint();
}

void ObjectLayer::setFillsView(bool fills)
{
    if (m_fills == fills) return;
    m_fills = fills;
    requestRepaint();
}

void ObjectLayer::layoutChanged()
{
    m_boxesValid = false;
    requestRepaint();
}

size_t ObjectLayer::entryOf(int id) const
{
    const auto it = m_byId.find(id);
    return it == m_byId.end() ? m_entries.size() : it->second;
}

void ObjectLayer::setDeleted(int id, bool deleted)
{
    const size_t i = entryOf(id);
    if (i >= m_entries.size()) return;
    Entry &e = m_entries[i];
    if (e.deleted == deleted) return;
    e.deleted = deleted;
    m_firstLive = -2;
    if (deleted && m_selected == id) m_selected = -1;
    requestRepaint();
}

bool ObjectLayer::isDeleted(int id) const
{
    const size_t i = entryOf(id);
    return i < m_entries.size() && m_entries[i].deleted;
}

void ObjectLayer::setAnchor(int id, const DrawingAnchor &anchor)
{
    const size_t i = entryOf(id);
    if (i >= m_entries.size()) return;
    m_entries[i].object.anchor = anchor;
    m_boxesValid = false;
    requestRepaint();
}

ObjectBox ObjectLayer::lastBox(int id, bool *placed) const
{
    const size_t i = entryOf(id);
    if (i < m_entries.size()) {
        if (placed) *placed = m_entries[i].placed;
        return m_entries[i].box;
    }
    if (placed) *placed = false;
    return ObjectBox();
}

ObjectBox ObjectLayer::boxNow(int id, bool *ok) const
{
    if (ok) *ok = false;
    if (!m_layout) return ObjectBox();
    ensureBoxes(*m_layout);
    const size_t i = entryOf(id);
    if (i >= m_entries.size()) return ObjectBox();
    if (ok) *ok = true;
    return m_entries[i].box;
}

void ObjectLayer::invalidate(int id)
{
    const size_t i = entryOf(id);
    if (i >= m_entries.size()) return;
    m_entries[i].stale = true;
    m_entries[i].failed = false;
    schedule(kStaleDelayMillis);
}

void ObjectLayer::invalidateAll()
{
    for (Entry &e : m_entries) {
        e.stale = true;
        e.failed = false;
    }
    if (!m_entries.empty()) schedule(kStaleDelayMillis);
}

void ObjectLayer::select(int id)
{
    if (id == m_selected) return;
    m_selected = id;
    requestRepaint();
    Q_EMIT selectionChanged(id);
}

void ObjectLayer::ensureBoxes(const GridLayout &layout) const
{
    const std::vector<qint64> signature = {layout.rows.total(), layout.rows.placeCount(), layout.rows.defaultSize(), layout.rows.skipped(),
                                           layout.cols.total(), layout.cols.placeCount(), layout.cols.defaultSize(), layout.cols.skipped()};
    if (m_boxesValid && signature == m_signature) return;
    m_signature = signature;
    m_boxesValid = true;
    for (const Entry &e : m_entries) {
        e.box = anchorBox(e.object.anchor, layout.rows, layout.cols);
        e.placed = true;
    }
    m_indexValid = false;
    m_stats.placed += qint64(m_entries.size());
}

int ObjectLayer::firstLive() const
{
    if (m_firstLive == -2) {
        m_firstLive = -1;
        for (size_t i = 0; i < m_entries.size(); ++i)
            if (!m_entries[i].deleted) {
                m_firstLive = int(i);
                break;
            }
    }
    return m_firstLive;
}

void ObjectLayer::buildIndex() const
{
    m_index.squares.clear();
    m_index.large.clear();
    for (size_t i = 0; i < m_entries.size(); ++i) {
        const ObjectBox &b = m_entries[i].box;
        const qint64 x0 = squareOf(b.x), x1 = squareOf(b.x + b.w), y0 = squareOf(b.y), y1 = squareOf(b.y + b.h);
        if ((x1 - x0 + 1) * (y1 - y0 + 1) > kMostSquares) {
            m_index.large.push_back(quint32(i));
            continue;
        }
        for (qint64 sx = x0; sx <= x1; ++sx)
            for (qint64 sy = y0; sy <= y1; ++sy) m_index.squares[keyOf(sx, sy)].push_back(quint32(i));
    }
    m_indexValid = true;
}

void ObjectLayer::candidates(const GridLayout &layout, Pane pane, const QRect &area, std::vector<size_t> *out) const
{
    out->clear();
    if (area.isEmpty() || m_entries.empty()) return;
    ensureBoxes(layout);
    if (m_fills) {
        // (the object that fills the view is the first that is not deleted)
        const int first = firstLive();
        if (first >= 0 && pane == Pane::Body) out->push_back(size_t(first));
        m_stats.examined += qint64(out->size());
        return;
    }
    if (!m_indexValid) buildIndex();
    // the area on the sheet (the grid's units): the screen's place less where the part's origin is, over the zoom; a margin around it for what an object draws past its box
    const bool fr = pane == Pane::Corner || pane == Pane::Top, fc = pane == Pane::Corner || pane == Pane::Left;
    const double zoom = std::max(1e-6, layout.zoom());
    const double ox = layout.screenXAt(0, fc), oy = layout.screenYAt(0, fr);
    const double pad = m_maxMargin + 3;
    const double lx0 = (area.left() - ox) / zoom - pad, lx1 = (area.right() + 1 - ox) / zoom + pad;
    const double ly0 = (area.top() - oy) / zoom - pad, ly1 = (area.bottom() + 1 - oy) / zoom + pad;
    const qint64 x0 = squareOf(lx0), x1 = squareOf(lx1), y0 = squareOf(ly0), y1 = squareOf(ly1);
    if ((x1 - x0 + 1) * (y1 - y0 + 1) > kMostQueried) {
        // (a window that shows more of the sheet than the index can be asked about square by square: every object is looked at)
        out->resize(m_entries.size());
        for (size_t i = 0; i < m_entries.size(); ++i) (*out)[i] = i;
        m_stats.examined += qint64(out->size());
        return;
    }
    if (m_seen.size() != m_entries.size()) m_seen.assign(m_entries.size(), 0);
    if (++m_query == 0) {
        std::fill(m_seen.begin(), m_seen.end(), 0);
        m_query = 1;
    }
    auto take = [&](quint32 i) {
        if (m_seen[i] == m_query) return;
        m_seen[i] = m_query;
        out->push_back(i);
    };
    for (const quint32 i : m_index.large) take(i);
    for (qint64 sx = x0; sx <= x1; ++sx)
        for (qint64 sy = y0; sy <= y1; ++sy) {
            const auto it = m_index.squares.find(keyOf(sx, sy));
            if (it == m_index.squares.end()) continue;
            for (const quint32 i : it->second) take(i);
        }
    std::sort(out->begin(), out->end());
    m_stats.examined += qint64(out->size());
}

void ObjectLayer::candidatesAll(const GridLayout &layout, double widen, std::vector<size_t> *out) const
{
    out->clear();
    std::vector<size_t> part;
    for (const Pane pane : kPanes) {
        const QRect shown = layout.pane(pane).rect;
        if (shown.isEmpty()) continue;
        const QRect area = widen <= 0 ? shown : shown.adjusted(-int(shown.width() * widen), -int(shown.height() * widen), int(shown.width() * widen), int(shown.height() * widen));
        candidates(layout, pane, area, &part);
        out->insert(out->end(), part.begin(), part.end());
    }
    std::sort(out->begin(), out->end());
    out->erase(std::unique(out->begin(), out->end()), out->end());
}

QRect ObjectLayer::rectIn(const GridLayout &layout, Pane pane, const Entry &e) const
{
    if (e.deleted) return QRect();
    if (m_fills) {
        // the first object that is not deleted fills the part that scrolls
        const int first = firstLive();
        return first >= 0 && &m_entries[size_t(first)] == &e && pane == Pane::Body ? layout.pane(Pane::Body).rect : QRect();
    }
    if (e.object.thin ? e.box.isNothing() : e.box.isEmpty()) return QRect();
    const bool fr = pane == Pane::Corner || pane == Pane::Top, fc = pane == Pane::Corner || pane == Pane::Left;
    const qint64 x0 = std::llround(std::max(-1e12, std::min(1e12, e.box.x))), y0 = std::llround(std::max(-1e12, std::min(1e12, e.box.y)));
    const qint64 x1 = std::llround(std::max(-1e12, std::min(1e12, e.box.x + e.box.w))), y1 = std::llround(std::max(-1e12, std::min(1e12, e.box.y + e.box.h)));
    const int left = clampPlace(layout.screenXAt(x0, fc)), right = clampPlace(layout.screenXAt(x1, fc));
    const int top = clampPlace(layout.screenYAt(y0, fr)), bottom = clampPlace(layout.screenYAt(y1, fr));
    return QRect(left, top, std::max(1, right - left), std::max(1, bottom - top));
}

QRect ObjectLayer::drawRect(const QRect &box, const Entry &e, double zoom) const
{
    const int m = e.object.margin > 0 ? int(std::ceil(e.object.margin * zoom)) : 0;
    return m > 0 ? box.adjusted(-m, -m, m, m) : box;
}

RenderRequest ObjectLayer::requestFor(const Entry &e, const QRect &box, const QSize &wanted, double zoom) const
{
    const QRect whole = drawRect(box, e, zoom);
    RenderRequest q;
    q.pixels = wanted;
    q.zoom = zoom;
    q.units = m_fills ? QSizeF(double(box.width()) / zoom, double(box.height()) / zoom) : QSizeF(e.box.w, e.box.h);
    const double sx = double(wanted.width()) / double(whole.width()), sy = double(wanted.height()) / double(whole.height());
    q.inner = QRectF((box.x() - whole.x()) * sx, (box.y() - whole.y()) * sy, box.width() * sx, box.height() * sy);
    return q;
}

QSize ObjectLayer::sizeWanted(const QRect &rect, double dpr) const
{
    double w = std::ceil(double(rect.width()) * dpr * m_shrink), h = std::ceil(double(rect.height()) * dpr * m_shrink);
    const double pixels = w * h;
    if (pixels > double(kMaxImagePixels)) {
        const double k = std::sqrt(double(kMaxImagePixels) / pixels);
        w = std::floor(w * k);
        h = std::floor(h * k);
    }
    return QSize(int(std::max(1.0, w)), int(std::max(1.0, h)));
}

bool ObjectLayer::missing(const Entry &e, const QSize &wanted, double zoom) const
{
    // (the same object in two parts of the grid is a pixel different in size, by the rounding of the frozen edge)
    const bool sameSize = std::abs(e.requested.width() - wanted.width()) <= 2 && std::abs(e.requested.height() - wanted.height()) <= 2 && e.zoom == zoom;
    if (e.failed && sameSize) return false;
    if (e.image.isNull()) return true;
    return !sameSize;
}

void ObjectLayer::settle(Entry &e)
{
    if (e.settled) return;
    e.settled = true;
    if (e.object.settle) e.object.settle(&e.object);
    m_maxMargin = std::max(m_maxMargin, e.object.margin);
}

void ObjectLayer::store(Entry &e, const QImage &image, const QSize &wanted, double zoom)
{
    m_bytes -= e.image.isNull() ? 0 : e.image.sizeInBytes();
    e.image = image;
    e.requested = wanted;
    e.zoom = zoom;
    e.stale = false;
    e.failed = image.isNull();
    m_bytes += e.image.isNull() ? 0 : e.image.sizeInBytes();
    e.lastUsed = m_frame;
    ++m_stats.renders;
    if (m_painting) ++m_stats.paintRenders;
}

void ObjectLayer::evict()
{
    // the limit is hard: the least recently shown pictures go until what is held, and what is being made, is within it
    if (m_bytes + m_reserved <= kCacheBytes) return;
    std::vector<Entry *> held;
    for (Entry &e : m_entries)
        if (!e.image.isNull()) held.push_back(&e);
    std::sort(held.begin(), held.end(), [](const Entry *a, const Entry *b) { return a->lastUsed < b->lastUsed; });
    for (Entry *e : held) {
        if (m_bytes + m_reserved <= kCacheBytes) break;
        m_bytes -= e->image.sizeInBytes();
        e->image = QImage();
        ++m_stats.evicted;
    }
}

void ObjectLayer::schedule(int millis)
{
    if (!m_timer.isActive() || m_timer.remainingTime() > millis) m_timer.start(millis);
}

QRect ObjectLayer::visibleRect(const Entry &e) const
{
    QRect all;
    if (!m_layout) return all;
    for (const Pane pane : kPanes) {
        const QRect shown = m_layout->pane(pane).rect;
        const QRect box = rectIn(*m_layout, pane, e);
        if (box.isEmpty() || shown.isEmpty()) continue;
        all = all.united(drawRect(box, e, m_layout->zoom()).intersected(shown));
    }
    return all;
}

void ObjectLayer::paint(QPainter *p, const GridLayout &layout, Pane pane, const QRect &clip)
{
    m_layout = &layout;
    if (m_entries.empty() || clip.isEmpty()) return;
    m_dpr = std::max(1.0, p->device()->devicePixelRatioF());
    ensureBoxes(layout);
    ++m_frame;
    m_painting = true;   // (a picture made while this is set is counted: a paint makes none)
    const double zoom = layout.zoom();
    bool later = false;
    p->save();
    p->setClipRect(clip);
    std::vector<size_t> near;
    candidates(layout, pane, clip, &near);
    for (const size_t index : near) {
        Entry &e = m_entries[index];
        const QRect box = rectIn(layout, pane, e);
        if (box.isEmpty()) continue;
        const QRect r = drawRect(box, e, zoom);
        if (!r.intersects(clip)) continue;
        // (a paint makes nothing: what it lacks is asked for, and made between events)
        if (!e.pending && (e.stale || missing(e, sizeWanted(r, m_dpr), zoom))) later = true;
        e.lastUsed = m_frame;
        const QRect target = r.intersected(clip);
        if (m_fills) p->fillRect(target, Qt::white);   // (a chart sheet is a white page under what the chart does not paint)
        if (!e.image.isNull()) {
            // the part of the picture under the clip, so that an object a million pixels wide costs the clip
            const double sx = double(e.image.width()) / double(r.width()), sy = double(e.image.height()) / double(r.height());
            const QRectF source((target.x() - r.x()) * sx, (target.y() - r.y()) * sy, target.width() * sx, target.height() * sy);
            const bool scaled = std::abs(sx - m_dpr) > 1e-3 || std::abs(sy - m_dpr) > 1e-3;
            p->setRenderHint(QPainter::SmoothPixmapTransform, scaled);
            p->drawImage(QRectF(target), e.image, source);
            ++m_stats.drawn;
        } else if (!e.failed || e.object.render || e.object.prepare) {
            // no picture yet: an outline in the object's place
            p->setPen(QPen(QColor(0x80, 0x80, 0x80), 1, Qt::DashLine));
            p->setBrush(Qt::NoBrush);
            p->drawRect(box.adjusted(0, 0, -1, -1));
            ++m_stats.placeholders;
        }
    }
    // the object that is picked
    if (const size_t at = m_selected >= 0 ? entryOf(m_selected) : m_entries.size(); at < m_entries.size()) {
        const QRect r = rectIn(layout, pane, m_entries[at]);
        if (!r.isEmpty() && r.intersects(clip)) {
            const QColor accent(0x13, 0x77, 0x3A);
            p->setBrush(Qt::NoBrush);
            p->setPen(QPen(accent, 2));
            p->drawRect(r.adjusted(1, 1, -2, -2));
            p->setPen(QPen(accent, 1));
            p->setBrush(Qt::white);
            const int h = 3;
            const QPoint corners[] = {r.topLeft(), r.topRight(), r.bottomLeft(), r.bottomRight(), QPoint(r.center().x(), r.top()), QPoint(r.center().x(), r.bottom()),
                                      QPoint(r.left(), r.center().y()), QPoint(r.right(), r.center().y())};
            for (const QPoint &c : corners) p->drawRect(QRect(c.x() - h, c.y() - h, 2 * h, 2 * h));
        }
    }
    p->restore();
    if (later) schedule(0);
    // the grid has moved since the objects near the screen were looked at: they are, a moment after it stops
    const std::vector<qint64> key = {layout.scrollX(), layout.scrollY(), qint64(layout.zoom() * 1000), layout.viewport().width(), layout.viewport().height(), layout.frozenRows(), layout.frozenCols()};
    if (key != m_nearKey) {
        m_nearKey = key;
        m_timer.start(kNearDelayMillis);
    }
    m_painting = false;
}

void ObjectLayer::updateShrink(const GridLayout &layout)
{
    // what is on the screen, made at full size: if it would not fit the budget, every picture is made smaller (a half, a quarter...) until it does
    const double zoom = layout.zoom();
    double bytes = 0;
    const double keep = m_shrink;
    m_shrink = 1.0;
    std::vector<size_t> onScreen;
    candidatesAll(layout, 0, &onScreen);
    for (const size_t index : onScreen) {
        const Entry &e = m_entries[index];
        bool seen = false;
        for (const Pane pane : kPanes) {
            const QRect shown = layout.pane(pane).rect;
            const QRect box = rectIn(layout, pane, e);
            if (box.isEmpty() || shown.isEmpty() || !drawRect(box, e, zoom).intersects(shown)) continue;
            if (!seen) {
                const QSize s = sizeWanted(drawRect(box, e, zoom), m_dpr);
                bytes += double(s.width()) * double(s.height()) * 4.0;
                seen = true;
            }
        }
    }
    double shrink = 1.0;
    while (bytes * shrink * shrink > kBudgetShare * double(kCacheBytes) && shrink > 1.0 / 64.0) shrink *= 0.5;
    m_shrink = shrink;
    if (m_shrink != keep && m_shrink < 1.0) ++m_stats.shrunk;
}

void ObjectLayer::startJob(Entry &e, size_t index, const QSize &wanted, double zoom, const RenderRequest &request)
{
    std::function<QImage()> job = e.object.prepare(request);   // (on the window's thread: what the picture needs is taken here)
    if (!job) {
        store(e, QImage(), wanted, zoom);
        return;
    }
    if (m_painting) ++m_stats.paintRenders;
    ++e.token;
    e.pending = true;
    e.stale = false;
    e.requested = wanted;
    e.zoom = zoom;
    e.reserved = qint64(wanted.width()) * wanted.height() * 4;
    m_reserved += e.reserved;
    ++m_inflight;
    ++m_stats.jobs;
    const quint64 generation = m_generation, token = e.token;
    m_pool.start([this, job = std::move(job), generation, index, token, wanted, zoom] {
        QImage image = job();
        {
            std::lock_guard<std::mutex> lock(m_resultsMutex);
            m_results.push_back(Result{generation, index, token, wanted, zoom, std::move(image)});
        }
        QMetaObject::invokeMethod(this, "deliver", Qt::QueuedConnection);
    });
}

void ObjectLayer::deliver()
{
    std::vector<Result> got;
    {
        std::lock_guard<std::mutex> lock(m_resultsMutex);
        got.swap(m_results);
    }
    for (Result &r : got) {
        if (r.generation != m_generation || r.index >= m_entries.size()) continue;   // (for objects there are no more)
        Entry &e = m_entries[r.index];
        --m_inflight;
        if (e.token != r.token || !e.pending) continue;
        m_reserved -= e.reserved;
        e.reserved = 0;
        e.pending = false;
        const bool wasStale = e.stale;
        store(e, r.image, r.requested, r.zoom);
        e.stale = wasStale;   // (its cells changed while it was being made: it is made again)
        if (wasStale) schedule(kStaleDelayMillis);
        evict();
        const QRect area = visibleRect(e);
        if (!area.isEmpty()) requestRepaint(area);
    }
    if (m_inflight < 0) m_inflight = 0;
    if (!got.empty()) schedule(0);   // (more may be waiting for a job to be free)
}

bool ObjectLayer::makePictures(bool all, bool near)
{
    if (!m_layout || m_entries.empty()) return false;
    const GridLayout &layout = *m_layout;
    ensureBoxes(layout);
    updateShrink(layout);
    ++m_frame;
    QElapsedTimer clock;
    clock.start();
    const double zoom = layout.zoom();
    int made = 0;
    bool left = false;
    // (the objects on the screen, and then those a screenful beyond it)
    for (int pass = 0; pass < (near ? 2 : 1); ++pass) {
        for (const Pane pane : kPanes) {
            const QRect shown = layout.pane(pane).rect;
            if (shown.isEmpty()) continue;
            const QRect area = pass == 0 ? shown : shown.adjusted(-shown.width(), -shown.height(), shown.width(), shown.height());
            std::vector<size_t> found;
            candidates(layout, pane, area, &found);
            for (const size_t i : found) {
                Entry &e = m_entries[i];
                QRect box = rectIn(layout, pane, e);
                if (box.isEmpty()) continue;
                QRect r = drawRect(box, e, zoom);
                if (!r.intersects(area)) continue;
                QSize wanted = sizeWanted(r, m_dpr);
                e.lastUsed = m_frame;
                if (e.pending || (!missing(e, wanted, zoom) && !e.stale)) continue;
                if (!e.settled) {
                    // (the object tells what it is when a picture of it is first wanted: its margin and whether it is a line, which are the box and the picture's size)
                    settle(e);
                    box = rectIn(layout, pane, e);
                    if (box.isEmpty()) continue;
                    r = drawRect(box, e, zoom);
                    wanted = sizeWanted(r, m_dpr);
                }
                if (!e.object.prepare && !e.object.render) {
                    store(e, QImage(), wanted, zoom);   // (nothing to draw)
                    continue;
                }
                if (e.object.prepare) {
                    if (m_inflight >= kMaxJobs || (!all && clock.elapsed() >= kStepMillis)) {
                        left = true;
                        continue;
                    }
                    startJob(e, i, wanted, zoom, requestFor(e, box, wanted, zoom));
                    continue;
                }
                if (!all && (made >= kPicturesPerStep || clock.elapsed() >= kStepMillis)) {
                    left = true;
                    continue;
                }
                store(e, e.object.render(requestFor(e, box, wanted, zoom)), wanted, zoom);
                ++made;
                const QRect now = r.intersected(shown);
                if (!now.isEmpty() && pass == 0) requestRepaint(now);
            }
        }
    }
    evict();
    return left;
}

void ObjectLayer::step()
{
    if (makePictures(false, true)) schedule(0);
}

void ObjectLayer::prepareAll()
{
    for (;;) {
        const bool left = makePictures(true, false);
        m_pool.waitForDone();
        deliver();
        if (!left) break;
    }
}

int ObjectLayer::pendingRenders() const
{
    if (!m_layout) return 0;
    const GridLayout &layout = *m_layout;
    ensureBoxes(layout);
    int pending = 0;
    std::vector<size_t> onScreen;
    candidatesAll(layout, 0, &onScreen);
    for (const size_t index : onScreen) {
        const Entry &e = m_entries[index];
        bool counted = false;
        for (const Pane pane : kPanes) {
            const QRect area = layout.pane(pane).rect;
            const QRect box = counted || area.isEmpty() ? QRect() : rectIn(layout, pane, e);
            if (box.isEmpty()) continue;
            const QRect r = drawRect(box, e, layout.zoom());
            if (!r.intersects(area)) continue;
            if (e.pending || e.stale || missing(e, sizeWanted(r, m_dpr), layout.zoom())) ++pending;
            counted = true;
        }
    }
    return pending;
}

QRect ObjectLayer::deviceRect(int id, Pane pane) const
{
    if (!m_layout) return QRect();
    ensureBoxes(*m_layout);
    const size_t i = entryOf(id);
    if (i >= m_entries.size()) return QRect();
    const QRect r = rectIn(*m_layout, pane, m_entries[i]);
    return r.intersects(m_layout->pane(pane).rect) ? r : QRect();
}

bool ObjectLayer::inkAt(const Entry &e, const GridLayout &layout, Pane pane, const QPoint &point) const
{
    // a place where the picture is not clear (a pixel or two either side of the point) has ink under the point; with no picture yet, the box counts
    if (e.image.isNull()) return !e.failed;
    const QRect box = rectIn(layout, pane, e);
    const QRect r = drawRect(box, e, layout.zoom());
    const double sx = double(e.image.width()) / double(r.width()), sy = double(e.image.height()) / double(r.height());
    const int cx = int((point.x() - r.x()) * sx), cy = int((point.y() - r.y()) * sy);
    const int reach = std::max(2, int(std::ceil(2 * std::max(sx, sy))));
    for (int y = std::max(0, cy - reach); y <= std::min(e.image.height() - 1, cy + reach); ++y)
        for (int x = std::max(0, cx - reach); x <= std::min(e.image.width() - 1, cx + reach); ++x)
            if (qAlpha(e.image.pixel(x, y)) > 24) return true;
    return false;
}

int ObjectLayer::objectAt(const QPoint &point) const
{
    if (!m_layout) return -1;
    const GridLayout &layout = *m_layout;
    ensureBoxes(layout);
    for (const Pane pane : kPanes) {
        if (!layout.pane(pane).rect.contains(point)) continue;
        std::vector<size_t> here;
        candidates(layout, pane, QRect(point, QSize(1, 1)), &here);
        for (size_t k = here.size(); k-- > 0;) {
            const Entry &e = m_entries[here[k]];
            if (!rectIn(layout, pane, e).contains(point)) continue;
            if (!e.object.hitInk || inkAt(e, layout, pane, point)) return e.object.id;
        }
        return -1;
    }
    return -1;
}

bool ObjectLayer::press(const QPoint &point, const GridLayout &layout, Qt::KeyboardModifiers)
{
    m_layout = &layout;
    const int hit = objectAt(point);
    if (hit >= 0) {
        select(hit);
        return true;
    }
    select(-1);
    return false;
}

bool ObjectLayer::key(QKeyEvent *event)
{
    if (m_selected < 0) return false;
    switch (event->key()) {
    case Qt::Key_Shift:
    case Qt::Key_Control:
    case Qt::Key_Alt:
    case Qt::Key_Meta:
    case Qt::Key_AltGr: return false;
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
        if (!m_deleteHook) break;
        m_deleteHook(m_selected);
        return true;
    case Qt::Key_Escape: select(-1); return true;
    default: break;
    }
    select(-1);   // (any other key goes to the cells, and the object is let go)
    return false;
}

} // namespace jp::grid
