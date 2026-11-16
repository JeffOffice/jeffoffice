#include "wml/wmlwrap.h"

#include "core/wrapoutline.h"

#include <algorithm>
#include <cmath>

namespace jp::office::wml {

QPolygonF readWrapPolygon(const XmlTree &t, int wrap)
{
    int poly = -1;
    for (int c = wrap >= 0 ? t.firstChild(wrap) : -1; c >= 0 && poly < 0; c = t.next(c))
        if (t.localName(c) == "wrapPolygon") poly = c;
    if (poly < 0) return QPolygonF();
    QVector<QPointF> points;
    auto take = [&](int n) {
        const double x = t.attr(n, "x").toDouble(), y = t.attr(n, "y").toDouble();
        points.push_back(QPointF(x / jp::kWrapPolygonUnits, y / jp::kWrapPolygonUnits));
    };
    for (int c = t.firstChild(poly); c >= 0 && points.size() < jp::kMostWrapPoints; c = t.next(c)) {
        const QByteArray name = t.localName(c);
        if (name == "start" || name == "lineTo") take(c);
    }
    // (A polygon is closed by a last point where it began: that point is the closing, not another corner.)
    if (points.size() > 3 && points.first() == points.last()) points.removeLast();
    return jp::boundedWrapOutline(points);
}

QPolygonF readWrapPolygonXml(const QByteArray &wrapElement)
{
    XmlTree t;
    static const QByteArray open = "<x xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\">";
    if (!t.parse(open + wrapElement + "</x>")) return QPolygonF();
    return readWrapPolygon(t, t.firstChild(t.root()));
}

QByteArray wrapPolygonXml(const QPolygonF &outline, bool edited)
{
    const QPolygonF kept = jp::boundedWrapOutline(QVector<QPointF>(outline.begin(), outline.end()));
    if (kept.isEmpty()) return QByteArray();
    auto unit = [](double v) { return QByteArray::number(qint64(std::llround(v * jp::kWrapPolygonUnits))); };
    QByteArray out = edited ? "<wp:wrapPolygon edited=\"1\">" : "<wp:wrapPolygon edited=\"0\">";
    out += "<wp:start x=\"" + unit(kept[0].x()) + "\" y=\"" + unit(kept[0].y()) + "\"/>";
    for (int i = 1; i < kept.size(); ++i) out += "<wp:lineTo x=\"" + unit(kept[i].x()) + "\" y=\"" + unit(kept[i].y()) + "\"/>";
    out += "<wp:lineTo x=\"" + unit(kept[0].x()) + "\" y=\"" + unit(kept[0].y()) + "\"/></wp:wrapPolygon>";
    return out;
}

} // namespace jp::office::wml
