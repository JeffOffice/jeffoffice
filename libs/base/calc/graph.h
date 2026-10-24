#pragma once
// The dependency graph: which formulas read which cells. A formula (a node, numbered by the caller) is told what it reads, as single cells and as blocks of cells; asked
// what reads a cell, or a block, it names the formulas. A block is kept as a block and is never expanded to its cells, so a formula over a whole column costs a few
// entries however many cells the column has. Each sheet has a tree over its 16,384 columns, and a block is put in the few nodes of it that together are its columns (one node
// for a single column, at most 28 for any block); a node holds the intervals of rows that were told to it, in sorted runs that are merged as they grow (so that putting one in
// costs little, and asking costs the logarithm of what the node holds, at any size). A query looks at the nodes that hold its column, and in each at the runs, and counts what
// it looked at (queryWork), so that a test can see that it did not look at much.
//
// What a formula told before is dropped when it tells again: at once for the reader (a generation for each formula says which of its entries are its latest), and for the
// memory when runs are merged and, once the dropped are as many as the entries held, all at once in one pass. A formula that tells exactly what it told before costs nothing
// and changes nothing. The graph takes at most the bytes it is given: past them a formula is not tracked, which the caller sees, and works that formula out every time; it
// says no at once, and does not go over what it holds again for each formula it refuses. Queries may be made from one thread at a time. No GUI.

#include "calc/area.h"

#include <QtGlobal>

#include <atomic>
#include <memory>
#include <vector>

namespace jp::calc {

class DependencyGraph {
public:
    using Node = quint32;
    // What the graph may take unless it is told otherwise (the workbook's cells and the formulas' values take theirs besides).
    static constexpr qint64 kDefaultBytes = qint64(512) << 20;
    // `maxBytes`: the most memory the graph may take, as memoryBytes() counts it.
    explicit DependencyGraph(qint64 maxBytes = kDefaultBytes);
    ~DependencyGraph();
    DependencyGraph(const DependencyGraph &) = delete;
    DependencyGraph &operator=(const DependencyGraph &) = delete;

    // Says what `node` reads: each area is a cell, or a block of one sheet or of several (a 3-D reference); whatever the node was told before is dropped. False, with the
    // node told nothing, if the graph would take more bytes than it may.
    bool setPrecedents(Node node, const std::vector<CellArea> &areas);
    // The node reads nothing now.
    void clearPrecedents(Node node);
    // Rows were inserted into `sheet` at `first` (`delta` more than 0: that many) or deleted from it (less than 0: that many): everything every formula reads on that sheet is where its
    // cells are now (a block that has a row put in it grows, one that has rows deleted loses them, what was only in the rows deleted is read no more, and one to the end of the sheet,
    // `limit` rows, still is). What a formula told is as it was told and the formulas are not asked: the cells that move are those that every formula that reads them is rewritten
    // for, and a formula that reads several sheets (a block of more than one) is not rewritten for a change of one of them: the caller tells such a formula again.
    void shiftRows(qint32 sheet, quint32 first, qint64 delta, quint32 limit);

    // The formulas that read the cell, or a cell of the block (one sheet), added to `out`: one may come more than once if it reads the cell in more than one way.
    // With `stop`, the flag is looked at as the answer is made (every few thousand formulas found: a cell that two hundred thousand formulas read is one query of that many): if it is set the query
    // ends there, false is returned, and `out` holds some of the answer. True when all of it was found.
    bool dependentsOfCell(qint32 sheet, quint32 row, quint32 col, std::vector<Node> *out, const std::atomic<bool> *stop = nullptr) const;
    bool dependentsOfArea(const CellArea &area, std::vector<Node> *out, const std::atomic<bool> *stop = nullptr) const;

    // What the graph holds: the entries the formulas' latest reads take (a block is one for each node of the tree it is put in), and the bytes it takes.
    qint64 entryCount() const;
    qint64 memoryBytes() const;
    // Lets go of what was dropped, all at once (this is done from time to time as the dropped come to as many as are held, and when the graph is nearly full of them).
    void compact();

    // Counted, for the tests: the items and nodes the queries looked at, the times the graph was compacted, and the items moved in sorting, merging, and compacting.
    qint64 queryWork() const;
    qint64 compactions() const;
    qint64 buildWork() const;

private:
    struct Impl;
    std::unique_ptr<Impl> d;
    qint64 m_max;
};

} // namespace jp::calc
