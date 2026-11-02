#include "sheet/pivot.h"

#include <algorithm>
#include <map>
#include <tuple>
#include <cmath>
#include <limits>
#include <string>
#include <unordered_map>

namespace jp::office::sheet {

namespace {

enum class Fn : quint8 { Sum, Count, CountNums, Average, Max, Min, Product, StdDev, StdDevP, Var, VarP, None };

Fn fnOf(const QString &s)
{
    if (s == QLatin1String("sum")) return Fn::Sum;
    if (s == QLatin1String("count") || s == QLatin1String("countA")) return Fn::Count;
    if (s == QLatin1String("countNums")) return Fn::CountNums;
    if (s == QLatin1String("average") || s == QLatin1String("avg")) return Fn::Average;
    if (s == QLatin1String("max")) return Fn::Max;
    if (s == QLatin1String("min")) return Fn::Min;
    if (s == QLatin1String("product")) return Fn::Product;
    if (s == QLatin1String("stdDev")) return Fn::StdDev;
    if (s == QLatin1String("stdDevp") || s == QLatin1String("stdDevP")) return Fn::StdDevP;
    if (s == QLatin1String("var")) return Fn::Var;
    if (s == QLatin1String("varp") || s == QLatin1String("varP")) return Fn::VarP;
    return Fn::None;
}

// What a subtotal line of an axis is called after its item.
QString fnWord(const QString &t)
{
    if (t == QLatin1String("sum")) return QStringLiteral("Sum");
    if (t == QLatin1String("countA")) return QStringLiteral("Count");
    if (t == QLatin1String("count")) return QStringLiteral("Count Nums");
    if (t == QLatin1String("avg")) return QStringLiteral("Average");
    if (t == QLatin1String("max")) return QStringLiteral("Max");
    if (t == QLatin1String("min")) return QStringLiteral("Min");
    if (t == QLatin1String("product")) return QStringLiteral("Product");
    if (t == QLatin1String("stdDev")) return QStringLiteral("StdDev");
    if (t == QLatin1String("stdDevP")) return QStringLiteral("StdDevp");
    if (t == QLatin1String("var")) return QStringLiteral("Var");
    if (t == QLatin1String("varP")) return QStringLiteral("Varp");
    return QStringLiteral("Total");
}

// A subtotal line named t works the values out with its own function (the subtotal named "count" counts numbers, "countA" everything).
Fn fnOfLine(const QString &t)
{
    if (t == QLatin1String("count")) return Fn::CountNums;
    return fnOf(t);
}

// A sum as the original program adds: a result that is only the rounding left over from two nearly equal numbers cancelling is zero.
static double addNumbers(double a, double b, double big)
{
    const double r = a + b;
    if (r != 0 && std::fabs(r) < 1e-15 * std::max(big, std::max(std::fabs(a), std::fabs(b)))) return 0;
    return r;
}

struct Acc {
    qint64 n = 0, nn = 0;   // values that are not missing, and those of them that are numbers
    double sum = 0, big = 0, mean = 0, m2 = 0, prod = 1;
    double mn = std::numeric_limits<double>::infinity(), mx = -std::numeric_limits<double>::infinity();
    QString error;
    double shown = 0;      // the sum of a group by itself: its values added from the largest to the smallest, which is how the original program shows it
    bool hasShown = false;
    void add(const PivotValue &v)
    {
        if (v.kind == PivotValue::Missing) return;
        ++n;
        if (v.kind == PivotValue::Error) {
            if (error.isEmpty()) error = v.text;
            return;
        }
        if (!v.isNumeric()) return;
        ++nn;
        const double x = v.number;
        sum = addNumbers(sum, x, big);
        big = std::max(big, std::max(std::fabs(x), std::fabs(sum)));
        prod *= x;
        const double delta = x - mean;
        mean += delta / double(nn);
        m2 += delta * (x - mean);
        mn = std::min(mn, x);
        mx = std::max(mx, x);
    }
    void merge(const Acc &o)
    {
        if (o.n == 0) return;
        hasShown = false;
        if (error.isEmpty()) error = o.error;
        if (o.nn > 0) {
            const double delta = o.mean - mean;
            const qint64 total = nn + o.nn;
            m2 += o.m2 + delta * delta * double(nn) * double(o.nn) / double(total);
            mean += delta * double(o.nn) / double(total);
            nn = total;
            sum = addNumbers(sum, o.sum, std::max(big, o.big));
            big = std::max(big, o.big);
            prod *= o.prod;
            mn = std::min(mn, o.mn);
            mx = std::max(mx, o.mx);
        }
        n += o.n;
    }
};

PivotCell result(const Acc &a, Fn fn, int numFmtId)
{
    PivotCell c;
    c.isValue = true;
    c.numFmtId = numFmtId;
    if (a.n == 0) return c;
    auto number = [&](double d) {
        c.kind = PivotCell::Number;
        c.number = d;
    };
    auto err = [&](const char *e) {
        c.kind = PivotCell::Error;
        c.text = QString::fromLatin1(e);
    };
    if (!a.error.isEmpty() && fn != Fn::Count) {
        c.kind = PivotCell::Error;
        c.text = a.error;
        return c;
    }
    switch (fn) {
    case Fn::Sum: number(a.hasShown ? a.shown : a.sum); break;
    case Fn::Count: number(double(a.n)); break;
    case Fn::CountNums: number(double(a.nn)); break;
    case Fn::Average: if (a.nn > 0) number((a.hasShown ? a.shown : a.sum) / double(a.nn)); else err("#DIV/0!"); break;
    case Fn::Max: number(a.nn > 0 ? a.mx : 0); break;
    case Fn::Min: number(a.nn > 0 ? a.mn : 0); break;
    case Fn::Product: number(a.nn > 0 ? a.prod : 0); break;
    case Fn::StdDev: if (a.nn > 1) number(std::sqrt(a.m2 / double(a.nn - 1))); else err("#DIV/0!"); break;
    case Fn::StdDevP: if (a.nn > 0) number(std::sqrt(a.m2 / double(a.nn))); else err("#DIV/0!"); break;
    case Fn::Var: if (a.nn > 1) number(a.m2 / double(a.nn - 1)); else err("#DIV/0!"); break;
    case Fn::VarP: if (a.nn > 0) number(a.m2 / double(a.nn)); else err("#DIV/0!"); break;
    case Fn::None: break;
    }
    return c;
}

using Tuple = std::vector<int>;

std::string keyOf(const Tuple &t)
{
    return std::string(reinterpret_cast<const char *>(t.data()), t.size() * sizeof(int));
}

// The lines of one axis, with the lines each leaf (a distinct combination of the axis's items that the records have) is a part of.
struct Axis {
    QVector<int> fields;       // the field of each level (-2: the values)
    int dSlot = -1;            // the level of the values, or -1
    std::vector<Tuple> leaves; // sorted
    QVector<PivotAxisItem> items;
    std::vector<std::vector<int>> members;   // for each leaf, the lines it is in
};

struct AxisOptions {
    std::vector<QStringList> subtotals;   // for each level, the lines it has after (or before) its group: "default", "sum", ...
    std::vector<bool> bottom, blankRow;   // for each level
    bool grand = true;
    int dataCount = 1;
    int form = 0;                         // 0 tabular, 1 outline, 2 compact
};

int lcp(const Tuple &a, const Tuple &b)
{
    size_t i = 0;
    while (i < a.size() && i < b.size() && a[i] == b[i]) ++i;
    return int(i);
}

// How many levels a leaf has (a leaf of a collapsed item has none of the levels below it: -1).
int depthOf(const Tuple &t)
{
    int d = int(t.size());
    while (d > 0 && t[d - 1] < 0) --d;
    return d;
}

bool buildAxis(Axis *ax, const AxisOptions &o, QString *why)
{
    const int nL = int(ax->fields.size());
    const int n = int(ax->leaves.size());
    ax->members.assign(n, {});
    if (nL == 0) {
        ax->items.append(PivotAxisItem());
        for (int k = 0; k < n; ++k) ax->members[k].push_back(0);
        return true;
    }
    int lastReal = -1, nReal = 0;
    for (int l = 0; l < nL; ++l)
        if (ax->fields[l] != -2) {
            lastReal = l;
            ++nReal;
        }
    const int D = std::max(1, o.dataCount);
    auto add = [&](const PivotAxisItem &it) {
        ax->items.append(it);
        return int(ax->items.size()) - 1;
    };
    auto grandLines = [&]() {
        if (!o.grand || nReal == 0) return;
        const int copies = ax->dSlot >= 0 ? D : 1;
        for (int d = 0; d < copies; ++d) {
            PivotAxisItem it;
            it.t = QStringLiteral("grand");
            it.x.append(0);
            it.i = ax->dSlot >= 0 ? d : 0;
            const int id = add(it);
            for (int k = 0; k < n; ++k)
                if (ax->dSlot < 0 || ax->leaves[k][ax->dSlot] == d) ax->members[k].push_back(id);
        }
    };
    if (n == 0) {
        grandLines();
        return true;
    }
    // a group of leaves [lo, hi) that share their first level+1 items
    auto subtotal = [&](int level, int lo, int hi) {
        for (const QString &t : o.subtotals[level]) {
            const int copies = (ax->dSlot > level) ? D : 1;
            for (int d = 0; d < copies; ++d) {
                PivotAxisItem it;
                it.t = t;
                it.r = level;
                it.x.append(ax->leaves[lo][level]);
                it.i = ax->dSlot >= 0 && ax->dSlot < level ? ax->leaves[lo][ax->dSlot] : (ax->dSlot > level ? d : 0);
                const int id = add(it);
                for (int k = lo; k < hi; ++k)
                    if (ax->dSlot <= level || ax->leaves[k][ax->dSlot] == d) ax->members[k].push_back(id);
            }
        }
    };
    // the line of an item that has its group under it (outline and compact forms): it shows the group's total when the subtotals are on top
    auto header = [&](int level, int lo, int hi) {
        const bool carries = !o.bottom[level] && o.subtotals[level].contains(QStringLiteral("default"));
        const int copies = (ax->dSlot > level) ? D : 1;
        for (int d = 0; d < copies; ++d) {
            PivotAxisItem it;
            it.r = level;
            it.x.append(ax->leaves[lo][level]);
            it.i = ax->dSlot >= 0 && ax->dSlot < level ? ax->leaves[lo][ax->dSlot] : (ax->dSlot > level ? d : 0);
            it.labelOnly = !carries;
            const int id = add(it);
            if (carries)
                for (int k = lo; k < hi; ++k)
                    if (ax->dSlot <= level || ax->leaves[k][ax->dSlot] == d) ax->members[k].push_back(id);
            if (copies > 1) break;   // (the line of the item is one, whatever the values)
        }
    };
    auto blank = [&](int level, int lo) {
        if (!o.blankRow[level]) return;
        PivotAxisItem it;
        it.t = QStringLiteral("blank");
        it.r = level;
        it.x.append(ax->leaves[lo][level]);
        if (ax->dSlot >= 0 && ax->dSlot < level) it.i = ax->leaves[lo][ax->dSlot];
        add(it);
    };
    std::vector<int> groupStart(nL, 0);
    Tuple prevPath;
    int prevDepth = 0;
    auto closeGroups = [&](int from, int downTo, int hi) {
        // the groups of levels from..downTo end at hi
        for (int l = from; l >= downTo; --l) {
            if (l < 0 || l >= nL - 1) continue;
            if (l < lastReal && ax->fields[l] != -2 && !o.subtotals[l].isEmpty() && o.bottom[l]) {
                subtotal(l, groupStart[l], hi);
                prevPath.assign(ax->leaves[groupStart[l]].begin(), ax->leaves[groupStart[l]].begin() + l + 1);
            }
            blank(l, groupStart[l]);
        }
    };
    for (int k = 0; k < n; ++k) {
        const Tuple &cur = ax->leaves[k];
        const int dp = depthOf(cur);
        const int common = k == 0 ? 0 : lcp(ax->leaves[k - 1], cur);
        if (k > 0) {
            if (prevDepth > 0 && prevDepth - 1 < lastReal && o.blankRow[prevDepth - 1] && ax->fields[prevDepth - 1] != -2) blank(prevDepth - 1, k - 1);
            closeGroups(prevDepth - 2, common, k);
        }
        for (int l = common; l <= dp - 2; ++l) {
            groupStart[l] = k;
            if (l >= lastReal || ax->fields[l] == -2) continue;
            int hi = k + 1;
            while (hi < n && lcp(ax->leaves[hi], cur) > l) ++hi;
            if (o.form != 0) {
                header(l, k, hi);
                prevPath.assign(cur.begin(), cur.begin() + l + 1);
            } else if (!o.bottom[l] && !o.subtotals[l].isEmpty()) {
                subtotal(l, k, hi);
                prevPath.assign(cur.begin(), cur.begin() + l + 1);
            }
        }
        PivotAxisItem it;
        const int r = std::min<int>(lcp(prevPath, cur), dp - 1);
        it.r = r;
        for (int l = r; l < dp; ++l) it.x.append(cur[l]);
        it.i = ax->dSlot >= 0 ? cur[ax->dSlot] : 0;
        const int id = add(it);
        ax->members[k].push_back(id);
        prevPath.assign(cur.begin(), cur.begin() + dp);
        prevDepth = dp;
        if (ax->items.size() > 4000000) {
            if (why) *why = QStringLiteral("too many lines");
            return false;
        }
    }
    if (prevDepth > 0 && prevDepth - 1 < lastReal && o.blankRow[prevDepth - 1] && ax->fields[prevDepth - 1] != -2) blank(prevDepth - 1, n - 1);
    closeGroups(prevDepth - 2, 0, n);
    grandLines();
    return true;
}

} // namespace

bool computePivot(const PivotTable &t, const PivotCache &c, PivotGrid *out, QString *why, const PivotOptions &options)
{
    *out = PivotGrid();
    auto fail = [&](const QString &m) {
        if (why) *why = m;
        return false;
    };
    if (!t.unsupported.isEmpty()) return fail(t.unsupported);
    if (!c.unsupported.isEmpty()) return fail(c.unsupported);
    const int nf = int(c.fields.size());
    if (t.fields.size() != nf) return fail(QStringLiteral("the table and the cache have different fields"));
    const int D = int(t.dataFields.size());
    for (const PivotDataField &d : t.dataFields) {
        if (d.fld < 0 || d.fld >= nf) return fail(QStringLiteral("a value field is not in the cache"));
        if (d.showDataAs != QLatin1String("normal")) return fail(QStringLiteral("a value shown as a percent, difference, or rank"));
        if (fnOf(d.subtotal) == Fn::None) return fail(QStringLiteral("a value function this does not work out"));
    }
    int dataSlotRow = -1, dataSlotCol = -1;
    for (int pass = 0; pass < 2; ++pass) {
        const QVector<int> &list = pass == 0 ? t.rowFields : t.colFields;
        for (int l = 0; l < list.size(); ++l) {
            const int f = list[l];
            if (f == -2) {
                if (dataSlotRow >= 0 || dataSlotCol >= 0) return fail(QStringLiteral("the values are twice on the axes"));
                (pass == 0 ? dataSlotRow : dataSlotCol) = l;
            } else if (f < 0 || f >= nf) {
                return fail(QStringLiteral("an axis names a field that is not in the cache"));
            }
        }
    }
    if (D > 1 && dataSlotRow < 0 && dataSlotCol < 0) return fail(QStringLiteral("several values and no place for them"));
    for (const PivotPageField &p : t.pageFields)
        if (p.fld < 0 || p.fld >= nf) return fail(QStringLiteral("a filter names a field that is not in the cache"));
    const qint64 nrec = nf > 0 && !c.fields.isEmpty() ? c.fields[0].records.size() : 0;
    for (const PivotCacheField &f : c.fields)
        if (f.records.size() != nrec) return fail(QStringLiteral("the cache records are uneven"));

    // the place of each shared item in its field's list of items (-1: hidden, or not listed)
    auto positions = [&](int f, QVector<qint32> *pos, QVector<const PivotTableItem *> *itemOfPos) {
        const PivotField &pf = t.fields[f];
        pos->assign(c.fields[f].items.size(), -1);
        if (!pf.hasItems) {
            for (int i = 0; i < pos->size(); ++i) (*pos)[i] = i;
            return;
        }
        int p = 0;
        for (const PivotTableItem &it : pf.items) {
            if (!it.t.isEmpty()) continue;
            if (it.x >= 0 && it.x < pos->size() && !it.hidden) (*pos)[it.x] = p;
            itemOfPos->append(&it);
            ++p;
        }
    };
    // the fields that filter: those on an axis or a page
    QVector<QVector<qint32>> posOf(nf);
    QVector<QVector<const PivotTableItem *>> itemOfPos(nf);   // for each field on an axis or a page, its items by place
    QVector<bool> used(nf, false);
    for (int f : t.rowFields) if (f >= 0) used[f] = true;
    for (int f : t.colFields) if (f >= 0) used[f] = true;
    for (const PivotPageField &p : t.pageFields) used[p.fld] = true;
    for (int f = 0; f < nf; ++f)
        if (used[f]) positions(f, &posOf[f], &itemOfPos[f]);

    // the records that are in
    std::vector<bool> in(size_t(nrec), true);
    for (const PivotPageField &p : t.pageFields) {
        const auto &recs = c.fields[p.fld].records;
        for (qint64 r = 0; r < nrec; ++r) {
            if (!in[size_t(r)]) continue;
            const int pos = posOf[p.fld][recs[r]];
            if (pos < 0 && t.fields[p.fld].hasItems) in[size_t(r)] = false;
            else if (p.item >= 0 && pos != p.item) in[size_t(r)] = false;
        }
    }

    // an item that is collapsed (its details are not shown) takes in the items under it
    auto collapsedAt = [&](int f, int pos) {
        const auto &list = itemOfPos[f];
        return pos >= 0 && pos < list.size() && !list[pos]->showDetails;
    };
    bool anyCollapsed = false;
    for (int f = 0; f < nf && !anyCollapsed; ++f)
        if (used[f])
            for (const PivotTableItem &it : t.fields[f].items) anyCollapsed |= it.t.isEmpty() && !it.showDetails;
    QHash<quint64, bool> collapsedCache;
    auto collapse = [&](const QVector<int> &fields, Tuple &tp, int dSlot) {
        if (!anyCollapsed) return;
        int lastReal = -1;
        for (int l = 0; l < fields.size(); ++l)
            if (fields[l] >= 0) lastReal = l;
        for (int l = 0; l < lastReal; ++l) {
            if (fields[l] < 0) continue;
            const quint64 key = (quint64(fields[l]) << 32) | quint32(tp[l]);
            auto h = collapsedCache.find(key);
            if (h == collapsedCache.end()) h = collapsedCache.insert(key, collapsedAt(fields[l], tp[l]));
            if (!h.value()) continue;
            if (dSlot > l) return;   // (the values are under it: it can't be collapsed)
            for (int m = l + 1; m < tp.size(); ++m) tp[m] = -1;
            return;
        }
    };

    Axis rowAx, colAx;
    rowAx.fields = t.rowFields;
    colAx.fields = t.colFields;
    rowAx.dSlot = dataSlotRow;
    colAx.dSlot = dataSlotCol;
    std::unordered_map<std::string, int> rowId, colId;
    std::vector<Tuple> rowTuples, colTuples;
    struct LeafKey {
        int row, col;
    };
    // what each record adds to, in the order of the records: its place on each axis, the record, and which value (the original program adds the values of a cell in the order of the records,
    // one at a time, so that the last digits of a total are those it has)
    struct Entry {
        int row, col;
        qint32 record;
        qint32 value;
    };
    std::vector<Entry> entries;
    // the fields sorted by a value: the value of each group of items is worked out in the order of the records, as the program that made the file does (ties and the last digits depend on it)
    struct SortSpec {
        std::vector<char> byValue, desc;
        std::vector<int> data;
        std::vector<std::unordered_map<std::string, Acc>> groups;
        bool any = false;
    };
    auto makeSpec = [&](const QVector<int> &fields) {
        SortSpec sp;
        const int nL = int(fields.size());
        sp.byValue.assign(nL, 0);
        sp.desc.assign(nL, 0);
        sp.data.assign(nL, -1);
        sp.groups.resize(nL);
        for (int l = 0; l < nL; ++l) {
            if (fields[l] < 0) continue;
            const PivotField &pf = t.fields[fields[l]];
            if (pf.sortByData >= 0 && pf.sortByData < D && (pf.sortType == QLatin1String("ascending") || pf.sortType == QLatin1String("descending"))) {
                sp.byValue[l] = 1;
                sp.desc[l] = pf.sortType == QLatin1String("descending");
                sp.data[l] = pf.sortByData;
                sp.any = true;
            }
        }
        return sp;
    };
    SortSpec rowSort = makeSpec(t.rowFields), colSort = makeSpec(t.colFields);
    const int dLoop = std::max(1, (dataSlotRow >= 0 || dataSlotCol >= 0) ? D : 1);
    Tuple rt(t.rowFields.size()), ct(t.colFields.size());
    for (qint64 r = 0; r < nrec; ++r) {
        if (!in[size_t(r)]) continue;
        bool ok = true;
        for (int l = 0; l < t.rowFields.size() && ok; ++l)
            if (t.rowFields[l] >= 0) {
                rt[l] = posOf[t.rowFields[l]][c.fields[t.rowFields[l]].records[r]];
                ok = rt[l] >= 0;
            }
        for (int l = 0; l < t.colFields.size() && ok; ++l)
            if (t.colFields[l] >= 0) {
                ct[l] = posOf[t.colFields[l]][c.fields[t.colFields[l]].records[r]];
                ok = ct[l] >= 0;
            }
        if (!ok) continue;
        collapse(t.rowFields, rt, dataSlotRow);
        collapse(t.colFields, ct, dataSlotCol);
        for (int d = 0; d < dLoop; ++d) {
            if (dataSlotRow >= 0) rt[dataSlotRow] = d;
            if (dataSlotCol >= 0) ct[dataSlotCol] = d;
            auto ri = rowId.emplace(keyOf(rt), int(rowTuples.size()));
            if (ri.second) rowTuples.push_back(rt);
            auto ci = colId.emplace(keyOf(ct), int(colTuples.size()));
            if (ci.second) colTuples.push_back(ct);
            // (more lines than a sheet has rows, or columns than it has, are refused before the lines are made)
            if (rowTuples.size() > size_t(kPivotMaxRows) || colTuples.size() > size_t(kPivotMaxCols)) return fail(QStringLiteral("the table would have too many lines"));
            entries.push_back({ri.first->second, ci.first->second, qint32(r), qint32(d)});
            if (D > 0) {
                const PivotValue &val = c.fields[t.dataFields[d].fld].items[c.fields[t.dataFields[d].fld].records[r]];
                for (int pass = 0; pass < 2; ++pass) {
                    SortSpec &sp = pass == 0 ? rowSort : colSort;
                    if (!sp.any) continue;
                    const Tuple &tp = pass == 0 ? rt : ct;
                    for (size_t l = 0; l < sp.byValue.size(); ++l)
                        if (sp.byValue[l] && sp.data[l] == d) sp.groups[l][keyOf(Tuple(tp.begin(), tp.begin() + l + 1))].add(val);
                }
            }
            if (qint64(entries.size()) > kPivotMaxLeaves) return fail(QStringLiteral("too many combinations"));
        }
    }

    // an axis that holds only the values has them whatever the records
    auto onlyValues = [&](const QVector<int> &fields, std::vector<Tuple> &tuples) {
        if (fields.size() == 1 && fields[0] == -2 && tuples.empty())
            for (int d = 0; d < std::max(D, 1); ++d) tuples.push_back(Tuple{d});
    };
    onlyValues(t.rowFields, rowTuples);
    onlyValues(t.colFields, colTuples);
    // the leaves in order: by the places of the items, and for a field sorted by a value, by that value of the items in their group
    auto sortLeaves = [&](std::vector<Tuple> &tuples, std::vector<int> *newOf, const QVector<int> &fields, SortSpec &sp) {
        const int nL = int(fields.size());
        std::vector<std::vector<double>> keys;
        const bool any = sp.any;
        const std::vector<char> &byValue = sp.byValue, &desc = sp.desc;
        if (any) {
            keys.assign(tuples.size(), std::vector<double>(nL, 0));
            for (int l = 0; l < nL; ++l) {
                if (!byValue[l]) continue;
                const Fn fn = fnOf(t.dataFields[sp.data[l]].subtotal);
                for (size_t i = 0; i < tuples.size(); ++i) {
                    const auto g = sp.groups[l].find(keyOf(Tuple(tuples[i].begin(), tuples[i].begin() + l + 1)));
                    double v = -std::numeric_limits<double>::infinity();
                    if (g != sp.groups[l].end()) {
                        const PivotCell pc = result(g->second, fn, 0);
                        if (pc.kind == PivotCell::Number) v = pc.number;
                    }
                    keys[i][l] = v;
                }
            }
        }
        std::vector<int> order(tuples.size());
        for (size_t i = 0; i < order.size(); ++i) order[i] = int(i);
        std::sort(order.begin(), order.end(), [&](int a, int b) {
            for (int l = 0; l < nL; ++l) {
                if (any && byValue[l] && keys[a][l] != keys[b][l]) return desc[l] ? keys[a][l] > keys[b][l] : keys[a][l] < keys[b][l];
                if (tuples[a][l] != tuples[b][l]) return tuples[a][l] < tuples[b][l];
            }
            return false;
        });
        newOf->assign(tuples.size(), 0);
        std::vector<Tuple> sorted;
        sorted.reserve(tuples.size());
        for (size_t i = 0; i < order.size(); ++i) {
            (*newOf)[order[i]] = int(i);
            sorted.push_back(std::move(tuples[order[i]]));
        }
        tuples.swap(sorted);
    };
    std::vector<int> rowNew, colNew;
    sortLeaves(rowTuples, &rowNew, t.rowFields, rowSort);
    sortLeaves(colTuples, &colNew, t.colFields, colSort);
    rowAx.leaves = std::move(rowTuples);
    colAx.leaves = std::move(colTuples);

    // the lines
    auto formOf = [&](const QVector<int> &fields) {
        for (int f : fields)
            if (f >= 0) return t.fields[f].compact ? 2 : (t.fields[f].outline ? 1 : 0);
        return 0;
    };
    auto optionsFor = [&](const QVector<int> &fields, bool grand, int form) {
        AxisOptions o;
        o.grand = grand;
        o.dataCount = D;
        o.form = form;
        for (int f : fields) {
            QStringList subs;
            bool bottom = true, blankRow = false;
            if (f >= 0) {
                const PivotField &pf = t.fields[f];
                if (pf.hasItems) {
                    for (const PivotTableItem &it : pf.items)
                        if (!it.t.isEmpty()) subs << it.t;
                } else {
                    if (pf.defaultSubtotal) subs << QStringLiteral("default");
                    subs << pf.subtotals;
                }
                bottom = form == 0 || !pf.subtotalTop;
                blankRow = pf.insertBlankRow;
            }
            o.subtotals.push_back(subs);
            o.bottom.push_back(bottom);
            o.blankRow.push_back(blankRow);
        }
        return o;
    };
    const int rowForm = formOf(t.rowFields);
    QString err;
    if (!buildAxis(&rowAx, optionsFor(t.rowFields, t.rowGrandTotals, rowForm), &err)) return fail(err);
    if (!buildAxis(&colAx, optionsFor(t.colFields, t.colGrandTotals, 0), &err)) return fail(err);
    const int nRI = int(rowAx.items.size()), nCI = int(colAx.items.size());
    if (qint64(nRI) * nCI > kPivotMaxOutputCells) return fail(QStringLiteral("the table would have too many cells"));

    // the values
    std::vector<Acc> acc(size_t(nRI) * size_t(nCI));
    if (D > 0) {
        // each group of records (a line and a column, and a value) is added in the order of the records; a total is then the sum of those sums, group by group, as the original program has it
        struct Leaf {
            int row, col, value;
            std::vector<double> numbers;
            Acc acc;
        };
        std::map<std::tuple<int, int, int>, size_t> where;
        std::vector<Leaf> leaves;
        for (const Entry &e : entries) {
            const PivotValue &val = c.fields[t.dataFields[e.value].fld].items[c.fields[t.dataFields[e.value].fld].records[e.record]];
            const auto key = std::make_tuple(rowNew[e.row], colNew[e.col], int(e.value));
            const auto it = where.find(key);
            size_t at;
            if (it == where.end()) {
                at = leaves.size();
                where.emplace(key, at);
                leaves.push_back({rowNew[e.row], colNew[e.col], int(e.value), {}, Acc()});
            } else {
                at = it->second;
            }
            leaves[at].acc.add(val);
            if (val.isNumeric()) leaves[at].numbers.push_back(val.number);
        }
        for (Leaf &l : leaves) {
            std::stable_sort(l.numbers.begin(), l.numbers.end(), [](double a, double b) { return std::fabs(a) > std::fabs(b); });
            double sum = 0, big = 0;
            for (double x : l.numbers) {
                sum = addNumbers(sum, x, big);
                big = std::max(big, std::max(std::fabs(x), std::fabs(sum)));
            }
            l.acc.shown = sum;
            l.acc.hasShown = true;
            l.numbers.clear();
            l.numbers.shrink_to_fit();
        }
        std::sort(leaves.begin(), leaves.end(), [](const Leaf &a, const Leaf &b) { return std::tie(a.row, a.col, a.value) < std::tie(b.row, b.col, b.value); });
        for (const Leaf &l : leaves)
            for (int ri : rowAx.members[l.row])
                for (int ci : colAx.members[l.col]) {
                    Acc &a = acc[size_t(ri) * nCI + ci];
                    if (a.n == 0 && a.error.isEmpty()) a = l.acc;
                    else a.merge(l.acc);
                }
    }

    // the grid
    out->rowItems = rowAx.items;
    out->colItems = colAx.items;
    const int nR = int(t.rowFields.size()), nC = int(t.colFields.size());
    int realC = 0;
    for (int f : t.colFields) realC += f >= 0;
    const bool compact = rowForm == 2;
    const bool tabular = rowForm == 0;
    const int labelCols = compact ? 1 : std::max(1, nR);
    const bool lone = nC == 0 && dataSlotRow < 0;   // no column field and one value
    const bool bare = lone && nR == 0;               // and no row field: the value's name, and one line "Total" with the value
    // (a table with the classic grid has the line of the value's name over the others in every form, the tabular one's own and the others' too)
    const int captionRow = bare ? 1 : (t.autoFormatId == 4097 ? 0 :(tabular ? ((nC > 0 || lone) ? 1 : 0) : ((realC > 0 || ((lone || nC > 0) && t.gridDropZones)) ? 1 : 0)));
    const int itemRows = bare ? 0 : std::max(nC, 1);
    const int headerRows = captionRow + itemRows;
    const int lastItemsRow = headerRows - 1;
    out->firstDataRow = headerRows;
    out->firstDataCol = labelCols;
    out->firstHeaderRow = nC == 0 ? headerRows : captionRow;
    out->rows = headerRows + nRI;
    out->cols = std::max(labelCols + nCI, (tabular && captionRow > 0) ? labelCols + nC : 0);
    if (qint64(out->rows) * out->cols > kPivotMaxOutputCells) return fail(QStringLiteral("the table would have too many cells"));
    out->cells.resize(qsizetype(out->rows) * out->cols);

    auto caption = [&](int f) { return t.fields[f].name.isEmpty() ? c.fields[f].name : t.fields[f].name; };
    auto text = [&](const QString &s) {
        PivotCell pc;
        pc.kind = PivotCell::Text;
        pc.text = s;
        return pc;
    };
    auto valueCell = [&](const PivotValue &v, int numFmt) {
        PivotCell pc;
        pc.numFmtId = numFmt;
        switch (v.kind) {
        case PivotValue::Missing: pc.kind = PivotCell::Text; pc.text = QStringLiteral("(blank)"); break;
        case PivotValue::Number:
        case PivotValue::Date: pc.kind = PivotCell::Number; pc.number = v.number; break;
        case PivotValue::Bool: pc.kind = PivotCell::Bool; pc.number = v.number; break;
        case PivotValue::Text: pc.kind = v.text.isEmpty() ? PivotCell::Empty : PivotCell::Text; pc.text = v.text; break;
        case PivotValue::Error: pc.kind = PivotCell::Error; pc.text = v.text; break;
        }
        return pc;
    };
    // an item's label as text (the "7/1/2001" of "7/1/2001 Total"): a number as the cell it is in shows it
    auto plain = [&](const PivotCell &lab, int numFmt, int row, int col) {
        if (lab.kind == PivotCell::Text) return lab.text;
        if (lab.kind == PivotCell::Empty) return QString();
        if (options.format) return options.format(lab.number, numFmt, row, col);
        return QString::number(lab.number, 'g', 15);
    };
    // the label of the item at a level of an axis
    auto itemLabel = [&](const QVector<int> &fields, int level, int pos) -> PivotCell {
        const int f = fields[level];
        if (f == -2) return text(pos >= 0 && pos < D ? t.dataFields[pos].name : QString());
        const PivotField &pf = t.fields[f];
        int shared = -1;
        if (pf.hasItems) {
            const auto &list = itemOfPos[f];
            if (pos >= 0 && pos < list.size()) {
                shared = list[pos]->x;
                if (!list[pos]->n.isEmpty()) return text(list[pos]->n);
            }
        } else {
            shared = pos;
        }
        if (shared < 0 || shared >= c.fields[f].items.size()) return text(QString());
        return valueCell(c.fields[f].items[shared], pf.numFmtId);
    };
    auto set = [&](int r, int col, const PivotCell &pc) {
        if (r >= 0 && r < out->rows && col >= 0 && col < out->cols) out->at(r, col) = pc;
    };
    auto dataName = [&](int i) { return i >= 0 && i < D ? t.dataFields[i].name : QString(); };
    auto fieldFmt = [&](const QVector<int> &fields, int level) { return fields[level] >= 0 ? t.fields[fields[level]].numFmtId : 0; };

    // the rows' labels
    for (int ri = 0; ri < nRI; ++ri) {
        const PivotAxisItem &it = rowAx.items[ri];
        const int row = headerRows + ri;
        if (it.t == QLatin1String("grand")) {
            QString s = t.grandTotalCaption;
            if (rowAx.dSlot >= 0 && D > 1) s = QStringLiteral("Total ") + dataName(it.i);
            set(row, 0, text(s));
            continue;
        }
        if (it.t == QLatin1String("blank")) continue;
        if (nR == 0) {
            set(row, 0, text(QStringLiteral("Total")));
            continue;
        }
        for (int k = 0; k < it.x.size(); ++k) {
            const int level = it.r + k;
            if (level >= nR) break;
            if (compact && k != it.x.size() - 1) continue;
            PivotCell lab = itemLabel(t.rowFields, level, it.x[k]);
            if (!it.t.isEmpty() && (lab.kind == PivotCell::Text || lab.kind == PivotCell::Number || lab.kind == PivotCell::Empty))
                lab = text(plain(lab, fieldFmt(t.rowFields, level), row, compact ? 0 : level) + QLatin1Char(' ') + fnWord(it.t));
            set(row, compact ? 0 : level, lab);
        }
    }
    // the columns' labels
    for (int ci = 0; ci < nCI; ++ci) {
        const PivotAxisItem &it = colAx.items[ci];
        const int col = labelCols + ci;
        if (it.t == QLatin1String("grand")) {
            QString s = t.grandTotalCaption;
            if (colAx.dSlot >= 0 && D > 1) s = QStringLiteral("Total ") + dataName(it.i);
            set(captionRow, col, text(s));
            continue;
        }
        if (it.t == QLatin1String("blank")) continue;
        for (int k = 0; k < it.x.size(); ++k) {
            const int level = it.r + k;
            if (level >= nC) break;
            PivotCell lab = itemLabel(t.colFields, level, it.x[k]);
            if (!it.t.isEmpty() && (lab.kind == PivotCell::Text || lab.kind == PivotCell::Number || lab.kind == PivotCell::Empty))
                lab = text(plain(lab, fieldFmt(t.colFields, level), captionRow + level, col) + QLatin1Char(' ') + fnWord(it.t));
            set(captionRow + level, col, lab);
        }
    }
    // the corner: captions
    auto rowCaption = [&](int level) -> QString {
        const int f = t.rowFields[level];
        if (f == -2) return t.dataCaption;
        return caption(f);
    };
    auto rowLabels = [&](int row) {
        if (compact) {
            if (nR > 0) set(row, 0, text(t.rowHeaderCaption.isEmpty() ? QStringLiteral("Row Labels") : t.rowHeaderCaption));
            return;
        }
        for (int l = 0; l < nR; ++l) set(row, l, text(rowCaption(l)));
    };
    const bool oneValue = D == 1 && dataSlotRow < 0 && dataSlotCol < 0;
    if (captionRow > 0) {
        if (oneValue) set(0, 0, text(t.dataFields[0].name));
        if (bare) set(0, labelCols, text(QStringLiteral("Total")));   // (with the line "Total" under it, which holds the value)
        if (compact) {
            if (realC > 0) set(0, labelCols, text(t.colHeaderCaption.isEmpty() ? QStringLiteral("Column Labels") : t.colHeaderCaption));
            else if (nC > 0 && t.colFields[0] == -2) set(0, labelCols, text(t.dataCaption));   // (the values across, with nothing else: their caption)
        } else {
            for (int l = 0; l < nC; ++l) {
                const int f = t.colFields[l];
                set(0, labelCols + l, text(f == -2 ? t.dataCaption : caption(f)));
            }
        }
    }
    if (!bare) rowLabels(lastItemsRow);
    if (nC == 0 && !bare) {
        if (tabular || captionRow > 0 || !oneValue) set(lastItemsRow, labelCols, text(QStringLiteral("Total")));
        else set(lastItemsRow, labelCols, text(t.dataFields[0].name));
    }
    // the values
    for (int ri = 0; ri < nRI; ++ri) {
        const PivotAxisItem &rit = rowAx.items[ri];
        if (rit.labelOnly) continue;
        for (int ci = 0; ci < nCI; ++ci) {
            const PivotAxisItem &cit = colAx.items[ci];
            if (rit.t == QLatin1String("blank") || cit.t == QLatin1String("blank") || cit.labelOnly) continue;
            const int di = rowAx.dSlot >= 0 ? rit.i : (colAx.dSlot >= 0 ? cit.i : 0);
            if (D == 0) continue;
            Fn fn = fnOf(t.dataFields[di].subtotal);
            const QString &lt = !rit.t.isEmpty() && rit.t != QLatin1String("default") && rit.t != QLatin1String("grand") ? rit.t : cit.t;
            if (!lt.isEmpty() && lt != QLatin1String("default") && lt != QLatin1String("grand")) fn = fnOfLine(lt);
            PivotCell pc = result(acc[size_t(ri) * nCI + ci], fn, t.dataFields[di].numFmtId);
            if (pc.kind == PivotCell::Empty && !t.showMissing && (fn == Fn::Sum || fn == Fn::Count || fn == Fn::CountNums)) {
                pc.kind = PivotCell::Number;
                pc.number = 0;
            }
            if (pc.kind == PivotCell::Empty && t.showMissing && !t.missingCaption.isEmpty()) {
                pc.kind = PivotCell::Text;
                pc.text = t.missingCaption;
            }
            if (pc.kind == PivotCell::Error && t.showError && !t.errorCaption.isEmpty()) {
                pc.kind = PivotCell::Text;
                pc.text = t.errorCaption;
            }
            set(headerRows + ri, labelCols + ci, pc);
        }
    }
    // the filters, over the table: a field's name and its item
    for (const PivotPageField &p : t.pageFields) {
        out->pageLabels.append(text(caption(p.fld)));
        const PivotField &pf = t.fields[p.fld];
        bool anyHidden = false;
        for (const PivotTableItem &it : pf.items) anyHidden = anyHidden || (it.t.isEmpty() && it.hidden && !it.missing);
        if (p.item >= 0) out->pageLabels.append(itemLabel(QVector<int>{p.fld}, 0, p.item));
        else out->pageLabels.append(text(anyHidden ? QStringLiteral("(Multiple Items)") : QStringLiteral("(All)")));
    }
    // a table with nothing in it: the empty area the original program keeps for it
    if (t.rowFields.isEmpty() && t.colFields.isEmpty() && t.dataFields.isEmpty()) {
        out->rows = 18;
        out->cols = 3;
        out->cells.fill(PivotCell(), qsizetype(out->rows) * out->cols);
        out->firstHeaderRow = out->firstDataRow = out->firstDataCol = 1;
        out->rowItems.clear();
        out->colItems.clear();
    }
    return true;
}

} // namespace jp::office::sheet
