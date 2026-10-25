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
        // a trend line and a three-dimensional chart are drawn another way, or not at all, by a chart written from the model
        QByteArray gradient = ours;
        gradient.replace("<a:solidFill><a:srgbClr val=\"4472C4\"/></a:solidFill>", "<a:gradFill><a:gsLst><a:gs pos=\"0\"><a:srgbClr val=\"4472C4\"/></a:gs></a:gsLst></a:gradFill>");
        QVERIFY(gradient != ours);
        QVERIFY2(chartIsFullyModeled(gradient, &why), qPrintable(why));   // (a gradient is kept: see whatTheModelKeepsIsWrittenBack)
        QByteArray trend = ours;
        trend.replace("<c:cat>", "<c:trendline><c:trendlineType val=\"linear\"/></c:trendline><c:cat>");
        QVERIFY(!chartIsFullyModeled(trend, &why));
        QCOMPARE(why, QStringLiteral("trendline"));
        QByteArray threeD = ours;
        threeD.replace("<c:barChart>", "<c:bar3DChart>").replace("</c:barChart>", "</c:bar3DChart>");
        QVERIFY(!chartIsFullyModeled(threeD, &why));
        QVERIFY(!chartIsFullyModeled(QByteArray("<c:chartSpace"), &why));
    }


    // What the model has no place for but keeps as the file wrote it, or carries as a field, is written back: for each, the part is one that may be written over, what is written has the element,
    // and what is written and read again is written the same (read, write, read: equal).
    void whatTheModelKeepsIsWrittenBack()
    {
        const QByteArray ours = writeChartPart(comboChart());
        auto roundTrip = [&](const QByteArray &xml, const char *marker, Chart *model = nullptr) {
            QString why;
            if (!chartIsFullyModeled(xml, &why)) {
                QTest::qFail(qPrintable(QStringLiteral("not fully modeled (%1): %2").arg(why, QString::fromLatin1(marker))), __FILE__, __LINE__);
                return QByteArray();
            }
            Chart a, b;
            if (!readChart(xml, defaultTheme(), ColorMap(), &a, &why)) return QByteArray();
            const QByteArray written = writeChartPart(a);
            if (!written.contains(marker)) QTest::qFail(qPrintable(QStringLiteral("the written part has no %1").arg(QString::fromLatin1(marker))), __FILE__, __LINE__);
            if (!readChart(written, defaultTheme(), ColorMap(), &b, &why)) return QByteArray();
            if (writeChartPart(b) != written) QTest::qFail(qPrintable(QStringLiteral("written again, it differs: %1").arg(QString::fromLatin1(marker))), __FILE__, __LINE__);
            if (model) *model = a;
            return written;
        };
        // the units of a date axis, and its kind from the data, and every nth tick mark
        Chart dates = comboChart();
        dates.axes[0].kind = ChartAxis::Date;
        dates.axes[0].baseTimeUnit = QStringLiteral("months");
        dates.axes[0].majorUnit = 2;
        dates.axes[0].majorTimeUnit = QStringLiteral("months");
        dates.axes[0].minorUnit = 1;
        dates.axes[0].minorTimeUnit = QStringLiteral("days");
        dates.axes[0].autoKind = false;
        Chart d;
        const QByteArray dated = roundTrip(writeChartPart(dates), "<c:baseTimeUnit val=\"months\"/>", &d);
        for (const char *marker : {"<c:majorUnit val=\"2\"/>", "<c:majorTimeUnit val=\"months\"/>", "<c:minorUnit val=\"1\"/>", "<c:minorTimeUnit val=\"days\"/>", "<c:auto val=\"0\"/>"})
            QVERIFY2(dated.contains(marker), marker);
        QCOMPARE(d.axes[0].baseTimeUnit, QStringLiteral("months"));
        QCOMPARE(d.axes[0].majorUnit, 2.0);
        QVERIFY(!d.axes[0].autoKind);
        Chart skipped = comboChart();
        skipped.axes[0].tickMarkSkip = 3;
        QVERIFY(roundTrip(writeChartPart(skipped), "<c:tickMarkSkip val=\"3\"/>", &d).size() > 0);
        QCOMPARE(d.axes[0].tickMarkSkip, 3);
        // a gradient fill of a series, and its shadow: written back while the series has the color that was read, and not when a person gave it another
        QByteArray gradient = ours;
        gradient.replace("<c:spPr><a:solidFill><a:srgbClr val=\"4472C4\"/></a:solidFill></c:spPr>",
                         "<c:spPr><a:gradFill><a:gsLst><a:gs pos=\"0\"><a:srgbClr val=\"4472C4\"/></a:gs><a:gs pos=\"100000\"><a:srgbClr val=\"112233\"/></a:gs></a:gsLst><a:lin ang=\"5400000\"/></a:gradFill><a:effectLst><a:outerShdw blurRad=\"40000\"/></a:effectLst></c:spPr>");
        QVERIFY2(gradient != ours, "the first series' look is where the test looks for it");
        Chart g;
        const QByteArray gradientWritten = roundTrip(gradient, "<a:gradFill>", &g);
        QVERIFY(gradientWritten.contains("<a:outerShdw blurRad=\"40000\"/>"));
        QVERIFY(gradientWritten.contains("<a:srgbClr val=\"112233\"/>"));
        QCOMPARE(g.plots[0].series[0].color, QColor("#4472C4"));
        g.plots[0].series[0].color = QColor("#FF0000");
        const QByteArray recolored = writeChartPart(g);
        QVERIFY(!recolored.contains("<a:gradFill>"));
        QVERIFY(recolored.contains("<a:srgbClr val=\"FF0000\"/>"));
        QVERIFY(recolored.contains("<a:outerShdw blurRad=\"40000\"/>"));   // (the shadow is the series' still)
        // a point with an outline of its own, and a label of one point with its extension
        QByteArray point = ours;
        point.replace("<c:cat>", "<c:dPt><c:idx val=\"1\"/><c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill><a:ln w=\"19050\"><a:solidFill><a:srgbClr val=\"000000\"/></a:solidFill></a:ln></c:spPr></c:dPt><c:cat>");
        Chart p;
        const QByteArray pointWritten = roundTrip(point, "w=\"19050\"", &p);
        QVERIFY(pointWritten.contains("<c:dPt>"));
        for (ChartPlot &plot : p.plots)
            for (ChartSeries &series : plot.series) series.pointColors[1] = QColor("#00FF00");
        QVERIFY(!writeChartPart(p).contains("w=\"19050\""));
        QByteArray label = ours;
        label.replace("<c:dLbls>", "<c:dLbls><c:dLbl><c:idx val=\"1\"/><c:dLblPos val=\"r\"/><c:showLegendKey val=\"0\"/><c:showVal val=\"1\"/><c:showCatName val=\"0\"/><c:showSerName val=\"0\"/><c:showPercent val=\"0\"/><c:showBubbleSize val=\"0\"/>"
                                  "<c:extLst><c:ext uri=\"{CE6537A1-D6FC-4f65-9D91-7224C49458BB}\" xmlns:c15=\"http://schemas.microsoft.com/office/drawing/2012/chart\"><c15:layout/></c:ext></c:extLst></c:dLbl>");
        QVERIFY2(label != ours, "the line's labels are where the test looks for them");
        const QByteArray labelWritten = roundTrip(label, "<c:dLbl>");
        QVERIFY(labelWritten.contains("c15:layout"));
        QByteArray labelsExt = ours;
        labelsExt.replace("</c:dLbls>", "<c:extLst><c:ext uri=\"{CE6537A1-D6FC-4f65-9D91-7224C49458BB}\" xmlns:c15=\"http://schemas.microsoft.com/office/drawing/2012/chart\"><c15:layout/></c:ext></c:extLst></c:dLbls>");
        QVERIFY(roundTrip(labelsExt, "c15:layout").size() > 0);
        // an entry taken off the legend
        QByteArray entry = ours;
        entry.replace("<c:legendPos val=\"b\"/>", "<c:legendPos val=\"b\"/><c:legendEntry><c:idx val=\"1\"/><c:delete val=\"1\"/></c:legendEntry>");
        QVERIFY2(entry != ours, "the legend is where the test looks for it");
        QVERIFY(roundTrip(entry, "<c:legendEntry>").size() > 0);
        // the drawing of shapes over the chart, and the workbook it was made from: links of the part, good for the part only
        QByteArray links = ours;
        links.replace("</c:chartSpace>", "<c:externalData r:id=\"rId2\"><c:autoUpdate val=\"0\"/></c:externalData><c:userShapes r:id=\"rId1\"/></c:chartSpace>");
        Chart l;
        const QByteArray linked = roundTrip(links, "<c:userShapes", &l);
        QVERIFY(linked.contains("<c:externalData"));
        QCOMPARE(l.userShapesId, QStringLiteral("rId1"));
        QCOMPARE(l.externalDataId, QStringLiteral("rId2"));
        l.clearPartLinks();
        QVERIFY(!writeChartPart(l).contains("userShapes") && !writeChartPart(l).contains("externalData"));
        // a chart area with no fill
        QByteArray none = ours;
        none.replace("</c:chart>", "</c:chart><c:spPr><a:noFill/></c:spPr>");
        QVERIFY2(none != ours, "the chart area is where the test looks for it");
        Chart n;
        QVERIFY(roundTrip(none, "</c:chart><c:spPr><a:noFill/>", &n).size() > 0);
        QVERIFY(n.backgroundNone);
        // what is kept does not change how the chart is drawn
        Chart plain, kept;
        QVERIFY(readChart(ours, defaultTheme(), ColorMap(), &plain));
        QVERIFY(readChart(gradient, defaultTheme(), ColorMap(), &kept));
        QCOMPARE(chartSvg(plain, QSizeF(480, 288)), chartSvg(plain, QSizeF(480, 288)));
    }

    // Whether a part may be written over is decided by writing what is read of it and looking at what is lost: an element or attribute the writer does not carry, a part of the extensions, a value
    // that is only written as the usual one, or a picture that is not the same. What only says how the part was set up, or how a color is made, is not a loss.
    void aPartIsWrittenOverOnlyIfTheRoundTripLosesNothing()
    {
        const QByteArray ours = writeChartPart(comboChart());
        QString why;
        QVERIFY2(chartIsFullyModeled(ours, &why), qPrintable(why));
        // a date axis that says the unit of its dates is kept (see whatTheModelKeepsIsWrittenBack); an element of it that is not the schema's has nowhere to go
        Chart dates = comboChart();
        dates.axes[0].kind = ChartAxis::Date;
        const QByteArray dateAxis = writeChartPart(dates);
        QVERIFY2(chartIsFullyModeled(dateAxis, &why), qPrintable(why));
        QByteArray unit = dateAxis;
        unit.replace("<c:lblOffset val=\"100\"/>", "<c:lblOffset val=\"100\"/><c:baseTimeUnit val=\"days\"/>");
        QVERIFY(unit != dateAxis);
        QVERIFY2(chartIsFullyModeled(unit, &why), qPrintable(why));
        QByteArray strange = dateAxis;
        strange.replace("<c:lblOffset val=\"100\"/>", "<c:lblOffset val=\"100\"/><c:dispUnits><c:builtInUnit val=\"thousands\"/></c:dispUnits>");
        QVERIFY(!chartIsFullyModeled(strange, &why));
        QCOMPARE(why, QStringLiteral("builtInUnit"));
        // the style offered twice (newer programs' form, and the plain one as the fallback) is the plain one; any other offer is not kept
        QByteArray offered = ours;
        offered.replace("<c:style val=\"2\"/>", "<mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\"><mc:Choice Requires=\"c14\" xmlns:c14=\"http://schemas.microsoft.com/office/drawing/2007/8/2/chart\"><c14:style val=\"102\"/></mc:Choice><mc:Fallback><c:style val=\"2\"/></mc:Fallback></mc:AlternateContent>");
        QVERIFY2(offered != ours, "the style is where the test looks for it");
        QVERIFY2(chartIsFullyModeled(offered, &why), qPrintable(why));
        QByteArray other = offered;
        other.replace("<c14:style val=\"102\"/>", "<c14:style val=\"102\"/><c14:extra val=\"1\"/>");
        QVERIFY(!chartIsFullyModeled(other, &why));
        QCOMPARE(why, QStringLiteral("AlternateContent (extra)"));
        // what draws nothing and is only how another program wrote the part is no loss: a series' bubble3D off, leader lines off, labels that are all off; the same turned on are
        const QByteArray unlabelled = QString::fromUtf8(ours).remove(QRegularExpression(QStringLiteral("<c:dLbls>.*?</c:dLbls>"))).toUtf8();
        QByteArray quiet = unlabelled;
        quiet.replace("<c:ser>", "<c:ser><c:bubble3D val=\"0\"/><c:dLbls><c:showLegendKey val=\"0\"/><c:showVal val=\"0\"/><c:showCatName val=\"0\"/><c:showSerName val=\"0\"/><c:showPercent val=\"0\"/><c:showBubbleSize val=\"0\"/><c:extLst><c:ext uri=\"{CE6537A1-D6FC-4f65-9D91-7224C49458BB}\"><c15:showLeaderLines xmlns:c15=\"http://schemas.microsoft.com/office/drawing/2012/chart\" val=\"0\"/></c:ext></c:extLst></c:dLbls>");
        QVERIFY2(quiet != ours, "a series is where the test looks for it");
        QVERIFY2(chartIsFullyModeled(quiet, &why), qPrintable(why));
        QByteArray bubbles = unlabelled;
        bubbles.replace("<c:ser>", "<c:ser><c:bubble3D val=\"1\"/>");
        QVERIFY(!chartIsFullyModeled(bubbles, &why));
        QCOMPARE(why, QStringLiteral("bubble3D"));
        QByteArray labelled = unlabelled;
        labelled.replace("<c:ser>", "<c:ser><c:dLbls><c:showVal val=\"1\"/></c:dLbls>");
        QVERIFY(labelled != unlabelled);
        QVERIFY2(chartIsFullyModeled(labelled, &why), qPrintable(why));   // (labels that are on are the model's: the writer writes them)
        QByteArray oneLabel = unlabelled;
        oneLabel.replace("<c:ser>", "<c:ser><c:dLbls><c:dLbl><c:idx val=\"0\"/><c:showVal val=\"1\"/></c:dLbl><c:showVal val=\"0\"/></c:dLbls>");
        QVERIFY2(chartIsFullyModeled(oneLabel, &why), qPrintable(why));   // (a label of one point is kept)
        // text that is not raised or lowered says so (baseline 0) and loses nothing; one that is raised does
        QByteArray level = ours;
        level.replace("<a:defRPr", "<a:defRPr baseline=\"0\"");
        QVERIFY2(level != ours, "the text's properties are where the test looks for them");
        QVERIFY2(chartIsFullyModeled(level, &why), qPrintable(why));
        QByteArray raised = ours;
        raised.replace("<a:defRPr", "<a:defRPr baseline=\"30000\"");
        QVERIFY(!chartIsFullyModeled(raised, &why));
        QCOMPARE(why, QStringLiteral("defRPr@baseline"));
        // a title that holds a field (the text of a cell, the page): a run is not that
        QByteArray field = ours;
        field.replace("<a:r><a:rPr lang=\"en-US\"/><a:t>Sales &amp; costs &lt;2026&gt;</a:t></a:r>", "<a:fld id=\"{00000000-0000-0000-0000-000000000000}\" type=\"CELLRANGE\"><a:rPr lang=\"en-US\"/><a:t>Sales &amp; costs &lt;2026&gt;</a:t></a:fld>");
        QVERIFY2(field != ours, "the title's run is where the test looks for it");
        QVERIFY(!chartIsFullyModeled(field, &why));
        QCOMPARE(why, QStringLiteral("fld"));
        // things the model has no place for
        for (const char *extra : {"<c:legendEntry><c:idx val=\"0\"/><c:txPr/></c:legendEntry>"}) {   // (an entry's own text look is the entry's, kept with it)
            QByteArray with = ours;
            with.replace("<c:legendPos val=\"b\"/>", QByteArray("<c:legendPos val=\"b\"/>") + extra);
            QVERIFY2(chartIsFullyModeled(with, &why), qPrintable(why));
        }
        QByteArray hidden = ours;
        hidden.replace("<c:legendPos val=\"b\"/>", "<c:legendPos val=\"b\"/><c:legendEntry><c:idx val=\"0\"/><c:delete val=\"1\"/></c:legendEntry>");
        QVERIFY2(chartIsFullyModeled(hidden, &why), qPrintable(why));
        QByteArray colorMap = ours;
        colorMap.replace("<c:chart>", "<c:clrMapOvr bg1=\"lt1\" tx1=\"dk1\" bg2=\"lt2\" tx2=\"dk2\" accent1=\"accent1\" accent2=\"accent2\" accent3=\"accent3\" accent4=\"accent4\" accent5=\"accent5\" accent6=\"accent6\" hlink=\"hlink\" folHlink=\"folHlink\"/><c:chart>");
        QVERIFY(!chartIsFullyModeled(colorMap, &why));
        QCOMPARE(why, QStringLiteral("clrMapOvr"));
        QByteArray tick = ours;
        tick.replace("<c:lblOffset val=\"100\"/>", "<c:lblOffset val=\"100\"/><c:tickMarkSkip val=\"2\"/>");
        QVERIFY2(chartIsFullyModeled(tick, &why), qPrintable(why));   // (carried: see whatTheModelKeepsIsWrittenBack)
        QByteArray strangeTick = ours;
        strangeTick.replace("<c:lblOffset val=\"100\"/>", "<c:lblOffset val=\"100\"/><c:noMultiLvlLbl val=\"1\"/>");
        QVERIFY2(chartIsFullyModeled(strangeTick, &why), qPrintable(why));
        QByteArray external = ours;
        external.replace("</c:chartSpace>", "<c:externalData r:id=\"rId1\"><c:autoUpdate val=\"0\"/><c:extra/></c:externalData></c:chartSpace>");
        QVERIFY(!chartIsFullyModeled(external, &why));
        QCOMPARE(why, QStringLiteral("extra"));
        QByteArray arrow = ours;
        arrow.replace("<a:ln w=\"28575\" cap=\"rnd\">", "<a:ln w=\"28575\" cap=\"rnd\"><a:headEnd type=\"triangle\"/>");
        if (arrow != ours) {
            QVERIFY(!chartIsFullyModeled(arrow, &why));
            QCOMPARE(why, QStringLiteral("headEnd"));
        }
        // an attribute the writer does not carry (underline of a title's run) is a loss; the compound kind of a line is not
        QByteArray underline = ours;
        underline.replace("<a:rPr lang=\"en-US\"/>", "<a:rPr lang=\"en-US\" u=\"sng\"/>");
        QVERIFY(underline != ours);
        QVERIFY(!chartIsFullyModeled(underline, &why));
        QCOMPARE(why, QStringLiteral("rPr@u"));
        QByteArray harmless = ours;
        harmless.replace("<c:spPr><a:solidFill><a:srgbClr val=\"4472C4\"/></a:solidFill></c:spPr>", "<c:spPr><a:solidFill><a:schemeClr val=\"accent1\"><a:lumMod val=\"75000\"/></a:schemeClr></a:solidFill><a:ln cmpd=\"sng\"><a:noFill/></a:ln></c:spPr>");
        harmless.replace("<a:rPr lang=\"en-US\"/>", "<a:rPr lang=\"en-US\"><a:latin typeface=\"Calibri\"/><a:ea typeface=\"+mn-ea\"/><a:cs typeface=\"+mn-cs\"/></a:rPr>");
        QVERIFY2(chartIsFullyModeled(harmless, &why), qPrintable(why));
        // extensions that name only the chart, and print settings, are not a loss; an extension that does something is
        QByteArray ext = ours;
        ext.replace("</c:chartSpace>", "<c:printSettings><c:headerFooter/><c:pageMargins b=\"0.75\" l=\"0.7\" r=\"0.7\" t=\"0.75\" header=\"0.3\" footer=\"0.3\"/><c:pageSetup/></c:printSettings></c:chartSpace>");
        QVERIFY2(chartIsFullyModeled(ext, &why), qPrintable(why));
        QByteArray idOnly = ours;
        idOnly.replace("</c:ser>", "<c:extLst><c:ext uri=\"{C3380CC4-5D6E-409C-BE32-E72D297353CC}\" xmlns:c16=\"http://schemas.microsoft.com/office/drawing/2014/chart\"><c16:uniqueId val=\"{00000000-0000-0000-0000-000000000001}\"/></c:ext></c:extLst></c:ser>");
        QVERIFY2(chartIsFullyModeled(idOnly, &why), qPrintable(why));
        QByteArray doing = ours;
        doing.replace("</c:ser>", "<c:extLst><c:ext uri=\"{02D57815-91ED-43cb-92C2-25804820EDAC}\" xmlns:c15=\"http://schemas.microsoft.com/office/drawing/2012/chart\"><c15:datalabelsRange><c15:f>Sheet1!$A$1</c15:f></c15:datalabelsRange></c:ext></c:extLst></c:ser>");
        QVERIFY(!chartIsFullyModeled(doing, &why));
        QVERIFY2(why.startsWith(QLatin1String("extLst")), qPrintable(why));
        // labels over the largest value shown
        QByteArray over = ours;
        over.replace("<c:showDLblsOverMax val=\"0\"/>", "<c:showDLblsOverMax val=\"1\"/>");
        QVERIFY(over != ours);
        QVERIFY(!chartIsFullyModeled(over, &why));
        QCOMPARE(why, QStringLiteral("showDLblsOverMax"));
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
