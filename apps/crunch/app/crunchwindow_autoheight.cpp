// Automatic row heights: a row whose height is not a person's follows what is in it. Text that wraps, a larger font, and an entry of several lines (Alt+Enter) make the row taller, and the
// row is as tall as it needs when they go. The rows are measured when they are in view (and the rows an edit changed, a few at a time), not the whole sheet at once; a row is measured
// again when what is in it changes, and all the rows in view when the width of a column does. A row a person gave a height, or hid, is left as it is. See EditSession::setAutoRowHeights.

#include "app/crunchwindow.h"

#include <QElapsedTimer>

#include <algorithm>

namespace jp {

namespace {
// The cells looked at in one slice of the measuring (a slice is a few milliseconds of the window's time; it goes on in the next).
constexpr qint64 kCellsPerSlice = 1500;
// The most cells of one row looked at, and the most rows an edit marks to be measured one by one (past it, only the rows in view are measured as they come into view).
constexpr qint64 kCellsPerRow = 2000;
constexpr size_t kMaxDirtyRows = 20000;
constexpr size_t kMaxMarkedAtOnce = 2000;
} // namespace

void CrunchWindow::scheduleAutoHeights()
{
    if (!m_autoTimer.isActive() && m_haveCalc) m_autoTimer.start();
}

void CrunchWindow::noteRowsChanged(int sheet, const std::vector<jp::grid::ValueStore::Item> &cells)
{
    if (m_applyingHeights || cells.empty()) return;
    std::set<quint32> &dirty = m_autoDirty[sheet];
    if (cells.size() > kMaxMarkedAtOnce || dirty.size() + cells.size() > kMaxDirtyRows) {
        // (so many: the rows in view are measured as they are looked at, and no row is marked one by one on the window's thread)
        dirty.clear();
        ++m_autoGen[sheet];
    } else {
        for (const auto &c : cells) dirty.insert(c.row);
    }
    if (sheet == m_sheet) scheduleAutoHeights();
}

void CrunchWindow::noteWidthsChanged(int sheet)
{
    if (m_applyingHeights) return;
    ++m_autoGen[sheet];
    if (sheet == m_sheet) scheduleAutoHeights();
}

void CrunchWindow::stepAutoHeights()
{
    if (!m_session || !m_loaded || !m_loaded->calc || m_sheet < 0 || m_chartSheetShown || m_applyingHeights || m_loading) return;
    if (m_loaded->book->info().sheets[m_sheet].kind != jp::office::sheet::SheetKind::Worksheet) return;
    const int sheet = m_sheet;
    const int gen = m_autoGen[sheet];
    std::set<quint32> &dirty = m_autoDirty[sheet];
    std::unordered_map<quint32, int> &seen = m_autoSeen[sheet];
    const grid::GridSheet &gs = m_view->sheet();
    const jp::office::sheet::Calculation *calc = m_loaded->calc.get();
    const double defaultPoints = std::max(1.0, calc->sheet(sheet).format.defaultRowHeight);
    // the rows to look at: those in view, then the rows an edit changed
    std::vector<quint32> order;
    const qint64 first = std::max<qint64>(0, m_view->firstRowInView());
    const qint64 count = std::max<qint64>(1, m_view->wholeRowsInView()) + 4;
    for (qint64 r = first; r < first + count && r < qint64(jp::office::sheet::kMaxRows); ++r) order.push_back(quint32(r));
    for (const quint32 r : dirty)
        if (order.size() < order.capacity() + 20000 && (r < first || r >= first + count)) order.push_back(r);
    qint64 cells = 0;
    std::vector<std::pair<quint32, double>> heights;
    bool more = false;
    for (const quint32 row : order) {
        const bool isDirty = dirty.count(row) != 0;
        const auto s = seen.find(row);
        const bool stale = isDirty || s == seen.end() || s->second != gen;
        if (!stale) continue;
        if (cells >= kCellsPerSlice) {
            more = true;
            break;
        }
        if (gs.layout().rows.size(row) == 0) {   // (hidden)
            seen[row] = gen;
            dirty.erase(row);
            continue;
        }
        const std::vector<jp::office::sheet::RowSpec> specs = calc->rowSpecsIn(sheet, row, row);
        const bool custom = !specs.empty() && specs.front().customHeight;
        // a row the file gave a height that is not a person's, and that nothing has changed: its height is what the program that wrote the file worked out, and stays
        const bool trusted = !specs.empty() && specs.front().height > 0 && !isDirty && gen == 0 && s == seen.end();
        if (custom || trusted) {
            seen[row] = gen;
            dirty.erase(row);
            continue;
        }
        const double needed = gs.autoHeight(row, kCellsPerRow);
        cells += 20 + (needed > 0 ? 20 : 0);
        heights.emplace_back(row, std::max(defaultPoints, needed));
        seen[row] = gen;
        dirty.erase(row);
    }
    if (!heights.empty()) {
        // (a row that needs only the default height, and has no height of its own, has nothing put in it)
        std::vector<std::pair<quint32, double>> put;
        for (const auto &h : heights) {
            const std::vector<jp::office::sheet::RowSpec> specs = calc->rowSpecsIn(sheet, h.first, h.first);
            if (specs.empty() && std::abs(h.second - defaultPoints) < 0.01) continue;
            put.push_back(h);
        }
        m_applyingHeights = true;
        m_session->setAutoRowHeights(sheet, put);
        m_applyingHeights = false;
    }
    if (more) scheduleAutoHeights();
}

} // namespace jp
