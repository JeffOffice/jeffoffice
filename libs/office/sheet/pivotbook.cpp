#include "sheet/pivotbook.h"

#include "office/package.h"

#include <QDateTime>

namespace jp::office::sheet {

PivotBook::PivotBook(const Package *package, const QVector<QString> &sheetParts, const QStringList &sheetNames, jp::calc::DateSystem dates)
    : m_package(package), m_names(sheetNames), m_dates(dates)
{
    QHash<QString, int> cacheOfPart;
    for (int sheet = 0; sheet < sheetParts.size(); ++sheet) {
        if (sheetParts[sheet].isEmpty()) continue;
        for (const Relationship &rel : package->relationshipsOfType(sheetParts[sheet], QStringLiteral("/pivotTable"))) {
            if (rel.external) continue;
            PivotTableEntry e;
            e.sheet = sheet;
            e.part = resolvePartName(sheetParts[sheet], rel.target);
            QString why;
            const qint64 size = e.part.isEmpty() ? -1 : package->partSize(e.part);
            if (size < 0 || size > kPivotMaxTablePartBytes) {
                e.why = size < 0 ? QStringLiteral("the pivot table is not in the file") : QStringLiteral("the pivot table is too big");
            } else {
                // (a table that can't be read whole still says where it is)
                const QByteArray bytes = package->part(e.part);
                QString ref;
                CellRange where;
                if (readPivotTableLocation(bytes, &ref) && parseRangeRef(ref, &where)) e.fallback = where;
                if (pivotXmlDepth(bytes, kPivotMaxXmlDepth) > kPivotMaxXmlDepth) {
                    e.why = QStringLiteral("the pivot table is nested too deep");
                } else {
                    auto parsed = std::make_shared<PivotTable>();
                    if (!readPivotTable(bytes, parsed.get(), &why)) e.why = why.isEmpty() ? QStringLiteral("the pivot table can't be read") : why;
                    else e.table = e.fileTable = parsed;
                }
            }
            QString definition;
            for (const Relationship &cr : package->relationshipsOfType(e.part, QStringLiteral("/pivotCacheDefinition")))
                if (!cr.external) definition = resolvePartName(e.part, cr.target);
            if (!definition.isEmpty()) {
                auto at = cacheOfPart.find(definition);
                if (at == cacheOfPart.end()) {
                    PivotCacheEntry c;
                    c.definitionPart = definition;
                    for (const Relationship &rr : package->relationshipsOfType(definition, QStringLiteral("/pivotCacheRecords")))
                        if (!rr.external) c.recordsPart = resolvePartName(definition, rr.target);
                    // where the data is, read now: the edits of rows and columns that move it are followed from the start, whether or not the cache is ever read
                    const qint64 defSize = package->partSize(definition);
                    if (defSize < 0 || defSize > kPivotMaxDefinitionBytes || (!c.recordsPart.isEmpty() && package->partSize(c.recordsPart) > kPivotMaxRecordsBytes)) {
                        c.unreadable = true;
                        c.why = QStringLiteral("the cache is too big");
                    } else {
                        const QByteArray def = package->part(definition);
                        if (pivotXmlDepth(def, kPivotMaxXmlDepth) > kPivotMaxXmlDepth) {
                            c.unreadable = true;
                            c.why = QStringLiteral("the cache is nested too deep");
                        }
                        PivotCache head;
                        if (readPivotCacheSource(def, &head)) {
                            c.sourceName = head.name;
                            c.fileSheetName = head.sheet;
                            if (head.sourceType == QLatin1String("worksheet") && head.name.isEmpty()) {
                                for (int s2 = 0; s2 < m_names.size(); ++s2)
                                    if (!m_names[s2].isEmpty() && m_names[s2].compare(head.sheet, Qt::CaseInsensitive) == 0) c.sourceSheet = s2;
                                CellRange r;
                                if (parseRangeRef(head.ref, &r)) c.source = c.fileSource = r;
                            }
                        }
                    }
                    m_caches.push_back(c);
                    at = cacheOfPart.insert(definition, int(m_caches.size()) - 1);
                }
                e.cache = at.value();
            }
            m_tables.push_back(std::move(e));
        }
    }
}

void PivotBook::load(int i) const
{
    PivotCacheEntry &c = m_caches[size_t(i)];
    if (c.tried) return;
    c.tried = true;
    if (c.unreadable) return;
    auto cache = std::make_shared<PivotCache>();
    QString why;
    if (!readPivotCache(m_package->part(c.definitionPart), c.recordsPart.isEmpty() ? QByteArray() : m_package->part(c.recordsPart), m_dates, cache.get(), &why)) {
        c.why = why;
        return;
    }
    c.cache = c.fileCache = cache;
    if (!cache->unsupported.isEmpty()) c.why = cache->unsupported;
}

const PivotCacheEntry &PivotBook::cache(int i) const
{
    load(i);
    return m_caches[size_t(i)];
}

std::vector<int> PivotBook::tablesOn(int sheet) const
{
    std::vector<int> out;
    for (size_t i = 0; i < m_tables.size(); ++i)
        if (m_tables[i].sheet == sheet) out.push_back(int(i));
    return out;
}

std::vector<int> PivotBook::tablesOfCache(int cache) const
{
    std::vector<int> out;
    for (size_t i = 0; i < m_tables.size(); ++i)
        if (m_tables[i].cache == cache) out.push_back(int(i));
    return out;
}

CellRange PivotBook::locationOf(int i) const
{
    CellRange r;
    const PivotTableEntry &e = m_tables[size_t(i)];
    if (e.table) parseRangeRef(e.table->ref, &r);
    else r = e.fallback;
    return r;
}

void PivotBook::setTable(int i, std::shared_ptr<const PivotTable> table, bool structureChanged)
{
    m_tables[size_t(i)].table = std::move(table);
    m_tables[size_t(i)].structureChanged = structureChanged;
}

void PivotBook::setCache(int i, std::shared_ptr<const PivotCache> cache, const CellRange &source, int sourceSheet, const QString &sourceName)
{
    load(i);
    PivotCacheEntry &c = m_caches[size_t(i)];
    c.cache = std::move(cache);
    c.source = source;
    c.sourceSheet = sourceSheet;
    c.sourceName = sourceName;
}

std::vector<PivotBook::SourcePlace> PivotBook::moveSources(const StructEdit &edit)
{
    std::vector<SourcePlace> before;
    for (size_t i = 0; i < m_caches.size(); ++i) {
        PivotCacheEntry &c = m_caches[i];
        if (!c.source.valid || c.sourceSheet < 0) continue;
        CellRange now = c.source;
        if (!adjustRange(edit, c.sourceSheet, &now) || now == c.source) continue;
        before.push_back({int(i), c.source});
        c.source = now;
    }
    return before;
}

void PivotBook::putSourcesBack(const std::vector<SourcePlace> &places)
{
    for (const SourcePlace &p : places) m_caches[size_t(p.cache)].source = p.was;
}

bool PivotBook::changed() const
{
    for (const PivotTableEntry &t : m_tables)
        if (t.table != t.fileTable) return true;
    for (const PivotCacheEntry &c : m_caches)
        if ((c.tried && c.cache != c.fileCache) || !(c.source == c.fileSource)) return true;
    return false;
}

std::map<QString, QByteArray> PivotBook::changedParts(const QStringList &sheetNames) const
{
    std::map<QString, QByteArray> out;
    for (const PivotTableEntry &t : m_tables) {
        if (!t.table || t.table == t.fileTable) continue;
        const QByteArray bytes = writePivotTable(m_package->part(t.part), *t.table, t.structureChanged);
        if (!bytes.isEmpty()) out[t.part] = bytes;
    }
    // the date a refresh is made, in the workbook's own count of days
    const double unixDays = double(QDateTime::currentDateTimeUtc().toSecsSinceEpoch()) / 86400.0;
    const double now = unixDays + (m_dates == jp::calc::DateSystem::Date1904 ? 24107.0 : 25569.0);
    for (const PivotCacheEntry &c : m_caches) {
        const bool sourceMoved = c.sourceSheet >= 0 && c.sourceSheet < sheetNames.size() && !sheetNames[c.sourceSheet].isEmpty() && c.source.valid && c.fileSource.valid
                                 && (!(c.source == c.fileSource) || sheetNames[c.sourceSheet].compare(c.fileSheetName, Qt::CaseInsensitive) != 0);
        if (c.tried && c.cache && c.cache != c.fileCache) {
            PivotCache cache = *c.cache;
            if (c.sourceSheet >= 0 && c.sourceSheet < sheetNames.size() && c.source.valid) {
                cache.sheet = sheetNames[c.sourceSheet];
                cache.ref = pivotRangeRef(c.source.row1, c.source.col1, c.source.row2, c.source.col2);
                cache.name.clear();
            } else {
                cache.name = c.sourceName;
            }
            const QByteArray def = writePivotCacheDefinition(m_package->part(c.definitionPart), cache, cache.recordCount, now, m_dates);
            if (def.isEmpty()) continue;
            out[c.definitionPart] = def;
            if (!c.recordsPart.isEmpty()) out[c.recordsPart] = writePivotCacheRecords(cache, m_dates);
        } else if (sourceMoved) {
            // (the data was not read again; only where it is changed: that one attribute is written and the rest of the part is the file's own)
            out[c.definitionPart] = patchPivotCacheSource(m_package->part(c.definitionPart), sheetNames[c.sourceSheet], pivotRangeRef(c.source.row1, c.source.col1, c.source.row2, c.source.col2));
        }
    }
    return out;
}

} // namespace jp::office::sheet
