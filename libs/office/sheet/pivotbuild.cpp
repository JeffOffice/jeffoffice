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
        pf.numFmtId = p >= 0 ? prev.fields[p].numFmtId : (f < src.numFmtIds.size() ? src.numFmtIds[f] : 0);
        Distinct dist;
        QVector<qint32> temp;
        temp.reserve(int(nRows));
        bool hasText = false, longText = false;
        for (qint64 r = 0; r < nRows; ++r) {
            const PivotValue v = src.cell(src.row1 + 1 + quint32(r), src.col1 + quint32(f));
            longText = longText || (v.kind == PivotValue::Text && v.text.size() > 255);
            hasText = hasText || v.kind == PivotValue::Text || v.kind == PivotValue::Bool || v.kind == PivotValue::Error;
            temp.append(dist.idOf(v));
        }
        distinctAll += dist.values.size();
        if (dist.values.size() > kPivotMaxItemsPerField || distinctAll > 12000000) return fail(QStringLiteral("the source has too many different values"));
        // (a field keeps being shared or not as it was, as the original program keeps it; a new field of text is shared, unless a text is too long for a shared item (more than 255 characters))
        const bool onAxis = p >= 0 && src.axisFields.contains(p);
        pf.hasShared = onAxis || (!longText && (p >= 0 ? prev.fields[p].storedHasShared() : hasText));
        // the order: the shared items of the previous cache that remain (in their order), then the others as they first appear
        QVector<qint32> order;
        order.reserve(dist.values.size());
        QVector<qint32> newId(dist.values.size(), -1);
        if (p >= 0 && pf.hasShared) {
            const bool grouped = prev.fields[p].group.kind != PivotFieldGroup::None;   // (the places of the groups are worked out below)
            for (int i = 0; i < prev.fields[p].storedSharedCount(); ++i) {
                // find the previous item among the new values
                const PivotValue &pv = prev.fields[p].storedItems()[i];
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
                    if (!grouped) out->remap[p][i] = newId[id];
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
    // the groups the previous cache had are made again from the new values (a field added for a group goes after the others)
    bool anyGroup = false;
    for (const PivotCacheField &pf : prev.fields) anyGroup = anyGroup || pf.group.kind != PivotFieldGroup::None;
    if (anyGroup) {
        QVector<int> newOfPrev(prev.fields.size(), -1);
        for (int f = 0; f < out->fields.size(); ++f)
            if (out->fromField[f] >= 0) newOfPrev[out->fromField[f]] = f;
        for (int p = 0; p < prev.fields.size(); ++p) {
            const PivotFieldGroup &pg = prev.fields[p].group;
            if (pg.kind == PivotFieldGroup::None || pg.databaseField) continue;
            if (out->fields.size() >= kPivotMaxFields) return fail(QStringLiteral("too many fields"));
            PivotCacheField nf;
            nf.name = prev.fields[p].name;
            nf.numFmtId = prev.fields[p].numFmtId;
            out->fields.append(nf);
            out->fromField.append(p);
            newOfPrev[p] = int(out->fields.size()) - 1;
        }
        for (int f = 0; f < out->fields.size(); ++f) {
            const int p = out->fromField[f];
            if (p < 0 || prev.fields[p].group.kind == PivotFieldGroup::None) continue;
            PivotFieldGroup g = prev.fields[p].group;
            if (g.kind == PivotFieldGroup::Discrete) return fail(QStringLiteral("a field has groups of items that were made by hand, which can't be read again yet"));
            g.labels.clear();
            g.base = g.base >= 0 && g.base < newOfPrev.size() ? newOfPrev[g.base] : -1;
            g.par = g.par >= 0 && g.par < newOfPrev.size() ? newOfPrev[g.par] : -1;
            if (g.base < 0) return fail(QStringLiteral("the field a group is made from is not in the data any more"));
            out->fields[f].group = g;
        }
        for (int pass = 0; pass < 2; ++pass)
            for (int f = 0; f < out->fields.size(); ++f) {
                if (out->fields[f].group.kind == PivotFieldGroup::None || out->fields[f].group.databaseField != (pass == 0)) continue;
                QString problem;
                if (!pivotApplyGroup(out, f, src.dates, &problem)) return fail(QStringLiteral("a group can't be made again: %1").arg(problem));
            }
        // each group item that was is where its label is now
        for (int f = 0; f < out->fields.size(); ++f) {
            const int p = out->fromField[f];
            if (p < 0 || prev.fields[p].group.kind == PivotFieldGroup::None) continue;
            for (int i = 0; i < prev.fields[p].items.size(); ++i)
                for (int k = 0; k < out->fields[f].items.size(); ++k)
                    if (out->fields[f].items[k].text == prev.fields[p].items[i].text) {
                        out->remap[p][i] = k;
                        break;
                    }
        }
    }
    pivotKeepNumericFlags(out);
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
            if (!old.fields[pg.fld].hasItems) oldX = pg.item;   // (a field with no list of items: the place is the shared item's)
            else
                for (const PivotTableItem &it : old.fields[pg.fld].items) {
                    if (!it.t.isEmpty()) continue;
                    if (pos++ == pg.item) oldX = it.x;
                }
            const int p = pg.fld;
            const QVector<qint32> &map = p < fresh.remap.size() ? fresh.remap[p] : QVector<qint32>();
            const int newX = oldX >= 0 && oldX < map.size() ? map[oldX] : -1;
            n.item = -1;
            pos = 0;
            if (!t->fields[n.fld].hasItems) n.item = newX >= 0 ? newX : -1;
            else
                for (const PivotTableItem &it : t->fields[n.fld].items) {
                    if (!it.t.isEmpty()) continue;
                    if (it.x == newX && newX >= 0) n.item = pos;
                    ++pos;
                }
        }
        t->pageFields.append(n);
    }
    pivotRemapDataFields(old, t);
    pivotSyncRoles(t);
    Q_UNUSED(prev)
}

