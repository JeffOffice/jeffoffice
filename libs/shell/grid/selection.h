#pragma once
// What is selected in a sheet, and the keys and clicks that change it. A selection is the cell with the cursor (the active cell, the first cell of a merged range) and a range of
// cells that grows from it; whole rows or whole columns are a range of all the columns or rows. The Navigator moves it as people expect: the arrow keys over hidden rows and
// whole merged ranges, Ctrl with an arrow to the edge of the data, Home, Ctrl+Home, Ctrl+End, Page Up and Down, Tab and Enter through the selection, and clicks, drags, and
// shift-clicks, with a merged range always kept whole. Looking for the edge of the data in a sheet of a million rows can take longer than a screen may wait, so it is a walk
// that does a given work at a time (JumpWalk) and goes on later.

#include "grid/layout.h"

namespace jp::grid {

struct GridRange {
    qint64 row1 = 0, col1 = 0, row2 = 0, col2 = 0;
    bool contains(qint64 row, qint64 col) const { return row >= row1 && row <= row2 && col >= col1 && col <= col2; }
    qint64 rows() const { return row2 - row1 + 1; }
    qint64 cols() const { return col2 - col1 + 1; }
    bool operator==(const GridRange &o) const { return row1 == o.row1 && col1 == o.col1 && row2 == o.row2 && col2 == o.col2; }
};

struct Selection {
    enum class Whole { None, Rows, Columns, All };
    qint64 activeRow = 0, activeCol = 0;   // the cell with the cursor
    qint64 headRow = 0, headCol = 0;       // the corner that moves when the selection is extended (the active cell is the other)
    Whole whole = Whole::None;             // whole rows or columns or the whole sheet: the range reaches every column or row
    GridRange range;                       // what is selected; of one cell (or the merged range it is in) when only the active cell is

    bool isOneCell() const { return range.row1 == range.row2 && range.col1 == range.col2; }
    bool operator==(const Selection &o) const
    {
        return activeRow == o.activeRow && activeCol == o.activeCol && headRow == o.headRow && headCol == o.headCol && whole == o.whole && range == o.range;
    }
};

// What the navigator asks of the sheet's cells.
class NavSource {
public:
    virtual ~NavSource();
    // The cell has something in it (a value, or a formula).
    virtual bool hasValue(qint64 row, qint64 col) const = 0;
    // The rows and columns the cells reach (what Ctrl+End goes to the end of).
    virtual void usedExtent(qint64 *rows, qint64 *cols) const = 0;
};

class Navigator;

// A search for the edge of the data, a little at a time. step() does at most about `budget` cell looks and says whether it is done; the cell it stands on is where it is.
class JumpWalk {
public:
    JumpWalk(const Navigator *nav, qint64 row, qint64 col, int dRow, int dCol);
    // True when the walk has found its place. A budget of 0 is no limit.
    bool step(qint64 budget);
    qint64 row() const { return m_row; }
    qint64 col() const { return m_col; }
    qint64 looks() const { return m_looks; }

private:
    const Navigator *m_nav;
    qint64 m_row, m_col;
    int m_dRow, m_dCol;
    enum class Phase { Start, Run, Seek, Done } m_phase = Phase::Start;
    qint64 m_looks = 0;
};

class Navigator {
public:
    Navigator(const GridLayout *layout, const NavSource *source) : m_layout(layout), m_source(source) {}

    // A click: the cell (the first of a merged range) alone. A drag, or a click with Shift: the range from the active cell to the cell, kept whole round merged ranges.
    Selection click(qint64 row, qint64 col) const;
    Selection extendTo(const Selection &s, qint64 row, qint64 col) const;
    // The rows (or columns) from one to another, whole; the sheet; the whole rows or columns the selection touches.
    Selection selectRows(qint64 from, qint64 to) const;
    Selection selectColumns(qint64 from, qint64 to) const;
    Selection selectAll(const Selection &s) const;
    Selection selectWholeRows(const Selection &s) const;      // Shift+Space
    Selection selectWholeColumns(const Selection &s) const;   // Ctrl+Space
    // The headers' drags and shift-clicks: from the rows (columns) the selection began at to this one.
    Selection extendRows(const Selection &s, qint64 to) const;
    Selection extendColumns(const Selection &s, qint64 to) const;

    // An arrow key: one cell, over hidden rows and columns and whole merged ranges; with `extend`, the selection's moving corner moves and the active cell stays.
    Selection move(const Selection &s, int dRow, int dCol, bool extend) const;
    // The cell Ctrl and an arrow go to (the walk, to be stepped), and the selection once it has: the cursor there, or extended to there. A walk to extend goes from the
    // moving corner; a merged range is one cell, left from its edge.
    JumpWalk jump(const Selection &s, int dRow, int dCol, bool extend = false) const;
    Selection arrive(const Selection &s, qint64 row, qint64 col, bool extend) const;
    // Home (the first column of the row), Ctrl+Home (the first cell that scrolls, or A1), and Ctrl+End (the last cell of the data). `extend` keeps the active cell.
    Selection home(const Selection &s, bool ctrl, bool extend) const;
    Selection end(const Selection &s, bool extend) const;
    // Page Down (direction 1) or Up (-1): `rows` shown rows at a time.
    Selection page(const Selection &s, int direction, int rows, bool extend) const;
    // Tab and Shift+Tab (to the right and left) and Enter and Shift+Enter (down and up): through the cells of the selection when there are more than one, else one cell.
    Selection tab(const Selection &s, bool back) const;
    Selection enter(const Selection &s, bool up) const;

    // The range, made larger until no merged range is half in it.
    GridRange expanded(GridRange range) const;

    // The nearest shown row or column to this one in a direction (the place itself if it is shown), kept inside the sheet: the first or last shown one past the end.
    qint64 shownRow(qint64 row, int direction) const;
    qint64 shownCol(qint64 col, int direction) const;
    const GridLayout &layout() const { return *m_layout; }
    const NavSource &source() const { return *m_source; }

private:
    friend class JumpWalk;
    Selection single(qint64 row, qint64 col) const;
    void setRange(Selection *s) const;
    const GridLayout *m_layout;
    const NavSource *m_source;
};

} // namespace jp::grid
