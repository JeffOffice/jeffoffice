// The pictures of a binary drawing (odraw.h): the records of the picture
// store, and each picture record turned into the file it holds.

#include "office/odraw.h"

#include "office/odraw_inflate.h"
#include "office/odraw_read.h"

#include <QtMath>
#include <algorithm>

namespace jp::office {

namespace {

using namespace odraw;

constexpr qint64 kMaxMetafile = 256 * 1024 * 1024;
constexpr int kMaxStoreEntries = 100000;

// A device-independent bitmap with a file header in front, which is what bitmap
// readers expect; empty when its header is not one of the known forms.
QByteArray bitmapFileOf(const QByteArray &dib)
{
    const quint32 headerSize = u32(dib, 0);
    qint64 colors = 0, entrySize = 4, masks = 0;
    if (headerSize == 12) {
        const int bits = u16(dib, 10);
        if (bits <= 8) colors = qint64(1) << bits;
        entrySize = 3;
    } else if (headerSize >= 40 && headerSize <= 124 && dib.size() >= 40) {
        const int bits = u16(dib, 14);
        const quint32 compression = u32(dib, 16), used = u32(dib, 32);
        colors = used ? used : bits <= 8 ? qint64(1) << bits : 0;
        if (bits <= 8) colors = std::min<qint64>(colors, qint64(1) << bits);
        colors = std::min<qint64>(colors, 65536);
        if (headerSize == 40) masks = compression == 3 ? 12 : compression == 6 ? 16 : 0;
    } else {
        return QByteArray();
    }
    const qint64 offset = qint64(headerSize) + masks + colors * entrySize;
    if (offset > dib.size()) return QByteArray();
    QByteArray file("BM", 2);
    QByteArray fields(12, '\0');
    qToLittleEndian<quint32>(quint32(14 + dib.size()), fields.data());
    qToLittleEndian<quint32>(quint32(14 + offset), fields.data() + 8);
    return file + fields + dib;
}

// The 22-byte header that gives a plain metafile its size, from the bounds
// (in the metafile's own units) and the size in EMU the picture record keeps.
QByteArray placeableHeader(qint32 left, qint32 top, qint32 right, qint32 bottom, qint32 widthEmu, qint32 heightEmu)
{
    qint64 w = qint64(right) - left, h = qint64(bottom) - top;
    qint64 l = left, t = top;
    qint64 inch = 1440;
    if (w > 0 && h > 0) {
        if (widthEmu > 0) inch = qint64(qRound64(double(w) * 914400.0 / widthEmu));
    } else {
        // No bounds: hundredths of a millimeter (360 EMU), 2540 to the inch.
        l = t = 0;
        w = widthEmu > 0 ? widthEmu / 360 : 1000;
        h = heightEmu > 0 ? heightEmu / 360 : 1000;
        inch = 2540;
    }
    // The fields hold 16 bits: shrink what is too big for them, all together.
    qint64 big = std::max({qAbs(l), qAbs(t), qAbs(l + w), qAbs(t + h)});
    const qint64 k = big > 32767 ? (big + 32766) / 32767 : 1;
    l /= k; t /= k; w /= k; h /= k;
    inch = std::clamp<qint64>(inch / k, 1, 65535);
    QByteArray head(22, '\0');
    uchar *p = reinterpret_cast<uchar *>(head.data());
    qToLittleEndian<quint32>(0x9AC6CDD7u, p);
    qToLittleEndian<qint16>(qint16(l), p + 6);
    qToLittleEndian<qint16>(qint16(t), p + 8);
    qToLittleEndian<qint16>(qint16(l + w), p + 10);
    qToLittleEndian<qint16>(qint16(t + h), p + 12);
    qToLittleEndian<quint16>(quint16(inch), p + 14);
    quint16 sum = 0;
    for (int i = 0; i < 10; ++i) sum ^= qFromLittleEndian<quint16>(p + i * 2);
    qToLittleEndian<quint16>(sum, p + 20);
    return head;
}

} // namespace

QByteArray odrawBlip(const QByteArray &data, QString *format)
{
    if (format) format->clear();
    const QVector<OdrawRecord> records = odrawRecords(data);
    if (records.isEmpty()) return QByteArray();
    const OdrawRecord r = records.first();
    const qint64 end = qint64(r.offset) + r.length;
    // The uid of the picture, and a second one when the instance is odd.
    qint64 p = r.offset + ((r.instance & 1) ? 32 : 16);
    QString name;
    bool metafile = false;
    switch (r.type) {
    case 0xF01A: name = QStringLiteral("emf"); metafile = true; break;
    case 0xF01B: name = QStringLiteral("wmf"); metafile = true; break;
    case 0xF01C: return QByteArray();   // a Macintosh picture: nothing here shows one
    case 0xF01D: case 0xF02A: name = QStringLiteral("jpeg"); break;
    case 0xF01E: name = QStringLiteral("png"); break;
    case 0xF01F: name = QStringLiteral("bmp"); break;
    case 0xF029: name = QStringLiteral("tiff"); break;
    default: return QByteArray();
    }
    QByteArray file;
    if (metafile) {
        // cbSize, rcBounds, ptSize, cbSave, compression, filter.
        if (end - p < 34) return QByteArray();
        const quint32 size = u32(data, p), saved = u32(data, p + 28);
        const quint8 compression = u8(data, p + 32);
        const qint64 start = p + 34, available = end - start;
        const qint64 length = saved > 0 ? std::min<qint64>(saved, available) : available;
        if (compression == 0x00) {
            if (size > kMaxMetafile) return QByteArray();
            bool ok = false;
            file = odrawInflate(data.mid(start, length), size, &ok);
            if (!ok) return QByteArray();
        } else if (compression == 0xFE) {
            if (length > kMaxMetafile) return QByteArray();
            file = data.mid(start, length);
        } else {
            return QByteArray();
        }
        if (name == QLatin1String("wmf") && u32(file, 0) != 0x9AC6CDD7u)
            file = placeableHeader(i32(data, p + 4), i32(data, p + 8), i32(data, p + 12), i32(data, p + 16), i32(data, p + 20), i32(data, p + 24)) + file;
    } else {
        // The tag byte, then the file.
        if (end - p < 1) return QByteArray();
        file = data.mid(p + 1, end - p - 1);
        if (name == QLatin1String("bmp")) file = bitmapFileOf(file);
    }
    if (file.isEmpty()) return QByteArray();
    if (format) *format = name;
    return file;
}

QVector<OdrawStoreEntry> odrawStore(const QByteArray &bstoreContainer)
{
    QVector<OdrawStoreEntry> out;
    // The container's record, or the file blocks without it.
    QVector<OdrawRecord> blocks = odrawRecords(bstoreContainer);
    if (!blocks.isEmpty() && blocks.first().type == 0xF001) {
        const OdrawRecord c = blocks.first();
        blocks = odrawRecords(bstoreContainer, c.offset, qsizetype(qint64(c.offset) + c.length));
    }
    for (const OdrawRecord &r : blocks) {
        if (out.size() >= kMaxStoreEntries) break;
        const qint64 end = qint64(r.offset) + r.length;
        OdrawStoreEntry e;
        e.index = int(out.size()) + 1;
        if (r.type == 0xF007) {
            // btWin32, btMacOS, uid, tag, size, cRef, foDelay, then the name's
            // length and its bytes; a picture record follows when it is kept here.
            if (r.length >= 36) {
                e.delayOffset = u32(bstoreContainer, r.offset + 28);
                const qint64 start = qint64(r.offset) + 36 + u8(bstoreContainer, r.offset + 33);
                if (end - start >= 8) {
                    const QVector<OdrawRecord> blip = odrawRecords(bstoreContainer, qsizetype(start), qsizetype(end));
                    if (!blip.isEmpty() && blip.first().type >= 0xF018 && blip.first().type <= 0xF117)
                        e.embedded = bstoreContainer.mid(qsizetype(start), qsizetype(8 + qint64(blip.first().length)));
                }
            }
        } else if (r.type >= 0xF018 && r.type <= 0xF117) {
            e.embedded = bstoreContainer.mid(r.offset - 8, qsizetype(8 + qint64(r.length)));
        } else {
            continue;
        }
        out << e;
    }
    return out;
}

} // namespace jp::office
