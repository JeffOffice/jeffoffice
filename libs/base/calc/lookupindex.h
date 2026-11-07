#pragma once
// An index of one column or row of constants that lookups search again and again (VLOOKUP, HLOOKUP, MATCH, and LOOKUP over the same range): built from the second search
// of the same line, it answers an exact search by a hash and lets a search in sorted data halve a vector it holds, instead of reading the sheet's cells place by place.
//
// What an answer is does not change: the index holds the very values the cells give, and the searches of fn_lookup.cpp compare the same ways. A line is indexed only if every
// cell of it is a plain constant (no formula, no cell of an array, nothing the file marked); the source says so as it is read (CellSource::visitConstants), and gives a token
// that changes with any write to the sheet's cells or any edit of its rows or columns (CellSource::constantsToken). An index whose token is not the source's now is dropped
// at the next search. The indexes together take at most kMaxBytes, and at most kMaxEntries of them are held (the one used longest ago goes first). No GUI. One thread at a time.

#include "calc/eval.h"

#include <QHash>

class QThread;

#include <list>
#include <memory>
#include <unordered_map>
#include <vector>

namespace jp::calc {

// A column (or a row) of a sheet: the cells from `from` for `count` places along the line `fixed` (the column when `column`, else the row).
struct LineKey {
    qint32 sheet = 0;
    bool column = true;
    quint32 fixed = 0, from = 0;
    qint64 count = 0;
    bool operator==(const LineKey &o) const { return sheet == o.sheet && column == o.column && fixed == o.fixed && from == o.from && count == o.count; }
};

class LineSnapshot {
public:
    // The value at place i of the line (empty for a place that holds nothing, or is outside).
    const Value &at(qint64 i) const;
    // The first place from i up to `last` that holds something, or last + 1; the last place from i down to `first` that does, or first - 1.
    qint64 nextFilled(qint64 i, qint64 last) const;
    qint64 previousFilled(qint64 i, qint64 first) const;
    qint64 filledCount() const { return qint64(m_places.size()); }
    // For an exact search: the number of places that hold something up to and including the first one equal to the key (as a lookup compares: numbers exactly, texts without regard to
    // case, true and false), found by hash where the key allows it; 0 for none. `*usable` false when the key must be searched for place by place (a pattern, or a text the hash cannot
    // stand for); the answer is then not given.
    qint64 firstEqualRank(const Value &key, bool *usable) const;
    // The place of the n-th (from 1) place that holds something.
    qint64 placeOfRank(qint64 rank) const { return m_places[size_t(rank - 1)]; }
    const Value &valueOfRank(qint64 rank) const { return m_values[size_t(rank - 1)]; }
    // The work the walk that built this took to count, in the steps the sheet counts for a walk over its cells, for all of the filled places (an exact search that stops early takes its part).
    qint64 walkWork() const { return m_walkWork; }
    size_t bytes() const { return m_bytes; }
    size_t estimateBytes() const;

private:
    friend class LookupCache;
    qint64 m_count = 0;
    bool m_dense = false;               // every place holds something: place i is rank i + 1
    std::vector<qint32> m_places;       // the places that hold something, in order
    std::vector<Value> m_values;        // and what they hold
    QHash<quint64, qint32> m_numbers;   // number (its bits; -0 as 0) -> rank of the first
    qint32 m_firstBool[2] = {0, 0};
    QHash<QString, qint32> m_texts;       // text of ASCII characters only, in lower case -> rank of the first
    std::vector<qint32> m_wideTexts;    // ranks of texts that have a character that is not ASCII, which are compared one by one
    std::vector<qint32> m_longTexts;    // ranks of the texts of ASCII of 32 characters or more: a long key compared with them takes steps (compareText), which an indexed search takes too
    qint64 m_walkWork = 0;
    size_t m_bytes = 0;
    static const Value s_empty;
};

struct LookupStats {
    qint64 builds = 0;         // indexes made
    qint64 hits = 0;           // searches an index served
    qint64 dropped = 0;        // indexes thrown away because their token was not the source's now
    qint64 evicted = 0;        // indexes thrown away to keep within the bounds
    qint64 refused = 0;        // lines that could not be indexed (a formula in them, or too big)
    qint64 scanSteps = 0;      // steps of searches that were not served by an index
    qint64 bytes = 0;          // held now
    qint64 entries = 0;        // indexes held now
};

class LookupCache {
public:
    static constexpr qint64 kMaxBytes = qint64(128) << 20;
    static constexpr qint64 kMaxEntries = 64;     // indexes held
    static constexpr qint64 kMaxSeen = 4096;      // lines remembered (a few bytes each)
    static constexpr qint64 kMinPlaces = 32;      // a line shorter than this is read as it was: a build would not pay

    // The index of the line, or null. A line is indexed once the searches of it that were not served by an index have cost (noteScan) as much as a build would, and again only after
    // more, if the bounds had it thrown out: a line that does not stay is not built over and over. The pointer holds until the next call.
    const LineSnapshot *find(const CellSource &source, const LineKey &key);
    // Told, after a search of a line that find() gave no index for, the steps the search took.
    void noteScan(const LineKey &key, qint64 steps);
    void clear();
    LookupStats stats() const;

private:
    struct KeyHash {
        size_t operator()(const LineKey &k) const
        {
            quint64 h = quint64(k.sheet) * 0x9E3779B97F4A7C15ull;
            h ^= (quint64(k.fixed) << 20) ^ (quint64(k.from) << 3) ^ quint64(k.count) * 0xC2B2AE3D27D4EB4Full ^ (k.column ? 1 : 0);
            return size_t(h ^ (h >> 29));
        }
    };
    struct Entry {
        LineKey key;
        LineToken token;
        std::unique_ptr<LineSnapshot> snapshot;
        std::list<LineKey>::iterator builtAt;
        qint64 scanned = 0;      // steps of the searches since the cells were last written (or the index thrown out)
        int evictions = 0;       // times the index was thrown out for room: each doubles what the line must cost before it is built again
        bool refused = false;
    };
    using Lru = std::list<Entry>;
    void dropSnapshot(Entry &e);
    std::unique_ptr<LineSnapshot> build(const CellSource &source, const LineKey &key, bool *refused);
    Lru m_lru;                                          // most recently used first
    std::unordered_map<LineKey, Lru::iterator, KeyHash> m_index;
    std::list<LineKey> m_built;                         // the lines that have an index, most recently used first
    qint64 m_bytes = 0;
    LookupStats m_stats;
    QThread *m_thread = nullptr;                        // the thread that asked (a debug build checks that it is the only one)
};

} // namespace jp::calc
