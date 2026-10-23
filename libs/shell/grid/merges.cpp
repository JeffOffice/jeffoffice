#include "grid/merges.h"

#include <algorithm>

namespace jp::grid {

namespace {

bool before(const CellRange &a, const CellRange &b)
{
    if (a.row1 != b.row1) return a.row1 < b.row1;
    if (a.col1 != b.col1) return a.col1 < b.col1;
    if (a.row2 != b.row2) return a.row2 < b.row2;
    return a.col2 < b.col2;
}

} // namespace

void MergeIndex::assign(const QVector<CellRange> &merges)
{
    std::vector<CellRange> normal, tall;
    for (const CellRange &r : merges) {
        if (!r.valid || r.cellCount() < 2) continue;
        if (r.row2 - r.row1 >= kTallRows) {
            if (int(tall.size()) < kMaxTall) tall.push_back(r);
        } else {
            normal.push_back(r);
        }
    }
    std::sort(normal.begin(), normal.end(), before);
    normal.erase(std::unique(normal.begin(), normal.end()), normal.end());
    std::sort(tall.begin(), tall.end(), before);
    tall.erase(std::unique(tall.begin(), tall.end()), tall.end());
    m_widestSpan = 0;
    for (const CellRange &r : normal) m_widestSpan = std::max(m_widestSpan, r.row2 - r.row1);
    m_normal = normal.size();
    m_merges = std::move(normal);
    m_merges.insert(m_merges.end(), tall.begin(), tall.end());
}

int MergeIndex::at(quint32 row, quint32 col) const
{
    // the merges that begin on a row from which one could reach this one, up to this one
    CellRange key;
    key.row1 = row > m_widestSpan ? row - m_widestSpan : 0;
    const auto first = std::lower_bound(m_merges.begin(), m_merges.begin() + std::ptrdiff_t(m_normal), key.row1, [](const CellRange &r, quint32 v) { return r.row1 < v; });
    qint64 looked = 0;
    for (auto it = first; it != m_merges.begin() + std::ptrdiff_t(m_normal) && it->row1 <= row; ++it) {
        if (++looked > kMaxScan) break;
        ++m_examined;
        if (it->row2 >= row && it->col1 <= col && col <= it->col2) return int(it - m_merges.begin());
    }
    for (size_t i = m_normal; i < m_merges.size(); ++i) {
        ++m_examined;
        const CellRange &r = m_merges[i];
        if (r.row1 <= row && row <= r.row2 && r.col1 <= col && col <= r.col2) return int(i);
    }
    return -1;
}

bool MergeIndex::crossing(quint32 row1, quint32 col1, quint32 row2, quint32 col2, int limit, std::vector<int> *out) const
{
    const quint32 start = row1 > m_widestSpan ? row1 - m_widestSpan : 0;
    const auto first = std::lower_bound(m_merges.begin(), m_merges.begin() + std::ptrdiff_t(m_normal), start, [](const CellRange &r, quint32 v) { return r.row1 < v; });
    qint64 looked = 0;
    bool whole = true;
    auto take = [&](size_t i) {
        if (int(out->size()) >= limit) {
            whole = false;
            return false;
        }
        out->push_back(int(i));
        return true;
    };
    for (auto it = first; it != m_merges.begin() + std::ptrdiff_t(m_normal) && it->row1 <= row2; ++it) {
        if (++looked > kMaxScan) {
            whole = false;
            break;
        }
        ++m_examined;
        if (it->row2 >= row1 && it->col2 >= col1 && it->col1 <= col2 && !take(size_t(it - m_merges.begin()))) return false;
    }
    for (size_t i = m_normal; i < m_merges.size(); ++i) {
        ++m_examined;
        const CellRange &r = m_merges[i];
        if (r.row1 <= row2 && r.row2 >= row1 && r.col2 >= col1 && r.col1 <= col2 && !take(i)) return false;
    }
    return whole;
}

} // namespace jp::grid
