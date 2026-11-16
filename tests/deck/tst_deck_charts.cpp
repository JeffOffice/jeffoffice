// JeffDeck's tests of charts the person makes: Insert > Chart, the Chart tab's changes, and how they are saved.
#include "deckhelpers.h"
#include "../chartmenu.h"
#include "../withdialog.h"
#include "app/chartactions.h"
#include "app/chartdesign.h"
#include "app/ribbon.h"
#include "app/theme.h"
#include "app/chartdialogs.h"
#include "sheet/chartedit.h"
#include "app/chartedit.h"
#include "core/chartobject.h"
#include "office/chart_write.h"
#include "office/package.h"
#include "core/items.h"
#include "sheet/chartembed.h"
#include "sheet/sheetreader.h"
#include "../chartembedhelpers.h"
#include <QTableWidget>

#include <QCheckBox>
#include <QSet>
#include <QSignalSpy>
#include <QStatusBar>
#include <QToolButton>
#include <QDoubleSpinBox>
#include <QXmlStreamReader>

class DeckChartTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

private:
    static QVector<PictureItem *> chartsOn(const Document &d, int slide = 0)
    {
        QVector<PictureItem *> out;
        for (const ItemPtr &it : d.pages[slide]->items)
            if (it->type() == ItemType::Picture && !static_cast<PictureItem *>(it.get())->chartXml.isEmpty()) out << static_cast<PictureItem *>(it.get());
        return out;
    }

    static std::unique_ptr<Document> deckWithChart(office::sheet::ChartForm form = office::sheet::ChartForm::Column)
    {
        auto d = deck::newPresentation();
        auto pic = jp::makeChartObject(*d, form, office::sheet::sampleChartData(), QRectF(100, 100, 400, 250));
        d->pages[0]->items.push_back(pic);
        return d;
    }

    static QByteArray pdfOf(const Document &d, QString *err)
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("out.pdf"));
        return dir.isValid() && deck::exportPdf(d, path, deck::OutputOptions(), err) ? fileBytes(path) : QByteArray();
    }

    // What a PDF page draws: its compressed streams, one after another.
    static QByteArray pdfContent(const QByteArray &pdf)
    {
        QByteArray all;
        for (qsizetype at = pdf.indexOf("stream\n"); at >= 0; at = pdf.indexOf("stream\n", at + 1)) {
            if (at >= 3 && pdf.mid(at - 3, 3) == "end") continue;
            const qsizetype from = at + 7, to = pdf.indexOf("endstream", from);
            if (to < 0) break;
            if (pdf[from] == 'x') all += qUncompress(QByteArray(4, '\0') + pdf.mid(from, to - from));
        }
        return all;
    }

