#include "app/slideshow.h"

#include "app/icons.h"
#include "app/morph.h"
#include "app/presenterview.h"
#include "app/rehearsebar.h"
#include "app/settings.h"
#include "app/showmedia.h"
#include "core/media.h"
#include "core/document.h"
#include "core/reuse.h"
#include "render/renderer.h"

#include <QAction>
#include <QActionGroup>
#include <QCloseEvent>
#include <QCursor>
#include <QDesktopServices>
#include <QDir>
#include <QRegularExpression>
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>
#include <QToolTip>
#include "app/plaintext.h"
#include "core/linkareas.h"
#include "io/presentationfile.h"
#include "office/hyperlink.h"
#include "office/package.h"
#include <QGuiApplication>
#include <QKeyEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QRandomGenerator>
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
    QString by;         // a wheel's spokes, a shape's form
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

// ---- the common effects of the older programs: bands, cells, and shapes the new slide shows through

// A random number from a place and a seed, so that a frame is the same each time it is drawn.
quint32 noise(quint32 a, quint32 seed)
{
    quint32 h = a * 0x9E3779B1u + seed * 0x85EBCA6Bu;
    h ^= h >> 15;
    h *= 0x2C1B3C6Du;
    h ^= h >> 12;
    h *= 0x297A2D39u;
    h ^= h >> 15;
    return h;
}

// The part of an image in a rectangle (in whole device pixels, it is a plain copy).
void drawPart(const Frame &f, const QImage &image, const QRectF &area)
{
    const QRectF r = area & QRectF(QPointF(0, 0), f.size);
    if (r.width() <= 0 || r.height() <= 0) return;
    f.p->drawImage(r, image, QRectF(r.topLeft() * f.dpr, r.size() * f.dpr));
}

// Draws the new slide in a shape over the old one.
void drawThrough(const Frame &f, const QPainterPath &shape)
{
    f.p->drawImage(QPointF(0, 0), f.from);
    f.p->setRenderHint(QPainter::Antialiasing);
    f.p->setClipPath(shape, Qt::IntersectClip);
    f.p->drawImage(QPointF(0, 0), f.to);
}

constexpr int kBlinds = 6, kCombStrips = 8, kRandomBars = 40, kCheckerColumns = 8, kStripColumns = 12;

// Whether the effect's bands run across (their length is the width) for the orientation: "h" bands are horizontal.
bool horizontal(const Frame &f) { return f.dir != QLatin1String("v"); }

// Slats that open from one edge of each band.
void drawBlinds(const Frame &f)
{
    const bool h = horizontal(f);
    const double length = h ? f.size.height() : f.size.width(), w = f.size.width(), ht = f.size.height();
    f.p->drawImage(QPointF(0, 0), f.from);
    for (int i = 0; i < kBlinds; ++i) {
        const double a = snapped(f, length * i / kBlinds), b = snapped(f, length * (i + 1) / kBlinds), open = snapped(f, (b - a) * f.t);
        drawPart(f, f.to, h ? QRectF(0, a, w, open) : QRectF(a, 0, open, ht));
    }
}

// Strips that slide in from alternate sides.
void drawComb(const Frame &f)
{
    const bool h = horizontal(f);
    const double across = h ? f.size.height() : f.size.width(), w = f.size.width(), ht = f.size.height();
    f.p->drawImage(QPointF(0, 0), f.from);
    for (int i = 0; i < kCombStrips; ++i) {
        const double a = snapped(f, across * i / kCombStrips), b = snapped(f, across * (i + 1) / kCombStrips);
        const double len = snapped(f, (h ? w : ht) * f.t), full = h ? w : ht;
        const double start = i % 2 ? full - len : 0;
        drawPart(f, f.to, h ? QRectF(start, a, len, b - a) : QRectF(a, start, b - a, len));
    }
}

// Squares in a checkerboard that fill with the new slide, one color of them and then the other.
void drawCheckerboard(const Frame &f)
{
    const bool h = horizontal(f);
    const double w = f.size.width(), ht = f.size.height(), side = w / kCheckerColumns;
    const int rows = std::max(1, int(std::ceil(ht / side - 1e-9)));
    f.p->drawImage(QPointF(0, 0), f.from);
    for (int j = 0; j < rows; ++j)
        for (int i = 0; i < kCheckerColumns; ++i) {
            const bool first = (i + j) % 2 == 0;
            const double share = std::clamp(first ? f.t * 2 : f.t * 2 - 1, 0.0, 1.0);
            const double x0 = snapped(f, side * i), x1 = snapped(f, side * (i + 1)), y0 = snapped(f, side * j), y1 = snapped(f, std::min(ht, side * (j + 1)));
            drawPart(f, f.to, h ? QRectF(x0, y0, snapped(f, (x1 - x0) * share), y1 - y0) : QRectF(x0, y0, x1 - x0, snapped(f, (y1 - y0) * share)));
        }
}

// Thin bars of the new slide that appear one by one in an order the seed gives.
void drawRandomBars(const Frame &f)
{
    const bool h = horizontal(f);
    const double length = h ? f.size.height() : f.size.width();
    f.p->drawImage(QPointF(0, 0), f.from);
    for (int i = 0; i < kRandomBars; ++i) {
        if (f.t <= (noise(i, 7) >> 8) / double(1 << 24)) continue;
        const double a = snapped(f, length * i / kRandomBars), b = snapped(f, length * (i + 1) / kRandomBars);
        drawPart(f, f.to, h ? QRectF(0, a, f.size.width(), b - a) : QRectF(a, 0, b - a, f.size.height()));
    }
}

// The new slide shows through a circle that grows from the middle.
void drawCircle(const Frame &f)
{
    const double r = f.t * std::hypot(f.size.width(), f.size.height()) / 2;
    QPainterPath shape;
    shape.addEllipse(QPointF(f.size.width() / 2, f.size.height() / 2), r, r);
    drawThrough(f, shape);
}

