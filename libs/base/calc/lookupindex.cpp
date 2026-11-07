#include "calc/lookupindex.h"
#include "calc/machine.h"

#include <QThread>

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
constexpr qsizetype kMostLongTexts = 64;

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
        const bool longKey = text.size() >= 32;
        if (!isAscii(text) || hasWildcards(text) || m_wideTexts.size() > size_t(kMostWideTexts) || (longKey && m_longTexts.size() > size_t(kMostLongTexts))) {
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
        // (the walk compares the key with each text on its way, and a long text that agrees with a long key for 32 characters or more takes a step for each 32: the same steps are taken here)
        if (longKey) {
            const qint64 stop = rank ? rank : filledCount();
            for (const qint32 r : m_longTexts) {
                if (r > stop) break;
                if (std::binary_search(m_wideTexts.begin(), m_wideTexts.end(), r)) continue;   // (compared above)
                compareText(text, m_values[size_t(r - 1)].text());
            }
        }
        return rank;
    }
    return 0;   // (nothing but a number, a text, or true or false is equal to a cell)
}

size_t LineSnapshot::estimateBytes() const
{
    size_t n = sizeof(LineSnapshot) + m_places.capacity() * sizeof(qint32) + m_values.capacity() * sizeof(Value) + (m_wideTexts.capacity() + m_longTexts.capacity()) * sizeof(qint32);
    for (const Value &v : m_values)
        if (v.isText()) n += size_t(v.text().capacity()) * 2 + 32;
    n += size_t(m_numbers.capacity()) * 8 + size_t(m_numbers.size()) * 32;
    n += size_t(m_texts.capacity()) * 8;
    for (auto it = m_texts.constBegin(); it != m_texts.constEnd(); ++it) n += 48 + size_t(it.key().capacity()) * 2;
    return n;
}

void LookupCache::dropSnapshot(Entry &e)
{
    if (!e.snapshot) return;
    m_bytes -= qint64(e.snapshot->bytes());
    m_built.erase(e.builtAt);
    e.snapshot.reset();
}

std::unique_ptr<LineSnapshot> LookupCache::build(const CellSource &source, const LineKey &key, bool *refused)
{
    *refused = true;
    auto snap = std::make_unique<LineSnapshot>();
    snap->m_count = key.count;
    const quint32 last = quint32(qint64(key.from) + key.count - 1);
    bool tooBig = false;
    size_t running = 0;
    auto take = [&](quint32 row, quint32 col, const Value &v) {
        if (v.isEmpty()) return true;
        const qint64 place = key.column ? qint64(row) - qint64(key.from) : qint64(col) - qint64(key.from);
        const qint32 rank = qint32(snap->m_places.size()) + 1;
        snap->m_places.push_back(qint32(place));
        snap->m_values.push_back(v);
        running += sizeof(Value) + sizeof(qint32) + size_t(v.isText() ? v.text().capacity() * 2 + 32 : 0);
        if (v.isNumber()) {
            const quint64 bits = numberBits(v.number());
            if (!snap->m_numbers.contains(bits)) {
                snap->m_numbers.insert(bits, rank);
                running += 48;
            }
        } else if (v.isBool()) {
            if (!snap->m_firstBool[v.boolean() ? 1 : 0]) snap->m_firstBool[v.boolean() ? 1 : 0] = rank;
        } else if (v.isText()) {
            if (isAscii(v.text())) {
                const QString lower = v.text().toLower();
                if (!snap->m_texts.contains(lower)) {
                    running += 64 + size_t(lower.size()) * 2;
                    snap->m_texts.insert(lower, rank);
                }
                if (v.text().size() >= 32) snap->m_longTexts.push_back(rank);
            } else {
                snap->m_wideTexts.push_back(rank);
            }
        }
        if (qint64(running) > kMaxBytes / 2) {
            tooBig = true;
            return false;
        }
        return true;
    };
    const qint64 walk = key.column ? source.visitConstants(key.sheet, key.from, quint32(key.fixed), last, quint32(key.fixed), take)
                                   : source.visitConstants(key.sheet, quint32(key.fixed), key.from, quint32(key.fixed), last, take);
    if (walk < 0 || tooBig) return nullptr;
    snap->m_walkWork = walk;
    snap->m_dense = qint64(snap->m_places.size()) == snap->m_count;
    snap->m_places.shrink_to_fit();
    snap->m_values.shrink_to_fit();
    snap->m_bytes = snap->estimateBytes();
    if (qint64(snap->m_bytes) > kMaxBytes / 2) return nullptr;
    *refused = false;
    return snap;
}

