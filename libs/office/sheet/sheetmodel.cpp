#include "sheet/sheetmodel.h"

#include "sheet/formulaframe.h"
#include "sheet/refshift.h"

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
    return findById(rowId, colId);
}

const Cell *CellStore::findById(quint32 rowId, quint32 colId) const
{
    if (rowId == kNoLine || colId >= 0x10000) return nullptr;
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

quint64 CellStore::nextSerial()
{
    static std::atomic<quint64> next{1};
    return next++;
}

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

qint64 CellStore::liveCellsIn(bool rows, const std::vector<jp::calc::IdRun> &runs, bool kill, std::vector<Cell> *formulas, bool keep)
{
    qint64 live = 0;
    if (runs.empty()) return 0;
    if (keep) kill = false;   // (the cells are only counted: a cell of a dead line keeps its formula, which keeps its node)
    auto one = [&](Cell &c, bool otherLive) {
        if (otherLive) ++live;
        if (!kill || !otherLive) return;   // (a cell that is in a dead line of the other kind already is as that left it: its formula is kept, with its node, if the other edit kept it)
        if (formulas && (c.flags & HasFormula)) formulas->push_back(c);
        letGoOfFormula(c);
    };
    if (rows) {
        for (const jp::calc::IdRun &run : runs) {
            const quint32 last = run.first + run.count - 1;
            for (auto block = m_blocks.lower_bound(run.first / kBlockRows); block != m_blocks.end() && block->first <= last / kBlockRows; ++block) {
                std::vector<Cell> &cells = block->second.cells;
                auto at = std::lower_bound(cells.begin(), cells.end(), run.first, [](const Cell &c, quint32 id) { return c.rowId < id; });
                if (!kill && m_cols.isIdentity()) {
                    // (every column is live: the cells of the run in this block are the count, by two searches)
                    const auto end = std::upper_bound(at, cells.end(), last, [](quint32 v, const Cell &c) { return v < c.rowId; });
                    live += end - at;
                    continue;
                }
                for (; at != cells.end() && at->rowId <= last; ++at) one(*at, colPosOf(at->colId) != kNoLine);
            }
        }
        return live;
    }
    std::vector<jp::calc::IdRun> sorted = runs;
    std::sort(sorted.begin(), sorted.end(), [](const jp::calc::IdRun &a, const jp::calc::IdRun &b) { return a.first < b.first; });
    if (sorted.size() == 1) {
        // (one run of columns, the common case: two compares a cell, not a search)
        const quint32 first = sorted[0].first, count = sorted[0].count;
        for (auto &entry : m_blocks)
            for (Cell &c : entry.second.cells)
                if (quint32(c.colId) - first < count) one(c, rowPosOf(c.rowId) != kNoLine);
        return live;
    }
    for (auto &entry : m_blocks)
        for (Cell &c : entry.second.cells)
            if (inRuns(sorted, c.colId)) one(c, rowPosOf(c.rowId) != kNoLine);
    return live;
}

qint64 CellStore::purgeCells(bool rows, const std::vector<jp::calc::IdRun> &runs)
{
    qint64 gone = 0;
    if (runs.empty()) return 0;
    if (rows) {
        for (const jp::calc::IdRun &run : runs) {
            const quint32 last = run.first + run.count - 1;
            for (auto block = m_blocks.lower_bound(run.first / kBlockRows); block != m_blocks.end() && block->first <= last / kBlockRows;) {
                std::vector<Cell> &cells = block->second.cells;
                const auto lo = std::lower_bound(cells.begin(), cells.end(), run.first, [](const Cell &c, quint32 id) { return c.rowId < id; });
                const auto hi = std::upper_bound(lo, cells.end(), last, [](quint32 v, const Cell &c) { return v < c.rowId; });
                gone += hi - lo;
                cells.erase(lo, hi);
                if (cells.empty()) block = m_blocks.erase(block);
                else ++block;
            }
        }
    } else {
        std::vector<jp::calc::IdRun> sorted = runs;
        std::sort(sorted.begin(), sorted.end(), [](const jp::calc::IdRun &a, const jp::calc::IdRun &b) { return a.first < b.first; });
        const bool single = sorted.size() == 1;
        const quint32 first = sorted[0].first, count = sorted[0].count;
        for (auto block = m_blocks.begin(); block != m_blocks.end();) {
            std::vector<Cell> &cells = block->second.cells;
            const size_t before = cells.size();
            cells.erase(std::remove_if(cells.begin(), cells.end(), [&](const Cell &c) { return single ? quint32(c.colId) - first < count : inRuns(sorted, c.colId); }), cells.end());
            gone += qint64(before - cells.size());
            if (cells.empty()) block = m_blocks.erase(block);
            else ++block;
        }
    }
    m_count -= gone;
    m_dead -= gone;
    m_last = nullptr;
    return gone;
}

jp::calc::LineStatus CellStore::insertLines(bool rows, quint32 at, quint32 count, LineEdits *edits, LineShift *shift, std::vector<Cell> *deadFormulas)
{
    using jp::calc::LineStatus;
    jp::calc::LineMapCore &map = rows ? m_rows.core() : m_cols.core();
    if (count == 0 || at >= map.size()) return LineStatus::OutOfRange;
    // (lines inserted so near the end that they would not all fit: everything from `at` on is pushed off, and the new lines fill the place)
    if (uint64_t(at) + count > map.limits().capacity) return deleteLines(rows, at, map.limits().capacity - at, edits, shift, deadFormulas);
    LineEdits local;
    LineEdits &e = edits ? *edits : local;
    e = LineEdits();
    const LineStatus st = map.insert(at, count, &e.edit);
    if (st != LineStatus::Ok) return st;
    e.made = true;
    e.owner = m_serial;
    m_dead += liveCellsIn(rows, e.edit.pushed, true, deadFormulas);
    if (shift) *shift = {at, qint64(count)};
    return LineStatus::Ok;
}

jp::calc::LineStatus CellStore::deleteLines(bool rows, quint32 at, quint32 count, LineEdits *edits, LineShift *shift, std::vector<Cell> *deadFormulas, bool keepFormulas)
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
    e.owner = m_serial;
    m_dead += liveCellsIn(rows, e.edit.runs, true, deadFormulas, keepFormulas);
    if (shift) *shift = {at, -qint64(count)};
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

bool CellStore::needsCompaction(bool rows, quint32 count) const
{
    const jp::calc::LineMapCore &map = rows ? m_rows.core() : m_cols.core();
    const jp::calc::LineLimits &limits = map.limits();
    return map.needsRebase() || map.deadCount() >= limits.rebaseRuns || canEditLines(rows, count) != jp::calc::LineStatus::Ok;
}

jp::calc::LineStatus CellStore::undoLines(bool rows, LineEdits &e, LineShift *shift, std::vector<Cell> *deadFormulas)
{
    using jp::calc::LineStatus;
    using Kind = jp::calc::LineEdit::Kind;
    jp::calc::LineMapCore &map = rows ? m_rows.core() : m_cols.core();
    if (!e.made || !ownEdit(e)) return LineStatus::StaleStep;
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
    if (e.refilled) m_dead += liveCellsIn(rows, e.refill.runs, true, nullptr);
    if (e.edit.kind == Kind::Erase) {
        m_dead -= liveCellsIn(rows, e.edit.runs, false);
    } else {
        m_dead += liveCellsIn(rows, e.edit.runs, true, deadFormulas);
        m_dead -= liveCellsIn(rows, e.edit.pushed, false);
    }
    if (shift) *shift = {e.edit.at, e.edit.kind == Kind::Erase ? qint64(e.lines()) : -qint64(e.lines())};
    map.tryIdentity();
    return LineStatus::Ok;
}

jp::calc::LineStatus CellStore::redoLines(bool rows, LineEdits &e, LineShift *shift, std::vector<Cell> *deadFormulas, bool keepFormulas)
{
    using jp::calc::LineStatus;
    using Kind = jp::calc::LineEdit::Kind;
    jp::calc::LineMapCore &map = rows ? m_rows.core() : m_cols.core();
    if (e.made || !ownEdit(e)) return LineStatus::StaleStep;
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
        m_dead += liveCellsIn(rows, e.edit.runs, true, deadFormulas, keepFormulas);
    } else {
        m_dead -= liveCellsIn(rows, e.edit.runs, false);
        m_dead += liveCellsIn(rows, e.edit.pushed, true, deadFormulas);
    }
    if (e.refilled) m_dead -= liveCellsIn(rows, e.refill.runs, false);
    if (shift) *shift = {e.edit.at, e.edit.kind == Kind::Erase ? -qint64(e.lines()) : qint64(e.lines())};
    return LineStatus::Ok;
}

