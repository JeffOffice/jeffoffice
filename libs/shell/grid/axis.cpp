#include "grid/axis.h"

#include <algorithm>

namespace jp::grid {

namespace {
int boundedSize(int size) { return std::min(std::max(size, 0), Axis::kMaxSize); }
}

struct Axis::Data {
    std::vector<Place> places;
    std::vector<qint64> before;      // for place k, how much the places before it differ from the default, together: the sum of (size - default)
    std::vector<qint64> hiddenEnd;   // for a hidden place, the index of the first row after the run of hidden rows it begins (its own index + 1 for a place of its own size)
    std::vector<qint64> hiddenStart; // for a hidden place, the index of the first row of the run of hidden rows it is in
    bool dirty = false;
    // where a build in slices has got to (the places that have been passed in the first pass, and those in the second, which goes back from the last)
    bool building = false;
    size_t forward = 0, backward = 0;
    qint64 sum = 0;
    qint64 assignLast = -1;   // the index of the last place appended by assign, kept or dropped
};

Axis::Axis(qint64 count, int defaultSize) : m_count(std::max<qint64>(count, 0)), m_default(std::max(1, std::min(defaultSize, kMaxSize))), m_d(std::make_shared<Data>()) {}

// A copy shares the places (and what is worked out from them) with the axis it is made from: both are worked out first, so that what is shared is never changed by being asked.
Axis::Axis(const Axis &o) : m_count(o.m_count), m_skip(o.m_skip), m_default(o.m_default), m_d(o.m_d), m_probes(o.m_probes)
{
    o.ensureBuilt();
}

Axis &Axis::operator=(const Axis &o)
{
    if (this != &o) {
        o.ensureBuilt();
        m_count = o.m_count;
        m_skip = o.m_skip;
        m_default = o.m_default;
        m_d = o.m_d;
        m_probes = o.m_probes;
    }
    return *this;
}

Axis::~Axis() = default;

bool Axis::sharesStorageWith(const Axis &other) const { return m_d == other.m_d; }

// What a change is made to: the places, as only this axis has them (they are copied first if another axis holds them too).
Axis::Data &Axis::writable()
{
    if (m_d.use_count() > 1) m_d = std::make_shared<Data>(*m_d);
    return *m_d;
}

int Axis::placeCount() const { return int(m_d->places.size()); }

void Axis::setCount(qint64 count)
{
    m_count = std::max<qint64>(count, 0);
    m_skip = std::min(m_skip, m_count);
    // (the places past the last row are not rows)
    Data &d = writable();
    while (!d.places.empty() && d.places.back().index >= m_count) d.places.pop_back();
    d.dirty = true;
    d.building = false;
}

void Axis::setDefaultSize(int size)
{
    m_default = std::max(1, std::min(size, kMaxSize));
    // (a place of the new default size is no place of its own)
    Data &d = writable();
    d.places.erase(std::remove_if(d.places.begin(), d.places.end(), [&](const Place &p) { return p.size == m_default; }), d.places.end());
    d.dirty = true;
    d.building = false;
}

int Axis::find(qint64 index) const
{
    const std::vector<Place> &places = m_d->places;
    const auto it = std::lower_bound(places.begin(), places.end(), index, [](const Place &p, qint64 i) { return p.index < i; });
    return it != places.end() && it->index == index ? int(it - places.begin()) : -1;
}

void Axis::setSize(qint64 index, int size)
{
    if (index < 0 || index >= m_count) return;
    size = boundedSize(size);
    Data &d = writable();
    const auto it = std::lower_bound(d.places.begin(), d.places.end(), index, [](const Place &p, qint64 i) { return p.index < i; });
    const bool have = it != d.places.end() && it->index == index;
    if (size == m_default) {
        if (have) d.places.erase(it);
    } else if (have) {
        it->size = size;
    } else {
        d.places.insert(it, Place{index, size});
    }
    d.dirty = true;
    d.building = false;
}

void Axis::assign(std::vector<Place> places)
{
    beginAssign(places.size());
    for (const Place &p : places) appendPlace(p);
}

void Axis::beginAssign(size_t expected)
{
    Data &d = writable();
    d.places.clear();
    d.places.reserve(expected);
    d.assignLast = -1;
    d.dirty = true;
    d.building = false;
}

void Axis::appendPlace(Place p)
{
    Data &d = *m_d;
    if (p.index <= d.assignLast || p.index >= m_count) return;
    d.assignLast = p.index;
    p.size = boundedSize(p.size);
    if (p.size != m_default) d.places.push_back(p);
    d.dirty = true;
    d.building = false;
}

void Axis::ensureBuilt() const
{
    if (m_d->dirty || m_d->before.size() != m_d->places.size() + 1) rebuild();
}

void Axis::rebuild() const
{
    Work all;
    m_d->building = false;
    buildSome(all);
}

// Works out, from the places, the sums and the runs of hidden places: in one pass forward (the sums, the start of each run, and an end for each place of its own), and one back from the
// last (the end of each run of hidden places). Stops when the work is spent and goes on from there at the next call; false until it is built.
bool Axis::buildSome(Work &work) const
{
    Data &d = *m_d;
    if (!d.dirty && d.before.size() == d.places.size() + 1) return true;
    const size_t n = d.places.size();
    if (!d.building) {
        d.before.clear();
        d.hiddenEnd.clear();
        d.hiddenStart.clear();
        d.before.reserve(n + 1);
        d.hiddenEnd.reserve(n);
        d.hiddenStart.reserve(n);
        d.forward = 0;
        d.backward = n;
        d.sum = 0;
        d.building = true;
    }
    while (d.forward < n) {
        if (work.spent()) return false;
        const size_t k = d.forward++;
        const Place &p = d.places[k];
        d.before.push_back(d.sum);
        d.sum += p.size - m_default;
        d.hiddenEnd.push_back(p.index + 1);
        // the first row of the run each hidden place is in
        d.hiddenStart.push_back(p.size == 0 && k > 0 && d.places[k - 1].size == 0 && d.places[k - 1].index == p.index - 1 ? d.hiddenStart[k - 1] : p.index);
        work.add();
    }
    while (d.backward > 0) {
        if (work.spent()) return false;
        const size_t k = --d.backward;
        const Place &p = d.places[k];
        // the end of the run of hidden rows each hidden place begins
        if (p.size == 0 && k + 1 < n && d.places[k + 1].size == 0 && d.places[k + 1].index == p.index + 1) d.hiddenEnd[k] = d.hiddenEnd[k + 1];
        work.add();
    }
    d.before.push_back(d.sum);
    d.dirty = false;
    d.building = false;
    return true;
}

void Axis::setSkipped(qint64 count) { m_skip = std::max<qint64>(0, std::min(count, m_count)); }

int Axis::size(qint64 index) const
{
    if (index < 0 || index >= m_count || index < m_skip) return 0;
    const int k = find(index);
    return k >= 0 ? m_d->places[size_t(k)].size : m_default;
}

qint64 Axis::position(qint64 index) const
{
    if (m_skip <= 0) return rawPosition(index);
    return index <= m_skip ? 0 : rawPosition(index) - rawPosition(m_skip);
}

qint64 Axis::rawPosition(qint64 index) const
{
    if (index <= 0) return 0;
    if (index > m_count) index = m_count;
    ensureBuilt();
    // the places before this row
    qint64 lo = 0, hi = qint64(m_d->places.size());
    while (lo < hi) {
        ++m_probes;
        const qint64 mid = (lo + hi) / 2;
        if (m_d->places[size_t(mid)].index < index) lo = mid + 1;
        else hi = mid;
    }
    return index * m_default + m_d->before[size_t(lo)];
}

qint64 Axis::indexAt(qint64 given) const
{
    if (given < 0 || given >= total()) return -1;
    const qint64 pos = given + (m_skip > 0 ? rawPosition(m_skip) : 0);
    // the places that begin at or before pos: a place's own start is its index times the default, and what the places before it differ by
    auto startOf = [&](qint64 k) { return m_d->places[size_t(k)].index * m_default + m_d->before[size_t(k)]; };
    qint64 lo = 0, hi = qint64(m_d->places.size());
    while (lo < hi) {
        ++m_probes;
        const qint64 mid = (lo + hi) / 2;
        if (startOf(mid) <= pos) lo = mid + 1;
        else hi = mid;
    }
    if (lo == 0) return pos / m_default;
    const qint64 k = lo - 1, start = startOf(k);
    const Place &p = m_d->places[size_t(k)];
    if (pos < start + p.size) return p.index;
    // the rows of the default size after that place, up to the next
    return p.index + 1 + (pos - start - p.size) / m_default;
}

qint64 Axis::nextVisible(qint64 index) const
{
    qint64 next = std::max(index + 1, m_skip);
    if (next >= m_count) return m_count;
    if (next < 0) next = 0;
    ensureBuilt();
    // a hidden row is stepped over with the run it is in
    while (next < m_count) {
        ++m_probes;
        const int k = find(next);
        if (k < 0 || m_d->places[size_t(k)].size != 0) return next;
        next = m_d->hiddenEnd[size_t(k)];
    }
    return m_count;
}

qint64 Axis::previousVisible(qint64 index) const
{
    qint64 prev = std::min(index, m_count) - 1;
    if (prev < 0) return -1;
    ensureBuilt();
    // a hidden row is stepped over with the run it is in, back to the run's first (and the rows that are skipped are hidden, all of them)
    while (prev >= m_skip) {
        ++m_probes;
        const int k = find(prev);
        if (k < 0 || m_d->places[size_t(k)].size != 0) return prev;
        prev = m_d->hiddenStart[size_t(k)] - 1;
    }
    return -1;
}

} // namespace jp::grid
