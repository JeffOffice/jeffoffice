#include "sheet/dates.h"

#include <cmath>

namespace jp::office::sheet {

namespace {

// Days from 1970-01-01 to the date in the proleptic Gregorian calendar (the days-from-civil algorithm).
qint64 daysFromCivil(qint64 y, unsigned m, unsigned d)
{
    y -= m <= 2;
    const qint64 era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = unsigned(y - era * 400);
    const unsigned doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + qint64(doe) - 719468;
}

void civilFromDays(qint64 z, int *year, int *month, int *day)
{
    z += 719468;
    const qint64 era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = unsigned(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    const qint64 y = qint64(yoe) + era * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    const unsigned d = doy - (153 * mp + 2) / 5 + 1;
    const unsigned m = mp < 10 ? mp + 3 : mp - 9;
    *year = int(y + (m <= 2));
    *month = int(m);
    *day = int(d);
}

bool leap(int y) { return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0; }

int daysInMonth(int y, int m)
{
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return m == 2 && leap(y) ? 29 : days[m - 1];
}

// The day numbers of two dates the systems are counted from: 1899-12-30 for the 1900 system (from March 1, 1900), 1904-01-01 for the 1904 one.
const qint64 kEpoch1899 = -25569;   // days from 1970-01-01 to 1899-12-30
const qint64 kEpoch1904 = -24107;   // and to 1904-01-01

} // namespace

bool dateToSerial(int year, int month, int day, DateSystem system, double *serial)
{
    if (year < 1900 || year > 9999 || month < 1 || month > 12 || day < 1) return false;
    // The 1900 system has a February 29, 1900, which is no date in the calendar.
    const bool fiction = system == DateSystem::Date1900 && year == 1900 && month == 2 && day == 29;
    if (!fiction && day > daysInMonth(year, month)) return false;
    if (fiction) {
        *serial = 60;
        return true;
    }
    const qint64 days = daysFromCivil(year, unsigned(month), unsigned(day));
    if (system == DateSystem::Date1904) {
        const qint64 n = days - kEpoch1904;
        if (n < 0) return false;
        *serial = double(n);
        return true;
    }
    qint64 n = days - kEpoch1899;
    // Day 60 is the February 29 that never was, so the days before March 1 count one less than the calendar's.
    if (n < 61) --n;
    if (n < 1) return false;
    *serial = double(n);
    return true;
}

bool serialToDate(double serial, DateSystem system, int *year, int *month, int *day)
{
    if (!(serial >= 0) || serial >= 2958466) return false;
    const qint64 whole = qint64(std::floor(serial));
    if (system == DateSystem::Date1904) {
        civilFromDays(whole + kEpoch1904, year, month, day);
        return *year <= 9999;
    }
    if (whole < 1) return false;
    if (whole == 60) {
        *year = 1900;
        *month = 2;
        *day = 29;
        return true;
    }
    civilFromDays(whole + kEpoch1899 + (whole < 60 ? 1 : 0), year, month, day);
    return *year <= 9999;
}

bool parseIsoDate(const QString &text, DateSystem system, double *serial)
{
    const QString s = text.trimmed();
    int i = 0;
    auto digits = [&](int count, int *value) {
        if (i + count > s.size()) return false;
        int v = 0;
        for (int k = 0; k < count; ++k) {
            const ushort c = s[i + k].unicode();
            if (c < '0' || c > '9') return false;
            v = v * 10 + (c - '0');
        }
        *value = v;
        i += count;
        return true;
    };
    auto expect = [&](QChar c) {
        if (i < s.size() && s[i] == c) {
            ++i;
            return true;
        }
        return false;
    };
    double day = 0, time = 0;
    bool haveDate = false;
    // A date first, or a time alone (what has a colon in its third place).
    if (!(s.size() >= 3 && s[2] == QLatin1Char(':'))) {
        int y = 0, m = 0, d = 0;
        if (!digits(4, &y) || !expect(QLatin1Char('-')) || !digits(2, &m) || !expect(QLatin1Char('-')) || !digits(2, &d)) return false;
        if (!dateToSerial(y, m, d, system, &day)) return false;
        haveDate = true;
    }
    if (i < s.size()) {
        if (haveDate && !expect(QLatin1Char('T')) && !expect(QLatin1Char(' '))) return false;
        int hh = 0, mm = 0, ss = 0;
        if (!digits(2, &hh) || !expect(QLatin1Char(':')) || !digits(2, &mm)) return false;
        if (expect(QLatin1Char(':'))) {
            if (!digits(2, &ss)) return false;
        }
        double fraction = 0;
        if (expect(QLatin1Char('.'))) {
            double scale = 0.1;
            int count = 0;
            while (i < s.size() && s[i].unicode() >= '0' && s[i].unicode() <= '9') {
                fraction += scale * (s[i].unicode() - '0');
                scale /= 10;
                ++i;
                ++count;
            }
            if (count == 0) return false;
        }
        if (hh > 24 || mm > 59 || ss > 59 || (hh == 24 && (mm || ss))) return false;
        time = (hh * 3600.0 + mm * 60.0 + ss + fraction) / 86400.0;
        // A zone ("Z", "+02:00") is read and dropped.
        if (i < s.size() && s[i] == QLatin1Char('Z')) {
            ++i;
        } else if (i < s.size() && (s[i] == QLatin1Char('+') || s[i] == QLatin1Char('-'))) {
            ++i;
            int zh = 0, zm = 0;
            if (!digits(2, &zh)) return false;
            expect(QLatin1Char(':'));
            if (i < s.size() && !digits(2, &zm)) return false;
        }
        if (i != s.size()) return false;
    } else if (!haveDate) {
        return false;
    }
    *serial = day + time;
    return true;
}

} // namespace jp::office::sheet
