#pragma once
// What a worksheet holds, as it was read from a file: its cells in blocks of
// rows (only the cells the file has; a sheet of ten cells is ten cells), the
// text of its strings and formulas, and what the file says about its shape.
// The shared strings of a workbook are one list that every sheet refers to.

#include "sheet/cellref.h"
#include "sheet/styles.h"

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
    ArrayMember = 16,    // a cell of the range of an array formula that is not its first cell: its value is the formula's (set by the calculation; `formula` then names the array's formula)
    Tainted = 32,        // a value the calculation could not check (a cell of a data table, or of an array whose formula it kept apart)
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
    // Edits, for a store that is finished: the cell at the place, to be changed where it is (null if there is none); a cell put where it belongs, in place of the one that is
    // there, and the cell as it is held; a cell taken away (false if there was none). A cell is put in at the cost of moving the cells after it in its block of rows.
    Cell *findMutable(quint32 row, quint32 col);
    Cell *set(const Cell &cell);
    bool erase(quint32 row, quint32 col);
    // Calls f(cell) for each cell in order of row, then column.
    template <class F>
    void forEach(F f) const
    {
        for (const auto &block : m_blocks)
            for (const Cell &c : block.second.cells) f(c);
    }
    // What a walk over a range did, counted as it did it: the cells it stepped over one by one (every move of the cursor along a row of a block is counted), and the
    // comparisons of its searches for the first cell of each row. A test counts them to see that a narrow range of a wide sheet costs what it holds.
    struct RangeWalk {
        qint64 stepped = 0;
        qint64 compared = 0;
    };
    // Calls f(cell) for each cell inside the rectangle (both ends in it), in order of row and then column; f gives false to stop. Only the rows and cells in it are
    // looked at: a narrow range of a wide sheet costs what it holds, and a search for each row's first cell, not a walk over the cells before it. (`walk`, if given,
    // counts what was done: see RangeWalk.)
    template <class F>
    void forEachInRange(quint32 row1, quint32 col1, quint32 row2, quint32 col2, F f, RangeWalk *walk = nullptr) const
    {
        // The place in a block's cells. Every step along them is made, and counted, here.
        class Cursor {
        public:
            Cursor(std::vector<Cell>::const_iterator at, RangeWalk *walk) : m_at(at), m_walk(walk) {}
            const Cell &operator*() const { return *m_at; }
            const Cell *operator->() const { return &*m_at; }
            Cursor &operator++()
            {
                ++m_at;
                if (m_walk) ++m_walk->stepped;
                return *this;
            }
            std::vector<Cell>::const_iterator at() const { return m_at; }
            void jumpTo(std::vector<Cell>::const_iterator at) { m_at = at; }

        private:
            std::vector<Cell>::const_iterator m_at;
            RangeWalk *m_walk;
        };
        for (auto block = m_blocks.lower_bound(row1 / kBlockRows); block != m_blocks.end() && block->first * kBlockRows <= row2; ++block) {
            const std::vector<Cell> &cells = block->second.cells;
            const quint32 first = block->first * kBlockRows;
            quint32 r = std::max(row1, first);
            const quint32 last = std::min(row2, first + kBlockRows - 1);
            Cursor pos(cells.begin(), walk);
            while (r <= last && pos.at() != cells.end()) {
                Cell key;
                key.row = r;
                key.col = quint16(col1);
                pos.jumpTo(std::lower_bound(pos.at(), cells.end(), key, [walk](const Cell &a, const Cell &b) {
                    if (walk) ++walk->compared;
                    return a.row != b.row ? a.row < b.row : a.col < b.col;
                }));
                if (pos.at() == cells.end() || pos->row > last) break;
                if (pos->row != r) {
                    // that row has no cell at or after the first column: look for the first column again in the row that has one
                    r = pos->row;
                    continue;
                }
                while (pos.at() != cells.end() && pos->row == r && pos->col <= col2) {
                    if (!f(*pos)) return;
                    ++pos;
                }
                // the rest of this row's cells are right of the range: the search for the next row's first cell passes them
                ++r;
            }
        }
    }
    // Moves cells about, for rows and columns inserted and deleted and cells shifted: every cell for which `affected(cell)` is true (those of the blocks from the one of row
    // `firstRow` on are looked at, and no others) is taken out; `map(cell)` changes the cell to what it is after (its place), and gives false for one that is gone (it is given to
    // `removed`, as it was); `moved(before, after)` is told of each that stays; they are put in again, and the blocks are put in order. The cost is that of the cells taken out.
    template <class Affected, class Map, class Moved>
    void reshuffle(quint32 firstRow, Affected affected, Map map, Moved moved, std::vector<Cell> *removed)
    {
        std::vector<Cell> moving;
        for (auto it = m_blocks.lower_bound(firstRow / kBlockRows); it != m_blocks.end();) {
            std::vector<Cell> &cells = it->second.cells;
            size_t keep = 0;
            for (size_t i = 0; i < cells.size(); ++i) {
                if (affected(cells[i])) moving.push_back(cells[i]);
                else cells[keep++] = cells[i];
            }
            cells.resize(keep);
            if (cells.empty()) it = m_blocks.erase(it);
            else ++it;
        }
        m_last = nullptr;
        m_count -= qint64(moving.size());
        for (const Cell &before : moving) {
            Cell after = before;
            if (!map(&after)) {
                if (removed) removed->push_back(before);
                continue;
            }
            moved(before, after);
            add(after);
        }
        reorder();
    }
    // Puts the blocks that were added to out of order in order (the cells that were put in again by reshuffle).
    void reorder();
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
    qint64 columnSpans = 0;          // `col` elements
    qint64 rowSpecs = 0;             // rows that were kept as not the default
};

