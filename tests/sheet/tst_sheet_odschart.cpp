// Tests of the charts of a spreadsheet in the OpenDocument form (libs/office/sheet/odschart.h): each kind of chart written to an .ods (a frame with an embedded chart object, `Object N/content.xml`)
// and read back keeps its kind, series with their cell ranges, categories, titles, legend, axes, labels, and colors; a chart another program wrote is read; what a chart may be is bounded.
// Every file is built in memory.
#include "drawinghelpers.h"
#include "odshelpers.h"

#include "io/zip.h"
#include "office/chart.h"
#include "office/chart_write.h"
#include "sheet/drawing.h"
#include "sheet/odschart.h"
#include "sheet/odsdraw.h"
#include "sheet/oldsave.h"
#include "sheet/xlsxsave.h"

#include <QDomDocument>

namespace {

using namespace drawtest;
using jp::office::Chart;
using jp::office::ChartType;

QByteArray sheetWithChart(const QString &anchors, const QByteArray &chart)
{
    return bookWithDrawing({dataSheet()}, drawingPart(anchors), {chartTarget(QStringLiteral("rId1"), 1, chart)});
}

struct Saved {
    QByteArray ods;
    QMap<QString, QByteArray> parts;
    jp::office::sheet::CopyReport report;
};

bool saveAsOds(const QByteArray &xlsx, Saved *s, QString *why)
{
    Package package;
    Workbook wb;
    if (!package.readLazy(xlsx, why) || !wb.open(package, why)) return false;
    if (!saveOds(wb, &s->ods, &s->report, why)) return false;
    return jp::readZip(s->ods, s->parts);
}

// What an .ods comes to as the .xlsx a save of it makes: its package, and the charts and drawing objects in it.
struct Back {
    QByteArray xlsx;
    Package package;
    Workbook book;
    SheetDrawing drawing;
    QVector<Chart> charts;
    Sheet sheet;
};

bool readBack(const QByteArray &ods, Back *b, QString *why, const Limits &limits = Limits())
{
    Workbook wb;
    if (!openBytes(ods, QStringLiteral("c.ods"), &wb, why, limits)) return false;
    if (!wb.readSheet(0, &b->sheet)) {
        *why = b->sheet.error;
        return false;
    }
    if (!saveXlsx(wb, SaveChanges(), &b->xlsx, why)) return false;
    if (!b->package.readLazy(b->xlsx, why) || !b->book.open(b->package, why)) return false;
    const QString part = drawingPartOf(b->package, b->book.info().sheets.value(0).part);
    if (part.isEmpty()) {
        *why = QStringLiteral("no drawing");
        return true;
    }
    if (!readDrawing(b->package, part, &b->drawing, why)) return false;
    for (const DrawingObject &o : std::as_const(b->drawing.objects)) {
        if (o.kind != ObjectKind::Chart) continue;
        Chart c;
        if (!jp::office::readChart(b->package.part(o.chartPart), jp::office::defaultTheme(), jp::office::ColorMap(), &c, why)) return false;
        b->charts.append(c);
    }
    return true;
}

Chart modelOf(const QByteArray &part)
{
    Chart c;
    QString why;
    jp::office::readChart(part, jp::office::defaultTheme(), jp::office::ColorMap(), &c, &why);
    return c;
}

QVector<SeriesSpec> twoSeries()
{
    return {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {"Q1", "Q2", "Q3"}, {10, 20, 30}, QString()},
            SeriesSpec{QStringLiteral("Sheet1!$C$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$C$2:$C$4"), QStringLiteral("West"), {"Q1", "Q2", "Q3"}, {5, 10, 15}, QString()}};
}