// ... through a diamond.
void drawDiamond(const Frame &f)
{
    const double cx = f.size.width() / 2, cy = f.size.height() / 2, rx = f.size.width() * f.t, ry = f.size.height() * f.t;
    QPainterPath shape;
    shape.addPolygon(QPolygonF({QPointF(cx - rx, cy), QPointF(cx, cy - ry), QPointF(cx + rx, cy), QPointF(cx, cy + ry)}));
    drawThrough(f, shape);
}

// ... through a plus whose arms thicken until they fill the slide.
void drawPlus(const Frame &f)
{
    const double w = f.size.width(), h = f.size.height();
    f.p->drawImage(QPointF(0, 0), f.from);
    const double armH = snapped(f, h * f.t), armW = snapped(f, w * f.t);
    drawPart(f, f.to, QRectF(0, snapped(f, (h - armH) / 2), w, armH));
    drawPart(f, f.to, QRectF(snapped(f, (w - armW) / 2), 0, armW, h));
}

// A pie whose slices begin at an angle (Qt's: degrees counterclockwise from 3 o'clock) and sweep by it.
void addSlice(QPainterPath &path, const QPointF &c, double radius, double start, double sweep)
{
    path.moveTo(c);
    path.arcTo(QRectF(c.x() - radius, c.y() - radius, 2 * radius, 2 * radius), start, sweep);
    path.closeSubpath();
}

// ... through a wedge that opens from the top, on both sides.
void drawWedge(const Frame &f)
{
    const QPointF c(f.size.width() / 2, f.size.height() / 2);
    QPainterPath shape;
    addSlice(shape, c, std::hypot(f.size.width(), f.size.height()), 90 - 180 * f.t, 360 * f.t);
    drawThrough(f, shape);
}

// ... through spokes that each sweep clockwise from the top.
void drawWheel(const Frame &f)
{
    const QPointF c(f.size.width() / 2, f.size.height() / 2);
    const int spokes = std::clamp(f.by.toInt(), 1, 8);
    QPainterPath shape;
    for (int k = 0; k < spokes; ++k) addSlice(shape, c, std::hypot(f.size.width(), f.size.height()), 90 - 360.0 * k / spokes, -360.0 / spokes * f.t);
    drawThrough(f, shape);
}

// The shape's form is the option.
void drawShape(const Frame &f)
{
    if (f.by == QLatin1String("diamond")) drawDiamond(f);
    else if (f.by == QLatin1String("plus")) drawPlus(f);
    else drawCircle(f);
}

// A flash of white: the old slide goes to white, and the new one comes out of it.
void drawFlash(const Frame &f)
{
    f.p->drawImage(QPointF(0, 0), f.t < 0.5 ? f.from : f.to);
    f.p->setOpacity(f.t < 0.5 ? f.t * 2 : (1 - f.t) * 2);
    f.p->fillRect(QRectF(QPointF(0, 0), f.size), Qt::white);
}

// The new slide spins in from small over the old one.
void drawNewsflash(const Frame &f)
{
    f.p->drawImage(QPointF(0, 0), f.from);
    f.p->setRenderHint(QPainter::SmoothPixmapTransform);
    f.p->translate(f.size.width() / 2, f.size.height() / 2);
    f.p->rotate(540 * (1 - f.t));
    f.p->scale(f.t, f.t);
    f.p->drawImage(QPointF(-f.size.width() / 2, -f.size.height() / 2), f.to);
}

// Diagonal strips, a staircase of cells that fills from a corner. The direction is the way it travels, so it starts at the corner behind it.
void drawStrips(const Frame &f)
{
    const double w = f.size.width(), h = f.size.height(), side = w / kStripColumns;
    const int rows = std::max(1, int(std::ceil(h / side - 1e-9))), steps = kStripColumns + rows - 1;
    const bool fromRight = f.dir.startsWith(QLatin1Char('l')), fromBottom = f.dir.endsWith(QLatin1Char('u'));
    f.p->drawImage(QPointF(0, 0), f.from);
    const double reach = f.t * steps;   // the diagonals whose cells are in
    for (int j = 0; j < rows; ++j) {
        const int row = fromBottom ? rows - 1 - j : j;
        const int cells = std::clamp(int(std::floor(reach - row)), 0, kStripColumns);
        if (cells == 0) continue;
        const double y0 = snapped(f, side * j), y1 = snapped(f, std::min(h, side * (j + 1)));
        const double len = cells == kStripColumns ? w : snapped(f, side * cells);
        drawPart(f, f.to, QRectF(fromRight ? w - len : 0, y0, len, y1 - y0));
    }
}

