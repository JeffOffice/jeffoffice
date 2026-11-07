// The Formulas tab, the parts about checking formulas: arrows from what a formula reads to the formula, and from a cell to the formulas that read it, one step more with each click; and the
// outlines on cells in circles. The arrows are drawn by the grid (GridView::setCheckMarks); here is what they are.
#include "app/crunchwindow.h"
#include "core/gotospecial.h"

#include <QAction>

#include <algorithm>

namespace jp {

using jp::office::sheet::CellRange;

void CrunchWindow::createAuditActions()
{
    mk("formulas.tracePrecedents", tr("Show What Feeds This Cell"), "arrow-down-to-dot", QKeySequence(), [this] { tracePrecedents(); }, false, false);
    mk("formulas.traceDependents", tr("Show What Uses This Cell"), "arrow-up-from-dot", QKeySequence(), [this] { traceDependents(); }, false, false);
    mk("formulas.traceError", tr("Trace Error"), "triangle-alert", QKeySequence(), [this] { traceError(); }, false, false);
    mk("formulas.removeArrows", tr("Clear Arrows"), "eraser", QKeySequence(), [this] { removeArrows(); }, false, false);
    mk("formulas.nextError", tr("Next Error"), "arrow-big-down", QKeySequence(), [this] { stepError(true); }, false, false);
    mk("formulas.previousError", tr("Previous Error"), "arrow-big-up", QKeySequence(), [this] { stepError(false); }, false, false);
    for (const char *id : {"formulas.tracePrecedents", "formulas.traceDependents", "formulas.traceError", "formulas.removeArrows", "formulas.nextError", "formulas.previousError"}) m_opsActions << QString::fromLatin1(id);
    updateEditActions();
}

namespace {

// The cells of the blocks that hold a formula, in order, at most `cap` of them (and the blocks are looked at only up to a bound).
void formulaCellsIn(const jp::office::sheet::Calculation &calc, const crunch::TraceBlock &b, size_t cap, std::vector<crunch::TraceCell> *out)
{
    if (b.sheet < 0 || b.sheet >= calc.sheetCount()) return;
    const jp::office::sheet::Sheet &s = calc.sheet(b.sheet);
    qint64 looked = 0;
    s.cells.forEachInRange(b.cells.row1, b.cells.col1, b.cells.row2, b.cells.col2, [&](quint32 row, quint32 col, const jp::office::sheet::Cell &c) {
        if (c.hasFormula() && out->size() < cap) out->push_back({b.sheet, row, quint32(col)});
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
    if (!m_traceRoot.set || m_traceRoot.sheet != m_sheet || m_traceRoot.row != row || m_traceRoot.col != col || m_tracedError) {
        m_tracedError = false;
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
        for (size_t at = 0; at < readers.size(); ++at) {
            const crunch::TraceCell &d = readers[at];
            if (m_trace.size() >= grid::GridView::kMaxCheckArrows) {
                notDrawn += qint64(readers.size() - at);   // (the rest of this cell's readers)
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

// Trace Error: from a cell that shows an error, the arrows go to the cells it reads that are errors too, one step more with each click, until a cell reads no error: that one makes it.
bool CrunchWindow::traceError(bool quiet, bool *cut)
{
    bool fanCut = false;
    if (!m_session || !m_loaded || !commitEdit()) return false;
    const quint32 row = quint32(std::max<qint64>(0, m_view->selection().activeRow)), col = quint32(std::max<qint64>(0, m_view->selection().activeCol));
    if (!m_traceRoot.set || m_traceRoot.sheet != m_sheet || m_traceRoot.row != row || m_traceRoot.col != col || !m_tracedError) {
        m_trace.clear();
        m_traceUp.clear();
        m_traceDown.clear();
        m_tracedUp = m_tracedDown = false;
        m_traceRoot = {true, m_sheet, row, col};
        m_tracedError = true;
        if (!m_session->valueAt(m_sheet, row, col).isError()) {
            m_tracedError = false;
            m_traceRoot = TraceRoot();
            refreshCheckMarks();
            setStatus(tr("This cell is not an error."));
            return false;
        }
        m_traceUp = {{m_sheet, row, col}};
    }
    const jp::office::sheet::Calculation *calc = m_session->calculation();
    const size_t before = m_trace.size();
    std::vector<crunch::TraceCell> next;
    for (const crunch::TraceCell &f : m_traceUp) {
        if (m_trace.size() >= grid::GridView::kMaxCheckArrows) {
            fanCut = true;
            break;
        }
        bool cutRead = false;
        const std::vector<crunch::TraceBlock> blocks = crunch::cellsRead(*calc, f.sheet, f.row, f.col, &cutRead);
        fanCut = fanCut || cutRead;
        for (const crunch::TraceBlock &b : blocks) {
            if (m_trace.size() >= grid::GridView::kMaxCheckArrows) {
                fanCut = true;
                break;
            }
            std::vector<crunch::TraceCell> formulas;
            formulaCellsIn(*calc, b, 200, &formulas);
            fanCut = fanCut || formulas.size() >= 200;
            bool any = false;
            for (const crunch::TraceCell &c : formulas) {
                if (!m_session->valueAt(c.sheet, c.row, c.col).isError()) continue;
                any = true;
                if (next.size() < 200) next.push_back(c);
                else fanCut = true;
            }
            if (!any) continue;
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
        }
    }
    m_traceUp = next;
    if (cut) *cut = fanCut;
    if (quiet) return m_trace.size() != before;
    refreshCheckMarks();
    if (m_trace.size() == before) {
        setStatus(tr("No cell this one reads has an error: this cell makes the error."));
        return false;
    }
    setStatus(tr("%n arrow(s) drawn.", nullptr, int(m_trace.size())) + (fanCut ? QLatin1Char(' ') + tr("Some cells it reads were not followed: there are too many.") : QString()));
    return true;
}

// Next Error and Previous Error: the cells of the sheet that show an error are stepped through in the order of the rows (and in the row of the columns), round from the last to the first. The cell
// is selected and the arrows are drawn all the way to the cell that makes the error.
bool CrunchWindow::stepError(bool forward)
{
    if (!m_session || !m_loaded || !commitEdit()) return false;
    const grid::Selection &sel = m_view->selection();
    const qint64 row = std::max<qint64>(0, sel.activeRow), col = std::max<qint64>(0, sel.activeCol);
    // the list of cells that show an error is kept until the sheet's values change (the window drops it then): a press steps through it, and does not look at the sheet again
    if (!m_errorListValid || m_errorListSheet != m_sheet) {
        crunch::SpecialOptions options;
        options.kind = crunch::SpecialKind::Errors;
        const crunch::SpecialResult found = crunch::goToSpecial(*m_session, m_sheet, CellRange(), quint32(row), quint32(col), options);
        constexpr qint64 kMostErrors = 200000;
        m_errorCells.clear();
        for (const CellRange &r : found.ranges)
            for (qint64 y = r.row1; y <= r.row2 && qint64(m_errorCells.size()) < kMostErrors; ++y)
                for (qint64 x = r.col1; x <= r.col2 && qint64(m_errorCells.size()) < kMostErrors; ++x) m_errorCells.push_back({y, x});
        std::sort(m_errorCells.begin(), m_errorCells.end());
        m_errorListTruncated = found.truncated;
        m_errorListSheet = m_sheet;
        m_errorListValid = true;
        ++m_errorListBuilds;
    }
    const std::vector<std::pair<qint64, qint64>> &cells = m_errorCells;
    if (cells.empty()) {
        setStatus(tr("No cell on this sheet shows an error."));
        return false;
    }
    const std::pair<qint64, qint64> here{row, col};
    size_t at = 0;
    if (forward) {
        const auto it = std::upper_bound(cells.begin(), cells.end(), here);
        at = it == cells.end() ? 0 : size_t(it - cells.begin());
    } else {
        const auto it = std::lower_bound(cells.begin(), cells.end(), here);
        at = it == cells.begin() ? cells.size() - 1 : size_t(it - cells.begin()) - 1;
    }
    removeArrows();
    m_view->selectCell(cells[at].first, cells[at].second);
    m_view->ensureVisible(cells[at].first, cells[at].second);
    // the arrows to what makes it: one step more at a time until a cell reads no error, or the steps this takes run out
    constexpr int kMostSteps = 200;
    bool fanCut = false, more = true;
    int steps = 0;
    for (; steps < kMostSteps; ++steps)
        if (!traceError(true, &fanCut)) {
            more = false;
            break;
        }
    refreshCheckMarks();
    QString how;
    if (more) how = tr("The arrows stop before the cell that makes it: the chain is too long to follow.");
    else if (m_trace.empty()) how = tr("The cell makes the error itself.");
    else how = tr("The arrows go to the cell that makes it.");
    if (fanCut) how += QLatin1Char(' ') + tr("Some cells it reads were not followed: there are too many.");
    setStatus(tr("Error %1 of %2%3.").arg(int(at) + 1).arg(int(cells.size())).arg(m_errorListTruncated ? tr(" or more") : QString()) + QLatin1Char(' ') + how);
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
    m_tracedError = false;
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
    refreshCellFlags();
}

} // namespace jp
