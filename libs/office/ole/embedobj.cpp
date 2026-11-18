#include "ole/embedobj.h"

#include <QBuffer>

namespace jp::office::ole {

QString progIdOf(Kind kind)
{
    switch (kind) {
#define JO_OLE_KIND(k, id, ext) case Kind::k: return QStringLiteral(id);
#include "ole/progids.inc"
#undef JO_OLE_KIND
    case Kind::Other: break;
    }
    return QString();
}

QString extensionOf(Kind kind)
{
    switch (kind) {
#define JO_OLE_KIND(k, id, ext) case Kind::k: return QStringLiteral(ext);
#include "ole/progids.inc"
#undef JO_OLE_KIND
    case Kind::Other: break;
    }
    return QString();
}

Kind kindOfProgId(const QString &progId)
{
#define JO_OLE_KIND(k, id, ext) if (progId.compare(QLatin1String(id), Qt::CaseInsensitive) == 0) return Kind::k;
#include "ole/progids.inc"
#undef JO_OLE_KIND
    return Kind::Other;
}

Kind EmbeddedObject::kind() const { return kindOfProgId(progId); }

bool ObjectBudget::admit(qint64 bytes)
{
    if (bytes < 0 || bytes > kMaxObjectBytes) return false;
    if (m_count >= kMaxObjects || m_total + bytes > kMaxTotalBytes) return false;
    ++m_count;
    m_total += bytes;
    return true;
}

bool looksLikePackage(const QByteArray &bytes) { return bytes.size() >= 4 && bytes.startsWith("PK\x03\x04"); }

QByteArray pngOf(const QImage &image)
{
    if (image.isNull() || image.width() > kMaxPictureSide || image.height() > kMaxPictureSide) return {};
    QByteArray out;
    QBuffer buf(&out);
    buf.open(QIODevice::WriteOnly);
    if (!image.save(&buf, "PNG")) return {};
    return out;
}

std::optional<EmbeddedObject> makeObject(Kind kind, const QByteArray &partBytes, qint64 widthEmu, qint64 heightEmu, const PictureFn &draw, const QString &origin)
{
    if (kind == Kind::Other || partBytes.isEmpty() || partBytes.size() > kMaxObjectBytes || !draw) return std::nullopt;
    QByteArray png = pngOf(draw(kind, partBytes, widthEmu, heightEmu, kPictureScale));
    if (png.isEmpty()) return std::nullopt;
    EmbeddedObject o;
    o.progId = progIdOf(kind);
    o.partBytes = partBytes;
    o.previewPng = png;
    o.widthEmu = widthEmu;
    o.heightEmu = heightEmu;
    o.isOurs = true;
    o.origin = origin;
    return o;
}

std::optional<EmbeddedObject> keepObject(const QString &progId, const QByteArray &partBytes, const QByteArray &previewPng, qint64 widthEmu, qint64 heightEmu, const QString &origin)
{
    if (partBytes.size() > kMaxObjectBytes) return std::nullopt;
    EmbeddedObject o;
    o.progId = progId;
    o.partBytes = partBytes;
    o.previewPng = previewPng;
    o.widthEmu = widthEmu;
    o.heightEmu = heightEmu;
    o.isOurs = kindOfProgId(progId) != Kind::Other;
    o.origin = origin;
    return o;
}

std::optional<EmbeddedObject> withNewBytes(const EmbeddedObject &object, const QByteArray &partBytes, const PictureFn &draw)
{
    Kind kind = object.kind();
    if (!object.isOurs || kind == Kind::Other || !looksLikePackage(partBytes) || partBytes.size() > kMaxObjectBytes) return std::nullopt;
    EmbeddedObject o = object;
    o.partBytes = partBytes;
    if (draw) {
        QByteArray png = pngOf(draw(kind, partBytes, o.widthEmu, o.heightEmu, kPictureScale));
        if (!png.isEmpty()) o.previewPng = png;
    }
    return o;
}

} // namespace jp::office::ole
