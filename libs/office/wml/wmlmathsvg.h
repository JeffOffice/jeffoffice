#pragma once
// An equation as a picture of its outlines (SVG), for a page that sets it in a line of text: the layout of wmlmathpath.h at the letters' size, one filled path, sized in points. The page
// shows, prints, and saves it by the same picture path as any other drawing, so it is the same shape in all three.

#include <QByteArray>
#include <QColor>
#include <QSizeF>

namespace jp::office::wml {

struct EquationSvg {
    QByteArray svg;         // empty for markup that is not an equation, or that is cut short (nested too deep, or too much of it): the page keeps its line of text
    QSizeF size;            // the picture's width and height in points
    double ascent = 0;      // the part of the height above the line's baseline (the rest, `size.height() - ascent`, hangs below it)
    bool valid() const { return !svg.isEmpty(); }
};

// The most bytes of one equation's picture (a larger one stays as its line of text).
constexpr int kMostEquationSvgBytes = 1 << 20;
// The picture of the markup (an m:oMath or m:oMathPara element) with letters of `fontPoints` points, filled with `color`. The text of the layout's letters is the font size given, so the
// picture grows with the run's size; `fontPoints` is held to 1 to 1,000.
EquationSvg equationSvg(const QByteArray &markup, double fontPoints, const QColor &color = Qt::black);

} // namespace jp::office::wml
