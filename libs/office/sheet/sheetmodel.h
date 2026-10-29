#pragma once
// What a worksheet holds, as it was read from a file: its cells in blocks of
// rows (only the cells the file has; a sheet of ten cells is ten cells), the
// text of its strings and formulas, and what the file says about its shape.
// The shared strings of a workbook are one list that every sheet refers to.

#include "calc/linemap.h"
#include "sheet/cellref.h"
#include "sheet/links.h"
#include "sheet/pagesetup.h"
#include "sheet/protection.h"
#include "sheet/styles.h"

#include <QHash>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>

#include <algorithm>
#include <atomic>
#include <functional>
#include <limits>
#include <map>
#include <vector>

namespace jp::office::sheet {

class FrameSheets;

enum class CellType : quint8 {
    Empty,           // the cell is there (it has a style, or a formula with no result stored) but holds no value
    Number,          // number
    SharedString,    // number holds the place of its text in the workbook's shared strings
    InlineString,    // number holds the place of its text in the sheet's texts
    FormulaString,   // the text a formula gave, as in InlineString
    Bool,            // number is 0 (false) or 1 (true)
    Error,           // number is a CellError
    Date,            // number is the day count (a fraction of a day for a time) in the workbook's date system
};

// The errors a cell can hold; the file spells them "#DIV/0!" and so on.
enum class CellError : quint8 { Null, Div0, Value, Ref, Name, Num, NA, GettingData, Spill, Calc, Field, Blocked, Connect, External, Unknown };
QString errorText(CellError error);
// The error a text spells; Unknown (and false in `known`) for any other.
CellError errorFromText(QStringView text, bool *known = nullptr);

// What more a cell has: the bits of Cell::flags.
enum CellFlag : quint8 {
    HasFormula = 1,
    ArrayFormula = 2,    // the cell holds the formula of an array (its range is in the formula)
    SharedFormula = 4,   // the formula is one a group of cells share, as written for the first of them
    DataTable = 8,       // the formula is a what-if data table
    ArrayMember = 16,    // a cell of the range of an array formula that is not its first cell: its value is the formula's (set by the calculation; `formula` then names the array's formula)
    Tainted = 32,        // a value the calculation could not check (a cell of a data table, or of an array whose formula it kept apart)
};

// One cell: 24 bytes. Its value is in number (and, for text, the pool it points
// to), and its formula, if it has one, in the sheet's formulas. The ids of its
// row and column are the store's (they are the cell's place until a row or column
// is inserted or deleted before it, and then they stay as they were), and only
// CellStore reads them. A cell is put in at a place (CellStore::add, set) and is
// given its place by the store's walks, never by asking it.
struct Cell {
    double number = 0;
    quint32 rowId = 0;
    quint16 colId = 0;
    CellType type = CellType::Empty;
    quint8 flags = 0;
    quint32 style = 0;       // the place of its format in the workbook's formats, as the file gives it
    qint32 formula = -1;     // the place in Sheet::formulas; -1 for none

    bool hasFormula() const { return flags & HasFormula; }
    // A formula cell has a result when the file stored one.
    bool hasResult() const { return type != CellType::Empty; }
    qint32 textIndex() const { return qint32(number); }
};
static_assert(sizeof(Cell) == 24, "a cell is 24 bytes");

// A cell with the place it is at (what the store hands out where a cell is taken from it or moved).
struct PlacedCell {
    quint32 row = 0;
    quint32 col = 0;
    Cell cell;
};

// A short list that lives on the stack up to N entries and on the heap past that (the runs of a walk are few, and a walk is made very often).
template <class T, size_t N>
class SmallList {
public:
    SmallList() = default;
    SmallList(const SmallList &) = delete;
    SmallList &operator=(const SmallList &) = delete;
    size_t size() const { return m_n; }
    bool empty() const { return m_n == 0; }
    T &operator[](size_t i) { return data()[i]; }
    const T &operator[](size_t i) const { return data()[i]; }
    T *begin() { return data(); }
    T *end() { return data() + m_n; }
    const T *begin() const { return data(); }
    const T *end() const { return data() + m_n; }
    void push_back(const T &v)
    {
        if (!m_onHeap && m_n == N) {
            m_heap.assign(m_inline, m_inline + N);
            m_onHeap = true;
        }
        if (m_onHeap) m_heap.push_back(v);
        else m_inline[m_n] = v;
        ++m_n;
    }
    void resize(size_t n)
    {
        if (!m_onHeap && n > N) {
            m_heap.assign(m_inline, m_inline + m_n);
            m_onHeap = true;
        }
        if (m_onHeap) m_heap.resize(n);
        m_n = n;
    }

private:
    T *data() { return m_onHeap ? m_heap.data() : m_inline; }
    const T *data() const { return m_onHeap ? m_heap.data() : m_inline; }
    T m_inline[N];
    std::vector<T> m_heap;
    size_t m_n = 0;
    bool m_onHeap = false;
};

// The cells of one sheet, kept in blocks of rows: a block holds the cells of
// kBlockRows row ids, in order of row id and then column id, and only blocks
// that have a cell exist. Cells may be added in any order; finish() puts each
// block in order.
//
// A cell is held by the id of its row and of its column, never by its place: the store has the
// order of the sheet's rows and of its columns (`rows()`, `cols()`: calc/linemap.h), and every call
// that names a cell takes the place and turns it into ids on the way in; every walk gives the place
// beside the cell, going along the live runs of the order. Rows and columns inserted or deleted are
// operations on that order (insertLines and the others below), and no cell moves. A row or column
// that is deleted leaves the order but keeps its cells (they come back with Undo) until the step that
// could bring them back is let go of (forgetLines); walks never reach them, and cellCount() does not
// count them. A sheet whose order has grown too many runs or used too many ids is put back to one run
// (rebaseLines), which renumbers the cells and lets go of the dead ones.
class CellStore {
public:
    static constexpr quint32 kBlockRows = 64;
    static constexpr quint32 kNoLine = jp::calc::LineMapCore::kNone;

    // Ids from `first`, `count` of them, at the places from `base`.
    struct Span {
        quint32 first, count, base;
    };

    CellStore() : m_serial(nextSerial()) {}
    CellStore(const CellStore &o) : m_blocks(o.m_blocks), m_count(o.m_count), m_dead(o.m_dead), m_rows(o.m_rows), m_cols(o.m_cols), m_serial(nextSerial()) {}
    CellStore &operator=(const CellStore &o)
    {
        m_blocks = o.m_blocks;
        m_count = o.m_count;
        m_dead = o.m_dead;
        m_rows = o.m_rows;
        m_cols = o.m_cols;
        m_serial = nextSerial();
        m_last = nullptr;
        return *this;
    }
    // A move takes the blocks and leaves the store it was taken from empty, with no memory of where it last added a cell (that place is a block it no longer has).
    CellStore(CellStore &&o) noexcept : m_blocks(std::move(o.m_blocks)), m_count(o.m_count), m_dead(o.m_dead), m_rows(std::move(o.m_rows)), m_cols(std::move(o.m_cols)), m_serial(o.m_serial)
    {
        o.reset();
    }
    CellStore &operator=(CellStore &&o) noexcept
    {
        if (this != &o) {
            m_blocks = std::move(o.m_blocks);
            m_count = o.m_count;
            m_dead = o.m_dead;
            m_rows = std::move(o.m_rows);
            m_cols = std::move(o.m_cols);
            m_serial = o.m_serial;
            m_last = nullptr;
            o.reset();
        }
        return *this;
    }

    // The order of the sheet's rows and of its columns, as ids (see calc/linemap.h).
    const jp::calc::RowMap &rows() const { return m_rows; }
    const jp::calc::ColMap &cols() const { return m_cols; }
    // A number that is this store's alone (a record of an edit of its rows or columns names it, so that it is never given to another sheet's order).
    quint64 serial() const { return m_serial; }

