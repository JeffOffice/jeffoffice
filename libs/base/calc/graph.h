#pragma once
// The dependency graph: which formulas read which cells. A formula (a node, numbered by the caller) is told what it reads, as single cells and as blocks of cells; asked
// what reads a cell, or a block, it names the formulas. A block is kept as a block, in an index of intervals of rows for each column, and is never expanded to its cells,
// so a formula over a whole column costs one entry however many cells the column has, and an edit finds what reads its cell without going over the blocks that do not.
// Several formulas that read the same block share its entry. What a formula read before it is told again (a formula that reads where its values say tells what it read
// each time it is worked out) is dropped at once for all its uses, without looking for them. The graph holds at most a number of entries; past it a formula is not
// tracked, which the caller sees, and works that formula out every time. No GUI.

#include "calc/area.h"

#include <QtGlobal>

#include <memory>
#include <vector>

namespace jp::calc {

class DependencyGraph {
public:
    using Node = quint32;
    // `maxEntries`: the most single-cell reads and block reads (and blocks) the graph keeps.
    explicit DependencyGraph(qint64 maxEntries = 16000000);
    ~DependencyGraph();
    DependencyGraph(const DependencyGraph &) = delete;
    DependencyGraph &operator=(const DependencyGraph &) = delete;

    // Says what `node` reads: each area is a cell, or a block of one sheet or of several (a 3-D reference); whatever the node was told before is dropped. False, with the
    // node told nothing, if the graph would hold more than it may.
    bool setPrecedents(Node node, const std::vector<CellArea> &areas);
    // The node reads nothing now.
    void clearPrecedents(Node node);

    // The formulas that read the cell, or a cell of the block (one sheet), added to `out`: one may come more than once if it reads the cell in more than one way.
    void dependentsOfCell(qint32 sheet, quint32 row, quint32 col, std::vector<Node> *out) const;
    void dependentsOfArea(const CellArea &area, std::vector<Node> *out) const;

    // What the graph holds: the entries that are counted against the limit, and about how many bytes it takes.
    qint64 entryCount() const { return m_entries; }
    qint64 memoryBytes() const;
    // Lets go of what was dropped (this is done when the graph is full, and from time to time as it fills).
    void compact();

private:
    struct Impl;
    std::unique_ptr<Impl> d;
    qint64 m_entries = 0, m_max;
};

} // namespace jp::calc
