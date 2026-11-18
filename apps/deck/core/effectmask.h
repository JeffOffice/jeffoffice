#pragma once
// The shapes the common effects uncover a shape with: blinds, checkerboard, random bars, box, circle, diamond,
// plus, wheel, and dissolve. Each is drawn as a path over the unit square (the shape's own box is 0 to 1 across
// and down), so the show, the Animations gallery's pictures, and the tests all have one way to say what shows.

#include <QPainterPath>
#include <QString>

namespace jp::deck {

// The part of a shape an effect's filter ("blinds(horizontal)", "checkerboard(across)", "randombar(vertical)",
// "box(in)", "circle(out)", "diamond(in)", "plus(out)", "wheel(4)", "dissolve") shows `p` of the way (0 to 1)
// through, as a path over the unit square: an entrance shows what the effect has uncovered, an exit what it
// has not reached yet. False for a filter it has no picture for (a fade, a wipe, and every other name).
bool effectMask(const QString &filter, double p, bool entering, QPainterPath *shown);

} // namespace jp::deck
