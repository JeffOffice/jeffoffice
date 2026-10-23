// Tests of the lazy package reader (Package::readLazy): the same package as read() gives, with each part kept compressed until it is asked for.
#include "io/zip.h"
#include "io/zipindex.h"
#include "office/package.h"

#include <QtEndian>
#include <QtTest>

#include "../testrunner.h"

using namespace jp::office;

namespace {

using Entries = QVector<QPair<QString, QByteArray>>;

const QString kRelsType = QStringLiteral("application/vnd.openxmlformats-package.relationships+xml");
const QString kOfficeDoc = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument");

QByteArray zipOf(const Entries &entries, bool compress = true)
{
    jp::ZipWriter z;
    for (const auto &e : entries) z.add(e.first, e.second, compress);
    return z.finish();
}

QByteArray typesXml()
{
    return "<?xml version=\"1.0\"?><Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
           "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
           "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
           "<Override PartName=\"/xl/workbook.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/></Types>";
}

QByteArray relsXml(const QString &body)
{
    return ("<?xml version=\"1.0\"?><Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">" + body + "</Relationships>").toUtf8();
}

// A small package: a workbook, two sheets, and a relationship from the workbook to each.
Entries smallEntries()
{
    return {
        {QStringLiteral("[Content_Types].xml"), typesXml()},
        {QStringLiteral("_rels/.rels"), relsXml("<Relationship Id=\"rId1\" Type=\"" + kOfficeDoc + "\" Target=\"xl/workbook.xml\"/>")},
        {QStringLiteral("xl/workbook.xml"), "<workbook/>"},
        {QStringLiteral("xl/_rels/workbook.xml.rels"),
         relsXml("<Relationship Id=\"rId1\" Type=\"http://x/worksheet\" Target=\"worksheets/sheet1.xml\"/>"
                 "<Relationship Id=\"rId2\" Type=\"http://x/worksheet\" Target=\"worksheets/sheet2.xml\"/>")},
        {QStringLiteral("xl/worksheets/sheet1.xml"), "<worksheet>one</worksheet>"},
        {QStringLiteral("xl/worksheets/sheet2.xml"), "<worksheet>two two two two two two</worksheet>"},
    };
}

// Sets a 32-bit field of the central directory entry whose name starts with `prefix`.
void patchEntry(QByteArray &zip, const QString &prefix, int fieldAt, quint32 value)
{
    for (qsizetype at = zip.indexOf(QByteArray("PK\x01\x02", 4)); at >= 0; at = zip.indexOf(QByteArray("PK\x01\x02", 4), at + 4)) {
        const quint16 nameLength = qFromLittleEndian<quint16>(zip.constData() + at + 28);
        if (QString::fromUtf8(zip.mid(at + 46, nameLength)).startsWith(prefix)) qToLittleEndian<quint32>(value, zip.data() + at + fieldAt);
    }
}

// The same package in both modes: what each says must be the same.
void compareModes(const QByteArray &zip, const char *what)
{
    Package eager, lazy;
    QString eagerError, lazyError;
    const bool a = eager.read(zip, &eagerError);
    const bool b = lazy.readLazy(zip, &lazyError);
    QVERIFY2(a == b, what);
    if (!a) {
        QVERIFY2(eagerError == lazyError, what);
        return;
    }
    QVERIFY2(lazy.isLazy() && !eager.isLazy(), what);
    QCOMPARE(lazy.partNames(), eager.partNames());
    QCOMPARE(lazy.skippedCount(), eager.skippedCount());
    QCOMPARE(lazy.mainPart(), eager.mainPart());
    for (const QString &n : eager.partNames()) {
        QVERIFY2(lazy.hasPart(n), what);
        QVERIFY2(lazy.part(n) == eager.part(n), qPrintable(QString::fromLatin1(what) + ": " + n));
        QCOMPARE(lazy.partSize(n), qint64(eager.part(n).size()));
        QCOMPARE(lazy.contentType(n), eager.contentType(n));
        QCOMPARE(lazy.relationships(n).size(), eager.relationships(n).size());
    }
    QCOMPARE(lazy.relationships(QStringLiteral("/")).size(), eager.relationships(QStringLiteral("/")).size());
}

} // namespace

class SheetPackageTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // A part that is never asked for is never inflated: the work is counted, in the bytes that came out of an inflater.
    void aPartNeverAskedForIsNeverInflated()
    {
        Entries e = smallEntries();
        const qint64 hugeSize = qint64(48) << 20;
        e.append({QStringLiteral("xl/worksheets/huge.xml"), QByteArray(int(hugeSize), 'a')});
        const QByteArray zip = zipOf(e);
        QVERIFY2(zip.size() < (1 << 20), "the huge part compresses to little");
        Package p;
        QString why;
        QVERIFY2(p.readLazy(zip, &why), qPrintable(why));
        QVERIFY(p.isLazy());
        QVERIFY(p.hasPart(QStringLiteral("/xl/worksheets/huge.xml")));
        QCOMPARE(p.partSize(QStringLiteral("/xl/worksheets/huge.xml")), hugeSize);
        // Only [Content_Types].xml and the relationship parts were inflated.
        QVERIFY2(p.inflatedBytes() < 4096, qPrintable(QString::number(p.inflatedBytes())));
        const qint64 before = p.inflatedBytes();
        QCOMPARE(p.part(QStringLiteral("/xl/worksheets/sheet1.xml")), QByteArray("<worksheet>one</worksheet>"));
        QCOMPARE(p.inflatedBytes() - before, qint64(26));
        QVERIFY(p.inflatedBytes() < 4096);
        // Asked for, it is inflated: all of it.
        QCOMPARE(p.part(QStringLiteral("/xl/worksheets/huge.xml")).size(), hugeSize);
        QVERIFY(p.inflatedBytes() >= hugeSize);
        // A package read whole inflated it at the start.
        Package whole;
        QVERIFY(whole.read(zip));
        QVERIFY(!whole.isLazy());
        QVERIFY(whole.inflatedBytes() >= hugeSize);
    }

