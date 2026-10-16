#pragma once
// The QuickDraw pictures of a Macintosh (a PICT, as a picture record of a
// binary drawing holds one), replayed into an image.

#include <QByteArray>
#include <QImage>

namespace jp::office {

// A version 2 picture's operations drawn as an image (its longest side at
// most 2,048 pixels, about twice its own units where they are points): the
// lines, rectangles, ovals, arcs, polygons, regions, text, and bitmaps that
// drawing programs and chart makers write. `pict` starts at the picture's size
// word (a file's 512-byte header is not part of it). A null image when it is
// not a picture this can draw; drawing stops where an operation it can't read
// is, or where the picture has done as much work as one may (about a second's).
// `work` is what is left of the work all the pictures of one file may do: this
// takes what it does from it, and draws nothing when none is left. (The work of
// a picture of a real presentation is some tens of thousands of the million units
// one picture may do.)
QImage odrawPictImage(const QByteArray &pict, qint64 *work = nullptr);


} // namespace jp::office
