// SPIKE (measurement only, not wired into the build, not product code): the core of two designs for cheap structural edits, side by side, on the real dependency graph.
//   A: row and column ids kept in run maps (LineMap), cells held by id, formulas stored in the id frame (relative parts are id distances), the graph holds ids and is never shifted.
//   B: the rival: formulas stored relative to their own cell with the absolute parts apart (no text rewrite on rigid moves), a block-offset row map (blocks of rows with a Fenwick tree over
//      their counts and a stable token for each row) for the cell store only, positions everywhere else, and the graph's existing shiftRows pass.
// Build (see docs/jeffcrunch-rowids.md, "The spike"): g++ -std=c++17 -O2 with the Qt include paths, libs/base/calc/graph.cpp, and QtCore.
// Run: rowid_spike A|B ROWS NUMERIC_COLS [steady_edits=1000] [check]
#include "calc/graph.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <vector>

using jp::calc::CellArea;
using jp::calc::DependencyGraph;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int64_t i64;

namespace {

constexpr u32 kRows = 1u << 20;
constexpr u32 kNone = 0xFFFFFFFFu;
enum Kind : u8 { K_SUMROW, K_DIFF, K_TOTAL, K_WHOLE, K_FILL };

struct Cell {
    double number = 0;
    u32 row = 0;      // A: the row id; B: unused
    u16 col = 0;      // A: the column id; B: the column position
    u8 type = 1, flags = 0;
    u32 style = 0;
    int32_t formula = -1;
};
static_assert(sizeof(Cell) == 24, "a cell is 24 bytes");

double now()
{
    using namespace std::chrono;
    return duration<double, std::milli>(steady_clock::now().time_since_epoch()).count();
}

i64 rssMb()
{
    FILE *f = fopen("/proc/self/status", "r");
    char line[256];
    i64 v = -1;
    while (f && fgets(line, sizeof line, f))
        if (!strncmp(line, "VmRSS:", 6)) v = atoll(line + 6) / 1024;
    if (f) fclose(f);
    return v;
}

double numberAt(u32 r, u32 c) { return double((r * 7 + c * 13) % 1000); }
double bad() { return 7777.0; }

struct Node {
    u32 row = 0;   // A: row id; B: row token
    u16 col = 0;
    u8 kind = 0;
    double value = 0;
};

// What the driver asks of each design.
struct Engine {
    virtual ~Engine() = default;
    virtual void build(u32 rows, u32 numeric) = 0;
    virtual void registerAll() = 0;
    virtual double recalc() = 0;            // checksum of all values in the order of position
    virtual i64 total() = 0;                // live rows
    // each returns the milliseconds the window thread took; `undo` is the same for the inverse
    virtual double insertRows(i64 pos, u32 n, double *workerQuestion) = 0;
    virtual double deleteRows(i64 pos, u32 n, double *workerQuestion) = 0;
    virtual double undoLastRows() = 0;
    virtual double insertCol(i64 pos, double *workerQuestion) = 0;
    virtual double undoLastCol() = 0;
    virtual double paint() = 0;
    virtual double fillColumn(u32 &made) = 0;
    virtual double pasteBlock(u32 rows, u32 cols) = 0;
    virtual double readRefs(u32 count) = 0;   // the positions a formula bar or Find asks for: where each of `count` formulas (all if 0) and what it reads are now
    virtual size_t runs() = 0;
    virtual i64 graphEntries() = 0;
    virtual i64 graphBytes() = 0;
};

// ===================================================================================== A

struct LineMap {
    struct Run {
        u32 first, count;
    };
    struct Dead {
        u32 first, count, before, after;
    };
    std::vector<Run> runs;
    std::vector<i64> pre;
    std::vector<u32> byFirst;
    std::vector<Dead> dead;   // sorted by first
    u32 next;
    LineMap(u32 total, u32 nextId) : next(nextId)
    {
        runs.push_back({0, total});
        rebuild();
    }
    void rebuild()
    {
        pre.assign(runs.size() + 1, 0);
        for (size_t i = 0; i < runs.size(); ++i) pre[i + 1] = pre[i] + runs[i].count;
        byFirst.resize(runs.size());
        for (size_t i = 0; i < runs.size(); ++i) byFirst[i] = u32(i);
        std::sort(byFirst.begin(), byFirst.end(), [&](u32 a, u32 b) { return runs[a].first < runs[b].first; });
    }
    i64 total() const { return pre.back(); }
    size_t runAt(i64 p) const { return size_t(std::upper_bound(pre.begin(), pre.end(), p) - pre.begin()) - 1; }
    u32 idAt(i64 p) const
    {
        if (runs.size() == 1) return u32(p);
        const size_t r = runAt(p);
        return runs[r].first + u32(p - pre[r]);
    }
    i64 posOf(u32 id) const
    {
        if (runs.size() == 1) return id < runs[0].count ? i64(id) : -1;
        size_t lo = 0, hi = byFirst.size();
        while (lo < hi) {
            const size_t mid = (lo + hi) / 2;
            if (runs[byFirst[mid]].first <= id) lo = mid + 1;
            else hi = mid;
        }
        if (lo == 0) return -1;
        const size_t r = byFirst[lo - 1];
        if (id >= runs[r].first + runs[r].count) return -1;
        return pre[r] + (id - runs[r].first);
    }
    bool live(u32 id) const { return posOf(id) >= 0; }
    // splits so that a run boundary is at position p (0 < p < total)
    void splitAt(i64 p)
    {
        const size_t r = runAt(p);
        if (pre[r] == p) return;
        const u32 left = u32(p - pre[r]);
        Run second{runs[r].first + left, runs[r].count - left};
        runs[r].count = left;
        runs.insert(runs.begin() + long(r) + 1, second);
        rebuild();
    }
    Run insert(i64 p, u32 n)
    {
        Run nr{next, n};
        next += n;
        if (p >= total()) {
            runs.push_back(nr);
        } else {
            if (p > 0) splitAt(p);
            runs.insert(runs.begin() + long(p > 0 ? runAt(p) : 0), nr);
        }
        rebuild();
        return nr;
    }
    std::vector<Run> erase(i64 p, u32 n)
    {
        std::vector<Run> out;
        const u32 before = p > 0 ? idAt(p - 1) : kNone;
        if (p > 0) splitAt(p);
        if (p + n < total()) splitAt(p + n);
        const u32 after = p + n < total() ? idAt(p + n) : kNone;
        size_t r = p == 0 ? 0 : runAt(p);
        i64 left = n;
        while (left > 0) {
            out.push_back(runs[r]);
            left -= runs[r].count;
            runs.erase(runs.begin() + long(r));
        }
        rebuild();
        for (const Run &x : out) {
            Dead d{x.first, x.count, before, after};
            dead.insert(std::lower_bound(dead.begin(), dead.end(), d, [](const Dead &a, const Dead &b) { return a.first < b.first; }), d);
        }
        return out;
    }
    void revive(i64 p, const std::vector<Run> &rs)
    {
        for (const Run &x : rs) {
            auto it = std::lower_bound(dead.begin(), dead.end(), x.first, [](const Dead &a, u32 f) { return a.first < f; });
            if (it != dead.end() && it->first == x.first) dead.erase(it);
        }
        if (p >= total()) {
            for (const Run &x : rs) runs.push_back(x);
        } else {
            if (p > 0) splitAt(p);
            const size_t at = p > 0 ? runAt(p) : 0;
            runs.insert(runs.begin() + long(at), rs.begin(), rs.end());
        }
        rebuild();
    }
    u32 deadRec(u32 id, bool wantAfter) const
    {
        auto it = std::upper_bound(dead.begin(), dead.end(), id, [](u32 f, const Dead &a) { return f < a.first; });
        if (it == dead.begin()) return kNone;
        --it;
        if (id >= it->first + it->count) return kNone;
        return wantAfter ? it->after : it->before;
    }
    // a start that is not live resolves to what followed the rows when they were deleted; an end to what preceded them
    u32 resolveStart(u32 id) const
    {
        for (int guard = 0; id != kNone && !live(id) && guard < 100000; ++guard) id = deadRec(id, true);
        return id;
    }
    u32 resolveEnd(u32 id) const
    {
        for (int guard = 0; id != kNone && !live(id) && guard < 100000; ++guard) id = deadRec(id, false);
        return id;
    }
    // the id runs a range of positions covers
    std::vector<Run> walk(i64 p1, i64 p2) const
    {
        std::vector<Run> out;
        if (p1 > p2) return out;
        size_t r = runAt(p1);
        for (; r < runs.size() && pre[r] <= p2; ++r) {
            const i64 a = std::max(p1, pre[r]), b = std::min(p2, pre[r + 1] - 1);
            out.push_back({runs[r].first + u32(a - pre[r]), u32(b - a + 1)});
        }
        return out;
    }
};

class EngineA : public Engine {
public:
    u32 R = 0, C = 0;   // C: numeric columns; the sum is column C, the difference column C+1
    LineMap rows{kRows, kRows}, cols{8000, 8000};
    std::map<u32, std::vector<Cell>> blocks;   // by row id / 64, in order of (row id, column id)
    std::vector<Node> nodes;
    std::vector<double> sumOf;                  // by row id (original rows)
    DependencyGraph graph{qint64(3) << 30};
    std::vector<LineMap::Run> lastRows;
    i64 lastPos = 0;
    bool lastWasInsert = false;
    // the cache of position ranges to id runs, made again after an edit (the epoch)
    bool cacheOk = false;
    std::vector<LineMap::Run> cacheCols, cacheRows;
    u32 totalsFirst = 0;
    u32 sumCol() const { return C; }

