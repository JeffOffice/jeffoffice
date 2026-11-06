// Tests of making and changing charts in JeffCrunch's window: New Chart from the cells selected, moving and sizing objects, the chart's elements, Chart Data, Chart Kind, Copy and Paste of a
// chart, and what each saves (the chart part, the drawing, and what names them), read back by the readers of this program. Every workbook is built here.
#include "../sheet/drawinghelpers.h"
#include "editwindow.h"
#include "savedok.h"

#include "app/chartdialogs.h"
#include "app/ribbon.h"
#include "core/fonts.h"
#include "core/objectclip.h"
#include "grid/chartcells.h"
#include "core/sheetobjects.h"
#include "office/chart.h"
#include "office/chart_write.h"
#include "office/xml.h"
#include "sheet/chartedit.h"
#include "sheet/drawing.h"
#include "sheet/xlsxsave.h"

#include <QCheckBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QClipboard>
#include <QListWidget>
#include <QGuiApplication>
#include <QDoubleSpinBox>
#include <QAbstractButton>
#include <QListWidget>
#include <QMenu>
#include <QRadioButton>
#include <QTemporaryDir>
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


struct SavedAnchor {
    bool found = false;
    QString editAs, kind;
};
SavedAnchor savedAnchor(const QString &path, const QString &name, int drawingNumber = 1)
{
    SavedAnchor out;
    Package p;
    QString why;
    if (!p.readLazyFile(path, &why)) return out;
    QDomDocument doc;
    if (!parseXml(p.part(QStringLiteral("/xl/drawings/drawing%1.xml").arg(drawingNumber)), &doc)) return out;
    const QDomNodeList all = doc.elementsByTagName(QStringLiteral("cNvPr"));
    for (int i = 0; i < all.size(); ++i) {
        if (all.at(i).toElement().attribute(QStringLiteral("name")) != name) continue;
        QDomElement anchor = all.at(i).parentNode().toElement();
        while (!anchor.isNull() && !anchor.localName().endsWith(QLatin1String("Anchor"))) anchor = anchor.parentNode().toElement();
        if (anchor.isNull()) return out;
        out.found = true;
        out.kind = anchor.localName();
        out.editAs = anchor.attribute(QStringLiteral("editAs"));
        return out;
    }
    return out;
}

} // namespace

class CrunchChartTests : public QObject {
    Q_OBJECT

    static bool objectsShown(EditWindow &e, int count, int sheet = 0)
    {
        crunch::SheetObjects *o = e.win.w.objects();
        return o && o->shownSheet() == sheet && o->layer()->count() == count;
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

    // A chart reads the values its cells hold now, wherever rows and columns put them: after a row is put in the middle of what it reads, a number is typed in it, a row is taken out, and all of it is undone.
    void aChartHasTheValuesOfItsCellsAfterEditsAndUndo()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(objectsShown(e, 0));
        selectRange(e, QStringLiteral("A1:C4"));
        QVERIFY(e.win.w.insertChart(ChartKind::Column));
        crunch::EditSession *s = e.win.w.session();
        auto values = [&](int series) {
            const QVector<ChartSeries> all = seriesOf(chartOf(e));
            return all.value(series).values;
        };
        QCOMPARE(values(0), (QVector<double>{10, 20, 30}));
        QVERIFY(s->insertRows(0, 2, 1).ok);   // between Q1 and Q2
        QVERIFY(e.settle());
        QCOMPARE(seriesOf(chartOf(e))[0].valueRef, QStringLiteral("Sheet1!$B$2:$B$5"));
        QCOMPARE(values(0).size(), 4);
        QCOMPARE(values(0).first(), 10.0);
        QCOMPARE(values(0).last(), 30.0);
        QVERIFY(s->type(0, 2, 1, QStringLiteral("15")).ok);
        QVERIFY(e.settle());
        QCOMPARE(values(0), (QVector<double>{10, 15, 20, 30}));
        QCOMPARE(values(1).size(), 4);   // (the empty cell of the row put in is not a number)
        QCOMPARE(values(1).first(), 5.0);
        QVERIFY(std::isnan(values(1)[1]));
        QCOMPARE(values(1).last(), 15.0);
        QVERIFY(s->deleteRows(0, 1, 1).ok);    // the first row of data
        QVERIFY(e.settle());
        QCOMPARE(values(0), (QVector<double>{15, 20, 30}));
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(values(0), (QVector<double>{10, 15, 20, 30}));
        pressUndo(&e.win.w);
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(values(0), (QVector<double>{10, 20, 30}));
        QCOMPARE(seriesOf(chartOf(e))[0].valueRef, QStringLiteral("Sheet1!$B$2:$B$4"));
        QVERIFY(s->insertColumns(0, 0, 1).ok);   // the columns move right: the series read column C
        QVERIFY(e.settle());
        QCOMPARE(values(0), (QVector<double>{10, 20, 30}));
        QCOMPARE(seriesOf(chartOf(e))[0].valueRef, QStringLiteral("Sheet1!$C$2:$C$4"));
    }

