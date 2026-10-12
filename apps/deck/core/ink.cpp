#include "core/ink.h"

#include "core/color.h"
#include "core/items.h"

#include <QLineF>
#include <algorithm>
#include <cmath>

namespace jp::deck {

namespace {

double scaleFor(double pageWidth) { return pageWidth > 0 ? pageWidth / kInkSlideWidth : 1.0; }

// Keeps the points of a run that matter at `tolerance`: the farthest from the chord stays, and each side is thinned in turn.
void thin(const QVector<QPointF> &in, int a, int b, double tolerance, QVector<bool> &keep)
{
    // A stack, not recursion: a stroke has as many points as the pointer moved through.
    QVector<QPair<int, int>> todo{{a, b}};
    while (!todo.isEmpty()) {
        const auto [lo, hi] = todo.takeLast();
        if (hi <= lo + 1) continue;
        const QLineF chord(in[lo], in[hi]);
        double far = -1;
        int at = -1;
        for (int i = lo + 1; i < hi; ++i) {
            double dist;
            if (chord.length() > 1e-9) {
                const QPointF d = in[i] - in[lo], u = (in[hi] - in[lo]) / chord.length();
                dist = std::abs(d.x() * u.y() - d.y() * u.x());
            } else {
                dist = QLineF(in[lo], in[i]).length();
            }
            if (dist > far) {
                far = dist;
                at = i;
            }
        }
        if (far <= tolerance) continue;
        keep[at] = true;
        todo << qMakePair(lo, at) << qMakePair(at, hi);
    }
}

QVector<QPointF> thinned(const QVector<QPointF> &points, double tolerance)
{
    // The same point twice in a row adds nothing.
    QVector<QPointF> p;
    for (const QPointF &pt : points)
        if (p.isEmpty() || p.last() != pt) p << pt;
    if (p.size() < 3) return p;
    QVector<bool> keep(p.size(), false);
    keep.first() = keep.last() = true;
    thin(p, 0, int(p.size()) - 1, tolerance, keep);
    QVector<QPointF> out;
    for (int i = 0; i < p.size(); ++i)
        if (keep[i]) out << p[i];
    return out;
}

} // namespace

double penWeightFor(double pageWidth) { return kPenWeight * scaleFor(pageWidth); }
double highlighterWeightFor(double pageWidth) { return kHighlighterWeight * scaleFor(pageWidth); }
double eraserReachFor(double pageWidth) { return kEraserReach * scaleFor(pageWidth); }
QColor highlighterColor() { return QColor(255, 255, 0); }

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
