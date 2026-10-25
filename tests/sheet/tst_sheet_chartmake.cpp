// Tests of making a chart from cells (libs/office/sheet/chartmake.h): which cells of a selection are the names, categories, and values, which way the series run, the chart of each kind,
// and the bounds on what a selection of any size makes. The cells are a fake.
#include "drawinghelpers.h"

#include "core/fonts.h"
#include "office/chart_write.h"
#include "office/theme.h"
#include "sheet/chartmake.h"

#include <QMap>

using namespace jp::office;

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
                                   ChartKind::ScatterLines, ChartKind::Combo};
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
