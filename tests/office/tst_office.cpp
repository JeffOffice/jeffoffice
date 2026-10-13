// Tests of libs/office: packages, the theme, and DrawingML colors. Every
// package they read is built in memory (never a real-world file).
#include "office/chart.h"
#include "office/drawingml.h"
#include "office/drawingml_color.h"
#include "office/drawingml_geometry.h"
#include "office/drawingml_style.h"
#include "office/drawingml_table.h"
#include "office/drawingml_write.h"
#include "office/odraw.h"
#include "office/package.h"
#include "office/theme.h"
#include "office/xml.h"
#include "io/zip.h"
#include "render/metafile.h"
#include "render/shapes.h"
#include "text/textengine.h"
#include "text/textprops.h"

#include <QBuffer>
#include <QElapsedTimer>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QTextDocument>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QStringConverter>
#include <QSvgRenderer>
#include <QTemporaryDir>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QtEndian>
#include <QtTest>

#include <cmath>
#include <limits>
#include <memory>
#include <vector>

using namespace jp::office;
using jp::Arrow;
using jp::ColorRef;
using jp::Fill;
using jp::GradientStop;
using jp::GroupItem;
using jp::ItemList;
using jp::ItemPtr;
using jp::ItemType;
using jp::LineItem;
using jp::PictureItem;
using jp::ShapeItem;
using jp::Stroke;
using jp::TableItem;
using jp::TextArtItem;
using jp::TextItem;

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
        QElapsedTimer t;
        t.start();
        for (int i = 0; i < ids.size(); ++i)
            QCOMPARE(pkg.relationship(QStringLiteral("/ppt/slides/slide1.xml"), ids[i]).target, QStringLiteral("/ppt/media/image%1.png").arg(i));
        QVERIFY2(t.elapsed() < 2000, qPrintable(QString::number(t.elapsed())));
        QVERIFY(pkg.relationship(QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("rIdNone")).id.isEmpty());
        QVERIFY(pkg.relationship(QStringLiteral("/nowhere.xml"), ids[0]).id.isEmpty());
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
        QElapsedTimer timer;
        timer.start();
        QVERIFY(!parseXml(xml, &doc, &err));
        QVERIFY(!err.isEmpty());
        QVERIFY(doc.documentElement().isNull());
        // Counted on the way in, not found out by building the tree.
        QVERIFY2(timer.elapsed() < 10000, "the XML was refused too slowly");
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
            QElapsedTimer timer;
            timer.start();
            QVERIFY2(!parseXml(xml, &doc, &err), qPrintable(QString::number(count)));
            QVERIFY(!err.isEmpty());
            QVERIFY2(timer.elapsed() < 5000, "the XML was refused too slowly");
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
        QElapsedTimer timer;
        timer.start();
        QVERIFY2(parseXml(xml, &doc, &err), qPrintable(err));
        QCOMPARE(doc.documentElement().childNodes().size(), 61000);
        QVERIFY2(timer.elapsed() < 5000, "the XML took too long to parse");

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
            QElapsedTimer timer;
            timer.start();
            QVERIFY(!parseXml(bytes, &doc, &err));
            QVERIFY(!err.isEmpty());
            QVERIFY2(timer.elapsed() < 5000, "the XML was refused too slowly");

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
        QElapsedTimer timer;
        timer.start();
        QVERIFY2(parseXml(QByteArray("<a>").repeated(kMaxXmlDepth) + QByteArray("</a>").repeated(kMaxXmlDepth), &doc, &err), qPrintable(err));
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

    // ---- charts

    void chartReadsClusteredColumns()
    {
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/><c:varyColors val=\"0\"/>")
                             + chartSeries(0, QStringLiteral("Q1"), {"East", "West", "North"}, {10, 20, 40}, QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></c:spPr>"))
                             + chartSeries(1, QStringLiteral("Q2"), {"East", "West", "North"}, {5, 15, 25})
                             + QStringLiteral("<c:gapWidth val=\"80\"/><c:overlap val=\"-10\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        Chart c;
        QString err;
        const Theme theme = defaultTheme();
        QVERIFY2(readChart(chartPart(plot), theme, ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(c.plots.size()), 1);
        const ChartPlot &p = c.plots[0];
        QCOMPARE(int(p.type), int(ChartType::Bar));
        QCOMPARE(int(c.type()), int(ChartType::Bar));
        QVERIFY(!p.horizontal);
        QCOMPARE(int(p.grouping), int(ChartGrouping::Clustered));
        QCOMPARE(p.gapWidth, 80);
        QCOMPARE(p.overlap, -10);
        QVERIFY(!p.varyColors);
        QCOMPARE(int(p.series.size()), 2);
        QCOMPARE(p.series[0].name, QStringLiteral("Q1"));
        QCOMPARE(p.series[0].categories, QStringList({"East", "West", "North"}));
        QCOMPARE(p.series[0].values, (QVector<double>{10, 20, 40}));
        QCOMPARE(p.series[1].name, QStringLiteral("Q2"));
        QCOMPARE(p.series[1].values, (QVector<double>{5, 15, 25}));
        // The axes: where the file says they are, and which the bars are drawn against.
        QCOMPARE(int(c.axes.size()), 2);
        QCOMPARE(int(p.axisIds[0]), 111);
        QCOMPARE(int(p.axisIds[1]), 222);
        QVERIFY(c.axis(111) && c.axis(222));
        QCOMPARE(int(c.axis(111)->kind), int(ChartAxis::Category));
        QCOMPARE(int(c.axis(222)->kind), int(ChartAxis::Value));
        QVERIFY(c.axis(222)->majorGridlines);
        QVERIFY(!c.axis(111)->majorGridlines);
        QVERIFY(c.axis(222)->autoMin && c.axis(222)->autoMax);
    }

    void chartReadsStackedBars()
    {
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"bar\"/><c:grouping val=\"stacked\"/><c:varyColors val=\"0\"/>")
                             + chartSeries(0, QStringLiteral("A"), {"x", "y"}, {1, 2}) + chartSeries(1, QStringLiteral("B"), {"x", "y"}, {3, 4})
                             + QStringLiteral("<c:gapWidth val=\"150\"/><c:overlap val=\"100\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(c.plots.size()), 1);
        QVERIFY(c.plots[0].horizontal);
        QCOMPARE(int(c.plots[0].grouping), int(ChartGrouping::Stacked));
        QCOMPARE(c.plots[0].overlap, 100);
        // Percent stacked, and the three-dimensional kinds, which are drawn flat.
        QString percent = plot;
        percent.replace(QStringLiteral("\"stacked\""), QStringLiteral("\"percentStacked\""));
        QVERIFY2(readChart(chartPart(percent), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(c.plots[0].grouping), int(ChartGrouping::PercentStacked));
        QString threeD = plot;
        threeD.replace(QStringLiteral("barChart>"), QStringLiteral("bar3DChart>"));
        QVERIFY2(readChart(chartPart(threeD), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(c.plots[0].type), int(ChartType::Bar));
    }

    void chartReadsLinesWithMarkers()
    {
        const QString marked = QStringLiteral("<c:marker><c:symbol val=\"circle\"/><c:size val=\"9\"/><c:spPr><a:solidFill><a:srgbClr val=\"00AA00\"/></a:solidFill></c:spPr></c:marker>");
        const QString bare = QStringLiteral("<c:marker><c:symbol val=\"none\"/></c:marker>");
        const QString plot = QStringLiteral("<c:lineChart><c:grouping val=\"standard\"/><c:varyColors val=\"0\"/>")
                             + chartSeries(0, QStringLiteral("One"), {"a", "b", "c"}, {1, 3, 2}, QStringLiteral("<c:spPr><a:ln w=\"38100\"><a:solidFill><a:srgbClr val=\"0000FF\"/></a:solidFill><a:prstDash val=\"dash\"/></a:ln></c:spPr>"),
                                           marked, QStringLiteral("<c:smooth val=\"1\"/>"))
                             + chartSeries(1, QStringLiteral("Two"), {"a", "b", "c"}, {2, 2, 4}, QString(), bare, QStringLiteral("<c:smooth val=\"0\"/>"))
                             + QStringLiteral("<c:marker val=\"1\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:lineChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const ChartPlot &p = c.plots[0];
        QCOMPARE(int(p.type), int(ChartType::Line));
        QCOMPARE(int(p.grouping), int(ChartGrouping::Standard));
        QVERIFY(p.markers);
        QCOMPARE(int(p.series[0].marker.symbol), int(ChartMarker::Circle));
        QCOMPARE(p.series[0].marker.size, 9.0);
        QCOMPARE(p.series[0].marker.fill, QColor(0x00, 0xAA, 0x00));
        QVERIFY(p.series[0].smooth);
        QCOMPARE(p.series[0].color, QColor(0, 0, 0xFF));   // a line's color is its stroke's
        QCOMPARE(p.series[0].outline.width, 3.0);
        QCOMPARE(p.series[0].outline.dash, QStringLiteral("dash"));
        QCOMPARE(int(p.series[1].marker.symbol), int(ChartMarker::None));
        QVERIFY(!p.series[1].smooth);
        QCOMPARE(p.series[1].color, defaultTheme().colors[Theme::Accent2]);
    }

    void chartReadsPiesWithDataLabels()
    {
        const QString labels = QStringLiteral("<c:dLbls><c:numFmt formatCode=\"0%\" sourceLinked=\"0\"/><c:txPr><a:bodyPr/><a:p><a:pPr><a:defRPr sz=\"1200\" b=\"1\"/></a:pPr><a:endParaRPr lang=\"en-US\"/></a:p></c:txPr>"
                                              "<c:dLblPos val=\"outEnd\"/><c:showLegendKey val=\"0\"/><c:showVal val=\"0\"/><c:showCatName val=\"0\"/><c:showSerName val=\"0\"/><c:showPercent val=\"1\"/><c:showBubbleSize val=\"0\"/>"
                                              "<c:showLeaderLines val=\"1\"/></c:dLbls>");
        const QString point = QStringLiteral("<c:dPt><c:idx val=\"2\"/><c:bubble3D val=\"0\"/><c:spPr><a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill></c:spPr></c:dPt>");
        const QString plot = QStringLiteral("<c:pieChart><c:varyColors val=\"1\"/>")
                             + chartSeries(0, QStringLiteral("Share"), {"a", "b", "c", "d"}, {1, 2, 3, 4}, QString(), point + labels)
                             + QStringLiteral("<c:firstSliceAng val=\"90\"/></c:pieChart>");
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const ChartPlot &p = c.plots[0];
        QCOMPARE(int(p.type), int(ChartType::Pie));
        QVERIFY(p.varyColors);
        QCOMPARE(p.firstSliceAngle, 90);
        const ChartSeries &s = p.series[0];
        QVERIFY(s.labels.percent);
        QVERIFY(!s.labels.value && !s.labels.category && !s.labels.series);
        QCOMPARE(s.labels.format, QStringLiteral("0%"));
        QCOMPARE(s.labels.position, QStringLiteral("outEnd"));
        QCOMPARE(s.labels.font.size, 12.0);
        QCOMPARE(s.labels.font.bold, 1);
        QCOMPARE(int(s.pointColors.size()), 1);
        QCOMPARE(s.pointColors.value(2), QColor(0, 0xFF, 0));
        QVERIFY(c.axes.isEmpty());   // a pie has no axes
        // A doughnut has its hole.
        QString ring = plot;
        ring.replace(QStringLiteral("pieChart"), QStringLiteral("doughnutChart")).replace(QStringLiteral("<c:firstSliceAng val=\"90\"/>"), QStringLiteral("<c:firstSliceAng val=\"0\"/><c:holeSize val=\"60\"/>"));
        QVERIFY2(readChart(chartPart(ring), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(c.plots[0].type), int(ChartType::Doughnut));
        QCOMPARE(c.plots[0].holeSize, 60);
    }

    void chartReadsScatterPlots()
    {
        const QString xs = QStringLiteral("<c:xVal><c:numRef><c:f>Sheet1!$A$2:$A$5</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val=\"4\"/>"
                                          "<c:pt idx=\"0\"><c:v>1</c:v></c:pt><c:pt idx=\"1\"><c:v>2.5</c:v></c:pt><c:pt idx=\"2\"><c:v>3</c:v></c:pt><c:pt idx=\"3\"><c:v>7</c:v></c:pt></c:numCache></c:numRef></c:xVal>");
        const QString ys = QStringLiteral("<c:yVal><c:numLit><c:formatCode>General</c:formatCode><c:ptCount val=\"4\"/>"
                                          "<c:pt idx=\"0\"><c:v>2</c:v></c:pt><c:pt idx=\"1\"><c:v>4</c:v></c:pt><c:pt idx=\"3\"><c:v>9</c:v></c:pt></c:numLit></c:yVal>");
        const QString series = QStringLiteral("<c:ser><c:idx val=\"0\"/><c:order val=\"0\"/><c:tx><c:v>Trial</c:v></c:tx><c:spPr><a:ln w=\"19050\"><a:noFill/></a:ln></c:spPr>"
                                              "<c:marker><c:symbol val=\"diamond\"/><c:size val=\"7\"/></c:marker>") + xs + ys + QStringLiteral("<c:smooth val=\"0\"/></c:ser>");
        const QString axes = QStringLiteral("<c:valAx><c:axId val=\"111\"/><c:scaling><c:orientation val=\"minMax\"/><c:max val=\"10\"/><c:min val=\"0\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"b\"/>"
                                            "<c:numFmt formatCode=\"General\" sourceLinked=\"1\"/><c:crossAx val=\"222\"/><c:crosses val=\"autoZero\"/><c:crossBetween val=\"midCat\"/></c:valAx>"
                                            "<c:valAx><c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"l\"/><c:majorGridlines/>"
                                            "<c:numFmt formatCode=\"General\" sourceLinked=\"1\"/><c:crossAx val=\"111\"/><c:crosses val=\"autoZero\"/><c:crossBetween val=\"midCat\"/></c:valAx>");
        const QString plot = QStringLiteral("<c:scatterChart><c:scatterStyle val=\"lineMarker\"/><c:varyColors val=\"0\"/>") + series
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:scatterChart>") + axes;
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const ChartPlot &p = c.plots[0];
        QCOMPARE(int(p.type), int(ChartType::Scatter));
        QCOMPARE(p.series[0].name, QStringLiteral("Trial"));
        QCOMPARE(p.series[0].xValues, (QVector<double>{1, 2.5, 3, 7}));
        QCOMPARE(int(p.series[0].values.size()), 4);
        QCOMPARE(p.series[0].values[0], 2.0);
        QCOMPARE(p.series[0].values[1], 4.0);
        QVERIFY(std::isnan(p.series[0].values[2]));   // a point the file has no value for
        QCOMPARE(p.series[0].values[3], 9.0);
        QVERIFY(p.series[0].outline.none);            // markers alone
        QCOMPARE(int(p.series[0].marker.symbol), int(ChartMarker::Diamond));
        QCOMPARE(int(c.axes.size()), 2);
        QCOMPARE(int(c.axis(111)->kind), int(ChartAxis::Value));
        QVERIFY(!c.axis(111)->autoMin && !c.axis(111)->autoMax);
        QCOMPARE(c.axis(111)->min, 0.0);
        QCOMPARE(c.axis(111)->max, 10.0);
        QVERIFY(c.axis(222)->autoMax);
    }

    void chartTakesColorsFromTheFileAndTheTheme()
    {
        const QString accent = QStringLiteral("<c:spPr><a:solidFill><a:schemeClr val=\"accent4\"/></a:solidFill></c:spPr>");
        const QString darker = QStringLiteral("<c:spPr><a:solidFill><a:schemeClr val=\"accent3\"><a:lumMod val=\"50000\"/></a:schemeClr></a:solidFill></c:spPr>");
        QString series = chartSeries(0, QStringLiteral("S0"), {"a"}, {1}, accent) + chartSeries(1, QStringLiteral("S1"), {"a"}, {1}, darker);
        for (int i = 2; i < 14; ++i) series += chartSeries(i, QStringLiteral("S%1").arg(i), {"a"}, {1});
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + series + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        Chart c;
        QString err;
        const Theme theme = defaultTheme();
        QVERIFY2(readChart(chartPart(plot), theme, ColorMap(), &c, &err), qPrintable(err));
        const QVector<ChartSeries> &s = c.plots[0].series;
        QCOMPARE(int(s.size()), 14);
        QCOMPARE(s[0].color, theme.colors[Theme::Accent4]);
        QVERIFY(s[0].colorFromFile);
        // A transform of a theme color is applied.
        QVERIFY(s[1].colorFromFile);
        QVERIFY(s[1].color.lightnessF() < theme.colors[Theme::Accent3].lightnessF() * 0.7);
        QVERIFY(qAbs(s[1].color.hueF() - theme.colors[Theme::Accent3].hueF()) < 0.02);
        // The rest take the accents in order (the file's own colors don't use them up).
        for (int i = 2; i < 6; ++i) {
            QVERIFY(!s[i].colorFromFile);
            QCOMPARE(s[i].color, theme.colors[Theme::Accent1 + i]);
        }
        // More series than accents: the accents again, each darker, then lighter.
        QVERIFY(qAbs(s[6].color.hueF() - theme.colors[Theme::Accent1].hueF()) < 0.02);
        QVERIFY(s[6].color != theme.colors[Theme::Accent1]);
        QVERIFY(s[6].color.lightnessF() < theme.colors[Theme::Accent1].lightnessF());
        QVERIFY(s[12].color.lightnessF() > s[6].color.lightnessF());
        QVERIFY(s[12].color != theme.colors[Theme::Accent1]);
        // The color map renames what the file asks for: a dark slide's "tx1" is its light one.
        ColorMap dark;
        dark.tx1 = Theme::Light1;
        dark.bg1 = Theme::Dark1;
        const QString text = QStringLiteral("<c:txPr><a:bodyPr/><a:p><a:pPr><a:defRPr><a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill></a:defRPr></a:pPr><a:endParaRPr lang=\"en-US\"/></a:p></c:txPr>");
        QVERIFY2(readChart(chartPart(plot, QString(), QString(), text), theme, dark, &c, &err), qPrintable(err));
        QCOMPARE(c.font.color, theme.colors[Theme::Light1]);
    }

    void chartStyleColorsTheSeriesTheFileLeavesBare()
    {
        QString series;
        for (int i = 0; i < 4; ++i) series += chartSeries(i, QStringLiteral("S%1").arg(i), {"a", "b"}, {1.0 + i, 2.0});
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + series + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        const Theme theme = defaultTheme();
        auto styled = [&](int style) {
            QByteArray xml = chartPart(plot);
            xml.replace("<c:roundedCorners val=\"0\"/>", QByteArray("<c:roundedCorners val=\"0\"/><c:style val=\"") + QByteArray::number(style) + "\"/>");
            Chart c;
            QString err;
            [&] { QVERIFY2(readChart(xml, theme, ColorMap(), &c, &err), qPrintable(err)); }();
            return c;
        };
        // Style 2, and what has none, takes the accents; so do the styles that count up from it eight at a time.
        for (int style : {0, 2, 10, 26, 42}) {
            const Chart c = styled(style);
            QCOMPARE(c.style, style == 0 ? 2 : style);
            for (int i = 0; i < 4; ++i) QCOMPARE(c.plots[0].series[i].color, theme.colors[Theme::Accent1 + i]);
        }
        // Style 1 is shades of gray, darkest first.
        const Chart gray = styled(1);
        for (int i = 0; i < 4; ++i) {
            QVERIFY2(gray.plots[0].series[i].color.saturationF() < 0.05, qPrintable(gray.plots[0].series[i].color.name()));
            if (i > 0) QVERIFY(gray.plots[0].series[i].color.lightnessF() > gray.plots[0].series[i - 1].color.lightnessF());
        }
        // Style 3 is shades of the first accent, style 8 of the sixth (and 19 again of the first).
        for (const QPair<int, int> &p : {qMakePair(3, 0), qMakePair(8, 5), qMakePair(19, 0)}) {
            const Chart c = styled(p.first);
            const QColor accent = theme.colors[Theme::Accent1 + p.second];
            for (int i = 0; i < 4; ++i) {
                const QColor shade = c.plots[0].series[i].color;
                QVERIFY2(qAbs(shade.hueF() - accent.hueF()) < 0.03 || accent.saturationF() < 0.1, qPrintable(QString("%1: %2").arg(p.first).arg(shade.name())));
                if (i > 0) QVERIFY(shade.lightnessF() > c.plots[0].series[i - 1].color.lightnessF());
            }
            QVERIFY(c.plots[0].series[0].color.lightnessF() < accent.lightnessF());
            QVERIFY(c.plots[0].series[3].color.lightnessF() > accent.lightnessF());
        }
        // A color the file gives stays.
        const QString own = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>")
                            + chartSeries(0, QStringLiteral("A"), {"a"}, {1}, QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></c:spPr>")) + chartSeries(1, QStringLiteral("B"), {"a"}, {2})
                            + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        QByteArray xml = chartPart(own);
        xml.replace("<c:roundedCorners val=\"0\"/>", "<c:roundedCorners val=\"0\"/><c:style val=\"3\"/>");
        Chart c;
        QString err;
        QVERIFY2(readChart(xml, theme, ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(c.plots[0].series[0].color, QColor(0xFF, 0, 0));
        QVERIFY(!c.plots[0].series[1].colorFromFile);
        QVERIFY(qAbs(c.plots[0].series[1].color.hueF() - theme.colors[Theme::Accent1].hueF()) < 0.03);
    }

    void chartReadsTitleLegendAndAxes()
    {
        const QString title = QStringLiteral("<c:title><c:tx><c:rich><a:bodyPr/><a:p><a:pPr><a:defRPr sz=\"1400\" b=\"1\"/></a:pPr><a:r><a:rPr lang=\"en-US\" sz=\"1800\" b=\"0\"/><a:t>Sales by </a:t></a:r>"
                                             "<a:r><a:rPr lang=\"en-US\" sz=\"1800\" b=\"0\"/><a:t>region</a:t></a:r></a:p><a:p><a:r><a:rPr lang=\"en-US\" sz=\"1000\"/><a:t>in thousands</a:t></a:r></a:p></c:rich></c:tx>"
                                             "<c:overlay val=\"0\"/></c:title><c:autoTitleDeleted val=\"0\"/>");
        const QString axes = QStringLiteral("<c:catAx><c:axId val=\"111\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"b\"/>"
                                            "<c:title><c:tx><c:rich><a:bodyPr/><a:p><a:r><a:t>Region</a:t></a:r></a:p></c:rich></c:tx><c:overlay val=\"0\"/></c:title>"
                                            "<c:numFmt formatCode=\"General\" sourceLinked=\"1\"/><c:majorTickMark val=\"none\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"low\"/>"
                                            "<c:crossAx val=\"222\"/><c:crosses val=\"autoZero\"/><c:auto val=\"1\"/><c:lblAlgn val=\"ctr\"/><c:lblOffset val=\"100\"/><c:tickLblSkip val=\"2\"/><c:noMultiLvlLbl val=\"0\"/></c:catAx>"
                                            "<c:valAx><c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/><c:max val=\"50\"/><c:min val=\"-10\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"l\"/>"
                                            "<c:majorGridlines><c:spPr><a:ln w=\"9525\"><a:solidFill><a:srgbClr val=\"D9D9D9\"/></a:solidFill></a:ln></c:spPr></c:majorGridlines>"
                                            "<c:title><c:tx><c:rich><a:bodyPr rot=\"-5400000\" vert=\"horz\"/><a:p><a:r><a:t>Units</a:t></a:r></a:p></c:rich></c:tx><c:overlay val=\"0\"/></c:title>"
                                            "<c:numFmt formatCode=\"#,##0&quot; units&quot;\" sourceLinked=\"0\"/><c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/>"
                                            "<c:txPr><a:bodyPr/><a:p><a:pPr><a:defRPr sz=\"900\"/></a:pPr><a:endParaRPr lang=\"en-US\"/></a:p></c:txPr>"
                                            "<c:crossAx val=\"111\"/><c:crosses val=\"autoZero\"/><c:crossBetween val=\"between\"/><c:majorUnit val=\"10\"/></c:valAx>");
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("Q1"), {"East", "West"}, {10, 20})
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + axes;
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot, title, QStringLiteral("<c:legend><c:legendPos val=\"r\"/><c:overlay val=\"0\"/></c:legend>")), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QVERIFY(c.title.present);
        QCOMPARE(c.title.text, QStringLiteral("Sales by region\nin thousands"));
        QCOMPARE(c.title.font.size, 18.0);   // the first run's, over the paragraph's default
        QCOMPARE(c.title.font.bold, 0);
        QVERIFY(!c.title.overlay);
        QVERIFY(c.legend.shown);
        QCOMPARE(c.legend.position, QStringLiteral("r"));
        const ChartAxis *cat = c.axis(111), *val = c.axis(222);
        QVERIFY(cat && val);
        QCOMPARE(cat->title.text, QStringLiteral("Region"));
        QCOMPARE(cat->labelPosition, QStringLiteral("low"));
        QCOMPARE(cat->majorTick, QStringLiteral("none"));
        QCOMPARE(cat->labelSkip, 2);
        QCOMPARE(cat->position, QStringLiteral("b"));
        QCOMPARE(val->title.text, QStringLiteral("Units"));
        QCOMPARE(val->position, QStringLiteral("l"));
        QVERIFY(!val->autoMin && !val->autoMax);
        QCOMPARE(val->min, -10.0);
        QCOMPARE(val->max, 50.0);
        QCOMPARE(val->majorUnit, 10.0);
        QVERIFY(val->majorGridlines);
        QCOMPARE(val->gridline.color, QColor(0xD9, 0xD9, 0xD9));
        QCOMPARE(val->gridline.width, 0.75);
        QCOMPARE(val->format, QStringLiteral("#,##0\" units\""));
        QVERIFY(!val->formatLinked);
        QCOMPARE(val->font.size, 9.0);
        QVERIFY(val->between);
        // No title element: none, and no legend: none.
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QVERIFY(!c.title.present);
        QVERIFY(!c.legend.shown);
    }

    void chartReadsAreasAndLiterals()
    {
        // A category the file stores as numbers, as text.
        const QString cats = QStringLiteral("<c:cat><c:numRef><c:f>Sheet1!$A$2:$A$4</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val=\"3\"/>"
                                            "<c:pt idx=\"0\"><c:v>1774</c:v></c:pt><c:pt idx=\"2\"><c:v>1790</c:v></c:pt></c:numCache></c:numRef></c:cat>");
        const QString vals = QStringLiteral("<c:val><c:numRef><c:f>Sheet1!$B$2:$B$4</c:f><c:numCache><c:formatCode>0.0</c:formatCode><c:ptCount val=\"3\"/>"
                                            "<c:pt idx=\"1\"><c:v>0.217</c:v></c:pt><c:pt idx=\"2\"><c:v>7.8718E-2</c:v></c:pt></c:numCache></c:numRef></c:val>");
        const QString plot = QStringLiteral("<c:areaChart><c:grouping val=\"stacked\"/><c:varyColors val=\"0\"/><c:ser><c:idx val=\"0\"/><c:order val=\"0\"/>") + cats + vals
                             + QStringLiteral("</c:ser><c:axId val=\"111\"/><c:axId val=\"222\"/></c:areaChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(c.plots[0].type), int(ChartType::Area));
        QCOMPARE(int(c.plots[0].grouping), int(ChartGrouping::Stacked));
        const ChartSeries &s = c.plots[0].series[0];
        QCOMPARE(s.categories, QStringList({"1774", "", "1790"}));
        QCOMPARE(int(s.values.size()), 3);
        QVERIFY(std::isnan(s.values[0]));
        QCOMPARE(s.values[1], 0.217);
        QCOMPARE(s.values[2], 0.078718);
        QCOMPARE(s.format, QStringLiteral("0.0"));
        QCOMPARE(s.name, QStringLiteral("Series1"));   // the file names none
        QCOMPARE(c.blanksAs, QStringLiteral("gap"));
    }

    void chartKeepsWhatItCannotDrawAsColumns()
    {
        // Kinds not drawn yet read as columns of their values, with the file's name for them.
        const QString plot = QStringLiteral("<c:radarChart><c:radarStyle val=\"marker\"/>") + chartSeries(0, QStringLiteral("R"), {"a", "b", "c"}, {3, 1, 2})
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:radarChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(c.plots.size()), 1);
        QCOMPARE(int(c.plots[0].type), int(ChartType::Other));
        QCOMPARE(c.plots[0].element, QStringLiteral("radarChart"));
        QCOMPARE(c.plots[0].series[0].values, (QVector<double>{3, 1, 2}));
        // Two kinds at once: columns with a line over them.
        const QString both = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("Bars"), {"a", "b"}, {3, 1})
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart><c:lineChart><c:grouping val=\"standard\"/>") + chartSeries(1, QStringLiteral("Line"), {"a", "b"}, {2, 2})
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:lineChart>") + chartAxes();
        QVERIFY2(readChart(chartPart(both), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(c.plots.size()), 2);
        QCOMPARE(int(c.plots[0].type), int(ChartType::Bar));
        QCOMPARE(int(c.plots[1].type), int(ChartType::Line));
        // The colors continue from one plot to the next.
        QCOMPARE(c.plots[0].series[0].color, defaultTheme().colors[Theme::Accent1]);
        QCOMPARE(c.plots[1].series[0].color, defaultTheme().colors[Theme::Accent2]);
    }

    void chartRefusesWhatIsNotAChart()
    {
        Chart c;
        QString err;
        QVERIFY(!readChart(QByteArray("<a:theme xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\"/>"), defaultTheme(), ColorMap(), &c, &err));
        QVERIFY(!err.isEmpty());
        err.clear();
        QVERIFY(!readChart(QByteArray("this is not XML"), defaultTheme(), ColorMap(), &c, &err));
        QVERIFY(!err.isEmpty());
        QVERIFY(!readChart(QByteArray(), defaultTheme(), ColorMap(), &c, nullptr));
        // A chart with no series is one: it draws as an empty chart.
        QVERIFY2(readChart(chartPart(QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes()),
                           defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QVERIFY(c.plots[0].series.isEmpty());
        QVERIFY(chartSvg(c, QSizeF(300, 200)).contains("<svg"));
    }

    void chartFramesAreFoundByTheirUri()
    {
        const QString frame = QStringLiteral("<p:graphicFrame xmlns:p=\"%1\" xmlns:a=\"%2\" xmlns:r=\"%3\"><p:nvGraphicFramePr><p:cNvPr id=\"4\" name=\"Chart 3\"/><p:cNvGraphicFramePr/><p:nvPr/></p:nvGraphicFramePr>"
                                            "<p:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"100\" cy=\"100\"/></p:xfrm><a:graphic><a:graphicData uri=\"%4\"><c:chart xmlns:c=\"%4\" r:id=\"rId7\"/></a:graphicData></a:graphic></p:graphicFrame>");
        QDomDocument doc;
        const QString chartUri = QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/chart");
        QVERIFY(parseXml(frame.arg(QString(ns::P), QString(ns::A), QString(ns::R), chartUri).toUtf8(), &doc));
        QVERIFY(isChartFrame(doc.documentElement()));
        QCOMPARE(chartFrameRelId(doc.documentElement()), QStringLiteral("rId7"));
        QVERIFY(parseXml(frame.arg(QString(ns::P), QString(ns::A), QString(ns::R), QStringLiteral("urn:test:other")).toUtf8(), &doc));
        QVERIFY(!isChartFrame(doc.documentElement()));
        QVERIFY(!isChartFrame(QDomElement()));
    }

    void chartFormatsNumbersAsTheirCodesSay()
    {
        struct Case { double value; const char *code, *text; };
        const Case cases[] = {
            {1234.5, "General", "1234.5"}, {0.1 + 0.2, "General", "0.3"}, {0, "General", "0"}, {16.112566, "", "16.112566"}, {-2, "General", "-2"},
            {1234.567, "0", "1235"}, {1234.567, "0.0", "1234.6"}, {1234.567, "#,##0.00", "1,234.57"}, {1234567, "#,##0", "1,234,567"}, {12, "000", "012"},
            {0.256, "0%", "26%"}, {0.256, "0.0%", "25.6%"}, {1, "0.0%", "100.0%"}, {-1.5, "0.0", "-1.5"},
            {5, "$#,##0.00;($#,##0.00)", "$5.00"}, {-5, "$#,##0.00;($#,##0.00)", "($5.00)"}, {0, "0;-0;\"none\"", "none"},
            {5, "0\" units\"", "5 units"}, {1500, "0.0,\"k\"", "1.5k"}, {2500000, "0.0,,\"M\"", "2.5M"}, {12345.678, "0.00E+00", "1.23E+04"}, {0.5, "#.##", ".5"}, {3, "0.0#", "3.0"},
            {45000, "m/d/yyyy", "3/15/2023"}, {45000, "yyyy-mm-dd", "2023-03-15"}, {45000, "mmm-yy", "Mar-23"}, {45000, "dddd", "Wednesday"}, {45000.75, "h:mm AM/PM", "6:00 PM"},
            {45000.75, "d mmmm yyyy, hh:mm", "15 March 2023, 18:00"}};
        for (const Case &c : cases) QCOMPARE(formatChartNumber(c.value, QString::fromLatin1(c.code)), QString::fromLatin1(c.text));
        QVERIFY(formatChartNumber(std::nan(""), QStringLiteral("0")).isEmpty());
    }

    void chartSvgIsInPointsWithTheTextAsText()
    {
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("Revenue"), {"East", "West"}, {10, 20})
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        const QString title = QStringLiteral("<c:title><c:tx><c:rich><a:bodyPr/><a:p><a:r><a:rPr lang=\"en-US\"/><a:t>Quarterly &amp; yearly</a:t></a:r></a:p></c:rich></c:tx><c:overlay val=\"0\"/></c:title>");
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot, title, QStringLiteral("<c:legend><c:legendPos val=\"b\"/></c:legend>")), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QByteArray svg = chartSvg(c, QSizeF(432, 252));
        QSvgRenderer r(svg);
        QVERIFY(r.isValid());
        QCOMPARE(r.viewBoxF(), QRectF(0, 0, 432, 252));
        // Text is text, in the chart's font.
        const QString text = QString::fromUtf8(svg);
        QVERIFY(text.contains(QStringLiteral("<text")));
        QVERIFY(text.contains(QStringLiteral(">Quarterly &amp; yearly<")));
        QVERIFY(text.contains(QStringLiteral(">East<")));
        QVERIFY(text.contains(QStringLiteral(">West<")));
        QVERIFY(text.contains(QStringLiteral(">Revenue<")));   // the legend
        QVERIFY(text.contains(QStringLiteral("font-family=\"%1\"").arg(defaultTheme().minorLatin)));
        // An empty size draws nothing and doesn't fail.
        QVERIFY(chartSvg(c, QSizeF(0, 0)).contains("<svg"));
    }

    void chartSvgDrawsColumnsInProportion()
    {
        const QString red = QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></c:spPr>");
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("S"), {"a", "b", "c"}, {10, 20, 40}, red)
                             + QStringLiteral("<c:gapWidth val=\"100\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QImage img = renderChart(c, QSizeF(400, 300), 2);
        // Ink in the plot area besides the bars: gridlines and labels.
        int inked = 0, redPixels = 0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x) {
                const QRgb p = img.pixel(x, y);
                if (isColor(p, Qt::red, 40)) ++redPixels;
                else if (qRed(p) < 235 || qGreen(p) < 235 || qBlue(p) < 235) ++inked;
            }
        QVERIFY(inked > 400);
        QVERIFY(redPixels > 1000);
        const QVector<QRect> bars = colorRuns(img, Qt::red, false);
        QCOMPARE(int(bars.size()), 3);
        // Standing on one line, equally wide, in the order of their values and 1 : 2 : 4 high.
        QCOMPARE(bars[0].bottom(), bars[1].bottom());
        QCOMPARE(bars[1].bottom(), bars[2].bottom());
        QVERIFY(qAbs(bars[0].width() - bars[1].width()) <= 2 && qAbs(bars[1].width() - bars[2].width()) <= 2);
        QVERIFY(bars[0].left() < bars[1].left() && bars[1].left() < bars[2].left());
        QVERIFY2(qAbs(bars[1].height() / double(bars[0].height()) - 2) < 0.08, qPrintable(QString::number(bars[1].height() / double(bars[0].height()))));
        QVERIFY2(qAbs(bars[2].height() / double(bars[0].height()) - 4) < 0.15, qPrintable(QString::number(bars[2].height() / double(bars[0].height()))));
        // Gaps of 100% leave a bar's width between bars.
        QVERIFY2(qAbs((bars[1].left() - bars[0].right()) / double(bars[0].width()) - 1) < 0.1, qPrintable(QString::number((bars[1].left() - bars[0].right()) / double(bars[0].width()))));
        // The bars stand inside the picture, below the top and clear of the edges.
        QVERIFY(bars[0].left() > 20 && bars[2].right() < img.width() - 10);
        QVERIFY(bars[2].top() > 10 && bars[2].bottom() < img.height() - 20);
    }

    void chartSvgStacksColumns()
    {
        const QString red = QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></c:spPr>");
        const QString blue = QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"0000FF\"/></a:solidFill></c:spPr>");
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"stacked\"/>") + chartSeries(0, QStringLiteral("R"), {"a", "b"}, {10, 20}, red)
                             + chartSeries(1, QStringLiteral("B"), {"a", "b"}, {30, 10}, blue) + QStringLiteral("<c:gapWidth val=\"50\"/><c:overlap val=\"100\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QImage img = renderChart(c, QSizeF(400, 300), 2);
        const QVector<QRect> reds = colorRuns(img, Qt::red, false), blues = colorRuns(img, Qt::blue, false);
        QCOMPARE(int(reds.size()), 2);
        QCOMPARE(int(blues.size()), 2);
        // Each blue segment sits on its red one; the heights are the values, 10 + 30 and 20 + 10.
        QVERIFY(qAbs(blues[0].bottom() - reds[0].top()) <= 2);
        QVERIFY(qAbs(blues[1].bottom() - reds[1].top()) <= 2);
        QVERIFY2(qAbs(blues[0].height() / double(reds[0].height()) - 3) < 0.15, qPrintable(QString::number(blues[0].height() / double(reds[0].height()))));
        QVERIFY2(qAbs(reds[1].height() / double(blues[1].height()) - 2) < 0.1, qPrintable(QString::number(reds[1].height() / double(blues[1].height()))));
        const double total0 = reds[0].height() + blues[0].height(), total1 = reds[1].height() + blues[1].height();
        QVERIFY2(qAbs(total0 / total1 - 4.0 / 3.0) < 0.06, qPrintable(QString::number(total0 / total1)));
        // Percent stacked: both stacks reach the top.
        QString percent = plot;
        percent.replace(QStringLiteral("\"stacked\""), QStringLiteral("\"percentStacked\""));
        QVERIFY2(readChart(chartPart(percent), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QImage full = renderChart(c, QSizeF(400, 300), 2);
        const QVector<QRect> a = colorRuns(full, Qt::red, false), b = colorRuns(full, Qt::blue, false);
        QCOMPARE(int(b.size()), 2);
        QVERIFY(qAbs(b[0].top() - b[1].top()) <= 2);
        QVERIFY2(qAbs(a[0].height() / double(b[0].height()) - 1.0 / 3.0) < 0.05, qPrintable(QString::number(a[0].height() / double(b[0].height()))));
    }

    void chartSvgDrawsBarsAcross()
    {
        const QString red = QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></c:spPr>");
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"bar\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("S"), {"a", "b", "c"}, {10, 20, 40}, red)
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>")
                             + QStringLiteral("<c:catAx><c:axId val=\"111\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"l\"/><c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/>"
                                              "<c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"222\"/><c:crosses val=\"autoZero\"/><c:auto val=\"1\"/><c:lblAlgn val=\"ctr\"/><c:lblOffset val=\"100\"/></c:catAx>"
                                              "<c:valAx><c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"b\"/><c:majorGridlines/><c:numFmt formatCode=\"General\" sourceLinked=\"1\"/>"
                                              "<c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"111\"/><c:crosses val=\"autoZero\"/><c:crossBetween val=\"between\"/></c:valAx>");
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QVERIFY(c.plots[0].horizontal);
        const QImage img = renderChart(c, QSizeF(400, 300), 2);
        const QVector<QRect> bars = colorRuns(img, Qt::red, true);   // from the top
        QCOMPARE(int(bars.size()), 3);
        // The first category is at the bottom: from the top the bars are 40, 20, and 10 wide, from one edge.
        QCOMPARE(bars[0].left(), bars[1].left());
        QCOMPARE(bars[1].left(), bars[2].left());
        QVERIFY2(qAbs(bars[0].width() / double(bars[1].width()) - 2) < 0.08, qPrintable(QString::number(bars[0].width() / double(bars[1].width()))));
        QVERIFY2(qAbs(bars[1].width() / double(bars[2].width()) - 2) < 0.12, qPrintable(QString::number(bars[1].width() / double(bars[2].width()))));
        QVERIFY(qAbs(bars[0].height() - bars[2].height()) <= 2);
    }

    void chartSvgPutsReversedCategoriesFirstAtTheTop()
    {
        const QString red = QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></c:spPr>");
        auto bars = [&](const QString &order, const QString &crosses) {
            return QStringLiteral("<c:barChart><c:barDir val=\"bar\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("S"), {"a", "b", "c"}, {10, 20, 40}, red)
                   + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>")
                   + QStringLiteral("<c:catAx><c:axId val=\"111\"/><c:scaling><c:orientation val=\"%1\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"l\"/><c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/>"
                                    "<c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"222\"/><c:crosses val=\"autoZero\"/><c:auto val=\"1\"/><c:lblAlgn val=\"ctr\"/><c:lblOffset val=\"100\"/></c:catAx>"
                                    "<c:valAx><c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"b\"/><c:majorGridlines/><c:numFmt formatCode=\"General\" sourceLinked=\"1\"/>"
                                    "<c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"111\"/><c:crosses val=\"%2\"/><c:crossBetween val=\"between\"/></c:valAx>").arg(order, crosses);
        };
        // Where the value axis' labels are: dark text along the bottom of the picture, or along its top. (The bars are red, the lines gray.)
        auto darkIn = [](const QImage &img, int from, int to) {
            for (int y = from; y < to; ++y)
                for (int x = 0; x < img.width(); ++x)
                    if (qRed(img.pixel(x, y)) < 100 && qGreen(img.pixel(x, y)) < 100 && qBlue(img.pixel(x, y)) < 120) return true;
            return false;
        };
        Chart plain, turned, atTop;
        QString err;
        QVERIFY2(readChart(chartPart(bars(QStringLiteral("minMax"), QStringLiteral("autoZero"))), defaultTheme(), ColorMap(), &plain, &err), qPrintable(err));
        QVERIFY2(readChart(chartPart(bars(QStringLiteral("maxMin"), QStringLiteral("max"))), defaultTheme(), ColorMap(), &turned, &err), qPrintable(err));
        QVERIFY2(readChart(chartPart(bars(QStringLiteral("maxMin"), QStringLiteral("autoZero"))), defaultTheme(), ColorMap(), &atTop, &err), qPrintable(err));
        QVERIFY(plain.axis(111) && !plain.axis(111)->reversed);
        QVERIFY(turned.axis(111)->reversed);
        // The first category is at the bottom, or at the top once the categories are turned round.
        const QVector<QRect> up = colorRuns(renderChart(plain, QSizeF(400, 300), 2), Qt::red, true), down = colorRuns(renderChart(turned, QSizeF(400, 300), 2), Qt::red, true);
        QCOMPARE(int(up.size()), 3);
        QCOMPARE(int(down.size()), 3);
        QVERIFY(up[0].width() > up[2].width());
        QVERIFY(down[0].width() < down[2].width());
        // The value axis crosses at the first category, which turned round is the top, unless it is said to cross at the last.
        const QImage a = renderChart(plain, QSizeF(400, 300), 2), b = renderChart(turned, QSizeF(400, 300), 2), t = renderChart(atTop, QSizeF(400, 300), 2);
        QVERIFY(darkIn(a, 540, 600) && !darkIn(a, 0, 60));
        QVERIFY(darkIn(b, 540, 600) && !darkIn(b, 0, 60));
        QVERIFY(darkIn(t, 0, 60) && !darkIn(t, 540, 600));
    }

    void chartSvgDrawsPieSlices()
    {
        const QString colored = QStringLiteral("<c:dPt><c:idx val=\"0\"/><c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></c:spPr></c:dPt>"
                                               "<c:dPt><c:idx val=\"1\"/><c:spPr><a:solidFill><a:srgbClr val=\"0000FF\"/></a:solidFill></c:spPr></c:dPt>"
                                               "<c:dPt><c:idx val=\"2\"/><c:spPr><a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill></c:spPr></c:dPt>");
        const QString plot = QStringLiteral("<c:pieChart><c:varyColors val=\"1\"/>") + chartSeries(0, QStringLiteral("S"), {"a", "b", "c"}, {1, 1, 2}, QString(), colored)
                             + QStringLiteral("<c:firstSliceAng val=\"0\"/></c:pieChart>");
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QImage img = renderChart(c, QSizeF(300, 300), 2);
        const QColor green(0, 0xFF, 0);
        const double r = colorArea(img, Qt::red), b = colorArea(img, Qt::blue), g = colorArea(img, green);
        QVERIFY(r > 5000);
        QVERIFY2(qAbs(b / r - 1) < 0.08, qPrintable(QString::number(b / r)));
        QVERIFY2(qAbs(g / r - 2) < 0.12, qPrintable(QString::number(g / r)));
        // The first slice starts at twelve o'clock and runs clockwise: red upper right, blue lower right, green the left half.
        const QPointF cr = colorCentroid(img, Qt::red), cb = colorCentroid(img, Qt::blue), cg = colorCentroid(img, green);
        QVERIFY(cr.x() > cg.x() && cb.x() > cg.x());
        QVERIFY(cr.y() < cb.y());
        QVERIFY(qAbs(cg.y() - (cr.y() + cb.y()) / 2) < 12);
        // A pie is round: its sides are as wide as it is high.
        const QRect box = colorBounds(img, QColor(), true);
        QVERIFY2(qAbs(box.width() - box.height()) <= 3, qPrintable(QString("%1 x %2").arg(box.width()).arg(box.height())));
        // Percentages as the slices' labels, as text.
        QString labeled = plot;
        labeled.replace(QStringLiteral("<c:firstSliceAng"),
                        QStringLiteral("<c:dLbls><c:showLegendKey val=\"0\"/><c:showVal val=\"0\"/><c:showCatName val=\"0\"/><c:showSerName val=\"0\"/><c:showPercent val=\"1\"/><c:showBubbleSize val=\"0\"/></c:dLbls><c:firstSliceAng"));
        QVERIFY2(readChart(chartPart(labeled), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QVERIFY(c.plots[0].series[0].labels.percent);   // the plot's labels are the series'
        const QString svg = QString::fromUtf8(chartSvg(c, QSizeF(300, 300)));
        QVERIFY(svg.contains(QStringLiteral(">50%<")));
        QCOMPARE(int(svg.count(QStringLiteral(">25%<"))), 2);
    }

    void chartSvgDrawsLinesAndMarkers()
    {
        const QString line = QStringLiteral("<c:spPr><a:ln w=\"28575\"><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></a:ln></c:spPr>");
        const QString plot = QStringLiteral("<c:lineChart><c:grouping val=\"standard\"/>") + chartSeries(0, QStringLiteral("S"), {"a", "b", "c", "d"}, {1, 2, 3, 4}, line,
                                                                                                      QStringLiteral("<c:marker><c:symbol val=\"square\"/><c:size val=\"9\"/><c:spPr><a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill></c:spPr></c:marker>"))
                             + QStringLiteral("<c:marker val=\"1\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:lineChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QImage img = renderChart(c, QSizeF(400, 300), 2);
        QVERIFY(colorArea(img, Qt::red) > 500);
        // Four square markers, in the file's color, along the rising line.
        const QVector<QPointF> markers = colorComponents(img, QColor(0, 0xFF, 0), 20);
        QCOMPARE(int(markers.size()), 4);
        for (int i = 1; i < markers.size(); ++i) {
            QVERIFY(markers[i].x() > markers[i - 1].x());
            QVERIFY(markers[i].y() < markers[i - 1].y());
        }
        // Equal steps in the values are equal steps up the page.
        QVERIFY2(qAbs((markers[1].y() - markers[0].y()) - (markers[3].y() - markers[2].y())) < 2, "unequal steps");
        // Points sit in the middle of their categories.
        QVERIFY(qAbs((markers[1].x() - markers[0].x()) - (markers[3].x() - markers[2].x())) < 2);
    }

    void chartSvgDrawsScatterPoints()
    {
        const QString plot = QStringLiteral("<c:scatterChart><c:scatterStyle val=\"lineMarker\"/>")
                             + QStringLiteral("<c:ser><c:idx val=\"0\"/><c:order val=\"0\"/><c:tx><c:v>P</c:v></c:tx><c:spPr><a:ln w=\"19050\"><a:noFill/></a:ln></c:spPr>"
                                              "<c:marker><c:symbol val=\"circle\"/><c:size val=\"9\"/><c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill><a:ln><a:noFill/></a:ln></c:spPr></c:marker>"
                                              "<c:xVal><c:numLit><c:ptCount val=\"3\"/><c:pt idx=\"0\"><c:v>1</c:v></c:pt><c:pt idx=\"1\"><c:v>2</c:v></c:pt><c:pt idx=\"2\"><c:v>4</c:v></c:pt></c:numLit></c:xVal>"
                                              "<c:yVal><c:numLit><c:ptCount val=\"3\"/><c:pt idx=\"0\"><c:v>1</c:v></c:pt><c:pt idx=\"1\"><c:v>4</c:v></c:pt><c:pt idx=\"2\"><c:v>9</c:v></c:pt></c:numLit></c:yVal></c:ser>")
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:scatterChart>")
                             + QStringLiteral("<c:valAx><c:axId val=\"111\"/><c:scaling><c:orientation val=\"minMax\"/><c:max val=\"5\"/><c:min val=\"0\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"b\"/><c:numFmt formatCode=\"General\" sourceLinked=\"1\"/>"
                                              "<c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"222\"/><c:crosses val=\"autoZero\"/><c:crossBetween val=\"midCat\"/></c:valAx>"
                                              "<c:valAx><c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/><c:max val=\"10\"/><c:min val=\"0\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"l\"/><c:majorGridlines/><c:numFmt formatCode=\"General\" sourceLinked=\"1\"/>"
                                              "<c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"111\"/><c:crosses val=\"autoZero\"/><c:crossBetween val=\"midCat\"/></c:valAx>");
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QImage img = renderChart(c, QSizeF(400, 300), 2);
        const QVector<QPointF> dots = colorComponents(img, Qt::red, 20);
        QCOMPARE(int(dots.size()), 3);   // markers alone, not joined
        // Placed by their values on both axes: x at 1, 2, 4 of 0 to 5, and y at 1, 4, 9 of 0 to 10.
        const double dx1 = dots[1].x() - dots[0].x(), dx2 = dots[2].x() - dots[1].x();
        const double dy1 = dots[0].y() - dots[1].y(), dy2 = dots[1].y() - dots[2].y();
        QVERIFY2(qAbs(dx2 / dx1 - 2) < 0.05, qPrintable(QString::number(dx2 / dx1)));
        QVERIFY2(qAbs(dy2 / dy1 - 5.0 / 3.0) < 0.05, qPrintable(QString::number(dy2 / dy1)));
        // The file's 0 to 10 is labeled in steps of 1, and goes no further.
        const QString svg = QString::fromUtf8(chartSvg(c, QSizeF(400, 300)));
        QVERIFY(svg.contains(QStringLiteral(">10<")));
        QVERIFY(!svg.contains(QStringLiteral(">12<")));
    }

    void chartSvgScalesTheAxisToTheValues()
    {
        // The largest value, 16.1, gets an axis to 18 in steps of 2, from 0.
        const QString plot = QStringLiteral("<c:areaChart><c:grouping val=\"standard\"/>") + chartSeries(0, QStringLiteral("S"), {"a", "b", "c"}, {0.2, 16.112566, 5.72})
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:areaChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QString svg = QString::fromUtf8(chartSvg(c, QSizeF(400, 300)));
        for (const char *label : {"0", "2", "4", "8", "16", "18"}) QVERIFY2(svg.contains(QStringLiteral(">%1<").arg(QLatin1String(label))), label);
        QVERIFY(!svg.contains(QStringLiteral(">20<")));
        QVERIFY(!svg.contains(QStringLiteral(">1<")));
        // Labels in the axis' number format.
        QString thousands = plot;
        thousands.replace(QStringLiteral("<c:numFmt formatCode=\"General\" sourceLinked=\"1\"/><c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"111\"/>"),
                        QStringLiteral("<c:numFmt formatCode=\"0.0&quot;k&quot;\" sourceLinked=\"0\"/><c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"111\"/>"));
        QVERIFY2(readChart(chartPart(thousands), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QString formatted = QString::fromUtf8(chartSvg(c, QSizeF(400, 300)));
        QVERIFY2(formatted.contains(QStringLiteral(">18.0k<")), qPrintable(formatted.left(200)));
    }

    void chartSvgPlacesTheLegend()
    {
        const QString red = QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></c:spPr>");
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("Only series"), {"a", "b"}, {1, 0}, red)
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        for (const QString &pos : {QStringLiteral("b"), QStringLiteral("r"), QStringLiteral("t"), QStringLiteral("l")}) {
            Chart c;
            QString err;
            QVERIFY2(readChart(chartPart(plot, QString(), QStringLiteral("<c:legend><c:legendPos val=\"%1\"/></c:legend>").arg(pos)), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
            QCOMPARE(c.legend.position, pos);
            const QImage img = renderChart(c, QSizeF(400, 300), 2);
            // The key is a small red square beside the series' name, at that side; the bar of value 0 is not drawn.
            const QVector<QPointF> bits = colorComponents(img, Qt::red, 4);
            QVERIFY2(!bits.isEmpty(), qPrintable(pos));
            QPointF key = bits.last();
            for (const QPointF &p : bits) {
                if (pos == QLatin1String("b") && p.y() > key.y()) key = p;
                if (pos == QLatin1String("t") && p.y() < key.y()) key = p;
                if (pos == QLatin1String("r") && p.x() > key.x()) key = p;
                if (pos == QLatin1String("l") && p.x() < key.x()) key = p;
            }
            if (pos == QLatin1String("b")) QVERIFY2(key.y() > img.height() * 0.85, qPrintable(QString::number(key.y())));
            if (pos == QLatin1String("t")) QVERIFY2(key.y() < img.height() * 0.15, qPrintable(QString::number(key.y())));
            // At a side, the key is one name's width in from the edge, and names are wider with some systems' fonts
            // (Windows: 68% across); a legend at the top or bottom has it left of the middle.
            if (pos == QLatin1String("r")) QVERIFY2(key.x() > img.width() * 0.6, qPrintable(QString::number(key.x())));
            if (pos == QLatin1String("l")) QVERIFY2(key.x() < img.width() * 0.4, qPrintable(QString::number(key.x())));
            QVERIFY(QString::fromUtf8(chartSvg(c, QSizeF(400, 300))).contains(QStringLiteral(">Only series<")));
        }
    }

    // A number format code is at most 255 characters (the spreadsheet limit); a longer one is not applied, and the numbers show as they are.
    void chartIgnoresFormatCodesPastTheSpreadsheetLimit()
    {
        const QString fits = QStringLiteral("0.0") + QString(252, QLatin1Char('x')), tooLong = fits + QLatin1Char('x');
        QCOMPARE(fits.size(), 255);
        QCOMPARE(formatChartNumber(1234.5, fits), QStringLiteral("1234.5") + QString(252, QLatin1Char('x')));
        QCOMPARE(formatChartNumber(1234.5, tooLong), QStringLiteral("1234.5"));
        // The same through a chart: 100 categories that are numbers, with a code of 20,000 characters, read in no time.
        QString points;
        for (int i = 0; i < 100; ++i) points += QStringLiteral("<c:pt idx=\"%1\"><c:v>%1.5</c:v></c:pt>").arg(i);
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/><c:ser><c:idx val=\"0\"/><c:order val=\"0\"/>"
                                            "<c:cat><c:numRef><c:f>Sheet1!$A$2:$A$101</c:f><c:numCache><c:formatCode>0.0%1</c:formatCode><c:ptCount val=\"100\"/>%2</c:numCache></c:numRef></c:cat>"
                                            "<c:val><c:numRef><c:f>Sheet1!$B$2:$B$101</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val=\"100\"/>%2</c:numCache></c:numRef></c:val></c:ser>"
                                            "<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>")
                                 .arg(QString(20000, QLatin1Char('x')), points) + chartAxes();
        Chart c;
        QString err;
        QElapsedTimer timer;
        timer.start();
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QVERIFY2(timer.elapsed() < 500, qPrintable(QString::number(timer.elapsed())));
        QCOMPARE(c.plots[0].series[0].categories[3], QStringLiteral("3.5"));
    }

    // The text a chart keeps for a label is cut at a few hundred characters, and what can't be written in a picture is left out of it.
    void chartKeepsLabelsShortAndWritable()
    {
        const QString huge(100000, QLatin1Char('m'));
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>")
                             + chartSeries(0, huge, {huge, QStringLiteral("b")}, {1, 2}) + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot, QString(), QStringLiteral("<c:legend><c:legendPos val=\"r\"/></c:legend>")), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QVERIFY(c.plots[0].series[0].categories[0].size() <= kChartMaxLabel);
        QVERIFY(c.plots[0].series[0].name.size() <= kChartMaxLabel);
        QVERIFY2(chartSvg(c, QSizeF(400, 300)).size() < 100000, "the picture repeats the long text");
        // Text with characters XML has no place for (control characters, half a surrogate pair) still makes a picture a reader can parse.
        c.plots[0].series[0].categories[1] = QStringLiteral("a") + QChar(1) + QStringLiteral("b") + QChar(7) + QStringLiteral("c") + QChar(0xD800) + QChar(0xFFFE) + QStringLiteral("<&\">'");
        c.plots[0].series[0].name = QStringLiteral("n") + QChar(2) + QChar(0xDC00);
        const QByteArray svg = chartSvg(c, QSizeF(400, 300));
        QXmlStreamReader xml(svg);
        while (!xml.atEnd()) xml.readNext();
        QVERIFY2(!xml.hasError(), qPrintable(xml.errorString()));
        QVERIFY(QSvgRenderer(svg).isValid());
    }

    // However much data a chart holds, its picture is a few thousand tags at most: the time QSvgRenderer takes to read a picture grows with the
    // square of them, and 20,000 columns took minutes.
    void chartDrawsHugeDataInFewPieces()
    {
        const int n = 6000;
        QStringList names;
        QVector<double> values, blanks;
        for (int i = 0; i < n; ++i) {
            names << QStringLiteral("c%1").arg(i);
            values << 10 + (i * 7) % 50;
            blanks << (i % 2 ? values.last() : std::nan(""));
        }
        const QString labels = QStringLiteral("<c:dLbls><c:showLegendKey val=\"0\"/><c:showVal val=\"1\"/><c:showCatName val=\"1\"/><c:showSerName val=\"0\"/><c:showPercent val=\"0\"/></c:dLbls>");
        const QString bar = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>%1<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>");
        const QString line = QStringLiteral("<c:lineChart><c:grouping val=\"standard\"/>%1<c:marker val=\"1\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:lineChart>");
        const QString area = QStringLiteral("<c:areaChart><c:grouping val=\"standard\"/>%1<c:axId val=\"111\"/><c:axId val=\"222\"/></c:areaChart>");
        const QString pie = QStringLiteral("<c:pieChart><c:varyColors val=\"1\"/>%1</c:pieChart>");
        const QString legend = QStringLiteral("<c:legend><c:legendPos val=\"r\"/></c:legend>");
        QString xs, ys;
        for (int i = 0; i < n; ++i) {
            xs += QStringLiteral("<c:pt idx=\"%1\"><c:v>%2</c:v></c:pt>").arg(i).arg((i * 37) % 1000);
            ys += QStringLiteral("<c:pt idx=\"%1\"><c:v>%2</c:v></c:pt>").arg(i).arg(values[i]);
        }
        const QString scatter = QStringLiteral("<c:scatterChart><c:scatterStyle val=\"lineMarker\"/><c:ser><c:idx val=\"0\"/><c:order val=\"0\"/><c:xVal><c:numRef><c:f>S!$A$1</c:f><c:numCache><c:formatCode>General</c:formatCode>"
                                               "<c:ptCount val=\"%1\"/>%2</c:numCache></c:numRef></c:xVal><c:yVal><c:numRef><c:f>S!$B$1</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val=\"%1\"/>%3</c:numCache></c:numRef></c:yVal></c:ser>"
                                               "<c:axId val=\"111\"/><c:axId val=\"222\"/></c:scatterChart>").arg(n).arg(xs, ys);
        QString everyLabel = chartAxes(), tinyUnit = chartAxes();
        everyLabel.replace(QStringLiteral("<c:crossAx val=\"222\"/>"), QStringLiteral("<c:crossAx val=\"222\"/><c:tickLblSkip val=\"1\"/>"));
        tinyUnit.replace(QStringLiteral("<c:crossBetween val=\"between\"/>"), QStringLiteral("<c:crossBetween val=\"between\"/><c:majorUnit val=\"0.001\"/>"));
        struct Case {
            const char *name;
            QString plot, legend;
        };
        const Case cases[] = {
            {"columns", bar.arg(chartSeries(0, QStringLiteral("S"), names, values)) + chartAxes(), QString()},
            {"columns with a label on each", bar.arg(chartSeries(0, QStringLiteral("S"), names, values, QString(), labels)) + chartAxes(), legend},
            {"columns with every category named", bar.arg(chartSeries(0, QStringLiteral("S"), names, values)) + everyLabel, QString()},
            {"columns on a scale of tiny steps", bar.arg(chartSeries(0, QStringLiteral("S"), names, values)) + tinyUnit, QString()},
            {"many series of columns", bar.arg(chartSeries(0, QStringLiteral("A"), names.mid(0, 2000), values.mid(0, 2000)) + chartSeries(1, QStringLiteral("B"), names.mid(0, 2000), values.mid(0, 2000))
                                               + chartSeries(2, QStringLiteral("C"), names.mid(0, 2000), values.mid(0, 2000))) + chartAxes(), legend},
            {"line with markers", line.arg(chartSeries(0, QStringLiteral("S"), names, values)) + chartAxes(), legend},
            {"line with a label on each", line.arg(chartSeries(0, QStringLiteral("S"), names, values, QString(), labels)) + chartAxes(), QString()},
            {"area broken at blanks", area.arg(chartSeries(0, QStringLiteral("S"), names, blanks)) + chartAxes(), QString()},
            {"scatter", scatter + chartAxes(), legend},
            {"pie", pie.arg(chartSeries(0, QStringLiteral("S"), names, values)), legend},
            {"pie with a label on each", pie.arg(chartSeries(0, QStringLiteral("S"), names, values, QString(), labels)), QString()},
        };
        for (const Case &k : cases) {
            Chart c;
            QString err;
            QVERIFY2(readChart(chartPart(k.plot, QString(), k.legend), defaultTheme(), ColorMap(), &c, &err), k.name);
            QElapsedTimer timer;
            timer.start();
            const QByteArray svg = chartSvg(c, QSizeF(480, 320));
            const qint64 tags = svg.count('<');
            QVERIFY2(tags <= 8000, qPrintable(QStringLiteral("%1: %2 tags").arg(QLatin1String(k.name)).arg(tags)));
            QSvgRenderer r(svg);
            QVERIFY2(r.isValid(), k.name);
            // And drawn, as a picture is for the page it is on.
            QImage img(600, 400, QImage::Format_ARGB32_Premultiplied);
            img.fill(Qt::white);
            QPainter painter(&img);
            r.render(&painter, QRectF(QPointF(), QSizeF(img.size())));
            painter.end();
            QVERIFY2(timer.elapsed() < 20000, qPrintable(QStringLiteral("%1: %2 ms").arg(QLatin1String(k.name)).arg(timer.elapsed())));
            // It is still a picture of the chart: its axis, and (but for a pie) the plot's ink, are there.
            QVERIFY2(tags > 10, k.name);
        }
    }

    // A chart too big to draw says so, and one that is not does not.
    void chartSaysWhenItIsDrawnCoarser()
    {
        QStringList names;
        QVector<double> values;
        for (int i = 0; i < 12000; ++i) {
            names << QStringLiteral("c%1").arg(i);
            values << i % 90;
        }
        const QString line = QStringLiteral("<c:lineChart><c:grouping val=\"standard\"/>%1<c:marker val=\"0\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:lineChart>");
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(line.arg(chartSeries(0, QStringLiteral("S"), names, values)) + chartAxes()), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        bool coarser = false;
        const QByteArray big = chartSvg(c, QSizeF(480, 320), &coarser);
        QVERIFY(coarser);
        QVERIFY(big.size() < 2000000);
        QVERIFY(readChart(chartPart(line.arg(chartSeries(0, QStringLiteral("S"), names.mid(0, 200), values.mid(0, 200))) + chartAxes()), defaultTheme(), ColorMap(), &c, &err));
        coarser = true;
        chartSvg(c, QSizeF(480, 320), &coarser);
        QVERIFY(!coarser);
        // So is one that has points enough, but more labels on them than can be read.
        const QString labels = QStringLiteral("<c:dLbls><c:showLegendKey val=\"0\"/><c:showVal val=\"1\"/><c:showCatName val=\"0\"/><c:showSerName val=\"0\"/><c:showPercent val=\"0\"/></c:dLbls>");
        const QString bars = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>%1<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>");
        QVERIFY(readChart(chartPart(bars.arg(chartSeries(0, QStringLiteral("S"), names.mid(0, 800), values.mid(0, 800), QString(), labels)) + chartAxes()), defaultTheme(), ColorMap(), &c, &err));
        coarser = false;
        chartSvg(c, QSizeF(480, 320), &coarser);
        QVERIFY(coarser);
    }

    // A font name with the characters an SVG attribute can't hold doesn't make the whole chart vanish.
    void chartSurvivesAnyFontName()
    {
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("Only series"), {"a", "b"}, {1, 2})
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        const QString legend = QStringLiteral("<c:legend><c:legendPos val=\"r\"/></c:legend>");
        for (const QString &name : {QStringLiteral("Bad\"Name"), QStringLiteral("A<b>C"), QStringLiteral("R&amp;D"), QStringLiteral("It's"), QStringLiteral("x\"/><script>y"), QStringLiteral("a") + QChar(1) + QStringLiteral("b"), QString(5000, QLatin1Char('f'))}) {
            Chart c;
            QString err;
            QVERIFY2(readChart(chartPart(plot, QString(), legend), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
            // As the file names it (through a typeface attribute, which XML escapes), and as another reader might set it.
            c.font.family = name;
            c.fontFamily = name;
            const QByteArray svg = chartSvg(c, QSizeF(400, 300));
            QXmlStreamReader xml(svg);
            while (!xml.atEnd()) xml.readNext();
            QVERIFY2(!xml.hasError(), qPrintable(name.left(20) + QLatin1Char(' ') + xml.errorString()));
            QVERIFY2(svg.size() < 100000, "the font name is written once for every text");
            QVERIFY2(svg.contains(">Only series<"), qPrintable(name.left(20)));
            QVERIFY2(!svg.contains("<script"), qPrintable(name.left(20)));
            QVERIFY(QSvgRenderer(svg).isValid());
        }
    }

    // Numbers no picture can hold (a size or an axis of 1e300, values of 1e308) draw a picture and not coordinates that are not numbers.
    void chartSurvivesNumbersBeyondAPicture()
    {
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("S"), {"a", "b", "c"}, {1, 2, 3})
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>");
        QString huge = chartAxes();
        huge.replace(QStringLiteral("<c:valAx><c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/>"),
                     QStringLiteral("<c:valAx><c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/><c:max val=\"1e308\"/><c:min val=\"-1e308\"/>"));
        static const QRegularExpression notANumber(QStringLiteral("(?<![A-Za-z])(nan|-?inf)(?![A-Za-z])"));
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot + huge), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QVERIFY(c.axes.size() == 2 && !c.axes[1].autoMax);
        const QByteArray big = chartSvg(c, QSizeF(400, 300));
        QVERIFY2(!QString::fromUtf8(big).contains(notANumber), qPrintable(notANumber.match(QString::fromUtf8(big)).captured()));
        QVERIFY(QSvgRenderer(big).isValid());
        // Values past what an axis can be made of are blanks.
        for (const double v : {1e308, -1e308, 1e200}) {
            Chart d;
            QVERIFY2(readChart(chartPart(QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("S"), {"a", "b", "c"}, {1, v, -v})
                                         + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes()), defaultTheme(), ColorMap(), &d, &err), qPrintable(err));
            const QString svg = QString::fromUtf8(chartSvg(d, QSizeF(400, 300)));
            QVERIFY2(!svg.contains(notANumber), qPrintable(QString::number(v)));
            QVERIFY(QSvgRenderer(svg.toUtf8()).isValid());
        }
        // A size is at most a slide's longest side, and a size that is no number is a point.
        const QString huger = QString::fromUtf8(chartSvg(c, QSizeF(1e300, 1e300)));
        QVERIFY2(huger.contains(QStringLiteral("viewBox=\"0 0 4032 4032\"")), qPrintable(huger.left(300)));
        const QString none = QString::fromUtf8(chartSvg(c, QSizeF(std::nan(""), std::numeric_limits<double>::infinity())));
        QVERIFY2(!none.contains(notANumber), qPrintable(none.left(300)));
    }

    // Gridlines, the marks of axes, and markers drawn one by one when a chart has few, and as one path when it has many, and a line of many
    // points drawn as one path or in paths of some segments each, make the same picture.
    void chartMergedPiecesLookTheSame()
    {
        const QString dashed = QStringLiteral("<c:spPr><a:ln w=\"9525\"><a:solidFill><a:srgbClr val=\"808080\"/></a:solidFill><a:prstDash val=\"dash\"/></a:ln></c:spPr>");
        QStringList names;
        QVector<double> a, b, walk;
        double at = 50;
        for (int i = 0; i < 40; ++i) {
            names << QStringLiteral("cat %1").arg(i);
            a << 20 + (i * 13) % 37;
            b << 10 + (i * 29) % 41;
        }
        QStringList manyNames;
        for (int i = 0; i < 1500; ++i) {
            manyNames << QString::number(i);
            at += ((i * 7919) % 31) - 15;
            walk << at;
        }
        QString dashedAxes = chartAxes();
        dashedAxes.replace(QStringLiteral("<c:majorGridlines/>"), QStringLiteral("<c:majorGridlines>") + dashed + QStringLiteral("</c:majorGridlines>"));
        const QString red = QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill><a:ln w=\"12700\"><a:solidFill><a:srgbClr val=\"000000\"/></a:solidFill></a:ln></c:spPr>");
        const QString bars = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>%1<c:gapWidth val=\"60\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>");
        const QString line = QStringLiteral("<c:lineChart><c:grouping val=\"standard\"/>%1<c:marker val=\"0\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:lineChart>");
        struct Case {
            const char *name;
            QString plot;
            int separateLines, segments, separateMarkers;   // what makes the picture another way, against the usual
        };
        const QString dots = QStringLiteral("<c:marker><c:symbol val=\"circle\"/><c:size val=\"6\"/></c:marker>"), crosses = QStringLiteral("<c:marker><c:symbol val=\"x\"/><c:size val=\"6\"/></c:marker>");
        const QString withMarkers = QStringLiteral("<c:lineChart><c:grouping val=\"standard\"/>%1<c:marker val=\"1\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:lineChart>");
        const Case cases[] = {
            {"columns with gridlines and ticks", bars.arg(chartSeries(0, QStringLiteral("A"), names, a, red) + chartSeries(1, QStringLiteral("B"), names, b)) + chartAxes(), 0, 0, -1},
            {"columns with dashed gridlines", bars.arg(chartSeries(0, QStringLiteral("A"), names, a)) + dashedAxes, 0, 0, -1},
            {"a line of 1,500 points", line.arg(chartSeries(0, QStringLiteral("A"), manyNames, walk)) + chartAxes(), -1, 20, -1},
            {"a line of 1,500 points, one path", line.arg(chartSeries(0, QStringLiteral("A"), manyNames, walk)) + chartAxes(), -1, 100000, -1},
            {"lines with markers", withMarkers.arg(chartSeries(0, QStringLiteral("A"), names, a, QString(), dots) + chartSeries(1, QStringLiteral("B"), names, b, QString(), crosses)) + chartAxes(), -1, 0, 0},
        };
        for (const Case &k : cases) {
            Chart c;
            QString err;
            QVERIFY2(readChart(chartPart(k.plot, QString(), QStringLiteral("<c:legend><c:legendPos val=\"b\"/></c:legend>")), defaultTheme(), ColorMap(), &c, &err), k.name);
            const QSizeF size(500, 300);
            auto draw = [&](const QByteArray &svg) {
                QSvgRenderer r(svg);
                QImage img(1000, 600, QImage::Format_ARGB32);
                img.fill(Qt::white);
                QPainter p(&img);
                r.render(&p, QRectF(QPointF(), QSizeF(img.size())));
                return img;
            };
            const QByteArray usual = chartSvg(c, size), other = chartSvgTuned(c, size, k.separateLines, k.segments, k.separateMarkers);
            QVERIFY2(usual != other, qPrintable(QStringLiteral("%1 is drawn the same way both times").arg(QLatin1String(k.name))));
            const QImage one = draw(usual), two = draw(other);
            // Where two strokes meet the edges come out a shade apart (as dark as the strokes' own where they overlap); nothing else may: what
            // differs is a small part of the ink of the picture.
            double differs = 0, ink = 0;
            for (int y = 0; y < one.height(); ++y)
                for (int x = 0; x < one.width(); ++x) {
                    ink += 255 - qGray(one.pixel(x, y));
                    differs += qAbs(qGray(one.pixel(x, y)) - qGray(two.pixel(x, y)));
                }
            QVERIFY2(ink > 20000, qPrintable(QStringLiteral("%1 draws no ink").arg(QLatin1String(k.name))));
            QVERIFY2(differs <= 0.02 * ink, qPrintable(QStringLiteral("%1: %2 of %3").arg(QLatin1String(k.name)).arg(differs).arg(ink)));
        }
    }

    // A line that crosses its plot back and forth thousands of times takes seconds to fill; drawn from fewer of its points, it takes a fraction of one.
    void chartDrawsLinesThatCrossTheirPlotQuickly()
    {
        QString xs, ys;
        quint32 state = 12345;
        auto next = [&] { state = state * 1664525u + 1013904223u; return (state >> 8) % 1000; };
        for (int i = 0; i < 2000; ++i) {
            xs += QStringLiteral("<c:pt idx=\"%1\"><c:v>%2</c:v></c:pt>").arg(i).arg(next());
            ys += QStringLiteral("<c:pt idx=\"%1\"><c:v>%2</c:v></c:pt>").arg(i).arg(next());
        }
        const QString scatter = QStringLiteral("<c:scatterChart><c:scatterStyle val=\"lineMarker\"/><c:ser><c:idx val=\"0\"/><c:order val=\"0\"/><c:spPr><a:ln w=\"28575\"><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></a:ln></c:spPr>"
                                               "<c:marker><c:symbol val=\"none\"/></c:marker><c:xVal><c:numRef><c:f>S!$A$1</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val=\"2000\"/>%1</c:numCache></c:numRef></c:xVal>"
                                               "<c:yVal><c:numRef><c:f>S!$B$1</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val=\"2000\"/>%2</c:numCache></c:numRef></c:yVal></c:ser>"
                                               "<c:axId val=\"111\"/><c:axId val=\"222\"/></c:scatterChart>").arg(xs, ys);
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(scatter + chartAxes()), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QElapsedTimer timer;
        timer.start();
        const QImage img = renderChart(c, QSizeF(500, 333), 2);
        QVERIFY2(timer.elapsed() < 20000, qPrintable(QString::number(timer.elapsed())));
        // It is still a picture of the line: the plot has plenty of red in it.
        QVERIFY(colorArea(img, Qt::red) > 2000);
    }

private:
    // A chart part: `body` (the plot area's kinds and axes) in a c:chart, after an optional title, with an optional legend.
    static QByteArray chartPart(const QString &body, const QString &title = QString(), const QString &legend = QString(), const QString &chartSpaceExtra = QString())
    {
        return (QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<c:chartSpace xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\" "
                               "xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
                               "<c:date1904 val=\"0\"/><c:roundedCorners val=\"0\"/><c:chart>")
                + title + (title.isEmpty() ? QStringLiteral("<c:autoTitleDeleted val=\"1\"/>") : QString())
                + QStringLiteral("<c:plotArea><c:layout/>") + body + QStringLiteral("</c:plotArea>") + legend
                + QStringLiteral("<c:plotVisOnly val=\"1\"/><c:dispBlanksAs val=\"gap\"/></c:chart>") + chartSpaceExtra + QStringLiteral("</c:chartSpace>")).toUtf8();
    }
    static QString chartAxes()
    {
        return QStringLiteral("<c:catAx><c:axId val=\"111\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"b\"/><c:numFmt formatCode=\"General\" sourceLinked=\"1\"/>"
                              "<c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"222\"/><c:crosses val=\"autoZero\"/><c:auto val=\"1\"/><c:lblAlgn val=\"ctr\"/>"
                              "<c:lblOffset val=\"100\"/><c:noMultiLvlLbl val=\"0\"/></c:catAx>"
                              "<c:valAx><c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"l\"/><c:majorGridlines/><c:numFmt formatCode=\"General\" sourceLinked=\"1\"/>"
                              "<c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"111\"/><c:crosses val=\"autoZero\"/><c:crossBetween val=\"between\"/></c:valAx>");
    }
    // A series: its name, categories, and values as the file caches them (a NaN value is a point left out), then what the schema puts
    // after the color (markers, points, labels) and after the values.
    static QString chartSeries(int idx, const QString &name, const QStringList &categories, const QVector<double> &values, const QString &spPr = QString(), const QString &beforeData = QString(),
                               const QString &after = QString())
    {
        QString cats, vals;
        for (int i = 0; i < categories.size(); ++i) cats += QStringLiteral("<c:pt idx=\"%1\"><c:v>%2</c:v></c:pt>").arg(i).arg(categories[i]);
        for (int i = 0; i < values.size(); ++i)
            if (!std::isnan(values[i])) vals += QStringLiteral("<c:pt idx=\"%1\"><c:v>%2</c:v></c:pt>").arg(i).arg(values[i]);
        return QStringLiteral("<c:ser><c:idx val=\"%1\"/><c:order val=\"%1\"/><c:tx><c:strRef><c:f>Sheet1!$%2$1</c:f><c:strCache><c:ptCount val=\"1\"/><c:pt idx=\"0\"><c:v>%3</c:v></c:pt></c:strCache></c:strRef></c:tx>")
                   .arg(idx).arg(QChar('B' + idx % 20)).arg(name)
               + spPr + beforeData
               + QStringLiteral("<c:cat><c:strRef><c:f>Sheet1!$A$2:$A$%1</c:f><c:strCache><c:ptCount val=\"%2\"/>%3</c:strCache></c:strRef></c:cat>").arg(categories.size() + 1).arg(categories.size()).arg(cats)
               + QStringLiteral("<c:val><c:numRef><c:f>Sheet1!$B$2:$B$%1</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val=\"%2\"/>%3</c:numCache></c:numRef></c:val>")
                     .arg(values.size() + 1).arg(values.size()).arg(vals)
               + after + QStringLiteral("</c:ser>");
    }
    // The chart drawn into a white picture, `scale` pixels to the point.
    static QImage renderChart(const Chart &chart, const QSizeF &size, int scale)
    {
        QSvgRenderer r(chartSvg(chart, size));
        QImage img(int(size.width() * scale), int(size.height() * scale), QImage::Format_ARGB32);
        img.fill(Qt::white);
        QPainter p(&img);
        r.render(&p, QRectF(QPointF(), QSizeF(img.size())));
        return img;
    }
    static bool isColor(QRgb p, const QColor &c, int tolerance)
    {
        return qAbs(qRed(p) - c.red()) <= tolerance && qAbs(qGreen(p) - c.green()) <= tolerance && qAbs(qBlue(p) - c.blue()) <= tolerance;
    }
    // The boxes of the separate patches of one color at least four pixels across and high (the bars of a chart), in order from the left, or from the top.
    static QVector<QRect> colorRuns(const QImage &img, const QColor &color, bool fromTop)
    {
        QVector<QRect> out;
        QVector<char> seen(img.width() * img.height(), 0);
        for (int y0 = 0; y0 < img.height(); ++y0)
            for (int x0 = 0; x0 < img.width(); ++x0) {
                if (seen[y0 * img.width() + x0] || !isColor(img.pixel(x0, y0), color, 40)) continue;
                QRect box(x0, y0, 1, 1);
                QVector<QPoint> stack{QPoint(x0, y0)};
                seen[y0 * img.width() + x0] = 1;
                while (!stack.isEmpty()) {
                    const QPoint p = stack.takeLast();
                    box = box.united(QRect(p, QSize(1, 1)));
                    for (const QPoint d : {QPoint(1, 0), QPoint(-1, 0), QPoint(0, 1), QPoint(0, -1)}) {
                        const QPoint q = p + d;
                        if (q.x() < 0 || q.y() < 0 || q.x() >= img.width() || q.y() >= img.height() || seen[q.y() * img.width() + q.x()] || !isColor(img.pixel(q), color, 40)) continue;
                        seen[q.y() * img.width() + q.x()] = 1;
                        stack << q;
                    }
                }
                if (box.width() >= 4 && box.height() >= 4) out << box;
            }
        std::sort(out.begin(), out.end(), [fromTop](const QRect &a, const QRect &b) { return fromTop ? a.top() < b.top() : a.left() < b.left(); });
        return out;
    }
    // The box of the pixels of one color; of everything that isn't white or nearly, for an invalid color.
    static QRect colorBounds(const QImage &img, const QColor &color, bool anyInk)
    {
        QRect box;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x) {
                const QRgb p = img.pixel(x, y);
                const bool hit = anyInk || !color.isValid() ? (qRed(p) < 200 || qGreen(p) < 200 || qBlue(p) < 200) : isColor(p, color, 40);
                if (!hit) continue;
                box = box.isNull() ? QRect(x, y, 1, 1) : box.united(QRect(x, y, 1, 1));
            }
        return box;
    }
    static double colorArea(const QImage &img, const QColor &color)
    {
        double n = 0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x)
                if (isColor(img.pixel(x, y), color, 40)) ++n;
        return n;
    }
    static QPointF colorCentroid(const QImage &img, const QColor &color)
    {
        double n = 0, sx = 0, sy = 0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x)
                if (isColor(img.pixel(x, y), color, 40)) {
                    ++n;
                    sx += x;
                    sy += y;
                }
        return n > 0 ? QPointF(sx / n, sy / n) : QPointF();
    }
    // The centers of the separate patches of one color with at least `minPixels` pixels, from the left.
    static QVector<QPointF> colorComponents(const QImage &img, const QColor &color, int minPixels)
    {
        QVector<QPointF> out;
        QVector<char> seen(img.width() * img.height(), 0);
        for (int y0 = 0; y0 < img.height(); ++y0)
            for (int x0 = 0; x0 < img.width(); ++x0) {
                if (seen[y0 * img.width() + x0] || !isColor(img.pixel(x0, y0), color, 40)) continue;
                double n = 0, sx = 0, sy = 0;
                QVector<QPoint> stack{QPoint(x0, y0)};
                seen[y0 * img.width() + x0] = 1;
                while (!stack.isEmpty()) {
                    const QPoint p = stack.takeLast();
                    ++n;
                    sx += p.x();
                    sy += p.y();
                    for (const QPoint d : {QPoint(1, 0), QPoint(-1, 0), QPoint(0, 1), QPoint(0, -1)}) {
                        const QPoint q = p + d;
                        if (q.x() < 0 || q.y() < 0 || q.x() >= img.width() || q.y() >= img.height() || seen[q.y() * img.width() + q.x()] || !isColor(img.pixel(q), color, 40)) continue;
                        seen[q.y() * img.width() + q.x()] = 1;
                        stack << q;
                    }
                }
                if (n >= minPixels) out << QPointF(sx / n, sy / n);
            }
        std::sort(out.begin(), out.end(), [](const QPointF &a, const QPointF &b) { return a.x() < b.x(); });
        return out;
    }

private Q_SLOTS:
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

private:
    // ---- binary drawings ([MS-ODRAW]): every record is built by hand here

    static QByteArray odU16(quint32 v)
    {
        QByteArray b(2, 0);
        qToLittleEndian<quint16>(quint16(v), b.data());
        return b;
    }
    static QByteArray odU32(quint32 v)
    {
        QByteArray b(4, 0);
        qToLittleEndian<quint32>(v, b.data());
        return b;
    }
    static QByteArray odRecord(quint16 version, quint16 instance, quint16 type, const QByteArray &data)
    {
        return odU16(quint32(instance << 4) | version) + odU16(type) + odU32(quint32(data.size())) + data;
    }
    static QByteArray odContainer(quint16 type, const QByteArray &children, quint16 instance = 0)
    {
        return odRecord(0xF, instance, type, children);
    }
    static QByteArray odUid(char fill) { return QByteArray(16, fill); }
    // A bitmap picture record: its one or two uids, the tag byte, the file.
    static QByteArray odBitmapBlip(quint16 type, quint16 instance, const QByteArray &file)
    {
        const int uids = (instance & 1) ? 2 : 1;
        return odRecord(0, instance, type, odUid('u') + (uids == 2 ? odUid('v') : QByteArray()) + QByteArray(1, '\xFF') + file);
    }
    // A metafile picture record: uids, the 34-byte header (size, bounds, size in
    // EMU, saved size, compression, filter), then the data deflated or stored.
    static QByteArray odMetafileBlip(quint16 type, quint16 instance, const QByteArray &file, bool deflate, const QRect &bounds = QRect(0, 0, 200, 100),
                                     const QSize &emu = QSize(2540000, 1270000))
    {
        const int uids = (instance & 1) ? 2 : 1;
        const QByteArray saved = deflate ? qCompress(file, 9).mid(4) : file;
        QByteArray head = odU32(quint32(file.size()));
        for (int v : {bounds.left(), bounds.top(), bounds.left() + bounds.width(), bounds.top() + bounds.height(), emu.width(), emu.height()}) head += odU32(quint32(v));
        head += odU32(quint32(saved.size())) + QByteArray(1, deflate ? '\x00' : '\xFE') + QByteArray(1, '\xFE');
        return odRecord(0, instance, type, odUid('m') + (uids == 2 ? odUid('n') : QByteArray()) + head + saved);
    }
    static QByteArray odFakeEmf(int size = 120)
    {
        QByteArray e(size, '\0');
        qToLittleEndian<quint32>(1, e.data());
        qToLittleEndian<quint32>(88, e.data() + 4);
        qToLittleEndian<quint32>(0x464D4520u, e.data() + 40);
        for (int i = 88; i < size; ++i) e[i] = char(i * 7);
        return e;
    }
    static QByteArray odFakeWmf()
    {
        QByteArray w;
        w += odU16(1) + odU16(9) + odU16(0x300) + odU32(60) + odU16(1) + odU32(20) + odU16(0);
        w += odU32(3) + odU16(0) + odU32(3) + odU16(0);
        return w + QByteArray(40, '\x01');
    }
    // A DIB: the info header, a palette of `colors` entries, then rows.
    static QByteArray odDib(int w, int h, int bits, int colors, const QByteArray &rows)
    {
        QByteArray d = odU32(40) + odU32(quint32(w)) + odU32(quint32(h)) + odU16(1) + odU16(bits) + odU32(0) + odU32(quint32(rows.size())) +
                       odU32(2835) + odU32(2835) + odU32(quint32(colors)) + odU32(0);
        for (int i = 0; i < colors; ++i) d += QByteArray("\x10\x20\x30\x00", 4);
        return d + rows;
    }
    // A picture store entry: the file block record that holds or points to a picture.
    static QByteArray odFbse(quint16 blipType, quint32 size, quint32 refs, quint32 delayOffset, const QByteArray &name = QByteArray(), const QByteArray &blip = QByteArray())
    {
        QByteArray d;
        d += QByteArray(1, char(blipType)) + QByteArray(1, '\x00') + odUid('s') + odU16(0xFF) + odU32(size) + odU32(refs) + odU32(delayOffset);
        d += QByteArray(1, '\0') + QByteArray(1, char(name.size())) + QByteArray(2, '\0') + name + blip;
        return odRecord(2, blipType, 0xF007, d);
    }

private Q_SLOTS:
    void odrawBlipReadsBitmapPictures()
    {
        const QByteArray png = QByteArray("\x89PNG\r\n\x1a\n", 8) + QByteArray(50, 'p');
        QString format;
        // One uid (0x6E0) or two (0x6E1) before the tag byte.
        QCOMPARE(odrawBlip(odBitmapBlip(0xF01E, 0x6E0, png), &format), png);
        QCOMPARE(format, QStringLiteral("png"));
        QCOMPARE(odrawBlip(odBitmapBlip(0xF01E, 0x6E1, png), &format), png);
        QCOMPARE(format, QStringLiteral("png"));
        const QByteArray jpeg = QByteArray("\xFF\xD8\xFF\xE0", 4) + QByteArray(30, 'j');
        for (quint16 inst : {0x46A, 0x46B, 0x6E2, 0x6E3}) {
            QCOMPARE(odrawBlip(odBitmapBlip(0xF01D, inst, jpeg), &format), jpeg);
            QCOMPARE(format, QStringLiteral("jpeg"));
        }
        QCOMPARE(odrawBlip(odBitmapBlip(0xF02A, 0x6E2, jpeg), &format), jpeg);
        QCOMPARE(format, QStringLiteral("jpeg"));
        const QByteArray tiff = QByteArray("II*\0", 4) + QByteArray(30, 't');
        QCOMPARE(odrawBlip(odBitmapBlip(0xF029, 0x6E4, tiff), &format), tiff);
        QCOMPARE(format, QStringLiteral("tiff"));
        QCOMPARE(odrawBlip(odBitmapBlip(0xF029, 0x6E5, tiff), &format), tiff);
        // The format pointer is optional, and the record may be followed by more.
        QCOMPARE(odrawBlip(odBitmapBlip(0xF01E, 0x6E0, png) + QByteArray(9, 'x'), nullptr), png);
    }

    void odrawBlipInflatesMetafiles()
    {
        const QByteArray emf = odFakeEmf();
        QString format;
        QCOMPARE(odrawBlip(odMetafileBlip(0xF01A, 0x3D4, emf, true), &format), emf);
        QCOMPARE(format, QStringLiteral("emf"));
        QCOMPARE(odrawBlip(odMetafileBlip(0xF01A, 0x3D5, emf, true), &format), emf);   // two uids
        QCOMPARE(format, QStringLiteral("emf"));
        QCOMPARE(odrawBlip(odMetafileBlip(0xF01A, 0x3D4, emf, false), &format), emf);  // stored, compression 0xFE
        QCOMPARE(format, QStringLiteral("emf"));
        // Inflating agrees with zlib at every level, for data of every kind.
        QByteArray noise(70000, '\0');
        QRandomGenerator g(7);
        for (char &c : noise) c = char(g.generate());
        QByteArray text;
        for (int i = 0; i < 4000; ++i) text += QByteArray("line ") + QByteArray::number(i % 37) + " of a metafile\n";
        const QByteArray runs = QByteArray(300000, 'a') + QByteArray(5, 'b') + QByteArray(100000, 'c');
        for (const QByteArray &payload : {QByteArray(), QByteArray("x"), noise, text, runs}) {
            QByteArray file = odFakeEmf() + payload;
            for (int level : {0, 1, 6, 9}) {
                const QByteArray saved = qCompress(file, level).mid(4);
                QByteArray head = odU32(quint32(file.size())) + QByteArray(16, '\0') + QByteArray(8, '\0') + odU32(quint32(saved.size())) + QByteArray("\x00\xFE", 2);
                const QByteArray rec = odRecord(0, 0x3D4, 0xF01A, odUid('m') + head + saved);
                QCOMPARE(odrawBlip(rec, &format), file);
            }
        }
    }

    void odrawBlipRebuildsAPlaceableHeader()
    {
        // A WMF in a picture record has no placeable header; one is made from
        // the bounds and the size in EMU (2 x 1 inch here, in 100 units per inch).
        QString format;
        const QByteArray wmf = odFakeWmf();
        const QByteArray out = odrawBlip(odMetafileBlip(0xF01B, 0x216, wmf, true, QRect(0, 0, 200, 100), QSize(1828800, 914400)), &format);
        QCOMPARE(format, QStringLiteral("wmf"));
        QCOMPARE(out.size(), wmf.size() + 22);
        QCOMPARE(out.mid(22), wmf);
        QCOMPARE(qFromLittleEndian<quint32>(out.constData()), 0x9AC6CDD7u);
        QCOMPARE(qFromLittleEndian<qint16>(out.constData() + 6), qint16(0));
        QCOMPARE(qFromLittleEndian<qint16>(out.constData() + 10), qint16(200));
        QCOMPARE(qFromLittleEndian<qint16>(out.constData() + 12), qint16(100));
        QCOMPARE(qFromLittleEndian<quint16>(out.constData() + 14), quint16(100));
        quint16 sum = 0;
        for (int i = 0; i < 10; ++i) sum ^= qFromLittleEndian<quint16>(out.constData() + i * 2);
        QCOMPARE(qFromLittleEndian<quint16>(out.constData() + 20), sum);
        // The player reads it with that size: 2 x 1 inch, in points.
        jp::Metafile mf;
        QVERIFY(mf.load(out));
        QVERIFY(!mf.isEmf());
        QVERIFY(qAbs(mf.naturalSize().width() - 144) < 0.5 && qAbs(mf.naturalSize().height() - 72) < 0.5);
        // A metafile that has its header already is left as it is.
        QCOMPARE(odrawBlip(odMetafileBlip(0xF01B, 0x217, out, false), &format), out);
        QCOMPARE(format, QStringLiteral("wmf"));
        // Without bounds, the size in EMU gives them (hundredths of a millimeter).
        const QByteArray noBounds = odrawBlip(odMetafileBlip(0xF01B, 0x216, wmf, true, QRect(), QSize(3600000, 1800000)), &format);
        jp::Metafile mf2;
        QVERIFY(mf2.load(noBounds));
        QVERIFY(qAbs(mf2.naturalSize().width() - 283.46) < 1 && qAbs(mf2.naturalSize().height() - 141.73) < 1);
    }

    void odrawBlipMakesABitmapFileFromADib()
    {
        QString format;
        // 2 x 2, 24 bits: rows of 6 bytes padded to 8, bottom row first.
        const QByteArray rows = QByteArray("\x00\x00\xFF\x00\xFF\x00\x00\x00", 8) + QByteArray("\xFF\x00\x00\xFF\xFF\xFF\x00\x00", 8);
        QByteArray bmp = odrawBlip(odBitmapBlip(0xF01F, 0x7A8, odDib(2, 2, 24, 0, rows)), &format);
        QCOMPARE(format, QStringLiteral("bmp"));
        QCOMPARE(bmp.left(2), QByteArray("BM"));
        QCOMPARE(qFromLittleEndian<quint32>(bmp.constData() + 2), quint32(bmp.size()));
        QCOMPARE(qFromLittleEndian<quint32>(bmp.constData() + 10), quint32(54));
        QImage im = QImage::fromData(bmp);
        QCOMPARE(im.size(), QSize(2, 2));
        QCOMPARE(im.pixel(0, 1), qRgb(255, 0, 0));      // bottom-left, stored BGR 00 00 FF
        QCOMPARE(im.pixel(1, 1), qRgb(0, 255, 0));
        QCOMPARE(im.pixel(0, 0), qRgb(0, 0, 255));
        QCOMPARE(im.pixel(1, 0), qRgb(255, 255, 255));
        // With a palette (and two uids): the pixels start after its entries.
        bmp = odrawBlip(odBitmapBlip(0xF01F, 0x7A9, odDib(8, 1, 1, 2, QByteArray("\xA5\0\0\0", 4))), &format);
        QCOMPARE(qFromLittleEndian<quint32>(bmp.constData() + 10), quint32(14 + 40 + 8));
        QCOMPARE(QImage::fromData(bmp).size(), QSize(8, 1));
        // An 8-bit one without a color count has 256 entries.
        bmp = odrawBlip(odBitmapBlip(0xF01F, 0x7A8, odDib(1, 1, 8, 0, QByteArray(4, '\0'))), &format);
        QVERIFY(bmp.isEmpty());   // no room for the 256 entries
        QVERIFY(format.isEmpty());
    }

    void odrawBlipRefusesDamagedRecords()
    {
        const QByteArray png = QByteArray(40, 'p');
        QString format = QStringLiteral("x");
        QVERIFY(odrawBlip(QByteArray(), &format).isEmpty());
        QVERIFY(format.isEmpty());
        const QByteArray good = odBitmapBlip(0xF01E, 0x6E0, png);
        for (int cut : {0, 3, 7, 8, 20, 24, 25, 40, int(good.size()) - 1}) {
            format = QStringLiteral("x");
            QVERIFY2(odrawBlip(good.left(cut), &format).isEmpty(), qPrintable(QString::number(cut)));
            QVERIFY(format.isEmpty());
        }
        // A length that runs past the data; an unknown record type; a picture with no data.
        QByteArray past = good;
        qToLittleEndian<quint32>(0x7FFFFFF0u, past.data() + 4);
        QVERIFY(odrawBlip(past, &format).isEmpty());
        QVERIFY(odrawBlip(odRecord(0, 0x6E0, 0xF020, odUid('u') + QByteArray(1, 'x') + png), &format).isEmpty());
        QVERIFY(odrawBlip(odRecord(0, 0x6E0, 0xF01E, odUid('u') + QByteArray(1, '\xFF')), &format).isEmpty());
        QVERIFY(odrawBlip(odRecord(0, 0x6E1, 0xF01E, odUid('u') + QByteArray(5, 'x')), &format).isEmpty());   // two uids need 32 bytes
        // A Macintosh picture can't be shown.
        QVERIFY(odrawBlip(odMetafileBlip(0xF01C, 0x542, QByteArray(100, 'q'), true), &format).isEmpty());
        QVERIFY(format.isEmpty());
        // Deflated data that is not a deflate stream, cut short, or too big to keep.
        const QByteArray emf = odFakeEmf();
        QByteArray rec = odMetafileBlip(0xF01A, 0x3D4, emf, true);
        QByteArray junk = rec;
        for (int i = 8 + 16 + 34; i < junk.size(); ++i) junk[i] = char(0x5A + i);
        junk[8 + 16 + 34] = '\x78';
        QVERIFY(odrawBlip(junk, &format).isEmpty());
        QVERIFY(odrawBlip(rec.left(rec.size() - 6), &format).isEmpty());
        QByteArray huge = rec;
        qToLittleEndian<quint32>(0x20000000u, huge.data() + 8 + 16);   // cbSize of 512 MB
        QVERIFY(odrawBlip(huge, &format).isEmpty());
        QVERIFY(format.isEmpty());
        // A small stream that inflates to far more than the size it gives is damaged.
        const QByteArray bomb = qCompress(QByteArray(60 * 1024 * 1024, '\0'), 9).mid(4);
        const QByteArray head = odU32(100) + QByteArray(24, '\0') + odU32(quint32(bomb.size())) + QByteArray("\x00\xFE", 2);
        QVERIFY(odrawBlip(odRecord(0, 0x3D4, 0xF01A, odUid('m') + head + bomb), &format).isEmpty());
        // Bytes of every kind never crash it.
        QRandomGenerator g(11);
        for (int round = 0; round < 300; ++round) {
            QByteArray b = round % 3 == 0 ? rec : round % 3 == 1 ? good : odBitmapBlip(0xF01F, 0x7A8, odDib(2, 2, 24, 0, QByteArray(16, 'r')));
            for (int k = 0; k < 4; ++k) b[int(g.bounded(quint32(b.size())))] = char(g.generate());
            odrawBlip(b.left(int(g.bounded(quint32(b.size() + 1)))), &format);
        }
    }

    void odrawStoreListsEmbeddedAndDelayedBlips()
    {
        const QByteArray png = QByteArray("\x89PNG", 4) + QByteArray(30, 'p');
        const QByteArray blip1 = odBitmapBlip(0xF01E, 0x6E0, png);
        const QByteArray emfBlip = odMetafileBlip(0xF01A, 0x3D4, odFakeEmf(), true);
        QByteArray children;
        children += odFbse(6, quint32(blip1.size()), 1, 0xFFFFFFFFu, QByteArray(), blip1);              // 1: inside the record
        children += odFbse(2, 12345, 2, 0x4000);                                                         // 2: at 0x4000 in the other stream
        children += odFbse(2, 999, 1, 0x80, QByteArray("n\0a\0", 4), emfBlip);                          // 3: with a name, and its picture
        children += odFbse(0, 0, 0, 0xFFFFFFFFu);                                                        // 4: an empty slot
        children += blip1;                                                                               // 5: a picture record by itself
        const QByteArray store = odContainer(0xF001, children, 5);
        const QVector<OdrawStoreEntry> e = odrawStore(store);
        QCOMPARE(e.size(), 5);
        for (int i = 0; i < 5; ++i) QCOMPARE(e[i].index, i + 1);
        QCOMPARE(e[0].embedded, blip1);
        QCOMPARE(e[1].embedded, QByteArray());
        QCOMPARE(e[1].delayOffset, quint32(0x4000));
        QCOMPARE(e[2].embedded, emfBlip);
        QCOMPARE(e[2].delayOffset, quint32(0x80));
        QVERIFY(e[3].embedded.isEmpty());
        QCOMPARE(e[3].delayOffset, 0xFFFFFFFFu);
        QCOMPARE(e[4].embedded, blip1);
        QString format;
        QCOMPARE(odrawBlip(e[0].embedded, &format), png);
        QCOMPARE(format, QStringLiteral("png"));
        QCOMPARE(odrawBlip(e[2].embedded, &format), odFakeEmf());
        // The container's own header may be left off: the file blocks alone.
        QCOMPARE(odrawStore(children).size(), 5);
        QVERIFY(odrawStore(QByteArray()).isEmpty());
        // Damage: cut anywhere, a name past the end, a lying count of entries, and bytes of every kind.
        for (int cut = 0; cut < store.size(); cut += 3) odrawStore(store.left(cut));
        QByteArray longName = odFbse(6, 10, 1, 0, QByteArray(), QByteArray());
        longName[8 + 33] = char(250);
        QVERIFY(odrawStore(odContainer(0xF001, longName)).size() == 1);
        QVERIFY(odrawStore(odContainer(0xF001, longName))[0].embedded.isEmpty());
        QRandomGenerator g(5);
        for (int round = 0; round < 300; ++round) {
            QByteArray b = store;
            for (int k = 0; k < 5; ++k) b[int(g.bounded(quint32(b.size())))] = char(g.generate());
            odrawStore(b);
        }
    }

private:
    // A property of a shape: its number, its value, the complex data after the
    // table (then the value is its length), and whether it is a picture index.
    struct OdProp {
        quint16 id;
        quint32 value = 0;
        QByteArray complex = QByteArray();
        bool blip = false;
        int lessBy = 0;   // the length is that much short of the data's (an array whose header isn't counted)
    };
    static QByteArray odProps(const QVector<OdProp> &props, quint16 type = 0xF00B)
    {
        QByteArray table, tail;
        for (const OdProp &p : props) {
            quint16 id = p.id;
            quint32 value = p.value;
            if (p.blip) id |= 0x4000;
            if (!p.complex.isEmpty()) {
                id |= 0x8000;
                value = quint32(p.complex.size() - p.lessBy);
                tail += p.complex;
            }
            table += odU16(id) + odU32(value);
        }
        return odRecord(3, quint16(props.size()), type, table + tail);
    }
    static QByteArray odFsp(int shapeType, quint32 flags, quint32 spid = 1025) { return odRecord(2, quint16(shapeType), 0xF00A, odU32(spid) + odU32(flags)); }
    // The host's anchor in these tests: left, top, right, bottom in points, 16 bits each.
    static QByteArray odClientAnchor(int l, int t, int r, int b) { return odRecord(0, 0, 0xF010, odU16(quint32(l)) + odU16(quint32(t)) + odU16(quint32(r)) + odU16(quint32(b))); }
    static QByteArray odChildAnchor(int l, int t, int r, int b)
    {
        return odRecord(0, 0, 0xF00F, odU32(quint32(l)) + odU32(quint32(t)) + odU32(quint32(r)) + odU32(quint32(b)));
    }
    // A shape container: its shape record, properties, anchor, and any client records.
    static QByteArray odShape(int type, quint32 flags, const QVector<OdProp> &props, const QByteArray &anchor = QByteArray(), const QByteArray &client = QByteArray())
    {
        return odContainer(0xF004, odFsp(type, flags) + (props.isEmpty() ? QByteArray() : odProps(props)) + anchor + client);
    }
    // A group: the shape that describes it (its child rectangle, flags, anchor), then its shapes.
    static QByteArray odGroup(const QRect &children, quint32 flags, const QByteArray &anchor, const QByteArray &shapes, const QVector<OdProp> &props = {})
    {
        const QByteArray rect = odU32(quint32(children.left())) + odU32(quint32(children.top())) + odU32(quint32(children.left() + children.width())) + odU32(quint32(children.top() + children.height()));
        return odContainer(0xF003, odContainer(0xF004, odRecord(1, 0, 0xF009, rect) + odFsp(0, 0x1 | flags) + (props.isEmpty() ? QByteArray() : odProps(props)) + anchor) + shapes);
    }
    // A drawing: the top group (its first shape is the patriarch), and maybe a background shape.
    static QByteArray odDrawing(const QByteArray &shapes, const QByteArray &background = QByteArray())
    {
        const QByteArray patriarch = odContainer(0xF004, odRecord(1, 0, 0xF009, QByteArray(16, '\0')) + odFsp(0, 0x5, 1024));
        return odContainer(0xF002, odRecord(0, 1, 0xF008, odU32(5) + odU32(1025)) + odContainer(0xF003, patriarch + shapes) + background);
    }
    static QByteArray odUtf16z(const QString &text)
    {
        QByteArray b;
        for (const QChar c : text) b += odU16(c.unicode());
        return b + odU16(0);
    }
    // An array property's data: the count twice, the element size, and the elements.
    static QByteArray odArray(int count, int elementSize, const QByteArray &elements)
    {
        return odU16(quint32(count)) + odU16(quint32(count)) + odU16(quint32(elementSize)) + elements;
    }
    static QByteArray odVertices(const QVector<QPoint> &points, int elementSize = 8)
    {
        QByteArray e;
        for (const QPoint &pt : points) e += elementSize == 8 ? odU32(quint32(pt.x())) + odU32(quint32(pt.y())) : odU16(quint32(pt.x())) + odU16(quint32(pt.y()));
        return odArray(int(points.size()), elementSize == 8 ? 8 : 0xFFF0, e);
    }
    static QByteArray odSegments(const QVector<quint16> &segments)
    {
        QByteArray e;
        for (quint16 seg : segments) e += odU16(seg);
        return odArray(int(segments.size()), 2, e);
    }
    static QString odPathText(const QPainterPath &path)
    {
        QStringList parts;
        auto pt = [](const QPainterPath::Element &e) { return QStringLiteral("%1,%2").arg(e.x, 0, 'g', 6).arg(e.y, 0, 'g', 6); };
        for (int i = 0; i < path.elementCount(); ++i) {
            const QPainterPath::Element e = path.elementAt(i);
            if (e.type == QPainterPath::MoveToElement) parts << QStringLiteral("M") + pt(e);
            else if (e.type == QPainterPath::LineToElement) parts << QStringLiteral("L") + pt(e);
            else if (e.type == QPainterPath::CurveToElement && i + 2 < path.elementCount()) {
                parts << QStringLiteral("C") + pt(e) + QLatin1Char(' ') + pt(path.elementAt(i + 1)) + QLatin1Char(' ') + pt(path.elementAt(i + 2));
                i += 2;
            }
        }
        return parts.join(QLatin1Char(' '));
    }
    // The hyperlink a shape holds (an IHlink), with an address, a place, and the text shown.
    static QByteArray odHyperlinkString(const QString &text) { return odU32(quint32(text.size() + 1)) + odUtf16z(text); }
    static QByteArray odUrlHyperlink(const QString &url, const QString &place, bool asString)
    {
        const quint32 flags = 0x1 | 0x10 | (place.isEmpty() ? 0 : 0x8) | (asString ? 0x100 : 0);
        QByteArray h = odU32(2) + odU32(flags) + odHyperlinkString(QStringLiteral("shown"));
        if (asString) {
            h += odHyperlinkString(url);
        } else {
            const QByteArray guid("\xE0\xC9\xEA\x79\xF9\xBA\xCE\x11\x8C\x82\x00\xAA\x00\x4B\xA9\x0B", 16);
            h += guid + odU32(quint32(odUtf16z(url).size() + 24)) + odUtf16z(url) + QByteArray(24, 's');
        }
        if (!place.isEmpty()) h += odHyperlinkString(place);
        return h;
    }
    static QByteArray odFileHyperlink(const QString &ansi, const QString &wide)
    {
        const QByteArray guid("\x03\x03\x00\x00\x00\x00\x00\x00\xC0\x00\x00\x00\x00\x00\x00\x46", 16);
        QByteArray h = odU32(2) + odU32(0x1) + guid + odU16(0) + odU32(quint32(ansi.size() + 1)) + ansi.toLatin1() + QByteArray(1, '\0');
        h += odU16(0) + odU16(0) + QByteArray(20, '\0');
        const QByteArray unicode = odU32(quint32(wide.size() * 2)) + odU16(3) + odUtf16z(wide).left(wide.size() * 2);
        return h + odU32(quint32(unicode.size())) + unicode;
    }

    static OdrawContext odContext()
    {
        OdrawContext ctx;
        ctx.clientAnchor = [](const QByteArray &a) {
            if (a.size() < 8) return QRectF();
            const double l = qFromLittleEndian<qint16>(a.constData()), t = qFromLittleEndian<qint16>(a.constData() + 2);
            return QRectF(QPointF(l, t), QPointF(qFromLittleEndian<qint16>(a.constData() + 4), qFromLittleEndian<qint16>(a.constData() + 6)));
        };
        ctx.schemeColor = [](int index) { return index >= 0 && index < 8 ? QColor(10 * index, 20 + index, 30 + 2 * index) : QColor(); };
        ctx.picture = [](int index) { return index > 0 ? QStringLiteral("pic%1").arg(index) : QString(); };
        return ctx;
    }
    static ShapeItem *odShapeOf(const ItemList &items, int i)
    {
        return i < int(items.size()) && items[i]->type() == ItemType::Shape ? static_cast<ShapeItem *>(items[i].get()) : nullptr;
    }
    static QColor odRgb(const ColorRef &c) { return c.kind() == ColorRef::Rgb ? c.rgbValue() : QColor(); }

private Q_SLOTS:
    void odrawReadsAFilledRectangle()
    {
        const QByteArray sp = odShape(1, 0xA00, {{0x0181, 0x00C07030}, {0x0182, 0x8000}, {0x01BF, 0x00100010}, {0x01C0, 0x00402010}, {0x01CB, 25400}, {0x01FF, 0x00080008},
                                                  {0x0380, 0, QByteArray("R\0e\0c\0t\0\0\0", 10)}, {0x0381, 0, QByteArray("a\0l\0t\0\0\0", 8)}},
                                     odClientAnchor(100, 50, 300, 150));
        const ItemList items = readOdrawShapes(odDrawing(sp), odContext());
        QCOMPARE(int(items.size()), 1);
        ShapeItem *s = odShapeOf(items, 0);
        QVERIFY(s);
        QCOMPARE(s->shape, QStringLiteral("rect"));
        QCOMPARE(s->rect, QRectF(100, 50, 200, 100));
        QVERIFY(s->fill.type == Fill::Solid);
        QCOMPARE(odRgb(s->fill.color), QColor(0x30, 0x70, 0xC0));
        QVERIFY(qAbs(s->fill.transparency - 0.5) < 0.001);
        QVERIFY(!s->stroke.isNone());
        QCOMPARE(odRgb(s->stroke.color), QColor(0x10, 0x20, 0x40));
        QVERIFY(qAbs(s->stroke.width - 2.0) < 1e-9);
        QCOMPARE(s->name, QStringLiteral("Rect"));
        QCOMPARE(s->altText, QStringLiteral("alt"));
        QVERIFY(s->rotation == 0 && !s->flipH && !s->flipV);
        // The same shape, as the format has it when nothing is said: a white fill and a thin black line.
        const ItemList plain = readOdrawShapes(odDrawing(odShape(1, 0xA00, {}, odClientAnchor(0, 0, 10, 10))), odContext());
        s = odShapeOf(plain, 0);
        QVERIFY(s);
        QCOMPARE(odRgb(s->fill.color), QColor(Qt::white));
        QCOMPARE(odRgb(s->stroke.color), QColor(Qt::black));
        QVERIFY(qAbs(s->stroke.width - 0.75) < 1e-9);
        // Not filled and without a line.
        const ItemList bare = readOdrawShapes(odDrawing(odShape(1, 0xA00, {{0x01BF, 0x00100000}, {0x01FF, 0x00080000}}, odClientAnchor(0, 0, 10, 10))), odContext());
        s = odShapeOf(bare, 0);
        QVERIFY(s);
        QVERIFY(s->fill.isNone());
        QVERIFY(s->stroke.isNone());
        // A shape with no anchor has no place; the host's client records may give it one.
        const ItemList unplaced = readOdrawShapes(odDrawing(odShape(1, 0xA00, {})), odContext());
        QCOMPARE(int(unplaced.size()), 1);
        QVERIFY(unplaced[0]->rect.isNull());
        // The drawing's top group alone reads the same, and so does one shape by itself.
        const QByteArray top = odContainer(0xF003, odContainer(0xF004, odRecord(1, 0, 0xF009, QByteArray(16, '\0')) + odFsp(0, 0x5)) + sp);
        QCOMPARE(int(readOdrawShapes(top, odContext()).size()), 1);
        QCOMPARE(readOdrawShapes(top, odContext())[0]->rect, QRectF(100, 50, 200, 100));
        QCOMPARE(int(readOdrawShapes(sp, odContext()).size()), 1);
        QVERIFY(readOdrawShapes(odRecord(0, 0, 0xF00B, QByteArray()), odContext()).empty());
    }

    void odrawReadsPresetShapesLinesAndAdjustments()
    {
        const QByteArray anchor = odClientAnchor(0, 0, 100, 50);
        auto shape = [&](int type, const QVector<OdProp> &props) {
            const ItemList items = readOdrawShapes(odDrawing(odShape(type, 0xA00, props, anchor)), odContext());
            return items.empty() ? ItemPtr() : items[0];
        };
        // Numbers the format gives presets; the ones JeffOffice has no shape for are rectangles.
        auto presetOf = [&](int type) { return static_cast<ShapeItem *>(shape(type, {}).get())->shape; };
        QCOMPARE(presetOf(3), QStringLiteral("ellipse"));
        QCOMPARE(presetOf(4), QStringLiteral("diamond"));
        QCOMPARE(presetOf(96), QStringLiteral("smiley"));
        QCOMPARE(presetOf(4000), QStringLiteral("rect"));
        QCOMPARE(presetOf(41), QStringLiteral("rect"));
        QCOMPARE(presetOf(0), QStringLiteral("rect"));
        // A rounded corner is a share of the shorter side, in 21600ths; a triangle's peak a share of the width.
        const ItemPtr roundRect = shape(2, {{0x0147, 5400}});
        auto *rr = static_cast<ShapeItem *>(roundRect.get());
        QCOMPARE(rr->shape, QStringLiteral("roundRect"));
        QCOMPARE(int(rr->adj.size()), 1);
        QVERIFY(qAbs(rr->adj[0] - 0.25) < 1e-9);
        const ItemPtr triangle = shape(5, {{0x0147, 21600}});
        auto *tri = static_cast<ShapeItem *>(triangle.get());
        QVERIFY(tri->adj.size() == 1 && qAbs(tri->adj[0] - 1.0) < 1e-9);
        QVERIFY(static_cast<ShapeItem *>(shape(2, {}).get())->adj.isEmpty());   // the preset's own
        // Block arrows: the shaft as a share of the width of the arrow, the head as a share of the shorter side, from where the format puts the head's rear.
        auto arrow = [&](int type, const QVector<OdProp> &props) {
            const ItemPtr it = shape(type, props);
            return static_cast<ShapeItem *>(it.get())->adj;
        };
        auto same = [](const QVector<double> &a, const QVector<double> &b) {
            if (a.size() != b.size()) return false;
            for (int i = 0; i < a.size(); ++i) if (qAbs(a[i] - b[i]) > 1e-9) return false;
            return true;
        };
        QVERIFY(same(arrow(13, {}), {0.5, 0.5}));                                              // the defaults, 100 wide and 50 high
        QVERIFY(same(arrow(13, {{0x0147, 12000}, {0x0148, 3000}}), {1 - 2 * 3000 / 21600.0, (1 - 12000 / 21600.0) * 2}));
        QVERIFY(same(arrow(66, {{0x0147, 9000}}), {0.5, 9000 / 21600.0 * 2}));
        QVERIFY(same(arrow(68, {{0x0147, 4000}, {0x0148, 3000}}), {1 - 2 * 3000 / 21600.0, 4000 / 21600.0}));
        QVERIFY(same(arrow(67, {{0x0147, 14000}}), {0.5, 1 - 14000 / 21600.0}));
        // A head longer than the arrow is the arrow.
        QVERIFY(same(arrow(13, {{0x0147, 0}}), {0.5, 1.0}));
        // The dashes of a line: the format's numbers in JeffOffice's order.
        const struct { int dashing; Stroke::Dash dash; } dashes[] = {{0, Stroke::SolidLine}, {1, Stroke::DashLine}, {2, Stroke::SquareDot}, {3, Stroke::DashDot},
                                                                       {4, Stroke::LongDashDotDot}, {5, Stroke::RoundDot}, {6, Stroke::DashLine}, {7, Stroke::LongDash},
                                                                       {8, Stroke::DashDot}, {9, Stroke::LongDashDot}, {10, Stroke::LongDashDotDot}};
        for (const auto &d : dashes) {
            const ItemPtr it = shape(3, {{0x01CE, quint32(d.dashing)}});
            QVERIFY2(it->stroke.dash == d.dash, qPrintable(QString::number(d.dashing)));
        }
        // Width, compound line, join, cap, and opacity.
        const ItemPtr styled = shape(3, {{0x01C0, 0x0000FF}, {0x01C1, 0x4000}, {0x01CB, 19050}, {0x01CD, 2}, {0x01D6, 1}, {0x01D7, 0}, {0x01FF, 0x00080008}});
        QVERIFY(qAbs(styled->stroke.width - 1.5) < 1e-9);
        QVERIFY(styled->stroke.compound == Stroke::ThickThin);
        QVERIFY(styled->stroke.join == Qt::RoundJoin);
        QVERIFY(styled->stroke.cap == Qt::RoundCap);
        QVERIFY(qAbs(styled->stroke.transparency - 0.75) < 1e-3);
        QCOMPARE(odRgb(styled->stroke.color), QColor(255, 0, 0));
        const ItemPtr mitered = shape(3, {{0x01D6, 2}, {0x01D7, 1}, {0x01CD, 4}});
        QVERIFY(mitered->stroke.join == Qt::MiterJoin && mitered->stroke.cap == Qt::SquareCap && mitered->stroke.compound == Stroke::Triple);
        QVERIFY(shape(3, {{0x01D6, 0}})->stroke.join == Qt::BevelJoin);
        QVERIFY(shape(3, {{0x01D7, 2}})->stroke.cap == Qt::FlatCap);
    }

    void odrawReadsRotationAndFlips()
    {
        const QByteArray anchor = odClientAnchor(10, 20, 110, 70);
        auto one = [&](quint32 flags, const QVector<OdProp> &props, int type = 1) {
            return readOdrawShapes(odDrawing(odShape(type, 0xA00 | flags, props, anchor)), odContext())[0];
        };
        ItemPtr it = one(0, {{0x0004, 30u << 16}});
        QVERIFY(qAbs(it->rotation - 30) < 1e-9);
        QCOMPARE(it->rect, QRectF(10, 20, 100, 50));   // the frame is the unturned one
        // The format turns a shape and then flips it, so one flip shows the turn the other way round.
        it = one(0x40, {{0x0004, quint32(-30 * 65536)}});   // negative: counterclockwise, as 330
        QVERIFY(qAbs(it->rotation - 30) < 1e-9);
        QVERIFY(it->flipH && !it->flipV);
        it = one(0x80, {{0x0004, 0x00008000}});             // half a degree
        QVERIFY(qAbs(it->rotation - 359.5) < 1e-9);
        QVERIFY(!it->flipH && it->flipV);
        it = one(0xC0, {{0x0004, 400u << 16}});             // both flips: a half turn, which turns the same way
        QVERIFY(qAbs(it->rotation - 40) < 1e-9);
        QVERIFY(it->flipH && it->flipV);
        it = one(0x40, {});
        QVERIFY(it->rotation == 0 && it->flipH);
        // Turned about a quarter, the frame the file gives has its width and height exchanged, about its center.
        for (double deg : {45.0, 90.0, 134.0, 225.0, 270.0, 314.0}) {
            it = one(0, {{0x0004, quint32(deg * 65536)}});
            QVERIFY2(qAbs(it->rect.width() - 50) < 1e-9 && qAbs(it->rect.height() - 100) < 1e-9 && qAbs(it->rect.center().x() - 60) < 1e-9 && qAbs(it->rect.center().y() - 45) < 1e-9,
                     qPrintable(QString::number(deg)));
        }
        for (double deg : {0.0, 44.0, 135.0, 180.0, 224.0, 315.0, 359.0}) {
            it = one(0, {{0x0004, quint32(deg * 65536)}});
            QVERIFY2(it->rect == QRectF(10, 20, 100, 50), qPrintable(QString::number(deg)));
        }
        // The numbered trapezoid is wide at the top and JeffOffice's at the bottom: it is flipped to match, and its turn goes with the flip.
        it = one(0, {}, 8);
        QVERIFY(it->flipV && !it->flipH && it->rotation == 0);
        it = one(0x80, {}, 8);   // as the format has one flipped to be wide at the bottom
        QVERIFY(!it->flipV && !it->flipH);
        it = one(0, {{0x0004, 30u << 16}}, 8);
        QVERIFY(it->flipV && qAbs(it->rotation - 30) < 1e-9);
        it = one(0x80, {{0x0004, 30u << 16}}, 8);
        QVERIFY(!it->flipV && qAbs(it->rotation - 330) < 1e-9);
    }

    void odrawMapsGroupChildrenIntoTheGroupFrame()
    {
        // A group at (100,100)-(300,200) whose children are placed in 1000 x 1000 units.
        const QByteArray inner = odGroup(QRect(0, 0, 100, 100), 0x2, odChildAnchor(0, 0, 500, 1000),
                                         odShape(1, 0xA02, {}, odChildAnchor(0, 0, 50, 50)));
        const QByteArray group = odGroup(QRect(0, 0, 1000, 1000), 0, odClientAnchor(100, 100, 300, 200),
                                         odShape(1, 0xA02, {{0x0380, 0, QByteArray("A\0\0\0", 4)}}, odChildAnchor(0, 0, 500, 500)) +
                                         odShape(3, 0xA02, {}, odChildAnchor(500, 500, 1000, 1000)) + inner);
        const ItemList items = readOdrawShapes(odDrawing(group + odShape(1, 0xA00, {}, odClientAnchor(0, 0, 5, 5))), odContext());
        QCOMPARE(int(items.size()), 2);
        QVERIFY(items[0]->type() == ItemType::Group);
        auto *g = static_cast<GroupItem *>(items[0].get());
        QCOMPARE(int(g->children.size()), 3);
        QCOMPARE(g->children[0]->rect, QRectF(100, 100, 100, 50));
        QCOMPARE(g->children[0]->name, QStringLiteral("A"));
        QCOMPARE(g->children[1]->rect, QRectF(200, 150, 100, 50));
        QVERIFY(static_cast<ShapeItem *>(g->children[1].get())->shape == QLatin1String("ellipse"));
        QVERIFY(g->children[2]->type() == ItemType::Group);
        auto *g2 = static_cast<GroupItem *>(g->children[2].get());
        QCOMPARE(int(g2->children.size()), 1);
        QCOMPARE(g2->children[0]->rect, QRectF(100, 100, 50, 50));   // the inner group is 100 wide, for 100 units
        QCOMPARE(g->rect, QRectF(100, 100, 200, 100));
        QCOMPARE(g2->rect, QRectF(100, 100, 50, 50));
        QVERIFY(items[1]->type() == ItemType::Shape);
        QCOMPARE(items[1]->rect, QRectF(0, 0, 5, 5));
        // A group whose first shape isn't made for describing one has no frame: its shapes are its own.
        const ItemList plain = readOdrawShapes(odDrawing(odContainer(0xF003, odShape(1, 0xA00, {}, odClientAnchor(0, 0, 10, 10)) + odShape(3, 0xA00, {}, odClientAnchor(20, 0, 30, 10)))), odContext());
        QCOMPARE(int(plain.size()), 1);
        QVERIFY(plain[0]->type() == ItemType::Group);
        QCOMPARE(int(static_cast<GroupItem *>(plain[0].get())->children.size()), 2);
        QCOMPARE(plain[0]->rect, QRectF(0, 0, 30, 10));
    }

    void odrawTurnsAndFlipsAGroupAboutItsFrame()
    {
        auto groupOf = [&](quint32 flags, const QVector<OdProp> &props) {
            const QByteArray g = odGroup(QRect(0, 0, 100, 100), flags, odClientAnchor(0, 0, 100, 100), odShape(1, 0xA02, {}, odChildAnchor(10, 10, 30, 20)), props);
            return readOdrawShapes(odDrawing(g), odContext());
        };
        ItemList items = groupOf(0x40, {});   // flipped across
        auto *g = static_cast<GroupItem *>(items[0].get());
        QCOMPARE(g->children[0]->rect, QRectF(70, 10, 20, 10));
        QVERIFY(g->children[0]->flipH && !g->children[0]->flipV);
        items = groupOf(0x80, {});            // flipped upside down
        g = static_cast<GroupItem *>(items[0].get());
        QCOMPARE(g->children[0]->rect, QRectF(10, 80, 20, 10));
        QVERIFY(!g->children[0]->flipH && g->children[0]->flipV);
        items = groupOf(0, {{0x0004, 90u << 16}});   // a quarter turn about the middle
        g = static_cast<GroupItem *>(items[0].get());
        QVERIFY(qAbs(g->children[0]->rotation - 90) < 1e-9);
        QVERIFY(qAbs(g->children[0]->rect.center().x() - 85) < 1e-9 && qAbs(g->children[0]->rect.center().y() - 20) < 1e-9);
        QVERIFY(qAbs(g->children[0]->rect.width() - 20) < 1e-9);
        // Turned, then flipped, as a shape is: the child (at 85,20 after the quarter turn) across the middle.
        items = groupOf(0x40, {{0x0004, 90u << 16}});
        g = static_cast<GroupItem *>(items[0].get());
        QVERIFY(g->children[0]->flipH && !g->children[0]->flipV);
        QVERIFY(qAbs(g->children[0]->rotation - 270) < 1e-9);
        QVERIFY(qAbs(g->children[0]->rect.center().x() - 15) < 1e-9 && qAbs(g->children[0]->rect.center().y() - 20) < 1e-9);
    }

    void odrawSkipsDeletedHiddenAndBackgroundShapes()
    {
        const QByteArray at = odClientAnchor(0, 0, 10, 10);
        const QByteArray shapes = odShape(1, 0xA08, {}, at) +                          // deleted
                                  odShape(1, 0xA00, {{0x03BF, 0x00020002}}, at) +      // hidden
                                  odShape(1, 0xC00, {}, at) +                          // the background
                                  odShape(3, 0xA00, {}, at) +                          // kept
                                  odShape(1, 0xA01, {}, at) +                          // a group's own shape alone
                                  odContainer(0xF004, odRecord(0, 0, 0xF00B, QByteArray()) + at) +   // no shape record
                                  odGroup(QRect(0, 0, 10, 10), 0x8, at, odShape(1, 0xA02, {}, odChildAnchor(0, 0, 5, 5))) +   // a deleted group
                                  odGroup(QRect(0, 0, 10, 10), 0, at, odShape(1, 0xA0A, {}, odChildAnchor(0, 0, 5, 5)));      // a group of nothing
        const ItemList items = readOdrawShapes(odDrawing(shapes), odContext());
        QCOMPARE(int(items.size()), 1);
        QVERIFY(odShapeOf(items, 0) && odShapeOf(items, 0)->shape == QLatin1String("ellipse"));
    }

    void odrawResolvesEveryKindOfColor()
    {
        auto fillOf = [&](quint32 color, const QVector<OdProp> &more = {}, const OdrawContext *c = nullptr) {
            QVector<OdProp> props = {{0x0181, color}, {0x01BF, 0x00100010}};
            props += more;
            const OdrawContext ctx = c ? *c : odContext();
            const ItemList items = readOdrawShapes(odDrawing(odShape(1, 0xA00, props, odClientAnchor(0, 0, 10, 10))), ctx);
            return odRgb(items[0]->fill.color);
        };
        QCOMPARE(fillOf(0x00C07030), QColor(0x30, 0x70, 0xC0));
        QCOMPARE(fillOf(0x02C07030), QColor(0x30, 0x70, 0xC0));   // from the palette: the color it gave
        QCOMPARE(fillOf(0x04C07030), QColor(0x30, 0x70, 0xC0));   // a system RGB
        // A scheme color is the host's; with none, or one it can't give, the fill is white.
        QCOMPARE(fillOf(0x08000003), QColor(30, 23, 36));
        OdrawContext none = odContext();
        none.schemeColor = nullptr;
        QCOMPARE(fillOf(0x08000003, {}, &none), QColor(Qt::white));
        QCOMPARE(fillOf(0x08000009), QColor(Qt::white));
        // System colors: a Windows one, and the references to the shape's own colors.
        QCOMPARE(fillOf(0x10000005), QColor(Qt::white));          // window
        QCOMPARE(fillOf(0x10000008), QColor(Qt::black));          // window text
        QCOMPARE(fillOf(0x100000F2, {{0x01C0, 0x00102030}}), QColor(0x30, 0x20, 0x10));      // the line color
        QCOMPARE(fillOf(0x100000F5, {{0x0183, 0x00010203}}), QColor(3, 2, 1));              // the back color
        QCOMPARE(fillOf(0x100000F3, {{0x0201, 0x00FF8000}}), QColor(0, 0x80, 0xFF));        // the shadow color
        // With a darkening or lightening function (1 and 2), by an amount of 255.
        QCOMPARE(fillOf(0x10800100 | 0xF2, {{0x01C0, 0x00C8C8C8}}), QColor(100, 100, 100));
        QCOMPARE(fillOf(0x10800200 | 0xF2, {{0x01C0, 0x00000000}}), QColor(126, 126, 126));
        // A color that refers to itself ends at black.
        QCOMPARE(fillOf(0x100000F0), QColor(Qt::black));
        QCOMPARE(fillOf(0x100000F2, {{0x01C0, 0x100000F0}}), QColor(Qt::black));
    }

    void odrawBackgroundIsTheBackgroundShapesFill()
    {
        const QByteArray at = odClientAnchor(0, 0, 10, 10);
        const QByteArray bg = odShape(1, 0xC00, {{0x0181, 0x00FF8000}, {0x01BF, 0x00120012}});
        const QByteArray drawing = odDrawing(odShape(1, 0xA00, {}, at), bg);
        Fill f = odrawBackground(drawing, odContext());
        QVERIFY(f.type == Fill::Solid);
        QCOMPARE(odRgb(f.color), QColor(0, 0x80, 0xFF));
        QCOMPARE(int(readOdrawShapes(drawing, odContext()).size()), 1);   // not among the shapes
        // A scheme color is the host's.
        f = odrawBackground(odDrawing(QByteArray(), odShape(1, 0xC00, {{0x0181, 0x08000002}})), odContext());
        QCOMPARE(odRgb(f.color), QColor(20, 22, 34));
        // No background shape, or one that isn't filled, has no fill.
        QVERIFY(odrawBackground(odDrawing(odShape(1, 0xA00, {}, at)), odContext()).isNone());
        QVERIFY(odrawBackground(odDrawing(QByteArray(), odShape(1, 0xC00, {{0x0181, 0x00FF8000}, {0x01BF, 0x00120002}})), odContext()).isNone());
        QVERIFY(odrawBackground(QByteArray(), odContext()).isNone());
    }

    void odrawReadsTextBoxesThroughTheClientRecords()
    {
        QVector<QByteArray> boxes, datas;
        OdrawContext ctx = odContext();
        ctx.client = [&](jp::Item &item, const QByteArray &box, const QByteArray &data) {
            boxes << box;
            datas << data;
            if (item.type() == ItemType::Text) static_cast<TextItem &>(item).storyId = QStringLiteral("story:") + QString::fromLatin1(box);
            if (item.type() == ItemType::Shape) static_cast<ShapeItem &>(item).storyId = QStringLiteral("story:") + QString::fromLatin1(box);
            item.role = QString::fromLatin1(data);
        };
        const QByteArray at = odClientAnchor(0, 0, 100, 50);
        const QByteArray text = odRecord(0, 0, 0xF00D, QByteArray("hello")) + odRecord(0, 0, 0xF011, QByteArray("cd"));
        // A text box: not filled, no line, the text's margins, where it sits, which way it runs.
        ItemList items = readOdrawShapes(odDrawing(odShape(202, 0xA00, {{0x0081, 127000}, {0x0082, 63500}, {0x0083, 0}, {0x0084, 25400}, {0x0087, 2}, {0x0088, 1}, {0x00BF, 0x00010001}}, at, text)), ctx);
        QCOMPARE(int(items.size()), 1);
        QVERIFY(items[0]->type() == ItemType::Text);
        auto *t = static_cast<TextItem *>(items[0].get());
        QCOMPARE(t->storyId, QStringLiteral("story:hello"));
        QCOMPARE(t->role, QStringLiteral("cd"));
        QVERIFY(t->fill.isNone() && t->stroke.isNone());
        QCOMPARE(t->insets, QMarginsF(10, 5, 0, 2));
        QVERIFY(t->valign == jp::VAlign::Bottom);
        QVERIFY(t->vertical);
        QVERIFY(t->autofit == TextItem::GrowBox);
        // Its own fill and line when it has them; the format's margins when it says none.
        items = readOdrawShapes(odDrawing(odShape(202, 0xA00, {{0x0181, 0x0000FF}, {0x01BF, 0x00100010}, {0x01FF, 0x00080008}, {0x0087, 1}}, at, text)), ctx);
        t = static_cast<TextItem *>(items[0].get());
        QCOMPARE(odRgb(t->fill.color), QColor(255, 0, 0));
        QVERIFY(!t->stroke.isNone());
        QCOMPARE(t->insets, QMarginsF(7.2, 3.6, 7.2, 3.6));
        QVERIFY(t->valign == jp::VAlign::Middle && !t->vertical && t->autofit == TextItem::NoAutofit);
        // A rectangle with text is a text box; with a text id and no client records too (and the host isn't asked).
        items = readOdrawShapes(odDrawing(odShape(1, 0xA00, {{0x0181, 0x0000FF}, {0x01BF, 0x00100010}}, at, text)), ctx);
        QVERIFY(items[0]->type() == ItemType::Text);
        QCOMPARE(odRgb(items[0]->fill.color), QColor(255, 0, 0));
        const int calls = int(boxes.size());
        items = readOdrawShapes(odDrawing(odShape(1, 0xA00, {{0x0080, 5}}, at)), ctx);
        QVERIFY(items[0]->type() == ItemType::Text);
        QCOMPARE(int(boxes.size()), calls);
        // Another shape with text stays that shape, and its story is set by the host.
        items = readOdrawShapes(odDrawing(odShape(3, 0xA00, {{0x0087, 4}}, at, text)), ctx);
        QVERIFY(items[0]->type() == ItemType::Shape);
        auto *s = static_cast<ShapeItem *>(items[0].get());
        QCOMPARE(s->shape, QStringLiteral("ellipse"));
        QCOMPARE(s->storyId, QStringLiteral("story:hello"));
        QVERIFY(s->valign == jp::VAlign::Middle);
        QVERIFY(!s->fill.isNone());
        // Client data alone (a placeholder): the host gets an empty text box, and the shape's place is its to give.
        boxes.clear();
        datas.clear();
        items = readOdrawShapes(odDrawing(odShape(1, 0xA00, {}, QByteArray(), odRecord(0, 0, 0xF011, QByteArray("ph")))), ctx);
        QCOMPARE(int(boxes.size()), 1);
        QVERIFY(boxes[0].isEmpty());
        QCOMPARE(datas[0], QByteArray("ph"));
        QCOMPARE(items[0]->role, QStringLiteral("ph"));
        QVERIFY(items[0]->rect.isNull());
        // The host's functions are all optional.
        QCOMPARE(int(readOdrawShapes(odDrawing(odShape(202, 0xA00, {}, at, text)), OdrawContext()).size()), 1);
    }

    void odrawReadsPicturesWithTheirCrop()
    {
        QVector<int> asked;
        OdrawContext ctx = odContext();
        ctx.picture = [&](int index) {
            asked << index;
            return index == 9 ? QString() : QStringLiteral("pic%1").arg(index);
        };
        const QByteArray at = odClientAnchor(10, 20, 110, 70);
        auto picture = [&](int type, quint32 flags, const QVector<OdProp> &props) {
            const ItemList items = readOdrawShapes(odDrawing(odShape(type, flags, props, at)), ctx);
            return items.empty() || items[0]->type() != ItemType::Picture ? std::shared_ptr<PictureItem>() : std::static_pointer_cast<PictureItem>(items[0]);
        };
        // A picture frame: the picture in the store at that number, in the frame, uncropped.
        std::shared_ptr<PictureItem> pic = picture(75, 0xA00, {{0x0104, 3, {}, true}});
        QVERIFY(pic);
        QCOMPARE(pic->imageId, QStringLiteral("pic3"));
        QCOMPARE(asked, QVector<int>({3}));
        QCOMPARE(pic->rect, QRectF(10, 20, 100, 50));
        QCOMPARE(pic->imgRect, QRectF(0, 0, 100, 50));
        QVERIFY(pic->stroke.isNone() && pic->fill.isNone());
        // Cropped: an eighth off the left and top, a quarter off the right and bottom, so the frame holds 5/8 of each side.
        pic = picture(75, 0xA00, {{0x0104, 3, {}, true}, {0x0100, 0x2000}, {0x0101, 0x4000}, {0x0102, 0x2000}, {0x0103, 0x4000}});
        QCOMPARE(pic->imgRect, QRectF(-20, -10, 160, 80));
        // Negative: the picture is smaller than the frame, with room around it.
        pic = picture(75, 0xA00, {{0x0104, 3, {}, true}, {0x0102, quint32(-0x4000)}, {0x0103, quint32(-0x4000)}});
        QCOMPARE(pic->imgRect, QRectF(50.0 / 3, 0, 200.0 / 3, 50));
        // Brightness and contrast, grayscale and black and white, a color made clear.
        pic = picture(75, 0xA00, {{0x0104, 3, {}, true}, {0x0109, 16384}, {0x0108, 0x18000}, {0x013F, 0x00040004}, {0x0107, 0x00FFFFFF}});
        QVERIFY(qAbs(pic->brightness - 50) < 1e-9 && qAbs(pic->contrast - 50) < 1e-9);
        QVERIFY(pic->recolor == PictureItem::Grayscale);
        QVERIFY(pic->hasTransparentColor);
        QCOMPARE(pic->transparentColor, QColor(Qt::white));
        pic = picture(75, 0xA00, {{0x0104, 3, {}, true}, {0x013F, 0x00020002}});
        QVERIFY(pic->recolor == PictureItem::BlackWhite && pic->brightness == 0 && pic->contrast == 0 && !pic->hasTransparentColor);
        // A frame around it, when the shape has a line.
        pic = picture(75, 0xA00, {{0x0104, 3, {}, true}, {0x01FF, 0x00080008}, {0x01CB, 12700}});
        QVERIFY(!pic->stroke.isNone() && qAbs(pic->stroke.width - 1) < 1e-9);
        // A picture the host can't give, a frame with none, a rectangle that names one, and an object with its picture.
        pic = picture(75, 0xA00, {{0x0104, 9, {}, true}});
        QVERIFY(pic && pic->imageId.isEmpty());
        pic = picture(75, 0xA00, {});
        QVERIFY(pic && pic->imageId.isEmpty());
        pic = picture(1, 0xA00, {{0x0104, 4, {}, true}});
        QVERIFY(pic && pic->imageId == QLatin1String("pic4"));
        pic = picture(1, 0xA10, {{0x0104, 5, {}, true}});   // an OLE object
        QVERIFY(pic && pic->imageId == QLatin1String("pic5"));
        // The format's number in a picture's name: not asked for a missing one.
        asked.clear();
        picture(75, 0xA00, {{0x0104, 0, {}, true}});
        QVERIFY(asked.isEmpty());
    }

    void odrawReadsGradientAndPictureFills()
    {
        auto fillOf = [&](const QVector<OdProp> &more, OdrawContext *c = nullptr) {
            QVector<OdProp> props = {{0x01BF, 0x00100010}, {0x019C, 0}};   // no shading curve: the colors run straight
            props += more;
            const OdrawContext ctx = c ? *c : odContext();
            return readOdrawShapes(odDrawing(odShape(1, 0xA00, props, odClientAnchor(0, 0, 10, 10))), ctx)[0]->fill;
        };
        const OdProp red{0x0181, 0x0000FF}, blue{0x0183, 0xFF0000};
        // Linear: the first color runs toward 270 - a degrees for a stored angle a of 0 or more, 90 - a for a negative one.
        const struct { double stored; double toward; } angles[] = {{0, 270}, {45, 225}, {90, 180}, {135, 135}, {180, 90}, {225, 45}, {270, 0}, {315, 315}, {-45, 135}, {-90, 180}, {-135, 225}};
        for (const auto &a : angles) {
            const Fill f = fillOf({{0x0180, 7}, red, blue, {0x018B, quint32(qint32(a.stored * 65536))}});
            QVERIFY(f.type == Fill::Gradient && f.gradType == Fill::Linear);
            QVERIFY2(qAbs(f.angle - a.toward) < 1e-6, qPrintable(QString::number(a.stored)));
        }
        Fill f = fillOf({{0x0180, 4}, red, blue});
        QVERIFY(f.type == Fill::Gradient && f.gradType == Fill::Linear);
        QCOMPARE(int(f.stops.size()), 2);
        QCOMPARE(f.stops[0].pos, 0.0);
        QCOMPARE(odRgb(f.stops[0].color), QColor(255, 0, 0));
        QCOMPARE(f.stops[1].pos, 1.0);
        QCOMPARE(odRgb(f.stops[1].color), QColor(0, 0, 255));
        QCOMPARE(odRgb(f.color), QColor(255, 0, 0));
        QCOMPARE(odRgb(f.color2), QColor(0, 0, 255));
        QVERIFY(fillOf({{0x0180, 8}, red, blue}).gradType == Fill::Linear);
        // The focus: both ends the first color at 50, the colors the other way round at 100, swapped below zero.
        auto stopsOf = [&](int focus) {
            const Fill g = fillOf({{0x0180, 7}, red, blue, {0x018C, quint32(focus)}});
            QStringList out;
            for (const GradientStop &st : g.stops) out << QStringLiteral("%1%2").arg(odRgb(st.color) == QColor(255, 0, 0) ? 'r' : 'b').arg(st.pos);
            return out.join(QLatin1Char(' '));
        };
        QCOMPARE(stopsOf(0), QStringLiteral("r0 b1"));
        QCOMPARE(stopsOf(50), QStringLiteral("r0 b0.5 r1"));
        QCOMPARE(stopsOf(100), QStringLiteral("b0 r1"));
        QCOMPARE(stopsOf(-100), QStringLiteral("r0 b1"));
        QCOMPARE(stopsOf(-50), QStringLiteral("b0 r0.5 b1"));
        // Opacity of each color.
        f = fillOf({{0x0180, 7}, red, blue, {0x0182, 0x8000}, {0x0184, 0x4000}});
        QVERIFY(qAbs(f.stops[0].transparency - 0.5) < 1e-6 && qAbs(f.stops[1].transparency - 0.75) < 1e-6);
        // A list of colors with their places, which may come in any order.
        QByteArray list;
        list += odU32(0x00FF0000) + odU32(65536);       // blue at the end
        list += odU32(0x0000FF00) + odU32(0x4000);      // green a quarter along
        list += odU32(0x000000FF) + odU32(0);           // red at the start
        f = fillOf({{0x0180, 7}, red, blue, {0x0197, 0, odArray(3, 8, list)}});
        QCOMPARE(int(f.stops.size()), 3);
        QVERIFY(f.stops[0].pos == 0 && f.stops[1].pos == 0.25 && f.stops[2].pos == 1);
        QCOMPARE(odRgb(f.stops[1].color), QColor(0, 255, 0));
        QCOMPARE(odRgb(f.stops[2].color), QColor(0, 0, 255));
        // From the center or along the outline, the first color is the outer one: JeffOffice puts the center first.
        f = fillOf({{0x0180, 6}, red, blue});
        QVERIFY(f.gradType == Fill::PathGrad);
        QCOMPARE(odRgb(f.stops[0].color), QColor(0, 0, 255));
        QCOMPARE(f.stops[0].pos, 0.0);
        QCOMPARE(odRgb(f.stops[1].color), QColor(255, 0, 0));
        QCOMPARE(f.stops[1].pos, 1.0);
        f = fillOf({{0x0180, 5}, red, blue, {0x018C, 100}});
        QVERIFY(f.gradType == Fill::Rectangular);
        QCOMPARE(odRgb(f.stops[0].color), QColor(255, 0, 0));
        // A picture: stretched, or tiled for a texture or a pattern; solid when there's none to be had.
        f = fillOf({{0x0180, 3}, red, {0x0186, 2, {}, true}, {0x0182, 0x8000}});
        QVERIFY(f.type == Fill::Picture && f.imageId == QLatin1String("pic2") && !f.tile && qAbs(f.transparency - 0.5) < 1e-6);
        f = fillOf({{0x0180, 2}, {0x0186, 2, {}, true}});
        QVERIFY(f.type == Fill::Texture && f.imageId == QLatin1String("pic2") && f.tile);
        f = fillOf({{0x0180, 1}, {0x0186, 7, {}, true}});
        QVERIFY(f.type == Fill::Texture && f.imageId == QLatin1String("pic7"));
        f = fillOf({{0x0180, 3}, red, {0x0186, 0, {}, true}});
        QVERIFY(f.type == Fill::Solid);
        QCOMPARE(odRgb(f.color), QColor(255, 0, 0));
        OdrawContext none = odContext();
        none.picture = nullptr;
        QVERIFY(fillOf({{0x0180, 3}, red, {0x0186, 2, {}, true}}, &none).type == Fill::Solid);
        // The page's background: nothing of its own.
        QVERIFY(fillOf({{0x0180, 9}, red}).isNone());
        // Not filled: no gradient either.
        QVERIFY(readOdrawShapes(odDrawing(odShape(1, 0xA00, {{0x0180, 7}, red, blue, {0x01BF, 0x00100000}}, odClientAnchor(0, 0, 1, 1))), odContext())[0]->fill.isNone());
    }

    void odrawReadsArrowheadsShadowsAndHyperlinks()
    {
        const QByteArray at = odClientAnchor(0, 0, 100, 50);
        auto one = [&](int type, const QVector<OdProp> &props) { return readOdrawShapes(odDrawing(odShape(type, 0xA00, props, at)), odContext())[0]; };
        // Heads: the kind, and a size from the width and length (0 small to 2 large).
        ItemPtr it = one(20, {{0x01D0, 1}, {0x01D2, 2}, {0x01D3, 2}, {0x01D1, 5}, {0x01D4, 0}, {0x01D5, 1}});
        QVERIFY(it->type() == ItemType::Line);
        QVERIFY(it->stroke.startArrow == Arrow::Triangle && it->stroke.startSize == 2);
        QVERIFY(it->stroke.endArrow == Arrow::Open && it->stroke.endSize == 1);
        const struct { quint32 kind; Arrow arrow; } heads[] = {{0, Arrow::None}, {1, Arrow::Triangle}, {2, Arrow::Stealth}, {3, Arrow::Diamond}, {4, Arrow::Oval}, {5, Arrow::Open}, {6, Arrow::Open}, {99, Arrow::None}};
        for (const auto &h : heads) QVERIFY(one(20, {{0x01D1, h.kind}})->stroke.endArrow == h.arrow);
        it = one(20, {{0x01D0, 4}});
        QVERIFY(it->stroke.startSize == 1 && it->stroke.endArrow == Arrow::None);   // medium without a say
        // A shadow: its color, how clear, and where it falls (3 points across and down).
        it = one(1, {{0x023F, 0x00020002}, {0x0201, 0x00000080}, {0x0204, 0x8000}, {0x0205, 38100}, {0x0206, 38100}});
        QVERIFY(it->fx.shadow.on && !it->fx.shadow.inner);
        QCOMPARE(odRgb(it->fx.shadow.color), QColor(0x80, 0, 0));
        QVERIFY(qAbs(it->fx.shadow.transparency - 0.5) < 1e-6);
        QVERIFY(qAbs(it->fx.shadow.distance - 3 * std::sqrt(2.0)) < 1e-6 && qAbs(it->fx.shadow.angle - 45) < 1e-6 && it->fx.shadow.blur == 0);
        it = one(1, {{0x023F, 0x00020002}, {0x0205, 0}, {0x0206, 50800}});
        QVERIFY(qAbs(it->fx.shadow.distance - 4) < 1e-6 && qAbs(it->fx.shadow.angle - 90) < 1e-6);
        QVERIFY(!one(1, {{0x023F, 0x00020000}})->fx.shadow.on);
        QVERIFY(!one(1, {})->fx.shadow.on);
        // Hyperlinks: an address (a moniker, or a string), a place in it, a file.
        auto linkOf = [&](const QByteArray &h) { return one(1, {{0x0382, 0, h}})->hyperlink; };
        QCOMPARE(linkOf(odUrlHyperlink(QStringLiteral("http://example.com/a"), QString(), false)), QStringLiteral("http://example.com/a"));
        QCOMPARE(linkOf(odUrlHyperlink(QStringLiteral("http://example.com/a"), QStringLiteral("top"), false)), QStringLiteral("http://example.com/a#top"));
        QCOMPARE(linkOf(odUrlHyperlink(QStringLiteral("https://example.org/"), QString(), true)), QStringLiteral("https://example.org/"));
        QCOMPARE(linkOf(odUrlHyperlink(QStringLiteral("https://example.org/"), QStringLiteral("p"), true)), QStringLiteral("https://example.org/#p"));
        QCOMPARE(linkOf(odFileHyperlink(QStringLiteral("C:\\docs\\a.ppt"), QString())), QStringLiteral("C:\\docs\\a.ppt"));
        QCOMPARE(linkOf(odFileHyperlink(QStringLiteral("C:\\docs\\a.ppt"), QStringLiteral("C:\\docs\\b.ppt"))), QStringLiteral("C:\\docs\\b.ppt"));
        QVERIFY(linkOf(QByteArray()).isEmpty());
        QVERIFY(linkOf(odU32(2) + odU32(0)).isEmpty());
        const QByteArray whole = odUrlHyperlink(QStringLiteral("http://example.com/a"), QStringLiteral("top"), false);
        for (int cut = 0; cut < whole.size(); ++cut) linkOf(whole.left(cut));   // none of them is a crash
        QByteArray bad = whole;
        qToLittleEndian<quint32>(0xFFFFFFF0u, bad.data() + 8);   // a length of the text shown that's past the end
        QVERIFY(linkOf(bad).isEmpty());
        // The name and the description, as the shape's.
        it = one(1, {{0x0380, 0, odUtf16z(QStringLiteral("N\u00E9m"))}, {0x0381, 0, odUtf16z(QStringLiteral("d"))}});
        QCOMPARE(it->name, QStringLiteral("N\u00E9m"));
        QCOMPARE(it->altText, QStringLiteral("d"));
    }

    void odrawReadsLinesAndConnectors()
    {
        const QByteArray at = odClientAnchor(10, 20, 110, 70);
        auto lineOf = [&](int type, quint32 flags, const QVector<OdProp> &props = {}) {
            const ItemList items = readOdrawShapes(odDrawing(odShape(type, 0xA00 | flags, props, at)), odContext());
            return items.empty() || items[0]->type() != ItemType::Line ? std::shared_ptr<LineItem>() : std::static_pointer_cast<LineItem>(items[0]);
        };
        auto near = [](QPointF a, QPointF b) { return qAbs(a.x() - b.x()) < 1e-6 && qAbs(a.y() - b.y()) < 1e-6; };
        // A line runs from the frame's top left to its bottom right, over and down for a flip.
        std::shared_ptr<LineItem> l = lineOf(20, 0);
        QVERIFY(l);
        QVERIFY(near(l->p1, QPointF(10, 20)) && near(l->p2, QPointF(110, 70)));
        QVERIFY(qAbs(l->stroke.width - 0.75) < 1e-9 && odRgb(l->stroke.color) == QColor(Qt::black));
        QCOMPARE(l->rect, QRectF(10, 20, 100, 50));
        QVERIFY(l->rotation == 0 && !l->flipH && !l->flipV);
        l = lineOf(20, 0x40);
        QVERIFY(near(l->p1, QPointF(110, 20)) && near(l->p2, QPointF(10, 70)));
        l = lineOf(20, 0x80);
        QVERIFY(near(l->p1, QPointF(10, 70)) && near(l->p2, QPointF(110, 20)));
        l = lineOf(32, 0xC0);   // a straight connector, both ways
        QVERIFY(near(l->p1, QPointF(110, 70)) && near(l->p2, QPointF(10, 20)));
        // Turned about the middle of the frame (60,45), a quarter turn: the frame has its sides exchanged first.
        l = lineOf(20, 0, {{0x0004, 90u << 16}});
        QVERIFY(near(l->p1, QPointF(110, 20)) && near(l->p2, QPointF(10, 70)));
        QVERIFY(l->rotation == 0);
        l = lineOf(20, 0, {{0x0004, 30u << 16}});
        QVERIFY(near(l->p1, QPointF(60, 45) + QTransform().rotate(30).map(QPointF(-50, -25))) && near(l->p2, QPointF(60, 45) + QTransform().rotate(30).map(QPointF(50, 25))));
        // Without a line there is nothing to draw.
        QVERIFY(lineOf(20, 0, {{0x01FF, 0x00080000}})->stroke.isNone());
        // Bent and curved connectors are shapes with a path, in a frame of 100 x 50.
        auto path = [&](int type, const QVector<OdProp> &props = {}) {
            const ItemList items = readOdrawShapes(odDrawing(odShape(type, 0xA00, props, odClientAnchor(0, 0, 100, 50))), odContext());
            return odShapeOf(items, 0) ? odPathText(odShapeOf(items, 0)->customPath) : QStringLiteral("not a shape");
        };
        QCOMPARE(path(33), QStringLiteral("M0,0 L100,0 L100,50"));
        QCOMPARE(path(34), QStringLiteral("M0,0 L50,0 L50,50 L100,50"));
        QCOMPARE(path(34, {{0x0147, 5400}}), QStringLiteral("M0,0 L25,0 L25,50 L100,50"));
        QCOMPARE(path(35, {{0x0147, 5400}, {0x0148, 5400}}), QStringLiteral("M0,0 L25,0 L25,12.5 L100,12.5 L100,50"));
        QCOMPARE(path(36, {{0x0147, 5400}, {0x0148, 16200}, {0x0149, 10800}}), QStringLiteral("M0,0 L25,0 L25,37.5 L50,37.5 L50,50 L100,50"));
        QCOMPARE(path(37), QStringLiteral("M0,0 C50,0 100,25 100,50"));
        QCOMPARE(path(38), QStringLiteral("M0,0 C25,0 50,12.5 50,25 C50,37.5 75,50 100,50"));
        for (int type : {39, 40}) {
            const ItemList items = readOdrawShapes(odDrawing(odShape(type, 0xA00, {}, odClientAnchor(0, 0, 100, 50))), odContext());
            const QPainterPath &cp = odShapeOf(items, 0)->customPath;
            QVERIFY(cp.elementCount() > 3);
            QVERIFY(cp.elementAt(0).x == 0 && cp.elementAt(0).y == 0);
            QVERIFY(cp.elementAt(cp.elementCount() - 1).x == 100 && cp.elementAt(cp.elementCount() - 1).y == 50);
        }
        const ItemList conn = readOdrawShapes(odDrawing(odShape(34, 0xA00, {}, odClientAnchor(0, 0, 100, 50))), odContext());
        QVERIFY(odShapeOf(conn, 0)->fill.isNone() && !odShapeOf(conn, 0)->stroke.isNone());
    }

    void odrawReadsFreeforms()
    {
        auto path = [&](const QVector<OdProp> &props, int type = 0) {
            const ItemList items = readOdrawShapes(odDrawing(odShape(type, 0xA00, props, odClientAnchor(0, 0, 100, 50))), odContext());
            return odShapeOf(items, 0) ? odPathText(odShapeOf(items, 0)->customPath) : QStringLiteral("not a shape");
        };
        const QVector<QPoint> triangle = {{0, 0}, {21600, 0}, {10800, 21600}};
        // Vertices alone: a closed polygon, scaled from the 21600 square to the frame.
        QCOMPARE(path({{0x0145, 0, odVertices(triangle)}}), QStringLiteral("M0,0 L100,0 L50,50 L0,0"));
        QCOMPARE(path({{0x0145, 0, odVertices(triangle)}, {0x0144, 0}}), QStringLiteral("M0,0 L100,0 L50,50"));       // an open one
        QCOMPARE(path({{0x0145, 0, odVertices(triangle)}, {0x0144, 1}}), QStringLiteral("M0,0 L100,0 L50,50 L0,0"));
        // Another geometry rectangle.
        QCOMPARE(path({{0x0145, 0, odVertices({{1000, 2000}, {3000, 2000}, {2000, 4000}})}, {0x0140, 1000}, {0x0141, 2000}, {0x0142, 3000}, {0x0143, 4000}}),
                 QStringLiteral("M0,0 L100,0 L50,50 L0,0"));
        // Vertices of 16 bits.
        QCOMPARE(path({{0x0145, 0, odVertices({{0, 0}, {21600, 0}, {10800, 21600}}, 4)}}), QStringLiteral("M0,0 L100,0 L50,50 L0,0"));
        QCOMPARE(path({{0x0145, 0, odVertices({{-100, 0}, {100, 100}}, 4)}, {0x0140, quint32(-100)}, {0x0142, 100}, {0x0143, 100}}), QStringLiteral("M0,0 L100,50 L0,0"));
        // Curves in threes.
        QCOMPARE(path({{0x0145, 0, odVertices({{0, 0}, {10800, 0}, {21600, 10800}, {21600, 21600}})}, {0x0144, 2}}), QStringLiteral("M0,0 C50,0 100,25 100,50"));
        // Segments: move, two lines, a curve, close, end.
        const QVector<QPoint> six = {{0, 0}, {10800, 0}, {10800, 10800}, {21600, 10800}, {21600, 21600}, {10800, 21600}};
        QCOMPARE(path({{0x0145, 0, odVertices(six)}, {0x0146, 0, odSegments({0x4000, 0x0002, 0x2001, 0x6001, 0x8000})}, {0x0144, 4}}),
                 QStringLiteral("M0,0 L50,0 L50,25 C100,25 100,50 50,50 L0,0"));
        // Several subpaths, and the marks for no fill or no line.
        QCOMPARE(path({{0x0145, 0, odVertices({{0, 0}, {21600, 0}, {0, 10800}, {21600, 10800}})}, {0x0146, 0, odSegments({0x4000, 0xAB00, 0x0001, 0x8000, 0x4000, 0x0001, 0x8000})}}),
                 QStringLiteral("M0,0 L100,0 M0,25 L100,25"));
        // The marks for the kind of point (here auto, smooth, and symmetric curves) between segments take no vertices.
        QCOMPARE(path({{0x0145, 0, odVertices({{0, 0}, {10800, 0}, {21600, 10800}, {21600, 21600}, {0, 21600}})}, {0x0146, 0, odSegments({0x4000, 0xB300, 0x2001, 0xAD00, 0xB100, 0x0001, 0x8000})}}),
                 QStringLiteral("M0,0 C50,0 100,25 100,50 L0,50"));
        // A length that leaves the array's header out of its count.
        const QVector<OdProp> trimmed = {{0x0145, 0, odVertices(triangle), false, 6}, {0x0380, 0, odUtf16z(QStringLiteral("after"))}};
        const ItemList items = readOdrawShapes(odDrawing(odShape(0, 0xA00, trimmed, odClientAnchor(0, 0, 100, 50))), odContext());
        QCOMPARE(odPathText(odShapeOf(items, 0)->customPath), QStringLiteral("M0,0 L100,0 L50,50 L0,0"));
        QCOMPARE(odShapeOf(items, 0)->name, QStringLiteral("after"));
        // Quarters of an ellipse, starting along the width and along the height.
        const QVector<QPoint> two = {{0, 0}, {21600, 21600}};
        QCOMPARE(path({{0x0145, 0, odVertices(two)}, {0x0146, 0, odSegments({0x4000, 0xA701, 0x8000})}}), QStringLiteral("M0,0 C55.2285,0 100,22.3858 100,50"));
        QCOMPARE(path({{0x0145, 0, odVertices(two)}, {0x0146, 0, odSegments({0x4000, 0xA801, 0x8000})}}), QStringLiteral("M0,0 C0,27.6142 44.7715,50 100,50"));
        // Not enough vertices for the segments: what there is, and no more. One vertex is no path.
        QCOMPARE(path({{0x0145, 0, odVertices(triangle)}, {0x0146, 0, odSegments({0x4000, 0x0009, 0x6001})}}), QStringLiteral("M0,0 L100,0 L50,50"));
        QVERIFY(odShapeOf(readOdrawShapes(odDrawing(odShape(0, 0xA00, {{0x0145, 0, odVertices({{5, 5}})}}, odClientAnchor(0, 0, 100, 50))), odContext()), 0)->customPath.isEmpty());
        // A preset keeps its form when there are no vertices; one with them takes theirs.
        QVERIFY(odShapeOf(readOdrawShapes(odDrawing(odShape(3, 0xA00, {}, odClientAnchor(0, 0, 100, 50))), odContext()), 0)->customPath.isEmpty());
        QCOMPARE(path({{0x0145, 0, odVertices(triangle)}}, 3), QStringLiteral("M0,0 L100,0 L50,50 L0,0"));
    }

    void odrawReadsTextShapesAsTextArt()
    {
        const QByteArray at = odClientAnchor(0, 0, 200, 50);
        auto art = [&](int type, const QVector<OdProp> &props) {
            const ItemList items = readOdrawShapes(odDrawing(odShape(type, 0xA00, props, at)), odContext());
            return items.empty() || items[0]->type() != ItemType::TextArt ? std::shared_ptr<TextArtItem>() : std::static_pointer_cast<TextArtItem>(items[0]);
        };
        const OdProp words{0x00C0, 0, odUtf16z(QStringLiteral("Big\rNews"))};
        std::shared_ptr<TextArtItem> t = art(136, {words});
        QVERIFY(t);
        QCOMPARE(t->text, QStringLiteral("Big\nNews"));
        QCOMPARE(t->transform_, QStringLiteral("plain"));
        QCOMPARE(t->size, 36.0);
        QCOMPARE(t->align, 1);
        QVERIFY(!t->bold && !t->italic && !t->vertical && !t->evenHeight);
        QCOMPARE(odRgb(t->fill.color), QColor(Qt::white));   // the format's fill with nothing said
        t = art(144, {words, {0x00C5, 0, odUtf16z(QStringLiteral("Arial"))}, {0x00C3, 24u << 16}, {0x00C4, 0x18000}, {0x00C2, 2}, {0x00FF, 0x00300030u}, {0x0181, 0x0000FF}});
        QCOMPARE(t->transform_, QStringLiteral("archUp"));
        QCOMPARE(t->font, QStringLiteral("Arial"));
        QCOMPARE(t->size, 24.0);
        QCOMPARE(t->spacing, 1.5);
        QCOMPARE(t->align, 0);
        QVERIFY(t->bold && t->italic && !t->evenHeight && !t->vertical);
        QCOMPARE(odRgb(t->fill.color), QColor(255, 0, 0));
        t = art(175, {words, {0x00FF, 0x20802080u}});
        QVERIFY(t->vertical && t->evenHeight && !t->bold);
        // Without words it is the shape it names.
        const ItemList items = readOdrawShapes(odDrawing(odShape(136, 0xA00, {}, at)), odContext());
        QVERIFY(items[0]->type() == ItemType::Shape);
    }

    void odrawSurvivesDamagedDrawings()
    {
        // A drawing with a bit of everything, cut anywhere and mangled in many ways.
        OdrawContext ctx = odContext();
        ctx.client = [](jp::Item &item, const QByteArray &box, const QByteArray &data) { item.role = QString::number(box.size() + data.size()); };
        const QByteArray at = odClientAnchor(0, 0, 100, 50);
        const QByteArray text = odRecord(0, 0xF, 0xF00D, QByteArray("hello")) + odRecord(0, 0, 0xF011, QByteArray("cd"));
        QByteArray list;
        list += odU32(0x00FF0000) + odU32(65536) + odU32(0x000000FF) + odU32(0);
        const QByteArray inner = odGroup(QRect(0, 0, 100, 100), 0x2, odChildAnchor(0, 0, 500, 500), odShape(5, 0xA02, {{0x0147, 5400}}, odChildAnchor(0, 0, 50, 50)));
        const QByteArray shapes =
            odShape(1, 0xA00, {{0x0181, 0x08000003}, {0x0382, 0, odUrlHyperlink(QStringLiteral("http://example.com/a"), QStringLiteral("p"), false)}, {0x0380, 0, odUtf16z(QStringLiteral("n"))}, {0x0004, 30u << 16}}, at) +
            odShape(0, 0xA00, {{0x0145, 0, odVertices({{0, 0}, {10800, 0}, {21600, 10800}, {21600, 21600}, {0, 21600}})}, {0x0146, 0, odSegments({0x4000, 0xB300, 0x2001, 0x0001, 0x6001, 0x8000})}}, at) +
            odShape(202, 0xA00, {{0x0081, 127000}, {0x0087, 2}}, at, text) +
            odShape(75, 0xA00, {{0x0104, 2, {}, true}, {0x0100, 0x2000}, {0x0102, 0x2000}}, at) +
            odShape(3, 0xA00, {{0x0180, 7}, {0x0197, 0, odArray(2, 8, list)}, {0x01D0, 1}}, at) +
            odShape(20, 0xA40, {}, at) + odShape(34, 0xA00, {{0x0147, 5400}}, at) +
            odGroup(QRect(0, 0, 1000, 1000), 0x40, odClientAnchor(0, 0, 100, 100), odShape(1, 0xA02, {}, odChildAnchor(0, 0, 500, 500)) + inner, {{0x0004, 90u << 16}});
        const QByteArray drawing = odDrawing(shapes, odShape(1, 0xC00, {{0x0181, 0x00FF8000}}));
        QVERIFY(readOdrawShapes(drawing, ctx).size() >= 7);
        for (int cut = 0; cut <= drawing.size(); ++cut) {
            readOdrawShapes(drawing.left(cut), ctx);
            odrawBackground(drawing.left(cut), ctx);
        }
        QRandomGenerator g(3);
        const quint32 extremes[] = {0xFFFFFFFFu, 0x80000000u, 0x7FFFFFFFu, 0, 1, 0x00010000u};
        for (int round = 0; round < 4000; ++round) {
            QByteArray b = drawing;
            const int n = 1 + int(g.bounded(8u));
            for (int k = 0; k < n; ++k) b[int(g.bounded(quint32(b.size())))] = char(g.generate());
            if (round % 3 == 0) qToLittleEndian<quint32>(extremes[g.bounded(6u)], b.data() + int(g.bounded(quint32(b.size() - 4))));
            readOdrawShapes(b, ctx);
            odrawBackground(b, ctx);
        }
        // Not drawings at all.
        for (int round = 0; round < 200; ++round) {
            QByteArray b(int(g.bounded(400u)), '\0');
            for (char &c : b) c = char(g.generate());
            readOdrawShapes(b, ctx);
            odrawBackground(b, ctx);
            odrawRecords(b);
        }
    }

    void odrawBoundsDepthCountsAndSizes()
    {
        const OdrawContext ctx = odContext();
        const QByteArray at = odClientAnchor(0, 0, 10, 10);
        // Groups inside groups: as deep as 64 are read, and nothing deeper (or its stack) is trusted.
        auto nested = [&](int levels) {
            QByteArray g = odShape(1, 0xA02, {}, odChildAnchor(0, 0, 10, 10));
            for (int i = 0; i < levels; ++i) g = odGroup(QRect(0, 0, 10, 10), 0x2, odChildAnchor(0, 0, 10, 10), g);
            return readOdrawShapes(odDrawing(g), ctx);
        };
        int depth = 0;
        ItemList items = nested(60);
        QCOMPARE(int(items.size()), 1);
        for (const jp::Item *it = items[0].get(); it->type() == ItemType::Group; it = static_cast<const GroupItem *>(it)->children[0].get()) ++depth;
        QCOMPARE(depth, 60);
        QVERIFY(nested(100).empty());
        QVERIFY(nested(3000).empty());
        // More shapes than anyone has: the first 100,000.
        QByteArray many;
        const QByteArray one = odShape(1, 0xA00, {}, at);
        for (int i = 0; i < 100100; ++i) many += one;
        QCOMPARE(int(readOdrawShapes(odDrawing(many), ctx).size()), 100000);
        // Lengths and counts that promise more than the data holds.
        QByteArray shape = odShape(0, 0xA00, {{0x0145, 0, odVertices({{0, 0}, {5, 5}})}, {0x0146, 0, odSegments({0x0001})}}, at);
        auto tryShape = [&](const QByteArray &sp) { return readOdrawShapes(odDrawing(sp), ctx); };
        QByteArray big = odRecord(3, 0xFFF, 0xF00B, QByteArray(20, '\x11'));              // 4095 properties in 20 bytes
        QCOMPARE(int(tryShape(odContainer(0xF004, odFsp(1, 0xA00) + big + at)).size()), 1);
        big = odRecord(3, 1, 0xF00B, odU16(0x8000 | 0x0145) + odU32(0xFFFFFFF0u) + QByteArray(4, '\0'));   // complex data of 4 GB
        QCOMPARE(int(tryShape(odContainer(0xF004, odFsp(0, 0xA00) + big + at)).size()), 1);
        const QByteArray manyVertices = odArray(65535, 8, QByteArray(16, '\x07'));              // 65535 vertices of which two are there
        QVERIFY(tryShape(odShape(0, 0xA00, {{0x0145, 0, manyVertices}}, at)).size() == 1);
        QByteArray segs;
        for (int i = 0; i < 20000; ++i) segs += odU16(0x1FFF);                                   // 20000 runs of 8191 lines, over three vertices
        QVERIFY(tryShape(odShape(0, 0xA00, {{0x0145, 0, odVertices({{0, 0}, {1, 1}, {2, 2}})}, {0x0146, 0, odArray(20000, 2, segs)}}, at)).size() == 1);
        QVERIFY(tryShape(odShape(3, 0xA00, {{0x0180, 7}, {0x0197, 0, odArray(65535, 8, QByteArray(24, '\x03'))}}, at)).size() == 1);
        for (quint32 length : {0xFFFFFFFFu, 0x80000000u, 0x7FFFFFFFu}) {
            QByteArray d = odDrawing(shape);
            for (int at = 0; at + 4 <= d.size(); at += 2) {
                QByteArray b = d;
                qToLittleEndian<quint32>(length, b.data() + at);
                readOdrawShapes(b, ctx);
            }
        }
        // A picture promising a metafile of 4 GB, and a store of ten million empty blocks (kept to what fits).
        QVERIFY(odrawBlip(odRecord(0, 0x3D4, 0xF01A, odUid('m') + odU32(0xFFFFFFFFu) + QByteArray(24, '\0') + odU32(0xFFFFFFFFu) + QByteArray("\x00\xFE", 2) + QByteArray(10, 'x')), nullptr).isEmpty());
        QByteArray store;
        for (int i = 0; i < 120000; ++i) store += odRecord(0, 0, 0xF007, QByteArray());
        QVERIFY(odrawStore(odContainer(0xF001, store)).size() <= 100000);
        QCOMPARE(int(odrawRecords(QByteArray(9, '\xFF')).size()), 0);
    }

    // A host that reads many drawings gives them one budget: the shapes are read until it is spent (and the records looked at to find a
    // background count too), and it says when a drawing had more, so one drawing that a file has read again and again, or many large
    // ones, are read to a bound whatever their number.
    void odrawKeepsOneBudgetForManyDrawings()
    {
        OdrawContext ctx = odContext();
        const QByteArray at = odClientAnchor(0, 0, 10, 10);
        QByteArray shapes;
        for (int i = 0; i < 100; ++i) shapes += odShape(1, 0xA00, {}, at);
        const QByteArray drawing = odDrawing(shapes);
        OdrawBudget budget;
        budget.left = 250;
        ctx.budget = &budget;
        QCOMPARE(int(readOdrawShapes(drawing, ctx).size()), 100);
        QCOMPARE(int(readOdrawShapes(drawing, ctx).size()), 100);
        QCOMPARE(budget.left, qint64(50));
        QVERIFY(!budget.refused);
        QCOMPARE(int(readOdrawShapes(drawing, ctx).size()), 50);
        QVERIFY(budget.refused);
        QCOMPARE(int(readOdrawShapes(drawing, ctx).size()), 0);
        // Groups cost one each, and what is in them.
        budget = OdrawBudget();
        budget.left = 10;
        QByteArray inner;
        for (int i = 0; i < 20; ++i) inner += odShape(1, 0xA02, {}, odChildAnchor(0, 0, 5, 5));
        const ItemList grouped = readOdrawShapes(odDrawing(odGroup(QRect(0, 0, 10, 10), 0x2, odChildAnchor(0, 0, 10, 10), inner)), ctx);
        QCOMPARE(int(grouped.size()), 1);
        QCOMPARE(int(static_cast<const GroupItem *>(grouped[0].get())->children.size()), 9);
        QVERIFY(budget.refused);
        // A drawing that fits the budget exactly is not a refusal.
        budget = OdrawBudget();
        budget.left = 100;
        QCOMPARE(int(readOdrawShapes(drawing, ctx).size()), 100);
        QVERIFY(!budget.refused);
        // A background is looked for as far as the budget goes: the 100 shapes before it (in their group) use it up, and it is not found;
        // with the budget it needs, it is; and with none given, the same.
        const QByteArray withBackground = odDrawing(shapes, odShape(1, 0xC00, {{0x0181, 0x00FF8000}}));
        budget = OdrawBudget();
        budget.left = 50;
        QVERIFY(odrawBackground(withBackground, ctx).isNone());
        QVERIFY(budget.refused);
        budget = OdrawBudget();
        budget.left = 1000;
        QVERIFY(!odrawBackground(withBackground, ctx).isNone());
        QVERIFY(!budget.refused);
        ctx.budget = nullptr;
        QVERIFY(!odrawBackground(withBackground, ctx).isNone());
        QCOMPARE(int(readOdrawShapes(drawing, ctx).size()), 100);
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

    // The whole of an import draws from one budget: what two reads keep between them is what one would.
    void dmlSharedBudgetHoldsWhatManyReadsKeep()
    {
        DmlEnv env;
        DrawingBudget budget;
        budget.left[DrawingBudget::Objects] = 10;
        env.ctx.budget = &budget;
        const QString leaf = dmlSp(dmlXfrm(0, 0, 10, 10) + QStringLiteral("<a:prstGeom prst=\"rect\"/>"));
        QString six;
        for (int i = 0; i < 6; ++i) six += leaf;
        auto count = [&](const ItemList &list) {
            int n = 0;
            jp::walkItems(list, [&](const ItemPtr &) { ++n; });
            return n;
        };
        QCOMPARE(count(dmlTree(env, six)), 6);
        QVERIFY(!budget.ranOut[DrawingBudget::Objects]);
        QCOMPARE(count(dmlTree(env, six)), 4);   // the second read gets what is left
        QVERIFY(budget.ranOut[DrawingBudget::Objects]);
        QCOMPARE(budget.left[DrawingBudget::Objects], qint64(0));
        QCOMPARE(count(dmlTree(env, six)), 0);
        QVERIFY(!dmlOne(env, leaf));
        // A group costs one, and what is in it one each.
        DmlEnv again;
        DrawingBudget small;
        small.left[DrawingBudget::Objects] = 3;
        again.ctx.budget = &small;
        const QString frame = dmlGroupFrame(QRectF(0, 0, 100, 100), QRectF(0, 0, 100, 100));
        const ItemList grouped = dmlTree(again, dmlGrp(frame, leaf + leaf + leaf));
        QCOMPARE(count(grouped), 3);   // the group, and two of its three
        QVERIFY(small.ranOut[DrawingBudget::Objects]);
        // With no budget a read has only its own limits.
        DmlEnv free;
        QCOMPARE(count(dmlTree(free, six + six + six)), 18);
    }

    // A text box or a shape's text is a story; past the budget's stories a box is left out, and a shape stays without its text.
    void dmlSharedBudgetHoldsStoriesAndCells()
    {
        DmlEnv env;
        DrawingBudget budget;
        budget.left[DrawingBudget::Objects] = 100;
        budget.left[DrawingBudget::Stories] = 2;
        env.ctx.budget = &budget;
        const QString box = dmlSp(dmlXfrm(0, 0, 100, 30) + QStringLiteral("<a:prstGeom prst=\"rect\"/>"), dmlBody(), QString(), QStringLiteral("txBox=\"1\""));
        const QString ellipse = dmlSp(dmlXfrm(0, 0, 100, 30) + QStringLiteral("<a:prstGeom prst=\"ellipse\"/>"), dmlBody());
        const ItemList boxes = dmlTree(env, box + box + box + box);
        QCOMPARE(int(boxes.size()), 2);
        QCOMPARE(int(env.stories.size()), 2);
        QVERIFY(budget.ranOut[DrawingBudget::Stories]);
        const ItemList shapes = dmlTree(env, ellipse);
        QCOMPARE(int(shapes.size()), 1);
        QVERIFY(static_cast<ShapeItem &>(*shapes[0]).storyId.isEmpty());
        QCOMPARE(int(env.stories.size()), 2);

        // A table takes a cell each; one that would pass the budget is left to the caller.
        DmlEnv tables;
        DrawingBudget cells;
        cells.left[DrawingBudget::Cells] = 7;
        tables.ctx.budget = &cells;
        const QString grid = QStringLiteral("<a:tblGrid><a:gridCol w=\"%1\"/><a:gridCol w=\"%1\"/><a:gridCol w=\"%1\"/></a:tblGrid>").arg(emu(50));
        auto row = [&] { return QStringLiteral("<a:tr h=\"%1\">%2%2%2</a:tr>").arg(emu(20), dmlTc(QStringLiteral("x"))); };
        const QString twoRows = dmlTableFrame(QStringLiteral("<a:tblPr/>"), grid, row() + row(), 0, 0, 150, 40);
        QVERIFY(!readTable(tables.element(twoRows), tables.ctx));   // 6 cells and the table's own two, 7 left
        QVERIFY(cells.ranOut[DrawingBudget::Cells]);
        QCOMPARE(cells.left[DrawingBudget::Cells], qint64(7));
        QCOMPARE(int(tables.stories.size()), 0);
        const QString oneRow = dmlTableFrame(QStringLiteral("<a:tblPr/>"), grid, row(), 0, 0, 150, 20);
        QVERIFY(readTable(tables.element(oneRow), tables.ctx));
        QCOMPARE(cells.left[DrawingBudget::Cells], qint64(2));   // 3 cells and the two
        QCOMPARE(int(tables.stories.size()), 3);
    }

    // The paragraphs, runs, and letters of every text body of an import come out of one budget.
    void dmlSharedBudgetHoldsText()
    {
        auto read = [](DrawingBudget *budget, const QString &paragraphs) {
            QDomDocument dom;
            const QString xml = QStringLiteral("<p:txBody %1><a:bodyPr/><a:lstStyle/>%2</p:txBody>").arg(dmlNamespaces(), paragraphs);
            if (!parseXml(xml.toUtf8(), &dom)) qWarning("bad test XML");
            DrawingContext ctx;
            ctx.budget = budget;
            auto doc = std::make_unique<QTextDocument>();
            readTextBody(dom.documentElement(), doc.get(), ctx);
            return doc;
        };
        auto paragraph = [](const QString &text) { return QStringLiteral("<a:p><a:r><a:t>%1</a:t></a:r></a:p>").arg(text); };
        DrawingBudget budget;
        budget.left[DrawingBudget::Paragraphs] = 5;
        budget.left[DrawingBudget::Runs] = 1000;
        budget.left[DrawingBudget::Letters] = 1000;
        QString eight;
        for (int i = 0; i < 8; ++i) eight += paragraph(QStringLiteral("p%1").arg(i));
        QCOMPARE(read(&budget, eight)->blockCount(), 5);
        QVERIFY(budget.ranOut[DrawingBudget::Paragraphs]);
        // What is left when a body is read again is none: the body is one empty paragraph.
        QCOMPARE(read(&budget, eight)->toPlainText(), QString());
        QCOMPARE(read(&budget, eight)->blockCount(), 1);

        DrawingBudget letters;
        letters.left[DrawingBudget::Paragraphs] = 100;
        letters.left[DrawingBudget::Runs] = 100;
        letters.left[DrawingBudget::Letters] = 7;
        const auto cut = read(&letters, paragraph(QStringLiteral("abcd")) + paragraph(QStringLiteral("efgh")) + paragraph(QStringLiteral("ijkl")));
        QCOMPARE(cut->toPlainText().remove(QLatin1Char('\n')).remove(QChar::ParagraphSeparator), QStringLiteral("abcdefg"));
        QVERIFY(letters.ranOut[DrawingBudget::Letters]);
        QCOMPARE(letters.left[DrawingBudget::Letters], qint64(0));

        DrawingBudget runs;
        runs.left[DrawingBudget::Paragraphs] = 100;
        runs.left[DrawingBudget::Runs] = 4;
        runs.left[DrawingBudget::Letters] = 100;
        QString six;
        for (int i = 0; i < 6; ++i) six += QStringLiteral("<a:r><a:t>a</a:t></a:r>");
        QCOMPARE(read(&runs, QStringLiteral("<a:p>%1</a:p>").arg(six))->toPlainText(), QStringLiteral("aaaa"));
        QVERIFY(runs.ranOut[DrawingBudget::Runs]);
    }

    // One run longer than a body may be is cut where the body ends (real bodies have a few thousand letters at most).
    void dmlTextBodyCutsARunPastTheLettersOfABody()
    {
        QDomDocument dom;
        const QString xml = QStringLiteral("<p:txBody %1><a:bodyPr/><a:lstStyle/><a:p><a:r><a:t>%2</a:t></a:r></a:p></p:txBody>").arg(dmlNamespaces(), QString(2000100, QLatin1Char('z')));
        QVERIFY(parseXml(xml.toUtf8(), &dom));
        QTextDocument doc;
        readTextBody(dom.documentElement(), &doc, DrawingContext());
        QCOMPARE(doc.characterCount() - 1, 2000000);
        // With a budget, which has plenty left, it says that a body was cut, and that nothing ran out.
        DrawingBudget budget;
        for (qint64 &left : budget.left) left = 100000000;
        DrawingContext ctx;
        ctx.budget = &budget;
        readTextBody(dom.documentElement(), &doc, ctx);
        QCOMPARE(doc.characterCount() - 1, 2000000);
        QVERIFY(budget.longBody);
        for (bool out : budget.ranOut) QVERIFY(!out);
        QCOMPARE(budget.left[DrawingBudget::Letters], qint64(100000000 - 2000000));
    }

    // Qt adds a block to a list in time proportional to the list's length, so one long bulleted run is read as lists of a few hundred each,
    // numbered on from one to the next.
    void dmlBulletedParagraphsAreReadInLinearTime()
    {
        auto read = [](const QString &pPr, int count) {
            QString paragraphs;
            for (int i = 0; i < count; ++i) paragraphs += QStringLiteral("<a:p><a:pPr marL=\"285750\" indent=\"-285750\">%1</a:pPr><a:r><a:t>x</a:t></a:r></a:p>").arg(pPr);
            QDomDocument dom;
            const QString xml = QStringLiteral("<p:txBody %1><a:bodyPr/><a:lstStyle/>%2</p:txBody>").arg(dmlNamespaces(), paragraphs);
            if (!parseXml(xml.toUtf8(), &dom)) qWarning("bad test XML");
            auto doc = std::make_unique<QTextDocument>();
            readTextBody(dom.documentElement(), doc.get(), DrawingContext());
            return doc;
        };
        QElapsedTimer timer;
        timer.start();
        const auto bullets = read(QStringLiteral("<a:buChar char=\"&#8226;\"/>"), 10000);
        const qint64 ms = timer.elapsed();
        QCOMPARE(bullets->blockCount(), 10000);
        QHash<QTextList *, int> lists;
        for (QTextBlock b = bullets->begin(); b.isValid(); b = b.next()) {
            QVERIFY(b.textList());
            ++lists[b.textList()];
        }
        int longest = 0;
        for (int blocks : std::as_const(lists)) longest = std::max(longest, blocks);
        QVERIFY2(longest <= 500 && lists.size() >= 20, qPrintable(QStringLiteral("%1 lists, the longest of %2").arg(lists.size()).arg(longest)));
        QVERIFY2(ms < 3000, qPrintable(QStringLiteral("10,000 bulleted paragraphs took %1 ms").arg(ms)));
        // Counted on across the lists they are split into.
        const auto numbered = read(QStringLiteral("<a:buAutoNum type=\"arabicPeriod\"/>"), 1300);
        QCOMPARE(numbered->blockCount(), 1300);
        QCOMPARE(numbered->begin().textList()->itemText(numbered->begin()), QStringLiteral("1."));
        const QTextBlock last = numbered->findBlockByNumber(1299);
        QCOMPARE(last.textList()->itemText(last), QStringLiteral("1300."));
        const QTextBlock middle = numbered->findBlockByNumber(700);
        QCOMPARE(middle.textList()->itemText(middle), QStringLiteral("701."));
        const auto from5 = read(QStringLiteral("<a:buAutoNum type=\"arabicPeriod\" startAt=\"5\"/>"), 1300);
        const QTextBlock end = from5->findBlockByNumber(1299);
        QCOMPARE(end.textList()->itemText(end), QStringLiteral("1304."));
    }

    // One cache of pictures across every read of a file: the same picture on the next shape, or the next page, is handed over once.
    void dmlPicturesAreKeptOnceAcrossReads()
    {
        DmlEnv env;
        env.usePackage();
        QHash<QString, QString> kept;
        env.ctx.images = &kept;
        QString pics;
        for (int i = 0; i < 5; ++i) pics += dmlPic(QString(), dmlXfrm(0, 0, 10, 10) + QStringLiteral("<a:prstGeom prst=\"rect\"/>"));
        for (int page = 0; page < 3; ++page) {
            const ItemList list = dmlTree(env, pics);
            QCOMPARE(int(list.size()), 5);
            QCOMPARE(static_cast<PictureItem &>(*list[4]).imageId, static_cast<PictureItem &>(*list[0]).imageId);
        }
        for (int page = 0; page < 3; ++page) QVERIFY(dmlOne(env, dmlPic(QString(), dmlXfrm(0, 0, 10, 10) + QStringLiteral("<a:prstGeom prst=\"rect\"/>"))));
        QCOMPARE(env.images.size(), 1);   // addImage was called once, for all 18
        QCOMPARE(kept.size(), 1);
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
        // Text goes in the box, not on the arrow.
        QVERIFY2(dmlRectIs(right.textRect, 0, 0, 200 * 0.64977, 120, 1e-4), qPrintable(dmlRectText(right.textRect)));
        const Geometry left = presetGeometry(QStringLiteral("leftArrowCallout"), {}, QSizeF(200, 120));
        QVERIFY2(dmlRectIs(left.textRect, 200 * (1 - 0.64977), 0, 200 * 0.64977, 120, 1e-4), qPrintable(dmlRectText(left.textRect)));
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

    // A blur, glow, or soft edge is held to 200 points: real presentations use 20,
    // 10, and 7 at most, and one of the standard's 1,584 points makes a small
    // shape take most of a minute and gigabytes to draw.
    void dmlEffectRadiiStopAtTwoHundredPoints()
    {
        DmlEnv env;
        const QString rect = dmlXfrm(0, 0, 100, 50) + QStringLiteral("<a:prstGeom prst=\"rect\"/>");
        ItemPtr it = dmlOne(env, dmlSp(rect + QStringLiteral("<a:effectLst><a:outerShdw blurRad=\"20116800\" dist=\"0\" dir=\"0\"><a:srgbClr val=\"000000\"/></a:outerShdw>"
                                                             "<a:glow rad=\"20116800\"><a:srgbClr val=\"00FF00\"/></a:glow><a:softEdge rad=\"20116800\"/></a:effectLst>")));
        QVERIFY(it->fx.shadow.on && it->fx.glow.on);
        QVERIFY(near(it->fx.shadow.blur, 200) && near(it->fx.glow.size, 200) && near(it->fx.softEdge, 200));
        it = dmlOne(env, dmlSp(rect + QStringLiteral("<a:effectLst><a:innerShdw blurRad=\"20116800\" dist=\"0\" dir=\"0\"><a:srgbClr val=\"000000\"/></a:innerShdw></a:effectLst>")));
        QVERIFY(it->fx.shadow.inner && near(it->fx.shadow.blur, 200));
        // The largest radii the reference decks have are kept as they are.
        it = dmlOne(env, dmlSp(rect + QStringLiteral("<a:effectLst><a:outerShdw blurRad=\"254160\" dist=\"0\" dir=\"0\"><a:srgbClr val=\"000000\"/></a:outerShdw>"
                                                     "<a:glow rad=\"127000\"><a:srgbClr val=\"00FF00\"/></a:glow><a:softEdge rad=\"88900\"/></a:effectLst>")));
        QVERIFY(near(it->fx.shadow.blur, 20.01, 1e-2) && near(it->fx.glow.size, 10) && near(it->fx.softEdge, 7));
    }

    // A metafile in a picture record inflates to at most 32 MB (the reference
    // decks' largest is under 1 MB): a small stream that would give more is left
    // out, not inflated into memory.
    void odrawBlipLeavesOutMetafilesOverTheLimit()
    {
        QString format;
        const QByteArray small = odFakeEmf() + QByteArray(1024 * 1024, 'a');
        QCOMPARE(odrawBlip(odMetafileBlip(0xF01A, 0x3D4, small, true), &format), small);
        const QByteArray huge = odFakeEmf() + QByteArray(40 * 1024 * 1024, 'a');
        QVERIFY(odrawBlip(odMetafileBlip(0xF01A, 0x3D4, huge, true), &format).isEmpty());   // deflated: 40 KB
        QVERIFY(odrawBlip(odMetafileBlip(0xF01A, 0x3D4, huge, false), &format).isEmpty());  // stored
        QVERIFY(odrawBlip(odMetafileBlip(0xF01B, 0x216, huge, true), &format).isEmpty());
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

    // A background that refers to a picture style of the theme: the picture
    // is named by the THEME part's relationships, not the slide's, and its
    // a:duotone recolors it from the first color (black) to the second
    // (white).
    void dmlThemePictureBackgroundWithDuotone()
    {
        DmlEnv env;
        QImage source(2, 1, QImage::Format_RGB32);
        source.setPixel(0, 0, qRgb(0, 0, 0));
        source.setPixel(1, 0, qRgb(255, 255, 255));
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        source.save(&buffer, "PNG");
        PackageWriter w;
        w.addDefault(QStringLiteral("png"), QStringLiteral("image/png"));
        w.addPart(QStringLiteral("/ppt/slideMasters/slideMaster1.xml"), "<p:sldMaster/>", QStringLiteral("application/xml"));
        w.addPart(QStringLiteral("/ppt/theme/theme1.xml"), "<a:theme/>", QStringLiteral("application/xml"));
        w.addPart(QStringLiteral("/ppt/media/texture.png"), png, QStringLiteral("image/png"));
        w.addPart(QStringLiteral("/ppt/media/other.png"), dmlPicture(), QStringLiteral("image/png"));
        // rId1 means different pictures to the master and to the theme.
        w.addRelationship(QStringLiteral("/ppt/slideMasters/slideMaster1.xml"), kRelBase + "image", QStringLiteral("/ppt/media/other.png"));
        w.addRelationship(QStringLiteral("/ppt/theme/theme1.xml"), kRelBase + "image", QStringLiteral("/ppt/media/texture.png"));
        QVERIFY(env.package.read(w.finish()));
        env.ctx.package = &env.package;
        env.ctx.part = QStringLiteral("/ppt/slideMasters/slideMaster1.xml");
        env.ctx.themePart = QStringLiteral("/ppt/theme/theme1.xml");
        env.ctx.formats = env.element(QStringLiteral(
            "<a:fmtScheme><a:fillStyleLst/><a:lnStyleLst/><a:effectStyleLst/><a:bgFillStyleLst>"
            "<a:blipFill><a:blip r:embed=\"rId1\"><a:duotone><a:srgbClr val=\"000080\"/><a:schemeClr val=\"phClr\"/></a:duotone></a:blip><a:stretch/></a:blipFill>"
            "</a:bgFillStyleLst></a:fmtScheme>"));
        const Fill f = readBackground(env.element(QStringLiteral("<p:bg><p:bgRef idx=\"1001\"><a:srgbClr val=\"FFFF00\"/></p:bgRef></p:bg>")), env.ctx);
        QVERIFY(f.type == Fill::Picture);
        QCOMPARE(env.images.size(), 1);
        QImage got;
        QVERIFY(got.loadFromData(env.images[0].bytes));
        QCOMPARE(got.size(), QSize(2, 1));   // the theme's picture, not the master's
        QCOMPARE(QColor(got.pixel(0, 0)), QColor(0x00, 0x00, 0x80));   // black takes the first color
        QCOMPARE(QColor(got.pixel(1, 0)), QColor(0xFF, 0xFF, 0x00));   // white the second (here the reference's)
    }

    // A shape of a diagram's drawing (dsp:sp) is a p:sp with one more element, dsp:txXfrm: the rectangle its text is
    // laid out in, which need not be the shape's own, and which turns by its own angle on top of the shape's.
    static QString dspSp(const QString &spPr, const QString &txXfrm, const QString &text = QStringLiteral("Hello"), const QString &style = QString())
    {
        return QStringLiteral("<dsp:sp xmlns:dsp=\"http://schemas.microsoft.com/office/drawing/2008/diagram\" modelId=\"{1}\">"
                              "<dsp:nvSpPr><dsp:cNvPr id=\"0\" name=\"\"/><dsp:cNvSpPr/></dsp:nvSpPr><dsp:spPr>%1</dsp:spPr>%4"
                              "<dsp:txBody><a:bodyPr lIns=\"0\" tIns=\"0\" rIns=\"0\" bIns=\"0\" anchor=\"ctr\"><a:noAutofit/></a:bodyPr><a:lstStyle/>%2</dsp:txBody>%3</dsp:sp>")
            .arg(spPr, text.isEmpty() ? QStringLiteral("<a:p/>") : QStringLiteral("<a:p><a:r><a:t>%1</a:t></a:r></a:p>").arg(text), txXfrm, style);
    }

    void dmlDiagramShapeLaysItsTextInTheTextTransform()
    {
        DmlEnv env;
        const QString plain = dmlXfrm(10, 10, 200, 100) + QStringLiteral("<a:prstGeom prst=\"rect\"/>") + dmlSolid(QStringLiteral("336699"));
        // Text in the shape's own rectangle: the shape is its text, as a p:sp with a text body is.
        ItemPtr it = dmlOne(env, dspSp(plain, dmlXfrm(10, 10, 200, 100, QString(), "dsp:txXfrm")));
        auto t = dmlAs<TextItem>(it);
        QVERIFY(t);
        QCOMPARE(dmlHex(t->fill.color), QStringLiteral("336699"));
        QVERIFY(dmlRectIs(t->rect, 10, 10, 200, 100));
        QVERIFY(env.stories.last()->toPlainText().contains(QStringLiteral("Hello")));
        QVERIFY(t->valign == VAlign::Middle);
        QVERIFY(t->insets == QMarginsF(0, 0, 0, 0));
        // A rectangle for the text that is not the shape's: the shape draws, and its text is a box of its own, with neither fill nor line.
        const QString round = dmlXfrm(10, 10, 200, 100) + QStringLiteral("<a:prstGeom prst=\"roundRect\"/>") + dmlSolid(QStringLiteral("336699"))
                              + QStringLiteral("<a:ln w=\"12700\">%1</a:ln>").arg(dmlSolid(QStringLiteral("FFFFFF")));
        it = dmlOne(env, dspSp(round, dmlXfrm(30, 20, 160, 80, QString(), "dsp:txXfrm")));
        auto g = dmlAs<GroupItem>(it);
        QVERIFY(g);
        QCOMPARE(int(g->children.size()), 2);
        auto shape = dmlAs<ShapeItem>(g->children[0]);
        t = dmlAs<TextItem>(g->children[1]);
        QVERIFY(shape && t);
        QCOMPARE(shape->shape, QStringLiteral("roundRect"));
        QVERIFY(dmlRectIs(shape->rect, 10, 10, 200, 100));
        QCOMPARE(dmlHex(shape->fill.color), QStringLiteral("336699"));
        QVERIFY(shape->storyId.isEmpty());
        QVERIFY(dmlRectIs(t->rect, 30, 20, 160, 80));
        QVERIFY(t->fill.isNone() && t->stroke.isNone());
        QVERIFY(near(t->rotation, 0));
        QVERIFY(env.stories.last()->toPlainText().contains(QStringLiteral("Hello")));
        QVERIFY(dmlRectIs(g->rect, 10, 10, 200, 100));
        // The text turns with the shape: the same rectangle, the text's own angle added to the shape's.
        it = dmlOne(env, dspSp(dmlXfrm(0, 0, 100, 40, QStringLiteral("rot=\"16200000\"")) + QStringLiteral("<a:prstGeom prst=\"rect\"/>") + dmlSolid(QStringLiteral("336699")),
                               dmlXfrm(0, 0, 100, 40, QString(), "dsp:txXfrm")));
        t = dmlAs<TextItem>(it);
        QVERIFY(t);
        QVERIFY(near(t->rotation, 270));
        QVERIFY(dmlRectIs(t->rect, 0, 0, 100, 40));
        // A shape turned a quarter, its text turned back, and its text's rectangle off the shape's center: the center goes round with the shape.
        it = dmlOne(env, dspSp(dmlXfrm(0, 0, 100, 40, QStringLiteral("rot=\"5400000\"")) + QStringLiteral("<a:prstGeom prst=\"rightArrow\"/>") + dmlSolid(QStringLiteral("336699")),
                               dmlXfrm(0, 0, 60, 40, QStringLiteral("rot=\"-5400000\""), "dsp:txXfrm")));
        g = dmlAs<GroupItem>(it);
        QVERIFY(g && g->children.size() == 2);
        t = dmlAs<TextItem>(g->children[1]);
        QVERIFY(t);
        QVERIFY(near(t->rotation, 0));
        QVERIFY(dmlRectIs(t->rect, 20, -20, 60, 40));   // (30, 20) about the shape's center (50, 20) a quarter turn clockwise: (50, 0)
        QVERIFY(dmlAs<ShapeItem>(g->children[0]) && near(g->children[0]->rotation, 90));
        // A flip puts the rectangle on the other side of the shape's center.
        it = dmlOne(env, dspSp(dmlXfrm(0, 0, 100, 40, QStringLiteral("flipH=\"1\"")) + QStringLiteral("<a:prstGeom prst=\"rightArrow\"/>") + dmlSolid(QStringLiteral("336699")),
                               dmlXfrm(0, 0, 60, 40, QString(), "dsp:txXfrm")));
        g = dmlAs<GroupItem>(it);
        QVERIFY(g && g->children.size() == 2);
        QVERIFY(dmlRectIs(g->children[1]->rect, 40, 0, 60, 40));
        // No text, no text box; no rectangle for the text, the shape's own place for it.
        it = dmlOne(env, dspSp(round, dmlXfrm(30, 20, 160, 80, QString(), "dsp:txXfrm"), QString()));
        QVERIFY(dmlAs<ShapeItem>(it) && dmlAs<ShapeItem>(it)->storyId.isEmpty());
        it = dmlOne(env, dspSp(round, QString()));
        QVERIFY(dmlAs<ShapeItem>(it) && !dmlAs<ShapeItem>(it)->storyId.isEmpty());
        // The font of the shape's style gives the text its color, as it does a p:sp's.
        it = dmlOne(env, dspSp(plain, dmlXfrm(10, 10, 200, 100, QString(), "dsp:txXfrm"), QStringLiteral("Hello"),
                               QStringLiteral("<dsp:style><a:lnRef idx=\"0\"><a:scrgbClr r=\"0\" g=\"0\" b=\"0\"/></a:lnRef><a:fillRef idx=\"0\"><a:scrgbClr r=\"0\" g=\"0\" b=\"0\"/></a:fillRef>"
                                              "<a:effectRef idx=\"0\"><a:scrgbClr r=\"0\" g=\"0\" b=\"0\"/></a:effectRef><a:fontRef idx=\"minor\"><a:srgbClr val=\"FF0000\"/></a:fontRef></dsp:style>")));
        t = dmlAs<TextItem>(it);
        QVERIFY(t);
        QCOMPARE(env.stories.last()->begin().begin().fragment().charFormat().foreground().color(), QColor(0xFF, 0x00, 0x00));
    }

    // A picture a diagram's shape is filled with is cut to fill it by spreading it past the shape (a negative a:fillRect) or by cutting it (a:srcRect):
    // a fill has no such cut, so the shape is a picture, cut to the shape's outline, with the shape's line and text over it.
    void dmlDiagramShapeWithACutPictureIsAPictureUnderTheShape()
    {
        DmlEnv env;
        env.usePackage();
        auto picShape = [](const QString &blipFillInner, const QString &prst, const QString &txXfrm = QString(), const QString &text = QString()) {
            return dspSp(dmlXfrm(10, 10, 100, 50) + QStringLiteral("<a:prstGeom prst=\"%1\"/><a:blipFill><a:blip r:embed=\"rId1\"/>%2</a:blipFill><a:ln w=\"12700\">%3</a:ln>")
                                                        .arg(prst, blipFillInner, dmlSolid(QStringLiteral("FFFFFF"))),
                         txXfrm, text);
        };
        // Spread past the shape by a tenth of its width on each side: the picture is that much larger, and centered.
        ItemPtr it = dmlOne(env, picShape(QStringLiteral("<a:stretch><a:fillRect l=\"-10000\" r=\"-10000\"/></a:stretch>"), QStringLiteral("ellipse"), QString(), QString()));
        auto g = dmlAs<GroupItem>(it);
        QVERIFY(g && g->children.size() == 2);
        auto pic = dmlAs<PictureItem>(g->children[0]);
        auto shape = dmlAs<ShapeItem>(g->children[1]);
        QVERIFY(pic && shape);
        QCOMPARE(pic->maskShape, QStringLiteral("ellipse"));
        QVERIFY(dmlRectIs(pic->rect, 10, 10, 100, 50));
        QVERIFY(dmlRectIs(pic->imgRect, -10, 0, 120, 50));
        QVERIFY(!pic->imageId.isEmpty());
        QVERIFY(shape->fill.isNone());                  // the picture is no longer the shape's fill
        QVERIFY(near(shape->stroke.width, 1));          // its line stays, over the picture
        QCOMPARE(shape->shape, QStringLiteral("ellipse"));
        // Cut by a:srcRect, a quarter off the top and the bottom: the picture is twice as tall as the shape, spread evenly above and below.
        it = dmlOne(env, picShape(QStringLiteral("<a:srcRect t=\"25000\" b=\"25000\"/><a:stretch><a:fillRect/></a:stretch>"), QStringLiteral("rect")));
        g = dmlAs<GroupItem>(it);
        QVERIFY(g && g->children.size() == 2);
        pic = dmlAs<PictureItem>(g->children[0]);
        QVERIFY(pic);
        QVERIFY(dmlRectIs(pic->imgRect, 0, -25, 100, 100));
        // With text in a rectangle of its own, the text box comes with the shape, over the picture.
        it = dmlOne(env, picShape(QStringLiteral("<a:stretch><a:fillRect l=\"-10000\" r=\"-10000\"/></a:stretch>"), QStringLiteral("roundRect"), dmlXfrm(20, 20, 80, 30, QString(), "dsp:txXfrm"), QStringLiteral("Caption")));
        g = dmlAs<GroupItem>(it);
        QVERIFY(g && g->children.size() == 3);
        QVERIFY(dmlAs<PictureItem>(g->children[0]) && dmlAs<ShapeItem>(g->children[1]) && dmlAs<TextItem>(g->children[2]));
        QVERIFY(g->children[1]->fill.isNone());
        // A picture that fills it plainly stays a fill, and so does one in a shape drawn by its own paths.
        it = dmlOne(env, picShape(QStringLiteral("<a:stretch><a:fillRect/></a:stretch>"), QStringLiteral("rect")));
        QVERIFY(dmlAs<ShapeItem>(it) && dmlAs<ShapeItem>(it)->fill.type == Fill::Picture);
    }

    // A diagram's drawing is a tree of such shapes (dsp:spTree), with groups (dsp:grpSp) that place theirs as p:grpSp does.
    void dmlDiagramTreeReadsItsShapesAndGroups()
    {
        DmlEnv env;
        const QString dsp = QStringLiteral("xmlns:dsp=\"http://schemas.microsoft.com/office/drawing/2008/diagram\"");
        auto sp = [](double x, double y, const QString &text, double inset = 0) {
            return dspSp(dmlXfrm(x, y, 50, 20) + QStringLiteral("<a:prstGeom prst=\"rect\"/>") + dmlSolid(QStringLiteral("336699")),
                         dmlXfrm(x + inset, y + inset, 50 - 2 * inset, 20 - 2 * inset, QString(), "dsp:txXfrm"), text);
        };
        const QString tree = QStringLiteral("<dsp:spTree %1><dsp:nvGrpSpPr><dsp:cNvPr id=\"0\" name=\"\"/><dsp:cNvGrpSpPr/></dsp:nvGrpSpPr><dsp:grpSpPr/>%2%3"
                                            "<dsp:grpSp><dsp:nvGrpSpPr><dsp:cNvPr id=\"0\" name=\"\"/><dsp:cNvGrpSpPr/></dsp:nvGrpSpPr><dsp:grpSpPr><a:xfrm><a:off x=\"%4\" y=\"0\"/><a:ext cx=\"%5\" cy=\"%6\"/>"
                                            "<a:chOff x=\"0\" y=\"0\"/><a:chExt cx=\"%5\" cy=\"%6\"/></a:xfrm></dsp:grpSpPr>%7</dsp:grpSp></dsp:spTree>")
                                 .arg(dsp, sp(0, 0, QStringLiteral("One")), sp(60, 0, QStringLiteral("Two"), 2), emu(200), emu(50), emu(20), sp(0, 0, QStringLiteral("Inner")));
        const ItemList items = readShapeTree(env.element(tree), env.ctx);
        QCOMPARE(int(items.size()), 3);
        QVERIFY(dmlRectIs(items[0]->rect, 0, 0, 50, 20));
        // The second has a rectangle for its text inside it: a shape and its text box.
        auto second = dmlAs<GroupItem>(items[1]);
        QVERIFY(second && second->children.size() == 2);
        QVERIFY(dmlRectIs(second->rect, 60, 0, 50, 20));
        QVERIFY(dmlRectIs(second->children[1]->rect, 62, 2, 46, 16));
        auto group = dmlAs<GroupItem>(items[2]);
        QVERIFY(group && group->children.size() == 1);
        QVERIFY(dmlRectIs(group->children[0]->rect, 200, 0, 50, 20));
        QCOMPARE(env.stories.size(), 3);
    }

    // ---- DrawingML shapes: writing shapes (the counterpart of reading them)

private:
    // Items written as the shape tree of a slide part, then the package read
    // and the tree read back, with a record of what was written.
    struct WriteRound {
        PackageWriter writer;
        QHash<QString, QString> media;
        QMap<QString, QByteArray> pictures;                      // by image id
        QMap<QString, std::shared_ptr<QTextDocument>> stories;   // by story id
        DrawingWriteContext ctx;
        QStringList skipped;
        DmlEnv env;
        QByteArray tree;       // the shape tree as written
        QDomDocument dom;
        QDomElement spTree;
        ItemList items;        // what reading it gave
        QString error;

        WriteRound()
        {
            ctx.package = &writer;
            ctx.part = QStringLiteral("/ppt/slides/slide1.xml");
            ctx.theme = colorTestTheme();
            ctx.colors = jp::builtinColorSchemes().value(0);
            ctx.mediaParts = &media;
            ctx.imageBytes = [this](const QString &id, QString *format) {
                if (format) *format = QStringLiteral("png");
                return pictures.value(id);
            };
            ctx.storyDoc = [this](const QString &id) -> const QTextDocument * { return stories.value(id).get(); };
        }
        QString addPicture(const QColor &color = Qt::red)
        {
            const QString id = QStringLiteral("pic%1").arg(pictures.size() + 1);
            pictures.insert(id, dmlPicture(color));
            return id;
        }
        QString addStory(const QString &text = QString())
        {
            const QString id = QStringLiteral("st%1").arg(stories.size() + 1);
            auto doc = std::make_shared<QTextDocument>();
            doc->setPlainText(text);
            stories.insert(id, doc);
            return id;
        }
        QTextDocument *story(const QString &id) const { return stories.value(id).get(); }
        // The story an item read back holds.
        QTextDocument *storyOf(const ItemPtr &it) const
        {
            const QString id = it->type() == ItemType::Text ? static_cast<TextItem *>(it.get())->storyId : static_cast<ShapeItem *>(it.get())->storyId;
            const int n = id.mid(5).toInt();
            return n >= 1 && n <= env.stories.size() ? env.stories[n - 1].get() : nullptr;
        }
        bool runWith(const std::function<void(QXmlStreamWriter &)> &write)
        {
            QByteArray body;
            QXmlStreamWriter w(&body);
            w.writeStartElement(QStringLiteral("p:spTree"));
            write(w);
            w.writeEndElement();
            tree = body;
            const QByteArray part = "<p:sld " + dmlNamespaces().toUtf8() + "><p:cSld>" + body + "</p:cSld></p:sld>";
            writer.addPart(ctx.part, part, kSlideType);
            if (!env.package.read(writer.finish(), &error) || !parseXml(part, &dom, &error)) return false;
            env.ctx.package = &env.package;
            env.ctx.part = ctx.part;
            spTree = child(child(dom.documentElement(), QString(ns::P), QStringLiteral("cSld")), QString(ns::P), QStringLiteral("spTree"));
            return true;
        }
        bool run(const ItemList &list)
        {
            if (!runWith([&](QXmlStreamWriter &w) {
                    for (const ItemPtr &it : list) writeShape(w, *it, ctx, &skipped);
                }))
                return false;
            items = readShapeTree(spTree, env.ctx);
            return true;
        }
        QString text() const { return QString::fromUtf8(tree); }
    };

    struct Run {
        QString text;
        QTextCharFormat format;
    };
    static QVector<Run> runsOf(const QTextBlock &b)
    {
        QVector<Run> out;
        for (auto it = b.begin(); !it.atEnd(); ++it) out.append({it.fragment().text(), it.fragment().charFormat()});
        return out;
    }

    static bool sameRect(const QRectF &a, const QRectF &b, double tolerance = 1e-3)
    {
        return near(a.x(), b.x(), tolerance) && near(a.y(), b.y(), tolerance) && near(a.width(), b.width(), tolerance) && near(a.height(), b.height(), tolerance);
    }

    static QStringList childNamesOf(const QDomElement &e)
    {
        QStringList out;
        for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) out << c.localName();
        return out;
    }
    // Whether names come in the order of the schema's sequence (each at or
    // after the one before); an entry of the sequence may name alternatives, "a|b".
    static bool inSequence(const QStringList &names, const QStringList &sequence)
    {
        int at = 0;
        for (const QString &n : names) {
            int i = -1;
            for (int k = 0; k < sequence.size() && i < 0; ++k)
                if (sequence[k].split(QLatin1Char('|')).contains(n)) i = k;
            if (i < 0 || i < at) return false;
            at = i;
        }
        return true;
    }
    // Every element called `local` below `e`, in document order.
    static QVector<QDomElement> elementsNamed(const QDomElement &e, const char *local)
    {
        QVector<QDomElement> out;
        for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
            if (c.localName() == QLatin1String(local)) out << c;
            out += elementsNamed(c, local);
        }
        return out;
    }

    static std::shared_ptr<TextItem> writtenTextBox(WriteRound &r, const QRectF &rect, const QString &text)
    {
        auto t = std::make_shared<TextItem>();
        t->rect = rect;
        t->storyId = r.addStory(text);
        return t;
    }

private Q_SLOTS:
    void dmlWriteTextBoxKeepsItsFrameAndBody()
    {
        WriteRound r;
        auto t = writtenTextBox(r, QRectF(36.5, 40.25, 200, 80.5), QStringLiteral("Hello"));
        t->rotation = 30;
        t->flipH = true;
        t->name = QStringLiteral("Title box");
        t->altText = QStringLiteral("A <title> & \"more\"");
        t->fill = Fill::solid(ColorRef::rgb(QColor(0xFF, 0x88, 0x00)), 0.25);
        t->stroke = Stroke::line(ColorRef::rgb(QColor(0x11, 0x22, 0x33)), 2);
        t->insets = QMarginsF(3, 4, 5, 6);
        t->valign = jp::VAlign::Middle;
        t->autofit = TextItem::ShrinkOnOverflow;
        t->fitScale = 0.8;
        t->columns = 2;
        t->columnGap = 12;
        QVERIFY2(r.run({t}), qPrintable(r.error));
        QVERIFY2(r.skipped.isEmpty(), qPrintable(r.skipped.join('\n')));
        QCOMPARE(int(r.items.size()), 1);
        QCOMPARE(int(r.items[0]->type()), int(ItemType::Text));
        const auto *b = static_cast<TextItem *>(r.items[0].get());
        QVERIFY(sameRect(b->rect, t->rect));
        QVERIFY(near(b->rotation, 30, 1e-4));
        QVERIFY(b->flipH && !b->flipV);
        QCOMPARE(b->name, t->name);
        QCOMPARE(b->altText, t->altText);
        QVERIFY(b->fill.type == Fill::Solid);
        QCOMPARE(dmlHex(b->fill.color), QStringLiteral("FF8800"));
        QVERIFY(near(b->fill.transparency, 0.25, 1e-3));
        QVERIFY(near(b->stroke.width, 2));
        QCOMPARE(dmlHex(b->stroke.color), QStringLiteral("112233"));
        QCOMPARE(b->insets, QMarginsF(3, 4, 5, 6));
        QVERIFY(b->valign == jp::VAlign::Middle);
        QVERIFY(b->autofit == TextItem::ShrinkOnOverflow);
        QVERIFY(near(b->fitScale, 0.8, 1e-5));
        QCOMPARE(b->columns, 2);
        QVERIFY(near(b->columnGap, 12));
        QVERIFY(!b->vertical);
        QCOMPARE(r.storyOf(r.items[0])->toPlainText(), QStringLiteral("Hello"));
        // A plain text box says so (it is no placeholder), and has a name even where it had none.
        QVERIFY(r.text().contains(QStringLiteral("txBox=\"1\"")));
        WriteRound unnamed;
        auto u = writtenTextBox(unnamed, QRectF(0, 0, 10, 10), QStringLiteral("x"));
        QVERIFY2(unnamed.run({u}), qPrintable(unnamed.error));
        QVERIFY(!static_cast<TextItem *>(unnamed.items[0].get())->name.isEmpty());
        QVERIFY(!unnamed.text().contains(QStringLiteral("descr")));
    }

    void dmlWriteTextBodyKeepsAutofitAndDirection()
    {
        const struct { TextItem::Autofit autofit; const char *element; } fits[] = {
            {TextItem::NoAutofit, "noAutofit"}, {TextItem::ShrinkOnOverflow, "normAutofit"}, {TextItem::GrowBox, "spAutoFit"}};
        for (const auto &f : fits) {
            WriteRound r;
            auto t = writtenTextBox(r, QRectF(0, 0, 100, 50), QStringLiteral("x"));
            t->autofit = f.autofit;
            t->vertical = true;
            t->valign = jp::VAlign::Bottom;
            QVERIFY2(r.run({t}), qPrintable(r.error));
            const auto *b = static_cast<TextItem *>(r.items[0].get());
            QVERIFY2(b->autofit == f.autofit, f.element);
            QVERIFY(near(b->fitScale, 1.0));
            QVERIFY(b->vertical);
            QVERIFY(b->valign == jp::VAlign::Bottom);
            QCOMPARE(int(elementsNamed(r.spTree, f.element).size()), 1);
        }
    }

    void dmlWriteRunsKeepTheirLook()
    {
        WriteRound r;
        const QString id = r.addStory();
        QTextCursor c(r.story(id));
        auto put = [&](const QString &text, const std::function<void(QTextCharFormat &)> &set) {
            QTextCharFormat f;
            f.setFontPointSize(14);
            set(f);
            c.insertText(text, f);
        };
        put(QStringLiteral("plain "), [](QTextCharFormat &) {});
        put(QStringLiteral("styled "), [](QTextCharFormat &f) {
            f.setFontPointSize(20.5);
            f.setFontWeight(QFont::Bold);
            f.setFontItalic(true);
            f.setUnderlineStyle(QTextCharFormat::SingleUnderline);
            f.setFontStrikeOut(true);
            f.setForeground(QColor(0x11, 0x22, 0x33));
            f.setProperty(jp::tp::ColorRefP, QStringLiteral("#112233"));
        });
        put(QStringLiteral("doubled "), [](QTextCharFormat &f) {
            f.setUnderlineStyle(QTextCharFormat::SingleUnderline);
            f.setProperty(jp::tp::UnderlineKind, 1);
        });
        put(QStringLiteral("wavy "), [](QTextCharFormat &f) { f.setUnderlineStyle(QTextCharFormat::WaveUnderline); });
        put(QStringLiteral("up "), [](QTextCharFormat &f) { f.setVerticalAlignment(QTextCharFormat::AlignSuperScript); });
        put(QStringLiteral("down "), [](QTextCharFormat &f) { f.setVerticalAlignment(QTextCharFormat::AlignSubScript); });
        put(QStringLiteral("spaced "), [](QTextCharFormat &f) {
            f.setFontLetterSpacingType(QFont::AbsoluteSpacing);
            f.setFontLetterSpacing(1.5);
            f.setProperty(jp::tp::KernAbove, 12.0);
        });
        put(QStringLiteral("caps "), [](QTextCharFormat &f) { f.setFontCapitalization(QFont::AllUppercase); });
        put(QStringLiteral("small "), [](QTextCharFormat &f) { f.setFontCapitalization(QFont::SmallCaps); });
        put(QStringLiteral("face "), [](QTextCharFormat &f) { f.setFontFamilies({QStringLiteral("Carlito"), QStringLiteral("Noto Sans CJK JP")}); });
        put(QStringLiteral("heading "), [](QTextCharFormat &f) { f.setProperty(jp::tp::ThemeFont, QStringLiteral("major")); });
        put(QStringLiteral("body "), [](QTextCharFormat &f) { f.setProperty(jp::tp::ThemeFont, QStringLiteral("minor")); });
        put(QStringLiteral("marked "), [](QTextCharFormat &f) { f.setProperty(jp::tp::HighlightRefP, QStringLiteral("#FFFF00")); });
        put(QStringLiteral("french "), [](QTextCharFormat &f) {
            f.setProperty(jp::tp::Language, QStringLiteral("fr-FR"));
            f.setProperty(jp::tp::NoProof, true);
        });
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(0, 0, 400, 100);
        t->storyId = id;
        QVERIFY2(r.run({t}), qPrintable(r.error));
        const QVector<Run> runs = runsOf(r.storyOf(r.items[0])->firstBlock());
        QCOMPARE(int(runs.size()), 14);
        QStringList text;
        for (const Run &run : runs) text << run.text;
        QCOMPARE(text, (QStringList{"plain ", "styled ", "doubled ", "wavy ", "up ", "down ", "spaced ", "caps ", "small ", "face ", "heading ", "body ", "marked ", "french "}));
        for (const Run &run : runs) QVERIFY2(run.format.fontWeight() == QFont::Normal || run.text == QLatin1String("styled "), qPrintable(run.text));
        QVERIFY(near(runs[0].format.fontPointSize(), 14));
        QVERIFY(!runs[0].format.fontItalic() && !runs[0].format.fontStrikeOut());
        QVERIFY(runs[0].format.underlineStyle() == QTextCharFormat::NoUnderline);
        QVERIFY(near(runs[1].format.fontPointSize(), 20.5));
        QCOMPARE(int(runs[1].format.fontWeight()), int(QFont::Bold));
        QVERIFY(runs[1].format.fontItalic() && runs[1].format.fontStrikeOut());
        QVERIFY(runs[1].format.underlineStyle() == QTextCharFormat::SingleUnderline);
        QCOMPARE(runs[1].format.stringProperty(jp::tp::ColorRefP), QStringLiteral("#112233"));
        QCOMPARE(runs[1].format.foreground().color(), QColor(0x11, 0x22, 0x33));
        QVERIFY(runs[2].format.underlineStyle() == QTextCharFormat::SingleUnderline);
        QCOMPARE(runs[2].format.intProperty(jp::tp::UnderlineKind), 1);
        QVERIFY(runs[3].format.underlineStyle() == QTextCharFormat::WaveUnderline);
        QVERIFY(runs[4].format.verticalAlignment() == QTextCharFormat::AlignSuperScript);
        QVERIFY(runs[5].format.verticalAlignment() == QTextCharFormat::AlignSubScript);
        QVERIFY(runs[6].format.fontLetterSpacingType() == QFont::AbsoluteSpacing && near(runs[6].format.fontLetterSpacing(), 1.5));
        QVERIFY(near(runs[6].format.property(jp::tp::KernAbove).toDouble(), 12));
        QVERIFY(runs[7].format.fontCapitalization() == QFont::AllUppercase);
        QVERIFY(runs[8].format.fontCapitalization() == QFont::SmallCaps);
        QCOMPARE(runs[9].format.fontFamilies().toStringList(), (QStringList{"Carlito", "Noto Sans CJK JP"}));
        QCOMPARE(runs[10].format.stringProperty(jp::tp::ThemeFont), QStringLiteral("major"));
        QCOMPARE(runs[11].format.stringProperty(jp::tp::ThemeFont), QStringLiteral("minor"));
        QVERIFY(runs[10].format.fontFamilies().toStringList().isEmpty());
        QCOMPARE(runs[12].format.stringProperty(jp::tp::HighlightRefP), QStringLiteral("#FFFF00"));
        QCOMPARE(runs[13].format.stringProperty(jp::tp::Language), QStringLiteral("fr-FR"));
        QVERIFY(runs[13].format.boolProperty(jp::tp::NoProof));
        // The theme font names no typeface of its own, and the others name theirs.
        QVERIFY(r.text().contains(QStringLiteral("+mj-lt")) && r.text().contains(QStringLiteral("+mn-lt")));
        QVERIFY(r.text().contains(QStringLiteral("typeface=\"Carlito\"")));
    }

    void dmlWriteTextEscapesAndCleansWhatXmlCannotHold()
    {
        WriteRound r;
        const QString odd = QStringLiteral("a < b & c > \"d\" 'e' \u00E9\U0001F600") + QChar(0x01) + QChar(0x0B) + QChar(0xFFFE) + QStringLiteral("end\ttab");
        auto t = writtenTextBox(r, QRectF(0, 0, 100, 50), odd);
        t->name = QStringLiteral("n\"ame") + QChar(0x02);
        QVERIFY2(r.run({t}), qPrintable(r.error));
        QCOMPARE(r.storyOf(r.items[0])->toPlainText(), QStringLiteral("a < b & c > \"d\" 'e' \u00E9\U0001F600end\ttab"));
        QCOMPARE(static_cast<TextItem *>(r.items[0].get())->name, QStringLiteral("n\"ame"));
    }

    void dmlWriteParagraphsKeepTheirFormat()
    {
        WriteRound r;
        const QString id = r.addStory();
        QTextCursor c(r.story(id));
        QTextCharFormat cf;
        cf.setFontPointSize(16);
        auto block = [&](const QString &text, const std::function<void(QTextBlockFormat &)> &set, bool first = false) {
            QTextBlockFormat bf;
            set(bf);
            if (!first) c.insertBlock(bf, cf);
            else c.setBlockFormat(bf);
            c.insertText(text, cf);
        };
        block(QStringLiteral("centered"), [](QTextBlockFormat &bf) {
            bf.setAlignment(Qt::AlignHCenter);
            bf.setLeftMargin(36);
            bf.setTextIndent(-18);
            bf.setTopMargin(6);
            bf.setBottomMargin(12.5);
            bf.setLineHeight(120, QTextBlockFormat::ProportionalHeight);
        }, true);
        block(QStringLiteral("right"), [](QTextBlockFormat &bf) {
            bf.setAlignment(Qt::AlignRight);
            bf.setRightMargin(10);
            bf.setLineHeight(24, QTextBlockFormat::FixedHeight);
        });
        block(QStringLiteral("justified"), [](QTextBlockFormat &bf) { bf.setAlignment(Qt::AlignJustify); });
        block(QStringLiteral("spread"), [](QTextBlockFormat &bf) {
            bf.setAlignment(Qt::AlignJustify);
            bf.setProperty(jp::tp::Distribute, true);
        });
        block(QStringLiteral("left, no spacing"), [](QTextBlockFormat &bf) { bf.setAlignment(Qt::AlignLeft); });
        block(QStringLiteral("deeper"), [](QTextBlockFormat &bf) {
            bf.setProperty(jp::tp::ListLevel, 2);
            bf.setLeftMargin(72);
        });
        block(QStringLiteral("tabbed\tstops"), [](QTextBlockFormat &bf) {
            QList<QTextOption::Tab> tabs;
            tabs << QTextOption::Tab(36, QTextOption::LeftTab) << QTextOption::Tab(100, QTextOption::RightTab) << QTextOption::Tab(200, QTextOption::CenterTab)
                 << QTextOption::Tab(300, QTextOption::DelimiterTab, QLatin1Char('.'));
            bf.setTabPositions(tabs);
        });
        block(QStringLiteral("backwards"), [](QTextBlockFormat &bf) {
            bf.setLayoutDirection(Qt::RightToLeft);
            bf.setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
        });
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(0, 0, 400, 300);
        t->storyId = id;
        QVERIFY2(r.run({t}), qPrintable(r.error));
        const QTextDocument *back = r.storyOf(r.items[0]);
        QCOMPARE(back->blockCount(), 8);
        QVector<QTextBlockFormat> f;
        for (QTextBlock b = back->begin(); b.isValid(); b = b.next()) f << b.blockFormat();
        QCOMPARE(int(f[0].alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignHCenter));
        QVERIFY(near(f[0].leftMargin(), 36) && near(f[0].textIndent(), -18) && near(f[0].topMargin(), 6) && near(f[0].bottomMargin(), 12.5));
        QVERIFY(f[0].lineHeightType() == QTextBlockFormat::ProportionalHeight && near(f[0].lineHeight(), 120));
        QCOMPARE(int(f[1].alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignRight));
        QVERIFY(near(f[1].rightMargin(), 10));
        QVERIFY(f[1].lineHeightType() == QTextBlockFormat::FixedHeight && near(f[1].lineHeight(), 24));
        QCOMPARE(int(f[2].alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignJustify));
        QVERIFY(!f[2].boolProperty(jp::tp::Distribute));
        QCOMPARE(int(f[3].alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignJustify));
        QVERIFY(f[3].boolProperty(jp::tp::Distribute));
        QCOMPARE(int(f[4].alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignLeft));
        QVERIFY(near(f[4].leftMargin(), 0) && near(f[4].textIndent(), 0));
        QCOMPARE(f[5].intProperty(jp::tp::ListLevel), 2);
        QVERIFY(near(f[5].leftMargin(), 72));
        const QList<QTextOption::Tab> tabs = f[6].tabPositions();
        QCOMPARE(int(tabs.size()), 4);
        QVERIFY(near(tabs[0].position, 36) && tabs[0].type == QTextOption::LeftTab);
        QVERIFY(near(tabs[1].position, 100) && tabs[1].type == QTextOption::RightTab);
        QVERIFY(near(tabs[2].position, 200) && tabs[2].type == QTextOption::CenterTab);
        QVERIFY(near(tabs[3].position, 300) && tabs[3].type == QTextOption::DelimiterTab);
        QVERIFY(f[7].layoutDirection() == Qt::RightToLeft);
        QVERIFY(f[7].alignment() & Qt::AlignAbsolute);
        QVERIFY(f[7].alignment() & Qt::AlignLeft);
    }

    void dmlWriteTabsLeaveOutTheDefaultStopsAReaderMade()
    {
        WriteRound r;
        const QString id = r.addStory();
        QTextCursor c(r.story(id));
        QTextBlockFormat bf;
        QList<QTextOption::Tab> tabs;
        for (int i = 1; i <= 40; ++i) tabs << QTextOption::Tab(i * 36.0, QTextOption::LeftTab);
        bf.setTabPositions(tabs);
        c.setBlockFormat(bf);
        c.insertText(QStringLiteral("a\tb"));
        auto t = std::make_shared<TextItem>();
        t->storyId = id;
        QVERIFY2(r.run({t}), qPrintable(r.error));
        // Forty stops a point-and-a-half apart in a row are the default ones: they come as a distance.
        QVERIFY2(elementsNamed(r.spTree, "tab").size() < 10, qPrintable(QString::number(elementsNamed(r.spTree, "tab").size())));
        QVERIFY(r.text().contains(QStringLiteral("defTabSz=\"457200\"")));
        const QTextBlockFormat back = r.storyOf(r.items[0])->firstBlock().blockFormat();
        QVERIFY(back.tabPositions().isEmpty() || near(back.tabPositions().first().position, 36));
    }

    void dmlWriteBulletsAndNumbersKeepTheirList()
    {
        WriteRound r;
        const QString id = r.addStory();
        QTextCursor c(r.story(id));
        QTextCharFormat cf;
        cf.setFontPointSize(20);
        auto item = [&](const QString &text, const QTextListFormat &lf, bool first = false) {
            if (!first) c.insertBlock(QTextBlockFormat(), cf);
            c.insertText(text, cf);
            if (lf.style() != QTextListFormat::ListStyleUndefined) c.createList(lf);
        };
        QTextListFormat custom;
        custom.setStyle(QTextListFormat::ListDisc);
        custom.setProperty(jp::tp::BulletChar, QStringLiteral("\u00A7"));
        custom.setProperty(jp::tp::BulletFont, QStringLiteral("Wingdings"));
        custom.setProperty(jp::tp::BulletColor, QStringLiteral("#FF0000"));
        custom.setProperty(jp::tp::BulletSize, 10.0);
        item(QStringLiteral("marked"), custom, true);
        QTextListFormat plainDisc;
        plainDisc.setStyle(QTextListFormat::ListDisc);
        item(QStringLiteral("plain bullet"), plainDisc);
        item(QStringLiteral("no list"), QTextListFormat());
        QTextListFormat decimal;
        decimal.setStyle(QTextListFormat::ListDecimal);
        decimal.setNumberSuffix(QStringLiteral("."));
        decimal.setStart(3);
        decimal.setProperty(jp::tp::NumberFormat, 1);
        item(QStringLiteral("three"), decimal);
        item(QStringLiteral("four"), QTextListFormat());   // joins the list above (below)
        c.block().previous().textList()->add(c.block());
        QTextListFormat alpha;
        alpha.setStyle(QTextListFormat::ListLowerAlpha);
        alpha.setNumberPrefix(QStringLiteral("("));
        alpha.setNumberSuffix(QStringLiteral(")"));
        item(QStringLiteral("alpha"), alpha);
        QTextListFormat roman;
        roman.setStyle(QTextListFormat::ListUpperRoman);
        roman.setNumberSuffix(QStringLiteral(")"));
        item(QStringLiteral("roman"), roman);
        QTextListFormat inner;
        inner.setStyle(QTextListFormat::ListDisc);
        inner.setProperty(jp::tp::ListLevel, 1);
        item(QStringLiteral("inner"), inner);
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(0, 0, 400, 300);
        t->storyId = id;
        QVERIFY2(r.run({t}), qPrintable(r.error));
        const QTextDocument *back = r.storyOf(r.items[0]);
        QCOMPARE(back->blockCount(), 8);
        QVector<QTextBlock> blocks;
        for (QTextBlock b = back->begin(); b.isValid(); b = b.next()) blocks << b;
        QTextList *l0 = blocks[0].textList();
        QVERIFY(l0 && l0->format().style() == QTextListFormat::ListDisc);
        QCOMPARE(l0->format().stringProperty(jp::tp::BulletChar), QStringLiteral("\u00A7"));
        QCOMPARE(l0->format().stringProperty(jp::tp::BulletFont), QStringLiteral("Wingdings"));
        QCOMPARE(l0->format().stringProperty(jp::tp::BulletColor), QStringLiteral("#FF0000"));
        QVERIFY(near(l0->format().property(jp::tp::BulletSize).toDouble(), 10));
        QVERIFY(blocks[1].textList() && blocks[1].textList() != l0);
        QVERIFY(blocks[1].textList()->format().style() == QTextListFormat::ListDisc);
        QVERIFY(!blocks[1].textList()->format().hasProperty(jp::tp::BulletFont));
        QVERIFY(!blocks[2].textList());
        QTextList *numbers = blocks[3].textList();
        QVERIFY(numbers && numbers->format().style() == QTextListFormat::ListDecimal);
        QCOMPARE(numbers->format().numberSuffix(), QStringLiteral("."));
        QCOMPARE(numbers->format().start(), 3);
        QVERIFY(blocks[4].textList() == numbers);   // the next item goes on in the list
        QCOMPARE(numbers->itemNumber(blocks[4]), 1);
        QTextList *letters = blocks[5].textList();
        QVERIFY(letters && letters->format().style() == QTextListFormat::ListLowerAlpha);
        QCOMPARE(letters->format().numberPrefix(), QStringLiteral("("));
        QCOMPARE(letters->format().numberSuffix(), QStringLiteral(")"));
        QTextList *romans = blocks[6].textList();
        QVERIFY(romans && romans->format().style() == QTextListFormat::ListUpperRoman);
        QCOMPARE(romans->format().numberSuffix(), QStringLiteral(")"));
        QTextList *deeper = blocks[7].textList();
        QVERIFY(deeper);
        QCOMPARE(deeper->format().intProperty(jp::tp::ListLevel), 1);
        QCOMPARE(blocks[7].blockFormat().intProperty(jp::tp::ListLevel), 1);
        // A bullet with no mark of its own says a bullet, in the text's font.
        QVERIFY(!r.text().contains(QStringLiteral("typeface=\"Arial\"")));
        QCOMPARE(int(elementsNamed(r.spTree, "buNone").size()), 0);
    }

    void dmlWritePlaceholderTextSaysNoBulletWhenItHasNone()
    {
        WriteRound r;
        const QString id = r.addStory();
        QTextCursor c(r.story(id));
        c.insertText(QStringLiteral("listed"));
        QTextListFormat lf;
        lf.setStyle(QTextListFormat::ListDisc);
        c.createList(lf);
        c.insertBlock(QTextBlockFormat(), QTextCharFormat());
        c.insertText(QStringLiteral("plain"));
        auto holder = std::make_shared<TextItem>();
        holder->role = QStringLiteral("ph:body:1");
        holder->storyId = id;
        auto box = std::make_shared<TextItem>();
        box->storyId = r.addStory(QStringLiteral("one\ntwo"));
        QVERIFY2(r.run({holder, box}), qPrintable(r.error));
        // Only the placeholder's paragraph with no list says so; a text box says nothing.
        const QVector<QDomElement> bodies = elementsNamed(r.spTree, "txBody");
        QCOMPARE(int(bodies.size()), 2);
        QCOMPARE(int(elementsNamed(bodies[0], "buNone").size()), 1);
        QCOMPARE(int(elementsNamed(bodies[0], "buChar").size()), 1);
        QCOMPARE(int(elementsNamed(bodies[1], "buNone").size()), 0);
        // The layout and master may space and mark paragraphs in their own way: a paragraph of a placeholder says how it is.
        QCOMPARE(int(elementsNamed(bodies[0], "lnSpc").size()), 2);
        QCOMPARE(int(elementsNamed(bodies[0], "spcBef").size()), 2);
        QCOMPARE(int(elementsNamed(bodies[0], "spcAft").size()), 2);
        QCOMPARE(int(elementsNamed(bodies[0], "buClrTx").size()), 1);
        QCOMPARE(int(elementsNamed(bodies[0], "buSzTx").size()), 1);
        QCOMPARE(int(elementsNamed(bodies[0], "buFontTx").size()), 1);
        for (const char *name : {"lnSpc", "spcBef", "spcAft", "buClrTx", "buSzTx", "buFontTx"}) QCOMPARE(int(elementsNamed(bodies[1], name).size()), 0);
        // And the flag is the writer's own: it is off again after a placeholder.
        QVERIFY(!r.ctx.placeholderText);
    }

    void dmlWriteFieldsBreaksAndEmptyParagraphs()
    {
        WriteRound r;
        const QString id = r.addStory();
        QTextCursor c(r.story(id));
        QTextCharFormat cf;
        cf.setFontPointSize(12);
        c.setBlockCharFormat(cf);
        c.insertText(QStringLiteral("Page "), cf);
        QTextCharFormat field = cf;
        field.setProperty(jp::tp::Field, QStringLiteral("page"));
        c.insertText(QString(QChar::ObjectReplacementCharacter), field);
        c.insertText(QStringLiteral(" of "), cf);
        field.setProperty(jp::tp::Field, QStringLiteral("page"));
        c.insertText(QString(QChar::ObjectReplacementCharacter), field);
        c.insertText(QString(QChar::LineSeparator) + QStringLiteral("second line"), cf);
        c.insertBlock(QTextBlockFormat(), cf);
        field.setProperty(jp::tp::Field, QStringLiteral("date:M/d/yyyy"));
        c.insertText(QString(QChar::ObjectReplacementCharacter), field);
        c.insertText(QStringLiteral(" "), cf);
        field.setProperty(jp::tp::Field, QStringLiteral("time:HH:mm"));
        c.insertText(QString(QChar::ObjectReplacementCharacter), field);
        // An empty paragraph keeps its size, and one with only a break is not empty.
        QTextCharFormat big;
        big.setFontPointSize(31);
        c.insertBlock(QTextBlockFormat(), big);
        c.insertBlock(QTextBlockFormat(), cf);
        c.insertText(QString(QChar::LineSeparator), cf);
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(0, 0, 400, 300);
        t->storyId = id;
        QVERIFY2(r.run({t}), qPrintable(r.error));
        const QTextDocument *back = r.storyOf(r.items[0]);
        QCOMPARE(back->blockCount(), 4);
        const QVector<Run> first = runsOf(back->firstBlock());
        QStringList codes;
        for (const Run &run : first)
            if (!run.format.stringProperty(jp::tp::Field).isEmpty()) codes << run.format.stringProperty(jp::tp::Field);
        QCOMPARE(codes, (QStringList{"page", "page"}));
        QCOMPARE(back->firstBlock().text(), QString(QStringLiteral("Page ") + QChar::ObjectReplacementCharacter + QStringLiteral(" of ") + QChar::ObjectReplacementCharacter
                                                    + QChar(QChar::LineSeparator) + QStringLiteral("second line")));
        const QVector<Run> second = runsOf(back->firstBlock().next());
        QCOMPARE(second[0].format.stringProperty(jp::tp::Field), QStringLiteral("date:M/d/yyyy"));
        QCOMPARE(second.last().format.stringProperty(jp::tp::Field), QStringLiteral("time:HH:mm"));
        // Each field has an id of its own, and a type the standard names.
        const QVector<QDomElement> fields = elementsNamed(r.spTree, "fld");
        QCOMPARE(int(fields.size()), 4);
        QSet<QString> fieldIds;
        for (const QDomElement &f : fields) {
            QVERIFY(QRegularExpression(QStringLiteral("^\\{[0-9A-F]{8}-[0-9A-F]{4}-[0-9A-F]{4}-[0-9A-F]{4}-[0-9A-F]{12}\\}$")).match(f.attribute(QStringLiteral("id"))).hasMatch());
            fieldIds << f.attribute(QStringLiteral("id"));
        }
        QCOMPARE(int(fieldIds.size()), 4);
        QCOMPARE(fields[0].attribute(QStringLiteral("type")), QStringLiteral("slidenum"));
        QVERIFY(fields[2].attribute(QStringLiteral("type")).startsWith(QLatin1String("datetime")));
        // The empty paragraph is as tall as it was.
        QVERIFY(near(back->firstBlock().next().next().charFormat().fontPointSize(), 31));
        QVERIFY(back->firstBlock().next().next().text().isEmpty());
        QVERIFY(!back->lastBlock().text().isEmpty());
        QCOMPARE(int(elementsNamed(r.spTree, "endParaRPr").size()), 1);
    }

    void dmlWriteHyperlinksAreRelationshipsMadeOnce()
    {
        WriteRound r;
        const QString id = r.addStory();
        QTextCursor c(r.story(id));
        QTextCharFormat cf;
        cf.setFontPointSize(12);
        c.insertText(QStringLiteral("see "), cf);
        QTextCharFormat link = cf;
        link.setAnchor(true);
        link.setAnchorHref(QStringLiteral("https://example.com/a?b=1&c=2"));
        c.insertText(QStringLiteral("here"), link);
        c.insertText(QStringLiteral(" or "), cf);
        c.insertText(QStringLiteral("there"), link);
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(0, 0, 400, 100);
        t->storyId = id;
        t->hyperlink = QStringLiteral("mailto:someone@example.com");
        QVERIFY2(r.run({t}), qPrintable(r.error));
        const QVector<Run> runs = runsOf(r.storyOf(r.items[0])->firstBlock());
        QCOMPARE(int(runs.size()), 4);
        QVERIFY(!runs[0].format.isAnchor());
        QVERIFY(runs[1].format.isAnchor() && runs[3].format.isAnchor());
        QCOMPARE(runs[1].format.anchorHref(), QStringLiteral("https://example.com/a?b=1&c=2"));
        QCOMPARE(runs[3].format.anchorHref(), QStringLiteral("https://example.com/a?b=1&c=2"));
        QCOMPARE(r.items[0]->hyperlink, QStringLiteral("mailto:someone@example.com"));
        // Two addresses: the item's, and the one both runs use.
        const QVector<Relationship> rels = r.env.package.relationships(r.ctx.part);
        int external = 0;
        for (const Relationship &rel : rels) external += rel.external && rel.type.endsWith(QLatin1String("/hyperlink"));
        QCOMPARE(external, 2);
    }

    void dmlWriteShapesKeepPresetAndAdjustments_data()
    {
        QTest::addColumn<QString>("shape");
        QTest::addColumn<QString>("preset");
        QTest::addColumn<QSizeF>("size");
        QTest::addColumn<QVector<double>>("adj");
        QTest::newRow("rect") << "rect" << "rect" << QSizeF(100, 50) << QVector<double>();
        QTest::newRow("roundRect") << "roundRect" << "roundRect" << QSizeF(100, 50) << QVector<double>{0.3};
        QTest::newRow("right arrow") << "rightArrow" << "rightArrow" << QSizeF(160, 80) << QVector<double>{0.4, 0.6};
        QTest::newRow("up arrow") << "upArrow" << "upArrow" << QSizeF(80, 160) << QVector<double>{0.3, 0.5};
        QTest::newRow("parallelogram") << "parallelogram" << "parallelogram" << QSizeF(200, 100) << QVector<double>{0.15};
        QTest::newRow("trapezoid") << "trapezoid" << "trapezoid" << QSizeF(120, 200) << QVector<double>{0.2};
        QTest::newRow("star") << "star5" << "star5" << QSizeF(100, 100) << QVector<double>{0.3};
        QTest::newRow("quad arrow") << "quadArrow" << "quadArrow" << QSizeF(100, 100) << QVector<double>{0.1, 0.2, 0.25};
        QTest::newRow("u-turn arrow") << "uturnArrow" << "uturnArrow" << QSizeF(100, 100) << QVector<double>{0.2, 0.3};
        QTest::newRow("diagonal snip") << "snip2diag" << "snip2DiagRect" << QSizeF(100, 60) << QVector<double>{0.25};
        QTest::newRow("double wave") << "doubleWave" << "doubleWave" << QSizeF(100, 60) << QVector<double>{0.05};
        QTest::newRow("bent arrow") << "bentArrow" << "bentArrow" << QSizeF(100, 100) << QVector<double>{0.1, 0.2, 0.3};
        QTest::newRow("triangle") << "triangle" << "triangle" << QSizeF(100, 100) << QVector<double>{0.3};
        QTest::newRow("ellipse") << "ellipse" << "ellipse" << QSizeF(100, 100) << QVector<double>();
        QTest::newRow("flow chart") << "fcDecision" << "flowChartDecision" << QSizeF(100, 100) << QVector<double>();
        QTest::newRow("smiley") << "smiley" << "smileyFace" << QSizeF(100, 100) << QVector<double>{0.02};
    }

    void dmlWriteShapesKeepPresetAndAdjustments()
    {
        QFETCH(QString, shape);
        QFETCH(QString, preset);
        QFETCH(QSizeF, size);
        QFETCH(QVector<double>, adj);
        QCOMPARE(presetForShape(shape), preset);
        WriteRound r;
        auto s = std::make_shared<ShapeItem>();
        s->rect = QRectF(20, 30, size.width(), size.height());
        s->shape = shape;
        s->adj = adj;
        s->fill = Fill::solid(ColorRef::rgb(QColor(0x20, 0x40, 0x80)));
        s->stroke = Stroke::line(ColorRef::rgb(QColor(0x80, 0x40, 0x20)), 1.5);
        QVERIFY2(r.run({s}), qPrintable(r.error));
        QVERIFY2(r.skipped.isEmpty(), qPrintable(r.skipped.join('\n')));
        QCOMPARE(int(r.items.size()), 1);
        QCOMPARE(int(r.items[0]->type()), int(ItemType::Shape));
        const auto *b = static_cast<ShapeItem *>(r.items[0].get());
        QCOMPARE(b->shape, shape);
        QVERIFY(b->customPath.isEmpty());
        QVERIFY(sameRect(b->rect, s->rect));
        QCOMPARE(dmlHex(b->fill.color), QStringLiteral("204080"));
        QVERIFY(near(b->stroke.width, 1.5));
        QCOMPARE(elementsNamed(r.spTree, "prstGeom").value(0).attribute(QStringLiteral("prst")), preset);
        // The adjustments come back as they were, whether or not they were written (the standard's own are not).
        const QVector<double> want = adj.isEmpty() ? libraryAdjustments(preset, {}, size) : adj;
        QCOMPARE(int(b->adj.size()), int(want.size()));
        for (int i = 0; i < want.size(); ++i) QVERIFY2(near(b->adj[i], want[i], 1e-4), qPrintable(QStringLiteral("adjustment %1: %2 for %3").arg(i).arg(b->adj[i]).arg(want[i])));
    }

    void dmlWriteShapesLeaveOutAnAdjustmentThatIsTheStandards()
    {
        WriteRound r;
        auto s = std::make_shared<ShapeItem>();
        s->rect = QRectF(0, 0, 100, 100);
        s->shape = QStringLiteral("roundRect");
        s->adj = {0.16667};
        QVERIFY2(r.run({s}), qPrintable(r.error));
        QCOMPARE(int(elementsNamed(r.spTree, "gd").size()), 0);
        WriteRound other;
        s->adj = {0.4};
        QVERIFY2(other.run({s}), qPrintable(other.error));
        const QVector<QDomElement> gds = elementsNamed(other.spTree, "gd");
        QCOMPARE(int(gds.size()), 1);
        QCOMPARE(gds[0].attribute(QStringLiteral("name")), QStringLiteral("adj"));
        QCOMPARE(gds[0].attribute(QStringLiteral("fmla")), QStringLiteral("val 40000"));
    }

    // A preset with more than one adjustment has all of them written once one
    // is not the standard's: the presentation program refuses a file whose
    // u-turn arrow says adj1 and not adj2 to adj5 (or whose arrow says adj2
    // and not adj1), and opens the same file with all or none.
    void dmlWriteShapesWriteAllOfAPresetsAdjustmentsOrNone()
    {
        auto names = [](const QVector<QDomElement> &gds) {
            QStringList out;
            for (const QDomElement &g : gds) out << g.attribute(QStringLiteral("name")) + QLatin1Char('=') + g.attribute(QStringLiteral("fmla"));
            return out;
        };
        auto s = std::make_shared<ShapeItem>();
        s->rect = QRectF(0, 0, 160, 80);
        s->shape = QStringLiteral("rightArrow");
        s->adj = {0.5, 0.75};   // the shaft as the standard has it, a longer head
        WriteRound arrow;
        QVERIFY2(arrow.run({s}), qPrintable(arrow.error));
        QCOMPARE(names(elementsNamed(arrow.spTree, "gd")), (QStringList{"adj1=val 50000", "adj2=val 75000"}));

        s->shape = QStringLiteral("uturnArrow");
        s->rect = QRectF(0, 0, 100, 100);
        s->adj = {0.2};          // the library's first is the standard's adj1; the others keep the standard's
        WriteRound turn;
        QVERIFY2(turn.run({s}), qPrintable(turn.error));
        QCOMPARE(names(elementsNamed(turn.spTree, "gd")),
                 (QStringList{"adj1=val 20000", "adj2=val 25000", "adj3=val 25000", "adj4=val 43750", "adj5=val 75000"}));

        s->shape = QStringLiteral("rightArrow");
        s->rect = QRectF(0, 0, 160, 80);
        s->adj = {0.5, 0.5};     // the standard's own: none
        WriteRound plain;
        QVERIFY2(plain.run({s}), qPrintable(plain.error));
        QCOMPARE(int(elementsNamed(plain.spTree, "gd").size()), 0);
    }

    void dmlWriteShapeHoldsItsText()
    {
        WriteRound r;
        auto s = std::make_shared<ShapeItem>();
        s->rect = QRectF(0, 0, 160, 80);
        s->shape = QStringLiteral("roundRect");
        s->storyId = r.addStory(QStringLiteral("Inside"));
        s->insets = QMarginsF(5, 6, 7, 8);
        s->valign = jp::VAlign::Bottom;
        auto empty = std::make_shared<ShapeItem>();
        empty->rect = QRectF(200, 0, 50, 50);
        QVERIFY2(r.run({s, empty}), qPrintable(r.error));
        QCOMPARE(int(r.items.size()), 2);
        const auto *a = static_cast<ShapeItem *>(r.items[0].get());
        QCOMPARE(r.storyOf(r.items[0])->toPlainText(), QStringLiteral("Inside"));
        QCOMPARE(a->insets, QMarginsF(5, 6, 7, 8));
        QVERIFY(a->valign == jp::VAlign::Bottom);
        // A shape with no text has a text body with none in it, which keeps where text would sit, and no story.
        QVERIFY(static_cast<ShapeItem *>(r.items[1].get())->storyId.isEmpty());
        QCOMPARE(int(elementsNamed(r.spTree, "txBody").size()), 2);
        QCOMPARE(static_cast<ShapeItem *>(r.items[1].get())->insets, empty->insets);
        QVERIFY(static_cast<ShapeItem *>(r.items[1].get())->valign == empty->valign);
    }

    void dmlWriteFreeformShapesKeepTheirPath()
    {
        WriteRound r;
        auto s = std::make_shared<ShapeItem>();
        s->rect = QRectF(10, 20, 200, 100);
        s->shape = QStringLiteral("rect");
        QPainterPath p;
        p.moveTo(0, 100);
        p.lineTo(100, 0);
        p.cubicTo(150, 20, 190, 60, 200, 100);
        p.closeSubpath();
        p.moveTo(80, 90);
        p.lineTo(120, 90);
        p.lineTo(100, 50);   // an open one, left open
        p.moveTo(150, 90);
        p.cubicTo(170, 60, 190, 60, 150, 90);   // one that ends where it began, closed by nothing
        p.closeSubpath();
        s->customPath = p;
        s->fill = Fill::solid(ColorRef::rgb(Qt::green));
        s->stroke = Stroke::line(ColorRef::rgb(Qt::black), 1);
        QVERIFY2(r.run({s}), qPrintable(r.error));
        QCOMPARE(int(r.items.size()), 1);
        const auto *b = static_cast<ShapeItem *>(r.items[0].get());
        QVERIFY(sameRect(b->rect, s->rect));
        QVERIFY(!b->customPath.isEmpty());
        QVERIFY(sameRect(b->customPath.boundingRect(), p.boundingRect(), 1e-3));
        QCOMPARE(int(elementsNamed(r.spTree, "custGeom").size()), 1);
        QCOMPARE(int(elementsNamed(r.spTree, "moveTo").size()), 3);
        QCOMPARE(int(elementsNamed(r.spTree, "cubicBezTo").size()), 2);
        QCOMPARE(int(elementsNamed(r.spTree, "close").size()), 2);
        // The path is the same path: the points of the curve and of both pieces.
        QCOMPARE(b->customPath.elementCount(), p.elementCount());
        for (int i = 0; i < p.elementCount(); ++i) {
            QVERIFY2(near(b->customPath.elementAt(i).x, p.elementAt(i).x, 1e-3) && near(b->customPath.elementAt(i).y, p.elementAt(i).y, 1e-3), qPrintable(QString::number(i)));
            QCOMPARE(int(b->customPath.elementAt(i).type), int(p.elementAt(i).type));
        }
    }

    void dmlWriteLibraryShapesWithoutAPresetBecomeFreeforms()
    {
        const jp::ShapeDef *def = jp::shapeDef(QStringLiteral("tab"));
        QVERIFY(def);
        QVERIFY(presetForShape(def->id).isEmpty());
        WriteRound r;
        auto s = std::make_shared<ShapeItem>();
        s->rect = QRectF(0, 0, 120, 80);
        s->shape = def->id;
        QVERIFY2(r.run({s}), qPrintable(r.error));
        QVERIFY2(r.skipped.isEmpty(), qPrintable(r.skipped.join('\n')));
        const auto *b = static_cast<ShapeItem *>(r.items[0].get());
        QVERIFY(!b->customPath.isEmpty());
        const QRectF want = jp::shapePath(def->id, s->rect.size()).boundingRect();
        QVERIFY(sameRect(b->customPath.boundingRect(), want, 1e-3));
        QCOMPARE(int(elementsNamed(r.spTree, "prstGeom").size()), 0);
    }

    void dmlWriteFillsKeepTheirKind()
    {
        const auto through = [](const Fill &fill, WriteRound *keep = nullptr) {
            WriteRound local;
            WriteRound &r = keep ? *keep : local;
            const bool ok = r.runWith([&](QXmlStreamWriter &w) {
                w.writeStartElement(QStringLiteral("a:spPr"));
                writeFill(w, fill, r.ctx);
                w.writeEndElement();
            });
            if (!ok) return Fill::solid(ColorRef::rgb(Qt::magenta));
            return readFill(child(r.spTree, QString(ns::A), QStringLiteral("spPr")), QDomElement(), r.env.ctx);
        };
        // None, and a color that is none.
        QVERIFY(through(Fill::none()).isNone());
        QVERIFY(through(Fill::solid(ColorRef::none())).isNone());
        // Solid, with its transparency; and a scheme color is the color the scheme gives.
        Fill f = through(Fill::solid(ColorRef::rgb(QColor(0x33, 0x66, 0xCC)), 0.4));
        QVERIFY(f.type == Fill::Solid);
        QCOMPARE(dmlHex(f.color), QStringLiteral("3366CC"));
        QVERIFY(near(f.transparency, 0.4, 1e-2));
        f = through(Fill::solid(ColorRef::rgb(QColor(0x33, 0x66, 0xCC))));
        QCOMPARE(f.transparency, 0.0);
        WriteRound scheme;
        f = through(Fill::solid(ColorRef::scheme(jp::Accent2)), &scheme);
        QCOMPARE(dmlHex(f.color), hexOf(ColorRef::scheme(jp::Accent2).resolve(scheme.ctx.colors)));
        // A gradient: its stops with their places and clearness, and its direction.
        Fill g;
        g.type = Fill::Gradient;
        g.angle = 45;
        g.stops = {{0, ColorRef::rgb(QColor(0xFF, 0, 0)), 0}, {0.5, ColorRef::rgb(QColor(0, 0xFF, 0)), 0.5}, {1, ColorRef::rgb(QColor(0, 0, 0xFF)), 0}};
        f = through(g);
        QVERIFY(f.type == Fill::Gradient && f.gradType == Fill::Linear);
        QVERIFY(near(f.angle, 45, 1e-3));
        QCOMPARE(int(f.stops.size()), 3);
        QVERIFY(near(f.stops[1].pos, 0.5, 1e-4));
        QCOMPARE(dmlHex(f.stops[0].color), QStringLiteral("FF0000"));
        QCOMPARE(dmlHex(f.stops[1].color), QStringLiteral("00FF00"));
        QCOMPARE(dmlHex(f.stops[2].color), QStringLiteral("0000FF"));
        QVERIFY(near(f.stops[1].transparency, 0.5, 1e-2));
        // A gradient of two colors and no stops has the two for stops.
        f = through(Fill::gradient(ColorRef::rgb(Qt::white), ColorRef::rgb(Qt::black), 90));
        QCOMPARE(int(f.stops.size()), 2);
        QCOMPARE(dmlHex(f.stops[0].color), QStringLiteral("FFFFFF"));
        QCOMPARE(dmlHex(f.stops[1].color), QStringLiteral("000000"));
        QVERIFY(near(f.angle, 90, 1e-3));
        const struct { Fill::GradType type; const char *path; } kinds[] = {{Fill::Radial, "circle"}, {Fill::Rectangular, "rect"}, {Fill::PathGrad, "shape"}};
        for (const auto &k : kinds) {
            WriteRound r;
            g.gradType = k.type;
            f = through(g, &r);
            QVERIFY2(f.type == Fill::Gradient && f.gradType == k.type, k.path);
            QCOMPARE(elementsNamed(r.spTree, "path").value(0).attribute(QStringLiteral("path")), QString::fromLatin1(k.path));
        }
        // A gradient with one stop is a plain color.
        Fill one;
        one.type = Fill::Gradient;
        one.stops = {{0.3, ColorRef::rgb(QColor(1, 2, 3)), 0}};
        f = through(one);
        QVERIFY(f.type == Fill::Solid);
        QCOMPARE(dmlHex(f.color), QStringLiteral("010203"));
        // Patterns: each of the library's, with its two colors.
        for (int i = 0; i < 13; ++i) {
            Fill p;
            p.type = Fill::Pattern;
            p.pattern = i;
            p.color = ColorRef::rgb(QColor(0x10, 0x20, 0x30));
            p.color2 = ColorRef::rgb(QColor(0xF0, 0xE0, 0xD0));
            f = through(p);
            QVERIFY2(f.type == Fill::Pattern && f.pattern == i, qPrintable(QString::number(i)));
            QCOMPARE(dmlHex(f.color), QStringLiteral("102030"));
            QCOMPARE(dmlHex(f.color2), QStringLiteral("F0E0D0"));
        }
        // Pictures: stretched, with some clearness, and tiled at a scale.
        WriteRound pictures;
        const QString id = pictures.addPicture(Qt::blue);
        Fill pic;
        pic.type = Fill::Picture;
        pic.imageId = id;
        pic.transparency = 0.3;
        f = through(pic, &pictures);
        QVERIFY(f.type == Fill::Picture && !f.tile);
        QVERIFY(near(f.transparency, 0.3, 1e-4));
        QCOMPARE(pictures.env.images.size(), 1);
        QCOMPARE(pictures.env.images[0].bytes, pictures.pictures[id]);
        WriteRound tiles;
        pic.imageId = tiles.addPicture(Qt::blue);
        pic.transparency = 0;
        pic.tile = true;
        pic.tileScale = 0.5;
        f = through(pic, &tiles);
        QVERIFY(f.type == Fill::Picture && f.tile);
        QVERIFY(near(f.tileScale, 0.5, 1e-4));
        // A texture is a picture that is tiled; a picture that isn't there is no fill.
        WriteRound textures;
        Fill tex;
        tex.type = Fill::Texture;
        tex.imageId = textures.addPicture(Qt::green);
        f = through(tex, &textures);
        QVERIFY(f.type == Fill::Picture && f.tile);
        QVERIFY(through(pic).isNone());
    }

    void dmlWriteLinesKeepTheirKind()
    {
        const auto through = [](const Stroke &stroke, WriteRound *keep = nullptr) {
            WriteRound local;
            WriteRound &r = keep ? *keep : local;
            const bool ok = r.runWith([&](QXmlStreamWriter &w) {
                w.writeStartElement(QStringLiteral("a:spPr"));
                writeLine(w, stroke, r.ctx);
                w.writeEndElement();
            });
            if (!ok) return Stroke::line(ColorRef::rgb(Qt::magenta), 99);
            return readLine(child(r.spTree, QString(ns::A), QStringLiteral("spPr")), QDomElement(), r.env.ctx);
        };
        Stroke s = through(Stroke::line(ColorRef::rgb(QColor(0x12, 0x34, 0x56)), 3.5));
        QVERIFY(!s.isNone());
        QVERIFY(near(s.width, 3.5));
        QCOMPARE(dmlHex(s.color), QStringLiteral("123456"));
        QCOMPARE(s.transparency, 0.0);
        QVERIFY(s.dash == Stroke::SolidLine && s.compound == Stroke::Single && s.cap == Qt::FlatCap && s.join == Qt::MiterJoin);
        QVERIFY(through(Stroke::none()).isNone());
        Stroke clear = Stroke::line(ColorRef::rgb(Qt::black), 0);
        QVERIFY(through(clear).isNone());
        s = Stroke::line(ColorRef::rgb(Qt::black), 1);
        s.transparency = 0.5;
        QVERIFY(near(through(s).transparency, 0.5, 1e-2));
        for (int d = int(Stroke::SolidLine); d <= int(Stroke::LongDashDotDot); ++d) {
            Stroke t = Stroke::line(ColorRef::rgb(Qt::black), 1);
            t.dash = Stroke::Dash(d);
            QVERIFY2(through(t).dash == t.dash, qPrintable(QString::number(d)));
        }
        for (Qt::PenCapStyle cap : {Qt::FlatCap, Qt::RoundCap, Qt::SquareCap}) {
            Stroke t = Stroke::line(ColorRef::rgb(Qt::black), 1);
            t.cap = cap;
            QVERIFY(through(t).cap == cap);
        }
        for (Qt::PenJoinStyle join : {Qt::MiterJoin, Qt::RoundJoin, Qt::BevelJoin}) {
            Stroke t = Stroke::line(ColorRef::rgb(Qt::black), 1);
            t.join = join;
            QVERIFY(through(t).join == join);
        }
        for (int c = int(Stroke::Single); c <= int(Stroke::Triple); ++c) {
            Stroke t = Stroke::line(ColorRef::rgb(Qt::black), 4);
            t.compound = Stroke::Compound(c);
            QVERIFY2(through(t).compound == t.compound, qPrintable(QString::number(c)));
        }
        Stroke inset = Stroke::line(ColorRef::rgb(Qt::black), 4);
        inset.inset = true;
        QVERIFY(through(inset).inset);
        // Arrowheads: each kind and size at either end.
        for (int a = int(Arrow::None); a <= int(Arrow::Oval); ++a)
            for (int size = 0; size <= 2; ++size) {
                Stroke t = Stroke::line(ColorRef::rgb(Qt::black), 1);
                t.startArrow = Arrow(a);
                t.endArrow = Arrow((a + 1) % 6);
                t.startSize = size;
                t.endSize = 2 - size;
                const Stroke back = through(t);
                QVERIFY2(back.startArrow == t.startArrow && back.endArrow == t.endArrow, qPrintable(QString::number(a)));
                if (t.startArrow != Arrow::None) QCOMPARE(back.startSize, size);
                if (t.endArrow != Arrow::None) QCOMPARE(back.endSize, 2 - size);
            }
    }

    void dmlWriteEffectsKeepTheirShadowGlowAndSoftEdges()
    {
        WriteRound r;
        auto s = std::make_shared<ShapeItem>();
        s->rect = QRectF(0, 0, 100, 60);
        s->fx.shadow.on = true;
        s->fx.shadow.color = ColorRef::rgb(QColor(0x40, 0x40, 0x40));
        s->fx.shadow.transparency = 0.5;
        s->fx.shadow.blur = 6;
        s->fx.shadow.distance = 4;
        s->fx.shadow.angle = 135;
        s->fx.glow.on = true;
        s->fx.glow.color = ColorRef::rgb(QColor(0xFF, 0xCC, 0));
        s->fx.glow.size = 8;
        s->fx.glow.transparency = 0.6;
        s->fx.softEdge = 3;
        auto inner = std::make_shared<ShapeItem>();
        inner->rect = QRectF(200, 0, 100, 60);
        inner->fx.shadow.on = true;
        inner->fx.shadow.inner = true;
        auto plain = std::make_shared<ShapeItem>();
        plain->rect = QRectF(400, 0, 100, 60);
        QVERIFY2(r.run({s, inner, plain}), qPrintable(r.error));
        QCOMPARE(int(r.items.size()), 3);
        const jp::Effects fx = r.items[0]->fx;
        QVERIFY(fx.shadow.on && !fx.shadow.inner);
        QCOMPARE(dmlHex(fx.shadow.color), QStringLiteral("404040"));
        QVERIFY(near(fx.shadow.transparency, 0.5, 1e-2));
        QVERIFY(near(fx.shadow.blur, 6) && near(fx.shadow.distance, 4) && near(fx.shadow.angle, 135, 1e-3));
        QVERIFY(fx.glow.on);
        QCOMPARE(dmlHex(fx.glow.color), QStringLiteral("FFCC00"));
        QVERIFY(near(fx.glow.size, 8) && near(fx.glow.transparency, 0.6, 1e-2));
        QVERIFY(near(fx.softEdge, 3));
        QVERIFY(r.items[1]->fx.shadow.on && r.items[1]->fx.shadow.inner);
        QVERIFY(!r.items[2]->fx.any());
        QCOMPARE(int(elementsNamed(r.spTree, "effectLst").size()), 2);
    }

    void dmlWritePicturesAreStoredOnce()
    {
        WriteRound r;
        const QString red = r.addPicture(Qt::red), blue = r.addPicture(Qt::blue);
        auto make = [&](const QString &image, const QRectF &rect) {
            auto p = std::make_shared<PictureItem>();
            p->imageId = image;
            p->rect = rect;
            p->imgRect = QRectF(0, 0, rect.width(), rect.height());
            return p;
        };
        auto first = make(red, QRectF(10, 10, 200, 100));
        first->imgRect = QRectF(-20, -10, 240, 130);   // cut at the left, top, and bottom; and at the right
        first->name = QStringLiteral("Logo");
        first->altText = QStringLiteral("The logo");
        auto second = make(red, QRectF(300, 10, 100, 100));
        second->maskShape = QStringLiteral("ellipse");
        second->rotation = 90;
        second->transparency = 0.3;
        second->brightness = 20;
        second->contrast = -10;
        second->recolor = PictureItem::Grayscale;
        auto third = make(blue, QRectF(10, 200, 50, 50));
        third->imgRect = QRectF(5, 5, 40, 40);   // room is left around: a negative cut
        QVERIFY2(r.run({first, second, third}), qPrintable(r.error));
        QVERIFY2(r.skipped.isEmpty(), qPrintable(r.skipped.join('\n')));
        // Two pictures were stored, not three, and the package says what they are.
        QCOMPARE(r.media.size(), 2);
        QVERIFY(r.media.contains(red) && r.media.contains(blue));
        for (const QString &part : r.media) {
            QVERIFY2(r.env.package.hasPart(part), qPrintable(part));
            QCOMPARE(r.env.package.contentType(part), QStringLiteral("image/png"));
        }
        QCOMPARE(r.env.package.part(r.media[red]), r.pictures[red]);
        int pictureLinks = 0;
        for (const Relationship &rel : r.env.package.relationships(r.ctx.part)) pictureLinks += rel.type.endsWith(QLatin1String("/image"));
        QCOMPARE(pictureLinks, 2);
        QCOMPARE(int(r.items.size()), 3);
        QCOMPARE(r.env.images.size(), 2);
        const auto *a = static_cast<PictureItem *>(r.items[0].get());
        const auto *b = static_cast<PictureItem *>(r.items[1].get());
        const auto *c = static_cast<PictureItem *>(r.items[2].get());
        QCOMPARE(a->imageId, b->imageId);
        QVERIFY(a->imageId != c->imageId);
        QCOMPARE(r.env.images[0].bytes, r.pictures[red]);
        QCOMPARE(r.env.images[1].bytes, r.pictures[blue]);
        QVERIFY(sameRect(a->rect, first->rect));
        QVERIFY(sameRect(a->imgRect, first->imgRect, 5e-3));
        QCOMPARE(a->name, QStringLiteral("Logo"));
        QCOMPARE(a->altText, QStringLiteral("The logo"));
        QCOMPARE(a->maskShape, QStringLiteral("rect"));
        QCOMPARE(b->maskShape, QStringLiteral("ellipse"));
        QVERIFY(near(b->rotation, 90, 1e-4));
        QVERIFY(near(b->transparency, 0.3, 1e-4));
        QVERIFY(near(b->brightness, 20, 1e-3) && near(b->contrast, -10, 1e-3));
        QVERIFY(b->recolor == PictureItem::Grayscale);
        QVERIFY(sameRect(c->imgRect, third->imgRect, 5e-3));
        // A cut is a source rectangle and the picture is stretched over the frame.
        QVERIFY(r.text().contains(QStringLiteral("<a:srcRect")));
        QCOMPARE(int(elementsNamed(r.spTree, "srcRect").size()), 2);
        QCOMPARE(int(elementsNamed(r.spTree, "stretch").size()), 3);
    }

    void dmlWritePicturesKeepTheirFormat()
    {
        WriteRound r;
        QMap<QString, QPair<QString, QByteArray>> files;   // image id -> format named, bytes
        QImage img(6, 3, QImage::Format_RGB32);
        img.fill(Qt::darkCyan);
        for (const char *format : {"PNG", "JPG", "BMP"}) {
            QByteArray bytes;
            QBuffer buffer(&bytes);
            buffer.open(QIODevice::WriteOnly);
            QVERIFY(img.save(&buffer, format));
            files.insert(QString::fromLatin1(format).toLower(), {QString::fromLatin1(format).toLower(), bytes});
        }
        files.insert(QStringLiteral("vector"), {QStringLiteral("svg"), "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"40\" height=\"20\"><rect width=\"40\" height=\"20\" fill=\"red\"/></svg>"});
        files.insert(QStringLiteral("metafile"), {QStringLiteral("emf"), QByteArray::fromHex("010000005800000000000000")});
        // A format the name of which is wrong or unknown is told by its bytes.
        files.insert(QStringLiteral("unnamed"), {QString(), files.value(QStringLiteral("png")).second});
        r.ctx.imageBytes = [&](const QString &id, QString *format) {
            if (format) *format = files.value(id).first;
            return files.value(id).second;
        };
        ItemList list;
        for (auto it = files.cbegin(); it != files.cend(); ++it) {
            auto p = std::make_shared<PictureItem>();
            p->imageId = it.key();
            p->rect = QRectF(0, 0, 40, 20);
            p->imgRect = QRectF(0, 0, 40, 20);
            p->name = it.key();
            list.push_back(p);
        }
        QVERIFY2(r.run(list), qPrintable(r.error));
        QVERIFY2(r.skipped.isEmpty(), qPrintable(r.skipped.join('\n')));
        const QMap<QString, QPair<QString, QString>> want = {   // part's extension, type
            {"png", {"png", "image/png"}}, {"jpg", {"jpeg", "image/jpeg"}}, {"bmp", {"bmp", "image/bmp"}}, {"vector", {"png", "image/png"}},
            {"metafile", {"emf", "image/x-emf"}}, {"unnamed", {"png", "image/png"}}};
        QCOMPARE(r.media.size(), int(want.size()) + 1);   // and the vector picture's SVG, beside its pixels
        for (auto it = want.cbegin(); it != want.cend(); ++it) {
            const QString part = r.media.value(it.key());
            QVERIFY2(part.endsWith(QLatin1Char('.') + it.value().first), qPrintable(it.key() + ": " + part));
            QCOMPARE(r.env.package.contentType(part), it.value().second);
        }
        // A vector picture is drawn as pixels, and what was read back is a picture.
        QVERIFY(r.env.package.part(r.media.value(QStringLiteral("vector"))).startsWith("\x89PNG"));
        QImage drawn;
        QVERIFY(drawn.loadFromData(r.env.package.part(r.media.value(QStringLiteral("vector")))));
        QCOMPARE(QColor(drawn.pixel(drawn.width() / 2, drawn.height() / 2)), QColor(Qt::red));
        QCOMPARE(drawn.width() * 20, drawn.height() * 40);
        // And the SVG itself is linked from the blip, as the presentation program does, in a part of its own.
        const QString svgPart = r.media.value(QStringLiteral("svg:vector"));
        QVERIFY2(svgPart.endsWith(QLatin1String(".svg")), qPrintable(svgPart));
        QCOMPARE(r.env.package.contentType(svgPart), QStringLiteral("image/svg+xml"));
        QCOMPARE(r.env.package.part(svgPart), files.value(QStringLiteral("vector")).second);
        QCOMPARE(int(elementsNamed(r.spTree, "svgBlip").size()), 1);
        QSet<QString> parts;
        for (const QString &part : std::as_const(r.media)) parts << part;
        QCOMPARE(int(parts.size()), int(r.media.size()));   // every picture its own part
    }

    void dmlWritePicturesWithoutBytesAreLeftOutWithANote()
    {
        WriteRound r;
        auto p = std::make_shared<PictureItem>();
        p->imageId = QStringLiteral("gone");
        p->rect = QRectF(0, 0, 50, 50);
        QVERIFY2(r.run({p}), qPrintable(r.error));
        QCOMPARE(int(r.items.size()), 0);
        QCOMPARE(int(r.skipped.size()), 1);
        QVERIFY(r.media.isEmpty());
    }

    void dmlWritePicturePlaceholderIsAShapeWithAPictureSlot()
    {
        WriteRound r;
        auto p = std::make_shared<PictureItem>();
        p->role = QStringLiteral("ph:pic:13");
        p->rect = QRectF(10, 20, 300, 200);
        QVERIFY2(r.run({p}), qPrintable(r.error));
        const QVector<QDomElement> phs = elementsNamed(r.spTree, "ph");
        QCOMPARE(int(phs.size()), 1);
        QCOMPARE(phs[0].attribute(QStringLiteral("type")), QStringLiteral("pic"));
        QCOMPARE(phs[0].attribute(QStringLiteral("idx")), QStringLiteral("13"));
        QCOMPARE(int(elementsNamed(r.spTree, "pic").size()), 0);
        QCOMPARE(int(r.items.size()), 1);
        QCOMPARE(int(r.items[0]->type()), int(ItemType::Picture));
        QVERIFY(static_cast<PictureItem *>(r.items[0].get())->imageId.isEmpty());
        QVERIFY(sameRect(r.items[0]->rect, p->rect));
        QVERIFY(r.media.isEmpty());
    }

    void dmlWriteLinesKeepTheirEnds()
    {
        const QPointF a(10, 10), b(110, 60), c(10, 60), d(110, 10);
        const QPair<QPointF, QPointF> ends[] = {{a, b}, {b, a}, {c, d}, {d, c}, {a, d}, {a, c}};
        for (const auto &e : ends) {
            WriteRound r;
            auto l = std::make_shared<LineItem>();
            l->p1 = e.first;
            l->p2 = e.second;
            l->syncRect();
            l->stroke = Stroke::line(ColorRef::rgb(Qt::red), 2);
            l->stroke.startArrow = Arrow::Oval;
            l->stroke.endArrow = Arrow::Triangle;
            l->stroke.endSize = 2;
            l->name = QStringLiteral("Line");
            QVERIFY2(r.run({l}), qPrintable(r.error));
            QCOMPARE(int(r.items.size()), 1);
            QCOMPARE(int(r.items[0]->type()), int(ItemType::Line));
            const auto *back = static_cast<LineItem *>(r.items[0].get());
            QVERIFY2(near(back->p1.x(), e.first.x()) && near(back->p1.y(), e.first.y()), qPrintable(QStringLiteral("%1,%2").arg(e.first.x()).arg(e.first.y())));
            QVERIFY(near(back->p2.x(), e.second.x()) && near(back->p2.y(), e.second.y()));
            QVERIFY(back->route == LineItem::Straight);
            QVERIFY(near(back->stroke.width, 2));
            QVERIFY(back->stroke.startArrow == Arrow::Oval && back->stroke.endArrow == Arrow::Triangle && back->stroke.endSize == 2);
            QCOMPARE(int(elementsNamed(r.spTree, "cxnSp").size()), 1);
        }
    }

    void dmlWriteBentAndCurvedConnectorsKeepTheirRoute()
    {
        const QPair<QPointF, QPointF> ends[] = {{QPointF(10, 10), QPointF(110, 60)}, {QPointF(110, 60), QPointF(10, 10)},
                                                {QPointF(10, 60), QPointF(110, 10)}, {QPointF(110, 10), QPointF(10, 60)}};
        for (LineItem::Route route : {LineItem::Elbow, LineItem::Curved})
            for (const auto &e : ends)
                for (int sv = 0; sv < 2; ++sv)
                    for (int ev = 0; ev < 2; ++ev) {
                        WriteRound r;
                        auto l = std::make_shared<LineItem>();
                        l->p1 = e.first;
                        l->p2 = e.second;
                        l->route = route;
                        l->bend = 0.3;
                        l->startVertical = sv;
                        l->endVertical = ev;
                        l->syncRect();
                        const QString what = QStringLiteral("route %1 from %2,%3 to %4,%5 vertical %6 %7")
                                                 .arg(int(route)).arg(e.first.x()).arg(e.first.y()).arg(e.second.x()).arg(e.second.y()).arg(sv).arg(ev);
                        QVERIFY2(r.run({l}), qPrintable(r.error));
                        QVERIFY2(r.items.size() == 1 && r.items[0]->type() == ItemType::Line, qPrintable(what));
                        const auto *back = static_cast<LineItem *>(r.items[0].get());
                        QVERIFY2(back->route == route, qPrintable(what));
                        QVERIFY2(near(back->p1.x(), e.first.x()) && near(back->p1.y(), e.first.y()) && near(back->p2.x(), e.second.x()) && near(back->p2.y(), e.second.y()), qPrintable(what));
                        QVERIFY2(back->startVertical == bool(sv) && back->endVertical == bool(ev), qPrintable(what));
                        // The route's corners are where they were (a corner of one turn is the same corner).
                        const QVector<QPointF> was = l->routePoints(), now = back->routePoints();
                        QVERIFY2(was.size() == now.size(), qPrintable(what));
                        for (int i = 0; i < was.size(); ++i) QVERIFY2(near(was[i].x(), now[i].x(), 1e-4) && near(was[i].y(), now[i].y(), 1e-4), qPrintable(what));
                        if (sv == ev) QVERIFY2(near(back->bend, 0.3, 1e-4), qPrintable(what));
                    }
    }

    void dmlWriteConnectorsStayAttachedToTheirShapes()
    {
        auto box = std::make_shared<ShapeItem>();
        box->rect = QRectF(10, 10, 80, 40);
        auto circle = std::make_shared<ShapeItem>();
        circle->rect = QRectF(200, 100, 60, 60);
        circle->shape = QStringLiteral("ellipse");
        auto cloud = std::make_shared<ShapeItem>();
        cloud->rect = QRectF(300, 10, 60, 60);
        cloud->shape = QStringLiteral("cloud");
        auto late = std::make_shared<TextItem>();
        late->rect = QRectF(300, 200, 60, 60);
        auto l = std::make_shared<LineItem>();
        l->p1 = QPointF(90, 30);
        l->p2 = QPointF(200, 130);
        l->start = LineItem::Glue{box->id, 3};
        l->end = LineItem::Glue{circle->id, 0};
        l->syncRect();
        // The second is attached to a shape that comes after it, and to one that has no points to join to.
        auto early = std::make_shared<LineItem>();
        early->p1 = QPointF(0, 0);
        early->p2 = QPointF(50, 50);
        early->start = LineItem::Glue{late->id, 1};
        early->end = LineItem::Glue{cloud->id, 2};
        early->syncRect();
        const ItemList list{box, circle, l, early, cloud, late};
        // Written as they come, a connector is attached to what was written before it.
        {
            WriteRound r;
            r.run(list);
            QCOMPARE(int(r.items.size()), 6);
            const auto *back = static_cast<LineItem *>(r.items[2].get());
            QCOMPARE(back->start.id, r.items[0]->id);
            QCOMPARE(back->start.site, 3);
            QCOMPARE(back->end.id, r.items[1]->id);
            QCOMPARE(back->end.site, 0);
            const auto *loose = static_cast<LineItem *>(r.items[3].get());
            QVERIFY(loose->start.id.isEmpty() && loose->end.id.isEmpty());
            QVERIFY(r.text().contains(QStringLiteral("stCxn")));
        }
        // With the numbers given first, to what is written after it too.
        {
            WriteRound r;
            QVERIFY2(r.runWith([&](QXmlStreamWriter &w) {
                assignShapeIds(list, r.ctx);
                for (const ItemPtr &it : list) writeShape(w, *it, r.ctx);
            }), qPrintable(r.error));
            r.items = readShapeTree(r.spTree, r.env.ctx);
            QCOMPARE(int(r.items.size()), 6);
            const auto *early2 = static_cast<LineItem *>(r.items[3].get());
            QCOMPARE(early2->start.id, r.items[5]->id);
            QCOMPARE(early2->start.site, 1);
            QVERIFY(early2->end.id.isEmpty());   // a cloud has no points to join
            QCOMPARE(static_cast<LineItem *>(r.items[2].get())->start.id, r.items[0]->id);
            // The numbers are the ones the shapes are written with.
            QSet<int> numbers;
            for (const ItemPtr &it : r.items) numbers << it->sourceId;
            QCOMPARE(int(numbers.size()), 6);
        }
    }

    // What can't be written is given no number, so nothing is attached to it.
    void dmlWriteGivesNoNumberToWhatItCannotWrite()
    {
        WriteRound r;
        auto table = std::make_shared<jp::TableItem>();
        auto gone = std::make_shared<PictureItem>();
        gone->imageId = QStringLiteral("gone");
        auto empty = std::make_shared<GroupItem>();
        auto shape = std::make_shared<ShapeItem>();
        const ItemList list{table, gone, empty, shape};
        assignShapeIds(list, r.ctx);
        QCOMPARE(int(r.ctx.shapeIds.size()), 1);
        QVERIFY(r.ctx.shapeIds.contains(shape->id));
        QCOMPARE(r.ctx.shapeIds.value(shape->id), 2);
    }

    void dmlWriteGroupsKeepTheirChildren()
    {
        WriteRound r;
        auto text = writtenTextBox(r, QRectF(20, 30, 100, 40), QStringLiteral("In a group"));
        auto shape = std::make_shared<ShapeItem>();
        shape->rect = QRectF(150, 40, 60, 60);
        shape->shape = QStringLiteral("ellipse");
        auto a = std::make_shared<ShapeItem>(), b = std::make_shared<ShapeItem>();
        a->rect = QRectF(300, 300, 30, 30);
        b->rect = QRectF(340, 320, 30, 30);
        b->rotation = 45;
        auto inner = std::make_shared<GroupItem>();
        inner->children = {a, b};
        inner->syncRect();
        auto g = std::make_shared<GroupItem>();
        g->children = {text, shape, inner};
        g->syncRect();
        g->name = QStringLiteral("Group A");
        g->altText = QStringLiteral("Three things");
        QVERIFY2(r.run({g}), qPrintable(r.error));
        QCOMPARE(int(r.items.size()), 1);
        QCOMPARE(int(r.items[0]->type()), int(ItemType::Group));
        const auto *back = static_cast<GroupItem *>(r.items[0].get());
        QCOMPARE(back->name, QStringLiteral("Group A"));
        QCOMPARE(back->altText, QStringLiteral("Three things"));
        QCOMPARE(int(back->children.size()), 3);
        QVERIFY(sameRect(back->children[0]->rect, text->rect));
        QVERIFY(sameRect(back->children[1]->rect, shape->rect));
        QCOMPARE(static_cast<ShapeItem *>(back->children[1].get())->shape, QStringLiteral("ellipse"));
        QCOMPARE(int(back->children[2]->type()), int(ItemType::Group));
        const auto *innerBack = static_cast<GroupItem *>(back->children[2].get());
        QCOMPARE(int(innerBack->children.size()), 2);
        QVERIFY(sameRect(innerBack->children[0]->rect, a->rect));
        QVERIFY(sameRect(innerBack->children[1]->rect, b->rect));
        QVERIFY(near(innerBack->children[1]->rotation, 45, 1e-4));
        QVERIFY(sameRect(back->bounds(), g->bounds(), 1e-3));
        // The group's frame is its children's: the child space is the page's.
        const QVector<QDomElement> xfrms = elementsNamed(r.spTree, "chOff");
        QVERIFY(!xfrms.isEmpty());
        const QDomElement x = xfrms[0].parentNode().toElement();
        QCOMPARE(child(x, QString(ns::A), QStringLiteral("off")).attribute(QStringLiteral("x")), child(x, QString(ns::A), QStringLiteral("chOff")).attribute(QStringLiteral("x")));
        QCOMPARE(child(x, QString(ns::A), QStringLiteral("ext")).attribute(QStringLiteral("cx")), child(x, QString(ns::A), QStringLiteral("chExt")).attribute(QStringLiteral("cx")));
        // A group with nothing in it is left out.
        WriteRound empty;
        QVERIFY2(empty.run({std::make_shared<GroupItem>()}), qPrintable(empty.error));
        QCOMPARE(int(empty.items.size()), 0);
        QCOMPARE(int(empty.skipped.size()), 1);
    }

    void dmlWritePlaceholdersKeepTheirTypeAndIndex_data()
    {
        QTest::addColumn<QString>("role");
        QTest::addColumn<QString>("type");
        QTest::addColumn<QString>("idx");
        QTest::newRow("title") << "ph:title" << "title" << "";
        QTest::newRow("centered title") << "ph:ctrTitle" << "ctrTitle" << "";
        QTest::newRow("subtitle") << "ph:subTitle:1" << "subTitle" << "1";
        QTest::newRow("body") << "ph:body:1" << "body" << "1";
        QTest::newRow("content has no type") << "ph:obj" << "" << "";
        QTest::newRow("second content") << "ph:obj:2" << "" << "2";
        QTest::newRow("date") << "ph:dt:10" << "dt" << "10";
        QTest::newRow("footer") << "ph:ftr:11" << "ftr" << "11";
        QTest::newRow("slide number") << "ph:sldNum:12" << "sldNum" << "12";
        QTest::newRow("big index") << "ph:body:4294967295" << "body" << "4294967295";
    }

    void dmlWritePlaceholdersKeepTheirTypeAndIndex()
    {
        QFETCH(QString, role);
        QFETCH(QString, type);
        QFETCH(QString, idx);
        WriteRound r;
        auto t = writtenTextBox(r, QRectF(0, 0, 100, 50), QStringLiteral("x"));
        t->role = role;
        QVERIFY2(r.run({t}), qPrintable(r.error));
        const QVector<QDomElement> phs = elementsNamed(r.spTree, "ph");
        QCOMPARE(int(phs.size()), 1);
        QCOMPARE(phs[0].hasAttribute(QStringLiteral("type")), !type.isEmpty());
        QCOMPARE(phs[0].attribute(QStringLiteral("type")), type);
        QCOMPARE(phs[0].hasAttribute(QStringLiteral("idx")), !idx.isEmpty());
        QCOMPARE(phs[0].attribute(QStringLiteral("idx")), idx);
        // A placeholder is no text box, and its place in the layout is not to be moved by the group.
        QVERIFY(!r.text().contains(QStringLiteral("txBox")));
        QVERIFY(r.text().contains(QStringLiteral("noGrp=\"1\"")));
        QCOMPARE(int(r.items.size()), 1);
        QCOMPARE(int(r.items[0]->type()), int(ItemType::Text));
    }

    void dmlWriteOtherRolesAreNotPlaceholders()
    {
        WriteRound r;
        auto t = writtenTextBox(r, QRectF(0, 0, 100, 50), QStringLiteral("x"));
        t->role = QStringLiteral("title");
        QVERIFY2(r.run({t}), qPrintable(r.error));
        QCOMPARE(int(elementsNamed(r.spTree, "ph").size()), 0);
        QVERIFY(r.text().contains(QStringLiteral("txBox=\"1\"")));
    }

    void dmlWriteKeepsTheNumbersAFileGave()
    {
        WriteRound r;
        r.ctx.nextShapeId = 2;
        r.ctx.usedIds = {1, 5};
        r.ctx.reservedIds = {3, 7, 20, 21};
        auto shape = [&](int source) {
            auto s = std::make_shared<ShapeItem>();
            s->rect = QRectF(0, 0, 10, 10);
            s->sourceId = source;
            return s;
        };
        auto a = shape(7), fresh = shape(0), c = shape(3), duplicate = shape(7), one = shape(1), used = shape(5);
        auto e = shape(21), f = shape(0);
        auto g = std::make_shared<GroupItem>();
        g->children = {e, f};
        g->syncRect();
        g->sourceId = 20;
        QVERIFY2(r.run({a, fresh, c, duplicate, one, used, g}), qPrintable(r.error));
        // The numbers a file gave are kept where free; a new shape takes the next that no one has asked for.
        QCOMPARE(int(r.items.size()), 7);
        QCOMPARE(r.items[0]->sourceId, 7);
        QCOMPARE(r.items[1]->sourceId, 2);
        QCOMPARE(r.items[2]->sourceId, 3);
        QCOMPARE(r.items[3]->sourceId, 4);   // 7 was taken
        QCOMPARE(r.items[4]->sourceId, 6);   // 1 is the shape tree's own
        QCOMPARE(r.items[5]->sourceId, 8);   // 5 was taken before
        QCOMPARE(r.items[6]->sourceId, 20);
        const auto *back = static_cast<GroupItem *>(r.items[6].get());
        QCOMPARE(back->children[0]->sourceId, 21);
        QCOMPARE(back->children[1]->sourceId, 9);
        QSet<int> seen;
        for (const QDomElement &p : elementsNamed(r.spTree, "cNvPr")) {
            QVERIFY2(!seen.contains(p.attribute(QStringLiteral("id")).toInt()), qPrintable(p.attribute(QStringLiteral("id"))));
            seen << p.attribute(QStringLiteral("id")).toInt();
        }
        QCOMPARE(int(seen.size()), 9);
        QVERIFY(r.ctx.usedIds.contains(7) && r.ctx.usedIds.contains(21) && r.ctx.usedIds.contains(9));
        // What the context says each item got.
        QCOMPARE(r.ctx.shapeIds.value(a->id), 7);
        QCOMPARE(r.ctx.shapeIds.value(duplicate->id), 4);
        QCOMPARE(r.ctx.shapeIds.value(e->id), 21);
    }

    void dmlWriteNumbersNewShapesFromTheContext()
    {
        WriteRound r;
        r.ctx.nextShapeId = 100;
        auto a = std::make_shared<ShapeItem>();
        auto b = std::make_shared<LineItem>();
        auto t = writtenTextBox(r, QRectF(0, 0, 10, 10), QStringLiteral("x"));
        auto p = std::make_shared<PictureItem>();
        p->imageId = r.addPicture();
        QVERIFY2(r.run({a, b, t, p}), qPrintable(r.error));
        QCOMPARE(int(r.items.size()), 4);
        QCOMPARE(r.items[0]->sourceId, 100);
        QCOMPARE(r.items[1]->sourceId, 101);
        QCOMPARE(r.items[2]->sourceId, 102);
        QCOMPARE(r.items[3]->sourceId, 103);
        QCOMPARE(r.ctx.nextShapeId, 104);
    }

    // A table with no cells is left out with a note; text art is written as
    // a box whose text is warped into it.
    void dmlWriteLeavesTablesWithNoCellsOutWithANote()
    {
        WriteRound r;
        auto table = std::make_shared<jp::TableItem>();
        table->name = QStringLiteral("Prices");
        auto art = std::make_shared<TextArtItem>();
        art->text = QStringLiteral("Sale");
        QVERIFY2(r.run({table, art}), qPrintable(r.error));
        QCOMPARE(int(r.skipped.size()), 1);
        QVERIFY(r.skipped[0].contains(QStringLiteral("Prices")));
        const QVector<QDomElement> warps = elementsNamed(r.spTree, "prstTxWarp");
        QCOMPARE(int(warps.size()), 1);
        QCOMPARE(warps[0].attribute(QStringLiteral("prst")), QStringLiteral("textPlain"));
    }

    // A table is a frame of the cells' own fills, lines, and text; the cells
    // that are hidden under a merged one say how.
    void dmlWriteTablesKeepTheirCellsAndMerges()
    {
        WriteRound r;
        auto t = std::make_shared<jp::TableItem>();
        t->name = QStringLiteral("Table <1>");
        t->rect = QRectF(20, 30, 1, 1);
        t->rows = 3;
        t->cols = 3;
        t->colW = {100, 80, 60};
        t->rowH = {30, 30, 40};
        t->cells.resize(9);
        for (int row = 0; row < 3; ++row)
            for (int col = 0; col < 3; ++col) {
                jp::TableCell &c = t->cell(row, col);
                c.storyId = r.addStory(QStringLiteral("r%1c%2").arg(row).arg(col));
                c.margins = QMarginsF(5, 3, 5, 3);
            }
        t->cell(0, 0).colSpan = 2;
        t->cell(0, 0).rowSpan = 2;
        for (const auto &at : {QPair<int, int>{0, 1}, {1, 0}, {1, 1}}) t->cell(at.first, at.second).covered = true;
        t->cell(0, 0).fill = Fill::solid(ColorRef::rgb(QColor(0xFF, 0, 0)));
        t->cell(0, 0).border.top = Stroke::line(ColorRef::rgb(QColor(0, 0, 0xFF)), 2);
        t->cell(1, 2).border.left = Stroke::line(ColorRef::rgb(QColor(0, 0x80, 0)), 1);   // and the cell on its left, which has none, shares it
        jp::TableCell &last = t->cell(2, 2);
        last.margins = QMarginsF(1, 2, 3, 4);
        last.valign = jp::VAlign::Bottom;
        last.vertical = true;
        last.diagonal = 1;
        last.diagonalLine = Stroke::line(ColorRef::rgb(Qt::black), 0.5);
        t->fill = Fill::solid(ColorRef::rgb(QColor(0xEE, 0xEE, 0xEE)));
        t->stroke = Stroke::line(ColorRef::rgb(QColor(0x10, 0x20, 0x30)), 3);
        QVERIFY2(r.run({t}), qPrintable(r.error));
        QVERIFY2(r.skipped.isEmpty(), qPrintable(r.skipped.join('\n')));
        // The frame's place and the table's size, from its columns and rows.
        const QDomElement frame = elementsNamed(r.spTree, "graphicFrame").value(0);
        QVERIFY(!frame.isNull());
        const QDomElement xfrm = child(frame, QString(ns::P), QStringLiteral("xfrm"));
        QCOMPARE(child(xfrm, QString(ns::A), QStringLiteral("off")).attribute(QStringLiteral("x")), QStringLiteral("254000"));
        QCOMPARE(child(xfrm, QString(ns::A), QStringLiteral("ext")).attribute(QStringLiteral("cx")), QStringLiteral("3048000"));
        QCOMPARE(child(xfrm, QString(ns::A), QStringLiteral("ext")).attribute(QStringLiteral("cy")), QStringLiteral("1270000"));
        QCOMPARE(child(child(frame, QString(ns::P), QStringLiteral("nvGraphicFramePr")), QString(ns::P), QStringLiteral("cNvPr")).attribute(QStringLiteral("name")), QStringLiteral("Table <1>"));
        QStringList widths, heights;
        for (const QDomElement &g : elementsNamed(frame, "gridCol")) widths << g.attribute(QStringLiteral("w"));
        for (const QDomElement &row : elementsNamed(frame, "tr")) heights << row.attribute(QStringLiteral("h"));
        QCOMPARE(widths, (QStringList{"1270000", "1016000", "762000"}));
        QCOMPARE(heights, (QStringList{"381000", "381000", "508000"}));
        const QVector<QDomElement> cells = elementsNamed(frame, "tc");
        QCOMPARE(int(cells.size()), 9);
        // A merged cell says how far it reaches; those under it say that they are, across or down or both.
        QCOMPARE(cells[0].attribute(QStringLiteral("gridSpan")), QStringLiteral("2"));
        QCOMPARE(cells[0].attribute(QStringLiteral("rowSpan")), QStringLiteral("2"));
        QVERIFY(cells[1].attribute(QStringLiteral("hMerge")) == QLatin1String("1") && !cells[1].hasAttribute(QStringLiteral("vMerge")));
        QVERIFY(cells[3].attribute(QStringLiteral("vMerge")) == QLatin1String("1") && !cells[3].hasAttribute(QStringLiteral("hMerge")));
        QVERIFY(cells[4].attribute(QStringLiteral("hMerge")) == QLatin1String("1") && cells[4].attribute(QStringLiteral("vMerge")) == QLatin1String("1"));
        QVERIFY(!cells[2].hasAttribute(QStringLiteral("gridSpan")) && !cells[2].hasAttribute(QStringLiteral("hMerge")));
        auto textOf = [&](int i) {
            QString out;
            for (const QDomElement &tt : elementsNamed(cells[i], "t")) out += tt.text();
            return out;
        };
        QCOMPARE(textOf(0), QStringLiteral("r0c0"));
        QCOMPARE(textOf(5), QStringLiteral("r1c2"));
        QCOMPARE(textOf(1), QString());   // hidden: no text
        auto props = [&](int i) { return child(cells[i], QString(ns::A), QStringLiteral("tcPr")); };
        auto line = [&](int i, const char *side) { return child(props(i), QString(ns::A), QString::fromLatin1(side)); };
        auto fillColor = [&](int i) { return child(child(props(i), QString(ns::A), QStringLiteral("solidFill")), QString(ns::A), QStringLiteral("srgbClr")).attribute(QStringLiteral("val")); };
        // Fills: its own, and the table's where it has none.
        QCOMPARE(fillColor(0), QStringLiteral("FF0000"));
        QCOMPARE(fillColor(2), QStringLiteral("EEEEEE"));
        // Lines: the cell's own, the table's round its edge, and one shared with the cell next to it.
        QCOMPARE(line(0, "lnT").attribute(QStringLiteral("w")), QStringLiteral("38100"));   // the table's own line is on its edge, over the cell's
        QCOMPARE(child(child(line(5, "lnL"), QString(ns::A), QStringLiteral("solidFill")), QString(ns::A), QStringLiteral("srgbClr")).attribute(QStringLiteral("val")), QStringLiteral("008000"));
        QVERIFY(!child(line(5, "lnT"), QString(ns::A), QStringLiteral("noFill")).isNull());
        QCOMPARE(line(2, "lnR").attribute(QStringLiteral("w")), QStringLiteral("38100"));
        // The text's place in the cell.
        QCOMPARE(props(8).attribute(QStringLiteral("marL")), QStringLiteral("12700"));
        QCOMPARE(props(8).attribute(QStringLiteral("marT")), QStringLiteral("25400"));
        QCOMPARE(props(8).attribute(QStringLiteral("marR")), QStringLiteral("38100"));
        QCOMPARE(props(8).attribute(QStringLiteral("marB")), QStringLiteral("50800"));
        QCOMPARE(props(8).attribute(QStringLiteral("anchor")), QStringLiteral("b"));
        QCOMPARE(props(8).attribute(QStringLiteral("vert")), QStringLiteral("vert"));
        QVERIFY(!line(8, "lnTlToBr").isNull() && line(8, "lnBlToTr").isNull());
        QCOMPARE(props(0).attribute(QStringLiteral("anchor")), QStringLiteral("t"));
        // The table is no shape that is read back as one.
        QCOMPARE(int(r.items.size()), 0);
    }

    void dmlWriteElementsComeInTheSchemasOrder()
    {
        WriteRound r;
        const QString id = r.addStory();
        QTextCursor c(r.story(id));
        QTextCharFormat cf;
        cf.setFontPointSize(18);
        cf.setFontWeight(QFont::Bold);
        cf.setFontFamilies({QStringLiteral("Carlito")});
        cf.setForeground(QColor(1, 2, 3));
        cf.setProperty(jp::tp::ColorRefP, QStringLiteral("#010203"));
        cf.setProperty(jp::tp::HighlightRefP, QStringLiteral("#FFFF00"));
        cf.setAnchor(true);
        cf.setAnchorHref(QStringLiteral("https://example.com/"));
        QTextBlockFormat bf;
        bf.setLineHeight(110, QTextBlockFormat::ProportionalHeight);
        bf.setTopMargin(3);
        bf.setBottomMargin(4);
        bf.setTabPositions({QTextOption::Tab(50, QTextOption::LeftTab)});
        c.setBlockFormat(bf);
        c.insertText(QStringLiteral("rich"), cf);
        QTextListFormat lf;
        lf.setStyle(QTextListFormat::ListDisc);
        lf.setProperty(jp::tp::BulletFont, QStringLiteral("Arial"));
        lf.setProperty(jp::tp::BulletColor, QStringLiteral("#FF0000"));
        lf.setProperty(jp::tp::BulletSize, 24.0);
        c.createList(lf);
        c.insertBlock();
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(0, 0, 200, 100);
        t->storyId = id;
        t->role = QStringLiteral("ph:body:1");
        t->hyperlink = QStringLiteral("https://example.com/item");
        t->fill = Fill::gradient(ColorRef::rgb(Qt::red), ColorRef::rgb(Qt::blue));
        t->stroke = Stroke::line(ColorRef::rgb(Qt::black), 2);
        t->stroke.dash = Stroke::DashLine;
        t->stroke.join = Qt::RoundJoin;
        t->stroke.startArrow = Arrow::Oval;
        t->stroke.endArrow = Arrow::Triangle;
        t->fx.shadow.on = true;
        t->fx.glow.on = true;
        t->fx.softEdge = 2;
        t->autofit = TextItem::ShrinkOnOverflow;
        auto s = std::make_shared<ShapeItem>();
        s->rect = QRectF(250, 0, 100, 100);
        s->shape = QStringLiteral("roundRect");
        s->adj = {0.3};
        s->storyId = r.addStory(QStringLiteral("shape text"));
        auto p = std::make_shared<PictureItem>();
        p->rect = QRectF(400, 0, 100, 100);
        p->imageId = r.addPicture();
        p->imgRect = QRectF(-10, -10, 120, 120);
        p->hyperlink = QStringLiteral("https://example.com/pic");
        auto l = std::make_shared<LineItem>();
        l->p1 = QPointF(0, 150);
        l->p2 = QPointF(100, 250);
        l->route = LineItem::Elbow;
        auto g = std::make_shared<GroupItem>();
        g->children = {std::make_shared<ShapeItem>()};
        g->syncRect();
        auto free = std::make_shared<ShapeItem>();
        free->rect = QRectF(0, 300, 50, 50);
        QPainterPath path;
        path.addEllipse(0, 0, 50, 50);
        free->customPath = path;
        Fill picFill;
        picFill.type = Fill::Picture;
        picFill.imageId = p->imageId;
        free->fill = picFill;
        auto table = std::make_shared<jp::TableItem>();
        table->rows = 2;
        table->cols = 2;
        table->colW = {50, 50};
        table->rowH = {20, 20};
        table->cells.resize(4);
        for (jp::TableCell &cell : table->cells) {
            cell.storyId = r.addStory(QStringLiteral("cell"));
            cell.fill = Fill::gradient(ColorRef::rgb(Qt::red), ColorRef::rgb(Qt::blue));
            cell.border.left = cell.border.right = cell.border.top = cell.border.bottom = Stroke::line(ColorRef::rgb(Qt::black), 1);
        }
        table->cell(0, 0).diagonal = 2;
        table->cell(0, 0).diagonalLine = Stroke::line(ColorRef::rgb(Qt::black), 1);
        QVERIFY2(r.run({t, s, p, l, g, free, table}), qPrintable(r.error));
        const QMap<QString, QStringList> orders = {
            {"sp", {"nvSpPr", "spPr", "style", "txBody"}},
            {"pic", {"nvPicPr", "blipFill", "spPr", "style"}},
            {"cxnSp", {"nvCxnSpPr", "spPr", "style"}},
            {"grpSp", {"nvGrpSpPr", "grpSpPr", "sp|grpSp|graphicFrame|cxnSp|pic"}},
            {"nvSpPr", {"cNvPr", "cNvSpPr", "nvPr"}},
            {"nvPicPr", {"cNvPr", "cNvPicPr", "nvPr"}},
            {"nvCxnSpPr", {"cNvPr", "cNvCxnSpPr", "nvPr"}},
            {"nvGrpSpPr", {"cNvPr", "cNvGrpSpPr", "nvPr"}},
            {"cNvPr", {"hlinkClick", "hlinkHover"}},
            {"spPr", {"xfrm", "custGeom|prstGeom", "noFill|solidFill|gradFill|blipFill|pattFill|grpFill", "ln", "effectLst"}},
            {"grpSpPr", {"xfrm"}},
            {"xfrm", {"off", "ext", "chOff", "chExt"}},
            {"custGeom", {"avLst", "gdLst", "ahLst", "cxnLst", "rect", "pathLst"}},
            {"ln", {"noFill|solidFill|gradFill|pattFill", "prstDash|custDash", "round|bevel|miter", "headEnd", "tailEnd"}},
            {"effectLst", {"glow", "innerShdw", "outerShdw", "softEdge"}},
            {"gradFill", {"gsLst", "lin|path"}},
            {"blipFill", {"blip", "srcRect", "tile|stretch"}},
            {"txBody", {"bodyPr", "lstStyle", "p"}},
            {"bodyPr", {"noAutofit|normAutofit|spAutoFit"}},
            {"p", {"pPr", "r|br|fld", "endParaRPr"}},
            {"pPr", {"lnSpc", "spcBef", "spcAft", "buClrTx|buClr", "buSzTx|buSzPct|buSzPts", "buFontTx|buFont", "buNone|buAutoNum|buChar|buBlip", "tabLst", "defRPr"}},
            {"rPr", {"ln", "noFill|solidFill|gradFill|blipFill|pattFill|grpFill", "effectLst", "highlight", "uLnTx|uLn", "uFillTx|uFill", "latin", "ea", "cs", "sym", "hlinkClick|hlinkMouseOver", "rtl"}},
            {"r", {"rPr", "t"}},
            {"graphicFrame", {"nvGraphicFramePr", "xfrm", "graphic"}},
            {"nvGraphicFramePr", {"cNvPr", "cNvGraphicFramePr", "nvPr"}},
            {"tbl", {"tblPr", "tblGrid", "tr"}},
            {"tc", {"txBody", "tcPr"}},
            {"tcPr", {"lnL", "lnR", "lnT", "lnB", "lnTlToBr", "lnBlToTr", "noFill|solidFill|gradFill|blipFill|pattFill|grpFill"}},
            {"lnL", {"noFill|solidFill|gradFill|pattFill", "prstDash|custDash", "round|bevel|miter", "headEnd", "tailEnd"}},
        };
        QVector<QDomElement> stack{r.spTree};
        int checked = 0;
        while (!stack.isEmpty()) {
            const QDomElement e = stack.takeLast();
            const auto rule = orders.constFind(e.localName());
            if (rule != orders.constEnd()) {
                const QStringList names = childNamesOf(e);
                QVERIFY2(inSequence(names, *rule), qPrintable(e.localName() + ": " + names.join(' ')));
                ++checked;
            }
            for (QDomElement k = e.firstChildElement(); !k.isNull(); k = k.nextSiblingElement()) stack << k;
        }
        QVERIFY(checked > 40);
        // The parts every element must have.
        for (const QDomElement &p2 : elementsNamed(r.spTree, "cNvPr")) {
            QVERIFY(p2.hasAttribute(QStringLiteral("id")));
            QVERIFY(p2.hasAttribute(QStringLiteral("name")));
        }
        for (const QDomElement &e : elementsNamed(r.spTree, "fld")) QVERIFY(e.hasAttribute(QStringLiteral("id")));
        for (const QDomElement &e : elementsNamed(r.spTree, "xfrm")) QVERIFY(child(e, QString(ns::A), QStringLiteral("off")).hasAttribute(QStringLiteral("x")));
    }

    void dmlWriteTheFrameOfAnItemIsInEmuAndWhole()
    {
        WriteRound r;
        auto s = std::make_shared<ShapeItem>();
        s->rect = QRectF(-12.5, 1e9, -5, 0.00001);   // a shape out of the page, and one with no size
        s->rotation = -90.25;
        s->flipV = true;
        QVERIFY2(r.run({s}), qPrintable(r.error));
        const QDomElement xfrm = child(child(elementsNamed(r.spTree, "sp").value(0), QString(ns::P), QStringLiteral("spPr")), QString(ns::A), QStringLiteral("xfrm"));
        QVERIFY(!xfrm.isNull());
        bool ok = false;
        const qint64 rot = xfrm.attribute(QStringLiteral("rot")).toLongLong(&ok);
        QVERIFY(ok && rot >= 0 && rot < 21600000);
        QVERIFY(near(rot / 60000.0, 269.75, 1e-6));
        QCOMPARE(xfrm.attribute(QStringLiteral("flipV")), QStringLiteral("1"));
        QVERIFY(!xfrm.hasAttribute(QStringLiteral("flipH")));
        const QDomElement ext = child(xfrm, QString(ns::A), QStringLiteral("ext"));
        QVERIFY(ext.attribute(QStringLiteral("cx")).toLongLong() >= 0);
        QVERIFY(ext.attribute(QStringLiteral("cy")).toLongLong() >= 0);
        QCOMPARE(child(xfrm, QString(ns::A), QStringLiteral("off")).attribute(QStringLiteral("x")), QStringLiteral("-158750"));
    }

    // A context with nothing in it (no package, no pictures, no stories) writes what it can, and leaves out the rest.
    void dmlWriteWithAnEmptyContextWritesWhatItCan()
    {
        DrawingWriteContext ctx;
        ctx.part = QStringLiteral("/ppt/slides/slide1.xml");
        QStringList skipped;
        QByteArray out;
        QXmlStreamWriter w(&out);
        w.writeStartElement(QStringLiteral("p:spTree"));
        auto shape = std::make_shared<ShapeItem>();
        shape->hyperlink = QStringLiteral("https://example.com/");
        auto text = std::make_shared<TextItem>();
        text->storyId = QStringLiteral("none");
        auto pic = std::make_shared<PictureItem>();
        pic->imageId = QStringLiteral("none");
        auto line = std::make_shared<LineItem>();
        auto group = std::make_shared<GroupItem>();
        group->children = {shape, text, pic};
        for (const ItemPtr &it : ItemList{shape, text, pic, line, group}) writeShape(w, *it, ctx, &skipped);
        writeShape(w, *shape, ctx);   // and no list of what is left out
        w.writeEndElement();
        QDomDocument dom;
        QVERIFY(parseXml("<p:spTree " + dmlNamespaces().toUtf8() + ">" + out.mid(out.indexOf('>') + 1), &dom));
        QCOMPARE(int(skipped.size()), 2);   // the picture, and the one inside the group
        QVERIFY(!out.contains("hlinkClick"));
        QVERIFY(!out.contains("r:embed"));
        // The text of a story that isn't there is one empty paragraph.
        QVERIFY(out.contains("<a:p><a:endParaRPr"));
    }

    // Numbers that aren't numbers, or are far out of reach, do not make a file that can't be read.
    void dmlWriteHoldsOddNumbersToWhatAFileCanHold()
    {
        const double nan = std::numeric_limits<double>::quiet_NaN(), inf = std::numeric_limits<double>::infinity();
        WriteRound r;
        auto s = std::make_shared<ShapeItem>();
        s->rect = QRectF(nan, inf, -inf, 1e300);
        s->rotation = nan;
        s->shape = QStringLiteral("roundRect");
        s->adj = {nan};
        s->stroke = Stroke::line(ColorRef::rgb(Qt::black), inf);
        s->stroke.startSize = 99;
        s->stroke.startArrow = Arrow::Oval;
        s->fx.shadow.on = true;
        s->fx.shadow.blur = nan;
        s->fx.shadow.distance = 1e300;
        s->fx.shadow.angle = inf;
        s->fx.softEdge = -5;
        s->fx.glow.on = true;
        s->fx.glow.size = nan;
        QPainterPath p;
        p.moveTo(nan, 0);
        p.lineTo(1e300, -1e300);
        p.lineTo(5, 5);
        s->customPath = p;
        auto t = writtenTextBox(r, QRectF(0, 0, 100, 50), QStringLiteral("x"));
        t->insets = QMarginsF(nan, inf, -1e300, 1e300);
        t->fitScale = nan;
        t->autofit = TextItem::ShrinkOnOverflow;
        t->columns = 1000000;
        t->columnGap = nan;
        QTextCursor c(r.story(t->storyId));
        QTextCharFormat cf;
        cf.setFontPointSize(1e300);
        cf.setProperty(jp::tp::KernAbove, nan);
        cf.setFontLetterSpacingType(QFont::AbsoluteSpacing);
        cf.setFontLetterSpacing(inf);
        c.insertText(QStringLiteral("odd"), cf);
        QTextBlockFormat bf;
        bf.setLineHeight(1e300, QTextBlockFormat::ProportionalHeight);
        bf.setTopMargin(inf);
        bf.setLeftMargin(-inf);
        c.setBlockFormat(bf);
        auto l = std::make_shared<LineItem>();
        l->p1 = QPointF(nan, nan);
        l->p2 = QPointF(inf, -inf);
        l->route = LineItem::Elbow;
        l->bend = nan;
        auto pic = std::make_shared<PictureItem>();
        pic->imageId = r.addPicture();
        pic->rect = QRectF(nan, nan, nan, nan);
        pic->imgRect = QRectF(0, 0, 1e-300, 1e300);
        pic->brightness = nan;
        pic->transparency = inf;
        QVERIFY2(r.run({s, t, l, pic}), qPrintable(r.error));
        QVERIFY(!r.text().contains(QLatin1String("nan")) && !r.text().contains(QLatin1String("inf")));
        for (const QDomElement &e : elementsNamed(r.spTree, "ext")) {
            QVERIFY(e.attribute(QStringLiteral("cx")).toLongLong() >= 0);
            QVERIFY(e.attribute(QStringLiteral("cy")).toLongLong() >= 0);
        }
    }

    void itemSourceIdIsKeptInJsonAndNotByCopies()
    {
        auto s = std::make_shared<ShapeItem>();
        QVERIFY(!s->toJson().contains(QStringLiteral("srcId")));
        s->sourceId = 42;
        QCOMPARE(s->toJson().value(QStringLiteral("srcId")).toInt(), 42);
        const ItemPtr back = jp::Item::fromJsonAny(s->toJson());
        QCOMPARE(back->sourceId, 42);
        // A copy is a new shape.
        QCOMPARE(s->clone()->sourceId, 0);
        auto t = std::make_shared<TextItem>();
        t->sourceId = 9;
        QCOMPARE(t->clone()->sourceId, 0);
        auto g = std::make_shared<GroupItem>();
        g->children = {s};
        g->sourceId = 3;
        const ItemPtr copy = g->clone();
        QCOMPARE(copy->sourceId, 0);
        QCOMPARE(static_cast<GroupItem *>(copy.get())->children[0]->sourceId, 0);
    }

    void dmlReadKeepsTheNumbersShapesHad()
    {
        DmlEnv env;
        ItemPtr sp = readShape(env.element(dmlSp(dmlXfrm(0, 0, 10, 10) + QStringLiteral("<a:prstGeom prst=\"ellipse\"/>"), QString(), QString(), QString(), QString(), QString(), QStringLiteral("17"))), env.ctx);
        QVERIFY(sp);
        QCOMPARE(sp->sourceId, 17);
        ItemPtr text = readShape(env.element(dmlSp(dmlXfrm(0, 0, 10, 10), dmlBody(), QString(), QStringLiteral("txBox=\"1\""), QString(), QString(), QStringLiteral("23"))), env.ctx);
        QVERIFY(text);
        QCOMPARE(text->sourceId, 23);
        ItemPtr group = readShape(env.element(QStringLiteral("<p:grpSp><p:nvGrpSpPr><p:cNvPr id=\"30\" name=\"G\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr>%1</p:grpSpPr>%2</p:grpSp>")
                                                .arg(dmlXfrm(0, 0, 10, 10).replace(QStringLiteral("</a:xfrm>"), QStringLiteral("<a:chOff x=\"0\" y=\"0\"/><a:chExt cx=\"127000\" cy=\"127000\"/></a:xfrm>")),
                                                     dmlSp(dmlXfrm(0, 0, 5, 5) + QStringLiteral("<a:prstGeom prst=\"ellipse\"/>"), QString(), QString(), QString(), QString(), QString(), QStringLiteral("31")))), env.ctx);
        QVERIFY(group);
        QCOMPARE(group->sourceId, 30);
        QCOMPARE(static_cast<GroupItem *>(group.get())->children[0]->sourceId, 31);
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

    // ---- DrawingML tables

    // A 3 by 3 table: the first cell spans two columns and the first of the next row two rows; cells with
    // fills, borders, margins, and anchors of their own, and a bold text cell. No style, so what the cells
    // say is all there is.
    void dmlTableReadsStructureMergesFillsAndBorders()
    {
        DmlEnv env;
        const QString line = QStringLiteral("<a:solidFill><a:srgbClr val=\"%1\"/></a:solidFill>");
        const QString rows =
            QStringLiteral("<a:tr h=\"%1\">").arg(emu(30))
            + dmlTc(QStringLiteral("Total"), QStringLiteral("gridSpan=\"2\""),
                    QStringLiteral("<a:lnT><a:noFill/></a:lnT><a:lnB w=\"25400\">%1</a:lnB>").arg(line.arg(QStringLiteral("0000FF"))) + dmlSolid(QStringLiteral("FF0000")),
                    QStringLiteral("marL=\"127000\" marR=\"63500\" marT=\"38100\" marB=\"25400\" anchor=\"ctr\""), QStringLiteral("b=\"1\""))
            + dmlTc(QString(), QStringLiteral("hMerge=\"1\"")) + dmlTc(QStringLiteral("C0")) + QStringLiteral("</a:tr>")
            + QStringLiteral("<a:tr h=\"%1\">").arg(emu(30))
            + dmlTc(QStringLiteral("tall"), QStringLiteral("rowSpan=\"2\""), QString(), QStringLiteral("anchor=\"b\" vert=\"vert\""))
            + dmlTc(QStringLiteral("E1"))
            + dmlTc(QStringLiteral("F1"), QString(),
                    QStringLiteral("<a:lnR w=\"12700\">%1<a:prstDash val=\"dash\"/></a:lnR><a:lnTlToBr w=\"12700\">%2</a:lnTlToBr>").arg(line.arg(QStringLiteral("00FF00")), line.arg(QStringLiteral("808080"))))
            + QStringLiteral("</a:tr>")
            + QStringLiteral("<a:tr h=\"%1\">").arg(emu(30))
            + dmlTc(QString(), QStringLiteral("vMerge=\"1\"")) + dmlTc(QStringLiteral("H2")) + dmlTc(QStringLiteral("I2"), QString(), dmlSolid(QStringLiteral("00FF00")))
            + QStringLiteral("</a:tr>");
        const QString grid = QStringLiteral("<a:tblGrid><a:gridCol w=\"%1\"/><a:gridCol w=\"%2\"/><a:gridCol w=\"%3\"/></a:tblGrid>").arg(emu(100), emu(150), emu(50));
        auto t = dmlAs<TableItem>(readTable(env.element(dmlTableFrame(QStringLiteral("<a:tblPr/>"), grid, rows, 36, 72, 300, 90)), env.ctx));
        QVERIFY(t);
        QCOMPARE(t->rows, 3);
        QCOMPARE(t->cols, 3);
        QCOMPARE(int(t->cells.size()), 9);
        QVERIFY2(dmlRectIs(t->rect, 36, 72, 300, 90), qPrintable(dmlRectText(t->rect)));
        QCOMPARE(t->colW, (QVector<double>{100, 150, 50}));
        QCOMPARE(t->rowH, (QVector<double>{30, 30, 30}));
        QCOMPARE(t->name, QStringLiteral("Table 3"));
        QCOMPARE(t->altText, QStringLiteral("Figures"));
        QVERIFY(t->wrap.mode == jp::Wrap::None);
        QVERIFY(!t->header && !t->banded);
        QVERIFY(t->format.isEmpty());
        QVERIFY(t->fill.isNone() && t->stroke.isNone());

        // Merges: the cell that spans carries the span, the cells under it are covered.
        QCOMPARE(t->cell(0, 0).colSpan, 2);
        QCOMPARE(t->cell(0, 0).rowSpan, 1);
        QVERIFY(!t->cell(0, 0).covered && t->cell(0, 1).covered && !t->cell(0, 2).covered);
        QCOMPARE(t->cell(1, 0).rowSpan, 2);
        QCOMPARE(t->cell(1, 0).colSpan, 1);
        QVERIFY(!t->cell(1, 0).covered && t->cell(2, 0).covered && !t->cell(2, 1).covered);
        QVERIFY2(dmlRectIs(t->cellRect(0, 0), 0, 0, 250, 30), qPrintable(dmlRectText(t->cellRect(0, 0))));
        QVERIFY2(dmlRectIs(t->cellRect(1, 0), 0, 30, 100, 60), qPrintable(dmlRectText(t->cellRect(1, 0))));

        // Text: each cell has a story, the bold run is bold.
        const auto text = [&](int r, int c) { return env.ctx.storyDoc(t->cell(r, c).storyId)->toPlainText(); };
        QCOMPARE(text(0, 0), QStringLiteral("Total"));
        QCOMPARE(text(0, 2), QStringLiteral("C0"));
        QCOMPARE(text(2, 2), QStringLiteral("I2"));
        QVERIFY(text(0, 1).isEmpty());
        const auto weight = [&](int r, int c) { return env.ctx.storyDoc(t->cell(r, c).storyId)->begin().begin().fragment().charFormat().fontWeight(); };
        QCOMPARE(weight(0, 0), int(QFont::Bold));
        QVERIFY(weight(0, 2) < int(QFont::Bold));

        // Margins and anchors; a cell that says nothing has the usual ones.
        QCOMPARE(t->cell(0, 0).margins, QMarginsF(10, 3, 5, 2));
        QCOMPARE(t->cell(0, 2).margins, QMarginsF(7.2, 3.6, 7.2, 3.6));
        QVERIFY(t->cell(0, 0).valign == jp::VAlign::Middle);
        QVERIFY(t->cell(1, 0).valign == jp::VAlign::Bottom && t->cell(1, 0).vertical);
        QVERIFY(t->cell(0, 2).valign == jp::VAlign::Top && !t->cell(0, 2).vertical);
        QVERIFY(!t->cell(0, 0).hyphenate);

        // Fills: explicit ones, and none where there are none.
        QVERIFY(t->cell(0, 0).fill.type == Fill::Solid);
        QCOMPARE(dmlHex(t->cell(0, 0).fill.color), QStringLiteral("FF0000"));
        QCOMPARE(dmlHex(t->cell(2, 2).fill.color), QStringLiteral("00FF00"));
        QVERIFY(t->cell(0, 2).fill.isNone() && t->cell(1, 1).fill.isNone());

        // Borders: where they are, and none where they aren't or say so.
        QVERIFY(!t->cell(0, 0).border.bottom.isNone());
        QVERIFY(near(t->cell(0, 0).border.bottom.width, 2));
        QCOMPARE(dmlHex(t->cell(0, 0).border.bottom.color), QStringLiteral("0000FF"));
        QVERIFY(t->cell(0, 0).border.top.isNone() && t->cell(0, 0).border.left.isNone() && t->cell(0, 0).border.right.isNone());
        QVERIFY(near(t->cell(1, 2).border.right.width, 1));
        QCOMPARE(dmlHex(t->cell(1, 2).border.right.color), QStringLiteral("00FF00"));
        QVERIFY(t->cell(1, 2).border.right.dash == Stroke::DashLine);
        // A diagonal is a cell's diagonal line.
        QCOMPARE(t->cell(1, 2).diagonal, 1);
        QCOMPARE(dmlHex(t->cell(1, 2).diagonalLine.color), QStringLiteral("808080"));
        QCOMPARE(t->cell(1, 1).diagonal, 0);

        // The grid, not the frame, says how big the table is.
        auto big = dmlAs<TableItem>(readTable(env.element(dmlTableFrame(QStringLiteral("<a:tblPr firstRow=\"1\" bandRow=\"1\"/>"), grid, rows, 10, 20, 500, 500)), env.ctx));
        QVERIFY(big);
        QVERIFY2(dmlRectIs(big->rect, 10, 20, 300, 90), qPrintable(dmlRectText(big->rect)));
        QVERIFY(big->header && big->banded);
    }

    // The table styles part gives the cells their fills, borders, and text; what a cell says itself wins.
    void dmlTableAppliesItsStyleToTheCells()
    {
        const QString ln = QStringLiteral("<a:ln w=\"%1\"><a:solidFill><a:srgbClr val=\"%2\"/></a:solidFill></a:ln>");
        const QString id = QStringLiteral("{11111111-1111-1111-1111-111111111111}");
        const QString styles = QStringLiteral(
            "<a:tblStyleLst %1 def=\"%2\"><a:tblStyle styleId=\"%2\" styleName=\"Test Style\">"
            "<a:wholeTbl><a:tcTxStyle><a:fontRef idx=\"minor\"><a:prstClr val=\"black\"/></a:fontRef><a:srgbClr val=\"202020\"/></a:tcTxStyle>"
            "<a:tcStyle><a:tcBdr><a:left>%3</a:left><a:right>%3</a:right><a:top>%3</a:top><a:bottom>%3</a:bottom><a:insideH>%4</a:insideH><a:insideV>%5</a:insideV></a:tcBdr>"
            "<a:fill>%6</a:fill></a:tcStyle></a:wholeTbl>"
            "<a:band1H><a:tcStyle><a:tcBdr/><a:fill>%7</a:fill></a:tcStyle></a:band1H><a:band2H><a:tcStyle><a:tcBdr/></a:tcStyle></a:band2H>"
            "<a:lastRow><a:tcTxStyle b=\"on\"><a:fontRef idx=\"minor\"/><a:srgbClr val=\"FFFF00\"/></a:tcTxStyle><a:tcStyle><a:tcBdr><a:top>%8</a:top></a:tcBdr><a:fill>%9</a:fill></a:tcStyle></a:lastRow>"
            "<a:firstCol><a:tcTxStyle b=\"on\" i=\"on\"><a:fontRef idx=\"minor\"/><a:srgbClr val=\"FFFFFF\"/></a:tcTxStyle><a:tcStyle><a:tcBdr/></a:tcStyle></a:firstCol>"
            "<a:firstRow><a:tcTxStyle b=\"on\"><a:fontRef idx=\"minor\"/><a:srgbClr val=\"FFFFFF\"/></a:tcTxStyle><a:tcStyle><a:tcBdr><a:bottom>%10</a:bottom></a:tcBdr><a:fill>%11</a:fill></a:tcStyle></a:firstRow>"
            "</a:tblStyle></a:tblStyleLst>")
            .arg(dmlNamespaces(), id, ln.arg(12700).arg(QStringLiteral("FF0000")), ln.arg(6350).arg(QStringLiteral("00FF00")), ln.arg(6350).arg(QStringLiteral("00AA00")),
                 dmlSolid(QStringLiteral("EEEEEE")), dmlSolid(QStringLiteral("DDDDDD")), ln.arg(38100).arg(QStringLiteral("FFFF00")), dmlSolid(QStringLiteral("444444")),
                 ln.arg(38100).arg(QStringLiteral("0000FF")), dmlSolid(QStringLiteral("333333")));
        DmlEnv env;
        dmlUseTableStyles(env, styles.toUtf8());
        // Three columns, four rows; the cell at row 2, column 2 has a yellow fill and no bottom border of its own.
        QString rows;
        for (int r = 0; r < 4; ++r) {
            rows += QStringLiteral("<a:tr h=\"%1\">").arg(emu(20));
            for (int c = 0; c < 3; ++c)
                rows += dmlTc(QStringLiteral("r%1c%2").arg(r).arg(c), QString(), r == 2 && c == 2 ? dmlSolid(QStringLiteral("FFFF00")) + QStringLiteral("<a:lnB><a:noFill/></a:lnB>") : QString());
            rows += QStringLiteral("</a:tr>");
        }
        const QString grid = QStringLiteral("<a:tblGrid><a:gridCol w=\"%1\"/><a:gridCol w=\"%1\"/><a:gridCol w=\"%1\"/></a:tblGrid>").arg(emu(80));
        const QString tblPr = QStringLiteral("<a:tblPr firstRow=\"1\" bandRow=\"1\" firstCol=\"1\" lastRow=\"1\"><a:tableStyleId>%1</a:tableStyleId></a:tblPr>").arg(id);
        auto t = dmlAs<TableItem>(readTable(env.element(dmlTableFrame(tblPr, grid, rows, 0, 0, 240, 80)), env.ctx));
        QVERIFY(t);
        QCOMPARE(t->format, id);
        QVERIFY(t->header && t->banded);

        const auto fill = [&](int r, int c) { return dmlHex(t->cell(r, c).fill.color); };
        QCOMPARE(fill(0, 1), QStringLiteral("333333"));       // the header row
        QCOMPARE(fill(1, 1), QStringLiteral("DDDDDD"));       // the first band
        QCOMPARE(fill(2, 1), QStringLiteral("EEEEEE"));       // the second has the table's
        QCOMPARE(fill(3, 1), QStringLiteral("444444"));       // the last row
        QCOMPARE(fill(1, 0), QStringLiteral("DDDDDD"));       // the first column says nothing of fill
        QCOMPARE(fill(2, 2), QStringLiteral("FFFF00"));       // the cell's own

        const auto border = [&](const jp::Stroke &s) { return s.isNone() ? QStringLiteral("none") : QStringLiteral("%1 %2").arg(s.width).arg(dmlHex(s.color)); };
        // The outside of the table, the edges between cells, and the edge a region of the style has (on both sides of it).
        QCOMPARE(border(t->cell(0, 0).border.left), QStringLiteral("1 FF0000"));
        QCOMPARE(border(t->cell(2, 2).border.right), QStringLiteral("1 FF0000"));
        QCOMPARE(border(t->cell(0, 1).border.top), QStringLiteral("1 FF0000"));
        QCOMPARE(border(t->cell(3, 1).border.bottom), QStringLiteral("1 FF0000"));
        QCOMPARE(border(t->cell(2, 0).border.right), QStringLiteral("0.5 00AA00"));
        QCOMPARE(border(t->cell(2, 1).border.left), QStringLiteral("0.5 00AA00"));
        QCOMPARE(border(t->cell(1, 1).border.bottom), QStringLiteral("0.5 00FF00"));
        QCOMPARE(border(t->cell(2, 1).border.top), QStringLiteral("0.5 00FF00"));
        QCOMPARE(border(t->cell(0, 1).border.bottom), QStringLiteral("3 0000FF"));
        QCOMPARE(border(t->cell(1, 1).border.top), QStringLiteral("3 0000FF"));
        QCOMPARE(border(t->cell(3, 1).border.top), QStringLiteral("3 FFFF00"));
        QCOMPARE(border(t->cell(2, 1).border.bottom), QStringLiteral("3 FFFF00"));
        // A border a cell turns off is off on both sides.
        QCOMPARE(border(t->cell(2, 2).border.bottom), QStringLiteral("none"));
        QCOMPARE(border(t->cell(3, 2).border.top), QStringLiteral("none"));

        // The text: the style's color and weight, in the stories.
        const auto format = [&](int r, int c) { return env.ctx.storyDoc(t->cell(r, c).storyId)->begin().begin().fragment().charFormat(); };
        QCOMPARE(hexOf(format(0, 1).foreground().color()), QStringLiteral("FFFFFF"));
        QCOMPARE(format(0, 1).fontWeight(), int(QFont::Bold));
        QCOMPARE(hexOf(format(2, 1).foreground().color()), QStringLiteral("202020"));
        QVERIFY(format(2, 1).fontWeight() < int(QFont::Bold) && !format(2, 1).fontItalic());
        QCOMPARE(hexOf(format(3, 1).foreground().color()), QStringLiteral("FFFF00"));
        QCOMPARE(format(3, 1).fontWeight(), int(QFont::Bold));
        QVERIFY(format(1, 0).fontWeight() == int(QFont::Bold) && format(1, 0).fontItalic());
        QCOMPARE(hexOf(format(1, 0).foreground().color()), QStringLiteral("FFFFFF"));
        QVERIFY(format(1, 1).fontWeight() < int(QFont::Bold));

        // Written, the table has all it got from its style in the cells themselves: read where the style isn't
        // known, the cells are the same.
        DmlEnv back;
        auto u = dmlAs<TableItem>(readTable(back.element(dmlWriteTable(*t, env, 2)), back.ctx));
        QVERIFY(u);
        QCOMPARE(u->format, id);
        for (int r = 0; r < t->rows; ++r)
            for (int c = 0; c < t->cols; ++c) {
                const QByteArray at = QStringLiteral("row %1, column %2").arg(r).arg(c).toLatin1();
                QVERIFY2(u->cell(r, c).fill == t->cell(r, c).fill, at);
                const jp::CellBorder &a = t->cell(r, c).border, &b = u->cell(r, c).border;
                QVERIFY2(a.top == b.top && a.bottom == b.bottom && a.left == b.left && a.right == b.right, at);
            }
    }

    // A style the file doesn't define is approximated by the usual one (an accent header, banded rows in tints, white
    // borders); the two plain styles are what they say.
    void dmlTableApproximatesStylesItDoesNotHave()
    {
        const auto table = [&](DmlEnv &env, const QString &style) {
            const QString rows = QStringLiteral("<a:tr h=\"%1\">%2%2</a:tr><a:tr h=\"%1\">%2%2</a:tr><a:tr h=\"%1\">%2%2</a:tr>").arg(emu(20), dmlTc(QStringLiteral("x")));
            const QString grid = QStringLiteral("<a:tblGrid><a:gridCol w=\"%1\"/><a:gridCol w=\"%1\"/></a:tblGrid>").arg(emu(80));
            const QString tblPr = QStringLiteral("<a:tblPr firstRow=\"1\" bandRow=\"1\">%1</a:tblPr>").arg(style.isEmpty() ? QString() : QStringLiteral("<a:tableStyleId>%1</a:tableStyleId>").arg(style));
            return dmlAs<TableItem>(readTable(env.element(dmlTableFrame(tblPr, grid, rows, 0, 0, 160, 60)), env.ctx));
        };
        const QString medium = QStringLiteral("{5C22544A-7EE6-4342-B048-85BDC9FD1C3A}");
        // With no table styles part at all, and with one that has other styles, for the usual style's id and another's.
        for (int variant = 0; variant < 4; ++variant) {
            const QString id = variant % 2 ? QStringLiteral("{ABCDEF01-2345-6789-ABCD-EF0123456789}") : medium;
            DmlEnv env;
            if (variant >= 2)
                dmlUseTableStyles(env, QStringLiteral("<a:tblStyleLst %1 def=\"{22222222-2222-2222-2222-222222222222}\"><a:tblStyle styleId=\"{22222222-2222-2222-2222-222222222222}\" styleName=\"Other\">"
                                                      "<a:wholeTbl><a:tcStyle><a:tcBdr/></a:tcStyle></a:wholeTbl></a:tblStyle></a:tblStyleLst>").arg(dmlNamespaces()).toUtf8());
            auto t = table(env, id);
            QVERIFY(t);
            QCOMPARE(t->format, id);
            QCOMPARE(dmlHex(t->cell(0, 0).fill.color), QStringLiteral("4472C4"));      // accent 1
            QVERIFY(t->cell(1, 0).fill.type == Fill::Solid && t->cell(2, 0).fill.type == Fill::Solid);
            QVERIFY(t->cell(1, 0).fill.color != t->cell(2, 0).fill.color);             // banded
            QVERIFY(t->cell(1, 0).fill.color.rgbValue().lightness() > QColor(0x44, 0x72, 0xC4).lightness());
            QCOMPARE(dmlHex(t->cell(1, 0).border.left.color), QStringLiteral("FEFEFE"));   // the theme's light color
            QVERIFY(!t->cell(1, 0).border.bottom.isNone() && !t->cell(1, 1).border.left.isNone());
            const auto format = env.ctx.storyDoc(t->cell(0, 0).storyId)->begin().begin().fragment().charFormat();
            QCOMPARE(format.fontWeight(), int(QFont::Bold));
            QCOMPARE(hexOf(format.foreground().color()), QStringLiteral("FEFEFE"));
            QVERIFY(env.ctx.storyDoc(t->cell(1, 0).storyId)->begin().begin().fragment().charFormat().fontWeight() < int(QFont::Bold));
        }
        // "No Style, No Grid" draws nothing; "No Style, Table Grid" draws a thin line around every cell.
        DmlEnv env;
        auto plain = table(env, QStringLiteral("{2D5ABB26-0587-4C30-8999-92F81FD0307C}"));
        QVERIFY(plain);
        for (const auto &c : plain->cells) {
            QVERIFY(c.fill.isNone());
            QVERIFY(c.border.top.isNone() && c.border.left.isNone());
        }
        auto grid = table(env, QStringLiteral("{5940675A-B579-460E-94D1-54222C63F5DA}"));
        QVERIFY(grid);
        for (const auto &c : grid->cells) {
            QVERIFY(c.fill.isNone());
            QVERIFY(!c.border.top.isNone() && !c.border.bottom.isNone() && !c.border.left.isNone() && !c.border.right.isNone());
            QVERIFY(near(c.border.top.width, 1));
        }
        // No style at all: nothing from one.
        auto none = table(env, QString());
        QVERIFY(none);
        for (const auto &c : none->cells) QVERIFY(c.fill.isNone() && c.border.top.isNone());
    }

    // What isn't a table, or is too much of one, is not read; spans that go past the table stop at its edge.
    void dmlTableLeavesWhatItCannotRead()
    {
        DmlEnv env;
        const QString grid = QStringLiteral("<a:tblGrid><a:gridCol w=\"%1\"/><a:gridCol w=\"%1\"/></a:tblGrid>").arg(emu(50));
        const QString row = QStringLiteral("<a:tr h=\"%1\">%2%2</a:tr>").arg(emu(20), dmlTc(QStringLiteral("x")));
        QVERIFY(!readTable(QDomElement(), env.ctx));
        QVERIFY(!isTableFrame(QDomElement()));
        // A frame of something else, a table with no columns, one with no rows.
        const QString chart = QStringLiteral("<p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id=\"2\" name=\"Chart\"/><p:cNvGraphicFramePr/><p:nvPr/></p:nvGraphicFramePr>"
                                             "<p:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"100\" cy=\"100\"/></p:xfrm><a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/chart\"/></a:graphic></p:graphicFrame>");
        QVERIFY(!isTableFrame(env.element(chart)));
        QVERIFY(!readTable(env.element(chart), env.ctx));
        QVERIFY(isTableFrame(env.element(dmlTableFrame(QString(), grid, row, 0, 0, 100, 20))));
        // The strict form of the standard names the table by another URI.
        QString strict = dmlTableFrame(QString(), grid, row, 0, 0, 100, 20);
        strict.replace(QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/table"), QStringLiteral("http://purl.oclc.org/ooxml/drawingml/table"));
        QVERIFY(readTable(env.element(strict), env.ctx));
        QVERIFY(!readTable(env.element(dmlTableFrame(QString(), QStringLiteral("<a:tblGrid/>"), row, 0, 0, 100, 20)), env.ctx));
        QVERIFY(!readTable(env.element(dmlTableFrame(QString(), grid, QString(), 0, 0, 100, 20)), env.ctx));
        // More columns than the table keeps.
        QString wide = QStringLiteral("<a:tblGrid>");
        for (int i = 0; i < 300; ++i) wide += QStringLiteral("<a:gridCol w=\"12700\"/>");
        QVERIFY(!readTable(env.element(dmlTableFrame(QString(), wide + QStringLiteral("</a:tblGrid>"), row, 0, 0, 100, 20)), env.ctx));
        // Spans past the edge, a span that covers another span's cell, and a row with too few or too many cells.
        const QString rows = QStringLiteral("<a:tr h=\"254000\">") + dmlTc(QStringLiteral("a"), QStringLiteral("gridSpan=\"99\" rowSpan=\"99\"")) + dmlTc(QStringLiteral("b")) + QStringLiteral("</a:tr>")
            + QStringLiteral("<a:tr h=\"254000\">") + dmlTc(QStringLiteral("c"), QStringLiteral("gridSpan=\"2\"")) + dmlTc(QStringLiteral("d"), QStringLiteral("rowSpan=\"2\" gridSpan=\"2\"")) + dmlTc(QStringLiteral("e")) + QStringLiteral("</a:tr>")
            + QStringLiteral("<a:tr h=\"254000\">") + dmlTc(QStringLiteral("f")) + QStringLiteral("</a:tr>");
        auto t = dmlAs<TableItem>(readTable(env.element(dmlTableFrame(QString(), grid, rows, 0, 0, 100, 60)), env.ctx));
        QVERIFY(t);
        QCOMPARE(t->rows, 3);
        QCOMPARE(t->cols, 2);
        QCOMPARE(t->cell(0, 0).colSpan, 2);
        QCOMPARE(t->cell(0, 0).rowSpan, 3);
        QVERIFY(t->cell(0, 1).covered && t->cell(1, 0).covered && t->cell(1, 1).covered && t->cell(2, 0).covered && t->cell(2, 1).covered);
        QCOMPARE(t->cell(1, 0).colSpan, 1);   // covered, so what it said of itself is gone
        QCOMPARE(int(t->cells.size()), 6);
        for (const auto &c : t->cells) QVERIFY(!c.storyId.isEmpty());
        // A row that has too few cells is filled out with empty ones.
        auto few = dmlAs<TableItem>(readTable(env.element(dmlTableFrame(QString(), grid, QStringLiteral("<a:tr h=\"254000\">") + dmlTc(QStringLiteral("a")) + QStringLiteral("</a:tr>"), 0, 0, 100, 20)), env.ctx));
        QVERIFY(few);
        QCOMPARE(few->cols, 2);
        QVERIFY(!few->cell(0, 1).covered && few->cell(0, 1).rowSpan == 1);
        QVERIFY(env.ctx.storyDoc(few->cell(0, 1).storyId)->toPlainText().isEmpty());
    }

    // Written and read again, a table is the table it was: its grid, merges, fills, borders, margins, anchors, and
    // text, and the style it names; and it is a frame with a table in it, as the presentation program writes one.
    void dmlTableWritesWhatItRead()
    {
        const QString ln = QStringLiteral("<a:solidFill><a:srgbClr val=\"%1\"/></a:solidFill>");
        const QString rows =
            QStringLiteral("<a:tr h=\"%1\">").arg(emu(30))
            + dmlTc(QStringLiteral("Total"), QStringLiteral("gridSpan=\"2\""), QStringLiteral("<a:lnT><a:noFill/></a:lnT><a:lnB w=\"25400\">%1</a:lnB>").arg(ln.arg(QStringLiteral("0000FF"))) + dmlSolid(QStringLiteral("FF0000")),
                    QStringLiteral("marL=\"127000\" marR=\"63500\" marT=\"38100\" marB=\"25400\" anchor=\"ctr\""), QStringLiteral("b=\"1\""))
            + dmlTc(QString(), QStringLiteral("hMerge=\"1\"")) + dmlTc(QStringLiteral("C0")) + QStringLiteral("</a:tr>")
            + QStringLiteral("<a:tr h=\"%1\">").arg(emu(25))
            + dmlTc(QStringLiteral("tall"), QStringLiteral("rowSpan=\"2\""), QString(), QStringLiteral("anchor=\"b\" vert=\"vert\""))
            + dmlTc(QStringLiteral("E1"), QString(), QStringLiteral("<a:noFill/>"))
            + dmlTc(QStringLiteral("F1"), QString(), QStringLiteral("<a:lnR w=\"12700\" cmpd=\"dbl\">%1<a:prstDash val=\"dash\"/></a:lnR><a:lnBlToTr w=\"12700\">%2</a:lnBlToTr>").arg(ln.arg(QStringLiteral("00FF00")), ln.arg(QStringLiteral("808080"))))
            + QStringLiteral("</a:tr>")
            + QStringLiteral("<a:tr h=\"%1\">").arg(emu(20))
            + dmlTc(QString(), QStringLiteral("vMerge=\"1\"")) + dmlTc(QStringLiteral("H2")) + dmlTc(QStringLiteral("I2"), QString(), dmlSolid(QStringLiteral("00FF00")))
            + QStringLiteral("</a:tr>");
        const QString grid = QStringLiteral("<a:tblGrid><a:gridCol w=\"%1\"/><a:gridCol w=\"%2\"/><a:gridCol w=\"%3\"/></a:tblGrid>").arg(emu(100), emu(150), emu(50));
        const QString tblPr = QStringLiteral("<a:tblPr firstRow=\"1\" bandRow=\"1\"><a:tableStyleId>{5C22544A-7EE6-4342-B048-85BDC9FD1C3A}</a:tableStyleId></a:tblPr>");
        DmlEnv env;
        auto t = dmlAs<TableItem>(readTable(env.element(dmlTableFrame(tblPr, grid, rows, 36, 72, 300, 75)), env.ctx));
        QVERIFY(t);
        t->role = QStringLiteral("ph:tbl:3");

        int next = 0;
        const QString xml = dmlWriteTable(*t, env, 7, &next);
        QCOMPARE(next, 8);
        QVERIFY2(xml.startsWith(QStringLiteral("<p:graphicFrame>")), qPrintable(xml));
        QVERIFY(xml.contains(QStringLiteral("<p:cNvPr id=\"7\"")));
        QVERIFY(xml.contains(QStringLiteral("<a:graphicFrameLocks noGrp=\"1\"/>")));
        QVERIFY(xml.contains(QStringLiteral("<p:ph type=\"tbl\" idx=\"3\"/>")));
        QVERIFY(xml.contains(QStringLiteral("<a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/table\">")));
        QVERIFY(xml.contains(QStringLiteral("<a:tblPr firstRow=\"1\" bandRow=\"1\">")));
        QVERIFY(xml.contains(QStringLiteral("<a:tableStyleId>{5C22544A-7EE6-4342-B048-85BDC9FD1C3A}</a:tableStyleId>")));
        QVERIFY(xml.contains(QStringLiteral("<a:gridCol w=\"1270000\"/><a:gridCol w=\"1905000\"/><a:gridCol w=\"635000\"/>")));
        QVERIFY(xml.contains(QStringLiteral("<a:tr h=\"381000\">")) && xml.contains(QStringLiteral("<a:tr h=\"317500\">")));
        QVERIFY(xml.contains(QStringLiteral("gridSpan=\"2\"")) && xml.contains(QStringLiteral("hMerge=\"1\"")));
        QVERIFY(xml.contains(QStringLiteral("rowSpan=\"2\"")) && xml.contains(QStringLiteral("vMerge=\"1\"")));
        QVERIFY(xml.contains(QStringLiteral("<a:off x=\"457200\" y=\"914400\"/><a:ext cx=\"3810000\" cy=\"952500\"/>")));

        DmlEnv back;
        auto u = dmlAs<TableItem>(readTable(back.element(xml), back.ctx));
        QVERIFY2(u, qPrintable(xml));
        QCOMPARE(u->rows, t->rows);
        QCOMPARE(u->cols, t->cols);
        QCOMPARE(u->colW, t->colW);
        QCOMPARE(u->rowH, t->rowH);
        QCOMPARE(u->rect, t->rect);
        QCOMPARE(u->name, t->name);
        QCOMPARE(u->altText, t->altText);
        QCOMPARE(u->format, t->format);
        QCOMPARE(u->header, t->header);
        QCOMPARE(u->banded, t->banded);
        for (int r = 0; r < t->rows; ++r)
            for (int c = 0; c < t->cols; ++c) {
                const QByteArray at = QStringLiteral("row %1, column %2").arg(r).arg(c).toLatin1();
                const jp::TableCell &a = t->cell(r, c), &b = u->cell(r, c);
                QVERIFY2(a.rowSpan == b.rowSpan && a.colSpan == b.colSpan && a.covered == b.covered, at);
                QVERIFY2(a.fill == b.fill, at);
                QVERIFY2(a.border.top == b.border.top && a.border.bottom == b.border.bottom && a.border.left == b.border.left && a.border.right == b.border.right, at);
                QVERIFY2(a.diagonal == b.diagonal && a.diagonalLine == b.diagonalLine, at);
                QVERIFY2(a.margins == b.margins && a.valign == b.valign && a.vertical == b.vertical, at);
            }
        // The style's look came back as the cells' own, so none of it is read from the style twice.
        QCOMPARE(dmlHex(u->cell(0, 2).fill.color), dmlHex(t->cell(0, 2).fill.color));
        QVERIFY(u->cell(1, 1).fill.isNone());
        // The text, once the drawing writer writes text.
        const auto texts = [](DmlEnv &e, const TableItem &item) {
            QStringList out;
            for (const auto &c : item.cells) out << e.ctx.storyDoc(c.storyId)->toPlainText();
            return out;
        };
        if (!dmlWriterWritesText()) QEXPECT_FAIL("", "drawing writer pending", Continue);
        QCOMPARE(texts(back, *u), texts(env, *t));
        if (!dmlWriterWritesText()) QEXPECT_FAIL("", "drawing writer pending", Continue);
        QCOMPARE(back.ctx.storyDoc(u->cell(0, 0).storyId)->begin().begin().fragment().charFormat().fontWeight(), int(QFont::Bold));

        // A table with nothing in it is nothing to write.
        TableItem empty;
        int used = 0;
        QVERIFY(dmlWriteTable(empty, env, 11, &used).isEmpty());
        QCOMPARE(used, 11);

        // A table made here, with a style of ours by name, names none; its role is kept only as a placeholder's.
        auto own = std::make_shared<TableItem>();
        own->rows = own->cols = 1;
        own->colW = {72};
        own->rowH = {36};
        own->cells.resize(1);
        own->format = QStringLiteral("Basic 1");
        own->header = false;
        own->banded = false;
        own->rect = QRectF(10, 20, 72, 36);
        const QString plain = dmlWriteTable(*own, env, 2);
        QVERIFY2(!plain.contains(QStringLiteral("tableStyleId")), qPrintable(plain));
        QVERIFY(!plain.contains(QStringLiteral("<p:ph")));
        QVERIFY(plain.contains(QStringLiteral("<a:tblPr/>")));
        QVERIFY(plain.contains(QStringLiteral("name=\"Table ")));
        DmlEnv again;
        auto v = dmlAs<TableItem>(readTable(again.element(plain), again.ctx));
        QVERIFY(v);
        QVERIFY(v->format.isEmpty() && !v->header && !v->banded);
        QVERIFY(v->cell(0, 0).fill.isNone() && v->cell(0, 0).border.top.isNone());
    }

private:
    // A cell: its text (none if empty), attributes, the children and attributes of its a:tcPr, and its run's attributes.
    static QString dmlTc(const QString &text, const QString &attrs = QString(), const QString &pr = QString(), const QString &prAttrs = QString(),
                         const QString &rPr = QString())
    {
        const QString run = text.isEmpty() ? QString() : QStringLiteral("<a:r><a:rPr lang=\"en-US\" %2/><a:t>%1</a:t></a:r>").arg(text, rPr);
        return QStringLiteral("<a:tc %1><a:txBody><a:bodyPr/><a:lstStyle/><a:p>%2</a:p></a:txBody><a:tcPr %3>%4</a:tcPr></a:tc>").arg(attrs, run, prAttrs, pr);
    }

    // A graphic frame holding a table, at the place given in points.
    static QString dmlTableFrame(const QString &tblPr, const QString &grid, const QString &rows, double x, double y, double w, double h)
    {
        return QStringLiteral("<p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id=\"4\" name=\"Table 3\" descr=\"Figures\"/><p:cNvGraphicFramePr><a:graphicFrameLocks noGrp=\"1\"/></p:cNvGraphicFramePr>"
                              "<p:nvPr/></p:nvGraphicFramePr>%1<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/table\"><a:tbl>%2%3%4</a:tbl></a:graphicData></a:graphic></p:graphicFrame>")
            .arg(dmlXfrm(x, y, w, h, QString(), "p:xfrm"), tblPr, grid, rows);
    }

    // The presentation's table styles part, linked from its main part, as `env`'s package.
    static void dmlUseTableStyles(DmlEnv &env, const QByteArray &xml)
    {
        PackageWriter w;
        w.addPart(QStringLiteral("/ppt/presentation.xml"), "<p:presentation/>", kPresentationType);
        w.addPart(QStringLiteral("/ppt/tableStyles.xml"), xml, QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.tableStyles+xml"));
        w.addRelationship(QStringLiteral("/"), kRelBase + "officeDocument", QStringLiteral("/ppt/presentation.xml"));
        w.addRelationship(QStringLiteral("/ppt/presentation.xml"), kRelBase + "tableStyles", QStringLiteral("/ppt/tableStyles.xml"));
        QVERIFY(env.package.read(w.finish()));
        env.ctx.package = &env.package;
        env.ctx.part = QStringLiteral("/ppt/slides/slide1.xml");
    }

    // The table as the writer writes it, with its stories from `from`.
    static QString dmlWriteTable(const TableItem &item, DmlEnv &from, int firstId, int *nextId = nullptr)
    {
        DrawingWriteContext wctx;
        wctx.nextShapeId = firstId;
        wctx.storyDoc = [&from](const QString &id) -> const QTextDocument * { return from.ctx.storyDoc(id); };
        QByteArray xml;
        QXmlStreamWriter w(&xml);
        writeTable(w, item, wctx);
        if (nextId) *nextId = wctx.nextShapeId;
        return QString::fromUtf8(xml);
    }

    // Whether the drawing writer writes the text of a story yet.
    static bool dmlWriterWritesText()
    {
        QTextDocument doc;
        doc.setPlainText(QStringLiteral("x"));
        QByteArray xml;
        QXmlStreamWriter w(&xml);
        DrawingWriteContext ctx;
        writeParagraphs(w, &doc, ctx);
        return xml.contains("<a:t>");
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
        // An arc is its curve, and with a fill its sector too, drawn as shapes whose names don't say "no fill".
        auto arcItem = dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 50) + QStringLiteral("<a:prstGeom prst=\"arc\"/>") + dmlSolid(QStringLiteral("FF0000")) +
                                         QStringLiteral("<a:ln w=\"12700\">%1</a:ln>").arg(dmlSolid(QStringLiteral("000000")))));
        auto arcGroup = dmlAs<GroupItem>(arcItem);
        QVERIFY(arcGroup && arcGroup->children.size() == 2);
        QVERIFY(dmlAs<ShapeItem>(arcGroup->children[0])->shape != QLatin1String("arc") && !dmlAs<ShapeItem>(arcGroup->children[0])->fill.isNone());
        QVERIFY(dmlAs<ShapeItem>(arcGroup->children[1])->fill.isNone() && !dmlAs<ShapeItem>(arcGroup->children[1])->stroke.isNone());
        auto arcOnly = dmlAs<ShapeItem>(dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 50) + QStringLiteral("<a:prstGeom prst=\"arc\"/><a:noFill/><a:ln w=\"12700\">%1</a:ln>").arg(dmlSolid(QStringLiteral("000000"))))));
        QVERIFY(arcOnly && arcOnly->fill.isNone() && !arcOnly->customPath.isEmpty());
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

    void dmlTextTakesTheStylesFontReference()
    {
        DmlEnv env;
        const QString a(ns::A);
        const QDomElement base = env.element(QStringLiteral("<a:lstStyle><a:lvl1pPr><a:defRPr sz=\"1800\"/></a:lvl1pPr></a:lstStyle>"));
        const QDomElement other = env.element(QStringLiteral("<a:lstStyle><a:lvl1pPr algn=\"ctr\"/></a:lstStyle>"));
        const QDomElement style = env.element(dmlStyle(QStringLiteral("1"), QStringLiteral("1")));
        QDomDocument keep;
        // As one more list style, above the others: the font reference's color (light 1 here) at every level.
        TextStyleStack stack = withFontReference({base}, style, QDomElement(), &keep);
        QCOMPARE(int(stack.size()), 2);
        QVERIFY(stack[0] == base);
        for (int level = 1; level <= 9; ++level) {
            const QDomElement run = child(child(stack[1], a, QStringLiteral("lvl%1pPr").arg(level)), a, QStringLiteral("defRPr"));
            QVERIFY(!run.isNull());
            QCOMPARE(attr(child(child(run, a, QStringLiteral("solidFill")), a, QStringLiteral("schemeClr")), QStringLiteral("val")), QStringLiteral("lt1"));
        }
        // Below the shape's own list, when the stack ends in it.
        stack = withFontReference({other, base}, style, base, &keep);
        QCOMPARE(int(stack.size()), 3);
        QVERIFY(stack[0] == other && stack[2] == base);
        QVERIFY(!child(stack[1], a, QStringLiteral("lvl3pPr")).isNull());
        stack = withFontReference({base, other}, style, base, &keep);
        QVERIFY(stack.last() != other && stack.size() == 3);
        // The heading font, with or without a color; no reference, nothing to say, no change.
        const QDomElement major = env.element(QStringLiteral("<p:style><a:fontRef idx=\"major\"/></p:style>"));
        stack = withFontReference({base}, major, QDomElement(), &keep);
        QCOMPARE(int(stack.size()), 2);
        const QDomElement run = child(child(stack[1], a, QStringLiteral("lvl1pPr")), a, QStringLiteral("defRPr"));
        QCOMPARE(attr(child(run, a, QStringLiteral("latin")), QStringLiteral("typeface")), QStringLiteral("+mj-lt"));
        QVERIFY(child(run, a, QStringLiteral("solidFill")).isNull());
        QCOMPARE(int(withFontReference({base}, env.element(QStringLiteral("<p:style><a:fontRef idx=\"minor\"/></p:style>")), QDomElement(), &keep).size()), 1);
        QCOMPARE(int(withFontReference({base}, env.element(QStringLiteral("<p:style/>")), QDomElement(), &keep).size()), 1);
        QCOMPARE(int(withFontReference({base}, QDomElement(), QDomElement(), &keep).size()), 1);
        QCOMPARE(int(withFontReference({}, style, QDomElement(), &keep).size()), 1);
        // A shape that has the style reads its text with it.
        const QString rect = dmlXfrm(0, 0, 100, 40) + QStringLiteral("<a:prstGeom prst=\"rect\"/>");
        env.useFormats();
        const ItemPtr it = dmlOne(env, dmlSp(rect, dmlBody(), dmlStyle(QStringLiteral("1"), QStringLiteral("1"))));
        QVERIFY(dmlAs<TextItem>(it));
        QVERIFY(env.stories[0]->toPlainText().contains(QStringLiteral("Hello")));
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
        // A connector that says nothing of its shape is a line from corner to corner.
        it = dmlOne(env, dmlCxn(dmlXfrm(10, 20, 100, 50)));
        l = dmlAs<LineItem>(it);
        QVERIFY(l && near(l->p1.x(), 10) && near(l->p1.y(), 20) && near(l->p2.x(), 110) && near(l->p2.y(), 70));
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

    void dmlReadsATreeAsTheProgramsWriteIt()
    {
        DmlEnv env;
        env.useFormats();
        env.usePackage();
        // A slide's shape tree, written with the extra elements and attributes real files carry.
        const QString tree = QStringLiteral(R"(<p:spTree>
  <p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>
  <p:grpSpPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="0" cy="0"/><a:chOff x="0" y="0"/><a:chExt cx="0" cy="0"/></a:xfrm></p:grpSpPr>
  <p:sp>
    <p:nvSpPr><p:cNvPr id="2" name="Title 1"><a:extLst><a:ext uri="{FF2B5EF4-FFF2-40B4-BE49-F238E27FC236}"/></a:extLst></p:cNvPr>
      <p:cNvSpPr><a:spLocks noGrp="1"/></p:cNvSpPr><p:nvPr><p:ph type="title"/></p:nvPr></p:nvSpPr>
    <p:spPr/>
    <p:txBody><a:bodyPr/><a:lstStyle/><a:p><a:r><a:rPr lang="en-US" dirty="0"/><a:t>A title</a:t></a:r><a:endParaRPr lang="en-US" dirty="0"/></a:p></p:txBody>
  </p:sp>
  <p:sp>
    <p:nvSpPr><p:cNvPr id="3" name="TextBox 2"/><p:cNvSpPr txBox="1"/><p:nvPr/></p:nvSpPr>
    <p:spPr><a:xfrm><a:off x="1143000" y="1524000"/><a:ext cx="2286000" cy="369332"/></a:xfrm><a:prstGeom prst="rect"><a:avLst/></a:prstGeom><a:noFill/></p:spPr>
    <p:txBody><a:bodyPr wrap="none" rtlCol="0"><a:spAutoFit/></a:bodyPr><a:lstStyle/><a:p><a:r><a:rPr lang="en-US" dirty="0"/><a:t>Some text</a:t></a:r></a:p></p:txBody>
  </p:sp>
  <p:sp>
    <p:nvSpPr><p:cNvPr id="4" name="Rounded Rectangle 3"/><p:cNvSpPr/><p:nvPr/></p:nvSpPr>
    <p:spPr><a:xfrm><a:off x="1143000" y="2286000"/><a:ext cx="2743200" cy="914400"/></a:xfrm><a:prstGeom prst="roundRect"><a:avLst/></a:prstGeom></p:spPr>
    <p:style><a:lnRef idx="2"><a:schemeClr val="accent1"><a:shade val="50000"/></a:schemeClr></a:lnRef><a:fillRef idx="1"><a:schemeClr val="accent1"/></a:fillRef>
      <a:effectRef idx="0"><a:schemeClr val="accent1"/></a:effectRef><a:fontRef idx="minor"><a:schemeClr val="lt1"/></a:fontRef></p:style>
    <p:txBody><a:bodyPr rtlCol="0" anchor="ctr"/><a:lstStyle/><a:p><a:pPr algn="ctr"/><a:r><a:rPr lang="en-US" dirty="0"/><a:t>Shape</a:t></a:r></a:p></p:txBody>
  </p:sp>
  <p:pic>
    <p:nvPicPr><p:cNvPr id="5" name="Picture 4" descr="A red square"/><p:cNvPicPr><a:picLocks noChangeAspect="1"/></p:cNvPicPr><p:nvPr/></p:nvPicPr>
    <p:blipFill><a:blip r:embed="rId1"><a:extLst><a:ext uri="{28A0092B-C50C-407E-A947-70E740481C1C}"/></a:extLst></a:blip><a:stretch><a:fillRect/></a:stretch></p:blipFill>
    <p:spPr><a:xfrm><a:off x="4572000" y="1524000"/><a:ext cx="1828800" cy="1371600"/></a:xfrm><a:prstGeom prst="rect"><a:avLst/></a:prstGeom></p:spPr>
  </p:pic>
  <p:grpSp>
    <p:nvGrpSpPr><p:cNvPr id="10" name="Group 9"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>
    <p:grpSpPr><a:xfrm><a:off x="6400800" y="1524000"/><a:ext cx="2286000" cy="1143000"/><a:chOff x="6400800" y="1524000"/><a:chExt cx="2286000" cy="1143000"/></a:xfrm></p:grpSpPr>
    <p:sp><p:nvSpPr><p:cNvPr id="11" name="Oval 10"/><p:cNvSpPr/><p:nvPr/></p:nvSpPr><p:spPr><a:xfrm><a:off x="6400800" y="1524000"/><a:ext cx="914400" cy="1143000"/></a:xfrm><a:prstGeom prst="ellipse"><a:avLst/></a:prstGeom></p:spPr>
      <p:style><a:lnRef idx="2"><a:schemeClr val="accent1"/></a:lnRef><a:fillRef idx="1"><a:schemeClr val="accent2"/></a:fillRef><a:effectRef idx="0"><a:schemeClr val="accent1"/></a:effectRef><a:fontRef idx="minor"><a:schemeClr val="lt1"/></a:fontRef></p:style></p:sp>
    <p:sp><p:nvSpPr><p:cNvPr id="12" name="Right Arrow 11"/><p:cNvSpPr/><p:nvPr/></p:nvSpPr><p:spPr><a:xfrm><a:off x="7543800" y="1752600"/><a:ext cx="1143000" cy="685800"/></a:xfrm><a:prstGeom prst="rightArrow"><a:avLst><a:gd name="adj1" fmla="val 50000"/><a:gd name="adj2" fmla="val 50000"/></a:avLst></a:prstGeom></p:spPr></p:sp>
  </p:grpSp>
  <p:cxnSp>
    <p:nvCxnSpPr><p:cNvPr id="13" name="Elbow Connector 12"/><p:cNvCxnSpPr><a:stCxn id="4" idx="3"/><a:endCxn id="5" idx="1"/></p:cNvCxnSpPr><p:nvPr/></p:nvCxnSpPr>
    <p:spPr><a:xfrm><a:off x="3886200" y="2209800"/><a:ext cx="685800" cy="762000"/></a:xfrm><a:prstGeom prst="bentConnector3"><a:avLst><a:gd name="adj1" fmla="val 50000"/></a:avLst></a:prstGeom><a:ln w="19050"><a:tailEnd type="triangle"/></a:ln></p:spPr>
    <p:style><a:lnRef idx="1"><a:schemeClr val="accent1"/></a:lnRef><a:fillRef idx="0"><a:schemeClr val="accent1"/></a:fillRef><a:effectRef idx="0"><a:schemeClr val="accent1"/></a:effectRef><a:fontRef idx="minor"><a:schemeClr val="tx1"/></a:fontRef></p:style>
  </p:cxnSp>
  <p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id="14" name="Table 13"/><p:cNvGraphicFramePr/><p:nvPr/></p:nvGraphicFramePr><p:xfrm><a:off x="0" y="0"/><a:ext cx="1" cy="1"/></p:xfrm></p:graphicFrame>
</p:spTree>)");
        const QString layoutTitle = dmlSp(dmlXfrm(100, 30, 760, 80), QString(), QString(), QString(), QStringLiteral("<p:ph type=\"title\"/>"));
        QDomElement spTree = env.element(tree);
        QVERIFY(!spTree.isNull());
        // The tree reads in the order it is written, the table left for the caller to keep.
        const ItemList list = readShapeTree(spTree, env.ctx);
        QCOMPARE(int(list.size()), 6);
        QVERIFY(dmlAs<TextItem>(list[0]));            // the title, with no place of its own (a placeholder read through a tree)
        auto box = dmlAs<TextItem>(list[1]);
        QVERIFY(box);
        QVERIFY(dmlRectIs(box->rect, 90, 120, 180, 369332.0 / 12700, 1e-6));
        QVERIFY(box->fill.isNone() && box->stroke.isNone());
        QVERIFY(box->autofit == TextItem::GrowBox);
        auto rounded = dmlAs<TextItem>(list[2]);      // a rectangle would be a text frame; this one is rounded, so a shape
        QVERIFY(!rounded);
        auto shape = dmlAs<ShapeItem>(list[2]);
        QVERIFY(shape);
        QCOMPARE(shape->shape, QStringLiteral("roundRect"));
        QCOMPARE(dmlHex(shape->fill.color), QStringLiteral("4472C4"));
        QVERIFY(near(shape->stroke.width, 1));
        QVERIFY(shape->valign == VAlign::Middle);
        QVERIFY(!shape->storyId.isEmpty());
        auto picture = dmlAs<PictureItem>(list[3]);
        QVERIFY(picture && !picture->imageId.isEmpty());
        QCOMPARE(picture->altText, QStringLiteral("A red square"));
        auto group = dmlAs<GroupItem>(list[4]);
        QVERIFY(group && group->children.size() == 2);
        QVERIFY(dmlRectIs(group->children[0]->rect, 504, 120, 72, 90));
        QVERIFY(dmlRectIs(group->rect, 504, 120, 180, 90));
        auto elbow = dmlAs<LineItem>(list[5]);
        QVERIFY(elbow && elbow->route == LineItem::Elbow);
        QVERIFY(elbow->stroke.endArrow == Arrow::Triangle);
        QCOMPARE(elbow->start.id, list[2]->id);
        QCOMPARE(elbow->start.site, 3);
        QCOMPARE(elbow->end.id, list[3]->id);
        QCOMPARE(elbow->end.site, 1);
        // What is read can be saved and opened again by the program's own format.
        jp::walkItems(list, [&](const ItemPtr &it) {
            const ItemPtr again = Item::fromJsonAny(it->toJson());
            QVERIFY(again && again->type() == it->type());
            QVERIFY(dmlRectIs(again->rect, it->rect.x(), it->rect.y(), it->rect.width(), it->rect.height(), 1e-9));
        });
        // The same title as a placeholder with a layout to take its place from.
        QDomElement titleElement = spTree.firstChildElement();
        while (!titleElement.isNull() && titleElement.localName() != QLatin1String("sp")) titleElement = titleElement.nextSiblingElement();
        QVERIFY(!titleElement.isNull());
        const ItemPtr placed = readShape(titleElement, env.ctx, {}, {env.element(layoutTitle)});
        QVERIFY2(dmlAs<TextItem>(placed), qPrintable(placed ? QString::number(int(placed->type())) : QStringLiteral("none")));
        QVERIFY(dmlRectIs(placed->rect, 100, 30, 760, 80));
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

    // What a shape leaves out, the drawing's table of default properties says (its colors, whether the fill and the line are drawn);
    // what the shape says wins, and so does a flag it uses, however the table has it.
    void odrawFallsBackOnTheDrawingsDefaultProperties()
    {
        OdrawContext ctx = odContext();
        // Filled in the fills color (scheme 4), a line in the text color (1), a shadow in the shadow color (2), a line 2 points wide.
        ctx.defaults = odProps({{0x0181, 0x08000004}, {0x0183, 0x08000000}, {0x01BF, 0x00100010}, {0x01C0, 0x08000001}, {0x01CB, 25400}, {0x01FF, 0x00080008}, {0x0201, 0x08000002}});
        auto shapeOf = [&](const QVector<OdProp> &props, int type = 1) {
            return readOdrawShapes(odDrawing(odShape(type, 0xA00, props, odClientAnchor(0, 0, 10, 10))), ctx)[0];
        };
        ItemPtr it = shapeOf({});
        QVERIFY(it->fill.type == Fill::Solid);
        QCOMPARE(odRgb(it->fill.color), QColor(40, 24, 38));
        QCOMPARE(odRgb(it->stroke.color), QColor(10, 21, 32));
        QVERIFY(qAbs(it->stroke.width - 2.0) < 1e-9);
        // The shape's own colors and width win.
        it = shapeOf({{0x0181, 0x0000FF}, {0x01CB, 12700}});
        QCOMPARE(odRgb(it->fill.color), QColor(255, 0, 0));
        QVERIFY(qAbs(it->stroke.width - 1.0) < 1e-9);
        // The colors a gradient has are the table's too.
        it = shapeOf({{0x0180, 4}});
        QVERIFY(it->fill.type == Fill::Gradient);
        QCOMPARE(odRgb(it->fill.stops.first().color), QColor(40, 24, 38));
        QCOMPARE(odRgb(it->fill.stops.last().color), QColor(0, 20, 30));
        // A system color that stands for the shape's fill color means the table's when the shape has none of its own.
        QCOMPARE(odRgb(shapeOf({{0x01C0, 0x100000F0}})->stroke.color), QColor(40, 24, 38));
        // The shape turns the fill off (its use bit set, its value clear); a flag it doesn't use, the table's stays.
        QVERIFY(shapeOf({{0x01BF, 0x00100000}})->fill.isNone());
        QVERIFY(shapeOf({{0x01FF, 0x00080000}})->stroke.isNone());
        QVERIFY(!shapeOf({{0x01BF, 0x00020000}})->fill.isNone());
        QVERIFY(!shapeOf({{0x01BF, 0x00200020}})->fill.isNone());
        // A table that leaves the shapes unfilled: a rectangle is, unless it says it isn't.
        ctx.defaults = odProps({{0x01BF, 0x00100000}, {0x01FF, 0x00080000}});
        QVERIFY(shapeOf({})->fill.isNone());
        QVERIFY(shapeOf({})->stroke.isNone());
        QVERIFY(!shapeOf({{0x01BF, 0x00100010}})->fill.isNone());
        // A table with no say about it leaves the format's own: a white fill and a thin black line.
        ctx.defaults = odProps({{0x0183, 0x0000FF}});
        it = shapeOf({});
        QCOMPARE(odRgb(it->fill.color), QColor(Qt::white));
        QCOMPARE(odRgb(it->stroke.color), QColor(Qt::black));
        QVERIFY(qAbs(it->stroke.width - 0.75) < 1e-9);
        // No table, or one that can't be read: the same.
        for (const QByteArray &none : {QByteArray(), QByteArray("junk"), odRecord(3, 40, 0xF00B, QByteArray(10, '\x01'))}) {
            ctx.defaults = none;
            QCOMPARE(odRgb(shapeOf({})->fill.color), QColor(Qt::white));
        }
        // The table's other records (the tertiary one, a second primary) read as well, a later one replacing an earlier.
        ctx.defaults = odProps({{0x0181, 0x0000FF}}) + odProps({{0x0181, 0x00FF00}}, 0xF122) + odRecord(0, 0, 0xF006, QByteArray(16, '\0'));
        QCOMPARE(odRgb(shapeOf({})->fill.color), QColor(0, 255, 0));
        // The background shape's fill falls back on it too.
        ctx.defaults = odProps({{0x0181, 0x08000004}, {0x01BF, 0x00100010}});
        const Fill bg = odrawBackground(odDrawing(QByteArray(), odShape(1, 0x400, {})), ctx);
        QVERIFY(bg.type == Fill::Solid);
        QCOMPARE(odRgb(bg.color), QColor(40, 24, 38));
    }

    // The flags of a boolean property (the fill's, the line's) sit in one property that the shape's tables may each hold a part of:
    // each table's flags that it uses are the shape's.
    void odrawMergesTheFlagsOfAPropertyAcrossTables()
    {
        const QByteArray at = odClientAnchor(0, 0, 10, 10);
        // Primary: the line is used, and off; tertiary: another flag of the same property (the pen is inset). The line stays off.
        QByteArray tables = odProps({{0x01BF, 0x00100010}, {0x01FF, 0x00080000}}) + odProps({{0x01FF, 0x00400040}}, 0xF122);
        ItemList items = readOdrawShapes(odDrawing(odContainer(0xF004, odFsp(1, 0xA00) + tables + at)), odContext());
        QVERIFY(items[0]->stroke.isNone());
        QVERIFY(!items[0]->fill.isNone());
        // The later table has the say over a flag both use.
        tables = odProps({{0x01FF, 0x00080008}}) + odProps({{0x01FF, 0x00480000}}, 0xF122);
        items = readOdrawShapes(odDrawing(odContainer(0xF004, odFsp(1, 0xA00) + tables + at)), odContext());
        QVERIFY(items[0]->stroke.isNone());
        // A flag the later table doesn't use stays as it was; one neither uses is the default's.
        tables = odProps({{0x01FF, 0x00080008}}) + odProps({{0x01FF, 0x00400000}}, 0xF122);
        items = readOdrawShapes(odDrawing(odContainer(0xF004, odFsp(1, 0xA00) + tables + at)), odContext());
        QVERIFY(!items[0]->stroke.isNone());
        // Properties that are not flags are replaced, as they were.
        tables = odProps({{0x01CB, 25400}}) + odProps({{0x01CB, 12700}}, 0xF121);
        items = readOdrawShapes(odDrawing(odContainer(0xF004, odFsp(1, 0xA00) + tables + at)), odContext());
        QVERIFY(qAbs(items[0]->stroke.width - 1.0) < 1e-9);
    }

    // A shape that takes after another (a slide's placeholder after its master's) has what that shape's table says for what it leaves out,
    // and the drawing's defaults for what neither says.
    void odrawTakesWhatItLeavesOutFromTheShapeItTakesAfter()
    {
        OdrawContext ctx = odContext();
        // Defaults: filled in scheme color 4, and a line.
        ctx.defaults = odProps({{0x0181, 0x08000004}, {0x01BF, 0x00100010}, {0x01C0, 0x08000001}, {0x01FF, 0x00080008}});
        QStringList asked;
        ctx.inherits = [&](const QByteArray &data) {
            asked << QString::fromLatin1(data);
            // One shape's table: not filled and no line, a green line color, 3 points wide.
            return data == "title" ? odProps({{0x01BF, 0x00100000}, {0x01FF, 0x00080000}, {0x01C0, 0x00FF00}, {0x01CB, 38100}}) : QByteArray();
        };
        const QByteArray at = odClientAnchor(0, 0, 10, 10);
        auto shapeOf = [&](const QByteArray &data, const QVector<OdProp> &props) {
            return readOdrawShapes(odDrawing(odShape(1, 0xA00, props, at, odRecord(0, 0, 0xF011, data))), ctx)[0];
        };
        ItemPtr it = shapeOf("title", {});
        QCOMPARE(asked, QStringList({QStringLiteral("title")}));   // the client data's contents
        QVERIFY(it->fill.isNone());
        QVERIFY(it->stroke.isNone());
        // What the shape says wins, and the colors it leaves out are the other shape's, then the defaults'.
        it = shapeOf("title", {{0x01BF, 0x00100010}, {0x01FF, 0x00080008}});
        QCOMPARE(odRgb(it->fill.color), QColor(40, 24, 38));
        QCOMPARE(odRgb(it->stroke.color), QColor(0, 255, 0));
        QVERIFY(qAbs(it->stroke.width - 3.0) < 1e-9);
        QCOMPARE(odRgb(shapeOf("title", {{0x01BF, 0x00100010}, {0x0181, 0x0000FF}})->fill.color), QColor(255, 0, 0));
        // A shape that takes after none, or one with no client data, has the defaults.
        it = shapeOf("body", {});
        QCOMPARE(odRgb(it->fill.color), QColor(40, 24, 38));
        asked.clear();
        it = readOdrawShapes(odDrawing(odShape(1, 0xA00, {}, at)), ctx)[0];
        QVERIFY(asked.isEmpty());
        QCOMPARE(odRgb(it->fill.color), QColor(40, 24, 38));
    }

    // The colors of a gradient (fillShadeColors, property 0x0197; the one before it is the preset's number) run along it as the focus
    // says, like the two colors do: at 100 they run the other way, at 50 there and back, and below zero from the other end.
    void odrawShadeColorsFollowTheFocus()
    {
        auto stopsOf = [&](int focus, const QByteArray &list, const QVector<OdProp> &more = {}) {
            QVector<OdProp> props = {{0x01BF, 0x00100010}, {0x019C, 0}, {0x0180, 4}, {0x0181, 0x0000FF}, {0x0183, 0xFF0000}, {0x018C, quint32(focus)}, {0x0197, 0, odArray(list.size() / 8, 8, list)}};
            props += more;
            const Fill f = readOdrawShapes(odDrawing(odShape(1, 0xA00, props, odClientAnchor(0, 0, 10, 10))), odContext())[0]->fill;
            QStringList out;
            for (const GradientStop &st : f.stops)
                out << QStringLiteral("%1%2").arg(odRgb(st.color) == QColor(255, 0, 0) ? 'r' : odRgb(st.color) == QColor(0, 0, 255) ? 'b' : 'g').arg(st.pos, 0, 'f', 2);
            return out.join(QLatin1Char(' '));
        };
        // Red, then red up to 65 percent, then blue: how a dark band over a fade is stored.
        QByteArray band;
        band += odU32(0x000000FF) + odU32(0);
        band += odU32(0x000000FF) + odU32(0xA666);
        band += odU32(0x00FF0000) + odU32(65536);
        QCOMPARE(stopsOf(0, band), QStringLiteral("r0.00 r0.65 b1.00"));
        QCOMPARE(stopsOf(100, band), QStringLiteral("b0.00 r0.35 r1.00"));
        QCOMPARE(stopsOf(-100, band), QStringLiteral("r0.00 r0.65 b1.00"));   // from the other end and the other way: as at 0
        QCOMPARE(stopsOf(50, band), QStringLiteral("r0.00 r0.32 b0.50 r0.68 r1.00"));
        // Two colors in a list are the same as two colors without one.
        QByteArray two;
        two += odU32(0x000000FF) + odU32(0);
        two += odU32(0x00FF0000) + odU32(65536);
        for (int focus : {0, 50, 100, -100, -50}) {
            const Fill plain = readOdrawShapes(odDrawing(odShape(1, 0xA00, {{0x01BF, 0x00100010}, {0x019C, 0}, {0x0180, 4}, {0x0181, 0x0000FF}, {0x0183, 0xFF0000}, {0x018C, quint32(focus)}}, odClientAnchor(0, 0, 10, 10))),
                                              odContext())[0]->fill;
            QStringList expected;
            for (const GradientStop &st : plain.stops) expected << QStringLiteral("%1%2").arg(odRgb(st.color) == QColor(255, 0, 0) ? 'r' : 'b').arg(st.pos, 0, 'f', 2);
            QCOMPARE(stopsOf(focus, two), expected.join(QLatin1Char(' ')));
        }
        // The property before it (the preset) isn't the list: a gradient with that one alone has its two colors.
        const Fill preset = readOdrawShapes(odDrawing(odShape(1, 0xA00, {{0x01BF, 0x00100010}, {0x019C, 0}, {0x0180, 4}, {0x0181, 0x0000FF}, {0x0183, 0xFF0000}, {0x0196, 0, odArray(3, 8, band)}},
                                                              odClientAnchor(0, 0, 10, 10))), odContext())[0]->fill;
        QCOMPARE(int(preset.stops.size()), 2);
    }

    // A shaded fill of two colors runs along a curve unless the shape turns it off (fillShadeType 0x019C: gamma 1, sigma 2): the position is
    // eased by the sigma curve (3u^2 - 2u^3) and the colors blend in linear light (a power of 2.2), as the program draws them; a gradient
    // that says nothing of it is drawn that way (gamma and sigma). A list of colors is not.
    void odrawShadesFollowTheirCurve()
    {
        auto stopsOf = [&](const QVector<OdProp> &more, const QByteArray &list = QByteArray()) {
            QVector<OdProp> props = {{0x01BF, 0x00100010}, {0x0180, 4}, {0x0181, 0x0000FF}, {0x0183, 0xFF0000}};
            if (!list.isEmpty()) props << OdProp{0x0197, 0, odArray(list.size() / 8, 8, list)};
            props += more;
            return readOdrawShapes(odDrawing(odShape(1, 0xA00, props, odClientAnchor(0, 0, 10, 10))), odContext())[0]->fill.stops;
        };
        // Red to blue, eight steps: the middle one, a quarter of the way eased by the sigma curve to the red or the blue.
        auto expectedRed = [&](double u, int type) {
            const double e = (type & 2) ? u * u * (3 - 2 * u) : u;
            return (type & 1) ? 255 * std::pow((1 - e) * 1.0, 1 / 2.2) : 255 * (1 - e);
        };
        for (const QVector<OdProp> &more : {QVector<OdProp>(), QVector<OdProp>{{0x019C, 3}}, QVector<OdProp>{{0x019C, 0x40000003}}}) {
            const QVector<GradientStop> st = stopsOf(more);
            QCOMPARE(int(st.size()), 9);
            QCOMPARE(odRgb(st.first().color), QColor(255, 0, 0));
            QCOMPARE(odRgb(st.last().color), QColor(0, 0, 255));
            for (int k = 1; k < 8; ++k) {
                QVERIFY(qAbs(st[k].pos - k / 8.0) < 1e-9);
                const QColor c = odRgb(st[k].color);
                QVERIFY2(qAbs(c.red() - expectedRed(k / 8.0, 3)) <= 1.5 && qAbs(c.blue() - expectedRed(1 - k / 8.0, 3)) <= 1.5 && c.green() == 0, qPrintable(QString::number(k)));
            }
        }
        // The middle of it: 186, not 128.
        QVERIFY(qAbs(odRgb(stopsOf({})[4].color).red() - 186) <= 1);
        // Sigma alone eases the position; gamma alone blends in linear light; neither is two colors.
        QVector<GradientStop> st = stopsOf({{0x019C, 2}});
        QCOMPARE(int(st.size()), 9);
        QVERIFY(qAbs(odRgb(st[1].color).red() - expectedRed(1 / 8.0, 2)) <= 1.5);
        QVERIFY(qAbs(odRgb(st[4].color).red() - 128) <= 1);
        st = stopsOf({{0x019C, 1}});
        QVERIFY(qAbs(odRgb(st[1].color).red() - expectedRed(1 / 8.0, 1)) <= 1.5);
        QCOMPARE(int(stopsOf({{0x019C, 0}}).size()), 2);
        QCOMPARE(int(stopsOf({{0x019C, 0x40000000}}).size()), 2);
        // A list of colors is drawn as it stands: the colors at their places, and nothing between.
        QByteArray list;
        list += odU32(0x000000FF) + odU32(0);
        list += odU32(0x000000FF) + odU32(0x8000);
        list += odU32(0x00FF0000) + odU32(65536);
        QCOMPARE(int(stopsOf({}, list).size()), 3);
        QCOMPARE(int(stopsOf({{0x019C, 3}}, list).size()), 3);
        // The opacity runs along it evenly.
        st = stopsOf({{0x0182, 0x10000}, {0x0184, 0x8000}});
        QVERIFY(qAbs(st[4].transparency - 0.25) < 1e-6);
    }

    // A shape that carries the XML its writer made for it (property 0x03A9) hands it to the host, which may take the shape's look from it.
    // A shape that takes after another doesn't have that one's XML, or its name, or its link.
    void odrawGivesTheWritersXmlToTheHost()
    {
        OdrawContext ctx = odContext();
        QStringList got;
        ctx.richLook = [&](Item &item, const QByteArray &xml) {
            got << QString::fromLatin1(xml);
            item.altText = QStringLiteral("seen");
        };
        ctx.inherits = [](const QByteArray &data) {
            return data == "ph" ? odProps({{0x03A9, 0, QByteArray("master xml")}, {0x0380, 0, odUtf16z(QStringLiteral("Master name"))}, {0x0381, 0, odUtf16z(QStringLiteral("Master alt"))},
                                           {0x0382, 0, odUrlHyperlink(QStringLiteral("https://example.org"), QString(), true)}, {0x0181, 0x0000FF}, {0x01BF, 0x00100010}})
                                   : QByteArray();
        };
        auto shapeOf = [&](const QVector<OdProp> &props, const QByteArray &data) {
            return readOdrawShapes(odDrawing(odShape(1, 0xA00, props, odClientAnchor(0, 0, 10, 10), data.isEmpty() ? QByteArray() : odRecord(0, 0, 0xF011, data))), ctx)[0];
        };
        ItemPtr it = shapeOf({{0x03A9, 0, QByteArray("<p:sp/>")}}, QByteArray());
        QCOMPARE(got, QStringList({QStringLiteral("<p:sp/>")}));
        QCOMPARE(it->altText, QStringLiteral("seen"));
        got.clear();
        it = shapeOf({}, "ph");
        QVERIFY(got.isEmpty());
        QVERIFY(it->name.isEmpty());
        QVERIFY(it->altText.isEmpty());
        QVERIFY(it->hyperlink.isEmpty());
        QCOMPARE(odRgb(it->fill.color), QColor(255, 0, 0));   // what it looks like, it does have
        // No host function, or no XML: nothing happens.
        ctx.richLook = nullptr;
        QVERIFY(shapeOf({{0x03A9, 0, QByteArray("<p:sp/>")}}, QByteArray())->altText.isEmpty());
    }

    // A picture or texture fill has the brightness and contrast of its shape (a texture washed out for a background): the host makes
    // the picture the fill shows from them, as the picture items have them (a percent each way).
    void odrawFillsAdjustTheirPictures()
    {
        OdrawContext ctx = odContext();
        QStringList asked;
        ctx.adjusted = [&](const QString &id, double brightness, double contrast) {
            asked << QStringLiteral("%1 %2 %3").arg(id).arg(brightness, 0, 'f', 1).arg(contrast, 0, 'f', 1);
            return id + QLatin1Char('+');
        };
        auto fillOf = [&](int type, const QVector<OdProp> &more) {
            QVector<OdProp> props = {{0x01BF, 0x00100010}, {0x0180, quint32(type)}, {0x0186, 2, {}, true}};
            props += more;
            return readOdrawShapes(odDrawing(odShape(1, 0xA00, props, odClientAnchor(0, 0, 10, 10))), ctx)[0]->fill;
        };
        // Brightness 0x5C29 of 0x8000 (72 percent), and contrast 0xD70A of 0x10000 (16 percent less than none).
        Fill f = fillOf(2, {{0x0109, 0x5C29}, {0x0108, 0xD70A}});
        QVERIFY(f.type == Fill::Texture);
        QCOMPARE(f.imageId, QStringLiteral("pic2+"));
        QCOMPARE(asked, QStringList({QStringLiteral("pic2 72.0 -16.0")}));
        asked.clear();
        QCOMPARE(fillOf(3, {{0x0109, 0xFFFFC000}}).imageId, QStringLiteral("pic2+"));
        QCOMPARE(asked, QStringList({QStringLiteral("pic2 -50.0 0.0")}));
        asked.clear();
        QCOMPARE(fillOf(1, {{0x0108, 0x20000}}).imageId, QStringLiteral("pic2+"));
        QCOMPARE(asked, QStringList({QStringLiteral("pic2 0.0 100.0")}));
        // Neither set: the picture as it is, and the host isn't asked.
        asked.clear();
        QCOMPARE(fillOf(2, {}).imageId, QStringLiteral("pic2"));
        QVERIFY(asked.isEmpty());
        // A picture the host has none for stays solid.
        ctx.picture = nullptr;
        QVERIFY(fillOf(2, {{0x0109, 0x5C29}}).type == Fill::Solid);
        QVERIFY(asked.isEmpty());
    }

    // A picture takes its brightness and contrast the same way when the host can make the picture as adjusted: the picture is the
    // adjusted one, and the item has none of its own; where it can't, the item has them.
    void odrawPicturesTakeAdjustedImagesFromTheHost()
    {
        OdrawContext ctx = odContext();
        QStringList asked;
        bool works = true;
        ctx.adjusted = [&](const QString &id, double brightness, double contrast) {
            asked << QStringLiteral("%1 %2 %3").arg(id).arg(brightness, 0, 'f', 1).arg(contrast, 0, 'f', 1);
            return works ? id + QLatin1Char('+') : id;
        };
        // The list read is kept until the next: the picture is one of its items.
        ItemList read;
        auto pictureOf = [&](const QVector<OdProp> &props) {
            read = readOdrawShapes(odDrawing(odShape(75, 0xA00, props, odClientAnchor(0, 0, 10, 10))), ctx);
            return static_cast<PictureItem *>(read[0].get());
        };
        PictureItem *pic = pictureOf({{0x0104, 3, {}, true}, {0x0109, 0x5C29}, {0x0108, 0xD70A}});
        QCOMPARE(pic->imageId, QStringLiteral("pic3+"));
        QCOMPARE(pic->brightness, 0.0);
        QCOMPARE(pic->contrast, 0.0);
        QCOMPARE(asked, QStringList({QStringLiteral("pic3 72.0 -16.0")}));
        // Nothing to adjust: the host isn't asked.
        asked.clear();
        pic = pictureOf({{0x0104, 3, {}, true}});
        QCOMPARE(pic->imageId, QStringLiteral("pic3"));
        QVERIFY(asked.isEmpty());
        // The host can't: the item has them, as when there is no host.
        works = false;
        pic = pictureOf({{0x0104, 3, {}, true}, {0x0109, 0x5C29}});
        QCOMPARE(pic->imageId, QStringLiteral("pic3"));
        QVERIFY(qAbs(pic->brightness - 72.0) < 0.01);
        ctx.adjusted = nullptr;
        pic = pictureOf({{0x0104, 3, {}, true}, {0x0108, 0x20000}});
        QVERIFY(qAbs(pic->contrast - 100.0) < 0.01);
    }

    // A fill of the background kind shows the page's own background where the shape is: the host says what that is, given the place.
    void odrawFillsWithThePagesBackground()
    {
        OdrawContext ctx = odContext();
        auto fillOf = [&]() {
            return readOdrawShapes(odDrawing(odShape(1, 0xA00, {{0x0180, 9}, {0x01BF, 0x00100010}, {0x0181, 0x0000FF}}, odClientAnchor(0, 0, 10, 10))), ctx)[0]->fill;
        };
        QVERIFY(fillOf().isNone());   // no page to say
        Fill background = Fill::gradient(ColorRef::rgb(Qt::red), ColorRef::rgb(Qt::blue));
        QRectF seen;
        ctx.background = [&](const QRectF &shape) {
            seen = shape;
            return background;
        };
        Fill f = fillOf();
        QCOMPARE(seen, QRectF(0, 0, 10, 10));   // where the shape is, for a fill that keeps its place on the page
        QVERIFY(f.type == Fill::Gradient);
        QCOMPARE(odRgb(f.color), QColor(Qt::red));
        QCOMPARE(odRgb(f.color2), QColor(Qt::blue));
        background = Fill::solid(ColorRef::rgb(Qt::green));
        QVERIFY(fillOf().type == Fill::Solid);
        // Not filled at all: nothing, whatever the page has.
        QVERIFY(readOdrawShapes(odDrawing(odShape(1, 0xA00, {{0x0180, 9}, {0x01BF, 0x00100000}}, odClientAnchor(0, 0, 10, 10))), ctx)[0]->fill.isNone());
    }

    // ---- text bodies (DrawingML text read into stories)

private:
    // The namespace declarations of a fragment; the strict forms may be given.
    static QString declarations(const QString &a = QString(ns::A), const QString &p = QString(ns::P), const QString &r = QString(ns::R))
    {
        return QStringLiteral("xmlns:a=\"%1\" xmlns:p=\"%2\" xmlns:r=\"%3\"").arg(a, p, r);
    }

    // A text body read into a story. `paragraphs` is the XML of its a:p
    // elements; `layers` are the children of one a:lstStyle each, the lowest
    // priority first (the stack the shape would hand over). `lstStyle` is the
    // body's own.
    struct Body {
        std::vector<std::unique_ptr<QDomDocument>> parsed;
        QTextDocument doc;
        DrawingContext ctx;
        TextStyleStack styles;

        QDomElement element(const QString &xml)
        {
            parsed.push_back(std::make_unique<QDomDocument>());
            QString error;
            if (!parseXml(xml.toUtf8(), parsed.back().get(), &error)) qWarning("bad test XML: %s", qPrintable(error));
            return parsed.back()->documentElement();
        }

        explicit Body(const QString &paragraphs, const QStringList &layers = {}, const QString &lstStyle = QString(), const Package *package = nullptr,
                      const Theme &theme = colorTestTheme(), const QString &decl = declarations())
        {
            ctx.theme = theme;
            ctx.package = package;
            if (package) ctx.part = QStringLiteral("/ppt/slides/slide1.xml");
            for (const QString &layer : layers) styles.append(element(QStringLiteral("<a:lstStyle %1>%2</a:lstStyle>").arg(decl, layer)));
            readTextBody(element(QStringLiteral("<p:txBody %1><a:bodyPr/><a:lstStyle>%2</a:lstStyle>%3</p:txBody>").arg(decl, lstStyle, paragraphs)),
                         &doc, ctx, styles);
        }

        // The blocks' text, joined by line feeds: toPlainText turns a line separator into one too.
        QString text() const
        {
            QStringList lines;
            for (QTextBlock b = doc.begin(); b.isValid(); b = b.next()) lines << b.text();
            return lines.join(QLatin1Char('\n'));
        }
        QTextBlock block(int n = 0) const { return doc.findBlockByNumber(n); }
        QTextBlockFormat blockFormat(int n = 0) const { return block(n).blockFormat(); }
        // The character format of the letter at `offset` in the block.
        QTextCharFormat charAt(int n = 0, int offset = 0) const
        {
            QTextCursor c(block(n));
            c.setPosition(block(n).position() + offset + 1);
            return c.charFormat();
        }
    };

    // <a:p> with one run of `text` whose a:rPr has the given attributes and children.
    static QString run(const QString &text, const QString &attrs = QString(), const QString &children = QString())
    {
        return QStringLiteral("<a:r><a:rPr %1>%2</a:rPr><a:t>%3</a:t></a:r>").arg(attrs, children, text);
    }
    static QString para(const QString &runs, const QString &pPr = QString())
    {
        return QStringLiteral("<a:p>%1%2</a:p>").arg(pPr, runs);
    }
    static QString solid(const QString &hex) { return QStringLiteral("<a:solidFill><a:srgbClr val=\"%1\"/></a:solidFill>").arg(hex); }

private Q_SLOTS:
    void textBodyReadsParagraphsAndRunsExactly()
    {
        Body b(QStringLiteral("<a:p><a:r><a:t>Hello </a:t></a:r><a:r><a:t>world</a:t></a:r></a:p>"
                              "<a:p><a:r><a:t>  Second\there  </a:t></a:r></a:p>"));
        QCOMPARE(b.doc.blockCount(), 2);
        QCOMPARE(b.text(), QStringLiteral("Hello world\n  Second\there  "));
    }

    void textBodyWithoutParagraphsIsOneEmptyParagraph()
    {
        Body none{QString()};
        QCOMPARE(none.doc.blockCount(), 1);
        QVERIFY(none.doc.isEmpty());
        // Reading replaces what the story held, and a missing element or
        // story is no crash.
        QTextDocument doc;
        doc.setPlainText(QStringLiteral("old\ntext"));
        readTextBody(QDomElement(), &doc, DrawingContext());
        QCOMPARE(doc.blockCount(), 1);
        QVERIFY(doc.isEmpty());
        readTextBody(QDomElement(), nullptr, DrawingContext());
    }

    void textBodyReadsTheStrictNamespaces()
    {
        Body b(QStringLiteral("<a:p><a:r><a:rPr sz=\"2000\" b=\"1\"/><a:t>strict</a:t></a:r></a:p>"), {}, QString(), nullptr, colorTestTheme(),
               declarations(QString(ns::AStrict), QString(ns::PStrict), QString(ns::RStrict)));
        QCOMPARE(b.text(), QStringLiteral("strict"));
        QCOMPARE(b.charAt().fontPointSize(), 20.0);
        QCOMPARE(b.charAt().fontWeight(), int(QFont::Bold));
    }

    void textHasThePresentationProgramsDefaultSize()
    {
        Body b(para(QStringLiteral("<a:r><a:t>x</a:t></a:r>")));
        QCOMPARE(b.charAt().fontPointSize(), 18.0);
    }

    void textInheritsFromThreeStyleLayers()
    {
        // Lowest first: a size, italic, and a theme color; then a bigger
        // size; then bold. The first run adds a color of its own, the second
        // its own size and no bold.
        const QStringList layers = {
            QStringLiteral("<a:lvl1pPr><a:defRPr sz=\"1800\" i=\"1\" b=\"0\"><a:solidFill><a:schemeClr val=\"accent1\"/></a:solidFill></a:defRPr></a:lvl1pPr>"),
            QStringLiteral("<a:lvl1pPr><a:defRPr sz=\"2400\"/></a:lvl1pPr>"),
            QStringLiteral("<a:lvl1pPr><a:defRPr b=\"1\"/></a:lvl1pPr>")};
        Body b(para(run(QStringLiteral("one"), QString(), solid(QStringLiteral("FF0000"))) + run(QStringLiteral(" two"), QStringLiteral("sz=\"3000\" b=\"0\""))), layers);
        const QTextCharFormat one = b.charAt(0, 0), two = b.charAt(0, 5);
        QCOMPARE(one.fontPointSize(), 24.0);
        QCOMPARE(one.fontWeight(), int(QFont::Bold));
        QVERIFY(one.fontItalic());
        QCOMPARE(one.foreground().color(), QColor(0xFF, 0, 0));
        QCOMPARE(one.stringProperty(jp::tp::ColorRefP), QStringLiteral("#FF0000"));
        QCOMPARE(two.fontPointSize(), 30.0);
        QVERIFY(two.fontWeight() != QFont::Bold);
        QVERIFY(two.fontItalic());
        QCOMPARE(two.foreground().color(), QColor(0x44, 0x72, 0xC4));
        QCOMPARE(two.stringProperty(jp::tp::ColorRefP), QStringLiteral("#4472C4"));
    }

    void textTakesTheDefaultsOfAListOnlyWhereTheLevelsLeaveGaps()
    {
        // a:defPPr is below the level of its own layer: it fills in a size
        // the level doesn't give, and the level's wins where both say.
        const QStringList layers = {
            QStringLiteral("<a:defPPr><a:defRPr sz=\"1000\" i=\"1\"/></a:defPPr><a:lvl2pPr><a:defRPr sz=\"2200\"/></a:lvl2pPr>")};
        Body b(para(run(QStringLiteral("a"))) + para(run(QStringLiteral("b")), QStringLiteral("<a:pPr lvl=\"1\"/>")), layers);
        QCOMPARE(b.charAt(0).fontPointSize(), 10.0);
        QVERIFY(b.charAt(0).fontItalic());
        QCOMPARE(b.charAt(1).fontPointSize(), 22.0);
        QVERIFY(b.charAt(1).fontItalic());
    }

    void textBodysOwnListStyleComesFirst()
    {
        const QStringList layers = {QStringLiteral("<a:lvl1pPr algn=\"r\"><a:defRPr sz=\"2400\"/></a:lvl1pPr>")};
        Body b(para(run(QStringLiteral("x"))), layers, QStringLiteral("<a:lvl1pPr algn=\"ctr\"><a:defRPr sz=\"4000\"/></a:lvl1pPr>"));
        QCOMPARE(b.charAt().fontPointSize(), 40.0);
        QCOMPARE(int(b.blockFormat().alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignHCenter));
        // Handed over as the top of the stack already, it reads the same.
        Body again{QString()};
        const QDomElement body = again.element(QStringLiteral("<p:txBody %1><a:bodyPr/><a:lstStyle><a:lvl1pPr><a:defRPr sz=\"3600\"/></a:lvl1pPr></a:lstStyle>%2</p:txBody>")
                                                   .arg(declarations(), para(run(QStringLiteral("x")))));
        readTextBody(body, &again.doc, again.ctx, TextStyleStack{body.firstChildElement().nextSiblingElement()});
        QCOMPARE(again.charAt().fontPointSize(), 36.0);
    }

    void textReadsAStackShapedLikeAPresentations()
    {
        // The presentation's default text style, a master's body style, and a
        // layout placeholder's list style, lowest first.
        const QStringList layers = {
            QStringLiteral("<a:defPPr><a:defRPr lang=\"en-US\"/></a:defPPr><a:lvl1pPr marL=\"0\" algn=\"l\" defTabSz=\"914400\" rtl=\"0\">"
                           "<a:defRPr sz=\"1800\" kern=\"1200\"><a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill><a:latin typeface=\"+mn-lt\"/>"
                           "<a:ea typeface=\"+mn-ea\"/><a:cs typeface=\"+mn-cs\"/></a:defRPr></a:lvl1pPr>"),
            QStringLiteral("<a:lvl1pPr marL=\"228600\" indent=\"-228600\"><a:lnSpc><a:spcPct val=\"90000\"/></a:lnSpc><a:spcBef><a:spcPts val=\"1000\"/></a:spcBef>"
                           "<a:buFont typeface=\"Arial\"/><a:buChar char=\"&#8226;\"/><a:defRPr sz=\"2800\"/></a:lvl1pPr>"
                           "<a:lvl2pPr marL=\"685800\" indent=\"-228600\"><a:lnSpc><a:spcPct val=\"90000\"/></a:lnSpc><a:spcBef><a:spcPts val=\"500\"/></a:spcBef>"
                           "<a:buFont typeface=\"Arial\"/><a:buChar char=\"&#8226;\"/><a:defRPr sz=\"2400\"/></a:lvl2pPr>"),
            QStringLiteral("<a:lvl1pPr><a:defRPr b=\"1\"/></a:lvl1pPr>")};
        Body b(para(run(QStringLiteral("Heading"), QString(), QStringLiteral("<a:latin typeface=\"+mj-lt\"/>"))) + para(run(QStringLiteral("Detail")), QStringLiteral("<a:pPr lvl=\"1\"/>")), layers);
        QCOMPARE(b.charAt(0).fontPointSize(), 28.0);
        QCOMPARE(b.charAt(0).fontWeight(), int(QFont::Bold));
        QCOMPARE(b.charAt(0).stringProperty(jp::tp::ThemeFont), QStringLiteral("major"));
        QCOMPARE(b.charAt(0).property(jp::tp::KernAbove).toDouble(), 12.0);
        QCOMPARE(b.charAt(0).stringProperty(jp::tp::Language), QStringLiteral("en-US"));
        QCOMPARE(b.charAt(0).stringProperty(jp::tp::ColorRefP), QStringLiteral("#101010"));
        QCOMPARE(b.blockFormat(0).leftMargin(), 18.0);
        QCOMPARE(b.blockFormat(0).textIndent(), -18.0);
        QCOMPARE(b.blockFormat(0).topMargin(), 10.0);
        QCOMPARE(b.blockFormat(0).lineHeight(), 90.0);
        QVERIFY(b.block(0).textList());
        // The level below has its own size and margins, and nothing of the first layer's bold.
        QCOMPARE(b.charAt(1).fontPointSize(), 24.0);
        QVERIFY(b.charAt(1).fontWeight() != QFont::Bold);
        QCOMPARE(b.charAt(1).stringProperty(jp::tp::ThemeFont), QString());
        QCOMPARE(b.blockFormat(1).leftMargin(), 54.0);
        QCOMPARE(b.blockFormat(1).topMargin(), 5.0);
        QCOMPARE(b.block(1).textList()->format().intProperty(jp::tp::ListLevel), 1);
    }

    void textLevelsHaveTheirOwnIndentsAndBullets()
    {
        const QStringList layers = {QStringLiteral(
            "<a:lvl1pPr marL=\"0\" indent=\"0\"><a:buNone/><a:defRPr sz=\"3200\"/></a:lvl1pPr>"
            "<a:lvl2pPr marL=\"457200\" indent=\"-285750\"><a:buFont typeface=\"Arial\"/><a:buChar char=\"&#8226;\"/><a:defRPr sz=\"2800\"/></a:lvl2pPr>"
            "<a:lvl3pPr marL=\"914400\" indent=\"-228600\"><a:buFont typeface=\"Wingdings\"/><a:buChar char=\"&#167;\"/></a:lvl3pPr>")};
        Body b(para(run(QStringLiteral("one"))) + para(run(QStringLiteral("two")), QStringLiteral("<a:pPr lvl=\"1\"/>"))
                   + para(run(QStringLiteral("three")), QStringLiteral("<a:pPr lvl=\"2\"/>")),
               layers);
        QCOMPARE(b.doc.blockCount(), 3);
        // Level 1: no bullet; a block at level 0.
        QVERIFY(!b.block(0).textList());
        QCOMPARE(b.blockFormat(0).leftMargin(), 0.0);
        QCOMPARE(b.blockFormat(0).textIndent(), 0.0);
        QCOMPARE(b.blockFormat(0).intProperty(jp::tp::ListLevel), 0);
        QCOMPARE(b.charAt(0).fontPointSize(), 32.0);
        // Level 2: a bullet in Arial, the text 36 pt in, the bullet 22.5 pt before it.
        QTextList *two = b.block(1).textList();
        QVERIFY(two);
        QCOMPARE(two->format().style(), QTextListFormat::ListDisc);
        QCOMPARE(two->format().stringProperty(jp::tp::BulletChar), QString(QChar(0x2022)));
        QCOMPARE(two->format().stringProperty(jp::tp::BulletFont), QStringLiteral("Arial"));
        QCOMPARE(two->format().intProperty(jp::tp::ListLevel), 1);
        QCOMPARE(b.blockFormat(1).intProperty(jp::tp::ListLevel), 1);
        QCOMPARE(b.blockFormat(1).leftMargin(), 36.0);
        QCOMPARE(b.blockFormat(1).textIndent(), -22.5);
        QCOMPARE(b.charAt(1).fontPointSize(), 28.0);
        // Level 3: another mark in another font, indented further; its size is the list's (none) so the default.
        QTextList *three = b.block(2).textList();
        QVERIFY(three);
        QCOMPARE(three->format().stringProperty(jp::tp::BulletChar), QString(QChar(0xA7)));
        QCOMPARE(three->format().stringProperty(jp::tp::BulletFont), QStringLiteral("Wingdings"));
        QCOMPARE(three->format().intProperty(jp::tp::ListLevel), 2);
        QCOMPARE(b.blockFormat(2).leftMargin(), 72.0);
        QCOMPARE(b.blockFormat(2).textIndent(), -18.0);
        QCOMPARE(b.charAt(2).fontPointSize(), 18.0);
    }

    void textBulletsInheritTheirPartsSeparately()
    {
        // The layer gives the color and the size; the paragraph the mark and
        // the font; the next paragraph turns the bullet off, then follows the
        // text's color and size again.
        const QStringList layers = {QStringLiteral("<a:lvl1pPr><a:buClr><a:srgbClr val=\"00FF00\"/></a:buClr><a:buSzPct val=\"50000\"/></a:lvl1pPr>")};
        Body b(para(run(QStringLiteral("a"), QStringLiteral("sz=\"2000\"")), QStringLiteral("<a:pPr><a:buFont typeface=\"Courier New\"/><a:buChar char=\"-\"/></a:pPr>"))
                   + para(run(QStringLiteral("b")), QStringLiteral("<a:pPr><a:buNone/></a:pPr>"))
                   + para(run(QStringLiteral("c"), QStringLiteral("sz=\"2000\"")),
                          QStringLiteral("<a:pPr><a:buClrTx/><a:buSzPts val=\"1500\"/><a:buFontTx/><a:buChar char=\"*\"/></a:pPr>"))
                   + para(run(QStringLiteral("d"), QStringLiteral("sz=\"2000\"")), QStringLiteral("<a:pPr><a:buClrTx/><a:buSzTx/><a:buBlip/></a:pPr>")),
               layers);
        const QTextListFormat a = b.block(0).textList()->format();
        QCOMPARE(a.stringProperty(jp::tp::BulletChar), QStringLiteral("-"));
        QCOMPARE(a.stringProperty(jp::tp::BulletFont), QStringLiteral("Courier New"));
        QCOMPARE(jp::ColorRef::fromString(a.stringProperty(jp::tp::BulletColor)).rgbValue(), QColor(0, 255, 0));
        QCOMPARE(a.property(jp::tp::BulletSize).toDouble(), 10.0);   // half of the first run's 20 pt
        QVERIFY(!b.block(1).textList());
        const QTextListFormat c = b.block(2).textList()->format();
        QCOMPARE(c.stringProperty(jp::tp::BulletChar), QStringLiteral("*"));
        QVERIFY(!c.hasProperty(jp::tp::BulletColor));
        QVERIFY(!c.hasProperty(jp::tp::BulletFont));
        QCOMPARE(c.property(jp::tp::BulletSize).toDouble(), 15.0);
        // A picture bullet is a plain one.
        const QTextListFormat d = b.block(3).textList()->format();
        QCOMPARE(d.style(), QTextListFormat::ListDisc);
        QVERIFY(!d.hasProperty(jp::tp::BulletChar));
        QVERIFY(!d.hasProperty(jp::tp::BulletColor));
        QVERIFY(!d.hasProperty(jp::tp::BulletSize));
    }

    void textBulletFontsMayNameTheThemeFonts()
    {
        Theme theme;
        QVERIFY(readTheme(handWrittenTheme(), &theme));
        Body b(para(run(QStringLiteral("a")), QStringLiteral("<a:pPr><a:buFont typeface=\"+mj-lt\"/><a:buChar char=\"o\"/></a:pPr>"))
                   + para(run(QStringLiteral("b")), QStringLiteral("<a:pPr><a:buFont typeface=\"+mn-lt\"/><a:buChar char=\"o\"/></a:pPr>")),
               {}, QString(), nullptr, theme);
        QCOMPARE(b.block(0).textList()->format().stringProperty(jp::tp::BulletFont), QStringLiteral("Georgia"));
        QCOMPARE(b.block(1).textList()->format().stringProperty(jp::tp::BulletFont), QStringLiteral("Verdana"));
    }

    void textBulletsDrawnAtTheirIndents()
    {
        // The hanging bullet of a one-level list, laid out: the text starts
        // at marL, the bullet an indent before it.
        Body b(para(run(QStringLiteral("alpha beta"), QStringLiteral("sz=\"2000\"")), QStringLiteral("<a:pPr marL=\"457200\" indent=\"-228600\"><a:buChar char=\"&#8226;\"/></a:pPr>")));
        jp::FrameSpec fs;
        fs.size = QSizeF(400, 200);
        fs.insets = QMarginsF(0, 0, 0, 0);
        jp::StoryLayout lay;
        lay.build(&b.doc, {fs}, jp::LayoutEnv());
        const auto lines = lay.lineInfo(0);
        QCOMPARE(lines.size(), 1);
        QVERIFY2(qAbs(lines[0].rect.x() - 36) < 0.01, qPrintable(QString::number(lines[0].rect.x())));
    }

    void textAutonumbersContinueUntilInterrupted()
    {
        auto num = [](const QString &text, int lvl, const QString &bu) {
            return para(run(text), QStringLiteral("<a:pPr lvl=\"%1\">%2</a:pPr>").arg(lvl).arg(bu));
        };
        const QString arabic = QStringLiteral("<a:buAutoNum type=\"arabicPeriod\"/>");
        const QString arabicFrom5 = QStringLiteral("<a:buAutoNum type=\"arabicPeriod\" startAt=\"5\"/>");
        const QString alpha = QStringLiteral("<a:buAutoNum type=\"alphaLcParenR\"/>");
        Body b(num(QStringLiteral("a"), 0, arabic) + num(QStringLiteral("b"), 0, arabic)
                   + num(QStringLiteral("c"), 1, alpha) + num(QStringLiteral("d"), 1, alpha)   // nested, with their own count
                   + num(QStringLiteral("e"), 0, arabic)                                        // goes on at 3
                   + num(QStringLiteral("f"), 0, QStringLiteral("<a:buNone/>"))                 // interrupts
                   + num(QStringLiteral("g"), 0, arabic)                                        // starts again at 1
                   + num(QStringLiteral("h"), 0, arabicFrom5)                                   // another start, another count
                   + num(QStringLiteral("i"), 1, alpha)                                         // the outer item before it restarts the inner
                   + num(QStringLiteral("j"), 0, arabicFrom5),
               {});
        const QStringList want = {QStringLiteral("1."), QStringLiteral("2."), QStringLiteral("a)"), QStringLiteral("b)"), QStringLiteral("3."), QString(),
                                  QStringLiteral("1."), QStringLiteral("5."), QStringLiteral("a)"), QStringLiteral("6.")};
        QCOMPARE(b.doc.blockCount(), int(want.size()));
        for (int i = 0; i < want.size(); ++i) {
            QTextList *l = b.block(i).textList();
            QCOMPARE(l ? l->itemText(b.block(i)) : QString(), want[i]);
        }
    }

    void textEmptyParagraphsHaveNoBulletAndKeepTheNumbering()
    {
        const QString pPr = QStringLiteral("<a:pPr><a:buAutoNum type=\"arabicPeriod\"/></a:pPr>");
        Body b(para(run(QStringLiteral("a")), pPr) + QStringLiteral("<a:p>") + pPr + QStringLiteral("<a:endParaRPr sz=\"1800\"/></a:p>") + para(run(QStringLiteral("b")), pPr));
        QVERIFY(b.block(0).textList());
        QVERIFY(!b.block(1).textList());
        QCOMPARE(b.block(2).textList()->itemText(b.block(2)), QStringLiteral("2."));
    }

    void textAutonumberSchemesBecomeListStyles()
    {
        using S = QTextListFormat;
        struct Row { const char *type; S::Style style; const char *prefix; const char *suffix; int numberFormat; };
        const Row rows[] = {
            {"arabicPeriod", S::ListDecimal, "", ".", 1},      {"arabicParenR", S::ListDecimal, "", ")", 6},
            {"arabicParenBoth", S::ListDecimal, "(", ")", 7},  {"arabicPlain", S::ListDecimal, "", "", 0},
            {"alphaLcPeriod", S::ListLowerAlpha, "", ".", 2},  {"alphaLcParenR", S::ListLowerAlpha, "", ")", 0},
            {"alphaLcParenBoth", S::ListLowerAlpha, "(", ")", 0}, {"alphaUcPeriod", S::ListUpperAlpha, "", ".", 3},
            {"alphaUcParenR", S::ListUpperAlpha, "", ")", 0},  {"alphaUcParenBoth", S::ListUpperAlpha, "(", ")", 0},
            {"romanLcPeriod", S::ListLowerRoman, "", ".", 4},  {"romanLcParenR", S::ListLowerRoman, "", ")", 0},
            {"romanLcParenBoth", S::ListLowerRoman, "(", ")", 0}, {"romanUcPeriod", S::ListUpperRoman, "", ".", 5},
            {"romanUcParenR", S::ListUpperRoman, "", ")", 0},  {"romanUcParenBoth", S::ListUpperRoman, "(", ")", 0},
            // The ones of other scripts count in digits, with the same marks.
            {"circleNumDbPlain", S::ListDecimal, "", "", 0},   {"circleNumWdBlackPlain", S::ListDecimal, "", "", 0},
            {"circleNumWdWhitePlain", S::ListDecimal, "", "", 0}, {"arabicDbPeriod", S::ListDecimal, "", ".", 1},
            {"arabicDbPlain", S::ListDecimal, "", "", 0},      {"ea1ChsPeriod", S::ListDecimal, "", ".", 1},
            {"ea1ChsPlain", S::ListDecimal, "", "", 0},        {"ea1ChtPeriod", S::ListDecimal, "", ".", 1},
            {"ea1ChtPlain", S::ListDecimal, "", "", 0},        {"ea1JpnChsDbPeriod", S::ListDecimal, "", ".", 1},
            {"ea1JpnKorPlain", S::ListDecimal, "", "", 0},     {"ea1JpnKorPeriod", S::ListDecimal, "", ".", 1},
            {"arabic1Minus", S::ListDecimal, "", "", 0},       {"arabic2Minus", S::ListDecimal, "", "", 0},
            {"hebrew2Minus", S::ListDecimal, "", "", 0},       {"thaiAlphaPeriod", S::ListLowerAlpha, "", ".", 2},
            {"thaiAlphaParenR", S::ListLowerAlpha, "", ")", 0}, {"thaiAlphaParenBoth", S::ListLowerAlpha, "(", ")", 0},
            {"thaiNumPeriod", S::ListDecimal, "", ".", 1},     {"thaiNumParenR", S::ListDecimal, "", ")", 6},
            {"thaiNumParenBoth", S::ListDecimal, "(", ")", 7}, {"hindiAlphaPeriod", S::ListLowerAlpha, "", ".", 2},
            {"hindiAlpha1Period", S::ListLowerAlpha, "", ".", 2}, {"hindiNumPeriod", S::ListDecimal, "", ".", 1},
            {"hindiNumParenR", S::ListDecimal, "", ")", 6},
            // A scheme this edition doesn't know counts as the usual one.
            {"fromTheFuture", S::ListDecimal, "", ".", 1},
        };
        for (const Row &r : rows) {
            Body b(para(run(QStringLiteral("x")), QStringLiteral("<a:pPr><a:buAutoNum type=\"%1\"/></a:pPr>").arg(QLatin1String(r.type))));
            QTextList *l = b.block(0).textList();
            QVERIFY2(l, r.type);
            const QTextListFormat f = l->format();
            QVERIFY2(f.style() == r.style, r.type);
            QVERIFY2(f.numberPrefix() == QLatin1String(r.prefix), r.type);
            QVERIFY2(f.numberSuffix() == QLatin1String(r.suffix), r.type);
            QVERIFY2(f.intProperty(jp::tp::NumberFormat) == r.numberFormat, r.type);
            QVERIFY2(!jp::isBulletList(f.style()), r.type);
        }
    }

    void textAutonumberStartsAtItsNumber()
    {
        Body b(para(run(QStringLiteral("x")), QStringLiteral("<a:pPr><a:buAutoNum type=\"romanUcPeriod\" startAt=\"4\"/></a:pPr>"))
               + para(run(QStringLiteral("y")), QStringLiteral("<a:pPr><a:buAutoNum type=\"romanUcPeriod\" startAt=\"4\"/></a:pPr>")));
        QCOMPARE(b.block(0).textList()->itemText(b.block(0)), QStringLiteral("IV."));
        QCOMPARE(b.block(1).textList()->itemText(b.block(1)), QStringLiteral("V."));
    }

    void textParagraphsTakeAlignmentIndentsAndSpacing()
    {
        const QStringList layers = {QStringLiteral("<a:lvl1pPr marL=\"114300\" indent=\"-57150\" algn=\"just\"><a:spcBef><a:spcPts val=\"600\"/></a:spcBef></a:lvl1pPr>")};
        Body b(para(run(QStringLiteral("a"), QStringLiteral("sz=\"2000\"")), QStringLiteral("<a:pPr marR=\"127000\"/>"))
                   + para(run(QStringLiteral("b")), QStringLiteral("<a:pPr algn=\"ctr\" marL=\"0\" indent=\"254000\"><a:spcBef><a:spcPct val=\"50000\"/></a:spcBef>"
                                                                   "<a:spcAft><a:spcPts val=\"1250\"/></a:spcAft></a:pPr>"))
                   + para(run(QStringLiteral("c")), QStringLiteral("<a:pPr algn=\"r\"/>")) + para(run(QStringLiteral("d")), QStringLiteral("<a:pPr algn=\"l\"/>")),
               layers);
        QCOMPARE(b.blockFormat(0).leftMargin(), 9.0);
        QCOMPARE(b.blockFormat(0).textIndent(), -4.5);
        QCOMPARE(b.blockFormat(0).rightMargin(), 10.0);
        QCOMPARE(b.blockFormat(0).topMargin(), 6.0);
        QCOMPARE(b.blockFormat(0).bottomMargin(), 0.0);
        QCOMPARE(int(b.blockFormat(0).alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignJustify));
        QVERIFY(!b.blockFormat(0).boolProperty(jp::tp::Distribute));
        QCOMPARE(b.blockFormat(1).leftMargin(), 0.0);
        QCOMPARE(b.blockFormat(1).textIndent(), 20.0);
        // A percentage is of the line: 50% of one 18 pt line, 1.2 times the size.
        QCOMPARE(b.blockFormat(1).topMargin(), 0.5 * 1.2 * 18);
        QCOMPARE(b.blockFormat(1).bottomMargin(), 12.5);
        QCOMPARE(int(b.blockFormat(1).alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignHCenter));
        QCOMPARE(int(b.blockFormat(2).alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignRight));
        QCOMPARE(int(b.blockFormat(3).alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignLeft));
    }

    void textPercentSpacingIsOfTheParagraphsLargestRun()
    {
        Body b(para(run(QStringLiteral("a"), QStringLiteral("sz=\"1000\"")) + run(QStringLiteral("b"), QStringLiteral("sz=\"3000\"")),
                    QStringLiteral("<a:pPr><a:spcBef><a:spcPct val=\"100000\"/></a:spcBef><a:spcAft><a:spcPct val=\"50000\"/></a:spcAft></a:pPr>")));
        QCOMPARE(b.blockFormat().topMargin(), 1.2 * 30);
        QCOMPARE(b.blockFormat().bottomMargin(), 0.5 * 1.2 * 30);
    }

    void textDistributedAlignmentSpreadsTheLastLineToo()
    {
        Body b(para(run(QStringLiteral("a")), QStringLiteral("<a:pPr algn=\"dist\"/>")) + para(run(QStringLiteral("b")), QStringLiteral("<a:pPr algn=\"justLow\"/>")));
        QCOMPARE(int(b.blockFormat(0).alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignJustify));
        QVERIFY(b.blockFormat(0).boolProperty(jp::tp::Distribute));
        QCOMPARE(int(b.blockFormat(1).alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignJustify));
        QVERIFY(!b.blockFormat(1).boolProperty(jp::tp::Distribute));
    }

    void textLineSpacingIsProportionalOrExact()
    {
        Body b(para(run(QStringLiteral("a")), QStringLiteral("<a:pPr><a:lnSpc><a:spcPct val=\"90000\"/></a:lnSpc></a:pPr>"))
                   + para(run(QStringLiteral("b")), QStringLiteral("<a:pPr><a:lnSpc><a:spcPts val=\"2400\"/></a:lnSpc></a:pPr>"))
                   + para(run(QStringLiteral("c"))),
               {QStringLiteral("<a:lvl1pPr><a:lnSpc><a:spcPct val=\"120000\"/></a:lnSpc></a:lvl1pPr>")});
        QCOMPARE(b.blockFormat(0).lineHeightType(), int(QTextBlockFormat::ProportionalHeight));
        QCOMPARE(b.blockFormat(0).lineHeight(), 90.0);
        QCOMPARE(b.blockFormat(1).lineHeightType(), int(QTextBlockFormat::FixedHeight));
        QCOMPARE(b.blockFormat(1).lineHeight(), 24.0);
        QCOMPARE(b.blockFormat(2).lineHeightType(), int(QTextBlockFormat::ProportionalHeight));
        QCOMPARE(b.blockFormat(2).lineHeight(), 120.0);
    }

    void textTabStopsAreReadWithTheirAlignment()
    {
        Body b(para(run(QStringLiteral("a\tb")),
                    QStringLiteral("<a:pPr><a:tabLst><a:tab pos=\"914400\" algn=\"l\"/><a:tab pos=\"1828800\" algn=\"r\"/><a:tab pos=\"2743200\" algn=\"ctr\"/>"
                                   "<a:tab pos=\"3657600\" algn=\"dec\"/><a:tab algn=\"l\"/></a:tabLst></a:pPr>")));
        const QList<QTextOption::Tab> tabs = b.blockFormat().tabPositions();
        QCOMPARE(tabs.size(), 4);
        QCOMPARE(tabs[0].position, 72.0);
        QCOMPARE(int(tabs[0].type), int(QTextOption::LeftTab));
        QCOMPARE(tabs[1].position, 144.0);
        QCOMPARE(int(tabs[1].type), int(QTextOption::RightTab));
        QCOMPARE(tabs[2].position, 216.0);
        QCOMPARE(int(tabs[2].type), int(QTextOption::CenterTab));
        QCOMPARE(tabs[3].position, 288.0);
        QCOMPARE(int(tabs[3].type), int(QTextOption::DelimiterTab));
        QCOMPARE(tabs[3].delimiter, QChar('.'));
    }

    void textDefaultTabSizeMakesStopsOnlyWhereATabIsUsed()
    {
        // 36 pt apart, from the last stop of its own on.
        Body b(para(run(QStringLiteral("a\tb")), QStringLiteral("<a:pPr defTabSz=\"457200\"/>"))
                   + para(run(QStringLiteral("no tab here")), QStringLiteral("<a:pPr defTabSz=\"457200\"/>"))
                   + para(run(QStringLiteral("a\tb")), QStringLiteral("<a:pPr defTabSz=\"457200\"><a:tabLst><a:tab pos=\"1000000\" algn=\"l\"/></a:tabLst></a:pPr>")));
        const QList<QTextOption::Tab> first = b.blockFormat(0).tabPositions();
        QVERIFY(first.size() >= 3);
        QCOMPARE(first[0].position, 36.0);
        QCOMPARE(first[1].position, 72.0);
        QCOMPARE(first[2].position, 108.0);
        QVERIFY(b.blockFormat(1).tabPositions().isEmpty());
        const QList<QTextOption::Tab> third = b.blockFormat(2).tabPositions();
        QVERIFY(third.size() >= 2);
        QCOMPARE(third[0].position, 1000000 / 12700.0);
        QVERIFY(third[1].position > third[0].position);
        QCOMPARE(std::fmod(third[1].position, 36.0), 0.0);
    }

    void textRightToLeftParagraphsKeepTheirSides()
    {
        Body b(para(run(QStringLiteral("a")), QStringLiteral("<a:pPr rtl=\"1\" algn=\"r\"/>")) + para(run(QStringLiteral("b")), QStringLiteral("<a:pPr rtl=\"1\" algn=\"l\"/>"))
               + para(run(QStringLiteral("c")), QStringLiteral("<a:pPr rtl=\"0\" algn=\"r\"/>")));
        QCOMPARE(b.blockFormat(0).layoutDirection(), Qt::RightToLeft);
        // The standard's left and right are the page's: the engine swaps a
        // right-to-left paragraph's, unless told they are absolute.
        QVERIFY(bool(b.blockFormat(0).alignment() & Qt::AlignRight));
        QVERIFY(bool(b.blockFormat(0).alignment() & Qt::AlignAbsolute));
        QVERIFY(bool(b.blockFormat(1).alignment() & Qt::AlignLeft));
        QVERIFY(bool(b.blockFormat(1).alignment() & Qt::AlignAbsolute));
        QVERIFY(b.blockFormat(2).layoutDirection() != Qt::RightToLeft);
        QVERIFY(bool(b.blockFormat(2).alignment() & Qt::AlignRight));
    }

    void textUnderlinesBecomeStylesAndKinds()
    {
        using U = QTextCharFormat;
        struct Row { const char *u; U::UnderlineStyle style; int kind; };
        const Row rows[] = {
            {"sng", U::SingleUnderline, 0},       {"words", U::SingleUnderline, 2},      {"dbl", U::SingleUnderline, 1},
            {"heavy", U::SingleUnderline, 4},     {"dotted", U::DotLine, 0},             {"dottedHeavy", U::DotLine, 4},
            {"dash", U::DashUnderline, 0},        {"dashHeavy", U::DashUnderline, 4},    {"dashLong", U::DashUnderline, 0},
            {"dashLongHeavy", U::DashUnderline, 4}, {"dotDash", U::DashDotLine, 0},      {"dotDashHeavy", U::DashDotLine, 4},
            {"dotDotDash", U::DashDotDotLine, 0}, {"dotDotDashHeavy", U::DashDotDotLine, 4}, {"wavy", U::WaveUnderline, 0},
            {"wavyHeavy", U::WaveUnderline, 4},   {"wavyDbl", U::WaveUnderline, 1},      {"none", U::NoUnderline, 0},
        };
        for (const Row &r : rows) {
            Body b(para(run(QStringLiteral("x"), QStringLiteral("u=\"%1\"").arg(QLatin1String(r.u)))));
            const QTextCharFormat f = b.charAt();
            QVERIFY2(f.underlineStyle() == r.style, r.u);
            QVERIFY2(f.intProperty(jp::tp::UnderlineKind) == r.kind, r.u);
        }
        // A run turns an underline from its layer off, and the layer's reaches a run that says nothing.
        Body b(para(run(QStringLiteral("a")) + run(QStringLiteral("b"), QStringLiteral("u=\"none\""))), {QStringLiteral("<a:lvl1pPr><a:defRPr u=\"sng\"/></a:lvl1pPr>")});
        QCOMPARE(b.charAt(0, 0).underlineStyle(), QTextCharFormat::SingleUnderline);
        QCOMPARE(b.charAt(0, 1).underlineStyle(), QTextCharFormat::NoUnderline);
    }

    void textStrikeBaselineSpacingCapsAndKerning()
    {
        Body b(para(run(QStringLiteral("a"), QStringLiteral("strike=\"sngStrike\"")) + run(QStringLiteral("b"), QStringLiteral("strike=\"dblStrike\""))
                    + run(QStringLiteral("c"), QStringLiteral("strike=\"noStrike\"")) + run(QStringLiteral("d"), QStringLiteral("baseline=\"30000\""))
                    + run(QStringLiteral("e"), QStringLiteral("baseline=\"-25000\"")) + run(QStringLiteral("f"), QStringLiteral("baseline=\"0\""))
                    + run(QStringLiteral("g"), QStringLiteral("spc=\"300\"")) + run(QStringLiteral("h"), QStringLiteral("spc=\"-50\""))
                    + run(QStringLiteral("i"), QStringLiteral("cap=\"all\"")) + run(QStringLiteral("j"), QStringLiteral("cap=\"small\""))
                    + run(QStringLiteral("k"), QStringLiteral("cap=\"none\"")) + run(QStringLiteral("l"), QStringLiteral("kern=\"1200\""))
                    + run(QStringLiteral("m"), QStringLiteral("kern=\"0\"")) + run(QStringLiteral("n"), QStringLiteral("lang=\"fr-CA\" noProof=\"1\""))),
               {QStringLiteral("<a:lvl1pPr><a:defRPr cap=\"all\"/></a:lvl1pPr>")});
        QCOMPARE(int(b.charAt(0, 0).fontCapitalization()), int(QFont::AllUppercase));   // from the layer
        QVERIFY(b.charAt(0, 0).fontStrikeOut());
        QVERIFY(b.charAt(0, 1).fontStrikeOut());
        QVERIFY(!b.charAt(0, 2).fontStrikeOut());
        QCOMPARE(b.charAt(0, 3).verticalAlignment(), QTextCharFormat::AlignSuperScript);
        QCOMPARE(b.charAt(0, 4).verticalAlignment(), QTextCharFormat::AlignSubScript);
        QCOMPARE(b.charAt(0, 5).verticalAlignment(), QTextCharFormat::AlignNormal);
        // Spacing is in points after each letter, as the engine's kerning is.
        QCOMPARE(b.charAt(0, 6).fontLetterSpacingType(), QFont::AbsoluteSpacing);
        QCOMPARE(b.charAt(0, 6).fontLetterSpacing(), 3.0);
        QCOMPARE(b.charAt(0, 7).fontLetterSpacing(), -0.5);
        QVERIFY(!b.charAt(0, 6).hasProperty(jp::tp::Tracking));
        QCOMPARE(int(b.charAt(0, 8).fontCapitalization()), int(QFont::AllUppercase));
        QCOMPARE(int(b.charAt(0, 9).fontCapitalization()), int(QFont::SmallCaps));
        QCOMPARE(int(b.charAt(0, 10).fontCapitalization()), int(QFont::MixedCase));
        QCOMPARE(b.charAt(0, 11).property(jp::tp::KernAbove).toDouble(), 12.0);
        QVERIFY(b.charAt(0, 12).hasProperty(jp::tp::KernAbove));
        QCOMPARE(b.charAt(0, 12).property(jp::tp::KernAbove).toDouble(), 0.0);
        QVERIFY(!b.charAt(0, 0).hasProperty(jp::tp::KernAbove));
        QCOMPARE(b.charAt(0, 13).stringProperty(jp::tp::Language), QStringLiteral("fr-CA"));
        QVERIFY(b.charAt(0, 13).boolProperty(jp::tp::NoProof));
        QVERIFY(b.charAt(0, 0).stringProperty(jp::tp::Language).isEmpty());
    }

    void textHighlightAndFillKinds()
    {
        Body b(para(run(QStringLiteral("a"), QString(), QStringLiteral("<a:highlight><a:srgbClr val=\"FFFF00\"/></a:highlight>"))
                    + run(QStringLiteral("b"), QString(), QStringLiteral("<a:solidFill><a:schemeClr val=\"tx1\"><a:alpha val=\"50000\"/></a:schemeClr></a:solidFill>"))
                    + run(QStringLiteral("c"), QString(), QStringLiteral("<a:gradFill><a:gsLst><a:gs pos=\"0\"><a:srgbClr val=\"123456\"/></a:gs>"
                                                                         "<a:gs pos=\"100000\"><a:srgbClr val=\"FFFFFF\"/></a:gs></a:gsLst></a:gradFill>"))));
        QCOMPARE(b.charAt(0, 0).stringProperty(jp::tp::HighlightRefP), QStringLiteral("#FFFF00"));
        QVERIFY(b.charAt(0, 1).stringProperty(jp::tp::HighlightRefP).isEmpty());
        QVERIFY(qAbs(b.charAt(0, 1).foreground().color().alpha() - 128) <= 1);
        QVERIFY(qAbs(jp::ColorRef::fromString(b.charAt(0, 1).stringProperty(jp::tp::ColorRefP)).rgbValue().alpha() - 128) <= 1);
        QCOMPARE(b.charAt(0, 2).foreground().color(), QColor(0x12, 0x34, 0x56));
    }

    void textFontsAreThemeReferencesOrFamilies()
    {
        Theme theme;
        QVERIFY(readTheme(handWrittenTheme(), &theme));
        auto latin = [](const QString &typeface, const QString &more = QString()) {
            return QStringLiteral("<a:latin typeface=\"%1\"/>%2").arg(typeface, more);
        };
        Body b(para(run(QStringLiteral("a"), QString(), latin(QStringLiteral("+mj-lt")))
                    + run(QStringLiteral("b"), QString(), latin(QStringLiteral("+mn-lt")))
                    + run(QStringLiteral("c"), QString(), latin(QStringLiteral("Arial")))
                    + run(QStringLiteral("d"), QString(), latin(QStringLiteral("Arial"), QStringLiteral("<a:ea typeface=\"+mn-ea\"/><a:cs typeface=\"Tahoma\"/>")))
                    + run(QStringLiteral("e"), QString(), QStringLiteral("<a:ea typeface=\"+mj-ea\"/>"))
                    + run(QStringLiteral("f"), QString(), QStringLiteral("<a:cs typeface=\"+mn-cs\"/>"))
                    + run(QStringLiteral("g"), QString(), QStringLiteral("<a:latin typeface=\"\"/>"))
                    + run(QStringLiteral("h"))),
               {QStringLiteral("<a:lvl1pPr><a:defRPr><a:latin typeface=\"Calibri\"/></a:defRPr></a:lvl1pPr>")}, QString(), nullptr, theme);
        auto families = [&](int i) { return b.charAt(0, i).fontFamilies().toStringList(); };
        // A theme font is no family: the engine takes the story's font scheme.
        QCOMPARE(b.charAt(0, 0).stringProperty(jp::tp::ThemeFont), QStringLiteral("major"));
        QVERIFY(!b.charAt(0, 0).hasProperty(QTextFormat::FontFamilies));
        QCOMPARE(b.charAt(0, 1).stringProperty(jp::tp::ThemeFont), QStringLiteral("minor"));
        QVERIFY(!b.charAt(0, 1).hasProperty(QTextFormat::FontFamilies));
        QCOMPARE(families(2), QStringList({QStringLiteral("Arial")}));
        QVERIFY(b.charAt(0, 2).stringProperty(jp::tp::ThemeFont).isEmpty());
        // The East Asian and complex-script fonts follow as fallbacks when they are families.
        QCOMPARE(families(3), QStringList({QStringLiteral("Arial"), QStringLiteral("Tahoma")}));
        // With no Latin font of its own anywhere above a run takes its layer's Calibri.
        QCOMPARE(families(4), QStringList({QStringLiteral("Calibri")}));
        QCOMPARE(families(5), QStringList({QStringLiteral("Calibri")}));
        QCOMPARE(families(6), QStringList({QStringLiteral("Calibri")}));
        QCOMPARE(families(7), QStringList({QStringLiteral("Calibri")}));
        // And the other way round: a theme font in a run replaces the layer's family.
        QVERIFY(!b.charAt(0, 0).fontFamilies().toStringList().contains(QStringLiteral("Calibri")));
    }

    void textThemeFontsOfEastAsianAndComplexScriptsSetTheThemeFont()
    {
        Body b(para(run(QStringLiteral("a"), QString(), QStringLiteral("<a:ea typeface=\"+mj-ea\"/>")) + run(QStringLiteral("b"), QString(), QStringLiteral("<a:cs typeface=\"+mn-cs\"/>"))
                    + run(QStringLiteral("c"), QString(), QStringLiteral("<a:ea typeface=\"SimSun\"/>"))));
        QCOMPARE(b.charAt(0, 0).stringProperty(jp::tp::ThemeFont), QStringLiteral("major"));
        QCOMPARE(b.charAt(0, 1).stringProperty(jp::tp::ThemeFont), QStringLiteral("minor"));
        // A family for the East Asian script alone is not the Latin text's font.
        QVERIFY(!b.charAt(0, 2).hasProperty(QTextFormat::FontFamilies));
    }

    void textSlideNumberIsThePageField()
    {
        Body b(QStringLiteral("<a:p><a:r><a:t>Slide </a:t></a:r><a:fld id=\"{B6F15528-21DE-4FAA-801E-634DDDAF4B2B}\" type=\"slidenum\"><a:rPr sz=\"1200\" b=\"1\"/>"
                              "<a:t>&#8249;#&#8250;</a:t></a:fld></a:p>"));
        QCOMPARE(b.text(), QStringLiteral("Slide ") + QString(QChar::ObjectReplacementCharacter));
        QCOMPARE(b.charAt(0, 6).stringProperty(jp::tp::Field), QStringLiteral("page"));
        QCOMPARE(b.charAt(0, 6).fontPointSize(), 12.0);
        QCOMPARE(b.charAt(0, 6).fontWeight(), int(QFont::Bold));
        QVERIFY(b.charAt(0, 0).stringProperty(jp::tp::Field).isEmpty());
        // Laid out on slide 7, it reads 7.
        jp::FrameSpec fs;
        fs.size = QSizeF(400, 100);
        fs.insets = QMarginsF(0, 0, 0, 0);
        fs.ctx.pageNumber = 7;
        jp::StoryLayout lay;
        lay.build(&b.doc, {fs}, jp::LayoutEnv());
        const auto lines = lay.lineInfo(0);
        QCOMPARE(lines.size(), 1);
        QCOMPARE(lines[0].text, QStringLiteral("Slide 7"));
    }

    void textDateFieldsHaveTheirFormats()
    {
        // The names of days and months are the locale's.
        struct Restore {
            QLocale was;
            ~Restore() { QLocale::setDefault(was); }
        } restore;
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
        struct Row { const char *type; const char *shown; };
        const Row rows[] = {
            {"datetime", "10/11/2026"},                     {"datetime1", "10/11/2026"},
            {"datetime2", "Sunday, October 11, 2026"},      {"datetime3", "11 October 2026"},
            {"datetime4", "October 11, 2026"},              {"datetime5", "11-Oct-26"},
            {"datetime6", "October 26"},                    {"datetime7", "Oct-26"},
            {"datetime8", "10/11/2026 4:07 PM"},            {"datetime9", "10/11/2026 4:07:09 PM"},
            {"datetime10", "16:07"},                        {"datetime11", "16:07:09"},
            {"datetime12", "4:07 PM"},                      {"datetime13", "4:07:09 PM"},
        };
        jp::FieldContext now;
        now.now = QDateTime(QDate(2026, 10, 11), QTime(16, 7, 9));
        for (const Row &r : rows) {
            Body b(QStringLiteral("<a:p><a:fld id=\"{00000000-0000-0000-0000-000000000000}\" type=\"%1\"><a:rPr/><a:t>whatever it said</a:t></a:fld></a:p>").arg(QLatin1String(r.type)));
            const QString code = b.charAt().stringProperty(jp::tp::Field);
            QVERIFY2(!code.isEmpty(), r.type);
            QVERIFY2(now.resolve(code) == QLatin1String(r.shown), qPrintable(QStringLiteral("%1: %2 (%3)").arg(QLatin1String(r.type), now.resolve(code), code)));
            QCOMPARE(b.text(), QString(QChar::ObjectReplacementCharacter));
        }
    }

    void textOtherFieldsShowWhatTheyHeld()
    {
        Body b(QStringLiteral("<a:p><a:r><a:t>[</a:t></a:r><a:fld id=\"{1}\" type=\"datetimeFigureOut\"><a:rPr/><a:t>Tuesday</a:t></a:fld>"
                              "<a:fld id=\"{2}\" type=\"somethingNew\"><a:t>cached</a:t></a:fld><a:fld id=\"{3}\"/><a:r><a:t>]</a:t></a:r></a:p>"));
        QCOMPARE(b.text(), QStringLiteral("[Tuesdaycached]"));
        for (int i = 0; i < b.doc.characterCount() - 1; ++i) QVERIFY(b.charAt(0, i).stringProperty(jp::tp::Field).isEmpty());
    }

    void textHyperlinksAreAnchorsToTheirExternalTargets()
    {
        PackageWriter w;
        w.addPart(QStringLiteral("/ppt/slides/slide1.xml"), "<p:sld/>", kSlideType);
        w.addPart(QStringLiteral("/ppt/slides/slide2.xml"), "<p:sld/>", kSlideType);
        const QString slide = QStringLiteral("/ppt/slides/slide1.xml");
        QCOMPARE(w.addRelationship(slide, kRelBase + "hyperlink", QStringLiteral("https://example.com/a?b=1&c=2"), true), QStringLiteral("rId1"));
        QCOMPARE(w.addRelationship(slide, kRelBase + "slide", QStringLiteral("/ppt/slides/slide2.xml")), QStringLiteral("rId2"));
        Package p;
        QVERIFY(p.read(w.finish()));
        auto link = [](const QString &id, const QString &extra = QString()) {
            return QStringLiteral("<a:hlinkClick r:id=\"%1\" %2/>").arg(id, extra);
        };
        Body b(para(run(QStringLiteral("a"), QString(), solid(QStringLiteral("FF0000")) + link(QStringLiteral("rId1")))
                    + run(QStringLiteral("b"), QString(), solid(QStringLiteral("FF0000")) + link(QStringLiteral("rId2")))
                    + run(QStringLiteral("c"), QString(), link(QStringLiteral("rId9")))
                    + run(QStringLiteral("d"), QString(), link(QString()))
                    + run(QStringLiteral("e"))),
               {}, QString(), &p);
        const QTextCharFormat a = b.charAt(0, 0);
        QVERIFY(a.isAnchor());
        QCOMPARE(a.anchorHref(), QStringLiteral("https://example.com/a?b=1&c=2"));
        // The presentation program shows a link in the theme's link color, underlined.
        QCOMPARE(a.foreground().color(), QColor(0x05, 0x63, 0xC1));
        QCOMPARE(a.stringProperty(jp::tp::ColorRefP), QStringLiteral("#0563C1"));
        QCOMPARE(a.underlineStyle(), QTextCharFormat::SingleUnderline);
        // A jump to a slide has no address; a link that names nothing is no link.
        for (int i = 1; i <= 3; ++i) QVERIFY(!b.charAt(0, i).isAnchor());
        QVERIFY(!b.charAt(0, 4).isAnchor());
        QCOMPARE(b.charAt(0, 1).foreground().color(), QColor(0xFF, 0, 0));
        // Without a package there is nothing to look the id up in.
        Body none(para(run(QStringLiteral("a"), QString(), link(QStringLiteral("rId1")))));
        QVERIFY(!none.charAt().isAnchor());
    }

    void textHyperlinkMayKeepTheTextColor()
    {
        PackageWriter w;
        w.addPart(QStringLiteral("/ppt/slides/slide1.xml"), "<p:sld/>", kSlideType);
        w.addRelationship(QStringLiteral("/ppt/slides/slide1.xml"), kRelBase + "hyperlink", QStringLiteral("mailto:a@example.com"), true);
        Package p;
        QVERIFY(p.read(w.finish()));
        const QString keep = QStringLiteral("<a:hlinkClick r:id=\"rId1\"><a:extLst><a:ext uri=\"{A12FA001-AC4F-418D-AE19-62706E023703}\">"
                                            "<ahyp:hlinkClr xmlns:ahyp=\"urn:example:hyperlink-color\" val=\"tx\"/></a:ext></a:extLst></a:hlinkClick>");
        Body b(para(run(QStringLiteral("a"), QString(), solid(QStringLiteral("FF0000")) + keep)), {}, QString(), &p);
        QVERIFY(b.charAt().isAnchor());
        QCOMPARE(b.charAt().foreground().color(), QColor(0xFF, 0, 0));
    }

    void textEmptyParagraphKeepsItsEndSize()
    {
        Body b(QStringLiteral("<a:p><a:endParaRPr sz=\"4400\"/></a:p><a:p><a:endParaRPr sz=\"1200\"/></a:p><a:p><a:r><a:t>x</a:t></a:r></a:p>"));
        QCOMPARE(b.block(0).charFormat().fontPointSize(), 44.0);
        QCOMPARE(b.block(1).charFormat().fontPointSize(), 12.0);
        QVERIFY(b.block(0).text().isEmpty());
        // Laid out, the empty line is as tall as its size says.
        jp::FrameSpec fs;
        fs.size = QSizeF(300, 400);
        fs.insets = QMarginsF(0, 0, 0, 0);
        jp::StoryLayout lay;
        lay.build(&b.doc, {fs}, jp::LayoutEnv());
        const auto lines = lay.lineInfo(0);
        QCOMPARE(lines.size(), 3);
        QVERIFY2(lines[0].rect.height() > 3 * lines[1].rect.height(), qPrintable(QStringLiteral("%1 and %2").arg(lines[0].rect.height()).arg(lines[1].rect.height())));
        QVERIFY(lines[0].rect.height() >= 44);
    }

    void textEndParagraphSizeInheritsToo()
    {
        Body b(QStringLiteral("<a:p/><a:p><a:endParaRPr b=\"1\"/></a:p>"), {QStringLiteral("<a:lvl1pPr><a:defRPr sz=\"3600\"/></a:lvl1pPr>")});
        QCOMPARE(b.block(0).charFormat().fontPointSize(), 36.0);
        QCOMPARE(b.block(1).charFormat().fontPointSize(), 36.0);
        QCOMPARE(b.block(1).charFormat().fontWeight(), int(QFont::Bold));
    }

    void textLineBreaksAndTabsStayInTheirParagraph()
    {
        Body b(QStringLiteral("<a:p><a:r><a:t>a</a:t></a:r><a:br><a:rPr sz=\"2000\"/></a:br><a:r><a:t>b c</a:t></a:r><a:br/><a:r><a:t>x_x000B_y&#9;z_x0009_w</a:t></a:r></a:p>"));
        QCOMPARE(b.doc.blockCount(), 1);
        const QChar sep(QChar::LineSeparator);
        QCOMPARE(b.text(), QStringLiteral("a") + sep + QStringLiteral("b c") + sep + QStringLiteral("x") + sep + QStringLiteral("y\tz\tw"));
        QCOMPARE(b.charAt(0, 1).fontPointSize(), 20.0);   // the break has its own size
        // A line feed inside a run is a line break too, never a new paragraph.
        Body lf(QStringLiteral("<a:p><a:r><a:t>one&#10;two&#13;three</a:t></a:r></a:p>"));
        QCOMPARE(lf.doc.blockCount(), 1);
        QVERIFY(lf.text().contains(sep));
        QVERIFY(!lf.text().contains(QLatin1Char('\n')));
    }

    void textBreaksLinesWhereTheSizeSaysItMust()
    {
        // The same two paragraphs at two sizes: bigger letters, more lines.
        auto lines = [](int sz) {
            const QString words = QStringLiteral("The quick brown fox jumps over the lazy dog and then runs back to the barn");
            Body b(para(run(words, QStringLiteral("sz=\"%1\"").arg(sz))) + para(run(QStringLiteral("Second paragraph with a few more words in it"), QStringLiteral("sz=\"%1\"").arg(sz))));
            jp::FrameSpec fs;
            fs.size = QSizeF(240, 2000);
            fs.insets = QMarginsF(0, 0, 0, 0);
            jp::StoryLayout lay;
            lay.build(&b.doc, {fs}, jp::LayoutEnv());
            return int(lay.lineInfo(0).size());
        };
        const int fewer = lines(1200), more = lines(3600);
        QVERIFY2(more > fewer, qPrintable(QStringLiteral("%1 lines at 12 pt, %2 at 36 pt").arg(fewer).arg(more)));
        QVERIFY(fewer >= 2);
    }

    void textBodyHostileInputIsClamped()
    {
        // Sizes within 1 to 4000 pt; nonsense is ignored.
        Body b(para(run(QStringLiteral("a"), QStringLiteral("sz=\"1\"")) + run(QStringLiteral("b"), QStringLiteral("sz=\"99999999\""))
                    + run(QStringLiteral("c"), QStringLiteral("sz=\"0\"")) + run(QStringLiteral("d"), QStringLiteral("sz=\"-300\""))
                    + run(QStringLiteral("e"), QStringLiteral("sz=\"abc\" spc=\"x\" baseline=\"y\" kern=\"z\"")) + run(QStringLiteral("f"), QStringLiteral("spc=\"99999999\""))));
        QCOMPARE(b.charAt(0, 0).fontPointSize(), 1.0);
        QCOMPARE(b.charAt(0, 1).fontPointSize(), 4000.0);
        QCOMPARE(b.charAt(0, 2).fontPointSize(), 1.0);
        QCOMPARE(b.charAt(0, 3).fontPointSize(), 1.0);
        QCOMPARE(b.charAt(0, 4).fontPointSize(), 18.0);
        QVERIFY(qAbs(b.charAt(0, 5).fontLetterSpacing()) <= 1584.0);
    }

    void textBodyDamagedElementsAreSurvived()
    {
        const QString junk = QStringLiteral(
            "<a:p><a:pPr lvl=\"99\" marL=\"abc\" indent=\"\" algn=\"sideways\" defTabSz=\"x\" rtl=\"maybe\"><a:lnSpc/><a:spcBef><a:spcPct/></a:spcBef><a:spcAft><a:spcPts val=\"-5\"/></a:spcAft>"
            "<a:buClr/><a:buSzPct/><a:buSzPts val=\"q\"/><a:buFont/><a:buChar/><a:buAutoNum/><a:buAutoNum type=\"arabicPeriod\" startAt=\"-4\"/>"
            "<a:tabLst><a:tab/><a:tab pos=\"x\"/></a:tabLst><a:defRPr sz=\"abc\"/></a:pPr>"
            "<a:r/><a:r><a:rPr><a:hlinkClick/><a:solidFill/><a:highlight/><a:latin/><a:ea/><a:cs/><a:gradFill/></a:rPr></a:r><a:fld/><a:br/><a:endParaRPr/><a:r><a:t/></a:r></a:p>"
            "<a:p><a:pPr lvl=\"-3\"/></a:p><a:p/><a:p><a:r><a:rPr u=\"bogus\" strike=\"bogus\" cap=\"bogus\" b=\"bogus\"/><a:t>t</a:t></a:r></a:p>");
        Body b(junk, {QStringLiteral("<a:lvl1pPr><a:defRPr/></a:lvl1pPr><a:lvl9pPr/><a:defPPr/>"), QString()});
        QCOMPARE(b.doc.blockCount(), 4);
        QCOMPARE(b.block(0).blockFormat().intProperty(jp::tp::ListLevel), 8);
        QCOMPARE(b.block(1).blockFormat().intProperty(jp::tp::ListLevel), 0);
        // And a body of the wrong kind of element.
        QDomDocument dom;
        QVERIFY(parseXml("<a:txBody xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\"><a:p><a:p><a:r><a:r/></a:r></a:p></a:p><a:t>stray</a:t></a:txBody>", &dom));
        QTextDocument doc;
        readTextBody(dom.documentElement(), &doc, DrawingContext());
        QVERIFY(doc.blockCount() >= 1);
        // And a stack with nothing in some of its layers.
        readTextBody(dom.documentElement(), &doc, DrawingContext(), TextStyleStack{QDomElement(), dom.documentElement(), QDomElement()});
        QVERIFY(doc.blockCount() >= 1);
    }

    void textBodySurvivesRandomDamage()
    {
        const QString layer = QStringLiteral(
            "<a:lstStyle %1><a:defPPr><a:defRPr sz=\"1800\"/></a:defPPr><a:lvl1pPr marL=\"342900\" indent=\"-342900\" algn=\"ctr\" defTabSz=\"914400\" rtl=\"0\">"
            "<a:lnSpc><a:spcPct val=\"90000\"/></a:lnSpc><a:spcBef><a:spcPts val=\"1000\"/></a:spcBef><a:buClr><a:srgbClr val=\"FF0000\"/></a:buClr>"
            "<a:buSzPct val=\"80000\"/><a:buFont typeface=\"Arial\"/><a:buChar char=\"&#8226;\"/><a:tabLst><a:tab pos=\"914400\" algn=\"r\"/></a:tabLst>"
            "<a:defRPr sz=\"2800\" kern=\"1200\" b=\"1\"><a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill><a:latin typeface=\"+mn-lt\"/></a:defRPr></a:lvl1pPr>"
            "</a:lstStyle>").arg(declarations());
        const QString body = QStringLiteral(
            "<p:txBody %1><a:bodyPr/><a:lstStyle/>"
            "<a:p><a:pPr lvl=\"0\" marL=\"457200\"><a:buAutoNum type=\"arabicPeriod\" startAt=\"2\"/></a:pPr>"
            "<a:r><a:rPr lang=\"en-US\" sz=\"2000\" b=\"1\" i=\"1\" u=\"wavyHeavy\" strike=\"sngStrike\" baseline=\"30000\" spc=\"-50\" cap=\"small\">"
            "<a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill><a:highlight><a:srgbClr val=\"FFFF00\"/></a:highlight><a:latin typeface=\"Arial\"/>"
            "<a:hlinkClick r:id=\"rId1\"/></a:rPr><a:t>Some text</a:t></a:r><a:br><a:rPr sz=\"1200\"/></a:br>"
            "<a:fld id=\"{1}\" type=\"slidenum\"><a:rPr/><a:t>1</a:t></a:fld><a:fld id=\"{2}\" type=\"datetime9\"><a:t>now</a:t></a:fld>"
            "<a:endParaRPr sz=\"900\"/></a:p>"
            "<a:p><a:pPr lvl=\"1\" algn=\"dist\" rtl=\"1\"><a:lnSpc><a:spcPts val=\"2400\"/></a:lnSpc><a:spcAft><a:spcPct val=\"50000\"/></a:spcAft><a:buNone/></a:pPr>"
            "<a:r><a:t>tab\there</a:t></a:r></a:p></p:txBody>").arg(declarations());
        PackageWriter w;
        w.addPart(QStringLiteral("/ppt/slides/slide1.xml"), "<p:sld/>", kSlideType);
        w.addRelationship(QStringLiteral("/ppt/slides/slide1.xml"), kRelBase + "hyperlink", QStringLiteral("https://example.com/"), true);
        Package package;
        QVERIFY(package.read(w.finish()));
        DrawingContext ctx;
        ctx.package = &package;
        ctx.part = QStringLiteral("/ppt/slides/slide1.xml");
        ctx.theme = colorTestTheme();
        QRandomGenerator gen(4711);
        int read = 0;
        for (int i = 0; i < 600; ++i) {
            QByteArray badBody = body.toUtf8(), badLayer = layer.toUtf8();
            for (QByteArray *bad : {&badBody, &badLayer})
                if (gen.bounded(3) != 0)
                    for (int k = 0, n = 1 + int(gen.bounded(4)); k < n; ++k) (*bad)[int(gen.bounded(quint32(bad->size())))] = char(gen.bounded(32u, 127u));
            QDomDocument bodyDom, layerDom;
            if (!parseXml(badBody, &bodyDom) || !parseXml(badLayer, &layerDom)) continue;
            ++read;
            QTextDocument doc;
            readTextBody(bodyDom.documentElement(), &doc, ctx, TextStyleStack{layerDom.documentElement()});
            QVERIFY(doc.blockCount() >= 1);
            if (i % 12 == 0) {
                // What was read goes through the text engine too.
                jp::FrameSpec fs;
                fs.size = QSizeF(300, 300);
                jp::StoryLayout lay;
                lay.build(&doc, {fs}, jp::LayoutEnv());
            }
        }
        QVERIFY(read > 100);
    }

    void textBodyStopsAtTenThousandParagraphsAndTwentyThousandRuns()
    {
        {
            QString many;
            many.reserve(10001 * 6);
            for (int i = 0; i < 10001; ++i) many += QStringLiteral("<a:p/>");
            Body b(many);
            QCOMPARE(b.doc.blockCount(), 10000);
        }
        {
            QString runs = QStringLiteral("<a:p>");
            runs.reserve(20001 * 20);
            for (int i = 0; i < 20001; ++i) runs += QStringLiteral("<a:r><a:t>x</a:t></a:r>");
            runs += QStringLiteral("</a:p>");
            Body b(runs);
            QCOMPARE(b.doc.blockCount(), 1);
            QCOMPARE(b.text().size(), 20000);
        }
    }
};

QTEST_MAIN(FormatTests)
#include "tst_office.moc"
