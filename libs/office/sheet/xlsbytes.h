#pragma once
// Bytes read with their bounds kept, for the readers of the old workbook format: a read past the end gives zeros and marks the reader bad.

#include <QByteArray>
#include <QtEndian>

namespace jp::office::sheet {

class Bytes {
public:
    explicit Bytes(const QByteArray &b) : m_p(reinterpret_cast<const uchar *>(b.constData())), m_n(b.size()) {}
    Bytes(const QByteArray &b, qsizetype from, qsizetype to) : m_p(reinterpret_cast<const uchar *>(b.constData())), m_n(qMin<qsizetype>(to, b.size())), m_o(from)
    {
        if (from < 0 || from > m_n) fail();
    }
    bool bad() const { return m_bad; }
    qsizetype left() const { return m_n - m_o; }
    qsizetype at() const { return m_o; }
    quint8 u8()
    {
        if (m_o + 1 > m_n) return quint8(fail());
        return m_p[m_o++];
    }
    quint16 u16()
    {
        if (m_o + 2 > m_n) return quint16(fail());
        const quint16 v = qFromLittleEndian<quint16>(m_p + m_o);
        m_o += 2;
        return v;
    }
    quint32 u32()
    {
        if (m_o + 4 > m_n) return quint32(fail());
        const quint32 v = qFromLittleEndian<quint32>(m_p + m_o);
        m_o += 4;
        return v;
    }
    double f64()
    {
        if (m_o + 8 > m_n) return fail();
        const double v = qFromLittleEndian<double>(m_p + m_o);
        m_o += 8;
        return v;
    }
    void skip(qint64 n)
    {
        if (n < 0 || m_o + n > m_n) {
            m_o = m_n;
            m_bad = true;
        } else {
            m_o += qsizetype(n);
        }
    }
    const char *take(qint64 n)
    {
        if (n < 0 || m_o + n > m_n) {
            m_o = m_n;
            m_bad = true;
            return nullptr;
        }
        const char *p = reinterpret_cast<const char *>(m_p + m_o);
        m_o += qsizetype(n);
        return p;
    }

private:
    int fail()
    {
        m_bad = true;
        m_o = m_n;
        return 0;
    }
    const uchar *m_p;
    qsizetype m_n;
    qsizetype m_o = 0;
    bool m_bad = false;
};

} // namespace jp::office::sheet
