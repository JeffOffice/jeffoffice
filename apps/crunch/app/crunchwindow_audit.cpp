// The Formulas tab, the parts about checking formulas: arrows from what a formula reads to the formula, and from a cell to the formulas that read it, one step more with each click; and the
// outlines on cells in circles. The arrows are drawn by the grid (GridView::setCheckMarks); here is what they are.
#include "app/crunchwindow.h"

#include <QAction>

#include <algorithm>

namespace jp {

using jp::office::sheet::CellRange;

void CrunchWindow::createAuditActions()
{
    mk("formulas.tracePrecedents", tr("Show What Feeds This Cell"), "arrow-down-to-dot", QKeySequence(), [this] { tracePrecedents(); }, false, false);
    mk("formulas.traceDependents", tr("Show What Uses This Cell"), "arrow-up-from-dot", QKeySequence(), [this] { traceDependents(); }, false, false);
    mk("formulas.removeArrows", tr("Clear Arrows"), "eraser", QKeySequence(), [this] { removeArrows(); }, false, false);
    for (const char *id : {"formulas.tracePrecedents", "formulas.traceDependents", "formulas.removeArrows"}) m_opsActions << QString::fromLatin1(id);
    updateEditActions();
}

namespace {

// The cells of the blocks that hold a formula, in order, at most `cap` of them (and the blocks are looked at only up to a bound).
void formulaCellsIn(const jp::office::sheet::Calculation &calc, const crunch::TraceBlock &b, size_t cap, std::vector<crunch::TraceCell> *out)
{
    if (b.sheet < 0 || b.sheet >= calc.sheetCount()) return;
    const jp::office::sheet::Sheet &s = calc.sheet(b.sheet);
    qint64 looked = 0;
    s.cells.forEachInRange(b.cells.row1, b.cells.col1, b.cells.row2, b.cells.col2, [&](const jp::office::sheet::Cell &c) {
        if (c.hasFormula() && out->size() < cap) out->push_back({b.sheet, c.row, quint32(c.col)});
        return out->size() < cap && ++looked < 20000;
    });
}

} // namespace

// The first click draws the arrows of the cell with the cursor; each click after it, of the formulas those came from, on and on.
bool CrunchWindow::startTrace(bool up)
{
    if (!m_session || !m_loaded || !commitEdit()) return false;
    const quint32 row = quint32(std::max<qint64>(0, m_view->selection().activeRow)), col = quint32(std::max<qint64>(0, m_view->selection().activeCol));
    // another cell: the arrows begin again from it
    if (!m_traceRoot.set || m_traceRoot.sheet != m_sheet || m_traceRoot.row != row || m_traceRoot.col != col) {
        m_trace.clear();
        m_traceUp.clear();
        m_traceDown.clear();
        m_tracedUp = m_tracedDown = false;
        m_traceRoot = {true, m_sheet, row, col};
    }
    std::vector<crunch::TraceCell> &front = up ? m_traceUp : m_traceDown;
    bool &started = up ? m_tracedUp : m_tracedDown;
    if (!started) {
        front = {{m_sheet, row, col}};
        started = true;
    }
    return true;
}

bool CrunchWindow::tracePrecedents()
{
    if (!startTrace(true)) return false;
    const jp::office::sheet::Calculation *calc = m_session->calculation();
    const size_t before = m_trace.size();
    std::vector<crunch::TraceCell> next;
    bool truncated = false;
    for (const crunch::TraceCell &f : m_traceUp) {
        if (m_trace.size() >= grid::GridView::kMaxCheckArrows) {
            truncated = true;
            break;   // (the arrows there is room for are drawn: the rest is not looked for)
        }
        bool cut = false;
        const std::vector<crunch::TraceBlock> blocks = crunch::cellsRead(*calc, f.sheet, f.row, f.col, &cut);
        truncated = truncated || cut;
        for (const crunch::TraceBlock &b : blocks) {
            if (m_trace.size() >= grid::GridView::kMaxCheckArrows) {
                truncated = true;
                break;
            }
            TraceArrow a;
            a.fromSheet = b.sheet;
            a.from = b.cells;
            a.toSheet = f.sheet;
            a.toRow = f.row;
            a.toCol = f.col;
            const bool dup = std::any_of(m_trace.begin(), m_trace.end(), [&](const TraceArrow &t) {
                return t.fromSheet == a.fromSheet && t.from == a.from && t.toSheet == a.toSheet && t.toRow == a.toRow && t.toCol == a.toCol;
            });
            if (!dup) m_trace.push_back(a);
            formulaCellsIn(*calc, b, 200, &next);
        }
    }
    m_traceUp = next;
    if (m_trace.size() == before) {
        setStatus(m_trace.empty() ? tr("The formula in this cell does not read any cell, or this cell has no formula.") : tr("There is nothing further back."));
        refreshCheckMarks();
        return false;
    }
    setStatus(truncated ? tr("Only the first %n arrow(s) are drawn.", nullptr, int(m_trace.size())) : tr("%n arrow(s) drawn.", nullptr, int(m_trace.size())));
    refreshCheckMarks();
    return true;
}

bool CrunchWindow::traceDependents()
{
    if (!startTrace(false)) return false;
    const jp::office::sheet::Calculation *calc = m_session->calculation();
    const size_t before = m_trace.size();
    std::vector<crunch::TraceCell> next;
    qint64 notDrawn = 0;
    for (const crunch::TraceCell &f : m_traceDown) {
        if (m_trace.size() >= grid::GridView::kMaxCheckArrows) {
            ++notDrawn;
            break;   // (the arrows there is room for are drawn: the rest is not looked for)
        }
        qint64 total = 0;
        const std::vector<crunch::TraceCell> readers = crunch::readersOf(*calc, f.sheet, f.row, f.col, crunch::kMaxTraceReaders, &total);
        notDrawn += std::max<qint64>(0, total - qint64(readers.size()));
        for (const crunch::TraceCell &d : readers) {
            if (m_trace.size() >= grid::GridView::kMaxCheckArrows) {
                ++notDrawn;
                break;
            }
            TraceArrow a;
            a.fromSheet = f.sheet;
            a.from.valid = true;
            a.from.row1 = a.from.row2 = f.row;
            a.from.col1 = a.from.col2 = f.col;
            a.toSheet = d.sheet;
            a.toRow = d.row;
            a.toCol = d.col;
            const bool dup = std::any_of(m_trace.begin(), m_trace.end(), [&](const TraceArrow &t) {
                return t.fromSheet == a.fromSheet && t.from == a.from && t.toSheet == a.toSheet && t.toRow == a.toRow && t.toCol == a.toCol;
            });
            if (!dup) m_trace.push_back(a);
            if (next.size() < 200) next.push_back(d);
        }
    }
    m_traceDown = next;
    if (m_trace.size() == before) {
        setStatus(m_trace.empty() ? tr("No formula reads this cell.") : tr("There is nothing further on."));
        refreshCheckMarks();
        return false;
    }
    setStatus(notDrawn > 0 ? tr("%n arrow(s) drawn. %1 more are not drawn: too many.", nullptr, int(m_trace.size())).arg(notDrawn) : tr("%n arrow(s) drawn.", nullptr, int(m_trace.size())));
    refreshCheckMarks();
    return true;
}

void CrunchWindow::removeArrows()
{
    if (m_trace.empty() && !m_traceRoot.set) return;
    m_trace.clear();
    m_traceUp.clear();
    m_traceDown.clear();
    m_tracedUp = m_tracedDown = false;
    m_traceRoot = TraceRoot();
    refreshCheckMarks();
}

// The outlines of the cells in circles and the arrows, for the sheet in view: an end of an arrow on another sheet is a sheet icon at this one.
void CrunchWindow::refreshCheckMarks()
{
    std::vector<grid::GridRange> circles;
    for (const auto &c : m_circleCells)
        if (c.sheet == m_sheet) circles.push_back(grid::GridRange{qint64(c.row), qint64(c.col), qint64(c.row), qint64(c.col)});
    std::vector<grid::GridView::CheckArrow> arrows;
    for (const TraceArrow &t : m_trace) {
        const bool fromHere = t.fromSheet == m_sheet, toHere = t.toSheet == m_sheet;
        if (!fromHere && !toHere) continue;
        grid::GridView::CheckArrow a;
        a.from = grid::GridRange{qint64(t.from.row1), qint64(t.from.col1), qint64(t.from.row2), qint64(t.from.col2)};
        a.toRow = t.toRow;
        a.toCol = t.toCol;
        a.fromOtherSheet = !fromHere;
        a.toOtherSheet = !toHere;
        arrows.push_back(a);
    }
    m_view->setCheckMarks(std::move(circles), std::move(arrows));
}

} // namespace jp
