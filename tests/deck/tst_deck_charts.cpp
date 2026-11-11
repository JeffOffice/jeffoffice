// JeffDeck's tests of charts the person makes: Insert > Chart, the Chart tab's changes, and how they are saved.
#include "deckhelpers.h"
#include "app/chartedit.h"
#include "core/chartobject.h"
#include "office/chart_write.h"
#include "office/package.h"

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
        big.plots[0].series.resize(jp::kMostChartObjectSeries + 1, big.plots[0].series[0]);
        PictureItem wide = *one;
        wide.chartXml = office::writeChartPart(big);
        QVERIFY(!jp::readChartOf(*copy, wide, nullptr));
    }
};

JP_TEST_CLASS(DeckChartTests)
#include "tst_deck_charts.moc"
