// JeffDeck's tests of charts the person makes: Insert > Chart, the Chart tab's changes, and how they are saved.
#include "deckhelpers.h"
#include "app/chartedit.h"
#include "core/chartobject.h"
#include "office/chart_write.h"
#include "office/package.h"
#include "core/items.h"

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
