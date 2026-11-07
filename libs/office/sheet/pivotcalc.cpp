#include "sheet/pivot.h"
#include "sheet/pivotformula.h"

#include <algorithm>
#include <functional>
#include <memory>
#include <map>
#include <tuple>
#include <cmath>
#include <limits>
#include <QHash>
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
    case Fn::Sum: number(a.sum); break;
    case Fn::Count: number(double(a.n)); break;
    case Fn::CountNums: number(double(a.nn)); break;
    case Fn::Average: if (a.nn > 0) number(a.sum / double(a.nn)); else err("#DIV/0!"); break;
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
    // the work: the lines made, and the places leaves are put in them, are counted as they are made (a bound inside each step, not after the leaves)
    bool tooMuch = false;
    qint64 memberTotal = 0;
    auto charge = [&](qint64 n) {
        memberTotal += n;
        if (memberTotal > kPivotMaxFanOut || ax->items.size() > 4000000) tooMuch = true;
        return !tooMuch;
    };
    auto add = [&](const PivotAxisItem &it) {
        ax->items.append(it);
        if (ax->items.size() > 4000000) tooMuch = true;
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
            if (!charge(n)) return;
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
            for (int d = 0; d < copies && !tooMuch; ++d) {
                PivotAxisItem it;
                it.t = t;
                it.r = level;
                it.x.append(ax->leaves[lo][level]);
                it.i = ax->dSlot >= 0 && ax->dSlot < level ? ax->leaves[lo][ax->dSlot] : (ax->dSlot > level ? d : 0);
                const int id = add(it);
                if (!charge(hi - lo)) return;
                for (int k = lo; k < hi; ++k)
                    if (ax->dSlot <= level || ax->leaves[k][ax->dSlot] == d) ax->members[k].push_back(id);
            }
        }
    };
    // the line of an item that has its group under it (outline and compact forms): it shows the group's total when the subtotals are on top
    auto header = [&](int level, int lo, int hi) {
        const bool carries = !o.bottom[level] && o.subtotals[level].contains(QStringLiteral("default"));
        const int copies = (ax->dSlot > level) ? D : 1;
        for (int d = 0; d < copies && !tooMuch; ++d) {
            PivotAxisItem it;
            it.r = level;
            it.x.append(ax->leaves[lo][level]);
            it.i = ax->dSlot >= 0 && ax->dSlot < level ? ax->leaves[lo][ax->dSlot] : (ax->dSlot > level ? d : 0);
            it.labelOnly = !carries;
            const int id = add(it);
            if (carries && !charge(hi - lo)) return;
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
        for (int l = from; l >= downTo && !tooMuch; --l) {
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
        if (tooMuch) {
            if (why) *why = QStringLiteral("too many lines");
            return false;
        }
    }
    if (tooMuch) {
        if (why) *why = QStringLiteral("too many lines");
        return false;
    }
    if (prevDepth > 0 && prevDepth - 1 < lastReal && o.blankRow[prevDepth - 1] && ax->fields[prevDepth - 1] != -2) blank(prevDepth - 1, n - 1);
    closeGroups(prevDepth - 2, 0, n);
    grandLines();
    if (tooMuch) {
        if (why) *why = QStringLiteral("too many lines");
        return false;
    }
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
        static const QSet<QString> kShown = {QStringLiteral("rankAscending"), QStringLiteral("rankDescending"), QStringLiteral("percentOfParentRow"), QStringLiteral("percentOfParentCol"), QStringLiteral("normal"), QStringLiteral("difference"), QStringLiteral("percent"), QStringLiteral("percentDiff"), QStringLiteral("runTotal"),
                                             QStringLiteral("percentOfRow"), QStringLiteral("percentOfCol"), QStringLiteral("percentOfTotal"), QStringLiteral("index")};
        if (!kShown.contains(d.showDataAs)) return fail(QStringLiteral("a value shown as something this does not work out (") + d.showDataAs + QLatin1Char(')'));
        if (fnOf(d.subtotal) == Fn::None) return fail(QStringLiteral("a value function this does not work out"));
        if (c.fields[d.fld].isCalculated() && d.showDataAs != QLatin1String("normal")) return fail(QStringLiteral("a calculated field shown as something this does not work out"));
    }
    // the calculated fields the values use: each formula is read once, and the fields whose totals it needs are found (those of other calculated fields it uses too)
    std::vector<std::shared_ptr<const PivotFormula>> formulaOf;
    formulaOf.resize(size_t(nf));
    QHash<QString, int> fieldByName;
    for (int f = nf - 1; f >= 0; --f) fieldByName.insert(c.fields[f].name.toLower(), f);
    QVector<QVector<int>> refsOf(nf);   // for a calculated field, the fields with records its formula needs the totals of
    {
        std::function<bool(int, int)> read = [&](int f, int depth) {
            if (formulaOf[size_t(f)]) return true;
            if (depth > 16) return false;
            QString problem;
            formulaOf[size_t(f)] = PivotFormula::parse(c.fields[f].formula, &problem);
            if (!formulaOf[size_t(f)]) return false;
            for (const QString &name : formulaOf[size_t(f)]->names()) {
                const int g = fieldByName.value(name.toLower(), -1);
                if (g < 0 || g == f) continue;
                if (c.fields[g].isCalculated()) {
                    if (!read(g, depth + 1)) return false;
                    for (int x : refsOf[g]) if (!refsOf[f].contains(x)) refsOf[f].append(x);
                } else if (!refsOf[f].contains(g)) {
                    refsOf[f].append(g);
                }
            }
            return true;
        };
        for (const PivotDataField &d : t.dataFields)
            if (c.fields[d.fld].isCalculated() && !read(d.fld, 0)) return fail(QStringLiteral("the formula of a calculated field can't be worked out"));
    }
    QVector<int> exFields, exIndex(nf, -1);   // the fields whose totals the formulas need, and each one's place among them
    for (const PivotDataField &d : t.dataFields)
        if (c.fields[d.fld].isCalculated())
            for (int g : refsOf[d.fld])
                if (exIndex[g] < 0) {
                    exIndex[g] = int(exFields.size());
                    exFields.append(g);
                }
    const int nEx = int(exFields.size());
    for (int f = 0; f < nf; ++f)
        if (c.fields[f].isCalculated()) {
            if (t.rowFields.contains(f) || t.colFields.contains(f)) return fail(QStringLiteral("a calculated field is in the rows or the columns"));
            for (const PivotPageField &p : t.pageFields)
                if (p.fld == f) return fail(QStringLiteral("a calculated field is a filter"));
        }
    for (const PivotField &pf : t.fields)
        if (pf.sortByData >= 0 && pf.sortByData < D && c.fields[t.dataFields[pf.sortByData].fld].isCalculated()) return fail(QStringLiteral("a field sorted by a calculated field"));
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
    if (t.rowFields.size() > kPivotMaxLevels || t.colFields.size() > kPivotMaxLevels) return fail(QStringLiteral("more fields on an axis than this works out"));
    if (D > 1 && dataSlotRow < 0 && dataSlotCol < 0) return fail(QStringLiteral("several values and no place for them"));
    for (const PivotPageField &p : t.pageFields)
        if (p.fld < 0 || p.fld >= nf) return fail(QStringLiteral("a filter names a field that is not in the cache"));
    int firstReal = -1;
    for (int f = 0; f < nf && firstReal < 0; ++f)
        if (!c.fields[f].isCalculated()) firstReal = f;
    const qint64 nrec = firstReal >= 0 ? c.fields[firstReal].records.size() : 0;
    for (const PivotCacheField &f : c.fields)
        if (!f.isCalculated() && f.records.size() != nrec) return fail(QStringLiteral("the cache records are uneven"));

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

    // the calculated items of fields on an axis: each one's item, the items its formula names, and its formula, read once
    struct CalcInfo {
        int field = -1, shared = -1, order = 1;
        std::shared_ptr<const PivotFormula> formula;
        QStringList names;
        QVector<qint32> refShared;   // for each name, the place of the item in the field's shared items (-1: none of that name)
    };
    std::vector<CalcInfo> calcs;
    QHash<quint64, int> calcOfItem;   // (field, shared item) to its place in `calcs`
    {
        auto label = [](const PivotValue &v) {
            switch (v.kind) {
            case PivotValue::Text: return v.text;
            case PivotValue::Number: return QString::number(v.number, 'g', 15);
            default: return QString();
            }
        };
        for (const PivotCalcItem &ci : c.calcItems) {
            CalcInfo info;
            for (int f = 0; f < nf && info.field < 0; ++f)
                if (!c.fields[f].isCalculated() && c.fields[f].name.compare(ci.field, Qt::CaseInsensitive) == 0) info.field = f;
            if (info.field < 0 || !used[info.field]) continue;
            const auto &items = c.fields[info.field].items;
            for (int i = 0; i < items.size() && info.shared < 0; ++i)
                if (items[i].kind == PivotValue::Text && items[i].text.compare(ci.name, Qt::CaseInsensitive) == 0) info.shared = i;
            if (info.shared < 0 || calcOfItem.contains((quint64(info.field) << 32) | quint32(info.shared))) continue;
            info.order = ci.solveOrder;
            info.formula = PivotFormula::parse(ci.formula, nullptr);
            if (info.formula) {
                info.names = info.formula->names();
                for (const QString &n : info.names) {
                    qint32 at = -1;
                    for (int i = 0; i < items.size() && at < 0; ++i)
                        if (label(items[i]).compare(n, Qt::CaseInsensitive) == 0) at = i;
                    info.refShared.append(at);
                }
            }
            calcOfItem.insert((quint64(info.field) << 32) | quint32(info.shared), int(calcs.size()));
            calcs.push_back(std::move(info));
        }
    }

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
            if (qint64(rowTuples.size()) * (rt.size() + 1) > kPivotMaxLevelCells || qint64(colTuples.size()) * (ct.size() + 1) > kPivotMaxLevelCells) return fail(QStringLiteral("the table would have too many lines"));
            entries.push_back({ri.first->second, ci.first->second, qint32(r), qint32(d)});
            if (D > 0 && (rowSort.any || colSort.any)) {
                const PivotCacheField &dataField = c.fields[t.dataFields[d].fld];
                const PivotValue &val = dataField.storedItems()[dataField.storedRecords()[r]];   // (a field that is grouped is added up by its own values)
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
    // a calculated item has no records, so its lines are made from the others: wherever an item of its field has a line, the calculated item has one too (a table with two calculated items
    // across fields has a line for each pair)
    auto injectCalculated = [&](const QVector<int> &fields, std::vector<Tuple> &tuples, std::unordered_map<std::string, int> &ids, bool isRows) -> bool {
        if (calcs.empty()) return true;
        struct Level {
            int level;
            std::vector<int> own;       // the places of the calculated items of the level's field
            std::vector<char> isOwn;    // by place
        };
        std::vector<Level> levels;
        for (int l = 0; l < fields.size(); ++l) {
            if (fields[l] < 0) continue;
            Level lv;
            lv.level = l;
            lv.isOwn.assign(posOf[fields[l]].size() + 1, 0);
            for (const CalcInfo &ci : calcs) {
                const int own = ci.field == fields[l] ? posOf[ci.field][ci.shared] : -1;
                if (own < 0) continue;
                lv.own.push_back(own);
                if (size_t(own) < lv.isOwn.size()) lv.isOwn[size_t(own)] = 1;
            }
            if (!lv.own.empty()) levels.push_back(std::move(lv));
        }
        for (size_t i = 0; i < tuples.size(); ++i)
            for (const Level &lv : levels) {
                const int p = tuples[i][size_t(lv.level)];
                if (p < 0 || (size_t(p) < lv.isOwn.size() && lv.isOwn[size_t(p)])) continue;
                for (int own : lv.own) {
                    Tuple made = tuples[i];
                    made[size_t(lv.level)] = own;
                    if (!ids.emplace(keyOf(made), int(tuples.size())).second) continue;
                    tuples.push_back(std::move(made));
                    if (tuples.size() > size_t(isRows ? kPivotMaxRows : kPivotMaxCols) || qint64(tuples.size()) * (qint64(fields.size()) + 1) > kPivotMaxLevelCells) return false;
                }
            }
        return true;
    };
    if (!injectCalculated(t.rowFields, rowTuples, rowId, true) || !injectCalculated(t.colFields, colTuples, colId, false)) return fail(QStringLiteral("the table would have too many lines"));
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
            // (one line of each function, and no more than there are functions: a part that lists a million of them gets the first ones)
            QStringList kinds;
            for (const QString &k : subs)
                if (!kinds.contains(k) && kinds.size() < kPivotMaxSubtotalKinds) kinds << k;
            o.subtotals.push_back(kinds);
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
    if (qint64(nRI) * nCI * nEx > 16000000) return fail(QStringLiteral("the table would take too much work to total"));
    std::vector<Acc> exCell(size_t(nRI) * size_t(nCI) * size_t(nEx));   // (calculated fields) for each cell, the totals of the fields the formulas use
    bool anyShownAs = false;
    for (const PivotDataField &d : t.dataFields) anyShownAs |= d.showDataAs != QLatin1String("normal");
    // (for a value shown as a share of its line, its column, or the whole: those totals, which no line of the table need hold)
    std::vector<Acc> rowTot, colTot, grandTot;
    if (anyShownAs) {
        rowTot.resize(size_t(nRI) * size_t(D));
        colTot.resize(size_t(nCI) * size_t(D));
        grandTot.resize(size_t(D));
    }
    if (D > 0) {
        // each group of records (a line and a column, and a value) is added in the order of the records; a total is then the sum of those sums, group by group, as the original program has it
        struct Leaf {
            int row, col, value;
            Acc acc;
            std::vector<Acc> ex;   // (a calculated value) the totals of the fields its formula uses
        };
        std::map<std::tuple<int, int, int>, size_t> where;
        std::vector<Leaf> leaves;
        for (const Entry &e : entries) {
            const PivotCacheField &dataField = c.fields[t.dataFields[e.value].fld];
            const bool calc = dataField.isCalculated();
            PivotValue marker;
            marker.kind = PivotValue::Number;
            const PivotValue &val = calc ? marker : dataField.storedItems()[dataField.storedRecords()[e.record]];
            const auto key = std::make_tuple(rowNew[e.row], colNew[e.col], int(e.value));
            const auto it = where.find(key);
            size_t at;
            if (it == where.end()) {
                at = leaves.size();
                where.emplace(key, at);
                leaves.push_back({rowNew[e.row], colNew[e.col], int(e.value), Acc(), {}});
                if (calc) leaves.back().ex.resize(size_t(nEx));
            } else {
                at = it->second;
            }
            leaves[at].acc.add(val);
            if (calc)
                for (int g : refsOf[t.dataFields[e.value].fld]) {
                    const PivotCacheField &rf = c.fields[g];
                    leaves[at].ex[size_t(exIndex[g])].add(rf.storedItems()[rf.storedRecords()[e.record]]);
                }
        }
        std::sort(leaves.begin(), leaves.end(), [](const Leaf &a, const Leaf &b) { return std::tie(a.row, a.col, a.value) < std::tie(b.row, b.col, b.value); });
        // (each group goes to the lines it is a part of on both axes: the work is the groups times the lines of each, which is bounded before it is done)
        qint64 fan = 0;
        for (const Leaf &l : leaves) {
            fan += qint64(rowAx.members[l.row].size()) * qint64(colAx.members[l.col].size());
            if (fan > kPivotMaxFanOut) return fail(QStringLiteral("the table would take too much work to total"));
        }
        if (anyShownAs)
            for (const Leaf &l : leaves) {
                auto into = [&](Acc &a) {
                    if (a.n == 0 && a.error.isEmpty()) a = l.acc;
                    else a.merge(l.acc);
                };
                for (int ri : rowAx.members[l.row]) into(rowTot[size_t(ri) * D + l.value]);
                for (int ci : colAx.members[l.col]) into(colTot[size_t(ci) * D + l.value]);
                into(grandTot[size_t(l.value)]);
            }
        for (const Leaf &l : leaves)
            for (int ri : rowAx.members[l.row])
                for (int ci : colAx.members[l.col]) {
                    Acc &a = acc[size_t(ri) * nCI + ci];
                    if (a.n == 0 && a.error.isEmpty()) a = l.acc;
                    else a.merge(l.acc);
                    for (size_t k = 0; k < l.ex.size(); ++k) {
                        Acc &x = exCell[(size_t(ri) * nCI + ci) * size_t(nEx) + k];
                        if (x.n == 0 && x.error.isEmpty()) x = l.ex[k];
                        else x.merge(l.ex[k]);
                    }
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
    const int captionRow = bare ? 1 : (t.autoFormatId == 4097 ? 0 :(tabular ? ((nC > 0 || (lone && t.gridDropZones)) ? 1 : 0) : ((realC > 0 || nC > 0 || (lone && t.gridDropZones)) ? 1 : 0)));
    // (a classic-grid table has the line of the value's name over the others in every form; a table made now with one value and
    // no field across has the row fields' names and the value's name in one header line; with the values across, or a field
    // across, any form has a line over the header: the caption of the values, or of the column labels)
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
    QVector<int> rowCur(nR, -1);   // (the place of the item at each level, as the lines go down: a field that repeats its labels shows it on every line)
    for (int ri = 0; ri < nRI; ++ri) {
        const PivotAxisItem &it = rowAx.items[ri];
        const int row = headerRows + ri;
        if (!compact && nR > 0 && it.t != QLatin1String("grand") && it.t != QLatin1String("blank")) {
            for (int k = 0; k < it.x.size() && it.r + k < nR; ++k) rowCur[it.r + k] = it.x[k];
            for (int level = 0; level < it.r && level < nR && it.t.isEmpty(); ++level)
                if (t.rowFields[level] >= 0 && t.fields[t.rowFields[level]].fillDownLabels && rowCur[level] >= 0) set(row, level, itemLabel(t.rowFields, level, rowCur[level]));
        }
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
        if (captionRow > 0 || !oneValue || t.autoFormatId == 4097) set(lastItemsRow, labelCols, text(QStringLiteral("Total")));
        else set(lastItemsRow, labelCols, text(t.dataFields[0].name));
    }
    // the values
    std::vector<PivotCell> shown(size_t(nRI) * size_t(nCI));
    std::vector<int> dataOf(size_t(nRI) * size_t(nCI), -1);
    std::vector<Fn> fnAt(size_t(nRI) * size_t(nCI), Fn::None);
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
            PivotCell pc = result(acc[size_t(ri) * nCI + ci], fn, t.dataFields[di].numFmtCode.isEmpty() ? t.dataFields[di].numFmtId : -2 - di);
            if (c.fields[t.dataFields[di].fld].isCalculated() && acc[size_t(ri) * nCI + ci].n > 0) {
                const size_t cellAt = size_t(ri) * size_t(nCI) + size_t(ci);
                pc = PivotCell();
                pc.isValue = true;
                pc.numFmtId = t.dataFields[di].numFmtCode.isEmpty() ? t.dataFields[di].numFmtId : -2 - di;
                fn = Fn::Sum;
                std::function<PivotFormulaValue(int, int)> work = [&](int field, int depth) -> PivotFormulaValue {
                    return formulaOf[size_t(field)]->evaluate([&](const QString &name) -> PivotFormulaValue {
                        const int g = fieldByName.value(name.toLower(), -1);
                        if (g < 0 || depth > 16) return PivotFormulaValue::err(QStringLiteral("#NAME?"));
                        if (g == field) return PivotFormulaValue::err(QStringLiteral("#REF!"));
                        if (c.fields[g].isCalculated()) return work(g, depth + 1);
                        const Acc &a = exCell[cellAt * size_t(nEx) + size_t(exIndex[g])];
                        if (!a.error.isEmpty()) return PivotFormulaValue::err(a.error);
                        return PivotFormulaValue::num(a.n == 0 ? 0 : a.sum);
                    });
                };
                const PivotFormulaValue fv = work(t.dataFields[di].fld, 0);
                switch (fv.kind) {
                case PivotFormulaValue::Number: pc.kind = PivotCell::Number; pc.number = fv.number; break;
                case PivotFormulaValue::Bool: pc.kind = PivotCell::Bool; pc.number = fv.number; break;
                case PivotFormulaValue::Text: pc.kind = PivotCell::Text; pc.text = fv.text; break;
                case PivotFormulaValue::Error: pc.kind = PivotCell::Error; pc.text = fv.text; break;
                }
            }
            if (pc.kind == PivotCell::Empty && !t.showMissing && (fn == Fn::Sum || fn == Fn::Count || fn == Fn::CountNums)) {
                pc.kind = PivotCell::Number;
                pc.number = 0;
            }
            shown[size_t(ri) * nCI + ci] = pc;
            dataOf[size_t(ri) * nCI + ci] = di;
            fnAt[size_t(ri) * nCI + ci] = fn;
        }
    }
    if (!calcs.empty() && D > 0) {
        // the path of each line (the place of its item at each level it reaches), the line of a kind and a path, and the calculated items a line has
        struct Side {
            std::vector<Tuple> path;
            std::unordered_map<std::string, int> lineOf;
            std::vector<std::vector<std::pair<int, int>>> comps;   // for each line, (level, calculated item) pairs
            bool any = false;
        };
        auto keyOfLine = [](const PivotAxisItem &it, const Tuple &path) {
            Tuple k(path);
            k.push_back(-7);
            k.push_back(it.t.isEmpty() ? 0 : it.t.at(0).unicode());
            k.push_back(it.i);
            return keyOf(k);
        };
        auto sideOf = [&](const Axis &ax, const QVector<int> &fields) {
            Side sd;
            const int nLevels = int(fields.size());
            sd.path.resize(ax.items.size());
            sd.comps.resize(ax.items.size());
            Tuple cur(size_t(nLevels), -1);
            for (size_t i = 0; i < ax.items.size(); ++i) {
                const PivotAxisItem &it = ax.items[i];
                if (it.t != QLatin1String("grand") && it.t != QLatin1String("blank")) {
                    int levels = 0;
                    for (int k = 0; k < it.x.size() && it.r + k < nLevels; ++k) {
                        cur[size_t(it.r + k)] = it.x[k];
                        levels = it.r + k + 1;
                    }
                    for (int l = levels; l < nLevels; ++l) cur[size_t(l)] = -1;
                    sd.path[i] = Tuple(cur.begin(), cur.begin() + levels);
                }
                if (it.labelOnly) continue;
                sd.lineOf.emplace(keyOfLine(it, sd.path[i]), int(i));
                for (int l = 0; l < int(sd.path[i].size()); ++l) {
                    if (fields[l] < 0 || sd.path[i][size_t(l)] < 0) continue;
                    const auto &list = itemOfPos[fields[l]];
                    const int pos = sd.path[i][size_t(l)];
                    const int shared = t.fields[fields[l]].hasItems ? (pos < list.size() ? list[pos]->x : -1) : pos;
                    const auto found = calcOfItem.constFind((quint64(fields[l]) << 32) | quint32(shared));
                    if (shared >= 0 && found != calcOfItem.constEnd()) {
                        sd.comps[i].emplace_back(l, found.value());
                        sd.any = true;
                    }
                }
            }
            return sd;
        };
        const Side rowSide = sideOf(rowAx, t.rowFields), colSide = sideOf(colAx, t.colFields);
        if (rowSide.any || colSide.any) {
            std::vector<char> state(size_t(nRI) * size_t(nCI), 0);
            qint64 work = 0;
            bool tooMuch = false;
            std::function<void(int, int, int)> eval = [&](int ri, int ci, int depth) {
                const size_t at = size_t(ri) * size_t(nCI) + size_t(ci);
                if (state[at] == 2 || dataOf[at] < 0) return;
                PivotCell &out = shown[at];
                auto fail = [&](const QString &e) {
                    out = PivotCell();
                    out.kind = PivotCell::Error;
                    out.text = e;
                    out.isValue = true;
                    state[at] = 2;
                };
                if (state[at] == 1 || depth > 400) return fail(QStringLiteral("#REF!"));
                // the item worked out last: the highest solve order (of two with the same, the one made later)
                int axis = -1, level = 0, best = -1;
                for (int a = 0; a < 2; ++a)
                    for (const auto &lc : (a == 0 ? rowSide : colSide).comps[size_t(a == 0 ? ri : ci)])
                        if (best < 0 || std::tie(calcs[size_t(lc.second)].order, lc.second) > std::tie(calcs[size_t(best)].order, best)) {
                            axis = a;
                            level = lc.first;
                            best = lc.second;
                        }
                if (best < 0) return;
                const CalcInfo &info = calcs[size_t(best)];
                if (!info.formula) return fail(QStringLiteral("#NAME?"));
                work += qint64(info.names.size()) + 1;
                if (work > kPivotMaxCalcWork) {
                    tooMuch = true;
                    return fail(QStringLiteral("#REF!"));
                }
                state[at] = 1;
                bool anyValue = false;
                const Side &side = axis == 0 ? rowSide : colSide;
                const int line = axis == 0 ? ri : ci;
                const PivotFormulaValue fv = info.formula->evaluate([&](const QString &name) -> PivotFormulaValue {
                    const int j = int(info.names.indexOf(name));
                    const qint32 x = j >= 0 && j < info.refShared.size() ? info.refShared[j] : -1;
                    if (x < 0) return PivotFormulaValue::err(QStringLiteral("#NAME?"));
                    const int pos = x == info.shared ? -1 : posOf[info.field][x];
                    if (pos < 0) return PivotFormulaValue::err(QStringLiteral("#REF!"));
                    Tuple path = side.path[size_t(line)];
                    path[size_t(level)] = pos;
                    const auto found = side.lineOf.find(keyOfLine((axis == 0 ? rowAx : colAx).items[line], path));
                    if (found == side.lineOf.end()) return PivotFormulaValue::num(0);   // (no line: the item has nothing in this place)
                    const int rl = axis == 0 ? found->second : ri, cl = axis == 0 ? ci : found->second;
                    eval(rl, cl, depth + 1);
                    const PivotCell &ref = shown[size_t(rl) * size_t(nCI) + size_t(cl)];
                    switch (ref.kind) {
                    case PivotCell::Number: anyValue = true; return PivotFormulaValue::num(ref.number);
                    case PivotCell::Empty: return PivotFormulaValue::num(0);
                    case PivotCell::Error: return PivotFormulaValue::err(ref.text);
                    default: return PivotFormulaValue::err(QStringLiteral("#VALUE!"));
                    }
                });
                const int di = dataOf[at];
                PivotCell pc;
                pc.isValue = true;
                pc.numFmtId = t.dataFields[di].numFmtCode.isEmpty() ? t.dataFields[di].numFmtId : -2 - di;
                switch (fv.kind) {
                case PivotFormulaValue::Number:
                    if (anyValue) {
                        pc.kind = PivotCell::Number;
                        pc.number = fv.number;
                    }
                    break;
                case PivotFormulaValue::Bool: pc.kind = PivotCell::Bool; pc.number = fv.number; break;
                case PivotFormulaValue::Text: pc.kind = PivotCell::Text; pc.text = fv.text; break;
                case PivotFormulaValue::Error: pc.kind = PivotCell::Error; pc.text = fv.text; break;
                }
                out = pc;
                state[at] = 2;
            };
            for (int ri = 0; ri < nRI && !tooMuch; ++ri)
                for (int ci = 0; ci < nCI && !tooMuch; ++ci)
                    if (!rowSide.comps[size_t(ri)].empty() || !colSide.comps[size_t(ci)].empty()) eval(ri, ci, 0);
            if (tooMuch) return fail(QStringLiteral("the calculated items would take too much work to total"));
        }
    }
    if (anyShownAs) {
        // the path of each line: the place of its item at each level it reaches
        auto pathsOf = [&](const Axis &ax, int nLevels) {
            std::vector<Tuple> paths(ax.items.size());
            Tuple cur(size_t(nLevels), -1);
            for (size_t i = 0; i < ax.items.size(); ++i) {
                const PivotAxisItem &it = ax.items[i];
                if (it.t == QLatin1String("grand") || it.t == QLatin1String("blank")) continue;
                int levels = 0;
                for (int k = 0; k < it.x.size() && it.r + k < nLevels; ++k) {
                    cur[size_t(it.r + k)] = it.x[k];
                    levels = it.r + k + 1;
                }
                for (int l = levels; l < nLevels; ++l) cur[size_t(l)] = -1;
                paths[i] = Tuple(cur.begin(), cur.begin() + levels);
            }
            return paths;
        };
        const std::vector<Tuple> rowPath = pathsOf(rowAx, nR), colPath = pathsOf(colAx, nC);
        auto levelOf = [&](const QVector<int> &fields, int f) {
            for (int l = 0; l < fields.size(); ++l)
                if (fields[l] == f) return l;
            return -1;
        };
        auto keyWith = [](const PivotAxisItem &it, const Tuple &path, int level, int place) {
            Tuple k(path);
            if (level < int(k.size())) k[size_t(level)] = place;
            k.push_back(-7);
            k.push_back(it.t.isEmpty() ? 0 : it.t.at(0).unicode());
            return keyOf(k);
        };
        struct Side {
            std::unordered_map<std::string, int> exact;                 // (the line's kind, its path) to the line
            std::unordered_map<std::string, std::vector<int>> group;    // (the same with the base level left out) to the lines, in order
            std::vector<const std::vector<int> *> sibsOf;               // for each line, the lines of its group (null for one with none)
            std::vector<int> placeOf;                                   // for each line, its place among them
        };
        auto sideOf = [&](const Axis &ax, const std::vector<Tuple> &paths, int level) {
            Side sd;
            std::vector<std::string> groupKey(ax.items.size());
            sd.placeOf.assign(ax.items.size(), -1);
            for (size_t i = 0; i < ax.items.size(); ++i) {
                const PivotAxisItem &it = ax.items[i];
                if (it.t == QLatin1String("grand") || it.t == QLatin1String("blank") || it.labelOnly || int(paths[i].size()) <= level) continue;
                sd.exact.emplace(keyWith(it, paths[i], level, paths[i][size_t(level)]), int(i));
                groupKey[i] = keyWith(it, paths[i], level, -3);
                std::vector<int> &g = sd.group[groupKey[i]];
                sd.placeOf[i] = int(g.size());
                g.push_back(int(i));
            }
            // (the lists are not moved once made: a pointer to each is kept with the line, for the cells to use)
            sd.sibsOf.assign(ax.items.size(), nullptr);
            for (size_t i = 0; i < ax.items.size(); ++i)
                if (sd.placeOf[i] >= 0) sd.sibsOf[i] = &sd.group.find(groupKey[i])->second;
            return sd;
        };
        std::vector<std::unique_ptr<Side>> rowSides(size_t(std::max(D, 1))), colSides(size_t(std::max(D, 1)));
        std::vector<int> baseLevel(size_t(std::max(D, 1)), -1);
        std::vector<char> baseOnRows(size_t(std::max(D, 1)), 1);
        for (int d = 0; d < D; ++d) {
            const PivotDataField &df = t.dataFields[d];
            if (df.showDataAs != QLatin1String("difference") && df.showDataAs != QLatin1String("percent") && df.showDataAs != QLatin1String("percentDiff") && df.showDataAs != QLatin1String("runTotal") &&
                df.showDataAs != QLatin1String("rankAscending") && df.showDataAs != QLatin1String("rankDescending"))
                continue;
            int l = levelOf(t.rowFields, df.baseField);
            if (l >= 0) {
                rowSides[d].reset(new Side(sideOf(rowAx, rowPath, l)));
            } else {
                l = levelOf(t.colFields, df.baseField);
                if (l >= 0) {
                    colSides[d].reset(new Side(sideOf(colAx, colPath, l)));
                    baseOnRows[d] = 0;
                }
            }
            baseLevel[d] = l;
        }
        const std::vector<PivotCell> raw = shown;
        // (for a percent of a parent: the line of each line's parent on its axis, made when first wanted; -1 for none found, -2 for the grand total)
        std::vector<int> parentOfRow, parentOfCol;
        auto parentsOf = [&](const Axis &ax, const std::vector<Tuple> &paths, std::vector<int> *out) {
            if (!out->empty()) return;
            std::unordered_map<std::string, int> byPath;
            for (size_t i = 0; i < ax.items.size(); ++i) {
                const PivotAxisItem &it = ax.items[i];
                if (it.t == QLatin1String("grand") || it.t == QLatin1String("blank") || it.labelOnly || paths[i].empty()) continue;
                byPath.emplace(keyOf(paths[i]), int(i));
            }
            out->assign(ax.items.size(), -1);
            for (size_t i = 0; i < ax.items.size(); ++i) {
                const Tuple &p = paths[i];
                if (p.empty()) continue;
                if (p.size() == 1) {
                    (*out)[i] = -2;
                    continue;
                }
                const auto f = byPath.find(keyOf(Tuple(p.begin(), p.end() - 1)));
                if (f != byPath.end()) (*out)[i] = f->second;
            }
        };
        std::vector<std::vector<double>> running(size_t(std::max(D, 1)));   // (for a running total: its sum at each cell, made when first wanted)
        auto number = [&](const PivotCell &c, double *v) {
            if (c.kind != PivotCell::Number) return false;
            *v = c.number;
            return true;
        };
        auto divide = [&](PivotCell *pc, double num, double den) {
            if (den == 0) {
                pc->kind = PivotCell::Error;
                pc->text = QStringLiteral("#DIV/0!");
            } else {
                pc->kind = PivotCell::Number;
                pc->number = num / den;
            }
        };
        auto blankCell = [&](size_t at) {
            PivotCell pc;
            pc.isValue = true;
            shown[at] = pc;
        };
        for (int ri = 0; ri < nRI; ++ri)
            for (int ci = 0; ci < nCI; ++ci) {
                const size_t at = size_t(ri) * nCI + ci;
                const int d = dataOf[at];
                if (d < 0 || raw[at].kind == PivotCell::Empty) continue;
                const PivotDataField &df = t.dataFields[d];
                if (df.showDataAs == QLatin1String("normal")) continue;
                PivotCell pc = raw[at];
                double v = 0;
                if (!number(pc, &v)) continue;   // (an error stays the error)
                const Fn fn = fnAt[at];
                auto totalOf = [&](const Acc &a) {
                    const PivotCell c = result(a, fn, 0);
                    return c.kind == PivotCell::Number ? c.number : 0.0;
                };
                if (df.showDataAs == QLatin1String("percentOfTotal")) {
                    divide(&pc, v, totalOf(grandTot[size_t(d)]));
                } else if (df.showDataAs == QLatin1String("percentOfRow")) {
                    divide(&pc, v, totalOf(rowTot[size_t(ri) * D + d]));
                } else if (df.showDataAs == QLatin1String("percentOfCol")) {
                    divide(&pc, v, totalOf(colTot[size_t(ci) * D + d]));
                } else if (df.showDataAs == QLatin1String("percentOfParentRow") || df.showDataAs == QLatin1String("percentOfParentCol")) {
                    const bool onRows = df.showDataAs == QLatin1String("percentOfParentRow");
                    const PivotAxisItem &own = onRows ? rowAx.items[ri] : colAx.items[ci];
                    if (own.t == QLatin1String("grand") || own.t == QLatin1String("blank")) {
                        blankCell(at);
                        continue;
                    }
                    parentsOf(onRows ? rowAx : colAx, onRows ? rowPath : colPath, onRows ? &parentOfRow : &parentOfCol);
                    const int parent = (onRows ? parentOfRow : parentOfCol)[size_t(onRows ? ri : ci)];
                    double den = 0;
                    if (parent == -2) den = totalOf(onRows ? colTot[size_t(ci) * D + d] : rowTot[size_t(ri) * D + d]);
                    else if (parent < 0 || !number(onRows ? raw[size_t(parent) * nCI + ci] : raw[size_t(ri) * nCI + parent], &den)) {
                        blankCell(at);
                        continue;
                    }
                    divide(&pc, v, den);
                } else if (df.showDataAs == QLatin1String("index")) {
                    const double rt = totalOf(rowTot[size_t(ri) * D + d]), ct = totalOf(colTot[size_t(ci) * D + d]);
                    divide(&pc, v * totalOf(grandTot[size_t(d)]), rt * ct);
                } else {
                    // against another item of the base field: the same line of the other axis
                    const bool onRows = baseOnRows[d] != 0;
                    const int level = baseLevel[d];
                    const Side *side = onRows ? rowSides[d].get() : colSides[d].get();
                    const PivotAxisItem &own = onRows ? rowAx.items[ri] : colAx.items[ci];
                    const Tuple &ownPath = onRows ? rowPath[size_t(ri)] : colPath[size_t(ci)];
                    if (!side || level < 0 || int(ownPath.size()) <= level || own.t == QLatin1String("grand") || own.t == QLatin1String("blank")) {
                        blankCell(at);
                        continue;
                    }
                    const int ownLine = onRows ? ri : ci;
                    if (ownLine >= int(side->placeOf.size()) || side->placeOf[size_t(ownLine)] < 0) {
                        blankCell(at);
                        continue;
                    }
                    const std::vector<int> &sibs = *side->sibsOf[size_t(ownLine)];
                    const int me = side->placeOf[size_t(ownLine)];
                    auto cellOf = [&](int line) -> const PivotCell & { return onRows ? raw[size_t(line) * nCI + ci] : raw[size_t(ri) * nCI + line]; };
                    if (df.showDataAs == QLatin1String("rankAscending") || df.showDataAs == QLatin1String("rankDescending")) {
                        // (the place of the value among those of the group's lines, 1 for the largest, or for the smallest when ascending)
                        const bool up = df.showDataAs == QLatin1String("rankAscending");
                        int place = 1;
                        for (int line : sibs) {
                            double x = 0;
                            if (number(cellOf(line), &x) && (up ? x < v : x > v)) ++place;
                        }
                        pc.number = place;
                    } else if (df.showDataAs == QLatin1String("runTotal")) {
                        // (the sums are worked out once for the value, line by line along each group, and kept)
                        std::vector<double> &run = running[size_t(d)];
                        if (run.empty()) {
                            run.assign(size_t(nRI) * nCI, 0);
                            for (const auto &grp : side->group) {
                                const int others = onRows ? nCI : nRI;
                                for (int o = 0; o < others; ++o) {
                                    double sum = 0;
                                    for (int line : grp.second) {
                                        const size_t cellAt = onRows ? size_t(line) * nCI + o : size_t(o) * nCI + line;
                                        double x = 0;
                                        if (number(raw[cellAt], &x)) sum += x;
                                        run[cellAt] = sum;
                                    }
                                }
                            }
                        }
                        pc.number = run[onRows ? size_t(ri) * nCI + ci : size_t(ri) * nCI + ci];
                    } else {
                        int baseLine = -1;
                        if (df.baseItem == kPivotPreviousItem) baseLine = me > 0 ? sibs[size_t(me) - 1] : -1;
                        else if (df.baseItem == kPivotNextItem) baseLine = me + 1 < int(sibs.size()) ? sibs[size_t(me) + 1] : -1;
                        else {
                            const auto e = side->exact.find(keyWith(own, ownPath, level, df.baseItem));
                            if (e != side->exact.end()) baseLine = e->second;
                        }
                        double b = 0;
                        if (baseLine < 0 || !number(cellOf(baseLine), &b)) {
                            blankCell(at);
                            continue;
                        }
                        if (df.showDataAs == QLatin1String("percent")) divide(&pc, v, b);
                        else if (baseLine == (onRows ? ri : ci)) {
                            blankCell(at);
                            continue;
                        } else if (df.showDataAs == QLatin1String("difference")) pc.number = v - b;
                        else divide(&pc, v - b, b);
                    }
                }
                pc.isValue = true;
                pc.numFmtId = df.numFmtCode.isEmpty() ? df.numFmtId : -2 - d;
                shown[at] = pc;
            }
    }
    for (int ri = 0; ri < nRI; ++ri) {
        for (int ci = 0; ci < nCI; ++ci) {
            const size_t at = size_t(ri) * nCI + ci;
            if (dataOf[at] < 0) continue;
            PivotCell pc = shown[at];
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

namespace jp::office::sheet {

bool pivotFilterField(PivotTable *t, const PivotCache &c, int field, const PivotFilter &filter, QString *why)
{
    auto fail = [&](const QString &m) {
        if (why) *why = m;
        return false;
    };
    if (field < 0 || field >= t->fields.size() || field >= c.fields.size()) return fail(QStringLiteral("the field is not there"));
    PivotField &pf = t->fields[field];
    const PivotCacheField &cf = c.fields[field];
    if (!pf.hasItems) {
        pf.hasItems = true;
        for (int i = 0; i < cf.items.size(); ++i) {
            PivotTableItem it;
            it.x = i;
            pf.items.append(it);
        }
        PivotTableItem total;
        total.t = QStringLiteral("default");
        if (pf.defaultSubtotal) pf.items.append(total);
    }
    const bool byValue = filter.kind >= PivotFilter::ValueGreater;
    // the total of the data field for each shared item, over the records the filters of the page let in
    QVector<Acc> accs;
    QVector<bool> present;
    Fn fn = Fn::Sum;
    if (byValue) {
        if (filter.dataField < 0 || filter.dataField >= t->dataFields.size()) return fail(QStringLiteral("the value is not there"));
        const PivotDataField &df = t->dataFields[filter.dataField];
        if (df.fld < 0 || df.fld >= c.fields.size()) return fail(QStringLiteral("the value is not in the cache"));
        if (c.fields[df.fld].isCalculated()) return fail(QStringLiteral("a filter by a calculated field is not offered"));
        fn = fnOf(df.subtotal);
        accs.resize(cf.items.size());
        present.fill(false, cf.items.size());
        const PivotCacheField &vf = c.fields[df.fld];
        const qint64 n = cf.records.size();
        // the item each other filter of the page lets in, worked out once (a record is tested by one number each)
        QVector<QPair<int, qint32>> pageWant;
        for (const PivotPageField &p : t->pageFields) {
            if (p.fld == field || p.fld < 0 || p.fld >= c.fields.size() || p.item < 0) continue;
            const PivotField &ppf = t->fields[p.fld];
            int want = p.item;
            if (ppf.hasItems) {
                int pos = 0;
                want = -1;
                for (const PivotTableItem &it : ppf.items) {
                    if (!it.t.isEmpty()) continue;
                    if (pos++ == p.item) want = it.x;
                }
            }
            pageWant.append({p.fld, want});
        }
        for (qint64 r = 0; r < n && r < vf.records.size(); ++r) {
            bool in = true;
            for (const auto &pw : pageWant)
                if (c.fields[pw.first].records[r] != pw.second) {
                    in = false;
                    break;
                }
            if (!in) continue;
            const qint32 x = cf.records[r];
            if (x < 0 || x >= accs.size()) continue;
            accs[x].add(vf.storedItems()[vf.storedRecords()[r]]);
            present[x] = true;
        }
    }
    auto totalOf = [&](int x, double *v) {
        if (x < 0 || x >= accs.size() || !present[x]) return false;
        const PivotCell pc = result(accs[x], fn, 0);
        if (pc.kind != PivotCell::Number) return false;
        *v = pc.number;
        return true;
    };
    auto labelOf = [&](int x) {
        const PivotValue &v = cf.items[x];
        if (v.kind == PivotValue::Missing) return QString();
        return v.isNumeric() || v.kind == PivotValue::Bool ? QString::number(v.number, 'g', 15) : v.text;
    };
    // the items that stay
    QSet<int> keep;
    QVector<std::pair<double, int>> ranked;
    for (const PivotTableItem &it : pf.items) {
        if (!it.t.isEmpty()) continue;
        if (it.x < 0 || it.x >= cf.items.size()) continue;
        bool ok = true;
        double v = 0;
        const QString label = it.n.isEmpty() ? labelOf(it.x) : it.n;
        switch (filter.kind) {
        case PivotFilter::None: break;
        case PivotFilter::LabelBeginsWith: ok = label.startsWith(filter.text, Qt::CaseInsensitive); break;
        case PivotFilter::LabelEndsWith: ok = label.endsWith(filter.text, Qt::CaseInsensitive); break;
        case PivotFilter::LabelContains: ok = label.contains(filter.text, Qt::CaseInsensitive); break;
        case PivotFilter::LabelNotContains: ok = !label.contains(filter.text, Qt::CaseInsensitive); break;
        case PivotFilter::LabelEquals: ok = label.compare(filter.text, Qt::CaseInsensitive) == 0; break;
        case PivotFilter::LabelNotEquals: ok = label.compare(filter.text, Qt::CaseInsensitive) != 0; break;
        case PivotFilter::TopItems:
        case PivotFilter::BottomItems:
            ok = false;
            if (totalOf(it.x, &v)) ranked.append({v, it.x});
            break;
        default: {
            const bool have = totalOf(it.x, &v);
            const double lo = std::min(filter.a, filter.b), hi = std::max(filter.a, filter.b);
            switch (filter.kind) {
            case PivotFilter::ValueGreater: ok = have && v > filter.a; break;
            case PivotFilter::ValueGreaterOrEqual: ok = have && v >= filter.a; break;
            case PivotFilter::ValueLess: ok = have && v < filter.a; break;
            case PivotFilter::ValueLessOrEqual: ok = have && v <= filter.a; break;
            case PivotFilter::ValueEquals: ok = have && v == filter.a; break;
            case PivotFilter::ValueNotEquals: ok = !have || v != filter.a; break;
            case PivotFilter::ValueBetween: ok = have && v >= lo && v <= hi; break;
            case PivotFilter::ValueNotBetween: ok = !have || v < lo || v > hi; break;
            default: break;
            }
        }
        }
        if (ok) keep.insert(it.x);
    }
    if (filter.kind == PivotFilter::TopItems || filter.kind == PivotFilter::BottomItems) {
        const bool top = filter.kind == PivotFilter::TopItems;
        std::stable_sort(ranked.begin(), ranked.end(), [&](const auto &x, const auto &y) { return top ? x.first > y.first : x.first < y.first; });
        const int n = filter.a >= 1 ? int(std::min(filter.a, double(ranked.size()))) : 0;   // (a number from a person or a file: kept in range before it is made an int)
        for (int i = 0; i < n && i < ranked.size(); ++i) keep.insert(ranked[i].second);
    }
    for (PivotTableItem &it : pf.items)
        if (it.t.isEmpty()) it.hidden = filter.kind != PivotFilter::None && !keep.contains(it.x);
    return true;
}

} // namespace jp::office::sheet
