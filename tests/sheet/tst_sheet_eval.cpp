// Tests of the evaluator (calc/eval.h): the order of the operators at work, how values are read as numbers and text, comparisons, errors passing through, the
// sums that should be zero, a range read by a formula for one value and by an array formula, other sheets, defined names, shared formulas, and the bounds on
// hostile formulas. Each expected value is written from the rules of the language, not from what the evaluator gave.
#include "calchelpers.h"

#include <QRandomGenerator>

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

Grid sample()
{
    Grid g;
    g.column("A1", {1, 2, 3});
    g.column("B1", {10, 20, 30});
    g.row("A5", {10, 20, 30});
    return g;
}

} // namespace

class SheetEvalTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void operatorsWorkInTheirOrder()
    {
        const E cases[] = {
            {"-2^2", "n:4"},               // a minus sign in front binds tighter than "^"
            {"2^-2", "n:0.25"},
            {"1-2-3", "n:-4"},
            {"\"a\"&1+1", "s:a2"},
            {"2^3^2", "n:64"},             // "^" groups from the left
            {"-(2^2)", "n:-4"},
            {"5%", "n:0.05"},
            {"50%*2", "n:1"},
            {"2*3+4*5", "n:26"},
            {"2^2*3", "n:12"},
            {"1+2&3", "s:33"},
            {"1=1", "b:1"},
            {"1<2<3", "b:0"},              // (1<2) is TRUE, and TRUE is greater than every number
            {"10-5%", "n:9.95"},
            {"2*-3", "n:-6"},
            {"-3^2+1", "n:10"},
            {"--5", "n:5"},
            {"10/4", "n:2.5"},
            {"10/2/5", "n:1"},
            {"1+2=3", "b:1"},
            {"2^0.5", "n:1.41421356237309"},       // a power is exp(0.5 ln 2) in doubles, one unit of the last place under the square root
            {"1/3*3", "n:1"},
            {"(1+2)*(3+4)", "n:21"},
            {"200%", "n:2"},
            {"-5%", "n:-0.05"},
            {"2^10", "n:1024"},
            {"7-2*3^2", "n:-11"},
        };
        CHECK_EVALS(cases, Grid(), Settings());
    }

    void textIsReadAsANumberWhereANumberIsNeeded()
    {
        const E cases[] = {
            {"\"5\"+1", "n:6"},
            {"\"1,000\"+1", "n:1001"},
            {"\" 7 \"*2", "n:14"},
            {"\"1e2\"+0", "n:100"},
            {"\"$5\"+1", "n:6"},
            {"\"5%\"+1", "n:1.05"},
            {"\"-5\"+0", "n:-5"},
            {"\"+5\"+0", "n:5"},
            {"\"1\"+\"2\"", "n:3"},
            {"\"1.5\"*\"2\"", "n:3"},
            {".5+\"5.\"", "n:5.5"},
            {"\"abc\"+1", "e:#VALUE!"},
            {"\"\"+1", "e:#VALUE!"},
            {"\"12abc\"+0", "e:#VALUE!"},
            {"\"1 000\"+0", "e:#VALUE!"},
            {"\"--5\"+0", "e:#VALUE!"},
            {"\"1e\"+0", "e:#VALUE!"},
            {"\"TRUE\"+1", "e:#VALUE!"},       // the text TRUE is no boolean in arithmetic
            {"TRUE+1", "n:2"},
            {"FALSE*5", "n:0"},
            {"-TRUE", "n:-1"},
            {"-\"5\"", "n:-5"},
            {"+\"a\"", "s:a"},                 // a plus sign in front changes nothing
            {"\"3/15/2024\"+0", "n:45366"},
            {"\"2024-03-15\"+0", "n:45366"},
            {"\"10:30\"+0", "n:0.4375"},
            {"\"12:00 PM\"+0", "n:0.5"},
            {"\"3/15/2024 12:00\"+0", "n:45366.5"},
        };
        CHECK_EVALS(cases, Grid(), Settings());
    }

    void emptyCellsAreZeroOrNothing()
    {
        Grid g;
        const E cases[] = {
            {"A1+1", "n:1"},
            {"A1", "n:0"},                     // a formula that is a reference to an empty cell is 0
            {"A1&\"x\"", "s:x"},
            {"A1*5", "n:0"},
            {"-A1", "n:0"},
            {"A1=0", "b:1"},
            {"A1=\"\"", "b:1"},
            {"A1=FALSE", "b:1"},
            {"A1<1", "b:1"},
            {"A1=A2", "b:1"},
            {"A1<>\"\"", "b:0"},
            {"ISBLANK(A1)", "b:1"},
            {"A1&A2", "s:"},
        };
        CHECK_EVALS(cases, g, Settings());
    }

    void numbersAreMadeIntoTextWithFifteenDigits()
    {
        const E cases[] = {
            {"1&2", "s:12"},
            {"1.5&\"x\"", "s:1.5x"},
            {"TRUE&\"x\"", "s:TRUEx"},
            {"(0.1+0.2)&\"\"", "s:0.3"},
            {"1/3&\"\"", "s:0.333333333333333"},
            {"123456789012345678&\"\"", "s:1.23456789012346E+17"},
            {"2^70&\"\"", "s:1.18059162071741E+21"},
            {"-1&\"\"", "s:-1"},
            {"100&\"\"", "s:100"},
            {"\"a\"&\"b\"&\"c\"", "s:abc"},
        };
        CHECK_EVALS(cases, Grid(), Settings());
    }

    void comparisonsOrderNumbersThenTextThenBooleans()
    {
        const E cases[] = {
            {"\"a\"=\"A\"", "b:1"},
            {"\"a\"<\"B\"", "b:1"},
            {"\"Z\"<\"a\"", "b:0"},            // without regard to case: z comes after a
            {"\"apple\"<\"banana\"", "b:1"},
            {"\"a\"<>\"A\"", "b:0"},
            {"1<\"a\"", "b:1"},                // a number is less than a text
            {"\"a\"<TRUE", "b:1"},             // a text is less than a boolean
            {"TRUE>100", "b:1"},
            {"FALSE<0", "b:0"},
            {"\"10\"<\"9\"", "b:1"},           // text compares as text
            {"\"10\"=10", "b:0"},
            {"\"1\"=1", "b:0"},
            {"1=TRUE", "b:0"},
            {"TRUE=TRUE", "b:1"},
            {"\"\"<\"a\"", "b:1"},
            {"\"1\"<\"a\"", "b:1"},            // digits come before letters
            {"2>1", "b:1"},
            {"0.1+0.2=0.3", "b:1"},            // the same to 15 digits is equal
            {"1=1.0000000000000002", "b:1"},
            {"1=1.00000000000001", "b:0"},
            {"1<>1.00000000000001", "b:1"},
            {"2>=2", "b:1"},
            {"2<=1", "b:0"},
        };
        CHECK_EVALS(cases, Grid(), Settings());
    }

    void errorsPassThrough()
    {
        Grid g;
        g.e("A1", ErrorKind::Div0);
        g.n("A2", 5);
        const E cases[] = {
            {"#N/A+1", "e:#N/A"},
            {"1+#DIV/0!", "e:#DIV/0!"},
            {"#N/A+#DIV/0!", "e:#N/A"},         // the first error is the one that stands
            {"#DIV/0!+#N/A", "e:#DIV/0!"},
            {"#N/A&\"x\"", "e:#N/A"},
            {"-#NULL!", "e:#NULL!"},
            {"#N/A=1", "e:#N/A"},
            {"#N/A%", "e:#N/A"},
            {"#REF!^2", "e:#REF!"},
            {"A1+1", "e:#DIV/0!"},
            {"A1", "e:#DIV/0!"},
            {"A1&\"x\"", "e:#DIV/0!"},
            {"A2/A1", "e:#DIV/0!"},
            {"SUM(A1:A2)", "e:#DIV/0!"},
            {"SUM(1,A1)", "e:#DIV/0!"},
            {"IF(A1,1,2)", "e:#DIV/0!"},
            {"ISERROR(A1)", "b:1"},
            {"1/0", "e:#DIV/0!"},
            {"0/0", "e:#DIV/0!"},
            {"A2/0", "e:#DIV/0!"},
            {"(-8)^(1/3)", "e:#NUM!"},
            {"0^0", "e:#NUM!"},
            {"0^-1", "e:#DIV/0!"},
            {"2^1024", "e:#NUM!"},
            {"1E308*10", "e:#NUM!"},
            {"(-8)^3", "n:-512"},
            {"4^0.5", "n:2"},
            {"(-2)^2", "n:4"},
            {"10^-2", "n:0.01"},
            {"0^2", "n:0"},
        };
        CHECK_EVALS(cases, g, Settings());
    }

    void sumsThatShouldBeZeroAreZero()
    {
        const E zero[] = {
            {"0.1+0.2-0.3", "n:0"},
            {"1-0.9-0.1", "n:0"},
            {"1.1-1-0.1", "n:0"},
            {"0.3-0.1-0.2", "n:0"},
            {"0.1*3-0.3", "n:0"},
            {"1E-20-1E-20", "n:0"},
        };
        CHECK_EVALS(zero, Grid(), Settings());
        // only in the last operation of the formula: inside parentheses, or under another operator, the rounding error stays
        QCOMPARE(ev("1*(0.5-0.4-0.1)"), num(-2.7755575615628914e-17));
        QCOMPARE(ev("(0.5-0.4-0.1)"), num(-2.7755575615628914e-17));
        QCOMPARE(ev("1E-20+0"), num(1e-20));
        QCOMPARE(ev("1E-20"), num(1e-20));
        // a difference of numbers that are not near each other is its own
        QCOMPARE(ev("1-0.9"), num(0.09999999999999998));
        QCOMPARE(ev("1000000-0.000001"), num(999999.999999));
    }

    void aRangeInAFormulaForOneValueIsTheCellItCrossesAndInAnArrayFormulaItIsAnArray()
    {
        Grid g = sample();
        Settings atC2;
        atC2.at = "C2";
        Settings atC5;
        atC5.at = "C5";
        Settings atB9;
        atB9.at = "B9";
        Settings atC3;
        atC3.at = "C3";
        QCOMPARE(ev("A1:A3", g, atC2), num(2));
        QCOMPARE(ev("A1:A3*2", g, atC3), num(6));
        QCOMPARE(ev("A1:A3", g, atC5), QStringLiteral("e:#VALUE!"));      // row 5 crosses nothing
        QCOMPARE(ev("A1:A3*2", g, atC5), QStringLiteral("e:#VALUE!"));
        QCOMPARE(ev("A5:C5", g, atB9), num(20));                          // column B crosses the row
        QCOMPARE(ev("A1:B3", g, atC2), QStringLiteral("e:#VALUE!"));       // a block must be crossed by both the row and the column
        Settings atB2;
        atB2.at = "B2";
        QCOMPARE(ev("A1:B3", g, atB2), num(20));
        QCOMPARE(ev("A1:A3 A2:B2", g), num(2));                           // the intersection of two ranges is the cell they share
        QCOMPARE(ev("A1:A3 B1:B3", g), QStringLiteral("e:#NULL!"));      // and none when they share nothing
        QCOMPARE(ev("SUM(A1:B3 A2:A3)", g), num(5));

        Settings array;
        array.array = true;
        const E cases[] = {
            {"A1:A3*2", "a:3x1[n:2;n:4;n:6]"},
            {"{1,2;3,4}+10", "a:2x2[n:11,n:12;n:13,n:14]"},
            {"{1,2,3}+{10;20}", "a:2x3[n:11,n:12,n:13;n:21,n:22,n:23]"},   // a row and a column make a block
            {"{1,2,3}+{10,20}", "a:1x3[n:11,n:22,e:#N/A]"},                 // what has no pair is #N/A
            {"A1:A3", "a:3x1[n:1;n:2;n:3]"},
            {"SUM(A1:A3*2)", "n:12"},
            {"{1,2,3}=2", "a:1x3[b:0,b:1,b:0]"},
            {"-{1,2}", "a:1x2[n:-1,n:-2]"},
            {"{1,2}&\"x\"", "a:1x2[s:1x,s:2x]"},
            {"IF({1,0,1},\"y\",\"n\")", "a:1x3[s:y,s:n,s:y]"},
            {"ABS({-1,2,-3})", "a:1x3[n:1,n:2,n:3]"},
            {"{1,\"a\";TRUE,#N/A}", "a:2x2[n:1,s:a;b:1,e:#N/A]"},
            {"A1:A3+B1:B3", "a:3x1[n:11;n:22;n:33]"},
            {"SUMPRODUCT((A1:A3>1)*(B1:B3))", "n:50"},
            {"A1:B2*{1,10}", "a:2x2[n:1,n:100;n:2,n:200]"},
        };
        CHECK_EVALS(cases, g, array);
        // an empty cell in the array a formula makes is 0, as it is anywhere a formula's value is
        Grid sparse;
        sparse.n("A1", 1);
        QCOMPARE(ev("A1:A2", sparse, array), QStringLiteral("a:2x1[n:1;n:0]"));
        QCOMPARE(ev("TRANSPOSE(A1:B1)", sparse, array), QStringLiteral("a:2x1[n:1;n:0]"));
        // The same functions of a formula that is not an array formula work on one value: the first of an array.
        const E single[] = {
            {"ABS({-1,2,-3})", "n:1"},
            {"{1,2,3}*2", "n:2"},
            {"SUMPRODUCT((A1:A3>1)*(B1:B3))", "n:50"},     // SUMPRODUCT works its arguments out as arrays
            {"SUMPRODUCT(A1:A3,B1:B3)", "n:140"},
            {"SUMPRODUCT(--(A1:A3>1))", "n:2"},
        };
        CHECK_EVALS(single, g, Settings());
    }

    void otherSheetsAndNames()
    {
        Grid g;
        g.addSheet("Sheet2");
        g.addSheet("My Sheet");
        g.addSheet("Sheet4");
        g.n("A1", 1, 0).n("A1", 2, 1).n("B2", 7, 2).n("A1", 4, 3);
        g.column("A3", {1, 2, 3}, 0);
        g.name("Rate", "0.05");
        g.name("MyRange", "Sheet1!$A$3:$A$5");
        g.name("Loc", "10", 0);
        g.name("Loc", "20");
        g.name("Glob", "30");
        g.name("Chain", "Rate*2");
        g.name("Loop", "Loop+1");
        g.name("Bad", "SUM(");
        // a name whose reference moves with the cell is written for A1: used far from it, past the edge of the sheet, it comes round the other side
        g.name("Far", "B1048537:B1048555");
        g.n("K10", 5);
        Settings farAway;
        farAway.at = "J50";
        QCOMPARE(ev("SUM(Far)", g, farAway), num(5));
        const E cases[] = {
            {"Sheet2!A1+1", "n:3"},
            {"sheet2!a1", "n:2"},
            {"'My Sheet'!B2", "n:7"},
            {"Sheet9!A1", "e:#REF!"},
            {"SUM(Sheet1:Sheet4!A1)", "n:7"},       // sheets 1 to 4, in the order the workbook has them
            {"SUM(Sheet4:Sheet1!A1)", "n:7"},
            {"SUM(Sheet1:Sheet2!A1)", "n:3"},
            {"Rate*100", "n:5"},
            {"SUM(MyRange)", "n:6"},
            {"Loc", "n:10"},                        // the name of the sheet the formula is in comes first
            {"Sheet2!Loc", "n:20"},                 // another sheet has none of its own: the workbook's
            {"Glob+1", "n:31"},
            {"Chain", "n:0.1"},
            {"Foo", "e:#NAME?"},
            {"Foo+1", "e:#NAME?"},
            {"IFERROR(Foo,5)", "n:5"},
            {"Loop", "e:#REF!"},                    // a name that stands for itself is no number
            {"Bad", "e:#NAME?"},
            {"ISERROR(Foo)", "b:1"},
        };
        CHECK_EVALS(cases, g, Settings());
    }

    void sharedFormulasMoveTheirReferences()
    {
        Grid g;
        g.n("A1", 10).n("B1", 1).n("B2", 100).n("C1", 5).n("C2", 6).n("C3", 7);
        Settings moved;
        moved.shiftRows = 1;
        moved.shiftCols = 1;
        QCOMPARE(ev("A1+$B$1", g, moved), num(101));      // A1 moves to B2; $B$1 stays
        QCOMPARE(ev("SUM(A:A)", g, moved), num(1 + 100));  // A:A moves to B:B
        Settings up;
        up.shiftRows = -1;
        QCOMPARE(ev("A1", g, up), QStringLiteral("e:#REF!"));   // the grid ends at row 1
        Settings down;
        down.shiftRows = 2;
        QCOMPARE(ev("SUM(C1:C1)", g, down), num(7));
        QCOMPARE(ev("SUM(C$1:C1)", g, down), num(5 + 6 + 7));
    }

    void ifChoosesAndOnlyWorksOutWhatItNeeds()
    {
        Grid g;
        g.e("A1", ErrorKind::Div0);
        g.n("A2", 1);
        const E cases[] = {
            {"IF(TRUE,1,1/0)", "n:1"},
            {"IF(FALSE,1/0,2)", "n:2"},
            {"IF(1,2)", "n:2"},
            {"IF(0,2)", "b:0"},
            {"IF(0,2,)", "n:0"},
            {"IF(1,,5)", "n:0"},
            {"IF(\"TRUE\",1,2)", "n:1"},
            {"IF(\"x\",1,2)", "e:#VALUE!"},
            {"IF(A3,1,2)", "n:2"},                 // an empty cell is false
            {"IF(A2,\"yes\",\"no\")", "s:yes"},
            {"IF(2>1,\"a\",\"b\")&\"!\"", "s:a!"},
            {"IFERROR(1/0,5)", "n:5"},
            {"IFERROR(3,1/0)", "n:3"},
            {"IFERROR(A1,\"x\")", "s:x"},
            {"IFERROR(#N/A,1)+1", "n:2"},
            {"_xlfn.IFNA(#N/A,7)", "n:7"},
            {"_xlfn.IFNA(#DIV/0!,7)", "e:#DIV/0!"},
            {"_xlfn.IFNA(4,7)", "n:4"},
            {"CHOOSE(2,\"a\",\"b\",\"c\")", "s:b"},
            {"CHOOSE(1,5,1/0)", "n:5"},
            {"CHOOSE(4,\"a\",\"b\",\"c\")", "e:#VALUE!"},
            {"CHOOSE(0,\"a\",\"b\")", "e:#VALUE!"},
            {"CHOOSE(2.9,\"a\",\"b\",\"c\")", "s:b"},
            {"CHOOSE(\"2\",\"a\",\"b\")", "s:b"},
            {"SUM(CHOOSE(2,A1:A1,A2:A2))", "n:1"},
            {"IF(1,2,3)+IF(0,2,3)", "n:5"},
            {"IF(IF(1,0,1),\"a\",\"b\")", "s:b"},
            {"IF(1,IF(0,1,2),3)", "n:2"},
        };
        CHECK_EVALS(cases, g, Settings());
    }

    void aProgramRunsOverDifferentSources()
    {
        // The same parsed formula, a different source of cells each time: a sheet's place is looked up again.
        const ParseResult p = parseFormula("Sheet2!A1*2");
        QVERIFY(p.ok());
        Grid a;
        a.addSheet("Sheet2");
        a.n("A1", 5, 1);
        Grid b;
        b.addSheet("Other");
        b.addSheet("Sheet2");
        b.n("A1", 7, 2);
        Evaluator evaluator;
        EvalContext ctx;
        ctx.row = 9;
        ctx.col = 9;
        ctx.source = &a;
        QCOMPARE(evaluator.evaluate(*p.program, ctx).debug(), num(10));
        ctx.source = &b;
        QCOMPARE(evaluator.evaluate(*p.program, ctx).debug(), num(14));
        ctx.source = &a;
        QCOMPARE(evaluator.evaluate(*p.program, ctx).debug(), num(10));
        QCOMPARE(evaluator.status(), EvalStatus::Ok);
    }

    void aLongChainDoesNotNeedDeepRecursion()
    {
        QString sum = QStringLiteral("1");
        QString minus = QStringLiteral("100000");
        QString cat = QStringLiteral("\"\"");
        for (int i = 0; i < 9000; ++i) {
            sum += QStringLiteral("+1");
            minus += QStringLiteral("-1");
        }
        for (int i = 0; i < 3000; ++i) cat += QStringLiteral("&\"x\"");
        QCOMPARE(ev(sum), num(9001));
        QCOMPARE(ev(minus), num(100000 - 9000));
        QCOMPARE(ev(cat).size(), 2 + 3000);
        QCOMPARE(ev(QString("-").repeated(9000) + "5"), num(5));
        QCOMPARE(ev("5" + QString("%").repeated(3)), num(5e-6));
    }

    void workAndSizeAreBounded()
    {
        Grid g;
        QVariantList many;
        for (int i = 0; i < 100000; ++i) many << i;
        g.column("A1", many);
        EvalStatus status = EvalStatus::Ok;
        QCOMPARE(evStatus("SUM(A1:A100000)", g, Settings(), &status), num(4999950000.0));
        QCOMPARE(status, EvalStatus::Ok);
        // past the steps a evaluation may take
        const QString over = evStatus("SUM(A1:A100000)+SUM(A1:A100000)", g, Settings(), &status, 150000);
        QVERIFY2(over.startsWith("e:"), qPrintable(over));
        QCOMPARE(status, EvalStatus::TooMuchWork);
        // an array of more cells than any sheet has room for
        Settings array;
        array.array = true;
        const QString big = evStatus("ROW(1:1048576)*COLUMN(A:Z)", g, array, &status);
        QVERIFY2(big.startsWith("e:"), qPrintable(big));
        QCOMPARE(status, EvalStatus::TooLarge);
        // a text of more than a cell holds
        QCOMPARE(ev("REPT(\"x\",40000)"), QStringLiteral("e:#VALUE!"));
        QCOMPARE(ev("REPT(\"x\",20000)&REPT(\"x\",20000)"), QStringLiteral("e:#VALUE!"));
        QCOMPARE(ev("LEN(REPT(\"x\",32767))"), num(32767));
        // names inside names inside names
        Grid names;
        for (int i = 1; i <= 30; ++i) names.name(QStringLiteral("Lvl_%1").arg(i), QStringLiteral("Lvl_%1+1").arg(i + 1));
        names.name("Lvl_31", "0");
        const QString deep = ev("Lvl_1", names);
        QVERIFY2(deep.startsWith("e:"), qPrintable(deep));
        Grid shallow;
        for (int i = 1; i <= 5; ++i) shallow.name(QStringLiteral("Lvl_%1").arg(i), QStringLiteral("Lvl_%1+1").arg(i + 1));
        shallow.name("Lvl_6", "0");
        QCOMPARE(ev("Lvl_1", shallow), num(5));
        // A whole column of text read and joined stays inside the steps
        Grid texts;
        for (int i = 1; i <= 3000; ++i) texts.s(QStringLiteral("A%1").arg(i), QStringLiteral("some words here"));
        QVERIFY(ev("SUM(LEN(A1:A3000))", texts, array) == num(3000 * 15));
    }

    // Formulas as hostile as a file can make them (within the length a formula may have) are worked out, or refused, in bounded time and memory.
    void hostileFormulasAreBounded()
    {
        QCOMPARE(ev("SUM(" + QString("1,").repeated(999) + "1)"), num(1000));
        QCOMPARE(ev(QString("ABS(").repeated(100) + "-1" + QString(")").repeated(100)), num(1));
        QCOMPARE(ev(QString("(").repeated(100) + "7" + QString(")").repeated(100)), num(7));
        QCOMPARE(ev(QString("IF(TRUE,").repeated(100) + "9" + QString(",0)").repeated(100)), num(9));
        // names that call themselves, through INDIRECT as well
        Grid loops;
        loops.name("Selfie", "INDIRECT(\"Selfie\")+1");
        loops.name("Ping", "Pong+1");
        loops.name("Pong", "Ping+1");
        for (const char *f : {"Selfie", "Ping", "Selfie+Ping", "SUM(Pong)"}) QVERIFY2(ev(f, loops).startsWith("e:"), f);
        // arrays of the most that a formula can write, and of a column of the grid
        Settings array;
        array.array = true;
        QString ones = QStringLiteral("{1");
        for (int i = 1; i < 7000; ++i) ones += QStringLiteral(",1");
        ones += QLatin1Char('}');
        QCOMPARE(ev("SUM(" + ones + "*" + ones + ")", Grid(), array), num(7000));
        QCOMPARE(ev("SUM(ROW(1:1048576))", Grid(), array), num(549756338176.0));
        QCOMPARE(ev("SUM(IF(ROW(1:100000)>5,1,0))", Grid(), array), num(99995));
        // through a great many sheets
        Grid sheets;
        for (int i = 2; i <= 200; ++i) sheets.addSheet(QStringLiteral("Sheet%1").arg(i));
        for (int i = 0; i < 200; ++i) sheets.n("A1", 1, i);
        QCOMPARE(ev("SUM(Sheet1:Sheet200!A1)", sheets), num(200));
        QCOMPARE(ev("SUM(Sheet1:Sheet200!A:A)", sheets), num(200));
        // a long text, built and cut
        QCOMPARE(ev("LEN(REPT(\"ab\",16000)&\"x\")"), num(32001));
        QCOMPARE(ev("LEN(REPT(\"ab\",16000)&REPT(\"c\",800))"), QStringLiteral("e:#VALUE!"));
    }

    void randomFunctionCallsNeverCrashOrLoop()
    {
        Grid g = sample();
        g.s("C1", "text").s("C2", "").e("C3", ErrorKind::NA).b("D1", true);
        static const char *const args[] = {"1", "-1", "0", "2.5", "\"a\"", "\"\"", "\"1e5\"", "TRUE", "#N/A", "A1", "A1:B3", "C1:D3", "{1,2;3,4}", "{\"a\",\"b\"}", "1E308", "-1E308", "1E-308",
                                           "A:A", "1:1", "99999999999", "1/0", "ROW(A1:A3)", "A1:A3", "\"x*\"", "\">1\"", "-0.5", "100", "45366", "\"2024-03-15\"", "Sheet1!A1", "Sheet2!A1",
                                           "NOW()", "\"R1C1\"", "\"A1\"", "\"Sheet1!A1:B2\"", "65536", "1000000", "A1:A1048576", "\"%%\"", "\"[\"", "0.0000001"};
        const int nargs = int(sizeof(args) / sizeof(args[0]));
        QRandomGenerator rng(20261022);
        Settings plain;
        Settings array;
        array.array = true;
        for (int i = 0; i < functionCount(); ++i) {
            const FunctionInfo &f = functionAt(i);
            const int most = f.maxArgs == 255 ? 6 : int(f.maxArgs);
            for (int trial = 0; trial < 120; ++trial) {
                const int n = int(f.minArgs) + int(rng.bounded(quint32(qMax(1, most - int(f.minArgs) + 1))));
                QString text = QString::fromLatin1((f.flags & kFnFuture) ? "_xlfn." : "") + QString::fromLatin1(f.name) + QLatin1Char('(');
                for (int k = 0; k < n; ++k) {
                    if (k) text += QLatin1Char(',');
                    // (an argument is left out now and then, but one alone would make the call look as if it had none)
                    if (n == 1 || rng.bounded(12) != 0) text += QString::fromLatin1(args[rng.bounded(quint32(nargs))]);
                }
                text += QLatin1Char(')');
                EvalStatus status = EvalStatus::Ok;
                const QString a = evStatus(text, g, trial % 2 ? array : plain, &status, 200000);
                QVERIFY2(!a.isEmpty() && !a.startsWith("!parse"), qPrintable(text + QStringLiteral(" -> ") + a));
            }
        }
    }
};

JP_TEST_CLASS(SheetEvalTests)
#include "tst_sheet_eval.moc"
