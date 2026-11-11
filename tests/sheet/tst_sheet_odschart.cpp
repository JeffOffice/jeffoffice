// Tests of the charts of a spreadsheet in the OpenDocument form (libs/office/sheet/odschart.h): each kind of chart written to an .ods (a frame with an embedded chart object, `Object N/content.xml`)
// and read back keeps its kind, series with their cell ranges, categories, titles, legend, axes, labels, and colors; a chart another program wrote is read; what a chart may be is bounded.
// Every file is built in memory.
#include "drawinghelpers.h"
#include "odshelpers.h"

#include "io/zip.h"
#include "office/chart.h"
#include "office/chart_write.h"
#include "office/package.h"
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

// A chart document of the standard's own elements: `plot` inside the plot area, `styles` as automatic styles, of the class, and `table` after the plot area.
QByteArray miniChart(const QString &cls, const QString &plot, const QString &styles = QString(), const QString &table = QString())
{
    return (QStringLiteral("<office:document-content xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\" xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" "
                           "xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\" xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\" xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\" "
                           "xmlns:chart=\"urn:oasis:names:tc:opendocument:xmlns:chart:1.0\"><office:automatic-styles>%3</office:automatic-styles><office:body><office:chart><chart:chart chart:class=\"%1\" svg:width=\"5cm\" svg:height=\"4cm\">"
                           "<chart:plot-area chart:style-name=\"p1\">%2</chart:plot-area>%4</chart:chart></office:chart></office:body></office:document-content>")
                .arg(cls, plot, styles, table)).toUtf8();
}

// An .ods with `frames` frames of charts that name Object 1, on sheets called `names`, and `inside` more shapes in each sheet.
QByteArray odsWithObject(const QByteArray &object, int frames, const QStringList &names = {QStringLiteral("S")}, const QString &inside = QString())
{
    QString shape;
    for (int i = 0; i < frames; ++i) shape += QStringLiteral("<draw:frame draw:name=\"C%1\" svg:width=\"1in\" svg:height=\"1in\" svg:x=\"0in\" svg:y=\"0in\"><draw:object xlink:href=\"./Object 1\"/></draw:frame>").arg(i);
    QString tables;
    for (const QString &n : names)
        tables += QStringLiteral("<table:table table:name=\"%1\"><table:shapes>%2%3</table:shapes><table:table-column table:number-columns-repeated=\"3\"/>"
                                 "<table:table-row><table:table-cell office:value-type=\"float\" office:value=\"7\"/></table:table-row></table:table>").arg(n, shape, inside);
    return buildOds(contentXml(tables), QString(), QByteArray(), {{QStringLiteral("Object 1/content.xml"), object}});
}

