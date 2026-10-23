#include "calc/dates.h"

#include <QVector>

#include <cmath>

namespace jp::calc {

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

namespace {

// A date written out is its pieces: whole numbers, words, and the characters between them.
struct Piece {
    bool number = false;
    int value = 0;       // a number
    int digits = 0;
    QString word;        // lower case
    QChar sep;           // a separator
};

bool splitDateText(const QString &t, QVector<Piece> *out)
{
    int i = 0;
    while (i < t.size()) {
        const QChar c = t[i];
        Piece p;
        if (c.unicode() >= '0' && c.unicode() <= '9') {
            int v = 0, n = 0;
            while (i < t.size() && t[i].unicode() >= '0' && t[i].unicode() <= '9') {
                if (++n > 6) return false;
                v = v * 10 + (t[i].unicode() - '0');
                ++i;
            }
            p.number = true;
            p.value = v;
            p.digits = n;
        } else if (c.unicode() < 128 && c.isLetter()) {
            int j = i;
            while (j < t.size() && t[j].unicode() < 128 && t[j].isLetter()) ++j;
            p.word = t.mid(i, j - i).toLower();
            i = j;
        } else if (c == QLatin1Char('/') || c == QLatin1Char('-') || c == QLatin1Char('.') || c == QLatin1Char(',') || c == QLatin1Char(' ')) {
            p.sep = c;
            ++i;
        } else {
            return false;
        }
        out->append(p);
        if (out->size() > 12) return false;
    }
    return true;
}

int monthOfName(const QString &w)
{
    static const char *const names[12] = {"january", "february", "march", "april", "may", "june", "july", "august", "september", "october", "november", "december"};
    if (w.size() < 3) return 0;
    for (int m = 0; m < 12; ++m) {
        const QString full = QLatin1String(names[m]);
        if (w == full || w == full.left(3) || (m == 8 && w == QLatin1String("sept"))) return m + 1;
    }
    return 0;
}

// A year as it is written: four digits are the year, one or two are in 1930 to 2029 (00 to 29 are 2000 to 2029).
bool yearOf(const Piece &p, int *year)
{
    if (!p.number) return false;
    if (p.digits == 4) *year = p.value;
    else if (p.digits == 1 || p.digits == 2) *year = p.value < 30 ? 2000 + p.value : 1900 + p.value;
    else return false;
    return true;
}

// A time of day after a date: h:mm, h:mm:ss, or with fractions of a second, and AM or PM after it if the hour is of 12. It adds nothing to a day number.
bool timeOfDayText(const QString &t)
{
    int i = 0;
    auto number = [&](int minDigits, int maxDigits, int *v) {
        int n = 0, value = 0;
        while (i < t.size() && t[i].unicode() >= '0' && t[i].unicode() <= '9' && n < maxDigits) {
            value = value * 10 + (t[i].unicode() - '0');
            ++i;
            ++n;
        }
        *v = value;
        return n >= minDigits;
    };
    int hour = 0, minute = 0, second = 0;
    if (!number(1, 2, &hour) || i >= t.size() || t[i] != QLatin1Char(':')) return false;
    ++i;
    if (!number(2, 2, &minute)) return false;
    if (i < t.size() && t[i] == QLatin1Char(':')) {
        ++i;
        if (!number(2, 2, &second)) return false;
        if (i < t.size() && t[i] == QLatin1Char('.')) {
            ++i;
            int frac = 0;
            if (!number(1, 9, &frac)) return false;
        }
    }
    bool twelve = false;
    while (i < t.size() && t[i] == QLatin1Char(' ')) ++i;
    if (i < t.size()) {
        const QString rest = t.mid(i).toLower();
        if (rest != QLatin1String("am") && rest != QLatin1String("pm")) return false;
        twelve = true;
    }
    if (minute > 59 || second > 59) return false;
    return twelve ? hour >= 1 && hour <= 12 : hour <= 23;
}

} // namespace

bool dateValueOf(const QString &text, DateSystem system, double *serial)
{
    QString t = text.trimmed();
    if (t.isEmpty() || t.size() > 64) return false;
    // A time after the date is read and dropped.
    const int colon = int(t.indexOf(QLatin1Char(':')));
    if (colon >= 0) {
        const int space = int(t.lastIndexOf(QLatin1Char(' '), colon));
        if (space < 0 || !timeOfDayText(t.mid(space + 1).trimmed())) return false;
        t = t.left(space).trimmed();
    }
    QVector<Piece> p;
    if (!splitDateText(t, &p)) return false;
    int year = 0, month = 0, day = 0;
    auto small = [](const Piece &x) { return x.number && x.digits >= 1 && x.digits <= 2; };
    auto isSep = [](const Piece &x, QChar c) { return !x.number && x.word.isEmpty() && x.sep == c; };
    if (p.size() == 5 && p[0].number && p[0].digits == 4 && p[2].number && p[4].number && (isSep(p[1], QLatin1Char('-')) || isSep(p[1], QLatin1Char('/'))) && isSep(p[3], p[1].sep)) {
        // year, month, day
        if (!small(p[2]) || !small(p[4])) return false;
        year = p[0].value;
        month = p[2].value;
        day = p[4].value;
    } else if (p.size() == 5 && p[0].number && p[2].number && p[4].number && !p[1].number && p[1].word.isEmpty() && (p[1].sep == QLatin1Char('/') || p[1].sep == QLatin1Char('-') || p[1].sep == QLatin1Char('.'))
        && isSep(p[3], p[1].sep)) {
        // month, day, year
        if (!small(p[0]) || !small(p[2]) || !yearOf(p[4], &year)) return false;
        month = p[0].value;
        day = p[2].value;
    } else if (p.size() == 5 && small(p[0]) && !p[2].word.isEmpty() && p[4].number && !p[1].number && !p[3].number && p[1].word.isEmpty() && p[3].word.isEmpty()
               && (p[1].sep == QLatin1Char('-') || p[1].sep == QLatin1Char(' ')) && isSep(p[3], p[1].sep)) {
        // day, month's name, year
        month = monthOfName(p[2].word);
        day = p[0].value;
        if (!month || !yearOf(p[4], &year)) return false;
    } else if ((p.size() == 5 || p.size() == 6) && !p[0].number && !p[0].word.isEmpty() && isSep(p[1], QLatin1Char(' ')) && small(p[2])) {
        // month's name, day, year (with a comma after the day or not)
        month = monthOfName(p[0].word);
        day = p[2].value;
        int k = 3;
        if (isSep(p[k], QLatin1Char(','))) ++k;
        if (k >= p.size() || !isSep(p[k], QLatin1Char(' ')) || k + 2 != p.size() || !month || !yearOf(p[k + 1], &year)) return false;
    } else {
        return false;
    }
    return dateToSerial(year, month, day, system, serial);
}

} // namespace jp::calc
