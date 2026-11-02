#include "sheet/pivot.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace jp::office::sheet {

bool pivotValueLess(const PivotValue &a, const PivotValue &b)
{
    auto rank = [](const PivotValue &v) {
        switch (v.kind) {
        case PivotValue::Number:
        case PivotValue::Date: return 0;
        case PivotValue::Text: return 1;
        case PivotValue::Bool: return 2;
        case PivotValue::Error: return 3;
        case PivotValue::Missing: return 4;
        }
        return 5;
    };
    const int ra = rank(a), rb = rank(b);
    if (ra != rb) return ra < rb;
    if (ra == 0) return a.number < b.number;
    if (ra == 1 || ra == 3) {
        const int c = a.text.compare(b.text, Qt::CaseInsensitive);
        return c != 0 ? c < 0 : a.text < b.text;
    }
    if (ra == 2) return a.number < b.number;
    return false;
}

namespace {

// The text of a header cell as a field's name.
QString headerText(const PivotValue &v)
{
    switch (v.kind) {
    case PivotValue::Text:
    case PivotValue::Error: return v.text;
    case PivotValue::Number:
    case PivotValue::Date: return QString::number(v.number, 'g', 15);
    case PivotValue::Bool: return v.number != 0 ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
    case PivotValue::Missing: break;
    }
    return QString();
}

struct Distinct {
    std::unordered_map<double, qint32> numbers, dates;
    QHash<QString, qint32> texts, errors;   // (text by its case-folded form: the program treats "a" and "A" as one item)
    qint32 bools[2] = {-1, -1}, missing = -1;
    QVector<PivotValue> values;
    QVector<qint32> count;
    qint32 idOf(const PivotValue &v)
    {
        qint32 *slot = nullptr;
        qint32 found = -1;
        switch (v.kind) {
        case PivotValue::Missing: slot = &missing; break;
        case PivotValue::Bool: slot = &bools[v.number != 0]; break;
        case PivotValue::Number: {
            const auto it = numbers.find(v.number);
            if (it != numbers.end()) found = it->second;
            break;
        }
        case PivotValue::Date: {
            const auto it = dates.find(v.number);
            if (it != dates.end()) found = it->second;
            break;
        }
        case PivotValue::Text: {
            const auto it = texts.constFind(v.text.toCaseFolded());
            if (it != texts.constEnd()) found = it.value();
            break;
        }
        case PivotValue::Error: {
            const auto it = errors.constFind(v.text);
            if (it != errors.constEnd()) found = it.value();
            break;
        }
        }
        if (slot) found = *slot;
        if (found >= 0) {
            ++count[found];
            return found;
        }
        const qint32 id = qint32(values.size());
        values.append(v);
        count.append(1);
        switch (v.kind) {
        case PivotValue::Missing:
        case PivotValue::Bool: *slot = id; break;
        case PivotValue::Number: numbers.emplace(v.number, id); break;
        case PivotValue::Date: dates.emplace(v.number, id); break;
        case PivotValue::Text: texts.insert(v.text.toCaseFolded(), id); break;
        case PivotValue::Error: errors.insert(v.text, id); break;
        }
        return id;
    }
};

} // namespace

