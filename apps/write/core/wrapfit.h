#pragma once
// Where a Tight or Through wrap starts: the outline of what the object shows (a shape's path, the shape a picture is cut to, or the opaque part of a picture), as shares of its
// frame, instead of the frame's four corners. A file's own polygon is never replaced by it; that is for the caller to check. A wrap polygon is in the object's frame before it is turned
// (the paginator and the page layer both place it in the unrotated frame), so a turn is not applied here.

#include <QByteArray>
#include <QImage>
#include <QPainterPath>
#include <QPolygonF>
#include <QSizeF>
#include <QString>

namespace jp::write {

// How a picture is shown in its frame: the shares cut from each edge (a:srcRect), and whether it is flipped across or down.
struct WrapSeedView {
    double cutLeft = 0, cutTop = 0, cutRight = 0, cutBottom = 0;
    bool flipH = false, flipV = false;
};

// The outline for an object of `frame` points: the outline of `shape` (a path in the frame's points) when it has one, else of the preset `cropShape` ("" and "rect" for none) drawn in the
// frame, else of the transparency of `image` (cut and flipped as `view` says). Empty where there is nothing to follow (the frame stays).
QPolygonF seededWrapOutline(const QPainterPath &shape, const QString &cropShape, const QImage &image, const QSizeF &frame, const WrapSeedView &view = WrapSeedView());

// A picture file read for its transparency: a PNG, GIF, or TIFF of at most kMostSeedPixels as its header declares (the readers decode a whole picture before they scale it, so a larger one
// is not read, and keeps its frame), whole. Nothing for another format, a file that is not a picture, or one without alpha.
constexpr qint64 kMostSeedPixels = 4000000;
QImage wrapSeedImage(const QByteArray &bytes);
// How many pictures wrapSeedImage has decoded (for the tests).
quint64 wrapSeedDecodes();

} // namespace jp::write
