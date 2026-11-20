#pragma once
// How a merge field shows its value: as the list holds it, or as a number, a currency amount, a percent, a date, or a time. A field's code is "merge:Name", and with a format
// "merge:Name|format"; the formats are written "number:2" (decimal places, 0 to 4), "currency" and "currency:0", "percent:1", "date:short", "date:long", "date:iso", "date:monthday",
// "date:monthyear", and "time:short", "time:12", "time:24". A value that is not a number or a date shows as the list holds it. Built once, for every app with merge fields.

#include "core/export.h"

#include <QString>

namespace jp {

struct MergeFieldCode {
    QString name;
    QString format;   // empty: as the list holds it
};

// The name and the format of what follows "merge:" in a field's code. A part after the last "|" that is not a format is a part of the name.
JO_COMMON_EXPORT MergeFieldCode splitMergeField(const QString &arg);
JO_COMMON_EXPORT QString joinMergeField(const QString &name, const QString &format);

JO_COMMON_EXPORT bool isMergeFormat(const QString &format);

// The value as the format shows it. Reading the value and showing the result both follow `language` (a tag such as "de-DE": the language of the text at the field; empty is US English,
// as in the rest of the layout), never the machine's: 03/04/2026 is the 3rd of April in a day-first language, and "1.234,5" is 1234.5 in German. A year of two digits is 2000 to 2029 for
// 00 to 29, and 1930 to 1999 for 30 to 99. An empty format, an empty value, a value the format can't read (nan, inf, words, a date for a time format or a time for a date format) give
// the value exactly as it is. "(500)" is -500.
JO_COMMON_EXPORT QString formatMergeValue(const QString &value, const QString &format, const QString &language = QString());

} // namespace jp
