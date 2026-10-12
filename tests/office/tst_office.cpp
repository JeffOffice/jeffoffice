// Tests of libs/office: packages, the theme, and DrawingML colors. Every
// package they read is built in memory (never a real-world file).
#include "office/drawingml.h"
#include "office/drawingml_color.h"
#include "office/drawingml_geometry.h"
#include "office/package.h"
#include "office/theme.h"
#include "office/xml.h"
#include "io/zip.h"
#include "render/shapes.h"

#include <QBuffer>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QTextDocument>
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

    using Arrow = jp::Arrow;
    using ColorRef = jp::ColorRef;
    using Effects = jp::Effects;
    using Fill = jp::Fill;
    using GroupItem = jp::GroupItem;
    using Item = jp::Item;
    using ItemList = jp::ItemList;
    using ItemPtr = jp::ItemPtr;
    using ItemType = jp::ItemType;
    using LineItem = jp::LineItem;
    using PictureItem = jp::PictureItem;
    using ShapeItem = jp::ShapeItem;
    using Stroke = jp::Stroke;
    using TextItem = jp::TextItem;
    using VAlign = jp::VAlign;
    using Wrap = jp::Wrap;

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

    // A small format scheme, as a theme holds: each list has the three styles the references 1 to 3 (and 1001 to 1003) pick.
    static QString dmlFormats()
    {
        return QStringLiteral(
            "<a:fmtScheme name=\"T\"><a:fillStyleLst>"
            "<a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill>"
            "<a:gradFill rotWithShape=\"1\"><a:gsLst><a:gs pos=\"0\"><a:schemeClr val=\"phClr\"><a:tint val=\"50000\"/></a:schemeClr></a:gs>"
            "<a:gs pos=\"100000\"><a:schemeClr val=\"phClr\"/></a:gs></a:gsLst><a:lin ang=\"5400000\" scaled=\"0\"/></a:gradFill>"
            "<a:solidFill><a:schemeClr val=\"phClr\"><a:lumMod val=\"50000\"/></a:schemeClr></a:solidFill></a:fillStyleLst>"
            "<a:lnStyleLst>"
            "<a:ln w=\"6350\" cap=\"flat\" cmpd=\"sng\" algn=\"ctr\"><a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill><a:prstDash val=\"solid\"/><a:miter lim=\"800000\"/></a:ln>"
            "<a:ln w=\"12700\" cap=\"flat\" cmpd=\"sng\" algn=\"ctr\"><a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill><a:prstDash val=\"dash\"/><a:round/></a:ln>"
            "<a:ln w=\"19050\" cap=\"rnd\" cmpd=\"dbl\" algn=\"ctr\"><a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill><a:prstDash val=\"solid\"/><a:bevel/></a:ln>"
            "</a:lnStyleLst><a:effectStyleLst>"
            "<a:effectStyle><a:effectLst/></a:effectStyle><a:effectStyle><a:effectLst/></a:effectStyle>"
            "<a:effectStyle><a:effectLst><a:outerShdw blurRad=\"57150\" dist=\"19050\" dir=\"5400000\" algn=\"ctr\" rotWithShape=\"0\">"
            "<a:srgbClr val=\"000000\"><a:alpha val=\"63000\"/></a:srgbClr></a:outerShdw></a:effectLst></a:effectStyle></a:effectStyleLst>"
            "<a:bgFillStyleLst><a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill>"
            "<a:solidFill><a:schemeClr val=\"phClr\"><a:tint val=\"95000\"/></a:schemeClr></a:solidFill>"
            "<a:gradFill><a:gsLst><a:gs pos=\"0\"><a:schemeClr val=\"phClr\"/></a:gs><a:gs pos=\"100000\"><a:schemeClr val=\"phClr\"><a:shade val=\"80000\"/></a:schemeClr></a:gs></a:gsLst>"
            "<a:path path=\"circle\"><a:fillToRect l=\"50000\" t=\"50000\" r=\"50000\" b=\"50000\"/></a:path></a:gradFill></a:bgFillStyleLst></a:fmtScheme>");
    }

    // A 4 by 4 picture of one color, as PNG bytes.
    static QByteArray dmlPicture(const QColor &color = Qt::red)
    {
        QImage img(4, 4, QImage::Format_RGB32);
        img.fill(color);
        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        img.save(&buffer, "PNG");
        return bytes;
    }

    // What reading a drawing is given, with a record of what it asks for: the pictures it keeps and the stories it starts.
    struct DmlEnv {
        struct Image { QByteArray bytes; QString format; };
        DrawingContext ctx;
        Package package;
        QVector<Image> images;
        QVector<std::shared_ptr<QTextDocument>> stories;
        QVector<std::shared_ptr<QDomDocument>> documents;

        DmlEnv()
        {
            ctx.theme = colorTestTheme();
            ctx.addImage = [this](const QByteArray &bytes, const QString &format) {
                images.append({bytes, format});
                return QStringLiteral("img%1").arg(images.size());
            };
            ctx.newStory = [this]() {
                stories.append(std::make_shared<QTextDocument>());
                return QStringLiteral("story%1").arg(stories.size());
            };
            ctx.storyDoc = [this](const QString &id) -> QTextDocument * {
                const int n = id.mid(5).toInt();
                return n >= 1 && n <= stories.size() ? stories[n - 1].get() : nullptr;
            };
        }
        DmlEnv(const DmlEnv &) = delete;
        DmlEnv &operator=(const DmlEnv &) = delete;

        // The first element in `xml`, with a, p, and r declared; it stays valid as long as this does.
        QDomElement element(const QString &xml)
        {
            auto doc = std::make_shared<QDomDocument>();
            documents.append(doc);
            const QByteArray text = QStringLiteral("<root %1>%2</root>").arg(dmlNamespaces(), xml).toUtf8();
            if (!parseXml(text, doc.get())) return QDomElement();
            return doc->documentElement().firstChildElement();
        }
        void useFormats() { ctx.formats = element(dmlFormats()); }
        // A package whose slide part has a picture (rId1) and an outside link (rId2, to `link`).
        void usePackage(const QByteArray &picture = dmlPicture(), const QString &link = QStringLiteral("https://example.com/x"))
        {
            PackageWriter w;
            w.addDefault(QStringLiteral("png"), QStringLiteral("image/png"));
            w.addPart(QStringLiteral("/ppt/slides/slide1.xml"), "<p:sld/>", kSlideType);
            w.addPart(QStringLiteral("/ppt/media/image1.png"), picture, QStringLiteral("image/png"));
            w.addRelationship(QStringLiteral("/ppt/slides/slide1.xml"), kRelBase + "image", QStringLiteral("/ppt/media/image1.png"));
            w.addRelationship(QStringLiteral("/ppt/slides/slide1.xml"), kRelBase + "hyperlink", link, true);
            package.read(w.finish());
            ctx.package = &package;
            ctx.part = QStringLiteral("/ppt/slides/slide1.xml");
        }
    };

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

    // ---- DrawingML shapes: preset geometry

private:
    static QStringList dmlAllPresets()
    {
        return QStringLiteral(
            "line lineInv triangle rtTriangle rect diamond parallelogram trapezoid nonIsoscelesTrapezoid pentagon hexagon heptagon octagon decagon "
            "dodecagon star4 star5 star6 star7 star8 star10 star12 star16 star24 star32 roundRect round1Rect round2SameRect round2DiagRect "
            "snipRoundRect snip1Rect snip2SameRect snip2DiagRect plaque ellipse teardrop homePlate chevron pieWedge pie blockArc donut noSmoking "
            "rightArrow leftArrow upArrow downArrow stripedRightArrow notchedRightArrow bentUpArrow leftRightArrow upDownArrow leftUpArrow "
            "leftRightUpArrow quadArrow leftArrowCallout rightArrowCallout upArrowCallout downArrowCallout leftRightArrowCallout upDownArrowCallout "
            "quadArrowCallout bentArrow uturnArrow circularArrow leftCircularArrow leftRightCircularArrow curvedRightArrow curvedLeftArrow "
            "curvedUpArrow curvedDownArrow swooshArrow cube can lightningBolt heart sun moon smileyFace irregularSeal1 irregularSeal2 foldedCorner "
            "bevel frame halfFrame corner diagStripe chord arc leftBracket rightBracket leftBrace rightBrace bracketPair bracePair straightConnector1 "
            "bentConnector2 bentConnector3 bentConnector4 bentConnector5 curvedConnector2 curvedConnector3 curvedConnector4 curvedConnector5 "
            "callout1 callout2 callout3 accentCallout1 accentCallout2 accentCallout3 borderCallout1 borderCallout2 borderCallout3 "
            "accentBorderCallout1 accentBorderCallout2 accentBorderCallout3 wedgeRectCallout wedgeRoundRectCallout wedgeEllipseCallout cloudCallout "
            "cloud ribbon ribbon2 ellipseRibbon ellipseRibbon2 leftRightRibbon verticalScroll horizontalScroll wave doubleWave plus "
            "flowChartProcess flowChartAlternateProcess flowChartDecision flowChartInputOutput flowChartPredefinedProcess flowChartInternalStorage "
            "flowChartDocument flowChartMultidocument flowChartTerminator flowChartPreparation flowChartManualInput flowChartManualOperation "
            "flowChartConnector flowChartOffpageConnector flowChartPunchedCard flowChartPunchedTape flowChartSummingJunction flowChartOr "
            "flowChartCollate flowChartSort flowChartExtract flowChartMerge flowChartOfflineStorage flowChartOnlineStorage flowChartMagneticTape "
            "flowChartMagneticDisk flowChartMagneticDrum flowChartDisplay flowChartDelay actionButtonBlank actionButtonHome actionButtonHelp "
            "actionButtonInformation actionButtonForwardNext actionButtonBackPrevious actionButtonEnd actionButtonBeginning actionButtonReturn "
            "actionButtonDocument actionButtonSound actionButtonMovie gear6 gear9 funnel mathPlus mathMinus mathMultiply mathDivide mathEqual "
            "mathNotEqual cornerTabs squareTabs plaqueTabs chartX chartStar chartPlus").split(QLatin1Char(' '), Qt::SkipEmptyParts);
    }

    // The presets that no shape and no definition here draws as they should be: the ones the report lists.
    static QStringList dmlUnsupportedPresets()
    {
        return QStringLiteral("leftRightUpArrow quadArrowCallout leftCircularArrow leftRightCircularArrow swooshArrow "
                              "leftRightRibbon gear6 gear9 funnel cornerTabs squareTabs plaqueTabs chartX chartStar chartPlus").split(QLatin1Char(' '));
    }

private Q_SLOTS:
    void dmlPresetNamesMapToTheShapeLibrary_data()
    {
        QTest::addColumn<QString>("preset");
        QTest::addColumn<QString>("shape");
        const struct { const char *preset, *shape; } map[] = {
            {"rect", "rect"}, {"roundRect", "roundRect"}, {"ellipse", "ellipse"}, {"triangle", "triangle"}, {"rtTriangle", "rtTriangle"},
            {"diamond", "diamond"}, {"parallelogram", "parallelogram"}, {"trapezoid", "trapezoid"}, {"pentagon", "pentagon"},
            {"hexagon", "hexagon"}, {"heptagon", "heptagon"}, {"octagon", "octagon"}, {"decagon", "decagon"}, {"dodecagon", "dodecagon"},
            {"snip1Rect", "snip1"}, {"snip2SameRect", "snip2same"}, {"snip2DiagRect", "snip2diag"}, {"snipRoundRect", "snipRound"},
            {"round1Rect", "round1"}, {"round2SameRect", "round2same"}, {"round2DiagRect", "round2diag"},
            {"plaque", "plaque"}, {"teardrop", "teardrop"}, {"homePlate", "homePlate"}, {"chevron", "chevron"}, {"pie", "pie"}, {"chord", "chord"},
            {"blockArc", "blockArc"}, {"arc", "arc"}, {"donut", "donut"}, {"noSmoking", "noSmoking"}, {"frame", "frame"}, {"halfFrame", "halfFrame"},
            {"corner", "corner"}, {"diagStripe", "diagStripe"}, {"plus", "plus"}, {"can", "can"}, {"cube", "cube"}, {"bevel", "bevel"},
            {"foldedCorner", "foldedCorner"}, {"smileyFace", "smiley"}, {"heart", "heart"}, {"lightningBolt", "lightning"}, {"sun", "sun"},
            {"moon", "moon"}, {"cloud", "cloud"}, {"bracketPair", "bracketPair"}, {"bracePair", "bracePair"}, {"leftBracket", "leftBracket"},
            {"rightBracket", "rightBracket"}, {"leftBrace", "leftBrace"}, {"rightBrace", "rightBrace"},
            {"rightArrow", "rightArrow"}, {"leftArrow", "leftArrow"}, {"upArrow", "upArrow"}, {"downArrow", "downArrow"},
            {"leftRightArrow", "leftRightArrow"}, {"upDownArrow", "upDownArrow"}, {"quadArrow", "quadArrow"}, {"bentArrow", "bentArrow"},
            {"uturnArrow", "uturnArrow"}, {"leftUpArrow", "leftUpArrow"}, {"bentUpArrow", "bentUpArrow"}, {"curvedRightArrow", "curvedRightArrow"},
            {"curvedLeftArrow", "curvedLeftArrow"}, {"curvedUpArrow", "curvedUpArrow"}, {"curvedDownArrow", "curvedDownArrow"},
            {"stripedRightArrow", "stripedRightArrow"}, {"notchedRightArrow", "notchedRightArrow"}, {"rightArrowCallout", "rightArrowCallout"},
            {"downArrowCallout", "downArrowCallout"}, {"circularArrow", "circularArrow"},
            {"mathPlus", "mathPlus"}, {"mathMinus", "mathMinus"}, {"mathMultiply", "mathMultiply"}, {"mathDivide", "mathDivide"},
            {"mathEqual", "mathEqual"}, {"mathNotEqual", "mathNotEqual"},
            {"flowChartProcess", "fcProcess"}, {"flowChartAlternateProcess", "fcAltProcess"}, {"flowChartDecision", "fcDecision"},
            {"flowChartInputOutput", "fcData"}, {"flowChartPredefinedProcess", "fcPredefined"}, {"flowChartInternalStorage", "fcInternalStorage"},
            {"flowChartDocument", "fcDocument"}, {"flowChartMultidocument", "fcMultidocument"}, {"flowChartTerminator", "fcTerminator"},
            {"flowChartPreparation", "fcPreparation"}, {"flowChartManualInput", "fcManualInput"}, {"flowChartManualOperation", "fcManualOperation"},
            {"flowChartConnector", "fcConnector"}, {"flowChartOffpageConnector", "fcOffpage"}, {"flowChartPunchedCard", "fcCard"},
            {"flowChartPunchedTape", "fcPunchedTape"}, {"flowChartSummingJunction", "fcSummingJunction"}, {"flowChartOr", "fcOr"},
            {"flowChartCollate", "fcCollate"}, {"flowChartSort", "fcSort"}, {"flowChartExtract", "fcExtract"}, {"flowChartMerge", "fcMerge"},
            {"flowChartOnlineStorage", "fcStoredData"}, {"flowChartDelay", "fcDelay"}, {"flowChartMagneticTape", "fcSequential"},
            {"flowChartMagneticDisk", "fcMagneticDisk"}, {"flowChartMagneticDrum", "fcDirectAccess"}, {"flowChartDisplay", "fcDisplay"},
            {"irregularSeal1", "irregularSeal1"}, {"irregularSeal2", "irregularSeal2"},
            {"star4", "star4"}, {"star5", "star5"}, {"star6", "star6"}, {"star7", "star7"}, {"star8", "star8"}, {"star10", "star10"},
            {"star12", "star12"}, {"star16", "star16"}, {"star24", "star24"}, {"star32", "star32"},
            {"ribbon", "ribbon"}, {"ribbon2", "ribbon2"}, {"ellipseRibbon", "ellipseRibbon"}, {"ellipseRibbon2", "ellipseRibbon2"},
            {"verticalScroll", "verticalScroll"}, {"horizontalScroll", "horizontalScroll"}, {"wave", "wave"}, {"doubleWave", "doubleWave"},
            {"wedgeRectCallout", "wedgeRectCallout"}, {"wedgeRoundRectCallout", "wedgeRoundRectCallout"}, {"wedgeEllipseCallout", "wedgeEllipseCallout"},
            {"borderCallout1", "borderCallout1"}, {"borderCallout2", "borderCallout2"}, {"accentCallout1", "accentCallout1"}, {"callout1", "callout1"},
            {"actionButtonBlank", "actionButtonBlank"}, {"actionButtonHome", "actionButtonBlank"}, {"actionButtonSound", "actionButtonBlank"}};
        for (const auto &m : map) QTest::newRow(m.preset) << QString::fromLatin1(m.preset) << QString::fromLatin1(m.shape);
    }

    void dmlPresetNamesMapToTheShapeLibrary()
    {
        QFETCH(QString, preset);
        QFETCH(QString, shape);
        QCOMPARE(shapeForPreset(preset), shape);
        QVERIFY2(jp::shapeDef(shape) != nullptr, qPrintable(shape));
    }

    void dmlEveryShapeTheLibraryDrawsHasAPreset()
    {
        // The library holds two shapes the standard doesn't name (a tab and a flow arrow);
        // every other has a preset that maps to it.
        QSet<QString> reached;
        for (const QString &prst : dmlAllPresets()) {
            const QString id = shapeForPreset(prst);
            if (!id.isEmpty()) {
                QVERIFY2(jp::shapeDef(id) != nullptr, qPrintable(prst));
                reached.insert(id);
            }
        }
        for (const jp::ShapeDef &d : jp::shapeLibrary())
            if (d.id != QLatin1String("tab") && d.id != QLatin1String("flowArrow")) QVERIFY2(reached.contains(d.id), qPrintable(d.id));
    }

    void dmlEveryPresetHasAShapeOrADefinitionOrIsListed()
    {
        const QStringList unsupported = dmlUnsupportedPresets();
        for (const QString &prst : dmlAllPresets()) {
            const bool line = prst == QLatin1String("line") || prst == QLatin1String("lineInv") || prst == QLatin1String("straightConnector1");
            const bool drawn = !shapeForPreset(prst).isEmpty() || hasPresetGeometry(prst) || line;
            QVERIFY2(drawn != unsupported.contains(prst), qPrintable(prst));
        }
        QVERIFY(shapeForPreset(QStringLiteral("noSuchShape")).isEmpty());
        QVERIFY(shapeForPreset(QString()).isEmpty());
        QVERIFY(shapeForPreset(QStringLiteral("Rect")).isEmpty());   // names are case sensitive
    }

private:
    static QHash<QString, double> dmlAdjust(const QString &list)
    {
        QHash<QString, double> out;
        for (const QString &item : list.split(QLatin1Char(';'), Qt::SkipEmptyParts)) out.insert(item.section(QLatin1Char('='), 0, 0), item.section(QLatin1Char('='), 1).toDouble());
        return out;
    }

