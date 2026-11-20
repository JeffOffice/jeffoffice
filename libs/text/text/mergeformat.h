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

// The value as the format shows it, in the person's language and region. An empty format, a value that is empty, and a value the format can't read give the value as it is.
JO_COMMON_EXPORT QString formatMergeValue(const QString &value, const QString &format);

} // namespace jp
