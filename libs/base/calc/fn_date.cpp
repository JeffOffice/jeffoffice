// The date and time functions, by the serial numbers of calc/dates.h.
#include "calc/machine.h"

#include <algorithm>
#include <cmath>

namespace jp::calc {

namespace {

constexpr double kLastSerial = 2958465;   // December 31, 9999

bool isLeap(int y) { return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0; }

// The days of a month in the calendar: the February 29, 1900 of the 1900 system is no day of it, and EDATE, EOMONTH, and the days of a year count February 1900 as the calendar does.
int monthDays(int y, int m, DateSystem)
{
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (m == 2 && isLeap(y)) return 29;
    return days[m - 1];
}

// The date of a serial number, whole days: false if it is before day 0 or after December 31, 9999. Day 0 of the 1900 system is January 0, 1900.
bool dateOf(double serial, DateSystem system, int *y, int *m, int *d)
{
    if (!(serial >= 0) || serial > kLastSerial) return false;
    const double whole = std::floor(serial);
    if (system == DateSystem::Date1900 && whole == 0) {
        *y = 1900;
        *m = 1;
        *d = 0;
        return true;
    }
    return serialToDate(whole, system, y, m, d);
}

// A number of an argument that is a date or a time: a number, or a text that reads as one.
bool serialArg(Call &c, int i, double *out)
{
    if (!c.number(i, out)) return false;
    if (*out < 0 || *out > kLastSerial + 1) {
        c.error(ErrorKind::Num);
        return false;
    }
    return true;
}

Value fnDate(Call &c)
{
    double yd, md, dd;
    if (!c.number(0, &yd) || !c.number(1, &md) || !c.number(2, &dd)) return c.failure();
    yd = std::trunc(yd);
    md = std::trunc(md);
    dd = std::trunc(dd);
    if (yd < 0 || yd > 9999 || std::fabs(md) > 1e7 || std::fabs(dd) > 1e9) return c.error(ErrorKind::Num);
    qint64 year = qint64(yd);
    if (year < 1900) year += 1900;
    qint64 month0 = (year * 12 + qint64(md) - 1);
    const qint64 y = month0 >= 0 ? month0 / 12 : -((-month0 + 11) / 12);
    const int m = int(month0 - y * 12) + 1;
    if (y < 1900 && c.context().dateSystem == DateSystem::Date1900) return c.error(ErrorKind::Num);
    if (y < 1900 || y > 9999) return c.error(ErrorKind::Num);
    double base;
    if (!dateToSerial(int(y), m, 1, c.context().dateSystem, &base)) return c.error(ErrorKind::Num);
    const double serial = base + dd - 1;
    if (serial < 0 || serial > kLastSerial) return c.error(ErrorKind::Num);
    return Value::number(serial);
}

Value part(Call &c, int which)
{
    double s;
    if (!serialArg(c, 0, &s)) return c.failure();
    int y, m, d;
    if (!dateOf(s, c.context().dateSystem, &y, &m, &d)) return c.error(ErrorKind::Num);
    return Value::number(which == 0 ? y : which == 1 ? m : d);
}
Value fnYear(Call &c) { return part(c, 0); }
Value fnMonth(Call &c) { return part(c, 1); }
Value fnDay(Call &c) { return part(c, 2); }

// The day of the week of a serial number, 0 for Sunday.
int weekdayOf(double serial, DateSystem system)
{
    const qint64 n = qint64(std::floor(serial));
    const qint64 r = system == DateSystem::Date1904 ? (n + 5) % 7 : ((n - 1) % 7 + 7) % 7;
    return int(r);
}

Value fnWeekday(Call &c)
{
    double s, type;
    if (!serialArg(c, 0, &s) || !c.numberOr(1, 1, &type)) return c.failure();
    const int dow = weekdayOf(s, c.context().dateSystem);
    const int t = int(std::trunc(type));
    if (t == 1) return Value::number(dow + 1);
    if (t == 2) return Value::number((dow + 6) % 7 + 1);
    if (t == 3) return Value::number((dow + 6) % 7);
    if (t >= 11 && t <= 17) {
        const int start = (t - 10) % 7;   // 11: Monday ... 16: Saturday, 17: Sunday
        return Value::number((dow - start + 7) % 7 + 1);
    }
    return c.error(ErrorKind::Num);
}

// The serial number of January 1 of a year, in the days of the system: for a year before the system's first (the year a week of the first days may belong to) or after
// 9999, the calendar's count of days from or to a year it has.
qint64 firstDayOf(int year, DateSystem sys)
{
    double serial;
    if (dateToSerial(year, 1, 1, sys, &serial)) return qint64(serial);
    if (year < 5000) return firstDayOf(year + 1, sys) - (isLeap(year) ? 366 : 365);
    return firstDayOf(year - 1, sys) + (isLeap(year - 1) ? 366 : 365);
}

// The week of ISO 8601: weeks begin on Monday, and week 1 of a year is the one with its first Thursday, so the days at the turn of a year may be in the last week of the
// year before or the first of the year after. The Thursday of a day's week gives the year, and the weeks from the year's first day give the number.
bool isoWeekOf(double serial, DateSystem sys, int *week)
{
    int y, m, d;
    if (!dateOf(serial, sys, &y, &m, &d)) return false;
    const int dow = (weekdayOf(serial, sys) + 6) % 7;   // Monday = 0
    const qint64 thursday = qint64(std::floor(serial)) - dow + 3;
    int year = y;
    if (thursday < firstDayOf(y, sys)) year = y - 1;
    else if (thursday >= firstDayOf(y + 1, sys)) year = y + 1;
    *week = int((thursday - firstDayOf(year, sys)) / 7 + 1);
    return true;
}

Value fnWeekNum(Call &c)
{
    double s, type;
    if (!serialArg(c, 0, &s) || !c.numberOr(1, 1, &type)) return c.failure();
    const DateSystem sys = c.context().dateSystem;
    int y, m, d;
    if (!dateOf(s, sys, &y, &m, &d)) return c.error(ErrorKind::Num);
    const int t = int(std::trunc(type));
    if (t == 21) {
        int week;
        if (!isoWeekOf(s, sys, &week)) return c.error(ErrorKind::Num);
        return Value::number(week);
    }
    double jan1;
    if (!dateToSerial(y, 1, 1, sys, &jan1)) return c.error(ErrorKind::Num);
    const double doy = std::floor(s) - jan1;   // from 0
    const int jan1Dow = weekdayOf(jan1, sys);
    int start;   // the day weeks start on, 0 for Sunday
    if (t == 1) start = 0;
    else if (t == 2) start = 1;
    else if (t >= 11 && t <= 17) start = (t - 10) % 7;
    else return c.error(ErrorKind::Num);
    const int offset = (jan1Dow - start + 7) % 7;
    return Value::number(std::floor((doy + offset) / 7.0) + 1);
}

Value fnIsoWeekNum(Call &c)
{
    double s;
    if (!serialArg(c, 0, &s)) return c.failure();
    int week;
    if (!isoWeekOf(s, c.context().dateSystem, &week)) return c.error(ErrorKind::Num);
    return Value::number(week);
}

// The seconds of a day of a serial number's fraction, to the nearest second.
qint64 secondsOfDay(double serial)
{
    const double frac = serial - std::floor(serial);
    return qint64(std::floor(frac * 86400.0 + 0.5)) % 86400;
}

Value fnHour(Call &c)
{
    double s;
    if (!serialArg(c, 0, &s)) return c.failure();
    return Value::number(double(secondsOfDay(s) / 3600));
}
Value fnMinute(Call &c)
{
    double s;
    if (!serialArg(c, 0, &s)) return c.failure();
    return Value::number(double((secondsOfDay(s) / 60) % 60));
}
Value fnSecond(Call &c)
{
    double s;
    if (!serialArg(c, 0, &s)) return c.failure();
    return Value::number(double(secondsOfDay(s) % 60));
}

Value fnTime(Call &c)
{
    double h, m, s;
    if (!c.number(0, &h) || !c.number(1, &m) || !c.number(2, &s)) return c.failure();
    h = std::trunc(h);
    m = std::trunc(m);
    s = std::trunc(s);
    if (h < 0 || m < 0 || s < 0 || h > 32767 || m > 32767 || s > 32767) return c.error(ErrorKind::Num);
    const double total = h * 3600 + m * 60 + s;
    return Value::number(std::fmod(total, 86400.0) / 86400.0);
}

Value fnTimeValue(Call &c)
{
    const Value v = c.scalar(0);
    if (v.isError()) return v;
    if (!v.isText() || !v.text().contains(QLatin1Char(':'))) return c.error(ErrorKind::Value);
    double x;
    if (!textToNumber(v.text(), c.context().dateSystem, &x)) return c.error(ErrorKind::Value);
    return Value::number(x - std::floor(x));
}

Value fnDateValue(Call &c)
{
    const Value v = c.scalar(0);
    if (v.isError()) return v;
    if (!v.isText()) return c.error(ErrorKind::Value);
    double x;
    if (!dateValueOf(v.text(), c.context().dateSystem, &x)) return c.error(ErrorKind::Value);
    return Value::number(std::floor(x));
}

Value fnToday(Call &c) { return Value::number(std::floor(c.machine().now())); }
Value fnNow(Call &c) { return Value::number(c.machine().now()); }

// A date `months` after the one of a serial number: the day of the month is kept (EDATE) or is the last (EOMONTH).
Value monthsFrom(Call &c, bool endOfMonth)
{
    double s, months;
    if (!serialArg(c, 0, &s) || !c.number(1, &months)) return c.failure();
    const DateSystem sys = c.context().dateSystem;
    int y, m, d;
    if (!dateOf(s, sys, &y, &m, &d)) return c.error(ErrorKind::Num);
    months = std::trunc(months);
    if (std::fabs(months) > 1e6) return c.error(ErrorKind::Num);
    if (d == 0) d = 1;
    const qint64 total = qint64(y) * 12 + (m - 1) + qint64(months);
    const qint64 ny = total >= 0 ? total / 12 : -((-total + 11) / 12);
    const int nm = int(total - ny * 12) + 1;
    if (ny < 1900 || ny > 9999) return c.error(ErrorKind::Num);
    const int days = monthDays(int(ny), nm, sys);
    double serial;
    if (!dateToSerial(int(ny), nm, endOfMonth ? days : std::min(d, days), sys, &serial)) return c.error(ErrorKind::Num);
    return Value::number(serial);
}
Value fnEdate(Call &c) { return monthsFrom(c, false); }
Value fnEomonth(Call &c) { return monthsFrom(c, true); }

// The holidays of an argument: whole serial numbers.
bool holidays(Call &c, int i, std::vector<double> *out)
{
    if (i >= c.argc() || c.missing(i)) return true;
    Value error;
    bool ok = true;
    const bool done = c.each(i, [&](const Value &v, bool) {
        if (v.isError()) {
            error = v;
            ok = false;
            return false;
        }
        if (v.isNumber()) out->push_back(std::floor(v.number()));
        return true;
    });
    if (!done) {
        c.error(ok ? ErrorKind::Value : error.errorKind());
        return false;
    }
    return true;
}

bool isWorkday(double serial, DateSystem sys, const std::vector<double> &off)
{
    const int dow = weekdayOf(serial, sys);
    if (dow == 0 || dow == 6) return false;
    return std::find(off.begin(), off.end(), serial) == off.end();
}

Value fnWorkday(Call &c)
{
    double start, days;
    std::vector<double> off;
    if (!serialArg(c, 0, &start) || !c.number(1, &days) || !holidays(c, 2, &off)) return c.failure();
    const DateSystem sys = c.context().dateSystem;
    double d = std::floor(start);
    days = std::trunc(days);
    if (std::fabs(days) > 1e6) return c.error(ErrorKind::Num);
    const int step = days < 0 ? -1 : 1;
    for (qint64 left = qint64(std::fabs(days)); left > 0;) {
        d += step;
        if (d < 0 || d > kLastSerial) return c.error(ErrorKind::Num);
        if (!c.machine().charge(1)) return c.error(ErrorKind::Value);
        if (isWorkday(d, sys, off)) --left;
    }
    return Value::number(d);
}

Value fnNetworkDays(Call &c)
{
    double a, b;
    std::vector<double> off;
    if (!serialArg(c, 0, &a) || !serialArg(c, 1, &b) || !holidays(c, 2, &off)) return c.failure();
    const DateSystem sys = c.context().dateSystem;
    a = std::floor(a);
    b = std::floor(b);
    const double lo = std::min(a, b), hi = std::max(a, b);
    if (hi - lo > 1e7) return c.error(ErrorKind::Num);
    qint64 n = 0;
    for (double d = lo; d <= hi; d += 1) {
        if (!c.machine().charge(1)) return c.error(ErrorKind::Value);
        if (isWorkday(d, sys, off)) ++n;
    }
    return Value::number(a <= b ? double(n) : -double(n));
}

// DATEDIF(start, end, unit): complete years ("Y"), months ("M"), or days ("D") between two dates, or the months ("YM"), days ("MD"), or days ("YD") left over once the
// years, the years and months, or the years are taken off.
Value fnDatedif(Call &c)
{
    double a, b;
    QString unit;
    if (!serialArg(c, 0, &a) || !serialArg(c, 1, &b) || !c.text(2, &unit)) return c.failure();
    a = std::floor(a);
    b = std::floor(b);
    if (a > b) return c.error(ErrorKind::Num);
    const DateSystem sys = c.context().dateSystem;
    int y1, m1, d1, y2, m2, d2;
    if (!dateOf(a, sys, &y1, &m1, &d1) || !dateOf(b, sys, &y2, &m2, &d2)) return c.error(ErrorKind::Num);
    unit = unit.toUpper();
    int months = (y2 - y1) * 12 + (m2 - m1);
    if (d2 < d1) --months;
    if (unit == QLatin1String("D")) return Value::number(b - a);
    if (unit == QLatin1String("M")) return Value::number(months);
    if (unit == QLatin1String("Y")) return Value::number(months / 12);
    if (unit == QLatin1String("YM")) return Value::number(months % 12);
    if (unit == QLatin1String("MD")) {
        if (d2 >= d1) return Value::number(d2 - d1);
        const int pm = m2 == 1 ? 12 : m2 - 1, py = m2 == 1 ? y2 - 1 : y2;
        return Value::number(monthDays(py, pm, sys) - d1 + d2);
    }
    if (unit == QLatin1String("YD")) {
        double anchor;
        int ay = y2;
        if (!dateToSerial(ay, m1, std::min(d1, monthDays(ay, m1, sys)), sys, &anchor)) return c.error(ErrorKind::Num);
        if (anchor > b) {
            --ay;
            if (!dateToSerial(ay, m1, std::min(d1, monthDays(ay, m1, sys)), sys, &anchor)) return c.error(ErrorKind::Num);
        }
        return Value::number(b - anchor);
    }
    return c.error(ErrorKind::Num);
}

Value fnDays(Call &c)
{
    double e, s;
    if (!serialArg(c, 0, &e) || !serialArg(c, 1, &s)) return c.failure();
    return Value::number(std::floor(e) - std::floor(s));
}

// Days between two dates counting 30 days to a month (the United States' rule, or with european = true the european one).
double days360(int y1, int m1, int d1, int y2, int m2, int d2, bool european, DateSystem sys)
{
    if (european) {
        if (d1 == 31) d1 = 30;
        if (d2 == 31) d2 = 30;
    } else {
        const bool startLast = d1 == monthDays(y1, m1, sys);
        const bool endLast = d2 == monthDays(y2, m2, sys);
        if (startLast) d1 = 30;
        if (endLast) {
            if (d1 < 30) {
                d2 = 1;
                if (++m2 > 12) {
                    m2 = 1;
                    ++y2;
                }
            } else {
                d2 = 30;
            }
        }
    }
    return double((y2 - y1) * 360 + (m2 - m1) * 30 + (d2 - d1));
}

Value fnDays360(Call &c)
{
    double a, b;
    bool european;
    if (!serialArg(c, 0, &a) || !serialArg(c, 1, &b) || !c.booleanOr(2, false, &european)) return c.failure();
    const DateSystem sys = c.context().dateSystem;
    int y1, m1, d1, y2, m2, d2;
    if (!dateOf(a, sys, &y1, &m1, &d1) || !dateOf(b, sys, &y2, &m2, &d2)) return c.error(ErrorKind::Num);
    return Value::number(days360(y1, m1, d1, y2, m2, d2, european, sys));
}

Value fnYearFrac(Call &c)
{
    double a, b, basis;
    if (!serialArg(c, 0, &a) || !serialArg(c, 1, &b) || !c.numberOr(2, 0, &basis)) return c.failure();
    const DateSystem sys = c.context().dateSystem;
    a = std::floor(a);
    b = std::floor(b);
    if (a > b) std::swap(a, b);
    int y1, m1, d1, y2, m2, d2;
    if (!dateOf(a, sys, &y1, &m1, &d1) || !dateOf(b, sys, &y2, &m2, &d2)) return c.error(ErrorKind::Num);
    switch (int(std::trunc(basis))) {
    case 0: return Value::number(days360(y1, m1, d1, y2, m2, d2, false, sys) / 360.0);
    case 4: return Value::number(days360(y1, m1, d1, y2, m2, d2, true, sys) / 360.0);
    case 2: return Value::number((b - a) / 360.0);
    case 3: return Value::number((b - a) / 365.0);
    case 1: {
        // actual days over the average length of the years the dates span
        double years = 0;
        for (int y = y1; y <= y2; ++y) years += isLeap(y) ? 366 : 365;
        const double avg = years / double(y2 - y1 + 1);
        if (y1 == y2) return Value::number((b - a) / (isLeap(y1) ? 366.0 : 365.0));
        return Value::number((b - a) / avg);
    }
    default: return c.error(ErrorKind::Num);
    }
}

} // namespace

void registerDateFunctions(std::vector<FunctionInfo> &r)
{
    r.push_back({"DATE", 3, 3, "vvv", 0, fnDate});
    r.push_back({"YEAR", 1, 1, "v", 0, fnYear});
    r.push_back({"MONTH", 1, 1, "v", 0, fnMonth});
    r.push_back({"DAY", 1, 1, "v", 0, fnDay});
    r.push_back({"WEEKDAY", 1, 2, "vv", 0, fnWeekday});
    r.push_back({"WEEKNUM", 1, 2, "vv", 0, fnWeekNum});
    r.push_back({"ISOWEEKNUM", 1, 1, "v", kFnFuture, fnIsoWeekNum});
    r.push_back({"HOUR", 1, 1, "v", 0, fnHour});
    r.push_back({"MINUTE", 1, 1, "v", 0, fnMinute});
    r.push_back({"SECOND", 1, 1, "v", 0, fnSecond});
    r.push_back({"TIME", 3, 3, "vvv", 0, fnTime});
    r.push_back({"TIMEVALUE", 1, 1, "v", 0, fnTimeValue});
    r.push_back({"DATEVALUE", 1, 1, "v", 0, fnDateValue});
    r.push_back({"TODAY", 0, 0, "", kFnVolatile, fnToday});
    r.push_back({"NOW", 0, 0, "", kFnVolatile, fnNow});
    r.push_back({"EDATE", 2, 2, "vv", 0, fnEdate});
    r.push_back({"EOMONTH", 2, 2, "vv", 0, fnEomonth});
    r.push_back({"WORKDAY", 2, 3, "vvr", 0, fnWorkday});
    r.push_back({"NETWORKDAYS", 2, 3, "vvr", 0, fnNetworkDays});
    r.push_back({"DAYS", 2, 2, "vv", kFnFuture, fnDays});
    r.push_back({"DATEDIF", 3, 3, "vvv", 0, fnDatedif});
    r.push_back({"DAYS360", 2, 3, "vvv", 0, fnDays360});
    r.push_back({"YEARFRAC", 2, 3, "vvv", 0, fnYearFrac});
}

} // namespace jp::calc
