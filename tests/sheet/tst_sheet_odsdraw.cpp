// Tests of the drawing objects of a spreadsheet in the OpenDocument form (libs/office/sheet/odsdraw.h): each kind of object (a picture with its crop, a shape with text, a text box, Text Art as the
// fontwork shape the standard has, a group, a line) written to an .ods and read back keeps its place, size, turn, order, names, look, words, and picture; objects that another program wrote
// are read (a frame in a cell, a shape of the sheet between two cells, alternative text); and a drawing that is huge or broken is capped or left out. Every file is built in memory.
#include "drawinghelpers.h"
#include "odshelpers.h"

#include "io/zip.h"
#include "sheet/drawing.h"
#include "sheet/drawingadd.h"
#include "sheet/odsdraw.h"
#include "sheet/oldsave.h"
#include "sheet/xlsxbase.h"
#include "sheet/xlsxsave.h"
#include "core/fonts.h"
#include "office/drawingml.h"
#include "office/mathobject.h"
#include "wml/wmlmathedit.h"
#include "wml/wmlmathpath.h"

#include <QDomDocument>
#include <QRegularExpression>
#include <QThread>

#include "ole/odfobject.h"
#include "sheet/foreignbook.h"

namespace {

using namespace drawtest;

QByteArray xlsxWith(const QString &anchors, const QVector<DrawingTarget> &targets = {}) { return bookWithDrawing({sheettest::worksheet(QString())}, drawingPart(anchors), targets); }

struct Saved {
    QByteArray ods;
    QMap<QString, QByteArray> parts;
    jp::office::sheet::CopyReport report;
};

bool saveAsOds(const QByteArray &xlsx, Saved *s, QString *why)
{
    Package package;
    Workbook wb;
    if (!package.readLazy(xlsx, why) || !wb.open(package, why)) return false;
    if (!saveOds(wb, &s->ods, &s->report, why)) return false;
    return jp::readZip(s->ods, s->parts);
}

struct Read {
    Workbook wb;
    Sheet sheet;
    SheetDrawing drawing;
};

bool readOds(const QByteArray &ods, Read *r, QString *why, const Limits &limits = Limits())
{
    if (!openBytes(ods, QStringLiteral("d.ods"), &r->wb, why, limits)) return false;
    if (!r->wb.readSheet(0, &r->sheet)) {
        *why = r->sheet.error;
        return false;
    }
    QByteArray anchors;
    for (const QByteArray &a : r->sheet.importedDrawing.anchors) anchors += a;
    QString error;
    const std::shared_ptr<QDomDocument> parsed = parseDrawing(newDrawingXml(anchors), &error);
    if (!parsed) {
        *why = error;
        return false;
    }
    return readDrawing(Package(), QStringLiteral("/d.xml"), &r->drawing, why, parsed);
}

QDomElement child(const QDomElement &e, const char *local)
{
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (c.localName() == QLatin1String(local)) return c;
    return QDomElement();
}
QDomElement spPr(const DrawingObject &o) { return child(o.element, "spPr"); }
int rotationOf(const DrawingObject &o) { return child(child(spPr(o), "xfrm").isNull() ? o.element : spPr(o), "xfrm").attribute(QStringLiteral("rot")).toInt(); }

QString pictureXml(int id, const QString &rId, const QString &name, const QString &descr, int rot = 0, const QString &crop = QString())
{
    return QStringLiteral("<xdr:pic><xdr:nvPicPr><xdr:cNvPr id=\"%1\" name=\"%2\" descr=\"%3\" title=\"%2 title\"/><xdr:cNvPicPr/></xdr:nvPicPr><xdr:blipFill><a:blip r:embed=\"%4\"/>%5<a:stretch><a:fillRect/></a:stretch></xdr:blipFill>"
                          "<xdr:spPr><a:xfrm%6><a:off x=\"0\" y=\"0\"/><a:ext cx=\"1828800\" cy=\"914400\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></xdr:spPr></xdr:pic>")
        .arg(id).arg(name, descr, rId, crop, rot ? QStringLiteral(" rot=\"%1\"").arg(rot) : QString());
}

QString textBoxXml(int id, const QString &name, const QString &text)
{
    return QStringLiteral("<xdr:sp macro=\"\" textlink=\"\"><xdr:nvSpPr><xdr:cNvPr id=\"%1\" name=\"%2\" descr=\"a note\"/><xdr:cNvSpPr txBox=\"1\"/></xdr:nvSpPr><xdr:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"1828800\" cy=\"457200\"/></a:xfrm>"
                          "<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom><a:solidFill><a:srgbClr val=\"FFFFCC\"/></a:solidFill><a:ln w=\"12700\"><a:solidFill><a:srgbClr val=\"336699\"/></a:solidFill></a:ln></xdr:spPr>"
                          "<xdr:txBody><a:bodyPr wrap=\"square\" rtlCol=\"0\"/><a:lstStyle/><a:p><a:r><a:rPr lang=\"en-US\" sz=\"1400\" b=\"1\"><a:solidFill><a:srgbClr val=\"CC0000\"/></a:solidFill></a:rPr><a:t>%3</a:t></a:r></a:p></xdr:txBody></xdr:sp>")
        .arg(id).arg(name, text.toHtmlEscaped());
}

QString textArtXml(int id, const QString &name, const QString &warp, const QString &words)
{
    return QStringLiteral("<xdr:sp macro=\"\" textlink=\"\"><xdr:nvSpPr><xdr:cNvPr id=\"%1\" name=\"%2\" descr=\"curved words\"/><xdr:cNvSpPr/></xdr:nvSpPr><xdr:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"2857500\" cy=\"952500\"/></a:xfrm>"
                          "<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom><a:noFill/></xdr:spPr><xdr:txBody><a:bodyPr wrap=\"none\" lIns=\"0\" tIns=\"0\" rIns=\"0\" bIns=\"0\" fromWordArt=\"1\" anchor=\"ctr\"><a:prstTxWarp prst=\"%3\"><a:avLst/></a:prstTxWarp><a:normAutofit/></a:bodyPr>"
                          "<a:lstStyle/><a:p><a:pPr algn=\"ctr\"/><a:r><a:rPr lang=\"en-US\" sz=\"3600\" b=\"1\" i=\"0\"><a:ln w=\"12700\"><a:solidFill><a:srgbClr val=\"112233\"/></a:solidFill></a:ln><a:solidFill><a:srgbClr val=\"FF8800\"/></a:solidFill>"
                          "<a:latin typeface=\"Archivo Black\"/></a:rPr><a:t>%4</a:t></a:r></a:p></xdr:txBody></xdr:sp>")
        .arg(id).arg(name, warp, words.toHtmlEscaped());
}

QString textOf(const QDomElement &e)
{
    QString out;
    for (QDomElement t = e.firstChildElement(); !t.isNull(); t = t.nextSiblingElement()) {
        if (t.localName() == QLatin1String("t")) out += t.text();
        else out += textOf(t);
    }
    return out;
}

// The shapes of a hand-written sheet: 10 columns an inch wide and 10 rows half an inch high, and `inside` in the table before the columns.
QByteArray handWritten(const QString &shapes, const QString &rows = QString(), const QMap<QString, QByteArray> &extra = QMap<QString, QByteArray>(), const QString &styles = QString())
{
    const QString styleXml = QStringLiteral("<style:style style:name=\"co1\" style:family=\"table-column\"><style:table-column-properties style:column-width=\"1in\"/></style:style>"
                                            "<style:style style:name=\"ro1\" style:family=\"table-row\"><style:table-row-properties style:row-height=\"0.5in\"/></style:style>");
    const QString body = QStringLiteral("<table:table table:name=\"S\"><table:shapes>%1</table:shapes><table:table-column table:style-name=\"co1\" table:number-columns-repeated=\"10\"/>%2</table:table>")
                             .arg(shapes, rows.isEmpty() ? QStringLiteral("<table:table-row table:style-name=\"ro1\" table:number-rows-repeated=\"10\"><table:table-cell table:number-columns-repeated=\"10\"/></table:table-row>") : rows);
    return buildOds(contentXml(body, styleXml + styles), QString(), QByteArray(), extra);
}

// An .ods opened as a workbook, saved as an .xlsx that goes on to be an .ods (as the program saves one), and read from that.
bool resaved(const QByteArray &ods, Saved *s, QString *why)
{
    Workbook wb;
    if (!openBytes(ods, QStringLiteral("o.ods"), &wb, why)) return false;
    QByteArray xlsx;
    SaveReport report;
    SaveChanges changes;
    changes.keepOdfObjects = true;
    if (!saveXlsx(wb, changes, &xlsx, why, &report)) return false;
    return saveAsOds(xlsx, s, why);
}

// An .ods of one sheet with the frames in its shapes, the parts given, and a manifest that gives them their media types.
QByteArray objectSheet(const QString &frames, const QMap<QString, QByteArray> &parts, const QMap<QString, QString> &types)
{
    QByteArray manifest = "<?xml version=\"1.0\"?><manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\"><manifest:file-entry manifest:full-path=\"/\" manifest:media-type=\"application/vnd.oasis.opendocument.spreadsheet\"/>";
    for (auto it = types.cbegin(); it != types.cend(); ++it) manifest += "<manifest:file-entry manifest:full-path=\"" + it.key().toUtf8() + "\" manifest:media-type=\"" + it.value().toUtf8() + "\"/>";
    manifest += "</manifest:manifest>";
    const QString styleXml = QStringLiteral("<style:style style:name=\"co1\" style:family=\"table-column\"><style:table-column-properties style:column-width=\"1in\"/></style:style>"
                                            "<style:style style:name=\"ro1\" style:family=\"table-row\"><style:table-row-properties style:row-height=\"0.5in\"/></style:style>");
    const QString body = QStringLiteral("<table:table table:name=\"S\"><table:shapes>%1</table:shapes><table:table-column table:style-name=\"co1\" table:number-columns-repeated=\"10\"/>"
                                        "<table:table-row table:style-name=\"ro1\" table:number-rows-repeated=\"10\"><table:table-cell table:number-columns-repeated=\"10\"/></table:table-row></table:table>")
                             .arg(frames);
    return buildOds(contentXml(body, styleXml), QString(), QByteArray(), parts, QStringLiteral("application/vnd.oasis.opendocument.spreadsheet"), manifest);
}

QByteArray blobOf(int seed, int size)
{
    QByteArray b;
    for (int i = 0; i < size; ++i) b.append(char((i * 31 + seed * 7) & 0xFF));
    return b;
}

QString objectFrame(const QString &object, bool binary, int i = 0, const QString &picture = QString())
{
    return QStringLiteral("<draw:frame draw:name=\"O%4\" svg:width=\"1.5in\" svg:height=\"0.5in\" svg:x=\"1in\" svg:y=\"%3in\"><draw:%1 xlink:href=\"./%2\"/>%5</draw:frame>")
        .arg(binary ? QStringLiteral("object-ole") : QStringLiteral("object"), object).arg(0.1 + (i % 20) * 0.1).arg(i).arg(picture.isEmpty() ? QString() : QStringLiteral("<draw:image xlink:href=\"%1\"/>").arg(picture));
}

// Shrinks the object core's limits for a test and puts the real ones back when it ends.
struct LimitGuard {
    ~LimitGuard() { jp::office::ole::resetLimits(); }
};

} // namespace

class SheetOdsDrawTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // A picture between two cells, turned, cut, named, and described: it is a frame with an image in the .ods, with its bytes in the package, and comes back the same.
    void aPictureSurvivesSaveAsOds()
    {
        const QByteArray png = pngBytes(8, 4, QColor(200, 30, 30));
        const QString crop = QStringLiteral("<a:srcRect l=\"10000\" t=\"20000\"/>");
        const QString anchors = twoCell(1, 2, 4, 6, pictureXml(2, QStringLiteral("rId1"), QStringLiteral("Logo"), QStringLiteral("a red logo"), 1800000, crop), QString(), 100000, 50000, 200000, 30000);
        Saved s;
        QString why;
        QVERIFY2(saveAsOds(xlsxWith(anchors, {imageTarget(QStringLiteral("rId1"), 1, png)}), &s, &why), qPrintable(why));
        const QByteArray content = s.parts.value(QStringLiteral("content.xml"));
        QVERIFY2(content.contains("<draw:image"), "no picture in content.xml");
        QVERIFY(content.contains("<svg:title>Logo title</svg:title>"));
        QVERIFY(content.contains("<svg:desc>a red logo</svg:desc>"));
        QVERIFY(!s.report.notKept.contains(QStringLiteral("pictures and other drawing objects")));
        QString picture;
        for (auto it = s.parts.cbegin(); it != s.parts.cend(); ++it)
            if (it.key().startsWith(QLatin1String("Pictures/"))) picture = it.key();
        QVERIFY(!picture.isEmpty());
        QCOMPARE(s.parts.value(picture), png);
        QVERIFY(s.parts.value(QStringLiteral("META-INF/manifest.xml")).contains(picture.toUtf8()));
        Read r;
        QVERIFY2(readOds(s.ods, &r, &why), qPrintable(why));
        QCOMPARE(r.drawing.objects.size(), 1);
        const DrawingObject &o = r.drawing.objects[0];
        QCOMPARE(int(o.kind), int(ObjectKind::Picture));
        QCOMPARE(o.name, QStringLiteral("Logo"));
        QCOMPARE(o.description, QStringLiteral("a red logo"));
        QCOMPARE(child(o.element, "nvPicPr").firstChildElement(QStringLiteral("cNvPr")).attribute(QStringLiteral("title")), QStringLiteral("Logo title"));
        QCOMPARE(int(o.anchor.kind), int(AnchorKind::TwoCell));
        QCOMPARE(int(o.anchor.from.col), 1);
        QCOMPARE(int(o.anchor.from.row), 2);
        QCOMPARE(int(o.anchor.to.col), 4);
        QCOMPARE(int(o.anchor.to.row), 6);
        QVERIFY(std::abs(o.anchor.from.colOff - 100000) <= 2);
        QVERIFY(std::abs(o.anchor.from.rowOff - 50000) <= 2);
        QVERIFY(std::abs(o.anchor.to.colOff - 200000) <= 2);
        QVERIFY(std::abs(o.anchor.to.rowOff - 30000) <= 2);
        QVERIFY(std::abs(rotationOf(o) - 1800000) <= 60);
        const QDomElement src = child(child(o.element, "blipFill"), "srcRect");
        QVERIFY(!src.isNull());
        QVERIFY(std::abs(src.attribute(QStringLiteral("l")).toInt() - 10000) <= 50);
        QVERIFY(std::abs(src.attribute(QStringLiteral("t")).toInt() - 20000) <= 50);
        QCOMPARE(r.sheet.importedDrawing.media.size(), 1);
        QCOMPARE(r.sheet.importedDrawing.media[0].bytes, png);
    }

    // A shape with a fill, an outline, and words; a text box; and the order they are drawn in (the first is at the back).
    void shapesAndTextBoxesKeepTheirLookAndOrder()
    {
        const QString shape = QStringLiteral("<xdr:sp macro=\"\" textlink=\"\"><xdr:nvSpPr><xdr:cNvPr id=\"2\" name=\"Oval\" descr=\"an oval\"/><xdr:cNvSpPr/></xdr:nvSpPr><xdr:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"1828800\" cy=\"914400\"/></a:xfrm>"
                                             "<a:prstGeom prst=\"ellipse\"><a:avLst/></a:prstGeom><a:solidFill><a:srgbClr val=\"3366CC\"/></a:solidFill><a:ln w=\"25400\"><a:solidFill><a:srgbClr val=\"AA0000\"/></a:solidFill></a:ln></xdr:spPr>"
                                             "<xdr:txBody><a:bodyPr/><a:lstStyle/><a:p><a:r><a:rPr lang=\"en-US\"/><a:t>Inside the oval</a:t></a:r></a:p></xdr:txBody></xdr:sp>");
        const QString anchors = twoCell(0, 0, 3, 3, shape) + oneCell(5, 5, 1828800, 457200, textBoxXml(3, QStringLiteral("Note"), QStringLiteral("Check the totals")), 20000, 10000)
                                + absolute(952500, 1905000, 914400, 914400, shapeElement(4, QStringLiteral("Square"), QStringLiteral("rect"), QString(), QStringLiteral("00FF00")));
        Saved s;
        QString why;
        QVERIFY2(saveAsOds(xlsxWith(anchors), &s, &why), qPrintable(why));
        const QByteArray content = s.parts.value(QStringLiteral("content.xml"));
        QVERIFY2(content.contains("<draw:custom-shape"), "no shape in content.xml");
        QVERIFY(content.contains("<draw:text-box"));
        Read r;
        QVERIFY2(readOds(s.ods, &r, &why), qPrintable(why));
        QCOMPARE(r.drawing.objects.size(), 3);
        const DrawingObject oval = r.drawing.objects[0], note = r.drawing.objects[1], square = r.drawing.objects[2];
        QCOMPARE(oval.name, QStringLiteral("Oval"));
        QCOMPARE(oval.description, QStringLiteral("an oval"));
        QCOMPARE(int(oval.kind), int(ObjectKind::Shape));
        QCOMPARE(child(spPr(oval), "prstGeom").attribute(QStringLiteral("prst")), QStringLiteral("ellipse"));
        QCOMPARE(child(child(spPr(oval), "solidFill"), "srgbClr").attribute(QStringLiteral("val")).toUpper(), QStringLiteral("3366CC"));
        const QDomElement line = child(spPr(oval), "ln");
        QCOMPARE(child(child(line, "solidFill"), "srgbClr").attribute(QStringLiteral("val")).toUpper(), QStringLiteral("AA0000"));
        QVERIFY(std::abs(line.attribute(QStringLiteral("w")).toInt() - 25400) <= 100);
        QVERIFY(textOf(child(oval.element, "txBody")).contains(QStringLiteral("Inside the oval")));
        QCOMPARE(int(oval.anchor.kind), int(AnchorKind::TwoCell));
        QCOMPARE(int(oval.anchor.to.col), 3);
        QCOMPARE(note.name, QStringLiteral("Note"));
        QCOMPARE(child(note.element, "nvSpPr").firstChildElement(QStringLiteral("cNvSpPr")).attribute(QStringLiteral("txBox")), QStringLiteral("1"));
        QCOMPARE(textOf(child(note.element, "txBody")), QStringLiteral("Check the totals"));
        QCOMPARE(child(child(spPr(note), "solidFill"), "srgbClr").attribute(QStringLiteral("val")).toUpper(), QStringLiteral("FFFFCC"));
        QCOMPARE(int(note.anchor.kind), int(AnchorKind::OneCell));
        QCOMPARE(int(note.anchor.from.col), 5);
        QCOMPARE(int(note.anchor.from.row), 5);
        QVERIFY(std::abs(note.anchor.from.colOff - 20000) <= 2);
        QVERIFY(std::abs(note.anchor.cx - 1828800) <= 2);
        QVERIFY(std::abs(note.anchor.cy - 457200) <= 2);
        QCOMPARE(int(square.anchor.kind), int(AnchorKind::Absolute));
        QVERIFY(std::abs(square.anchor.x - 952500) <= 2);
        QVERIFY(std::abs(square.anchor.y - 1905000) <= 2);
        QVERIFY(std::abs(square.anchor.cx - 914400) <= 2);
    }

    // A text box whose words run turned (90 degrees: the writing mode tb-rl of the standard; 270: bt-lr of the extension's namespace) comes back turned; a direction this program does not know is across.
    void aTurnedTextBoxKeepsItsDirectionInAnOds()
    {
        struct Case {
            const char *vert, *written;
        };
        for (const Case c : {Case{"vert", "style:writing-mode=\"tb-rl\""}, Case{"vert270", "loext:writing-mode=\"bt-lr\""}, Case{"horz", nullptr}, Case{"junk", nullptr}}) {
            QString box = textBoxXml(3, QStringLiteral("Note"), QStringLiteral("Check the totals"));
            box.replace(QStringLiteral("<a:bodyPr wrap=\"square\" rtlCol=\"0\"/>"), QStringLiteral("<a:bodyPr vert=\"%1\" wrap=\"square\" rtlCol=\"0\"/>").arg(QLatin1String(c.vert)));
            Saved s;
            QString why;
            QVERIFY2(saveAsOds(xlsxWith(oneCell(5, 5, 1828800, 457200, box, 20000, 10000)), &s, &why), qPrintable(why));
            const QByteArray content = s.parts.value(QStringLiteral("content.xml"));
            if (c.written) QVERIFY2(content.contains(c.written), c.vert);
            else QVERIFY2(!content.contains("writing-mode=\"tb") && !content.contains("writing-mode=\"bt"), c.vert);
            Read r;
            QVERIFY2(readOds(s.ods, &r, &why), qPrintable(why));
            QCOMPARE(r.drawing.objects.size(), 1);
            const QString back = child(child(r.drawing.objects[0].element, "txBody"), "bodyPr").attribute(QStringLiteral("vert"));
            QCOMPARE(back, QLatin1String(c.vert) == QLatin1String("vert") || QLatin1String(c.vert) == QLatin1String("vert270") ? QLatin1String(c.vert) : QString());
        }
    }

    // The mode may be in the paragraph properties of the shape's own style (where other programs put it) when its graphic properties do not say; a paragraph style of the words is not looked at.
    void aTextBoxStyleCanSayItsDirectionInItsParagraphProperties()
    {
        QString box = textBoxXml(3, QStringLiteral("Note"), QStringLiteral("Check the totals"));
        box.replace(QStringLiteral("<a:bodyPr wrap=\"square\" rtlCol=\"0\"/>"), QStringLiteral("<a:bodyPr vert=\"vert270\" wrap=\"square\" rtlCol=\"0\"/>"));
        Saved s;
        QString why;
        QVERIFY2(saveAsOds(xlsxWith(oneCell(5, 5, 1828800, 457200, box, 20000, 10000)), &s, &why), qPrintable(why));
        const QString content = QString::fromUtf8(s.parts.value(QStringLiteral("content.xml")));
        const QRegularExpression styleWithMode(QStringLiteral("(<style:style [^>]*>)((?:(?!</style:style>).)*?)(<style:graphic-properties[^>]*?) style:writing-mode=\"tb-rl\" loext:writing-mode=\"bt-lr\"([^>]*/>)((?:(?!</style:style>).)*)</style:style>"),
                                               QRegularExpression::DotMatchesEverythingOption);
        QVERIFY(styleWithMode.match(content).hasMatch());
        auto readBack = [&](const QString &changed) {
            QMap<QString, QByteArray> parts = s.parts;
            parts.insert(QStringLiteral("content.xml"), changed.toUtf8());
            jp::ZipWriter z;
            for (auto it = parts.cbegin(); it != parts.cend(); ++it) z.add(it.key(), it.value(), true);
            Read r;
            QString err;
            if (!readOds(z.finish(), &r, &err) || r.drawing.objects.size() != 1) return QString::fromLatin1("unread");
            return child(child(r.drawing.objects[0].element, "txBody"), "bodyPr").attribute(QStringLiteral("vert"));
        };
        // (moved to the paragraph properties of the same style)
        QString moved = content;
        moved.replace(styleWithMode, QStringLiteral("\\1\\2\\3\\4<style:paragraph-properties style:writing-mode=\"tb-rl\" loext:writing-mode=\"bt-lr\"/>\\5</style:style>"));
        QVERIFY(moved != content);
        QCOMPARE(readBack(moved), QStringLiteral("vert270"));
        // (the graphic properties win when they say; and a paragraph style of the words is not the shape's)
        QString onWords = content;
        onWords.replace(styleWithMode, QStringLiteral("\\1\\2\\3\\4\\5</style:style>"));
        onWords.replace(QStringLiteral("</office:automatic-styles>"), QStringLiteral("<style:style style:name=\"PW\" style:family=\"paragraph\"><style:paragraph-properties style:writing-mode=\"tb-rl\"/></style:style></office:automatic-styles>"));
        static const QRegularExpression wordsStyle(QStringLiteral("<text:p text:style-name=\"[^\"]*\""));
        QVERIFY(onWords.contains(wordsStyle));
        onWords.replace(wordsStyle, QStringLiteral("<text:p text:style-name=\"PW\""));
        QCOMPARE(readBack(onWords), QString());
    }

    // Text Art is a fontwork shape in the .ods: a custom shape with an enhanced geometry that says its words are set along a path. It comes back as Text Art with the same words, font, style, and warp.
    void textArtIsAFontworkShape()
    {
        Saved s;
        QString why;
        QVERIFY2(saveAsOds(xlsxWith(twoCell(1, 1, 5, 4, textArtXml(2, QStringLiteral("Banner"), QStringLiteral("textArchUp"), QStringLiteral("Sale Today")))), &s, &why), qPrintable(why));
        const QByteArray content = s.parts.value(QStringLiteral("content.xml"));
        QVERIFY2(content.contains("<draw:custom-shape"), "no custom shape");
        QVERIFY(content.contains("<draw:enhanced-geometry"));
        QVERIFY(content.contains("draw:fontwork-"));
        QVERIFY(content.contains("draw:text-path=\"true\""));
        QVERIFY(content.contains("Sale Today"));
        Read r;
        QVERIFY2(readOds(s.ods, &r, &why), qPrintable(why));
        QCOMPARE(r.drawing.objects.size(), 1);
        const DrawingObject o = r.drawing.objects[0];
        QCOMPARE(o.name, QStringLiteral("Banner"));
        QCOMPARE(o.description, QStringLiteral("curved words"));
        const QDomElement body = child(o.element, "txBody");
        const QDomElement props = child(body, "bodyPr");
        QCOMPARE(props.attribute(QStringLiteral("fromWordArt")), QStringLiteral("1"));
        QCOMPARE(child(props, "prstTxWarp").attribute(QStringLiteral("prst")), QStringLiteral("textArchUp"));
        QCOMPARE(textOf(body), QStringLiteral("Sale Today"));
        const QDomElement look = child(child(child(body, "p"), "r"), "rPr");
        QCOMPARE(look.attribute(QStringLiteral("b")), QStringLiteral("1"));
        QCOMPARE(child(look, "latin").attribute(QStringLiteral("typeface")), QStringLiteral("Archivo Black"));
        QCOMPARE(child(child(look, "solidFill"), "srgbClr").attribute(QStringLiteral("val")).toUpper(), QStringLiteral("FF8800"));
        QCOMPARE(child(child(child(look, "ln"), "solidFill"), "srgbClr").attribute(QStringLiteral("val")).toUpper(), QStringLiteral("112233"));
    }

    // A warp the standard has no shape for keeps the words and the look; the warp is written as plain, and the save says nothing is lost silently.
    void aWarpWithNoFontworkShapeKeepsWordsAndStyle()
    {
        Saved s;
        QString why;
        QVERIFY2(saveAsOds(xlsxWith(twoCell(0, 0, 4, 3, textArtXml(2, QStringLiteral("Wavy"), QStringLiteral("textWave2"), QStringLiteral("Two Waves")))), &s, &why), qPrintable(why));
        Read r;
        QVERIFY2(readOds(s.ods, &r, &why), qPrintable(why));
        QCOMPARE(r.drawing.objects.size(), 1);
        const QDomElement body = child(r.drawing.objects[0].element, "txBody");
        QCOMPARE(child(body, "bodyPr").attribute(QStringLiteral("fromWordArt")), QStringLiteral("1"));
        QCOMPARE(textOf(body), QStringLiteral("Two Waves"));
        QCOMPARE(child(child(child(body, "p"), "r"), "rPr").attribute(QStringLiteral("b")), QStringLiteral("1"));
    }

    // A group of shapes and a line: the group is a group in the file, and its members are in it.
    void groupsAndLinesAreKept()
    {
        const QString group = groupElement(2, shapeElement(3, QStringLiteral("Left"), QStringLiteral("rect"), QString(), QStringLiteral("FF0000"), 914400, 457200)
                                                  + shapeElement(4, QStringLiteral("Right"), QStringLiteral("ellipse"), QString(), QStringLiteral("0000FF"), 914400, 457200));
        const QString anchors = twoCell(0, 0, 2, 1, group) + twoCell(0, 3, 3, 4, connectorElement(7));
        Saved s;
        QString why;
        QVERIFY2(saveAsOds(xlsxWith(anchors), &s, &why), qPrintable(why));
        const QByteArray content = s.parts.value(QStringLiteral("content.xml"));
        QVERIFY2(content.contains("<draw:g"), "no group in content.xml");
        QVERIFY(content.contains("<draw:line"));
        Read r;
        QVERIFY2(readOds(s.ods, &r, &why), qPrintable(why));
        QCOMPARE(r.drawing.objects.size(), 2);
        QCOMPARE(int(r.drawing.objects[0].kind), int(ObjectKind::Group));
        QCOMPARE(r.drawing.objects[0].element.elementsByTagNameNS(kXdr, QStringLiteral("sp")).size(), 2);
        QCOMPARE(int(r.drawing.objects[1].kind), int(ObjectKind::Connector));
    }

    // What another program writes: a frame inside a cell (its place is from the cell), a shape of the sheet between two cells (its first cell is where it lies), alternative text in svg:title and
    // svg:desc, a picture in Pictures/ with a clip, a text box with styled words, and a shape of the older simple kind.
    void readsAnOdsAnotherProgramWrote()
    {
        const QByteArray png = pngBytes(100, 50, QColor(10, 200, 10));
        const QString styles = QStringLiteral("<style:style style:name=\"gr1\" style:family=\"graphic\"><style:graphic-properties draw:fill=\"solid\" draw:fill-color=\"#ffcc00\" draw:stroke=\"solid\" svg:stroke-width=\"0.0278in\" svg:stroke-color=\"#112233\"/></style:style>"
                                                "<style:style style:name=\"gr2\" style:family=\"graphic\"><style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\" style:mirror=\"none\" fo:clip=\"rect(0.1in, 0in, 0.2in, 0.3in)\"/></style:style>"
                                                "<style:style style:name=\"T1\" style:family=\"text\"><style:text-properties fo:font-weight=\"bold\" fo:color=\"#ff0000\" fo:font-size=\"16pt\"/></style:style>");
        const QString shapes = QStringLiteral("<draw:custom-shape draw:name=\"Sun\" draw:style-name=\"gr1\" draw:z-index=\"1\" svg:width=\"1in\" svg:height=\"0.5in\" svg:x=\"2.5in\" svg:y=\"0.75in\" table:end-cell-address=\"S.D5\" table:end-x=\"0.1in\" table:end-y=\"0.2in\">"
                                              "<svg:title>The sun</svg:title><svg:desc>A yellow oval</svg:desc><text:p>Hot <text:span text:style-name=\"T1\">day</text:span></text:p>"
                                              "<draw:enhanced-geometry svg:viewBox=\"0 0 21600 21600\" draw:type=\"ellipse\" draw:enhanced-path=\"U 10800 10800 10800 10800 0 360 Z N\"/></draw:custom-shape>"
                                              "<draw:rect draw:name=\"Old\" draw:style-name=\"gr1\" draw:z-index=\"0\" svg:x=\"0in\" svg:y=\"0in\" svg:width=\"1in\" svg:height=\"1in\"><text:p>old kind</text:p></draw:rect>");
        const QString inCell = QStringLiteral("<draw:frame draw:name=\"Photo\" draw:style-name=\"gr2\" draw:z-index=\"2\" svg:width=\"1in\" svg:height=\"0.5in\" svg:x=\"0.1in\" svg:y=\"0.05in\">"
                                             "<draw:image xlink:href=\"Pictures/photo.png\" xlink:type=\"simple\" xlink:show=\"embed\" xlink:actuate=\"onLoad\"/><svg:title>Photo title</svg:title><svg:desc>A green photo</svg:desc></draw:frame>");
        const QString box = QStringLiteral("<draw:frame draw:name=\"Words\" draw:z-index=\"3\" svg:width=\"1.5in\" svg:height=\"0.5in\" draw:transform=\"rotate (0.5236) translate (4.5in 1.5in)\"><draw:text-box><text:p>Plain box</text:p></draw:text-box></draw:frame>");
        const QString rows = QStringLiteral("<table:table-row table:style-name=\"ro1\" table:number-rows-repeated=\"3\"><table:table-cell table:number-columns-repeated=\"10\"/></table:table-row>"
                                            "<table:table-row table:style-name=\"ro1\"><table:table-cell table:number-columns-repeated=\"2\"/><table:table-cell>")
                                + inCell + QStringLiteral("</table:table-cell><table:table-cell table:number-columns-repeated=\"7\"/></table:table-row><table:table-row table:style-name=\"ro1\" table:number-rows-repeated=\"6\"><table:table-cell table:number-columns-repeated=\"10\"/></table:table-row>");
        QString why;
        Read r;
        QVERIFY2(readOds(handWritten(shapes + box, rows, {{QStringLiteral("Pictures/photo.png"), png}}, styles), &r, &why), qPrintable(why));
        QCOMPARE(r.drawing.objects.size(), 4);
        // (drawn in the order of z-index: Old, Sun, Photo, Words)
        QCOMPARE(r.drawing.objects[0].name, QStringLiteral("Old"));
        const DrawingObject sun = r.drawing.objects[1], photo = r.drawing.objects[2], words = r.drawing.objects[3];
        QCOMPARE(sun.name, QStringLiteral("Sun"));
        QCOMPARE(sun.description, QStringLiteral("A yellow oval"));
        QCOMPARE(int(sun.anchor.kind), int(AnchorKind::TwoCell));
        QCOMPARE(int(sun.anchor.from.col), 2);
        QCOMPARE(int(sun.anchor.from.row), 1);
        QVERIFY(std::abs(sun.anchor.from.colOff - 457200) <= 2);
        QVERIFY(std::abs(sun.anchor.from.rowOff - 228600) <= 2);
        QCOMPARE(int(sun.anchor.to.col), 3);
        QCOMPARE(int(sun.anchor.to.row), 4);
        QVERIFY(std::abs(sun.anchor.to.colOff - 91440) <= 2);
        QVERIFY(std::abs(sun.anchor.to.rowOff - 182880) <= 2);
        QCOMPARE(child(child(spPr(sun), "solidFill"), "srgbClr").attribute(QStringLiteral("val")).toUpper(), QStringLiteral("FFCC00"));
        QCOMPARE(child(spPr(sun), "prstGeom").attribute(QStringLiteral("prst")), QStringLiteral("ellipse"));
        QVERIFY(textOf(child(sun.element, "txBody")).contains(QStringLiteral("Hot day")));
        QCOMPARE(int(photo.kind), int(ObjectKind::Picture));
        QCOMPARE(photo.name, QStringLiteral("Photo"));
        QCOMPARE(photo.description, QStringLiteral("A green photo"));
        QCOMPARE(int(photo.anchor.kind), int(AnchorKind::OneCell));
        QCOMPARE(int(photo.anchor.from.col), 2);
        QCOMPARE(int(photo.anchor.from.row), 3);
        QVERIFY(std::abs(photo.anchor.from.colOff - 91440) <= 2);
        QVERIFY(std::abs(photo.anchor.cx - 914400) <= 2);
        QVERIFY(!child(child(photo.element, "blipFill"), "srcRect").isNull());
        QCOMPARE(r.sheet.importedDrawing.media.size(), 1);
        QCOMPARE(r.sheet.importedDrawing.media[0].bytes, png);
        QCOMPARE(textOf(child(words.element, "txBody")), QStringLiteral("Plain box"));
        QCOMPARE(int(words.anchor.kind), int(AnchorKind::Absolute));
        QVERIFY(std::abs(rotationOf(words) - 19800000) <= 200);
        QVERIFY(std::abs(words.anchor.x - qRound(4.5245 * 914400)) <= 6000);
        QVERIFY(std::abs(words.anchor.y - qRound(1.0915 * 914400)) <= 6000);
        // the cells are there too
        QVERIFY(r.sheet.error.isEmpty());
    }

    // A workbook opened from an .ods keeps its drawing objects when it is saved as an .xlsx, and they read from that.
    void anOdsWithObjectsSavesAsXlsxWithThem()
    {
        const QByteArray png = pngBytes(6, 6, QColor(1, 2, 3));
        const QString shapes = QStringLiteral("<draw:frame draw:name=\"Pic\" draw:z-index=\"0\" svg:width=\"1in\" svg:height=\"1in\" svg:x=\"1in\" svg:y=\"0.5in\"><draw:image xlink:href=\"Pictures/p.png\"/></draw:frame>"
                                              "<draw:custom-shape draw:name=\"Box\" draw:z-index=\"1\" svg:width=\"1in\" svg:height=\"1in\" svg:x=\"3in\" svg:y=\"0.5in\"><text:p>hi</text:p><draw:enhanced-geometry draw:type=\"rectangle\"/></draw:custom-shape>");
        Workbook wb;
        QString why;
        QVERIFY2(openBytes(handWritten(shapes, QString(), {{QStringLiteral("Pictures/p.png"), png}}), QStringLiteral("x.ods"), &wb, &why), qPrintable(why));
        QByteArray xlsx;
        SaveReport report;
        QVERIFY2(saveXlsx(wb, SaveChanges(), &xlsx, &why, &report), qPrintable(why));
        Package package;
        Workbook again;
        QVERIFY2(package.readLazy(xlsx, &why) && again.open(package, &why), qPrintable(why));
        const QString part = drawingPartOf(package, again.info().sheets.value(0).part);
        QVERIFY2(!part.isEmpty(), "the saved workbook has no drawing");
        SheetDrawing d;
        QVERIFY2(readDrawing(package, part, &d, &why), qPrintable(why));
        QCOMPARE(d.objects.size(), 2);
        QCOMPARE(int(d.objects[0].kind), int(ObjectKind::Picture));
        QCOMPARE(d.objects[0].name, QStringLiteral("Pic"));
        QCOMPARE(int(d.objects[1].kind), int(ObjectKind::Shape));
        const QStringList names = package.partNames();
        QVERIFY(std::any_of(names.cbegin(), names.cend(), [&](const QString &n) { return n.contains(QLatin1String("/media/")) && package.part(n) == png; }));
    }

    // ---- equations as formula objects

    static QString equationAnchors(int count, const QString &oMath = QStringLiteral("<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\"><m:f><m:num><m:r><m:t>a</m:t></m:r></m:num><m:den><m:r><m:t>b</m:t></m:r></m:den></m:f><m:r><m:t>+x</m:t></m:r></m:oMath>"))
    {
        QString out;
        for (int i = 0; i < count; ++i) {
            const QString shape = QStringLiteral("<xdr:sp macro=\"\" textlink=\"\"><xdr:nvSpPr><xdr:cNvPr id=\"%1\" name=\"Equation %1\"/><xdr:cNvSpPr txBox=\"1\"/></xdr:nvSpPr><xdr:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"1828800\" cy=\"457200\"/></a:xfrm>"
                                                "<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom><a:noFill/></xdr:spPr><xdr:txBody><a:bodyPr wrap=\"none\"/><a:lstStyle/><a:p><a14:m xmlns:a14=\"http://schemas.microsoft.com/office/drawing/2010/main\">"
                                                "<m:oMathPara xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">%2</m:oMathPara></a14:m></a:p></xdr:txBody></xdr:sp>")
                                      .arg(i + 2).arg(oMath);
            out += QStringLiteral("<mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\"><mc:Choice xmlns:a14=\"http://schemas.microsoft.com/office/drawing/2010/main\" Requires=\"a14\">")
                   + twoCell(1, 2 + i % 5, 4, 4 + i % 5, shape) + QStringLiteral("</mc:Choice></mc:AlternateContent>");
        }
        return out;
    }
    static QStringList equationLinesOf(const Read &r)
    {
        QStringList out;
        for (const DrawingObject &o : r.drawing.objects)
            if (const QByteArray markup = jp::office::shapeMath(o.element); !markup.isEmpty()) out << jp::office::wml::equationLine(markup);
        return out;
    }
    static int formulaPartsOf(const QMap<QString, QByteArray> &parts)
    {
        int n = 0;
        for (auto it = parts.cbegin(); it != parts.cend(); ++it)
            if (it.key().startsWith(QLatin1String("Object ")) && it.key().endsWith(QLatin1String("/content.xml")) && it.value().contains("MathML")) ++n;
        return n;
    }
    static QByteArray hello() { return "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mfrac><mi>p</mi><mi>q</mi></mfrac></math>"; }
    static QString formulaFrame(const QString &object, int i = 0)
    {
        return QStringLiteral("<draw:frame draw:name=\"F%3\" svg:width=\"1.5in\" svg:height=\"0.5in\" svg:x=\"1in\" svg:y=\"%2in\"><draw:object xlink:href=\"./%1\"/><draw:image xlink:href=\"ObjectReplacements/%1.png\"/></draw:frame>")
            .arg(object).arg(0.1 + (i % 20) * 0.1).arg(i);
    }

    void anEquationIsAFormulaObjectInAnOds()
    {
        jp::initCore();
        Saved s;
        QString why;
        QVERIFY2(saveAsOds(xlsxWith(equationAnchors(2)), &s, &why), qPrintable(why));
        QVERIFY(s.parts.value(QStringLiteral("content.xml")).contains("<draw:object"));
        QVERIFY(s.parts.value(QStringLiteral("Object 1/content.xml")).contains("MathML"));
        QVERIFY(s.parts.value(QStringLiteral("Object 2/content.xml")).contains("<mfrac"));
        QVERIFY(s.parts.value(QStringLiteral("META-INF/manifest.xml")).contains("application/vnd.oasis.opendocument.formula"));
        QVERIFY2(!s.report.notKept.join(QLatin1Char('|')).contains(QLatin1String("quation")), qPrintable(s.report.notKept.join(QLatin1Char('|'))));
        QCOMPARE(formulaPartsOf(s.parts), 2);
        Read r;
        QVERIFY2(readOds(s.ods, &r, &why), qPrintable(why));
        QVERIFY2(r.sheet.importedDrawing.notRead.isEmpty(), qPrintable(r.sheet.importedDrawing.notRead.join(QLatin1Char('|'))));
        const QStringList lines = equationLinesOf(r);
        QCOMPARE(lines.size(), 2);
        QVERIFY(!lines[0].isEmpty());
        // The line is the equation's own, as made from the first file.
        Read first;
        QVERIFY2(readOds(handWritten(QString()), &first, &why), qPrintable(why));
        QCOMPARE(lines[0], jp::office::wml::equationLine(jp::office::wml::equationMarkupDeclared(QByteArray("<m:oMath><m:f><m:num><m:r><m:t>a</m:t></m:r></m:num><m:den><m:r><m:t>b</m:t></m:r></m:den></m:f><m:r><m:t>+x</m:t></m:r></m:oMath>"))));
        // Opened and saved as a workbook, and as an .ods again: the same objects and the same parts.
        Workbook wb;
        QVERIFY2(openBytes(s.ods, QStringLiteral("e.ods"), &wb, &why), qPrintable(why));
        QByteArray xlsx;
        SaveReport report;
        QVERIFY2(saveXlsx(wb, SaveChanges(), &xlsx, &why, &report), qPrintable(why));
        Saved again;
        QVERIFY2(saveAsOds(xlsx, &again, &why), qPrintable(why));
        QCOMPARE(QStringList(again.parts.keys()), QStringList(s.parts.keys()));
        Read second;
        QVERIFY2(readOds(again.ods, &second, &why), qPrintable(why));
        QCOMPARE(equationLinesOf(second), lines);
    }

    void aHandWrittenFormulaObjectIsAnEquationOnTheSheet()
    {
        jp::initCore();
        QString why;
        Read r;
        QVERIFY2(readOds(handWritten(formulaFrame(QStringLiteral("Object 1")), QString(), {{QStringLiteral("Object 1/content.xml"), hello()}, {QStringLiteral("ObjectReplacements/Object 1.png"), pngBytes(4, 4, QColor(1, 2, 3))}}), &r, &why),
                  qPrintable(why));
        QVERIFY2(r.sheet.importedDrawing.notRead.isEmpty(), qPrintable(r.sheet.importedDrawing.notRead.join(QLatin1Char('|'))));
        QCOMPARE(equationLinesOf(r), QStringList{jp::office::wml::equationLine(jp::office::wml::equationMarkupDeclared(jp::office::mathObjectToOmml(hello())))});
    }

    void aFormulaObjectThatRunsOrLinksOrIsNotMathIsItsPictureOnTheSheet()
    {
        const QByteArray script = "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mi>x</mi><script>alert(1)</script></math>";
        const QByteArray link = "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mi href=\"http://example.com/\">x</mi></math>";
        const QByteArray other = "<svg xmlns=\"http://www.w3.org/2000/svg\"><mi>x</mi></svg>";
        for (const QByteArray &bad : {script, link, other}) {
            QString why;
            Read r;
            QVERIFY2(readOds(handWritten(formulaFrame(QStringLiteral("Object 1")), QString(), {{QStringLiteral("Object 1/content.xml"), bad}, {QStringLiteral("ObjectReplacements/Object 1.png"), pngBytes(4, 4, QColor(1, 2, 3))}}), &r, &why),
                      qPrintable(why));
            QVERIFY2(equationLinesOf(r).isEmpty(), bad.constData());
            QCOMPARE(r.drawing.objects.size(), 1);
            QCOMPARE(int(r.drawing.objects[0].kind), int(ObjectKind::Picture));
        }
    }

    // ---- embedded objects of other kinds

    static void twoObjectParts(QMap<QString, QByteArray> *parts, QMap<QString, QString> *types)
    {
        parts->insert(QStringLiteral("Object 1"), blobOf(1, 3000));
        types->insert(QStringLiteral("Object 1"), QStringLiteral("application/vnd.sun.star.oleobject"));
        parts->insert(QStringLiteral("Object 2/content.xml"), "<?xml version=\"1.0\"?><office:document-content xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\"><office:body><office:drawing/></office:body></office:document-content>");
        parts->insert(QStringLiteral("Object 2/Pictures/a.png"), pngBytes(4, 4, QColor(9, 9, 9)));
        parts->insert(QStringLiteral("Object 2/Inner/data.bin"), blobOf(2, 500));
        types->insert(QStringLiteral("Object 2/"), QStringLiteral("application/vnd.oasis.opendocument.graphics"));
        types->insert(QStringLiteral("Object 2/content.xml"), QStringLiteral("text/xml"));
        types->insert(QStringLiteral("Object 2/Pictures/a.png"), QStringLiteral("image/png"));
        types->insert(QStringLiteral("Object 2/Inner/"), QStringLiteral("application/x-inner"));
        types->insert(QStringLiteral("Object 2/Inner/data.bin"), QStringLiteral("application/octet-stream"));
        parts->insert(QStringLiteral("ObjectReplacements/Object 1.png"), pngBytes(4, 4, QColor(1, 2, 3)));
        parts->insert(QStringLiteral("ObjectReplacements/Object 2.png"), pngBytes(4, 4, QColor(3, 2, 1)));
    }

    static QMap<QString, QString> manifestTypesOf(const QMap<QString, QByteArray> &parts)
    {
        QMap<QString, QString> out;
        const QHash<QString, QString> types = jp::office::ole::odfManifestTypes(parts.value(QStringLiteral("META-INF/manifest.xml")));
        for (auto it = types.cbegin(); it != types.cend(); ++it) out.insert(it.key(), it.value());
        return out;
    }

    // A binary object of another program and a sub-document object: the picture shows, and saving writes every part byte for byte, with the manifest entries they had.
    void foreignObjectsSurviveAnOdsWithAllTheirParts()
    {
        QMap<QString, QByteArray> parts;
        QMap<QString, QString> types;
        twoObjectParts(&parts, &types);
        const QByteArray ods = objectSheet(objectFrame(QStringLiteral("Object 1"), true, 0, QStringLiteral("ObjectReplacements/Object 1.png")) + objectFrame(QStringLiteral("Object 2"), false, 1, QStringLiteral("ObjectReplacements/Object 2.png")), parts, types);
        QString why;
        Read r;
        QVERIFY2(readOds(ods, &r, &why), qPrintable(why));
        QCOMPARE(r.sheet.importedDrawing.objects.size(), 2);
        QCOMPARE(r.drawing.objects.size(), 2);   // (each shows its picture)
        QVERIFY2(r.sheet.importedDrawing.notRead.isEmpty(), qPrintable(r.sheet.importedDrawing.notRead.join(QLatin1Char('|'))));
        QVERIFY2(!r.wb.foreignBook()->skipped().join(QLatin1Char('|')).contains(QLatin1String("charts")), qPrintable(r.wb.foreignBook()->skipped().join(QLatin1Char('|'))));
        // (opened as a workbook and saved as an .xlsx and from it as an .ods, as the program does)
        Saved s;
        QVERIFY2(resaved(ods, &s, &why), qPrintable(why));
        const QMap<QString, QByteArray> &got = s.parts;
        for (auto it = parts.cbegin(); it != parts.cend(); ++it) {
            if (it.key().startsWith(QLatin1String("ObjectReplacements/"))) continue;
            QVERIFY2(got.contains(it.key()), qPrintable(it.key()));
            QVERIFY2(got.value(it.key()) == it.value(), qPrintable(it.key()));
        }
        const QMap<QString, QString> gotTypes = manifestTypesOf(got);
        for (auto it = types.cbegin(); it != types.cend(); ++it) QVERIFY2(gotTypes.value(it.key()) == it.value(), qPrintable(it.key()));
        // (the manifest lists exactly the parts written)
        for (auto it = gotTypes.cbegin(); it != gotTypes.cend(); ++it)
            if (it.key() != QLatin1String("/") && !it.key().endsWith(QLatin1Char('/'))) QVERIFY2(got.contains(it.key()), qPrintable(it.key()));
        for (auto it = got.cbegin(); it != got.cend(); ++it)
            if (it.key() != QLatin1String("mimetype") && it.key() != QLatin1String("META-INF/manifest.xml")) QVERIFY2(gotTypes.contains(it.key()), qPrintable(it.key()));
        const QByteArray content = got.value(QStringLiteral("content.xml"));
        QVERIFY2(content.contains("<draw:object-ole xlink:href=\"./Object 1\"") && content.contains("<draw:object xlink:href=\"./Object 2\""), content.right(1500).constData());
        QCOMPARE(content.count("<draw:image "), 2);
        QVERIFY2(!s.report.notKept.join(QLatin1Char('|')).contains(QLatin1String("bject")), qPrintable(s.report.notKept.join(QLatin1Char('|'))));
        // Saved again: the same parts.
        Saved again;
        QVERIFY2(resaved(s.ods, &again, &why), qPrintable(why));
        QCOMPARE(QStringList(again.parts.keys()), QStringList(got.keys()));
        QCOMPARE(manifestTypesOf(again.parts), gotTypes);
        for (auto it = got.cbegin(); it != got.cend(); ++it)
            if (it.key().startsWith(QLatin1String("Object"))) QVERIFY2(again.parts.value(it.key()) == it.value(), qPrintable(it.key()));
    }

    // Past the count nothing more is inflated; the others are their pictures.
    void objectsOfASheetPastTheCountAreNotInflated()
    {
        LimitGuard guard;
        jp::office::ole::Limits l = jp::office::ole::limits();
        l.objects = 3;
        jp::office::ole::setLimits(l);
        QString frames;
        QMap<QString, QByteArray> parts;
        QMap<QString, QString> types;
        for (int i = 1; i <= 8; ++i) {
            frames += objectFrame(QStringLiteral("Object %1").arg(i), true, i, QStringLiteral("ObjectReplacements/Object %1.png").arg(i));
            parts.insert(QStringLiteral("Object %1").arg(i), blobOf(i, 100));
            types.insert(QStringLiteral("Object %1").arg(i), QStringLiteral("application/vnd.sun.star.oleobject"));
            parts.insert(QStringLiteral("ObjectReplacements/Object %1.png").arg(i), pngBytes(4, 4, QColor(i, 2, 3)));
        }
        const qint64 before = jp::office::ole::odfObjectTestHooks().objectReads.load();
        QString why;
        Read r;
        QVERIFY2(readOds(objectSheet(frames, parts, types), &r, &why), qPrintable(why));
        QCOMPARE(jp::office::ole::odfObjectTestHooks().objectReads.load() - before, qint64(3));
        QCOMPARE(r.sheet.importedDrawing.objects.size(), 3);
        QCOMPARE(r.drawing.objects.size(), 8);
        QVERIFY2(r.sheet.importedDrawing.notRead.join(QLatin1Char('|')).contains(QLatin1String("embedded objects past")), qPrintable(r.sheet.importedDrawing.notRead.join(QLatin1Char('|'))));
    }

    // ... and past the bytes.
    void objectsOfASheetPastTheBytesAreNotInflated()
    {
        LimitGuard guard;
        jp::office::ole::Limits l = jp::office::ole::limits();
        l.totalBytes = 5000;
        l.objectBytes = 4000;
        jp::office::ole::setLimits(l);
        QString frames;
        QMap<QString, QByteArray> parts;
        QMap<QString, QString> types;
        for (int i = 1; i <= 7; ++i) {
            frames += objectFrame(QStringLiteral("Object %1").arg(i), true, i, QStringLiteral("ObjectReplacements/Object 1.png"));
            parts.insert(QStringLiteral("Object %1").arg(i), blobOf(i, i == 7 ? 4500 : 2000));
            types.insert(QStringLiteral("Object %1").arg(i), QStringLiteral("application/vnd.sun.star.oleobject"));
        }
        parts.insert(QStringLiteral("ObjectReplacements/Object 1.png"), pngBytes(4, 4, QColor(1, 2, 3)));
        const qint64 before = jp::office::ole::odfObjectTestHooks().objectReads.load();
        QString why;
        Read r;
        QVERIFY2(readOds(objectSheet(frames, parts, types), &r, &why), qPrintable(why));
        QCOMPARE(jp::office::ole::odfObjectTestHooks().objectReads.load() - before, qint64(2));
        QCOMPARE(r.sheet.importedDrawing.objects.size(), 2);
    }

    // A part a thousand frames name is read once.
    void anObjectAThousandFramesNameIsReadOnce()
    {
        QString frames;
        QMap<QString, QByteArray> parts;
        QMap<QString, QString> types;
        parts.insert(QStringLiteral("Object 1/content.xml"), "<?xml version=\"1.0\"?><office:document-content xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\"/>");
        types.insert(QStringLiteral("Object 1/"), QStringLiteral("application/vnd.oasis.opendocument.graphics"));
        parts.insert(QStringLiteral("ObjectReplacements/Object 1.png"), pngBytes(4, 4, QColor(1, 2, 3)));
        for (int i = 0; i < 1000; ++i) frames += objectFrame(QStringLiteral("Object 1"), false, i, QStringLiteral("ObjectReplacements/Object 1.png"));
        const qint64 before = jp::office::ole::odfObjectTestHooks().inflatedParts.load();
        QString why;
        Read r;
        QVERIFY2(readOds(objectSheet(frames, parts, types), &r, &why), qPrintable(why));
        QCOMPARE(jp::office::ole::odfObjectTestHooks().inflatedParts.load() - before, qint64(1));   // (counted where the part is really inflated)
        QCOMPARE(r.sheet.importedDrawing.objects.size(), 1);   // (one package, named by every picture)
        QCOMPARE(r.drawing.objects.size(), 1000);
    }

    // An object folder with entries that climb out of it writes nothing outside it.
    void aHostileObjectFolderOfASheetWritesNothingOutsideIt()
    {
        QMap<QString, QByteArray> parts;
        QMap<QString, QString> types;
        parts.insert(QStringLiteral("Object 1/content.xml"), "<x/>");
        parts.insert(QStringLiteral("Object 1/../../evil.xml"), "evil");
        parts.insert(QStringLiteral("Object 1/..\\evil2.xml"), "evil");
        types.insert(QStringLiteral("Object 1/"), QStringLiteral("application/vnd.oasis.opendocument.graphics"));
        parts.insert(QStringLiteral("ObjectReplacements/Object 1.png"), pngBytes(4, 4, QColor(1, 2, 3)));
        QString why;
        Read r;
        QVERIFY2(readOds(objectSheet(objectFrame(QStringLiteral("Object 1"), false, 0, QStringLiteral("ObjectReplacements/Object 1.png")), parts, types), &r, &why), qPrintable(why));
        Saved s;
        QVERIFY2(resaved(objectSheet(objectFrame(QStringLiteral("Object 1"), false, 0, QStringLiteral("ObjectReplacements/Object 1.png")), parts, types), &s, &why), qPrintable(why));
        QVERIFY(s.parts.contains(QStringLiteral("Object 1/content.xml")));
        for (const QString &n : s.parts.keys()) {
            QVERIFY2(!n.contains(QLatin1String("..")) && !n.contains(QLatin1Char('\\')) && !n.startsWith(QLatin1Char('/')) && !n.contains(QLatin1String("evil")), qPrintable(n));
        }
    }

    // A file with no objects saves no part of an object.
    void aSheetWithoutObjectsSavesNoObjectParts()
    {
        const QByteArray png = pngBytes(8, 4, QColor(200, 30, 30));
        const QString anchors = twoCell(1, 2, 4, 6, pictureXml(2, QStringLiteral("rId1"), QStringLiteral("Logo"), QStringLiteral("a red logo")), QString(), 0, 0, 0, 0);
        Saved s;
        QString why;
        QVERIFY2(saveAsOds(xlsxWith(anchors, {imageTarget(QStringLiteral("rId1"), 1, png)}), &s, &why), qPrintable(why));
        QStringList names = QStringList(s.parts.keys());
        names.erase(std::remove_if(names.begin(), names.end(), [](const QString &n) { return n.startsWith(QLatin1String("Pictures/")); }), names.end());
        QCOMPARE(names, (QStringList{QStringLiteral("META-INF/manifest.xml"), QStringLiteral("content.xml"), QStringLiteral("meta.xml"), QStringLiteral("mimetype"), QStringLiteral("settings.xml"), QStringLiteral("styles.xml")}));
    }

    // An object an OpenDocument file held that is not one of ours is written to an .xlsx as its picture alone: no part, no relationship, no extension that names it, and the save says so. One of ours
    // is an embedded package, and comes back whole.
    void anOdfObjectOfASheetIsOnlyItsPictureInAnXlsx()
    {
        QMap<QString, QByteArray> parts;
        QMap<QString, QString> types;
        twoObjectParts(&parts, &types);
        jp::ZipWriter inner;
        inner.add(QStringLiteral("a.txt"), "x", false);
        const QByteArray ownPackage = inner.finish();
        parts.insert(QStringLiteral("Object 3"), ownPackage);
        types.insert(QStringLiteral("Object 3"), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"));
        parts.insert(QStringLiteral("ObjectReplacements/Object 3.png"), pngBytes(4, 4, QColor(5, 5, 5)));
        const QByteArray ods = objectSheet(objectFrame(QStringLiteral("Object 1"), true, 0, QStringLiteral("ObjectReplacements/Object 1.png")) + objectFrame(QStringLiteral("Object 2"), false, 1, QStringLiteral("ObjectReplacements/Object 2.png")) +
                                               objectFrame(QStringLiteral("Object 3"), true, 2, QStringLiteral("ObjectReplacements/Object 3.png")), parts, types);
        Workbook wb;
        QString why;
        QVERIFY2(openBytes(ods, QStringLiteral("o.ods"), &wb, &why), qPrintable(why));
        QByteArray xlsx;
        QStringList notes;
        QVERIFY2(writeBaseXlsx(wb, &xlsx, &why, nullptr, &notes), qPrintable(why));
        QVERIFY2(notes.join(QLatin1Char('|')).contains(QLatin1String("kept as pictures")), qPrintable(notes.join(QLatin1Char('|'))));
        QMap<QString, QByteArray> got;
        QVERIFY(jp::readZip(xlsx, got));
        QStringList embeddings, media;
        for (const QString &n : got.keys()) {
            if (n.startsWith(QLatin1String("xl/embeddings/"))) embeddings << n;
            if (n.startsWith(QLatin1String("xl/media/"))) media << n;
        }
        QCOMPARE(embeddings, QStringList{QStringLiteral("xl/embeddings/object1.xlsx")});   // (the one of ours)
        QCOMPARE(got.value(QStringLiteral("xl/embeddings/object1.xlsx")), ownPackage);
        QCOMPARE(media.size(), 3);   // (the pictures: all three)
        const QByteArray rels = got.value(QStringLiteral("xl/drawings/_rels/drawing1.xml.rels")), drawing = got.value(QStringLiteral("xl/drawings/drawing1.xml")), types2 = got.value(QStringLiteral("[Content_Types].xml"));
        QCOMPARE(rels.count("/package\""), 1);
        QVERIFY(!rels.contains("oleObject") && !types2.contains("oleObject") && !types2.contains("Extension=\"bin\""));
        QCOMPARE(drawing.count("<jo:object "), 1);   // (the one of ours)
        QVERIFY(drawing.contains(jp::office::ole::progIdOf(jp::office::ole::Kind::Sheet).toLatin1()));
        // (the same through the save of the workbook)
        QByteArray viaSave;
        SaveReport report;
        QVERIFY2(saveXlsx(wb, SaveChanges(), &viaSave, &why, &report), qPrintable(why));
        QVERIFY2(report.notes.join(QLatin1Char('|')).contains(QLatin1String("kept as pictures")), qPrintable(report.notes.join(QLatin1Char('|'))));
        // (and read again, the object of ours is an object of the .ods again; the others are pictures)
        Saved s;
        QVERIFY2(saveAsOds(xlsx, &s, &why), qPrintable(why));
        const QByteArray content = s.parts.value(QStringLiteral("content.xml"));
        QCOMPARE(content.count("<draw:object-ole "), 1);
        QCOMPARE(content.count("<draw:object "), 0);
        QCOMPARE(s.parts.value(QStringLiteral("Object 1")), ownPackage);
        QCOMPARE(manifestTypesOf(s.parts).value(QStringLiteral("Object 1")), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"));
    }

    // Only a relationship to an embedded package or binary, to a part of the embeddings, names an object: one that points at a worksheet copies nothing.
    void anObjectRelationshipToAnotherPartCopiesNothing()
    {
        jp::ZipWriter inner;
        inner.add(QStringLiteral("a.txt"), "x", false);
        QMap<QString, QByteArray> parts;
        QMap<QString, QString> types;
        parts.insert(QStringLiteral("Object 1"), inner.finish());
        types.insert(QStringLiteral("Object 1"), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"));
        parts.insert(QStringLiteral("ObjectReplacements/Object 1.png"), pngBytes(4, 4, QColor(1, 2, 3)));
        Workbook wb;
        QString why;
        QVERIFY2(openBytes(objectSheet(objectFrame(QStringLiteral("Object 1"), true, 0, QStringLiteral("ObjectReplacements/Object 1.png")), parts, types), QStringLiteral("o.ods"), &wb, &why), qPrintable(why));
        QByteArray xlsx;
        QVERIFY2(writeBaseXlsx(wb, &xlsx, &why), qPrintable(why));
        QMap<QString, QByteArray> files;
        QVERIFY(jp::readZip(xlsx, files));
        const QString relsPart = QStringLiteral("xl/drawings/_rels/drawing1.xml.rels");
        const QByteArray rels = files.value(relsPart);
        QVERIFY(rels.contains("/package\" Target=\"../embeddings/object1.xlsx\""));
        // (the unchanged file gives the object back)
        Saved plain;
        QVERIFY2(saveAsOds(xlsx, &plain, &why), qPrintable(why));
        QVERIFY(plain.parts.contains(QStringLiteral("Object 1")));
        const QList<QPair<QByteArray, QByteArray>> edits = {{"/package\" Target=\"../embeddings/object1.xlsx\"", "/package\" Target=\"../worksheets/sheet1.xml\""},
                                                            {"/package\" Target=\"../embeddings/object1.xlsx\"", "/worksheet\" Target=\"../embeddings/object1.xlsx\""}};
        for (const auto &edit : edits) {
            QMap<QString, QByteArray> changed = files;
            QByteArray r = rels;
            r.replace(edit.first, edit.second);
            changed.insert(relsPart, r);
            jp::ZipWriter z;
            for (auto it = changed.cbegin(); it != changed.cend(); ++it) z.add(it.key(), it.value(), true);
            Saved s;
            QVERIFY2(saveAsOds(z.finish(), &s, &why), qPrintable(why));
            QVERIFY2(!s.parts.contains(QStringLiteral("Object 1")), qPrintable(QString::fromUtf8(edit.second)));
            QVERIFY(!s.parts.value(QStringLiteral("content.xml")).contains("<draw:object"));
        }
    }

    // A formula object that is refused is read once, whatever number of frames name it.
    void aRefusedFormulaObjectOfASheetIsReadOnce()
    {
        const QByteArray script = "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mi>x</mi><script>alert(1)</script></math>";
        QString frames;
        for (int i = 0; i < 1000; ++i) frames += formulaFrame(QStringLiteral("Object 1"), i);
        const qint64 before = odsdraw::testHooks().pictureReads.load();
        QString why;
        Read r;
        QVERIFY2(readOds(handWritten(frames, QString(), {{QStringLiteral("Object 1/content.xml"), script}}), &r, &why), qPrintable(why));
        QCOMPARE(odsdraw::testHooks().pictureReads.load() - before, qint64(1));
        QVERIFY(equationLinesOf(r).isEmpty());
    }

    // The count and the bytes are looked at before a document is inflated: 2,500 objects of their own inflate 2,000, and 20 of a megabyte inflate 16.
    void formulaObjectsOfASheetPastTheLimitsAreNotInflated()
    {
        jp::initCore();
        QString frames;
        QMap<QString, QByteArray> parts;
        for (int i = 1; i <= 2500; ++i) {
            frames += formulaFrame(QStringLiteral("Object %1").arg(i), i);
            parts.insert(QStringLiteral("Object %1/content.xml").arg(i), hello());
        }
        qint64 before = odsdraw::testHooks().pictureReads.load();
        QString why;
        Read r;
        QVERIFY2(readOds(handWritten(frames, QString(), parts), &r, &why), qPrintable(why));
        QCOMPARE(odsdraw::testHooks().pictureReads.load() - before, qint64(2000));
        QCOMPARE(equationLinesOf(r).size(), 2000);
        QByteArray big = hello();
        big.insert(big.size() - 7, "<!--" + QByteArray(999000, 'x') + "-->");
        frames.clear();
        parts.clear();
        for (int i = 1; i <= 20; ++i) {
            frames += formulaFrame(QStringLiteral("Object %1").arg(i), i);
            parts.insert(QStringLiteral("Object %1/content.xml").arg(i), big);
        }
        before = odsdraw::testHooks().pictureReads.load();
        Read heavy;
        QVERIFY2(readOds(handWritten(frames, QString(), parts), &heavy, &why), qPrintable(why));
        QCOMPARE(odsdraw::testHooks().pictureReads.load() - before, qint64((16 << 20) / big.size()));
    }

    // At most 2,000 formula objects are read and written, and 16 MB of their documents.
    void formulaObjectsOfASheetAreHeldToTheirLimits()
    {
        jp::initCore();
        QString why;
        QString frames;
        for (int i = 0; i < 2000 + 1; ++i) frames += formulaFrame(QStringLiteral("Object 1"), i);
        Read r;
        QVERIFY2(readOds(handWritten(frames, QString(), {{QStringLiteral("Object 1/content.xml"), hello()}, {QStringLiteral("ObjectReplacements/Object 1.png"), pngBytes(4, 4, QColor(1, 2, 3))}}), &r, &why), qPrintable(why));
        QCOMPARE(equationLinesOf(r).size(), 2000);
        // Written: 2,001 equations make 2,000 formula objects.
        Saved s;
        QVERIFY2(saveAsOds(xlsxWith(equationAnchors(2000 + 1)), &s, &why), qPrintable(why));
        QCOMPARE(formulaPartsOf(s.parts), 2000);
        // Read: documents of about a megabyte each: 16 fit in 16 MB.
        QByteArray big = hello();
        big.insert(big.size() - 7, "<!--" + QByteArray(999000, 'x') + "-->");
        QString many;
        for (int i = 0; i < 20; ++i) many += formulaFrame(QStringLiteral("Object 1"), i);
        Read heavy;
        QVERIFY2(readOds(handWritten(many, QString(), {{QStringLiteral("Object 1/content.xml"), big}, {QStringLiteral("ObjectReplacements/Object 1.png"), pngBytes(4, 4, QColor(1, 2, 3))}}), &heavy, &why), qPrintable(why));
        QCOMPARE(equationLinesOf(heavy).size(), int((16 << 20) / big.size()));
    }

    // A chart in an .ods is still not read: the reader says so, and the writer says the copy has none.
    void chartsAreStillNamedAsNotRead()
    {
        const QString shapes = QStringLiteral("<draw:frame draw:name=\"Chart\" svg:width=\"1in\" svg:height=\"1in\" svg:x=\"1in\" svg:y=\"1in\"><draw:object xlink:href=\"./Object 1\"/></draw:frame>");
        Workbook wb;
        QString why;
        QVERIFY2(openBytes(handWritten(shapes), QStringLiteral("c.ods"), &wb, &why), qPrintable(why));
        QVERIFY(wb.skippedFeatures().contains(QStringLiteral("charts")));
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QVERIFY(s.importedDrawing.anchors.isEmpty());
    }

    // Hostile: more objects than a sheet may have are capped (and named); a picture past the limit, a missing picture, numbers that are not numbers, and groups nested without end are left out; the
    // cells of the sheet are read all the same.
    void hugeAndBrokenDrawingsAreCappedNotCrashed()
    {
        QString many;
        for (int i = 0; i < kMaxDrawingAnchors + 500; ++i)
            many += QStringLiteral("<draw:custom-shape draw:name=\"s%1\" svg:width=\"0.1in\" svg:height=\"0.1in\" svg:x=\"%2in\" svg:y=\"0in\"><draw:enhanced-geometry draw:type=\"rectangle\"/></draw:custom-shape>").arg(i).arg(i % 9);
        {
            Read r;
            QString why;
            QVERIFY2(readOds(handWritten(many), &r, &why), qPrintable(why));
            QCOMPARE(r.sheet.importedDrawing.anchors.size(), kMaxDrawingAnchors);
            QVERIFY(!r.sheet.importedDrawing.notRead.isEmpty());
        }
        const QString broken = QStringLiteral("<draw:frame draw:name=\"NoPicture\" svg:width=\"1in\" svg:height=\"1in\"><draw:image xlink:href=\"Pictures/missing.png\"/></draw:frame>"
                                              "<draw:frame draw:name=\"Empty\" svg:width=\"1in\" svg:height=\"1in\"/>"
                                              "<draw:custom-shape draw:name=\"Bad\" svg:width=\"abc\" svg:height=\"-5in\" svg:x=\"1e99in\" svg:y=\"NaN\" draw:transform=\"rotate (zzz)\"><draw:enhanced-geometry/></draw:custom-shape>")
                               ;
        {
            Workbook wb;
            QString why;
            QVERIFY2(openBytes(handWritten(broken), QStringLiteral("b.ods"), &wb, &why), qPrintable(why));
            Sheet s;
            QVERIFY2(wb.readSheet(0, &s), qPrintable(s.error));
            QVERIFY(s.importedDrawing.anchors.size() <= 4);
        }
        {
            // groups nested past the most a group may be (16), and well inside what the document may nest: the cells are read, the shape is left out, and the notice says why
            QString deep;
            for (int i = 0; i < 40; ++i) deep += QStringLiteral("<draw:g>");
            deep += QStringLiteral("<draw:custom-shape svg:width=\"1in\" svg:height=\"1in\"><draw:enhanced-geometry draw:type=\"rectangle\"/></draw:custom-shape>");
            for (int i = 0; i < 40; ++i) deep += QStringLiteral("</draw:g>");
            Workbook wb;
            QString why;
            const QString rows = QStringLiteral("<table:table-row><table:table-cell office:value-type=\"float\" office:value=\"42\"/></table:table-row>");
            QVERIFY2(openBytes(handWritten(deep, rows), QStringLiteral("deep.ods"), &wb, &why), qPrintable(why));
            Sheet s;
            QVERIFY2(wb.readSheet(0, &s), qPrintable(s.error));
            QCOMPARE(shown(wb, s, QStringLiteral("A1")), QStringLiteral("n:42"));
            QCOMPARE(s.importedDrawing.anchors.size(), 0);
            QVERIFY(s.importedDrawing.notRead.join(QLatin1Char('|')).contains(QLatin1String("nested deeper")));
        }
        {
            // nesting past what a document may have is refused in words
            QString deep;
            for (int i = 0; i < 400; ++i) deep += QStringLiteral("<draw:g>");
            for (int i = 0; i < 400; ++i) deep += QStringLiteral("</draw:g>");
            Workbook wb;
            QString why;
            QVERIFY2(openBytes(handWritten(deep), QStringLiteral("deeper.ods"), &wb, &why), qPrintable(why));
            Sheet s;
            QVERIFY(!wb.readSheet(0, &s));
            QVERIFY(!s.error.isEmpty());
        }
        {
            // a picture past the most a picture may be
            QByteArray huge(int(odsdraw::kMaxPictureBytes) + 1024, '\0');
            const QString shapes = QStringLiteral("<draw:frame draw:name=\"Huge\" svg:width=\"1in\" svg:height=\"1in\"><draw:image xlink:href=\"Pictures/huge.png\"/></draw:frame>");
            Read r;
            QString why;
            QVERIFY2(readOds(handWritten(shapes, QString(), {{QStringLiteral("Pictures/huge.png"), huge}}), &r, &why), qPrintable(why));
            QCOMPARE(r.sheet.importedDrawing.media.size(), 0);
            QVERIFY(!r.sheet.importedDrawing.notRead.isEmpty());
        }
        {
            // the words of one object are cut
            const QString text(odsdraw::kMaxTextChars + 5000, QLatin1Char('x'));
            const QString shapes = QStringLiteral("<draw:frame draw:name=\"Long\" svg:width=\"1in\" svg:height=\"1in\"><draw:text-box><text:p>%1</text:p></draw:text-box></draw:frame>").arg(text);
            Read r;
            QString why;
            QVERIFY2(readOds(handWritten(shapes), &r, &why), qPrintable(why));
            QCOMPARE(r.drawing.objects.size(), 1);
            QVERIFY(textOf(child(r.drawing.objects[0].element, "txBody")).size() <= odsdraw::kMaxTextChars);
            QVERIFY(textOf(child(r.drawing.objects[0].element, "txBody")).size() > 1000);
        }
    }

    // A drawing a save would write too large is cut at the same limits: a picture past the most a picture may be is left out and named, and the objects around it are written.
    void aSaveWritesNoMoreThanTheLimits()
    {
        const QByteArray png = pngBytes(4, 4, QColor(9, 9, 9));
        QByteArray huge = png;
        huge += QByteArray(int(odsdraw::kMaxPictureBytes) + 1024, '\0');
        QString anchors = twoCell(0, 0, 2, 1, pictureXml(2, QStringLiteral("rId1"), QStringLiteral("Big"), QStringLiteral("too big")));
        for (int i = 0; i < 30; ++i) anchors += twoCell(0, i + 2, 2, i + 3, shapeElement(i + 3, QStringLiteral("S%1").arg(i)));
        Saved s;
        QString why;
        QVERIFY2(saveAsOds(xlsxWith(anchors, {imageTarget(QStringLiteral("rId1"), 1, huge)}), &s, &why), qPrintable(why));
        QVERIFY2(s.report.notKept.join(QLatin1Char('|')).contains(QLatin1String("pictures larger than 32 MB")), qPrintable(s.report.notKept.join(QLatin1Char('|'))));
        for (auto it = s.parts.cbegin(); it != s.parts.cend(); ++it) QVERIFY(!it.key().startsWith(QLatin1String("Pictures/")));
        Read r;
        QVERIFY2(readOds(s.ods, &r, &why), qPrintable(why));
        QCOMPARE(r.drawing.objects.size(), 30);
        QCOMPARE(r.drawing.objects[29].name, QStringLiteral("S29"));
    }

    // Rotated objects keep where they are: the box's place and size come back from the turn the standard writes, in each kind of anchor.
    void aTurnedObjectKeepsItsPlace()
    {
        QString shape = shapeElement(2, QStringLiteral("Tilted"));
        shape.replace(QStringLiteral("<a:xfrm>"), QStringLiteral("<a:xfrm rot=\"2700000\">"));
        const QString anchors = oneCell(3, 4, 1828800, 914400, shape, 12345, 6789) + twoCell(1, 1, 3, 3, shape.replace(QStringLiteral("Tilted"), QStringLiteral("Tilted2")), QString(), 1000, 2000, 3000, 4000);
        Saved s;
        QString why;
        QVERIFY2(saveAsOds(xlsxWith(anchors), &s, &why), qPrintable(why));
        Read r;
        QVERIFY2(readOds(s.ods, &r, &why), qPrintable(why));
        QCOMPARE(r.drawing.objects.size(), 2);
        const DrawingObject one = r.drawing.objects[0], two = r.drawing.objects[1];
        QCOMPARE(int(one.anchor.from.col), 3);
        QCOMPARE(int(one.anchor.from.row), 4);
        QVERIFY(std::abs(one.anchor.from.colOff - 12345) <= 3);
        QVERIFY(std::abs(one.anchor.from.rowOff - 6789) <= 3);
        QVERIFY(std::abs(one.anchor.cx - 1828800) <= 3);
        QVERIFY(std::abs(one.anchor.cy - 914400) <= 3);
        QVERIFY(std::abs(rotationOf(one) - 2700000) <= 60);
        QCOMPARE(int(two.anchor.from.col), 1);
        QCOMPARE(int(two.anchor.from.row), 1);
        QVERIFY(std::abs(two.anchor.from.colOff - 1000) <= 3);
        QVERIFY(std::abs(two.anchor.from.rowOff - 2000) <= 3);
        QCOMPARE(int(two.anchor.to.col), 3);
        QCOMPARE(int(two.anchor.to.row), 3);
        QVERIFY(std::abs(rotationOf(two) - 2700000) <= 60);
    }

    // The members of a group count against the limits on objects like any other object, at any depth.
    void groupMembersCountAgainstTheLimits()
    {
        QString members;
        for (int i = 0; i < kMaxDrawingAnchors + 100; ++i) members += QStringLiteral("<draw:frame svg:width=\"0.1in\" svg:height=\"0.1in\" svg:x=\"0in\" svg:y=\"0in\"><draw:text-box/></draw:frame>");
        Read r;
        QString why;
        QVERIFY2(readOds(handWritten(QStringLiteral("<draw:g>") + members + QStringLiteral("</draw:g>")), &r, &why), qPrintable(why));
        QCOMPARE(r.drawing.objects.size(), 1);
        QVERIFY(r.drawing.objects[0].element.elementsByTagNameNS(kXdr, QStringLiteral("sp")).size() <= kMaxDrawingAnchors);
        QVERIFY(r.sheet.importedDrawing.notRead.join(QLatin1Char('|')).contains(QLatin1String("past the")));
        // (and one group in another: the inner members count too)
        Read r2;
        QVERIFY2(readOds(handWritten(QStringLiteral("<draw:g><draw:g>") + members + QStringLiteral("</draw:g></draw:g>")), &r2, &why), qPrintable(why));
        QVERIFY(r2.drawing.objects.isEmpty() || r2.drawing.objects[0].element.elementsByTagNameNS(kXdr, QStringLiteral("sp")).size() <= kMaxDrawingAnchors);
    }

    // Letters outside the Basic Multilingual Plane (an emoji, a rare Chinese character) are kept in a shape, a text box, and Text Art.
    void lettersOutsideTheBasicPlaneSurvive()
    {
        const QString words = QString::fromUtf8("a\xF0\x9F\x98\x80" "b\xF0\xA0\x80\x80" "c");
        const QString anchors = twoCell(0, 0, 3, 3, shapeElement(2, QStringLiteral("Shape"), QStringLiteral("rect"), words)) + twoCell(0, 4, 3, 6, textBoxXml(3, QStringLiteral("Box"), words))
                                + twoCell(4, 0, 8, 3, textArtXml(4, QStringLiteral("Art"), QStringLiteral("textPlain"), words));
        Saved s;
        QString why;
        QVERIFY2(saveAsOds(xlsxWith(anchors), &s, &why), qPrintable(why));
        QVERIFY(s.parts.value(QStringLiteral("content.xml")).contains(words.toUtf8()));
        Read r;
        QVERIFY2(readOds(s.ods, &r, &why), qPrintable(why));
        QCOMPARE(r.drawing.objects.size(), 3);
        for (const DrawingObject &o : std::as_const(r.drawing.objects)) QCOMPARE(textOf(child(o.element, "txBody")), words);
    }

    // A column of a size no sheet has (1e30 points) is kept to a sane one, and a shape still lands in a cell.
    void anAbsurdColumnWidthIsKeptToWhatASheetHolds()
    {
        const QString styles = QStringLiteral("<style:style style:name=\"co9\" style:family=\"table-column\"><style:table-column-properties style:column-width=\"1e30pt\"/></style:style>"
                                              "<style:style style:name=\"ro9\" style:family=\"table-row\"><style:table-row-properties style:row-height=\"1e30pt\"/></style:style>");
        const QString shape = QStringLiteral("<draw:custom-shape draw:name=\"Far\" svg:width=\"1in\" svg:height=\"1in\" svg:x=\"3in\" svg:y=\"2in\" draw:anchor-type=\"cell\"><draw:enhanced-geometry draw:type=\"rectangle\"/></draw:custom-shape>");
        const QString body = QStringLiteral("<table:table table:name=\"S\"><table:shapes>%1</table:shapes><table:table-column table:style-name=\"co9\"/><table:table-column table:number-columns-repeated=\"5\"/>"
                                            "<table:table-row table:style-name=\"ro9\"><table:table-cell/></table:table-row><table:table-row><table:table-cell/></table:table-row></table:table>").arg(shape);
        Workbook wb;
        QString why;
        QVERIFY2(openBytes(buildOds(contentXml(body, styles)), QStringLiteral("wide.ods"), &wb, &why), qPrintable(why));
        Sheet s;
        QVERIFY2(wb.readSheet(0, &s), qPrintable(s.error));
        QCOMPARE(s.importedDrawing.anchors.size(), 1);
        QByteArray xml;
        for (const QByteArray &a : s.importedDrawing.anchors) xml += a;
        const std::shared_ptr<QDomDocument> parsed = parseDrawing(newDrawingXml(xml), &why);
        QVERIFY(parsed);
        SheetDrawing d;
        QVERIFY(readDrawing(Package(), QStringLiteral("/d.xml"), &d, &why, parsed));
        QCOMPARE(d.objects.size(), 1);
        QCOMPARE(int(d.objects[0].anchor.from.col), 0);   // (the first column is wider than 3 inches, whatever it was written as)
        QCOMPARE(int(d.objects[0].anchor.from.row), 0);
        QVERIFY(d.objects[0].anchor.from.colOff >= 0 && d.objects[0].anchor.from.colOff < (qint64(1) << 40));
    }

    // A picture whose Base 64 is cut short is refused, not decoded as far as it goes; one inside more characters than a sheet's shapes may hold is left out, and both are named.
    void aTruncatedInlinePictureIsRefused()
    {
        const QByteArray png = pngBytes(20, 20, QColor(5, 6, 7));
        const QByteArray whole = png.toBase64();
        auto frame = [](const QByteArray &data) {
            return QStringLiteral("<draw:frame svg:width=\"1in\" svg:height=\"1in\"><draw:image><office:binary-data>%1</office:binary-data></draw:image></draw:frame>").arg(QString::fromLatin1(data));
        };
        Read good;
        QString why;
        QVERIFY2(readOds(handWritten(frame(whole)), &good, &why), qPrintable(why));
        QCOMPARE(good.sheet.importedDrawing.media.size(), 1);
        Read cut;
        QVERIFY2(readOds(handWritten(frame(whole.left(whole.size() - 3))), &cut, &why), qPrintable(why));
        QCOMPARE(cut.sheet.importedDrawing.media.size(), 0);
        QVERIFY(!cut.sheet.importedDrawing.notRead.isEmpty());
        // (a sheet's shapes may hold only so many characters: the rest of an element is not kept, and a picture in it is left out)
        const qint64 before = odsdraw::testHooks().nodeCharLimit.exchange(2000);
        Read over;
        const bool ok = readOds(handWritten(frame(whole.repeated(1) + QByteArray(3000, 'A'))), &over, &why);
        odsdraw::testHooks().nodeCharLimit = before;
        QVERIFY2(ok, qPrintable(why));
        QCOMPARE(over.sheet.importedDrawing.media.size(), 0);
        QVERIFY(over.sheet.importedDrawing.notRead.join(QLatin1Char('|')).contains(QLatin1String("past the limits")));
    }

    // The bytes of an element's attributes are charged to the budget of characters like its text is.
    void attributesAreChargedToTheBudget()
    {
        const qint64 before = odsdraw::testHooks().nodeCharLimit.exchange(2000);
        const QString shape = QStringLiteral("<draw:custom-shape draw:name=\"%1\" svg:width=\"1in\" svg:height=\"1in\"><draw:enhanced-geometry draw:type=\"rectangle\"/></draw:custom-shape>").arg(QString(5000, QLatin1Char('n')));
        Read r;
        QString why;
        const bool ok = readOds(handWritten(shape), &r, &why);
        odsdraw::testHooks().nodeCharLimit = before;
        QVERIFY2(ok, qPrintable(why));
        QVERIFY(r.sheet.importedDrawing.notRead.join(QLatin1Char('|')).contains(QLatin1String("past the limits")));
    }

    // A sheet with no default height of its rows has the standard's (0.178 inch), so a shape 17.8 inches down is in row 101.
    void rowsWithNoHeightAreTheStandardsDefault()
    {
        const QString shape = QStringLiteral("<draw:custom-shape draw:name=\"Low\" svg:width=\"1in\" svg:height=\"1in\" svg:x=\"0in\" svg:y=\"17.8in\" draw:anchor-type=\"cell\"><draw:enhanced-geometry draw:type=\"rectangle\"/></draw:custom-shape>");
        const QString rows = QStringLiteral("<table:table-row table:number-rows-repeated=\"200\"><table:table-cell table:number-columns-repeated=\"10\"/></table:table-row>");
        Read r;
        QString why;
        QVERIFY2(readOds(handWritten(shape, rows), &r, &why), qPrintable(why));
        QCOMPARE(r.drawing.objects.size(), 1);
        QCOMPARE(int(r.drawing.objects[0].anchor.from.row), 100);
    }

    // A picture that many frames use is read from the package, and made, once.
    void aPictureManyFramesUseIsReadOnce()
    {
        const QByteArray png = pngBytes(8, 8, QColor(7, 7, 7));
        QString frames;
        for (int i = 0; i < 5; ++i) frames += QStringLiteral("<draw:frame draw:name=\"F%1\" svg:width=\"1in\" svg:height=\"1in\" svg:x=\"%1in\" svg:y=\"0in\"><draw:image xlink:href=\"Pictures/same.png\"/></draw:frame>").arg(i);
        const qint64 reads = odsdraw::testHooks().pictureReads.load();
        Read r;
        QString why;
        QVERIFY2(readOds(handWritten(frames, QString(), {{QStringLiteral("Pictures/same.png"), png}}), &r, &why), qPrintable(why));
        QCOMPARE(r.drawing.objects.size(), 5);
        QCOMPARE(r.sheet.importedDrawing.media.size(), 1);
        QCOMPARE(odsdraw::testHooks().pictureReads.load() - reads, qint64(1));
        // (and the same on the way out: one picture a package holds that five objects use is hashed once)
        QString anchors;
        for (int i = 0; i < 5; ++i) anchors += twoCell(0, i * 2, 2, i * 2 + 1, pictureXml(i + 2, QStringLiteral("rId1"), QStringLiteral("P%1").arg(i), QStringLiteral("same")));
        const qint64 hashes = odsdraw::testHooks().pictureHashes.load();
        Saved s;
        QVERIFY2(saveAsOds(xlsxWith(anchors, {imageTarget(QStringLiteral("rId1"), 1, png)}), &s, &why), qPrintable(why));
        QCOMPARE(odsdraw::testHooks().pictureHashes.load() - hashes, qint64(1));
        int pictures = 0;
        for (auto it = s.parts.cbegin(); it != s.parts.cend(); ++it) pictures += it.key().startsWith(QLatin1String("Pictures/")) ? 1 : 0;
        QCOMPARE(pictures, 1);
    }

    // Saving a workbook opened from an .ods reads each sheet once.
    void aSaveReadsEachSheetOnce()
    {
        const QByteArray png = pngBytes(8, 8, QColor(3, 3, 3));
        const QString shapes = QStringLiteral("<draw:frame draw:name=\"Pic\" svg:width=\"1in\" svg:height=\"1in\"><draw:image xlink:href=\"Pictures/p.png\"/></draw:frame>");
        Workbook wb;
        QString why;
        QVERIFY2(openBytes(handWritten(shapes, QString(), {{QStringLiteral("Pictures/p.png"), png}}), QStringLiteral("once.ods"), &wb, &why), qPrintable(why));
        const qint64 reads = odsdraw::testHooks().sheetReads.load();
        QByteArray xlsx;
        QVERIFY2(saveXlsx(wb, SaveChanges(), &xlsx, &why), qPrintable(why));
        QCOMPARE(odsdraw::testHooks().sheetReads.load() - reads, qint64(1));
    }

    // Sheets read on several threads can't take more than the workbook's limit between them.
    void theBudgetIsTakenInOneStep()
    {
        odsdraw::Budget budget;
        std::atomic<qint64> got{0};
        QVector<QThread *> threads;
        for (int t = 0; t < 8; ++t) {
            QThread *th = QThread::create([&] {
                for (int i = 0; i < 2000; ++i) {
                    if (budget.takeObject(1000)) ++got;
                    if (budget.takeBytes(1, 1000)) ++got;
                }
            });
            threads << th;
            th->start();
        }
        for (QThread *th : std::as_const(threads)) {
            th->wait();
            delete th;
        }
        QCOMPARE(got.load(), qint64(2000));
        QCOMPARE(budget.objects.load(), qint64(1000));
        QCOMPARE(budget.pictureBytes.load(), qint64(1000));
    }

    // A crop with room around the picture (negative insets), and a crop on a picture whose size isn't known, are named in the report and not dropped without a word.
    void cropsThatAreNotKeptAreNamed()
    {
        const QByteArray png = pngBytes(8, 8, QColor(1, 1, 1));
        Saved s;
        QString why;
        QVERIFY2(saveAsOds(xlsxWith(twoCell(0, 0, 2, 2, pictureXml(2, QStringLiteral("rId1"), QStringLiteral("Room"), QStringLiteral("room"), 0, QStringLiteral("<a:srcRect l=\"-10000\" t=\"-5000\"/>"))), {imageTarget(QStringLiteral("rId1"), 1, png)}), &s, &why), qPrintable(why));
        QVERIFY2(s.report.notKept.join(QLatin1Char('|')).contains(QLatin1String("room around")), qPrintable(s.report.notKept.join(QLatin1Char('|'))));
        const QByteArray svg = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"10\" height=\"10\"><rect width=\"10\" height=\"10\"/></svg>";
        Saved t;
        const DrawingTarget target{QStringLiteral("rId1"), QStringLiteral("xl/media/image1.svg"), svg, QStringLiteral("image/svg+xml"), QStringLiteral("image")};
        QVERIFY2(saveAsOds(xlsxWith(twoCell(0, 0, 2, 2, pictureXml(2, QStringLiteral("rId1"), QStringLiteral("Vec"), QStringLiteral("vec"), 0, QStringLiteral("<a:srcRect l=\"10000\"/>"))), {target}), &t, &why), qPrintable(why));
        QVERIFY2(t.report.notKept.join(QLatin1Char('|')).contains(QLatin1String("crop")), qPrintable(t.report.notKept.join(QLatin1Char('|'))));
    }
};

JP_TEST_CLASS(SheetOdsDrawTests)
#include "tst_sheet_odsdraw.moc"
