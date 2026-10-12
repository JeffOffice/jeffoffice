#pragma once
// Ink drawn on the slides during a slide show with the pen or the
// highlighter: one stroke is a run of points in the slide's own units
// (points), a color, and a weight. Kept ink becomes freeform line shapes on
// the slide, one for each stroke, so it is saved with the presentation and
// other programs show it: the pen is opaque and thin, the highlighter wide
// and see-through. The weights are for a slide 960 points across and grow and
// shrink with the slide, so that the stroke looks the same on the screen as
// on the slide (a 4:3 slide is 720 points across).

#include "core/document.h"

#include <QColor>
#include <QHash>
#include <QPainterPath>
#include <QPointF>
#include <QVector>

namespace jp::deck {

struct InkStroke {
    QVector<QPointF> points;   // slide points
    QColor color;
    double width = 2.5;        // points
    bool highlighter = false;
};

// What the pen and highlighter weigh and how clear the highlighter is, for a slide this wide (960 points: 13.333 inches).
constexpr double kInkSlideWidth = 960, kPenWeight = 2.5, kHighlighterWeight = 18, kHighlighterClearness = 0.5;
// How far from a stroke the eraser reaches (points, same slide).
constexpr double kEraserReach = 8;

// The weight a stroke gets on a slide `pageWidth` points across.
double penWeightFor(double pageWidth);
double highlighterWeightFor(double pageWidth);
double eraserReachFor(double pageWidth);
// The default color of the highlighter (a yellow).
QColor highlighterColor();
// The colors the pen and the highlighter can be chosen in, by name (Red, Orange, Yellow, Green, Light Blue, Blue, Purple, Black, White),
// and the name a color has: its own when the list has it, else its code ("#123456").
struct InkColor {
    QString name;
    QColor color;
};
QVector<InkColor> inkColors();
QString inkColorName(const QColor &color);

// The stroke as a path (a stroke of one point is a dot).
QPainterPath inkPath(const InkStroke &stroke);
// Whether the point is within `reach` of the stroke (its line, and half its weight).
bool inkTouches(const InkStroke &stroke, const QPointF &p, double reach);

// A freeform line for the stroke, thinned to the points that matter; null for a stroke with no point.
ItemPtr inkShape(const InkStroke &stroke);
// Puts the strokes on the slide as shapes above its other objects.
void addInk(Page &slide, const QVector<InkStroke> &strokes);

} // namespace jp::deck
