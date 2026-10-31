#include "grid/axis.h"

#include <algorithm>
#include <atomic>

namespace jp::grid {

namespace {
std::atomic<qint64> g_axisBuilds{0};   // full builds of the sums (buildSome from the first place)
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
        g_axisBuilds.fetch_add(1, std::memory_order_relaxed);
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

qint64 Axis::buildsForTest() { return g_axisBuilds.load(std::memory_order_relaxed); }

std::vector<Axis::Place> Axis::places() const { return m_d->places; }

namespace {
// Whether the place and the next in the list are both hidden and in one run of hidden rows.
bool chained(const std::vector<Axis::Place> &places, size_t a) { return places[a].size == 0 && places[a + 1].size == 0 && places[a].index + 1 == places[a + 1].index; }

// Works the runs of hidden places out again for the places from `lo` to `hi` and the whole runs they are parts of (the places outside have the right ones already).
void fixRuns(const std::vector<Axis::Place> &places, std::vector<qint64> &hiddenEnd, std::vector<qint64> &hiddenStart, size_t lo, size_t hi)
{
    if (places.empty()) return;
    hi = std::min(hi, places.size() - 1);
    lo = std::min(lo, hi);
    while (lo > 0 && chained(places, lo - 1)) --lo;
    while (hi + 1 < places.size() && chained(places, hi)) ++hi;
    for (size_t k = lo; k <= hi; ++k) {
        hiddenEnd[k] = places[k].index + 1;
        hiddenStart[k] = places[k].size == 0 && k > lo && chained(places, k - 1) ? hiddenStart[k - 1] : places[k].index;
    }
    for (size_t k = hi + 1; k-- > lo;)
        if (places[k].size == 0 && k < hi && chained(places, k)) hiddenEnd[k] = hiddenEnd[k + 1];
}
} // namespace

void Axis::insertPlaces(qint64 at, qint64 count, const std::vector<Place> &own)
{
    if (count <= 0 || at < 0 || at >= m_count) return;
    Data &d = writable();
    const bool built = !d.dirty && !d.building && d.before.size() == d.places.size() + 1;
    auto lower = [&](qint64 index) { return size_t(std::lower_bound(d.places.begin(), d.places.end(), index, [](const Place &p, qint64 i) { return p.index < i; }) - d.places.begin()); };
    const size_t p = lower(at);
    const size_t q = std::max(p, lower(m_count - count));   // the places that fall off the end
    std::vector<Place> mid;
    for (Place o : own) {
        o.size = boundedSize(o.size);
        if (o.index < at || o.index >= at + count || o.index >= m_count || o.size == m_default || (!mid.empty() && o.index <= mid.back().index)) continue;
        mid.push_back(o);
    }
    const size_t m = mid.size();
    d.places.resize(q);
    d.places.insert(d.places.begin() + std::ptrdiff_t(p), mid.begin(), mid.end());
    for (size_t k = p + m; k < d.places.size(); ++k) d.places[k].index += count;
    d.building = false;
    if (!built) {
        d.dirty = true;
        return;
    }
    qint64 added = 0;
    std::vector<qint64> sums(m);
    for (size_t i = 0; i < m; ++i) {
        sums[i] = d.before[p] + added;
        added += mid[i].size - m_default;
    }
    d.before.resize(q + 1);
    d.before.insert(d.before.begin() + std::ptrdiff_t(p), sums.begin(), sums.end());
    for (size_t k = p + m; k < d.before.size(); ++k) d.before[k] += added;
    d.hiddenEnd.resize(q);
    d.hiddenStart.resize(q);
    d.hiddenEnd.insert(d.hiddenEnd.begin() + std::ptrdiff_t(p), m, 0);
    d.hiddenStart.insert(d.hiddenStart.begin() + std::ptrdiff_t(p), m, 0);
    for (size_t k = p + m; k < d.places.size(); ++k) {
        d.hiddenEnd[k] += count;
        d.hiddenStart[k] += count;
    }
    // (a run of hidden rows that went past the last row ends at it)
    for (size_t k = d.places.size(); k-- > p + m && d.hiddenEnd[k] > m_count;) d.hiddenEnd[k] = m_count;
    // (a run of hidden rows the new rows were put in the middle of is two runs now, and new hidden rows may join what is on either side)
    fixRuns(d.places, d.hiddenEnd, d.hiddenStart, p > 0 ? p - 1 : 0, p + m);
    d.sum = d.before.back();
}

void Axis::erasePlaces(qint64 at, qint64 count)
{
    if (count <= 0 || at < 0 || at >= m_count) return;
    count = std::min(count, m_count - at);
    Data &d = writable();
    const bool built = !d.dirty && !d.building && d.before.size() == d.places.size() + 1;
    auto lower = [&](qint64 index) { return size_t(std::lower_bound(d.places.begin(), d.places.end(), index, [](const Place &p, qint64 i) { return p.index < i; }) - d.places.begin()); };
    const size_t p = lower(at), q = lower(at + count);
    const qint64 taken = built ? d.before[q] - d.before[p] : 0;
    d.places.erase(d.places.begin() + std::ptrdiff_t(p), d.places.begin() + std::ptrdiff_t(q));
    for (size_t k = p; k < d.places.size(); ++k) d.places[k].index -= count;
    d.building = false;
    if (!built) {
        d.dirty = true;
        return;
    }
    d.before.erase(d.before.begin() + std::ptrdiff_t(p), d.before.begin() + std::ptrdiff_t(q));
    for (size_t k = p; k < d.before.size(); ++k) d.before[k] -= taken;
    d.hiddenEnd.erase(d.hiddenEnd.begin() + std::ptrdiff_t(p), d.hiddenEnd.begin() + std::ptrdiff_t(q));
    d.hiddenStart.erase(d.hiddenStart.begin() + std::ptrdiff_t(p), d.hiddenStart.begin() + std::ptrdiff_t(q));
    for (size_t k = p; k < d.places.size(); ++k) {
        d.hiddenEnd[k] -= count;
        d.hiddenStart[k] -= count;
    }
    // (the two ends of a run of hidden rows that rows between were taken out of are one run now)
    fixRuns(d.places, d.hiddenEnd, d.hiddenStart, p > 0 ? p - 1 : 0, p);
    d.sum = d.before.back();
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