void CellStore::forgetLines(bool rows, LineEdits *edits, bool keepDead)
{
    using Kind = jp::calc::LineEdit::Kind;
    LineEdits &e = *edits;
    jp::calc::LineMapCore &map = rows ? m_rows.core() : m_cols.core();
    if (!ownEdit(e) || e.edit.runs.empty()) {
        e = LineEdits();
        return;
    }
    // the lines that only this step could bring back: what it killed, if it is done; what it made, if it is undone
    if (e.refilled && !e.made) {
        purgeCells(rows, e.refill.runs);
        map.forget(e.refill);
    }
    if (e.edit.kind == Kind::Erase) {
        if (e.made) {
            purgeCells(rows, e.edit.runs);
            if (!keepDead) map.dropDead(e.edit.runs);
        }
    } else if (e.made) {
        purgeCells(rows, e.edit.pushed);
        if (!keepDead) map.dropDead(e.edit.pushed);
    } else {
        purgeCells(rows, e.edit.runs);
        map.forget(e.edit);
    }
    map.tryIdentity();
    e = LineEdits();
}

qint64 CellStore::cellsInLines(bool rows, quint32 first, quint32 count) const
{
    if (count == 0) return 0;
    std::vector<jp::calc::IdRun> runs;
    (rows ? m_rows.core() : m_cols.core()).walk(first, first + count - 1, &runs);
    qint64 n = 0;
    if (rows) {
        for (const jp::calc::IdRun &run : runs) {
            const quint32 last = run.first + run.count - 1;
            for (auto block = m_blocks.lower_bound(run.first / kBlockRows); block != m_blocks.end() && block->first <= last / kBlockRows; ++block) {
                const std::vector<Cell> &cells = block->second.cells;
                const auto lo = std::lower_bound(cells.begin(), cells.end(), run.first, [](const Cell &c, quint32 id) { return c.rowId < id; });
                const auto hi = std::upper_bound(lo, cells.end(), last, [](quint32 v, const Cell &c) { return v < c.rowId; });
                n += hi - lo;
            }
        }
        return n;
    }
    forEachInIdRuns(false, runs, [&](const Cell &) {
        ++n;
        return true;
    });
    return n;
}

