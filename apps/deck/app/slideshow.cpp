#include "app/slideshow.h"

#include "app/icons.h"
#include "app/presenterview.h"
#include "app/rehearsebar.h"
#include "app/settings.h"
#include "core/document.h"
#include "core/reuse.h"
#include "render/renderer.h"

#include <QAction>
#include <QActionGroup>
#include <QCloseEvent>
#include <QCursor>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QRadialGradient>
#include <QScreen>
#include <QSet>
#include <QUrl>
#include <algorithm>
#include <optional>

namespace jp {

namespace {

// ---- drawing a slide ----

// Where a slide goes in a window: as large as it fits, in the middle.
QRectF fitSlide(const QSizeF &page, const QSizeF &area)
{
    const double scale = std::min(area.width() / page.width(), area.height() / page.height());
    const QSizeF shown = page * scale;
    return QRectF(QPointF((area.width() - shown.width()) / 2, (area.height() - shown.height()) / 2), shown);
}

// ---- the transitions, one function each ----

using deck::Transition;

struct Frame {
    QPainter *p;
    const QImage &from, &to;
    double t;           // 0 to 1, eased
    QString dir;
    QSizeF size;        // of the images, in the painter's units
    qreal dpr;
};

// The way things move on the screen for a direction.
QPointF motion(const QString &dir)
{
    if (dir == QLatin1String("r")) return QPointF(1, 0);
    if (dir == QLatin1String("u")) return QPointF(0, -1);
    if (dir == QLatin1String("d")) return QPointF(0, 1);
    return QPointF(-1, 0);
}

// A share of the image's size, in whole device pixels, so that drawing an
// image at it stays a plain copy.
QPointF shifted(const Frame &f, const QPointF &share)
{
    return QPointF(std::round(share.x() * f.size.width() * f.dpr) / f.dpr, std::round(share.y() * f.size.height() * f.dpr) / f.dpr);
}

double snapped(const Frame &f, double v) { return std::round(v * f.dpr) / f.dpr; }

void drawClipped(const Frame &f, const QImage &image, const QRectF &area)
{
    if (area.isEmpty()) return;
    f.p->save();
    f.p->setClipRect(area, Qt::IntersectClip);
    f.p->drawImage(QPointF(0, 0), image);
    f.p->restore();
}

void drawFade(const Frame &f)
{
    f.p->drawImage(QPointF(0, 0), f.from);
    f.p->setOpacity(f.t);
    f.p->drawImage(QPointF(0, 0), f.to);
}

// The old slide is shoved off by the new one.
void drawPush(const Frame &f)
{
    const QPointF v = motion(f.dir);
    f.p->drawImage(shifted(f, v * f.t), f.from);
    f.p->drawImage(shifted(f, v * (f.t - 1)), f.to);
}

// An edge crosses the slide and shows the new one behind it.
void drawWipe(const Frame &f)
{
    const QPointF v = motion(f.dir);
    const double w = f.size.width(), h = f.size.height();
    QRectF fresh, old;
    if (v.x() != 0) {
        const double edge = snapped(f, v.x() < 0 ? w * (1 - f.t) : w * f.t);
        fresh = v.x() < 0 ? QRectF(edge, 0, w - edge, h) : QRectF(0, 0, edge, h);
        old = v.x() < 0 ? QRectF(0, 0, edge, h) : QRectF(edge, 0, w - edge, h);
    } else {
        const double edge = snapped(f, v.y() < 0 ? h * (1 - f.t) : h * f.t);
        fresh = v.y() < 0 ? QRectF(0, edge, w, h - edge) : QRectF(0, 0, w, edge);
        old = v.y() < 0 ? QRectF(0, 0, w, edge) : QRectF(0, edge, w, h - edge);
    }
    drawClipped(f, f.to, fresh);
    drawClipped(f, f.from, old);
}

// The new slide opens from the middle outward, or closes in from both sides.
void drawSplit(const Frame &f)
{
    const double w = f.size.width(), h = f.size.height();
    if (f.dir == QLatin1String("in")) {
        const double side = snapped(f, w * f.t / 2);
        drawClipped(f, f.to, QRectF(0, 0, side, h));
        drawClipped(f, f.to, QRectF(w - side, 0, side, h));
        drawClipped(f, f.from, QRectF(side, 0, w - 2 * side, h));
    } else {
        const double side = snapped(f, w * (1 - f.t) / 2);
        drawClipped(f, f.to, QRectF(side, 0, w - 2 * side, h));
        drawClipped(f, f.from, QRectF(0, 0, side, h));
        drawClipped(f, f.from, QRectF(w - side, 0, side, h));
    }
}

// The old slide slides away and thins out, and the new one is there.
void drawReveal(const Frame &f)
{
    f.p->drawImage(QPointF(0, 0), f.to);
    f.p->setOpacity(1 - f.t);
    f.p->drawImage(shifted(f, motion(f.dir) * f.t), f.from);
}

// The new slide slides in over the old one.
void drawCover(const Frame &f)
{
    f.p->drawImage(QPointF(0, 0), f.from);
    f.p->drawImage(shifted(f, motion(f.dir) * (f.t - 1)), f.to);
}

// The old slide slides off the new one.
void drawUncover(const Frame &f)
{
    f.p->drawImage(QPointF(0, 0), f.to);
    f.p->drawImage(shifted(f, motion(f.dir) * f.t), f.from);
}

// An image grown or shrunk about the middle.
void drawScaled(const Frame &f, const QImage &image, double scale, double opacity)
{
    const QSizeF s(snapped(f, f.size.width() * scale), snapped(f, f.size.height() * scale));
    if (s.width() < 1 || s.height() < 1) return;
    f.p->setRenderHint(QPainter::SmoothPixmapTransform);
    f.p->setOpacity(std::clamp(opacity, 0.0, 1.0));
    f.p->drawImage(QRectF(QPointF(snapped(f, (f.size.width() - s.width()) / 2), snapped(f, (f.size.height() - s.height()) / 2)), s), image);
}

// The new slide grows from the middle, or the old one shrinks into it.
void drawZoom(const Frame &f)
{
    if (f.dir == QLatin1String("out")) {
        f.p->drawImage(QPointF(0, 0), f.to);
        drawScaled(f, f.from, 1 - f.t, (1 - f.t) * 2.5);
    } else {
        f.p->drawImage(QPointF(0, 0), f.from);
        drawScaled(f, f.to, f.t, f.t * 2.5);
    }
}

// The old slide goes, the screen is black for a moment, and the new one comes.
void drawCut(const Frame &f)
{
    if (f.t < 0.35) f.p->drawImage(QPointF(0, 0), f.from);
    else if (f.t < 0.65) f.p->fillRect(QRectF(QPointF(0, 0), f.size), Qt::black);
    else f.p->drawImage(QPointF(0, 0), f.to);
}

void drawNone(const Frame &f) { f.p->drawImage(QPointF(0, 0), f.t < 1 ? f.from : f.to); }

// A band of the frame between two edges, a little wider than they are for the
// pixels an edge is snapped to.
QRegion band(const QSizeF &size, bool across, double a, double b)
{
    const double lo = std::min(a, b) - 2, hi = std::max(a, b) + 2;
    const QRectF r = across ? QRectF(lo, 0, hi - lo, size.height()) : QRectF(0, lo, size.width(), hi - lo);
    return QRegion(r.toAlignedRect() & QRect(QPoint(0, 0), size.toSize()));
}

} // namespace

QRegion transitionChange(const deck::Transition &t, const QSizeF &size, double before, double after)
{
    const QRegion all(QRect(QPoint(0, 0), size.toSize()));
    const double w = size.width(), h = size.height();
    if (t.type == QLatin1String("wipe")) {
        const QPointF v = motion(t.dir);
        const bool across = v.x() != 0;
        const double length = across ? w : h, along = across ? v.x() : v.y();
        auto edge = [&](double p) { return along < 0 ? length * (1 - p) : length * p; };
        return band(size, across, edge(before), edge(after));
    }
    if (t.type == QLatin1String("split")) {
        // The two edges that move in or out from the sides.
        auto side = [&](double p) { return t.dir == QLatin1String("in") ? w * p / 2 : w * (1 - p) / 2; };
        return band(size, true, side(before), side(after)).united(band(size, true, w - side(before), w - side(after)));
    }
    if (t.type == QLatin1String("cut")) {
        // The picture only changes when it goes black and when it comes back.
        auto step = [](double p) { return (p < 0.35 ? 0 : 1) + (p < 0.65 ? 0 : 1); };
        return step(before) == step(after) ? QRegion() : all;
    }
    return all;
}

namespace {

QSize pixelsOf(const QSize &size, qreal dpr) { return QSize(std::max(1, qRound(size.width() * dpr)), std::max(1, qRound(size.height() * dpr))); }

// Has `p` paint in slide points on an image of `px` pixels at `dpr`: the slide
// as large as it goes, in the middle, and nothing outside it. Drawn with gray
// text smoothing; a screen image has no use for the colored fringes of
// subpixel smoothing, and the projector may not be this screen.
void placeSlide(QPainter &p, const QSizeF &page, const QSize &px, qreal dpr)
{
    const QRectF at = fitSlide(page, QSizeF(px) / dpr);
    const double scale = at.width() / page.width();
    p.translate(at.topLeft());
    p.scale(scale, scale);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setClipRect(QRectF(QPointF(0, 0), page));
}

// The slide's page painted opaque, but for the objects in `hidden`.
QImage paintSlide(const Document *doc, LayoutCache *cache, int slide, const QSize &size, qreal dpr, const QColor &bars, const deck::States *states, const QSet<QString> &hidden)
{
    const QSize px = pixelsOf(size, dpr);
    QImage img(px, QImage::Format_ARGB32_Premultiplied);
    img.setDevicePixelRatio(dpr);
    img.fill(bars);
    if (slide >= 0 && slide < doc->pages.size()) {
        QPainter p(&img);
        const QSizeF page = doc->pageSize();
        placeSlide(p, page, px, dpr);
        p.fillRect(QRectF(QPointF(0, 0), page), Qt::white);
        PaintContext ctx;
        ctx.doc = doc;
        ctx.cache = cache;
        ctx.opt.hidden = hidden;
        if (states) deck::animate(ctx, *doc, *states);
        Renderer::paintPage(&p, ctx, slide);
    }
    // Opaque, so that painting it on the screen is a copy.
    QImage opaque = img.convertToFormat(QImage::Format_RGB32);
    opaque.setDevicePixelRatio(dpr);
    return opaque;
}

bool holdsTarget(const Item &it, const QSet<int> &targets)
{
    if (targets.contains(it.sourceId)) return true;
    if (it.type() == ItemType::Group)
        for (const ItemPtr &c : static_cast<const GroupItem &>(it).children)
            if (holdsTarget(*c, targets)) return true;
    return false;
}

} // namespace

QImage renderSlideImage(const Document *doc, LayoutCache *cache, int slide, const QSize &size, qreal dpr, const QColor &bars, const deck::States *states)
{
    return paintSlide(doc, cache, slide, size, dpr, bars, states, QSet<QString>());
}

// A slide drawn again for each frame of one of its steps. The objects the
// step moves are painted every time; what is beneath them and what is above
// them stays as it is, and is painted once.
class StepFrames {
public:
    StepFrames(const Document *doc, LayoutCache *cache, int slide, const QSize &size, qreal dpr, const QColor &bars, const deck::Timeline &timeline,
               const deck::Geometry &geometry, int step)
        : m_doc(doc), m_cache(cache), m_slide(slide), m_size(size), m_ratio(dpr), m_bars(bars), m_timeline(timeline), m_geometry(geometry), m_step(step)
    {
        const ItemList &items = doc->pages[slide]->items;
        QSet<int> targets;
        for (const deck::Effect &e : timeline.steps[step].effects)
            for (const deck::Behavior &b : e.behaviors)
                if (b.target.shape > 0) targets.insert(b.target.shape);
        m_first = int(items.size());
        for (int i = 0; i < int(items.size()); ++i)
            if (holdsTarget(*items[i], targets)) {
                m_first = std::min(m_first, i);
                m_last = i;
            }
        const deck::States rest = deck::statesAt(timeline, geometry, step, -1);
        QSet<QString> above;
        for (int i = m_first; i < int(items.size()); ++i) above.insert(items[i]->id);
        m_below = paintSlide(doc, cache, slide, size, dpr, bars, &rest, above);
        if (m_last >= 0 && m_last + 1 < int(items.size())) {
            const QSize px = pixelsOf(size, dpr);
            m_above = QImage(px, QImage::Format_ARGB32_Premultiplied);
            m_above.setDevicePixelRatio(dpr);
            m_above.fill(Qt::transparent);
            QPainter p(&m_above);
            placeSlide(p, doc->pageSize(), px, dpr);
            paintObjects(p, rest, m_last + 1, int(items.size()));
        }
    }