    // The package is the one read() gives: its names, content types, relationships, and the bytes of every part.
    void aLazyPackageIsTheSameAsOneReadWhole()
    {
        compareModes(zipOf(smallEntries()), "small");
        compareModes(zipOf(smallEntries(), false), "small, stored");
        Entries odd = smallEntries();
        odd.append({QStringLiteral("xl/a%20b.xml"), "<a/>"});
        odd.append({QStringLiteral("xl/../escape.xml"), "<bad/>"});
        odd.append({QStringLiteral("XL/Worksheets/SHEET1.xml"), "<dup/>"});
        odd.append({QStringLiteral("xl\\win.xml"), "<win/>"});
        odd.append({QStringLiteral("/xl/slash.xml"), "<slash/>"});
        odd.append({QStringLiteral("xl/folder/"), QByteArray()});
        odd.append({QStringLiteral("xl/empty.xml"), QByteArray()});
        compareModes(zipOf(odd), "odd names");
        Package lazy;
        QVERIFY(lazy.readLazy(zipOf(odd)));
        QVERIFY(lazy.skippedCount() >= 2);
        QVERIFY(lazy.hasPart(QStringLiteral("/xl/a b.xml")));
        QVERIFY(lazy.hasPart(QStringLiteral("/xl/win.xml")));
        QVERIFY(!lazy.hasPart(QStringLiteral("/escape.xml")));
        QCOMPARE(lazy.part(QStringLiteral("/xl/worksheets/sheet1.xml")), QByteArray("<worksheet>one</worksheet>"));
        QCOMPARE(lazy.relationships(QStringLiteral("/xl/workbook.xml")).size(), 2);
        QCOMPARE(lazy.relationship(QStringLiteral("/xl/workbook.xml"), QStringLiteral("rId2")).target, QStringLiteral("/xl/worksheets/sheet2.xml"));
    }

