// Tests of libs/office: packages, the theme, and DrawingML colors. Every
// package they read is built in memory (never a real-world file).
#include "office/drawingml.h"
#include "office/drawingml_color.h"
#include "office/drawingml_geometry.h"
#include "office/package.h"
#include "office/theme.h"
#include "office/xml.h"
#include "io/zip.h"

#include <QFile>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QtEndian>
#include <QtTest>

using namespace jp::office;

namespace {

const QString kRelBase = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
const QString kStrictRelBase = QStringLiteral("http://purl.oclc.org/ooxml/officeDocument/relationships/");
const QString kPresentationType = QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml");
const QString kSlideType = QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slide+xml");
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

QString rel(const QString &id, const QString &type, const QString &target, bool external = false)
{
    return QStringLiteral("<Relationship Id=\"%1\" Type=\"%2\" Target=\"%3\"%4/>")
        .arg(id, type, target, external ? QStringLiteral(" TargetMode=\"External\"") : QString());
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


// A theme part written by hand, with the twelve colors (the first two as
// system colors, whose lastClr is the value) and a font scheme with a
// per-script font the struct has no place for.
QByteArray handWrittenTheme(const QString &a = QString(ns::A))
{
    return QStringLiteral(R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<a:theme xmlns:a="%1" name="Test Theme">
  <a:themeElements>
    <a:clrScheme name="Test Colors">
      <a:dk1><a:sysClr val="windowText" lastClr="101010"/></a:dk1>
      <a:lt1><a:sysClr val="window" lastClr="FEFEFE"/></a:lt1>
      <a:dk2><a:srgbClr val="44546A"/></a:dk2>
      <a:lt2><a:srgbClr val="E7E6E6"/></a:lt2>
      <a:accent1><a:srgbClr val="4472C4"/></a:accent1>
      <a:accent2><a:srgbClr val="ED7D31"/></a:accent2>
      <a:accent3><a:srgbClr val="A5A5A5"/></a:accent3>
      <a:accent4><a:srgbClr val="FFC000"/></a:accent4>
      <a:accent5><a:srgbClr val="5B9BD5"/></a:accent5>
      <a:accent6><a:srgbClr val="70AD47"/></a:accent6>
      <a:hlink><a:srgbClr val="0563C1"/></a:hlink>
      <a:folHlink><a:srgbClr val="954F72"/></a:folHlink>
    </a:clrScheme>
    <a:fontScheme name="Test Fonts">
      <a:majorFont>
        <a:latin typeface="Georgia" panose="02040502050405020303"/>
        <a:ea typeface="Noto Serif CJK JP"/>
        <a:cs typeface="Arial"/>
        <a:font script="Jpan" typeface="Yu Mincho"/>
      </a:majorFont>
      <a:minorFont>
        <a:latin typeface="Verdana"/>
        <a:ea typeface=""/>
        <a:cs typeface="Tahoma"/>
      </a:minorFont>
    </a:fontScheme>
    <a:fmtScheme name="Test Formats"><a:fillStyleLst><a:noFill/></a:fillStyleLst></a:fmtScheme>
  </a:themeElements>
  <a:objectDefaults/>
  <a:extraClrSchemeLst/>
</a:theme>)").arg(a).toUtf8();
}

const QColor kHandWrittenColors[Theme::ColorCount] = {
    QColor(0x10, 0x10, 0x10), QColor(0xFE, 0xFE, 0xFE), QColor(0x44, 0x54, 0x6A), QColor(0xE7, 0xE6, 0xE6),
    QColor(0x44, 0x72, 0xC4), QColor(0xED, 0x7D, 0x31), QColor(0xA5, 0xA5, 0xA5), QColor(0xFF, 0xC0, 0x00),
    QColor(0x5B, 0x9B, 0xD5), QColor(0x70, 0xAD, 0x47), QColor(0x05, 0x63, 0xC1), QColor(0x95, 0x4F, 0x72)};


// ---- colors

Theme colorTestTheme()
{
    Theme t;
    for (int i = 0; i < Theme::ColorCount; ++i) t.colors[i] = kHandWrittenColors[i];
    return t;
}

QString hexOf(const QColor &c)
{
    return c.isValid() ? c.name(QColor::HexRgb).mid(1).toUpper() : QStringLiteral("(invalid)");
}

// Whether each channel is within `tolerance` of the color written as hex.
bool nearTo(const QColor &c, const QString &hex, int tolerance)
{
    const QColor want(QLatin1Char('#') + hex);
    return c.isValid() && qAbs(c.red() - want.red()) <= tolerance && qAbs(c.green() - want.green()) <= tolerance
        && qAbs(c.blue() - want.blue()) <= tolerance;
}

// <a:name val="value"/> for each "name value" given.
QString transforms(const QStringList &list)
{
    QString out;
    for (const QString &t : list) {
        const QStringList parts = t.split(QLatin1Char(' '));
        out += parts.size() > 1 ? QStringLiteral("<a:%1 val=\"%2\"/>").arg(parts[0], parts[1]) : QStringLiteral("<a:%1/>").arg(parts[0]);
    }
    return out;
}

QString srgbClr(const QString &hex, const QStringList &list = {})
{
    return QStringLiteral("<a:srgbClr val=\"%1\">%2</a:srgbClr>").arg(hex, transforms(list));
}

QString schemeClr(const QString &val, const QStringList &list = {})
{
    return QStringLiteral("<a:schemeClr val=\"%1\">%2</a:schemeClr>").arg(val, transforms(list));
}

// The color in `<a:solidFill>` holding `inner`.
QColor colorOf(const QString &inner, const QColor &phClr = QColor(), const ColorMap &map = ColorMap(), const Theme &theme = colorTestTheme(),
               const QString &a = QString(ns::A))
{
    QDomDocument doc;
    const QByteArray xml = QStringLiteral("<a:solidFill xmlns:a=\"%1\">%2</a:solidFill>").arg(a, inner).toUtf8();
    if (!parseXml(xml, &doc)) return QColor();
    return readColor(doc.documentElement(), theme, map, phClr);
}

} // namespace

class FormatTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // ---- packages

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
        auto with = [](const QString &name) {
            Entries e = miniEntries();
            e.append({name, "x"});
            return zipOf(e);
        };
        const QByteArray mini = zipOf(miniEntries());

