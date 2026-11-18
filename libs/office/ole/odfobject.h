#pragma once
// Embedded objects as an OpenDocument file keeps them (a draw:object whose sub-document is not a formula or a chart, and a draw:object-ole whose part is binary), read into the object core's
// model (embedobj.h) and written from it. A sub-document is kept whole as a package of its own (its parts and their media types), so that saving writes every part and manifest entry as it was
// found; a binary object is kept byte for byte. Shared by the readers and writers of the text, presentation, and spreadsheet forms. A formula or a chart is not an object here: those keep their paths.

#include "core/export.h"
#include "ole/embedobj.h"

#include <QByteArray>
#include <QHash>
#include <QStringList>

#include <atomic>
#include <functional>

namespace jp::office {
class Package;
}

namespace jp::office::ole {

// The most files one object's sub-document may hold (its bytes are held to Limits::objectBytes).
constexpr int kMostFolderEntries = 512;

// The program IDs an object of this form has when it comes from a file of this form: a sub-document, and a binary part with its media type after a slash (an object of ours has its own ID).
JO_COMMON_EXPORT QString odfFolderProgId();
JO_COMMON_EXPORT QString odfBinaryProgId(const QString &mediaType);

// What tests count: the objects whose parts were inflated.
struct OdfObjectTestHooks {
    std::atomic<qint64> objectReads{0};
};
JO_COMMON_EXPORT OdfObjectTestHooks &odfObjectTestHooks();

// The package of a file, as the reader of any of the three forms gives it: part names (no slash in front), the bytes a part holds once inflated (-1 for none), the bytes of a part, its first
// bytes, and the media types the manifest gives ("Object 1/" for a folder).
struct OdfSource {
    std::function<QStringList()> names;
    std::function<qint64(const QString &)> size;
    std::function<QByteArray(const QString &)> bytes;
    std::function<QByteArray(const QString &)> head;
    QHash<QString, QString> types;
};
// The media types of a manifest (full path, "./" and a slash in front dropped, a folder's slash kept).
JO_COMMON_EXPORT QHash<QString, QString> odfManifestTypes(const QByteArray &manifestXml);
JO_COMMON_EXPORT OdfSource odfSourceOfPackage(const Package &package);

class JO_COMMON_EXPORT OdfObjectReader {
public:
    explicit OdfObjectReader(OdfSource source) : m_source(std::move(source)) {}
    // NotObject: not one to keep (a formula, a chart, a link, a name that leaves the file, a part that is missing); the frame is read as before.
    enum class Result { NotObject, Kept, PastLimit };
    // The name an href gives the object (empty for one that is no usable name).
    static QString pathOf(const QString &href);
    // Whether the object is one this reads, and how many bytes its parts come to (before any is inflated); -1 if it is not one.
    qint64 sizeOf(const QString &href, bool binary);
    // Reads the object the href names (a binary part when `binary`, else a folder). The count and the bytes are looked at before any part is inflated, a part named again is inflated once.
    Result read(const QString &href, bool binary, qint64 widthEmu, qint64 heightEmu, const QByteArray &preview, ObjectBudget &budget, EmbeddedObject *out);
    int pastLimit() const { return m_past; }

private:
    struct Plan {
        enum Kind { Not, Binary, Folder } kind = Not;
        QStringList files;
        qint64 total = 0;
        bool over = false;
        QString mediaType;
    };
    const Plan &planOf(const QString &path, bool binary);
    OdfSource m_source;
    QStringList m_names;
    bool m_named = false;
    QHash<QString, Plan> m_plans;
    QHash<QString, QPair<QString, QByteArray>> m_blobs;   // the program ID and bytes of each object read
    int m_past = 0;
};

// An object as the parts a file of this form holds, under `dir` ("Object 3"): a binary object is the one part `dir`; a sub-document is the folder `dir/` (no bytes) and its files. A path that ends
// in a slash is a folder, listed in the manifest only. Empty for an object that is refused (a folder that is not a package, or one that names a path out of itself, or is past the caps).
struct OdfOutPart {
    QString path;
    QByteArray bytes;
    QString mediaType;
    bool compress = false;
};
JO_COMMON_EXPORT QVector<OdfOutPart> odfPartsOf(const EmbeddedObject &object, const QString &dir);

// Whether a path is one an object's file may have inside its folder: no empty, "." or ".." part, no backslash, colon, or control character, not absolute.
JO_COMMON_EXPORT bool odfPathIsSafe(const QString &path);

} // namespace jp::office::ole