    const Cell *find(u32 row, u16 col) const
    {
        auto it = blocks.find(row >> 6);
        if (it == blocks.end()) return nullptr;
        const auto &v = it->second;
        auto c = std::lower_bound(v.begin(), v.end(), std::make_pair(row, col), [](const Cell &a, const std::pair<u32, u16> &k) { return a.row != k.first ? a.row < k.first : a.col < k.second; });
        return c != v.end() && c->row == row && c->col == col ? &*c : nullptr;
    }
    void put(const Cell &cell)
    {
        auto &v = blocks[cell.row >> 6];
        auto c = std::lower_bound(v.begin(), v.end(), cell, [](const Cell &a, const Cell &k) { return a.row != k.row ? a.row < k.row : a.col < k.col; });
        if (c != v.end() && c->row == cell.row && c->col == cell.col) *c = cell;
        else v.insert(c, cell);
    }
    void build(u32 rowsN, u32 numeric) override
    {
        R = rowsN;
        C = numeric;
        for (u32 b = 0; b <= (R - 1) >> 6; ++b) {
            auto &v = blocks[b];
            v.reserve(64 * (C + 2));
            for (u32 r = b * 64; r < std::min(R, b * 64 + 64); ++r) {
                for (u32 c = 0; c < C; ++c) {
                    Cell cell;
                    cell.number = numberAt(r, c);
                    cell.row = r;
                    cell.col = u16(c);
                    v.push_back(cell);
                }
                Cell s;
                s.row = r;
                s.col = u16(C);
                s.formula = int32_t(r);
                v.push_back(s);
                Cell d;
                d.row = r;
                d.col = u16(C + 1);
                d.formula = int32_t(R + r);
                v.push_back(d);
            }
        }
        nodes.resize(size_t(R) * 2 + 7);
        sumOf.assign(R, 0);
        for (u32 r = 0; r < R; ++r) {
            nodes[r] = Node{r, u16(C), K_SUMROW, 0};
            nodes[R + r] = Node{r, u16(C + 1), K_DIFF, 0};
        }
        totalsFirst = 2 * R;
        for (u32 i = 0; i < 6; ++i) nodes[totalsFirst + i] = Node{0, u16(C + 2 + i), K_TOTAL, 0};
        nodes[totalsFirst + 6] = Node{0, u16(C + 8), K_WHOLE, 0};
    }
    void registerNode(u32 n)
    {
        const Node &nd = nodes[n];
        std::vector<CellArea> a;
        CellArea x;
        switch (nd.kind) {
        case K_SUMROW: x.row1 = x.row2 = nd.row; x.col1 = 0; x.col2 = C - 1; a.push_back(x); break;
        case K_DIFF: x.row1 = x.row2 = nd.row; x.col1 = x.col2 = C; a.push_back(x); if (nd.row > 0) { x.row1 = x.row2 = nd.row - 1; a.push_back(x); } break;
        case K_TOTAL: x.row1 = 0; x.row2 = R - 1; x.col1 = x.col2 = C; a.push_back(x); break;
        case K_WHOLE: x.row1 = 0; x.row2 = 0xFFFFFFFFu; x.col1 = x.col2 = 0; a.push_back(x); break;
        case K_FILL: x.row1 = x.row2 = nd.row; x.col1 = x.col2 = C; a.push_back(x); break;
        }
        graph.setPrecedents(n, a);
    }
    void registerAll() override
    {
        for (u32 n = 0; n < nodes.size(); ++n) registerNode(n);
    }
    void epoch()
    {
        cacheOk = false;
    }
    void refreshCache()
    {
        if (cacheOk) return;
        // SUM(A:T of the row): the numeric columns, from the column with id 0 to the one with id C-1
        const u32 s = cols.resolveStart(0), e = cols.resolveEnd(C - 1);
        cacheCols.clear();
        if (s != kNone && e != kNone) cacheCols = cols.walk(cols.posOf(s), cols.posOf(e));
        const u32 rs = rows.resolveStart(0), re = rows.resolveEnd(R - 1);
        cacheRows.clear();
        if (rs != kNone && re != kNone && rows.posOf(rs) <= rows.posOf(re)) cacheRows = rows.walk(rows.posOf(rs), rows.posOf(re));
        cacheOk = true;
    }
    double rowSum(u32 rowId)
    {
        double sum = 0;
        auto it = blocks.find(rowId >> 6);
        if (it == blocks.end()) return 0;
        const auto &v = it->second;
        auto c = std::lower_bound(v.begin(), v.end(), rowId, [](const Cell &a, u32 r) { return a.row < r; });
        for (; c != v.end() && c->row == rowId; ++c)
            for (const auto &cr : cacheCols)
                if (c->col >= cr.first && c->col < cr.first + cr.count) {
                    sum += c->number;
                    break;
                }
        return sum;
    }
    i64 total() override { return rows.total() - (kRows - R); }   // (the rows past the used ones are ids too, but hold nothing)
    // the live id runs that can hold cells, in order of id: the original rows up to R, and the inserted ones
    std::vector<LineMap::Run> usedRuns() const
    {
        std::vector<LineMap::Run> out;
        for (size_t i : rows.byFirst) {
            const auto &run = rows.runs[i];
            const u32 end = run.first < kRows ? std::min(run.first + run.count, R) : run.first + run.count;
            if (end > run.first) out.push_back({run.first, end - run.first});
        }
        return out;
    }
    template <class F>
    void forCellsInIds(u32 a, u32 b, F f) const
    {
        for (auto it = blocks.lower_bound(a >> 6); it != blocks.end() && it->first <= (b >> 6); ++it) {
            const auto &v = it->second;
            auto c = std::lower_bound(v.begin(), v.end(), a, [](const Cell &x, u32 r) { return x.row < r; });
            for (; c != v.end() && c->row <= b; ++c) f(*c);
        }
    }
    // the ids that can hold cells: the original rows up to R and every inserted id
    template <class F>
    void eachUsedId(F f)
    {
        for (const auto &run : rows.runs) {
            const u32 end = run.first < kRows ? std::min(run.first + run.count, R) : run.first + run.count;
            for (u32 id = run.first; id < end; ++id) f(id);
        }
    }
    double recalc() override
    {
        refreshCache();
        for (const auto &run : usedRuns()) {
            if (run.first >= R) continue;
            u32 cur = kNone;
            double acc = 0;
            forCellsInIds(run.first, run.first + run.count - 1, [&](const Cell &c) {
                if (c.row != cur) {
                    if (cur != kNone) sumOf[cur] = acc;
                    cur = c.row;
                    acc = 0;
                }
                for (const auto &cr : cacheCols)
                    if (c.col >= cr.first && c.col < cr.first + cr.count) {
                        acc += c.number;
                        break;
                    }
            });
            if (cur != kNone) sumOf[cur] = acc;
        }
        // differences: the row the formula was written against, by id
        eachUsedId([&](u32 id) {
            if (id >= R) return;
            if (id == 0 || !rows.live(id - 1)) nodes[R + id].value = bad();
            else nodes[R + id].value = sumOf[id] - sumOf[id - 1];
        });
        // totals over the column of sums
        double t = 0;
        for (const auto &r : cacheRows)
            for (u32 id = r.first; id < r.first + r.count && id < R; ++id) t += sumOf[id];
        for (u32 i = 0; i < 6; ++i) nodes[totalsFirst + i].value = t + i;
        double whole = 0;
        {
            const u32 colId0 = cols.resolveStart(0);
            for (const auto &run : usedRuns())
                forCellsInIds(run.first, run.first + run.count - 1, [&](const Cell &c) {
                    if (c.col == colId0) whole += c.number;
                });
        }
        nodes[totalsFirst + 6].value = whole;
        // fills
        for (size_t n = size_t(totalsFirst) + 7; n < nodes.size(); ++n) {
            const Cell *c = find(nodes[n].row, u16(C));
            nodes[n].value = (c ? sumOf[nodes[n].row < R ? nodes[n].row : 0] : 0) * 2;
        }
        // the checksum, in the order of position
        double sum = 0;
        i64 p = 0;
        eachUsedId([&](u32 id) {
            if (id < R) sum += (sumOf[id] + nodes[R + id].value * 3) * double(p % 1000 + 1);
            ++p;
        });
        for (u32 i = 0; i < 7; ++i) sum += nodes[totalsFirst + i].value;
        return sum;
    }
    double insertRows(i64 pos, u32 n, double *wq) override
    {
        const double t0 = now();
        lastRows = {rows.insert(pos, n)};
        lastPos = pos;
        lastWasInsert = true;
        epoch();
        const double t1 = now();
        if (wq) {
            // the worker's question: the formulas that read around the gap
            const u32 a = pos > 0 ? rows.idAt(pos - 1) : 0, b = rows.idAt(pos + n < rows.total() ? pos + n : rows.total() - 1);
            std::vector<u32> out;
            CellArea q;
            q.row1 = std::min(a, b);
            q.row2 = std::max(a, b);
            q.col2 = 16383;
            const double w0 = now();
            graph.dependentsOfArea(q, &out);
            *wq = now() - w0;
        }
        return t1 - t0;
    }
    double deleteRows(i64 pos, u32 n, double *wq) override
    {
        const double t0 = now();
        lastRows = rows.erase(pos, n);
        lastPos = pos;
        lastWasInsert = false;
        epoch();
        const double t1 = now();
        if (wq) {
            std::vector<u32> out;
            const double w0 = now();
            for (const auto &r : lastRows) {
                CellArea q;
                q.row1 = r.first;
                q.row2 = r.first + r.count - 1;
                q.col2 = 16383;
                graph.dependentsOfArea(q, &out);
            }
            *wq = now() - w0;
        }
        return t1 - t0;
    }
    double undoLastRows() override
    {
        const double t0 = now();
        if (lastWasInsert) {
            u32 n = 0;
            for (auto &r : lastRows) n += r.count;
            rows.erase(lastPos, n);
            // (the dead record of an undone insert is dropped: nothing reads those ids)
        } else {
            rows.revive(lastPos, lastRows);
        }
        epoch();
        return now() - t0;
    }
    double insertCol(i64 pos, double *wq) override
    {
        const double t0 = now();
        cols.insert(pos, 1);
        epoch();
        const double t1 = now();
        if (wq) {
            std::vector<u32> out;
            CellArea q;
            q.col1 = u32(cols.idAt(pos - 1));
            q.col2 = u32(cols.idAt(pos + 1));
            q.row2 = kRows - 1;
            const double w0 = now();
            graph.dependentsOfArea(q, &out);
            *wq = now() - w0;
        }
        return t1 - t0;
    }
    double undoLastCol() override
    {
        const double t0 = now();
        cols.erase(2, 1);
        epoch();
        return now() - t0;
    }
    double paint() override
    {
        const double t0 = now();
        double acc = 0;
        const i64 start = rows.total() / 2;
        for (int rep = 0; rep < 20; ++rep)
            for (i64 p = start; p < start + 50; ++p) {
                const u32 id = rows.idAt(p);
                for (i64 cp = 0; cp < 30; ++cp) {
                    const u32 cid = cols.idAt(cp);
                    const Cell *c = find(id, u16(cid));
                    if (c) acc += c->number;
                }
            }
        if (acc == -1) puts("");
        return (now() - t0) / 20;
    }
    // new cells (in order of row id) merged into their blocks
    void bulkAdd(std::vector<Cell> &fresh)
    {
        size_t i = 0;
        while (i < fresh.size()) {
            const u32 key = fresh[i].row >> 6;
            size_t j = i;
            while (j < fresh.size() && (fresh[j].row >> 6) == key) ++j;
            auto &v = blocks[key];
            const size_t mid = v.size();
            v.insert(v.end(), fresh.begin() + long(i), fresh.begin() + long(j));
            std::inplace_merge(v.begin(), v.begin() + long(mid), v.end(), [](const Cell &a, const Cell &b) { return a.row != b.row ? a.row < b.row : a.col < b.col; });
            i = j;
        }
    }
    double fillColumn(u32 &made) override
    {
        // a formula filled down a new column (the column with id C+10), one per live row
        const double t0 = now();
        made = 0;
        const size_t first = nodes.size();
        std::vector<Cell> fresh;
        for (const auto &run : usedRuns())
            for (u32 id = run.first; id < run.first + run.count; ++id) {
                Cell c;
                c.row = id;
                c.col = u16(C + 10);
                c.formula = int32_t(nodes.size());
                fresh.push_back(c);
                nodes.push_back(Node{id, u16(C + 10), K_FILL, 0});
                ++made;
            }
        bulkAdd(fresh);
        for (size_t n = first; n < nodes.size(); ++n) registerNode(u32(n));
        return now() - t0;
    }
    double pasteBlock(u32 nrows, u32 ncols) override
    {
        const double t0 = now();
        i64 p = 0;
        std::vector<Cell> fresh;
        for (const auto &run : usedRuns())
            for (u32 id = run.first; id < run.first + run.count && p < nrows; ++id, ++p)
                for (u32 c = 0; c < ncols; ++c) {
                    Cell cell;
                    cell.row = id;
                    cell.col = u16(C + 20 + c);
                    cell.number = double(p + c);
                    fresh.push_back(cell);
                }
        bulkAdd(fresh);
        return now() - t0;
    }
    double readRefs(u32 count) override
    {
        const double t0 = now();
        double acc = 0;
        const size_t n = nodes.size();
        const size_t step = count ? std::max<size_t>(1, n / count) : 1;
        for (size_t i = 0; i < n; i += step) {
            const Node &nd = nodes[i];
            acc += double(rows.posOf(nd.row)) + double(cols.posOf(nd.col));
            if (nd.kind == K_DIFF && nd.row > 0) acc += double(rows.posOf(nd.row - 1));
        }
        if (acc == -1) puts("");
        return now() - t0;
    }
    size_t runs() override { return rows.runs.size() + cols.runs.size(); }
    i64 graphEntries() override { return graph.entryCount(); }
    i64 graphBytes() override { return graph.memoryBytes(); }
};

// ===================================================================================== B

struct Fenwick {
    std::vector<i64> t;
    void build(const std::vector<i64> &counts)
    {
        t.assign(counts.size() + 1, 0);
        for (size_t i = 0; i < counts.size(); ++i) {
            t[i + 1] += counts[i];
            const size_t j = (i + 1) + ((i + 1) & (~(i + 1) + 1));
            if (j < t.size()) t[j] += t[i + 1];
        }
    }
    void add(size_t i, i64 d)
    {
        for (++i; i < t.size(); i += i & (~i + 1)) t[i] += d;
    }
    i64 prefix(size_t i) const   // rows in blocks before block i
    {
        i64 s = 0;
        for (; i > 0; i -= i & (~i + 1)) s += t[i];
        return s;
    }
    size_t findBlock(i64 pos, i64 *before) const   // the block holding the position
    {
        size_t at = 0;
        i64 rem = pos;
        size_t step = 1;
        while (step * 2 < t.size()) step *= 2;
        for (; step > 0; step /= 2)
            if (at + step < t.size() && t[at + step] <= rem) {
                at += step;
                rem -= t[at];
            }
        *before = pos - rem;
        return at;
    }
};

struct Block {
    size_t idx = 0;
    std::vector<u32> tok;
    std::vector<u32> start;   // start[i]..start[i+1] are the cells of row i
    std::vector<Cell> cells;
};

class EngineB : public Engine {
public:
    u32 R = 0, C = 0;
    std::vector<std::unique_ptr<Block>> blocks;
    Fenwick fw;
    struct Loc {
        Block *b = nullptr;
        u32 slot = 0;
        bool live = false;
    };
    std::vector<Loc> loc;   // by token
    u32 nextTok = 0;
    std::vector<Node> nodes;
    std::vector<double> sumOf;   // by token
    std::map<u32, u32> exception;   // DIFF node (by token) -> target token, or kNone for a reference that is gone
    DependencyGraph graph{qint64(3) << 30};
    // the families' absolute parts, as positions
    i64 sumColLo = 0, sumColHi = 0;
    i64 totRowLo = 0, totRowHi = 0, totCol = 0;
    i64 wholeCol = 0;
    i64 diffCol = 0;
    u32 totalsFirst = 0;
    struct Gone {
        u32 tok;
        std::vector<Cell> cells;
    };
    std::vector<Gone> lastGone;
    i64 lastPos = 0;
    bool lastWasInsert = false;
    std::vector<std::pair<u32, i64>> lastColCells;
    i64 lastColPos = 0;
    std::map<u32, u32> savedExceptions;

