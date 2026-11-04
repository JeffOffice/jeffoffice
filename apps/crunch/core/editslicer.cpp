// Slicers: what the pane shows of them, and the click that chooses items.
#include "core/editsession.h"

#include "sheet/slicer.h"

#include <QCoreApplication>
#include <QUndoStack>

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
