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
    Cell cell = placed;
    cell.rowId = row;
    cell.colId = quint16(col);
    const quint32 key = row / kBlockRows;
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

qint64 CellStore::shiftRows(quint32 first, qint64 delta, quint32 limit, const std::function<void(const PlacedCell &, const PlacedCell &)> &formulaMoved, std::vector<PlacedCell> *removed)
{
    m_last = nullptr;
    // the blocks from the one that holds row `first` are taken out, and their cells (in order of row and column, which a shift keeps) are made into blocks again as they come
    std::vector<Block> taken;
    for (auto it = m_blocks.lower_bound(first / kBlockRows); it != m_blocks.end(); it = m_blocks.erase(it)) taken.push_back(std::move(it->second));
    std::vector<Cell> current;
    quint32 currentKey = 0;
    qint64 moved = 0;
    auto flush = [&]() {
        if (current.empty()) return;
        Block block;
        block.cells = std::move(current);
        m_blocks.emplace_hint(m_blocks.end(), currentKey, std::move(block));
        current.clear();
    };
    const qint64 deletedEnd = delta < 0 ? qint64(first) - delta : 0;
    for (Block &block : taken) {
        const size_t hint = block.cells.size();   // (the cells of a block are about as many in the block that is made of it: room for them at once, not a doubling at a time)
        if (current.capacity() < hint) current.reserve(hint);
        for (Cell &c : block.cells) {
            const Cell before = c;
            if (c.rowId >= first) {
                const qint64 row = qint64(c.rowId) + delta;
                if ((delta < 0 && qint64(c.rowId) < deletedEnd) || row >= qint64(limit) || row < 0) {
                    --m_count;
                    if (removed) removed->push_back(PlacedCell{before.rowId, before.colId, before});
                    continue;
                }
                c.rowId = quint32(row);
                ++moved;
                if (formulaMoved && (c.hasFormula() || (c.flags & ArrayMember))) formulaMoved(PlacedCell{before.rowId, before.colId, before}, PlacedCell{c.rowId, c.colId, c});
            }
            const quint32 key = c.rowId / kBlockRows;
            if (!current.empty() && key != currentKey) {
                flush();
                current.reserve(hint);
            }
            currentKey = key;
            current.push_back(c);
        }
    }
    flush();
    return moved;
}

qint64 CellStore::shiftColumns(quint32 first, qint64 delta, quint32 limit, const std::function<void(const PlacedCell &, const PlacedCell &)> &formulaMoved, std::vector<PlacedCell> *removed)
{
    m_last = nullptr;
    qint64 moved = 0;
    const qint64 deletedEnd = delta < 0 ? qint64(first) - delta : 0;
    for (auto it = m_blocks.begin(); it != m_blocks.end();) {
        std::vector<Cell> &cells = it->second.cells;
        size_t keep = 0;
        for (size_t i = 0; i < cells.size(); ++i) {
            Cell &c = cells[i];
            if (c.colId >= first) {
                const qint64 col = qint64(c.colId) + delta;
                if ((delta < 0 && qint64(c.colId) < deletedEnd) || col >= qint64(limit) || col < 0) {
                    --m_count;
                    if (removed) removed->push_back(PlacedCell{c.rowId, c.colId, c});
                    continue;
                }
                if (formulaMoved && (c.hasFormula() || (c.flags & ArrayMember))) {
                    const Cell before = c;
                    c.colId = quint16(col);
                    formulaMoved(PlacedCell{before.rowId, before.colId, before}, PlacedCell{c.rowId, c.colId, c});
                } else {
                    c.colId = quint16(col);
                }
                ++moved;
            }
            if (keep != i) cells[keep] = c;
            ++keep;
        }
        cells.resize(keep);
        if (cells.empty()) it = m_blocks.erase(it);
        else ++it;
    }
    return moved;
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
    const auto block = m_blocks.find(row / kBlockRows);
    if (block == m_blocks.end()) return nullptr;
    Cell key;
    key.rowId = row;
    key.colId = quint16(col);
    const auto &cells = block->second.cells;
    const auto at = std::lower_bound(cells.begin(), cells.end(), key, before);
    return at != cells.end() && at->rowId == row && at->colId == col ? &*at : nullptr;
}

Cell *CellStore::findMutable(quint32 row, quint32 col) { return const_cast<Cell *>(find(row, col)); }

Cell *CellStore::set(quint32 row, quint32 col, const Cell &placed)
{
    Cell cell = placed;
    cell.rowId = row;
    cell.colId = quint16(col);
    Block &block = m_blocks[row / kBlockRows];
    m_last = nullptr;
    const auto at = std::lower_bound(block.cells.begin(), block.cells.end(), cell, before);
    if (at != block.cells.end() && at->rowId == row && at->colId == col) {
        *at = cell;
        return &*at;
    }
    ++m_count;
    return &*block.cells.insert(at, cell);
}

bool CellStore::erase(quint32 row, quint32 col)
{
    const auto block = m_blocks.find(row / kBlockRows);
    if (block == m_blocks.end()) return false;
    Cell key;
    key.rowId = row;
    key.colId = quint16(col);
    auto &cells = block->second.cells;
    const auto at = std::lower_bound(cells.begin(), cells.end(), key, before);
    if (at == cells.end() || at->rowId != row || at->colId != col) return false;
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
