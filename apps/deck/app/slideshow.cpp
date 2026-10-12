#include "app/slideshow.h"

#include "app/presenterview.h"
#include "app/settings.h"
#include "core/document.h"
#include "render/renderer.h"

#include <QCloseEvent>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QPainter>
#include <QScreen>
#include <QSet>
#include <QUrl>

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

QImage renderSlideImage(const Document *doc, LayoutCache *cache, int slide, const QSize &size, qreal dpr, const QColor &bars, const deck::States *states)
{
    const QSize px(std::max(1, qRound(size.width() * dpr)), std::max(1, qRound(size.height() * dpr)));
    // Drawn with gray text smoothing; a screen image has no use for the
    // colored fringes of subpixel smoothing, and the projector may not be this screen.
    QImage img(px, QImage::Format_ARGB32_Premultiplied);
    img.setDevicePixelRatio(dpr);
    img.fill(bars);
    if (slide >= 0 && slide < doc->pages.size()) {
        QPainter p(&img);
        const QSizeF page = doc->pageSize();
        const QRectF at = fitSlide(page, QSizeF(px) / dpr);
        const double scale = at.width() / page.width();
        p.translate(at.topLeft());
        p.scale(scale, scale);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::TextAntialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        p.setClipRect(QRectF(QPointF(0, 0), page));
        p.fillRect(QRectF(QPointF(0, 0), page), Qt::white);
        PaintContext ctx;
        ctx.doc = doc;
        ctx.cache = cache;
        if (states) deck::animate(ctx, *doc, *states);
        Renderer::paintPage(&p, ctx, slide);
    }
    // Opaque, so that painting it on the screen is a copy.
    QImage opaque = img.convertToFormat(QImage::Format_RGB32);
    opaque.setDevicePixelRatio(dpr);
    return opaque;
}

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
    m_slide = nextShown(std::clamp(startSlide, 0, std::max(0, int(doc->pages.size()) - 1)), 1);
    if (m_slide < 0) m_atEnd = true;
    m_target = m_slide;
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

int SlideShow::nextShown(int from, int step) const
{
    for (int i = from; i >= 0 && i < m_doc->pages.size(); i += step)
        if (!m_doc->pages[i]->hidden) return i;
    return -1;
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
    const int n = nextShown(m_slide + 1, 1);
    if (n < 0) {
        m_atEnd = true;
        update();
        Q_EMIT changed();
        return;
    }
    const deck::Transition t = deck::transitionOf(*m_doc->pages[n]);
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
        showSlide(m_slide, stepCount());
    } else if (m_playing >= 0) {
        // The effect stops, and its step is not played.
        m_frame.stop();
        m_done = m_playing;
        m_playing = -1;
        stepChanged();
    } else if (m_done > 0) {
        --m_done;
        stepChanged();
    } else if (const int p = nextShown(m_slide - 1, -1); p >= 0) {
        showSlide(p, stepsOf(p));
    } else {
        showSlide(m_slide);
    }
}

void SlideShow::goTo(int slide)
{
    if (slide < 0 || slide >= m_doc->pages.size()) return;
    finishTransition();
    m_auto.stop();
    m_blank = QColor();
    m_atEnd = false;
    showSlide(slide);
}

