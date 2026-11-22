#pragma once
// Whole-pixel drawing of axis-aligned solid lines and rectangles into pixels (a screen or a picture of a page, not a PDF or paper), as the page's own
// renderers draw them: each edge goes to the nearest device pixel line, and a rule is never under a pixel wide, not a smear over the two pixels it
// straddles. Device pixels at any device pixel ratio and zoom: the logical coordinates go through the painter's own device transform. Every function
// returns false (nothing drawn) or no box where the painter is turned or slanted, so the caller draws as it did.

#include <QBrush>
#include <QColor>
#include <QPainter>
#include <QPointF>
#include <QRect>
#include <QRectF>
#include <optional>

namespace jp {

// Whether the painter's device transform only scales and moves (no turn, slant, or mirror).
bool paintsAxisAligned(const QPainter *p);

// The device pixels (a box of whole pixels) of the rule from a to b, `width` thick (logical units): the thickness comes from the width alone (never under
// one pixel), and where it falls is snapped on its own, by the edge on the outside; `fromFar` keeps the far edge (a rule drawn up from an edge) instead.
// None unless a and b are on one axis.
std::optional<QRect> wholePixelRuleBox(const QPainter *p, QPointF a, QPointF b, double width, bool fromFar = false);
// The device pixels of a rectangle: each edge to the nearest pixel line, at least one pixel across.
std::optional<QRect> wholePixelRectBox(const QPainter *p, const QRectF &r);

// Fills a box of device pixels (without smoothing) with a brush.
void fillDeviceBox(QPainter *p, const QRect &box, const QBrush &brush);
// The rule, a rectangle, or a rectangle's outline (`width` thick, centered on its edges, the four bars one pixel run each, corners square) in whole pixels.
bool fillWholePixelRule(QPainter *p, QPointF a, QPointF b, double width, const QBrush &brush, bool fromFar = false);
bool fillWholePixelRect(QPainter *p, const QRectF &r, const QBrush &brush);
bool strokeWholePixelRect(QPainter *p, const QRectF &r, double width, const QColor &color);

// A rectangle in logical coordinates whose device pixels are those of the whole-pixel rectangle (for a picture's place); r where the painter is turned.
QRectF wholePixelRectF(const QPainter *p, const QRectF &r);

} // namespace jp
