// The slicers and timelines of a workbook: the parts its relationships name, read when asked for.
#include "sheet/sheetreader.h"

namespace jp::office::sheet {

namespace {

// The parts one workbook is let list: far more than real files have (a workbook has a cache for each slicer, a few dozen at most).
constexpr int kMaxSlicerParts = 20000;
constexpr qint64 kMaxSlicerPartBytes = qint64(8) << 20;

} // namespace

QVector<SlicerCache> Workbook::slicerCaches() const
{
    QVector<SlicerCache> out;
    if (m_info.sheets.isEmpty() || mainPart().isEmpty()) return out;
    for (const char *type : {"/slicerCache", "/timelineCache"}) {
        const QVector<Relationship> rels = package().relationshipsOfType(mainPart(), QString::fromLatin1(type));
        for (const Relationship &r : rels) {
            if (r.external || out.size() >= kMaxSlicerParts) continue;
            const QByteArray xml = package().part(r.target);
            if (xml.isEmpty() || xml.size() > kMaxSlicerPartBytes) continue;
            SlicerCache c;
            if (readSlicerCache(xml, &c)) {
                c.part = r.target;
                out.append(c);
            }
        }
    }
    return out;
}

QVector<SlicerView> Workbook::slicerViews(int sheet) const
{
    QVector<SlicerView> out;
    if (sheet < 0 || sheet >= m_info.sheets.size() || m_info.sheets[sheet].part.isEmpty()) return out;
    for (const char *type : {"/slicer", "/timeline"}) {
        const QVector<Relationship> rels = package().relationshipsOfType(m_info.sheets[sheet].part, QString::fromLatin1(type));
        for (const Relationship &r : rels) {
            if (r.external || out.size() >= kMaxSlicerParts) continue;
            const QByteArray xml = package().part(r.target);
            if (xml.isEmpty() || xml.size() > kMaxSlicerPartBytes) continue;
            QVector<SlicerView> v;
            if (readSlicerViews(xml, &v)) out += v;
        }
    }
    return out;
}

QVector<std::pair<int, int>> slicerTargets(const Workbook &book, const SlicerCache &slicer)
{
    QVector<std::pair<int, int>> out;
    if (slicer.timeline) return out;
    const PivotBook &pb = book.pivots();
    for (const SlicerCache::Connection &c : slicer.tables) {
        for (int t = 0; t < pb.tableCount(); ++t) {
            const PivotTableEntry &entry = pb.table(t);
            if (!entry.table || entry.table->name.compare(c.name, Qt::CaseInsensitive) != 0 || entry.cache < 0 || !pb.cache(entry.cache).cache) continue;
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
        if (!pb.tableChanged(targets[0].first) && !book.slicerPartsWritten().contains(sc.part)) continue;   // (a table as the file has it leaves the slicer as the file has it, whatever the two say)
        const PivotTableEntry &entry = pb.table(targets[0].first);
        const PivotCache &cache = *pb.cache(entry.cache).cache;
        const QSet<qint32> shown = pivotShownItems(*entry.table, cache, targets[0].second);
        QSet<qint32> inFile;
        for (const SlicerItem &it : sc.items)
            if (it.selected) inFile.insert(it.x);
        if (slicerSameChoice(shown, inFile, cache.fields[targets[0].second].items.size())) continue;
        out[sc.part] = patchSlicerSelection(book.package().part(sc.part), shown);
        book.slicerPartsWritten().insert(sc.part);
    }
    return out;
}

} // namespace jp::office::sheet