    bool matches(int slide, int step, const QSize &size, qreal dpr, const QColor &bars) const
    {
        return slide == m_slide && step == m_step && size == m_size && dpr == m_ratio && bars == m_bars;
    }

    QImage frame(double ms)
    {
        QImage out = m_below;
        if (m_last < 0) return out;
        const QSize px = pixelsOf(m_size, m_ratio);
        QPainter p(&out);
        placeSlide(p, m_doc->pageSize(), px, m_ratio);
        paintObjects(p, deck::statesAt(m_timeline, m_geometry, m_step, ms), m_first, m_last + 1);
        if (!m_above.isNull()) {
            p.resetTransform();
            p.setClipping(false);
            p.drawImage(0, 0, m_above);
        }
        return out;
    }

private:
    // Objects `from` up to `to` of the page, with the states.
    void paintObjects(QPainter &p, const deck::States &states, int from, int to)
    {
        PaintContext ctx;
        ctx.doc = m_doc;
        ctx.cache = m_cache;
        ctx.pageNumber = m_slide + 1;
        ctx.pageCount = int(m_doc->pages.size());
        deck::animate(ctx, *m_doc, states);
        const ItemList &items = m_doc->pages[m_slide]->items;
        for (int i = from; i < to; ++i) {
            const Item &it = *items[i];
            ctx.around(&p, it, [&](QPainter *pp) { Renderer::paintItem(pp, ctx, it); });
        }
    }
    const Document *m_doc;
    LayoutCache *m_cache;
    int m_slide;
    QSize m_size;
    qreal m_ratio;
    QColor m_bars;
    deck::Timeline m_timeline;
    deck::Geometry m_geometry;
    int m_step;
    int m_first = 0, m_last = -1;   // the page's objects the step moves, the lowest and the highest
    QImage m_below, m_above;        // the page up to the first of them, opaque; the page above the last, on nothing
};

void paintTransition(QPainter *p, const deck::Transition &t, const QImage &from, const QImage &to, double progress)
{
    using Draw = void (*)(const Frame &);
    static const QHash<QString, Draw> draws = {
        {QStringLiteral("fade"), drawFade},     {QStringLiteral("push"), drawPush},   {QStringLiteral("wipe"), drawWipe},
        {QStringLiteral("split"), drawSplit},   {QStringLiteral("reveal"), drawReveal}, {QStringLiteral("cover"), drawCover},
        {QStringLiteral("uncover"), drawUncover}, {QStringLiteral("zoom"), drawZoom}, {QStringLiteral("cut"), drawCut},
    };
    const Frame f{p, from, to, std::clamp(progress, 0.0, 1.0), t.dir, from.deviceIndependentSize(), from.devicePixelRatio()};
    p->save();
    draws.value(t.type, drawNone)(f);
    p->restore();
}

QRectF slideRectIn(const QSizeF &page, const QSizeF &area) { return fitSlide(page, area); }

// ---- the show ----

SlideShow::SlideShow(const Document *doc, LayoutCache *cache, int startSlide, QWidget *parent)
    : QWidget(parent, Qt::Window | Qt::FramelessWindowHint), m_doc(doc), m_cache(cache)
{
    setObjectName(QStringLiteral("slideShow"));
    setAccessibleName(tr("Slide Show"));
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::BlankCursor);
    setMouseTracking(true);
    for (const auto &page : doc->pages) m_ids << page->id;
    m_startSlide = std::clamp(startSlide, 0, std::max(0, int(doc->pages.size()) - 1));
    m_pos = nextShown(m_startSlide, 1);
    m_atEnd = m_pos < 0;   // every slide hidden: the end, on no slide
    m_slide = m_target = m_atEnd ? -1 : slideAt(m_pos);
    m_pos = m_targetPos = std::max(0, m_pos);
    m_penColor = m_settings.penColor;
    m_highlighterColor = deck::highlighterColor();
    m_clock = [] {
        static QElapsedTimer timer = [] {
            QElapsedTimer t;
            t.start();
            return t;
        }();
        return timer.elapsed();
    };
    m_anim.setStartValue(0.0);
    m_anim.setEndValue(1.0);
    m_anim.setEasingCurve(QEasingCurve::InOutQuad);
    // Only what changed is painted again, over the frame before.
    connect(&m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        const double progress = value.toDouble();
        update(m_painted < 0 ? QRegion(rect()) : transitionChange(m_trans, size(), m_painted, progress));
        m_painted = progress;
    });
    connect(&m_anim, &QVariantAnimation::finished, this, &SlideShow::endTransition);
    m_auto.setSingleShot(true);
    connect(&m_auto, &QTimer::timeout, this, [this] { moveOn(); });
    m_frame.setTimerType(Qt::PreciseTimer);
    connect(&m_frame, &QTimer::timeout, this, &SlideShow::stepFrame);
}

