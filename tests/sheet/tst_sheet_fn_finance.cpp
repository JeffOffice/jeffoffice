// Tests of the financial functions that have a closed form: loans and savings (PMT, PV, FV, NPER, IPMT, PPMT), NPV, and the rates and depreciation made from a
// few numbers. The expected values are the figures in the function descriptions of the format and their definitions, written out by hand (rounded where the figure is).
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

} // namespace

class SheetFnFinanceTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // Depreciation by the declining balance (DDB, and VDB, which goes over to the straight line when that is more) with the figures of the function descriptions, and the
    // rates and values of cash flows that are found by trying (IRR, XIRR) or counted by the day (XNPV): worked out by hand and by bisection, rounded to the places given.
    void declinesAndFlows()
    {
        const E cases[] = {
            {"ROUND(DDB(2400,300,3650,1),6)", "n:1.315068"},          // the first day of ten years
            {"DDB(2400,300,120,1,2)", "n:40"},
            {"ROUND(DDB(2400,300,10,2,1.5),6)", "n:306"},
            {"ROUND(DDB(2400,300,10,10),6)", "n:22.122547"},
            {"DDB(2400,300,10,11)", "e:#NUM!"},
            {"DDB(-1,300,10,1)", "e:#NUM!"},
            {"ROUND(VDB(2400,300,3650,0,1),6)", "n:1.315068"},
            {"VDB(2400,300,120,0,1)", "n:40"},
            {"VDB(2400,300,10,0,1)", "n:480"},
            {"ROUND(VDB(2400,300,120,6,18),6)", "n:396.306053"},
            {"ROUND(VDB(2400,300,120,6,18,1.5),6)", "n:311.808937"},
            {"ROUND(VDB(2400,300,10,0,0.875,1.5),6)", "n:315"},
            {"ROUND(VDB(2400,300,10,0,10),6)", "n:2100"},               // all of it, down to the salvage value
            {"VDB(2400,300,10,5,3)", "e:#NUM!"},
            {"VDB(2400,300,10,0,11)", "e:#NUM!"},
            {"ROUND(IRR({-70000,12000,15000,18000,21000,26000}),6)", "n:0.086631"},
            {"ROUND(IRR({-70000,12000,15000,18000,21000}),6)", "n:-0.021245"},
            {"ROUND(IRR({-70000,12000,15000},-0.5),6)", "n:-0.443507"},   // from a guess below it, the one rate there is
            {"IRR({1,-2,3})", "e:#NUM!"},                                // a sum that never comes to nothing
            {"IRR({1,2,3})", "e:#NUM!"},                                 // no sign changes
            {"ROUND(XNPV(0.09,{-10000,2750,4250,3250,2750},{39448,39508,39751,39859,39904}),4)", "n:2086.6476"},
            {"ROUND(XIRR({-10000,2750,4250,3250,2750},{39448,39508,39751,39859,39904}),6)", "n:0.373363"},
            {"XNPV(0,{100,100},{40000,40365})", "n:200"},
            {"XNPV(0.1,{100,100},{40000,39999})", "e:#NUM!"},             // a date before the first
            {"XNPV(-1,{100,100},{40000,40365})", "e:#NUM!"},
            {"XIRR({1,2},{40000,40365})", "e:#NUM!"},
            {"XIRR({-1,2},{40000})", "e:#NUM!"},                         // as many dates as values
        };
        CHECK_EVALS(cases, Grid(), Settings());
    }

    void loansAndSavings()
    {
        const E cases[] = {
            {"ROUND(PMT(0.05/12,360,200000),2)", "n:-1073.64"},      // a 30-year loan of 200,000 at 5%
            {"PMT(0,10,1000)", "n:-100"},
            {"ROUND(PMT(0.1,2,1000,0,1),4)", "n:-523.8095"},         // payments at the start of each period
            {"ROUND(FV(0.06/12,10,-200,-500,1),2)", "n:2581.4"},
            {"FV(0,10,-100)", "n:1000"},
            {"ROUND(PV(0.08/12,240,500),2)", "n:-59777.15"},
            {"PV(0,10,-100)", "n:1000"},
            {"ROUND(NPER(0.12/12,-100,-1000,10000,1),2)", "n:59.67"},
            {"NPER(0,-100,1000)", "n:10"},
            {"ROUND(IPMT(0.1/12,1,36,8000),2)", "n:-66.67"},
            {"ROUND(PPMT(0.1/12,1,24,2000),2)", "n:-75.62"},
            {"IPMT(0.1,1,3,1000,0,1)", "n:0"},                       // nothing is owed in the first period of payments at the start
            {"ROUND(IPMT(0.1,2,3,1000),4)", "n:-69.7885"},
            {"ROUND(PMT(0.1,3,1000),6)", "n:-402.114804"},
            {"ROUND(IPMT(0.1,2,3,1000)+PPMT(0.1,2,3,1000)-PMT(0.1,3,1000),6)", "n:0"},   // a payment is interest and principal
            {"PMT(0.1,0,1000)", "e:#NUM!"},
            {"PMT(\"x\",3,1000)", "e:#VALUE!"},
        };
        CHECK_EVALS(cases, Grid(), Settings());
    }

    void presentValuesAndDepreciation()
    {
        Grid g;
        g.column("A1", {-10000, 3000, 4200, 6800});
        const E cases[] = {
            {"ROUND(NPV(0.1,-10000,3000,4200,6800),2)", "n:1188.44"},
            {"ROUND(NPV(0.1,A1:A4),2)", "n:1188.44"},
            {"ROUND(NPV(0.1,A1:A2,4200,6800),2)", "n:1188.44"},
            {"NPV(0.1,\"x\")", "e:#VALUE!"},
            {"SLN(30000,7500,10)", "n:2250"},
            {"SLN(30000,7500,0)", "e:#DIV/0!"},
            {"ROUND(SYD(30000,7500,10,1),4)", "n:4090.9091"},
            {"SYD(30000,7500,10,11)", "e:#NUM!"},
            {"ROUND(EFFECT(0.0525,4),8)", "n:0.05354267"},
            {"ROUND(NOMINAL(0.053543,4),8)", "n:0.05250032"},
        };
        CHECK_EVALS(cases, g, Settings());
    }
};

JP_TEST_CLASS(SheetFnFinanceTests)
#include "tst_sheet_fn_finance.moc"
