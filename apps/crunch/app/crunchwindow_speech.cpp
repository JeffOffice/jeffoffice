// JeffCrunch's Read Aloud and missing fonts (libs/shell/app/readaloud, libs/shell/app/missingfonts): the selected cells row by row, one cell said at a time and tinted as it is said, and the
// fonts the cells use.
#include "app/crunchwindow.h"
#include "core/editsession.h"
#include "grid/gridview.h"
#include "sheet/calculation.h"

#include <QCoreApplication>
#include <QUndoStack>

#include <algorithm>
#include <memory>
#include <vector>

namespace jp {

namespace {

struct ReadCells {
    std::vector<std::pair<quint32, quint32>> cells;   // (row, column) of each paragraph: its story number is its place here
    bool capped = false;
    bool lit = false;
};

// The most cells that are read at once, and the most that are looked at to find them.
constexpr size_t kMostCellsRead = 20000, kMostCellsLooked = 2000000;

}

jp::ReadAloudHost CrunchWindow::readAloudHost()
{
    auto state = std::make_shared<ReadCells>();
    jp::ReadAloudHost host;
    host.text = [this, state] {
        QList<jp::ReadAloudParagraph> out;
        state->cells.clear();
        state->capped = false;
        if (!m_session || m_sheet < 0 || !commitEdit()) return out;
        const jp::office::sheet::Calculation *calc = m_session->calculation();
        if (!calc || m_sheet >= calc->sheetCount()) return out;
        const auto &sheet = calc->sheet(m_sheet);
        if (sheet.extentRows == 0 || sheet.extentCols == 0) return out;
        const grid::Selection &sel = m_view->selection();
        const jp::office::sheet::CellRange range = selectedRange();
        // One cell selected: from it to the end of the sheet, row by row; more: the selected cells alone.
        const bool fromCell = sel.isOneCell();
        const quint32 r1 = range.row1, c1 = range.col1;
        const quint32 r2 = fromCell ? sheet.extentRows - 1 : std::min(range.row2, sheet.extentRows - 1);
        const quint32 c2 = fromCell ? sheet.extentCols - 1 : std::min(range.col2, sheet.extentCols - 1);
        if (r1 > r2 || c1 > c2) return out;
        // (all the cells are put in row-major order first, then the first kMostCellsRead are kept: the cap never skips the cells that come first)
        std::vector<std::pair<quint32, quint32>> cells;
        size_t looked = 0;
        sheet.cells.forEachInRange(r1, fromCell ? 0 : c1, r2, c2, [&](quint32 row, quint32 col, const jp::office::sheet::Cell &) {
            if (fromCell && row == r1 && col < c1) return true;
            cells.emplace_back(row, col);
            return ++looked < kMostCellsLooked;
        });
        std::sort(cells.begin(), cells.end());
        for (const auto &rc : cells) {
            const QString shown = m_session->displayText(m_sheet, rc.first, rc.second).simplified();
            if (shown.isEmpty()) continue;
            if (state->cells.size() >= kMostCellsRead) {
                state->capped = true;
                break;
            }
            state->cells.push_back(rc);
            out.push_back({shown, int(state->cells.size()) - 1, 0});   // (one cell is one paragraph: what is said at once)
        }
        if (looked >= kMostCellsLooked) state->capped = true;
        return out;
    };
    host.note = [state] { return state->capped ? QCoreApplication::translate("jp::CrunchWindow", "Read Aloud stops after 20,000 cells") : QString(); };
    host.highlight = [this, state](const jp::ReadAloudParagraph &p, int, int) {
        if (p.story < 0 || size_t(p.story) >= state->cells.size()) return;
        const auto rc = state->cells[size_t(p.story)];
        m_settingMarks = true;
        m_view->setMarks({grid::GridRange{qint64(rc.first), qint64(rc.second), qint64(rc.first), qint64(rc.second)}});
        m_view->ensureVisible(rc.first, rc.second);
        m_settingMarks = false;
        state->lit = true;
    };
    host.clear = [this, state] {
        if (!state->lit) return;
        state->lit = false;
        m_settingMarks = true;
        m_view->setMarks({});
        m_settingMarks = false;
    };
    // An edit of any kind (a cell, a row, a column, or a sheet inserted or deleted: all are steps of the undo stack), or another workbook, ends the reading; so does a switch of sheet
    // (setActiveSheet tells the bar).
    host.watch = [this](const std::function<void()> &changed) {
        QList<QMetaObject::Connection> c;
        if (m_session) c << connect(m_session->undoStack(), &QUndoStack::indexChanged, this, [changed] { changed(); });
        c << connect(this, &CrunchWindow::loadFinished, this, [changed] { changed(); });
        return c;
    };
    return host;
}

jp::MissingFontsHost CrunchWindow::missingFontsHost()
{
    jp::MissingFontsHost host;
    host.usage = [this] {
        jp::FontUsage u;
        if (!m_session) return u;
        for (const crunch::EditSession::FontUse &use : m_session->fontsInUse())
            for (const QString &sheet : use.sheets) u.add(use.family, sheet);
        return u;
    };
    host.replace = [this](const QString &from, const QString &to) { return replaceMissingFont(from, to); };
    host.refresh = [this] { m_view->viewport()->update(); };
    return host;
}

// Replace in Document: the one place JeffCrunch changes the fonts of the file for the dialog (a shared Replace Fonts can take this place later).
int CrunchWindow::replaceMissingFont(const QString &from, const QString &to)
{
    if (!m_session || !commitEdit() || !editingAllowed()) return -1;
    int cells = 0;
    const crunch::EditSession::Outcome out = m_session->replaceFont(from, to, tr("Replace Font"), &cells);
    if (!out.ok) {
        notice(tr("Replace Font"), out.problem);
        return -1;
    }
    m_view->viewport()->update();
    return cells;
}

} // namespace jp