    // Adds a cell at the place. A cell at a place that has one already replaces it (the
    // later one stands), as finish() decides.
    void add(quint32 row, quint32 col, const Cell &cell);
    void add(const PlacedCell &at) { add(at.row, at.col, at.cell); }
    // Puts every block in order and drops a cell a later one replaces. Reads
    // (find, forEach) are only right after this.
    void finish();
    // How many cells there are (before finish(), counting any a later one will replace); those of rows and columns that are deleted are not counted.
    qint64 cellCount() const { return m_count - m_dead; }
    // The cell at the place, or null.
    const Cell *find(quint32 row, quint32 col) const;
    // The same by the ids of the cell's row and column (what a formula's node and the window's marks hold): no turning of places into ids, so the cell is found wherever its row and column
    // are now (a deleted one is still found: it is held, dead).
    const Cell *findById(quint32 rowId, quint32 colId) const;
    Cell *findMutableById(quint32 rowId, quint32 colId) { return const_cast<Cell *>(findById(rowId, colId)); }
    // Ids and places, one for the other (kNoLine for a place past the sheet's end, and for the id of a row or column that is deleted).
    quint32 rowIdAt(quint32 place) const { return rowIdOf(place); }
    quint32 colIdAt(quint32 place) const { return colIdOf(place); }
    quint32 rowPlaceOf(quint32 id) const { return rowPosOf(id); }
    quint32 colPlaceOf(quint32 id) const { return colPosOf(id); }
    // The places of the lines a reference names by the ids at its ends (`startId` is the end written first): both alive, their places in order; an end that is dead, the line next to where it was (a
    // start is the line that followed it, an end the one that preceded it: docs/jeffcrunch-rowids.md, 4.4). False when nothing is left of it: a single line that is dead, an end with no line to go
    // to, the start after the end.
    bool rowRangeOf(quint32 startId, quint32 endId, quint32 *first, quint32 *last) const { return lineRangeOf(m_rows, startId, endId, first, last); }
    bool colRangeOf(quint32 startId, quint32 endId, quint32 *first, quint32 *last) const { return lineRangeOf(m_cols, startId, endId, first, last); }
    // Whether no row and no column of the sheet has been inserted or deleted: every id is its place.
    bool isPlain() const { return plain(); }
    // How many runs of consecutive ids the rows (the columns) are in; for the bench and the tests.
    size_t rowRunCount() const { return m_rows.runCount(); }
    size_t colRunCount() const { return m_cols.runCount(); }
    // Whether anything can still name the id: its line is in the order, or was deleted and a step could bring it back. (A mark of the window on an id that is not is let go of before the id is given again.)
    bool rowIdKnown(quint32 id) const { return m_rows.isIdentity() ? id < m_rows.size() : (m_rows.core().posOf(id) != kNoLine || m_rows.core().holdsDead(id)); }
    bool colIdKnown(quint32 id) const { return m_cols.isIdentity() ? id < m_cols.size() : (m_cols.core().posOf(id) != kNoLine || m_cols.core().holdsDead(id)); }
    // Edits, for a store that is finished: the cell at the place, to be changed where it is (null if there is none); a cell put at the place, in place of the one that is
    // there, and the cell as it is held; a cell taken away (false if there was none). A cell is put in at the cost of moving the cells after it in its block of rows.
    Cell *findMutable(quint32 row, quint32 col);
    Cell *set(quint32 row, quint32 col, const Cell &cell);
    Cell *set(const PlacedCell &at) { return set(at.row, at.col, at.cell); }
    bool erase(quint32 row, quint32 col);

    // What a walk over a range did, counted as it did it: the cells it stepped over one by one (every move of the cursor along a row of a block is counted, and every run of rows or of columns it
    // goes along), and the comparisons of its searches for the first cell of each row. A test counts them to see that a narrow range of a wide sheet costs what it holds.
    struct RangeWalk {
        qint64 stepped = 0;
        qint64 compared = 0;
    };

