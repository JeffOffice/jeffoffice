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

QString numberText(double d) { return QString::number(d, 'g', 15); }

// What an item says, as one key: texts without regard to case (an accent is a difference), numbers and dates by their 15 digits, and so on.
QString keyOf(const PivotValue &v)
{
    switch (v.kind) {
    case PivotValue::Missing: return QStringLiteral("m");
    case PivotValue::Text: return QStringLiteral("t:") + v.text.toCaseFolded();
    case PivotValue::Number:
    case PivotValue::Date: return QStringLiteral("n:") + numberText(v.number);
    case PivotValue::Bool: return v.number != 0 ? QStringLiteral("b:1") : QStringLiteral("b:0");
    case PivotValue::Error: return QStringLiteral("e:") + v.text.toCaseFolded();
    }
    return QString();
}

// The keys an asked-for value may be an item by.
QStringList keysAsked(const PivotValue &v)
{
    QStringList out{keyOf(v)};
    switch (v.kind) {
    case PivotValue::Text:
        if (v.reading) out << QStringLiteral("n:") + numberText(v.number);
        if (v.text.isEmpty() || v.text.compare(QLatin1String("(blank)"), Qt::CaseInsensitive) == 0) out << QStringLiteral("m");
        break;
    case PivotValue::Number:
    case PivotValue::Date: out << QStringLiteral("t:") + numberText(v.number); break;
    default: break;
    }
    return out;
}

QString fieldCaption(const PivotTable &t, const PivotCache &c, int f)
{
    return f >= 0 && f < t.fields.size() && !t.fields[f].name.isEmpty() ? t.fields[f].name : c.fields[f].name;
}

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

// The key of a line of an axis: its kind, its depth, the value when the line is one for each value, and the places of its items.
QByteArray lineKey(char kind, int depth, int dataIndex, const int *path)
{
    QByteArray key;
    key.reserve(8 + depth * 4);
    key.append(kind);
    key.append(reinterpret_cast<const char *>(&depth), sizeof(int));
    key.append(reinterpret_cast<const char *>(&dataIndex), sizeof(int));
    if (depth > 0) key.append(reinterpret_cast<const char *>(path), depth * int(sizeof(int)));
    return key;
}

} // namespace

struct PivotLookup::Impl {
    std::shared_ptr<const PivotTable> t;
    std::shared_ptr<const PivotCache> c;
    std::shared_ptr<const PivotGrid> g;
    std::vector<LinePath> rowLines, colLines;
    QHash<QByteArray, int> rowAt, colAt;             // line keys -> the line
    QHash<QString, int> fieldByName;                  // lower case caption or name of a field -> the field
    std::vector<QHash<QString, int>> itemByKey;       // for a field on an axis or a page: item keys -> the item's place among the cache's
    int dRow = -1, dCol = -1;