// The chart of the kind through an .ods and back, compared with the model it began as.
void roundTrip(const QString &kind, const QString &title, QByteArray part = QByteArray())
{
    if (part.isEmpty()) part = chartPart(kind, twoSeries(), title);
    const QString anchors = twoCell(1, 5, 6, 20, chartFrame(2, QStringLiteral("rId1"), QStringLiteral("Sales")), QString(), 1000, 2000, 3000, 4000);
    Saved s;
    QString why;
    QVERIFY2(saveAsOds(sheetWithChart(anchors, part), &s, &why), qPrintable(why));
    const QByteArray content = s.parts.value(QStringLiteral("content.xml"));
    QVERIFY2(content.contains("<draw:object"), "no chart object in content.xml");
    QVERIFY2(s.parts.contains(QStringLiteral("Object 1/content.xml")), qPrintable(QStringList(s.parts.keys()).join(QLatin1Char('|'))));
    QVERIFY(s.parts.value(QStringLiteral("Object 1/content.xml")).contains("<chart:chart"));
    QVERIFY(s.parts.value(QStringLiteral("META-INF/manifest.xml")).contains("Object 1/content.xml"));
    QVERIFY(!s.report.notKept.contains(QStringLiteral("charts")));
    Back b;
    QVERIFY2(readBack(s.ods, &b, &why), qPrintable(why));
    QCOMPARE(b.charts.size(), 1);
    const Chart before = modelOf(part), &after = b.charts[0];
    QCOMPARE(int(after.type()), int(before.type()));
    QCOMPARE(after.plots.size(), before.plots.size());
    QCOMPARE(after.plots[0].series.size(), before.plots[0].series.size());
    QCOMPARE(after.title.text, before.title.text);
    QCOMPARE(after.legend.shown, before.legend.shown);
    QCOMPARE(after.legend.position, before.legend.position);
    for (int i = 0; i < before.plots[0].series.size(); ++i) {
        const auto &x = before.plots[0].series[i], &y = after.plots[0].series[i];
        QCOMPARE(y.name, x.name);
        QCOMPARE(y.nameRef, x.nameRef);
        QCOMPARE(y.valueRef, x.valueRef);
        QCOMPARE(y.categoryRef, x.categoryRef);
        QCOMPARE(y.values, x.values);
        QCOMPARE(y.categories, x.categories);
        if (before.type() == ChartType::Scatter) QCOMPARE(y.xRef, x.xRef);
    }
    QCOMPARE(b.drawing.objects.size(), 1);
    const DrawingAnchor &a = b.drawing.objects[0].anchor;
    QCOMPARE(int(a.kind), int(AnchorKind::TwoCell));
    QCOMPARE(int(a.from.col), 1);
    QCOMPARE(int(a.from.row), 5);
    QCOMPARE(int(a.to.col), 6);
    QCOMPARE(int(a.to.row), 20);
    QVERIFY(std::abs(a.from.colOff - 1000) <= 3);
    QVERIFY(std::abs(a.to.rowOff - 4000) <= 3);
    QCOMPARE(b.drawing.objects[0].name, QStringLiteral("Sales"));
}

QString textOfTitle(const Chart &c) { return c.title.text; }

} // namespace

class SheetOdsChartTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void aColumnChartSurvivesSaveAsOds() { roundTrip(QStringLiteral("barChart"), QStringLiteral("Sales by quarter")); }
    void aLineChartSurvivesSaveAsOds() { roundTrip(QStringLiteral("lineChart"), QStringLiteral("Trend")); }
    void anAreaChartSurvivesSaveAsOds() { roundTrip(QStringLiteral("areaChart"), QStringLiteral("Totals")); }
    void aPieChartSurvivesSaveAsOds()
    {
        const QVector<SeriesSpec> one = {twoSeries()[0]};
        roundTrip(QStringLiteral("pieChart"), QStringLiteral("Share"), chartPart(QStringLiteral("pieChart"), one, QStringLiteral("Share")));
    }
    void aScatterChartSurvivesSaveAsOds()
    {
        QVector<SeriesSpec> xy = twoSeries();
        for (SeriesSpec &s : xy) {
            s.xRef = QStringLiteral("Sheet1!$B$2:$B$4");
            s.categories = {"1", "2", "3"};
        }
        roundTrip(QStringLiteral("scatterChart"), QStringLiteral("Plot"), chartPart(QStringLiteral("scatterChart"), xy, QStringLiteral("Plot")));
    }
    void aDoughnutChartSurvivesSaveAsOds()
    {
        QByteArray part = chartPart(QStringLiteral("pieChart"), {twoSeries()[0]}, QStringLiteral("Ring"));
        part.replace("<c:pieChart>", "<c:doughnutChart>").replace("</c:pieChart>", "<c:holeSize val=\"60\"/></c:doughnutChart>");
        roundTrip(QStringLiteral("doughnutChart"), QStringLiteral("Ring"), part);
    }

    // A chart with a look: stacked horizontal bars, a series color and outline, titles on its axes, a scale with a least and greatest value, a number format, data labels, and a legend at the bottom.
    void aChartKeepsItsLookThroughAnOds()
    {
        QByteArray part = chartPart(QStringLiteral("barChart"), twoSeries(), QStringLiteral("Look"));
        part.replace("<c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>", "<c:barDir val=\"bar\"/><c:grouping val=\"stacked\"/>");
        part.replace("<c:order val=\"0\"/>", "<c:order val=\"0\"/>");
        part.replace("<c:cat>", "<c:spPr><a:solidFill><a:srgbClr val=\"3366CC\"/></a:solidFill><a:ln w=\"19050\"><a:solidFill><a:srgbClr val=\"112233\"/></a:solidFill></a:ln></c:spPr><c:dLbls><c:showLegendKey val=\"0\"/><c:showVal val=\"1\"/><c:showCatName val=\"0\"/><c:showSerName val=\"0\"/><c:showPercent val=\"0\"/><c:showBubbleSize val=\"0\"/></c:dLbls><c:cat>");
        part.replace("<c:legendPos val=\"r\"/>", "<c:legendPos val=\"b\"/>");
        part.replace("<c:axPos val=\"b\"/>", "<c:axPos val=\"b\"/><c:title><c:tx><c:rich><a:bodyPr/><a:p><a:r><a:t>Quarter</a:t></a:r></a:p></c:rich></c:tx></c:title>");
        part.replace("<c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"l\"/><c:majorGridlines/>",
                     "<c:scaling><c:orientation val=\"minMax\"/><c:max val=\"80\"/><c:min val=\"-5\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"l\"/><c:majorGridlines/>"
                     "<c:title><c:tx><c:rich><a:bodyPr/><a:p><a:r><a:t>Amount</a:t></a:r></a:p></c:rich></c:tx></c:title><c:numFmt formatCode=\"#,##0.00\" sourceLinked=\"0\"/>");
        Saved s;
        QString why;
        QVERIFY2(saveAsOds(sheetWithChart(twoCell(0, 6, 5, 20, chartFrame(2, QStringLiteral("rId1"))), part), &s, &why), qPrintable(why));
        Back b;
        QVERIFY2(readBack(s.ods, &b, &why), qPrintable(why));
        QCOMPARE(b.charts.size(), 1);
        const Chart before = modelOf(part), &after = b.charts[0];
        QVERIFY(before.plots[0].horizontal);
        QCOMPARE(after.plots[0].horizontal, true);
        QCOMPARE(int(after.plots[0].grouping), int(jp::office::ChartGrouping::Stacked));
        QCOMPARE(after.legend.position, QStringLiteral("b"));
        const auto &y = after.plots[0].series[0];
        QCOMPARE(y.color.name(), QStringLiteral("#3366cc"));
        QVERIFY(y.labels.value);
        QCOMPARE(y.outline.color.name(), QStringLiteral("#112233"));
        QVERIFY(std::abs(y.outline.width - 1.5) < 0.2);
        const jp::office::ChartAxis *value = nullptr, *category = nullptr;
        for (const auto &ax : after.axes) (ax.kind == jp::office::ChartAxis::Value ? value : category) = &ax;
        QVERIFY(value && category);
        QCOMPARE(value->title.text, QStringLiteral("Amount"));
        QCOMPARE(category->title.text, QStringLiteral("Quarter"));
        QVERIFY(!value->autoMax && !value->autoMin);
        QCOMPARE(value->max, 80.0);
        QCOMPARE(value->min, -5.0);
        QCOMPARE(value->format, QStringLiteral("#,##0.00"));
        QVERIFY(!value->formatLinked);
        QVERIFY(value->majorGridlines);
    }

    // Two charts on one sheet, and a chart beside a picture: each is its own object, in the order they are drawn.
    void severalChartsAndAPictureAreKept()
    {
        const QByteArray one = chartPart(QStringLiteral("barChart"), twoSeries(), QStringLiteral("First")), two = chartPart(QStringLiteral("lineChart"), twoSeries(), QStringLiteral("Second"));
        const QString anchors = twoCell(0, 6, 4, 14, chartFrame(2, QStringLiteral("rId1"), QStringLiteral("One"))) + twoCell(5, 6, 9, 14, chartFrame(3, QStringLiteral("rId2"), QStringLiteral("Two")))
                                + twoCell(0, 0, 2, 2, pictureElement(4, QStringLiteral("rId3"), QStringLiteral("Pic")));
        const QByteArray xlsx = bookWithDrawing({dataSheet()}, drawingPart(anchors), {chartTarget(QStringLiteral("rId1"), 1, one), chartTarget(QStringLiteral("rId2"), 2, two), imageTarget(QStringLiteral("rId3"), 1, pngBytes(4, 4, QColor(1, 2, 3)))});
        Saved s;
        QString why;
        QVERIFY2(saveAsOds(xlsx, &s, &why), qPrintable(why));
        QVERIFY(s.parts.contains(QStringLiteral("Object 1/content.xml")));
        QVERIFY(s.parts.contains(QStringLiteral("Object 2/content.xml")));
        Back b;
        QVERIFY2(readBack(s.ods, &b, &why), qPrintable(why));
        QCOMPARE(b.drawing.objects.size(), 3);
        QCOMPARE(int(b.drawing.objects[0].kind), int(ObjectKind::Chart));
        QCOMPARE(int(b.drawing.objects[1].kind), int(ObjectKind::Chart));
        QCOMPARE(int(b.drawing.objects[2].kind), int(ObjectKind::Picture));
        QCOMPARE(textOfTitle(b.charts[0]), QStringLiteral("First"));
        QCOMPARE(textOfTitle(b.charts[1]), QStringLiteral("Second"));
        QCOMPARE(int(b.charts[1].type()), int(ChartType::Line));
    }

    // What a chart has that an .ods chart does not is named in the copy's report: trendlines and error bars of a series.
    void whatIsNotKeptOfAChartIsNamed()
    {
        QByteArray part = chartPart(QStringLiteral("lineChart"), twoSeries(), QStringLiteral("Fitted"));
        part.replace("<c:cat>", "<c:trendline><c:trendlineType val=\"linear\"/></c:trendline><c:errBars><c:errBarType val=\"both\"/><c:errValType val=\"fixedVal\"/><c:val val=\"1\"/></c:errBars><c:cat>");
        Saved s;
        QString why;
        QVERIFY2(saveAsOds(sheetWithChart(twoCell(0, 6, 5, 20, chartFrame(2, QStringLiteral("rId1"))), part), &s, &why), qPrintable(why));
        const QString notes = s.report.notKept.join(QLatin1Char('|'));
        QVERIFY2(notes.contains(QLatin1String("trendlines")), qPrintable(notes));
        QVERIFY2(notes.contains(QLatin1String("error bars")), qPrintable(notes));
    }

    // A chart that another program wrote (the standard's own elements, a series' cells on a sheet with a quoted name, styles by name): a frame in a cell, and one between two cells.
    void readsAChartAnotherProgramWrote()
    {
        const QString chart = QStringLiteral(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?><office:document-content xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\" "
            "xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\" xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\" "
            "xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\" xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\" xmlns:chart=\"urn:oasis:names:tc:opendocument:xmlns:chart:1.0\" "
            "xmlns:number=\"urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0\" office:version=\"1.2\"><office:automatic-styles>"
            "<number:number-style style:name=\"N2\"><number:number number:decimal-places=\"2\" number:min-integer-digits=\"1\" number:grouping=\"true\"/></number:number-style>"
            "<style:style style:name=\"ch1\" style:family=\"chart\"><style:graphic-properties draw:stroke=\"none\" svg:stroke-color=\"#b3b3b3\" draw:fill-color=\"#ffffff\"/></style:style>"
            "<style:style style:name=\"ch2\" style:family=\"chart\"><style:text-properties fo:font-size=\"13pt\" fo:font-weight=\"bold\"/></style:style>"
            "<style:style style:name=\"ch3\" style:family=\"chart\"><style:chart-properties chart:vertical=\"false\" chart:stacked=\"false\" chart:gap-width=\"100\" chart:overlap=\"-10\"/></style:style>"
            "<style:style style:name=\"ch4\" style:family=\"chart\" style:data-style-name=\"N2\"><style:chart-properties chart:minimum=\"0\" chart:maximum=\"100\" chart:interval-major=\"20\" chart:logarithmic=\"false\" chart:link-data-style-to-source=\"false\"/></style:style>"
            "<style:style style:name=\"ch5\" style:family=\"chart\"><style:graphic-properties draw:stroke=\"solid\" svg:stroke-width=\"0.05cm\" svg:stroke-color=\"#00ff00\" draw:fill=\"solid\" draw:fill-color=\"#ff3300\"/>"
            "<style:chart-properties chart:data-label-number=\"value\"/></style:style>"
            "</office:automatic-styles><office:body><office:chart>"
            "<chart:chart svg:width=\"12cm\" svg:height=\"8cm\" chart:class=\"chart:bar\" chart:style-name=\"ch1\">"
            "<chart:title svg:x=\"4cm\" svg:y=\"0.3cm\" chart:style-name=\"ch2\"><text:p>Revenue</text:p></chart:title>"
            "<chart:legend chart:legend-position=\"top\" style:legend-expansion=\"high\"/>"
            "<chart:plot-area chart:style-name=\"ch3\" table:cell-range-address=\"'My Data'.A1:'My Data'.B4\" chart:data-source-has-labels=\"both\">"
            "<chart:axis chart:dimension=\"x\" chart:name=\"primary-x\"><chart:title><text:p>Month</text:p></chart:title><chart:categories table:cell-range-address=\"'My Data'.A2:'My Data'.A4\"/></chart:axis>"
            "<chart:axis chart:dimension=\"y\" chart:name=\"primary-y\" chart:style-name=\"ch4\"><chart:grid chart:class=\"major\"/></chart:axis>"
            "<chart:series chart:values-cell-range-address=\"'My Data'.B2:'My Data'.B4\" chart:label-cell-address=\"'My Data'.B1\" chart:class=\"chart:bar\" chart:style-name=\"ch5\"><chart:data-point chart:repeated=\"3\"/></chart:series>"
            "<chart:wall chart:style-name=\"ch1\"/></chart:plot-area>"
            "<table:table table:name=\"local-table\"><table:table-header-columns><table:table-column/></table:table-header-columns><table:table-columns><table:table-column/></table:table-columns>"
            "<table:table-header-rows><table:table-row><table:table-cell/><table:table-cell office:value-type=\"string\"><text:p>Sales</text:p></table:table-cell></table:table-row></table:table-header-rows>"
            "<table:table-rows>"
            "<table:table-row><table:table-cell office:value-type=\"string\"><text:p>Jan</text:p></table:table-cell><table:table-cell office:value-type=\"float\" office:value=\"10\"/></table:table-row>"
            "<table:table-row><table:table-cell office:value-type=\"string\"><text:p>Feb</text:p></table:table-cell><table:table-cell office:value-type=\"float\" office:value=\"40\"/></table:table-row>"
            "<table:table-row><table:table-cell office:value-type=\"string\"><text:p>Mar</text:p></table:table-cell><table:table-cell office:value-type=\"float\" office:value=\"25\"/></table:table-row>"
            "</table:table-rows></table:table></chart:chart></office:chart></office:body></office:document-content>");
        const QString shapes = QStringLiteral("<draw:frame draw:name=\"Rev\" draw:z-index=\"0\" svg:width=\"3in\" svg:height=\"2in\" svg:x=\"1in\" svg:y=\"0.5in\" table:end-cell-address=\"'My Data'.D8\" table:end-x=\"0.2in\" table:end-y=\"0.1in\">"
                                              "<draw:object xlink:href=\"./Object 1\" xlink:type=\"simple\" xlink:show=\"embed\" xlink:actuate=\"onLoad\"/><svg:title>Revenue chart</svg:title><svg:desc>Bars by month</svg:desc></draw:frame>");
        const QString body = QStringLiteral("<table:table table:name=\"My Data\"><table:shapes>%1</table:shapes><table:table-column table:number-columns-repeated=\"10\"/>"
                                            "<table:table-row table:number-rows-repeated=\"10\"><table:table-cell table:number-columns-repeated=\"10\"/></table:table-row></table:table>").arg(shapes);
        const QByteArray ods = buildOds(contentXml(body), QString(), QByteArray(), {{QStringLiteral("Object 1/content.xml"), chart.toUtf8()}});
        Workbook probe;
        QString why;
        QVERIFY2(openBytes(ods, QStringLiteral("lo.ods"), &probe, &why), qPrintable(why));
        QVERIFY2(!probe.skippedFeatures().contains(QStringLiteral("charts")), qPrintable(probe.skippedFeatures().join(QLatin1Char('|'))));
        Back b;
        QVERIFY2(readBack(ods, &b, &why), qPrintable(why));
        QCOMPARE(b.charts.size(), 1);
        const Chart &c = b.charts[0];
        QCOMPARE(int(c.type()), int(ChartType::Bar));
        QCOMPARE(c.title.text, QStringLiteral("Revenue"));
        QVERIFY(c.legend.shown);
        QCOMPARE(c.legend.position, QStringLiteral("t"));
        QCOMPARE(c.plots[0].gapWidth, 100);
        QCOMPARE(c.plots[0].overlap, -10);
        QCOMPARE(c.plots[0].series.size(), 1);
        const auto &sr = c.plots[0].series[0];
        QCOMPARE(sr.valueRef, QStringLiteral("'My Data'!$B$2:$B$4"));
        QCOMPARE(sr.categoryRef, QStringLiteral("'My Data'!$A$2:$A$4"));
        QCOMPARE(sr.nameRef, QStringLiteral("'My Data'!$B$1"));
        QCOMPARE(sr.name, QStringLiteral("Sales"));
        QCOMPARE(sr.values, (QVector<double>{10, 40, 25}));
        QCOMPARE(sr.categories, (QStringList{"Jan", "Feb", "Mar"}));
        QCOMPARE(sr.color.name(), QStringLiteral("#ff3300"));
        QVERIFY(sr.labels.value);
        const jp::office::ChartAxis *value = nullptr, *category = nullptr;
        for (const auto &ax : c.axes) (ax.kind == jp::office::ChartAxis::Value ? value : category) = &ax;
        QVERIFY(value && category);
        QCOMPARE(category->title.text, QStringLiteral("Month"));
        QCOMPARE(value->min, 0.0);
        QCOMPARE(value->max, 100.0);
        QCOMPARE(value->majorUnit, 20.0);
        QCOMPARE(value->format, QStringLiteral("#,##0.00"));
        QCOMPARE(b.drawing.objects.size(), 1);
        QCOMPARE(b.drawing.objects[0].name, QStringLiteral("Rev"));
        QCOMPARE(int(b.drawing.objects[0].anchor.kind), int(AnchorKind::TwoCell));
        QCOMPARE(int(b.drawing.objects[0].anchor.from.col), 1);
        QCOMPARE(int(b.drawing.objects[0].anchor.to.col), 3);
        QCOMPARE(int(b.drawing.objects[0].anchor.to.row), 7);
    }

    // A chart's cells name a sheet by the name the document gave it, and a sheet whose name the .xlsx form does not allow is renamed when the file is opened: the chart follows it.
    void aChartFollowsASheetRenamedOnOpen()
    {
        const QString chart = QStringLiteral(
            "<office:document-content xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\" "
            "xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\" xmlns:chart=\"urn:oasis:names:tc:opendocument:xmlns:chart:1.0\"><office:body><office:chart>"
            "<chart:chart chart:class=\"chart:line\" svg:width=\"5cm\" svg:height=\"4cm\"><chart:plot-area><chart:axis chart:dimension=\"x\" chart:name=\"primary-x\"><chart:categories table:cell-range-address=\"$'Q1:Q2'.$A$2:.$A$4\"/></chart:axis>"
            "<chart:series chart:values-cell-range-address=\"'Q1:Q2'.B2:'Q1:Q2'.B4\" chart:label-cell-address=\"'Q1:Q2'.B1\" chart:class=\"chart:line\"/></chart:plot-area></chart:chart></office:chart></office:body></office:document-content>");
        const QString shapes = QStringLiteral("<draw:frame draw:name=\"C\" svg:width=\"3in\" svg:height=\"2in\" svg:x=\"0in\" svg:y=\"0in\"><draw:object xlink:href=\"./Object 1\"/></draw:frame>");
        const QString body = QStringLiteral("<table:table table:name=\"Q1:Q2\"><table:shapes>%1</table:shapes><table:table-column table:number-columns-repeated=\"3\"/>"
                                            "<table:table-row><table:table-cell office:value-type=\"float\" office:value=\"7\"/></table:table-row></table:table>").arg(shapes);
        Back b;
        QString why;
        QVERIFY2(readBack(buildOds(contentXml(body), QString(), QByteArray(), {{QStringLiteral("Object 1/content.xml"), chart.toUtf8()}}), &b, &why), qPrintable(why));
        QCOMPARE(b.charts.size(), 1);
        QCOMPARE(b.book.info().sheets.value(0).name, QStringLiteral("Q1_Q2"));
        const auto &sr = b.charts[0].plots[0].series[0];
        QCOMPARE(sr.valueRef, QStringLiteral("Q1_Q2!$B$2:$B$4"));
        QCOMPARE(sr.categoryRef, QStringLiteral("Q1_Q2!$A$2:$A$4"));
        QCOMPARE(sr.nameRef, QStringLiteral("Q1_Q2!$B$1"));
    }

    // Hostile: a chart of more series than a chart may have, of more points than a series may, inside a part larger than a chart part may be, or not XML at all, is capped or left out and named; the
    // cells of the sheet are read, and nothing crashes.
    void hugeAndBrokenChartsAreCappedNotCrashed()
    {
        auto odsWith = [](const QByteArray &chart) {
            const QString shapes = QStringLiteral("<draw:frame draw:name=\"C\" svg:width=\"3in\" svg:height=\"2in\" svg:x=\"0in\" svg:y=\"0in\"><draw:object xlink:href=\"./Object 1\"/></draw:frame>");
            const QString body = QStringLiteral("<table:table table:name=\"S\"><table:shapes>%1</table:shapes><table:table-column table:number-columns-repeated=\"3\"/>"
                                                "<table:table-row><table:table-cell office:value-type=\"float\" office:value=\"7\"/></table:table-row></table:table>").arg(shapes);
            return buildOds(contentXml(body), QString(), QByteArray(), {{QStringLiteral("Object 1/content.xml"), chart}});
        };
        auto chartWith = [](const QString &plot) {
            return (QStringLiteral("<office:document-content xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" "
                                   "xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\" xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\" xmlns:chart=\"urn:oasis:names:tc:opendocument:xmlns:chart:1.0\">"
                                   "<office:body><office:chart><chart:chart chart:class=\"chart:line\" svg:width=\"5cm\" svg:height=\"4cm\"><chart:plot-area>%1</chart:plot-area></chart:chart></office:chart></office:body></office:document-content>")
                        .arg(plot)).toUtf8();
        };
        {
            // series past the most a chart has
            QString series;
            for (int i = 0; i < jp::office::sheet::odschart::kMaxSeries + 50; ++i) series += QStringLiteral("<chart:series chart:values-cell-range-address=\"S.A1:S.A3\" chart:class=\"chart:line\"/>");
            Back b;
            QString why;
            QVERIFY2(readBack(odsWith(chartWith(series)), &b, &why), qPrintable(why));
            QCOMPARE(b.charts.size(), 1);
            QCOMPARE(int(b.charts[0].plots[0].series.size()), jp::office::sheet::odschart::kMaxSeries);
            QVERIFY(b.sheet.importedDrawing.notRead.join(QLatin1Char('|')).contains(QLatin1String("series")));
        }
        {
            // points past the most a series has, in the chart's own table
            QString rows;
            const int points = jp::office::sheet::odschart::kMaxPoints + 500;
            for (int i = 0; i < points; ++i) rows += QStringLiteral("<table:table-row><table:table-cell office:value-type=\"float\" office:value=\"%1\"/></table:table-row>").arg(i);
            const QString table = QStringLiteral("<table:table table:name=\"local-table\"><table:table-rows>%1</table:table-rows></table:table>").arg(rows);
            QByteArray chart = chartWith(QStringLiteral("<chart:series chart:values-cell-range-address=\"local-table.A1:local-table.A%1\" chart:class=\"chart:line\"/>").arg(points));
            chart.replace("</chart:chart>", (table + QStringLiteral("</chart:chart>")).toUtf8());
            Back b;
            QString why;
            QVERIFY2(readBack(odsWith(chart), &b, &why), qPrintable(why));
            QCOMPARE(b.charts.size(), 1);
            QVERIFY(b.charts[0].plots[0].series[0].values.size() <= jp::office::sheet::odschart::kMaxPoints);
            QVERIFY(b.sheet.importedDrawing.notRead.join(QLatin1Char('|')).contains(QLatin1String("points")));
        }
        {
            // a part larger than a chart part may be is not read
            QByteArray huge = chartWith(QString());
            huge.replace("</chart:chart>", QByteArray(int(jp::office::sheet::odschart::kMaxPartBytes) + 1024, ' ') + "</chart:chart>");
            Back b;
            QString why;
            QVERIFY2(readBack(odsWith(huge), &b, &why), qPrintable(why));
            QCOMPARE(b.charts.size(), 0);
            QVERIFY(!b.sheet.importedDrawing.notRead.isEmpty());
            QCOMPARE(b.sheet.cells.cellCount(), qint64(1));
        }
        {
            // a part that is not XML, and one that is XML and not a chart
            for (const QByteArray &bad : {QByteArray("<<<not xml"), QByteArray("<office:document-content xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\"><office:body/></office:document-content>")}) {
                Back b;
                QString why;
                QVERIFY2(readBack(odsWith(bad), &b, &why), qPrintable(why));
                QCOMPARE(b.charts.size(), 0);
                QVERIFY(!b.sheet.importedDrawing.notRead.isEmpty());
            }
        }
        {
            // charts past the most a sheet may have
            QString shapes;
            const int count = jp::office::sheet::odschart::kMaxChartsPerSheet + 20;
            for (int i = 0; i < count; ++i)
                shapes += QStringLiteral("<draw:frame draw:name=\"C%1\" svg:width=\"1in\" svg:height=\"1in\" svg:x=\"0in\" svg:y=\"0in\"><draw:object xlink:href=\"./Object 1\"/></draw:frame>").arg(i);
            const QString body = QStringLiteral("<table:table table:name=\"S\"><table:shapes>%1</table:shapes><table:table-column/><table:table-row><table:table-cell/></table:table-row></table:table>").arg(shapes);
            const QByteArray ods = buildOds(contentXml(body), QString(), QByteArray(), {{QStringLiteral("Object 1/content.xml"), chartWith(QStringLiteral("<chart:series chart:values-cell-range-address=\"S.A1:S.A3\"/>"))}});
            Workbook wb;
            QString why;
            QVERIFY2(openBytes(ods, QStringLiteral("many.ods"), &wb, &why), qPrintable(why));
            Sheet s;
            QVERIFY2(wb.readSheet(0, &s), qPrintable(s.error));
            QCOMPARE(int(s.importedDrawing.charts.size()), jp::office::sheet::odschart::kMaxChartsPerSheet);
            QVERIFY(s.importedDrawing.notRead.join(QLatin1Char('|')).contains(QLatin1String("charts")));
        }
    }

    // The save of a chart that is itself past the limits writes what a chart may be, and says so.
    void aSaveWritesNoMoreThanTheChartLimits()
    {
        QVector<SeriesSpec> many;
        for (int i = 0; i < jp::office::sheet::odschart::kMaxSeries + 30; ++i)
            many << SeriesSpec{QString(), QString(), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("S%1").arg(i), {"Q1", "Q2", "Q3"}, {1, 2, 3}, QString()};
        Saved s;
        QString why;
        QVERIFY2(saveAsOds(sheetWithChart(twoCell(0, 6, 5, 20, chartFrame(2, QStringLiteral("rId1"))), chartPart(QStringLiteral("barChart"), many)), &s, &why), qPrintable(why));
        QVERIFY2(s.report.notKept.join(QLatin1Char('|')).contains(QLatin1String("series")), qPrintable(s.report.notKept.join(QLatin1Char('|'))));
        Back b;
        QVERIFY2(readBack(s.ods, &b, &why), qPrintable(why));
        QVERIFY2(b.charts.size() == 1, qPrintable(b.sheet.importedDrawing.notRead.join(QLatin1Char('|')) + QLatin1String(" / ") + why));
        QCOMPARE(int(b.charts[0].plots[0].series.size()), jp::office::sheet::odschart::kMaxSeries);
    }
};

JP_TEST_CLASS(SheetOdsChartTests)
#include "tst_sheet_odschart.moc"
