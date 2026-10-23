#pragma once
// What a worksheet holds, as it was read from a file: its cells in blocks of
// rows (only the cells the file has; a sheet of ten cells is ten cells), the
// text of its strings and formulas, and what the file says about its shape.
// The shared strings of a workbook are one list that every sheet refers to.

#include "sheet/cellref.h"

#include <QHash>
#include <QString>
#include <QVector>

#include <algorithm>
#include <map>
#include <vector>

namespace jp::office::sheet {

enum class CellType : quint8 {
    Empty,           // the cell is there (it has a style, or a formula with no result stored) but holds no value
    Number,          // number
    SharedString,    // number holds the place of its text in the workbook's shared strings
    InlineString,    // number holds the place of its text in the sheet's texts
    FormulaString,   // the text a formula gave, as in InlineString
    Bool,            // number is 0 (false) or 1 (true)
    Error,           // number is a CellError
    Date,            // number is the day count (a fraction of a day for a time) in the workbook's date system
};

// The errors a cell can hold; the file spells them "#DIV/0!" and so on.
enum class CellError : quint8 { Null, Div0, Value, Ref, Name, Num, NA, GettingData, Spill, Calc, Field, Blocked, Connect, External, Unknown };
QString errorText(CellError error);
// The error a text spells; Unknown (and false in `known`) for any other.
CellError errorFromText(QStringView text, bool *known = nullptr);

// What more a cell has: the bits of Cell::flags.
enum CellFlag : quint8 {
    HasFormula = 1,
    ArrayFormula = 2,    // the cell holds the formula of an array (its range is in the formula)
    SharedFormula = 4,   // the formula is one a group of cells share, as written for the first of them
    DataTable = 8,       // the formula is a what-if data table
};

// One cell: 24 bytes. A cell always has its place in the grid; its value is
// in number (and, for text, the pool it points to), and its formula, if it
// has one, in the sheet's formulas.
struct Cell {
    double number = 0;
    quint32 row = 0;
    quint16 col = 0;
    CellType type = CellType::Empty;
    quint8 flags = 0;
    quint32 style = 0;       // the place of its format in the workbook's formats, as the file gives it
    qint32 formula = -1;     // the place in Sheet::formulas; -1 for none

    bool hasFormula() const { return flags & HasFormula; }
    // A formula cell has a result when the file stored one.
    bool hasResult() const { return type != CellType::Empty; }
    qint32 textIndex() const { return qint32(number); }
};
static_assert(sizeof(Cell) == 24, "a cell is 24 bytes");

// The cells of one sheet, kept in blocks of rows: a block holds the cells of
// kBlockRows rows, in order of row and then column, and only blocks that have
// a cell exist. Cells may be added in any order; finish() puts each block in
// order.
class CellStore {
public:
    static constexpr quint32 kBlockRows = 64;

    CellStore() = default;
    CellStore(const CellStore &o) : m_blocks(o.m_blocks), m_count(o.m_count) {}
    CellStore &operator=(const CellStore &o)
    {
        m_blocks = o.m_blocks;
        m_count = o.m_count;
        m_last = nullptr;
        return *this;
    }
    // A move takes the blocks and leaves the store it was taken from empty, with no memory of where it last added a cell (that place is a block it no longer has).
    CellStore(CellStore &&o) noexcept : m_blocks(std::move(o.m_blocks)), m_count(o.m_count)
    {
        o.m_blocks.clear();
        o.m_count = 0;
        o.m_last = nullptr;
    }
    CellStore &operator=(CellStore &&o) noexcept
    {
        if (this != &o) {
            m_blocks = std::move(o.m_blocks);
            m_count = o.m_count;
            m_last = nullptr;
            o.m_blocks.clear();
            o.m_count = 0;
            o.m_last = nullptr;
        }
        return *this;
    }

