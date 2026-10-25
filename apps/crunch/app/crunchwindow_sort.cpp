// Sort on the window: Sort A–Z and Z–A, Sort… and its dialog, and the work behind them. The keys are read from the cells a slice at a time on the window's thread, the order is
// worked out on a thread of its own from the keys alone (and can be given up with Esc), and the cells are put in order as one step on the window's thread.
#include "app/crunchwindow.h"

#include "app/crunchinfo.h"
#include "sheet/cellref.h"

#include <QAction>
#include <QThread>

namespace jp {

using jp::office::sheet::CellRange;

namespace {
// The cells a sort does without a thread (it is quicker than the thread's start).
constexpr size_t kSmallSort = 2000;
}

void CrunchWindow::createSortActions()
{
    mk("edit.sort", tr("Sort and Pick Rows"), "arrow-down-a-z", QKeySequence(), [this] { sortSelection(2); });
    mk("sort.ascending", tr("Sort A–Z"), "arrow-down-a-z", QKeySequence(), [this] { sortSelection(0); });
    mk("sort.descending", tr("Sort Z–A"), "arrow-up-z-a", QKeySequence(), [this] { sortSelection(1); });
    mk("sort.custom", tr("Sort…"), "list-ordered", QKeySequence(), [this] { sortSelection(2); });
    for (const char *id : {"edit.sort", "sort.ascending", "sort.descending", "sort.custom"}) m_opsActions << QString::fromLatin1(id);
    m_sortTimer.setInterval(0);
    connect(&m_sortTimer, &QTimer::timeout, this, [this] { stepSort(); });
    updateEditActions();
}

// The cells a sort on the selection is for: the selection, or with one cell the block around it; `key` is the column of the cell with the cursor in it. `expanded` says the block was taken in
// place of what was selected.
bool CrunchWindow::sortRangeOf(CellRange *range, int *key, bool *expanded)
{
    *expanded = false;
    const grid::Selection &sel = m_view->selection();
    const quint32 activeRow = quint32(std::max<qint64>(0, sel.activeRow)), activeCol = quint32(std::max<qint64>(0, sel.activeCol));
    CellRange picked = selectedRange();
    const bool oneCell = picked.row1 == picked.row2 && picked.col1 == picked.col2;
    bool regionTruncated = false;
    const CellRange region = crunch::currentRegion(*m_session, m_sheet, activeRow, activeCol, &regionTruncated);
    if (oneCell || sel.whole != grid::Selection::Whole::None) {
        if (sel.whole == grid::Selection::Whole::None) {
            if (regionTruncated) {
                notice(tr("Sort"), tr("The block of data around this cell is too big to find its edges quickly. Select the cells to put in order."));
                return false;
            }
            if (region.cellCount() <= 1) {
                notice(tr("Sort"), tr("There is nothing to sort here. Select a cell in the data, or select the cells to put in order."));
                return false;
            }
            picked = region;
            *expanded = true;
        } else {
            // whole rows or columns: the cells of them that are used
            const jp::office::sheet::Calculation *calc = m_session->calculation();
            const jp::office::sheet::Sheet &s = calc->sheet(m_sheet);
            if (s.extentRows == 0) {
                notice(tr("Sort"), tr("There is nothing to sort here."));
                return false;
            }
            picked.row2 = std::min(picked.row2, s.extentRows - 1);
            picked.col2 = std::min(picked.col2, s.extentCols - 1);
        }
    } else {
        // data next to a selection that is the whole of some columns of it would be left behind
        const bool beside = region.cellCount() > picked.cellCount() && region.row1 <= picked.row1 && region.row2 >= picked.row2 && region.col1 <= picked.col1 && region.col2 >= picked.col2
                            && (region.col1 < picked.col1 || region.col2 > picked.col2);
        if (beside) {
            const bool expand = m_expandAsker ? m_expandAsker()
                                              : confirm(tr("Sort"), tr("There is data next to the cells you selected. It would not be sorted with them, and the rows would no longer match. Sort the whole block of data instead?"));
            if (expand) {
                picked = region;
                *expanded = true;
            }
        }
    }
    *range = picked;
    *key = int(std::clamp<qint64>(qint64(activeCol) - picked.col1, 0, qint64(picked.col2) - picked.col1));
    return true;
}

void CrunchWindow::sortSelection(int mode)
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed() || m_sortPlan) return;
    CellRange range;
    int key = 0;
    bool expanded = false;
    if (!sortRangeOf(&range, &key, &expanded)) return;
    crunch::SortSpec spec;
    spec.hasHeader = m_session->looksLikeHeader(m_sheet, range);
    crunch::SortLevel level;
    level.key = key;
    level.descending = mode == 1;
    spec.levels.append(level);
    if (mode == 2) {
        if (m_sortAsker) {
            if (!m_sortAsker(range, &spec)) return;
        } else {
            const int sheet = m_sheet;
            SortDialog::Provider provider;
            provider.names = [this, range, sheet](bool header, bool leftToRight) {
                QStringList names;
                const quint32 count = leftToRight ? range.row2 - range.row1 + 1 : range.col2 - range.col1 + 1;
                for (quint32 i = 0; i < std::min<quint32>(count, 1000); ++i) {
                    const quint32 row = leftToRight ? range.row1 + i : range.row1, col = leftToRight ? range.col1 : range.col1 + i;
                    const QString head = header ? m_session->displayText(sheet, row, col).trimmed().left(40) : QString();
                    names << (!head.isEmpty() ? head : leftToRight ? tr("Row %1").arg(row + 1) : tr("Column %1").arg(jp::office::sheet::columnName(col)));
                }
                return names;
            };
            provider.colors = [this, range, sheet](int key, bool font, bool leftToRight) {
                SortDialog::Colors out;
                CellRange line = range;
                if (leftToRight) line.row1 = line.row2 = range.row1 + quint32(key);
                else line.col1 = line.col2 = range.col1 + quint32(key);
                const jp::office::sheet::Styles &st = m_loaded->book->styles();
                for (const jp::office::sheet::ColorSpec &c : m_session->colorsIn(sheet, line, font, &out.none)) out.colors.append({c, QColor::fromRgb(st.resolve(c, 0x000000))});
                return out;
            };
            SortDialog dialog(provider, spec, this);
            if (dialog.exec() != QDialog::Accepted) return;
            spec = dialog.spec();
        }
    }
    sortRange(range, spec);
}

