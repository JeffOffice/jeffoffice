#include "calc/graph.h"

#include <algorithm>
#include <functional>
#include <limits>
#include <unordered_map>

namespace jp::calc {

namespace {

constexpr quint32 kColumns = 65536;                    // the column ids of a sheet: the leaves of the tree over them (the 16,384 of the grid, and the ids the inserted columns take)
constexpr int kLevels = 17;                            // the levels of the tree: a node of level L holds 2^L columns (the one of level 16 holds all of them)
constexpr int kMaxSheetsOfABlock = 1024;
constexpr quint32 kMarkBit = 0x80000000u;              // in an interval's node: the formula was told as marked (see setPrecedents), which a query can leave out
constexpr quint32 kNodeMask = ~kMarkBit;
constexpr size_t kBufferMax = 64;                      // the intervals a node holds that are not yet in a run, which a query looks at one by one
constexpr quint32 kMaxNode = quint32(1) << 28;         // (the caller numbers its formulas from 0; a number past this is refused rather than made room for)
constexpr qint64 kPlanItemBytes = 24;                  // what an interval is counted to take before it is in a run (20 there: 16 and 4 for the largest end below it)
constexpr qint64 kStructureBytes = 160;                // what a node of the tree takes, besides its intervals and their buffer
constexpr size_t kKeepPlan = 4096;                     // a list of the places of a formula longer than this is let go of when the formula is done
constexpr size_t kMostPlaces = size_t(1) << 22;        // a formula that names more places than this is refused before they are listed
constexpr qint64 kStaleFloor = 1 << 14;                // the dropped intervals that are let go of only past this many (or as many as are held)

struct Item {
    quint32 lo, hi, node, gen;
};

// What a node of the tree that is not there yet is counted to take: its entry in the map, and the buffer it reserves for its first intervals.
constexpr qint64 kFreshStructureBytes = kStructureBytes + qint64(kBufferMax * sizeof(Item));

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
    bool live(const Item &it) const { return (it.node & kNodeMask) < rec->size() && (*rec)[it.node & kNodeMask].gen == it.gen; }
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
void search(const Run &run, size_t l, size_t r, quint32 qlo, quint32 qhi, F &f, qint64 &work, const bool *abort)
{
    while (l < r) {
        if (abort && *abort) return;
        const size_t mid = l + (r - l) / 2;
        ++work;
        if (run.maxHi[mid] < qlo) return;
        search(run, l, mid, qlo, qhi, f, work, abort);
        if (abort && *abort) return;
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

    // An interval is counted to take what it takes in a run as soon as it is put in the buffer (which is reserved whole with the first), so that the node does not grow past
    // what was counted when its intervals are made a run.
    void add(const Item &item, Ctx &ctx)
    {
        if (m_buffer.capacity() == 0) {
            m_buffer.reserve(kBufferMax);
            m_held += qint64(kBufferMax * sizeof(Item));
        }
        m_buffer.push_back(item);
        m_held += kPlanItemBytes;
        if (m_buffer.size() >= kBufferMax) flush(ctx);
    }

    // Calls f(item) for each interval that meets [lo, hi], and adds what it looked at to `work`.
    template <class F>
    void overlap(quint32 lo, quint32 hi, F &&f, qint64 &work, const bool *abort = nullptr) const
    {
        work += 1 + qint64(m_buffer.size());
        for (const Item &it : m_buffer) {
            if (abort && *abort) return;
            if (it.lo <= hi && it.hi >= lo) f(it);
        }
        for (const Run &run : m_runs) {
            if (abort && *abort) return;
            ++work;
            search(run, 0, run.items.size(), lo, hi, f, work, abort);
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
        m_buffer.clear();   // (its room stays reserved)
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
        qint64 bytes = qint64(m_buffer.capacity() * sizeof(Item) + m_runs.capacity() * sizeof(Run)) + qint64(m_buffer.size()) * kPlanItemBytes;
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

quint64 hashOf(const std::vector<CellArea> &areas, bool marked)
{
    quint64 h = 1469598103934665603ull ^ quint64(areas.size()) ^ (marked ? 0x5bd1e995ull : 0);
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

    qint64 planBytes() const { return qint64(plan.capacity() * sizeof(Placement)); }

    // Where each of the areas goes, in order of sheet and node. Returns how many of the nodes of the tree they go to have no entry yet; -1 if there are more places than `most`
    // (a formula that names more than the graph could ever hold is refused before its places are listed, whatever the sheets and columns it names).
    qint64 placements(const std::vector<CellArea> &areas, size_t most)
    {
        plan.clear();
        int levels[2 * kLevels];
        quint32 indices[2 * kLevels];
        for (const CellArea &a : areas) {
            if (a.sheet1 < 0 || a.sheet2 < a.sheet1 || a.col1 >= kColumns || a.col1 > a.col2 || a.row1 > a.row2) continue;
            const int n = decompose(a.col1, std::min<quint32>(a.col2, kColumns - 1), levels, indices);
            const quint32 last = std::min<quint32>(quint32(a.sheet2), quint32(a.sheet1) + kMaxSheetsOfABlock - 1);
            for (quint32 s = quint32(a.sheet1); s <= last; ++s)
                for (int i = 0; i < n; ++i) {
                    if (plan.size() >= most) return -1;
                    plan.push_back({s, nodeKey(levels[i], indices[i]), a.row1, a.row2});
                }
        }
        std::sort(plan.begin(), plan.end(), [](const Placement &x, const Placement &y) { return x.sheet != y.sheet ? x.sheet < y.sheet : x.key < y.key; });
        qint64 fresh = 0;
        for (size_t i = 0; i < plan.size(); ++i) {
            if (i > 0 && plan[i].sheet == plan[i - 1].sheet && plan[i].key == plan[i - 1].key) continue;   // (the nodes are counted once each)
            if (plan[i].sheet >= sheets.size() || sheets[plan[i].sheet].nodes.find(plan[i].key) == sheets[plan[i].sheet].nodes.end()) ++fresh;
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

bool DependencyGraph::setPrecedents(Node node, const std::vector<CellArea> &areas, bool marked)
{
    Impl &g = *d;
    if (node >= kMaxNode) return false;
    const quint64 hash = hashOf(areas, marked);
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
    // what it will take: its place in the table of formulas, its intervals, the nodes of the tree that have none yet, and the sheets it names that the graph has not made room
    // for; and, while the formula is told, the list of its places
    struct Scratch {
        Impl &g;
        ~Scratch()
        {
            if (g.plan.capacity() > kKeepPlan) std::vector<Placement>().swap(g.plan);
        }
    } scratch{g};
    size_t tableWant = g.rec.capacity();
    if (size_t(node) + 1 > g.rec.capacity()) tableWant = std::max<size_t>(size_t(node) + 1, g.rec.capacity() + g.rec.capacity() / 2);
    const qint64 room = std::max<qint64>(1, (m_max - g.bytes) / (kPlanItemBytes + qint64(sizeof(Placement))));
    const qint64 fresh = g.placements(areas, size_t(std::min<qint64>(room, qint64(kMostPlaces))));
    if (fresh < 0) return false;
    const size_t sheetsWant = g.plan.empty() ? g.sheets.size() : std::max<size_t>(g.sheets.size(), size_t(g.plan.back().sheet) + 1);
    const qint64 sheetGrowth = sheetsWant > g.sheets.capacity() ? qint64(sheetsWant - g.sheets.capacity()) * qint64(sizeof(SheetIndex)) : 0;
    const qint64 wanted = qint64(tableWant - g.rec.capacity()) * qint64(sizeof(Rec)) + qint64(g.plan.size()) * kPlanItemBytes + fresh * kFreshStructureBytes + sheetGrowth;
    // (a margin of a 256th of the bytes is left, for what grows by more than it was counted to: the lists of runs)
    auto over = [&]() { return g.bytes + g.planBytes() + wanted + m_max / 256 > m_max; };
    if (over()) {
        // letting go of the dropped is done only if it gives back a sixteenth of what the graph may take at least, so that it is not done again for every formula: else the
        // graph is full, and says no at once, and goes on saying it
        if (g.stale * 20 >= m_max / 16) g.compactAll();
        if (over()) return false;
    }
    if (g.rec.size() <= node) {
        const qint64 before = g.tableBytes();
        g.rec.reserve(tableWant);
        g.rec.resize(size_t(node) + 1);
        g.bytes += g.tableBytes() - before;
    }
    if (sheetsWant > g.sheets.size()) {
        const qint64 before = g.tableBytes();
        g.sheets.reserve(sheetsWant);
        g.sheets.resize(sheetsWant);
        g.bytes += g.tableBytes() - before;
    }
    if (g.rec[node].gen == 0) g.rec[node].gen = 1;   // (a formula not told before)
    const quint32 gen = g.rec[node].gen;
    for (const Placement &p : g.plan) {
        SheetIndex &sh = g.sheets[p.sheet];
        auto found = sh.nodes.try_emplace(p.key);
        if (found.second) {
            g.bytes += kStructureBytes;
            sh.levels |= quint32(1) << (p.key >> 16);
        }
        Intervals &iv = found.first->second;
        const qint64 held = iv.held();
        iv.add(Item{p.lo, p.hi, node | (marked ? kMarkBit : 0u), gen}, g.ctx);
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

namespace {

// Calls report(item) for each interval of the sheet that meets the block's rows ([row1, row2], or, when row1 is past row2, those that hold both: begin at or before row2 and end at or
// after row1) in the nodes whose columns meet [col1, col2]; `work` counts what was looked at. Stops when `aborted` is set.
template <class F>
void overlapSheet(const SheetIndex &sh, quint32 row1, quint32 row2, quint32 col1, quint32 col2, F &report, qint64 &work, const bool *abortFlag, const bool *aborted)
{
    // the nodes whose columns meet the block's: for each level, those from the one that holds its first column to the one that holds its last
    qint64 lookups = 0;
    for (int level = 0; level < kLevels; ++level)
        if (sh.levels & (quint32(1) << level)) lookups += qint64(col2 >> level) - qint64(col1 >> level) + 1;
    if (lookups <= qint64(sh.nodes.size())) {
        for (int level = 0; level < kLevels; ++level) {
            if (!(sh.levels & (quint32(1) << level))) continue;
            for (quint32 index = col1 >> level; index <= (col2 >> level) && !*aborted; ++index) {
                ++work;
                const auto found = sh.nodes.find(nodeKey(level, index));
                if (found != sh.nodes.end()) found->second.overlap(row1, row2, report, work, abortFlag);
            }
        }
    } else {
        for (const auto &node : sh.nodes) {
            if (*aborted) break;
            ++work;
            const quint32 level = node.first >> 16, index = node.first & 0xFFFF;
            if (((index + 1) << level) <= col1 || (index << level) > col2) continue;
            node.second.overlap(row1, row2, report, work, abortFlag);
        }
    }
}

} // namespace

bool DependencyGraph::dependentsOfCell(qint32 sheet, quint32 row, quint32 col, std::vector<Node> *out, const std::atomic<bool> *stop, bool skipMarked) const
{
    const Impl &g = *d;
    if (sheet < 0 || size_t(sheet) >= g.sheets.size() || col >= kColumns) return true;
    const SheetIndex &sh = g.sheets[size_t(sheet)];
    qint64 &work = g.queryWork;
    bool aborted = false;
    qint64 found = 0;
    auto report = [&](const Item &it) {
        ++work;
        if (skipMarked && (it.node & kMarkBit)) return;
        if (g.ctx.live(it)) out->push_back(it.node & kNodeMask);
        if (stop && (++found & 4095) == 0 && stop->load(std::memory_order_relaxed)) aborted = true;
    };
    for (int level = 0; level < kLevels && !aborted; ++level) {
        if (!(sh.levels & (quint32(1) << level))) continue;
        ++work;
        const auto node = sh.nodes.find(nodeKey(level, col >> level));
        if (node != sh.nodes.end()) node->second.overlap(row, row, report, work, stop ? &aborted : nullptr);
    }
    return !aborted;
}

bool DependencyGraph::dependentsOfArea(const CellArea &area, std::vector<Node> *out, const std::atomic<bool> *stop, bool skipMarked) const
{
    const Impl &g = *d;
    if (area.col1 >= kColumns || area.col1 > area.col2 || area.row1 > area.row2) return true;
    const quint32 col2 = std::min<quint32>(area.col2, kColumns - 1);
    qint64 &work = g.queryWork;
    bool aborted = false;
    qint64 found = 0;
    auto report = [&](const Item &it) {
        ++work;
        if (skipMarked && (it.node & kMarkBit)) return;
        if (g.ctx.live(it)) out->push_back(it.node & kNodeMask);
        if (stop && (++found & 4095) == 0 && stop->load(std::memory_order_relaxed)) aborted = true;
    };
    for (qint32 sheet = std::max<qint32>(0, area.sheet1); sheet <= area.sheet2 && size_t(sheet) < g.sheets.size() && !aborted; ++sheet)
        overlapSheet(g.sheets[size_t(sheet)], area.row1, area.row2, area.col1, col2, report, work, stop ? &aborted : nullptr, &aborted);
    return !aborted;
}

bool DependencyGraph::forEachDependent(const CellArea &area, const std::function<bool(Node)> &visit, bool skipMarked) const
{
    const Impl &g = *d;
    if (area.col1 >= kColumns || area.col1 > area.col2 || area.row1 > area.row2) return true;
    const quint32 col2 = std::min<quint32>(area.col2, kColumns - 1);
    qint64 &work = g.queryWork;
    bool aborted = false;
    auto report = [&](const Item &it) {
        ++work;
        if (skipMarked && (it.node & kMarkBit)) return;
        if (g.ctx.live(it) && !visit(it.node & kNodeMask)) aborted = true;
    };
    for (qint32 sheet = std::max<qint32>(0, area.sheet1); sheet <= area.sheet2 && size_t(sheet) < g.sheets.size() && !aborted; ++sheet)
        overlapSheet(g.sheets[size_t(sheet)], area.row1, area.row2, area.col1, col2, report, work, &aborted, &aborted);
    return !aborted;
}

bool DependencyGraph::dependentsHoldingRows(qint32 sheet, quint32 before, quint32 after, std::vector<Node> *out, const std::atomic<bool> *stop, bool skipMarked) const
{
    const Impl &g = *d;
    if (sheet < 0 || size_t(sheet) >= g.sheets.size() || before >= after) return true;
    qint64 &work = g.queryWork;
    bool aborted = false;
    qint64 found = 0;
    auto report = [&](const Item &it) {
        ++work;
        if (skipMarked && (it.node & kMarkBit)) return;
        if (g.ctx.live(it)) out->push_back(it.node & kNodeMask);
        if (stop && (++found & 4095) == 0 && stop->load(std::memory_order_relaxed)) aborted = true;
    };
    // (an interval holds both when it begins at or before `before` and ends at or after `after`: the search for those that meet [after, before], which the search takes for an empty block)
    overlapSheet(g.sheets[size_t(sheet)], after, before, 0, kColumns - 1, report, work, stop ? &aborted : nullptr, &aborted);
    return !aborted;
}

bool DependencyGraph::forEachHoldingRows(qint32 sheet, quint32 before, quint32 after, const std::function<bool(Node)> &visit, bool skipMarked) const
{
    const Impl &g = *d;
    if (sheet < 0 || size_t(sheet) >= g.sheets.size() || before >= after) return true;
    qint64 &work = g.queryWork;
    bool aborted = false;
    auto report = [&](const Item &it) {
        ++work;
        if (skipMarked && (it.node & kMarkBit)) return;
        if (g.ctx.live(it) && !visit(it.node & kNodeMask)) aborted = true;
    };
    overlapSheet(g.sheets[size_t(sheet)], after, before, 0, kColumns - 1, report, work, &aborted, &aborted);
    return !aborted;
}

qint64 DependencyGraph::entryCount() const { return d->live; }
qint64 DependencyGraph::memoryBytes() const { return d->bytes + d->planBytes(); }
void DependencyGraph::compact() { d->compactAll(); }
qint64 DependencyGraph::queryWork() const { return d->queryWork; }
qint64 DependencyGraph::compactions() const { return d->compactions; }
qint64 DependencyGraph::buildWork() const { return d->ctx.buildWork; }

} // namespace jp::calc
