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
// The last 128 are kept, by filter, direction, and progress in 256ths (the path is the one for the nearest 256th).
bool effectMask(const QString &filter, double p, bool entering, QPainterPath *shown);

// How the cache has done (for the tests): masks asked for that were kept, masks built, and a way to start over.
int effectMaskCacheHits();
int effectMaskBuilds();
int effectMaskCacheSize();
void clearEffectMaskCache();

} // namespace jp::deck
