// Tests of charts that follow their cells (libs/office/sheet/chartdata.h): reading where a series gets its data from the chart part, resolving the references (sheets, names,
// several areas, whole columns), and putting the cells' contents in the chart. The cells are a fake that counts what it is asked, so the bounds are counted, not timed.
#include "drawinghelpers.h"

#include "core/fonts.h"
#include "grid/chartcells.h"
#include "office/theme.h"
#include "sheet/chartdata.h"
#include "sheet/recalc.h"

#include <QMap>
#include <QRandomGenerator>
#include <cmath>

using namespace drawtest;

namespace {

// Sheets of cells by address, with hidden rows and columns and defined names.
class FakeCells : public ChartCells {
public:
    struct Sheet {
        QString name;
        QMap<QPair<quint32, quint32>, Value> cells;
        QMap<QPair<quint32, quint32>, QString> texts;   // what a cell shows, when it is not its value's plain text
        QSet<quint32> hiddenRows, hiddenCols;
        QMap<QString, QString> names;
    };
    QVector<Sheet> sheets;
    QMap<QString, QString> bookNames;
    mutable qint64 reads = 0, textCalls = 0, nameLookups = 0;

    FakeCells() { sheets.push_back(Sheet{QStringLiteral("Sheet1"), {}, {}, {}, {}, {}}); }
    int sheetCount() const override { return int(sheets.size()); }
    int sheetIndex(const QString &name) const override
    {
        for (int i = 0; i < sheets.size(); ++i)
            if (sheets[i].name.compare(name, Qt::CaseInsensitive) == 0) return i;
        return -1;
    }
    // (the sheet, or an empty one: a copy of a sheet would not be the container it is asked about twice)
    const Sheet &at(int sheet) const
    {
        static const Sheet none;
        return sheet >= 0 && sheet < sheets.size() ? sheets[sheet] : none;
    }
    Value value(int sheet, quint32 row, quint32 col) const override
    {
        ++reads;
        return at(sheet).cells.value({row, col});
    }
    QString text(int sheet, quint32 row, quint32 col) const override
    {
        ++textCalls;
        const Sheet &s = at(sheet);
        if (s.texts.contains({row, col})) return s.texts.value({row, col});
        const Value v = s.cells.value({row, col});
        return v.isText() ? v.text() : v.isNumber() ? numberToText(v.number()) : QString();
    }
    bool rowHidden(int sheet, quint32 row) const override { return at(sheet).hiddenRows.contains(row); }
    bool columnHidden(int sheet, quint32 col) const override { return at(sheet).hiddenCols.contains(col); }
    void extent(int sheet, quint32 *rows, quint32 *cols) const override
    {
        *rows = *cols = 0;
        const Sheet &s = at(sheet);
        for (auto it = s.cells.begin(); it != s.cells.end(); ++it) {
            *rows = std::max(*rows, it.key().first + 1);
            *cols = std::max(*cols, it.key().second + 1);
        }
    }
    QString definedName(const QString &name, int sheet) const override
    {
        ++nameLookups;
        if (sheet >= 0 && at(sheet).names.contains(name.toUpper())) return at(sheet).names.value(name.toUpper());
        return bookNames.value(name.toUpper());
    }
    void set(int sheet, const QString &ref, const Value &v)
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        sheets[sheet].cells[{r, c}] = v;
    }
    // The sheet of the data sheet's cells: A1:C4.
    static FakeCells regional()
    {
        FakeCells f;
        f.set(0, QStringLiteral("A1"), Value::text(QStringLiteral("Region")));
        f.set(0, QStringLiteral("B1"), Value::text(QStringLiteral("East")));
        f.set(0, QStringLiteral("C1"), Value::text(QStringLiteral("West")));
        const char *labels[] = {"Q1", "Q2", "Q3"};
        for (int i = 0; i < 3; ++i) {
            f.set(0, QStringLiteral("A%1").arg(i + 2), Value::text(QLatin1String(labels[i])));
            f.set(0, QStringLiteral("B%1").arg(i + 2), Value::number(10 * (i + 1)));
            f.set(0, QStringLiteral("C%1").arg(i + 2), Value::number(5 * (i + 1)));
        }
        return f;
    }
};

Chart readIt(const QByteArray &part)
{
    Chart c;
    QString why;
    Theme theme;
    if (!readChart(part, theme, ColorMap(), &c, &why)) qFatal("%s", qPrintable(why));
    return c;
}

QVector<SeriesSpec> eastWest()
{
    return {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {"Q1", "Q2", "Q3"}, {1, 2, 3}, QString()},
            SeriesSpec{QStringLiteral("Sheet1!$C$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$C$2:$C$4"), QStringLiteral("West"), {"Q1", "Q2", "Q3"}, {7, 8, 9}, QString()}};
}

QString blocksText(const QVector<ChartBlock> &blocks)
{
    QStringList out;
    for (const ChartBlock &b : blocks) out << QStringLiteral("%1:%2").arg(b.sheet).arg(rangeName(b.range));
    return out.join(QLatin1Char(' '));
}

double nan() { return std::numeric_limits<double>::quiet_NaN(); }

bool sameNumbers(const QVector<double> &a, const QVector<double> &b)
{
    if (a.size() != b.size()) return false;
    for (int i = 0; i < a.size(); ++i)
        if (std::isnan(a[i]) != std::isnan(b[i]) || (!std::isnan(a[i]) && a[i] != b[i])) return false;
    return true;
}

} // namespace

class SheetChartDataTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    // The chart model keeps where each series reads, and the plot's choices about hidden cells; a series with literal data keeps none.
    void chartKeepsWhereItReadsItsData()
    {
        const Chart c = readIt(chartPart(QStringLiteral("barChart"), eastWest(), QString(), false, QStringLiteral("zero")));
        QCOMPARE(int(c.plots.size()), 1);
        QCOMPARE(int(c.plots[0].series.size()), 2);
        const ChartSeries &s = c.plots[0].series[0];
        QCOMPARE(s.nameRef, QStringLiteral("Sheet1!$B$1"));
        QCOMPARE(s.categoryRef, QStringLiteral("Sheet1!$A$2:$A$4"));
        QCOMPARE(s.valueRef, QStringLiteral("Sheet1!$B$2:$B$4"));
        QVERIFY(!s.categoryRefNumeric);
        QCOMPARE(c.plots[0].series[1].valueRef, QStringLiteral("Sheet1!$C$2:$C$4"));
        QVERIFY(!c.plotVisibleOnly);
        QCOMPARE(c.blanksAs, QStringLiteral("zero"));
        QVERIFY(readIt(chartPart(QStringLiteral("barChart"), eastWest())).plotVisibleOnly);
        // the values the file stored are still what the series holds until the cells are looked up
        QCOMPARE(s.values, (QVector<double>{1, 2, 3}));
        QCOMPARE(s.categories, (QStringList{"Q1", "Q2", "Q3"}));
        // a scatter's x and y, and a series that has no reference
        const Chart sc = readIt(chartPart(QStringLiteral("scatterChart"), {SeriesSpec{QString(), QString(), QStringLiteral("Sheet1!$C$2:$C$4"), QString(), {"1", "2", "3"}, {4, 5, 6}, QStringLiteral("Sheet1!$B$2:$B$4")}}));
        QCOMPARE(sc.plots[0].series[0].xRef, QStringLiteral("Sheet1!$B$2:$B$4"));
        QCOMPARE(sc.plots[0].series[0].valueRef, QStringLiteral("Sheet1!$C$2:$C$4"));
        QVERIFY(sc.plots[0].series[0].categoryRef.isEmpty());
        const Chart lit = readIt(chartPart(QStringLiteral("barChart"), {SeriesSpec{QString(), QString(), QString(), QStringLiteral("Lit"), {"a"}, {1}, QString()}}));
        QVERIFY(lit.plots[0].series[0].valueRef.isEmpty() && lit.plots[0].series[0].nameRef.isEmpty());
    }

    // The references a series can have, and the blocks each is.
    void resolvesReferences()
    {
        FakeCells f = FakeCells::regional();
        f.sheets.push_back(FakeCells::Sheet{QStringLiteral("My Sheet"), {}, {}, {}, {}, {}});
        f.sheets.push_back(FakeCells::Sheet{QStringLiteral("It's"), {}, {}, {}, {}, {}});
        f.sheets[0].names[QStringLiteral("LOCALNAME")] = QStringLiteral("Sheet1!$B$2:$B$3");
        f.bookNames[QStringLiteral("SALES")] = QStringLiteral("'My Sheet'!$A$1:$A$2");
        f.bookNames[QStringLiteral("LOOP")] = QStringLiteral("LOOP");
        f.bookNames[QStringLiteral("DEEP1")] = QStringLiteral("DEEP2");
        f.bookNames[QStringLiteral("DEEP2")] = QStringLiteral("DEEP3");
        f.bookNames[QStringLiteral("DEEP3")] = QStringLiteral("DEEP4");
        f.bookNames[QStringLiteral("DEEP4")] = QStringLiteral("DEEP5");
        f.bookNames[QStringLiteral("DEEP5")] = QStringLiteral("Sheet1!$A$1");
        f.bookNames[QStringLiteral("FORMULA")] = QStringLiteral("SUM(Sheet1!$A$1:$A$3)");
        struct Case {
            const char *what;
            QString ref;
            bool ok;
            QString blocks;
        };
        const QVector<Case> cases = {
            {"a range", QStringLiteral("Sheet1!$B$2:$B$4"), true, QStringLiteral("0:B2:B4")},
            {"a cell", QStringLiteral("Sheet1!$B$1"), true, QStringLiteral("0:B1")},
            {"a leading equals sign", QStringLiteral("=Sheet1!$B$2:$B$4"), true, QStringLiteral("0:B2:B4")},
            {"capitals of the sheet's name", QStringLiteral("SHEET1!B2:B4"), true, QStringLiteral("0:B2:B4")},
            {"a sheet with a space", QStringLiteral("'My Sheet'!$A$1:$B$2"), true, QStringLiteral("1:A1:B2")},
            {"a sheet with an apostrophe", QStringLiteral("'It''s'!$C$3"), true, QStringLiteral("2:C3")},
            {"no sheet: the chart's own", QStringLiteral("$B$2:$B$4"), true, QStringLiteral("0:B2:B4")},
            {"reversed corners", QStringLiteral("Sheet1!$B$4:$B$2"), true, QStringLiteral("0:B2:B4")},
            {"several areas in parentheses", QStringLiteral("(Sheet1!$A$2:$A$3,Sheet1!$A$4)"), true, QStringLiteral("0:A2:A3 0:A4")},
            {"several areas on two sheets", QStringLiteral("(Sheet1!$A$2:$A$3,'My Sheet'!$A$4)"), true, QStringLiteral("0:A2:A3 1:A4")},
            {"several areas, no parentheses", QStringLiteral("Sheet1!$A$2,Sheet1!$A$4"), true, QStringLiteral("0:A2 0:A4")},
            {"a name of the sheet", QStringLiteral("Sheet1!LocalName"), true, QStringLiteral("0:B2:B3")},
            {"a name of the workbook", QStringLiteral("Sales"), true, QStringLiteral("1:A1:A2")},
            {"a name of the workbook, written [0]!", QStringLiteral("[0]!Sales"), true, QStringLiteral("1:A1:A2")},
            {"a name followed four deep", QStringLiteral("Deep2"), true, QStringLiteral("0:A1")},
            {"a name followed too deep", QStringLiteral("Deep1"), false, QString()},
            {"a name that is itself", QStringLiteral("Loop"), false, QString()},
            {"a name that is a formula", QStringLiteral("Formula"), false, QString()},
            {"a name nobody defined", QStringLiteral("Nothing"), false, QString()},
            {"a sheet that is not there", QStringLiteral("Gone!$A$1:$A$3"), false, QString()},
            {"another workbook", QStringLiteral("[1]Sheet1!$A$1:$A$3"), false, QString()},
            {"a three-dimensional range", QStringLiteral("Sheet1:Sheet2!$A$1"), false, QString()},
            {"an error from a deleted range", QStringLiteral("Sheet1!#REF!"), false, QString()},
            {"nothing", QString(), false, QString()},
            {"one good area and one that is not", QStringLiteral("(Sheet1!$A$2,Gone!$A$1)"), true, QStringLiteral("0:A2")},
            {"a whole column", QStringLiteral("Sheet1!$B:$B"), true, QStringLiteral("0:B1:B1048576")},
            {"a whole row", QStringLiteral("Sheet1!$2:$2"), true, QStringLiteral("0:A2:XFD2")},
            {"a cell outside the grid", QStringLiteral("Sheet1!$ZZZZ$1"), false, QString()},
        };
        for (const Case &c : cases) {
            QVector<ChartBlock> blocks;
            const bool ok = chartBlocks(c.ref, 0, f, &blocks);
            QVERIFY2(ok == c.ok, c.what);
            QCOMPARE(blocksText(blocks), c.blocks);
        }
        // a sheet's own name wins over the workbook's, and the chart's sheet is the one a bare reference means
        QVector<ChartBlock> blocks;
        QVERIFY(chartBlocks(QStringLiteral("$A$1"), 1, f, &blocks));
        QCOMPARE(blocksText(blocks), QStringLiteral("1:A1"));
        // a reference of a hostile length: more areas than the limit are not read
        QString many = QStringLiteral("(");
        for (int i = 0; i < kMaxChartBlocks + 50; ++i) many += QStringLiteral("Sheet1!$A$1,");
        many += QStringLiteral("Sheet1!$A$2)");
        blocks.clear();
        QVERIFY(chartBlocks(many, 0, f, &blocks));
        QCOMPARE(int(blocks.size()), kMaxChartBlocks);
    }

    // A series takes its name, categories, and values from the cells; changing a cell changes the chart; the picture is drawn from them and changes with them.
    void seriesFollowTheirCells()
    {
        FakeCells f = FakeCells::regional();
        Chart c = readIt(chartPart(QStringLiteral("barChart"), eastWest()));
        QCOMPARE(applyChartCells(&c, 0, f), 2);
        QCOMPARE(c.plots[0].series[0].name, QStringLiteral("East"));
        QCOMPARE(c.plots[0].series[0].categories, (QStringList{"Q1", "Q2", "Q3"}));
        QVERIFY(sameNumbers(c.plots[0].series[0].values, {10, 20, 30}));   // (the file stored 1, 2, 3)
        QVERIFY(sameNumbers(c.plots[0].series[1].values, {5, 10, 15}));
        const QByteArray before = chartSvg(c, QSizeF(480, 288));
        // the cell changes
        f.set(0, QStringLiteral("B3"), Value::number(99));
        f.set(0, QStringLiteral("B1"), Value::text(QStringLiteral("Eastern")));
        f.set(0, QStringLiteral("A4"), Value::text(QStringLiteral("Last")));
        QCOMPARE(applyChartCells(&c, 0, f), 2);
        QVERIFY(sameNumbers(c.plots[0].series[0].values, {10, 99, 30}));
        QCOMPARE(c.plots[0].series[0].name, QStringLiteral("Eastern"));
        QCOMPARE(c.plots[0].series[1].categories.value(2), QStringLiteral("Last"));
        const QByteArray after = chartSvg(c, QSizeF(480, 288));
        QVERIFY(before != after);
        QVERIFY(after.contains("Eastern") && after.contains("Last"));
        // the same cells again: the same picture
        Chart again = readIt(chartPart(QStringLiteral("barChart"), eastWest()));
        applyChartCells(&again, 0, f);
        QCOMPARE(chartSvg(again, QSizeF(480, 288)), after);
        // the categories show the way the cells do (a date, a percent)
        f.set(0, QStringLiteral("A2"), Value::number(45366));
        f.sheets[0].texts[{1, 0}] = QStringLiteral("Mar 15, 2024");
        applyChartCells(&c, 0, f);
        QCOMPARE(c.plots[0].series[0].categories.value(0), QStringLiteral("Mar 15, 2024"));
    }

    // Hidden rows and columns are left out when the chart says so; empty cells, text, true and false, and errors are what a chart makes of them.
    void hiddenAndBlankCells()
    {
        FakeCells f = FakeCells::regional();
        f.sheets[0].hiddenRows.insert(2);   // Q2
        Chart c = readIt(chartPart(QStringLiteral("barChart"), eastWest(), QString(), true));
        applyChartCells(&c, 0, f);
        QVERIFY(sameNumbers(c.plots[0].series[0].values, {10, 30}));
        QCOMPARE(c.plots[0].series[0].categories, (QStringList{"Q1", "Q3"}));
        QVERIFY(sameNumbers(c.plots[0].series[1].values, {5, 15}));
        Chart all = readIt(chartPart(QStringLiteral("barChart"), eastWest(), QString(), false));
        applyChartCells(&all, 0, f);
        QVERIFY(sameNumbers(all.plots[0].series[0].values, {10, 20, 30}));
        // a hidden column holds a series' cells
        FakeCells g = FakeCells::regional();
        g.sheets[0].hiddenCols.insert(1);
        Chart d = readIt(chartPart(QStringLiteral("barChart"), {SeriesSpec{QString(), QString(), QStringLiteral("Sheet1!$A$2:$C$2"), QString(), {}, {1, 2, 3}, QString()}}, QString(), true));
        applyChartCells(&d, 0, g);
        QCOMPARE(int(d.plots[0].series[0].values.size()), 2);   // (A2 is text: 0; C2 is 5; B2 is hidden)
        QVERIFY(sameNumbers(d.plots[0].series[0].values, {0, 5}));
        // blanks, text, booleans, errors
        FakeCells h;
        h.set(0, QStringLiteral("A1"), Value::number(4));
        h.set(0, QStringLiteral("A3"), Value::text(QStringLiteral("n/a")));
        h.set(0, QStringLiteral("A4"), Value::boolean(true));
        h.set(0, QStringLiteral("A5"), Value::error(ErrorKind::Div0));
        h.set(0, QStringLiteral("A6"), Value::error(ErrorKind::NA));
        h.set(0, QStringLiteral("A7"), Value());
        h.set(0, QStringLiteral("A8"), Value::number(8));
        Chart b = readIt(chartPart(QStringLiteral("lineChart"), {SeriesSpec{QString(), QString(), QStringLiteral("Sheet1!$A$1:$A$8"), QString(), {}, {1}, QString()}}));
        applyChartCells(&b, 0, h);
        QVERIFY(sameNumbers(b.plots[0].series[0].values, {4, nan(), 0, 1, nan(), nan(), nan(), 8}));
    }

    // Several areas make one series; a name stands for its cells; a scatter's x values; text for x is 1, 2, 3.
    void namesAreasAndScatter()
    {
        FakeCells f = FakeCells::regional();
        f.bookNames[QStringLiteral("EASTVALUES")] = QStringLiteral("Sheet1!$B$2:$B$4");
        f.sheets[0].names[QStringLiteral("QUARTERS")] = QStringLiteral("Sheet1!$A$2:$A$4");
        Chart c = readIt(chartPart(QStringLiteral("barChart"), {SeriesSpec{QString(), QStringLiteral("Sheet1!Quarters"), QStringLiteral("EastValues"), QString(), {"x"}, {0}, QString()},
                                                               SeriesSpec{QStringLiteral("Sheet1!$B$1:$C$1"), QString(), QStringLiteral("(Sheet1!$B$2,Sheet1!$C$4:$C$3)"), QStringLiteral("old"), {}, {0}, QString()}}));
        applyChartCells(&c, 0, f);
        QVERIFY(sameNumbers(c.plots[0].series[0].values, {10, 20, 30}));
        QCOMPARE(c.plots[0].series[0].categories, (QStringList{"Q1", "Q2", "Q3"}));
        QVERIFY(sameNumbers(c.plots[0].series[1].values, {10, 10, 15}));
        QCOMPARE(c.plots[0].series[1].name, QStringLiteral("East West"));   // (the cells of a name joined by a space)
        // scatter
        Chart s = readIt(chartPart(QStringLiteral("scatterChart"), {SeriesSpec{QString(), QString(), QStringLiteral("Sheet1!$C$2:$C$4"), QString(), {"1", "2", "3"}, {1, 1, 1}, QStringLiteral("Sheet1!$B$2:$B$4")},
                                                                    SeriesSpec{QString(), QString(), QStringLiteral("Sheet1!$C$2:$C$4"), QString(), {"1", "2", "3"}, {1, 1, 1}, QStringLiteral("Sheet1!$A$2:$A$4")}}));
        applyChartCells(&s, 0, f);
        QVERIFY(sameNumbers(s.plots[0].series[0].xValues, {10, 20, 30}));
        QVERIFY(sameNumbers(s.plots[0].series[0].values, {5, 10, 15}));
        QVERIFY(sameNumbers(s.plots[0].series[1].xValues, {1, 2, 3}));   // (x of text: the places of the points)
    }

    // What the file stored is kept for what can't be resolved, and a series with no reference is left alone.
    void whatCannotBeResolvedKeepsTheCache()
    {
        FakeCells f = FakeCells::regional();
        Chart c = readIt(chartPart(QStringLiteral("barChart"), {SeriesSpec{QStringLiteral("Gone!$A$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Gone!$B$2:$B$4"), QStringLiteral("Kept"), {"a", "b", "c"}, {7, 8, 9}, QString()},
                                                               SeriesSpec{QString(), QString(), QString(), QStringLiteral("Lit"), {"p", "q"}, {3, 4}, QString()}}));
        QCOMPARE(applyChartCells(&c, 0, f), 0);
        QCOMPARE(c.plots[0].series[0].name, QStringLiteral("Kept"));
        QVERIFY(sameNumbers(c.plots[0].series[0].values, {7, 8, 9}));
        QCOMPARE(c.plots[0].series[0].categories, (QStringList{"Q1", "Q2", "Q3"}));   // (this reference does resolve)
        QVERIFY(sameNumbers(c.plots[0].series[1].values, {3, 4}));
        QCOMPARE(c.plots[0].series[1].categories, (QStringList{"p", "q"}));
    }

    // An empty cell is a blank in the chart, and the chart draws it as it says (a gap, zero, or the line carried over it): three pictures.
    void blankCellsAreDrawnAsTheChartSays()
    {
        FakeCells f;
        for (int i : {1, 2, 4, 5}) f.set(0, QStringLiteral("A%1").arg(i), Value::number(i * 10));   // (A3 is empty)
        auto svgFor = [&](const char *blanks) {
            Chart c = readIt(chartPart(QStringLiteral("lineChart"), {SeriesSpec{QString(), QString(), QStringLiteral("Sheet1!$A$1:$A$5"), QStringLiteral("S"), {}, {1, 1, 1, 1, 1}, QString()}}, QString(), true, QLatin1String(blanks)));
            applyChartCells(&c, 0, f);
            [&] {
                QVERIFY(std::isnan(c.plots[0].series[0].values[2]));
                QCOMPARE(c.blanksAs, QLatin1String(blanks));
            }();
            return chartSvg(c, QSizeF(480, 288));
        };
        const QByteArray gap = svgFor("gap"), zero = svgFor("zero"), span = svgFor("span");
        QVERIFY(gap != zero);
        QVERIFY(gap != span);
        QVERIFY(zero != span);
    }

    // A name that lists itself eighty times is read once (a name is not read inside itself), and names that each list eighty more are visited up to a bound, however many they would be: the
    // work is counted in the names looked up.
    void namesThatListEachOtherCostBoundedWork()
    {
        FakeCells f = FakeCells::regional();
        QStringList self;
        for (int i = 0; i < 80; ++i) self << QStringLiteral("N");
        f.bookNames[QStringLiteral("N")] = self.join(QLatin1Char(',')) + QStringLiteral(",Sheet1!$B$2:$B$4");
        QVector<ChartBlock> blocks;
        QVERIFY(chartBlocks(QStringLiteral("N"), 0, f, &blocks));
        QCOMPARE(blocksText(blocks), QStringLiteral("0:B2:B4"));
        QVERIFY2(f.nameLookups <= 2, qPrintable(QString::number(f.nameLookups)));
        // A lists B_0 to B_79, each of which lists C_0 to C_79, none of which is defined: 6,400 names if all were looked at
        QStringList bs, cs;
        for (int i = 0; i < 80; ++i) {
            bs << QStringLiteral("B_%1").arg(i);
            cs << QStringLiteral("C_%1").arg(i);
        }
        f.bookNames[QStringLiteral("A")] = bs.join(QLatin1Char(','));
        for (int i = 0; i < 80; ++i) f.bookNames[QStringLiteral("B_%1").arg(i)] = cs.join(QLatin1Char(','));
        f.nameLookups = 0;
        blocks.clear();
        QVERIFY(!chartBlocks(QStringLiteral("A"), 0, f, &blocks));
        QVERIFY2(f.nameLookups <= kMaxChartVisits, qPrintable(QString::number(f.nameLookups)));
        QVERIFY(f.nameLookups > 80);
        // the same through a chart: its reads, and the cells asked for
        f.nameLookups = 0;
        Chart c = readIt(chartPart(QStringLiteral("barChart"), {SeriesSpec{QStringLiteral("N"), QStringLiteral("A"), QStringLiteral("N"), QStringLiteral("n"), {"a"}, {1}, QString()}}));
        bool cut = false;
        chartReads(c, 0, f, &cut);
        applyChartCells(&c, 0, f);
        QVERIFY2(f.nameLookups <= 4 * kMaxChartVisits, qPrintable(QString::number(f.nameLookups)));
    }

    // Values are read as numbers, with no text made of them (a series of 10,000 numbers has no labels); names and categories do have theirs.
    void valuesAreReadWithoutTheirText()
    {
        FakeCells f = FakeCells::regional();
        ChartRefData d;
        f.textCalls = 0;
        QVERIFY(readChartRef(QStringLiteral("Sheet1!$B$2:$C$4"), 0, f, 100, &d, false));
        QCOMPARE(int(d.points.size()), 6);
        QCOMPARE(f.textCalls, qint64(0));
        QVERIFY(d.points[0].isNumber && d.points[0].text.isEmpty());
        QVERIFY(readChartRef(QStringLiteral("Sheet1!$B$2:$C$4"), 0, f, 100, &d));
        QCOMPARE(f.textCalls, qint64(6));
        f.textCalls = 0;
        Chart c = readIt(chartPart(QStringLiteral("barChart"), eastWest()));
        applyChartCells(&c, 0, f);
        // two names (one cell each) and two columns of three categories: no text for the six values
        QCOMPARE(f.textCalls, qint64(2 + 6));
    }

    // Which cells a chart reads is found without going over its blocks: the index agrees with asking each block, for blocks of any shape; one cell asked of two thousand stacked blocks costs
    // a few probes; and a chart that reads more blocks than the limit is cut at it.
    void whatAChartReadsIsIndexedAndCapped()
    {
        QRandomGenerator rng(2026);
        QVector<ChartBlock> blocks;
        for (int i = 0; i < 1500; ++i) {
            ChartBlock b;
            b.sheet = int(rng.bounded(3));
            const quint32 r = rng.bounded(5000), c = rng.bounded(40);
            b.range = CellRange{r, c, r + rng.bounded(60), c + rng.bounded(5), true};
            blocks.push_back(b);
        }
        const ChartReadIndex index(blocks);
        QCOMPARE(index.blockCount(), 1500);
        qint64 hits = 0;
        for (int i = 0; i < 20000; ++i) {
            const int sheet = int(rng.bounded(4));
            const quint32 row = rng.bounded(5100), col = rng.bounded(50);
            bool want = false;
            for (const ChartBlock &b : std::as_const(blocks))
                if (b.sheet == sheet && b.range.contains(row, col)) want = true;
            QCOMPARE(index.reads(sheet, row, col), want);
            hits += want;
        }
        QVERIFY(hits > 1000);
        QVERIFY(!index.readsSheet(3));
        QVERIFY(index.readsSheet(0) && index.hull(0).valid);
        // two thousand blocks of one row each, one under another: a cell is found in a few probes
        QVector<ChartBlock> stack;
        for (quint32 r = 0; r < 2000; ++r) stack.push_back(ChartBlock{0, CellRange{r, 0, r, 3, true}});
        const ChartReadIndex tall(stack);
        QVERIFY(tall.reads(0, 1234, 2));
        QVERIFY(!tall.reads(0, 1234, 9));
        QVERIFY2(tall.probes() <= 8, qPrintable(QString::number(tall.probes())));
        // a cell inside the hull that no block holds (the rows' blocks alternate between two groups of columns): it is found not read without going back over every block above it
        QVector<ChartBlock> apart;
        for (quint32 r = 0; r < 2000; ++r) apart.push_back(ChartBlock{0, CellRange{r, r % 2 ? 10u : 0u, r, r % 2 ? 13u : 3u, true}});
        const ChartReadIndex spread(apart);
        QVERIFY(!spread.reads(0, 1234, 5));
        QVERIFY(spread.reads(0, 1234, 2) && spread.reads(0, 1235, 12));
        QVERIFY2(spread.probes() <= 12, qPrintable(QString::number(spread.probes())));
        // a chart of many series that each read hundreds of areas: its reads are cut at the limit, and the index still covers what was cut
        FakeCells f = FakeCells::regional();
        QStringList areas;
        for (int i = 0; i < 300; ++i) areas << QStringLiteral("Sheet1!$B$%1").arg(2 + i % 3);
        const QString many = QStringLiteral("(") + areas.join(QLatin1Char(',')) + QStringLiteral(")");
        QVector<SeriesSpec> series;
        for (int i = 0; i < 20; ++i) series.push_back(SeriesSpec{QString(), many, many, QStringLiteral("S"), {"a"}, {1}, QString()});
        Chart c = readIt(chartPart(QStringLiteral("lineChart"), series));
        bool cut = false;
        const QVector<ChartBlock> reads = chartReads(c, 0, f, &cut);
        QVERIFY(cut);
        QCOMPARE(int(reads.size()), kMaxChartReadBlocks);
        QVERIFY(ChartReadIndex(reads, cut).reads(0, 3, 1));
    }

    // Numbers as categories (dates) are shown through the category axis's own format when it is not linked to the source's, else as the source shows them: in the part, and from the cells.
    void dateCategoriesFollowTheAxisFormat()
    {
        const SeriesSpec spec{QString(), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("S"), {"45366", "45397", "45427"}, {1, 2, 3}, QString(), true, QStringLiteral("m/d/yyyy")};
        // from the numbers the part stores
        const Chart own = readIt(chartPart(QStringLiteral("barChart"), {spec}, QString(), true, QStringLiteral("gap"), QStringLiteral("mmm-yy"), false));
        QCOMPARE(own.plots[0].series[0].categories, (QStringList{"Mar-24", "Apr-24", "May-24"}));
        QCOMPARE(own.plots[0].series[0].categoryNumbers, (QVector<double>{45366, 45397, 45427}));
        QCOMPARE(own.plots[0].series[0].categoryFormat, QStringLiteral("m/d/yyyy"));
        const Chart linked = readIt(chartPart(QStringLiteral("barChart"), {spec}, QString(), true, QStringLiteral("gap"), QStringLiteral("mmm-yy"), true));
        QCOMPARE(linked.plots[0].series[0].categories, (QStringList{"3/15/2024", "4/15/2024", "5/15/2024"}));
        QCOMPARE(readIt(chartPart(QStringLiteral("barChart"), {spec})).plots[0].series[0].categories, (QStringList{"3/15/2024", "4/15/2024", "5/15/2024"}));   // (an axis that says nothing)
        // from the cells: their text is in the format of the cell
        FakeCells f = FakeCells::regional();
        f.set(0, QStringLiteral("A2"), Value::number(45366));
        f.set(0, QStringLiteral("A3"), Value::number(45397));
        f.set(0, QStringLiteral("A4"), Value::number(45427));
        f.sheets[0].texts[{1, 0}] = QStringLiteral("15 Mar 2024");
        f.sheets[0].texts[{2, 0}] = QStringLiteral("15 Apr 2024");
        f.sheets[0].texts[{3, 0}] = QStringLiteral("15 May 2024");
        Chart a = own, b = linked;
        applyChartCells(&a, 0, f);
        applyChartCells(&b, 0, f);
        QCOMPARE(a.plots[0].series[0].categories, (QStringList{"Mar-24", "Apr-24", "May-24"}));
        QCOMPARE(b.plots[0].series[0].categories, (QStringList{"15 Mar 2024", "15 Apr 2024", "15 May 2024"}));
        QCOMPARE(a.plots[0].series[0].categoryNumbers, (QVector<double>{45366, 45397, 45427}));
        // a text among the numbers is shown as the text it is
        f.set(0, QStringLiteral("A3"), Value::text(QStringLiteral("n/a")));
        f.sheets[0].texts.remove({2, 0});
        applyChartCells(&a, 0, f);
        QCOMPARE(a.plots[0].series[0].categories.value(1), QStringLiteral("n/a"));
        QVERIFY(a.plots[0].series[0].categoryNumbers.isEmpty());
    }

    // A title linked to a cell, and a title named for the only series, follow the cells.
    void titlesFollow()
    {
        FakeCells f = FakeCells::regional();
        const QString linked = QStringLiteral("<c:title><c:tx><c:strRef><c:f>Sheet1!$A$1</c:f><c:strCache><c:ptCount val=\"1\"/><c:pt idx=\"0\"><c:v>old</c:v></c:pt></c:strCache></c:strRef></c:tx></c:title>");
        QByteArray part = chartPart(QStringLiteral("barChart"), {eastWest()[0]});
        part.replace("<c:autoTitleDeleted val=\"1\"/>", linked.toUtf8());
        Chart c = readIt(part);
        QCOMPARE(c.title.ref, QStringLiteral("Sheet1!$A$1"));
        QCOMPARE(c.title.text, QStringLiteral("old"));
        applyChartCells(&c, 0, f);
        QCOMPARE(c.title.text, QStringLiteral("Region"));
        // a title with no text of its own is named for the series
        QByteArray autoPart = chartPart(QStringLiteral("barChart"), {eastWest()[0]});
        autoPart.replace("<c:autoTitleDeleted val=\"1\"/>", "<c:title><c:overlay val=\"0\"/></c:title>");
        Chart a = readIt(autoPart);
        QVERIFY(a.title.automatic);
        QCOMPARE(a.title.text, QStringLiteral("East"));
        f.set(0, QStringLiteral("B1"), Value::text(QStringLiteral("Eastern")));
        applyChartCells(&a, 0, f);
        QCOMPARE(a.title.text, QStringLiteral("Eastern"));
    }

    // The cells a chart reads: every block of every series, and its linked title.
    void chartReadsItsCells()
    {
        FakeCells f = FakeCells::regional();
        const Chart c = readIt(chartPart(QStringLiteral("barChart"), eastWest()));
        const QVector<ChartBlock> reads = chartReads(c, 0, f);
        QStringList text;
        for (const ChartBlock &b : reads) text << blocksText({b});
        text.sort();
        QCOMPARE(text, (QStringList{"0:A2:A4", "0:A2:A4", "0:B1", "0:B2:B4", "0:C1", "0:C2:C4"}));
        QVERIFY(chartReads(readIt(chartPart(QStringLiteral("barChart"), {SeriesSpec{QString(), QString(), QString(), QStringLiteral("Lit"), {"p"}, {3}, QString()}})), 0, f).isEmpty());
    }

    // Bounds: a whole column is read as far as the sheet's cells go; a range of a million rows is cut at the limit; a chart reads at most its limit in all; and the work is counted.
    void readsAreBounded()
    {
        FakeCells f = FakeCells::regional();
        // a whole column
        ChartRefData d;
        f.reads = 0;
        QVERIFY(readChartRef(QStringLiteral("Sheet1!$B:$B"), 0, f, kMaxChartSeriesCells, &d));
        QCOMPARE(int(d.points.size()), 4);
        QVERIFY(!d.cut);
        QCOMPARE(f.reads, qint64(4));
        // a long range of cells that are mostly empty and beyond the sheet's cells
        f.reads = 0;
        QVERIFY(readChartRef(QStringLiteral("Sheet1!$B$1:$B$1048576"), 0, f, kMaxChartSeriesCells, &d));
        QVERIFY(d.points.size() <= 4);   // (the rows beyond every cell of the sheet are cut, as for a whole column)
        QVERIFY(f.reads <= kMaxChartSeriesCells);
        // a range with more cells than the limit, all there
        FakeCells big;
        for (int i = 0; i < kMaxChartSeriesCells + 500; ++i) big.set(0, QStringLiteral("A%1").arg(i + 1), Value::number(i));
        big.reads = 0;
        QVERIFY(readChartRef(QStringLiteral("Sheet1!$A$1:$A$20000"), 0, big, kMaxChartSeriesCells, &d));
        QCOMPARE(int(d.points.size()), kMaxChartSeriesCells);
        QVERIFY(d.cut);
        QVERIFY(big.reads <= kMaxChartSeriesCells);
        // a chart of many series reads at most kMaxChartCells in all
        QVector<SeriesSpec> series;
        for (int i = 0; i < 30; ++i) series.push_back(SeriesSpec{QString(), QString(), QStringLiteral("Sheet1!$A$1:$A$%1").arg(kMaxChartSeriesCells + 400), QString(), {}, {1}, QString()});
        Chart c = readIt(chartPart(QStringLiteral("lineChart"), series));
        big.reads = 0;
        applyChartCells(&c, 0, big);
        QVERIFY2(big.reads <= kMaxChartCells, qPrintable(QString::number(big.reads)));
        qint64 points = 0;
        for (const ChartSeries &s : c.plots[0].series) points += s.values.size();
        QVERIFY(points <= kMaxChartCells);
    }
};

