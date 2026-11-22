#include "sheet/spillindex.h"

#include "sheet/sheetmodel.h"

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

const SpillRect *SpillTable::at(quint32 row, quint32 col, qint64 *probes) const
{
    const auto list = m_buckets.find(key(row / kBlockRows, col / kBlockCols));
    if (list == m_buckets.end()) return nullptr;
    for (const qint32 i : list->second) {
        if (probes) ++*probes;
        if (m_rects[size_t(i)].contains(row, col)) return &m_rects[size_t(i)];
    }
    return nullptr;
}

const SpillRect *SpillTable::ofFormula(qint32 formula) const
{
    const auto at = m_byFormula.find(formula);
    return at == m_byFormula.end() ? nullptr : &m_rects[size_t(at->second)];
}

void SpillTable::erase(qint32 formula)
{
    const auto at = m_byFormula.find(formula);
    if (at == m_byFormula.end()) return;
    const qint32 i = at->second;
    const SpillRect r = m_rects[size_t(i)];
    for (quint32 rb = r.row1 / kBlockRows; rb <= r.row2 / kBlockRows; ++rb)
        for (quint32 cb = r.col1 / kBlockCols; cb <= r.col2 / kBlockCols; ++cb) {
            const auto list = m_buckets.find(key(rb, cb));
            if (list == m_buckets.end()) continue;
            list->second.erase(std::remove(list->second.begin(), list->second.end(), i), list->second.end());
            if (list->second.empty()) m_buckets.erase(list);
        }
    m_bucketsInAll -= (qint64(r.row2 / kBlockRows) - r.row1 / kBlockRows + 1) * (qint64(r.col2 / kBlockCols) - r.col1 / kBlockCols + 1);
    m_rects[size_t(i)].formula = -1;
    m_free.push_back(i);
    m_byFormula.erase(at);
}

bool SpillTable::set(const SpillRect &r)
{
    erase(r.formula);
    if (r.row2 < r.row1 || r.col2 < r.col1) return false;
    const qint64 buckets = (qint64(r.row2 / kBlockRows) - r.row1 / kBlockRows + 1) * (qint64(r.col2 / kBlockCols) - r.col1 / kBlockCols + 1);
    if (buckets > kMaxBuckets || m_bucketsInAll + buckets > kMaxTableBuckets) return false;
    m_bucketsInAll += buckets;
    qint32 i;
    if (!m_free.empty()) {
        i = m_free.back();
        m_free.pop_back();
        m_rects[size_t(i)] = r;
    } else {
        i = qint32(m_rects.size());
        m_rects.push_back(r);
    }
    m_byFormula[r.formula] = i;
    for (quint32 rb = r.row1 / kBlockRows; rb <= r.row2 / kBlockRows; ++rb)
        for (quint32 cb = r.col1 / kBlockCols; cb <= r.col2 / kBlockCols; ++cb) m_buckets[key(rb, cb)].push_back(i);
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
    }
    m_anyPublished.store(!m_mine.empty(), std::memory_order_release);
    m_publications.fetch_add(1, std::memory_order_relaxed);
}

std::shared_ptr<const SpillTable> SpillPublisher::published() const
{
    std::lock_guard<std::mutex> lock(m_lock);
    return m_published;
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
