#pragma once
// Pictures as files: the formats a drawing can be saved as (PNG, JPEG, GIF, TIFF, BMP), written from a QImage. Qt writes all of them but GIF, which is written here
// (at most 256 colors, as the format has).

#include <QByteArray>
#include <QImage>
#include <QString>

class QIODevice;

namespace jp {

enum class PictureFormat { Png, Jpeg, Gif, Tiff, Bmp };

// The file name's ending for a format, without the dot ("png", "jpg", "gif", "tif", "bmp").
QString pictureExtension(PictureFormat format);
// The format a file name's ending names (not case sensitive); false when it names none.
bool pictureFormatOfExtension(const QString &extension, PictureFormat *format);

// The picture as a GIF: its colors cut down to the 256 that stand for most of it (the picture is on white where it has any see-through), then LZW packed. Empty when the picture is
// empty or wider or taller than the format's 65,535.
QByteArray encodeGif(const QImage &image);

// Writes the picture in the format; `quality` is JPEG's (0 to 100). False when the picture is empty or the writer fails; `why` then says it.
bool writePicture(QIODevice *to, const QImage &image, PictureFormat format, int quality = 92, QString *why = nullptr);

} // namespace jp