bool buildPivotCache(const PivotSource &src, const PivotCache &prev, PivotCache *out, QString *why)
{
    *out = PivotCache();
    auto fail = [&](const QString &m) {
        if (why) *why = m;
        return false;
    };
    if (src.row2 <= src.row1 || src.col2 < src.col1) return fail(QStringLiteral("the source needs a row of names and at least one row of data"));
    const qint64 nCols = qint64(src.col2) - src.col1 + 1, nRows = qint64(src.row2) - src.row1;
    if (nCols > kPivotMaxFields) return fail(QStringLiteral("the source has too many columns"));
    if (nRows > kPivotMaxRecords || nRows * nCols > kPivotMaxRecordCells) return fail(QStringLiteral("the source has too many cells"));
    out->sourceType = prev.sourceType;
    out->sheet = prev.sheet;
    out->ref = prev.ref;
    out->name = prev.name;
    out->saveData = prev.saveData;
    out->refreshOnLoad = prev.refreshOnLoad;
    out->recordCount = nRows;
    out->fields.resize(int(nCols));
    out->fromField.assign(int(nCols), -1);
    out->remap.resize(prev.fields.size());
    for (int p = 0; p < prev.fields.size(); ++p) out->remap[p].assign(prev.fields[p].items.size(), -1);
    // names
    QSet<QString> taken;
    for (int f = 0; f < nCols; ++f) {
        QString name = headerText(src.cell(src.row1, src.col1 + quint32(f)));
        if (name.isEmpty()) name = QStringLiteral("Column%1").arg(f + 1);
        QString use = name;
        for (int n = 2; taken.contains(use.toCaseFolded()); ++n) use = name + QString::number(n);
        taken.insert(use.toCaseFolded());
        out->fields[f].name = use;
        for (int p = 0; p < prev.fields.size(); ++p)
            if (prev.fields[p].name.compare(use, Qt::CaseInsensitive) == 0) {
                out->fromField[f] = p;
                break;
            }
    }
    qint64 distinctAll = 0;
    for (int f = 0; f < nCols; ++f) {
        PivotCacheField &pf = out->fields[f];
        const int p = out->fromField[f];
        pf.numFmtId = f < src.numFmtIds.size() ? src.numFmtIds[f] : (p >= 0 ? prev.fields[p].numFmtId : 0);
        Distinct dist;
        QVector<qint32> temp;
        temp.reserve(int(nRows));
        bool hasText = false;
        for (qint64 r = 0; r < nRows; ++r) {
            const PivotValue v = src.cell(src.row1 + 1 + quint32(r), src.col1 + quint32(f));
            hasText = hasText || v.kind == PivotValue::Text || v.kind == PivotValue::Bool || v.kind == PivotValue::Error;
            temp.append(dist.idOf(v));
        }
        distinctAll += dist.values.size();
        if (dist.values.size() > kPivotMaxItemsPerField || distinctAll > 12000000) return fail(QStringLiteral("the source has too many different values"));
        pf.hasShared = (p >= 0 && src.axisFields.contains(p)) || hasText || (p >= 0 && prev.fields[p].hasShared);
        // the order: the shared items of the previous cache that remain (in their order), then the others as they first appear
        QVector<qint32> order;
        order.reserve(dist.values.size());
        QVector<qint32> newId(dist.values.size(), -1);
        if (p >= 0 && pf.hasShared) {
            for (int i = 0; i < prev.fields[p].sharedCount; ++i) {
                // find the previous item among the new values
                const PivotValue &pv = prev.fields[p].items[i];
                qint32 id = -1;
                switch (pv.kind) {
                case PivotValue::Missing: id = dist.missing; break;
                case PivotValue::Bool: id = dist.bools[pv.number != 0]; break;
                case PivotValue::Number: { const auto it = dist.numbers.find(pv.number); if (it != dist.numbers.end()) id = it->second; break; }
                case PivotValue::Date: { const auto it = dist.dates.find(pv.number); if (it != dist.dates.end()) id = it->second; break; }
                case PivotValue::Text: { const auto it = dist.texts.constFind(pv.text.toCaseFolded()); if (it != dist.texts.constEnd()) id = it.value(); break; }
                case PivotValue::Error: { const auto it = dist.errors.constFind(pv.text); if (it != dist.errors.constEnd()) id = it.value(); break; }
                }
                if (id >= 0 && newId[id] < 0) {
                    newId[id] = qint32(order.size());
                    order.append(id);
                    out->remap[p][i] = newId[id];
                }
            }
        }
        for (qint32 i = 0; i < dist.values.size(); ++i)
            if (newId[i] < 0) {
                newId[i] = qint32(order.size());
                order.append(i);
            }
        pf.items.reserve(order.size());
        for (qint32 id : order) pf.items.append(dist.values[id]);
        pf.sharedCount = pf.hasShared ? int(pf.items.size()) : 0;
        pf.records.resize(temp.size());
        for (qsizetype i = 0; i < temp.size(); ++i) pf.records[i] = newId[temp[i]];
    }
    return true;
}