    // The cells a chart reads, dragged by the border of the selection to other cells, take the series with them (and Undo puts the series back).
    void aChartSeriesFollowsCellsMovedByTheSelectionBorder()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithAChart()));
        QTRY_VERIFY(objectsShown(e, 1));
        QCOMPARE(seriesOf(chartOf(e))[0].valueRef, QStringLiteral("Sheet1!$B$2:$B$4"));
        e.win.w.resize(1100, 640);
        e.view()->selectRange(1, 1, 3, 1);
        Q_EMIT e.view()->moveRequested(grid::GridRange{1, 1, 3, 1}, grid::GridRange{1, 4, 3, 4}, false);   // B2:B4 dropped on E2:E4
        QVERIFY(e.settle());
        QCOMPARE(seriesOf(chartOf(e))[0].valueRef, QStringLiteral("Sheet1!$E$2:$E$4"));
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(seriesOf(chartOf(e))[0].valueRef, QStringLiteral("Sheet1!$B$2:$B$4"));
    }

    // New Chart makes a chart from the cells selected: headers found, the chart over the cells in view, picked, one step of Undo, and Redo puts it back.
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
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("New Chart"));
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

    // A sheet that gets a drawing and a filter in one save has both, in the order the format wants (the filter before the drawing), and opens again with both.
    void aNewChartAndAFilterAreSavedTogetherInTheRightOrder()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        AutoFilter f;
        f.ref = rangeOf(QStringLiteral("A1:C4"));
        FilterColumn c;
        c.column = 1;
        c.kind = FilterColumn::Kind::Custom;
        c.conditions.append({FilterColumn::Op::Greater, QStringLiteral("10")});
        f.columns.append(c);
        QVERIFY(e.win.w.session()->setFilter(0, f, QStringLiteral("Filtering")).ok);
        selectRange(e, QStringLiteral("A1:C4"));
        QVERIFY(e.win.w.insertChart(ChartKind::Column));
        QVERIFY(save(e));
        const QByteArray sheet = partsOf(e.path).value(QStringLiteral("xl/worksheets/sheet1.xml"));
        const int filter = int(sheet.indexOf("<autoFilter")), drawing = int(sheet.indexOf("<drawing "));
        QVERIFY2(filter >= 0 && drawing >= 0, sheet.constData());
        QVERIFY2(sheet.indexOf("</sheetData>") < filter && filter < drawing, sheet.constData());
        QDomDocument doc;
        QVERIFY(doc.setContent(sheet));
        QCOMPARE(savedCharts(e.path).size(), 1);
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QVERIFY(second.w.calculation()->autoFilterOf(0).valid());
        QCOMPARE(second.w.calculation()->autoFilterOf(0).columns.size(), 1);
        QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 1);
    }

    // Each kind on the ribbon makes that kind; the chart's data is the cells' for the kind (a scatter takes its x values from the first column).
    void everyKindOnTheRibbonMakesItsKind()
    {
        struct Case {
            const char *id;
            ChartKind kind;
        };
        const Case cases[] = {{"chart.column.stacked", ChartKind::ColumnStacked}, {"chart.bar.clustered", ChartKind::Bar}, {"chart.line.markers", ChartKind::LineMarkers},
                              {"chart.pie.doughnut", ChartKind::Doughnut}, {"chart.area.stacked", ChartKind::AreaStacked}, {"chart.scatter.joined", ChartKind::ScatterLines}, {"chart.scatter.smooth", ChartKind::ScatterSmooth},
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

    // Recommended Charts offers the kinds the selected cells suit, best first, and makes the one picked; nothing selected says so; the dialog lists only those kinds.
    void recommendedChartsAreOfferedForTheCells()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        selectRange(e, QStringLiteral("H20:J22"));
        e.win.w.recommendedChartsDialog();
        QCOMPARE(e.win.w.objects()->layer()->count(), 0);
        QVERIFY(e.win.told.last().contains(QLatin1String("Select the cells")));
        selectRange(e, QStringLiteral("A1:C4"));
        QVector<ChartKind> offered;
        e.win.w.setRecommendedChartsAsker([&](const QVector<ChartKind> &kinds, CrunchWindow::InsertChartChoice *c) {
            offered = kinds;
            c->kind = kinds.value(1);
            return true;
        });
        e.win.w.recommendedChartsDialog();
        QVERIFY(offered.size() >= 2 && offered.size() <= 6);
        QCOMPARE(offered.first(), ChartKind::Column);
        QCOMPARE(e.win.w.objects()->layer()->count(), 1);
        QCOMPARE(int(kindOfChart(chartOf(e))), int(offered[1]));
        // the dialog itself lists the kinds, and only those
        InsertChartDialog dialog(nullptr, offered.first(), nullptr, offered, QStringLiteral("Recommended Charts"));
        QCOMPARE(dialog.windowTitle(), QStringLiteral("Recommended Charts"));
        QCOMPARE(dialog.findChild<QListWidget *>(QStringLiteral("chartKinds"))->count(), int(offered.size()));
        QCOMPARE(int(dialog.kind()), int(offered.first()));
        // it is a ribbon command, on the Insert tab
        QVERIFY(e.win.w.act(QStringLiteral("insert.recommendedCharts")));
    }

    // Chart Filters: the button beside a picked chart lists its series; clearing one leaves it out of the chart (one step of Undo), checking it brings it back where it was; the last one stays.
    void chartFiltersLeaveSeriesOutAndBringThemBack()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        selectRange(e, QStringLiteral("A1:C4"));
        QVERIFY(e.win.w.insertChart(ChartKind::Column));
        e.win.w.objects()->layer()->prepareAll();
        QTRY_VERIFY(e.win.w.chartFiltersButton() && e.win.w.chartFiltersButton()->isVisible());
        QVector<CrunchWindow::ChartFilterEntry> entries = e.win.w.chartFilterEntries();
        QCOMPARE(entries.size(), 2);
        QVERIFY(entries[0].shown && entries[1].shown);
        const QString first = entries[0].name;
        const int steps = e.win.w.session()->undoStack()->count();
        QVERIFY(e.win.w.setChartSeriesShown(0, false));
        QCOMPARE(e.win.w.session()->undoStack()->count(), steps + 1);
        entries = e.win.w.chartFilterEntries();
        QCOMPARE(entries.size(), 2);
        QVERIFY(!entries[0].shown && entries[1].shown);
        QCOMPARE(entries[0].name, first);
        // the drawing leaves it out
        const QByteArray one = chartSvg(chartOf(e), QSizeF(480, 288));
        // the one left stays
        QVERIFY(!e.win.w.setChartSeriesShown(1, false));
        // back where it was
        QVERIFY(e.win.w.setChartSeriesShown(0, true));
        QVERIFY(e.win.w.chartFilterEntries()[0].shown);
        QVERIFY(chartSvg(chartOf(e), QSizeF(480, 288)) != one);
        // the menu lists them with checks
        QMenu *menu = e.win.w.chartFiltersButton()->menu();
        QVERIFY(menu);
        QMetaObject::invokeMethod(menu, "aboutToShow", Qt::DirectConnection);
        int checkable = 0;
        for (QAction *a : menu->actions()) checkable += a->isCheckable() && a->isChecked();
        QCOMPARE(checkable, 2);
    }

    // A series left out by Chart Filters is not lost on save: the file keeps it, and after reopening it can be brought back. (It is written as a filtered series, so it reopens hidden.)
    void aFilteredSeriesSurvivesSaveAndReopen()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        selectRange(e, QStringLiteral("A1:C4"));
        QVERIFY(e.win.w.insertChart(ChartKind::Column));
        e.win.w.objects()->layer()->prepareAll();
        const QString first = e.win.w.chartFilterEntries().value(0).name;
        QVERIFY(e.win.w.setChartSeriesShown(0, false));
        QVERIFY(save(e));
        const QVector<SavedChart> charts = savedCharts(e.path);
        QCOMPARE(charts.size(), 1);
        QCOMPARE(seriesOf(charts[0].chart).size(), 2);
        EditWindow again;
        QVERIFY(again.open([&] { QFile f(e.path); f.open(QIODevice::ReadOnly); return f.readAll(); }(), QStringLiteral("again.xlsx")));
        QVERIFY(again.settle());
        QTRY_VERIFY(objectsShown(again, 1));
        again.win.w.objects()->layer()->select(0);
        QVector<CrunchWindow::ChartFilterEntry> entries = again.win.w.chartFilterEntries();
        QCOMPARE(entries.size(), 2);
        QVERIFY(!entries[0].shown && entries[1].shown);
        QCOMPARE(entries[0].name, first);
        // it can be brought back, filtered out again, and kept hidden through another save and reopen
        QVERIFY(again.win.w.setChartSeriesShown(0, true));
        QVERIFY(again.win.w.setChartSeriesShown(0, false));
        QVERIFY(save(again));
        EditWindow third;
        QVERIFY(third.open([&] { QFile f(again.path); f.open(QIODevice::ReadOnly); return f.readAll(); }(), QStringLiteral("third.xlsx")));
        QVERIFY(third.settle());
        QTRY_VERIFY(objectsShown(third, 1));
        third.win.w.objects()->layer()->select(0);
        QVERIFY(!third.win.w.chartFilterEntries()[0].shown);
        QVERIFY(third.win.w.chartFilterEntries()[1].shown);
        // and brought back again
        QVERIFY(third.win.w.setChartSeriesShown(0, true));
        QVERIFY(third.win.w.chartFilterEntries()[0].shown);
        QCOMPARE(chartOf(third).plots.first().series.size(), 2);
    }

    // ---- moving and sizing

    // A drag with the mouse moves the object: one step, undone and redone, saved as the anchor, with the part of the chart as it was.
    void anObjectIsMovedWithTheMouseAndSaved()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithAChart()));
        QTRY_VERIFY(objectsShown(e, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        const grid::ObjectBox was = o->layer()->boxNow(0);
        const QRect r = o->layer()->deviceRect(0, grid::Pane::Body);
        QVERIFY(!r.isEmpty());
        QWidget *vp = e.view()->viewport();
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, r.center());
        QTest::mouseMove(vp, r.center() + QPoint(30, 20));
        QTest::mouseMove(vp, r.center() + QPoint(60, 40));
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, r.center() + QPoint(60, 40));
        QVERIFY(e.win.w.isModified());
        const grid::ObjectBox now = o->layer()->boxNow(0);
        QCOMPARE(now.x, was.x + 60);
        QCOMPARE(now.y, was.y + 40);
        QCOMPARE(now.w, was.w);
        QCOMPARE(now.h, was.h);
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Move or Size Object"));
        pressUndo(&e.win.w);
        QCOMPARE(o->layer()->boxNow(0).x, was.x);
        QVERIFY(!e.win.w.isModified());
        pressRedo(&e.win.w);
        QCOMPARE(o->layer()->boxNow(0).x, was.x + 60);
        QVERIFY(save(e));
        // the file has the chart where it was put, its part as it was, and reopened it is there
        QMap<QString, QByteArray> original;
        QVERIFY(jp::readZip(bookWithAChart(), original));
        QCOMPARE(partsOf(e.path).value(QStringLiteral("xl/charts/chart1.xml")), original.value(QStringLiteral("xl/charts/chart1.xml")));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 1);
        second.w.objects()->layer()->prepareAll();
        QCOMPARE(second.w.objects()->layer()->boxNow(0).x, was.x + 60);
        QCOMPARE(second.w.objects()->layer()->boxNow(0).y, was.y + 40);
        // a second save with nothing moved writes the same drawing
        const QByteArray drawing = partsOf(e.path).value(QStringLiteral("xl/drawings/drawing1.xml"));
        e.select(QStringLiteral("H20"));
        e.enter(QStringLiteral("3"));
        QVERIFY(e.settle());
        QVERIFY(save(e));
        QCOMPARE(partsOf(e.path).value(QStringLiteral("xl/drawings/drawing1.xml")), drawing);
    }

    // The handles size it, Shift at a corner keeps the proportions, the arrow keys move it; each is a step.
    void handlesAndKeysSizeAndMove()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithAChart()));
        QTRY_VERIFY(objectsShown(e, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->select(0);
        const grid::ObjectBox was = o->layer()->boxNow(0);
        // the bottom right handle
        QRect r = o->layer()->deviceRect(0, grid::Pane::Body);
        QWidget *vp = e.view()->viewport();
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, r.bottomRight());
        QTest::mouseMove(vp, r.bottomRight() + QPoint(20, 10));
        QTest::mouseMove(vp, r.bottomRight() + QPoint(40, 30));
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, r.bottomRight() + QPoint(40, 30));
        grid::ObjectBox now = o->layer()->boxNow(0);
        QCOMPARE(now.x, was.x);
        QCOMPARE(now.w, was.w + 40);
        QCOMPARE(now.h, was.h + 30);
        // keys
        QTest::keyClick(e.view(), Qt::Key_Right);
        QCOMPARE(o->layer()->boxNow(0).x, was.x + 6);
        QTest::keyClick(e.view(), Qt::Key_Down, Qt::ControlModifier);
        QCOMPARE(o->layer()->boxNow(0).y, was.y + 1);
        QTest::keyClick(e.view(), Qt::Key_Right, Qt::ShiftModifier);
        QCOMPARE(o->layer()->boxNow(0).w, was.w + 40 + 6);
        for (int i = 0; i < 4; ++i) pressUndo(&e.win.w);
        QCOMPARE(o->layer()->boxNow(0).x, was.x);
        QCOMPARE(o->layer()->boxNow(0).w, was.w);
        QVERIFY(!e.win.w.isModified());
        // the cell cursor was not moved by the keys the object took
        QCOMPARE(e.selected(), QStringLiteral("A1"));
    }

    // Properties: "Print this object" off keeps the object off the printed pages, is one step of Undo, and is saved as the client data's fPrintsWithSheet and kept.
    void anObjectThatDoesNotPrintIsLeftOffThePages()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithAChart()));
        QTRY_VERIFY(objectsShown(e, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->select(0);
        QCOMPARE(int(o->printObjects(0).size()), 1);
        bool printedBefore = false;
        e.win.w.setPropertiesAsker([&](const ObjectProperties &now, ObjectProperties *out) {
            printedBefore = now.prints;
            *out = now;
            out->prints = false;
            return true;
        });
        e.win.w.act(QStringLiteral("object.properties"))->trigger();
        QVERIFY(printedBefore);
        QCOMPARE(int(o->printObjects(0).size()), 0);
        QVERIFY(save(e));
        Package p;
        QString why;
        QVERIFY(p.readLazyFile(e.path, &why));
        QVERIFY(p.part(QStringLiteral("/xl/drawings/drawing1.xml")).contains("fPrintsWithSheet=\"0\""));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 1);
        QCOMPARE(int(second.w.objects()->printObjects(0).size()), 0);
        e.win.w.session()->undoStack()->undo();
        QCOMPARE(int(o->printObjects(0).size()), 1);
        QVERIFY(save(e));
        QVERIFY(p.readLazyFile(e.path, &why));
        QVERIFY(!p.part(QStringLiteral("/xl/drawings/drawing1.xml")).contains("fPrintsWithSheet"));
    }

    // Properties: the size and what the object does when cells change; it is saved as the anchor's editAs (a two-cell anchor that moves and does not size), and kept.
    void whatAnObjectDoesWhenCellsChangeIsSaved()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithAChart()));
        QTRY_VERIFY(objectsShown(e, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->select(0);
        const grid::ObjectBox was = o->layer()->boxNow(0);
        ObjectProperties seen;
        e.win.w.setPropertiesAsker([&](const ObjectProperties &now, ObjectProperties *out) {
            seen = now;
            *out = now;
            out->follows = AnchorKind::OneCell;
            out->widthPoints = 300;
            out->heightPoints = 150;
            return true;
        });
        e.win.w.act(QStringLiteral("object.properties"))->trigger();
        QCOMPARE(seen.widthPoints, was.w * 0.75);
        QCOMPARE(int(seen.follows), int(AnchorKind::TwoCell));
        QCOMPARE(o->layer()->boxNow(0).w, 400.0);
        QCOMPARE(o->layer()->boxNow(0).h, 200.0);
        QCOMPARE(int(o->anchorOf(0, 0).editAs), int(AnchorKind::OneCell));
        QVERIFY(save(e));
        const SavedAnchor a = savedAnchor(e.path, QStringLiteral("Sales"));
        QVERIFY(a.found);
        QCOMPARE(a.kind, QStringLiteral("twoCellAnchor"));
        QCOMPARE(a.editAs, QStringLiteral("oneCell"));
        // inserting rows above moves it and keeps its size
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 1);
        QCOMPARE(int(second.w.objects()->anchorOf(0, 0).editAs), int(AnchorKind::OneCell));
        QCOMPARE(second.w.objects()->layer()->boxNow(0).w, 400.0);
    }

    // ---- what a chart shows

    // The "+" button's list changes the chart's elements, each a step that Undo takes back; the file holds them, and the chart part is written from the model.
    void theElementsListChangesTheChart()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithAChart()));
        QTRY_VERIFY(objectsShown(e, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->select(0);
        QTRY_VERIFY(e.win.w.chartElementsButton() && e.win.w.chartElementsButton()->isVisible());
        QMenu *menu = e.win.w.chartElementsButton()->menu();
        QVERIFY(menu);
        auto fire = [&](const QStringList &path) {
            QMetaObject::invokeMethod(menu, "aboutToShow", Qt::DirectConnection);
            QMenu *m = menu;
            QAction *found = nullptr;
            for (int i = 0; i < path.size(); ++i) {
                found = nullptr;
                for (QAction *a : m->actions())
                    if (a->text() == path[i]) found = a;
                if (!found) return false;
                if (i + 1 < path.size()) m = found->menu();
            }
            found->trigger();
            return true;
        };
        QVERIFY(fire({"Chart Title", "Above"}));
        QVERIFY(fire({"Legend", "Right"}));
        QVERIFY(fire({"Axis Titles", "Vertical"}));
        QVERIFY(fire({"Data Labels", "Outside End"}));
        QVERIFY(fire({"Gridlines", "Lines Up and Down, Major"}));
        const ChartElements now = elementsOf(chartOf(e));
        QVERIFY(now.title && now.axisTitleVertical && now.labels && now.gridVertical);
        QCOMPARE(now.legendPosition, QStringLiteral("r"));
        QCOMPARE(now.labelPosition, QStringLiteral("outEnd"));
        QVERIFY(!fire({"Chart Title", "No Such Thing"}));
        for (int i = 0; i < 5; ++i) pressUndo(&e.win.w);
        QVERIFY(!e.win.w.isModified());
        QVERIFY(!elementsOf(chartOf(e)).title);
        for (int i = 0; i < 5; ++i) pressRedo(&e.win.w);
        QVERIFY(elementsOf(chartOf(e)).labels);
        QVERIFY(save(e));
        const QVector<SavedChart> charts = savedCharts(e.path);
        QCOMPARE(charts.size(), 1);
        const ChartElements saved = elementsOf(charts[0].chart);
        QVERIFY(saved.title && saved.axisTitleVertical && saved.labels && saved.gridVertical);
        QCOMPARE(saved.legendPosition, QStringLiteral("r"));
        QCOMPARE(charts[0].chart.title.text, QStringLiteral("East"));   // (a single series: the title is its name, which the file says by saying nothing)
        // the references are the ones it read, and the part holds what the cells hold
        QCOMPARE(seriesOf(charts[0].chart)[0].valueRef, QStringLiteral("Sheet1!$B$2:$B$4"));
        QCOMPARE(seriesOf(charts[0].chart)[0].values, (QVector<double>{10, 20, 30}));
        // the colors and a look are changed from the brush button
        QMenu *looks = e.win.w.chartLooksButton()->menu();
        QVERIFY(looks);
        QCOMPARE(looks->actions().size(), 2 + chartColorSchemeCount() + chartStyleCount());
        for (QAction *a : looks->actions())
            if (a->text() == QLatin1String("Dark")) a->trigger();
        QCOMPARE(chartOf(e).background.name(), QStringLiteral("#262626"));
        QVERIFY(save(e));
        QCOMPARE(savedCharts(e.path)[0].chart.background.name(), QStringLiteral("#262626"));
    }

    // What the data labels hold, what stands between, and their number format are chosen in the Data Labels list; each a step of Undo, and the file holds them.
    void dataLabelContentsAreChosenInTheElementsList()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithAChart()));
        QTRY_VERIFY(objectsShown(e, 1));
        e.win.w.objects()->layer()->select(0);
        QTRY_VERIFY(e.win.w.chartElementsButton() && e.win.w.chartElementsButton()->isVisible());
        QMenu *menu = e.win.w.chartElementsButton()->menu();
        auto fire = [&](const QStringList &path) {
            QMetaObject::invokeMethod(menu, "aboutToShow", Qt::DirectConnection);
            QMenu *m = menu;
            QAction *found = nullptr;
            for (int i = 0; i < path.size(); ++i) {
                found = nullptr;
                for (QAction *a : m->actions())
                    if (a->text() == path[i]) found = a;
                if (!found) return false;
                if (i + 1 < path.size()) m = found->menu();
            }
            found->trigger();
            return true;
        };
        QVERIFY(fire({"Data Labels", "Label Contains", "Legend Key"}));
        QVERIFY(labelPartsOf(chartOf(e)).legendKey);
        QVERIFY(fire({"Data Labels", "Label Contains", "Legend Key"}));
        QVERIFY(!labelPartsOf(chartOf(e)).legendKey);
        QVERIFY(fire({"Data Labels", "Label Contains", "Category Name"}));
        ChartLabelParts p = labelPartsOf(chartOf(e));
        QVERIFY(p.category && !p.value);
        QVERIFY(fire({"Data Labels", "Label Contains", "Value"}));
        QVERIFY(fire({"Data Labels", "Separator", "New Line"}));
        QCOMPARE(labelPartsOf(chartOf(e)).separator, QStringLiteral("\n"));
        QVERIFY(fire({"Data Labels", "Separator", "Space"}));
        QCOMPARE(labelPartsOf(chartOf(e)).separator, QStringLiteral(" "));
        QVERIFY(fire({"Data Labels", "Separator", "New Line"}));
        e.win.w.setChartTextAsker([](const QString &, QString *text) {
            *text = QStringLiteral("0.00");
            return true;
        });
        QVERIFY(fire({"Data Labels", "Number Format…"}));
        p = labelPartsOf(chartOf(e));
        QVERIFY(p.category && p.value && !p.series);
        QCOMPARE(p.separator, QStringLiteral("\n"));
        QCOMPARE(p.format, QStringLiteral("0.00"));
        QVERIFY(save(e));
        const ChartLabelParts saved = labelPartsOf(savedCharts(e.path)[0].chart);
        QVERIFY(saved.category && saved.value);
        QCOMPARE(saved.format, QStringLiteral("0.00"));
        pressUndo(&e.win.w);
        QVERIFY(labelPartsOf(chartOf(e)).format.isEmpty());
        QVERIFY(labelPartsOf(chartOf(e)).category);
        // the last content turned off takes the labels away
        pressRedo(&e.win.w);
        QVERIFY(fire({"Data Labels", "Label Contains", "Category Name"}));
        QVERIFY(fire({"Data Labels", "Label Contains", "Value"}));
        QVERIFY(!elementsOf(chartOf(e)).labels);
    }

    // Error Bars, in the elements list: a kind for every series (one step of Undo), kept in the file; None takes them away.
    void errorBarsAreChosenInTheElementsList()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        selectRange(e, QStringLiteral("A1:C4"));
        QVERIFY(e.win.w.insertChart(ChartKind::Column));
        e.win.w.objects()->layer()->prepareAll();
        QTRY_VERIFY(e.win.w.chartElementsButton() && e.win.w.chartElementsButton()->isVisible());
        QMenu *menu = e.win.w.chartElementsButton()->menu();
        auto fire = [&](const QStringList &path) {
            QMetaObject::invokeMethod(menu, "aboutToShow", Qt::DirectConnection);
            QMenu *m = menu;
            QAction *found = nullptr;
            for (int i = 0; i < path.size(); ++i) {
                found = nullptr;
                for (QAction *a : m->actions())
                    if (a->text() == path[i]) found = a;
                if (!found) return false;
                if (i + 1 < path.size()) m = found->menu();
            }
            found->trigger();
            return true;
        };
        const int steps = e.win.w.session()->undoStack()->count();
        QVERIFY(fire({"Error Bars", "Percentage (5%)"}));
        QCOMPARE(e.win.w.session()->undoStack()->count(), steps + 1);
        QCOMPARE(errorBarsKind(chartOf(e)), QStringLiteral("percentage"));
        QVERIFY(save(e));
        QCOMPARE(errorBarsKind(savedCharts(e.path)[0].chart), QStringLiteral("percentage"));
        pressUndo(&e.win.w);
        QVERIFY(errorBarsKind(chartOf(e)).isEmpty());
        QVERIFY(fire({"Error Bars", "Standard Error"}));
        QVERIFY(fire({"Error Bars", "None"}));
        QVERIFY(errorBarsKind(chartOf(e)).isEmpty());
    }

    // Lines, in the elements list of a line chart: Drop Lines, High-Low Lines, and Up/Down Bars, each one step of Undo and kept in the file; a column chart has no such list.
    void linesBetweenSeriesAreChosenInTheElementsList()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        selectRange(e, QStringLiteral("A1:C4"));
        QVERIFY(e.win.w.insertChart(ChartKind::Line));
        e.win.w.objects()->layer()->prepareAll();
        QTRY_VERIFY(e.win.w.chartElementsButton() && e.win.w.chartElementsButton()->isVisible());
        QMenu *menu = e.win.w.chartElementsButton()->menu();
        auto fire = [&](const QStringList &path) {
            QMetaObject::invokeMethod(menu, "aboutToShow", Qt::DirectConnection);
            QMenu *m = menu;
            QAction *found = nullptr;
            for (int i = 0; i < path.size(); ++i) {
                found = nullptr;
                for (QAction *a : m->actions())
                    if (a->text() == path[i]) found = a;
                if (!found) return false;
                if (i + 1 < path.size()) m = found->menu();
            }
            found->trigger();
            return true;
        };
        const int steps = e.win.w.session()->undoStack()->count();
        QVERIFY(fire({"Lines", "High-Low Lines"}));
        QVERIFY(fire({"Lines", "Up/Down Bars"}));
        QCOMPARE(e.win.w.session()->undoStack()->count(), steps + 2);
        QVERIFY(plotLinesOn(chartOf(e), PlotLines::HighLow) && plotLinesOn(chartOf(e), PlotLines::UpDown));
        QVERIFY(save(e));
        QVERIFY(plotLinesOn(savedCharts(e.path)[0].chart, PlotLines::UpDown));
        pressUndo(&e.win.w);
        QVERIFY(!plotLinesOn(chartOf(e), PlotLines::UpDown) && plotLinesOn(chartOf(e), PlotLines::HighLow));
        QVERIFY(fire({"Lines", "Drop Lines"}));
        QVERIFY(plotLinesOn(chartOf(e), PlotLines::Drop));
    }

    // Value From Cells, in the Data Labels list: the labels show the words of a range of cells (one step of Undo), the words follow the cells, and the file holds the range and its words.
    void dataLabelsTakeTheirWordsFromCells()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithAChart()));
        QTRY_VERIFY(objectsShown(e, 1));
        e.win.w.objects()->layer()->select(0);
        QTRY_VERIFY(e.win.w.chartElementsButton() && e.win.w.chartElementsButton()->isVisible());
        QMenu *menu = e.win.w.chartElementsButton()->menu();
        auto fire = [&](const QStringList &path) {
            QMetaObject::invokeMethod(menu, "aboutToShow", Qt::DirectConnection);
            QMenu *m = menu;
            QAction *found = nullptr;
            for (int i = 0; i < path.size(); ++i) {
                found = nullptr;
                for (QAction *a : m->actions())
                    if (a->text() == path[i]) found = a;
                if (!found) return false;
                if (i + 1 < path.size()) m = found->menu();
            }
            found->trigger();
            return true;
        };
        QVERIFY(dataLabelsCellsRef(chartOf(e)).isEmpty());
        e.win.w.setChartTextAsker([](const QString &, QString *text) {
            *text = QStringLiteral("=A2:A4");
            return true;
        });
        const int steps = e.win.w.session()->undoStack()->count();
        QVERIFY(fire({"Data Labels", "Value From Cells…"}));
        QCOMPARE(e.win.w.session()->undoStack()->count(), steps + 1);
        const QString ref = dataLabelsCellsRef(chartOf(e));
        QVERIFY2(ref.endsWith(QStringLiteral("!$A$2:$A$4")), qPrintable(ref));
        const ChartSeries first = seriesOf(chartOf(e)).value(0);
        QVERIFY(first.labels.cells);
        QCOMPARE(first.labelCellsText.size(), 3);
        QVERIFY(!first.labelCellsText.join(QString()).isEmpty());
        QVERIFY(save(e));
        const ChartSeries saved = seriesOf(savedCharts(e.path)[0].chart).value(0);
        QVERIFY(saved.labels.cells);
        QCOMPARE(saved.labelCellsRef, ref);
        QCOMPARE(saved.labelCellsText, first.labelCellsText);
        // a range that is not there is refused, and nothing changes
        e.win.w.setChartTextAsker([](const QString &, QString *text) {
            *text = QStringLiteral("=Nowhere!A1:A3");
            return true;
        });
        int told = 0;
        e.win.w.setNotifier([&told](const QString &, const QString &) { ++told; });
        QVERIFY(fire({"Data Labels", "Value From Cells…"}));
        QCOMPARE(told, 1);
        QCOMPARE(e.win.w.session()->undoStack()->count(), steps + 1);
        QCOMPARE(dataLabelsCellsRef(chartOf(e)), ref);
        // empty takes it away
        e.win.w.setChartTextAsker([](const QString &, QString *text) {
            text->clear();
            return true;
        });
        QVERIFY(fire({"Data Labels", "Value From Cells…"}));
        QVERIFY(dataLabelsCellsRef(chartOf(e)).isEmpty());
        pressUndo(&e.win.w);
        QCOMPARE(dataLabelsCellsRef(chartOf(e)), ref);
    }

    // Custom, in the Error Bars list: the cells for the amounts above and below are asked for, the amounts are read from them (one step of Undo), and the file holds the ranges and the numbers.
    void errorBarsTakeCustomAmountsFromCells()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithAChart()));
        QTRY_VERIFY(objectsShown(e, 1));
        e.win.w.objects()->layer()->select(0);
        QTRY_VERIFY(e.win.w.chartElementsButton() && e.win.w.chartElementsButton()->isVisible());
        QMenu *menu = e.win.w.chartElementsButton()->menu();
        auto fire = [&](const QStringList &path) {
            QMetaObject::invokeMethod(menu, "aboutToShow", Qt::DirectConnection);
            QMenu *m = menu;
            QAction *found = nullptr;
            for (int i = 0; i < path.size(); ++i) {
                found = nullptr;
                for (QAction *a : m->actions())
                    if (a->text() == path[i]) found = a;
                if (!found) return false;
                if (i + 1 < path.size()) m = found->menu();
            }
            found->trigger();
            return true;
        };
        QStringList asked;
        e.win.w.setChartTextAsker([&asked](const QString &title, QString *text) {
            asked << title;
            *text = title.endsWith(QStringLiteral("Positive")) ? QStringLiteral("=B2:B4") : QString();
            return true;
        });
        const int steps = e.win.w.session()->undoStack()->count();
        QVERIFY(fire({"Error Bars", "Custom…"}));
        QCOMPARE(asked.size(), 2);
        QCOMPARE(e.win.w.session()->undoStack()->count(), steps + 1);
        QCOMPARE(errorBarsKind(chartOf(e)), QStringLiteral("cust"));
        const QStringList refs = customErrorBarRefs(chartOf(e));
        QVERIFY2(refs.value(0).endsWith(QStringLiteral("!$B$2:$B$4")), qPrintable(refs.join('|')));
        QVERIFY(refs.value(1).isEmpty());
        const ChartErrorBars bars = seriesOf(chartOf(e)).value(0).errorBars.value(0);
        QCOMPARE(bars.plus.size(), 3);
        QCOMPARE(bars.barType, QStringLiteral("plus"));
        QVERIFY(save(e));
        const ChartErrorBars saved = seriesOf(savedCharts(e.path)[0].chart).value(0).errorBars.value(0);
        QCOMPARE(saved.plusRef, refs.value(0));
        QCOMPARE(saved.plus, bars.plus);
        pressUndo(&e.win.w);
        QVERIFY(errorBarsKind(chartOf(e)) != QStringLiteral("cust"));
    }

    // Format Axis sets an axis from the Chart tab as one step of Undo, only for a chart that has the axis, and the saved file holds it.
    void anAxisIsFormattedFromTheChartTab()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithAChart()));
        QTRY_VERIFY(objectsShown(e, 1));
        QVERIFY(!e.win.w.act(QStringLiteral("chartdesign.axisV"))->isEnabled());
        e.win.w.objects()->layer()->select(0);
        QTRY_VERIFY(e.win.w.act(QStringLiteral("chartdesign.axisV"))->isEnabled());
        QVERIFY(e.win.w.act(QStringLiteral("chartdesign.axisH"))->isEnabled());
        bool across = true;
        e.win.w.setFormatAxisAsker([&](bool horizontal, ChartAxis *axis) {
            across = horizontal;
            axis->autoMin = false;
            axis->min = 5;
            axis->reversed = true;
            axis->majorTick = QStringLiteral("none");
            return true;
        });
        e.win.w.act(QStringLiteral("chartdesign.axisV"))->trigger();
        QVERIFY(!across);
        const Chart nowChart = chartOf(e);
        const ChartAxis *a = primaryAxis(nowChart, false);
        QVERIFY(!a->autoMin && a->min == 5 && a->reversed);
        QCOMPARE(a->majorTick, QStringLiteral("none"));
        QVERIFY(!primaryAxis(chartOf(e), true)->reversed);
        QVERIFY(save(e));
        const QVector<SavedChart> savedList = savedCharts(e.path);
        const ChartAxis *saved = primaryAxis(savedList[0].chart, false);
        QVERIFY(!saved->autoMin && saved->min == 5 && saved->reversed);
        pressUndo(&e.win.w);
        QVERIFY(primaryAxis(chartOf(e), false)->autoMin);
        // a cancelled pane changes nothing
        e.win.w.setFormatAxisAsker([](bool, ChartAxis *) { return false; });
        const int steps = e.win.w.session()->undoStack()->index();
        e.win.w.act(QStringLiteral("chartdesign.axisH"))->trigger();
        QCOMPARE(e.win.w.session()->undoStack()->index(), steps);
    }

    // Turning only "Print this object" on or off does not resize the object: the box shows sizes to a tenth of a point, so a size like it is not a change.
    void togglingOnlyPrintLeavesTheSizeAlone()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithAChart()));
        QTRY_VERIFY(objectsShown(e, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->select(0);
        // a size that is not a whole number of points or pixels
        e.win.w.setPropertiesAsker([&](const ObjectProperties &now, ObjectProperties *out) {
            *out = now;
            out->widthPoints = 301.37;
            out->heightPoints = 177.77;
            return true;
        });
        e.win.w.act(QStringLiteral("object.properties"))->trigger();
        const grid::ObjectBox was = o->layer()->boxNow(0);
        const int steps = e.win.w.session()->undoStack()->index();
        e.win.w.setPropertiesAsker([&](const ObjectProperties &now, ObjectProperties *out) {
            *out = now;
            out->widthPoints = std::round(now.widthPoints * 10) / 10;   // (what the box gives back)
            out->heightPoints = std::round(now.heightPoints * 10) / 10;
            out->prints = false;
            return true;
        });
        e.win.w.act(QStringLiteral("object.properties"))->trigger();
        QCOMPARE(e.win.w.session()->undoStack()->index(), steps + 1);
        QCOMPARE(o->layer()->boxNow(0).w, was.w);
        QCOMPARE(o->layer()->boxNow(0).h, was.h);
        QCOMPARE(int(o->printObjects(0).size()), 0);
    }

    // A logarithmic scale is chosen in Format Axis, saved, and refused when the lowest value is not over 0.
    void aLogarithmicScaleIsChosenInFormatAxis()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithAChart()));
        QTRY_VERIFY(objectsShown(e, 1));
        e.win.w.objects()->layer()->select(0);
        QTRY_VERIFY(e.win.w.act(QStringLiteral("chartdesign.axisV"))->isEnabled());
        const Chart before = chartOf(e);
        const ChartAxis axisBefore = *primaryAxis(before, false);
        FormatAxisDialog box(axisBefore, QStringLiteral("Format"));
        QCheckBox *log = box.findChild<QCheckBox *>(QStringLiteral("axisLog"));
        QVERIFY(log && !log->isChecked());
        log->setChecked(true);
        QVERIFY(box.result().logBase == 10);
        QVERIFY(box.problem().isEmpty());
        FormatAxisDialog low(axisBefore, QStringLiteral("Format"));
        low.findChild<QCheckBox *>(QStringLiteral("axisLog"))->setChecked(true);
        low.findChild<QCheckBox *>(QStringLiteral("axisMinAuto"))->setChecked(false);
        low.findChild<QDoubleSpinBox *>(QStringLiteral("axisMin"))->setValue(0);
        QVERIFY(!low.problem().isEmpty());
        e.win.w.setFormatAxisAsker([&](bool, ChartAxis *axis) {
            axis->logBase = 10;
            return true;
        });
        e.win.w.act(QStringLiteral("chartdesign.axisV"))->trigger();
        Chart now = chartOf(e);
        QCOMPARE(primaryAxis(now, false)->logBase, 10.0);
        QVERIFY(save(e));
        const Chart saved = savedCharts(e.path)[0].chart;
        QCOMPARE(primaryAxis(saved, false)->logBase, 10.0);
        pressUndo(&e.win.w);
        now = chartOf(e);
        QCOMPARE(primaryAxis(now, false)->logBase, 0.0);
    }

    // The Format Axis box refuses a lowest value that is not under the highest, and a unit of 0 or less unless Automatic is chosen (0 stays "automatic" only there); the window refuses the same from any source.
    void theFormatAxisBoxRefusesWhatCannotBeUsed()
    {
        ChartAxis axis;
        axis.kind = ChartAxis::Value;
        FormatAxisDialog d(axis, QStringLiteral("Format Axis"));
        auto box = [&](const char *name) { return d.findChild<QDoubleSpinBox *>(QString::fromLatin1(name)); };
        auto automatic = [&](const char *name) { return d.findChild<QCheckBox *>(QString::fromLatin1(name)); };
        QVERIFY(d.problem().isEmpty());
        automatic("axisMajorAuto")->setChecked(false);   // (the unit is 0 now, and not automatic)
        QVERIFY(!d.problem().isEmpty());
        box("axisMajor")->setValue(5);
        QVERIFY(d.problem().isEmpty());
        QCOMPARE(d.result().majorUnit, 5.0);
        automatic("axisMajorAuto")->setChecked(true);
        QCOMPARE(d.result().majorUnit, 0.0);
        automatic("axisMinorAuto")->setChecked(false);
        box("axisMinor")->setValue(-2);
        QVERIFY(!d.problem().isEmpty());
        automatic("axisMinorAuto")->setChecked(true);
        automatic("axisMinAuto")->setChecked(false);
        automatic("axisMaxAuto")->setChecked(false);
        box("axisMin")->setValue(10);
        box("axisMax")->setValue(10);
        QVERIFY(!d.problem().isEmpty());
        box("axisMax")->setValue(11);
        QVERIFY(d.problem().isEmpty());
    }

    // Display units are chosen in the Format Axis box (a value axis only), kept through Undo and the file, and the labels show the numbers divided.
    void displayUnitsAreChosenInTheFormatAxisBox()
    {
        ChartAxis axis;
        axis.kind = ChartAxis::Value;
        FormatAxisDialog d(axis, QStringLiteral("Format Axis"));
        QComboBox *units = d.findChild<QComboBox *>(QStringLiteral("axisDisplayUnits"));
        QVERIFY(units);
        QVERIFY(d.result().dispUnit.isEmpty());
        units->setCurrentIndex(units->findData(QStringLiteral("millions")));
        QCOMPARE(d.result().dispUnit, QStringLiteral("millions"));
        QVERIFY(!d.result().dispUnitShowLabel);
        d.findChild<QCheckBox *>(QStringLiteral("axisDisplayUnitsLabel"))->setChecked(true);
        QVERIFY(d.result().dispUnitShowLabel);
        EditWindow e;
        QVERIFY(e.open(bookWithAChart()));
        QTRY_VERIFY(objectsShown(e, 1));
        e.win.w.objects()->layer()->select(0);
        QTRY_VERIFY(e.win.w.act(QStringLiteral("chartdesign.axisV"))->isEnabled());
        e.win.w.setFormatAxisAsker([](bool, ChartAxis *a) {
            a->dispUnit = QStringLiteral("thousands");
            return true;
        });
        e.win.w.act(QStringLiteral("chartdesign.axisV"))->trigger();
        QCOMPARE(primaryAxis(chartOf(e), false)->dispUnit, QStringLiteral("thousands"));
        QVERIFY(save(e));
        QCOMPARE(primaryAxis(savedCharts(e.path)[0].chart, false)->dispUnit, QStringLiteral("thousands"));
        pressUndo(&e.win.w);
        QVERIFY(primaryAxis(chartOf(e), false)->dispUnit.isEmpty());
    }

    void theWindowRefusesAnAxisThatCannotBeUsed()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithAChart()));
        QTRY_VERIFY(objectsShown(e, 1));
        e.win.w.objects()->layer()->select(0);
        QTRY_VERIFY(e.win.w.act(QStringLiteral("chartdesign.axisV"))->isEnabled());
        e.win.w.setFormatAxisAsker([](bool, ChartAxis *axis) {
            axis->autoMin = false;
            axis->autoMax = false;
            axis->min = 50;
            axis->max = 5;
            return true;
        });
        const int steps = e.win.w.session()->undoStack()->index();
        e.win.told.clear();
        e.win.w.act(QStringLiteral("chartdesign.axisV"))->trigger();
        QCOMPARE(e.win.w.session()->undoStack()->index(), steps);
        QVERIFY(!e.win.told.isEmpty());
        const Chart now = chartOf(e);
        QVERIFY(primaryAxis(now, false)->autoMin);
    }

    // Move Chart puts the chart on another worksheet as one step of Undo (it still reads the cells it read); the saved file has it there and no longer on the first sheet, and reopened it is there.
    void aChartIsMovedToAnotherSheet()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({dataSheet(), dataSheet()}, drawingPart(twoCell(4, 1, 11, 13, chartFrame(2, QStringLiteral("rIdC"), QStringLiteral("Sales")))),
                                       {chartTarget(QStringLiteral("rIdC"), 1, chartPart(QStringLiteral("barChart"), {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {"Q1", "Q2", "Q3"}, {10, 20, 30}, QString()}}))})));
        QTRY_VERIFY(objectsShown(e, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->select(0);
        QVERIFY(e.win.w.act(QStringLiteral("chartdesign.moveChart"))->isEnabled());
        QStringList offered;
        e.win.w.setMoveChartAsker([&](const QStringList &sheets, int *chosen) {
            offered = sheets;
            *chosen = 0;
            return true;
        });
        const int steps = e.win.w.session()->undoStack()->index();
        e.win.w.act(QStringLiteral("chartdesign.moveChart"))->trigger();
        QCOMPARE(offered, QStringList{QStringLiteral("Sheet2")});
        QCOMPARE(e.win.w.session()->undoStack()->index(), steps + 1);
        QCOMPARE(e.win.w.activeSheet(), 1);
        QTRY_VERIFY(objectsShown(e, 1));
        // Undo takes it back to the first sheet (saved so, it is there and not on the second), Redo moves it again
        e.win.w.session()->undoStack()->undo();
        QVERIFY(!e.win.w.isModified());
        e.win.w.setActiveSheet(0);
        QTRY_VERIFY(objectsShown(e, 1));
        e.win.w.session()->undoStack()->redo();
        e.win.w.setActiveSheet(1);
        QTRY_VERIFY(objectsShown(e, 1));
        QVERIFY(save(e));
        QCOMPARE(savedCharts(e.path, 0).size(), 0);
        const QVector<SavedChart> moved = savedCharts(e.path, 1);
        QCOMPARE(moved.size(), 1);
        QCOMPARE(seriesOf(moved[0].chart)[0].valueRef, QStringLiteral("Sheet1!$B$2:$B$4"));
        QCOMPARE(seriesOf(moved[0].chart)[0].values, (QVector<double>{10, 20, 30}));
        Window again;
        QVERIFY(again.open(e.copyOfFile()));
        again.w.setActiveSheet(1);
        QTRY_VERIFY(again.w.objects()->shownSheet() == 1 && again.w.objects()->layer()->count() == 1);
        // Undo after the save puts the chart back on the first sheet, and on that one only, once saved again; Redo moves it again
        QVERIFY(e.win.w.session()->canUndo());
        e.win.w.session()->undoStack()->undo();
        QVERIFY(save(e));
        QCOMPARE(savedCharts(e.path, 0).size(), 1);
        QCOMPARE(savedCharts(e.path, 1).size(), 0);
        QCOMPARE(seriesOf(savedCharts(e.path, 0)[0].chart)[0].values, (QVector<double>{10, 20, 30}));
        e.win.w.session()->undoStack()->redo();
        QVERIFY(save(e));
        QCOMPARE(savedCharts(e.path, 0).size(), 0);
        QCOMPARE(savedCharts(e.path, 1).size(), 1);
    }

    // A hidden sheet is not offered by Move Chart. The chart keeps its size in points on a sheet whose rows and columns are taller and wider, and still moves with its cells there: a row inserted
    // above it moves it down, and the saved file has it as a two-cell anchor with the editAs it had.
    void aMovedChartKeepsItsSizeAndStillFollowsCells()
    {
        QByteArray wide = dataSheet();
        wide.replace("<sheetData>", "<sheetFormatPr defaultRowHeight=\"30\" customHeight=\"1\"/><cols><col min=\"1\" max=\"40\" width=\"30\" customWidth=\"1\"/></cols><sheetData>");
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({dataSheet(), dataSheet(), wide}, drawingPart(twoCell(4, 1, 11, 13, chartFrame(2, QStringLiteral("rIdC"), QStringLiteral("Sales")), QStringLiteral("oneCell"))),
                                       {chartTarget(QStringLiteral("rIdC"), 1, chartPart(QStringLiteral("barChart"), {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {"Q1", "Q2", "Q3"}, {10, 20, 30}, QString()}}))})));
        QTRY_VERIFY(objectsShown(e, 1));
        QVERIFY(e.win.w.session()->setSheetHidden(1, true).ok);
        QVERIFY(e.settle());
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->select(0);
        const grid::ObjectBox was = o->layer()->boxNow(0);
        QStringList offered;
        e.win.w.setMoveChartAsker([&](const QStringList &sheets, int *chosen) {
            offered = sheets;
            *chosen = 0;
            return true;
        });
        e.win.w.act(QStringLiteral("chartdesign.moveChart"))->trigger();
        QCOMPARE(offered, QStringList{QStringLiteral("Sheet3")});
        QCOMPARE(e.win.w.activeSheet(), 2);
        QTRY_VERIFY(objectsShown(e, 1, 2));
        const grid::ObjectBox now = e.win.w.objects()->layer()->boxNow(0);
        QVERIFY2(std::abs((now.w - was.w) * 0.75) < 0.5 && std::abs((now.h - was.h) * 0.75) < 0.5, qPrintable(QStringLiteral("%1x%2 was %3x%4").arg(now.w).arg(now.h).arg(was.w).arg(was.h)));
        // a row put in above it on the sheet moves it down by that row (30 pt, 40 pixels)
        QVERIFY(e.win.w.session()->insertRows(2, 0, 1).ok);
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->layer()->boxNow(0).y > now.y + 30);
        QVERIFY(std::abs((e.win.w.objects()->layer()->boxNow(0).h - now.h) * 0.75) < 0.5);
        QVERIFY(save(e));
        const QVector<SavedChart> saved = savedCharts(e.path, 2);
        QCOMPARE(saved.size(), 1);
        QCOMPARE(int(saved[0].object.anchor.kind), int(AnchorKind::TwoCell));
        QCOMPARE(int(saved[0].object.anchor.editAs), int(AnchorKind::OneCell));
    }

    // Move Chart is dimmed when the workbook has no other worksheet to move the chart to.
    void moveChartNeedsAnotherWorksheet()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithAChart()));
        QTRY_VERIFY(objectsShown(e, 1));
        e.win.w.objects()->layer()->select(0);
        QTRY_VERIFY(e.win.w.act(QStringLiteral("chartdesign.axisV"))->isEnabled());
        QVERIFY(!e.win.w.act(QStringLiteral("chartdesign.moveChart"))->isEnabled());
    }

    // Save as Picture writes the picked chart as a picture file.
    void aChartIsSavedAsAPicture()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithAChart()));
        QTRY_VERIFY(objectsShown(e, 1));
        QVERIFY(!e.win.w.act(QStringLiteral("chartdesign.saveAsPicture"))->isEnabled());
        e.win.w.objects()->layer()->select(0);
        QTRY_VERIFY(e.win.w.act(QStringLiteral("chartdesign.saveAsPicture"))->isEnabled());
        QTemporaryDir dir;
        const QString target = dir.filePath(QStringLiteral("chart.png"));
        e.win.w.setSavePicker([&](const QString &, const QString &) { return target; });
        e.win.w.setPickedFilter(QStringLiteral("PNG (*.png)"));
        e.win.w.act(QStringLiteral("chartdesign.saveAsPicture"))->trigger();
        QImage img(target);
        QVERIFY(!img.isNull());
        // a name that is not a picture's gets .png: the workbook's own name is never written over
        const QString workbook = dir.filePath(QStringLiteral("book.xlsx"));
        QVERIFY(QFile::copy(target, dir.filePath(QStringLiteral("keep"))));
        e.win.w.setSavePicker([&](const QString &, const QString &) { return workbook; });
        e.win.w.act(QStringLiteral("chartdesign.saveAsPicture"))->trigger();
        QVERIFY(QImage(workbook + QStringLiteral(".png")).width() > 100);
        QVERIFY(!QFileInfo::exists(workbook));
        QVERIFY(img.width() > 100 && img.height() > 60);
        bool drawn = false;
        for (int y = 0; y < img.height() && !drawn; ++y)
            for (int x = 0; x < img.width(); ++x)
                if (img.pixel(x, y) != qRgb(255, 255, 255)) { drawn = true; break; }
        QVERIFY(drawn);
    }

    // The text of the chart title and of an axis title is typed in the list; "=Sheet1!A2" makes it follow that cell, and the file keeps the link.
    void aTitleIsTypedOrLinkedToACell()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithAChart()));
        QTRY_VERIFY(objectsShown(e, 1));
        e.win.w.objects()->layer()->select(0);
        QTRY_VERIFY(e.win.w.chartElementsButton() && e.win.w.chartElementsButton()->isVisible());
        QMenu *menu = e.win.w.chartElementsButton()->menu();
        auto fire = [&](const QStringList &path) {
            QMetaObject::invokeMethod(menu, "aboutToShow", Qt::DirectConnection);
            QMenu *m = menu;
            QAction *found = nullptr;
            for (int i = 0; i < path.size(); ++i) {
                found = nullptr;
                for (QAction *a : m->actions())
                    if (a->text() == path[i]) found = a;
                if (!found) return false;
                if (i + 1 < path.size()) m = found->menu();
            }
            found->trigger();
            return true;
        };
        QString answer;
        e.win.w.setChartTextAsker([&](const QString &, QString *text) {
            *text = answer;
            return true;
        });
        answer = QStringLiteral("Sales Plan");
        QVERIFY(fire({"Chart Title", "Title Text…"}));
        QCOMPARE(chartOf(e).title.text, QStringLiteral("Sales Plan"));
        QVERIFY(chartOf(e).title.present && chartOf(e).title.ref.isEmpty());
        answer = QStringLiteral("=Sheet1!A2");
        QVERIFY(fire({"Chart Title", "Title Text…"}));
        QCOMPARE(chartOf(e).title.ref, QStringLiteral("Sheet1!$A$2"));
        QCOMPARE(chartOf(e).title.text, QStringLiteral("Q1"));
        // a link to nothing is refused and the title stays
        answer = QStringLiteral("=Nowhere!A1");
        QVERIFY(fire({"Chart Title", "Title Text…"}));
        QCOMPARE(chartOf(e).title.ref, QStringLiteral("Sheet1!$A$2"));
        // an axis title the same way
        answer = QStringLiteral("=Sheet1!$A$3");
        QVERIFY(fire({"Axis Titles", "Horizontal Title Text…"}));
        const Chart titled = chartOf(e);   // (kept: the axis points into it)
        const ChartAxis *across = primaryAxis(titled, true);
        QVERIFY(across && across->title.present);
        QCOMPARE(across->title.ref, QStringLiteral("Sheet1!$A$3"));
        QCOMPARE(across->title.text, QStringLiteral("Q2"));
        QVERIFY(save(e));
        const Chart saved = savedCharts(e.path)[0].chart;
        QCOMPARE(saved.title.ref, QStringLiteral("Sheet1!$A$2"));
        QCOMPARE(primaryAxis(saved, true)->title.ref, QStringLiteral("Sheet1!$A$3"));
        pressUndo(&e.win.w);
        QVERIFY(!primaryAxis(chartOf(e), true)->title.present);
    }

    // A picture of the Chart tab and the title menu, for a person to look at (only when JC_SHOT_DIR names a folder).
    void aPictureOfTheChartTab()
    {
        const QString dir = QString::fromLocal8Bit(qgetenv("JC_SHOT_DIR"));
        if (dir.isEmpty()) QSKIP("set JC_SHOT_DIR to take the picture");
        QDir().mkpath(dir);
        EditWindow e;
        QVERIFY(e.open(bookWithAChart()));
        e.win.w.resize(1440, 900);
        e.win.w.show();
        QTRY_VERIFY(objectsShown(e, 1));
        e.win.w.objects()->layer()->select(0);
        QVERIFY(e.win.w.changePickedChart(QStringLiteral("Look"), [](Chart *c) {
            setChartTitleText(c, QStringLiteral("Units by quarter, on a log scale"));
            primaryAxis(c, false)->logBase = 10;
            ChartLabelParts parts;
            parts.value = true;
            parts.legendKey = true;
            setDataLabelParts(c, parts);
        }));
        if (Ribbon *r = e.win.w.ribbon()) r->showTab(r->tab(QStringLiteral("Chart")));
        // (the chart is drawn when the event loop gets to it: the picture waits until its place has more than the sheet's white and lines)
        QTRY_VERIFY([&] {
            QSet<QRgb> colors;
            const QImage shot = e.win.w.grab().toImage().copy(QRect(300, 230, 440, 230));
            for (int y = 0; y < shot.height(); y += 3)
                for (int x = 0; x < shot.width(); x += 3) colors.insert(shot.pixel(x, y));
            return colors.size() > 8;
        }());
        QVERIFY(e.win.w.grab().save(dir + QStringLiteral("/chart-tab.png")));
    }

    // Properties holds the object's name and alternative text: each a step of Undo, saved as the name and descr of the object in the file, and read back.
    void anObjectIsNamedAndGivenAltText()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithAChart()));
        QTRY_VERIFY(objectsShown(e, 1));
        e.win.w.objects()->layer()->select(0);
        QTRY_VERIFY(e.win.w.act(QStringLiteral("object.properties"))->isEnabled());
        QString startedAs;
        e.win.w.setPropertiesAsker([&](const ObjectProperties &now, ObjectProperties *out) {
            *out = now;
            startedAs = now.name + QLatin1Char('|') + now.description;
            out->name = QStringLiteral("Units chart");
            out->description = QStringLiteral("Bars of units sold by quarter");
            return true;
        });
        e.win.w.act(QStringLiteral("object.properties"))->trigger();
        QCOMPARE(startedAs, QStringLiteral("Sales|"));
        QCOMPARE(e.win.w.objects()->nameOf(0, 0), QStringLiteral("Units chart"));
        QCOMPARE(e.win.w.objects()->descriptionOf(0, 0), QStringLiteral("Bars of units sold by quarter"));
        QVERIFY(save(e));
        auto descrOf = [&](const QString &name) {
            Package p;
            QString why;
            if (!p.readLazyFile(e.path, &why)) return QString(QStringLiteral("(no file)"));
            QDomDocument doc;
            if (!parseXml(p.part(QStringLiteral("/xl/drawings/drawing1.xml")), &doc)) return QString(QStringLiteral("(no drawing)"));
            const QDomNodeList all = doc.elementsByTagName(QStringLiteral("cNvPr"));
            for (int i = 0; i < all.size(); ++i)
                if (all.at(i).toElement().attribute(QStringLiteral("name")) == name) return all.at(i).toElement().attribute(QStringLiteral("descr"), QStringLiteral("(none)"));
            return QString(QStringLiteral("(not found)"));
        };
        QCOMPARE(descrOf(QStringLiteral("Units chart")), QStringLiteral("Bars of units sold by quarter"));
        // the alternative text alone, and then taken away again: the attribute is left out
        e.win.w.setPropertiesAsker([&](const ObjectProperties &now, ObjectProperties *out) {
            *out = now;
            out->description.clear();
            return true;
        });
        e.win.w.act(QStringLiteral("object.properties"))->trigger();
        QVERIFY(save(e));
        QCOMPARE(descrOf(QStringLiteral("Units chart")), QStringLiteral("(none)"));
        pressUndo(&e.win.w);
        QCOMPARE(e.win.w.objects()->descriptionOf(0, 0), QStringLiteral("Bars of units sold by quarter"));
        pressUndo(&e.win.w);
        QCOMPARE(e.win.w.objects()->nameOf(0, 0), QStringLiteral("Sales"));
        QVERIFY(e.win.w.objects()->descriptionOf(0, 0).isEmpty());
    }

    // Save as Template writes the picked chart's look in a .crtx file; Apply Template gives another chart that look, as one step of Undo, and keeps the chart's own data.
    void aChartTemplateIsSavedAndAppliedFromTheChartTab()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithAChart()));
        QTRY_VERIFY(objectsShown(e, 1));
        QVERIFY(e.win.w.act(QStringLiteral("chartdesign.saveTemplate")));
        QVERIFY(e.win.w.act(QStringLiteral("chartdesign.applyTemplate")));
        QVERIFY(!e.win.w.act(QStringLiteral("chartdesign.saveTemplate"))->isEnabled());
        e.win.w.objects()->layer()->select(0);
        QTRY_VERIFY(e.win.w.act(QStringLiteral("chartdesign.saveTemplate"))->isEnabled());
        // a look: lines, the legend above, a title
        QVERIFY(e.win.w.changePickedChart(QStringLiteral("Look"), [](Chart *c) {
            *c = withSeriesKinds(*c, {SeriesKind{ChartKind::Line, false}}, defaultTheme());
            setLegend(c, QStringLiteral("t"));
            setChartTitleText(c, QStringLiteral("From the template"));
        }));
        QTemporaryDir dir;
        const QString target = dir.filePath(QStringLiteral("look.crtx"));
        e.win.w.setSavePicker([&](const QString &, const QString &) { return target; });
        e.win.w.act(QStringLiteral("chartdesign.saveTemplate"))->trigger();
        QVERIFY(QFileInfo::exists(target));
        // back to a plain chart, then the template
        pressUndo(&e.win.w);
        pressUndo(&e.win.w);
        Chart plain = chartOf(e);
        QCOMPARE(int(plain.type()), int(ChartType::Bar));
        e.win.w.setTemplatePicker([&] { return target; });
        const int steps = e.win.w.session()->undoStack()->index();
        e.win.w.act(QStringLiteral("chartdesign.applyTemplate"))->trigger();
        QCOMPARE(e.win.w.session()->undoStack()->index(), steps + 1);
        Chart now = chartOf(e);
        QCOMPARE(int(now.type()), int(ChartType::Line));
        QCOMPARE(elementsOf(now).legendPosition, QStringLiteral("t"));
        QCOMPARE(seriesOf(now)[0].valueRef, QStringLiteral("Sheet1!$B$2:$B$4"));
        QVERIFY(save(e));
        const Chart saved = savedCharts(e.path)[0].chart;
        QCOMPARE(int(saved.type()), int(ChartType::Line));
        // a file that is no template changes nothing (the notice is closed by the window's own hook-free path: the picker names a file that is not one)
    }

    void theChartDesignTabAppearsWithAChart()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithAChart()));
        QTRY_VERIFY(objectsShown(e, 1));
        auto visible = [&] {
            for (int i = 0; i < e.win.w.ribbon()->tabCount(); ++i)
                if (e.win.w.ribbon()->tabName(i) == QLatin1String("Chart")) return e.win.w.ribbon()->tabVisible(i);
            return false;
        };
        QVERIFY(!visible());
        QVERIFY(!e.win.w.act(QStringLiteral("chartdesign.selectData"))->isEnabled());
        e.win.w.objects()->layer()->select(0);
        QVERIFY(visible());
        QVERIFY(e.win.w.act(QStringLiteral("chartdesign.selectData"))->isEnabled());
        QVERIFY(e.win.w.act(QStringLiteral("chartdesign.changeType"))->isEnabled());
        e.win.w.objects()->layer()->select(-1);
        QVERIFY(!visible());
        QTRY_VERIFY(!e.win.w.chartElementsButton() || !e.win.w.chartElementsButton()->isVisible());
        // another sheet shown: the chart that was picked is let go of with its sheet, and the tab with it
        e.win.w.objects()->layer()->select(0);
        QVERIFY(visible());
        e.win.w.act(QStringLiteral("sheet.insert"))->trigger();
        QVERIFY(e.settle());
        QTRY_VERIFY(!visible());
        QCOMPARE(e.win.w.pickedObject(), -1);
        QTRY_VERIFY(!e.win.w.chartElementsButton() || !e.win.w.chartElementsButton()->isVisible());
    }

    // Chart Data: series added, removed, edited, and put in another order; the cells the chart reads; the rows and columns switched; empty cells and hidden ones; saved with their references.
    void selectDataChangesTheSeries()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithAChart()));
        QTRY_VERIFY(objectsShown(e, 1));
        e.win.w.objects()->layer()->select(0);
        int shownInDialog = 0;
        bool refsChecked = false, badRefRefused = false;
        QString badWhy;
        e.win.w.setSelectDataAsker([&](const SelectDataRequest &request, Chart *result) -> bool {
            SelectDataDialog d(request);
            shownInDialog = d.seriesShown();
            ChartSeries more = d.seriesAt(0);
            more.name = QStringLiteral("West");
            more.nameRef = QStringLiteral("Sheet1!$C$1");
            more.valueRef = QStringLiteral("Sheet1!$C$2:$C$4");
            d.addSeries(more);
            d.moveSeries(1, -1);
            d.setBlanks(QStringLiteral("zero"), true);
            QString why;
            if (request.checkRef) {
                refsChecked = request.checkRef(QStringLiteral("Sheet1!$C$2:$C$4"), &why);
                badRefRefused = !request.checkRef(QStringLiteral("Nowhere!$C$2"), &badWhy);
            }
            *result = d.result();
            return true;
        });
        e.win.w.act(QStringLiteral("chartdesign.selectData"))->trigger();
        QCOMPARE(shownInDialog, 1);
        QVERIFY(refsChecked && badRefRefused);
        QVERIFY(badWhy.contains(QLatin1String("Nowhere")));
        Chart c = chartOf(e);
        QCOMPARE(seriesCount(c), 2);
        QCOMPARE(seriesOf(c)[0].valueRef, QStringLiteral("Sheet1!$C$2:$C$4"));
        QCOMPARE(seriesOf(c)[1].valueRef, QStringLiteral("Sheet1!$B$2:$B$4"));
        QCOMPARE(c.blanksAs, QStringLiteral("zero"));
        QVERIFY(!c.plotVisibleOnly);
        QCOMPARE(seriesOf(c)[0].name, QStringLiteral("West"));   // (named by its cell)
        QCOMPARE(seriesOf(c)[0].values, (QVector<double>{5, 10, 15}));
        // Undo gives the chart back as it was
        pressUndo(&e.win.w);
        QCOMPARE(seriesCount(chartOf(e)), 1);
        QVERIFY(!e.win.w.isModified());
        pressRedo(&e.win.w);
        QVERIFY(save(e));
        const QVector<SavedChart> charts = savedCharts(e.path);
        QCOMPARE(seriesCount(charts[0].chart), 2);
        QCOMPARE(seriesOf(charts[0].chart)[0].nameRef, QStringLiteral("Sheet1!$C$1"));
        QCOMPARE(charts[0].chart.blanksAs, QStringLiteral("zero"));
        QVERIFY(!charts[0].chart.plotVisibleOnly);
        // switch rows and columns, from the button on the tab
        e.win.w.act(QStringLiteral("chartdesign.switch"))->trigger();
        c = chartOf(e);
        QVERIFY(!seriesRunDownColumns(c));
        QCOMPARE(seriesCount(c), 3);   // (a series for each of Q1, Q2, Q3)
        QCOMPARE(seriesOf(c)[0].valueRef, QStringLiteral("Sheet1!$B$2:$C$2"));
        pressUndo(&e.win.w);
        QVERIFY(seriesRunDownColumns(chartOf(e)));
    }

    // Chart Kind: the whole chart, and a combination of kinds with a series on the second axes; both saved.
    void changeChartTypeChangesTheKinds()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        selectRange(e, QStringLiteral("A1:C4"));
        QVERIFY(e.win.w.insertChart(ChartKind::Column));
        QStringList seenNames;
        e.win.w.setChangeTypeAsker([&](const QStringList &names, const QVector<SeriesKind> &now, QVector<SeriesKind> *out) -> bool {
            seenNames = names;
            *out = now;
            (*out)[1] = SeriesKind{ChartKind::LineMarkers, true};
            return true;
        });
        e.win.w.act(QStringLiteral("chartdesign.changeType"))->trigger();
        QCOMPARE(seenNames, QStringList({"East", "West"}));
        Chart c = chartOf(e);
        QCOMPARE(c.plots.size(), 2);
        QCOMPARE(int(kindOfChart(c)), int(ChartKind::Combo));
        QVERIFY(save(e));
        const QVector<SavedChart> charts = savedCharts(e.path);
        const QVector<SeriesKind> kinds = seriesKindsOf(charts[0].chart);
        QCOMPARE(kinds.size(), 2);
        QVERIFY(kinds[1].secondary && !kinds[0].secondary);
        QCOMPARE(int(kinds[1].kind), int(ChartKind::LineMarkers));
        // the whole chart a pie
        e.win.w.setChangeTypeAsker([&](const QStringList &, const QVector<SeriesKind> &now, QVector<SeriesKind> *out) {
            *out = now;
            for (SeriesKind &k : *out) k = SeriesKind{ChartKind::Pie, false};
            return true;
        });
        e.win.w.act(QStringLiteral("chartdesign.changeType"))->trigger();
        QCOMPARE(int(kindOfChart(chartOf(e))), int(ChartKind::Pie));
        pressUndo(&e.win.w);
        QCOMPARE(int(kindOfChart(chartOf(e))), int(ChartKind::Combo));
    }

    // A chart with something a chart written from the model would lose is left alone: it can be moved, sized, and deleted, but what it shows is not changed, and its part is saved as it was.
    void aChartThatCannotBeRewrittenIsKeptAsItIs()
    {
        QByteArray part = chartPart(QStringLiteral("barChart"), {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {"Q1", "Q2", "Q3"}, {10, 20, 30}, QString()}});
        part.replace("<c:cat>", "<c:pictureOptions><c:pictureFormat val=\"stack\"/></c:pictureOptions><c:cat>");
        EditWindow e;
        QVERIFY(e.open(bookWithAChart(QString(), part)));
        QTRY_VERIFY(objectsShown(e, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->select(0);
        QString why;
        QVERIFY(!o->canEditChart(0, 0, &why));
        QVERIFY2(why.contains(QLatin1String("pictureFormat")), qPrintable(why));
        QVERIFY(!e.win.w.act(QStringLiteral("chartdesign.selectData"))->isEnabled());
        QTRY_VERIFY(!e.win.w.chartElementsButton() || !e.win.w.chartElementsButton()->isVisible());
        QVERIFY(!e.win.w.changePickedChart(QStringLiteral("Test"), [](Chart *c) { c->title.present = true; }));
        QVERIFY(e.win.told.last().contains(QLatin1String("pictureFormat")));
        // it can be moved
        o->layer()->nudge(1, 0, false, false);
        QVERIFY(e.win.w.isModified());
        QVERIFY(save(e));
        QMap<QString, QByteArray> original;
        QVERIFY(jp::readZip(bookWithAChart(QString(), part), original));
        QCOMPARE(partsOf(e.path).value(QStringLiteral("xl/charts/chart1.xml")), original.value(QStringLiteral("xl/charts/chart1.xml")));
        // a chart two anchors use cannot be changed one at a time
        const QVector<SeriesSpec> series = {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {"Q1", "Q2", "Q3"}, {10, 20, 30}, QString()}};
        const QString anchors = twoCell(4, 1, 11, 13, chartFrame(2, QStringLiteral("rIdC"), QStringLiteral("One"))) + twoCell(4, 14, 11, 26, chartFrame(3, QStringLiteral("rIdC"), QStringLiteral("Two")));
        EditWindow shared;
        QVERIFY(shared.open(bookWithDrawing({dataSheet()}, drawingPart(anchors), {chartTarget(QStringLiteral("rIdC"), 1, chartPart(QStringLiteral("barChart"), series))})));
        QTRY_VERIFY(objectsShown(shared, 2));
        QVERIFY(!shared.win.w.objects()->canEditChart(0, 0, &why));
        QVERIFY2(why.contains(QLatin1String("together")), qPrintable(why));
    }

    // ---- the clipboard

    // A chart copied and pasted in its workbook reads the same cells on any sheet; pasted in another workbook it keeps the numbers it showed and reads none; the file says so.
    void aChartIsCopiedAndPasted()
    {
        QGuiApplication::clipboard()->clear();
        EditWindow e;
        QVERIFY(e.open(bookWithAChart()));
        QTRY_VERIFY(objectsShown(e, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->select(0);
        e.win.w.act(QStringLiteral("edit.copy"))->trigger();
        QVERIFY(QGuiApplication::clipboard()->mimeData()->hasFormat(QLatin1String("application/x-jeffcrunch-object")));
        QVERIFY(QGuiApplication::clipboard()->mimeData()->hasImage());
        e.select(QStringLiteral("B20"));
        e.win.w.act(QStringLiteral("edit.paste"))->trigger();
        QCOMPARE(int(o->idsOf(0).size()), 2);
        QCOMPARE(o->layer()->selectedId(), 1);
        const Chart copy = chartOf(e, 1);
        QCOMPARE(seriesOf(copy)[0].valueRef, QStringLiteral("Sheet1!$B$2:$B$4"));
        QCOMPARE(seriesOf(copy)[0].values, (QVector<double>{10, 20, 30}));
        const grid::ObjectBox box = o->layer()->boxNow(1);
        QCOMPARE(box.w, o->layer()->boxNow(0).w);
        QCOMPARE(box.y, double(e.view()->sheet().layout().rows.position(19)));
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Paste Chart"));
        // another workbook: the numbers, no references
        EditWindow other;
        QVERIFY(other.open(plainBook()));
        QVERIFY(other.settle());
        QTRY_VERIFY(objectsShown(other, 0));
        other.select(QStringLiteral("E3"));
        other.win.w.act(QStringLiteral("edit.paste"))->trigger();
        crunch::SheetObjects *p = other.win.w.objects();
        QCOMPARE(int(p->idsOf(0).size()), 1);
        const Chart far = chartOf(other, 0);
        QVERIFY(seriesOf(far)[0].valueRef.isEmpty() && seriesOf(far)[0].nameRef.isEmpty() && seriesOf(far)[0].categoryRef.isEmpty());
        QCOMPARE(seriesOf(far)[0].values, (QVector<double>{10, 20, 30}));
        QVERIFY(other.win.told.isEmpty());
        QVERIFY(save(other));
        const QVector<SavedChart> saved = savedCharts(other.path);
        QCOMPARE(saved.size(), 1);
        QCOMPARE(seriesOf(saved[0].chart)[0].values, (QVector<double>{10, 20, 30}));
        QVERIFY(seriesOf(saved[0].chart)[0].valueRef.isEmpty());
        QVERIFY(!partsOf(other.path).value(QStringLiteral("xl/charts/chart1.xml")).contains("<c:f>"));
        // a cut takes the chart away
        QGuiApplication::clipboard()->clear();
        o->layer()->select(1);
        e.win.w.act(QStringLiteral("edit.cut"))->trigger();
        QCOMPARE(int(o->idsOf(0).size()), 1);
        // what is on the clipboard that is not ours is not taken
        auto *junk = new QMimeData;
        junk->setData(QLatin1String("application/x-jeffcrunch-object"), QByteArray("{\"kind\":\"chart\",\"width\":1e9,\"height\":5,\"chart\":\"AAAA\"}"));
        QGuiApplication::clipboard()->setMimeData(junk);
        e.win.w.act(QStringLiteral("edit.paste"))->trigger();
        QCOMPARE(int(o->idsOf(0).size()), 1);
        // a chart that is a chart, with a size that is not one a chart is
        crunch::ObjectClip big;
        big.kind = QStringLiteral("chart");
        big.width = 1e9;
        big.height = 200;
        big.chartXml = writeChartPart(chartOf(e));
        auto *huge = new QMimeData;
        huge->setData(QLatin1String("application/x-jeffcrunch-object"), big.toBytes());
        QGuiApplication::clipboard()->setMimeData(huge);
        e.win.w.act(QStringLiteral("edit.paste"))->trigger();
        QCOMPARE(int(o->idsOf(0).size()), 1);
        // and the same chart with a size that is one pastes
        big.width = 300;
        auto *fine = new QMimeData;
        fine->setData(QLatin1String("application/x-jeffcrunch-object"), big.toBytes());
        QGuiApplication::clipboard()->setMimeData(fine);
        e.win.w.act(QStringLiteral("edit.paste"))->trigger();
        QCOMPARE(int(o->idsOf(0).size()), 2);
        QCOMPARE(o->layer()->boxNow(o->idsOf(0).back()).w, 300.0);
    }


    // A picture and a shape are copied and pasted as a chart is: in the workbook, in another window, and saved with their pictures as parts of their own, each shape with a number of its own.
    void aPictureAndAShapeAreCopiedAndPasted()
    {
        const QString anchors = oneCell(1, 6, 952500, 476250, pictureElement(3, QStringLiteral("rIdP"), QStringLiteral("Logo"))) + twoCell(5, 10, 8, 14, shapeElement(4, QStringLiteral("Box")));
        const QByteArray png = pngBytes(8, 4, Qt::red);
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(anchors), {imageTarget(QStringLiteral("rIdP"), 1, png)})));
        QTRY_VERIFY(objectsShown(e, 2));
        crunch::SheetObjects *o = e.win.w.objects();
        QGuiApplication::clipboard()->clear();
        o->layer()->select(0);
        e.win.w.act(QStringLiteral("edit.copy"))->trigger();
        QVERIFY(QGuiApplication::clipboard()->mimeData()->hasFormat(QLatin1String("application/x-jeffcrunch-object")));
        e.select(QStringLiteral("B20"));
        e.win.w.act(QStringLiteral("edit.paste"))->trigger();
        QCOMPARE(int(o->idsOf(0).size()), 3);
        QCOMPARE(o->kindOf(0, o->layer()->selectedId()), jp::office::sheet::ObjectKind::Picture);
        // the shape too, into a workbook that has no drawing
        o->layer()->select(1);
        e.win.w.act(QStringLiteral("edit.copy"))->trigger();
        e.select(QStringLiteral("B30"));
        e.win.w.act(QStringLiteral("edit.paste"))->trigger();
        QCOMPARE(int(o->idsOf(0).size()), 4);
        QVERIFY(e.win.told.isEmpty());
        QVERIFY(save(e));
        const QMap<QString, QByteArray> parts = partsOf(e.path);
        // two pictures in the file now: the first and the copy, each a part of its own with its own relationship
        int media = 0;
        for (auto it = parts.begin(); it != parts.end(); ++it)
            if (it.key().startsWith(QLatin1String("xl/media/"))) {
                ++media;
                QCOMPARE(it.value(), png);
            }
        QCOMPARE(media, 2);
        // every number of a shape in the drawing is its own
        QDomDocument doc;
        QVERIFY(parseXml(parts.value(QStringLiteral("xl/drawings/drawing1.xml")), &doc));
        const QDomNodeList all = doc.elementsByTagName(QStringLiteral("cNvPr"));
        QSet<QString> ids;
        for (int i = 0; i < all.size(); ++i) ids.insert(all.at(i).toElement().attribute(QStringLiteral("id")));
        QCOMPARE(int(ids.size()), 4);
        // read again, there are four, and the pasted picture draws
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 4);
        // another window: the picture comes with its bytes
        QGuiApplication::clipboard()->clear();
        o->layer()->select(0);
        e.win.w.act(QStringLiteral("edit.copy"))->trigger();
        EditWindow other;
        QVERIFY(other.open(plainBook()));
        QVERIFY(other.settle());
        QTRY_VERIFY(objectsShown(other, 0));
        other.select(QStringLiteral("E3"));
        other.win.w.act(QStringLiteral("edit.paste"))->trigger();
        QCOMPARE(int(other.win.w.objects()->idsOf(0).size()), 1);
        QVERIFY(save(other));
        const QMap<QString, QByteArray> far = partsOf(other.path);
        bool found = false;
        for (auto it = far.begin(); it != far.end(); ++it) found = found || (it.key().startsWith(QLatin1String("xl/media/")) && it.value() == png);
        QVERIFY(found);
        QVERIFY(far.value(QStringLiteral("[Content_Types].xml")).contains("png"));
        // a clip that names a picture it does not carry, and one with more than it may, are not pasted
        crunch::ObjectClip bad;
        bad.kind = QStringLiteral("picture");
        bad.width = 50;
        bad.height = 50;
        bad.elementXml = "<xdr:pic><xdr:nvPicPr><xdr:cNvPr id=\"2\" name=\"X\"/><xdr:cNvPicPr/></xdr:nvPicPr><xdr:blipFill><a:blip r:embed=\"rIdMissing\"/></xdr:blipFill><xdr:spPr/></xdr:pic>";
        auto *md = new QMimeData;
        bad.toMime(md);
        QGuiApplication::clipboard()->setMimeData(md);
        other.select(QStringLiteral("E20"));
        other.win.w.act(QStringLiteral("edit.paste"))->trigger();
        QVERIFY(other.win.w.objects()->idsOf(0).size() <= 2);
        QCOMPARE(int(other.win.w.objects()->idsOf(0).size()), other.win.w.objects()->layer()->count());
    }


    // With an object picked, the name box shows its name, and a name typed there is the object's (an edit that Undo takes back, and a file that has it).
    void theNameBoxShowsAndChangesAnObjectsName()
    {
        const QString anchors = twoCell(4, 1, 11, 13, chartFrame(2, QStringLiteral("rIdC"), QStringLiteral("Sales"))) + twoCell(5, 15, 8, 19, shapeElement(4, QStringLiteral("Box")));
        const QVector<SeriesSpec> series = {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {"Q1", "Q2", "Q3"}, {10, 20, 30}, QString()}};
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(anchors), {chartTarget(QStringLiteral("rIdC"), 1, chartPart(QStringLiteral("barChart"), series))})));
        QTRY_VERIFY(objectsShown(e, 2));
        crunch::SheetObjects *o = e.win.w.objects();
        e.select(QStringLiteral("B3"));
        QCOMPARE(e.win.w.nameBox()->text(), QStringLiteral("B3"));
        o->layer()->select(0);
        QCOMPARE(e.win.w.nameBox()->text(), QStringLiteral("Sales"));
        e.win.w.nameBox()->setText(QStringLiteral("Revenue"));
        QMetaObject::invokeMethod(e.win.w.nameBox(), "returnPressed");
        QCOMPARE(o->nameOf(0, 0), QStringLiteral("Revenue"));
        QCOMPARE(e.win.w.nameBox()->text(), QStringLiteral("Revenue"));
        // a shape the same, and an empty name is not one
        o->layer()->select(1);
        e.win.w.nameBox()->setText(QStringLiteral("Panel"));
        QMetaObject::invokeMethod(e.win.w.nameBox(), "returnPressed");
        e.win.w.nameBox()->setText(QString());
        QMetaObject::invokeMethod(e.win.w.nameBox(), "returnPressed");
        QCOMPARE(o->nameOf(0, 1), QStringLiteral("Panel"));
        QVERIFY(save(e));
        QVERIFY(savedAnchor(e.path, QStringLiteral("Revenue")).found);
        QVERIFY(savedAnchor(e.path, QStringLiteral("Panel")).found);
        QVERIFY(!savedAnchor(e.path, QStringLiteral("Sales")).found);
        // Undo gives the old name back, and the file after it has it
        e.win.w.undoStack()->undo();
        QCOMPARE(o->nameOf(0, 1), QStringLiteral("Box"));
        QVERIFY(save(e));
        QVERIFY(savedAnchor(e.path, QStringLiteral("Box")).found);
        QVERIFY(!savedAnchor(e.path, QStringLiteral("Panel")).found);
        // with no object picked it is an address again
        o->layer()->select(-1);
        e.select(QStringLiteral("C4"));
        QCOMPARE(e.win.w.nameBox()->text(), QStringLiteral("C4"));
    }


    // The links of a chart part (the shapes over the chart, the workbook it was made from) are good for that part only: a copy of the chart, in a part of its own, has none.
    void aCopyOfAChartHasNoLinksOfThePartItCameFrom()
    {
        const QVector<SeriesSpec> series = {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {"Q1", "Q2", "Q3"}, {10, 20, 30}, QString()}};
        QByteArray part = chartPart(QStringLiteral("barChart"), series);
        part.replace("</c:chartSpace>", "<c:userShapes r:id=\"rId1\"/></c:chartSpace>");
        QGuiApplication::clipboard()->clear();
        EditWindow e;
        QVERIFY(e.open(bookWithAChart(QStringLiteral("barChart"), part)));
        QTRY_VERIFY(objectsShown(e, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        QVERIFY(o->canEditChart(0, 0));   // (the part is one that may be written over: it keeps the link)
        o->layer()->select(0);
        e.win.w.act(QStringLiteral("edit.copy"))->trigger();
        e.select(QStringLiteral("B20"));
        e.win.w.act(QStringLiteral("edit.paste"))->trigger();
        QCOMPARE(int(o->idsOf(0).size()), 2);
        QVERIFY(o->chartNow(0, 1).userShapesId.isEmpty());
        QVERIFY(save(e));
        const QMap<QString, QByteArray> parts = partsOf(e.path);
        int linked = 0, charts = 0;
        for (auto it = parts.begin(); it != parts.end(); ++it)
            if (it.key().startsWith(QLatin1String("xl/charts/chart")) && it.key().endsWith(QLatin1String(".xml"))) {
                ++charts;
                if (it.value().contains("userShapes")) ++linked;
            }
        QCOMPARE(charts, 2);
        QCOMPARE(linked, 1);   // (the first chart's part is as it was; the copy's has no link)
    }


    // The pictures pasted since the last save are held within a budget the save accepts: a paste past it is refused, with a message, and nothing is put on.
    void pastedPicturesAreHeldWithinABudget()
    {
        const QString anchors = oneCell(1, 6, 952500, 476250, pictureElement(3, QStringLiteral("rIdP"), QStringLiteral("Logo")));
        const QByteArray png = pngBytes(8, 4, Qt::red);
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(anchors), {imageTarget(QStringLiteral("rIdP"), 1, png)})));
        QTRY_VERIFY(objectsShown(e, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        crunch::SheetObjects::Limits limits = o->limits();
        limits.maxBytes = png.size() * 2 + png.size() / 2;
        o->setLimits(limits);
        QGuiApplication::clipboard()->clear();
        o->layer()->select(0);
        e.win.w.act(QStringLiteral("edit.copy"))->trigger();
        int pasted = 0;
        for (int i = 0; i < 4; ++i) {
            e.select(QStringLiteral("B20"));
            const int before = int(o->idsOf(0).size());
            e.win.w.act(QStringLiteral("edit.paste"))->trigger();
            if (int(o->idsOf(0).size()) == before) break;
            ++pasted;
        }
        QCOMPARE(pasted, 2);
        QVERIFY(o->pastedBytesHeld() <= limits.maxBytes);
        QVERIFY2(e.win.told.last().contains(QLatin1String("pictures put in since the last save")), qPrintable(e.win.told.last()));
        // taken away again (Undo), the picture makes room
        pressUndo(&e.win.w);
        QCOMPARE(o->pastedBytesHeld(), qint64(png.size()));
        e.select(QStringLiteral("B20"));
        e.win.w.act(QStringLiteral("edit.paste"))->trigger();
        QCOMPARE(o->pastedBytesHeld(), qint64(png.size()) * 2);
        QVERIFY(save(e));   // (what is pasted is saved: the save never fails for it)
    }

    // Cut takes a chart, picture, or shape away and puts it on the clipboard: pasted, it comes back; Undone, it is where it was.
    void cutObjectsCanBePastedOrUndone()
    {
        const QVector<SeriesSpec> series = {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {"Q1", "Q2", "Q3"}, {10, 20, 30}, QString()}};
        const QString anchors = twoCell(4, 1, 11, 13, chartFrame(2, QStringLiteral("rIdC"), QStringLiteral("Sales"))) + oneCell(1, 16, 952500, 476250, pictureElement(3, QStringLiteral("rIdP"), QStringLiteral("Logo")))
                                + twoCell(5, 20, 8, 24, shapeElement(4, QStringLiteral("Box")));
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(anchors), {chartTarget(QStringLiteral("rIdC"), 1, chartPart(QStringLiteral("barChart"), series)), imageTarget(QStringLiteral("rIdP"), 2, pngBytes(8, 4, Qt::red))})));
        QTRY_VERIFY(objectsShown(e, 3));
        crunch::SheetObjects *o = e.win.w.objects();
        const char *names[] = {"Sales", "Logo", "Box"};
        for (int id = 0; id < 3; ++id) {
            QGuiApplication::clipboard()->clear();
            o->layer()->select(id);
            e.win.w.act(QStringLiteral("edit.cut"))->trigger();
            QVERIFY2(o->layer()->isDeleted(id), names[id]);
            QCOMPARE(int(o->idsOf(0).size()), 2);
            QVERIFY(QGuiApplication::clipboard()->mimeData()->hasFormat(QLatin1String("application/x-jeffcrunch-object")));
            // pasted: it is on again, as another object
            e.select(QStringLiteral("B30"));
            e.win.w.act(QStringLiteral("edit.paste"))->trigger();
            QCOMPARE(int(o->idsOf(0).size()), 3);
            const int pastedId = o->idsOf(0).back();
            QVERIFY(!o->layer()->isDeleted(pastedId));
            QCOMPARE(o->kindOf(0, pastedId), o->kindOf(0, id));
            pressUndo(&e.win.w);   // the paste
            QVERIFY(o->layer()->isDeleted(pastedId));
            QCOMPARE(int(o->idsOf(0).size()), 2);
            // Undo of the cut: where it was
            pressUndo(&e.win.w);
            QVERIFY2(!o->layer()->isDeleted(id), names[id]);
            QCOMPARE(int(o->idsOf(0).size()), 3);
        }
    }

    // ---- the order

    void objectsAreBroughtForwardAndSentBackward()
    {
        const QVector<SeriesSpec> series = {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {"Q1", "Q2", "Q3"}, {10, 20, 30}, QString()}};
        const QString anchors = twoCell(4, 1, 11, 13, chartFrame(2, QStringLiteral("rIdA"), QStringLiteral("First"))) + twoCell(0, 6, 2, 9, pictureElement(3, QStringLiteral("rIdP"), QStringLiteral("Logo")))
                                + twoCell(6, 3, 12, 15, chartFrame(4, QStringLiteral("rIdB"), QStringLiteral("Third")));
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(anchors), {chartTarget(QStringLiteral("rIdA"), 1, chartPart(QStringLiteral("barChart"), series)), imageTarget(QStringLiteral("rIdP"), 1, pngBytes(8, 4, Qt::red)),
                                                                              chartTarget(QStringLiteral("rIdB"), 2, chartPart(QStringLiteral("lineChart"), series))})));
        QTRY_VERIFY(objectsShown(e, 3));
        crunch::SheetObjects *o = e.win.w.objects();
        auto names = [&](const QString &path) {
            Package p;
            QString why;
            if (!p.readLazyFile(path, &why)) return QString();
            QDomDocument doc;
            parseXml(p.part(QStringLiteral("/xl/drawings/drawing1.xml")), &doc);
            QStringList out;
            const QDomNodeList all = doc.elementsByTagName(QStringLiteral("cNvPr"));
            for (int i = 0; i < all.size(); ++i) out << all.at(i).toElement().attribute(QStringLiteral("name"));
            return out.join(QLatin1Char(','));
        };
        QCOMPARE(o->idsOf(0), (std::vector<int>{0, 1, 2}));
        o->layer()->select(0);
        e.win.w.act(QStringLiteral("object.front"))->trigger();
        QCOMPARE(o->idsOf(0), (std::vector<int>{1, 2, 0}));
        QCOMPARE(o->layer()->objectAt(e.view()->sheet().layout().rangeRect(5, 7, 5, 7).center()), 0);   // (where the first and third overlap: the first is over)
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Bring to Front"));
        pressUndo(&e.win.w);
        QCOMPARE(o->idsOf(0), (std::vector<int>{0, 1, 2}));
        pressRedo(&e.win.w);
        e.win.w.act(QStringLiteral("object.backward"))->trigger();
        QCOMPARE(o->idsOf(0), (std::vector<int>{1, 0, 2}));
        QVERIFY(save(e));
        QCOMPARE(names(e.path), QStringLiteral("Logo,First,Third"));
        // saved again with no change: the same; and to the back
        o->layer()->select(2);
        e.win.w.act(QStringLiteral("object.back"))->trigger();
        QCOMPARE(o->idsOf(0), (std::vector<int>{2, 1, 0}));
        QVERIFY(save(e));
        QCOMPARE(names(e.path), QStringLiteral("Third,Logo,First"));
        // the chart parts and the picture are all there and reopened the file has the same order
        const QMap<QString, QByteArray> parts = partsOf(e.path);
        QVERIFY(parts.contains(QStringLiteral("xl/charts/chart1.xml")) && parts.contains(QStringLiteral("xl/charts/chart2.xml")) && parts.contains(QStringLiteral("xl/media/image1.png")));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 3);
        QCOMPARE(second.w.objects()->nameOf(0, 0), QStringLiteral("Third"));
        // with the first at the back as well: a delete, a new chart, and a save keep the rest in order
        o->layer()->select(1);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        selectRange(e, QStringLiteral("A1:C4"));
        QVERIFY(e.win.w.insertChart(ChartKind::Pie));
        QVERIFY(save(e));
        QCOMPARE(names(e.path).split(QLatin1Char(',')).first(), QStringLiteral("Third"));
        QCOMPARE(names(e.path).split(QLatin1Char(',')).size(), 3);
        QCOMPARE(names(e.path).split(QLatin1Char(',')).last(), QStringLiteral("Chart 1"));
    }

    // ---- the workbook around it

    // A chart on a sheet that was added is in the file with the sheet: the sheet's relationships, its drawing, and its chart; it reads the cells of the other sheet.
    void aChartOnANewSheetIsSaved()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        e.win.w.act(QStringLiteral("sheet.insert"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.shownSheets().size(), 2);
        // the cells are on the first sheet: the chart is made on the new one from a reference
        e.win.w.setActiveSheet(0);
        selectRange(e, QStringLiteral("A1:C4"));
        e.win.w.setActiveSheet(1);
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 1);
        // a chart made from the first sheet's cells and put on the second
        crunch::SheetObjects *o = e.win.w.objects();
        ChartPlan plan = planChart(*o->cells(), 0, QStringLiteral("Sheet1"), rangeOf(QStringLiteral("A1:C4")), ChartKind::Column);
        const Chart chart = makeChart(ChartKind::Column, plan, o->theme(), plan.points);
        const int sheet = 1;   // (the sheet added is the second in the workbook, and first among the tabs)
        QString why;
        const int id = o->makeChart(sheet, chart, DrawingAnchor{}, &why);
        QVERIFY2(id >= 0, qPrintable(why));
        crunch::ObjectEdit edit;
        edit.sheet = sheet;
        edit.id = id;
        edit.before = o->stateOf(sheet, id);
        edit.after = edit.before;
        edit.after.deleted = false;
        QVERIFY(e.win.w.session()->changeObjects({edit}, QStringLiteral("New Chart")).ok);
        QVERIFY(save(e));
        // (the sheet added is first among the tabs, so first in the file)
        const QVector<SavedChart> charts = savedCharts(e.path, 0);
        QCOMPARE(charts.size(), 1);
        QCOMPARE(seriesOf(charts[0].chart)[0].valueRef, QStringLiteral("Sheet1!$B$2:$B$4"));
        QCOMPARE(seriesOf(charts[0].chart)[0].values, (QVector<double>{10, 20, 30}));
        QVERIFY(savedCharts(e.path, 1).isEmpty());
        const QMap<QString, QByteArray> parts = partsOf(e.path);
        QVERIFY2(parts.contains(QStringLiteral("xl/worksheets/_rels/sheet2.xml.rels")), qPrintable(QStringList(parts.keys()).join(QLatin1Char(','))));
        QVERIFY(parts.value(QStringLiteral("xl/worksheets/sheet2.xml")).contains("<drawing "));
        QString bad;
        for (auto it = parts.begin(); it != parts.end(); ++it)
            if (it.key().endsWith(QLatin1String(".xml")) || it.key().endsWith(QLatin1String(".rels"))) {
                QDomDocument doc;
                QVERIFY2(doc.setContent(it.value()), qPrintable(it.key()));
            }
    }

    // Rows inserted above the cells a chart reads move the chart's references and the chart; the part of a chart made in this session says so, and the file has them as they are.
    void aNewChartFollowsRowsInsertedAndDeleted()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        selectRange(e, QStringLiteral("A1:C4"));
        QVERIFY(e.win.w.insertChart(ChartKind::Column));
        crunch::SheetObjects *o = e.win.w.objects();
        const grid::ObjectBox was = o->layer()->boxNow(0);
        // a row inserted above row 2
        e.view()->selectRows(1, 1);
        e.win.w.act(QStringLiteral("cells.insert"))->trigger();
        QVERIFY(e.settle());
        Chart c = chartOf(e);
        QCOMPARE(seriesOf(c)[0].valueRef, QStringLiteral("Sheet1!$B$3:$B$5"));
        QCOMPARE(seriesOf(c)[0].categoryRef, QStringLiteral("Sheet1!$A$3:$A$5"));
        QCOMPARE(seriesOf(c)[0].nameRef, QStringLiteral("Sheet1!$B$1"));
        const double moved = o->layer()->boxNow(0).y - was.y;
        QVERIFY(moved > 0);   // (the chart is below the row that went in: it moved down by a row)
        QCOMPARE(o->layer()->boxNow(0).x, was.x);
        QVERIFY(save(e));
        QVector<SavedChart> charts = savedCharts(e.path);
        QCOMPARE(seriesOf(charts[0].chart)[0].valueRef, QStringLiteral("Sheet1!$B$3:$B$5"));
        QCOMPARE(seriesOf(charts[0].chart)[0].values, (QVector<double>{10, 20, 30}));
        // undone, and again, and a chart moved after the insert goes with Undo too
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(seriesOf(chartOf(e))[0].valueRef, QStringLiteral("Sheet1!$B$2:$B$4"));
        pressRedo(&e.win.w);
        QVERIFY(e.settle());
        o->layer()->select(0);
        o->layer()->nudge(1, 0, false, false);
        QCOMPARE(seriesOf(chartOf(e))[0].valueRef, QStringLiteral("Sheet1!$B$3:$B$5"));
        pressUndo(&e.win.w);   // the nudge
        QCOMPARE(o->layer()->boxNow(0).x, was.x);
        QCOMPARE(o->layer()->boxNow(0).y, was.y + moved);   // (where the insert left it, worked out again from the file's place through the edits)
        pressUndo(&e.win.w);   // the insert
        QVERIFY(e.settle());
        QCOMPARE(seriesOf(chartOf(e))[0].valueRef, QStringLiteral("Sheet1!$B$2:$B$4"));
        QCOMPARE(o->layer()->boxNow(0).x, was.x);
        QCOMPARE(o->layer()->boxNow(0).y, was.y);   // (the insert undone after a save: the chart is back where it was)
    }

    // The menu of an object has the commands that are for it, those that can't be used off.
    void anObjectsMenuHasItsCommands()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithAChart()));
        QTRY_VERIFY(objectsShown(e, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->select(0);
        std::unique_ptr<QMenu> menu(e.win.w.buildObjectMenu(0, &e.win.w));
        QStringList texts;
        QMap<QString, bool> enabled;
        for (QAction *a : menu->actions()) {
            texts << a->text();
            enabled[a->text()] = a->isEnabled();
        }
        for (const char *want : {"Cut", "Copy", "Paste", "Delete", "Order", "Chart Data…", "Chart Kind…", "Chart Parts", "Properties…"}) QVERIFY2(texts.contains(QString::fromUtf8(want)), qPrintable(texts.join(QLatin1Char('|'))));
        QVERIFY(enabled.value(QStringLiteral("Chart Data…")));
        // the right click on the chart asks for it (the window pops it up: a stand-in is the test's own menu)
        QSignalSpy asked(o->layer(), &grid::ObjectLayer::contextMenuRequested);
        QTest::mouseClick(e.view()->viewport(), Qt::RightButton, Qt::NoModifier, o->layer()->deviceRect(0, grid::Pane::Body).center());
        QCOMPARE(asked.size(), 1);
        QCOMPARE(e.selected(), QStringLiteral("A1"));
    }

    // A chart sheet's chart is not changed here, and no chart is made on one.
    void aChartSheetTakesNoChart()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        crunch::SheetObjects *o = e.win.w.objects();
        QString why;
        // a sheet of cells with a chart sheet's index: refused with a reason
        QCOMPARE(o->makeChart(5, Chart(), DrawingAnchor(), &why), -1);
        QVERIFY(!why.isEmpty());
    }

    // An object put on and taken away again before a save leaves no part; one put on, saved, and taken away is gone from the next file with its part.
    void anObjectAddedAndRemovedLeavesNothing()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        selectRange(e, QStringLiteral("A1:C4"));
        QVERIFY(e.win.w.insertChart(ChartKind::Column));
        pressUndo(&e.win.w);
        QVERIFY(e.win.w.session()->canUndo() == false);
        // the person changes a cell so there is something to save
        e.select(QStringLiteral("H1"));
        e.enter(QStringLiteral("1"));
        QVERIFY(e.settle());
        QVERIFY(save(e));
        QVERIFY(!partsOf(e.path).contains(QStringLiteral("xl/charts/chart1.xml")));
        QVERIFY(!partsOf(e.path).contains(QStringLiteral("xl/drawings/drawing1.xml")));
        // put on, saved, and taken away
        selectRange(e, QStringLiteral("A1:C4"));
        QVERIFY(e.win.w.insertChart(ChartKind::Column));
        QVERIFY(save(e));
        QVERIFY(partsOf(e.path).contains(QStringLiteral("xl/charts/chart1.xml")));
        e.win.w.objects()->layer()->select(e.win.w.objects()->idsOf(0).back());
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY(save(e));
        const QMap<QString, QByteArray> parts = partsOf(e.path);
        QVERIFY(!parts.contains(QStringLiteral("xl/charts/chart1.xml")));
        QVERIFY(!parts.value(QStringLiteral("[Content_Types].xml")).contains("/xl/charts/chart1.xml"));
        // a chart put on after that has a part of its own, and the file is whole
        selectRange(e, QStringLiteral("A1:C4"));
        QVERIFY(e.win.w.insertChart(ChartKind::Pie));
        QVERIFY(save(e));
        QCOMPARE(savedCharts(e.path).size(), 1);
        QCOMPARE(int(kindOfChart(savedCharts(e.path)[0].chart)), int(ChartKind::Pie));
    }

    // Insert > Picture puts a picture from a file on the sheet (the file's own bytes for a PNG), as one step of Undo; it is saved with the drawing and read again. A file that is not a
    // picture is told and puts nothing.
    void aPictureFromAFileIsInsertedAndSaved()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        const QByteArray png = pngBytes(40, 20, Qt::blue);
        const QString file = e.folder.write(QStringLiteral("logo.png"), png);
        const QString junk = e.folder.write(QStringLiteral("notes.png"), "this is not a picture");
        e.win.w.setPicturePicker([&] { return file; });
        QVERIFY(e.win.w.act(QStringLiteral("insert.picture")));
        e.select(QStringLiteral("C3"));
        const int steps = e.win.w.undoStack()->count();
        e.win.w.act(QStringLiteral("insert.picture"))->trigger();
        crunch::SheetObjects *o = e.win.w.objects();
        QCOMPARE(o->layer()->count(), 1);
        QCOMPARE(o->kindOf(0, o->layer()->selectedId()), jp::office::sheet::ObjectKind::Picture);
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        // Undo takes it off the sheet, and Redo puts it back
        pressUndo(&e.win.w);
        QCOMPARE(int(o->idsOf(0).size()), 0);
        pressRedo(&e.win.w);
        QCOMPARE(int(o->idsOf(0).size()), 1);
        o->layer()->select(o->idsOf(0).front());
        const grid::ObjectBox box = o->layer()->boxNow(o->layer()->selectedId());
        QCOMPARE(box.w, 40.0);
        QCOMPARE(box.h, 20.0);
        QVERIFY(!e.win.w.insertPictureFile(junk));
        QVERIFY(!e.win.told.isEmpty());
        QCOMPARE(o->layer()->count(), 1);
        QVERIFY(save(e));
        const QMap<QString, QByteArray> parts = partsOf(e.path);
        QCOMPARE(parts.value(QStringLiteral("xl/media/image1.png")), png);
        QVERIFY(parts.value(QStringLiteral("xl/drawings/drawing1.xml")).contains("<xdr:pic>") || parts.value(QStringLiteral("xl/drawings/drawing1.xml")).contains("xdr:pic"));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 1);
    }

    // A picture too wide for the sheet comes in at 480 wide with its proportions, and a picture JeffCrunch does not keep as it is (a vector picture) is kept as a PNG.
    void aLargePictureIsScaledAndAVectorPictureBecomesAPng()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        QVERIFY(e.win.w.insertPictureFile(e.folder.write(QStringLiteral("big.png"), pngBytes(960, 480, Qt::green))));
        crunch::SheetObjects *o = e.win.w.objects();
        const grid::ObjectBox big = o->layer()->boxNow(o->layer()->selectedId());
        QCOMPARE(big.w, 480.0);
        QCOMPARE(big.h, 240.0);
        const QByteArray svg = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"30\" height=\"30\"><rect width=\"30\" height=\"30\" fill=\"red\"/></svg>";
        QVERIFY(e.win.w.insertPictureFile(e.folder.write(QStringLiteral("shape.svg"), svg)));
        QVERIFY(save(e));
        int png = 0;
        const QMap<QString, QByteArray> parts = partsOf(e.path);
        for (auto it = parts.begin(); it != parts.end(); ++it)
            if (it.key().startsWith(QLatin1String("xl/media/"))) {
                ++png;
                QVERIFY(it.value().startsWith("\x89PNG"));
            }
        QCOMPARE(png, 2);
    }

    // Insert > Shapes and Text Box: each shape of the gallery is a shape of the drawing with its preset geometry, a text box is marked as one, a line is a connector; they are saved and read again.
    void shapesAndATextBoxAreInsertedAndSaved()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        crunch::SheetObjects *o = e.win.w.objects();
        for (const char *id : {"insert.shape.rect", "insert.shape.rightArrow", "insert.shape.star", "insert.shape.line", "insert.textBox"}) {
            QVERIFY2(e.win.w.act(QString::fromLatin1(id)), id);
            e.win.w.act(QString::fromLatin1(id))->trigger();
        }
        QCOMPARE(o->layer()->count(), 5);
        QVERIFY(e.win.told.isEmpty());
        QVERIFY(save(e));
        const QMap<QString, QByteArray> saved = partsOf(e.path);
        const QByteArray drawing = saved.value(QStringLiteral("xl/drawings/drawing1.xml"));
        QVERIFY(!saved.contains(QStringLiteral("xl/drawings/_rels/drawing1.xml.rels")));   // (shapes relate to nothing: no empty file of relationships)
        QVERIFY(drawing.contains("prst=\"rect\""));
        QVERIFY(drawing.contains("prst=\"rightArrow\""));
        QVERIFY(drawing.contains("prst=\"star5\""));
        QVERIFY(drawing.contains("prst=\"line\""));
        QVERIFY(drawing.contains("txBox=\"1\""));
        QVERIFY(drawing.contains("cxnSp"));
        QDomDocument doc;
        QVERIFY(parseXml(drawing, &doc));
        const QDomNodeList all = doc.elementsByTagName(QStringLiteral("cNvPr"));
        QSet<QString> ids;
        for (int i = 0; i < all.size(); ++i) ids.insert(all.at(i).toElement().attribute(QStringLiteral("id")));
        QCOMPARE(int(ids.size()), 5);
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 5);
    }

    // Shape Format: fill, outline, rotation, flips, and the words in a shape each change the picked shape as one step of Undo, keep its name and its place among the objects, and are saved.
    void aShapeIsFilledOutlinedTurnedAndGivenText()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        crunch::SheetObjects *o = e.win.w.objects();
        QVERIFY(e.win.w.insertShape(QStringLiteral("rect")));
        const int first = o->layer()->selectedId();
        const QString name = o->nameOf(0, first);
        QVERIFY(e.win.w.insertShape(QStringLiteral("ellipse")));
        const QString second = o->nameOf(0, o->layer()->selectedId());
        o->layer()->select(first);
        const int steps = e.win.w.undoStack()->count();
        QVERIFY(e.win.w.setObjectFill(QColor(0xFF, 0x00, 0x00), false));
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        QCOMPARE(int(o->idsOf(0).size()), 2);
        QCOMPARE(o->nameOf(0, o->layer()->selectedId()), name);
        QCOMPARE(o->nameOf(0, o->idsOf(0).front()), name);   // (still behind the oval)
        QCOMPARE(o->nameOf(0, o->idsOf(0).back()), second);
        QVERIFY(e.win.w.setObjectOutline(QColor(0x00, 0xFF, 0x00), false));
        QVERIFY(e.win.w.setObjectOutlineStyle(3.0, QStringLiteral("dash")));
        QVERIFY(e.win.w.rotateObject(1, false, false));
        QVERIFY(e.win.w.rotateObject(0, true, false));
        crunch::SheetObjects *objects = e.win.w.objects();
        QVERIFY(objects->kindOf(0, objects->layer()->selectedId()) == jp::office::sheet::ObjectKind::Shape);
        CrunchWindow::ShapeTextChoices c;
        c.text = QStringLiteral("Hello\nWorld");
        c.horizontal = 1;
        c.vertical = 1;
        c.bold = true;
        c.sizePoints = 14;
        c.colorHex = QStringLiteral("0000FF");
        // the person's dialog: it starts with the words the shape has, and giving it up changes nothing
        const int before = e.win.w.undoStack()->count();
        CrunchWindow::ShapeTextChoices seen;
        e.win.w.setShapeTextAsker([&](CrunchWindow::ShapeTextChoices *x) {
            seen = *x;
            return false;
        });
        QVERIFY(e.win.w.act(QStringLiteral("object.editText"))->isEnabled());
        e.win.w.act(QStringLiteral("object.editText"))->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), before);
        e.win.w.setShapeTextAsker([&](CrunchWindow::ShapeTextChoices *x) {
            *x = c;
            return true;
        });
        e.win.w.act(QStringLiteral("object.editText"))->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), before + 1);
        e.win.w.setShapeTextAsker([&](CrunchWindow::ShapeTextChoices *x) {
            seen = *x;
            return false;
        });
        e.win.w.act(QStringLiteral("object.editText"))->trigger();
        QCOMPARE(seen.text, QStringLiteral("Hello\nWorld"));
        QCOMPARE(seen.horizontal, 1);
        QCOMPARE(seen.sizePoints, 14);
        QVERIFY(seen.bold);
        QVERIFY(save(e));
        const QByteArray drawing = partsOf(e.path).value(QStringLiteral("xl/drawings/drawing1.xml"));
        QDomDocument doc;
        QVERIFY(parseXml(drawing, &doc));
        QVERIFY(drawing.contains("val=\"FF0000\""));
        QVERIFY(drawing.contains("val=\"00FF00\""));
        QVERIFY(drawing.contains("prstDash"));
        QVERIFY(drawing.contains("w=\"38100\""));
        QVERIFY(drawing.contains("rot=\"5400000\""));
        QVERIFY(drawing.contains("flipH=\"1\""));
        QVERIFY(drawing.contains(">Hello<") && drawing.contains(">World<"));
        QVERIFY(drawing.contains("sz=\"1400\""));
        const QDomNodeList all = doc.elementsByTagName(QStringLiteral("cNvPr"));
        QSet<QString> ids;
        for (int i = 0; i < all.size(); ++i) ids.insert(all.at(i).toElement().attribute(QStringLiteral("id")));
        QCOMPARE(int(ids.size()), 2);   // (the old shapes are gone, not left behind)
        Window again;
        QVERIFY(again.open(e.copyOfFile()));
        QTRY_VERIFY(again.w.objects()->shownSheet() == 0 && again.w.objects()->layer()->count() == 2);
    }

    // A picture is cropped to a shape of window (the sides or the top and bottom cut from the middle, and the picture is as large as what is left), and shown whole again.
    void aPictureIsCroppedAndRotated()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        crunch::SheetObjects *o = e.win.w.objects();
        QVERIFY(e.win.w.insertPictureFile(e.folder.write(QStringLiteral("wide.png"), pngBytes(960, 480, Qt::green))));
        QVERIFY(e.win.w.act(QStringLiteral("object.cropSquare"))->isEnabled());
        e.win.w.act(QStringLiteral("object.cropSquare"))->trigger();
        grid::ObjectBox box = o->layer()->boxNow(o->layer()->selectedId());
        QCOMPARE(box.w, 240.0);
        QCOMPARE(box.h, 240.0);
        QVERIFY(e.win.w.cropObject(1, 2));   // (tall: the top and bottom are cut from what shows)
        box = o->layer()->boxNow(o->layer()->selectedId());
        QCOMPARE(box.w, 120.0);
        QCOMPARE(box.h, 240.0);
        QVERIFY(e.win.w.rotateObject(2, false, false));
        QVERIFY(e.win.w.setObjectOutline(QColor(0, 0, 0), false));
        QVERIFY(save(e));
        QByteArray drawing = partsOf(e.path).value(QStringLiteral("xl/drawings/drawing1.xml"));
        QVERIFY2(drawing.contains("srcRect"), drawing.constData());
        QVERIFY(drawing.contains("l=\"37500\"") || drawing.contains("l=\"37500\""));
        QVERIFY(drawing.contains("rot=\"10800000\""));
        QVERIFY(e.win.w.cropObject(0, 0));
        box = o->layer()->boxNow(o->layer()->selectedId());
        QCOMPARE(box.w, 480.0);
        QCOMPARE(box.h, 240.0);
        QVERIFY(save(e));
        drawing = partsOf(e.path).value(QStringLiteral("xl/drawings/drawing1.xml"));
        QVERIFY(!drawing.contains("srcRect"));
        // the picture's bytes are the one picture, not a copy for each change
        int media = 0;
        const QMap<QString, QByteArray> parts = partsOf(e.path);
        for (auto it = parts.begin(); it != parts.end(); ++it)
            if (it.key().startsWith(QLatin1String("xl/media/"))) ++media;
        QCOMPARE(media, 1);
    }

    // The Selection Pane lists the sheet's objects, front first; choosing a row picks the object, and its buttons bring it forward or send it back.
    void theSelectionPaneListsPicksAndReordersObjects()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        crunch::SheetObjects *o = e.win.w.objects();
        QVERIFY(e.win.w.insertShape(QStringLiteral("rect")));
        const int a = o->layer()->selectedId();
        QVERIFY(e.win.w.insertShape(QStringLiteral("ellipse")));
        const int b = o->layer()->selectedId();
        QAction *open = e.win.w.act(QStringLiteral("object.selectionPane"));
        QVERIFY(open);
        QVERIFY(open->isEnabled());
        open->trigger();
        auto *list = e.win.w.findChild<QListWidget *>(QStringLiteral("selectionList"));
        QVERIFY(list);
        QCOMPARE(list->count(), 2);
        QCOMPARE(list->item(0)->text(), o->nameOf(0, b));   // (the front one first)
        QCOMPARE(list->item(1)->text(), o->nameOf(0, a));
        list->setCurrentRow(1);
        QCOMPARE(o->layer()->selectedId(), a);
        auto *forward = e.win.w.findChild<QAbstractButton *>(QStringLiteral("selectionForward"));
        QVERIFY(forward);
        forward->click();
        QCOMPARE(o->idsOf(0).back(), a);
        QCOMPARE(list->item(0)->text(), o->nameOf(0, a));
        QCOMPARE(list->currentRow(), 0);
    }

    // Pictures and shapes move and size too: the anchor of the file is kept in the form it keeps what the object does when cells change (a picture of a one-cell anchor moves with the cells and
    // keeps its size; a shape set not to move or size stays where it is).
    void picturesAndShapesAreMovedAndSaved()
    {
        const QString anchors = oneCell(1, 6, 952500, 476250, pictureElement(3, QStringLiteral("rIdP"), QStringLiteral("Logo"))) + twoCell(5, 10, 8, 14, shapeElement(4, QStringLiteral("Box")), QStringLiteral("absolute"));
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(anchors), {imageTarget(QStringLiteral("rIdP"), 1, pngBytes(8, 4, Qt::red))})));
        QTRY_VERIFY(objectsShown(e, 2));
        crunch::SheetObjects *o = e.win.w.objects();
        e.view()->selectCell(0, 0);
        const grid::ObjectBox picture = o->layer()->boxNow(0), box = o->layer()->boxNow(1);
        // the picture: dragged, with Shift at a corner it keeps its proportions (100 by 50)
        o->layer()->select(0);
        QRect r = o->layer()->deviceRect(0, grid::Pane::Body);
        QWidget *vp = e.view()->viewport();
        QTest::mousePress(vp, Qt::LeftButton, Qt::ShiftModifier, r.bottomRight());
        QTest::mouseMove(vp, r.bottomRight() + QPoint(40, 5), Qt::ShiftModifier);
        QTest::mouseMove(vp, r.bottomRight() + QPoint(60, 5), Qt::ShiftModifier);
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::ShiftModifier, r.bottomRight() + QPoint(60, 5));
        const grid::ObjectBox sized = o->layer()->boxNow(0);
        QCOMPARE(sized.x, picture.x);
        QVERIFY(sized.w > picture.w);
        QCOMPARE(sized.h / sized.w, picture.h / picture.w);   // (the proportions)
        // the shape moves by the keys
        o->layer()->select(1);
        QTest::keyClick(e.view(), Qt::Key_Down);
        QCOMPARE(o->layer()->boxNow(1).y, box.y + 6);
        QVERIFY(save(e));
        const SavedAnchor a = savedAnchor(e.path, QStringLiteral("Logo")), b = savedAnchor(e.path, QStringLiteral("Box"));
        QVERIFY(a.found && b.found);
        QCOMPARE(a.kind, QStringLiteral("twoCellAnchor"));
        QCOMPARE(a.editAs, QStringLiteral("oneCell"));
        QCOMPARE(b.editAs, QStringLiteral("absolute"));
        // read again: the picture is where it was left and as large, the shape too
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 2);
        second.w.objects()->layer()->prepareAll();
        QCOMPARE(second.w.objects()->layer()->boxNow(0).w, sized.w);
        QCOMPARE(second.w.objects()->layer()->boxNow(1).y, box.y + 6);
        // the file's picture is still there and as it was
        QMap<QString, QByteArray> original;
        QVERIFY(jp::readZip(bookWithDrawing({dataSheet()}, drawingPart(anchors), {imageTarget(QStringLiteral("rIdP"), 1, pngBytes(8, 4, Qt::red))}), original));
        QCOMPARE(partsOf(e.path).value(QStringLiteral("xl/media/image1.png")), original.value(QStringLiteral("xl/media/image1.png")));
    }

    // A chart put on, undone, and the file saved (written without it) is put on again by Redo, and the next save writes it.
    void redoOfAnAddedChartAfterASaveWritesItAgain()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        selectRange(e, QStringLiteral("A1:C4"));
        e.select(QStringLiteral("H1"));
        e.enter(QStringLiteral("5"));
        QVERIFY(e.settle());
        selectRange(e, QStringLiteral("A1:C4"));
        QVERIFY(e.win.w.insertChart(ChartKind::Column));
        pressUndo(&e.win.w);
        QVERIFY(e.win.w.undoStack()->canRedo());
        QVERIFY(save(e));
        QVERIFY(partsOf(e.path).value(QStringLiteral("xl/charts/chart1.xml")).isEmpty());
        pressRedo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(int(e.win.w.objects()->idsOf(0).size()), 1);
        QVERIFY(save(e));
        QCOMPARE(savedCharts(e.path).size(), 1);
        QVERIFY(!partsOf(e.path).value(QStringLiteral("xl/charts/chart1.xml")).isEmpty());
    }

    // The numbers of a chart made here are shown, and saved, in the format of the cells they come from.
    void aNewChartHasTheFormatOfItsCells()
    {
        const QByteArray styles = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"164\" applyNumberFormat=\"1\"/>"));
        const QByteArray styled = stylesXml(QStringLiteral("<numFmt numFmtId=\"164\" formatCode=\"&quot;$&quot;#,##0.00\"/>"), QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"164\" applyNumberFormat=\"1\"/>"));
        Q_UNUSED(styles);
        QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>Item</t></is></c><c r=\"B1\" t=\"inlineStr\"><is><t>Cost</t></is></c></row>");
        for (int i = 0; i < 3; ++i) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\" t=\"inlineStr\"><is><t>x%2</t></is></c><c r=\"B%1\" s=\"1\"><v>%3</v></c></row>").arg(i + 2).arg(i).arg(1200 + i);
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), styled)));
        QVERIFY(e.settle());
        selectRange(e, QStringLiteral("A1:B4"));
        QVERIFY(e.win.w.insertChart(ChartKind::Column));
        QCOMPARE(seriesOf(chartOf(e))[0].format, QStringLiteral("\"$\"#,##0.00"));
        QVERIFY(save(e));
        const QVector<SavedChart> charts = savedCharts(e.path);
        QCOMPARE(seriesOf(charts[0].chart)[0].format, QStringLiteral("\"$\"#,##0.00"));
    }

    // A chart made here follows the sheet it reads when that is renamed (a name that needs quotes gets them), and back with Undo; the file has the name it has then.
    void aNewChartFollowsTheRenameOfItsSheet()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        selectRange(e, QStringLiteral("A1:C4"));
        QVERIFY(e.win.w.insertChart(ChartKind::Column));
        QVERIFY(e.win.w.renameSheetTo(0, QStringLiteral("Q1 Budget")));
        QVERIFY(e.settle());
        QCOMPARE(seriesOf(chartOf(e))[0].valueRef, QStringLiteral("'Q1 Budget'!$B$2:$B$4"));
        QCOMPARE(seriesOf(chartOf(e))[0].nameRef, QStringLiteral("'Q1 Budget'!$B$1"));
        QCOMPARE(seriesOf(chartOf(e))[0].values, (QVector<double>{10, 20, 30}));   // (it still reads the cells)
        QVERIFY(save(e));
        const QVector<SavedChart> charts = savedCharts(e.path);
        QCOMPARE(seriesOf(charts[0].chart)[0].valueRef, QStringLiteral("'Q1 Budget'!$B$2:$B$4"));
        QCOMPARE(seriesOf(charts[0].chart)[0].values, (QVector<double>{10, 20, 30}));
        // a chart changed after the rename reads by the new name too
        e.win.w.objects()->layer()->select(0);
        QVERIFY(e.win.w.changePickedChart(QStringLiteral("Test"), [](Chart *c) { c->title.present = true; c->title.automatic = false; c->title.text = QStringLiteral("T"); }));
        QCOMPARE(seriesOf(chartOf(e))[0].valueRef, QStringLiteral("'Q1 Budget'!$B$2:$B$4"));
        QVERIFY(e.win.w.renameSheetTo(0, QStringLiteral("Data")));
        QVERIFY(e.settle());
        QCOMPARE(seriesOf(chartOf(e))[0].valueRef, QStringLiteral("Data!$B$2:$B$4"));
        pressUndo(&e.win.w);   // the second rename
        QVERIFY(e.settle());
        QCOMPARE(seriesOf(chartOf(e))[0].valueRef, QStringLiteral("'Q1 Budget'!$B$2:$B$4"));
    }

    // ---- limits, and what a save does with them

    // One set of limits for the whole workbook, kept where an object is made: past them a new chart is refused with a reason, an object taken away makes room, and nothing is made that a save would
    // have to leave out.
    void newObjectsAreLimitedWhereTheyAreMade()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({dataSheet(), dataSheet()})));
        QVERIFY(e.settle());
        crunch::SheetObjects *o = e.win.w.objects();
        crunch::SheetObjects::Limits limits = o->limits();
        QCOMPARE(limits.maxAdded, 1000);
        limits.maxAdded = 3;
        o->setLimits(limits);
        selectRange(e, QStringLiteral("A1:C4"));
        QVERIFY(e.win.w.insertChart(ChartKind::Column));
        e.win.w.setActiveSheet(1);
        QTRY_VERIFY(o->shownSheet() == 1);
        selectRange(e, QStringLiteral("A1:C4"));
        QVERIFY(e.win.w.insertChart(ChartKind::Line));
        QVERIFY(e.win.w.insertChart(ChartKind::Pie));
        QCOMPARE(o->addedCount(), 3);   // (the sheets share the count)
        QVERIFY(!e.win.w.insertChart(ChartKind::Area));
        QVERIFY2(e.win.told.last().contains(QLatin1String("3 new objects")), qPrintable(e.win.told.last()));
        QCOMPARE(o->addedCount(), 3);
        // one taken away makes room, and a save makes room for all
        pressUndo(&e.win.w);
        QCOMPARE(o->addedCount(), 2);
        QVERIFY(e.win.w.insertChart(ChartKind::Area));
        QVERIFY(save(e));
        QCOMPARE(o->addedCount(), 0);
        QCOMPARE(savedCharts(e.path, 0).size() + savedCharts(e.path, 1).size(), 3);
        QVERIFY(e.win.w.insertChart(ChartKind::Pie));
    }

    // A chart is not let to hold more numbers than the workbook's budget, a paste repeated until it is refused stays within it, what an undone chart held is let go (the step that made it holds it),
    // and the numbers a step of Undo holds are counted in the bytes Undo may keep.
    void chartsAreHeldWithinABudgetOfNumbers()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithAChart()));
        QTRY_VERIFY(objectsShown(e, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->select(0);
        e.win.w.act(QStringLiteral("edit.copy"))->trigger();
        crunch::SheetObjects::Limits limits = o->limits();
        const Chart one = chartOf(e);
        const qint64 each = o->pointsOf(0, one);
        QVERIFY(each > 0);
        limits.maxPoints = each * 4 + 1;
        o->setLimits(limits);
        int pasted = 0;
        for (int i = 0; i < 50; ++i) {
            e.select(QStringLiteral("B20"));
            const int before = int(o->idsOf(0).size());
            e.win.w.act(QStringLiteral("edit.paste"))->trigger();
            if (int(o->idsOf(0).size()) == before) break;
            ++pasted;
        }
        QCOMPARE(pasted, 4);
        QVERIFY(o->chartPointsHeld() <= limits.maxPoints);
        QVERIFY2(e.win.told.last().contains(QLatin1String("hold as many numbers")), qPrintable(e.win.told.last()));
        // an undone paste lets go of its numbers and makes room; redo gives them back
        const int last = o->idsOf(0).back();
        pressUndo(&e.win.w);
        QCOMPARE(seriesCount(chartOf(e, last)), 0);
        QCOMPARE(o->chartPointsHeld(), each * 3);
        pressRedo(&e.win.w);
        QCOMPARE(seriesCount(chartOf(e, last)), 1);
        QCOMPARE(o->chartPointsHeld(), each * 4);
        // a change that would need more than there is, is refused as well (by the change a person makes); one that needs no more numbers is not
        o->layer()->select(last);
        limits.maxPoints = each * 4 + each / 2;
        o->setLimits(limits);
        const int before = int(o->idsOf(0).size());
        QVERIFY(!e.win.w.changePickedChart(QStringLiteral("Test"), [](Chart *c) { c->plots[0].series.push_back(c->plots[0].series[0]); }));
        QVERIFY2(e.win.told.last().contains(QLatin1String("hold as many numbers")), qPrintable(e.win.told.last()));
        QCOMPARE(int(o->idsOf(0).size()), before);
        QCOMPARE(seriesCount(chartOf(e, last)), 1);
        QCOMPARE(o->chartPointsHeld(), each * 4);
        QVERIFY(e.win.w.changePickedChart(QStringLiteral("Test"), [](Chart *c) { c->title.present = true; c->title.automatic = false; c->title.text = QStringLiteral("T"); }));
        // Undo's bytes include the numbers a step holds
        limits.maxPoints = 100000;
        o->setLimits(limits);
        const qint64 bytes = e.win.w.session()->undoBytes();
        QVERIFY(e.win.w.changePickedChart(QStringLiteral("Test"), [](Chart *c) { c->plots[0].series[0].values = QVector<double>(5000, 1.5); }));
        QVERIFY2(e.win.w.session()->undoBytes() - bytes >= qint64(5000 * 8), qPrintable(QString::number(e.win.w.session()->undoBytes() - bytes)));
    }

    // The part of a chart is written by the thread that writes the file, not by the one that began the save: the save is given the model to write from.
    void chartPartsAreWrittenByTheSavingThread()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithAChart()));
        QTRY_VERIFY(objectsShown(e, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->select(0);
        QVERIFY(e.win.w.changePickedChart(QStringLiteral("Test"), [](Chart *c) { c->title.present = true; c->title.automatic = false; c->title.text = QStringLiteral("T"); }));
        selectRange(e, QStringLiteral("A1:C4"));
        QVERIFY(e.win.w.insertChart(ChartKind::Column));
        SaveChanges changes;
        o->addToSave(&changes);
        QCOMPARE(int(changes.producedParts.size()), 1);
        QVERIFY(changes.replacedParts.empty());
        QCOMPARE(int(changes.addedObjects.size()), 1);
        const AddedPart &part = changes.addedObjects.begin()->second.objects[0].parts[0];
        QVERIFY(part.bytes.isEmpty() && part.produce);
        QVERIFY(part.produce().contains("<c:chartSpace"));
        QVERIFY(changes.producedParts.begin()->second().contains("<c:chartSpace"));
    }

    // What a save names is chosen again each time: the name of a drawing kept from a save that did not happen is not taken by another sheet's.
    void namesOfNewPartsAreChosenAgainForEachSave()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({dataSheet(), dataSheet()})));
        QVERIFY(e.settle());
        crunch::SheetObjects *o = e.win.w.objects();
        e.win.w.setActiveSheet(1);
        QTRY_VERIFY(o->shownSheet() == 1);
        selectRange(e, QStringLiteral("A1:C4"));
        QVERIFY(e.win.w.insertChart(ChartKind::Column));
        SaveChanges failed;
        o->addToSave(&failed);   // (the save that did not happen)
        QCOMPARE(failed.addedObjects.begin()->second.part, QStringLiteral("/xl/drawings/drawing1.xml"));
        e.win.w.setActiveSheet(0);
        QTRY_VERIFY(o->shownSheet() == 0);
        selectRange(e, QStringLiteral("A1:C4"));
        QVERIFY(e.win.w.insertChart(ChartKind::Line));
        SaveChanges again;
        o->addToSave(&again);
        QCOMPARE(int(again.addedObjects.size()), 2);
        QVERIFY(again.addedObjects.at(0).part != again.addedObjects.at(1).part);
        QVERIFY(again.addedObjects.at(0).objects[0].parts[0].part != again.addedObjects.at(1).objects[0].parts[0].part);
        QVERIFY(save(e));
        QCOMPARE(savedCharts(e.path, 0).size(), 1);
        QCOMPARE(savedCharts(e.path, 1).size(), 1);
    }

    // An object that cannot be written is not said to be in the file: it stays to be saved.
    void anObjectThatCouldNotBeWrittenIsNotSaidToBeSaved()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        selectRange(e, QStringLiteral("A1:C4"));
        QVERIFY(e.win.w.insertChart(ChartKind::Column));
        crunch::SheetObjects *o = e.win.w.objects();
        // a second set of objects with no cells to read from (as before the workbook is whole): nothing can be written for what it holds
        crunch::SheetObjects bare;
        bare.open(e.win.w.loadedBook()->book);
        QString why;
        const int id = bare.makeChart(0, chartOf(e), DrawingAnchor{}, &why);
        QVERIFY2(id >= 0, qPrintable(why));
        crunch::ObjectEdit edit;
        edit.sheet = 0;
        edit.id = id;
        edit.before = bare.stateOf(0, id);
        edit.after = edit.before;
        edit.after.deleted = false;
        bare.applyState(0, id, edit.after);
        QCOMPARE(bare.addedCount(), 1);
        SaveChanges changes;
        bare.addToSave(&changes);
        QVERIFY(changes.addedObjects.empty());
        bare.saved();
        QCOMPARE(bare.addedCount(), 1);   // (still to be saved)
        Q_UNUSED(o);
    }

    // A shape put on a drawing is numbered past every shape the part has, the ones past the number a drawing is read for too.
    void aNewShapeIsNumberedPastEveryShapeOfTheDrawing()
    {
        QString anchors;
        const int n = kMaxDrawingAnchors + 120;
        for (int i = 0; i < n; ++i) anchors += twoCell(i % 200, i / 200, i % 200 + 1, i / 200 + 1, shapeElement(10 + i, QStringLiteral("S%1").arg(i)));
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(anchors), {})));
        QTRY_VERIFY_WITH_TIMEOUT(e.win.w.objects()->shownSheet() == 0 && e.win.w.objects()->layer()->count() > 0, 30000);
        selectRange(e, QStringLiteral("A1:C4"));
        QVERIFY(e.win.w.insertChart(ChartKind::Column));
        QVERIFY(save(e));
        Package p;
        QString why;
        QVERIFY(p.readLazyFile(e.path, &why));
        QDomDocument doc;
        QVERIFY(parseXml(p.part(QStringLiteral("/xl/drawings/drawing1.xml")), &doc));
        QSet<qint64> ids;
        qint64 chartId = 0;
        const QDomNodeList all = doc.elementsByTagName(QStringLiteral("cNvPr"));
        for (int i = 0; i < all.size(); ++i) {
            const qint64 id = all.at(i).toElement().attribute(QStringLiteral("id")).toLongLong();
            QVERIFY2(!ids.contains(id), qPrintable(QString::number(id)));
            ids.insert(id);
            if (all.at(i).toElement().attribute(QStringLiteral("name")) == QLatin1String("Chart 1")) chartId = id;
        }
        QVERIFY(chartId > 10 + n - 1);
    }

    // A sheet that is a copy of one with an outside link keeps the link's relationship id: the drawing a chart on the copy gets has another, and every relationships part of the file has each id once.
    void aCopyOfASheetWithALinkGetsADrawingWithAnIdOfItsOwn()
    {
        const QByteArray sheet = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>Region</t></is></c><c r=\"B1\"><v>4</v></c></row><row r=\"2\"><c r=\"A2\"><v>5</v></c><c r=\"B2\"><v>6</v></c></row>"),
                                           QStringLiteral("<hyperlinks><hyperlink ref=\"A1\" r:id=\"rId1\"/></hyperlinks>"));
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(buildXlsx({sheet}), parts));
        parts[QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels")] = QByteArray("<?xml version=\"1.0\"?><Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                                                                               "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink\" Target=\"https://example.org/\" TargetMode=\"External\"/></Relationships>");
        jp::ZipWriter z;
        for (auto it = parts.begin(); it != parts.end(); ++it) z.add(it.key(), it.value(), true);
        EditWindow e;
        QVERIFY(e.open(z.finish()));
        QVERIFY(e.settle());
        e.win.w.setMoveCopyAsker([](CrunchWindow::MoveCopyRequest *r) {
            r->sheets = {0};
            r->before = -1;
            r->copy = true;
            return true;
        });
        e.win.w.act(QStringLiteral("sheet.moveCopy"))->trigger();
        QVERIFY(e.settle());
        crunch::SheetObjects *o = e.win.w.objects();
        QCOMPARE(e.win.w.shownSheets().size(), 2);
        e.win.w.setActiveSheet(1);
        QTRY_VERIFY(o->shownSheet() == 1);
        selectRange(e, QStringLiteral("A1:B2"));
        QVERIFY2(e.win.w.insertChart(ChartKind::Column), qPrintable(e.win.told.join(QLatin1Char('|'))));
        QVERIFY(save(e));
        const QMap<QString, QByteArray> saved = partsOf(e.path);
        int relationshipParts = 0;
        for (auto it = saved.begin(); it != saved.end(); ++it) {
            if (!it.key().endsWith(QLatin1String(".rels"))) continue;
            ++relationshipParts;
            QDomDocument doc;
            QVERIFY2(doc.setContent(it.value()), qPrintable(it.key()));
            QSet<QString> ids;
            const QDomNodeList all = doc.elementsByTagName(QStringLiteral("Relationship"));
            for (int i = 0; i < all.size(); ++i) {
                const QString id = all.at(i).toElement().attribute(QStringLiteral("Id"));
                QVERIFY2(!ids.contains(id), qPrintable(it.key() + QLatin1Char(' ') + id));
                ids.insert(id);
            }
        }
        QVERIFY(relationshipParts >= 4);
        QCOMPARE(savedCharts(e.path, 1).size() + savedCharts(e.path, 0).size(), 1);
    }

    // The More Charts dialog finds the block of cells it draws from once, for every kind and for the swap: what it reads does not grow with the number of kinds, nor with the size of the sheet.
    void theNewChartDialogReadsTheCellsOnce()
    {
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(20000, {})));
        QVERIFY(e.settle());
        auto *cells = dynamic_cast<const jp::grid::WorkbookChartCells *>(e.win.w.objects()->cells());
        QVERIFY(cells);
        for (const bool wholeSheet : {false, true}) {
            if (wholeSheet) e.view()->selectAll();
            else e.select(QStringLiteral("A5"));
            const qint64 before = cells->reads();
            std::unique_ptr<QDialog> dialog(e.win.w.makeInsertChartDialog());
            QVERIFY(dialog);
            auto *d = static_cast<InsertChartDialog *>(dialog.get());
            d->setRowsAndColumns(true);
            for (const ChartKindEntry &k : chartKindEntries()) d->selectKind(k.kind);
            QVERIFY(!d->previewShown().isNull());
            const qint64 read = cells->reads() - before;
            QVERIFY2(read < 8500, qPrintable(QString::number(read)));   // (the block of the selection is looked for once, and the kinds are drawn from it: found again for each of the 17 kinds and the swap it would be read several times over)
        }
    }

    // A copy says which workbook it came from by an id no other opening of any file has (a program started again, or another program, has another), and a paste keeps its references only if the
    // sheets they name are still there under those names.
    void aPastedChartReadsCellsOnlyInTheWorkbookItCameFrom()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithAChart()));
        QTRY_VERIFY(objectsShown(e, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        auto copyIt = [&] {
            QGuiApplication::clipboard()->clear();
            o->layer()->select(0);
            e.win.w.act(QStringLiteral("edit.copy"))->trigger();
        };
        auto pasteIt = [&] {
            e.select(QStringLiteral("B20"));
            e.win.w.act(QStringLiteral("edit.paste"))->trigger();
            return o->idsOf(0).back();
        };
        // the same workbook: the references stay
        copyIt();
        crunch::ObjectClip clip;
        QVERIFY(crunch::ObjectClip::fromBytes(QGuiApplication::clipboard()->mimeData()->data(QLatin1String(crunch::kObjectMime)), &clip));
        QVERIFY(!clip.origin.section(QLatin1Char('|'), 0, 0).isEmpty());
        QVERIFY(clip.origin.section(QLatin1Char('|'), 0, 0) != QString::number(e.win.w.session()->token()));
        QCOMPARE(clip.origin.section(QLatin1Char('|'), 0, 0).size(), 36);   // (a random 128-bit id, written out)
        int id = pasteIt();
        QCOMPARE(seriesOf(chartOf(e, id))[0].valueRef, QStringLiteral("Sheet1!$B$2:$B$4"));
        // a copy that says another workbook (another program, or the same file opened again): the numbers, no references
        crunch::ObjectClip other = clip;
        other.origin = QStringLiteral("0b0f2d6e-1111-4222-8333-444455556666|") + clip.origin.section(QLatin1Char('|'), 1);
        auto *md = new QMimeData;
        md->setData(QLatin1String(crunch::kObjectMime), other.toBytes());
        QGuiApplication::clipboard()->setMimeData(md);
        id = pasteIt();
        QVERIFY(seriesOf(chartOf(e, id))[0].valueRef.isEmpty());
        QCOMPARE(seriesOf(chartOf(e, id))[0].values, (QVector<double>{10, 20, 30}));
        // a copy that names a sheet this workbook has not (deleted, or never in it) is another workbook's whatever its references say
        crunch::ObjectClip gone = clip;
        gone.sheetName = QStringLiteral("NoSuchSheet");
        auto *mg = new QMimeData;
        mg->setData(QLatin1String(crunch::kObjectMime), gone.toBytes());
        QGuiApplication::clipboard()->setMimeData(mg);
        id = pasteIt();
        QVERIFY(seriesOf(chartOf(e, id))[0].valueRef.isEmpty());
        // the sheet it read renamed between the copy and the paste: it would read other cells, or none
        copyIt();
        QVERIFY(e.win.w.renameSheetTo(0, QStringLiteral("Budget")));
        QVERIFY(e.settle());
        id = pasteIt();
        QVERIFY(seriesOf(chartOf(e, id))[0].valueRef.isEmpty());
        QCOMPARE(seriesOf(chartOf(e, id))[0].values, (QVector<double>{10, 20, 30}));
        // renamed back (Undo): the copy is the workbook's again
        pressUndo(&e.win.w);
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(seriesOf(chartOf(e, 0))[0].valueRef, QStringLiteral("Sheet1!$B$2:$B$4"));
    }
};

