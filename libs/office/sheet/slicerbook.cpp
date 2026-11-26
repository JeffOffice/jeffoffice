// The slicers and timelines of a workbook: the parts its relationships name, read once (a part named by many relationships is read once too).
#include "sheet/sheetreader.h"
#include "sheet/xmlscan.h"

#include <QRegularExpression>

#include <algorithm>
#include <set>

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

namespace {

using xmlscan::Tag;

qsizetype endOfElement(const QByteArray &xml, const Tag &open)
{
    if (open.selfClosing) return open.end;
    int depth = 1;
    qsizetype pos = open.end;
    Tag t;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing) {
            if (--depth == 0) return t.end;
        } else if (!t.selfClosing) {
            ++depth;
        }
    }
    return -1;
}

QString attributeOf(const QByteArray &xml, const Tag &t, const char *name)
{
    bool found = false;
    const QByteArrayView v = xmlscan::attribute(QByteArrayView(xml).sliced(t.start, t.end - t.start), name, &found);
    return found ? xmlscan::unescape(v) : QString();
}

// Cuts the ranges out of the bytes (they do not overlap).
QByteArray cut(const QByteArray &xml, std::vector<std::pair<qsizetype, qsizetype>> ranges)
{
    std::sort(ranges.begin(), ranges.end());
    QByteArray out;
    qsizetype at = 0;
    for (const auto &r : ranges) {
        if (r.first < at) continue;
        out += xml.mid(at, r.first - at);
        at = r.second;
    }
    return out + xml.mid(at);
}

} // namespace

QByteArray removeSlicerElements(const QByteArray &xml, const QSet<QString> &names, int *left, bool *any)
{
    std::vector<std::pair<qsizetype, qsizetype>> ranges;
    int depth = 0, kept = 0;
    Tag t;
    qsizetype pos = 0;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing) {
            --depth;
            continue;
        }
        if (depth == 1 && (t.local == "slicer" || t.local == "timeline")) {
            if (t.local == "slicer" && names.contains(attributeOf(xml, t, "name"))) {
                const qsizetype end = endOfElement(xml, t);
                if (end > 0) {
                    ranges.emplace_back(t.start, end);
                    pos = end;
                    continue;
                }
            }
            ++kept;
        }
        if (!t.selfClosing) ++depth;
    }
    *any = !ranges.empty();
    *left = kept;
    return ranges.empty() ? xml : cut(xml, ranges);
}

QByteArray dropExtensionEntries(const QByteArray &xml, const QByteArray &entry, const QStringList &relIds)
{
    std::vector<std::pair<qsizetype, qsizetype>> ranges;
    Tag t;
    qsizetype pos = 0;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing || t.local != QByteArrayView(entry)) continue;
        const QString id = attributeOf(xml, t, "r:id");
        const qsizetype end = endOfElement(xml, t);
        if (end < 0) break;
        if (!id.isEmpty() && relIds.contains(id)) ranges.emplace_back(t.start, end);
        pos = end;
    }
    if (ranges.empty()) return xml;
    QByteArray out = cut(xml, ranges);
    // (a list left with nothing in it, the extension that held it, and an extension list left with no extension)
    static const QRegularExpression emptyList(QStringLiteral("<(\\w+:)?(slicerList|slicerCaches)\\b[^>]*?(/>|>\\s*</(\\w+:)?(slicerList|slicerCaches)>)"));
    static const QRegularExpression emptyExt(QStringLiteral("<(\\w+:)?ext\\b[^>]*?>\\s*</(\\w+:)?ext>"));
    static const QRegularExpression emptyExtLst(QStringLiteral("<(\\w+:)?extLst>\\s*</(\\w+:)?extLst>"));
    QString text = QString::fromUtf8(out);
    text.remove(emptyList);
    text.remove(emptyExt);
    text.remove(emptyExtLst);
    return text.toUtf8();
}

QByteArray dropDefinedNames(const QByteArray &xml, const QStringList &names)
{
    std::vector<std::pair<qsizetype, qsizetype>> ranges;
    Tag t;
    qsizetype pos = 0;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing || t.local != "definedName") continue;
        const qsizetype end = endOfElement(xml, t);
        if (end < 0) break;
        pos = end;
        if (!names.contains(attributeOf(xml, t, "name"), Qt::CaseSensitive) || attributeOf(xml, t, "localSheetId").size() > 0) continue;
        const QByteArray body = xml.mid(t.end, end - t.end);
        if (body.startsWith("#N/A")) ranges.emplace_back(t.start, end);
    }
    if (ranges.empty()) return xml;
    QByteArray out = cut(xml, ranges);
    static const QRegularExpression emptyNames(QStringLiteral("<(\\w+:)?definedNames>\\s*</(\\w+:)?definedNames>"));
    QString text = QString::fromUtf8(out);
    text.remove(emptyNames);
    return text.toUtf8();
}

bool planSlicerRemovals(const Workbook &book, const std::vector<std::pair<int, QString>> &removed, SlicerRemovalPlan *out)
{
    if (removed.empty()) return true;
    const Package &package = book.package();
    QSet<QString> caches;   // the caches the removed slicers read
    std::map<QString, QByteArray> parts;   // the slicer parts as they are with what is taken out so far
    std::set<QString> emptied;
    for (const auto &[sheet, name] : removed) {
        if (sheet < 0 || sheet >= book.info().sheets.size() || book.info().sheets[sheet].part.isEmpty()) continue;
        const QString sheetPart = book.info().sheets[sheet].part;
        for (const Relationship &r : package.relationshipsOfType(sheetPart, QStringLiteral("/slicer"))) {
            if (r.external || emptied.count(r.target)) continue;
            const QByteArray had = parts.count(r.target) ? parts[r.target] : package.part(r.target);
            QVector<SlicerView> views;
            if (had.isEmpty() || !readSlicerViews(had, &views)) continue;
            QString cache;
            bool mine = false;
            for (const SlicerView &v : views)
                if (v.name == name && !v.timeline) {
                    cache = v.cache;
                    mine = true;
                }
            if (!mine) continue;
            int left = 0;
            bool any = false;
            const QByteArray now = removeSlicerElements(had, {name}, &left, &any);
            if (!any) continue;
            caches.insert(cache);
            if (left == 0) {
                emptied.insert(r.target);
                parts.erase(r.target);
                out->released.emplace_back(sheetPart, QStringList{r.id});
                out->sheetEntries[sheet] << r.id;
            } else {
                parts[r.target] = now;
            }
        }
    }
    out->partsKept = parts;
    // (a cache that no slicer left in the file reads goes: its link from the workbook, its entry in the workbook's list, and its name)
    for (const QString &cacheName : caches) {
        bool used = false;
        for (int s = 0; s < book.info().sheets.size() && !used; ++s) {
            if (book.info().sheets[s].deleted) continue;
            for (const SlicerView &v : book.slicerViews(s)) {
                if (v.cache != cacheName) continue;
                if (std::find(removed.begin(), removed.end(), std::make_pair(s, v.name)) == removed.end()) used = true;
            }
        }
        if (used) continue;
        for (const SlicerCache &c : book.slicerCaches()) {
            if (c.name != cacheName || c.timeline) continue;
            for (const Relationship &r : package.relationshipsOfType(book.mainPart(), QStringLiteral("/slicerCache"))) {
                if (r.external || r.target.compare(c.part, Qt::CaseInsensitive) != 0) continue;
                out->released.emplace_back(book.mainPart(), QStringList{r.id});
                out->workbookEntries << r.id;
            }
            out->definedNames << c.name;
        }
    }
    return true;
}

} // namespace jp::office::sheet