QString allNotes(const Saved &s) { return s.report.notKept.join(QLatin1Char('|')) + QLatin1Char('|') + s.report.trimmed.join(QLatin1Char('|')); }

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
            // charts past the most a sheet may have: the frames count, and the Object they name is read, and kept, once
            const int count = jp::office::sheet::odschart::kMaxChartsPerSheet + 20;
            Workbook wb;
            QString why;
            const qint64 reads = odsdraw::testHooks().pictureReads.load();
            QVERIFY2(openBytes(odsWithObject(chartWith(QStringLiteral("<chart:series chart:values-cell-range-address=\"S.A1:S.A3\"/>")), count), QStringLiteral("many.ods"), &wb, &why), qPrintable(why));
            Sheet s;
            QVERIFY2(wb.readSheet(0, &s), qPrintable(s.error));
            QCOMPARE(int(s.importedDrawing.anchors.size()), jp::office::sheet::odschart::kMaxChartsPerSheet);
            QCOMPARE(int(s.importedDrawing.charts.size()), 1);
            QCOMPARE(odsdraw::testHooks().pictureReads.load() - reads, qint64(1));
            QVERIFY(s.importedDrawing.notRead.join(QLatin1Char('|')).contains(QLatin1String("charts")));
        }
    }

    // The charts of a whole workbook are bounded too, whatever sheets they are on.
    void chartsPastTheWorkbookLimitAreNamed()
    {
        QStringList names;
        for (int i = 0; i < 12; ++i) names << QStringLiteral("S%1").arg(i);
        Workbook wb;
        QString why;
        QVERIFY2(openBytes(odsWithObject(miniChart(QStringLiteral("chart:bar"), QStringLiteral("<chart:series chart:values-cell-range-address=\"S0.A1:S0.A3\"/>")), 190, names), QStringLiteral("wide.ods"), &wb, &why), qPrintable(why));
        int anchors = 0;
        QString notes;
        for (int i = 0; i < 12; ++i) {
            Sheet s;
            QVERIFY2(wb.readSheet(i, &s), qPrintable(s.error));
            anchors += int(s.importedDrawing.anchors.size());
            notes += s.importedDrawing.notRead.join(QLatin1Char('|'));
        }
        QVERIFY2(anchors <= jp::office::sheet::odschart::kMaxChartsPerWorkbook, qPrintable(QString::number(anchors)));
        QVERIFY2(notes.contains(QStringLiteral("a workbook may have")), qPrintable(notes));
    }

    // A chart sheet has no counterpart in an .ods: the copy says so.
    void aChartSheetIsNamedInTheCopyReport()
    {
        PackageWriter w;
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        w.addPart(QStringLiteral("/xl/workbook.xml"), QByteArray("<?xml version=\"1.0\"?><workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
                                                                  "<sheets><sheet name=\"Data\" sheetId=\"1\" r:id=\"rId1\"/><sheet name=\"Big chart\" sheetId=\"2\" r:id=\"rId2\"/></sheets></workbook>"),
                  QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"));
        w.addRelationship(QStringLiteral("/"), rel + "officeDocument", QStringLiteral("/xl/workbook.xml"));
        w.addPart(QStringLiteral("/xl/worksheets/sheet1.xml"), dataSheet(), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"));
        w.addRelationship(QStringLiteral("/xl/workbook.xml"), rel + "worksheet", QStringLiteral("/xl/worksheets/sheet1.xml"));
        w.addPart(QStringLiteral("/xl/chartsheets/sheet1.xml"), QByteArray("<chartsheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"><drawing r:id=\"rId1\"/></chartsheet>"),
                  QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.chartsheet+xml"));
        w.addRelationship(QStringLiteral("/xl/workbook.xml"), rel + "chartsheet", QStringLiteral("/xl/chartsheets/sheet1.xml"));
        w.addPart(QStringLiteral("/xl/drawings/drawing1.xml"), drawingPart(absolute(0, 0, 9525 * 640, 9525 * 480, chartFrame(2, QStringLiteral("rId1")))), QStringLiteral("application/vnd.openxmlformats-officedocument.drawing+xml"));
        w.addRelationship(QStringLiteral("/xl/chartsheets/sheet1.xml"), rel + "drawing", QStringLiteral("/xl/drawings/drawing1.xml"));
        w.addPart(QStringLiteral("/xl/charts/chart1.xml"), chartPart(QStringLiteral("barChart"), twoSeries()), QStringLiteral("application/vnd.openxmlformats-officedocument.drawingml.chart+xml"));
        w.addRelationship(QStringLiteral("/xl/drawings/drawing1.xml"), rel + "chart", QStringLiteral("/xl/charts/chart1.xml"));
        Saved s;
        QString why;
        QVERIFY2(saveAsOds(w.finish(), &s, &why), qPrintable(why));
        QVERIFY2(allNotes(s).contains(QStringLiteral("chart sheet")), qPrintable(allNotes(s)));
    }

    // A chart in a place the reader does not look (inside a text box) is named, not hidden because the file holds chart documents.
    void aChartInsideATextBoxIsNamed()
    {
        const QString box = QStringLiteral("<draw:frame draw:name=\"Box\" svg:width=\"3in\" svg:height=\"2in\" svg:x=\"0in\" svg:y=\"0in\"><draw:text-box><draw:frame svg:width=\"1in\" svg:height=\"1in\"><draw:object xlink:href=\"./Object 1\"/></draw:frame></draw:text-box></draw:frame>");
        Workbook wb;
        QString why;
        QVERIFY2(openBytes(odsWithObject(miniChart(QStringLiteral("chart:bar"), QString()), 0, {QStringLiteral("S")}, box), QStringLiteral("nested.ods"), &wb, &why), qPrintable(why));
        Sheet s;
        QVERIFY2(wb.readSheet(0, &s), qPrintable(s.error));
        QVERIFY2(s.importedDrawing.notRead.join(QLatin1Char('|')).contains(QLatin1String("inside other objects")), qPrintable(s.importedDrawing.notRead.join(QLatin1Char('|'))));
    }

    // An object that holds a formula (math) is named as one, not as a chart that could not be read.
    void anEquationObjectIsNamedAsOne()
    {
        const QByteArray math = "<math:math xmlns:math=\"http://www.w3.org/1998/Math/MathML\"><math:mi>x</math:mi></math:math>";
        Workbook wb;
        QString why;
        QVERIFY2(openBytes(odsWithObject(math, 1), QStringLiteral("math.ods"), &wb, &why), qPrintable(why));
        Sheet s;
        QVERIFY2(wb.readSheet(0, &s), qPrintable(s.error));
        const QString notes = s.importedDrawing.notRead.join(QLatin1Char('|'));
        QVERIFY2(notes.contains(QLatin1String("equations")), qPrintable(notes));
        QVERIFY2(!notes.contains(QLatin1String("charts that could not")), qPrintable(notes));
    }

    // A sheet whose name looks like a cell (Q1) is quoted in the chart's references.
    void sheetNamesThatLookLikeCellsAreQuoted()
    {
        Back b;
        QString why;
        const QString series = QStringLiteral("<chart:series chart:values-cell-range-address=\"Q1.B2:Q1.B4\" chart:class=\"chart:bar\"/>");
        QVERIFY2(readBack(odsWithObject(miniChart(QStringLiteral("chart:bar"), series), 1, {QStringLiteral("Q1")}), &b, &why), qPrintable(why));
        QCOMPARE(b.charts.size(), 1);
        QCOMPARE(b.charts[0].plots[0].series[0].valueRef, QStringLiteral("'Q1'!$B$2:$B$4"));
        const DrawingObject &o = b.drawing.objects[0];
        QVERIFY(b.package.part(o.chartPart).contains("<c:f>'Q1'!$B$2:$B$4</c:f>"));
    }

    // A sheet called "Sheet1 (2)" (a copy of a sheet) keeps its link; whole columns and several ranges are written as the standard writes them.
    void rangesWithParenthesesColumnsAndSeveralAreConverted()
    {
        QVector<SeriesSpec> list = {SeriesSpec{QString(), QString(), QStringLiteral("'Sheet1 (2)'!$B$2:$B$4"), QStringLiteral("Copy"), {"Q1", "Q2", "Q3"}, {1, 2, 3}, QString()},
                                    SeriesSpec{QString(), QString(), QStringLiteral("Sheet1!$B:$B"), QStringLiteral("Column"), {"Q1", "Q2", "Q3"}, {1, 2, 3}, QString()},
                                    SeriesSpec{QString(), QString(), QStringLiteral("(Sheet1!$B$2:$B$3,Sheet1!$B$4)"), QStringLiteral("Several"), {"Q1", "Q2", "Q3"}, {1, 2, 3}, QString()}};
        Saved s;
        QString why;
        const QByteArray xlsx = bookWithDrawing({dataSheet(), dataSheet()}, drawingPart(twoCell(0, 6, 5, 20, chartFrame(2, QStringLiteral("rId1")))), {chartTarget(QStringLiteral("rId1"), 1, chartPart(QStringLiteral("barChart"), list))},
                                                {QStringLiteral("Sheet1"), QStringLiteral("Sheet1 (2)")});
        QVERIFY2(saveAsOds(xlsx, &s, &why), qPrintable(why));
        const QByteArray object = s.parts.value(QStringLiteral("Object 1/content.xml"));
        QVERIFY2(object.contains("chart:values-cell-range-address=\"'Sheet1 (2)'.B2:'Sheet1 (2)'.B4\""), object.left(3000).constData());
        QVERIFY2(object.contains("chart:values-cell-range-address=\"Sheet1.B1:Sheet1.B1048576\""), "whole column");
        QVERIFY2(object.contains("chart:values-cell-range-address=\"Sheet1.B2:Sheet1.B3 Sheet1.B4\""), "several ranges");
        Back b;
        QVERIFY2(readBack(s.ods, &b, &why), qPrintable(why));
        QCOMPARE(b.charts.size(), 1);
        QCOMPARE(b.charts[0].plots[0].series[0].valueRef, QStringLiteral("'Sheet1 (2)'!$B$2:$B$4"));
        QCOMPARE(b.charts[0].plots[0].series[2].valueRef, QStringLiteral("(Sheet1!$B$2:$B$3,Sheet1!$B$4)"));
    }

    // The first slice of a pie starts where the standard says: its angle is counter-clockwise from three o'clock (90 is twelve o'clock); the model's is clockwise from twelve.
    void aPieStartsWhereTheStandardSaysItDoes()
    {
        QByteArray part = chartPart(QStringLiteral("pieChart"), {twoSeries()[0]}, QStringLiteral("Slices"));
        part.replace("</c:pieChart>", "<c:firstSliceAng val=\"30\"/></c:pieChart>");
        Saved s;
        QString why;
        QVERIFY2(saveAsOds(sheetWithChart(twoCell(0, 6, 5, 20, chartFrame(2, QStringLiteral("rId1"))), part), &s, &why), qPrintable(why));
        QVERIFY2(s.parts.value(QStringLiteral("Object 1/content.xml")).contains("chart:angle-offset=\"60\""), "30 clockwise from the top is 60 counter-clockwise from the right");
        const QVector<QPair<QString, int>> cases = {{QStringLiteral("90"), 0}, {QStringLiteral("60"), 30}, {QStringLiteral("0"), 90}, {QStringLiteral("180"), 270}};
        for (const auto &c : cases) {
            Back b;
            const QString series = QStringLiteral("<chart:series chart:values-cell-range-address=\"S.A1:S.A3\" chart:class=\"chart:circle\"/>");
            const QString styles = QStringLiteral("<style:style style:name=\"p1\" style:family=\"chart\"><style:chart-properties chart:angle-offset=\"%1\"/></style:style>").arg(c.first);
            QVERIFY2(readBack(odsWithObject(miniChart(QStringLiteral("chart:circle"), series, styles), 1), &b, &why), qPrintable(why));
            QCOMPARE(b.charts.size(), 1);
            QCOMPARE(b.charts[0].plots[0].firstSliceAngle, c.second);
        }
    }

    // What the chart's own table repeats is kept, not cut to a few: a row repeated ten times is ten rows, a point color repeated a hundred times is a hundred points.
    void repeatedRowsCellsAndPointsAreKept()
    {
        const QString table = QStringLiteral("<table:table table:name=\"local-table\"><table:table-rows><table:table-row table:number-rows-repeated=\"10\"><table:table-cell office:value-type=\"float\" office:value=\"3\" table:number-columns-repeated=\"12\"/></table:table-row></table:table-rows></table:table>");
        const QString styles = QStringLiteral("<style:style style:name=\"d1\" style:family=\"chart\"><style:graphic-properties draw:fill-color=\"#123456\"/></style:style>");
        const QString series = QStringLiteral("<chart:series chart:values-cell-range-address=\"local-table.A1:local-table.A10\" chart:class=\"chart:bar\"><chart:data-point chart:style-name=\"d1\" chart:repeated=\"100\"/></chart:series>");
        Back b;
        QString why;
        QVERIFY2(readBack(odsWithObject(miniChart(QStringLiteral("chart:bar"), series, styles, table), 1), &b, &why), qPrintable(why));
        QCOMPARE(b.charts.size(), 1);
        const auto &sr = b.charts[0].plots[0].series[0];
        QCOMPARE(int(sr.values.size()), 10);
        QCOMPARE(int(sr.pointColors.size()), 100);
        // (a count of a thousand million is read as far as a series may go, and the cut is named)
        const QString huge = QStringLiteral("<table:table table:name=\"local-table\"><table:table-rows><table:table-row table:number-rows-repeated=\"1000000000\"><table:table-cell office:value-type=\"float\" office:value=\"3\"/></table:table-row></table:table-rows></table:table>");
        Back c;
        QVERIFY2(readBack(odsWithObject(miniChart(QStringLiteral("chart:bar"), QStringLiteral("<chart:series chart:values-cell-range-address=\"local-table.A1:local-table.A2000000\"/>"), QString(), huge), 1), &c, &why), qPrintable(why));
        QCOMPARE(c.charts.size(), 1);
        QVERIFY(c.charts[0].plots[0].series[0].values.size() <= jp::office::sheet::odschart::kMaxPoints);
        QVERIFY(c.charts[0].plots[0].series[0].values.size() > 1000);
        QVERIFY(c.sheet.importedDrawing.notRead.join(QLatin1Char('|')).contains(QLatin1String("points")));
    }

    // Series with the same number format share one data style; more different ones than a chart's document may hold are named.
    void numberFormatsAreSharedAndCounted()
    {
        auto many = [](int n, const std::function<QString(int)> &format) {
            QVector<SeriesSpec> v;
            for (int i = 0; i < n; ++i) v << SeriesSpec{QString(), QString(), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("S%1").arg(i), {"Q1", "Q2", "Q3"}, {1, 2, 3}, QString()};
            QByteArray part = chartPart(QStringLiteral("barChart"), v);
            int at = 0, i = 0;
            while ((at = int(part.indexOf("<c:cat>", at))) >= 0) {
                const QByteArray labels = "<c:dLbls><c:numFmt formatCode=\"" + format(i++).toHtmlEscaped().toUtf8() + "\" sourceLinked=\"0\"/><c:showLegendKey val=\"0\"/><c:showVal val=\"1\"/><c:showCatName val=\"0\"/><c:showSerName val=\"0\"/><c:showPercent val=\"0\"/><c:showBubbleSize val=\"0\"/></c:dLbls>";
                part.insert(at, labels);
                at += int(labels.size()) + 7;
            }
            return part;
        };
        {
            Saved s;
            QString why;
            QVERIFY2(saveAsOds(sheetWithChart(twoCell(0, 6, 5, 20, chartFrame(2, QStringLiteral("rId1"))), many(250, [](int) { return QStringLiteral("0.0%"); })), &s, &why), qPrintable(why));
            const QByteArray object = s.parts.value(QStringLiteral("Object 1/content.xml"));
            QCOMPARE(int(object.count("<number:percentage-style ")), 1);
            QVERIFY2(!allNotes(s).contains(QStringLiteral("number formats")), qPrintable(allNotes(s)));
        }
        {
            Saved s;
            QString why;
            QVERIFY2(saveAsOds(sheetWithChart(twoCell(0, 6, 5, 20, chartFrame(2, QStringLiteral("rId1"))), many(250, [](int i) { return QStringLiteral("0 \"n%1\"").arg(i); })), &s, &why), qPrintable(why));
            QVERIFY2(allNotes(s).contains(QStringLiteral("number formats")), qPrintable(allNotes(s)));
        }
    }

    // The points a chart holds are bounded in all: a chart of one long series and many short ones is written small, and one past the total for a chart is cut and named.
    void aChartOfManyPointsWritesABoundedDocument()
    {
        auto series = [](int longOnes, int longLength, int shortOnes) {
            QVector<SeriesSpec> v;
            for (int i = 0; i < longOnes; ++i) {
                SeriesSpec sp{QString(), QString(), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("Long%1").arg(i), {}, {}, QString()};
                for (int k = 0; k < longLength; ++k) sp.values << double(k % 97);
                v << sp;
            }
            for (int i = 0; i < shortOnes; ++i) v << SeriesSpec{QString(), QString(), QStringLiteral("Sheet1!$C$2:$C$4"), QStringLiteral("Short%1").arg(i), {}, {1, 2, 3}, QString()};
            return v;
        };
        {
            Saved s;
            QString why;
            QVERIFY2(saveAsOds(sheetWithChart(twoCell(0, 6, 5, 20, chartFrame(2, QStringLiteral("rId1"))), chartPart(QStringLiteral("lineChart"), series(1, jp::office::sheet::odschart::kMaxPoints, 254))), &s, &why), qPrintable(why));
            const qint64 size = s.parts.value(QStringLiteral("Object 1/content.xml")).size();
            QVERIFY2(size < (qint64(24) << 20), qPrintable(QString::number(size)));
        }
        {
            // (a model of more points than a chart may hold, made here: the part a file can hold is smaller than that)
            Chart model = jp::office::blankChart(jp::office::defaultTheme());
            jp::office::ChartPlot plot;
            plot.type = ChartType::Line;
            for (int i = 0; i < 5; ++i) {
                jp::office::ChartSeries sr;
                sr.name = QStringLiteral("S%1").arg(i);
                for (int k = 0; k < jp::office::sheet::odschart::kMaxPoints; ++k) sr.values << double(k % 97);
                plot.series.append(sr);
            }
            model.plots.append(plot);
            QStringList notes;
            const QByteArray document = jp::office::sheet::odschart::chartToOdf(model, 4000000, 3000000, &notes);
            QVERIFY2(notes.join(QLatin1Char('|')).contains(QStringLiteral("points past the 400000 a chart may have")), qPrintable(notes.join(QLatin1Char('|'))));
            QVERIFY(document.size() < (qint64(60) << 20));
            // (and what was written is read again as the points that were kept)
            QHash<QString, QString> none;
            jp::office::sheet::ods::StyleStore store;
            QString why;
            QVERIFY2(jp::office::sheet::odschart::parseStyles(document, Limits(), &store, &why), qPrintable(why));
            Chart back;
            QVERIFY2(jp::office::sheet::odschart::chartFromOdf(document, store, &none, &back, &why, nullptr), qPrintable(why));
            qint64 total = 0;
            QStringList sizes;
            for (const auto &sr : back.plots[0].series) {
                total += sr.values.size();
                sizes << QString::number(sr.values.size());
            }
            QVERIFY2(total == jp::office::sheet::odschart::kMaxPointsInChart, qPrintable(sizes.join(QLatin1Char(' '))));
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