    // What read() refuses, readLazy() refuses, with the same message: a total too large (from the directory, before any part is inflated), too many parts,
    // no content types, damaged XML in the package's own parts, and a file that isn't a ZIP.
    void aLazyPackageIsRefusedAsOneReadWholeIs()
    {
        compareModes(QByteArray("this is not a zip file at all, not even close"), "not a zip");
        Entries noTypes = smallEntries();
        noTypes.remove(0);
        compareModes(zipOf(noTypes), "no content types");
        Entries badTypes = smallEntries();
        badTypes[0].second = "<Types><unclosed>";
        compareModes(zipOf(badTypes), "bad content types");
        Entries badRels = smallEntries();
        badRels[3].second = "<Relationships><Relationship Id=\"x\"";
        compareModes(zipOf(badRels), "bad relationships");
        Entries doctype = smallEntries();
        doctype[1].second = "<!DOCTYPE x [<!ENTITY a \"b\">]>" + relsXml(QString());
        compareModes(zipOf(doctype), "DOCTYPE in relationships");
        Entries many = smallEntries();
        for (int i = 0; i < 10001; ++i) many.append({QStringLiteral("xl/p/%1.xml").arg(i), "<a/>"});
        compareModes(zipOf(many, false), "too many parts");
        Package p;
        QString why;
        QVERIFY(!p.readLazy(zipOf(many, false), &why));
        QVERIFY2(why.contains(QStringLiteral("10,000")), qPrintable(why));
        QVERIFY(p.partNames().isEmpty());
        // Too much to inflate: refused from the sizes in the directory, whatever the caller allows.
        Entries bulky = smallEntries();
        bulky.append({QStringLiteral("xl/worksheets/big.xml"), QByteArray(5 << 20, 'b')});
        const QByteArray bulkyZip = zipOf(bulky);
        Package eager, lazy;
        QString eagerWhy, lazyWhy;
        QVERIFY(!eager.read(bulkyZip, &eagerWhy, 1 << 20));
        QVERIFY(!lazy.readLazy(bulkyZip, &lazyWhy, 1 << 20));
        QCOMPARE(lazyWhy, eagerWhy);
        QVERIFY(lazy.partNames().isEmpty());
        QVERIFY(lazy.readLazy(bulkyZip, &lazyWhy, 6 << 20));
        // A directory that claims 1 GB for each of three parts is refused from what it says, before anything is inflated.
        Entries lying = smallEntries();
        for (int i = 1; i <= 3; ++i) lying.append({QStringLiteral("xl/big%1.bin").arg(i), "x"});
        for (int i = 4; i <= 5; ++i) lying.append({QStringLiteral("xl/big%1.bin").arg(i), "x"});
        QByteArray lyingZip = zipOf(lying, false);
        patchEntry(lyingZip, QStringLiteral("xl/big"), 24, 0x40000000u);
        // Five parts of 1 GB are more than a lazy package may come to (4 GB) as they are more than one read whole may (2 GB), each by its own limit.
        Package eagerLying, lazyLying;
        QString eagerLyingWhy, lazyLyingWhy;
        QVERIFY(!eagerLying.read(lyingZip, &eagerLyingWhy));
        QVERIFY(!lazyLying.readLazy(lyingZip, &lazyLyingWhy));
        QVERIFY2(lazyLyingWhy.contains(QStringLiteral("4")), qPrintable(lazyLyingWhy));
        QVERIFY(lazyLying.partNames().isEmpty());
    }

