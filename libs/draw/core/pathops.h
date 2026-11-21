#pragma once
// Merge Shapes for every app: the outlines of several shapes made into one (union, combine, subtract, intersect) or cut into the pieces they make (fragment), a path
// taken apart into its subpaths, and an outline turned into straight lines or into the outline of its stroke. All in one coordinate system (the caller maps each shape to the page).

#include <QPainterPath>
#include <QVector>

namespace jp::pathops {

// What the editing is held to, so a file's shape or a long selection cannot make the work grow without end.
constexpr int kMostShapes = 64;          // outlines merged in one command
constexpr int kMostElements = 6000;      // elements in one outline that is merged (pts::kMostElements)
constexpr int kMostTotalElements = 24000;   // elements in all the outlines of one command together
constexpr int kMostPieces = 256;         // the pieces a fragment or a break apart makes (and the subpaths read for it)
constexpr int kMostResultElements = 60000;  // elements in a result: a larger one is refused

enum class Op {
    Union,       // one outline with everything the shapes cover
    Combine,     // what only one of the shapes covers: the overlaps are cut out
    Subtract,    // the first shape without the others
    Intersect,   // what all the shapes cover
    Fragment     // each piece the shapes cut one another into
};

// The result outlines (one for each piece; one for all but a fragment). Empty, with `*refused` told why when it is not null, when the input is past the limits or the result is
// empty or past the limits: 0 nothing is left, 1 too many shapes or elements, 2 too many pieces. Outlines with fewer than two are refused as 1.
QVector<QPainterPath> merge(Op op, const QVector<QPainterPath> &outlines, int *refused = nullptr);

// The subpaths of a path in the order they come, each with the curves it had; an empty one is not kept. At most kMostPieces + 1 (so that a caller can tell there are too many); no more are read.
QVector<QPainterPath> subpaths(const QPainterPath &path);
// The path cut into islands: each outer outline with the holes inside it. Empty when there are more than kMostPieces subpaths.
QVector<QPainterPath> islands(const QPainterPath &path);
// The same outline with every curve made of straight lines no further than `tolerance` points from it.
QPainterPath polygon(const QPainterPath &path, double tolerance = 0.25);
// The outline of the path's stroke of this width (points), as an area to fill.
QPainterPath contour(const QPainterPath &path, double width);

} // namespace jp::pathops