    void rebuildFenwick()
    {
        std::vector<i64> counts(blocks.size());
        for (size_t i = 0; i < blocks.size(); ++i) {
            blocks[i]->idx = i;
            counts[i] = i64(blocks[i]->tok.size());
        }
        fw.build(counts);
    }
    void build(u32 rowsN, u32 numeric) override
    {
        R = rowsN;
        C = numeric;
        loc.resize(R + 16);
        for (u32 b0 = 0; b0 < R; b0 += 64) {
            auto blk = std::make_unique<Block>();
            blk->cells.reserve(64 * (C + 2));
            blk->tok.reserve(64);
            blk->start.reserve(65);
            for (u32 r = b0; r < std::min(R, b0 + 64); ++r) {
                blk->start.push_back(u32(blk->cells.size()));
                for (u32 c = 0; c < C; ++c) {
                    Cell cell;
                    cell.number = numberAt(r, c);
                    cell.col = u16(c);
                    blk->cells.push_back(cell);
                }
                Cell s;
                s.col = u16(C);
                s.formula = int32_t(r);
                blk->cells.push_back(s);
                Cell d;
                d.col = u16(C + 1);
                d.formula = int32_t(R + r);
                blk->cells.push_back(d);
                blk->tok.push_back(r);
                loc[r] = Loc{blk.get(), u32(blk->tok.size() - 1), true};
            }
            blk->start.push_back(u32(blk->cells.size()));
            blocks.push_back(std::move(blk));
        }
        nextTok = R;
        rebuildFenwick();
        nodes.resize(size_t(R) * 2 + 7);
        sumOf.assign(R + 16, 0);
        for (u32 r = 0; r < R; ++r) {
            nodes[r] = Node{r, u16(C), K_SUMROW, 0};
            nodes[R + r] = Node{r, u16(C + 1), K_DIFF, 0};
        }
        totalsFirst = 2 * R;
        for (u32 i = 0; i < 6; ++i) nodes[totalsFirst + i] = Node{0, u16(C + 2 + i), K_TOTAL, 0};
        nodes[totalsFirst + 6] = Node{0, u16(C + 8), K_WHOLE, 0};
        sumColLo = 0;
        sumColHi = C - 1;
        totRowLo = 0;
        totRowHi = R - 1;
        totCol = C;
        wholeCol = 0;
        diffCol = C + 1;
    }
    i64 total() override { return fw.prefix(blocks.size()); }
    i64 posOfTok(u32 tok) const
    {
        const Loc &l = loc[tok];
        return fw.prefix(l.b->idx) + l.slot;
    }
    Loc select(i64 pos) const
    {
        i64 before = 0;
        const size_t b = fw.findBlock(pos, &before);
        Loc l;
        l.b = blocks[b].get();
        l.slot = u32(pos - before);
        l.live = true;
        return l;
    }
    void registerNode(u32 n)
    {
        const Node &nd = nodes[n];
        std::vector<CellArea> a;
        CellArea x;
        switch (nd.kind) {
        case K_SUMROW: x.row1 = x.row2 = u32(posOfTok(nd.row)); x.col1 = u32(sumColLo); x.col2 = u32(sumColHi); a.push_back(x); break;
        case K_DIFF: {
            const i64 p = posOfTok(nd.row);
            x.row1 = x.row2 = u32(p); x.col1 = x.col2 = u32(totCol); a.push_back(x);
            auto e = exception.find(nd.row);
            if (e != exception.end()) {
                if (e->second != kNone) { x.row1 = x.row2 = u32(posOfTok(e->second)); a.push_back(x); }
            } else if (p > 0) {
                x.row1 = x.row2 = u32(p - 1); a.push_back(x);
            }
            break;
        }
        case K_TOTAL: x.row1 = u32(totRowLo); x.row2 = u32(totRowHi); x.col1 = x.col2 = u32(totCol); a.push_back(x); break;
        case K_WHOLE: x.row1 = 0; x.row2 = kRows - 1; x.col1 = x.col2 = u32(wholeCol); a.push_back(x); break;
        case K_FILL: x.row1 = x.row2 = u32(posOfTok(nd.row)); x.col1 = x.col2 = u32(totCol); a.push_back(x); break;
        }
        graph.setPrecedents(n, a);
    }
    void registerAll() override
    {
        for (u32 n = 0; n < nodes.size(); ++n) registerNode(n);
    }
    double rowSum(u32 tok) const
    {
        const Loc &l = loc[tok];
        double sum = 0;
        for (u32 i = l.b->start[l.slot]; i < l.b->start[l.slot + 1]; ++i) {
            const Cell &c = l.b->cells[i];
            if (c.col >= sumColLo && c.col <= sumColHi) sum += c.number;
        }
        return sum;
    }
    double recalc() override
    {
        // sums, then differences (the row above, or the row an exception names), totals, the whole column
        for (auto &b : blocks)
            for (u32 s = 0; s < b->tok.size(); ++s)
                if (b->tok[s] < R) sumOf[b->tok[s]] = rowSum(b->tok[s]);
        for (size_t bi = 0; bi < blocks.size(); ++bi) {
            Block &b = *blocks[bi];
            for (u32 s = 0; s < b.tok.size(); ++s) {
                const u32 tok = b.tok[s];
                if (tok >= R) continue;
                auto e = exception.find(tok);
                u32 target;
                if (e != exception.end()) {
                    target = e->second;
                } else if (s > 0) {
                    target = b.tok[s - 1];
                } else if (bi > 0) {
                    target = blocks[bi - 1]->tok.back();
                } else {
                    target = kNone;
                }
                nodes[R + tok].value = target == kNone || target >= R ? bad() : sumOf[tok] - sumOf[target];
            }
        }
        double t = 0;
        {
            i64 p = totRowLo;
            const i64 end = std::min<i64>(totRowHi, total() - 1);
            if (p <= end) {
                Loc l = select(p);
                size_t bi = l.b->idx;
                u32 s = l.slot;
                for (; p <= end; ++bi, s = 0) {
                    Block &b = *blocks[bi];
                    for (; s < b.tok.size() && p <= end; ++s, ++p)
                        if (b.tok[s] < R) t += sumOf[b.tok[s]];
                }
            }
        }
        for (u32 i = 0; i < 6; ++i) nodes[totalsFirst + i].value = t + i;
        double whole = 0;
        for (auto &b : blocks)
            for (const Cell &c : b->cells)
                if (c.col == wholeCol) whole += c.number;
        nodes[totalsFirst + 6].value = whole;
        for (size_t n = size_t(totalsFirst) + 7; n < nodes.size(); ++n) nodes[n].value = sumOf[nodes[n].row < R ? nodes[n].row : 0] * 2;
        double sum = 0;
        i64 p = 0;
        for (auto &b : blocks)
            for (u32 s = 0; s < b->tok.size(); ++s, ++p)
                if (b->tok[s] < R) sum += (sumOf[b->tok[s]] + nodes[R + b->tok[s]].value * 3) * double(p % 1000 + 1);
        for (u32 i = 0; i < 7; ++i) sum += nodes[totalsFirst + i].value;
        return sum;
    }
    void splitIfBig(Block *b)
    {
        if (b->tok.size() <= 128) return;
        auto nb = std::make_unique<Block>();
        const size_t half = b->tok.size() / 2;
        nb->tok.assign(b->tok.begin() + long(half), b->tok.end());
        const u32 cut = b->start[half];
        nb->cells.assign(b->cells.begin() + cut, b->cells.end());
        for (size_t i = half; i < b->start.size(); ++i) nb->start.push_back(b->start[i] - cut);
        b->tok.resize(half);
        b->cells.resize(cut);
        b->start.resize(half + 1);
        for (u32 s = 0; s < nb->tok.size(); ++s) loc[nb->tok[s]] = Loc{nb.get(), s, true};
        const size_t at = b->idx + 1;
        blocks.insert(blocks.begin() + long(at), std::move(nb));
        rebuildFenwick();
    }
    void insertRowAt(i64 pos, u32 tok, std::vector<Cell> *cells)
    {
        if (pos >= total()) {
            Block *b = blocks.back().get();
            b->tok.push_back(tok);
            if (cells) b->cells.insert(b->cells.end(), cells->begin(), cells->end());
            b->start.push_back(u32(b->cells.size()));
            loc[tok] = Loc{b, u32(b->tok.size() - 1), true};
            fw.add(b->idx, 1);
            splitIfBig(b);
            return;
        }
        Loc l = select(pos);
        Block *b = l.b;
        b->tok.insert(b->tok.begin() + l.slot, tok);
        const u32 at = b->start[l.slot];
        if (cells) b->cells.insert(b->cells.begin() + at, cells->begin(), cells->end());
        b->start.insert(b->start.begin() + l.slot, at);
        const u32 n = cells ? u32(cells->size()) : 0;
        for (size_t i = l.slot + 1; i < b->start.size(); ++i) b->start[i] += n;
        for (u32 s = l.slot; s < b->tok.size(); ++s) loc[b->tok[s]] = Loc{b, s, true};
        fw.add(b->idx, 1);
        splitIfBig(b);
    }
    Gone eraseRowAt(i64 pos)
    {
        Loc l = select(pos);
        Block *b = l.b;
        Gone g;
        g.tok = b->tok[l.slot];
        const u32 s0 = b->start[l.slot], s1 = b->start[l.slot + 1];
        g.cells.assign(b->cells.begin() + s0, b->cells.begin() + s1);
        b->cells.erase(b->cells.begin() + s0, b->cells.begin() + s1);
        b->tok.erase(b->tok.begin() + l.slot);
        b->start.erase(b->start.begin() + l.slot);
        for (size_t i = l.slot; i < b->start.size(); ++i) b->start[i] -= (s1 - s0);
        for (u32 s = l.slot; s < b->tok.size(); ++s) loc[b->tok[s]].slot = s;
        loc[g.tok].live = false;
        fw.add(b->idx, -1);
        return g;
    }
    double insertRows(i64 pos, u32 n, double *wq) override
    {
        const double t0 = now();
        lastWasInsert = true;
        lastPos = pos;
        // the row below reads the row above: it is given the row it read (an exception), before the rows move
        std::vector<u32> touched;
        if (pos > 0 && pos < total()) {
            const u32 below = select(pos).b->tok[select(pos).slot], above = select(pos - 1).b->tok[select(pos - 1).slot];
            if (below < R && !exception.count(below)) {
                exception[below] = above;
                touched.push_back(R + below);
            }
        }
        for (u32 i = 0; i < n; ++i) insertRowAt(pos, nextTok++ , nullptr);
        if (loc.size() < nextTok + 16) loc.resize(nextTok + 1024);
        // the absolute parts follow
        if (pos <= totRowLo) ++totRowLo;
        if (pos <= totRowHi) totRowHi += n;
        graph.shiftRows(0, u32(pos), qint64(n), kRows);
        for (u32 i = 0; i < 7; ++i) touched.push_back(totalsFirst + i);
        for (u32 id : touched) registerNode(id);
        const double t1 = now();
        if (wq) *wq = 0;
        return t1 - t0;
    }
    double deleteRows(i64 pos, u32 n, double *wq) override
    {
        const double t0 = now();
        lastWasInsert = false;
        lastPos = pos;
        lastGone.clear();
        savedExceptions = exception;
        std::vector<u32> touched;
        // the row after the deleted rows read a deleted one; and exceptions that named deleted rows
        std::vector<u32> deadToks;
        for (u32 i = 0; i < n; ++i) {
            Loc l = select(pos + i);
            deadToks.push_back(l.b->tok[l.slot]);
        }
        const i64 after = pos + n;
        if (after < total()) {
            Loc l = select(after);
            const u32 tok = l.b->tok[l.slot];
            if (tok < R && !exception.count(tok)) {
                // reads the row above the deleted rows only if that is the one it read: it read the deleted row just before it
                exception[tok] = kNone;
                touched.push_back(R + tok);
            }
        }
        for (auto &e : exception)
            if (std::find(deadToks.begin(), deadToks.end(), e.second) != deadToks.end() && e.second != kNone) {
                e.second = kNone;
                touched.push_back(R + e.first);
            }
        for (u32 i = 0; i < n; ++i) lastGone.push_back(eraseRowAt(pos));
        if (pos + n - 1 <= totRowHi) {
            if (pos > totRowHi) {}
        }
        // absolute rows: [lo, hi] after the rows [pos, pos+n) are deleted
        auto adj = [&](i64 &lo, i64 &hi) {
            const i64 end = pos + n;
            if (hi >= pos) hi = hi < end ? pos - 1 : hi - n;
            if (lo >= pos) lo = lo < end ? pos : lo - n;
        };
        adj(totRowLo, totRowHi);
        graph.shiftRows(0, u32(pos), -qint64(n), kRows);
        for (u32 i = 0; i < 7; ++i) touched.push_back(totalsFirst + i);
        for (u32 id : touched) registerNode(id);
        const double t1 = now();
        if (wq) *wq = 0;
        return t1 - t0;
    }
    double undoLastRows() override
    {
        const double t0 = now();
        if (lastWasInsert) {
            // the rows inserted are taken out again; the exception made stays valid (it names the row it read)
            const u32 n = 1;
            for (u32 i = 0; i < n; ++i) eraseRowAt(lastPos);
            if (lastPos <= totRowLo) --totRowLo;
            if (lastPos <= totRowHi) --totRowHi;
            graph.shiftRows(0, u32(lastPos), -1, kRows);
            for (u32 i = 0; i < 7; ++i) registerNode(totalsFirst + i);
            if (lastPos > 0) {
                Loc l = select(lastPos);
                const u32 tok = l.b->tok[l.slot];
                if (tok < R) registerNode(R + tok);
            }
        } else {
            exception = savedExceptions;
            for (size_t i = lastGone.size(); i-- > 0;) insertRowAt(lastPos, lastGone[i].tok, &lastGone[i].cells);
            const i64 n = i64(lastGone.size());
            if (lastPos <= totRowLo) totRowLo += n;
            if (lastPos <= totRowHi) totRowHi += n;
            graph.shiftRows(0, u32(lastPos), n, kRows);
            // the intervals of the deleted rows were lost by the shift: their formulas are told again
            for (const Gone &g : lastGone)
                if (g.tok < R) {
                    registerNode(g.tok);
                    registerNode(R + g.tok);
                }
            for (u32 i = 0; i < 7; ++i) registerNode(totalsFirst + i);
            registerNode(R + (lastPos < total() ? select(lastPos + n).b->tok[select(lastPos + n).slot] : 0));
        }
        return now() - t0;
    }
    double insertCol(i64 pos, double *wq) override
    {
        const double t0 = now();
        lastColPos = pos;
        for (auto &b : blocks)
            for (Cell &c : b->cells)
                if (c.col >= pos) ++c.col;
        if (pos <= sumColLo) { ++sumColLo; ++sumColHi; }
        else if (pos <= sumColHi) ++sumColHi;
        if (pos <= totCol) ++totCol;
        if (pos <= wholeCol) ++wholeCol;
        // every formula that read columns at or after the edit is told again: the graph has no shift for columns
        for (u32 n = 0; n < nodes.size(); ++n) registerNode(n);
        if (wq) *wq = 0;
        return now() - t0;
    }
    double undoLastCol() override
    {
        const double t0 = now();
        for (auto &b : blocks)
            for (Cell &c : b->cells)
                if (c.col > lastColPos) --c.col;
        if (lastColPos <= sumColLo) { --sumColLo; --sumColHi; }
        else if (lastColPos <= sumColHi) --sumColHi;
        if (lastColPos <= totCol) --totCol;
        if (lastColPos <= wholeCol) --wholeCol;
        for (u32 n = 0; n < nodes.size(); ++n) registerNode(n);
        return now() - t0;
    }
    double paint() override
    {
        const double t0 = now();
        double acc = 0;
        const i64 start = total() / 2;
        for (int rep = 0; rep < 20; ++rep)
            for (i64 p = start; p < start + 50; ++p) {
                Loc l = select(p);
                const Cell *a = l.b->cells.data() + l.b->start[l.slot], *e = l.b->cells.data() + l.b->start[l.slot + 1];
                for (i64 cp = 0; cp < 30; ++cp) {
                    auto c = std::lower_bound(a, e, cp, [](const Cell &x, i64 k) { return x.col < k; });
                    if (c != e && c->col == cp) acc += c->number;
                }
            }
        if (acc == -1) puts("");
        return (now() - t0) / 20;
    }
    double fillColumn(u32 &made) override
    {
        const double t0 = now();
        made = 0;
        const size_t first = nodes.size();
        for (auto &b : blocks) {
            std::vector<Cell> cells;
            std::vector<u32> start;
            cells.reserve(b->cells.size() + b->tok.size());
            for (u32 s = 0; s < b->tok.size(); ++s) {
                start.push_back(u32(cells.size()));
                cells.insert(cells.end(), b->cells.begin() + b->start[s], b->cells.begin() + b->start[s + 1]);
                Cell c;
                c.col = u16(C + 10);
                c.formula = int32_t(nodes.size());
                cells.push_back(c);
                nodes.push_back(Node{b->tok[s], u16(C + 10), K_FILL, 0});
                ++made;
            }
            start.push_back(u32(cells.size()));
            b->cells.swap(cells);
            b->start.swap(start);
        }
        for (size_t n = first; n < nodes.size(); ++n) registerNode(u32(n));
        return now() - t0;
    }
    double pasteBlock(u32 nrows, u32 ncols) override
    {
        const double t0 = now();
        i64 p = 0;
        for (auto &b : blocks) {
            if (p >= nrows) break;
            std::vector<Cell> cells;
            std::vector<u32> start;
            for (u32 s = 0; s < b->tok.size(); ++s, ++p) {
                start.push_back(u32(cells.size()));
                cells.insert(cells.end(), b->cells.begin() + b->start[s], b->cells.begin() + b->start[s + 1]);
                if (p < nrows)
                    for (u32 c = 0; c < ncols; ++c) {
                        Cell cell;
                        cell.col = u16(C + 20 + c);
                        cell.number = double(p + c);
                        cells.push_back(cell);
                    }
            }
            start.push_back(u32(cells.size()));
            b->cells.swap(cells);
            b->start.swap(start);
        }
        return now() - t0;
    }
    double readRefs(u32 count) override
    {
        const double t0 = now();
        double acc = 0;
        const size_t n = nodes.size();
        const size_t step = count ? std::max<size_t>(1, n / count) : 1;
        for (size_t i = 0; i < n; i += step) {
            const Node &nd = nodes[i];
            acc += double(posOfTok(nd.row)) + double(nd.col);
            if (nd.kind == K_DIFF) acc += 1;
        }
        if (acc == -1) puts("");
        return now() - t0;
    }
    size_t runs() override { return blocks.size(); }
    i64 graphEntries() override { return graph.entryCount(); }
    i64 graphBytes() override { return graph.memoryBytes(); }
};

double median(std::vector<double> v)
{
    std::sort(v.begin(), v.end());
    return v.empty() ? 0 : v[v.size() / 2];
}

} // namespace