    // (For a field on an axis or a page, the places of its items are made once, when the lookup is: a lookup asks for an item of a table of a million lines at each call.)
    std::vector<QVector<int>> posByShared, sharedByPos;
    std::vector<bool> placesMade;
    void makePlaces(int f)
    {
        const PivotField &pf = t->fields[f];
        QVector<int> &pos = posByShared[size_t(f)];
        QVector<int> &shared = sharedByPos[size_t(f)];
        pos.fill(-2, c->fields[f].items.size());   // (-2: not an item of the field's list)
        int p = 0;
        for (const PivotTableItem &it : pf.items) {
            if (!it.t.isEmpty()) continue;
            if (it.x >= 0 && it.x < pos.size() && pos[it.x] == -2) pos[it.x] = it.hidden ? -1 : p;
            shared.append(it.x);
            ++p;
        }
        placesMade[size_t(f)] = true;
    }
    int positionOf(int f, int shared) const
    {
        const PivotField &pf = t->fields[f];
        if (!pf.hasItems) return shared;
        if (size_t(f) < placesMade.size() && placesMade[size_t(f)]) return shared >= 0 && shared < posByShared[size_t(f)].size() && posByShared[size_t(f)][shared] != -2 ? posByShared[size_t(f)][shared] : -1;
        int p = 0;
        for (const PivotTableItem &it : pf.items) {
            if (!it.t.isEmpty()) continue;
            if (it.x == shared) return it.hidden ? -1 : p;
            ++p;
        }
        return -1;
    }
    int sharedOfPosition(int f, int position) const
    {
        const PivotField &pf = t->fields[f];
        if (!pf.hasItems) return position;
        if (size_t(f) < placesMade.size() && placesMade[size_t(f)]) return position >= 0 && position < sharedByPos[size_t(f)].size() ? sharedByPos[size_t(f)][position] : -1;
        int p = 0;
        for (const PivotTableItem &it : pf.items) {
            if (!it.t.isEmpty()) continue;
            if (p++ == position) return it.x;
        }
        return -1;
    }
    int dataIndexOf(const QString &name) const
    {
        for (int i = 0; i < t->dataFields.size(); ++i)
            if (t->dataFields[i].name.compare(name, Qt::CaseInsensitive) == 0) return i;
        int found = -1;
        for (int i = 0; i < t->dataFields.size(); ++i) {
            const int f = t->dataFields[i].fld;
            if (f >= 0 && f < c->fields.size() && fieldCaption(*t, *c, f).compare(name, Qt::CaseInsensitive) == 0) {
                if (found >= 0) return -1;   // (the name is the field of two values: it does not say which)
                found = i;
            }
        }
        return found;
    }
    // The line of an axis that the requirements name, or -1. `req[level]` is the place asked for at a level (-1: not asked).
    int findLine(const std::vector<LinePath> &lines, const QHash<QByteArray, int> &at, const QVector<int> &fields, std::vector<int> req, int dSlot, int d) const
    {
        const int levels = int(fields.size());
        if (levels == 0) return lines.empty() ? -1 : 0;
        int k = 0;
        while (k < levels && (req[size_t(k)] >= 0 || fields[k] == -2)) ++k;
        for (int l = k; l < levels; ++l)
            if (req[size_t(l)] >= 0) return -1;   // (a field asked for after one that was not: not a group the table has)
        for (int l = 0; l < k; ++l)
            if (fields[l] == -2) req[size_t(l)] = d;
        const char kind = k == 0 ? 'g' : (k == levels ? 'l' : 's');
        // (a line that does not name the values in its path is one for each value: the one of this value)
        const int dataIndex = dSlot >= 0 && dSlot >= k ? d : 0;
        const auto found = at.constFind(lineKey(kind, k, dataIndex, req.data()));
        return found == at.constEnd() ? -1 : found.value();
    }
};

PivotLookup::PivotLookup(std::shared_ptr<const PivotTable> table, std::shared_ptr<const PivotCache> cache, std::shared_ptr<const PivotGrid> grid) : d(new Impl)
{
    d->t = std::move(table);
    d->c = std::move(cache);
    d->g = std::move(grid);
    const PivotTable &t = *d->t;
    const PivotCache &c = *d->c;
    if (t.fields.size() != c.fields.size()) return;
    d->dRow = t.rowFields.indexOf(-2);
    d->dCol = t.colFields.indexOf(-2);
    d->rowLines = pathsOf(d->g->rowItems, int(t.rowFields.size()));
    d->colLines = pathsOf(d->g->colItems, int(t.colFields.size()));
    // (a line's key: the data index counts only where the path does not hold it, as findLine asks)
    auto indexAxis = [&](const std::vector<LinePath> &lines, int levels, int dSlot, QHash<QByteArray, int> *at) {
        if (levels == 0) return;
        for (size_t i = 0; i < lines.size(); ++i) {
            const LinePath &lp = lines[i];
            if (lp.labelOnly || lp.t == QLatin1String("blank")) continue;
            char kind;
            if (lp.t == QLatin1String("grand")) kind = 'g';
            else if (lp.t.isEmpty()) {
                if (lp.depth != levels) continue;
                kind = 'l';
            } else {
                if (lp.depth < 1 || lp.depth >= levels) continue;
                kind = 's';
            }
            const int dataIndex = dSlot >= 0 && dSlot >= lp.depth ? lp.dataIndex : 0;
            const QByteArray key = lineKey(kind, lp.depth, dataIndex, lp.path.data());
            if (!at->contains(key)) at->insert(key, int(i));
        }
    };
    indexAxis(d->rowLines, int(t.rowFields.size()), d->dRow, &d->rowAt);
    indexAxis(d->colLines, int(t.colFields.size()), d->dCol, &d->colAt);
    for (int f = 0; f < c.fields.size(); ++f) {
        const QString a = fieldCaption(t, c, f).toLower(), b = c.fields[f].name.toLower();
        if (!d->fieldByName.contains(a)) d->fieldByName.insert(a, f);
        if (!d->fieldByName.contains(b)) d->fieldByName.insert(b, f);
    }
    // the items of the fields a name can pick out (on an axis or a page)
    d->itemByKey.assign(size_t(c.fields.size()), QHash<QString, int>());
    QVector<bool> used(c.fields.size(), false);
    for (int f : t.rowFields) if (f >= 0) used[f] = true;
    for (int f : t.colFields) if (f >= 0) used[f] = true;
    for (const PivotPageField &p : t.pageFields) if (p.fld >= 0 && p.fld < used.size()) used[p.fld] = true;
    d->posByShared.assign(size_t(c.fields.size()), QVector<int>());
    d->sharedByPos.assign(size_t(c.fields.size()), QVector<int>());
    d->placesMade.assign(size_t(c.fields.size()), false);
    for (int f = 0; f < c.fields.size(); ++f)
        if (used[f] && t.fields[f].hasItems) d->makePlaces(f);
    for (int f = 0; f < c.fields.size(); ++f) {
        if (!used[f]) continue;
        QHash<QString, int> &map = d->itemByKey[size_t(f)];
        for (int i = 0; i < c.fields[f].items.size(); ++i) {
            const QString k = keyOf(c.fields[f].items[i]);
            if (!map.contains(k)) map.insert(k, i);
        }
    }
}

