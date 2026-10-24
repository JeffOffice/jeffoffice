// Tests of the drawing part of a sheet (libs/office/sheet/drawing.h): the three kinds of anchor with their editAs, what each anchor holds (a chart, a picture, a shape, a text box, a group,
// a connector), a chart sheet's drawing, and the hostile forms. Every workbook is built in memory.
#include "drawinghelpers.h"

#include "sheet/drawing.h"

using namespace drawtest;

namespace {

struct Opened {
    Package package;
    Workbook book;
    QString sheetPart;
    QString drawingPart;
};

bool open(Opened *o, const QByteArray &xlsx, QString *why)
{
    if (!o->package.readLazy(xlsx, why)) return false;
    if (!o->book.open(o->package, why)) return false;
    o->sheetPart = o->book.info().sheets.value(0).part;
    o->drawingPart = drawingPartOf(o->package, o->sheetPart);
    return true;
}

QByteArray simpleChart() { return chartPart(QStringLiteral("barChart"), {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {"Q1", "Q2", "Q3"}, {10, 20, 30}, QString()}}); }

} // namespace

class SheetDrawingTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // The three anchors, each with the place it says and what editAs makes of it.
    void readsTwoCellOneCellAndAbsoluteAnchors()
    {
        const QString anchors = twoCell(1, 2, 4, 9, shapeElement(2, QStringLiteral("Two")), QStringLiteral("oneCell"), 100, 200, 300, 400) + oneCell(3, 5, 1905000, 952500, shapeElement(3, QStringLiteral("One")), 10, 20)
                                + absolute(952500, 1905000, 2857500, 1428750, shapeElement(4, QStringLiteral("Abs"))) + twoCell(0, 0, 2, 2, shapeElement(5, QStringLiteral("Plain")))
                                + twoCell(0, 0, 1, 1, shapeElement(6, QStringLiteral("Fixed")), QStringLiteral("absolute"));
        Opened o;
        QString why;
        QVERIFY2(open(&o, bookWithDrawing({dataSheet()}, drawingPart(anchors), {}), &why), qPrintable(why));
        QVERIFY(!o.drawingPart.isEmpty());
        SheetDrawing d;
        QVERIFY2(readDrawing(o.package, o.drawingPart, &d, &why), qPrintable(why));
        QCOMPARE(d.objects.size(), 5);
        QCOMPARE(d.anchorsInPart, 5);
        const DrawingObject &two = d.objects[0];
        QCOMPARE(int(two.anchor.kind), int(AnchorKind::TwoCell));
        QCOMPARE(int(two.anchor.editAs), int(AnchorKind::OneCell));
        QCOMPARE(int(two.anchor.from.col), 1);
        QCOMPARE(int(two.anchor.from.row), 2);
        QCOMPARE(two.anchor.from.colOff, qint64(100));
        QCOMPARE(two.anchor.from.rowOff, qint64(200));
        QCOMPARE(int(two.anchor.to.col), 4);
        QCOMPARE(int(two.anchor.to.row), 9);
        QCOMPARE(two.anchor.to.colOff, qint64(300));
        QCOMPARE(two.anchor.to.rowOff, qint64(400));
        QCOMPARE(two.name, QStringLiteral("Two"));
        QCOMPARE(two.id, qint64(2));
        const DrawingObject &one = d.objects[1];
        QCOMPARE(int(one.anchor.kind), int(AnchorKind::OneCell));
        QCOMPARE(int(one.anchor.editAs), int(AnchorKind::OneCell));
        QCOMPARE(int(one.anchor.from.col), 3);
        QCOMPARE(one.anchor.cx, qint64(1905000));
        QCOMPARE(one.anchor.cy, qint64(952500));
        const DrawingObject &abs = d.objects[2];
        QCOMPARE(int(abs.anchor.kind), int(AnchorKind::Absolute));
        QCOMPARE(abs.anchor.x, qint64(952500));
        QCOMPARE(abs.anchor.y, qint64(1905000));
        QCOMPARE(abs.anchor.cx, qint64(2857500));
        QCOMPARE(int(d.objects[3].anchor.editAs), int(AnchorKind::TwoCell));   // (no editAs: moves and sizes with the cells)
        QCOMPARE(int(d.objects[4].anchor.editAs), int(AnchorKind::Absolute));
        // (the order is the file's: the first is at the back)
        for (int i = 0; i < d.objects.size(); ++i) QCOMPARE(d.objects[i].index, i);
    }

    // What an anchor holds: the chart's part is found through the drawing's relationships; a picture, a shape with text, a group, and a connector are read for what they are.
    void findsEachKindOfObject()
    {
        const QString anchors = twoCell(0, 0, 3, 8, chartFrame(2, QStringLiteral("rIdC"), QStringLiteral("Sales chart"))) + twoCell(4, 0, 6, 3, pictureElement(3, QStringLiteral("rIdP"), QStringLiteral("Logo")))
                                + twoCell(0, 10, 2, 12, shapeElement(4, QStringLiteral("Box"), QStringLiteral("rect"), QStringLiteral("Hello"))) + twoCell(3, 10, 5, 12, groupElement(5, shapeElement(6, QStringLiteral("In")), 1905000, 476250))
                                + twoCell(0, 14, 2, 16, connectorElement(7));
        Opened o;
        QString why;
        QVERIFY2(open(&o, bookWithDrawing({dataSheet()}, drawingPart(anchors), {chartTarget(QStringLiteral("rIdC"), 1, simpleChart()), imageTarget(QStringLiteral("rIdP"), 1, pngBytes(8, 4, Qt::red))}), &why), qPrintable(why));
        SheetDrawing d;
        QVERIFY2(readDrawing(o.package, o.drawingPart, &d, &why), qPrintable(why));
        QCOMPARE(d.objects.size(), 5);
        QCOMPARE(int(d.objects[0].kind), int(ObjectKind::Chart));
        QCOMPARE(d.objects[0].chartPart, QStringLiteral("/xl/charts/chart1.xml"));
        QCOMPARE(d.objects[0].name, QStringLiteral("Sales chart"));
        QCOMPARE(int(d.objects[1].kind), int(ObjectKind::Picture));
        QCOMPARE(d.objects[1].description, QStringLiteral("a picture"));
        QCOMPARE(int(d.objects[2].kind), int(ObjectKind::Shape));
        QCOMPARE(int(d.objects[3].kind), int(ObjectKind::Group));
        QCOMPARE(int(d.objects[4].kind), int(ObjectKind::Connector));
        for (const DrawingObject &obj : d.objects) QVERIFY(!obj.element.isNull());
        // each element is the object's own
        QCOMPARE(d.objects[1].element.localName(), QStringLiteral("pic"));
        QCOMPARE(d.objects[3].element.localName(), QStringLiteral("grpSp"));
    }

    // An anchor that holds a table or a slicer, a chart the drawing has no relationship for, or nothing is counted and left out; a hidden object is read (it says it is hidden).
    void leavesOutWhatIsNotRead()
    {
        const QString table = QStringLiteral("<xdr:graphicFrame><xdr:nvGraphicFramePr><xdr:cNvPr id=\"9\" name=\"T\"/><xdr:cNvGraphicFramePr/></xdr:nvGraphicFramePr><xdr:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"1\" cy=\"1\"/></xdr:xfrm>"
                                              "<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/table\"/></a:graphic></xdr:graphicFrame>");
        QString hidden = shapeElement(8, QStringLiteral("Hidden"));
        hidden.replace(QStringLiteral("name=\"Hidden\""), QStringLiteral("name=\"Hidden\" hidden=\"1\""));
        const QString anchors = twoCell(0, 0, 1, 1, table) + twoCell(0, 0, 1, 1, chartFrame(10, QStringLiteral("rIdNone"))) + twoCell(0, 0, 1, 1, QString()) + twoCell(0, 0, 1, 1, hidden)
                                + twoCell(0, 0, 1, 1, chartFrame(11, QStringLiteral("rIdC")));
        Opened o;
        QString why;
        QVERIFY2(open(&o, bookWithDrawing({dataSheet()}, drawingPart(anchors), {chartTarget(QStringLiteral("rIdC"), 1, simpleChart())}), &why), qPrintable(why));
        SheetDrawing d;
        QVERIFY2(readDrawing(o.package, o.drawingPart, &d, &why), qPrintable(why));
        QCOMPARE(d.anchorsInPart, 5);
        QCOMPARE(d.skipped, 3);
        QCOMPARE(d.objects.size(), 2);
        QCOMPARE(d.objects[0].name, QStringLiteral("Hidden"));
        QVERIFY(d.objects[0].hidden);
        QCOMPARE(int(d.objects[1].kind), int(ObjectKind::Chart));
    }

    // A drawing a newer program wrapped for older ones: the object of the branch that can be read.
    void readsAnchorsInAlternateContent()
    {
        const QString wrapped = QStringLiteral("<mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\"><mc:Choice Requires=\"a14\">%1</mc:Choice><mc:Fallback>%2</mc:Fallback></mc:AlternateContent>")
                                    .arg(twoCell(0, 0, 2, 2, shapeElement(2, QStringLiteral("Newer"))), twoCell(0, 0, 2, 2, shapeElement(2, QStringLiteral("Older"))));
        const QString anchors = twoCell(5, 5, 6, 6, shapeElement(3, QStringLiteral("Plain"))) + wrapped;
        Opened o;
        QString why;
        QVERIFY2(open(&o, bookWithDrawing({dataSheet()}, drawingPart(anchors), {}), &why), qPrintable(why));
        SheetDrawing d;
        QVERIFY2(readDrawing(o.package, o.drawingPart, &d, &why), qPrintable(why));
        QCOMPARE(d.objects.size(), 2);
        QCOMPARE(d.objects[1].name, QStringLiteral("Newer"));
        QCOMPARE(d.objects[0].ordinal, 0);
        QCOMPARE(d.objects[1].ordinal, 1);   // (the place of the wrapper among the part's children: what saving removes by)
    }

    // A chart sheet has a drawing of its own, named by the sheet part's relationships.
    void aChartSheetHasItsDrawing()
    {
        const QByteArray drawing = drawingPart(absolute(0, 0, 9525 * 640, 9525 * 480, chartFrame(2, QStringLiteral("rIdC"))));
        QVector<ExtraPart> parts;
        parts.append({QStringLiteral("xl/chartsheets/sheet1.xml"), QByteArray("<?xml version=\"1.0\"?><chartsheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"><drawing r:id=\"rIdD\"/></chartsheet>"),
                      QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.chartsheet+xml"), -1, QString(), QString(), QString()});
        parts.append({QStringLiteral("xl/chartsheets/_rels/sheet1.xml.rels"),
                      QByteArray("<?xml version=\"1.0\"?><Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\"><Relationship Id=\"rIdD\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/drawing\" Target=\"../drawings/drawing1.xml\"/></Relationships>"),
                      QString(), -1, QString(), QString(), QString()});
        parts.append({QStringLiteral("xl/drawings/drawing1.xml"), drawing, QStringLiteral("application/vnd.openxmlformats-officedocument.drawing+xml"), -1, QString(), QString(), QString()});
        parts.append({QStringLiteral("xl/drawings/_rels/drawing1.xml.rels"),
                      QByteArray("<?xml version=\"1.0\"?><Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\"><Relationship Id=\"rIdC\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/chart\" Target=\"../charts/chart1.xml\"/></Relationships>"),
                      QString(), -1, QString(), QString(), QString()});
        parts.append({QStringLiteral("xl/charts/chart1.xml"), simpleChart(), QStringLiteral("application/vnd.openxmlformats-officedocument.drawingml.chart+xml"), -1, QString(), QString(), QString()});
        Package p;
        QString why;
        QVERIFY(p.readLazy(withParts(buildXlsx({dataSheet()}), parts), &why));
        QCOMPARE(drawingPartOf(p, QStringLiteral("/xl/chartsheets/sheet1.xml")), QStringLiteral("/xl/drawings/drawing1.xml"));
        SheetDrawing d;
        QVERIFY2(readDrawing(p, QStringLiteral("/xl/drawings/drawing1.xml"), &d, &why), qPrintable(why));
        QCOMPARE(d.objects.size(), 1);
        QCOMPARE(int(d.objects[0].kind), int(ObjectKind::Chart));
        QCOMPARE(d.objects[0].chartPart, QStringLiteral("/xl/charts/chart1.xml"));
        QCOMPARE(d.objects[0].anchor.cx, qint64(9525 * 640));
        // a sheet with no drawing, and a name that is no sheet's, have none
        QVERIFY(drawingPartOf(p, QStringLiteral("/xl/worksheets/sheet1.xml")).isEmpty());
        QVERIFY(drawingPartOf(p, QString()).isEmpty());
        QVERIFY(drawingPartOf(p, QStringLiteral("/xl/nothing.xml")).isEmpty());
    }

    // More anchors than the limit are counted and not read; positions far outside any sheet are kept to what a sheet can hold; negative numbers and text are zero; a missing place is the first cell.
    void hostileAnchorsAreBounded()
    {
        QString many;
        for (int i = 0; i < kMaxDrawingAnchors + 25; ++i) many += twoCell(i % 50, i % 90, i % 50 + 1, i % 90 + 1, shapeElement(2 + i, QStringLiteral("S")));
        Opened o;
        QString why;
        QVERIFY2(open(&o, bookWithDrawing({dataSheet()}, drawingPart(many), {}), &why), qPrintable(why));
        SheetDrawing d;
        QVERIFY2(readDrawing(o.package, o.drawingPart, &d, &why), qPrintable(why));
        QCOMPARE(int(d.objects.size()), kMaxDrawingAnchors);
        QCOMPARE(d.anchorsInPart, kMaxDrawingAnchors + 25);
        QVERIFY(d.truncated);

        const QString far = twoCell(99999999, 99999999, 2147483647, 2000000000, shapeElement(2, QStringLiteral("Far")), QString(), 9223372036854775807LL, -5, 123456789012345LL, 77)
                            + absolute(9223372036854775807LL, 4000000000000000000LL, 9223372036854775807LL, 1, shapeElement(3, QStringLiteral("Huge")))
                            + QStringLiteral("<xdr:twoCellAnchor>") + QStringLiteral("<xdr:from><xdr:col>x</xdr:col><xdr:row>-4</xdr:row></xdr:from>") + shapeElement(4, QStringLiteral("NoTo")) + QStringLiteral("</xdr:twoCellAnchor>");
        QVERIFY2(open(&o, bookWithDrawing({dataSheet()}, drawingPart(far), {}), &why), qPrintable(why));
        QVERIFY2(readDrawing(o.package, o.drawingPart, &d, &why), qPrintable(why));
        QCOMPARE(d.objects.size(), 3);
        const DrawingAnchor &a = d.objects[0].anchor;
        QCOMPARE(a.from.col, kMaxColumns - 1);
        QCOMPARE(a.from.row, kMaxRows - 1);
        QCOMPARE(a.to.col, kMaxColumns - 1);
        QCOMPARE(a.to.row, kMaxRows - 1);
        QCOMPARE(a.from.colOff, kMaxDrawingEmu);
        QCOMPARE(a.from.rowOff, qint64(0));
        QCOMPARE(a.to.colOff, kMaxDrawingEmu);
        const DrawingAnchor &h = d.objects[1].anchor;
        QCOMPARE(h.x, kMaxDrawingEmu);
        QCOMPARE(h.y, kMaxDrawingEmu);
        QCOMPARE(h.cx, kMaxDrawingEmu);
        QCOMPARE(h.cy, qint64(1));
        const DrawingAnchor &n = d.objects[2].anchor;
        QCOMPARE(int(n.from.col), 0);
        QCOMPARE(int(n.from.row), 0);
        QCOMPARE(int(n.to.col), 0);
    }

    // A drawing that is not XML, has a DOCTYPE, is nested past the limit, or is another kind of part, is refused with a message and nothing is read; a group nested 300 deep is refused whole.
    void refusesWhatIsNotADrawing()
    {
        Opened o;
        QString why;
        const QByteArray doctype = "<?xml version=\"1.0\"?><!DOCTYPE x [<!ENTITY a \"b\">]><xdr:wsDr xmlns:xdr=\"http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing\"/>";
        const QByteArray other = "<?xml version=\"1.0\"?><worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"/>";
        QString deep = shapeElement(9, QStringLiteral("Core"));
        for (int i = 0; i < 300; ++i) deep = groupElement(10 + i, deep);
        const QVector<QPair<const char *, QByteArray>> cases = {{"damaged", "<xdr:wsDr"}, {"a DOCTYPE", doctype}, {"another kind of part", other}, {"groups nested 300 deep", drawingPart(twoCell(0, 0, 1, 1, deep))}};
        for (const auto &c : cases) {
            QVERIFY2(open(&o, bookWithDrawing({dataSheet()}, c.second, {}), &why), c.first);
            SheetDrawing d;
            QString error;
            QVERIFY2(!readDrawing(o.package, o.drawingPart, &d, &error), c.first);
            QVERIFY2(!error.isEmpty(), c.first);
            QVERIFY2(d.objects.isEmpty(), c.first);
        }
        // (and a drawing that is one is read, by the same code)
        QVERIFY2(open(&o, bookWithDrawing({dataSheet()}, drawingPart(twoCell(0, 0, 1, 1, shapeElement(9, QStringLiteral("Core")))), {}), &why), qPrintable(why));
        SheetDrawing ok;
        QVERIFY2(readDrawing(o.package, o.drawingPart, &ok, &why), qPrintable(why));
        QCOMPARE(ok.objects.size(), 1);
    }
};

JP_TEST_CLASS(SheetDrawingTests)
#include "tst_sheet_drawing.moc"