    // Calls f(row, col, cell) for each cell in order of row, then column.
    template <class F>
    void forEach(F f) const
    {
        if (plain()) {
            for (const auto &block : m_blocks)
                for (const Cell &c : block.second.cells) f(quint32(c.rowId), quint32(c.colId), c);
            return;
        }
        forEachByPlace(f);
    }
    template <class F>
    Q_NEVER_INLINE void forEachByPlace(F f) const
    {
        ColPlan plan;
        planFor(m_cols, 0, quint32(-1), &plan);
        SmallList<Seg, 8> segs;
        segs.resize(plan.spans.size());
        walkRows(0, quint32(-1), nullptr, [&](quint32 rowPos, CellIt a, CellIt b) {
            return visitRow(a, b, plan, true, nullptr, segs.begin(), [&](quint32 colPos, const Cell &c) {
                f(rowPos, colPos, c);
                return true;
            });
        });
    }
    // Calls f(row, col, cell) for the cells of each row from its last cell backwards, until f gives true for one (then the next row up begins): for the last cell of each row that is of use,
    // at the cost of the rows (and a search in a block for where each begins), not of every cell. Rows are visited from the last to the first.
    template <class F>
    void forEachRowTail(F f) const
    {
        RowTailCursor cursor;
        qint64 steps = 0;
        forEachRowTailSome(&cursor, std::numeric_limits<qint64>::max(), &steps, f);
    }
    // The same walk in slices: the place it has got to. Start with a fresh cursor; each call goes on from where the last stopped. The place is a row's place, so rows or columns inserted or deleted between
    // two calls (the walk of a store that was edited, or was not) go on from that place.
    struct RowTailCursor {
        bool started = false, done = false;
        quint32 nextPos = 0;                     // the rows before this place are still to be walked
        std::vector<Span> rows, cols;     // (the runs of the rows and of the columns, kept while the order is as it was)
        quint64 rowsVersion = ~quint64(0), colsVersion = ~quint64(0);
        quint64 seenRows = ~quint64(0), seenCols = ~quint64(0);   // (the order of the rows and of the columns at the last call)
        bool restarted = false;   // the order was edited since the last call, so the walk began again from the end: what the caller took from the walk before is stale (the caller sets this false)
    };
    // Walks on from the cursor as forEachRowTail does, adding to `*steps` one for each row and each cell it looks at, and stops after the row it is in once `*steps` has reached `limit`
    // (so a slice takes at most a row's cells past it). True when every cell has been walked. A store changed between two calls is walked from what is there (never past it).
    template <class F>
    bool forEachRowTailSome(RowTailCursor *cur, qint64 limit, qint64 *steps, F f) const
    {
        if (cur->done) return true;
        if (!cur->started) {
            cur->started = true;
            cur->nextPos = quint32(-1);
        } else if (cur->seenRows != m_rows.version() || cur->seenCols != m_cols.version()) {
            // (rows or columns were inserted or deleted since the last slice: a place is not the row it was)
            cur->nextPos = quint32(-1);
            cur->restarted = true;
        }
        cur->seenRows = m_rows.version();
        cur->seenCols = m_cols.version();
        quint32 next = cur->nextPos;
        if (plain()) {
            if (next == 0 || m_blocks.empty()) {
                cur->done = true;
                return true;
            }
            // (where the walk is: in the last block that has a row before the place, at the cells before it)
            auto block = m_blocks.upper_bound((next - 1) / kBlockRows);
            if (block == m_blocks.begin()) {
                cur->done = true;
                return true;
            }
            --block;
            std::ptrdiff_t end = std::lower_bound(block->second.cells.begin(), block->second.cells.end(), next, [](const Cell &c, quint32 r) { return c.rowId < r; }) - block->second.cells.begin();
            while (*steps < limit) {
                const std::vector<Cell> &cells = block->second.cells;
                if (end <= 0) {
                    if (block == m_blocks.begin()) {
                        cur->done = true;
                        return true;
                    }
                    --block;
                    end = std::ptrdiff_t(block->second.cells.size());
                    ++*steps;
                    continue;
                }
                const quint32 row = cells[size_t(end - 1)].rowId;
                const auto first = std::lower_bound(cells.begin(), cells.begin() + end, row, [](const Cell &c, quint32 r) { return c.rowId < r; });
                ++*steps;
                for (auto at = cells.begin() + end; at != first;) {
                    --at;
                    ++*steps;
                    if (f(quint32(at->rowId), quint32(at->colId), *at)) break;
                }
                end = first - cells.begin();
                next = row;
            }
            cur->nextPos = next;
            return false;
        }
        return tailSomeByPlace(cur, next, limit, steps, f);
    }
    template <class F>
    Q_NEVER_INLINE bool tailSomeByPlace(RowTailCursor *cur, quint32 next, qint64 limit, qint64 *steps, F f) const
    {
        refreshSpans(m_rows, &cur->rows, &cur->rowsVersion);
        refreshSpans(m_cols, &cur->cols, &cur->colsVersion);
        next = std::min(next, m_rows.size());
        const std::vector<Span> &rs = cur->rows;
        const std::vector<Span> &cs = cur->cols;
        ColPlan plan;
        for (const Span &c : cs) plan.spans.push_back(c);
        plan.finish();
        SmallList<Seg, 8> segs;
        segs.resize(plan.spans.size());
        while (*steps < limit) {
            if (next == 0 || rs.empty()) {
                cur->done = true;
                return true;
            }
            // the run of rows that holds the place before `next` (the last run that begins before it)
            const auto it = std::lower_bound(rs.begin(), rs.end(), next, [](const Span &s, quint32 v) { return s.base < v; });
            const Span &s = *(it - 1);
            const quint32 hiId = s.first + (next - 1 - s.base);
            const std::vector<Cell> *held = nullptr;
            CellIt end;
            for (auto block = m_blocks.upper_bound(hiId / kBlockRows); block != m_blocks.begin() && !held;) {
                --block;
                if (block->first * kBlockRows + kBlockRows - 1 < s.first) break;
                const std::vector<Cell> &cells = block->second.cells;
                ++*steps;
                end = std::upper_bound(cells.begin(), cells.end(), hiId, [](quint32 v, const Cell &c) { return v < c.rowId; });
                if (end == cells.begin()) continue;
                if ((end - 1)->rowId < s.first) break;
                held = &cells;
            }
            if (!held) {
                next = s.base;
                continue;
            }
            // (the rows of that block, from there down, while they are in this run of rows)
            while (end != held->begin() && *steps < limit) {
                const quint32 id = (end - 1)->rowId;
                if (id < s.first) {
                    next = s.base;
                    break;
                }
                const CellIt first = std::lower_bound(held->begin(), end, id, [](const Cell &c, quint32 v) { return c.rowId < v; });
                const quint32 rowPos = s.base + (id - s.first);
                ++*steps;
                visitRowBack(first, end, plan, segs.begin(), [&](quint32 colPos, const Cell &c) {
                    ++*steps;
                    return f(rowPos, colPos, c);
                });
                next = rowPos;
                end = first;
            }
        }
        cur->nextPos = next;
        return false;
    }
    // Calls f(row, col, cell) for each cell inside the rectangle (both ends in it), in order of row and then column; f gives false to stop. Only the rows and cells in it are
    // looked at: a narrow range of a wide sheet costs what it holds, and a search for each row's first cell, not a walk over the cells before it. (`walk`, if given,
    // counts what was done: see RangeWalk.)
    template <class F>
    void forEachInRange(quint32 row1, quint32 col1, quint32 row2, quint32 col2, F f, RangeWalk *walk = nullptr) const
    {
        if (!plain()) {
            inRangeByPlace(row1, col1, row2, col2, f, walk);
            return;
        }
        // The place in a block's cells. Every step along them is made, and counted, here.
        class Cursor {
        public:
            Cursor(std::vector<Cell>::const_iterator at, RangeWalk *walk) : m_at(at), m_walk(walk) {}
            const Cell &operator*() const { return *m_at; }
            const Cell *operator->() const { return &*m_at; }
            Cursor &operator++()
            {
                ++m_at;
                if (m_walk) ++m_walk->stepped;
                return *this;
            }
            std::vector<Cell>::const_iterator at() const { return m_at; }
            void jumpTo(std::vector<Cell>::const_iterator at) { m_at = at; }

        private:
            std::vector<Cell>::const_iterator m_at;
            RangeWalk *m_walk;
        };
        for (auto block = m_blocks.lower_bound(row1 / kBlockRows); block != m_blocks.end() && block->first * kBlockRows <= row2; ++block) {
            const std::vector<Cell> &cells = block->second.cells;
            const quint32 first = block->first * kBlockRows;
            quint32 r = std::max(row1, first);
            const quint32 last = std::min(row2, first + kBlockRows - 1);
            Cursor pos(cells.begin(), walk);
            while (r <= last && pos.at() != cells.end()) {
                Cell key;
                key.rowId = r;
                key.colId = quint16(col1);
                pos.jumpTo(std::lower_bound(pos.at(), cells.end(), key, [walk](const Cell &a, const Cell &b) {
                    if (walk) ++walk->compared;
                    return a.rowId != b.rowId ? a.rowId < b.rowId : a.colId < b.colId;
                }));
                if (pos.at() == cells.end() || pos->rowId > last) break;
                if (pos->rowId != r) {
                    // that row has no cell at or after the first column: look for the first column again in the row that has one
                    r = pos->rowId;
                    continue;
                }
                while (pos.at() != cells.end() && pos->rowId == r && pos->colId <= col2) {
                    if (!f(r, quint32(pos->colId), *pos)) return;
                    ++pos;
                }
                // the rest of this row's cells are right of the range: the search for the next row's first cell passes them
                ++r;
            }
        }
    }
    // Moves cells about, for cells shifted (inserted and deleted among the cells of some columns or rows; whole rows and columns are insertLines and deleteLines, which move nothing): every cell
    // for which `affected(row, col, cell)` is true (those of the blocks that hold rows from `firstRow` on are looked at, and no others) is taken out; `map(placed)` changes the PlacedCell to what it is
    // after (its place and the cell), and gives false for one that is gone (it is given to
    // `removed`, as it was, with its place); `moved(before, after)` (PlacedCells, each with the ids of the cell's row and column, before and after) is told of each that stays; they are put in again, and the blocks are put in order. The cost is that of the cells taken out.
    template <class Affected, class Map, class Moved>
    void reshuffle(quint32 firstRow, Affected affected, Map map, Moved moved, std::vector<PlacedCell> *removed)
    {
        std::vector<PlacedCell> moving;
        quint32 firstKey = firstRow / kBlockRows;
        if (!m_rows.isIdentity()) {
            // (the rows from `firstRow` on are the runs of ids from that place: the block of the lowest id among them is where it begins)
            std::vector<Span> rs;
            spans(m_rows, firstRow, quint32(-1), &rs);
            if (rs.empty()) return;
            firstKey = ~0u;
            for (const Span &s : rs) firstKey = std::min(firstKey, s.first / kBlockRows);
        }
        for (auto it = m_blocks.lower_bound(firstKey); it != m_blocks.end();) {
            std::vector<Cell> &cells = it->second.cells;
            size_t keep = 0;
            for (size_t i = 0; i < cells.size(); ++i) {
                const quint32 row = rowPosOf(cells[i].rowId), col = colPosOf(cells[i].colId);
                // (a cell of a row or column that is deleted stays where it is)
                if (row != kNoLine && col != kNoLine && affected(row, col, cells[i])) moving.push_back(PlacedCell{row, col, cells[i]});
                else cells[keep++] = cells[i];
            }
            cells.resize(keep);
            if (cells.empty()) it = m_blocks.erase(it);
            else ++it;
        }
        m_last = nullptr;
        m_count -= qint64(moving.size());
        // (the cells come in order of row and column, and a shift keeps that order: they are put into the blocks they come to a run at a time, and each run is merged with what the block
        // kept, which costs the cells of the block and not a sort of them)
        std::vector<Cell> run;
        quint32 runKey = 0;
        auto flush = [&]() {
            if (run.empty()) return;
            auto less = [](const Cell &a, const Cell &b) { return a.rowId != b.rowId ? a.rowId < b.rowId : a.colId < b.colId; };
            if (!std::is_sorted(run.begin(), run.end(), less)) std::sort(run.begin(), run.end(), less);
            Block &block = m_blocks[runKey];
            const size_t mid = block.cells.size();
            block.cells.insert(block.cells.end(), run.begin(), run.end());
            if (mid > 0 && less(block.cells[mid], block.cells[mid - 1])) std::inplace_merge(block.cells.begin(), block.cells.begin() + std::ptrdiff_t(mid), block.cells.end(), less);
            m_count += qint64(run.size());
            run.clear();
        };
        for (const PlacedCell &was : moving) {
            PlacedCell after = was;
            if (!map(&after)) {
                if (removed) removed->push_back(was);
                continue;
            }
            const quint32 rowId = rowIdOf(after.row), colId = colIdOf(after.col);
            if (rowId == kNoLine || colId == kNoLine) {
                if (removed) removed->push_back(was);
                continue;
            }
            after.cell.rowId = rowId;
            after.cell.colId = quint16(colId);
            moved(was, after);
            const quint32 key = rowId / kBlockRows;
            if (!run.empty() && key != runKey) flush();
            runKey = key;
            run.push_back(after.cell);
        }
        flush();
        m_last = nullptr;
    }

