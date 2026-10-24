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

    // The rate of IRR and XIRR is where the value of the flows is nothing: a guess close to -1 does not end the search early because the steps there are small (a step of a
    // billionth is not a rate found), and the search does not leave the rates above -1.
    void aRateIsFoundWhereTheValueIsNothingFromAGuessNearMinusOne()
    {
        const E cases[] = {
            {"ROUND(IRR({-100,110},-0.999999999),6)", "n:0.1"},
            {"ROUND(IRR({-100,110},-0.9999999),6)", "n:0.1"},
            {"ROUND(IRR({-100,50,60},-0.99999999),6)", "n:0.063941"},
            {"ROUND(IRR({-100,50,60},-0.999),6)", "n:0.063941"},
            {"ROUND(IRR({-100,50,60},-0.99),6)", "n:0.063941"},
            {"ROUND(IRR({-100,50,60},0.5),6)", "n:0.063941"},
            {"ROUND(IRR({-100,50,60},100),6)", "n:0.063941"},
            {"ROUND(XIRR({-100,110},{40000,40365},-0.999999999),6)", "n:0.1"},
            {"ROUND(XIRR({-100,110},{40000,40365},-0.9999),6)", "n:0.1"},
            {"ROUND(XIRR({-100,50,60},{40000,40365,40730},-0.99999999),6)", "n:0.063941"},
            {"ROUND(-100+110/(1+IRR({-100,110},-0.999999999)),9)", "n:0"},   // the value at the rate it gives is nothing
            {"IRR({1,-2,3})", "e:#NUM!"},                                  // a rate where there is none
            {"IRR({1,2,3},-0.5)", "e:#NUM!"},
        };
        CHECK_EVALS(cases, Grid(), Settings());
    }

    // A logical or a text in a reference is not a flow: IRR leaves it out, as it does from an array (a logical is not counted as 1).
    void referencesIgnoreLogicalsAndText()
    {
        Grid g;
        g.n("C1", -10).b("C2", true).n("C3", 20);
        g.n("D1", -10).s("D2", "x").n("D3", 20);
        const E cases[] = {
            {"IRR(C1:C3)", "n:1"},
            {"IRR(D1:D3)", "n:1"},
            {"IRR({-10,TRUE,20})", "n:1"},
        };
        CHECK_EVALS(cases, g, Settings());
    }

    // XIRR and XNPV take the values and the dates as pairs by position (the n-th value is on the n-th date), whatever the cells hold: an empty cell is 0, a number is a number,
    // and anything else (a text or a logical, in either) is the error #VALUE! (the dates that are no dates are, in the descriptions of the functions); as many of each, else
    // #NUM!. A logical in one range and not at that place in the other does not pair a value with the date of another.
    void valuesAndDatesArePairsByPosition()
    {
        Grid g;
        // values -100, 50, TRUE, 60 and dates 40000, TRUE, 40365, 40730: leaving out the logicals one range at a time would pair 50 with 40365
        g.n("A1", -100).n("A2", 50).b("A3", true).n("A4", 60);
        g.n("B1", 40000).b("B2", true).n("B3", 40365).n("B4", 40730);
        // text in the values (and the same count), and text in the dates
        g.n("C1", -100).s("C2", "x").n("C3", 110);
        g.n("D1", 40000).n("D2", 40100).n("D3", 40365);
        g.n("E1", -100).n("E2", 0).n("E3", 110);
        g.n("F1", 40000).s("F2", "x").n("F3", 40365);
        // an empty cell among the values is a flow of nothing; the dates are all there
        g.n("G1", -100).n("G3", 110);
        g.n("H1", 40000).n("H2", 40100).n("H3", 40365);
        // an empty date is the day 0, before the first
        g.n("I1", -100).n("I2", 5).n("I3", 110);
        g.n("J1", 40000).n("J3", 40365);
        const E cases[] = {
            {"XIRR(A1:A4,B1:B4)", "e:#VALUE!"},
            {"XNPV(0.1,A1:A4,B1:B4)", "e:#VALUE!"},
            {"XIRR(C1:C3,D1:D3)", "e:#VALUE!"},
            {"XNPV(0.1,C1:C3,D1:D3)", "e:#VALUE!"},
            {"XIRR(E1:E3,F1:F3)", "e:#VALUE!"},
            {"XNPV(0.1,E1:E3,F1:F3)", "e:#VALUE!"},
            {"ROUND(XIRR(G1:G3,H1:H3),6)", "n:0.1"},
            {"ROUND(XNPV(0.1,G1:G3,H1:H3),6)", "n:0"},
            {"XIRR(I1:I3,J1:J3)", "e:#NUM!"},
            {"XIRR({-100,50,110},{40000,40365})", "e:#NUM!"},          // as many dates as values
            {"XIRR({-100,TRUE,110},{40000,40100,40365})", "e:#VALUE!"},
            {"ROUND(XIRR({-100,110},{40000,40365}),6)", "n:0.1"},
        };
        CHECK_EVALS(cases, g, Settings());
    }

    // VDB for a stretch that begins or ends part way through a period takes the part of the period's amount that the stretch covers, and the amounts of stretches that make
    // up a longer one make up its amount, whatever the factor and whether or not the straight line is gone over to.
    void depreciationOfPartsOfPeriodsAddsUp()
    {
        auto vdb = [](const QString &args) {
            const QString got = ev(QStringLiteral("VDB(") + args + QLatin1Char(')'));
            [&] { QVERIFY2(got.startsWith(QStringLiteral("n:")), qPrintable(args + QStringLiteral(" gave ") + got)); }();
            return got.mid(2).toDouble();
        };
        QVERIFY(!QTest::currentTestFailed());
        struct Case {
            const char *asset;
            const char *tail;   // the factor and no_switch
        };
        const Case assets[] = {{"2400,300,10", ""}, {"2400,300,10", ",1.5"}, {"2400,300,10", ",3"}, {"2400,300,10", ",2,TRUE"}, {"10000,0,7.5", ",2"}, {"1500,100,5", ",1.2"}, {"500,50,3", ",4"}};
        const double points[] = {0, 0.5, 1, 1.25, 2, 2.5, 3.3, 4, 5.75, 6, 7, 8.2, 9, 10};
        for (const Case &a : assets) {
            const double life = QString::fromLatin1(a.asset).split(',')[2].toDouble();
            for (size_t i = 0; i < sizeof(points) / sizeof(points[0]); ++i)
                for (size_t j = i + 1; j < sizeof(points) / sizeof(points[0]); ++j)
                    for (size_t k = j + 1; k < sizeof(points) / sizeof(points[0]); ++k) {
                        if (points[k] > life) continue;
                        const auto part = [&](double from, double to) { return vdb(QStringLiteral("%1,%2,%3%4").arg(QString::fromLatin1(a.asset)).arg(from).arg(to).arg(QString::fromLatin1(a.tail))); };
                        const double whole = part(points[i], points[k]), sum = part(points[i], points[j]) + part(points[j], points[k]);
                        QVERIFY2(std::fabs(whole - sum) <= 1e-9 * std::max(1.0, std::fabs(whole)),
                                 qPrintable(QStringLiteral("%1%2: from %3 to %4 is %5, and by way of %6 it is %7").arg(QString::fromLatin1(a.asset), QString::fromLatin1(a.tail)).arg(points[i]).arg(points[k]).arg(whole, 0, 'f', 9).arg(points[j]).arg(sum, 0, 'f', 9)));
                    }
        }
        const E cases[] = {
            {"ROUND(VDB(2400,300,10,2,4),6)", "n:552.96"},
            {"ROUND(VDB(2400,300,10,2,2.5)+VDB(2400,300,10,2.5,4),6)", "n:552.96"},
            {"ROUND(VDB(2400,300,10,0,0.875,1.5),6)", "n:315"},
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

    // The loan and savings functions are worked out in the order the files show, to their last digits. PV is made in double arithmetic with the power made as the ^
    // operator makes it, and (power - 1) / rate worked out before it is multiplied by the payment; NPV divides each value by the power of 1 + rate (a double) and adds the
    // quotients, as doubles, from the last to the first. Each expected value is the result a workbook of the practice set stored with the formula, to 15 digits (a
    // difference in the 16th and 17th is not seen here): the same worked out in the widest type, or in another order, differs in the 14th or 15th digit in 68 cells
    // of PV and 123 of NPV.
    void presentValuesAreWorkedOutInTheOrderOfTheFiles()
    {
        const E cases[] = {
            {"-PV(0.057/12,30,339/30,,0)", "n:315.257709516189"},
            {"-PV(0.0545/12,30,339/30,,0)", "n:316.249885806128"},
            {"-PV(0.052000000000000005/12,30,339/30,,0)", "n:317.246412835253"},
            {"-PV(0.059500000000000004/12,30,339/30,,0)", "n:314.26986184396"},
            {"-PV(0.062/12,30,339/30,,0)", "n:313.286320791819"},
            {"-PV(0.0645/12,30,339/30,,0)", "n:312.307064485212"},
            {"PV(0.057/2,40,-0.35)", "n:8.2899809681198"},
            {"PV(0.0545/2,40,-0.35)", "n:8.46220653613245"},
            {"PV(0.052000000000000005/2,40,-0.35)", "n:8.63983757654465"},
            {"PV(0.059500000000000004/2,40,-0.35)", "n:8.12296493652326"},
            {"PV(0.062/2,40,-0.35)", "n:7.96097038706455"},
            {"PV(0.0645/2,40,-0.35)", "n:7.80381680633254"},
            // a payment at the start of the periods, a future value, and the rate 0
            {"ROUND(PV(0.08/12,240,500,0,1),6)", "n:-60175.660157"},
            {"PV(0,10,-100,-50)", "n:1050"},
        };
        CHECK_EVALS(cases, Grid(), Settings());
    }

    void netPresentValuesAreWorkedOutInTheOrderOfTheFiles()
    {
        const E cases[] = {
            {"NPV(0.14,{2971.5999999999999,10608.611999999999,10290.353640000001,10187.450103600002,10187.450103600002,11024.27636211,11244.761889352201,11694.552364926289,12162.334459523341,12648.827837904277,8769.8539676136334,9559.1408246988613,10419.463498921759,11357.215213824717,12379.36458306894,40480.522186635426,38456.496077303651,36533.671273438464,34706.987709766538,32971.63832427821})-74290", "n:2873.4637848007"},
            {"NPV(0.11,{0,0,0,0,0,0,0,0,0,0,0,20278.583448167043,20231.594449314605,20064.72572973022,20354.978254486396,20133.805406542218,20111.242048039556})", "n:27118.1236441402"},
            {"NPV(0.09,{4160.6237265749805,4281.5267153076838,4225.3346116552439,4551.4995605436834,-8748.4574836101965,-10168.2499490586,-11242.348250328605,-10234.906857368949,-13158.936656617258,-17353.046990676015,-74729.634758884422,-165696.72772564122,-171987.28986387292,-169789.95196906009,-174953.57723573316,-225888.96993078798,-296591.66444099892,-307925.24552010681,-318511.75590712193,-307060.29120875947})", "n:-572770.015812095"},
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
