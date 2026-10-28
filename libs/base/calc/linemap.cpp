#include "calc/linemap.h"

#include <QCoreApplication>

#include <algorithm>
#include <mutex>
#include <unordered_map>

namespace jp::calc {

namespace {
constexpr size_t kNoIndex = ~size_t(0);
// What the cache of runs for ranges may hold: past either, it is emptied (readers keep what they were given).
constexpr size_t kCacheMaxEntries = 8192;
constexpr size_t kCacheMaxRuns = 2000000;
} // namespace

QString lineStatusText(LineStatus s)
{
    switch (s) {
    case LineStatus::Ok: return QString();
    case LineStatus::TooManyRuns:
    case LineStatus::IdsExhausted:
    case LineStatus::StillOverCap: return QCoreApplication::translate("LineMap", "This sheet has been edited too many times. Save it and open it again.");
    case LineStatus::OutOfRange: return QCoreApplication::translate("LineMap", "Those rows or columns are not on the sheet.");
    case LineStatus::StaleStep: return QCoreApplication::translate("LineMap", "This step can no longer be undone or redone.");
    }
    return QString();
}

struct LineMapCore::Cache {
    std::mutex lock;
    uint64_t version = ~uint64_t(0);
    std::unordered_map<uint64_t, Runs> map;
    size_t runs = 0;
    uint64_t hits = 0, misses = 0;
};

LineMapCore::LineMapCore(const LineLimits &limits, uint32_t live) : m_limits(limits), m_cache(new Cache)
{
    m_live = std::min({live, limits.capacity, limits.firstNew});
    m_next = limits.firstNew;
}

LineMapCore::LineMapCore(const LineMapCore &o)
    : m_limits(o.m_limits), m_live(o.m_live), m_identity(o.m_identity), m_byPos(o.m_byPos), m_byId(o.m_byId), m_dead(o.m_dead), m_next(o.m_next), m_version(o.m_version), m_state(o.m_state),
      m_stateSeq(o.m_stateSeq), m_work(o.m_work), m_cache(new Cache)
{
}

LineMapCore &LineMapCore::operator=(const LineMapCore &o)
{
    if (this == &o) return *this;
    m_limits = o.m_limits;
    m_live = o.m_live;
    m_identity = o.m_identity;
    m_byPos = o.m_byPos;
    m_byId = o.m_byId;
    m_dead = o.m_dead;
    m_next = o.m_next;
    m_version = o.m_version;
    m_state = o.m_state;
    m_stateSeq = o.m_stateSeq;
    m_work = o.m_work;
    m_cache.reset(new Cache);
    return *this;
}

LineMapCore::LineMapCore(LineMapCore &&) noexcept = default;
LineMapCore &LineMapCore::operator=(LineMapCore &&) noexcept = default;
LineMapCore::~LineMapCore() = default;

bool LineMapCore::overSoftCap(size_t runs, uint64_t idsUsed) const
{
    const uint64_t space = m_limits.idLimit > m_limits.firstNew ? m_limits.idLimit - m_limits.firstNew : 0;
    return runs >= m_limits.rebaseRuns || idsUsed * 5 >= space * 4;
}

bool LineMapCore::needsRebase() const { return overSoftCap(m_byPos.size(), m_next - m_limits.firstNew); }

// ------------------------------------------------------------------------------------------------ reading

size_t LineMapCore::runAtPos(uint32_t pos, LineWork *w) const
{
    size_t lo = 0, hi = m_byPos.size();   // the last run whose pre is at most pos
    while (hi - lo > 1) {
        const size_t mid = (lo + hi) / 2;
        if (w) ++w->comparisons;
        if (m_byPos[mid].pre <= pos) lo = mid;
        else hi = mid;
    }
    return lo;
}

size_t LineMapCore::entryAtId(uint32_t id, LineWork *w) const
{
    size_t lo = 0, hi = m_byId.size();   // the last entry whose first is at most id
    while (lo < hi) {
        const size_t mid = (lo + hi) / 2;
        if (w) ++w->comparisons;
        if (m_byId[mid].first <= id) lo = mid + 1;
        else hi = mid;
    }
    return lo == 0 ? kNoIndex : lo - 1;
}

uint32_t LineMapCore::idAt(uint32_t pos, LineHint *hint, LineWork *w) const
{
    if (pos >= m_live) return kNone;
    if (m_identity) return pos;
    auto holds = [&](size_t r) { return r < m_byPos.size() && m_byPos[r].pre <= pos && pos - m_byPos[r].pre < m_byPos[r].count; };
    size_t r = kNoIndex;
    if (hint && hint->version == m_version) {
        const size_t h = hint->pos;
        if (w) ++w->comparisons;
        if (holds(h)) r = h;
        else {
            if (w) ++w->comparisons;
            if (holds(h + 1)) r = h + 1;
        }
    }
    if (r == kNoIndex) r = runAtPos(pos, w);
    if (hint) {
        hint->version = m_version;
        hint->pos = uint32_t(r);
    }
    return m_byPos[r].first + (pos - m_byPos[r].pre);
}

uint32_t LineMapCore::posOf(uint32_t id, LineHint *hint, LineWork *w) const
{
    if (m_identity) return id < m_live ? id : kNone;
    auto holds = [&](size_t e) { return e < m_byId.size() && m_byId[e].first <= id && id - m_byId[e].first < m_byId[e].count; };
    size_t e = kNoIndex;
    if (hint && hint->version == m_version) {
        const size_t h = hint->id;
        if (w) ++w->comparisons;
        if (holds(h)) e = h;
        else {
            if (w) ++w->comparisons;
            if (holds(h + 1)) e = h + 1;
        }
    }
    if (e == kNoIndex) {
        e = entryAtId(id, w);
        if (e == kNoIndex || id - m_byId[e].first >= m_byId[e].count) return kNone;
    }
    if (hint) {
        hint->version = m_version;
        hint->id = uint32_t(e);
    }
    return m_byId[e].pre + (id - m_byId[e].first);
}

const LineDead *LineMapCore::deadFor(uint32_t id, LineWork *w) const
{
    size_t lo = 0, hi = m_dead.size();
    while (lo < hi) {
        const size_t mid = (lo + hi) / 2;
        if (w) ++w->comparisons;
        if (m_dead[mid].first <= id) lo = mid + 1;
        else hi = mid;
    }
    if (lo == 0) return nullptr;
    const LineDead &d = m_dead[lo - 1];
    return id - d.first < d.count ? &d : nullptr;
}

uint32_t LineMapCore::resolve(uint32_t id, bool wantAfter, LineWork *w) const
{
    // (Every hop goes to an id that died later than the one it left, so the chain ends; the count is a guard that the records cannot reach.)
    for (size_t hops = 0; id != kNone; ++hops) {
        if (posOf(id) != kNone) return id;
        const LineDead *d = deadFor(id, w);
        if (!d || hops > m_dead.size()) return kNone;
        if (w) ++w->hops;
        id = wantAfter ? d->after : d->before;
    }
    return kNone;
}

uint32_t LineMapCore::resolveStart(uint32_t id, LineWork *w) const { return resolve(id, true, w); }
uint32_t LineMapCore::resolveEnd(uint32_t id, LineWork *w) const { return resolve(id, false, w); }

bool LineMapCore::resolveRange(uint32_t startId, uint32_t endId, uint32_t *first, uint32_t *last, LineWork *w) const
{
    const uint32_t s = resolveStart(startId, w), e = resolveEnd(endId, w);
    if (s == kNone || e == kNone) return false;
    const uint32_t ps = posOf(s), pe = posOf(e);
    if (ps == kNone || pe == kNone || ps > pe) return false;
    if (first) *first = ps;
    if (last) *last = pe;
    return true;
}

bool LineMapCore::wholeRange(uint32_t *first, uint32_t *last) const
{
    if (m_live == 0) return false;
    if (first) *first = 0;
    if (last) *last = m_live - 1;
    return true;
}

uint32_t LineMapCore::origBefore(uint32_t pos) const
{
    if (m_live == 0) return kNone;
    if (pos >= m_live) pos = m_live - 1;
    if (m_identity) return pos;
    const size_t r = runAtPos(pos, nullptr);
    const PosRun &run = m_byPos[r];
    const uint32_t id = run.first + (pos - run.pre);
    if (id < m_limits.firstNew) return id;
    // (ids in a run go up by one: a run that began among the originals and reaches this one holds the last original, firstNew - 1, before it)
    if (run.first < m_limits.firstNew) return m_limits.firstNew - 1;
    for (size_t k = r; k-- > 0;)
        if (m_byPos[k].first < m_limits.firstNew) return std::min(m_byPos[k].first + m_byPos[k].count - 1, m_limits.firstNew - 1);
    return kNone;
}

uint32_t LineMapCore::origAfter(uint32_t pos) const
{
    if (pos >= m_live) return kNone;
    if (m_identity) return pos;
    const size_t r = runAtPos(pos, nullptr);
    const PosRun &run = m_byPos[r];
    const uint32_t id = run.first + (pos - run.pre);
    if (id < m_limits.firstNew) return id;
    for (size_t k = r + 1; k < m_byPos.size(); ++k)
        if (m_byPos[k].first < m_limits.firstNew) return m_byPos[k].first;
    return kNone;
}

void LineMapCore::walk(uint32_t first, uint32_t last, std::vector<IdRun> *out, LineWork *w) const
{
    out->clear();
    if (first > last || last >= m_live) return;
    if (m_identity) {
        out->push_back({first, last - first + 1});
        if (w) ++w->runsVisited;
        return;
    }
    for (size_t r = runAtPos(first, w); r < m_byPos.size() && m_byPos[r].pre <= last; ++r) {
        const PosRun &run = m_byPos[r];
        const uint32_t a = std::max(first, run.pre), b = std::min(last, run.pre + run.count - 1);
        out->push_back({run.first + (a - run.pre), b - a + 1});
        if (w) ++w->runsVisited;
    }
}

LineMapCore::Runs LineMapCore::runsFor(uint32_t first, uint32_t last) const
{
    Cache &c = *m_cache;
    std::lock_guard<std::mutex> g(c.lock);
    if (c.version != m_version) {
        c.map.clear();
        c.runs = 0;
        c.version = m_version;
    }
    const uint64_t key = (uint64_t(first) << 32) | last;
    auto it = c.map.find(key);
    if (it != c.map.end()) {
        ++c.hits;
        return it->second;
    }
    ++c.misses;
    auto runs = std::make_shared<std::vector<IdRun>>();
    walk(first, last, runs.get());
    if (c.map.size() >= kCacheMaxEntries || c.runs + runs->size() > kCacheMaxRuns) {
        c.map.clear();
        c.runs = 0;
    }
    c.runs += runs->size();
    Runs result = runs;
    c.map.emplace(key, result);
    return result;
}

uint64_t LineMapCore::cacheHits() const
{
    std::lock_guard<std::mutex> g(m_cache->lock);
    return m_cache->hits;
}
uint64_t LineMapCore::cacheMisses() const
{
    std::lock_guard<std::mutex> g(m_cache->lock);
    return m_cache->misses;
}
size_t LineMapCore::cacheEntries() const
{
    std::lock_guard<std::mutex> g(m_cache->lock);
    return m_cache->map.size();
}

std::vector<IdRun> LineMapCore::liveRuns() const
{
    std::vector<IdRun> out;
    if (m_identity) {
        if (m_live) out.push_back({0, m_live});
        return out;
    }
    for (const PosRun &r : m_byPos) out.push_back({r.first, r.count});
    return out;
}

// ------------------------------------------------------------------------------------------------ changing

void LineMapCore::materialize()
{
    if (!m_identity) return;
    m_identity = false;
    if (m_live) {
        m_byPos.push_back({0, m_live, 0});
        m_byId.push_back({0, m_live, 0});
    }
}

// Makes a run start at `pos` (0 < pos < size) and returns its index.
size_t LineMapCore::splitAt(uint32_t pos)
{
    const size_t r = runAtPos(pos, nullptr);
    if (m_byPos[r].pre == pos) return r;
    const uint32_t keep = pos - m_byPos[r].pre;
    const PosRun second{m_byPos[r].first + keep, m_byPos[r].count - keep, pos};
    const size_t e = entryAtId(m_byPos[r].first, nullptr);
    m_byPos[r].count = keep;
    m_work.entriesMoved += (m_byPos.size() - r) + (m_byId.size() - e);
    m_byPos.insert(m_byPos.begin() + long(r) + 1, second);
    m_byId[e].count = keep;
    m_byId.insert(m_byId.begin() + long(e) + 1, {second.first, second.count, pos});
    return r + 1;
}

// Takes the runs r0 .. r1 - 1 out of the order of position and of the order of id.
void LineMapCore::eraseRuns(size_t r0, size_t r1)
{
    if (r0 >= r1) return;
    const uint32_t lo = m_byPos[r0].pre, hi = m_byPos[r1 - 1].pre + m_byPos[r1 - 1].count, total = hi - lo;
    m_byPos.erase(m_byPos.begin() + long(r0), m_byPos.begin() + long(r1));
    for (size_t i = r0; i < m_byPos.size(); ++i) m_byPos[i].pre -= total;
    // (the runs taken are the entries whose pre lies in lo .. hi - 1; those after them move up)
    size_t out = 0;
    for (size_t i = 0; i < m_byId.size(); ++i) {
        IdEntry e = m_byId[i];
        if (e.pre >= lo && e.pre < hi) continue;
        if (e.pre >= hi) e.pre -= total;
        m_byId[out++] = e;
    }
    m_byId.resize(out);
    m_work.entriesMoved += (m_byPos.size() - r0) + m_byId.size();
}

// Puts runs in at the boundary before run r (r == size() appends); the ids need not be in order of id.
void LineMapCore::insertRuns(size_t r, const std::vector<IdRun> &runs)
{
    if (runs.empty()) return;
    uint32_t total = 0;
    for (const IdRun &x : runs) total += x.count;
    const uint32_t base = r < m_byPos.size() ? m_byPos[r].pre : m_live;
    std::vector<PosRun> add;
    add.reserve(runs.size());
    uint32_t pre = base;
    for (const IdRun &x : runs) {
        add.push_back({x.first, x.count, pre});
        pre += x.count;
    }
    for (size_t i = r; i < m_byPos.size(); ++i) m_byPos[i].pre += total;
    m_work.entriesMoved += m_byPos.size() - r;
    if (r < m_byPos.size()) {
        for (IdEntry &e : m_byId) e.pre += e.pre >= base ? total : 0;
        m_work.entriesMoved += m_byId.size();
    }
    m_byPos.insert(m_byPos.begin() + long(r), add.begin(), add.end());
    std::vector<IdEntry> entries;
    entries.reserve(add.size());
    for (const PosRun &x : add) entries.push_back({x.first, x.count, x.pre});
    std::sort(entries.begin(), entries.end(), [](const IdEntry &a, const IdEntry &b) { return a.first < b.first; });
    if (entries.size() == 1) {
        const auto at = std::lower_bound(m_byId.begin(), m_byId.end(), entries[0].first, [](const IdEntry &a, uint32_t f) { return a.first < f; });
        m_work.entriesMoved += size_t(m_byId.end() - at);
        m_byId.insert(at, entries[0]);
    } else {
        const size_t mid = m_byId.size();
        m_byId.insert(m_byId.end(), entries.begin(), entries.end());
        std::inplace_merge(m_byId.begin(), m_byId.begin() + long(mid), m_byId.end(), [](const IdEntry &a, const IdEntry &b) { return a.first < b.first; });
        m_work.entriesMoved += m_byId.size();
    }
}

// Joins run r and run r + 1 if they are one run of consecutive ids.
void LineMapCore::tryMerge(size_t r)
{
    if (r + 1 >= m_byPos.size()) return;
    const PosRun a = m_byPos[r], b = m_byPos[r + 1];
    if (a.first + a.count != b.first) return;
    const size_t ea = entryAtId(a.first, nullptr);
    m_byPos[r].count += b.count;
    m_byPos.erase(m_byPos.begin() + long(r) + 1);
    m_byId[ea].count += b.count;
    m_byId.erase(m_byId.begin() + long(ea) + 1);   // (its ids follow a's, so its entry follows a's)
    m_work.entriesMoved += (m_byPos.size() - r) + (m_byId.size() - ea);
}

// The pieces (one for each run they lie in) that the positions at .. at + count - 1 hold.
void LineMapCore::pieces(uint32_t at, uint32_t count, std::vector<IdRun> *out) const
{
    out->clear();
    if (m_identity) {
        out->push_back({at, count});
        return;
    }
    for (size_t r = runAtPos(at, nullptr); r < m_byPos.size() && m_byPos[r].pre < at + uint64_t(count); ++r) {
        const PosRun &run = m_byPos[r];
        const uint32_t a = std::max(at, run.pre), b = std::min(at + count - 1, run.pre + run.count - 1);
        out->push_back({run.first + (a - run.pre), b - a + 1});
    }
}

// Takes the lines at at .. at + total - 1 (which hold exactly these pieces) out of the order and writes a record for each piece.
void LineMapCore::kill(uint32_t at, const std::vector<IdRun> &pcs)
{
    materialize();
    uint32_t total = 0;
    for (const IdRun &p : pcs) total += p.count;
    if (total == 0) return;
    const uint32_t before = at > 0 ? idAt(at - 1) : kNone;
    const uint32_t after = at + uint64_t(total) < m_live ? idAt(at + total) : kNone;
    const size_t r0 = at == 0 ? 0 : splitAt(at);
    const size_t r1 = at + uint64_t(total) >= m_live ? m_byPos.size() : splitAt(at + total);
    eraseRuns(r0, r1);
    m_live -= total;
    if (r0 > 0) tryMerge(r0 - 1);   // (what was split to take them out may be one run again)
    std::vector<LineDead> add;
    add.reserve(pcs.size());
    for (const IdRun &p : pcs) add.push_back({p.first, p.count, before, after});
    std::sort(add.begin(), add.end(), [](const LineDead &x, const LineDead &y) { return x.first < y.first; });
    if (add.size() == 1) {
        const auto it = std::lower_bound(m_dead.begin(), m_dead.end(), add[0].first, [](const LineDead &x, uint32_t f) { return x.first < f; });
        m_work.entriesMoved += size_t(m_dead.end() - it);
        m_dead.insert(it, add[0]);
    } else {
        const size_t mid = m_dead.size();
        m_dead.insert(m_dead.end(), add.begin(), add.end());
        std::inplace_merge(m_dead.begin(), m_dead.begin() + long(mid), m_dead.end(), [](const LineDead &x, const LineDead &y) { return x.first < y.first; });
        m_work.entriesMoved += m_dead.size();
    }
    ++m_version;
}

// Puts the pieces back (or in, for new ids) at the position `at`; their records, if any, are removed.
void LineMapCore::revive(uint32_t at, const std::vector<IdRun> &pcs)
{
    materialize();
    std::vector<IdRun> joined;   // (pieces whose ids follow one another are one run)
    std::vector<uint32_t> gone;  // (the records of the pieces)
    uint32_t total = 0;
    for (const IdRun &p : pcs) {
        total += p.count;
        gone.push_back(p.first);
        if (!joined.empty() && joined.back().first + joined.back().count == p.first) joined.back().count += p.count;
        else joined.push_back(p);
    }
    std::sort(gone.begin(), gone.end());
    if (gone.size() == 1) {
        const auto it = std::lower_bound(m_dead.begin(), m_dead.end(), gone[0], [](const LineDead &x, uint32_t f) { return x.first < f; });
        if (it != m_dead.end() && it->first == gone[0]) {
            m_work.entriesMoved += size_t(m_dead.end() - it);
            m_dead.erase(it);
        }
    } else if (!m_dead.empty()) {
        size_t out = 0, g = 0;
        for (size_t i = 0; i < m_dead.size(); ++i) {
            while (g < gone.size() && gone[g] < m_dead[i].first) ++g;
            if (g < gone.size() && gone[g] == m_dead[i].first) continue;
            m_dead[out++] = m_dead[i];
        }
        m_dead.resize(out);
        m_work.entriesMoved += m_dead.size();
    }
    if (total == 0) return;
    const size_t r = at == 0 || m_byPos.empty() ? 0 : (at >= m_live ? m_byPos.size() : splitAt(at));
    insertRuns(r, joined);
    m_live += total;
    // (the new runs may join the run before them, the run after them, or one another)
    for (long k = long(r + joined.size()) - 1; k >= (r > 0 ? long(r) - 1 : 0); --k) tryMerge(size_t(k));
    ++m_version;
}

LineStatus LineMapCore::insert(uint32_t at, uint32_t count, LineEdit *edit)
{
    m_work = LineWork();
    if (count == 0 || at > m_live || at + uint64_t(count) > m_limits.capacity) return LineStatus::OutOfRange;
    if (uint64_t(m_next) + count > m_limits.idLimit) return LineStatus::IdsExhausted;
    const uint32_t over = pushOff(count);
    std::vector<IdRun> tail;
    if (over) pieces(m_live - over, over, &tail);
    if (m_byPos.size() + 3 > m_limits.maxRuns || m_dead.size() + tail.size() > m_limits.maxRuns) return LineStatus::TooManyRuns;
    LineEdit e;
    e.kind = LineEdit::Kind::Insert;
    e.at = at;
    e.runs = {{m_next, count}};
    e.pushedAt = m_live - over;
    e.pushed = tail;
    if (over) kill(e.pushedAt, tail);
    revive(at, e.runs);
    m_next += count;
    e.before = m_state;
    e.after = m_state = ++m_stateSeq;
    if (edit) *edit = std::move(e);
    return LineStatus::Ok;
}

LineStatus LineMapCore::erase(uint32_t at, uint32_t count, LineEdit *edit)
{
    m_work = LineWork();
    if (count == 0 || at >= m_live || count > m_live - at) return LineStatus::OutOfRange;
    LineEdit e;
    e.kind = LineEdit::Kind::Erase;
    e.at = at;
    pieces(at, count, &e.runs);
    if (m_byPos.size() + 1 > m_limits.maxRuns || m_dead.size() + e.runs.size() > m_limits.maxRuns) return LineStatus::TooManyRuns;
    kill(at, e.runs);
    e.before = m_state;
    e.after = m_state = ++m_stateSeq;
    if (edit) *edit = std::move(e);
    return LineStatus::Ok;
}

LineStatus LineMapCore::undo(LineEdit &e)
{
    m_work = LineWork();
    if (e.forgotten || m_state != e.after) return LineStatus::StaleStep;
    if (e.kind == LineEdit::Kind::Insert) {
        kill(e.at, e.runs);
        revive(e.pushedAt, e.pushed);
    } else {
        revive(e.at, e.runs);
    }
    m_state = e.before;
    return LineStatus::Ok;
}

LineStatus LineMapCore::redo(LineEdit &e)
{
    m_work = LineWork();
    if (e.forgotten || m_state != e.before) return LineStatus::StaleStep;
    if (e.kind == LineEdit::Kind::Insert) {
        kill(e.pushedAt, e.pushed);
        revive(e.at, e.runs);
    } else {
        kill(e.at, e.runs);
    }
    m_state = e.after;
    return LineStatus::Ok;
}

bool LineMapCore::forget(LineEdit &e)
{
    m_work = LineWork();
    if (e.forgotten) return true;
    if (e.runs.empty()) return false;
    // (An insert that is still applied has its ids live and is not dropped. An erase leaves nothing to let go of; the caller drops only steps it will never undo or redo.)
    if (e.kind == LineEdit::Kind::Insert) {
        if (posOf(e.runs[0].first) != kNone) return false;
        for (const IdRun &r : e.runs) {
            const auto it = std::lower_bound(m_dead.begin(), m_dead.end(), r.first, [](const LineDead &x, uint32_t f) { return x.first < f; });
            if (it != m_dead.end() && it->first == r.first) m_dead.erase(it);
            if (r.first + uint64_t(r.count) == m_next) m_next = r.first;
        }
        ++m_version;
    }
    e.forgotten = true;
    return true;
}

void LineMapCore::dropDead(const std::vector<IdRun> &runs)
{
    if (m_dead.empty()) return;
    size_t out = 0;
    for (size_t i = 0; i < m_dead.size(); ++i) {
        bool drop = false;
        for (const IdRun &r : runs)
            if (m_dead[i].first >= r.first && m_dead[i].first - r.first < r.count) {
                drop = true;
                break;
            }
        if (!drop) m_dead[out++] = m_dead[i];
    }
    if (out != m_dead.size()) {
        m_work.entriesMoved += m_dead.size();
        m_dead.resize(out);
        ++m_version;
    }
}

bool LineMapCore::tryIdentity()
{
    if (m_identity) return true;
    if (!m_dead.empty() || m_byPos.size() > 1) return false;
    if (m_live && (m_byPos.empty() || m_byPos[0].first != 0 || m_byPos[0].count != m_live)) return false;
    m_identity = true;
    std::vector<PosRun>().swap(m_byPos);
    std::vector<IdEntry>().swap(m_byId);
    ++m_version;
    return true;
}

LineStatus LineMapCore::rebase(std::vector<Renumber> *table)
{
    m_work = LineWork();
    if (table) table->clear();
    if (m_identity) {
        // (no order to renumber; the ids taken since, if the order came back to identity, are given back, and every step made before is stale)
        m_next = m_limits.firstNew;
        ++m_version;
        m_state = ++m_stateSeq;
        return LineStatus::Ok;
    }
    if (overSoftCap(m_live ? 1 : 0, 0)) return LineStatus::StillOverCap;
    if (table)
        for (const PosRun &r : m_byPos) table->push_back({r.first, r.count, r.pre});
    m_identity = true;
    std::vector<PosRun>().swap(m_byPos);
    std::vector<IdEntry>().swap(m_byId);
    std::vector<LineDead>().swap(m_dead);
    m_next = m_limits.firstNew;
    ++m_version;
    m_state = ++m_stateSeq;
    return LineStatus::Ok;
}

bool LineMapCore::selfCheck() const
{
    if (m_identity) return m_byPos.empty() && m_byId.empty() && m_dead.empty() && m_live <= m_limits.capacity;
    if (m_byPos.size() != m_byId.size()) return false;
    uint64_t pre = 0;
    for (size_t i = 0; i < m_byPos.size(); ++i) {
        const PosRun &r = m_byPos[i];
        if (r.count == 0 || r.pre != pre) return false;
        pre += r.count;
        if (uint64_t(r.first) + r.count > m_next) return false;
        const size_t e = entryAtId(r.first, nullptr);
        if (e == kNoIndex || m_byId[e].first != r.first || m_byId[e].count != r.count || m_byId[e].pre != r.pre) return false;
    }
    if (pre != m_live) return false;
    for (size_t i = 1; i < m_byId.size(); ++i)
        if (uint64_t(m_byId[i - 1].first) + m_byId[i - 1].count > m_byId[i].first) return false;
    for (size_t i = 0; i < m_dead.size(); ++i) {
        const LineDead &d = m_dead[i];
        if (d.count == 0) return false;
        if (i && uint64_t(m_dead[i - 1].first) + m_dead[i - 1].count > d.first) return false;
        if (posOf(d.first) != kNone || posOf(d.first + d.count - 1) != kNone) return false;
    }
    return true;
}

} // namespace jp::calc