const Page *SlideShow::pageOf(int slide) const { return slide >= 0 && slide < m_doc->pages.size() ? m_doc->pages[slide].get() : nullptr; }

QString SlideShow::idOf(int slide) const
{
    const Page *page = pageOf(slide);
    return page ? page->id : QString();
}

int SlideShow::nextShown(int from, int step) const
{
    for (int i = from; i >= 0 && i < count(); i += step) {
        // A slide that is not there (the deck was edited) is not shown.
        const Page *page = pageOf(slideAt(i));
        if (page && (m_hiddenToo || !page->hidden)) return i;
    }
    return -1;
}

// The first place in the order the slide has; -1 if it plays nowhere.
int SlideShow::positionOf(int slide) const
{
    if (m_order.isEmpty()) return slide >= 0 && slide < m_doc->pages.size() ? slide : -1;
    return int(m_order.indexOf(slide));
}

// The next place after the one the show is at that has the slide, going round to the first.
int SlideShow::placeOf(int slide) const
{
    if (m_order.isEmpty()) return positionOf(slide);
    const int at = m_atEnd ? -1 : m_animating ? m_targetPos : m_pos, n = int(m_order.size());
    for (int k = 1; k <= n; ++k)
        if (const int pos = (at + k) % n; m_order[pos] == slide) return pos;
    return -1;
}

// What a click goes to: the next slide, and in a looping show the first after the last.
int SlideShow::slideAfter() const
{
    const int pos = m_animating ? m_targetPos : m_pos;
    int n = nextShown(pos + 1, 1);
    if (n < 0 && m_settings.looping()) n = nextShown(0, 1);
    return n < 0 ? -1 : slideAt(n);
}

QList<int> SlideShow::playedSlides() const
{
    QList<int> out;
    for (int i = 0; i < count(); ++i)
        if (const Page *page = pageOf(slideAt(i)); page && (m_hiddenToo || !page->hidden)) out << slideAt(i);
    return out;
}

void SlideShow::setSettings(const deck::ShowSettings &settings)
{
    m_settings = settings;
    m_penColor = settings.penColor;
    // By an individual the show is a window, as in a rehearsal; the others fill the screen.
    if (settings.type == deck::ShowSettings::Type::Individual) {
        setWindowFlag(Qt::FramelessWindowHint, false);
        unsetCursor();
        setWindowTitle(tr("Slide Show"));
    } else {
        if (!m_rehearsal) setWindowFlag(Qt::FramelessWindowHint, true);
        updateCursor();
    }
    m_plays.clear();
}

// The slides to play, in order; the show starts at the first of them (a show of every slide starts where it was asked to).
void SlideShow::setSlides(const QList<int> &slides, bool includeHidden)
{
    m_order.clear();
    m_orderIds.clear();
    for (const int slide : slides)
        if (slide >= 0 && slide < m_doc->pages.size()) {
            m_order << slide;
            m_orderIds << m_doc->pages[slide]->id;
        }
    m_hiddenToo = includeHidden && !m_order.isEmpty();
    m_pos = nextShown(m_order.isEmpty() ? m_startSlide : 0, 1);
    m_atEnd = m_pos < 0;
    m_slide = m_target = m_atEnd ? -1 : slideAt(m_pos);
    m_pos = m_targetPos = std::max(0, m_pos);
    m_images.clear();
    m_nowValid = false;
}

// ---- following the deck when it is edited ----

void SlideShow::documentChanged()
{
    if (m_rebound) return;
    QStringList ids;
    ids.reserve(m_doc->pages.size());
    for (const auto &page : m_doc->pages) ids << page->id;
    // What was drawn is drawn again: the slides may have changed inside.
    m_images.clear();
    m_frames.clear();
    m_nowValid = false;
    if (ids == m_ids) {
        update();
        Q_EMIT slidesChanged();
        return;
    }
    // The pages are not as they were: the show stays on its page, or on the place of the one that went.
    const int oldPos = m_animating ? m_targetPos : m_pos;
    const QString nowId = m_ids.value(m_animating ? m_target : m_slide);
    m_anim.stop();
    m_animating = false;
    m_fromOverlay = m_toOverlay = QImage();
    m_frame.stop();
    m_auto.stop();
    m_playing = -1;
    m_done = 0;
    m_drawing = m_erasing = false;
    m_plays.clear();
    QHash<QString, int> index;
    for (int i = 0; i < ids.size(); ++i) index.insert(ids[i], i);
    bool gone = ids.isEmpty();
    if (!m_order.isEmpty()) {
        QList<int> order;
        QStringList orderIds;
        int exact = -1, before = 0;
        for (int i = 0; i < m_orderIds.size(); ++i) {
            const auto it = index.constFind(m_orderIds[i]);
            if (it == index.constEnd()) continue;
            if (i < oldPos) ++before;
            if (i == oldPos) exact = int(order.size());
            order << it.value();
            orderIds << m_orderIds[i];
        }
        m_order = order;
        m_orderIds = orderIds;
        gone = order.isEmpty();
        if (!gone) m_pos = exact >= 0 ? exact : std::min(before, int(order.size()) - 1);
    } else if (!gone) {
        const auto it = index.constFind(nowId);
        m_pos = it != index.constEnd() ? it.value() : std::clamp(oldPos, 0, int(ids.size()) - 1);
    }
    m_ids = ids;
    // The times of slides that are gone are gone.
    for (auto it = m_recorded.begin(); it != m_recorded.end();) it = index.contains(it.key()) ? std::next(it) : m_recorded.erase(it);
    if (gone) {
        m_atEnd = true;
        m_slide = m_target = -1;
        m_pos = m_targetPos = 0;
        update();
    } else {
        m_targetPos = m_pos;
        m_slide = m_target = slideAt(m_pos);
        settle();
    }
    Q_EMIT changed();
    Q_EMIT slidesChanged();
}

void SlideShow::rebind(const Document *doc)
{
    m_doc = doc;
    m_rebound = true;
    m_anim.stop();
    m_animating = false;
    m_fromOverlay = m_toOverlay = QImage();
    m_frame.stop();
    m_auto.stop();
    m_playing = -1;
    m_done = 0;
    m_drawing = m_erasing = false;
    m_plays.clear();
    m_images.clear();
    m_frames.clear();
    m_nowValid = false;
    m_inkLayers.clear();
    m_inkLayerOrder.clear();
    m_ids.clear();
    m_order.clear();
    m_orderIds.clear();
    m_hiddenToo = false;
    m_atEnd = true;
    m_slide = m_target = -1;
    m_pos = m_targetPos = 0;
    update();
}

PresenterView *SlideShow::presenterView() const { return m_presenter; }

bool SlideShow::usePresenterView() { return Settings::get().value(QStringLiteral("show/usePresenterView"), true).toBool(); }

void SlideShow::setUsePresenterView(bool on) { Settings::get().setValue(QStringLiteral("show/usePresenterView"), on); }

void SlideShow::setRehearsal(bool on)
{
    m_rehearsal = on;
    setWindowFlag(Qt::FramelessWindowHint, !on);
    if (on) unsetCursor();
    else setCursor(Qt::BlankCursor);
    setWindowTitle(on ? tr("Slide Show") : QString());
}

// ---- moving ----

// A click: it finishes the effect that plays, else plays the slide's next step, else moves on.
void SlideShow::next()
{
    const bool inTransition = m_animating;
    finishTransition();
    if (inTransition) finishStep();   // the effects that began when the transition ended
    if (m_playing >= 0) {
        m_blank = QColor();
        finishStep();
        update();
        return;
    }
    if (!m_atEnd && m_done < stepCount()) {
        m_auto.stop();
        m_blank = QColor();
        startStep(m_done);
        return;
    }
    moveOn();
}

// To the next slide, past the steps left.
void SlideShow::moveOn()
{
    finishTransition();
    m_auto.stop();
    const bool wasBlank = m_blank.isValid();
    m_blank = QColor();
    if (m_atEnd) {
        close();
        return;
    }
    int n = nextShown(m_pos + 1, 1);
    // A looping show starts again at its first slide.
    if (n < 0 && m_settings.looping()) n = nextShown(0, 1);
    if (n < 0) {
        m_atEnd = true;
        timeSlide(-1);
        update();
        Q_EMIT changed();
        return;
    }
    const deck::Transition t = deck::transitionOf(*pageOf(slideAt(n)));
    if (t.type != QLatin1String("none") && !wasBlank) startTransition(n, t);
    else showSlide(n);
}

void SlideShow::previous()
{
    finishTransition();
    m_auto.stop();
    m_blank = QColor();
    if (m_atEnd) {
        m_atEnd = false;
        showSlide(m_pos, m_slide >= 0 ? stepCount() : 0);
    } else if (m_playing >= 0) {
        // The effect stops, and its step is not played.
        m_frame.stop();
        m_done = m_playing;
        m_playing = -1;
        stepChanged();
    } else if (m_done > 0) {
        --m_done;
        stepChanged();
    } else if (const int p = nextShown(m_pos - 1, -1); p >= 0) {
        showSlide(p, stepsOf(slideAt(p)));
    } else {
        showSlide(m_pos);
    }
}

