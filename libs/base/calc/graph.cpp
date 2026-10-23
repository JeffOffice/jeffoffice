#include "calc/graph.h"

#include <algorithm>
#include <limits>
#include <unordered_map>

namespace jp::calc {

namespace {

constexpr quint32 kColumns = 16384;                    // the columns of a sheet: the leaves of the tree over them
constexpr int kLevels = 15;                            // the levels of the tree: a node of level L holds 2^L columns (the one of level 14 holds all of them)
constexpr int kMaxSheetsOfABlock = 1024;
constexpr size_t kBufferMax = 64;                      // the intervals a node holds that are not yet in a run, which a query looks at one by one
constexpr quint32 kMaxNode = quint32(1) << 28;         // (the caller numbers its formulas from 0; a number past this is refused rather than made room for)
constexpr qint64 kPlanItemBytes = 24;                  // what an interval is counted to take before it is in a run (20 there: 16 and 4 for the largest end below it)
constexpr qint64 kStructureBytes = 160;                // what a node of the tree takes, besides its intervals
constexpr qint64 kStaleFloor = 1 << 14;                // the dropped intervals that are let go of only past this many (or as many as are held)

struct Item {
    quint32 lo, hi, node, gen;
};

// What the graph knows of each formula: the generation of what it told last (what it told before has another), how many intervals that took, and a hash of it, so that
// telling the same again is known.
struct Rec {
    quint32 gen = 0;
    quint32 items = 0;
    quint64 hash = 0;
};

// What the sorting and merging need to know, and what they found.
struct Ctx {
    const std::vector<Rec> *rec = nullptr;
    qint64 dropped = 0;     // intervals of formulas that told again, left out of a run
    qint64 buildWork = 0;   // intervals moved
    bool live(const Item &it) const { return it.node < rec->size() && (*rec)[it.node].gen == it.gen; }
};

// A run of intervals in order of where they begin, as a balanced tree laid over the array without any pointers: the middle is the root of the whole, and of the two halves
// either side; `maxHi` at a middle is the largest end of any interval in its half, so that a half that ends before the query begins is passed over whole.
struct Run {
    std::vector<Item> items;
    std::vector<quint32> maxHi;
    qint64 bytes() const { return qint64(items.capacity() * sizeof(Item) + maxHi.capacity() * sizeof(quint32)); }
};

quint32 fillMax(Run &run, size_t l, size_t r)
{
    if (l >= r) return 0;
    const size_t mid = l + (r - l) / 2;
    quint32 m = std::max(run.items[mid].hi, fillMax(run, l, mid));
    m = std::max(m, fillMax(run, mid + 1, r));
    run.maxHi[mid] = m;
    return m;
}

void finishRun(Run &run)
{
    if (run.items.capacity() != run.items.size()) run.items.shrink_to_fit();
    run.maxHi.assign(run.items.size(), 0);
    fillMax(run, 0, run.items.size());
}

// Calls f(item) for each interval of the half [l, r) of the run that meets [qlo, qhi]; the number of middles looked at is added to `work`.
template <class F>
void search(const Run &run, size_t l, size_t r, quint32 qlo, quint32 qhi, F &f, qint64 &work)
{
    while (l < r) {
        const size_t mid = l + (r - l) / 2;
        ++work;
        if (run.maxHi[mid] < qlo) return;
        search(run, l, mid, qlo, qhi, f, work);
        const Item &it = run.items[mid];
        if (it.lo > qhi) return;
        if (it.hi >= qlo) f(it);
        l = mid + 1;
    }
}

bool byLo(const Item &a, const Item &b) { return a.lo < b.lo; }

// The intervals of rows told to one node of the tree: the last few in a buffer, the rest in runs that are merged as they come to be about as long as the one before.
class Intervals {
public:
    qint64 held() const { return m_held; }
    bool empty() const { return m_buffer.empty() && m_runs.empty(); }

    void add(const Item &item, Ctx &ctx)
    {
        const size_t cap = m_buffer.capacity();
        m_buffer.push_back(item);
        m_held += qint64(m_buffer.capacity() - cap) * qint64(sizeof(Item));
        if (m_buffer.size() >= kBufferMax) flush(ctx);
    }

    // Calls f(item) for each interval that meets [lo, hi], and adds what it looked at to `work`.
    template <class F>
    void overlap(quint32 lo, quint32 hi, F &&f, qint64 &work) const
    {
        work += 1 + qint64(m_buffer.size());
        for (const Item &it : m_buffer)
            if (it.lo <= hi && it.hi >= lo) f(it);
        for (const Run &run : m_runs) {
            ++work;
            search(run, 0, run.items.size(), lo, hi, f, work);
        }
    }

