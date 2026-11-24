#pragma once
// Taking the background off a picture. The picture itself is never changed: what is kept is a mask beside it (a byte for each pixel of a
// copy of at most kRemoveBgWorkPixels pixels: 0 where the background was taken off, 255 where it stays) and the strokes the person drew to
// correct it, so the removal can be opened again, edited, or undone. The mask is found from the colors that touch the picture's edges: what is
// like them, and joined to the edges through pixels that are like them, is background.

#include <QByteArray>
#include <QColor>
#include <QImage>
#include <QJsonObject>
#include <QPointF>
#include <QSize>
#include <QVector>
#include <functional>

namespace jp {

// The pixels the colors are worked out on (a larger picture is shrunk to this for the work, and the mask is made larger again).
constexpr qint64 kRemoveBgWorkPixels = 1'000'000;
// The largest picture the work is done for (the same as a picture may have to be opened), and the most strokes and points a removal keeps.
constexpr qint64 kRemoveBgMaxSourcePixels = 100LL * 1000 * 1000;
constexpr int kRemoveBgMaxMarks = 256;
constexpr int kRemoveBgMaxMarkPoints = 8192;
// The passes the color groups are fitted in, and the most groups of one kind.
constexpr int kRemoveBgFitPasses = 6;
constexpr int kRemoveBgMaxGroups = 8;

// A stroke dragged over the picture: where the pixels under it are to stay (keep) or go. The points are shares of the picture's width and
// height (0 to 1), and the radius a share of its longer side.
struct BackgroundMark {
    bool keep = true;
    double radius = 0.02;
    QVector<QPointF> points;
    bool operator==(const BackgroundMark &o) const { return keep == o.keep && radius == o.radius && points == o.points; }
};

struct BackgroundRemoval {
    QSize source;               // the picture's size in pixels when the mask was made (a mask is used only for a picture of that size)
    QSize size;                 // the mask's size
    QByteArray mask;            // size.width() * size.height() bytes, row by row
    QVector<BackgroundMark> marks;

    bool isValid() const;       // the mask has the size it says, and the sizes are inside what is allowed
    bool operator==(const BackgroundRemoval &o) const { return source == o.source && size == o.size && mask == o.mask && marks == o.marks; }
    // Different for removals that differ in what they draw (a key for a picture kept changed).
    QString key() const;
    QJsonObject toJson() const;
    static BackgroundRemoval fromJson(const QJsonObject &o);   // an invalid one (nothing) when the object is not a mask this program could have made
};

// The removal for `picture` with the strokes `marks` taken into account. `progress` is told how far the work is (0 to 100) now and then, and
// answers false to stop it. Invalid when the picture is empty, has more than kRemoveBgMaxSourcePixels pixels, or the work was stopped.
BackgroundRemoval findBackground(const QImage &picture, const QVector<BackgroundMark> &marks, const std::function<bool(int)> &progress = {});

// `picture` with the removed part made clear (more or less, at the soft edge) as an image of 32 bits. A removal that is not for a picture of
// this size leaves the picture as it is. With `tint` set, the removed part is shown in that color over the picture instead (for the person
// to see what is going).
QImage applyBackgroundRemoval(const QImage &picture, const BackgroundRemoval &removal, const QColor &tint = QColor());

} // namespace jp