void SlideShow::goTo(int slide)
{
    if (slide < 0 || slide >= m_doc->pages.size()) return;
    goToPosition(placeOf(slide));
}

void SlideShow::goToPosition(int pos)
{
    if (pos < 0 || pos >= count() || !pageOf(slideAt(pos))) return;
    finishTransition();
    m_auto.stop();
    m_blank = QColor();
    m_atEnd = false;
    showSlide(pos);
}

// The slide at a place in the order on the screen with `steps` of its steps played.
void SlideShow::showSlide(int pos, int steps)
{
    m_frame.stop();
    // A stroke being drawn stays on the slide it began on.
    m_drawing = m_erasing = false;
    m_pos = m_targetPos = pos;
    m_slide = m_target = slideAt(pos);
    m_done = std::min(steps, stepsOf(m_slide));
    m_playing = -1;
    m_nowValid = false;
    timeSlide(m_slide);
    settle();
    Q_EMIT changed();
}

void SlideShow::toggleBlank(const QColor &color)
{
    finishTransition();
    m_blank = m_blank == color ? QColor() : color;
    update();
    Q_EMIT changed();
}

void SlideShow::startTransition(int pos, const deck::Transition &t)
{
    // Both images are ready before the first frame, with the ink the slides hold.
    const int slide = slideAt(pos);
    m_fromOverlay = withOverlay(nowImage(), m_slide);
    m_toOverlay = withOverlay(imageOf(slide), slide);
    m_drawing = m_erasing = false;
    m_targetPos = pos;
    m_target = slide;
    timeSlide(slide);
    m_trans = t;
    m_animating = true;
    m_painted = -1;
    m_anim.stop();
    m_anim.setDuration(std::max(1, qRound(t.duration * 1000)));
    m_anim.start();
    update();
    Q_EMIT changed();
}

void SlideShow::finishTransition()
{
    if (m_animating) endTransition();
}

void SlideShow::endTransition()
{
    if (!m_animating) return;
    m_animating = false;
    m_anim.stop();
    m_fromOverlay = m_toOverlay = QImage();
    m_drawing = m_erasing = false;
    m_slide = m_target;
    m_pos = m_targetPos;
    m_done = 0;
    m_playing = -1;
    m_nowValid = false;
    settle();
}

// The slide is on the screen to stay, for now.
void SlideShow::settle()
{
    update();
    startAutomaticSteps();
    startAutoAdvance();
    QTimer::singleShot(0, this, &SlideShow::prefetch);
}

void SlideShow::startAutoAdvance()
{
    m_auto.stop();
    // The slides' times move the show on, unless the presenter did not ask for them, or they are being made.
    if (!m_started || m_atEnd || m_animating || m_playing >= 0 || m_recording || !m_settings.usesTimings()) return;
    const Page *page = pageOf(m_slide);
    if (!page) return;
    if (const auto after = deck::transitionOf(*page).advanceAfter) m_auto.start(std::max(0, qRound(*after * 1000)));
}

// ---- the slide's animations ----

const SlideShow::Play &SlideShow::playOf(int slide) const
{
    static const Play none;
    // Without animation the slides show as they are.
    if (slide < 0 || slide >= m_doc->pages.size() || !m_settings.animation) return none;
    auto it = m_plays.find(slide);
    if (it == m_plays.end()) {
        Play play;
        play.timeline = deck::playableTimeline(*m_doc->pages[slide]);
        if (!play.timeline.isEmpty()) play.geometry = deck::geometryOf(*m_doc, *m_doc->pages[slide]);
        it = m_plays.insert(slide, play);
    }
    return it.value();
}

int SlideShow::stepCount() const
{
    return m_atEnd ? 0 : stepsOf(m_slide);
}

SlideShow::View SlideShow::currentView() const
{
    if (m_atEnd) return View();
    if (m_animating) return View{m_target, 0};
    return View{m_slide, m_playing >= 0 ? m_playing : m_done};
}

double SlideShow::currentMs() const { return m_playing >= 0 && !m_animating && !m_atEnd ? m_stepMs : -1; }

SlideShow::View SlideShow::nextView() const
{
    const View now = currentView();
    if (now.slide < 0) return View();
    // The click after the one that finishes the effect that plays.
    const int played = m_playing >= 0 && !m_animating ? m_playing + 1 : now.steps;
    if (played < stepsOf(now.slide)) return View{now.slide, played + 1};
    return View{slideAfter(), 0};
}

QImage SlideShow::renderView(const View &view, double ms, const QSize &size, qreal dpr, const QColor &bars)
{
    const Play &play = playOf(view.slide);
    if (play.timeline.isEmpty()) return renderSlideImage(m_doc, m_cache, view.slide, size, dpr, bars);
    if (ms >= 0 && view.steps < play.timeline.steps.size()) return framesOf(view, size, dpr, bars)->frame(ms);
    const deck::States states = deck::statesAt(play.timeline, play.geometry, view.steps, ms);
    return renderSlideImage(m_doc, m_cache, view.slide, size, dpr, bars, &states);
}

// What draws the frames of the step after the view's steps at this size: kept for the few views that ask.
std::shared_ptr<StepFrames> SlideShow::framesOf(const View &view, const QSize &size, qreal dpr, const QColor &bars)
{
    for (auto it = m_frames.begin(); it != m_frames.end(); ++it)
        if ((*it)->matches(view.slide, view.steps, size, dpr, bars)) {
            std::rotate(m_frames.begin(), it, it + 1);
            return m_frames.front();
        }
    const Play &play = playOf(view.slide);
    m_frames.insert(m_frames.begin(), std::make_shared<StepFrames>(m_doc, m_cache, view.slide, size, dpr, bars, play.timeline, play.geometry, view.steps));
    if (m_frames.size() > 3) m_frames.pop_back();
    return m_frames.front();
}

// The step plays from its click, one frame after another, and ends at its last effect's end.
void SlideShow::startStep(int step)
{
    m_playing = step;
    m_stepLength = playOf(m_slide).timeline.steps[step].duration();
    m_stepMs = 0;
    m_nowValid = false;
    if (m_stepLength < 2) {   // an appearing shape: a set that takes a millisecond
        finishStep();
        return;
    }
    m_stepClock.start();
    const qreal rate = screen() ? screen()->refreshRate() : 60;
    m_frame.start(std::clamp(qRound(1000 / (rate > 1 ? rate : 60)), 4, 33));
    update();
    Q_EMIT changed();
}

void SlideShow::stepFrame()
{
    if (m_playing < 0) {
        m_frame.stop();
        return;
    }
    const double ms = m_stepClock.nsecsElapsed() / 1e6;
    if (ms >= m_stepLength) {
        finishStep();
        return;
    }
    m_stepMs = ms;
    m_nowValid = false;
    update();
    Q_EMIT frameChanged();
}

void SlideShow::finishStep()
{
    if (m_playing < 0) return;
    m_frame.stop();
    m_done = m_playing + 1;
    m_playing = -1;
    m_nowValid = false;
    update();
    Q_EMIT changed();
    // The step after it may begin by itself; if none does, the slide's seconds count from here.
    startAutomaticSteps();
    startAutoAdvance();
    QTimer::singleShot(0, this, &SlideShow::prefetch);
}

// The steps played changed: draw the slide again, count the seconds from here, and tell Presenter View.
void SlideShow::stepChanged()
{
    m_nowValid = false;
    update();
    startAutoAdvance();
    Q_EMIT changed();
    QTimer::singleShot(0, this, &SlideShow::prefetch);
}

// A slide shown at its start, or a step after one that has played, that begins by itself.
void SlideShow::startAutomaticSteps()
{
    if (!m_started || m_atEnd || m_animating || m_playing >= 0) return;
    const deck::Timeline &tl = playOf(m_slide).timeline;
    if (m_done < tl.steps.size() && tl.steps[m_done].automatic) startStep(m_done);
}

// ---- drawing ----

QImage SlideShow::imageOf(int slide)
{
    const qreal dpr = devicePixelRatioF();
    if (size() != m_imageSize || dpr != m_imageRatio) {
        m_images.clear();
        m_imageSize = size();
        m_imageRatio = dpr;
    }
    auto it = m_images.constFind(slide);
    if (it != m_images.constEnd()) return *it;
    const QImage img = renderView(View{slide, 0}, -1, m_imageSize, m_imageRatio);
    m_images.insert(slide, img);
    return img;
}