PivotLookup::~PivotLookup() = default;

bool PivotLookup::find(const QString &dataField, const std::vector<std::pair<QString, PivotValue>> &items, PivotCell *cell) const
{
    const PivotTable &t = *d->t;
    const PivotCache &c = *d->c;
    if (t.fields.size() != c.fields.size()) return false;
    const int dIndex = d->dataIndexOf(dataField);
    if (dIndex < 0) return false;
    std::vector<int> rowReq(size_t(t.rowFields.size()), -1), colReq(size_t(t.colFields.size()), -1);
    for (const auto &[name, asked] : items) {
        const auto field = d->fieldByName.constFind(name.toLower());
        if (field == d->fieldByName.constEnd()) return false;
        const int f = field.value();
        int shared = -1;
        for (const QString &key : keysAsked(asked)) {
            const auto at = d->itemByKey[size_t(f)].constFind(key);
            if (at != d->itemByKey[size_t(f)].constEnd()) {
                shared = at.value();
                break;
            }
        }
        if (shared < 0) {
            // (a field that is not on an axis or a page has no list kept: look at its items)
            if (d->itemByKey[size_t(f)].isEmpty())
                for (int i = 0; i < c.fields[f].items.size() && shared < 0; ++i)
                    for (const QString &key : keysAsked(asked))
                        if (keyOf(c.fields[f].items[i]) == key) {
                            shared = i;
                            break;
                        }
            if (shared < 0) return false;
        }
        const int pos = d->positionOf(f, shared);
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
    }
    const int ri = d->findLine(d->rowLines, d->rowAt, t.rowFields, rowReq, d->dRow, dIndex), ci = d->findLine(d->colLines, d->colAt, t.colFields, colReq, d->dCol, dIndex);
    if (ri < 0 || ci < 0) return false;
    const PivotGrid &g = *d->g;
    const int r = g.firstDataRow + ri, k = g.firstDataCol + ci;
    if (r < 0 || r >= g.rows || k < 0 || k >= g.cols) return false;
    const PivotCell &found = g.at(r, k);
    if (found.kind == PivotCell::Empty) return false;
    *cell = found;
    return true;
}

bool PivotLookup::arguments(int gridRow, int gridCol, QString *dataField, std::vector<std::pair<QString, PivotValue>> *items) const
{
    const PivotTable &t = *d->t;
    const PivotCache &c = *d->c;
    const PivotGrid &g = *d->g;
    if (t.fields.size() != c.fields.size() || t.dataFields.isEmpty()) return false;
    const int ri = gridRow - g.firstDataRow, ci = gridCol - g.firstDataCol;
    if (ri < 0 || ci < 0 || ri >= g.rowItems.size() || ci >= g.colItems.size() || gridRow >= g.rows || gridCol >= g.cols) return false;
    const PivotAxisItem &rowItem = g.rowItems[ri], &colItem = g.colItems[ci];
    if (rowItem.labelOnly || colItem.labelOnly || rowItem.t == QLatin1String("blank") || colItem.t == QLatin1String("blank")) return false;
    if (!g.at(gridRow, gridCol).isValue) return false;
    const LinePath &rp = d->rowLines[size_t(ri)], &cp = d->colLines[size_t(ci)];
    int dIndex = 0;
    if (d->dRow >= 0) dIndex = d->dRow < rp.depth ? rp.path[size_t(d->dRow)] : rp.dataIndex;
    else if (d->dCol >= 0) dIndex = d->dCol < cp.depth ? cp.path[size_t(d->dCol)] : cp.dataIndex;
    if (dIndex < 0 || dIndex >= t.dataFields.size()) return false;
    *dataField = t.dataFields[dIndex].name;
    items->clear();
    auto add = [&](const QVector<int> &fields, const LinePath &lp) {
        for (int l = 0; l < lp.depth && l < fields.size(); ++l) {
            const int f = fields[l];
            if (f < 0) continue;
            const int shared = d->sharedOfPosition(f, lp.path[size_t(l)]);
            if (shared < 0 || shared >= c.fields[f].items.size()) return false;
            items->emplace_back(fieldCaption(t, c, f), c.fields[f].items[shared]);
        }
        return true;
    };
    return add(t.rowFields, rp) && add(t.colFields, cp);
}