private Q_SLOTS:
    // Insert, undo, redo: the chart is a selected object of the slide with the sample data, and each change of it is one step.
    void insertChartThenUndoAndRedo()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        const int before = int(ed->doc()->pages[ed->currentPage()]->items.size());
        w.act(QStringLiteral("insert.chart.column"))->trigger();
        QCOMPARE(int(ed->doc()->pages[ed->currentPage()]->items.size()), before + 1);
        PictureItem *chart = selectedChart(ed);
        QVERIFY(chart);
        QVERIFY(!chart->imageId.isEmpty());
        QCOMPARE(ed->doc()->images.value(chart->imageId).format, QStringLiteral("svg"));
        office::sheet::ChartInfo info = selectedChartInfo(ed);
        QVERIFY(info.valid);
        QCOMPARE(info.form, office::sheet::ChartForm::Column);
        QCOMPARE(info.data.values, office::sheet::sampleChartData().values);
        QVERIFY(info.dataEditable);
        const QByteArray first = chart->chartXml;
        // A change of kind is one step; its undo gives back the column chart and its redo the pie.
        QVERIFY(w.act(QStringLiteral("chart.type.pie"))->isEnabled());
        w.act(QStringLiteral("chart.type.pie"))->trigger();
        QCOMPARE(selectedChartInfo(ed).form, office::sheet::ChartForm::Pie);
        QVERIFY(selectedChart(ed)->chartXml != first);
        ed->undo();
        QVERIFY(selectedChart(ed));
        QCOMPARE(selectedChartInfo(ed).form, office::sheet::ChartForm::Column);
        QCOMPARE(selectedChart(ed)->chartXml, first);
        // New data and a title.
        QVERIFY(changeSelectedChart(ed, QStringLiteral("Title"), [](office::Chart &c) { return office::sheet::chartSetTitle(c, true, QStringLiteral("Sales")); }));
        QCOMPARE(selectedChartInfo(ed).title, QStringLiteral("Sales"));
        office::sheet::ChartData data = office::sheet::sampleChartData();
        data.values[0][1] = 9.5;
        QVERIFY(changeSelectedChart(ed, QStringLiteral("Data"), [&](office::Chart &c) { return office::sheet::chartSetData(c, data, office::defaultTheme()); }));
        QCOMPARE(selectedChartInfo(ed).data.values[0][1], 9.5);
        ed->undo();
        QCOMPARE(selectedChartInfo(ed).data.values[0][1], 2.5);
        ed->undo();
        QVERIFY(!selectedChartInfo(ed).titleShown);
        ed->undo();
        QCOMPARE(int(ed->doc()->pages[ed->currentPage()]->items.size()), before);
        ed->redo();
        QCOMPARE(int(ed->doc()->pages[ed->currentPage()]->items.size()), before + 1);
        ed->redo();
        ed->redo();
        ed->redo();
        ed->select(chartsOn(*ed->doc(), ed->currentPage()).first()->id);
        QCOMPARE(selectedChartInfo(ed).data.values[0][1], 9.5);
        QCOMPARE(selectedChartInfo(ed).title, QStringLiteral("Sales"));
        // Sized anew, the chart is drawn again for its frame.
        const QString drawn = selectedChart(ed)->imageId;
        ed->change(QStringLiteral("Resize"), [&] { selectedChart(ed)->rect.setWidth(180); });
        QVERIFY(selectedChart(ed)->imageId != drawn);
        QCOMPARE(selectedChart(ed)->chartSize.width(), 180.0);
        // (With JP_SHOT_DIR set, the window with a chart on its slide is kept as a picture.)
        if (const QString shots = qEnvironmentVariable("JP_SHOT_DIR"); !shots.isEmpty()) {
            ed->change(QStringLiteral("Resize"), [&] { selectedChart(ed)->rect = QRectF(120, 100, 520, 320); });
            w.resize(1280, 800);
            w.show();
            QVERIFY(QTest::qWaitForWindowExposed(&w));
            QVERIFY(w.grab().save(shots + QStringLiteral("/deck-chart.png")));
        }
    }

    // The Chart tab and the buttons beside a selected chart (the editor every program shares): each entry of Add Chart Element changes the chart as one step, and Undo gives it back.
    void theElementsMenuChangesTheChartOneStepEach()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.resize(1280, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.act(QStringLiteral("insert.chart.column"))->trigger();
        QVERIFY(selectedChart(ed));
        ChartActions *actions = w.chartActions();
        QTRY_VERIFY(actions->buttons() && actions->buttons()->elements()->isVisible());
        QVERIFY(actions->buttons()->looks()->isVisible());
        QVERIFY(actions->buttons()->filters()->isVisible());
        QMenu *menu = actions->buttons()->elements()->menu();
        QVERIFY(menu);
        auto chart = [&] {
            office::Chart c;
            [&] { QVERIFY(readChartOf(*ed->doc(), *selectedChart(ed), &c)); }();
            return c;
        };
        QVERIFY(!selectedChartInfo(ed).titleShown);
        QVERIFY(fireChartMenu(menu, QStringLiteral("chartTitleMenu.Above")));
        QVERIFY(selectedChartInfo(ed).titleShown);
        QVERIFY(fireChartMenu(menu, QStringLiteral("errorBarsMenu.Fixed Value (1)")));
        QCOMPARE(office::sheet::errorBarsKind(chart()), QStringLiteral("fixedVal"));
        QVERIFY(fireChartMenu(menu, QStringLiteral("trendlineMenu.Linear")));
        QCOMPARE(office::sheet::elementsOf(chart()).trendline, QStringLiteral("linear"));
        QVERIFY(fireChartMenu(menu, QStringLiteral("dataLabelsMenu.outEnd")));
        QCOMPARE(office::sheet::elementsOf(chart()).labelPosition, QStringLiteral("outEnd"));
        QVERIFY(fireChartMenu(menu, QStringLiteral("gridlinesMenu.Lines Across, Minor")));
        QVERIFY(office::sheet::elementsOf(chart()).minorGridHorizontal);
        // (A deck has no cells: the entries that read cells are not offered.)
        QVERIFY(!findChartMenuEntry(menu, QStringLiteral("labelsFromCellsAction")));
        QVERIFY(!findChartMenuEntry(menu, QStringLiteral("customErrorBarsAction")));
        // Each was a step: five undos bring back the chart that was inserted.
        for (int i = 0; i < 5; ++i) ed->undo();
        QVERIFY(!selectedChartInfo(ed).titleShown);
        QVERIFY(office::sheet::errorBarsKind(chart()).isEmpty());
        QVERIFY(office::sheet::elementsOf(chart()).trendline.isEmpty());
        // What was added is kept in the file and read again.
        ed->redo();
        ed->redo();
        QString err;
        const QByteArray bytes = deck::exportPresentation(*ed->doc(), &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        office::Chart again;
        QVERIFY(readChartOf(*back, *chartsOn(*back).first(), &again));
        QVERIFY(again.title.present);
        QCOMPARE(office::sheet::errorBarsKind(again), QStringLiteral("fixedVal"));
        // (With JO_SHOT_DIR set: the Chart tab and two of its format panes, in the new look.)
        if (!qEnvironmentVariable("JO_SHOT_DIR").isEmpty()) {
            jp::applyUiTheme(1);
            w.ribbon()->showTab(w.ribbon()->tab(QStringLiteral("Chart")));
            QApplication::processEvents();
            shotOf(&w, QStringLiteral("deck-chart-design"));
            withDialog([&] { w.act(QStringLiteral("chartdesign.formatPart"))->trigger(); }, QStringLiteral("formatChartPartDialog"), [](QDialog *d) { shotOf(d, QStringLiteral("deck-format-chart-part")); }, false);
            withDialog([&] { w.act(QStringLiteral("chartdesign.axisV"))->trigger(); }, QStringLiteral("formatAxisDialog"), [](QDialog *d) { shotOf(d, QStringLiteral("deck-format-axis")); }, false);
        }
    }

    void quickLayoutColorsAndStylesChangeTheChart()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.act(QStringLiteral("insert.chart.column"))->trigger();
        ChartDesign *design = w.chartActions()->design();
        const QByteArray first = selectedChart(ed)->chartXml;
        std::unique_ptr<QMenu> layouts(design->layoutsMenu(nullptr)), colors(design->colorsMenu(nullptr)), looks(design->stylesMenu(nullptr));
        QVERIFY(layouts->actions().size() >= 2);
        QVERIFY(colors->actions().size() >= 2);
        QVERIFY(looks->actions().size() >= 2);
        colors->actions().last()->trigger();
        const QByteArray colored = selectedChart(ed)->chartXml;
        QVERIFY(colored != first);
        looks->actions().last()->trigger();
        QVERIFY(selectedChart(ed)->chartXml != colored);
        const QByteArray styled = selectedChart(ed)->chartXml;
        layouts->actions().last()->trigger();
        QVERIFY(selectedChart(ed)->chartXml != styled);
        for (int i = 0; i < 3; ++i) ed->undo();
        QCOMPARE(selectedChart(ed)->chartXml, first);
    }

    void switchRowAndColumnSelectDataAndChangeType()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.act(QStringLiteral("insert.chart.column"))->trigger();
        const office::sheet::ChartData first = selectedChartInfo(ed).data;
        QVERIFY(w.act(QStringLiteral("chartdesign.switch"))->isEnabled());
        w.act(QStringLiteral("chartdesign.switch"))->trigger();
        office::sheet::ChartData now = selectedChartInfo(ed).data;
        QCOMPARE(now.categories, first.seriesNames);
        QCOMPARE(now.seriesNames, first.categories);
        QCOMPARE(now.values[0][0], first.values[0][0]);
        QCOMPARE(now.values[1][0], first.values[0][1]);
        ed->undo();
        QCOMPARE(selectedChartInfo(ed).data.values, first.values);
        // Select Data: a series left out is not in the chart.
        QCOMPARE(int(first.seriesNames.size()), 2);
        withDialog([&] { w.act(QStringLiteral("chartdesign.selectData"))->trigger(); }, QStringLiteral("selectChartDataDialog"), [](QDialog *d) {
            auto *dialog = static_cast<SelectChartDataDialog *>(d);
            QCOMPARE(dialog->seriesShown(), 2);
            dialog->moveSeries(0, 1);
            dialog->setSeriesChecked(0, false);
        });
        now = selectedChartInfo(ed).data;
        QCOMPARE(int(now.seriesNames.size()), 1);
        QCOMPARE(now.seriesNames[0], first.seriesNames[0]);
        ed->undo();
        QCOMPARE(int(selectedChartInfo(ed).data.seriesNames.size()), 2);
        // Change Chart Type: one kind for the whole chart.
        withDialog([&] { w.act(QStringLiteral("chartdesign.changeType"))->trigger(); }, QStringLiteral("changeTypeDialog"), [](QDialog *d) { static_cast<ChangeChartTypeDialog *>(d)->chooseKind(office::sheet::ChartKind::Line); });
        QCOMPARE(selectedChartInfo(ed).form, office::sheet::ChartForm::Line);
        // Format Axis: a lowest value for the vertical axis.
        withDialog([&] { w.act(QStringLiteral("chartdesign.axisV"))->trigger(); }, QStringLiteral("formatAxisDialog"), [](QDialog *d) {
            d->findChild<QCheckBox *>(QStringLiteral("axisMinAuto"))->setChecked(false);
            d->findChild<QDoubleSpinBox *>(QStringLiteral("axisMin"))->setValue(-5);
        });
        office::Chart c;
        QVERIFY(readChartOf(*ed->doc(), *selectedChart(ed), &c));
        const office::ChartAxis *axis = office::sheet::primaryAxis(c, false);
        QVERIFY(axis && !axis->autoMin);
        QCOMPARE(axis->min, -5.0);
        // Format Chart Part: the page the chart stands on and one series.
        withDialog([&] { w.act(QStringLiteral("chartdesign.formatPart"))->trigger(); }, QStringLiteral("formatChartPartDialog"), [](QDialog *d) {
            auto *dialog = static_cast<FormatChartPartDialog *>(d);
            QCOMPARE(dialog->partsShown(), 4);
            dialog->setFill(QColor(0xFF, 0xF0, 0xC0));
            dialog->choosePart(3);
            dialog->setFill(QColor(0x20, 0x40, 0x60));
            dialog->setLineWidth(2.5);
        });
        QVERIFY(readChartOf(*ed->doc(), *selectedChart(ed), &c));
        QCOMPARE(office::sheet::partLookOf(c, office::sheet::ChartPart::Area).fill.name(), QStringLiteral("#fff0c0"));
        QCOMPARE(office::sheet::partLookOf(c, office::sheet::ChartPart::Series, 1).fill.name(), QStringLiteral("#204060"));
        QCOMPARE(office::sheet::partLookOf(c, office::sheet::ChartPart::Series, 1).line.width, 2.5);
        QVERIFY(office::sheet::partLookOf(c, office::sheet::ChartPart::Series, 0).fill.name() != QStringLiteral("#204060"));
        ed->undo();
        QVERIFY(readChartOf(*ed->doc(), *selectedChart(ed), &c));
        QVERIFY(office::sheet::partLookOf(c, office::sheet::ChartPart::Area).fill.name() != QStringLiteral("#fff0c0"));
    }

    void chartFiltersLeaveASeriesOutAndKeepTheLastOne()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.act(QStringLiteral("insert.chart.column"))->trigger();
        ChartDesign *design = w.chartActions()->design();
        QCOMPARE(int(design->filterEntries().size()), 2);
        QVERIFY(design->setSeriesShown(0, false));
        QVERIFY(!design->filterEntries()[0].shown);
        QVERIFY(!design->setSeriesShown(1, false));
        QVERIFY(design->filterEntries()[1].shown);
        ed->undo();
        QVERIFY(design->filterEntries()[0].shown);
    }

    // Switching rows and columns never makes a chart with more series than a chart holds; a chart that reads cells has no numbers of its own to switch.
    void switchingRowsAndColumnsIsHeldToTheMostSeriesAChartHas()
    {
        using namespace office::sheet;
        ChartData wide;
        for (int i = 0; i < kMostChartSeries + 1; ++i) wide.categories << QString::number(i);
        wide.seriesNames << QStringLiteral("A");
        wide.values << QVector<double>(wide.categories.size(), 1.0);
        ChartData out;
        QVERIFY(!jp::switchedChartData(wide, &out));
        wide.categories.removeLast();
        wide.values[0].removeLast();
        QVERIFY(jp::switchedChartData(wide, &out));
        QCOMPARE(int(out.seriesNames.size()), kMostChartSeries);
        QCOMPARE(out.categories, wide.seriesNames);
        office::Chart chart = chartOfData(ChartForm::Column, sampleChartData(), office::defaultTheme());
        QVERIFY(jp::chartOwnData(chart, &out));
        chart.plots[0].series[0].valueRef = QStringLiteral("Sheet1!$B$2:$B$4");
        QVERIFY(!jp::chartOwnData(chart, &out));
    }

    // The Chart Parts menu has a Data Table list: the table on and off, and its four options (alone, and only while the table is on); each is one step, drawn on the slide, and kept in the file.
    void theChartPartsMenuHasADataTableAndItsOptions()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.act(QStringLiteral("insert.chart.column"))->trigger();
        QVERIFY(selectedChart(ed));
        QMenu *menu = w.chartActions()->elementsMenu(&w);
        auto chart = [&] {
            office::Chart c;
            [&] { QVERIFY(readChartOf(*ed->doc(), *selectedChart(ed), &c)); }();
            return c;
        };
        QVERIFY(!chart().dataTable.present);
        QAction *keys = findChartMenuEntry(menu, QStringLiteral("dataTableMenu.Legend Keys"));
        QVERIFY(keys && !keys->isEnabled() && !keys->isChecked());
        QVERIFY(fireChartMenu(menu, QStringLiteral("dataTableMenu.Show Data Table")));
        QVERIFY(chart().dataTable.present && chart().dataTable.horzBorder && chart().dataTable.vertBorder && chart().dataTable.outline && !chart().dataTable.keys);
        keys = findChartMenuEntry(menu, QStringLiteral("dataTableMenu.Legend Keys"));
        QVERIFY(keys->isEnabled());
        QVERIFY(findChartMenuEntry(menu, QStringLiteral("dataTableMenu.Show Data Table"))->isChecked());
        QVERIFY(fireChartMenu(menu, QStringLiteral("dataTableMenu.Legend Keys")));
        QVERIFY(fireChartMenu(menu, QStringLiteral("dataTableMenu.Lines Between Columns")));
        QVERIFY(chart().dataTable.keys && !chart().dataTable.vertBorder && chart().dataTable.horzBorder);
        // it is in the file
        QString err;
        const QByteArray bytes = deck::exportPresentation(*ed->doc(), &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        office::Chart again;
        QVERIFY(readChartOf(*back, *chartsOn(*back).first(), &again));
        QVERIFY(again.dataTable.present && again.dataTable.keys && !again.dataTable.vertBorder && again.dataTable.horzBorder && again.dataTable.outline);
        // one step each: three steps back, no table
        ed->undo();
        QVERIFY(chart().dataTable.keys && chart().dataTable.vertBorder);
        ed->undo();
        QVERIFY(chart().dataTable.present && !chart().dataTable.keys);
        ed->undo();
        QVERIFY(!chart().dataTable.present);
        // a deck saved as an OpenDocument file does not keep a table: the entry is off, with the reason
        ed->setFilePath(QStringLiteral("/tmp/none.odp"));
        QAction *shown = findChartMenuEntry(menu, QStringLiteral("dataTableMenu.Show Data Table"));
        QVERIFY(shown && !shown->isEnabled() && shown->toolTip().contains(QStringLiteral("doesn't keep a data table")));
        ed->setFilePath(QStringLiteral("/tmp/none.pptx"));
        QVERIFY(findChartMenuEntry(menu, QStringLiteral("dataTableMenu.Show Data Table"))->isEnabled());
        // a pie has none
        w.act(QStringLiteral("insert.chart.pie"))->trigger();
        QMenu *pieMenu = w.chartActions()->elementsMenu(&w);
        QVERIFY(!findChartMenuEntry(pieMenu, QStringLiteral("dataTableMenu.Show Data Table")));
        ed->undo();
        QCOMPARE(int(chartsOn(*ed->doc()).size()), 1);
        // (With JO_SHOT_DIR set: a column chart with its table, and the right-click menu of a chart.)
        if (!qEnvironmentVariable("JO_SHOT_DIR").isEmpty()) {
            jp::applyUiTheme(1);
            ed->select(chartsOn(*ed->doc()).first()->id);
            QVERIFY(fireChartMenu(menu, QStringLiteral("dataTableMenu.Show Data Table")));
            w.resize(1280, 800);
            w.show();
            QVERIFY(QTest::qWaitForWindowExposed(&w));
            QApplication::processEvents();
            shotOf(&w, QStringLiteral("deck-chart-data-table"));
            std::unique_ptr<QMenu> context(w.slideContextMenu());
            context->popup(w.canvas()->mapToGlobal(QPoint(380, 260)));
            QApplication::processEvents();
            shotOf(context.get(), QStringLiteral("deck-chart-context-menu"));
            context->close();
        }
    }

    // A chart's right-click menu has JeffCrunch's chart entries (Chart Data, Swap Rows and Columns, Chart Kind, Format Chart Part, and the two axes), by those names; for a chart whose numbers are not its
    // own the data entries are shown off with the reason the Chart tab gives; a shape's menu has none of them.
    void aChartsRightClickMenuHasTheChartEntries()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        const QStringList ids = {QStringLiteral("chartdesign.selectData"), QStringLiteral("chartdesign.switch"), QStringLiteral("chartdesign.changeType"), QStringLiteral("chartdesign.formatPart"),
                                 QStringLiteral("chartdesign.axisH"), QStringLiteral("chartdesign.axisV")};
        const QStringList names = {QStringLiteral("Chart Data…"), QStringLiteral("Swap Rows and Columns"), QStringLiteral("Chart Kind…"), QStringLiteral("Format Chart Part…"),
                                   QStringLiteral("Format Horizontal Axis…"), QStringLiteral("Format Vertical Axis…")};
        auto entries = [&](QMenu *menu) {
            QList<QAction *> found;
            for (const QString &id : ids) {
                QAction *one = nullptr;
                for (QAction *a : menu->actions())
                    if (a->objectName() == id) one = a;
                found << one;
            }
            return found;
        };
        w.act(QStringLiteral("insert.chart.column"))->trigger();
        PictureItem *pic = selectedChart(ed);
        QVERIFY(pic);
        {
            std::unique_ptr<QMenu> menu(w.slideContextMenu());
            const QList<QAction *> found = entries(menu.get());
            for (int i = 0; i < ids.size(); ++i) {
                QVERIFY2(found[i], qPrintable(ids[i]));
                QCOMPARE(found[i]->text(), names[i]);
                QVERIFY2(found[i]->isEnabled(), qPrintable(ids[i]));
            }
            // (the entries are the window's own actions: one that is chosen does what the Chart tab's does)
            QVERIFY(found[0] == w.act(ids[0]));
            QVERIFY(menu->toolTipsVisible());
        }
        // a chart that reads a workbook: the data entries are off, with the reason
        QVERIFY(jp::changeChartOf(*ed->doc(), *pic, [](office::Chart &c) {
            for (office::ChartSeries &s : c.plots[0].series) s.valueRef = QStringLiteral("Sheet1!$B$2:$B$4");
            c.externalDataId = QStringLiteral("rId9");
            return true;
        }));
        ed->select(pic->id);
        {
            std::unique_ptr<QMenu> menu(w.slideContextMenu());
            const QList<QAction *> found = entries(menu.get());
            for (int i = 0; i < 2; ++i) {
                QVERIFY2(found[i] && !found[i]->isEnabled(), qPrintable(ids[i]));
                QVERIFY2(found[i]->toolTip().contains(QStringLiteral("workbook")), qPrintable(ids[i]));
            }
            QVERIFY(found[2]->isEnabled() && found[3]->isEnabled());
        }
        // a shape has none
        auto shape = std::make_shared<ShapeItem>();
        shape->shape = QStringLiteral("rect");
        shape->rect = QRectF(600, 380, 200, 80);
        ed->surfaceItems().push_back(shape);
        ed->select(shape->id);
        QVERIFY(!selectedChart(ed));
        std::unique_ptr<QMenu> menu(w.slideContextMenu());
        for (QAction *a : entries(menu.get())) QVERIFY(!a);
    }

    // The names are JeffCrunch's: one name for one thing in every program.
    void theChartTabAndItsCommandsHaveJeffCrunchsNames()
    {
        DeckWindow w;
        w.act(QStringLiteral("insert.chart.column"))->trigger();
        QVERIFY(w.ribbon()->tab(QStringLiteral("Chart")));
        QVERIFY(!w.ribbon()->tab(QStringLiteral("Chart Design")));
        QCOMPARE(w.act(QStringLiteral("chartdesign.selectData"))->text(), QStringLiteral("Chart Data…"));
        QCOMPARE(w.act(QStringLiteral("chartdesign.changeType"))->text(), QStringLiteral("Chart Kind…"));
        QCOMPARE(w.act(QStringLiteral("chartdesign.switch"))->text(), QStringLiteral("Swap Rows and Columns"));
        // (the old numbered styles are Chart Styles, on the Chart Format tab; the new looks are Looks, on the Chart tab: no menu has two names, and no name is two menus)
        auto menuNames = [&](const QString &tab) {
            QSet<QString> names;
            for (QToolButton *b : w.ribbon()->tab(tab)->findChildren<QToolButton *>())
                if (b->menu() && !b->text().isEmpty()) names.insert(b->text());
            return names;
        };
        const QSet<QString> design = menuNames(QStringLiteral("Chart")), format = menuNames(QStringLiteral("Chart Format"));
        QVERIFY(design.contains(QStringLiteral("Looks")) && design.contains(QStringLiteral("Chart Parts")) && design.contains(QStringLiteral("Layouts")) && design.contains(QStringLiteral("Colors")));
        QVERIFY(format.contains(QStringLiteral("Chart Styles")));
        QVERIFY(!design.contains(QStringLiteral("Chart Styles")) && !format.contains(QStringLiteral("Looks")));
        // (the groups that fold into a button have the same names on both tabs, as they do in JeffCrunch: only menus are compared)
        QSet<QString> shared = QSet<QString>(design).intersect(format);
        for (const QString &group : {QStringLiteral("Data"), QStringLiteral("Format"), QStringLiteral("Chart Layouts")}) shared.remove(group);
        QVERIFY2(shared.isEmpty(), qPrintable(QStringList(shared.values()).join(QLatin1Char(','))));
    }

    // New numbers are new series: a filter belongs to the series that was there, and a chart always draws something.
    void switchingRowsAndColumnsAfterFilteringSeriesLeavesSomethingDrawn()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        office::sheet::ChartData data;
        data.categories = {QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c")};
        data.seriesNames = {QStringLiteral("S1"), QStringLiteral("S2"), QStringLiteral("S3"), QStringLiteral("S4")};
        for (int s = 0; s < 4; ++s) data.values << QVector<double>{1.0 + s, 2.0 + s, 3.0 + s};
        auto pic = jp::makeChartObject(*ed->doc(), office::sheet::ChartForm::Column, data, QRectF(100, 100, 400, 250));
        QVERIFY(pic);
        ed->surfaceItems().push_back(pic);
        ed->select(pic->id);
        ChartDesign *design = w.chartActions()->design();
        for (int i = 0; i < 3; ++i) QVERIFY(design->setSeriesShown(i, false));
        QVERIFY(design->switchRowsAndColumns());
        const auto entries = design->filterEntries();
        QCOMPARE(int(entries.size()), 3);
        for (const auto &e : entries) QVERIFY(e.shown);
        // (Edit Data does the same)
        QVERIFY(design->setSeriesShown(0, false));
        withDialog([&] { design->editData(); }, QStringLiteral("chartDataDialog"), [](QDialog *) {});
        for (const auto &e : design->filterEntries()) QVERIFY(e.shown);
    }

    // The numbers of a chart that another program saved with its workbook (c:f references and c:externalData) or that is too big to edit here are not edited: the commands are off, the tooltip says why,
    // and a command that is made anyway says it.
    void aChartWhoseNumbersAreNotItsOwnHasItsDataCommandsOffWithTheReason()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.act(QStringLiteral("insert.chart.column"))->trigger();
        QVERIFY(w.act(QStringLiteral("chartdesign.switch"))->isEnabled());
        QCOMPARE(w.act(QStringLiteral("chartdesign.switch"))->toolTip(), w.act(QStringLiteral("chartdesign.switch"))->text());
        PictureItem *pic = selectedChart(ed);
        QVERIFY(pic);
        QVERIFY(jp::changeChartOf(*ed->doc(), *pic, [](office::Chart &c) {
            for (office::ChartSeries &s : c.plots[0].series) {
                s.valueRef = QStringLiteral("Sheet1!$B$2:$B$4");
                s.categoryRef = QStringLiteral("Sheet1!$A$2:$A$4");
                s.nameRef = QStringLiteral("Sheet1!$B$1");
            }
            c.externalDataId = QStringLiteral("rId9");
            return true;
        }));
        ed->select(pic->id);
        w.chartActions()->refresh();
        const char *ids[] = {"chart.editData", "chartdesign.editData", "chartdesign.selectData", "chartdesign.switch"};
        for (const char *id : ids) {
            QVERIFY2(!w.act(QString::fromLatin1(id))->isEnabled(), id);
            QVERIFY2(w.act(QString::fromLatin1(id))->toolTip().contains(QStringLiteral("workbook")), id);
        }
        QVERIFY(w.act(QStringLiteral("chartdesign.changeType"))->isEnabled());
        // (the old Edit Data command, asked for anyway, says why)
        w.chartActions()->design()->editData();
        QVERIFY(w.statusBar()->currentMessage().contains(QStringLiteral("workbook")));
        // A chart past what a chart made here holds (25 series) has them off too, with the limit in the reason.
        w.act(QStringLiteral("insert.chart.column"))->trigger();
        PictureItem *big = selectedChart(ed);
        QVERIFY(big && big != pic);
        QVERIFY(jp::changeChartOf(*ed->doc(), *big, [](office::Chart &c) {
            while (c.plots[0].series.size() < office::sheet::kMostChartSeries + 1) c.plots[0].series.append(c.plots[0].series.last());
            return true;
        }));
        ed->select(big->id);
        w.chartActions()->refresh();
        for (const char *id : ids) {
            QVERIFY2(!w.act(QString::fromLatin1(id))->isEnabled(), id);
            QVERIFY2(w.act(QString::fromLatin1(id))->toolTip().contains(QString::number(office::sheet::kMostChartSeries)), id);
        }
        QVERIFY(!jp::chartOwnData(office::Chart(), nullptr));
    }

    // Select Data: after Swap Rows and Columns the series that were unchecked are left out, and a series nobody named stays unnamed.
    void selectDataSwapLeavesOutUncheckedSeriesAndKeepsUnnamedOnesUnnamed()
    {
        office::sheet::ChartData data;
        data.categories = {QStringLiteral("a"), QStringLiteral("b")};
        data.seriesNames = {QString(), QStringLiteral("Named"), QStringLiteral("Gone")};
        data.values = {{1, 2}, {3, 4}, {5, 6}};
        SelectChartDataDialog dialog(data);
        dialog.setSeriesChecked(2, false);
        dialog.switchRowsAndColumns();
        const office::sheet::ChartData made = dialog.result();
        QCOMPARE(made.seriesNames, (QStringList{QStringLiteral("a"), QStringLiteral("b")}));
        QCOMPARE(made.categories, (QStringList{QString(), QStringLiteral("Named")}));
        QCOMPARE(dialog.seriesShown(), 2);
        QVERIFY(dialog.seriesChecked(0) && dialog.seriesChecked(1));
    }

    // The buttons stand outside the chart (at its left when there is no room at its right), clear of its handles, and say when a mouse button was let go.
    void theChartButtonsStandClearOfTheChartAndSayWhenAMouseButtonIsReleased()
    {
        QWidget view;
        view.resize(400, 300);
        ChartDesign *design = nullptr;
        struct Host : ChartDesignHost {
            QWidget *chartParent() override { return nullptr; }
            bool chartNow(office::Chart *) override { return false; }
            office::Theme chartTheme() override { return office::Theme(); }
            bool changeChart(const QString &, const std::function<void(office::Chart *)> &) override { return false; }
            void chartNotice(const QString &, const QString &, bool) override {}
        } host;
        design = new ChartDesign(&host, &view);
        ChartButtons buttons(design, &view, [](const QString &) { return QIcon(); });
        auto clear = [&](const QRect &chart) {
            buttons.place(chart);
            for (QToolButton *b : {buttons.elements(), buttons.looks(), buttons.filters()}) {
                QVERIFY(b->isVisible() || !view.isVisible());
                QVERIFY2(!b->geometry().intersects(chart.adjusted(-6, -6, 6, 6)), "a button is over the chart or its handles");
            }
        };
        view.show();
        clear(QRect(20, 40, 200, 150));    // room at the right
        QVERIFY(buttons.elements()->x() > 220);
        clear(QRect(150, 40, 245, 150));   // none at the right: at the left
        QVERIFY(buttons.elements()->x() + buttons.elements()->width() < 150);
        QSignalSpy released(&buttons, &ChartButtons::released);
        QTest::mouseClick(&view, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5));
        QTRY_COMPARE(released.count(), 1);
    }

    // A chart made here is a chart part in the .pptx, and opens as a chart that can be changed, with the same data.
    void pptxKeepsTheChartAsAChartPart()
    {
        auto d = deckWithChart(office::sheet::ChartForm::Line);
        PictureItem *made = chartsOn(*d).first();
        made->rotation = 15;
        QString err;
        QStringList skipped;
        const QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QVERIFY2(skipped.isEmpty(), qPrintable(skipped.join(';')));
        office::Package pkg;
        QVERIFY(pkg.read(bytes, &err));
        const QByteArray part = pkg.part(QStringLiteral("/ppt/charts/chart1.xml"));
        QVERIFY(part.contains("<c:chartSpace"));
        QVERIFY(part.contains("<c:lineChart"));
        QVERIFY(pkg.part(QStringLiteral("/ppt/slides/slide1.xml")).contains("drawingml/2006/chart"));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        const QVector<PictureItem *> charts = chartsOn(*back);
        QCOMPARE(int(charts.size()), 1);
        QCOMPARE(charts[0]->rect, made->rect);
        QCOMPARE(charts[0]->rotation, 15.0);
        const office::sheet::ChartInfo info = jp::chartInfoOfPicture(*back, *charts[0]);
        QVERIFY(info.valid);
        QCOMPARE(info.form, office::sheet::ChartForm::Line);
        QCOMPARE(info.data.categories, office::sheet::sampleChartData().categories);
        QCOMPARE(info.data.values, office::sheet::sampleChartData().values);
        QVERIFY(info.dataEditable);
        // It can be changed after it was opened, and saved again as the changed chart.
        QVERIFY(jp::changeChartOf(*back, *charts[0], [](office::Chart &c) { return office::sheet::chartSetTitle(c, true, QStringLiteral("Again")); }));
        const QByteArray again = deck::exportPresentation(*back, &err);
        auto third = deck::importPresentation(again, &err);
        QVERIFY(third);
        QCOMPARE(int(chartsOn(*third).size()), 1);
        QCOMPARE(jp::chartInfoOfPicture(*third, *chartsOn(*third).first()).title, QStringLiteral("Again"));
        // A chart read and not changed is written as it was read.
        auto plain = deck::importPresentation(bytes, &err);
        QCOMPARE(office::Package().read(deck::exportPresentation(*plain, &err)), true);
    }

    // A .odp has no chart that opens as one here: the chart is a picture, and the copy report says so.
    void odpSavesAPictureAndNamesIt()
    {
        auto d = deckWithChart();
        QString err;
        QStringList skipped;
        const QByteArray bytes = deck::exportOdp(*d, false, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QVERIFY2(skipped.join(QLatin1Char(' ')).contains(QStringLiteral("Charts are saved as pictures")), qPrintable(skipped.join(';')));
        auto back = deck::importOdp(bytes, &err);
        QVERIFY(back);
        QVERIFY(chartsOn(*back).isEmpty());
        int pictures = 0;
        for (const ItemPtr &it : back->pages[0]->items) pictures += it->type() == ItemType::Picture;
        QVERIFY(pictures >= 1);
    }

    // PDF output draws the chart as vector paths and text, not as an image.
    void pdfHoldsTheChartAsPaths()
    {
        auto d = deckWithChart(office::sheet::ChartForm::ColumnStacked);
        QString err;
        const QByteArray pdf = pdfOf(*d, &err);
        QVERIFY2(!pdf.isEmpty(), qPrintable(err));
        QVERIFY(!pdf.contains("/Subtype /Image"));
        const QByteArray content = pdfContent(pdf);
        QVERIFY(content.contains(" m\n"));   // drawn as paths: a move to, then lines and curves
        QVERIFY(content.contains(" l\n") || content.contains(" c\n"));
    }

    // The most charts a file holds as charts: past them the pictures stay and the parts go.
    void aFileHoldsOnlySoManyCharts()
    {
        auto d = deck::newPresentation();
        for (int i = 0; i < office::sheet::kMostChartsInFile + 5; ++i) d->pages[0]->items.push_back(jp::makeChartObject(*d, office::sheet::ChartForm::Pie, office::sheet::sampleChartData(), QRectF(10 + i, 10, 120, 80)));
        QCOMPARE(int(chartsOn(*d).size()), office::sheet::kMostChartsInFile + 5);
        auto copy = std::make_unique<Document>();
        copy->fromJson(d->toJson());
        QCOMPARE(int(chartsOn(*copy).size()), office::sheet::kMostChartsInFile);
        // A part of a chart that has more series or points than a chart object keeps is not read as a chart.
        PictureItem *one = chartsOn(*copy).first();
        office::Chart big;
        QVERIFY(jp::readChartOf(*copy, *one, &big));
        // (a copy: resize() may move the list, and a reference to its own first item would then be to freed memory)
        const office::ChartSeries first = big.plots[0].series[0];
        big.plots[0].series.resize(jp::kMostChartObjectSeries + 1, first);
        PictureItem wide = *one;
        wide.chartXml = office::writeChartPart(big);
        QVERIFY(!jp::readChartOf(*copy, wide, nullptr));
    }

    // The words of the chart dialogs and commands were moved out of the word processor's window into the shared ones: in every language the new context holds the translation the old one had.
    void movedChartTextKeepsItsTranslations()
    {
        const QStringList moved = {
            "Chart Title", "Words for the title of the chart:", "Title of the Axis of Values", "Title of the Axis of Categories", "Words for the title of the axis (leave empty for none):", "Chart Data",
            "Add Row", "Add Column", "Remove Last Row", "Remove Last Column", "Row %1 has a cell that is not a number, so the data was not changed.",
            "The first column holds the words under the points. The first row holds the names of the series. The other cells are numbers."};
        const QStringList commands = {"Column", "Stacked Column", "100% Stacked Column", "Bar", "Stacked Bar", "Line", "Line with Markers", "Pie", "Doughnut", "Area", "Stacked Area", "Scatter",
                                      "Edit Data\u2026", "Edit Chart Title\u2026", "Axis of Values", "Axis of Categories", "Title of the Axis of Values\u2026", "Title of the Axis of Categories\u2026",
                                      "Major Gridlines of Values", "Minor Gridlines of Values", "Major Gridlines of Categories", "Style %1", "Data Labels", "No Legend", "Legend at Right",
                                      "Legend at Top", "Legend at Left", "Legend at Bottom", "Chart Title"};
        int checked = 0;
        for (const char *lang : {"de", "es", "fr", "it", "nl", "pt_BR", "pt_PT"}) {
            QFile ts(QStringLiteral(JD_SOURCE_DIR "/translations/jeffoffice_%1.ts").arg(QLatin1String(lang)));
            QVERIFY(ts.open(QIODevice::ReadOnly));
            QHash<QString, QString> made;   // (context, a tab, and the text) to what it was translated to
            QXmlStreamReader xml(&ts);
            QString context, source;
            while (!xml.atEnd()) {
                xml.readNext();
                if (!xml.isStartElement()) continue;
                if (xml.name() == QLatin1String("name")) context = xml.readElementText();
                else if (xml.name() == QLatin1String("source")) source = xml.readElementText();
                else if (xml.name() == QLatin1String("translation") && xml.attributes().value(QLatin1String("type")) != QLatin1String("vanished")) made.insert(context + QLatin1Char('\t') + source, xml.readElementText(QXmlStreamReader::SkipChildElements));
            }
            QVERIFY(!xml.hasError());
            auto check = [&](const QString &from, const QString &to, const QStringList &texts) {
                for (const QString &text : texts) {
                    const QString was = made.value(from + QLatin1Char('\t') + text);
                    if (was.isEmpty()) continue;   // (a language that never had it)
                    ++checked;
                    QVERIFY2(made.value(to + QLatin1Char('\t') + text) == was, qPrintable(QStringLiteral("%1: \"%2\" is not translated in %3").arg(QLatin1String(lang), text, to)));
                }
            };
            check(QStringLiteral("jp::WriteWindow"), QStringLiteral("jp::ChartDialogs"), moved);
            check(QStringLiteral("jp::WriteWindow"), QStringLiteral("jp::ChartActions"), commands);
        }
        QVERIFY2(checked > 100, qPrintable(QString::number(checked)));
    }

    // The chart of the imported sample (it has a workbook and shapes over it), as a slide's picture.
    static PictureItem *importedChart(Document &d, int slide = 0)
    {
        const QVector<PictureItem *> charts = chartsOn(d, slide);
        return charts.isEmpty() ? nullptr : charts.first();
    }

    static QString slideXml(const QByteArray &bytes)
    {
        office::Package pkg;
        return pkg.read(bytes) ? QString::fromUtf8(pkg.part(QStringLiteral("/ppt/slides/slide1.xml"))) : QString();
    }

    // A chart opened from a file is read again, after a resize or an edit, with the theme and the color map of its slide: on a dark master its words stay light.
    void aChartKeepsItsColorsAfterAResize()
    {
        Parts p = chartPptx();
        QByteArray master = p.data["/ppt/slideMasters/slideMaster1.xml"].xml;
        QVERIFY(master.contains("bg1=\"lt1\" tx1=\"dk1\" bg2=\"lt2\" tx2=\"dk2\""));
        master.replace("bg1=\"lt1\" tx1=\"dk1\" bg2=\"lt2\" tx2=\"dk2\"", "bg1=\"dk1\" tx1=\"lt1\" bg2=\"dk2\" tx2=\"lt2\"");
        p.data["/ppt/slideMasters/slideMaster1.xml"].xml = master;
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        PictureItem *chart = importedChart(*d);
        QVERIFY(chart);
        office::Chart before;
        QVERIFY(jp::readChartOf(*d, *chart, &before));
        QVERIFY2(before.textColor.lightness() > 128, "the words of a chart on a dark master are light");
        chart->rect.setWidth(chart->rect.width() + 100);
        QVERIFY(jp::redrawResizedCharts(*d));
        office::Chart after;
        QVERIFY(jp::readChartOf(*d, *chart, &after));
        QCOMPARE(after.textColor, before.textColor);
        QVERIFY(after.textColor.lightness() > 128);
    }

    // A chart copied to another slide has no workbook or shapes over it to name; one that is changed in kind keeps the ones it has.
    void aPastedChartNamesNoWorkbookAndAChangeOfKindKeepsIt()
    {
        QString err;
        auto d = deck::importPresentation(pack(chartPptx()), &err);
        QVERIFY2(d, qPrintable(err));
        PictureItem *chart = importedChart(*d);
        QVERIFY(chart);
        // A copy of it on the slide (a paste makes a new object with its own number: nothing the file kept is its own).
        ItemPtr copy = chart->clone();
        copy->id = newId();
        copy->sourceId = 0;
        copy->rect.translate(40, 40);
        d->pages[0]->items.push_back(copy);
        // The chart is changed to a pie: its workbook and shapes stay.
        QVERIFY(jp::changeChartOf(*d, *chart, [&](office::Chart &c) { return office::sheet::chartSetForm(c, office::sheet::ChartForm::Pie, jp::chartThemeOfPicture(*d, *chart)); }));
        QVERIFY(chart->chartXml.contains("externalData"));
        QVERIFY(chart->chartXml.contains("userShapes"));
        // What the chart editor's menus add (a title with its words, a legend below) is kept with the workbook too.
        QVERIFY(jp::changeChartOf(*d, *chart, [](office::Chart &c) {
            office::sheet::setChartTitleText(&c, QStringLiteral("Kept title"));
            office::sheet::setLegend(&c, QStringLiteral("b"));
            return true;
        }));
        QVERIFY(chart->chartXml.contains("Kept title") && chart->chartXml.contains("externalData") && chart->chartXml.contains("userShapes"));
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        office::Package pkg;
        QVERIFY(pkg.read(bytes, &err));
        int withWorkbook = 0, without = 0;
        for (const QString &name : pkg.partNames()) {
            if (!name.startsWith(QStringLiteral("/ppt/charts/chart"))) continue;
            const QByteArray part = pkg.part(name);
            if (part.contains("<c:pieChart")) {
                // Whichever names the workbook has the link in its relationships; the one that does not says nothing of it.
                const bool names = part.contains("externalData");
                if (names) {
                    ++withWorkbook;
                    QVERIFY(!pkg.relationshipsOfType(name, QStringLiteral("/package")).isEmpty());
                    QVERIFY(!pkg.relationshipsOfType(name, QStringLiteral("/chartUserShapes")).isEmpty());
                } else {
                    ++without;
                    QVERIFY(!part.contains("userShapes"));
                }
            } else {
                // The copy is a column chart still: it names nothing.
                QVERIFY2(!part.contains("externalData") && !part.contains("userShapes"), qPrintable(name));
                ++without;
            }
        }
        QCOMPARE(withWorkbook, 1);
        QCOMPARE(without, 1);
        QVERIFY(pkg.hasPart(QStringLiteral("/ppt/embeddings/Sheet1.xlsx")));
        // Every link a chart part names is there.
        for (const QString &name : pkg.partNames())
            if (name.startsWith(QStringLiteral("/ppt/charts/chart")))
                for (const office::Relationship &r : pkg.relationships(name)) QVERIFY2(r.external || pkg.hasPart(r.target), qPrintable(r.target));
    }

    // ---- the numbers of a chart another program saved with its workbook
    // The sample chart with a workbook that is a workbook (the cells its references name) and a second series that reads its own column.
    static Parts workbookChartPptx(const QByteArray &book = jp::testing::salesBook())
    {
        Parts p = chartPptx();
        QString chart = QString::fromUtf8(p.data["/ppt/charts/chart1.xml"].xml);
        const int second = chart.indexOf(QStringLiteral("<c:ser>"), chart.indexOf(QStringLiteral("<c:ser>")) + 1);
        QString tail = chart.mid(second);
        tail.replace(QStringLiteral("$B$"), QStringLiteral("$C$"));
        p.data["/ppt/charts/chart1.xml"].xml = (chart.left(second) + tail).toUtf8();
        p.data["/ppt/embeddings/Sheet1.xlsx"].xml = book;
        return p;
    }

    // Edit Data on such a chart is the grid of the cells: values and names change in the workbook and in the chart, as one step; saved, the workbook part is the new workbook (and only it), the chart
    // part has the new numbers, and both read again.
    void theNumbersOfAChartWithItsWorkbookAreEditedAsOneStepAndSavedInBoth()
    {
        QString err;
        auto d = deck::importPresentation(pack(workbookChartPptx()), &err);
        QVERIFY2(d, qPrintable(err));
        DeckWindow w;
        w.editor()->setDocument(std::move(d));
        Editor *ed = w.editor();
        ed->setCurrentPage(0);
        PictureItem *pic = chartsOn(*ed->doc()).first();
        ed->select(pic->id);
        w.chartActions()->refresh();
        // The commands that change numbers and names are on; the ones that would add, take away, or swap a row or column say why they are not.
        QVERIFY(w.act(QStringLiteral("chartdesign.editData"))->isEnabled());
        QVERIFY(w.act(QStringLiteral("chart.editData"))->isEnabled());
        for (const char *id : {"chartdesign.selectData", "chartdesign.switch"}) {
            QVERIFY2(!w.act(QString::fromLatin1(id))->isEnabled(), id);
            QVERIFY2(w.act(QString::fromLatin1(id))->toolTip().contains(QStringLiteral("workbook")), id);
        }
        const QByteArray bookBefore = jp::chartWorkbookOf(*ed->doc(), *pic);
        QVERIFY(bookBefore.startsWith("PK"));
        // Refreshing the commands again and again, with the same chart selected, opens its workbook no more.
        const qint64 opened = office::sheet::embeddedWorkbooksOpenedForTest();
        const qint64 said = jp::chartInfosMadeForTest();
        for (int i = 0; i < 12; ++i) {
            w.chartActions()->refresh();
            QVERIFY(w.chartActions()->design()->dataProblem().isEmpty());
            QVERIFY(!w.chartActions()->design()->structureProblem().isEmpty());
        }
        QCOMPARE(office::sheet::embeddedWorkbooksOpenedForTest() - opened, qint64(0));
        QCOMPARE(jp::chartInfosMadeForTest() - said, qint64(0));
        withDialog([&] { w.act(QStringLiteral("chartdesign.editData"))->trigger(); }, QStringLiteral("chartDataDialog"), [&](QDialog *dlg) {
            auto *table = dlg->findChild<QTableWidget *>(QStringLiteral("chartDataTable"));
            QVERIFY(table);
            QCOMPARE(table->rowCount(), 4);
            QCOMPARE(table->columnCount(), 3);
            QCOMPARE(table->item(0, 1)->text(), QStringLiteral("Sales"));
            QCOMPARE(table->item(2, 0)->text(), QStringLiteral("Q2"));
            QCOMPARE(table->item(3, 2)->text(), QStringLiteral("6"));
            table->item(2, 1)->setText(QStringLiteral("42"));
            table->item(0, 2)->setText(QStringLiteral("Expenses"));
        });
        QVERIFY(jp::chartWorkbookOf(*ed->doc(), *pic) != bookBefore);
        office::Chart now;
        QVERIFY(jp::readChartOf(*ed->doc(), *pic, &now));
        QCOMPARE(now.plots[0].series[0].values, (QVector<double>{1, 42, 3}));
        QCOMPARE(now.plots[0].series[1].name, QStringLiteral("Expenses"));
        QVERIFY(jp::chartInfoOfPicture(*ed->doc(), *pic).dataEditable);
        // One step: Undo puts back the chart and the workbook, Redo the edit.
        ed->undo();
        pic = chartsOn(*ed->doc()).first();
        QCOMPARE(jp::chartWorkbookOf(*ed->doc(), *pic), bookBefore);
        QVERIFY(jp::readChartOf(*ed->doc(), *pic, &now));
        QCOMPARE(now.plots[0].series[0].values, (QVector<double>{10, 20, 40}));
        ed->redo();
        pic = chartsOn(*ed->doc()).first();
        QVERIFY(jp::readChartOf(*ed->doc(), *pic, &now));
        QCOMPARE(now.plots[0].series[0].values, (QVector<double>{1, 42, 3}));
        // Saved: one workbook part, the new one, which opens as a workbook with the new numbers; the chart part names the same cells with the new values; and it all reads again.
        const QByteArray bytes = deck::exportPresentation(*ed->doc(), &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        office::Package pkg;
        QVERIFY(pkg.read(bytes, &err));
        int books = 0;
        for (const QString &name : pkg.partNames())
            if (name.endsWith(QStringLiteral(".xlsx"))) ++books;
        QCOMPARE(books, 1);
        const QByteArray book = pkg.part(QStringLiteral("/ppt/embeddings/Sheet1.xlsx"));
        office::sheet::ChartData cells;
        office::Chart saved;
        QVERIFY(office::readChart(pkg.part(QStringLiteral("/ppt/charts/chart1.xml")), office::defaultTheme(), office::ColorMap(), &saved));
        QCOMPARE(saved.plots[0].series[0].values, (QVector<double>{1, 42, 3}));
        QCOMPARE(saved.plots[0].series[0].valueRef, QStringLiteral("Sheet1!$B$2:$B$4"));
        QVERIFY2(office::sheet::embeddedChartProblem(saved, book, &cells).isEmpty(), qPrintable(office::sheet::embeddedChartProblem(saved, book)));
        QCOMPARE(cells.values[0], (QVector<double>{1, 42, 3}));
        QCOMPARE(cells.seriesNames[1], QStringLiteral("Expenses"));
        // (the workbook opens as a workbook with the numbers, as JeffCrunch reads it)
        office::Package inner;
        QVERIFY(inner.readLazy(book));
        office::sheet::Workbook wb;
        QVERIFY(wb.open(inner));
        office::sheet::Sheet sheet;
        QVERIFY(wb.readSheet(0, &sheet));
        const office::sheet::Cell *cell = sheet.cells.find(2, 1);
        QVERIFY(cell && cell->number == 42);
        auto again = deck::importPresentation(bytes, &err);
        QVERIFY2(again, qPrintable(err));
        PictureItem *back = chartsOn(*again).first();
        QCOMPARE(jp::chartWorkbookOf(*again, *back), book);
        QVERIFY(jp::chartInfoOfPicture(*again, *back).dataEditable);
        QCOMPARE(jp::chartInfoOfPicture(*again, *back).data.values[0], (QVector<double>{1, 42, 3}));
    }

    // The grid shows every digit of a number and an edit elsewhere leaves it, and every other cell, as it was in the workbook.
    void theGridKeepsEveryDigitOfANumberAndAnEditElsewhereLeavesItAsItWas()
    {
        using namespace jp::testing;
        const QByteArray book = bookOf(bookRow(1, {bookText(QStringLiteral("B1"), QStringLiteral("Sales")), bookText(QStringLiteral("C1"), QStringLiteral("Costs"))}) +
                                       bookRow(2, {bookText(QStringLiteral("A2"), QStringLiteral("Q1")), bookNumber(QStringLiteral("B2"), 0.30000000000000004), bookNumber(QStringLiteral("C2"), 4)}) +
                                       bookRow(3, {bookText(QStringLiteral("A3"), QStringLiteral("Q2")), bookNumber(QStringLiteral("B3"), 2), bookNumber(QStringLiteral("C3"), 5)}) +
                                       bookRow(4, {bookText(QStringLiteral("A4"), QStringLiteral("Q3")), bookNumber(QStringLiteral("B4"), 3), bookNumber(QStringLiteral("C4"), 6)}));
        QString err;
        auto d = deck::importPresentation(pack(workbookChartPptx(book)), &err);
        QVERIFY2(d, qPrintable(err));
        DeckWindow w;
        w.editor()->setDocument(std::move(d));
        w.editor()->setCurrentPage(0);
        w.editor()->select(chartsOn(*w.editor()->doc()).first()->id);
        w.chartActions()->refresh();
        withDialog([&] { w.act(QStringLiteral("chartdesign.editData"))->trigger(); }, QStringLiteral("chartDataDialog"), [&](QDialog *dlg) {
            auto *table = dlg->findChild<QTableWidget *>(QStringLiteral("chartDataTable"));
            QVERIFY(table);
            QCOMPARE(table->item(1, 1)->text(), QStringLiteral("0.30000000000000004"));
            table->item(3, 2)->setText(QStringLiteral("7"));
        });
        const QByteArray bytes = deck::exportPresentation(*w.editor()->doc(), &err);
        office::Package pkg;
        QVERIFY(pkg.read(bytes, &err));
        office::Package inner;
        QVERIFY(inner.readLazy(pkg.part(QStringLiteral("/ppt/embeddings/Sheet1.xlsx"))));
        const QByteArray sheet = inner.part(QStringLiteral("/xl/worksheets/sheet1.xml"));
        QVERIFY(sheet.contains("0.30000000000000004"));
        QVERIFY(sheet.contains("<v>7</v>"));
    }

    // A reference that can't be followed keeps the chart read-only, with the reason (here a second series that reads the cells of the first).
    void aChartWhoseReferencesCantBeFollowedStaysReadOnlyWithItsWorkbook()
    {
        Parts p = workbookChartPptx();
        QString chart = QString::fromUtf8(p.data["/ppt/charts/chart1.xml"].xml);
        chart.replace(QStringLiteral("$C$"), QStringLiteral("$B$"));
        p.data["/ppt/charts/chart1.xml"].xml = chart.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        const PictureItem *pic = chartsOn(*d).first();
        QVERIFY(!jp::chartInfoOfPicture(*d, *pic).dataEditable);
        DeckWindow w;
        w.editor()->setDocument(std::move(d));
        w.editor()->setCurrentPage(0);
        w.editor()->select(chartsOn(*w.editor()->doc()).first()->id);
        w.chartActions()->refresh();
        QVERIFY(!w.act(QStringLiteral("chartdesign.editData"))->isEnabled());
        QVERIFY(w.act(QStringLiteral("chartdesign.editData"))->toolTip().contains(QStringLiteral("workbook")));
    }

    // A screenshot of the data grid on such a chart (only when JO_SHOT_DIR is set).
    void shotOfTheGridOfAChartWithItsWorkbook()
    {
        if (qEnvironmentVariableIsEmpty("JO_SHOT_DIR")) QSKIP("set JO_SHOT_DIR for a screenshot");
        jp::applyUiTheme(1);
        QString err;
        auto d = deck::importPresentation(pack(workbookChartPptx()), &err);
        QVERIFY2(d, qPrintable(err));
        DeckWindow w;
        w.editor()->setDocument(std::move(d));
        w.editor()->setCurrentPage(0);
        w.editor()->select(chartsOn(*w.editor()->doc()).first()->id);
        w.chartActions()->refresh();
        withDialog([&] { w.act(QStringLiteral("chartdesign.editData"))->trigger(); }, QStringLiteral("chartDataDialog"), [](QDialog *dlg) { shotOf(dlg, QStringLiteral("deck-embedded-chart-data")); }, false);
    }

    // A frame's turn is the file's rot: sixty-thousandths of a degree, clockwise. Turned back to zero it says nothing.
    void aChartsTurnIsWrittenAsTheFileFormatDefinesIt()
    {
        auto d = deckWithChart();
        PictureItem *chart = chartsOn(*d).first();
        QString err;
        chart->rotation = 90;
        QVERIFY(slideXml(deck::exportPresentation(*d, &err)).contains(QStringLiteral("rot=\"5400000\"")));
        chart->rotation = 15.5;
        QVERIFY(slideXml(deck::exportPresentation(*d, &err)).contains(QStringLiteral("rot=\"930000\"")));
        chart->rotation = -30;   // (a turn counterclockwise is the same as 330 degrees clockwise)
        QVERIFY(slideXml(deck::exportPresentation(*d, &err)).contains(QStringLiteral("rot=\"19800000\"")));
        // And one the file turns by its definition is read as degrees clockwise.
        chart->rotation = 90;
        auto turned = deck::importPresentation(deck::exportPresentation(*d, &err), &err);
        QVERIFY(turned);
        QCOMPARE(chartsOn(*turned).first()->rotation, 90.0);
        // Turned back to zero, the old turn is gone from the frame.
        chartsOn(*turned).first()->rotation = 0;
        const QString xml = slideXml(deck::exportPresentation(*turned, &err));
        QVERIFY(xml.contains(QStringLiteral("drawingml/2006/chart")));
        QVERIFY2(!xml.contains(QStringLiteral("rot=")), qPrintable(xml.left(1500)));
    }

    // A chart inside a group is a chart in the file too.
    void aChartInAGroupIsWrittenAsAChartPart()
    {
        auto d = deck::newPresentation();
        auto group = std::make_shared<GroupItem>();
        group->rect = QRectF(50, 50, 500, 300);
        group->children.push_back(jp::makeChartObject(*d, office::sheet::ChartForm::Doughnut, office::sheet::sampleChartData(), QRectF(60, 60, 300, 200)));
        d->pages[0]->items.push_back(group);
        QString err;
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        office::Package pkg;
        QVERIFY(pkg.read(bytes, &err));
        QVERIFY(pkg.part(QStringLiteral("/ppt/charts/chart1.xml")).contains("<c:doughnutChart"));
        const QString xml = slideXml(bytes);
        QVERIFY(xml.contains(QStringLiteral("<p:grpSp>")) && xml.contains(QStringLiteral("drawingml/2006/chart")));
    }

    // The most charts a file keeps as charts: a file with more says so when it is opened, and Insert stops there with a message.
    void theLimitOnChartsIsSaidOnOpenAndOnInsert()
    {
        Parts p = chartPptx();
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        const qsizetype named = xml.indexOf(QStringLiteral("name=\"Chart 8\"")), from = xml.lastIndexOf(QStringLiteral("<p:graphicFrame>"), named), to = xml.indexOf(QStringLiteral("</p:graphicFrame>"), named) + 17;
        QVERIFY(named > 0 && from > 0);
        const QString frame = xml.mid(from, to - from);
        QString more;
        for (int i = 0; i < office::sheet::kMostChartsInFile + 3; ++i) {
            QString copy = frame;
            copy.replace(QStringLiteral("id=\"9\" name=\"Chart 8\""), QStringLiteral("id=\"%1\" name=\"Chart %1\"").arg(100 + i));
            more += copy;
        }
        xml.insert(to, more);
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        QString notes;
        auto d = deck::importPresentation(pack(p), &notes);
        QVERIFY(d);
        QCOMPARE(jp::chartObjectsIn(*d), office::sheet::kMostChartsInFile);
        QVERIFY2(notes.contains(QStringLiteral("more charts than can be changed")), qPrintable(notes));
        // At the limit, Insert > Chart adds none and says why.
        DeckWindow w;
        w.editor()->setDocument(std::move(d));
        QSignalSpy said(w.editor(), &Editor::status);
        const int items = int(w.editor()->doc()->pages[0]->items.size());
        w.editor()->setCurrentPage(0);
        QVERIFY(jp::insertChart(w.editor(), office::sheet::ChartForm::Column).isEmpty());
        QCOMPARE(int(w.editor()->doc()->pages[0]->items.size()), items);
        QCOMPARE(said.size(), 1);
        QVERIFY(said.first().first().toString().contains(QStringLiteral("most charts")));
    }

    // A chart that is not changed is written as the file had it, byte for byte; the other parts it links to come with it.
    void anUnchangedChartPartIsWrittenAsItWasRead()
    {
        Parts p = chartPptx();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        office::Package pkg;
        QVERIFY(pkg.read(bytes, &err));
        QCOMPARE(pkg.part(QStringLiteral("/ppt/charts/chart1.xml")), p.data["/ppt/charts/chart1.xml"].xml);
        // And again, through a second save of what that gave.
        auto again = deck::importPresentation(bytes, &err);
        office::Package second;
        QVERIFY(second.read(deck::exportPresentation(*again, &err), &err));
        QCOMPARE(second.part(QStringLiteral("/ppt/charts/chart1.xml")), p.data["/ppt/charts/chart1.xml"].xml);
    }

    // A chart is not read, checked, and drawn again each time something asks what it is.
    void aChartIsReadOnceForAllThatAskWhatItIs()
    {
        auto d = deckWithChart();
        PictureItem *chart = chartsOn(*d).first();
        chart->chartXml += " ";   // (a part this thread has not been asked about)
        const qint64 before = jp::chartInfosMadeForTest();
        for (int i = 0; i < 12; ++i) QVERIFY(jp::chartInfoOfPicture(*d, *chart).valid);
        QCOMPARE(jp::chartInfosMadeForTest() - before, qint64(1));
        // A change makes a new part, which is read once.
        QVERIFY(jp::changeChartOf(*d, *chart, [](office::Chart &c) { return office::sheet::chartSetTitle(c, true, QStringLiteral("T")); }));
        const qint64 mid = jp::chartInfosMadeForTest();
        for (int i = 0; i < 12; ++i) QVERIFY(jp::chartInfoOfPicture(*d, *chart).titleShown);
        QCOMPARE(jp::chartInfosMadeForTest() - mid, qint64(1));
    }

    // A chart part's text in a file is not decoded when it is longer than a part may be.
    void anOversizeChartPartIsNotDecoded()
    {
        PictureItem small;
        small.chartXml = "<c:chartSpace/>";
        small.chartSize = QSizeF(10, 10);
        PictureItem back;
        back.fromJson(small.toJson());
        QCOMPARE(back.chartXml, small.chartXml);
        QJsonObject big = small.toJson();
        big["chart"] = QString(int(kMaxChartPartBytes) / 3 * 4 + 1000, QLatin1Char('A'));
        PictureItem refused;
        refused.fromJson(big);
        QVERIFY(refused.chartXml.isEmpty());
    }
};

JP_TEST_CLASS(DeckChartTests)
#include "tst_deck_charts.moc"
