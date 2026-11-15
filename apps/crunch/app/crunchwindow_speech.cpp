// JeffCrunch's Read Aloud and missing fonts (libs/shell/app/readaloud, libs/shell/app/missingfonts): the selected cells row by row, each cell shown as it is said, and the fonts the cells use.
#include "app/crunchwindow.h"
#include "core/editsession.h"
#include "grid/gridview.h"
#include "sheet/calculation.h"

#include <algorithm>
#include <memory>
#include <vector>

namespace jp {

namespace {

// The cells of a paragraph (a row), each where its words are in the paragraph's text.
struct RowCells {
    struct Cell {
        int start = 0, length = 0;
        quint32 row = 0, col = 0;
    };
    std::vector<Cell> cells;
};

struct ReadCells {
    std::vector<RowCells> rows;   // (the number of a paragraph's story: its place here)
    bool lit = false;
};

// The most cells that are read at once.
constexpr size_t kMostCellsRead = 20000;

}

jp::ReadAloudHost CrunchWindow::readAloudHost()
{
    auto state = std::make_shared<ReadCells>();
    jp::ReadAloudHost host;
    host.text = [this, state] {
        QList<jp::ReadAloudParagraph> out;
        state->rows.clear();
        if (!m_session || m_sheet < 0 || !commitEdit()) return out;
        const jp::office::sheet::Calculation *calc = m_session->calculation();
        if (!calc || m_sheet >= calc->sheetCount()) return out;
        const auto &sheet = calc->sheet(m_sheet);
        if (sheet.extentRows == 0 || sheet.extentCols == 0) return out;
        const grid::Selection &sel = m_view->selection();
        const jp::office::sheet::CellRange range = selectedRange();
        // One cell selected: from it to the end of the sheet, row by row; more: the selected cells alone.
        const bool fromCell = sel.isOneCell();
        quint32 r1 = range.row1, c1 = range.col1, r2 = std::min(range.row2, sheet.extentRows - 1), c2 = std::min(range.col2, sheet.extentCols - 1);
        if (fromCell) {
            r2 = sheet.extentRows - 1;
            c2 = sheet.extentCols - 1;
        }
        if (r1 > r2 || c1 > c2) return out;
        std::vector<std::pair<quint32, quint32>> cells;
        sheet.cells.forEachInRange(r1, fromCell ? 0 : c1, r2, c2, [&](quint32 row, quint32 col, const jp::office::sheet::Cell &) {
            if (fromCell && row == r1 && col < c1) return true;
            cells.emplace_back(row, col);
            return cells.size() < kMostCellsRead;
        });
        std::sort(cells.begin(), cells.end());
        quint32 current = 0;
        QString text;
        RowCells row;
        auto flush = [&] {
            if (text.isEmpty()) return;
            state->rows.push_back(row);
            out.push_back({text, int(state->rows.size()) - 1, 0});
            text.clear();
            row = RowCells();
        };
        bool first = true;
        for (const auto &rc : cells) {
            const QString shown = m_session->displayText(m_sheet, rc.first, rc.second).simplified();
            if (shown.isEmpty()) continue;
            if (!first && rc.first != current) flush();
            first = false;
            current = rc.first;
            if (!text.isEmpty()) text += QStringLiteral(", ");
            row.cells.push_back({int(text.size()), int(shown.size()), rc.first, rc.second});
            text += shown;
        }
        flush();
        return out;
    };
    host.highlight = [this, state](const jp::ReadAloudParagraph &p, int start, int) {
        if (p.story < 0 || size_t(p.story) >= state->rows.size()) return;
        const RowCells &row = state->rows[size_t(p.story)];
        const RowCells::Cell *at = nullptr;
        for (const RowCells::Cell &c : row.cells)
            if (c.start <= start) at = &c;
        if (!at) return;
        m_settingMarks = true;
        m_view->setMarks({grid::GridRange{qint64(at->row), qint64(at->col), qint64(at->row), qint64(at->col)}});
        m_view->ensureVisible(at->row, at->col);
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
    host.replace = [this](const QString &from, const QString &to) {
        if (!m_session || !commitEdit() || !editingAllowed()) return -1;
        int cells = 0;
        const crunch::EditSession::Outcome out = m_session->replaceFont(from, to, tr("Replace Font"), &cells);
        if (!out.ok) {
            notice(tr("Replace Font"), out.problem);
            return -1;
        }
        m_view->viewport()->update();
        return cells;
    };
    host.refresh = [this] { m_view->viewport()->update(); };
    return host;
}

} // namespace jp
