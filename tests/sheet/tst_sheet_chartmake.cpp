// Tests of making a chart from cells (libs/office/sheet/chartmake.h): which cells of a selection are the names, categories, and values, which way the series run, the chart of each kind,
// and the bounds on what a selection of any size makes. The cells are a fake.
#include "drawinghelpers.h"

#include "core/fonts.h"
#include "office/chart_write.h"
#include "office/theme.h"
#include "sheet/chartmake.h"
#include "sheet/chartops.h"

#include <QMap>

using namespace jp::office;
using namespace jp::office::sheet;

namespace {

class Cells : public ChartCells {
public:
    QMap<QPair<quint32, quint32>, Value> cells;
    QMap<QPair<quint32, quint32>, QString> shown;   // what a cell shows, when it is not the value's plain text (a date)
    QMap<QPair<quint32, quint32>, QString> codes;   // the number format codes of cells that have one
    quint32 rows = 0, cols = 0;                     // the extent, when it is more than the cells reach
    mutable qint64 reads = 0;
    int sheetCount() const override { return 1; }
    int sheetIndex(const QString &) const override { return 0; }
    Value value(int, quint32 row, quint32 col) const override
    {
        ++reads;
        return cells.value({row, col});
    }
    QString text(int, quint32 row, quint32 col) const override
    {
        if (shown.contains({row, col})) return shown.value({row, col});
        const Value v = cells.value({row, col});
        return v.isText() ? v.text() : v.isNumber() ? numberToText(v.number()) : QString();
    }
    QString formatCode(int, quint32 row, quint32 col) const override { return codes.value({row, col}, QStringLiteral("General")); }
    bool rowHidden(int, quint32) const override { return false; }
    bool columnHidden(int, quint32) const override { return false; }
    void extent(int, quint32 *r, quint32 *c) const override
    {
        *r = rows;
        *c = cols;
        for (auto it = cells.begin(); it != cells.end(); ++it) {
            *r = std::max(*r, it.key().first + 1);
            *c = std::max(*c, it.key().second + 1);
        }
    }
    QString definedName(const QString &, int) const override { return QString(); }
    void put(const QString &ref, const Value &v)
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        cells[{r, c}] = v;
    }
    void put(const QString &ref, const QString &t) { put(ref, Value::text(t)); }
    void put(const QString &ref, double n) { put(ref, Value::number(n)); }
};

Cells regional()
{
    Cells f;
    f.put(QStringLiteral("A1"), QStringLiteral("Region"));
    f.put(QStringLiteral("B1"), QStringLiteral("East"));
    f.put(QStringLiteral("C1"), QStringLiteral("West"));
    const char *labels[] = {"Q1", "Q2", "Q3"};
    for (int i = 0; i < 3; ++i) {
        f.put(QStringLiteral("A%1").arg(i + 2), QLatin1String(labels[i]));
        f.put(QStringLiteral("B%1").arg(i + 2), 10.0 * (i + 1));
        f.put(QStringLiteral("C%1").arg(i + 2), 5.0 * (i + 1));
    }
    return f;
}

CellRange range(const QString &text)
{
    CellRange r;
    parseRangeRef(text, &r);
    return r;
}

} // namespace

class SheetChartMakeTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    // Recommended charts: a column chart always, a pie for one series of a few positive values, a line for dates, points for numbers with no labels, and none for no data.
    void chartsAreRecommendedForTheShapeOfTheData()
    {
        Cells f = regional();
        ChartPlan plan = planChart(f, 0, QStringLiteral("Sheet1"), range(QStringLiteral("A1:C4")), ChartKind::Column);
        QVector<ChartKind> kinds = recommendChartKinds(f, 0, plan);
        QCOMPARE(kinds.value(0), ChartKind::Column);
        QVERIFY(kinds.contains(ChartKind::Bar) && kinds.contains(ChartKind::ColumnStacked));
        QVERIFY(!kinds.contains(ChartKind::Pie));   // (two series)
        QVERIFY(!kinds.contains(ChartKind::Scatter));
        QVERIFY(kinds.size() <= 6);
        // one series of a few positive values, with labels: a pie too
        plan = planChart(f, 0, QStringLiteral("Sheet1"), range(QStringLiteral("A1:B4")), ChartKind::Column);
        QVERIFY(recommendChartKinds(f, 0, plan).contains(ChartKind::Pie));
        // a negative value is no slice
        f.put(QStringLiteral("B3"), -4.0);
        plan = planChart(f, 0, QStringLiteral("Sheet1"), range(QStringLiteral("A1:B4")), ChartKind::Column);
        QVERIFY(!recommendChartKinds(f, 0, plan).contains(ChartKind::Pie));
        // dates down the side: a line with markers and an area, and no bars
        Cells d;
        d.put(QStringLiteral("A1"), QStringLiteral("Day"));
        d.put(QStringLiteral("B1"), QStringLiteral("Sales"));
        for (int i = 0; i < 6; ++i) {
            d.put(QStringLiteral("A%1").arg(i + 2), 45000.0 + i);
            d.shown[{quint32(i + 1), 0}] = QStringLiteral("1/%1/2023").arg(i + 1);
            d.put(QStringLiteral("B%1").arg(i + 2), 3.0 + i);
        }
        plan = planChart(d, 0, QStringLiteral("Sheet1"), range(QStringLiteral("A1:B7")), ChartKind::Column);
        kinds = recommendChartKinds(d, 0, plan);
        QVERIFY2(kinds.contains(ChartKind::LineMarkers) && kinds.contains(ChartKind::Area) && !kinds.contains(ChartKind::Bar), "dates");
        // two columns of plain numbers: points
        Cells n;
        for (int i = 0; i < 6; ++i) {
            n.put(QStringLiteral("A%1").arg(i + 1), 1.5 * i + 1);
            n.put(QStringLiteral("B%1").arg(i + 1), 2.0 * i * i);
        }
        plan = planChart(n, 0, QStringLiteral("Sheet1"), range(QStringLiteral("A1:B6")), ChartKind::Column);
        QCOMPARE(recommendChartKinds(n, 0, plan).value(0), ChartKind::Scatter);
        // nothing to chart
        QVERIFY(recommendChartKinds(Cells(), 0, ChartPlan()).isEmpty());
    }

    void theFirstRowAndColumnAreTheNamesAndTheCategories()
    {
        const Cells f = regional();
        const ChartPlan plan = planChart(f, 0, QStringLiteral("Sheet1"), range(QStringLiteral("A1:C4")), ChartKind::Column);
        QCOMPARE(plan.series.size(), 2);
        QVERIFY(plan.headerRow && plan.headerColumn && !plan.seriesInRows);
        QCOMPARE(plan.series[0].nameRef, QStringLiteral("Sheet1!$B$1"));
        QCOMPARE(plan.series[0].categoryRef, QStringLiteral("Sheet1!$A$2:$A$4"));
        QCOMPARE(plan.series[0].valueRef, QStringLiteral("Sheet1!$B$2:$B$4"));
        QCOMPARE(plan.series[1].nameRef, QStringLiteral("Sheet1!$C$1"));
        QCOMPARE(plan.series[1].valueRef, QStringLiteral("Sheet1!$C$2:$C$4"));
        QCOMPARE(plan.points, 3);
        // numbers alone: no names, no categories
        const ChartPlan bare = planChart(f, 0, QStringLiteral("Sheet1"), range(QStringLiteral("B2:C4")), ChartKind::Column);
        QCOMPARE(bare.series.size(), 2);
        QVERIFY(!bare.headerRow && !bare.headerColumn);
        QVERIFY(bare.series[0].nameRef.isEmpty() && bare.series[0].categoryRef.isEmpty());
        QCOMPARE(bare.series[1].valueRef, QStringLiteral("Sheet1!$C$2:$C$4"));
        // the names above the numbers but no categories beside them
        const ChartPlan named = planChart(f, 0, QStringLiteral("Sheet1"), range(QStringLiteral("B1:C4")), ChartKind::Column);
        QVERIFY(named.headerRow && !named.headerColumn);
        QCOMPARE(named.series[0].nameRef, QStringLiteral("Sheet1!$B$1"));
        QVERIFY(named.series[0].categoryRef.isEmpty());
        // a cell inside the block takes the block around it
        const ChartPlan around = planChart(f, 0, QStringLiteral("Sheet1"), range(QStringLiteral("B3")), ChartKind::Column);
        QCOMPARE(around.series.size(), 2);
        QCOMPARE(around.series[0].valueRef, QStringLiteral("Sheet1!$B$2:$B$4"));
        // whole columns go as far as the cells do
        CellRange whole;
        whole.row1 = 0;
        whole.row2 = 1048575;
        whole.col1 = 0;
        whole.col2 = 2;
        whole.valid = true;
        const ChartPlan columns = planChart(f, 0, QStringLiteral("Sheet1"), whole, ChartKind::Column);
        QCOMPARE(columns.series.size(), 2);
        QCOMPARE(columns.series[0].valueRef, QStringLiteral("Sheet1!$B$2:$B$4"));
    }

    void seriesRunTheShortWay()
    {
        Cells f;
        f.put(QStringLiteral("A1"), QStringLiteral("Item"));
        for (int c = 1; c <= 5; ++c) f.put(QStringLiteral("%1%2").arg(QChar('A' + c)).arg(1), QStringLiteral("M%1").arg(c));
        f.put(QStringLiteral("A2"), QStringLiteral("Rent"));
        f.put(QStringLiteral("A3"), QStringLiteral("Food"));
        for (int c = 1; c <= 5; ++c) {
            f.put(QStringLiteral("%1%2").arg(QChar('A' + c)).arg(2), 100.0 * c);
            f.put(QStringLiteral("%1%2").arg(QChar('A' + c)).arg(3), 10.0 * c);
        }
        const ChartPlan plan = planChart(f, 0, QStringLiteral("Sheet1"), range(QStringLiteral("A1:F3")), ChartKind::Column);
        QVERIFY(plan.seriesInRows);
        QCOMPARE(plan.series.size(), 2);
        QCOMPARE(plan.series[0].nameRef, QStringLiteral("Sheet1!$A$2"));
        QCOMPARE(plan.series[0].categoryRef, QStringLiteral("Sheet1!$B$1:$F$1"));
        QCOMPARE(plan.series[0].valueRef, QStringLiteral("Sheet1!$B$2:$F$2"));
        // the person's choice of the other way
        const ChartPlan columns = planChart(f, 0, QStringLiteral("Sheet1"), range(QStringLiteral("A1:F3")), ChartKind::Column, 0);
        QVERIFY(!columns.seriesInRows);
        QCOMPARE(columns.series.size(), 5);
        QCOMPARE(columns.series[0].nameRef, QStringLiteral("Sheet1!$B$1"));
        QCOMPARE(columns.series[0].categoryRef, QStringLiteral("Sheet1!$A$2:$A$3"));
        // equal numbers of rows and columns: columns
        const ChartPlan square = planChart(regional(), 0, QStringLiteral("S"), range(QStringLiteral("B2:C3")), ChartKind::Column);
        QVERIFY(!square.seriesInRows);
    }

    void datesAreCategoriesAndPlainNumbersAreNot()
    {
        Cells f;
        f.put(QStringLiteral("A1"), QStringLiteral("When"));
        f.put(QStringLiteral("B1"), QStringLiteral("Sales"));
        for (int i = 0; i < 3; ++i) {
            f.put(QStringLiteral("A%1").arg(i + 2), 46000.0 + i);
            f.shown[{quint32(i + 1), 0}] = QStringLiteral("1/%1/2026").arg(i + 1);
            f.put(QStringLiteral("B%1").arg(i + 2), 10.0 + i);
        }
        const ChartPlan dates = planChart(f, 0, QStringLiteral("Sheet1"), range(QStringLiteral("A1:B4")), ChartKind::Line);
        QVERIFY(dates.headerRow && dates.headerColumn);
        QCOMPARE(dates.series.size(), 1);
        QCOMPARE(dates.series[0].categoryRef, QStringLiteral("Sheet1!$A$2:$A$4"));
        // the same numbers shown as numbers are a series of their own
        f.shown.clear();
        const ChartPlan numbers = planChart(f, 0, QStringLiteral("Sheet1"), range(QStringLiteral("A1:B4")), ChartKind::Line);
        QVERIFY(numbers.headerRow && !numbers.headerColumn);
        QCOMPARE(numbers.series.size(), 2);
        // an empty corner says both are headers, whatever they hold
        f.cells.remove({0, 0});
        f.put(QStringLiteral("A2"), 2019.0);
        f.put(QStringLiteral("A3"), 2020.0);
        f.put(QStringLiteral("A4"), 2021.0);
        const ChartPlan corner = planChart(f, 0, QStringLiteral("Sheet1"), range(QStringLiteral("A1:B4")), ChartKind::Column);
        QVERIFY(corner.headerRow && corner.headerColumn);
        QCOMPARE(corner.series.size(), 1);
    }

    void aScatterTakesItsXValuesFromTheFirstColumn()
    {
        Cells f;
        f.put(QStringLiteral("A1"), QStringLiteral("x"));
        f.put(QStringLiteral("B1"), QStringLiteral("y"));
        f.put(QStringLiteral("C1"), QStringLiteral("z"));
        for (int i = 0; i < 4; ++i) {
            f.put(QStringLiteral("A%1").arg(i + 2), 1.5 * i);
            f.put(QStringLiteral("B%1").arg(i + 2), 2.0 * i);
            f.put(QStringLiteral("C%1").arg(i + 2), 3.0 * i);
        }
        const ChartPlan plan = planChart(f, 0, QStringLiteral("Sheet1"), range(QStringLiteral("A1:C5")), ChartKind::Scatter);
        QCOMPARE(plan.series.size(), 2);
        QCOMPARE(plan.series[0].xRef, QStringLiteral("Sheet1!$A$2:$A$5"));
        QCOMPARE(plan.series[1].valueRef, QStringLiteral("Sheet1!$C$2:$C$5"));
        QVERIFY(plan.series[0].categoryRef.isEmpty());
        // a pie is its first series only
        const ChartPlan pie = planChart(regional(), 0, QStringLiteral("Sheet1"), range(QStringLiteral("A1:C4")), ChartKind::Pie);
        QCOMPARE(pie.series.size(), 1);
    }

    void sheetNamesAreQuotedWhenTheyNeedIt()
    {
        CellRange one = range(QStringLiteral("B2")), many = range(QStringLiteral("B2:C9"));
        QCOMPARE(chartRef(QStringLiteral("Sheet1"), one), QStringLiteral("Sheet1!$B$2"));
        QCOMPARE(chartRef(QStringLiteral("My Sheet"), many), QStringLiteral("'My Sheet'!$B$2:$C$9"));
        QCOMPARE(chartRef(QStringLiteral("O'Brien"), one), QStringLiteral("'O''Brien'!$B$2"));
        QCOMPARE(chartRef(QStringLiteral("A1"), one), QStringLiteral("'A1'!$B$2"));
        QCOMPARE(chartRef(QStringLiteral("2026"), one), QStringLiteral("'2026'!$B$2"));
        QCOMPARE(chartRef(QStringLiteral("Q&A"), one), QStringLiteral("'Q&A'!$B$2"));
    }

    void everyKindOfChartIsMadeWrittenAndReadBackAsTheSameKind()
    {
        const Cells f = regional();
        const ChartKind kinds[] = {ChartKind::Column, ChartKind::ColumnStacked, ChartKind::ColumnPercent, ChartKind::Bar, ChartKind::BarStacked, ChartKind::BarPercent, ChartKind::Line,
                                   ChartKind::LineMarkers, ChartKind::LineStacked, ChartKind::Pie, ChartKind::Doughnut, ChartKind::Area, ChartKind::AreaStacked, ChartKind::Scatter,
                                   ChartKind::ScatterLines, ChartKind::ScatterSmooth, ChartKind::Combo, ChartKind::Radar, ChartKind::RadarMarkers, ChartKind::RadarFilled};
        for (const ChartKind kind : kinds) {
            const ChartPlan plan = planChart(f, 0, QStringLiteral("Sheet1"), range(QStringLiteral("A1:C4")), kind);
            QVERIFY(!plan.series.isEmpty());
            const Chart made = makeChart(kind, plan, defaultTheme(), 3);
            QCOMPARE(int(kindOfChart(made)), int(kind));
            const QByteArray part = writeChartPart(made);
            QString why;
            QVERIFY2(chartIsFullyModeled(part, &why), qPrintable(why));
            Chart back;
            QVERIFY2(readChart(part, defaultTheme(), ColorMap(), &back, &why), qPrintable(why));
            QCOMPARE(int(kindOfChart(back)), int(kind));
            int series = 0;
            for (const ChartPlot &p : back.plots) series += int(p.series.size());
            QCOMPARE(series, int(plan.series.size()));
            QCOMPARE(back.plots[0].series[0].valueRef, plan.series[0].valueRef);
            // and it can be drawn
            QVERIFY(!chartSvg(back, QSizeF(400, 300)).isEmpty());
        }
        // a combo is columns with the last series a line on a second pair of axes
        const Chart combo = makeChart(ChartKind::Combo, planChart(f, 0, QStringLiteral("Sheet1"), range(QStringLiteral("A1:C4")), ChartKind::Combo), defaultTheme());
        QCOMPARE(combo.plots.size(), 2);
        QCOMPARE(combo.axes.size(), 4);
        QVERIFY(combo.axis(combo.plots[1].axisIds[0])->deleted);
        QCOMPARE(combo.axis(combo.plots[1].axisIds[1])->position, QStringLiteral("r"));
        // a pie's slices each have a color of their own; a single series is titled by its name
        const Chart pie = makeChart(ChartKind::Pie, planChart(f, 0, QStringLiteral("Sheet1"), range(QStringLiteral("A1:B4")), ChartKind::Pie), defaultTheme(), 3);
        QCOMPARE(pie.plots[0].series[0].pointColors.size(), 3);
        QVERIFY(pie.title.present && pie.title.automatic);
        QVERIFY(pie.legend.shown);
    }

    // A radar is made with the spokes and rings drawn, holds only radar of the first series' look, and is no part of a combination.
    void aRadarChartIsMadeAlone()
    {
        const Cells f = regional();
        const ChartPlan plan = planChart(f, 0, QStringLiteral("Sheet1"), range(QStringLiteral("A1:C4")), ChartKind::Radar);
        const Chart radar = makeChart(ChartKind::Radar, plan, defaultTheme());
        QCOMPARE(radar.plots.size(), 1);
        QCOMPARE(int(radar.plots[0].type), int(ChartType::Radar));
        QVERIFY(!radar.plots[0].filled && !radar.plots[0].markers);
        QCOMPARE(radar.axes.size(), 2);
        QVERIFY(radar.axis(radar.plots[0].axisIds[0])->majorGridlines);   // the spokes
        QVERIFY(radar.axis(radar.plots[0].axisIds[1])->majorGridlines);   // the rings
        for (const ChartSeries &s : radar.plots[0].series) QCOMPARE(int(s.marker.symbol), int(ChartMarker::None));
        QVERIFY(radar.legend.shown);
        const Chart filled = makeChart(ChartKind::RadarFilled, plan, defaultTheme());
        QVERIFY(filled.plots[0].filled);
        QCOMPARE(int(kindOfChart(filled)), int(ChartKind::RadarFilled));
        // the kind of the first series decides: the others follow it
        QVector<ChartSeries> series = radar.plots[0].series;
        QCOMPARE(series.size(), 2);
        const Chart radarFirst = chartFromSeries(series, {SeriesKind{ChartKind::RadarMarkers, false}, SeriesKind{ChartKind::Column, false}}, defaultTheme());
        QCOMPARE(radarFirst.plots.size(), 1);
        QCOMPARE(int(radarFirst.plots[0].type), int(ChartType::Radar));
        QCOMPARE(radarFirst.plots[0].series.size(), 2);
        QVERIFY(radarFirst.plots[0].markers);
        // a radar that is not first is columns
        const Chart columnsFirst = chartFromSeries(series, {SeriesKind{ChartKind::Column, false}, SeriesKind{ChartKind::Radar, false}}, defaultTheme());
        for (const ChartPlot &p : columnsFirst.plots) QVERIFY(p.type != ChartType::Radar);
    }

    // ---- stock charts: made, changed to and from, and refused when the series do not fit