// A slide on the screen with `steps` of its steps played.
void SlideShow::showSlide(int slide, int steps)
{
    m_frame.stop();
    m_slide = m_target = slide;
    m_done = std::min(steps, stepsOf(slide));
    m_playing = -1;
    m_nowValid = false;
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

void SlideShow::startTransition(int slide, const deck::Transition &t)
{
    // Both images are ready before the first frame.
    nowImage();
    imageOf(slide);
    m_target = slide;
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
    m_slide = m_target;
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
    if (!m_started || m_atEnd || m_animating || m_playing >= 0) return;
    if (const auto after = deck::transitionOf(*m_doc->pages[m_slide]).advanceAfter) m_auto.start(std::max(0, qRound(*after * 1000)));
}

// ---- the slide's animations ----

const SlideShow::Play &SlideShow::playOf(int slide) const
{
    static const Play none;
    if (slide < 0 || slide >= m_doc->pages.size()) return none;
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
    return View{slideAfter(now.slide), 0};
}

QImage SlideShow::renderView(const View &view, double ms, const QSize &size, qreal dpr, const QColor &bars)
{
    const Play &play = playOf(view.slide);
    if (play.timeline.isEmpty()) return renderSlideImage(m_doc, m_cache, view.slide, size, dpr, bars);
    const deck::States states = deck::statesAt(play.timeline, play.geometry, view.steps, ms);
    return renderSlideImage(m_doc, m_cache, view.slide, size, dpr, bars, &states);
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
}

// The steps played changed: draw the slide again, count the seconds from here, and tell Presenter View.
void SlideShow::stepChanged()
{
    m_nowValid = false;
    update();
    startAutoAdvance();
    Q_EMIT changed();
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
    const int ahead = nextShown(m_slide + 1, 1), behind = nextShown(m_slide - 1, -1);
    QSet<int> keep{m_slide, ahead, behind};
    if (m_animating) keep.insert(m_target);
    for (auto it = m_images.begin(); it != m_images.end();) it = keep.contains(it.key()) ? std::next(it) : m_images.erase(it);
    for (const int slide : {ahead, behind}) {
        if (slide < 0 || m_images.contains(slide)) continue;
        imageOf(slide);
        QTimer::singleShot(0, this, &SlideShow::prefetch);
        return;
    }
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
    if (m_animating) paintTransition(&p, m_trans, now, imageOf(m_target), m_anim.currentValue().toDouble());
    else p.drawImage(0, 0, now);
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
        close();
        break;
    case Qt::Key_Right: case Qt::Key_Down: case Qt::Key_Space: case Qt::Key_Return: case Qt::Key_Enter:
    case Qt::Key_PageDown: case Qt::Key_N:
        next();
        break;
    case Qt::Key_Left: case Qt::Key_Up: case Qt::Key_Backspace: case Qt::Key_PageUp: case Qt::Key_P:
        previous();
        break;
    case Qt::Key_Home:
        goTo(nextShown(0, 1));
        break;
    case Qt::Key_End:
        goTo(nextShown(int(m_doc->pages.size()) - 1, -1));
        break;
    case Qt::Key_B: case Qt::Key_Period:
        toggleBlank(Qt::black);
        break;
    case Qt::Key_W: case Qt::Key_Comma:
        toggleBlank(Qt::white);
        break;
    default:
        QWidget::keyPressEvent(e);
    }
}

QRectF SlideShow::slideRect() const { return fitSlide(m_doc->pageSize(), size()); }

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
        if (const QString link = linkAt(e->position()); !link.isEmpty()) {
            // A page, a mail address; not a file or a program a slide names.
            const QUrl url = QUrl::fromUserInput(link);
            if (url.isValid() && (url.scheme() == QLatin1String("http") || url.scheme() == QLatin1String("https") || url.scheme() == QLatin1String("mailto")))
                QDesktopServices::openUrl(url);
            return;
        }
        // A slide that doesn't move on at a click leaves that to the keys (its steps still play at one); the end screen always does.
        const int shown = m_animating ? m_target : m_slide;
        const bool stepsLeft = !m_animating && (m_playing >= 0 || m_done < stepCount());
        if (m_atEnd || stepsLeft || deck::transitionOf(*m_doc->pages[shown]).advanceOnClick) next();
    } else if (e->button() == Qt::RightButton) {
        previous();
    }
}

void SlideShow::mouseMoveEvent(QMouseEvent *e)
{
    if (m_rehearsal) return;
    setCursor(linkAt(e->position()).isEmpty() ? Qt::BlankCursor : Qt::PointingHandCursor);
}

void SlideShow::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    if (m_started) return;
    m_started = true;
    startAutomaticSteps();
    startAutoAdvance();
}

void SlideShow::closeEvent(QCloseEvent *e)
{
    m_anim.stop();
    m_frame.stop();
    m_auto.stop();
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
    if (QGuiApplication::screens().size() < 2 || !usePresenterView()) return;
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
    m_to = renderSlideImage(m_doc, m_cache, slide, m_area.size(), dpr);
    m_from = slide > 0 ? renderSlideImage(m_doc, m_cache, slide - 1, m_area.size(), dpr) : renderSlideImage(m_doc, m_cache, -1, m_area.size(), dpr);
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
