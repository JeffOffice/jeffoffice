// Tests of libs/office: packages, the theme, and DrawingML colors. Every
// package they read is built in memory (never a real-world file).
#include "office/drawingml_color.h"
#include "office/odraw.h"
#include "office/package.h"
#include "office/theme.h"
#include "office/xml.h"
#include "io/zip.h"
#include "render/metafile.h"

#include <QFile>
#include <QImage>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QtEndian>
#include <QtTest>

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
            QVector<OdProp> props = {{0x01BF, 0x00100010}};
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
        f = fillOf({{0x0180, 7}, red, blue, {0x0196, 0, odArray(3, 8, list)}});
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
            odShape(3, 0xA00, {{0x0180, 7}, {0x0196, 0, odArray(2, 8, list)}, {0x01D0, 1}}, at) +
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
        QVERIFY(tryShape(odShape(3, 0xA00, {{0x0180, 7}, {0x0196, 0, odArray(65535, 8, QByteArray(24, '\x03'))}}, at)).size() == 1);
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
