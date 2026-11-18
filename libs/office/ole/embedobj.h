#pragma once
// An object placed inside another app's document: a range or chart of the spreadsheet, or a page of the word processor. It is kept as the package that holds it (so the program that
// made it opens it again) with its saved picture, which is what the document shows. An object of any other kind, or one whose bytes are not a safe package, is kept byte for byte
// and shown by its picture too. The picture is drawn by a callback the app gives (one per kind), so this layer includes no app. A reader that opens the package itself (a PictureFn
// does) keeps its own limits: the checks here only keep a file from making the work unbounded before the callback is called.

#include <QByteArray>
#include <QImage>
#include <QString>

#include <functional>
#include <optional>

namespace jp::office::ole {

// The caps. limits() is what every check reads; setLimits is for tests (it shrinks the caps so a test need not build 64 MB), and resetLimits puts the real ones back.
struct Limits {
    qint64 objectBytes = 64LL * 1024 * 1024;         // one object's package
    qint64 totalBytes = 256LL * 1024 * 1024;         // all objects of a file, packages and pictures together
    int objects = 2000;
    qint64 pictureBytes = 16LL * 1024 * 1024;        // one saved picture
    int pictureSide = 8192;                          // pixels, either way
    qint64 expandedBytes = 1024LL * 1024 * 1024;     // what a package's entries say they hold once unpacked
    int packageEntries = 10000;
};
const Limits &limits();
void setLimits(const Limits &limits);
void resetLimits();

constexpr int kPictureScale = 2;
constexpr qint64 kEmuPerPoint = 12700;
constexpr qint64 kMaxEmu = 1000000000000LL;   // a size above this is not a size

enum class Kind { Sheet, Page, Other };

struct EmbeddedObject {
    QString progId;
    QByteArray partBytes;
    QByteArray previewPng;
    qint64 widthEmu = 0;
    qint64 heightEmu = 0;
    bool isOurs = false;   // made by one of our apps and a safe package: one we can open for editing
    QString origin;        // where a reader found it (the part's name), kept so a writer can put it back

    Kind kind() const { return isOurs ? kindOfProgIdImpl(progId) : Kind::Other; }
private:
    static Kind kindOfProgIdImpl(const QString &progId);
};

// The program ID of a kind of ours, the extension of its package, and the kind of a program ID, with or without its version (Other for any not ours).
QString progIdOf(Kind kind);
QString extensionOf(Kind kind);
Kind kindOfProgId(const QString &progId);

// Counts what a file holds as its objects are read; each admit() says whether the next one fits, and a refusal counts nothing.
class ObjectBudget {
public:
    bool admit(qint64 partBytes, qint64 pictureBytes = 0);
    int count() const { return m_count; }
    qint64 total() const { return m_total; }
private:
    int m_count = 0;
    qint64 m_total = 0;
};

// Draws the picture of an object: the package's bytes, the size in EMU, and the scale (pixels per point of the object's size, 2 for a picture twice as sharp as the screen).
using PictureFn = std::function<QImage(const QByteArray &partBytes, qint64 widthEmu, qint64 heightEmu, int scale)>;

// Whether a size can be drawn at kPictureScale within the picture's side cap; checked before any callback is called.
bool pictureSizeIsSane(qint64 widthEmu, qint64 heightEmu);
QByteArray pngOf(const QImage &image);   // empty if the image is null, too large a side, or over the picture's byte cap

// Whether bytes begin as a package (a zip) does, and whether its central directory is within the caps (entries, and the sum of the sizes the entries say they unpack to).
bool looksLikePackage(const QByteArray &bytes);
bool packageWithinLimits(const QByteArray &bytes);

// An object of ours made from its package and the callback's picture. Nothing if the package is not a safe package or is over the cap, the size is not one, the callback gives no
// picture, or the budget says no.
std::optional<EmbeddedObject> makeObject(Kind kind, const QByteArray &partBytes, qint64 widthEmu, qint64 heightEmu, const PictureFn &draw, ObjectBudget &budget, const QString &origin = QString());

// An object read from a file: kept as it is. It is ours only if its program ID is one of ours and its bytes are a safe package. A picture that is not a PNG, is too large a side, or is
// over the byte cap is left out (the object is kept without it). Nothing if the object is over the cap or the budget says no.
std::optional<EmbeddedObject> keepObject(const QString &progId, const QByteArray &partBytes, const QByteArray &previewPng, qint64 widthEmu, qint64 heightEmu, ObjectBudget &budget, const QString &origin = QString());

// What an edit's save gives back: new bytes and a picture for them, which is empty and flagged stale when it could not be drawn.
struct SavedObject {
    QByteArray partBytes;
    QByteArray previewPng;
    bool pictureStale = false;
};
// A save read: nothing if the kind is not ours or the bytes are not a safe package or over the cap. The picture is drawn at the size given.
std::optional<SavedObject> readSave(Kind kind, const QByteArray &partBytes, qint64 widthEmu, qint64 heightEmu, const PictureFn &draw);
// Puts a save into the window's current object: the bytes and, unless stale, the picture; everything else (size, position, origin) stays as the window has it now.
void applySave(EmbeddedObject &object, const SavedObject &saved);

} // namespace jp::office::ole