    // What read() finds wrong inside a part (a CRC, a size that isn't the directory's) is found when the part is asked for: an empty part, a failed stream.
    void aDamagedPartIsFoundWhenItIsAskedFor()
    {
        Entries e = smallEntries();
        QByteArray zip = zipOf(e);
        QByteArray badCrc = zip;
        patchEntry(badCrc, QStringLiteral("xl/worksheets/sheet2.xml"), 16, 0x12345678u);
        Package whole;
        QVERIFY(!whole.read(badCrc));
        Package lazy;
        QVERIFY(lazy.readLazy(badCrc));
        QCOMPARE(lazy.part(QStringLiteral("/xl/worksheets/sheet1.xml")), QByteArray("<worksheet>one</worksheet>"));
        QVERIFY(lazy.part(QStringLiteral("/xl/worksheets/sheet2.xml")).isEmpty());
        auto device = lazy.openPart(QStringLiteral("/xl/worksheets/sheet2.xml"));
        QVERIFY(device);
        device->readAll();
        auto *entry = dynamic_cast<jp::ZipEntryDevice *>(device.get());
        QVERIFY(entry && entry->failed());
        // A directory that understates the size: the part runs past it.
        QByteArray small = zip;
        patchEntry(small, QStringLiteral("xl/worksheets/sheet2.xml"), 24, 5);
        QVERIFY(!Package().read(small));
        Package lazy2;
        QVERIFY(lazy2.readLazy(small));
        QVERIFY(lazy2.part(QStringLiteral("/xl/worksheets/sheet2.xml")).isEmpty());
        auto device2 = lazy2.openPart(QStringLiteral("/xl/worksheets/sheet2.xml"));
        QVERIFY(device2);
        device2->readAll();
        auto *entry2 = dynamic_cast<jp::ZipEntryDevice *>(device2.get());
        QVERIFY(entry2 && entry2->failed());
        // One that overstates it: the stream ends before it.
        QByteArray big = zip;
        patchEntry(big, QStringLiteral("xl/worksheets/sheet2.xml"), 24, 500);
        Package lazy3;
        QVERIFY(lazy3.readLazy(big));
        QVERIFY(lazy3.part(QStringLiteral("/xl/worksheets/sheet2.xml")).isEmpty());
        // A damaged relationship part is not left for later: it is part of what the package is made of.
        QByteArray badRels = zip;
        patchEntry(badRels, QStringLiteral("xl/_rels/workbook.xml.rels"), 16, 1);
        QVERIFY(!Package().readLazy(badRels));
        // Nothing but what was asked for was damaged.
        QCOMPARE(lazy.relationships(QStringLiteral("/xl/workbook.xml")).size(), 2);
    }

    // A part as a stream is inflated in the pieces it is read in: after a first small read, little has been inflated.
    void aPartAsAStreamIsInflatedAsItIsRead()
    {
        const qint64 size = qint64(16) << 20;
        Entries e = smallEntries();
        QByteArray content(int(size), Qt::Uninitialized);
        for (qsizetype i = 0; i < content.size(); ++i) content[i] = char('a' + (i * 7 + i / 1000) % 26);
        e.append({QStringLiteral("xl/worksheets/big.xml"), content});
        Package p;
        QVERIFY(p.readLazy(zipOf(e)));
        const qint64 before = p.inflatedBytes();
        auto device = p.openPart(QStringLiteral("/xl/worksheets/big.xml"));
        QVERIFY(device);
        QByteArray piece = device->read(4096);
        QCOMPARE(piece, content.left(4096));
        QVERIFY2(p.inflatedBytes() - before <= 4096 + 32768, qPrintable(QString::number(p.inflatedBytes() - before)));
        // Read to the end, in pieces of 100,000 bytes: every byte, in order, the total counted, and nothing held but the piece.
        qint64 got = piece.size();
        qint64 largestAhead = 0;
        for (;;) {
            const QByteArray more = device->read(100000);
            if (more.isEmpty()) break;
            QVERIFY(content.mid(int(got), more.size()) == more);
            got += more.size();
            largestAhead = std::max(largestAhead, p.inflatedBytes() - before - got);
        }
        QCOMPARE(got, size);
        QCOMPARE(p.inflatedBytes() - before, size);
        QVERIFY2(largestAhead <= 16384, qPrintable(QString::number(largestAhead)));
        auto *entry = dynamic_cast<jp::ZipEntryDevice *>(device.get());
        QVERIFY(entry && !entry->failed());
        // No such part: nothing.
        QVERIFY(!p.openPart(QStringLiteral("/xl/nothing.xml")));
        QCOMPARE(p.partSize(QStringLiteral("/xl/nothing.xml")), qint64(-1));
        // Whole, a package gives its part as a stream too.
        Package whole;
        QVERIFY(whole.read(zipOf(smallEntries())));
        auto one = whole.openPart(QStringLiteral("/xl/worksheets/sheet1.xml"));
        QVERIFY(one);
        QCOMPARE(one->readAll(), QByteArray("<worksheet>one</worksheet>"));
    }

