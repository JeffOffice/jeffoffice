// Tests of libs/office: charts: reading them and drawing them as SVG.
#include "officehelpers.h"

#include "core/fonts.h"
#include "office/chart_trend.h"
#include "office/chart_write.h"
#include "core/workthread.h"
#include <algorithm>
#include <cmath>

class OfficeChartTests : public QObject {
    Q_OBJECT


private Q_SLOTS:
    // The program's own fonts and their stand-ins, as the apps load them at their start: the legend's size comes from its text, and a computer with
    // other fonts (or, under the offscreen platform on Windows, none) would put the legend somewhere else.
    void initTestCase() { jp::initCore(); }

    // ---- trendlines

    // A fit is made once for its data, whichever thread draws the chart (the count of fits made, not the time).
    void aTrendlineIsFitOnceForAllThreads()
    {
        using jp::office::fitTrend;
        const QVector<double> x = {1, 2, 3, 4, 5, 6};
        const QVector<double> y = {2.5, 4.1, 6.7, 8.2, 9.9, 13.4};
        jp::office::ChartTrendline t;
        const qint64 before = jp::office::trendFitsMadeForTest();
        const jp::office::TrendFit first = fitTrend(t, x, y, 61);
        QCOMPARE(jp::office::trendFitsMadeForTest() - before, qint64(1));
        QString onWorker;
        jp::WorkThread other([&] { onWorker = fitTrend(t, x, y, 61).equation; });
        other.join();
        QCOMPARE(onWorker, first.equation);
        QCOMPARE(fitTrend(t, x, y, 61).equation, first.equation);
        QCOMPARE(jp::office::trendFitsMadeForTest() - before, qint64(1));
    }

    void trendlinesFitTheirKinds()
    {
        using jp::office::ChartTrendline;
        using jp::office::fitTrend;
        const QVector<double> x = {1, 2, 3, 4, 5};
        ChartTrendline t;
        // linear: y = 2x + 1, exactly, and R-squared 1
        QVector<double> y = {3, 5, 7, 9, 11};
        jp::office::TrendFit f = fitTrend(t, x, y);
        QVERIFY(f.ok);
        QCOMPARE(f.equation, QStringLiteral("y = 2x + 1"));
        QVERIFY(std::fabs(f.rSquared - 1) < 1e-9);
        QCOMPARE(f.curve.size(), 2);
        QVERIFY(std::fabs(f.curve.first().x() - 1) < 1e-12 && std::fabs(f.curve.last().y() - 11) < 1e-9);
        // forward and backward run it past the data; an intercept makes it pass through that point on the vertical axis
        t.forward = 2;
        t.backward = 1;
        f = fitTrend(t, x, y);
        QVERIFY(std::fabs(f.curve.first().x() - 0) < 1e-12 && std::fabs(f.curve.last().x() - 7) < 1e-12);
        t = ChartTrendline();
        t.hasIntercept = true;
        t.intercept = 0;
        f = fitTrend(t, x, QVector<double>{2, 4, 6, 8, 10});
        QCOMPARE(f.equation, QStringLiteral("y = 2x"));   // (a zero constant is not written)
        // polynomial: y = x^2 + 1
        t = ChartTrendline();
        t.type = QStringLiteral("poly");
        f = fitTrend(t, x, QVector<double>{2, 5, 10, 17, 26});
        QVERIFY(f.ok);
        QVERIFY(std::fabs(f.coefficients[2] - 1) < 1e-6 && std::fabs(f.coefficients[1]) < 1e-6 && std::fabs(f.coefficients[0] - 1) < 1e-6);
        QVERIFY(f.curve.size() > 10);
        // a cubic through a chosen point on the vertical axis, and as many points as it has terms
        t.order = 3;
        t.hasIntercept = true;
        t.intercept = 2;
        f = fitTrend(t, x, QVector<double>{3, 10, 29, 66, 127});   // x^3 + 2
        QVERIFY(f.ok);
        QVERIFY(std::fabs(f.coefficients[3] - 1) < 1e-6 && std::fabs(f.coefficients[0] - 2) < 1e-9);
        f = fitTrend(t, QVector<double>{1, 2, 3}, QVector<double>{3, 10, 29});
        QVERIFY(f.ok);
        t.hasIntercept = false;
        t.order = 2;
        // exponential: y = 2 e^(0.5 x); power: y = 3 x^2; logarithmic: y = 4 ln x + 1
        QVector<double> e, pw, lg;
        for (double v : x) {
            e << 2 * std::exp(0.5 * v);
            pw << 3 * v * v;
            lg << 4 * std::log(v) + 1;
        }
        t.type = QStringLiteral("exp");
        f = fitTrend(t, x, e);
        QVERIFY(f.ok && std::fabs(f.coefficients[0] - 2) < 1e-6 && std::fabs(f.coefficients[1] - 0.5) < 1e-6);
        t.type = QStringLiteral("power");
        f = fitTrend(t, x, pw);
        QVERIFY(f.ok && std::fabs(f.coefficients[0] - 3) < 1e-6 && std::fabs(f.coefficients[1] - 2) < 1e-6);
        t.type = QStringLiteral("log");
        f = fitTrend(t, x, lg);
        QVERIFY(f.ok && std::fabs(f.coefficients[0] - 1) < 1e-6 && std::fabs(f.coefficients[1] - 4) < 1e-6);
        // what cannot be fitted is not: a logarithm of x at 0 or below, an exponential through a value at 0 or below, one point
        t.type = QStringLiteral("log");
        QVERIFY(!fitTrend(t, QVector<double>{0, 1, 2}, QVector<double>{1, 2, 3}).ok);
        t.type = QStringLiteral("exp");
        QVERIFY(!fitTrend(t, x, QVector<double>{1, 2, -3, 4, 5}).ok);
        QVERIFY(!fitTrend(ChartTrendline(), QVector<double>{1}, QVector<double>{1}).ok);
        // a moving average of 2: the mean of each value and the one before it
        t = ChartTrendline();
        t.type = QStringLiteral("movingAvg");
        t.period = 2;
        f = fitTrend(t, x, QVector<double>{2, 4, 6, 8, 10});
        QCOMPARE(f.curve.size(), 4);
        QCOMPARE(f.curve[0], QPointF(2, 3));
        QCOMPARE(f.curve[3], QPointF(5, 9));
        QVERIFY(f.equation.isEmpty() && !f.hasRSquared);
        // blanks are left out
        t = ChartTrendline();
        f = fitTrend(t, x, QVector<double>{3, std::nan(""), 7, 9, 11});
        QVERIFY(f.ok && f.equation == QStringLiteral("y = 2x + 1"));
        QCOMPARE(jp::office::trendlineName(ChartTrendline(), QStringLiteral("Sales")), QStringLiteral("Linear (Sales)"));
    }

    // An equation is written as a person writes it: rounded first, the sign chosen from what is left, no "1x", and no term that rounds to nothing.
    void anEquationIsWrittenAsAPersonWritesIt()
    {
        QVector<double> x = {1, 2, 3, 4, 5};
        ChartTrendline t;
        auto eq = [&](const QVector<double> &y) { return fitTrend(t, x, y).equation; };
        QCOMPARE(eq({3, 4, 5, 6, 7}), QStringLiteral("y = x + 2"));
        QCOMPARE(eq({1, 0, -1, -2, -3}), QStringLiteral("y = -x + 2"));
        QCOMPARE(eq({-1, -2, -3, -4, -5}), QStringLiteral("y = -x"));
        QCOMPARE(eq({2, 4, 6, 8, 10}), QStringLiteral("y = 2x"));
        QCOMPARE(eq({2, 4, 6, 8, 10.0000001}), QStringLiteral("y = 2x"));        // (rounds to a zero constant: no "- 0")
        QCOMPARE(eq({-0.0000001 + 2, 4, 6, 8, 10}), QStringLiteral("y = 2x"));
        QCOMPARE(eq({0, -1, -2, -3, -4}), QStringLiteral("y = -x + 1"));
        QCOMPARE(eq({-1, 1, 3, 5, 7}), QStringLiteral("y = 2x - 3"));
        QCOMPARE(eq({5, 5, 5, 5, 5}), QStringLiteral("y = 5"));
        t.type = QStringLiteral("poly");
        QCOMPARE(eq({2, 5, 10, 17, 26}), QStringLiteral("y = x\u00B2 + 1"));
        t.type = QStringLiteral("log");
        QCOMPARE(fitTrend(t, x, {1, 1 + std::log(2), 1 + std::log(3), 1 + std::log(4), 1 + std::log(5)}).equation, QStringLiteral("y = ln(x) + 1"));
    }

    // A moving average by a running sum is the mean of each window, whatever the order the points come in and however long the line.
    void aMovingAverageIsTheMeanOfEachWindow()
    {
        ChartTrendline t;
        t.type = QStringLiteral("movingAvg");
        t.period = 4;
        QVector<double> x, y;
        for (int i = 0; i < 100000; ++i) {
            x << (i * 7919) % 100003;   // (not in order)
            y << (i % 13) * 1.5;
        }
        const TrendFit f = fitTrend(t, x, y);
        QVERIFY(f.ok);
        QCOMPARE(f.curve.size(), 100000 - 3);
        // the window of the sorted points, by the plain sum
        QVector<int> order(x.size());
        for (int i = 0; i < order.size(); ++i) order[i] = i;
        std::stable_sort(order.begin(), order.end(), [&](int a, int b) { return x[a] < x[b]; });
        for (const int at : {0, 1, 500, 99996}) {
            double sum = 0;
            for (int k = 0; k < 4; ++k) sum += y[order[at + 3 - k]];
            QCOMPARE(f.curve[at].x(), x[order[at + 3]]);
            QVERIFY(std::fabs(f.curve[at].y() - sum / 4) < 1e-9);
        }
        // the same line asked for again is the same
        QVERIFY(fitTrend(t, x, y).curve == f.curve);
    }

    // ---- charts