// The slide on the screen: at its start, the image kept for it, else drawn as its steps have it.
QImage SlideShow::nowImage()
{
    if (m_playing < 0 && m_done == 0) return imageOf(m_slide);
    const qreal dpr = devicePixelRatioF();
    if (!m_nowValid || size() != m_nowSize || dpr != m_nowRatio) {
        m_now = renderView(View{m_slide, m_playing >= 0 ? m_playing : m_done}, m_playing >= 0 ? m_stepMs : -1, size(), dpr);
        m_nowSize = size();
        m_nowRatio = dpr;
        m_nowValid = true;
    }
    return m_now;
}

// The slides the presenter can move to next are drawn while this one is talked
// over, one at a time so that keys and clicks are not kept waiting.
void SlideShow::prefetch()
{
    if (!isVisible() || m_atEnd) return;
    const int aheadPos = nextShown(m_pos + 1, 1), behindPos = nextShown(m_pos - 1, -1);
    const int ahead = aheadPos < 0 ? -1 : slideAt(aheadPos), behind = behindPos < 0 ? -1 : slideAt(behindPos);
    QSet<int> keep{m_slide, ahead, behind};
    if (m_animating) keep.insert(m_target);
    for (auto it = m_images.begin(); it != m_images.end();) it = keep.contains(it.key()) ? std::next(it) : m_images.erase(it);
    for (const int slide : {ahead, behind}) {
        if (slide < 0 || m_images.contains(slide)) continue;
        imageOf(slide);
        QTimer::singleShot(0, this, &SlideShow::prefetch);
        return;
    }
    // The next step of this slide, ready to play at the click.
    if (m_imageSize.isValid() && !m_animating && m_playing < 0 && m_done < stepCount()) framesOf(View{m_slide, m_done}, m_imageSize, m_imageRatio, Qt::black);
}

void SlideShow::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    if (m_blank.isValid()) {
        p.fillRect(rect(), m_blank);
        return;
    }
    if (m_atEnd) {
        p.fillRect(rect(), Qt::black);
        p.setPen(Qt::white);
        QFont f = font();
        f.setPointSizeF(16);
        p.setFont(f);
        p.drawText(rect().adjusted(0, 24, 0, 0), Qt::AlignHCenter | Qt::AlignTop, tr("End of slide show, click to exit."));
        return;
    }
    const QImage now = nowImage();
    if (now.deviceIndependentSize().toSize() != size()) p.fillRect(rect(), Qt::black);
    if (m_animating) {
        paintTransition(&p, m_trans, m_fromOverlay.isNull() ? now : m_fromOverlay, m_toOverlay.isNull() ? imageOf(m_target) : m_toOverlay, m_anim.currentValue().toDouble());
    } else {
        p.drawImage(0, 0, now);
        paintOverlay(&p, m_slide, slideRect());
    }
}

void SlideShow::resizeEvent(QResizeEvent *e)
{
    finishTransition();
    m_nowValid = false;
    QWidget::resizeEvent(e);
}

// ---- keys and mouse ----

void SlideShow::keyPressEvent(QKeyEvent *e)
{
    const int k = e->key();
    // At a kiosk nobody moves the show but its slides' times: Esc is all that is left.
    if (isKiosk()) {
        if (k == Qt::Key_Escape) close();
        return;
    }
    // Ctrl and a letter choose the pointer (the pen is not for a rehearsal).
    if ((e->modifiers() & Qt::ControlModifier) && !(e->modifiers() & Qt::AltModifier)) {
        std::optional<Pointer> chosen;
        switch (k) {
        case Qt::Key_P: chosen = Pointer::Pen; break;
        case Qt::Key_I: chosen = Pointer::Highlighter; break;
        case Qt::Key_L: chosen = Pointer::Laser; break;
        case Qt::Key_E: chosen = Pointer::Eraser; break;
        case Qt::Key_A: chosen = Pointer::Arrow; break;
        default: break;
        }
        if (chosen) {
            m_typed.clear();
            setPointer(*chosen);
            return;
        }
    }
    if (k >= Qt::Key_0 && k <= Qt::Key_9) {
        m_typed += QChar('0' + (k - Qt::Key_0));
        return;
    }
    if ((k == Qt::Key_Return || k == Qt::Key_Enter) && !m_typed.isEmpty()) {
        goTo(m_typed.toInt() - 1);
        m_typed.clear();
        return;
    }
    m_typed.clear();
    switch (k) {
    case Qt::Key_Escape:
        // Esc brings the arrow back first.
        if (m_pointer != Pointer::Arrow) setPointer(Pointer::Arrow);
        else close();
        break;
    case Qt::Key_Right: case Qt::Key_Down: case Qt::Key_Space: case Qt::Key_Return: case Qt::Key_Enter:
    case Qt::Key_PageDown: case Qt::Key_N:
        next();
        break;
    case Qt::Key_Left: case Qt::Key_Up: case Qt::Key_Backspace: case Qt::Key_PageUp: case Qt::Key_P:
        previous();
        break;
    case Qt::Key_Home:
        goToPosition(nextShown(0, 1));
        break;
    case Qt::Key_End:
        goToPosition(nextShown(count() - 1, -1));
        break;
    case Qt::Key_B: case Qt::Key_Period:
        toggleBlank(Qt::black);
        break;
    case Qt::Key_W: case Qt::Key_Comma:
        toggleBlank(Qt::white);
        break;
    case Qt::Key_E:
        eraseInk();
        break;
    default:
        QWidget::keyPressEvent(e);
    }
}

QRectF SlideShow::slideRect() const { return fitSlide(m_doc->pageSize(), size()); }

// A point of the window as a point of the slide, in the slide's own units.
QPointF SlideShow::pageAt(const QPointF &pos) const
{
    const QRectF at = slideRect();
    return at.width() > 0 ? (pos - at.topLeft()) * (m_doc->pageSize().width() / at.width()) : QPointF();
}

// The address of the first object with a link under `pos`, topmost first.
QString SlideShow::linkAt(const QPointF &pos) const
{
    if (m_atEnd || m_animating || m_blank.isValid() || m_slide < 0 || m_slide >= m_doc->pages.size()) return QString();
    const QRectF at = slideRect();
    if (!at.contains(pos)) return QString();
    const QPointF page = (pos - at.topLeft()) * (m_doc->pageSize().width() / at.width());
    // A shape the animations have not shown yet, or have hidden, has no link to click.
    const Play &play = playOf(m_slide);
    const deck::States states = play.timeline.isEmpty() ? deck::States() : deck::statesAt(play.timeline, play.geometry, m_playing >= 0 ? m_playing : m_done, m_playing >= 0 ? m_stepMs : -1);
    const auto find = [&](const auto &self, const ItemList &items) -> QString {
        for (auto it = items.rbegin(); it != items.rend(); ++it) {
            const Item &item = **it;
            if (!item.bounds().contains(page)) continue;
            if (const auto state = states.constFind(deck::Target{item.sourceId, -1, -1}); item.sourceId > 0 && state != states.constEnd() && (!state->visible || state->opacity <= 0)) continue;
            if (item.type() == ItemType::Group)
                if (const QString inside = self(self, static_cast<const GroupItem &>(item).children); !inside.isEmpty()) return inside;
            if (!item.hyperlink.isEmpty()) return item.hyperlink;
        }
        return QString();
    };
    return find(find, m_doc->pages[m_slide]->items);
}

void SlideShow::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        const bool inking = !m_atEnd && !isKiosk() && !m_recording;
        // Ctrl and the button: the laser pointer, while they are held.
        if (inking && (e->modifiers() & Qt::ControlModifier) && m_pointer == Pointer::Arrow) {
            m_tempLaser = true;
            updateCursor();
            moveTo(pageAt(e->position()));
            return;
        }
        if (inking && (m_pointer == Pointer::Pen || m_pointer == Pointer::Highlighter || m_pointer == Pointer::Eraser)) {
            pressAt(pageAt(e->position()));
            return;
        }
        if (const QString link = linkAt(e->position()); !link.isEmpty()) {
            // A page, a mail address; not a file or a program a slide names.
            const QUrl url = QUrl::fromUserInput(link);
            if (url.isValid() && (url.scheme() == QLatin1String("http") || url.scheme() == QLatin1String("https") || url.scheme() == QLatin1String("mailto")))
                QDesktopServices::openUrl(url);
            return;
        }
        if (isKiosk()) return;
        // A slide that doesn't move on at a click leaves that to the keys (its steps still play at one); the end screen always does.
        const int shown = m_animating ? m_target : m_slide;
        const bool stepsLeft = !m_animating && (m_playing >= 0 || m_done < stepCount());
        const Page *shownPage = pageOf(shown);
        if (m_atEnd || stepsLeft || !shownPage || deck::transitionOf(*shownPage).advanceOnClick) next();
    } else if (e->button() == Qt::RightButton) {
        if (isKiosk()) return;
        // The menu; a rehearsal has none, and goes back.
        if (m_recording) {
            previous();
            return;
        }
        QMenu *menu = buildMenu(this);
        menu->setAttribute(Qt::WA_DeleteOnClose);
        menu->popup(e->globalPosition().toPoint());
    }
}

