#include "office/oleprops.h"

#include "sheet/xlscodepage.h"

#include <QTimeZone>
#include <QtEndian>
#include <algorithm>

namespace jp::office {

namespace {

quint32 u32(const QByteArray &d, qint64 o) { return o >= 0 && o + 4 <= d.size() ? qFromLittleEndian<quint32>(d.constData() + o) : 0; }
quint16 u16(const QByteArray &d, qint64 o) { return o >= 0 && o + 2 <= d.size() ? qFromLittleEndian<quint16>(d.constData() + o) : 0; }
quint64 u64(const QByteArray &d, qint64 o) { return o >= 0 && o + 8 <= d.size() ? qFromLittleEndian<quint64>(d.constData() + o) : 0; }

// A GUID as the stream stores it.
QByteArray guid(quint32 a, quint16 b, quint16 c, const unsigned char (&rest)[8])
{
    QByteArray g(16, '\0');
    qToLittleEndian<quint32>(a, g.data());
    qToLittleEndian<quint16>(b, g.data() + 4);
    qToLittleEndian<quint16>(c, g.data() + 6);
    for (int i = 0; i < 8; ++i) g[8 + i] = char(rest[i]);
    return g;
}

// A text value (section 2.5): its size in bytes (with the null that ends it), then the characters, in the set's code page, or in 16 bits for the Unicode page; or (the wide
// string type) a count of 16-bit characters. `at` is where the value's size begins.
QString textAt(const QByteArray &d, qint64 at, bool wide, int codePage)
{
    const quint32 n = u32(d, at);
    const bool sixteen = wide || codePage == 1200;
    const qint64 bytes = wide ? qint64(n) * 2 : qint64(n);
    if (n == 0 || at + 4 > d.size() || bytes > d.size() - (at + 4)) return QString();
    const qint64 room = std::min<qint64>(bytes, 2 * qint64(kMaxOlePropertyChars));
    const char *p = d.constData() + at + 4;
    if (sixteen) {
        QString out;
        for (qint64 i = 0; i + 1 < room; i += 2) {
            const ushort c = qFromLittleEndian<quint16>(p + i);
            if (c == 0) break;
            out += QChar(c);
        }
        return out;
    }
    qint64 len = 0;
    while (len < room && p[len]) ++len;
    if (codePage == 65001) return QString::fromUtf8(p, qsizetype(len));
    return sheet::xlsBytesToText(codePage ? codePage : 1252, p, qsizetype(len));
}

} // namespace

QByteArray summaryInformationFormat()
{
    static const unsigned char rest[8] = {0xAB, 0x91, 0x08, 0x00, 0x2B, 0x27, 0xB3, 0xD9};
    return guid(0xF29F85E0, 0x4FF9, 0x1068, rest);
}

QByteArray documentSummaryInformationFormat()
{
    static const unsigned char rest[8] = {0x93, 0x97, 0x08, 0x00, 0x2B, 0x2C, 0xF9, 0xAE};
    return guid(0xD5CDD502, 0x2E9C, 0x101B, rest);
}

QVector<OlePropertySet> readOlePropertySets(const QByteArray &d)
{
    QVector<OlePropertySet> out;
    // The header (section 2.21): the byte order mark, the version, who wrote it, a class, and the count of sets, each with its kind and its place.
    if (d.size() < 48 || d.size() > kMaxOlePropertyStream || u16(d, 0) != 0xFFFE) return out;
    const quint32 sets = std::min<quint32>(u32(d, 24), 2);
    for (quint32 s = 0; s < sets; ++s) {
        const qint64 entry = 28 + qint64(s) * 20;
        if (entry + 20 > d.size()) break;
        const qint64 base = u32(d, entry + 16);
        // The set (section 2.20): its size, the number of properties, and for each the number and where it is (from the set's start).
        if (base < entry + 20 || base + 8 > d.size()) continue;
        const qint64 size = std::min<qint64>(u32(d, base), d.size() - base);
        if (size < 8) continue;
        const qint64 count = std::min<qint64>({qint64(u32(d, base + 4)), (size - 8) / 8, kMaxOleProperties});
        OlePropertySet set;
        set.format = d.mid(entry, 16);
        struct Place {
            quint32 id;
            qint64 at;
        };
        QVector<Place> places;
        for (qint64 i = 0; i < count; ++i) {
            const quint32 id = u32(d, base + 8 + i * 8);
            const qint64 off = u32(d, base + 12 + i * 8);
            if (off < 8 + count * 8 || off + 8 > size) continue;
            places.push_back({id, base + off});
        }
        // (The code page is read first: the texts are in it, wherever the property is.)
        for (const Place &p : places)
            if (p.id == 1 && u16(d, p.at) == 2) set.codePage = int(u16(d, p.at + 4));
        for (const Place &p : places) {
            if (p.id == 0 || p.id == 1 || p.id >= 0x80000000u) continue;   // (the dictionary, the code page, and the locale and behavior)
            OleProperty v;
            v.type = u16(d, p.at);
            const qint64 value = p.at + 4;
            switch (v.type) {
            case 2: v.number = qint16(u16(d, value)); break;
            case 18: v.number = u16(d, value); break;
            case 3: case 22: v.number = qint32(u32(d, value)); break;
            case 19: case 23: v.number = u32(d, value); break;
            case 20: case 21: v.number = qint64(u64(d, value)); break;
            case 8: case 0x1E: v.text = textAt(d, value, false, set.codePage); break;
            case 0x1F: v.text = textAt(d, value, true, set.codePage); break;
            case 0x40: {
                const quint64 ticks = u64(d, value);
                v.number = qint64(std::min<quint64>(ticks, quint64(INT64_MAX)));
                // From 1601 to 1970 is 11,644,473,600 seconds.
                const qint64 ms = qint64(ticks / 10000) - 11644473600000LL;
                if (ticks && ms > 0 && ms < 253402300800000LL) v.time = QDateTime::fromMSecsSinceEpoch(ms, QTimeZone::UTC);
                break;
            }
            default: continue;
            }
            set.values.insert(p.id, v);
        }
        out.push_back(set);
    }
    return out;
}

} // namespace jp::office
