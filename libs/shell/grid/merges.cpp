#include "grid/merges.h"

#include <algorithm>
#include <climits>
#include <set>

namespace jp::grid {

namespace {

constexpr int kLevels = 21;                 // the tree's levels: 0 is the root (all the rows), 20 a single row
constexpr quint32 kLeaves = 1u << 20;       // the rows of a sheet: 1,048,576

bool before(const CellRange &a, const CellRange &b)
{
    if (a.row1 != b.row1) return a.row1 < b.row1;
    if (a.col1 != b.col1) return a.col1 < b.col1;
    if (a.row2 != b.row2) return a.row2 < b.row2;
    return a.col2 < b.col2;
}

// The number of bits a number takes (the level of a node of the tree, one less).
int bitsOf(quint32 v)
{
    int n = 0;
    for (; v; v >>= 1) ++n;
    return n;
}

int levelOf(quint32 node) { return bitsOf(node) - 1; }

} // namespace

struct MergeIndex::Data {
    // A node of the tree that has merges kept at it: where its list (by first column) and its stretches (of columns, each with the merge that comes first there) begin and how long they are.
    struct Info {
        quint32 node = 0;
        quint32 listFirst = 0, listCount = 0;
        quint32 segFirst = 0, segCount = 0;
    };
    // From `col` on (to the next stretch), the first in order of the merges kept at the node that cover the column: -1 for none.
    struct Seg {
        quint32 col = 0;
        qint32 id = -1;
    };
    std::vector<CellRange> merges;           // in order of first row, first column, last row, last column: a merge's place here is what at() gives
    std::vector<Info> infos;                 // in order of node (so level by level)
    quint32 levelStart[kLevels + 1] = {};    // where in `infos` each level begins
    std::vector<quint32> listIds;            // the merges kept at each node, in order of first column
    std::vector<quint32> listReach;          // by the same place: the farthest last column of that merge and of those before it in its node's list
    std::vector<Seg> segs;
};

int MergeIndex::count() const { return m_d ? int(m_d->merges.size()) : 0; }

const CellRange &MergeIndex::range(int i) const { return m_d->merges[size_t(i)]; }

void MergeIndex::assign(const QVector<CellRange> &given)
{
    auto d = std::make_shared<Data>();
    std::vector<CellRange> &merges = d->merges;
    merges.reserve(size_t(given.size()));
    for (const CellRange &r : given) {
        if (!r.valid || r.row1 > r.row2 || r.col1 > r.col2 || r.row2 >= kLeaves || r.col2 >= jp::office::sheet::kMaxColumns || r.cellCount() < 2) continue;
        merges.push_back(r);
    }
    std::sort(merges.begin(), merges.end(), before);
    merges.erase(std::unique(merges.begin(), merges.end()), merges.end());
    m_examined = 0;
    if (merges.empty()) {
        m_d = d;
        return;
    }
    // the nodes each merge is kept at: those whose rows it covers whole, and whose parent's it does not
    std::vector<quint64> entries;   // node in the high half, the merge's place in the low
    entries.reserve(merges.size() * 2);
    for (size_t id = 0; id < merges.size(); ++id) {
        quint32 lo = merges[id].row1 + kLeaves, hi = merges[id].row2 + kLeaves + 1;
        while (lo < hi) {
            if (lo & 1) entries.push_back((quint64(lo++) << 32) | id);
            if (hi & 1) entries.push_back((quint64(--hi) << 32) | id);
            lo >>= 1;
            hi >>= 1;
        }
    }
    std::sort(entries.begin(), entries.end());
    d->listIds.reserve(entries.size());
    d->listReach.reserve(entries.size());
    std::vector<quint32> group;
    struct Event {
        quint32 col;
        bool add;
        quint32 id;
    };
    std::vector<Event> events;
    for (size_t i = 0; i < entries.size();) {
        const quint32 node = quint32(entries[i] >> 32);
        group.clear();
        while (i < entries.size() && quint32(entries[i] >> 32) == node) group.push_back(quint32(entries[i++] & 0xFFFFFFFFu));
        Data::Info info;
        info.node = node;
        // the list, by first column
        std::sort(group.begin(), group.end(), [&](quint32 a, quint32 b) { return merges[a].col1 != merges[b].col1 ? merges[a].col1 < merges[b].col1 : a < b; });
        info.listFirst = quint32(d->listIds.size());
        info.listCount = quint32(group.size());
        quint32 reach = 0;
        for (const quint32 id : group) {
            reach = std::max(reach, merges[id].col2);
            d->listIds.push_back(id);
            d->listReach.push_back(reach);
        }
        // the stretches: the first merge in order that covers each stretch of columns
        info.segFirst = quint32(d->segs.size());
        if (group.size() == 1) {
            d->segs.push_back({merges[group[0]].col1, qint32(group[0])});
            d->segs.push_back({merges[group[0]].col2 + 1, -1});
        } else {
            events.clear();
            for (const quint32 id : group) {
                events.push_back({merges[id].col1, true, id});
                events.push_back({merges[id].col2 + 1, false, id});
            }
            std::sort(events.begin(), events.end(), [](const Event &a, const Event &b) { return a.col < b.col; });
            std::set<quint32> active;
            qint32 last = -2;
            for (size_t k = 0; k < events.size();) {
                const quint32 col = events[k].col;
                for (; k < events.size() && events[k].col == col; ++k) {
                    if (events[k].add) active.insert(events[k].id);
                    else active.erase(events[k].id);
                }
                const qint32 now = active.empty() ? -1 : qint32(*active.begin());
                if (now != last) {
                    d->segs.push_back({col, now});
                    last = now;
                }
            }
        }
        info.segCount = quint32(d->segs.size()) - info.segFirst;
        d->infos.push_back(info);
    }
    // where each level of the tree begins in the list of nodes
    size_t at = 0;
    for (int level = 0; level <= kLevels; ++level) {
        const quint32 firstNode = level < kLevels ? (1u << level) : 0xFFFFFFFFu;
        while (at < d->infos.size() && d->infos[at].node < firstNode) ++at;
        d->levelStart[level] = quint32(at);
    }
    m_d = d;
}

int MergeIndex::at(quint32 row, quint32 col) const
{
    const Data *d = m_d.get();
    if (!d || d->infos.empty() || row >= kLeaves) return -1;
    qint32 best = INT_MAX;
    // each node on the way from the row's leaf to the root, that has merges kept at it
    for (int level = kLevels - 1; level >= 0; --level) {
        const quint32 node = (kLeaves + row) >> (kLevels - 1 - level);
        size_t lo = d->levelStart[level], hi = d->levelStart[level + 1];
        if (lo == hi) continue;
        while (lo < hi) {
            ++m_examined;
            const size_t mid = (lo + hi) / 2;
            if (d->infos[mid].node < node) lo = mid + 1;
            else hi = mid;
        }
        if (lo >= d->levelStart[level + 1] || d->infos[lo].node != node) continue;
        const Data::Info &info = d->infos[lo];
        // the stretch of columns that holds the column
        size_t a = info.segFirst, b = info.segFirst + info.segCount;
        while (a < b) {
            ++m_examined;
            const size_t mid = (a + b) / 2;
            if (d->segs[mid].col <= col) a = mid + 1;
            else b = mid;
        }
        if (a == info.segFirst) continue;
        const qint32 id = d->segs[a - 1].id;
        if (id >= 0 && id < best) best = id;
    }
    return best == INT_MAX ? -1 : int(best);
}

bool MergeIndex::crossing(quint32 row1, quint32 col1, quint32 row2, quint32 col2, int limit, std::vector<int> *out) const
{
    const Data *d = m_d.get();
    if (!d || d->infos.empty() || row1 >= kLeaves) return true;
    row2 = std::min(row2, kLeaves - 1);
    if (row2 < row1 || col2 < col1) return true;
    // the nodes whose rows touch the block: those on the way to its first and last rows, and the nodes whose rows it covers whole, and those below them
    std::vector<size_t> touched;
    auto addRange = [&](int level, quint32 firstNode, quint32 lastNode) {
        size_t lo = d->levelStart[level], hi = d->levelStart[level + 1];
        while (lo < hi) {
            ++m_examined;
            const size_t mid = (lo + hi) / 2;
            if (d->infos[mid].node < firstNode) lo = mid + 1;
            else hi = mid;
        }
        for (; lo < d->levelStart[level + 1] && d->infos[lo].node <= lastNode; ++lo) touched.push_back(lo);
    };
    const quint32 first = row1 + kLeaves, last = row2 + kLeaves;
    for (int k = 0; k < kLevels; ++k) {
        const int level = kLevels - 1 - k;
        addRange(level, first >> k, first >> k);
        if ((last >> k) != (first >> k)) addRange(level, last >> k, last >> k);
    }
    quint32 lo = first, hi = last + 1;
    while (lo < hi) {
        auto below = [&](quint32 node) {
            const int level = levelOf(node);
            for (int e = 0; level + e < kLevels; ++e) addRange(level + e, node << e, ((node + 1) << e) - 1);
        };
        if (lo & 1) below(lo++);
        if (hi & 1) below(--hi);
        lo >>= 1;
        hi >>= 1;
    }
    std::sort(touched.begin(), touched.end());
    touched.erase(std::unique(touched.begin(), touched.end()), touched.end());
    // their merges that reach the block's columns
    std::vector<quint32> found;
    qint64 visits = 0;
    bool whole = true;
    for (const size_t t : touched) {
        const Data::Info &info = d->infos[t];
        const auto reachBegin = d->listReach.begin() + info.listFirst;
        const auto reachEnd = reachBegin + info.listCount;
        // (the list's merges begin at columns in order; those before the first that reaches the block's first column do not touch it)
        size_t i = size_t(std::lower_bound(reachBegin, reachEnd, col1) - d->listReach.begin());
        m_examined += bitsOf(info.listCount) + 1;   // (the steps of that search)
        for (const size_t end = size_t(info.listFirst) + info.listCount; i < end; ++i) {
            const quint32 id = d->listIds[i];
            const CellRange &m = d->merges[id];
            if (m.col1 > col2) break;
            ++m_examined;
            if (++visits > kMaxCrossingVisits) {
                whole = false;
                break;
            }
            if (m.col2 >= col1) found.push_back(id);
        }
        if (!whole) break;
    }
    std::sort(found.begin(), found.end());
    found.erase(std::unique(found.begin(), found.end()), found.end());
    for (const quint32 id : found) {
        if (int(out->size()) >= limit) return false;
        out->push_back(int(id));
    }
    return whole;
}

} // namespace jp::grid
