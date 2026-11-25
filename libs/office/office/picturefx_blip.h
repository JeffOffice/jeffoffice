#pragma once
// The picture filters (core/picturefx.h) as a file keeps them, for every program that writes DrawingML: the blip is the finished picture, and an extension of ours beside it holds the settings and
// names the original the picture was made from (so that the effect can be changed or taken off again). The presentation and publication programs write it from their pictures (drawingml_write.cpp)
// and read it back (drawingml_shape.cpp); the word processor and the spreadsheet keep a picture as the text of its drawing, and change that text with the functions below.

#include "core/blipeffect.h"

#include <QByteArray>
#include <QImage>
#include <QString>
#include <atomic>
#include <QVector>

namespace jp::office {

// The most bytes of the extension's settings that are read (a longer one is left alone).
constexpr qint64 kMostPictureFxDataBytes = 1024 * 1024;

// The settings as the extension holds them (the filters of `effects`, as compact JSON in base64), and back (only the filters; empty for text that is not settings or is too long).
QString pictureFxData(const QVector<BlipEffect> &effects);
QVector<BlipEffect> pictureFxFromData(const QString &data);

// What the first a:blip of `xml` says: the picture it shows, the original its filters were made from (the same when it has none), and the filters.
struct PictureFxBlip {
    QString shownRid, originalRid;
    QVector<BlipEffect> filters;
    bool present = false;   // the blip has the extension, with filters that change something
};
bool readPictureFxBlip(const QByteArray &xml, PictureFxBlip *out);
// `xml` with the first blip showing the picture `shownRid` and, when `filters` change something, the extension (the settings and `originalRid`); otherwise the extension is taken off. What else the
// blip holds (other extensions, the color effects) stays as it was. Unchanged when xml holds no blip.
QByteArray withPictureFxBlip(const QByteArray &xml, const QString &shownRid, const QString &originalRid, const QVector<BlipEffect> &filters);
// The extension's own identifier, for a writer that keeps only the finished picture and has to say so (the OpenDocument and Rich Text writers' Check Compatibility lines).
const char *pictureFxExtensionUri();
// What names a picture for the cache of finished pictures (blipAdjusted): its bytes and size, as the renderer names it (blipSourceKey), so that the picture an edit made is found by the next draw.
QString pictureFxKeyFor(const QByteArray &originalBytes, const QSize &pixels);
// The finished picture of `original` with `filters` (a PNG; empty when it can't be made). `sourceKey` names the picture for the cache, so that a second save of the same picture does not work it out again.
QByteArray pictureFxFinishedPng(const QImage &original, const QVector<BlipEffect> &filters, const QString &sourceKey, const std::atomic<bool> *stop = nullptr);

} // namespace jp::office