void SlideShow::mouseMoveEvent(QMouseEvent *e)
{
    if (m_drawing || m_erasing || m_pointer == Pointer::Laser || m_tempLaser) {
        moveTo(pageAt(e->position()));
        return;
    }
    if (m_pointer != Pointer::Arrow) return;
    const bool link = !linkAt(e->position()).isEmpty();
    if (link) setCursor(Qt::PointingHandCursor);
    else if (showsCursor()) unsetCursor();
    else setCursor(Qt::BlankCursor);
}

void SlideShow::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) releasePointer();
}

void SlideShow::leaveEvent(QEvent *e)
{
    pointerLeft();
    QWidget::leaveEvent(e);
}

// ---- the pointer and the ink ----

void SlideShow::setPointer(Pointer pointer)
{
    // A kiosk and a rehearsal have no pen.
    if (isKiosk() || m_recording) pointer = Pointer::Arrow;
    if (pointer == m_pointer) return;
    m_pointer = pointer;
    m_drawing = m_erasing = m_tempLaser = false;
    m_laserShown = false;
    updateCursor();
    changeOverlay();
}

QColor SlideShow::inkColor() const { return m_pointer == Pointer::Highlighter ? m_highlighterColor : m_penColor; }

void SlideShow::setInkColor(const QColor &color)
{
    if (!color.isValid()) return;
    const QColor c(color.red(), color.green(), color.blue());
    (m_pointer == Pointer::Highlighter ? m_highlighterColor : m_penColor) = c;
    updateCursor();
}

const QVector<deck::InkStroke> &SlideShow::ink(int slide) const
{
    static const QVector<deck::InkStroke> none;
    const auto it = m_ink.constFind(idOf(slide));
    return it == m_ink.constEnd() ? none : it.value();
}

int SlideShow::inkCount() const
{
    int n = 0;
    for (const auto &strokes : m_ink) n += int(strokes.size());
    return n;
}

void SlideShow::eraseInk(int slide)
{
    if (slide < 0) slide = m_slide;
    if (m_recording || isKiosk() || !m_ink.remove(idOf(slide))) return;
    ++m_inkRevision;
    m_drawing = m_erasing = false;
    changeOverlay();
}

void SlideShow::pressAt(const QPointF &page)
{
    if (m_atEnd || m_animating || !pageOf(m_slide) || isKiosk() || m_recording) return;
    const Pointer pointer = m_tempLaser ? Pointer::Laser : m_pointer;
    const double width = m_doc->pageSize().width();
    if (pointer == Pointer::Pen || pointer == Pointer::Highlighter) {
        deck::InkStroke stroke;
        stroke.points << page;
        stroke.highlighter = pointer == Pointer::Highlighter;
        stroke.color = stroke.highlighter ? m_highlighterColor : m_penColor;
        stroke.width = stroke.highlighter ? deck::highlighterWeightFor(width) : deck::penWeightFor(width);
        m_ink[idOf(m_slide)].append(stroke);
        m_drawing = true;
        const double r = stroke.width;
        changeOverlay(QRectF(page, page).adjusted(-r, -r, r, r));
    } else if (pointer == Pointer::Eraser) {
        m_erasing = true;
        erasePoint(page);
    } else if (pointer == Pointer::Laser) {
        moveTo(page);
    }
}

void SlideShow::moveTo(const QPointF &page)
{
    if (m_atEnd || m_animating) return;
    const double width = m_doc->pageSize().width();
    if (m_drawing) {
        auto it = m_ink.find(idOf(m_slide));
        if (it == m_ink.end() || it->isEmpty() || it->last().points.isEmpty()) return;
        deck::InkStroke &stroke = it->last();
        const QPointF last = stroke.points.last();
        // Moves too small to see add nothing, and a stroke has only so many points.
        if (QLineF(last, page).length() < 0.25 * width / deck::kInkSlideWidth || stroke.points.size() >= 100000) return;
        stroke.points << page;
        const double r = stroke.width;
        changeOverlay(QRectF(last, page).normalized().adjusted(-r, -r, r, r));
    } else if (m_erasing) {
        erasePoint(page);
    } else if (m_pointer == Pointer::Laser || m_tempLaser) {
        const double r = 20 * width / deck::kInkSlideWidth;
        QRectF dirty(m_laser - QPointF(r, r), QSizeF(2 * r, 2 * r));
        if (m_laserShown) dirty = dirty.united(QRectF(page - QPointF(r, r), QSizeF(2 * r, 2 * r)));
        else dirty = QRectF(page - QPointF(r, r), QSizeF(2 * r, 2 * r));
        m_laser = page;
        m_laserShown = true;
        changeOverlay(dirty);
    }
}

void SlideShow::erasePoint(const QPointF &page)
{
    auto it = m_ink.find(idOf(m_slide));
    if (it == m_ink.end()) return;
    const double reach = deck::eraserReachFor(m_doc->pageSize().width());
    bool any = false;
    for (int i = int(it->size()) - 1; i >= 0; --i)
        if (deck::inkTouches((*it)[i], page, reach)) {
            it->removeAt(i);
            any = true;
        }
    if (it->isEmpty()) m_ink.erase(it);
    if (any) {
        ++m_inkRevision;
        changeOverlay();
    }
}

void SlideShow::releasePointer()
{
    m_drawing = m_erasing = false;
    if (m_tempLaser) {
        m_tempLaser = false;
        m_laserShown = false;
        updateCursor();
        changeOverlay();
    }
}

void SlideShow::pointerLeft()
{
    if (!m_laserShown) return;
    m_laserShown = false;
    changeOverlay();
}

// The window repaints what the ink or the laser changed (the whole slide when no part is named, in slide points), and Presenter View follows.
void SlideShow::changeOverlay(const QRectF &page)
{
    const QRectF at = slideRect();
    if (page.isNull() || at.width() <= 0) {
        update();
    } else {
        const double scale = at.width() / m_doc->pageSize().width();
        update(QRectF(at.topLeft() + page.topLeft() * scale, page.size() * scale).toAlignedRect().adjusted(-2, -2, 2, 2));
    }
    Q_EMIT overlayChanged();
}

// The cursor for the pointer: a dot in the ink's color for the pen, a square for the highlighter, a ring for the eraser; none for the
// laser pointer, which is drawn on the slide; the arrow hidden, or shown in a window.
void SlideShow::updateCursor()
{
    const Pointer pointer = m_tempLaser ? Pointer::Laser : m_pointer;
    if (pointer == Pointer::Arrow || pointer == Pointer::Laser) {
        // The arrow shows in a window and while the times are recorded (the bar is clicked with it); the laser pointer is drawn instead.
        if (showsCursor() && pointer == Pointer::Arrow) unsetCursor();
        else setCursor(Qt::BlankCursor);
        return;
    }
    const qreal dpr = devicePixelRatioF();
    const int px = 28;
    QPixmap pix(QSize(qRound(px * dpr), qRound(px * dpr)));
    pix.setDevicePixelRatio(dpr);
    pix.fill(Qt::transparent);
    {
        QPainter p(&pix);
        p.setRenderHint(QPainter::Antialiasing);
        const QPointF c(px / 2.0, px / 2.0);
        if (pointer == Pointer::Eraser) {
            p.setBrush(QColor(255, 255, 255, 90));
            p.setPen(QPen(Qt::black, 1.5));
            p.drawEllipse(c, 9, 9);
            p.setPen(QPen(Qt::white, 0.75));
            p.drawEllipse(c, 7.6, 7.6);
        } else if (pointer == Pointer::Highlighter) {
            QColor fill = m_highlighterColor;
            fill.setAlpha(130);
            p.setPen(QPen(QColor(0, 0, 0, 120), 1));
            p.setBrush(fill);
            p.drawRect(QRectF(c.x() - 4, c.y() - 9, 8, 18));
        } else {
            p.setPen(QPen(QColor(255, 255, 255, 200), 1.5));
            p.setBrush(m_penColor);
            p.drawEllipse(c, 4.5, 4.5);
        }
    }
    setCursor(QCursor(pix, px / 2, px / 2));
}

// Ink, and the laser pointer's dot, over a slide drawn to fill `at` (a window of the show's, or Presenter View's).
void SlideShow::paintOverlay(QPainter *p, int slide, const QRectF &at) const { paintInk(p, slide, at, true); }

