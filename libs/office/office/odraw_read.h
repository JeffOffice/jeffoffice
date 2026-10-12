#pragma once
// Reading little-endian numbers from a binary drawing's bytes, with every
// offset and length checked in 64 bits: what lies outside the data reads as 0.

#include <QByteArray>
#include <QtEndian>

namespace jp::office::odraw {

inline bool fits(const QByteArray &d, qint64 offset, qint64 length)
{
    return offset >= 0 && length >= 0 && offset <= qint64(d.size()) && length <= qint64(d.size()) - offset;
}
inline quint8 u8(const QByteArray &d, qint64 offset) { return fits(d, offset, 1) ? quint8(d.at(offset)) : 0; }
inline quint16 u16(const QByteArray &d, qint64 offset) { return fits(d, offset, 2) ? qFromLittleEndian<quint16>(d.constData() + offset) : 0; }
inline quint32 u32(const QByteArray &d, qint64 offset) { return fits(d, offset, 4) ? qFromLittleEndian<quint32>(d.constData() + offset) : 0; }
inline qint32 i32(const QByteArray &d, qint64 offset) { return qint32(u32(d, offset)); }

} // namespace jp::office::odraw