void matchPivotItems(PivotTable *t, const PivotCache &prev, const PivotCache &fresh)
{
    const PivotTable old = *t;
    const int nf = int(fresh.fields.size());
    t->fields.clear();
    QVector<int> newOfOld(old.fields.size(), -1);
    for (int f = 0; f < nf; ++f) {
        const int p = f < fresh.fromField.size() ? fresh.fromField[f] : -1;
        if (p >= 0 && p < old.fields.size()) {
            t->fields.append(old.fields[p]);
            newOfOld[p] = f;
        } else {
            t->fields.append(PivotField());
        }
    }
    auto mapList = [&](const QVector<int> &list) {
        QVector<int> out;
        for (int f : list) {
            if (f == -2) out.append(-2);
            else if (f >= 0 && f < newOfOld.size() && newOfOld[f] >= 0) out.append(newOfOld[f]);
        }
        return out;
    };
    t->rowFields = mapList(old.rowFields);
    t->colFields = mapList(old.colFields);
    t->pageFields.clear();
    t->dataFields.clear();
    for (const PivotDataField &d : old.dataFields)
        if (d.fld >= 0 && d.fld < newOfOld.size() && newOfOld[d.fld] >= 0) {
            PivotDataField nd = d;
            nd.fld = newOfOld[d.fld];
            t->dataFields.append(nd);
        }
    for (int f = 0; f < nf; ++f) {
        const int p = fresh.fromField[f];
        if (p < 0 || p >= old.fields.size()) {
            continue;
        }
        const PivotField &of = old.fields[p];
        PivotField &nf2 = t->fields[f];
        nf2.dataField = false;
        if (!of.hasItems) continue;
        const QVector<qint32> &map = p < fresh.remap.size() ? fresh.remap[p] : QVector<qint32>();
        QVector<PivotTableItem> items, totals;
        QSet<int> used;
        for (const PivotTableItem &it : of.items) {
            if (!it.t.isEmpty()) {
                totals.append(it);
                continue;
            }
            if (it.missing || it.x < 0 || it.x >= map.size() || map[it.x] < 0) continue;
            PivotTableItem n = it;
            n.x = map[it.x];
            used.insert(n.x);
            items.append(n);
        }
        const PivotCacheField &cf = fresh.fields[f];
        for (int i = 0; i < cf.sharedCount; ++i)
            if (!used.contains(i)) {
                PivotTableItem n;
                n.x = i;
                items.append(n);
            }
        if (of.sortType == QLatin1String("ascending") || of.sortType == QLatin1String("descending")) {
            const bool desc = of.sortType == QLatin1String("descending");
            std::stable_sort(items.begin(), items.end(), [&](const PivotTableItem &a, const PivotTableItem &b) {
                const PivotValue &va = cf.items[a.x], &vb = cf.items[b.x];
                return desc ? pivotValueLess(vb, va) : pivotValueLess(va, vb);
            });
        }
        nf2.items = items + totals;
        nf2.hasItems = true;
    }
    // the filters
    for (const PivotPageField &pg : old.pageFields) {
        if (pg.fld < 0 || pg.fld >= newOfOld.size() || newOfOld[pg.fld] < 0) continue;
        PivotPageField n = pg;
        n.fld = newOfOld[pg.fld];
        if (pg.item >= 0) {
            int oldX = -1, pos = 0;
            for (const PivotTableItem &it : old.fields[pg.fld].items) {
                if (!it.t.isEmpty()) continue;
                if (pos++ == pg.item) oldX = it.x;
            }
            const int p = pg.fld;
            const QVector<qint32> &map = p < fresh.remap.size() ? fresh.remap[p] : QVector<qint32>();
            const int newX = oldX >= 0 && oldX < map.size() ? map[oldX] : -1;
            n.item = -1;
            pos = 0;
            for (const PivotTableItem &it : t->fields[n.fld].items) {
                if (!it.t.isEmpty()) continue;
                if (it.x == newX && newX >= 0) n.item = pos;
                ++pos;
            }
        }
        t->pageFields.append(n);
    }
    pivotSyncRoles(t);
    Q_UNUSED(prev)
}

void pivotSyncRoles(PivotTable *t)
{
    for (PivotField &f : t->fields) {
        f.axis.clear();
        f.dataField = false;
    }
    for (int f : t->rowFields)
        if (f >= 0 && f < t->fields.size()) t->fields[f].axis = QStringLiteral("axisRow");
    for (int f : t->colFields)
        if (f >= 0 && f < t->fields.size()) t->fields[f].axis = QStringLiteral("axisCol");
    for (const PivotPageField &p : t->pageFields)
        if (p.fld >= 0 && p.fld < t->fields.size()) t->fields[p.fld].axis = QStringLiteral("axisPage");
    for (const PivotDataField &d : t->dataFields)
        if (d.fld >= 0 && d.fld < t->fields.size()) t->fields[d.fld].dataField = true;
}

} // namespace jp::office::sheet