    // ---- whole rows and columns inserted and deleted: operations on the order of the rows or of the columns, which move no cell.
    // What one edit did to the order, for Undo and Redo: the order's own record of the edit, and (a delete) of the rows or columns added at the end to keep the sheet as long as it was.
    struct LineEdits {
        jp::calc::LineEdit edit;
        jp::calc::LineEdit refill;
        bool refilled = false;    // a delete: `refill` is the lines added at the end
        bool made = false;        // the edit has been made (undo and redo go by it)
        quint64 owner = 0;        // the store whose order it is
        quint32 lines() const
        {
            quint32 n = 0;
            for (const jp::calc::IdRun &r : edit.runs) n += r.count;
            return n;
        }
    };
    // How the places of lines changed: the lines at the places from `at` on (from `at` + delta on, after an insert) moved by `delta` (up when it is less than 0).
    struct LineShift {
        quint32 at = 0;
        qint64 delta = 0;
    };
    // `count` rows (columns) inserted at the place `at`; lines pushed off the end of the sheet are deleted (their cells stay with them). Else the status (nothing is changed). `shift` is how the places
    // changed, and `deadFormulas` is given the cells with a formula that were in lines that died.
    jp::calc::LineStatus insertLines(bool rows, quint32 at, quint32 count, LineEdits *edits, LineShift *shift = nullptr, std::vector<Cell> *deadFormulas = nullptr);
    // `count` rows (columns) deleted at the place `at` (a count past the end is cut to it): the sheet gets as many new, empty rows (columns) at its end, so it is as long as it was. Their cells stay, as dead cells.
    jp::calc::LineStatus deleteLines(bool rows, quint32 at, quint32 count, LineEdits *edits, LineShift *shift = nullptr, std::vector<Cell> *deadFormulas = nullptr);
    // Whether an edit of `count` rows (columns) can be made: the order is not full of runs or ids (else the status that says so).
    jp::calc::LineStatus canEditLines(bool rows, quint32 count) const;
    // Whether the order should be put back to one run (rebaseLines) before an edit of `count` lines is made: it is near the most runs, dead records, or ids it may have.
    bool needsCompaction(bool rows, quint32 count) const;
    // The order as it was before the edit was made, and as it was after it. Else the status (the edit is not the last one made to the order, or is not of this store's, and nothing is changed).
    jp::calc::LineStatus undoLines(bool rows, LineEdits &edits, LineShift *shift = nullptr, std::vector<Cell> *deadFormulas = nullptr);
    jp::calc::LineStatus redoLines(bool rows, LineEdits &edits, LineShift *shift = nullptr, std::vector<Cell> *deadFormulas = nullptr);
    // The step that made the edit will never be undone or done again: the cells of the lines that only it could bring back are let go of, and so are the records of those lines (and the ids that
    // nothing can name are given back if they were the last taken). The edits are empty after. `keepDead`: the records of lines that were deleted stay, for formulas held in ids that name them by id
    // (an end that was deleted is the line next to where it was); a rebase is what lets go of them.
    void forgetLines(bool rows, LineEdits *edits, bool keepDead = false);
    // Puts the order of the rows (columns) back to one run, from 0: the cells are given the new ids, those of dead lines are let go of, and every edit made to the order before is stale (Undo of it
    // is refused). The status if that cannot be done.
    // `table`, if given, is how the ids of the live lines were renumbered (sorted by old first id): see renumberLine.
    jp::calc::LineStatus rebaseLines(bool rows, std::vector<jp::calc::LineMapCore::Renumber> *table = nullptr);
    // The new id of an id that a rebase renumbered by `table`, or kNoLine for a line that was dead (and is gone).
    static quint32 renumberLine(const std::vector<jp::calc::LineMapCore::Renumber> &table, quint32 id);
    // Limits for the order of the rows and of the columns (for a test of the caps: the caps of a sheet are those of jp::calc::LineLimits). Only for a store whose order has no edit.
    void setLineLimits(const jp::calc::LineLimits &rows, const jp::calc::LineLimits &cols);
    // Calls f(before, after) (PlacedCells) for each cell with a formula, or that belongs to an array formula, whose place changed by `shift` (read after the edit).
    template <class F>
    void forEachMoved(bool rows, const LineShift &shift, F f) const
    {
        if (shift.delta == 0) return;
        const quint32 from = shift.delta > 0 ? shift.at + quint32(shift.delta) : shift.at;
        auto emit = [&](quint32 row, quint32 col, const Cell &c) {
            PlacedCell now{row, col, c};
            PlacedCell was = now;
            if (rows) was.row = quint32(qint64(row) - shift.delta);
            else was.col = quint32(qint64(col) - shift.delta);
            f(was, now);
        };
        if (rows) {
            walkRows(from, quint32(-1), nullptr, [&](quint32 rowPos, CellIt a, CellIt b) {
                for (; a != b; ++a) {
                    if (!(a->flags & (HasFormula | ArrayMember))) continue;
                    const quint32 col = colPosOf(a->colId);
                    if (col != kNoLine) emit(rowPos, col, *a);
                }
                return true;
            });
            return;
        }
        for (const auto &block : m_blocks)
            for (const Cell &c : block.second.cells) {
                if (!(c.flags & (HasFormula | ArrayMember))) continue;
                const quint32 col = colPosOf(c.colId), row = rowPosOf(c.rowId);
                if (row != kNoLine && col != kNoLine && col >= from) emit(row, col, c);
            }
    }
    // Where a cell given by a walk is now (false if its row or column is deleted).
    bool placeOf(const PlacedCell &was, quint32 *row, quint32 *col) const
    {
        *row = rowPosOf(was.cell.rowId);
        *col = colPosOf(was.cell.colId);
        return *row != kNoLine && *col != kNoLine;
    }
    // Cells held that are in rows or columns that are deleted, and the most that Undo can bring back.
    qint64 deadCount() const { return m_dead; }
    // The same, counted cell by cell (for a check of deadCount and cellCount, which are kept as the order is edited).
    qint64 deadCountByScan() const;

