#include "sheet/pivotbook.h"

#include "office/package.h"

#include <QDateTime>
#include <QIODevice>

namespace jp::office::sheet {

namespace {
qint64 gTableLimit = 0, gDefinitionLimit = 0, gRecordsLimit = 0;
qint64 tableLimit() { return gTableLimit > 0 ? gTableLimit : kPivotMaxTablePartBytes; }
qint64 definitionLimit() { return gDefinitionLimit > 0 ? gDefinitionLimit : kPivotMaxDefinitionBytes; }
qint64 recordsLimit() { return gRecordsLimit > 0 ? gRecordsLimit : kPivotMaxRecordsBytes; }
} // namespace

void pivotSetPartLimitsForTest(qint64 table, qint64 definition, qint64 records)
{
    gTableLimit = table;
    gDefinitionLimit = definition;
    gRecordsLimit = records;
}

namespace {

// The first bytes of a part (where a table's location and a cache's source are): the part is not read whole to find them.
QByteArray headOf(const Package *package, const QString &part, qint64 bytes)
{
    const auto device = package->openPart(part);
    if (!device || (!device->isOpen() && !device->open(QIODevice::ReadOnly))) return QByteArray();
    return device->read(bytes);
}

} // namespace

PivotBook::PivotBook(const Package *package, const QVector<QString> &sheetParts, const QStringList &sheetNames, jp::calc::DateSystem dates)
    : m_package(package), m_names(sheetNames), m_dates(dates)
{
    // (only where things are is read here: the tables and the caches are read when something asks for them)
    QHash<QString, int> cacheOfPart;
    for (int sheet = 0; sheet < sheetParts.size(); ++sheet) {
        if (sheetParts[sheet].isEmpty()) continue;
        for (const Relationship &rel : package->relationshipsOfType(sheetParts[sheet], QStringLiteral("/pivotTable"))) {
            if (rel.external) continue;
            PivotTableEntry e;
            e.sheet = sheet;
            e.part = resolvePartName(sheetParts[sheet], rel.target);
            const qint64 size = e.part.isEmpty() ? -1 : package->partSize(e.part);
            if (size < 0 || size > tableLimit()) {
                e.why = size < 0 ? QStringLiteral("the pivot table is not in the file") : QStringLiteral("the pivot table is too big");
                e.loaded = true;
            } else {
                // (a table that can't be read whole still says where it is)
                QString ref;
                CellRange where;
                QByteArray head = headOf(package, e.part, 16384);
                if (!(readPivotTableLocation(head, &ref) && parseRangeRef(ref, &where)) && size > head.size()) {
                    head = package->part(e.part);
                    if (readPivotTableLocation(head, &ref) && parseRangeRef(ref, &where)) e.fallback = where;
                } else if (where.valid) {
                    e.fallback = where;
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
                    if (defSize < 0 || defSize > definitionLimit() || (!c.recordsPart.isEmpty() && package->partSize(c.recordsPart) > recordsLimit())) {
                        c.unreadable = true;
                        c.why = QStringLiteral("the cache is too big");
                    } else {
                        PivotCache head;
                        QByteArray def = headOf(package, definition, 32768);
                        bool found = readPivotCacheSource(def, &head);
                        if (!found && defSize > def.size()) {
                            def = package->part(definition);
                            found = readPivotCacheSource(def, &head);
                        }
                        if (found) {
                            c.sourceName = head.name;
                            c.fileSheetName = head.sheet;
                            c.external = head.externalSource;
                            // (a source in another workbook is not a range of this one: it is not followed when rows move, and is not read again from here)
                            if (!c.external && head.sourceType == QLatin1String("worksheet") && head.name.isEmpty()) {
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

void PivotBook::loadTable(int i) const
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    PivotTableEntry &e = m_tables[size_t(i)];
    if (e.loaded) return;
    e.loaded = true;
    const QByteArray bytes = m_package->part(e.part);
    QString why;
    if (pivotXmlDepth(bytes, kPivotMaxXmlDepth) > kPivotMaxXmlDepth) {
        e.why = QStringLiteral("the pivot table is nested too deep");
        return;
    }
    auto parsed = std::make_shared<PivotTable>();
    if (!readPivotTable(bytes, parsed.get(), &why)) e.why = why.isEmpty() ? QStringLiteral("the pivot table can't be read") : why;
    else e.table = e.fileTable = parsed;
}

const PivotTableEntry &PivotBook::table(int i) const
{
    loadTable(i);
    return m_tables[size_t(i)];
}

bool PivotBook::unreadable(int i) const
{
    loadTable(i);
    return !m_tables[size_t(i)].table;
}

void PivotBook::load(int i) const
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    PivotCacheEntry &c = m_caches[size_t(i)];
    if (c.tried) return;
    c.tried = true;
    if (c.unreadable) return;
    const QByteArray def = m_package->part(c.definitionPart);
    if (pivotXmlDepth(def, kPivotMaxXmlDepth) > kPivotMaxXmlDepth) {
        c.unreadable = true;
        c.why = QStringLiteral("the cache is nested too deep");
        return;
    }
    auto cache = std::make_shared<PivotCache>();
    QString why;
    if (!readPivotCache(def, c.recordsPart.isEmpty() ? QByteArray() : m_package->part(c.recordsPart), m_dates, cache.get(), &why)) {
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

bool PivotBook::viewAt(int sheet, quint32 row, quint32 col, View *out) const
{
    std::shared_ptr<const PivotTable> table;
    std::shared_ptr<const PivotCache> cache;
    CellRange where;
    int index = -1;
    auto fill = [&](const GridMemo &m) {
        if (!m.lookup) return false;
        out->table = m.table;
        out->cache = m.cache;
        out->grid = m.grid;
        out->lookup = m.lookup;
        out->where = where;
        out->index = index;
        return true;
    };
    {
        std::lock_guard<std::recursive_mutex> lock(m_lock);
        for (size_t i = 0; i < m_tables.size() && index < 0; ++i) {
            if (m_tables[i].sheet != sheet) continue;
            loadTable(int(i));
            const PivotTableEntry &e = m_tables[i];
            if (!e.table || e.cache < 0) continue;
            const CellRange here = locationOf(int(i));
            if (!here.valid) continue;
            // (the cells of the table, and the lines of its filters above them)
            const quint32 pages = e.table->rowPageCount > 0 ? quint32(e.table->rowPageCount) + 1 : 0;
            const quint32 top = here.row1 >= pages ? here.row1 - pages : 0;
            if (row < top || row > here.row2 || col < here.col1 || col > here.col2) continue;
            load(e.cache);
            const PivotCacheEntry &c = m_caches[size_t(e.cache)];
            if (!c.cache || !c.cache->unsupported.isEmpty()) continue;
            index = int(i);
            where = here;
            table = e.table;
            cache = c.cache;
        }
        if (index < 0) return false;
        for (const GridMemo &m : m_grids)
            if (m.table == table && m.cache == cache) return fill(m);
    }
    // (the table is worked out, and the lookup made, with no lock held: other asks of other tables are not kept waiting; a table asked twice at once is worked out twice, and the first kept)
    GridMemo made;
    made.table = table;
    made.cache = cache;
    auto grid = std::make_shared<PivotGrid>();
    QString why;
    if (computePivot(*table, *cache, grid.get(), &why)) {
        made.grid = grid;
        made.lookup = std::make_shared<PivotLookup>(table, cache, grid);
    }
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    for (const GridMemo &m : m_grids)
        if (m.table == table && m.cache == cache) return fill(m);
    if (m_grids.size() >= 8) m_grids.erase(m_grids.begin());
    m_grids.push_back(made);
    return fill(made);
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
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    loadTable(i);
    m_tables[size_t(i)].table = std::move(table);
    m_tables[size_t(i)].structureChanged = structureChanged;
}

void PivotBook::setCache(int i, std::shared_ptr<const PivotCache> cache, const CellRange &source, int sourceSheet, const QString &sourceName)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
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

void PivotBook::markSaved(const QStringList &sheetNames)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    QSet<int> cachesWritten, cachesUsed;
    for (PivotTableEntry &t : m_tables) {
        cachesUsed.insert(t.cache);
        if (t.isNew) {
            // (a table made here is in the file now, with its parts, unless its sheet was deleted and nothing of it was written: it is the file's own from here on)
            if (t.cache < 0 || t.sheet < 0 || t.sheet >= sheetNames.size() || sheetNames[t.sheet].isEmpty()) continue;
            t.isNew = false;
            cachesWritten.insert(t.cache);
        }
        // (a table on a sheet that is deleted had nothing of it written: it is not marked as in the file)
        if (t.sheet >= 0 && t.sheet < sheetNames.size() && sheetNames[t.sheet].isEmpty()) continue;
        t.fileTable = t.table;
        t.structureChanged = false;
        cachesWritten.insert(t.cache);
    }
    for (size_t i = 0; i < m_caches.size(); ++i) {
        PivotCacheEntry &c = m_caches[i];
        if (c.isNew && !cachesWritten.contains(int(i))) continue;   // (not written)
        if (cachesUsed.contains(int(i)) && !cachesWritten.contains(int(i))) continue;   // (every table of it is on a deleted sheet: nothing of it was written)
        c.isNew = false;
        c.fileCache = c.cache;
        c.fileSource = c.source;
        if (c.sourceSheet >= 0 && c.sourceSheet < sheetNames.size() && !sheetNames[c.sourceSheet].isEmpty()) c.fileSheetName = sheetNames[c.sourceSheet];
    }
}

bool PivotBook::changed() const
{
    for (const PivotTableEntry &t : m_tables)
        if (t.table != t.fileTable) return true;
    for (const PivotCacheEntry &c : m_caches)
        if ((c.tried && c.cache != c.fileCache) || !(c.source == c.fileSource)) return true;
    return false;
}

namespace {

// The number after `stem` in the names of the package's parts ("/xl/pivotTables/pivotTable7.xml" with stem "/xl/pivotTables/pivotTable" is 7), the greatest.
int greatestNumber(const Package *package, const QString &stem, const QStringList &also)
{
    int most = 0;
    auto look = [&](const QString &name) {
        if (!name.startsWith(stem, Qt::CaseInsensitive) || !name.endsWith(QLatin1String(".xml"), Qt::CaseInsensitive)) return;
        bool ok = false;
        const int n = name.mid(stem.size(), name.size() - stem.size() - 4).toInt(&ok);
        if (ok) most = std::max(most, n);
    };
    for (const QString &name : package->partNames()) look(name.startsWith(QLatin1Char('/')) ? name : QLatin1Char('/') + name);
    for (const QString &name : also) look(name);
    return most;
}

} // namespace

int PivotBook::addCache(std::shared_ptr<const PivotCache> cache, const CellRange &source, int sourceSheet, const QString &sourceName, const QByteArray &created, const QString &sourceSheetName)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    PivotCacheEntry c;
    QStringList used;
    for (const PivotCacheEntry &o : m_caches) used << o.definitionPart << o.recordsPart;
    const int n = greatestNumber(m_package, QStringLiteral("/xl/pivotCache/pivotCacheDefinition"), used) + 1;
    const int r = greatestNumber(m_package, QStringLiteral("/xl/pivotCache/pivotCacheRecords"), used) + 1;
    c.definitionPart = QStringLiteral("/xl/pivotCache/pivotCacheDefinition%1.xml").arg(std::max(n, r));
    c.recordsPart = QStringLiteral("/xl/pivotCache/pivotCacheRecords%1.xml").arg(std::max(n, r));
    c.cache = std::move(cache);
    c.tried = true;
    c.source = source;
    c.sourceSheet = sourceSheet;
    c.sourceName = sourceName;
    c.isNew = true;
    c.created = created;
    c.fileSheetName = sourceSheetName;   // (what the definition names if the sheet is deleted before the save)
    m_caches.push_back(std::move(c));
    return int(m_caches.size()) - 1;
}

int PivotBook::addTable(int sheet, std::shared_ptr<const PivotTable> table, int cache, const QByteArray &created)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    PivotTableEntry e;
    QStringList used;
    for (const PivotTableEntry &o : m_tables) used << o.part;
    const int n = greatestNumber(m_package, QStringLiteral("/xl/pivotTables/pivotTable"), used) + 1;
    e.sheet = sheet;
    e.part = QStringLiteral("/xl/pivotTables/pivotTable%1.xml").arg(n);
    e.cache = cache;
    e.table = std::move(table);
    e.loaded = true;
    e.isNew = true;
    e.created = created;
    m_tables.push_back(std::move(e));
    return int(m_tables.size()) - 1;
}

void PivotBook::removeLastTable()
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (!m_tables.empty()) m_tables.pop_back();
}

void PivotBook::removeLastCache()
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (!m_caches.empty()) m_caches.pop_back();
}

std::vector<PivotBook::AddedParts> PivotBook::addedParts(const QStringList &sheetNames) const
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    std::vector<AddedParts> out;
    QSet<int> cachesSeen;
    for (size_t i = 0; i < m_tables.size(); ++i) {
        const PivotTableEntry &t = m_tables[i];
        if (!t.isNew || t.cache < 0) continue;
        if (t.sheet < 0 || t.sheet >= sheetNames.size() || sheetNames[t.sheet].isEmpty()) continue;   // (on a sheet that was deleted: nothing of it is written)
        AddedParts a;
        a.table = int(i);
        a.cache = t.cache;
        a.sheet = t.sheet;
        a.tablePart = t.part;
        const PivotCacheEntry &c = m_caches[size_t(t.cache)];
        a.definitionPart = c.definitionPart;
        a.recordsPart = c.recordsPart;
        a.newCache = c.isNew && !cachesSeen.contains(t.cache);
        cachesSeen.insert(t.cache);
        out.push_back(a);
    }
    return out;
}

std::function<QByteArray(int)> PivotBook::tableProducer(int table) const
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    const PivotTableEntry &t = m_tables[size_t(table)];
    const QByteArray original = t.created;
    const std::shared_ptr<const PivotTable> now = t.table;
    return [original, now](int cacheId) {
        QByteArray xml = original;
        xml.replace("@CACHEID@", QByteArray::number(cacheId));
        const QByteArray made = writePivotTable(xml, *now, false);
        return made.isEmpty() ? xml : made;
    };
}

std::function<QByteArray()> PivotBook::definitionProducer(int cache, const QStringList &sheetNames) const
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    const PivotCacheEntry &c = m_caches[size_t(cache)];
    const QByteArray original = c.created;
    const std::shared_ptr<const PivotCache> now = c.cache;
    QString sheet, ref, name;
    if (c.sourceSheet >= 0 && c.source.valid && c.sourceName.isEmpty()) {
        // (a source sheet that is deleted by the time of the save is still what the definition names, as the programs people know leave it: the table shows what it did, and can't be refreshed)
        sheet = c.sourceSheet < sheetNames.size() && !sheetNames[c.sourceSheet].isEmpty() ? sheetNames[c.sourceSheet] : c.fileSheetName;
        ref = pivotRangeRef(c.source.row1, c.source.col1, c.source.row2, c.source.col2);
    } else {
        name = c.sourceName;
    }
    const double unixDays = double(QDateTime::currentDateTimeUtc().toSecsSinceEpoch()) / 86400.0;
    const jp::calc::DateSystem dates = m_dates;
    const double day = unixDays + (dates == jp::calc::DateSystem::Date1904 ? 24107.0 : 25569.0);
    return [original, now, sheet, ref, name, day, dates] {
        PivotCache made = *now;
        made.sheet = sheet;
        made.ref = ref;
        made.name = name;
        const QByteArray def = writePivotCacheDefinition(original, made, made.recordCount, day, dates);
        return def.isEmpty() ? original : def;
    };
}

std::function<QByteArray()> PivotBook::recordsProducer(int cache) const
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    const std::shared_ptr<const PivotCache> now = m_caches[size_t(cache)].cache;
    const jp::calc::DateSystem dates = m_dates;
    return [now, dates] { return writePivotCacheRecords(*now, dates); };
}

