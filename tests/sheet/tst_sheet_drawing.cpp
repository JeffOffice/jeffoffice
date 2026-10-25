// Tests of the drawing part of a sheet (libs/office/sheet/drawing.h): the three kinds of anchor with their editAs, what each anchor holds (a chart, a picture, a shape, a text box, a group,
// a connector), a chart sheet's drawing, and the hostile forms. Every workbook is built in memory.
#include "drawinghelpers.h"

#include "office/xml.h"
#include "sheet/chartdata.h"
#include "sheet/drawing.h"
#include "sheet/drawingsave.h"
#include "sheet/xlsxsave.h"
#include "sheet/structsave.h"

#include <QRandomGenerator>

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

    // The anchors move on the screen as saving moves them in the file: for any run of inserted and deleted rows and columns, the anchor movedAnchor gives is the one the saved drawing part has.
    void anchorsMoveAsSavingMovesThem()
    {
        QString anchors;
        const QString kinds[] = {QString(), QStringLiteral("oneCell"), QStringLiteral("absolute"), QStringLiteral("twoCell")};
        for (int i = 0; i < 24; ++i)
            anchors += twoCell(i % 7, (i * 3) % 11, i % 7 + 1 + i % 3, (i * 3) % 11 + 1 + i % 4, shapeElement(2 + i, QStringLiteral("S")), kinds[i % 4], i, i * 2, i * 3, i * 4);
        anchors += oneCell(2, 6, 952500, 476250, shapeElement(40, QStringLiteral("One"))) + absolute(100000, 200000, 300000, 400000, shapeElement(41, QStringLiteral("Abs")));
        const QByteArray original = drawingPart(anchors);
        Opened o;
        QString why;
        QVERIFY2(open(&o, bookWithDrawing({dataSheet()}, original, {}), &why), qPrintable(why));
        SheetDrawing before;
        QVERIFY2(readDrawing(o.package, o.drawingPart, &before, &why), qPrintable(why));
        QRandomGenerator rng(12345);
        for (int seed = 0; seed < 60; ++seed) {
            MoveContext ctx;
            ctx.sheetNames = {QStringLiteral("Sheet1"), QStringLiteral("Sheet2")};
            const int n = 1 + int(rng.bounded(5));
            for (int k = 0; k < n; ++k) {
                const int sheet = rng.bounded(4) == 0 ? 1 : 0;
                const quint32 first = rng.bounded(12), count = 1 + rng.bounded(3);
                switch (rng.bounded(4)) {
                case 0: ctx.edits.push_back(StructEdit::insertRows(sheet, first, count)); break;
                case 1: ctx.edits.push_back(StructEdit::deleteRows(sheet, first, count)); break;
                case 2: ctx.edits.push_back(StructEdit::insertColumns(sheet, first, count)); break;
                default: ctx.edits.push_back(StructEdit::deleteColumns(sheet, first, count)); break;
                }
            }
            QByteArray moved;
            bool changed = false;
            QVERIFY(moveDrawingPart(original, ctx, 0, &moved, &changed));
            QVector<ExtraPart> none;
            Opened after;
            QVERIFY2(open(&after, bookWithDrawing({dataSheet()}, moved, {}), &why), qPrintable(why));
            SheetDrawing now;
            QVERIFY2(readDrawing(after.package, after.drawingPart, &now, &why), qPrintable(why));
            QCOMPARE(now.objects.size(), before.objects.size());
            for (int i = 0; i < before.objects.size(); ++i) {
                const DrawingAnchor want = now.objects[i].anchor, got = movedAnchor(before.objects[i].anchor, 0, ctx.edits);
                QVERIFY2(got.from.col == want.from.col && got.from.row == want.from.row && got.from.colOff == want.from.colOff && got.from.rowOff == want.from.rowOff, qPrintable(QStringLiteral("seed %1 anchor %2 from").arg(seed).arg(i)));
                if (want.kind == AnchorKind::TwoCell) QVERIFY2(got.to.col == want.to.col && got.to.row == want.to.row, qPrintable(QStringLiteral("seed %1 anchor %2 to").arg(seed).arg(i)));
                QVERIFY(got.kind == want.kind && got.editAs == want.editAs && got.x == want.x && got.cx == want.cx);
            }
        }
    }

    // A reference with no sheet in it is the chart's own sheet's, in the file and on the screen alike: a chart on the second sheet that reads "$B$2:$B$4" moves with rows inserted on that sheet,
    // and not with rows inserted on the first.
    void sheetlessReferencesAreTheChartsOwnSheets()
    {
        const QVector<SeriesSpec> series = {SeriesSpec{QStringLiteral("$B$1"), QStringLiteral("$A$2:$A$4"), QStringLiteral("$B$2:$B$4"), QStringLiteral("East"), {"Q1", "Q2", "Q3"}, {1, 2, 3}, QString()}};
        const QByteArray xlsx = bookWithDrawing({dataSheet(), dataSheet()}, drawingPart(twoCell(4, 1, 11, 13, chartFrame(2, QStringLiteral("rIdC")))), {chartTarget(QStringLiteral("rIdC"), 1, chartPart(QStringLiteral("barChart"), series))},
                                                QStringList(), QString(), QByteArray(), 1);
        Package p;
        QString why;
        QVERIFY(p.readLazy(xlsx, &why));
        Workbook b;
        QVERIFY2(b.open(p, &why), qPrintable(why));
        Chart model;
        Theme theme;
        QVERIFY(readChart(p.part(QStringLiteral("/xl/charts/chart1.xml")), theme, ColorMap(), &model, &why));
        for (int sheet = 0; sheet < 2; ++sheet) {
            SaveChanges changes;
            changes.edits.push_back(StructEdit::insertRows(sheet, 0, 2));
            QByteArray saved;
            QVERIFY2(saveXlsx(b, changes, &saved, &why), qPrintable(why));
            Package q;
            QVERIFY2(q.readLazy(saved, &why), qPrintable(why));
            Chart after;
            QVERIFY(readChart(q.part(QStringLiteral("/xl/charts/chart1.xml")), theme, ColorMap(), &after, &why));
            Chart live = model;
            moveChartRefs(&live, {QStringLiteral("Sheet1"), QStringLiteral("Sheet2")}, changes.edits, 1);
            QCOMPARE(after.plots[0].series[0].valueRef, live.plots[0].series[0].valueRef);
            QCOMPARE(after.plots[0].series[0].nameRef, live.plots[0].series[0].nameRef);
            // (the chart is on the second sheet: rows inserted on it move the reference, rows inserted on the first do not)
            QCOMPARE(after.plots[0].series[0].valueRef, sheet == 1 ? QStringLiteral("$B$4:$B$6") : QStringLiteral("$B$2:$B$4"));
        }
    }

    // A chart's references move as saving moves them in the chart part.
    void chartReferencesMoveAsSavingMovesThem()
    {
        const QVector<SeriesSpec> series = {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$9"), QStringLiteral("Sheet1!$B$2:$B$9"), QStringLiteral("n"), {"a"}, {1}, QString()},
                                            SeriesSpec{QStringLiteral("Sheet2!$B$1"), QStringLiteral("Sheet2!$A$2:$A$9"), QStringLiteral("(Sheet1!$C$2:$C$4,Sheet2!$C$6:$C$9)"), QStringLiteral("m"), {"a"}, {1}, QString()}};
        const QByteArray part = chartPart(QStringLiteral("lineChart"), series);
        Chart base;
        Theme theme;
        QString why;
        QVERIFY(readChart(part, theme, ColorMap(), &base, &why));
        QRandomGenerator rng(777);
        for (int seed = 0; seed < 80; ++seed) {
            MoveContext ctx;
            ctx.sheetNames = {QStringLiteral("Sheet1"), QStringLiteral("Sheet2")};
            const int n = 1 + int(rng.bounded(4));
            for (int k = 0; k < n; ++k) {
                const int sheet = int(rng.bounded(2));
                const quint32 first = rng.bounded(11), count = 1 + rng.bounded(3);
                switch (rng.bounded(4)) {
                case 0: ctx.edits.push_back(StructEdit::insertRows(sheet, first, count)); break;
                case 1: ctx.edits.push_back(StructEdit::deleteRows(sheet, first, count)); break;
                case 2: ctx.edits.push_back(StructEdit::insertColumns(sheet, first, count)); break;
                default: ctx.edits.push_back(StructEdit::deleteColumns(sheet, first, count)); break;
                }
            }
            QByteArray moved;
            bool changed = false;
            QVERIFY(moveChartPart(part, ctx, &moved, &changed));
            Chart saved;
            QVERIFY(readChart(moved, theme, ColorMap(), &saved, &why));
            Chart live = base;
            moveChartRefs(&live, ctx.sheetNames, ctx.edits);
            for (int i = 0; i < 2; ++i) {
                const ChartSeries &want = saved.plots[0].series[i], &got = live.plots[0].series[i];
                QCOMPARE(got.nameRef, want.nameRef);
                QCOMPARE(got.categoryRef, want.categoryRef);
                QCOMPARE(got.valueRef, want.valueRef);
            }
        }
    }
};

