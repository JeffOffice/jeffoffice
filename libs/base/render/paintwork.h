#pragma once
// The work of painting a picture that is drawn from a file's records (a
// metafile, a Macintosh picture): what a record costs, so a file that makes
// the painter do more than any picture does is stopped where it stands.
// Work is counted in units of about a microsecond on the computers it was
// measured on.

#include <QPainter>
#include <QPainterPath>
#include <QRectF>

namespace jp {

// What painting a rectangle (in the painter's own coordinates) costs: a few
// units, and for a painter that draws pixels, the area it covers in them (about
// 2,000 to a unit). A painter that writes a file (a PDF, a printer) pays no more
// for a big one than for a small one.
double areaWork(const QPainter *p, const QRectF &box);

// What filling a path costs a painter that draws pixels, from how many of its
// edges cross each row: the more there are, the more the rasterizer has to sort
// in each (a polygon of 16,000 points that crosses itself takes 15 seconds,
// one of as many that doesn't, nothing). Nothing for one that writes a file.
double fillWork(const QPainter *p, const QPainterPath &path);

} // namespace jp