// What the file says about a run of columns (a `col` element): its width in characters of the default font's widest digit (with the padding the format adds),
// whether it is hidden, and the cell format that the cells of the columns that hold nothing have.
struct ColumnSpan {
    quint32 first = 0, last = 0;   // the columns (from 0), both ends in the run
    double width = -1;             // characters; -1 when the file gives none
    bool hidden = false;
    bool customWidth = false;
    quint32 style = 0;             // the place of a cell format; 0 for none
    quint8 outline = 0;            // the outline level
    bool collapsed = false, bestFit = false, phonetic = false;   // and the rest of what a `col` element says, for writing it again
};

// What the file says about a row that is not as the sheet's default: its height in points (-1 for none), whether it is hidden, and the cell format (when the row
// asks for it, `customFormat`) that the cells of the row that hold nothing have.
struct RowSpec {
    quint32 row = 0;
    double height = -1;
    bool hidden = false;
    bool customHeight = false;
    bool customFormat = false;
    quint32 style = 0;
};

// `sheetFormatPr`: the sizes of rows and columns that have none of their own.
struct SheetFormat {
    double defaultRowHeight = 15;   // points
    double defaultColWidth = 0;     // characters, with the padding; 0 when the file gives none (the base width says)
    int baseColWidth = 8;           // characters of the default font's widest digit, with no padding
    bool zeroHeight = false;        // the rows that have no row element are hidden
    bool customHeight = false;
};

// `sheetView`: how the sheet was left on the screen.
struct SheetView {
    bool showGridLines = true;
    bool showHeadings = true;
    bool showZeros = true;
    bool rightToLeft = false;
    bool tabSelected = false;
    int zoom = 100;                 // percent
    bool frozen = false;            // the first rows and columns stay in place (a split that is not frozen is not kept)
    quint32 frozenRows = 0, frozenCols = 0;
    quint32 topRow = 0, leftCol = 0;           // the cell at the top left of what shows (of the frozen part, when there is one)
    quint32 paneRow = 0, paneCol = 0;          // the cell at the top left of the part that scrolls, when frozen
    quint32 activeRow = 0, activeCol = 0;      // the cell the sheet was left on
};

struct Sheet {
    CellStore cells;
    QVector<QString> texts;           // inline strings and the texts of formula results
    QVector<QString> formulaTexts;    // as written, and no "=" in front
    QVector<Formula> formulas;
    QVector<CellRange> ranges;        // the cells of array formulas and data tables
    CellRange dimension;              // what the part says the used cells are (it may be wrong, or missing)
    qint64 mergeCount = 0;            // merged ranges
    QVector<CellRange> merges;        // the merged ranges read (as many as the limit allows), in the order of the file
    QVector<ColumnSpan> columns;      // in the order of the file
    QVector<RowSpec> rowSpecs;        // the rows that are not as the default, in the order of the file
    SheetFormat format;
    SheetView view;
    ColorSpec tabColor;               // the color of the sheet's tab
    bool isProtected = false;         // the sheet is protected (sheetProtection): its locked cells can't be changed
    bool partial = false;             // the read was stopped at a row asked for (ReadOptions::stopAfterRow): the cells of the rows after it, and what follows the cells (the merged ranges), are not here
    quint32 extentRows = 0;           // the rows and columns the cells reach: the most of a cell that holds a value or a formula or has a format of its own (a style other than the first)
    quint32 extentCols = 0;
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
