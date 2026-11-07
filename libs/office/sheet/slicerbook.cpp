// The slicers and timelines of a workbook: the parts its relationships name, read once (a part named by many relationships is read once too).
#include "sheet/sheetreader.h"

#include <atomic>

namespace jp::office::sheet {

namespace {

// The parts one workbook is let list: far more than real files have (a workbook has a cache for each slicer, a few dozen at most).
constexpr int kMaxSlicerParts = 20000;
constexpr qint64 kMaxSlicerPartBytes = qint64(8) << 20;

std::atomic<int> gParsed{0};

} // namespace

int slicerPartsParsed()
{
    return gParsed.load();
}

void Workbook::slicerItemRemoved(int pivotCache, int field, qint32 item)
{
    slicerCaches();   // (read, if they have not been)
    for (SlicerCache &sc : m_slicerCaches) {
        if (sc.timeline) continue;
        bool mine = false;
        for (const auto &t : slicerTargets(*this, sc))
            if (pivots().table(t.first).cache == pivotCache && t.second == field) mine = true;
        if (!mine) continue;
        QVector<SlicerItem> kept;
        for (SlicerItem it : sc.items) {
            if (it.x == item) continue;
            if (it.x > item) --it.x;
            kept.append(it);
        }
        sc.items = kept;
    }
}

QVector<SlicerCache> Workbook::slicerCaches() const
{
    if (m_slicerCachesRead) return m_slicerCaches;
    m_slicerCachesRead = true;
    m_slicerCaches.clear();
    if (m_info.sheets.isEmpty() || mainPart().isEmpty()) return m_slicerCaches;
    QSet<QString> seen;
    for (const char *type : {"/slicerCache", "/timelineCache"}) {
        const QVector<Relationship> rels = package().relationshipsOfType(mainPart(), QString::fromLatin1(type));
        for (const Relationship &r : rels) {
            if (r.external || m_slicerCaches.size() >= kMaxSlicerParts || seen.contains(r.target.toLower())) continue;
            seen.insert(r.target.toLower());
            const QByteArray xml = package().part(r.target);
            if (xml.isEmpty() || xml.size() > kMaxSlicerPartBytes) continue;
            ++gParsed;
            SlicerCache c;
            if (readSlicerCache(xml, &c)) {
                c.part = r.target;
                m_slicerCaches.append(c);
            }
        }
    }
    return m_slicerCaches;
}

QVector<SlicerView> Workbook::slicerViews(int sheet) const
{
    if (sheet < 0 || sheet >= m_info.sheets.size() || m_info.sheets[sheet].part.isEmpty()) return {};
    const auto known = m_slicerViews.constFind(sheet);
    if (known != m_slicerViews.constEnd()) return known.value();
    QVector<SlicerView> out;
    QSet<QString> seen;
    for (const char *type : {"/slicer", "/timeline"}) {
        const QVector<Relationship> rels = package().relationshipsOfType(m_info.sheets[sheet].part, QString::fromLatin1(type));
        for (const Relationship &r : rels) {
            if (r.external || out.size() >= kMaxSlicerParts || seen.contains(r.target.toLower())) continue;
            seen.insert(r.target.toLower());
            const QByteArray xml = package().part(r.target);
            if (xml.isEmpty() || xml.size() > kMaxSlicerPartBytes) continue;
            ++gParsed;
            QVector<SlicerView> v;
            if (readSlicerViews(xml, &v)) out += v;
        }
    }
    m_slicerViews.insert(sheet, out);
    return out;
}

QVector<std::pair<int, int>> slicerTargets(const Workbook &book, const SlicerCache &slicer)
{
    QVector<std::pair<int, int>> out;
    if (slicer.timeline) return out;
    const PivotBook &pb = book.pivots();
    QSet<int> seenTables;   // (a cache that names a table many times filters it once)
    for (const SlicerCache::Connection &c : slicer.tables) {
        for (int t = 0; t < pb.tableCount(); ++t) {
            const PivotTableEntry &entry = pb.table(t);
            if (!entry.table || entry.table->name.compare(c.name, Qt::CaseInsensitive) != 0 || entry.cache < 0 || !pb.cache(entry.cache).cache) continue;
            if (seenTables.contains(t)) break;
            seenTables.insert(t);
            const PivotCache &cache = *pb.cache(entry.cache).cache;
            for (int f = 0; f < cache.fields.size() && f < entry.table->fields.size(); ++f) {
                const QString shown = entry.table->fields[f].name.isEmpty() ? cache.fields[f].name : entry.table->fields[f].name;
                if (cache.fields[f].name.compare(slicer.sourceName, Qt::CaseInsensitive) == 0 || shown.compare(slicer.sourceName, Qt::CaseInsensitive) == 0) {
                    out.append({t, f});
                    break;
                }
            }
            break;
        }
    }
    return out;
}

std::map<QString, QByteArray> slicerPartPatches(const Workbook &book)
{
    std::map<QString, QByteArray> out;
    const PivotBook &pb = book.pivots();
    if (pb.tableCount() == 0) return out;
    for (const SlicerCache &sc : book.slicerCaches()) {
        const auto targets = slicerTargets(book, sc);
        if (targets.isEmpty() || sc.part.isEmpty()) continue;
        // (the choice is that of the first table, among those with the first one's data, that this session changed: the tables of a slicer show the same items)
        const int firstCache = pb.table(targets[0].first).cache;
        std::pair<int, int> from(-1, -1);
        for (const auto &t : targets)
            if (pb.table(t.first).cache == firstCache && (pb.tableChanged(t.first) || book.slicerPartsWritten().contains(sc.part))) {
                from = t;
                break;
            }
        if (from.first < 0) continue;   // (a table as the file has it leaves the slicer as the file has it, whatever the two say)
        const PivotTableEntry &entry = pb.table(from.first);
        const PivotCache &cache = *pb.cache(entry.cache).cache;
        const QSet<qint32> shown = pivotShownItems(*entry.table, cache, from.second);
        QSet<qint32> inFile;
        for (const SlicerItem &it : sc.items)
            if (it.selected) inFile.insert(it.x);
        const qint64 count = cache.fields[from.second].items.size();
        if (slicerSameChoice(shown, inFile, count)) continue;
        // (no filter on is written as no item chosen)
        const QByteArray patched = patchSlicerSelection(book.package().part(sc.part), shown.size() >= count ? QSet<qint32>() : shown);
        if (patched.isEmpty()) continue;
        out[sc.part] = patched;
        book.slicerPartsWritten().insert(sc.part);
    }
    return out;
}

} // namespace jp::office::sheet
