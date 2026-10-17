#pragma once
// Reading little-endian numbers from the bytes of an old binary document: every offset and length is checked in 64 bits, and what lies
// outside the data reads as 0, so a damaged or hostile file can only give wrong numbers, never read out of bounds.

#include <QByteArray>
#include <QtEndian>

namespace jp::office::doc {

// A read-only window onto bytes held elsewhere.
class Span {
public:
    Span() = default;
    Span(const QByteArray &d) : m_p(reinterpret_cast<const uchar *>(d.constData())), m_n(d.size()) {}
    Span(const uchar *p, qint64 n) : m_p(p), m_n(p ? n : 0) {}

    qint64 size() const { return m_n; }
    bool isEmpty() const { return m_n == 0; }
    bool fits(qint64 off, qint64 len) const { return off >= 0 && len >= 0 && off <= m_n && len <= m_n - off; }
    quint8 u8(qint64 o) const { return fits(o, 1) ? m_p[o] : 0; }
    quint16 u16(qint64 o) const { return fits(o, 2) ? qFromLittleEndian<quint16>(m_p + o) : 0; }
    quint32 u32(qint64 o) const { return fits(o, 4) ? qFromLittleEndian<quint32>(m_p + o) : 0; }
    qint8 i8(qint64 o) const { return qint8(u8(o)); }
    qint16 i16(qint64 o) const { return qint16(u16(o)); }
    qint32 i32(qint64 o) const { return qint32(u32(o)); }
    // The part from `off` for `len` bytes, cut to what there is.
    Span sub(qint64 off, qint64 len) const
    {
        if (off < 0 || off > m_n || len <= 0) return Span();
        return Span(m_p + off, len > m_n - off ? m_n - off : len);
    }
    Span from(qint64 off) const { return sub(off, m_n); }
    QByteArray bytes(qint64 off, qint64 len) const
    {
        const Span s = sub(off, len);
        return s.m_n ? QByteArray(reinterpret_cast<const char *>(s.m_p), qsizetype(s.m_n)) : QByteArray();
    }
    const uchar *data() const { return m_p; }

private:
    const uchar *m_p = nullptr;
    qint64 m_n = 0;
};

} // namespace jp::office::doc
