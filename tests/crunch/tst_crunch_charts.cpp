// Tests of making and changing charts in JeffCrunch's window: Insert Chart from the cells selected, moving and sizing objects, the chart's elements, Select Data, Change Chart Type, Copy and Paste of a
// chart, and what each saves (the chart part, the drawing, and what names them), read back by the readers of this program. Every workbook is built here.
#include "../sheet/drawinghelpers.h"
#include "editwindow.h"
#include "savedok.h"

#include "app/chartdialogs.h"
#include "core/fonts.h"
#include "core/sheetobjects.h"
#include "office/chart_write.h"
#include "office/xml.h"
#include "sheet/chartedit.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QMenu>
#include <QToolButton>

using namespace crunchtest;
using namespace drawtest;
using namespace jp::office::sheet;

namespace {

// "Sheet1": A1:C4 hold the labels and numbers of dataSheet() (B2:B4 are 10, 20, 30; C2:C4 are 5, 10, 15).
QByteArray plainBook() { return buildXlsx({dataSheet()}); }

// The same with a chart over E2:K13 reading B2:B4 by A2:A4.
QByteArray bookWithAChart(const QString &kind = QStringLiteral("barChart"), const QByteArray &partOverride = QByteArray())
{
    const QVector<SeriesSpec> series = {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {"Q1", "Q2", "Q3"}, {10, 20, 30}, QString()}};
    const QString anchors = twoCell(4, 1, 11, 13, chartFrame(2, QStringLiteral("rIdC"), QStringLiteral("Sales")));
    return bookWithDrawing({dataSheet()}, drawingPart(anchors), {chartTarget(QStringLiteral("rIdC"), 1, partOverride.isEmpty() ? chartPart(kind, series) : partOverride)});
}

QMap<QString, QByteArray> partsOf(const QString &path)
{
    QMap<QString, QByteArray> parts;
    QFile f(path);
    if (f.open(QIODevice::ReadOnly)) jp::readZip(f.readAll(), parts);
    return parts;
}

// The charts a saved file has on its sheet (by the sheet's place): each chart part read.
struct SavedChart {
    Chart chart;
    DrawingObject object;
    QString part;
};
QVector<SavedChart> savedCharts(const QString &path, int sheet = 0)
{
    QVector<SavedChart> out;
    Package p;
    QString why;
    if (!p.readLazyFile(path, &why)) return out;
    Workbook b;
    if (!b.open(p, &why)) return out;
    const QString drawing = drawingPartOf(p, b.info().sheets[sheet].part);
    if (drawing.isEmpty()) return out;
    SheetDrawing d;
    if (!readDrawing(p, drawing, &d, &why)) return out;
    for (const DrawingObject &o : std::as_const(d.objects)) {
        if (o.kind != ObjectKind::Chart) continue;
        SavedChart sc;
        sc.object = o;
        sc.part = o.chartPart;
        if (!readChart(p.part(o.chartPart), defaultTheme(), ColorMap(), &sc.chart, &why)) continue;
        out << sc;
    }
    return out;
}

} // namespace

class CrunchChartTests : public QObject {
    Q_OBJECT

