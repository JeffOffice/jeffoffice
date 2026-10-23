// Tests of the lookup and reference functions: VLOOKUP, HLOOKUP, MATCH, and LOOKUP in their exact and approximate modes (the approximate ones search by halving
// the range, so on data that is not in order they find what the halving finds), INDEX, OFFSET, INDIRECT, and the functions that tell where a reference is.
// Each expected value is written from the function's definition, and the halving worked out by hand, not from what the code gave.
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

// A1:C4 = a 1 x / b 2 y / c 3 z / B 4 w;  E1:F5 = 10..50 with their names;  H1:I4 = apple..date with 1..4;  K1:K5 = 50..10 descending;
// L1:M5 = 30 p / 10 q / 20 r / 5 s / 40 t (not in order);  N1:Q2 = a b c d over 1 2 3 4;  G1:G5 = 30 10 20 5 40
Grid sample()
{
    Grid g;
    g.row("A1", {QString("a"), 1, QString("x")}).row("A2", {QString("b"), 2, QString("y")}).row("A3", {QString("c"), 3, QString("z")}).row("A4", {QString("B"), 4, QString("w")});
    g.column("E1", {10, 20, 30, 40, 50});
    g.column("F1", {QString("ten"), QString("twenty"), QString("thirty"), QString("forty"), QString("fifty")});
    g.column("H1", {QString("apple"), QString("banana"), QString("cherry"), QString("date")});
    g.column("I1", {1, 2, 3, 4});
    g.column("K1", {50, 40, 30, 20, 10});
    g.column("L1", {30, 10, 20, 5, 40});
    g.column("M1", {QString("p"), QString("q"), QString("r"), QString("s"), QString("t")});
    g.row("N1", {QString("a"), QString("b"), QString("c"), QString("d")});
    g.row("N2", {1, 2, 3, 4});
    g.column("G1", {30, 10, 20, 5, 40});
    return g;
}

} // namespace

class SheetFnLookupTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void vlookupFindsAnExactMatch()
    {
        const Grid g = sample();
        const E cases[] = {
            {"VLOOKUP(\"b\",A1:C4,2,FALSE)", "n:2"},          // the first of the matches
            {"VLOOKUP(\"B\",A1:C4,2,FALSE)", "n:2"},          // without regard to case
            {"VLOOKUP(\"d\",A1:C4,2,FALSE)", "e:#N/A"},
            {"VLOOKUP(\"c\",A1:C4,3,FALSE)", "s:z"},
            {"VLOOKUP(\"a\",A1:C4,2,0)", "n:1"},
            {"VLOOKUP(\"a\",A1:C4,0,FALSE)", "e:#VALUE!"},
            {"VLOOKUP(\"a\",A1:C4,4,FALSE)", "e:#REF!"},
            {"VLOOKUP(\"a\",A1:C4,\"2\",FALSE)", "n:1"},
            {"VLOOKUP(\"*\",A1:C4,2,FALSE)", "n:1"},          // wildcards
            {"VLOOKUP(\"?\",A1:C4,2,FALSE)", "n:1"},
            {"VLOOKUP(\"c*\",A1:C4,3,FALSE)", "s:z"},
            {"VLOOKUP(\"~*\",A1:C4,2,FALSE)", "e:#N/A"},
            {"VLOOKUP(2,A1:C4,2,FALSE)", "e:#N/A"},            // a number is not a text
            {"VLOOKUP(2,B1:C4,2,FALSE)", "s:y"},
            {"VLOOKUP(\"2\",B1:C4,2,FALSE)", "e:#N/A"},
            {"VLOOKUP(2,E1:F5,2,FALSE)", "e:#N/A"},
            {"VLOOKUP(30,E1:F5,2,FALSE)", "s:thirty"},
            {"VLOOKUP(30,E1:F5,2,)", "s:thirty"},               // an argument left out is FALSE
            {"VLOOKUP(25,E1:F5,2,)", "e:#N/A"},
            {"VLOOKUP(#N/A,E1:F5,2,FALSE)", "e:#N/A"},
            {"VLOOKUP(30,E1:F5,2,#DIV/0!)", "e:#DIV/0!"},
            {"VLOOKUP(5,L1:M5,2,FALSE)", "s:s"},                // exact finds it wherever it stands
            {"VLOOKUP(A9,E1:F5,2,FALSE)", "e:#N/A"},            // an empty cell looked up is 0 for a number column
            {"VLOOKUP(\"c\",A:C,3,FALSE)", "s:z"},              // whole columns
            {"VLOOKUP(\"c\",A1:C4,2,FALSE)+1", "n:4"},
            {"VLOOKUP(\"b\",{\"a\",1;\"b\",2},2,FALSE)", "n:2"},
        };
        CHECK_EVALS(cases, g, Settings());
    }

    void approximateModeSearchesByHalvingTheRange()
    {
        const Grid g = sample();
        const E cases[] = {
            {"VLOOKUP(25,E1:F5,2)", "s:twenty"},               // the last one not greater than the value
            {"VLOOKUP(20,E1:F5,2)", "s:twenty"},
            {"VLOOKUP(5,E1:F5,2)", "e:#N/A"},                  // smaller than all
            {"VLOOKUP(100,E1:F5,2)", "s:fifty"},
            {"VLOOKUP(10,E1:F5,2,TRUE)", "s:ten"},
            {"VLOOKUP(49.99,E1:F5,2,TRUE)", "s:forty"},
            {"VLOOKUP(25,E1:F5,2,1)", "s:twenty"},
            {"VLOOKUP(\"cat\",H1:I4,2,TRUE)", "n:2"},          // banana <= cat < cherry
            {"VLOOKUP(\"zzz\",H1:I4,2,TRUE)", "n:4"},
            {"VLOOKUP(\"a\",H1:I4,2,TRUE)", "e:#N/A"},
            {"VLOOKUP(\"CHERRY\",H1:I4,2,TRUE)", "n:3"},
            {"VLOOKUP(\"cherry\",E1:F5,2,TRUE)", "e:#N/A"},    // a text finds no number
            // Not in order: L1:L5 = 30 10 20 5 40. The range is halved, keeping the upper part when the middle is not greater than the value. For 25: the middle,
            // 20, is not greater, so the part from 5 on is searched; 5 is not greater, so 40 is searched; 40 is greater; the last not greater that was met is 5.
            {"VLOOKUP(25,L1:M5,2,TRUE)", "s:s"},
            {"VLOOKUP(35,L1:M5,2,TRUE)", "s:s"},
            {"VLOOKUP(45,L1:M5,2,TRUE)", "s:t"},
            // For 15: the middle, 20, is greater, so the lower part is searched, and its middle, 30, is greater, so nothing is left: #N/A, though 10 and 5 are there.
            {"VLOOKUP(15,L1:M5,2,TRUE)", "e:#N/A"},
            {"VLOOKUP(5,L1:M5,2,TRUE)", "e:#N/A"},
        };
        CHECK_EVALS(cases, g, Settings());
    }

    // The halving stops at a cell equal to the value and takes the last of the equal cells that follow it (a run), instead of going on to cells after it that are less:
    // in a file of the practice workbooks, VLOOKUP(37104, ...) over 36251, 36951, 37104, 37012, 37377, ... gave the row of 37104, not the row of 37012 that the
    // halving met after it (12 cells, and no cell of the 4,176 other lookups of data out of order told otherwise).
    void approximateModeStopsAtACellEqualToTheValue()
    {
        Grid g;
        g.column("A1", {1, 2, 20, 10, 15});            // 20 is met first, and 10 and 15 are less than it
        g.column("B1", {QString("a"), QString("b"), QString("c"), QString("d"), QString("e")});
        g.column("C1", {1, 5, 20, 20, 30});            // in order, with equal cells
        g.column("D1", {20, 20, 20, 5, 20});           // the run of equal cells ends at the first cell that is not equal
        g.column("E1", {5, 9, 7, 8, 6});               // for MATCH -1
        g.column("F1", {QString("a"), QString("z"), QString("m"), QString("b"), QString("c")});
        g.column("H1", {1, 2, 3, 4, 5});
        g.column("I1", {QString("a"), QString("b"), QString("c"), QString("d"), QString("e")});
        const E cases[] = {
            {"VLOOKUP(20,A1:B5,2)", "s:c"},
            {"VLOOKUP(20,A1:B5,2,TRUE)", "s:c"},
            {"MATCH(20,A1:A5,1)", "n:3"},
            {"MATCH(20,A1:A5)", "n:3"},
            {"LOOKUP(20,A1:A5,B1:B5)", "s:c"},
            {"LOOKUP(20,A1:A5)", "n:20"},
            {"HLOOKUP(20,{1,2,20,10,15;\"a\",\"b\",\"c\",\"d\",\"e\"},2)", "s:c"},
            {"VLOOKUP(20,{1,\"a\";2,\"b\";20,\"c\";10,\"d\";15,\"e\"},2)", "s:c"},
            // equal cells in order: the last of them (the search reaches the first at the middle and goes on to the next)
            {"MATCH(20,C1:C5,1)", "n:4"},
            {"VLOOKUP(20,C1:D5,2)", "n:5"},       // (the second of the cells of 20 in C is in row 4, where D holds 5)
            // the run is cut where a cell that is not equal comes
            {"MATCH(20,D1:D5,1)", "n:3"},
            // descending: the smallest cell not less than the value, stopping at an equal one
            {"MATCH(7,E1:E5,-1)", "n:3"},
            {"MATCH(50,{50,40,30,20,10},-1)", "n:1"},
            {"MATCH(30,{50,40,30,30,10},-1)", "n:4"},
            // text
            {"MATCH(\"m\",F1:F5,1)", "n:3"},
            {"VLOOKUP(\"M\",F1:F5,1)", "s:m"},
            // a value that is not met keeps the rule of the last cell not after it
            {"MATCH(12,A1:A5,1)", "n:2"},       // (20 is greater, then 1 and 2 are not: the 10 and the 15 are not met)
            {"MATCH(3,H1:H5,1)", "n:3"},
            {"MATCH(3.5,H1:H5,1)", "n:3"},
        };
        CHECK_EVALS(cases, g, Settings());
    }

    // Cells at the end of the range that hold nothing, and cells that are errors or another kind than the value, are passed over: the range of the lookup is the cells
    // that can be compared (215 cells of the practice workbooks: a table of 21 rows and one empty, looked up by a value past the last, gave the last row, not the empty one).
    void emptyCellsAndOtherKindsAreNotMet()
    {
        Grid g;
        g.column("A1", {1, 5, 9, QString(), QString()});
        g.column("B1", {QString("one"), QString("five"), QString("nine"), QString(), QString()});
        g.column("D1", {QString("hdr"), 1, 5, 9, QString("total")});
        g.column("E1", {QString("h"), QString("a"), QString("b"), QString("c"), QString("t")});
        g.e("G1", ErrorKind::NA).n("G2", 1).e("G3", ErrorKind::Div0).n("G4", 3).e("G5", ErrorKind::NA);
        const E cases[] = {
            {"MATCH(100,A1:A5,1)", "n:3"},
            {"VLOOKUP(100,A1:B5,2)", "s:nine"},
            {"VLOOKUP(6,A1:B5,2)", "s:five"},
            {"VLOOKUP(0,A1:B5,2)", "e:#N/A"},
            {"MATCH(100,A1:A8,1)", "n:3"},                      // empty cells past the data
            {"MATCH(7,D1:D5,1)", "n:3"},                        // a heading before, a total after
            {"VLOOKUP(7,D1:E5,2)", "s:b"},
            {"VLOOKUP(100,D1:E5,2)", "s:c"},
            {"MATCH(\"zzz\",D1:D5,1)", "n:5"},                  // the last text: numbers before it are another kind
            {"MATCH(\"a\",D1:D5,1)", "e:#N/A"},                 // "a" is before "hdr", and the only text before it is none
            {"MATCH(2,G1:G5,1)", "n:2"},                        // errors passed over
            {"MATCH(9.99E+307,G1:G5)", "n:4"},
            {"LOOKUP(2,1/(G1:G5>0),G1:G5)", "n:3"},
            {"MATCH(TRUE,D1:D5,1)", "e:#N/A"},                  // no logical value among the cells
        };
        CHECK_EVALS(cases, g, Settings());
    }

    // The end of a run of equal cells is found by doubling and halving, not by looking at each cell (a lookup of a date in a sorted column of a hundred thousand
    // rows with one date each of a thousand times must not take the steps of the rows).
    void aLongRunOfEqualCellsCostsNoMoreThanItsLogarithm()
    {
        Grid g;
        QVariantList values;
        for (int i = 0; i < 40000; ++i) values << (i < 10 ? 1 : (i < 39990 ? 5 : 9));
        g.column("A1", values);
        const E cases[] = {
            {"MATCH(5,A1:A40000,1)", "n:39990"},
            {"MATCH(5,A1:A40000,-1)", "n:39990"},
            {"LOOKUP(5,A1:A40000)", "n:5"},
            {"VLOOKUP(5,A1:A40000,1)", "n:5"},
        };
        for (const auto &c : cases) {
            const Outcome o = evOutcome(QString::fromUtf8(c.formula), g);
            QVERIFY2(o.text == QString::fromUtf8(c.value), qPrintable(QString::fromUtf8(c.formula) + QStringLiteral(" gave ") + o.text));
            QVERIFY2(o.work < 400, qPrintable(QString::fromUtf8(c.formula) + QStringLiteral(" took ") + QString::number(o.work) + QStringLiteral(" steps")));
        }
    }

    // A value of more than 255 characters is a #VALUE! in VLOOKUP, HLOOKUP, and MATCH, whatever the mode, whether or not it has a wildcard in it; one of 255 is looked up.
    void aTextValueOfMoreThan255CharactersIsAValueError()
    {
        Grid g;
        g.column("A1", {QString(255, QLatin1Char('a')), QString("b")});
        g.column("B1", {1, 2});
        const E cases[] = {
            {"VLOOKUP(REPT(\"a\",255),A1:B2,2,FALSE)", "n:1"},
            {"VLOOKUP(REPT(\"a\",256),A1:B2,2,FALSE)", "e:#VALUE!"},
            {"VLOOKUP(REPT(\"a\",256),A1:B2,2)", "e:#VALUE!"},
            {"VLOOKUP(REPT(\"a\",256),A1:B2,2,TRUE)", "e:#VALUE!"},
            {"VLOOKUP(REPT(\"a\",255)&\"*\",A1:B2,2,FALSE)", "e:#VALUE!"},
            {"HLOOKUP(REPT(\"a\",256),A1:B2,2,FALSE)", "e:#VALUE!"},
            {"HLOOKUP(REPT(\"a\",256),A1:B2,2)", "e:#VALUE!"},
            {"MATCH(REPT(\"a\",256),A1:A2,0)", "e:#VALUE!"},
            {"MATCH(REPT(\"a\",256),A1:A2,1)", "e:#VALUE!"},
            {"MATCH(REPT(\"a\",256),A1:A2)", "e:#VALUE!"},
            {"MATCH(REPT(\"a\",255),A1:A2,0)", "n:1"},
        };
        CHECK_EVALS(cases, g, Settings());
    }

    void matchInItsThreeModes()
    {
        const Grid g = sample();
        const E cases[] = {
            {"MATCH(\"b\",{\"a\",\"b\"},0)", "n:2"},
            {"MATCH(5,G1:G5,0)", "n:4"},
            {"MATCH(99,G1:G5,0)", "e:#N/A"},
            {"MATCH(\"B\",A1:A4,0)", "n:2"},                   // the first, without regard to case
            {"MATCH(\"c*\",A1:A4,0)", "n:3"},
            {"MATCH(\"?\",A1:A4,0)", "n:1"},
            {"MATCH(2,B1:B4,0)", "n:2"},
            {"MATCH(\"2\",B1:B4,0)", "e:#N/A"},
            {"MATCH(TRUE,{FALSE,TRUE},0)", "n:2"},
            {"MATCH(1,{1,2},0)", "n:1"},
            {"MATCH(1,A1:C4,0)", "e:#N/A"},                    // the range must be one row or one column
            {"MATCH(30,E1:E5,0)", "n:3"},
            {"MATCH(25,E1:E5)", "n:2"},                         // the type left out is 1
            {"MATCH(25,E1:E5,)", "e:#N/A"},                     // and an empty one is 0: an exact match
            {"MATCH(20,E1:E5,)", "n:2"},
            {"MATCH(20,E1:E5,1)", "n:2"},
            {"MATCH(5,E1:E5,1)", "e:#N/A"},
            {"MATCH(100,E1:E5,1)", "n:5"},
            {"MATCH(\"cat\",H1:H4,1)", "n:2"},
            // not in order: G1:G5 = 30 10 20 5 40
            {"MATCH(25,G1:G5,1)", "n:4"},
            {"MATCH(5,G1:G5,1)", "e:#N/A"},                    // 5 is there, but the halving does not meet it
            {"MATCH(45,G1:G5,1)", "n:5"},
            // descending: K1:K5 = 50 40 30 20 10, the smallest value not less than the one looked for
            {"MATCH(25,K1:K5,-1)", "n:3"},
            {"MATCH(30,K1:K5,-1)", "n:3"},
            {"MATCH(60,K1:K5,-1)", "e:#N/A"},
            {"MATCH(5,K1:K5,-1)", "n:5"},
            {"MATCH(\"a\",N1:Q1,0)", "n:1"},
            {"MATCH(\"d\",N1:Q1,0)", "n:4"},
            {"MATCH(\"e\",N1:Q1,0)", "e:#N/A"},
            {"MATCH(#N/A,N1:Q1,0)", "e:#N/A"},
        };
        CHECK_EVALS(cases, g, Settings());
    }

    // The approximate mode compares two numbers exactly: -0.09999999999999998 is after -0.1, though they agree to 15 digits and the = operator calls them the same.
    void numbersInALookupAreComparedExactly()
    {
        Grid g;
        g.column("A1", {-0.12, -0.11, -0.09999999999999998, -0.09});
        g.column("B1", {QString("a"), QString("b"), QString("c"), QString("d")});
        const E cases[] = {
            {"-0.1=A3", "b:1"},
            {"VLOOKUP(-0.1,A1:B4,2)", "s:b"},
            {"MATCH(-0.1,A1:A4,1)", "n:2"},
            {"VLOOKUP(-0.09999999999999998,A1:B4,2)", "s:c"},
        };
        CHECK_EVALS(cases, g, Settings());
    }

    // CELL tells where a cell is and what it holds; what depends on the file or the program (the file's name, the width of a column) is not worked out here.
    void cellTellsAboutACell()
    {
        Grid g;
        g.n("A1", 5).s("A2", "text");
        const E cases[] = {
            {"CELL(\"address\",C5)", "s:$C$5"},
            {"CELL(\"col\",C5)", "n:3"},
            {"CELL(\"row\",C5)", "n:5"},
            {"CELL(\"contents\",A1)", "n:5"},
            {"CELL(\"type\",A1)", "s:v"},
            {"CELL(\"type\",A2)", "s:l"},
            {"CELL(\"type\",A3)", "s:b"},
            {"CELL(\"ADDRESS\",Sheet1!B2)", "s:$B$2"},
            {"CELL(\"nonsense\",A1)", "e:#VALUE!"},
        };
        CHECK_EVALS(cases, g, Settings());
        // not worked out: the engine says so (the yardstick then lists the cell apart)
        const ParseResult p = parseFormula(QStringLiteral("CELL(\"filename\",A1)"));
        QVERIFY(p.ok());
        Evaluator evaluator;
        EvalContext ctx;
        ctx.source = &g;
        ctx.row = 9;
        ctx.col = 9;
        evaluator.evaluate(*p.program, ctx);
        QCOMPARE(evaluator.unsupportedKinds(), 4);
        const ParseResult q = parseFormula(QStringLiteral("SUM(1,2)"));
        evaluator.evaluate(*q.program, ctx);
        QCOMPARE(evaluator.unsupportedKinds(), 0);
    }

    void hlookupAndLookup()
    {
        const Grid g = sample();
        const E cases[] = {
            {"HLOOKUP(\"c\",N1:Q2,2,FALSE)", "n:3"},
            {"HLOOKUP(\"e\",N1:Q2,2,FALSE)", "e:#N/A"},
            {"HLOOKUP(\"C\",N1:Q2,2,FALSE)", "n:3"},
            {"HLOOKUP(\"c\",N1:Q2,3,FALSE)", "e:#REF!"},
            {"HLOOKUP(\"c\",N1:Q2,0,FALSE)", "e:#VALUE!"},
            {"HLOOKUP(\"cat\",N1:Q2,2)", "n:3"},               // approximate: "c" is the last not after "cat"
            {"HLOOKUP(2.5,{1,2,3;\"x\",\"y\",\"z\"},2,TRUE)", "s:y"},
            {"HLOOKUP(0,{1,2,3;\"x\",\"y\",\"z\"},2)", "e:#N/A"},
            {"LOOKUP(5,{1,3,7},{\"a\",\"b\",\"c\"})", "s:b"},
            {"LOOKUP(0,{1,3,7},{\"a\",\"b\",\"c\"})", "e:#N/A"},
            {"LOOKUP(7,{1,3,7},{\"a\",\"b\",\"c\"})", "s:c"},
            {"LOOKUP(100,{1,3,7},{\"a\",\"b\",\"c\"})", "s:c"},
            {"LOOKUP(25,E1:E5,F1:F5)", "s:twenty"},
            {"LOOKUP(25,E1:E5)", "n:20"},
            {"LOOKUP(2,E1:E5,F1:F5)", "e:#N/A"},
            {"LOOKUP(\"b\",{\"a\",\"b\",\"c\"},{1,2,3})", "n:2"},
            // the last match: 1/(test) is an error where the test fails, and the search passes over errors
            {"LOOKUP(2,1/(A1:A4=\"b\"),C1:C4)", "s:w"},
            {"LOOKUP(2,1/(E1:E5>15),F1:F5)", "s:fifty"},
        };
        CHECK_EVALS(cases, g, Settings());
    }

    void indexPicksFromARangeOrAnArray()
    {
        const Grid g = sample();
        const E cases[] = {
            {"INDEX(A1:C4,2,3)", "s:y"},
            {"INDEX(A1:A4,3)", "s:c"},
            {"INDEX(A1:C1,2)", "n:1"},                          // one row: the number counts the columns
            {"INDEX(A1:C4,4,1)", "s:B"},
            {"INDEX(A1:C4,5,1)", "e:#REF!"},
            {"INDEX(A1:C4,1,4)", "e:#REF!"},
            {"INDEX(A1:C4,-1,1)", "e:#VALUE!"},
            {"INDEX(A1:A4,\"2\")", "s:b"},
            {"SUM(INDEX(A1:C4,0,2))", "n:10"},                  // a 0 takes the whole column
            {"SUM(INDEX(B1:C4,2,0))", "n:2"},                   // or the whole row
            {"INDEX({1,2;3,4},2,1)", "n:3"},
            {"INDEX({1,2,3},2)", "n:2"},
            {"INDEX({1,2,3},0,3)", "n:3"},
            {"SUM(INDEX(E1:E5,2):INDEX(E1:E5,4))", "n:90"},     // a range between two cells that INDEX points at
            {"SUM(E1:INDEX(E1:E5,3))", "n:60"},
            {"INDEX(E1:E5,3)+1", "n:31"},
            {"ROWS(INDEX(A1:C4,0,1))", "n:4"},
            {"INDEX(F1:F5,MATCH(30,E1:E5,0))", "s:thirty"},
            {"INDEX(A1:C4,MATCH(\"c\",A1:A4,0),MATCH(3,B1:B4,0))", "s:z"},
            {"INDEX(N1:Q1,2)", "s:b"},
        };
        CHECK_EVALS(cases, g, Settings());
    }

    void offsetAndIndirectMakeReferences()
    {
        Grid g = sample();
        g.addSheet("Sheet2");
        g.n("A1", 7, 1).n("B2", 8, 1);
        g.s("P9", "B2").s("P10", "E1:E3").s("P11", "R2C2").name("MyName", "Sheet1!$E$3");
        const E cases[] = {
            {"OFFSET(A1,1,1)", "n:2"},
            {"OFFSET(A1,0,0)", "s:a"},
            {"SUM(OFFSET(E1,0,0,3,1))", "n:60"},
            {"SUM(OFFSET(E1,1,0,2))", "n:50"},                  // the height 2, the width of the reference
            {"SUM(OFFSET(E1:F1,1,0))", "n:20"},
            {"OFFSET(A1,-1,0)", "e:#REF!"},
            {"OFFSET(E1,2,0)", "n:30"},
            {"OFFSET(E5,-4,0)", "n:10"},
            {"OFFSET(A1,1,1,0,1)", "e:#REF!"},
            {"OFFSET(A1,1,1,1,1)", "n:2"},
            {"ROWS(OFFSET(E1,0,0,3,1))", "n:3"},
            {"COLUMNS(OFFSET(E1,0,0,3,2))", "n:2"},
            {"OFFSET(A1,1.9,1.9)", "n:2"},                      // cut to whole numbers
            {"OFFSET(E1,\"1\",0)", "n:20"},
            {"SUM(OFFSET(E1,0,0,5,1))", "n:150"},
            {"INDIRECT(\"B2\")", "n:2"},
            {"INDIRECT(\"E\"&3)", "n:30"},
            {"INDIRECT(\"Sheet2!A1\")", "n:7"},
            {"INDIRECT(\"Sheet2!B2\")", "n:8"},
            {"INDIRECT(\"R2C2\",FALSE)", "n:2"},
            {"INDIRECT(\"bad!\")", "e:#REF!"},
            {"INDIRECT(\"\")", "e:#REF!"},
            {"INDIRECT(\"Sheet9!A1\")", "e:#REF!"},
            {"SUM(INDIRECT(\"E1:E3\"))", "n:60"},
            {"INDIRECT(P9)", "n:2"},
            {"SUM(INDIRECT(P10))", "n:60"},
            {"INDIRECT(P11,FALSE)", "n:2"},
            {"INDIRECT(\"MyName\")", "n:30"},
            {"INDIRECT(\"b2\")", "n:2"},
            {"INDIRECT(\"$B$2\")", "n:2"},
            {"INDIRECT(1)", "e:#REF!"},
            {"INDIRECT(#N/A)", "e:#N/A"},
        };
        CHECK_EVALS(cases, g, Settings());
    }

    // The text of R1C1 relative to a cell is read from the cell it is in: one evaluator working out the same formula in B1, B2, and B3 gives each its own.
    void indirectOfRelativeR1C1IsReadFromItsOwnCell()
    {
        Grid g;
        g.column("A1", {11, 22, 33, 44});
        const QStringList cells = {"B1", "B2", "B3"};
        QCOMPARE(evInCells("INDIRECT(\"R[1]C[-1]\",FALSE)", g, cells), (QStringList{"n:22", "n:33", "n:44"}));
        QCOMPARE(evInCells("INDIRECT(\"RC[-1]\",FALSE)", g, {"B1", "B2", "B3", "B4"}), (QStringList{"n:11", "n:22", "n:33", "n:44"}));
        QCOMPARE(evInCells("SUM(INDIRECT(\"R[0]C[-1]:R[1]C[-1]\",FALSE))", g, cells), (QStringList{"n:33", "n:55", "n:77"}));
        QCOMPARE(evInCells("INDIRECT(\"R[-1]C[-1]\",FALSE)", g, {"B2", "B3", "B4"}), (QStringList{"n:11", "n:22", "n:33"}));
        // the same text in A1 style, and in R1C1 with the cell given whole, are the same at every cell
        QCOMPARE(evInCells("INDIRECT(\"A\"&ROW()+1)", g, cells), (QStringList{"n:22", "n:33", "n:44"}));
        QCOMPARE(evInCells("INDIRECT(\"R2C1\",FALSE)", g, cells), (QStringList{"n:22", "n:22", "n:22"}));
        QCOMPARE(evInCells("INDIRECT(\"A2\")", g, cells), (QStringList{"n:22", "n:22", "n:22"}));
        // and the formula itself written in R1C1
        Settings r1c1;
        r1c1.r1c1 = true;
        QCOMPARE(evInCells("INDIRECT(\"R[1]C[-1]\",FALSE)", g, cells, r1c1), (QStringList{"n:22", "n:33", "n:44"}));
        // a name that moves with the cell is a different thing: it is written for A1
        g.name("NextToLeft", "Sheet1!A2");
        QCOMPARE(evInCells("INDIRECT(\"NextToLeft\")", g, {"A1", "B1"}), (QStringList{"n:22", "n:0"}));
    }

    // INDIRECT takes a text that names a reference (a cell, a range, a name, with a sheet or without): not a formula, whose value it would give.
    void indirectDoesNotWorkOutFormulas()
    {
        Grid g = sample();
        g.name("Moving", "OFFSET(Sheet1!$E$1,1,0)");
        g.name("Calc", "Sheet1!$E$1+1");
        const E cases[] = {
            {"INDIRECT(\"OFFSET(A1,1,0)\")", "e:#REF!"},
            {"INDIRECT(\"IF(TRUE,A1,A2)\")", "e:#REF!"},
            {"INDIRECT(\"A1+1\")", "e:#REF!"},
            {"INDIRECT(\"SUM(E1:E3)\")", "e:#REF!"},
            {"INDIRECT(\"1\")", "e:#REF!"},
            {"INDIRECT(\"\"\"x\"\"\")", "e:#REF!"},
            {"INDIRECT(\"TRUE\")", "e:#REF!"},
            {"SUM(INDIRECT(\"E1:E3\"))", "n:60"},
            {"SUM(INDIRECT(\"Sheet1!E1:E3\"))", "n:60"},
            {"INDIRECT(\"Moving\")", "n:20"},         // a name stands for what it was given, a function or not
            {"INDIRECT(\"Calc\")", "e:#REF!"},
            {"INDIRECT(\"R2C2\",FALSE)", "n:2"},
            {"INDIRECT(\"R[-8]C[-8]\",FALSE)", "n:2"},
            {"INDIRECT(\"OFFSET(R1C1,1,0)\",FALSE)", "e:#REF!"},
        };
        CHECK_EVALS(cases, g, Settings());
    }

    void rowsColumnsAndAddresses()
    {
        const Grid g = sample();
        const E cases[] = {
            {"ROW(A5)", "n:5"},
            {"COLUMN(C1)", "n:3"},
            {"ROW()", "n:10"},
            {"COLUMN()", "n:10"},
            {"ROWS(A1:A10)", "n:10"},
            {"COLUMNS(A1:C1)", "n:3"},
            {"ROWS(E1:F5)", "n:5"},
            {"COLUMNS(E1:F5)", "n:2"},
            {"ROW(A1:A3)", "n:1"},                              // in a formula for one value: the first
            {"COLUMN(A1:C1)", "n:1"},
            {"ROWS({1,2;3,4})", "n:2"},
            {"COLUMNS({1,2;3,4})", "n:2"},
            {"ROW(Sheet1!A7)", "n:7"},
            {"ROWS(A:A)", "n:1048576"},
            {"COLUMNS(A:C)", "n:3"},
            {"ROW(1:3)", "n:1"},
            {"ROW(1)", "e:#VALUE!"},
            {"ROW(\"a\")", "e:#VALUE!"},
            {"ADDRESS(1,1)", "s:$A$1"},
            {"ADDRESS(2,3,4)", "s:C2"},
            {"ADDRESS(2,3,2)", "s:C$2"},
            {"ADDRESS(2,3,3)", "s:$C2"},
            {"ADDRESS(1,1,1,FALSE)", "s:R1C1"},
            {"ADDRESS(2,3,4,FALSE)", "s:R[2]C[3]"},
            {"ADDRESS(1,1,1,TRUE,\"Sheet2\")", "s:Sheet2!$A$1"},
            {"ADDRESS(1,1,1,TRUE,\"My Sheet\")", "s:'My Sheet'!$A$1"},
            {"ADDRESS(0,1)", "e:#VALUE!"},
            {"ADDRESS(1,16385)", "e:#VALUE!"},
            {"ADDRESS(5,28)", "s:$AB$5"},
            {"ISREF(A1)", "b:1"},
            {"ISREF(1)", "b:0"},
            {"ISREF(A1:B2)", "b:1"},
            {"ISREF(OFFSET(A1,1,1))", "b:1"},
            {"ISREF(\"A1\")", "b:0"},
        };
        CHECK_EVALS(cases, g, Settings());
        Settings array;
        array.array = true;
        const E arrays[] = {
            {"ROW(A1:A3)", "a:3x1[n:1;n:2;n:3]"},
            {"COLUMN(A1:C1)", "a:1x3[n:1,n:2,n:3]"},
            {"TRANSPOSE(E1:E3)", "a:1x3[n:10,n:20,n:30]"},
            {"TRANSPOSE({1,2;3,4})", "a:2x2[n:1,n:3;n:2,n:4]"},
            {"ROW(A1:A2)*COLUMN(A1:C1)", "a:2x3[n:1,n:2,n:3;n:2,n:4,n:6]"},
            {"INDEX(E1:E5,{1;3})", "a:2x1[n:10;n:30]"},
        };
        CHECK_EVALS(arrays, g, array);
    }
};

JP_TEST_CLASS(SheetFnLookupTests)
#include "tst_sheet_fn_lookup.moc"
