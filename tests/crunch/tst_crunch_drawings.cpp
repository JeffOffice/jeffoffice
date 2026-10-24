// Tests of the charts, pictures, and shapes over the cells in JeffCrunch's window: they are shown where their anchors put them, a chart is drawn from the cells it reads and again when one
// changes (typed, or worked out), and objects and charts move when rows are inserted or deleted, undone and redone. Every workbook is built here.
#include "../sheet/drawinghelpers.h"
#include "editwindow.h"

#include "core/sheetobjects.h"

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
};

JP_TEST_CLASS(CrunchDrawingTests)
#include "tst_crunch_drawings.moc"