    // Puts the blocks that were added to out of order in order (the cells that were put in again by reshuffle).
    void reorder();
    int blockCount() const { return int(m_blocks.size()); }
    // What the cells take, in bytes, as the blocks hold them.
    qint64 memoryBytes() const;

private:
    struct Block {
        std::vector<Cell> cells;
        bool ordered = true;
    };
    using CellIt = std::vector<Cell>::const_iterator;
    // The columns of a walk: their runs in the order of their places, and the order of their ids (cells are in the order of ids).
    struct ColPlan {
        SmallList<Span, 8> spans;
        SmallList<quint32, 8> byId;
        bool inOrder = true;   // the ids go up with the places
        void finish()
        {
            for (size_t i = 0; i < spans.size(); ++i) byId.push_back(quint32(i));
            std::sort(byId.begin(), byId.end(), [&](quint32 a, quint32 b) { return spans[a].first < spans[b].first; });
            for (size_t i = 0; i < byId.size(); ++i) inOrder = inOrder && byId[i] == i;
        }
    };
    static quint64 nextSerial();
    void reset()
    {
        m_blocks.clear();
        m_count = 0;
        m_dead = 0;
        m_last = nullptr;
        m_serial = nextSerial();
        m_rows = jp::calc::RowMap(jp::calc::LineLimits::rows(), jp::calc::LineLimits::rows().capacity);
        m_cols = jp::calc::ColMap(jp::calc::LineLimits::cols(), jp::calc::LineLimits::cols().capacity);
    }
    // No row or column has been inserted or deleted: ids are places.
    bool plain() const { return m_rows.isIdentity() && m_cols.isIdentity(); }
    // (a sheet with no edit has ids that are places, whatever the place; else the last place a thread looked up is where it looks first)
    quint32 rowIdOf(quint32 pos) const { return m_rows.isIdentity() ? pos : m_rows.core().idAt(pos, &hint(0)); }
    quint32 colIdOf(quint32 pos) const { return m_cols.isIdentity() ? pos : m_cols.core().idAt(pos, &hint(1)); }
    quint32 rowPosOf(quint32 id) const { return m_rows.isIdentity() ? id : m_rows.core().posOf(id, &hint(2)); }
    quint32 colPosOf(quint32 id) const { return m_cols.isIdentity() ? id : m_cols.core().posOf(id, &hint(3)); }
    template <class Map>
    static bool lineRangeOf(const Map &map, quint32 startId, quint32 endId, quint32 *first, quint32 *last)
    {
        if (map.isIdentity()) {
            if (startId >= map.size() || endId >= map.size()) return false;
            *first = std::min(startId, endId);
            *last = std::max(startId, endId);
            return true;
        }
        const jp::calc::LineMapCore &core = map.core();
        const uint32_t a = core.posOf(startId), b = core.posOf(endId);
        if (a != kNoLine && b != kNoLine) {
            *first = std::min(a, b);
            *last = std::max(a, b);
            return true;
        }
        return core.resolveRange(startId, endId, first, last);
    }
    static jp::calc::LineHint &hint(int which)
    {
        thread_local jp::calc::LineHint hints[4];
        return hints[which];
    }
    // The runs of ids at the places first..last (cut to the sheet), with the place each begins at.
    template <class Map>
    static void spans(const Map &map, quint32 first, quint32 last, std::vector<Span> *out)
    {
        out->clear();
        if (map.size() == 0) return;
        last = std::min(last, map.size() - 1);
        if (first > last) return;
        std::vector<jp::calc::IdRun> runs;
        map.core().walk(first, last, &runs);
        quint32 base = first;
        for (const jp::calc::IdRun &r : runs) {
            out->push_back({r.first, r.count, base});
            base += r.count;
        }
    }
    // The runs of all the places, kept while the order is as it was.
    template <class Map>
    static void refreshSpans(const Map &map, std::vector<Span> *out, quint64 *version)
    {
        if (*version == map.version() && !out->empty()) return;
        spans(map, 0, quint32(-1), out);
        *version = map.version();
    }
    // The runs of the columns at the places first..last, from the order's cache of them (a range read by many formulas is walked once).
    template <class Map>
    static void planFor(const Map &map, quint32 first, quint32 last, ColPlan *plan)
    {
        if (map.size() == 0) return;
        last = std::min(last, map.size() - 1);
        if (first > last) return;
        const auto runs = map.core().runsFor(first, last);
        quint32 base = first;
        for (const jp::calc::IdRun &r : *runs) {
            plan->spans.push_back({r.first, r.count, base});
            base += r.count;
        }
        plan->finish();
    }
    using Seg = std::pair<CellIt, CellIt>;
    static CellIt findFrom(CellIt from, CellIt b, quint32 id, bool dense, size_t runs, RangeWalk *walk)
    {
        if (dense || size_t(b - from) <= 16 * runs) {
            while (from != b && quint32(from->colId) < id) {
                ++from;
                if (walk) ++walk->stepped;
            }
            return from;
        }
        return std::lower_bound(from, b, id, [walk](const Cell &c, quint32 v) {
            if (walk) ++walk->compared;
            return quint32(c.colId) < v;
        });
    }
    // Where the cells of a row [a, b) are in each run of columns (`segs` has a place for each run), and which runs have some (a bit for each, the runs being at most 64). `dense`: the row is walked
    // whole anyway (one pass along it, each cell looked at once); else a search finds where each run of columns begins (so a narrow range of a wide row costs what it holds).
    static quint64 segmentsOf(CellIt a, CellIt b, const ColPlan &plan, bool dense, RangeWalk *walk, Seg *segs)
    {
        const size_t n = plan.spans.size();
        const Span *sp = plan.spans.begin();
        const quint32 *byId = plan.byId.begin();
        quint64 mask = 0;
        if (dense) {
            size_t si = 0;
            CellIt it = a;
            while (it != b) {
                const quint32 id = it->colId;
                while (si < n && sp[byId[si]].first + sp[byId[si]].count <= id) ++si;
                if (si == n) break;
                const Span &s = sp[byId[si]];
                if (id < s.first) {   // (a cell of a column that is deleted)
                    ++it;
                    continue;
                }
                const CellIt start = it;
                while (it != b && quint32(it->colId) - s.first < s.count) ++it;
                segs[byId[si]] = {start, it};
                mask |= quint64(1) << byId[si];
                ++si;
            }
            return mask;
        }
        CellIt at = a;
        for (size_t i = 0; i < n; ++i) {
            const Span &s = sp[byId[i]];
            at = findFrom(at, b, s.first, false, n, walk);
            CellIt end = at;
            while (end != b && quint32(end->colId) - s.first < s.count) {
                ++end;
                if (walk) ++walk->stepped;
            }
            if (end != at) {
                segs[byId[i]] = {at, end};
                mask |= quint64(1) << byId[i];
            }
            at = end;
        }
        return mask;
    }
    // The cells of a row [a, b) in each run of columns that has some, in the order of the ids, for orders of more than 64 runs of columns (a rare thing: a bit for each run is not kept).
    struct Used {
        quint32 span;
        CellIt a, b;
    };
    static void usedOf(CellIt a, CellIt b, const ColPlan &plan, bool dense, RangeWalk *walk, std::vector<Used> *used)
    {
        const size_t n = plan.spans.size();
        CellIt at = a;
        if (dense) {
            size_t si = 0;
            while (at != b) {
                const quint32 id = at->colId;
                while (si < n && plan.spans[plan.byId[si]].first + plan.spans[plan.byId[si]].count <= id) ++si;
                if (si == n) break;
                const Span &s = plan.spans[plan.byId[si]];
                if (id < s.first) {
                    ++at;
                    continue;
                }
                const CellIt start = at;
                while (at != b && quint32(at->colId) - s.first < s.count) ++at;
                used->push_back({plan.byId[si], start, at});
                ++si;
            }
            return;
        }
        for (size_t i = 0; i < n; ++i) {
            const Span &s = plan.spans[plan.byId[i]];
            at = findFrom(at, b, s.first, false, n, walk);
            CellIt end = at;
            while (end != b && quint32(end->colId) - s.first < s.count) {
                ++end;
                if (walk) ++walk->stepped;
            }
            if (end != at) used->push_back({plan.byId[i], at, end});
            at = end;
        }
    }
    // Calls g(place, cell) for the cells of a row [a, b) in order of the columns' places (those of live columns only); g gives false to stop. The cost is the row's cells and the runs of columns, not a
    // search for each run in each row. `segs` has a place for each run of columns.
    template <class G>
    static bool visitRow(CellIt a, CellIt b, const ColPlan &plan, bool dense, RangeWalk *walk, Seg *segs, G g)
    {
        const size_t n = plan.spans.size();
        if (n == 0) return true;
        if (n == 1) {
            const Span &s = plan.spans[0];
            for (CellIt at = findFrom(a, b, s.first, dense, 1, walk); at != b && quint32(at->colId) - s.first < s.count; ++at) {
                if (walk) ++walk->stepped;
                if (!g(s.base + (quint32(at->colId) - s.first), *at)) return false;
            }
            return true;
        }
        if (n > 64) {
            std::vector<Used> used;
            usedOf(a, b, plan, dense, walk, &used);
            std::sort(used.begin(), used.end(), [](const Used &x, const Used &y) { return x.span < y.span; });
            for (const Used &u : used) {
                const Span &s = plan.spans[u.span];
                for (CellIt it = u.a; it != u.b; ++it) {
                    if (walk) ++walk->stepped;
                    if (!g(s.base + (quint32(it->colId) - s.first), *it)) return false;
                }
            }
            return true;
        }
        quint64 mask = segmentsOf(a, b, plan, dense, walk, segs);
        while (mask) {
            const unsigned k = unsigned(qCountTrailingZeroBits(mask));
            mask &= mask - 1;
            const Span &s = plan.spans[k];
            for (CellIt it = segs[k].first; it != segs[k].second; ++it) {
                if (walk) ++walk->stepped;
                if (!g(s.base + (quint32(it->colId) - s.first), *it)) return false;
            }
        }
        return true;
    }
    // The same from the last cell back: g gives true to stop (then the walk goes on with the next row).
    template <class G>
    static void visitRowBack(CellIt a, CellIt b, const ColPlan &plan, Seg *segs, G g)
    {
        const size_t n = plan.spans.size();
        if (n == 0) return;
        if (n > 64) {
            std::vector<Used> used;
            usedOf(a, b, plan, true, nullptr, &used);
            std::sort(used.begin(), used.end(), [](const Used &x, const Used &y) { return x.span < y.span; });
            for (size_t k = used.size(); k > 0; --k) {
                const Used &u = used[k - 1];
                const Span &s = plan.spans[u.span];
                for (CellIt it = u.b; it != u.a;) {
                    --it;
                    if (g(s.base + (quint32(it->colId) - s.first), *it)) return;
                }
            }
            return;
        }
        quint64 mask = segmentsOf(a, b, plan, true, nullptr, segs);
        while (mask) {
            const unsigned k = 63u - unsigned(qCountLeadingZeroBits(mask));
            mask &= ~(quint64(1) << k);
            const Span &s = plan.spans[k];
            for (CellIt it = segs[k].second; it != segs[k].first;) {
                --it;
                if (g(s.base + (quint32(it->colId) - s.first), *it)) return;
            }
        }
    }
    // Calls h(row place, first cell of the row, end of its cells) for each row that has cells, in order of the rows' places, for the rows at the places from `first` to `last`; h gives false to stop.
    template <class H>
    bool walkRows(quint32 first, quint32 last, RangeWalk *walk, H h) const
    {
        std::vector<Span> rs;
        spans(m_rows, first, last, &rs);
        for (const Span &s : rs) {
            const quint32 lastId = s.first + s.count - 1;
            for (auto block = m_blocks.lower_bound(s.first / kBlockRows); block != m_blocks.end() && block->first <= lastId / kBlockRows; ++block) {
                const std::vector<Cell> &cells = block->second.cells;
                auto at = std::lower_bound(cells.begin(), cells.end(), s.first, [walk](const Cell &c, quint32 id) {
                    if (walk) ++walk->compared;
                    return c.rowId < id;
                });
                while (at != cells.end() && at->rowId <= lastId) {
                    const quint32 id = at->rowId;
                    auto end = at;
                    while (end != cells.end() && end->rowId == id) ++end;
                    if (!h(s.base + (id - s.first), at, end)) return false;
                    at = end;
                }
            }
        }
        return true;
    }
    template <class F>
    void inRangeByPlace(quint32 row1, quint32 col1, quint32 row2, quint32 col2, F f, RangeWalk *walk) const
    {
        row2 = std::min(row2, m_rows.size() ? m_rows.size() - 1 : 0u);
        col2 = std::min(col2, m_cols.size() ? m_cols.size() - 1 : 0u);
        if (m_rows.size() == 0 || m_cols.size() == 0 || row1 > row2 || col1 > col2) return;
        const auto rowRuns = m_rows.core().runsFor(row1, row2);
        ColPlan plan;
        planFor(m_cols, col1, col2, &plan);
        SmallList<Seg, 8> segs;
        segs.resize(plan.spans.size());
        if (walk) walk->stepped += qint64(rowRuns->size() + plan.spans.size());
        quint32 base = row1;
        for (const jp::calc::IdRun &s : *rowRuns) {
            const quint32 lastId = s.first + s.count - 1;
            for (auto block = m_blocks.lower_bound(s.first / kBlockRows); block != m_blocks.end() && block->first <= lastId / kBlockRows; ++block) {
                const std::vector<Cell> &cells = block->second.cells;
                const quint32 blockFirst = block->first * kBlockRows;
                quint32 r = std::max(s.first, blockFirst);
                const quint32 last = std::min(lastId, blockFirst + kBlockRows - 1);
                auto pos = cells.begin();
                while (r <= last && pos != cells.end()) {
                    pos = std::lower_bound(pos, cells.end(), r, [walk](const Cell &c, quint32 id) {
                        if (walk) ++walk->compared;
                        return c.rowId < id;
                    });
                    if (pos == cells.end() || pos->rowId > last) break;
                    r = pos->rowId;
                    auto end = pos;
                    while (end != cells.end() && end->rowId == r) {
                        ++end;
                        if (walk) ++walk->stepped;
                    }
                    const quint32 rowPos = base + (r - s.first);
                    if (!visitRow(pos, end, plan, false, walk, segs.begin(), [&](quint32 colPos, const Cell &c) { return f(rowPos, colPos, c); })) return;
                    pos = end;
                    ++r;
                }
            }
            base += s.count;
        }
    }
    // How many of the cells in the lines at these ids are in a line of the other kind that is live. `kill`: the lines die, the cells with a formula are given to `formulas`, and every cell in them
    // is made one with no formula (a cell of a deleted line holds none).
    qint64 liveCellsIn(bool rows, const std::vector<jp::calc::IdRun> &runs, bool kill, std::vector<Cell> *formulas = nullptr);
    // The cells in the lines at these ids are let go of; how many.
    qint64 purgeCells(bool rows, const std::vector<jp::calc::IdRun> &runs);
    bool ownEdit(const LineEdits &e) const { return e.owner == m_serial; }