// A fine, even dissolve: blocks of pixels change one by one in an order the seed gives.
void drawDissolve(const Frame &f)
{
    using Px = quint32;
    const auto fits = [](const QImage &i) { return i.format() == QImage::Format_ARGB32_Premultiplied || i.format() == QImage::Format_RGB32; };
    if (!fits(f.from) || !fits(f.to) || f.from.size() != f.to.size()) {
        drawFade(f);
        return;
    }
    static thread_local QImage scratch;
    if (scratch.size() != f.from.size() || scratch.format() != QImage::Format_ARGB32_Premultiplied) scratch = QImage(f.from.size(), QImage::Format_ARGB32_Premultiplied);
    scratch.setDevicePixelRatio(f.from.devicePixelRatio());
    const quint32 limit = quint32(f.t * double(1 << 24));
    constexpr int kBlock = 4;
    const int w = f.from.width(), h = f.from.height(), blocks = (w + kBlock - 1) / kBlock;
    for (int y = 0; y < h; ++y) {
        const Px *a = reinterpret_cast<const Px *>(f.from.constScanLine(y)), *b = reinterpret_cast<const Px *>(f.to.constScanLine(y));
        Px *out = reinterpret_cast<Px *>(scratch.scanLine(y));
        const quint32 row = quint32(y / kBlock) * quint32(blocks);
        for (int bx = 0, x = 0; bx < blocks; ++bx) {
            const Px *src = (noise(row + bx, 11) >> 8) < limit ? b : a;
            for (const int end = std::min(w, x + kBlock); x < end; ++x) out[x] = src[x] | 0xFF000000u * (f.from.format() == QImage::Format_RGB32);
        }
    }
    f.p->drawImage(QPointF(0, 0), scratch);
}

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
        ctx.opt.hidden = hidden | deck::mediaHiddenWhenStopped(*doc->pages[slide]);   // a movie or sound that shows only while it plays
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
        {QStringLiteral("fade"), drawFade},     {QStringLiteral("morph"), drawFade}, {QStringLiteral("push"), drawPush},   {QStringLiteral("wipe"), drawWipe},
        {QStringLiteral("split"), drawSplit},   {QStringLiteral("reveal"), drawReveal}, {QStringLiteral("cover"), drawCover},
        {QStringLiteral("uncover"), drawUncover}, {QStringLiteral("zoom"), drawZoom}, {QStringLiteral("cut"), drawCut},
        {QStringLiteral("dissolve"), drawDissolve}, {QStringLiteral("blinds"), drawBlinds}, {QStringLiteral("checkerboard"), drawCheckerboard},
        {QStringLiteral("comb"), drawComb}, {QStringLiteral("randombars"), drawRandomBars}, {QStringLiteral("circle"), drawCircle},
        {QStringLiteral("diamond"), drawDiamond}, {QStringLiteral("plus"), drawPlus}, {QStringLiteral("wedge"), drawWedge},
        {QStringLiteral("wheel"), drawWheel}, {QStringLiteral("shape"), drawShape}, {QStringLiteral("flash"), drawFlash},
        {QStringLiteral("newsflash"), drawNewsflash}, {QStringLiteral("strips"), drawStrips},
    };
    // A random one is the same effect each time a frame is drawn; the show chooses before it plays.
    const Transition tr = t.resolved(0);
    const Frame f{p, from, to, std::clamp(progress, 0.0, 1.0), tr.dir, tr.by, from.deviceIndependentSize(), from.devicePixelRatio()};
    p->save();
    // The two ends are the two slides, whatever the effect.
    if (f.t >= 1 && tr.type != QLatin1String("none") && !tr.isMorph()) p->drawImage(QPointF(0, 0), to);
    else if (f.t <= 0 && tr.type != QLatin1String("none") && !tr.isMorph() && tr.type != QLatin1String("cut")) p->drawImage(QPointF(0, 0), from);
    else draws.value(tr.type, drawNone)(f);
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
    m_media = new ShowMedia(doc, this);
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
    m_toastTimer.setSingleShot(true);
    connect(&m_toastTimer, &QTimer::timeout, this, [this] {
        m_toast.clear();
        update();
    });
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
    m_morph.reset();
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
    if (gone) {
        m_atEnd = true;
        m_slide = m_target = -1;
        m_pos = m_targetPos = 0;
        timeSlide(-1);
        update();
    } else {
        m_targetPos = m_pos;
        m_slide = m_target = slideAt(m_pos);
        timeSlide(m_slide);   // (the time goes to the slide that is on the screen now)
        settle();
    }
    // The times of slides that are gone are gone.
    for (auto it = m_recorded.begin(); it != m_recorded.end();) it = index.contains(it.key()) ? std::next(it) : m_recorded.erase(it);
    Q_EMIT changed();
    Q_EMIT slidesChanged();
}

