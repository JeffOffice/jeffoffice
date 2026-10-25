// Tests of making and changing charts in JeffCrunch's window: New Chart from the cells selected, moving and sizing objects, the chart's elements, Chart Data, Chart Kind, Copy and Paste of a
// chart, and what each saves (the chart part, the drawing, and what names them), read back by the readers of this program. Every workbook is built here.
#include "../sheet/drawinghelpers.h"
#include "editwindow.h"
#include "savedok.h"

#include "app/chartdialogs.h"
#include "app/ribbon.h"
#include "core/fonts.h"
#include "core/objectclip.h"
#include "core/sheetobjects.h"
#include "office/chart_write.h"
#include "office/xml.h"
#include "sheet/chartedit.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QDoubleSpinBox>
#include <QMenu>
#include <QRadioButton>
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
        part.replace("<c:cat>", "<c:trendline><c:trendlineType val=\"linear\"/></c:trendline><c:cat>");
        EditWindow e;
        QVERIFY(e.open(bookWithAChart(QString(), part)));
        QTRY_VERIFY(objectsShown(e, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->select(0);
        QString why;
        QVERIFY(!o->canEditChart(0, 0, &why));
        QVERIFY2(why.contains(QLatin1String("trendline")), qPrintable(why));
        QVERIFY(!e.win.w.act(QStringLiteral("chartdesign.selectData"))->isEnabled());
        QTRY_VERIFY(!e.win.w.chartElementsButton() || !e.win.w.chartElementsButton()->isVisible());
        QVERIFY(!e.win.w.changePickedChart(QStringLiteral("Test"), [](Chart *c) { c->title.present = true; }));
        QVERIFY(e.win.told.last().contains(QLatin1String("trendline")));
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

    // Pictures and shapes move and size too: the anchor of the file is kept in the form it keeps what the object does when cells change (a picture of a one-cell anchor moves with the cells and
    // keeps its size; a shape set not to move or size stays where it is), and Copy on one says what can be copied.
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
        // Copy on them says what can be copied and leaves the cells' clipboard alone
        QGuiApplication::clipboard()->clear();
        e.win.w.act(QStringLiteral("edit.copy"))->trigger();
        QVERIFY(!QGuiApplication::clipboard()->mimeData() || !QGuiApplication::clipboard()->mimeData()->hasText());
        QVERIFY(e.win.w.statusText().contains(QLatin1String("Only a chart")));
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

    // A chart put on, undone, and the file saved cannot be put on again by Redo (the file was written without it): Redo is given up and nothing breaks; an object taken away after
    // the save, with its steps before it, can't be undone past the save either.
    void redoOfAnAddedChartAfterASaveIsGivenUp()
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
        pressRedo(&e.win.w);
        QVERIFY(e.settle());
        // the chart is not there (it is not in the file either), and the window is whole
        QCOMPARE(int(e.win.w.objects()->idsOf(0).size()), 0);
        QVERIFY(partsOf(e.path).value(QStringLiteral("xl/charts/chart1.xml")).isEmpty());
        selectRange(e, QStringLiteral("A1:C4"));
        QVERIFY(e.win.w.insertChart(ChartKind::Line));
        QCOMPARE(int(e.win.w.objects()->idsOf(0).size()), 1);
        QVERIFY(save(e));
        QCOMPARE(savedCharts(e.path).size(), 1);
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
