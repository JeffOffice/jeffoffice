// Tests of the date and time functions, by the serial rules of calc/dates.h: day 1 is January 1, 1900; the 1900 system has a February 29, 1900 (day 60) that
// the calendar never had, so the days from March 1 are one more than the true count; day 0 is "January 0, 1900"; the 1904 system counts from January 1, 1904.
// Each expected value is a date worked out by hand or from the calendar, not from what the code gave.
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

class SheetFnDateTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void datesAreMadeOfYearMonthAndDay()
    {
        const E cases[] = {
            {"DATE(2024,3,15)", "n:45366"},
            {"DATE(2024,14,1)", "n:45689"},        // month 14 is February of the next year
            {"DATE(2024,0,1)", "n:45261"},          // month 0 is December of the year before
            {"DATE(2024,1,0)", "n:45291"},          // day 0 is the day before
            {"DATE(2024,1,-1)", "n:45290"},
            {"DATE(2024,3,32)", "n:45383"},
            {"DATE(99,1,1)", "n:36161"},            // years 0 to 1899 have 1900 added
            {"DATE(0,1,1)", "n:1"},
            {"DATE(1900,1,1)", "n:1"},
            {"DATE(1900,2,28)", "n:59"},
            {"DATE(1900,2,29)", "n:60"},            // the day that never was
            {"DATE(1900,3,1)", "n:61"},
            {"DATE(1900,1,0)", "n:0"},
            {"DATE(1900,1,-1)", "e:#NUM!"},
            {"DATE(2024.9,3.9,15.9)", "n:45366"},   // cut to whole numbers
            {"DATE(\"2024\",\"3\",\"15\")", "n:45366"},
            {"DATE(10000,1,1)", "e:#NUM!"},
            {"DATE(-1,1,1)", "e:#NUM!"},
            {"DATE(9999,12,31)", "n:2958465"},
            {"DATE(9999,12,32)", "e:#NUM!"},
            {"DATE(2000,1,1)", "n:36526"},
            {"DATE(2024,2,30)", "n:45352"},         // February 30 of a leap year is March 1
            {"DATE(\"x\",1,1)", "e:#VALUE!"},
            {"DATE(#N/A,1,1)", "e:#N/A"},
            {"DATE(2024,3,15)-DATE(2024,3,1)", "n:14"},
        };
        CHECK_EVALS(cases, Grid(), Settings());
    }

    void partsOfADate()
    {
        const E cases[] = {
            {"YEAR(45366)", "n:2024"}, {"MONTH(45366)", "n:3"}, {"DAY(45366)", "n:15"},
            {"YEAR(60)", "n:1900"}, {"MONTH(60)", "n:2"}, {"DAY(60)", "n:29"},
            {"YEAR(61)", "n:1900"}, {"MONTH(61)", "n:3"}, {"DAY(61)", "n:1"},
            {"YEAR(59)", "n:1900"}, {"MONTH(59)", "n:2"}, {"DAY(59)", "n:28"},
            {"YEAR(1)", "n:1900"}, {"MONTH(1)", "n:1"}, {"DAY(1)", "n:1"},
            {"YEAR(0)", "n:1900"}, {"MONTH(0)", "n:1"}, {"DAY(0)", "n:0"},
            {"YEAR(-1)", "e:#NUM!"}, {"MONTH(-0.5)", "e:#NUM!"},
            {"YEAR(45366.9)", "n:2024"},
            {"YEAR(\"2024-03-15\")", "n:2024"}, {"MONTH(\"3/15/2024\")", "n:3"}, {"DAY(\"15-Mar-2024\")", "n:15"},
            {"YEAR(\"abc\")", "e:#VALUE!"},
            {"YEAR(2958465)", "n:9999"}, {"MONTH(2958465)", "n:12"}, {"DAY(2958465)", "n:31"},
            {"YEAR(2958466)", "e:#NUM!"},
            {"YEAR(A1)", "n:1900"},                  // an empty cell is day 0
            {"YEAR(TRUE)", "n:1900"},
            {"MONTH(DATE(2024,12,31))", "n:12"},
            {"DAY(DATE(2024,2,29))", "n:29"},
            {"YEAR(#N/A)", "e:#N/A"},
        };
        CHECK_EVALS(cases, Grid(), Settings());
        Settings s1904;
        s1904.system = DateSystem::Date1904;
        const E cases1904[] = {
            {"DATE(1904,1,1)", "n:0"}, {"DATE(2024,3,15)", "n:43904"}, {"DATE(1900,1,1)", "e:#NUM!"},
            {"YEAR(0)", "n:1904"}, {"MONTH(0)", "n:1"}, {"DAY(0)", "n:1"}, {"YEAR(43904)", "n:2024"}, {"DAY(43904)", "n:15"},
            {"YEAR(-1)", "e:#NUM!"}, {"WEEKDAY(0)", "n:6"},
        };
        CHECK_EVALS(cases1904, Grid(), s1904);
    }

    void weekdaysCountFromTheSerialNumber()
    {
        const E cases[] = {
            {"WEEKDAY(45366)", "n:6"},             // a Friday, counted from Sunday as 1
            {"WEEKDAY(45366,1)", "n:6"}, {"WEEKDAY(45366,2)", "n:5"}, {"WEEKDAY(45366,3)", "n:4"},
            {"WEEKDAY(45366,11)", "n:5"}, {"WEEKDAY(45366,12)", "n:4"}, {"WEEKDAY(45366,13)", "n:3"}, {"WEEKDAY(45366,14)", "n:2"},
            {"WEEKDAY(45366,15)", "n:1"}, {"WEEKDAY(45366,16)", "n:7"}, {"WEEKDAY(45366,17)", "n:6"},
            {"WEEKDAY(1)", "n:1"},                 // the 1900 system takes January 1, 1900 for a Sunday
            {"WEEKDAY(2)", "n:2"},
            {"WEEKDAY(7)", "n:7"},
            {"WEEKDAY(8)", "n:1"},
            {"WEEKDAY(60)", "n:4"},
            {"WEEKDAY(61)", "n:5"},
            {"WEEKDAY(0)", "n:7"},
            {"WEEKDAY(45366,4)", "e:#NUM!"},
            {"WEEKDAY(45366,0)", "e:#NUM!"},
            {"WEEKDAY(\"abc\")", "e:#VALUE!"},
            {"WEEKDAY(-1)", "e:#NUM!"},
            {"WEEKDAY(45366.99)", "n:6"},
            {"WEEKDAY(\"2024-03-15\")", "n:6"},
            {"WEEKDAY(DATE(2000,1,1))", "n:7"},     // a Saturday
            {"WEEKDAY(DATE(2024,2,29))", "n:5"},    // a Thursday
            {"WEEKNUM(45366)", "n:11"},
            {"WEEKNUM(45366,2)", "n:11"},
            {"WEEKNUM(DATE(2024,1,1))", "n:1"},
            {"WEEKNUM(DATE(2024,12,31))", "n:53"},
        };
        CHECK_EVALS(cases, Grid(), Settings());
    }

    void timesAreFractionsOfADay()
    {
        const E cases[] = {
            {"HOUR(0.5)", "n:12"}, {"HOUR(0.75)", "n:18"}, {"HOUR(45366.75)", "n:18"}, {"HOUR(0.99)", "n:23"}, {"HOUR(0)", "n:0"},
            {"MINUTE(0.0208333333333)", "n:30"}, {"MINUTE(0.00347222222222)", "n:5"}, {"SECOND(0.00347222222222)", "n:0"},
            {"SECOND(1/86400)", "n:1"}, {"SECOND(0.5)", "n:0"}, {"SECOND(0.5+30/86400)", "n:30"},
            {"HOUR(\"12:30\")", "n:12"}, {"MINUTE(\"12:30:45\")", "n:30"}, {"SECOND(\"12:30:45\")", "n:45"},
            {"HOUR(\"6:00 PM\")", "n:18"},
            {"HOUR(-1)", "e:#NUM!"}, {"HOUR(\"abc\")", "e:#VALUE!"},
            {"TIME(12,30,0)", "n:0.520833333333333"}, {"TIME(12,0,0)", "n:0.5"}, {"TIME(25,0,0)", "n:0.0416666666666667"},
            {"TIME(0,90,0)", "n:0.0625"}, {"TIME(24,0,0)", "n:0"}, {"TIME(0,0,-1)", "e:#NUM!"}, {"TIME(32768,0,0)", "e:#NUM!"},
            {"TIME(6,0,0)*24", "n:6"},
            {"TIMEVALUE(\"12:00\")", "n:0.5"}, {"TIMEVALUE(\"6:00 PM\")", "n:0.75"}, {"TIMEVALUE(\"abc\")", "e:#VALUE!"},
            {"DATEVALUE(\"2024-03-15\")", "n:45366"}, {"DATEVALUE(\"3/15/2024\")", "n:45366"}, {"DATEVALUE(\"15-Mar-2024\")", "n:45366"},
            {"DATEVALUE(\"abc\")", "e:#VALUE!"}, {"DATEVALUE(45366)", "e:#VALUE!"},
        };
        CHECK_EVALS(cases, Grid(), Settings());
    }

    void monthsAndWorkdays()
    {
        const E cases[] = {
            {"EOMONTH(45366,0)", "n:45382"}, {"EOMONTH(45366,1)", "n:45412"}, {"EOMONTH(45366,-3)", "n:45291"}, {"EOMONTH(45366,12)", "n:45747"},
            {"EOMONTH(DATE(2024,1,31),1)", "n:45351"}, {"EOMONTH(\"x\",1)", "e:#VALUE!"},
            {"EDATE(45366,1)", "n:45397"}, {"EDATE(DATE(2024,1,31),1)", "n:45351"}, {"EDATE(45366,-1)", "n:45337"}, {"EDATE(45366,0)", "n:45366"},
            {"EDATE(\"x\",1)", "e:#VALUE!"},
            {"WORKDAY(45366,1)", "n:45369"}, {"WORKDAY(45366,5)", "n:45373"}, {"WORKDAY(45366,0)", "n:45366"}, {"WORKDAY(45369,-1)", "n:45366"},
            {"NETWORKDAYS(45366,45373)", "n:6"}, {"NETWORKDAYS(45373,45366)", "n:-6"}, {"NETWORKDAYS(45367,45368)", "n:0"},
            {"DAYS(45400,45366)", "n:34"},
            {"DAYS360(DATE(2024,1,1),DATE(2024,12,31))", "n:360"}, {"DAYS360(DATE(2024,1,1),DATE(2024,12,31),TRUE)", "n:359"},
            {"DAYS360(DATE(2024,1,31),DATE(2024,3,31))", "n:60"},
            {"YEARFRAC(DATE(2024,1,1),DATE(2024,7,1))", "n:0.5"},
        };
        CHECK_EVALS(cases, Grid(), Settings());
    }

    void nowAndTodayAreTheClockAsTheContextSaysIt()
    {
        Settings s;
        s.now = 45366.5;
        const E cases[] = {
            {"NOW()", "n:45366.5"}, {"TODAY()", "n:45366"}, {"TODAY()-DATE(2024,3,1)", "n:14"}, {"YEAR(TODAY())", "n:2024"}, {"NOW()-TODAY()", "n:0.5"},
            {"HOUR(NOW())", "n:12"}, {"MONTH(NOW())", "n:3"},
        };
        CHECK_EVALS(cases, Grid(), s);
        // with no time given, the clock: a day number after January 1, 2024
        Settings clock;
        clock.now = 0;
        const QString today = ev("TODAY()", Grid(), clock);
        QVERIFY2(today.startsWith("n:") && today.mid(2).toDouble() > 45292, qPrintable(today));
        const QString now = ev("NOW()-TODAY()", Grid(), clock);
        QVERIFY2(now.startsWith("n:") && now.mid(2).toDouble() >= 0 && now.mid(2).toDouble() < 1, qPrintable(now));
    }
};

JP_TEST_CLASS(SheetFnDateTests)
#include "tst_sheet_fn_date.moc"