private:
    static QVector<ChartSeries> stockSeries(int count)
    {
        QVector<ChartSeries> out;
        for (int i = 0; i < count; ++i) {
            ChartSeries s;
            s.name = QStringLiteral("S%1").arg(i);
            s.nameRef = QStringLiteral("Sheet1!$%1$1").arg(QChar('B' + i));
            s.categoryRef = QStringLiteral("Sheet1!$A$2:$A$5");
            s.valueRef = QStringLiteral("Sheet1!$%1$2:$%1$5").arg(QChar('B' + i));
            s.categories = {"a", "b", "c", "d"};
            s.values = {1.0 + i, 2.0 + i, 3.0 + i, 4.0 + i};
            out << s;
        }
        return out;
    }

private Q_SLOTS:
    // Each form is made with its series in their order, the volume as columns on the first axis and the prices against the second, written as a stock chart that this writes over, and read back as the same form.
    void aStockChartIsMadeInEachFormWrittenAndReadBack()
    {
        struct Form {
            ChartKind kind;
            int series;
            int plots;
            bool bars;
        };
        const Form forms[] = {{ChartKind::StockHLC, 3, 1, false}, {ChartKind::StockOHLC, 4, 1, true}, {ChartKind::StockVHLC, 4, 2, false}, {ChartKind::StockVOHLC, 5, 2, true}};
        for (const Form &f : forms) {
            const QVector<ChartSeries> series = stockSeries(f.series);
            const Chart made = chartFromSeries(series, QVector<SeriesKind>(f.series, SeriesKind{f.kind, false}), defaultTheme());
            QCOMPARE(int(kindOfChart(made)), int(f.kind));
            QCOMPARE(int(made.plots.size()), f.plots);
            const ChartPlot &stock = made.plots.last();
            QVERIFY(stock.stock && stock.hiLowLines);
            QCOMPARE(stock.upDownBars, f.bars);
            QCOMPARE(int(stock.series.size()), f.series - (f.plots == 2 ? 1 : 0));
            // no line through a series, and the close alone is marked where there are no bars
            for (int i = 0; i < stock.series.size(); ++i) {
                QVERIFY(stock.series[i].outline.none);
                QCOMPARE(int(stock.series[i].marker.symbol), int(!f.bars && i == stock.series.size() - 1 ? ChartMarker::Dash : ChartMarker::None));
            }
            if (f.plots == 2) {
                // the volume is columns on the first axes; the prices are against a second pair, whose value axis is on the right
                QCOMPARE(int(made.plots[0].type), int(ChartType::Bar));
                QCOMPARE(int(made.plots[0].series.size()), 1);
                QCOMPARE(made.axes.size(), 4);
                QCOMPARE(made.axis(stock.axisIds[1])->position, QStringLiteral("r"));
                QVERIFY(made.plots[0].axisIds[1] != stock.axisIds[1]);
            } else {
                QCOMPARE(made.axes.size(), 2);
            }
            const QByteArray part = writeChartPart(made);
            QVERIFY(part.contains("<c:stockChart>"));
            QString why;
            QVERIFY2(chartIsFullyModeled(part, &why), qPrintable(why));
            Chart back;
            QVERIFY2(readChart(part, defaultTheme(), ColorMap(), &back, &why), qPrintable(why));
            QCOMPARE(int(kindOfChart(back)), int(f.kind));
            QCOMPARE(chartSvg(back, QSizeF(400, 300)), chartSvg(made, QSizeF(400, 300)));
            QCOMPARE(writeChartPart(back), part);
        }
    }

    // The columns are drawn as given: close, high, low is a high-low-close chart of exactly those series in that order (nothing checks their order), the close's mark on the third.
    void aStockChartIsMadeFromTheColumnsInTheOrderGiven()
    {
        QVector<ChartSeries> series = stockSeries(3);
        const QStringList names = {"Close", "High", "Low"};
        for (int i = 0; i < 3; ++i) series[i].name = names[i];
        series[0].values = {5, 6, 7, 8};   // (a close below the high and above the low, or not: no value is looked at)
        series[1].values = {9, 9, 9, 9};
        series[2].values = {1, 1, 1, 1};
        const Chart made = chartFromSeries(series, QVector<SeriesKind>(3, SeriesKind{ChartKind::StockHLC, false}), defaultTheme());
        QCOMPARE(int(made.plots.size()), 1);
        QVERIFY(made.plots[0].stock);
        QCOMPARE(int(made.plots[0].series.size()), 3);
        for (int i = 0; i < 3; ++i) {
            QCOMPARE(made.plots[0].series[i].name, names[i]);
            QCOMPARE(made.plots[0].series[i].values, series[i].values);
        }
        QVERIFY(made.plots[0].series[0].marker.symbol == ChartMarker::None && made.plots[0].series[1].marker.symbol == ChartMarker::None);
        QVERIFY(made.plots[0].series[2].marker.symbol != ChartMarker::None);
        // and a chart changed to the form keeps its series in the order they stand
        Chart c = chartFromSeries(series, QVector<SeriesKind>(3, SeriesKind{ChartKind::LineMarkers, false}), defaultTheme());
        QVERIFY(chartSetForm(c, ChartForm::StockHighLowClose, defaultTheme()));
        const QVector<ChartSeries> now = chartAllSeries(c);
        for (int i = 0; i < 3; ++i) QCOMPARE(now[i].name, names[i]);
    }

    // The forms are told apart by the number and the order of the series: other series are refused with a sentence that names the order, and never made into something else.
    void aStockChartOfSeriesThatDoNotFitIsRefusedAndNeverDrawnAsAnother()
    {
        const struct { ChartKind kind; int right; const char *order; } forms[] = {{ChartKind::StockHLC, 3, "high, low, and close"}, {ChartKind::StockOHLC, 4, "open, high, low, and close"},
                                                                                  {ChartKind::StockVHLC, 4, "volume, high, low, and close"}, {ChartKind::StockVOHLC, 5, "volume, open, high, low, and close"}};
        for (const auto &f : forms) {
            for (int n = 1; n <= 7; ++n) {
                const QString problem = stockProblem(f.kind, n);
                QCOMPARE(problem.isEmpty(), n == f.right);
                if (n == f.right) {
                    QVERIFY(isStockKind(f.kind));
                    continue;
                }
                QVERIFY2(problem.contains(QLatin1String(f.order)), qPrintable(problem));
                // and the chart made of them anyway is no stock chart (lines): nothing is drawn as stock that is not
                const Chart c = chartFromSeries(stockSeries(n), QVector<SeriesKind>(n, SeriesKind{f.kind, false}), defaultTheme());
                for (const ChartPlot &p : c.plots) QVERIFY(!p.stock && !p.hiLowLines && !p.upDownBars);
                QCOMPARE(int(chartStockForm(c)), int(ChartStockForm::None));
            }
        }
        QVERIFY(stockProblem(ChartKind::Column, 2).isEmpty());
    }

    // A chart changed to a stock form keeps its numbers, and changed from it keeps them and the look of the new kind only: the lines and marks stock gave its series, and the scale of its prices or volume, are not carried.
    void aChartChangedToAndFromStockKeepsItsDataAndTheNewKindsLook()
    {
        const QVector<ChartSeries> series = stockSeries(4);
        const Chart line = chartFromSeries(series, QVector<SeriesKind>(4, SeriesKind{ChartKind::LineMarkers, false}), defaultTheme());
        QCOMPARE(chartAllSeries(line).size(), 4);
        for (const ChartForm form : {ChartForm::StockOpenHighLowClose, ChartForm::StockVolumeHighLowClose}) {
            Chart c = line;
            ChartAxis *val = chartAxisOf(c, ChartAxisKind::Value);
            val->autoMax = false;   // (a scale fixed for the old kind's numbers)
            val->max = 7;
            QVERIFY(chartFormProblem(c, form).isEmpty());
            QVERIFY(chartSetForm(c, form, defaultTheme()));
            QCOMPARE(int(chartInfoOf(c, QByteArray()).form), int(form));
            const QVector<ChartSeries> now = chartAllSeries(c);
            QCOMPARE(now.size(), 4);
            for (int i = 0; i < 4; ++i) QCOMPARE(now[i].values, series[i].values);
            const bool volumeForm = form == ChartForm::StockVolumeHighLowClose;
            // (a scale fixed for numbers of the same kind goes along; one fixed for the other kind's numbers, the volume's or the prices', does not)
            QCOMPARE(chartAxisOf(c, ChartAxisKind::Value)->autoMax, volumeForm);
            // and back to a line with markers: the series are drawn again, the lines and marks are the line's own
            QVERIFY(chartSetForm(c, ChartForm::LineMarkers, defaultTheme()));
            QCOMPARE(c.plots.size(), 1);
            QVERIFY(!c.plots[0].stock && !c.plots[0].hiLowLines && !c.plots[0].upDownBars);
            QCOMPARE(int(c.plots[0].series.size()), 4);
            for (int i = 0; i < 4; ++i) {
                QVERIFY(!c.plots[0].series[i].outline.none);
                QCOMPARE(int(c.plots[0].series[i].marker.symbol), int(ChartMarker::Auto));
                QCOMPARE(c.plots[0].series[i].values, series[i].values);
            }
            QCOMPARE(chartAxisOf(c, ChartAxisKind::Value)->autoMax, volumeForm);
        }
        // fixed scales of a volume form do not go to the columns it is changed to either
        Chart volume = chartFromSeries(stockSeries(4), QVector<SeriesKind>(4, SeriesKind{ChartKind::StockVHLC, false}), defaultTheme());
        chartAxisOf(volume, ChartAxisKind::Value)->autoMax = false;
        chartAxisOf(volume, ChartAxisKind::Value)->max = 99;
        QVERIFY(chartSetForm(volume, ChartForm::Column, defaultTheme()));
        QVERIFY(chartAxisOf(volume, ChartAxisKind::Value)->autoMax);
        // from a column chart of the other kinds' gridlines nothing else is lost: the value axis has its gridlines as every kind's has
        QVERIFY(chartAxisOf(volume, ChartAxisKind::Value)->majorGridlines);
    }

    // A chart of the wrong series for a stock form is not changed, and the reason is the sentence of the order; the sample data of each form fits it.
    void aChartIsNotChangedToAStockFormItsSeriesDoNotFit()
    {
        const Chart two = chartOfData(ChartForm::Line, sampleChartData(), defaultTheme());
        for (const ChartForm form : {ChartForm::StockHighLowClose, ChartForm::StockOpenHighLowClose, ChartForm::StockVolumeHighLowClose, ChartForm::StockVolumeOpenHighLowClose}) {
            Chart c = two;
            const QString problem = chartFormProblem(c, form);
            QVERIFY(problem.contains(QStringLiteral("in this order")));
            const QByteArray before = writeChartPart(c);
            QVERIFY(!chartSetForm(c, form, defaultTheme()));
            QCOMPARE(writeChartPart(c), before);
            // the sample numbers of the form are made into the form
            const ChartData data = sampleChartData(form);
            QVERIFY(chartDataIsUsable(data));
            const Chart made = chartOfData(form, data, defaultTheme());
            QCOMPARE(int(chartInfoOf(made, QByteArray()).form), int(form));
            QVERIFY(chartFormProblem(made, form).isEmpty());
            // and a stock chart's series are not added to or taken away by editing its numbers
            Chart edited = made;
            ChartData more = data;
            more.seriesNames << QStringLiteral("Extra");
            more.values << data.values.first();
            QVERIFY(!chartSetData(edited, more, defaultTheme()));
            QVERIFY(chartSetData(edited, data, defaultTheme()));
            QCOMPARE(int(chartInfoOf(edited, QByteArray()).form), int(form));
        }
    }

    // A chart changed between radar and another kind takes the new kind's own gridlines: the spokes are not left behind as upright lines, and a column is not left without them.
    void aChartChangedToOrFromRadarTakesTheNewKindsGridlines()
    {
        const Cells f = regional();
        const ChartPlan plan = planChart(f, 0, QStringLiteral("Sheet1"), range(QStringLiteral("A1:C4")), ChartKind::Radar);
        const Chart radar = makeChart(ChartKind::Radar, plan, defaultTheme());
        const QVector<ChartSeries> series = radar.plots[0].series;
        const ChartKind others[] = {ChartKind::Column, ChartKind::Bar, ChartKind::Line};
        for (const ChartKind kind : others) {
            const QVector<SeriesKind> kinds(series.size(), SeriesKind{kind, false});
            const Chart changed = chartFromSeries(series, kinds, defaultTheme(), &radar);
            const Chart fresh = makeChart(kind, plan, defaultTheme());
            QCOMPARE(changed.axis(changed.plots[0].axisIds[0])->majorGridlines, fresh.axis(fresh.plots[0].axisIds[0])->majorGridlines);
            QCOMPARE(changed.axis(changed.plots[0].axisIds[1])->majorGridlines, fresh.axis(fresh.plots[0].axisIds[1])->majorGridlines);
            QCOMPARE(chartSvg(changed, QSizeF(400, 300)), chartSvg(fresh, QSizeF(400, 300)));
            // and back: the spokes and rings are there again
            const QVector<SeriesKind> back(series.size(), SeriesKind{ChartKind::Radar, false});
            const Chart again = chartFromSeries(changed.plots[0].series, back, defaultTheme(), &changed);
            QVERIFY(again.axis(again.plots[0].axisIds[0])->majorGridlines);
            QVERIFY(again.axis(again.plots[0].axisIds[1])->majorGridlines);
            QCOMPARE(chartSvg(again, QSizeF(400, 300)), chartSvg(radar, QSizeF(400, 300)));
        }
        // a column that has its gridlines turned off keeps them off through a change to a line
        Chart column = makeChart(ChartKind::Column, plan, defaultTheme());
        column.axes[1].majorGridlines = false;
        const QVector<SeriesKind> asLine(series.size(), SeriesKind{ChartKind::Line, false});
        QVERIFY(!chartFromSeries(column.plots[0].series, asLine, defaultTheme(), &column).axes[1].majorGridlines);
    }

    void aSelectionOfAnySizeMakesALimitedChart()
    {
        Cells f;
        f.rows = 30000;
        f.cols = 700;
        f.put(QStringLiteral("A1"), QStringLiteral("x"));
        for (quint32 c = 1; c < 700; ++c) f.cells[{0, c}] = Value::text(QStringLiteral("s%1").arg(c));
        for (quint32 r = 1; r < 30; ++r)
            for (quint32 c = 1; c < 700; ++c) f.cells[{r, c}] = Value::number(r + c);
        f.cells[{29999, 1}] = Value::number(1);
        CellRange all = range(QStringLiteral("A1:ZZ30000"));
        const ChartPlan plan = planChart(f, 0, QStringLiteral("Sheet1"), all, ChartKind::Column, 0);
        QVERIFY(plan.truncated);
        QVERIFY(plan.series.size() <= kMaxMadeSeries);
        QVERIFY(plan.points <= kMaxChartSeriesCells);
        QVERIFY(qint64(plan.series.size()) * plan.points <= kMaxChartCells);
        QVERIFY(f.reads < 6000000);
        // nothing to chart
        const Cells empty;
        QVERIFY(planChart(empty, 0, QStringLiteral("S"), range(QStringLiteral("A1:C3")), ChartKind::Column).series.isEmpty());
        QVERIFY(planChart(f, 0, QStringLiteral("S"), CellRange(), ChartKind::Column).series.isEmpty());
    }

    // A chart made here has no format of its own: the series' values take the format of the cells they are read from, unless the chart already says one.
    void seriesTakeTheFormatOfTheirCells()
    {
        Cells f = regional();
        f.codes[{1, 1}] = QStringLiteral("$#,##0.00");
        f.codes[{1, 2}] = QStringLiteral("0%");
        Chart c = makeChart(ChartKind::Column, planChart(f, 0, QStringLiteral("Sheet1"), range(QStringLiteral("A1:C4")), ChartKind::Column), defaultTheme());
        c.plots[0].series[1].format = QStringLiteral("0.0");   // (this one says its own)
        applyChartFormats(&c, 0, f);
        QCOMPARE(c.plots[0].series[0].format, QStringLiteral("$#,##0.00"));
        QCOMPARE(c.plots[0].series[1].format, QStringLiteral("0.0"));
        // written, and read back, the format is in the part
        Chart back;
        QVERIFY(readChart(writeChartPart(c), defaultTheme(), ColorMap(), &back));
        QCOMPARE(back.plots[0].series[0].format, QStringLiteral("$#,##0.00"));
        // cells that keep no format to ask for leave it alone
        Chart plain = makeChart(ChartKind::Column, planChart(f, 0, QStringLiteral("Sheet1"), range(QStringLiteral("A1:C4")), ChartKind::Column), defaultTheme());
        struct Mute : Cells {
            QString formatCode(int, quint32, quint32) const override { return QString(); }
        } mute;
        mute.cells = f.cells;
        applyChartFormats(&plain, 0, mute);
        QVERIFY(plain.plots[0].series[0].format.isEmpty());
    }

    // A block that is cut to a few rows and columns is cut before the cells are looked at: a whole sheet of numbers, or the block around one cell of it, costs what the cut block holds.
    void aBlockCutShortIsCutBeforeItIsLookedAt()
    {
        Cells f;
        for (quint32 r = 0; r < 30000; ++r)
            for (quint32 c = 0; c < 4; ++c) f.cells[{r, c}] = Value::number(r + c);
        CellRange whole = range(QStringLiteral("A1:XFD1048576"));
        const qint64 before = f.reads;
        const CellRange cut = chartBlockOf(f, 0, whole, nullptr, 41, 13);
        QCOMPARE(int(cut.row2 - cut.row1 + 1), 41);
        QCOMPARE(int(cut.col2 - cut.col1 + 1), 4);
        const qint64 cutReads = f.reads - before;
        QVERIFY2(cutReads < 2000, qPrintable(QString::number(cutReads)));
        const qint64 mid = f.reads;
        const CellRange around = chartBlockOf(f, 0, range(QStringLiteral("B5")), nullptr, 41, 13);
        QVERIFY(around.valid);
        QVERIFY(int(around.row2 - around.row1 + 1) <= 41);
        QVERIFY2(f.reads - mid < 3000, qPrintable(QString::number(f.reads - mid)));
        // without the cut the block is all of it
        const qint64 start = f.reads;
        const CellRange all = chartBlockOf(f, 0, range(QStringLiteral("B5")));
        QCOMPARE(int(all.row2 - all.row1 + 1), 30000);
        QVERIFY(f.reads - start > 20000);
    }
};

JP_TEST_CLASS(SheetChartMakeTests)
#include "tst_sheet_chartmake.moc"
