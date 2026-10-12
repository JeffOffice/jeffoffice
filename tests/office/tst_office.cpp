// Tests of libs/office: packages, the theme, and DrawingML colors. Every
// package they read is built in memory (never a real-world file).
#include "office/drawingml.h"
#include "office/drawingml_color.h"
#include "office/package.h"
#include "office/theme.h"
#include "office/xml.h"
#include "io/zip.h"
#include "text/textengine.h"
#include "text/textprops.h"

#include <QFile>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextList>
#include <QtEndian>
#include <QtTest>

#include <cmath>
#include <memory>
#include <vector>

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
        QVERIFY(fewer >= 3);
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
};

QTEST_MAIN(FormatTests)
#include "tst_office.moc"
