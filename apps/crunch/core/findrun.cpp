#include "core/findrun.h"

#include <QCoreApplication>

#include <algorithm>
#include <functional>
#include <iterator>

namespace jp::crunch {

using namespace jp::office::sheet;

FindRun::FindRun(const EditSession *session, const FindOptions &options, Want want, const FoundCell &from)
    : m_session(session), m_options(options), m_want(want), m_matcher(options.pattern), m_from(from), m_haveFrom(want == Want::Next)
{
    if (!m_matcher.valid()) {
        m_problem = options.pattern.text.isEmpty() ? QCoreApplication::translate("jp::crunch::FindRun", "Type the text to look for.")
                                                   : QCoreApplication::translate("jp::crunch::FindRun", "The text to look for can have up to %1 characters.").arg(kMaxPatternChars);
        m_done = true;
        return;
    }
    const Workbook *book = session ? session->workbook() : nullptr;
    const Calculation *calc = session ? session->calculation() : nullptr;
    if (!book || !calc) {
        m_done = true;
        return;
    }
    auto searchable = [&](int sheet) {
        if (sheet < 0 || sheet >= book->info().sheets.size() || sheet >= calc->sheetCount()) return false;
        const SheetEntry &e = book->info().sheets[sheet];
        return !e.deleted && e.kind == SheetKind::Worksheet;
    };
    if (options.scope == FindOptions::Scope::Sheet) {
        if (searchable(options.sheet)) m_sheets.push_back(options.sheet);
    } else {
        // (the sheets that are shown, in the order of the tabs; the sheet in view too, if it is hidden)
        for (int sheet : session->sheetOrder())
            if (searchable(sheet) && (book->info().sheets[sheet].state == SheetState::Visible || sheet == options.sheet)) m_sheets.push_back(sheet);
    }
    m_ordered = want == Want::Next;
    if (m_ordered) {
        // the stretches in the order of the search, from just after the cell with the cursor, round to just before it
        const int fromPos = positionOf(from.sheet);
        auto full = [&](int pos) {
            Segment g;
            g.sheet = m_sheets[size_t(pos)];
            m_segments.push_back(g);
        };
        if (fromPos >= 0) {
            Segment first;
            first.sheet = from.sheet;
            first.hasStart = true;
            first.startRow = from.row;
            first.startCol = from.col;
            m_segments.push_back(first);
            for (int pos = fromPos + 1; pos < int(m_sheets.size()); ++pos) full(pos);
            for (int pos = 0; pos < fromPos; ++pos) full(pos);
            Segment last;
            last.sheet = from.sheet;
            last.hasEnd = true;
            last.endRow = from.row;
            last.endCol = from.col;
            m_segments.push_back(last);
        } else {
            for (int pos = 0; pos < int(m_sheets.size()); ++pos) full(pos);
        }
    } else {
        for (int sheet : m_sheets) {
            Segment g;
            g.sheet = sheet;
            m_segments.push_back(g);
        }
    }
    if (options.lookIn == FindOptions::LookIn::Notes) {
        for (const int sheet : m_sheets) {
            bool cannot = false;
            QString why;
            session->notesOf(sheet, &cannot, &why);
            if (cannot) {
                m_problem = why;
                break;
            }
        }
    }
    if (!m_problem.isEmpty()) {
        m_sheets.clear();
        m_segments.clear();
        m_done = true;
    }
}

int FindRun::positionOf(int sheet) const
{
    for (size_t i = 0; i < m_sheets.size(); ++i)
        if (m_sheets[i] == sheet) return int(i);
    return -1;
}

FindRun::Key FindRun::keyOf(int sheetPos, quint32 row, quint32 col) const
{
    return m_options.order == FindOptions::Order::ByRows ? Key(sheetPos, row, col) : Key(sheetPos, col, row);
}

qint64 FindRun::look(int sheet, quint32 row, quint32 col, const std::map<quint64, QString> *notes)
{
    QString text;
    switch (m_options.lookIn) {
    case FindOptions::LookIn::Formulas: text = m_session->searchText(sheet, row, col); break;
    case FindOptions::LookIn::Values: text = m_session->displayText(sheet, row, col); break;
    case FindOptions::LookIn::Notes: {
        const auto found = notes->find((quint64(row) << 32) | col);
        if (found != notes->end()) text = found->second;
        break;
    }
    }
    if (text.isEmpty()) return 1;
    qint64 steps = 0;
    const bool matched = m_matcher.matches(text, &steps);
    // (a long text costs more than a short one: about a unit for each 64 comparisons)
    const qint64 work = 1 + steps / 64;
    if (!matched) return work;
    FoundCell hit;
    hit.sheet = sheet;
    hit.row = row;
    hit.col = col;
    hit.text = text.left(200);
    if (m_want == Want::All) {
        if (qint64(m_hits.size()) >= kMaxFindHits) {
            m_truncated = true;
            return work;
        }
        m_hits.push_back(hit);
        return work;
    }
    if (m_ordered) {
        // (the first hit in the order of the search is the answer)
        m_hits.assign(1, hit);
        m_stop = true;
        return work;
    }
    const Key key = keyOf(positionOf(sheet), row, col);
    if (!m_haveFirst || key < keyOf(positionOf(m_first.sheet), m_first.row, m_first.col)) {
        m_first = hit;
        m_haveFirst = true;
    }
    const Key from = keyOf(positionOf(m_from.sheet), m_from.row, m_from.col);
    if (key > from && (!m_haveAfter || key < keyOf(positionOf(m_after.sheet), m_after.row, m_after.col))) {
        m_after = hit;
        m_haveAfter = true;
    }
    return work;
}

// Walks the cells of the sheet from where the last step stopped, until the budget is spent. True when the sheet is done.
bool FindRun::walkSheet(Sweep &sw, const Segment &segment, qint64 budget, qint64 *looked)
{
    const Calculation *calc = m_session->calculation();
    const Sheet &s = calc->sheet(sw.sheet);
    auto afterEnd = [&](quint32 row, quint32 col) { return segment.hasEnd && (row > segment.endRow || (row == segment.endRow && col > segment.endCol)); };
    if (segment.hasStart && !sw.started && !sw.begun) {
        // (from just after a cell: the walk is as if it had stopped on that cell)
        sw.begun = true;
        sw.row = segment.startRow;
        sw.col = segment.startCol;
        sw.started = true;
    }
    if (m_options.lookIn == FindOptions::LookIn::Notes) {
        const bool byCols = m_options.order == FindOptions::Order::ByColumns;
        // (the order of the search: by rows, or by columns)
        auto inOrder = [&](quint64 key) { return byCols ? ((quint64(key & 0xFFFFFFFFu) << 32) | (key >> 32)) : key; };
        if (!sw.notesRead) {
            sw.notes = m_session->notesOf(sw.sheet);
            sw.notesRead = true;
            for (const auto &n : *sw.notes) sw.noteOrder.push_back(n.first);
            if (byCols) std::sort(sw.noteOrder.begin(), sw.noteOrder.end(), [&](quint64 x, quint64 y) { return inOrder(x) < inOrder(y); });
            sw.noteAt = 0;
            if (segment.hasStart) {
                const quint64 from = inOrder((quint64(segment.startRow) << 32) | segment.startCol);
                sw.noteAt = size_t(std::upper_bound(sw.noteOrder.begin(), sw.noteOrder.end(), from, [&](quint64 v, quint64 key) { return v < inOrder(key); }) - sw.noteOrder.begin());
            }
        }
        for (; sw.noteAt < sw.noteOrder.size(); ++sw.noteAt) {
            const quint64 key = sw.noteOrder[sw.noteAt];
            const quint32 row = quint32(key >> 32), col = quint32(key & 0xFFFFFFFFu);
            if (segment.hasEnd && inOrder(key) > inOrder((quint64(segment.endRow) << 32) | segment.endCol)) return true;
            if (m_options.within.valid && m_options.scope == FindOptions::Scope::Sheet && !m_options.within.contains(row, col)) continue;
            const qint64 work = look(sw.sheet, row, col, sw.notes.get());
            sw.row = row;
            sw.col = col;
            sw.started = true;
            if (m_stop) return true;
            ++m_looked;
            *looked += work;
            if (budget && *looked >= budget) {
                ++sw.noteAt;
                return sw.noteAt >= sw.noteOrder.size();
            }
        }
        return true;
    }
    if (m_options.order == FindOptions::Order::ByColumns && m_want == Want::Next) return walkColumns(sw, segment, budget, looked);
    if (s.extentRows == 0 || s.extentCols == 0) return true;
    quint32 r1 = 0, c1 = 0, r2 = s.extentRows - 1, c2 = s.extentCols - 1;
    if (m_options.within.valid && m_options.scope == FindOptions::Scope::Sheet) {
        r1 = m_options.within.row1;
        c1 = m_options.within.col1;
        r2 = std::min(r2, m_options.within.row2);
        c2 = std::min(c2, m_options.within.col2);
        if (r1 > r2 || c1 > c2) return true;
    }
    bool stopped = false;
    auto visit = [&](quint32 row, quint32 col, const Cell &c) {
        if (afterEnd(row, col)) {
            sw.ended = true;
            return false;
        }
        sw.row = row;
        sw.col = col;
        sw.started = true;
        ++m_looked;
        qint64 work = 1;
        if (c.type != CellType::Empty || c.hasFormula()) work = look(sw.sheet, row, col, nullptr);
        if (m_stop) {
            sw.ended = true;
            return false;
        }
        *looked += work;
        if (budget && *looked >= budget) {
            stopped = true;
            return false;
        }
        return true;
    };
    // the cells after the one the walk stands on (all of them if it stands on none yet, or on a place before the cells looked at), within the bounds
    auto walkAfter = [&](const std::function<bool(quint32, quint32, const Cell &)> &f) {
        if (!sw.started || sw.row < r1) {
            s.cells.forEachInRange(r1, c1, r2, c2, f);
            return;
        }
        bool more = true;
        auto stopOnFalse = [&](quint32 row, quint32 col, const Cell &c) {
            more = f(row, col, c);
            return more;
        };
        if (sw.row <= r2) {
            const quint32 startCol = std::max<quint32>(c1, sw.col + 1);
            if (startCol <= c2) s.cells.forEachInRange(sw.row, startCol, sw.row, c2, stopOnFalse);
        }
        if (more && sw.row + 1 <= r2) s.cells.forEachInRange(std::max<quint32>(sw.row + 1, r1), c1, r2, c2, stopOnFalse);
    };
    walkAfter(visit);
    if (sw.ended) return true;
    if (stopped) {
        // (stopped on the very last cell: the sheet is done)
        bool more = false;
        walkAfter([&](quint32, quint32, const Cell &) {
            more = true;
            return false;
        });
        return !more;
    }
    return true;
}

// Next by columns: the cells column by column, each column from the top (only a column's cells are looked at), from just after the place the search stands on; it stops at the first hit.
bool FindRun::walkColumns(Sweep &sw, const Segment &segment, qint64 budget, qint64 *looked)
{
    const Sheet &s = m_session->calculation()->sheet(sw.sheet);
    if (s.extentRows == 0 || s.extentCols == 0) return true;
    quint32 r1 = 0, c1 = 0, r2 = s.extentRows - 1, c2 = s.extentCols - 1;
    if (m_options.within.valid && m_options.scope == FindOptions::Scope::Sheet) {
        r1 = m_options.within.row1;
        c1 = m_options.within.col1;
        r2 = std::min(r2, m_options.within.row2);
        c2 = std::min(c2, m_options.within.col2);
        if (r1 > r2 || c1 > c2) return true;
    }
    if (segment.hasStart && !sw.started && !sw.begun) {
        sw.begun = true;
        sw.row = segment.startRow;
        sw.col = segment.startCol;
        sw.started = true;
    }
    quint32 col = c1, rowFrom = r1;
    if (sw.started) {
        if (sw.col > c2) return true;
        if (sw.col >= c1) {
            col = sw.col;
            rowFrom = quint32(std::max<qint64>(r1, qint64(sw.row) + 1));
        }
    }
    for (; col <= c2; ++col, rowFrom = r1) {
        if (segment.hasEnd && col > segment.endCol) return true;
        if (rowFrom > r2) continue;
        bool stopped = false, over = false;
        s.cells.forEachInRange(rowFrom, col, r2, col, [&](quint32 cellRow, quint32 cellCol, const Cell &c) {
            if (segment.hasEnd && col == segment.endCol && cellRow > segment.endRow) {
                over = true;
                return false;
            }
            sw.row = cellRow;
            sw.col = cellCol;
            sw.started = true;
            ++m_looked;
            qint64 work = 1;
            if (c.type != CellType::Empty || c.hasFormula()) work = look(sw.sheet, cellRow, cellCol, nullptr);
            if (m_stop) return false;
            *looked += work;
            if (budget && *looked >= budget) {
                stopped = true;
                return false;
            }
            return true;
        });
        if (over || m_stop) return true;
        if (stopped) return false;
        // (the column is done: the walk stands at its end)
        sw.col = col;
        sw.row = r2;
        sw.started = true;
        if (budget && *looked >= budget && col < c2) return false;
    }
    return true;
}

void FindRun::finish()
{
    if (m_ordered) {
        // (m_hits has the first hit, or nothing)
    } else if (m_want == Want::Next) {
        m_hits.clear();
        if (m_haveAfter) m_hits.push_back(m_after);
        else if (m_haveFirst) m_hits.push_back(m_first);
    } else if (m_options.order == FindOptions::Order::ByColumns) {
        std::sort(m_hits.begin(), m_hits.end(), [&](const FoundCell &a, const FoundCell &b) {
            return keyOf(positionOf(a.sheet), a.row, a.col) < keyOf(positionOf(b.sheet), b.row, b.col);
        });
    }
    m_done = true;
}

bool FindRun::step(qint64 budget)
{
    if (m_done) return true;
    qint64 looked = 0;
    while (!m_done) {
        if (m_at >= m_segments.size() || m_stop) {
            finish();
            break;
        }
        m_sweep.sheet = m_segments[m_at].sheet;
        if (walkSheet(m_sweep, m_segments[m_at], budget, &looked)) {
            ++m_at;
            m_sweep = Sweep();
        }
        if (m_stop) {
            finish();
            break;
        }
        if (budget && looked >= budget) {
            if (m_at >= m_segments.size()) finish();
            break;
        }
    }
    return m_done;
}

} // namespace jp::crunch