void CellStore::finishForgetLines(bool rows, LineEdits *edits, bool keepDead)
{
    using Kind = jp::calc::LineEdit::Kind;
    LineEdits &e = *edits;
    jp::calc::LineMapCore &map = rows ? m_rows.core() : m_cols.core();
    if (ownEdit(e) && !e.edit.runs.empty() && e.edit.kind == Kind::Erase && e.made && !keepDead) map.dropDead(e.edit.runs);
    map.tryIdentity();
    e = LineEdits();
}

quint32 CellStore::renumberLine(const std::vector<jp::calc::LineMapCore::Renumber> &table, quint32 id)
{
    auto it = std::upper_bound(table.begin(), table.end(), id, [](quint32 v, const auto &r) { return v < r.oldFirst; });
    if (it == table.begin() || id - (it - 1)->oldFirst >= (it - 1)->count) return kNoLine;
    return (it - 1)->newFirst + (id - (it - 1)->oldFirst);
}

jp::calc::LineStatus CellStore::rebaseLines(bool rows, std::vector<jp::calc::LineMapCore::Renumber> *out)
{
    using jp::calc::LineStatus;
    jp::calc::LineMapCore &map = rows ? m_rows.core() : m_cols.core();
    std::vector<jp::calc::LineMapCore::Renumber> table;
    const LineStatus st = map.rebase(&table);
    if (out) out->clear();
    if (st != LineStatus::Ok || table.empty()) return st;
    std::sort(table.begin(), table.end(), [](const auto &a, const auto &b) { return a.oldFirst < b.oldFirst; });
    if (out) *out = table;
    auto renumber = [&](quint32 id) -> quint32 { return renumberLine(table, id); };
    std::map<quint32, Block> blocks;
    qint64 gone = 0;
    for (auto &entry : m_blocks)
        for (Cell &c : entry.second.cells) {
            const quint32 id = renumber(rows ? c.rowId : c.colId);
            if (id == kNoLine) {
                ++gone;
                continue;
            }
            if (rows) c.rowId = id;
            else c.colId = quint16(id);
            Block &block = blocks[c.rowId / kBlockRows];
            if (!block.cells.empty() && !before(block.cells.back(), c)) block.ordered = false;
            block.cells.push_back(c);
        }
    for (auto &entry : blocks) {
        Block &block = entry.second;
        if (!block.ordered) std::sort(block.cells.begin(), block.cells.end(), before);
        block.ordered = true;
        block.cells.shrink_to_fit();
    }
    m_blocks = std::move(blocks);
    m_count -= gone;
    m_dead -= gone;
    m_last = nullptr;
    return LineStatus::Ok;
}

