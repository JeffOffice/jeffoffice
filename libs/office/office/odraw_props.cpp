// A shape's properties (odraw_props.h).

#include "office/odraw_props.h"

#include "office/odraw_read.h"

#include <algorithm>

namespace jp::office::odraw {

namespace {

// The properties that hold arrays; some writers leave the 6-byte header out of the length.
bool isArrayProperty(quint16 id)
{
    switch (id) {
    case 0x0145: case 0x0146: case 0x0151: case 0x0152: case 0x0155: case 0x0156: case 0x0157: case 0x0159:
    case 0x0196: case 0x0197: case 0x01CF: case 0x0383:
        return true;
    default:
        return false;
    }
}

int elementSize(quint16 recorded) { return recorded == 0xFFF0 ? 4 : recorded; }

// The properties that are sets of flags: the number ends in 0x3F.
bool isFlagProperty(quint16 id) { return (id & 0x3F) == 0x3F; }

// The flags of a boolean property with `over`'s used ones set over `base`'s.
quint32 mergedFlags(quint32 base, quint32 over)
{
    const quint32 used = over >> 16;
    return ((base >> 16 | used) << 16) | ((base & ~used & 0xFFFF) | (over & used));
}

} // namespace

void Props::add(const QByteArray &data, const OdrawRecord &record)
{
    const qint64 start = record.offset, end = qint64(record.offset) + record.length;
    const qint64 count = std::min<qint64>(record.instance, (end - start) / 6);
    qint64 tail = start + count * 6;   // where the complex data begins
    for (qint64 i = 0; i < count; ++i) {
        const quint16 opid = u16(data, start + i * 6);
        const quint16 id = opid & 0x3FFF;
        Prop prop;
        prop.value = u32(data, start + i * 6 + 2);
        if ((opid & 0x8000) && prop.value > 0) {
            qint64 size = prop.value;
            if (isArrayProperty(id) && end - tail >= 6) {
                // The length may count the elements only: the header comes before them.
                const qint64 n = u16(data, tail), each = elementSize(u16(data, tail + 4));
                if (n > 0 && n * each == size) size += 6;
            }
            if (size <= end - tail) {
                prop.complex = data.mid(qsizetype(tail), qsizetype(size));
                tail += size;
            } else {
                tail = end;   // damaged: nothing more to read after the table
            }
        }
        if (isFlagProperty(id) && !(opid & 0x8000)) {
            const auto earlier = m_props.constFind(id);
            if (earlier != m_props.constEnd()) prop.value = mergedFlags(earlier->value, prop.value);
        }
        m_props.insert(id, prop);
    }
}

void Props::set(quint16 id, quint32 value)
{
    Prop prop;
    prop.value = value;
    m_props.insert(id, prop);
}

void Props::inherit(const Props &under)
{
    for (auto it = under.m_props.cbegin(); it != under.m_props.cend(); ++it) {
        const auto mine = m_props.find(it.key());
        if (mine == m_props.end()) m_props.insert(it.key(), it.value());
        else if (isFlagProperty(it.key()) && mine->complex.isEmpty()) mine->value = mergedFlags(it->value, mine->value);
    }
}

quint32 Props::value(quint16 id, quint32 fallback) const
{
    const auto it = m_props.constFind(id);
    if (it == m_props.constEnd()) return m_fallback ? m_fallback->value(id, fallback) : fallback;
    return it->value;
}

double Props::fixed(quint16 id, double fallback) const
{
    const auto it = m_props.constFind(id);
    if (it == m_props.constEnd()) return m_fallback ? m_fallback->fixed(id, fallback) : fallback;
    return qint32(it->value) / 65536.0;
}

bool Props::flag(quint16 id, quint32 bit, bool fallback) const
{
    const auto it = m_props.constFind(id);
    if (it == m_props.constEnd() || !(it->value & (bit << 16))) return m_fallback ? m_fallback->flag(id, bit, fallback) : fallback;
    return (it->value & bit) != 0;
}

QByteArray Props::complex(quint16 id) const
{
    const auto it = m_props.constFind(id);
    if (it == m_props.constEnd()) return m_fallback ? m_fallback->complex(id) : QByteArray();
    return it->complex;
}

QString Props::text(quint16 id) const { return utf16Text(complex(id)); }

ArrayView arrayOf(const QByteArray &complex)
{
    ArrayView v;
    if (complex.size() < 6) return v;
    v.size = elementSize(u16(complex, 4));
    if (v.size <= 0) return v;
    v.count = int(std::min<qint64>(u16(complex, 0), (qint64(complex.size()) - 6) / v.size));
    return v;
}

QString utf16Text(const QByteArray &bytes, qint64 offset)
{
    QString s;
    for (qint64 p = offset; p + 2 <= bytes.size(); p += 2) {
        const quint16 c = u16(bytes, p);
        if (c == 0) break;
        s += QChar(c);
    }
    return s;
}

} // namespace jp::office::odraw
