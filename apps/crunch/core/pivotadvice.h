#pragma once
// Recommended pivot tables: a few ready layouts picked from the shape of the data behind a pivot table (which columns hold text that repeats, numbers, or dates). No GUI.

#include "sheet/pivot.h"

#include <QString>
#include <QStringList>
#include <QVector>

namespace jp::crunch {

constexpr int kPivotAdviceMax = 6;

struct PivotAdvice {
    QString title;         // "Sum of Amount by Region"
    QString description;   // a sentence that says what the table shows
    int rowField = -1;     // the field down the left (a field of the cache)
    int valueField = -1;   // the field that is totaled or counted
    bool count = false;    // counted (a count of the row field's lines), not totaled
    bool byDate = false;   // the rows are dates, grouped by years and months
};

// The layouts that suit the cache, best first, at most kPivotAdviceMax: the number of lines of each text column that repeats (counted), the numbers totaled by each such column, and by a
// column of dates. None for data with no column to group by.
QVector<PivotAdvice> adviseFromCache(const jp::office::sheet::PivotCache &cache);

} // namespace jp::crunch
