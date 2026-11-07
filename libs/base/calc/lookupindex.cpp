#include "calc/lookupindex.h"
#include "calc/machine.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace jp::calc {

const Value LineSnapshot::s_empty;

const Value &LineSnapshot::at(qint64 i) const
{
    if (i < 0 || i >= m_count) return s_empty;
    if (m_dense) return m_values[size_t(i)];
    const auto it = std::lower_bound(m_places.begin(), m_places.end(), qint32(i));
    if (it == m_places.end() || *it != i) return s_empty;
    return m_values[size_t(it - m_places.begin())];
}

qint64 LineSnapshot::nextFilled(qint64 i, qint64 last) const
{
    if (i > last) return last + 1;
    if (i < 0) i = 0;
    if (m_dense) return i < m_count && i <= last ? i : last + 1;
    const auto it = std::lower_bound(m_places.begin(), m_places.end(), qint32(std::min<qint64>(i, m_count)));
    if (it == m_places.end() || *it > last) return last + 1;
    return *it;
}

qint64 LineSnapshot::previousFilled(qint64 i, qint64 first) const
{
    if (i < first) return first - 1;
    if (i >= m_count) i = m_count - 1;
    if (i < first) return first - 1;
    if (m_dense) return i;
    const auto it = std::upper_bound(m_places.begin(), m_places.end(), qint32(i));
    if (it == m_places.begin() || *(it - 1) < first) return first - 1;
    return *(it - 1);
}

namespace {

bool isAscii(const QString &s)
{
    for (const QChar ch : s)
        if (ch.unicode() > 127) return false;
    return true;
}

quint64 numberBits(double v)
{
    if (v == 0) v = 0.0;   // (-0 is 0)
    quint64 bits;
    std::memcpy(&bits, &v, sizeof bits);
    return bits;
}

constexpr qsizetype kMostWideTexts = 16;

} // namespace

qint64 LineSnapshot::firstEqualRank(const Value &key, bool *usable) const
{
    *usable = true;
    if (key.isNumber()) {
        if (std::isnan(key.number())) return 0;
        const auto it = m_numbers.constFind(numberBits(key.number()));
        return it == m_numbers.constEnd() ? 0 : *it;
    }
    if (key.isBool()) return m_firstBool[key.boolean() ? 1 : 0];
    if (key.isText()) {
        const QString &text = key.text();
        if (!isAscii(text) || hasWildcards(text) || m_wideTexts.size() > size_t(kMostWideTexts)) {
            *usable = false;
            return 0;
        }
        qint64 rank = 0;
        const auto it = m_texts.constFind(text.toLower());
        if (it != m_texts.constEnd()) rank = *it;
        // (a text with a character that is not ASCII may be the same as an ASCII text under the order of texts: those are compared one by one, those before the hit only)
        for (const qint32 r : m_wideTexts) {
            if (rank && r > rank) break;
            if (compareText(text, m_values[size_t(r - 1)].text()) == 0) {
                rank = r;
                break;
            }
        }
        return rank;
    }
    return 0;   // (nothing but a number, a text, or true or false is equal to a cell)
}

