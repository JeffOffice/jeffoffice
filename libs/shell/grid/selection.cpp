#include "grid/selection.h"

#include <algorithm>

namespace jp::grid {

NavSource::~NavSource() = default;

namespace {
qint64 clampTo(qint64 v, qint64 low, qint64 high) { return std::max(low, std::min(v, high)); }
} // namespace

// ---- the shown rows and columns

qint64 Navigator::shownRow(qint64 row, int direction) const
{
    const Axis &a = m_layout->rows;
    row = clampTo(row, 0, a.count() - 1);
    if (!a.isHidden(row)) return row;
    if (direction >= 0) {
        const qint64 next = a.nextVisible(row);
        return next < a.count() ? next : a.lastVisible();
    }
    const qint64 prev = a.previousVisible(row);
    return prev >= 0 ? prev : a.firstVisible();
}

qint64 Navigator::shownCol(qint64 col, int direction) const
{
    const Axis &a = m_layout->cols;
    col = clampTo(col, 0, a.count() - 1);
    if (!a.isHidden(col)) return col;
    if (direction >= 0) {
        const qint64 next = a.nextVisible(col);
        return next < a.count() ? next : a.lastVisible();
    }
    const qint64 prev = a.previousVisible(col);
    return prev >= 0 ? prev : a.firstVisible();
}

// ---- selections

GridRange Navigator::expanded(GridRange r) const
{
    // (a range that takes in a merged range takes in the ranges that cross that one's new edges too, a few times round at the most)
    for (int round = 0; round < 8; ++round) {
        std::vector<int> found;
        const bool whole = m_layout->merges.crossing(quint32(r.row1), quint32(r.col1), quint32(r.row2), quint32(r.col2), 2000, &found);
        GridRange next = r;
        for (int i : found) {
            const jp::office::sheet::CellRange &m = m_layout->merges.range(i);
            next.row1 = std::min<qint64>(next.row1, m.row1);
            next.col1 = std::min<qint64>(next.col1, m.col1);
            next.row2 = std::max<qint64>(next.row2, m.row2);
            next.col2 = std::max<qint64>(next.col2, m.col2);
        }
        if (next == r || !whole) return next;
        r = next;
    }
    return r;
}

Selection Navigator::single(qint64 row, qint64 col) const
{
    Selection s;
    row = clampTo(row, 0, m_layout->rows.count() - 1);
    col = clampTo(col, 0, m_layout->cols.count() - 1);
    s.range.row1 = s.range.row2 = row;
    s.range.col1 = s.range.col2 = col;
    const int m = m_layout->merges.at(quint32(row), quint32(col));
    if (m >= 0) {
        const jp::office::sheet::CellRange &r = m_layout->merges.range(m);
        row = r.row1;
        col = r.col1;
        s.range.row1 = r.row1;
        s.range.col1 = r.col1;
        s.range.row2 = r.row2;
        s.range.col2 = r.col2;
    }
    s.activeRow = s.headRow = row;
    s.activeCol = s.headCol = col;
    return s;
}

Selection Navigator::click(qint64 row, qint64 col) const { return single(row, col); }

Selection Navigator::extendTo(const Selection &s, qint64 row, qint64 col) const
{
    Selection out = s;
    out.whole = Selection::Whole::None;
    out.headRow = clampTo(row, 0, m_layout->rows.count() - 1);
    out.headCol = clampTo(col, 0, m_layout->cols.count() - 1);
    GridRange r;
    r.row1 = std::min(out.activeRow, out.headRow);
    r.row2 = std::max(out.activeRow, out.headRow);
    r.col1 = std::min(out.activeCol, out.headCol);
    r.col2 = std::max(out.activeCol, out.headCol);
    out.range = expanded(r);
    return out;
}

Selection Navigator::selectRows(qint64 from, qint64 to) const
{
    Selection s;
    from = clampTo(from, 0, m_layout->rows.count() - 1);
    to = clampTo(to, 0, m_layout->rows.count() - 1);
    s.whole = Selection::Whole::Rows;
    s.range.row1 = std::min(from, to);
    s.range.row2 = std::max(from, to);
    s.range.col1 = 0;
    s.range.col2 = m_layout->cols.count() - 1;
    s.activeRow = from;
    s.headRow = to;
    s.activeCol = s.headCol = shownCol(0, 1);
    return s;
}

Selection Navigator::selectColumns(qint64 from, qint64 to) const
{
    Selection s;
    from = clampTo(from, 0, m_layout->cols.count() - 1);
    to = clampTo(to, 0, m_layout->cols.count() - 1);
    s.whole = Selection::Whole::Columns;
    s.range.col1 = std::min(from, to);
    s.range.col2 = std::max(from, to);
    s.range.row1 = 0;
    s.range.row2 = m_layout->rows.count() - 1;
    s.activeCol = from;
    s.headCol = to;
    s.activeRow = s.headRow = shownRow(0, 1);
    return s;
}

Selection Navigator::extendRows(const Selection &s, qint64 to) const { return selectRows(s.activeRow, to); }
Selection Navigator::extendColumns(const Selection &s, qint64 to) const { return selectColumns(s.activeCol, to); }

Selection Navigator::selectAll(const Selection &s) const
{
    Selection out = s;
    out.whole = Selection::Whole::All;
    out.range = GridRange{0, 0, m_layout->rows.count() - 1, m_layout->cols.count() - 1};
    out.headRow = out.range.row2;
    out.headCol = out.range.col2;
    return out;
}

Selection Navigator::selectWholeRows(const Selection &s) const
{
    Selection out = s;
    out.whole = Selection::Whole::Rows;
    out.range.col1 = 0;
    out.range.col2 = m_layout->cols.count() - 1;
    return out;
}

Selection Navigator::selectWholeColumns(const Selection &s) const
{
    Selection out = s;
    out.whole = Selection::Whole::Columns;
    out.range.row1 = 0;
    out.range.row2 = m_layout->rows.count() - 1;
    return out;
}

// ---- the keys

namespace {

// The rows (or columns) a cell, or the merged range that holds it, takes.
struct Span {
    qint64 row1, col1, row2, col2;
};

} // namespace

Selection Navigator::move(const Selection &s, int dRow, int dCol, bool extend) const
{
    const qint64 fromRow = extend ? s.headRow : s.activeRow, fromCol = extend ? s.headCol : s.activeCol;
    Span at{fromRow, fromCol, fromRow, fromCol};
    const int m = m_layout->merges.at(quint32(fromRow), quint32(fromCol));
    if (m >= 0) {
        const jp::office::sheet::CellRange &r = m_layout->merges.range(m);
        at = Span{r.row1, r.col1, r.row2, r.col2};
    }
    qint64 row = fromRow, col = fromCol;
    const Axis &rows = m_layout->rows, &cols = m_layout->cols;
    if (dRow > 0) {
        const qint64 next = rows.nextVisible(at.row2);
        if (next < rows.count()) row = next;
    } else if (dRow < 0) {
        const qint64 prev = rows.previousVisible(at.row1);
        if (prev >= 0) row = prev;
    }
    if (dCol > 0) {
        const qint64 next = cols.nextVisible(at.col2);
        if (next < cols.count()) col = next;
    } else if (dCol < 0) {
        const qint64 prev = cols.previousVisible(at.col1);
        if (prev >= 0) col = prev;
    }
    return extend ? extendTo(s, row, col) : single(row, col);
}

Selection Navigator::arrive(const Selection &s, qint64 row, qint64 col, bool extend) const { return extend ? extendTo(s, row, col) : single(row, col); }

Selection Navigator::home(const Selection &s, bool ctrl, bool extend) const
{
    const qint64 row = ctrl ? shownRow(m_layout->frozenRows(), 1) : extend ? s.headRow : s.activeRow;
    const qint64 col = ctrl ? shownCol(m_layout->frozenCols(), 1) : shownCol(0, 1);
    return arrive(s, row, col, extend);
}

Selection Navigator::end(const Selection &s, bool extend) const
{
    qint64 rows = 0, cols = 0;
    m_source->usedExtent(&rows, &cols);
    const qint64 row = rows > 0 ? shownRow(rows - 1, -1) : shownRow(0, 1), col = cols > 0 ? shownCol(cols - 1, -1) : shownCol(0, 1);
    return arrive(s, row, col, extend);
}

Selection Navigator::page(const Selection &s, int direction, int rows, bool extend) const
{
    qint64 row = extend ? s.headRow : s.activeRow;
    const Axis &a = m_layout->rows;
    for (int i = 0; i < std::min(rows, 100000); ++i) {
        const qint64 next = direction > 0 ? a.nextVisible(row) : a.previousVisible(row);
        if (next < 0 || next >= a.count()) break;
        row = next;
    }
    return arrive(s, row, extend ? s.headCol : s.activeCol, extend);
}

Selection Navigator::tab(const Selection &s, bool back) const
{
    if (s.isOneCell() || s.whole == Selection::Whole::All) return move(s, 0, back ? -1 : 1, false);
    const GridRange &r = s.range;
    const Axis &rows = m_layout->rows, &cols = m_layout->cols;
    qint64 row = s.activeRow, col = s.activeCol;
    const int m = m_layout->merges.at(quint32(row), quint32(col));
    qint64 colEnd = col, colStart = col;
    if (m >= 0) {
        colEnd = m_layout->merges.range(m).col2;
        colStart = m_layout->merges.range(m).col1;
    }
    Selection out = s;
    if (!back) {
        qint64 next = cols.nextVisible(colEnd);
        if (next <= r.col2 && next < cols.count()) {
            col = next;
        } else {
            col = shownCol(r.col1, 1);
            const qint64 nextRow = rows.nextVisible(row);
            row = nextRow <= r.row2 && nextRow < rows.count() ? nextRow : shownRow(r.row1, 1);
        }
    } else {
        const qint64 prev = cols.previousVisible(colStart);
        if (prev >= r.col1) {
            col = prev;
        } else {
            col = shownCol(r.col2, -1);
            const qint64 prevRow = rows.previousVisible(row);
            row = prevRow >= r.row1 ? prevRow : shownRow(r.row2, -1);
        }
    }
    const int target = m_layout->merges.at(quint32(row), quint32(col));
    if (target >= 0) {
        row = m_layout->merges.range(target).row1;
        col = m_layout->merges.range(target).col1;
    }
    out.activeRow = row;
    out.activeCol = col;
    return out;
}

Selection Navigator::enter(const Selection &s, bool up) const
{
    if (s.isOneCell() || s.whole == Selection::Whole::All) return move(s, up ? -1 : 1, 0, false);
    const GridRange &r = s.range;
    const Axis &rows = m_layout->rows, &cols = m_layout->cols;
    qint64 row = s.activeRow, col = s.activeCol;
    const int m = m_layout->merges.at(quint32(row), quint32(col));
    qint64 rowEnd = row, rowStart = row;
    if (m >= 0) {
        rowEnd = m_layout->merges.range(m).row2;
        rowStart = m_layout->merges.range(m).row1;
    }
    Selection out = s;
    if (!up) {
        const qint64 next = rows.nextVisible(rowEnd);
        if (next <= r.row2 && next < rows.count()) {
            row = next;
        } else {
            row = shownRow(r.row1, 1);
            const qint64 nextCol = cols.nextVisible(col);
            col = nextCol <= r.col2 && nextCol < cols.count() ? nextCol : shownCol(r.col1, 1);
        }
    } else {
        const qint64 prev = rows.previousVisible(rowStart);
        if (prev >= r.row1) {
            row = prev;
        } else {
            row = shownRow(r.row2, -1);
            const qint64 prevCol = cols.previousVisible(col);
            col = prevCol >= r.col1 ? prevCol : shownCol(r.col2, -1);
        }
    }
    const int target = m_layout->merges.at(quint32(row), quint32(col));
    if (target >= 0) {
        row = m_layout->merges.range(target).row1;
        col = m_layout->merges.range(target).col1;
    }
    out.activeRow = row;
    out.activeCol = col;
    return out;
}

// ---- Ctrl and an arrow

JumpWalk Navigator::jump(const Selection &s, int dRow, int dCol, bool extend) const
{
    qint64 row = extend ? s.headRow : s.activeRow, col = extend ? s.headCol : s.activeCol;
    const int m = m_layout->merges.at(quint32(row), quint32(col));
    if (m >= 0) {
        const jp::office::sheet::CellRange &r = m_layout->merges.range(m);
        // (a merged range is one cell, left from its edge)
        if (dRow > 0) row = r.row2;
        else if (dRow < 0) row = r.row1;
        if (dCol > 0) col = r.col2;
        else if (dCol < 0) col = r.col1;
    }
    return JumpWalk(m_layout, m_source, row, col, dRow, dCol);
}

JumpWalk::JumpWalk(const GridLayout *layout, const NavSource *source, qint64 row, qint64 col, int dRow, int dCol)
    : m_layout(layout), m_source(source), m_row(row), m_col(col), m_dRow(dRow), m_dCol(dCol)
{
}

bool JumpWalk::step(qint64 budget)
{
    const Axis &rows = m_layout->rows, &cols = m_layout->cols;
    const NavSource &src = *m_source;
    const qint64 start = m_looks;
    // The next shown cell in the direction (false at the edge of the sheet).
    auto next = [&](qint64 *row, qint64 *col) {
        ++m_looks;
        if (m_dRow != 0) {
            const qint64 n = m_dRow > 0 ? rows.nextVisible(*row) : rows.previousVisible(*row);
            if (n < 0 || n >= rows.count()) return false;
            *row = n;
        } else {
            const qint64 n = m_dCol > 0 ? cols.nextVisible(*col) : cols.previousVisible(*col);
            if (n < 0 || n >= cols.count()) return false;
            *col = n;
        }
        return true;
    };
    if (m_dRow == 0 && m_dCol == 0) m_phase = Phase::Done;
    if (m_phase == Phase::Start) {
        qint64 r = m_row, c = m_col;
        if (!next(&r, &c)) {
            m_phase = Phase::Done;
        } else {
            ++m_looks;
            const bool here = src.hasValue(m_row, m_col);
            ++m_looks;
            m_phase = here && src.hasValue(r, c) ? Phase::Run : Phase::Seek;
        }
    }
    while (m_phase == Phase::Run || m_phase == Phase::Seek) {
        if (budget > 0 && m_looks - start >= budget) return false;
        qint64 r = m_row, c = m_col;
        if (!next(&r, &c)) {
            // the edge: a run ends on its last cell, and a search that found nothing is at the edge
            m_phase = Phase::Done;
            break;
        }
        ++m_looks;
        const bool has = src.hasValue(r, c);
        if (m_phase == Phase::Run) {
            if (!has) {
                m_phase = Phase::Done;
                break;
            }
            m_row = r;
            m_col = c;
        } else {
            m_row = r;
            m_col = c;
            if (has) m_phase = Phase::Done;
        }
    }
    return m_phase == Phase::Done;
}

} // namespace jp::grid
