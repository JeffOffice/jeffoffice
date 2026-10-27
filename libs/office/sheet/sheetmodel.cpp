#include "sheet/sheetmodel.h"

#include <algorithm>

namespace jp::office::sheet {

namespace {

const char *const kErrorTexts[] = {"#NULL!", "#DIV/0!", "#VALUE!", "#REF!", "#NAME?", "#NUM!", "#N/A", "#GETTING_DATA", "#SPILL!", "#CALC!", "#FIELD!", "#BLOCKED!", "#CONNECT!", "#EXTERNAL!"};
constexpr int kKnownErrors = int(sizeof(kErrorTexts) / sizeof(kErrorTexts[0]));

bool before(const Cell &a, const Cell &b) { return a.rowId != b.rowId ? a.rowId < b.rowId : a.colId < b.colId; }

} // namespace

QString errorText(CellError error)
{
    const int i = int(error);
    return i >= 0 && i < kKnownErrors ? QString::fromLatin1(kErrorTexts[i]) : QStringLiteral("#UNKNOWN!");
}

CellError errorFromText(QStringView text, bool *known)
{
    for (int i = 0; i < kKnownErrors; ++i)
        if (text.compare(QLatin1String(kErrorTexts[i]), Qt::CaseInsensitive) == 0) {
            if (known) *known = true;
            return CellError(i);
        }
    if (known) *known = false;
    return CellError::Unknown;
}

void CellStore::add(quint32 row, quint32 col, const Cell &placed)
{
    const quint32 rowId = rowIdOf(row), colId = colIdOf(col);
    if (rowId == kNoLine || colId == kNoLine) return;
    Cell cell = placed;
    cell.rowId = rowId;
    cell.colId = quint16(colId);
    const quint32 key = rowId / kBlockRows;
    if (!m_last || m_lastKey != key) {
        m_last = &m_blocks[key];
        m_lastKey = key;
    }
    Block &block = *m_last;
    if (!block.cells.empty() && !before(block.cells.back(), cell)) block.ordered = false;
    block.cells.push_back(cell);
    ++m_count;
}

void CellStore::finish()
{
    m_count = 0;
    for (auto &entry : m_blocks) {
        Block &block = entry.second;
        if (!block.ordered) {
            // A cell added later for the same place stands, so the sort keeps the order of those added.
            std::stable_sort(block.cells.begin(), block.cells.end(), before);
            auto out = block.cells.begin();
            for (auto in = block.cells.begin(); in != block.cells.end(); ++in) {
                const auto next = in + 1;
                if (next != block.cells.end() && !before(*in, *next)) continue;
                *out++ = *in;
            }
            block.cells.erase(out, block.cells.end());
            block.ordered = true;
        }
        block.cells.shrink_to_fit();
        m_count += qint64(block.cells.size());
    }
    m_last = nullptr;
}

void CellStore::reorder()
{
    for (auto &entry : m_blocks) {
        Block &block = entry.second;
        if (block.ordered) continue;
        std::stable_sort(block.cells.begin(), block.cells.end(), before);
        block.ordered = true;
    }
    m_last = nullptr;
}

const Cell *CellStore::find(quint32 row, quint32 col) const
{
    const quint32 rowId = rowIdOf(row), colId = colIdOf(col);
    if (rowId == kNoLine || colId == kNoLine) return nullptr;
    const auto block = m_blocks.find(rowId / kBlockRows);
    if (block == m_blocks.end()) return nullptr;
    Cell key;
    key.rowId = rowId;
    key.colId = quint16(colId);
    const auto &cells = block->second.cells;
    const auto at = std::lower_bound(cells.begin(), cells.end(), key, before);
    return at != cells.end() && at->rowId == rowId && at->colId == colId ? &*at : nullptr;
}

Cell *CellStore::findMutable(quint32 row, quint32 col) { return const_cast<Cell *>(find(row, col)); }

Cell *CellStore::set(quint32 row, quint32 col, const Cell &placed)
{
    const quint32 rowId = rowIdOf(row), colId = colIdOf(col);
    if (rowId == kNoLine || colId == kNoLine) return nullptr;
    Cell cell = placed;
    cell.rowId = rowId;
    cell.colId = quint16(colId);
    Block &block = m_blocks[rowId / kBlockRows];
    m_last = nullptr;
    const auto at = std::lower_bound(block.cells.begin(), block.cells.end(), cell, before);
    if (at != block.cells.end() && at->rowId == rowId && at->colId == colId) {
        *at = cell;
        return &*at;
    }
    ++m_count;
    return &*block.cells.insert(at, cell);
}

bool CellStore::erase(quint32 row, quint32 col)
{
    const quint32 rowId = rowIdOf(row), colId = colIdOf(col);
    if (rowId == kNoLine || colId == kNoLine) return false;
    const auto block = m_blocks.find(rowId / kBlockRows);
    if (block == m_blocks.end()) return false;
    Cell key;
    key.rowId = rowId;
    key.colId = quint16(colId);
    auto &cells = block->second.cells;
    const auto at = std::lower_bound(cells.begin(), cells.end(), key, before);
    if (at == cells.end() || at->rowId != rowId || at->colId != colId) return false;
    cells.erase(at);
    --m_count;
    m_last = nullptr;
    if (cells.empty()) m_blocks.erase(block);
    return true;
}

qint64 CellStore::memoryBytes() const
{
    qint64 bytes = 0;
    for (const auto &entry : m_blocks) bytes += qint64(entry.second.cells.capacity()) * qint64(sizeof(Cell)) + 64;
    return bytes;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// whole rows and columns inserted and deleted

namespace {

// A cell of a deleted row or column holds no formula (the node of it is let go of, and Undo makes the formula again from the record): nothing that is not live may be read as one.
void letGoOfFormula(Cell &c)
{
    c.flags &= quint8(~(HasFormula | ArrayFormula | SharedFormula | DataTable | ArrayMember | Tainted));
    c.formula = -1;
}

bool inRuns(const std::vector<jp::calc::IdRun> &sorted, quint32 id)
{
    auto it = std::upper_bound(sorted.begin(), sorted.end(), id, [](quint32 v, const jp::calc::IdRun &r) { return v < r.first; });
    return it != sorted.begin() && id - (it - 1)->first < (it - 1)->count;
}

} // namespace

qint64 CellStore::liveCellsIn(bool rows, const std::vector<jp::calc::IdRun> &runs, bool kill)
{
    qint64 live = 0;
    if (runs.empty()) return 0;
    if (rows) {
        for (const jp::calc::IdRun &run : runs) {
            const quint32 last = run.first + run.count - 1;
            for (auto block = m_blocks.lower_bound(run.first / kBlockRows); block != m_blocks.end() && block->first <= last / kBlockRows; ++block) {
                std::vector<Cell> &cells = block->second.cells;
                auto at = std::lower_bound(cells.begin(), cells.end(), run.first, [](const Cell &c, quint32 id) { return c.rowId < id; });
                for (; at != cells.end() && at->rowId <= last; ++at) {
                    if (colPosOf(at->colId) != kNoLine) ++live;
                    if (kill) letGoOfFormula(*at);
                }
            }
        }
        return live;
    }
    std::vector<jp::calc::IdRun> sorted = runs;
    std::sort(sorted.begin(), sorted.end(), [](const jp::calc::IdRun &a, const jp::calc::IdRun &b) { return a.first < b.first; });
    for (auto &entry : m_blocks)
        for (Cell &c : entry.second.cells) {
            if (!inRuns(sorted, c.colId)) continue;
            if (rowPosOf(c.rowId) != kNoLine) ++live;
            if (kill) letGoOfFormula(c);
        }
    return live;
}

jp::calc::LineStatus CellStore::insertLines(bool rows, quint32 at, quint32 count, LineEdits *edits)
{
    using jp::calc::LineStatus;
    jp::calc::LineMapCore &map = rows ? m_rows.core() : m_cols.core();
    if (count == 0 || at >= map.size()) return LineStatus::OutOfRange;
    // (lines inserted so near the end that they would not all fit: everything from `at` on is pushed off, and the new lines fill the place)
    if (uint64_t(at) + count > map.limits().capacity) return deleteLines(rows, at, map.limits().capacity - at, edits);
    LineEdits local;
    LineEdits &e = edits ? *edits : local;
    e = LineEdits();
    const LineStatus st = map.insert(at, count, &e.edit);
    if (st != LineStatus::Ok) return st;
    e.made = true;
    m_dead += liveCellsIn(rows, e.edit.pushed, true);
    return LineStatus::Ok;
}

jp::calc::LineStatus CellStore::deleteLines(bool rows, quint32 at, quint32 count, LineEdits *edits)
{
    using jp::calc::LineStatus;
    jp::calc::LineMapCore &map = rows ? m_rows.core() : m_cols.core();
    if (count == 0 || at >= map.size()) return LineStatus::OutOfRange;
    count = std::min(count, map.size() - at);
    LineEdits local;
    LineEdits &e = edits ? *edits : local;
    e = LineEdits();
    LineStatus st = map.erase(at, count, &e.edit);
    if (st != LineStatus::Ok) return st;
    st = map.insert(map.size(), count, &e.refill);
    if (st != LineStatus::Ok) {
        map.undo(e.edit);
        return st;
    }
    e.refilled = true;
    e.made = true;
    m_dead += liveCellsIn(rows, e.edit.runs, true);
    return LineStatus::Ok;
}

jp::calc::LineStatus CellStore::canEditLines(bool rows, quint32 count) const
{
    using jp::calc::LineStatus;
    const jp::calc::LineMapCore &map = rows ? m_rows.core() : m_cols.core();
    const jp::calc::LineLimits &limits = map.limits();
    if (uint64_t(map.runCount()) + map.deadCount() + 8 > limits.maxRuns) return LineStatus::TooManyRuns;
    if (uint64_t(map.nextId()) + 2 * uint64_t(count) > limits.idLimit) return LineStatus::IdsExhausted;
    return LineStatus::Ok;
}

jp::calc::LineStatus CellStore::undoLines(bool rows, LineEdits &e)
{
    using jp::calc::LineStatus;
    using Kind = jp::calc::LineEdit::Kind;
    jp::calc::LineMapCore &map = rows ? m_rows.core() : m_cols.core();
    if (!e.made) return LineStatus::StaleStep;
    if (e.refilled) {
        const LineStatus st = map.undo(e.refill);
        if (st != LineStatus::Ok) return st;
    }
    const LineStatus st = map.undo(e.edit);
    if (st != LineStatus::Ok) {
        if (e.refilled) map.redo(e.refill);
        return st;
    }
    e.made = false;
    if (e.refilled) m_dead += liveCellsIn(rows, e.refill.runs, true);
    if (e.edit.kind == Kind::Erase) {
        m_dead -= liveCellsIn(rows, e.edit.runs, false);
    } else {
        m_dead += liveCellsIn(rows, e.edit.runs, true);
        m_dead -= liveCellsIn(rows, e.edit.pushed, false);
    }
    return LineStatus::Ok;
}

jp::calc::LineStatus CellStore::redoLines(bool rows, LineEdits &e)
{
    using jp::calc::LineStatus;
    using Kind = jp::calc::LineEdit::Kind;
    jp::calc::LineMapCore &map = rows ? m_rows.core() : m_cols.core();
    if (e.made) return LineStatus::StaleStep;
    LineStatus st = map.redo(e.edit);
    if (st != LineStatus::Ok) return st;
    if (e.refilled) {
        st = map.redo(e.refill);
        if (st != LineStatus::Ok) {
            map.undo(e.edit);
            return st;
        }
    }
    e.made = true;
    if (e.edit.kind == Kind::Erase) {
        m_dead += liveCellsIn(rows, e.edit.runs, true);
    } else {
        m_dead -= liveCellsIn(rows, e.edit.runs, false);
        m_dead += liveCellsIn(rows, e.edit.pushed, true);
    }
    if (e.refilled) m_dead -= liveCellsIn(rows, e.refill.runs, false);
    return LineStatus::Ok;
}

void CellStore::formulaCells(bool rows, quint32 first, std::vector<PlacedCell> *out) const
{
    if (rows && !m_rows.isIdentity()) {
        // (the rows at the places from `first` on, in the order of their places: the blocks that hold them, and no others)
        walkRows(first, quint32(-1), [&](quint32 row, std::vector<Cell>::const_iterator a, std::vector<Cell>::const_iterator b) {
            std::size_t from = out->size();
            for (; a != b; ++a) {
                if (!(a->flags & (HasFormula | ArrayMember))) continue;
                const quint32 col = colPosOf(a->colId);
                if (col != kNoLine) out->push_back(PlacedCell{row, col, *a});
            }
            if (!m_cols.isIdentity()) std::sort(out->begin() + std::ptrdiff_t(from), out->end(), [](const PlacedCell &x, const PlacedCell &y) { return x.col < y.col; });
            return true;
        });
        return;
    }
    for (auto block = m_blocks.lower_bound(rows ? first / kBlockRows : 0u); block != m_blocks.end(); ++block)
        for (const Cell &c : block->second.cells) {
            if (!(c.flags & (HasFormula | ArrayMember))) continue;
            const quint32 row = rowPosOf(c.rowId), col = colPosOf(c.colId);
            if (row == kNoLine || col == kNoLine || (rows ? row : col) < first) continue;
            out->push_back(PlacedCell{row, col, c});
        }
    if (!plain()) std::sort(out->begin(), out->end(), [](const PlacedCell &a, const PlacedCell &b) { return a.row != b.row ? a.row < b.row : a.col < b.col; });
}

QString Sheet::text(const Cell &cell, const SharedStrings &shared) const
{
    switch (cell.type) {
    case CellType::SharedString: return shared.at(cell.textIndex());
    case CellType::InlineString:
    case CellType::FormulaString: return texts.value(cell.textIndex());
    default: return {};
    }
}

QString Sheet::formulaText(const Cell &cell) const
{
    if (!cell.hasFormula() || cell.formula < 0 || cell.formula >= formulas.size()) return {};
    const Formula &f = formulas[cell.formula];
    if (f.text < 0 || f.text >= formulaTexts.size()) return {};
    const QString &text = formulaTexts[f.text];
    return f.dRow == 0 && f.dCol == 0 ? text : shiftFormula(text, f.dRow, f.dCol);
}

CellRange Sheet::formulaRange(const Cell &cell) const
{
    if (!cell.hasFormula() || cell.formula < 0 || cell.formula >= formulas.size()) return {};
    const Formula &f = formulas[cell.formula];
    return f.range >= 0 && f.range < ranges.size() ? ranges[f.range] : CellRange();
}

} // namespace jp::office::sheet