const LineSnapshot *LookupCache::find(const CellSource &source, const LineKey &key)
{
    LineToken token;
    if (!source.constantsToken(key.sheet, &token)) return nullptr;
    ++m_clock;
    auto it = m_entries.find(key);
    if (it != m_entries.end()) {
        Entry &e = it->second;
        if (e.token == token) {
            e.used = m_clock;
            if (e.snapshot) {
                ++m_stats.hits;
                return e.snapshot.get();
            }
            if (e.refused) return nullptr;
            // the second search of the line: the index is made now
            ++e.seen;
        } else {
            // the cells changed: what was held is stale, and the line starts again as a line seen once
            if (e.snapshot) {
                m_bytes -= qint64(e.snapshot->bytes());
                --m_built;
                e.snapshot.reset();
                ++m_stats.dropped;
            }
            e.token = token;
            e.seen = 1;
            e.refused = false;
            e.used = m_clock;
            return nullptr;
        }
    } else {
        if (m_entries.size() >= size_t(kMaxSeen)) {
            // (the line searched longest ago is forgotten, with its index if it has one)
            auto oldest = m_entries.begin();
            for (auto i = m_entries.begin(); i != m_entries.end(); ++i)
                if (i->second.used < oldest->second.used) oldest = i;
            if (oldest->second.snapshot) {
                m_bytes -= qint64(oldest->second.snapshot->bytes());
                --m_built;
                ++m_stats.evicted;
            }
            m_entries.erase(oldest);
        }
        Entry fresh;
        fresh.key = key;
        fresh.token = token;
        fresh.seen = 1;
        fresh.used = m_clock;
        m_entries.emplace(key, std::move(fresh));
        return nullptr;
    }
    Entry &e = it->second;
    if (key.count < kMinPlaces || key.count > qint64(INT32_MAX) / 2) {
        e.refused = true;
        return nullptr;
    }
    auto snap = std::make_unique<LineSnapshot>();
    snap->m_count = key.count;
    const quint32 last = quint32(qint64(key.from) + key.count - 1);
    qint64 walk;
    bool tooBig = false;
    auto take = [&](quint32 row, quint32 col, const Value &v) {
        if (v.isEmpty()) return true;
        const qint64 place = key.column ? qint64(row) - qint64(key.from) : qint64(col) - qint64(key.from);
        const qint32 rank = qint32(snap->m_places.size()) + 1;
        snap->m_places.push_back(qint32(place));
        snap->m_values.push_back(v);
        snap->m_bytes += sizeof(Value) + sizeof(qint32) + size_t(v.isText() ? v.text().size() * 2 : 0);
        if (v.isNumber()) {
            if (!snap->m_numbers.contains(numberBits(v.number()))) {
                snap->m_numbers.insert(numberBits(v.number()), rank);
                snap->m_bytes += 48;
            }
        } else if (v.isBool()) {
            if (!snap->m_firstBool[v.boolean() ? 1 : 0]) snap->m_firstBool[v.boolean() ? 1 : 0] = rank;
        } else if (v.isText()) {
            if (isAscii(v.text())) {
                const QString lower = v.text().toLower();
                if (!snap->m_texts.contains(lower)) {
                    snap->m_bytes += 64 + size_t(lower.size()) * 2;
                    snap->m_texts.insert(lower, rank);
                }
            } else {
                snap->m_wideTexts.push_back(rank);
            }
        }
        if (qint64(snap->m_bytes) > kMaxBytes / 2) {
            tooBig = true;
            return false;
        }
        return true;
    };
    if (key.column) walk = source.visitConstants(key.sheet, key.from, quint32(key.fixed), last, quint32(key.fixed), take);
    else walk = source.visitConstants(key.sheet, quint32(key.fixed), key.from, quint32(key.fixed), last, take);
    if (walk < 0 || tooBig) {
        e.refused = true;
        ++m_stats.refused;
        return nullptr;
    }
    snap->m_walkWork = walk;
    snap->m_dense = qint64(snap->m_places.size()) == snap->m_count;
    snap->m_bytes += sizeof(LineSnapshot);
    // room for it: the indexes used longest ago go first
    while (m_bytes + qint64(snap->m_bytes) > kMaxBytes || m_built >= kMaxEntries) {
        auto oldest = m_entries.end();
        for (auto i = m_entries.begin(); i != m_entries.end(); ++i)
            if (i->second.snapshot && (oldest == m_entries.end() || i->second.used < oldest->second.used)) oldest = i;
        if (oldest == m_entries.end()) break;
        m_bytes -= qint64(oldest->second.snapshot->bytes());
        --m_built;
        oldest->second.snapshot.reset();
        oldest->second.refused = false;
        oldest->second.seen = 1;
        ++m_stats.evicted;
    }
    m_bytes += qint64(snap->m_bytes);
    ++m_built;
    ++m_stats.builds;
    e.snapshot = std::move(snap);
    return e.snapshot.get();
}

void LookupCache::clear()
{
    m_entries.clear();
    m_bytes = 0;
    m_built = 0;
}

LookupStats LookupCache::stats() const
{
    LookupStats s = m_stats;
    s.bytes = m_bytes;
    s.entries = m_built;
    return s;
}

} // namespace jp::calc
