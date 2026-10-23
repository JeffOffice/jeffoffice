#pragma once
// The merged ranges of a sheet, found by the cell they hold and by the block of the screen they cross. A sheet may have a hundred thousand of them (a file may have more:
// the reader keeps a million), so they are kept in order of their first row with the most rows any of them spans, and a search looks only at those whose first row is
// near: the work of finding the merge of one cell is the logarithm of the count and the merges near it. A merge of more than kTallRows rows is kept apart in a short list
// of its own (so that one tall merge cannot make every search look at every merge), and at most kMaxTall are kept; the rest of a file's tall ones are not shown merged.
// Merges that overlap are allowed to (a file may say so): the first in order is the one a cell is in.

#include "sheet/cellref.h"

#include <QVector>

#include <vector>

namespace jp::grid {

using jp::office::sheet::CellRange;

class MergeIndex {
public:
    static constexpr quint32 kTallRows = 1000;
    static constexpr int kMaxTall = 256;
    // The most merges one search looks at; a file made to have more near one place has the merges past it left out of that search.
    static constexpr qint64 kMaxScan = 50000;

    // Takes the merges (in any order, with those of one cell or none left out); the old ones are let go.
    void assign(const QVector<CellRange> &merges);
    int count() const { return int(m_merges.size()); }
    const CellRange &range(int i) const { return m_merges[size_t(i)]; }

    // The merge the cell is in: its place (for range()), or -1.
    int at(quint32 row, quint32 col) const;
    // Calls out->push_back for the merges that cross the block (both ends in it), in order, at most `limit` of them; false if there were more.
    bool crossing(quint32 row1, quint32 col1, quint32 row2, quint32 col2, int limit, std::vector<int> *out) const;

    // Merges looked at by the searches so far (a test counts them to see that a search costs what is near it).
    qint64 examined() const { return m_examined; }

private:
    std::vector<CellRange> m_merges;     // in order of the first row, then the first column; the tall ones are after them, in the same order
    size_t m_normal = 0;                 // how many are not tall
    quint32 m_widestSpan = 0;            // the most rows a merge among them has, less one
    mutable qint64 m_examined = 0;
};

} // namespace jp::grid
