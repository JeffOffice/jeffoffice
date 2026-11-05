#include "core/pivotadvice.h"

#include <QCoreApplication>

#include <algorithm>

namespace jp::crunch {

using namespace jp::office::sheet;

namespace {

bool isDateField(const PivotCacheField &f)
{
    bool any = false;
    for (const PivotValue &v : f.storedItems()) {
        if (v.kind == PivotValue::Missing) continue;
        if (v.kind != PivotValue::Date) return false;
        any = true;
    }
    return any;
}

// How many different values the field has (blanks left out).
int distinctOf(const PivotCacheField &f)
{
    int n = 0;
    for (const PivotValue &v : f.storedItems())
        if (v.kind != PivotValue::Missing) ++n;
    return n;
}

} // namespace

QVector<PivotAdvice> adviseFromCache(const PivotCache &cache)
{
    QVector<PivotAdvice> out;
    if (cache.recordCount < 2 || cache.fields.isEmpty() || !cache.unsupported.isEmpty()) return out;
    QVector<int> groups, sums, dates;
    for (int f = 0; f < cache.fields.size(); ++f) {
        const PivotCacheField &field = cache.fields[f];
        if (field.group.kind != PivotFieldGroup::None || field.name.isEmpty()) continue;
        const int distinct = distinctOf(field);
        if (isDateField(field)) {
            if (distinct >= 2 && distinct < cache.recordCount) dates << f;
        } else if (pivotFieldIsNumeric(cache, f)) {
            // (a column that is different on every line is a number or an id of each line, not a figure to total)
            if (distinct >= 1) sums << f;
        } else if (distinct >= 2 && distinct <= 100 && distinct < cache.recordCount) {
            groups << f;
        }
    }
    if (groups.size() > 3) groups.resize(3);
    if (sums.size() > 2) sums.resize(2);
    // the first numbers by the first column that repeats, the first numbers by dates, the lines counted by each column that repeats, and then the rest
    auto add = [&](int row, int value, bool count, bool byDate) {
        if (out.size() >= kPivotAdviceMax) return;
        for (const PivotAdvice &a : std::as_const(out))
            if (a.rowField == row && a.valueField == value && a.count == count) return;
        PivotAdvice a;
        a.rowField = row;
        a.valueField = value;
        a.count = count;
        a.byDate = byDate;
        const QString rowName = cache.fields[row].name;
        const QString valueName = cache.fields[value].name;
        if (count) {
            a.title = QCoreApplication::translate("jp::crunch::PivotAdvice", "Count of %1").arg(rowName);
            a.description = QCoreApplication::translate("jp::crunch::PivotAdvice", "Shows how many lines there are for each value of %1.").arg(rowName);
        } else if (byDate) {
            a.title = QCoreApplication::translate("jp::crunch::PivotAdvice", "Sum of %1 by %2").arg(valueName, rowName);
            a.description = QCoreApplication::translate("jp::crunch::PivotAdvice", "Shows the total of %1 for each month of %2, grouped by year.").arg(valueName, rowName);
        } else {
            a.title = QCoreApplication::translate("jp::crunch::PivotAdvice", "Sum of %1 by %2").arg(valueName, rowName);
            a.description = QCoreApplication::translate("jp::crunch::PivotAdvice", "Shows the total of %1 for each value of %2.").arg(valueName, rowName);
        }
        out << a;
    };
    if (!groups.isEmpty() && !sums.isEmpty()) add(groups[0], sums[0], false, false);
    if (!dates.isEmpty() && !sums.isEmpty()) add(dates[0], sums[0], false, true);
    if (!groups.isEmpty()) add(groups[0], groups[0], true, false);
    for (int g = 0; g < groups.size(); ++g)
        for (int s = 0; s < sums.size(); ++s) add(groups[g], sums[s], false, false);
    for (int g = 1; g < groups.size(); ++g) add(groups[g], groups[g], true, false);
    if (!dates.isEmpty() && sums.size() > 1) add(dates[0], sums[1], false, true);
    return out;
}

} // namespace jp::crunch