    void chartReadsClusteredColumns()
    {
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/><c:varyColors val=\"0\"/>")
                             + chartSeries(0, QStringLiteral("Q1"), {"East", "West", "North"}, {10, 20, 40}, QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></c:spPr>"))
                             + chartSeries(1, QStringLiteral("Q2"), {"East", "West", "North"}, {5, 15, 25})
                             + QStringLiteral("<c:gapWidth val=\"80\"/><c:overlap val=\"-10\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        Chart c;
        QString err;
        const Theme theme = defaultTheme();
        QVERIFY2(readChart(chartPart(plot), theme, ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(c.plots.size()), 1);
        const ChartPlot &p = c.plots[0];
        QCOMPARE(int(p.type), int(ChartType::Bar));
        QCOMPARE(int(c.type()), int(ChartType::Bar));
        QVERIFY(!p.horizontal);
        QCOMPARE(int(p.grouping), int(ChartGrouping::Clustered));
        QCOMPARE(p.gapWidth, 80);
        QCOMPARE(p.overlap, -10);
        QVERIFY(!p.varyColors);
        QCOMPARE(int(p.series.size()), 2);
        QCOMPARE(p.series[0].name, QStringLiteral("Q1"));
        QCOMPARE(p.series[0].categories, QStringList({"East", "West", "North"}));
        QCOMPARE(p.series[0].values, (QVector<double>{10, 20, 40}));
        QCOMPARE(p.series[1].name, QStringLiteral("Q2"));
        QCOMPARE(p.series[1].values, (QVector<double>{5, 15, 25}));
        // The axes: where the file says they are, and which the bars are drawn against.
        QCOMPARE(int(c.axes.size()), 2);
        QCOMPARE(int(p.axisIds[0]), 111);
        QCOMPARE(int(p.axisIds[1]), 222);
        QVERIFY(c.axis(111) && c.axis(222));
        QCOMPARE(int(c.axis(111)->kind), int(ChartAxis::Category));
        QCOMPARE(int(c.axis(222)->kind), int(ChartAxis::Value));
        QVERIFY(c.axis(222)->majorGridlines);
        QVERIFY(!c.axis(111)->majorGridlines);
        QVERIFY(c.axis(222)->autoMin && c.axis(222)->autoMax);
    }

    void chartReadsStackedBars()
    {
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"bar\"/><c:grouping val=\"stacked\"/><c:varyColors val=\"0\"/>")
                             + chartSeries(0, QStringLiteral("A"), {"x", "y"}, {1, 2}) + chartSeries(1, QStringLiteral("B"), {"x", "y"}, {3, 4})
                             + QStringLiteral("<c:gapWidth val=\"150\"/><c:overlap val=\"100\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(c.plots.size()), 1);
        QVERIFY(c.plots[0].horizontal);
        QCOMPARE(int(c.plots[0].grouping), int(ChartGrouping::Stacked));
        QCOMPARE(c.plots[0].overlap, 100);
        // Percent stacked, and the three-dimensional kinds, which are drawn flat.
        QString percent = plot;
        percent.replace(QStringLiteral("\"stacked\""), QStringLiteral("\"percentStacked\""));
        QVERIFY2(readChart(chartPart(percent), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(c.plots[0].grouping), int(ChartGrouping::PercentStacked));
        QString threeD = plot;
        threeD.replace(QStringLiteral("barChart>"), QStringLiteral("bar3DChart>"));
        QVERIFY2(readChart(chartPart(threeD), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(c.plots[0].type), int(ChartType::Bar));
    }

    void chartReadsLinesWithMarkers()
    {
        const QString marked = QStringLiteral("<c:marker><c:symbol val=\"circle\"/><c:size val=\"9\"/><c:spPr><a:solidFill><a:srgbClr val=\"00AA00\"/></a:solidFill></c:spPr></c:marker>");
        const QString bare = QStringLiteral("<c:marker><c:symbol val=\"none\"/></c:marker>");
        const QString plot = QStringLiteral("<c:lineChart><c:grouping val=\"standard\"/><c:varyColors val=\"0\"/>")
                             + chartSeries(0, QStringLiteral("One"), {"a", "b", "c"}, {1, 3, 2}, QStringLiteral("<c:spPr><a:ln w=\"38100\"><a:solidFill><a:srgbClr val=\"0000FF\"/></a:solidFill><a:prstDash val=\"dash\"/></a:ln></c:spPr>"),
                                           marked, QStringLiteral("<c:smooth val=\"1\"/>"))
                             + chartSeries(1, QStringLiteral("Two"), {"a", "b", "c"}, {2, 2, 4}, QString(), bare, QStringLiteral("<c:smooth val=\"0\"/>"))
                             + QStringLiteral("<c:marker val=\"1\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:lineChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const ChartPlot &p = c.plots[0];
        QCOMPARE(int(p.type), int(ChartType::Line));
        QCOMPARE(int(p.grouping), int(ChartGrouping::Standard));
        QVERIFY(p.markers);
        QCOMPARE(int(p.series[0].marker.symbol), int(ChartMarker::Circle));
        QCOMPARE(p.series[0].marker.size, 9.0);
        QCOMPARE(p.series[0].marker.fill, QColor(0x00, 0xAA, 0x00));
        QVERIFY(p.series[0].smooth);
        QCOMPARE(p.series[0].color, QColor(0, 0, 0xFF));   // a line's color is its stroke's
        QCOMPARE(p.series[0].outline.width, 3.0);
        QCOMPARE(p.series[0].outline.dash, QStringLiteral("dash"));
        QCOMPARE(int(p.series[1].marker.symbol), int(ChartMarker::None));
        QVERIFY(!p.series[1].smooth);
        QCOMPARE(p.series[1].color, defaultTheme().colors[Theme::Accent2]);
    }

    void chartReadsPiesWithDataLabels()
    {
        const QString labels = QStringLiteral("<c:dLbls><c:numFmt formatCode=\"0%\" sourceLinked=\"0\"/><c:txPr><a:bodyPr/><a:p><a:pPr><a:defRPr sz=\"1200\" b=\"1\"/></a:pPr><a:endParaRPr lang=\"en-US\"/></a:p></c:txPr>"
                                              "<c:dLblPos val=\"outEnd\"/><c:showLegendKey val=\"0\"/><c:showVal val=\"0\"/><c:showCatName val=\"0\"/><c:showSerName val=\"0\"/><c:showPercent val=\"1\"/><c:showBubbleSize val=\"0\"/>"
                                              "<c:showLeaderLines val=\"1\"/></c:dLbls>");
        const QString point = QStringLiteral("<c:dPt><c:idx val=\"2\"/><c:bubble3D val=\"0\"/><c:spPr><a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill></c:spPr></c:dPt>");
        const QString plot = QStringLiteral("<c:pieChart><c:varyColors val=\"1\"/>")
                             + chartSeries(0, QStringLiteral("Share"), {"a", "b", "c", "d"}, {1, 2, 3, 4}, QString(), point + labels)
                             + QStringLiteral("<c:firstSliceAng val=\"90\"/></c:pieChart>");
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const ChartPlot &p = c.plots[0];
        QCOMPARE(int(p.type), int(ChartType::Pie));
        QVERIFY(p.varyColors);
        QCOMPARE(p.firstSliceAngle, 90);
        const ChartSeries &s = p.series[0];
        QVERIFY(s.labels.percent);
        QVERIFY(!s.labels.value && !s.labels.category && !s.labels.series);
        QCOMPARE(s.labels.format, QStringLiteral("0%"));
        QCOMPARE(s.labels.position, QStringLiteral("outEnd"));
        QCOMPARE(s.labels.font.size, 12.0);
        QCOMPARE(s.labels.font.bold, 1);
        QCOMPARE(int(s.pointColors.size()), 1);
        QCOMPARE(s.pointColors.value(2), QColor(0, 0xFF, 0));
        QVERIFY(c.axes.isEmpty());   // a pie has no axes
        // A doughnut has its hole.
        QString ring = plot;
        ring.replace(QStringLiteral("pieChart"), QStringLiteral("doughnutChart")).replace(QStringLiteral("<c:firstSliceAng val=\"90\"/>"), QStringLiteral("<c:firstSliceAng val=\"0\"/><c:holeSize val=\"60\"/>"));
        QVERIFY2(readChart(chartPart(ring), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(c.plots[0].type), int(ChartType::Doughnut));
        QCOMPARE(c.plots[0].holeSize, 60);
    }

    void chartReadsScatterPlots()
    {
        const QString xs = QStringLiteral("<c:xVal><c:numRef><c:f>Sheet1!$A$2:$A$5</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val=\"4\"/>"
                                          "<c:pt idx=\"0\"><c:v>1</c:v></c:pt><c:pt idx=\"1\"><c:v>2.5</c:v></c:pt><c:pt idx=\"2\"><c:v>3</c:v></c:pt><c:pt idx=\"3\"><c:v>7</c:v></c:pt></c:numCache></c:numRef></c:xVal>");
        const QString ys = QStringLiteral("<c:yVal><c:numLit><c:formatCode>General</c:formatCode><c:ptCount val=\"4\"/>"
                                          "<c:pt idx=\"0\"><c:v>2</c:v></c:pt><c:pt idx=\"1\"><c:v>4</c:v></c:pt><c:pt idx=\"3\"><c:v>9</c:v></c:pt></c:numLit></c:yVal>");
        const QString series = QStringLiteral("<c:ser><c:idx val=\"0\"/><c:order val=\"0\"/><c:tx><c:v>Trial</c:v></c:tx><c:spPr><a:ln w=\"19050\"><a:noFill/></a:ln></c:spPr>"
                                              "<c:marker><c:symbol val=\"diamond\"/><c:size val=\"7\"/></c:marker>") + xs + ys + QStringLiteral("<c:smooth val=\"0\"/></c:ser>");
        const QString axes = QStringLiteral("<c:valAx><c:axId val=\"111\"/><c:scaling><c:orientation val=\"minMax\"/><c:max val=\"10\"/><c:min val=\"0\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"b\"/>"
                                            "<c:numFmt formatCode=\"General\" sourceLinked=\"1\"/><c:crossAx val=\"222\"/><c:crosses val=\"autoZero\"/><c:crossBetween val=\"midCat\"/></c:valAx>"
                                            "<c:valAx><c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"l\"/><c:majorGridlines/>"
                                            "<c:numFmt formatCode=\"General\" sourceLinked=\"1\"/><c:crossAx val=\"111\"/><c:crosses val=\"autoZero\"/><c:crossBetween val=\"midCat\"/></c:valAx>");
        const QString plot = QStringLiteral("<c:scatterChart><c:scatterStyle val=\"lineMarker\"/><c:varyColors val=\"0\"/>") + series
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:scatterChart>") + axes;
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const ChartPlot &p = c.plots[0];
        QCOMPARE(int(p.type), int(ChartType::Scatter));
        QCOMPARE(p.series[0].name, QStringLiteral("Trial"));
        QCOMPARE(p.series[0].xValues, (QVector<double>{1, 2.5, 3, 7}));
        QCOMPARE(int(p.series[0].values.size()), 4);
        QCOMPARE(p.series[0].values[0], 2.0);
        QCOMPARE(p.series[0].values[1], 4.0);
        QVERIFY(std::isnan(p.series[0].values[2]));   // a point the file has no value for
        QCOMPARE(p.series[0].values[3], 9.0);
        QVERIFY(p.series[0].outline.none);            // markers alone
        QCOMPARE(int(p.series[0].marker.symbol), int(ChartMarker::Diamond));
        QCOMPARE(int(c.axes.size()), 2);
        QCOMPARE(int(c.axis(111)->kind), int(ChartAxis::Value));
        QVERIFY(!c.axis(111)->autoMin && !c.axis(111)->autoMax);
        QCOMPARE(c.axis(111)->min, 0.0);
        QCOMPARE(c.axis(111)->max, 10.0);
        QVERIFY(c.axis(222)->autoMax);
    }

    void chartTakesColorsFromTheFileAndTheTheme()
    {
        const QString accent = QStringLiteral("<c:spPr><a:solidFill><a:schemeClr val=\"accent4\"/></a:solidFill></c:spPr>");
        const QString darker = QStringLiteral("<c:spPr><a:solidFill><a:schemeClr val=\"accent3\"><a:lumMod val=\"50000\"/></a:schemeClr></a:solidFill></c:spPr>");
        QString series = chartSeries(0, QStringLiteral("S0"), {"a"}, {1}, accent) + chartSeries(1, QStringLiteral("S1"), {"a"}, {1}, darker);
        for (int i = 2; i < 14; ++i) series += chartSeries(i, QStringLiteral("S%1").arg(i), {"a"}, {1});
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + series + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        Chart c;
        QString err;
        const Theme theme = defaultTheme();
        QVERIFY2(readChart(chartPart(plot), theme, ColorMap(), &c, &err), qPrintable(err));
        const QVector<ChartSeries> &s = c.plots[0].series;
        QCOMPARE(int(s.size()), 14);
        QCOMPARE(s[0].color, theme.colors[Theme::Accent4]);
        QVERIFY(s[0].colorFromFile);
        // A transform of a theme color is applied.
        QVERIFY(s[1].colorFromFile);
        QVERIFY(s[1].color.lightnessF() < theme.colors[Theme::Accent3].lightnessF() * 0.7);
        QVERIFY(qAbs(s[1].color.hueF() - theme.colors[Theme::Accent3].hueF()) < 0.02);
        // The rest take the accents in order (the file's own colors don't use them up).
        for (int i = 2; i < 6; ++i) {
            QVERIFY(!s[i].colorFromFile);
            QCOMPARE(s[i].color, theme.colors[Theme::Accent1 + i]);
        }
        // More series than accents: the accents again, each darker, then lighter.
        QVERIFY(qAbs(s[6].color.hueF() - theme.colors[Theme::Accent1].hueF()) < 0.02);
        QVERIFY(s[6].color != theme.colors[Theme::Accent1]);
        QVERIFY(s[6].color.lightnessF() < theme.colors[Theme::Accent1].lightnessF());
        QVERIFY(s[12].color.lightnessF() > s[6].color.lightnessF());
        QVERIFY(s[12].color != theme.colors[Theme::Accent1]);
        // The color map renames what the file asks for: a dark slide's "tx1" is its light one.
        ColorMap dark;
        dark.tx1 = Theme::Light1;
        dark.bg1 = Theme::Dark1;
        const QString text = QStringLiteral("<c:txPr><a:bodyPr/><a:p><a:pPr><a:defRPr><a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill></a:defRPr></a:pPr><a:endParaRPr lang=\"en-US\"/></a:p></c:txPr>");
        QVERIFY2(readChart(chartPart(plot, QString(), QString(), text), theme, dark, &c, &err), qPrintable(err));
        QCOMPARE(c.font.color, theme.colors[Theme::Light1]);
    }

    void chartStyleColorsTheSeriesTheFileLeavesBare()
    {
        QString series;
        for (int i = 0; i < 4; ++i) series += chartSeries(i, QStringLiteral("S%1").arg(i), {"a", "b"}, {1.0 + i, 2.0});
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + series + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        const Theme theme = defaultTheme();
        auto styled = [&](int style) {
            QByteArray xml = chartPart(plot);
            xml.replace("<c:roundedCorners val=\"0\"/>", QByteArray("<c:roundedCorners val=\"0\"/><c:style val=\"") + QByteArray::number(style) + "\"/>");
            Chart c;
            QString err;
            [&] { QVERIFY2(readChart(xml, theme, ColorMap(), &c, &err), qPrintable(err)); }();
            return c;
        };
        // Style 2, and what has none, takes the accents; so do the styles that count up from it eight at a time.
        for (int style : {0, 2, 10, 26, 42}) {
            const Chart c = styled(style);
            QCOMPARE(c.style, style == 0 ? 2 : style);
            for (int i = 0; i < 4; ++i) QCOMPARE(c.plots[0].series[i].color, theme.colors[Theme::Accent1 + i]);
        }
        // Style 1 is shades of gray, darkest first.
        const Chart gray = styled(1);
        for (int i = 0; i < 4; ++i) {
            QVERIFY2(gray.plots[0].series[i].color.saturationF() < 0.05, qPrintable(gray.plots[0].series[i].color.name()));
            if (i > 0) QVERIFY(gray.plots[0].series[i].color.lightnessF() > gray.plots[0].series[i - 1].color.lightnessF());
        }
        // Style 3 is shades of the first accent, style 8 of the sixth (and 19 again of the first).
        for (const QPair<int, int> &p : {qMakePair(3, 0), qMakePair(8, 5), qMakePair(19, 0)}) {
            const Chart c = styled(p.first);
            const QColor accent = theme.colors[Theme::Accent1 + p.second];
            for (int i = 0; i < 4; ++i) {
                const QColor shade = c.plots[0].series[i].color;
                QVERIFY2(qAbs(shade.hueF() - accent.hueF()) < 0.03 || accent.saturationF() < 0.1, qPrintable(QString("%1: %2").arg(p.first).arg(shade.name())));
                if (i > 0) QVERIFY(shade.lightnessF() > c.plots[0].series[i - 1].color.lightnessF());
            }
            QVERIFY(c.plots[0].series[0].color.lightnessF() < accent.lightnessF());
            QVERIFY(c.plots[0].series[3].color.lightnessF() > accent.lightnessF());
        }
        // A color the file gives stays.
        const QString own = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>")
                            + chartSeries(0, QStringLiteral("A"), {"a"}, {1}, QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></c:spPr>")) + chartSeries(1, QStringLiteral("B"), {"a"}, {2})
                            + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        QByteArray xml = chartPart(own);
        xml.replace("<c:roundedCorners val=\"0\"/>", "<c:roundedCorners val=\"0\"/><c:style val=\"3\"/>");
        Chart c;
        QString err;
        QVERIFY2(readChart(xml, theme, ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(c.plots[0].series[0].color, QColor(0xFF, 0, 0));
        QVERIFY(!c.plots[0].series[1].colorFromFile);
        QVERIFY(qAbs(c.plots[0].series[1].color.hueF() - theme.colors[Theme::Accent1].hueF()) < 0.03);
    }

    void chartReadsTitleLegendAndAxes()
    {
        const QString title = QStringLiteral("<c:title><c:tx><c:rich><a:bodyPr/><a:p><a:pPr><a:defRPr sz=\"1400\" b=\"1\"/></a:pPr><a:r><a:rPr lang=\"en-US\" sz=\"1800\" b=\"0\"/><a:t>Sales by </a:t></a:r>"
                                             "<a:r><a:rPr lang=\"en-US\" sz=\"1800\" b=\"0\"/><a:t>region</a:t></a:r></a:p><a:p><a:r><a:rPr lang=\"en-US\" sz=\"1000\"/><a:t>in thousands</a:t></a:r></a:p></c:rich></c:tx>"
                                             "<c:overlay val=\"0\"/></c:title><c:autoTitleDeleted val=\"0\"/>");
        const QString axes = QStringLiteral("<c:catAx><c:axId val=\"111\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"b\"/>"
                                            "<c:title><c:tx><c:rich><a:bodyPr/><a:p><a:r><a:t>Region</a:t></a:r></a:p></c:rich></c:tx><c:overlay val=\"0\"/></c:title>"
                                            "<c:numFmt formatCode=\"General\" sourceLinked=\"1\"/><c:majorTickMark val=\"none\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"low\"/>"
                                            "<c:crossAx val=\"222\"/><c:crosses val=\"autoZero\"/><c:auto val=\"1\"/><c:lblAlgn val=\"ctr\"/><c:lblOffset val=\"100\"/><c:tickLblSkip val=\"2\"/><c:noMultiLvlLbl val=\"0\"/></c:catAx>"
                                            "<c:valAx><c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/><c:max val=\"50\"/><c:min val=\"-10\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"l\"/>"
                                            "<c:majorGridlines><c:spPr><a:ln w=\"9525\"><a:solidFill><a:srgbClr val=\"D9D9D9\"/></a:solidFill></a:ln></c:spPr></c:majorGridlines>"
                                            "<c:title><c:tx><c:rich><a:bodyPr rot=\"-5400000\" vert=\"horz\"/><a:p><a:r><a:t>Units</a:t></a:r></a:p></c:rich></c:tx><c:overlay val=\"0\"/></c:title>"
                                            "<c:numFmt formatCode=\"#,##0&quot; units&quot;\" sourceLinked=\"0\"/><c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/>"
                                            "<c:txPr><a:bodyPr/><a:p><a:pPr><a:defRPr sz=\"900\"/></a:pPr><a:endParaRPr lang=\"en-US\"/></a:p></c:txPr>"
                                            "<c:crossAx val=\"111\"/><c:crosses val=\"autoZero\"/><c:crossBetween val=\"between\"/><c:majorUnit val=\"10\"/></c:valAx>");
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("Q1"), {"East", "West"}, {10, 20})
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + axes;
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot, title, QStringLiteral("<c:legend><c:legendPos val=\"r\"/><c:overlay val=\"0\"/></c:legend>")), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QVERIFY(c.title.present);
        QCOMPARE(c.title.text, QStringLiteral("Sales by region\nin thousands"));
        QCOMPARE(c.title.font.size, 18.0);   // the first run's, over the paragraph's default
        QCOMPARE(c.title.font.bold, 0);
        QVERIFY(!c.title.overlay);
        QVERIFY(c.legend.shown);
        QCOMPARE(c.legend.position, QStringLiteral("r"));
        const ChartAxis *cat = c.axis(111), *val = c.axis(222);
        QVERIFY(cat && val);
        QCOMPARE(cat->title.text, QStringLiteral("Region"));
        QCOMPARE(cat->labelPosition, QStringLiteral("low"));
        QCOMPARE(cat->majorTick, QStringLiteral("none"));
        QCOMPARE(cat->labelSkip, 2);
        QCOMPARE(cat->position, QStringLiteral("b"));
        QCOMPARE(val->title.text, QStringLiteral("Units"));
        QCOMPARE(val->position, QStringLiteral("l"));
        QVERIFY(!val->autoMin && !val->autoMax);
        QCOMPARE(val->min, -10.0);
        QCOMPARE(val->max, 50.0);
        QCOMPARE(val->majorUnit, 10.0);
        QVERIFY(val->majorGridlines);
        QCOMPARE(val->gridline.color, QColor(0xD9, 0xD9, 0xD9));
        QCOMPARE(val->gridline.width, 0.75);
        QCOMPARE(val->format, QStringLiteral("#,##0\" units\""));
        QVERIFY(!val->formatLinked);
        QCOMPARE(val->font.size, 9.0);
        QVERIFY(val->between);
        // No title element: none, and no legend: none.
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QVERIFY(!c.title.present);
        QVERIFY(!c.legend.shown);
    }

    void chartReadsAreasAndLiterals()
    {
        // A category the file stores as numbers, as text.
        const QString cats = QStringLiteral("<c:cat><c:numRef><c:f>Sheet1!$A$2:$A$4</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val=\"3\"/>"
                                            "<c:pt idx=\"0\"><c:v>1774</c:v></c:pt><c:pt idx=\"2\"><c:v>1790</c:v></c:pt></c:numCache></c:numRef></c:cat>");
        const QString vals = QStringLiteral("<c:val><c:numRef><c:f>Sheet1!$B$2:$B$4</c:f><c:numCache><c:formatCode>0.0</c:formatCode><c:ptCount val=\"3\"/>"
                                            "<c:pt idx=\"1\"><c:v>0.217</c:v></c:pt><c:pt idx=\"2\"><c:v>7.8718E-2</c:v></c:pt></c:numCache></c:numRef></c:val>");
        const QString plot = QStringLiteral("<c:areaChart><c:grouping val=\"stacked\"/><c:varyColors val=\"0\"/><c:ser><c:idx val=\"0\"/><c:order val=\"0\"/>") + cats + vals
                             + QStringLiteral("</c:ser><c:axId val=\"111\"/><c:axId val=\"222\"/></c:areaChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(c.plots[0].type), int(ChartType::Area));
        QCOMPARE(int(c.plots[0].grouping), int(ChartGrouping::Stacked));
        const ChartSeries &s = c.plots[0].series[0];
        QCOMPARE(s.categories, QStringList({"1774", "", "1790"}));
        QCOMPARE(int(s.values.size()), 3);
        QVERIFY(std::isnan(s.values[0]));
        QCOMPARE(s.values[1], 0.217);
        QCOMPARE(s.values[2], 0.078718);
        QCOMPARE(s.format, QStringLiteral("0.0"));
        QCOMPARE(s.name, QStringLiteral("Series1"));   // the file names none
        QCOMPARE(c.blanksAs, QStringLiteral("gap"));
    }

    void chartKeepsWhatItCannotDrawAsColumns()
    {
        // Kinds not drawn yet read as columns of their values, with the file's name for them.
        const QString plot = QStringLiteral("<c:stockChart>") + chartSeries(0, QStringLiteral("R"), {"a", "b", "c"}, {3, 1, 2})
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:stockChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(c.plots.size()), 1);
        QCOMPARE(int(c.plots[0].type), int(ChartType::Other));
        QCOMPARE(c.plots[0].element, QStringLiteral("stockChart"));
        QCOMPARE(c.plots[0].series[0].values, (QVector<double>{3, 1, 2}));
        // Two kinds at once: columns with a line over them.
        const QString both = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("Bars"), {"a", "b"}, {3, 1})
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart><c:lineChart><c:grouping val=\"standard\"/>") + chartSeries(1, QStringLiteral("Line"), {"a", "b"}, {2, 2})
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:lineChart>") + chartAxes();
        QVERIFY2(readChart(chartPart(both), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(c.plots.size()), 2);
        QCOMPARE(int(c.plots[0].type), int(ChartType::Bar));
        QCOMPARE(int(c.plots[1].type), int(ChartType::Line));
        // The colors continue from one plot to the next.
        QCOMPARE(c.plots[0].series[0].color, defaultTheme().colors[Theme::Accent1]);
        QCOMPARE(c.plots[1].series[0].color, defaultTheme().colors[Theme::Accent2]);
    }

    // A radar chart is a kind of its own, in its three styles: lines, lines with markers, and filled.
    void chartReadsRadarInItsThreeStyles()
    {
        const char *styles[] = {"standard", "marker", "filled"};
        for (int i = 0; i < 3; ++i) {
            const QString plot = QStringLiteral("<c:radarChart><c:radarStyle val=\"%1\"/><c:varyColors val=\"0\"/>").arg(QLatin1String(styles[i]))
                                 + chartSeries(0, QStringLiteral("R"), {"a", "b", "c"}, {3, 1, 2}) + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:radarChart>") + chartAxes();
            Chart c;
            QString err;
            QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
            QCOMPARE(int(c.plots.size()), 1);
            QCOMPARE(int(c.plots[0].type), int(ChartType::Radar));
            QCOMPARE(c.plots[0].filled, i == 2);
            QCOMPARE(c.plots[0].markers, i == 1);
            QCOMPARE(c.plots[0].series[0].values, (QVector<double>{3, 1, 2}));
            QCOMPARE(c.plots[0].axisIds[0], qint64(111));
            QCOMPARE(c.plots[0].axisIds[1], qint64(222));
        }
        // The style "marker" is written for the plain radar and for the one with markers; they differ by the symbol "none" the plain one's series each have.
        {
            const QString none = QStringLiteral("<c:marker><c:symbol val=\"none\"/></c:marker>");
            const QString plain = QStringLiteral("<c:radarChart><c:radarStyle val=\"marker\"/><c:varyColors val=\"0\"/>") + chartSeries(0, QStringLiteral("A"), {"a", "b", "c"}, {3, 1, 2}, QString(), none)
                                  + chartSeries(1, QStringLiteral("B"), {"a", "b", "c"}, {1, 2, 3}, QString(), none) + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:radarChart>") + chartAxes();
            Chart c;
            QString err;
            QVERIFY2(readChart(chartPart(plain), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
            QVERIFY(!c.plots[0].markers && !c.plots[0].filled);
            // and a radar of the style "standard" has no markers, the series saying so
            QVERIFY2(readChart(chartPart(QStringLiteral("<c:radarChart><c:radarStyle val=\"standard\"/>") + chartSeries(0, QStringLiteral("A"), {"a", "b", "c"}, {3, 1, 2})
                                         + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:radarChart>") + chartAxes()), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
            QVERIFY(!c.plots[0].markers);
            QCOMPARE(int(c.plots[0].series[0].marker.symbol), int(ChartMarker::None));
        }
        // A line radar takes its color from the line, a filled one from the fill.
        const QString line = QStringLiteral("<c:spPr><a:ln w=\"28575\"><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></a:ln></c:spPr>");
        const QString fill = QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill></c:spPr>");
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(QStringLiteral("<c:radarChart><c:radarStyle val=\"standard\"/>") + chartSeries(0, QStringLiteral("R"), {"a", "b"}, {1, 2}, line)
                                     + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:radarChart>") + chartAxes()), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(c.plots[0].series[0].color, QColor(0xFF, 0, 0));
        QVERIFY2(readChart(chartPart(QStringLiteral("<c:radarChart><c:radarStyle val=\"filled\"/>") + chartSeries(0, QStringLiteral("R"), {"a", "b"}, {1, 2}, fill)
                                     + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:radarChart>") + chartAxes()), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(c.plots[0].series[0].color, QColor(0, 0xFF, 0));
    }

    // A radar beside another plot has no place on one set of axes: the chart is drawn as a kind not drawn yet is (every plot as columns, as before radar was a kind), and the part is kept as written.
    void aRadarBesideAnotherPlotIsDrawnAsColumnsAndKept()
    {
        const QString radar = QStringLiteral("<c:radarChart><c:radarStyle val=\"marker\"/><c:varyColors val=\"0\"/>") + chartSeries(1, QStringLiteral("R"), {"a", "b", "c"}, {3, 1, 2})
                              + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:radarChart>");
        const QString bars = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/><c:varyColors val=\"0\"/>") + chartSeries(0, QStringLiteral("B"), {"a", "b", "c"}, {1, 2, 3})
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>");
        for (const QString &body : {bars + radar + chartAxes(), radar + bars + chartAxes()}) {
            const QByteArray part = chartPart(body);
            Chart c;
            QString err;
            QVERIFY2(readChart(part, defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
            QCOMPARE(int(c.plots.size()), 2);
            for (const ChartPlot &p : c.plots) QVERIFY(p.type != ChartType::Radar);
            // both are drawn: the same picture as the chart whose second kind is one not drawn yet, and in it the series of both
            QString other = body;
            other.replace(QStringLiteral("radarChart"), QStringLiteral("stockChart"));
            Chart o;
            QVERIFY2(readChart(chartPart(other), defaultTheme(), ColorMap(), &o, &err), qPrintable(err));
            QCOMPARE(chartSvg(c, QSizeF(400, 300)), chartSvg(o, QSizeF(400, 300)));
            const QImage img = renderChart(c, QSizeF(400, 300), 2);
            QVERIFY(colorArea(img, defaultTheme().colors[Theme::Accent1]) > 200);
            QVERIFY(colorArea(img, defaultTheme().colors[Theme::Accent2]) > 200);
            // not written over: the part is kept as it is
            QVERIFY(!chartIsFullyModeled(part));
        }
        // alone it is a radar
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(radar + chartAxes()), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(c.plots[0].type), int(ChartType::Radar));
    }

    void chartRefusesWhatIsNotAChart()
    {
        Chart c;
        QString err;
        QVERIFY(!readChart(QByteArray("<a:theme xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\"/>"), defaultTheme(), ColorMap(), &c, &err));
        QVERIFY(!err.isEmpty());
        err.clear();
        QVERIFY(!readChart(QByteArray("this is not XML"), defaultTheme(), ColorMap(), &c, &err));
        QVERIFY(!err.isEmpty());
        QVERIFY(!readChart(QByteArray(), defaultTheme(), ColorMap(), &c, nullptr));
        // A chart with no series is one: it draws as an empty chart.
        QVERIFY2(readChart(chartPart(QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes()),
                           defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QVERIFY(c.plots[0].series.isEmpty());
        QVERIFY(chartSvg(c, QSizeF(300, 200)).contains("<svg"));
    }

    void chartFramesAreFoundByTheirUri()
    {
        const QString frame = QStringLiteral("<p:graphicFrame xmlns:p=\"%1\" xmlns:a=\"%2\" xmlns:r=\"%3\"><p:nvGraphicFramePr><p:cNvPr id=\"4\" name=\"Chart 3\"/><p:cNvGraphicFramePr/><p:nvPr/></p:nvGraphicFramePr>"
                                            "<p:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"100\" cy=\"100\"/></p:xfrm><a:graphic><a:graphicData uri=\"%4\"><c:chart xmlns:c=\"%4\" r:id=\"rId7\"/></a:graphicData></a:graphic></p:graphicFrame>");
        QDomDocument doc;
        const QString chartUri = QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/chart");
        QVERIFY(parseXml(frame.arg(QString(ns::P), QString(ns::A), QString(ns::R), chartUri).toUtf8(), &doc));
        QVERIFY(isChartFrame(doc.documentElement()));
        QCOMPARE(chartFrameRelId(doc.documentElement()), QStringLiteral("rId7"));
        QVERIFY(parseXml(frame.arg(QString(ns::P), QString(ns::A), QString(ns::R), QStringLiteral("urn:test:other")).toUtf8(), &doc));
        QVERIFY(!isChartFrame(doc.documentElement()));
        QVERIFY(!isChartFrame(QDomElement()));
    }

    void chartFormatsNumbersAsTheirCodesSay()
    {
        struct Case { double value; const char *code, *text; };
        const Case cases[] = {
            {1234.5, "General", "1234.5"}, {0.1 + 0.2, "General", "0.3"}, {0, "General", "0"}, {16.112566, "", "16.112566"}, {-2, "General", "-2"},
            {1234.567, "0", "1235"}, {1234.567, "0.0", "1234.6"}, {1234.567, "#,##0.00", "1,234.57"}, {1234567, "#,##0", "1,234,567"}, {12, "000", "012"},
            {0.256, "0%", "26%"}, {0.256, "0.0%", "25.6%"}, {1, "0.0%", "100.0%"}, {-1.5, "0.0", "-1.5"},
            {5, "$#,##0.00;($#,##0.00)", "$5.00"}, {-5, "$#,##0.00;($#,##0.00)", "($5.00)"}, {0, "0;-0;\"none\"", "none"},
            {5, "0\" units\"", "5 units"}, {1500, "0.0,\"k\"", "1.5k"}, {2500000, "0.0,,\"M\"", "2.5M"}, {12345.678, "0.00E+00", "1.23E+04"}, {0.5, "#.##", ".5"}, {3, "0.0#", "3.0"},
            {45000, "m/d/yyyy", "3/15/2023"}, {45000, "yyyy-mm-dd", "2023-03-15"}, {45000, "mmm-yy", "Mar-23"}, {45000, "dddd", "Wednesday"}, {45000.75, "h:mm AM/PM", "6:00 PM"},
            {45000.75, "d mmmm yyyy, hh:mm", "15 March 2023, 18:00"}};
        for (const Case &c : cases) QCOMPARE(formatChartNumber(c.value, QString::fromLatin1(c.code)), QString::fromLatin1(c.text));
        QVERIFY(formatChartNumber(std::nan(""), QStringLiteral("0")).isEmpty());
    }

    void chartSvgIsInPointsWithTheTextAsText()
    {
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("Revenue"), {"East", "West"}, {10, 20})
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        const QString title = QStringLiteral("<c:title><c:tx><c:rich><a:bodyPr/><a:p><a:r><a:rPr lang=\"en-US\"/><a:t>Quarterly &amp; yearly</a:t></a:r></a:p></c:rich></c:tx><c:overlay val=\"0\"/></c:title>");
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot, title, QStringLiteral("<c:legend><c:legendPos val=\"b\"/></c:legend>")), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QByteArray svg = chartSvg(c, QSizeF(432, 252));
        QSvgRenderer r(svg);
        QVERIFY(r.isValid());
        QCOMPARE(r.viewBoxF(), QRectF(0, 0, 432, 252));
        // Text is text, in the chart's font.
        const QString text = QString::fromUtf8(svg);
        QVERIFY(text.contains(QStringLiteral("<text")));
        QVERIFY(text.contains(QStringLiteral(">Quarterly &amp; yearly<")));
        QVERIFY(text.contains(QStringLiteral(">East<")));
        QVERIFY(text.contains(QStringLiteral(">West<")));
        QVERIFY(text.contains(QStringLiteral(">Revenue<")));   // the legend
        QVERIFY(text.contains(QStringLiteral("font-family=\"%1\"").arg(defaultTheme().minorLatin)));
        // An empty size draws nothing and doesn't fail.
        QVERIFY(chartSvg(c, QSizeF(0, 0)).contains("<svg"));
    }

    void chartSvgDrawsColumnsInProportion()
    {
        const QString red = QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></c:spPr>");
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("S"), {"a", "b", "c"}, {10, 20, 40}, red)
                             + QStringLiteral("<c:gapWidth val=\"100\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QImage img = renderChart(c, QSizeF(400, 300), 2);
        // Ink in the plot area besides the bars: gridlines and labels.
        int inked = 0, redPixels = 0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x) {
                const QRgb p = img.pixel(x, y);
                if (isColor(p, Qt::red, 40)) ++redPixels;
                else if (qRed(p) < 235 || qGreen(p) < 235 || qBlue(p) < 235) ++inked;
            }
        QVERIFY(inked > 400);
        QVERIFY(redPixels > 1000);
        const QVector<QRect> bars = colorRuns(img, Qt::red, false);
        QCOMPARE(int(bars.size()), 3);
        // Standing on one line, equally wide, in the order of their values and 1 : 2 : 4 high.
        QCOMPARE(bars[0].bottom(), bars[1].bottom());
        QCOMPARE(bars[1].bottom(), bars[2].bottom());
        QVERIFY(qAbs(bars[0].width() - bars[1].width()) <= 2 && qAbs(bars[1].width() - bars[2].width()) <= 2);
        QVERIFY(bars[0].left() < bars[1].left() && bars[1].left() < bars[2].left());
        QVERIFY2(qAbs(bars[1].height() / double(bars[0].height()) - 2) < 0.08, qPrintable(QString::number(bars[1].height() / double(bars[0].height()))));
        QVERIFY2(qAbs(bars[2].height() / double(bars[0].height()) - 4) < 0.15, qPrintable(QString::number(bars[2].height() / double(bars[0].height()))));
        // Gaps of 100% leave a bar's width between bars.
        QVERIFY2(qAbs((bars[1].left() - bars[0].right()) / double(bars[0].width()) - 1) < 0.1, qPrintable(QString::number((bars[1].left() - bars[0].right()) / double(bars[0].width()))));
        // The bars stand inside the picture, below the top and clear of the edges.
        QVERIFY(bars[0].left() > 20 && bars[2].right() < img.width() - 10);
        QVERIFY(bars[2].top() > 10 && bars[2].bottom() < img.height() - 20);
    }

    void chartSvgStacksColumns()
    {
        const QString red = QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></c:spPr>");
        const QString blue = QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"0000FF\"/></a:solidFill></c:spPr>");
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"stacked\"/>") + chartSeries(0, QStringLiteral("R"), {"a", "b"}, {10, 20}, red)
                             + chartSeries(1, QStringLiteral("B"), {"a", "b"}, {30, 10}, blue) + QStringLiteral("<c:gapWidth val=\"50\"/><c:overlap val=\"100\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QImage img = renderChart(c, QSizeF(400, 300), 2);
        const QVector<QRect> reds = colorRuns(img, Qt::red, false), blues = colorRuns(img, Qt::blue, false);
        QCOMPARE(int(reds.size()), 2);
        QCOMPARE(int(blues.size()), 2);
        // Each blue segment sits on its red one; the heights are the values, 10 + 30 and 20 + 10.
        QVERIFY(qAbs(blues[0].bottom() - reds[0].top()) <= 2);
        QVERIFY(qAbs(blues[1].bottom() - reds[1].top()) <= 2);
        QVERIFY2(qAbs(blues[0].height() / double(reds[0].height()) - 3) < 0.15, qPrintable(QString::number(blues[0].height() / double(reds[0].height()))));
        QVERIFY2(qAbs(reds[1].height() / double(blues[1].height()) - 2) < 0.1, qPrintable(QString::number(reds[1].height() / double(blues[1].height()))));
        const double total0 = reds[0].height() + blues[0].height(), total1 = reds[1].height() + blues[1].height();
        QVERIFY2(qAbs(total0 / total1 - 4.0 / 3.0) < 0.06, qPrintable(QString::number(total0 / total1)));
        // Percent stacked: both stacks reach the top.
        QString percent = plot;
        percent.replace(QStringLiteral("\"stacked\""), QStringLiteral("\"percentStacked\""));
        QVERIFY2(readChart(chartPart(percent), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QImage full = renderChart(c, QSizeF(400, 300), 2);
        const QVector<QRect> a = colorRuns(full, Qt::red, false), b = colorRuns(full, Qt::blue, false);
        QCOMPARE(int(b.size()), 2);
        QVERIFY(qAbs(b[0].top() - b[1].top()) <= 2);
        QVERIFY2(qAbs(a[0].height() / double(b[0].height()) - 1.0 / 3.0) < 0.05, qPrintable(QString::number(a[0].height() / double(b[0].height()))));
    }

    void chartSvgDrawsBarsAcross()
    {
        const QString red = QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></c:spPr>");
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"bar\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("S"), {"a", "b", "c"}, {10, 20, 40}, red)
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>")
                             + QStringLiteral("<c:catAx><c:axId val=\"111\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"l\"/><c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/>"
                                              "<c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"222\"/><c:crosses val=\"autoZero\"/><c:auto val=\"1\"/><c:lblAlgn val=\"ctr\"/><c:lblOffset val=\"100\"/></c:catAx>"
                                              "<c:valAx><c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"b\"/><c:majorGridlines/><c:numFmt formatCode=\"General\" sourceLinked=\"1\"/>"
                                              "<c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"111\"/><c:crosses val=\"autoZero\"/><c:crossBetween val=\"between\"/></c:valAx>");
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QVERIFY(c.plots[0].horizontal);
        const QImage img = renderChart(c, QSizeF(400, 300), 2);
        const QVector<QRect> bars = colorRuns(img, Qt::red, true);   // from the top
        QCOMPARE(int(bars.size()), 3);
        // The first category is at the bottom: from the top the bars are 40, 20, and 10 wide, from one edge.
        QCOMPARE(bars[0].left(), bars[1].left());
        QCOMPARE(bars[1].left(), bars[2].left());
        QVERIFY2(qAbs(bars[0].width() / double(bars[1].width()) - 2) < 0.08, qPrintable(QString::number(bars[0].width() / double(bars[1].width()))));
        QVERIFY2(qAbs(bars[1].width() / double(bars[2].width()) - 2) < 0.12, qPrintable(QString::number(bars[1].width() / double(bars[2].width()))));
        QVERIFY(qAbs(bars[0].height() - bars[2].height()) <= 2);
    }

    void chartSvgPutsReversedCategoriesFirstAtTheTop()
    {
        const QString red = QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></c:spPr>");
        auto bars = [&](const QString &order, const QString &crosses) {
            return QStringLiteral("<c:barChart><c:barDir val=\"bar\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("S"), {"a", "b", "c"}, {10, 20, 40}, red)
                   + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>")
                   + QStringLiteral("<c:catAx><c:axId val=\"111\"/><c:scaling><c:orientation val=\"%1\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"l\"/><c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/>"
                                    "<c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"222\"/><c:crosses val=\"autoZero\"/><c:auto val=\"1\"/><c:lblAlgn val=\"ctr\"/><c:lblOffset val=\"100\"/></c:catAx>"
                                    "<c:valAx><c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"b\"/><c:majorGridlines/><c:numFmt formatCode=\"General\" sourceLinked=\"1\"/>"
                                    "<c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"111\"/><c:crosses val=\"%2\"/><c:crossBetween val=\"between\"/></c:valAx>").arg(order, crosses);
        };
        // Where the value axis' labels are: dark text along the bottom of the picture, or along its top. (The bars are red, the lines gray.)
        auto darkIn = [](const QImage &img, int from, int to) {
            for (int y = from; y < to; ++y)
                for (int x = 0; x < img.width(); ++x)
                    if (qRed(img.pixel(x, y)) < 100 && qGreen(img.pixel(x, y)) < 100 && qBlue(img.pixel(x, y)) < 120) return true;
            return false;
        };
        Chart plain, turned, atTop;
        QString err;
        QVERIFY2(readChart(chartPart(bars(QStringLiteral("minMax"), QStringLiteral("autoZero"))), defaultTheme(), ColorMap(), &plain, &err), qPrintable(err));
        QVERIFY2(readChart(chartPart(bars(QStringLiteral("maxMin"), QStringLiteral("max"))), defaultTheme(), ColorMap(), &turned, &err), qPrintable(err));
        QVERIFY2(readChart(chartPart(bars(QStringLiteral("maxMin"), QStringLiteral("autoZero"))), defaultTheme(), ColorMap(), &atTop, &err), qPrintable(err));
        QVERIFY(plain.axis(111) && !plain.axis(111)->reversed);
        QVERIFY(turned.axis(111)->reversed);
        // The first category is at the bottom, or at the top once the categories are turned round.
        const QVector<QRect> up = colorRuns(renderChart(plain, QSizeF(400, 300), 2), Qt::red, true), down = colorRuns(renderChart(turned, QSizeF(400, 300), 2), Qt::red, true);
        QCOMPARE(int(up.size()), 3);
        QCOMPARE(int(down.size()), 3);
        QVERIFY(up[0].width() > up[2].width());
        QVERIFY(down[0].width() < down[2].width());
        // The value axis crosses at the first category, which turned round is the top, unless it is said to cross at the last.
        const QImage a = renderChart(plain, QSizeF(400, 300), 2), b = renderChart(turned, QSizeF(400, 300), 2), t = renderChart(atTop, QSizeF(400, 300), 2);
        QVERIFY(darkIn(a, 540, 600) && !darkIn(a, 0, 60));
        QVERIFY(darkIn(b, 540, 600) && !darkIn(b, 0, 60));
        QVERIFY(darkIn(t, 0, 60) && !darkIn(t, 540, 600));
    }

    void chartSvgDrawsPieSlices()
    {
        const QString colored = QStringLiteral("<c:dPt><c:idx val=\"0\"/><c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></c:spPr></c:dPt>"
                                               "<c:dPt><c:idx val=\"1\"/><c:spPr><a:solidFill><a:srgbClr val=\"0000FF\"/></a:solidFill></c:spPr></c:dPt>"
                                               "<c:dPt><c:idx val=\"2\"/><c:spPr><a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill></c:spPr></c:dPt>");
        const QString plot = QStringLiteral("<c:pieChart><c:varyColors val=\"1\"/>") + chartSeries(0, QStringLiteral("S"), {"a", "b", "c"}, {1, 1, 2}, QString(), colored)
                             + QStringLiteral("<c:firstSliceAng val=\"0\"/></c:pieChart>");
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QImage img = renderChart(c, QSizeF(300, 300), 2);
        const QColor green(0, 0xFF, 0);
        const double r = colorArea(img, Qt::red), b = colorArea(img, Qt::blue), g = colorArea(img, green);
        QVERIFY(r > 5000);
        QVERIFY2(qAbs(b / r - 1) < 0.08, qPrintable(QString::number(b / r)));
        QVERIFY2(qAbs(g / r - 2) < 0.12, qPrintable(QString::number(g / r)));
        // The first slice starts at twelve o'clock and runs clockwise: red upper right, blue lower right, green the left half.
        const QPointF cr = colorCentroid(img, Qt::red), cb = colorCentroid(img, Qt::blue), cg = colorCentroid(img, green);
        QVERIFY(cr.x() > cg.x() && cb.x() > cg.x());
        QVERIFY(cr.y() < cb.y());
        QVERIFY(qAbs(cg.y() - (cr.y() + cb.y()) / 2) < 12);
        // A pie is round: its sides are as wide as it is high.
        const QRect box = colorBounds(img, QColor(), true);
        QVERIFY2(qAbs(box.width() - box.height()) <= 3, qPrintable(QString("%1 x %2").arg(box.width()).arg(box.height())));
        // Percentages as the slices' labels, as text.
        QString labeled = plot;
        labeled.replace(QStringLiteral("<c:firstSliceAng"),
                        QStringLiteral("<c:dLbls><c:showLegendKey val=\"0\"/><c:showVal val=\"0\"/><c:showCatName val=\"0\"/><c:showSerName val=\"0\"/><c:showPercent val=\"1\"/><c:showBubbleSize val=\"0\"/></c:dLbls><c:firstSliceAng"));
        QVERIFY2(readChart(chartPart(labeled), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QVERIFY(c.plots[0].series[0].labels.percent);   // the plot's labels are the series'
        const QString svg = QString::fromUtf8(chartSvg(c, QSizeF(300, 300)));
        QVERIFY(svg.contains(QStringLiteral(">50%<")));
        QCOMPARE(int(svg.count(QStringLiteral(">25%<"))), 2);
    }

    void chartSvgDrawsLinesAndMarkers()
    {
        const QString line = QStringLiteral("<c:spPr><a:ln w=\"28575\"><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></a:ln></c:spPr>");
        const QString plot = QStringLiteral("<c:lineChart><c:grouping val=\"standard\"/>") + chartSeries(0, QStringLiteral("S"), {"a", "b", "c", "d"}, {1, 2, 3, 4}, line,
                                                                                                      QStringLiteral("<c:marker><c:symbol val=\"square\"/><c:size val=\"9\"/><c:spPr><a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill></c:spPr></c:marker>"))
                             + QStringLiteral("<c:marker val=\"1\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:lineChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QImage img = renderChart(c, QSizeF(400, 300), 2);
        QVERIFY(colorArea(img, Qt::red) > 500);
        // Four square markers, in the file's color, along the rising line.
        const QVector<QPointF> markers = colorComponents(img, QColor(0, 0xFF, 0), 20);
        QCOMPARE(int(markers.size()), 4);
        for (int i = 1; i < markers.size(); ++i) {
            QVERIFY(markers[i].x() > markers[i - 1].x());
            QVERIFY(markers[i].y() < markers[i - 1].y());
        }
        // Equal steps in the values are equal steps up the page.
        QVERIFY2(qAbs((markers[1].y() - markers[0].y()) - (markers[3].y() - markers[2].y())) < 2, "unequal steps");
        // Points sit in the middle of their categories.
        QVERIFY(qAbs((markers[1].x() - markers[0].x()) - (markers[3].x() - markers[2].x())) < 2);
    }

    void chartSvgDrawsScatterPoints()
    {
        const QString plot = QStringLiteral("<c:scatterChart><c:scatterStyle val=\"lineMarker\"/>")
                             + QStringLiteral("<c:ser><c:idx val=\"0\"/><c:order val=\"0\"/><c:tx><c:v>P</c:v></c:tx><c:spPr><a:ln w=\"19050\"><a:noFill/></a:ln></c:spPr>"
                                              "<c:marker><c:symbol val=\"circle\"/><c:size val=\"9\"/><c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill><a:ln><a:noFill/></a:ln></c:spPr></c:marker>"
                                              "<c:xVal><c:numLit><c:ptCount val=\"3\"/><c:pt idx=\"0\"><c:v>1</c:v></c:pt><c:pt idx=\"1\"><c:v>2</c:v></c:pt><c:pt idx=\"2\"><c:v>4</c:v></c:pt></c:numLit></c:xVal>"
                                              "<c:yVal><c:numLit><c:ptCount val=\"3\"/><c:pt idx=\"0\"><c:v>1</c:v></c:pt><c:pt idx=\"1\"><c:v>4</c:v></c:pt><c:pt idx=\"2\"><c:v>9</c:v></c:pt></c:numLit></c:yVal></c:ser>")
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:scatterChart>")
                             + QStringLiteral("<c:valAx><c:axId val=\"111\"/><c:scaling><c:orientation val=\"minMax\"/><c:max val=\"5\"/><c:min val=\"0\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"b\"/><c:numFmt formatCode=\"General\" sourceLinked=\"1\"/>"
                                              "<c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"222\"/><c:crosses val=\"autoZero\"/><c:crossBetween val=\"midCat\"/></c:valAx>"
                                              "<c:valAx><c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/><c:max val=\"10\"/><c:min val=\"0\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"l\"/><c:majorGridlines/><c:numFmt formatCode=\"General\" sourceLinked=\"1\"/>"
                                              "<c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"111\"/><c:crosses val=\"autoZero\"/><c:crossBetween val=\"midCat\"/></c:valAx>");
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QImage img = renderChart(c, QSizeF(400, 300), 2);
        const QVector<QPointF> dots = colorComponents(img, Qt::red, 20);
        QCOMPARE(int(dots.size()), 3);   // markers alone, not joined
        // Placed by their values on both axes: x at 1, 2, 4 of 0 to 5, and y at 1, 4, 9 of 0 to 10.
        const double dx1 = dots[1].x() - dots[0].x(), dx2 = dots[2].x() - dots[1].x();
        const double dy1 = dots[0].y() - dots[1].y(), dy2 = dots[1].y() - dots[2].y();
        QVERIFY2(qAbs(dx2 / dx1 - 2) < 0.05, qPrintable(QString::number(dx2 / dx1)));
        QVERIFY2(qAbs(dy2 / dy1 - 5.0 / 3.0) < 0.05, qPrintable(QString::number(dy2 / dy1)));
        // The file's 0 to 10 is labeled in steps of 1, and goes no further.
        const QString svg = QString::fromUtf8(chartSvg(c, QSizeF(400, 300)));
        QVERIFY(svg.contains(QStringLiteral(">10<")));
        QVERIFY(!svg.contains(QStringLiteral(">12<")));
    }

    void chartSvgDrawsRadarPointsOnTheirSpokes()
    {
        const QString line = QStringLiteral("<c:spPr><a:ln w=\"28575\"><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></a:ln></c:spPr>");
        const QString marker = QStringLiteral("<c:marker><c:symbol val=\"square\"/><c:size val=\"9\"/><c:spPr><a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill></c:spPr></c:marker>");
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(radarPlot("marker", line, marker)), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QImage img = renderChart(c, QSizeF(400, 300), 2);
        QVector<QPointF> m = colorComponents(img, QColor(0, 0xFF, 0), 20);
        QCOMPARE(int(m.size()), 4);
        std::sort(m.begin(), m.end(), [](const QPointF &a, const QPointF &b) { return a.y() < b.y(); });
        const QPointF top = m[0], bottom = m[3];
        QPointF left = m[1], right = m[2];
        if (left.x() > right.x()) std::swap(left, right);
        const double cx = (top.x() + bottom.x()) / 2, cy = (top.y() + bottom.y()) / 2, rim = (bottom.y() - top.y()) / 2;
        QVERIFY(qAbs(top.x() - bottom.x()) < 2);                                 // up and down the same spoke line
        QVERIFY2(qAbs(left.y() - cy) < 2 && qAbs(right.y() - cy) < 2, "the side points are level with the middle");
        QVERIFY2(qAbs((right.x() - cx) - (cx - left.x())) < 2, "the side points are as far to either side");
        QVERIFY2(qAbs((right.x() - cx) / rim - 0.5) < 0.03, qPrintable(QString::number((right.x() - cx) / rim)));   // 2 of 4
        QVERIFY(rim > 120);                                               // the figure fills the picture, not a corner of it
        // joined: the middle of the edge from the left point to the top one is on the red line
        const QPointF mid = (left + top) / 2;
        QVERIFY(colorArea(img.copy(int(mid.x()) - 4, int(mid.y()) - 4, 9, 9), Qt::red) > 6);
        // not filled: the middle of the shape is white
        QVERIFY(!isColor(img.pixel(int(cx) + 7, int(cy) + 13), Qt::red, 60));
    }

    void chartSvgFillsARadarShape()
    {
        const QString fill = QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></c:spPr>");
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(radarPlot("filled", fill, QString())), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QImage img = renderChart(c, QSizeF(400, 300), 2);
        const QRect box = colorBounds(img, Qt::red, false);
        QVERIFY(!box.isEmpty());
        const double rim = box.height() / 2.0;   // the top and bottom points are at the rim
        // a diamond of half-diagonals rim and rim / 2 holds rim squared
        const double area = colorArea(img, Qt::red);
        QVERIFY2(qAbs(area / (rim * rim) - 1) < 0.08, qPrintable(QString::number(area / (rim * rim))));
        QVERIFY2(qAbs(box.width() / (rim * 2 * 0.5) - 1) < 0.08, qPrintable(QString::number(box.width() / (rim * 2 * 0.5))));
        // its middle is filled
        QVERIFY(isColor(img.pixel(box.center().x(), box.center().y()), Qt::red, 40));
    }

    // Many spokes are drawn from fewer of them, and the picture stays small and says so.
    void chartSvgDrawsAManySpokedRadarCoarser()
    {
        QStringList cats;
        QVector<double> values;
        for (int i = 0; i < 5000; ++i) {
            cats << QString::number(i);
            values << 1 + (i % 7);
        }
        const QString plot = QStringLiteral("<c:radarChart><c:radarStyle val=\"marker\"/>") + chartSeries(0, QStringLiteral("R"), cats, values) + chartSeries(1, QStringLiteral("S"), cats, values)
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:radarChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        bool coarser = false;
        const QByteArray svg = chartSvg(c, QSizeF(400, 300), &coarser);
        QVERIFY(coarser);
        // Counted: no shape of the picture has more corners than the 500 spokes a radar is drawn with (5,000 were read; without the rule 1,667 are left, from every third), and the picture
        // is held to its tags.
        int corners = 0;
        for (const QRegularExpressionMatch &m : QRegularExpression(QStringLiteral(" points=\"([^\"]*)\"")).globalMatch(QString::fromUtf8(svg))) corners = std::max<int>(corners, m.captured(1).count(QLatin1Char(',')));
        QVERIFY2(corners > 100 && corners <= 500 && svg.count('<') < 6000, qPrintable(QStringLiteral("%1 corners, %2 tags").arg(corners).arg(svg.count('<'))));
        QVERIFY(svg.count("<text") < 300);
    }

    void chartSvgNamesRadarCategoriesAroundTheRim()
    {
        const QString plot = QStringLiteral("<c:radarChart><c:radarStyle val=\"standard\"/>") + chartSeries(0, QStringLiteral("R"), {"North", "East", "South", "Far far West"}, {1, 2, 3, 4})
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:radarChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QString svg = QString::fromUtf8(chartSvg(c, QSizeF(400, 300)));
        for (const char *name : {"North", "East", "South", "Far far West"}) QVERIFY2(svg.contains(QStringLiteral(">%1<").arg(QLatin1String(name))), name);
        // the value axis is marked along the top spoke, to the scale's end
        QVERIFY(svg.contains(QStringLiteral(">4<")) && svg.contains(QStringLiteral(">0<")));
        QVERIFY(!svg.contains(QStringLiteral(">6<")));
        // Where each name is, from the picture's own text: North above the middle with East, South, and West following clockwise; each name flush against the spoke it is beside
        {
            const QVector<TextAt> t = textsOf(chartSvg(c, QSizeF(400, 300)));
            QVERIFY2(t.size() >= 4, qPrintable(QString::number(t.size())));
            const TextAt n = textNamed(t, "North"), e = textNamed(t, "East"), so = textNamed(t, "South"), w = textNamed(t, "Far far West");
            const double mid = n.x + n.width / 2;
            QVERIFY2(qAbs(so.x + so.width / 2 - mid) < 3, "North and South are on one vertical line, each centered on it");
            QVERIFY2(e.x > mid + 2, "East is to the right of the top spoke");
            // flush against the rim: East starts as far right of the middle as West ends left of it, whatever their lengths
            QVERIFY2(qAbs((e.x - mid) - (mid - (w.x + w.width))) < 2, qPrintable(QStringLiteral("%1 and %2").arg(e.x - mid).arg(mid - (w.x + w.width))));
            QVERIFY2(w.x + w.width < mid - 2, "West is to the left of the top spoke");
            QVERIFY2(n.y < e.y && e.y < so.y, "from the top: North, East, South");
            QVERIFY2(qAbs(e.y - w.y) < 1 && qAbs(e.y - (n.y + so.y) / 2) < 1, "East and West are level, halfway between North and South");
            // the first name is above the middle of the figure, not beside it
            QVERIFY2(n.y < e.y - 20 && so.y > e.y + 20, "North is well above the middle and South well below it");
        }
        {
            // three names: the first at the top, the second to the lower right, the third to the lower left
            const QString three = QStringLiteral("<c:radarChart><c:radarStyle val=\"standard\"/>") + chartSeries(0, QStringLiteral("R"), {"Alpha", "Bravo", "Charlie"}, {3, 2, 1})
                                  + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:radarChart>") + chartAxes();
            Chart c3;
            QVERIFY2(readChart(chartPart(three), defaultTheme(), ColorMap(), &c3, &err), qPrintable(err));
            const QVector<TextAt> t = textsOf(chartSvg(c3, QSizeF(400, 300)));
            const TextAt a = textNamed(t, "Alpha"), b = textNamed(t, "Bravo"), ch = textNamed(t, "Charlie");
            const double mid = a.x + a.width / 2;
            QVERIFY2(b.x > mid && ch.x + ch.width < mid, "the second name is right of the first one's spoke and the third left of it");
            QVERIFY2(a.y < b.y && qAbs(b.y - ch.y) < 1, "the first is highest; the second and third are level");
        }
        // North is named at the top of the figure, South at the bottom, West at the left, and East at the right: dark text there, in the middle of the edges
        const QImage img = renderChart(c, QSizeF(400, 300), 2);
        auto dark = [&](const QRect &r) {
            int n = 0;
            for (int y = r.top(); y < r.bottom(); ++y)
                for (int x = r.left(); x < r.right(); ++x) n += qRed(img.pixel(x, y)) < 110 && qGreen(img.pixel(x, y)) < 110 && qBlue(img.pixel(x, y)) < 110;
            return n;
        };
        const int w = img.width(), h = img.height();
        QVERIFY(dark(QRect(w * 4 / 10, 0, w / 5, h / 8)) > 15);                 // top
        QVERIFY(dark(QRect(w * 4 / 10, h - h / 8, w / 5, h / 8 - 2)) > 15);     // bottom
        QVERIFY(dark(QRect(0, h * 4 / 10, w / 5, h / 5)) > 15);                 // left
        QVERIFY(dark(QRect(w - w / 5, h * 4 / 10, w / 5 - 2, h / 5)) > 15);     // right
    }

    // One category has no shape to draw: the spoke and the scale are there and the series is not. Two draw the series as a line from the first category's point, at the top, to the second's, straight below
    // (a filled one shows nothing). Never an empty plot, never a dot with nothing around it.
    void aRadarOfOneOrTwoCategoriesDrawsItsSpokesAndTheLineBetweenTwo()
    {
        const QString red = QStringLiteral("<c:spPr><a:ln w=\"28575\"><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></a:ln></c:spPr>");
        const QString redFill = QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></c:spPr>");
        auto make = [&](const char *style, const QStringList &cats, const QVector<double> &values, const QString &spPr, const QString &marker = QString()) {
            const QString plot = QStringLiteral("<c:radarChart><c:radarStyle val=\"%1\"/>").arg(QLatin1String(style)) + chartSeries(0, QStringLiteral("R"), cats, values, spPr, marker)
                                 + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:radarChart>") + radarAxes();
            Chart c;
            QString err;
            [&] { QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err)); }();
            return c;
        };
        // one category: nothing of the series, but the top spoke is drawn (gray pixels above the middle of the picture's figure)
        {
            const Chart c = make("standard", {"Only"}, {3}, red);
            const QImage img = renderChart(c, QSizeF(400, 300), 2);
            QVERIFY(colorArea(img, Qt::red) == 0);
            // the spoke: a vertical run of gridline-colored pixels
            int best = 0;
            for (int x = 0; x < img.width(); ++x) {
                int run = 0;
                for (int y = 0; y < img.height(); ++y) {
                    const QRgb p = img.pixel(x, y);
                    run = (qRed(p) < 235 && qRed(p) > 100 && qAbs(qRed(p) - qBlue(p)) < 12) ? run + 1 : 0;
                    best = std::max(best, run);
                }
            }
            QVERIFY2(best > 60, qPrintable(QString::number(best)));
            // not a dot either, with markers on
            const QString marker = QStringLiteral("<c:marker><c:symbol val=\"square\"/><c:size val=\"9\"/><c:spPr><a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill></c:spPr></c:marker>");
            QCOMPARE(colorArea(renderChart(make("marker", {"Only"}, {3}, red, marker), QSizeF(400, 300), 2), QColor(0, 0xFF, 0)), 0.0);
        }
        // two categories: a vertical line of the series from the top (value 3 of 4, three quarters of the way out) to straight below (value 1: a quarter of the way out the other way)
        {
            const Chart c = make("standard", {"Up", "Down"}, {4, 2}, red);
            const QImage img = renderChart(c, QSizeF(400, 300), 2);
            const QRect box = colorBounds(img, Qt::red, false);
            QVERIFY(!box.isEmpty());
            QVERIFY2(box.width() < 8, qPrintable(QString::number(box.width())));
            // from the rim at the top to the middle of the half-way mark below it: 1.5 radii; the figure's radius is half the height of the gray rings
            QVERIFY2(box.height() > 60, qPrintable(QString::number(box.height())));
            // the series line is all one vertical run of red
            int redRows = 0;
            for (int y = box.top(); y <= box.bottom(); ++y) redRows += colorArea(img.copy(box.left(), y, box.width(), 1), Qt::red) > 0;
            QVERIFY(redRows >= box.height() - 2);
            // filled: nothing to see
            const Chart f = make("filled", {"Up", "Down"}, {4, 2}, redFill);
            QCOMPARE(colorArea(renderChart(f, QSizeF(400, 300), 2), Qt::red), 0.0);
        }
    }

    // The marker and color a file gives a single point of a radar are drawn: the point's own symbol, size, and fill, and its color for a marker that has none of its own.
    void aRadarDrawsTheMarkersAndColorsOfSinglePoints()
    {
        const QString line = QStringLiteral("<c:spPr><a:ln w=\"28575\"><a:solidFill><a:srgbClr val=\"888888\"/></a:solidFill></a:ln></c:spPr>");
        const QString marker = QStringLiteral("<c:marker><c:symbol val=\"circle\"/><c:size val=\"9\"/><c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></c:spPr></c:marker>");
        const QString one = QStringLiteral("<c:dPt><c:idx val=\"1\"/><c:marker><c:symbol val=\"square\"/><c:size val=\"13\"/><c:spPr><a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill></c:spPr></c:marker><c:bubble3D val=\"0\"/></c:dPt>");
        const QString three = QStringLiteral("<c:dPt><c:idx val=\"3\"/><c:spPr><a:solidFill><a:srgbClr val=\"0000FF\"/></a:solidFill></c:spPr></c:dPt>");
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(radarPlot("marker", line, marker + one + three)), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QImage img = renderChart(c, QSizeF(400, 300), 2);
        const QVector<QPointF> green = colorComponents(img, QColor(0, 0xFF, 0), 20), blue = colorComponents(img, QColor(0, 0, 0xFF), 20), red = colorComponents(img, Qt::red, 20);
        QCOMPARE(int(green.size()), 1);   // the square of the second point
        QCOMPARE(int(blue.size()), 1);    // the fourth point, in its own color
        QCOMPARE(int(red.size()), 2);     // the first and third, as the series has them
        QVERIFY(green[0].x() > blue[0].x() + 20);   // the second point is to the right, the fourth to the left
        // the square is the bigger: size 13 against 9
        QVERIFY(colorArea(img, QColor(0, 0xFF, 0)) > colorArea(img, QColor(0, 0, 0xFF)) * 1.3);
    }

    // A line radar joins across a blank as the chart says empty cells are shown (gaps and "connect" join the points either side; zero goes to the middle), and a filled one closes over it the same way.
    void aRadarDrawsABlankAsTheEmptyCellsSettingSays()
    {
        const QString green = QStringLiteral("<c:marker><c:symbol val=\"square\"/><c:size val=\"9\"/><c:spPr><a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill></c:spPr></c:marker>");
        const QString red = QStringLiteral("<c:spPr><a:ln w=\"28575\"><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></a:ln></c:spPr>");
        const QString redFill = QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></c:spPr>");
        auto make = [&](const char *style, const QStringList &cats, const QVector<double> &values, const QString &spPr, const QString &marker, const char *blanks) {
            const QString plot = QStringLiteral("<c:radarChart><c:radarStyle val=\"%1\"/>").arg(QLatin1String(style)) + chartSeries(0, QStringLiteral("R"), cats, values, spPr, marker)
                                 + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:radarChart>") + radarAxes();
            Chart c;
            QString err;
            [&] { QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err)); }();
            c.blanksAs = QLatin1String(blanks);
            return renderChart(c, QSizeF(400, 300), 2);
        };
        auto redAt = [](const QImage &img, const QPointF &p) { return colorArea(img.copy(int(p.x()) - 4, int(p.y()) - 4, 9, 9), Qt::red); };
        // three spokes: the first at the top, the third to the lower left, the second (blank) to the lower right
        for (const char *blanks : {"gap", "span", "zero"}) {
            const QImage img = make("marker", {"a", "b", "c"}, {4, std::nan(""), 4}, red, green, blanks);
            const QVector<QPointF> m = colorComponents(img, QColor(0, 0xFF, 0), 20);
            QCOMPARE(int(m.size()), 2);   // a blank has no marker, whichever way it is shown
            const QPointF top = m[0].y() < m[1].y() ? m[0] : m[1], other = m[0].y() < m[1].y() ? m[1] : m[0];
            const double r = (other.y() - top.y()) / 1.5;
            const QPointF centre(top.x(), top.y() + r), middle = (top + other) / 2;
            if (QLatin1String(blanks) == QLatin1String("zero")) {
                QVERIFY2(redAt(img, centre) > 6, blanks);   // down to the middle and out again
            } else {
                QVERIFY2(redAt(img, middle) > 6 && redAt(img, centre) == 0, blanks);   // straight from one point to the other
            }
        }
        // filled, five spokes with the third blank: closed straight over it, or in to the middle
        for (const char *blanks : {"gap", "span", "zero"}) {
            const QImage img = make("filled", {"a", "b", "c", "d", "e"}, {4, 4, std::nan(""), 4, 4}, redFill, QString(), blanks);
            const QRect box = colorBounds(img, Qt::red, false);
            QVERIFY(!box.isEmpty());
            const double r = box.width() / (2 * 0.951);
            const QPointF below(box.center().x(), box.top() + r + 0.2 * r);   // a little below the middle, in the notch
            if (QLatin1String(blanks) == QLatin1String("zero")) QVERIFY2(redAt(img, below) == 0, blanks);
            else QVERIFY2(redAt(img, below) > 6, blanks);
        }
    }

    // Forty names all stand around the rim, none left out, and none is over the title.
    void aRadarOfFortyCategoriesNamesThemAllAndKeepsClearOfTheTitle()
    {
        QStringList cats;
        QVector<double> values;
        for (int i = 1; i <= 40; ++i) {
            cats << QStringLiteral("C%1").arg(i);
            values << 1 + i % 7;
        }
        const QString plot = QStringLiteral("<c:radarChart><c:radarStyle val=\"standard\"/>") + chartSeries(0, QStringLiteral("R"), cats, values) + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:radarChart>") + chartAxes();
        const QString title = QStringLiteral("<c:title><c:tx><c:rich><a:bodyPr/><a:p><a:r><a:t>Models</a:t></a:r></a:p></c:rich></c:tx><c:overlay val=\"0\"/></c:title>");
        for (const QSizeF &size : {QSizeF(400, 300), QSizeF(300, 240), QSizeF(500, 280), QSizeF(260, 220)}) {
            Chart c;
            QString err;
            QVERIFY2(readChart(chartPart(plot, title), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
            const QVector<TextAt> texts = textsOf(chartSvg(c, size));
            QVector<QRectF> names;
            QRectF head;
            for (const TextAt &t : texts) {
                const double px = 10;
                if (t.text == QLatin1String("Models")) head = QRectF(t.x, t.y - 18, t.width, 21);
                else if (QRegularExpression(QStringLiteral("^C[0-9]+$")).match(t.text).hasMatch()) names << QRectF(t.x, t.y - 0.8 * px, t.width, 1.2 * px);
            }
            QVERIFY(!head.isEmpty());
            QVERIFY2(names.size() == 40, qPrintable(QStringLiteral("%1 names at %2 wide").arg(names.size()).arg(size.width())));
            for (const QRectF &n : std::as_const(names)) QVERIFY2(!n.intersects(head), qPrintable(QStringLiteral("a name is over the title at %1 wide").arg(size.width())));
        }
    }

    // A radar beside another radar on a second pair of axes is drawn on the first scale and is not written over: its part is kept as the file has it.
    void aRadarOnASecondPairOfAxesIsDrawnOnTheFirstScaleAndKept()
    {
        const QString red = QStringLiteral("<c:spPr><a:ln w=\"28575\"><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></a:ln></c:spPr>");
        const QString green = QStringLiteral("<c:spPr><a:ln w=\"28575\"><a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill></a:ln></c:spPr>");
        const QString body = QStringLiteral("<c:radarChart><c:radarStyle val=\"standard\"/><c:varyColors val=\"0\"/>") + chartSeries(0, QStringLiteral("A"), {"a", "b", "c", "d"}, {4, 2, 4, 2}, red)
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:radarChart><c:radarChart><c:radarStyle val=\"standard\"/><c:varyColors val=\"0\"/>")
                             + chartSeries(1, QStringLiteral("B"), {"a", "b", "c", "d"}, {2, 4, 2, 4}, green) + QStringLiteral("<c:axId val=\"333\"/><c:axId val=\"444\"/></c:radarChart>") + radarAxes()
                             + QStringLiteral("<c:valAx><c:axId val=\"444\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"r\"/><c:crossAx val=\"333\"/><c:crosses val=\"max\"/></c:valAx>"
                                              "<c:catAx><c:axId val=\"333\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"1\"/><c:axPos val=\"b\"/><c:crossAx val=\"444\"/></c:catAx>");
        const QByteArray part = chartPart(body);
        Chart c;
        QString err;
        QVERIFY2(readChart(part, defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(c.plots.size()), 2);
        QCOMPARE(int(c.plots[0].type), int(ChartType::Radar));
        QCOMPARE(int(c.plots[1].type), int(ChartType::Radar));
        const QImage img = renderChart(c, QSizeF(400, 300), 2);
        QVERIFY(colorArea(img, Qt::red) > 100);
        QVERIFY(colorArea(img, QColor(0, 0xFF, 0)) > 100);
        QVERIFY(!chartIsFullyModeled(part));
    }

    // (With CHART_SHOT_DIR set, the three looks of a radar chart of five categories and two series are kept as pictures.)
    void chartSvgRadarShots()
    {
        const QString dir = qEnvironmentVariable("CHART_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("CHART_SHOT_DIR is not set");
        const QStringList cats = {"Speed", "Power", "Range", "Comfort", "Price"};
        const QString lines = QStringLiteral("<c:spPr><a:ln w=\"28575\"><a:solidFill><a:srgbClr val=\"4472C4\"/></a:solidFill></a:ln></c:spPr>");
        for (const char *style : {"standard", "marker", "filled"}) {
            const QString plot = QStringLiteral("<c:radarChart><c:radarStyle val=\"%1\"/>").arg(QLatin1String(style))
                                 + chartSeries(0, QStringLiteral("Model A"), cats, {8, 5, 7, 9, 4}) + chartSeries(1, QStringLiteral("Model B"), cats, {5, 8, 4, 6, 9})
                                 + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:radarChart>") + chartAxes();
            Chart c;
            QString err;
            QVERIFY2(readChart(chartPart(plot, QStringLiteral("<c:title><c:tx><c:rich><a:bodyPr/><a:p><a:r><a:t>Models</a:t></a:r></a:p></c:rich></c:tx><c:overlay val=\"0\"/></c:title>"),
                                         QStringLiteral("<c:legend><c:legendPos val=\"b\"/></c:legend>")),
                              defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
            QVERIFY(renderChart(c, QSizeF(432, 288), 2).save(QStringLiteral("%1/radar-%2.png").arg(dir, QLatin1String(style))));
        }
        Q_UNUSED(lines);
    }

    // (With CHART_KINDS_DIR set, the pictures of the kinds that stock must not change, and of the stock forms, are kept as SVG files, to be compared before and after a change to the drawing.)
    void chartSvgUnchangedKindsShots()
    {
        const QString dir = qEnvironmentVariable("CHART_KINDS_DIR");
        if (dir.isEmpty()) QSKIP("CHART_KINDS_DIR is not set");
        const QStringList cats = {"a", "b", "c", "d"};
        const QString two = chartSeries(0, QStringLiteral("A"), cats, {3, 5, 4, 6}) + chartSeries(1, QStringLiteral("B"), cats, {2, 4, 6, 3});
        const QString axes = QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/>");
        const QString bars = QStringLiteral("<c:barChart><c:barDir val=\"%1\"/><c:grouping val=\"clustered\"/>");
        const QMap<QString, QString> kinds = {
            {QStringLiteral("column"), bars.arg(QStringLiteral("col")) + two + axes + QStringLiteral("</c:barChart>") + chartAxes()},
            {QStringLiteral("bar"), bars.arg(QStringLiteral("bar")) + two + axes + QStringLiteral("</c:barChart>") + chartAxes()},
            {QStringLiteral("line"), QStringLiteral("<c:lineChart><c:grouping val=\"standard\"/>") + two + QStringLiteral("<c:marker val=\"1\"/>") + axes + QStringLiteral("</c:lineChart>") + chartAxes()},
            {QStringLiteral("lineHiLow"), QStringLiteral("<c:lineChart><c:grouping val=\"standard\"/>") + two + QStringLiteral("<c:hiLowLines/><c:upDownBars><c:gapWidth val=\"150\"/><c:upBars/><c:downBars/></c:upDownBars>")
                                              + QStringLiteral("<c:marker val=\"1\"/>") + axes + QStringLiteral("</c:lineChart>") + chartAxes()},
            {QStringLiteral("area"), QStringLiteral("<c:areaChart><c:grouping val=\"standard\"/>") + two + axes + QStringLiteral("</c:areaChart>") + chartAxes()},
            {QStringLiteral("pie"), QStringLiteral("<c:pieChart><c:varyColors val=\"1\"/>") + chartSeries(0, QStringLiteral("A"), cats, {3, 5, 4, 6}) + QStringLiteral("<c:firstSliceAng val=\"0\"/></c:pieChart>")},
            {QStringLiteral("doughnut"), QStringLiteral("<c:doughnutChart><c:varyColors val=\"1\"/>") + chartSeries(0, QStringLiteral("A"), cats, {3, 5, 4, 6}) + QStringLiteral("<c:holeSize val=\"50\"/></c:doughnutChart>")},
            {QStringLiteral("scatter"), QStringLiteral("<c:scatterChart><c:scatterStyle val=\"lineMarker\"/>") + chartSeries(0, QStringLiteral("A"), cats, {3, 5, 4, 6}) + axes + QStringLiteral("</c:scatterChart>") + chartAxes()},
            {QStringLiteral("radar"), QStringLiteral("<c:radarChart><c:radarStyle val=\"marker\"/>") + two + axes + QStringLiteral("</c:radarChart>") + radarAxes()},
            {QStringLiteral("stockOneSeries"), QStringLiteral("<c:stockChart>") + chartSeries(0, QStringLiteral("A"), cats, {3, 5, 4, 6}) + axes + QStringLiteral("</c:stockChart>") + chartAxes()},
            {QStringLiteral("stockFormHLC"), stockPlot({{9, 8, 7, 6}, {1, 4, 2, 3}, {5, 7, 3, 5}}, false) + stockAxes(10)},
            {QStringLiteral("stockFormVHLC"), volumePlot({100, 50, 150, 200}) + stockPlot({{9, 8, 7, 6}, {1, 4, 2, 3}, {5, 7, 3, 5}}, false, QStringLiteral("<c:axId val=\"333\"/><c:axId val=\"444\"/>"), 1) + stockAxes(200) + stockAxes(10, true)},
            {QStringLiteral("stockTwoSeries"), QStringLiteral("<c:stockChart>") + two + axes + QStringLiteral("</c:stockChart>") + chartAxes()},
        };
        for (auto it = kinds.cbegin(); it != kinds.cend(); ++it) {
            Chart c;
            QString err;
            QVERIFY2(readChart(chartPart(it.value()), defaultTheme(), ColorMap(), &c, &err), qPrintable(it.key() + err));
            QFile f(QStringLiteral("%1/%2.svg").arg(dir, it.key()));
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(chartSvg(c, QSizeF(400, 300)));
        }
    }

    // ---- stock charts: the four forms (high-low-close; open-high-low-close; with the volume before them), told apart by how many series there are and the column plot of the volume

private:
    static QString stockLook() { return QStringLiteral("<c:spPr><a:ln w=\"19050\"><a:noFill/></a:ln></c:spPr>"); }
    // the close's mark: a dash, in blue
    static QString closeMark()
    {
        return QStringLiteral("<c:marker><c:symbol val=\"dash\"/><c:size val=\"9\"/><c:spPr><a:solidFill><a:srgbClr val=\"0000FF\"/></a:solidFill><a:ln><a:solidFill><a:srgbClr val=\"0000FF\"/></a:solidFill></a:ln></c:spPr></c:marker>");
    }
    static QString noMark() { return QStringLiteral("<c:marker><c:symbol val=\"none\"/></c:marker>"); }
    // A stock plot of the price series (high, low, close, or open first), the high-low lines red and 1 point wide, the up bars hollow with a blue edge and the down bars green.
    static QString stockPlot(const QVector<QVector<double>> &prices, bool bars, const QString &axes = QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/>"), int firstIndex = 0, const QStringList &cats = {"a", "b", "c", "d"})
    {
        QString out = QStringLiteral("<c:stockChart>");
        for (int i = 0; i < prices.size(); ++i) out += chartSeries(firstIndex + i, QStringLiteral("P%1").arg(i), cats, prices[i], stockLook(), i == prices.size() - 1 && !bars ? closeMark() : noMark());
        out += QStringLiteral("<c:hiLowLines><c:spPr><a:ln w=\"12700\"><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></a:ln></c:spPr></c:hiLowLines>");
        if (bars)
            out += QStringLiteral("<c:upDownBars><c:gapWidth val=\"150\"/><c:upBars><c:spPr><a:solidFill><a:srgbClr val=\"FFFFFF\"/></a:solidFill><a:ln w=\"12700\"><a:solidFill><a:srgbClr val=\"0000FF\"/></a:solidFill></a:ln></c:spPr></c:upBars>"
                                  "<c:downBars><c:spPr><a:solidFill><a:srgbClr val=\"00AA00\"/></a:solidFill><a:ln><a:noFill/></a:ln></c:spPr></c:downBars></c:upDownBars>");
        return out + axes + QStringLiteral("</c:stockChart>");
    }
    // The axes: categories and a value axis 0 to `max` (id pair 111/222, or 333/444 for the second pair, which is on the right and has no categories shown).
    static QString stockAxes(double max, bool second = false)
    {
        const QString cat = second ? QStringLiteral("333") : QStringLiteral("111"), val = second ? QStringLiteral("444") : QStringLiteral("222");
        return QStringLiteral("<c:catAx><c:axId val=\"%1\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"%3\"/><c:axPos val=\"b\"/><c:numFmt formatCode=\"General\" sourceLinked=\"1\"/>"
                              "<c:majorTickMark val=\"none\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"%2\"/><c:crosses val=\"autoZero\"/><c:auto val=\"1\"/><c:lblAlgn val=\"ctr\"/><c:lblOffset val=\"100\"/></c:catAx>")
                   .arg(cat, val, second ? QStringLiteral("1") : QStringLiteral("0"))
               + QStringLiteral("<c:valAx><c:axId val=\"%1\"/><c:scaling><c:orientation val=\"minMax\"/><c:max val=\"%3\"/><c:min val=\"0\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"%4\"/><c:numFmt formatCode=\"General\" sourceLinked=\"1\"/>"
                                "<c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"%2\"/><c:crosses val=\"%5\"/><c:crossBetween val=\"between\"/></c:valAx>")
                     .arg(val, cat).arg(max).arg(second ? QStringLiteral("r") : QStringLiteral("l"), second ? QStringLiteral("max") : QStringLiteral("autoZero"));
    }
    static QString volumePlot(const QVector<double> &volume, const QStringList &cats = {"a", "b", "c", "d"})
    {
        return QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/><c:varyColors val=\"0\"/>")
               + chartSeries(0, QStringLiteral("Volume"), cats, volume, QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill></c:spPr>"), QString(), QString())
               + QStringLiteral("<c:gapWidth val=\"150\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>");
    }
    // The vertical runs of a color in a picture: for each group of neighboring columns that hold the color, its columns and the top and bottom of the color there, from the left.
    struct Run {
        int left, right, top, bottom;
        double middle() const { return (top + bottom) / 2.0; }
    };
    static QVector<Run> colorColumns(const QImage &img, const QColor &color, int xFrom = 0, int xTo = -1)
    {
        QVector<Run> out;
        const int last = xTo < 0 ? img.width() - 1 : std::min(xTo, img.width() - 1);
        for (int x = std::max(0, xFrom); x <= last; ++x) {
            int top = -1, bottom = -1;
            for (int y = 0; y < img.height(); ++y)
                if (isColor(img.pixel(x, y), color, 40)) {
                    if (top < 0) top = y;
                    bottom = y;
                }
            if (top < 0) continue;
            if (!out.isEmpty() && out.last().right >= x - 2) {
                Run &r = out.last();
                r.right = x;
                r.top = std::min(r.top, top);
                r.bottom = std::max(r.bottom, bottom);
            } else {
                out << Run{x, x, top, bottom};
            }
        }
        return out;
    }
    // The picture row of a value on a scale known from one run whose top and bottom are a high and a low value.
    struct RowScale {
        double y0 = 0, perUnit = 0;   // the row of value 0, and rows for one unit up
        double rowOf(double v) const { return y0 - perUnit * v; }
    };
    static RowScale scaleFrom(const Run &a, double aHigh, double aLow)
    {
        RowScale s;
        s.perUnit = (a.bottom - a.top) / (aHigh - aLow);
        s.y0 = a.bottom + s.perUnit * aLow;
        return s;
    }

private Q_SLOTS:
    // The four forms are read as stock charts: one plot of hi-low lines (and up-down bars when it has an open), the volume's columns before it against the first axes and the prices against a second pair.
    void chartReadsTheFourStockForms()
    {
        const QVector<double> h{9, 8, 7, 6}, l{1, 4, 2, 3}, cl{5, 7, 3, 5}, op{3, 7, 2, 5};
        struct Case {
            QString body;
            ChartStockForm form;
            int plots;
            bool bars;
        };
        const QString second = QStringLiteral("<c:axId val=\"333\"/><c:axId val=\"444\"/>");
        const QVector<Case> cases = {
            {stockPlot({h, l, cl}, false) + stockAxes(10), ChartStockForm::HighLowClose, 1, false},
            {stockPlot({op, h, l, cl}, true) + stockAxes(10), ChartStockForm::OpenHighLowClose, 1, true},
            {volumePlot({100, 50, 150, 100}) + stockPlot({h, l, cl}, false, second, 1) + stockAxes(200) + stockAxes(10, true), ChartStockForm::VolumeHighLowClose, 2, false},
            {volumePlot({100, 50, 150, 100}) + stockPlot({op, h, l, cl}, true, second, 1) + stockAxes(200) + stockAxes(10, true), ChartStockForm::VolumeOpenHighLowClose, 2, true},
        };
        for (const Case &k : cases) {
            Chart c;
            QString err;
            QVERIFY2(readChart(chartPart(k.body), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
            QCOMPARE(int(chartStockForm(c)), int(k.form));
            QCOMPARE(int(c.plots.size()), k.plots);
            const ChartPlot &stock = c.plots.last();
            QVERIFY(stock.stock && stock.type == ChartType::Line && stock.hiLowLines);
            QCOMPARE(stock.upDownBars, k.bars);
            QCOMPARE(int(stock.series.last().marker.symbol), int(k.bars ? ChartMarker::None : ChartMarker::Dash));
            QVERIFY(stock.series.first().outline.none);
            if (k.plots == 2) QCOMPARE(int(c.plots[0].type), int(ChartType::Bar));
        }
        // series that fit none of the forms (two prices, five) are what a kind not drawn yet is: columns, and the part is kept
        for (const QVector<QVector<double>> &prices : {QVector<QVector<double>>{h, l}, QVector<QVector<double>>{op, h, l, cl, h}}) {
            const QByteArray part = chartPart(stockPlot(prices, false) + stockAxes(10));
            Chart c;
            QString err;
            QVERIFY2(readChart(part, defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
            QCOMPARE(int(chartStockForm(c)), int(ChartStockForm::None));
            QCOMPARE(int(c.plots[0].type), int(ChartType::Other));
            QVERIFY(!c.plots[0].stock);
            QVERIFY(!chartIsFullyModeled(part));
        }
        // a stock plot beside a line plot, or a volume on the same axes as the prices, is not a form either
        const QString line = QStringLiteral("<c:lineChart><c:grouping val=\"standard\"/>") + chartSeries(9, QStringLiteral("L"), {"a", "b", "c", "d"}, {1, 2, 3, 4}) + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:lineChart>");
        const QStringList odd = {line + stockPlot({h, l, cl}, false) + stockAxes(10), volumePlot({1, 2, 3, 4}) + stockPlot({h, l, cl}, false, QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/>"), 1) + stockAxes(10)};
        for (const QString &body : odd) {
            Chart c;
            QString err;
            QVERIFY2(readChart(chartPart(body), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
            QCOMPARE(int(chartStockForm(c)), int(ChartStockForm::None));
            for (const ChartPlot &p : c.plots) QVERIFY(!p.stock);
            QVERIFY(!chartIsFullyModeled(chartPart(body)));
        }
    }

    // High-low-close: a red line for each category from its high to its low, the close marked in blue at the close's place, and no line through the series; every place read from the scale.
    void chartSvgDrawsHighLowCloseWhereTheNumbersAre()
    {
        const QVector<double> h{9, 8, 7, 6}, l{1, 4, 2, 3}, cl{5, 7, 3, 5};
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(stockPlot({h, l, cl}, false) + stockAxes(10)), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QImage img = renderChart(c, QSizeF(400, 300), 2);
        const QVector<Run> red = colorColumns(img, Qt::red);
        QCOMPARE(int(red.size()), 4);
        const RowScale s = scaleFrom(red[0], h[0], l[0]);
        QVERIFY(s.perUnit > 10);
        for (int i = 0; i < 4; ++i) {
            QVERIFY2(std::abs(red[i].top - s.rowOf(h[i])) <= 3 && std::abs(red[i].bottom - s.rowOf(l[i])) <= 3, qPrintable(QStringLiteral("line %1: %2 to %3, wanted %4 to %5").arg(i).arg(red[i].top).arg(red[i].bottom).arg(s.rowOf(h[i])).arg(s.rowOf(l[i]))));
            // the close's mark sits across its line at the close's row
            const QVector<Run> blue = colorColumns(img, QColor(0, 0, 0xFF), red[i].left - 12, red[i].right + 12);
            QCOMPARE(int(blue.size()), 1);
            QVERIFY2(std::abs(blue[0].middle() - s.rowOf(cl[i])) <= 3, qPrintable(QStringLiteral("close %1: %2, wanted %3").arg(i).arg(blue[0].middle()).arg(s.rowOf(cl[i]))));
            QVERIFY(blue[0].right - blue[0].left > red[i].right - red[i].left + 6);
        }
        // nothing joins the series (their lines are none): the colors a series would be drawn in are nowhere
        for (const QColor &series : {defaultTheme().colors[Theme::Accent1], defaultTheme().colors[Theme::Accent2], defaultTheme().colors[Theme::Accent3]}) QCOMPARE(colorArea(img, series), 0.0);
    }

    // Open-high-low-close: the bar between the open and the close, hollow with the file's edge for a rise and filled for a fall, over the high-low line, at the rows the numbers give.
    void chartSvgDrawsUpAndDownBarsBetweenOpenAndClose()
    {
        const QVector<double> op{3, 7, 2, 5}, h{9, 8, 7, 6}, l{1, 4, 1, 2}, cl{6, 4, 5, 3};
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(stockPlot({op, h, l, cl}, true) + stockAxes(10)), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QImage img = renderChart(c, QSizeF(400, 300), 2);
        const QVector<Run> red = colorColumns(img, Qt::red);
        QCOMPARE(int(red.size()), 4);
        const RowScale s = scaleFrom(red[0], h[0], l[0]);
        // the falls (categories 2 and 4) are filled green, from the open down to the close
        const QVector<Run> green = colorColumns(img, QColor(0, 0xAA, 0));
        QCOMPARE(int(green.size()), 2);
        const int falls[] = {1, 3};
        for (int k = 0; k < 2; ++k) {
            QVERIFY2(std::abs(green[k].top - s.rowOf(op[falls[k]])) <= 3 && std::abs(green[k].bottom - s.rowOf(cl[falls[k]])) <= 3, qPrintable(QStringLiteral("fall %1: %2 to %3").arg(k).arg(green[k].top).arg(green[k].bottom)));
            QVERIFY(green[k].left > red[falls[k]].left - 40 && green[k].right < red[falls[k]].right + 40);   // (on its category's line)
        }
        // the rises (1 and 3) are hollow with a blue edge from the close up to the open: the edge spans them, the inside is white
        const QVector<Run> blue = colorColumns(img, QColor(0, 0, 0xFF));
        QCOMPARE(int(blue.size()), 2);
        const int rises[] = {0, 2};
        for (int k = 0; k < 2; ++k) {
            QVERIFY2(std::abs(blue[k].top - s.rowOf(cl[rises[k]])) <= 3 && std::abs(blue[k].bottom - s.rowOf(op[rises[k]])) <= 3, qPrintable(QStringLiteral("rise %1: %2 to %3").arg(k).arg(blue[k].top).arg(blue[k].bottom)));
            const int inside = int(std::lround((blue[k].left + blue[k].right) / 2.0 + (blue[k].right - blue[k].left) / 4.0));
            QVERIFY(isColor(img.pixel(inside, int(std::lround(blue[k].middle()))), Qt::white, 10));
        }
    }

    // The volume forms: the volume as columns read on the first (left) axis, the prices on the second axis on the right, each on its own scale.
    void chartSvgDrawsTheVolumeOnTheFirstAxisAndThePricesOnTheSecond()
    {
        const QVector<double> h{9, 8, 7, 6}, l{1, 4, 2, 3}, cl{5, 7, 3, 5}, volume{100, 50, 150, 200};
        Chart c;
        QString err;
        const QString body = volumePlot(volume) + stockPlot({h, l, cl}, false, QStringLiteral("<c:axId val=\"333\"/><c:axId val=\"444\"/>"), 1) + stockAxes(200) + stockAxes(10, true);
        QVERIFY2(readChart(chartPart(body), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QImage img = renderChart(c, QSizeF(400, 300), 2);
        const QVector<Run> red = colorColumns(img, Qt::red);
        QCOMPARE(int(red.size()), 4);
        const RowScale prices = scaleFrom(red[0], h[0], l[0]);   // (rows of the right axis, 0 to 10)
        const QVector<Run> columns = colorColumns(img, QColor(0, 0xFF, 0));
        QCOMPARE(int(columns.size()), 4);
        for (int i = 0; i < 4; ++i) {
            // a volume of 100 on the axis to 200 is half of the plot's height, which is 5 on the price axis to 10: the column's top is where a price of volume / 20 would be
            QVERIFY2(std::abs(columns[i].top - prices.rowOf(volume[i] / 20)) <= 3, qPrintable(QStringLiteral("column %1 top %2, wanted %3").arg(i).arg(columns[i].top).arg(prices.rowOf(volume[i] / 20))));
            QVERIFY2(std::abs(columns[i].bottom - prices.rowOf(0)) <= 3, qPrintable(QString::number(columns[i].bottom)));
            QVERIFY(columns[i].right < red[i].left + 60 && columns[i].left > red[i].left - 60);   // (the prices stand on their category)
        }
        // the first axis' numbers are the volume's, on the left, and the second axis' the prices', on the right
        const QString svg = QString::fromUtf8(chartSvg(c, QSizeF(400, 300)));
        // (a text is placed by the group's transform and its own x)
        auto placeOf = [&](const char *word) {
            const QRegularExpression at(QStringLiteral("matrix\\(1,0,0,1,([0-9.]+),[0-9.]+\\)\"(?:(?!</g>).)*?x=\"(-?[0-9.]+)\"(?:(?!</g>).)*?>%1</text>").arg(QLatin1String(word)), QRegularExpression::DotMatchesEverythingOption);
            const QRegularExpressionMatch m = at.match(svg);
            return m.hasMatch() ? m.captured(1).toDouble() + m.captured(2).toDouble() : -1000.0;
        };
        const double left = placeOf("200"), right = placeOf("10");
        QVERIFY2(left > -100 && left < 100 && right > 300, qPrintable(QStringLiteral("%1, %2").arg(left).arg(right)));
    }

    // An empty cell is shown as the chart says: left out (the line spans what is there), or as zero (the line runs down to it).
    void aStockChartDrawsAnEmptyCellAsTheChartSays()
    {
        const double none = std::nan("");
        const QVector<double> h{8, 9, 7, 6}, l{2, none, 3, 3}, cl{5, 6, 4, 5};
        for (const QString &mode : {QStringLiteral("gap"), QStringLiteral("span"), QStringLiteral("zero")}) {
            QByteArray part = chartPart(stockPlot({h, l, cl}, false) + stockAxes(10));
            part.replace("<c:dispBlanksAs val=\"gap\"/>", QByteArray("<c:dispBlanksAs val=\"") + mode.toLatin1() + "\"/>");
            Chart c;
            QString err;
            QVERIFY2(readChart(part, defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
            QCOMPARE(c.blanksAs, mode);
            const QImage img = renderChart(c, QSizeF(400, 300), 2);
            const QVector<Run> red = colorColumns(img, Qt::red);
            QCOMPARE(int(red.size()), 4);
            const RowScale s = scaleFrom(red[0], h[0], l[0]);
            // category 2: its low is empty; left out, the line runs from the high to the close (6); as zero, down to the axis
            const double wantedLow = mode == QLatin1String("zero") ? 0 : 6;
            QVERIFY2(std::abs(red[1].top - s.rowOf(9)) <= 3 && std::abs(red[1].bottom - s.rowOf(wantedLow)) <= 3, qPrintable(mode + QStringLiteral(": %1 to %2, wanted %3").arg(red[1].top).arg(red[1].bottom).arg(s.rowOf(wantedLow))));
            // the others are as ever
            QVERIFY(std::abs(red[2].bottom - s.rowOf(3)) <= 3);
        }
    }

    // A chart with more categories than a picture can show is drawn from every nth, in a bounded number of lines and bars.
    void aStockChartOfManyCategoriesIsDrawnCoarserWithinItsLimit()
    {
        QStringList cats;
        QVector<double> op, h, l, cl;
        for (int i = 0; i < 10000; ++i) {
            cats << QString::number(i);
            op << 3 + i % 3;
            h << 8;
            l << 1;
            cl << 4 + i % 2;
        }
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(stockPlot({op, h, l, cl}, true, QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/>"), 0, cats) + stockAxes(10)), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(chartStockForm(c)), int(ChartStockForm::OpenHighLowClose));
        QCOMPARE(c.plots[0].series[0].values.size(), qsizetype(10000));
        bool coarser = false;
        const QString svg = QString::fromUtf8(chartSvg(c, QSizeF(400, 300), &coarser));
        QVERIFY(coarser);
        // counted: the high-low lines are red, one for each category drawn (10,000 were read; a stride of 27 leaves 371)
        int lines = 0;
        for (const QRegularExpressionMatch &m : QRegularExpression(QStringLiteral("<g [^>]*stroke=\"#ff0000\"[^>]*>(.*?)</g>"), QRegularExpression::DotMatchesEverythingOption).globalMatch(svg)) lines += int(m.captured(1).count(QStringLiteral("<polyline")));
        QVERIFY2(lines > 100 && lines <= 400 && svg.count(QLatin1Char('<')) < 6000, qPrintable(QStringLiteral("%1 lines, %2 tags").arg(lines).arg(svg.count(QLatin1Char('<')))));
    }

    // Thinning a long stock chart keeps every extreme: each drawn point is the first open, the highest high, the lowest low, and the last close of its bucket,
    // and the axis covers them. (10,000 days: one day with a high of 200, another with a low of -100.)
    static QPair<double, double> axisNumbersRange(const QString &svg)
    {
        double lo = 1e300, hi = -1e300;
        for (const QRegularExpressionMatch &m : QRegularExpression(QStringLiteral(">(-?[0-9]+(?:\\.[0-9]+)?)</text>")).globalMatch(svg)) {
            const double v = m.captured(1).toDouble();
            lo = std::min(lo, v);
            hi = std::max(hi, v);
        }
        return {lo, hi};
    }
    void aThinnedStockChartKeepsEveryHighAndLow()
    {
        QStringList cats;
        QVector<double> op, h, l, cl;
        for (int i = 0; i < 10000; ++i) {
            cats << QStringLiteral("d%1").arg(i);
            op << 30 + i % 3;
            h << (i == 3001 ? 200 : 40);
            l << (i == 7003 ? -100 : 20);
            cl << 31 + i % 2;
        }
        for (const bool bars : {false, true}) {
            Chart c;
            QString err;
            const QVector<QVector<double>> prices = bars ? QVector<QVector<double>>{op, h, l, cl} : QVector<QVector<double>>{h, l, cl};
            QString axes = stockAxes(10);
            axes.replace(QStringLiteral("<c:max val=\"10\"/><c:min val=\"0\"/>"), QString());   // (an automatic axis)
            QVERIFY2(readChart(chartPart(stockPlot(prices, bars, QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/>"), 0, cats) + axes), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
            bool coarser = false;
            const QString svg = QString::fromUtf8(chartSvg(c, QSizeF(400, 300), &coarser));
            QVERIFY(coarser);
            const QPair<double, double> range = axisNumbersRange(svg);
            QVERIFY2(range.second >= 200 && range.first <= -100, qPrintable(QStringLiteral("axis %1 to %2").arg(range.first).arg(range.second)));
        }
    }

    // The same for a line plot with high-low lines, and the volume's columns of a thinned stock chart add up (10,000 days of 100 reach a few thousand, not 100).
    void aThinnedLinePlotWithHighLowLinesKeepsEveryExtreme()
    {
        QStringList cats;
        QVector<double> a, b;
        for (int i = 0; i < 10000; ++i) {
            cats << QStringLiteral("d%1").arg(i);
            a << (i == 3001 ? 200 : 40);
            b << (i == 7003 ? -100 : 20);
        }
        QString axes = stockAxes(10);
        axes.replace(QStringLiteral("<c:max val=\"10\"/><c:min val=\"0\"/>"), QString());
        QString plot = stockPlot({a, b}, false, QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/>"), 0, cats);
        plot.replace(QStringLiteral("stockChart"), QStringLiteral("lineChart"));
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot + axes), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QVERIFY(c.plots[0].hiLowLines && !c.plots[0].stock);
        bool coarser = false;
        const QPair<double, double> range = axisNumbersRange(QString::fromUtf8(chartSvg(c, QSizeF(400, 300), &coarser)));
        QVERIFY(coarser);
        QVERIFY2(range.second >= 200 && range.first <= -100, qPrintable(QStringLiteral("axis %1 to %2").arg(range.first).arg(range.second)));
    }
    void aThinnedStockChartAddsUpItsVolume()
    {
        QStringList cats;
        QVector<double> volume, h, l, cl;
        for (int i = 0; i < 10000; ++i) {
            cats << QStringLiteral("d%1").arg(i);
            volume << 100;
            h << 8;
            l << 1;
            cl << 4;
        }
        QString axes = stockAxes(200);
        axes.replace(QStringLiteral("<c:max val=\"200\"/><c:min val=\"0\"/>"), QString());
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(volumePlot(volume, cats) + stockPlot({h, l, cl}, false, QStringLiteral("<c:axId val=\"333\"/><c:axId val=\"444\"/>"), 1, cats) + axes + stockAxes(10, true)), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(chartStockForm(c)), int(ChartStockForm::VolumeHighLowClose));
        bool coarser = false;
        const QPair<double, double> range = axisNumbersRange(QString::fromUtf8(chartSvg(c, QSizeF(400, 300), &coarser)));
        QVERIFY(coarser);
        QVERIFY2(range.second >= 1000, qPrintable(QStringLiteral("axis %1 to %2").arg(range.first).arg(range.second)));
    }

    // A stock plot whose series a filter leaves fitting no form is drawn as columns and never as stock.
    void aStockChartWithASeriesFilteredOutIsDrawnAsColumns()
    {
        const QVector<double> h{9, 8, 7, 6}, l{1, 4, 2, 3}, cl{5, 7, 3, 5};
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(stockPlot({h, l, cl}, false) + stockAxes(10)), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QImage whole = renderChart(c, QSizeF(400, 300), 2);
        QVERIFY(!colorColumns(whole, Qt::red).isEmpty());
        c.plots[0].series[1].filtered = true;
        const QImage cut = renderChart(c, QSizeF(400, 300), 2);
        QVERIFY(colorColumns(cut, Qt::red).isEmpty());   // (no high-low lines: not a stock chart now)
    }

    // A stock part as a program writes it (the series' lines none, the close's mark, the hi-low and up-down looks) is the model's to write: kept byte for byte when unedited, written the same when it is.
    void aStockPartIsFullyModeledAndWrittenBackTheSame()
    {
        const QVector<double> h{9, 8, 7, 6}, l{1, 4, 2, 3}, cl{5, 7, 3, 5}, op{3, 7, 2, 5}, volume{100, 50, 150, 200};
        const QStringList bodies = {stockPlot({h, l, cl}, false) + stockAxes(10), stockPlot({op, h, l, cl}, true) + stockAxes(10),
                                    volumePlot(volume) + stockPlot({h, l, cl}, false, QStringLiteral("<c:axId val=\"333\"/><c:axId val=\"444\"/>"), 1) + stockAxes(200) + stockAxes(10, true)};
        for (const QString &body : bodies) {
            const QByteArray part = chartPart(body);
            QString why;
            QVERIFY2(chartIsFullyModeled(part, &why), qPrintable(why));
            Chart c;
            QString err;
            QVERIFY2(readChart(part, defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
            const QByteArray written = writeChartPart(c);
            QVERIFY(written.contains("<c:stockChart>") && !written.contains("<c:lineChart>"));
            // (schema order: the series, the lines, the bars, then the axes; no grouping, varied colors, or marker flag of a line plot)
            const qsizetype start = written.indexOf("<c:stockChart>"), end = written.indexOf("</c:stockChart>");
            const QByteArray inside = written.mid(start, end - start);
            QVERIFY(!inside.contains("<c:grouping") && !inside.contains("<c:varyColors") && !inside.contains("<c:marker val"));
            QVERIFY(inside.indexOf("<c:hiLowLines>") > inside.lastIndexOf("</c:ser>") && inside.indexOf("<c:axId") > inside.indexOf("<c:hiLowLines>"));
            Chart again;
            QVERIFY2(readChart(written, defaultTheme(), ColorMap(), &again, &err), qPrintable(err));
            QCOMPARE(int(chartStockForm(again)), int(chartStockForm(c)));
            QCOMPARE(chartSvg(again, QSizeF(400, 300)), chartSvg(c, QSizeF(400, 300)));
            QCOMPARE(writeChartPart(again), written);
        }
        // a hidden series (the extension's list of filtered series inside the stock plot) is not something this writes: the part is kept
        QString hidden = bodies[0];
        QString ser = chartSeries(7, QStringLiteral("Hidden"), {"a", "b", "c", "d"}, {1, 2, 3, 4});
        ser.replace(QStringLiteral("c:ser"), QStringLiteral("c15:ser"));
        hidden.replace(QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:stockChart>"),
                       QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/><c:extLst><c:ext uri=\"{02D57815-91ED-43cb-92C2-25804820EDAC}\" xmlns:c15=\"http://schemas.microsoft.com/office/drawing/2012/chart\"><c15:filteredLineSeries>")
                           + ser + QStringLiteral("</c15:filteredLineSeries></c:ext></c:extLst></c:stockChart>"));
        const QByteArray part = chartPart(hidden);
        QString why;
        QVERIFY(!chartIsFullyModeled(part, &why));
        QVERIFY2(why.contains(QStringLiteral("hidden series")), qPrintable(why));
    }

    // A stock chart with a date axis (c:dateAx) is read, drawn with its categories along the bottom, and written back with the date axis.
    void aStockChartOnADateAxisIsReadAndWrittenBack()
    {
        const QVector<double> h{9, 8, 7, 6}, l{1, 4, 2, 3}, cl{5, 7, 3, 5};
        QString axes = stockAxes(10);
        axes.replace(QStringLiteral("<c:catAx>"), QStringLiteral("<c:dateAx>")).replace(QStringLiteral("</c:catAx>"), QStringLiteral("</c:dateAx>")).remove(QStringLiteral("<c:lblAlgn val=\"ctr\"/>"));
        const QByteArray part = chartPart(stockPlot({h, l, cl}, false, QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/>"), 0, {"45000", "45001", "45002", "45005"}) + axes);
        Chart c;
        QString err;
        QVERIFY2(readChart(part, defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(chartStockForm(c)), int(ChartStockForm::HighLowClose));
        QCOMPARE(int(c.axes[0].kind), int(ChartAxis::Date));
        QVERIFY(QString::fromUtf8(chartSvg(c, QSizeF(400, 300))).contains(QStringLiteral(">45005<")));
        QString why;
        QVERIFY2(chartIsFullyModeled(part, &why), qPrintable(why));
        QVERIFY(writeChartPart(c).contains("<c:dateAx>"));
    }

    void chartSvgScalesTheAxisToTheValues()
    {
        // The largest value, 16.1, gets an axis to 18 in steps of 2, from 0.
        const QString plot = QStringLiteral("<c:areaChart><c:grouping val=\"standard\"/>") + chartSeries(0, QStringLiteral("S"), {"a", "b", "c"}, {0.2, 16.112566, 5.72})
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:areaChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QString svg = QString::fromUtf8(chartSvg(c, QSizeF(400, 300)));
        for (const char *label : {"0", "2", "4", "8", "16", "18"}) QVERIFY2(svg.contains(QStringLiteral(">%1<").arg(QLatin1String(label))), label);
        QVERIFY(!svg.contains(QStringLiteral(">20<")));
        QVERIFY(!svg.contains(QStringLiteral(">1<")));
        // Labels in the axis' number format.
        QString thousands = plot;
        thousands.replace(QStringLiteral("<c:numFmt formatCode=\"General\" sourceLinked=\"1\"/><c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"111\"/>"),
                        QStringLiteral("<c:numFmt formatCode=\"0.0&quot;k&quot;\" sourceLinked=\"0\"/><c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"111\"/>"));
        QVERIFY2(readChart(chartPart(thousands), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QString formatted = QString::fromUtf8(chartSvg(c, QSizeF(400, 300)));
        QVERIFY2(formatted.contains(QStringLiteral(">18.0k<")), qPrintable(formatted.left(200)));
    }

    void chartSvgPlacesTheLegend()
    {
        const QString red = QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></c:spPr>");
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("Only series"), {"a", "b"}, {1, 0}, red)
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        for (const QString &pos : {QStringLiteral("b"), QStringLiteral("r"), QStringLiteral("t"), QStringLiteral("l")}) {
            Chart c;
            QString err;
            QVERIFY2(readChart(chartPart(plot, QString(), QStringLiteral("<c:legend><c:legendPos val=\"%1\"/></c:legend>").arg(pos)), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
            QCOMPARE(c.legend.position, pos);
            const QImage img = renderChart(c, QSizeF(400, 300), 2);
            // The key is a small red square beside the series' name, at that side; the bar of value 0 is not drawn.
            const QVector<QPointF> bits = colorComponents(img, Qt::red, 4);
            QVERIFY2(!bits.isEmpty(), qPrintable(pos));
            QPointF key = bits.last();
            for (const QPointF &p : bits) {
                if (pos == QLatin1String("b") && p.y() > key.y()) key = p;
                if (pos == QLatin1String("t") && p.y() < key.y()) key = p;
                if (pos == QLatin1String("r") && p.x() > key.x()) key = p;
                if (pos == QLatin1String("l") && p.x() < key.x()) key = p;
            }
            if (pos == QLatin1String("b")) QVERIFY2(key.y() > img.height() * 0.85, qPrintable(QString::number(key.y())));
            if (pos == QLatin1String("t")) QVERIFY2(key.y() < img.height() * 0.15, qPrintable(QString::number(key.y())));
            // At a side, the key is one name's width in from the edge, and names are wider with some systems' fonts
            // (Windows: 68% across); a legend at the top or bottom has it left of the middle.
            if (pos == QLatin1String("r")) QVERIFY2(key.x() > img.width() * 0.6, qPrintable(QString::number(key.x())));
            if (pos == QLatin1String("l")) QVERIFY2(key.x() < img.width() * 0.4, qPrintable(QString::number(key.x())));
            QVERIFY(QString::fromUtf8(chartSvg(c, QSizeF(400, 300))).contains(QStringLiteral(">Only series<")));
        }
    }

    // ---- the places a file gives to the legend and the titles

    // A bar chart of one series of the color, `values` over their names, whose columns touch (so that a column covers what is behind it), with an axis title if asked.
    static QString oneSeriesPlot(const QString &color, const QVector<double> &values, const QString &axisTitle = QString(), const QString &axisTitleXml = QString(), const QString &catTitleXml = QString())
    {
        QStringList names;
        for (int i = 0; i < values.size(); ++i) names << QString(QChar('a' + i));
        const QString fill = QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"%1\"/></a:solidFill></c:spPr>").arg(color);
        QString axes = chartAxes();
        // (the value axis stops at the largest value, so that the tallest column reaches the top of the plot)
        axes.replace(QStringLiteral("<c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling>"), QStringLiteral("<c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/><c:max val=\"1\"/><c:min val=\"0\"/></c:scaling>"));
        if (!axisTitle.isEmpty()) axes.replace(QStringLiteral("<c:majorGridlines/>"), QStringLiteral("<c:majorGridlines/>") + axisTitleXml.arg(axisTitle));
        if (!catTitleXml.isEmpty()) axes.replace(QStringLiteral("<c:axPos val=\"b\"/>"), QStringLiteral("<c:axPos val=\"b\"/>") + catTitleXml);
        return QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("Only series"), names, values, fill) + QStringLiteral("<c:gapWidth val=\"0\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + axes;
    }
    static QString manualLayout(double x, double y, double w, double h, const QString &target = QString())
    {
        return QStringLiteral("<c:layout><c:manualLayout>%1<c:xMode val=\"edge\"/><c:yMode val=\"edge\"/><c:x val=\"%2\"/><c:y val=\"%3\"/>%4</c:manualLayout></c:layout>")
            .arg(target.isEmpty() ? QString() : QStringLiteral("<c:layoutTarget val=\"%1\"/>").arg(target)).arg(x).arg(y)
            .arg(w > 0 ? QStringLiteral("<c:w val=\"%1\"/><c:h val=\"%2\"/>").arg(w).arg(h) : QString());
    }
    static QByteArray withPlotLayout(const QByteArray &part, const QString &layout)
    {
        QByteArray out = part;
        out.replace("<c:plotArea><c:layout/>", ("<c:plotArea>" + layout).toUtf8());
        return out;
    }
    static Chart readOrFail(const QByteArray &part)
    {
        Chart c;
        QString err;
        if (!readChart(part, defaultTheme(), ColorMap(), &c, &err)) qFatal("%s", qPrintable(err));
        return c;
    }

    // The legend's own place (c:legend's c:layout) is read: where its corner is, and how large it is if the file says; a file that gives no place, or none that can be used, has none.
    void chartReadsTheLegendsOwnPlace()
    {
        const QString plot = oneSeriesPlot(QStringLiteral("FF0000"), {1, 0});
        auto legend = [](const QString &layout) { return QStringLiteral("<c:legend><c:legendPos val=\"r\"/>") + layout + QStringLiteral("<c:overlay val=\"0\"/></c:legend>"); };
        Chart c = readOrFail(chartPart(plot, QString(), legend(manualLayout(0.15, 0.2, 0.3, 0.1))));
        QVERIFY(c.legend.manual);
        QVERIFY(c.legend.edge);
        QCOMPARE(c.legend.x, 0.15);
        QCOMPARE(c.legend.y, 0.2);
        QCOMPARE(c.legend.w, 0.3);
        QCOMPARE(c.legend.h, 0.1);
        // only a corner: its size is the entries'
        c = readOrFail(chartPart(plot, QString(), legend(manualLayout(0.4, 0.5, 0, 0))));
        QVERIFY(c.legend.manual);
        QCOMPARE(c.legend.w, 0.0);
        // a place that is how far to move it from where its position puts it
        QString factor = manualLayout(0.1, -0.05, 0.2, 0.2);
        factor.replace(QStringLiteral("<c:xMode val=\"edge\"/><c:yMode val=\"edge\"/>"), QStringLiteral("<c:xMode val=\"factor\"/><c:yMode val=\"factor\"/>"));
        c = readOrFail(chartPart(plot, QString(), legend(factor)));
        QVERIFY(c.legend.manual);
        QVERIFY(!c.legend.edge);
        QCOMPARE(c.legend.y, -0.05);
        // none, and what is out of range
        QVERIFY(!readOrFail(chartPart(plot, QString(), legend(QString()))).legend.manual);
        c = readOrFail(chartPart(plot, QString(), legend(manualLayout(7, -3, 9, 0.2))));
        QVERIFY(c.legend.manual);
        QVERIFY(c.legend.x >= 0 && c.legend.x <= 1 && c.legend.y >= 0 && c.legend.y <= 1 && c.legend.w <= 1);
        // the same for the legend's own fill and line: read when the file gives them
        QString filled = legend(QString());
        filled.replace(QStringLiteral("<c:overlay val=\"0\"/>"), QStringLiteral("<c:overlay val=\"0\"/><c:spPr><a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill><a:ln w=\"12700\"><a:solidFill><a:srgbClr val=\"0000FF\"/></a:solidFill></a:ln></c:spPr>"));
        c = readOrFail(chartPart(plot, QString(), filled));
        QCOMPARE(c.legend.fill, QColor(0, 255, 0));
        QVERIFY(c.legend.border.set);
        QCOMPARE(c.legend.border.color, QColor(0, 0, 255));
    }

    // The legend is where the file puts it: its key (a small square in the series' color) is inside the place, and not at the side its position says.
    void aLegendWithItsOwnPlaceIsDrawnThere()
    {
        const QString plot = oneSeriesPlot(QStringLiteral("FF0000"), {1, 0});
        const Chart c = readOrFail(chartPart(plot, QString(), QStringLiteral("<c:legend><c:legendPos val=\"r\"/>") + manualLayout(0.55, 0.05, 0.4, 0.15) + QStringLiteral("<c:overlay val=\"0\"/></c:legend>")));
        const QSizeF size(400, 300);
        const QImage img = renderChart(c, size, 2);
        const QVector<QPointF> keys = colorComponents(img, Qt::red, 4);
        // (the first column is the red at the left; the key is the red patch at the right)
        QVERIFY(keys.size() >= 2);
        const QPointF key = keys.last();
        const QRectF place(0.55 * size.width() * 2, 0.05 * size.height() * 2, 0.4 * size.width() * 2, 0.15 * size.height() * 2);
        QVERIFY2(place.contains(key), qPrintable(QStringLiteral("%1,%2").arg(key.x()).arg(key.y())));
        // a legend of its own size and place (the file gives the width and height) draws its name inside the place
        QRect text;
        for (int y = int(place.top()); y < int(place.bottom()); ++y)
            for (int x = int(place.left()); x < int(place.right()); ++x)
                if (qRed(img.pixel(x, y)) < 90 && qGreen(img.pixel(x, y)) < 90 && qBlue(img.pixel(x, y)) < 90) text = text.isNull() ? QRect(x, y, 1, 1) : text.united(QRect(x, y, 1, 1));
        QVERIFY(!text.isNull());
        QVERIFY(text.left() > key.x());
    }

    // A legend over the plot (its overlay on) is drawn after it, so a column does not cover its name.
    void aLegendOverThePlotIsNotCovered()
    {
        const QString plot = oneSeriesPlot(QStringLiteral("FF0000"), {0, 1});
        const Chart c = readOrFail(chartPart(plot, QString(), QStringLiteral("<c:legend><c:legendPos val=\"tr\"/><c:overlay val=\"1\"/></c:legend>")));
        const QSizeF size(400, 300);
        const QImage img = renderChart(c, size, 2);
        // the column at the right is red from the top of the plot down, under the legend; the legend's name is dark on it
        int dark = 0;
        for (int y = 0; y < int(0.12 * img.height()); ++y)
            for (int x = int(0.55 * img.width()); x < img.width(); ++x) {
                const QRgb p = img.pixel(x, y);
                if (qRed(p) < 80 && qGreen(p) < 80 && qBlue(p) < 80) ++dark;
            }
        QVERIFY2(dark > 30, qPrintable(QString::number(dark)));
        // and what is laid over the column is the legend's own white, when the file gives the legend a fill
        QString filled = QStringLiteral("<c:legend><c:legendPos val=\"tr\"/><c:overlay val=\"1\"/><c:spPr><a:solidFill><a:srgbClr val=\"FFFFFF\"/></a:solidFill></c:spPr></c:legend>");
        const QImage white = renderChart(readOrFail(chartPart(plot, QString(), filled)), size, 2);
        // (inside the legend's corner: the column's red where the legend has no fill, the legend's white where it has)
        QCOMPARE(white.pixel(img.width() - 40, 48), QColor(Qt::white).rgb());
        QCOMPARE(img.pixel(img.width() - 40, 48), QColor(Qt::red).rgb());
    }

    // A plot the file fixes (as a rectangle of the chart) does not run under a legend that takes room at its side.
    void aPlotTheFileFixesStaysOffTheLegend()
    {
        const QString plot = oneSeriesPlot(QStringLiteral("FF0000"), {1, 1, 1});
        const QByteArray part = withPlotLayout(chartPart(plot, QString(), QStringLiteral("<c:legend><c:legendPos val=\"r\"/><c:overlay val=\"0\"/></c:legend>")), manualLayout(0.05, 0.05, 0.9, 0.8, QStringLiteral("inner")));
        const Chart c = readOrFail(part);
        QVERIFY(c.plotManual);
        const QImage img = renderChart(c, QSizeF(400, 300), 2);
        // the columns touch and fill the plot's width: where they end is where the plot does; the key (a red square) is at the legend, and the plot stops short of it (apart from it: two
        // patches of red, not one that the plot runs into the legend with)
        // (the thin lines between the categories may cut the columns' red, by where they fall in a pixel, so the plot is every red run but the last, which is the key)
        const QVector<QRect> reds = colorRuns(img, Qt::red, false);
        QVERIFY(reds.size() >= 2);
        const QRect key = reds.last();
        QVERIFY2(key.height() < img.height() / 10, qPrintable(QString::number(key.height())));
        int plotRight = 0;
        for (int i = 0; i + 1 < reds.size(); ++i) plotRight = std::max(plotRight, reds[i].right());
        QVERIFY2(plotRight < key.left() - 4, qPrintable(QStringLiteral("%1 %2").arg(plotRight).arg(key.left())));
        // with the legend moved over the plot, nothing is taken from it
        const Chart over = readOrFail(withPlotLayout(chartPart(plot, QString(), QStringLiteral("<c:legend><c:legendPos val=\"r\"/><c:overlay val=\"1\"/></c:legend>")), manualLayout(0.05, 0.05, 0.9, 0.8, QStringLiteral("inner"))));
        const QImage full = renderChart(over, QSizeF(400, 300), 2);
        QVERIFY(colorBounds(full, Qt::red, false).right() > full.width() * 0.9);
    }

    // The title of an axis stands at the edge of the chart, outside the plot whatever the plot's own place, and where its own layout puts it, if it has one.
    void axisTitlesStandOutsideAPlotTheFileFixes()
    {
        const QString title = QStringLiteral("<c:title><c:tx><c:rich><a:bodyPr/><a:p><a:pPr><a:defRPr sz=\"1000\"/></a:pPr><a:r><a:rPr lang=\"en-US\" sz=\"1000\"><a:solidFill><a:srgbClr val=\"FF00FF\"/></a:solidFill></a:rPr><a:t>%1</a:t></a:r></a:p></c:rich></c:tx>%2<c:overlay val=\"0\"/></c:title>");
        const QString plot = oneSeriesPlot(QStringLiteral("0000FF"), {1, 0.5}, QStringLiteral("Dollars"), QString(title).replace(QStringLiteral("%2"), QString()));
        const QSizeF size(400, 300);
        const QString inner = manualLayout(0.2, 0.1, 0.75, 0.7, QStringLiteral("inner"));
        const Chart c = readOrFail(withPlotLayout(chartPart(plot), inner));
        QVERIFY(c.plotManual && c.plotInner);
        const QImage img = renderChart(c, size, 2);
        const QRect text = colorBounds(img, QColor(255, 0, 255), false);
        QVERIFY(!text.isNull());
        // the title is a column of turned text at the left edge of the chart, left of the plot (and of the labels along it)
        QVERIFY2(text.right() < 0.2 * img.width() - 12, qPrintable(QString::number(text.right())));
        QVERIFY2(text.left() < 0.08 * img.width(), qPrintable(QString::number(text.left())));
        // the same with a plot whose place is the whole of what the plot and its labels take (an outer target)
        const Chart outerPlot = readOrFail(withPlotLayout(chartPart(plot), manualLayout(0.2, 0.1, 0.75, 0.7, QStringLiteral("outer"))));
        const QRect outerText = colorBounds(renderChart(outerPlot, size, 2), QColor(255, 0, 255), false);
        QVERIFY(!outerText.isNull());
        QVERIFY2(outerText.left() >= 0.2 * 2 * size.width() - 6 && outerText.left() < 0.2 * 2 * size.width() + 20, qPrintable(QString::number(outerText.left())));
        // a title with a place of its own: its corner is where the layout says (as fractions of the chart)
        const QString placed = QString(title).replace(QStringLiteral("%2"), manualLayout(0.02, 0.30, 0, 0));
        const Chart own = readOrFail(withPlotLayout(chartPart(oneSeriesPlot(QStringLiteral("0000FF"), {1, 0.5}, QStringLiteral("Dollars"), placed)), inner));
        QVERIFY(own.axes.size() >= 2);
        bool found = false;
        for (const ChartAxis &a : own.axes)
            if (a.title.manual) {
                found = true;
                QCOMPARE(a.title.x, 0.02);
                QCOMPARE(a.title.y, 0.30);
            }
        QVERIFY(found);
        const QRect put = colorBounds(renderChart(own, size, 2), QColor(255, 0, 255), false);
        QVERIFY(!put.isNull());
        // (the box of the turned text starts there; the letters a little in from its sides)
        QVERIFY2(put.left() >= 0.02 * 2 * size.width() - 2 && put.left() <= 0.02 * 2 * size.width() + 12, qPrintable(QString::number(put.left())));
        QVERIFY2(put.top() >= 0.30 * 2 * size.height() - 2 && put.top() <= 0.30 * 2 * size.height() + 12, qPrintable(QString::number(put.top())));
        // the title of the category axis, the same way
        const QString catPlaced = QString(title).replace(QStringLiteral("%2"), manualLayout(0.40, 0.90, 0, 0));
        const Chart cat = readOrFail(withPlotLayout(chartPart(oneSeriesPlot(QStringLiteral("0000FF"), {1, 0.5}, QString(), QString(), catPlaced.arg(QStringLiteral("Months")))), inner));
        const QRect catPut = colorBounds(renderChart(cat, size, 2), QColor(255, 0, 255), false);
        QVERIFY(!catPut.isNull());
        QVERIFY2(catPut.left() >= 0.40 * 2 * size.width() - 2 && catPut.left() <= 0.40 * 2 * size.width() + 12 && catPut.top() >= 0.90 * 2 * size.height() - 2 && catPut.top() <= 0.90 * 2 * size.height() + 14,
                 qPrintable(QStringLiteral("%1,%2").arg(catPut.left()).arg(catPut.top())));
    }

    // The title of the chart is across the top, or where its own layout puts its corner (as fractions of the chart); the plot is laid out the same either way.
    void aChartTitleStandsWhereItsLayoutPutsIt()
    {
        auto titled = [](const QString &layout) {
            return QStringLiteral("<c:title><c:tx><c:rich><a:bodyPr/><a:p><a:pPr><a:defRPr sz=\"1400\"/></a:pPr><a:r><a:rPr lang=\"en-US\" sz=\"1400\"><a:solidFill><a:srgbClr val=\"FF00FF\"/></a:solidFill></a:rPr><a:t>Sales</a:t></a:r></a:p></c:rich></c:tx>%1<c:overlay val=\"0\"/></c:title>")
                .arg(layout);
        };
        const QString plot = oneSeriesPlot(QStringLiteral("0000FF"), {1, 0.5});
        const QSizeF size(400, 300);
        const QColor magenta(255, 0, 255), blue(0, 0, 255);
        Chart usual = readOrFail(chartPart(plot, titled(QString())));
        QVERIFY(usual.title.present && !usual.title.manual);
        const QImage across = renderChart(usual, size, 2);
        const QRect centered = colorBounds(across, magenta, false);
        QVERIFY(!centered.isNull());
        QVERIFY2(qAbs(centered.center().x() - int(size.width())) <= 8, qPrintable(QString::number(centered.center().x())));   // (the middle of the 800 pixels)
        QVERIFY2(centered.top() < 40, qPrintable(QString::number(centered.top())));
        Chart own = readOrFail(chartPart(plot, titled(manualLayout(0.55, 0.30, 0, 0))));
        QVERIFY(own.title.manual);
        QCOMPARE(own.title.x, 0.55);
        QCOMPARE(own.title.y, 0.30);
        const QImage placed = renderChart(own, size, 2);
        const QRect put = colorBounds(placed, magenta, false);
        QVERIFY(!put.isNull());
        // (the box of the text starts at the corner; the letters a little in from it)
        QVERIFY2(put.left() >= 0.55 * 2 * size.width() - 2 && put.left() <= 0.55 * 2 * size.width() + 12, qPrintable(QString::number(put.left())));
        QVERIFY2(put.top() >= 0.30 * 2 * size.height() - 2 && put.top() <= 0.30 * 2 * size.height() + 14, qPrintable(QString::number(put.top())));
        // the plot is where it was (the room the title would have taken is kept)
        QCOMPARE(colorBounds(placed, blue, false), colorBounds(across, blue, false));
    }

    // A pie of negative values is drawn by their size, as the common programs draw it: the same as the pie of their absolute values.
    void aPieOfNegativeValuesIsDrawnByTheirSize()
    {
        auto pie = [&](const QVector<double> &values) {
            const QString plot = QStringLiteral("<c:pieChart><c:varyColors val=\"1\"/>") + chartSeries(0, QStringLiteral("S"), {"a", "b", "c"}, values) + QStringLiteral("<c:firstSliceAng val=\"0\"/></c:pieChart>");
            return renderChart(readOrFail(chartPart(plot)), QSizeF(300, 300), 2);
        };
        const QImage positive = pie({3, 1, 2});
        QVERIFY(colorBounds(positive, QColor(), true).width() > 200);
        QCOMPARE(pie({-3, -1, -2}), positive);
        QCOMPARE(pie({3, -1, 2}), positive);
        // values of nothing are no slice, and nothing at all is a blank page
        QVERIFY(colorBounds(pie({3, 0, 2}), QColor(), true).width() > 200);
        QVERIFY(colorBounds(pie({0, 0, 0}), QColor(), true).isNull());
    }

    // A number format code is at most 255 characters (the spreadsheet limit); a longer one is not applied, and the numbers show as they are.
    void chartIgnoresFormatCodesPastTheSpreadsheetLimit()
    {
        const QString fits = QStringLiteral("0.0") + QString(252, QLatin1Char('x')), tooLong = fits + QLatin1Char('x');
        QCOMPARE(fits.size(), 255);
        QCOMPARE(formatChartNumber(1234.5, fits), QStringLiteral("1234.5") + QString(252, QLatin1Char('x')));
        QCOMPARE(formatChartNumber(1234.5, tooLong), QStringLiteral("1234.5"));
        // The same through a chart: 100 categories that are numbers, with a code of 20,000 characters, read in no time.
        QString points;
        for (int i = 0; i < 100; ++i) points += QStringLiteral("<c:pt idx=\"%1\"><c:v>%1.5</c:v></c:pt>").arg(i);
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/><c:ser><c:idx val=\"0\"/><c:order val=\"0\"/>"
                                            "<c:cat><c:numRef><c:f>Sheet1!$A$2:$A$101</c:f><c:numCache><c:formatCode>0.0%1</c:formatCode><c:ptCount val=\"100\"/>%2</c:numCache></c:numRef></c:cat>"
                                            "<c:val><c:numRef><c:f>Sheet1!$B$2:$B$101</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val=\"100\"/>%2</c:numCache></c:numRef></c:val></c:ser>"
                                            "<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>")
                                 .arg(QString(20000, QLatin1Char('x')), points) + chartAxes();
        Chart c;
        QString err;
        const qint64 parsed = chartFormatCharsReadForTest();
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(c.plots[0].series[0].categories[3], QStringLiteral("3.5"));
        // (Counted: not one character of the 20,000 is parsed, once for each of the 100 categories or at all.)
        QCOMPARE(chartFormatCharsReadForTest() - parsed, qint64(0));
        // A code of the limit's length is parsed for each of its numbers, so the count can tell.
        QString fitsPlot = plot;
        fitsPlot.replace(QString(20000, QLatin1Char('x')), QString(252, QLatin1Char('x')));
        const qint64 parsedFits = chartFormatCharsReadForTest();
        QVERIFY2(readChart(chartPart(fitsPlot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QVERIFY(chartFormatCharsReadForTest() - parsedFits > 0);
    }

    // The text a chart keeps for a label is cut at a few hundred characters, and what can't be written in a picture is left out of it.
    void chartKeepsLabelsShortAndWritable()
    {
        const QString huge(100000, QLatin1Char('m'));
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>")
                             + chartSeries(0, huge, {huge, QStringLiteral("b")}, {1, 2}) + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot, QString(), QStringLiteral("<c:legend><c:legendPos val=\"r\"/></c:legend>")), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QVERIFY(c.plots[0].series[0].categories[0].size() <= kChartMaxLabel);
        QVERIFY(c.plots[0].series[0].name.size() <= kChartMaxLabel);
        QVERIFY2(chartSvg(c, QSizeF(400, 300)).size() < 100000, "the picture repeats the long text");
        // Text with characters XML has no place for (control characters, half a surrogate pair) still makes a picture a reader can parse.
        c.plots[0].series[0].categories[1] = QStringLiteral("a") + QChar(1) + QStringLiteral("b") + QChar(7) + QStringLiteral("c") + QChar(0xD800) + QChar(0xFFFE) + QStringLiteral("<&\">'");
        c.plots[0].series[0].name = QStringLiteral("n") + QChar(2) + QChar(0xDC00);
        const QByteArray svg = chartSvg(c, QSizeF(400, 300));
        QXmlStreamReader xml(svg);
        while (!xml.atEnd()) xml.readNext();
        QVERIFY2(!xml.hasError(), qPrintable(xml.errorString()));
        QVERIFY(QSvgRenderer(svg).isValid());
    }

    // However much data a chart holds, its picture is a few thousand tags at most: the time QSvgRenderer takes to read a picture grows with the
    // square of them, and 20,000 columns took minutes.
    void chartDrawsHugeDataInFewPieces()
    {
        const int n = 6000;
        QStringList names;
        QVector<double> values, blanks;
        for (int i = 0; i < n; ++i) {
            names << QStringLiteral("c%1").arg(i);
            values << 10 + (i * 7) % 50;
            blanks << (i % 2 ? values.last() : std::nan(""));
        }
        const QString labels = QStringLiteral("<c:dLbls><c:showLegendKey val=\"0\"/><c:showVal val=\"1\"/><c:showCatName val=\"1\"/><c:showSerName val=\"0\"/><c:showPercent val=\"0\"/></c:dLbls>");
        const QString bar = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>%1<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>");
        const QString line = QStringLiteral("<c:lineChart><c:grouping val=\"standard\"/>%1<c:marker val=\"1\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:lineChart>");
        const QString area = QStringLiteral("<c:areaChart><c:grouping val=\"standard\"/>%1<c:axId val=\"111\"/><c:axId val=\"222\"/></c:areaChart>");
        const QString pie = QStringLiteral("<c:pieChart><c:varyColors val=\"1\"/>%1</c:pieChart>");
        const QString legend = QStringLiteral("<c:legend><c:legendPos val=\"r\"/></c:legend>");
        QString xs, ys;
        for (int i = 0; i < n; ++i) {
            xs += QStringLiteral("<c:pt idx=\"%1\"><c:v>%2</c:v></c:pt>").arg(i).arg((i * 37) % 1000);
            ys += QStringLiteral("<c:pt idx=\"%1\"><c:v>%2</c:v></c:pt>").arg(i).arg(values[i]);
        }
        const QString scatter = QStringLiteral("<c:scatterChart><c:scatterStyle val=\"lineMarker\"/><c:ser><c:idx val=\"0\"/><c:order val=\"0\"/><c:xVal><c:numRef><c:f>S!$A$1</c:f><c:numCache><c:formatCode>General</c:formatCode>"
                                               "<c:ptCount val=\"%1\"/>%2</c:numCache></c:numRef></c:xVal><c:yVal><c:numRef><c:f>S!$B$1</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val=\"%1\"/>%3</c:numCache></c:numRef></c:yVal></c:ser>"
                                               "<c:axId val=\"111\"/><c:axId val=\"222\"/></c:scatterChart>").arg(n).arg(xs, ys);
        QString everyLabel = chartAxes(), tinyUnit = chartAxes();
        everyLabel.replace(QStringLiteral("<c:crossAx val=\"222\"/>"), QStringLiteral("<c:crossAx val=\"222\"/><c:tickLblSkip val=\"1\"/>"));
        tinyUnit.replace(QStringLiteral("<c:crossBetween val=\"between\"/>"), QStringLiteral("<c:crossBetween val=\"between\"/><c:majorUnit val=\"0.001\"/>"));
        struct Case {
            const char *name;
            QString plot, legend;
        };
        const Case cases[] = {
            {"columns", bar.arg(chartSeries(0, QStringLiteral("S"), names, values)) + chartAxes(), QString()},
            {"columns with a label on each", bar.arg(chartSeries(0, QStringLiteral("S"), names, values, QString(), labels)) + chartAxes(), legend},
            {"columns with every category named", bar.arg(chartSeries(0, QStringLiteral("S"), names, values)) + everyLabel, QString()},
            {"columns on a scale of tiny steps", bar.arg(chartSeries(0, QStringLiteral("S"), names, values)) + tinyUnit, QString()},
            {"many series of columns", bar.arg(chartSeries(0, QStringLiteral("A"), names.mid(0, 2000), values.mid(0, 2000)) + chartSeries(1, QStringLiteral("B"), names.mid(0, 2000), values.mid(0, 2000))
                                               + chartSeries(2, QStringLiteral("C"), names.mid(0, 2000), values.mid(0, 2000))) + chartAxes(), legend},
            {"line with markers", line.arg(chartSeries(0, QStringLiteral("S"), names, values)) + chartAxes(), legend},
            {"line with a label on each", line.arg(chartSeries(0, QStringLiteral("S"), names, values, QString(), labels)) + chartAxes(), QString()},
            {"area broken at blanks", area.arg(chartSeries(0, QStringLiteral("S"), names, blanks)) + chartAxes(), QString()},
            {"scatter", scatter + chartAxes(), legend},
            {"pie", pie.arg(chartSeries(0, QStringLiteral("S"), names, values)), legend},
            {"pie with a label on each", pie.arg(chartSeries(0, QStringLiteral("S"), names, values, QString(), labels)), QString()},
        };
        for (const Case &k : cases) {
            Chart c;
            QString err;
            QVERIFY2(readChart(chartPart(k.plot, QString(), k.legend), defaultTheme(), ColorMap(), &c, &err), k.name);
            QByteArray svg;
            std::unique_ptr<QSvgRenderer> r;
            QImage img(600, 400, QImage::Format_ARGB32_Premultiplied);
            svg = chartSvg(c, QSizeF(480, 320));
            r = std::make_unique<QSvgRenderer>(svg);
            // And drawn, as a picture is for the page it is on.
            img.fill(Qt::white);
            {
                QPainter painter(&img);
                r->render(&painter, QRectF(QPointF(), QSizeF(img.size())));
            }
            const qint64 tags = svg.count('<');
            QVERIFY2(tags <= 8000, qPrintable(QStringLiteral("%1: %2 tags").arg(QLatin1String(k.name)).arg(tags)));
            QVERIFY2(r->isValid(), k.name);
            // (The work of drawing it is its tags, counted above: the picture is held to a few thousand of them, however many points the chart has.)
            // It is still a picture of the chart: its axis, and (but for a pie) the plot's ink, are there.
            QVERIFY2(tags > 10, k.name);
        }
    }

    // A chart too big to draw says so, and one that is not does not.
    void chartSaysWhenItIsDrawnCoarser()
    {
        QStringList names;
        QVector<double> values;
        for (int i = 0; i < 12000; ++i) {
            names << QStringLiteral("c%1").arg(i);
            values << i % 90;
        }
        const QString line = QStringLiteral("<c:lineChart><c:grouping val=\"standard\"/>%1<c:marker val=\"0\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:lineChart>");
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(line.arg(chartSeries(0, QStringLiteral("S"), names, values)) + chartAxes()), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        bool coarser = false;
        const QByteArray big = chartSvg(c, QSizeF(480, 320), &coarser);
        QVERIFY(coarser);
        QVERIFY(big.size() < 2000000);
        QVERIFY(readChart(chartPart(line.arg(chartSeries(0, QStringLiteral("S"), names.mid(0, 200), values.mid(0, 200))) + chartAxes()), defaultTheme(), ColorMap(), &c, &err));
        coarser = true;
        chartSvg(c, QSizeF(480, 320), &coarser);
        QVERIFY(!coarser);
        // So is one that has points enough, but more labels on them than can be read.
        const QString labels = QStringLiteral("<c:dLbls><c:showLegendKey val=\"0\"/><c:showVal val=\"1\"/><c:showCatName val=\"0\"/><c:showSerName val=\"0\"/><c:showPercent val=\"0\"/></c:dLbls>");
        const QString bars = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>%1<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>");
        QVERIFY(readChart(chartPart(bars.arg(chartSeries(0, QStringLiteral("S"), names.mid(0, 800), values.mid(0, 800), QString(), labels)) + chartAxes()), defaultTheme(), ColorMap(), &c, &err));
        coarser = false;
        chartSvg(c, QSizeF(480, 320), &coarser);
        QVERIFY(coarser);
    }

    // A font name with the characters an SVG attribute can't hold doesn't make the whole chart vanish.
    void chartSurvivesAnyFontName()
    {
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("Only series"), {"a", "b"}, {1, 2})
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        const QString legend = QStringLiteral("<c:legend><c:legendPos val=\"r\"/></c:legend>");
        for (const QString &name : {QStringLiteral("Bad\"Name"), QStringLiteral("A<b>C"), QStringLiteral("R&amp;D"), QStringLiteral("It's"), QStringLiteral("x\"/><script>y"), QStringLiteral("a") + QChar(1) + QStringLiteral("b"), QString(5000, QLatin1Char('f'))}) {
            Chart c;
            QString err;
            QVERIFY2(readChart(chartPart(plot, QString(), legend), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
            // As the file names it (through a typeface attribute, which XML escapes), and as another reader might set it.
            c.font.family = name;
            c.fontFamily = name;
            const QByteArray svg = chartSvg(c, QSizeF(400, 300));
            QXmlStreamReader xml(svg);
            while (!xml.atEnd()) xml.readNext();
            QVERIFY2(!xml.hasError(), qPrintable(name.left(20) + QLatin1Char(' ') + xml.errorString()));
            QVERIFY2(svg.size() < 100000, "the font name is written once for every text");
            QVERIFY2(svg.contains(">Only series<"), qPrintable(name.left(20)));
            QVERIFY2(!svg.contains("<script"), qPrintable(name.left(20)));
            QVERIFY(QSvgRenderer(svg).isValid());
        }
    }

    // Numbers no picture can hold (a size or an axis of 1e300, values of 1e308) draw a picture and not coordinates that are not numbers.
    void chartSurvivesNumbersBeyondAPicture()
    {
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("S"), {"a", "b", "c"}, {1, 2, 3})
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>");
        QString huge = chartAxes();
        huge.replace(QStringLiteral("<c:valAx><c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/>"),
                     QStringLiteral("<c:valAx><c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/><c:max val=\"1e308\"/><c:min val=\"-1e308\"/>"));
        static const QRegularExpression notANumber(QStringLiteral("(?<![A-Za-z])(nan|-?inf)(?![A-Za-z])"));
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot + huge), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QVERIFY(c.axes.size() == 2 && !c.axes[1].autoMax);
        const QByteArray big = chartSvg(c, QSizeF(400, 300));
        QVERIFY2(!QString::fromUtf8(big).contains(notANumber), qPrintable(notANumber.match(QString::fromUtf8(big)).captured()));
        QVERIFY(QSvgRenderer(big).isValid());
        // Values past what an axis can be made of are blanks.
        for (const double v : {1e308, -1e308, 1e200}) {
            Chart d;
            QVERIFY2(readChart(chartPart(QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("S"), {"a", "b", "c"}, {1, v, -v})
                                         + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes()), defaultTheme(), ColorMap(), &d, &err), qPrintable(err));
            const QString svg = QString::fromUtf8(chartSvg(d, QSizeF(400, 300)));
            QVERIFY2(!svg.contains(notANumber), qPrintable(QString::number(v)));
            QVERIFY(QSvgRenderer(svg.toUtf8()).isValid());
        }
        // A size is at most a slide's longest side, and a size that is no number is a point.
        const QString huger = QString::fromUtf8(chartSvg(c, QSizeF(1e300, 1e300)));
        QVERIFY2(huger.contains(QStringLiteral("viewBox=\"0 0 4032 4032\"")), qPrintable(huger.left(300)));
        const QString none = QString::fromUtf8(chartSvg(c, QSizeF(std::nan(""), std::numeric_limits<double>::infinity())));
        QVERIFY2(!none.contains(notANumber), qPrintable(none.left(300)));
    }

    // Gridlines, the marks of axes, and markers drawn one by one when a chart has few, and as one path when it has many, and a line of many
    // points drawn as one path or in paths of some segments each, make the same picture.
    void chartMergedPiecesLookTheSame()
    {
        const QString dashed = QStringLiteral("<c:spPr><a:ln w=\"9525\"><a:solidFill><a:srgbClr val=\"808080\"/></a:solidFill><a:prstDash val=\"dash\"/></a:ln></c:spPr>");
        QStringList names;
        QVector<double> a, b, walk;
        double at = 50;
        for (int i = 0; i < 40; ++i) {
            names << QStringLiteral("cat %1").arg(i);
            a << 20 + (i * 13) % 37;
            b << 10 + (i * 29) % 41;
        }
        QStringList manyNames;
        for (int i = 0; i < 1500; ++i) {
            manyNames << QString::number(i);
            at += ((i * 7919) % 31) - 15;
            walk << at;
        }
        QString dashedAxes = chartAxes();
        dashedAxes.replace(QStringLiteral("<c:majorGridlines/>"), QStringLiteral("<c:majorGridlines>") + dashed + QStringLiteral("</c:majorGridlines>"));
        const QString red = QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill><a:ln w=\"12700\"><a:solidFill><a:srgbClr val=\"000000\"/></a:solidFill></a:ln></c:spPr>");
        const QString bars = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>%1<c:gapWidth val=\"60\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>");
        const QString line = QStringLiteral("<c:lineChart><c:grouping val=\"standard\"/>%1<c:marker val=\"0\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:lineChart>");
        struct Case {
            const char *name;
            QString plot;
            int separateLines, segments, separateMarkers;   // what makes the picture another way, against the usual
        };
        const QString dots = QStringLiteral("<c:marker><c:symbol val=\"circle\"/><c:size val=\"6\"/></c:marker>"), crosses = QStringLiteral("<c:marker><c:symbol val=\"x\"/><c:size val=\"6\"/></c:marker>");
        const QString withMarkers = QStringLiteral("<c:lineChart><c:grouping val=\"standard\"/>%1<c:marker val=\"1\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:lineChart>");
        const Case cases[] = {
            {"columns with gridlines and ticks", bars.arg(chartSeries(0, QStringLiteral("A"), names, a, red) + chartSeries(1, QStringLiteral("B"), names, b)) + chartAxes(), 0, 0, -1},
            {"columns with dashed gridlines", bars.arg(chartSeries(0, QStringLiteral("A"), names, a)) + dashedAxes, 0, 0, -1},
            {"a line of 1,500 points", line.arg(chartSeries(0, QStringLiteral("A"), manyNames, walk)) + chartAxes(), -1, 20, -1},
            {"a line of 1,500 points, one path", line.arg(chartSeries(0, QStringLiteral("A"), manyNames, walk)) + chartAxes(), -1, 100000, -1},
            {"lines with markers", withMarkers.arg(chartSeries(0, QStringLiteral("A"), names, a, QString(), dots) + chartSeries(1, QStringLiteral("B"), names, b, QString(), crosses)) + chartAxes(), -1, 0, 0},
        };
        for (const Case &k : cases) {
            Chart c;
            QString err;
            QVERIFY2(readChart(chartPart(k.plot, QString(), QStringLiteral("<c:legend><c:legendPos val=\"b\"/></c:legend>")), defaultTheme(), ColorMap(), &c, &err), k.name);
            const QSizeF size(500, 300);
            auto draw = [&](const QByteArray &svg) {
                QSvgRenderer r(svg);
                QImage img(1000, 600, QImage::Format_ARGB32);
                img.fill(Qt::white);
                QPainter p(&img);
                r.render(&p, QRectF(QPointF(), QSizeF(img.size())));
                return img;
            };
            const QByteArray usual = chartSvg(c, size), other = chartSvgTuned(c, size, k.separateLines, k.segments, k.separateMarkers);
            QVERIFY2(usual != other, qPrintable(QStringLiteral("%1 is drawn the same way both times").arg(QLatin1String(k.name))));
            const QImage one = draw(usual), two = draw(other);
            // Where two strokes meet the edges come out a shade apart (as dark as the strokes' own where they overlap); nothing else may: what
            // differs is a small part of the ink of the picture.
            double differs = 0, ink = 0;
            for (int y = 0; y < one.height(); ++y)
                for (int x = 0; x < one.width(); ++x) {
                    ink += 255 - qGray(one.pixel(x, y));
                    differs += qAbs(qGray(one.pixel(x, y)) - qGray(two.pixel(x, y)));
                }
            QVERIFY2(ink > 20000, qPrintable(QStringLiteral("%1 draws no ink").arg(QLatin1String(k.name))));
            QVERIFY2(differs <= 0.02 * ink, qPrintable(QStringLiteral("%1: %2 of %3").arg(QLatin1String(k.name)).arg(differs).arg(ink)));
        }
    }

    // A line that crosses its plot back and forth thousands of times takes seconds to fill; drawn from fewer of its points, it takes a fraction of one.
    void chartDrawsLinesThatCrossTheirPlotQuickly()
    {
        QString xs, ys;
        quint32 state = 12345;
        auto next = [&] { state = state * 1664525u + 1013904223u; return (state >> 8) % 1000; };
        for (int i = 0; i < 2000; ++i) {
            xs += QStringLiteral("<c:pt idx=\"%1\"><c:v>%2</c:v></c:pt>").arg(i).arg(next());
            ys += QStringLiteral("<c:pt idx=\"%1\"><c:v>%2</c:v></c:pt>").arg(i).arg(next());
        }
        const QString scatter = QStringLiteral("<c:scatterChart><c:scatterStyle val=\"lineMarker\"/><c:ser><c:idx val=\"0\"/><c:order val=\"0\"/><c:spPr><a:ln w=\"28575\"><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></a:ln></c:spPr>"
                                               "<c:marker><c:symbol val=\"none\"/></c:marker><c:xVal><c:numRef><c:f>S!$A$1</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val=\"2000\"/>%1</c:numCache></c:numRef></c:xVal>"
                                               "<c:yVal><c:numRef><c:f>S!$B$1</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val=\"2000\"/>%2</c:numCache></c:numRef></c:yVal></c:ser>"
                                               "<c:axId val=\"111\"/><c:axId val=\"222\"/></c:scatterChart>").arg(xs, ys);
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(scatter + chartAxes()), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QImage img = renderChart(c, QSizeF(500, 333), 2);
        // Counted: a line of two thousand crossings is drawn in paths of a few dozen segments each (a path of all of them takes seconds to fill), and in all the tags the
        // picture is held to.
        const QByteArray svg = chartSvg(c, QSizeF(500, 333));
        int longest = 0;
        for (const QRegularExpressionMatch &m : QRegularExpression(QStringLiteral(" d=\"([^\"]*)\"")).globalMatch(QString::fromUtf8(svg)))
            longest = std::max<int>(longest, m.captured(1).count(QRegularExpression(QStringLiteral("[LCQ]"))));
        QVERIFY2(longest > 0 && longest <= 64 && svg.count('<') <= 8000, qPrintable(QStringLiteral("%1 segments in the longest path, %2 tags").arg(longest).arg(svg.count('<'))));
        // It is still a picture of the line: the plot has plenty of red in it.
        QVERIFY(colorArea(img, Qt::red) > 2000);
    }

private:
    // The text of a chart picture: each piece's left end, its baseline, and its width (measured in the font the picture names).
    struct TextAt {
        QString text;
        double x = 0, y = 0, width = 0;
    };
    static QVector<TextAt> textsOf(const QByteArray &svg)
    {
        QVector<TextAt> out;
        const QRegularExpression re(QStringLiteral("transform=\"matrix\\(1,0,0,1,([-0-9.e]+),([-0-9.e]+)\\)\"[^>]*>\\s*<text [^>]* x=\"([-0-9.e]+)\" y=\"([-0-9.e]+)\" font-family=\"([^\"]*)\" font-size=\"([0-9.]+)\"[^>]*>([^<]+)</text>"));
        for (const QRegularExpressionMatch &m : re.globalMatch(QString::fromUtf8(svg))) {
            TextAt t;
            t.text = m.captured(7);
            t.x = m.captured(1).toDouble() + m.captured(3).toDouble();
            t.y = m.captured(2).toDouble() + m.captured(4).toDouble();
            QFont f(m.captured(5));
            f.setPixelSize(int(m.captured(6).toDouble()));
            t.width = QFontMetricsF(f).horizontalAdvance(t.text);
            out << t;
        }
        return out;
    }
    static TextAt textNamed(const QVector<TextAt> &texts, const char *name)
    {
        for (const TextAt &t : texts)
            if (t.text == QLatin1String(name)) return t;
        return TextAt();
    }
    // A chart part: `body` (the plot area's kinds and axes) in a c:chart, after an optional title, with an optional legend.
    static QByteArray chartPart(const QString &body, const QString &title = QString(), const QString &legend = QString(), const QString &chartSpaceExtra = QString())
    {
        return (QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<c:chartSpace xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\" "
                               "xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
                               "<c:date1904 val=\"0\"/><c:roundedCorners val=\"0\"/><c:chart>")
                + title + (title.isEmpty() ? QStringLiteral("<c:autoTitleDeleted val=\"1\"/>") : QString())
                + QStringLiteral("<c:plotArea><c:layout/>") + body + QStringLiteral("</c:plotArea>") + legend
                + QStringLiteral("<c:plotVisOnly val=\"1\"/><c:dispBlanksAs val=\"gap\"/></c:chart>") + chartSpaceExtra + QStringLiteral("</c:chartSpace>")).toUtf8();
    }
    static QString chartAxes()
    {
        return QStringLiteral("<c:catAx><c:axId val=\"111\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"b\"/><c:numFmt formatCode=\"General\" sourceLinked=\"1\"/>"
                              "<c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"222\"/><c:crosses val=\"autoZero\"/><c:auto val=\"1\"/><c:lblAlgn val=\"ctr\"/>"
                              "<c:lblOffset val=\"100\"/><c:noMultiLvlLbl val=\"0\"/></c:catAx>"
                              "<c:valAx><c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"l\"/><c:majorGridlines/><c:numFmt formatCode=\"General\" sourceLinked=\"1\"/>"
                              "<c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"111\"/><c:crosses val=\"autoZero\"/><c:crossBetween val=\"between\"/></c:valAx>");
    }
    // A series: its name, categories, and values as the file caches them (a NaN value is a point left out), then what the schema puts
    // after the color (markers, points, labels) and after the values.
    static QString chartSeries(int idx, const QString &name, const QStringList &categories, const QVector<double> &values, const QString &spPr = QString(), const QString &beforeData = QString(),
                               const QString &after = QString())
    {
        QString cats, vals;
        for (int i = 0; i < categories.size(); ++i) cats += QStringLiteral("<c:pt idx=\"%1\"><c:v>%2</c:v></c:pt>").arg(i).arg(categories[i]);
        for (int i = 0; i < values.size(); ++i)
            if (!std::isnan(values[i])) vals += QStringLiteral("<c:pt idx=\"%1\"><c:v>%2</c:v></c:pt>").arg(i).arg(values[i]);
        return QStringLiteral("<c:ser><c:idx val=\"%1\"/><c:order val=\"%1\"/><c:tx><c:strRef><c:f>Sheet1!$%2$1</c:f><c:strCache><c:ptCount val=\"1\"/><c:pt idx=\"0\"><c:v>%3</c:v></c:pt></c:strCache></c:strRef></c:tx>")
                   .arg(idx).arg(QChar('B' + idx % 20)).arg(name)
               + spPr + beforeData
               + QStringLiteral("<c:cat><c:strRef><c:f>Sheet1!$A$2:$A$%1</c:f><c:strCache><c:ptCount val=\"%2\"/>%3</c:strCache></c:strRef></c:cat>").arg(categories.size() + 1).arg(categories.size()).arg(cats)
               + QStringLiteral("<c:val><c:numRef><c:f>Sheet1!$B$2:$B$%1</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val=\"%2\"/>%3</c:numCache></c:numRef></c:val>")
                     .arg(values.size() + 1).arg(values.size()).arg(vals)
               + after + QStringLiteral("</c:ser>");
    }
    // The axes of a radar chart made here: the spokes and rings drawn, no labels on the axes, the scale 0 to 4 in steps of 1.
    static QString radarAxes()
    {
        return QStringLiteral("<c:catAx><c:axId val=\"111\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"1\"/><c:axPos val=\"b\"/><c:majorGridlines/><c:numFmt formatCode=\"General\" sourceLinked=\"1\"/>"
                              "<c:majorTickMark val=\"none\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"222\"/><c:crosses val=\"autoZero\"/><c:auto val=\"1\"/><c:lblAlgn val=\"ctr\"/>"
                              "<c:lblOffset val=\"100\"/><c:noMultiLvlLbl val=\"0\"/></c:catAx>"
                              "<c:valAx><c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/><c:max val=\"4\"/><c:min val=\"0\"/></c:scaling><c:delete val=\"1\"/><c:axPos val=\"l\"/><c:majorGridlines/>"
                              "<c:numFmt formatCode=\"General\" sourceLinked=\"1\"/><c:majorTickMark val=\"none\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"111\"/><c:crosses val=\"autoZero\"/>"
                              "<c:crossBetween val=\"between\"/><c:majorUnit val=\"1\"/></c:valAx>");
    }
    // A radar chart of four categories (spokes up, right, down, and left) with values 4, 2, 4, 2 on a scale of 0 to 4: the top and bottom points are at the rim, the left and right at half of it.
    static QString radarPlot(const char *style, const QString &spPr, const QString &beforeData)
    {
        return QStringLiteral("<c:radarChart><c:radarStyle val=\"%1\"/><c:varyColors val=\"0\"/>").arg(QLatin1String(style))
               + chartSeries(0, QStringLiteral("R"), {"a", "b", "c", "d"}, {4, 2, 4, 2}, spPr, beforeData) + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:radarChart>") + radarAxes();
    }

    // The chart drawn into a white picture, `scale` pixels to the point.
    static QImage renderChart(const Chart &chart, const QSizeF &size, int scale)
    {
        QSvgRenderer r(chartSvg(chart, size));
        QImage img(int(size.width() * scale), int(size.height() * scale), QImage::Format_ARGB32);
        img.fill(Qt::white);
        QPainter p(&img);
        r.render(&p, QRectF(QPointF(), QSizeF(img.size())));
        return img;
    }
    static bool isColor(QRgb p, const QColor &c, int tolerance)
    {
        return qAbs(qRed(p) - c.red()) <= tolerance && qAbs(qGreen(p) - c.green()) <= tolerance && qAbs(qBlue(p) - c.blue()) <= tolerance;
    }
    // The boxes of the separate patches of one color at least four pixels across and high (the bars of a chart), in order from the left, or from the top.
    static QVector<QRect> colorRuns(const QImage &img, const QColor &color, bool fromTop)
    {
        QVector<QRect> out;
        QVector<char> seen(img.width() * img.height(), 0);
        for (int y0 = 0; y0 < img.height(); ++y0)
            for (int x0 = 0; x0 < img.width(); ++x0) {
                if (seen[y0 * img.width() + x0] || !isColor(img.pixel(x0, y0), color, 40)) continue;
                QRect box(x0, y0, 1, 1);
                QVector<QPoint> stack{QPoint(x0, y0)};
                seen[y0 * img.width() + x0] = 1;
                while (!stack.isEmpty()) {
                    const QPoint p = stack.takeLast();
                    box = box.united(QRect(p, QSize(1, 1)));
                    for (const QPoint d : {QPoint(1, 0), QPoint(-1, 0), QPoint(0, 1), QPoint(0, -1)}) {
                        const QPoint q = p + d;
                        if (q.x() < 0 || q.y() < 0 || q.x() >= img.width() || q.y() >= img.height() || seen[q.y() * img.width() + q.x()] || !isColor(img.pixel(q), color, 40)) continue;
                        seen[q.y() * img.width() + q.x()] = 1;
                        stack << q;
                    }
                }
                if (box.width() >= 4 && box.height() >= 4) out << box;
            }
        std::sort(out.begin(), out.end(), [fromTop](const QRect &a, const QRect &b) { return fromTop ? a.top() < b.top() : a.left() < b.left(); });
        return out;
    }
    // The box of the pixels of one color; of everything that isn't white or nearly, for an invalid color.
    static QRect colorBounds(const QImage &img, const QColor &color, bool anyInk)
    {
        QRect box;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x) {
                const QRgb p = img.pixel(x, y);
                const bool hit = anyInk || !color.isValid() ? (qRed(p) < 200 || qGreen(p) < 200 || qBlue(p) < 200) : isColor(p, color, 40);
                if (!hit) continue;
                box = box.isNull() ? QRect(x, y, 1, 1) : box.united(QRect(x, y, 1, 1));
            }
        return box;
    }
    static double colorArea(const QImage &img, const QColor &color)
    {
        double n = 0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x)
                if (isColor(img.pixel(x, y), color, 40)) ++n;
        return n;
    }
    static QPointF colorCentroid(const QImage &img, const QColor &color)
    {
        double n = 0, sx = 0, sy = 0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x)
                if (isColor(img.pixel(x, y), color, 40)) {
                    ++n;
                    sx += x;
                    sy += y;
                }
        return n > 0 ? QPointF(sx / n, sy / n) : QPointF();
    }
    // The centers of the separate patches of one color with at least `minPixels` pixels, from the left.
    static QVector<QPointF> colorComponents(const QImage &img, const QColor &color, int minPixels)
    {
        QVector<QPointF> out;
        QVector<char> seen(img.width() * img.height(), 0);
        for (int y0 = 0; y0 < img.height(); ++y0)
            for (int x0 = 0; x0 < img.width(); ++x0) {
                if (seen[y0 * img.width() + x0] || !isColor(img.pixel(x0, y0), color, 40)) continue;
                double n = 0, sx = 0, sy = 0;
                QVector<QPoint> stack{QPoint(x0, y0)};
                seen[y0 * img.width() + x0] = 1;
                while (!stack.isEmpty()) {
                    const QPoint p = stack.takeLast();
                    ++n;
                    sx += p.x();
                    sy += p.y();
                    for (const QPoint d : {QPoint(1, 0), QPoint(-1, 0), QPoint(0, 1), QPoint(0, -1)}) {
                        const QPoint q = p + d;
                        if (q.x() < 0 || q.y() < 0 || q.x() >= img.width() || q.y() >= img.height() || seen[q.y() * img.width() + q.x()] || !isColor(img.pixel(q), color, 40)) continue;
                        seen[q.y() * img.width() + q.x()] = 1;
                        stack << q;
                    }
                }
                if (n >= minPixels) out << QPointF(sx / n, sy / n);
            }
        std::sort(out.begin(), out.end(), [](const QPointF &a, const QPointF &b) { return a.x() < b.x(); });
        return out;
    }
};

JP_TEST_CLASS(OfficeChartTests)
#include "tst_office_chart.moc"
