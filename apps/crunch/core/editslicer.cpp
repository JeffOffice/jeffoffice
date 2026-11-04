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
    const std::vector<SlicerInfo> all = slicers();
    if (slicer < 0 || slicer >= int(all.size()) || all[size_t(slicer)].cache.timeline) return refuse(tr("This slicer can't be used."));
    const Workbook &book = *m_book->book;
    const auto targets = slicerTargets(book, all[size_t(slicer)].cache);
    if (targets.isEmpty()) return refuse(tr("This slicer filters no pivot table that JeffCrunch can read."));
    SlicerCache choice = all[size_t(slicer)].cache;
    choice.items.clear();
    const PivotBook &pb = book.pivots();
    std::vector<std::pair<int, PivotTable>> changes;
    for (const auto &t : targets) {
        const PivotCacheEntry &ce = pb.cache(pb.table(t.first).cache);
        PivotTable changed = *pb.table(t.first).table;
        SlicerCache sc = choice;
        for (qint32 i = 0; i < ce.cache->fields[t.second].items.size(); ++i) {
            SlicerItem it;
            it.x = i;
            it.selected = chosen.contains(i);
            sc.items.append(it);
        }
        if (!pivotApplySlicer(&changed, *ce.cache, t.second, sc)) return refuse(tr("This slicer can't be used."));
        changes.push_back({t.first, changed});
    }
    // (the tables are changed one after the other, as one step of Undo)
    undoStack()->beginMacro(tr("Filter by Slicer"));
    for (const auto &c : changes) {
        out = setPivot(c.first, c.second, true, tr("Filter by Slicer"));
        if (!out.ok) break;
    }
    undoStack()->endMacro();
    return out;
}

} // namespace jp::crunch