    std::map<quint32, Block> m_blocks;
    qint64 m_count = 0;
    qint64 m_dead = 0;
    Block *m_last = nullptr;
    quint32 m_lastKey = 0;
    jp::calc::RowMap m_rows{jp::calc::LineLimits::rows(), jp::calc::LineLimits::rows().capacity};
    jp::calc::ColMap m_cols{jp::calc::LineLimits::cols(), jp::calc::LineLimits::cols().capacity};
    quint64 m_serial = 0;
};

// What a formula cell holds. Cells of a shared formula point at one text (the
// first cell's) and say how far they are from that cell; the text of a given
// cell is worked out when asked for (Sheet::formulaText).
struct Formula {
    qint32 text = -1;          // the place in Sheet::formulaTexts
    qint32 dRow = 0;           // rows below (negative: above) the cell the text was written for
    qint32 dCol = 0;           // and columns to the right
    qint32 range = -1;         // the place in Sheet::ranges of an array formula's cells or a data table's; -1 for none
};

// A run of the formatting inside a text: its characters, from `start` (counted in UTF-16 units) to where the next run begins, or to the end of the text, are in the font at `font`
// among the fonts the strings keep (SharedStrings::richFonts, Sheet::richFonts); kCellFont is a run in the font of the cell the text is in.
struct RichRun {
    static constexpr quint32 kCellFont = 0xFFFFFFFFu;
    quint32 start = 0;
    quint32 font = kCellFont;
};
using RichRuns = QVector<RichRun>;

// Every string the sheets of a workbook have in common, in order. The text of
// a rich string is its runs one after another.
struct SharedStrings {
    QVector<QString> strings;
    qint64 chars = 0;
    // The runs of the strings that have more than one font (by the string's place), when they were asked for (Limits::richText) and always for a workbook of the older binary form, and
    // the fonts the runs name. The first run of a string begins at 0.
    QHash<qint32, RichRuns> rich;
    QVector<FontSpec> richFonts;
    QString at(qint64 i) const { return i >= 0 && i < strings.size() ? strings[int(i)] : QString(); }
};

// What the reading of one sheet found out about the file, for the limits to be
// chosen by: the most (largest or longest) any one thing came to.
struct SheetStats {
    qint64 longestString = 0;        // characters, of an inline string or the text of a formula's result
    qint64 longestFormula = 0;
    qint64 sharedFormulas = 0;       // distinct shared formulas
    qint64 largestSharedRange = 0;   // cells
    qint64 expandedFormulaChars = 0; // the formulas' characters, a shared formula counted once for each cell
    qint64 xmlBytes = 0;             // what was read of the part
    qint64 heldBytes = 0;            // what the sheet takes to hold, as Limits::sheetBytes counts it
    qint64 largestToken = 0;         // the most bytes of XML in one piece (a tag with its attributes, a text, a comment) the parser was given
    qint64 largestRead = 0;          // the most bytes the parser asked for at one time
    qint64 columnSpans = 0;          // `col` elements
    qint64 rowSpecs = 0;             // rows that were kept as not the default
    qint64 formulaWork = 0;          // the work reading the formulas of a binary workbook's sheet took (see kMaxFormulaWork), counted: a shared formula is read once, not for each cell
    qint64 unreadFormulas = 0;       // formulas of a binary workbook that could not be turned into text: their cells keep the result the file stored
};

// What the file says about a run of columns (a `col` element): its width in characters of the default font's widest digit (with the padding the format adds),
// whether it is hidden, and the cell format that the cells of the columns that hold nothing have.
struct ColumnSpan {
    quint32 first = 0, last = 0;   // the columns (from 0), both ends in the run
    double width = -1;             // characters; -1 when the file gives none
    bool hidden = false;
    bool customWidth = false;
    quint32 style = 0;             // the place of a cell format; 0 for none
    quint8 outline = 0;            // the outline level
    bool collapsed = false, bestFit = false, phonetic = false;   // and the rest of what a `col` element says, for writing it again
};

// What the file says about a row that is not as the sheet's default: its height in points (-1 for none), whether it is hidden, and the cell format (when the row
// asks for it, `customFormat`) that the cells of the row that hold nothing have.
struct RowSpec {
    quint32 row = 0;
    double height = -1;
    bool hidden = false;
    bool customHeight = false;
    bool customFormat = false;
    quint32 style = 0;
    quint8 outline = 0;            // the outline level (0 to 7)
    bool collapsed = false;        // the row's group is collapsed (the row that follows the group says so)
    // Why a row that is hidden is: `hand` (a person hid it) and `filtered` (the sheet's filter does). `hidden` is whether either is (what the file says, and what the grid shows). A row the
    // file says is hidden is `filtered` when it is among the rows of a filter that has conditions (the filter hid it), else `hand`.
    bool hand = false;
    bool filtered = false;
};

// `sheetFormatPr`: the sizes of rows and columns that have none of their own.
struct SheetFormat {
    double defaultRowHeight = 15;   // points
    double defaultColWidth = 0;     // characters, with the padding; 0 when the file gives none (the base width says)
    int baseColWidth = 8;           // characters of the default font's widest digit, with no padding
    bool zeroHeight = false;        // the rows that have no row element are hidden
    bool customHeight = false;
    quint8 outlineLevelRow = 0;     // the deepest outline levels of the rows and of the columns (what the buttons of the outline show)
    quint8 outlineLevelCol = 0;
};

// `sheetView`: how the sheet was left on the screen.
struct SheetView {
    bool showGridLines = true;
    bool showHeadings = true;
    bool showZeros = true;
    bool rightToLeft = false;
    bool tabSelected = false;
    // How the sheet was left: the cells as they are (the usual), the sheet cut into the pages it prints on, or the pages themselves (the file's `view`).
    enum class Kind : quint8 { Normal, PageBreaks, PageLayout } kind = Kind::Normal;
    int zoom = 100;                 // percent: the zoom the sheet is shown at in the view it was left in (`zoomScale`)
    int normalZoom = 100;           // percent: the zoom of the cells as they are (`zoomScaleNormal`; the zoom above when the sheet was left in that view and the file says none)
    bool frozen = false;            // the first rows and columns stay in place (a split that is not frozen is not kept)
    quint32 frozenRows = 0, frozenCols = 0;
    quint32 topRow = 0, leftCol = 0;           // the cell at the top left of what shows (of the frozen part, when there is one)
    quint32 paneRow = 0, paneCol = 0;          // the cell at the top left of the part that scrolls, when frozen
    quint32 activeRow = 0, activeCol = 0;      // the cell the sheet was left on
};

// A rule for what may be put in cells (a `dataValidation` element): the kind of value, how it is compared, what happens when it is not allowed, and the messages.
struct Validation {
    enum class Type : quint8 { Any, Whole, Decimal, List, Date, Time, TextLength, Custom } type = Type::Any;
    enum class Style : quint8 { Stop, Warning, Information } style = Style::Stop;
    enum class Compare : quint8 { Between, NotBetween, Equal, NotEqual, Greater, Less, GreaterEqual, LessEqual } compare = Compare::Between;
    bool allowBlank = false;
    bool dropDown = true;          // a list is offered from a button on the cell (the file's showDropDown="1" turns it off)
    bool showError = false;        // the file asks for an alert when a value is not allowed (else it lets it in)
    bool showInput = false;
    QString errorTitle, error, promptTitle, prompt;
    QVector<CellRange> ranges;     // the cells it is for (sqref)
    QString formula1, formula2;    // as the file writes them (the limits, the choices, or the formula), relative to the first cell of the first range
};

// A value a color scale, data bar, or icon set is cut at (a `cfvo`): a number, a percent, the lowest or highest value, a formula, or a percentile.
struct CondValue {
    enum class Type : quint8 { Number, Percent, Max, Min, Formula, Percentile, AutoMin, AutoMax } type = Type::Number;
    QString value;       // the number or the formula, as the file writes it
    bool gte = true;     // an icon set: the icon is for values at or over this one (else over it)
    bool operator==(const CondValue &o) const { return type == o.type && value == o.value && gte == o.gte; }
};

// A rule of conditional formatting (a `cfRule`), for the cells of the ranges its `conditionalFormatting` names. The rules of a sheet are in the order of their priority (lower first).
struct CondRule {
    enum class Type : quint8 {
        CellIs, Expression, ColorScale, DataBar, IconSet, Top10, UniqueValues, DuplicateValues, ContainsText, NotContainsText, BeginsWith, EndsWith, ContainsBlanks, NotContainsBlanks,
        ContainsErrors, NotContainsErrors, TimePeriod, AboveAverage
    } type = Type::Expression;
    enum class Operator : quint8 { LessThan, LessThanOrEqual, Equal, NotEqual, GreaterThanOrEqual, GreaterThan, Between, NotBetween } op = Operator::Equal;
    int priority = 0;
    qint32 dxf = -1;                 // the differential format (Styles::dxfs) the rule applies; -1 for none
    bool stopIfTrue = false;
    QString formula1, formula2;      // as the file writes them, for the first cell of the first range (the file's `formula` elements)
    QString text;                    // the text of the rules that look for text
    QString timePeriod;              // today, yesterday, tomorrow, last7Days, thisWeek, lastWeek, nextWeek, thisMonth, lastMonth, nextMonth
    int rank = 10;                   // top or bottom N (a percent of the cells with `percent`)
    bool percent = false, bottom = false;
    bool above = true, equalAverage = false;   // above or below the average (or equal to it too)
    int stdDev = 0;                  // that many standard deviations from the average (0 for none)
    // a color scale (two or three values and their colors), a data bar (the shortest and longest, one color), an icon set (the values the icons change at)
    QVector<CondValue> values;
    QVector<ColorSpec> colors;
    QString iconSet;                 // "3TrafficLights1" and the like
    bool reverse = false, showValue = true;
    int minLength = 10, maxLength = 90;          // a data bar: the shortest and longest, in percent of the cell's width
    bool gradient = true, border = false, negativeSameColor = false;
    ColorSpec negativeColor, borderColor, negativeBorderColor, axisColor;
    enum class Axis : quint8 { Automatic, Middle, None } axis = Axis::Automatic;
    bool rightToLeft = false;
    QVector<CellRange> ranges;       // the cells it is for (the `sqref` of its `conditionalFormatting`)
    QString extId;                   // the id that ties a data bar to its extension (x14), while the sheet is read
};

// What a filter asks of the cells of one column (a `filterColumn`): the values that are let through, or one or two conditions, or the top or bottom of the values, or a kind of date or
// the values above or below the average, or a color.
struct FilterColumn {
    // A group of dates a list of values lets through: all those in a year, a month, a day, ... (the parts that are `has` are the group).
    struct DateGroup {
        enum class Unit : quint8 { Year, Month, Day, Hour, Minute, Second } unit = Unit::Day;
        int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
        bool operator==(const DateGroup &o) const { return unit == o.unit && year == o.year && month == o.month && day == o.day && hour == o.hour && minute == o.minute && second == o.second; }
    };
    enum class Op : quint8 { Equal, NotEqual, Greater, GreaterEqual, Less, LessEqual };
    struct Condition {
        Op op = Op::Equal;
        QString value;      // as the file writes it; the characters ? and * mean any one and any run, as in Find
        bool operator==(const Condition &o) const { return op == o.op && value == o.value; }
    };
    enum class Kind : quint8 { None, Values, Custom, Top10, Dynamic, Color } kind = Kind::None;
    quint32 column = 0;         // the column, from the first of the filter's range (`colId`)
    bool hideButton = false;    // the column has no drop-down button (`hiddenButton`)
    // Values: what is let through (the text the cells show), whether empty cells are, and groups of dates.
    QStringList values;
    bool blank = false;
    QVector<DateGroup> dates;
    // Custom: one condition, or two joined by and (else or).
    QVector<Condition> conditions;
    bool both = false;
    // Top10: the top (else the bottom) of the values, that many (`val`), or that many percent, and the number that is the limit (`filterVal`, as the file wrote it when it saved).
    bool top = true, percent = false;
    double count = 10;
    double limit = 0;
    bool hasLimit = false;
    // Dynamic: the kind ("aboveAverage", "today", "M3", "Q1", ...), and the numbers the file wrote (`val` and `maxVal`).
    QString dynamic;
    double dynVal = 0, dynMax = 0;
    bool hasDynVal = false;
    // Color: the color the cells have (the fill, or with `font` the text), as the file's differential format `dxf` says (-1 when it is a color the file has no format for).
    ColorSpec color;
    bool fontColor = false;
    qint32 dxf = -1;
    bool operator==(const FilterColumn &o) const
    {
        return kind == o.kind && column == o.column && hideButton == o.hideButton && values == o.values && blank == o.blank && dates == o.dates && conditions == o.conditions && both == o.both
               && top == o.top && percent == o.percent && count == o.count && limit == o.limit && hasLimit == o.hasLimit && dynamic == o.dynamic && dynVal == o.dynVal && dynMax == o.dynMax
               && hasDynVal == o.hasDynVal && color == o.color && fontColor == o.fontColor && dxf == o.dxf;
    }
};

// A filter on a range of cells (`autoFilter`): the range, whose first row has the buttons, and the conditions of its columns.
struct AutoFilter {
    CellRange ref;
    QVector<FilterColumn> columns;     // those with a condition or no button, in the order of the file
    QString sortState;                 // the file's `sortState` inside the filter, as it is (written again unchanged)
    bool valid() const { return ref.valid; }
    bool active() const { return ref.valid && !conditionColumns().isEmpty(); }
    // The columns (from the first of the range) that have a condition.
    QVector<quint32> conditionColumns() const
    {
        QVector<quint32> out;
        for (const FilterColumn &c : columns)
            if (c.kind != FilterColumn::Kind::None) out.append(c.column);
        return out;
    }
    const FilterColumn *columnAt(quint32 column) const
    {
        for (const FilterColumn &c : columns)
            if (c.column == column) return &c;
        return nullptr;
    }
    bool operator==(const AutoFilter &o) const { return ref == o.ref && columns == o.columns && sortState == o.sortState; }
};

struct Sheet {
    CellStore cells;
    QVector<QString> texts;           // inline strings and the texts of formula results
    QHash<qint32, RichRuns> richTexts;   // the runs of the inline strings that have more than one font (by the place in `texts`; only a workbook of the older binary form has them)
    QVector<FontSpec> richFonts;         // the fonts the runs of `richTexts` name
    QVector<QString> formulaTexts;    // as written, and no "=" in front
    QVector<Formula> formulas;
    // A workbook that holds its formulas in the id frame (sheet/formulaframe.h; CalcOptions::idFrame) gives each of its sheets the workbook it is in (`frame`) and its own place there: the texts name
    // rows and columns by ids, which formulaText turns into places. Of the texts, `positionalTexts` are those held in places (a reference through several sheets, a name that could be read as a
    // cell past XFD) and `wideTexts` those that have ids past the grid in them. Empty for every other sheet.
    const FrameSheets *frame = nullptr;
    int frameIndex = -1;
    QSet<qint32> positionalTexts, wideTexts;
    QVector<CellRange> ranges;        // the cells of array formulas and data tables
    CellRange dimension;              // what the part says the used cells are (it may be wrong, or missing)
    qint64 mergeCount = 0;            // merged ranges
    QVector<CellRange> merges;        // the merged ranges read (as many as the limit allows), in the order of the file
    QVector<ColumnSpan> columns;      // in the order of the file
    QVector<RowSpec> rowSpecs;        // the rows that are not as the default, in the order of the file
    AutoFilter autoFilter;            // the sheet's filter (not valid when it has none)
    QVector<Validation> validations;  // the rules for what may be put in cells
    QVector<CondRule> conditionalFormats;   // the rules of conditional formatting, by priority
    SheetFormat format;
    SheetView view;
    PageSetup page;                   // how it prints: the paper, margins, header and footer, breaks, and the print area and titles (see sheet/pagesetup.h)
    ColorSpec tabColor;               // the color of the sheet's tab
    bool transitionEvaluation = false; // sheetPr transitionEvaluation (the old rules of formula evaluation: see EvalContext::transition)
    bool transitionEntry = false;      // sheetPr transitionEntry (kept; no behavior)
    bool isProtected = false;         // the sheet is protected (sheetProtection): its locked cells can't be changed
    SheetProtection protection;       // and what the protection says (isProtected is its `enabled`)
    QVector<LinkSpec> links;          // the links on cells, in the order of the file (an address outside the workbook is resolved by Workbook::readSheet; a sheet read alone has the relationship's id)
    QStringList notKept;              // what the sheet's file has that the program neither shows nor writes into a copy of the sheet, in words (sparklines, say)
    bool partial = false;             // the read was stopped at a row asked for (ReadOptions::stopAfterRow): the cells of the rows after it, and what follows the cells (the merged ranges), are not here
    quint32 extentRows = 0;           // the rows and columns the cells reach: the most of a cell that holds a value or a formula or has a format of its own (a style other than the first)
    quint32 extentCols = 0;
    qint64 formulaCells = 0;          // cells with a formula
    qint64 storedResults = 0;         // of them, those the file stored a result for
    SheetStats stats;
    // Empty when the whole part was read. Else the cells read before the
    // problem are here and this says what the problem was (the XML is damaged,
    // or something was past a limit).
    QString error;

    // The order of the sheet's rows and of its columns (as ids: calc/linemap.h); the cells are held by those ids.
    const jp::calc::RowMap &rows() const { return cells.rows(); }
    const jp::calc::ColMap &cols() const { return cells.cols(); }
    // The cell's text: that of a shared, inline, or formula string; for any
    // other the empty string.
    QString text(const Cell &cell, const SharedStrings &shared) const;
    // The formula of the cell as written for it: a shared formula as the cell
    // has it, relative references moved to where it is. Empty for a cell with
    // no formula.
    QString formulaText(const Cell &cell) const;
    // The cells an array formula (or a data table) covers; not valid for any other cell.
    CellRange formulaRange(const Cell &cell) const;
};

} // namespace jp::office::sheet
