#pragma once
// An object placed inside another app's document: a range or chart of the spreadsheet, or a page of the word processor. It is kept as the package that holds it (so the program that
// made it opens it again) with its saved picture, which is what the document shows. An object of any other kind is kept byte for byte and shown by its picture too.
// The picture is drawn by a callback the app gives, so this layer includes no app.

#include <QByteArray>
#include <QImage>
#include <QString>

#include <functional>
#include <optional>

namespace jp::office::ole {

// What the caps are: every reader of a file with objects asks an ObjectBudget, so a hostile file cannot multiply the work.
constexpr qint64 kMaxObjectBytes = 64LL * 1024 * 1024;
constexpr qint64 kMaxTotalBytes = 256LL * 1024 * 1024;
constexpr int kMaxObjects = 2000;
constexpr int kMaxPictureSide = 8192;   // pixels; a picture larger than this on either side is not kept
constexpr int kPictureScale = 2;

enum class Kind { Sheet, Page, Other };

struct EmbeddedObject {
    QString progId;
    QByteArray partBytes;
    QByteArray previewPng;
    qint64 widthEmu = 0;
    qint64 heightEmu = 0;
    bool isOurs = false;   // made by one of our apps: a package we can open for editing
    QString origin;        // where a reader found it (the part's name), kept so a writer can put it back

    Kind kind() const;
};

// The program ID of a kind of ours, the extension of its package, and the kind of a program ID (Other for any not ours).
QString progIdOf(Kind kind);
QString extensionOf(Kind kind);
Kind kindOfProgId(const QString &progId);

// Counts what a file holds as its objects are read; each admit() says whether the next one fits.
class ObjectBudget {
public:
    bool admit(qint64 bytes);
    int count() const { return m_count; }
    qint64 total() const { return m_total; }
private:
    int m_count = 0;
    qint64 m_total = 0;
};

// Draws the picture of an object: the package's bytes, the size in EMU, and the scale (pixels per point of the object's size, 2 for a picture twice as sharp as the screen).
using PictureFn = std::function<QImage(Kind kind, const QByteArray &partBytes, qint64 widthEmu, qint64 heightEmu, int scale)>;

QByteArray pngOf(const QImage &image);   // empty if the image is null or larger than kMaxPictureSide

// An object of ours made from its package and the callback's picture; nothing if the package is empty or over the cap, or the callback gives no picture.
std::optional<EmbeddedObject> makeObject(Kind kind, const QByteArray &partBytes, qint64 widthEmu, qint64 heightEmu, const PictureFn &draw, const QString &origin = QString());

// An object read from a file: kept as it is, ours or not. Nothing if it is over the per-object cap.
std::optional<EmbeddedObject> keepObject(const QString &progId, const QByteArray &partBytes, const QByteArray &previewPng, qint64 widthEmu, qint64 heightEmu, const QString &origin = QString());

// The object with new bytes (after an edit) and a picture drawn again; without a picture from the callback it keeps the old one. Nothing if the object is not ours or the bytes are not a package or over the cap.
std::optional<EmbeddedObject> withNewBytes(const EmbeddedObject &object, const QByteArray &partBytes, const PictureFn &draw);

// Whether bytes begin as a package (a zip) does.
bool looksLikePackage(const QByteArray &bytes);

} // namespace jp::office::ole