JP_TEST_CLASS(SheetDrawingTests)

// What saving does with drawings and charts: a deleted object is taken out of its drawing part, and the cache of a chart whose cells changed is made again; nothing else of a part, and no
// other part, is touched.
class SheetDrawingSaveTests : public QObject {
    Q_OBJECT

    static QString namesOf(const QByteArray &drawing, const Package &package)
    {
        Q_UNUSED(package);
        QDomDocument doc;
        parseXml(drawing, &doc);
        QStringList names;
        const QDomNodeList all = doc.elementsByTagName(QStringLiteral("cNvPr"));
        for (int i = 0; i < all.size(); ++i) names << all.at(i).toElement().attribute(QStringLiteral("name"));
        return names.join(QLatin1Char(','));
    }
    static ChartRefData numbers(const QVector<double> &values, const QVector<bool> &blank = QVector<bool>())
    {
        ChartRefData d;
        for (int i = 0; i < values.size(); ++i) {
            ChartPoint p;
            p.empty = i < blank.size() && blank[i];
            p.isNumber = !p.empty;
            p.number = values[i];
            p.text = QString::number(values[i]);
            d.points.push_back(p);
        }
        return d;
    }
    static ChartRefData texts(const QStringList &items)
    {
        ChartRefData d;
        for (const QString &t : items) {
            ChartPoint p;
            p.empty = t.isEmpty();
            p.text = t;
            d.points.push_back(p);
        }
        return d;
    }

private Q_SLOTS:
    // Objects are taken out by their place among the children of the root: a plain anchor, one wrapped for older programs, and one that closes itself; the rest is as it was to the byte.
    void deletedObjectsAreTakenOutOfTheDrawing()
    {
        const QString wrapped = QStringLiteral("<mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\"><mc:Choice Requires=\"a14\">%1</mc:Choice><mc:Fallback/></mc:AlternateContent>")
                                    .arg(twoCell(0, 0, 2, 2, shapeElement(4, QStringLiteral("Third"))));
        const QByteArray drawing = drawingPart(twoCell(0, 0, 1, 1, shapeElement(2, QStringLiteral("First"))) + QStringLiteral("\n  ") + twoCell(1, 1, 2, 2, shapeElement(3, QStringLiteral("Second"))) + wrapped
                                               + QStringLiteral("<xdr:note/>") + twoCell(2, 2, 3, 3, shapeElement(5, QStringLiteral("Fifth"))));
        Package p;
        QByteArray out;
        bool changed = true;
        QVERIFY(removeDrawingObjects(drawing, {}, &out, &changed));
        QVERIFY(!changed);
        QCOMPARE(out, drawing);
        QVERIFY(removeDrawingObjects(drawing, {1, 3}, &out, &changed));
        QVERIFY(changed);
        QCOMPARE(namesOf(out, p), QStringLiteral("First,Third,Fifth"));
        QVERIFY(removeDrawingObjects(drawing, {2}, &out, &changed));   // the wrapper goes whole
        QCOMPARE(namesOf(out, p), QStringLiteral("First,Second,Fifth"));
        QVERIFY(removeDrawingObjects(drawing, {0, 1, 2, 3, 4}, &out, &changed));
        QCOMPARE(namesOf(out, p), QString());
        QVERIFY(out.contains("wsDr"));
        QVERIFY(removeDrawingObjects(drawing, {9, 99}, &out, &changed));   // (places the part does not have)
        QVERIFY(!changed);
        QCOMPARE(out, drawing);
        // what is left is as written: the bytes before the first and after the last object are the same
        QVERIFY(removeDrawingObjects(drawing, {4}, &out, &changed));
        QVERIFY(out.startsWith(drawing.left(drawing.indexOf("<xdr:twoCellAnchor", drawing.indexOf("Second")) + 10)));
        QVERIFY(out.endsWith("</xdr:wsDr>"));
        // a part with no root, and one whose root closes itself
        QVERIFY(!removeDrawingObjects(QByteArray("not xml"), {0}, &out, &changed));
        QVERIFY(removeDrawingObjects(QByteArray("<xdr:wsDr xmlns:xdr=\"x\"/>"), {0}, &out, &changed));
        QVERIFY(!changed);
    }

