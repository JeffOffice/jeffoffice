// Binary OfficeArt drawings (odraw.h). This first version walks records; the
// shapes, pictures, and picture store follow.

#include "office/odraw.h"

#include <QtEndian>

namespace jp::office {

QVector<OdrawRecord> odrawRecords(const QByteArray &data, qsizetype offset, qsizetype end)
{
    QVector<OdrawRecord> out;
    if (end < 0 || end > data.size()) end = data.size();
    while (offset >= 0 && offset + 8 <= end) {
        const auto *p = reinterpret_cast<const uchar *>(data.constData() + offset);
        OdrawRecord r;
        const quint16 verInst = qFromLittleEndian<quint16>(p);
        r.version = verInst & 0x0F;
        r.instance = verInst >> 4;
        r.type = qFromLittleEndian<quint16>(p + 2);
        r.length = qFromLittleEndian<quint32>(p + 4);
        r.offset = offset + 8;
        if (qint64(r.offset) + qint64(r.length) > qint64(end)) break;
        out << r;
        offset = r.offset + r.length;
    }
    return out;
}

ItemList readOdrawShapes(const QByteArray &, const OdrawContext &) { return {}; }

Fill odrawBackground(const QByteArray &, const OdrawContext &) { return Fill(); }

QByteArray odrawBlip(const QByteArray &, QString *format)
{
    if (format) format->clear();
    return QByteArray();
}

QVector<OdrawStoreEntry> odrawStore(const QByteArray &) { return {}; }

} // namespace jp::office