void SlideShow::rebind(const Document *doc)
{
    m_doc = doc;
    m_media->rebind(doc);
    m_rebound = true;
    m_anim.stop();
    m_animating = false;
    m_fromOverlay = m_toOverlay = QImage();
    m_morph.reset();
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

QString SlideShow::showOn() { return Settings::get().value(QStringLiteral("show/showOn")).toString(); }

void SlideShow::setShowOn(const QString &name)
{
    Settings::get().setValue(QStringLiteral("show/showOn"), name);
}

void SlideShow::editNotes(const QString &pageId, const QString &text) { Q_EMIT notesEdited(pageId, text); }

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
    // A step of only a call to a movie or sound is a click of its own.
    if (!m_atEnd && m_media->takeClick(m_slide, m_done)) {
        m_auto.stop();
        m_blank = QColor();
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
    if (n < 0 && !m_returns.isEmpty()) {
        returnFromCustomShow();
        return;
    }
    if (n < 0 && m_settings.looping()) n = nextShown(0, 1);
    if (n < 0) {
        m_atEnd = true;
        m_media->end();
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
    m_media->leave();
    m_frame.stop();
    // A stroke being drawn stays on the slide it began on.
    m_drawing = m_erasing = false;
    m_pos = m_targetPos = pos;
    m_focusIndex = -1;
    m_hoverItem = nullptr;
    m_hoverPrompted = false;
    if (m_slide != slideAt(pos) && m_slide >= 0) m_lastViewed = m_slide;
    m_slide = m_target = slideAt(pos);
    m_areasSlide = -1;
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
    m_media->leave();
    // A morph moves the slides' objects: it needs no pictures of them.
    m_morph = t.isMorph() ? morphFor(slide, t) : nullptr;
    m_fromOverlay = m_morph ? QImage() : withOverlay(nowImage(), m_slide);
    m_toOverlay = m_morph ? QImage() : withOverlay(imageOf(slide), slide);
    m_drawing = m_erasing = false;
    m_targetPos = pos;
    m_target = slide;
    timeSlide(slide);
    m_trans = t.resolved(QRandomGenerator::global()->generate());
    m_animating = true;
    m_painted = -1;
    m_anim.stop();
    m_anim.setDuration(std::max(1, qRound(t.duration * 1000)));
    m_anim.start();
    update();
    Q_EMIT changed();
}

// The morph from the slide on the screen, as it is (its steps played, or the one that plays), to a slide at its start.
std::shared_ptr<MorphFrames> SlideShow::morphFor(int to, const deck::Transition &t)
{
    // Copies: asking for the second slide's animations can move the first's in their table.
    const Play now = playOf(m_slide), next = playOf(to);
    const deck::States before = now.timeline.isEmpty() ? deck::States()
                                                       : deck::statesAt(now.timeline, now.geometry, m_playing >= 0 ? m_playing : m_done, m_playing >= 0 ? m_stepMs : -1);
    const deck::States after = next.timeline.isEmpty() ? deck::States() : deck::statesAt(next.timeline, next.geometry, 0);
    return std::make_shared<MorphFrames>(m_doc, m_slide, to, t.by, size(), devicePixelRatioF(), Qt::black, &before, &after);
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
    m_morph.reset();
    m_drawing = m_erasing = false;
    m_focusIndex = -1;
    m_hoverItem = nullptr;
    m_hoverPrompted = false;
    if (m_slide != m_target && m_slide >= 0) m_lastViewed = m_slide;
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
    m_media->arrive(m_slide, m_done);
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
        play.timeline = deck::playableTimeline(*m_doc, *m_doc->pages[slide]);
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
    m_media->stepStarted(m_slide, step);
    m_focusIndex = -1;
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
    m_focusIndex = -1;
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
    m_focusIndex = -1;
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
    if (m_animating && m_morph) {
        const double progress = m_anim.currentValue().toDouble();
        m_morph->paint(&p, progress);
        // The ink each slide holds fades out and in with it.
        p.setOpacity(1 - progress);
        paintInk(&p, m_slide, slideRect(), false);
        p.setOpacity(progress);
        paintInk(&p, m_target, slideRect(), false);
        p.setOpacity(1);
    } else if (m_animating) {
        paintTransition(&p, m_trans, m_fromOverlay.isNull() ? now : m_fromOverlay, m_toOverlay.isNull() ? imageOf(m_target) : m_toOverlay, m_anim.currentValue().toDouble());
    } else {
        p.drawImage(0, 0, now);
        m_media->paint(&p);
        paintOverlay(&p, m_slide, slideRect());
        // The link chosen with Tab has a ring round it.
        if (m_focusSlide == m_slide && m_focusIndex >= 0) {
            const QVector<const deck::LinkArea *> areas = tabAreas();
            if (m_focusIndex < areas.size()) {
                const QRectF at = slideRect();
                const double k = at.width() / m_doc->pageSize().width();
                const QRectF r = areas[m_focusIndex]->rect;
                // Two outlines, a dark one outside a light one, so that the ring shows on any slide.
                const QRectF ring(at.topLeft() + r.topLeft() * k, r.size() * k);
                p.setBrush(Qt::NoBrush);
                p.setPen(QPen(QColor(0x10, 0x10, 0x10), 3));
                p.drawRect(ring.adjusted(-3, -3, 3, 3));
                p.setPen(QPen(Qt::white, 2));
                p.drawRect(ring.adjusted(-1, -1, 1, 1));
            }
        }
        if (!m_toast.isEmpty()) {
            QFont f = font();
            f.setPointSizeF(14);
            p.setFont(f);
            const QRect box = QFontMetrics(f).boundingRect(rect().adjusted(40, 0, -40, 0), Qt::AlignCenter | Qt::TextWordWrap, m_toast).adjusted(-16, -8, 16, 8);
            const QRect at = QRect(QPoint((width() - box.width()) / 2, height() - box.height() - 36), box.size());
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(0, 0, 0, 200));
            p.drawRoundedRect(at, 8, 8);
            p.setPen(Qt::white);
            p.drawText(at, Qt::AlignCenter | Qt::TextWordWrap, m_toast);
        }
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
    m_hoverHold = false;
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
    if ((k == Qt::Key_Return || k == Qt::Key_Enter) && !focusedLink().isEmpty()) {
        followLink(focusedLink());
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

// The areas of the slide on the screen are worked out once (with the layout the drawing made), not at every move of the mouse.
const QVector<deck::LinkArea> &SlideShow::areasNow() const
{
    if (m_areasSlide != m_slide) {
        m_areas = deck::linkAreas(*m_doc, *m_cache, m_slide, RenderOptions(), true);
        m_areasSlide = m_slide;
    }
    return m_areas;
}

deck::States SlideShow::linkStates() const
{
    const Play &play = playOf(m_slide);
    return play.timeline.isEmpty() ? deck::States() : deck::statesAt(play.timeline, play.geometry, m_playing >= 0 ? m_playing : m_done, m_playing >= 0 ? m_stepMs : -1);
}

// A shape the animations have not shown yet, or have hidden, has no link to click.
static bool areaShown(const deck::LinkArea &area, const deck::States &states)
{
    const Item &item = *area.item;
    const auto state = states.constFind(deck::Target{item.sourceId, -1, -1});
    return !(item.sourceId > 0 && state != states.constEnd() && (!state->visible || state->opacity <= 0));
}

// The link of the topmost object or line of text with one under `pos`.
QString SlideShow::linkAt(const QPointF &pos) const
{
    if (m_atEnd || m_animating || m_blank.isValid() || m_slide < 0 || m_slide >= m_doc->pages.size()) return QString();
    const QRectF at = slideRect();
    if (!at.contains(pos)) return QString();
    const QPointF page = (pos - at.topLeft()) * (m_doc->pageSize().width() / at.width());
    const deck::States states = linkStates();
    const QVector<deck::LinkArea> &areas = areasNow();
    for (auto it = areas.crbegin(); it != areas.crend(); ++it)
        if (!it->hover && it->rect.contains(page) && areaShown(*it, states)) return it->link;
    return QString();
}

// The mouse-over action of the topmost object under `pos` that has one.
const deck::LinkArea *SlideShow::hoverAreaAt(const QPointF &pos) const
{
    if (m_atEnd || m_animating || m_blank.isValid() || m_slide < 0 || m_slide >= m_doc->pages.size()) return nullptr;
    const QRectF at = slideRect();
    if (!at.contains(pos)) return nullptr;
    const QPointF page = (pos - at.topLeft()) * (m_doc->pageSize().width() / at.width());
    const deck::States states = linkStates();
    const QVector<deck::LinkArea> &areas = areasNow();
    for (auto it = areas.crbegin(); it != areas.crend(); ++it)
        if (it->hover && it->rect.contains(page) && areaShown(*it, states)) return &*it;
    return nullptr;
}

// What Tab goes through: the links on the screen, bottom object first, a link on several lines of words once.
QVector<const deck::LinkArea *> SlideShow::tabAreas() const
{
    QVector<const deck::LinkArea *> out;
    if (m_atEnd || m_animating || m_blank.isValid() || m_slide < 0 || m_slide >= m_doc->pages.size()) return out;
    const deck::States states = linkStates();
    for (const deck::LinkArea &a : areasNow()) {
        if (a.hover || !areaShown(a, states)) continue;
        if (!out.isEmpty() && out.last()->item == a.item && out.last()->link == a.link) continue;
        out << &a;
    }
    return out;
}

QString SlideShow::focusedLink() const
{
    if (m_focusSlide != m_slide || m_focusIndex < 0) return QString();
    const QVector<const deck::LinkArea *> areas = tabAreas();
    return m_focusIndex < areas.size() ? areas[m_focusIndex]->link : QString();
}

// Tab and Shift+Tab go from link to link on the slide, round at the ends; Esc and the other keys are as they were.
bool SlideShow::focusNextPrevChild(bool next)
{
    if (isKiosk() || m_recording) return true;
    const QVector<const deck::LinkArea *> areas = tabAreas();
    if (areas.isEmpty()) return true;
    const int now = m_focusSlide == m_slide ? m_focusIndex : -1, n = int(areas.size());
    m_focusSlide = m_slide;
    m_focusIndex = next ? (now + 1) % n : now <= 0 ? n - 1 : now - 1;
    update();
    return true;
}

void SlideShow::setLinkPrompt(std::function<bool(const QString &address, bool remote)> ask) { m_linkPrompt = std::move(ask); }
void SlideShow::setLinkOpener(std::function<void(const QUrl &)> open) { m_linkOpener = std::move(open); }
void SlideShow::setLinkNotice(std::function<void(const QString &)> say) { m_linkNotice = std::move(say); }

// What the tip over the link at `pos` says: the address of a web or file link, else nothing.
QString SlideShow::tipAt(const QPointF &pos) const { return office::linkAddress(linkAt(pos)); }

// What a click on a link does: a jump moves the show, the end ends it, and a web address, mail address, or file is opened only when the person says yes.
// A file on another computer is asked about in its own words and never looked for first.
// The path a link to another presentation names, in full: a "file:" address as the path it is, a relative one counted from `base` (the folder of the presentation playing; on
// another computer it stays on that computer).
static QString resolvedPresentationPath(const QString &address, const QString &base)
{
    QString path = address.trimmed();
    if (path.startsWith(QLatin1String("file:"), Qt::CaseInsensitive)) {
        if (const QString local = QUrl(path).toLocalFile(); !local.isEmpty()) path = local;
        else return path;   // (a file address with a host: it is judged as it stands)
    }
    if (path.startsWith(QLatin1String("\\\\")) || path.startsWith(QLatin1String("//")) || QRegularExpression(QStringLiteral("^[A-Za-z]:[\\\\/]")).match(path).hasMatch() || QDir::isAbsolutePath(path)) return path;
    if (base.isEmpty()) return path;
    const bool share = base.startsWith(QLatin1String("\\\\")) || base.startsWith(QLatin1String("//"));
    if (!share) return QDir(base).filePath(path);
    const QChar sep = base.contains(QLatin1Char('\\')) && !base.contains(QLatin1Char('/')) ? QLatin1Char('\\') : QLatin1Char('/');
    return base + (base.endsWith(QLatin1Char('/')) || base.endsWith(QLatin1Char('\\')) ? QString() : QString(sep)) + path;
}

void SlideShow::followLink(const QString &link)
{
    const office::LinkTarget t = office::parseLink(link);
    switch (t.kind) {
    case office::LinkKind::Next: next(); return;
    case office::LinkKind::Previous: previous(); return;
    case office::LinkKind::First: goToPosition(0); return;
    case office::LinkKind::Last: goToPosition(count() - 1); return;
    case office::LinkKind::Slide: goTo(t.slide - 1); return;
    case office::LinkKind::EndShow: close(); return;
    case office::LinkKind::LastViewed:
        if (m_lastViewed >= 0 && placeOf(m_lastViewed) >= 0) goTo(m_lastViewed);
        else say(tr("There is no earlier slide to go back to."));
        return;
    case office::LinkKind::CustomShow: playCustomShow(t.slide, t.back); return;
    case office::LinkKind::None: return;
    default: break;
    }
    // Another presentation is judged by the path it is at, counted from the folder of this one: "evil.exe" and "sub/a.exe" are as much programs as a whole path is.
    const QString target = t.kind == office::LinkKind::Deck ? resolvedPresentationPath(t.address, m_baseDir) : t.address;
    const bool isFile = t.kind == office::LinkKind::File || t.kind == office::LinkKind::Deck;
    const bool remote = false;   // (a file on another computer is refused, never asked about)
    QString local;
    if (isFile) {
        // A file or another presentation is opened only when the text of its link alone says it is a whole path on this computer of a document or picture type (another presentation: of a
        // presentation type), as JeffWrite decides (office::isOpenableFileLink): a path on another computer, a relative path, a hidden character, a program, and any other ending are refused,
        // and nothing is looked at (no file is touched before the question, and none for the refusal).
        static const QStringList presentationEndings = {QStringLiteral("pptx"), QStringLiteral("ppt"), QStringLiteral("ppsx"), QStringLiteral("pps"), QStringLiteral("odp")};
        if (!office::isOpenableFileLink(target, &local, t.kind == office::LinkKind::Deck ? &presentationEndings : nullptr)) {
            const QString text = office::isProgramName(target) ? tr("This link opens a program, so it was not opened.") : tr("This link goes to a file that is not opened from a link, so it was not opened.");
            if (m_linkNotice) m_linkNotice(text);
            else jp::plainWarning(this, tr("Open Link"), text);
            return;
        }
    }
    QUrl url;
    if (isFile) {
        url = QUrl::fromLocalFile(local);   // (for another presentation only to ask with: it is read by its path)
    } else {
        url = QUrl::fromUserInput(t.address);
        if (!url.isValid()) return;
    }
    const QString shown = target;
    bool yes = false;
    if (m_linkPrompt) yes = m_linkPrompt(shown, remote);
    else {
        jp::MessageBox box(QMessageBox::Question, tr("Open Link"), remote ? tr("This link goes to a file on another computer. Open it?") : tr("Open this link?"), QMessageBox::NoButton, this);
        box.setTextFormat(Qt::PlainText);
        box.setInformativeText(shown);
        box.addButton(tr("Open"), QMessageBox::AcceptRole);
        QPushButton *no = box.addButton(QMessageBox::Cancel);
        box.setDefaultButton(no);
        box.exec();
        yes = box.clickedButton() != no && box.result() != QMessageBox::Cancel;
    }
    if (yes) {
        if (t.kind == office::LinkKind::Deck) openPresentation(target);
        else if (m_linkOpener) m_linkOpener(url);
        else QDesktopServices::openUrl(url);
    }
}

// A custom show played from a link: its slides replace the show's, and with `back` the show comes back to the slide it was on when they are played.
void SlideShow::playCustomShow(int id, bool back)
{
    const QVector<deck::CustomShow> shows = deck::customShowsOf(*m_doc);
    const deck::CustomShow *cs = deck::customShowById(shows, id);
    if (!cs) {
        say(tr("That custom show is no longer in this presentation."));
        return;
    }
    QHash<QString, int> index;
    for (int i = 0; i < m_doc->pages.size(); ++i) index.insert(m_doc->pages[i]->id, i);
    QList<int> slides;
    QStringList ids;
    for (const QString &page : cs->slides)
        if (index.contains(page)) {
            slides << index.value(page);
            ids << page;
        }
    if (slides.isEmpty()) {
        say(tr("That custom show has no slides to play."));
        return;
    }
    // Only so many shows wait to be come back to; a link followed inside the show it leads to waits for nothing more.
    constexpr int kMostReturns = 32;
    const bool sameShow = m_order == slides;
    if (back && !sameShow && m_returns.size() >= kMostReturns) {
        say(tr("Too many custom shows are waiting to be returned from."));
        return;
    }
    const Return here{m_order, m_orderIds, m_hiddenToo, m_slide};
    const QList<int> order = m_order;
    const QStringList orderIds = m_orderIds;
    const bool hiddenToo = m_hiddenToo;
    m_order = slides;
    m_orderIds = ids;
    m_hiddenToo = false;
    const int first = nextShown(0, 1);
    if (first < 0) {
        m_order = order;
        m_orderIds = orderIds;
        m_hiddenToo = hiddenToo;
        return;
    }
    if (!back) m_returns.clear();
    else if (!sameShow) m_returns << here;
    finishTransition();
    m_auto.stop();
    m_blank = QColor();
    m_atEnd = false;
    m_images.clear();
    showSlide(first);
}

// At the end of a custom show followed with "and return": the slide it was followed from, in the show it was in.
void SlideShow::returnFromCustomShow()
{
    const Return r = m_returns.takeLast();
    m_order = r.order;
    m_orderIds = r.orderIds;
    m_hiddenToo = r.hiddenToo;
    int pos = positionOf(r.slide);
    if (pos < 0 || !pageOf(slideAt(pos))) pos = nextShown(0, 1);
    m_images.clear();
    if (pos < 0) {
        m_atEnd = true;
        update();
        return;
    }
    showSlide(pos);
}

namespace {
// Holds the presentation a nested show plays (and its layouts); the nested show owns it, so that it lives as long as the show does.
class NestedHolder : public QObject {
public:
    std::unique_ptr<Document> doc;
    LayoutCache cache;
};
} // namespace

// Another presentation played as a show of its own over this one: it is read from its file only when the person has said yes, and when it ends this one goes on where it was.
void SlideShow::openPresentation(const QString &address)
{
    constexpr int kMostDepth = 4;
    if (m_nested) return;
    if (m_depth >= kMostDepth) {
        say(tr("Presentations are nested too deeply to open another."));
        return;
    }
    // `address` is the path in full (see resolvedPresentationPath).
    const QString path = address;
    const QFileInfo info(path);
    const QString canonical = info.canonicalFilePath().isEmpty() ? QDir::cleanPath(path) : info.canonicalFilePath();
    if (m_chain.contains(canonical)) {
        say(tr("%1 is already playing, so it was not opened again.").arg(info.fileName()));
        return;
    }
    if (info.size() > office::kMaxOpenFileBytes) {
        say(tr("The presentation %1 is too large to open.").arg(info.fileName()));
        return;
    }
    QString err;
    auto doc = deck::importAnyPresentationFile(path, &err);
    if (!doc || doc->pages.isEmpty()) {
        const QString text = tr("The presentation %1 can't be opened.").arg(QFileInfo(path).fileName());
        if (m_linkNotice) m_linkNotice(text);
        else jp::plainWarning(this, tr("Open Link"), text);
        return;
    }
    auto *holder = new NestedHolder;
    holder->doc = std::move(doc);
    auto *show = new SlideShow(holder->doc.get(), &holder->cache, 0);
    holder->setParent(show);
    show->m_depth = m_depth + 1;
    show->setBaseDirectory(info.absolutePath());
    show->m_chain = m_chain + QStringList{canonical};
    show->setLinkPrompt(m_linkPrompt);
    show->setLinkOpener(m_linkOpener);
    show->setLinkNotice(m_linkNotice);
    m_nested = show;
    // (If this show goes first, the nested one goes with it.)
    connect(this, &QObject::destroyed, show, [show] { show->close(); });
    connect(show, &QObject::destroyed, this, [this] {
        activateWindow();
        setFocus();
    });
    show->setGeometry(geometry());
    show->setWindowModality(Qt::ApplicationModal);
    if (isFullScreen()) show->showFullScreen();
    else show->show();
    show->activateWindow();
    show->setFocus();
}

void SlideShow::setFilePath(const QString &path)
{
    const QFileInfo info(path);
    m_chain = QStringList{info.canonicalFilePath().isEmpty() ? QDir::cleanPath(path) : info.canonicalFilePath()};
    setBaseDirectory(info.absolutePath());
}

void SlideShow::setBaseDirectory(const QString &dir)
{
    m_baseDir = dir;
    if (m_media) m_media->setBaseDirectory(dir);
}

// A word to the person that a link could not be followed: to the hook when there is one, else on the screen for a few seconds.
void SlideShow::say(const QString &text)
{
    if (m_linkNotice) {
        m_linkNotice(text);
        return;
    }
    m_toast = text;
    m_toastTimer.start(4000);
    update();
}

// A mouse-over action: a question (web, mail, a file, another presentation) is asked once during a visit to a slide, and a jump inside the presentation is followed once and then
// holds until the pointer has left the actions or the person clicks or presses a key, so that moving the mouse can't run the show on.
void SlideShow::followHover(const QString &link)
{
    switch (office::parseLink(link).kind) {
    case office::LinkKind::None: return;
    case office::LinkKind::Web: case office::LinkKind::Mail: case office::LinkKind::File: case office::LinkKind::Deck:
        if (m_hoverPrompted) return;
        m_hoverPrompted = true;
        break;
    default:
        m_hoverHold = true;
        break;
    }
    followLink(link);
}

void SlideShow::mousePressEvent(QMouseEvent *e)
{
    m_hoverHold = false;
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
        if (!m_animating && !m_atEnd && !m_blank.isValid() && m_media->mousePress(e->position())) return;
        if (const QString link = linkAt(e->position()); !link.isEmpty()) {
            followLink(link);
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
    // A mouse-over action is followed once as the pointer comes onto the object, and not again until it has left.
    if (const deck::LinkArea *over = hoverAreaAt(e->position())) {
        if (!isKiosk() && !m_hoverHold && m_hoverItem != over->item) {
            m_hoverItem = over->item;
            followHover(over->link);
            return;
        }
    } else {
        m_hoverItem = nullptr;
        m_hoverHold = false;
    }
    if (!m_animating && !m_atEnd && !m_blank.isValid() && m_media->mouseMove(e->position())) {
        setCursor(Qt::ArrowCursor);
        return;
    }
    const QString link = linkAt(e->position());
    if (!link.isEmpty()) {
        setCursor(Qt::PointingHandCursor);
        // A web or file link shows where it goes.
        if (const QString address = office::linkAddress(link); !address.isEmpty()) QToolTip::showText(e->globalPosition().toPoint(), jp::plainToolTip(address), this);
        else QToolTip::hideText();
    } else {
        QToolTip::hideText();
        if (showsCursor()) unsetCursor();
        else setCursor(Qt::BlankCursor);
    }
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

thread_local qint64 g_inkPointsDrawn = 0;

void drawStrokes(QPainter *p, const QVector<deck::InkStroke> &strokes, int from, int to)
{
    p->setBrush(Qt::NoBrush);
    for (int i = from; i < to; ++i) {
        const deck::InkStroke &stroke = strokes[i];
        g_inkPointsDrawn += stroke.points.size();
        QColor c = stroke.color;
        if (stroke.highlighter) c.setAlphaF(1 - deck::kHighlighterClearness);
        p->setPen(QPen(c, stroke.width, Qt::SolidLine, stroke.highlighter ? Qt::SquareCap : Qt::RoundCap, Qt::RoundJoin));
        p->drawPath(deck::inkPath(stroke));
    }
}
} // namespace

qint64 SlideShow::inkPointsDrawnForTest() { return g_inkPointsDrawn; }

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
    m_media->askAboutLinks(this);
    m_media->arrive(m_slide, m_done);
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
    if (m_nested) m_nested->close();
    m_media->end();
    m_anim.stop();
    m_frame.stop();
    m_auto.stop();
    // The slide on the screen has its time.
    timeSlide(-1);
    m_drawing = m_erasing = false;
    if (m_presenter) m_presenter->commitNotes();
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

QString ScreenInfo::label() const { return QStringLiteral("%1 (%2 x %3)").arg(name).arg(geometry.width()).arg(geometry.height()); }

QString screenName(const QString &name, int position) { return name.isEmpty() ? QCoreApplication::translate("Show", "Screen %1").arg(position) : name; }

QVector<ScreenInfo> screensNow()
{
    QVector<ScreenInfo> out;
    for (QScreen *s : QGuiApplication::screens()) out << ScreenInfo{screenName(s->name(), int(out.size()) + 1), s->geometry(), s->availableGeometry(), s == QGuiApplication::primaryScreen()};
    return out;
}

ScreenChoice chooseScreens(const QVector<ScreenInfo> &screens, const QString &setting, int windowScreen)
{
    const int n = int(screens.size());
    if (n == 0) return {};
    if (n == 1) return {0, -1};
    int primary = 0;
    for (int i = 0; i < n; ++i)
        if (screens.at(i).primary) {
            primary = i;
            break;
        }
    const int window = windowScreen >= 0 && windowScreen < n ? windowScreen : primary;
    int chosen = -1;
    if (!setting.isEmpty())
        for (int i = 0; i < n && chosen < 0; ++i)
            if (screens.at(i).name == setting) chosen = i;
    // Any screen but `not`, the main one when it will do.
    auto other = [&](int skip) { return primary != skip ? primary : (skip == 0 ? 1 : 0); };
    ScreenChoice c;
    if (chosen >= 0) c.audience = chosen;
    else c.audience = window != primary ? primary : (primary == 0 ? 1 : 0);
    c.presenter = window != c.audience ? window : other(c.audience);
    return c;
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
    if (QGuiApplication::screens().size() < 2 || m_settings.type != deck::ShowSettings::Type::Speaker || m_recording) return;
    const QVector<ScreenInfo> screens = screensNow();
    int window = 0;
    for (int i = 0; i < screens.size(); ++i)
        if (screens.at(i).name == mine->name() && screens.at(i).geometry == mine->geometry()) window = i;
    const ScreenChoice c = chooseScreens(screens, showOn(), window);
    if (c.audience < 0 || c.presenter < 0) return;
    // Without Presenter View, only a chosen screen moves the show.
    if (!usePresenterView()) {
        if (showOn().isEmpty() || screens.at(c.audience).name != showOn()) return;
        placeOn(screens.at(c.audience), ScreenInfo());
        return;
    }
    placeOn(screens.at(c.audience), screens.at(c.presenter));
}


void SlideShow::placeOn(const ScreenInfo &audience, const ScreenInfo &presenter)
{
    if (!m_watchingScreens) {
        m_watchingScreens = true;
        connect(qApp, &QGuiApplication::screenRemoved, this, [this] { reconcileScreens(screensNow()); });
        connect(qApp, &QGuiApplication::screenAdded, this, [this] { reconcileScreens(screensNow()); });
    }
    m_audience = audience;
    m_presenterScreen = presenter;
    applyScreens();
    if (m_presenter) m_presenter->refresh();
}

static QScreen *screenOf(const ScreenInfo &info)
{
    for (QScreen *s : QGuiApplication::screens())
        if (s->name() == info.name && s->geometry() == info.geometry) return s;
    return nullptr;
}

// The windows go where m_audience and m_presenterScreen say, when those screens are the computer's (a place that is not a screen is only kept).
void SlideShow::applyScreens()
{
    if (QScreen *s = screenOf(m_audience)) {
        setScreen(s);   // the screen first: a full-screen window goes full screen again on the new one
        if (isVisible() && isFullScreen()) showFullScreen();
        else setGeometry(m_audience.geometry);
    }
    if (!m_presenterScreen.geometry.isEmpty()) {
        QScreen *s = screenOf(m_presenterScreen);
        if (!m_presenter) {
            openPresenterView(s);   // (on no screen of the computer when the screen is only a place)
            return;
        }
        if (s) {
            m_presenter->setScreen(s);
            m_presenter->setGeometry(m_presenterScreen.available);
        }
    } else if (m_presenter && screenOf(m_audience)) {
        m_presenter->setGeometry(m_audience.available);   // one screen left: Presenter View is a window on it
    }
}

void SlideShow::reconcileScreens(const QVector<ScreenInfo> &live)
{
    if (m_audience.geometry.isEmpty() || live.isEmpty()) return;
    auto at = [&live](const ScreenInfo &info) {
        for (int i = 0; i < live.size(); ++i)
            if (live.at(i).name == info.name && live.at(i).geometry == info.geometry) return i;
        return -1;
    };
    const int audience = at(m_audience), presenter = m_presenterScreen.geometry.isEmpty() ? -2 : at(m_presenterScreen);
    if (audience >= 0 && presenter != -1) {   // both are still there (-2: there is no Presenter View screen)
        if (m_presenter) m_presenter->refresh();
        return;
    }
    if (audience >= 0) {
        // Presenter View's screen is gone: the show stays, and Presenter View takes another screen (the main one when it will do).
        int other = -1;
        for (int i = 0; i < live.size() && other < 0; ++i)
            if (i != audience && live.at(i).primary) other = i;
        for (int i = 0; i < live.size() && other < 0; ++i)
            if (i != audience) other = i;
        m_presenterScreen = other >= 0 ? live.at(other) : ScreenInfo();
        m_audience = live.at(audience);
    } else {
        // The show's screen is gone: Presenter View stays where it is (if it is still there), and the show goes by the usual rule.
        const ScreenChoice c = chooseScreens(live, showOn(), presenter);
        m_audience = live.at(c.audience);
        m_presenterScreen = c.presenter >= 0 ? live.at(c.presenter) : ScreenInfo();
    }
    applyScreens();
    if (m_presenter) m_presenter->refresh();
}

bool SlideShow::canSwapScreens() const
{
    return m_presenter && !m_rehearsal && !m_audience.geometry.isEmpty() && !m_presenterScreen.geometry.isEmpty() && m_audience.geometry != m_presenterScreen.geometry;
}

void SlideShow::swapScreens()
{
    if (!canSwapScreens()) return;
    std::swap(m_audience, m_presenterScreen);
    applyScreens();
}

void SlideShow::openPresenterView(QScreen *on)
{
    m_presenter = new PresenterView(this);
    if (m_rehearsal) {
        const QRect area = on->availableGeometry();
        const int right = geometry().left() - 12;
        m_presenter->setGeometry(QRect(area.left() + 12, area.top() + 12, std::max(640, right - area.left() - 12), area.height() - 24));
    } else {
        if (on) {
            m_presenter->setScreen(on);
            m_presenter->setGeometry(m_presenterScreen.available);
        }
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
    m_trans = deck::transitionOf(*m_doc->pages[slide]).resolved(QRandomGenerator::global()->generate());
    if (m_trans.type == QLatin1String("none")) return;
    setGeometry(parentWidget()->rect());
    const QRectF at = fitSlide(m_doc->pageSize(), QSizeF(size()) * 0.94);
    m_area = QRectF(at.translated(QPointF(size().width() * 0.03, size().height() * 0.03))).toAlignedRect();
    const qreal dpr = devicePixelRatioF();
    // As the show has them: the slide at its start, and the one before with its steps played.
    auto drawn = [&](int index, bool played) {
        const deck::Timeline tl = index >= 0 ? deck::playableTimeline(*m_doc, *m_doc->pages[index]) : deck::Timeline();
        if (tl.isEmpty()) return renderSlideImage(m_doc, m_cache, index, m_area.size(), dpr);
        const deck::States states = deck::statesAt(tl, deck::geometryOf(*m_doc, *m_doc->pages[index]), played ? int(tl.steps.size()) : 0);
        return renderSlideImage(m_doc, m_cache, index, m_area.size(), dpr, Qt::black, &states);
    };
    m_to = drawn(slide, false);
    m_from = drawn(slide - 1, true);
    // A morph plays from the objects of the two slides (the first slide has none before it: it fades in).
    m_morph.reset();
    if (m_trans.isMorph() && slide > 0) {
        auto statesOf = [&](int index, bool played) {
            const deck::Timeline tl = deck::playableTimeline(*m_doc, *m_doc->pages[index]);
            return tl.isEmpty() ? deck::States() : deck::statesAt(tl, deck::geometryOf(*m_doc, *m_doc->pages[index]), played ? int(tl.steps.size()) : 0);
        };
        const deck::States before = statesOf(slide - 1, true), after = statesOf(slide, false);
        m_morph = std::make_shared<MorphFrames>(m_doc, slide - 1, slide, m_trans.by, m_area.size(), dpr, Qt::black, &before, &after);
    }
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
    m_morph.reset();
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
    if (m_morph) m_morph->paint(&p, m_anim.currentValue().toDouble());
    else paintTransition(&p, m_trans, m_from, m_to, m_anim.currentValue().toDouble());
}

} // namespace jp
