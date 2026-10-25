// Tests of libs/office: writing charts (office/chart_write.h): a chart written and read again is the chart; a part with more in it than the model holds is not written over.
#include "officehelpers.h"

#include "office/chart_write.h"

#include <QDomDocument>

using namespace jp::office;

namespace {

ChartSeries seriesOf(const QString &name, const QString &nameRef, const QString &catRef, const QString &valRef, const QStringList &cats, const QVector<double> &values, const QColor &color)
{
    ChartSeries s;
    s.name = name;
    s.nameRef = nameRef;
    s.categoryRef = catRef;
    s.valueRef = valRef;
    s.categories = cats;
    s.values = values;
    s.color = color;
    s.colorFromFile = true;
    return s;
}

// Columns with a line over them, the line against a second value axis, with a title, axis titles, a legend at the bottom, and data labels on the line.
Chart comboChart()
{
    Chart c = blankChart(defaultTheme());
    ChartPlot bars;
    bars.type = ChartType::Bar;
    bars.axisIds[0] = 111;
    bars.axisIds[1] = 222;
    bars.gapWidth = 80;
    bars.series << seriesOf(QStringLiteral("East"), QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), {"Q1", "Q2", "Q3"}, {10, 20, 30}, QColor("#4472C4"));
    bars.series << seriesOf(QStringLiteral("West"), QStringLiteral("Sheet1!$C$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$C$2:$C$4"), {"Q1", "Q2", "Q3"}, {5, 10, 15}, QColor("#ED7D31"));
    ChartPlot line;
    line.type = ChartType::Line;
    line.grouping = ChartGrouping::Standard;
    line.axisIds[0] = 333;
    line.axisIds[1] = 444;
    ChartSeries total = seriesOf(QStringLiteral("Total"), QString(), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$D$2:$D$4"), {"Q1", "Q2", "Q3"}, {15, 30, 45}, QColor("#70AD47"));
    total.labels.value = true;
    total.labels.position = QStringLiteral("t");
    total.smooth = true;
    line.series << total;
    c.plots << bars << line;
    ChartAxis cat, val, cat2, val2;
    cat.kind = ChartAxis::Category;
    cat.id = 111;
    cat.crossId = 222;
    cat.title.present = true;
    cat.title.text = QStringLiteral("Quarter");
    val.kind = ChartAxis::Value;
    val.id = 222;
    val.crossId = 111;
    val.position = QStringLiteral("l");
    val.majorGridlines = true;
    val.autoMax = false;
    val.max = 50;
    val.title.present = true;
    val.title.text = QStringLiteral("Units");
    cat2 = cat;
    cat2.id = 333;
    cat2.crossId = 444;
    cat2.deleted = true;
    cat2.title = ChartTitle();
    val2 = val;
    val2.id = 444;
    val2.crossId = 333;
    val2.position = QStringLiteral("r");
    val2.crosses = QStringLiteral("max");
    val2.majorGridlines = false;
    val2.autoMax = true;
    val2.title = ChartTitle();
    c.axes << cat << val << cat2 << val2;
    c.title.present = true;
    c.title.text = QStringLiteral("Sales & costs <2026>");
    c.legend.shown = true;
    c.legend.position = QStringLiteral("b");
    c.blanksAs = QStringLiteral("zero");
    c.plotVisibleOnly = false;
    return c;
}

} // namespace

class OfficeChartWriteTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    void aWrittenChartReadsBackAsTheChart()
    {
        const Chart c = comboChart();
        const QByteArray xml = writeChartPart(c);
        Chart r;
        QString err;
        QVERIFY2(readChart(xml, defaultTheme(), ColorMap(), &r, &err), qPrintable(err));
        QCOMPARE(int(r.plots.size()), 2);
        QCOMPARE(int(r.plots[0].type), int(ChartType::Bar));
        QCOMPARE(int(r.plots[1].type), int(ChartType::Line));
        QCOMPARE(r.plots[0].gapWidth, 80);
        QCOMPARE(r.plots[0].series.size(), 2);
        QCOMPARE(r.plots[0].series[1].name, QStringLiteral("West"));
        QCOMPARE(r.plots[0].series[1].nameRef, QStringLiteral("Sheet1!$C$1"));
        QCOMPARE(r.plots[0].series[1].categoryRef, QStringLiteral("Sheet1!$A$2:$A$4"));
        QCOMPARE(r.plots[0].series[1].valueRef, QStringLiteral("Sheet1!$C$2:$C$4"));
        QCOMPARE(r.plots[0].series[1].values, (QVector<double>{5, 10, 15}));
        QCOMPARE(r.plots[0].series[0].categories, QStringList({"Q1", "Q2", "Q3"}));
        QCOMPARE(r.plots[0].series[0].color.name(), QStringLiteral("#4472c4"));
        QVERIFY(r.plots[1].series[0].smooth);
        QVERIFY(r.plots[1].series[0].labels.value);
        QCOMPARE(r.plots[1].series[0].labels.position, QStringLiteral("t"));
        QVERIFY(r.plots[1].series[0].nameRef.isEmpty());
        QCOMPARE(r.plots[1].series[0].name, QStringLiteral("Total"));
        // axes, with the second pair of them the line is drawn against
        QCOMPARE(r.axes.size(), 4);
        QCOMPARE(r.plots[1].axisIds[1], qint64(444));
        QVERIFY(r.axis(444) && r.axis(444)->crosses == QLatin1String("max"));
        QVERIFY(r.axis(333)->deleted);
        QVERIFY(r.axis(222)->majorGridlines);
        QVERIFY(!r.axis(222)->autoMax);
        QCOMPARE(r.axis(222)->max, 50.0);
        QCOMPARE(r.axis(222)->title.text, QStringLiteral("Units"));
        QCOMPARE(r.axis(111)->title.text, QStringLiteral("Quarter"));
        // the chart's own words, with what XML cannot hold in a text set right
        QCOMPARE(r.title.text, QStringLiteral("Sales & costs <2026>"));
        QVERIFY(r.legend.shown);
        QCOMPARE(r.legend.position, QStringLiteral("b"));
        QCOMPARE(r.blanksAs, QStringLiteral("zero"));
        QVERIFY(!r.plotVisibleOnly);
    }

    void aChartMadeOfTheModelIsDrawnAsTheModelIs()
    {
        // what a person sees of a chart made here is what the reader makes of the part it is saved as
        const Chart c = comboChart();
        Chart r;
        QVERIFY(readChart(writeChartPart(c), defaultTheme(), ColorMap(), &r));
        QCOMPARE(chartSvg(r, QSizeF(480, 288)), chartSvg(c, QSizeF(480, 288)));
    }

    void everyKindOfChartIsWrittenAndReadBack()
    {
        const ChartType kinds[] = {ChartType::Bar, ChartType::Line, ChartType::Pie, ChartType::Doughnut, ChartType::Area, ChartType::Scatter};
        for (const ChartType kind : kinds) {
            Chart c = blankChart(defaultTheme());
            ChartPlot p;
            p.type = kind;
            p.grouping = kind == ChartType::Bar ? ChartGrouping::Clustered : ChartGrouping::Standard;
            if (kind != ChartType::Pie && kind != ChartType::Doughnut) {
                p.axisIds[0] = 1;
                p.axisIds[1] = 2;
                ChartAxis a, b;
                a.id = 1;
                a.crossId = 2;
                a.kind = kind == ChartType::Scatter ? ChartAxis::Value : ChartAxis::Category;
                b.kind = ChartAxis::Value;
                b.id = 2;
                b.crossId = 1;
                b.position = QStringLiteral("l");
                c.axes << a << b;
            }
            ChartSeries s = seriesOf(QStringLiteral("S"), QString(), QStringLiteral("Sheet1!$A$2:$A$3"), QStringLiteral("Sheet1!$B$2:$B$3"), {"a", "b"}, {1, 2}, QColor("#112233"));
            if (kind == ChartType::Scatter) {
                s.categories.clear();
                s.xRef = QStringLiteral("Sheet1!$A$2:$A$3");
                s.categoryRef.clear();
                s.xValues = {1, 2};
            }
            p.series << s;
            c.plots << p;
            Chart r;
            QString err;
            QVERIFY2(readChart(writeChartPart(c), defaultTheme(), ColorMap(), &r, &err), qPrintable(err));
            QCOMPARE(int(r.plots.size()), 1);
            QCOMPARE(int(r.plots[0].type), int(kind));
            QCOMPARE(r.plots[0].series[0].values, (QVector<double>{1, 2}));
            QCOMPARE(r.plots[0].series[0].valueRef, QStringLiteral("Sheet1!$B$2:$B$3"));
            if (kind == ChartType::Scatter) QCOMPARE(r.plots[0].series[0].xRef, QStringLiteral("Sheet1!$A$2:$A$3"));
            else QCOMPARE(r.plots[0].series[0].categoryRef, QStringLiteral("Sheet1!$A$2:$A$3"));
            QString why;
            QVERIFY2(chartIsFullyModeled(writeChartPart(c), &why), qPrintable(why));
        }
    }

    void aPartWithMoreInItThanTheModelHoldsIsNotWrittenOver()
    {
        const QByteArray ours = writeChartPart(comboChart());
        QString why;
        QVERIFY2(chartIsFullyModeled(ours, &why), qPrintable(why));
        // a gradient fill, a trend line, and a three-dimensional chart are drawn another way, or not at all, by a chart written from the model
        QByteArray gradient = ours;
        gradient.replace("<a:solidFill><a:srgbClr val=\"4472C4\"/></a:solidFill>", "<a:gradFill><a:gsLst><a:gs pos=\"0\"><a:srgbClr val=\"4472C4\"/></a:gs></a:gsLst></a:gradFill>");
        QVERIFY(gradient != ours);
        QVERIFY(!chartIsFullyModeled(gradient, &why));
        QCOMPARE(why, QStringLiteral("gradFill"));
        QByteArray trend = ours;
        trend.replace("<c:cat>", "<c:trendline><c:trendlineType val=\"linear\"/></c:trendline><c:cat>");
        QVERIFY(!chartIsFullyModeled(trend, &why));
        QCOMPARE(why, QStringLiteral("trendline"));
        QByteArray threeD = ours;
        threeD.replace("<c:barChart>", "<c:bar3DChart>").replace("</c:barChart>", "</c:bar3DChart>");
        QVERIFY(!chartIsFullyModeled(threeD, &why));
        QVERIFY(!chartIsFullyModeled(QByteArray("<c:chartSpace"), &why));
    }

    void textThatXmlCannotHoldIsLeftOut()
    {
        Chart c = comboChart();
        c.title.text = QStringLiteral("a\x01 b\x0B") + QChar(0xFFFE) + QStringLiteral("c");
        c.plots[0].series[0].name = QStringLiteral("x\x02y");
        c.plots[0].series[0].nameRef.clear();
        Chart r;
        QString err;
        QVERIFY2(readChart(writeChartPart(c), defaultTheme(), ColorMap(), &r, &err), qPrintable(err));
        QCOMPARE(r.title.text, QStringLiteral("a bc"));
        QCOMPARE(r.plots[0].series[0].name, QStringLiteral("xy"));
    }
};

JP_TEST_CLASS(OfficeChartWriteTests)
#include "tst_office_chartwrite.moc"
