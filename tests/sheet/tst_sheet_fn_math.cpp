// Tests of the number functions: sums and counts (what a range holds and what a typed argument holds are read differently), rounding, powers and logarithms,
// conditional sums, and the statistics the files use most. Each expected value is written from the function's own definition and the rules of the
// language (calc/value.h), not from what the code gave.
#include "calchelpers.h"

#include <cmath>

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

// A1:A3 = 1, 2, 3; B1:B3 = 10, 20, 30; C1:C4 = 1, "x", TRUE, empty; D1:D5 = apple, banana, apple, cherry, Apple; E1:E5 = 1..5; F1:F3 = 1, "1", 2
Grid sample()
{
    Grid g;
    g.column("A1", {1, 2, 3});
    g.column("B1", {10, 20, 30});
    g.column("C1", {1, QString("x"), true, QString()});
    g.column("D1", {QString("apple"), QString("banana"), QString("apple"), QString("cherry"), QString("Apple")});
    g.column("E1", {1, 2, 3, 4, 5});
    g.column("F1", {1, QString("1"), 2});
    return g;
}

} // namespace

class SheetFnMathTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void sumsReadARangeAndATypedArgumentDifferently()
    {
        const Grid g = sample();
        const E cases[] = {
            {"SUM(1,2,3)", "n:6"},
            {"SUM(A1:A3)", "n:6"},
            {"SUM(A1:A3,B1:B3)", "n:66"},
            {"SUM(\"2\",1)", "n:3"},               // a typed text that reads as a number is a number
            {"SUM(\"a\")", "e:#VALUE!"},
            {"SUM(TRUE,TRUE)", "n:2"},             // a typed boolean is 1 or 0
            {"SUM(C1:C4)", "n:1"},                 // in a range, text and booleans and empty cells are left out
            {"SUM({1,\"2\",TRUE})", "n:1"},        // and in an array
            {"SUM(1,G1)", "n:1"},
            {"SUM(A:A)", "n:6"},
            {"SUM((A1:A2,A3))", "n:6"},
            {"SUM(A1:A3)/COUNT(A1:A3)", "n:2"},
            {"SUM(1,,2)", "n:3"},                  // an argument left out is 0
            {"SUM(\"\")", "e:#VALUE!"},
            {"SUM(A1:A3,\"4\")", "n:10"},
            {"SUM(0.1,0.2,0.3)", "n:0.6"},
        };
        CHECK_EVALS(cases, g, Settings());
    }

    void maxMinAndAverage()
    {
        const Grid g = sample();
        const E cases[] = {
            {"MAX(A1:A3)", "n:3"},
            {"MAX(1,\"5\")", "n:5"},
            {"MAX(\"a\")", "e:#VALUE!"},
            {"MAX(G1:G3)", "n:0"},                 // nothing to take the greatest of
            {"MIN(G1:G3)", "n:0"},
            {"MAX(TRUE,0.5)", "n:1"},
            {"MAX(C1:C4)", "n:1"},
            {"MAX(A1:A3,10)", "n:10"},
            {"MAX({1,5,3})", "n:5"},
            {"MAX(-1,-5)", "n:-1"},
            {"MAX(#N/A,1)", "e:#N/A"},
            {"MIN(A1:A3)", "n:1"},
            {"MIN(B1:B3,5)", "n:5"},
            {"MIN({4,2,8})", "n:2"},
            {"MAXA(C1:C4)", "n:1"},                // a text counts as 0, TRUE as 1
            {"MINA(C1:C4)", "n:0"},
            {"AVERAGE(A1:A3)", "n:2"},
            {"AVERAGE(1,\"3\")", "n:2"},
            {"AVERAGE(1,TRUE)", "n:1"},
            {"AVERAGE(C1:C4)", "n:1"},
            {"AVERAGE(D1:D5)", "e:#DIV/0!"},        // no numbers at all
            {"AVERAGE(G1:G3)", "e:#DIV/0!"},
            {"AVERAGE(A1:A3,B1:B3)", "n:11"},
            {"AVERAGEA(C1:C4)", "n:0.666666666666667"},
            {"AVERAGE(#N/A)", "e:#N/A"},
        };
        CHECK_EVALS(cases, g, Settings());
    }

    void countsTellNumbersTextAndBlanksApart()
    {
        const Grid g = sample();
        const E cases[] = {
            {"COUNT(C1:C4)", "n:1"},
            {"COUNT(1,\"2\",TRUE,\"a\")", "n:3"},   // typed: numbers, text that reads as one, and booleans
            {"COUNT({1,2,\"a\"})", "n:2"},
            {"COUNT(#N/A,1)", "n:1"},               // an error is no number, and no error
            {"COUNT(A1:A3,B1:B3)", "n:6"},
            {"COUNT(G1:G9)", "n:0"},
            {"COUNTA(C1:C4)", "n:3"},
            {"COUNTA(D1:D5)", "n:5"},
            {"COUNTA(1,\"\",#N/A)", "n:3"},
            {"COUNTA(G1:G9)", "n:0"},
            {"COUNTBLANK(C1:C4)", "n:1"},
            {"COUNTBLANK(G1:G3)", "n:3"},
            {"COUNTBLANK(A1:A3)", "n:0"},
        };
        CHECK_EVALS(cases, g, Settings());
        Grid blanks;
        blanks.s("A1", "").n("A2", 1);
        QCOMPARE(ev("COUNTBLANK(A1:A3)", blanks), num(2));   // an empty text is blank, and so is a cell with nothing
        QCOMPARE(ev("COUNTA(A1:A3)", blanks), num(2));       // but it is not nothing for COUNTA
    }

    void roundingGoesAwayFromZeroOnTheFifteenDigits()
    {
        const E cases[] = {
            {"ROUND(2.5,0)", "n:3"},
            {"ROUND(-2.5,0)", "n:-3"},
            {"ROUND(2.675,2)", "n:2.68"},
            {"ROUND(1.005,2)", "n:1.01"},
            {"ROUND(2.345,2)", "n:2.35"},
            {"ROUND(-1.005,2)", "n:-1.01"},
            {"ROUND(0.285,2)", "n:0.29"},
            {"ROUND(5.55,1)", "n:5.6"},
            {"ROUND(1234.567,-2)", "n:1200"},
            {"ROUND(1234.5,-1)", "n:1230"},
            {"ROUND(5,-1)", "n:10"},
            {"ROUND(4,-1)", "n:0"},
            {"ROUND(15,-1)", "n:20"},
            {"ROUND(0.5,0)", "n:1"},
            {"ROUND(0.4999,0)", "n:0"},
            {"ROUND(1.23456,4)", "n:1.2346"},
            {"ROUND(\"1.5\",0)", "n:2"},
            {"ROUND(0.1+0.2,1)", "n:0.3"},
            {"ROUND(1E300,2)", "n:1e+300"},
            {"ROUND(\"a\",0)", "e:#VALUE!"},
            {"ROUND(2.5,0.9)", "n:3"},               // the number of places is cut to a whole number
            {"ROUND(1.2345,2.9)", "n:1.23"},
            {"ROUNDUP(1.01,1)", "n:1.1"},
            {"ROUNDUP(-1.01,1)", "n:-1.1"},
            {"ROUNDUP(1,0)", "n:1"},
            {"ROUNDUP(1.2,0)", "n:2"},
            {"ROUNDUP(123,-1)", "n:130"},
            {"ROUNDUP(0.1+0.2,1)", "n:0.3"},          // 0.30000000000000004 is 0.3 to 15 digits, and 0.3 is not rounded up
            {"ROUNDDOWN(-1.99,1)", "n:-1.9"},
            {"ROUNDDOWN(1.99,0)", "n:1"},
            {"ROUNDDOWN(123.9,-1)", "n:120"},
            {"INT(-1.5)", "n:-2"},
            {"INT(1.9)", "n:1"},
            {"INT(-0.5)", "n:-1"},
            {"INT(\"3.7\")", "n:3"},
            {"INT(TRUE)", "n:1"},
            {"TRUNC(-1.9)", "n:-1"},
            {"TRUNC(1.987,2)", "n:1.98"},
            {"TRUNC(-1.987,2)", "n:-1.98"},
            {"TRUNC(1234,-2)", "n:1200"},
            {"CEILING(2.1,1)", "n:3"},
            {"CEILING(4.2,0.5)", "n:4.5"},
            {"CEILING(-2.1,-1)", "n:-3"},
            {"CEILING(2.1,0)", "n:0"},
            {"CEILING(2.1,-1)", "e:#NUM!"},
            {"FLOOR(2.9,1)", "n:2"},
            {"FLOOR(4.2,0.5)", "n:4"},
            {"FLOOR(-2.1,-1)", "n:-2"},
            {"FLOOR(2.9,0)", "e:#DIV/0!"},
        };
        CHECK_EVALS(cases, Grid(), Settings());
    }

    // CEILING.MATH(number, [significance], [mode]) and FLOOR.MATH(number, [significance], [mode]) (written _xlfn.CEILING.MATH in a file) as the common program defines them: the significance is 1 when it is left out, its sign
    // is ignored, and 0 gives 0; a positive number goes up (CEILING.MATH) or down (FLOOR.MATH) to a multiple of it; a negative number goes toward plus infinity (CEILING.MATH) or away from
    // zero (FLOOR.MATH) unless the mode is not 0, which turns that round (away from zero for CEILING.MATH, toward zero for FLOOR.MATH); the mode means nothing for a positive number.
    void ceilingMathAndFloorMathFollowTheirDefinition()
    {
        const E cases[] = {
            {"_xlfn.CEILING.MATH(24.3,5)", "n:25"},
            {"_xlfn.CEILING.MATH(6.7)", "n:7"},
            {"_xlfn.CEILING.MATH(-8.1,2)", "n:-8"},
            {"_xlfn.CEILING.MATH(-8.1)", "n:-8"},
            {"_xlfn.CEILING.MATH(5,0)", "n:0"},
            {"_xlfn.CEILING.MATH(-5.5,2,-1)", "n:-6"},
            {"_xlfn.CEILING.MATH(-5.5,2,1)", "n:-6"},
            {"_xlfn.CEILING.MATH(-5.5,2,0)", "n:-4"},
            {"_xlfn.CEILING.MATH(-5.5,-2)", "n:-4"},
            {"_xlfn.CEILING.MATH(24.3,-5)", "n:25"},
            {"_xlfn.CEILING.MATH(6.7,1,-1)", "n:7"},
            {"_xlfn.CEILING.MATH(4.2,0.5)", "n:4.5"},
            {"_xlfn.CEILING.MATH(0.3,0.1)", "n:0.3"},
            {"_xlfn.CEILING.MATH(0)", "n:0"},
            {"_xlfn.CEILING.MATH(-0.3)", "n:0"},
            {"_xlfn.FLOOR.MATH(0.3)", "n:0"},
            {"_xlfn.FLOOR.MATH(-0.3,1,1)", "n:0"},
            {"_xlfn.CEILING.MATH(\"6.7\")", "n:7"},
            {"_xlfn.CEILING.MATH(TRUE)", "n:1"},
            {"_xlfn.CEILING.MATH(\"x\")", "e:#VALUE!"},
            {"_xlfn.CEILING.MATH(2.1,\"x\")", "e:#VALUE!"},
            {"_xlfn.ceiling.math(2.1)", "n:3"},
            {"CEILING.MATH(2.1)", "!unsupported: CEILING.MATH"},   // (a newer function is written with its _xlfn. in a file; a typed entry gets it)
            {"FLOOR.MATH(2.1)", "!unsupported: FLOOR.MATH"},
            {"_xlfn.CEILING.MATH(2.1)", "n:3"},
            {"_xlfn.FLOOR.MATH(24.3,5)", "n:20"},
            {"_xlfn.FLOOR.MATH(6.7)", "n:6"},
            {"_xlfn.FLOOR.MATH(-8.1,2)", "n:-10"},
            {"_xlfn.FLOOR.MATH(-8.1)", "n:-9"},
            {"_xlfn.FLOOR.MATH(5,0)", "n:0"},
            {"_xlfn.FLOOR.MATH(-5.5,2,-1)", "n:-4"},
            {"_xlfn.FLOOR.MATH(-5.5,2,1)", "n:-4"},
            {"_xlfn.FLOOR.MATH(-5.5,2,0)", "n:-6"},
            {"_xlfn.FLOOR.MATH(-5.5,-2)", "n:-6"},
            {"_xlfn.FLOOR.MATH(24.3,-5)", "n:20"},
            {"_xlfn.FLOOR.MATH(6.7,1,-1)", "n:6"},
            {"_xlfn.FLOOR.MATH(4.2,0.5)", "n:4"},
            {"_xlfn.FLOOR.MATH(0.7,0.1)", "n:0.7"},
            {"_xlfn.FLOOR.MATH(-0.5)", "n:-1"},
            {"_xlfn.FLOOR.MATH(0)", "n:0"},
            {"_xlfn.FLOOR.MATH(\"x\")", "e:#VALUE!"},
            {"_xlfn.FLOOR.MATH(6.7)", "n:6"},
        };
        CHECK_EVALS(cases, Grid(), Settings());
    }

    void powersLogarithmsAndSigns()
    {
        const E cases[] = {
            {"ABS(-3)", "n:3"},
            {"ABS(\"-3\")", "n:3"},
            {"ABS(\"x\")", "e:#VALUE!"},
            {"ABS(G9)", "n:0"},
            {"ABS(TRUE)", "n:1"},
            {"SQRT(16)", "n:4"},
            {"SQRT(-1)", "e:#NUM!"},
            {"SQRT(\"4\")", "n:2"},
            {"SQRT(0)", "n:0"},
            {"SQRT(2)", "n:1.4142135623731"},
            {"EXP(0)", "n:1"},
            {"EXP(1)", "n:2.71828182845905"},
            {"EXP(710)", "e:#NUM!"},
            {"EXP(-1000)", "n:0"},
            {"LN(1)", "n:0"},
            {"LN(0)", "e:#NUM!"},
            {"LN(-1)", "e:#NUM!"},
            {"LN(EXP(2))", "n:2"},
            {"LN(2.718281828459045)", "n:1"},
            {"LOG(100)", "n:2"},
            {"LOG(8,2)", "n:3"},
            {"LOG(100,10)", "n:2"},
            {"LOG(0)", "e:#NUM!"},
            {"LOG(10,1)", "e:#DIV/0!"},
            {"LOG10(1000)", "n:3"},
            {"POWER(2,10)", "n:1024"},
            {"POWER(2,-1)", "n:0.5"},
            {"POWER(0,0)", "e:#NUM!"},
            {"POWER(-8,1/3)", "e:#NUM!"},
            {"SIGN(-5)", "n:-1"},
            {"SIGN(0)", "n:0"},
            {"SIGN(3)", "n:1"},
            {"MOD(10,3)", "n:1"},
            {"MOD(-10,3)", "n:2"},                 // the sign is the divisor's
            {"MOD(10,-3)", "n:-2"},
            {"MOD(5,0)", "e:#DIV/0!"},
            {"MOD(5.5,2)", "n:1.5"},
            {"MOD(3,3)", "n:0"},
            {"MOD(\"7\",4)", "n:3"},
            {"PI()", "n:3.14159265358979"},
            {"PRODUCT(2,3,4)", "n:24"},
            {"PRODUCT(A1:A3)", "n:6"},
            {"PRODUCT(A1:A3,2)", "n:12"},
            {"PRODUCT(\"a\")", "e:#VALUE!"},
            {"PRODUCT(G1:G3)", "n:0"},
            {"SUMSQ(1,2,3)", "n:14"},
            {"SUMSQ(A1:A3)", "n:14"},
            {"QUOTIENT(7,2)", "n:3"},
            {"QUOTIENT(-7,2)", "n:-3"},
            {"QUOTIENT(7,0)", "e:#DIV/0!"},
            {"FACT(5)", "n:120"},
            {"FACT(0)", "n:1"},
            {"FACT(-1)", "e:#NUM!"},
            {"EVEN(3)", "n:4"},
            {"ODD(2)", "n:3"},
            {"EVEN(-1)", "n:-2"},
        };
        CHECK_EVALS(cases, sample(), Settings());
    }

    // A criterion that an empty cell meets ("", "=", "<>x") takes the cells beside the empty ones into a sum range too, though the empty ones are not stored.
    void criteriaThatEmptyCellsMeetTakeTheCellsBesideThem()
    {
        Grid g;
        g.s("A1", "x").s("A3", "y");
        g.column("B1", {10, 20, 30, 40});
        g.column("C1", {1, 2, 3, 4});
        const E cases[] = {
            {"SUMIF(A1:A4,\"<>x\",B1:B4)", "n:90"},
            {"SUMIF(A1:A4,\"\",B1:B4)", "n:60"},
            {"SUMIF(A1:A4,\"=\",B1:B4)", "n:60"},
            {"SUMIF(A1:A4,\"<>\",B1:B4)", "n:40"},
            {"SUMIF(A1:A4,\"y\",B1:B4)", "n:30"},
            {"SUMIF(A1:A4,\"<>q\",B1:B4)", "n:100"},
            {"SUMIF(A1:A4,\"<>x\",C1:C4)", "n:9"},
            {"SUMIF(A1:A4,\"\",C1:C4)", "n:6"},
            {"SUMIF(A1:A4,\"<>x\")", "n:0"},                   // (the cells it sums are the texts of A1:A4)
            {"SUMIF(A1:A2,\"\",B3:B4)", "n:40"},              // the sum range takes the shape of the criteria range, from its first cell: B3:B4
            {"SUMIF(A2:A4,\"=\",B1:B4)", "n:40"},             // (B1:B3 then: A2 and A4 are empty, and beside them are B1 and B3)
            {"COUNTIF(A1:A4,\"\")", "n:2"},
            {"COUNTIF(A1:A4,\"=\")", "n:2"},
            {"COUNTIF(A1:A4,\"<>x\")", "n:3"},
            {"COUNTIF(A1:A4,\"<>\")", "n:2"},
            {"AVERAGEIF(A1:A4,\"<>x\",B1:B4)", "n:30"},
            {"AVERAGEIF(A1:A4,\"\",B1:B4)", "n:30"},
            {"AVERAGEIF(A1:A4,\"=\")", "e:#DIV/0!"},
            {"SUMIFS(B1:B4,A1:A4,\"<>x\")", "n:90"},
            {"SUMIFS(B1:B4,A1:A4,\"\")", "n:60"},
            {"SUMIFS(B1:B4,A1:A4,\"=\")", "n:60"},
            {"SUMIFS(B1:B4,A1:A4,\"\",C1:C4,\">2\")", "n:40"},
            {"COUNTIFS(A1:A4,\"\")", "n:2"},
            {"COUNTIFS(A1:A4,\"<>x\",B1:B4,\">20\")", "n:2"},
            {"AVERAGEIFS(B1:B4,A1:A4,\"\")", "n:30"},
            {"_xlfn.MAXIFS(B1:B4,A1:A4,\"\")", "n:40"},
            {"_xlfn.MINIFS(B1:B4,A1:A4,\"<>x\")", "n:20"},
        };
        CHECK_EVALS(cases, g, Settings());
        // a cell that holds an empty text meets "" but not "="; the cell after it, empty, meets both
        Grid h;
        h.s("A1", "x").set("A2", Value::text("")).s("A3", "y");
        h.column("B1", {10, 20, 30, 40});
        const E texts[] = {
            {"SUMIF(A1:A4,\"\",B1:B4)", "n:60"},
            {"SUMIF(A1:A4,\"=\",B1:B4)", "n:40"},
            {"SUMIF(A1:A4,\"<>\",B1:B4)", "n:60"},
            {"COUNTIF(A1:A4,\"\")", "n:2"},
            {"COUNTIF(A1:A4,\"=\")", "n:1"},
        };
        CHECK_EVALS(texts, h, Settings());
        // an error in the sum range is the result only where the criterion is met
        Grid err;
        err.s("A1", "x").s("A3", "y");
        err.column("B1", {10, 20, 30, 40});
        err.e("B1", ErrorKind::Div0);
        QCOMPARE(ev("SUMIF(A1:A4,\"<>x\",B1:B4)", err), QStringLiteral("n:90"));
        QCOMPARE(ev("SUMIF(A1:A4,\"x\",B1:B4)", err), QStringLiteral("e:#DIV/0!"));
        QCOMPARE(ev("SUMIF(A1:A4,\"\",B1:B4)", err), QStringLiteral("n:60"));
    }

    // A least common multiple past what a number holds exactly (2^53) is #NUM!; it does not become infinite, and go on being looked for.
    void aLeastCommonMultipleThatPassesTheLimitIsAnError()
    {
        Settings array;
        array.array = true;
        const E cases[] = {
            {"LCM(4,6)", "n:12"},
            {"LCM(ROW(1:40))=5342931457063200", "b:1"},       // the least common multiple of 1 to 40
            {"LCM(ROW(1:41))", "e:#NUM!"},
            {"LCM(ROW(1:100))", "e:#NUM!"},
            {"LCM(2.5,ROW(1:1000),-1)", "e:#NUM!"},
            {"LCM(9007199254740991,9007199254740990)", "e:#NUM!"},
            {"LCM(9007199254740991,1)=9007199254740991", "b:1"},
            {"GCD(ROW(1:100))", "n:1"},
        };
        CHECK_EVALS(cases, Grid(), array);
    }

    void conditionalSumsAndCountsFollowTheirCriteria()
    {
        const Grid g = sample();
        const E cases[] = {
            {"SUMIF(D1:D5,\"apple\",E1:E5)", "n:9"},        // without regard to case
            {"SUMIF(D1:D5,\"a*\",E1:E5)", "n:9"},
            {"SUMIF(D1:D5,\"?anana\",E1:E5)", "n:2"},
            {"SUMIF(D1:D5,\"<>apple\",E1:E5)", "n:6"},
            {"SUMIF(E1:E5,\">2\")", "n:12"},
            {"SUMIF(E1:E5,\">=3\",E1:E5)", "n:12"},
            {"SUMIF(E1:E5,3)", "n:3"},
            {"SUMIF(E1:E5,\"<>3\")", "n:12"},
            {"SUMIF(E1:E5,\">\"&2)", "n:12"},
            {"SUMIF(D1:D3,\"apple\",E2:E2)", "n:6"},          // the sum range takes the shape of the criteria range, from its first cell: E2:E4
            {"SUMIF(E1:E5,\"<=2\")", "n:3"},
            {"SUMIF(E1:E5,\"=2\")", "n:2"},
            {"SUMIF(D1:D5,\"zzz\",E1:E5)", "n:0"},
            {"COUNTIF(D1:D5,\"apple\")", "n:3"},
            {"COUNTIF(E1:E5,\">2\")", "n:3"},
            {"COUNTIF(D1:D5,\"*\")", "n:5"},                   // every text
            {"COUNTIF(E1:E5,\"<>\")", "n:5"},
            {"COUNTIF(D1:D5,\"?pple\")", "n:3"},
            {"COUNTIF(F1:F3,\"1\")", "n:2"},                   // a number and a text that read the same
            {"COUNTIF(C1:C4,TRUE)", "n:1"},
            {"COUNTIF(E1:E5,\">\"&AVERAGE(E1:E5))", "n:2"},
            {"COUNTIF(E1:E5,\"=\")", "n:0"},
            {"COUNTIF(C1:C4,\"=\")", "n:1"},
            {"COUNTIF(D1:D5,\"b*\")", "n:1"},
            {"COUNTIF(D1:D5,\"~*\")", "n:0"},
            {"SUMPRODUCT(A1:A3,B1:B3)", "n:140"},
            {"SUMPRODUCT(A1:A3)", "n:6"},
            {"SUMPRODUCT(A1:A3*B1:B3)", "n:140"},
            {"SUMPRODUCT(--(D1:D5=\"apple\"))", "n:3"},
            {"SUMPRODUCT(A1:A3,C1:C3)", "n:1"},                // a text or a boolean in an array counts as 0
            {"SUMPRODUCT(A1:A3,B1:B2)", "e:#VALUE!"},          // arrays of different sizes
            {"SUMPRODUCT((E1:E5>2)*(E1:E5<5))", "n:2"},
            {"SUMPRODUCT({1,2,3},{4,5,6})", "n:32"},
            {"SUMPRODUCT(\"a\",2)", "e:#VALUE!"},            // a text that is one value, or the only cell of a range, is no array of zeros
            {"SUMPRODUCT(C2:C2,A2:A2)", "e:#VALUE!"},
            {"SUMPRODUCT(C1:C2,A1:A2)", "n:1"},
        };
        CHECK_EVALS(cases, g, Settings());
    }

    void subtotalsAndStatistics()
    {
        Grid g = sample();
        g.column("H1", {2, 4, 4, 4, 5, 5, 7, 9});
        const E cases[] = {
            {"SUBTOTAL(9,A1:A3)", "n:6"},
            {"SUBTOTAL(1,A1:A3)", "n:2"},
            {"SUBTOTAL(2,A1:A3)", "n:3"},
            {"SUBTOTAL(3,C1:C4)", "n:3"},
            {"SUBTOTAL(4,A1:A3)", "n:3"},
            {"SUBTOTAL(5,A1:A3)", "n:1"},
            {"SUBTOTAL(6,A1:A3)", "n:6"},
            {"SUBTOTAL(109,A1:A3)", "n:6"},
            {"SUBTOTAL(12,A1:A3)", "e:#VALUE!"},
            {"MEDIAN(1,2,3,10)", "n:2.5"},
            {"MEDIAN(A1:A3)", "n:2"},
            {"MEDIAN(G1:G3)", "e:#NUM!"},
            {"STDEV(H1:H8)", "n:2.1380899352994"},
            {"STDEVP(H1:H8)", "n:2"},
            {"VAR(H1:H8)", "n:4.57142857142857"},
            {"VARP(H1:H8)", "n:4"},
            {"STDEV(A1:A1)", "e:#DIV/0!"},
            {"LARGE(A1:A3,1)", "n:3"},
            {"LARGE(A1:A3,3)", "n:1"},
            {"LARGE(A1:A3,4)", "e:#NUM!"},
            {"SMALL(A1:A3,2)", "n:2"},
            {"RANK(2,A1:A3)", "n:2"},
            {"RANK(2,A1:A3,1)", "n:2"},
            {"RANK(3,A1:A3)", "n:1"},
            {"RANK(5,A1:A3)", "e:#N/A"},
        };
        CHECK_EVALS(cases, g, Settings());
    }

    // A sum of numbers that cancel is zero where the rounding error of the terms is all that is left of it (SUM does it at each step); a real small sum is kept.
    void sumsThatCancelAreZero()
    {
        Grid g;
        g.column("A1", {2.3, -1.5, -0.8});
        const E cases[] = {
            {"SUM(2.3,-1.5,-0.8)", "n:0"},
            {"SUM(0.1,0.2,-0.3)", "n:0"},
            {"SUM(A1:A3)", "n:0"},
            {"AVERAGE(A1:A3)", "n:0"},
            {"SUM(1,-1)", "n:0"},
            {"SUM(1E20,-1E20,1)", "n:1"},
            {"SUM(1000,-999.5)", "n:0.5"},
            {"SUM(1E-20,1)", "n:1"},
            {"SUM(1E-20,-1E-20)", "n:0"},
            {"SUM(1E-20,2E-20)", "n:3e-20"},
        };
        CHECK_EVALS(cases, g, Settings());
    }

    // SUBTOTAL leaves out the cells of its range that are SUBTOTALs themselves, so that a list with subtotals in it is not added twice.
    void subtotalLeavesOutSubtotals()
    {
        Grid g;
        g.column("A1", {1, 2, 10, 3});
        g.subtotal("A3");
        const E cases[] = {
            {"SUBTOTAL(9,A1:A4)", "n:6"},
            {"SUBTOTAL(109,A1:A4)", "n:6"},
            {"SUBTOTAL(1,A1:A4)", "n:2"},
            {"SUBTOTAL(2,A1:A4)", "n:3"},
            {"SUBTOTAL(3,A1:A4)", "n:3"},
            {"SUBTOTAL(4,A1:A4)", "n:3"},
            {"SUM(A1:A4)", "n:16"},                // SUM leaves out nothing
        };
        CHECK_EVALS(cases, g, Settings());
    }

    void randomNumbersAreInTheirRange()
    {
        const E cases[] = {
            {"AND(RAND()>=0,RAND()<1)", "b:1"},
            {"RAND()=RAND()", "b:0"},
            {"RANDBETWEEN(5,5)", "n:5"},
            {"AND(RANDBETWEEN(1,6)>=1,RANDBETWEEN(1,6)<=6)", "b:1"},
            {"RANDBETWEEN(6,1)", "e:#NUM!"},
        };
        for (int i = 0; i < 5; ++i) {
            QCOMPARE(ev(cases[0].formula), QString::fromLatin1(cases[0].value));
            QCOMPARE(ev(cases[2].formula), QString::fromLatin1(cases[2].value));
            QCOMPARE(ev(cases[3].formula), QString::fromLatin1(cases[3].value));
            QCOMPARE(ev(cases[4].formula), QString::fromLatin1(cases[4].value));
        }
        QCOMPARE(ev(cases[1].formula), QString::fromLatin1(cases[1].value));
    }

    void volatileFunctionsAreMarked()
    {
        for (const char *name : {"NOW", "TODAY", "RAND", "RANDBETWEEN", "OFFSET", "INDIRECT"})
            QVERIFY2(functionIndex(name) >= 0 && (functionAt(functionIndex(name)).flags & kFnVolatile), name);
        for (const char *name : {"SUM", "IF", "VLOOKUP", "ROUND", "DATE", "TEXT", "INDEX", "MATCH", "ROW", "SUMIF"})
            QVERIFY2(functionIndex(name) >= 0 && !(functionAt(functionIndex(name)).flags & kFnVolatile), name);
    }
};

JP_TEST_CLASS(SheetFnMathTests)
#include "tst_sheet_fn_math.moc"