namespace {
// A slide with at least this many points in its finished strokes (a pen stroke has some hundreds) is drawn from a picture of them.
constexpr qint64 kInkPictureAt = 5000;
// The pictures kept: each is as large as the slide's place on a screen.
constexpr int kInkPictures = 4;

void drawStrokes(QPainter *p, const QVector<deck::InkStroke> &strokes, int from, int to)
{
    p->setBrush(Qt::NoBrush);
    for (int i = from; i < to; ++i) {
        const deck::InkStroke &stroke = strokes[i];
        QColor c = stroke.color;
        if (stroke.highlighter) c.setAlphaF(1 - deck::kHighlighterClearness);
        p->setPen(QPen(c, stroke.width, Qt::SolidLine, stroke.highlighter ? Qt::SquareCap : Qt::RoundCap, Qt::RoundJoin));
        p->drawPath(deck::inkPath(stroke));
    }
}
} // namespace

void SlideShow::paintInk(QPainter *p, int slide, const QRectF &at, bool laser) const
{
    const QString id = idOf(slide);
    const auto it = m_ink.constFind(id);
    const bool ink = it != m_ink.constEnd() && !it->isEmpty();
    const bool dot = laser && m_laserShown && slide == m_slide && !m_animating && !m_atEnd && !m_blank.isValid() && (m_pointer == Pointer::Laser || m_tempLaser);
    const double width = m_doc->pageSize().width();
    if ((!ink && !dot) || width <= 0 || at.isEmpty()) return;
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    p->setClipRect(at);
    // The strokes that are finished come from a picture, made once and added to as strokes finish (and made again when ink is taken
    // away); the one being drawn, and a slide with little ink, are drawn straight.
    int straight = 0;
    if (ink) {
        const QVector<deck::InkStroke> &strokes = *it;
        const int finished = int(strokes.size()) - (m_drawing && slide == m_slide ? 1 : 0);
        qint64 points = 0;
        for (int i = 0; i < finished && points < kInkPictureAt; ++i) points += strokes[i].points.size();
        if (points >= kInkPictureAt) {
            const qreal dpr = p->device()->devicePixelRatioF();
            const QSize px(std::max(1, qRound(at.width() * dpr)), std::max(1, qRound(at.height() * dpr)));
            const QString key = id + QLatin1Char('|') + QString::number(px.width()) + QLatin1Char('x') + QString::number(px.height());
            m_inkLayerOrder.removeAll(key);
            m_inkLayerOrder << key;
            while (m_inkLayerOrder.size() > kInkPictures) m_inkLayers.remove(m_inkLayerOrder.takeFirst());
            InkLayer &layer = m_inkLayers[key];
            if (layer.image.size() != px || layer.revision != m_inkRevision || layer.drawn > finished) {
                if (layer.image.size() != px) {
                    layer.image = QImage(px, QImage::Format_ARGB32_Premultiplied);
                    layer.image.setDevicePixelRatio(dpr);
                }
                layer.image.fill(Qt::transparent);
                layer.drawn = 0;
                layer.revision = m_inkRevision;
            }
            if (layer.drawn < finished) {
                QPainter lp(&layer.image);
                lp.setRenderHint(QPainter::Antialiasing);
                const double scale = (px.width() / dpr) / width;
                lp.scale(scale, scale);
                drawStrokes(&lp, strokes, layer.drawn, finished);
                layer.drawn = finished;
            }
            p->drawImage(QRectF(at.topLeft(), QSizeF(px) / dpr), layer.image);
            straight = finished;
        }
    }
    p->translate(at.topLeft());
    const double scale = at.width() / width;
    p->scale(scale, scale);
    if (ink) drawStrokes(p, *it, straight, int(it->size()));
    if (dot) {
        const double r = 7 * width / deck::kInkSlideWidth;
        QColor core = m_settings.laserColor, glow = core;
        glow.setAlpha(0);
        QRadialGradient g(m_laser, r * 2.6);
        g.setColorAt(0, core);
        g.setColorAt(0.38, core);
        QColor halo = core;
        halo.setAlpha(120);
        g.setColorAt(0.42, halo);
        g.setColorAt(1, glow);
        p->setPen(Qt::NoPen);
        p->setBrush(g);
        p->drawEllipse(m_laser, r * 2.6, r * 2.6);
    }
    p->restore();
}

// An image of a slide with its ink on it, as a transition plays it; the image itself when the slide has none.
QImage SlideShow::withOverlay(const QImage &image, int slide) const
{
    const auto it = m_ink.constFind(idOf(slide));
    if (image.isNull() || it == m_ink.constEnd() || it->isEmpty()) return image;
    QImage copy = image;
    QPainter p(&copy);
    paintInk(&p, slide, slideRectIn(m_doc->pageSize(), image.deviceIndependentSize()), false);
    return copy;
}

// ---- the menu a right click opens ----

namespace {
// A small square of a color, for the lists of colors.
QIcon swatch(const QColor &c)
{
    QPixmap pix(16, 16);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor(0, 0, 0, 90), 1));
    p.setBrush(c);
    p.drawRoundedRect(QRectF(1.5, 1.5, 13, 13), 2, 2);
    return QIcon(pix);
}
}

QMenu *SlideShow::pointerMenu(QWidget *parent)
{
    auto *menu = new QMenu(tr("Pointer Options"), parent);
    menu->setObjectName(QStringLiteral("pointerMenu"));
    auto *group = new QActionGroup(menu);
    const struct { Pointer pointer; const char *id; QString text; QKeySequence key; } kinds[] = {
        {Pointer::Arrow, "pointerArrow", tr("Arrow"), QKeySequence(Qt::CTRL | Qt::Key_A)},
        {Pointer::Laser, "pointerLaser", tr("Laser Pointer"), QKeySequence(Qt::CTRL | Qt::Key_L)},
        {Pointer::Pen, "pointerPen", tr("Pen"), QKeySequence(Qt::CTRL | Qt::Key_P)},
        {Pointer::Highlighter, "pointerHighlighter", tr("Highlighter"), QKeySequence(Qt::CTRL | Qt::Key_I)},
        {Pointer::Eraser, "pointerEraser", tr("Eraser"), QKeySequence(Qt::CTRL | Qt::Key_E)},
    };
    for (const auto &k : kinds) {
        // The key is shown beside the name; the show reads it itself, so the menu must not hold it as a shortcut.
        QAction *a = menu->addAction(k.text + QLatin1Char('\t') + k.key.toString(QKeySequence::NativeText));
        a->setObjectName(QLatin1String(k.id));
        a->setCheckable(true);
        a->setActionGroup(group);
        a->setChecked(m_pointer == k.pointer);
        const Pointer pointer = k.pointer;
        connect(a, &QAction::triggered, this, [this, pointer] { setPointer(pointer); });
    }
    menu->addSeparator();
    QMenu *colors = menu->addMenu(tr("Ink Color"));
    colors->setObjectName(QStringLiteral("inkColorMenu"));
    auto *colorGroup = new QActionGroup(colors);
    for (const deck::InkColor &c : deck::inkColors()) {
        QAction *a = colors->addAction(swatch(c.color), c.name);
        a->setCheckable(true);
        a->setActionGroup(colorGroup);
        a->setData(c.color);
        a->setChecked(inkColor() == c.color);
        const QColor color = c.color;
        connect(a, &QAction::triggered, this, [this, color] { setInkColor(color); });
    }
    QAction *erase = menu->addAction(tr("Erase All Ink on Slide") + QLatin1Char('\t') + QKeySequence(Qt::Key_E).toString(QKeySequence::NativeText));
    erase->setObjectName(QStringLiteral("eraseAllInk"));
    erase->setEnabled(!ink(m_slide).isEmpty());
    connect(erase, &QAction::triggered, this, [this] { eraseInk(); });
    return menu;
}

QMenu *SlideShow::buildMenu(QWidget *parent)
{
    auto *menu = new QMenu(parent);
    menu->setObjectName(QStringLiteral("showMenu"));
    QAction *nextAction = menu->addAction(tr("Next"));
    nextAction->setObjectName(QStringLiteral("menuNext"));
    connect(nextAction, &QAction::triggered, this, [this] { next(); });
    QAction *previousAction = menu->addAction(tr("Previous"));
    previousAction->setObjectName(QStringLiteral("menuPrevious"));
    connect(previousAction, &QAction::triggered, this, [this] { previous(); });
    QMenu *go = menu->addMenu(tr("Go to Slide"));
    go->setObjectName(QStringLiteral("goToSlideMenu"));
    // One for each place the show has (a custom show may list a slide twice), the one it is at checked; an ampersand in a title is not a key.
    const int here = m_animating ? m_targetPos : m_pos;
    for (int pos = 0; pos < count(); ++pos) {
        const Page *page = pageOf(slideAt(pos));
        if (!page || (!m_hiddenToo && page->hidden)) continue;
        const int slide = slideAt(pos);
        QAction *a = go->addAction(QStringLiteral("%1  %2").arg(slide + 1).arg(deck::slideTitle(*m_doc, slide).replace(QLatin1Char('&'), QStringLiteral("&&"))));
        a->setCheckable(true);
        a->setChecked(!m_atEnd && pos == here);
        connect(a, &QAction::triggered, this, [this, pos] { goToPosition(pos); });
    }
    menu->addSeparator();
    menu->addMenu(pointerMenu(menu));
    QMenu *screen = menu->addMenu(tr("Screen"));
    screen->setObjectName(QStringLiteral("screenMenu"));
    QAction *black = screen->addAction(tr("Black Screen"));
    black->setCheckable(true);
    black->setChecked(m_blank == QColor(Qt::black));
    connect(black, &QAction::triggered, this, [this] { toggleBlank(Qt::black); });
    QAction *white = screen->addAction(tr("White Screen"));
    white->setCheckable(true);
    white->setChecked(m_blank == QColor(Qt::white));
    connect(white, &QAction::triggered, this, [this] { toggleBlank(Qt::white); });
    menu->addSeparator();
    QAction *end = menu->addAction(tr("End Show"));
    end->setObjectName(QStringLiteral("menuEnd"));
    connect(end, &QAction::triggered, this, [this] { close(); });
    return menu;
}