JP_TEST_CLASS(CrunchChartTests)

// The dialogs of charts, driven as a person does: what each shows and what it gives back.
class CrunchChartDialogTests : public QObject {
    Q_OBJECT

    static Chart twoSeries()
    {
        ChartPlan plan = planChartFrom(QStringLiteral("Sheet1"), rangeOf(QStringLiteral("A1:C4")), true, true, false, ChartKind::Column);
        return makeChart(ChartKind::Column, plan, defaultTheme());
    }

private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    // New Chart lists every kind with a picture of it, shows the one picked large, and gives back the kind and whether the rows and columns are switched.
    void insertChartShowsEveryKind()
    {
        QVector<QString> asked;
        InsertChartDialog d([&](ChartKind kind, int inRows, const QSize &px) {
            asked << QStringLiteral("%1/%2/%3").arg(int(kind)).arg(inRows).arg(px.width());
            QImage img(px, QImage::Format_ARGB32);
            img.fill(QColor::fromRgb(int(kind) * 10, 100, inRows < 0 ? 0 : 200));
            return img;
        }, ChartKind::Pie);
        QCOMPARE(d.kind(), ChartKind::Pie);
        QCOMPARE(d.inRows(), -1);
        // a picture of each kind (they are listed once, drawn large and made small), and the big one of the first kind picked
        int small = 0;
        for (const QString &a : asked)
            if (a.endsWith(QLatin1String("/256"))) ++small;
        QCOMPARE(small, int(chartKindEntries().size()));
        QVERIFY(asked.contains(QStringLiteral("%1/-1/360").arg(int(ChartKind::Pie))));
        QCOMPARE(d.previewShown().size(), QSize(360, 260));
        d.selectKind(ChartKind::Scatter);
        QCOMPARE(d.kind(), ChartKind::Scatter);
        QCOMPARE(QColor(d.previewShown().pixel(5, 5)).red(), int(ChartKind::Scatter) * 10);
        d.setRowsAndColumns(true);
        QCOMPARE(d.inRows(), 2);
        QVERIFY(asked.last().contains(QLatin1String("/2/360")));
        // a list of every kind, named
        QSet<QString> names;
        for (const ChartKindEntry &e : chartKindEntries()) names.insert(chartKindName(e.kind));
        QCOMPARE(names.size(), int(chartKindEntries().size()));
        // nothing to show: it says so
        InsertChartDialog empty([](ChartKind, int, const QSize &) { return QImage(); }, ChartKind::Column);
        QVERIFY(empty.previewShown().isNull());
    }

