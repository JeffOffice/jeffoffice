// Tests of libs/office: packages, the XML they hold, and the writer.
#include "officehelpers.h"
#include "../timing.h"

#include <QFile>
#include <QTemporaryDir>

namespace {
const QString kStrictRelBase = QStringLiteral("http://purl.oclc.org/ooxml/officeDocument/relationships/");
const QString kRelsType = QStringLiteral("application/vnd.openxmlformats-package.relationships+xml");

using Entries = QVector<QPair<QString, QByteArray>>;

QByteArray zipOf(const Entries &entries, bool compress = true)
{
    jp::ZipWriter z;
    for (const auto &e : entries) z.add(e.first, e.second, compress);
    return z.finish();
}

QByteArray contentTypesXml(const QString &body)
{
    return ("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
            "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">" + body + "</Types>").toUtf8();
}

QByteArray relsXml(const QString &body)
{
    return ("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
            "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">" + body + "</Relationships>").toUtf8();
}

QByteArray typesOfMini()
{
    return contentTypesXml(QStringLiteral(
        "<Default Extension=\"rels\" ContentType=\"%1\"/>"
        "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
        "<Default Extension=\"png\" ContentType=\"image/png\"/>"
        "<Override PartName=\"/ppt/presentation.xml\" ContentType=\"%2\"/>"
        "<Override PartName=\"/ppt/slides/slide1.xml\" ContentType=\"%3\"/>").arg(kRelsType, kPresentationType, kSlideType));
}

// A small valid package: a presentation with one slide, a layout, a theme,
// and a picture, linked with relative, "..", and absolute targets.
Entries miniEntries()
{
    return {
        {QStringLiteral("[Content_Types].xml"), typesOfMini()},
        {QStringLiteral("_rels/.rels"), relsXml(rel("rId1", kRelBase + "officeDocument", "ppt/presentation.xml")
                                                + rel("rId2", kRelBase + "hyperlink", "https://example.com/", true))},
        {QStringLiteral("ppt/presentation.xml"), "<p:presentation/>"},
        {QStringLiteral("ppt/_rels/presentation.xml.rels"), relsXml(rel("rId1", kRelBase + "slide", "slides/slide1.xml")
                                                                    + rel("rId2", kRelBase + "theme", "theme/theme1.xml")
                                                                    + rel("rId3", kRelBase + "hyperlink", "https://example.com/a?b=1", true))},
        {QStringLiteral("ppt/slides/slide1.xml"), "<p:sld/>"},
        {QStringLiteral("ppt/slides/_rels/slide1.xml.rels"), relsXml(rel("rId1", kRelBase + "slideLayout", "../slideLayouts/slideLayout1.xml")
                                                                     + rel("rId2", kRelBase + "image", "../media/image1.png")
                                                                     + rel("rId3", kRelBase + "theme", "/ppt/theme/theme1.xml"))},
        {QStringLiteral("ppt/slideLayouts/slideLayout1.xml"), "<p:sldLayout/>"},
        {QStringLiteral("ppt/media/image1.png"), QByteArray::fromHex("89504e470d0a1a0a")},
        {QStringLiteral("ppt/theme/theme1.xml"), "<a:theme/>"},
    };
}

// The XML with a DOCTYPE after its declaration, and nothing else wrong.
QByteArray withDoctype(QByteArray xml, const QByteArray &root, const QByteArray &subset = " [<!ENTITY e \"x\">]")
{
    xml.insert(xml.indexOf("?>") + 2, "\n<!DOCTYPE " + root + subset + ">");
    return xml;
}

Entries replaced(Entries entries, const QString &name, const QByteArray &data)
{
    for (auto &e : entries)
        if (e.first == name) e.second = data;
    return entries;
}

Entries without(Entries entries, const QString &name)
{
    for (int i = 0; i < entries.size(); ++i)
        if (entries[i].first == name) {
            entries.removeAt(i);
            break;
        }
    return entries;
}

// The minimal package grown to `parts` parts in all.
Entries withPartCount(int parts)
{
    Entries e = miniEntries();
    for (int i = 5; i < parts; ++i) e.append({QStringLiteral("ppt/extra/part%1.xml").arg(i), "<a/>"});
    return e;
}

// Three entries that claim 1 GB each when inflated: more than a package may
// hold. The numbers are in the ZIP directory, so none of it is really there.
QByteArray declaredHuge()
{
    Entries e = miniEntries();
    for (int i = 1; i <= 3; ++i) e.append({QStringLiteral("ppt/big%1.bin").arg(i), "x"});
    QByteArray zip = zipOf(e, false);
    for (qsizetype at = zip.indexOf(QByteArray("PK\x01\x02", 4)); at >= 0; at = zip.indexOf(QByteArray("PK\x01\x02", 4), at + 4)) {
        const quint16 nameLength = qFromLittleEndian<quint16>(zip.constData() + at + 28);
        if (QString::fromUtf8(zip.mid(at + 46, nameLength)).startsWith(QLatin1String("ppt/big")))
            qToLittleEndian<quint32>(0x40000000u, zip.data() + at + 24);
    }
    return zip;
}

} // namespace

class OfficePackageTests : public QObject {
    Q_OBJECT


private Q_SLOTS:
    // ---- packages

    // A file larger than the most the apps open is refused before anything is read (each is held whole at once); one of the most is read.
    void aFileLargerThanTheMostIsRefusedBeforeItIsRead()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("big.docx"));
        {
            QFile f(path);
            QVERIFY(f.open(QIODevice::WriteOnly));
            QCOMPARE(f.write(QByteArray(100, 'x')), qint64(100));
        }
        QByteArray bytes("left over");
        QString why;
        QVERIFY(!readWholeFile(path, &bytes, &why, 99));
        QVERIFY(bytes.isEmpty());
        QVERIFY(!why.isEmpty());
        QVERIFY(readWholeFile(path, &bytes, &why, 100));
        QCOMPARE(bytes.size(), qsizetype(100));
        QVERIFY(!readWholeFile(dir.filePath(QStringLiteral("none.docx")), &bytes, &why));
        QVERIFY(bytes.isEmpty());
        QCOMPARE(kMaxOpenFileBytes, qint64(1) << 30);
    }

    void packageReadsMinimal()
    {
        Package p;
        QString err;
        QVERIFY2(p.read(zipOf(miniEntries()), &err), qPrintable(err));
        QCOMPARE(p.partNames(), QStringList({QStringLiteral("/ppt/media/image1.png"), QStringLiteral("/ppt/presentation.xml"),
                                             QStringLiteral("/ppt/slideLayouts/slideLayout1.xml"), QStringLiteral("/ppt/slides/slide1.xml"),
                                             QStringLiteral("/ppt/theme/theme1.xml")}));
        QCOMPARE(p.part(QStringLiteral("/ppt/presentation.xml")), QByteArray("<p:presentation/>"));
        QCOMPARE(p.part(QStringLiteral("/ppt/media/image1.png")), QByteArray::fromHex("89504e470d0a1a0a"));
        QVERIFY(p.part(QStringLiteral("/ppt/nothing.xml")).isEmpty());
        QVERIFY(p.hasPart(QStringLiteral("/ppt/theme/theme1.xml")));
        QVERIFY(!p.hasPart(QStringLiteral("/ppt/nothing.xml")));
        // The bookkeeping parts aren't parts.
        QVERIFY(!p.hasPart(QStringLiteral("/[Content_Types].xml")));
        QVERIFY(!p.hasPart(QStringLiteral("/_rels/.rels")));
        QCOMPARE(p.mainPart(), QStringLiteral("/ppt/presentation.xml"));
    }

    void packageResolvesRelationships()
    {
        Package p;
        QVERIFY(p.read(zipOf(miniEntries())));
        const QVector<Relationship> own = p.relationships(QStringLiteral("/"));
        QCOMPARE(own.size(), 2);
        QCOMPARE(own[0].id, QStringLiteral("rId1"));
        QCOMPARE(own[0].type, kRelBase + "officeDocument");
        QCOMPARE(own[0].target, QStringLiteral("/ppt/presentation.xml"));
        QVERIFY(!own[0].external);
        QVERIFY(own[1].external);
        QCOMPARE(own[1].target, QStringLiteral("https://example.com/"));

        const QVector<Relationship> pres = p.relationships(QStringLiteral("/ppt/presentation.xml"));
        QCOMPARE(pres.size(), 3);
        QCOMPARE(pres[0].target, QStringLiteral("/ppt/slides/slide1.xml"));
        QCOMPARE(pres[1].target, QStringLiteral("/ppt/theme/theme1.xml"));
        QVERIFY(pres[2].external);
        QCOMPARE(pres[2].target, QStringLiteral("https://example.com/a?b=1"));   // as written, not resolved

        const QVector<Relationship> slide = p.relationships(QStringLiteral("/ppt/slides/slide1.xml"));
        QCOMPARE(slide.size(), 3);
        QCOMPARE(slide[0].target, QStringLiteral("/ppt/slideLayouts/slideLayout1.xml"));   // "../"
        QCOMPARE(slide[1].target, QStringLiteral("/ppt/media/image1.png"));
        QCOMPARE(slide[2].target, QStringLiteral("/ppt/theme/theme1.xml"));                // absolute

        QVERIFY(p.relationships(QStringLiteral("/ppt/theme/theme1.xml")).isEmpty());
        QVERIFY(p.relationships(QStringLiteral("/ppt/nothing.xml")).isEmpty());
    }