// ---- rehearsing ----

void SlideShow::setRecording(bool on)
{
    m_recording = on;
    m_pointer = on ? Pointer::Arrow : m_pointer;
    updateCursor();   // the arrow shows while the bar is used
    delete m_bar;
    m_bar = nullptr;
    m_recorded.clear();
    m_recPaused = false;
    m_recId = on && !m_atEnd ? idOf(m_slide) : QString();
    m_recBase = 0;
    m_recSince = clockNow();
    if (on) {
        m_bar = new RehearseBar(this);
        m_bar->move(12, 12);
        m_bar->show();
        m_bar->raise();
    }
}

qint64 SlideShow::slideClock() const
{
    return m_recId.isEmpty() ? 0 : m_recBase + (m_recPaused ? 0 : std::max<qint64>(0, clockNow() - m_recSince));
}

qint64 SlideShow::totalClock() const
{
    qint64 total = slideClock();
    for (auto it = m_recorded.cbegin(); it != m_recorded.cend(); ++it)
        if (it.key() != m_recId) total += it.value();
    return total;
}

// The seconds of each slide the show is on or was, by the slide's number now; a slide that is gone from the presentation has none.
QMap<int, double> SlideShow::recordedSeconds() const
{
    const QHash<QString, int> pages = deck::pageIndexById(*m_doc);
    QMap<int, double> out;
    for (auto it = m_recorded.cbegin(); it != m_recorded.cend(); ++it)
        if (const auto at = pages.constFind(it.key()); at != pages.constEnd()) out.insert(at.value(), it.value() / 1000.0);
    if (!m_recId.isEmpty() && slideClock() > 0)
        if (const auto at = pages.constFind(m_recId); at != pages.constEnd()) out.insert(at.value(), slideClock() / 1000.0);
    return out;
}

double SlideShow::recordedTotal() const { return totalClock() / 1000.0; }

// The slide on the screen is the one being timed: the one before keeps its time.
void SlideShow::timeSlide(int slide)
{
    const QString id = idOf(slide);
    if (!m_recording || id == m_recId) return;
    // A slide left at once has no time to record, and keeps the one it had.
    if (!m_recId.isEmpty() && slideClock() > 0) m_recorded[m_recId] = slideClock();
    m_recId = id;
    m_recBase = 0;
    m_recSince = clockNow();
}

void SlideShow::pauseRecording(bool paused)
{
    if (!m_recording || paused == m_recPaused) return;
    if (paused) m_recBase = slideClock();
    else m_recSince = clockNow();
    m_recPaused = paused;
}

void SlideShow::repeatSlide()
{
    if (!m_recording || m_recId.isEmpty()) return;
    m_recBase = 0;
    m_recSince = clockNow();
}

void SlideShow::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    if (m_started) return;
    m_started = true;
    // The time on the first slide counts from when it is on the screen.
    if (m_recording && !m_recId.isEmpty()) {
        m_recBase = 0;
        m_recSince = clockNow();
    }
    startAutomaticSteps();
    startAutoAdvance();
}

void SlideShow::closeEvent(QCloseEvent *e)
{
    m_anim.stop();
    m_frame.stop();
    m_auto.stop();
    // The slide on the screen has its time.
    timeSlide(-1);
    m_drawing = m_erasing = false;
    Q_EMIT ended(m_atEnd ? m_slide : targetSlide());
    QWidget::closeEvent(e);
}

// ---- two screens ----

void SlideShow::setVisible(bool visible)
{
    if (visible && !m_placed) {
        m_placed = true;
        place();
    }
    QWidget::setVisible(visible);
}

// Where the show and Presenter View go, the first time the show is shown.
void SlideShow::place()
{
    QScreen *mine = QGuiApplication::screenAt(geometry().center());
    if (!mine) mine = screen();
    if (m_rehearsal) {
        // The show small at a corner, Presenter View over the rest.
        const QRect area = mine->availableGeometry();
        const int width = std::max(320, area.width() / 3);
        setGeometry(QRect(area.right() - width - 12, area.top() + 12, width, width * 9 / 16));
        openPresenterView(mine);
        return;
    }
    if (QGuiApplication::screens().size() < 2 || !usePresenterView() || m_settings.type != deck::ShowSettings::Type::Speaker || m_recording) return;
    // The other screen is the audience's: the primary one when the window is not on it.
    QScreen *other = mine == QGuiApplication::primaryScreen() ? nullptr : QGuiApplication::primaryScreen();
    for (QScreen *s : QGuiApplication::screens())
        if (!other && s != mine) other = s;
    if (!other) return;
    setScreen(other);
    setGeometry(other->geometry());
    openPresenterView(mine);
}

void SlideShow::openPresenterView(QScreen *on)
{
    m_presenter = new PresenterView(this);
    const QRect area = on->availableGeometry();
    if (m_rehearsal) {
        const int right = geometry().left() - 12;
        m_presenter->setGeometry(QRect(area.left() + 12, area.top() + 12, std::max(640, right - area.left() - 12), area.height() - 24));
    } else {
        m_presenter->setScreen(on);
        m_presenter->setGeometry(area);
    }
    m_presenter->show();
}

// ---- the ribbon's preview ----

TransitionPreview::TransitionPreview(const Document *doc, LayoutCache *cache, QWidget *parent)
    : QWidget(parent), m_doc(doc), m_cache(cache)
{
    setObjectName(QStringLiteral("transitionPreview"));
    setAccessibleName(tr("Transition Preview"));
    setAttribute(Qt::WA_OpaquePaintEvent);
    setFocusPolicy(Qt::StrongFocus);
    hide();
    m_anim.setStartValue(0.0);
    m_anim.setEndValue(1.0);
    m_anim.setEasingCurve(QEasingCurve::InOutQuad);
    connect(&m_anim, &QVariantAnimation::valueChanged, this, [this] { update(); });
    connect(&m_anim, &QVariantAnimation::finished, this, &TransitionPreview::stop);
}

void TransitionPreview::play(int slide)
{
    if (!m_doc || slide < 0 || slide >= m_doc->pages.size() || !parentWidget()) return;
    m_trans = deck::transitionOf(*m_doc->pages[slide]);
    if (m_trans.type == QLatin1String("none")) return;
    setGeometry(parentWidget()->rect());
    const QRectF at = fitSlide(m_doc->pageSize(), QSizeF(size()) * 0.94);
    m_area = QRectF(at.translated(QPointF(size().width() * 0.03, size().height() * 0.03))).toAlignedRect();
    const qreal dpr = devicePixelRatioF();
    // As the show has them: the slide at its start, and the one before with its steps played.
    auto drawn = [&](int index, bool played) {
        const deck::Timeline tl = index >= 0 ? deck::playableTimeline(*m_doc->pages[index]) : deck::Timeline();
        if (tl.isEmpty()) return renderSlideImage(m_doc, m_cache, index, m_area.size(), dpr);
        const deck::States states = deck::statesAt(tl, deck::geometryOf(*m_doc, *m_doc->pages[index]), played ? int(tl.steps.size()) : 0);
        return renderSlideImage(m_doc, m_cache, index, m_area.size(), dpr, Qt::black, &states);
    };
    m_to = drawn(slide, false);
    m_from = drawn(slide - 1, true);
    m_anim.stop();
    m_anim.setDuration(std::max(1, qRound(m_trans.duration * 1000)));
    show();
    raise();
    setFocus();
    m_anim.start();
}

void TransitionPreview::stop()
{
    m_anim.stop();
    hide();
}

void TransitionPreview::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) stop();
    else QWidget::keyPressEvent(e);
}

void TransitionPreview::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), QColor(0x3A, 0x3A, 0x3A));
    if (m_to.isNull()) return;
    p.translate(m_area.topLeft());
    p.setClipRect(QRectF(QPointF(0, 0), QSizeF(m_area.size())));
    paintTransition(&p, m_trans, m_from, m_to, m_anim.currentValue().toDouble());
}

} // namespace jp