    // However many times parts are asked for, what is inflated over the package's life comes to no more than the package was allowed at the start: no list of
    // parts, and no part asked for again and again, can multiply the work.
    void theTotalInflatedOverAPackagesLifeIsBounded()
    {
        const int size = 1 << 20;
        Entries e = smallEntries();
        e.append({QStringLiteral("xl/worksheets/big.xml"), QByteArray(size, 'a')});
        const QByteArray zip = zipOf(e);
        Package p;
        QString why;
        // The parts come to a little over 1 MB; 2.5 MB is allowed.
        QVERIFY2(p.readLazy(zip, &why, qint64(size) * 5 / 2), qPrintable(why));
        QCOMPARE(p.part(QStringLiteral("/xl/worksheets/big.xml")).size(), size);
        QCOMPARE(p.part(QStringLiteral("/xl/worksheets/big.xml")).size(), size);
        // A third time would take it past, so it is refused, as a stream too.
        QVERIFY(p.part(QStringLiteral("/xl/worksheets/big.xml")).isEmpty());
        QVERIFY(p.inflatedBytes() <= qint64(size) * 5 / 2);
        auto device = p.openPart(QStringLiteral("/xl/worksheets/big.xml"));
        QVERIFY(device);
        QVERIFY(device->readAll().size() < size);
        auto *entry = dynamic_cast<jp::ZipEntryDevice *>(device.get());
        QVERIFY(entry && entry->failed());
        QVERIFY(entry->failure() == jp::ZipEntryDevice::Failure::OverBudget);
        QVERIFY(p.inflatedBytes() <= qint64(size) * 5 / 2);
        // What still fits is still given.
        QCOMPARE(p.part(QStringLiteral("/xl/worksheets/sheet1.xml")), QByteArray("<worksheet>one</worksheet>"));
    }

    // Each entry's local header starts with its signature; one that doesn't is no entry.
    void aLocalHeaderThatIsNoLocalHeaderIsRefused()
    {
        QByteArray zip = zipOf(smallEntries());
        QVERIFY(Package().readLazy(zip));
        qsizetype wrecked = 0;
        for (qsizetype at = zip.indexOf(QByteArray("PK\x01\x02", 4)); at >= 0; at = zip.indexOf(QByteArray("PK\x01\x02", 4), at + 4)) {
            const quint16 nameLength = qFromLittleEndian<quint16>(zip.constData() + at + 28);
            if (QString::fromUtf8(zip.mid(at + 46, nameLength)) != QLatin1String("xl/worksheets/sheet2.xml")) continue;
            const quint32 local = qFromLittleEndian<quint32>(zip.constData() + at + 42);
            QCOMPARE(zip.mid(int(local), 4), QByteArray("PK\x03\x04", 4));
            zip.replace(int(local), 4, "XXXX");
            ++wrecked;
        }
        QCOMPARE(wrecked, qsizetype(1));
        Package p;
        QVERIFY(!p.readLazy(zip));
        QVERIFY(p.partNames().isEmpty());
    }

    // A stored entry has its CRC checked when it is asked for, as a deflated one does.
    void aStoredEntrysChecksumIsChecked()
    {
        QByteArray zip = zipOf(smallEntries(), false);
        QVERIFY(Package().read(zip));
        patchEntry(zip, QStringLiteral("xl/worksheets/sheet2.xml"), 16, 0x12345678u);
        Package p;
        QVERIFY(p.readLazy(zip));
        QCOMPARE(p.part(QStringLiteral("/xl/worksheets/sheet1.xml")), QByteArray("<worksheet>one</worksheet>"));
        QVERIFY(p.part(QStringLiteral("/xl/worksheets/sheet2.xml")).isEmpty());
        auto device = p.openPart(QStringLiteral("/xl/worksheets/sheet2.xml"));
        QVERIFY(device);
        device->readAll();
        auto *entry = dynamic_cast<jp::ZipEntryDevice *>(device.get());
        QVERIFY(entry && entry->failed());
        QVERIFY(entry->failure() == jp::ZipEntryDevice::Failure::Damaged);
    }