std::map<QString, std::function<QByteArray()>> PivotBook::changedPartProducers(const QStringList &sheetNames) const
{
    std::map<QString, std::function<QByteArray()>> out;
    for (const PivotTableEntry &t : m_tables) {
        if (t.isNew || !t.table || t.table == t.fileTable) continue;
        const QByteArray original = m_package->part(t.part);
        const std::shared_ptr<const PivotTable> table = t.table;
        const bool structure = t.structureChanged;
        out[t.part] = [original, table, structure] {
            const QByteArray made = writePivotTable(original, *table, structure);
            return made.isEmpty() ? original : made;   // (what can't be made is the file's own part)
        };
    }
    // the date a refresh is made, in the workbook's own count of days
    const double unixDays = double(QDateTime::currentDateTimeUtc().toSecsSinceEpoch()) / 86400.0;
    const jp::calc::DateSystem dates = m_dates;
    const double now = unixDays + (dates == jp::calc::DateSystem::Date1904 ? 24107.0 : 25569.0);
    for (const PivotCacheEntry &c : m_caches) {
        if (c.isNew) continue;
        const bool sourceMoved = c.sourceSheet >= 0 && c.sourceSheet < sheetNames.size() && !sheetNames[c.sourceSheet].isEmpty() && c.source.valid && c.fileSource.valid
                                 && (!(c.source == c.fileSource) || sheetNames[c.sourceSheet].compare(c.fileSheetName, Qt::CaseInsensitive) != 0);
        if (c.tried && c.cache && c.cache != c.fileCache) {
            const QByteArray original = m_package->part(c.definitionPart);
            const std::shared_ptr<const PivotCache> cache = c.cache;
            QString sheet, ref, name;
            if (c.sourceSheet >= 0 && c.source.valid && c.sourceName.isEmpty()) {
                sheet = c.sourceSheet < sheetNames.size() && !sheetNames[c.sourceSheet].isEmpty() ? sheetNames[c.sourceSheet] : c.fileSheetName;   // (a deleted sheet: the name the file had)
                ref = pivotRangeRef(c.source.row1, c.source.col1, c.source.row2, c.source.col2);
            } else {
                name = c.sourceName;
            }
            const bool hasRecords = !c.recordsPart.isEmpty();
            out[c.definitionPart] = [original, cache, sheet, ref, name, now, dates] {
                PivotCache made = *cache;
                if (!sheet.isEmpty()) {
                    made.sheet = sheet;
                    made.ref = ref;
                    made.name.clear();
                } else {
                    made.name = name;
                }
                const QByteArray def = writePivotCacheDefinition(original, made, made.recordCount, now, dates);
                return def.isEmpty() ? original : def;
            };
            if (hasRecords) out[c.recordsPart] = [cache, dates] { return writePivotCacheRecords(*cache, dates); };
        } else if (sourceMoved) {
            const QByteArray original = m_package->part(c.definitionPart);
            const QString sheet = sheetNames[c.sourceSheet], ref = pivotRangeRef(c.source.row1, c.source.col1, c.source.row2, c.source.col2);
            out[c.definitionPart] = [original, sheet, ref] { return patchPivotCacheSource(original, sheet, ref); };
        }
    }
    return out;
}

std::map<QString, QByteArray> PivotBook::changedParts(const QStringList &sheetNames) const
{
    std::map<QString, QByteArray> out;
    for (const auto &[part, make] : changedPartProducers(sheetNames)) {
        const QByteArray bytes = make();
        if (!bytes.isEmpty()) out[part] = bytes;
    }
    return out;
}

} // namespace jp::office::sheet
