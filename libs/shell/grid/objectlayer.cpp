#include "grid/objectlayer.h"

#include <QElapsedTimer>
#include <QPalette>

#include <algorithm>
#include <cmath>

namespace jp::grid {

using jp::office::sheet::AnchorKind;
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

// A place along an axis: the start of the row or column, and how far into it (EMU), no further than it is high or wide.
double placeOf(const Axis &axis, quint32 index, qint64 offsetEmu)
{
    if (qint64(index) >= axis.count()) return double(axis.total()) + double(offsetEmu) / double(kEmuPerPixel);
    const double into = std::min(double(offsetEmu) / double(kEmuPerPixel), double(axis.size(qint64(index))));
    return double(axis.position(qint64(index))) + into;
}

int clampPlace(qint64 v) { return int(std::max(-kFarthest, std::min(kFarthest, v))); }

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

ObjectLayer::ObjectLayer(QObject *parent) : QObject(parent)
{
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, [this] { step(); });
}

ObjectLayer::~ObjectLayer() = default;

void ObjectLayer::setObjects(std::vector<LayerObject> objects)
{
    m_timer.stop();
    m_entries.clear();
    m_bytes = 0;
    m_selected = -1;
    m_boxesValid = false;
    const size_t n = std::min<size_t>(objects.size(), size_t(kMaxObjects));
    m_entries.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        Entry e;
        e.object = std::move(objects[i]);
        m_entries.push_back(std::move(e));
    }
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

void ObjectLayer::setDeleted(int id, bool deleted)
{
    for (Entry &e : m_entries)
        if (e.object.id == id) {
            if (e.deleted == deleted) return;
            e.deleted = deleted;
            if (deleted && m_selected == id) m_selected = -1;
            requestRepaint();
            return;
        }
}

bool ObjectLayer::isDeleted(int id) const
{
    for (const Entry &e : m_entries)
        if (e.object.id == id) return e.deleted;
    return false;
}

void ObjectLayer::setAnchor(int id, const DrawingAnchor &anchor)
{
    for (Entry &e : m_entries)
        if (e.object.id == id) {
            e.object.anchor = anchor;
            m_boxesValid = false;
            requestRepaint();
            return;
        }
}

void ObjectLayer::invalidate(int id)
{
    for (Entry &e : m_entries)
        if (e.object.id == id) {
            e.stale = true;
            e.failed = false;
            schedule(kStaleDelayMillis);
            return;
        }
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
    for (const Entry &e : m_entries) e.box = anchorBox(e.object.anchor, layout.rows, layout.cols);
    m_stats.placed += qint64(m_entries.size());
}

