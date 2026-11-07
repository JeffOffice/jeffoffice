// Tests of the ZIP layer (io/zip.h, io/zipindex.h) against rules it used to rely on without checking: how much a whole read may inflate, entries that share their bytes, the
// bytes a scan inflates for entries it leaves out, the signature and checksum of what is read, one name met twice, and a ZIP that has more entries or bytes than the old
// fields hold. Every file is built here.
#include "io/zip.h"
#include "io/zipindex.h"

#include <QTemporaryDir>
#include <QtEndian>
#include <QtTest>

#include <zlib.h>

#include "../testrunner.h"

namespace {

void le16(QByteArray &a, quint16 v) { char b[2]; qToLittleEndian(v, b); a.append(b, 2); }
void le32(QByteArray &a, quint32 v) { char b[4]; qToLittleEndian(v, b); a.append(b, 4); }

QByteArray deflated(const QByteArray &in)
{
    z_stream zs{};
    deflateInit2(&zs, 6, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
    QByteArray out(qsizetype(deflateBound(&zs, uLong(in.size()))), Qt::Uninitialized);
    zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(in.constData()));
    zs.avail_in = uInt(in.size());
    zs.next_out = reinterpret_cast<Bytef *>(out.data());
    zs.avail_out = uInt(out.size());
    ::deflate(&zs, Z_FINISH);
    out.resize(qsizetype(zs.total_out));
    deflateEnd(&zs);
    return out;
}

// One local entry: its bytes (deflated or stored) and the header in front.
struct Made {
    QByteArray bytes;
    quint32 crc = 0, csize = 0, usize = 0;
    quint16 method = 0;
};

Made makeEntry(const QString &name, const QByteArray &data, bool compress)
{
    Made m;
    const QByteArray packed = compress ? deflated(data) : data;
    m.crc = jp::crc32(data);
    m.csize = quint32(packed.size());
    m.usize = quint32(data.size());
    m.method = compress ? 8 : 0;
    const QByteArray n = name.toUtf8();
    le32(m.bytes, 0x04034b50);
    le16(m.bytes, 20); le16(m.bytes, 0); le16(m.bytes, m.method); le16(m.bytes, 0); le16(m.bytes, 0x21);
    le32(m.bytes, m.crc); le32(m.bytes, m.csize); le32(m.bytes, m.usize);
    le16(m.bytes, quint16(n.size())); le16(m.bytes, 0);
    m.bytes.append(n);
    m.bytes.append(packed);
    return m;
}

// A directory record for an entry whose local header is at `offset`.
QByteArray directoryRecord(const QString &name, const Made &m, quint32 offset)
{
    QByteArray r;
    const QByteArray n = name.toUtf8();
    le32(r, 0x02014b50);
    le16(r, 20); le16(r, 20); le16(r, 0); le16(r, m.method); le16(r, 0); le16(r, 0x21);
    le32(r, m.crc); le32(r, m.csize); le32(r, m.usize);
    le16(r, quint16(n.size())); le16(r, 0); le16(r, 0); le16(r, 0); le16(r, 0);
    le32(r, 0); le32(r, offset);
    r.append(n);
    return r;
}

QByteArray endRecord(int count, quint32 dirSize, quint32 dirAt)
{
    QByteArray r;
    le32(r, 0x06054b50);
    le16(r, 0); le16(r, 0); le16(r, quint16(count)); le16(r, quint16(count));
    le32(r, dirSize); le32(r, dirAt); le16(r, 0);
    return r;
}

// Entries (name, data), each with its own bytes, and a directory that has the entries named in `points`: for each, the index of the made entry its directory record points at.
QByteArray zipWithPoints(const QVector<QPair<QString, QByteArray>> &entries, const QVector<QPair<QString, int>> &points, bool compress)
{
    QByteArray body, dir;
    QVector<Made> made;
    QVector<quint32> at;
    for (const auto &e : entries) {
        at << quint32(body.size());
        made << makeEntry(e.first, e.second, compress);
        body.append(made.last().bytes);
    }
    for (const auto &p : points) dir.append(directoryRecord(p.first, made[p.second], at[p.second]));
    return body + dir + endRecord(int(points.size()), quint32(dir.size()), quint32(body.size()));
}

// Bytes that do not compress to nothing: a simple generator.
QByteArray noise(int n, quint32 seed)
{
    QByteArray b(n, Qt::Uninitialized);
    for (int i = 0; i < n; ++i) {
        seed = seed * 1664525u + 1013904223u;
        b[i] = char(seed >> 24);
    }
    return b;
}

} // namespace

class SheetZipTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // Entries read together may not inflate to more than a total, as a package's may not: each one is under the limit of an entry, and together they are over.
    void aWholeReadStopsAtATotalInflated()
    {
        QVector<QPair<QString, QByteArray>> entries;
        QVector<QPair<QString, int>> points;
        for (int i = 0; i < 5; ++i) {
            entries << qMakePair(QStringLiteral("e%1").arg(i), QByteArray(600000, char('a' + i)));
            points << qMakePair(QStringLiteral("e%1").arg(i), i);
        }
        const QByteArray zip = zipWithPoints(entries, points, true);
        QMap<QString, QByteArray> out;
        QString why;
        QVERIFY(!jp::readZip(zip, out, &why, qint64(2) << 20));
        QVERIFY(!why.isEmpty());
        out.clear();
        QVERIFY(jp::readZip(zip, out, &why, qint64(4) << 20));
        QCOMPARE(out.size(), 5);
    }

    // Two directory records that point at the same bytes, or at bytes that overlap, are no ZIP any reader here takes: forty of them would be forty times the bytes.
    void entriesThatShareTheirBytesAreRefused()
    {
        const QVector<QPair<QString, QByteArray>> entries = {{QStringLiteral("a"), QByteArray(5000, 'x')}, {QStringLiteral("b"), QByteArray(5000, 'y')}};
        const QByteArray fine = zipWithPoints(entries, {{QStringLiteral("a"), 0}, {QStringLiteral("b"), 1}}, true);
        QMap<QString, QByteArray> out;
        QVector<jp::ZipEntry> dir;
        QVERIFY(jp::readZip(fine, out));
        QVERIFY(jp::readZipDirectory(fine, &dir));
        const QByteArray shared = zipWithPoints(entries, {{QStringLiteral("a"), 0}, {QStringLiteral("b"), 0}, {QStringLiteral("c"), 0}}, true);
        out.clear();
        QVERIFY(!jp::readZip(shared, out));
        QVERIFY(!jp::readZipDirectory(shared, &dir));
        // a record whose bytes begin inside another's
        QByteArray body = makeEntry(QStringLiteral("a"), noise(3000, 1), false).bytes;
        const Made second = makeEntry(QStringLiteral("b"), noise(100, 2), false);
        QByteArray overlap = body + directoryRecord(QStringLiteral("a"), makeEntry(QStringLiteral("a"), noise(3000, 1), false), 0);
        const quint32 inside = quint32(body.size() - 500);
        overlap += directoryRecord(QStringLiteral("b"), second, inside);
        overlap += endRecord(2, quint32(overlap.size() - body.size()), quint32(body.size()));
        QVERIFY(!jp::readZipDirectory(overlap, &dir));
    }

    // A scan charges what it inflates for an entry it then leaves out as too large, so entries of that kind run it out of its total, not out of time.
    void aScanChargesTheBytesItInflatedForTooLargeEntries()
    {
        QByteArray zip;
        for (int i = 0; i < 20; ++i) zip.append(makeEntry(QStringLiteral("big%1").arg(i), QByteArray(1200000, 'z'), true).bytes);
        int seen = 0;
        jp::ZipScanStats stats;
        jp::scanZipEach(zip, [&](jp::ZipScanEntry &e) {
            QCOMPARE(e.problem, QStringLiteral("too large"));
            ++seen;
        }, 10000, 1000000, 4000000, &stats);
        QVERIFY2(seen >= 3 && seen <= 5, qPrintable(QString::number(seen)));
        QVERIFY(stats.sizeLeftOut);
    }

    // A directory record that points where no local header is, or at a stored entry whose bytes are not the ones its checksum is of, is damage.
    void theLocalHeaderAndTheChecksumOfAStoredEntryAreChecked()
    {
        QByteArray zip = zipWithPoints({{QStringLiteral("a"), QByteArray("hello world")}}, {{QStringLiteral("a"), 0}}, false);
        QMap<QString, QByteArray> out;
        QVERIFY(jp::readZip(zip, out));
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        auto save = [&](const QString &name, const QByteArray &bytes) {
            QFile f(dir.filePath(name));
            if (!f.open(QIODevice::WriteOnly)) return QString();
            f.write(bytes);
            return f.fileName();
        };
        QCOMPARE(jp::readZipEntry(save(QStringLiteral("ok.zip"), zip), QStringLiteral("a")), QByteArray("hello world"));
        QByteArray noHeader = zip;
        noHeader[0] = 'X';
        out.clear();
        QVERIFY(!jp::readZip(noHeader, out));
        QVERIFY(jp::readZipEntry(save(QStringLiteral("nohead.zip"), noHeader), QStringLiteral("a")).isEmpty());
        QByteArray wrongBytes = zip;
        wrongBytes[wrongBytes.indexOf("hello") + 1] = 'E';
        out.clear();
        QVERIFY(!jp::readZip(wrongBytes, out));
        QVERIFY(jp::readZipEntry(save(QStringLiteral("crc.zip"), wrongBytes), QStringLiteral("a")).isEmpty());
    }

    // Of two entries with one name, every reader gives the same one: the later.
    void oneNameMetTwiceResolvesTheSameInEveryReader()
    {
        jp::ZipWriter w;
        w.add(QStringLiteral("a"), QByteArray("first"));
        w.add(QStringLiteral("a"), QByteArray("second"));
        const QByteArray zip = w.finish();
        QMap<QString, QByteArray> out;
        QVERIFY(jp::readZip(zip, out));
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QFile f(dir.filePath(QStringLiteral("dup.zip")));
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(zip);
        f.close();
        QCOMPARE(jp::readZipEntry(f.fileName(), QStringLiteral("a")), out.value(QStringLiteral("a")));
        QCOMPARE(out.value(QStringLiteral("a")), QByteArray("second"));
    }

    // More entries than the old count field holds are written in the form that has room, and every reader reads them all back.
    void moreThanSixtyFiveThousandEntriesReadBack()
    {
        constexpr int kCount = 70000;
        jp::ZipWriter w;
        for (int i = 0; i < kCount; ++i) w.add(QStringLiteral("p/%1.txt").arg(i), QByteArray::number(i));
        const QByteArray zip = w.finish();
        QVERIFY(zip.contains(QByteArray("PK\x06\x06")));   // (the long end record)
        QMap<QString, QByteArray> out;
        QVERIFY(jp::readZip(zip, out));
        QCOMPARE(out.size(), kCount);
        QCOMPARE(out.value(QStringLiteral("p/69999.txt")), QByteArray("69999"));
        QCOMPARE(out.value(QStringLiteral("p/0.txt")), QByteArray("0"));
        QVector<jp::ZipEntry> dir;
        QVERIFY(jp::readZipDirectory(zip, &dir));
        QCOMPARE(dir.size(), kCount);
        QByteArray bytes;
        QVERIFY(jp::inflateZipEntry(zip, dir.last(), &bytes));
        QCOMPARE(bytes, QByteArray("69999"));
        QTemporaryDir tmp;
        QVERIFY(tmp.isValid());
        QFile f(tmp.filePath(QStringLiteral("many.zip")));
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(zip);
        f.close();
        QCOMPARE(jp::readZipEntry(f.fileName(), QStringLiteral("p/65536.txt")), QByteArray("65536"));
    }

    // The form that has room for sizes and places past 4 GiB, written for every entry, reads back the same as the short form.
    void theLongFormOfEveryRecordReadsBack()
    {
        jp::ZipWriter w;
        w.useLongRecords(true);
        w.add(QStringLiteral("a.txt"), QByteArray("alpha"));
        w.add(QStringLiteral("b.txt"), QByteArray(5000, 'b'), true);
        w.beginStream(QStringLiteral("c.txt"));
        w.write("gamma", 5);
        w.endStream();
        const QByteArray zip = w.finish();
        QVERIFY(zip.contains(QByteArray("PK\x06\x06")));
        QMap<QString, QByteArray> out;
        QVERIFY(jp::readZip(zip, out));
        QCOMPARE(out.size(), 3);
        QCOMPARE(out.value(QStringLiteral("a.txt")), QByteArray("alpha"));
        QCOMPARE(out.value(QStringLiteral("b.txt")), QByteArray(5000, 'b'));
        QCOMPARE(out.value(QStringLiteral("c.txt")), QByteArray("gamma"));
        QVector<jp::ZipEntry> dir;
        QVERIFY(jp::readZipDirectory(zip, &dir));
        QCOMPARE(dir.size(), 3);
        QCOMPARE(dir[1].usize, qint64(5000));
        QCOMPARE(dir[1].name, QStringLiteral("b.txt"));
    }
};

JP_TEST_CLASS(SheetZipTests)
#include "tst_sheet_zip.moc"