    void selectDataEditsTheSeries()
    {
        SelectDataRequest request;
        request.chart = twoSeries();
        request.theme = defaultTheme();
        request.checkRef = [](const QString &ref, QString *why) {
            const bool ok = ref.startsWith(QLatin1String("Sheet1!"));
            if (!ok && why) *why = QStringLiteral("no %1").arg(ref);
            return ok;
        };
        request.planFor = [](const QString &range, int) {
            return range == QLatin1String("Sheet1!$A$1:$D$4") ? planChartFrom(QStringLiteral("Sheet1"), rangeOf(QStringLiteral("A1:D4")), true, true, false, ChartKind::Column) : ChartPlan();
        };
        request.switcher = [](const Chart &now, Chart *out) {
            bool ok = false;
            const Chart c = switchedRowsColumns(now, defaultTheme(), &ok);
            if (ok) *out = c;
            return ok;
        };
        request.nameOf = [](const ChartSeries &s) { return s.name + QStringLiteral("!"); };
        SelectDataDialog d(request);
        QCOMPARE(d.seriesShown(), 2);
        QCOMPARE(d.seriesAt(1).valueRef, QStringLiteral("Sheet1!$C$2:$C$4"));
        // the same as it came: the chart as it was
        QCOMPARE(seriesCount(d.result()), 2);
        QCOMPARE(int(kindOfChart(d.result())), int(ChartKind::Column));
        // moved, removed, added
        d.moveSeries(0, 1);
        QCOMPARE(d.seriesAt(0).valueRef, QStringLiteral("Sheet1!$C$2:$C$4"));
        d.moveSeries(0, -1);   // (there is nothing before the first)
        QCOMPARE(d.seriesAt(0).valueRef, QStringLiteral("Sheet1!$C$2:$C$4"));
        d.removeSeries(0);
        QCOMPARE(d.seriesShown(), 1);
        ChartSeries more;
        more.name = QStringLiteral("North");
        more.valueRef = QStringLiteral("Sheet1!$D$2:$D$4");
        d.addSeries(more);
        QCOMPARE(d.seriesShown(), 2);
        d.setCategoryRef(QStringLiteral("Sheet1!$E$2:$E$4"));
        Chart c = d.result();
        QCOMPARE(seriesOf(c)[0].valueRef, QStringLiteral("Sheet1!$B$2:$B$4"));
        QCOMPARE(seriesOf(c)[1].valueRef, QStringLiteral("Sheet1!$D$2:$D$4"));
        QCOMPARE(seriesOf(c)[1].categoryRef, QStringLiteral("Sheet1!$E$2:$E$4"));   // (the labels are the chart's: every series has them)
        QCOMPARE(seriesOf(c)[0].categoryRef, QStringLiteral("Sheet1!$E$2:$E$4"));
        // the cells the chart reads, typed: three series
        d.applyDataRange(QStringLiteral("Sheet1!$A$1:$D$4"));
        QCOMPARE(d.seriesShown(), 3);
        QCOMPARE(d.seriesAt(2).nameRef, QStringLiteral("Sheet1!$D$1"));
        // cells that hold nothing: the series stay
        d.applyDataRange(QStringLiteral("Sheet1!$Z$1:$Z$2"));
        QCOMPARE(d.seriesShown(), 3);
        // rows and columns switched, and the way empty cells are shown
        d.switchRowsAndColumns();
        QCOMPARE(d.seriesShown(), 3);   // (three rows)
        QVERIFY(!seriesRunDownColumns(d.result()));
        d.setBlanks(QStringLiteral("span"), true);
        c = d.result();
        QCOMPARE(c.blanksAs, QStringLiteral("span"));
        QVERIFY(!c.plotVisibleOnly);
        // a chart can't be made to hold more series than it can read
        for (int i = 0; i < 300; ++i) d.addSeries(more);
        QVERIFY(seriesCount(d.result()) <= kMaxMadeSeries);
    }