QRect ObjectLayer::rectIn(const GridLayout &layout, Pane pane, const Entry &e) const
{
    if (e.deleted) return QRect();
    if (m_fills) {
        // the first object that is not deleted fills the part that scrolls
        for (const Entry &first : m_entries)
            if (!first.deleted) return &first == &e && pane == Pane::Body ? layout.pane(Pane::Body).rect : QRect();
        return QRect();
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

QSize ObjectLayer::capped(const QSize &wanted)
{
    const double pixels = double(wanted.width()) * double(wanted.height());
    if (pixels <= double(kMaxImagePixels)) return wanted.expandedTo(QSize(1, 1));
    const double k = std::sqrt(double(kMaxImagePixels) / pixels);
    return QSize(std::max(1, int(wanted.width() * k)), std::max(1, int(wanted.height() * k)));
}

QSize ObjectLayer::sizeWanted(const QRect &rect, double dpr) const
{
    return capped(QSize(int(std::ceil(double(rect.width()) * dpr)), int(std::ceil(double(rect.height()) * dpr))));
}

bool ObjectLayer::missing(const Entry &e, const QSize &wanted, double zoom) const
{
    if (e.failed && std::abs(e.requested.width() - wanted.width()) <= 2 && std::abs(e.requested.height() - wanted.height()) <= 2 && e.zoom == zoom) return false;
    if (e.image.isNull()) return true;
    // (the same object in two parts of the grid is a pixel different in size, by the rounding of the frozen edge)
    return std::abs(e.requested.width() - wanted.width()) > 2 || std::abs(e.requested.height() - wanted.height()) > 2 || e.zoom != zoom;
}

void ObjectLayer::render(Entry &e, const QSize &wanted, double zoom, const RenderRequest &request)
{
    m_bytes -= e.image.isNull() ? 0 : e.image.sizeInBytes();
    ++m_stats.renders;
    QImage image = e.object.render ? e.object.render(request) : QImage();
    e.requested = wanted;
    e.zoom = zoom;
    e.stale = false;
    e.failed = image.isNull();
    e.image = image;
    m_bytes += e.image.isNull() ? 0 : e.image.sizeInBytes();
    e.lastUsed = m_frame;
}

void ObjectLayer::evict()
{
    if (m_bytes <= kCacheBytes) return;
    std::vector<Entry *> held;
    for (Entry &e : m_entries)
        if (!e.image.isNull() && e.lastUsed != m_frame) held.push_back(&e);
    std::sort(held.begin(), held.end(), [](const Entry *a, const Entry *b) { return a->lastUsed < b->lastUsed; });
    for (Entry *e : held) {
        // pictures shown in the last few frames go only if the cache is twice its budget (a screen of pictures too large for it is made, not thrown away and made again)
        if (m_bytes <= kCacheBytes) break;
        if (e->lastUsed + 8 >= m_frame && m_bytes <= 2 * kCacheBytes) continue;
        m_bytes -= e->image.sizeInBytes();
        e->image = QImage();
        ++m_stats.evicted;
    }
}

void ObjectLayer::schedule(int millis)
{
    if (!m_timer.isActive() || m_timer.remainingTime() > millis) m_timer.start(millis);
}

void ObjectLayer::paint(QPainter *p, const GridLayout &layout, Pane pane, const QRect &clip)
{
    m_layout = &layout;
    if (m_entries.empty() || clip.isEmpty()) return;
    m_dpr = std::max(1.0, p->device()->devicePixelRatioF());
    ensureBoxes(layout);
    ++m_frame;
    const double zoom = layout.zoom();
    int made = 0;
    bool later = false;
    p->save();
    p->setClipRect(clip);
    for (Entry &e : m_entries) {
        const QRect box = rectIn(layout, pane, e);
        if (box.isEmpty()) continue;
        const QRect r = drawRect(box, e, zoom);
        if (!r.intersects(clip)) continue;
        const QSize wanted = sizeWanted(r, m_dpr);
        if (missing(e, wanted, zoom)) {
            if (made < kMaxRendersPerPaint) {
                render(e, wanted, zoom, requestFor(e, box, wanted, zoom));
                ++made;
            } else {
                later = true;
            }
        } else if (e.stale) {
            later = true;   // (drawn as it was; made again in a moment)
        }
        e.lastUsed = m_frame;
        const QRect target = r.intersected(clip);
        if (!e.image.isNull()) {
            // the part of the picture under the clip, so that an object a million pixels wide costs the clip
            const double sx = double(e.image.width()) / double(r.width()), sy = double(e.image.height()) / double(r.height());
            const QRectF source((target.x() - r.x()) * sx, (target.y() - r.y()) * sy, target.width() * sx, target.height() * sy);
            const bool scaled = std::abs(sx - m_dpr) > 1e-3 || std::abs(sy - m_dpr) > 1e-3;
            p->setRenderHint(QPainter::SmoothPixmapTransform, scaled);
            p->drawImage(QRectF(target), e.image, source);
            ++m_stats.drawn;
        } else {
            // no picture yet (or none can be made): an outline in the object's place
            p->setPen(QPen(QColor(0x80, 0x80, 0x80), 1, Qt::DashLine));
            p->setBrush(Qt::NoBrush);
            p->drawRect(box.adjusted(0, 0, -1, -1));
            ++m_stats.placeholders;
        }
    }
    // the object that is picked
    if (m_selected >= 0)
        for (const Entry &e : m_entries)
            if (e.object.id == m_selected) {
                const QRect r = rectIn(layout, pane, e);
                if (r.isEmpty() || !r.intersects(clip)) break;
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
                break;
            }
    p->restore();
    evict();
    if (later) schedule(made >= kMaxRendersPerPaint ? 0 : kStaleDelayMillis);
    // the grid has moved since the objects near the screen were looked at: they are, a moment after it stops
    const std::vector<qint64> key = {layout.scrollX(), layout.scrollY(), qint64(layout.zoom() * 1000), layout.viewport().width(), layout.viewport().height(), layout.frozenRows(), layout.frozenCols()};
    if (key != m_nearKey) {
        m_nearKey = key;
        m_timer.start(kNearDelayMillis);
    }
}

bool ObjectLayer::makePictures(bool all, bool near)
{
    if (!m_layout || m_entries.empty()) return false;
    const GridLayout &layout = *m_layout;
    ensureBoxes(layout);
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
            for (Entry &e : m_entries) {
                const QRect box = rectIn(layout, pane, e);
                if (box.isEmpty()) continue;
                const QRect r = drawRect(box, e, zoom);
                if (!r.intersects(area)) continue;
                const QSize wanted = sizeWanted(r, m_dpr);
                e.lastUsed = m_frame;
                if (!missing(e, wanted, zoom) && !e.stale) continue;
                if (!all && (made >= kPicturesPerStep || clock.elapsed() >= kStepMillis)) {
                    left = true;
                    continue;
                }
                render(e, wanted, zoom, requestFor(e, box, wanted, zoom));
                ++made;
                const QRect now = r.intersected(shown);
                if (!now.isEmpty()) requestRepaint(now);
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
    m_timer.stop();
    makePictures(true, false);
}

int ObjectLayer::pendingRenders() const
{
    if (!m_layout) return 0;
    const GridLayout &layout = *m_layout;
    ensureBoxes(layout);
    int pending = 0;
    for (const Entry &e : m_entries) {
        bool counted = false;
        for (const Pane pane : kPanes) {
            const QRect area = layout.pane(pane).rect;
            const QRect box = counted || area.isEmpty() ? QRect() : rectIn(layout, pane, e);
            if (box.isEmpty()) continue;
            const QRect r = drawRect(box, e, layout.zoom());
            if (!r.intersects(area)) continue;
            if (missing(e, sizeWanted(r, m_dpr), layout.zoom()) || e.stale) ++pending;
            counted = true;
        }
    }
    return pending;
}

QRect ObjectLayer::deviceRect(int id, Pane pane) const
{
    if (!m_layout) return QRect();
    ensureBoxes(*m_layout);
    for (const Entry &e : m_entries)
        if (e.object.id == id) {
            const QRect r = rectIn(*m_layout, pane, e);
            return r.intersects(m_layout->pane(pane).rect) ? r : QRect();
        }
    return QRect();
}

int ObjectLayer::objectAt(const QPoint &point) const
{
    if (!m_layout) return -1;
    const GridLayout &layout = *m_layout;
    ensureBoxes(layout);
    for (const Pane pane : kPanes) {
        if (!layout.pane(pane).rect.contains(point)) continue;
        for (size_t i = m_entries.size(); i-- > 0;) {
            const Entry &e = m_entries[i];
            if (rectIn(layout, pane, e).contains(point)) return e.object.id;
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