qint64 CellStore::deadCountByScan() const
{
    qint64 n = 0;
    for (const auto &entry : m_blocks)
        for (const Cell &c : entry.second.cells)
            if (rowPosOf(c.rowId) == kNoLine || colPosOf(c.colId) == kNoLine) ++n;
    return n;
}

void CellStore::setLineLimits(const jp::calc::LineLimits &rows, const jp::calc::LineLimits &cols)
{
    if (!plain()) return;
    m_rows = jp::calc::RowMap(rows, rows.capacity);
    m_cols = jp::calc::ColMap(cols, cols.capacity);
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

QVector<RowSpec> Sheet::placedRowSpecs() const
{
    if (!rowSpecsAreIds || cells.rows().isIdentity()) return rowSpecs;
    QVector<RowSpec> out;
    out.reserve(rowSpecs.size());
    for (RowSpec r : rowSpecs) {
        const quint32 place = cells.rowPlaceOf(r.row);
        if (place == CellStore::kNoLine) continue;   // (the row is deleted: its spec comes back with it)
        r.row = place;
        out.append(r);
    }
    std::stable_sort(out.begin(), out.end(), [](const RowSpec &a, const RowSpec &b) { return a.row < b.row; });
    return out;
}

QString Sheet::formulaText(const Cell &cell) const
{
    if (!cell.hasFormula() || cell.formula < 0 || cell.formula >= formulas.size()) return {};
    const Formula &f = formulas[cell.formula];
    if (f.text < 0 || f.text >= formulaTexts.size()) return {};
    const QString &text = formulaTexts[f.text];
    if (frame && !positionalTexts.contains(f.text)) {
        // (held in ids: the text for the cell, then the places the ids are at now)
        const bool wide = wideTexts.contains(f.text);
        const jp::calc::ParseLimits limits = wide ? jp::calc::ParseLimits::ids() : jp::calc::ParseLimits::grid();
        const QString own = f.dRow == 0 && f.dCol == 0 ? text : wide ? offsetFormula(text, f.dRow, f.dCol, nullptr, limits) : shiftFormula(text, f.dRow, f.dCol);
        return frame->allPlain() ? own : idsToPlaces(*frame, frameIndex, own, limits);
    }
    return f.dRow == 0 && f.dCol == 0 ? text : shiftFormula(text, f.dRow, f.dCol);
}

QString Sheet::formulaTextOfTheFile(const Cell &cell) const
{
    if (!frame || !cell.hasFormula() || cell.formula < 0 || cell.formula >= formulas.size()) return {};
    const Formula &f = formulas[cell.formula];
    if (f.text < 0 || f.text >= formulaTexts.size() || positionalTexts.contains(f.text)) return {};
    const QString &text = formulaTexts[f.text];
    const bool wide = wideTexts.contains(f.text);
    return f.dRow == 0 && f.dCol == 0 ? text : wide ? offsetFormula(text, f.dRow, f.dCol, nullptr, jp::calc::ParseLimits::ids()) : shiftFormula(text, f.dRow, f.dCol);
}

CellRange Sheet::formulaRange(const Cell &cell) const
{
    if (!cell.hasFormula() || cell.formula < 0 || cell.formula >= formulas.size()) return {};
    const Formula &f = formulas[cell.formula];
    return f.range >= 0 && f.range < ranges.size() ? ranges[f.range] : CellRange();
}

} // namespace jp::office::sheet
