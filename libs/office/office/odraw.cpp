// Binary OfficeArt drawings (odraw.h): the records. The shapes are in
// odraw_shapes.cpp, the pictures and the picture store in odraw_blip.cpp.

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

} // namespace jp::office
