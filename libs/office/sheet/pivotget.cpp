// GETPIVOTDATA: the cell of a worked-out pivot table that a data field and a set of items name, and the other way round (the arguments for a cell).
#include "sheet/pivot.h"

#include <algorithm>

namespace jp::office::sheet {

namespace {

// A line of an axis with the places of its items in each level (as the file's repeated items are read), and what kind of line it is.
struct LinePath {
    int depth = 0;                 // how many levels of the path are the line's
    std::vector<int> path;         // the place of the item in each level's list (for the values' level, the data field)
    QString t;                     // "" a line of data, default or another function for a subtotal, grand, blank
    int dataIndex = 0;             // the data field of the line, when the values are on this axis and the line does not name them in its path
    bool labelOnly = false;
};

std::vector<LinePath> pathsOf(const QVector<PivotAxisItem> &items, int levels)
{
    std::vector<LinePath> out;
    out.reserve(size_t(items.size()));
    std::vector<int> cur(size_t(std::max(levels, 0)), -1);
    for (const PivotAxisItem &it : items) {
        LinePath lp;
        lp.t = it.t;
        lp.dataIndex = it.i;
        lp.labelOnly = it.labelOnly;
        if (it.t == QLatin1String("grand")) {
            lp.depth = 0;
        } else {
            for (int k = 0; k < it.x.size(); ++k)
                if (it.r + k < levels) cur[size_t(it.r + k)] = it.x[k];
            lp.depth = std::min(levels, it.r + int(it.x.size()));
        }
        lp.path = cur;
        out.push_back(std::move(lp));
    }
    return out;
}

bool sameValue(const PivotValue &item, const PivotValue &asked)
{
    auto numberText = [](double d) { return QString::number(d, 'g', 15); };
    switch (item.kind) {
    case PivotValue::Missing: return asked.kind == PivotValue::Missing || (asked.kind == PivotValue::Text && (asked.text.isEmpty() || asked.text.compare(QLatin1String("(blank)"), Qt::CaseInsensitive) == 0));
    case PivotValue::Text:
        if (asked.kind == PivotValue::Text) return item.text.compare(asked.text, Qt::CaseInsensitive) == 0;
        if (asked.kind == PivotValue::Number || asked.kind == PivotValue::Date) return item.text == numberText(asked.number);
        return false;
    case PivotValue::Number:
    case PivotValue::Date:
        if (asked.kind == PivotValue::Number || asked.kind == PivotValue::Date) return item.number == asked.number;
        if (asked.kind == PivotValue::Text) {
            bool ok = false;
            const double d = asked.text.toDouble(&ok);
            return ok && d == item.number;
        }
        return false;
    case PivotValue::Bool: return asked.kind == PivotValue::Bool && (item.number != 0) == (asked.number != 0);
    case PivotValue::Error: return asked.kind == PivotValue::Error && item.text.compare(asked.text, Qt::CaseInsensitive) == 0;
    }
    return false;
}

QString fieldCaption(const PivotTable &t, const PivotCache &c, int f)
{
    return f >= 0 && f < t.fields.size() && !t.fields[f].name.isEmpty() ? t.fields[f].name : c.fields[f].name;
}

// The place of an item of a field's list (its shared item's place in the cache) among the table's items of the field, or -1 when the table does not list it (hidden).
int positionOf(const PivotTable &t, int f, int sharedIndex)
{
    const PivotField &pf = t.fields[f];
    if (!pf.hasItems) return sharedIndex;
    int p = 0;
    for (const PivotTableItem &it : pf.items) {
        if (!it.t.isEmpty()) continue;
        if (it.x == sharedIndex) return it.hidden ? -1 : p;
        ++p;
    }
    return -1;
}

int sharedOfPosition(const PivotTable &t, int f, int position)
{
    const PivotField &pf = t.fields[f];
    if (!pf.hasItems) return position;
    int p = 0;
    for (const PivotTableItem &it : pf.items) {
        if (!it.t.isEmpty()) continue;
        if (p++ == position) return it.x;
    }
    return -1;
}

int dataIndexOf(const PivotTable &t, const PivotCache &c, const QString &name)
{
    for (int i = 0; i < t.dataFields.size(); ++i)
        if (t.dataFields[i].name.compare(name, Qt::CaseInsensitive) == 0) return i;
    int found = -1;
    for (int i = 0; i < t.dataFields.size(); ++i) {
        const int f = t.dataFields[i].fld;
        if (f >= 0 && f < c.fields.size() && fieldCaption(t, c, f).compare(name, Qt::CaseInsensitive) == 0) {
            if (found >= 0) return -1;   // (the name is the field of two values: it does not say which)
            found = i;
        }
    }
    return found;
}

// The place of the first line of an axis that the requirements name: `req[level]` is the place asked for at a level (-1: not asked). Levels asked for must be the first ones.
int findLine(const std::vector<LinePath> &lines, const QVector<int> &fields, const std::vector<int> &req, int dSlot, int d)
{
    const int levels = int(fields.size());
    if (levels == 0) return lines.empty() ? -1 : 0;
    int k = 0;
    while (k < levels && (req[size_t(k)] >= 0 || fields[k] == -2)) ++k;
    for (int l = k; l < levels; ++l)
        if (req[size_t(l)] >= 0) return -1;   // (a field asked for after one that was not: not a group the table has)
    for (size_t i = 0; i < lines.size(); ++i) {
        const LinePath &lp = lines[i];
        if (lp.labelOnly || lp.t == QLatin1String("blank")) continue;
        if (k == 0) {
            if (lp.t != QLatin1String("grand")) continue;
        } else if (k == levels) {
            if (!lp.t.isEmpty() || lp.depth != levels) continue;
        } else {
            if (lp.t.isEmpty() || lp.t == QLatin1String("grand") || lp.depth != k) continue;
        }
        bool ok = true;
        for (int l = 0; l < k && ok; ++l) {
            if (fields[l] == -2) ok = lp.path[size_t(l)] == d;
            else ok = lp.path[size_t(l)] == req[size_t(l)];
        }
        if (!ok) continue;
        // (a line that does not name the values in its path is one for each value: the one of this value)
        if (dSlot >= 0 && dSlot >= lp.depth && lp.dataIndex != d) continue;
        return int(i);
    }
    return -1;
}

} // namespace

bool pivotGetData(const PivotTable &t, const PivotCache &c, const PivotGrid &g, const QString &dataField, const std::vector<std::pair<QString, PivotValue>> &items, PivotCell *cell)
{
    if (t.fields.size() != c.fields.size()) return false;
    const int d = dataIndexOf(t, c, dataField);
    if (d < 0) return false;
    std::vector<int> rowReq(size_t(t.rowFields.size()), -1), colReq(size_t(t.colFields.size()), -1);
    int used = 0;
    for (const auto &[name, asked] : items) {
        int f = -1;
        for (int i = 0; i < c.fields.size() && f < 0; ++i)
            if (fieldCaption(t, c, i).compare(name, Qt::CaseInsensitive) == 0 || c.fields[i].name.compare(name, Qt::CaseInsensitive) == 0) f = i;
        if (f < 0) return false;
        int shared = -1;
        for (int i = 0; i < c.fields[f].items.size(); ++i)
            if (sameValue(c.fields[f].items[i], asked)) {
                shared = i;
                break;
            }
        if (shared < 0) return false;
        const int pos = positionOf(t, f, shared);
        if (pos < 0) return false;
        const int rowLevel = t.rowFields.indexOf(f), colLevel = t.colFields.indexOf(f);
        if (rowLevel >= 0) {
            if (rowReq[size_t(rowLevel)] >= 0 && rowReq[size_t(rowLevel)] != pos) return false;
            rowReq[size_t(rowLevel)] = pos;
        } else if (colLevel >= 0) {
            if (colReq[size_t(colLevel)] >= 0 && colReq[size_t(colLevel)] != pos) return false;
            colReq[size_t(colLevel)] = pos;
        } else {
            // a filter: the item must be the one shown
            bool onPage = false;
            for (const PivotPageField &p : t.pageFields)
                if (p.fld == f) {
                    onPage = true;
                    if (p.item < 0 || p.item != pos) return false;
                }
            if (!onPage) return false;
        }
        ++used;
    }
    const int dRow = t.rowFields.indexOf(-2), dCol = t.colFields.indexOf(-2);
    const std::vector<LinePath> rows = pathsOf(g.rowItems, int(t.rowFields.size())), cols = pathsOf(g.colItems, int(t.colFields.size()));
    const int ri = findLine(rows, t.rowFields, rowReq, dRow, d), ci = findLine(cols, t.colFields, colReq, dCol, d);
    if (ri < 0 || ci < 0) return false;
    const int r = g.firstDataRow + ri, k = g.firstDataCol + ci;
    if (r < 0 || r >= g.rows || k < 0 || k >= g.cols) return false;
    const PivotCell &found = g.at(r, k);
    if (found.kind == PivotCell::Empty) return false;
    *cell = found;
    return true;
}

bool pivotGetDataArguments(const PivotTable &t, const PivotCache &c, const PivotGrid &g, int gridRow, int gridCol, QString *dataField, std::vector<std::pair<QString, PivotValue>> *items)
{
    if (t.fields.size() != c.fields.size() || t.dataFields.isEmpty()) return false;
    const int ri = gridRow - g.firstDataRow, ci = gridCol - g.firstDataCol;
    if (ri < 0 || ci < 0 || ri >= g.rowItems.size() || ci >= g.colItems.size() || gridRow >= g.rows || gridCol >= g.cols) return false;
    const PivotAxisItem &rowItem = g.rowItems[ri], &colItem = g.colItems[ci];
    if (rowItem.labelOnly || colItem.labelOnly || rowItem.t == QLatin1String("blank") || colItem.t == QLatin1String("blank")) return false;
    if (!g.at(gridRow, gridCol).isValue) return false;
    const int dRow = t.rowFields.indexOf(-2), dCol = t.colFields.indexOf(-2);
    const std::vector<LinePath> rows = pathsOf(g.rowItems, int(t.rowFields.size())), cols = pathsOf(g.colItems, int(t.colFields.size()));
    const LinePath &rp = rows[size_t(ri)], &cp = cols[size_t(ci)];
    int d = 0;
    if (dRow >= 0) d = dRow < rp.depth ? rp.path[size_t(dRow)] : rp.dataIndex;
    else if (dCol >= 0) d = dCol < cp.depth ? cp.path[size_t(dCol)] : cp.dataIndex;
    if (d < 0 || d >= t.dataFields.size()) return false;
    *dataField = t.dataFields[d].name;
    items->clear();
    auto add = [&](const QVector<int> &fields, const LinePath &lp) {
        for (int l = 0; l < lp.depth && l < fields.size(); ++l) {
            const int f = fields[l];
            if (f < 0) continue;
            const int shared = sharedOfPosition(t, f, lp.path[size_t(l)]);
            if (shared < 0 || shared >= c.fields[f].items.size()) return false;
            items->emplace_back(fieldCaption(t, c, f), c.fields[f].items[shared]);
        }
        return true;
    };
    return add(t.rowFields, rp) && add(t.colFields, cp);
}

} // namespace jp::office::sheet
