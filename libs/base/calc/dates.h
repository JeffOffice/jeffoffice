#pragma once
// Dates as a spreadsheet counts them: a day number, and a fraction of a day for
// the time. The file says which of two systems it counts in.

#include <QString>

namespace jp::calc {

enum class DateSystem {
    Date1900,   // day 1 is January 1, 1900; the days from March 1, 1900 are one more than the true count, because day 60 is the February 29, 1900 that never was
    Date1904,   // day 0 is January 1, 1904
};

// The day number of a date (year, month 1-12, day 1-31, as the calendar has them; February 29, 1900 is a date in the
// 1900 system, as it is in the files). False if the date isn't one or is before the system's day 0 or past year 9999.
bool dateToSerial(int year, int month, int day, DateSystem system, double *serial);
// The date of a day number (its whole part); false if it is before the system's day 0 or past December 31, 9999.
// Day 60 of the 1900 system is February 29, 1900.
bool serialToDate(double serial, DateSystem system, int *year, int *month, int *day);
// A date or time as a file writes one in a cell of type "d": "2024-03-15", "2024-03-15T13:45:30" (with fractions of
// a second, and a "Z" or an offset, which is read and dropped: the time is as written), or a time alone ("13:45:30").
// The day number it comes to; false if it is none of these.
bool parseIsoDate(const QString &text, DateSystem system, double *serial);

// DATEVALUE: the day number of a date written as text: month, day, and year as numbers with "/", "-", or "." between ("3/15/2024", "03-15-24"), the year
// first with "/" or "-" ("2024-03-15", "2024/3/5"), or with the month in English ("15-Mar-2024", "15 March 2024", "Mar 15, 2024"). A year of two digits is
// 2000 to 2029 for 00 to 29 and 1930 to 1999 for 30 to 99. A time of day after the date ("10:30", "10:30:15.5", "10:30 PM") is read and ignored. False if the
// text is none of these or not a date.
bool dateValueOf(const QString &text, DateSystem system, double *serial);

} // namespace jp::calc