bool CrunchWindow::sortRange(const CellRange &given, const crunch::SortSpec &spec)
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed() || m_sortPlan) return false;
    CellRange range = given;
    if (!range.valid) {
        int key = 0;
        bool expanded = false;
        if (!sortRangeOf(&range, &key, &expanded)) return false;
    }
    QString why;
    m_sortPlan = m_session->planSort(m_sheet, range, spec, &why);
    if (!m_sortPlan) {
        notice(tr("Sort"), why);
        Q_EMIT sortFinished(false);
        return false;
    }
    m_sortCancel = false;
    m_sortOrdering = false;
    m_sortOrderedOk = false;
    setStatus(tr("Sorting…"));
    // a small sort is done at once; a large one a slice at a time
    m_sortSmall = range.cellCount() <= qint64(kSmallSort);
    if (m_sortSmall) {
        m_session->prepareSort(*m_sortPlan, 0);
        const bool ordered = crunch::EditSession::orderSort(*m_sortPlan, &m_sortCancel);
        finishSort(ordered);
        return true;
    }
    m_sortTimer.start();
    return true;
}

void CrunchWindow::stepSort()
{
    if (!m_sortPlan || m_sortOrdering) return;
    if (!m_session) {
        cancelSort();
        return;
    }
    if (m_sortCancel) {
        finishSort(false);
        return;
    }
    m_sortTuner.begin();
    const qint64 budget = m_sortTuner.budget();
    const bool done = m_session->prepareSort(*m_sortPlan, budget);
    m_sortTuner.end(budget);
    if (!done) return;
    // the keys are read: the order is worked out on a thread of its own
    m_sortTimer.stop();
    m_sortOrdering = true;
    const auto plan = m_sortPlan;
    auto *thread = QThread::create([this, plan] { m_sortOrderedOk = crunch::EditSession::orderSort(*plan, &m_sortCancel); });
    m_sortThread = thread;
    connect(thread, &QThread::finished, this, [this, thread] {
        thread->deleteLater();
        finishSort(m_sortOrderedOk);
    });
    thread->start();
}

void CrunchWindow::cancelSort()
{
    m_sortCancel = true;
    if (m_sortThread) return;   // (the thread ends the sort when it sees the flag)
    if (m_sortPlan) finishSort(false);
}

void CrunchWindow::finishSort(bool ordered)
{
    m_sortTimer.stop();
    m_sortOrdering = false;
    const auto plan = std::move(m_sortPlan);
    m_sortPlan.reset();
    m_sortThread.clear();
    bool made = false;
    if (plan && m_session) {
        if (ordered && !m_sortCancel && !m_sortSmall) {
            // the step is made and put in a slice at a time
            const qint64 before = m_loader.changesMade();
            runApply(
                1, [this, plan](qint64 budget) { return m_session->applySort(*plan, budget); },
                [this, before](const crunch::EditSession::Outcome &o) {
                    bool done = false;
                    if (!o.ok) {
                        if (!o.problem.isEmpty()) notice(tr("Sort"), o.problem);
                    } else {
                        done = true;
                        if (m_loader.changesMade() != before) m_calculating = true;
                    }
                    if (!m_calculating && !m_loading) setStatus(tr("Ready"));
                    Q_EMIT sortFinished(done);
                });
            return;
        }
        if (ordered && !m_sortCancel) {
            const qint64 before = m_loader.changesMade();
            const crunch::EditSession::Outcome o = doStructural([&] { return m_session->applySort(*plan); });
            if (!o.ok) {
                if (!o.problem.isEmpty()) notice(tr("Sort"), o.problem);
            } else {
                made = true;
                if (m_loader.changesMade() != before) m_calculating = true;
            }
        } else if (m_sortCancel) {
            setStatus(tr("The sort was stopped."));
            Q_EMIT sortFinished(false);
            return;
        } else {
            notice(tr("Sort"), tr("The cells hold too much text to sort at once. Select fewer."));
        }
    }
    if (!m_calculating && !m_loading) setStatus(tr("Ready"));
    Q_EMIT sortFinished(made);
}

} // namespace jp
