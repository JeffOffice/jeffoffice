#include "grid/axis.h"

#include <algorithm>

namespace jp::grid {

namespace {
int boundedSize(int size) { return std::min(std::max(size, 0), Axis::kMaxSize); }
}

Axis::Axis(qint64 count, int defaultSize) : m_count(std::max<qint64>(count, 0)), m_default(std::max(1, std::min(defaultSize, kMaxSize))) {}

void Axis::setCount(qint64 count)
{
    m_count = std::max<qint64>(count, 0);
    // (the places past the last row are not rows)
    while (!m_places.empty() && m_places.back().index >= m_count) m_places.pop_back();
    m_dirty = true;
}

void Axis::setDefaultSize(int size)
{
    m_default = std::max(1, std::min(size, kMaxSize));
    // (a place of the new default size is no place of its own)
    m_places.erase(std::remove_if(m_places.begin(), m_places.end(), [&](const Place &p) { return p.size == m_default; }), m_places.end());
    m_dirty = true;
}

int Axis::find(qint64 index) const
{
    const auto it = std::lower_bound(m_places.begin(), m_places.end(), index, [](const Place &p, qint64 i) { return p.index < i; });
    return it != m_places.end() && it->index == index ? int(it - m_places.begin()) : -1;
}

void Axis::setSize(qint64 index, int size)
{
    if (index < 0 || index >= m_count) return;
    size = boundedSize(size);
    const auto it = std::lower_bound(m_places.begin(), m_places.end(), index, [](const Place &p, qint64 i) { return p.index < i; });
    const bool have = it != m_places.end() && it->index == index;
    if (size == m_default) {
        if (have) m_places.erase(it);
    } else if (have) {
        it->size = size;
    } else {
        m_places.insert(it, Place{index, size});
    }
    m_dirty = true;
}

void Axis::assign(std::vector<Place> places)
{
    m_places.clear();
    m_places.reserve(places.size());
    qint64 last = -1;
    for (Place &p : places) {
        if (p.index <= last || p.index >= m_count) continue;
        last = p.index;
        p.size = boundedSize(p.size);
        if (p.size != m_default) m_places.push_back(p);
    }
    m_dirty = true;
}

void Axis::rebuild() const
{
    m_before.resize(m_places.size() + 1);
    m_hiddenEnd.resize(m_places.size());
    m_hiddenStart.resize(m_places.size());
    qint64 sum = 0;
    for (size_t k = 0; k < m_places.size(); ++k) {
        m_before[k] = sum;
        sum += m_places[k].size - m_default;
    }
    m_before[m_places.size()] = sum;
    // the end of the run of hidden rows each hidden place begins, from the last place back
    for (size_t k = m_places.size(); k-- > 0;) {
        const Place &p = m_places[k];
        m_hiddenEnd[k] = p.size == 0 && k + 1 < m_places.size() && m_places[k + 1].size == 0 && m_places[k + 1].index == p.index + 1 ? m_hiddenEnd[k + 1] : p.index + 1;
    }
    // and the first row of the run each hidden place is in, from the first place on
    for (size_t k = 0; k < m_places.size(); ++k) {
        const Place &p = m_places[k];
        m_hiddenStart[k] = p.size == 0 && k > 0 && m_places[k - 1].size == 0 && m_places[k - 1].index == p.index - 1 ? m_hiddenStart[k - 1] : p.index;
    }
    m_dirty = false;
}

int Axis::size(qint64 index) const
{
    if (index < 0 || index >= m_count) return 0;
    const int k = find(index);
    return k >= 0 ? m_places[size_t(k)].size : m_default;
}

qint64 Axis::position(qint64 index) const
{
    if (index <= 0) return 0;
    if (index > m_count) index = m_count;
    if (m_dirty || m_before.size() != m_places.size() + 1) rebuild();
    // the places before this row
    qint64 lo = 0, hi = qint64(m_places.size());
    while (lo < hi) {
        ++m_probes;
        const qint64 mid = (lo + hi) / 2;
        if (m_places[size_t(mid)].index < index) lo = mid + 1;
        else hi = mid;
    }
    return index * m_default + m_before[size_t(lo)];
}

qint64 Axis::indexAt(qint64 pos) const
{
    if (pos < 0 || pos >= total()) return -1;
    // the places that begin at or before pos: a place's own start is its index times the default, and what the places before it differ by
    auto startOf = [&](qint64 k) { return m_places[size_t(k)].index * m_default + m_before[size_t(k)]; };
    qint64 lo = 0, hi = qint64(m_places.size());
    while (lo < hi) {
        ++m_probes;
        const qint64 mid = (lo + hi) / 2;
        if (startOf(mid) <= pos) lo = mid + 1;
        else hi = mid;
    }
    if (lo == 0) return pos / m_default;
    const qint64 k = lo - 1, start = startOf(k);
    const Place &p = m_places[size_t(k)];
    if (pos < start + p.size) return p.index;
    // the rows of the default size after that place, up to the next
    return p.index + 1 + (pos - start - p.size) / m_default;
}

qint64 Axis::nextVisible(qint64 index) const
{
    qint64 next = index + 1;
    if (next >= m_count) return m_count;
    if (next < 0) next = 0;
    if (m_dirty || m_before.size() != m_places.size() + 1) rebuild();
    // a hidden row is stepped over with the run it is in
    while (next < m_count) {
        ++m_probes;
        const int k = find(next);
        if (k < 0 || m_places[size_t(k)].size != 0) return next;
        next = m_hiddenEnd[size_t(k)];
    }
    return m_count;
}

qint64 Axis::previousVisible(qint64 index) const
{
    qint64 prev = std::min(index, m_count) - 1;
    if (prev < 0) return -1;
    if (m_dirty || m_before.size() != m_places.size() + 1) rebuild();
    // a hidden row is stepped over with the run it is in, back to the run's first
    while (prev >= 0) {
        ++m_probes;
        const int k = find(prev);
        if (k < 0 || m_places[size_t(k)].size != 0) return prev;
        prev = m_hiddenStart[size_t(k)] - 1;
    }
    return -1;
}

} // namespace jp::grid
