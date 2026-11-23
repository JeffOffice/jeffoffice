#pragma once
// Damage done to a compound file, built in code from a sound one our own writers make, for the tests of Open and Repair of every old binary format (a document, a presentation, a
// workbook). The functions work on the bytes: the header, the table of sectors, the directory, and the mini stream are found as the format lays them out.

#include <QByteArray>
#include <QList>
#include <QPair>
#include <QString>
#include <QtEndian>

#include <algorithm>
#include <functional>

namespace cfbdamage {

constexpr quint32 kEnd = 0xFFFFFFFE;

inline quint32 get32(const QByteArray &f, qint64 at) { return at >= 0 && at + 4 <= f.size() ? qFromLittleEndian<quint32>(f.constData() + at) : 0; }
inline void set32(QByteArray &f, qint64 at, quint32 v) { if (at >= 0 && at + 4 <= f.size()) qToLittleEndian(v, f.data() + at); }
inline void set16(QByteArray &f, qint64 at, quint16 v) { if (at >= 0 && at + 2 <= f.size()) qToLittleEndian(v, f.data() + at); }

// A table entry of the table of sectors (or of the mini stream's table, with `mini`), and the sectors of a chain, read from a sound file.
inline qint64 sectorAt(int n) { return (qint64(n) + 1) * 512; }
inline qint64 fatAt(const QByteArray &f, quint32 n)
{
    const quint32 sector = get32(f, 76 + (n / 128) * 4);
    return sectorAt(int(sector)) + (n % 128) * 4;
}
inline quint32 fatGet(const QByteArray &f, quint32 n) { return get32(f, fatAt(f, n)); }
inline void fatSet(QByteArray &f, quint32 n, quint32 v) { set32(f, fatAt(f, n), v); }
inline QList<quint32> chainOf(const QByteArray &f, quint32 start)
{
    QList<quint32> out;
    for (quint32 n = start; n < 100000 && out.size() < 100000; n = fatGet(f, n)) out << n;
    return out;
}
// The offset of a directory entry by name (the directory is read through the table of sectors).
inline qint64 entryOf(const QByteArray &f, const QString &name)
{
    for (quint32 s : chainOf(f, get32(f, 48)))
        for (qint64 o = sectorAt(int(s)); o + 128 <= sectorAt(int(s)) + 512 && o + 128 <= f.size(); o += 128) {
            const int len = qFromLittleEndian<quint16>(f.constData() + o + 64) / 2 - 1;
            if (len > 0 && QString::fromUtf16(reinterpret_cast<const char16_t *>(f.constData() + o), len) == name) return o;
        }
    return -1;
}
inline quint32 startOf(const QByteArray &f, const QString &name) { return get32(f, entryOf(f, name) + 116); }
inline quint32 sizeOf(const QByteArray &f, const QString &name) { return get32(f, entryOf(f, name) + 120); }
// The sectors of a stream kept in sectors (not in the mini stream).
inline QList<quint32> sectorsOf(const QByteArray &f, const QString &name) { return chainOf(f, startOf(f, name)); }

struct Variant {
    QString id;      // "a", "b", ..., with what was done
    QString what;
    QByteArray file;
};

// Two streams kept in sectors (`name` and `other`), both sound, with the first sectors of `other` moved to lie in the file right after the k-th sector of `name`'s chain (the chains
// are changed to follow the bytes, so that both streams still read). `*first` is the sector where `other` now starts, and `*moved` how many of its sectors lie there.
inline QByteArray placeAfter(const QByteArray &good, const QString &name, const QString &other, int k, quint32 *first = nullptr, int *moved = nullptr)
{
    QByteArray f = good;
    const QList<quint32> c = sectorsOf(good, name), o = sectorsOf(good, other);
    const int m = int(std::min<qsizetype>(o.size(), c.size() - k - 1));
    if (m <= 0) return f;
    for (int j = 0; j < m; ++j) {
        const QByteArray a = good.mid(sectorAt(int(c[k + 1 + j])), 512), b = good.mid(sectorAt(int(o[j])), 512);
        f.replace(sectorAt(int(c[k + 1 + j])), 512, b);
        f.replace(sectorAt(int(o[j])), 512, a);
    }
    QList<quint32> nameSeq = c.mid(0, k + 1), otherSeq;
    for (int j = 0; j < m; ++j) nameSeq << o[j];
    nameSeq += c.mid(k + 1 + m);
    for (int j = 0; j < m; ++j) otherSeq << c[k + 1 + j];
    otherSeq += o.mid(m);
    for (const QList<quint32> &seq : {nameSeq, otherSeq})
        for (int i = 0; i < int(seq.size()); ++i) fatSet(f, seq[i], i + 1 < int(seq.size()) ? seq[i + 1] : kEnd);
    set32(f, entryOf(f, other) + 116, otherSeq[0]);
    if (first) *first = otherSeq[0];
    if (moved) *moved = m;
    return f;
}

// The arrangements of a damaged stream beside a neighbor whose own start field is damaged, so that no chain claims its sectors. In both the neighbor's sectors follow the k-th
// sector of `name`'s chain in the file. (1) the chain ends there with its end marker and the directory gives three times the size; (2) the link after it is broken (past the end of the file).
inline QByteArray neighborAfterEnd(const QByteArray &placed, const QString &name, const QString &other, int k)
{
    QByteArray f = placed;
    const QList<quint32> c = sectorsOf(placed, name);
    fatSet(f, c[k], kEnd);
    set32(f, entryOf(f, name) + 120, sizeOf(placed, name) * 3);
    set32(f, entryOf(f, other) + 116, 0x00FFFFF0);
    return f;
}
inline QByteArray neighborAfterBrokenLink(const QByteArray &placed, const QString &name, const QString &other, int k)
{
    QByteArray f = placed;
    const QList<quint32> c = sectorsOf(placed, name);
    fatSet(f, c[k], 0x00FFFFF0);
    set32(f, entryOf(f, other) + 116, 0x00FFFFF0);
    return f;
}

// The damage to the sectors of the stream `name` (one kept in sectors, at least 8 of them) and, to name a second stream for the sharing, `other`.
inline QList<Variant> sectorDamage(const QByteArray &good, const QString &name, const QString &other)
{
    QList<Variant> out;
    const QList<quint32> c = sectorsOf(good, name);
    const int k = int(c.size()) / 2;
    {   // (a) two sectors of the stream swapped in place, and the chain made to follow them: sound, only out of order
        QByteArray f = good;
        const QByteArray one = f.mid(sectorAt(int(c[1])), 512), two = f.mid(sectorAt(int(c[2])), 512);
        f.replace(sectorAt(int(c[1])), 512, two);
        f.replace(sectorAt(int(c[2])), 512, one);
        // the stream was c1, c2; it is now read c2' = (c1's place holds c2's bytes): the chain is the places in the order of the bytes
        fatSet(f, c[0], c[2]);
        fatSet(f, c[2], c[1]);
        fatSet(f, c[1], c[3]);
        out << Variant{QStringLiteral("a"), QStringLiteral("two sectors of the stream out of order, the chain valid"), f};
    }
    {   // (b) a chain entry past the end of the file
        QByteArray f = good;
        fatSet(f, c[k], 0x00FFFFF0);
        out << Variant{QStringLiteral("b"), QStringLiteral("a chain entry points past the end of the file"), f};
    }
    {   // (c) a chain that loops back
        QByteArray f = good;
        fatSet(f, c[k], c[1]);
        out << Variant{QStringLiteral("c"), QStringLiteral("the chain loops back to an earlier sector"), f};
    }
    {   // (d) a chain that ends early
        QByteArray f = good;
        fatSet(f, c[k], kEnd);
        out << Variant{QStringLiteral("d"), QStringLiteral("the chain ends half way, the directory says more"), f};
    }
    {   // (e) another stream shares a sector of this one
        QByteArray f = good;
        set32(f, entryOf(f, other) + 116, c[k]);
        out << Variant{QStringLiteral("e"), QStringLiteral("another stream starts in a sector of this one"), f};
    }
    {   // (f) the directory entry of the stream: the start, and the size too large and too small
        QByteArray f = good;
        set32(f, entryOf(f, name) + 116, c[1]);
        out << Variant{QStringLiteral("f1"), QStringLiteral("directory start sector is the second sector"), f};
        f = good;
        set32(f, entryOf(f, name) + 116, 0x00FFFFF0);
        out << Variant{QStringLiteral("f2"), QStringLiteral("directory start sector is past the end"), f};
        f = good;
        set32(f, entryOf(f, name) + 120, sizeOf(good, name) * 3);
        out << Variant{QStringLiteral("f3"), QStringLiteral("directory size is three times too large"), f};
        f = good;
        set32(f, entryOf(f, name) + 120, sizeOf(good, name) / 2);
        out << Variant{QStringLiteral("f4"), QStringLiteral("directory size is half the stream"), f};
        f = good;
        set32(f, entryOf(f, name) + 120, 0);
        out << Variant{QStringLiteral("f5"), QStringLiteral("directory size is zero"), f};
    }
    return out;
}

// The damage to the container of the small streams: `mini` is a file with streams under 4096 bytes.
inline QList<Variant> miniDamage(const QByteArray &mini)
{
    QList<Variant> out;
    const quint32 rootStart = get32(mini, entryOf(mini, QStringLiteral("Root Entry")) + 116);
    const quint32 miniFat = get32(mini, 60);
    {
        QByteArray f = mini;
        f.replace(sectorAt(int(miniFat)), 512, QByteArray(512, '\0'));
        out << Variant{QStringLiteral("g1"), QStringLiteral("the table of the mini stream is zeroed"), f};
    }
    {
        QByteArray f = mini;
        set32(f, 60, kEnd);
        set32(f, 64, 0);
        out << Variant{QStringLiteral("g2"), QStringLiteral("the header says there is no table of the mini stream"), f};
    }
    {
        QByteArray f = mini;
        set32(f, entryOf(f, QStringLiteral("Root Entry")) + 116, kEnd);
        out << Variant{QStringLiteral("g3"), QStringLiteral("the root entry has no start sector, so the mini stream is lost"), f};
    }
    {
        QByteArray f = mini;
        set32(f, entryOf(f, QStringLiteral("Root Entry")) + 120, 64);
        out << Variant{QStringLiteral("g4"), QStringLiteral("the root entry gives the mini stream 64 bytes"), f};
    }
    {
        QByteArray f = mini;
        for (int i = 0; i < 128; ++i) set32(f, sectorAt(int(miniFat)) + i * 4, quint32(i % 3));   // every small chain loops
        out << Variant{QStringLiteral("g5"), QStringLiteral("every chain of the mini table loops"), f};
    }
    {
        QByteArray f = mini;
        f.replace(sectorAt(int(rootStart)), 512, QByteArray(512, '\0'));
        out << Variant{QStringLiteral("g6"), QStringLiteral("the first sector of the mini stream is zeroed"), f};
    }
    return out;
}

inline QList<Variant> cuts(const QByteArray &good)
{
    QList<Variant> out;
    for (int p : {25, 50, 90}) out << Variant{QStringLiteral("h%1").arg(p), QStringLiteral("cut at %1 percent").arg(p), good.left(good.size() * p / 100)};
    return out;
}

inline QList<Variant> headerDamage(const QByteArray &good)
{
    QList<Variant> out;
    auto add = [&](const char *id, const char *what, const std::function<void(QByteArray &)> &edit) {
        QByteArray f = good;
        edit(f);
        out << Variant{QString::fromLatin1(id), QString::fromLatin1(what), f};
    };
    add("i1", "sector size field is 0xFFFF", [](QByteArray &f) { set16(f, 30, 0xFFFF); });
    add("i2", "sector size field says 4096 bytes, the file is of 512", [](QByteArray &f) { set16(f, 30, 12); });
    add("i3", "table count is zero", [](QByteArray &f) { set32(f, 44, 0); });
    add("i4", "table count is 100000", [](QByteArray &f) { set32(f, 44, 100000); });
    add("i5", "directory start sector is past the end", [](QByteArray &f) { set32(f, 48, 0x00FFFFF0); });
    add("i6", "directory start sector is zero", [](QByteArray &f) { set32(f, 48, 0); });
    add("i7", "mini sector size field is zero", [](QByteArray &f) { set16(f, 32, 0); });
    add("i8", "small-stream cutoff is zero", [](QByteArray &f) { set32(f, 56, 0); });
    add("i9", "signature damaged by one byte", [](QByteArray &f) { f[3] = char(f[3] ^ 0x40); });
    return out;
}

} // namespace cfbdamage
