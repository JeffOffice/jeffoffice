#include "core/ink.h"

#include "core/color.h"
#include "core/items.h"

#include <QCoreApplication>
#include <QLineF>
#include <algorithm>
#include <cmath>

namespace jp::deck {

namespace {

double scaleFor(double pageWidth) { return pageWidth > 0 ? pageWidth / kInkSlideWidth : 1.0; }

// Keeps the points of a stroke that matter, in one pass: from a point the run goes on while the points stay within `tolerance` of the line
// through it and the point after it (and keep going the same way along it); the last such point is kept, and the next run starts there.
// Each point is looked at about twice, however the stroke winds.
thread_local qint64 g_pointsLooked = 0;

QVector<QPointF> thinned(const QVector<QPointF> &points, double tolerance)
{
    // The same point twice in a row adds nothing.
    QVector<QPointF> p;
    for (const QPointF &pt : points)
        if (p.isEmpty() || p.last() != pt) p << pt;
    if (p.size() < 3) return p;
    const int n = int(p.size());
    QVector<QPointF> out{p.first()};
    int i = 0;
    while (i < n - 1) {
        const QPointF a = p[i], d = p[i + 1] - a;
        const double length = std::hypot(d.x(), d.y());
        double along = length;   // how far along the line the last point of the run is
        int j = i + 2;
        for (; j < n; ++j, ++g_pointsLooked) {
            const QPointF v = p[j] - a;
            const double across = std::abs(v.x() * d.y() - v.y() * d.x()) / length, ahead = (v.x() * d.x() + v.y() * d.y()) / length;
            // Off the line, or turned back along it.
            if (across > tolerance || ahead < along - tolerance) break;
            along = std::max(along, ahead);
        }
        out << p[j - 1];
        i = j - 1;
    }
    return out;
}

} // namespace

qint64 inkPointsLookedAtForTest() { return g_pointsLooked; }

double penWeightFor(double pageWidth) { return kPenWeight * scaleFor(pageWidth); }
double highlighterWeightFor(double pageWidth) { return kHighlighterWeight * scaleFor(pageWidth); }
double eraserReachFor(double pageWidth) { return kEraserReach * scaleFor(pageWidth); }
QColor highlighterColor() { return QColor(255, 255, 0); }

QVector<InkColor> inkColors()
{
    auto name = [](const char *text) { return QCoreApplication::translate("Ink", text); };
    return {{name(QT_TRANSLATE_NOOP("Ink", "Red")), QColor(255, 0, 0)},       {name(QT_TRANSLATE_NOOP("Ink", "Orange")), QColor(255, 140, 0)},
            {name(QT_TRANSLATE_NOOP("Ink", "Yellow")), QColor(255, 255, 0)},  {name(QT_TRANSLATE_NOOP("Ink", "Green")), QColor(0, 176, 80)},
            {name(QT_TRANSLATE_NOOP("Ink", "Light Blue")), QColor(0, 176, 240)}, {name(QT_TRANSLATE_NOOP("Ink", "Blue")), QColor(0, 112, 192)},
            {name(QT_TRANSLATE_NOOP("Ink", "Purple")), QColor(112, 48, 160)}, {name(QT_TRANSLATE_NOOP("Ink", "Black")), QColor(0, 0, 0)},
            {name(QT_TRANSLATE_NOOP("Ink", "White")), QColor(255, 255, 255)}};
}

QString inkColorName(const QColor &color)
{
    const QColor rgb(color.red(), color.green(), color.blue());
    for (const InkColor &c : inkColors())
        if (c.color == rgb) return c.name;
    return rgb.name(QColor::HexRgb).toUpper();
}

QPainterPath inkPath(const InkStroke &stroke)
{
    QPainterPath path;
    if (stroke.points.isEmpty()) return path;
    path.moveTo(stroke.points.first());
    if (stroke.points.size() == 1) path.lineTo(stroke.points.first() + QPointF(0.01, 0));
    for (int i = 1; i < stroke.points.size(); ++i) path.lineTo(stroke.points[i]);
    return path;
}

bool inkTouches(const InkStroke &stroke, const QPointF &p, double reach)
{
    const double r = reach + stroke.width / 2;
    if (stroke.points.size() == 1) return QLineF(stroke.points.first(), p).length() <= r;
    for (int i = 1; i < stroke.points.size(); ++i) {
        const QLineF seg(stroke.points[i - 1], stroke.points[i]);
        const double len2 = seg.dx() * seg.dx() + seg.dy() * seg.dy();
        double t = len2 > 0 ? ((p.x() - seg.x1()) * seg.dx() + (p.y() - seg.y1()) * seg.dy()) / len2 : 0;
        t = std::clamp(t, 0.0, 1.0);
        if (QLineF(seg.pointAt(t), p).length() <= r) return true;
    }
    return false;
}

ItemPtr inkShape(const InkStroke &stroke)
{
    if (stroke.points.isEmpty()) return nullptr;
    InkStroke kept = stroke;
    kept.points = thinned(stroke.points, std::max(0.05, stroke.width * 0.1));
    const QPainterPath path = inkPath(kept);
    QRectF bounds = path.boundingRect();
    if (bounds.width() < 1) bounds.adjust(-0.5, 0, 0.5, 0);
    if (bounds.height() < 1) bounds.adjust(0, -0.5, 0, 0.5);
    auto shape = std::make_shared<ShapeItem>();
    shape->rect = bounds;
    shape->customPath = path.translated(-bounds.topLeft());
    shape->fill = Fill::none();
    Stroke line = Stroke::line(ColorRef::rgb(QColor(stroke.color.red(), stroke.color.green(), stroke.color.blue())), stroke.width);
    line.cap = stroke.highlighter ? Qt::SquareCap : Qt::RoundCap;
    line.join = Qt::RoundJoin;
    line.transparency = stroke.highlighter ? kHighlighterClearness : 0;
    shape->stroke = line;
    shape->wrap.mode = Wrap::None;
    return shape;
}

void addInk(Page &slide, const QVector<InkStroke> &strokes)
{
    int count = 0;
    for (const ItemPtr &it : slide.items)
        if (it->name.startsWith(QLatin1String("Ink "))) ++count;
    for (const InkStroke &stroke : strokes) {
        ItemPtr shape = inkShape(stroke);
        if (!shape) continue;
        shape->name = QStringLiteral("Ink %1").arg(++count);
        slide.items.push_back(shape);
    }
}

} // namespace jp::deck