const LineSnapshot *LookupCache::find(const CellSource &source, const LineKey &key)
{
    Q_ASSERT(!m_thread || m_thread == QThread::currentThread());
    if (!m_thread) m_thread = QThread::currentThread();
    LineToken token;
    if (key.count < kMinPlaces || key.count > qint64(INT32_MAX) / 2 || !source.constantsToken(key.sheet, &token)) return nullptr;
    auto at = m_index.find(key);
    if (at == m_index.end()) {
        if (m_lru.size() >= size_t(kMaxSeen)) {
            Entry &oldest = m_lru.back();
            dropSnapshot(oldest);
            m_index.erase(oldest.key);
            m_lru.pop_back();
        }
        m_lru.emplace_front();
        Entry &fresh = m_lru.front();
        fresh.key = key;
        fresh.token = token;
        m_index.emplace(key, m_lru.begin());
        return nullptr;
    }
    m_lru.splice(m_lru.begin(), m_lru, at->second);
    Entry &e = m_lru.front();
    if (e.token != token) {
        const quint32 first = e.key.from, lastPlace = quint32(qint64(e.key.from) + e.key.count - 1);
        const bool same = e.token.sameStructure(token)
                          && !source.wroteSince(key.sheet, e.token.e, key.column ? first : quint32(key.fixed), key.column ? quint32(key.fixed) : first, key.column ? lastPlace : quint32(key.fixed),
                                                key.column ? quint32(key.fixed) : lastPlace);
        e.token = token;
        if (!same) {
            // the cells of the line (or the lines) changed: what was held is stale, and the line starts again
            if (e.snapshot) ++m_stats.dropped;
            dropSnapshot(e);
            e.scanned = 0;
            e.refused = false;
            return nullptr;
        }
    }
    if (e.snapshot) {
        ++m_stats.hits;
        m_built.splice(m_built.begin(), m_built, e.builtAt);
        return e.snapshot.get();
    }
    if (e.refused) return nullptr;
    // built once the searches that were read from the cells have cost what a build costs (a line that was thrown out for room, twice that, and so on)
    if (e.scanned < e.key.count * (qint64(1) << std::min(e.evictions, 16))) return nullptr;
    bool refused = false;
    std::unique_ptr<LineSnapshot> snap = build(source, key, &refused);
    if (!snap) {
        e.refused = true;
        ++m_stats.refused;
        return nullptr;
    }
    // room for it: the indexes used longest ago go first
    while ((m_bytes + qint64(snap->bytes()) > kMaxBytes || qint64(m_built.size()) >= kMaxEntries) && !m_built.empty()) {
        Entry &victim = *m_index.at(m_built.back());
        ++victim.evictions;
        victim.scanned = 0;
        dropSnapshot(victim);
        ++m_stats.evicted;
    }
    m_bytes += qint64(snap->bytes());
    m_built.push_front(key);
    e.builtAt = m_built.begin();
    e.snapshot = std::move(snap);
    ++m_stats.builds;
    return e.snapshot.get();
}

void LookupCache::noteScan(const LineKey &key, qint64 steps)
{
    m_stats.scanSteps += steps;
    const auto at = m_index.find(key);
    if (at == m_index.end() || at->second->snapshot) return;
    at->second->scanned += steps;
}

void LookupCache::clear()
{
    m_lru.clear();
    m_index.clear();
    m_built.clear();
    m_bytes = 0;
    m_thread = nullptr;
}

LookupStats LookupCache::stats() const
{
    LookupStats s = m_stats;
    s.bytes = m_bytes;
    s.entries = qint64(m_built.size());
    return s;
}

} // namespace jp::calc