JP_TEST_CLASS(SheetChartDataTests)

// The cells of a real workbook: the values the file stored, the results the calculation told, the formats of the cells, hidden rows and columns, and names.
class GridChartCellsTests : public QObject {
    Q_OBJECT

    struct Loaded {
        Package package;
        Workbook book;
        std::unique_ptr<WorkbookSource> source;
        Chart chart;
    };
    static bool load(Loaded *l, const QByteArray &xlsx, QString *why)
    {
        if (!l->package.readLazy(xlsx, why) || !l->book.open(l->package, why)) return false;
        l->source = std::make_unique<WorkbookSource>(l->book);
        if (!l->source->loadAll()) return false;
        Theme theme;
        return readChart(l->package.part(QStringLiteral("/xl/charts/chart1.xml")), theme, ColorMap(), &l->chart, why);
    }
    static QByteArray book(const QByteArray &sheet, const QVector<SeriesSpec> &series, bool plotVisOnly = true, const QString &names = QString(), const QByteArray &styles = QByteArray())
    {
        return bookWithDrawing({sheet}, drawingPart(twoCell(0, 5, 4, 15, chartFrame(2, QStringLiteral("rIdC")))), {chartTarget(QStringLiteral("rIdC"), 1, chartPart(QStringLiteral("barChart"), series, QString(), plotVisOnly))},
                               QStringList(), names, styles);
    }

private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    // A chart in a real workbook: the names and values are those of the cells, a formula's is the result the file stored until the window is told another, a date category shows as its
    // format makes it, hidden rows and columns are left out, and a defined name stands for its cells.
    void aWorkbooksChartReadsItsCells()
    {
        const QByteArray styles = stylesXml(QStringLiteral("<numFmt numFmtId=\"164\" formatCode=\"mmm d, yyyy\"/>"), QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"164\"/>"));
        auto text = [](const QString &ref, const QString &t) { return QStringLiteral("<c r=\"%1\" t=\"inlineStr\"><is><t>%2</t></is></c>").arg(ref, t); };
        const QString rows = QStringLiteral("<row r=\"1\">") + text("A1", "When") + text("B1", "Sales") + QStringLiteral("</row>")
                             + QStringLiteral("<row r=\"2\"><c r=\"A2\" s=\"1\"><v>45366</v></c><c r=\"B2\"><v>4</v></c></row>")
                             + QStringLiteral("<row r=\"3\" hidden=\"1\"><c r=\"A3\" s=\"1\"><v>45367</v></c><c r=\"B3\"><v>5</v></c></row>")
                             + QStringLiteral("<row r=\"4\"><c r=\"A4\" s=\"1\"><v>45368</v></c><c r=\"B4\"><f>B2*10</f><v>1</v></c></row>");
        Loaded l;
        QString why;
        const QVector<SeriesSpec> series = {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sales"), QStringLiteral("old"), {"x", "y", "z"}, {9, 9, 9}, QString()}};
        QVERIFY2(load(&l, book(worksheet(rows), series, true, QStringLiteral("<definedName name=\"Sales\">Sheet1!$B$2:$B$4</definedName>"), styles), &why), qPrintable(why));
        jp::grid::ValueStore store;
        jp::grid::WorkbookChartCells cells(l.source.get(), &store, &l.book.styles(), l.book.info().dateSystem);
        QCOMPARE(cells.sheetCount(), 1);
        QCOMPARE(cells.sheetIndex(QStringLiteral("sheet1")), 0);
        QVERIFY(cells.rowHidden(0, 2) && !cells.rowHidden(0, 1));
        QCOMPARE(cells.text(0, 1, 0), QStringLiteral("Mar 15, 2024"));
        QCOMPARE(cells.text(0, 0, 1), QStringLiteral("Sales"));
        QCOMPARE(cells.value(0, 3, 1).number(), 1.0);   // (the result the file stored)
        QCOMPARE(cells.definedName(QStringLiteral("sales"), 0), QStringLiteral("Sheet1!$B$2:$B$4"));
        QVERIFY(cells.value(0, 9, 9).isEmpty());
        QVERIFY(cells.text(0, 9, 9).isEmpty());
        applyChartCells(&l.chart, 0, cells);
        const ChartSeries &s = l.chart.plots[0].series[0];
        QCOMPARE(s.name, QStringLiteral("Sales"));
        QCOMPARE(s.categories, (QStringList{"Mar 15, 2024", "Mar 17, 2024"}));
        QVERIFY(sameNumbers(s.values, {4, 1}));
        // the calculation tells the window another result for the formula: the chart has it
        store.apply({{0, 3, 1, Value::number(40)}});
        applyChartCells(&l.chart, 0, cells);
        QVERIFY(sameNumbers(l.chart.plots[0].series[0].values, {4, 40}));
        // a row is shown again, and a column hidden
        QVERIFY(chartReads(l.chart, 0, cells).size() == 3);
    }

