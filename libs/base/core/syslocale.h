#pragma once
// The person's locale (numbers, dates, month names, the separators) for code that runs on any thread. Qt's system locale (QLocale::system(), and a default-constructed QLocale() while the
// default is the system's) asks the operating system each time a format or a sign is wanted, and that is not safe beside another thread asking, or beside the main thread changing the
// default: on Windows a worker inside QSystemLocale::query crashed word-processor tests on November 20, 2026. So one place asks, on the main thread, and keeps an unchanging copy:
//   - jp::captureSystemLocale()  the main thread, at startup and when the language or the system's settings change
//   - jp::systemLocale()         the copy, by value, from any thread: a QLocale made from the language, script, and territory (it asks the system for nothing), and the person's own
//                                format strings and signs, which a QLocale cannot hold (a short date set in the system's settings), as plain text
// Worker code takes its locale from here and never calls QLocale::system(), never formats through a default-constructed QLocale(), and never uses QDate::toString(Qt::TextDate) or the
// Qt::SystemLocale formats; text of a fixed shape (a date in a file) goes through jp::fixedText below. tools/check_threads.py keeps them out of libs/ and apps/.
// On the main thread systemLocale() looks at the default locale again, so a test that sets it (tests/localepin.h) sees its own choice, as QLocale() did.

#include "core/export.h"

#include <QDate>
#include <QDateTime>
#include <QLocale>
#include <QString>
#include <QStringList>
#include <QTime>

#include <type_traits>

namespace jp {

struct JO_COMMON_EXPORT SystemLocale {
    QLocale locale;            // (made from the names, never the system's own object)
    QString shortDate, longDate, shortTime, longTime, shortDateTime, longDateTime;   // the person's formats, as the system gave them
    QString decimalPoint, groupSeparator;
    QString currencySign, amText, pmText;   // (the person's own: the currency sign, and the words for morning and afternoon)
    QStringList uiLanguages;

    // Dates and times in the person's formats (the strings above, with the month and day names of `locale`).
    QString toString(const QDate &date, QLocale::FormatType type = QLocale::ShortFormat) const;
    QString toString(const QTime &time, QLocale::FormatType type = QLocale::ShortFormat) const;
    QString toString(const QDateTime &when, QLocale::FormatType type = QLocale::ShortFormat) const;
    // A whole number or a decimal one, with the person's group and decimal signs.
    QString toString(qint64 number) const;
    template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
    QString toString(T number) const { return toString(static_cast<qint64>(number)); }
    QString toString(double number, char format = 'g', int precision = 6) const;
    // A size in bytes ("1.5 MB"), with the person's decimal sign.
    QString formattedDataSize(qint64 bytes, int precision = 2, QLocale::DataSizeFormats format = QLocale::DataSizeIecFormat) const;
};

// Main thread only: asks the system (through the default locale) and keeps the answer for every thread.
JO_COMMON_EXPORT void captureSystemLocale();
// Main thread only: asks again and keeps the answer if a format or sign differs from the copy's (the program's window brought to the front calls it).
JO_COMMON_EXPORT void refreshSystemLocaleIfChanged();
// For tests: how many reads found no capture yet (or came from a thread that is not the main one) and got the fixed default; and forgetting the capture and the main thread.
JO_COMMON_EXPORT qint64 uncapturedSystemLocaleReadsForTest();
JO_COMMON_EXPORT void forgetSystemLocaleForTest();
// For tests: a person's settings as the copy (the calling thread is named the main one if there is no application object), for a test of a custom decimal sign or date order on any platform.
JO_COMMON_EXPORT void setSystemLocaleForTest(const SystemLocale &person);
// Any thread. (Called on the main thread it first takes the default locale again if that is not the copy's.)
JO_COMMON_EXPORT SystemLocale systemLocale();

// The locale object of the copy: what worker code passes where a QLocale is wanted.
inline QLocale workerLocale() { return systemLocale().locale; }

// Text of a fixed shape (a date in a file, a version stamp): the format's letters are digits and punctuation, not the person's language. The C locale is Qt's own for it.
inline QString fixedText(const QDateTime &when, const QString &format) { return QLocale::c().toString(when, format); }
inline QString fixedText(const QDate &date, const QString &format) { return QLocale::c().toString(date, format); }
inline QString fixedText(const QTime &time, const QString &format) { return QLocale::c().toString(time, format); }

} // namespace jp