    // A cache is made again where the cells differ from it, with the format code and the prefix the part has; one that holds what the cells hold, and a reference that is not asked for, stay as they are.
    void chartCachesAreMadeAgainWhereTheyDiffer()
    {
        const QVector<SeriesSpec> series = {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {"Q1", "Q2", "Q3"}, {10, 20, 30}, QString()},
                                            SeriesSpec{QStringLiteral("Sheet1!$C$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$C$2:$C$4"), QStringLiteral("West"), {"Q1", "Q2", "Q3"}, {5, 10, 15}, QString()}};
        QByteArray part = chartPart(QStringLiteral("barChart"), series);
        part.replace("<c:formatCode>General</c:formatCode>", "<c:formatCode>0.0</c:formatCode>");
        QByteArray out;
        bool changed = true;
        // the same as the cache: nothing changes, to the byte
        ChartCaches same;
        same[QStringLiteral("Sheet1!$B$2:$B$4")] = numbers({10, 20, 30});
        same[QStringLiteral("Sheet1!$A$2:$A$4")] = texts({"Q1", "Q2", "Q3"});
        same[QStringLiteral("Sheet1!$B$1")] = texts({"East"});
        QVERIFY(refreshChartCaches(part, same, &out, &changed));
        QVERIFY(!changed);
        QCOMPARE(out, part);
        // different: the number, a text, and a name
        ChartCaches now;
        now[QStringLiteral("Sheet1!$B$2:$B$4")] = numbers({10, 99.5, 30.25});
        now[QStringLiteral("Sheet1!$A$2:$A$4")] = texts({"Q1", "Second & more", "Q3"});
        now[QStringLiteral("Sheet1!$B$1")] = texts({"Eastern"});
        QVERIFY(refreshChartCaches(part, now, &out, &changed));
        QVERIFY(changed);
        Chart c;
        QString why;
        Theme theme;
        QVERIFY2(readChart(out, theme, ColorMap(), &c, &why), qPrintable(why));
        QCOMPARE(c.plots[0].series[0].values, (QVector<double>{10, 99.5, 30.25}));
        QCOMPARE(c.plots[0].series[0].categories, (QStringList{"Q1", "Second & more", "Q3"}));
        QCOMPARE(c.plots[0].series[0].name, QStringLiteral("Eastern"));
        QCOMPARE(c.plots[0].series[0].format, QStringLiteral("0.0"));            // (the format code stays)
        QCOMPARE(c.plots[0].series[1].values, (QVector<double>{5, 10, 15}));     // (the other series is not asked for: as it was)
        QCOMPARE(c.plots[0].series[1].name, QStringLiteral("West"));
        // the part is the old one with only the caches of the references asked for replaced (the second series reads the same categories, so they are made again for it too): the rest is as it was
        const qsizetype west = part.indexOf("<c:val><c:numRef><c:f>Sheet1!$C$2");
        QVERIFY(west > 0);
        QVERIFY(out.endsWith(part.mid(west)));
        // a blank cell has no point, the count is the cells'; a longer range has more places
        ChartCaches blanks;
        blanks[QStringLiteral("Sheet1!$C$2:$C$4")] = numbers({5, 0, 15, 7}, {false, true, false, false});
        QVERIFY(refreshChartCaches(part, blanks, &out, &changed));
        QVERIFY(changed);
        QVERIFY(out.contains("<c:ptCount val=\"4\"/><c:pt idx=\"0\"><c:v>5</c:v></c:pt><c:pt idx=\"2\"><c:v>15</c:v></c:pt><c:pt idx=\"3\"><c:v>7</c:v></c:pt>"));
        QVERIFY(readChart(out, theme, ColorMap(), &c, &why));
        QVERIFY(std::isnan(c.plots[0].series[1].values.value(1)));
        // numbers equal to 15 digits are the cache's
        ChartCaches near;
        near[QStringLiteral("Sheet1!$B$2:$B$4")] = numbers({10.000000000000002, 20, 30});
        QVERIFY(refreshChartCaches(part, near, &out, &changed));
        QVERIFY(!changed);
        // a reference that has no cache is given one, and a reference of several levels, a literal, and a reference that is not in the part are not touched
        QByteArray bare = part;
        bare.replace("<c:numCache><c:formatCode>0.0</c:formatCode><c:ptCount val=\"3\"/><c:pt idx=\"0\"><c:v>10</c:v></c:pt><c:pt idx=\"1\"><c:v>20</c:v></c:pt><c:pt idx=\"2\"><c:v>30</c:v></c:pt></c:numCache>", "");
        QVERIFY(bare != part);
        ChartCaches fill;
        fill[QStringLiteral("Sheet1!$B$2:$B$4")] = numbers({1, 2, 3});
        fill[QStringLiteral("Elsewhere!$A$1")] = numbers({1});
        QVERIFY(refreshChartCaches(bare, fill, &out, &changed));
        QVERIFY(changed);
        QVERIFY(readChart(out, theme, ColorMap(), &c, &why));
        QCOMPARE(c.plots[0].series[0].values, (QVector<double>{1, 2, 3}));
        QByteArray multi = part;
        multi.replace("<c:strRef><c:f>Sheet1!$A$2:$A$4</c:f>", "<c:multiLvlStrRef><c:f>Sheet1!$A$2:$A$4</c:f>");
        multi.replace("</c:strCache></c:strRef></c:cat>", "</c:strCache></c:multiLvlStrRef></c:cat>");
        ChartCaches levels;
        levels[QStringLiteral("Sheet1!$A$2:$A$4")] = texts({"x", "y", "z"});
        QVERIFY(refreshChartCaches(multi, levels, &out, &changed));
        QVERIFY(!changed);
        QCOMPARE(out, multi);
        // a text with markup characters is escaped
        ChartCaches odd;
        odd[QStringLiteral("Sheet1!$B$1")] = texts({"A <b> & \"c\""});
        QVERIFY(refreshChartCaches(part, odd, &out, &changed));
        QVERIFY(readChart(out, theme, ColorMap(), &c, &why));
        QCOMPARE(c.plots[0].series[0].name, QStringLiteral("A <b> & \"c\""));
    }



    // What an object relates to goes with it when nothing else needs it: its link in the drawing's relationships, and the chart or picture part, with that part's own relationships, parts, and
    // type, if no other relationship of the package goes to it (every relationships part is looked at); a picture two objects use stays until both are gone.
    void aDeletedObjectsPartsGoWithIt()
    {
        const QVector<SeriesSpec> series = {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {"Q1", "Q2", "Q3"}, {10, 20, 30}, QString()}};
        const QString anchors = twoCell(4, 1, 11, 13, chartFrame(2, QStringLiteral("rIdC"))) + twoCell(0, 6, 2, 9, pictureElement(3, QStringLiteral("rIdP"), QStringLiteral("One")))
                                + twoCell(0, 10, 2, 12, pictureElement(4, QStringLiteral("rIdP"), QStringLiteral("Two"))) + twoCell(3, 14, 5, 16, shapeElement(5, QStringLiteral("Box")));
        QVector<ExtraPart> extra;
        extra.append({QStringLiteral("xl/charts/style1.xml"), QByteArray("<cs:chartStyle xmlns:cs=\"http://schemas.microsoft.com/office/drawing/2012/chartStyle\"/>"), QStringLiteral("application/vnd.ms-office.chartstyle+xml"), -1, QString(), QString(), QString()});
        extra.append({QStringLiteral("xl/charts/_rels/chart1.xml.rels"),
                      QByteArray("<?xml version=\"1.0\"?><Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\"><Relationship Id=\"rIdS\" Type=\"http://schemas.microsoft.com/office/2011/relationships/chartStyle\" Target=\"style1.xml\"/></Relationships>"),
                      QString(), -1, QString(), QString(), QString()});
        const QByteArray xlsx = withParts(bookWithDrawing({dataSheet()}, drawingPart(anchors), {chartTarget(QStringLiteral("rIdC"), 1, chartPart(QStringLiteral("barChart"), series)), imageTarget(QStringLiteral("rIdP"), 1, pngBytes(8, 4, Qt::red))}), extra);
        Package p;
        QString why;
        QVERIFY(p.readLazy(xlsx, &why));
        Workbook b;
        QVERIFY2(b.open(p, &why), qPrintable(why));
        auto save = [&](const std::vector<int> &ordinals, QMap<QString, QByteArray> *parts, Workbook *book = nullptr) {
            SaveChanges changes;
            changes.removedObjects[QStringLiteral("/xl/drawings/drawing1.xml")] = ordinals;
            QByteArray out;
            QString error;
            const bool ok = saveXlsx(book ? *book : b, changes, &out, &error);
            parts->clear();
            return ok && jp::readZip(out, *parts);
        };
        auto relationshipIds = [](const QByteArray &rels) {
            QStringList ids;
            QRegularExpressionMatchIterator it = QRegularExpression(QStringLiteral("Id=\"([^\"]+)\"")).globalMatch(QString::fromUtf8(rels));
            while (it.hasNext()) ids << it.next().captured(1);
            ids.sort();
            return ids;
        };
        QMap<QString, QByteArray> parts;
        // the chart: its part, its own relationships, the style part they name, and their types go; the pictures stay
        QVERIFY(save({0}, &parts));
        QVERIFY(!parts.contains(QStringLiteral("xl/charts/chart1.xml")) && !parts.contains(QStringLiteral("xl/charts/style1.xml")) && !parts.contains(QStringLiteral("xl/charts/_rels/chart1.xml.rels")));
        QVERIFY(parts.contains(QStringLiteral("xl/media/image1.png")));
        QCOMPARE(relationshipIds(parts.value(QStringLiteral("xl/drawings/_rels/drawing1.xml.rels"))), QStringList{QStringLiteral("rIdP")});
        const QByteArray types = parts.value(QStringLiteral("[Content_Types].xml"));
        QVERIFY2(!types.contains("/xl/charts/chart1.xml") && !types.contains("/xl/charts/style1.xml") && types.contains("/xl/drawings/drawing1.xml"), types.constData());
        QCOMPARE(namesOf(parts.value(QStringLiteral("xl/drawings/drawing1.xml")), p), QStringLiteral("One,Two,Box"));
        // one of the two objects that use a picture: it stays
        QVERIFY(save({1}, &parts));
        QVERIFY(parts.contains(QStringLiteral("xl/media/image1.png")) && parts.contains(QStringLiteral("xl/charts/chart1.xml")));
        QCOMPARE(relationshipIds(parts.value(QStringLiteral("xl/drawings/_rels/drawing1.xml.rels"))), (QStringList{QStringLiteral("rIdC"), QStringLiteral("rIdP")}));
        // both: it goes (the type of a picture is the extension's, which stays)
        QVERIFY(save({1, 2}, &parts));
        QVERIFY(!parts.contains(QStringLiteral("xl/media/image1.png")));
        QCOMPARE(relationshipIds(parts.value(QStringLiteral("xl/drawings/_rels/drawing1.xml.rels"))), QStringList{QStringLiteral("rIdC")});
        QVERIFY(parts.value(QStringLiteral("[Content_Types].xml")).contains("/xl/charts/chart1.xml"));
        // everything: every part is left only for the shape
        QVERIFY(save({0, 1, 2}, &parts));
        QVERIFY(!parts.contains(QStringLiteral("xl/media/image1.png")) && !parts.contains(QStringLiteral("xl/charts/chart1.xml")));
        QVERIFY(relationshipIds(parts.value(QStringLiteral("xl/drawings/_rels/drawing1.xml.rels"))).isEmpty());
        // a picture that another part's relationship goes to stays: the sheet's own
        QVector<ExtraPart> also;
        also.append({QStringLiteral("xl/media/image1.png"), pngBytes(8, 4, Qt::red), QString(), 0, QStringLiteral("rIdX"), QStringLiteral("image"), QStringLiteral("../media/image1.png")});
        QByteArray shared = withParts(xlsx, also);
        Package p2;
        QVERIFY(p2.readLazy(shared, &why));
        Workbook b2;
        QVERIFY2(b2.open(p2, &why), qPrintable(why));
        QVERIFY(save({1, 2}, &parts, &b2));
        QVERIFY(parts.contains(QStringLiteral("xl/media/image1.png")));
        QCOMPARE(relationshipIds(parts.value(QStringLiteral("xl/drawings/_rels/drawing1.xml.rels"))), QStringList{QStringLiteral("rIdC")});
        // a chart nothing but the drawing relates to, and no other object of its element: nothing else of the file changes
        QVERIFY(save({3}, &parts));
        for (const QString &name : p.partNames()) {
            const QString entry = name.mid(1);
            if (entry == QLatin1String("xl/drawings/drawing1.xml")) continue;
            QVERIFY2(parts.value(entry) == p.part(name), qPrintable(name));
        }
        QMap<QString, QByteArray> original;
        QVERIFY(jp::readZip(xlsx, original));
        QCOMPARE(parts.value(QStringLiteral("xl/drawings/_rels/drawing1.xml.rels")), original.value(QStringLiteral("xl/drawings/_rels/drawing1.xml.rels")));   // (a shape uses none: the links are as they were)
        QCOMPARE(parts.value(QStringLiteral("[Content_Types].xml")), original.value(QStringLiteral("[Content_Types].xml")));
    }

    // The anchors an object that keeps its size has are written again where they differ: the first two-cell anchor of the element at the place (a wrapped one too), nothing else.
    void anchorsAreWrittenAgain()
    {
        const QString wrapped = QStringLiteral("<mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\"><mc:Choice Requires=\"a14\">%1</mc:Choice><mc:Fallback/></mc:AlternateContent>")
                                    .arg(twoCell(0, 0, 2, 2, shapeElement(4, QStringLiteral("Wrapped")), QStringLiteral("oneCell")));
        const QByteArray drawing = drawingPart(twoCell(0, 0, 1, 1, shapeElement(2, QStringLiteral("First")), QStringLiteral("oneCell")) + twoCell(1, 1, 2, 2, shapeElement(3, QStringLiteral("Second"))) + wrapped
                                               + oneCell(2, 2, 952500, 476250, shapeElement(5, QStringLiteral("Sized"))));
        Package p;
        QString why;
        Opened o;
        QVERIFY2(open(&o, bookWithDrawing({dataSheet()}, drawing, {}), &why), qPrintable(why));
        std::map<int, DrawingAnchor> anchors;
        DrawingAnchor a;
        a.kind = AnchorKind::TwoCell;
        a.from = {3, 4, 9525, 19050};
        a.to = {6, 8, 28575, 38100};
        anchors[0] = a;
        anchors[2] = a;   // (the wrapper)
        anchors[3] = a;   // (a one-cell anchor: left as it is)
        anchors[9] = a;   // (a place that is not there)
        QByteArray out;
        bool changed = false;
        QVERIFY(setDrawingAnchors(o.package.part(o.drawingPart), anchors, &out, &changed));
        QVERIFY(changed);
        Opened after;
        QVERIFY2(open(&after, bookWithDrawing({dataSheet()}, out, {}), &why), qPrintable(why));
        SheetDrawing was, now;
        QVERIFY(readDrawing(o.package, o.drawingPart, &was, &why));
        QVERIFY(readDrawing(after.package, after.drawingPart, &now, &why));
        QCOMPARE(now.objects.size(), was.objects.size());
        for (int i : {0, 2}) {
            QCOMPARE(int(now.objects[i].anchor.from.col), 3);
            QCOMPARE(int(now.objects[i].anchor.from.row), 4);
            QCOMPARE(now.objects[i].anchor.from.colOff, qint64(9525));
            QCOMPARE(now.objects[i].anchor.from.rowOff, qint64(19050));
            QCOMPARE(int(now.objects[i].anchor.to.col), 6);
            QCOMPARE(int(now.objects[i].anchor.to.row), 8);
            QCOMPARE(now.objects[i].anchor.to.colOff, qint64(28575));
            QCOMPARE(now.objects[i].anchor.to.rowOff, qint64(38100));
            QCOMPARE(int(now.objects[i].anchor.editAs), int(AnchorKind::OneCell));   // (the rest of the anchor: as it was)
            QCOMPARE(now.objects[i].name, was.objects[i].name);
        }
        for (int i : {1, 3}) {   // (the second, which was not asked for, and the one-cell anchor)
            QCOMPARE(int(now.objects[i].anchor.from.col), int(was.objects[i].anchor.from.col));
            QCOMPARE(int(now.objects[i].anchor.to.col), int(was.objects[i].anchor.to.col));
            QCOMPARE(now.objects[i].anchor.cx, was.objects[i].anchor.cx);
        }
        // nothing to say: nothing changed
        QVERIFY(setDrawingAnchors(o.package.part(o.drawingPart), {}, &out, &changed));
        QVERIFY(!changed);
    }

    // The references of a chart part, as it writes them, and the ids a part names.
    void referencesAndRelationshipIdsAreFound()
    {
        const QByteArray part = chartPart(QStringLiteral("barChart"), {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("(Sheet1!$A$2:$A$3,Sheet1!$A$5)"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {"a"}, {1}, QString()}});
        QCOMPARE(chartPartRefs(part), (QStringList{QStringLiteral("Sheet1!$B$1"), QStringLiteral("(Sheet1!$A$2:$A$3,Sheet1!$A$5)"), QStringLiteral("Sheet1!$B$2:$B$4")}));
        QVERIFY(chartPartRefs(QByteArray("<a/>")).isEmpty());
        const QByteArray drawing = drawingPart(twoCell(0, 0, 1, 1, pictureElement(3, QStringLiteral("rIdP"))) + twoCell(0, 0, 1, 1, chartFrame(2, QStringLiteral("rIdC"))));
        QCOMPARE(namedRelationships(drawing), (QStringList{QStringLiteral("rIdP"), QStringLiteral("rIdC")}));
        QStringList ids;
        QByteArray out;
        bool changed = false;
        QVERIFY(removeDrawingObjects(drawing, {1}, &out, &changed, &ids));
        QCOMPARE(ids, QStringList{QStringLiteral("rIdC")});
        QCOMPARE(namedRelationships(out), QStringList{QStringLiteral("rIdP")});
    }

    // A save with both: the drawing part has no deleted object, the chart part the new numbers, and every other part of the file is the old file's to the byte.
    void aSaveWritesBothAndCopiesTheRest()
    {
        const QVector<SeriesSpec> series = {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {"Q1", "Q2", "Q3"}, {10, 20, 30}, QString()}};
        const QString anchors = twoCell(4, 1, 11, 13, chartFrame(2, QStringLiteral("rIdC"))) + twoCell(0, 6, 2, 9, pictureElement(3, QStringLiteral("rIdP"))) + twoCell(0, 10, 2, 12, shapeElement(4, QStringLiteral("Box")));
        const QByteArray xlsx = bookWithDrawing({dataSheet()}, drawingPart(anchors), {chartTarget(QStringLiteral("rIdC"), 1, chartPart(QStringLiteral("barChart"), series)), imageTarget(QStringLiteral("rIdP"), 1, pngBytes(8, 4, Qt::red))});
        Package p;
        QString why;
        QVERIFY(p.readLazy(xlsx, &why));
        Workbook b;
        QVERIFY2(b.open(p, &why), qPrintable(why));
        SaveChanges changes;
        changes.removedObjects[QStringLiteral("/xl/drawings/drawing1.xml")] = {1};
        changes.chartCaches[QStringLiteral("/xl/charts/chart1.xml")][QStringLiteral("Sheet1!$B$2:$B$4")] = numbers({10, 20, 77});
        QByteArray saved;
        SaveReport report;
        QVERIFY2(saveXlsx(b, changes, &saved, &why, &report), qPrintable(why));
        Package q;
        QVERIFY2(q.readLazy(saved, &why), qPrintable(why));
        QCOMPARE(namesOf(q.part(QStringLiteral("/xl/drawings/drawing1.xml")), q), QStringLiteral("Chart,Box"));
        Chart c;
        Theme theme;
        QVERIFY(readChart(q.part(QStringLiteral("/xl/charts/chart1.xml")), theme, ColorMap(), &c, &why));
        QCOMPARE(c.plots[0].series[0].values, (QVector<double>{10, 20, 77}));
        // (the drawing, the chart, the drawing's relationships without the deleted picture's, and the types without its part's; the picture's part is gone)
        QCOMPARE(report.partsChanged, 4);
        QVERIFY(!q.hasPart(QStringLiteral("/xl/media/image1.png")));
        for (const QString &name : p.partNames()) {
            if (name == QLatin1String("/xl/drawings/drawing1.xml") || name == QLatin1String("/xl/charts/chart1.xml") || name == QLatin1String("/xl/media/image1.png")) continue;
            QVERIFY2(q.part(name) == p.part(name), qPrintable(name));
        }
        // and nothing asked for changes nothing
        SaveChanges nothing;
        QByteArray same;
        SaveReport r2;
        QVERIFY(saveXlsx(b, nothing, &same, &why, &r2));
        QCOMPARE(r2.partsChanged, 0);
    }
};

JP_TEST_CLASS(SheetDrawingSaveTests)
#include "tst_sheet_drawing.moc"
