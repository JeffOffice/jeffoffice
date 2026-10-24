// Tests of the charts, pictures, and shapes over the cells in JeffCrunch's window: they are shown where their anchors put them, a chart is drawn from the cells it reads and again when one
// changes (typed, or worked out), and objects and charts move when rows are inserted or deleted, undone and redone. Every workbook is built here.
#include "../sheet/drawinghelpers.h"
#include "editwindow.h"

#include "core/fonts.h"
#include "core/sheetobjects.h"
#include "office/chart.h"
#include "office/package.h"
#include "office/theme.h"
#include "office/xml.h"

#include <QProcess>
#include <QToolButton>
#include <QThreadPool>
#include <QFileInfo>

using namespace crunchtest;
using namespace drawtest;

namespace {

// Sheet "Sheet1": A1:C4 hold the labels and numbers of dataSheet() (B2:B4 are 10, 20, 30; C2:C4 are 5, 10, 15) with B4 a formula, =B2*3, stored as 30 (which the calculation makes 30 again, and
// 60 when B2 is 20). The drawing: a chart of B2:B4 by A2:A4 over E2:K13, a red picture over A7:B9, a blue shape over A11:B12, a group of two shapes over D15:F16, and a line.
QByteArray drawingBook()
{
    auto text = [](const QString &ref, const QString &t) { return QStringLiteral("<c r=\"%1\" t=\"inlineStr\"><is><t>%2</t></is></c>").arg(ref, t); };
    const QString rows = QStringLiteral("<row r=\"1\">") + text("A1", "Region") + text("B1", "East") + text("C1", "West") + QStringLiteral("</row>")
                         + QStringLiteral("<row r=\"2\">") + text("A2", "Q1") + QStringLiteral("<c r=\"B2\"><v>10</v></c><c r=\"C2\"><v>5</v></c></row>")
                         + QStringLiteral("<row r=\"3\">") + text("A3", "Q2") + QStringLiteral("<c r=\"B3\"><v>20</v></c><c r=\"C3\"><v>10</v></c></row>")
                         + QStringLiteral("<row r=\"4\">") + text("A4", "Q3") + QStringLiteral("<c r=\"B4\"><f>B2*3</f><v>30</v></c><c r=\"C4\"><v>15</v></c></row>");
    const QVector<SeriesSpec> series = {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {"Q1", "Q2", "Q3"}, {10, 20, 30}, QString()}};
    const QString anchors = twoCell(4, 1, 11, 13, chartFrame(2, QStringLiteral("rIdC"), QStringLiteral("Sales"))) + twoCell(0, 6, 2, 9, pictureElement(3, QStringLiteral("rIdP"), QStringLiteral("Logo")))
                            + twoCell(0, 10, 2, 12, shapeElement(4, QStringLiteral("Box"), QStringLiteral("rect"), QString(), QStringLiteral("3366CC")), QStringLiteral("oneCell"))
                            + twoCell(3, 14, 6, 16, groupElement(5, shapeElement(6, QStringLiteral("In1"), QStringLiteral("rect"), QString(), QStringLiteral("00AA00"), 952500, 476250), 952500, 476250))
                            + twoCell(0, 17, 4, 17, connectorElement(7));
    return bookWithDrawing({worksheet(rows)}, drawingPart(anchors),
                           {chartTarget(QStringLiteral("rIdC"), 1, chartPart(QStringLiteral("barChart"), series)), imageTarget(QStringLiteral("rIdP"), 1, pngBytes(8, 4, Qt::red))});
}

// A PNG that says it is 100,000 by 100,000 pixels and holds no pixels (only its header and its end).
QByteArray hugePng()
{
    auto crc = [](const QByteArray &data) {
        quint32 c = 0xFFFFFFFFu;
        for (const char ch : data) {
            c ^= quint8(ch);
            for (int k = 0; k < 8; ++k) c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1u)));
        }
        return c ^ 0xFFFFFFFFu;
    };
    auto be32 = [](quint32 v) {
        QByteArray b;
        for (int shift = 24; shift >= 0; shift -= 8) b.append(char((v >> shift) & 0xFF));
        return b;
    };
    auto chunk = [&](const char *type, const QByteArray &body) {
        const QByteArray typed = QByteArray(type) + body;
        return be32(quint32(body.size())) + typed + be32(crc(typed));
    };
    QByteArray ihdr = be32(100000) + be32(100000);
    ihdr.append(char(8));
    ihdr.append(char(2));
    ihdr.append(char(0));
    ihdr.append(char(0));
    ihdr.append(char(0));
    return QByteArray("\x89PNG\r\n\x1a\n", 8) + chunk("IHDR", ihdr) + chunk("IEND", QByteArray());
}

// The data sheet, then a chart sheet "Chart1": a page that holds one chart of Sheet1!B2:B4 and no cells. The chart part has no fill of its own (the page behind it is the sheet's).
QByteArray bookWithChartSheet()
{
    const QVector<SeriesSpec> series = {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {"Q1", "Q2", "Q3"}, {10, 20, 30}, QString()}};
    QByteArray chart = chartPart(QStringLiteral("barChart"), series);
    chart.replace("</c:chart>", "</c:chart><c:spPr><a:noFill/><a:ln><a:noFill/></a:ln></c:spPr>");
    QMap<QString, QByteArray> parts;
    jp::readZip(buildXlsx({dataSheet()}), parts);
    const QString relNs = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships");
    parts[QStringLiteral("xl/workbook.xml")].replace("</sheets>", "<sheet name=\"Chart1\" sheetId=\"9\" r:id=\"rIdCS1\"/></sheets>");
    parts[QStringLiteral("xl/_rels/workbook.xml.rels")].replace("</Relationships>", QStringLiteral("<Relationship Id=\"rIdCS1\" Type=\"%1/chartsheet\" Target=\"chartsheets/sheet1.xml\"/></Relationships>").arg(relNs).toUtf8());
    parts[QStringLiteral("[Content_Types].xml")].replace("</Types>", "<Override PartName=\"/xl/chartsheets/sheet1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.chartsheet+xml\"/>"
                                                                      "<Override PartName=\"/xl/drawings/drawing1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.drawing+xml\"/>"
                                                                      "<Override PartName=\"/xl/charts/chart1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.drawingml.chart+xml\"/></Types>");
    parts[QStringLiteral("xl/chartsheets/sheet1.xml")] = QStringLiteral("<?xml version=\"1.0\"?><chartsheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"%1\"><drawing r:id=\"rIdD\"/></chartsheet>").arg(relNs).toUtf8();
    parts[QStringLiteral("xl/chartsheets/_rels/sheet1.xml.rels")] = QStringLiteral("<?xml version=\"1.0\"?><Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                                                                                  "<Relationship Id=\"rIdD\" Type=\"%1/drawing\" Target=\"../drawings/drawing1.xml\"/></Relationships>").arg(relNs).toUtf8();
    parts[QStringLiteral("xl/drawings/drawing1.xml")] = drawingPart(absolute(0, 0, 9525 * 640, 9525 * 480, chartFrame(2, QStringLiteral("rIdC"), QStringLiteral("Page"))));
    parts[QStringLiteral("xl/drawings/_rels/drawing1.xml.rels")] = QStringLiteral("<?xml version=\"1.0\"?><Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                                                                                  "<Relationship Id=\"rIdC\" Type=\"%1/chart\" Target=\"../charts/chart1.xml\"/></Relationships>").arg(relNs).toUtf8();
    parts[QStringLiteral("xl/charts/chart1.xml")] = chart;
    jp::ZipWriter z;
    for (auto it = parts.begin(); it != parts.end(); ++it) z.add(it.key(), it.value(), true);
    return z.finish();
}

} // namespace

class CrunchDrawingTests : public QObject {
    Q_OBJECT