    // Everything in one run, with what was dropped left out. False if nothing is left.
    bool rebuild(Ctx &ctx)
    {
        std::vector<Item> all;
        size_t total = m_buffer.size();
        for (const Run &run : m_runs) total += run.items.size();
        all.reserve(total);
        for (const Item &it : m_buffer) keep(it, ctx, &all);
        for (const Run &run : m_runs)
            for (const Item &it : run.items) keep(it, ctx, &all);
        m_buffer = std::vector<Item>();
        m_runs.clear();
        m_runs.shrink_to_fit();
        if (all.empty()) {
            m_held = 0;
            return false;
        }
        std::sort(all.begin(), all.end(), byLo);
        ctx.buildWork += qint64(all.size());
        Run run;
        run.items = std::move(all);
        finishRun(run);
        m_runs.push_back(std::move(run));
        recount();
        return true;
    }

private:
    static void keep(const Item &it, Ctx &ctx, std::vector<Item> *to)
    {
        if (ctx.live(it)) to->push_back(it);
        else ++ctx.dropped;
    }

    void flush(Ctx &ctx)
    {
        size_t kept = 0;
        for (const Item &it : m_buffer) {
            if (ctx.live(it)) m_buffer[kept++] = it;
            else ++ctx.dropped;
        }
        m_buffer.resize(kept);
        if (kept > 0) {
            std::sort(m_buffer.begin(), m_buffer.end(), byLo);
            ctx.buildWork += qint64(kept);
            Run run;
            run.items.assign(m_buffer.begin(), m_buffer.end());
            finishRun(run);
            m_runs.push_back(std::move(run));
            mergeDown(ctx);
        }
        m_buffer.clear();
        recount();
    }

    // Two runs at the end of which the first is no more than twice the second are made one, again and again.
    void mergeDown(Ctx &ctx)
    {
        while (m_runs.size() >= 2 && m_runs[m_runs.size() - 2].items.size() <= 2 * m_runs.back().items.size()) {
            Run merged = merge(m_runs[m_runs.size() - 2], m_runs.back(), ctx);
            m_runs.pop_back();
            m_runs.pop_back();
            if (!merged.items.empty()) m_runs.push_back(std::move(merged));
        }
    }

    static Run merge(const Run &a, const Run &b, Ctx &ctx)
    {
        Run out;
        out.items.reserve(a.items.size() + b.items.size());
        ctx.buildWork += qint64(a.items.size() + b.items.size());
        size_t i = 0, j = 0;
        auto take = [&](const Item &it) { keep(it, ctx, &out.items); };
        while (i < a.items.size() && j < b.items.size()) {
            if (b.items[j].lo < a.items[i].lo) take(b.items[j++]);
            else take(a.items[i++]);
        }
        while (i < a.items.size()) take(a.items[i++]);
        while (j < b.items.size()) take(b.items[j++]);
        finishRun(out);
        return out;
    }

    void recount()
    {
        qint64 bytes = qint64(m_buffer.capacity() * sizeof(Item) + m_runs.capacity() * sizeof(Run));
        for (const Run &run : m_runs) bytes += run.bytes();
        m_held = bytes;
    }

    std::vector<Item> m_buffer;
    std::vector<Run> m_runs;   // the longest first
    qint64 m_held = 0;
};

// A node of the tree over the columns of a sheet is named by its level and its place among the nodes of the level.
quint32 nodeKey(int level, quint32 index) { return (quint32(level) << 16) | index; }

struct SheetIndex {
    std::unordered_map<quint32, Intervals> nodes;
    quint32 levels = 0;   // the levels some node of which exists, a bit for each
};

struct Placement {
    quint32 sheet;
    quint32 key;
    quint32 lo, hi;
};

quint64 hashOf(const std::vector<CellArea> &areas)
{
    quint64 h = 1469598103934665603ull ^ quint64(areas.size());
    auto mix = [&](quint64 v) {
        h ^= v + 0x9E3779B97F4A7C15ull + (h << 6) + (h >> 2);
        h *= 0xFF51AFD7ED558CCDull;
        h ^= h >> 32;
    };
    for (const CellArea &a : areas) {
        mix((quint64(quint32(a.sheet1)) << 32) | quint64(quint32(a.sheet2)));
        mix((quint64(a.row1) << 32) | quint64(a.row2));
        mix((quint64(a.col1) << 32) | quint64(a.col2));
    }
    return h ? h : 1;
}

} // namespace

struct DependencyGraph::Impl {
    std::vector<Rec> rec;
    std::vector<SheetIndex> sheets;
    std::vector<Placement> plan;
    Ctx ctx;
    qint64 live = 0, stale = 0;   // intervals of the latest reads, and of the earlier
    qint64 bytes = 0;             // what the graph takes
    mutable qint64 queryWork = 0;
    qint64 compactions = 0;