private Q_SLOTS:
    void dmlLibraryAdjustments_data()
    {
        QTest::addColumn<QString>("preset");
        QTest::addColumn<QString>("given");
        QTest::addColumn<double>("width");
        QTest::addColumn<double>("height");
        QTest::addColumn<QList<double>>("expected");
        using L = QList<double>;
        QTest::newRow("roundRect default") << "roundRect" << "" << 100.0 << 50.0 << L{0.16667};
        QTest::newRow("roundRect half") << "roundRect" << "adj=50000" << 100.0 << 50.0 << L{0.5};
        QTest::newRow("roundRect past half") << "roundRect" << "adj=75000" << 100.0 << 50.0 << L{0.5};
        QTest::newRow("roundRect negative") << "roundRect" << "adj=-100" << 100.0 << 50.0 << L{0.0};
        QTest::newRow("roundRect huge") << "roundRect" << "adj=999999999999" << 100.0 << 50.0 << L{0.5};
        QTest::newRow("triangle default") << "triangle" << "" << 100.0 << 50.0 << L{0.5};
        QTest::newRow("triangle apex at the left") << "triangle" << "adj=0" << 100.0 << 50.0 << L{0.0};
        QTest::newRow("triangle apex at the right") << "triangle" << "adj=100000" << 100.0 << 50.0 << L{1.0};
        QTest::newRow("parallelogram wide") << "parallelogram" << "" << 100.0 << 50.0 << L{0.125};
        QTest::newRow("parallelogram tall") << "parallelogram" << "" << 50.0 << 100.0 << L{0.25};
        QTest::newRow("trapezoid") << "trapezoid" << "" << 100.0 << 50.0 << L{0.125};
        QTest::newRow("trapezoid set") << "trapezoid" << "adj=50000" << 200.0 << 100.0 << L{0.25};
        QTest::newRow("hexagon") << "hexagon" << "" << 100.0 << 50.0 << L{0.125};
        QTest::newRow("octagon") << "octagon" << "" << 100.0 << 50.0 << L{0.29289};
        QTest::newRow("plus") << "plus" << "" << 100.0 << 50.0 << L{0.25};
        QTest::newRow("can") << "can" << "adj=40000" << 100.0 << 50.0 << L{0.4};
        QTest::newRow("cube") << "cube" << "" << 100.0 << 50.0 << L{0.25};
        QTest::newRow("bevel") << "bevel" << "" << 100.0 << 50.0 << L{0.125};
        QTest::newRow("donut") << "donut" << "" << 100.0 << 50.0 << L{0.25};
        QTest::newRow("frame") << "frame" << "" << 100.0 << 50.0 << L{0.125};
        QTest::newRow("foldedCorner") << "foldedCorner" << "" << 100.0 << 50.0 << L{0.16667};
        QTest::newRow("chevron") << "chevron" << "" << 100.0 << 50.0 << L{0.5};
        QTest::newRow("homePlate") << "homePlate" << "adj=30000" << 100.0 << 50.0 << L{0.3};
        QTest::newRow("rightArrow default") << "rightArrow" << "" << 100.0 << 50.0 << L{0.5, 0.5};
        QTest::newRow("rightArrow set") << "rightArrow" << "adj1=25000;adj2=75000" << 100.0 << 50.0 << L{0.25, 0.75};
        QTest::newRow("leftArrow") << "leftArrow" << "adj1=30000" << 100.0 << 50.0 << L{0.3, 0.5};
        QTest::newRow("upArrow") << "upArrow" << "adj2=20000" << 100.0 << 50.0 << L{0.5, 0.2};
        QTest::newRow("leftRightArrow") << "leftRightArrow" << "" << 100.0 << 50.0 << L{0.5, 0.5};
        QTest::newRow("quadArrow") << "quadArrow" << "" << 100.0 << 100.0 << L{0.1125, 0.225, 0.225};
        QTest::newRow("star4") << "star4" << "" << 100.0 << 100.0 << L{0.125};
        QTest::newRow("star5") << "star5" << "" << 100.0 << 100.0 << L{0.38196};
        QTest::newRow("star5 set") << "star5" << "adj=25000" << 100.0 << 100.0 << L{0.5};
        QTest::newRow("star6") << "star6" << "" << 100.0 << 100.0 << L{0.57736};
        QTest::newRow("star24") << "star24" << "" << 100.0 << 100.0 << L{0.75};
        QTest::newRow("mathPlus") << "mathPlus" << "" << 100.0 << 100.0 << L{0.2352};
        QTest::newRow("wave") << "wave" << "" << 100.0 << 100.0 << L{0.125};
        QTest::newRow("doubleWave") << "doubleWave" << "" << 100.0 << 100.0 << L{0.0625};
        QTest::newRow("verticalScroll") << "verticalScroll" << "" << 100.0 << 100.0 << L{0.125};
        QTest::newRow("smileyFace") << "smileyFace" << "" << 100.0 << 100.0 << L{0.04653};
        QTest::newRow("sun") << "sun" << "" << 100.0 << 100.0 << L{0.25};
        QTest::newRow("moon") << "moon" << "" << 100.0 << 100.0 << L{0.5};
        QTest::newRow("snip1Rect") << "snip1Rect" << "adj=30000" << 100.0 << 50.0 << L{0.3};
        QTest::newRow("round2DiagRect") << "round2DiagRect" << "" << 100.0 << 50.0 << L{0.16667};
        QTest::newRow("a name the preset doesn't use") << "roundRect" << "adj9=5" << 100.0 << 50.0 << L{0.16667};
        QTest::newRow("no adjustments: rect") << "rect" << "adj=5" << 100.0 << 50.0 << L{};
        QTest::newRow("no adjustments: ellipse") << "ellipse" << "" << 100.0 << 50.0 << L{};
        QTest::newRow("no such preset") << "noSuchShape" << "adj=5" << 100.0 << 50.0 << L{};
    }

    void dmlLibraryAdjustments()
    {
        QFETCH(QString, preset);
        QFETCH(QString, given);
        QFETCH(double, width);
        QFETCH(double, height);
        QFETCH(QList<double>, expected);
        const QVector<double> got = libraryAdjustments(preset, dmlAdjust(given), QSizeF(width, height));
        QCOMPARE(got.size(), expected.size());
        for (int i = 0; i < expected.size(); ++i) QVERIFY2(qAbs(got[i] - expected[i]) < 1e-4, qPrintable(QStringLiteral("%1: %2 not %3").arg(i).arg(got[i]).arg(expected[i])));
    }

    void dmlLibraryAdjustmentsStayInRange()
    {
        // Whatever a file says, a library shape gets numbers in the range its handle has.
        const QList<double> hostile = {-1e18, -100000, -1, 0, 1, 99999, 100000, 250000, 1e18};
        for (const QString &prst : dmlAllPresets()) {
            const QString id = shapeForPreset(prst);
            const jp::ShapeDef *def = jp::shapeDef(id);
            if (!def) continue;
            for (double v : hostile) {
                QHash<QString, double> given;
                for (const char *name : {"adj", "adj1", "adj2", "adj3", "adj4", "adj5"}) given.insert(QString::fromLatin1(name), v);
                const QVector<double> adj = libraryAdjustments(prst, given, QSizeF(300, 120));
                QVERIFY2(adj.size() <= def->defaults.size() + 1, qPrintable(prst));
                for (double a : adj) QVERIFY2(std::isfinite(a) && a >= -1 && a <= 3, qPrintable(QStringLiteral("%1 %2 %3").arg(prst).arg(v).arg(a)));
                // And the shape can be drawn with them (a shape with a thickness of nothing may show nothing).
                const QPainterPath path = jp::shapePath(id, QSizeF(300, 120), adj);
                for (int i = 0; i < path.elementCount(); ++i) QVERIFY2(std::isfinite(path.elementAt(i).x) && std::isfinite(path.elementAt(i).y), qPrintable(prst));
            }
        }
    }

    void dmlPieAndChordFollowTheirAngles()
    {
        // A pie from the standard's defaults is three quarters, clockwise from the right: the top right is missing.
        Geometry g = presetGeometry(QStringLiteral("pie"), {}, QSizeF(100, 100));
        QCOMPARE(g.paths.size(), 1);
        QVERIFY(g.paths[0].path.contains(QPointF(25, 25)));
        QVERIFY(g.paths[0].path.contains(QPointF(75, 75)));
        QVERIFY(g.paths[0].path.contains(QPointF(25, 75)));
        QVERIFY(!g.paths[0].path.contains(QPointF(75, 25)));
        // A quarter from the right to the bottom.
        g = presetGeometry(QStringLiteral("pie"), {{"adj1", 0}, {"adj2", 5400000}}, QSizeF(100, 100));
        QVERIFY(g.paths[0].path.contains(QPointF(75, 75)));
        QVERIFY(!g.paths[0].path.contains(QPointF(25, 25)));
        QVERIFY(!g.paths[0].path.contains(QPointF(75, 25)));
        // The same angles on a wide ellipse still bound the same quarter.
        g = presetGeometry(QStringLiteral("pie"), {{"adj1", 0}, {"adj2", 5400000}}, QSizeF(300, 100));
        QVERIFY(g.paths[0].path.contains(QPointF(225, 75)));
        QVERIFY(!g.paths[0].path.contains(QPointF(75, 25)));
        const QRectF b = g.paths[0].path.boundingRect();
        QVERIFY2(near(b.left(), 150, 1e-3) && near(b.top(), 50, 1e-3) && near(b.right(), 300, 1e-3) && near(b.bottom(), 100, 1e-3),
                 qPrintable(QStringLiteral("%1,%2 %3x%4").arg(b.left()).arg(b.top()).arg(b.width()).arg(b.height())));
        // A chord, from 45 to 270 degrees, is cut off by a straight line from the top to the lower right.
        g = presetGeometry(QStringLiteral("chord"), {}, QSizeF(100, 100));
        QVERIFY(g.paths[0].path.contains(QPointF(20, 50)));
        QVERIFY(!g.paths[0].path.contains(QPointF(80, 30)));
        // Equal start and end angles are nothing odd, and no number makes it fail.
        for (double v : {-1e18, -1.0, 0.0, 21600000.0, 1e18}) {
            g = presetGeometry(QStringLiteral("pie"), {{"adj1", v}, {"adj2", v}}, QSizeF(100, 100));
            QVERIFY(!g.paths.isEmpty());
        }
    }

    void dmlArcDrawsAnOpenArcAndItsSector()
    {
        // The standard's arc is a quarter from the top to the right; its sector is filled, only the arc stroked.
        const Geometry g = presetGeometry(QStringLiteral("arc"), {}, QSizeF(100, 100));
        QCOMPARE(g.paths.size(), 2);
        QVERIFY(g.paths[0].fill == GeometryPath::Normal && !g.paths[0].stroke);
        QVERIFY(g.paths[1].fill == GeometryPath::None && g.paths[1].stroke);
        const QRectF arc = g.paths[1].path.boundingRect();
        QVERIFY2(near(arc.left(), 50, 1e-3) && near(arc.top(), 0, 1e-3) && near(arc.right(), 100, 1e-3) && near(arc.bottom(), 50, 1e-3),
                 qPrintable(QStringLiteral("%1,%2 %3x%4").arg(arc.left()).arg(arc.top()).arg(arc.width()).arg(arc.height())));
        QVERIFY(near(g.paths[1].path.elementAt(0).x, 50, 1e-3) && near(g.paths[1].path.elementAt(0).y, 0, 1e-3));
        QVERIFY(near(g.paths[1].path.currentPosition().x(), 100, 1e-3) && near(g.paths[1].path.currentPosition().y(), 50, 1e-3));
    }

    void dmlBlockArcIsARingBand()
    {
        const Geometry g = presetGeometry(QStringLiteral("blockArc"), {}, QSizeF(100, 100));
        QCOMPARE(g.paths.size(), 1);
        const QPainterPath &p = g.paths[0].path;
        QVERIFY(p.contains(QPointF(50, 20)));      // in the band at the top
        QVERIFY(p.contains(QPointF(10, 45)));      // and at its left end
        QVERIFY(!p.contains(QPointF(50, 40)));     // not in the hole
        QVERIFY(!p.contains(QPointF(50, 70)));     // not in the missing lower half
        // A thinner band.
        const Geometry thin = presetGeometry(QStringLiteral("blockArc"), {{"adj3", 10000}}, QSizeF(100, 100));
        QVERIFY(thin.paths[0].path.contains(QPointF(50, 5)));
        QVERIFY(!thin.paths[0].path.contains(QPointF(50, 20)));
    }

    void dmlWedgeCalloutPointsAtItsTarget()
    {
        // By default the pointer comes from the bottom edge, toward a point below and left of the center.
        Geometry g = presetGeometry(QStringLiteral("wedgeRectCallout"), {}, QSizeF(200, 100));
        QCOMPARE(g.paths.size(), 1);
        QPainterPath p = g.paths[0].path;
        QVERIFY(p.contains(QPointF(100, 50)));
        QVERIFY(p.contains(QPointF(58, 106)));
        QVERIFY(!p.contains(QPointF(150, 106)));
        QVERIFY(near(p.boundingRect().bottom(), 112.5, 1e-3));
        QVERIFY(near(p.boundingRect().top(), 0) && near(p.boundingRect().left(), 0) && near(p.boundingRect().right(), 200));
        // A pointer to the right of the box.
        g = presetGeometry(QStringLiteral("wedgeRectCallout"), {{"adj1", 90000}, {"adj2", 10000}}, QSizeF(200, 100));
        p = g.paths[0].path;
        QVERIFY(near(p.boundingRect().right(), 280, 1e-3));
        QVERIFY(p.contains(QPointF(240, 65)));
        QVERIFY(!p.contains(QPointF(240, 40)));
        // Rounded and oval ones reach the same point.
        for (const char *name : {"wedgeRoundRectCallout", "wedgeEllipseCallout"}) {
            g = presetGeometry(QString::fromLatin1(name), {{"adj1", -50000}, {"adj2", 90000}}, QSizeF(200, 100));
            QCOMPARE(g.paths.size(), 1);
            const QRectF r = g.paths[0].path.boundingRect();
            QVERIFY2(near(r.bottom(), 140, 1e-3) && r.left() <= 50 + 1e-3 && r.top() >= -0.1,
                     qPrintable(QStringLiteral("%1: %2,%3 %4x%5").arg(QString::fromLatin1(name)).arg(r.left()).arg(r.top()).arg(r.width()).arg(r.height())));
            QVERIFY2(g.paths[0].path.contains(QPointF(100, 40)), name);
        }
    }

    void dmlLineCalloutsHaveABoxAndALeader()
    {
        // borderCallout1: the box is stroked, the line (from outside the box to its target) is not filled.
        Geometry g = presetGeometry(QStringLiteral("borderCallout1"), {}, QSizeF(200, 100));
        QCOMPARE(g.paths.size(), 2);
        QVERIFY(g.paths[0].stroke && g.paths[0].fill == GeometryPath::Normal);
        QVERIFY(g.paths[1].fill == GeometryPath::None && g.paths[1].stroke);
        QVERIFY(near(g.paths[1].path.elementAt(0).x, -16.666, 1e-3) && near(g.paths[1].path.elementAt(0).y, 18.75, 1e-3));
        QVERIFY(near(g.paths[1].path.elementAt(1).x, -76.666, 1e-3) && near(g.paths[1].path.elementAt(1).y, 112.5, 1e-3));
        // callout1 has no border; accentCallout1 an accent bar too; with a border as well, the box is stroked.
        g = presetGeometry(QStringLiteral("callout1"), {}, QSizeF(200, 100));
        QVERIFY(!g.paths[0].stroke);
        g = presetGeometry(QStringLiteral("accentCallout1"), {}, QSizeF(200, 100));
        QCOMPARE(g.paths.size(), 3);
        QVERIFY(!g.paths[0].stroke);
        QVERIFY(near(g.paths[1].path.elementAt(0).x, -16.666, 1e-3) && near(g.paths[1].path.elementAt(1).x, -16.666, 1e-3));
        QVERIFY(near(g.paths[1].path.elementAt(1).y, 100));
        g = presetGeometry(QStringLiteral("accentBorderCallout1"), {}, QSizeF(200, 100));
        QVERIFY(g.paths[0].stroke);
        // The second and third kinds bend the leader once and twice more.
        g = presetGeometry(QStringLiteral("borderCallout2"), {}, QSizeF(200, 100));
        QCOMPARE(g.paths[1].path.elementCount(), 3);
        g = presetGeometry(QStringLiteral("borderCallout3"), {}, QSizeF(200, 100));
        QCOMPARE(g.paths[1].path.elementCount(), 4);
        QVERIFY(near(g.paths[1].path.currentPosition().x(), -16.666, 1e-3) && near(g.paths[1].path.currentPosition().y(), 112.963, 1e-3));
    }

    void dmlArrowCalloutsFillTheirFrame()
    {
        for (const char *name : {"rightArrowCallout", "leftArrowCallout", "upArrowCallout", "downArrowCallout", "leftRightArrowCallout", "upDownArrowCallout"}) {
            const Geometry g = presetGeometry(QString::fromLatin1(name), {}, QSizeF(200, 120));
            QCOMPARE(g.paths.size(), 1);
            const QRectF r = g.paths[0].path.boundingRect();
            QVERIFY2(near(r.left(), 0, 1e-3) && near(r.top(), 0, 1e-3) && near(r.right(), 200, 1e-3) && near(r.bottom(), 120, 1e-3), name);
            QVERIFY2(g.paths[0].path.contains(QPointF(100, 60)) || g.paths[0].path.contains(QPointF(60, 60)), name);
        }
        // The right arrow's head is at the right, the box at the left.
        const Geometry right = presetGeometry(QStringLiteral("rightArrowCallout"), {}, QSizeF(200, 120));
        QVERIFY(right.paths[0].path.contains(QPointF(10, 5)));
        QVERIFY(!right.paths[0].path.contains(QPointF(190, 5)));
        QVERIFY(right.paths[0].path.contains(QPointF(180, 60)));
        const Geometry down = presetGeometry(QStringLiteral("downArrowCallout"), {}, QSizeF(120, 200));
        QVERIFY(down.paths[0].path.contains(QPointF(5, 10)));
        QVERIFY(!down.paths[0].path.contains(QPointF(5, 190)));
        QVERIFY(down.paths[0].path.contains(QPointF(60, 190)));
    }

    void dmlShapesTheLibraryLacksHaveDefinitions()
    {
        // A quarter pie sits in the lower right corner; the trapezoid is narrower at the top on both sides.
        Geometry g = presetGeometry(QStringLiteral("pieWedge"), {}, QSizeF(100, 100));
        QCOMPARE(g.paths.size(), 1);
        QVERIFY(g.paths[0].path.contains(QPointF(90, 90)));
        QVERIFY(g.paths[0].path.contains(QPointF(5, 95)));
        QVERIFY(!g.paths[0].path.contains(QPointF(10, 10)));
        g = presetGeometry(QStringLiteral("nonIsoscelesTrapezoid"), {}, QSizeF(200, 100));
        QVERIFY(g.paths[0].path.contains(QPointF(100, 5)));
        QVERIFY(!g.paths[0].path.contains(QPointF(5, 5)));
        QVERIFY(g.paths[0].path.contains(QPointF(5, 95)));
    }

    void dmlConnectorPresetsAreOpenPaths()
    {
        // The elbow with one bend goes along the top and down the right; with two, across at the adjusted position.
        Geometry g = presetGeometry(QStringLiteral("bentConnector2"), {}, QSizeF(100, 60));
        QCOMPARE(g.paths.size(), 1);
        QVERIFY(g.paths[0].fill == GeometryPath::None);
        QCOMPARE(g.paths[0].path.elementCount(), 3);
        QVERIFY(near(g.paths[0].path.elementAt(1).x, 100) && near(g.paths[0].path.elementAt(1).y, 0));
        g = presetGeometry(QStringLiteral("bentConnector3"), {{"adj1", 25000}}, QSizeF(100, 60));
        QCOMPARE(g.paths[0].path.elementCount(), 4);
        QVERIFY(near(g.paths[0].path.elementAt(1).x, 25) && near(g.paths[0].path.elementAt(2).x, 25) && near(g.paths[0].path.elementAt(2).y, 60));
        g = presetGeometry(QStringLiteral("bentConnector4"), {}, QSizeF(100, 60));
        QCOMPARE(g.paths[0].path.elementCount(), 5);
        QVERIFY(near(g.paths[0].path.currentPosition().x(), 100) && near(g.paths[0].path.currentPosition().y(), 60));
        g = presetGeometry(QStringLiteral("bentConnector5"), {{"adj1", 25000}, {"adj3", 75000}}, QSizeF(100, 60));
        QCOMPARE(g.paths[0].path.elementCount(), 6);
        QVERIFY(near(g.paths[0].path.elementAt(1).x, 25) && near(g.paths[0].path.elementAt(4).x, 75));
        for (const char *name : {"curvedConnector2", "curvedConnector3", "curvedConnector4", "curvedConnector5"}) {
            g = presetGeometry(QString::fromLatin1(name), {}, QSizeF(100, 60));
            QCOMPARE(g.paths.size(), 1);
            QVERIFY(near(g.paths[0].path.elementAt(0).x, 0) && near(g.paths[0].path.elementAt(0).y, 0));
            QVERIFY2(near(g.paths[0].path.currentPosition().x(), 100) && near(g.paths[0].path.currentPosition().y(), 60), name);
        }
    }

    // ---- DrawingML shapes: fills, lines, and styles

private:
    static Fill dmlFillOf(DmlEnv &env, const QString &properties, const QString &style = QString())
    {
        return readFill(env.element(QStringLiteral("<p:spPr>%1</p:spPr>").arg(properties)), style.isEmpty() ? QDomElement() : env.element(style), env.ctx);
    }

    static Stroke dmlLineOf(DmlEnv &env, const QString &properties, const QString &style = QString())
    {
        return readLine(env.element(QStringLiteral("<p:spPr>%1</p:spPr>").arg(properties)), style.isEmpty() ? QDomElement() : env.element(style), env.ctx);
    }

    // <p:style> with the references given.
    static QString dmlStyle(const QString &lnIdx, const QString &fillIdx, const QString &effectIdx = QStringLiteral("0"), const QString &color = QStringLiteral("accent1"))
    {
        return QStringLiteral("<p:style><a:lnRef idx=\"%1\"><a:schemeClr val=\"%4\"/></a:lnRef><a:fillRef idx=\"%2\"><a:schemeClr val=\"%4\"/></a:fillRef>"
                              "<a:effectRef idx=\"%3\"><a:schemeClr val=\"%4\"/></a:effectRef><a:fontRef idx=\"minor\"><a:schemeClr val=\"lt1\"/></a:fontRef></p:style>")
            .arg(lnIdx, fillIdx, effectIdx, color);
    }

    static QString dmlHex(const ColorRef &c) { return hexOf(c.rgbValue()); }