    // A column hidden by the file, and one the sheet's own formats: the runs of columns are read.
    void hiddenColumnsAreRead()
    {
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><v>2</v></c><c r=\"C1\"><v>3</v></c><c r=\"D1\"><v>4</v></c></row>");
        Loaded l;
        QString why;
        QVERIFY2(load(&l, book(worksheet(rows, QString(), QStringLiteral("<cols><col min=\"2\" max=\"3\" width=\"9\" hidden=\"1\" customWidth=\"1\"/></cols>")),
                                {SeriesSpec{QString(), QString(), QStringLiteral("Sheet1!$A$1:$D$1"), QString(), {}, {0}, QString()}}), &why), qPrintable(why));
        jp::grid::WorkbookChartCells cells(l.source.get(), nullptr, &l.book.styles(), l.book.info().dateSystem);
        QVERIFY(!cells.columnHidden(0, 0) && cells.columnHidden(0, 1) && cells.columnHidden(0, 2) && !cells.columnHidden(0, 3));
        applyChartCells(&l.chart, 0, cells);
        QVERIFY(sameNumbers(l.chart.plots[0].series[0].values, {1, 4}));
        l.chart.plotVisibleOnly = false;
        applyChartCells(&l.chart, 0, cells);
        QVERIFY(sameNumbers(l.chart.plots[0].series[0].values, {1, 2, 3, 4}));
    }
};

JP_TEST_CLASS(GridChartCellsTests)
#include "tst_sheet_chartdata.moc"
