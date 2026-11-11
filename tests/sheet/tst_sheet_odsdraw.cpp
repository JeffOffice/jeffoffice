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
#include "sheet/xlsxsave.h"

#include <QDomDocument>
#include <QThread>

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
