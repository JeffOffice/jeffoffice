#include "calc/graph.h"

#include <algorithm>
#include <limits>
#include <unordered_map>

namespace jp::calc {

namespace {

constexpr int kNarrowColumns = 8;   // a block of this many columns or fewer is in the index of each of its columns; a wider one in one for all
constexpr size_t kMaxSheetsOfABlock = 1024;

quint64 cellKey(quint32 row, quint32 col) { return (quint64(row) << 14) | quint64(col); }

// A set of intervals of rows, each with a number, asked which of them hold a row or meet a stretch of rows. A tree built for the set as it was (a node holds the intervals
// that meet its centre, in two orders; the rest are in its left or right), and the intervals added since, searched one by one until there are enough of them to be built in.
class RowIntervals {
public:
    struct Item {
        quint32 lo, hi, id;
    };

    void add(quint32 lo, quint32 hi, quint32 id)
    {
        m_loose.push_back({lo, hi, id});
        if (m_loose.size() > std::max<size_t>(64, m_built / 8)) rebuild();
    }
    size_t size() const { return m_built + m_loose.size(); }
    void clear()
    {
        m_nodes.clear();
        m_byLo.clear();
        m_byHi.clear();
        m_loose.clear();
        m_built = 0;
    }
    // Calls f(id) for each interval that meets [lo, hi].
    template <class F>
    void overlap(quint32 lo, quint32 hi, F f) const
    {
        for (const Item &it : m_loose)
            if (it.lo <= hi && it.hi >= lo) f(it.id);
        if (m_nodes.empty()) return;
        size_t stack[64];
        int top = 0;
        stack[top++] = 0;
        while (top > 0) {
            const Node &n = m_nodes[stack[--top]];
            if (hi < n.center) {
                for (quint32 i = n.begin; i < n.begin + n.count && m_byLo[i].lo <= hi; ++i) f(m_byLo[i].id);
                if (n.left >= 0 && top < 62) stack[top++] = size_t(n.left);
            } else if (lo > n.center) {
                for (quint32 i = n.begin; i < n.begin + n.count && m_byHi[i].hi >= lo; ++i) f(m_byHi[i].id);
                if (n.right >= 0 && top < 62) stack[top++] = size_t(n.right);
            } else {
                for (quint32 i = n.begin; i < n.begin + n.count; ++i) f(m_byLo[i].id);
                if (n.left >= 0 && top < 62) stack[top++] = size_t(n.left);
                if (n.right >= 0 && top < 62) stack[top++] = size_t(n.right);
            }
        }
    }
    void rebuild()
    {
        std::vector<Item> all;
        all.reserve(m_byLo.size() + m_loose.size());
        // every interval is in exactly one node's lists: take them from the lists by lo
        for (const Item &it : m_byLo) all.push_back(it);
        for (const Item &it : m_loose) all.push_back(it);
        m_loose.clear();
        build(std::move(all));
    }
    qint64 memoryBytes() const { return qint64(m_nodes.capacity() * sizeof(Node) + (m_byLo.capacity() + m_byHi.capacity() + m_loose.capacity()) * sizeof(Item)); }

private:
    struct Node {
        quint32 center = 0;
        quint32 begin = 0, count = 0;
        qint32 left = -1, right = -1;
    };
    void build(std::vector<Item> items)
    {
        m_nodes.clear();
        m_byLo.clear();
        m_byHi.clear();
        m_built = items.size();
        if (items.empty()) return;
        // an explicit stack of (node, items) to build, so that no depth of tree is a depth of recursion
        struct Job {
            qint32 node;
            std::vector<Item> items;
        };
        std::vector<Job> jobs;
        m_nodes.emplace_back();
        jobs.push_back({0, std::move(items)});
        while (!jobs.empty()) {
            Job job = std::move(jobs.back());
            jobs.pop_back();
            std::vector<quint32> ends;
            ends.reserve(job.items.size() * 2);
            for (const Item &it : job.items) {
                ends.push_back(it.lo);
                ends.push_back(it.hi);
            }
            std::nth_element(ends.begin(), ends.begin() + ptrdiff_t(ends.size() / 2), ends.end());
            const quint32 center = ends[ends.size() / 2];
            std::vector<Item> left, right, here;
            for (const Item &it : job.items) {
                if (it.hi < center) left.push_back(it);
                else if (it.lo > center) right.push_back(it);
                else here.push_back(it);
            }
            Node &n = m_nodes[size_t(job.node)];
            n.center = center;
            n.begin = quint32(m_byLo.size());
            n.count = quint32(here.size());
            std::vector<Item> byHi = here;
            std::sort(here.begin(), here.end(), [](const Item &a, const Item &b) { return a.lo < b.lo; });
            std::sort(byHi.begin(), byHi.end(), [](const Item &a, const Item &b) { return a.hi > b.hi; });
            m_byLo.insert(m_byLo.end(), here.begin(), here.end());
            m_byHi.insert(m_byHi.end(), byHi.begin(), byHi.end());
            if (!left.empty()) {
                const qint32 l = qint32(m_nodes.size());
                m_nodes.emplace_back();
                m_nodes[size_t(job.node)].left = l;
                jobs.push_back({l, std::move(left)});
            }
            if (!right.empty()) {
                const qint32 r = qint32(m_nodes.size());
                m_nodes.emplace_back();
                m_nodes[size_t(job.node)].right = r;
                jobs.push_back({r, std::move(right)});
            }
        }
    }
    std::vector<Node> m_nodes;
    std::vector<Item> m_byLo, m_byHi;
    std::vector<Item> m_loose;
    size_t m_built = 0;
};

struct SingleEdge {
    quint64 key;
    quint32 node;
    quint32 gen;
};

struct Link {
    quint32 node;
    quint32 gen;
};

struct RangeEntry {
    quint32 row1, row2;
    quint16 col1, col2;
    std::vector<Link> links;
};

struct RangeKey {
    quint32 row1, row2;
    quint32 cols;   // col1 << 16 | col2
    bool operator==(const RangeKey &o) const { return row1 == o.row1 && row2 == o.row2 && cols == o.cols; }
};
struct RangeKeyHash {
    size_t operator()(const RangeKey &k) const
    {
        quint64 h = k.row1 * 0x9E3779B97F4A7C15ull;
        h ^= (k.row2 + 0x7F4A7C15ull) * 0xC2B2AE3D27D4EB4Full + (h << 6) + (h >> 2);
        h ^= (k.cols + 0x165667B1ull) * 0x9E3779B97F4A7C15ull + (h << 6) + (h >> 2);
        return size_t(h);
    }
};

struct SheetIndex {
    std::vector<SingleEdge> sorted;   // by key
    std::vector<SingleEdge> loose;    // told since, in no order
    std::vector<RangeEntry> ranges;
    std::unordered_map<RangeKey, quint32, RangeKeyHash> rangeIds;
    std::unordered_map<quint16, RowIntervals> byColumn;
    RowIntervals wide;
};

} // namespace

struct DependencyGraph::Impl {
    std::vector<SheetIndex> sheets;
    std::vector<quint32> generation;   // by node: the generation of what it told last; what it told before has another
};

DependencyGraph::DependencyGraph(qint64 maxEntries) : d(new Impl), m_max(maxEntries) {}
DependencyGraph::~DependencyGraph() = default;

bool DependencyGraph::setPrecedents(Node node, const std::vector<CellArea> &areas)
{
    if (d->generation.size() <= node) d->generation.resize(size_t(node) + 1, 0);
    ++d->generation[node];
    if (d->generation[node] == 0) d->generation[node] = 1;
    qint64 wanted = 0;
    for (const CellArea &a : areas) wanted += 1 + (a.single() ? 0 : 1) + qint64(std::min<size_t>(kMaxSheetsOfABlock, size_t(std::max<qint32>(0, a.sheet2 - a.sheet1))));
    if (m_entries + wanted > m_max) compact();
    if (m_entries + wanted > m_max) {
        ++d->generation[node];   // nothing told: whatever it told before is dropped too
        return false;
    }
    const quint32 gen = d->generation[node];
    for (const CellArea &a : areas) {
        if (a.sheet1 < 0 || a.sheet2 < a.sheet1) continue;
        const size_t last = std::min<size_t>(size_t(a.sheet2), size_t(a.sheet1) + kMaxSheetsOfABlock - 1);
        for (size_t s = size_t(a.sheet1); s <= last; ++s) {
            if (d->sheets.size() <= s) d->sheets.resize(s + 1);
            SheetIndex &sh = d->sheets[s];
            if (a.row1 == a.row2 && a.col1 == a.col2) {
                sh.loose.push_back({cellKey(a.row1, a.col1), node, gen});
                ++m_entries;
                if (sh.loose.size() > std::max<size_t>(2048, sh.sorted.size() / 4)) {
                    std::sort(sh.loose.begin(), sh.loose.end(), [](const SingleEdge &x, const SingleEdge &y) { return x.key < y.key; });
                    std::vector<SingleEdge> merged;
                    merged.reserve(sh.sorted.size() + sh.loose.size());
                    std::merge(sh.sorted.begin(), sh.sorted.end(), sh.loose.begin(), sh.loose.end(), std::back_inserter(merged), [](const SingleEdge &x, const SingleEdge &y) { return x.key < y.key; });
                    sh.sorted.swap(merged);
                    sh.loose.clear();
                }
                continue;
            }
            const RangeKey key{a.row1, a.row2, (quint32(a.col1) << 16) | quint32(a.col2)};
            auto it = sh.rangeIds.find(key);
            quint32 id;
            if (it == sh.rangeIds.end()) {
                id = quint32(sh.ranges.size());
                sh.ranges.push_back({a.row1, a.row2, quint16(a.col1), quint16(a.col2), {}});
                sh.rangeIds.emplace(key, id);
                ++m_entries;
                if (a.col2 - a.col1 < quint32(kNarrowColumns)) {
                    for (quint32 c = a.col1; c <= a.col2; ++c) sh.byColumn[quint16(c)].add(a.row1, a.row2, id);
                } else {
                    sh.wide.add(a.row1, a.row2, id);
                }
            } else {
                id = it->second;
            }
            std::vector<Link> &links = sh.ranges[id].links;
            if (links.empty() || links.back().node != node || links.back().gen != gen) {
                links.push_back({node, gen});
                ++m_entries;
            }
        }
    }
    return true;
}

void DependencyGraph::clearPrecedents(Node node)
{
    if (d->generation.size() <= node) return;
    ++d->generation[node];
    if (d->generation[node] == 0) d->generation[node] = 1;
}

void DependencyGraph::dependentsOfCell(qint32 sheet, quint32 row, quint32 col, std::vector<Node> *out) const
{
    if (sheet < 0 || size_t(sheet) >= d->sheets.size() || col >= 16384) return;
    const SheetIndex &sh = d->sheets[size_t(sheet)];
    auto live = [&](quint32 node, quint32 gen) { return node < d->generation.size() && d->generation[node] == gen; };
    const quint64 key = cellKey(row, col);
    const auto range = std::equal_range(sh.sorted.begin(), sh.sorted.end(), SingleEdge{key, 0, 0}, [](const SingleEdge &a, const SingleEdge &b) { return a.key < b.key; });
    for (auto it = range.first; it != range.second; ++it)
        if (live(it->node, it->gen)) out->push_back(it->node);
    for (const SingleEdge &e : sh.loose)
        if (e.key == key && live(e.node, e.gen)) out->push_back(e.node);
    auto report = [&](quint32 id) {
        for (const Link &l : sh.ranges[id].links)
            if (live(l.node, l.gen)) out->push_back(l.node);
    };
    const auto column = sh.byColumn.find(quint16(col));
    if (column != sh.byColumn.end()) column->second.overlap(row, row, report);
    sh.wide.overlap(row, row, [&](quint32 id) {
        const RangeEntry &e = sh.ranges[id];
        if (e.col1 <= col && col <= e.col2) report(id);
    });
}

void DependencyGraph::dependentsOfArea(const CellArea &area, std::vector<Node> *out) const
{
    for (qint32 sheet = std::max<qint32>(0, area.sheet1); sheet <= area.sheet2 && size_t(sheet) < d->sheets.size(); ++sheet) {
        const SheetIndex &sh = d->sheets[size_t(sheet)];
        auto live = [&](quint32 node, quint32 gen) { return node < d->generation.size() && d->generation[node] == gen; };
        const quint32 col2 = std::min<quint32>(area.col2, 16383);
        // single cells: the keys from the first cell to the last are the rows between, with the columns outside the block among them
        const quint64 lo = cellKey(area.row1, area.col1), hi = cellKey(area.row2, col2);
        auto inside = [&](quint64 key) {
            const quint32 c = quint32(key & 16383);
            return c >= area.col1 && c <= col2;
        };
        for (auto it = std::lower_bound(sh.sorted.begin(), sh.sorted.end(), SingleEdge{lo, 0, 0}, [](const SingleEdge &a, const SingleEdge &b) { return a.key < b.key; });
             it != sh.sorted.end() && it->key <= hi; ++it)
            if (inside(it->key) && live(it->node, it->gen)) out->push_back(it->node);
        for (const SingleEdge &e : sh.loose)
            if (e.key >= lo && e.key <= hi && inside(e.key) && live(e.node, e.gen)) out->push_back(e.node);
        // blocks that meet it
        std::vector<quint32> ids;
        if (col2 - area.col1 < 64) {
            for (quint32 c = area.col1; c <= col2; ++c) {
                const auto column = sh.byColumn.find(quint16(c));
                if (column != sh.byColumn.end()) column->second.overlap(area.row1, area.row2, [&](quint32 id) { ids.push_back(id); });
            }
        } else {
            for (const auto &column : sh.byColumn)
                if (column.first >= area.col1 && column.first <= col2) column.second.overlap(area.row1, area.row2, [&](quint32 id) { ids.push_back(id); });
        }
        sh.wide.overlap(area.row1, area.row2, [&](quint32 id) {
            const RangeEntry &e = sh.ranges[id];
            if (e.col1 <= col2 && e.col2 >= area.col1) ids.push_back(id);
        });
        std::sort(ids.begin(), ids.end());
        ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
        for (const quint32 id : ids)
            for (const Link &l : sh.ranges[id].links)
                if (live(l.node, l.gen)) out->push_back(l.node);
    }
}

void DependencyGraph::compact()
{
    qint64 held = 0;
    auto live = [&](quint32 node, quint32 gen) { return node < d->generation.size() && d->generation[node] == gen; };
    for (SheetIndex &sh : d->sheets) {
        auto alive = [&](const SingleEdge &e) { return live(e.node, e.gen); };
        sh.sorted.erase(std::remove_if(sh.sorted.begin(), sh.sorted.end(), [&](const SingleEdge &e) { return !alive(e); }), sh.sorted.end());
        sh.loose.erase(std::remove_if(sh.loose.begin(), sh.loose.end(), [&](const SingleEdge &e) { return !alive(e); }), sh.loose.end());
        sh.sorted.shrink_to_fit();
        held += qint64(sh.sorted.size() + sh.loose.size());
        // the blocks no formula reads now are dropped, and the index of the others made again
        std::vector<RangeEntry> kept;
        for (RangeEntry &e : sh.ranges) {
            e.links.erase(std::remove_if(e.links.begin(), e.links.end(), [&](const Link &l) { return !live(l.node, l.gen); }), e.links.end());
            if (e.links.empty()) continue;
            e.links.shrink_to_fit();
            kept.push_back(std::move(e));
        }
        sh.ranges.swap(kept);
        sh.rangeIds.clear();
        sh.byColumn.clear();
        sh.wide.clear();
        for (quint32 id = 0; id < sh.ranges.size(); ++id) {
            const RangeEntry &e = sh.ranges[id];
            sh.rangeIds.emplace(RangeKey{e.row1, e.row2, (quint32(e.col1) << 16) | quint32(e.col2)}, id);
            if (e.col2 - e.col1 < kNarrowColumns) {
                for (quint32 c = e.col1; c <= e.col2; ++c) sh.byColumn[quint16(c)].add(e.row1, e.row2, id);
            } else {
                sh.wide.add(e.row1, e.row2, id);
            }
            held += 1 + qint64(e.links.size());
        }
        for (auto &column : sh.byColumn) column.second.rebuild();
        sh.wide.rebuild();
    }
    m_entries = held;
}

qint64 DependencyGraph::memoryBytes() const
{
    qint64 bytes = qint64(d->generation.capacity() * sizeof(quint32));
    for (const SheetIndex &sh : d->sheets) {
        bytes += qint64((sh.sorted.capacity() + sh.loose.capacity()) * sizeof(SingleEdge));
        for (const RangeEntry &e : sh.ranges) bytes += qint64(sizeof(RangeEntry) + e.links.capacity() * sizeof(Link) + 48);
        for (const auto &column : sh.byColumn) bytes += column.second.memoryBytes() + 48;
        bytes += sh.wide.memoryBytes();
    }
    return bytes;
}

} // namespace jp::calc