        QTest::newRow("part named ..") << with(QStringLiteral("../evil.xml"));
        QTest::newRow("part climbing out") << with(QStringLiteral("ppt/../../evil.xml"));
        QTest::newRow("part with .. inside") << with(QStringLiteral("ppt/../evil.xml"));
        QTest::newRow("part with an encoded ..") << with(QStringLiteral("ppt/%2e%2e/evil.xml"));
        QTest::newRow("part with an encoded slash") << with(QStringLiteral("ppt/a%2fb.xml"));
        QTest::newRow("part with a backslash") << with(QStringLiteral("ppt\\evil.xml"));
        QTest::newRow("part with an empty segment") << with(QStringLiteral("ppt//evil.xml"));
        QTest::newRow("part with a leading slash") << with(QStringLiteral("/ppt/evil.xml"));
        QTest::newRow("part with a control character") << with(QStringLiteral("ppt/ev") + QChar(1) + QStringLiteral("il.xml"));
        QTest::newRow("part with a delete character") << with(QStringLiteral("ppt/ev") + QChar(0x7f) + QStringLiteral("il.xml"));
        QTest::newRow("two parts differing in case") << with(QStringLiteral("PPT/Presentation.xml"));

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
        QTest::newRow("relationship climbing out")
            << zipOf(replaced(miniEntries(), QStringLiteral("_rels/.rels"), relsXml(rel("rId1", kRelBase + "officeDocument", "../../evil.xml"))));
        QTest::newRow("absolute relationship climbing out")
            << zipOf(replaced(miniEntries(), QStringLiteral("_rels/.rels"), relsXml(rel("rId1", kRelBase + "officeDocument", "/ppt/../../evil.xml"))));
        QTest::newRow("relationship with a backslash")
            << zipOf(replaced(miniEntries(), QStringLiteral("_rels/.rels"), relsXml(rel("rId1", kRelBase + "officeDocument", "ppt\\presentation.xml"))));
        QTest::newRow("relationship with an encoded ..")
            << zipOf(replaced(miniEntries(), QStringLiteral("_rels/.rels"), relsXml(rel("rId1", kRelBase + "officeDocument", "%2e%2e/evil.xml"))));

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
        QTest::newRow("1,001 levels deep") << QByteArray("<a>").repeated(1001) + QByteArray("</a>").repeated(1001);
    }

    void xmlRefusesWhatIsNotSafe()
    {
        QFETCH(QByteArray, xml);
        QDomDocument doc;
        QString err;
        QVERIFY(!parseXml(xml, &doc, &err));
        QVERIFY(!err.isEmpty());
    }

    void xmlAcceptsDeepButReasonableNesting()
    {
        QDomDocument doc;
        QString err;
        QElapsedTimer timer;
        timer.start();
        QVERIFY2(parseXml(QByteArray("<a>").repeated(1000) + QByteArray("</a>").repeated(1000), &doc, &err), qPrintable(err));
        QVERIFY2(timer.elapsed() < 5000, "deep XML took too long to parse");
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

    // ---- the theme

    void themeColorNames()
    {
        const QStringList names = {"dk1", "lt1", "dk2", "lt2", "accent1", "accent2", "accent3", "accent4", "accent5", "accent6", "hlink", "folHlink"};
        QCOMPARE(names.size(), int(Theme::ColorCount));
        for (int i = 0; i < Theme::ColorCount; ++i) {
            QCOMPARE(themeColorName(Theme::Color(i)), names[i]);
            Theme::Color back = Theme::ColorCount;
            QVERIFY(themeColorFromName(names[i], &back));
            QCOMPARE(int(back), i);
        }
        Theme::Color slot = Theme::Accent3;
        for (const char *bad : {"", "bg1", "tx1", "accent7", "Accent1", "phClr"}) {
            QVERIFY2(!themeColorFromName(QString::fromLatin1(bad), &slot), bad);
            QCOMPARE(int(slot), int(Theme::Accent3));   // untouched
        }
    }

    void themeReadsColorsAndFonts()
    {
        Theme t;
        QString err;
        const QByteArray xml = handWrittenTheme();
        QVERIFY2(readTheme(xml, &t, &err), qPrintable(err));
        QCOMPARE(t.name, QStringLiteral("Test Theme"));
        QCOMPARE(t.colorSchemeName, QStringLiteral("Test Colors"));
        QCOMPARE(t.fontSchemeName, QStringLiteral("Test Fonts"));
        for (int i = 0; i < Theme::ColorCount; ++i) {
            QVERIFY2(t.colors[i].isValid(), qPrintable(themeColorName(Theme::Color(i))));
            QCOMPARE(t.colors[i].name(QColor::HexRgb), kHandWrittenColors[i].name(QColor::HexRgb));   // the sysClr two by their lastClr
            QCOMPARE(t.colors[i].alpha(), 255);
        }
        QCOMPARE(t.majorLatin, QStringLiteral("Georgia"));
        QCOMPARE(t.majorEastAsian, QStringLiteral("Noto Serif CJK JP"));
        QCOMPARE(t.majorComplex, QStringLiteral("Arial"));
        QCOMPARE(t.minorLatin, QStringLiteral("Verdana"));
        QCOMPARE(t.minorEastAsian, QString());
        QCOMPARE(t.minorComplex, QStringLiteral("Tahoma"));
        QCOMPARE(t.xml, xml);
    }

    void themeReadsStrictNamespace()
    {
        Theme t;
        QString err;
        QVERIFY2(readTheme(handWrittenTheme(QString(ns::AStrict)), &t, &err), qPrintable(err));
        QCOMPARE(t.name, QStringLiteral("Test Theme"));
        QCOMPARE(t.colors[Theme::Accent4].name(QColor::HexRgb), QStringLiteral("#ffc000"));
        QCOMPARE(t.minorLatin, QStringLiteral("Verdana"));
    }

    void themeFillsInWhatIsMissing()
    {
        // Only some colors, no fonts: the colors it lacks are the default's.
        const QByteArray xml = QStringLiteral(
            "<a:theme xmlns:a=\"%1\" name=\"Small\"><a:themeElements><a:clrScheme name=\"S\">"
            "<a:accent1><a:srgbClr val=\"123456\"/></a:accent1>"
            "<a:accent2><a:srgbClr val=\"nonsense\"/></a:accent2>"
            "<a:hlink/>"
            "</a:clrScheme></a:themeElements></a:theme>").arg(ns::A).toUtf8();
        Theme t;
        QString err;
        QVERIFY2(readTheme(xml, &t, &err), qPrintable(err));
        const Theme d = defaultTheme();
        QCOMPARE(t.name, QStringLiteral("Small"));
        QCOMPARE(t.colors[Theme::Accent1].name(QColor::HexRgb), QStringLiteral("#123456"));
        QCOMPARE(t.colors[Theme::Accent2], d.colors[Theme::Accent2]);   // unreadable
        QCOMPARE(t.colors[Theme::Hyperlink], d.colors[Theme::Hyperlink]);   // empty
        QCOMPARE(t.colors[Theme::Dark1], d.colors[Theme::Dark1]);   // absent
        QCOMPARE(t.fontSchemeName, QString());
        QCOMPARE(t.majorLatin, QString());
        QCOMPARE(t.minorLatin, QString());
    }

    void themeUsesSystemColorNamesWithoutLastClr()
    {
        const QByteArray xml = QStringLiteral(
            "<a:theme xmlns:a=\"%1\"><a:themeElements><a:clrScheme name=\"S\">"
            "<a:dk1><a:sysClr val=\"windowText\"/></a:dk1><a:lt1><a:sysClr val=\"window\"/></a:lt1>"
            "</a:clrScheme></a:themeElements></a:theme>").arg(ns::A).toUtf8();
        Theme t;
        QVERIFY(readTheme(xml, &t));
        QCOMPARE(t.colors[Theme::Dark1].name(QColor::HexRgb), QStringLiteral("#000000"));
        QCOMPARE(t.colors[Theme::Light1].name(QColor::HexRgb), QStringLiteral("#ffffff"));
    }

    void themeRejectsWhatIsNotATheme_data()
    {
        QTest::addColumn<QByteArray>("xml");
        QTest::newRow("empty") << QByteArray();
        QTest::newRow("not XML") << QByteArray("a theme, in words");
        QTest::newRow("another root") << QStringLiteral("<a:other xmlns:a=\"%1\"/>").arg(ns::A).toUtf8();
        QTest::newRow("a theme in another namespace") << QByteArray("<a:theme xmlns:a=\"urn:other\"><a:themeElements/></a:theme>");
        QTest::newRow("no theme elements") << QStringLiteral("<a:theme xmlns:a=\"%1\"/>").arg(ns::A).toUtf8();
        QTest::newRow("no color scheme") << QStringLiteral("<a:theme xmlns:a=\"%1\"><a:themeElements><a:fontScheme name=\"f\"/></a:themeElements></a:theme>").arg(ns::A).toUtf8();
        QTest::newRow("DOCTYPE") << QByteArray("<!DOCTYPE a:theme [<!ENTITY e \"x\">]>") + handWrittenTheme();
        QByteArray inner = handWrittenTheme();
        inner.insert(inner.indexOf("?>") + 2, "\n<!DOCTYPE a:theme [<!ENTITY e \"x\">]>");
        QTest::newRow("DOCTYPE after the declaration") << inner;
    }

    void themeRejectsWhatIsNotATheme()
    {
        QFETCH(QByteArray, xml);
        Theme t = defaultTheme();
        QString err;
        QVERIFY(!readTheme(xml, &t, &err));
        QVERIFY(!err.isEmpty());
        QCOMPARE(t.name, defaultTheme().name);   // left as it was
    }

    void themeWritesWhatItRead()
    {
        // A part read is written back byte for byte, whatever it holds that
        // the struct doesn't.
        QByteArray xml = handWrittenTheme();
        Theme t;
        QVERIFY(readTheme(xml, &t));
        QCOMPARE(writeTheme(t), xml);
        // Until the xml is cleared, a changed field is not written.
        t.colors[Theme::Accent1] = QColor(Qt::red);
        QCOMPARE(writeTheme(t), xml);
        t.xml.clear();
        Theme back;
        QVERIFY(readTheme(writeTheme(t), &back));
        QCOMPARE(back.colors[Theme::Accent1].name(QColor::HexRgb), QStringLiteral("#ff0000"));
    }

    void themeWritesFromFields()
    {
        Theme t;
        t.name = QStringLiteral("Fine & \"Dandy\" <1>");
        t.colorSchemeName = QStringLiteral("Colors > Others");
        t.fontSchemeName = QStringLiteral("Fonts 'n' Things");
        for (int i = 0; i < Theme::ColorCount; ++i) t.colors[i] = kHandWrittenColors[i];
        t.majorLatin = QStringLiteral("Georgia");
        t.minorLatin = QStringLiteral("Verdana");
        t.majorEastAsian = QStringLiteral("Noto Serif CJK JP");
        t.minorEastAsian = QString();
        t.majorComplex = QStringLiteral("Arial & Co");
        t.minorComplex = QStringLiteral("Tahoma");
        const QByteArray xml = writeTheme(t);
        QVERIFY(!xml.isEmpty());

        Theme back;
        QString err;
        QVERIFY2(readTheme(xml, &back, &err), qPrintable(err));
        QCOMPARE(back.name, t.name);
        QCOMPARE(back.colorSchemeName, t.colorSchemeName);
        QCOMPARE(back.fontSchemeName, t.fontSchemeName);
        for (int i = 0; i < Theme::ColorCount; ++i) QCOMPARE(back.colors[i].name(QColor::HexRgb), t.colors[i].name(QColor::HexRgb));
        QCOMPARE(back.majorLatin, t.majorLatin);
        QCOMPARE(back.minorLatin, t.minorLatin);
        QCOMPARE(back.majorEastAsian, t.majorEastAsian);
        QCOMPARE(back.minorEastAsian, t.minorEastAsian);
        QCOMPARE(back.majorComplex, t.majorComplex);
        QCOMPARE(back.minorComplex, t.minorComplex);
        QCOMPARE(back.xml, xml);
    }

    void themeWrittenHasACompleteStructure()
    {
        // The parts the schema requires: the three schemes in order, and in
        // the format scheme three of each style.
        QDomDocument doc;
        QString err;
        QVERIFY2(parseXml(writeTheme(defaultTheme()), &doc, &err), qPrintable(err));
        const QDomElement root = doc.documentElement();
        QVERIFY(isElement(root, ns::A, QStringLiteral("theme")));
        const QDomElement elements = child(root, ns::A, QStringLiteral("themeElements"));
        QVERIFY(!elements.isNull());
        const QVector<QDomElement> schemes = jp::office::children(elements);
        QCOMPARE(schemes.size(), 3);
        QVERIFY(isElement(schemes[0], ns::A, QStringLiteral("clrScheme")));
        QVERIFY(isElement(schemes[1], ns::A, QStringLiteral("fontScheme")));
        QVERIFY(isElement(schemes[2], ns::A, QStringLiteral("fmtScheme")));
        // Twelve colors, in the schema's order, each a plain RGB.
        const QStringList slots = {"dk1", "lt1", "dk2", "lt2", "accent1", "accent2", "accent3", "accent4", "accent5", "accent6", "hlink", "folHlink"};
        const QVector<QDomElement> colors = jp::office::children(schemes[0]);
        QCOMPARE(colors.size(), 12);
        for (int i = 0; i < 12; ++i) {
            QVERIFY(isElement(colors[i], ns::A, slots[i]));
            QVERIFY(!child(colors[i], ns::A, QStringLiteral("srgbClr")).isNull());
        }
        for (const QString &font : {QStringLiteral("majorFont"), QStringLiteral("minorFont")}) {
            const QDomElement f = child(schemes[1], ns::A, font);
            QVERIFY(!f.isNull());
            // latin, ea, and cs are all required, even when empty.
            QVERIFY(!child(f, ns::A, QStringLiteral("latin")).isNull());
            QVERIFY(!child(f, ns::A, QStringLiteral("ea")).isNull());
            QVERIFY(!child(f, ns::A, QStringLiteral("cs")).isNull());
        }
        const QVector<QDomElement> styles = jp::office::children(schemes[2]);
        QCOMPARE(styles.size(), 4);
        const QStringList lists = {"fillStyleLst", "lnStyleLst", "effectStyleLst", "bgFillStyleLst"};
        for (int i = 0; i < 4; ++i) {
            QVERIFY(isElement(styles[i], ns::A, lists[i]));
            QCOMPARE(jp::office::children(styles[i]).size(), 3);
        }
        QVERIFY(!child(root, ns::A, QStringLiteral("objectDefaults")).isNull());
        QVERIFY(!child(root, ns::A, QStringLiteral("extraClrSchemeLst")).isNull());
    }

    void themeWritesUnsetColorsAsDefault()
    {
        Theme t;
        t.name = QStringLiteral("Bare");
        Theme back;
        QVERIFY(readTheme(writeTheme(t), &back));
        const Theme d = defaultTheme();
        for (int i = 0; i < Theme::ColorCount; ++i) QCOMPARE(back.colors[i].name(QColor::HexRgb), d.colors[i].name(QColor::HexRgb));
    }

    void themeDefault()
    {
        const Theme d = defaultTheme();
        QCOMPARE(d.name, QStringLiteral("JeffOffice"));
        QCOMPARE(d.colorSchemeName, QStringLiteral("JeffOffice"));
        QCOMPARE(d.fontSchemeName, QStringLiteral("JeffOffice"));
        QCOMPARE(d.majorLatin, QStringLiteral("Carlito"));
        QCOMPARE(d.minorLatin, QStringLiteral("Carlito"));
        QVERIFY(d.xml.isEmpty());
        const char *expected[Theme::ColorCount] = {"#1f2937", "#ffffff", "#24364f", "#e8ecf2", "#1d55c4", "#b6420f",
                                                   "#13773a", "#9e1f63", "#0a6c8c", "#5e2ca5", "#1d55c4", "#5e2ca5"};
        for (int i = 0; i < Theme::ColorCount; ++i) {
            QCOMPARE(d.colors[i].name(QColor::HexRgb), QString::fromLatin1(expected[i]));
            QCOMPARE(d.colors[i].alpha(), 255);
        }
        // It makes a part and reads it back as it was.
        Theme back;
        QString err;
        QVERIFY2(readTheme(writeTheme(d), &back, &err), qPrintable(err));
        QCOMPARE(back.name, d.name);
        QCOMPARE(back.majorLatin, d.majorLatin);
        for (int i = 0; i < Theme::ColorCount; ++i) QCOMPARE(back.colors[i], d.colors[i]);
    }

    // ---- DrawingML colors

    void colorReadsRgb()
    {
        QCOMPARE(hexOf(colorOf(srgbClr(QStringLiteral("FF8000")))), QStringLiteral("FF8000"));
        QCOMPARE(hexOf(colorOf(srgbClr(QStringLiteral("0a1b2c")))), QStringLiteral("0A1B2C"));
        QCOMPARE(colorOf(srgbClr(QStringLiteral("FF8000"))).alpha(), 255);
        for (const char *bad : {"", "GG0000", "FF80", "#FF8000", "FF800000", "red", "0x1234", " FFFFF", "+FFFFF", "FFFFF ", "-FFFFF", "FF 000"})
            QVERIFY2(!colorOf(srgbClr(QString::fromLatin1(bad))).isValid(), bad);
    }

    void colorNeedsAColorChoice()
    {
        QVERIFY(!colorOf(QString()).isValid());
        QVERIFY(!colorOf(QStringLiteral("<a:noFill/>")).isValid());
        QVERIFY(!colorOf(QStringLiteral("<a:srgbClr/>")).isValid());
        QVERIFY(!readColor(QDomElement(), colorTestTheme(), ColorMap()).isValid());
        // The first color choice is the color, whatever else is there.
        QCOMPARE(hexOf(colorOf(QStringLiteral("<a:extLst/><a:srgbClr val=\"FF0000\"/><a:srgbClr val=\"00FF00\"/>"))), QStringLiteral("FF0000"));
        // A color in some other namespace is none.
        QVERIFY(!colorOf(QStringLiteral("<x:srgbClr xmlns:x=\"urn:other\" val=\"FF0000\"/>")).isValid());
    }

    void colorReadsStrictNamespace()
    {
        QCOMPARE(hexOf(colorOf(srgbClr(QStringLiteral("FF0000")), QColor(), ColorMap(), colorTestTheme(), QString(ns::AStrict))), QStringLiteral("FF0000"));
        QCOMPARE(hexOf(colorOf(schemeClr(QStringLiteral("accent1"), {"lumMod 75000"}), QColor(), ColorMap(), colorTestTheme(), QString(ns::AStrict))),
                 QStringLiteral("2F5597"));
    }

    void colorReadsSchemeColors()
    {
        const struct { const char *name; Theme::Color slot; } names[] = {
            {"dk1", Theme::Dark1}, {"lt1", Theme::Light1}, {"dk2", Theme::Dark2}, {"lt2", Theme::Light2},
            {"accent1", Theme::Accent1}, {"accent2", Theme::Accent2}, {"accent3", Theme::Accent3}, {"accent4", Theme::Accent4},
            {"accent5", Theme::Accent5}, {"accent6", Theme::Accent6}, {"hlink", Theme::Hyperlink}, {"folHlink", Theme::FollowedHyperlink},
            // Through the usual map.
            {"bg1", Theme::Light1}, {"tx1", Theme::Dark1}, {"bg2", Theme::Light2}, {"tx2", Theme::Dark2}};
        for (const auto &n : names)
            QCOMPARE(hexOf(colorOf(schemeClr(QString::fromLatin1(n.name)))), hexOf(kHandWrittenColors[n.slot]));
        for (const char *bad : {"", "foo", "Accent1", "accent7", "phClr", "window"})
            QVERIFY2(!colorOf(schemeClr(QString::fromLatin1(bad))).isValid(), bad);
        // A theme with a color unset gives none.
        QVERIFY(!colorOf(schemeClr(QStringLiteral("accent1")), QColor(), ColorMap(), Theme()).isValid());
    }

    void colorMapChoosesThemeColors()
    {
        const ColorMap usual;
        Theme::Color slot = Theme::ColorCount;
        QVERIFY(usual.resolve(QStringLiteral("bg1"), &slot));
        QCOMPARE(int(slot), int(Theme::Light1));
        QVERIFY(usual.resolve(QStringLiteral("tx2"), &slot));
        QCOMPARE(int(slot), int(Theme::Dark2));
        QVERIFY(usual.resolve(QStringLiteral("accent5"), &slot));
        QCOMPARE(int(slot), int(Theme::Accent5));
        QVERIFY(usual.resolve(QStringLiteral("dk2"), &slot));
        QCOMPARE(int(slot), int(Theme::Dark2));
        QVERIFY(!usual.resolve(QStringLiteral("phClr"), &slot));
        QVERIFY(!usual.resolve(QStringLiteral("nonsense"), &slot));

        // A dark slide: its background is dark 1, its text light 1, and two accents swap.
        QDomDocument doc;
        QVERIFY(parseXml(QStringLiteral("<p:clrMap xmlns:p=\"%1\" bg1=\"dk1\" tx1=\"lt1\" bg2=\"dk2\" tx2=\"lt2\" accent1=\"accent2\" accent2=\"accent1\" "
                                        "accent3=\"accent3\" accent4=\"accent4\" accent5=\"accent5\" accent6=\"accent6\" hlink=\"hlink\" folHlink=\"folHlink\"/>")
                             .arg(ns::P).toUtf8(), &doc));
        const ColorMap dark = readColorMap(doc.documentElement());
        QCOMPARE(hexOf(colorOf(schemeClr(QStringLiteral("bg1")), QColor(), dark)), hexOf(kHandWrittenColors[Theme::Dark1]));
        QCOMPARE(hexOf(colorOf(schemeClr(QStringLiteral("tx1")), QColor(), dark)), hexOf(kHandWrittenColors[Theme::Light1]));
        QCOMPARE(hexOf(colorOf(schemeClr(QStringLiteral("bg2")), QColor(), dark)), hexOf(kHandWrittenColors[Theme::Dark2]));
        QCOMPARE(hexOf(colorOf(schemeClr(QStringLiteral("tx2")), QColor(), dark)), hexOf(kHandWrittenColors[Theme::Light2]));
        QCOMPARE(hexOf(colorOf(schemeClr(QStringLiteral("accent1")), QColor(), dark)), hexOf(kHandWrittenColors[Theme::Accent2]));
        QCOMPARE(hexOf(colorOf(schemeClr(QStringLiteral("accent2")), QColor(), dark)), hexOf(kHandWrittenColors[Theme::Accent1]));
        // The theme's own names don't go through the map.
        QCOMPARE(hexOf(colorOf(schemeClr(QStringLiteral("dk1")), QColor(), dark)), hexOf(kHandWrittenColors[Theme::Dark1]));
        QCOMPARE(hexOf(colorOf(schemeClr(QStringLiteral("accent3")), QColor(), dark)), hexOf(kHandWrittenColors[Theme::Accent3]));
    }

    void colorMapDefaultsWhatIsMissing()
    {
        QDomDocument doc;
        QVERIFY(parseXml(QStringLiteral("<a:overrideClrMapping xmlns:a=\"%1\" bg1=\"dk1\" tx1=\"nonsense\" bg2=\"bg1\" accent4=\"accent6\"/>")
                             .arg(ns::A).toUtf8(), &doc));
        const ColorMap m = readColorMap(doc.documentElement());
        QCOMPARE(int(m.bg1), int(Theme::Dark1));
        QCOMPARE(int(m.tx1), int(Theme::Dark1));        // not a theme color: the usual
        QCOMPARE(int(m.bg2), int(Theme::Light2));       // "bg1" is not one either
        QCOMPARE(int(m.tx2), int(Theme::Dark2));        // absent
        QCOMPARE(int(m.accent[3]), int(Theme::Accent6));
        QCOMPARE(int(m.accent[0]), int(Theme::Accent1));
        QCOMPARE(int(m.hlink), int(Theme::Hyperlink));
        QCOMPARE(int(m.folHlink), int(Theme::FollowedHyperlink));
        // No element: the usual map.
        const ColorMap none = readColorMap(QDomElement());
        QCOMPARE(int(none.bg1), int(Theme::Light1));
        QCOMPARE(int(none.tx1), int(Theme::Dark1));
        QCOMPARE(int(none.accent[5]), int(Theme::Accent6));
    }

    void colorUsesThePlaceholderColor()
    {
        const QColor red(255, 0, 0);
        QCOMPARE(hexOf(colorOf(schemeClr(QStringLiteral("phClr")), red)), QStringLiteral("FF0000"));
        QCOMPARE(hexOf(colorOf(schemeClr(QStringLiteral("phClr"), {"lumMod 40000"}), red)), QStringLiteral("660000"));
        QVERIFY(!colorOf(schemeClr(QStringLiteral("phClr"))).isValid());
        // It is for "phClr" only.
        QCOMPARE(hexOf(colorOf(srgbClr(QStringLiteral("00FF00")), red)), QStringLiteral("00FF00"));
        QCOMPARE(hexOf(colorOf(schemeClr(QStringLiteral("accent1")), red)), hexOf(kHandWrittenColors[Theme::Accent1]));
        // The placeholder's own alpha goes along, and alpha can be set over it.
        const QColor half(255, 0, 0, 128);
        QCOMPARE(colorOf(schemeClr(QStringLiteral("phClr")), half).alpha(), 128);
        QCOMPARE(colorOf(schemeClr(QStringLiteral("phClr"), {"lumMod 40000"}), half).alpha(), 128);
        QCOMPARE(colorOf(schemeClr(QStringLiteral("phClr"), {"alpha 100000"}), half).alpha(), 255);
    }

    void colorAppliesTransforms_data()
    {
        QTest::addColumn<QString>("xml");
        QTest::addColumn<QString>("hex");
        QTest::addColumn<int>("tolerance");
        const QString accent1 = QStringLiteral("accent1");   // 4472C4
        // Luminance, saturation, and hue, in HSL.
        QTest::newRow("lumMod 60 lumOff 40") << schemeClr(accent1, {"lumMod 60000", "lumOff 40000"}) << "8FAADC" << 0;
        QTest::newRow("lumMod 75") << schemeClr(accent1, {"lumMod 75000"}) << "2F5597" << 0;
        QTest::newRow("lumMod 50") << schemeClr(accent1, {"lumMod 50000"}) << "203864" << 0;
        QTest::newRow("lumMod 20 lumOff 80") << schemeClr(accent1, {"lumMod 20000", "lumOff 80000"}) << "DAE3F3" << 0;
        QTest::newRow("lumMod 40 lumOff 60") << schemeClr(accent1, {"lumMod 40000", "lumOff 60000"}) << "B4C7E7" << 0;
        QTest::newRow("lumMod 200 stops at white") << schemeClr(accent1, {"lumMod 200000"}) << "FFFFFF" << 0;
        QTest::newRow("lumOff -80 stops at black") << schemeClr(accent1, {"lumOff -80000"}) << "000000" << 0;
        QTest::newRow("lum 50") << schemeClr(accent1, {"lum 50000"}) << "3D6DC2" << 0;
        QTest::newRow("lumMod 50 of a gray") << srgbClr(QStringLiteral("808080"), {"lumMod 50000"}) << "404040" << 0;
        QTest::newRow("satMod 50") << schemeClr(accent1, {"satMod 50000"}) << "647BA4" << 0;
        QTest::newRow("satOff 20") << schemeClr(accent1, {"satOff 20000"}) << "2B6BDD" << 0;
        QTest::newRow("satMod 300 stops at full") << schemeClr(accent1, {"satMod 300000"}) << "0961FF" << 0;
        QTest::newRow("sat 0 is a gray") << schemeClr(accent1, {"sat 0"}) << "848484" << 0;
        QTest::newRow("hueOff 60 degrees") << schemeClr(accent1, {"hueOff 3600000"}) << "9644C4" << 0;
        QTest::newRow("hueOff -60 degrees") << schemeClr(accent1, {"hueOff -3600000"}) << "44C496" << 0;
        QTest::newRow("hueOff a full turn") << schemeClr(accent1, {"hueOff 21600000"}) << "4472C4" << 0;
        QTest::newRow("hueMod 50") << schemeClr(accent1, {"hueMod 50000"}) << "5BC444" << 0;
        QTest::newRow("hue 0") << schemeClr(accent1, {"hue 0"}) << "C44444" << 0;
        QTest::newRow("comp") << schemeClr(accent1, {"comp"}) << "C49644" << 0;
        QTest::newRow("comp of red") << srgbClr(QStringLiteral("FF0000"), {"comp"}) << "00FFFF" << 0;
        QTest::newRow("inv") << schemeClr(accent1, {"inv"}) << "BB8D3B" << 0;
        QTest::newRow("inv of white") << srgbClr(QStringLiteral("FFFFFF"), {"inv"}) << "000000" << 0;
        QTest::newRow("gray") << schemeClr(accent1, {"gray"}) << "6E6E6E" << 0;
        QTest::newRow("gray of red") << srgbClr(QStringLiteral("FF0000"), {"gray"}) << "4C4C4C" << 0;
        QTest::newRow("gray of white") << srgbClr(QStringLiteral("FFFFFF"), {"gray"}) << "FFFFFF" << 0;
        // The channels, in sRGB.
        QTest::newRow("red 50") << schemeClr(accent1, {"red 50000"}) << "8072C4" << 0;
        QTest::newRow("redMod 50") << schemeClr(accent1, {"redMod 50000"}) << "2272C4" << 0;
        QTest::newRow("greenOff 10") << schemeClr(accent1, {"greenOff 10000"}) << "448CC4" << 0;
        QTest::newRow("blueMod 150 stops at full") << schemeClr(accent1, {"blueMod 150000"}) << "4472FF" << 0;
        QTest::newRow("redOff -100 stops at none") << schemeClr(accent1, {"redOff -100000"}) << "0072C4" << 0;
        // Shade and tint, in linear RGB.
        QTest::newRow("shade 50 of white") << srgbClr(QStringLiteral("FFFFFF"), {"shade 50000"}) << "BCBCBC" << 1;
        QTest::newRow("tint 50 of black") << srgbClr(QStringLiteral("000000"), {"tint 50000"}) << "BCBCBC" << 1;
        QTest::newRow("shade 50") << schemeClr(accent1, {"shade 50000"}) << "2F528F" << 1;
        QTest::newRow("tint 40") << schemeClr(accent1, {"tint 40000"}) << "CFD5EA" << 1;
        QTest::newRow("shade 25 of white") << srgbClr(QStringLiteral("FFFFFF"), {"shade 25000"}) << "898989" << 1;
        QTest::newRow("tint 80") << srgbClr(QStringLiteral("FF8000"), {"tint 80000"}) << "FFA47C" << 1;
        QTest::newRow("shade 100 keeps it") << schemeClr(accent1, {"shade 100000"}) << "4472C4" << 1;
        QTest::newRow("tint 100 keeps it") << schemeClr(accent1, {"tint 100000"}) << "4472C4" << 1;
        QTest::newRow("tint 0 is white") << schemeClr(accent1, {"tint 0"}) << "FFFFFF" << 0;
        QTest::newRow("shade 0 is black") << schemeClr(accent1, {"shade 0"}) << "000000" << 0;
        // In the order they are written.
        QTest::newRow("lumOff 20 then lumMod 50") << schemeClr(accent1, {"lumOff 20000", "lumMod 50000"}) << "2C4E8B" << 0;
        QTest::newRow("lumMod 50 then lumOff 20") << schemeClr(accent1, {"lumMod 50000", "lumOff 20000"}) << "3864B2" << 0;
        // What is not understood is passed over.
        QTest::newRow("gamma and invGamma") << schemeClr(accent1, {"gamma", "invGamma"}) << "4472C4" << 0;
        QTest::newRow("a transform of the future") << schemeClr(accent1, {"bogus 5", "lumMod 75000"}) << "2F5597" << 0;
        QTest::newRow("a transform with no value") << schemeClr(accent1, {"lumMod", "lumOff"}) << "4472C4" << 0;
        QTest::newRow("a transform with a bad value") << schemeClr(accent1, {"lumMod many"}) << "4472C4" << 0;
        QTest::newRow("alpha leaves the color") << schemeClr(accent1, {"alpha 50000"}) << "4472C4" << 0;
    }

    void colorAppliesTransforms()
    {
        QFETCH(QString, xml);
        QFETCH(QString, hex);
        QFETCH(int, tolerance);
        const QColor c = colorOf(xml);
        QVERIFY2(nearTo(c, hex, tolerance), qPrintable(QStringLiteral("%1 is not %2 (within %3)").arg(hexOf(c), hex).arg(tolerance)));
    }

    void colorAppliesAlpha()
    {
        auto alphaOf = [](const QStringList &list) { return colorOf(srgbClr(QStringLiteral("FF0000"), list)).alpha(); };
        QCOMPARE(alphaOf({}), 255);
        QVERIFY(qAbs(alphaOf({"alpha 50000"}) - 128) <= 1);
        QCOMPARE(alphaOf({"alpha 0"}), 0);
        QCOMPARE(alphaOf({"alpha 100000"}), 255);
        QCOMPARE(alphaOf({"alpha 25000"}), 64);
        QCOMPARE(alphaOf({"alpha 150000"}), 255);                   // stops at opaque
        QCOMPARE(alphaOf({"alpha 50000", "alphaMod 50000"}), 64);
        QCOMPARE(alphaOf({"alphaMod 50000"}), 128);                 // of opaque
        QCOMPARE(alphaOf({"alphaMod 0"}), 0);
        QCOMPARE(alphaOf({"alpha 100000", "alphaOff -25000"}), 191);
        QCOMPARE(alphaOf({"alphaOff -50000"}), 128);
        QCOMPARE(alphaOf({"alpha 80000", "alphaOff 50000"}), 255);  // stops at opaque
        QCOMPARE(alphaOf({"alphaOff -200000"}), 0);                 // stops at clear
        // The color under it is the color.
        QCOMPARE(hexOf(colorOf(srgbClr(QStringLiteral("FF0000"), {"alpha 50000"}))), QStringLiteral("FF0000"));
        // Other transforms keep the alpha.
        const QColor c = colorOf(srgbClr(QStringLiteral("FF0000"), {"alpha 50000", "lumMod 40000"}));
        QCOMPARE(hexOf(c), QStringLiteral("660000"));
        QVERIFY(qAbs(c.alpha() - 128) <= 1);
    }

    void colorReadsSystemColors()
    {
        // The value it had when saved wins over the name.
        QCOMPARE(hexOf(colorOf(QStringLiteral("<a:sysClr val=\"windowText\" lastClr=\"F0F0F0\"/>"))), QStringLiteral("F0F0F0"));
        QCOMPARE(hexOf(colorOf(QStringLiteral("<a:sysClr val=\"window\" lastClr=\"101010\"/>"))), QStringLiteral("101010"));
        // Without it, the name.
        QCOMPARE(hexOf(colorOf(QStringLiteral("<a:sysClr val=\"window\"/>"))), QStringLiteral("FFFFFF"));
        QCOMPARE(hexOf(colorOf(QStringLiteral("<a:sysClr val=\"windowText\"/>"))), QStringLiteral("000000"));
        QVERIFY(!colorOf(QStringLiteral("<a:sysClr val=\"menu\"/>")).isValid());
        QVERIFY(!colorOf(QStringLiteral("<a:sysClr/>")).isValid());
        // A bad lastClr is passed over for the name.
        QCOMPARE(hexOf(colorOf(QStringLiteral("<a:sysClr val=\"window\" lastClr=\"zz\"/>"))), QStringLiteral("FFFFFF"));
        QCOMPARE(hexOf(colorOf(QStringLiteral("<a:sysClr val=\"window\" lastClr=\"808080\"><a:lumMod val=\"50000\"/></a:sysClr>"))), QStringLiteral("404040"));
    }

    void colorReadsPresetColors()
    {
        const struct { const char *name, *hex; } known[] = {
            {"black", "000000"}, {"white", "FFFFFF"}, {"red", "FF0000"}, {"blue", "0000FF"}, {"lime", "00FF00"}, {"gray", "808080"},
            {"green", "008000"}, {"aliceBlue", "F0F8FF"}, {"yellowGreen", "9ACD32"}, {"dimGray", "696969"},
            // dk, lt, and med for dark, light, and medium.
            {"dkBlue", "00008B"}, {"dkGray", "A9A9A9"}, {"dkOliveGreen", "556B2F"}, {"dkOrange", "FF8C00"}, {"dkSlateGray", "2F4F4F"},
            {"ltGray", "D3D3D3"}, {"ltGoldenrodYellow", "FAFAD2"}, {"ltSlateGray", "778899"}, {"medAquamarine", "66CDAA"},
            {"medVioletRed", "C71585"}, {"ltGrey", "D3D3D3"}, {"dkGrey", "A9A9A9"}};
        for (const auto &k : known)
            QCOMPARE(hexOf(colorOf(QStringLiteral("<a:prstClr val=\"%1\"/>").arg(QString::fromLatin1(k.name)))), QString::fromLatin1(k.hex));
        // Every name the schema lists.
        const QStringList all = QStringLiteral(
            "aliceBlue antiqueWhite aqua aquamarine azure beige bisque black blanchedAlmond blue blueViolet brown burlyWood cadetBlue "
            "chartreuse chocolate coral cornflowerBlue cornsilk crimson cyan dkBlue dkCyan dkGoldenrod dkGray dkGreen dkKhaki dkMagenta "
            "dkOliveGreen dkOrange dkOrchid dkRed dkSalmon dkSeaGreen dkSlateBlue dkSlateGray dkTurquoise dkViolet deepPink deepSkyBlue "
            "dimGray dodgerBlue firebrick floralWhite forestGreen fuchsia gainsboro ghostWhite gold goldenrod gray green greenYellow "
            "honeydew hotPink indianRed indigo ivory khaki lavender lavenderBlush lawnGreen lemonChiffon ltBlue ltCoral ltCyan "
            "ltGoldenrodYellow ltGray ltGreen ltPink ltSalmon ltSeaGreen ltSkyBlue ltSlateGray ltSteelBlue ltYellow lime limeGreen linen "
            "magenta maroon medAquamarine medBlue medOrchid medPurple medSeaGreen medSlateBlue medSpringGreen medTurquoise medVioletRed "
            "midnightBlue mintCream mistyRose moccasin navajoWhite navy oldLace olive oliveDrab orange orangeRed orchid paleGoldenrod "
            "paleGreen paleTurquoise paleVioletRed papayaWhip peachPuff peru pink plum powderBlue purple red rosyBrown royalBlue "
            "saddleBrown salmon sandyBrown seaGreen seaShell sienna silver skyBlue slateBlue slateGray snow springGreen steelBlue tan "
            "teal thistle tomato turquoise violet wheat white whiteSmoke yellow yellowGreen "
            "dkGrey dimGrey dkSlateGrey grey ltGrey ltSlateGrey slateGrey").split(QLatin1Char(' '));
        QCOMPARE(all.size(), 147);
        for (const QString &name : all)
            QVERIFY2(colorOf(QStringLiteral("<a:prstClr val=\"%1\"/>").arg(name)).isValid(), qPrintable(name));
        for (const char *bad : {"", "nonsense", "dkNonsense", "transparent", "#FF0000", "re d", "red1"})
            QVERIFY2(!colorOf(QStringLiteral("<a:prstClr val=\"%1\"/>").arg(QString::fromLatin1(bad))).isValid(), bad);
        QVERIFY(!colorOf(QStringLiteral("<a:prstClr/>")).isValid());
    }

    void colorReadsHslAndScrgb()
    {
        auto hsl = [](const char *hue, const char *sat, const char *lum) {
            return colorOf(QStringLiteral("<a:hslClr hue=\"%1\" sat=\"%2\" lum=\"%3\"/>").arg(QString::fromLatin1(hue), QString::fromLatin1(sat), QString::fromLatin1(lum)));
        };
        QCOMPARE(hexOf(hsl("0", "100000", "50000")), QStringLiteral("FF0000"));
        QCOMPARE(hexOf(hsl("7200000", "50000", "25000")), QStringLiteral("206020"));      // 120 degrees
        QCOMPARE(hexOf(hsl("14400000", "100000", "50000")), QStringLiteral("0000FF"));    // 240 degrees
        QCOMPARE(hexOf(hsl("0", "0", "100000")), QStringLiteral("FFFFFF"));
        QCOMPARE(hexOf(hsl("0", "0", "0")), QStringLiteral("000000"));
        QVERIFY(!colorOf(QStringLiteral("<a:hslClr hue=\"0\" sat=\"100000\"/>")).isValid());   // lum missing

        auto scrgb = [](const char *r, const char *g, const char *b) {
            return colorOf(QStringLiteral("<a:scrgbClr r=\"%1\" g=\"%2\" b=\"%3\"/>").arg(QString::fromLatin1(r), QString::fromLatin1(g), QString::fromLatin1(b)));
        };
        QCOMPARE(hexOf(scrgb("100000", "0", "0")), QStringLiteral("FF0000"));
        QCOMPARE(hexOf(scrgb("0", "0", "0")), QStringLiteral("000000"));
        QCOMPARE(hexOf(scrgb("100000", "100000", "100000")), QStringLiteral("FFFFFF"));
        // The values are linear light, as scRGB is: half is a lighter sRGB gray.
        QVERIFY(nearTo(scrgb("50000", "50000", "50000"), QStringLiteral("BCBCBC"), 1));
        QVERIFY(nearTo(scrgb("25000", "50000", "75000"), QStringLiteral("89BCE1"), 1));
        QVERIFY(!colorOf(QStringLiteral("<a:scrgbClr r=\"x\" g=\"0\" b=\"0\"/>")).isValid());
    }

    void themeReadsAnyColorInASlot()
    {
        QByteArray xml = handWrittenTheme();
        xml.replace("<a:accent1><a:srgbClr val=\"4472C4\"/></a:accent1>", "<a:accent1><a:prstClr val=\"dkBlue\"/></a:accent1>");
        xml.replace("<a:accent2><a:srgbClr val=\"ED7D31\"/></a:accent2>", "<a:accent2><a:hslClr hue=\"0\" sat=\"100000\" lum=\"50000\"/></a:accent2>");
        xml.replace("<a:accent3><a:srgbClr val=\"A5A5A5\"/></a:accent3>", "<a:accent3><a:srgbClr val=\"808080\"><a:lumMod val=\"50000\"/></a:srgbClr></a:accent3>");
        Theme t;
        QString err;
        QVERIFY2(readTheme(xml, &t, &err), qPrintable(err));
        QCOMPARE(hexOf(t.colors[Theme::Accent1]), QStringLiteral("00008B"));
        QCOMPARE(hexOf(t.colors[Theme::Accent2]), QStringLiteral("FF0000"));
        QCOMPARE(hexOf(t.colors[Theme::Accent3]), QStringLiteral("404040"));
        QCOMPARE(hexOf(t.colors[Theme::Accent4]), QStringLiteral("FFC000"));   // the rest as before
    }

    void themeAndColorsSurviveRandomDamage()
    {
        const QByteArray good = handWrittenTheme();
        const QByteArray color = QStringLiteral("<a:solidFill xmlns:a=\"%1\"><a:schemeClr val=\"accent1\"><a:lumMod val=\"60000\"/><a:hueOff val=\"3600000\"/>"
                                                "<a:alpha val=\"50000\"/><a:tint val=\"40000\"/></a:schemeClr></a:solidFill>").arg(ns::A).toUtf8();
        QRandomGenerator gen(77);
        for (int i = 0; i < 500; ++i) {
            for (const QByteArray *source : {&good, &color}) {
                QByteArray bad = *source;
                for (int k = 0, n = 1 + int(gen.bounded(3)); k < n; ++k) bad[int(gen.bounded(quint32(bad.size())))] = char(gen.bounded(256u));
                Theme t;
                readTheme(bad, &t);
                QDomDocument doc;
                if (parseXml(bad, &doc)) readColor(doc.documentElement(), colorTestTheme(), ColorMap(), QColor(Qt::red));
            }
        }
    }

private:
    // ---- DrawingML shapes: what the tests below build their drawings from

    static QString dmlNamespaces()
    {
        return QStringLiteral("xmlns:a=\"%1\" xmlns:p=\"%2\" xmlns:r=\"%3\"").arg(QString(ns::A), QString(ns::P), QString(ns::R));
    }

    static QString emu(double points) { return QString::number(qint64(points * 12700)); }

    // <a:custGeom> holding `inner`, as the first child of a throwaway document.
    static QDomElement dmlElement(QDomDocument *doc, const QString &xml)
    {
        const QByteArray text = QStringLiteral("<root %1>%2</root>").arg(dmlNamespaces(), xml).toUtf8();
        if (!parseXml(text, doc)) return QDomElement();
        return doc->documentElement().firstChildElement();
    }

    static bool near(double a, double b, double tolerance = 1e-6) { return qAbs(a - b) <= tolerance * qMax(1.0, qAbs(b)); }

    static Geometry customGeometryOf(const QString &inner, const QSizeF &size)
    {
        QDomDocument doc;
        const QDomElement e = dmlElement(&doc, QStringLiteral("<a:custGeom>%1</a:custGeom>").arg(inner));
        return readCustomGeometry(e, size);
    }

    // The path "M x y L x y ..." the tests draw lines with, in a path of `w` by `h`.
    static QString linePath(const QString &attrs, const QList<QPair<QString, QString>> &points, bool close = true)
    {
        QString out = QStringLiteral("<a:path %1>").arg(attrs);
        for (int i = 0; i < points.size(); ++i)
            out += QStringLiteral("<a:%1><a:pt x=\"%2\" y=\"%3\"/></a:%1>").arg(i == 0 ? QStringLiteral("moveTo") : QStringLiteral("lnTo"), points[i].first, points[i].second);
        return out + (close ? QStringLiteral("<a:close/>") : QString()) + QStringLiteral("</a:path>");
    }

private Q_SLOTS:
    // ---- DrawingML shapes: guides and geometry

    void dmlGuideFormulas_data()
    {
        QTest::addColumn<QString>("formula");
        QTest::addColumn<double>("expected");
        QTest::newRow("val") << "val 42" << 42.0;
        QTest::newRow("multiply and divide") << "*/ 10 20 4" << 50.0;
        QTest::newRow("add and subtract") << "+- 10 3 4" << 9.0;
        QTest::newRow("add and divide") << "+/ 10 20 5" << 6.0;
        QTest::newRow("if positive") << "?: 1 7 9" << 7.0;
        QTest::newRow("if zero") << "?: 0 7 9" << 9.0;
        QTest::newRow("if negative") << "?: -1 7 9" << 9.0;
        QTest::newRow("abs") << "abs -5" << 5.0;
        QTest::newRow("at2 of 45 degrees") << "at2 1 1" << 2700000.0;
        QTest::newRow("at2 straight down") << "at2 0 1" << 5400000.0;
        QTest::newRow("at2 straight left") << "at2 -1 0" << 10800000.0;
        QTest::newRow("cat2") << "cat2 100 3 4" << 60.0;
        QTest::newRow("sat2") << "sat2 100 3 4" << 80.0;
        QTest::newRow("cos zero") << "cos 100 0" << 100.0;
        QTest::newRow("cos 60 degrees") << "cos 100 3600000" << 50.0;
        QTest::newRow("sin 90 degrees") << "sin 100 5400000" << 100.0;
        QTest::newRow("sin 45 degrees") << "sin 100 2700000" << 70.71067811865476;
        QTest::newRow("tan 45 degrees") << "tan 100 2700000" << 100.0;
        QTest::newRow("max") << "max 3 9" << 9.0;
        QTest::newRow("min") << "min 3 9" << 3.0;
        QTest::newRow("mod") << "mod 3 4 12" << 13.0;
        QTest::newRow("pin inside") << "pin 0 5 10" << 5.0;
        QTest::newRow("pin below") << "pin 0 -5 10" << 0.0;
        QTest::newRow("pin above") << "pin 0 15 10" << 10.0;
        QTest::newRow("sqrt") << "sqrt 144" << 12.0;
        QTest::newRow("sqrt of a negative") << "sqrt -4" << 0.0;
        QTest::newRow("divide by zero") << "*/ 1 2 0" << 0.0;
        QTest::newRow("add and divide by zero") << "+/ 1 2 0" << 0.0;
        QTest::newRow("unknown operator") << "frob 1 2 3" << 0.0;
        QTest::newRow("missing arguments") << "+-" << 0.0;
        QTest::newRow("empty") << "" << 0.0;
        QTest::newRow("extra spaces") << "  */   6   7  2 " << 21.0;
        QTest::newRow("a name never defined") << "+- nothing 3 0" << 3.0;
    }

    void dmlGuideFormulas()
    {
        QFETCH(QString, formula);
        QFETCH(double, expected);
        GuideEvaluator g(1000, 600);
        g.define(QStringLiteral("x"), formula);
        QVERIFY2(near(g.value(QStringLiteral("x")), expected, 1e-9), qPrintable(QString::number(g.value(QStringLiteral("x")))));
    }

    void dmlGuideBuiltInNames()
    {
        GuideEvaluator g(1000, 600);
        const struct { const char *name; double value; } names[] = {
            {"w", 1000}, {"h", 600}, {"l", 0}, {"t", 0}, {"r", 1000}, {"b", 600}, {"hc", 500}, {"vc", 300},
            {"wd2", 500}, {"wd3", 1000.0 / 3}, {"wd4", 250}, {"wd5", 200}, {"wd6", 1000.0 / 6}, {"wd8", 125}, {"wd10", 100}, {"wd32", 31.25},
            {"hd2", 300}, {"hd3", 200}, {"hd4", 150}, {"hd5", 120}, {"hd6", 100}, {"hd8", 75},
            {"ss", 600}, {"ls", 1000}, {"ssd2", 300}, {"ssd4", 150}, {"ssd6", 100}, {"ssd8", 75}, {"ssd16", 37.5}, {"ssd32", 18.75},
            {"cd2", 10800000}, {"cd4", 5400000}, {"cd8", 2700000}, {"3cd4", 16200000}, {"3cd8", 8100000}, {"5cd8", 13500000}, {"7cd8", 18900000}};
        for (const auto &n : names) QVERIFY2(near(g.value(QString::fromLatin1(n.name)), n.value), n.name);
        // A number written out is its value, and a name that was never defined is 0.
        QCOMPARE(g.value(QStringLiteral("123")), 123.0);
        QCOMPARE(g.value(QStringLiteral("-4.5")), -4.5);
        QCOMPARE(g.value(QStringLiteral("nothing")), 0.0);
        QCOMPARE(g.value(QStringLiteral("nan")), 0.0);
        QCOMPARE(g.value(QStringLiteral("inf")), 0.0);
        // The shorter side is ss however the shape is turned.
        GuideEvaluator tall(300, 900);
        QCOMPARE(tall.value(QStringLiteral("ss")), 300.0);
        QCOMPARE(tall.value(QStringLiteral("ls")), 900.0);
    }

    void dmlGuidesReferToEachOther()
    {
        GuideEvaluator g(1000, 600);
        g.set(QStringLiteral("adj"), 25000);
        g.define(QStringLiteral("dx"), QStringLiteral("*/ ss adj 100000"));       // 150
        g.define(QStringLiteral("x1"), QStringLiteral("+- l dx 0"));              // 150
        g.define(QStringLiteral("x2"), QStringLiteral("+- r 0 dx"));              // 850
        g.define(QStringLiteral("early"), QStringLiteral("+- late 1 0"));         // a guide used before it is defined
        g.define(QStringLiteral("late"), QStringLiteral("val 5"));
        QCOMPARE(g.value(QStringLiteral("x1")), 150.0);
        QCOMPARE(g.value(QStringLiteral("x2")), 850.0);
        QCOMPARE(g.value(QStringLiteral("early")), 6.0);
        // The first definition of a name is the one that counts.
        g.define(QStringLiteral("x1"), QStringLiteral("val 1"));
        QCOMPARE(g.value(QStringLiteral("x1")), 150.0);
        // A value set outright wins over a formula for the name.
        g.set(QStringLiteral("dx"), 7);
        QCOMPARE(g.value(QStringLiteral("dx")), 7.0);
    }

    void dmlGuidesSurviveLoopsAndExcess()
    {
        GuideEvaluator g(1000, 600);
        g.define(QStringLiteral("a"), QStringLiteral("+- b 1 0"));
        g.define(QStringLiteral("b"), QStringLiteral("+- a 1 0"));
        g.define(QStringLiteral("self"), QStringLiteral("+- self 1 0"));
        QVERIFY(std::isfinite(g.value(QStringLiteral("a"))));
        QVERIFY(std::isfinite(g.value(QStringLiteral("b"))));
        QVERIFY(std::isfinite(g.value(QStringLiteral("self"))));
        // A chain far longer than any drawing has: it ends, and gives a number.
        g.define(QStringLiteral("g0"), QStringLiteral("val 1"));
        for (int i = 1; i < 20000; ++i) g.define(QStringLiteral("g%1").arg(i), QStringLiteral("+- g%1 1 0").arg(i - 1));
        QVERIFY(std::isfinite(g.value(QStringLiteral("g19999"))));
        // Results are kept finite and of a size a path can use.
        g.define(QStringLiteral("big"), QStringLiteral("*/ 1000000000000 1000000000000 1"));
        QVERIFY(std::isfinite(g.value(QStringLiteral("big"))));
        QVERIFY(qAbs(g.value(QStringLiteral("big"))) <= 1e15);
        g.define(QStringLiteral("tan90"), QStringLiteral("tan 100 5400000"));
        QVERIFY(std::isfinite(g.value(QStringLiteral("tan90"))));
        g.define(QStringLiteral("nanner"), QStringLiteral("mod nan 1 1"));
        QVERIFY(std::isfinite(g.value(QStringLiteral("nanner"))));
    }

    void dmlCustomGeometryScalesItsPath()
    {
        // The path's own 100 by 50 is stretched over a frame of 200 by 100 points.
        const QString p = linePath(QStringLiteral("w=\"100\" h=\"50\""), {{"10", "10"}, {"90", "10"}, {"90", "40"}});
        const Geometry g = customGeometryOf(QStringLiteral("<a:pathLst>%1</a:pathLst>").arg(p), QSizeF(200, 100));
        QCOMPARE(g.paths.size(), 1);
        const QRectF r = g.paths[0].path.boundingRect();
        QVERIFY2(near(r.left(), 20) && near(r.top(), 20) && near(r.right(), 180) && near(r.bottom(), 80), qPrintable(QString::number(r.left())));
        QVERIFY(g.paths[0].fill == GeometryPath::Normal);
        QVERIFY(g.paths[0].stroke);
    }

    void dmlCustomGeometryWithoutPathSizeIsInEmu()
    {
        const QString p = linePath(QString(), {{"0", "0"}, {QStringLiteral("2540000"), QStringLiteral("1270000")}}, false);
        const Geometry g = customGeometryOf(QStringLiteral("<a:pathLst>%1</a:pathLst>").arg(p), QSizeF(300, 300));
        QCOMPARE(g.paths.size(), 1);
        const QRectF r = g.paths[0].path.boundingRect();
        QVERIFY(near(r.right(), 200) && near(r.bottom(), 100));
        // The frame's size doesn't stretch a path that names none.
        const Geometry g2 = customGeometryOf(QStringLiteral("<a:pathLst>%1</a:pathLst>").arg(p), QSizeF(30, 30));
        QVERIFY(near(g2.paths[0].path.boundingRect().right(), 200));
    }

    void dmlCustomGeometryEvaluatesGuides()
    {
        // A triangle whose apex is a quarter of the way along, by guides.
        const QString gd = QStringLiteral("<a:avLst><a:gd name=\"adj\" fmla=\"val 25000\"/></a:avLst>"
                                          "<a:gdLst><a:gd name=\"x1\" fmla=\"*/ w adj 100000\"/><a:gd name=\"y1\" fmla=\"*/ h 1 2\"/></a:gdLst>");
        const QString p = linePath(QString(), {{"l", "b"}, {"x1", "t"}, {"r", "y1"}});
        const Geometry g = customGeometryOf(gd + QStringLiteral("<a:pathLst>%1</a:pathLst>").arg(p), QSizeF(400, 200));
        QCOMPARE(g.paths.size(), 1);
        const QPainterPath &path = g.paths[0].path;
        QVERIFY(near(path.elementAt(0).x, 0) && near(path.elementAt(0).y, 200));
        QVERIFY(near(path.elementAt(1).x, 100) && near(path.elementAt(1).y, 0));
        QVERIFY(near(path.elementAt(2).x, 400) && near(path.elementAt(2).y, 100));
    }

    void dmlCustomGeometryDrawsArcsAndCurves()
    {
        // A whole circle from an arc: left-center, once around.
        const QString circle = QStringLiteral("<a:path><a:moveTo><a:pt x=\"l\" y=\"vc\"/></a:moveTo>"
                                              "<a:arcTo wR=\"wd2\" hR=\"hd2\" stAng=\"cd2\" swAng=\"21600000\"/><a:close/></a:path>");
        Geometry g = customGeometryOf(QStringLiteral("<a:pathLst>%1</a:pathLst>").arg(circle), QSizeF(100, 100));
        QCOMPARE(g.paths.size(), 1);
        QRectF r = g.paths[0].path.boundingRect();
        QVERIFY2(near(r.left(), 0, 1e-3) && near(r.top(), 0, 1e-3) && near(r.right(), 100, 1e-3) && near(r.bottom(), 100, 1e-3),
                 qPrintable(QStringLiteral("%1,%2 %3x%4").arg(r.left()).arg(r.top()).arg(r.width()).arg(r.height())));
        // The center of the circle is inside it and a corner is not.
        QVERIFY(g.paths[0].path.contains(QPointF(50, 50)));
        QVERIFY(!g.paths[0].path.contains(QPointF(3, 3)));
        // A quarter ellipse: from the left-center sweeping a quarter turn clockwise ends at the top-center.
        const QString quarter = QStringLiteral("<a:path><a:moveTo><a:pt x=\"l\" y=\"vc\"/></a:moveTo>"
                                               "<a:arcTo wR=\"wd2\" hR=\"hd2\" stAng=\"cd2\" swAng=\"5400000\"/></a:path>");
        g = customGeometryOf(QStringLiteral("<a:pathLst>%1</a:pathLst>").arg(quarter), QSizeF(300, 100));
        const QPointF end = g.paths[0].path.currentPosition();
        QVERIFY2(near(end.x(), 150, 1e-3) && near(end.y(), 0, 1e-3), qPrintable(QStringLiteral("%1,%2").arg(end.x()).arg(end.y())));
        // A counterclockwise quarter turn ends at the bottom-center.
        const QString back = QStringLiteral("<a:path><a:moveTo><a:pt x=\"l\" y=\"vc\"/></a:moveTo>"
                                            "<a:arcTo wR=\"wd2\" hR=\"hd2\" stAng=\"cd2\" swAng=\"-5400000\"/></a:path>");
        g = customGeometryOf(QStringLiteral("<a:pathLst>%1</a:pathLst>").arg(back), QSizeF(300, 100));
        QVERIFY(near(g.paths[0].path.currentPosition().x(), 150, 1e-3) && near(g.paths[0].path.currentPosition().y(), 100, 1e-3));
        // Quadratic and cubic curves end where they are told to.
        const QString curves = QStringLiteral("<a:path w=\"10\" h=\"10\"><a:moveTo><a:pt x=\"0\" y=\"0\"/></a:moveTo>"
                                              "<a:quadBezTo><a:pt x=\"5\" y=\"0\"/><a:pt x=\"10\" y=\"5\"/></a:quadBezTo>"
                                              "<a:cubicBezTo><a:pt x=\"10\" y=\"8\"/><a:pt x=\"8\" y=\"10\"/><a:pt x=\"5\" y=\"10\"/></a:cubicBezTo></a:path>");
        g = customGeometryOf(QStringLiteral("<a:pathLst>%1</a:pathLst>").arg(curves), QSizeF(100, 100));
        QVERIFY(near(g.paths[0].path.currentPosition().x(), 50) && near(g.paths[0].path.currentPosition().y(), 100));
        QVERIFY(g.paths[0].path.elementCount() >= 6);
    }

    void dmlCustomGeometryKeepsFillAndStrokeModes()
    {
        const QString sq = QStringLiteral("<a:moveTo><a:pt x=\"0\" y=\"0\"/></a:moveTo><a:lnTo><a:pt x=\"10\" y=\"0\"/></a:lnTo><a:lnTo><a:pt x=\"10\" y=\"10\"/></a:lnTo>");
        const QString paths = QStringLiteral("<a:pathLst><a:path w=\"10\" h=\"10\">%1</a:path><a:path w=\"10\" h=\"10\" fill=\"none\">%1</a:path>"
                                             "<a:path w=\"10\" h=\"10\" stroke=\"0\">%1</a:path><a:path w=\"10\" h=\"10\" fill=\"darkenLess\" stroke=\"false\">%1</a:path>"
                                             "<a:path w=\"10\" h=\"10\" fill=\"lighten\">%1</a:path><a:path w=\"10\" h=\"10\" fill=\"lightenLess\">%1</a:path>"
                                             "<a:path w=\"10\" h=\"10\" fill=\"darken\">%1</a:path></a:pathLst>").arg(sq);
        const Geometry g = customGeometryOf(paths, QSizeF(10, 10));
        QCOMPARE(g.paths.size(), 7);
        QVERIFY(g.paths[0].fill == GeometryPath::Normal && g.paths[0].stroke);
        QVERIFY(g.paths[1].fill == GeometryPath::None && g.paths[1].stroke);
        QVERIFY(g.paths[2].fill == GeometryPath::Normal && !g.paths[2].stroke);
        QVERIFY(g.paths[3].fill == GeometryPath::DarkenLess && !g.paths[3].stroke);
        QVERIFY(g.paths[4].fill == GeometryPath::Lighten);
        QVERIFY(g.paths[5].fill == GeometryPath::LightenLess);
        QVERIFY(g.paths[6].fill == GeometryPath::Darken);
    }

    void dmlCustomGeometryReadsTheTextRectangle()
    {
        const QString p = linePath(QStringLiteral("w=\"10\" h=\"10\""), {{"0", "0"}, {"10", "0"}, {"10", "10"}});
        Geometry g;
        g = customGeometryOf(QStringLiteral("<a:rect l=\"wd4\" t=\"hd4\" r=\"r\" b=\"b\"/><a:pathLst>%1</a:pathLst>").arg(p), QSizeF(200, 100));
        QVERIFY2(near(g.textRect.left(), 50) && near(g.textRect.top(), 25) && near(g.textRect.right(), 200) && near(g.textRect.bottom(), 100),
                 qPrintable(QStringLiteral("%1,%2 %3x%4").arg(g.textRect.left()).arg(g.textRect.top()).arg(g.textRect.width()).arg(g.textRect.height())));
        g = customGeometryOf(QStringLiteral("<a:pathLst>%1</a:pathLst>").arg(p), QSizeF(200, 100));
        QVERIFY(g.textRect.isNull());
    }

    void dmlCustomGeometryToleratesNonsense()
    {
        QVERIFY(customGeometryOf(QString(), QSizeF(10, 10)).isEmpty());
        QVERIFY(customGeometryOf(QStringLiteral("<a:pathLst/>"), QSizeF(10, 10)).isEmpty());
        QVERIFY(readCustomGeometry(QDomElement(), QSizeF(10, 10)).isEmpty());
        // Garbage coordinates are 0, a path with no points is skipped, a zero-size frame has nothing.
        Geometry g = customGeometryOf(QStringLiteral("<a:pathLst><a:path w=\"10\" h=\"10\"><a:moveTo><a:pt x=\"zz\" y=\"\"/></a:moveTo>"
                                                     "<a:lnTo/><a:lnTo><a:pt x=\"5\"/></a:lnTo><a:bogus/></a:path><a:path/></a:pathLst>"), QSizeF(10, 10));
        QVERIFY(g.paths.size() <= 1);
        if (!g.paths.isEmpty()) QVERIFY(g.paths[0].path.boundingRect().width() >= 0);
        const QString p = linePath(QStringLiteral("w=\"10\" h=\"10\""), {{"0", "0"}, {"10", "10"}});
        QVERIFY(customGeometryOf(QStringLiteral("<a:pathLst>%1</a:pathLst>").arg(p), QSizeF(0, 0)).isEmpty() ||
                customGeometryOf(QStringLiteral("<a:pathLst>%1</a:pathLst>").arg(p), QSizeF(0, 0)).paths[0].path.boundingRect().isEmpty());
    }

    void dmlCustomGeometryLimitsHostilePaths()
    {
        // Far more commands than a drawing has are cut off, and the points stay where a page can hold them.
        QString many = QStringLiteral("<a:path w=\"100\" h=\"100\"><a:moveTo><a:pt x=\"0\" y=\"0\"/></a:moveTo>");
        for (int i = 0; i < 150000; ++i) many += QStringLiteral("<a:lnTo><a:pt x=\"%1\" y=\"5\"/></a:lnTo>").arg(i % 100);
        many += QStringLiteral("</a:path>");
        Geometry g = customGeometryOf(QStringLiteral("<a:pathLst>%1</a:pathLst>").arg(many), QSizeF(100, 100));
        QVERIFY(g.paths.isEmpty() || g.paths[0].path.elementCount() <= 100000);
        const QString far = linePath(QString(), {{"0", "0"}, {QStringLiteral("99999999999999999"), QStringLiteral("-99999999999999999")}}, false);
        g = customGeometryOf(QStringLiteral("<a:pathLst>%1</a:pathLst>").arg(far), QSizeF(100, 100));
        QVERIFY(g.paths.isEmpty() || g.paths[0].path.boundingRect().right() <= 1e6);
        QVERIFY(g.paths.isEmpty() || g.paths[0].path.boundingRect().top() >= -1e6);
        // An arc with radii of nothing, or of billions, neither hangs nor makes a bad number.
        const QString arcs = QStringLiteral("<a:path><a:moveTo><a:pt x=\"0\" y=\"0\"/></a:moveTo><a:arcTo wR=\"0\" hR=\"0\" stAng=\"0\" swAng=\"5400000\"/>"
                                            "<a:arcTo wR=\"99999999999\" hR=\"99999999999\" stAng=\"0\" swAng=\"99999999999\"/></a:path>");
        g = customGeometryOf(QStringLiteral("<a:pathLst>%1</a:pathLst>").arg(arcs), QSizeF(100, 100));
        for (const GeometryPath &p : g.paths)
            for (int i = 0; i < p.path.elementCount(); ++i) QVERIFY(std::isfinite(p.path.elementAt(i).x) && std::isfinite(p.path.elementAt(i).y));
        // Too many guides are ignored from the limit on.
        QString gds;
        for (int i = 0; i < 20000; ++i) gds += QStringLiteral("<a:gd name=\"g%1\" fmla=\"val %1\"/>").arg(i);
        g = customGeometryOf(QStringLiteral("<a:gdLst>%1</a:gdLst><a:pathLst>%2</a:pathLst>").arg(gds, linePath(QString(), {{"g1", "g2"}, {"g3", "g4"}})), QSizeF(100, 100));
        QVERIFY(g.paths.size() <= 1);
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
                                   QStringLiteral("/a//b.xml"), QStringLiteral("/100%.xml"), QStringLiteral("/[Content_Types].xml"),
                                   QStringLiteral("/_rels/.rels")}) {
            QTest::ignoreMessage(QtWarningMsg, QRegularExpression(QStringLiteral("PackageWriter")));
            w.addPart(bad, "x", QStringLiteral("application/xml"));
        }
        Package p;
        QVERIFY(p.read(w.finish()));
        QCOMPARE(p.partNames(), QStringList({QStringLiteral("/ok.xml")}));
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

QTEST_MAIN(FormatTests)
#include "tst_office.moc"