private Q_SLOTS:
    void dmlFillReadsSolidColors()
    {
        DmlEnv env;
        Fill f = dmlFillOf(env, QStringLiteral("<a:solidFill><a:srgbClr val=\"FF8000\"/></a:solidFill>"));
        QVERIFY(f.type == Fill::Solid);
        QCOMPARE(dmlHex(f.color), QStringLiteral("FF8000"));
        QCOMPARE(f.transparency, 0.0);
        f = dmlFillOf(env, QStringLiteral("<a:solidFill><a:schemeClr val=\"accent1\"><a:alpha val=\"25000\"/></a:schemeClr></a:solidFill>"));
        QVERIFY(f.type == Fill::Solid);
        QCOMPARE(dmlHex(f.color), QStringLiteral("4472C4"));
        QVERIFY(near(f.transparency, 0.75, 1e-2));
        // No fill, or none of the kinds, is none.
        QVERIFY(dmlFillOf(env, QString()).isNone());
        QVERIFY(dmlFillOf(env, QStringLiteral("<a:noFill/>")).isNone());
        QVERIFY(dmlFillOf(env, QStringLiteral("<a:solidFill/>")).isNone());
        QVERIFY(readFill(QDomElement(), QDomElement(), env.ctx).isNone());
        // A strict file's elements are the same.
        QDomDocument doc;
        const QByteArray strict = QStringLiteral("<p:spPr xmlns:p=\"%2\" xmlns:a=\"%1\"><a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill></p:spPr>").arg(QString(ns::AStrict), QString(ns::PStrict)).toUtf8();
        QVERIFY(parseXml(strict, &doc));
        QCOMPARE(dmlHex(readFill(doc.documentElement(), QDomElement(), env.ctx).color), QStringLiteral("00FF00"));
    }

    void dmlFillReadsGradients()
    {
        DmlEnv env;
        const QString stops = QStringLiteral("<a:gsLst><a:gs pos=\"0\"><a:srgbClr val=\"FF0000\"/></a:gs><a:gs pos=\"60000\"><a:srgbClr val=\"00FF00\"><a:alpha val=\"50000\"/></a:srgbClr></a:gs>"
                                             "<a:gs pos=\"100000\"><a:srgbClr val=\"0000FF\"/></a:gs></a:gsLst>");
        Fill f = dmlFillOf(env, QStringLiteral("<a:gradFill rotWithShape=\"1\">%1<a:lin ang=\"2700000\" scaled=\"0\"/></a:gradFill>").arg(stops));
        QVERIFY(f.type == Fill::Gradient);
        QVERIFY(f.gradType == Fill::Linear);
        QVERIFY(near(f.angle, 45));
        QCOMPARE(f.stops.size(), 3);
        QVERIFY(near(f.stops[0].pos, 0) && near(f.stops[1].pos, 0.6) && near(f.stops[2].pos, 1));
        QCOMPARE(dmlHex(f.stops[0].color), QStringLiteral("FF0000"));
        QCOMPARE(dmlHex(f.stops[1].color), QStringLiteral("00FF00"));
        QVERIFY(near(f.stops[1].transparency, 0.5, 1e-2));
        QCOMPARE(f.stops[0].transparency, 0.0);
        QCOMPARE(dmlHex(f.color), QStringLiteral("FF0000"));
        QCOMPARE(dmlHex(f.color2), QStringLiteral("0000FF"));
        // Angles: 0 is left to right, a quarter turn is top to bottom, and any turn is read modulo a full one.
        f = dmlFillOf(env, QStringLiteral("<a:gradFill>%1<a:lin ang=\"0\"/></a:gradFill>").arg(stops));
        QVERIFY(near(f.angle, 0));
        f = dmlFillOf(env, QStringLiteral("<a:gradFill>%1<a:lin ang=\"5400000\"/></a:gradFill>").arg(stops));
        QVERIFY(near(f.angle, 90));
        f = dmlFillOf(env, QStringLiteral("<a:gradFill>%1<a:lin ang=\"-5400000\"/></a:gradFill>").arg(stops));
        QVERIFY(near(f.angle, 270));
        f = dmlFillOf(env, QStringLiteral("<a:gradFill>%1<a:lin ang=\"27000000\"/></a:gradFill>").arg(stops));
        QVERIFY(near(f.angle, 90));
        // Without a direction it runs left to right.
        f = dmlFillOf(env, QStringLiteral("<a:gradFill>%1</a:gradFill>").arg(stops));
        QVERIFY(f.type == Fill::Gradient && f.gradType == Fill::Linear && near(f.angle, 0));
        // Path gradients.
        f = dmlFillOf(env, QStringLiteral("<a:gradFill>%1<a:path path=\"circle\"><a:fillToRect l=\"50000\" t=\"50000\" r=\"50000\" b=\"50000\"/></a:path></a:gradFill>").arg(stops));
        QVERIFY(f.type == Fill::Gradient && f.gradType == Fill::Radial);
        f = dmlFillOf(env, QStringLiteral("<a:gradFill>%1<a:path path=\"rect\"/></a:gradFill>").arg(stops));
        QVERIFY(f.gradType == Fill::Rectangular);
        f = dmlFillOf(env, QStringLiteral("<a:gradFill>%1<a:path path=\"shape\"/></a:gradFill>").arg(stops));
        QVERIFY(f.gradType == Fill::PathGrad);
        // Stops come in order of position, whatever order they are written in; positions stay within the gradient.
        f = dmlFillOf(env, QStringLiteral("<a:gradFill><a:gsLst><a:gs pos=\"100000\"><a:srgbClr val=\"0000FF\"/></a:gs><a:gs pos=\"-5\"><a:srgbClr val=\"FF0000\"/></a:gs>"
                                          "<a:gs pos=\"900000\"><a:srgbClr val=\"00FF00\"/></a:gs></a:gsLst></a:gradFill>"));
        QCOMPARE(f.stops.size(), 3);
        QVERIFY(f.stops[0].pos == 0 && f.stops[1].pos <= f.stops[2].pos && f.stops[2].pos <= 1);
        QCOMPARE(dmlHex(f.stops[0].color), QStringLiteral("FF0000"));
        // One stop is a plain color; none is no fill.
        f = dmlFillOf(env, QStringLiteral("<a:gradFill><a:gsLst><a:gs pos=\"0\"><a:srgbClr val=\"123456\"/></a:gs></a:gsLst></a:gradFill>"));
        QVERIFY(f.type == Fill::Solid);
        QCOMPARE(dmlHex(f.color), QStringLiteral("123456"));
        QVERIFY(dmlFillOf(env, QStringLiteral("<a:gradFill><a:gsLst/></a:gradFill>")).isNone());
        QVERIFY(dmlFillOf(env, QStringLiteral("<a:gradFill/>")).isNone());
    }

    void dmlFillReadsPatterns()
    {
        DmlEnv env;
        const auto pattern = [&](const char *name) {
            return dmlFillOf(env, QStringLiteral("<a:pattFill prst=\"%1\"><a:fgClr><a:srgbClr val=\"FF0000\"/></a:fgClr><a:bgClr><a:srgbClr val=\"FFFFFF\"/></a:bgClr></a:pattFill>")
                                      .arg(QString::fromLatin1(name)));
        };
        Fill f = pattern("ltHorz");
        QVERIFY(f.type == Fill::Pattern);
        QCOMPARE(dmlHex(f.color), QStringLiteral("FF0000"));
        QCOMPARE(dmlHex(f.color2), QStringLiteral("FFFFFF"));
        const QVector<Qt::BrushStyle> brushes = jp::patternBrushes();
        QCOMPARE(brushes[pattern("horz").pattern], Qt::HorPattern);
        QCOMPARE(brushes[pattern("dkHorz").pattern], Qt::HorPattern);
        QCOMPARE(brushes[pattern("vert").pattern], Qt::VerPattern);
        QCOMPARE(brushes[pattern("cross").pattern], Qt::CrossPattern);
        QCOMPARE(brushes[pattern("smGrid").pattern], Qt::CrossPattern);
        QCOMPARE(brushes[pattern("dnDiag").pattern], Qt::FDiagPattern);
        QCOMPARE(brushes[pattern("upDiag").pattern], Qt::BDiagPattern);
        QCOMPARE(brushes[pattern("diagCross").pattern], Qt::DiagCrossPattern);
        QCOMPARE(brushes[pattern("pct5").pattern], Qt::Dense7Pattern);
        QCOMPARE(brushes[pattern("pct50").pattern], Qt::Dense4Pattern);
        QCOMPARE(brushes[pattern("pct90").pattern], Qt::Dense1Pattern);
        // Every pattern the standard names draws as something; one it doesn't is the foreground color alone.
        for (const char *name : {"pct10", "pct20", "pct25", "pct30", "pct40", "pct60", "pct70", "pct75", "pct80", "ltVert", "dkVert", "narHorz", "narVert",
                                 "dashHorz", "dashVert", "ltDnDiag", "ltUpDiag", "dkDnDiag", "dkUpDiag", "wdDnDiag", "wdUpDiag", "dashDnDiag", "dashUpDiag", "smCheck",
                                 "lgCheck", "lgGrid", "dotGrid", "smConfetti", "lgConfetti", "horzBrick", "diagBrick", "solidDmnd", "openDmnd", "dotDmnd", "plaid",
                                 "sphere", "weave", "divot", "shingle", "wave", "trellis", "zigZag"})
            QVERIFY2(pattern(name).type == Fill::Pattern && pattern(name).pattern >= 0 && pattern(name).pattern < brushes.size(), name);
        f = pattern("noSuchPattern");
        QVERIFY(f.type == Fill::Solid);
        QCOMPARE(dmlHex(f.color), QStringLiteral("FF0000"));
        // No colors given: black on white, as the standard says.
        f = dmlFillOf(env, QStringLiteral("<a:pattFill prst=\"horz\"/>"));
        QVERIFY(f.type == Fill::Pattern);
    }

    void dmlFillReadsPictures()
    {
        DmlEnv env;
        env.usePackage();
        Fill f = dmlFillOf(env, QStringLiteral("<a:blipFill><a:blip r:embed=\"rId1\"/><a:stretch><a:fillRect/></a:stretch></a:blipFill>"));
        QVERIFY(f.type == Fill::Picture);
        QCOMPARE(f.imageId, QStringLiteral("img1"));
        QVERIFY(!f.tile);
        QCOMPARE(env.images.size(), 1);
        QCOMPARE(env.images[0].format, QStringLiteral("png"));
        QCOMPARE(env.images[0].bytes, dmlPicture());
        // A tiled one, at half its size, and with some transparency.
        f = dmlFillOf(env, QStringLiteral("<a:blipFill><a:blip r:embed=\"rId1\"><a:alphaModFix amt=\"40000\"/></a:blip><a:tile tx=\"0\" ty=\"0\" sx=\"50000\" sy=\"50000\" flip=\"none\" algn=\"tl\"/></a:blipFill>"));
        QVERIFY(f.type == Fill::Picture && f.tile);
        QVERIFY(near(f.tileScale, 0.5, 1e-6));
        QVERIFY(near(f.transparency, 0.6, 1e-6));
        // A picture that isn't there, or isn't a picture, is no fill and no crash.
        QVERIFY(dmlFillOf(env, QStringLiteral("<a:blipFill><a:blip r:embed=\"rId9\"/></a:blipFill>")).isNone());
        QVERIFY(dmlFillOf(env, QStringLiteral("<a:blipFill><a:blip/></a:blipFill>")).isNone());
        QVERIFY(dmlFillOf(env, QStringLiteral("<a:blipFill/>")).isNone());
        QVERIFY(dmlFillOf(env, QStringLiteral("<a:blipFill><a:blip r:embed=\"rId2\"/></a:blipFill>")).isNone());   // an outside link
        DmlEnv bare;
        QVERIFY(dmlFillOf(bare, QStringLiteral("<a:blipFill><a:blip r:embed=\"rId1\"/></a:blipFill>")).isNone());
        // The format comes from what the bytes are, whatever the part is called.
        DmlEnv other;
        QImage img(2, 2, QImage::Format_RGB32);
        img.fill(Qt::blue);
        QByteArray jpeg;
        QBuffer buffer(&jpeg);
        buffer.open(QIODevice::WriteOnly);
        img.save(&buffer, "JPEG");
        other.usePackage(jpeg);
        f = dmlFillOf(other, QStringLiteral("<a:blipFill><a:blip r:embed=\"rId1\"/></a:blipFill>"));
        QVERIFY(f.type == Fill::Picture);
        QCOMPARE(other.images[0].format, QStringLiteral("jpeg"));
    }

    void dmlFillTakesTheStyleWhenThePropertiesSayNothing()
    {
        DmlEnv env;
        env.useFormats();
        // Reference 1: a plain fill of the reference's color.
        Fill f = dmlFillOf(env, QString(), dmlStyle(QStringLiteral("1"), QStringLiteral("1"), QStringLiteral("0"), QStringLiteral("accent2")));
        QVERIFY(f.type == Fill::Solid);
        QCOMPARE(dmlHex(f.color), QStringLiteral("ED7D31"));
        // 2: a gradient made of it, from lighter to the color itself.
        f = dmlFillOf(env, QString(), dmlStyle(QStringLiteral("1"), QStringLiteral("2")));
        QVERIFY(f.type == Fill::Gradient);
        QCOMPARE(f.stops.size(), 2);
        QCOMPARE(dmlHex(f.stops[1].color), QStringLiteral("4472C4"));
        QVERIFY(f.stops[0].color.rgbValue().lightness() > f.stops[1].color.rgbValue().lightness());
        QVERIFY(near(f.angle, 90));
        // 3: the darkened color.
        f = dmlFillOf(env, QString(), dmlStyle(QStringLiteral("1"), QStringLiteral("3")));
        QCOMPARE(dmlHex(f.color), hexOf(colorOf(schemeClr(QStringLiteral("accent1"), {"lumMod 50000"}))));
        // 0, and one that is past the list, are none; 1001 and on are the background fills.
        QVERIFY(dmlFillOf(env, QString(), dmlStyle(QStringLiteral("1"), QStringLiteral("0"))).isNone());
        QVERIFY(dmlFillOf(env, QString(), dmlStyle(QStringLiteral("1"), QStringLiteral("4"))).isNone());
        QVERIFY(dmlFillOf(env, QString(), dmlStyle(QStringLiteral("1"), QStringLiteral("-3"))).isNone());
        QVERIFY(dmlFillOf(env, QString(), dmlStyle(QStringLiteral("1"), QStringLiteral("junk"))).isNone());
        QVERIFY(dmlFillOf(env, QString(), dmlStyle(QStringLiteral("1"), QStringLiteral("99999999999999"))).isNone());
        f = dmlFillOf(env, QString(), dmlStyle(QStringLiteral("1"), QStringLiteral("1003"), QStringLiteral("0"), QStringLiteral("accent3")));
        QVERIFY(f.type == Fill::Gradient && f.gradType == Fill::Radial);
        QCOMPARE(dmlHex(f.stops[0].color), QStringLiteral("A5A5A5"));
        // The properties' own fill wins, and so does their saying no fill.
        f = dmlFillOf(env, QStringLiteral("<a:solidFill><a:srgbClr val=\"112233\"/></a:solidFill>"), dmlStyle(QStringLiteral("1"), QStringLiteral("1")));
        QCOMPARE(dmlHex(f.color), QStringLiteral("112233"));
        QVERIFY(dmlFillOf(env, QStringLiteral("<a:noFill/>"), dmlStyle(QStringLiteral("1"), QStringLiteral("1"))).isNone());
        // Other properties (a line) leave the fill to the style.
        f = dmlFillOf(env, QStringLiteral("<a:ln w=\"12700\"/>"), dmlStyle(QStringLiteral("1"), QStringLiteral("1")));
        QVERIFY(f.type == Fill::Solid);
        // Without a format scheme the style gives nothing.
        DmlEnv bare;
        QVERIFY(dmlFillOf(bare, QString(), dmlStyle(QStringLiteral("1"), QStringLiteral("1"))).isNone());
    }

    void dmlLineReadsWidthAndColor()
    {
        DmlEnv env;
        Stroke s = dmlLineOf(env, QStringLiteral("<a:ln w=\"25400\"><a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill></a:ln>"));
        QVERIFY(!s.isNone());
        QVERIFY(near(s.width, 2));
        QCOMPARE(dmlHex(s.color), QStringLiteral("00FF00"));
        QCOMPARE(s.transparency, 0.0);
        QVERIFY(s.dash == Stroke::SolidLine && s.compound == Stroke::Single && s.cap == Qt::FlatCap);
        // No width: the line's usual, a quarter point shy of a point.
        s = dmlLineOf(env, QStringLiteral("<a:ln><a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill></a:ln>"));
        QVERIFY(near(s.width, 0.75));
        s = dmlLineOf(env, QStringLiteral("<a:ln w=\"12700\"><a:solidFill><a:srgbClr val=\"00FF00\"><a:alpha val=\"20000\"/></a:srgbClr></a:solidFill></a:ln>"));
        QVERIFY(near(s.transparency, 0.8, 1e-2));
        // No line, no fill in it, or a fill that isn't one gives none.
        QVERIFY(dmlLineOf(env, QString()).isNone());
        QVERIFY(dmlLineOf(env, QStringLiteral("<a:ln w=\"12700\"/>")).isNone());
        QVERIFY(dmlLineOf(env, QStringLiteral("<a:ln w=\"12700\"><a:noFill/></a:ln>")).isNone());
        QVERIFY(readLine(QDomElement(), QDomElement(), env.ctx).isNone());
        // A line of no width is the thinnest there is; a huge one is cut down.
        s = dmlLineOf(env, QStringLiteral("<a:ln w=\"0\"><a:solidFill><a:srgbClr val=\"000000\"/></a:solidFill></a:ln>"));
        QVERIFY(!s.isNone() && s.width > 0 && s.width <= 0.5);
        s = dmlLineOf(env, QStringLiteral("<a:ln w=\"999999999999\"><a:solidFill><a:srgbClr val=\"000000\"/></a:solidFill></a:ln>"));
        QVERIFY(s.width <= 1584);
        s = dmlLineOf(env, QStringLiteral("<a:ln w=\"-5\"><a:solidFill><a:srgbClr val=\"000000\"/></a:solidFill></a:ln>"));
        QVERIFY(!s.isNone() && s.width > 0 && s.width <= 1);
        // A gradient line is the color of its first stop; a pattern line is its foreground.
        s = dmlLineOf(env, QStringLiteral("<a:ln w=\"12700\"><a:gradFill><a:gsLst><a:gs pos=\"0\"><a:srgbClr val=\"FF0000\"/></a:gs><a:gs pos=\"100000\"><a:srgbClr val=\"0000FF\"/></a:gs></a:gsLst></a:gradFill></a:ln>"));
        QCOMPARE(dmlHex(s.color), QStringLiteral("FF0000"));
        s = dmlLineOf(env, QStringLiteral("<a:ln w=\"12700\"><a:pattFill prst=\"horz\"><a:fgClr><a:srgbClr val=\"123456\"/></a:fgClr><a:bgClr><a:srgbClr val=\"FFFFFF\"/></a:bgClr></a:pattFill></a:ln>"));
        QCOMPARE(dmlHex(s.color), QStringLiteral("123456"));
    }

    void dmlLineReadsDashesCapsAndJoins()
    {
        DmlEnv env;
        const auto line = [&](const QString &inner, const QString &attrs = QString()) {
            return dmlLineOf(env, QStringLiteral("<a:ln w=\"12700\" %2><a:solidFill><a:srgbClr val=\"000000\"/></a:solidFill>%1</a:ln>").arg(inner, attrs));
        };
        const struct { const char *name; Stroke::Dash dash; } dashes[] = {
            {"solid", Stroke::SolidLine}, {"dot", Stroke::RoundDot}, {"sysDot", Stroke::SquareDot}, {"dash", Stroke::DashLine}, {"sysDash", Stroke::DashLine},
            {"dashDot", Stroke::DashDot}, {"sysDashDot", Stroke::DashDot}, {"lgDash", Stroke::LongDash}, {"lgDashDot", Stroke::LongDashDot},
            {"lgDashDotDot", Stroke::LongDashDotDot}, {"sysDashDotDot", Stroke::LongDashDotDot}};
        for (const auto &d : dashes) QVERIFY2(line(QStringLiteral("<a:prstDash val=\"%1\"/>").arg(QString::fromLatin1(d.name))).dash == d.dash, d.name);
        QVERIFY(line(QStringLiteral("<a:custDash><a:ds d=\"100000\" sp=\"50000\"/></a:custDash>")).dash == Stroke::DashLine);
        QVERIFY(line(QStringLiteral("<a:prstDash val=\"nonsense\"/>")).dash == Stroke::SolidLine);
        QVERIFY(line(QString(), QStringLiteral("cap=\"rnd\"")).cap == Qt::RoundCap);
        QVERIFY(line(QString(), QStringLiteral("cap=\"sq\"")).cap == Qt::SquareCap);
        QVERIFY(line(QString(), QStringLiteral("cap=\"flat\"")).cap == Qt::FlatCap);
        QVERIFY(line(QStringLiteral("<a:round/>")).join == Qt::RoundJoin);
        QVERIFY(line(QStringLiteral("<a:bevel/>")).join == Qt::BevelJoin);
        QVERIFY(line(QStringLiteral("<a:miter lim=\"800000\"/>")).join == Qt::MiterJoin);
        QVERIFY(line(QString(), QStringLiteral("cmpd=\"dbl\"")).compound == Stroke::Double);
        QVERIFY(line(QString(), QStringLiteral("cmpd=\"thickThin\"")).compound == Stroke::ThickThin);
        QVERIFY(line(QString(), QStringLiteral("cmpd=\"thinThick\"")).compound == Stroke::ThinThick);
        QVERIFY(line(QString(), QStringLiteral("cmpd=\"tri\"")).compound == Stroke::Triple);
        QVERIFY(line(QString(), QStringLiteral("cmpd=\"sng\"")).compound == Stroke::Single);
        QVERIFY(!line(QString()).inset);
        QVERIFY(line(QString(), QStringLiteral("algn=\"in\"")).inset);
    }

    void dmlLineReadsArrowheads()
    {
        DmlEnv env;
        const auto line = [&](const QString &ends) {
            return dmlLineOf(env, QStringLiteral("<a:ln w=\"12700\"><a:solidFill><a:srgbClr val=\"000000\"/></a:solidFill>%1</a:ln>").arg(ends));
        };
        Stroke s = line(QStringLiteral("<a:headEnd type=\"triangle\" w=\"lg\" len=\"lg\"/><a:tailEnd type=\"stealth\" w=\"sm\" len=\"sm\"/>"));
        QVERIFY(s.startArrow == Arrow::Triangle && s.endArrow == Arrow::Stealth);
        QCOMPARE(s.startSize, 2);
        QCOMPARE(s.endSize, 0);
        s = line(QStringLiteral("<a:tailEnd type=\"arrow\"/>"));
        QVERIFY(s.startArrow == Arrow::None && s.endArrow == Arrow::Open);
        QCOMPARE(s.endSize, 1);
        s = line(QStringLiteral("<a:headEnd type=\"diamond\"/><a:tailEnd type=\"oval\" w=\"med\" len=\"lg\"/>"));
        QVERIFY(s.startArrow == Arrow::Diamond && s.endArrow == Arrow::Oval);
        QVERIFY(s.endSize >= 1 && s.endSize <= 2);
        s = line(QStringLiteral("<a:headEnd type=\"none\"/><a:tailEnd type=\"bogus\"/>"));
        QVERIFY(s.startArrow == Arrow::None && s.endArrow == Arrow::None);
        s = line(QString());
        QVERIFY(s.startArrow == Arrow::None && s.endArrow == Arrow::None);
    }

    void dmlLineTakesTheStyleAndLetsPropertiesChangeIt()
    {
        DmlEnv env;
        env.useFormats();
        // Reference 1: a half point, the reference's color.
        Stroke s = dmlLineOf(env, QString(), dmlStyle(QStringLiteral("1"), QStringLiteral("0"), QStringLiteral("0"), QStringLiteral("accent2")));
        QVERIFY(!s.isNone());
        QVERIFY(near(s.width, 0.5));
        QCOMPARE(dmlHex(s.color), QStringLiteral("ED7D31"));
        // 2: a point, dashed, round joined; 3: one and a half, round capped, double, beveled.
        s = dmlLineOf(env, QString(), dmlStyle(QStringLiteral("2"), QStringLiteral("0")));
        QVERIFY(near(s.width, 1) && s.dash == Stroke::DashLine && s.join == Qt::RoundJoin);
        s = dmlLineOf(env, QString(), dmlStyle(QStringLiteral("3"), QStringLiteral("0")));
        QVERIFY(near(s.width, 1.5) && s.cap == Qt::RoundCap && s.compound == Stroke::Double && s.join == Qt::BevelJoin);
        // 0 and a reference past the list give no line.
        QVERIFY(dmlLineOf(env, QString(), dmlStyle(QStringLiteral("0"), QStringLiteral("0"))).isNone());
        QVERIFY(dmlLineOf(env, QString(), dmlStyle(QStringLiteral("4"), QStringLiteral("0"))).isNone());
        // The properties' width alone changes the width and keeps the style's color and dashes.
        s = dmlLineOf(env, QStringLiteral("<a:ln w=\"38100\"/>"), dmlStyle(QStringLiteral("2"), QStringLiteral("0")));
        QVERIFY(near(s.width, 3));
        QCOMPARE(dmlHex(s.color), QStringLiteral("4472C4"));
        QVERIFY(s.dash == Stroke::DashLine);
        // Their color keeps the style's width.
        s = dmlLineOf(env, QStringLiteral("<a:ln><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></a:ln>"), dmlStyle(QStringLiteral("3"), QStringLiteral("0")));
        QVERIFY(near(s.width, 1.5));
        QCOMPARE(dmlHex(s.color), QStringLiteral("FF0000"));
        // Their saying no fill is no line, and their dash replaces the style's.
        QVERIFY(dmlLineOf(env, QStringLiteral("<a:ln><a:noFill/></a:ln>"), dmlStyle(QStringLiteral("2"), QStringLiteral("0"))).isNone());
        s = dmlLineOf(env, QStringLiteral("<a:ln><a:prstDash val=\"solid\"/></a:ln>"), dmlStyle(QStringLiteral("2"), QStringLiteral("0")));
        QVERIFY(s.dash == Stroke::SolidLine && !s.isNone());
        // Without a format scheme, the style gives none.
        DmlEnv bare;
        QVERIFY(dmlLineOf(bare, QString(), dmlStyle(QStringLiteral("1"), QStringLiteral("0"))).isNone());
    }

    void dmlBackgroundReadsItsFill()
    {
        DmlEnv env;
        env.useFormats();
        Fill f = readBackground(env.element(QStringLiteral("<p:bg><p:bgPr><a:solidFill><a:srgbClr val=\"102030\"/></a:solidFill><a:effectLst/></p:bgPr></p:bg>")), env.ctx);
        QVERIFY(f.type == Fill::Solid);
        QCOMPARE(dmlHex(f.color), QStringLiteral("102030"));
        f = readBackground(env.element(QStringLiteral("<p:bg><p:bgPr><a:gradFill><a:gsLst><a:gs pos=\"0\"><a:srgbClr val=\"FF0000\"/></a:gs><a:gs pos=\"100000\"><a:srgbClr val=\"0000FF\"/></a:gs></a:gsLst>"
                                                      "<a:lin ang=\"5400000\"/></a:gradFill></p:bgPr></p:bg>")), env.ctx);
        QVERIFY(f.type == Fill::Gradient);
        QVERIFY(readBackground(env.element(QStringLiteral("<p:bg><p:bgPr><a:noFill/></p:bgPr></p:bg>")), env.ctx).isNone());
        // A reference into the theme's background fills, with the color it gives.
        f = readBackground(env.element(QStringLiteral("<p:bg><p:bgRef idx=\"1001\"><a:schemeClr val=\"bg1\"/></p:bgRef></p:bg>")), env.ctx);
        QVERIFY(f.type == Fill::Solid);
        QCOMPARE(dmlHex(f.color), QStringLiteral("FEFEFE"));
        f = readBackground(env.element(QStringLiteral("<p:bg><p:bgRef idx=\"1003\"><a:srgbClr val=\"336699\"/></p:bgRef></p:bg>")), env.ctx);
        QVERIFY(f.type == Fill::Gradient && f.gradType == Fill::Radial);
        QCOMPARE(dmlHex(f.stops[0].color), QStringLiteral("336699"));
        // A reference of 1 to 3 is into the fill styles.
        f = readBackground(env.element(QStringLiteral("<p:bg><p:bgRef idx=\"2\"><a:schemeClr val=\"accent1\"/></p:bgRef></p:bg>")), env.ctx);
        QVERIFY(f.type == Fill::Gradient);
        // Nothing, no scheme to refer to, or a reference past the list is no fill.
        QVERIFY(readBackground(QDomElement(), env.ctx).isNone());
        QVERIFY(readBackground(env.element(QStringLiteral("<p:bg/>")), env.ctx).isNone());
        QVERIFY(readBackground(env.element(QStringLiteral("<p:bg><p:bgRef idx=\"1009\"><a:schemeClr val=\"bg1\"/></p:bgRef></p:bg>")), env.ctx).isNone());
        DmlEnv bare;
        QVERIFY(readBackground(bare.element(QStringLiteral("<p:bg><p:bgRef idx=\"1001\"><a:schemeClr val=\"bg1\"/></p:bgRef></p:bg>")), bare.ctx).isNone());
    }

    // ---- DrawingML shapes: reading shapes

