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

    // The database functions read a table whose first row names its fields, and a block of criteria (the same names in its first row, a condition under each, the rows of
    // conditions joined by "or", the conditions of a row by "and"): a text with no operator in it matches the texts that begin with it.
    void databaseFunctionsReadTheRecordsThatMeetTheCriteria()
    {
        Grid g;
        g.row("A5", {QString("Tree"), QString("Height"), QString("Age"), QString("Yield"), QString("Profit")});
        g.row("A6", {QString("Apple"), 18, 20, 14, 105});
        g.row("A7", {QString("Pear"), 12, 12, 10, 96});
        g.row("A8", {QString("Cherry"), 13, 14, 9, 105});
        g.row("A9", {QString("Apple"), 14, 15, 10, 75});
        g.row("A10", {QString("Pear"), 9, 8, 8, 76.8});
        g.row("A11", {QString("Apple"), 8, 9, 6, 45});
        // criteria: A1:A3 "Tree" over "=Apple" and "=Pear" (or); A1:B2 over "=Apple" and ">10" (and); H1:H2 "Tree" over a prefix; J1:J2 "Tree" over Cherry; L1:L2 "Tree" over nothing
        g.row("A1", {QString("Tree"), QString("Height"), QString("Height")});
        g.s("A2", "=Apple").s("B2", ">10").s("C2", "<16");
        g.s("A3", "=Pear");
        g.s("H1", "Tree").s("H2", "App");
        g.s("J1", "Tree").s("J2", "Cherry");
        g.s("L1", "Tree").s("N1", "Tree").s("N2", "=App").s("P1", "Tree").s("P2", "x*");
        const E cases[] = {
            {"DSUM(A5:E11,\"Profit\",A1:A3)", "n:397.8"},                    // the apple trees and the pear trees
            {"DSUM(A5:E11,5,A1:B2)", "n:180"},                                // apple trees over 10 high: 105 and 75
            {"DSUM(A5:E11,\"profit\",A1:B2)", "n:180"},                      // a field's name without regard to case
            {"DCOUNT(A5:E11,\"Age\",A1:C2)", "n:1"},                         // an apple tree between 10 and 16 high
            {"DCOUNT(A5:E11,\"Tree\",A1:A3)", "n:0"},                        // numbers only
            {"DCOUNTA(A5:E11,\"Tree\",A1:A2)", "n:3"},
            {"DAVERAGE(A5:E11,\"Yield\",A1:B2)", "n:12"},
            {"DAVERAGE(A5:E11,\"Tree\",A1:B2)", "e:#DIV/0!"},
            {"DMAX(A5:E11,\"Profit\",A1:A3)", "n:105"},
            {"DMIN(A5:E11,\"Profit\",A1:A3)", "n:45"},
            {"DGET(A5:E11,\"Yield\",J1:J2)", "n:9"},
            {"DGET(A5:E11,\"Yield\",A1:A2)", "e:#NUM!"},                     // three apple trees
            {"DGET(A5:E11,\"Yield\",P1:P2)", "e:#VALUE!"},                   // none
            {"DSUM(A5:E11,\"Profit\",H1:H2)", "n:225"},                      // "App" matches Apple, three times
            {"DSUM(A5:E11,\"Profit\",N1:N2)", "n:0"},                        // "=App" is exactly App
            {"DSUM(A5:E11,\"Profit\",L1:L2)", "n:502.8"},                    // no condition under the name: every record
            {"DSUM(A5:E11,\"Profit\",P1:P2)", "n:0"},
            {"DPRODUCT(A5:E11,\"Yield\",A1:B2)", "n:140"},
            {"DSTDEV(A5:E11,\"Yield\",H1:H2)", "n:4"},                       // 14, 10, 6
            {"ROUND(DSTDEVP(A5:E11,\"Yield\",H1:H2),6)", "n:3.265986"},
            {"DVAR(A5:E11,\"Yield\",H1:H2)", "n:16"},
            {"ROUND(DVARP(A5:E11,\"Yield\",H1:H2),4)", "n:10.6667"},
            {"DSUM(A5:E11,\"Weight\",A1:A3)", "e:#VALUE!"},                  // no such field
            {"DSUM(A5:E11,0,A1:A3)", "e:#VALUE!"},
            {"DSUM(A5:E11,6,A1:A3)", "e:#VALUE!"},
            {"DSUM(A5:E11,\"Profit\",5)", "e:#VALUE!"},
            {"DSUM(A5,\"Profit\",A1:A3)", "e:#VALUE!"},
            {"DSUM(A5:E11,2.9,A1:A3)", "n:61"},                               // the second field (the number is cut to a whole one): the heights of the apple and pear trees
        };
        CHECK_EVALS(cases, g, Settings());
    }

    // A table of many fields is worked out in time in proportion to its fields: the name of the field and of each column of the criteria is looked for in one pass over the
    // table's first row, and the steps are counted (a table of sixteen thousand fields, with criteria that name every one of them, took seventeen seconds, and counted ten).
    void aDatabaseOfManyFieldsIsWorkedOutInTimeInProportionToItsFields()
    {
        Grid g;
        const int fields = 16384;
        auto column = [](int c) {
            QString name;
            for (int n = c + 1; n > 0; n = (n - 1) / 26) name.prepend(QChar(QLatin1Char('A' + (n - 1) % 26)));
            return name;
        };
        for (int c = 0; c < fields; ++c) {
            g.s(column(c) + "1", QStringLiteral("h%1").arg(c));
            g.n(column(c) + "2", 1);
        }
        QElapsedTimer clock;
        clock.start();
        const Outcome o = evOutcome("DSUM(A1:XFD3,1,A1:XFD3)", g);
        QVERIFY2(o.text == QStringLiteral("n:1"), qPrintable(said(o)));
        QVERIFY2(o.work >= fields && o.work <= 20 * qint64(fields), qPrintable(said(o)));
        QVERIFY2(clock.elapsed() < 5000, qPrintable(QStringLiteral("%1 ms").arg(clock.elapsed())));
        // two fields of one name: the first is the field
        Grid d;
        d.row("A1", {QString("x"), QString("X"), QString("y")});
        d.row("A2", {1, 2, 3});
        d.row("A3", {4, 5, 6});
        d.s("E1", "X").s("E2", ">1");
        QCOMPARE(ev("DSUM(A1:C3,\"x\",E1:E2)", d), QStringLiteral("n:4"));
        QCOMPARE(ev("DSUM(A1:C3,\"Y\",E1:E2)", d), QStringLiteral("n:6"));
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
