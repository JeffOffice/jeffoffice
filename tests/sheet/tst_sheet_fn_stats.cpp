// Tests of the statistics of two series, percentiles, the conditional sums and counts with more than one condition, and the functions that choose by conditions.
// Each expected value is worked out by hand from the definition of the function (the series 1 2 3 4 and 2 4 6 9 give a slope of 2.3, an intercept of -0.5, ...).
#include "calchelpers.h"

using namespace jp::calc;

namespace {

struct E {
    const char *formula;
    const char *value;
};

#define CHECK_EVALS(cases, grid, settings)                                                                                                                  \
    for (const auto &c : cases) {                                                                                                                           \
        const QString got = ev(QString::fromUtf8(c.formula), grid, settings);                                                                               \
        QVERIFY2(got == QString::fromUtf8(c.value), qPrintable(QString::fromUtf8(c.formula) + QStringLiteral(" gave [") + got + QStringLiteral("], expected [") \
                                                                   + QString::fromUtf8(c.value) + QLatin1Char(']')));                                      \
    }

// A1:A4 = 1 2 3 4; B1:B4 = 2 4 6 9; D1:D5 = apple banana apple cherry Apple; E1:E5 = 1..5; G1:G5 = x y x x y
Grid sample()
{
    Grid g;
    g.column("A1", {1, 2, 3, 4});
    g.column("B1", {2, 4, 6, 9});
    g.column("D1", {QString("apple"), QString("banana"), QString("apple"), QString("cherry"), QString("Apple")});
    g.column("E1", {1, 2, 3, 4, 5});
    g.column("G1", {QString("x"), QString("y"), QString("x"), QString("x"), QString("y")});
    return g;
}

} // namespace

class SheetFnStatsTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void twoSeries()
    {
        const E cases[] = {
            {"ROUND(CORREL(A1:A4,B1:B4),8)", "n:0.99437671"},
            {"ROUND(PEARSON(A1:A4,B1:B4),8)", "n:0.99437671"},
            {"SLOPE(B1:B4,A1:A4)", "n:2.3"},
            {"INTERCEPT(B1:B4,A1:A4)", "n:-0.5"},
            {"ROUND(RSQ(B1:B4,A1:A4),8)", "n:0.98878505"},
            {"FORECAST(5,B1:B4,A1:A4)", "n:11"},
            {"COVAR(A1:A4,B1:B4)", "n:2.875"},                  // the mean of the products of the deviations
            {"CORREL(A1:A4,B1:B3)", "e:#N/A"},                   // series of different lengths
            {"CORREL(A1:A1,B1:B1)", "e:#DIV/0!"},
            {"SLOPE(A1:A1,A1:A1)", "e:#DIV/0!"},
            {"PERCENTILE(A1:A4,0.5)", "n:2.5"},
            {"PERCENTILE(A1:A4,0.25)", "n:1.75"},
            {"PERCENTILE(A1:A4,0)", "n:1"},
            {"PERCENTILE(A1:A4,1)", "n:4"},
            {"PERCENTILE(A1:A4,1.5)", "e:#NUM!"},
            {"PERCENTILE(G1:G5,0.5)", "e:#NUM!"},
            {"QUARTILE(E1:E5,1)", "n:2"},
            {"QUARTILE(E1:E5,2)", "n:3"},
            {"QUARTILE(E1:E5,4)", "n:5"},
            {"QUARTILE(E1:E5,5)", "e:#NUM!"},
            {"MODE({1,2,2,3,3})", "n:2"},                         // the first of those that come most often
            {"MODE({1,2,3})", "e:#N/A"},
            {"GCD(12,18)", "n:6"},
            {"LCM(4,6)", "n:12"},
            {"COMBIN(5,2)", "n:10"},
            {"PERMUT(5,2)", "n:20"},
            {"MROUND(10,3)", "n:9"},
            {"MROUND(-10,-3)", "n:-9"},
            {"MROUND(5,-2)", "e:#NUM!"},
        };
        CHECK_EVALS(cases, sample(), Settings());
    }

    void conditionsOnSeveralRanges()
    {
        const E cases[] = {
            {"SUMIFS(E1:E5,D1:D5,\"apple\",G1:G5,\"x\")", "n:4"},
            {"SUMIFS(E1:E5,D1:D5,\"apple\")", "n:9"},
            {"SUMIFS(E1:E5,E1:E5,\">1\",E1:E5,\"<5\")", "n:9"},
            {"SUMIFS(E1:E5,D1:D5,\"apple\",G1:G4,\"x\")", "e:#VALUE!"},       // ranges of different shapes
            {"COUNTIFS(D1:D5,\"apple\",G1:G5,\"y\")", "n:1"},
            {"COUNTIFS(E1:E5,\">2\",E1:E5,\"<5\")", "n:2"},
            {"COUNTIFS(D1:D5,\"apple\")", "n:3"},
            {"COUNTIFS(D1:D5,\"<>apple\",G1:G5,\"x\")", "n:1"},
            {"COUNTIFS(D1:D5,\"zzz\",G1:G5,\"x\")", "n:0"},
            {"AVERAGEIF(D1:D5,\"apple\",E1:E5)", "n:3"},
            {"AVERAGEIF(E1:E5,\">2\")", "n:4"},
            {"AVERAGEIF(D1:D5,\"zzz\",E1:E5)", "e:#DIV/0!"},
            {"AVERAGEIFS(E1:E5,D1:D5,\"apple\",G1:G5,\"x\")", "n:2"},
            {"_xlfn.MAXIFS(E1:E5,D1:D5,\"apple\")", "n:5"},
            {"_xlfn.MINIFS(E1:E5,D1:D5,\"apple\")", "n:1"},
            {"_xlfn.MAXIFS(E1:E5,D1:D5,\"zzz\")", "n:0"},
            {"_xlfn.IFS(FALSE,1,TRUE,2)", "n:2"},
            {"_xlfn.IFS(1>2,\"a\",2>1,\"b\",TRUE,\"c\")", "s:b"},
            {"_xlfn.IFS(FALSE,1)", "e:#N/A"},
            {"_xlfn.SWITCH(2,1,\"a\",2,\"b\",\"z\")", "s:b"},
            {"_xlfn.SWITCH(9,1,\"a\",\"z\")", "s:z"},
            {"_xlfn.SWITCH(9,1,\"a\")", "e:#N/A"},
            {"_xlfn.SWITCH(\"B\",\"a\",1,\"b\",2)", "n:2"},
        };
        CHECK_EVALS(cases, sample(), Settings());
    }
};

JP_TEST_CLASS(SheetFnStatsTests)
#include "tst_sheet_fn_stats.moc"
