#pragma once
// The pivot tables and pivot caches of a workbook as a person changes them. The tables are read when the workbook is opened (they are small); a cache, with its records, when it is first needed.
// A table or a cache is held in a shared pointer, so that a step of Undo holds what it was and what it is without a copy of the records; a part that was not changed is not written (the file's
// part is kept to the byte), one that was is made from the file's own with the changes.

#include "office/package.h"
#include "sheet/cellref.h"
#include "sheet/pivot.h"
#include "sheet/refshift.h"

#include <map>
#include <memory>
#include <vector>

namespace jp::office::sheet {

struct PivotCacheEntry {
    QString definitionPart, recordsPart;           // the parts, as the package names them (no records part: the file does not keep the data)
    std::shared_ptr<const PivotCache> cache, fileCache;   // as it is now, and as the file has it; null until it is read
    bool tried = false;
    QString why;                                   // when it can't be used
    int sourceSheet = -1;                          // the sheet the source range is on (its place), -1 for a source that is a name or a table, or a sheet that is gone
    CellRange source, fileSource;                  // the range now, and as the file has it
    QString sourceName;                            // a defined name or a table the source is
};

struct PivotTableEntry {
    int sheet = 0;                                 // the sheet the table is on
    QString part;                                  // the table's part
    int cache = -1;                                // its place in the caches
    std::shared_ptr<const PivotTable> table, fileTable;
    bool structureChanged = false;                 // the fields were moved: the formats the file has for the old layout are left out of the part
    QString why;                                   // when the table can't be read
};

class PivotBook {
public:
    // The sheets' worksheet parts, by the places of the sheets (empty for a sheet that is not one).
    PivotBook(const Package *package, const QVector<QString> &sheetParts, const QStringList &sheetNames, jp::calc::DateSystem dates);

    int tableCount() const { return int(m_tables.size()); }
    const PivotTableEntry &table(int i) const { return m_tables[size_t(i)]; }
    int cacheCount() const { return int(m_caches.size()); }
    // The cache, read now if it was not (null, with `why`, when it can't be).
    const PivotCacheEntry &cache(int i) const;
    // The tables of a sheet, by their places.
    std::vector<int> tablesOn(int sheet) const;
    // The tables that are made from a cache.
    std::vector<int> tablesOfCache(int cache) const;
    // The cells the table covers (not valid when it can't be read: then it is taken to be the whole sheet by whoever asks).
    CellRange locationOf(int i) const;
    bool unreadable(int i) const { return !m_tables[size_t(i)].table; }

    // ---- changes (the window's thread). The same call puts a state back for Undo.
    void setTable(int i, std::shared_ptr<const PivotTable> table, bool structureChanged);
    void setCache(int i, std::shared_ptr<const PivotCache> cache, const CellRange &source, int sourceSheet, const QString &sourceName);
    // Rows and columns inserted or deleted: the sources that are ranges follow. Returns what each was, for putSourcesBack.
    struct SourcePlace {
        int cache;
        CellRange was;
    };
    std::vector<SourcePlace> moveSources(const StructEdit &edit);
    void putSourcesBack(const std::vector<SourcePlace> &places);

    // The parts that are not as the file has them, as they will be written (by the part's name). `sheetNames` are the names of the sheets now.
    std::map<QString, QByteArray> changedParts(const QStringList &sheetNames) const;
    bool changed() const;

private:
    void load(int i) const;
    const Package *m_package;
    QStringList m_names;
    jp::calc::DateSystem m_dates;
    std::vector<PivotTableEntry> m_tables;
    mutable std::vector<PivotCacheEntry> m_caches;
};

} // namespace jp::office::sheet
