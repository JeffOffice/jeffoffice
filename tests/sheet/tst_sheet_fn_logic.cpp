// Tests of the logical and information functions: AND, OR, NOT, the IS functions, type conversions, and the errors a formula can make on purpose. Each
// expected value is written from the function's definition and the rules of the language, not from what the code gave.
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

// A1 = 5, A2 = "text", A3 = TRUE, A4 = #DIV/0!, A5 = #N/A, A6 = "", A7 empty; B1:B3 = 1, 0, 2; C1:C2 = "x", "y"
Grid sample()
{
    Grid g;
    g.n("A1", 5).s("A2", "text").b("A3", true).e("A4", ErrorKind::Div0).e("A5", ErrorKind::NA).s("A6", "");
    g.column("B1", {1, 0, 2});
    g.column("C1", {QString("x"), QString("y")});
    return g;
}

} // namespace

class SheetFnLogicTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void andOrNotXor()
    {
        const Grid g = sample();
        const E cases[] = {
            {"AND(TRUE,1)", "b:1"},
            {"AND(TRUE,0)", "b:0"},
            {"AND(TRUE,FALSE,TRUE)", "b:0"},
            {"AND(B1:B3)", "b:0"},                  // a number in a range is true unless it is 0
            {"AND(B1,B3)", "b:1"},
            {"AND(C1:C2)", "e:#VALUE!"},            // a range with no boolean or number
            {"AND(C1:C2,TRUE)", "b:1"},             // text in a range is left out
            {"AND(\"x\")", "e:#VALUE!"},
            {"AND(A4,TRUE)", "e:#DIV/0!"},
            {"AND(FALSE,A4)", "e:#DIV/0!"},         // an error passes though the answer is known
            {"AND(1,2,3)", "b:1"},
            {"AND(A9:A10)", "e:#VALUE!"},
            {"OR(FALSE,1)", "b:1"},
            {"OR(FALSE,0)", "b:0"},
            {"OR(B1:B3)", "b:1"},
            {"OR(0,0)", "b:0"},
            {"OR(C1:C2)", "e:#VALUE!"},
            {"OR(TRUE,A5)", "e:#N/A"},
            {"NOT(TRUE)", "b:0"},
            {"NOT(0)", "b:1"},
            {"NOT(5)", "b:0"},
            {"NOT(\"a\")", "e:#VALUE!"},
            {"NOT(A9)", "b:1"},
            {"NOT(A4)", "e:#DIV/0!"},
            {"_xlfn.XOR(TRUE,FALSE)", "b:1"},
            {"_xlfn.XOR(TRUE,TRUE)", "b:0"},
            {"_xlfn.XOR(TRUE,TRUE,TRUE)", "b:1"},
            {"TRUE()", "b:1"},
            {"FALSE()", "b:0"},
            {"IF(AND(A1>1,A1<10),\"in\",\"out\")", "s:in"},
            {"IF(OR(A1<1,A1>10),\"out\",\"in\")", "s:in"},
        };
        CHECK_EVALS(cases, g, Settings());
    }

    void theIsFunctionsAskWhatAValueIs()
    {
        const Grid g = sample();
        const E cases[] = {
            {"ISNUMBER(A1)", "b:1"}, {"ISNUMBER(A2)", "b:0"}, {"ISNUMBER(A3)", "b:0"}, {"ISNUMBER(A4)", "b:0"}, {"ISNUMBER(A7)", "b:0"}, {"ISNUMBER(\"1\")", "b:0"}, {"ISNUMBER(1)", "b:1"},
            {"ISTEXT(A2)", "b:1"}, {"ISTEXT(A6)", "b:1"}, {"ISTEXT(A1)", "b:0"}, {"ISTEXT(A7)", "b:0"}, {"ISTEXT(\"x\")", "b:1"},
            {"ISNONTEXT(A1)", "b:1"}, {"ISNONTEXT(A2)", "b:0"}, {"ISNONTEXT(A7)", "b:1"},
            {"ISLOGICAL(A3)", "b:1"}, {"ISLOGICAL(A1)", "b:0"}, {"ISLOGICAL(1=1)", "b:1"},
            {"ISBLANK(A7)", "b:1"}, {"ISBLANK(A6)", "b:0"},      // an empty text is not blank
            {"ISBLANK(A1)", "b:0"}, {"ISBLANK(\"\")", "b:0"},
            {"ISERROR(A4)", "b:1"}, {"ISERROR(A5)", "b:1"}, {"ISERROR(A1)", "b:0"}, {"ISERROR(1/0)", "b:1"}, {"ISERROR(\"#N/A\")", "b:0"},
            {"ISERR(A4)", "b:1"}, {"ISERR(A5)", "b:0"},          // ISERR is any error but #N/A
            {"ISNA(A5)", "b:1"}, {"ISNA(A4)", "b:0"}, {"ISNA(NA())", "b:1"},
            {"ISEVEN(2)", "b:1"}, {"ISEVEN(3)", "b:0"}, {"ISODD(3)", "b:1"}, {"ISEVEN(-2)", "b:1"}, {"ISODD(2.9)", "b:0"}, {"ISEVEN(\"a\")", "e:#VALUE!"},
            {"NA()", "e:#N/A"},
            {"N(A1)", "n:5"}, {"N(A2)", "n:0"}, {"N(A3)", "n:1"}, {"N(A4)", "e:#DIV/0!"}, {"N(A7)", "n:0"},
            {"TYPE(1)", "n:1"}, {"TYPE(\"a\")", "n:2"}, {"TYPE(TRUE)", "n:4"}, {"TYPE(#N/A)", "n:16"}, {"TYPE({1,2})", "n:64"}, {"TYPE(A7)", "n:1"}, {"TYPE(A2)", "n:2"},
            {"ERROR.TYPE(#NULL!)", "n:1"}, {"ERROR.TYPE(#DIV/0!)", "n:2"}, {"ERROR.TYPE(#VALUE!)", "n:3"}, {"ERROR.TYPE(#REF!)", "n:4"}, {"ERROR.TYPE(#NAME?)", "n:5"},
            {"ERROR.TYPE(#NUM!)", "n:6"}, {"ERROR.TYPE(#N/A)", "n:7"}, {"ERROR.TYPE(1)", "e:#N/A"},
            {"IFERROR(A4,\"none\")", "s:none"}, {"IFERROR(A1,\"none\")", "n:5"}, {"_xlfn.IFNA(A5,\"none\")", "s:none"}, {"_xlfn.IFNA(A4,\"none\")", "e:#DIV/0!"},
            {"SUMPRODUCT(--ISNUMBER(B1:B3))", "n:3"},
            {"COUNT(B1:B3)=ROWS(B1:B3)", "b:1"},
            {"ISERROR(MATCH(\"zz\",C1:C2,0))", "b:1"},
            {"IF(ISERROR(VLOOKUP(\"zz\",C1:C2,1,FALSE)),\"missing\",\"found\")", "s:missing"},
        };
        CHECK_EVALS(cases, g, Settings());
    }

    void conditionsOnReferencesAndEmpties()
    {
        const Grid g = sample();
        const E cases[] = {
            {"IF(A7,1,2)", "n:2"},
            {"IF(A6,1,2)", "e:#VALUE!"},             // an empty text is no boolean
            {"IF(A3,1,2)", "n:1"},
            {"IF(B2,1,2)", "n:2"},
            {"IF(B1:B3,1,2)", "e:#VALUE!"},          // a range crossing no row
            {"IF(A1>3,\"big\",\"small\")", "s:big"},
            {"IF(A2=\"TEXT\",1,0)", "n:1"},
            {"IF(A9=\"\",1,0)", "n:1"},
            {"IF(ISBLANK(A9),\"blank\",\"filled\")", "s:blank"},
        };
        CHECK_EVALS(cases, g, Settings());
    }
};

JP_TEST_CLASS(SheetFnLogicTests)
#include "tst_sheet_fn_logic.moc"
