#pragma once
// Where a Tight or Through wrap starts: the outline of what the object shows (a shape's path, the shape a picture is cut to, or the opaque part of a picture), as shares of its
// frame, instead of the frame's four corners. A file's own polygon is never replaced by it; that is for the caller to check.

#include <QByteArray>
#include <QImage>
#include <QPainterPath>
#include <QPolygonF>
#include <QSizeF>
#include <QString>

namespace jp::write {

// The outline for an object of `frame` points: the outline of `shape` (a path in the frame's points) when it has one, else of the preset `cropShape` ("" and "rect" for none) drawn in the
// frame, else of the transparency of `image`. Empty where there is nothing to follow (the frame stays). `budget`, when given, is how many more outlines may be made (a document's pictures
// seeded at once come to no more than kMaxWrapShapesPerPage): each outline takes one, and none is made when it is 0.
QPolygonF seededWrapOutline(const QPainterPath &shape, const QString &cropShape, const QImage &image, const QSizeF &frame, int *budget = nullptr);

// A picture file read for its transparency: at most 256 pixels on the longer side, and nothing for a file that is not a picture, has no alpha, or is larger than kMostSeedPixels.
constexpr qint64 kMostSeedPixels = 40000000;
QImage wrapSeedImage(const QByteArray &bytes);

} // namespace jp::write