private:
    static QString dmlXfrm(double x, double y, double w, double h, const QString &attrs = QString(), const char *tag = "a:xfrm")
    {
        return QStringLiteral("<%6 %5><a:off x=\"%1\" y=\"%2\"/><a:ext cx=\"%3\" cy=\"%4\"/></%6>")
            .arg(emu(x), emu(y), emu(w), emu(h), attrs, QString::fromLatin1(tag));
    }

    // A p:sp with the properties, text body, and style given.
    static QString dmlSp(const QString &spPr, const QString &body = QString(), const QString &style = QString(), const QString &cNvSpPr = QString(),
                         const QString &nvPr = QString(), const QString &cNvPrInner = QString(), const QString &id = QStringLiteral("2"))
    {
        return QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"%7\" name=\"Shape %7\" descr=\"alt %7\">%6</p:cNvPr><p:cNvSpPr %4/><p:nvPr>%5</p:nvPr></p:nvSpPr>"
                              "<p:spPr>%1</p:spPr>%3%2</p:sp>")
            .arg(spPr, body, style, cNvSpPr, nvPr, cNvPrInner, id);
    }

    static QString dmlBody(const QString &text = QStringLiteral("Hello"), const QString &bodyPr = QStringLiteral("<a:bodyPr/>"))
    {
        return QStringLiteral("<p:txBody>%2<a:lstStyle/><a:p><a:r><a:t>%1</a:t></a:r></a:p></p:txBody>").arg(text, bodyPr);
    }

    static QString dmlPic(const QString &blipInner, const QString &spPr, const QString &blipFillInner = QStringLiteral("<a:stretch><a:fillRect/></a:stretch>"),
                          const QString &cNvPrInner = QString(), const QString &embed = QStringLiteral("rId1"))
    {
        return QStringLiteral("<p:pic><p:nvPicPr><p:cNvPr id=\"5\" name=\"Picture 5\" descr=\"a picture\">%4</p:cNvPr><p:cNvPicPr/><p:nvPr/></p:nvPicPr>"
                              "<p:blipFill><a:blip r:embed=\"%5\">%1</a:blip>%3</p:blipFill><p:spPr>%2</p:spPr></p:pic>")
            .arg(blipInner, spPr, blipFillInner, cNvPrInner, embed);
    }

    static QString dmlCxn(const QString &spPr, const QString &cNvCxnSpPr = QString(), const QString &style = QString(), const QString &id = QStringLiteral("9"))
    {
        return QStringLiteral("<p:cxnSp><p:nvCxnSpPr><p:cNvPr id=\"%4\" name=\"Connector %4\"/><p:cNvCxnSpPr>%2</p:cNvCxnSpPr><p:nvPr/></p:nvCxnSpPr>"
                              "<p:spPr>%1</p:spPr>%3</p:cxnSp>").arg(spPr, cNvCxnSpPr, style, id);
    }

    static QString dmlGrp(const QString &grpSpPr, const QString &children, const QString &id = QStringLiteral("20"))
    {
        return QStringLiteral("<p:grpSp><p:nvGrpSpPr><p:cNvPr id=\"%3\" name=\"Group %3\" descr=\"group alt\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>"
                              "<p:grpSpPr>%1</p:grpSpPr>%2</p:grpSp>").arg(grpSpPr, children, id);
    }

    // The group frame (off, ext) with the child space (chOff, chExt), all in points.
    static QString dmlGroupFrame(QRectF frame, QRectF child, const QString &attrs = QString())
    {
        return QStringLiteral("<a:xfrm %9><a:off x=\"%1\" y=\"%2\"/><a:ext cx=\"%3\" cy=\"%4\"/><a:chOff x=\"%5\" y=\"%6\"/><a:chExt cx=\"%7\" cy=\"%8\"/></a:xfrm>")
            .arg(emu(frame.x()), emu(frame.y()), emu(frame.width()), emu(frame.height()), emu(child.x()), emu(child.y()), emu(child.width()), emu(child.height()), attrs);
    }

    static QString dmlSolid(const QString &hex) { return QStringLiteral("<a:solidFill><a:srgbClr val=\"%1\"/></a:solidFill>").arg(hex); }

    static ItemList dmlTree(DmlEnv &env, const QString &shapes, const TextStyleStack &styles = {})
    {
        return readShapeTree(env.element(QStringLiteral("<p:spTree>%1</p:spTree>").arg(shapes)), env.ctx, styles);
    }

    static ItemPtr dmlOne(DmlEnv &env, const QString &shape, const QVector<QString> &inherits = {})
    {
        QVector<QDomElement> from;
        for (const QString &x : inherits) from.append(env.element(x));
        return readShape(env.element(shape), env.ctx, {}, from);
    }

    template <typename T> static std::shared_ptr<T> dmlAs(const ItemPtr &it) { return std::dynamic_pointer_cast<T>(it); }

    static bool dmlRectIs(const QRectF &r, double x, double y, double w, double h, double tolerance = 1e-6)
    {
        return near(r.x(), x, tolerance) && near(r.y(), y, tolerance) && near(r.width(), w, tolerance) && near(r.height(), h, tolerance);
    }

    static QString dmlRectText(const QRectF &r) { return QStringLiteral("%1,%2 %3x%4").arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height()); }

