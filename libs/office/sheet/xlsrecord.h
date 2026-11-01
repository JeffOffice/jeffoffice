#pragma once
// A record of the old binary workbook format as a writer puts it in a stream: its number, its length, and its bytes. What does not fit one record (8,224 bytes) goes on in CONTINUE records, never
// cut. (The writers keep to the size where the format has no CONTINUE for the record: a formula, a name, or an array that would not fit is not written as one.)

#include <QByteArray>

#include <algorithm>

namespace jp::office::sheet {

inline void xlsPutRecord(QByteArray *out, quint16 id, const QByteArray &data)
{
    constexpr qsizetype kMaxRecord = 8224;
    qsizetype at = 0;
    quint16 rid = id;
    do {
        const qsizetype n = std::min<qsizetype>(data.size() - at, kMaxRecord);
        out->append(char(rid & 0xFF));
        out->append(char(rid >> 8));
        out->append(char(n & 0xFF));
        out->append(char((n >> 8) & 0xFF));
        out->append(data.constData() + at, n);
        at += n;
        rid = 0x003C;
    } while (at < data.size());
}

} // namespace jp::office::sheet
