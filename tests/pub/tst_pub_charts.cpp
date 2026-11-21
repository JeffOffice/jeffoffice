// JeffPub's tests of charts the person makes: Insert > Chart, the Chart tab's changes, and how they are saved (.jpub keeps the chart, .pub a picture of it).
#include "pubhelpers.h"
#include "app/chartedit.h"
#include "core/chartobject.h"
#include "office/chart_write.h"
#include "../chartmenu.h"
#include "../withdialog.h"
#include "app/chartactions.h"
#include "app/chartdesign.h"
#include "app/chartdialogs.h"
#include "app/ribbon.h"
#include "app/theme.h"
#include "sheet/chartedit.h"
#include <QToolButton>

class PubChartTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initPubTestCase(); }

private:
    static QVector<jp::PictureItem *> chartsOn(const jp::Document &d, int page = 0)
    {
        QVector<jp::PictureItem *> out;
        for (const jp::ItemPtr &it : d.pages[page]->items)
            if (it->type() == jp::ItemType::Picture && !static_cast<jp::PictureItem *>(it.get())->chartXml.isEmpty()) out << static_cast<jp::PictureItem *>(it.get());
        return out;
    }

    static std::unique_ptr<jp::Document> publicationWithChart()
    {
        auto d = jp::Document::blank(QSizeF(612, 792));
        d->pages[0]->items.push_back(jp::makeChartObject(*d, jp::office::sheet::ChartForm::BarStacked, jp::office::sheet::sampleChartData(), QRectF(72, 72, 360, 216)));
        return d;
    }

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
    // Insert > Chart puts a selected chart object on the page; each change of it, and the insert, is a step that undoes and redoes.
    void insertChartThenUndoAndRedo()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        const int before = int(ed->doc()->pages[ed->currentPage()]->items.size());
        w.act(QStringLiteral("insert.chart.column"))->trigger();
        QCOMPARE(int(ed->doc()->pages[ed->currentPage()]->items.size()), before + 1);
        jp::PictureItem *chart = jp::selectedChart(ed);
        QVERIFY(chart);
        const jp::office::sheet::ChartInfo info = jp::selectedChartInfo(ed);
        QVERIFY(info.valid);
        QCOMPARE(info.form, jp::office::sheet::ChartForm::Column);
        QCOMPARE(info.data.values, jp::office::sheet::sampleChartData().values);
        const QByteArray first = chart->chartXml;
        w.act(QStringLiteral("chart.type.doughnut"))->trigger();
        QCOMPARE(jp::selectedChartInfo(ed).form, jp::office::sheet::ChartForm::Doughnut);
        w.act(QStringLiteral("chart.legend.bottom"))->trigger();
        QCOMPARE(jp::selectedChartInfo(ed).legendPosition, QStringLiteral("b"));
        ed->undo();
        QCOMPARE(jp::selectedChartInfo(ed).form, jp::office::sheet::ChartForm::Doughnut);
        ed->undo();
        QCOMPARE(jp::selectedChart(ed)->chartXml, first);
        ed->undo();
        QCOMPARE(int(ed->doc()->pages[ed->currentPage()]->items.size()), before);
        ed->redo();
        QCOMPARE(int(ed->doc()->pages[ed->currentPage()]->items.size()), before + 1);
        ed->redo();
        ed->redo();
        QCOMPARE(chartsOn(*ed->doc(), ed->currentPage()).size(), 1);
        ed->select(chartsOn(*ed->doc(), ed->currentPage()).first()->id);
        QCOMPARE(jp::selectedChartInfo(ed).form, jp::office::sheet::ChartForm::Doughnut);
        QCOMPARE(jp::selectedChartInfo(ed).legendPosition, QStringLiteral("b"));
        // The chart is an object like the others: moved, sized (drawn again for its frame), and turned.
        const QString drawn = jp::selectedChart(ed)->imageId;
        ed->change(QStringLiteral("Resize"), [&] { jp::selectedChart(ed)->rect.setSize(QSizeF(150, 300)); });
        QVERIFY(jp::selectedChart(ed)->imageId != drawn);
        ed->rotateSelection(30);
        QCOMPARE(jp::selectedChart(ed)->rotation, 30.0);
        QVERIFY(jp::selectedChart(ed)->chartSize == QSizeF(150, 300));
        // (With JP_SHOT_DIR set, the window with a chart on its page is kept as a picture.)
        if (const QString shots = qEnvironmentVariable("JP_SHOT_DIR"); !shots.isEmpty()) {
            ed->rotateSelection(-30);
            ed->change(QStringLiteral("Resize"), [&] { jp::selectedChart(ed)->rect = QRectF(100, 100, 400, 260); });
            w.resize(1280, 800);
            w.show();
            QVERIFY(QTest::qWaitForWindowExposed(&w));
            QVERIFY(w.grab().save(shots + QStringLiteral("/pub-chart.png")));
        }
    }

    // .jpub keeps the chart model: it opens as the same chart, with the same data, that can still be changed.
    // JeffPub shares the chart editor of libs/page, but a publication keeps no parts of a file's chart (no workbook beside it), so a chart that names one is read-only with the reason.
    void aChartThatNamesAWorkbookIsReadOnlyForWantOfTheWorkbookParts()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        w.act(QStringLiteral("insert.chart.column"))->trigger();
        jp::PictureItem *chart = jp::selectedChart(ed);
        QVERIFY(chart);
        QVERIFY(jp::changeChartOf(*ed->doc(), *chart, [](jp::office::Chart &c) {
            for (jp::office::ChartSeries &s : c.plots[0].series) {
                s.valueRef = QStringLiteral("Sheet1!$B$2:$B$4");
                s.categoryRef = QStringLiteral("Sheet1!$A$2:$A$4");
            }
            c.externalDataId = QStringLiteral("rId1");
            return true;
        }));
        QVERIFY(jp::chartWorkbookOf(*ed->doc(), *chart).isEmpty());
        QVERIFY(!jp::selectedChartInfo(ed).dataEditable);
    }

    // The Chart Design commands (shared with JeffDeck): each entry of Chart Parts, Layouts, Colors, and Looks changes the chart as one step, and the buttons beside the chart show.
    void chartPartsColorsAndLooksAreOneStepEach()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        w.resize(1280, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.act(QStringLiteral("insert.chart.column"))->trigger();
        QVERIFY(jp::selectedChart(ed));
        jp::ChartActions *actions = w.chartActions();
        QTRY_VERIFY(actions->buttons() && actions->buttons()->elements()->isVisible());
        QMenu *menu = actions->buttons()->elements()->menu();
        QVERIFY(menu);
        auto chart = [&] {
            jp::office::Chart c;
            [&] { QVERIFY(jp::readChartOf(*ed->doc(), *jp::selectedChart(ed), &c)); }();
            return c;
        };
        const int steps = ed->undoStack()->count();
        const QByteArray first = jp::selectedChart(ed)->chartXml;
        QVERIFY(fireChartMenu(menu, QStringLiteral("errorBarsMenu.Fixed Value (1)")));
        QCOMPARE(jp::office::sheet::errorBarsKind(chart()), QStringLiteral("fixedVal"));
        QCOMPARE(ed->undoStack()->count(), steps + 1);
        QVERIFY(fireChartMenu(menu, QStringLiteral("trendlineMenu.Linear")));
        QCOMPARE(jp::office::sheet::elementsOf(chart()).trendline, QStringLiteral("linear"));
        QCOMPARE(ed->undoStack()->count(), steps + 2);
        std::unique_ptr<QMenu> layouts(actions->layoutsMenu(nullptr)), colors(actions->colorsMenu(nullptr)), looks(actions->looksMenu(nullptr));
        QVERIFY(layouts->actions().size() >= 2 && colors->actions().size() >= 2 && looks->actions().size() >= 2);
        QByteArray before = jp::selectedChart(ed)->chartXml;
        for (QMenu *m : {colors.get(), looks.get(), layouts.get()}) {
            m->actions().last()->trigger();
            QVERIFY(jp::selectedChart(ed)->chartXml != before);
            before = jp::selectedChart(ed)->chartXml;
        }
        QCOMPARE(ed->undoStack()->count(), steps + 5);
        for (int i = 0; i < 5; ++i) ed->undo();
        QCOMPARE(jp::selectedChart(ed)->chartXml, first);
        for (int i = 0; i < 2; ++i) ed->redo();
        QCOMPARE(jp::office::sheet::elementsOf(chart()).trendline, QStringLiteral("linear"));
        // Format Chart Part: the look of the chart area and of one series, as one step.
        const int formatSteps = ed->undoStack()->index();
        withDialog([&] { w.act(QStringLiteral("chartdesign.formatPart"))->trigger(); }, QStringLiteral("formatChartPartDialog"), [](QDialog *d) {
            auto *dialog = static_cast<jp::FormatChartPartDialog *>(d);
            dialog->setFill(QColor(0xFF, 0xF0, 0xC0));
            dialog->choosePart(2);
            dialog->setFill(QColor(0x20, 0x40, 0x60));
        });
        QCOMPARE(ed->undoStack()->index(), formatSteps + 1);
        QCOMPARE(jp::office::sheet::partLookOf(chart(), jp::office::sheet::ChartPart::Area).fill.name(), QStringLiteral("#fff0c0"));
        QCOMPARE(jp::office::sheet::partLookOf(chart(), jp::office::sheet::ChartPart::Series, 0).fill.name(), QStringLiteral("#204060"));
        ed->undo();
        QVERIFY(jp::office::sheet::partLookOf(chart(), jp::office::sheet::ChartPart::Area).fill.name() != QStringLiteral("#fff0c0"));
        // The ribbon shows them on the Chart Format tab.
        QVERIFY(w.ribbon()->tab(QStringLiteral("Chart Format")));
        // .jpub keeps what was added; .pub keeps a picture.
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QString err;
        const QString path = dir.filePath(QStringLiteral("design.jpub"));
        QVERIFY2(jp::savePublication(*ed->doc(), path, QImage(), &err), qPrintable(err));
        auto back = jp::loadPublication(path, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(int(chartsOn(*back).size()), 1);
        jp::office::Chart again;
        QVERIFY(jp::readChartOf(*back, *chartsOn(*back).first(), &again));
        QCOMPARE(jp::office::sheet::errorBarsKind(again), QStringLiteral("fixedVal"));
        QCOMPARE(jp::office::sheet::elementsOf(again).trendline, QStringLiteral("linear"));
        QVERIFY(jp::exportPublisher(*ed->doc(), dir.filePath(QStringLiteral("design.pub")), &err));
        if (!qEnvironmentVariable("JO_SHOT_DIR").isEmpty()) {
            jp::applyUiTheme(1);
            w.ribbon()->showTab(w.ribbon()->tab(QStringLiteral("Chart Format")));
            QApplication::processEvents();
            shotOf(&w, QStringLiteral("pub-chart-design"));
            withDialog([&] { w.act(QStringLiteral("chartdesign.formatPart"))->trigger(); }, QStringLiteral("formatChartPartDialog"), [](QDialog *d) { shotOf(d, QStringLiteral("pub-format-chart-part")); }, false);
        }
    }

    // Current Selection lists the chart's parts, Format Selection opens the format pane at the one chosen, and Reset to Match Style takes away what was set on it as one step (none when the part
    // already matches); the saved .jpub, opened again in a window, keeps what the reset left and what it did not touch.
    void currentSelectionAndResetToMatchStyle()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        w.act(QStringLiteral("insert.chart.column"))->trigger();
        w.ribbon()->showTab(w.ribbon()->tab(QStringLiteral("Chart Format")));
        QComboBox *box = chartPartBoxOn(w.ribbon()->tab(QStringLiteral("Chart Format")));
        QVERIFY(box);
        QTRY_COMPARE(box->count(), 4);
        QCOMPARE(box->itemText(0), QStringLiteral("Chart Area"));
        QVERIFY(tabHasButtonFor(w.ribbon()->tab(QStringLiteral("Chart Format")), w.act(QStringLiteral("chartdesign.resetStyle"))));
        QVERIFY(tabHasButtonFor(w.ribbon()->tab(QStringLiteral("Chart Format")), w.act(QStringLiteral("chartdesign.formatSelection"))));
        jp::office::Chart c;
        QVERIFY(jp::readChartOf(*ed->doc(), *jp::selectedChart(ed), &c));
        const QColor style = jp::office::chartStyleSeriesColor(c, 1, 2);
        pickChartPart(box, 3);
        withDialog([&] { w.act(QStringLiteral("chartdesign.formatSelection"))->trigger(); }, QStringLiteral("formatChartPartDialog"), [](QDialog *d) {
            auto *dialog = static_cast<jp::FormatChartPartDialog *>(d);
            QCOMPARE(dialog->currentPart(), 3);
            dialog->setFill(QColor(0x20, 0x40, 0x60));
            dialog->setLineWidth(3);
            dialog->choosePart(0);
            dialog->setFill(QColor(0xFF, 0xF0, 0xC0));
        });
        auto look = [&](jp::office::sheet::ChartPart part, int series) {
            jp::office::Chart now;
            if (!jp::readChartOf(*ed->doc(), *jp::selectedChart(ed), &now)) qFatal("no chart");
            return jp::office::sheet::partLookOf(now, part, series);
        };
        QCOMPARE(look(jp::office::sheet::ChartPart::Series, 1).fill.name(), QStringLiteral("#204060"));
        const int steps = ed->undoStack()->index();
        w.act(QStringLiteral("chartdesign.resetStyle"))->trigger();
        QCOMPARE(ed->undoStack()->index(), steps + 1);
        QCOMPARE(look(jp::office::sheet::ChartPart::Series, 1).fill, style);
        // (the color a chart of this program's own format gives back is the palette's, and a color)
        QVERIFY(style.isValid() && !c.styleBase.isValid() && !c.palette.isEmpty());
        QCOMPARE(style.name(), c.palette[1 % c.palette.size()].name());
        QVERIFY(!look(jp::office::sheet::ChartPart::Series, 1).line.set);
        QCOMPARE(look(jp::office::sheet::ChartPart::Area, 0).fill.name(), QStringLiteral("#fff0c0"));
        ed->undo();
        QCOMPARE(look(jp::office::sheet::ChartPart::Series, 1).fill.name(), QStringLiteral("#204060"));
        ed->redo();
        // a part that already matches its style is no step
        pickChartPart(box, 1);
        w.act(QStringLiteral("chartdesign.resetStyle"))->trigger();
        QCOMPARE(ed->undoStack()->index(), steps + 1);
        // saved through the window and opened again in another
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("c.jpub"));
        QVERIFY(w.saveTo(path));
        jp::MainWindow second;
        QVERIFY(second.openFile(path));
        jp::Editor *again = second.editor();
        QVERIFY(!chartsOn(*again->doc(), again->currentPage()).isEmpty());
        again->select(chartsOn(*again->doc(), again->currentPage()).first()->id);
        jp::office::Chart back;
        QVERIFY(jp::readChartOf(*again->doc(), *jp::selectedChart(again), &back));
        QCOMPARE(jp::office::sheet::partLookOf(back, jp::office::sheet::ChartPart::Area).fill.name(), QStringLiteral("#fff0c0"));
        QVERIFY(jp::office::sheet::partLookOf(back, jp::office::sheet::ChartPart::Series, 1).fill.name() != QStringLiteral("#204060"));
        QVERIFY(!jp::office::sheet::partLookOf(back, jp::office::sheet::ChartPart::Series, 1).line.set);
        second.ribbon()->showTab(second.ribbon()->tab(QStringLiteral("Chart Format")));
        QTRY_COMPARE(chartPartBoxOn(second.ribbon()->tab(QStringLiteral("Chart Format")))->count(), 4);
        ed->clearSelection();
        QTRY_COMPARE(box->count(), 0);
    }

    void jpubKeepsTheChart()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        auto d = publicationWithChart();
        const QByteArray part = chartsOn(*d).first()->chartXml;
        QString err;
        const QString path = dir.filePath(QStringLiteral("chart.jpub"));
        QVERIFY2(jp::savePublication(*d, path, QImage(), &err), qPrintable(err));
        auto back = jp::loadPublication(path, &err);
        QVERIFY2(back, qPrintable(err));
        const auto charts = chartsOn(*back);
        QCOMPARE(int(charts.size()), 1);
        QCOMPARE(charts[0]->chartXml, part);
        QCOMPARE(charts[0]->rect, QRectF(72, 72, 360, 216));
        QVERIFY(!charts[0]->imageId.isEmpty());
        const auto info = jp::chartInfoOfPicture(*back, *charts[0]);
        QVERIFY(info.valid);
        QCOMPARE(info.form, jp::office::sheet::ChartForm::BarStacked);
        QCOMPARE(info.data.categories, jp::office::sheet::sampleChartData().categories);
        QCOMPARE(info.data.seriesNames, jp::office::sheet::sampleChartData().seriesNames);
        QCOMPARE(info.data.values, jp::office::sheet::sampleChartData().values);
        QVERIFY(jp::changeChartOf(*back, *charts[0], [](jp::office::Chart &c) { return jp::office::sheet::chartSetTitle(c, true, QStringLiteral("Totals")); }));
        QCOMPARE(jp::chartInfoOfPicture(*back, *charts[0]).title, QStringLiteral("Totals"));
        // A file holds only so many charts as charts: the rest stay the pictures they were drawn as.
        for (int i = 0; i < jp::office::sheet::kMostChartsInFile; ++i) back->pages[0]->items.push_back(jp::makeChartObject(*back, jp::office::sheet::ChartForm::Pie, jp::office::sheet::sampleChartData(), QRectF(10, 10, 100, 60)));
        auto copy = jp::Document::blank(QSizeF(612, 792));
        copy->fromJson(back->toJson());
        QCOMPARE(int(chartsOn(*copy).size()), jp::office::sheet::kMostChartsInFile);
        QCOMPARE(int(copy->pages[0]->items.size()), jp::office::sheet::kMostChartsInFile + 1);
        QCOMPARE(copy->chartsKeptAsPictures, 1);
        // Opened as a file, it says so, and Insert > Chart stops at the limit with a message.
        const QString many = dir.filePath(QStringLiteral("many.jpub"));
        QVERIFY2(jp::savePublication(*back, many, QImage(), &err), qPrintable(err));
        jp::MainWindow w;
        QVERIFY(w.openFile(many));
        QVERIFY2(w.statusBar()->currentMessage().contains(QStringLiteral("kept as pictures only")), qPrintable(w.statusBar()->currentMessage()));
        QSignalSpy said(w.editor(), &jp::Editor::status);
        QVERIFY(jp::insertChart(w.editor(), jp::office::sheet::ChartForm::Column).isEmpty());
        QCOMPARE(said.size(), 1);
    }

    // A .pub file has no chart other programs would open as one: the chart is saved as a picture, and the report names it.
    void pubSavesAPictureAndNamesIt()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        auto d = publicationWithChart();
        QString err;
        const QString path = dir.filePath(QStringLiteral("chart.pub"));
        QVERIFY(jp::exportPublisher(*d, path, &err));
        QVERIFY2(err.contains(QStringLiteral("chart")) && err.contains(QStringLiteral("saved as pictures")), qPrintable(err));
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY(back);
        QVERIFY(chartsOn(*back).isEmpty());
        int pictures = 0;
        for (const jp::ItemPtr &it : back->pages[0]->items) pictures += it->type() == jp::ItemType::Picture;
        QCOMPARE(pictures, 1);
        QCOMPARE(back->images.size(), 1);
        QVERIFY(!back->images.first().image().isNull());   // (a picture, whatever kind of picture the format keeps)
    }

    // PDF output draws the chart as vector paths and text, not as an image.
    void pdfHoldsTheChartAsPaths()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        jp::MainWindow w;
        w.editor()->setDocument(publicationWithChart());
        const QString path = dir.filePath(QStringLiteral("chart.pdf"));
        jp::MainWindow::PdfSettings ps;
        ps.preset = jp::MainWindow::PdfSettings::HighQuality;
        QVERIFY(w.exportPdfTo(path, ps));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray pdf = f.readAll();
        QVERIFY(pdf.startsWith("%PDF-"));
        QVERIFY(!pdf.contains("/Subtype /Image"));
        const QByteArray content = pdfContent(pdf);
        QVERIFY(content.contains(" m\n"));   // drawn as paths: a move to, then lines and curves
        QVERIFY(content.contains(" l\n") || content.contains(" c\n"));
    }
};

JP_TEST_CLASS(PubChartTests)
#include "tst_pub_charts.moc"