int main(int argc, char **argv)
{
    if (argc < 4) {
        puts("usage: rowid_spike A|B ROWS NUMERIC_COLS [steady_edits=1000] [check]");
        return 1;
    }
    const bool a = argv[1][0] == 'A';
    const u32 rowsN = u32(atoi(argv[2])), numeric = u32(atoi(argv[3]));
    const int steady = argc > 4 ? atoi(argv[4]) : 1000;
    const bool check = argc > 5;
    std::unique_ptr<Engine> e(a ? static_cast<Engine *>(new EngineA) : static_cast<Engine *>(new EngineB));
    const char *name = a ? "A" : "B";
    const i64 rss0 = rssMb();
    double t = now();
    e->build(rowsN, numeric);
    const double buildMs = now() - t;
    t = now();
    e->registerAll();
    const double regMs = now() - t;
    t = now();
    const double sum0 = e->recalc();
    const double recalc0 = now() - t;
    const i64 rss1 = rssMb();
    printf("%s\t%u rows x %u+2 cols\tbuild %.0f ms\tgraph registration %.0f ms\trecalc %.1f ms\tresident %lld MB (before %lld)\tgraph entries %lld, %lld MB\n", name, rowsN, numeric, buildMs, regMs, recalc0, (long long)rss1,
           (long long)rss0, (long long)e->graphEntries(), (long long)(e->graphBytes() >> 20));
    if (check) {
        // the same random edits on both designs must give the same values (run once with A, once with B, and compare the printed checksums)
        std::mt19937 rng(5);
        for (int i = 0; i < steady; ++i) {
            const i64 tot = e->total();
            const i64 p = 1 + i64(rng() % uint32_t(tot - 3));
            if (rng() % 2) e->insertRows(p, 1, nullptr);
            else e->deleteRows(p, 1, nullptr);
            if (i % 50 == 49) printf("checksum after %d edits: %.1f\n", i + 1, e->recalc());
        }
        return 0;
    }
    // the cases
    struct Case {
        const char *name;
        bool row, insert;
        i64 pos;
        u32 n;
    };
    const i64 mid = rowsN / 2;
    const u32 bigN = std::min<u32>(100000, rowsN / 10);
    const Case cases[] = {{"insert row top", true, true, 0, 1},       {"delete row top", true, false, 0, 1},   {"insert row middle", true, true, mid, 1}, {"delete row middle", true, false, mid, 1},
                          {"insert row end", true, true, rowsN - 3, 1}, {"delete 10% of rows (middle)", true, false, mid, bigN}, {"insert column near left", false, true, 2, 1}};
    printf("case\twindow ms\tundo ms\tworker question ms\n");
    for (const Case &c : cases) {
        double wq = 0, undo = 0, d = 0;
        if (c.row) {
            d = c.insert ? e->insertRows(c.pos, c.n, &wq) : e->deleteRows(c.pos, c.n, &wq);
            undo = e->undoLastRows();
        } else {
            d = e->insertCol(c.pos, &wq);
            undo = e->undoLastCol();
        }
        printf("%s\t%.4f\t%.4f\t%.1f\n", c.name, d, undo, wq);
        fflush(stdout);
    }
    // random edits, with the steady state measured after 1, after 1,000, and after the last
    std::mt19937 rng(11);
    std::vector<double> ed;
    auto steadyState = [&](int edits) {
        std::sort(ed.begin(), ed.end());
        const double r0 = now();
        e->recalc();
        const double rc = now() - r0;
        const double pt = e->paint();
        printf("after %d edits: recalculation %.1f ms (%.1f at rest), paint of 50x30 cells %.3f ms, formula-bar reads %.2f ms per 1,000, Find over every formula %.0f ms, resident %lld MB, map entries %zu\n", edits, rc, recalc0,
               pt, e->readRefs(1000), e->readRefs(0), (long long)rssMb(), e->runs());
        fflush(stdout);
    };
    std::vector<double> window;
    for (int i = 0; i < steady; ++i) {
        const i64 tot = e->total();
        const i64 p = 1 + i64(rng() % uint32_t(tot - 3));
        window.push_back(rng() % 2 ? e->insertRows(p, 1, nullptr) : e->deleteRows(p, 1, nullptr));
        if (i + 1 == 1 || i + 1 == 1000 || i + 1 == steady) {
            ed = window;
            steadyState(i + 1);
            printf("   edits so far: median %.4f ms, 99th %.4f ms, longest %.4f ms\n", median(ed), ed[size_t(double(ed.size()) * 0.99)], ed.back());
        }
    }
    {
        double wq = 0;
        const double d = e->insertRows(e->total() / 3, 1, &wq);
        printf("one more insert in the steady state: %.4f ms (worker question %.1f ms)\n", d, wq);
    }
    // fill and paste (on the steady-state sheet)
    u32 made = 0;
    const double fill = e->fillColumn(made);
    printf("fill a formula down a new column (%u formulas): %.0f ms\n", made, fill);
    printf("paste a block of %u rows by 10 values: %.0f ms\n", std::min<u32>(100000, rowsN), e->pasteBlock(std::min<u32>(100000, rowsN), 10));
    return 0;
}