    // A part may have tens of thousands of links; each is found by its Id at
    // once, not by going through the others.
    void packageFindsEachRelationshipAtOnce()
    {
        PackageWriter w;
        w.addPart(QStringLiteral("/ppt/slides/slide1.xml"), "<x/>", QStringLiteral("application/xml"));
        QStringList ids;
        for (int i = 0; i < 40000; ++i)
            ids << w.addRelationship(QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("http://x/image"), QStringLiteral("../media/image%1.png").arg(i));
        Package pkg;
        QString err;
        QVERIFY2(pkg.read(w.finish(), &err), qPrintable(err));
        const qint64 ms = bestNanoseconds([&] {
            for (int i = 0; i < ids.size(); ++i)
                QCOMPARE(pkg.relationship(QStringLiteral("/ppt/slides/slide1.xml"), ids[i]).target, QStringLiteral("/ppt/media/image%1.png").arg(i));
        }) / 1000000;
        QVERIFY2(ms < 2000, qPrintable(QString::number(ms)));
        QVERIFY(pkg.relationship(QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("rIdNone")).id.isEmpty());
        QVERIFY(pkg.relationship(QStringLiteral("/nowhere.xml"), ids[0]).id.isEmpty());
    }

    // Writing links and parts takes time in proportion to how many there are: four times as many take about four times as long, not sixteen
    // (compared with each other, not against a clock).
    void packageWriterIsLinearInLinksAndParts()
    {
        auto timeIt = [](int links, int parts, bool ownIds) {
            std::unique_ptr<PackageWriter> w;
            return bestNanoseconds([&] {
                for (int i = 0; i < parts; ++i) w->addPart(QStringLiteral("/word/media/image%1.png").arg(i), "x", QStringLiteral("image/png"));
                for (int i = 0; i < links; ++i) {
                    if (ownIds) w->addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdX%1").arg(i), QStringLiteral("http://x/hyperlink"), QStringLiteral("https://example.com/%1").arg(i), true);
                    else w->addRelationship(QStringLiteral("/word/document.xml"), QStringLiteral("http://x/hyperlink"), QStringLiteral("https://example.com/%1").arg(i), true);
                }
            }, 5, [&] { w = std::make_unique<PackageWriter>(); }) / 1e6;
        };
        for (bool ownIds : {false, true}) {
            const double small = std::max(timeIt(5000, 0, ownIds), 1.0), large = timeIt(20000, 0, ownIds);
            QVERIFY2(large < 9 * small, qPrintable(QStringLiteral("links, own ids %1: 5,000 took %2 ms, 20,000 took %3 ms").arg(ownIds).arg(small).arg(large)));
        }
        const double smallParts = std::max(timeIt(0, 1500, false), 1.0), largeParts = timeIt(0, 6000, false);
        QVERIFY2(largeParts < 9 * smallParts, qPrintable(QStringLiteral("parts: 1,500 took %1 ms, 6,000 took %2 ms").arg(smallParts).arg(largeParts)));
    }

    void packageFindsRelationships()
    {
        Package p;
        QVERIFY(p.read(zipOf(miniEntries())));
        const Relationship r = p.relationship(QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("rId2"));
        QCOMPARE(r.id, QStringLiteral("rId2"));
        QCOMPARE(r.target, QStringLiteral("/ppt/media/image1.png"));
        QVERIFY(p.relationship(QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("rId9")).id.isEmpty());
        QVERIFY(p.relationship(QStringLiteral("/ppt/nothing.xml"), QStringLiteral("rId1")).id.isEmpty());

        const auto themes = p.relationshipsOfType(QStringLiteral("/ppt/presentation.xml"), QStringLiteral("/theme"));
        QCOMPARE(themes.size(), 1);
        QCOMPARE(themes[0].target, QStringLiteral("/ppt/theme/theme1.xml"));
        // The end of the type URI, from its last slash: "/slide" is not "/slideLayout".
        QCOMPARE(p.relationshipsOfType(QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("/slide")).size(), 0);
        QCOMPARE(p.relationshipsOfType(QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("/slideLayout")).size(), 1);
        QCOMPARE(p.relationshipsOfType(QStringLiteral("/ppt/presentation.xml"), QStringLiteral("/hyperlink")).size(), 1);
    }

    void packageStrictMainPart()
    {
        // The strict standard's relationship types match by their ending too.
        Entries e = replaced(miniEntries(), QStringLiteral("_rels/.rels"),
                             relsXml(rel("rId7", kStrictRelBase + "officeDocument", "ppt/presentation.xml")));
        Package p;
        QVERIFY(p.read(zipOf(e)));
        QCOMPARE(p.mainPart(), QStringLiteral("/ppt/presentation.xml"));
        QCOMPARE(p.relationshipsOfType(QStringLiteral("/"), QStringLiteral("/officeDocument")).size(), 1);

        // No such relationship, no main part.
        Package none;
        QVERIFY(none.read(zipOf(replaced(miniEntries(), QStringLiteral("_rels/.rels"), relsXml(QString())))));
        QVERIFY(none.mainPart().isEmpty());
    }

    void packageContentTypes()
    {
        Package p;
        QVERIFY(p.read(zipOf(miniEntries())));
        // An Override wins over the extension's Default.
        QCOMPARE(p.contentType(QStringLiteral("/ppt/presentation.xml")), kPresentationType);
        QCOMPARE(p.contentType(QStringLiteral("/ppt/slides/slide1.xml")), kSlideType);
        // Else the Default for the extension.
        QCOMPARE(p.contentType(QStringLiteral("/ppt/theme/theme1.xml")), QStringLiteral("application/xml"));
        QCOMPARE(p.contentType(QStringLiteral("/ppt/media/image1.png")), QStringLiteral("image/png"));
        QCOMPARE(p.contentType(QStringLiteral("/ppt/media/IMAGE2.PNG")), QStringLiteral("image/png"));   // extensions, too, ignore case
        // Neither: nothing.
        QVERIFY(p.contentType(QStringLiteral("/ppt/media/clip.mp4")).isEmpty());
        QVERIFY(p.contentType(QStringLiteral("/ppt/README")).isEmpty());
    }

    void packagePartNamesIgnoreCase()
    {
        Package p;
        QVERIFY(p.read(zipOf(miniEntries())));
        QVERIFY(p.hasPart(QStringLiteral("/PPT/Presentation.XML")));
        QCOMPARE(p.part(QStringLiteral("/PPT/Presentation.XML")), QByteArray("<p:presentation/>"));
        QCOMPARE(p.contentType(QStringLiteral("/PPT/PRESENTATION.xml")), kPresentationType);
        QCOMPARE(p.relationships(QStringLiteral("/PPT/SLIDES/slide1.XML")).size(), 3);
        QCOMPARE(p.relationship(QStringLiteral("/Ppt/Slides/Slide1.xml"), QStringLiteral("rId1")).target, QStringLiteral("/ppt/slideLayouts/slideLayout1.xml"));
        // The names it lists are as the package spells them.
        QVERIFY(p.partNames().contains(QStringLiteral("/ppt/presentation.xml")));
    }

    void packageDecodesPercentEscapes()
    {
        // A name with a space, as an escape in the ZIP and as a plain space.
        for (const QString &item : {QStringLiteral("ppt/media/my%20image.png"), QStringLiteral("ppt/media/my image.png")}) {
            Entries e = miniEntries();
            e.append({item, "png"});
            e = replaced(e, QStringLiteral("ppt/slides/_rels/slide1.xml.rels"),
                         relsXml(rel("rId1", kRelBase + "image", "../media/my%20image.png")));
            Package p;
            QString err;
            QVERIFY2(p.read(zipOf(e), &err), qPrintable(err));
            QVERIFY(p.hasPart(QStringLiteral("/ppt/media/my image.png")));
            QCOMPARE(p.relationships(QStringLiteral("/ppt/slides/slide1.xml")).value(0).target, QStringLiteral("/ppt/media/my image.png"));
        }
    }

    void packageSkipsFolderEntries()
    {
        Entries e = miniEntries();
        e.append({QStringLiteral("ppt/"), QByteArray()});
        e.append({QStringLiteral("ppt/media/"), QByteArray()});
        Package p;
        QString err;
        QVERIFY2(p.read(zipOf(e), &err), qPrintable(err));
        QCOMPARE(p.partNames().size(), 5);
    }

