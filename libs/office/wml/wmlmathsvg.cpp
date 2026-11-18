#include "wml/wmlmathsvg.h"

#include "wml/wmlmathpath.h"

#include <QPainterPath>

#include <algorithm>
#include <cmath>

namespace jp::office::wml {

namespace {

// A number in the path data: at most three decimals, none for a whole number.
void number(QByteArray *out, double v)
{
    QByteArray n = QByteArray::number(v, 'f', 3);
    if (n.contains('.')) {
        while (n.endsWith('0')) n.chop(1);
        if (n.endsWith('.')) n.chop(1);
    }
    out->append(n == "-0" ? QByteArray("0") : n);
}

} // namespace

EquationSvg equationSvg(const QByteArray &markup, double fontPoints, const QColor &color)
{
    EquationSvg out;
    const auto shape = equationShapeCached(markup);
    if (!shape || !shape->valid() || shape->cut) return out;
    const double k = std::clamp(fontPoints, 1.0, 1000.0) / kEquationLayoutSize;
    const double w = shape->box.width() * k, h = shape->box.height() * k;
    if (!(w > 0) || !(h > 0) || !std::isfinite(w) || !std::isfinite(h) || w > 20000 || h > 20000) return out;
    QByteArray d;
    auto at = [&](const QPainterPath::Element &e) {
        number(&d, (e.x - shape->box.left()) * k);
        d += ' ';
        number(&d, (e.y - shape->box.top()) * k);
    };
    const QPainterPath &path = shape->path;
    for (int i = 0; i < path.elementCount(); ++i) {
        const QPainterPath::Element e = path.elementAt(i);
        if (e.type == QPainterPath::MoveToElement) { d += 'M'; at(e); }
        else if (e.type == QPainterPath::LineToElement) { d += 'L'; at(e); }
        else if (e.type == QPainterPath::CurveToElement && i + 2 < path.elementCount()) {
            d += 'C';
            at(e);
            d += ' ';
            at(path.elementAt(i + 1));
            d += ' ';
            at(path.elementAt(i + 2));
            i += 2;
        }
        if (d.size() > kMostEquationSvgBytes) return out;
    }
    if (d.isEmpty()) return out;
    const QByteArray size = QByteArray::number(w, 'f', 3) + ' ' + QByteArray::number(h, 'f', 3);
    out.svg = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" + QByteArray::number(w, 'f', 3) + "\" height=\"" + QByteArray::number(h, 'f', 3) + "\" viewBox=\"0 0 " + size +
              "\"><path fill=\"" + color.name(QColor::HexRgb).toLatin1() + "\" fill-rule=\"" + (path.fillRule() == Qt::OddEvenFill ? "evenodd" : "nonzero") + "\" d=\"" + d + "\"/></svg>";
    out.size = QSizeF(w, h);
    out.ascent = std::clamp(shape->baseline * k, 0.0, h);
    return out;
}

} // namespace jp::office::wml
