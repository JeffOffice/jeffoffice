#pragma once
// What the dependency graph is told and asked, in ids (docs/jeffcrunch-rowids.md, 4.6). The graph holds the ids of rows and columns, which an insert or a delete does not change; the
// formulas still read by place, so a block of places is turned into ids on the way in, and a change of cells on the way in as well.
//
// Two rules make the intervals right with no rewriting. The ids of the original lines (those below `firstNew`, the file's own) are in the order of their places however many lines are
// inserted among them, so an interval between two original ids covers exactly the original lines between them. A line that was inserted has an id that is no use as an end, so:
//   - a reader whose block ends in an inserted line is told as the original line at that end (the one before it for the first end, the one after it for the last end): the interval is a
//     little too wide and never too narrow; a block that begins at the first line of the sheet begins at id 0, and one that ends at the last line ends at the largest id, so that lines
//     put before them or after them are inside, as a whole column is;
//   - a change in an inserted line is asked as the original lines on either side of it.
// Both err in one direction only: a formula is worked out when it need not be, never the other way.
//
// No GUI.

#include "calc/area.h"
#include "calc/linemap.h"

namespace jp::calc {

// The largest id a row or a column can have (24 and 16 bits in a cell), and the last place of a row and of a column of the grid.
constexpr quint32 kMaxRowId = (1u << 24) - 1;
constexpr quint32 kMaxColId = (1u << 16) - 1;

// The ids [lo, hi] the places [first, last] of one kind of line are told or asked as. `lastPlace` is the sheet's last place and `maxId` the largest id of the kind. `reading`: a formula's block
// (the first place of the sheet begins at id 0, the last ends at the largest id); not reading: a change of cells (the ends are the lines' own ids, or the originals next to them).
struct IdSpan {
    quint32 lo, hi;
};
IdSpan idSpan(const LineMapCore &map, quint32 first, quint32 last, quint32 lastPlace, quint32 maxId, bool reading);

// The ids [lo, hi] a reference held by the ids at its two ends is told as (`startId` is the end that was written first). An original line (below `firstNew`) is its own id, whether it is alive or was
// deleted: the interval of a formula whose end died stays as wide as it was, so that bringing the line back finds the formula. An inserted line is the original line next to it (the one before it for the
// start, after it for the end; a dead one, next to the line it resolves to). False for an id past `maxId`.
bool idSpanOfEnds(const LineMapCore &map, qint64 startId, qint64 endId, quint32 maxId, IdSpan *out);

// A block of one sheet in places, as the ids it is told or asked as (the sheets of the area are not looked at: the caller says which sheet's order these are).
CellArea idArea(const RowMap &rows, const ColMap &cols, const CellArea &places, bool reading);

// The original lines either side of a gap: the last original line at a place up to `before` (kNone: none) and the first from `after` on. A gap with no original line on one side is
// inside no block (a block needs lines on both sides of it).
struct IdGap {
    quint32 before = LineMapCore::kNone, after = LineMapCore::kNone;
    bool holdable() const { return before != LineMapCore::kNone && after != LineMapCore::kNone; }
};
IdGap idGap(const LineMapCore &map, quint32 afterPlace);   // the gap just before the line at `afterPlace` (lines inserted there have been put in already)

} // namespace jp::calc
