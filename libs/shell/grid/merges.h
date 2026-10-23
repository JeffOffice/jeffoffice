#pragma once
// The merged ranges of a sheet, found by the cell they hold and by the block of the screen they cross. A sheet may have a million of them (a file may have more: the reader keeps a
// million), and no search may cost more for having them: the work of finding the merge of one cell is the same few steps for a hundred merges as for a million, and none is ever left
// out of a search for want of a limit.
//
// The index is a tree over the rows of the sheet (the rows are halved, and halved again, to single rows). A merge is kept at the few nodes of the tree whose rows it covers whole, so
// the merges that hold a row are those kept at the nodes on the way from the root to its leaf. At each node the merges are kept two ways: as a list in order of their first column
// (with the farthest column any of them so far reaches), for the merges that cross a block; and as the stretches of columns each of which has one merge that comes first in order of
// all that cover it, for the merge of a cell. A search of a cell is then a look at each of the twenty-one nodes on its row's way, in a list of the nodes that have merges; one of a block
// looks at the merges kept at the nodes whose rows touch the block, at most kMaxCrossingVisits, and says when it stopped short.
//
// Merges that overlap are allowed to (a file may say so): the first in order (the first row, then the first column) is the one a cell is in. A copy of the index is the same merges,
// shared, until the copy is given others.

#include "sheet/cellref.h"

#include <QVector>

#include <memory>
#include <vector>

namespace jp::grid {

using jp::office::sheet::CellRange;

class MergeIndex {
public:
    // The most merges a search of a block looks at (it says when there were more).
    static constexpr qint64 kMaxCrossingVisits = 400000;

    // Takes the merges (in any order, with those of one cell or none left out); the old ones are let go.
    void assign(const QVector<CellRange> &merges);
    int count() const;
    const CellRange &range(int i) const;

    // The merge the cell is in: its place (for range()), or -1.
    int at(quint32 row, quint32 col) const;
    // Calls out->push_back for the merges that cross the block (both ends in it), in order, at most `limit` of them; false if there were more, or if the search stopped short.
    bool crossing(quint32 row1, quint32 col1, quint32 row2, quint32 col2, int limit, std::vector<int> *out) const;

    // True if the two hold the same merges (a copy shares them until one is assigned others).
    bool sharesStorageWith(const MergeIndex &other) const { return m_d && m_d == other.m_d; }
    // Steps the searches have taken so far (a test counts them to see that a search costs the same for any number of merges).
    qint64 examined() const { return m_examined; }

private:
    struct Data;
    std::shared_ptr<const Data> m_d;
    mutable qint64 m_examined = 0;
};

} // namespace jp::grid