private Q_SLOTS:
    void dmlShapeReadsPositionSizeAndTurn()
    {
        DmlEnv env;
        ItemPtr it = dmlOne(env, dmlSp(dmlXfrm(10, 20, 100, 50, QStringLiteral("rot=\"5400000\" flipH=\"1\"")) + QStringLiteral("<a:prstGeom prst=\"rect\"/>")));
        auto s = dmlAs<ShapeItem>(it);
        QVERIFY(s);
        QVERIFY2(dmlRectIs(s->rect, 10, 20, 100, 50), qPrintable(dmlRectText(s->rect)));
        QCOMPARE(s->rotation, 90.0);
        QVERIFY(s->flipH && !s->flipV);
        QCOMPARE(s->shape, QStringLiteral("rect"));
        QCOMPARE(s->name, QStringLiteral("Shape 2"));
        QCOMPARE(s->altText, QStringLiteral("alt 2"));
        QVERIFY(s->storyId.isEmpty());
        QVERIFY(s->wrap.mode == jp::Wrap::None);
        // A turn is read as 0 up to a full one, whichever way it's written; both flips together.
        for (const auto &row : QList<QPair<QString, double>>{{"0", 0}, {"-5400000", 270}, {"21600000", 0}, {"27000000", 90}, {"10800000", 180}, {"1800000", 30}, {"junk", 0}}) {
            it = dmlOne(env, dmlSp(dmlXfrm(0, 0, 10, 10, QStringLiteral("rot=\"%1\" flipH=\"true\" flipV=\"1\"").arg(row.first)) + QStringLiteral("<a:prstGeom prst=\"rect\"/>")));
            QVERIFY2(near(it->rotation, row.second), qPrintable(row.first));
            QVERIFY(it->flipH && it->flipV);
        }
        // Negative positions are fine, and a shape that says nothing of its place is a point at the origin.
        it = dmlOne(env, dmlSp(dmlXfrm(-30, -40, 5, 6) + QStringLiteral("<a:prstGeom prst=\"rect\"/>")));
        QVERIFY(dmlRectIs(it->rect, -30, -40, 5, 6));
        it = dmlOne(env, dmlSp(QStringLiteral("<a:prstGeom prst=\"rect\"/>")));
        QVERIFY2(it->rect.x() == 0 && it->rect.y() == 0 && it->rect.width() == 0 && it->rect.height() == 0, qPrintable(dmlRectText(it->rect)));
        // Strict files write lengths with units.
        it = dmlOne(env, dmlSp(QStringLiteral("<a:xfrm><a:off x=\"1in\" y=\"2cm\"/><a:ext cx=\"36pt\" cy=\"10mm\"/></a:xfrm><a:prstGeom prst=\"rect\"/>")));
        QVERIFY2(dmlRectIs(it->rect, 72, 72 * 2 / 2.54, 36, 72 * 10 / 25.4, 1e-4), qPrintable(dmlRectText(it->rect)));
    }

    void dmlShapeMapsPresetsAndTheirAdjustments()
    {
        DmlEnv env;
        const auto shape = [&](const QString &geometry, const QString &style = QString()) {
            return dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 50) + geometry + dmlSolid(QStringLiteral("FF0000")), QString(), style));
        };
        auto rr = dmlAs<ShapeItem>(shape(QStringLiteral("<a:prstGeom prst=\"roundRect\"><a:avLst/></a:prstGeom>")));
        QVERIFY(rr);
        QCOMPARE(rr->shape, QStringLiteral("roundRect"));
        QCOMPARE(rr->adj.size(), 1);
        QVERIFY(near(rr->adj[0], 0.16667, 1e-4));
        QVERIFY(rr->customPath.isEmpty());
        auto half = shape(QStringLiteral("<a:prstGeom prst=\"roundRect\"><a:avLst><a:gd name=\"adj\" fmla=\"val 50000\"/></a:avLst></a:prstGeom>"));
        QVERIFY(near(dmlAs<ShapeItem>(half)->adj[0], 0.5));
        auto arrow = shape(QStringLiteral("<a:prstGeom prst=\"rightArrow\"><a:avLst><a:gd name=\"adj1\" fmla=\"val 30000\"/><a:gd name=\"adj2\" fmla=\"val 60000\"/></a:avLst></a:prstGeom>"));
        QCOMPARE(dmlAs<ShapeItem>(arrow)->shape, QStringLiteral("rightArrow"));
        QVERIFY(near(dmlAs<ShapeItem>(arrow)->adj[0], 0.3) && near(dmlAs<ShapeItem>(arrow)->adj[1], 0.6));
        auto flow = dmlAs<ShapeItem>(shape(QStringLiteral("<a:prstGeom prst=\"flowChartDecision\"/>")));
        QCOMPARE(flow->shape, QStringLiteral("fcDecision"));
        QVERIFY(flow->adj.isEmpty());
        // The standard's defaults apply when it gives none, though the library's differ.
        auto star = dmlAs<ShapeItem>(shape(QStringLiteral("<a:prstGeom prst=\"star24\"/>")));
        QCOMPARE(star->shape, QStringLiteral("star24"));
        QVERIFY(near(star->adj[0], 0.75));
        // Geometry the library draws wrongly or not at all comes as the path the standard defines.
        auto pie = dmlAs<ShapeItem>(shape(QStringLiteral("<a:prstGeom prst=\"pie\"/>")));
        QCOMPARE(pie->shape, QStringLiteral("pie"));
        QVERIFY(!pie->customPath.isEmpty());
        QVERIFY(pie->customPath.contains(QPointF(25, 12.5)));
        QVERIFY(!pie->customPath.contains(QPointF(75, 12.5)));
        auto wedge = dmlAs<ShapeItem>(shape(QStringLiteral("<a:prstGeom prst=\"wedgeRectCallout\"/>")));
        QVERIFY(!wedge->customPath.isEmpty());
        QVERIFY(wedge->customPath.boundingRect().bottom() > 50);
        auto trap = dmlAs<ShapeItem>(shape(QStringLiteral("<a:prstGeom prst=\"nonIsoscelesTrapezoid\"/>")));
        QVERIFY(!trap->customPath.isEmpty());
        // A preset nobody draws is a rectangle, and so is one that isn't one.
        auto gear = dmlAs<ShapeItem>(shape(QStringLiteral("<a:prstGeom prst=\"gear6\"/>")));
        QCOMPARE(gear->shape, QStringLiteral("rect"));
        QVERIFY(gear->customPath.isEmpty());
        QCOMPARE(dmlAs<ShapeItem>(shape(QStringLiteral("<a:prstGeom prst=\"noSuchThing\"/>")))->shape, QStringLiteral("rect"));
        QCOMPARE(dmlAs<ShapeItem>(shape(QString()))->shape, QStringLiteral("rect"));
        // Fill and outline.
        QCOMPARE(dmlHex(rr->fill.color), QStringLiteral("FF0000"));
        QVERIFY(rr->stroke.isNone());
    }

    void dmlShapeReadsCustomGeometry()
    {
        DmlEnv env;
        const QString triangle = QStringLiteral("<a:custGeom><a:avLst/><a:gdLst/><a:ahLst/><a:cxnLst/><a:rect l=\"0\" t=\"0\" r=\"r\" b=\"b\"/><a:pathLst>"
                                                "<a:path w=\"100\" h=\"100\"><a:moveTo><a:pt x=\"0\" y=\"100\"/></a:moveTo><a:lnTo><a:pt x=\"50\" y=\"0\"/></a:lnTo>"
                                                "<a:lnTo><a:pt x=\"100\" y=\"100\"/></a:lnTo><a:close/></a:path></a:pathLst></a:custGeom>");
        ItemPtr it = dmlOne(env, dmlSp(dmlXfrm(10, 10, 200, 100) + triangle + dmlSolid(QStringLiteral("00FF00")) + QStringLiteral("<a:ln w=\"12700\">%1</a:ln>").arg(dmlSolid(QStringLiteral("000000")))));
        auto s = dmlAs<ShapeItem>(it);
        QVERIFY(s);
        QVERIFY2(dmlRectIs(s->customPath.boundingRect(), 0, 0, 200, 100), qPrintable(dmlRectText(s->customPath.boundingRect())));
        QVERIFY(s->customPath.contains(QPointF(100, 90)));
        QVERIFY(!s->customPath.contains(QPointF(5, 5)));
        QCOMPARE(dmlHex(s->fill.color), QStringLiteral("00FF00"));
        QVERIFY(!s->stroke.isNone());
        // Paths drawn without fill, or without outline, are their own objects so that each is drawn as it says.
        const QString two = QStringLiteral("<a:custGeom><a:pathLst><a:path w=\"10\" h=\"10\"><a:moveTo><a:pt x=\"0\" y=\"0\"/></a:moveTo><a:lnTo><a:pt x=\"10\" y=\"0\"/></a:lnTo>"
                                           "<a:lnTo><a:pt x=\"10\" y=\"10\"/></a:lnTo><a:close/></a:path>"
                                           "<a:path w=\"10\" h=\"10\" fill=\"none\"><a:moveTo><a:pt x=\"0\" y=\"10\"/></a:moveTo><a:lnTo><a:pt x=\"10\" y=\"10\"/></a:lnTo></a:path></a:pathLst></a:custGeom>");
        const QString looks = dmlSolid(QStringLiteral("00FF00")) + QStringLiteral("<a:ln w=\"12700\">%1</a:ln>").arg(dmlSolid(QStringLiteral("000000")));
        it = dmlOne(env, dmlSp(dmlXfrm(10, 10, 100, 100, QStringLiteral("rot=\"900000\"")) + two + looks, dmlBody()));
        auto g = dmlAs<GroupItem>(it);
        QVERIFY(g);
        QCOMPARE(int(g->children.size()), 2);
        auto first = dmlAs<ShapeItem>(g->children[0]);
        auto second = dmlAs<ShapeItem>(g->children[1]);
        QVERIFY(first && second);
        QVERIFY(!first->fill.isNone() && !first->stroke.isNone());
        QVERIFY(second->fill.isNone() && !second->stroke.isNone());
        QVERIFY(dmlRectIs(first->rect, 10, 10, 100, 100) && dmlRectIs(second->rect, 10, 10, 100, 100));
        QCOMPARE(first->rotation, 15.0);
        QCOMPARE(second->rotation, 15.0);
        QVERIFY(!first->storyId.isEmpty() && second->storyId.isEmpty());   // the text is in the first
        QCOMPARE(g->name, QStringLiteral("Shape 2"));
        // With no fill on the shape, the path that is only for fill has nothing to do.
        it = dmlOne(env, dmlSp(dmlXfrm(10, 10, 100, 100) + two + QStringLiteral("<a:noFill/><a:ln w=\"12700\">%1</a:ln>").arg(dmlSolid(QStringLiteral("000000")))));
        QVERIFY(dmlAs<ShapeItem>(it));
        // A callout is its box and its leader line.
        it = dmlOne(env, dmlSp(dmlXfrm(100, 100, 200, 100) + QStringLiteral("<a:prstGeom prst=\"borderCallout1\"/>") + looks));
        g = dmlAs<GroupItem>(it);
        QVERIFY(g);
        QCOMPARE(int(g->children.size()), 2);
        // A line callout with no outline draws only the fill.
        it = dmlOne(env, dmlSp(dmlXfrm(100, 100, 200, 100) + QStringLiteral("<a:prstGeom prst=\"callout1\"/>") + dmlSolid(QStringLiteral("00FF00")) + QStringLiteral("<a:ln><a:noFill/></a:ln>")));
        QVERIFY(dmlAs<ShapeItem>(it));
        QVERIFY(dmlAs<ShapeItem>(it)->stroke.isNone());
    }

    void dmlShapeBecomesATextBoxOrAShapeWithText()
    {
        DmlEnv env;
        const QString rect = dmlXfrm(10, 10, 100, 40) + QStringLiteral("<a:prstGeom prst=\"rect\"/>");
        // A text box.
        ItemPtr it = dmlOne(env, dmlSp(rect, dmlBody(), QString(), QStringLiteral("txBox=\"1\"")));
        auto t = dmlAs<TextItem>(it);
        QVERIFY(t);
        QVERIFY(!t->storyId.isEmpty());
        QVERIFY(env.stories[0]->toPlainText().contains(QStringLiteral("Hello")));
        QVERIFY(t->fill.isNone() && t->stroke.isNone());
        QVERIFY(!t->hyphenate);
        QVERIFY(t->wrap.mode == jp::Wrap::None);
        // An empty one is a text box all the same, with a story to type into.
        it = dmlOne(env, dmlSp(rect, QStringLiteral("<p:txBody><a:bodyPr/><a:p/></p:txBody>"), QString(), QStringLiteral("txBox=\"1\"")));
        QVERIFY(dmlAs<TextItem>(it) && !dmlAs<TextItem>(it)->storyId.isEmpty());
        it = dmlOne(env, dmlSp(rect, QString(), QString(), QStringLiteral("txBox=\"1\"")));
        QVERIFY(dmlAs<TextItem>(it));
        // A placeholder, with its fill and line.
        it = dmlOne(env, dmlSp(rect + dmlSolid(QStringLiteral("123456")) + QStringLiteral("<a:ln w=\"25400\">%1</a:ln>").arg(dmlSolid(QStringLiteral("654321"))), dmlBody(),
                               QString(), QString(), QStringLiteral("<p:ph type=\"title\"/>")));
        t = dmlAs<TextItem>(it);
        QVERIFY(t);
        QCOMPARE(dmlHex(t->fill.color), QStringLiteral("123456"));
        QVERIFY(near(t->stroke.width, 2));
        // A placeholder with nothing in it still is one (and has no geometry of its own to say).
        it = dmlOne(env, dmlSp(QString(), QString(), QString(), QString(), QStringLiteral("<p:ph type=\"body\" idx=\"1\"/>")));
        QVERIFY(dmlAs<TextItem>(it));
        // A rectangle with text, and one without.
        it = dmlOne(env, dmlSp(rect + dmlSolid(QStringLiteral("FF0000")), dmlBody()));
        QVERIFY(dmlAs<TextItem>(it));
        QCOMPARE(dmlHex(it->fill.color), QStringLiteral("FF0000"));
        it = dmlOne(env, dmlSp(rect, QStringLiteral("<p:txBody><a:bodyPr/><a:p/></p:txBody>")));
        QVERIFY(dmlAs<ShapeItem>(it) && dmlAs<ShapeItem>(it)->storyId.isEmpty());
        // Text in another shape stays in the shape.
        it = dmlOne(env, dmlSp(dmlXfrm(10, 10, 100, 40) + QStringLiteral("<a:prstGeom prst=\"roundRect\"/>"), dmlBody(QStringLiteral("Round"))));
        auto s = dmlAs<ShapeItem>(it);
        QVERIFY(s && !s->storyId.isEmpty());
        QVERIFY(env.stories.last()->toPlainText().contains(QStringLiteral("Round")));
        QCOMPARE(s->shape, QStringLiteral("roundRect"));
        // A text box that isn't a rectangle is a shape with text, and one in a shape that is not a text box is not.
        it = dmlOne(env, dmlSp(dmlXfrm(10, 10, 100, 40) + QStringLiteral("<a:prstGeom prst=\"ellipse\"/>"), dmlBody(), QString(), QStringLiteral("txBox=\"1\"")));
        QVERIFY(dmlAs<ShapeItem>(it) && !dmlAs<ShapeItem>(it)->storyId.isEmpty());
        // An empty picture placeholder is a picture with no picture.
        it = dmlOne(env, dmlSp(QString(), QString(), QString(), QString(), QStringLiteral("<p:ph type=\"pic\" idx=\"13\"/>")));
        auto p = dmlAs<PictureItem>(it);
        QVERIFY(p && p->imageId.isEmpty());
        // Without a way to make stories, a text box has none and is no worse for it.
        DmlEnv bare;
        bare.ctx.newStory = nullptr;
        it = dmlOne(bare, dmlSp(rect, dmlBody(), QString(), QStringLiteral("txBox=\"1\"")));
        QVERIFY(dmlAs<TextItem>(it) && dmlAs<TextItem>(it)->storyId.isEmpty());
    }

    void dmlShapeReadsTextBoxProperties()
    {
        DmlEnv env;
        const QString rect = dmlXfrm(10, 10, 100, 40) + QStringLiteral("<a:prstGeom prst=\"rect\"/>");
        const auto box = [&](const QString &bodyPr, const QString &geometry = QString(), const QVector<QString> &inherits = {}) {
            const QString g = geometry.isEmpty() ? rect : dmlXfrm(10, 10, 100, 40) + geometry;
            return dmlOne(env, dmlSp(g, dmlBody(QStringLiteral("x"), bodyPr), QString(), QStringLiteral("txBox=\"1\""), QString()), inherits);
        };
        // The standard's insets and top anchor, which a shape or a text box has with nothing said.
        auto t = dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr/>")));
        QVERIFY(t);
        QCOMPARE(t->insets.left(), 7.2);
        QCOMPARE(t->insets.top(), 3.6);
        QCOMPARE(t->insets.right(), 7.2);
        QCOMPARE(t->insets.bottom(), 3.6);
        QVERIFY(t->valign == VAlign::Top);
        QVERIFY(t->autofit == TextItem::NoAutofit);
        QCOMPARE(t->columns, 1);
        QVERIFY(!t->vertical);
        auto s = dmlAs<ShapeItem>(box(QStringLiteral("<a:bodyPr/>"), QStringLiteral("<a:prstGeom prst=\"ellipse\"/>")));
        QVERIFY(s);
        QCOMPARE(s->insets.left(), 7.2);
        QCOMPARE(s->insets.top(), 3.6);
        QVERIFY(s->valign == VAlign::Top);
        // Each said inset, including none.
        t = dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr lIns=\"0\" tIns=\"12700\" rIns=\"25400\" bIns=\"38100\"/>")));
        QCOMPARE(t->insets.left(), 0.0);
        QCOMPARE(t->insets.top(), 1.0);
        QCOMPARE(t->insets.right(), 2.0);
        QCOMPARE(t->insets.bottom(), 3.0);
        s = dmlAs<ShapeItem>(box(QStringLiteral("<a:bodyPr lIns=\"12700\" bIns=\"0\"/>"), QStringLiteral("<a:prstGeom prst=\"ellipse\"/>")));
        QCOMPARE(s->insets.left(), 1.0);
        QCOMPARE(s->insets.bottom(), 0.0);
        QCOMPARE(s->insets.top(), 3.6);
        // Anchors.
        QVERIFY(dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr anchor=\"ctr\"/>")))->valign == VAlign::Middle);
        QVERIFY(dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr anchor=\"b\"/>")))->valign == VAlign::Bottom);
        QVERIFY(dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr anchor=\"t\"/>")))->valign == VAlign::Top);
        QVERIFY(dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr anchor=\"just\"/>")))->valign == VAlign::Top);
        QVERIFY(dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr anchor=\"nonsense\"/>")))->valign == VAlign::Top);
        QVERIFY(dmlAs<ShapeItem>(box(QStringLiteral("<a:bodyPr anchor=\"ctr\"/>"), QStringLiteral("<a:prstGeom prst=\"ellipse\"/>")))->valign == VAlign::Middle);
        QVERIFY(dmlAs<ShapeItem>(box(QStringLiteral("<a:bodyPr anchor=\"b\"/>"), QStringLiteral("<a:prstGeom prst=\"ellipse\"/>")))->valign == VAlign::Bottom);
        // Fitting the text to the box.
        t = dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr><a:normAutofit fontScale=\"62500\" lnSpcReduction=\"20000\"/></a:bodyPr>")));
        QVERIFY(t->autofit == TextItem::ShrinkOnOverflow);
        QVERIFY(near(t->fitScale, 0.625));
        t = dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr><a:normAutofit/></a:bodyPr>")));
        QVERIFY(t->autofit == TextItem::ShrinkOnOverflow);
        QCOMPARE(t->fitScale, 1.0);
        t = dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr><a:normAutofit fontScale=\"5\"/></a:bodyPr>")));
        QVERIFY(t->fitScale > 0 && t->fitScale <= 1);
        t = dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr><a:normAutofit fontScale=\"500000\"/></a:bodyPr>")));
        QVERIFY(t->fitScale > 0 && t->fitScale <= 1);
        QVERIFY(dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr><a:spAutoFit/></a:bodyPr>")))->autofit == TextItem::GrowBox);
        QVERIFY(dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr><a:noAutofit/></a:bodyPr>")))->autofit == TextItem::NoAutofit);
        // Columns, and the way the text runs.
        t = dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr numCol=\"3\" spcCol=\"228600\"/>")));
        QCOMPARE(t->columns, 3);
        QCOMPARE(t->columnGap, 18.0);
        QCOMPARE(dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr numCol=\"99999\"/>")))->columns <= 16, true);
        QCOMPARE(dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr numCol=\"0\"/>")))->columns, 1);
        QCOMPARE(dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr numCol=\"-4\"/>")))->columns, 1);
        QVERIFY(dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr vert=\"vert\"/>")))->vertical);
        QVERIFY(dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr vert=\"vert270\"/>")))->vertical);
        QVERIFY(!dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr vert=\"horz\"/>")))->vertical);
        QVERIFY(dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr wrap=\"none\"/>"))));
        // What a placeholder's own body leaves out comes from the layout's, and then from the master's.
        const QString layout = dmlSp(dmlXfrm(1, 2, 3, 4), QStringLiteral("<p:txBody><a:bodyPr lIns=\"0\" anchor=\"ctr\"><a:normAutofit/></a:bodyPr><a:p/></p:txBody>"));
        const QString master = dmlSp(dmlXfrm(1, 2, 3, 4), QStringLiteral("<p:txBody><a:bodyPr tIns=\"25400\" anchor=\"b\" rIns=\"12700\"><a:spAutoFit/></a:bodyPr><a:p/></p:txBody>"));
        t = dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr rIns=\"38100\"/>"), QString(), {layout, master}));
        QCOMPARE(t->insets.left(), 0.0);        // the layout's
        QCOMPARE(t->insets.top(), 2.0);         // the master's, which the layout lacks
        QCOMPARE(t->insets.right(), 3.0);       // its own
        QCOMPARE(t->insets.bottom(), 3.6);      // nobody's
        QVERIFY(t->valign == VAlign::Middle);   // the layout's, over the master's
        QVERIFY(t->autofit == TextItem::ShrinkOnOverflow);
        t = dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr anchor=\"t\"><a:noAutofit/></a:bodyPr>"), QString(), {layout, master}));
        QVERIFY(t->valign == VAlign::Top);
        QVERIFY(t->autofit == TextItem::NoAutofit);
    }

    void dmlShapeTakesWhatItInheritsFrom()
    {
        DmlEnv env;
        // A placeholder that says nothing is placed, shaped, filled, and lined as the layout's, else the master's.
        const QString layout = dmlSp(dmlXfrm(50, 60, 300, 100, QStringLiteral("rot=\"600000\"")) + dmlSolid(QStringLiteral("00FF00")),
                                     QString(), QString(), QString(), QStringLiteral("<p:ph type=\"body\" idx=\"1\"/>"));
        const QString master = dmlSp(dmlXfrm(1, 2, 3, 4) + QStringLiteral("<a:prstGeom prst=\"rect\"/>") + dmlSolid(QStringLiteral("0000FF")) +
                                     QStringLiteral("<a:ln w=\"25400\">%1</a:ln>").arg(dmlSolid(QStringLiteral("FF00FF"))), QString(), QString(), QString(), QStringLiteral("<p:ph type=\"body\" idx=\"1\"/>"));
        ItemPtr it = dmlOne(env, dmlSp(QString(), dmlBody(), QString(), QString(), QStringLiteral("<p:ph type=\"body\" idx=\"1\"/>")), {layout, master});
        auto t = dmlAs<TextItem>(it);
        QVERIFY(t);   // a placeholder is a text frame
        QVERIFY2(dmlRectIs(t->rect, 50, 60, 300, 100), qPrintable(dmlRectText(t->rect)));
        QVERIFY(near(t->rotation, 10));
        QCOMPARE(dmlHex(t->fill.color), QStringLiteral("00FF00"));       // the layout's fill
        QVERIFY(near(t->stroke.width, 2));                                // and the master's line
        QCOMPARE(dmlHex(t->stroke.color), QStringLiteral("FF00FF"));
        // Said itself, it's its own.
        it = dmlOne(env, dmlSp(dmlXfrm(5, 6, 7, 8) + dmlSolid(QStringLiteral("123456")) + QStringLiteral("<a:ln><a:noFill/></a:ln>"), dmlBody(), QString(), QString(),
                               QStringLiteral("<p:ph type=\"body\" idx=\"1\"/>")), {layout, master});
        QVERIFY(dmlRectIs(it->rect, 5, 6, 7, 8));
        QCOMPARE(dmlHex(it->fill.color), QStringLiteral("123456"));
        QVERIFY(it->stroke.isNone());
        // A shape inheriting a preset takes its shape, and its adjustments.
        const QString shaped = dmlSp(dmlXfrm(50, 60, 300, 100) + QStringLiteral("<a:prstGeom prst=\"roundRect\"><a:avLst><a:gd name=\"adj\" fmla=\"val 40000\"/></a:avLst></a:prstGeom>"));
        it = dmlOne(env, dmlSp(QString(), dmlBody(QStringLiteral("x")), QString(), QString(), QStringLiteral("<p:ph type=\"title\"/>")), {shaped});
        QVERIFY(dmlAs<ShapeItem>(it) && !dmlAs<ShapeItem>(it)->storyId.isEmpty());   // a placeholder that isn't a rectangle holds its text as a shape does
        it = dmlOne(env, dmlSp(QString(), QString(), QString(), QString(), QString()), {shaped});
        QCOMPARE(dmlAs<ShapeItem>(it)->shape, QStringLiteral("roundRect"));
        QVERIFY(near(dmlAs<ShapeItem>(it)->adj[0], 0.4));
        // A picture placeholder takes the position of the layout's too.
        const QString pic = dmlSp(dmlXfrm(11, 12, 130, 140), QString(), QString(), QString(), QStringLiteral("<p:ph type=\"pic\" idx=\"13\"/>"));
        it = dmlOne(env, QStringLiteral("<p:pic><p:nvPicPr><p:cNvPr id=\"4\" name=\"P\"/><p:cNvPicPr/><p:nvPr><p:ph type=\"pic\" idx=\"13\"/></p:nvPr></p:nvPicPr>"
                                        "<p:blipFill><a:blip/></p:blipFill><p:spPr/></p:pic>"), {pic});
        QVERIFY(dmlAs<PictureItem>(it));
        QVERIFY(dmlRectIs(it->rect, 11, 12, 130, 140));
    }

    void dmlShapeTakesItsStyle()
    {
        DmlEnv env;
        env.useFormats();
        const QString style = dmlStyle(QStringLiteral("2"), QStringLiteral("1"), QStringLiteral("3"));
        ItemPtr it = dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 50) + QStringLiteral("<a:prstGeom prst=\"rect\"/>"), QString(), style));
        auto s = dmlAs<ShapeItem>(it);
        QCOMPARE(dmlHex(s->fill.color), QStringLiteral("4472C4"));
        QCOMPARE(dmlHex(s->stroke.color), QStringLiteral("4472C4"));
        QVERIFY(near(s->stroke.width, 1));
        QVERIFY(s->stroke.dash == Stroke::DashLine);
        // A shadow from effect style 3 (a 4.5 point blur, 1.5 points down, 63% opaque black).
        QVERIFY(s->fx.shadow.on);
        QVERIFY(near(s->fx.shadow.blur, 4.5, 1e-6) && near(s->fx.shadow.distance, 1.5, 1e-6) && near(s->fx.shadow.angle, 90));
        QVERIFY(near(s->fx.shadow.transparency, 0.37, 2e-2));
        // The shape's own say overrides it all; an empty effect list takes the shadow away.
        it = dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 50) + QStringLiteral("<a:prstGeom prst=\"rect\"/><a:noFill/><a:ln><a:noFill/></a:ln><a:effectLst/>"), QString(), style));
        QVERIFY(it->fill.isNone() && it->stroke.isNone() && !it->fx.shadow.on);
        // A text box with no style has no fill or line, as it says.
        it = dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 50) + QStringLiteral("<a:prstGeom prst=\"rect\"/>"), dmlBody(), QString(), QStringLiteral("txBox=\"1\"")));
        QVERIFY(it->fill.isNone() && it->stroke.isNone());
    }

    void dmlShapeReadsEffects()
    {
        DmlEnv env;
        const QString shadow = QStringLiteral("<a:effectLst><a:outerShdw blurRad=\"50800\" dist=\"38100\" dir=\"2700000\" algn=\"tl\" rotWithShape=\"0\"><a:srgbClr val=\"FF0000\"><a:alpha val=\"40000\"/></a:srgbClr></a:outerShdw>"
                                              "<a:glow rad=\"101600\"><a:srgbClr val=\"00FF00\"><a:alpha val=\"60000\"/></a:srgbClr></a:glow><a:softEdge rad=\"63500\"/><a:reflection/></a:effectLst>");
        ItemPtr it = dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 50) + QStringLiteral("<a:prstGeom prst=\"rect\"/>") + shadow));
        QVERIFY(it->fx.shadow.on && !it->fx.shadow.inner);
        QVERIFY(near(it->fx.shadow.blur, 4) && near(it->fx.shadow.distance, 3) && near(it->fx.shadow.angle, 45));
        QCOMPARE(dmlHex(it->fx.shadow.color), QStringLiteral("FF0000"));
        QVERIFY(near(it->fx.shadow.transparency, 0.6, 2e-2));
        QVERIFY(it->fx.glow.on && near(it->fx.glow.size, 8) && near(it->fx.glow.transparency, 0.4, 2e-2));
        QCOMPARE(dmlHex(it->fx.glow.color), QStringLiteral("00FF00"));
        QVERIFY(near(it->fx.softEdge, 5));
        QVERIFY(!it->fx.reflection.on);
        // An inner shadow, when it's the only one.
        it = dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 50) + QStringLiteral("<a:prstGeom prst=\"rect\"/><a:effectLst><a:innerShdw blurRad=\"12700\" dist=\"12700\" dir=\"0\"><a:srgbClr val=\"000000\"/></a:innerShdw></a:effectLst>")));
        QVERIFY(it->fx.shadow.on && it->fx.shadow.inner);
        // Hostile numbers are held to what a page can show.
        it = dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 50) + QStringLiteral("<a:prstGeom prst=\"rect\"/><a:effectLst><a:outerShdw blurRad=\"99999999999999\" dist=\"-5\" dir=\"junk\"><a:srgbClr val=\"000000\"/></a:outerShdw>"
                                                                       "<a:softEdge rad=\"99999999999999\"/></a:effectLst>")));
        QVERIFY(it->fx.shadow.blur <= 1584 && it->fx.shadow.distance >= 0 && it->fx.softEdge <= 1584);
        QVERIFY(std::isfinite(it->fx.shadow.angle));
        // None is none.
        it = dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 50) + QStringLiteral("<a:prstGeom prst=\"rect\"/>")));
        QVERIFY(!it->fx.any());
    }

    void dmlShapeScalesAGradientAngleToItsShape()
    {
        DmlEnv env;
        const QString grad = QStringLiteral("<a:gradFill><a:gsLst><a:gs pos=\"0\"><a:srgbClr val=\"FF0000\"/></a:gs><a:gs pos=\"100000\"><a:srgbClr val=\"0000FF\"/></a:gs></a:gsLst>"
                                            "<a:lin ang=\"2700000\" scaled=\"%1\"/></a:gradFill>");
        ItemPtr it = dmlOne(env, dmlSp(dmlXfrm(0, 0, 400, 100) + QStringLiteral("<a:prstGeom prst=\"rect\"/>") + grad.arg(QStringLiteral("1"))));
        QVERIFY2(near(it->fill.angle, std::atan2(1.0 / 100, 1.0 / 400) * 180 / M_PI, 1e-6), qPrintable(QString::number(it->fill.angle)));
        it = dmlOne(env, dmlSp(dmlXfrm(0, 0, 400, 100) + QStringLiteral("<a:prstGeom prst=\"rect\"/>") + grad.arg(QStringLiteral("0"))));
        QVERIFY(near(it->fill.angle, 45));
        it = dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 100) + QStringLiteral("<a:prstGeom prst=\"rect\"/>") + grad.arg(QStringLiteral("1"))));
        QVERIFY(near(it->fill.angle, 45, 1e-6));
    }

    void dmlGroupMapsItsChildrenIntoItsFrame()
    {
        DmlEnv env;
        const auto rect = [](double x, double y, double w, double h, const QString &attrs = QString(), const QString &id = QStringLiteral("2")) {
            return dmlSp(dmlXfrm(x, y, w, h, attrs) + QStringLiteral("<a:prstGeom prst=\"rect\"/>"), QString(), QString(), QString(), QString(), QString(), id);
        };
        // Twice the size, and moved: the child space is 100 by 50, the frame 200 by 100.
        ItemList list = dmlTree(env, dmlGrp(dmlGroupFrame(QRectF(100, 100, 200, 100), QRectF(0, 0, 100, 50)), rect(10, 10, 20, 10)));
        QCOMPARE(int(list.size()), 1);
        auto g = dmlAs<GroupItem>(list[0]);
        QVERIFY(g);
        QCOMPARE(int(g->children.size()), 1);
        QVERIFY2(dmlRectIs(g->children[0]->rect, 120, 120, 40, 20), qPrintable(dmlRectText(g->children[0]->rect)));
        QVERIFY2(dmlRectIs(g->rect, 120, 120, 40, 20), qPrintable(dmlRectText(g->rect)));
        QCOMPARE(g->name, QStringLiteral("Group 20"));
        QCOMPARE(g->altText, QStringLiteral("group alt"));
        // The child space doesn't start at the origin.
        list = dmlTree(env, dmlGrp(dmlGroupFrame(QRectF(0, 0, 100, 100), QRectF(50, 50, 50, 50)), rect(60, 70, 10, 10)));
        QVERIFY2(dmlRectIs(dmlAs<GroupItem>(list[0])->children[0]->rect, 20, 40, 20, 20), qPrintable(dmlRectText(dmlAs<GroupItem>(list[0])->children[0]->rect)));
        // A different scale on each side, and a child turned a quarter: its width follows the other scale.
        list = dmlTree(env, dmlGrp(dmlGroupFrame(QRectF(0, 0, 200, 100), QRectF(0, 0, 100, 100)), rect(10, 10, 20, 20, QStringLiteral("rot=\"5400000\""))));
        const ItemPtr turned = dmlAs<GroupItem>(list[0])->children[0];
        QVERIFY2(dmlRectIs(turned->rect, 30, 0, 20, 40, 1e-6), qPrintable(dmlRectText(turned->rect)));
        QVERIFY(near(turned->rotation, 90));
        list = dmlTree(env, dmlGrp(dmlGroupFrame(QRectF(0, 0, 200, 100), QRectF(0, 0, 100, 100)), rect(10, 10, 20, 10)));
        QVERIFY(dmlRectIs(dmlAs<GroupItem>(list[0])->children[0]->rect, 20, 10, 40, 10));
        // The group turned a quarter about the middle of its frame: its children go round with it.
        list = dmlTree(env, dmlGrp(dmlGroupFrame(QRectF(100, 100, 200, 100), QRectF(0, 0, 100, 50), QStringLiteral("rot=\"5400000\"")), rect(10, 10, 20, 10)));
        const ItemPtr round = dmlAs<GroupItem>(list[0])->children[0];
        QVERIFY2(dmlRectIs(round->rect, 200, 80, 40, 20, 1e-6), qPrintable(dmlRectText(round->rect)));
        QVERIFY(near(round->rotation, 90));
        // Turned itself, a child's turn adds to the group's.
        list = dmlTree(env, dmlGrp(dmlGroupFrame(QRectF(100, 100, 200, 100), QRectF(0, 0, 100, 50), QStringLiteral("rot=\"5400000\"")),
                                    rect(10, 10, 20, 10, QStringLiteral("rot=\"1800000\""))));
        QVERIFY(near(dmlAs<GroupItem>(list[0])->children[0]->rotation, 120));
        // Flipped, a child goes to the other side, and is flipped too (its own turn reverses).
        list = dmlTree(env, dmlGrp(dmlGroupFrame(QRectF(100, 100, 200, 100), QRectF(0, 0, 100, 50), QStringLiteral("flipH=\"1\"")),
                                    rect(10, 10, 20, 10) + rect(10, 10, 20, 10, QStringLiteral("rot=\"1800000\""))));
        g = dmlAs<GroupItem>(list[0]);
        QVERIFY2(dmlRectIs(g->children[0]->rect, 240, 120, 40, 20), qPrintable(dmlRectText(g->children[0]->rect)));
        QVERIFY(g->children[0]->flipH && !g->children[0]->flipV);
        QVERIFY(g->children[1]->flipH);
        QVERIFY(near(g->children[1]->rotation, 330));
        list = dmlTree(env, dmlGrp(dmlGroupFrame(QRectF(100, 100, 200, 100), QRectF(0, 0, 100, 50), QStringLiteral("flipV=\"1\"")), rect(10, 10, 20, 10)));
        QVERIFY2(dmlRectIs(dmlAs<GroupItem>(list[0])->children[0]->rect, 120, 160, 40, 20), qPrintable(dmlRectText(dmlAs<GroupItem>(list[0])->children[0]->rect)));
        QVERIFY(dmlAs<GroupItem>(list[0])->children[0]->flipV);
        // Groups in groups: each maps into the next.
        const QString inner = dmlGrp(dmlGroupFrame(QRectF(10, 10, 20, 20), QRectF(0, 0, 20, 20)), rect(0, 0, 10, 10), QStringLiteral("21"));
        list = dmlTree(env, dmlGrp(dmlGroupFrame(QRectF(0, 0, 200, 200), QRectF(0, 0, 100, 100)), inner));
        auto outer = dmlAs<GroupItem>(list[0]);
        QVERIFY(outer);
        QVERIFY2(dmlRectIs(outer->children[0]->rect, 20, 20, 20, 20), qPrintable(dmlRectText(outer->children[0]->rect)));
        QVERIFY2(dmlRectIs(dmlAs<GroupItem>(outer->children[0])->children[0]->rect, 20, 20, 20, 20), qPrintable(dmlRectText(outer->children[0]->rect)));
        const QString innerTurned = dmlGrp(dmlGroupFrame(QRectF(10, 10, 20, 20), QRectF(0, 0, 20, 20), QStringLiteral("rot=\"5400000\"")), rect(0, 0, 10, 10), QStringLiteral("21"));
        list = dmlTree(env, dmlGrp(dmlGroupFrame(QRectF(0, 0, 200, 200), QRectF(0, 0, 100, 100)), innerTurned));
        outer = dmlAs<GroupItem>(list[0]);
        QVERIFY2(dmlRectIs(outer->children[0]->rect, 40, 20, 20, 20, 1e-6), qPrintable(dmlRectText(outer->children[0]->rect)));
        QVERIFY(near(dmlAs<GroupItem>(outer->children[0])->children[0]->rotation, 90));
        // A group that says nothing of its frame leaves what is in it where it is; one with no child space is not scaled.
        list = dmlTree(env, QStringLiteral("<p:grpSp><p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"g\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr/>") + rect(10, 20, 30, 40) + QStringLiteral("</p:grpSp>"));
        QVERIFY(dmlRectIs(dmlAs<GroupItem>(list[0])->children[0]->rect, 10, 20, 30, 40));
        list = dmlTree(env, dmlGrp(dmlGroupFrame(QRectF(10, 10, 100, 100), QRectF(0, 0, 0, 0)), rect(5, 5, 20, 20)));
        QVERIFY2(dmlRectIs(dmlAs<GroupItem>(list[0])->children[0]->rect, 15, 15, 20, 20), qPrintable(dmlRectText(dmlAs<GroupItem>(list[0])->children[0]->rect)));
        // No children, no group.
        QVERIFY(dmlTree(env, dmlGrp(dmlGroupFrame(QRectF(0, 0, 10, 10), QRectF(0, 0, 10, 10)), QString())).empty());
        QVERIFY(!dmlOne(env, dmlGrp(dmlGroupFrame(QRectF(0, 0, 10, 10), QRectF(0, 0, 10, 10)), QString())));
        // A group is a shape too.
        QVERIFY(dmlAs<GroupItem>(dmlOne(env, dmlGrp(dmlGroupFrame(QRectF(0, 0, 10, 10), QRectF(0, 0, 10, 10)), rect(0, 0, 5, 5)))));
    }

    void dmlGroupScalesWhatIsInIt()
    {
        DmlEnv env;
        env.usePackage();
        const QString frame = dmlGroupFrame(QRectF(100, 100, 200, 100), QRectF(0, 0, 100, 50));
        const QString triangle = QStringLiteral("<a:custGeom><a:pathLst><a:path w=\"100\" h=\"100\"><a:moveTo><a:pt x=\"0\" y=\"100\"/></a:moveTo><a:lnTo><a:pt x=\"50\" y=\"0\"/></a:lnTo>"
                                                "<a:lnTo><a:pt x=\"100\" y=\"100\"/></a:lnTo><a:close/></a:path></a:pathLst></a:custGeom>");
        const QString crop = QStringLiteral("<a:srcRect l=\"10000\" r=\"10000\"/>");
        ItemList list = dmlTree(env, dmlGrp(frame, dmlSp(dmlXfrm(0, 0, 20, 10) + triangle + dmlSolid(QStringLiteral("FF0000"))) +
                                                    dmlCxn(dmlXfrm(10, 10, 40, 20) + QStringLiteral("<a:prstGeom prst=\"line\"/>")) +
                                                    dmlPic(QString(), dmlXfrm(0, 20, 50, 20) + QStringLiteral("<a:prstGeom prst=\"rect\"/>"), crop)));
        auto g = dmlAs<GroupItem>(list[0]);
        QVERIFY(g && g->children.size() == 3);
        auto s = dmlAs<ShapeItem>(g->children[0]);
        QVERIFY(s);
        QVERIFY2(dmlRectIs(s->rect, 100, 100, 40, 20), qPrintable(dmlRectText(s->rect)));
        QVERIFY2(dmlRectIs(s->customPath.boundingRect(), 0, 0, 40, 20), qPrintable(dmlRectText(s->customPath.boundingRect())));
        auto l = dmlAs<LineItem>(g->children[1]);
        QVERIFY(l);
        QVERIFY(near(l->p1.x(), 120) && near(l->p1.y(), 120) && near(l->p2.x(), 200) && near(l->p2.y(), 160));
        auto p = dmlAs<PictureItem>(g->children[2]);
        QVERIFY(p);
        QVERIFY2(dmlRectIs(p->rect, 100, 140, 100, 40), qPrintable(dmlRectText(p->rect)));
        // The picture is 125 wide behind a 100 frame cropped a tenth each side: still so in the larger frame.
        QVERIFY2(dmlRectIs(p->imgRect, -12.5, 0, 125, 40), qPrintable(dmlRectText(p->imgRect)));
        // Lines and shadows keep their weight.
        list = dmlTree(env, dmlGrp(frame, dmlSp(dmlXfrm(0, 0, 20, 10) + QStringLiteral("<a:prstGeom prst=\"rect\"/><a:ln w=\"25400\">%1</a:ln>").arg(dmlSolid(QStringLiteral("000000"))))));
        QVERIFY(near(dmlAs<GroupItem>(list[0])->children[0]->stroke.width, 2));
    }

    void dmlGroupPassesItsFillDown()
    {
        DmlEnv env;
        const QString inGroup = dmlSp(dmlXfrm(0, 0, 10, 10) + QStringLiteral("<a:prstGeom prst=\"rect\"/><a:grpFill/>"));
        const QString frame = dmlGroupFrame(QRectF(0, 0, 100, 100), QRectF(0, 0, 100, 100));
        ItemList list = dmlTree(env, dmlGrp(frame + dmlSolid(QStringLiteral("00FF00")), inGroup));
        QCOMPARE(dmlHex(dmlAs<GroupItem>(list[0])->children[0]->fill.color), QStringLiteral("00FF00"));
        // A group in the group, with no fill of its own, passes the outer's on.
        list = dmlTree(env, dmlGrp(frame + dmlSolid(QStringLiteral("00FF00")), dmlGrp(frame, inGroup, QStringLiteral("21"))));
        QCOMPARE(dmlHex(dmlAs<GroupItem>(dmlAs<GroupItem>(list[0])->children[0])->children[0]->fill.color), QStringLiteral("00FF00"));
        // One with its own fill is a different group's.
        list = dmlTree(env, dmlGrp(frame + dmlSolid(QStringLiteral("00FF00")), dmlGrp(frame + dmlSolid(QStringLiteral("0000FF")), inGroup, QStringLiteral("21"))));
        QCOMPARE(dmlHex(dmlAs<GroupItem>(dmlAs<GroupItem>(list[0])->children[0])->children[0]->fill.color), QStringLiteral("0000FF"));
        // A group with no fill gives none, and one outside groups has no group's to take.
        list = dmlTree(env, dmlGrp(frame, inGroup));
        QVERIFY(dmlAs<GroupItem>(list[0])->children[0]->fill.isNone());
        list = dmlTree(env, inGroup);
        QVERIFY(list[0]->fill.isNone());
    }

    void dmlPictureReadsCropMaskAndAdjustments()
    {
        DmlEnv env;
        env.usePackage();
        const QString frame = dmlXfrm(10, 20, 100, 50);
        const auto pic = [&](const QString &blip, const QString &spPr = QString(), const QString &blipFill = QStringLiteral("<a:stretch><a:fillRect/></a:stretch>")) {
            return dmlAs<PictureItem>(dmlOne(env, dmlPic(blip, frame + (spPr.isEmpty() ? QStringLiteral("<a:prstGeom prst=\"rect\"/>") : spPr), blipFill)));
        };
        auto p = pic(QString());
        QVERIFY(p);
        QCOMPARE(p->imageId, QStringLiteral("img1"));
        QCOMPARE(env.images.size(), 1);
        QVERIFY2(dmlRectIs(p->rect, 10, 20, 100, 50), qPrintable(dmlRectText(p->rect)));
        QVERIFY2(dmlRectIs(p->imgRect, 0, 0, 100, 50), qPrintable(dmlRectText(p->imgRect)));
        QCOMPARE(p->maskShape, QStringLiteral("rect"));
        QCOMPARE(p->name, QStringLiteral("Picture 5"));
        QCOMPARE(p->altText, QStringLiteral("a picture"));
        QVERIFY(p->wrap.mode == jp::Wrap::None);
        QVERIFY(p->recolor == PictureItem::NoRecolor && p->transparency == 0 && p->brightness == 0 && p->contrast == 0 && !p->hasTransparentColor);
        // A tenth cut from the left and right, a fifth from the bottom: the whole picture is 125 wide and 62.5 high, begun 12.5 left of the frame.
        p = pic(QString(), QString(), QStringLiteral("<a:srcRect l=\"10000\" r=\"10000\" b=\"20000\"/><a:stretch><a:fillRect/></a:stretch>"));
        QVERIFY2(dmlRectIs(p->imgRect, -12.5, 0, 125, 62.5), qPrintable(dmlRectText(p->imgRect)));
        // Cut from the top, too.
        p = pic(QString(), QString(), QStringLiteral("<a:srcRect t=\"50000\"/><a:stretch><a:fillRect/></a:stretch>"));
        QVERIFY2(dmlRectIs(p->imgRect, 0, -50, 100, 100), qPrintable(dmlRectText(p->imgRect)));
        // Negative cuts are space around the picture.
        p = pic(QString(), QString(), QStringLiteral("<a:srcRect l=\"-20000\"/><a:stretch><a:fillRect/></a:stretch>"));
        QVERIFY2(dmlRectIs(p->imgRect, 100.0 / 1.2 * 0.2, 0, 100 / 1.2, 50), qPrintable(dmlRectText(p->imgRect)));
        // The picture stretched into part of the frame.
        p = pic(QString(), QString(), QStringLiteral("<a:stretch><a:fillRect l=\"10000\" t=\"20000\" r=\"30000\" b=\"0\"/></a:stretch>"));
        QVERIFY2(dmlRectIs(p->imgRect, 10, 10, 60, 40), qPrintable(dmlRectText(p->imgRect)));
        // Crops that cut away all of the picture (or more) are not applied.
        p = pic(QString(), QString(), QStringLiteral("<a:srcRect l=\"60000\" r=\"60000\"/><a:stretch><a:fillRect/></a:stretch>"));
        QVERIFY(dmlRectIs(p->imgRect, 0, 0, 100, 50));
        p = pic(QString(), QString(), QStringLiteral("<a:srcRect l=\"99999999999\" t=\"-99999999999\"/>"));
        QVERIFY(std::isfinite(p->imgRect.x()) && std::isfinite(p->imgRect.width()) && qAbs(p->imgRect.width()) <= 1e6 && qAbs(p->imgRect.x()) <= 1e6);
        // The shape the picture is cut to.
        QCOMPARE(pic(QString(), QStringLiteral("<a:prstGeom prst=\"ellipse\"/>"))->maskShape, QStringLiteral("ellipse"));
        QCOMPARE(pic(QString(), QStringLiteral("<a:prstGeom prst=\"roundRect\"/>"))->maskShape, QStringLiteral("roundRect"));
        QCOMPARE(pic(QString(), QStringLiteral("<a:prstGeom prst=\"star5\"/>"))->maskShape, QStringLiteral("star5"));
        QCOMPARE(pic(QString(), QStringLiteral("<a:prstGeom prst=\"flowChartTerminator\"/>"))->maskShape, QStringLiteral("fcTerminator"));
        QCOMPARE(pic(QString(), QStringLiteral("<a:prstGeom prst=\"gear6\"/>"))->maskShape, QStringLiteral("rect"));
        QCOMPARE(pic(QString(), QStringLiteral("<a:custGeom><a:pathLst/></a:custGeom>"))->maskShape, QStringLiteral("rect"));
        QVERIFY(jp::shapeDef(pic(QString(), QStringLiteral("<a:prstGeom prst=\"heart\"/>"))->maskShape) != nullptr);
        // Transparency, gray, black and white, brightness, contrast.
        p = pic(QStringLiteral("<a:alphaModFix amt=\"25000\"/>"));
        QVERIFY(near(p->transparency, 0.75));
        p = pic(QStringLiteral("<a:grayscl/>"));
        QVERIFY(p->recolor == PictureItem::Grayscale);
        p = pic(QStringLiteral("<a:biLevel thresh=\"50000\"/>"));
        QVERIFY(p->recolor == PictureItem::BlackWhite);
        p = pic(QStringLiteral("<a:lum bright=\"20000\" contrast=\"-30000\"/>"));
        QVERIFY(near(p->brightness, 20) && near(p->contrast, -30));
        p = pic(QStringLiteral("<a:lum bright=\"99999999\" contrast=\"-99999999\"/>"));
        QVERIFY(p->brightness == 100 && p->contrast == -100);
        // One color made clear.
        p = pic(QStringLiteral("<a:clrChange><a:clrFrom><a:srgbClr val=\"FFFFFF\"/></a:clrFrom><a:clrTo><a:srgbClr val=\"FFFFFF\"><a:alpha val=\"0\"/></a:srgbClr></a:clrTo></a:clrChange>"));
        QVERIFY(p->hasTransparentColor);
        QCOMPARE(p->transparentColor.name(), QStringLiteral("#ffffff"));
        // Outline and turn.
        p = dmlAs<PictureItem>(dmlOne(env, dmlPic(QString(), dmlXfrm(0, 0, 10, 10, QStringLiteral("rot=\"10800000\" flipV=\"1\"")) + QStringLiteral("<a:prstGeom prst=\"rect\"/><a:ln w=\"19050\">%1</a:ln>").arg(dmlSolid(QStringLiteral("00FF00"))))));
        QVERIFY(near(p->rotation, 180) && p->flipV && !p->flipH);
        QVERIFY(near(p->stroke.width, 1.5));
        // Two pictures of one picture keep it once.
        env.images.clear();
        const ItemList both = dmlTree(env, dmlPic(QString(), frame + QStringLiteral("<a:prstGeom prst=\"rect\"/>")) + dmlPic(QString(), frame + QStringLiteral("<a:prstGeom prst=\"rect\"/>")));
        QCOMPARE(int(both.size()), 2);
        QCOMPARE(env.images.size(), 1);
        QCOMPARE(dmlAs<PictureItem>(both[0])->imageId, dmlAs<PictureItem>(both[1])->imageId);
    }

    void dmlPictureThatCannotBeReadIsAPlaceholder()
    {
        DmlEnv env;
        env.usePackage();
        const QString frame = dmlXfrm(10, 20, 100, 50) + QStringLiteral("<a:prstGeom prst=\"rect\"/>");
        for (const char *embed : {"rId9", "rId2", "", "rIdX"}) {
            auto p = dmlAs<PictureItem>(dmlOne(env, dmlPic(QString(), frame, QStringLiteral("<a:stretch/>"), QString(), QString::fromLatin1(embed))));
            QVERIFY2(p, embed);
            QVERIFY2(p->imageId.isEmpty(), embed);
            QVERIFY(dmlRectIs(p->rect, 10, 20, 100, 50));
            QVERIFY(dmlRectIs(p->imgRect, 0, 0, 100, 50));
        }
        // Bytes that are no picture the program knows, a part without bytes, and no package at all.
        DmlEnv junk;
        junk.usePackage(QByteArray("this is not a picture at all, nor any kind of image"));
        auto p = dmlAs<PictureItem>(dmlOne(junk, dmlPic(QString(), frame)));
        QVERIFY(p && p->imageId.isEmpty());
        DmlEnv empty;
        empty.usePackage(QByteArray());
        p = dmlAs<PictureItem>(dmlOne(empty, dmlPic(QString(), frame)));
        QVERIFY(p && p->imageId.isEmpty());
        DmlEnv bare;
        p = dmlAs<PictureItem>(dmlOne(bare, dmlPic(QString(), frame)));
        QVERIFY(p && p->imageId.isEmpty());
        // A picture with no blip at all.
        p = dmlAs<PictureItem>(dmlOne(env, QStringLiteral("<p:pic><p:nvPicPr><p:cNvPr id=\"5\" name=\"P\"/><p:cNvPicPr/><p:nvPr/></p:nvPicPr><p:blipFill/><p:spPr>%1</p:spPr></p:pic>").arg(frame)));
        QVERIFY(p && p->imageId.isEmpty());
        p = dmlAs<PictureItem>(dmlOne(env, QStringLiteral("<p:pic/>")));
        QVERIFY(p && p->imageId.isEmpty());
        // Without a way to keep pictures, none is kept.
        DmlEnv noKeep;
        noKeep.usePackage();
        noKeep.ctx.addImage = nullptr;
        p = dmlAs<PictureItem>(dmlOne(noKeep, dmlPic(QString(), frame)));
        QVERIFY(p && p->imageId.isEmpty());
    }

    void dmlConnectorsBecomeLines()
    {
        DmlEnv env;
        env.useFormats();
        const QString style = dmlStyle(QStringLiteral("1"), QStringLiteral("0"), QStringLiteral("0"), QStringLiteral("accent1"));
        // The frame's corners, upside down.
        ItemPtr it = dmlOne(env, dmlCxn(dmlXfrm(10, 20, 100, 50, QStringLiteral("flipV=\"1\"")) + QStringLiteral("<a:prstGeom prst=\"straightConnector1\"/>"), QString(), style));
        auto l = dmlAs<LineItem>(it);
        QVERIFY(l);
        QVERIFY(near(l->p1.x(), 10) && near(l->p1.y(), 70) && near(l->p2.x(), 110) && near(l->p2.y(), 20));
        QVERIFY(l->route == LineItem::Straight);
        QCOMPARE(l->name, QStringLiteral("Connector 9"));
        QVERIFY(l->wrap.mode == jp::Wrap::None);
        QCOMPARE(dmlHex(l->stroke.color), QStringLiteral("4472C4"));
        // Turned a quarter about its center.
        it = dmlOne(env, dmlCxn(dmlXfrm(10, 20, 100, 50, QStringLiteral("rot=\"5400000\"")) + QStringLiteral("<a:prstGeom prst=\"line\"/>"), QString(), style));
        l = dmlAs<LineItem>(it);
        QVERIFY2(near(l->p1.x(), 85) && near(l->p1.y(), -5) && near(l->p2.x(), 35) && near(l->p2.y(), 95), qPrintable(QStringLiteral("%1,%2 %3,%4").arg(l->p1.x()).arg(l->p1.y()).arg(l->p2.x()).arg(l->p2.y())));
        // Arrowheads and weight, from the line properties.
        it = dmlOne(env, dmlCxn(dmlXfrm(0, 0, 100, 0) + QStringLiteral("<a:prstGeom prst=\"line\"/><a:ln w=\"38100\">%1<a:headEnd type=\"oval\"/><a:tailEnd type=\"triangle\" w=\"lg\" len=\"lg\"/></a:ln>")
                                   .arg(dmlSolid(QStringLiteral("FF0000")))));
        l = dmlAs<LineItem>(it);
        QVERIFY(near(l->stroke.width, 3));
        QVERIFY(l->stroke.startArrow == Arrow::Oval && l->stroke.endArrow == Arrow::Triangle);
        QVERIFY(near(l->p1.x(), 0) && near(l->p2.x(), 100) && near(l->p1.y(), 0) && near(l->p2.y(), 0));
        // A line drawn as a shape is a line.
        it = dmlOne(env, dmlSp(dmlXfrm(10, 10, 80, 20) + QStringLiteral("<a:prstGeom prst=\"line\"/><a:ln w=\"12700\">%1</a:ln>").arg(dmlSolid(QStringLiteral("000000")))));
        l = dmlAs<LineItem>(it);
        QVERIFY(l && near(l->p2.x(), 90) && near(l->p2.y(), 30));
        // Elbows and curves keep their bend and the way they leave.
        const auto route = [&](const char *prst, const QString &attrs, const QString &adj = QString()) {
            return dmlAs<LineItem>(dmlOne(env, dmlCxn(dmlXfrm(10, 20, 100, 50, attrs) + QStringLiteral("<a:prstGeom prst=\"%1\"><a:avLst>%2</a:avLst></a:prstGeom>").arg(QString::fromLatin1(prst), adj), QString(), style)));
        };
        l = route("bentConnector3", QString(), QStringLiteral("<a:gd name=\"adj1\" fmla=\"val 25000\"/>"));
        QVERIFY(l && l->route == LineItem::Elbow);
        QVERIFY(near(l->bend, 0.25));
        QVERIFY(!l->startVertical && !l->endVertical);
        l = route("bentConnector3", QStringLiteral("rot=\"5400000\""));
        QVERIFY(near(l->bend, 0.5));
        QVERIFY(l->startVertical && l->endVertical);
        l = route("bentConnector3", QStringLiteral("rot=\"10800000\""));
        QVERIFY(!l->startVertical && !l->endVertical);
        l = route("bentConnector3", QStringLiteral("rot=\"16200000\" flipH=\"1\""));
        QVERIFY(l->startVertical && l->endVertical);
        l = route("bentConnector3", QString(), QStringLiteral("<a:gd name=\"adj1\" fmla=\"val -300000\"/>"));
        QVERIFY(l->bend >= -10 && l->bend <= 11);
        l = route("bentConnector2", QString());
        QVERIFY(l->route == LineItem::Elbow && !l->startVertical && l->endVertical);
        l = route("bentConnector2", QStringLiteral("rot=\"5400000\""));
        QVERIFY(l->startVertical && !l->endVertical);
        l = route("curvedConnector3", QString());
        QVERIFY(l->route == LineItem::Curved && !l->startVertical && !l->endVertical);
        l = route("curvedConnector2", QString());
        QVERIFY(l->route == LineItem::Curved && !l->startVertical && l->endVertical);
        // With more bends than a line has, a path.
        it = dmlOne(env, dmlCxn(dmlXfrm(10, 20, 100, 50) + QStringLiteral("<a:prstGeom prst=\"bentConnector4\"/>"), QString(), style));
        auto s = dmlAs<ShapeItem>(it);
        QVERIFY(s);
        QVERIFY(s->fill.isNone());
        QVERIFY(!s->stroke.isNone());
        QVERIFY(s->customPath.elementCount() == 5);
        QVERIFY2(dmlRectIs(s->rect, 10, 20, 100, 50), qPrintable(dmlRectText(s->rect)));
        it = dmlOne(env, dmlCxn(dmlXfrm(10, 20, 100, 50) + QStringLiteral("<a:prstGeom prst=\"curvedConnector5\"/>"), QString(), style));
        QVERIFY(dmlAs<ShapeItem>(it) && !dmlAs<ShapeItem>(it)->customPath.isEmpty());
    }

    void dmlConnectorsKeepWhatTheyAreJoinedTo()
    {
        DmlEnv env;
        const QString rect = QStringLiteral("<a:prstGeom prst=\"rect\"/>");
        const QString shapes = dmlSp(dmlXfrm(10, 10, 50, 50) + rect, QString(), QString(), QString(), QString(), QString(), QStringLiteral("3")) +
                               dmlSp(dmlXfrm(200, 10, 50, 50) + QStringLiteral("<a:prstGeom prst=\"ellipse\"/>"), QString(), QString(), QString(), QString(), QString(), QStringLiteral("4"));
        const auto joined = [&](const QString &ends) {
            const ItemList list = dmlTree(env, shapes + dmlCxn(dmlXfrm(60, 35, 140, 0) + QStringLiteral("<a:prstGeom prst=\"straightConnector1\"/>"), ends));
            return QPair<ItemList, std::shared_ptr<LineItem>>(list, dmlAs<LineItem>(list.back()));
        };
        auto result = joined(QStringLiteral("<a:stCxn id=\"3\" idx=\"3\"/><a:endCxn id=\"4\" idx=\"2\"/>"));
        QCOMPARE(int(result.first.size()), 3);
        auto l = result.second;
        QVERIFY(l);
        QCOMPARE(l->start.id, result.first[0]->id);
        QCOMPARE(l->start.site, 3);          // the right side of the rectangle
        QCOMPARE(l->end.id, result.first[1]->id);
        QCOMPARE(l->end.site, 1);            // the ellipse's third point is its left
        result = joined(QStringLiteral("<a:stCxn id=\"3\" idx=\"0\"/><a:endCxn id=\"4\" idx=\"6\"/>"));
        QCOMPARE(result.second->start.site, 0);
        QCOMPARE(result.second->end.site, 3);
        result = joined(QStringLiteral("<a:stCxn id=\"3\" idx=\"2\"/>"));
        QCOMPARE(result.second->start.site, 2);
        QVERIFY(result.second->end.id.isEmpty());
        // A point that the library's shape has no place for, a shape that isn't there, and a place that is not one: not joined.
        result = joined(QStringLiteral("<a:stCxn id=\"4\" idx=\"3\"/><a:endCxn id=\"77\" idx=\"0\"/>"));
        QVERIFY(result.second->start.id.isEmpty() && result.second->end.id.isEmpty());
        result = joined(QStringLiteral("<a:stCxn id=\"3\" idx=\"9\"/><a:endCxn id=\"3\" idx=\"-1\"/>"));
        QVERIFY(result.second->start.id.isEmpty() && result.second->end.id.isEmpty());
        result = joined(QStringLiteral("<a:stCxn id=\"x\" idx=\"y\"/>"));
        QVERIFY(result.second->start.id.isEmpty());
        // Joined to something inside a group, and to something that comes after the line.
        const QString grouped = dmlGrp(dmlGroupFrame(QRectF(0, 0, 100, 100), QRectF(0, 0, 100, 100)), dmlSp(dmlXfrm(10, 10, 50, 50) + rect, QString(), QString(), QString(), QString(), QString(), QStringLiteral("30")));
        const ItemList list = dmlTree(env, dmlCxn(dmlXfrm(60, 35, 140, 0) + QStringLiteral("<a:prstGeom prst=\"straightConnector1\"/>"), QStringLiteral("<a:stCxn id=\"30\" idx=\"3\"/>")) + grouped);
        QCOMPARE(dmlAs<LineItem>(list[0])->start.id, dmlAs<GroupItem>(list[1])->children[0]->id);
    }

    void dmlHyperlinksAreTheirExternalTargets()
    {
        DmlEnv env;
        env.usePackage();
        const QString link = QStringLiteral("<a:hlinkClick r:id=\"rId2\"/>");
        const QString frame = dmlXfrm(0, 0, 10, 10) + QStringLiteral("<a:prstGeom prst=\"rect\"/>");
        QCOMPARE(dmlOne(env, dmlSp(frame, QString(), QString(), QString(), QString(), link))->hyperlink, QStringLiteral("https://example.com/x"));
        QCOMPARE(dmlOne(env, dmlSp(frame, dmlBody(), QString(), QStringLiteral("txBox=\"1\""), QString(), link))->hyperlink, QStringLiteral("https://example.com/x"));
        QCOMPARE(dmlOne(env, dmlPic(QString(), frame, QStringLiteral("<a:stretch/>"), link))->hyperlink, QStringLiteral("https://example.com/x"));
        const QString connector = QStringLiteral("<p:cxnSp><p:nvCxnSpPr><p:cNvPr id=\"9\" name=\"c\">%1</p:cNvPr><p:cNvCxnSpPr/><p:nvPr/></p:nvCxnSpPr><p:spPr>%2<a:prstGeom prst=\"line\"/></p:spPr></p:cxnSp>")
                                    .arg(link, dmlXfrm(0, 0, 10, 10));
        QCOMPARE(dmlOne(env, connector)->hyperlink, QStringLiteral("https://example.com/x"));
        // Nothing to go to: a link to a part inside the file, one that isn't there, and one with no target.
        QVERIFY(dmlOne(env, dmlSp(frame, QString(), QString(), QString(), QString(), QStringLiteral("<a:hlinkClick r:id=\"rId1\"/>")))->hyperlink.isEmpty());
        QVERIFY(dmlOne(env, dmlSp(frame, QString(), QString(), QString(), QString(), QStringLiteral("<a:hlinkClick r:id=\"rId9\"/>")))->hyperlink.isEmpty());
        QVERIFY(dmlOne(env, dmlSp(frame, QString(), QString(), QString(), QString(), QStringLiteral("<a:hlinkClick/>")))->hyperlink.isEmpty());
        QVERIFY(dmlOne(env, dmlSp(frame))->hyperlink.isEmpty());
        DmlEnv bare;
        QVERIFY(dmlOne(bare, dmlSp(frame, QString(), QString(), QString(), QString(), link))->hyperlink.isEmpty());
    }

    void dmlTreeSkipsWhatIsHiddenAndTakesTheFallback()
    {
        DmlEnv env;
        const QString frame = dmlXfrm(0, 0, 10, 10) + QStringLiteral("<a:prstGeom prst=\"rect\"/>");
        // A hidden shape isn't drawn.
        QString hidden = dmlSp(frame);
        hidden.replace(QStringLiteral("name=\"Shape 2\""), QStringLiteral("name=\"Shape 2\" hidden=\"1\""));
        QVERIFY(dmlTree(env, hidden).empty());
        QVERIFY(!dmlOne(env, hidden));
        QCOMPARE(int(dmlTree(env, hidden + dmlSp(frame)).size()), 1);
        // What a newer program wrote with a fallback for older ones: the fallback.
        const QString alt = QStringLiteral("<mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\"><mc:Choice Requires=\"a14\">%1</mc:Choice>"
                                           "<mc:Fallback>%2</mc:Fallback></mc:AlternateContent>");
        ItemList list = dmlTree(env, dmlSp(frame) + alt.arg(dmlSp(frame, QString(), QString(), QString(), QString(), QString(), QStringLiteral("7")), dmlSp(frame, QString(), QString(), QString(), QString(), QString(), QStringLiteral("8")) +
                                                                dmlSp(frame, QString(), QString(), QString(), QString(), QString(), QStringLiteral("9"))));
        QCOMPARE(int(list.size()), 3);
        QCOMPARE(list[1]->name, QStringLiteral("Shape 8"));
        QCOMPARE(list[2]->name, QStringLiteral("Shape 9"));
        // What isn't read yet (tables, charts) and the tree's own properties give nothing.
        list = dmlTree(env, QStringLiteral("<p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr/><p:graphicFrame/><p:contentPart/><p:unknown/>") + dmlSp(frame));
        QCOMPARE(int(list.size()), 1);
        QVERIFY(!dmlOne(env, QStringLiteral("<p:graphicFrame/>")));
        QVERIFY(!dmlOne(env, QStringLiteral("<p:unknown/>")));
        QVERIFY(!readShape(QDomElement(), env.ctx));
        QVERIFY(readShapeTree(QDomElement(), env.ctx).empty());
        // Shapes of a document's drawing are read the same, in their own namespace.
        QDomDocument doc;
        const QByteArray wps = QStringLiteral("<drawing xmlns:a=\"%1\" xmlns:wps=\"urn:test:wordprocessing-shape\"><wps:wsp><wps:cNvSpPr/>"
                                              "<wps:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"254000\" cy=\"127000\"/></a:xfrm><a:prstGeom prst=\"ellipse\"/></wps:spPr></wps:wsp></drawing>")
                                   .arg(QString(ns::A)).toUtf8();
        QVERIFY(parseXml(wps, &doc));
        const ItemList docShapes = readShapeTree(doc.documentElement(), env.ctx);
        QCOMPARE(int(docShapes.size()), 1);
        QCOMPARE(dmlAs<ShapeItem>(docShapes[0])->shape, QStringLiteral("ellipse"));
        QVERIFY(dmlRectIs(docShapes[0]->rect, 0, 0, 20, 10));
    }

    void dmlHostileShapesAreHeldToWhatAPageCanShow()
    {
        DmlEnv env;
        const auto check = [&](const ItemList &list) {
            jp::walkItems(list, [&](const ItemPtr &it) {
                QVERIFY(std::isfinite(it->rect.x()) && std::isfinite(it->rect.y()) && std::isfinite(it->rect.width()) && std::isfinite(it->rect.height()));
                QVERIFY(qAbs(it->rect.x()) <= 1e6 && qAbs(it->rect.y()) <= 1e6 && it->rect.width() >= 0 && it->rect.height() >= 0);
                if (it->type() != ItemType::Line && it->type() != ItemType::Group) QVERIFY(it->rect.width() <= 1e6 && it->rect.height() <= 1e6);
                QVERIFY(std::isfinite(it->rotation) && it->rotation >= 0 && it->rotation < 360);
                if (it->type() == ItemType::Line) {
                    auto *l = static_cast<LineItem *>(it.get());
                    QVERIFY(std::isfinite(l->p1.x()) && std::isfinite(l->p2.y()) && qAbs(l->p1.x()) <= 1e6 && qAbs(l->p2.y()) <= 1e6);
                }
            });
        };
        for (const char *huge : {"99999999999999999999", "-99999999999999999999", "1e300", "nan", "inf", "-5", "0", "9223372036854775807", "-9223372036854775808"}) {
            const QString v = QString::fromLatin1(huge);
            const QString xfrm = QStringLiteral("<a:xfrm rot=\"%1\"><a:off x=\"%1\" y=\"%1\"/><a:ext cx=\"%1\" cy=\"%1\"/></a:xfrm>").arg(v);
            const QString geometries[] = {QStringLiteral("<a:prstGeom prst=\"rect\"/>"), QStringLiteral("<a:prstGeom prst=\"line\"/>"), QStringLiteral("<a:prstGeom prst=\"pie\"/>"),
                                          QStringLiteral("<a:prstGeom prst=\"roundRect\"><a:avLst><a:gd name=\"adj\" fmla=\"val %1\"/></a:avLst></a:prstGeom>").arg(v)};
            const QString groupFrame = QStringLiteral("<a:xfrm><a:off x=\"%1\" y=\"%1\"/><a:ext cx=\"%1\" cy=\"%1\"/><a:chOff x=\"%1\" y=\"%1\"/><a:chExt cx=\"%1\" cy=\"%1\"/></a:xfrm>").arg(v);
            for (const QString &geometry : geometries) {
                const QString all = dmlSp(xfrm + geometry) + dmlCxn(xfrm + geometry) + dmlPic(QString(), xfrm + geometry);
                check(dmlTree(env, dmlSp(xfrm + geometry, dmlBody(), QString(), QStringLiteral("txBox=\"1\"")) + all + dmlGrp(groupFrame, all)));
            }
        }
        // A group scaled to a speck of its child space or from one: positions stay where they can.
        check(dmlTree(env, dmlGrp(dmlGroupFrame(QRectF(0, 0, 1e5, 1e5), QRectF(0, 0, 1e-4, 1e-4)), dmlSp(dmlXfrm(1, 1, 5, 5) + QStringLiteral("<a:prstGeom prst=\"rect\"/>")))));
        check(dmlTree(env, dmlGrp(dmlGroupFrame(QRectF(0, 0, 1e-4, 1e-4), QRectF(0, 0, 1e5, 1e5)), dmlSp(dmlXfrm(1, 1, 5, 5) + QStringLiteral("<a:prstGeom prst=\"rect\"/>")))));
    }

    void dmlGroupsAreReadToALimitedDepth()
    {
        DmlEnv env;
        const QString leaf = dmlSp(dmlXfrm(0, 0, 10, 10) + QStringLiteral("<a:prstGeom prst=\"rect\"/>"));
        const QString frame = dmlGroupFrame(QRectF(0, 0, 100, 100), QRectF(0, 0, 100, 100));
        const auto nested = [&](int levels) {
            QString xml = leaf;
            for (int i = 0; i < levels; ++i) xml = dmlGrp(frame, xml, QString::number(100 + i));
            return xml;
        };
        const auto depth = [](const ItemList &list) {
            int deepest = 0;
            std::function<void(const ItemList &, int)> walk = [&](const ItemList &l, int d) {
                for (const ItemPtr &it : l)
                    if (it->type() == ItemType::Group) walk(static_cast<GroupItem *>(it.get())->children, d + 1);
                    else deepest = qMax(deepest, d);
            };
            walk(list, 0);
            return deepest;
        };
        QCOMPARE(depth(dmlTree(env, nested(10))), 10);
        QCOMPARE(depth(dmlTree(env, nested(64))), 64);
        QVERIFY(depth(dmlTree(env, nested(65))) <= 64);
        QVERIFY(depth(dmlTree(env, nested(200))) <= 64);
        // A single group is read to the same limit.
        ItemList single;
        if (const ItemPtr one = dmlOne(env, nested(100))) single.push_back(one);
        QVERIFY(depth(single) <= 64);
    }

    void dmlTreesAreReadToALimitedSize()
    {
        DmlEnv env;
        const QString leaf = dmlSp(dmlXfrm(0, 0, 10, 10) + QStringLiteral("<a:prstGeom prst=\"rect\"/>"));
        QString many;
        for (int i = 0; i < 10001; ++i) many += leaf;
        int count = 0;
        jp::walkItems(dmlTree(env, many), [&](const ItemPtr &) { ++count; });
        QCOMPARE(count, 10000);
        // Counting shapes inside groups, and the groups themselves.
        QString groups;
        const QString frame = dmlGroupFrame(QRectF(0, 0, 100, 100), QRectF(0, 0, 100, 100));
        for (int g = 0; g < 200; ++g) {
            QString inside;
            for (int i = 0; i < 100; ++i) inside += leaf;
            groups += dmlGrp(frame, inside, QString::number(1000 + g));
        }
        count = 0;
        jp::walkItems(dmlTree(env, groups), [&](const ItemPtr &) { ++count; });
        QVERIFY2(count <= 10000, qPrintable(QString::number(count)));
        QVERIFY(count > 9000);
        // A million small shapes asked for through a tree of repeated pictures costs no more than the limit.
        env.usePackage();
        env.images.clear();
        QString pics;
        for (int i = 0; i < 3000; ++i) pics += dmlPic(QString(), dmlXfrm(0, 0, 10, 10) + QStringLiteral("<a:prstGeom prst=\"rect\"/>"));
        QCOMPARE(int(dmlTree(env, pics).size()), 3000);
        QCOMPARE(env.images.size(), 1);
    }

    void dmlShapesSurviveRandomDamage()
    {
        DmlEnv env;
        env.useFormats();
        env.usePackage();
        const QString tree = QStringLiteral("<p:spTree %1>").arg(dmlNamespaces()) +
            dmlSp(dmlXfrm(10, 10, 100, 50, QStringLiteral("rot=\"5400000\" flipH=\"1\"")) + QStringLiteral("<a:prstGeom prst=\"roundRect\"><a:avLst><a:gd name=\"adj\" fmla=\"val 30000\"/></a:avLst></a:prstGeom>") +
                  QStringLiteral("<a:gradFill><a:gsLst><a:gs pos=\"0\"><a:srgbClr val=\"FF0000\"/></a:gs><a:gs pos=\"100000\"><a:schemeClr val=\"accent1\"/></a:gs></a:gsLst><a:lin ang=\"2700000\" scaled=\"1\"/></a:gradFill>"
                                 "<a:ln w=\"12700\">%1<a:prstDash val=\"dash\"/><a:tailEnd type=\"triangle\"/></a:ln><a:effectLst><a:outerShdw blurRad=\"50800\" dist=\"38100\" dir=\"2700000\"><a:srgbClr val=\"000000\"/></a:outerShdw></a:effectLst>")
                      .arg(dmlSolid(QStringLiteral("00FF00"))),
                  dmlBody(QStringLiteral("Text"), QStringLiteral("<a:bodyPr lIns=\"0\" anchor=\"ctr\" numCol=\"2\"><a:normAutofit fontScale=\"50000\"/></a:bodyPr>")),
                  dmlStyle(QStringLiteral("2"), QStringLiteral("2"), QStringLiteral("3")), QStringLiteral("txBox=\"1\""), QString(), QStringLiteral("<a:hlinkClick r:id=\"rId2\"/>")) +
            dmlGrp(dmlGroupFrame(QRectF(100, 100, 200, 100), QRectF(0, 0, 100, 50), QStringLiteral("rot=\"600000\" flipV=\"1\"")) + QStringLiteral("<a:grpFill/>"),
                   dmlSp(dmlXfrm(1, 2, 3, 4) + QStringLiteral("<a:custGeom><a:avLst><a:gd name=\"a\" fmla=\"val 5\"/></a:avLst><a:gdLst><a:gd name=\"x\" fmla=\"*/ w a 100\"/></a:gdLst><a:pathLst>"
                                                              "<a:path w=\"10\" h=\"10\"><a:moveTo><a:pt x=\"0\" y=\"0\"/></a:moveTo><a:arcTo wR=\"x\" hR=\"hd2\" stAng=\"0\" swAng=\"5400000\"/>"
                                                              "<a:cubicBezTo><a:pt x=\"1\" y=\"2\"/><a:pt x=\"3\" y=\"4\"/><a:pt x=\"5\" y=\"6\"/></a:cubicBezTo><a:close/></a:path></a:pathLst></a:custGeom><a:grpFill/>")) +
                   dmlCxn(dmlXfrm(1, 2, 30, 40) + QStringLiteral("<a:prstGeom prst=\"bentConnector3\"><a:avLst><a:gd name=\"adj1\" fmla=\"val 20000\"/></a:avLst></a:prstGeom>"), QStringLiteral("<a:stCxn id=\"2\" idx=\"1\"/>")) +
                   dmlSp(dmlXfrm(1, 2, 3, 4) + QStringLiteral("<a:prstGeom prst=\"wedgeRectCallout\"/>"))) +
            dmlPic(QStringLiteral("<a:alphaModFix amt=\"50000\"/><a:lum bright=\"10000\"/>"), dmlXfrm(5, 5, 60, 40) + QStringLiteral("<a:prstGeom prst=\"ellipse\"/>"),
                   QStringLiteral("<a:srcRect l=\"10000\" t=\"5000\"/><a:stretch><a:fillRect/></a:stretch>")) +
            QStringLiteral("</p:spTree>");
        QDomDocument pristine;
        QVERIFY(parseXml(tree.toUtf8(), &pristine));
        QVERIFY(!readShapeTree(pristine.documentElement(), env.ctx).empty());
        QRandomGenerator gen(2024);
        const QByteArray source = tree.toUtf8();
        const QList<QByteArray> numbers = {"0", "1", "-1", "5400000", "21600000", "99999999999999999999", "-99999999999999999999", "100000", "12700", "7", "000"};
        const QRegularExpression digits(QStringLiteral("-?[0-9]+"));
        QList<QPair<int, int>> runs;   // where each number is written
        for (auto m = digits.globalMatch(QString::fromUtf8(source)); m.hasNext();) {
            const auto match = m.next();
            runs.append({int(match.capturedStart()), int(match.capturedLength())});
        }
        int read = 0;
        for (int i = 0; i < 900; ++i) {
            QByteArray bad = source;
            if (i % 3 == 0) {
                // Bytes of the file at random.
                for (int k = 0, n = 1 + int(gen.bounded(4)); k < n; ++k) bad[int(gen.bounded(quint32(bad.size())))] = char(gen.bounded(256u));
            } else if (i % 3 == 1) {
                // Letters and digits for others, which leaves most of it XML.
                for (int k = 0, n = 1 + int(gen.bounded(4)); k < n; ++k) {
                    const int at = int(gen.bounded(quint32(bad.size())));
                    if (QChar(bad[at]).isLetterOrNumber()) bad[at] = "0123456789abcxyzAZ-"[gen.bounded(19u)];
                }
            } else {
                // A number written another way, as many as three of them.
                for (int k = 0, n = 1 + int(gen.bounded(3)); k < n; ++k) {
                    const auto run = runs[int(gen.bounded(quint32(runs.size())))];
                    bad.replace(run.first, run.second, QByteArray(run.second, ' '));   // the length stays, so the others' places hold
                    bad.replace(run.first, 1, numbers[int(gen.bounded(quint32(numbers.size())))].left(run.second));
                }
            }
            QDomDocument doc;
            if (!parseXml(bad, &doc)) continue;
            ++read;
            const ItemList list = readShapeTree(doc.documentElement(), env.ctx);
            jp::walkItems(list, [&](const ItemPtr &it) {
                QVERIFY(std::isfinite(it->rect.x()) && std::isfinite(it->rect.width()) && std::isfinite(it->rotation));
                QVERIFY(qAbs(it->rect.x()) <= 1e6 && it->rect.width() >= 0);
            });
            // The same damage in the pieces' own forms.
            for (QDomElement e = doc.documentElement().firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) readShape(e, env.ctx, {}, {});
            env.images.clear();
            env.stories.clear();
        }
        QVERIFY2(read > 400, qPrintable(QString::number(read)));   // enough of the damage left XML to read
    }

    void dmlReadShapesDrawWhereTheyAre()
    {
        DmlEnv env;
        const auto render = [](const ItemPtr &it, const QSize &size) {
            QImage img(size, QImage::Format_ARGB32);
            img.fill(Qt::white);
            QPainter p(&img);
            auto s = dmlAs<ShapeItem>(it);
            const QPainterPath path = s->customPath.isEmpty() ? jp::shapePath(s->shape, s->rect.size(), s->adj) : s->customPath;
            p.fillPath(path, s->fill.brush(path.boundingRect(), jp::ColorScheme()));
            p.end();
            return img;
        };
        // A rounded rectangle is ink in the middle and at the edge of a side, but not in the corner.
        ItemPtr it = dmlOne(env, dmlSp(dmlXfrm(10, 10, 100, 60) + QStringLiteral("<a:prstGeom prst=\"roundRect\"><a:avLst><a:gd name=\"adj\" fmla=\"val 50000\"/></a:avLst></a:prstGeom>") + dmlSolid(QStringLiteral("FF0000"))));
        QImage img = render(it, QSize(100, 60));
        QCOMPARE(img.pixelColor(50, 30), QColor(255, 0, 0));
        QCOMPARE(img.pixelColor(50, 2), QColor(255, 0, 0));
        QCOMPARE(img.pixelColor(2, 30), QColor(255, 0, 0));
        QCOMPARE(img.pixelColor(1, 1), QColor(255, 255, 255));
        QCOMPARE(img.pixelColor(98, 58), QColor(255, 255, 255));
        // A pie is ink in three quarters.
        it = dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 100) + QStringLiteral("<a:prstGeom prst=\"pie\"/>") + dmlSolid(QStringLiteral("0000FF"))));
        img = render(it, QSize(100, 100));
        QCOMPARE(img.pixelColor(25, 25), QColor(0, 0, 255));
        QCOMPARE(img.pixelColor(75, 75), QColor(0, 0, 255));
        QCOMPARE(img.pixelColor(75, 25), QColor(255, 255, 255));
        // A shape of its own path, drawn only inside the path.
        it = dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 100) + QStringLiteral("<a:custGeom><a:pathLst><a:path w=\"100\" h=\"100\"><a:moveTo><a:pt x=\"0\" y=\"100\"/></a:moveTo>"
                                                                      "<a:lnTo><a:pt x=\"50\" y=\"0\"/></a:lnTo><a:lnTo><a:pt x=\"100\" y=\"100\"/></a:lnTo><a:close/></a:path></a:pathLst></a:custGeom>") + dmlSolid(QStringLiteral("00FF00"))));
        img = render(it, QSize(100, 100));
        QCOMPARE(img.pixelColor(50, 70), QColor(0, 255, 0));
        QCOMPARE(img.pixelColor(5, 20), QColor(255, 255, 255));
        // A star from the library, with the standard's own inner size.
        it = dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 100) + QStringLiteral("<a:prstGeom prst=\"star5\"/>") + dmlSolid(QStringLiteral("FF00FF"))));
        img = render(it, QSize(100, 100));
        QCOMPARE(img.pixelColor(50, 50), QColor(255, 0, 255));
        QCOMPARE(img.pixelColor(2, 2), QColor(255, 255, 255));
    }

    void dmlPresetGeometryUsesTheLibraryWhereItIsAsGood()
    {
        for (const char *name : {"rect", "roundRect", "ellipse", "triangle", "rightArrow", "star5", "hexagon", "flowChartProcess"})
            QVERIFY2(!prefersPresetGeometry(QString::fromLatin1(name)), name);
        for (const char *name : {"pie", "chord", "arc", "blockArc", "wedgeRectCallout", "wedgeRoundRectCallout", "wedgeEllipseCallout", "borderCallout1",
                                 "rightArrowCallout", "downArrowCallout"})
            QVERIFY2(prefersPresetGeometry(QString::fromLatin1(name)) && hasPresetGeometry(QString::fromLatin1(name)), name);
        QVERIFY(!hasPresetGeometry(QStringLiteral("rect")));
        QVERIFY(presetGeometry(QStringLiteral("rect"), {}, QSizeF(10, 10)).isEmpty());
        QVERIFY(presetGeometry(QStringLiteral("pie"), {}, QSizeF(0, 0)).isEmpty());
        QVERIFY(presetGeometry(QStringLiteral("pie"), {}, QSizeF(-5, 10)).isEmpty());
    }

    void dmlEveryPresetDefinitionSurvivesAnySizeAndAdjustment()
    {
        const QList<double> values = {-1e18, -100000, 0, 1, 50000, 100000, 21600000, 1e18};
        const QList<QSizeF> sizes = {QSizeF(100, 100), QSizeF(0, 50), QSizeF(50, 0), QSizeF(1e6, 1), QSizeF(1, 1e6), QSizeF(0.001, 0.001)};
        for (const QString &prst : dmlAllPresets()) {
            if (!hasPresetGeometry(prst)) continue;
            for (double v : values) {
                QHash<QString, double> adj;
                for (const char *name : {"adj", "adj1", "adj2", "adj3", "adj4", "adj5", "adj6", "adj7", "adj8"}) adj.insert(QString::fromLatin1(name), v);
                for (const QSizeF &size : sizes) {
                    const Geometry g = presetGeometry(prst, adj, size);
                    for (const GeometryPath &gp : g.paths)
                        for (int i = 0; i < gp.path.elementCount(); ++i) {
                            const auto e = gp.path.elementAt(i);
                            QVERIFY2(std::isfinite(e.x) && std::isfinite(e.y) && qAbs(e.x) <= 1e6 + 1 && qAbs(e.y) <= 1e6 + 1, qPrintable(prst));
                        }
                }
            }
        }
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