    // Adds a cell. A cell at a place that has one already replaces it (the
    // later one stands), as finish() decides.
    void add(const Cell &cell);
    // Puts every block in order and drops a cell a later one replaces. Reads
    // (find, forEach) are only right after this.
    void finish();
    // How many cells were added (before finish(), counting any a later one will replace).
    qint64 cellCount() const { return m_count; }
    // The cell at the place, or null.
    const Cell *find(quint32 row, quint32 col) const;
    // Calls f(cell) for each cell in order of row, then column.
    template <class F>
    void forEach(F f) const
    {
        for (const auto &block : m_blocks)
            for (const Cell &c : block.second.cells) f(c);
    }
    // Calls f(cell) for each cell inside the rectangle (both ends in it), in order of row and then column; f gives false to stop. Only the rows and cells in it are
    // looked at: a narrow range of a wide sheet costs what it holds.
    template <class F>
    void forEachInRange(quint32 row1, quint32 col1, quint32 row2, quint32 col2, F f) const
    {
        for (auto block = m_blocks.lower_bound(row1 / kBlockRows); block != m_blocks.end() && block->first * kBlockRows <= row2; ++block) {
            const std::vector<Cell> &cells = block->second.cells;
            const quint32 first = block->first * kBlockRows;
            quint32 r = std::max(row1, first);
            const quint32 last = std::min(row2, first + kBlockRows - 1);
            auto pos = cells.begin();
            while (r <= last && pos != cells.end()) {
                Cell key;
                key.row = r;
                key.col = quint16(col1);
                pos = std::lower_bound(pos, cells.end(), key, [](const Cell &a, const Cell &b) { return a.row != b.row ? a.row < b.row : a.col < b.col; });
                if (pos == cells.end() || pos->row > last) break;
                r = pos->row;
                while (pos != cells.end() && pos->row == r && pos->col <= col2) {
                    if (!f(*pos)) return;
                    ++pos;
                }
                // the rest of this row's cells are right of the range
                while (pos != cells.end() && pos->row == r) ++pos;
                ++r;
            }
        }
    }
    int blockCount() const { return int(m_blocks.size()); }
    // What the cells take, in bytes, as the blocks hold them.
    qint64 memoryBytes() const;

private:
    struct Block {
        std::vector<Cell> cells;
        bool ordered = true;
    };
    std::map<quint32, Block> m_blocks;
    qint64 m_count = 0;
    Block *m_last = nullptr;
    quint32 m_lastKey = 0;
};

// What a formula cell holds. Cells of a shared formula point at one text (the
// first cell's) and say how far they are from that cell; the text of a given
// cell is worked out when asked for (Sheet::formulaText).
struct Formula {
    qint32 text = -1;          // the place in Sheet::formulaTexts
    qint32 dRow = 0;           // rows below (negative: above) the cell the text was written for
    qint32 dCol = 0;           // and columns to the right
    qint32 range = -1;         // the place in Sheet::ranges of an array formula's cells or a data table's; -1 for none
};

// Every string the sheets of a workbook have in common, in order. The text of
// a rich string is its runs one after another.
struct SharedStrings {
    QVector<QString> strings;
    qint64 chars = 0;
    QString at(qint64 i) const { return i >= 0 && i < strings.size() ? strings[int(i)] : QString(); }
};

// What the reading of one sheet found out about the file, for the limits to be
// chosen by: the most (largest or longest) any one thing came to.
struct SheetStats {
    qint64 longestString = 0;        // characters, of an inline string or the text of a formula's result
    qint64 longestFormula = 0;
    qint64 sharedFormulas = 0;       // distinct shared formulas
    qint64 largestSharedRange = 0;   // cells
    qint64 expandedFormulaChars = 0; // the formulas' characters, a shared formula counted once for each cell
    qint64 xmlBytes = 0;             // what was read of the part
    qint64 heldBytes = 0;            // what the sheet takes to hold, as Limits::sheetBytes counts it
    qint64 largestToken = 0;         // the most bytes of XML in one piece (a tag with its attributes, a text, a comment) the parser was given
    qint64 largestRead = 0;          // the most bytes the parser asked for at one time
};

struct Sheet {
    CellStore cells;
    QVector<QString> texts;           // inline strings and the texts of formula results
    QVector<QString> formulaTexts;    // as written, and no "=" in front
    QVector<Formula> formulas;
    QVector<CellRange> ranges;        // the cells of array formulas and data tables
    CellRange dimension;              // what the part says the used cells are (it may be wrong, or missing)
    qint64 mergeCount = 0;            // merged ranges
    qint64 formulaCells = 0;          // cells with a formula
    qint64 storedResults = 0;         // of them, those the file stored a result for
    SheetStats stats;
    // Empty when the whole part was read. Else the cells read before the
    // problem are here and this says what the problem was (the XML is damaged,
    // or something was past a limit).
    QString error;

    // The cell's text: that of a shared, inline, or formula string; for any
    // other the empty string.
    QString text(const Cell &cell, const SharedStrings &shared) const;
    // The formula of the cell as written for it: a shared formula as the cell
    // has it, relative references moved to where it is. Empty for a cell with
    // no formula.
    QString formulaText(const Cell &cell) const;
    // The cells an array formula (or a data table) covers; not valid for any other cell.
    CellRange formulaRange(const Cell &cell) const;
};

} // namespace jp::office::sheet
