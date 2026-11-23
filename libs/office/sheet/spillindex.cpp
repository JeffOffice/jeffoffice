#include "sheet/spillindex.h"

#include "sheet/sheetmodel.h"

#include <QThread>

namespace jp::office::sheet {

namespace {
std::atomic<int> g_default{-1};   // -1: not read from the environment yet
}

bool spillIndexDefault()
{
    int v = g_default.load(std::memory_order_relaxed);
    if (v < 0) {
        v = qEnvironmentVariableIntValue("JO_SPILL_INDEX") != 0 ? 1 : 0;
        g_default.store(v, std::memory_order_relaxed);
    }
    return v != 0;
}

void setSpillIndexDefault(bool on) { g_default.store(on ? 1 : 0, std::memory_order_relaxed); }

SpillTable::BucketMap &SpillTable::bucketsForWrite(size_t i)
{
    auto &p = m_buckets[i];
    if (!p) {
        p = std::make_shared<BucketMap>();
    } else if (p.use_count() > 1) {
        m_copiedEntries += qint64(p->size());
        p = std::make_shared<BucketMap>(*p);
    }
    return *p;
}

SpillTable::FormulaMap &SpillTable::formulasForWrite(size_t i)
{
    auto &p = m_formulas[i];
    if (!p) {
        p = std::make_shared<FormulaMap>();
    } else if (p.use_count() > 1) {
        m_copiedEntries += qint64(p->size());
        p = std::make_shared<FormulaMap>(*p);
    }
    return *p;
}

const SpillRect *SpillTable::atInBuckets(quint32 row, quint32 col, qint64 *probes) const
{
    const quint64 k = key(row / kBlockRows, col / kBlockCols);
    const auto &shard = m_buckets[bucketShard(k)];
    if (!shard) return nullptr;
    const auto list = shard->find(k);
    if (list == shard->end()) return nullptr;
    for (const SpillRect &r : list->second) {
        if (probes) ++*probes;
        if (r.contains(row, col)) return &r;
    }
    return nullptr;
}

const SpillRect *SpillTable::ofFormula(qint32 formula) const
{
    const auto &shard = m_formulas[formulaShard(formula)];
    if (!shard) return nullptr;
    const auto at = shard->find(formula);
    return at == shard->end() ? nullptr : &at->second;
}

void SpillTable::erase(qint32 formula)
{
    const SpillRect *have = ofFormula(formula);
    if (!have) return;
    const SpillRect r = *have;
    for (quint32 rb = r.row1 / kBlockRows; rb <= r.row2 / kBlockRows; ++rb)
        for (quint32 cb = r.col1 / kBlockCols; cb <= r.col2 / kBlockCols; ++cb) {
            const quint64 k = key(rb, cb);
            BucketMap &map = bucketsForWrite(bucketShard(k));
            const auto list = map.find(k);
            if (list == map.end()) continue;
            list->second.erase(std::remove_if(list->second.begin(), list->second.end(), [&](const SpillRect &x) { return x.formula == formula; }), list->second.end());
            if (list->second.empty()) {
                map.erase(list);
                --m_bucketKeys;
            }
        }
    m_bucketsInAll -= (qint64(r.row2 / kBlockRows) - r.row1 / kBlockRows + 1) * (qint64(r.col2 / kBlockCols) - r.col1 / kBlockCols + 1);
    formulasForWrite(formulaShard(formula)).erase(formula);
    --m_size;
    if (m_size < kSmall) {
        for (size_t i = 0; i <= m_size; ++i)
            if (m_small[i].formula == formula) {
                m_small[i] = m_small[m_size];
                break;
            }
    } else if (m_size == kSmall) {
        rebuildSmall();
    }
}

void SpillTable::rebuildSmall()
{
    size_t n = 0;
    for (const auto &shard : m_formulas)
        if (shard)
            for (const auto &[formula, r] : *shard)
                if (n < kSmall) m_small[n++] = r;
}

bool SpillTable::set(const SpillRect &r)
{
    erase(r.formula);
    if (r.row2 < r.row1 || r.col2 < r.col1) return false;
    const qint64 buckets = (qint64(r.row2 / kBlockRows) - r.row1 / kBlockRows + 1) * (qint64(r.col2 / kBlockCols) - r.col1 / kBlockCols + 1);
    if (buckets > kMaxBuckets || m_bucketsInAll + buckets > kMaxTableBuckets) return false;
    m_bucketsInAll += buckets;
    formulasForWrite(formulaShard(r.formula))[r.formula] = r;
    ++m_size;
    if (m_size <= kSmall) m_small[m_size - 1] = r;
    for (quint32 rb = r.row1 / kBlockRows; rb <= r.row2 / kBlockRows; ++rb)
        for (quint32 cb = r.col1 / kBlockCols; cb <= r.col2 / kBlockCols; ++cb) {
            const quint64 k = key(rb, cb);
            BucketMap &map = bucketsForWrite(bucketShard(k));
            const auto at = map.find(k);
            if (at == map.end()) {
                map[k].push_back(r);
                ++m_bucketKeys;
            } else {
                at->second.push_back(r);
            }
        }
    m_firstRow = std::min(m_firstRow, r.row1);
    m_firstCol = std::min(m_firstCol, r.col1);
    m_lastRow = std::max(m_lastRow, r.row2);
    m_lastCol = std::max(m_lastCol, r.col2);
    return true;
}

bool SpillTable::meetsOther(quint32 r1, quint32 c1, quint32 r2, quint32 c2, qint32 except, qint64 *probes) const
{
    bool found = false;
    forEachMeeting(r1, c1, r2, c2, [&](const SpillRect &r) {
        if (r.formula == except) return true;
        found = true;
        return false;
    }, probes);
    return found;
}

std::shared_ptr<const SpillTable> SpillView::frozen() const
{
    if (!table) return nullptr;
    if (keep) return keep;
    return std::make_shared<const SpillTable>(*table);
}

void SpillPublisher::follow(const SpillTable *table)
{
    m_mine = table ? *table : SpillTable();
    noteChanged();
    if (m_guard && !m_guard->isWorkerOrNone()) publish();
}

void SpillPublisher::publish()
{
    if (!m_changed) return;
    m_changed = false;
    auto copy = std::make_shared<const SpillTable>(m_mine);
    {
        std::lock_guard<std::mutex> lock(m_lock);
        m_published = std::move(copy);
        m_staged.clear();   // (what the others read is newer than any copy kept apart)
    }
    m_anyPublished.store(!m_mine.empty(), std::memory_order_release);
    m_publications.fetch_add(1, std::memory_order_relaxed);
}

void SpillPublisher::stage(qint64 seq)
{
    if (!m_changed) return;
    m_changed = false;
    auto copy = std::make_shared<const SpillTable>(m_mine);
    {
        std::lock_guard<std::mutex> lock(m_lock);
        m_staged.emplace_back(seq, std::move(copy));
        if (m_staged.size() > kMaxStaged) m_staged.erase(m_staged.begin());
    }
    m_publications.fetch_add(1, std::memory_order_relaxed);
}

void SpillPublisher::adopt(qint64 seq)
{
    std::lock_guard<std::mutex> lock(m_lock);
    size_t n = 0;
    while (n < m_staged.size() && m_staged[n].first <= seq) ++n;
    if (n == 0) return;
    m_published = m_staged[n - 1].second;
    m_anyPublished.store(!m_published->empty(), std::memory_order_release);
    m_staged.erase(m_staged.begin(), m_staged.begin() + qsizetype(n));
}

qint64 SpillPublisher::stagedCount() const
{
    std::lock_guard<std::mutex> lock(m_lock);
    return qint64(m_staged.size());
}

std::shared_ptr<const SpillTable> SpillPublisher::published() const
{
    std::lock_guard<std::mutex> lock(m_lock);
    return m_published;
}

SpillPublisher::SpillPublisher(const MutationGuard *guard) : m_guard(guard), m_workerAtom(guard ? guard->workerAtom() : nullptr) {}

bool SpillPublisher::isThread(quintptr thread) { return thread == quintptr(QThread::currentThreadId()); }

std::shared_ptr<const SpillTable> SpillPublisher::frozen() const
{
    if (m_guard && m_guard->heldHere()) return m_mine.empty() ? nullptr : std::make_shared<const SpillTable>(m_mine);
    return view().frozen();
}

SpillView SpillPublisher::view() const
{
    SpillView v;
    if (!m_live.load(std::memory_order_relaxed)) return v;
    if (!m_guard || m_guard->isWorkerOrNone()) {
        if (!m_mine.empty()) v.table = &m_mine;
        return v;
    }
    if (!m_anyPublished.load(std::memory_order_acquire)) return v;
    v.keep = published();
    if (v.keep && !v.keep->empty()) v.table = v.keep.get();
    return v;
}

} // namespace jp::office::sheet