    // The most one part may inflate to, and all together, are at least ten times the most that real workbooks have (a sheet of 151 MB, and 302 MB in all).
    void theSizesAreTenTimesTheLargestFound()
    {
        QVERIFY(jp::kMaxZipEntryBytes >= 10 * qint64(151222800));
        QVERIFY(Package::kLazyMaxInflated >= 10 * qint64(302000000));
    }

    // A part past the most it may inflate to is said to be too large, not damaged.
    void aPartTooLargeIsSaidToBeTooLarge()
    {
        QByteArray zip = zipOf(smallEntries());
        patchEntry(zip, QStringLiteral("xl/worksheets/sheet2.xml"), 24, 0xF0000000u);
        Package p;
        QString why;
        QVERIFY2(p.readLazy(zip, &why, qint64(8) << 30), qPrintable(why));
        QVERIFY(p.part(QStringLiteral("/xl/worksheets/sheet2.xml")).isEmpty());
        auto device = p.openPart(QStringLiteral("/xl/worksheets/sheet2.xml"));
        QVERIFY(device);
        device->readAll();
        auto *entry = dynamic_cast<jp::ZipEntryDevice *>(device.get());
        QVERIFY(entry && entry->failed());
        QVERIFY(entry->failure() == jp::ZipEntryDevice::Failure::TooLarge);
        // Nothing was inflated for it.
        QVERIFY(p.inflatedBytes() < 4096);
    }

    // The XML a lazy package reads whole (its content types and relationships) passes the guard a sheet's does: UTF-16, and elements with more attributes than
    // any file has, are refused without the parser having them.
    void aPackagesOwnXmlIsGuardedAsASheetsIs()
    {
        QString attributes;
        for (int i = 0; i < 20000; ++i) attributes += QStringLiteral(" a\u0122%1=\"v\u0122\"").arg(i);
        QStringEncoder little(QStringConverter::Utf16LE, QStringConverter::Flag::Default);
        const QByteArray typesUtf16 = QByteArray("\xff\xfe", 2) + QByteArray(little(QString::fromUtf8(typesXml()).replace(QLatin1String("<Default Extension=\"xml\""), "<Default" + attributes + " Extension=\"xml\"")));
        Entries e = smallEntries();
        e[0].second = typesUtf16;
        Package p;
        QString why;
        QVERIFY(!p.readLazy(zipOf(e), &why));
        QVERIFY2(why.contains(QStringLiteral("Content_Types")), qPrintable(why));
        QVERIFY(p.partNames().isEmpty());
        // The same crowded in UTF-8, in the content types and in a relationship part.
        QString crowd;
        for (int i = 0; i < 2000; ++i) crowd += QStringLiteral(" a%1=\"v\"").arg(i);
        Entries f = smallEntries();
        f[0].second = QString::fromUtf8(typesXml()).replace(QLatin1String("<Default Extension=\"xml\""), "<Default" + crowd + " Extension=\"xml\"").toUtf8();
        QVERIFY(!Package().readLazy(zipOf(f), &why));
        Entries g = smallEntries();
        g[1].second = relsXml("<Relationship Id=\"rId1\" Type=\"" + kOfficeDoc + "\" Target=\"xl/workbook.xml\"" + crowd + "/>");
        QVERIFY(!Package().readLazy(zipOf(g), &why));
        QVERIFY2(why.contains(QStringLiteral("relationships")), qPrintable(why));
        // What a package read whole accepts of ordinary XML, a lazy one does.
        QVERIFY(Package().readLazy(zipOf(smallEntries())));
    }

    // A failed lazy read leaves the package empty, as a failed read does; and the same package is read again.
    void aFailedLazyReadLeavesNothing()
    {
        Package p;
        QVERIFY(p.readLazy(zipOf(smallEntries())));
        QVERIFY(!p.partNames().isEmpty());
        QVERIFY(!p.readLazy(QByteArray("nope")));
        QVERIFY(p.partNames().isEmpty());
        QVERIFY(!p.isLazy());
        QCOMPARE(p.inflatedBytes(), qint64(0));
    }
};

JP_TEST_CLASS(SheetPackageTests)
#include "tst_sheet_package.moc"
