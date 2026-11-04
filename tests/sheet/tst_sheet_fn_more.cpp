// Tests of the functions added for the first release and 1.0: more number functions, the distributions and tests of statistics, and fitted lines. Each expected value is
// the one the published definition of the function gives (the examples of its documentation, or worked out by hand from the formula).
#include "calchelpers.h"

#include <QElapsedTimer>
#include <QRegularExpression>

using namespace jp::calc;

namespace {

struct E {
    const char *formula;
    const char *value;   // "~1.2345" is a number agreeing to the decimals written; any other text is the exact result
};

// A function added after the first edition is written with its prefix in a file; a test types it without (the prefix is put in).
QString withPrefixes(QString formula)
{
    static const QStringList future = [] {
        QStringList names;
        for (int i = 0; i < functionCount(); ++i)
            if (functionAt(i).flags & kFnFuture) names << QString::fromLatin1(functionAt(i).name);
        std::sort(names.begin(), names.end(), [](const QString &a, const QString &b) { return a.size() > b.size(); });
        return names;
    }();
    for (const QString &n : future) {
        const QRegularExpression re(QStringLiteral("(?<![A-Za-z0-9_.])") + QRegularExpression::escape(n) + QStringLiteral("\\("));
        formula.replace(re, QStringLiteral("_xlfn.") + n + QStringLiteral("("));
    }
    return formula;
}

QString check(const E &c, const Grid &g, const Settings &s)
{
    const QString got = ev(withPrefixes(QString::fromUtf8(c.formula)), g, s);
    const QString want = QString::fromUtf8(c.value);
    if (!want.startsWith(QLatin1Char('~'))) return got == want ? QString() : QStringLiteral("%1 gave [%2], expected [%3]").arg(QString::fromUtf8(c.formula), got, want);
    if (!got.startsWith(QLatin1String("n:"))) return QStringLiteral("%1 gave [%2], expected %3").arg(QString::fromUtf8(c.formula), got, want);
    const QString digits = want.mid(1);
    const int dot = int(digits.indexOf(QLatin1Char('.')));
    const int decimals = dot < 0 ? 0 : int(digits.size()) - dot - 1;
    const double expected = digits.toDouble(), actual = got.mid(2).toDouble();
    const double tolerance = 0.6 * std::pow(10.0, -decimals);
    if (std::fabs(actual - expected) <= tolerance) return QString();
    return QStringLiteral("%1 gave %2, expected %3").arg(QString::fromUtf8(c.formula)).arg(actual, 0, 'g', 15).arg(expected, 0, 'g', 15);
}

#define CHECK_ALL(cases, grid, settings)                                                                                                                    \
    for (const auto &c : cases) {                                                                                                                           \
        const QString problem = check(c, grid, settings);                                                                                                   \
        QVERIFY2(problem.isEmpty(), qPrintable(problem));                                                                                                   \
    }

// A1:A4 = 1 2 3 4; B1:B4 = 2 4 6 9; C1:C5 hold 1 text "x" empty TRUE and an error; D1:D7 = 4 5 8 7 11 4 3; E1:E4 = 1 2 4 8
Grid sample()
{
    Grid g;
    g.column("A1", {1, 2, 3, 4});
    g.column("B1", {2, 4, 6, 9});
    g.n("C1", 1).s("C2", "x").b("C4", true).e("C5", ErrorKind::NA);
    g.column("D1", {4, 5, 8, 7, 11, 4, 3});
    g.column("E1", {1, 2, 4, 8});
    return g;
}

} // namespace

class SheetFnMoreTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void hyperbolicAndReciprocalTrigonometry()
    {
        const E cases[] = {
            {"ACOSH(1)", "n:0"},         {"ACOSH(10)", "~2.993223"},   {"ACOSH(0.5)", "e:#NUM!"},  {"ASINH(-2.5)", "~-1.647231"}, {"ATANH(-0.1)", "~-0.100335"},
            {"ATANH(1)", "e:#NUM!"},     {"ATANH(-1)", "e:#NUM!"},     {"ACOT(2)", "~0.463648"},    {"ACOT(0)", "~1.570796"},       {"ACOTH(6)", "~0.168236"},
            {"ACOTH(0.5)", "e:#NUM!"},   {"ACOTH(1)", "e:#NUM!"},      {"COT(30)", "~-0.15612"},    {"COT(0)", "e:#DIV/0!"},        {"COTH(2)", "~1.037315"},
            {"COTH(0)", "e:#DIV/0!"},    {"CSC(15)", "~1.53778"},      {"CSC(0)", "e:#DIV/0!"},     {"CSCH(1.5)", "~0.469642"},     {"SEC(45)", "~1.903594"},
            {"SECH(1)", "~0.648054"},    {"SEC(\"x\")", "e:#VALUE!"},  {"SQRTPI(1)", "~1.772454"},  {"SQRTPI(2)", "~2.506628"},     {"SQRTPI(-1)", "e:#NUM!"},
            {"SQRTPI(0)", "n:0"},        {"COT(1E9)", "e:#NUM!"},
        };
        CHECK_ALL(cases, Grid(), Settings())
    }

    void factorialsAndBases()
    {
        const E cases[] = {
            {"FACTDOUBLE(6)", "n:48"},       {"FACTDOUBLE(7)", "n:105"},     {"FACTDOUBLE(-1)", "n:1"},        {"FACTDOUBLE(-2)", "e:#NUM!"},  {"FACTDOUBLE(0)", "n:1"},
            {"FACTDOUBLE(6.9)", "n:48"},     {"MULTINOMIAL(2,3,4)", "n:1260"}, {"MULTINOMIAL(1,1)", "n:2"},    {"MULTINOMIAL(-1,2)", "e:#NUM!"}, {"MULTINOMIAL(0)", "n:1"},
            {"ROMAN(499,0)", "s:CDXCIX"},    {"ROMAN(499,1)", "s:LDVLIV"},   {"ROMAN(499,2)", "s:XDIX"},      {"ROMAN(499,3)", "s:VDIV"},     {"ROMAN(499,4)", "s:ID"},
            {"ROMAN(499)", "s:CDXCIX"},      {"ROMAN(499,TRUE)", "s:CDXCIX"}, {"ROMAN(499,FALSE)", "s:ID"},    {"ROMAN(1999)", "s:MCMXCIX"},   {"ROMAN(3999)", "s:MMMCMXCIX"},
            {"ROMAN(0)", "s:"},              {"ROMAN(4000)", "e:#VALUE!"},   {"ROMAN(-1)", "e:#VALUE!"},      {"ROMAN(2013)", "s:MMXIII"},    {"ROMAN(4,0)", "s:IV"},
            {"ROMAN(2013.9)", "s:MMXIII"},   {"ROMAN(5,5)", "e:#VALUE!"},
            {"ARABIC(\"LVII\")", "n:57"},    {"ARABIC(\"mcmxii\")", "n:1912"}, {"ARABIC(\"\")", "n:0"},        {"ARABIC(\"-IV\")", "n:-4"},    {"ARABIC(\"ABC\")", "e:#VALUE!"},
            {"ARABIC(\" XIV \")", "n:14"},
            {"BASE(7,2)", "s:111"},          {"BASE(100,16)", "s:64"},       {"BASE(15,2,10)", "s:0000001111"}, {"BASE(-1,2)", "e:#NUM!"},      {"BASE(5,1)", "e:#NUM!"},
            {"BASE(255,16)", "s:FF"},        {"BASE(35,36)", "s:Z"},         {"BASE(5,37)", "e:#NUM!"},        {"BASE(0,2)", "s:0"},
            {"DECIMAL(\"FF\",16)", "n:255"}, {"DECIMAL(\"111\",2)", "n:7"},  {"DECIMAL(\"zap\",36)", "n:45745"}, {"DECIMAL(\"9\",8)", "e:#NUM!"}, {"DECIMAL(\"1\",1)", "e:#NUM!"},
            {"COMBINA(4,3)", "n:20"},        {"COMBINA(10,3)", "n:220"},     {"COMBINA(0,0)", "n:1"},          {"COMBINA(0,2)", "e:#NUM!"},    {"COMBINA(-1,2)", "e:#NUM!"},
        };
        CHECK_ALL(cases, Grid(), Settings())
    }

    void roundingToMultiplesIgnoresTheSignOfTheMultiple()
    {
        const E cases[] = {
            {"CEILING.PRECISE(4.3)", "n:5"},     {"CEILING.PRECISE(-4.3)", "n:-4"},   {"CEILING.PRECISE(4.3,2)", "n:6"},   {"CEILING.PRECISE(4.3,-2)", "n:6"},
            {"CEILING.PRECISE(-4.3,2)", "n:-4"}, {"CEILING.PRECISE(-4.3,-2)", "n:-4"}, {"CEILING.PRECISE(4.3,0)", "n:0"},   {"ISO.CEILING(4.3)", "n:5"},
            {"ISO.CEILING(-4.3,-2)", "n:-4"},    {"FLOOR.PRECISE(-3.2,-1)", "n:-4"},   {"FLOOR.PRECISE(3.2,1)", "n:3"},     {"FLOOR.PRECISE(-3.2)", "n:-4"},
            {"FLOOR.PRECISE(3.2,-1)", "n:3"},    {"FLOOR.PRECISE(3.2,0)", "n:0"},      {"CEILING.PRECISE(0.3,0.1)", "~0.3"}, {"FLOOR.PRECISE(\"a\")", "e:#VALUE!"},
        };
        CHECK_ALL(cases, Grid(), Settings())
    }

    void sumsOfTwoSeriesAndPowerSeries()
    {
        const E cases[] = {
            {"SUMX2MY2({2,3,9,1,8,7,5},{6,5,11,7,5,4,4})", "n:-55"}, {"SUMX2PY2({2,3,9,1,8,7,5},{6,5,11,7,5,4,4})", "n:521"},
            {"SUMXMY2({2,3,9,1,8,7,5},{6,5,11,7,5,4,4})", "n:79"},   {"SUMXMY2({1,2},{1,2,3})", "e:#N/A"},
            {"SUMXMY2(A1:A4,B1:B4)", "n:39"},                         {"SUMXMY2(C1:C3,C1:C3)", "n:0"},
            {"SUMX2MY2(C1:C5,C1:C5)", "e:#N/A"},
            {"SERIESSUM(2,0,1,{1,2,3})", "n:17"},                    {"ROUND(SERIESSUM(PI()/4,0,2,{1,-0.5,0.041666667,-0.001388889}),6)", "n:0.707103"},
            {"SERIESSUM(0,0,1,{5,6})", "n:5"},                       {"SERIESSUM(2,0,1,{1,\"a\"})", "e:#VALUE!"},
        };
        CHECK_ALL(cases, sample(), Settings())
    }

    void matrices()
    {
        Settings array;
        array.array = true;
        const E cases[] = {
            {"MDETERM({3,6,1;1,1,0;3,10,2})", "~1"},
            {"ROUND(MDETERM({1,3,8,5;1,3,6,1;1,1,1,0;7,3,10,2}),8)", "n:88"},
            {"MDETERM({1,2;3,4})", "~-2"},
            {"MDETERM({1,2,3;4,5,6;7,8,9})", "~0"},
            {"MDETERM({1,2,3})", "e:#VALUE!"},
            {"MDETERM({1,\"a\";3,4})", "e:#VALUE!"},
            {"MDETERM(5)", "n:5"},
            {"MINVERSE({4,-1;2,0})", "a:2x2[n:0,n:0.5;n:-1,n:2]"},
            {"MINVERSE({1,2;2,4})", "e:#NUM!"},
            {"MINVERSE({1,2,3})", "e:#VALUE!"},
            {"MMULT({1,2;3,4},{5,6;7,8})", "a:2x2[n:19,n:22;n:43,n:50]"},
            {"MMULT({1,2,3},{1;2;3})", "a:1x1[n:14]"},
            {"MMULT({1,2},{1,2})", "e:#VALUE!"},
            {"MMULT({1,2},{\"a\";2})", "e:#VALUE!"},
            {"MUNIT(3)", "a:3x3[n:1,n:0,n:0;n:0,n:1,n:0;n:0,n:0,n:1]"},
            {"MUNIT(0)", "e:#VALUE!"},
        };
        for (const auto &c : cases) {
            const QString problem = check(c, Grid(), array);
            QVERIFY2(problem.isEmpty(), qPrintable(problem));
        }
    }

    void sequencesAndRandomArrays()
    {
        Settings array;
        array.array = true;
        const E cases[] = {
            {"SEQUENCE(3)", "a:3x1[n:1;n:2;n:3]"},
            {"SEQUENCE(2,3)", "a:2x3[n:1,n:2,n:3;n:4,n:5,n:6]"},
            {"SEQUENCE(2,2,10,5)", "a:2x2[n:10,n:15;n:20,n:25]"},
            {"SEQUENCE(1,3,0,-1)", "a:1x3[n:0,n:-1,n:-2]"},
            {"SEQUENCE(0)", "e:#CALC!"},
            {"SEQUENCE(2,0)", "e:#CALC!"},
            {"SEQUENCE(1E9)", "e:#NUM!"},
            {"SUM(SEQUENCE(10))", "n:55"},
            {"ROWS(RANDARRAY(4,3))", "n:4"},
            {"COLUMNS(RANDARRAY(4,3))", "n:3"},
            {"RANDARRAY(2,2,5,5,TRUE)", "a:2x2[n:5,n:5;n:5,n:5]"},
            {"RANDARRAY(1,1,3,1)", "e:#VALUE!"},
            {"AND(MIN(RANDARRAY(50,1,10,20))>=10,MAX(RANDARRAY(50,1,10,20))<20)", "b:1"},
            {"AND(MIN(RANDARRAY(50,1,1,6,TRUE))>=1,MAX(RANDARRAY(50,1,1,6,TRUE))<=6)", "b:1"},
        };
        for (const auto &c : cases) {
            const QString problem = check(c, Grid(), array);
            QVERIFY2(problem.isEmpty(), qPrintable(problem));
        }
    }

    void aggregateSkipsWhatItIsToldTo()
    {
        Grid g;
        g.column("A1", {5, 3, 8, 1});
        g.e("A3", ErrorKind::Div0);
        g.n("B1", 2).n("B2", 4).n("B3", 6).n("B4", 8);
        const E cases[] = {
            {"AGGREGATE(9,6,A1:A4)", "n:9"},         {"AGGREGATE(9,0,A1:A4)", "e:#DIV/0!"}, {"AGGREGATE(4,6,A1:A4)", "n:5"},     {"AGGREGATE(5,6,A1:A4)", "n:1"},
            {"AGGREGATE(1,6,A1:A4)", "n:3"},         {"AGGREGATE(2,6,A1:A4)", "n:3"},       {"AGGREGATE(3,6,A1:A4)", "n:3"},     {"AGGREGATE(12,6,A1:A4)", "n:3"},
            {"AGGREGATE(14,6,B1:B4,2)", "n:6"},      {"AGGREGATE(15,6,B1:B4,2)", "n:4"},    {"AGGREGATE(16,6,B1:B4,0.5)", "n:5"}, {"AGGREGATE(17,6,B1:B4,1)", "n:3.5"},
            {"AGGREGATE(18,6,B1:B4,0.5)", "n:5"},    {"AGGREGATE(19,6,B1:B4,1)", "n:2.5"},    {"AGGREGATE(14,6,B1:B4,5)", "e:#NUM!"}, {"AGGREGATE(20,6,B1:B4)", "e:#VALUE!"},
            {"AGGREGATE(9,8,B1:B4)", "e:#VALUE!"},   {"AGGREGATE(9,4,B1:B4)", "n:20"},         {"AGGREGATE(14,6,B1:B4)", "e:#VALUE!"},
            {"AGGREGATE(7,6,B1:B4)", "~2.581989"},   {"AGGREGATE(8,6,B1:B4)", "~2.236068"},  {"AGGREGATE(10,6,B1:B4)", "~6.666667"}, {"AGGREGATE(11,6,B1:B4)", "n:5"},
            {"AGGREGATE(6,6,B1:B4)", "n:384"},       {"AGGREGATE(13,6,{1,2,2,3})", "n:2"},  {"AGGREGATE(9,6,1,2,3)", "n:6"},
        };
        CHECK_ALL(cases, g, Settings())
    }

    void shapeAndSpreadOfNumbers()
    {
        const E cases[] = {
            {"SKEW(3,4,5,2,3,4,5,6,4,7)", "~0.359543"},   {"SKEW.P(3,4,5,2,3,4,5,6,4,7)", "~0.303193"},    {"KURT(3,4,5,2,3,4,5,6,4,7)", "~-0.151799637"},
            {"SKEW(1,2)", "e:#DIV/0!"},                   {"SKEW(2,2,2,2)", "e:#DIV/0!"},                  {"KURT(1,2,3)", "e:#DIV/0!"},
            {"SKEW.P(5)", "e:#DIV/0!"},                   {"SKEW(A1:A4)", "n:0"},                          {"SKEW(C1:C5)", "e:#N/A"},
            {"GEOMEAN(D1:D7)", "~5.476987"},              {"HARMEAN(D1:D7)", "~5.028376"},                 {"GEOMEAN(4,-1)", "e:#NUM!"},
            {"GEOMEAN(C2:C3)", "e:#NUM!"},                {"HARMEAN(0,1)", "e:#NUM!"},                     {"AVEDEV(D1:D7)", "~2.285714"},
            {"DEVSQ(D1:D7)", "n:48"},                     {"DEVSQ(C2:C3)", "e:#NUM!"},                     {"AVEDEV(C1:C1)", "n:0"},
            {"TRIMMEAN({4,5,6,7,2,3,4,5,1,2,3},0.2)", "~3.777778"}, {"TRIMMEAN(A1:A4,0)", "n:2.5"},        {"TRIMMEAN(A1:A4,1)", "e:#NUM!"},
            {"TRIMMEAN(A1:A4,0.5)", "n:2.5"},             {"TRIMMEAN(A1:A4,-0.1)", "e:#NUM!"},
            {"STANDARDIZE(42,40,1.5)", "~1.333333"},      {"STANDARDIZE(1,2,0)", "e:#NUM!"},               {"FISHER(0.75)", "~0.972955"},
            {"FISHER(1)", "e:#NUM!"},                     {"FISHERINV(0.972955)", "~0.75"},                {"FISHERINV(0)", "n:0"},
            {"GAMMALN(4)", "~1.791759"},                  {"GAMMALN(0)", "e:#NUM!"},                       {"GAMMALN.PRECISE(4)", "~1.791759"},
            {"GAMMA(2.5)", "~1.32934"},                   {"GAMMA(0)", "e:#NUM!"},                         {"GAMMA(-1)", "e:#NUM!"},
            {"GAMMA(-1.5)", "~2.363272"},                 {"GAMMA(5)", "n:24"},                            {"PERMUTATIONA(3,2)", "n:9"},
            {"PERMUTATIONA(-1,2)", "e:#NUM!"},            {"PERMUTATIONA(0,0)", "e:#NUM!"},
            {"STDEVA(C1:C4)", "~0.577350"},               {"STDEVPA(C1:C4)", "~0.471405"},                 {"VARA(C1:C4)", "~0.333333"},
            {"VARPA(C1:C4)", "~0.222222"},                {"VARA(1)", "e:#DIV/0!"},                        {"VARPA(C1:C5)", "e:#N/A"},
            {"VARA(1,2,3)", "n:1"},                       {"VARA(1,\"3\")", "n:2"},                        {"VARA(1,\"x\")", "e:#VALUE!"},
            {"VARPA(5)", "n:0"},
        };
        CHECK_ALL(cases, sample(), Settings())
    }

    void probabilitiesOfAListAndRanks()
    {
        Grid g = sample();
        g.column("F1", {0, 1, 2, 3});
        g.column("G1", {0.2, 0.3, 0.1, 0.4});
        g.column("H1", {89, 88, 92, 101, 94, 97, 95});
        const E cases[] = {
            {"PROB(F1:F4,G1:G4,2)", "~0.1"},          {"PROB(F1:F4,G1:G4,1,3)", "~0.8"},     {"PROB(F1:F4,G1:G4,5)", "n:0"},      {"PROB(F1:F4,G1:G3,2)", "e:#N/A"},
            {"PROB(F1:F4,F1:F4,2)", "e:#NUM!"},       {"PROB(F1:F4,G1:G4,3,1)", "~0.8"},
            {"RANK.AVG(94,H1:H7)", "n:4"},            {"RANK.AVG(3,{1,3,3,5})", "n:2.5"},    {"RANK.AVG(3,{1,3,3,5},1)", "n:2.5"}, {"RANK.AVG(2,{1,3,3,5})", "e:#N/A"},
            {"RANK.EQ(94,H1:H7)", "n:4"},             {"RANK.EQ(3,{1,3,3,5})", "n:2"},       {"RANK.EQ(94,H1:H7,1)", "n:4"},
            {"PERCENTILE.EXC({1,2,3,4},0.25)", "~1.25"}, {"PERCENTILE.EXC({1,2,3,4},0.1)", "e:#NUM!"}, {"PERCENTILE.EXC({1,2,3,4},0.99)", "e:#NUM!"},
            {"PERCENTILE.INC({1,2,3,4},0.25)", "~1.75"}, {"QUARTILE.EXC({1,2,3,4,5,6,7,8},1)", "~2.25"}, {"QUARTILE.EXC({1,2,3,4,5,6,7,8},4)", "e:#NUM!"},
            {"QUARTILE.INC({1,2,3,4,5},3)", "n:4"},   {"QUARTILE.INC({1,2,3,4,5},5)", "e:#NUM!"},
            {"PERCENTRANK({13,12,11,8,4,3,2,1,1,1},2)", "~0.333"}, {"PERCENTRANK.INC({13,12,11,8,4,3,2,1,1,1},8)", "~0.666"},
            {"PERCENTRANK.INC({13,12,11,8,4,3,2,1,1,1},5)", "~0.583"}, {"PERCENTRANK.INC({13,12,11,8,4,3,2,1,1,1},5,2)", "~0.58"},
            {"PERCENTRANK.INC({1,2,3},4)", "e:#N/A"}, {"PERCENTRANK.INC({1,2,3},2,0)", "e:#NUM!"}, {"PERCENTRANK.INC({5},5)", "n:1"},
            {"PERCENTRANK.EXC({1,2,3,6,6,6,7,8,9},8)", "~0.8"}, {"PERCENTRANK.EXC({1,2,3,6,6,6,7,8,9},4)", "~0.333"}, {"PERCENTRANK.EXC({1,2,3},1)", "~0.25"},
            {"MODE.SNGL({1,2,2,3,3})", "n:2"},        {"MODE.SNGL({1,2,3})", "e:#N/A"},
            {"MODE.MULT({1,2,3,4,3,2,1,2,3})", "a:2x1[n:2;n:3]"}, {"MODE.MULT({1,2,3})", "e:#N/A"},
            {"FREQUENCY({79,85,78,85,50,81,95,88,97},{70,79,89})", "a:4x1[n:1;n:2;n:4;n:2]"}, {"FREQUENCY(A1:A4,Z1:Z3)", "a:1x1[n:4]"},
            {"FREQUENCY(A1:A4,{2,2})", "a:3x1[n:2;n:0;n:2]"}, {"FREQUENCY(A1:A4,{3,1})", "a:3x1[n:2;n:1;n:1]"},
        };
        Settings array;
        array.array = true;
        CHECK_ALL(cases, g, array)
    }

    void covarianceAndTheErrorOfAFit()
    {
        const E cases[] = {
            {"COVARIANCE.S({3,2,4,5,6},{9,7,12,15,17})", "~6.5"}, {"COVARIANCE.P({3,2,4,5,6},{9,7,12,15,17})", "~5.2"}, {"COVARIANCE.S(A1:A4,B1:B4)", "~3.833333"},
            {"COVARIANCE.S(A1:A1,B1:B1)", "e:#DIV/0!"},           {"COVARIANCE.S(A1:A4,B1:B3)", "e:#N/A"},
            {"STEYX({2,3,9,1,8,7,5},{6,5,11,7,5,4,4})", "~3.305719"}, {"STEYX(B1:B4,A1:A4)", "~0.387298"}, {"STEYX({1,2},{1,2})", "e:#DIV/0!"},
            {"FORECAST.LINEAR(5,B1:B4,A1:A4)", "n:11"},
        };
        CHECK_ALL(cases, sample(), Settings())
    }

    void theNormalFamily()
    {
        const E cases[] = {
            {"NORM.S.DIST(1.333333,TRUE)", "~0.908789"}, {"NORM.S.DIST(0,FALSE)", "~0.398942"}, {"NORM.DIST(42,40,1.5,TRUE)", "~0.908789"}, {"NORM.DIST(42,40,1.5,FALSE)", "~0.109340"},
            {"NORM.INV(0.908789,40,1.5)", "~42.00000"},  {"NORM.S.INV(0.908789)", "~1.33333"}, {"NORM.S.INV(0)", "e:#NUM!"},               {"NORM.S.INV(1)", "e:#NUM!"},
            {"NORM.DIST(1,0,0,TRUE)", "e:#NUM!"},        {"GAUSS(2)", "~0.47725"},              {"PHI(0.75)", "~0.301137"},                  {"GAUSS(0)", "n:0"},
            {"LOGNORM.DIST(4,3.5,1.2,TRUE)", "~0.039084"}, {"LOGNORM.DIST(4,3.5,1.2,FALSE)", "~0.01762"}, {"LOGNORM.DIST(0,1,1,TRUE)", "e:#NUM!"}, {"LOGNORMDIST(4,3.5,1.2)", "~0.039084"},
            {"LOGNORM.INV(0.039084,3.5,1.2)", "~4.0000252"}, {"LOGINV(0.039084,3.5,1.2)", "~4.0000252"}, {"LOGNORM.INV(1.5,3.5,1.2)", "e:#NUM!"},
            {"CONFIDENCE.NORM(0.05,2.5,50)", "~0.692952"}, {"CONFIDENCE(0.05,2.5,50)", "~0.692952"}, {"CONFIDENCE.NORM(0,2.5,50)", "e:#NUM!"}, {"CONFIDENCE.NORM(0.05,0,50)", "e:#NUM!"},
            {"CONFIDENCE.NORM(0.05,2.5,0.5)", "e:#NUM!"}, {"CONFIDENCE.T(0.05,1,50)", "~0.284196855"}, {"CONFIDENCE.T(0.05,1,1)", "e:#DIV/0!"},
            {"Z.TEST({3,6,7,8,6,5,4,2,1,9},4)", "~0.090574"}, {"ZTEST({3,6,7,8,6,5,4,2,1,9},6)", "~0.863043"}, {"Z.TEST({3,6,7,8,6,5,4,2,1,9},4,2)", "~0.040995"},
            {"Z.TEST(A1:A1,4)", "e:#DIV/0!"},
        };
        CHECK_ALL(cases, sample(), Settings())
    }

    void theOtherContinuousDistributions()
    {
        const E cases[] = {
            {"EXPON.DIST(0.2,10,TRUE)", "~0.864665"},       {"EXPON.DIST(0.2,10,FALSE)", "~1.353353"},    {"EXPONDIST(0.2,10,TRUE)", "~0.864665"},  {"EXPON.DIST(-1,10,TRUE)", "e:#NUM!"},
            {"EXPON.DIST(1,0,TRUE)", "e:#NUM!"},
            {"WEIBULL.DIST(105,20,100,TRUE)", "~0.929581"}, {"WEIBULL.DIST(105,20,100,FALSE)", "~0.035589"}, {"WEIBULL(105,20,100,TRUE)", "~0.929581"}, {"WEIBULL.DIST(1,0,1,TRUE)", "e:#NUM!"},
            {"GAMMA.DIST(10.00001131,9,2,FALSE)", "~0.032639"}, {"GAMMA.DIST(10.00001131,9,2,TRUE)", "~0.068094"}, {"GAMMADIST(10.00001131,9,2,TRUE)", "~0.068094"},
            {"GAMMA.INV(0.068094,9,2)", "~10.00001119"},    {"GAMMAINV(0.068094,9,2)", "~10.00001119"},   {"GAMMA.INV(0,9,2)", "n:0"},                {"GAMMA.INV(1,9,2)", "e:#NUM!"},
            {"GAMMA.DIST(-1,9,2,TRUE)", "e:#NUM!"},
            {"BETA.DIST(2,8,10,TRUE,1,3)", "~0.685470581"}, {"BETA.DIST(2,8,10,FALSE,1,3)", "~1.4837646"}, {"BETA.DIST(0.5,2,2,TRUE)", "~0.5"},    {"BETADIST(2,8,10,1,3)", "~0.685470581"},
            {"BETA.INV(0.685470581,8,10,1,3)", "~2.0000000"}, {"BETAINV(0.685470581,8,10,1,3)", "~2.0000000"}, {"BETA.DIST(5,8,10,TRUE,1,3)", "e:#NUM!"}, {"BETA.INV(0,8,10)", "e:#NUM!"},
            {"CHISQ.DIST(0.5,1,TRUE)", "~0.520499878"},     {"CHISQ.DIST(0.5,1,FALSE)", "~0.439391289"},  {"CHISQ.DIST.RT(18.307,10)", "~0.0500006"}, {"CHIDIST(18.307,10)", "~0.0500006"},
            {"CHISQ.INV(0.93,1)", "~3.28302029"},          {"CHISQ.INV.RT(0.050001,10)", "~18.306973"},  {"CHIINV(0.050001,10)", "~18.306973"},     {"CHISQ.DIST(-1,1,TRUE)", "e:#NUM!"},
            {"CHISQ.DIST(1,0,TRUE)", "e:#NUM!"},            {"CHISQ.INV(1,2)", "e:#NUM!"},                {"CHISQ.INV.RT(0,2)", "e:#NUM!"},
            {"F.DIST(15.2069,6,4,TRUE)", "~0.99"},          {"F.DIST(15.2069,6,4,FALSE)", "~0.0012238"},  {"F.DIST.RT(15.2069,6,4)", "~0.01"},      {"FDIST(15.2069,6,4)", "~0.01"},
            {"F.INV(0.01,6,4)", "~0.10930991"},             {"F.INV.RT(0.01,6,12)", "~4.82"},             {"FINV(0.01,6,12)", "~4.82"},              {"F.DIST(-1,6,4,TRUE)", "e:#NUM!"},
            {"F.DIST(1,0,4,TRUE)", "e:#NUM!"},
            {"F.TEST({6,7,9,15,21},{20,28,31,38,40})", "~0.648318"}, {"FTEST({6,7,9,15,21},{20,28,31,38,40})", "~0.648318"}, {"F.TEST({1},{1,2})", "e:#DIV/0!"},
            {"T.DIST(60,1,TRUE)", "~0.99469533"},           {"T.DIST(8,3,FALSE)", "~0.00073691"},         {"T.DIST(-2,5,TRUE)", "~0.050970"},         {"T.DIST(1,0,TRUE)", "e:#NUM!"},
            {"T.DIST.2T(1.959999998,60)", "~0.054645"},     {"T.DIST.RT(1.959999998,60)", "~0.027322"},   {"T.DIST.2T(-1,60)", "e:#NUM!"},           {"TDIST(1.959999998,60,2)", "~0.054645"},
            {"TDIST(1.959999998,60,1)", "~0.027322"},       {"TDIST(1,60,3)", "e:#NUM!"},
            {"T.INV(0.75,2)", "~0.816497"},                 {"T.INV(0.25,2)", "~-0.816497"},              {"T.INV(0.5,7)", "n:0"},                   {"T.INV(0,7)", "e:#NUM!"},
            {"T.INV.2T(0.546449,60)", "~0.606533"},         {"TINV(0.546449,60)", "~0.606533"},           {"T.INV.2T(1,5)", "n:0"},                  {"T.INV.2T(0,5)", "e:#NUM!"},
            {"T.TEST({3,4,5,8,9,1,2,4,5},{6,19,3,2,14,4,5,17,1},2,1)", "~0.196016"}, {"TTEST({3,4,5,8,9,1,2,4,5},{6,19,3,2,14,4,5,17,1},2,2)", "~0.19200"},
            {"T.TEST({3,4,5,8,9,1,2,4,5},{6,19,3,2,14,4,5,17,1},1,3)", "~0.101147"}, {"T.TEST({1,2},{1,2,3},2,1)", "e:#N/A"}, {"T.TEST({1,2},{1,2},3,1)", "e:#NUM!"},
            {"T.TEST({1,2},{1,2},2,4)", "e:#NUM!"},
        };
        CHECK_ALL(cases, Grid(), Settings())
    }

    void theDiscreteDistributions()
    {
        const E cases[] = {
            {"BINOM.DIST(6,10,0.5,FALSE)", "~0.205078125"}, {"BINOM.DIST(6,10,0.5,TRUE)", "~0.828125"},    {"BINOMDIST(6,10,0.5,FALSE)", "~0.205078125"}, {"BINOM.DIST(11,10,0.5,TRUE)", "e:#NUM!"},
            {"BINOM.DIST(1,10,1.5,TRUE)", "e:#NUM!"},        {"BINOM.DIST(0,10,0,FALSE)", "n:1"},          {"BINOM.DIST(10,10,1,FALSE)", "n:1"},          {"BINOM.DIST(5.9,10,0.5,TRUE)", "~0.623046875"},
            {"BINOM.DIST.RANGE(60,0.75,48)", "~0.084"},      {"BINOM.DIST.RANGE(60,0.75,45,50)", "~0.524"}, {"BINOM.DIST.RANGE(60,0.75,61)", "e:#NUM!"}, {"BINOM.DIST.RANGE(10,0.5,5,3)", "e:#NUM!"},
            {"BINOM.INV(6,0.5,0.75)", "n:4"},                {"CRITBINOM(6,0.5,0.75)", "n:4"},             {"BINOM.INV(6,0.5,0)", "n:0"},                 {"BINOM.INV(6,0.5,1)", "n:6"},
            {"BINOM.INV(6,2,0.5)", "e:#NUM!"},
            {"NEGBINOM.DIST(10,5,0.25,FALSE)", "~0.055049"}, {"NEGBINOM.DIST(10,5,0.25,TRUE)", "~0.3135141"}, {"NEGBINOMDIST(10,5,0.25)", "~0.055049"},  {"NEGBINOM.DIST(1,0,0.5,FALSE)", "e:#NUM!"},
            {"NEGBINOM.DIST(0,3,1,FALSE)", "n:1"},
            {"HYPGEOM.DIST(1,4,8,20,TRUE)", "~0.4654"},      {"HYPGEOM.DIST(1,4,8,20,FALSE)", "~0.3633"},  {"HYPGEOMDIST(1,4,8,20)", "~0.3633"},          {"HYPGEOM.DIST(5,4,8,20,TRUE)", "e:#NUM!"},
            {"HYPGEOM.DIST(1,4,8,0,TRUE)", "e:#NUM!"},       {"HYPGEOM.DIST(0,4,19,20,FALSE)", "e:#NUM!"},
            {"POISSON.DIST(2,5,TRUE)", "~0.124652"},         {"POISSON.DIST(2,5,FALSE)", "~0.084224"},     {"POISSON(2,5,FALSE)", "~0.084224"},          {"POISSON.DIST(-1,5,TRUE)", "e:#NUM!"},
            {"POISSON.DIST(0,0,FALSE)", "n:1"},
        };
        CHECK_ALL(cases, Grid(), Settings())
    }

    void fittedLinesAndCurves()
    {
        Grid g = sample();
        g.column("F1", {1, 2, 3, 4, 5});
        g.column("G1", {2, 1, 4, 3, 6});
        g.column("H1", {9, 8, 19, 18, 29});
        Settings array;
        array.array = true;
        const E cases[] = {
            {"LINEST(B1:B4,A1:A4)", "a:1x2[n:2.3,n:-0.5]"},
            {"ROUND(INDEX(LINEST(B1:B4,A1:A4,TRUE,TRUE),2,1),6)", "n:0.173205"},
            {"ROUND(INDEX(LINEST(B1:B4,A1:A4,TRUE,TRUE),2,2),6)", "n:0.474342"},
            {"ROUND(INDEX(LINEST(B1:B4,A1:A4,TRUE,TRUE),3,1),8)", "n:0.98878505"},
            {"ROUND(INDEX(LINEST(B1:B4,A1:A4,TRUE,TRUE),3,2),6)", "n:0.387298"},
            {"ROUND(INDEX(LINEST(B1:B4,A1:A4,TRUE,TRUE),4,1),4)", "n:176.3333"},
            {"INDEX(LINEST(B1:B4,A1:A4,TRUE,TRUE),4,2)", "n:2"},
            {"ROUND(INDEX(LINEST(B1:B4,A1:A4,TRUE,TRUE),5,1),6)", "n:26.45"},
            {"ROUND(INDEX(LINEST(B1:B4,A1:A4,TRUE,TRUE),5,2),6)", "n:0.3"},
            {"INDEX(LINEST(B1:B4,A1:A4,TRUE,TRUE),3,3)", "e:#REF!"},
            {"INDEX(LINEST(H1:H5,F1:G5,TRUE,TRUE),3,3)", "e:#N/A"},
            {"ROUND(INDEX(LINEST(B1:B4,A1:A4,FALSE),1,1),6)", "n:2.133333"},
            {"INDEX(LINEST(B1:B4,A1:A4,FALSE),1,2)", "n:0"},
            {"LINEST(H1:H5,F1:G5)", "a:1x3[n:3,n:2,n:1]"},
            {"LINEST(B1:B4,A1:A3)", "e:#REF!"},
            {"LINEST(C1:C5,A1:A4)", "e:#VALUE!"},
            {"LINEST(B1:B4)", "a:1x2[n:2.3,n:-0.5]"},
            {"SUM(TREND(B1:B4,A1:A4,5))", "~11"},
            {"TREND(B1:B4,A1:A4)", "a:4x1[n:1.8;n:4.1;n:6.4;n:8.7]"},
            {"TREND(B1:B4,A1:A4,{5;6})", "a:2x1[n:11;n:13.3]"},
            {"SUM(TREND(H1:H5,F1:G5,{6,7}))", "~34"},
            {"LOGEST(E1:E4,A1:A4)", "a:1x2[n:2,n:0.5]"},
            {"SUM(GROWTH(E1:E4,A1:A4,5))", "~16"},
            {"GROWTH(E1:E4,A1:A4)", "a:4x1[n:1;n:2;n:4;n:8]"},
            {"LOGEST(B1:B4,A1:A4,FALSE)", "a:1x2[n:1.799779,n:1]"},
            {"GROWTH({1,-2},{1,2})", "e:#NUM!"},
        };
        for (const auto &c : cases) {
            QString got = ev(withPrefixes(QString::fromUtf8(c.formula)), g, array);
            const QString want = QString::fromUtf8(c.value);
            if (want.startsWith(QLatin1Char('~'))) {
                const QString problem = check(c, g, array);
                QVERIFY2(problem.isEmpty(), qPrintable(problem));
                continue;
            }
            // (arrays are compared to 9 digits)
            auto rounded = [](QString s) {
                static const QRegularExpression number(QStringLiteral("-?\\d+(\\.\\d+)?(e[+-]?\\d+)?"));
                QRegularExpressionMatchIterator it = number.globalMatch(s);
                QString out;
                int last = 0;
                while (it.hasNext()) {
                    const QRegularExpressionMatch m = it.next();
                    double d = m.captured().toDouble();
                    if (std::fabs(d) < 5e-7) d = 0;
                    out += s.mid(last, m.capturedStart() - last) + QString::number(d, 'f', 6);
                    last = m.capturedEnd();
                }
                return out + s.mid(last);
            };
            QVERIFY2(rounded(got) == rounded(want) || got == want, qPrintable(QString::fromUtf8(c.formula) + QStringLiteral(" gave [") + got + QStringLiteral("], expected [") + want + QLatin1Char(']')));
        }
    }

    void couponDatesOfABond()
    {
        const E cases[] = {
            {"COUPDAYBS(DATE(2011,1,25),DATE(2011,11,15),2,1)", "n:71"},   {"COUPDAYS(DATE(2011,1,25),DATE(2011,11,15),2,1)", "n:181"},
            {"COUPDAYSNC(DATE(2011,1,25),DATE(2011,11,15),2,1)", "n:110"}, {"COUPNCD(DATE(2011,1,25),DATE(2011,11,15),2,1)", "n:40678"},
            {"COUPPCD(DATE(2011,1,25),DATE(2011,11,15),2,1)", "n:40497"},  {"COUPNUM(DATE(2007,1,25),DATE(2008,11,15),2,1)", "n:4"},
            {"COUPDAYBS(DATE(2011,1,25),DATE(2011,11,15),2,0)", "n:70"},   {"COUPDAYS(DATE(2011,1,25),DATE(2011,11,15),4,3)", "n:91.25"},
            {"COUPDAYS(DATE(2011,1,25),DATE(2011,11,15),2,0)", "n:180"},   {"COUPDAYSNC(DATE(2011,1,25),DATE(2011,11,15),2,0)", "n:110"},
            {"COUPNUM(DATE(2011,1,25),DATE(2011,11,15),3,1)", "e:#NUM!"},  {"COUPNUM(DATE(2011,11,15),DATE(2011,1,25),2,1)", "e:#NUM!"},
            {"COUPNUM(DATE(2011,1,25),DATE(2011,11,15),2,5)", "e:#NUM!"},  {"COUPNUM(\"x\",DATE(2011,11,15),2,1)", "e:#VALUE!"},
            {"COUPNCD(DATE(2011,5,15),DATE(2011,11,15),2,1)", "n:40862"},  {"COUPPCD(DATE(2011,5,15),DATE(2011,11,15),2,1)", "n:40678"},
            {"COUPPCD(DATE(2011,2,10),DATE(2011,2,28),2,1)", "n:40421"},   {"COUPNCD(DATE(2011,2,10),DATE(2011,2,28),2,1)", "n:40602"},
        };
        CHECK_ALL(cases, Grid(), Settings())
    }

    void pricesYieldsAndDurationsOfBonds()
    {
        const E cases[] = {
            {"PRICE(DATE(2008,2,15),DATE(2017,11,15),0.0575,0.065,100,2,0)", "~94.63436162"},
            {"YIELD(DATE(2008,2,15),DATE(2016,11,15),0.0575,95.04287,100,2,0)", "~0.065"},
            {"DURATION(DATE(2008,1,1),DATE(2016,1,1),0.08,0.09,2,1)", "~5.993775"},
            {"MDURATION(DATE(2008,1,1),DATE(2016,1,1),0.08,0.09,2,1)", "~5.73567"},
            {"PRICE(DATE(2008,2,15),DATE(2008,11,15),0.05,0.06,100,2,0)", "~99.267936"},
            {"PRICE(DATE(2008,2,15),DATE(2017,11,15),-0.01,0.065,100,2,0)", "e:#NUM!"},
            {"PRICE(DATE(2008,2,15),DATE(2017,11,15),0.05,0.065,0,2,0)", "e:#NUM!"},
            {"PRICE(DATE(2008,2,15),DATE(2017,11,15),0.05,0.065,100,3,0)", "e:#NUM!"},
            {"YIELD(DATE(2008,2,15),DATE(2008,11,15),0.05,99.267935828,100,2,0)", "~0.060000"},
            {"DURATION(DATE(2008,7,1),DATE(2016,1,1),-1,0.09,2,1)", "e:#NUM!"},
            {"PRICEDISC(DATE(2008,2,16),DATE(2008,3,1),0.0525,100,2)", "~99.79583"},
            {"PRICEMAT(DATE(2008,2,15),DATE(2008,4,13),DATE(2007,11,11),0.061,0.061,0)", "~99.98449888"},
            {"YIELDDISC(DATE(2008,2,16),DATE(2008,3,1),99.795,100,2)", "~0.052823"},
            {"YIELDMAT(DATE(2008,3,15),DATE(2008,11,3),DATE(2007,11,8),0.0625,100.0123,0)", "~0.060954"},
            {"DISC(DATE(2007,1,25),DATE(2007,6,15),97.975,100,1)", "~0.05242"},
            {"INTRATE(DATE(2008,2,15),DATE(2008,5,15),1000000,1014420,2)", "~0.05768"},
            {"RECEIVED(DATE(2008,2,15),DATE(2008,5,15),1000000,0.0575,2)", "~1014584.654"},
            {"DISC(DATE(2007,6,15),DATE(2007,1,25),97.975,100,1)", "e:#NUM!"},
            {"RECEIVED(DATE(2008,2,15),DATE(2008,5,15),0,0.0575,2)", "e:#NUM!"},
            {"TBILLEQ(DATE(2008,3,31),DATE(2008,6,1),0.0914)", "~0.094151"},
            {"TBILLPRICE(DATE(2008,3,31),DATE(2008,6,1),0.09)", "~98.45"},
            {"TBILLYIELD(DATE(2008,3,31),DATE(2008,6,1),98.45)", "~0.0914"},
            {"TBILLPRICE(DATE(2008,3,31),DATE(2009,6,1),0.09)", "e:#NUM!"},
            {"TBILLEQ(DATE(2008,3,31),DATE(2008,12,1),0.09)", "~0.096028"},
            {"DOLLARDE(1.02,16)", "~1.125"},   {"DOLLARFR(1.125,16)", "~1.02"},     {"DOLLARDE(1.1,10)", "~1.1"},   {"DOLLARDE(1.1,0)", "e:#DIV/0!"},
            {"DOLLARDE(1.1,-1)", "e:#NUM!"},   {"DOLLARFR(1.1,0)", "e:#DIV/0!"},    {"DOLLARDE(-1.02,16)", "~-1.125"},
        };
        CHECK_ALL(cases, Grid(), Settings())
    }

    void interestAccruedAndOddPeriods()
    {
        const E cases[] = {
            {"ACCRINT(DATE(2008,3,1),DATE(2008,8,31),DATE(2008,5,1),0.1,1000,2,0)", "~16.66667"},
            {"ACCRINTM(DATE(2008,4,1),DATE(2008,6,15),0.1,1000,3)", "~20.54795"},
            {"ACCRINT(DATE(2008,3,1),DATE(2008,8,31),DATE(2008,5,1),0.1,1000,2,1)", "~16.576087"},
            {"ACCRINT(DATE(2008,3,1),DATE(2008,8,31),DATE(2008,5,1),-0.1,1000,2,0)", "e:#NUM!"},
            {"ACCRINTM(DATE(2008,6,15),DATE(2008,4,1),0.1,1000,3)", "e:#NUM!"},
            {"ODDFPRICE(DATE(2008,11,11),DATE(2021,3,1),DATE(2008,10,15),DATE(2009,3,1),0.0785,0.0625,100,2,1)", "~113.597717"},
            {"ODDFYIELD(DATE(2008,11,11),DATE(2021,3,1),DATE(2008,10,15),DATE(2009,3,1),0.0575,84.5,100,2,0)", "~0.0772"},
            {"ODDLPRICE(DATE(2008,2,7),DATE(2008,6,15),DATE(2007,10,15),0.0375,0.0405,100,2,0)", "~99.87829"},
            {"ODDLYIELD(DATE(2008,4,20),DATE(2008,6,15),DATE(2007,12,24),0.0375,99.875,100,2,0)", "~0.045192"},
            {"ODDFPRICE(DATE(2008,11,11),DATE(2021,3,1),DATE(2009,10,15),DATE(2009,3,1),0.0785,0.0625,100,2,1)", "e:#NUM!"},
            {"ODDLPRICE(DATE(2008,2,7),DATE(2008,6,15),DATE(2008,7,15),0.0375,0.0405,100,2,0)", "e:#NUM!"},
        };
        CHECK_ALL(cases, Grid(), Settings())
    }

    void loansDepreciationAndRates()
    {
        Grid g;
        g.column("A1", {-120000, 39000, 30000, 21000, 37000, 46000});
        g.column("B1", {0.09, 0.11, 0.1});
        const E cases[] = {
            {"CUMIPMT(0.09/12,30*12,125000,13,24,0)", "~-11135.23213"}, {"CUMPRINC(0.09/12,30*12,125000,13,24,0)", "~-934.1071"},
            {"CUMIPMT(0.09/12,30*12,125000,1,1,1)", "n:0"},             {"CUMIPMT(0,30,125000,1,2,0)", "e:#NUM!"},
            {"CUMIPMT(0.01,10,100,5,3,0)", "e:#NUM!"},                  {"CUMIPMT(0.01,10,100,1,11,0)", "e:#NUM!"},
            {"CUMIPMT(0.01,10,100,1,2,2)", "e:#NUM!"},                  {"CUMIPMT(0.01,10,0,1,2,0)", "e:#NUM!"},
            {"ISPMT(0.1/12,1,3*12,8000000)", "~-64814.81"},             {"ISPMT(0.1,1,0,100)", "e:#DIV/0!"},
            {"MIRR(A1:A6,0.1,0.12)", "~0.126094"},                      {"MIRR(A1:A2,0.1,0.12)", "~-0.675"},
            {"MIRR(B1:B3,0.1,0.12)", "e:#DIV/0!"},                      {"FVSCHEDULE(1,B1:B3)", "~1.33089"},
            {"FVSCHEDULE(1,{0.09,0.11,0.1})", "~1.33089"},              {"FVSCHEDULE(1,{0.09,\"a\"})", "e:#VALUE!"},
            {"DB(1000000,100000,6,1,7)", "~186083.33"},                 {"DB(1000000,100000,6,2,7)", "~259639.42"},
            {"DB(1000000,100000,6,3,7)", "~176814.44"},                 {"DB(1000000,100000,6,7,7)", "~15845.10"},
            {"DB(1000000,100000,6,8,7)", "e:#NUM!"},                    {"DB(1000000,100000,6,1)", "~319000"},
            {"DB(0,100000,6,1)", "n:0"},                                {"DB(1000000,100000,6,1,13)", "e:#NUM!"},
            {"AMORLINC(2400,DATE(2008,8,19),DATE(2008,12,31),300,1,0.15,1)", "~360"},
            {"AMORDEGRC(2400,DATE(2008,8,19),DATE(2008,12,31),300,1,0.15,1)", "~776"},
            {"AMORLINC(2400,DATE(2008,8,19),DATE(2008,12,31),300,0,0.15,1)", "~131.803279"},
            {"AMORDEGRC(2400,DATE(2008,8,19),DATE(2008,12,31),300,0,0.15,1)", "~330"},
            {"AMORLINC(2400,DATE(2008,8,19),DATE(2008,12,31),3000,1,0.15,1)", "e:#NUM!"},
            {"RATE(48,-200,8000)", "~0.007701472"},                     {"RATE(4*12,-200,8000)*12", "~0.09241767"},
            {"RATE(10,0,-100,200)", "~0.071773463"},                    {"RATE(0,-200,8000)", "e:#NUM!"},
            {"RATE(10,-10,100)", "~0"},
            {"PDURATION(0.025,2000,2200)", "~3.859866"},                {"PDURATION(0,2000,2200)", "e:#NUM!"},
            {"RRI(96,10000,11000)", "~0.000993"},                       {"RRI(0,10000,11000)", "e:#NUM!"},
        };
        CHECK_ALL(cases, g, Settings())
    }

    void numbersInOtherBases()
    {
        const E cases[] = {
            {"BIN2DEC(1010)", "n:10"},                  {"BIN2DEC(\"1111111111\")", "n:-1"},        {"BIN2DEC(\"1000000000\")", "n:-512"},      {"BIN2DEC(\"12\")", "e:#NUM!"},
            {"BIN2DEC(\"11111111111\")", "e:#NUM!"},    {"BIN2DEC(\"\")", "n:0"},                   {"BIN2HEX(\"11111011\",4)", "s:00FB"},       {"BIN2HEX(\"1111111111\")", "s:FFFFFFFFFF"},
            {"BIN2OCT(\"1001\",3)", "s:011"},           {"BIN2OCT(\"1111111111\")", "s:7777777777"}, {"DEC2BIN(9,4)", "s:1001"},                {"DEC2BIN(-100)", "s:1110011100"},
            {"DEC2BIN(512)", "e:#NUM!"},                {"DEC2BIN(-513)", "e:#NUM!"},               {"DEC2BIN(9,2)", "e:#NUM!"},                 {"DEC2BIN(9,0)", "e:#NUM!"},
            {"DEC2BIN(9,11)", "e:#NUM!"},               {"DEC2BIN(9.9)", "s:1001"},                 {"DEC2BIN(-1,2)", "s:1111111111"},           {"DEC2HEX(100,4)", "s:0064"},
            {"DEC2HEX(-54)", "s:FFFFFFFFCA"},           {"DEC2HEX(28)", "s:1C"},                    {"DEC2HEX(549755813888)", "e:#NUM!"},        {"DEC2OCT(58,3)", "s:072"},
            {"DEC2OCT(-100)", "s:7777777634"},          {"HEX2BIN(\"F\",8)", "s:00001111"},         {"HEX2BIN(\"FFFFFFFE00\")", "s:1000000000"}, {"HEX2BIN(\"200\")", "e:#NUM!"},
            {"HEX2DEC(\"A5\")", "n:165"},               {"HEX2DEC(\"FFFFFFFF5B\")", "n:-165"},      {"HEX2DEC(\"3DA408B9\")", "n:1034160313"},   {"HEX2DEC(\"G\")", "e:#NUM!"},
            {"HEX2OCT(\"F\",3)", "s:017"},              {"HEX2OCT(\"FFFFFFFF00\")", "s:7777777400"}, {"OCT2BIN(\"3\",3)", "s:011"},              {"OCT2BIN(\"7777777000\")", "s:1000000000"},
            {"OCT2DEC(\"54\")", "n:44"},                {"OCT2DEC(\"7777777533\")", "n:-165"},      {"OCT2DEC(\"8\")", "e:#NUM!"},               {"OCT2HEX(\"100\",4)", "s:0040"},
            {"OCT2HEX(\"7777777533\")", "s:FFFFFFFF5B"}, {"HEX2DEC(255)", "n:597"},                 {"BIN2DEC(TRUE)", "e:#VALUE!"},              {"BIN2DEC(-1)", "e:#NUM!"},
            {"BITAND(1,5)", "n:1"},                     {"BITAND(13,25)", "n:9"},                   {"BITOR(23,10)", "n:31"},                    {"BITXOR(5,3)", "n:6"},
            {"BITLSHIFT(4,2)", "n:16"},                 {"BITRSHIFT(13,2)", "n:3"},                 {"BITLSHIFT(4,-1)", "n:2"},                  {"BITRSHIFT(4,-2)", "n:16"},
            {"BITAND(-1,5)", "e:#NUM!"},                {"BITAND(1.5,5)", "e:#NUM!"},               {"BITAND(281474976710656,1)", "e:#NUM!"},    {"BITLSHIFT(1,54)", "e:#NUM!"},
            {"BITLSHIFT(281474976710655,1)", "e:#NUM!"},
            {"DELTA(5,4)", "n:0"},                      {"DELTA(5,5)", "n:1"},                      {"DELTA(0.5)", "n:0"},                       {"DELTA(\"a\")", "e:#VALUE!"},
            {"GESTEP(5,4)", "n:1"},                     {"GESTEP(-1)", "n:0"},                      {"GESTEP(0)", "n:1"},
        };
        CHECK_ALL(cases, Grid(), Settings())
    }

    void errorFunctionsAndBesselFunctions()
    {
        const E cases[] = {
            {"ERF(0.745)", "~0.707929"},      {"ERF(0,1)", "~0.842701"},       {"ERF(1,0)", "~-0.842701"},     {"ERF.PRECISE(1)", "~0.842701"},   {"ERF(-1)", "~-0.842701"},
            {"ERFC(1)", "~0.157299"},         {"ERFC.PRECISE(1)", "~0.157299"}, {"ERFC(0)", "n:1"},             {"ERF(\"a\")", "e:#VALUE!"},
            {"BESSELJ(1.9,2)", "~0.329926"}, {"BESSELJ(0,0)", "n:1"},       {"BESSELJ(1,0)", "~0.765197687"}, {"BESSELJ(10,3)", "~0.058379379"}, {"BESSELJ(1,-1)", "e:#NUM!"},
            {"BESSELI(1.5,1)", "~0.981666428"}, {"BESSELI(0,0)", "n:1"},       {"BESSELI(1,0)", "~1.266065878"}, {"BESSELI(-1,1)", "~-0.565159104"},
            {"BESSELK(1.5,1)", "~0.2773878"}, {"BESSELK(1,0)", "~0.421024438"}, {"BESSELK(0,1)", "e:#NUM!"},   {"BESSELK(2,3)", "~0.647385"},
            {"BESSELY(2.5,1)", "~0.145918138"}, {"BESSELY(1,0)", "~0.088256964"}, {"BESSELY(0,1)", "e:#NUM!"},   {"BESSELY(5,2)", "~0.36766288"},
        };
        CHECK_ALL(cases, Grid(), Settings())
    }

    void complexNumbers()
    {
        const E cases[] = {
            {"COMPLEX(3,4)", "s:3+4i"},                      {"COMPLEX(3,4,\"j\")", "s:3+4j"},              {"COMPLEX(0,1)", "s:i"},                       {"COMPLEX(3,-1)", "s:3-i"},
            {"COMPLEX(0,0)", "s:0"},                         {"COMPLEX(5,0)", "s:5"},                       {"COMPLEX(0,-2.5)", "s:-2.5i"},                {"COMPLEX(1,1,\"k\")", "e:#VALUE!"},
            {"IMREAL(\"6-9i\")", "n:6"},                     {"IMAGINARY(\"3+4i\")", "n:4"},                {"IMAGINARY(4)", "n:0"},                       {"IMREAL(\"i\")", "n:0"},
            {"IMAGINARY(\"-j\")", "n:-1"},                   {"IMREAL(\"abc\")", "e:#NUM!"},                {"IMREAL(\"3+4\")", "e:#NUM!"},                {"IMABS(\"5+12i\")", "n:13"},
            {"IMARGUMENT(\"3+4i\")", "~0.927295"},           {"IMARGUMENT(0)", "e:#DIV/0!"},                {"IMCONJUGATE(\"3+4i\")", "s:3-4i"},           {"IMSUM(\"3+4i\",\"5-3i\")", "s:8+i"},
            {"IMSUM(\"3+4i\",\"5-3j\")", "e:#VALUE!"},       {"IMSUB(\"13+4i\",\"5+3i\")", "s:8+i"},        {"IMPRODUCT(\"3+4i\",\"5-3i\")", "s:27+11i"},  {"IMDIV(\"-238+240i\",\"10+24i\")", "s:5+12i"},
            {"IMDIV(\"1\",0)", "e:#NUM!"},                   {"IMEXP(\"1+i\")", "s:1.46869393991589+2.28735528717884i"}, {"IMSQRT(\"1+i\")", "s:1.09868411346781+0.455089860562227i"},
            {"IMPOWER(\"2+3i\",3)", "s:x"},                   {"IMLN(\"3+4i\")", "s:1.6094379124341+0.927295218001612i"}, {"IMLN(0)", "e:#NUM!"},
            {"IMLOG10(\"3+4i\")", "s:0.698970004336019+0.402719196273373i"}, {"IMLOG2(\"3+4i\")", "s:2.32192809488736+1.33780421245098i"},
            {"IMSIN(\"3+4i\")", "s:3.85373803791938-27.0168132580039i"}, {"IMCOS(\"1+i\")", "s:0.833730025131149-0.988897705762865i"},
            {"IMSUM(A1:A2)", "s:0"},
        };
        for (const auto &c : cases) {
            // (IMPOWER is compared by its two parts: the rounding noise is not the same on every machine)
            if (QByteArray(c.formula).startsWith("IMPOWER")) {
                const E parts[] = {{"IMREAL(IMPOWER(\"2+3i\",3))", "~-46.000000"}, {"IMAGINARY(IMPOWER(\"2+3i\",3))", "~9.000000"}};
                for (const auto &part : parts) {
                    const QString problem = check(part, Grid(), Settings());
                    QVERIFY2(problem.isEmpty(), qPrintable(problem));
                }
                continue;
            }
            const QString problem = check(c, Grid(), Settings());
            QVERIFY2(problem.isEmpty(), qPrintable(problem));
        }
    }

    void unitConversions()
    {
        const E cases[] = {
            {"CONVERT(1,\"lbm\",\"kg\")", "~0.45359237"}, {"CONVERT(68,\"F\",\"C\")", "~20"},      {"CONVERT(1,\"mi\",\"m\")", "~1609.344"},     {"CONVERT(2.5,\"ft\",\"sec\")", "e:#N/A"},
            {"CONVERT(1,\"gal\",\"l\")", "~3.785411784"}, {"CONVERT(1,\"in\",\"ft\")", "~0.0833333333"}, {"CONVERT(100,\"C\",\"K\")", "~373.15"}, {"CONVERT(0,\"K\",\"F\")", "~-459.67"},
            {"CONVERT(1,\"km\",\"m\")", "~1000"},         {"CONVERT(1,\"m\",\"cm\")", "~100"},      {"CONVERT(1,\"hr\",\"mn\")", "~60"},          {"CONVERT(1,\"day\",\"sec\")", "~86400"},
            {"CONVERT(1,\"kg\",\"g\")", "~1000"},         {"CONVERT(1,\"MB\",\"byte\")", "e:#N/A"}, {"CONVERT(1,\"kbyte\",\"byte\")", "~1000"}, {"CONVERT(1,\"kibyte\",\"byte\")", "~1024"},
            {"CONVERT(1,\"kikbyte\",\"byte\")", "e:#N/A"}, {"CONVERT(1,\"Kibyte\",\"byte\")", "e:#N/A"}, {"CONVERT(1,\"kg\",\"lbm\")", "~2.20462262"}, {"CONVERT(1,\"HP\",\"W\")", "~745.6998716"},
            {"CONVERT(1,\"cal\",\"J\")", "~4.1868"},       {"CONVERT(1,\"atm\",\"Pa\")", "~101325"}, {"CONVERT(1,\"m2\",\"ft2\")", "~10.7639104"}, {"CONVERT(1,\"km2\",\"m2\")", "~1000000"},
            {"CONVERT(1,\"l\",\"m3\")", "~0.001"},        {"CONVERT(1,\"nm\",\"m\")", "~0.000000001"}, {"CONVERT(1,\"xx\",\"m\")", "e:#N/A"}, {"CONVERT(1,\"m/s\",\"mph\")", "~2.236936292"},
        };
        CHECK_ALL(cases, Grid(), Settings())
    }

    void textAndDateFunctionsOfTheNewerKind()
    {
        const E cases[] = {
            {"NUMBERVALUE(\"2.500,27\",\",\",\".\")", "~2500.27"}, {"NUMBERVALUE(\"3.5%\")", "~0.035"},     {"NUMBERVALUE(\" 3 000 \",\".\",\" \")", "n:3000"}, {"NUMBERVALUE(\"\")", "n:0"},
            {"NUMBERVALUE(\"1,2,3\")", "n:123"},               {"NUMBERVALUE(\"1.2.3\")", "e:#VALUE!"},   {"NUMBERVALUE(\"abc\")", "e:#VALUE!"},          {"NUMBERVALUE(\"1.5\",\".\",\".\")", "e:#VALUE!"},
            {"NUMBERVALUE(\"5%%\")", "~0.0005"},               {"NUMBERVALUE(\"1.5,2\")", "e:#VALUE!"},
            {"UNICHAR(65)", "s:A"},                            {"UNICHAR(0)", "e:#VALUE!"},               {"UNICHAR(55296)", "e:#N/A"},                    {"UNICHAR(1114112)", "e:#VALUE!"},
            {"UNICODE(\"A\")", "n:65"},                        {"UNICODE(\"\")", "e:#VALUE!"},            {"UNICODE(UNICHAR(128512))", "n:128512"},        {"LEN(UNICHAR(128512))", "n:2"},
            {"ENCODEURL(\"http://x.com/a b\")", "s:http%3A%2F%2Fx.com%2Fa%20b"}, {"ENCODEURL(\"\")", "s:"}, {"ENCODEURL(\"a-b_c.d~e\")", "s:a-b_c.d~e"},
            {"ASC(DBCS(\"AB1\"))", "s:AB1"},                   {"LEN(DBCS(\"AB1\"))", "n:3"},             {"CODE(\"A\")", "n:65"},
            {"LENB(\"abc\")", "n:3"},                          {"LEFTB(\"abcd\",2)", "s:ab"},             {"MIDB(\"abcd\",2,2)", "s:bc"},                  {"FINDB(\"c\",\"abcd\")", "n:3"},
            {"BAHTTEXT(1234)", "s:\xE0\xB8\xAB\xE0\xB8\x99\xE0\xB8\xB6\xE0\xB9\x88\xE0\xB8\x87\xE0\xB8\x9E\xE0\xB8\xB1\xE0\xB8\x99\xE0\xB8\xAA\xE0\xB8\xAD\xE0\xB8\x87\xE0\xB8\xA3\xE0\xB9\x89\xE0\xB8\xAD\xE0\xB8\xA2\xE0\xB8\xAA\xE0\xB8\xB2\xE0\xB8\xA1\xE0\xB8\xAA\xE0\xB8\xB4\xE0\xB8\x9A\xE0\xB8\xAA\xE0\xB8\xB5\xE0\xB9\x88\xE0\xB8\x9A\xE0\xB8\xB2\xE0\xB8\x97\xE0\xB8\x96\xE0\xB9\x89\xE0\xB8\xA7\xE0\xB8\x99"},
            {"WORKDAY.INTL(DATE(2012,1,1),30,0)", "e:#NUM!"},  {"WORKDAY.INTL(DATE(2012,1,1),30)", "n:40949"}, {"WORKDAY.INTL(DATE(2012,1,1),90,11)", "n:41013"}, {"NETWORKDAYS.INTL(DATE(2006,1,1),DATE(2006,1,31))", "n:22"},
            {"NETWORKDAYS.INTL(DATE(2006,1,1),DATE(2006,2,1),7)", "n:24"}, {"NETWORKDAYS.INTL(DATE(2006,1,1),DATE(2006,1,31),\"0000011\")", "n:22"}, {"NETWORKDAYS.INTL(DATE(2006,1,1),DATE(2006,1,31),\"1111111\")", "e:#VALUE!"},
            {"NETWORKDAYS.INTL(DATE(2006,1,1),DATE(2006,1,31),\"00000\")", "e:#VALUE!"}, {"NETWORKDAYS.INTL(DATE(2006,1,1),DATE(2006,1,31),18)", "e:#NUM!"},
            {"NETWORKDAYS.INTL(DATE(2006,1,31),DATE(2006,1,1))", "n:-22"}, {"NETWORKDAYS.INTL(DATE(2006,1,1),DATE(2006,1,31),1,DATE(2006,1,2))", "n:21"},
            {"WORKDAY.INTL(DATE(2012,1,1),-1,\"0000011\")", "n:40907"},
        };
        CHECK_ALL(cases, Grid(), Settings())
    }

    void lookupAndArrayFunctionsOfTheNewerKind()
    {
        Grid g;
        g.column("A1", {QString("apple"), QString("banana"), QString("cherry"), QString("Apple")});
        g.column("B1", {10, 20, 30, 40});
        g.column("C1", {QString("x"), QString("y"), QString("x"), QString("z")});
        g.column("E1", {5, 1, 4, 1, 3});
        g.row("G1", {3, 1, 2});
        g.row("G2", {QString("c"), QString("a"), QString("b")});
        Settings array;
        array.array = true;
        const E cases[] = {
            {"XMATCH(\"banana\",A1:A4)", "n:2"},               {"XMATCH(\"APPLE\",A1:A4)", "n:1"},                  {"XMATCH(\"APPLE\",A1:A4,0,-1)", "n:4"},          {"XMATCH(\"zzz\",A1:A4)", "e:#N/A"},
            {"XMATCH(25,B1:B4,-1)", "n:2"},                    {"XMATCH(25,B1:B4,1)", "n:3"},                       {"XMATCH(20,B1:B4,1)", "n:2"},                    {"XMATCH(5,B1:B4,-1)", "e:#N/A"},
            {"XMATCH(\"b*\",A1:A4,2)", "n:2"},                 {"XMATCH(\"?herry\",A1:A4,2)", "n:3"},               {"XMATCH(30,B1:B4,0,2)", "n:3"},                  {"XMATCH(2,G1:I1)", "n:3"},
            {"XMATCH(30,B1:B4,0,5)", "e:#VALUE!"},             {"XMATCH(1,A1:B4)", "e:#VALUE!"},                    {"XMATCH(#N/A,B1:B4)", "e:#N/A"},                 {"XMATCH(\"20\",B1:B4)", "e:#N/A"},
            {"XLOOKUP(\"cherry\",A1:A4,B1:B4)", "n:30"},       {"XLOOKUP(\"zzz\",A1:A4,B1:B4)", "e:#N/A"},          {"XLOOKUP(\"zzz\",A1:A4,B1:B4,\"none\")", "s:none"}, {"XLOOKUP(\"apple\",A1:A4,B1:B4,,0,-1)", "n:40"},
            {"XLOOKUP(25,B1:B4,A1:A4,,-1)", "s:banana"},       {"XLOOKUP(25,B1:B4,A1:A4,,1)", "s:cherry"},          {"XLOOKUP(\"banana\",A1:A4,B1:C4)", "a:1x2[n:20,s:y]"}, {"XLOOKUP(1,B1:B3,B1:B4)", "e:#VALUE!"},
            {"XLOOKUP(\"a\",G2:I2,G1:I1)", "n:1"},
            {"FILTER(B1:B4,C1:C4=\"x\")", "a:2x1[n:10;n:30]"}, {"FILTER(A1:B4,B1:B4>25)", "a:2x2[s:cherry,n:30;s:Apple,n:40]"}, {"FILTER(B1:B4,B1:B4>100)", "e:#CALC!"},
            {"FILTER(B1:B4,B1:B4>100,\"nothing\")", "s:nothing"}, {"FILTER(B1:B4,C1:C3=\"x\")", "e:#VALUE!"},         {"FILTER(G1:I2,G1:I1>1)", "a:2x2[n:3,n:2;s:c,s:b]"},
            {"FILTER(B1:B4,\"a\")", "e:#VALUE!"},
            {"SORT(E1:E5)", "a:5x1[n:1;n:1;n:3;n:4;n:5]"},     {"SORT(E1:E5,1,-1)", "a:5x1[n:5;n:4;n:3;n:1;n:1]"}, {"SORT(A1:B4)", "a:4x2[s:apple,n:10;s:Apple,n:40;s:banana,n:20;s:cherry,n:30]"},
            {"SORT(A1:B4,2,-1)", "a:4x2[s:Apple,n:40;s:cherry,n:30;s:banana,n:20;s:apple,n:10]"}, {"SORT(G1:I2,1,1,TRUE)", "a:2x3[n:1,n:2,n:3;s:a,s:b,s:c]"},
            {"SORT(E1:E5,2)", "e:#VALUE!"},                    {"SORT(E1:E5,1,0)", "e:#VALUE!"},
            {"SORTBY(A1:A4,B1:B4,-1)", "a:4x1[s:Apple;s:cherry;s:banana;s:apple]"}, {"SORTBY(B1:B4,C1:C4,1,B1:B4,-1)", "a:4x1[n:30;n:10;n:20;n:40]"},
            {"SORTBY(B1:B4,C1:C3)", "e:#VALUE!"},
            {"UNIQUE(E1:E5)", "a:4x1[n:5;n:1;n:4;n:3]"},      {"UNIQUE(E1:E5,FALSE,TRUE)", "a:3x1[n:5;n:4;n:3]"},   {"UNIQUE(A1:A4)", "a:3x1[s:apple;s:banana;s:cherry]"},
            {"UNIQUE(G1:I2,TRUE)", "a:2x3[n:3,n:1,n:2;s:c,s:a,s:b]"}, {"UNIQUE(C1:C4,FALSE,TRUE)", "a:2x1[s:y;s:z]"}, {"UNIQUE(E1:E2,FALSE,TRUE)", "a:2x1[n:5;n:1]"},
            {"AREAS(A1:B2)", "n:1"},                           {"AREAS((A1:B2,C1:C3))", "n:2"},                     {"AREAS(5)", "e:#VALUE!"},
            {"SHEET()", "n:1"},                                {"SHEET(B1)", "n:1"},                                {"SHEET(\"Sheet1\")", "n:1"},                     {"SHEET(\"nope\")", "e:#N/A"},
            {"SHEETS()", "n:1"},                               {"SHEETS(B1)", "n:1"},
            {"ISFORMULA(E1)", "b:1"},                          {"ISFORMULA(B1)", "b:0"},                            {"ISFORMULA(5)", "e:#VALUE!"},                    {"ISFORMULA(Z9)", "b:0"},
            {"FORMULATEXT(E1)", "s:=SUM(B1:B2)"},              {"FORMULATEXT(B1)", "e:#N/A"},                       {"FORMULATEXT(\"x\")", "e:#VALUE!"},
        };
        g.formula("E1", "SUM(B1:B2)");
        CHECK_ALL(cases, g, array)
    }

    // A formula whose numbers are huge or whose counts are unbounded ends at once, with a result or an error (each of these ran for minutes before).
    void budget()
    {
        static const char *const formulas[] = {
            "DB(1,0.5,1E300,1E300)", "AMORDEGRC(1,DATE(2020,1,1),DATE(2020,12,31),0,1E300,0.1)", "CUMIPMT(0.1,1E300,1,1,1E300,0)", "BINOM.DIST.RANGE(1E300,0.5,0,1E300)",
            "BINOM.DIST.RANGE(1E300,0.5,1E300,1E300)", "HYPGEOM.DIST(1E300,1E300,1E300,1E300,TRUE)", "COMBINA(1E15,1E15)", "COMBINA(1E300,1E300)", "BINOM.INV(1E17,0.5,0.3)",
            "BINOM.INV(1E300,0.5,0.3)", "CUMIPMT(0.1,1E15,1,1E15,1E15,0)", "MODE.MULT(SEQUENCE(20000,1,1,0))", "YIELD(DATE(2020,1,1),DATE(9999,12,31),0.05,90,100,4,1)",
            "PRICE(DATE(2020,1,1),DATE(9999,12,31),0.05,0.04,100,4,1)", "DURATION(DATE(2020,1,1),DATE(9999,12,31),0.05,0.04,4,1)",
            "ODDFYIELD(DATE(2020,1,1),DATE(9999,12,31),DATE(1900,1,2),DATE(2021,1,1),0.05,90,100,4,1)", "ODDLYIELD(DATE(9998,6,1),DATE(9999,12,31),DATE(1900,1,2),0.05,90,100,4,1)",
            "ACCRINT(DATE(1900,1,2),DATE(9999,12,31),DATE(9999,12,30),0.05,100,4,1)", "CHISQ.INV.RT(1E-300,1E9)", "GAMMA.INV(0.5,1E9,1E9)", "BETA.INV(0.5,1E8,1E-8)",
            "T.INV(1E-300,1E9)", "F.INV.RT(1E-300,1E9,1E9)", "CHISQ.DIST(1E9,1E9,TRUE)", "UNIQUE(SEQUENCE(100000,3,1,0.001))", "SORT(SEQUENCE(100000,2,1,0.001),{1,2})",
            "BESSELJ(1E300,1)", "BESSELY(1E300,1)", "BESSELK(1E-300,1000)", "FREQUENCY(SEQUENCE(1000000),SEQUENCE(1000))", "LINEST(SEQUENCE(1000),SEQUENCE(1000,10),TRUE,TRUE)",
        };
        Settings array;
        array.array = true;
        const Grid g = sample();
        for (const char *f : formulas) {
            QElapsedTimer timer;
            timer.start();
            const QString got = ev(withPrefixes(QString::fromUtf8(f)), g, array);
            QVERIFY2(!got.startsWith(QLatin1String("!")), qPrintable(QString::fromUtf8(f) + QStringLiteral(" -> ") + got.left(100)));
            QVERIFY2(timer.elapsed() < 8000, qPrintable(QStringLiteral("%1 took %2 ms").arg(QString::fromUtf8(f)).arg(timer.elapsed())));
        }
    }

    void resultsThatAreNotNumbersAreErrorsAndTheOddCasesAreRight()
    {
        Settings array;
        array.array = true;
        Grid g = sample();
        g.column("F1", {1, 2, 3, 4});
        const E cases[] = {
            {"ISERROR(INDEX(SEQUENCE(1,2,1E308,1E308),1,2))", "b:1"},   {"ISERROR(INDEX(RANDARRAY(1,1,-1E308,1E308),1,1))", "b:1"},
            {"ISERROR(INDEX(GROWTH({1,2,3},{1,2,3},1E300),1,1))", "b:1"}, {"BESSELJ(1E6,0)", "e:#NUM!"},       {"BESSELY(1E6,0)", "e:#NUM!"},
            {"AGGREGATE(1E300,6,A1)", "e:#VALUE!"},                      {"AGGREGATE(3,6,C1:C5)", "n:3"},     {"AGGREGATE(3,4,C1:C5)", "n:4"},        {"AGGREGATE(2,6,C1:C5)", "n:1"},
            {"SORT({1,2;1,1;0,5},{1,2},{1,-1})", "a:3x2[n:0,n:5;n:1,n:2;n:1,n:1]"}, {"SORT({1,2;1,1;0,5},{2,1})", "a:3x2[n:1,n:1;n:1,n:2;n:0,n:5]"},
            {"SORT(F1:F4,{1,2})", "e:#VALUE!"},                          {"SORT(F1:F4,{1,2},{1,2,3})", "e:#VALUE!"},
            {"ROUND(INDEX(LINEST({3;5;7;9},{1,2;2,4;3,6;4,8}),1,1),8)", "n:0"}, {"ROUND(INDEX(LINEST({3;5;7;9},{1,2;2,4;3,6;4,8}),1,2),8)", "n:2"},
            {"ROUND(INDEX(LINEST({3;5;7;9},{1,2;2,4;3,6;4,8}),1,3),8)", "n:1"},
            {"CONVERT(1,\"m^2\",\"ft^2\")", "~10.7639104"},                {"CONVERT(1,\"yd^3\",\"ft^3\")", "~27"}, {"CONVERT(1E20,\"ang^2\",\"m^2\")", "~1"}, {"CONVERT(1,\"ft^3\",\"in^3\")", "~1728"},
            {"CONVERT(1,\"in^2\",\"ft^2\")", "~0.00694444444"},           {"CONVERT(1,\"km^2\",\"m^2\")", "~1000000"},
            {"ABS(T.DIST(T.INV(1E-12,5),5,TRUE)/1E-12-1)<1E-9", "b:1"},  {"ABS(CHISQ.DIST.RT(CHISQ.INV.RT(1E-12,3),3)/1E-12-1)<1E-9", "b:1"},
            {"ABS(F.DIST.RT(F.INV.RT(1E-12,4,6),4,6)/1E-12-1)<1E-9", "b:1"}, {"ABS(T.DIST.2T(T.INV.2T(1E-12,7),7)/1E-12-1)<1E-9", "b:1"},
        };
        CHECK_ALL(cases, g, array)
    }
};

JP_TEST_CLASS(SheetFnMoreTests)
#include "tst_sheet_fn_more.moc"
