// Grouping the values of a pivot cache field: dates by their parts, numbers in steps, and items put in groups by a person (the file's fieldGroup).
#include "sheet/pivot.h"

#include <QString>

#include <algorithm>
#include <cmath>
#include <functional>

namespace jp::office::sheet {

namespace {

const char *const kMonths[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

QString dateText(double serial, jp::calc::DateSystem dates)
{
    int y = 0, m = 0, d = 0;
    if (!jp::calc::serialToDate(std::floor(serial), dates, &y, &m, &d)) return QString::number(serial, 'g', 15);
    return QStringLiteral("%1/%2/%3").arg(m).arg(d).arg(y);
}

int granularity(const QString &by)
{
    static const char *const order[] = {"seconds", "minutes", "hours", "days", "months", "quarters", "years"};
    for (int i = 0; i < 7; ++i)
        if (by == QLatin1String(order[i])) return i;
    return -1;
}

// The day of a leap year (1 to 366) of a month and a day, for the group of days.
int leapDay(int month, int day)
{
    static const int before[] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};
    return before[month - 1] + day;
}

} // namespace

bool pivotGroupItems(const QVector<PivotValue> &items, PivotFieldGroup *g, jp::calc::DateSystem dates, QVector<qint32> *map, QString *why)
{
    auto fail = [&](const QString &m) {
        if (why) *why = m;
        return false;
    };
    if (g->kind != PivotFieldGroup::Dates && g->kind != PivotFieldGroup::Range) return fail(QStringLiteral("not a grouping of dates or numbers"));
    double lo = 0, hi = 0;
    bool first = true;
    for (const PivotValue &v : items) {
        if (!v.isNumeric()) return fail(QStringLiteral("the field has text or blanks"));
        if (first) lo = hi = v.number;
        else {
            lo = std::min(lo, v.number);
            hi = std::max(hi, v.number);
        }
        first = false;
    }
    if (first) return fail(QStringLiteral("the field has no values"));
    const bool dateParts = g->kind == PivotFieldGroup::Dates;
    const int gran = dateParts ? granularity(g->groupBy) : 0;
    if (dateParts && gran < 0) return fail(QStringLiteral("a grouping JeffCrunch does not work out"));
    if (g->autoStart) g->start = dateParts && gran >= 3 ? std::floor(lo) : lo;
    if (g->autoEnd) g->end = dateParts && gran >= 3 ? std::floor(hi) : hi;
    if (g->end < g->start) return fail(QStringLiteral("the end is before the start"));
    QStringList labels;
    labels << QLatin1Char('<') + (dateParts ? dateText(g->start, dates) : QString::number(g->start, 'g', 15));
    int n = 0;   // the groups between the two ends
    int firstYear = 0;
    if (!dateParts) {
        if (!(g->interval > 0)) return fail(QStringLiteral("the step must be more than zero"));
        const double bins = std::ceil((g->end - g->start) / g->interval - 1e-9);
        if (bins > 100000) return fail(QStringLiteral("too many groups"));
        n = std::max(1, int(bins));
        const bool whole = g->start == std::floor(g->start) && g->interval == std::floor(g->interval);
        for (int k = 0; k < n; ++k) {
            const double a = g->start + k * g->interval, b = a + g->interval;
            labels << QStringLiteral("%1-%2").arg(a, 0, 'g', 15).arg(whole ? b - 1 : b, 0, 'g', 15);
        }
    } else if (g->groupBy == QLatin1String("years")) {
        int y0 = 0, y1 = 0, m = 0, d = 0;
        if (!jp::calc::serialToDate(std::floor(g->start), dates, &y0, &m, &d) || !jp::calc::serialToDate(std::floor(g->end), dates, &y1, &m, &d)) return fail(QStringLiteral("a value is not a date"));
        if (y1 - y0 > 10000) return fail(QStringLiteral("too many groups"));
        firstYear = y0;
        for (int y = y0; y <= y1; ++y) labels << QString::number(y);
        n = y1 - y0 + 1;
    } else if (g->groupBy == QLatin1String("quarters")) {
        for (int q = 1; q <= 4; ++q) labels << QStringLiteral("Qtr%1").arg(q);
        n = 4;
    } else if (g->groupBy == QLatin1String("months")) {
        for (const char *m : kMonths) labels << QLatin1String(m);
        n = 12;
    } else if (g->groupBy == QLatin1String("days")) {
        static const int lengths[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        for (int m = 0; m < 12; ++m)
            for (int d = 1; d <= lengths[m]; ++d) labels << QStringLiteral("%1-%2").arg(d).arg(QLatin1String(kMonths[m]));
        n = 366;
    } else if (g->groupBy == QLatin1String("hours")) {
        for (int h = 0; h < 24; ++h) labels << QString::number(h);
        n = 24;
    } else {
        for (int k = 0; k < 60; ++k) labels << QString::number(k);
        n = 60;
    }
    labels << QLatin1Char('>') + (dateParts ? dateText(g->end, dates) : QString::number(g->end, 'g', 15));
    g->labels = labels;
    if (!map) return true;
    map->assign(items.size(), -1);
    const qint32 last = qint32(labels.size()) - 1;
    for (qsizetype i = 0; i < items.size(); ++i) {
        const double v = items[i].number;
        qint32 at = 0;
        if (v < g->start) {
            at = 0;
        } else if (v > g->end && !(dateParts && gran >= 3 && std::floor(v) <= g->end)) {
            at = last;
        } else if (!dateParts) {
            at = 1 + std::min(n - 1, int(std::floor((v - g->start) / g->interval)));
        } else if (gran >= 3) {
            int y = 0, m = 0, d = 0;
            if (!jp::calc::serialToDate(std::floor(v), dates, &y, &m, &d)) return fail(QStringLiteral("a value is not a date"));
            if (g->groupBy == QLatin1String("years")) at = 1 + (y - firstYear);
            else if (g->groupBy == QLatin1String("quarters")) at = 1 + (m - 1) / 3;
            else if (g->groupBy == QLatin1String("months")) at = m;
            else at = leapDay(m, d);
        } else {
            const double frac = v - std::floor(v);
            const double seconds = std::floor(frac * 86400 + 0.5);
            const int total = int(seconds) % 86400;
            if (g->groupBy == QLatin1String("hours")) at = 1 + total / 3600;
            else if (g->groupBy == QLatin1String("minutes")) at = 1 + (total / 60) % 60;
            else at = 1 + total % 60;
        }
        (*map)[i] = std::min(at, last);
    }
    return true;
}

bool pivotApplyGroup(PivotCache *cache, int field, jp::calc::DateSystem dates, QString *why)
{
    auto fail = [&](const QString &m) {
        if (why) *why = m;
        return false;
    };
    if (field < 0 || field >= cache->fields.size()) return fail(QStringLiteral("no such field"));
    PivotCacheField &f = cache->fields[field];
    if (f.group.kind == PivotFieldGroup::None) return true;
    const int b = f.group.base >= 0 ? f.group.base : field;
    if (b >= cache->fields.size()) return fail(QStringLiteral("a group is made from a field that is not there"));
    PivotCacheField &bf = cache->fields[b];
    if (b == field && !f.hasRaw) {
        f.hasRaw = true;
        f.rawItems = f.items;
        f.rawRecords = f.records;
        f.rawSharedCount = f.sharedCount;
        f.rawHasShared = f.hasShared;
        f.rawSharedAttrs = f.sharedAttrs;
    }
    const QVector<PivotValue> &rawItems = bf.hasRaw ? bf.rawItems : bf.items;
    const QVector<qint32> &rawRecords = bf.hasRaw ? bf.rawRecords : bf.records;
    QVector<qint32> map;
    if (f.group.kind == PivotFieldGroup::Discrete) {
        map = f.group.discrete;
        if (map.size() < rawItems.size()) return fail(QStringLiteral("a group does not name each item"));
        for (qint32 m : map)
            if (m < 0 || m >= f.group.labels.size()) return fail(QStringLiteral("a group names an item that is not there"));
    } else {
        QString problem;
        const QStringList wasLabels = f.group.labels;
        const double fileStart = f.group.start, fileEnd = f.group.end;
        const bool autoStart = f.group.autoStart, autoEnd = f.group.autoEnd;
        if (!pivotGroupItems(rawItems, &f.group, dates, &map, &problem)) return fail(problem);
        if (!wasLabels.isEmpty() && wasLabels.size() != f.group.labels.size()) {
            // (the bounds the file gives, when the data's own do not make as many groups as it has)
            f.group.start = fileStart;
            f.group.end = fileEnd;
            f.group.autoStart = f.group.autoEnd = false;
            if (!pivotGroupItems(rawItems, &f.group, dates, &map, &problem)) return fail(problem);
            f.group.autoStart = autoStart;
            f.group.autoEnd = autoEnd;
        }
        // a part of a file that names its group items keeps them (when there are as many as the grouping makes)
        if (!wasLabels.isEmpty() && wasLabels.size() == f.group.labels.size()) f.group.labels = wasLabels;
        else if (!wasLabels.isEmpty()) return fail(QStringLiteral("the groups of the file are not the ones JeffCrunch works out"));
    }
    f.items.clear();
    for (const QString &label : f.group.labels) {
        PivotValue v;
        v.kind = PivotValue::Text;
        v.text = label;
        f.items.append(v);
    }
    f.sharedCount = int(f.items.size());
    f.hasShared = true;
    f.records.resize(rawRecords.size());
    for (qsizetype r = 0; r < rawRecords.size(); ++r) {
        const qint32 at = rawRecords[r];
        const qint32 m = at >= 0 && at < map.size() ? map[at] : -1;
        if (m < 0) return fail(QStringLiteral("a value can't be put in a group"));
        f.records[r] = m;
    }
    f.numericKnown = -1;
    return true;
}

bool pivotGroupField(PivotCache *cache, int field, const QStringList &levels, double start, double end, double step, bool autoStart, bool autoEnd, jp::calc::DateSystem dates, QVector<int> *added, QString *why)
{
    auto fail = [&](const QString &m) {
        if (why) *why = m;
        return false;
    };
    added->clear();
    if (field < 0 || field >= cache->fields.size()) return fail(QStringLiteral("no such field"));
    PivotCacheField &f = cache->fields[field];
    if (f.group.kind != PivotFieldGroup::None) return fail(QStringLiteral("the field is already grouped"));
    for (const PivotCacheField &o : cache->fields)
        if (o.group.kind != PivotFieldGroup::None && o.group.base == field) return fail(QStringLiteral("the field is already grouped"));
    const PivotCache saved = *cache;
    auto undo = [&] { *cache = saved; };
    bool dateKind = false;
    for (const PivotValue &v : f.items) dateKind = dateKind || v.kind == PivotValue::Date;
    if (levels.isEmpty()) {
        f.group.kind = PivotFieldGroup::Range;
        f.group.groupBy = QStringLiteral("range");
        f.group.start = start;
        f.group.end = end;
        f.group.interval = step;
        f.group.autoStart = autoStart;
        f.group.autoEnd = autoEnd;
        f.group.base = field;
        QString problem;
        if (!pivotApplyGroup(cache, field, dates, &problem)) {
            undo();
            return fail(problem);
        }
        added->append(field);
        return true;
    }
    QStringList ordered;
    for (const char *by : {"seconds", "minutes", "hours", "days", "months", "quarters", "years"})
        if (levels.contains(QLatin1String(by))) ordered << QLatin1String(by);
    if (ordered.isEmpty()) return fail(QStringLiteral("choose what to group by"));
    // the finest level is the field's own; each coarser level is a field added at the end (named for the level)
    const int firstNew = int(cache->fields.size());
    if (firstNew + ordered.size() > kPivotMaxFields) return fail(QStringLiteral("too many fields"));
    for (int k = 0; k < ordered.size(); ++k) {
        PivotFieldGroup g;
        g.kind = PivotFieldGroup::Dates;
        g.groupBy = ordered[k];
        g.start = start;
        g.end = end;
        g.autoStart = autoStart;
        g.autoEnd = autoEnd;
        g.base = field;
        if (k == 0) {
            f.group = g;
            f.group.par = ordered.size() > 1 ? firstNew : -1;
            continue;
        }
        PivotCacheField nf;
        QString name = ordered[k];
        name[0] = name[0].toUpper();
        for (int n = 2;; ++n) {
            bool taken = false;
            for (const PivotCacheField &o : cache->fields) taken = taken || o.name.compare(name, Qt::CaseInsensitive) == 0;
            if (!taken) break;
            name = ordered[k].left(1).toUpper() + ordered[k].mid(1) + QString::number(n);
        }
        nf.name = name;
        g.databaseField = false;
        g.par = k + 1 < ordered.size() ? firstNew + k : -1;
        nf.group = g;
        cache->fields.append(nf);
        added->append(int(cache->fields.size()) - 1);
    }
    (void)dateKind;
    QString problem;
    if (!pivotApplyGroup(cache, field, dates, &problem)) {
        undo();
        added->clear();
        return fail(problem);
    }
    for (int a : *added)
        if (!pivotApplyGroup(cache, a, dates, &problem)) {
            undo();
            added->clear();
            return fail(problem);
        }
    added->prepend(field);
    pivotKeepNumericFlags(cache);
    return true;
}

bool pivotUngroupField(PivotCache *cache, int field, QVector<int> *removed)
{
    removed->clear();
    if (field < 0 || field >= cache->fields.size() || cache->fields[field].group.kind == PivotFieldGroup::None) return false;
    const int b = cache->fields[field].group.base >= 0 ? cache->fields[field].group.base : field;
    if (b >= cache->fields.size()) return false;
    for (int i = 0; i < cache->fields.size(); ++i) {
        const PivotCacheField &o = cache->fields[i];
        if (o.group.kind != PivotFieldGroup::None && !o.group.databaseField && o.group.base == b) removed->append(i);
    }
    PivotCacheField &bf = cache->fields[b];
    if (bf.hasRaw) {
        bf.items = bf.rawItems;
        bf.records = bf.rawRecords;
        bf.sharedCount = bf.rawSharedCount;
        bf.hasShared = bf.rawHasShared;
        bf.sharedAttrs = bf.rawSharedAttrs;
        bf.rawItems.clear();
        bf.rawRecords.clear();
        bf.hasRaw = false;
    }
    bf.group = PivotFieldGroup();
    bf.numericKnown = -1;
    std::sort(removed->begin(), removed->end(), std::greater<int>());
    for (int r : *removed) cache->fields.removeAt(r);
    // the places the other groups name move up
    for (PivotCacheField &o : cache->fields) {
        auto shift = [&](int &ref) {
            if (ref < 0) return;
            int less = 0;
            for (int r : *removed)
                if (r < ref) ++less;
            ref -= less;
        };
        shift(o.group.base);
        shift(o.group.par);
    }
    pivotKeepNumericFlags(cache);
    return true;
}

} // namespace jp::office::sheet

namespace jp::office::sheet {

void pivotRemoveFields(PivotTable *t, const QVector<int> &removed)
{
    QVector<int> gone(removed);
    std::sort(gone.begin(), gone.end(), std::greater<int>());
    auto newPlace = [&](int f) {
        if (f < 0) return f;
        int less = 0;
        for (int r : gone) {
            if (r == f) return -1;
            if (r < f) ++less;
        }
        return f - less;
    };
    for (int r : gone)
        if (r >= 0 && r < t->fields.size()) t->fields.removeAt(r);
    for (QVector<int> *list : {&t->rowFields, &t->colFields}) {
        QVector<int> kept;
        for (int f : *list) {
            const int n = newPlace(f);
            if (n != -1 || f == -2) kept.append(f == -2 ? -2 : n);
        }
        *list = kept;
    }
    QVector<PivotPageField> pages;
    for (PivotPageField p : t->pageFields) {
        const int n = newPlace(p.fld);
        if (n < 0) continue;
        p.fld = n;
        pages.append(p);
    }
    t->pageFields = pages;
    QVector<PivotDataField> data;
    for (PivotDataField d : t->dataFields) {
        const int n = newPlace(d.fld);
        if (n < 0) continue;
        d.fld = n;
        d.baseField = d.baseField >= 0 ? std::max(0, newPlace(d.baseField)) : 0;
        data.append(d);
    }
    t->dataFields = data;
}

} // namespace jp::office::sheet
