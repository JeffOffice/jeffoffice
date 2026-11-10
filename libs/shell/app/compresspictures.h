#pragma once
// Compress Pictures, the same in the apps that have it: the box that asks for the target (commercial printing, desktop printing, or web), whether it is for every picture or the
// selected ones, and whether the cut-off parts of pictures go; and the two steps every app takes on each picture (made no wider than the target asks, then kept only if it is smaller).
// What a picture is, and how the smaller one takes its place, is the app's.

#include <QByteArray>
#include <QImage>
#include <QString>

class QWidget;

namespace jp {

struct CompressChoices {
    double ppi = 300;           // pixels to the inch of the target
    bool all = true;            // every picture (else the selected ones)
    bool deleteCropped = true;  // the cut-off parts of pictures go
};

// Asks; false when cancelled. `allText` is the words of the "every picture" box, which name what the pictures are in (a publication, a presentation, and so on).
bool askCompressPictures(QWidget *parent, const QString &allText, CompressChoices *choices);

// The image made no wider than `ppi` asks of a picture shown `shownWidth` points across (an image only a little wider is left as it is).
QImage scaledForPpi(const QImage &image, double shownWidth, double ppi);
// The image as a file (JPEG, or PNG when it has transparency) in `bytes` with its format ("jpg" or "png"); true only when that is fewer bytes than `originalBytes`.
bool encodedIfSmaller(const QImage &image, qint64 originalBytes, QByteArray *bytes, QByteArray *format);

} // namespace jp
