// JeffPub's tests of charts the person makes: Insert > Chart, the Chart tab's changes, and how they are saved (.jpub keeps the chart, .pub a picture of it).
#include "pubhelpers.h"
#include "app/chartedit.h"
#include "core/chartobject.h"
#include "office/chart_write.h"

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
        QCOMPARE(back->images.first().format, QStringLiteral("png"));
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