    Impl() { ctx.rec = &rec; }

    qint64 tableBytes() const { return qint64(rec.capacity() * sizeof(Rec) + sheets.capacity() * sizeof(SheetIndex)); }

    // The nodes of the tree that hold the columns [c1, c2] between them, and how many.
    static int decompose(quint32 c1, quint32 c2, int *levels, quint32 *indices)
    {
        int n = 0;
        quint32 lo = c1, hi = c2 + 1;
        for (int level = 0; lo < hi && level < kLevels; ++level, lo >>= 1, hi >>= 1) {
            if (lo & 1) {
                levels[n] = level;
                indices[n++] = lo++;
            }
            if (hi & 1) {
                levels[n] = level;
                indices[n++] = --hi;
            }
        }
        return n;
    }

    // Where each of the areas goes. Returns how many of the places have no node yet.
    qint64 placements(const std::vector<CellArea> &areas)
    {
        plan.clear();
        qint64 fresh = 0;
        int levels[2 * kLevels];
        quint32 indices[2 * kLevels];
        for (const CellArea &a : areas) {
            if (a.sheet1 < 0 || a.sheet2 < a.sheet1 || a.col1 >= kColumns || a.col1 > a.col2 || a.row1 > a.row2) continue;
            const int n = decompose(a.col1, std::min<quint32>(a.col2, kColumns - 1), levels, indices);
            const quint32 last = std::min<quint32>(quint32(a.sheet2), quint32(a.sheet1) + kMaxSheetsOfABlock - 1);
            for (quint32 s = quint32(a.sheet1); s <= last; ++s)
                for (int i = 0; i < n; ++i) {
                    const quint32 key = nodeKey(levels[i], indices[i]);
                    plan.push_back({s, key, a.row1, a.row2});
                    if (s >= sheets.size() || sheets[s].nodes.find(key) == sheets[s].nodes.end()) ++fresh;
                }
        }
        return fresh;
    }

    // Lets go of the dropped intervals in every node, and recounts.
    void compactAll()
    {
        ++compactions;
        qint64 held = 0;
        for (SheetIndex &sh : sheets) {
            sh.levels = 0;
            for (auto it = sh.nodes.begin(); it != sh.nodes.end();) {
                if (it->second.rebuild(ctx)) {
                    sh.levels |= quint32(1) << (it->first >> 16);
                    held += it->second.held() + kStructureBytes;
                    ++it;
                } else {
                    it = sh.nodes.erase(it);
                }
            }
        }
        stale = 0;
        ctx.dropped = 0;
        live = 0;
        for (const Rec &r : rec) live += r.items;
        bytes = held + tableBytes();
    }

