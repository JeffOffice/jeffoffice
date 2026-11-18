#include "render/shapes.h"

#include <QCoreApplication>
#include <QHash>
#include <QtMath>
#include <algorithm>
#include <cmath>

namespace jp {

namespace {

using V = const QVector<double> &;

QPainterPath poly(std::initializer_list<QPointF> pts)
{
    QPainterPath p;
    bool first = true;
    for (const QPointF &pt : pts) {
        if (first) p.moveTo(pt); else p.lineTo(pt);
        first = false;
    }
    p.closeSubpath();
    return p;
}

// An action button: the frame of the blank one with a small picture inside it, in the middle third of the button.
QPainterPath actionButton(double w, double h, const char *kind)
{
    QPainterPath p;
    p.addRect(0, 0, w, h);
    const double d = std::min(w, h) * 0.08;
    p.addRect(d, d, w - 2 * d, h - 2 * d);
    const double s = std::min(w, h) * 0.5, x0 = (w - s) / 2, y0 = (h - s) / 2;
    auto pt = [&](double x, double y) { return QPointF(x0 + x * s, y0 + y * s); };
    auto shape = [&](std::initializer_list<QPointF> pts) { p.addPath(poly(pts)); };
    const QString k = QLatin1String(kind);
    if (k == QLatin1String("Home")) {
        shape({pt(0.5, 0), pt(1, 0.45), pt(0, 0.45)});
        shape({pt(0.12, 0.45), pt(0.88, 0.45), pt(0.88, 1), pt(0.12, 1)});
    } else if (k == QLatin1String("Help")) {
        p.addEllipse(QRectF(pt(0.15, 0), pt(0.85, 0.6)));
        shape({pt(0.4, 0.7), pt(0.6, 0.7), pt(0.6, 0.9), pt(0.4, 0.9)});
    } else if (k == QLatin1String("Information")) {
        shape({pt(0.4, 0), pt(0.6, 0), pt(0.6, 0.2), pt(0.4, 0.2)});
        shape({pt(0.4, 0.35), pt(0.6, 0.35), pt(0.6, 1), pt(0.4, 1)});
    } else if (k == QLatin1String("ForwardNext")) {
        shape({pt(0.1, 0), pt(0.95, 0.5), pt(0.1, 1)});
    } else if (k == QLatin1String("BackPrevious")) {
        shape({pt(0.9, 0), pt(0.05, 0.5), pt(0.9, 1)});
    } else if (k == QLatin1String("End")) {
        shape({pt(0, 0), pt(0.75, 0.5), pt(0, 1)});
        shape({pt(0.8, 0), pt(1, 0), pt(1, 1), pt(0.8, 1)});
    } else if (k == QLatin1String("Beginning")) {
        shape({pt(0, 0), pt(0.2, 0), pt(0.2, 1), pt(0, 1)});
        shape({pt(1, 0), pt(0.25, 0.5), pt(1, 1)});
    } else if (k == QLatin1String("Return")) {
        shape({pt(0.1, 0.7), pt(0.1, 0.3), pt(0.7, 0.3), pt(0.7, 0.1), pt(1, 0.4), pt(0.7, 0.7), pt(0.7, 0.5), pt(0.3, 0.5), pt(0.3, 0.7)});
    } else if (k == QLatin1String("Document")) {
        shape({pt(0.15, 0), pt(0.6, 0), pt(0.85, 0.25), pt(0.85, 1), pt(0.15, 1)});
    } else if (k == QLatin1String("Sound")) {
        shape({pt(0, 0.35), pt(0.3, 0.35), pt(0.7, 0.05), pt(0.7, 0.95), pt(0.3, 0.65), pt(0, 0.65)});
    } else if (k == QLatin1String("Movie")) {
        shape({pt(0, 0.2), pt(0.7, 0.2), pt(0.7, 0.8), pt(0, 0.8)});
        shape({pt(0.75, 0.4), pt(1, 0.2), pt(1, 0.8), pt(0.75, 0.6)});
    }
    return p;
}

QPainterPath polyV(const QVector<QPointF> &pts)
{
    QPainterPath p;
    for (int i = 0; i < pts.size(); ++i) {
        if (i == 0) p.moveTo(pts[i]); else p.lineTo(pts[i]);
    }
    p.closeSubpath();
    return p;
}

// Points on a circle of radius 1 (the first at `first` degrees, going clockwise), stretched so that the outer ones fill the box, as the standard draws the regular polygons and the stars
// (their corners at the box's edges, the center not always in its middle). `inner` is the radius of the points between the outer ones.
QPainterPath stretchedPolygon(double w, double h, int n, double firstDeg, double inner)
{
    const double step = 2 * M_PI / n, first = firstDeg * M_PI / 180;
    double x0 = 1e9, x1 = -1e9, y0 = 1e9, y1 = -1e9;
    for (int i = 0; i < n; ++i) {
        x0 = std::min(x0, std::cos(first + i * step));
        x1 = std::max(x1, std::cos(first + i * step));
        y0 = std::min(y0, std::sin(first + i * step));
        y1 = std::max(y1, std::sin(first + i * step));
    }
    const double sx = w / (x1 - x0), sy = h / (y1 - y0);
    const auto at = [&](double r, double ang) { return QPointF((r * std::cos(ang) - x0) * sx, (r * std::sin(ang) - y0) * sy); };
    QVector<QPointF> pts;
    for (int i = 0; i < n; ++i) {
        pts << at(1.0, first + i * step);
        if (inner > 0) pts << at(inner, first + (i + 0.5) * step);
    }
    return polyV(pts);
}

QPainterPath starPath(double w, double h, int n, double inner)
{
    return stretchedPolygon(w, h, n, -90, inner);
}

QPainterPath ngon(double w, double h, int n, double firstDeg = -90)
{
    return stretchedPolygon(w, h, n, firstDeg, 0);
}

double A(V a, int i, double d) { return i < a.size() ? a[i] : d; }
double mn(double w, double h) { return std::min(w, h); }

QPainterPath roundRect(double w, double h, double r)
{
    QPainterPath p;
    r = std::min({r, w / 2, h / 2});
    p.addRoundedRect(QRectF(0, 0, w, h), r, r);
    return p;
}

QRectF insetRect(double w, double h, double fx, double fy) { return QRectF(w * fx, h * fy, w * (1 - 2 * fx), h * (1 - 2 * fy)); }

// Arrow pointing right with shaft thickness t (fraction of h) and head length hl (fraction of min).
QPainterPath arrowRight(double w, double h, double t, double hl)
{
    const double head = std::min(w, hl * mn(w, h));
    const double y0 = h * (0.5 - t / 2), y1 = h * (0.5 + t / 2);
    return poly({{0, y0}, {w - head, y0}, {w - head, 0}, {w, h / 2}, {w - head, h}, {w - head, y1}, {0, y1}});
}

QPainterPath rotated(const QPainterPath &p, double w, double h, double deg, double pw, double ph)
{
    // Rotate a path built in a pw×ph box into the w×h box.
    QTransform t;
    t.translate(w / 2, h / 2);
    t.rotate(deg);
    t.translate(-pw / 2, -ph / 2);
    return t.map(p);
}

// The text rectangle of a block arrow: the shaft, running into the head up to where the head's edge crosses the shaft's side (the arrow pointing right in a w x h box).
QRectF arrowRightText(double w, double h, double t, double hl)
{
    const double head = std::min(w, hl * mn(w, h));
    const double y0 = h * (0.5 - t / 2), y1 = h * (0.5 + t / 2);
    const double x2 = (w - head) + head * (h > 0 ? y0 / (h / 2) : 0);
    return QRectF(0, y0, std::clamp(x2, 0.0, w), y1 - y0);
}

QRectF rotatedRect(const QRectF &r, double w, double h, double deg, double pw, double ph)
{
    QTransform t;
    t.translate(w / 2, h / 2);
    t.rotate(deg);
    t.translate(-pw / 2, -ph / 2);
    return t.mapRect(r);
}

QPainterPath ellipse(double w, double h)
{
    QPainterPath p;
    p.addEllipse(QRectF(0, 0, w, h));
    return p;
}

QPainterPath wavePath(double w, double h, double amp, int waves)
{
    QPainterPath p;
    const double a = amp * h;
    p.moveTo(0, a);
    for (int i = 0; i < waves; ++i) {
        const double x0 = w * i / waves, x1 = w * (i + 1) / waves;
        p.cubicTo(x0 + (x1 - x0) / 3, -a * 0.5, x0 + 2 * (x1 - x0) / 3, a * 2.5, x1, a);
    }
    p.lineTo(w, h - a);
    for (int i = waves - 1; i >= 0; --i) {
        const double x1 = w * i / waves, x0 = w * (i + 1) / waves;
        p.cubicTo(x0 - (x0 - x1) / 3, h - a * 2.5 + a * 0 + a * 0, x0 - 2 * (x0 - x1) / 3, h + a * 0.5, x1, h - a);
    }
    p.closeSubpath();
    return p;
}

// A callout whose pointer is drawn as the standard draws it: the box fills the frame, the pointer's tip lies at (adj1, adj2) of the box's width and height from its center (the
// default is below the box, a fifth of the width to the left of the middle), and it leaves from the side the tip is nearest, 7/12 to 10/12 along it from the far corner.
struct WedgePoints {
    double x1, x2, y1, y2, xl, yl, xt, yt, xr, yr, xb, yb;
};

WedgePoints wedgePoints(double w, double h, double adj1, double adj2)
{
    const double dx = w * adj1, dy = h * adj2, xPos = w / 2 + dx, yPos = h / 2 + dy;
    const double dz = std::abs(dy) - std::abs(dx * h / w);
    WedgePoints q;
    q.x1 = w * (dx > 0 ? 7 : 2) / 12;
    q.x2 = w * (dx > 0 ? 10 : 5) / 12;
    q.y1 = h * (dy > 0 ? 7 : 2) / 12;
    q.y2 = h * (dy > 0 ? 10 : 5) / 12;
    q.xl = dz > 0 || dx > 0 ? 0 : xPos;
    q.xt = dz > 0 ? (dy > 0 ? q.x1 : xPos) : q.x1;
    q.xr = dz > 0 || dx <= 0 ? w : xPos;
    q.xb = dz > 0 ? (dy > 0 ? xPos : q.x1) : q.x1;
    q.yl = dz > 0 || dx > 0 ? q.y1 : yPos;
    q.yt = dz > 0 ? (dy > 0 ? 0 : yPos) : 0;
    q.yr = dz > 0 || dx <= 0 ? q.y1 : yPos;
    q.yb = dz > 0 ? (dy > 0 ? yPos : h) : h;
    return q;
}

QPainterPath wedgeRect(double w, double h, double adj1, double adj2)
{
    const WedgePoints q = wedgePoints(w, h, adj1, adj2);
    return poly({{0, 0}, {q.x1, 0}, {q.xt, q.yt}, {q.x2, 0}, {w, 0}, {w, q.y1}, {q.xr, q.yr}, {w, q.y2}, {w, h}, {q.x2, h}, {q.xb, q.yb}, {q.x1, h}, {0, h}, {0, q.y2}, {q.xl, q.yl}, {0, q.y1}});
}

QPainterPath wedgeRoundRect(double w, double h, double adj1, double adj2, double adj3)
{
    const WedgePoints q = wedgePoints(w, h, adj1, adj2);
    const double u = std::clamp(adj3, 0.0, 0.5) * mn(w, h);
    QPainterPath p;
    p.moveTo(0, u);
    p.arcTo(QRectF(0, 0, 2 * u, 2 * u), 180, -90);
    p.lineTo(q.x1, 0); p.lineTo(q.xt, q.yt); p.lineTo(q.x2, 0); p.lineTo(w - u, 0);
    p.arcTo(QRectF(w - 2 * u, 0, 2 * u, 2 * u), 90, -90);
    p.lineTo(w, q.y1); p.lineTo(q.xr, q.yr); p.lineTo(w, q.y2); p.lineTo(w, h - u);
    p.arcTo(QRectF(w - 2 * u, h - 2 * u, 2 * u, 2 * u), 0, -90);
    p.lineTo(q.x2, h); p.lineTo(q.xb, q.yb); p.lineTo(q.x1, h); p.lineTo(u, h);
    p.arcTo(QRectF(0, h - 2 * u, 2 * u, 2 * u), 270, -90);
    p.lineTo(0, q.y2); p.lineTo(q.xl, q.yl); p.lineTo(0, q.y1);
    p.closeSubpath();
    return p;
}

// The oval's outline runs all the way round except for 22 degrees at the pointer, which is drawn from the two ends of the gap to the tip.
QPainterPath wedgeEllipse(double w, double h, double adj1, double adj2)
{
    const double dx = w * adj1, dy = h * adj2;
    const double start = std::atan2(dy * w, dx * h) + 11 * M_PI / 180, sweep = 2 * M_PI - 22 * M_PI / 180;
    QVector<QPointF> pts;
    constexpr int kSteps = 180;
    for (int i = 0; i <= kSteps; ++i) pts << QPointF(w / 2 + w / 2 * std::cos(start + sweep * i / kSteps), h / 2 + h / 2 * std::sin(start + sweep * i / kSteps));
    pts << QPointF(w / 2 + dx, h / 2 + dy);
    return polyV(pts);
}

} // namespace

static QVector<ShapeDef> build()
{
    QVector<ShapeDef> L;
    auto add = [&](const char *id, const char *name, const char *cat, PathFn fn, QVector<double> defs = {}, QVector<AdjHandle> hs = {},
                   double inset = 0.1, RectFn tr = nullptr, bool open = false) {
        ShapeDef d;
        d.id = QString::fromLatin1(id);
        d.name = QCoreApplication::translate("Render", name);
        d.category = QCoreApplication::translate("Render", cat);
        d.path = fn;
        d.defaults = defs;
        d.handles = hs;
        d.inset = inset;
        d.textRect = tr;
        d.open = open;
        L << d;
    };

    // ---------------- Rectangles ----------------
    add("rect", QT_TRANSLATE_NOOP("Render", "Rectangle"), QT_TRANSLATE_NOOP("Render", "Rectangles"), [](double w, double h, V) { QPainterPath p; p.addRect(0, 0, w, h); return p; }, {}, {}, 0);
    add("roundRect", QT_TRANSLATE_NOOP("Render", "Rounded Rectangle"), QT_TRANSLATE_NOOP("Render", "Rectangles"), [](double w, double h, V a) { return roundRect(w, h, A(a, 0, 0.1667) * mn(w, h)); },
        {0.1667}, {{0, 0, 2, false, 0, 0, 0.5}}, 0.05);
    add("snip1", QT_TRANSLATE_NOOP("Render", "Snip Single Corner Rectangle"), QT_TRANSLATE_NOOP("Render", "Rectangles"), [](double w, double h, V a) {
        const double s = A(a, 0, 0.1667) * mn(w, h);
        return poly({{0, 0}, {w - s, 0}, {w, s}, {w, h}, {0, h}}); }, {0.1667}, {{0, 0, 2, true, 0, 0, 0.5}}, 0.05);
    add("snip2same", QT_TRANSLATE_NOOP("Render", "Snip Same Side Corner Rectangle"), QT_TRANSLATE_NOOP("Render", "Rectangles"), [](double w, double h, V a) {
        const double s = A(a, 0, 0.1667) * mn(w, h);
        return poly({{s, 0}, {w - s, 0}, {w, s}, {w, h}, {0, h}, {0, s}}); }, {0.1667}, {{0, 0, 2, false, 0, 0, 0.5}}, 0.05);
    add("snip2diag", QT_TRANSLATE_NOOP("Render", "Snip Diagonal Corner Rectangle"), QT_TRANSLATE_NOOP("Render", "Rectangles"), [](double w, double h, V a) {
        const double s = A(a, 0, 0.1667) * mn(w, h);
        return poly({{0, 0}, {w - s, 0}, {w, s}, {w, h}, {s, h}, {0, h - s}}); }, {0.1667}, {{0, 0, 2, true, 0, 0, 0.5}}, 0.05);
    add("snipRound", QT_TRANSLATE_NOOP("Render", "Snip and Round Single Corner Rectangle"), QT_TRANSLATE_NOOP("Render", "Rectangles"), [](double w, double h, V a) {
        const double s = A(a, 0, 0.1667) * mn(w, h);
        QPainterPath p; p.moveTo(s, 0); p.lineTo(w - s, 0); p.lineTo(w, s); p.lineTo(w, h); p.lineTo(0, h); p.lineTo(0, s);
        p.quadTo(0, 0, s, 0); p.closeSubpath(); return p; }, {0.1667}, {{0, 0, 2, false, 0, 0, 0.5}}, 0.05);
    add("round1", QT_TRANSLATE_NOOP("Render", "Round Single Corner Rectangle"), QT_TRANSLATE_NOOP("Render", "Rectangles"), [](double w, double h, V a) {
        const double r = A(a, 0, 0.1667) * mn(w, h);
        QPainterPath p; p.moveTo(0, 0); p.lineTo(w - r, 0); p.quadTo(w, 0, w, r); p.lineTo(w, h); p.lineTo(0, h); p.closeSubpath(); return p; },
        {0.1667}, {{0, 0, 2, true, 0, 0, 0.5}}, 0.05);
    add("round2same", QT_TRANSLATE_NOOP("Render", "Round Same Side Corner Rectangle"), QT_TRANSLATE_NOOP("Render", "Rectangles"), [](double w, double h, V a) {
        const double r = A(a, 0, 0.1667) * mn(w, h);
        QPainterPath p; p.moveTo(r, 0); p.lineTo(w - r, 0); p.quadTo(w, 0, w, r); p.lineTo(w, h); p.lineTo(0, h); p.lineTo(0, r);
        p.quadTo(0, 0, r, 0); p.closeSubpath(); return p; }, {0.1667}, {{0, 0, 2, false, 0, 0, 0.5}}, 0.05);
    add("round2diag", QT_TRANSLATE_NOOP("Render", "Round Diagonal Corner Rectangle"), QT_TRANSLATE_NOOP("Render", "Rectangles"), [](double w, double h, V a) {
        const double r = A(a, 0, 0.1667) * mn(w, h);
        QPainterPath p; p.moveTo(r, 0); p.lineTo(w, 0); p.lineTo(w, h - r); p.quadTo(w, h, w - r, h); p.lineTo(0, h); p.lineTo(0, r);
        p.quadTo(0, 0, r, 0); p.closeSubpath(); return p; }, {0.1667}, {{0, 0, 2, false, 0, 0, 0.5}}, 0.05);

    // ---------------- Basic Shapes ----------------
    add("ellipse", QT_TRANSLATE_NOOP("Render", "Oval"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V) { return ellipse(w, h); }, {}, {}, 0.15);
    add("triangle", QT_TRANSLATE_NOOP("Render", "Isosceles Triangle"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V a) {
        return poly({{A(a, 0, 0.5) * w, 0}, {w, h}, {0, h}}); }, {0.5}, {{0, 0, 0, false, 0, 0, 1}},
        0, [](double w, double h, V) { return QRectF(w * 0.25, h * 0.5, w * 0.5, h * 0.48); });
    add("rtTriangle", QT_TRANSLATE_NOOP("Render", "Right Triangle"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V) { return poly({{0, 0}, {w, h}, {0, h}}); }, {}, {},
        0, [](double w, double h, V) { return QRectF(w * 0.06, h * 0.5, w * 0.5, h * 0.45); });
    add("parallelogram", QT_TRANSLATE_NOOP("Render", "Parallelogram"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V a) {
        const double s = std::clamp(A(a, 0, 0.25), 0.0, w / mn(w, h)) * mn(w, h); return poly({{s, 0}, {w, 0}, {w - s, h}, {0, h}}); }, {0.25}, {{0, 0, 2, false, 0, 0, 100, 1}}, 0.15, nullptr);
    add("trapezoid", QT_TRANSLATE_NOOP("Render", "Trapezoid"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V a) {
        const double s = std::clamp(A(a, 0, 0.25), 0.0, 0.5 * w / mn(w, h)) * mn(w, h); return poly({{s, 0}, {w - s, 0}, {w, h}, {0, h}}); }, {0.25}, {{0, 0, 2, false, 0, 0, 100, 0.5}}, 0.15, nullptr);
    add("diamond", QT_TRANSLATE_NOOP("Render", "Diamond"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V) { return poly({{w / 2, 0}, {w, h / 2}, {w / 2, h}, {0, h / 2}}); }, {}, {}, 0.25);
    add("pentagon", QT_TRANSLATE_NOOP("Render", "Regular Pentagon"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V) { return ngon(w, h, 5, -90); }, {}, {}, 0.2, nullptr);
    add("hexagon", QT_TRANSLATE_NOOP("Render", "Hexagon"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V a) {
        const double s = std::clamp(A(a, 0, 0.25), 0.0, 0.5 * w / mn(w, h)) * mn(w, h); return poly({{s, 0}, {w - s, 0}, {w, h / 2}, {w - s, h}, {s, h}, {0, h / 2}}); }, {0.25}, {{0, 0, 2, false, 0, 0, 100, 0.5}}, 0.15, nullptr);
    add("heptagon", QT_TRANSLATE_NOOP("Render", "Heptagon"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V) { return ngon(w, h, 7, -90); }, {}, {}, 0.18, nullptr);
    add("octagon", QT_TRANSLATE_NOOP("Render", "Octagon"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V a) {
        const double s = A(a, 0, 0.29) * mn(w, h);
        return poly({{s, 0}, {w - s, 0}, {w, s}, {w, h - s}, {w - s, h}, {s, h}, {0, h - s}, {0, s}}); },
        {0.29}, {{0, 0, 2, false, 0, 0, 0.5}}, 0.12);
    add("decagon", QT_TRANSLATE_NOOP("Render", "Decagon"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V) { return ngon(w, h, 10, 0); }, {}, {}, 0.12, nullptr);
    add("dodecagon", QT_TRANSLATE_NOOP("Render", "Dodecagon"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V) { return ngon(w, h, 12, -75); }, {}, {}, 0.12, nullptr);
    add("pie", QT_TRANSLATE_NOOP("Render", "Partial Circle"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V a) {
        QPainterPath p; p.moveTo(w / 2, h / 2); p.arcTo(QRectF(0, 0, w, h), 0, A(a, 0, 0.75) * 360); p.closeSubpath(); return p; },
        {0.75}, {}, 0.2);
    add("chord", QT_TRANSLATE_NOOP("Render", "Chord"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V a) {
        QPainterPath p; p.arcMoveTo(QRectF(0, 0, w, h), 45); p.arcTo(QRectF(0, 0, w, h), 45, A(a, 0, 0.75) * 360); p.closeSubpath(); return p; },
        {0.75}, {}, 0.2);
    add("teardrop", QT_TRANSLATE_NOOP("Render", "Teardrop"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V) {
        QPainterPath p; p.moveTo(w / 2, 0); p.lineTo(w, 0); p.lineTo(w, h / 2);
        p.arcTo(QRectF(0, 0, w, h), 0, -270); p.closeSubpath(); return p; }, {}, {}, 0.15);
    add("frame", QT_TRANSLATE_NOOP("Render", "Frame"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V a) {
        const double t = A(a, 0, 0.125) * mn(w, h); QPainterPath p; p.addRect(0, 0, w, h); p.addRect(t, t, w - 2 * t, h - 2 * t); return p; },
        {0.125}, {{0, 0, 2, false, 0, 0, 0.5}}, 0.12);
    add("halfFrame", QT_TRANSLATE_NOOP("Render", "Half Frame"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V a) {
        const double ss = mn(w, h), x1 = std::clamp(A(a, 1, 0.33333), 0.0, w / ss) * ss, y1 = std::clamp(A(a, 0, 0.33333), 0.0, (h - h * x1 / w) / ss) * ss;
        return poly({{0, 0}, {w, 0}, {w - y1 * w / h, y1}, {x1, y1}, {x1, h - x1 * h / w}, {0, h}}); }, {0.33333, 0.33333}, {}, 0.1, nullptr);
    add("corner", QT_TRANSLATE_NOOP("Render", "L-Shape"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V a) {
        const double t = A(a, 0, 0.5) * mn(w, h); return poly({{0, 0}, {t, 0}, {t, h - t}, {w, h - t}, {w, h}, {0, h}}); },
        {0.5}, {{0, 0, 2, false, 0.5, 0, 1}}, 0.1);
    add("diagStripe", QT_TRANSLATE_NOOP("Render", "Diagonal Stripe"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V a) {
        const double t = A(a, 0, 0.5); return poly({{0, h * t}, {w * t, 0}, {w, 0}, {0, h}}); }, {0.5}, {}, 0.15);
    add("plus", QT_TRANSLATE_NOOP("Render", "Cross"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V a) {
        const double t = A(a, 0, 0.25) * mn(w, h);
        return poly({{t, 0}, {w - t, 0}, {w - t, t}, {w, t}, {w, h - t}, {w - t, h - t}, {w - t, h}, {t, h}, {t, h - t}, {0, h - t}, {0, t}, {t, t}}); },
        {0.25}, {{0, 0, 2, false, 0, 0, 0.5}}, 0.25);
    add("plaque", QT_TRANSLATE_NOOP("Render", "Plaque"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V a) {
        const double r = A(a, 0, 0.1667) * mn(w, h);
        QPainterPath p; p.moveTo(r, 0); p.lineTo(w - r, 0); p.arcTo(QRectF(w - r, -r, 2 * r, 2 * r), 180, 90);
        p.lineTo(w, h - r); p.arcTo(QRectF(w - r, h - r, 2 * r, 2 * r), 90, 90); p.lineTo(r, h);
        p.arcTo(QRectF(-r, h - r, 2 * r, 2 * r), 0, 90); p.lineTo(0, r); p.arcTo(QRectF(-r, -r, 2 * r, 2 * r), 270, 90); p.closeSubpath(); return p; },
        {0.1667}, {{0, 0, 2, false, 0, 0, 0.5}}, 0.12);
    add("can", QT_TRANSLATE_NOOP("Render", "Can"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V a) {
        const double e = A(a, 0, 0.25) * mn(w, h); QPainterPath p;
        p.moveTo(0, e / 2); p.arcTo(QRectF(0, 0, w, e), 180, -180); p.lineTo(w, h - e / 2); p.arcTo(QRectF(0, h - e, w, e), 0, -180); p.closeSubpath();
        p.moveTo(0, e / 2); p.arcTo(QRectF(0, 0, w, e), 180, 180); return p; },
        {0.25}, {{0, 1, 2, false, 0.5, 0, 0.5}}, 0, [](double w, double h, V a) { const double e = A(a, 0, 0.25) * mn(w, h); return QRectF(0, e, w, h - 1.5 * e); });
    add("cube", QT_TRANSLATE_NOOP("Render", "Cube"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V a) {
        const double d = A(a, 0, 0.25) * mn(w, h); QPainterPath p;
        p.addPolygon(QPolygonF({QPointF(0, d), QPointF(d, 0), QPointF(w, 0), QPointF(w, h - d), QPointF(w - d, h), QPointF(0, h), QPointF(0, d)}));
        p.moveTo(0, d); p.lineTo(w - d, d); p.lineTo(w, 0); p.moveTo(w - d, d); p.lineTo(w - d, h); return p; },
        {0.25}, {{0, 1, 2, false, 0, 0, 1}}, 0, [](double w, double h, V a) { const double d = A(a, 0, 0.25) * mn(w, h); return QRectF(0, d, w - d, h - d); });
    add("bevel", QT_TRANSLATE_NOOP("Render", "Bevel"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V a) {
        const double d = A(a, 0, 0.125) * mn(w, h); QPainterPath p; p.addRect(0, 0, w, h); p.addRect(d, d, w - 2 * d, h - 2 * d);
        p.moveTo(0, 0); p.lineTo(d, d); p.moveTo(w, 0); p.lineTo(w - d, d); p.moveTo(w, h); p.lineTo(w - d, h - d); p.moveTo(0, h); p.lineTo(d, h - d);
        return p; }, {0.125}, {{0, 0, 2, false, 0, 0, 0.5}}, 0.13);
    add("donut", QT_TRANSLATE_NOOP("Render", "Donut"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V a) {
        const double t = A(a, 0, 0.25) * mn(w, h); QPainterPath p; p.addEllipse(QRectF(0, 0, w, h)); p.addEllipse(QRectF(t, t, w - 2 * t, h - 2 * t)); return p; },
        {0.25}, {{0, 0, 2, false, 0.5, 0, 0.5}}, 0.3);
    add("noSmoking", QT_TRANSLATE_NOOP("Render", "\"No\" Symbol"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V a) {
        const double t = A(a, 0, 0.18) * mn(w, h); QPainterPath ring; ring.addEllipse(QRectF(0, 0, w, h));
        QPainterPath inner; inner.addEllipse(QRectF(t, t, w - 2 * t, h - 2 * t));
        QPainterPath bar; bar.addRect(QRectF(-w, -t / 2, 3 * w, t));
        QTransform tr; tr.translate(w / 2, h / 2); tr.rotate(45); tr.translate(-w / 2, 0);
        QPainterPath slash = tr.map(bar).intersected(inner);
        return ring.subtracted(inner).united(slash); }, {0.18}, {}, 0.3);
    add("blockArc", QT_TRANSLATE_NOOP("Render", "Block Arc"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V a) {
        const double t = A(a, 0, 0.25) * mn(w, h); QPainterPath p; p.arcMoveTo(QRectF(0, 0, w, h), 180); p.arcTo(QRectF(0, 0, w, h), 180, -180);
        p.lineTo(w - t, h / 2); p.arcTo(QRectF(t, t, w - 2 * t, h - 2 * t), 0, 180); p.closeSubpath(); return p; }, {0.25}, {}, 0.2);
    add("foldedCorner", QT_TRANSLATE_NOOP("Render", "Folded Corner"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V a) {
        const double f = A(a, 0, 0.1667) * mn(w, h); QPainterPath p = poly({{0, 0}, {w, 0}, {w, h - f}, {w - f, h}, {0, h}});
        p.moveTo(w - f, h); p.lineTo(w - f * 0.8, h - f * 0.8); p.lineTo(w, h - f); return p; }, {0.1667}, {{0, 0, 2, true, 1, 0, 0.5}}, 0.08);
    add("smiley", QT_TRANSLATE_NOOP("Render", "Smiley Face"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V a) {
        QPainterPath p; p.addEllipse(QRectF(0, 0, w, h));
        p.addEllipse(QRectF(w * 0.3, h * 0.3, w * 0.1, h * 0.12)); p.addEllipse(QRectF(w * 0.6, h * 0.3, w * 0.1, h * 0.12));
        const double s = A(a, 0, 0.05);
        p.moveTo(w * 0.28, h * 0.65); p.quadTo(w * 0.5, h * (0.65 + s * 3), w * 0.72, h * 0.65); return p; }, {0.05}, {}, 0.2);
    add("heart", QT_TRANSLATE_NOOP("Render", "Heart"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V) {
        QPainterPath p; p.moveTo(w / 2, h * 0.25);
        p.cubicTo(w / 2, h * 0.05, w * 0.08, -h * 0.02, w * 0.02, h * 0.3);
        p.cubicTo(-w * 0.04, h * 0.58, w * 0.3, h * 0.75, w / 2, h);
        p.cubicTo(w * 0.7, h * 0.75, w * 1.04, h * 0.58, w * 0.98, h * 0.3);
        p.cubicTo(w * 0.92, -h * 0.02, w / 2, h * 0.05, w / 2, h * 0.25); p.closeSubpath(); return p; }, {}, {}, 0.25);
    add("lightning", QT_TRANSLATE_NOOP("Render", "Lightning Bolt"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V) {
        // (The standard's points, in 21600ths of the box.)
        static const int pts[][2] = {{8472, 0}, {12860, 6080}, {11050, 6797}, {16577, 12007},
                                        {14767, 12877}, {21600, 21600}, {10012, 14915}, {12222, 13987},
                                        {5022, 9705}, {7602, 8382}, {0, 3890}};
        QVector<QPointF> v; for (const auto &p : pts) v << QPointF(p[0] * w / 21600, p[1] * h / 21600); return polyV(v); }, {}, {}, 0.3, nullptr);
    add("sun", QT_TRANSLATE_NOOP("Render", "Sun"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V a) {
        const double r = A(a, 0, 0.25); QPainterPath p; p.addEllipse(QRectF(w * r, h * r, w * (1 - 2 * r), h * (1 - 2 * r)));
        for (int i = 0; i < 8; ++i) {
            const double ang = i * M_PI / 4, a1 = ang - 0.13, a2 = ang + 0.13;
            const double ri = 0.5 - r + 0.06, ro = 0.5;
            p.addPolygon(QPolygonF({QPointF(w / 2 + std::cos(a1) * w * (0.5 - ri), h / 2 + std::sin(a1) * h * (0.5 - ri)),
                                    QPointF(w / 2 + std::cos(ang) * w * ro, h / 2 + std::sin(ang) * h * ro),
                                    QPointF(w / 2 + std::cos(a2) * w * (0.5 - ri), h / 2 + std::sin(a2) * h * (0.5 - ri))}));
            p.closeSubpath();
        }
        return p; }, {0.25}, {}, 0.3);
    add("moon", QT_TRANSLATE_NOOP("Render", "Moon"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V a) {
        const double t = A(a, 0, 0.5); QPainterPath p; p.moveTo(w, 0); p.arcTo(QRectF(0, 0, 2 * w, h), 90, 180);
        p.arcTo(QRectF(w * t, 0, 2 * w * (1 - t), h), 270, -180); p.closeSubpath(); return p; }, {0.5}, {{0, 0, 0, false, 0.5, 0.05, 0.95}}, 0.25);
    add("cloud", QT_TRANSLATE_NOOP("Render", "Cloud"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V) {
        QPainterPath p;
        const QRectF c[] = {{0.05, 0.3, 0.35, 0.4}, {0.2, 0.08, 0.35, 0.4}, {0.45, 0.05, 0.35, 0.42}, {0.62, 0.25, 0.36, 0.42},
                            {0.45, 0.5, 0.38, 0.42}, {0.15, 0.52, 0.38, 0.42}, {0.0, 0.42, 0.3, 0.32},
                            {0.25, 0.3, 0.5, 0.45}};   // the middle, so the bumps leave no gap
        for (const QRectF &r : c) { QPainterPath e; e.addEllipse(QRectF(r.x() * w, r.y() * h, r.width() * w, r.height() * h)); p = p.united(e); }
        return p; }, {}, {}, 0.22);
    add("arc", QT_TRANSLATE_NOOP("Render", "Arc"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V a) {
        QPainterPath p; p.arcMoveTo(QRectF(0, 0, w, h), 90); p.arcTo(QRectF(0, 0, w, h), 90, -A(a, 0, 0.25) * 360); return p; },
        {0.25}, {}, 0.2, nullptr, true);
    add("bracketPair", QT_TRANSLATE_NOOP("Render", "Double Bracket"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V a) {
        const double r = A(a, 0, 0.1667) * mn(w, h); QPainterPath p;
        p.moveTo(r, 0); p.quadTo(0, 0, 0, r); p.lineTo(0, h - r); p.quadTo(0, h, r, h);
        p.moveTo(w - r, 0); p.quadTo(w, 0, w, r); p.lineTo(w, h - r); p.quadTo(w, h, w - r, h); return p; }, {0.1667}, {}, 0.1, nullptr, true);
    add("bracePair", QT_TRANSLATE_NOOP("Render", "Double Brace"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V a) {
        const double r = A(a, 0, 0.0833) * mn(w, h) * 2; QPainterPath p;
        p.moveTo(2 * r, 0); p.quadTo(r, 0, r, r); p.lineTo(r, h / 2 - r); p.quadTo(r, h / 2, 0, h / 2); p.quadTo(r, h / 2, r, h / 2 + r);
        p.lineTo(r, h - r); p.quadTo(r, h, 2 * r, h);
        p.moveTo(w - 2 * r, 0); p.quadTo(w - r, 0, w - r, r); p.lineTo(w - r, h / 2 - r); p.quadTo(w - r, h / 2, w, h / 2);
        p.quadTo(w - r, h / 2, w - r, h / 2 + r); p.lineTo(w - r, h - r); p.quadTo(w - r, h, w - 2 * r, h); return p; }, {0.0833}, {}, 0.12, nullptr, true);
    add("leftBracket", QT_TRANSLATE_NOOP("Render", "Left Bracket"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V) {
        QPainterPath p; p.moveTo(w, 0); p.quadTo(0, 0, 0, w); p.lineTo(0, h - w); p.quadTo(0, h, w, h); return p; }, {}, {}, 0.1, nullptr, true);
    add("rightBracket", QT_TRANSLATE_NOOP("Render", "Right Bracket"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V) {
        QPainterPath p; p.moveTo(0, 0); p.quadTo(w, 0, w, w); p.lineTo(w, h - w); p.quadTo(w, h, 0, h); return p; }, {}, {}, 0.1, nullptr, true);
    add("leftBrace", QT_TRANSLATE_NOOP("Render", "Left Brace"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V) {
        QPainterPath p; p.moveTo(w, 0); p.quadTo(w / 2, 0, w / 2, w / 2); p.lineTo(w / 2, h / 2 - w / 2); p.quadTo(w / 2, h / 2, 0, h / 2);
        p.quadTo(w / 2, h / 2, w / 2, h / 2 + w / 2); p.lineTo(w / 2, h - w / 2); p.quadTo(w / 2, h, w, h); return p; }, {}, {}, 0.1, nullptr, true);
    add("rightBrace", QT_TRANSLATE_NOOP("Render", "Right Brace"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V) {
        QPainterPath p; p.moveTo(0, 0); p.quadTo(w / 2, 0, w / 2, w / 2); p.lineTo(w / 2, h / 2 - w / 2); p.quadTo(w / 2, h / 2, w, h / 2);
        p.quadTo(w / 2, h / 2, w / 2, h / 2 + w / 2); p.lineTo(w / 2, h - w / 2); p.quadTo(w / 2, h, 0, h); return p; }, {}, {}, 0.1, nullptr, true);

    // ---------------- Block Arrows ----------------
    add("rightArrow", QT_TRANSLATE_NOOP("Render", "Right Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"), [](double w, double h, V a) { return arrowRight(w, h, A(a, 0, 0.5), A(a, 1, 0.5)); },
        {0.5, 0.5}, {{0, 1, 1, false, 0, 0.05, 1}}, 0.15, [](double w, double h, V a) { return arrowRightText(w, h, A(a, 0, 0.5), A(a, 1, 0.5)); });
    add("leftArrow", QT_TRANSLATE_NOOP("Render", "Left Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"), [](double w, double h, V a) { return rotated(arrowRight(w, h, A(a, 0, 0.5), A(a, 1, 0.5)), w, h, 180, w, h); },
        {0.5, 0.5}, {}, 0.15, [](double w, double h, V a) { return rotatedRect(arrowRightText(w, h, A(a, 0, 0.5), A(a, 1, 0.5)), w, h, 180, w, h); });
    add("upArrow", QT_TRANSLATE_NOOP("Render", "Up Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"), [](double w, double h, V a) { return rotated(arrowRight(h, w, A(a, 0, 0.5), A(a, 1, 0.5)), w, h, -90, h, w); },
        {0.5, 0.5}, {}, 0.15, [](double w, double h, V a) { return rotatedRect(arrowRightText(h, w, A(a, 0, 0.5), A(a, 1, 0.5)), w, h, -90, h, w); });
    add("downArrow", QT_TRANSLATE_NOOP("Render", "Down Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"), [](double w, double h, V a) { return rotated(arrowRight(h, w, A(a, 0, 0.5), A(a, 1, 0.5)), w, h, 90, h, w); },
        {0.5, 0.5}, {}, 0.15, [](double w, double h, V a) { return rotatedRect(arrowRightText(h, w, A(a, 0, 0.5), A(a, 1, 0.5)), w, h, 90, h, w); });
    add("leftRightArrow", QT_TRANSLATE_NOOP("Render", "Left-Right Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"), [](double w, double h, V a) {
        const double t = A(a, 0, 0.5), hd = std::min(w / 2, A(a, 1, 0.5) * mn(w, h));
        const double y0 = h * (0.5 - t / 2), y1 = h * (0.5 + t / 2);
        return poly({{0, h / 2}, {hd, 0}, {hd, y0}, {w - hd, y0}, {w - hd, 0}, {w, h / 2}, {w - hd, h}, {w - hd, y1}, {hd, y1}, {hd, h}}); },
        {0.5, 0.5}, {}, 0.2, [](double w, double h, V a) {
            const double t = A(a, 0, 0.5), hd = std::min(w / 2, A(a, 1, 0.5) * mn(w, h));
            const double y0 = h * (0.5 - t / 2), y1 = h * (0.5 + t / 2), x = h > 0 ? hd * (h / 2 - y0) / (h / 2) : 0;
            return QRectF(x, y0, std::max(0.0, w - 2 * x), y1 - y0); });
    add("upDownArrow", QT_TRANSLATE_NOOP("Render", "Up-Down Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"), [](double w, double h, V a) {
        const double t = A(a, 0, 0.5), hd = std::min(h / 2, A(a, 1, 0.5) * mn(w, h));
        const double x0 = w * (0.5 - t / 2), x1 = w * (0.5 + t / 2);
        return poly({{w / 2, 0}, {w, hd}, {x1, hd}, {x1, h - hd}, {w, h - hd}, {w / 2, h}, {0, h - hd}, {x0, h - hd}, {x0, hd}, {0, hd}}); },
        {0.5, 0.5}, {}, 0.2, [](double w, double h, V a) {
            const double t = A(a, 0, 0.5), hd = std::min(h / 2, A(a, 1, 0.5) * mn(w, h));
            const double x0 = w * (0.5 - t / 2), x1 = w * (0.5 + t / 2), y = w > 0 ? hd * (w / 2 - x0) / (w / 2) : 0;
            return QRectF(x0, y, x1 - x0, std::max(0.0, h - 2 * y)); });
    add("quadArrow", QT_TRANSLATE_NOOP("Render", "Quad Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"), [](double w, double h, V a) {
        const double m = mn(w, h), t = A(a, 0, 0.12) * m, hw = A(a, 1, 0.22) * m, hl = A(a, 2, 0.22) * m;
        const double cx = w / 2, cy = h / 2;
        return poly({{cx, 0}, {cx + hw, hl}, {cx + t, hl}, {cx + t, cy - t}, {w - hl, cy - t}, {w - hl, cy - hw}, {w, cy}, {w - hl, cy + hw},
                     {w - hl, cy + t}, {cx + t, cy + t}, {cx + t, h - hl}, {cx + hw, h - hl}, {cx, h}, {cx - hw, h - hl}, {cx - t, h - hl},
                     {cx - t, cy + t}, {hl, cy + t}, {hl, cy + hw}, {0, cy}, {hl, cy - hw}, {hl, cy - t}, {cx - t, cy - t}, {cx - t, hl}, {cx - hw, hl}}); },
        {0.12, 0.22, 0.22}, {}, 0.3);
    add("bentArrow", QT_TRANSLATE_NOOP("Render", "Bent Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"), [](double w, double h, V a) {
        const double m = mn(w, h), t = A(a, 0, 0.25) * m, hd = A(a, 1, 0.25) * m * 2, hl = A(a, 2, 0.25) * m;
        QPainterPath p; p.moveTo(0, h); p.lineTo(0, hd / 2 + t); p.quadTo(0, hd / 2 - t / 2, t * 1.5, hd / 2 - t / 2);
        p.lineTo(w - hl, hd / 2 - t / 2); p.lineTo(w - hl, 0); p.lineTo(w, hd / 2); p.lineTo(w - hl, hd); p.lineTo(w - hl, hd / 2 + t / 2);
        p.lineTo(t * 1.5, hd / 2 + t / 2); p.quadTo(t, hd / 2 + t / 2, t, hd / 2 + t); p.lineTo(t, h); p.closeSubpath(); return p; },
        {0.25, 0.25, 0.25}, {}, 0.2);
    add("uturnArrow", QT_TRANSLATE_NOOP("Render", "U-Turn Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"), [](double w, double h, V a) {
        const double m = mn(w, h), t = A(a, 0, 0.25) * m, hl = A(a, 1, 0.25) * m;
        const double r = std::max(t, (w - hl * 1.5) / 2);
        QPainterPath p; p.moveTo(0, h); p.lineTo(0, r); p.arcTo(QRectF(0, 0, 2 * r, 2 * r), 180, -180);
        p.lineTo(2 * r, h - hl); p.lineTo(2 * r + hl * 0.75, h - hl); p.lineTo(2 * r - t / 2, h); p.lineTo(2 * r - t - hl * 0.75, h - hl);
        p.lineTo(2 * r - t, h - hl); p.lineTo(2 * r - t, r); p.arcTo(QRectF(t, t, 2 * r - 2 * t, 2 * r - 2 * t), 0, 180); p.lineTo(t, h);
        p.closeSubpath(); return p; }, {0.25, 0.25}, {}, 0.2);
    add("leftUpArrow", QT_TRANSLATE_NOOP("Render", "Left-Up Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"), [](double w, double h, V a) {
        const double m = mn(w, h), t = A(a, 0, 0.25) * m, hw = A(a, 1, 0.25) * m;
        return poly({{0, h - hw}, {hw, h - 2 * hw}, {hw, h - hw - t / 2}, {w - hw - t / 2, h - hw - t / 2}, {w - hw - t / 2, hw}, {w - 2 * hw, hw},
                     {w - hw, 0}, {w, hw}, {w - hw + t / 2, hw}, {w - hw + t / 2, h - hw + t / 2}, {hw, h - hw + t / 2}, {hw, h}}); },
        {0.25, 0.25}, {}, 0.25);
    add("bentUpArrow", QT_TRANSLATE_NOOP("Render", "Bent-Up Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"), [](double w, double h, V a) {
        const double m = mn(w, h), t = A(a, 0, 0.25) * m, hw = A(a, 1, 0.25) * m;
        return poly({{0, h - t}, {w - hw - t / 2, h - t}, {w - hw - t / 2, hw}, {w - 2 * hw, hw}, {w - hw, 0}, {w, hw}, {w - hw + t / 2, hw},
                     {w - hw + t / 2, h}, {0, h}}); }, {0.25, 0.25}, {}, 0.25);
    add("curvedRightArrow", QT_TRANSLATE_NOOP("Render", "Curved Right Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"), [](double w, double h, V) {
        QPainterPath p; p.moveTo(w, h * 0.72); p.lineTo(w * 0.6, h * 0.45); p.lineTo(w * 0.62, h * 0.62);
        p.cubicTo(w * 0.2, h * 0.55, 0, h * 0.35, 0, 0); p.lineTo(0, h * 0.28); p.cubicTo(0, h * 0.62, w * 0.25, h * 0.88, w * 0.62, h * 0.9);
        p.lineTo(w * 0.6, h); p.closeSubpath(); return p; }, {}, {}, 0.3);
    add("curvedLeftArrow", QT_TRANSLATE_NOOP("Render", "Curved Left Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"), [](double w, double h, V) {
        QPainterPath p; p.moveTo(0, h * 0.72); p.lineTo(w * 0.4, h * 0.45); p.lineTo(w * 0.38, h * 0.62);
        p.cubicTo(w * 0.8, h * 0.55, w, h * 0.35, w, 0); p.lineTo(w, h * 0.28); p.cubicTo(w, h * 0.62, w * 0.75, h * 0.88, w * 0.38, h * 0.9);
        p.lineTo(w * 0.4, h); p.closeSubpath(); return p; }, {}, {}, 0.3);
    add("curvedUpArrow", QT_TRANSLATE_NOOP("Render", "Curved Up Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"), [](double w, double h, V) {
        QPainterPath p; p.moveTo(w * 0.72, 0); p.lineTo(w * 0.45, h * 0.4); p.lineTo(w * 0.62, h * 0.38);
        p.cubicTo(w * 0.55, h * 0.8, w * 0.35, h, 0, h); p.lineTo(w * 0.28, h); p.cubicTo(w * 0.62, h, w * 0.88, h * 0.75, w * 0.9, h * 0.38);
        p.lineTo(w, h * 0.4); p.closeSubpath(); return p; }, {}, {}, 0.3);
    add("curvedDownArrow", QT_TRANSLATE_NOOP("Render", "Curved Down Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"), [](double w, double h, V) {
        QPainterPath p; p.moveTo(w * 0.72, h); p.lineTo(w * 0.45, h * 0.6); p.lineTo(w * 0.62, h * 0.62);
        p.cubicTo(w * 0.55, h * 0.2, w * 0.35, 0, 0, 0); p.lineTo(w * 0.28, 0); p.cubicTo(w * 0.62, 0, w * 0.88, h * 0.25, w * 0.9, h * 0.62);
        p.lineTo(w, h * 0.6); p.closeSubpath(); return p; }, {}, {}, 0.3);
    add("stripedRightArrow", QT_TRANSLATE_NOOP("Render", "Striped Right Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"), [](double w, double h, V a) {
        const double t = A(a, 0, 0.5), y0 = h * (0.5 - t / 2), y1 = h * (0.5 + t / 2), s = w * 0.04;
        QPainterPath p = arrowRight(w, h, t, A(a, 1, 0.5)).subtracted([&] { QPainterPath c; c.addRect(0, 0, s * 4.5, h); return c; }());
        p.addRect(0, y0, s, y1 - y0); p.addRect(s * 2, y0, s, y1 - y0); return p; }, {0.5, 0.5}, {}, 0.2);
    add("notchedRightArrow", QT_TRANSLATE_NOOP("Render", "Notched Right Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"), [](double w, double h, V a) {
        const double t = A(a, 0, 0.5), hd = std::min(w, A(a, 1, 0.5) * mn(w, h));
        const double y0 = h * (0.5 - t / 2), y1 = h * (0.5 + t / 2), n = t * hd;
        return poly({{0, y0}, {w - hd, y0}, {w - hd, 0}, {w, h / 2}, {w - hd, h}, {w - hd, y1}, {0, y1}, {n, h / 2}}); }, {0.5, 0.5}, {}, 0.2, nullptr);
    add("homePlate", QT_TRANSLATE_NOOP("Render", "Pentagon Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"), [](double w, double h, V a) {
        const double s = std::min(w, A(a, 0, 0.5) * mn(w, h)); return poly({{0, 0}, {w - s, 0}, {w, h / 2}, {w - s, h}, {0, h}}); },
        {0.5}, {{0, 0, 2, true, 0, 0, 1}}, 0.08);
    add("chevron", QT_TRANSLATE_NOOP("Render", "Chevron"), QT_TRANSLATE_NOOP("Render", "Block Arrows"), [](double w, double h, V a) {
        const double s = std::min(w / 2, A(a, 0, 0.5) * mn(w, h)); return poly({{0, 0}, {w - s, 0}, {w, h / 2}, {w - s, h}, {0, h}, {s, h / 2}}); },
        {0.5}, {{0, 0, 2, true, 0, 0, 1}}, 0.2);
    add("rightArrowCallout", QT_TRANSLATE_NOOP("Render", "Right Arrow Callout"), QT_TRANSLATE_NOOP("Render", "Block Arrows"), [](double w, double h, V) {
        return poly({{0, 0}, {w * 0.65, 0}, {w * 0.65, h * 0.38}, {w * 0.8, h * 0.38}, {w * 0.8, h * 0.25}, {w, h / 2}, {w * 0.8, h * 0.75},
                     {w * 0.8, h * 0.62}, {w * 0.65, h * 0.62}, {w * 0.65, h}, {0, h}}); }, {}, {}, 0,
        [](double w, double h, V) { return QRectF(w * 0.03, h * 0.05, w * 0.6, h * 0.9); });
    add("downArrowCallout", QT_TRANSLATE_NOOP("Render", "Down Arrow Callout"), QT_TRANSLATE_NOOP("Render", "Block Arrows"), [](double w, double h, V) {
        return poly({{0, 0}, {w, 0}, {w, h * 0.65}, {w * 0.62, h * 0.65}, {w * 0.62, h * 0.8}, {w * 0.75, h * 0.8}, {w / 2, h}, {w * 0.25, h * 0.8},
                     {w * 0.38, h * 0.8}, {w * 0.38, h * 0.65}, {0, h * 0.65}}); }, {}, {}, 0,
        [](double w, double h, V) { return QRectF(w * 0.05, h * 0.03, w * 0.9, h * 0.6); });
    add("circularArrow", QT_TRANSLATE_NOOP("Render", "Circular Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"), [](double w, double h, V a) {
        const double t = A(a, 0, 0.125) * mn(w, h);
        QPainterPath p; p.arcMoveTo(QRectF(0, 0, w, h), 180); p.arcTo(QRectF(0, 0, w, h), 180, -150);
        const QPointF tip(w / 2 + std::cos(qDegreesToRadians(-30.0)) * (w / 2 - t / 2), h / 2 + std::sin(qDegreesToRadians(-30.0)) * (h / 2 - t / 2));
        p.lineTo(tip.x() + t, tip.y()); p.lineTo(tip.x(), tip.y() + t * 2.2); p.lineTo(tip.x() - t * 2, tip.y() + t * 0.2);
        p.arcTo(QRectF(t, t, w - 2 * t, h - 2 * t), 30, 150); p.closeSubpath(); return p; }, {0.125}, {}, 0.3);

    // ---------------- Equation Shapes ----------------
    add("mathPlus", QT_TRANSLATE_NOOP("Render", "Plus"), QT_TRANSLATE_NOOP("Render", "Equation Shapes"), [](double w, double h, V a) {
        const double t = A(a, 0, 0.2352) * mn(w, h), cx = w / 2, cy = h / 2, ex = w * 0.36745, ey = h * 0.36745;
        return poly({{cx - t / 2, cy - ey}, {cx + t / 2, cy - ey}, {cx + t / 2, cy - t / 2}, {cx + ex, cy - t / 2}, {cx + ex, cy + t / 2},
                     {cx + t / 2, cy + t / 2}, {cx + t / 2, cy + ey}, {cx - t / 2, cy + ey}, {cx - t / 2, cy + t / 2}, {cx - ex, cy + t / 2},
                     {cx - ex, cy - t / 2}, {cx - t / 2, cy - t / 2}}); }, {0.2352}, {}, 0.3, nullptr);
    add("mathMinus", QT_TRANSLATE_NOOP("Render", "Minus"), QT_TRANSLATE_NOOP("Render", "Equation Shapes"), [](double w, double h, V a) {
        const double t = A(a, 0, 0.2352) * h; QPainterPath p; p.addRect(w * 0.13255, h / 2 - t / 2, w * 0.7349, t); return p; }, {0.2352}, {}, 0.3, nullptr);
    add("mathMultiply", QT_TRANSLATE_NOOP("Render", "Multiply"), QT_TRANSLATE_NOOP("Render", "Equation Shapes"), [](double w, double h, V a) {
        // (Two bars along the box's diagonals, as long as 51965 hundred-thousandths of the diagonal leaves room for.)
        const double th = std::clamp(A(a, 0, 0.2352), 0.0, 0.51965) * mn(w, h), ang = std::atan2(h, w), sa = std::sin(ang), ca = std::cos(ang), ta = std::tan(ang);
        const double dl = std::hypot(w, h), lM = dl - dl * 0.51965, xM = ca * lM / 2, yM = sa * lM / 2, dxAM = sa * th / 2, dyAM = ca * th / 2;
        const double xA = xM - dxAM, yA = yM + dyAM, xB = xM + dxAM, yB = yM - dyAM, yC = (w / 2 - xB) * ta + yB;
        const double xD = w - xB, xE = w - xA, xFE = (h / 2 - yA) / ta, xF = xE - xFE, xL = xA + xFE, yG = h - yA, yH = h - yB, yI = h - yC;
        return poly({{xA, yA}, {xB, yB}, {w / 2, yC}, {xD, yB}, {xE, yA}, {xF, h / 2}, {xE, yG}, {xD, yH}, {w / 2, yI}, {xB, yH}, {xA, yG}, {xL, h / 2}}); }, {0.2352}, {}, 0.3, nullptr);
    add("mathDivide", QT_TRANSLATE_NOOP("Render", "Division"), QT_TRANSLATE_NOOP("Render", "Equation Shapes"), [](double w, double h, V a) {
        // (A bar with a dot above and below it; the standard keeps the three inside the box by pinning their sizes against each other.)
        const double a1 = std::clamp(A(a, 0, 0.2352), 0.01, 0.36745), a3 = std::clamp(A(a, 2, 0.1176), 0.01, std::min((0.7349 - a1) / 4, 0.36745 * w / h));
        const double a2 = std::clamp(A(a, 1, 0.0588), 0.0, 0.7349 - 4 * a3 - a1);
        const double dy1 = h * a1 / 2, rad = h * a3, dx1 = w * 0.36745, y3 = h / 2 - dy1, y2 = y3 - (h * a2 + rad), y1 = y2 - rad;
        QPainterPath p; p.addRect(w / 2 - dx1, y3, 2 * dx1, 2 * dy1);
        p.addEllipse(QRectF(w / 2 - rad, y1, 2 * rad, 2 * rad)); p.addEllipse(QRectF(w / 2 - rad, h - y1 - 2 * rad, 2 * rad, 2 * rad)); return p; }, {0.2352, 0.0588, 0.1176}, {}, 0.3, nullptr);
    add("mathEqual", QT_TRANSLATE_NOOP("Render", "Equal"), QT_TRANSLATE_NOOP("Render", "Equation Shapes"), [](double w, double h, V a) {
        const double t = A(a, 0, 0.2352) * h, g = A(a, 1, 0.1176) * h; QPainterPath p;
        p.addRect(w * 0.13255, h / 2 - g / 2 - t, w * 0.7349, t); p.addRect(w * 0.13255, h / 2 + g / 2, w * 0.7349, t); return p; }, {0.2352, 0.1176}, {}, 0.3, nullptr);
    add("mathNotEqual", QT_TRANSLATE_NOOP("Render", "Not Equal"), QT_TRANSLATE_NOOP("Render", "Equation Shapes"), [](double w, double h, V a) {
        // (The standard's drawing: two bars and a slash across them. The bars' thickness and the gap between them are fractions of the height; the slash's angle is the second value, in
        // 100,000ths of 60,000ths of a degree, as the standard has it, which is 66 for its 110 degrees.)
        const double a1 = std::clamp(A(a, 0, 0.2352), 0.0, 0.5), crAng = std::clamp(A(a, 1, 66.0), 42.0, 66.0) * 100000 / 60000 * M_PI / 180;
        const double a3 = std::clamp(A(a, 2, 0.1176), 0.0, 1 - 2 * a1);
        const double hc = w / 2, vc = h / 2, hd2 = h / 2, dy1 = h * a1, dy2 = h * a3 / 2, dx1 = w * 0.7349 / 2;
        const double x1 = hc - dx1, x8 = hc + dx1, y2 = vc - dy2, y3 = vc + dy2, y1 = y2 - dy1, y4 = y3 + dy1;
        const double cadj2 = crAng - M_PI / 2, xadj2 = hd2 * std::tan(cadj2), len = std::hypot(xadj2, hd2), bhw = len * dy1 / hd2;
        const double x7 = hc + xadj2 - bhw / 2, x6 = x7 - xadj2 * y1 / hd2, x5 = x7 - xadj2 * y2 / hd2, x4 = x7 - xadj2 * y3 / hd2, x3 = x7 - xadj2 * y4 / hd2;
        const double rx7 = x7 + bhw, rx6 = x6 + bhw, rx5 = x5 + bhw, rx4 = x4 + bhw, rx3 = x3 + bhw;
        const double dx7 = dy1 * hd2 / len, dy3 = dy1 * xadj2 / len;
        const double rx = cadj2 > 0 ? x7 + dx7 : rx7, lx = cadj2 > 0 ? x7 : rx7 - dx7, ry = cadj2 > 0 ? dy3 : 0, ly = cadj2 > 0 ? 0 : -dy3;
        const double dlx = w - rx, drx = w - lx, dly = h - ry, dry = h - ly;
        return poly({{x1, y1}, {x6, y1}, {lx, ly}, {rx, ry}, {rx6, y1}, {x8, y1}, {x8, y2}, {rx5, y2}, {rx4, y3}, {x8, y3}, {x8, y4}, {rx3, y4}, {drx, dry}, {dlx, dly},
                     {x3, y4}, {x1, y4}, {x1, y3}, {x4, y3}, {x5, y2}, {x1, y2}}); }, {0.2352, 66.0, 0.1176}, {}, 0.3, nullptr);

    // ---------------- Flowchart ----------------
    add("fcProcess", QT_TRANSLATE_NOOP("Render", "Flowchart: Process"), QT_TRANSLATE_NOOP("Render", "Flowchart"), [](double w, double h, V) { QPainterPath p; p.addRect(0, 0, w, h); return p; }, {}, {}, 0.05);
    add("fcAltProcess", QT_TRANSLATE_NOOP("Render", "Flowchart: Alternate Process"), QT_TRANSLATE_NOOP("Render", "Flowchart"), [](double w, double h, V) { return roundRect(w, h, mn(w, h) * 0.17); }, {}, {}, 0.06);
    add("fcDecision", QT_TRANSLATE_NOOP("Render", "Flowchart: Decision"), QT_TRANSLATE_NOOP("Render", "Flowchart"), [](double w, double h, V) { return poly({{w / 2, 0}, {w, h / 2}, {w / 2, h}, {0, h / 2}}); }, {}, {}, 0.25);
    add("fcData", QT_TRANSLATE_NOOP("Render", "Flowchart: Data"), QT_TRANSLATE_NOOP("Render", "Flowchart"), [](double w, double h, V) { return poly({{w * 0.2, 0}, {w, 0}, {w * 0.8, h}, {0, h}}); }, {}, {}, 0.2);
    add("fcPredefined", QT_TRANSLATE_NOOP("Render", "Flowchart: Predefined Process"), QT_TRANSLATE_NOOP("Render", "Flowchart"), [](double w, double h, V) {
        QPainterPath p; p.addRect(0, 0, w, h); p.moveTo(w * 0.125, 0); p.lineTo(w * 0.125, h); p.moveTo(w * 0.875, 0); p.lineTo(w * 0.875, h); return p; },
        {}, {}, 0, [](double w, double h, V) { return QRectF(w * 0.13, 0, w * 0.74, h); });
    add("fcInternalStorage", QT_TRANSLATE_NOOP("Render", "Flowchart: Internal Storage"), QT_TRANSLATE_NOOP("Render", "Flowchart"), [](double w, double h, V) {
        QPainterPath p; p.addRect(0, 0, w, h); p.moveTo(w * 0.15, 0); p.lineTo(w * 0.15, h); p.moveTo(0, h * 0.15); p.lineTo(w, h * 0.15); return p; },
        {}, {}, 0, [](double w, double h, V) { return QRectF(w * 0.15, h * 0.15, w * 0.85, h * 0.85); });
    add("fcDocument", QT_TRANSLATE_NOOP("Render", "Flowchart: Document"), QT_TRANSLATE_NOOP("Render", "Flowchart"), [](double w, double h, V) {
        QPainterPath p; p.moveTo(0, 0); p.lineTo(w, 0); p.lineTo(w, h * 17322 / 21600); p.cubicTo(w / 2, h * 17322 / 21600, w / 2, h * 23922 / 21600, 0, h * 20172 / 21600);
        p.closeSubpath(); return p; }, {}, {}, 0, [](double w, double h, V) { return QRectF(0, 0, w, h * 17322 / 21600); });
    add("fcMultidocument", QT_TRANSLATE_NOOP("Render", "Flowchart: Multidocument"), QT_TRANSLATE_NOOP("Render", "Flowchart"), [](double w, double h, V) {
        // (The standard's three sheets, in 21600ths of the box.)
        const auto P = [&](double x, double y) { return QPointF(x * w / 21600, y * h / 21600); };
        QPainterPath p;
        p.moveTo(P(0, 20782)); p.cubicTo(P(9298, 23542), P(9298, 18022), P(18595, 18022)); p.lineTo(P(18595, 3675)); p.lineTo(P(0, 3675)); p.closeSubpath();
        p.moveTo(P(1532, 3675)); p.lineTo(P(1532, 1815)); p.lineTo(P(20000, 1815)); p.lineTo(P(20000, 16252)); p.cubicTo(P(19298, 16252), P(18595, 16352), P(18595, 16352));
        p.lineTo(P(18595, 3675)); p.closeSubpath();
        p.moveTo(P(2972, 1815)); p.lineTo(P(2972, 0)); p.lineTo(P(21600, 0)); p.lineTo(P(21600, 14392)); p.cubicTo(P(20800, 14392), P(20000, 14467), P(20000, 14467));
        p.lineTo(P(20000, 1815)); p.closeSubpath(); return p; }, {}, {}, 0, [](double w, double h, V) { return QRectF(0, h * 0.2, w * 0.86, h * 0.6); });
    add("fcTerminator", QT_TRANSLATE_NOOP("Render", "Flowchart: Terminator"), QT_TRANSLATE_NOOP("Render", "Flowchart"), [](double w, double h, V) { return roundRect(w, h, h / 2); }, {}, {}, 0.1);
    add("fcPreparation", QT_TRANSLATE_NOOP("Render", "Flowchart: Preparation"), QT_TRANSLATE_NOOP("Render", "Flowchart"), [](double w, double h, V) {
        return poly({{w * 0.2, 0}, {w * 0.8, 0}, {w, h / 2}, {w * 0.8, h}, {w * 0.2, h}, {0, h / 2}}); }, {}, {}, 0.18);
    add("fcManualInput", QT_TRANSLATE_NOOP("Render", "Flowchart: Manual Input"), QT_TRANSLATE_NOOP("Render", "Flowchart"), [](double w, double h, V) { return poly({{0, h * 0.2}, {w, 0}, {w, h}, {0, h}}); },
        {}, {}, 0, [](double w, double h, V) { return QRectF(0, h * 0.2, w, h * 0.8); });
    add("fcManualOperation", QT_TRANSLATE_NOOP("Render", "Flowchart: Manual Operation"), QT_TRANSLATE_NOOP("Render", "Flowchart"), [](double w, double h, V) { return poly({{0, 0}, {w, 0}, {w * 0.8, h}, {w * 0.2, h}}); },
        {}, {}, 0.18);
    add("fcConnector", QT_TRANSLATE_NOOP("Render", "Flowchart: Connector"), QT_TRANSLATE_NOOP("Render", "Flowchart"), [](double w, double h, V) { return ellipse(w, h); }, {}, {}, 0.15);
    add("fcOffpage", QT_TRANSLATE_NOOP("Render", "Flowchart: Off-page Connector"), QT_TRANSLATE_NOOP("Render", "Flowchart"), [](double w, double h, V) {
        return poly({{0, 0}, {w, 0}, {w, h * 0.8}, {w / 2, h}, {0, h * 0.8}}); }, {}, {}, 0, [](double w, double h, V) { return QRectF(0, 0, w, h * 0.8); });
    add("fcCard", QT_TRANSLATE_NOOP("Render", "Flowchart: Card"), QT_TRANSLATE_NOOP("Render", "Flowchart"), [](double w, double h, V) { return poly({{w * 0.2, 0}, {w, 0}, {w, h}, {0, h}, {0, h * 0.2}}); },
        {}, {}, 0, [](double w, double h, V) { return QRectF(0, h * 0.2, w, h * 0.8); });
    add("fcPunchedTape", QT_TRANSLATE_NOOP("Render", "Flowchart: Punched Tape"), QT_TRANSLATE_NOOP("Render", "Flowchart"), [](double w, double h, V) {
        // (Each edge is two half ellipses, a quarter of the width by a tenth of the height each: a dip and a bulge.)
        QPainterPath p; p.moveTo(0, h / 10); p.arcTo(QRectF(0, 0, w / 2, h / 5), 180, 180); p.arcTo(QRectF(w / 2, 0, w / 2, h / 5), 180, -180); p.lineTo(w, h * 0.9);
        p.arcTo(QRectF(w / 2, h * 0.8, w / 2, h / 5), 0, 180); p.arcTo(QRectF(0, h * 0.8, w / 2, h / 5), 0, -180); p.closeSubpath(); return p; }, {}, {}, 0.2, nullptr);
    add("fcSummingJunction", QT_TRANSLATE_NOOP("Render", "Flowchart: Summing Junction"), QT_TRANSLATE_NOOP("Render", "Flowchart"), [](double w, double h, V) {
        QPainterPath p = ellipse(w, h); const double k = 0.3536;
        p.moveTo(w * (0.5 - k), h * (0.5 - k)); p.lineTo(w * (0.5 + k), h * (0.5 + k)); p.moveTo(w * (0.5 + k), h * (0.5 - k)); p.lineTo(w * (0.5 - k), h * (0.5 + k));
        return p; }, {}, {}, 0.15);
    add("fcOr", QT_TRANSLATE_NOOP("Render", "Flowchart: Or"), QT_TRANSLATE_NOOP("Render", "Flowchart"), [](double w, double h, V) {
        QPainterPath p = ellipse(w, h); p.moveTo(w / 2, 0); p.lineTo(w / 2, h); p.moveTo(0, h / 2); p.lineTo(w, h / 2); return p; }, {}, {}, 0.15);
    add("fcCollate", QT_TRANSLATE_NOOP("Render", "Flowchart: Collate"), QT_TRANSLATE_NOOP("Render", "Flowchart"), [](double w, double h, V) {
        QPainterPath p = poly({{0, 0}, {w, 0}, {w / 2, h / 2}}); p.addPolygon(QPolygonF({QPointF(w / 2, h / 2), QPointF(w, h), QPointF(0, h), QPointF(w / 2, h / 2)}));
        return p; }, {}, {}, 0.25);
    add("fcSort", QT_TRANSLATE_NOOP("Render", "Flowchart: Sort"), QT_TRANSLATE_NOOP("Render", "Flowchart"), [](double w, double h, V) {
        QPainterPath p = poly({{w / 2, 0}, {w, h / 2}, {w / 2, h}, {0, h / 2}}); p.moveTo(0, h / 2); p.lineTo(w, h / 2); return p; }, {}, {}, 0.25);
    add("fcExtract", QT_TRANSLATE_NOOP("Render", "Flowchart: Extract"), QT_TRANSLATE_NOOP("Render", "Flowchart"), [](double w, double h, V) { return poly({{w / 2, 0}, {w, h}, {0, h}}); },
        {}, {}, 0, [](double w, double h, V) { return QRectF(w * 0.25, h * 0.5, w * 0.5, h * 0.5); });
    add("fcMerge", QT_TRANSLATE_NOOP("Render", "Flowchart: Merge"), QT_TRANSLATE_NOOP("Render", "Flowchart"), [](double w, double h, V) { return poly({{0, 0}, {w, 0}, {w / 2, h}}); },
        {}, {}, 0, [](double w, double h, V) { return QRectF(w * 0.25, 0, w * 0.5, h * 0.5); });
    add("fcStoredData", QT_TRANSLATE_NOOP("Render", "Flowchart: Stored Data"), QT_TRANSLATE_NOOP("Render", "Flowchart"), [](double w, double h, V) {
        QPainterPath p; p.moveTo(w / 6, 0); p.lineTo(w, 0); p.arcTo(QRectF(w * 5 / 6, 0, w / 3, h), 90, 180); p.lineTo(w / 6, h); p.arcTo(QRectF(0, 0, w / 3, h), 270, -180);
        p.closeSubpath(); return p; }, {}, {}, 0, [](double w, double h, V) { return QRectF(w / 6, 0, w * 2 / 3, h); });
    add("fcDelay", QT_TRANSLATE_NOOP("Render", "Flowchart: Delay"), QT_TRANSLATE_NOOP("Render", "Flowchart"), [](double w, double h, V) {
        QPainterPath p; p.moveTo(0, 0); p.lineTo(w / 2, 0); p.arcTo(QRectF(0, 0, w, h), 90, -180); p.lineTo(0, h); p.closeSubpath(); return p; },
        {}, {}, 0, [](double w, double h, V) { return QRectF(0, h * 0.15, w * 0.85, h * 0.7); });
    add("fcSequential", QT_TRANSLATE_NOOP("Render", "Flowchart: Sequential Access Storage"), QT_TRANSLATE_NOOP("Render", "Flowchart"), [](double w, double h, V) {
        QPainterPath p = ellipse(w, h); p.moveTo(w / 2, h); p.lineTo(w, h); p.lineTo(w, h * 0.85); return p; }, {}, {}, 0.15);
    add("fcMagneticDisk", QT_TRANSLATE_NOOP("Render", "Flowchart: Magnetic Disk"), QT_TRANSLATE_NOOP("Render", "Flowchart"), [](double w, double h, V) {
        const double e = h / 3; QPainterPath p; p.moveTo(0, e / 2); p.arcTo(QRectF(0, 0, w, e), 180, -180); p.lineTo(w, h - e / 2);
        p.arcTo(QRectF(0, h - e, w, e), 0, -180); p.closeSubpath(); p.moveTo(0, e / 2); p.arcTo(QRectF(0, 0, w, e), 180, 180); return p; },
        {}, {}, 0, [](double w, double h, V) { return QRectF(0, h * 0.25, w, h * 0.62); });
    add("fcDirectAccess", QT_TRANSLATE_NOOP("Render", "Flowchart: Direct Access Storage"), QT_TRANSLATE_NOOP("Render", "Flowchart"), [](double w, double h, V) {
        const double e = w / 3; QPainterPath p; p.moveTo(e / 2, 0); p.lineTo(w - e / 2, 0); p.arcTo(QRectF(w - e, 0, e, h), 90, -180);
        p.lineTo(e / 2, h); p.arcTo(QRectF(0, 0, e, h), 270, -180); p.closeSubpath(); p.moveTo(w - e / 2, 0); p.arcTo(QRectF(w - e, 0, e, h), 90, 180); return p; },
        {}, {}, 0, [](double w, double h, V) { return QRectF(w * 0.12, 0, w * 0.62, h); });
    add("fcDisplay", QT_TRANSLATE_NOOP("Render", "Flowchart: Display"), QT_TRANSLATE_NOOP("Render", "Flowchart"), [](double w, double h, V) {
        QPainterPath p; p.moveTo(0, h / 2); p.lineTo(w / 6, 0); p.lineTo(w * 5 / 6, 0); p.arcTo(QRectF(w * 2 / 3, 0, w / 3, h), 90, -180); p.lineTo(w / 6, h);
        p.closeSubpath(); return p; }, {}, {}, 0, [](double w, double h, V) { return QRectF(w / 6, 0, w * 2 / 3, h); });
    // ---------------- Stars and Banners ----------------
    add("irregularSeal1", QT_TRANSLATE_NOOP("Render", "Explosion 1"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"), [](double w, double h, V) {
        // (The standard's points, in 21600ths of the box.)
        static const int pts[][2] = {{10800, 5800}, {14522, 0}, {14155, 5325}, {18380, 4457},
                                        {16702, 7315}, {21097, 8137}, {17607, 10475}, {21600, 13290},
                                        {16837, 12942}, {18145, 18095}, {14020, 14457}, {13247, 19737},
                                        {10532, 14935}, {8485, 21600}, {7715, 15627}, {4762, 17617},
                                        {5667, 13937}, {135, 14587}, {3722, 11775}, {0, 8615},
                                        {4627, 7617}, {370, 2295}, {7312, 6320}, {8352, 2295}};
        QVector<QPointF> v; for (const auto &p : pts) v << QPointF(p[0] * w / 21600, p[1] * h / 21600); return polyV(v); }, {}, {}, 0.3, nullptr);
    add("irregularSeal2", QT_TRANSLATE_NOOP("Render", "Explosion 2"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"), [](double w, double h, V) {
        // (The standard's points, in 21600ths of the box.)
        static const int pts[][2] = {{11462, 4342}, {14790, 0}, {14525, 5777}, {18007, 3172},
                                        {16380, 6532}, {21600, 6645}, {16985, 9402}, {18270, 11290},
                                        {16380, 12310}, {18877, 15632}, {14640, 14350}, {14942, 17370},
                                        {12180, 15935}, {11612, 18842}, {9872, 17370}, {8700, 19712},
                                        {7527, 18125}, {4917, 21600}, {4805, 18240}, {1285, 17825},
                                        {3330, 15370}, {0, 12877}, {3935, 11592}, {1172, 8270},
                                        {5372, 7817}, {4502, 3625}, {8550, 6382}, {9722, 1887}};
        QVector<QPointF> v; for (const auto &p : pts) v << QPointF(p[0] * w / 21600, p[1] * h / 21600); return polyV(v); }, {}, {}, 0.3, nullptr);
    add("star4", QT_TRANSLATE_NOOP("Render", "4-Point Star"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"), [](double w, double h, V a) { return starPath(w, h, 4, A(a, 0, 0.125) * 2); }, {0.125}, {}, 0.3);
    add("star5", QT_TRANSLATE_NOOP("Render", "5-Point Star"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"), [](double w, double h, V a) { return starPath(w, h, 5, A(a, 0, 0.38197) * 1.0); }, {0.38197}, {}, 0.32);
    add("star6", QT_TRANSLATE_NOOP("Render", "6-Point Star"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"), [](double w, double h, V a) { return starPath(w, h, 6, A(a, 0, 0.57736)); }, {0.57736}, {}, 0.25);
    add("star7", QT_TRANSLATE_NOOP("Render", "7-Point Star"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"), [](double w, double h, V a) { return starPath(w, h, 7, A(a, 0, 0.69202)); }, {0.69202}, {}, 0.25);
    add("star8", QT_TRANSLATE_NOOP("Render", "8-Point Star"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"), [](double w, double h, V a) { return starPath(w, h, 8, A(a, 0, 0.75)); }, {0.75}, {}, 0.22);
    add("star10", QT_TRANSLATE_NOOP("Render", "10-Point Star"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"), [](double w, double h, V a) { return starPath(w, h, 10, A(a, 0, 0.85066)); }, {0.85066}, {}, 0.2);
    add("star12", QT_TRANSLATE_NOOP("Render", "12-Point Star"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"), [](double w, double h, V a) { return starPath(w, h, 12, A(a, 0, 0.75)); }, {0.75}, {}, 0.2);
    add("star16", QT_TRANSLATE_NOOP("Render", "16-Point Star"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"), [](double w, double h, V a) { return starPath(w, h, 16, A(a, 0, 0.75)); }, {0.75}, {}, 0.18);
    add("star24", QT_TRANSLATE_NOOP("Render", "24-Point Star"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"), [](double w, double h, V a) { return starPath(w, h, 24, A(a, 0, 0.75)); }, {0.75}, {}, 0.16);
    add("star32", QT_TRANSLATE_NOOP("Render", "32-Point Star"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"), [](double w, double h, V a) { return starPath(w, h, 32, A(a, 0, 0.75)); }, {0.75}, {}, 0.15);
    add("ribbon2", QT_TRANSLATE_NOOP("Render", "Up Ribbon"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"), [](double w, double h, V) {
        QPainterPath p = poly({{0, h * 0.2}, {w * 0.12, h * 0.2}, {w * 0.12, 0}, {w * 0.88, 0}, {w * 0.88, h * 0.2}, {w, h * 0.2}, {w * 0.93, h * 0.55},
                               {w, h * 0.9}, {w * 0.88, h * 0.9}, {w * 0.88, h * 0.7}, {w * 0.12, h * 0.7}, {w * 0.12, h * 0.9}, {0, h * 0.9}, {w * 0.07, h * 0.55}});
        return p; }, {}, {}, 0, [](double w, double h, V) { return QRectF(w * 0.14, h * 0.03, w * 0.72, h * 0.64); });
    add("ribbon", QT_TRANSLATE_NOOP("Render", "Down Ribbon"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"), [](double w, double h, V) {
        return poly({{0, h * 0.1}, {w * 0.12, h * 0.1}, {w * 0.12, h * 0.3}, {w * 0.88, h * 0.3}, {w * 0.88, h * 0.1}, {w, h * 0.1}, {w * 0.93, h * 0.45},
                     {w, h * 0.8}, {w * 0.88, h * 0.8}, {w * 0.88, h}, {w * 0.12, h}, {w * 0.12, h * 0.8}, {0, h * 0.8}, {w * 0.07, h * 0.45}}); },
        {}, {}, 0, [](double w, double h, V) { return QRectF(w * 0.14, h * 0.33, w * 0.72, h * 0.64); });
    add("ellipseRibbon2", QT_TRANSLATE_NOOP("Render", "Curved Up Ribbon"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"), [](double w, double h, V) {
        QPainterPath p; p.moveTo(0, h * 0.3); p.lineTo(w * 0.12, h * 0.3); p.lineTo(w * 0.12, h * 0.1); p.quadTo(w / 2, -h * 0.08, w * 0.88, h * 0.1);
        p.lineTo(w * 0.88, h * 0.3); p.lineTo(w, h * 0.3); p.lineTo(w * 0.93, h * 0.62); p.lineTo(w, h * 0.95); p.lineTo(w * 0.88, h * 0.95);
        p.lineTo(w * 0.88, h * 0.75); p.quadTo(w / 2, h * 0.57, w * 0.12, h * 0.75); p.lineTo(w * 0.12, h * 0.95); p.lineTo(0, h * 0.95);
        p.lineTo(w * 0.07, h * 0.62); p.closeSubpath(); return p; }, {}, {}, 0, [](double w, double h, V) { return QRectF(w * 0.14, h * 0.12, w * 0.72, h * 0.5); });
    add("ellipseRibbon", QT_TRANSLATE_NOOP("Render", "Curved Down Ribbon"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"), [](double w, double h, V) {
        QPainterPath p; p.moveTo(0, h * 0.05); p.lineTo(w * 0.12, h * 0.05); p.lineTo(w * 0.12, h * 0.25); p.quadTo(w / 2, h * 0.43, w * 0.88, h * 0.25);
        p.lineTo(w * 0.88, h * 0.05); p.lineTo(w, h * 0.05); p.lineTo(w * 0.93, h * 0.38); p.lineTo(w, h * 0.7); p.lineTo(w * 0.88, h * 0.7);
        p.lineTo(w * 0.88, h * 0.9); p.quadTo(w / 2, h * 1.08, w * 0.12, h * 0.9); p.lineTo(w * 0.12, h * 0.7); p.lineTo(0, h * 0.7);
        p.lineTo(w * 0.07, h * 0.38); p.closeSubpath(); return p; }, {}, {}, 0, [](double w, double h, V) { return QRectF(w * 0.14, h * 0.4, w * 0.72, h * 0.5); });
    add("verticalScroll", QT_TRANSLATE_NOOP("Render", "Vertical Scroll"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"), [](double w, double h, V a) {
        const double r = A(a, 0, 0.125) * mn(w, h); QPainterPath p;
        p.addRoundedRect(QRectF(r, r, w - 2 * r, h - 2 * r), 2, 2);
        p.addEllipse(QRectF(r, 0, w - r, 2 * r)); p.addEllipse(QRectF(0, h - 2 * r, w - r, 2 * r)); return p.simplified(); },
        {0.125}, {}, 0.2);
    add("horizontalScroll", QT_TRANSLATE_NOOP("Render", "Horizontal Scroll"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"), [](double w, double h, V a) {
        const double r = A(a, 0, 0.125) * mn(w, h); QPainterPath p;
        p.addRoundedRect(QRectF(r, r, w - 2 * r, h - 2 * r), 2, 2);
        p.addEllipse(QRectF(0, r, 2 * r, h - r)); p.addEllipse(QRectF(w - 2 * r, 0, 2 * r, h - r)); return p.simplified(); },
        {0.125}, {}, 0.2);
    add("wave", QT_TRANSLATE_NOOP("Render", "Wave"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"), [](double w, double h, V a) { return wavePath(w, h, A(a, 0, 0.125), 1); }, {0.125}, {}, 0.2);
    add("doubleWave", QT_TRANSLATE_NOOP("Render", "Double Wave"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"), [](double w, double h, V a) { return wavePath(w, h, A(a, 0, 0.0625), 2); }, {0.0625}, {}, 0.15);

    // ---------------- Callouts ----------------
    add("wedgeRectCallout", QT_TRANSLATE_NOOP("Render", "Rectangular Callout"), QT_TRANSLATE_NOOP("Render", "Callouts"), [](double w, double h, V a) { return wedgeRect(w, h, A(a, 0, -0.20833), A(a, 1, 0.625)); }, {-0.20833, 0.625}, {}, 0, [](double w, double h, V) { return QRectF(0, 0, w, h); });
    add("wedgeRoundRectCallout", QT_TRANSLATE_NOOP("Render", "Rounded Rectangular Callout"), QT_TRANSLATE_NOOP("Render", "Callouts"), [](double w, double h, V a) { return wedgeRoundRect(w, h, A(a, 0, -0.20833), A(a, 1, 0.625), A(a, 2, 0.16667)); }, {-0.20833, 0.625, 0.16667}, {}, 0, [](double w, double h, V a) { const double u = std::clamp(A(a, 2, 0.16667), 0.0, 0.5) * mn(w, h) * 0.29289; return QRectF(u, u, w - 2 * u, h - 2 * u); });
    add("wedgeEllipseCallout", QT_TRANSLATE_NOOP("Render", "Oval Callout"), QT_TRANSLATE_NOOP("Render", "Callouts"), [](double w, double h, V a) { return wedgeEllipse(w, h, A(a, 0, -0.20833), A(a, 1, 0.625)); }, {-0.20833, 0.625}, {}, 0, [](double w, double h, V) { return QRectF(w * 0.14645, h * 0.14645, w * 0.70711, h * 0.70711); });
    add("cloudCallout", QT_TRANSLATE_NOOP("Render", "Cloud Callout"), QT_TRANSLATE_NOOP("Render", "Callouts"), [](double w, double h, V) {
        QPainterPath p;
        const QRectF c[] = {{0.05, 0.25, 0.35, 0.35}, {0.2, 0.05, 0.35, 0.35}, {0.45, 0.03, 0.35, 0.37}, {0.62, 0.2, 0.36, 0.38},
                            {0.45, 0.42, 0.38, 0.36}, {0.15, 0.42, 0.38, 0.36}, {0.0, 0.35, 0.3, 0.28},
                            {0.25, 0.22, 0.5, 0.38}};   // the middle
        for (const QRectF &r : c) { QPainterPath e; e.addEllipse(QRectF(r.x() * w, r.y() * h, r.width() * w, r.height() * h)); p = p.united(e); }
        p.addEllipse(QRectF(w * 0.18, h * 0.82, w * 0.08, h * 0.07)); p.addEllipse(QRectF(w * 0.1, h * 0.92, w * 0.05, h * 0.05)); return p; },
        {}, {}, 0, [](double w, double h, V) { return QRectF(w * 0.18, h * 0.15, w * 0.64, h * 0.52); });
    add("borderCallout1", QT_TRANSLATE_NOOP("Render", "Line Callout 1"), QT_TRANSLATE_NOOP("Render", "Callouts"), [](double w, double h, V) {
        QPainterPath p; p.addRect(0, 0, w, h); p.moveTo(-w * 0.08333, h * 0.1875); p.lineTo(-w * 0.38333, h * 1.125); return p; }, {}, {}, 0, [](double w, double h, V) { return QRectF(w * 0.04, h * 0.04, w * 0.92, h * 0.92); });
    add("borderCallout2", QT_TRANSLATE_NOOP("Render", "Line Callout 2"), QT_TRANSLATE_NOOP("Render", "Callouts"), [](double w, double h, V) {
        QPainterPath p; p.addRect(0, 0, w, h); p.moveTo(-w * 0.08333, h * 0.1875); p.lineTo(-w * 0.16667, h * 0.1875); p.lineTo(-w * 0.46667, h * 1.125); return p; }, {}, {}, 0, [](double w, double h, V) { return QRectF(w * 0.04, h * 0.04, w * 0.92, h * 0.92); });
    add("accentCallout1", QT_TRANSLATE_NOOP("Render", "Line Callout 1 (Accent Bar)"), QT_TRANSLATE_NOOP("Render", "Callouts"), [](double w, double h, V) {
        QPainterPath p; p.addRect(0, 0, w, h); p.moveTo(-w * 0.08333, 0); p.lineTo(-w * 0.08333, h); p.moveTo(-w * 0.08333, h * 0.1875);
        p.lineTo(-w * 0.38333, h * 1.125); return p; }, {}, {}, 0, [](double w, double h, V) { return QRectF(w * 0.04, h * 0.04, w * 0.92, h * 0.92); });
    add("callout1", QT_TRANSLATE_NOOP("Render", "Line Callout 1 (No Border)"), QT_TRANSLATE_NOOP("Render", "Callouts"), [](double w, double h, V) {
        QPainterPath p; p.addRect(0, 0, w, h); p.moveTo(-w * 0.08333, h * 0.1875); p.lineTo(-w * 0.38333, h * 1.125); return p; }, {}, {}, 0, [](double w, double h, V) { return QRectF(w * 0.04, h * 0.04, w * 0.92, h * 0.92); });
    // ---------------- Action / misc ----------------
    add("actionButtonBlank", QT_TRANSLATE_NOOP("Render", "Action Button: Blank"), QT_TRANSLATE_NOOP("Render", "Action Buttons"), [](double w, double h, V) {
        QPainterPath p; p.addRect(0, 0, w, h); const double d = mn(w, h) * 0.08; p.addRect(d, d, w - 2 * d, h - 2 * d); return p; }, {}, {}, 0.1);
    add("actionButtonHome", QT_TRANSLATE_NOOP("Render", "Action Button: Home"), QT_TRANSLATE_NOOP("Render", "Action Buttons"), [](double w, double h, V) { return actionButton(w, h, "Home"); }, {}, {}, 0.1);
    L.back().slideAction = true;
    L.back().link = QStringLiteral("#first");
    add("actionButtonHelp", QT_TRANSLATE_NOOP("Render", "Action Button: Help"), QT_TRANSLATE_NOOP("Render", "Action Buttons"), [](double w, double h, V) { return actionButton(w, h, "Help"); }, {}, {}, 0.1);
    L.back().slideAction = true;
    L.back().link = QStringLiteral("");
    add("actionButtonInformation", QT_TRANSLATE_NOOP("Render", "Action Button: Information"), QT_TRANSLATE_NOOP("Render", "Action Buttons"), [](double w, double h, V) { return actionButton(w, h, "Information"); }, {}, {}, 0.1);
    L.back().slideAction = true;
    L.back().link = QStringLiteral("");
    add("actionButtonForwardNext", QT_TRANSLATE_NOOP("Render", "Action Button: Forward or Next"), QT_TRANSLATE_NOOP("Render", "Action Buttons"), [](double w, double h, V) { return actionButton(w, h, "ForwardNext"); }, {}, {}, 0.1);
    L.back().slideAction = true;
    L.back().link = QStringLiteral("#next");
    add("actionButtonBackPrevious", QT_TRANSLATE_NOOP("Render", "Action Button: Back or Previous"), QT_TRANSLATE_NOOP("Render", "Action Buttons"), [](double w, double h, V) { return actionButton(w, h, "BackPrevious"); }, {}, {}, 0.1);
    L.back().slideAction = true;
    L.back().link = QStringLiteral("#previous");
    add("actionButtonEnd", QT_TRANSLATE_NOOP("Render", "Action Button: End"), QT_TRANSLATE_NOOP("Render", "Action Buttons"), [](double w, double h, V) { return actionButton(w, h, "End"); }, {}, {}, 0.1);
    L.back().slideAction = true;
    L.back().link = QStringLiteral("#last");
    add("actionButtonBeginning", QT_TRANSLATE_NOOP("Render", "Action Button: Beginning"), QT_TRANSLATE_NOOP("Render", "Action Buttons"), [](double w, double h, V) { return actionButton(w, h, "Beginning"); }, {}, {}, 0.1);
    L.back().slideAction = true;
    L.back().link = QStringLiteral("#first");
    add("actionButtonReturn", QT_TRANSLATE_NOOP("Render", "Action Button: Return"), QT_TRANSLATE_NOOP("Render", "Action Buttons"), [](double w, double h, V) { return actionButton(w, h, "Return"); }, {}, {}, 0.1);
    L.back().slideAction = true;
    L.back().link = QStringLiteral("#previous");
    add("actionButtonDocument", QT_TRANSLATE_NOOP("Render", "Action Button: Document"), QT_TRANSLATE_NOOP("Render", "Action Buttons"), [](double w, double h, V) { return actionButton(w, h, "Document"); }, {}, {}, 0.1);
    L.back().slideAction = true;
    L.back().link = QStringLiteral("");
    add("actionButtonSound", QT_TRANSLATE_NOOP("Render", "Action Button: Sound"), QT_TRANSLATE_NOOP("Render", "Action Buttons"), [](double w, double h, V) { return actionButton(w, h, "Sound"); }, {}, {}, 0.1);
    L.back().slideAction = true;
    L.back().link = QStringLiteral("");
    add("actionButtonMovie", QT_TRANSLATE_NOOP("Render", "Action Button: Movie"), QT_TRANSLATE_NOOP("Render", "Action Buttons"), [](double w, double h, V) { return actionButton(w, h, "Movie"); }, {}, {}, 0.1);
    L.back().slideAction = true;
    L.back().link = QStringLiteral("");
    add("tab", QT_TRANSLATE_NOOP("Render", "Tab"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V) { return poly({{w * 0.15, 0}, {w * 0.85, 0}, {w, h}, {0, h}}); }, {}, {}, 0.18);
    add("flowArrow", QT_TRANSLATE_NOOP("Render", "Flow Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"), [](double w, double h, V) {
        return poly({{0, h * 0.2}, {w * 0.6, h * 0.2}, {w * 0.6, 0}, {w, h / 2}, {w * 0.6, h}, {w * 0.6, h * 0.8}, {0, h * 0.8}, {w * 0.15, h / 2}}); }, {}, {}, 0.2);
    return L;
}

const QVector<ShapeDef> &shapeLibrary()
{
    static const QVector<ShapeDef> lib = build();
    return lib;
}

const ShapeDef *shapeDef(const QString &id)
{
    for (const auto &d : shapeLibrary())
        if (d.id == id) return &d;
    return nullptr;
}

QVector<double> shapeAdj(const QString &id, const QVector<double> &adj)
{
    const ShapeDef *d = shapeDef(id);
    QVector<double> out = d ? d->defaults : QVector<double>();
    for (int i = 0; i < adj.size() && i < out.size(); ++i) out[i] = adj[i];
    return out;
}

QVector<double> adjFromOldLibrary(const QString &id, const QSizeF &size, const QVector<double> &adj)
{
    // (What each shape's list of defaults was, for the shapes whose adjustments changed their meaning or their defaults.)
    static const QHash<QString, QVector<double>> old = {
        {"parallelogram", {0.25}}, {"trapezoid", {0.25}}, {"hexagon", {0.25}}, {"halfFrame", {0.333}}, {"leftUpArrow", {0.18, 0.22}},
        {"mathEqual", {0.235}}, {"mathDivide", {0.235}}, {"star4", {0.19}}, {"star5", {0.38}}, {"star6", {0.58}}, {"star7", {0.6}}, {"star8", {0.72}},
        {"star10", {0.76}}, {"star12", {0.78}}, {"star16", {0.82}}, {"star24", {0.86}}, {"star32", {0.88}}, {"mathPlus", {0.235}}, {"mathMinus", {0.235}},
        {"mathMultiply", {0.235}}, {"mathNotEqual", {0.235}}, {"wedgeRectCallout", {0.2, 1.0}}, {"wedgeRoundRectCallout", {0.2, 1.0}}, {"wedgeEllipseCallout", {0.18, 1.0}}};
    const auto at = old.constFind(id);
    if (at == old.cend()) return adj;
    QVector<double> a = at.value();
    for (int i = 0; i < adj.size(); ++i) {
        if (i < a.size()) a[i] = adj[i];
        else a.append(adj[i]);
    }
    const double w = std::max(0.5, size.width()), h = std::max(0.5, size.height()), m = std::min(w, h);
    if (id == QLatin1String("parallelogram") || id == QLatin1String("trapezoid") || id == QLatin1String("hexagon")) {
        a[0] = a[0] * w / m;
    } else if (id == QLatin1String("halfFrame")) {
        a = {a[0], a[0]};
    } else if (id == QLatin1String("mathEqual")) {
        a = {a[0] * 0.8, a[0] * 0.48};
    } else if (id == QLatin1String("mathNotEqual")) {
        a = {a[0] * 0.8, 66.0, a[0] * 0.48};   // (the slash was turned 20 degrees: the standard's 110)
    } else if (id == QLatin1String("mathDivide")) {
        a = {a[0] * 0.6, a[0] * 0.396, a[0] * 0.33};
    } else if (id.startsWith(QLatin1String("wedge"))) {
        a[0] -= 0.5;
        a[1] -= 0.5;
        // (The rounded one's corner was 0.16 of the box above its pointer, now a share of the frame's shorter side.)
        if (id == QLatin1String("wedgeRoundRectCallout")) a = {a[0], a[1], 0.16 * std::min(w, h * 0.78) / m};
    }
    return a;
}

QPainterPath shapePath(const QString &id, const QSizeF &size, const QVector<double> &adj)
{
    const ShapeDef *d = shapeDef(id);
    if (!d) d = &shapeLibrary().front();
    const double w = std::max(0.5, size.width()), h = std::max(0.5, size.height());
    QPainterPath p = d->path(w, h, shapeAdj(d->id, adj));
    // Shapes with holes need even-odd; unions of overlapping parts need winding.
    static const QStringList holes = {"frame", "donut", "noSmoking", "bevel", "halfFrame"};
    p.setFillRule(holes.contains(d->id) ? Qt::OddEvenFill : Qt::WindingFill);
    return p;
}

QRectF shapeTextRect(const QString &id, const QSizeF &size, const QVector<double> &adj)
{
    const ShapeDef *d = shapeDef(id);
    const double w = size.width(), h = size.height();
    if (!d) return QRectF(0, 0, w, h);
    if (d->textRect) return d->textRect(w, h, shapeAdj(id, adj));
    return insetRect(w, h, d->inset, d->inset);
}

QPointF adjHandlePos(const AdjHandle &hd, const QSizeF &s, const QVector<double> &adj)
{
    const double basis = hd.basis == 0 ? s.width() : hd.basis == 1 ? s.height() : std::min(s.width(), s.height());
    const double v = adj.value(hd.index) * basis;
    if (hd.axis == 0) return QPointF(hd.fromEnd ? s.width() - v : v, hd.other * s.height());
    return QPointF(hd.other * s.width(), hd.fromEnd ? s.height() - v : v);
}

double adjFromPoint(const AdjHandle &hd, const QSizeF &s, const QPointF &p)
{
    const double basis = std::max(0.01, hd.basis == 0 ? s.width() : hd.basis == 1 ? s.height() : std::min(s.width(), s.height()));
    double v = hd.axis == 0 ? p.x() : p.y();
    if (hd.fromEnd) v = (hd.axis == 0 ? s.width() : s.height()) - v;
    const double most = hd.limit > 0 ? hd.limit * (hd.axis == 0 ? s.width() : s.height()) / basis : hd.max;
    return std::clamp(v / basis, hd.min, std::max(hd.min, most));
}

} // namespace jp