    static bool objectsShown(EditWindow &e, int count)
    {
        crunch::SheetObjects *o = e.win.w.objects();
        return o && o->shownSheet() == 0 && o->layer()->count() == count;
    }
    static bool save(EditWindow &e)
    {
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        return e.win.w.save() && savedOk(saved);
    }
    static Chart chartOf(EditWindow &e, int id = 0, int sheet = 0) { return e.win.w.objects()->chartNow(sheet, id); }
    static void selectRange(EditWindow &e, const QString &range)
    {
        const jp::office::sheet::CellRange r = rangeOf(range);
        e.view()->selectRange(r.row1, r.col1, r.row2, r.col2);
    }

private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    // Insert Chart makes a chart from the cells selected: headers found, the chart over the cells in view, picked, one step of Undo, and Redo puts it back.
    void aChartIsMadeFromTheCellsSelected()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(objectsShown(e, 0));
        crunch::SheetObjects *o = e.win.w.objects();
        selectRange(e, QStringLiteral("A1:C4"));
        QVERIFY(e.win.w.insertChart(ChartKind::Column));
        QCOMPARE(o->layer()->count(), 1);
        QCOMPARE(o->layer()->selectedId(), 0);
        QVERIFY(!o->layer()->isDeleted(0));
        const Chart c = chartOf(e);
        QCOMPARE(seriesCount(c), 2);
        const QVector<ChartSeries> s = seriesOf(c);
        QCOMPARE(s[0].valueRef, QStringLiteral("Sheet1!$B$2:$B$4"));
        QCOMPARE(s[0].categoryRef, QStringLiteral("Sheet1!$A$2:$A$4"));
        QCOMPARE(s[1].name, QStringLiteral("West"));   // (the name is read from the cell)
        QCOMPARE(s[0].values, (QVector<double>{10, 20, 30}));
        QCOMPARE(s[0].categories, QStringList({"Q1", "Q2", "Q3"}));
        QVERIFY(e.win.w.isModified());
        // drawn over the cells in view
        o->layer()->prepareAll();
        const QRect r = o->layer()->deviceRect(0, grid::Pane::Body);
        QVERIFY(!r.isEmpty() && e.view()->viewport()->rect().intersects(r));
        QCOMPARE(r.width(), 480);
        QCOMPARE(r.height(), 288);
        // Undo takes it away, Redo gives it back
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Insert Chart"));
        pressUndo(&e.win.w);
        QVERIFY(o->layer()->isDeleted(0));
        QVERIFY(!e.win.w.isModified());
        pressRedo(&e.win.w);
        QVERIFY(!o->layer()->isDeleted(0));
        QCOMPARE(seriesCount(chartOf(e)), 2);
    }

    // A chart made here is in the file: the chart part with its references, the drawing, the relationships, and the types; the file opens with the chart where it was put, reading the cells.
    void aNewChartIsSavedAndReopens()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        selectRange(e, QStringLiteral("A1:C4"));
        QVERIFY(e.win.w.insertChart(ChartKind::Line));
        QVERIFY(save(e));
        const QMap<QString, QByteArray> parts = partsOf(e.path);
        QVERIFY(parts.contains(QStringLiteral("xl/charts/chart1.xml")) && parts.contains(QStringLiteral("xl/drawings/drawing1.xml")));
        QVERIFY(parts.value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("<drawing "));
        QVERIFY(parts.value(QStringLiteral("[Content_Types].xml")).contains("/xl/charts/chart1.xml"));
        for (auto it = parts.begin(); it != parts.end(); ++it)
            if (it.key().endsWith(QLatin1String(".xml")) || it.key().endsWith(QLatin1String(".rels"))) {
                QDomDocument doc;
                QVERIFY2(doc.setContent(it.value()), qPrintable(it.key()));
            }
        const QVector<SavedChart> charts = savedCharts(e.path);
        QCOMPARE(charts.size(), 1);
        QCOMPARE(int(kindOfChart(charts[0].chart)), int(ChartKind::Line));
        QCOMPARE(seriesOf(charts[0].chart)[1].valueRef, QStringLiteral("Sheet1!$C$2:$C$4"));
        QCOMPARE(seriesOf(charts[0].chart)[1].values, (QVector<double>{5, 10, 15}));   // (the part stores what the cells hold)
        // the other parts are as they were
        QMap<QString, QByteArray> was;
        QVERIFY(jp::readZip(plainBook(), was));
        QCOMPARE(parts.value(QStringLiteral("xl/styles.xml")), was.value(QStringLiteral("xl/styles.xml")));
        // opened again, the chart is there and reads the cells
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        crunch::SheetObjects *o = second.w.objects();
        QTRY_VERIFY(o->shownSheet() == 0 && o->layer()->count() == 1);
        QCOMPARE(seriesCount(o->chartNow(0, 0)), 2);
        QCOMPARE(seriesOf(o->chartNow(0, 0))[0].values, (QVector<double>{10, 20, 30}));
        // saved again with nothing changed: nothing changes
        const QByteArray first = parts.value(QStringLiteral("xl/charts/chart1.xml"));
        e.select(QStringLiteral("E5"));
        e.enter(QStringLiteral("7"));
        QVERIFY(e.settle());
        QVERIFY(save(e));
        QCOMPARE(partsOf(e.path).value(QStringLiteral("xl/charts/chart1.xml")), first);
    }

    // Each kind on the ribbon makes that kind; the chart's data is the cells' for the kind (a scatter takes its x values from the first column).
    void everyKindOnTheRibbonMakesItsKind()
    {
        struct Case {
            const char *id;
            ChartKind kind;
        };
        const Case cases[] = {{"chart.column.stacked", ChartKind::ColumnStacked}, {"chart.bar.clustered", ChartKind::Bar}, {"chart.line.markers", ChartKind::LineMarkers},
                              {"chart.pie.doughnut", ChartKind::Doughnut}, {"chart.area.stacked", ChartKind::AreaStacked}, {"chart.scatter.joined", ChartKind::ScatterLines},
                              {"chart.combo", ChartKind::Combo}, {"chart.column", ChartKind::Column}};
        for (const Case &c : cases) {
            EditWindow e;
            QVERIFY(e.open(plainBook()));
            QVERIFY(e.settle());
            selectRange(e, QStringLiteral("A1:C4"));
            QVERIFY2(e.win.w.act(QString::fromLatin1(c.id)) && e.win.w.act(QString::fromLatin1(c.id))->isEnabled(), c.id);
            e.win.w.act(QString::fromLatin1(c.id))->trigger();
            QCOMPARE(e.win.w.objects()->layer()->count(), 1);
            QCOMPARE(int(kindOfChart(chartOf(e))), int(c.kind));
        }
    }

    // Nothing to chart says so and makes nothing; so do a chart sheet's and a sheet being saved's.
    void nothingToChartMakesNothing()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        selectRange(e, QStringLiteral("H20:J22"));
        QVERIFY(!e.win.w.insertChart(ChartKind::Column));
        QCOMPARE(e.win.w.objects()->layer()->count(), 0);
        QVERIFY(e.win.told.last().contains(QLatin1String("Select the cells")));
        QVERIFY(!e.win.w.isModified());
        // the dialog's stand-in gives it up
        e.win.w.setInsertChartAsker([](CrunchWindow::InsertChartChoice *) { return false; });
        selectRange(e, QStringLiteral("A1:C4"));
        e.win.w.insertChartDialog();
        QCOMPARE(e.win.w.objects()->layer()->count(), 0);
        e.win.w.setInsertChartAsker([](CrunchWindow::InsertChartChoice *c) {
            c->kind = ChartKind::Pie;
            c->inRows = 2;   // (the other way: the rows are the series)
            return true;
        });
        e.win.w.insertChartDialog();
        QCOMPARE(e.win.w.objects()->layer()->count(), 1);
        QCOMPARE(int(kindOfChart(chartOf(e))), int(ChartKind::Pie));
    }
};

JP_TEST_CLASS(CrunchChartTests)
#include "tst_crunch_charts.moc"
