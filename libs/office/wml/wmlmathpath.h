#pragma once
// Equations as they are drawn: the math markup (m:oMath, m:oMathPara; see wmlmath.h for the line of text the word processor's pages show) laid out in two dimensions, with stacked fractions,
// scripts, roots, sums and integrals with limits, brackets that grow, accents, and matrices, as outlines (a QPainterPath), so that a page, a sheet, a PDF, and a printer all draw the same vector shape.
//
// What the markup holds is bounded as every equation is (wml.h): nested at most kMaxEquationDepth deep, at most kMaxEquationChars characters of text, and a fixed number of elements; what is
// past that is left out of the drawing and `cut` says so.

#include <QByteArray>
#include <QColor>
#include <QPainterPath>
#include <QRectF>
#include <QString>

class QPainter;

namespace jp::office::wml {

struct EquationShape {
    QPainterPath path;      // the outlines, with the top left of `box` at 0, 0
    QRectF box;             // the room the equation takes (its ink, and the space the layout keeps around it)
    double baseline = 0;    // the distance from the top of `box` to the baseline of the first line
    bool cut = false;       // not all of the markup was drawn (too much text, too many elements, or nested too deep)
    bool valid() const { return !box.isEmpty(); }
};

// The markup (an m:oMath or m:oMathPara element; a prefix it uses may be left undeclared) drawn with letters `emSize` points tall in the family (the bundled "DejaVu Sans" by default). An
// empty shape for markup that is not an equation.
EquationShape equationShape(const QByteArray &markup, double emSize, const QString &family = QString());

// Draws the equation into `box` with the painter, as filled outlines in `color` (vector: a PDF or a printer gets paths, not a picture). It keeps its proportions, as large as fits, in the middle.
// False, and nothing drawn, for markup that is not an equation.
bool paintEquation(QPainter *painter, const QByteArray &markup, const QRectF &box, const QColor &color);

// A picture (PNG) of the equation in `color` on a transparent ground, `width` by `height` pixels (at most 4,000 each), the equation as paintEquation sets it; empty for markup that is not an
// equation. For the formats that have no equation: the picture stands for it.
QByteArray equationPng(const QByteArray &markup, int width, int height, const QColor &color = Qt::black);

// The markup with the namespace of the math elements declared on its first element, if it does not declare it (an element copied out from under a root that did).
QByteArray equationMarkupDeclared(const QByteArray &markup);

} // namespace jp::office::wml