    // The picture of the viewport, and where an object is in it.
    static QImage shot(EditWindow &e)
    {
        e.view()->viewport()->repaint();
        return e.view()->viewport()->grab().toImage();
    }
    // The color at a logical point of a grabbed picture (a picture of a screen with a device pixel ratio above 1 has more pixels than the widget has points).
    static QRgb at(const QImage &img, int x, int y)
    {
        const qreal r = img.devicePixelRatio();
        return img.pixel(qRound(x * r), qRound(y * r));
    }
    static QRgb at(const QImage &img, const QPoint &p) { return at(img, p.x(), p.y()); }
    static bool shown(EditWindow &e, int count, int sheet = 0)
    {
        crunch::SheetObjects *o = e.win.w.objects();
        return o && o->shownSheet() == sheet && o->layer()->count() == count;
    }
    // The parts of a saved file, by name (the names as the zip has them, no leading slash): what the package holds, [Content_Types].xml and the relationships among them.
    static QMap<QString, QByteArray> savedParts(const QString &path)
    {
        QMap<QString, QByteArray> parts;
        QFile f(path);
        if (f.open(QIODevice::ReadOnly)) jp::readZip(f.readAll(), parts);
        return parts;
    }
    // Whether the relationships part holds a relationship with this id, and whether the content types hold an override for this part.
    static bool hasRelationship(const QMap<QString, QByteArray> &parts, const QString &relsPart, const QString &id)
    {
        return parts.value(relsPart).contains(QStringLiteral("Id=\"%1\"").arg(id).toUtf8());
    }
    static bool hasOverride(const QMap<QString, QByteArray> &parts, const QString &partName)
    {
        return parts.value(QStringLiteral("[Content_Types].xml")).contains(QStringLiteral("PartName=\"%1\"").arg(partName).toUtf8());
    }
    // The anchor the saved drawing has for the object of this name: its first row and its last row (a one-cell anchor has the one), and the anchor's editAs.
    struct SavedAnchor {
        bool found = false;
        QString editAs, kind;
        int fromRow = -1, toRow = -1;
    };
    static SavedAnchor savedAnchor(const QString &path, const QString &name)
    {
        SavedAnchor out;
        office::Package p;
        QString why;
        if (!p.readLazyFile(path, &why)) return out;
        QDomDocument doc;
        if (!office::parseXml(p.part(QStringLiteral("/xl/drawings/drawing1.xml")), &doc)) return out;
        const QDomNodeList all = doc.elementsByTagName(QStringLiteral("cNvPr"));
        for (int i = 0; i < all.size(); ++i) {
            if (all.at(i).toElement().attribute(QStringLiteral("name")) != name) continue;
            QDomElement anchor = all.at(i).parentNode().toElement();
            while (!anchor.isNull() && !anchor.localName().endsWith(QLatin1String("Anchor"))) anchor = anchor.parentNode().toElement();
            if (anchor.isNull()) return out;
            out.found = true;
            out.kind = anchor.localName();
            out.editAs = anchor.attribute(QStringLiteral("editAs"));
            auto rowOf = [&](const char *tag) {
                for (QDomElement c = anchor.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
                    if (c.localName() == QLatin1String(tag)) return c.firstChildElement(QStringLiteral("row")).text().toInt();
                return -1;
            };
            out.fromRow = rowOf("from");
            out.toRow = out.kind == QLatin1String("twoCellAnchor") ? rowOf("to") : out.fromRow;
            return out;
        }
        return out;
    }

private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    // The objects of the sheet in view are made between events after it is shown, put where their anchors say, over the cells.
    void objectsAreShownOverTheCells()
    {
        EditWindow e;
        QVERIFY(e.open(drawingBook()));
        QTRY_VERIFY(shown(e, 5));
        crunch::SheetObjects *o = e.win.w.objects();
        const crunch::DrawingCounts c = o->counts(0);
        QCOMPARE(c.charts, 1);
        QCOMPARE(c.pictures, 1);
        QCOMPARE(c.shapes, 1);
        QCOMPARE(c.groups, 1);
        QCOMPARE(c.connectors, 1);
        QCOMPARE(c.shown, 5);
        QCOMPARE(c.leftOut, 0);
        QCOMPARE(o->drawingsParsedApart(), 0);   // (a small drawing is read where it is)
        o->layer()->prepareAll();
        const QImage img = shot(e);
        // the picture: red where the anchor puts it (A7 to B9, rows 6 to 8), the shape blue (A11 to B12)
        const grid::GridLayout &L = e.view()->sheet().layout();
        const QRect pic = L.rangeRect(6, 0, 8, 1), box = L.rangeRect(10, 0, 11, 1), chart = L.rangeRect(1, 4, 12, 10), group = L.rangeRect(14, 3, 15, 5);
        QCOMPARE(at(img, pic.center()), QColor(Qt::red).rgb());
        QCOMPARE(at(img, box.left() + 6, box.top() + 6), QColor(0x33, 0x66, 0xCC).rgb());
        QVERIFY(at(img, box.left() - 2, box.center().y()) != QColor(0x33, 0x66, 0xCC).rgb());
        QCOMPARE(at(img, group.left() + 6, group.top() + 6), QColor(0x00, 0xAA, 0x00).rgb());
        // the chart: a picture with more than the grid's white and gray in it, inside its anchor and not outside
        QSet<QRgb> colors;
        for (int y = chart.top(); y < chart.bottom(); y += 3)
            for (int x = chart.left(); x < chart.right(); x += 3) colors.insert(at(img, x, y));
        QVERIFY2(colors.size() >= 4, qPrintable(QString::number(colors.size())));
        QCOMPARE(at(img, chart.right() + 4, chart.center().y()), at(img, chart.right() + 8, chart.center().y()));   // (the plain cells beside it)
        // a click on the picture picks it; a click on a cell lets it go and moves the cursor
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, pic.center());
        QCOMPARE(o->layer()->selectedId(), 1);
        const QRect cell = L.cellRect(2, 3);
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, cell.center());
        QCOMPARE(o->layer()->selectedId(), -1);
        QCOMPARE(e.selected(), QStringLiteral("D3"));
    }

    // The chart is drawn from the cells: what the workbook holds, not what the chart part stored (the part says 10, 20, 30), and again when a cell it reads is typed over or a formula
    // it reads is worked out again.
    void aChartFollowsItsCells()
    {
        EditWindow e;
        // the part stores other numbers than the cells have
        QVERIFY(e.open(drawingBook()));
        QTRY_VERIFY(shown(e, 5));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        QCOMPARE(o->chartsDrawn() > 0, true);
        auto values = [&] { return o->chartNow(0, 0).plots.value(0).series.value(0).values; };
        QCOMPARE(values(), (QVector<double>{10, 20, 30}));
        QCOMPARE(o->chartReads(0, 0).size(), 3);   // (name, categories, values)
        const QImage before = shot(e).copy(e.view()->sheet().layout().rangeRect(1, 4, 12, 10));
        // a typed cell
        e.select(QStringLiteral("B3"));
        e.enter(QStringLiteral("55"));
        QVERIFY(e.settle());
        QCOMPARE(values(), (QVector<double>{10, 55, 30}));
        QTRY_COMPARE(o->layer()->pendingRenders(), 0);
        o->layer()->prepareAll();
        const QImage typed = shot(e).copy(e.view()->sheet().layout().rangeRect(1, 4, 12, 10));
        QVERIFY(typed != before);
        // a cell a formula reads: the formula's result changes, and the chart with it
        const qint64 drawn = o->chartsDrawn();
        e.select(QStringLiteral("B2"));
        e.enter(QStringLiteral("20"));
        QVERIFY(e.settle());
        QCOMPARE(values(), (QVector<double>{20, 55, 60}));
        QTRY_COMPARE(o->layer()->pendingRenders(), 0);
        QVERIFY(o->chartsDrawn() > drawn);
        // a cell the chart does not read changes nothing of it
        o->layer()->prepareAll();
        const qint64 steady = o->chartsDrawn();
        e.select(QStringLiteral("C3"));
        e.enter(QStringLiteral("77"));
        QVERIFY(e.settle());
        QCOMPARE(o->layer()->pendingRenders(), 0);
        QCOMPARE(o->chartsDrawn(), steady);
        // the name, and a category
        e.select(QStringLiteral("B1"));
        e.enter(QStringLiteral("Eastern"));
        e.select(QStringLiteral("A3"));
        e.enter(QStringLiteral("Second"));
        QVERIFY(e.settle());
        QCOMPARE(o->chartNow(0, 0).plots[0].series[0].name, QStringLiteral("Eastern"));
        QCOMPARE(o->chartNow(0, 0).plots[0].series[0].categories.value(1), QStringLiteral("Second"));
        // undo puts the number back, and the chart follows
        pressUndo(e.view());
        pressUndo(e.view());
        QVERIFY(e.settle());
        QCOMPARE(o->chartNow(0, 0).plots[0].series[0].name, QStringLiteral("East"));
    }

    // Rows and columns inserted or deleted move the objects with the cells, and the chart's references with its cells; Undo puts them back; a hidden row is left out of the chart.
    void objectsAndChartsMoveWithTheCells()
    {
        EditWindow e;
        QVERIFY(e.open(drawingBook()));
        QTRY_VERIFY(shown(e, 5));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        auto top = [&](int id) { return o->layer()->deviceRect(id, grid::Pane::Body).top(); };
        const grid::GridLayout &L = e.view()->sheet().layout();
        QCOMPARE(top(1), L.cellRect(6, 0).top());
        QVERIFY(e.win.w.session()->insertRows(0, 0, 3).ok);
        QCOMPARE(top(1), L.cellRect(9, 0).top());          // (the picture is three rows lower: its anchor was at row 7)
        QCOMPARE(top(0), L.cellRect(4, 4).top());          // (and the chart: from row 2 to row 5)
        QCOMPARE(o->chartReads(0, 0).value(2).range.row1, quint32(4));   // (its values were B2:B4: they are B5:B7)
        QVERIFY(e.settle());
        QCOMPARE(o->chartNow(0, 0).plots[0].series[0].values, (QVector<double>{10, 20, 30}));
        pressUndo(e.view());
        QVERIFY(e.settle());
        QCOMPARE(top(1), L.cellRect(6, 0).top());
        QCOMPARE(top(0), L.cellRect(1, 4).top());
        QCOMPARE(o->chartNow(0, 0).plots[0].series[0].values, (QVector<double>{10, 20, 30}));
        // a column inserted before the chart, and a row hidden in its data
        QVERIFY(e.win.w.session()->insertColumns(0, 0, 2).ok);
        QCOMPARE(o->layer()->deviceRect(0, grid::Pane::Body).left(), L.cellRect(1, 6).left());
        pressUndo(e.view());
        crunch::EditSession::SizeChange hide;
        hide.setHidden = true;
        hide.hidden = true;
        QVERIFY(e.win.w.session()->resizeRows(0, 2, 2, hide, QStringLiteral("Hide")).ok);
        QVERIFY(e.settle());
        QCOMPARE(o->chartNow(0, 0).plots[0].series[0].values, (QVector<double>{10, 30}));
        QCOMPARE(o->chartNow(0, 0).plots[0].series[0].categories, (QStringList{"Q1", "Q3"}));
        // the rows the picture is over are deleted: it is gone from the screen (no height)
        pressUndo(e.view());
        QVERIFY(e.win.w.session()->deleteRows(0, 6, 3).ok);
        QVERIFY(o->layer()->deviceRect(1, grid::Pane::Body).isEmpty());
        pressUndo(e.view());
        QVERIFY(!o->layer()->deviceRect(1, grid::Pane::Body).isEmpty());
    }

    // Delete with an object picked takes it away as a step Undo gives back; it is saved without it, and with the numbers of a chart whose cells changed; what was not touched is the old
    // file's to the byte.
    void anObjectIsDeletedUndoneAndSaved()
    {
        EditWindow e;
        const QByteArray original = drawingBook();
        QVERIFY(e.open(original));
        QTRY_VERIFY(shown(e, 5));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        const grid::GridLayout &L = e.view()->sheet().layout();
        const QRect pic = L.rangeRect(6, 0, 8, 1);
        QVERIFY(!e.win.w.isModified());
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, pic.center());
        QCOMPARE(o->layer()->selectedId(), 1);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY(o->isDeleted(0, 1));
        QVERIFY(e.win.w.isModified());
        QCOMPARE(o->layer()->selectedId(), -1);
        QVERIFY(at(shot(e), pic.center()) != QColor(Qt::red).rgb());
        pressUndo(e.view());
        QVERIFY(!o->isDeleted(0, 1));
        QVERIFY(!e.win.w.isModified());
        o->layer()->prepareAll();
        QCOMPARE(at(shot(e), pic.center()), QColor(Qt::red).rgb());
        pressRedo(e.view());
        QVERIFY(o->isDeleted(0, 1));
        // a cell the chart reads is typed over too
        e.select(QStringLiteral("B3"));
        e.enter(QStringLiteral("55"));
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY(saved.wait(20000));
        office::Package before, after;
        QString why;
        QVERIFY(before.readLazy(original, &why));
        QVERIFY2(after.readLazyFile(e.path, &why), qPrintable(why));
        QDomDocument drawing;
        QVERIFY(office::parseXml(after.part(QStringLiteral("/xl/drawings/drawing1.xml")), &drawing));
        QStringList names;
        const QDomNodeList all = drawing.elementsByTagName(QStringLiteral("cNvPr"));
        for (int i = 0; i < all.size(); ++i) names << all.at(i).toElement().attribute(QStringLiteral("name"));
        QVERIFY2(!names.contains(QStringLiteral("Logo")) && names.contains(QStringLiteral("Sales")) && names.contains(QStringLiteral("Box")), qPrintable(names.join(QLatin1Char(','))));
        office::Chart chart;
        QVERIFY(office::readChart(after.part(QStringLiteral("/xl/charts/chart1.xml")), office::defaultTheme(), office::ColorMap(), &chart, &why));
        QCOMPARE(chart.plots[0].series[0].values, (QVector<double>{10, 55, 30}));   // (the cache says what the cells hold: the file stored 10, 20, 30)
        QCOMPARE(chart.plots[0].series[0].name, QStringLiteral("East"));
        // (the deleted picture's own part is gone with it, and its relationship, and its entry of the content types; the chart's, which is used, stays)
        QVERIFY(!after.hasPart(QStringLiteral("/xl/media/image1.png")));
        const QMap<QString, QByteArray> parts = savedParts(e.path);
        QVERIFY(!parts.contains(QStringLiteral("xl/media/image1.png")));
        QVERIFY(hasRelationship(savedParts(e.copyOfFile()), QStringLiteral("xl/drawings/_rels/drawing1.xml.rels"), QStringLiteral("rIdC")));
        QVERIFY2(!hasRelationship(parts, QStringLiteral("xl/drawings/_rels/drawing1.xml.rels"), QStringLiteral("rIdP")), "the picture's relationship is gone");
        QVERIFY2(!hasOverride(parts, QStringLiteral("/xl/media/image1.png")), "the picture's entry of the content types is gone");
        QVERIFY(hasOverride(parts, QStringLiteral("/xl/charts/chart1.xml")));
        QVERIFY(hasOverride(parts, QStringLiteral("/xl/drawings/drawing1.xml")));
        for (const QString &name : before.partNames()) {
            if (name == QLatin1String("/xl/drawings/drawing1.xml") || name == QLatin1String("/xl/charts/chart1.xml") || name.contains(QLatin1String("/worksheets/")) || name == QLatin1String("/xl/workbook.xml") || name == QLatin1String("/xl/media/image1.png")) continue;
            QVERIFY2(after.part(name) == before.part(name), qPrintable(name));
        }
        // the objects are as the file has them: four
        QCOMPARE(o->layer()->count(), 4);
        // a chart whose cells did not change keeps its part as it was, whatever else is saved
        e.select(QStringLiteral("C3"));
        e.enter(QStringLiteral("77"));
        QVERIFY(e.settle());
        const QByteArray chartBefore = after.part(QStringLiteral("/xl/charts/chart1.xml"));
        saved.clear();
        QVERIFY(e.win.w.save());
        QVERIFY(saved.wait(20000));
        office::Package third;
        QVERIFY(third.readLazyFile(e.path, &why));
        QCOMPARE(third.part(QStringLiteral("/xl/charts/chart1.xml")), chartBefore);
        QCOMPARE(third.part(QStringLiteral("/xl/drawings/drawing1.xml")), after.part(QStringLiteral("/xl/drawings/drawing1.xml")));
        // Undo goes back over the typing and no further: the object that was taken away is not in the file to be given back
        pressUndo(e.view());
        pressUndo(e.view());
        QVERIFY(!e.win.w.session()->canUndo());
        QCOMPARE(o->layer()->count(), 4);
        // the file opens with four objects
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 4);
    }

    // Hostile drawings: more anchors than may be read, groups nested past the limit, a picture that says it has ten billion pixels, a chart of many series that read whole columns, and
    // objects at the far corner of the sheet. The window opens each, makes what it can, and the work of a paint stays what the layer bounds it to.
    void hostileDrawingsAreBounded()
    {
        {
            QString many;
            for (int i = 0; i < jp::office::sheet::kMaxDrawingAnchors + 2000; ++i) many += twoCell(i % 40, i % 300, i % 40 + 1, i % 300 + 1, shapeElement(2 + i, QStringLiteral("S")));
            EditWindow e;
            QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(many), {})));
            QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0 && e.win.w.objects()->layer()->count() > 0);
            crunch::SheetObjects *o = e.win.w.objects();
            // the sheet is listed, and an object is made when a picture of it is wanted: not the ten thousand, but those on and near the screen
            QCOMPARE(o->layer()->count(), jp::office::sheet::kMaxDrawingAnchors);
            QCOMPARE(o->drawingsParsedApart(), 1);   // (a drawing of megabytes is parsed on a worker: the window's thread only reads what the parser made)
            QTRY_VERIFY(!o->layer()->busy());
            QVERIFY2(o->objectsMade() < jp::office::sheet::kMaxDrawingAnchors / 2, qPrintable(QString::number(o->objectsMade())));
            const qint64 renders = o->layer()->stats().renders;
            shot(e);   // (a paint of the window makes nothing)
            QCOMPARE(o->layer()->stats().renders, renders);
            QCOMPARE(o->layer()->stats().paintRenders, qint64(0));
            QVERIFY(o->layer()->cacheBytes() <= jp::grid::ObjectLayer::kCacheBytes);
            const crunch::DrawingCounts c = o->counts(0);   // (this one makes them all)
            QCOMPARE(c.anchors, jp::office::sheet::kMaxDrawingAnchors + 2000);
            QVERIFY(c.truncated);
            QCOMPARE(c.shapes, jp::office::sheet::kMaxDrawingAnchors);
            QVERIFY(!c.why.isEmpty());
            QCOMPARE(o->objectsMade(), jp::office::sheet::kMaxDrawingAnchors);
        }
        {
            QString nest = shapeElement(9, QStringLiteral("Core"));
            for (int i = 0; i < 120; ++i) nest = groupElement(10 + i, nest);
            EditWindow e;
            QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(twoCell(1, 1, 5, 5, nest)), {})));
            QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
            const crunch::DrawingCounts c = e.win.w.objects()->counts(0);
            QCOMPARE(c.groups, 1);
            QVERIFY(c.shown + c.leftOut == 1);   // (drawn, or left out in words: no crash, no loop)
            e.win.w.objects()->layer()->prepareAll();
            shot(e);
        }
        {
            EditWindow e;
            const QString anchors = twoCell(1, 1, 5, 5, pictureElement(2, QStringLiteral("rIdP"))) + twoCell(7, 1, 9, 3, shapeElement(3, QStringLiteral("Box")));
            QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(anchors), {drawtest::DrawingTarget{QStringLiteral("rIdP"), QStringLiteral("xl/media/image1.png"), hugePng(), QStringLiteral("image/png"), QStringLiteral("image")}})));
            QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
            e.win.w.objects()->layer()->prepareAll();
            shot(e);
            // the picture says it has ten billion pixels and holds none: no byte of one is decoded (counted in the bytes the drawing code holds decoded), and what the layer holds is the shape's
            QVERIFY(e.win.w.objects()->counts(0).pictures == 1);
            QCOMPARE(e.win.w.objects()->imageBytesDecoded(), qint64(0));
            QVERIFY(e.win.w.objects()->layer()->cacheBytes() < (qint64(1) << 20));
        }
        {
            // thirty series that read whole columns of a sheet that holds a few cells, and objects at the far corner
            QVector<SeriesSpec> series;
            for (int i = 0; i < 30; ++i) series.push_back(SeriesSpec{QString(), QStringLiteral("Sheet1!$A:$A"), QStringLiteral("Sheet1!$B:$B"), QStringLiteral("S"), {"a"}, {1}, QString()});
            const QString anchors = twoCell(0, 5, 10, 15, chartFrame(2, QStringLiteral("rIdC"))) + twoCell(16383, 1048575, 16383, 1048575, shapeElement(3, QStringLiteral("Corner")), QString(), 9525 * 900, 9525 * 900, 9525 * 900, 9525 * 900)
                                    + twoCell(99999999, 2000000000, 2147483647, 2147483647, shapeElement(4, QStringLiteral("Far")));
            EditWindow e;
            QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(anchors), {chartTarget(QStringLiteral("rIdC"), 1, chartPart(QStringLiteral("lineChart"), series))})));
            QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
            e.win.w.objects()->layer()->prepareAll();
            shot(e);
            const jp::office::Chart c = e.win.w.objects()->chartNow(0, 0);
            QCOMPARE(int(c.plots[0].series.size()), 30);
            QCOMPARE(int(c.plots[0].series[0].values.size()), 4);   // (the column read as far as the sheet's cells go)
            QVERIFY(e.win.w.objects()->layer()->cacheBytes() <= jp::grid::ObjectLayer::kCacheBytes);
        }
    }

    // A chart sheet is a chart and nothing else: no cell cursor, no typing into a cell or the formula bar, no heading or gridlines; the page is white behind a chart with no fill of its own.
    void aChartSheetIsOnlyAChart()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithChartSheet()));
        QCOMPARE(e.win.w.shownSheets(), (QVector<int>{0, 1}));
        QVERIFY(e.view()->cellsEnabled());
        // (the cell cursor on a sheet of cells: the color it is drawn in at the left edge of A1, and how much of it there is)
        e.select(QStringLiteral("A1"));
        const QRect a1 = e.view()->sheet().layout().cellRect(0, 0);
        const QRect around = a1.adjusted(-3, -3, 3, 3);
        auto count = [&](const QImage &img, QRgb color) {
            int n = 0;
            for (int y = around.top(); y <= around.bottom(); ++y)
                for (int x = around.left(); x <= around.right(); ++x) n += at(img, x, y) == color;
            return n;
        };
        const QImage cells = shot(e);
        QRgb cursor = 0xFFFFFFFF;
        for (int x = a1.left() - 1; x <= a1.left() + 2; ++x)
            if (qGray(at(cells, x, a1.center().y())) < qGray(cursor)) cursor = at(cells, x, a1.center().y());
        QVERIFY2(qGray(cursor) < 160, "the cursor is dark at the edge of the cell");
        QVERIFY(count(cells, cursor) >= a1.height());
        e.win.w.setActiveSheet(1);
        QTRY_VERIFY(shown(e, 1, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        QVERIFY(o->isChartSheet(1));
        QVERIFY(!e.view()->cellsEnabled());
        QVERIFY(e.win.w.formulaBar()->isReadOnly());
        QVERIFY(!e.win.w.nameBox()->isEnabled());
        o->layer()->prepareAll();
        const QImage img = shot(e);
        // a white page, the chart's ink on it, and no cell cursor or heading
        QCOMPARE(at(img, 2, 2), QColor(Qt::white).rgb());
        QCOMPARE(count(img, cursor), 0);   // (no cell cursor over the page, by its pixels)
        QSet<QRgb> colors;
        const QRect box = o->layer()->deviceRect(0, grid::Pane::Body);
        QVERIFY(!box.isEmpty());
        for (int y = box.top(); y < box.bottom(); y += 3)
            for (int x = box.left(); x < box.right(); x += 3) colors.insert(at(img, x, y));
        QVERIFY2(colors.size() >= 4, qPrintable(QString::number(colors.size())));
        QVERIFY(!e.view()->showHeadings());
        // nothing a person types or clicks starts an entry or changes a cell
        const QString before = e.selected();
        QTest::keyClick(e.view(), Qt::Key_X);
        QVERIFY(!e.win.w.cellEditor()->isVisible());
        QTest::keyClick(e.win.w.formulaBar(), Qt::Key_Y);
        QVERIFY(e.win.w.formulaBar()->text().isEmpty());
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(box.center()));
        QCOMPARE(e.selected(), before);
        QVERIFY(!e.win.w.isModified());
        // the first sheet has its cells and its bars again
        e.win.w.setActiveSheet(0);
        QVERIFY(e.view()->cellsEnabled());
        QVERIFY(!e.win.w.formulaBar()->isReadOnly());
        QVERIFY(e.win.w.nameBox()->isEnabled());
        QTRY_VERIFY(shown(e, 0, 0));
    }

    // The commands that work on cells are off while a chart sheet is shown (they would act on cells nobody sees), by the commands themselves and on the ribbon's buttons, whatever is
    // triggered (by the command, or by its keys) does nothing, and they are on again on a sheet of cells: those that were on before.
    void aChartSheetTurnsOffEveryCommandOnCells()
    {
        const QStringList commands = {"cells.insert", "cells.insert.cells", "cells.insert.rows", "cells.insert.columns", "cells.delete", "cells.delete.cells", "cells.delete.rows", "cells.delete.columns",
                                      "cells.format", "format.rowHeight", "format.columnWidth", "format.fitRow", "format.fitColumn", "format.hideRows", "format.hideColumns", "format.unhideRows",
                                      "format.unhideColumns", "edit.cut", "edit.copy", "edit.paste", "paste.values", "paste.formulas", "paste.formats", "paste.transpose", "paste.special",
                                      "fill.down", "fill.right", "fill.up", "fill.left", "fill.series", "edit.fill", "edit.autosum", "edit.autosum.sum", "edit.autosum.average",
                                      "edit.autosum.count", "edit.autosum.max", "edit.autosum.min", "edit.selectAll", "edit.clear", "edit.sort", "edit.find", "view.formulas", "view.gridlines",
                                      "view.headings", "view.freeze", "view.freezeRow", "view.freezeCol", "view.unfreeze"};
        EditWindow e;
        QVERIFY(e.open(bookWithChartSheet()));
        CrunchWindow &w = e.win.w;
        QStringList on;
        for (const QString &id : commands) {
            QVERIFY2(w.act(id), qPrintable(id));
            if (w.act(id)->isEnabled()) on << id;
        }
        // (the ones that work are on, on a sheet of cells)
        for (const char *id : {"cells.insert", "cells.delete", "cells.format", "edit.autosum", "edit.fill", "edit.cut", "edit.copy", "edit.paste", "fill.down", "format.rowHeight", "view.freeze"})
            QVERIFY2(on.contains(QLatin1String(id)), id);
        w.setActiveSheet(1);
        QTRY_VERIFY(shown(e, 1, 1));
        for (const QString &id : commands) QVERIFY2(!w.act(id)->isEnabled(), qPrintable(id));
        // the buttons of the ribbon that carry them are off
        int buttons = 0;
        for (QToolButton *b : w.findChildren<QToolButton *>())
            if (b->defaultAction() && commands.contains(b->defaultAction()->objectName())) {
                ++buttons;
                QVERIFY2(!b->isEnabled(), qPrintable(b->defaultAction()->objectName()));
            }
        QVERIFY2(buttons >= 8, qPrintable(QString::number(buttons)));
        // triggering any of them, and pressing their keys, changes nothing: no step, no change of the file, no notice
        const bool undoable = w.session()->canUndo();
        for (const QString &id : commands) w.act(id)->trigger();
        for (const Qt::Key key : {Qt::Key_Plus, Qt::Key_Minus, Qt::Key_9, Qt::Key_0, Qt::Key_C, Qt::Key_X, Qt::Key_V, Qt::Key_A, Qt::Key_D, Qt::Key_R})
            QTest::keyClick(e.view(), key, Qt::ControlModifier);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QCOMPARE(w.session()->canUndo(), undoable);
        QVERIFY(!w.isModified());
        QVERIFY(e.win.told.isEmpty());
        QVERIFY(!w.cellEditor()->isVisible());
        // back on a sheet of cells: the ones that were on are on, the ones that were off (not yet available) are off
        w.setActiveSheet(0);
        QVERIFY(e.view()->cellsEnabled());
        for (const QString &id : commands) QCOMPARE(w.act(id)->isEnabled(), on.contains(id));
        // and a command works there again
        e.select(QStringLiteral("B2"));
        w.act("cells.insert.rows")->trigger();
        QVERIFY(w.isModified());
    }

    // Two objects use one picture, by one relationship or by two: deleting one leaves the picture, the relationship the other uses, and the picture's entry of the content types; deleting the
    // other takes them. (With two relationships to the picture the deleted object's own goes, and the picture stays because the other still goes to it.)
    void aPictureTwoObjectsUseStaysUntilBothAreGone()
    {
        for (const bool oneRelationship : {true, false}) {
            const QString secondId = oneRelationship ? QStringLiteral("rIdP") : QStringLiteral("rIdQ");
            const QString anchors = twoCell(0, 6, 2, 9, pictureElement(3, QStringLiteral("rIdP"), QStringLiteral("First"))) + twoCell(4, 6, 6, 9, pictureElement(4, secondId, QStringLiteral("Second")))
                                    + twoCell(0, 11, 2, 13, shapeElement(5, QStringLiteral("Box")));
            QVector<DrawingTarget> targets = {imageTarget(QStringLiteral("rIdP"), 1, pngBytes(8, 4, Qt::red))};
            if (!oneRelationship) targets.push_back(DrawingTarget{QStringLiteral("rIdQ"), QStringLiteral("xl/media/image1.png"), pngBytes(8, 4, Qt::red), QString(), QStringLiteral("image")});
            EditWindow e;
            QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(anchors), targets)));
            QTRY_VERIFY(shown(e, 3));
            crunch::SheetObjects *o = e.win.w.objects();
            auto save = [&]() -> bool {
                QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
                return e.win.w.save() && saved.wait(20000);
            };
            const QString rels = QStringLiteral("xl/drawings/_rels/drawing1.xml.rels");
            QVERIFY(hasRelationship(savedParts(e.path), rels, QStringLiteral("rIdP")));
            QVERIFY(e.win.w.session()->deleteObject(0, 0, QStringLiteral("Delete Object")).ok);
            QVERIFY(save());
            QMap<QString, QByteArray> parts = savedParts(e.path);
            QVERIFY2(parts.contains(QStringLiteral("xl/media/image1.png")), "the picture the other uses stays");
            QVERIFY(hasRelationship(parts, rels, secondId));
            QCOMPARE(hasRelationship(parts, rels, QStringLiteral("rIdP")), oneRelationship);   // (the one the deleted object had, if the other does not use it)
            QVERIFY(hasOverride(parts, QStringLiteral("/xl/media/image1.png")));
            QCOMPARE(o->layer()->count(), 2);
            QVERIFY(e.win.w.session()->deleteObject(0, 1, QStringLiteral("Delete Object")).ok);
            QVERIFY(save());
            parts = savedParts(e.path);
            QVERIFY2(!parts.contains(QStringLiteral("xl/media/image1.png")), "the last that used it is gone, and the picture");
            QVERIFY(!hasRelationship(parts, rels, QStringLiteral("rIdP")));
            QVERIFY(!hasRelationship(parts, rels, QStringLiteral("rIdQ")));
            QVERIFY(!hasOverride(parts, QStringLiteral("/xl/media/image1.png")));
            QCOMPARE(o->layer()->count(), 1);   // (the shape)
            // the file opens with the one object
            Window second;
            QVERIFY(second.open(e.copyOfFile()));
            QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 1);
        }
    }

    // An object anchored at a place of the sheet (an absolute anchor) keeps its place and size when rows and columns are inserted and resized, saved and opened again: on the screen,
    // and in the file, where its anchor is as it was.
    void anAbsoluteAnchorKeepsItsPlaceAndSize()
    {
        const qint64 emu = 9525;
        const QString anchors = absolute(300 * emu, 100 * emu, 120 * emu, 60 * emu, shapeElement(2, QStringLiteral("Fixed"))) + twoCell(0, 8, 2, 10, shapeElement(3, QStringLiteral("Moving")));
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(anchors), {})));
        QTRY_VERIFY(shown(e, 2));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        const QRect fixed = o->layer()->deviceRect(0, grid::Pane::Body), moving = o->layer()->deviceRect(1, grid::Pane::Body);
        QCOMPARE(fixed.size(), QSize(120, 60));
        QCOMPARE(fixed.topLeft() - e.view()->sheet().layout().gridRect().topLeft(), QPoint(300, 100));
        QVERIFY(e.win.w.session()->insertRows(0, 0, 4).ok);
        QVERIFY(e.win.w.session()->insertColumns(0, 0, 2).ok);
        crunch::EditSession::SizeChange tall;
        tall.setSize = true;
        tall.size = 60;   // (points)
        QVERIFY(e.win.w.session()->resizeRows(0, 2, 5, tall, QStringLiteral("Taller")).ok);
        QVERIFY(e.win.w.session()->resizeColumns(0, 0, 3, tall, QStringLiteral("Wider")).ok);
        QCOMPARE(o->layer()->deviceRect(0, grid::Pane::Body), fixed);        // (where it was, as large as it was)
        QVERIFY(o->layer()->deviceRect(1, grid::Pane::Body) != moving);      // (the object of the cells moved)
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY(saved.wait(20000));
        // the file has the anchor as it was: an absolute one, the same place and size
        office::Package p;
        QString why;
        QVERIFY2(p.readLazyFile(e.path, &why), qPrintable(why));
        QDomDocument doc;
        QVERIFY(office::parseXml(p.part(QStringLiteral("/xl/drawings/drawing1.xml")), &doc));
        bool found = false;
        const QDomNodeList absolutes = doc.elementsByTagName(QStringLiteral("absoluteAnchor"));
        for (int i = 0; i < absolutes.size(); ++i) {
            const QDomElement a = absolutes.at(i).toElement();
            const QDomElement pos = a.firstChildElement(QStringLiteral("pos")), ext = a.firstChildElement(QStringLiteral("ext"));
            if (a.elementsByTagName(QStringLiteral("cNvPr")).at(0).toElement().attribute(QStringLiteral("name")) != QLatin1String("Fixed")) continue;
            found = true;
            QCOMPARE(pos.attribute(QStringLiteral("x")).toLongLong(), 300 * emu);
            QCOMPARE(pos.attribute(QStringLiteral("y")).toLongLong(), 100 * emu);
            QCOMPARE(ext.attribute(QStringLiteral("cx")).toLongLong(), 120 * emu);
            QCOMPARE(ext.attribute(QStringLiteral("cy")).toLongLong(), 60 * emu);
        }
        QVERIFY2(found, "the absolute anchor is still one");
        // opened again, it is where it was
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 2);
        second.w.objects()->layer()->prepareAll();
        QCOMPARE(second.w.objects()->layer()->deviceRect(0, grid::Pane::Body).size(), fixed.size());
        QCOMPARE(second.w.objects()->layer()->deviceRect(0, grid::Pane::Body).topLeft() - second.w.view()->sheet().layout().gridRect().topLeft(), QPoint(300, 100));
    }

    // A name of the sheet itself in a chart on the second sheet, with no sheet in front of it, is that sheet's name: the numbers of that sheet's cells, not those of the first sheet's name
    // that has the same words.
    void aSheetLocalNameInAChartOnTheSecondSheetIsThatSheets()
    {
        auto numbers = [](int scale) {
            QString rows;
            for (int i = 0; i < 3; ++i) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\" t=\"inlineStr\"><is><t>Q%1</t></is></c><c r=\"B%1\"><v>%2</v></c></row>").arg(i + 2).arg((i + 1) * scale);
            return worksheet(rows);
        };
        const QVector<SeriesSpec> series = {SeriesSpec{QString(), QString(), QStringLiteral("Local"), QStringLiteral("S"), {}, {1, 2, 3}, QString()}};
        const QString names = QStringLiteral("<definedName name=\"Local\" localSheetId=\"0\">Sheet1!$B$2:$B$4</definedName><definedName name=\"Local\" localSheetId=\"1\">Sheet2!$B$2:$B$4</definedName>");
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({numbers(10), numbers(100)}, drawingPart(twoCell(4, 1, 11, 13, chartFrame(2, QStringLiteral("rIdC"), QStringLiteral("Sales")))),
                                       {chartTarget(QStringLiteral("rIdC"), 1, chartPart(QStringLiteral("barChart"), series))}, QStringList(), names, QByteArray(), 1)));
        crunch::SheetObjects *o = e.win.w.objects();
        QCOMPARE(o->chartNow(1, 0).plots[0].series[0].values, (QVector<double>{100, 200, 300}));   // (the second sheet's own: not the first's 10, 20, 30)
        QVERIFY(e.settle());
        QCOMPARE(o->chartReads(1, 0).value(0).sheet, 1);
    }

    // A part that several anchors name is read once, and a chart drawn again for its cells is not read from its part again.
    void aChartPartIsReadOnce()
    {
        const QVector<SeriesSpec> series = {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {"Q1", "Q2", "Q3"}, {10, 20, 30}, QString()}};
        QString anchors;
        for (int i = 0; i < 6; ++i) anchors += twoCell(4 + i, 1, 6 + i, 8, chartFrame(2 + i, QStringLiteral("rIdC"), QStringLiteral("Sales")));
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(anchors), {chartTarget(QStringLiteral("rIdC"), 1, chartPart(QStringLiteral("barChart"), series))})));
        QTRY_VERIFY(shown(e, 6));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        QCOMPARE(o->chartPartsRead(), 1);
        e.select(QStringLiteral("B3"));
        e.enter(QStringLiteral("55"));
        QVERIFY(e.settle());
        o->layer()->prepareAll();
        QCOMPARE(o->chartNow(0, 3).plots[0].series[0].values, (QVector<double>{10, 55, 30}));
        QCOMPARE(o->chartPartsRead(), 1);
    }

    // Saving looks for the charts in the parts of the file: a sheet that was never shown has its chart's numbers made again, and its drawing is not read, nor an object of it made.
    void savingDoesNotBuildTheSheetsNeverShown()
    {
        const QVector<SeriesSpec> series = {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {"Q1", "Q2", "Q3"}, {10, 20, 30}, QString()}};
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({dataSheet(), dataSheet()}, drawingPart(twoCell(1, 1, 8, 12, chartFrame(2, QStringLiteral("rIdC"), QStringLiteral("Sales")))),
                                       {chartTarget(QStringLiteral("rIdC"), 1, chartPart(QStringLiteral("barChart"), series))}, QStringList(), QString(), QByteArray(), 1)));
        crunch::SheetObjects *o = e.win.w.objects();
        QTRY_VERIFY(o->listed(0));
        e.select(QStringLiteral("B3"));
        e.enter(QStringLiteral("55"));
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY(saved.wait(20000));
        QVERIFY(!o->listed(1));
        QCOMPARE(o->objectsMade(), 0);
        QCOMPARE(o->chartPartsRead(), 0);
        office::Package after;
        QString why;
        QVERIFY2(after.readLazyFile(e.path, &why), qPrintable(why));
        office::Chart chart;
        QVERIFY(office::readChart(after.part(QStringLiteral("/xl/charts/chart1.xml")), office::defaultTheme(), office::ColorMap(), &chart, &why));
        QCOMPARE(chart.plots[0].series[0].values, (QVector<double>{10, 55, 30}));
    }

    // A shape that keeps its size when cells are resized (editAs "oneCell") does: a row inserted inside its span does not make it taller, and a taller row under it does not either; and the
    // anchor saved is the one that gives it the size it has.
    void anObjectThatKeepsItsSizeKeepsItAndIsSavedSo()
    {
        {
            EditWindow e;
            QVERIFY(e.open(drawingBook()));
            QTRY_VERIFY(shown(e, 5));
            crunch::SheetObjects *o = e.win.w.objects();
            o->layer()->prepareAll();
            const QRect box = o->layer()->deviceRect(2, grid::Pane::Body);
            QCOMPARE(savedAnchor(e.path, QStringLiteral("Box")).toRow, 12);
            QVERIFY(e.win.w.session()->insertRows(0, 11, 1).ok);   // (inside its two rows)
            const QRect now = o->layer()->deviceRect(2, grid::Pane::Body);
            QCOMPARE(now.height(), box.height());
            QCOMPARE(now.top(), box.top());
            QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
            QVERIFY(e.win.w.save());
            QVERIFY(saved.wait(20000));
            const SavedAnchor a = savedAnchor(e.path, QStringLiteral("Box"));
            QVERIFY(a.found);
            QCOMPARE(a.fromRow, 10);
            QCOMPARE(a.toRow, 12);   // (as tall as before: not 13)
            QCOMPARE(a.editAs, QStringLiteral("oneCell"));
            // a taller row under it: it keeps its height, and the file says where that ends
            crunch::EditSession::SizeChange tall;
            tall.setSize = true;
            tall.size = 60;   // (points)
            QVERIFY(e.win.w.session()->resizeRows(0, 10, 10, tall, QStringLiteral("Taller")).ok);
            QCOMPARE(o->layer()->deviceRect(2, grid::Pane::Body).height(), box.height());
            saved.clear();
            QVERIFY(e.win.w.save());
            QVERIFY(saved.wait(20000));
            const SavedAnchor b = savedAnchor(e.path, QStringLiteral("Box"));
            QVERIFY(b.found);
            QCOMPARE(b.fromRow, 10);
            QVERIFY2(b.toRow < 12, qPrintable(QString::number(b.toRow)));   // (a row of 80 pixels holds most of its 40)
            QCOMPARE(b.editAs, QStringLiteral("oneCell"));
        }
        {
            // an object that is not told to keep its size grows with the rows inserted inside it (the chart, over rows 1 to 13)
            EditWindow e;
            QVERIFY(e.open(drawingBook()));
            QTRY_VERIFY(shown(e, 5));
            crunch::SheetObjects *o = e.win.w.objects();
            o->layer()->prepareAll();
            const int height = o->layer()->deviceRect(0, grid::Pane::Body).height();
            QVERIFY(e.win.w.session()->insertRows(0, 5, 2).ok);
            QCOMPARE(o->layer()->deviceRect(0, grid::Pane::Body).height(), height + 2 * e.view()->sheet().layout().cellRect(5, 0).height());
        }
    }

    // A hidden row stays hidden from the chart when rows are inserted above it: the chart reads the rows it read, and still leaves the hidden one out.
    void aHiddenRowStaysLeftOutWhenRowsAreInserted()
    {
        EditWindow e;
        QVERIFY(e.open(drawingBook()));
        QTRY_VERIFY(shown(e, 5));
        crunch::SheetObjects *o = e.win.w.objects();
        crunch::EditSession::SizeChange hide;
        hide.setHidden = true;
        hide.hidden = true;
        QVERIFY(e.win.w.session()->resizeRows(0, 2, 2, hide, QStringLiteral("Hide")).ok);
        QVERIFY(e.settle());
        QCOMPARE(o->chartNow(0, 0).plots[0].series[0].values, (QVector<double>{10, 30}));
        QVERIFY(e.win.w.session()->insertRows(0, 0, 2).ok);
        QVERIFY(e.settle());
        QCOMPARE(o->chartNow(0, 0).plots[0].series[0].values, (QVector<double>{10, 30}));
        QCOMPARE(o->chartNow(0, 0).plots[0].series[0].categories, (QStringList{"Q1", "Q3"}));
        // and the same when the hidden row's place is deleted along with the rows above it, and undone
        pressUndo(e.view());
        QVERIFY(e.settle());
        QCOMPARE(o->chartNow(0, 0).plots[0].series[0].values, (QVector<double>{10, 30}));
    }

    // Rows inserted, and rows made taller, move the objects in place: the pictures made stay (no object is made again, no picture drawn again), and the object picked is still the one picked.
    void insertingRowsMovesTheObjectsInPlace()
    {
        EditWindow e;
        QVERIFY(e.open(drawingBook()));
        QTRY_VERIFY(shown(e, 5));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        const grid::GridLayout &L = e.view()->sheet().layout();
        // (what the window does after an edit is queued: it is let run before anything is looked at)
        auto letRun = [] {
            QCoreApplication::processEvents();
            QCoreApplication::sendPostedEvents(nullptr, 0);
            QCoreApplication::processEvents();
        };
        // the picture is picked
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, L.rangeRect(6, 0, 8, 1).center());
        QCOMPARE(o->layer()->selectedId(), 1);
        QTRY_COMPARE(o->layer()->pendingRenders(), 0);
        const jp::grid::ObjectLayer::Stats before = o->layer()->stats();
        const int made = o->objectsMade();
        const qint64 charts = o->chartsDrawn();
        const qint64 held = o->layer()->cacheBytes();
        QVERIFY(held > 0);
        const int top = o->layer()->deviceRect(1, grid::Pane::Body).top();
        // below every object, between them, and above them: the objects below move, none is made or drawn again, and the one picked stays picked
        QVERIFY(e.win.w.session()->insertRows(0, 30, 2).ok);
        QVERIFY(e.win.w.session()->insertRows(0, 14, 2).ok);
        letRun();
        QCOMPARE(o->layer()->deviceRect(1, grid::Pane::Body).top(), top);
        QCOMPARE(o->layer()->selectedId(), 1);
        QVERIFY(e.win.w.session()->insertRows(0, 0, 3).ok);
        letRun();
        QCOMPARE(o->layer()->deviceRect(1, grid::Pane::Body).top(), L.cellRect(9, 0).top());
        QCOMPARE(o->layer()->selectedId(), 1);
        // a row made taller, under no object
        crunch::EditSession::SizeChange tall;
        tall.setSize = true;
        tall.size = 60;
        QVERIFY(e.win.w.session()->resizeRows(0, 40, 40, tall, QStringLiteral("Taller")).ok);
        letRun();
        QCOMPARE(o->layer()->selectedId(), 1);
        QCOMPARE(o->layer()->cacheBytes(), held);
        o->layer()->prepareAll();
        QTRY_COMPARE(o->layer()->pendingRenders(), 0);
        QCOMPARE(o->objectsMade(), made);
        QCOMPARE(o->chartsDrawn(), charts);
        QCOMPARE(o->layer()->stats().renders, before.renders);
        QCOMPARE(o->layer()->cacheBytes(), held);
        // Undo and Redo of them keep the one picked, too
        pressUndo(e.view());
        pressUndo(e.view());
        letRun();
        QCOMPARE(o->layer()->selectedId(), 1);
        QCOMPARE(o->layer()->deviceRect(1, grid::Pane::Body).top(), top);   // (back where it was: the rows above it are as they were)
        pressRedo(e.view());
        letRun();
        QCOMPARE(o->layer()->selectedId(), 1);
        QCOMPARE(o->objectsMade(), made);
    }

    // A chart is drawn again when what it reads is not what it was drawn from, wherever it is: a row inserted among its cells (so there is a blank between) does it, though the chart only
    // moves down; a row inserted above them (they all move together) does not.
    void aChartIsDrawnAgainOnlyWhenItsNumbersChange()
    {
        const QVector<SeriesSpec> series = {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {"Q1", "Q2", "Q3"}, {10, 20, 30}, QString()}};
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(twoCell(4, 8, 11, 18, chartFrame(2, QStringLiteral("rIdC"), QStringLiteral("Sales")))), {chartTarget(QStringLiteral("rIdC"), 1, chartPart(QStringLiteral("barChart"), series))})));
        QTRY_VERIFY(shown(e, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        QTRY_COMPARE(o->layer()->pendingRenders(), 0);
        const qint64 first = o->chartsDrawn();
        QVERIFY(first > 0);
        const int height = o->layer()->deviceRect(0, grid::Pane::Body).height();
        QVERIFY(e.win.w.session()->insertRows(0, 0, 1).ok);
        QVERIFY(e.settle());
        o->layer()->prepareAll();
        QTRY_COMPARE(o->layer()->pendingRenders(), 0);
        QCOMPARE(o->chartsDrawn(), first);   // (the same three numbers, a row lower)
        QVERIFY(e.win.w.session()->insertRows(0, 3, 1).ok);   // (inside B3:B5, now where they are)
        QVERIFY(e.settle());
        QCOMPARE(o->layer()->deviceRect(0, grid::Pane::Body).height(), height);   // (the chart is below them: it moves, and is as large)
        QCOMPARE(o->chartNow(0, 0).plots[0].series[0].values.size(), 4);
        o->layer()->prepareAll();
        QTRY_COMPARE(o->layer()->pendingRenders(), 0);
        QVERIFY(o->chartsDrawn() > first);
    }

    // Each edit of rows is worked into the objects once, however many came before it (a hundred insertions are not a hundred times the work of the last), and Undo goes back over all.
    void manyEditsAreWorkedInOneAtATime()
    {
        EditWindow e;
        QVERIFY(e.open(drawingBook()));
        QTRY_VERIFY(shown(e, 5));
        crunch::SheetObjects *o = e.win.w.objects();
        const qint64 before = o->editSteps();
        for (int i = 0; i < 40; ++i) QVERIFY(e.win.w.session()->insertRows(0, 20 + quint32(i), 1).ok);
        QCOMPARE(o->editSteps() - before, qint64(40 * 5));   // (five objects, each given each edit once)
        const grid::GridLayout &L = e.view()->sheet().layout();
        o->layer()->prepareAll();
        QCOMPARE(o->layer()->deviceRect(1, grid::Pane::Body).top(), L.cellRect(6, 0).top());
        pressUndo(e.view());
        QCOMPARE(o->layer()->deviceRect(2, grid::Pane::Body).top(), L.cellRect(10, 0).top());
        // (an Undo is worked out from the objects as they were: each of the edits left, for each object)
        QCOMPARE(o->editSteps() - before, qint64(40 * 5 + 5 * 39));
    }

    // The program that saves a picture of its window and quits ends, with the window still open: the worker that calculates for the window is told to stop, or the program would wait
    // for it for ever as it ended (the hang was in the end of the program, after the picture was written).
    void aScreenshotRunEndsWithItsWindowOpen()
    {
        // (the program this build makes: its path is the build's, and the tests are not built without it; a test that did not run for want of it would say nothing)
        const QString exe = QStringLiteral(JC_CRUNCH_EXE);
        QVERIFY2(QFileInfo::exists(exe), qPrintable(exe));
        Folder folder;
        const QString book = folder.write(QStringLiteral("book.xlsx"), drawingBook());
        const QString png = folder.dir.filePath(QStringLiteral("shot.png"));
        QProcess child;
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("offscreen"));
        env.insert(QStringLiteral("JP_NO_TELEMETRY"), QStringLiteral("1"));
        child.setProcessEnvironment(env);
        child.start(exe, {QStringLiteral("--screenshot"), png, QStringLiteral("--size"), QStringLiteral("700x500"), book});
        QVERIFY(child.waitForStarted());
        // (a program that does not end is told by this bound; one that is right ends in a few seconds)
        const bool ended = child.waitForFinished(120000);
        if (!ended) {
            child.kill();
            child.waitForFinished();
        }
        QVERIFY2(ended, "the program did not end after saving its picture");
        QCOMPARE(child.exitStatus(), QProcess::NormalExit);
        QCOMPARE(child.exitCode(), 0);
        const QImage img(png);
        QVERIFY(!img.isNull());
        QVERIFY(img.width() >= 700 && img.height() >= 500);
    }

    // A drawing that is parsed on a worker, and asked about before it is done (the sheet's counts want it now): it is listed there and then, the worker's answer is let go, and the
    // objects still come into the layer when the answer arrives (they were wanted on the screen).
    void aDrawingBeingParsedIsListedWhenAskedAndStillShown()
    {
        QString many;
        const int n = 3000;
        for (int i = 0; i < n; ++i) many += twoCell(i % 40, i % 300, i % 40 + 1, i % 300 + 1, shapeElement(2 + i, QStringLiteral("S"), QStringLiteral("rect"), QStringLiteral("some words to make it larger than a few bytes")));
        const QByteArray bytes = bookWithDrawing({dataSheet()}, drawingPart(many), {});
        Folder folder;
        auto book = std::make_shared<office::sheet::Workbook>();
        QString why;
        QVERIFY2(book->openFile(folder.write(QStringLiteral("many.xlsx"), bytes), &why), qPrintable(why));
        crunch::SheetObjects objects;
        objects.open(book);
        objects.show(0);   // (the parsing is given to a worker; the layer is empty and the sheet is the one shown)
        QCOMPARE(objects.shownSheet(), 0);
        QCOMPARE(objects.layer()->count(), 0);
        QVERIFY(!objects.listed(0));
        const crunch::DrawingCounts c = objects.counts(0);   // (wants the drawing now: listed here, with the events not yet run)
        QCOMPARE(c.shapes, n);
        QVERIFY(objects.listed(0));
        QCOMPARE(objects.drawingsParsedApart(), 0);
        QTRY_COMPARE(objects.layer()->count(), n);   // (the worker's answer is let go, and the objects come in)
        QCOMPARE(objects.drawingsParsedApart(), 0);
        // the window says the sheet's rows and columns are whole (a workbook that is read whole) while the drawing is still being parsed: that does not list it on the window's thread
        crunch::SheetObjects third;
        third.open(book);
        third.show(0);
        third.geometryKnown(0);
        QVERIFY(!third.listed(0));
        QTRY_COMPARE(third.layer()->count(), n);
        QCOMPARE(third.drawingsParsedApart(), 1);
        // and a workbook opened again meanwhile is not given the answer of the one before (this one has fewer shapes, and is waited for as well)
        QString fewer;
        for (int i = 0; i < 2000; ++i) fewer += twoCell(i % 40, i % 300, i % 40 + 1, i % 300 + 1, shapeElement(2 + i, QStringLiteral("S"), QStringLiteral("rect"), QStringLiteral("some words to make it larger than a few bytes")));
        crunch::SheetObjects second;
        second.open(book);
        second.show(0);
        auto other = std::make_shared<office::sheet::Workbook>();
        QVERIFY2(other->openFile(folder.write(QStringLiteral("other.xlsx"), bookWithDrawing({dataSheet()}, drawingPart(fewer), {})), &why), qPrintable(why));
        second.open(other);
        second.show(0);
        QThreadPool::globalInstance()->waitForDone();   // (the worker has parsed both drawings, and posted its answers: the first is for the workbook before)
        QTRY_COMPARE(second.layer()->count(), 2000);
        QCOMPARE(second.drawingsParsedApart(), 1);
    }

    // A read that is told to have failed after the first screen was shown (the loader says so only before it, today, but the window must not depend on that) leaves no object over the
    // blank grid it goes back to, and no cell cursor missing.
    void aFailureAfterTheFirstScreenLeavesNoObjects()
    {
        EditWindow e;
        QVERIFY(e.open(drawingBook()));
        QTRY_VERIFY(shown(e, 5));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, e.view()->sheet().layout().rangeRect(6, 0, 8, 1).center());
        QCOMPARE(o->layer()->selectedId(), 1);
        Q_EMIT e.win.w.workbookLoader().finished(false, QStringLiteral("The file ends too soon."));
        QCOMPARE(o->layer()->count(), 0);
        QCOMPARE(o->layer()->selectedId(), -1);
        QCOMPARE(o->shownSheet(), -1);
        QVERIFY(e.view()->cellsEnabled());
        QCOMPARE(e.win.told.size(), 1);
        QVERIFY2(e.win.told[0].contains(QLatin1String("ends too soon")), qPrintable(e.win.told[0]));
        shot(e);
        QVERIFY(!e.win.w.hasWorkbook());
    }

    // Two objects the file keeps in one element (a wrapper with two shapes in it) are not taken away one at a time: Delete says why, and both stay.
    void objectsKeptTogetherAreNotDeletedApart() 
    {
        const QString wrapped = QStringLiteral("<mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\"><mc:Choice Requires=\"a14\">%1%2</mc:Choice><mc:Fallback/></mc:AlternateContent>")
                                    .arg(twoCell(0, 2, 2, 4, shapeElement(2, QStringLiteral("Left"), QStringLiteral("rect"), QString(), QStringLiteral("CC3333"))),
                                         twoCell(3, 2, 5, 4, shapeElement(3, QStringLiteral("Right"), QStringLiteral("rect"), QString(), QStringLiteral("33CC33"))));
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(twoCell(0, 8, 2, 10, shapeElement(4, QStringLiteral("Alone"))) + wrapped), {})));
        QTRY_VERIFY(shown(e, 3));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        const grid::GridLayout &L = e.view()->sheet().layout();
        QString why;
        QVERIFY(o->canDelete(0, 0, &why));   // (the one that is alone)
        QVERIFY(!o->canDelete(0, 1, &why));
        QVERIFY2(why.contains(QLatin1String("together")), qPrintable(why));
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, L.rangeRect(2, 0, 3, 1).center());
        QCOMPARE(o->layer()->selectedId(), 1);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QCOMPARE(e.win.told.size(), 1);
        QVERIFY2(e.win.told[0].contains(QLatin1String("together")), qPrintable(e.win.told[0]));
        QVERIFY(!o->isDeleted(0, 1));
        QVERIFY(!o->isDeleted(0, 2));
        QVERIFY(!e.win.w.isModified());
        QCOMPARE(o->layer()->count(), 3);
        // the one that is alone is taken away as before
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, L.rangeRect(8, 0, 9, 1).center());
        QCOMPARE(o->layer()->selectedId(), 0);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY(o->isDeleted(0, 0));
        QCOMPARE(e.win.told.size(), 1);
    }
};

JP_TEST_CLASS(CrunchDrawingTests)
#include "tst_crunch_drawings.moc"