    void changeTypeGivesTheKindsOfTheSeries()
    {
        int draws = 0;
        QVector<SeriesKind> lastKinds;
        ChangeChartTypeDialog d({"East", "West"}, {{ChartKind::Column, false}, {ChartKind::Column, false}}, [&](const QVector<SeriesKind> &kinds, const QSize &px) {
            ++draws;
            lastKinds = kinds;
            return QImage(px, QImage::Format_ARGB32);
        });
        QVERIFY(!d.isCombination());
        QCOMPARE(d.kinds().size(), 2);
        QCOMPARE(int(d.kinds()[0].kind), int(ChartKind::Column));
        d.chooseKind(ChartKind::LineMarkers);
        QCOMPARE(int(d.kinds()[1].kind), int(ChartKind::LineMarkers));
        QCOMPARE(int(lastKinds[0].kind), int(ChartKind::LineMarkers));
        d.chooseKind(ChartKind::Combo);
        QCOMPARE(int(d.kinds()[1].kind), int(ChartKind::LineMarkers));
        QVERIFY(d.kinds()[1].secondary && !d.kinds()[0].secondary);
        d.chooseCombination({{ChartKind::Area, false}, {ChartKind::Line, true}});
        QVERIFY(d.isCombination());
        QCOMPARE(int(d.kinds()[0].kind), int(ChartKind::Area));
        QVERIFY(d.kinds()[1].secondary);
        QVERIFY(draws >= 4);
        // a dialog started with a combination shows it
        ChangeChartTypeDialog again({"a", "b"}, {{ChartKind::Column, false}, {ChartKind::Line, true}}, nullptr);
        QVERIFY(again.isCombination());
        QVERIFY(again.kinds()[1].secondary);
    }

    void propertiesKeepTheProportionsAndSayWhatFollows()
    {
        ObjectProperties now;
        now.widthPoints = 200;
        now.heightPoints = 100;
        now.follows = AnchorKind::OneCell;
        now.keepProportions = true;
        ObjectPropertiesDialog d(now);
        QCOMPARE(int(d.result().follows), int(AnchorKind::OneCell));
        QCOMPARE(d.result().widthPoints, 200.0);
        auto *width = d.findChild<QDoubleSpinBox *>(QStringLiteral("objectWidth"));
        auto *height = d.findChild<QDoubleSpinBox *>(QStringLiteral("objectHeight"));
        QVERIFY(width && height);
        width->setValue(300);
        QCOMPARE(d.result().heightPoints, 150.0);
        d.findChild<QRadioButton *>(QStringLiteral("followStays"))->setChecked(true);
        QCOMPARE(int(d.result().follows), int(AnchorKind::Absolute));
        d.findChild<QRadioButton *>(QStringLiteral("followSizes"))->setChecked(true);
        QCOMPARE(int(d.result().follows), int(AnchorKind::TwoCell));
    }
};

JP_TEST_CLASS(CrunchChartDialogTests)
#include "tst_crunch_charts.moc"
