// Slicers: what the pane shows of them, and the click that chooses items.
#include "core/editsession.h"

#include "sheet/slicer.h"

#include <QCoreApplication>
#include <QUndoStack>

#include <algorithm>

namespace jp::crunch {

using namespace jp::office::sheet;

namespace {

QString itemText(const PivotValue &v)
{
    switch (v.kind) {
    case PivotValue::Missing: return QCoreApplication::translate("jp::crunch::EditSession", "(blank)");
    case PivotValue::Text:
    case PivotValue::Error: return v.text;
    case PivotValue::Bool: return v.number != 0 ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
    default: return QString::number(v.number, 'g', 15);
    }
}

// The look the part of a slicer's own sheet gives the cache's slicer (its caption and the rest), if it has one.
const SlicerView *viewOf(const QVector<SlicerView> &views, const SlicerCache &sc)
{
    for (const SlicerView &v : views)
        if (v.cache == sc.name && v.timeline == sc.timeline) return &v;
    return nullptr;
}

// The order of two items for a sorted slicer: numbers, then text (without regard to case), then truth values, errors, and blanks.
int rankOf(const PivotValue &v)
{
    switch (v.kind) {
    case PivotValue::Number:
    case PivotValue::Date: return 0;
    case PivotValue::Text: return 1;
    case PivotValue::Bool: return 2;
    case PivotValue::Error: return 3;
    default: return 4;
    }
}

bool itemLess(const PivotValue &a, const PivotValue &b)
{
    const int ra = rankOf(a), rb = rankOf(b);
    if (ra != rb) return ra < rb;
    if (ra == 0 || ra == 2) return a.number < b.number;
    return ra == 1 || ra == 3 ? QString::compare(a.text, b.text, Qt::CaseInsensitive) < 0 : false;
}

} // namespace

std::vector<EditSession::SlicerInfo> EditSession::slicers() const
{
    std::vector<SlicerInfo> out;
    if (!m_book || !m_book->book) return out;
    const Workbook &book = *m_book->book;
    QVector<SlicerView> views;
    for (int s = 0; s < book.info().sheets.size(); ++s) views += book.slicerViews(s);
    for (const SlicerCache &sc : book.slicerCaches()) {
        SlicerInfo info;
        info.cache = sc;
        info.caption = sc.sourceName;
        for (const SlicerView &v : views)
            if (v.cache == sc.name && v.timeline == sc.timeline) {
                if (!v.caption.isEmpty()) info.caption = v.caption;
                info.style = v.style;
                info.columns = v.columns;
                break;
            }
        const auto targets = slicerTargets(book, sc);
        const PivotBook &pb = book.pivots();
        for (const auto &t : targets) info.tables << pb.table(t.first).table->name;
        if (!targets.isEmpty()) {
            const PivotTableEntry &entry = pb.table(targets[0].first);
            const PivotCache &cache = *pb.cache(entry.cache).cache;
            info.field = targets[0].second;
            for (const PivotValue &v : cache.fields[info.field].items) info.itemTexts << itemText(v);
            info.shown = pivotShownItems(*entry.table, cache, info.field);
        }
        out.push_back(std::move(info));
    }
    return out;
}

bool EditSession::slicerChoice(int slicer, QSet<qint32> *shown, qint64 *itemCount) const
{
    if (!m_book || !m_book->book || slicer < 0) return false;
    const Workbook &book = *m_book->book;
    const QVector<SlicerCache> caches = book.slicerCaches();
    if (slicer >= caches.size()) return false;
    const auto targets = slicerTargets(book, caches[slicer]);
    if (targets.isEmpty()) return false;
    const PivotBook &pb = book.pivots();
    const PivotTableEntry &entry = pb.table(targets[0].first);
    const PivotCache &cache = *pb.cache(entry.cache).cache;
    *shown = pivotShownItems(*entry.table, cache, targets[0].second);
    *itemCount = cache.fields[targets[0].second].items.size();
    return true;
}

quintptr EditSession::pivotTablesStamp() const
{
    if (!m_book || !m_book->book) return 0;
    const PivotBook &pb = m_book->book->pivots();
    quintptr stamp = quintptr(pb.tableCount()) * 7919;
    for (int t = 0; t < pb.tableCount(); ++t) stamp = stamp * 31 + quintptr(pb.table(t).table.get());
    return stamp;
}

bool EditSession::slicerFace(int slicer, qint64 first, int most, SlicerFace *out) const
{
    *out = SlicerFace();
    if (!m_book || !m_book->book || slicer < 0 || most < 0) return false;
    const Workbook &book = *m_book->book;
    const QVector<SlicerCache> caches = book.slicerCaches();
    if (slicer >= caches.size() || caches[slicer].timeline) return false;
    const SlicerCache &sc = caches[slicer];
    const auto targets = slicerTargets(book, sc);
    if (targets.isEmpty()) return false;
    const PivotBook &pb = book.pivots();
    const PivotTableEntry &entry = pb.table(targets[0].first);
    const PivotCache &cache = *pb.cache(entry.cache).cache;
    const int field = targets[0].second;
    const QVector<PivotValue> &values = cache.fields[field].items;
    const qint64 n = values.size();
    out->caption = sc.sourceName;
    for (int s = 0; s < book.info().sheets.size(); ++s) {
        const QVector<SlicerView> views = book.slicerViews(s);
        if (const SlicerView *v = viewOf(views, sc)) {
            if (!v->caption.isEmpty()) out->caption = v->caption;
            out->style = v->style;
            out->columns = std::max(1, v->columns);
            out->rowHeight = v->rowHeight;
            out->showCaption = v->showCaption;
            break;
        }
    }
    // (the places of the buttons, in the order the part lists its items; the part may name a place twice or one the field does not have: each is shown once, and only the field's)
    struct Place {
        qint32 x;
        bool noData;
    };
    std::vector<Place> places;
    if (sc.items.isEmpty()) {
        places.reserve(size_t(std::min<qint64>(n, kMaxSlicerItems)));
        for (qint32 i = 0; i < n && i < kMaxSlicerItems; ++i) places.push_back({i, false});
    } else {
        QVector<bool> seen(int(std::min<qint64>(n, kMaxSlicerItems)), false);
        for (const SlicerItem &it : sc.items) {
            if (it.x < 0 || it.x >= seen.size() || seen[it.x]) continue;
            seen[it.x] = true;
            if (sc.hideNoData && it.noData) continue;
            places.push_back({it.x, it.noData});
        }
    }
    if (!sc.sortOrder.isEmpty() && sc.sortOrder != QLatin1String("none") && int(places.size()) <= kMaxSlicerSorted) {
        const bool down = sc.sortOrder == QLatin1String("descending");
        std::stable_sort(places.begin(), places.end(), [&](const Place &a, const Place &b) { return down ? itemLess(values[b.x], values[a.x]) : itemLess(values[a.x], values[b.x]); });
    }
    out->itemCount = qint64(places.size());
    const QSet<qint32> shown = pivotShownItems(*entry.table, cache, field);
    out->filtered = !(shown.isEmpty() || qint64(shown.size()) >= n);
    for (qint64 i = std::max<qint64>(0, first); i < out->itemCount && i < first + most; ++i) {
        SlicerButton b;
        b.place = places[size_t(i)].x;
        b.text = itemText(values[b.place]);
        b.selected = shown.contains(b.place);
        b.noData = places[size_t(i)].noData;
        out->buttons.push_back(std::move(b));
    }
    return true;
}

int EditSession::slicerNamed(const QString &name, int sheet) const
{
    if (!m_book || !m_book->book || name.isEmpty()) return -1;
    const Workbook &book = *m_book->book;
    const QVector<SlicerCache> caches = book.slicerCaches();
    auto find = [&](int s) {
        const QVector<SlicerView> views = book.slicerViews(s);
        for (const SlicerView &v : views) {
            if (v.timeline || v.name != name) continue;
            for (int i = 0; i < caches.size(); ++i)
                if (caches[i].name == v.cache && !caches[i].timeline) return i;
        }
        return -1;
    };
    int at = find(sheet);
    for (int s = 0; at < 0 && s < book.info().sheets.size(); ++s)
        if (s != sheet) at = find(s);
    return at;
}

EditSession::Outcome EditSession::clickSlicerItem(int slicer, qint32 place, bool extend)
{
    QSet<qint32> shown;
    qint64 count = 0;
    if (!slicerChoice(slicer, &shown, &count) || place < 0 || place >= count) return Outcome();   // (nothing to choose: nothing is done)
    if (extend) {
        if (shown.size() >= count) {
            shown.clear();   // (all were chosen: Ctrl-click takes the item out of them)
            for (qint32 i = 0; i < count; ++i)
                if (i != place) shown.insert(i);
        } else if (shown.contains(place)) {
            shown.remove(place);
        } else {
            shown.insert(place);
        }
    } else if (shown.size() == 1 && shown.contains(place)) {
        shown.clear();   // (the only one chosen is clicked again: the filter goes)
    } else {
        shown = {place};
    }
    if (shown.isEmpty() || shown.size() >= count) shown.clear();
    return chooseSlicerItems(slicer, shown);
}

EditSession::Outcome EditSession::chooseSlicerItems(int slicer, const QSet<qint32> &chosen)
{
    Outcome out;
    auto refuse = [&](const QString &why) {
        out.ok = false;
        out.problem = why;
        return out;
    };
    QString why;
    if (!canEdit(&why)) return refuse(why);
    const Workbook &book = *m_book->book;
    const QVector<SlicerCache> caches = book.slicerCaches();
    if (slicer < 0 || slicer >= caches.size() || caches[slicer].timeline) return refuse(tr("This slicer can't be used."));
    const auto targets = slicerTargets(book, caches[slicer]);
    if (targets.isEmpty()) return refuse(tr("This slicer filters no pivot table that JeffCrunch can read."));
    const PivotBook &pb = book.pivots();
    // (the items are places among the first table's data: only the tables made from that data take the choice, and every one must be one that can be changed before any is)
    const int firstCache = pb.table(targets[0].first).cache;
    std::vector<std::pair<int, PivotTable>> changes;
    for (const auto &t : targets) {
        if (pb.table(t.first).cache != firstCache) continue;
        QString problem;
        if (!pivotUsable(t.first, &problem)) return refuse(problem.isEmpty() ? tr("A pivot table this slicer filters can't be changed.") : problem);
        const PivotCacheEntry &ce = pb.cache(pb.table(t.first).cache);
        PivotTable changed = *pb.table(t.first).table;
        SlicerCache sc = caches[slicer];
        sc.items.clear();
        for (qint32 i = 0; i < ce.cache->fields[t.second].items.size(); ++i) {
            SlicerItem it;
            it.x = i;
            it.selected = chosen.contains(i);
            sc.items.append(it);
        }
        if (!pivotApplySlicer(&changed, *ce.cache, t.second, sc)) return refuse(tr("This slicer can't be used."));
        changes.push_back({t.first, changed});
    }
    // (the tables are changed one after the other, as one step of Undo; if one fails, those done are undone, so nothing is half done)
    undoStack()->beginMacro(tr("Filter by Slicer"));
    int done = 0;
    for (const auto &c : changes) {
        out = setPivot(c.first, c.second, true, tr("Filter by Slicer"));
        if (!out.ok) break;
        ++done;
    }
    undoStack()->endMacro();
    if (!out.ok && done > 0) undoStack()->undo();
    return out;
}

} // namespace jp::crunch