    void packageReadFile()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("mini.pkg"));
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(zipOf(miniEntries()));
        f.close();
        Package p;
        QString err;
        QVERIFY2(p.readFile(path, &err), qPrintable(err));
        QCOMPARE(p.mainPart(), QStringLiteral("/ppt/presentation.xml"));

        Package missing;
        QVERIFY(!missing.readFile(dir.filePath(QStringLiteral("nothing.pkg")), &err));
        QVERIFY(!err.isEmpty());
    }

    void packageAcceptsTenThousandParts()
    {
        Package p;
        QString err;
        QVERIFY2(p.read(zipOf(withPartCount(10000), false), &err), qPrintable(err));
        QCOMPARE(p.partNames().size(), 10000);
    }

    void packageRejectsHostile_data()
    {
        QTest::addColumn<QByteArray>("zip");
        const QByteArray mini = zipOf(miniEntries());

        QTest::newRow("no content types") << zipOf(without(miniEntries(), QStringLiteral("[Content_Types].xml")));
        QTest::newRow("content types not XML") << zipOf(replaced(miniEntries(), QStringLiteral("[Content_Types].xml"), "this is not xml"));
        QTest::newRow("content types cut short") << zipOf(replaced(miniEntries(), QStringLiteral("[Content_Types].xml"), typesOfMini().left(120)));
        QTest::newRow("content types with a DOCTYPE")
            << zipOf(replaced(miniEntries(), QStringLiteral("[Content_Types].xml"), withDoctype(typesOfMini(), "Types")));
        QTest::newRow("content types with a bare DOCTYPE")
            << zipOf(replaced(miniEntries(), QStringLiteral("[Content_Types].xml"), withDoctype(typesOfMini(), "Types", QByteArray())));
        QTest::newRow("content types with another root")
            << zipOf(replaced(miniEntries(), QStringLiteral("[Content_Types].xml"), "<Other/>"));
        QTest::newRow("package relationships not XML") << zipOf(replaced(miniEntries(), QStringLiteral("_rels/.rels"), "{\"json\": true}"));
        QTest::newRow("part relationships not XML")
            << zipOf(replaced(miniEntries(), QStringLiteral("ppt/_rels/presentation.xml.rels"), "PK not xml"));
        QTest::newRow("relationships with a DOCTYPE")
            << zipOf(replaced(miniEntries(), QStringLiteral("_rels/.rels"), withDoctype(relsXml(QString()), "Relationships")));

        QTest::newRow("10,001 parts") << zipOf(withPartCount(10001), false);
        QTest::newRow("more than 2 GB inflated") << declaredHuge();
        QTest::newRow("truncated") << mini.left(mini.size() * 2 / 3);
        QTest::newRow("directory cut off") << mini.left(mini.size() - 30);
        QTest::newRow("not a ZIP") << QByteArray("This is plain text, longer than a ZIP's smallest possible size, and no package at all.");
        QTest::newRow("empty") << QByteArray();
    }

    void packageRejectsHostile()
    {
        QFETCH(QByteArray, zip);
        Package p;
        QString err;
        QVERIFY(!p.read(zip, &err));
        QVERIFY(!err.isEmpty());
        // Nothing of it is kept.
        QVERIFY(p.partNames().isEmpty());
        QVERIFY(p.mainPart().isEmpty());
        QVERIFY(p.relationships(QStringLiteral("/")).isEmpty());
    }

    void packageToleratesOddRelationships()
    {
        const QString odd = rel("rId1", kRelBase + "hyperlink", "https://example.com/no-mode")   // an address with no TargetMode
            + QStringLiteral("<Relationship Type=\"%1\" Target=\"ppt/presentation.xml\"/>").arg(kRelBase + "slide")   // no Id
            + QStringLiteral("<Relationship Id=\"rId5\" Type=\"%1\"/>").arg(kRelBase + "slide")   // no Target
            + rel("rId6", kRelBase + "slide", "ppt/presentation.xml") + rel("rId6", kRelBase + "slide", "ppt/other.xml")   // a repeated Id
            + QStringLiteral("<Other xmlns=\"urn:other\"/>");
        Package p;
        QString err;
        QVERIFY2(p.read(zipOf(replaced(miniEntries(), QStringLiteral("_rels/.rels"), relsXml(odd))), &err), qPrintable(err));
        const QVector<Relationship> own = p.relationships(QStringLiteral("/"));
        QCOMPARE(own.size(), 2);
        QVERIFY(own[0].external);
        QCOMPARE(own[0].target, QStringLiteral("https://example.com/no-mode"));
        QCOMPARE(own[1].id, QStringLiteral("rId6"));
        QCOMPARE(own[1].target, QStringLiteral("/ppt/presentation.xml"));   // the first rId6
    }

    void packageSurvivesRandomDamage()
    {
        // Entries are stored, so the damage reaches the XML parsers (and not
        // just the checksums).
        const QByteArray good = zipOf(miniEntries(), false);
        QRandomGenerator gen(2024);
        int accepted = 0;
        for (int i = 0; i < 500; ++i) {
            QByteArray bad = good;
            for (int k = 0, n = 1 + int(gen.bounded(3)); k < n; ++k) bad[int(gen.bounded(quint32(bad.size())))] = char(gen.bounded(256u));
            Package p;
            if (!p.read(bad)) continue;
            ++accepted;
            for (const QString &name : p.partNames()) {
                p.part(name);
                p.contentType(name);
                p.relationships(name);
            }
            p.relationships(QStringLiteral("/"));
            p.mainPart();
        }
        QVERIFY(accepted > 0);   // damage inside a part's data is no damage to the package
    }

    void packageReadReplacesOldContents()
    {
        Package p;
        QVERIFY(p.read(zipOf(miniEntries())));
        QVERIFY(!p.read(QByteArray("not a package at all, not at all, not at all.")));
        QVERIFY(p.partNames().isEmpty());
        QVERIFY(p.read(zipOf(miniEntries())));
        QCOMPARE(p.partNames().size(), 5);
    }

    void resolvePartNames_data()
    {
        QTest::addColumn<QString>("source");
        QTest::addColumn<QString>("target");
        QTest::addColumn<QString>("expected");
        QTest::newRow("beside") << "/ppt/presentation.xml" << "slides/slide1.xml" << "/ppt/slides/slide1.xml";
        QTest::newRow("same folder") << "/ppt/slides/slide1.xml" << "slide2.xml" << "/ppt/slides/slide2.xml";
        QTest::newRow("dot") << "/ppt/slides/slide1.xml" << "./slide2.xml" << "/ppt/slides/slide2.xml";
        QTest::newRow("up one") << "/ppt/slides/slide1.xml" << "../media/a.png" << "/ppt/media/a.png";
        QTest::newRow("up two") << "/ppt/slides/slide1.xml" << "../../docProps/core.xml" << "/docProps/core.xml";
        QTest::newRow("up and down") << "/ppt/slides/slide1.xml" << "../slides/../theme/t.xml" << "/ppt/theme/t.xml";
        QTest::newRow("from the package") << "/" << "ppt/presentation.xml" << "/ppt/presentation.xml";
        QTest::newRow("absolute") << "/ppt/slides/slide1.xml" << "/ppt/theme/theme1.xml" << "/ppt/theme/theme1.xml";
        QTest::newRow("absolute with dot") << "/ppt/slides/slide1.xml" << "/ppt/./theme/../theme/theme1.xml" << "/ppt/theme/theme1.xml";
        QTest::newRow("escape") << "/ppt/my%20file.xml" << "media/a%20b.png" << "/ppt/media/a b.png";
        QTest::newRow("escaped non-ASCII") << "/ppt/p.xml" << "caf%C3%A9.png" << QString::fromUtf8("/ppt/caf\xC3\xA9.png");
        QTest::newRow("bad escape stays") << "/ppt/p.xml" << "100%.png" << "/ppt/100%.png";
        QTest::newRow("fragment dropped") << "/ppt/presentation.xml" << "slides/slide2.xml#top" << "/ppt/slides/slide2.xml";
        QTest::newRow("query dropped") << "/ppt/presentation.xml" << "slides/slide2.xml?x=1" << "/ppt/slides/slide2.xml";
        QTest::newRow("climbs above the root") << "/ppt/slides/slide1.xml" << "../../../evil.xml" << "";
        QTest::newRow("climbs from the package") << "/" << "../evil.xml" << "";
        QTest::newRow("absolute climbing") << "/ppt/p.xml" << "/ppt/../../evil.xml" << "";
        QTest::newRow("backslash") << "/ppt/p.xml" << "a\\b.xml" << "";
        QTest::newRow("empty segment") << "/ppt/p.xml" << "a//b.xml" << "";
        QTest::newRow("ends with a slash") << "/ppt/p.xml" << "slides/" << "";
        QTest::newRow("encoded ..") << "/ppt/p.xml" << "%2e%2e/evil.xml" << "";
        QTest::newRow("encoded .") << "/ppt/p.xml" << "a/%2e/b.xml" << "";
        QTest::newRow("encoded slash") << "/ppt/p.xml" << "a%2Fb.xml" << "";
        QTest::newRow("encoded backslash") << "/ppt/p.xml" << "a%5Cb.xml" << "";
        QTest::newRow("encoded control") << "/ppt/p.xml" << "a%01b.xml" << "";
        QTest::newRow("empty target") << "/ppt/p.xml" << "" << "";
    }

    void resolvePartNames()
    {
        QFETCH(QString, source);
        QFETCH(QString, target);
        QFETCH(QString, expected);
        QCOMPARE(resolvePartName(source, target), expected);
    }

    // ---- XML helpers

    void xmlParsesWithNamespaces()
    {
        const QByteArray xml =
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
            "<a:txBody xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\"\n"
            "          xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" r:id=\"rId3\" val=\"v\">\n"
            "  <a:r><a:t> </a:t></a:r>\n"
            "</a:txBody>";
        QDomDocument doc;
        QString err;
        QVERIFY2(parseXml(xml, &doc, &err), qPrintable(err));
        const QDomElement root = doc.documentElement();
        QCOMPARE(root.localName(), QStringLiteral("txBody"));
        QCOMPARE(root.namespaceURI(), QString(ns::A));
        // A run of one space is text, and stays.
        QCOMPARE(root.firstChildElement().firstChildElement().text(), QStringLiteral(" "));
    }

    void xmlRefusesWhatIsNotSafe_data()
    {
        QTest::addColumn<QByteArray>("xml");
        QTest::newRow("empty") << QByteArray();
        QTest::newRow("not XML") << QByteArray("plain text");
        QTest::newRow("cut short") << QByteArray("<a:x xmlns:a=\"urn:x\"><a:y>");
        QTest::newRow("two roots") << QByteArray("<a/><b/>");
        QTest::newRow("DOCTYPE") << QByteArray("<?xml version=\"1.0\"?>\n<!DOCTYPE x [<!ENTITY e \"y\">]>\n<x>&e;</x>");
        QTest::newRow("bare DOCTYPE") << QByteArray("<!DOCTYPE x>\n<x/>");
        QTest::newRow("external entity")
            << QByteArray("<?xml version=\"1.0\"?>\n<!DOCTYPE x [<!ENTITY e SYSTEM \"file:///etc/hostname\">]>\n<x>&e;</x>");
        QTest::newRow("entity with no DOCTYPE") << QByteArray("<x>&e;</x>");
        QTest::newRow("one level too deep") << QByteArray("<a>").repeated(kMaxXmlDepth + 1) + QByteArray("</a>").repeated(kMaxXmlDepth + 1);
    }

    void xmlRefusesWhatIsNotSafe()
    {
        QFETCH(QByteArray, xml);
        QDomDocument doc;
        QString err;
        QVERIFY(!parseXml(xml, &doc, &err));
        QVERIFY(!err.isEmpty());
    }

    // A few bytes repeated make a tree that takes hundreds of times their size, so what a part may hold is
    // limited. The XML is built here, in memory.
    void xmlRefusesWhatWouldFillMemory_data()
    {
        QTest::addColumn<QByteArray>("xml");
        const QByteArray head("<a:x xmlns:a=\"urn:x\">"), tail("</a:x>");
        QTest::newRow("too many elements") << head + QByteArray("<a:b/>").repeated(kMaxXmlNodes) + tail;
        QTest::newRow("too many attributes")
            << head + QByteArray("<a:b p0=\"1\" p1=\"1\" p2=\"1\" p3=\"1\" p4=\"1\" p5=\"1\" p6=\"1\" p7=\"1\" p8=\"1\" p9=\"1\"/>").repeated(kMaxXmlNodes / 10) + tail;
        QTest::newRow("too many runs of text") << head + QByteArray("<a:b>t</a:b>").repeated(kMaxXmlNodes / 2) + tail;
        QTest::newRow("too many comments") << head + QByteArray("<!---->").repeated(kMaxXmlNodes) + tail;
        QTest::newRow("too many bytes") << head + QByteArray(kMaxXmlBytes, 'x') + tail;
    }

    void xmlRefusesWhatWouldFillMemory()
    {
        QFETCH(QByteArray, xml);
        QDomDocument doc;
        QString err;
        const qint64 ms = bestNanoseconds([&] { QVERIFY(!parseXml(xml, &doc, &err)); }, 3, [&] {
            doc = QDomDocument();
            err.clear();
        }) / 1000000;
        QVERIFY(!err.isEmpty());
        QVERIFY(doc.documentElement().isNull());
        // Counted on the way in, not found out by building the tree.
        QVERIFY2(ms < 10000, "the XML was refused too slowly");
    }

    void xmlRefusesAnElementWithManyAttributes()
    {
        // The parsers' time grows with the square of the attributes on one element: 100,000 of them, in 1.4 MB,
        // keep them busy for minutes.
        for (int count : {kMaxAttributesPerElement + 1, 100000}) {
            QByteArray xml("<a:x xmlns:a=\"urn:x\"");
            for (int i = 0; i < count; ++i) xml += " p" + QByteArray::number(i) + "=\"1\"";
            xml += "/>";
            QDomDocument doc;
            QString err;
            const qint64 ms = bestNanoseconds([&] { QVERIFY2(!parseXml(xml, &doc, &err), qPrintable(QString::number(count))); }, 3, [&] {
                doc = QDomDocument();
                err.clear();
            }) / 1000000;
            QVERIFY(!err.isEmpty());
            QVERIFY2(ms < 5000, "the XML was refused too slowly");
        }
    }

    void xmlAcceptsMoreThanARealFileHolds()
    {
        // About three times the most any real part holds (54,000 nodes), in the shapes real parts have.
        QByteArray xml("<a:x xmlns:a=\"urn:x\">");
        xml += QByteArray("<a:b p=\"1\" q=\"2\"/>").repeated(40000) + QByteArray("<a:c>text</a:c>").repeated(20000);
        xml += QByteArray("<!-- = \" ' < -->").repeated(1000) + "</a:x>";
        QDomDocument doc;
        QString err;
        const qint64 ms = bestNanoseconds([&] { QVERIFY2(parseXml(xml, &doc, &err), qPrintable(err)); }, 3, [&] {
            doc = QDomDocument();
            err.clear();
        }) / 1000000;
        QCOMPARE(doc.documentElement().childNodes().size(), 61000);
        QVERIFY2(ms < 5000, "the XML took too long to parse");

        // The busiest element a part may have (each "=" in its tag is one attribute or namespace declaration).
        QByteArray busy("<a:x xmlns:a=\"urn:x\"");
        for (int i = 1; i < kMaxAttributesPerElement; ++i) busy += " p" + QByteArray::number(i) + "=\"1\"";
        QVERIFY2(parseXml(busy + "/>", &doc, &err), qPrintable(err));
        QCOMPARE(doc.documentElement().attributes().size(), kMaxAttributesPerElement - 1);
    }

    void xmlRefusesManyAttributesInUtf16()
    {
        // The same count is made for XML written in UTF-16, in either byte order.
        for (QStringConverter::Encoding encoding : {QStringConverter::Utf16LE, QStringConverter::Utf16BE}) {
            QString text = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-16\"?>\n<a:x xmlns:a=\"urn:x\"");
            for (int i = 0; i < 100000; ++i) text += QStringLiteral(" p%1=\"1\"").arg(i);
            text += QStringLiteral("/>");
            QStringEncoder encode(encoding, QStringConverter::Flag::WriteBom);
            const QByteArray bytes = encode(text);
            QDomDocument doc;
            QString err;
            const qint64 ms = bestNanoseconds([&] { QVERIFY(!parseXml(bytes, &doc, &err)); }, 3, [&] {
                doc = QDomDocument();
                err.clear();
            }) / 1000000;
            QVERIFY(!err.isEmpty());
            QVERIFY2(ms < 5000, "the XML was refused too slowly");

            // And a small document in UTF-16 is read as before.
            QStringEncoder small(encoding, QStringConverter::Flag::WriteBom);
            QVERIFY2(parseXml(small(QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-16\"?><a:x xmlns:a=\"urn:x\" p=\"1\">\u00e9</a:x>")), &doc, &err), qPrintable(err));
            QCOMPARE(doc.documentElement().text(), QStringLiteral("\u00e9"));
        }
    }

    void xmlCountsAttributesOnlyInTags()
    {
        // What looks like attributes in text, comments, CDATA, processing instructions, and quoted values is not
        // one, however many there are.
        const QByteArray equals(5000, '=');
        const QByteArray xml = "<a:x xmlns:a=\"urn:x\" note=\"" + equals + "\" other='" + equals + "'><!-- " + equals + " \"' -->"
                               "<![CDATA[" + equals + "]]><?pi " + equals + "?>" + equals + "<a:b t=\"a>b\"/> <a:c u='\"=\"=\"'/></a:x>";
        QDomDocument doc;
        QString err;
        QVERIFY2(parseXml(xml, &doc, &err), qPrintable(err));
        QCOMPARE(doc.documentElement().attribute(QStringLiteral("note")).size(), 5000);
    }

    // Some zip tools write every name with backslashes, or with a leading slash; they are the same names.
    void packageReadsNamesWrittenWithBackslashesOrALeadingSlash()
    {
        Package plain;
        QVERIFY(plain.read(zipOf(miniEntries())));
        for (const QString &style : {QStringLiteral("slash"), QStringLiteral("backslash"), QStringLiteral("leading backslash")}) {
            Entries e = miniEntries();
            for (auto &entry : e) {
                if (style == QLatin1String("slash")) entry.first.prepend(QLatin1Char('/'));
                else entry.first.replace(QLatin1Char('/'), QLatin1Char('\\'));
                if (style == QLatin1String("leading backslash")) entry.first.prepend(QLatin1Char('\\'));
            }
            Package p;
            QString err;
            QVERIFY2(p.read(zipOf(e), &err), qPrintable(style + QLatin1Char(' ') + err));
            QCOMPARE(p.partNames(), plain.partNames());
            QCOMPARE(p.mainPart(), QStringLiteral("/ppt/presentation.xml"));
            QCOMPARE(p.relationships(QStringLiteral("/ppt/slides/slide1.xml")).size(), 3);
            QCOMPARE(p.contentType(QStringLiteral("/ppt/presentation.xml")), kPresentationType);
            QCOMPARE(p.skippedCount(), 0);
        }
        Package odf;
        QString err;
        QVERIFY2(odf.readOpenDocument(zipOf({{QStringLiteral("\\mimetype"), "m"}, {QStringLiteral("/content.xml"), "c"}, {QStringLiteral("Pictures\\a.png"), "p"}}), &err), qPrintable(err));
        QCOMPARE(odf.partNames(), QStringList({QStringLiteral("/Pictures/a.png"), QStringLiteral("/content.xml"), QStringLiteral("/mimetype")}));
    }

    // One part with a name nothing could have is left out; it doesn't refuse the whole file.
    void packageSkipsPartsWithUnusableNames_data()
    {
        QTest::addColumn<QString>("name");
        QTest::newRow("..") << QStringLiteral("../evil.xml");
        QTest::newRow("climbing out") << QStringLiteral("ppt/../../evil.xml");
        QTest::newRow(".. inside") << QStringLiteral("ppt/../evil.xml");
        QTest::newRow("backslashes climbing out") << QStringLiteral("ppt\\..\\..\\evil.xml");
        QTest::newRow("encoded ..") << QStringLiteral("ppt/%2e%2e/evil.xml");
        QTest::newRow("encoded slash") << QStringLiteral("ppt/a%2fb.xml");
        QTest::newRow("empty segment") << QStringLiteral("ppt//evil.xml");
        QTest::newRow("two leading slashes") << QStringLiteral("//ppt/evil.xml");
        QTest::newRow("dot segment") << QStringLiteral("./ppt/evil.xml");
        QTest::newRow("control character") << QStringLiteral("ppt/ev") + QChar(1) + QStringLiteral("il.xml");
        QTest::newRow("delete character") << QStringLiteral("ppt/ev") + QChar(0x7f) + QStringLiteral("il.xml");
    }

    void packageSkipsPartsWithUnusableNames()
    {
        QFETCH(QString, name);
        Package p;
        QString err;
        Entries e = miniEntries();
        e.insert(3, qMakePair(name, QByteArray("x")));
        QVERIFY2(p.read(zipOf(e), &err), qPrintable(err));
        QCOMPARE(p.partNames().size(), 5);
        QCOMPARE(p.mainPart(), QStringLiteral("/ppt/presentation.xml"));
        QCOMPARE(p.part(QStringLiteral("/ppt/slides/slide1.xml")), QByteArray("<p:sld/>"));
        QCOMPARE(p.skippedCount(), 1);

        const Entries odf = {{QStringLiteral("mimetype"), "m"}, {name, "x"}, {QStringLiteral("content.xml"), "<content/>"}, {QStringLiteral("Pictures/a.png"), "p"}};
        QVERIFY2(p.readOpenDocument(zipOf(odf), &err), qPrintable(err));
        QCOMPARE(p.partNames(), QStringList({QStringLiteral("/Pictures/a.png"), QStringLiteral("/content.xml"), QStringLiteral("/mimetype")}));
        QCOMPARE(p.skippedCount(), 1);
        // What a later read finds is that file's, with nothing left from this one.
        QVERIFY(p.readOpenDocument(zipOf({{QStringLiteral("content.xml"), "c"}}), &err));
        QCOMPARE(p.skippedCount(), 0);
    }

    // Two names that differ only in case would be one part; the first in the file is kept, the other left out.
    void packageKeepsTheFirstOfTwoNamesDifferingInCase()
    {
        Package p;
        QString err;
        Entries e = miniEntries();
        e.prepend({QStringLiteral("PPT/Presentation.xml"), "<upper/>"});
        QVERIFY2(p.read(zipOf(e), &err), qPrintable(err));
        QCOMPARE(p.part(QStringLiteral("/ppt/presentation.xml")), QByteArray("<upper/>"));
        QCOMPARE(p.skippedCount(), 1);

        e = miniEntries();
        e.append({QStringLiteral("PPT/Presentation.xml"), "<upper/>"});
        QVERIFY2(p.read(zipOf(e), &err), qPrintable(err));
        QCOMPARE(p.part(QStringLiteral("/ppt/presentation.xml")), QByteArray("<p:presentation/>"));
        QCOMPARE(p.skippedCount(), 1);

        QVERIFY2(p.readOpenDocument(zipOf({{QStringLiteral("Pictures/b.png"), "kept"}, {QStringLiteral("content.xml"), "<c/>"}, {QStringLiteral("Pictures/B.PNG"), "left out"}}), &err), qPrintable(err));
        QCOMPARE(p.part(QStringLiteral("/Pictures/b.png")), QByteArray("kept"));
        QCOMPARE(p.partNames().size(), 2);
        QCOMPARE(p.skippedCount(), 1);
    }

    // A link to nowhere is left out; the file's other links stay.
    void packageDropsOnlyTheUnusableRelationships()
    {
        const QString good = rel("rId1", kRelBase + "officeDocument", "ppt/presentation.xml");
        const QString links = good
            + rel("rId2", kRelBase + "hyperlink", "C:\\Users\\x\\doc.docx")   // a path on someone's disk, no TargetMode
            + rel("rId3", kRelBase + "image", "../../../../etc/passwd")
            + rel("rId4", kRelBase + "image", "/ppt/../../evil.xml")
            + rel("rId5", kRelBase + "image", "ppt\\presentation.xml")
            + rel("rId6", kRelBase + "image", "%2e%2e/evil.xml")
            + rel("rId7", kRelBase + "hyperlink", "https://example.com/", true)
            + rel("rId8", kRelBase + "image", "ppt//presentation.xml");
        Entries e = replaced(miniEntries(), QStringLiteral("_rels/.rels"), relsXml(links));
        e = replaced(e, QStringLiteral("ppt/slides/_rels/slide1.xml.rels"), relsXml(rel("rIdA", kRelBase + "image", "../../../x.png") + rel("rIdB", kRelBase + "slideLayout", "../slideLayouts/slideLayout1.xml")));
        Package p;
        QString err;
        QVERIFY2(p.read(zipOf(e), &err), qPrintable(err));
        QCOMPARE(p.relationships(QStringLiteral("/")).size(), 2);
        QCOMPARE(p.relationships(QStringLiteral("/")).value(0).id, QStringLiteral("rId1"));
        QCOMPARE(p.relationships(QStringLiteral("/")).value(1).id, QStringLiteral("rId7"));
        QCOMPARE(p.mainPart(), QStringLiteral("/ppt/presentation.xml"));
        QCOMPARE(p.relationships(QStringLiteral("/ppt/slides/slide1.xml")).size(), 1);
        QCOMPARE(p.relationship(QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("rIdB")).target, QStringLiteral("/ppt/slideLayouts/slideLayout1.xml"));
        QCOMPARE(p.skippedCount(), 7);

        // An Id the left-out link had is free for the next one.
        e = replaced(miniEntries(), QStringLiteral("_rels/.rels"), relsXml(rel("rId1", kRelBase + "image", "../../x.png") + good));
        QVERIFY2(p.read(zipOf(e), &err), qPrintable(err));
        QCOMPARE(p.mainPart(), QStringLiteral("/ppt/presentation.xml"));
    }

    // The .ppt reader opens small packages that were inside its file, and gives a lower limit than a file on disk has.
    void packageReadsUnderALowerInflatedLimit()
    {
        Entries e = miniEntries();
        e.append({QStringLiteral("ppt/media/zeros.bin"), QByteArray(3 << 20, '\0')});
        qint64 total = 0;
        for (const auto &entry : e) total += entry.second.size();
        const QByteArray zip = zipOf(e);
        QVERIFY(zip.size() < 100000);

        Package p;
        QString err;
        QVERIFY2(p.read(zip, &err), qPrintable(err));   // the usual limit
        QCOMPARE(p.part(QStringLiteral("/ppt/media/zeros.bin")).size(), 3 << 20);
        QVERIFY2(p.read(zip, &err, total), qPrintable(err));   // exactly what it holds
        QVERIFY(!p.read(zip, &err, total - 1));
        QVERIFY(!err.isEmpty());
        QVERIFY(p.partNames().isEmpty());
        QVERIFY(!p.read(zip, &err, 1 << 20));
        const QString tooMuch = err;
        QVERIFY(p.mainPart().isEmpty());
        QVERIFY(p.read(zipOf(miniEntries()), &err, 4096));   // a small package under a small limit

        // Refused from the sizes in the directory, before anything is inflated: an entry whose data is damaged
        // (so inflating it would fail) is refused with the same message when the directory says it is too big.
        QByteArray damaged = zipOf({{QStringLiteral("a.bin"), QByteArray(3 << 20, '\0')}});
        damaged[35] = char(0xff);   // the first byte of the entry's data, after its 30-byte header and 5-byte name
        QVERIFY(!p.read(damaged, &err));
        QVERIFY(err != tooMuch);
        QVERIFY(!p.read(damaged, &err, 1 << 20));
        QCOMPARE(err, tooMuch);

        // A directory that says less than the data holds is no way round: that is damage, whatever the limit.
        QByteArray small = zipOf({{QStringLiteral("ppt/zeros.bin"), QByteArray(5 << 20, '\0')}}, true);
        const qsizetype entry = small.indexOf(QByteArray("PK\x01\x02", 4));
        qToLittleEndian<quint32>(1000u, small.data() + entry + 24);
        QVERIFY(!p.read(small, &err, 1 << 20));
        QVERIFY(!p.read(small, &err));
        QVERIFY(p.partNames().isEmpty());
    }

    void xmlAcceptsDeepButReasonableNesting()
    {
        QDomDocument doc;
        QString err;
        const qint64 ms = bestNanoseconds([&] {
            QVERIFY2(parseXml(QByteArray("<a>").repeated(kMaxXmlDepth) + QByteArray("</a>").repeated(kMaxXmlDepth), &doc, &err), qPrintable(err));
        }, 3, [&] {
            doc = QDomDocument();
            err.clear();
        }) / 1000000;
        QVERIFY2(ms < 5000, "deep XML took too long to parse");
    }

    void xmlNamespacesInBothForms()
    {
        QVERIFY(sameNamespace(ns::A, ns::A));
        QVERIFY(sameNamespace(ns::A, ns::AStrict));
        QVERIFY(sameNamespace(ns::PStrict, ns::P));
        QVERIFY(sameNamespace(ns::R, ns::RStrict));
        QVERIFY(!sameNamespace(ns::A, ns::P));
        QVERIFY(!sameNamespace(ns::AStrict, ns::PStrict));
        QVERIFY(!sameNamespace(ns::A, QString()));
        QVERIFY(sameNamespace(QString(), QString()));
        QVERIFY(!sameNamespace(ns::A, QStringLiteral("urn:other")));
        QVERIFY(sameNamespace(QStringLiteral("urn:other"), QStringLiteral("urn:other")));
    }

    void xmlFindsElements_data()
    {
        QTest::addColumn<QString>("a");
        QTest::addColumn<QString>("p");
        QTest::newRow("transitional") << QString(ns::A) << QString(ns::P);
        QTest::newRow("strict") << QString(ns::AStrict) << QString(ns::PStrict);
    }

    void xmlFindsElements()
    {
        QFETCH(QString, a);
        QFETCH(QString, p);
        const QByteArray xml = QStringLiteral(
            "<a:spPr xmlns:a=\"%1\" xmlns:p=\"%2\" xmlns:x=\"urn:other\">"
            "<a:ln w=\"1\"/><x:ln w=\"2\"/><p:ln w=\"3\"/><a:ln w=\"4\"/><a:fill/>"
            "</a:spPr>").arg(a, p).toUtf8();
        QDomDocument doc;
        QVERIFY(parseXml(xml, &doc));
        const QDomElement root = doc.documentElement();
        // Asked for in either form, only this namespace's elements are found.
        for (const QLatin1String &form : {ns::A, ns::AStrict}) {
            QCOMPARE(child(root, form, QStringLiteral("ln")).attribute(QStringLiteral("w")), QStringLiteral("1"));
            const QVector<QDomElement> lines = jp::office::children(root, form, QStringLiteral("ln"));
            QCOMPARE(lines.size(), 2);
            QCOMPARE(lines[1].attribute(QStringLiteral("w")), QStringLiteral("4"));
            QVERIFY(isElement(lines[0], form, QStringLiteral("ln")));
            QVERIFY(!isElement(lines[0], form, QStringLiteral("fill")));
            QVERIFY(!isElement(lines[0], ns::P, QStringLiteral("ln")));
        }
        QCOMPARE(child(root, ns::P, QStringLiteral("ln")).attribute(QStringLiteral("w")), QStringLiteral("3"));
        QCOMPARE(child(root, QStringLiteral("urn:other"), QStringLiteral("ln")).attribute(QStringLiteral("w")), QStringLiteral("2"));
        QVERIFY(child(root, ns::A, QStringLiteral("missing")).isNull());
        QVERIFY(jp::office::children(root, ns::A, QStringLiteral("missing")).isEmpty());
        QCOMPARE(jp::office::children(root).size(), 5);
        // Not found below the first level, and nothing from a null element.
        QVERIFY(child(root, ns::A, QStringLiteral("w")).isNull());
        QVERIFY(child(QDomElement(), ns::A, QStringLiteral("ln")).isNull());
        QVERIFY(jp::office::children(QDomElement()).isEmpty());
        QVERIFY(!isElement(QDomElement(), ns::A, QStringLiteral("ln")));
    }

    void xmlReadsAttributes_data()
    {
        QTest::addColumn<QString>("r");
        QTest::newRow("transitional") << QString(ns::R);
        QTest::newRow("strict") << QString(ns::RStrict);
    }

    void xmlReadsAttributes()
    {
        QFETCH(QString, r);
        const QByteArray xml = QStringLiteral(
            "<a:blip xmlns:a=\"%1\" xmlns:r=\"%2\" xmlns:x=\"urn:other\" r:embed=\"rId7\" x:embed=\"other\" val=\"1\" empty=\"\"/>")
            .arg(ns::A, r).toUtf8();
        QDomDocument doc;
        QVERIFY(parseXml(xml, &doc));
        const QDomElement e = doc.documentElement();
        QCOMPARE(attr(e, QStringLiteral("val")), QStringLiteral("1"));
        QCOMPARE(attr(e, QStringLiteral("none"), QStringLiteral("dflt")), QStringLiteral("dflt"));
        QCOMPARE(attr(e, QStringLiteral("empty"), QStringLiteral("dflt")), QString());   // present but empty
        QVERIFY(hasAttr(e, QStringLiteral("val")));
        QVERIFY(hasAttr(e, QStringLiteral("empty")));
        QVERIFY(!hasAttr(e, QStringLiteral("none")));
        // A namespaced attribute is no plain one, and the reverse.
        QVERIFY(!hasAttr(e, QStringLiteral("embed")));
        QCOMPARE(attr(e, QStringLiteral("embed")), QString());
        for (const QLatin1String &form : {ns::R, ns::RStrict}) {
            QCOMPARE(attrNs(e, form, QStringLiteral("embed")), QStringLiteral("rId7"));
            QCOMPARE(attrNs(e, form, QStringLiteral("none"), QStringLiteral("dflt")), QStringLiteral("dflt"));
        }
        QCOMPARE(attrNs(e, QStringLiteral("urn:other"), QStringLiteral("embed")), QStringLiteral("other"));
        QCOMPARE(attrNs(e, ns::R, QStringLiteral("val"), QStringLiteral("dflt")), QStringLiteral("dflt"));   // val has no namespace
        QCOMPARE(attr(QDomElement(), QStringLiteral("val"), QStringLiteral("dflt")), QStringLiteral("dflt"));
        QCOMPARE(attrNs(QDomElement(), ns::R, QStringLiteral("embed"), QStringLiteral("dflt")), QStringLiteral("dflt"));
    }

    void xmlConvertsEmuAndPoints()
    {
        QCOMPARE(EmuPerPoint, qint64(12700));
        QCOMPARE(emuToPoints(12700), 1.0);
        QCOMPARE(emuToPoints(127000), 10.0);
        QCOMPARE(emuToPoints(-6350), -0.5);
        QCOMPARE(emuToPoints(9144000), 720.0);   // a ten-inch slide
        QCOMPARE(pointsToEmu(10), qint64(127000));
        QCOMPARE(pointsToEmu(0.5), qint64(6350));
        QCOMPARE(pointsToEmu(-72), qint64(-914400));
        QCOMPARE(pointsToEmu(emuToPoints(4572000)), qint64(4572000));
    }

    void xmlParsesLengths_data()
    {
        QTest::addColumn<QString>("text");
        QTest::addColumn<double>("points");
        QTest::addColumn<bool>("ok");
        QTest::newRow("EMU") << "12700" << 1.0 << true;
        QTest::newRow("negative EMU") << "-25400" << -2.0 << true;
        QTest::newRow("zero") << "0" << 0.0 << true;
        QTest::newRow("spaces around") << " 127000 " << 10.0 << true;
        QTest::newRow("a fraction of an EMU") << "6350.5" << 6350.5 / 12700 << true;
        QTest::newRow("points") << "36pt" << 36.0 << true;
        QTest::newRow("inches") << "1in" << 72.0 << true;
        QTest::newRow("fractional inches") << "1.5in" << 108.0 << true;
        QTest::newRow("centimeters") << "2.54cm" << 72.0 << true;
        QTest::newRow("millimeters") << "25.4mm" << 72.0 << true;
        QTest::newRow("picas") << "1pc" << 12.0 << true;
        QTest::newRow("picas, the other way") << "2pi" << 24.0 << true;
        QTest::newRow("negative inches") << "-0.5in" << -36.0 << true;
        QTest::newRow("empty") << "" << 0.0 << false;
        QTest::newRow("word") << "wide" << 0.0 << false;
        QTest::newRow("unknown unit") << "5furlongs" << 0.0 << false;
        QTest::newRow("unit alone") << "pt" << 0.0 << false;
        QTest::newRow("two numbers") << "1 2" << 0.0 << false;
    }

    void xmlParsesLengths()
    {
        QFETCH(QString, text);
        QFETCH(double, points);
        QFETCH(bool, ok);
        bool gotOk = !ok;
        const double v = parseLength(text, -99, &gotOk);
        QCOMPARE(gotOk, ok);
        QCOMPARE(v, ok ? points : -99.0);
        QCOMPARE(parseLength(text, 7), ok ? points : 7.0);   // without `ok`, too
    }

    void xmlParsesPercentages_data()
    {
        QTest::addColumn<QString>("text");
        QTest::addColumn<double>("fraction");
        QTest::addColumn<bool>("ok");
        QTest::newRow("whole") << "100000" << 1.0 << true;
        QTest::newRow("half") << "50000" << 0.5 << true;
        QTest::newRow("over") << "250000" << 2.5 << true;
        QTest::newRow("negative") << "-25000" << -0.25 << true;
        QTest::newRow("zero") << "0" << 0.0 << true;
        QTest::newRow("strict, whole") << "100%" << 1.0 << true;
        QTest::newRow("strict, fraction") << "12.5%" << 0.125 << true;
        QTest::newRow("strict, negative") << "-50%" << -0.5 << true;
        QTest::newRow("spaces around") << " 50000 " << 0.5 << true;
        QTest::newRow("empty") << "" << 0.0 << false;
        QTest::newRow("word") << "half" << 0.0 << false;
        QTest::newRow("just a sign") << "%" << 0.0 << false;
    }

    void xmlParsesPercentages()
    {
        QFETCH(QString, text);
        QFETCH(double, fraction);
        QFETCH(bool, ok);
        bool gotOk = !ok;
        QCOMPARE(parsePercent(text, -99, &gotOk), ok ? fraction : -99.0);
        QCOMPARE(gotOk, ok);
    }

    void xmlParsesAngles()
    {
        bool ok = false;
        QCOMPARE(parseAngle(QStringLiteral("5400000"), 0, &ok), 90.0);
        QVERIFY(ok);
        QCOMPARE(parseAngle(QStringLiteral("21600000")), 360.0);
        QCOMPARE(parseAngle(QStringLiteral("-5400000")), -90.0);
        QCOMPARE(parseAngle(QStringLiteral("0")), 0.0);
        QCOMPARE(parseAngle(QStringLiteral("30000")), 0.5);
        QCOMPARE(parseAngle(QStringLiteral("x"), 45, &ok), 45.0);
        QVERIFY(!ok);
        QCOMPARE(parseAngle(QString(), 45), 45.0);
    }

    void xmlParsesBooleans()
    {
        bool ok = false;
        for (const char *t : {"1", "true", "on", "TRUE", "On"}) {
            ok = false;
            QVERIFY2(parseBool(QString::fromLatin1(t), false, &ok), t);
            QVERIFY2(ok, t);
        }
        for (const char *t : {"0", "false", "off", "False", "OFF"}) {
            ok = false;
            QVERIFY2(!parseBool(QString::fromLatin1(t), true, &ok), t);
            QVERIFY2(ok, t);
        }
        QVERIFY(parseBool(QString(), true, &ok));
        QVERIFY(!ok);
        QVERIFY(!parseBool(QStringLiteral("maybe"), false, &ok));
        QVERIFY(!ok);
        QVERIFY(parseBool(QStringLiteral("2"), true));
    }

    void xmlParsesIntegers()
    {
        bool ok = false;
        QCOMPARE(parseInt(QStringLiteral("12700"), 0, &ok), qint64(12700));
        QVERIFY(ok);
        QCOMPARE(parseInt(QStringLiteral("-3")), qint64(-3));
        QCOMPARE(parseInt(QStringLiteral("9144000000")), qint64(9144000000));   // past 32 bits
        QCOMPARE(parseInt(QStringLiteral("1.5"), 8, &ok), qint64(8));
        QVERIFY(!ok);
        QCOMPARE(parseInt(QString(), 8), qint64(8));
        QCOMPARE(parseInt(QStringLiteral("12x"), 8), qint64(8));
    }

    // A package of the open document kind has no content types or relationships: its parts are
    // read by name, with the same refusals as any package.
    void packageReadsAnOpenDocumentZip()
    {
        const QByteArray mime = "application/vnd.oasis.opendocument.presentation";
        const Entries entries = {{QStringLiteral("mimetype"), mime},
                                 {QStringLiteral("META-INF/manifest.xml"), "<manifest/>"},
                                 {QStringLiteral("content.xml"), "<content/>"},
                                 {QStringLiteral("Pictures/"), QByteArray()},
                                 {QStringLiteral("Pictures/10000 a.png"), "png"},
                                 {QStringLiteral("Object 1/content.xml"), "<chart/>"}};
        Package p;
        QString err;
        QVERIFY2(p.readOpenDocument(zipOf(entries, false), &err), qPrintable(err));
        QCOMPARE(p.part(QStringLiteral("/mimetype")), mime);
        QCOMPARE(p.part(QStringLiteral("/content.xml")), QByteArray("<content/>"));
        QCOMPARE(p.part(QStringLiteral("/pictures/10000 A.png")), QByteArray("png"));
        QVERIFY(p.hasPart(QStringLiteral("/Object 1/content.xml")));
        QVERIFY(p.hasPart(QStringLiteral("/META-INF/manifest.xml")));
        QCOMPARE(p.partNames().size(), 5);
        QVERIFY(p.mainPart().isEmpty());
        QVERIFY(p.relationships(QStringLiteral("/")).isEmpty());
        // The usual read still wants the content types an open document lacks.
        QVERIFY(!p.read(zipOf(entries, false)));
        QVERIFY(p.partNames().isEmpty());
        // A part with a hostile name is left out, and the rest is read.
        for (const QString &name : {QStringLiteral("../evil.xml"), QStringLiteral("Pictures/../../evil.png"),
                                    QStringLiteral("Pictures/%2e%2e/x.png"), QStringLiteral("CONTENT.xml")}) {
            Entries bad = entries;
            bad.append({name, "x"});
            QVERIFY2(p.readOpenDocument(zipOf(bad, false), &err), qPrintable(name));
            QCOMPARE(p.partNames().size(), 5);
            QCOMPARE(p.part(QStringLiteral("/content.xml")), QByteArray("<content/>"));
            QCOMPARE(p.skippedCount(), 1);
        }
        // Damage is refused, and nothing of the file is kept.
        err.clear();
        QVERIFY(!p.readOpenDocument(zipOf(entries, false).left(60), &err));
        QVERIFY(!err.isEmpty());
        QVERIFY(!p.readOpenDocument(declaredHuge(), &err));
        QVERIFY(!p.readOpenDocument(QByteArray(), &err));
        // A reader is told apart from a package that is no open document by what is in it, not by this.
        QVERIFY(p.readOpenDocument(zipOf({{QStringLiteral("a.txt"), "x"}}, false), &err));
    }

    void writerRoundTrip()
    {
        PackageWriter w;
        w.addDefault(QStringLiteral("png"), QStringLiteral("image/png"));
        w.addPart(QStringLiteral("/ppt/presentation.xml"), "<p:presentation/>", kPresentationType);
        w.addPart(QStringLiteral("/ppt/slides/slide1.xml"), "<p:sld/>", kSlideType);
        w.addPart(QStringLiteral("/ppt/theme/theme1.xml"), "<a:theme/>", QStringLiteral("application/vnd.openxmlformats-officedocument.theme+xml"));
        w.addPart(QStringLiteral("/ppt/media/image1.png"), QByteArray::fromHex("89504e470d0a1a0a"), QStringLiteral("image/png"));
        w.addPart(QStringLiteral("/ppt/data/notes.xml"), "<n/>", QStringLiteral("application/xml"));
        w.addPart(QStringLiteral("/ppt/media/my image.png"), "png2", QStringLiteral("image/png"));
        QCOMPARE(w.addRelationship(QStringLiteral("/"), kRelBase + "officeDocument", QStringLiteral("/ppt/presentation.xml")), QStringLiteral("rId1"));
        QCOMPARE(w.addRelationship(QStringLiteral("/"), kRelBase + "hyperlink", QStringLiteral("https://example.com/a?b=1&c=2"), true), QStringLiteral("rId2"));
        QCOMPARE(w.addRelationship(QStringLiteral("/ppt/presentation.xml"), kRelBase + "slide", QStringLiteral("/ppt/slides/slide1.xml")), QStringLiteral("rId1"));
        QCOMPARE(w.addRelationship(QStringLiteral("/ppt/presentation.xml"), kRelBase + "theme", QStringLiteral("/ppt/theme/theme1.xml")), QStringLiteral("rId2"));
        QCOMPARE(w.addRelationship(QStringLiteral("/ppt/slides/slide1.xml"), kRelBase + "image", QStringLiteral("/ppt/media/image1.png")), QStringLiteral("rId1"));
        QCOMPARE(w.addRelationship(QStringLiteral("/ppt/slides/slide1.xml"), kRelBase + "image", QStringLiteral("/ppt/media/my image.png")), QStringLiteral("rId2"));
        QCOMPARE(w.addRelationship(QStringLiteral("/ppt/slides/slide1.xml"), kRelBase + "notesSlide", QStringLiteral("/ppt/data/notes.xml")), QStringLiteral("rId3"));
        const QByteArray zip = w.finish();

        // [Content_Types].xml is the first entry in the file.
        QCOMPARE(zip.mid(30, 19), QByteArray("[Content_Types].xml"));

        Package p;
        QString err;
        QVERIFY2(p.read(zip, &err), qPrintable(err));
        QCOMPARE(p.partNames(), QStringList({QStringLiteral("/ppt/data/notes.xml"), QStringLiteral("/ppt/media/image1.png"),
                                             QStringLiteral("/ppt/media/my image.png"), QStringLiteral("/ppt/presentation.xml"),
                                             QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("/ppt/theme/theme1.xml")}));
        QCOMPARE(p.part(QStringLiteral("/ppt/slides/slide1.xml")), QByteArray("<p:sld/>"));
        QCOMPARE(p.part(QStringLiteral("/ppt/media/image1.png")), QByteArray::fromHex("89504e470d0a1a0a"));
        QCOMPARE(p.contentType(QStringLiteral("/ppt/presentation.xml")), kPresentationType);
        QCOMPARE(p.contentType(QStringLiteral("/ppt/media/image1.png")), QStringLiteral("image/png"));
        QCOMPARE(p.contentType(QStringLiteral("/ppt/media/my image.png")), QStringLiteral("image/png"));
        QCOMPARE(p.contentType(QStringLiteral("/ppt/data/notes.xml")), QStringLiteral("application/xml"));
        QCOMPARE(p.contentType(QStringLiteral("/_rels/.rels")), kRelsType);
        QCOMPARE(p.mainPart(), QStringLiteral("/ppt/presentation.xml"));

        const QVector<Relationship> own = p.relationships(QStringLiteral("/"));
        QCOMPARE(own.size(), 2);
        QVERIFY(own[1].external);
        QCOMPARE(own[1].target, QStringLiteral("https://example.com/a?b=1&c=2"));
        const QVector<Relationship> slide = p.relationships(QStringLiteral("/ppt/slides/slide1.xml"));
        QCOMPARE(slide.size(), 3);
        QCOMPARE(slide[0].id, QStringLiteral("rId1"));
        QCOMPARE(slide[0].type, kRelBase + "image");
        QCOMPARE(slide[0].target, QStringLiteral("/ppt/media/image1.png"));
        QCOMPARE(slide[1].target, QStringLiteral("/ppt/media/my image.png"));
        QCOMPARE(slide[2].target, QStringLiteral("/ppt/data/notes.xml"));
        QCOMPARE(p.relationships(QStringLiteral("/ppt/presentation.xml")).value(1).target, QStringLiteral("/ppt/theme/theme1.xml"));

        // What the file holds: targets relative to the source part's folder,
        // Overrides only where a Default doesn't fit.
        QMap<QString, QByteArray> items;
        QVERIFY(jp::readZip(zip, items));
        const QString presRels = QString::fromUtf8(items.value(QStringLiteral("ppt/_rels/presentation.xml.rels")));
        QVERIFY2(presRels.contains(QStringLiteral("Target=\"slides/slide1.xml\"")), qPrintable(presRels));
        QVERIFY(presRels.contains(QStringLiteral("Target=\"theme/theme1.xml\"")));
        const QString slideRels = QString::fromUtf8(items.value(QStringLiteral("ppt/slides/_rels/slide1.xml.rels")));
        QVERIFY2(slideRels.contains(QStringLiteral("Target=\"../media/image1.png\"")), qPrintable(slideRels));
        QVERIFY(slideRels.contains(QStringLiteral("Target=\"../media/my%20image.png\"")));
        QVERIFY(slideRels.contains(QStringLiteral("Target=\"../data/notes.xml\"")));
        const QString rootRels = QString::fromUtf8(items.value(QStringLiteral("_rels/.rels")));
        QVERIFY(rootRels.contains(QStringLiteral("Target=\"ppt/presentation.xml\"")));
        QVERIFY(rootRels.contains(QStringLiteral("TargetMode=\"External\"")));
        const QString types = QString::fromUtf8(items.value(QStringLiteral("[Content_Types].xml")));
        QVERIFY(types.contains(QStringLiteral("Override PartName=\"/ppt/presentation.xml\"")));
        QVERIFY2(!types.contains(QStringLiteral("/ppt/media/image1.png")), qPrintable(types));
        QVERIFY(!types.contains(QStringLiteral("/ppt/data/notes.xml")));
        QVERIFY(types.contains(QStringLiteral("Extension=\"rels\"")));
        QVERIFY(types.contains(QStringLiteral("Extension=\"png\"")));
    }

    void writerKeepsRelationshipsPerSource()
    {
        PackageWriter w;
        w.addPart(QStringLiteral("/a.xml"), "<a/>", QStringLiteral("application/xml"));
        QCOMPARE(w.addRelationship(QStringLiteral("/a.xml"), kRelBase + "x", QStringLiteral("/b.xml")), QStringLiteral("rId1"));
        QCOMPARE(w.addRelationship(QStringLiteral("/b.xml"), kRelBase + "x", QStringLiteral("/a.xml")), QStringLiteral("rId1"));
        QCOMPARE(w.addRelationship(QStringLiteral("/a.xml"), kRelBase + "x", QStringLiteral("/c.xml")), QStringLiteral("rId2"));
        // A package with no relationships of its own writes no /_rels/.rels.
        Package p;
        QVERIFY(p.read(w.finish()));
        QVERIFY(p.relationships(QStringLiteral("/")).isEmpty());
        QCOMPARE(p.relationships(QStringLiteral("/a.xml")).size(), 2);
        QCOMPARE(p.relationships(QStringLiteral("/b.xml")).value(0).target, QStringLiteral("/a.xml"));
    }

    void writerReplacesAPartAddedTwice()
    {
        PackageWriter w;
        w.addPart(QStringLiteral("/a.xml"), "first", QStringLiteral("text/x-first"));
        w.addPart(QStringLiteral("/a.xml"), "second", QStringLiteral("text/x-second"));
        Package p;
        QVERIFY(p.read(w.finish()));
        QCOMPARE(p.partNames().size(), 1);
        QCOMPARE(p.part(QStringLiteral("/a.xml")), QByteArray("second"));
        QCOMPARE(p.contentType(QStringLiteral("/a.xml")), QStringLiteral("text/x-second"));
    }

    void writerLeavesOutBadPartNames()
    {
        PackageWriter w;
        w.addPart(QStringLiteral("/ok.xml"), "<a/>", QStringLiteral("application/xml"));
        for (const QString &bad : {QStringLiteral("/ppt/../evil.xml"), QStringLiteral("ppt/relative.xml"), QStringLiteral("/a\\b.xml"),
                                   QStringLiteral("/a//b.xml"), QStringLiteral("/[Content_Types].xml"),
                                   QStringLiteral("/_rels/.rels")}) {
            QTest::ignoreMessage(QtWarningMsg, QRegularExpression(QStringLiteral("PackageWriter")));
            w.addPart(bad, "x", QStringLiteral("application/xml"));
        }
        Package p;
        QVERIFY(p.read(w.finish()));
        QCOMPARE(p.partNames(), QStringList({QStringLiteral("/ok.xml")}));
    }

    // A "%" (or a space, or a letter outside ASCII) is a character a part's name may have: the package writes it escaped, as the names of ZIP items,
    // Overrides, and targets are URIs, and reads it back as it was.
    void writerKeepsPartsWithEscapedCharactersInTheirNames()
    {
        PackageWriter w;
        const QStringList names = {QStringLiteral("/100%.xml"), QStringLiteral("/a b.png"), QStringLiteral("/\u00fc/\u00e9.bin"), QStringLiteral("/x%41.bin")};
        for (const QString &n : names) {
            w.addPart(n, "x", QStringLiteral("application/octet-stream"));
            w.addRelationship(QStringLiteral("/"), QStringLiteral("http://x/part"), n);
        }
        QMap<QString, QByteArray> items;
        const QByteArray zip = w.finish();
        QVERIFY(jp::readZip(zip, items));
        QVERIFY(items.contains(QStringLiteral("100%25.xml")) && items.contains(QStringLiteral("a%20b.png")) && items.contains(QStringLiteral("x%2541.bin")));
        Package p;
        QVERIFY(p.read(zip));
        QCOMPARE(p.skippedCount(), 0);
        for (const QString &n : names) {
            QVERIFY2(p.hasPart(n), qPrintable(n));
            QCOMPARE(p.contentType(n), QStringLiteral("application/octet-stream"));
        }
        QCOMPARE(p.relationships(QStringLiteral("/")).size(), 4);
        for (const Relationship &r : p.relationships(QStringLiteral("/"))) QVERIFY2(names.contains(r.target), qPrintable(r.target));
    }

    void writerDefaultsCanBeReplaced()
    {
        // A part whose type is the extension's Default needs no Override;
        // replacing the Default for "xml" changes what fits.
        PackageWriter w;
        w.addDefault(QStringLiteral("xml"), QStringLiteral("text/xml"));
        w.addPart(QStringLiteral("/a.xml"), "<a/>", QStringLiteral("text/xml"));
        w.addPart(QStringLiteral("/b.xml"), "<b/>", QStringLiteral("application/xml"));
        Package p;
        QVERIFY(p.read(w.finish()));
        QCOMPARE(p.contentType(QStringLiteral("/a.xml")), QStringLiteral("text/xml"));
        QCOMPARE(p.contentType(QStringLiteral("/b.xml")), QStringLiteral("application/xml"));
    }
};

JP_TEST_CLASS(OfficePackageTests)
#include "tst_office_package.moc"