    bool shouldLetGoOfTheDropped() const { return stale > std::max<qint64>(kStaleFloor, live); }
};

DependencyGraph::DependencyGraph(qint64 maxBytes) : d(new Impl), m_max(maxBytes) {}
DependencyGraph::~DependencyGraph() = default;

bool DependencyGraph::setPrecedents(Node node, const std::vector<CellArea> &areas)
{
    Impl &g = *d;
    if (node >= kMaxNode) return false;
    const quint64 hash = hashOf(areas);
    if (node < g.rec.size()) {
        Rec &r = g.rec[node];
        if (r.hash == hash) return true;   // what it told before: nothing changes
        // what it told before is dropped now, whatever comes of what it tells
        g.live -= r.items;
        g.stale += r.items;
        r.items = 0;
        r.hash = 0;
        if (++r.gen == 0) r.gen = 1;
    }
    // what it will take: its place in the table of formulas, and its intervals (and the nodes of the tree that have none yet)
    size_t tableWant = g.rec.capacity();
    if (size_t(node) + 1 > g.rec.capacity()) tableWant = std::max<size_t>(size_t(node) + 1, g.rec.capacity() + g.rec.capacity() / 2);
    const qint64 fresh = g.placements(areas);
    const qint64 wanted = qint64(tableWant - g.rec.capacity()) * qint64(sizeof(Rec)) + qint64(g.plan.size()) * kPlanItemBytes + fresh * kStructureBytes;
    if (g.bytes + wanted > m_max) {
        // letting go of the dropped is done only if it gives back a sixteenth of what the graph may take at least, so that it is not done again for every formula: else the
        // graph is full, and says no at once, and goes on saying it
        if (g.stale * 20 >= m_max / 16) g.compactAll();
        if (g.bytes + wanted > m_max) return false;
    }
    if (g.rec.size() <= node) {
        const qint64 before = g.tableBytes();
        g.rec.reserve(tableWant);
        g.rec.resize(size_t(node) + 1);
        g.bytes += g.tableBytes() - before;
    }
    if (g.rec[node].gen == 0) g.rec[node].gen = 1;   // (a formula not told before)
    const quint32 gen = g.rec[node].gen;
    for (const Placement &p : g.plan) {
        if (g.sheets.size() <= p.sheet) {
            const qint64 before = g.tableBytes();
            g.sheets.resize(size_t(p.sheet) + 1);
            g.bytes += g.tableBytes() - before;
        }
        SheetIndex &sh = g.sheets[p.sheet];
        auto found = sh.nodes.try_emplace(p.key);
        if (found.second) {
            g.bytes += kStructureBytes;
            sh.levels |= quint32(1) << (p.key >> 16);
        }
        Intervals &iv = found.first->second;
        const qint64 held = iv.held();
        iv.add(Item{p.lo, p.hi, node, gen}, g.ctx);
        g.bytes += iv.held() - held;
        g.stale -= g.ctx.dropped;
        g.ctx.dropped = 0;
    }
    Rec &after = g.rec[node];   // (the table did not move, but it is read again)
    after.items = quint32(std::min<size_t>(g.plan.size(), std::numeric_limits<quint32>::max()));
    after.hash = hash;
    g.live += after.items;
    if (g.shouldLetGoOfTheDropped()) g.compactAll();
    return true;
}

void DependencyGraph::clearPrecedents(Node node)
{
    Impl &g = *d;
    if (node >= g.rec.size()) return;
    Rec &r = g.rec[node];
    g.live -= r.items;
    g.stale += r.items;
    r.items = 0;
    r.hash = 0;
    if (++r.gen == 0) r.gen = 1;
}

void DependencyGraph::dependentsOfCell(qint32 sheet, quint32 row, quint32 col, std::vector<Node> *out) const
{
    const Impl &g = *d;
    if (sheet < 0 || size_t(sheet) >= g.sheets.size() || col >= kColumns) return;
    const SheetIndex &sh = g.sheets[size_t(sheet)];
    qint64 &work = g.queryWork;
    auto report = [&](const Item &it) {
        ++work;
        if (g.ctx.live(it)) out->push_back(it.node);
    };
    for (int level = 0; level < kLevels; ++level) {
        if (!(sh.levels & (quint32(1) << level))) continue;
        ++work;
        const auto found = sh.nodes.find(nodeKey(level, col >> level));
        if (found != sh.nodes.end()) found->second.overlap(row, row, report, work);
    }
}

void DependencyGraph::dependentsOfArea(const CellArea &area, std::vector<Node> *out) const
{
    const Impl &g = *d;
    if (area.col1 >= kColumns || area.col1 > area.col2 || area.row1 > area.row2) return;
    const quint32 col2 = std::min<quint32>(area.col2, kColumns - 1);
    qint64 &work = g.queryWork;
    auto report = [&](const Item &it) {
        ++work;
        if (g.ctx.live(it)) out->push_back(it.node);
    };
    for (qint32 sheet = std::max<qint32>(0, area.sheet1); sheet <= area.sheet2 && size_t(sheet) < g.sheets.size(); ++sheet) {
        const SheetIndex &sh = g.sheets[size_t(sheet)];
        // the nodes whose columns meet the block's: for each level, those from the one that holds its first column to the one that holds its last
        qint64 lookups = 0;
        for (int level = 0; level < kLevels; ++level)
            if (sh.levels & (quint32(1) << level)) lookups += qint64(col2 >> level) - qint64(area.col1 >> level) + 1;
        if (lookups <= qint64(sh.nodes.size())) {
            for (int level = 0; level < kLevels; ++level) {
                if (!(sh.levels & (quint32(1) << level))) continue;
                for (quint32 index = area.col1 >> level; index <= (col2 >> level); ++index) {
                    ++work;
                    const auto found = sh.nodes.find(nodeKey(level, index));
                    if (found != sh.nodes.end()) found->second.overlap(area.row1, area.row2, report, work);
                }
            }
        } else {
            for (const auto &node : sh.nodes) {
                ++work;
                const quint32 level = node.first >> 16, index = node.first & 0xFFFF;
                if (((index + 1) << level) <= area.col1 || (index << level) > col2) continue;
                node.second.overlap(area.row1, area.row2, report, work);
            }
        }
    }
}

qint64 DependencyGraph::entryCount() const { return d->live; }
qint64 DependencyGraph::memoryBytes() const { return d->bytes; }
void DependencyGraph::compact() { d->compactAll(); }
qint64 DependencyGraph::queryWork() const { return d->queryWork; }
qint64 DependencyGraph::compactions() const { return d->compactions; }
qint64 DependencyGraph::buildWork() const { return d->ctx.buildWork; }

} // namespace jp::calc
