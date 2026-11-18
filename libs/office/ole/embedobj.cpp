#include "ole/embedobj.h"

#include <QBuffer>
#include <QImageReader>

namespace jp::office::ole {

namespace {
Limits g_limits;

quint32 rd32(const uchar *p) { return quint32(p[0]) | quint32(p[1]) << 8 | quint32(p[2]) << 16 | quint32(p[3]) << 24; }
quint32 rd16(const uchar *p) { return quint32(p[0]) | quint32(p[1]) << 8; }
} // namespace

const Limits &limits() { return g_limits; }
void setLimits(const Limits &l) { g_limits = l; }
void resetLimits() { g_limits = Limits(); }

QString progIdOf(Kind kind)
{
    switch (kind) {
#define JO_OLE_ALIAS(k, id)
#define JO_OLE_KIND(k, id, ext) case Kind::k: return QStringLiteral(id);
#include "ole/progids.inc"
#undef JO_OLE_KIND
#undef JO_OLE_ALIAS
    case Kind::Other: break;
    }
    return QString();
}

QString extensionOf(Kind kind)
{
    switch (kind) {
#define JO_OLE_ALIAS(k, id)
#define JO_OLE_KIND(k, id, ext) case Kind::k: return QStringLiteral(ext);
#include "ole/progids.inc"
#undef JO_OLE_KIND
#undef JO_OLE_ALIAS
    case Kind::Other: break;
    }
    return QString();
}

Kind kindOfProgId(const QString &progId)
{
#define JO_OLE_ALIAS(k, id) if (progId.compare(QLatin1String(id), Qt::CaseInsensitive) == 0) return Kind::k;
#define JO_OLE_KIND(k, id, ext) if (progId.compare(QLatin1String(id), Qt::CaseInsensitive) == 0) return Kind::k;
#include "ole/progids.inc"
#undef JO_OLE_KIND
#undef JO_OLE_ALIAS
    return Kind::Other;
}

Kind EmbeddedObject::kindOfProgIdImpl(const QString &progId) { return kindOfProgId(progId); }

bool ObjectBudget::fits(qint64 partBytes, qint64 pictureBytes) const
{
    const Limits &l = limits();
    if (partBytes < 0 || pictureBytes < 0 || partBytes > l.objectBytes || pictureBytes > l.pictureBytes) return false;
    return m_count < l.objects && m_total + partBytes + pictureBytes <= l.totalBytes;
}

bool ObjectBudget::admit(qint64 partBytes, qint64 pictureBytes)
{
    if (!fits(partBytes, pictureBytes)) return false;
    ++m_count;
    m_total += partBytes + pictureBytes;
    return true;
}

bool looksLikePackage(const QByteArray &bytes) { return bytes.size() >= 4 && bytes.startsWith("PK\x03\x04"); }

bool packageWithinLimits(const QByteArray &bytes)
{
    if (!looksLikePackage(bytes)) return false;
    const Limits &l = limits();
    const uchar *p = reinterpret_cast<const uchar *>(bytes.constData());
    const qint64 n = bytes.size();
    qint64 eocd = -1;
    for (qint64 i = n - 22, lo = qMax<qint64>(0, n - 22 - 65535); i >= lo; --i)
        if (rd32(p + i) == 0x06054b50u) { eocd = i; break; }
    if (eocd < 0) return false;
    const quint32 count = rd16(p + eocd + 10), cdSize = rd32(p + eocd + 12), cdOff = rd32(p + eocd + 16);
    if (count == 0xFFFF || cdSize == 0xFFFFFFFFu || cdOff == 0xFFFFFFFFu) return false;   // the 64-bit form: beyond the caps
    if (qint64(count) > l.packageEntries) return false;
    if (qint64(cdOff) + cdSize > eocd) return false;
    qint64 pos = cdOff, total = 0;
    const qint64 end = qint64(cdOff) + cdSize;
    for (quint32 k = 0; k < count; ++k) {
        if (pos + 46 > end || rd32(p + pos) != 0x02014b50u) return false;
        quint32 u = rd32(p + pos + 24);
        total += u == 0xFFFFFFFFu ? 0x100000000LL : qint64(u);
        if (total > l.expandedBytes) return false;
        pos += 46 + rd16(p + pos + 28) + rd16(p + pos + 30) + rd16(p + pos + 32);
    }
    return pos <= end;
}

bool pictureSizeIsSane(qint64 widthEmu, qint64 heightEmu)
{
    if (widthEmu <= 0 || heightEmu <= 0 || widthEmu > kMaxEmu || heightEmu > kMaxEmu) return false;
    const qint64 side = limits().pictureSide;
    return (widthEmu * kPictureScale + kEmuPerPoint - 1) / kEmuPerPoint <= side && (heightEmu * kPictureScale + kEmuPerPoint - 1) / kEmuPerPoint <= side;
}

QByteArray pngOf(const QImage &image)
{
    if (image.isNull() || image.width() > limits().pictureSide || image.height() > limits().pictureSide) return {};
    QByteArray out;
    QBuffer buf(&out);
    buf.open(QIODevice::WriteOnly);
    if (!image.save(&buf, "PNG") || out.size() > limits().pictureBytes) return {};
    return out;
}

namespace {
// A saved picture read from a file is kept only if it is a PNG of a sane size and byte count; the header is read, the image is not decoded.
bool pictureIsAcceptable(const QByteArray &png)
{
    if (png.isEmpty() || png.size() > limits().pictureBytes) return false;
    QBuffer buf;
    buf.setData(png);
    buf.open(QIODevice::ReadOnly);
    QImageReader r(&buf);
    if (r.format() != "png") return false;
    QSize s = r.size();
    return s.isValid() && s.width() <= limits().pictureSide && s.height() <= limits().pictureSide;
}
} // namespace

std::optional<EmbeddedObject> makeObject(Kind kind, const QByteArray &partBytes, qint64 widthEmu, qint64 heightEmu, const PictureFn &draw, ObjectBudget &budget, const QString &origin)
{
    if (kind == Kind::Other || !draw || partBytes.size() > limits().objectBytes || !packageWithinLimits(partBytes) || !pictureSizeIsSane(widthEmu, heightEmu)) return std::nullopt;
    if (!budget.fits(partBytes.size())) return std::nullopt;   // ask before drawing: a refused object is not drawn
    QByteArray png = pngOf(draw(partBytes, widthEmu, heightEmu, kPictureScale));
    if (png.isEmpty() || !budget.admit(partBytes.size(), png.size())) return std::nullopt;
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

std::optional<EmbeddedObject> keepObject(const QString &progId, const QByteArray &partBytes, const QByteArray &previewPng, qint64 widthEmu, qint64 heightEmu, ObjectBudget &budget, const QString &origin)
{
    if (partBytes.size() > limits().objectBytes) return std::nullopt;
    QByteArray png = pictureIsAcceptable(previewPng) ? previewPng : QByteArray();
    if (!budget.admit(partBytes.size(), png.size())) return std::nullopt;
    EmbeddedObject o;
    o.progId = progId;
    o.partBytes = partBytes;
    o.previewPng = png;
    o.widthEmu = widthEmu;
    o.heightEmu = heightEmu;
    o.isOurs = kindOfProgId(progId) != Kind::Other && packageWithinLimits(partBytes);
    o.origin = origin;
    return o;
}

std::optional<SavedObject> readSave(Kind kind, const QByteArray &partBytes, qint64 widthEmu, qint64 heightEmu, const PictureFn &draw)
{
    if (kind == Kind::Other || partBytes.size() > limits().objectBytes || !packageWithinLimits(partBytes)) return std::nullopt;
    SavedObject s;
    s.partBytes = partBytes;
    if (draw && pictureSizeIsSane(widthEmu, heightEmu)) s.previewPng = pngOf(draw(partBytes, widthEmu, heightEmu, kPictureScale));
    s.pictureStale = s.previewPng.isEmpty();
    return s;
}

void applySave(EmbeddedObject &object, const SavedObject &saved)
{
    object.partBytes = saved.partBytes;
    if (!saved.pictureStale) object.previewPng = saved.previewPng;
}

} // namespace jp::office::ole
