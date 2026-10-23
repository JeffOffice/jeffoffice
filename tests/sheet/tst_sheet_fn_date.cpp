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

    // ISO 8601 weeks (type 21, and ISOWEEKNUM): weeks start on Monday and the first has the year's first Thursday; the days before it are the last week of the year before.
    void isoWeeksBelongToTheYearOfTheirThursday()
    {
        const E cases[] = {
            {"WEEKNUM(45366,21)", "n:11"},                        // Friday, March 15, 2024
            {"WEEKNUM(DATE(2024,1,1),21)", "n:1"},                // a Monday
            {"WEEKNUM(DATE(2024,12,29),21)", "n:52"},             // a Sunday
            {"WEEKNUM(DATE(2024,12,30),21)", "n:1"},              // the Monday that begins week 1 of 2025
            {"WEEKNUM(DATE(2021,1,1),21)", "n:53"},               // a Friday, in the last week of 2020
            {"WEEKNUM(DATE(2021,1,3),21)", "n:53"},
            {"WEEKNUM(DATE(2021,1,4),21)", "n:1"},
            {"WEEKNUM(DATE(2020,12,31),21)", "n:53"},
            {"WEEKNUM(DATE(2016,1,3),21)", "n:53"},
            {"WEEKNUM(DATE(2015,12,31),21)", "n:53"},
            {"WEEKNUM(DATE(2018,12,31),21)", "n:1"},
            {"WEEKNUM(DATE(2008,12,29),21)", "n:1"},
            {"WEEKNUM(DATE(2010,1,3),21)", "n:53"},
            {"WEEKNUM(DATE(2025,12,29),21)", "n:1"},
            {"WEEKNUM(DATE(2026,12,31),21)", "n:53"},
            {"WEEKNUM(DATE(2027,1,1),21)", "n:53"},
            {"WEEKNUM(DATE(2027,1,4),21)", "n:1"},
            {"WEEKNUM(DATE(2024,2,29),21)", "n:9"},
            {"WEEKNUM(DATE(9999,12,31),21)", "n:52"},
            // the first days of the 1900 system: day 1 is a Sunday, which closes the week before the one that begins on Monday, day 2
            {"WEEKNUM(1,21)", "n:52"},
            {"WEEKNUM(0,21)", "n:52"},
            {"WEEKNUM(2,21)", "n:1"},
            {"WEEKNUM(8,21)", "n:1"},
            {"WEEKNUM(9,21)", "n:2"},
            {"WEEKNUM(60,21)", "n:9"},
            {"WEEKNUM(61,21)", "n:9"},
        };
        CHECK_EVALS(cases, Grid(), Settings());
        // the 1904 system: day 0 is Friday, January 1, 1904, in the last week of 1903 (which has 53), and Monday, day 3, begins the first of 1904
        Settings system1904;
        system1904.system = DateSystem::Date1904;
        const E cases1904[] = {
            {"WEEKNUM(0,21)", "n:53"},
            {"WEEKNUM(1,21)", "n:53"},
            {"WEEKNUM(2,21)", "n:53"},
            {"WEEKNUM(3,21)", "n:1"},
            {"WEEKNUM(9,21)", "n:1"},
            {"WEEKNUM(10,21)", "n:2"},
            {"WEEKNUM(365,21)", "n:52"},                          // Saturday, December 31, 1904
            {"WEEKNUM(DATE(2024,1,1),21)", "n:1"},
            {"WEEKNUM(DATE(2021,1,1),21)", "n:53"},
            {"WEEKNUM(DATE(1904,1,1),21)", "n:53"},
            {"WEEKNUM(DATE(1905,1,1),21)", "n:52"},               // a Sunday
            {"WEEKNUM(DATE(1905,1,2),21)", "n:1"},
            {"WEEKNUM(0)", "n:1"},                                // (the other types never looked back a year)
            {"WEEKNUM(0,2)", "n:1"},
        };
        CHECK_EVALS(cases1904, Grid(), system1904);
    }

    // ISOWEEKNUM is the week of type 21 of WEEKNUM, in either system.
    void isoWeekNumIsTheIsoWeek()
    {
        const E cases[] = {
            {"_xlfn.ISOWEEKNUM(45366)", "n:11"},
            {"_xlfn.ISOWEEKNUM(DATE(2021,1,1))", "n:53"},
            {"_xlfn.ISOWEEKNUM(DATE(2024,12,30))", "n:1"},
            {"_xlfn.ISOWEEKNUM(\"2024-03-15\")", "n:11"},
            {"_xlfn.ISOWEEKNUM(1)", "n:52"},
            {"_xlfn.ISOWEEKNUM(2)", "n:1"},
            {"_xlfn.ISOWEEKNUM(-1)", "e:#NUM!"},
            {"_xlfn.ISOWEEKNUM(\"x\")", "e:#VALUE!"},
            {"ISOWEEKNUM(45366)", "!unsupported: ISOWEEKNUM"},    // (a newer function is written with its _xlfn. in a file)
        };
        CHECK_EVALS(cases, Grid(), Settings());
        Settings system1904;
        system1904.system = DateSystem::Date1904;
        const E cases1904[] = {
            {"_xlfn.ISOWEEKNUM(0)", "n:53"},
            {"_xlfn.ISOWEEKNUM(3)", "n:1"},
            {"_xlfn.ISOWEEKNUM(DATE(2024,12,30))", "n:1"},
        };
        CHECK_EVALS(cases1904, Grid(), system1904);
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
            {"DATEVALUE(\"Sep-2000\")", "n:36770"}, {"DATEVALUE(\"OCT-2000\")", "n:36800"}, {"DATEVALUE(\"March 2024\")", "n:45352"}, {"DATEVALUE(\"Foo-2000\")", "e:#VALUE!"},
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
            // February 1900 has 28 days to these functions (the calendar's), though day 60 is called February 29 by the others
            {"EOMONTH(DATE(1900,2,10),0)", "n:59"}, {"EOMONTH(31,0)", "n:31"}, {"EOMONTH(31,1)", "n:59"}, {"EDATE(DATE(1900,1,31),1)", "n:59"},
            {"WORKDAY(45366,1)", "n:45369"}, {"WORKDAY(45366,5)", "n:45373"}, {"WORKDAY(45366,0)", "n:45366"}, {"WORKDAY(45369,-1)", "n:45366"},
            {"NETWORKDAYS(45366,45373)", "n:6"}, {"NETWORKDAYS(45373,45366)", "n:-6"}, {"NETWORKDAYS(45367,45368)", "n:0"},
            {"_xlfn.DAYS(45400,45366)", "n:34"},
            {"DAYS360(DATE(2024,1,1),DATE(2024,12,31))", "n:360"}, {"DAYS360(DATE(2024,1,1),DATE(2024,12,31),TRUE)", "n:359"},
            {"DAYS360(DATE(2024,1,31),DATE(2024,3,31))", "n:60"},
            {"YEARFRAC(DATE(2024,1,1),DATE(2024,7,1))", "n:0.5"},
            {"DATEDIF(DATE(2020,1,15),DATE(2024,3,10),\"Y\")", "n:4"}, {"DATEDIF(DATE(2020,1,15),DATE(2024,3,10),\"M\")", "n:49"},
            {"DATEDIF(DATE(2020,1,15),DATE(2024,3,10),\"D\")", "n:1516"}, {"DATEDIF(DATE(2020,1,15),DATE(2024,3,10),\"YM\")", "n:1"},
            {"DATEDIF(DATE(2024,3,10),DATE(2020,1,15),\"Y\")", "e:#NUM!"}, {"DATEDIF(DATE(2020,1,15),DATE(2024,3,10),\"Q\")", "e:#NUM!"},
            {"DATEDIF(DATE(2020,1,15),DATE(2024,1,15),\"y\")", "n:4"}, {"DATEDIF(DATE(2020,1,15),DATE(2024,1,14),\"Y\")", "n:3"},
        };
        CHECK_EVALS(cases, Grid(), Settings());
    }

    // Counting 30 days to a month: the United States' (NASD) rule and the european one, with the ends of months, of February in years of 28 and of 29 days, and months of 30.
    void daysAreCountedThirtyToAMonth()
    {
        const E cases[] = {
            {"DAYS360(DATE(2011,1,1),DATE(2011,4,30))", "n:119"},       // the 30th of a 30-day month is the 30th, not the end of a month to move on from
            {"DAYS360(DATE(2011,1,1),DATE(2011,4,30),TRUE)", "n:119"},
            {"DAYS360(DATE(2023,1,15),DATE(2023,2,28))", "n:43"},       // the 28th of February is moved only when the start is the end of February too
            {"DAYS360(DATE(2023,1,15),DATE(2023,2,28),TRUE)", "n:43"},
            {"DAYS360(DATE(2011,1,31),DATE(2011,3,31))", "n:60"},
            {"DAYS360(DATE(2011,1,31),DATE(2011,3,31),TRUE)", "n:60"},
            {"DAYS360(DATE(2011,1,30),DATE(2011,3,31))", "n:60"},
            {"DAYS360(DATE(2011,1,29),DATE(2011,3,31))", "n:62"},       // the 31st stays when the start is before the 30th
            {"DAYS360(DATE(2011,1,29),DATE(2011,3,31),TRUE)", "n:61"},
            {"DAYS360(DATE(2011,2,28),DATE(2011,3,31))", "n:30"},       // the end of February is the 30th, and then the 31st is
            {"DAYS360(DATE(2011,2,28),DATE(2011,3,31),TRUE)", "n:32"},
            {"DAYS360(DATE(2012,2,29),DATE(2012,3,31))", "n:30"},
            {"DAYS360(DATE(2012,2,29),DATE(2012,3,31),TRUE)", "n:31"},
            {"DAYS360(DATE(2012,2,28),DATE(2012,3,31))", "n:33"},       // (the end of February 2012 is the 29th)
            {"DAYS360(DATE(2012,2,28),DATE(2012,3,31),TRUE)", "n:32"},
            {"DAYS360(DATE(2011,2,28),DATE(2012,2,29))", "n:360"},      // both the ends of February: the end is the 30th for both
            {"DAYS360(DATE(2011,2,28),DATE(2012,2,29),TRUE)", "n:361"},
            {"DAYS360(DATE(2011,2,28),DATE(2011,2,28))", "n:0"},
            {"DAYS360(DATE(2011,1,30),DATE(2011,2,28))", "n:28"},
            {"DAYS360(DATE(2011,1,30),DATE(2011,2,28),TRUE)", "n:28"},
            {"DAYS360(DATE(2011,4,30),DATE(2011,5,31))", "n:30"},
            {"DAYS360(DATE(2011,4,30),DATE(2011,5,31),TRUE)", "n:30"},
            {"DAYS360(DATE(2011,4,30),DATE(2011,6,30))", "n:60"},
            {"DAYS360(DATE(2011,6,15),DATE(2011,6,30))", "n:15"},
            {"DAYS360(DATE(2011,6,15),DATE(2011,6,30),TRUE)", "n:15"},
            {"DAYS360(DATE(2011,4,30),DATE(2011,4,30))", "n:0"},
            {"DAYS360(DATE(2011,4,30),DATE(2011,1,1))", "n:-119"},
            {"DAYS360(DATE(2011,3,31),DATE(2011,2,28))", "n:-32"},      // 31 is the 30th; the end of February stays the 28th
            {"DAYS360(DATE(2024,1,1),DATE(2024,12,31))", "n:360"},
            {"DAYS360(DATE(2024,1,1),DATE(2024,12,31),TRUE)", "n:359"},
            {"DAYS360(DATE(2024,2,29),DATE(2025,2,28))", "n:360"},      // the end of February to the end of February: both are the 30th
            {"DAYS360(DATE(2024,2,29),DATE(2025,2,28),TRUE)", "n:359"},
            {"DAYS360(DATE(2023,2,28),DATE(2024,2,29))", "n:360"},
            {"DAYS360(\"x\",DATE(2011,1,1))", "e:#VALUE!"},
            {"YEARFRAC(DATE(2011,1,1),DATE(2011,4,30))", "n:0.330555555555556"},
            {"YEARFRAC(DATE(2011,1,1),DATE(2011,4,30),4)", "n:0.330555555555556"},
            {"YEARFRAC(DATE(2011,1,29),DATE(2011,3,31),0)", "n:0.172222222222222"},
        };
        CHECK_EVALS(cases, Grid(), Settings());
    }

    // YEARFRAC by actual days over actual years: the length of the year is that of the one the span is in; of a span of a year or less that holds a February 29, 366; of a
    // longer one, the average of the years it touches.
    void aYearFractionOfActualDaysDividesByTheLengthOfTheYear()
    {
        const E cases[] = {
            {"YEARFRAC(DATE(2023,6,1),DATE(2024,3,1),1)", "n:0.748633879781421"},      // 274 days, a year with a February 29 in it: 366
            {"YEARFRAC(DATE(2023,1,1),DATE(2023,7,1),1)", "n:0.495890410958904"},      // 181 of 365
            {"YEARFRAC(DATE(2024,1,1),DATE(2024,7,1),1)", "n:0.497267759562842"},      // 182 of 366
            {"YEARFRAC(DATE(2023,3,1),DATE(2024,3,1),1)", "n:1"},
            {"YEARFRAC(DATE(2024,3,1),DATE(2025,2,28),1)", "n:0.997260273972603"},     // 364 of 365: the February 29 of 2024 is before it
            {"YEARFRAC(DATE(2024,2,1),DATE(2025,1,31),1)", "n:0.997267759562842"},     // 365 of 366: the February 29 of 2024 is in it
            {"YEARFRAC(DATE(2024,2,29),DATE(2025,2,28),1)", "n:0.997267759562842"},    // (from the 29th itself)
            {"YEARFRAC(DATE(2022,1,1),DATE(2024,7,1),1)", "n:2.4963503649635"},       // more than a year: 912 days over (365+365+366)/3
            {"YEARFRAC(DATE(2024,7,1),DATE(2022,1,1),1)", "n:2.4963503649635"},        // the dates the other way round
            {"YEARFRAC(DATE(2019,12,1),DATE(2020,12,1),1)", "n:1"},
            {"YEARFRAC(DATE(2019,12,1),DATE(2020,11,30),1)", "n:0.997267759562842"},   // 365 of 366
            {"YEARFRAC(DATE(2019,1,1),DATE(2019,1,1),1)", "n:0"},
            {"YEARFRAC(DATE(2023,6,1),DATE(2024,3,1),3)", "n:0.750684931506849"},
            {"YEARFRAC(DATE(2023,6,1),DATE(2024,3,1),2)", "n:0.761111111111111"},
            {"YEARFRAC(DATE(2023,6,1),DATE(2024,3,1),5)", "e:#NUM!"},
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
