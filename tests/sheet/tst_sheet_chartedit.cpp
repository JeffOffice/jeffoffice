// Tests of changing a chart's model (libs/office/sheet/chartedit.h): its elements on and off, colors and looks, kinds, series, and the rows and columns switched. Each change is also written and read
// back, since a change that is not saved is not made.
#include "drawinghelpers.h"

#include "core/fonts.h"
#include "office/chart_write.h"
#include "office/theme.h"
#include "sheet/chartedit.h"

using namespace jp::office;

namespace {

CellRange range(const QString &text)
{
    CellRange r;
    parseRangeRef(text, &r);
    return r;
}

ChartPlan planOf(const QString &sel, ChartKind kind, int inRows = -1)
{
    // the cells of "Sheet1" A1:C4 are headers and numbers: the plan is made from the block as it is
    Q_UNUSED(inRows);
    const CellRange r = range(sel);
    return planChartFrom(QStringLiteral("Sheet1"), r, true, true, false, kind);
}

Chart columns(ChartKind kind = ChartKind::Column)
{
    return makeChart(kind, planOf(QStringLiteral("A1:C4"), kind), defaultTheme(), 3);
}

Chart roundTrip(const Chart &c)
{
    Chart r;
    QString why;
    if (!readChart(writeChartPart(c), defaultTheme(), ColorMap(), &r, &why)) qFatal("%s", qPrintable(why));
    return r;
}

} // namespace

class SheetChartEditTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    // Drop lines, high-low lines, and up-down bars are offered for line plots (the last two with two series or more), set on every plot that takes them, and kept through writing.
    void linesBetweenSeriesGoOnAndOff()
    {
        Chart col = columns();
        QVERIFY(!plotLinesApply(col, PlotLines::Drop));
        Chart c = columns(ChartKind::Line);
        QVERIFY(plotLinesApply(c, PlotLines::Drop) && plotLinesApply(c, PlotLines::HighLow) && plotLinesApply(c, PlotLines::UpDown));
        QVERIFY(!plotLinesOn(c, PlotLines::HighLow));
        setPlotLines(&c, PlotLines::HighLow, true);
        setPlotLines(&c, PlotLines::UpDown, true);
        QVERIFY(plotLinesOn(c, PlotLines::HighLow) && plotLinesOn(c, PlotLines::UpDown) && !plotLinesOn(c, PlotLines::Drop));
        const Chart back = roundTrip(c);
        QVERIFY(plotLinesOn(back, PlotLines::HighLow) && plotLinesOn(back, PlotLines::UpDown));
        setPlotLines(&c, PlotLines::HighLow, false);
        QVERIFY(!plotLinesOn(c, PlotLines::HighLow));
        Chart area = columns(ChartKind::Area);
        QVERIFY(plotLinesApply(area, PlotLines::Drop) && !plotLinesApply(area, PlotLines::HighLow));
    }

    // "Value from cells" is set for every series from a reference, taken away again, and kept through writing and reading.
    void dataLabelsReadTheirWordsFromCells()
    {
        Chart c = columns();
        QVERIFY(dataLabelsCellsRef(c).isEmpty());
        setDataLabelsFromCells(&c, QStringLiteral("Sheet1!$D$2:$D$4"));
        QCOMPARE(dataLabelsCellsRef(c), QStringLiteral("Sheet1!$D$2:$D$4"));
        for (const ChartSeries &s : seriesOf(c)) {
            QVERIFY(s.labels.cells);
            QCOMPARE(s.labelCellsRef, QStringLiteral("Sheet1!$D$2:$D$4"));
        }
        const Chart back = roundTrip(c);
        QCOMPARE(dataLabelsCellsRef(back), QStringLiteral("Sheet1!$D$2:$D$4"));
        QVERIFY(seriesOf(back)[1].labels.cells);
        setDataLabelsFromCells(&c, QString());
        QVERIFY(dataLabelsCellsRef(c).isEmpty());
        QVERIFY(!seriesOf(c)[0].labels.shown());
    }

    // The elements of a chart on and off, and what the checks of a menu are told; each change is the same after the chart is written and read.
    void elementsGoOnAndOffAndStay()
    {
        Chart c = columns();
        ChartElements e = elementsOf(c);
        QVERIFY(!e.title && e.hasHorizontal && e.hasVertical && e.axisHorizontal && e.axisVertical && e.gridHorizontal && !e.gridVertical && e.legend);
        QCOMPARE(e.legendPosition, QStringLiteral("b"));
        setChartTitle(&c, TitleMode::Above);
        setAxisTitle(&c, true, true);
        setAxisTitle(&c, false, true);
        setGridlines(&c, false, false, true);   // (lines up and down)
        setGridlines(&c, true, true, true);     // (minor lines across)
        setLegend(&c, QStringLiteral("t"));
        setDataLabels(&c, true, QStringLiteral("outEnd"));
        setAxisShown(&c, true, false);
        for (const Chart &x : {c, roundTrip(c)}) {
            e = elementsOf(x);
            QVERIFY(e.title && !e.titleOverlay);
            QVERIFY(e.axisTitleHorizontal && e.axisTitleVertical);
            QVERIFY(e.gridVertical && e.gridHorizontal && e.minorGridHorizontal && !e.minorGridVertical);
            QCOMPARE(e.legendPosition, QStringLiteral("t"));
            QVERIFY(e.labels);
            QCOMPARE(e.labelPosition, QStringLiteral("outEnd"));
            QVERIFY(!e.axisHorizontal && e.axisVertical);
        }
        QCOMPARE(roundTrip(c).title.text, QStringLiteral("Chart Title"));
        QCOMPARE(primaryAxis(roundTrip(c), true)->title.text, QStringLiteral("Axis Title"));
        setChartTitle(&c, TitleMode::Overlay);
        QVERIFY(roundTrip(c).title.overlay);
        setChartTitle(&c, TitleMode::None);
        setLegend(&c, QString());
        setDataLabels(&c, false);
        setAxisShown(&c, true, true);
        setAxisTitle(&c, true, false);
        setGridlines(&c, false, false, false);
        e = elementsOf(roundTrip(c));
        QVERIFY(!e.title && !e.legend && !e.labels && e.axisHorizontal && !e.axisTitleHorizontal && !e.gridVertical);
        // bars across: the category axis runs up the chart, so lines across it are the value axis' ... the other way round
        Chart bars = columns(ChartKind::Bar);
        QVERIFY(primaryAxis(bars, false)->kind == ChartAxis::Category);
        setGridlines(&bars, true, false, false);   // (across: those of the axis that runs up, the categories')
        QVERIFY(!primaryAxis(bars, false)->majorGridlines);
        setGridlines(&bars, false, false, true);   // (up and down: the value axis, along the bottom)
        QVERIFY(primaryAxis(bars, true)->majorGridlines);
        // a pie has no axes: the changes to them do nothing and nothing breaks
        Chart pie = columns(ChartKind::Pie);
        e = elementsOf(pie);
        QVERIFY(!e.hasHorizontal && !e.hasVertical);
        setAxisTitle(&pie, true, true);
        setGridlines(&pie, true, false, true);
        setDataLabels(&pie, true, QStringLiteral("bestFit"));
        QVERIFY(pie.plots[0].series[0].labels.percent);
        QVERIFY(roundTrip(pie).plots[0].series[0].labels.percent);
    }

    // What a data label holds (the value, the category's name, the series' name, the percentage), what stands between them, and the number format: set for every series, kept by a save and a read.
    void dataLabelsHoldWhatIsChosen()
    {
        Chart c = columns();
        QVERIFY(!labelPartsOf(c).value);
        ChartLabelParts parts;
        parts.value = true;
        parts.category = true;
        parts.separator = QStringLiteral("; ");
        parts.format = QStringLiteral("0.0");
        setDataLabelParts(&c, parts);   // (none were shown: the labels come on, in the usual place)
        for (const Chart &x : {c, roundTrip(c)}) {
            const ChartLabelParts got = labelPartsOf(x);
            QVERIFY(got.value && got.category && !got.series && !got.percent);
            QCOMPARE(got.separator, QStringLiteral("; "));
            QCOMPARE(got.format, QStringLiteral("0.0"));
            for (const ChartPlot &p : x.plots)
                for (const ChartSeries &s : p.series) QVERIFY(s.labels.shown());
        }
        QVERIFY(elementsOf(c).labels);
        // the position is kept when the contents change
        setDataLabels(&c, true, QStringLiteral("outEnd"));
        parts.series = true;
        parts.value = false;
        setDataLabelParts(&c, parts);
        QCOMPARE(elementsOf(c).labelPosition, QStringLiteral("outEnd"));
        QVERIFY(labelPartsOf(roundTrip(c)).series && !labelPartsOf(roundTrip(c)).value);
        // everything off takes the labels away
        setDataLabelParts(&c, ChartLabelParts());
        QVERIFY(!elementsOf(c).labels);
    }

    // An axis formatted as a person does in its pane: bounds, units, order, where the other axis crosses, number format, tick marks, label place and turn; only those parts change, the rest of
    // the axis is as it was, and the file holds them.
    void anAxisIsFormatted()
    {
        Chart c = columns();
        const ChartAxis before = *primaryAxis(c, false);
        ChartAxis edit = before;
        edit.autoMin = false;
        edit.min = 5;
        edit.autoMax = false;
        edit.max = 50;
        edit.majorUnit = 10;
        edit.minorUnit = 2;
        edit.reversed = true;
        edit.crosses = QStringLiteral("max");
        edit.format = QStringLiteral("0.0");
        edit.formatLinked = false;
        edit.majorTick = QStringLiteral("in");
        edit.minorTick = QStringLiteral("cross");
        edit.labelPosition = QStringLiteral("low");
        edit.hasRotation = true;
        edit.labelRotation = -45;
        edit.title.text = QStringLiteral("not changed by this");
        setAxisFormat(&c, false, false, edit);
        for (const Chart &x : {c, roundTrip(c)}) {
            const ChartAxis *a = primaryAxis(x, false);
            QVERIFY(!a->autoMin && !a->autoMax);
            QCOMPARE(a->min, 5.0);
            QCOMPARE(a->max, 50.0);
            QCOMPARE(a->majorUnit, 10.0);
            QCOMPARE(a->minorUnit, 2.0);
            QVERIFY(a->reversed);
            QCOMPARE(a->crosses, QStringLiteral("max"));
            QCOMPARE(a->format, QStringLiteral("0.0"));
            QVERIFY(!a->formatLinked);
            QCOMPARE(a->majorTick, QStringLiteral("in"));
            QCOMPARE(a->minorTick, QStringLiteral("cross"));
            QCOMPARE(a->labelPosition, QStringLiteral("low"));
            QVERIFY(a->hasRotation);
            QCOMPARE(a->labelRotation, -45.0);
            QCOMPARE(a->title.text, before.title.text);   // (the title is not part of what the pane sets)
            QCOMPARE(a->id, before.id);
        }
        // the other axis is not touched
        QCOMPARE(primaryAxis(c, true)->reversed, false);
        // back to automatic
        setAxisFormat(&c, false, false, before);
        QVERIFY(primaryAxis(roundTrip(c), false)->autoMin);
        QVERIFY(!primaryAxis(roundTrip(c), false)->reversed);
        // an axis the chart does not have: nothing happens (a pie)
        Chart pie = columns(ChartKind::Pie);
        setAxisFormat(&pie, false, false, edit);
        QVERIFY(!primaryAxis(pie, false));
    }

    // An axis a pane made is refused when its lowest value is not under its highest (both set) or a unit is negative; automatic values are not compared.
    void anAxisThatCannotBeUsedIsFound()
    {
        Chart c = columns();
        ChartAxis a = *primaryAxis(c, false);
        QVERIFY(axisFormatProblem(a).isEmpty());
        a.autoMin = false;
        a.min = 10;
        QVERIFY(axisFormatProblem(a).isEmpty());   // (the highest is automatic)
        a.autoMax = false;
        a.max = 10;
        QVERIFY(!axisFormatProblem(a).isEmpty());
        a.max = 5;
        QVERIFY(!axisFormatProblem(a).isEmpty());
        a.max = 20;
        QVERIFY(axisFormatProblem(a).isEmpty());
        a.majorUnit = -1;
        QVERIFY(!axisFormatProblem(a).isEmpty());
        a.majorUnit = 0;
        a.minorUnit = -0.5;
        QVERIFY(!axisFormatProblem(a).isEmpty());
    }

    void aSingleSeriesTitleIsItsName()
    {
        Chart c = makeChart(ChartKind::Column, planOf(QStringLiteral("A1:B4"), ChartKind::Column), defaultTheme());
        QVERIFY(c.title.present && c.title.automatic);
        setChartTitle(&c, TitleMode::None);
        setChartTitle(&c, TitleMode::Above);
        QVERIFY(c.title.automatic);
        Chart two = columns();
        setChartTitle(&two, TitleMode::Above);
        QVERIFY(two.title.present && !two.title.automatic);
    }

    void labelPositionsFollowTheKind()
    {
        QVERIFY(labelPositionsFor(columns()).contains(QStringLiteral("outEnd")));
        QVERIFY(!labelPositionsFor(columns(ChartKind::ColumnStacked)).contains(QStringLiteral("outEnd")));
        QVERIFY(labelPositionsFor(columns(ChartKind::Line)).contains(QStringLiteral("t")));
        QVERIFY(labelPositionsFor(columns(ChartKind::Pie)).contains(QStringLiteral("bestFit")));
        QVERIFY(labelPositionsFor(columns(ChartKind::Area)).isEmpty());
    }

    void colorsAndLooksAreOurOwnAndStay()
    {
        QVERIFY(chartColorSchemeCount() >= 6 && chartStyleCount() >= 4 && chartLayoutCount() >= 4);
        for (int i = 0; i < chartColorSchemeCount(); ++i) QVERIFY(!chartColorSchemeName(i).isEmpty());
        Chart c = columns();
        const QColor before = c.plots[0].series[0].color;
        applyColorScheme(&c, 2);   // (greens)
        QVERIFY(c.plots[0].series[0].color != before);
        const Chart back = roundTrip(c);
        QCOMPARE(back.plots[0].series[0].color.name(), c.plots[0].series[0].color.name());
        QCOMPARE(back.plots[0].series[1].color.name(), chartSchemeColor(c, 2, 1).name());
        QVERIFY(back.plots[0].series[0].color != back.plots[0].series[1].color);
        // a pie's slices take the scheme too
        Chart pie = columns(ChartKind::Pie);
        applyColorScheme(&pie, 3);
        QCOMPARE(roundTrip(pie).plots[0].series[0].pointColors.value(1).name(), chartSchemeColor(pie, 3, 1).name());
        // a dark look turns the page and the text over, and is written
        applyChartStyle(&c, 2);
        const Chart dark = roundTrip(c);
        QCOMPARE(dark.background.name(), QStringLiteral("#262626"));
        QCOMPARE(dark.font.color.name(), QStringLiteral("#f2f2f2"));
        QVERIFY(chartSvg(dark, QSizeF(300, 200)) != chartSvg(roundTrip(columns()), QSizeF(300, 200)));
        applyChartStyle(&c, 1);
        const Chart clean = roundTrip(c);
        QVERIFY(clean.border.none);
        QVERIFY(!elementsOf(clean).gridHorizontal);
        applyChartStyle(&c, 0);
        QVERIFY(elementsOf(roundTrip(c)).gridHorizontal);
        applyChartStyle(&c, 3);
        QCOMPARE(roundTrip(c).plotFill.name(), QStringLiteral("#f2f2f2"));
        // the layouts set several things at once
        applyChartLayout(&c, 1);
        ChartElements e = elementsOf(roundTrip(c));
        QVERIFY(e.title && !e.legend && e.labels && !e.gridHorizontal);
        applyChartLayout(&c, 2);
        e = elementsOf(roundTrip(c));
        QVERIFY(e.axisTitleHorizontal && e.axisTitleVertical);
        QCOMPARE(e.legendPosition, QStringLiteral("r"));
        applyChartLayout(&c, 3);
        e = elementsOf(c);
        QVERIFY(!e.title && !e.legend && !e.labels && !e.gridHorizontal);
    }

    // The chart as another kind keeps its series and where they read, its words, and its look; columns with a line over them, the line against the second axes.
    void aChartChangesKindAndKeepsItsSeries()
    {
        Chart c = columns();
        c.plots[0].series[0].name = QStringLiteral("East");
        c.plots[0].series[0].color = QColor("#123456");
        setChartTitle(&c, TitleMode::Above);
        c.title.text = QStringLiteral("Sales");
        setAxisTitle(&c, false, true);
        c.legend.position = QStringLiteral("t");
        const Theme theme = defaultTheme();
        const ChartKind kinds[] = {ChartKind::Line, ChartKind::Area, ChartKind::Pie, ChartKind::Bar, ChartKind::ColumnStacked, ChartKind::Scatter, ChartKind::Doughnut, ChartKind::Combo};
        for (const ChartKind kind : kinds) {
            const Chart k = roundTrip(withKind(c, kind, theme));
            QCOMPARE(int(kindOfChart(k)), int(kind));
            QCOMPARE(seriesCount(k), kind == ChartKind::Pie || kind == ChartKind::Doughnut ? 2 : 2);
            const QVector<ChartSeries> s = seriesOf(k);
            QCOMPARE(s[0].valueRef, QStringLiteral("Sheet1!$B$2:$B$4"));
            QCOMPARE(s[1].valueRef, QStringLiteral("Sheet1!$C$2:$C$4"));
            QCOMPARE(k.title.text, QStringLiteral("Sales"));
        }
        // a pie: slices have colors; the series' own color is not what is drawn
        const Chart pie = withKind(c, ChartKind::Pie, theme);
        QCOMPARE(pie.plots[0].series.size(), 2);
        QVERIFY(!pie.plots[0].series[0].pointColors.isEmpty() || c.plots[0].series[0].values.isEmpty());
        // the old look: the legend's place and the vertical axis' title stay for a chart of the same shape
        const Chart line = withKind(c, ChartKind::Line, theme);
        QCOMPARE(line.legend.position, QStringLiteral("t"));
        QVERIFY(primaryAxis(line, false)->title.present);
        QCOMPARE(line.plots[0].series[0].color.name(), QStringLiteral("#123456"));
        // bars across turn the sides round: the title of the axis that ran up it is on the side that does now
        const Chart bars = withKind(c, ChartKind::Bar, theme);
        QVERIFY(primaryAxis(bars, false)->title.present);
        // a combo: the line against the second axes
        const Chart combo = roundTrip(withKind(c, ChartKind::Combo, theme));
        QCOMPARE(combo.plots.size(), 2);
        const QVector<SeriesKind> k = seriesKindsOf(combo);
        QVERIFY(!k[0].secondary && k[1].secondary);
        QCOMPARE(int(k[0].kind), int(ChartKind::Column));
        QCOMPARE(int(k[1].kind), int(ChartKind::LineMarkers));
        QCOMPARE(combo.axes.size(), 4);
    }

    // Choosing another kind changes the kind and nothing else of the trendline: its order, period, name, reach, and look stay, and so do the other trendlines of the series.
    void aChosenKindKeepsTheRestOfTheTrendlines()
    {
        Chart c = columns(ChartKind::Line);
        ChartTrendline first;
        first.type = QStringLiteral("poly");
        first.order = 4;
        first.period = 7;
        first.name = QStringLiteral("My fit");
        first.forward = 2;
        first.hasIntercept = true;
        first.intercept = 1.5;
        ChartTrendline second;
        second.type = QStringLiteral("movingAvg");
        second.period = 5;
        c.plots[0].series[0].trendlines = {first, second};
        setTrendlines(&c, QStringLiteral("exp"));
        const QVector<ChartTrendline> now = c.plots[0].series[0].trendlines;
        QCOMPARE(int(now.size()), 2);
        QCOMPARE(now[0].type, QStringLiteral("exp"));
        QCOMPARE(now[0].order, 4);
        QCOMPARE(now[0].period, 7);
        QCOMPARE(now[0].name, QStringLiteral("My fit"));
        QCOMPARE(now[0].forward, 2.0);
        QVERIFY(now[0].hasIntercept && now[0].intercept == 1.5);
        QCOMPARE(now[1].type, QStringLiteral("movingAvg"));
        QCOMPARE(now[1].period, 5);
        // a series with none is given one of the usual order and period
        QCOMPARE(c.plots[0].series[1].trendlines.value(0).type, QStringLiteral("exp"));
        QCOMPARE(c.plots[0].series[1].trendlines.value(0).order, 2);
        // an order asked for is the order
        setTrendlines(&c, QStringLiteral("poly"), 3);
        QCOMPARE(c.plots[0].series[0].trendlines[0].order, 3);
        QCOMPARE(c.plots[0].series[0].trendlines[0].period, 7);
    }

    // The trendline element: every series has one of the kind chosen, it keeps what it shows when the kind changes, it is written and read, and "none" takes it away.
    void trendlinesGoOnAndOff()
    {
        Chart c = columns(ChartKind::Line);
        QVERIFY(elementsOf(c).trendline.isEmpty());
        setTrendlines(&c, QStringLiteral("linear"));
        setTrendlineDisplay(&c, true, true);
        for (const Chart &x : {c, roundTrip(c)}) {
            const ChartElements e = elementsOf(x);
            QCOMPARE(e.trendline, QStringLiteral("linear"));
            QVERIFY(e.trendlineEquation && e.trendlineRSquared);
            for (const ChartSeries &s : seriesOf(x)) QCOMPARE(int(s.trendlines.size()), 1);
        }
        setTrendlines(&c, QStringLiteral("poly"), 4);
        const Chart back = roundTrip(c);
        QCOMPARE(elementsOf(back).trendline, QStringLiteral("poly"));
        QCOMPARE(seriesOf(back)[0].trendlines[0].order, 4);
        QVERIFY(elementsOf(back).trendlineEquation);   // (kept across the change of kind)
        QCOMPARE(seriesOf(back)[0].trendlines[0].line.color, seriesOf(back)[0].color);
        setTrendlines(&c, QStringLiteral("movingAvg"), 2, 3);
        QCOMPARE(roundTrip(c).plots[0].series[0].trendlines[0].period, 3);
        // a pie has no trendlines
        Chart pie = columns(ChartKind::Pie);
        setTrendlines(&pie, QStringLiteral("linear"));
        QVERIFY(pie.plots[0].series[0].trendlines.isEmpty());
        setTrendlines(&c, QString());
        QVERIFY(elementsOf(roundTrip(c)).trendline.isEmpty());
        // columns and a scatter take them too
        Chart bars = columns();
        setTrendlines(&bars, QStringLiteral("exp"));
        QCOMPARE(elementsOf(roundTrip(bars)).trendline, QStringLiteral("exp"));
        Chart sc = columns(ChartKind::Scatter);
        setTrendlines(&sc, QStringLiteral("power"));
        QCOMPARE(elementsOf(roundTrip(sc)).trendline, QStringLiteral("power"));
    }

    // The legend over the plot, and the elements of the second pair of axes (a series against them).
    void theLegendOverTheChartAndTheSecondAxes()
    {
        Chart c = columns();
        setLegend(&c, QStringLiteral("r"), true);
        QVERIFY(elementsOf(c).legendOverlay);
        QVERIFY(elementsOf(roundTrip(c)).legendOverlay);
        setLegend(&c, QStringLiteral("r"));
        QVERIFY(!elementsOf(roundTrip(c)).legendOverlay);
        // no second axes on a plain chart
        ChartElements e = elementsOf(c);
        QVERIFY(!e.hasSecondaryHorizontal && !e.hasSecondaryVertical);
        const Chart combo = roundTrip(withKind(c, ChartKind::Combo, defaultTheme()));
        e = elementsOf(combo);
        QVERIFY(e.hasSecondaryVertical && e.hasSecondaryHorizontal);
        QVERIFY(!e.axisSecondaryHorizontal);   // (a second horizontal axis is not shown: the usual of such a chart)
        Chart d = combo;
        setAxisTitle(&d, false, true, true);
        setAxisShown(&d, true, true, true);
        e = elementsOf(roundTrip(d));
        QVERIFY(e.axisTitleSecondaryVertical && !e.axisTitleVertical);
        QVERIFY(e.axisSecondaryHorizontal && e.axisSecondaryVertical);
        setAxisShown(&d, false, false, true);
        QVERIFY(!elementsOf(roundTrip(d)).axisSecondaryVertical);
        QVERIFY(elementsOf(roundTrip(d)).axisVertical);   // (the first pair is as it was)
    }

    void eachSeriesHasItsOwnKind()
    {
        Chart c = makeChart(ChartKind::Column, planOf(QStringLiteral("A1:D4"), ChartKind::Column), defaultTheme());
        QCOMPARE(seriesCount(c), 3);
        const QVector<SeriesKind> want = {{ChartKind::Area, false}, {ChartKind::ColumnStacked, false}, {ChartKind::Line, true}};
        const Chart k = roundTrip(withSeriesKinds(c, want, defaultTheme()));
        QCOMPARE(k.plots.size(), 3);
        // areas first, then bars, then the line against the second axes
        QCOMPARE(int(k.plots[0].type), int(ChartType::Area));
        QCOMPARE(int(k.plots[1].type), int(ChartType::Bar));
        QCOMPARE(int(k.plots[2].type), int(ChartType::Line));
        const QVector<SeriesKind> back = seriesKindsOf(k);
        QCOMPARE(back.size(), 3);
        QVERIFY(back[2].secondary && !back[0].secondary && !back[1].secondary);
        // a pie holds only pie; bars across with other kinds are columns
        const Chart pie = withSeriesKinds(c, {{ChartKind::Pie, false}, {ChartKind::Column, false}, {ChartKind::Line, false}}, defaultTheme());
        QCOMPARE(int(pie.plots[0].type), int(ChartType::Pie));
        QCOMPARE(pie.plots.size(), 1);
        const Chart mixed = withSeriesKinds(c, {{ChartKind::Bar, false}, {ChartKind::Line, false}, {ChartKind::Line, false}}, defaultTheme());
        QVERIFY(!mixed.plots[0].horizontal);
        // fewer kinds than series: the last goes on
        const Chart few = withSeriesKinds(c, {{ChartKind::Line, false}}, defaultTheme());
        QCOMPARE(int(few.plots[0].type), int(ChartType::Line));
        QCOMPARE(few.plots[0].series.size(), 3);
    }

    // Series added, taken away, and put in another order, each keeping the chart's look.
    void seriesAreAddedRemovedAndMoved()
    {
        Chart c = columns();
        c.title = ChartTitle();
        setChartTitle(&c, TitleMode::Above);
        QVector<ChartSeries> s = seriesOf(c);
        QCOMPARE(s.size(), 2);
        ChartSeries extra = s[0];
        extra.nameRef = QStringLiteral("Sheet1!$D$1");
        extra.valueRef = QStringLiteral("Sheet1!$D$2:$D$4");
        extra.name = QStringLiteral("Extra");
        s << extra;
        Chart more = withSeries(c, s, seriesKindsOf(c), defaultTheme());
        QCOMPARE(seriesCount(more), 3);
        QCOMPARE(seriesOf(more)[2].valueRef, QStringLiteral("Sheet1!$D$2:$D$4"));
        QVERIFY(more.title.present);
        // taken away
        s.removeAt(0);
        Chart fewer = roundTrip(withSeries(more, s, seriesKindsOf(more), defaultTheme()));
        QCOMPARE(seriesCount(fewer), 2);
        QCOMPARE(seriesOf(fewer)[0].valueRef, QStringLiteral("Sheet1!$C$2:$C$4"));
        // another order
        std::swap(s[0], s[1]);
        const Chart swapped = roundTrip(withSeries(fewer, s, seriesKindsOf(fewer), defaultTheme()));
        QCOMPARE(seriesOf(swapped)[0].valueRef, QStringLiteral("Sheet1!$D$2:$D$4"));
        // none left: an empty chart that can still be written
        const Chart none = withSeries(fewer, {}, {}, defaultTheme());
        QCOMPARE(seriesCount(none), 0);
        QVERIFY(!writeChartPart(none).isEmpty());
    }

    // Rows and columns switched: the series that ran down columns run along rows, and back again to where they were.
    void rowsAndColumnsAreSwitched()
    {
        Chart c = columns();   // A1:C4: two series down columns B and C, names above, categories A2:A4
        QVERIFY(seriesRunDownColumns(c));
        bool ok = false;
        const Chart rows = switchedRowsColumns(c, defaultTheme(), &ok);
        QVERIFY(ok);
        QVERIFY(!seriesRunDownColumns(rows));
        QCOMPARE(seriesCount(rows), 3);   // the three rows
        QVector<ChartSeries> s = seriesOf(rows);
        QCOMPARE(s[0].nameRef, QStringLiteral("Sheet1!$A$2"));
        QCOMPARE(s[0].valueRef, QStringLiteral("Sheet1!$B$2:$C$2"));
        QCOMPARE(s[0].categoryRef, QStringLiteral("Sheet1!$B$1:$C$1"));
        QCOMPARE(s[2].valueRef, QStringLiteral("Sheet1!$B$4:$C$4"));
        // and back
        const Chart again = switchedRowsColumns(rows, defaultTheme(), &ok);
        QVERIFY(ok);
        s = seriesOf(again);
        QCOMPARE(s.size(), 2);
        QCOMPARE(s[0].nameRef, QStringLiteral("Sheet1!$B$1"));
        QCOMPARE(s[0].categoryRef, QStringLiteral("Sheet1!$A$2:$A$4"));
        QCOMPARE(s[1].valueRef, QStringLiteral("Sheet1!$C$2:$C$4"));
        QCOMPARE(chartDataRange(c), QStringLiteral("Sheet1!$A$1:$C$4"));
        QCOMPARE(chartDataRange(rows), QStringLiteral("Sheet1!$A$1:$C$4"));
        // series that read no block that way: refused
        Chart odd = columns();
        odd.plots[0].series[0].valueRef = QStringLiteral("Sheet1!$B$2:$C$4");
        switchedRowsColumns(odd, defaultTheme(), &ok);
        QVERIFY(!ok);
        Chart other = columns();
        other.plots[0].series[1].valueRef = QStringLiteral("Sheet2!$C$2:$C$4");
        switchedRowsColumns(other, defaultTheme(), &ok);
        QVERIFY(!ok);
        QVERIFY(chartDataRange(other).isEmpty());
        Chart literal = columns();
        literal.plots[0].series[1].valueRef.clear();
        switchedRowsColumns(literal, defaultTheme(), &ok);
        QVERIFY(!ok);
    }

    // A chart saved as a template file and read again keeps its look; applied to another chart it gives that chart the look and keeps its data.
    void aChartTemplateKeepsTheLookAndNotTheData()
    {
        Chart look = columns(ChartKind::Line);
        setLegend(&look, QStringLiteral("t"));
        setChartTitleText(&look, QStringLiteral("Template title"));
        look.plots[0].series[0].color = QColor("#FF0000");
        look.plots[0].series[0].colorFromFile = true;
        const QByteArray file = chartTemplateFile(look);
        QVERIFY(!file.isEmpty());
        Chart back;
        QString why;
        QVERIFY2(readChartTemplate(file, &back, &why), qPrintable(why));
        QCOMPARE(int(back.type()), int(ChartType::Line));
        QCOMPARE(elementsOf(back).legendPosition, QStringLiteral("t"));
        QVERIFY(!readChartTemplate(QByteArray("not a package"), &back, &why));
        QVERIFY(!why.isEmpty());
        // one series of the target against two of the look
        Chart target = columns(ChartKind::Column);
        setChartTitleText(&target, QStringLiteral("Mine"));
        const QString valueRef = target.plots[0].series[0].valueRef;
        target.plots[0].series.removeLast();
        const Chart made = chartWithLook(target, back);
        QCOMPARE(int(made.type()), int(ChartType::Line));
        QCOMPARE(elementsOf(made).legendPosition, QStringLiteral("t"));
        QCOMPARE(made.plots[0].series.size(), 1);
        QCOMPARE(made.plots[0].series[0].valueRef, valueRef);
        QCOMPARE(made.plots[0].series[0].values, target.plots[0].series[0].values);
        QCOMPARE(made.plots[0].series[0].color, QColor("#FF0000"));
        QCOMPARE(made.title.text, QStringLiteral("Mine"));
        // three series against the look's two: the third has the look of the second
        Chart wide = columns(ChartKind::Column);
        ChartSeries extra = wide.plots[0].series[1];
        extra.name = QStringLiteral("Extra");
        extra.valueRef = QStringLiteral("Sheet1!$D$2:$D$4");
        wide.plots[0].series.push_back(extra);
        const Chart three = chartWithLook(wide, back);
        int count = 0;
        for (const ChartPlot &p : three.plots) count += int(p.series.size());
        QCOMPARE(count, 3);
        QCOMPARE(three.plots[0].series.last().valueRef, QStringLiteral("Sheet1!$D$2:$D$4"));
        // and the result is a chart that is written and read again
        QCOMPARE(roundTrip(three).plots[0].series.size(), three.plots[0].series.size());
    }
};

JP_TEST_CLASS(SheetChartEditTests)
#include "tst_sheet_chartedit.moc"