void pivotRemapDataFields(const PivotTable &old, PivotTable *now)
{
    QVector<int> map(old.dataFields.size(), -1);
    QVector<bool> taken(now->dataFields.size(), false);
    for (int pass = 0; pass < 3; ++pass)
        for (int i = 0; i < old.dataFields.size(); ++i) {
            if (map[i] >= 0) continue;
            for (int k = 0; k < now->dataFields.size(); ++k) {
                if (taken[k]) continue;
                // (by name; else by field and function; else, when a value was renamed and its function changed in one step, by the field alone)
                const bool same = pass == 0 ? now->dataFields[k].name == old.dataFields[i].name
                                            : (pass == 1 ? (now->dataFields[k].fld == old.dataFields[i].fld && now->dataFields[k].subtotal == old.dataFields[i].subtotal) : now->dataFields[k].fld == old.dataFields[i].fld);
                if (!same) continue;
                map[i] = k;
                taken[k] = true;
                break;
            }
        }
    for (PivotField &f : now->fields)
        if (f.sortByData >= 0) f.sortByData = f.sortByData < map.size() ? map[f.sortByData] : -1;
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

namespace jp::office::sheet {

void pivotNormalizeLayout(PivotTable *t)
{
    const int D = int(t->dataFields.size());
    const bool inRows = t->rowFields.contains(-2), inCols = t->colFields.contains(-2);
    if (inRows && inCols) {
        t->colFields.removeAll(-2);
    }
    if (D <= 1) {
        t->rowFields.removeAll(-2);
        t->colFields.removeAll(-2);
    } else if (!t->rowFields.contains(-2) && !t->colFields.contains(-2)) {
        (t->dataOnRows ? t->rowFields : t->colFields).append(-2);
    }
    if (t->rowFields.contains(-2)) t->dataOnRows = true;
    else if (t->colFields.contains(-2)) t->dataOnRows = false;
}

bool pivotFieldIsNumeric(const PivotCache &c, int f)
{
    if (f < 0 || f >= c.fields.size()) return false;
    const PivotCacheField &cf = c.fields[f];
    if (cf.numericKnown >= 0) return cf.numericKnown != 0;
    bool any = false;
    for (const PivotValue &v : cf.storedItems()) {
        if (v.kind == PivotValue::Missing) continue;
        if (v.kind != PivotValue::Number) return false;
        any = true;
    }
    return any;
}

void pivotKeepNumericFlags(PivotCache *c)
{
    for (int f = 0; f < c->fields.size(); ++f) {
        c->fields[f].numericKnown = -1;
        c->fields[f].numericKnown = pivotFieldIsNumeric(*c, f) ? 1 : 0;
    }
}

bool pivotPrepareFields(PivotTable *t, PivotCache *c)
{
    bool changed = false;
    auto prepare = [&](int f) {
        if (f < 0 || f >= c->fields.size() || f >= t->fields.size()) return;
        PivotCacheField &cf = c->fields[f];
        if (!cf.hasShared) {
            // (a text that differs from another only in case is the same item to the original program: those are one item)
            QHash<QString, qint32> first;
            QVector<qint32> newId(cf.items.size());
            QVector<PivotValue> items;
            bool merged = false;
            for (qint32 i = 0; i < cf.items.size(); ++i) {
                if (cf.items[i].kind == PivotValue::Text) {
                    const QString folded = cf.items[i].text.toCaseFolded();
                    const auto it = first.constFind(folded);
                    if (it != first.constEnd()) {
                        newId[i] = it.value();
                        merged = true;
                        continue;
                    }
                    first.insert(folded, qint32(items.size()));
                }
                newId[i] = qint32(items.size());
                items.append(cf.items[i]);
            }
            if (merged) {
                cf.items = items;
                for (qint32 &r : cf.records) r = newId[r];
            }
            cf.hasShared = true;
            cf.sharedCount = int(cf.items.size());
            cf.numericKnown = -1;   // (the items were merged: worked out again below, by the caller of this, which owns the cache)
            cf.numericKnown = pivotFieldIsNumeric(*c, f) ? 1 : 0;
            changed = true;
        }
        PivotField &pf = t->fields[f];
        if (pf.hasItems) return;
        QVector<qint32> order(cf.sharedCount);
        for (qint32 i = 0; i < cf.sharedCount; ++i) order[i] = i;
        // (the groups of a field are in the order the grouping has: January before February, 1-10 before 11-20)
        if (cf.group.kind == PivotFieldGroup::None) std::stable_sort(order.begin(), order.end(), [&](qint32 a, qint32 b) { return pivotValueLess(cf.items[a], cf.items[b]); });
        pf.items.clear();
        for (qint32 x : order) {
            PivotTableItem it;
            it.x = x;
            pf.items.append(it);
        }
        if (pf.defaultSubtotal) {
            PivotTableItem total;
            total.t = QStringLiteral("default");
            pf.items.append(total);
        }
        pf.hasItems = true;
    };
    for (int f : t->rowFields) prepare(f);
    for (int f : t->colFields) prepare(f);
    for (const PivotPageField &p : t->pageFields) prepare(p.fld);
    return changed;
}

bool pivotNeedsSharedItems(const PivotTable &t, const PivotCache &c)
{
    auto needs = [&](int f) { return f >= 0 && f < c.fields.size() && f < t.fields.size() && !c.fields[f].hasShared; };
    for (int f : t.rowFields)
        if (needs(f)) return true;
    for (int f : t.colFields)
        if (needs(f)) return true;
    for (const PivotPageField &p : t.pageFields)
        if (needs(p.fld)) return true;
    return false;
}

QString pivotFunctionName(const QString &fn)
{
    if (fn == QLatin1String("sum")) return QStringLiteral("Sum");
    if (fn == QLatin1String("count")) return QStringLiteral("Count");
    if (fn == QLatin1String("countNums")) return QStringLiteral("Count Numbers");
    if (fn == QLatin1String("average")) return QStringLiteral("Average");
    if (fn == QLatin1String("max")) return QStringLiteral("Max");
    if (fn == QLatin1String("min")) return QStringLiteral("Min");
    if (fn == QLatin1String("product")) return QStringLiteral("Product");
    if (fn == QLatin1String("stdDev")) return QStringLiteral("StdDev");
    if (fn == QLatin1String("stdDevp")) return QStringLiteral("StdDevp");
    if (fn == QLatin1String("var")) return QStringLiteral("Var");
    if (fn == QLatin1String("varp")) return QStringLiteral("Varp");
    return QStringLiteral("Sum");
}

QString pivotDataFieldName(const PivotTable &t, const PivotCache &c, int f, const QString &fn)
{
    const QString field = f >= 0 && f < t.fields.size() && !t.fields[f].name.isEmpty() ? t.fields[f].name : (f >= 0 && f < c.fields.size() ? c.fields[f].name : QString());
    QString name = pivotFunctionName(fn) + QStringLiteral(" of ") + field;
    auto taken = [&](const QString &n) {
        for (int i = 0; i < c.fields.size(); ++i)
            if (c.fields[i].name.compare(n, Qt::CaseInsensitive) == 0 || (i < t.fields.size() && t.fields[i].name.compare(n, Qt::CaseInsensitive) == 0)) return true;
        for (const PivotDataField &d : t.dataFields)
            if (d.name.compare(n, Qt::CaseInsensitive) == 0) return true;
        return false;
    };
    while (taken(name)) name += QLatin1Char(' ');
    return name;
}

} // namespace jp::office::sheet

namespace jp::office::sheet {

void pivotSortField(PivotTable *t, const PivotCache &cache, int field, PivotSort how)
{
    if (field < 0 || field >= t->fields.size() || field >= cache.fields.size()) return;
    PivotField &pf = t->fields[field];
    pf.sortByData = -1;
    pf.sortType = how == PivotSort::Ascending ? QStringLiteral("ascending") : how == PivotSort::Descending ? QStringLiteral("descending") : QStringLiteral("manual");
    if (how == PivotSort::Manual) return;
    const PivotCacheField &cf = cache.fields[field];
    if (!pf.hasItems) {
        // (a field with no list of items has all of the cache's, which are listed now so that the order is kept)
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
    QVector<PivotTableItem> data, totals;
    for (const PivotTableItem &it : pf.items) (it.t.isEmpty() ? data : totals).append(it);
    const bool desc = how == PivotSort::Descending;
    std::stable_sort(data.begin(), data.end(), [&](const PivotTableItem &a, const PivotTableItem &b) {
        if (a.x < 0 || a.x >= cf.items.size() || b.x < 0 || b.x >= cf.items.size()) return a.x < b.x;
        return desc ? pivotValueLess(cf.items[b.x], cf.items[a.x]) : pivotValueLess(cf.items[a.x], cf.items[b.x]);
    });
    pf.items = data + totals;
}

bool pivotSortFieldByValue(PivotTable *t, int field, int dataField, bool descending)
{
    if (field < 0 || field >= t->fields.size() || dataField < 0 || dataField >= t->dataFields.size()) return false;
    PivotField &pf = t->fields[field];
    pf.sortType = descending ? QStringLiteral("descending") : QStringLiteral("ascending");
    pf.sortByData = dataField;
    return true;
}

} // namespace jp::office::sheet