bool pivotGetData(const PivotTable &t, const PivotCache &c, const PivotGrid &g, const QString &dataField, const std::vector<std::pair<QString, PivotValue>> &items, PivotCell *cell)
{
    return PivotLookup(std::make_shared<PivotTable>(t), std::make_shared<PivotCache>(c), std::make_shared<PivotGrid>(g)).find(dataField, items, cell);
}

bool pivotGetDataArguments(const PivotTable &t, const PivotCache &c, const PivotGrid &g, int gridRow, int gridCol, QString *dataField, std::vector<std::pair<QString, PivotValue>> *items)
{
    return PivotLookup(std::make_shared<PivotTable>(t), std::make_shared<PivotCache>(c), std::make_shared<PivotGrid>(g)).arguments(gridRow, gridCol, dataField, items);
}

} // namespace jp::office::sheet

namespace jp::office::sheet {

bool pivotDetailRecords(const PivotTable &t, const PivotCache &c, const PivotGrid &grid, int gridRow, int gridCol, QVector<qint64> *records)
{
    records->clear();
    QString dataField;
    std::vector<std::pair<QString, PivotValue>> items;
    if (!pivotGetDataArguments(t, c, grid, gridRow, gridCol, &dataField, &items)) return false;
    const int nf = int(c.fields.size());
    const qint64 n = nf > 0 ? c.fields[0].records.size() : 0;
    auto fieldOf = [&](const QString &name) {
        for (int f = 0; f < nf && f < t.fields.size(); ++f)
            if ((t.fields[f].name.isEmpty() ? c.fields[f].name : t.fields[f].name) == name) return f;
        return -1;
    };
    // the places of the items the table shows, for a field with a list of items: a record in a hidden item is not there
    auto shownItem = [&](int f, qint32 x) {
        const PivotField &pf = t.fields[f];
        if (!pf.hasItems) return true;
        for (const PivotTableItem &it : pf.items)
            if (it.t.isEmpty() && it.x == x) return !it.hidden;
        return false;
    };
    QVector<QPair<int, PivotValue>> wanted;
    for (const auto &[name, value] : items) {
        const int f = fieldOf(name);
        if (f < 0) return false;
        wanted.append({f, value});
    }
    for (qint64 r = 0; r < n; ++r) {
        bool in = true;
        for (int f : t.rowFields)
            if (f >= 0 && !shownItem(f, c.fields[f].records[r])) in = false;
        for (int f : t.colFields)
            if (f >= 0 && !shownItem(f, c.fields[f].records[r])) in = false;
        for (const PivotPageField &p : t.pageFields) {
            const PivotField &pf = t.fields[p.fld];
            const qint32 x = c.fields[p.fld].records[r];
            if (!shownItem(p.fld, x)) in = false;
            if (p.item >= 0) {
                int want = p.item;
                if (pf.hasItems) {
                    int pos = 0;
                    want = -1;
                    for (const PivotTableItem &it : pf.items) {
                        if (!it.t.isEmpty()) continue;
                        if (pos++ == p.item) want = it.x;
                    }
                }
                if (x != want) in = false;
            }
        }
        for (int k = 0; k < wanted.size() && in; ++k) {
            const PivotValue &have = c.fields[wanted[k].first].items[c.fields[wanted[k].first].records[r]];
            const PivotValue &want = wanted[k].second;
            const bool same = have.isNumeric() && want.isNumeric() ? have.number == want.number
                              : have.kind == want.kind && (have.kind == PivotValue::Missing || have.number == want.number) && (have.kind != PivotValue::Text || have.text.compare(want.text, Qt::CaseInsensitive) == 0);
            if (!same) in = false;
        }
        if (in) records->append(r);
    }
    return true;
}

} // namespace jp::office::sheet
