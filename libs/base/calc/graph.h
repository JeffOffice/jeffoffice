#pragma once
// The dependency graph: which formulas read which cells. A formula (a node, numbered by the caller) is told what it reads, as single cells and as blocks of cells; asked
// what reads a cell, or a block, it names the formulas. A block is kept as a block and is never expanded to its cells, so a formula over a whole column costs a few
// entries however many cells the column has. The rows and columns the graph is told and asked are the ids of the sheet's lines (calc/idframe.h, docs/jeffcrunch-rowids.md, 4.6),
// not their places: an edit of whole rows or columns changes no id, so it changes nothing in the graph. Each sheet has a tree over its 65,536 column ids (the 16,384 of the grid and the
// ids that inserted columns take), and a block is put in the few nodes of it that together are its columns (one node for a single column, at most 32 for any block); a node holds the intervals of rows that were told to it, in sorted runs that are merged as they grow (so that putting one in
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
#include <functional>
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
    // The formulas that read the cell, or a cell of the block (one sheet), added to `out`: one may come more than once if it reads the cell in more than one way.
    // With `stop`, the flag is looked at as the answer is made (every few thousand formulas found: a cell that two hundred thousand formulas read is one query of that many): if it is set the query
    // ends there, false is returned, and `out` holds some of the answer. True when all of it was found.
    bool dependentsOfCell(qint32 sheet, quint32 row, quint32 col, std::vector<Node> *out, const std::atomic<bool> *stop = nullptr) const;
    bool dependentsOfArea(const CellArea &area, std::vector<Node> *out, const std::atomic<bool> *stop = nullptr) const;
    // The same, one formula at a time to `visit`, which says false to end the search (false is returned then): to ask whether there is any without finding all.
    bool forEachDependent(const CellArea &area, const std::function<bool(Node)> &visit) const;
    // The formulas that read a block of rows of `sheet` (any columns) that holds both of two rows: begins at or before `before` and ends at or after `after`. A gap between two rows (rows
    // inserted there) is inside the blocks this gives, and not inside the others, whichever columns they are in. Nothing for `before` at or past `after`.
    bool dependentsHoldingRows(qint32 sheet, quint32 before, quint32 after, std::vector<Node> *out, const std::atomic<bool> *stop = nullptr) const;
    bool forEachHoldingRows(qint32 sheet, quint32 before, quint32 after, const std::function<bool(Node)> &visit) const;

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
