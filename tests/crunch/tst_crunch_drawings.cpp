// Tests of the charts, pictures, and shapes over the cells in JeffCrunch's window: they are shown where their anchors put them, a chart is drawn from the cells it reads and again when one
// changes (typed, or worked out), and objects and charts move when rows are inserted or deleted, undone and redone. Every workbook is built here.
#include "../sheet/drawinghelpers.h"
#include "editwindow.h"

#include "core/sheetobjects.h"
#include "office/chart.h"
#include "office/package.h"
#include "office/theme.h"
#include "office/xml.h"

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

} // namespace

class CrunchDrawingTests : public QObject {
    Q_OBJECT

    // The picture of the viewport, and where an object is in it.
    static QImage shot(EditWindow &e)
    {
        e.view()->viewport()->repaint();
        return e.view()->viewport()->grab().toImage();
    }
    static bool shown(EditWindow &e, int count)
    {
        crunch::SheetObjects *o = e.win.w.objects();
        return o && o->shownSheet() == 0 && o->layer()->count() == count;
    }

private Q_SLOTS:
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
        o->layer()->prepareAll();
        const QImage img = shot(e);
        // the picture: red where the anchor puts it (A7 to B9, rows 6 to 8), the shape blue (A11 to B12)
        const grid::GridLayout &L = e.view()->sheet().layout();
        const QRect pic = L.rangeRect(6, 0, 8, 1), box = L.rangeRect(10, 0, 11, 1), chart = L.rangeRect(1, 4, 12, 10), group = L.rangeRect(14, 3, 15, 5);
        QCOMPARE(img.pixel(pic.center()), QColor(Qt::red).rgb());
        QCOMPARE(img.pixel(box.left() + 6, box.top() + 6), QColor(0x33, 0x66, 0xCC).rgb());
        QVERIFY(img.pixel(box.left() - 2, box.center().y()) != QColor(0x33, 0x66, 0xCC).rgb());
        QCOMPARE(img.pixel(group.left() + 6, group.top() + 6), QColor(0x00, 0xAA, 0x00).rgb());
        // the chart: a picture with more than the grid's white and gray in it, inside its anchor and not outside
        QSet<QRgb> colors;
        for (int y = chart.top(); y < chart.bottom(); y += 3)
            for (int x = chart.left(); x < chart.right(); x += 3) colors.insert(img.pixel(x, y));
        QVERIFY2(colors.size() >= 4, qPrintable(QString::number(colors.size())));
        QCOMPARE(img.pixel(chart.right() + 4, chart.center().y()), img.pixel(chart.right() + 8, chart.center().y()));   // (the plain cells beside it)
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
        QVERIFY(shot(e).pixel(pic.center()) != QColor(Qt::red).rgb());
        pressUndo(e.view());
        QVERIFY(!o->isDeleted(0, 1));
        QVERIFY(!e.win.w.isModified());
        o->layer()->prepareAll();
        QCOMPARE(shot(e).pixel(pic.center()), QColor(Qt::red).rgb());
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
        for (const QString &name : before.partNames()) {
            if (name == QLatin1String("/xl/drawings/drawing1.xml") || name == QLatin1String("/xl/charts/chart1.xml") || name.contains(QLatin1String("/worksheets/")) || name == QLatin1String("/xl/workbook.xml")) continue;
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
            const crunch::DrawingCounts c = o->counts(0);
            QCOMPARE(c.anchors, jp::office::sheet::kMaxDrawingAnchors + 2000);
            QVERIFY(c.truncated);
            QCOMPARE(c.shapes, jp::office::sheet::kMaxDrawingAnchors);
            QVERIFY(!c.why.isEmpty());
            const qint64 renders = o->layer()->stats().renders;
            shot(e);   // (a paint of the window: at most what the layer lets one make, in each of its parts)
            QVERIFY2(o->layer()->stats().renders - renders <= 4 * jp::grid::ObjectLayer::kMaxRendersPerPaint * 2, qPrintable(QString::number(o->layer()->stats().renders - renders)));
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
            shot(e);   // (the picture is not decoded: nothing of its size is made)
            QVERIFY(e.win.w.objects()->counts(0).pictures == 1);
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
        }
    }
};

JP_TEST_CLASS(CrunchDrawingTests)
#include "tst_crunch_drawings.moc"
