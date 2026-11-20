// Tests of changing a chart's model (libs/office/sheet/chartedit.h): its elements on and off, colors and looks, kinds, series, and the rows and columns switched. Each change is also written and read
// back, since a change that is not saved is not made.
#include "drawinghelpers.h"

#include "core/fonts.h"
#include "office/chart_write.h"
#include "office/theme.h"
#include "sheet/chartedit.h"
#include "sheet/chartembed.h"
#include "../chartembedhelpers.h"

#include <QRandomGenerator>

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

// A chart of the cells A1:C4 that keeps the workbook it reads beside it.
Chart bookChart()
{
    Chart c = columns();
    c.externalDataId = QStringLiteral("rId1");
    return c;
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

    // Error bars go on every series of a bar, line, or scatter chart by kind, come off again, and are kept through writing.
    void errorBarsGoOnAndOff()
    {
        Chart c = columns();
        QVERIFY(errorBarsKind(c).isEmpty());
        setErrorBars(&c, QStringLiteral("percentage"), 7);
        QCOMPARE(errorBarsKind(c), QStringLiteral("percentage"));
        for (const ChartSeries &s : seriesOf(c)) {
            QCOMPARE(s.errorBars.size(), 1);
            QCOMPARE(s.errorBars[0].value, 7.0);
        }
        const Chart back = roundTrip(c);
        QCOMPARE(errorBarsKind(back), QStringLiteral("percentage"));
        QCOMPARE(seriesOf(back)[1].errorBars[0].value, 7.0);
        setErrorBars(&c, QStringLiteral("stdErr"));
        QCOMPARE(errorBarsKind(c), QStringLiteral("stdErr"));
        setErrorBars(&c, QString());
        QVERIFY(errorBarsKind(c).isEmpty());
        Chart pie = columns(ChartKind::Pie);
        setErrorBars(&pie, QStringLiteral("fixedVal"));
        QVERIFY(errorBarsKind(pie).isEmpty());
    }

    // An area chart takes error bars too: they are set on its series, drawn, and kept through writing.
    void areaChartsTakeErrorBars()
    {
        Chart area = columns(ChartKind::Area);
        const QByteArray plain = chartSvg(area, QSizeF(480, 288));
        setErrorBars(&area, QStringLiteral("fixedVal"), 2);
        QCOMPARE(errorBarsKind(area), QStringLiteral("fixedVal"));
        QVERIFY(chartSvg(area, QSizeF(480, 288)) != plain);
        QCOMPARE(errorBarsKind(roundTrip(area)), QStringLiteral("fixedVal"));
    }

    // Custom error bars name the cells for each side, are kept through writing, and a one-sided pair makes a one-sided bar.
    void customErrorBarsNameTheirCells()
    {
        Chart c = columns();
        QVERIFY(customErrorBarRefs(c).isEmpty());
        setCustomErrorBars(&c, QStringLiteral("Sheet1!$E$2:$E$4"), QString());
        QCOMPARE(customErrorBarRefs(c), (QStringList{"Sheet1!$E$2:$E$4", QString()}));
        QCOMPARE(seriesOf(c)[0].errorBars[0].barType, QStringLiteral("plus"));
        const Chart back = roundTrip(c);
        QCOMPARE(errorBarsKind(back), QStringLiteral("cust"));
        QCOMPARE(customErrorBarRefs(back).value(0), QStringLiteral("Sheet1!$E$2:$E$4"));
        setErrorBars(&c, QStringLiteral("fixedVal"));
        QVERIFY(customErrorBarRefs(c).isEmpty());
    }

    // A scatter chart's x bars stay when the vertical ones are set, and the vertical bars keep their line and end caps when set again.
    void settingBarsKeepsTheOtherDirectionAndTheLook()
    {
        Chart c = columns(ChartKind::Scatter);
        for (ChartSeries &s : c.plots[0].series) {
            ChartErrorBars x;
            x.direction = QStringLiteral("x");
            x.valueType = QStringLiteral("fixedVal");
            x.value = 3;
            x.line.set = true;
            x.line.color = QColor("#00FF00");
            ChartErrorBars y;
            y.direction = QStringLiteral("y");
            y.valueType = QStringLiteral("percentage");
            y.noEndCap = true;
            y.line.set = true;
            y.line.color = QColor("#FF0000");
            s.errorBars = QVector<ChartErrorBars>{x, y};
        }
        setCustomErrorBars(&c, QStringLiteral("Sheet1!$E$2:$E$4"), QString());
        for (const ChartSeries &s : c.plots[0].series) {
            QCOMPARE(s.errorBars.size(), 2);
            QCOMPARE(s.errorBars[0].direction, QStringLiteral("x"));
            QCOMPARE(s.errorBars[0].value, 3.0);
            QCOMPARE(s.errorBars[0].line.color.name(), QStringLiteral("#00ff00"));
            QCOMPARE(s.errorBars[1].valueType, QStringLiteral("cust"));
            QVERIFY(s.errorBars[1].noEndCap);
            QCOMPARE(s.errorBars[1].line.color.name(), QStringLiteral("#ff0000"));
        }
        setErrorBars(&c, QStringLiteral("fixedVal"), 2);
        QCOMPARE(c.plots[0].series[0].errorBars.size(), 2);
        QCOMPARE(c.plots[0].series[0].errorBars[1].value, 2.0);
        QVERIFY(c.plots[0].series[0].errorBars[1].noEndCap);
    }

    // A percent-stacked area is drawn in percent, so it does not take error bars (in the units of the values): none are offered, set, or drawn; a stacked area does take them.
    void percentStackedAreasTakeNoErrorBars()
    {
        Chart area = columns(ChartKind::Area);
        QVERIFY(errorBarsApply(area));
        area.plots[0].grouping = ChartGrouping::PercentStacked;
        QVERIFY(!errorBarsApply(area));
        const QByteArray plain = chartSvg(area, QSizeF(480, 288));
        setErrorBars(&area, QStringLiteral("fixedVal"), 2);
        QVERIFY(errorBarsKind(area).isEmpty());
        for (ChartSeries &s : area.plots[0].series) {
            ChartErrorBars b;
            b.valueType = QStringLiteral("fixedVal");
            s.errorBars << b;
        }
        QCOMPARE(chartSvg(area, QSizeF(480, 288)), plain);   // (a file's bars are kept but not drawn out of scale)
        area.plots[0].grouping = ChartGrouping::Stacked;
        QVERIFY(errorBarsApply(area));
    }

    // A chart too small for the strips of field buttons draws without them and without a negative plot; a button with no name is not drawn.
    void tinyPivotChartsAndBlankNamesDrawNoButtons()
    {
        Chart c = columns();
        c.pivotName = QStringLiteral("[Book1.xlsx]Sheet1!PivotTable1");
        setPivotButtons(&c, PivotButton::All, true);
        c.pivotButtons.categoryNames = QStringList{QString(), QStringLiteral("Region")};
        c.pivotButtons.filterNames = QStringList{QStringLiteral("Year")};
        const QByteArray small = chartSvg(c, QSizeF(120, 60));
        QVERIFY(!small.contains("Region") && !small.contains("Year"));
        QVERIFY(!small.contains("NaN") && !small.contains("-nan"));
        c.pivotButtons.categoryNames = QStringList{QString()};
        c.pivotButtons.filterNames.clear();
        const QByteArray blank = chartSvg(c, QSizeF(480, 288));
        c.pivotButtons.categoryNames.clear();
        QCOMPARE(blank, chartSvg(c, QSizeF(480, 288)));
    }

    // The field buttons of a pivot chart go on and off by kind and are kept through writing; a chart that follows no table has none.
    void pivotChartFieldButtonsGoOnAndOff()
    {
        Chart c = columns();
        setPivotButtons(&c, PivotButton::All, true);
        QVERIFY(!pivotButtonsOn(c, PivotButton::All));   // (no table: no buttons)
        c.pivotName = QStringLiteral("[Book1.xlsx]Sheet1!PivotTable1");
        setPivotButtons(&c, PivotButton::All, true);
        QVERIFY(pivotButtonsOn(c, PivotButton::All));
        setPivotButtons(&c, PivotButton::Legend, false);
        QVERIFY(!pivotButtonsOn(c, PivotButton::Legend) && pivotButtonsOn(c, PivotButton::Axis) && !pivotButtonsOn(c, PivotButton::All));
        const Chart back = roundTrip(c);
        QVERIFY(!pivotButtonsOn(back, PivotButton::Legend) && pivotButtonsOn(back, PivotButton::Filter) && pivotButtonsOn(back, PivotButton::Value));
        // the buttons are drawn with the fields' names, and take room of their own
        const QByteArray plain = chartSvg(c, QSizeF(480, 288));
        c.pivotButtons.categoryNames = QStringList{QStringLiteral("Region")};
        c.pivotButtons.filterNames = QStringList{QStringLiteral("Year")};
        const QByteArray drawn = chartSvg(c, QSizeF(480, 288));
        QVERIFY(drawn != plain && drawn.contains("Region") && drawn.contains("Year"));
        setPivotButtons(&c, PivotButton::All, false);
        QVERIFY(!chartSvg(c, QSizeF(480, 288)).contains("Region"));
    }

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

    // The data table is offered for columns, lines, and areas, comes on with the lines and outline (no keys), has each of its four options set alone, and is kept through writing.
    void theDataTableGoesOnAndOffWithItsOptions()
    {
        Chart c = columns();
        QVERIFY(dataTableApplies(c) && !dataTableOn(c, DataTablePart::Table));
        setDataTable(&c, DataTablePart::Keys, true);   // (nothing to set while the table is off)
        QVERIFY(!dataTableOn(c, DataTablePart::Table) && !dataTableOn(c, DataTablePart::Keys));
        setDataTable(&c, DataTablePart::Table, true);
        QVERIFY(dataTableOn(c, DataTablePart::Table) && dataTableOn(c, DataTablePart::RowLines) && dataTableOn(c, DataTablePart::ColumnLines) && dataTableOn(c, DataTablePart::Outline));
        QVERIFY(!dataTableOn(c, DataTablePart::Keys));
        setDataTable(&c, DataTablePart::Keys, true);
        setDataTable(&c, DataTablePart::ColumnLines, false);
        const Chart back = roundTrip(c);
        QVERIFY(dataTableOn(back, DataTablePart::Keys) && !dataTableOn(back, DataTablePart::ColumnLines) && dataTableOn(back, DataTablePart::RowLines) && dataTableOn(back, DataTablePart::Outline));
        setDataTable(&c, DataTablePart::Table, false);
        QVERIFY(!dataTableOn(c, DataTablePart::Table) && !roundTrip(c).dataTable.present);
        QVERIFY(dataTableApplies(columns(ChartKind::Line)) && dataTableApplies(columns(ChartKind::Area)));
        // a change of kind to one that has no table leaves it out of the file
        Chart withTable = columns();
        setDataTable(&withTable, DataTablePart::Table, true);
        QVERIFY(writeChartPart(withTable).contains("c:dTable"));
        for (ChartKind kind : {ChartKind::Pie, ChartKind::Bar, ChartKind::Scatter}) {
            const Chart changed = withKind(withTable, kind, defaultTheme());
            QVERIFY(!dataTableApplies(changed));
            QVERIFY(!writeChartPart(changed).contains("c:dTable"));
        }
        QVERIFY(!dataTableApplies(columns(ChartKind::Pie)) && !dataTableApplies(columns(ChartKind::Bar)) && !dataTableApplies(columns(ChartKind::Scatter)));
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

    // The looks of the chart area, the plot area, and a series are set one part at a time, written, and read again.
    void thePartsOfAChartTakeALookEachAndItIsWritten()
    {
        Chart c = columns();
        QVERIFY(!partLookOf(c, ChartPart::Area).fill.isValid() || partLookOf(c, ChartPart::Area).fill != QColor(Qt::yellow));
        ChartPartLook area;
        area.fill = QColor(Qt::yellow);
        setPartLook(&c, ChartPart::Area, 0, area);
        ChartPartLook plot;
        plot.fill = QColor(0xEE, 0xEE, 0xFF);
        plot.line.set = true;
        plot.line.color = QColor(Qt::blue);
        plot.line.width = 2;
        setPartLook(&c, ChartPart::Plot, 0, plot);
        ChartPartLook series = partLookOf(c, ChartPart::Series, 1);
        series.fill = QColor(0x12, 0x34, 0x56);
        series.line.set = true;
        series.line.color = QColor(Qt::red);
        series.line.width = 3;
        setPartLook(&c, ChartPart::Series, 1, series);
        const Chart back = roundTrip(c);
        QCOMPARE(partLookOf(back, ChartPart::Area).fill.name(), QStringLiteral("#ffff00"));
        QCOMPARE(partLookOf(back, ChartPart::Plot).fill.name(), QStringLiteral("#eeeeff"));
        QCOMPARE(partLookOf(back, ChartPart::Plot).line.width, 2.0);
        QCOMPARE(partLookOf(back, ChartPart::Series, 1).fill.name(), QStringLiteral("#123456"));
        QCOMPARE(partLookOf(back, ChartPart::Series, 1).line.color.name(), QStringLiteral("#ff0000"));
        // the other series is as it was, and a place with no series gives an empty look
        QCOMPARE(partLookOf(back, ChartPart::Series, 0).fill.name(), partLookOf(columns(), ChartPart::Series, 0).fill.name());
        QVERIFY(!partLookOf(back, ChartPart::Series, 9).fill.isValid());
        // no fill takes the fill away, for the page and for the plot (which is written as no fill, not as a fill that was never set)
        ChartPartLook none;
        none.noFill = true;
        setPartLook(&c, ChartPart::Area, 0, none);
        QVERIFY(roundTrip(c).backgroundNone);
        setPartLook(&c, ChartPart::Plot, 0, none);
        const Chart noPlot = roundTrip(c);
        QVERIFY(noPlot.plotNone && !noPlot.plotFill.isValid());
        QVERIFY(partLookOf(noPlot, ChartPart::Plot).noFill);
        plot.fill = QColor(0xAA, 0xBB, 0xCC);
        setPartLook(&c, ChartPart::Plot, 0, plot);
        QVERIFY(!roundTrip(c).plotNone);
    }

    // Reset to Match Style takes away what was set on one part and leaves the others; what it leaves is what the style gives, for a style of accents in turn and for one of shades.
    void aPartIsResetToMatchItsStyle()
    {
        for (int style : {2, 3}) {
            Chart c = columns();
            c.style = style;
            c = roundTrip(c);
            QCOMPARE(c.styleBase.isValid(), style == 3);
            const QColor first = chartStyleSeriesColor(c, 0, 2), second = chartStyleSeriesColor(c, 1, 2);
            QVERIFY(first != second);
            if (style == 3) QVERIFY(second != c.palette[1]);
            ChartPartLook area;
            area.fill = QColor(Qt::yellow);
            area.line.set = true;
            area.line.width = 3;
            setPartLook(&c, ChartPart::Area, 0, area);
            setPartLook(&c, ChartPart::Plot, 0, area);
            ChartPartLook look = partLookOf(c, ChartPart::Series, 0);
            look.fill = QColor(0x12, 0x34, 0x56);
            look.line.set = true;
            look.line.width = 4;
            look.smooth = true;
            look.marker = ChartMarker::Star;
            setPartLook(&c, ChartPart::Series, 0, look);
            look = partLookOf(c, ChartPart::Series, 1);
            look.noFill = true;
            setPartLook(&c, ChartPart::Series, 1, look);
            // the series alone: its color is the style's, its stroke, line, and marks are gone, and nothing else changed
            resetPartToStyle(&c, ChartPart::Series, 0);
            const ChartPartLook now = partLookOf(c, ChartPart::Series, 0);
            QCOMPARE(now.fill, first);
            QVERIFY(!now.noFill && !now.line.set && !now.smooth && now.marker == ChartMarker::Auto);
            QVERIFY(partLookOf(c, ChartPart::Series, 1).noFill);
            QCOMPARE(partLookOf(c, ChartPart::Area).fill, QColor(Qt::yellow));
            // the other series, then the page and the plot
            resetPartToStyle(&c, ChartPart::Series, 1);
            QCOMPARE(partLookOf(c, ChartPart::Series, 1).fill, second);
            QVERIFY(!partLookOf(c, ChartPart::Series, 1).noFill);
            resetPartToStyle(&c, ChartPart::Area, 0);
            QVERIFY(!partLookOf(c, ChartPart::Area).fill.isValid() && !partLookOf(c, ChartPart::Area).line.set);
            QCOMPARE(partLookOf(c, ChartPart::Plot).fill, QColor(Qt::yellow));
            resetPartToStyle(&c, ChartPart::Plot, 0);
            QVERIFY(!partLookOf(c, ChartPart::Plot).fill.isValid() && !partLookOf(c, ChartPart::Plot).line.set);
            // a series the chart does not have changes nothing, and a reset of what matches the style is no change
            const QByteArray written = writeChartPart(c);
            resetPartToStyle(&c, ChartPart::Series, 9);
            resetPartToStyle(&c, ChartPart::Series, 0);
            QCOMPARE(writeChartPart(c), written);
            // and what was reset is what the file keeps
            const Chart back = roundTrip(c);
            QCOMPARE(partLookOf(back, ChartPart::Series, 0).fill.name(), chartStyleSeriesColor(back, 0, 2).name());
            QVERIFY(!partLookOf(back, ChartPart::Area).fill.isValid());
        }
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

    // ---- a chart that keeps its numbers in a workbook beside it (c:externalData)
    // The grid is what the cells the references name hold.
    void theCellsOfAnEmbeddedWorkbookAreTheGridOfTheChartsData()
    {
        const Chart chart = bookChart();
        ChartData grid;
        QCOMPARE(embeddedChartProblem(chart, jp::testing::salesBook(), &grid), QString());
        QCOMPARE(grid.categories, QStringList({"Q1", "Q2", "Q3"}));
        QCOMPARE(grid.seriesNames, QStringList({"Sales", "Costs"}));
        QCOMPARE(grid.values, (QVector<QVector<double>>{{1, 2, 3}, {4, 5, 6}}));
        // The same through the one rule of whether the data is editable.
        QCOMPARE(chartDataProblem(chart, nullptr, jp::testing::salesBook()), QString());
        QVERIFY(chartDataProblem(chart).contains(QStringLiteral("workbook")));
        QVERIFY(chartInfoOf(chart, writeChartPart(chart), jp::testing::salesBook()).dataEditable);
        QCOMPARE(chartInfoOf(chart, writeChartPart(chart), jp::testing::salesBook()).data.values, grid.values);
        QVERIFY(!chartInfoOf(chart, writeChartPart(chart)).dataEditable);
    }

    // A reference the code can't follow keeps the chart read-only, with a reason that says the workbook.
    void aReferenceThatCantBeFollowedKeepsTheNumbersReadOnly()
    {
        using namespace jp::testing;
        auto problem = [](const Chart &c, const QByteArray &book) { return embeddedChartProblem(c, book); };
        const QByteArray book = salesBook();
        QVERIFY(problem(bookChart(), book).isEmpty());
        // another sheet that is not there, an address in another file, a list of blocks, a whole column, a name
        for (const QString &ref : {QStringLiteral("Other!$B$2:$B$4"), QStringLiteral("[1]Sheet1!$B$2:$B$4"), QStringLiteral("(Sheet1!$B$2:$B$3,Sheet1!$B$4)"), QStringLiteral("Sheet1!$B:$B"), QStringLiteral("Sales")}) {
            Chart c = bookChart();
            c.plots[0].series[0].valueRef = ref;
            QVERIFY2(problem(c, book).contains(QStringLiteral("workbook")), qPrintable(ref));
        }
        // a block that is not a line, one that is longer than the other series', and one past the cells of the sheet
        for (const QString &ref : {QStringLiteral("Sheet1!$B$2:$C$4"), QStringLiteral("Sheet1!$B$2:$B$3"), QStringLiteral("Sheet1!$B$2:$B$9"), QStringLiteral("Sheet1!$H$2:$H$4")}) {
            Chart c = bookChart();
            c.plots[0].series[0].valueRef = ref;
            QVERIFY2(problem(c, book).contains(QStringLiteral("workbook")), qPrintable(ref));
        }
        // two series on the same numbers, a name that is one of the numbers, a scatter's x numbers, categories that differ
        Chart same = bookChart();
        same.plots[0].series[1].valueRef = same.plots[0].series[0].valueRef;
        QVERIFY(!problem(same, book).isEmpty());
        Chart named = bookChart();
        named.plots[0].series[0].nameRef = QStringLiteral("Sheet1!$B$2");
        QVERIFY(!problem(named, book).isEmpty());
        Chart scatter = bookChart();
        scatter.plots[0].series[0].xRef = QStringLiteral("Sheet1!$A$2:$A$4");
        QVERIFY(!problem(scatter, book).isEmpty());
        Chart cats = bookChart();
        cats.plots[0].series[1].categoryRef = QStringLiteral("Sheet1!$C$2:$C$4");
        QVERIFY(!problem(cats, book).isEmpty());
        // no workbook, and one that is not
        QVERIFY(problem(bookChart(), QByteArray()).contains(QStringLiteral("workbook")));
        QVERIFY(problem(bookChart(), QByteArray("PK not a workbook")).contains(QStringLiteral("workbook")));
        // cells that hold a formula, words where numbers are, nothing where numbers are
        const QString names = bookRow(1, {bookText(QStringLiteral("B1"), QStringLiteral("Sales")), bookText(QStringLiteral("C1"), QStringLiteral("Costs"))});
        const QString cats3 = bookRow(2, {bookText(QStringLiteral("A2"), QStringLiteral("Q1")), bookNumber(QStringLiteral("B2"), 1), bookNumber(QStringLiteral("C2"), 4)});
        const QString rest = bookRow(4, {bookText(QStringLiteral("A4"), QStringLiteral("Q3")), bookNumber(QStringLiteral("B4"), 3), bookNumber(QStringLiteral("C4"), 6)});
        const QString formula = bookRow(3, {bookText(QStringLiteral("A3"), QStringLiteral("Q2")), QStringLiteral("<c r=\"B3\"><f>1+1</f><v>2</v></c>"), bookNumber(QStringLiteral("C3"), 5)});
        const QString words = bookRow(3, {bookText(QStringLiteral("A3"), QStringLiteral("Q2")), bookText(QStringLiteral("B3"), QStringLiteral("two")), bookNumber(QStringLiteral("C3"), 5)});
        const QString blank = bookRow(3, {bookText(QStringLiteral("A3"), QStringLiteral("Q2")), bookNumber(QStringLiteral("C3"), 5)});
        QVERIFY(problem(bookChart(), bookOf(names + cats3 + bookRow(3, {bookText(QStringLiteral("A3"), QStringLiteral("Q2")), bookNumber(QStringLiteral("B3"), 2), bookNumber(QStringLiteral("C3"), 5)}) + rest)).isEmpty());
        for (const QString &row : {formula, words, blank}) QVERIFY2(problem(bookChart(), bookOf(names + cats3 + row + rest)).contains(QStringLiteral("workbook")), qPrintable(row));
    }

    // The edit changes the cells that changed, in the workbook and in what the chart stores; nothing else of the workbook.
    void aChangedGridIsWrittenToTheCellsAndToTheChartsStoredValues()
    {
        using namespace jp::testing;
        const QByteArray book = salesBook();
        Chart chart = bookChart();
        ChartData grid;
        QVERIFY(embeddedChartProblem(chart, book, &grid).isEmpty());
        grid.values[0][1] = 20;
        grid.seriesNames[1] = QStringLiteral("Expenses");
        grid.categories[2] = QStringLiteral("Q3 late");
        QByteArray after;
        QVERIFY(embeddedChartSetData(chart, book, grid, &after));
        // the workbook, opened as another program would: the cells are new, the others as they were, and the parts the edit does not name are the same
        ChartData again;
        QVERIFY(embeddedChartProblem(chart, after, &again).isEmpty());
        QCOMPARE(again.values, grid.values);
        QCOMPARE(again.seriesNames, grid.seriesNames);
        QCOMPARE(again.categories, grid.categories);
        Package was, now;
        QVERIFY(was.read(book));
        QVERIFY(now.read(after));
        QCOMPARE(now.partNames(), was.partNames());
        QCOMPARE(now.part(QStringLiteral("/docProps/custom-note.xml")), was.part(QStringLiteral("/docProps/custom-note.xml")));
        QVERIFY(now.part(QStringLiteral("/xl/worksheets/sheet1.xml")) != was.part(QStringLiteral("/xl/worksheets/sheet1.xml")));
        // what the chart stores follows (written and read again)
        const Chart back = roundTrip(chart);
        QCOMPARE(back.plots[0].series[0].values, (QVector<double>{1, 20, 3}));
        QCOMPARE(back.plots[0].series[1].name, QStringLiteral("Expenses"));
        QCOMPARE(back.plots[0].series[0].categories, QStringList({"Q1", "Q2", "Q3 late"}));
        QCOMPARE(back.plots[0].series[0].valueRef, chart.plots[0].series[0].valueRef);
        QCOMPARE(back.externalDataId, QStringLiteral("rId1"));
        // a grid of another shape, and one that changes nothing, change nothing
        ChartData wider = grid;
        wider.categories << QStringLiteral("Q4");
        for (QVector<double> &v : wider.values) v << 7;
        Chart untouched = chart;
        QVERIFY(!embeddedChartSetData(untouched, after, wider, nullptr));
        QVERIFY(!embeddedChartSetData(untouched, after, again, nullptr));
        QCOMPARE(writeChartPart(untouched), writeChartPart(chart));
    }

    // Numbers as categories (years) stay numbers in their cells and in the chart's numeric cache.
    void aNumberCategoryStaysANumberInItsCell()
    {
        using namespace jp::testing;
        const QByteArray book = bookOf(bookRow(1, {bookText(QStringLiteral("B1"), QStringLiteral("Sales"))}) + bookRow(2, {bookNumber(QStringLiteral("A2"), 2019), bookNumber(QStringLiteral("B2"), 1)}) +
                                       bookRow(3, {bookNumber(QStringLiteral("A3"), 2020), bookNumber(QStringLiteral("B3"), 2)}));
        Chart chart = makeChart(ChartKind::Column, planOf(QStringLiteral("A1:B3"), ChartKind::Column), defaultTheme());
        chart.externalDataId = QStringLiteral("rId1");
        ChartData grid;
        QVERIFY2(embeddedChartProblem(chart, book, &grid).isEmpty(), qPrintable(embeddedChartProblem(chart, book)));
        QCOMPARE(grid.categories, QStringList({"2019", "2020"}));
        grid.categories[1] = QStringLiteral("2021");
        QByteArray after;
        QVERIFY(embeddedChartSetData(chart, book, grid, &after));
        Package p;
        QVERIFY(p.read(after));
        const QByteArray xml = p.part(QStringLiteral("/xl/worksheets/sheet1.xml"));
        QVERIFY(xml.contains("<v>2021</v>"));
        QVERIFY(!xml.contains("2021</t>"));
    }
    // ---- review fixes
    // The cells of a chart that sits on a richer workbook (two sheets, shared strings, styles, a formula, a defined name) are edited, and all the rest of it is as it was.
    void aRicherWorkbookIsLeftAsItWasButForTheChangedCells()
    {
        using namespace jp::testing;
        const QString rows = bookRow(1, {bookShared(QStringLiteral("B1"), 0), bookShared(QStringLiteral("C1"), 1)}) + bookRow(2, {bookShared(QStringLiteral("A2"), 2), bookStyled(QStringLiteral("B2"), 2, 1), bookNumber(QStringLiteral("C2"), 4)}) +
                             bookRow(3, {bookShared(QStringLiteral("A3"), 3), bookNumber(QStringLiteral("B3"), 2), bookNumber(QStringLiteral("C3"), 5)}) + bookRow(4, {bookShared(QStringLiteral("A4"), 4), bookNumber(QStringLiteral("B4"), 3), bookNumber(QStringLiteral("C4"), 6)});
        const QByteArray book = richBook(rows);
        Chart chart = bookChart();
        ChartData grid;
        QVERIFY2(embeddedChartProblem(chart, book, &grid).isEmpty(), qPrintable(embeddedChartProblem(chart, book)));
        QCOMPARE(grid.categories, QStringList({"Q1", "Q2", "Q3"}));
        grid.values[0][1] = 22;
        grid.categories[0] = QStringLiteral("First");
        QByteArray after;
        QVERIFY(embeddedChartSetData(chart, book, grid, &after));
        Package was, now;
        QVERIFY(was.read(book));
        QVERIFY(now.read(after));
        QCOMPARE(now.partNames(), was.partNames());
        for (const char *part : {"/xl/worksheets/sheet2.xml", "/xl/styles.xml", "/xl/sharedStrings.xml"}) QCOMPARE(now.part(QString::fromLatin1(part)), was.part(QString::fromLatin1(part)));
        const QByteArray names = now.part(QStringLiteral("/xl/workbook.xml"));
        QVERIFY(names.contains("definedName name=\"Total\"") && names.contains("Sheet1!$B$2:$B$4"));
        // (the formula that reads the cells is worked out again when the workbook is opened)
        QVERIFY(names.contains("fullCalcOnLoad=\"1\""));
        QVERIFY(!was.part(QStringLiteral("/xl/workbook.xml")).contains("fullCalcOnLoad"));
        const QByteArray sheet = now.part(QStringLiteral("/xl/worksheets/sheet1.xml"));
        QVERIFY(sheet.contains("22"));
        QVERIFY(sheet.contains("s=\"2\""));   // (the changed cell keeps its cell format)
        ChartData again;
        QVERIFY(embeddedChartProblem(chart, after, &again).isEmpty());
        QCOMPARE(again.values[0], (QVector<double>{1, 22, 3}));
        QCOMPARE(again.categories[0], QStringLiteral("First"));
        QCOMPARE(again.categories[1], QStringLiteral("Q2"));
    }

    // Dates are not plain numbers: a cell in a date's format keeps the chart's numbers read-only, with the reason; a number with a format of its own is fine as a value (its format code stays) but not as a name or a word.
    void aDateFormattedCellKeepsTheNumbersReadOnlyWithTheReason()
    {
        using namespace jp::testing;
        auto rows = [](int catStyle, int valueStyle) {
            return bookRow(1, {bookShared(QStringLiteral("B1"), 0), bookShared(QStringLiteral("C1"), 1)}) + bookRow(2, {bookStyled(QStringLiteral("A2"), catStyle, 45292), bookStyled(QStringLiteral("B2"), valueStyle, 1), bookNumber(QStringLiteral("C2"), 4)}) +
                   bookRow(3, {bookStyled(QStringLiteral("A3"), catStyle, 45293), bookNumber(QStringLiteral("B3"), 2), bookNumber(QStringLiteral("C3"), 5)}) + bookRow(4, {bookStyled(QStringLiteral("A4"), catStyle, 45294), bookNumber(QStringLiteral("B4"), 3), bookNumber(QStringLiteral("C4"), 6)});
        };
        // dates as the words under the points, and as numbers
        const QString dateWhy = embeddedChartProblem(bookChart(), richBook(rows(1, 0)));
        QVERIFY2(dateWhy.contains(QStringLiteral("date")), qPrintable(dateWhy));
        QVERIFY(embeddedChartProblem(bookChart(), richBook(rows(0, 1))).contains(QStringLiteral("date")));
        // a number with a format of its own as a word (0.00) is not read as a plain one; as a value it is, and the format code the chart stores stays
        QVERIFY(embeddedChartProblem(bookChart(), richBook(rows(2, 0))).contains(QStringLiteral("workbook")));
        Chart chart = bookChart();
        chart.plots[0].series[0].format = QStringLiteral("0.00");
        ChartData grid;
        const QByteArray plain = richBook(rows(3, 2));
        QVERIFY2(embeddedChartProblem(chart, plain, &grid).isEmpty(), qPrintable(embeddedChartProblem(chart, plain)));
        grid.values[0][2] = 9;
        QByteArray after;
        QVERIFY(embeddedChartSetData(chart, plain, grid, &after));
        QCOMPARE(roundTrip(chart).plots[0].series[0].format, QStringLiteral("0.00"));
    }

    // Only the cells that were changed are written: long words, 17-digit numbers, and a number as a name are as they were.
    void onlyTheChangedCellsAreWrittenAndNumbersKeepEveryDigit()
    {
        using namespace jp::testing;
        const QString longWords(600, QLatin1Char('w'));
        const QString rows = bookRow(1, {bookNumber(QStringLiteral("B1"), 2020), bookText(QStringLiteral("C1"), QStringLiteral("Costs"))}) +
                             bookRow(2, {bookText(QStringLiteral("A2"), longWords), bookStyled(QStringLiteral("B2"), 0, 0.30000000000000004), bookStyled(QStringLiteral("C2"), 0, 1234567890.1234567)}) +
                             bookRow(3, {bookText(QStringLiteral("A3"), QStringLiteral("Q2")), bookNumber(QStringLiteral("B3"), 2), bookNumber(QStringLiteral("C3"), 5)}) + bookRow(4, {bookText(QStringLiteral("A4"), QStringLiteral("Q3")), bookNumber(QStringLiteral("B4"), 3), bookNumber(QStringLiteral("C4"), 6)});
        const QByteArray book = bookOf(rows);
        Chart chart = bookChart();
        ChartData grid;
        QVERIFY2(embeddedChartProblem(chart, book, &grid).isEmpty(), qPrintable(embeddedChartProblem(chart, book)));
        QCOMPARE(chartNumberText(grid.values[0][0]), QStringLiteral("0.30000000000000004"));
        QCOMPARE(grid.seriesNames[0], QStringLiteral("2020"));
        QCOMPARE(grid.categories[0].size(), 600);
        // another cell changes, and the name that is a number changes as a number
        grid.values[0][2] = 30;
        grid.seriesNames[0] = QStringLiteral("2021");
        QByteArray after;
        QVERIFY(embeddedChartSetData(chart, book, grid, &after));
        Package p;
        QVERIFY(p.read(after));
        const QByteArray sheet = p.part(QStringLiteral("/xl/worksheets/sheet1.xml"));
        QVERIFY(sheet.contains(longWords.toUtf8()));                  // (not cut to what a chart keeps of words)
        QVERIFY(sheet.contains("0.30000000000000004"));
        QVERIFY(sheet.contains("1234567890.1234567"));
        QVERIFY(sheet.contains("<v>2021</v>") && !sheet.contains("2021</t>"));   // (a number as a name stays a number)
    }

    // A number past 1e15 has its own reason.
    void aNumberPastTheOneAChartHoldsSaysSo()
    {
        using namespace jp::testing;
        const QString rows = bookRow(1, {bookText(QStringLiteral("B1"), QStringLiteral("Sales")), bookText(QStringLiteral("C1"), QStringLiteral("Costs"))}) + bookRow(2, {bookText(QStringLiteral("A2"), QStringLiteral("Q1")), bookNumber(QStringLiteral("B2"), 2e15), bookNumber(QStringLiteral("C2"), 4)}) +
                             bookRow(3, {bookText(QStringLiteral("A3"), QStringLiteral("Q2")), bookNumber(QStringLiteral("B3"), 2), bookNumber(QStringLiteral("C3"), 5)}) + bookRow(4, {bookText(QStringLiteral("A4"), QStringLiteral("Q3")), bookNumber(QStringLiteral("B4"), 3), bookNumber(QStringLiteral("C4"), 6)});
        const QString why = embeddedChartProblem(bookChart(), bookOf(rows));
        QVERIFY2(why.contains(QStringLiteral("1,000,000,000,000,000")) && !why.contains(QStringLiteral("series")), qPrintable(why));
    }

    // The limits: a workbook file of more than 16 MB, one that inflates past 64 MB (a bomb of zeros), a sheet of more than a million cells (but not one with a large formatted blank area), a
    // whole-column reference, and more series than a chart made here has. Each fails if its limit is removed.
    void aWorkbookPastTheLimitsIsNotReadAndSaysItIsTooLarge()
    {
        using namespace jp::testing;
        const QString names = bookRow(1, {bookText(QStringLiteral("B1"), QStringLiteral("Sales")), bookText(QStringLiteral("C1"), QStringLiteral("Costs"))});
        const QString rows = names + bookRow(2, {bookText(QStringLiteral("A2"), QStringLiteral("Q1")), bookNumber(QStringLiteral("B2"), 1), bookNumber(QStringLiteral("C2"), 4)}) +
                             bookRow(3, {bookText(QStringLiteral("A3"), QStringLiteral("Q2")), bookNumber(QStringLiteral("B3"), 2), bookNumber(QStringLiteral("C3"), 5)}) + bookRow(4, {bookText(QStringLiteral("A4"), QStringLiteral("Q3")), bookNumber(QStringLiteral("B4"), 3), bookNumber(QStringLiteral("C4"), 6)});
        QVERIFY(embeddedChartProblem(bookChart(), bookOf(rows)).isEmpty());
        // more than 16 MB of file (random bytes do not shrink)
        QByteArray noise(kMostEmbeddedWorkbookBytes, '\0');
        for (qsizetype i = 0; i < noise.size(); i += 4) {
            const quint32 r = QRandomGenerator::global()->generate();
            memcpy(noise.data() + i, &r, 4);
        }
        const QByteArray fat = bookOf(rows, QStringLiteral("Sheet1"), noise);
        QVERIFY(fat.size() > kMostEmbeddedWorkbookBytes);
        QVERIFY2(embeddedChartProblem(bookChart(), fat).contains(QStringLiteral("too large to edit here")), qPrintable(embeddedChartProblem(bookChart(), fat)));
        // a file that is small and inflates to more than 64 MB
        const QByteArray bomb = bookOf(rows, QStringLiteral("Sheet1"), QByteArray(kMostEmbeddedInflatedBytes + (1 << 20), '\0'));
        QVERIFY(bomb.size() < (4 << 20));
        QVERIFY2(embeddedChartProblem(bookChart(), bomb).contains(QStringLiteral("too large to edit here")), qPrintable(embeddedChartProblem(bookChart(), bomb)));
        // a sheet of more than a million cells is too large; one with 150,000 formatted blank cells (more than the old limit) is edited
        auto blanks = [&](int cells) {
            QString out;
            for (int r = 10; cells > 0; ++r) {
                QString row = QStringLiteral("<row r=\"%1\">").arg(r);
                for (int c = 0; c < 100 && cells > 0; ++c, --cells) row += QStringLiteral("<c r=\"%1%2\" s=\"1\"/>").arg(columnName(quint32(c))).arg(r);
                out += row + QStringLiteral("</row>");
            }
            return out;
        };
        const QString big = embeddedChartProblem(bookChart(), bookOf(rows + blanks(int(kMostEmbeddedSheetCells) + 100)));
        QVERIFY2(big.contains(QStringLiteral("too large to edit here")), qPrintable(big));
        QVERIFY2(embeddedChartProblem(bookChart(), bookOf(rows + blanks(150000))).isEmpty(), qPrintable(embeddedChartProblem(bookChart(), bookOf(rows + blanks(150000)))));
        // a whole column is not followed (it is not read as a million points), and more series than a chart made here has
        Chart column = bookChart();
        column.plots[0].series[0].valueRef = QStringLiteral("Sheet1!$A:$A");
        QVERIFY(embeddedChartProblem(column, bookOf(rows)).contains(QStringLiteral("can't be followed")));
        Chart wide = bookChart();
        while (wide.plots[0].series.size() < kMostChartSeries + 1) wide.plots[0].series.append(wide.plots[0].series.last());
        QVERIFY(embeddedChartProblem(wide, bookOf(rows)).contains(QString::number(kMostChartSeries)));
    }

    // A chart is asked about again and again (each refresh of the window's commands), and its workbook is opened once.
    void aWorkbookIsOpenedOnceForAChartThatIsAskedAboutAgainAndAgain()
    {
        const QByteArray book = jp::testing::salesBook();
        const Chart chart = bookChart();
        QVERIFY(chartInfoOf(chart, writeChartPart(chart), book).dataEditable);
        const qint64 before = embeddedWorkbooksOpenedForTest();
        for (int i = 0; i < 20; ++i) {
            QVERIFY(chartInfoOf(chart, writeChartPart(chart), book).dataEditable);
            QVERIFY(chartDataProblem(chart, nullptr, QByteArray(book)).isEmpty());
        }
        QCOMPARE(embeddedWorkbooksOpenedForTest() - before, qint64(0));
        // (another workbook, or other cells to read, is asked once each)
        Chart other = chart;
        other.plots[0].series[0].valueRef = QStringLiteral("Sheet1!$C$2:$C$4");
        other.plots[0].series[1].valueRef = QStringLiteral("Sheet1!$B$2:$B$4");
        for (int i = 0; i < 5; ++i) QVERIFY(chartDataProblem(other, nullptr, book).isEmpty());
        QCOMPARE(embeddedWorkbooksOpenedForTest() - before, qint64(1));
    }
};

JP_TEST_CLASS(SheetChartEditTests)
#include "tst_sheet_chartedit.moc"
