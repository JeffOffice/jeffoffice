#pragma once
// The grid's model of one sheet: the geometry the file gives it (sizes of rows and columns, hidden ones, merged ranges, frozen panes, zoom), how each cell looks, the value each
// cell shows (from the calculation once there is one, else the result the file stored), and the text of each value in its format, kept in a cache so that a screen is drawn from
// text made once. It reads the sheet and the workbook's styles and shared strings where they are (nothing is copied), and changes nothing in them.

#include "grid/condformat.h"
#include "grid/layout.h"
#include "grid/look.h"
#include "grid/text.h"
#include "grid/valuestore.h"
#include "sheet/calculation.h"
#include "sheet/sheetmodel.h"
#include "sheet/styles.h"
#include "sheet/tablestyle.h"

#include <memory>
#include <unordered_map>

namespace jp::grid {

using jp::office::sheet::Cell;
using jp::office::sheet::Sheet;
using jp::office::sheet::SharedStrings;
using jp::office::sheet::Styles;

// What a sheet's file says about its rows, columns, merged ranges, frozen panes, zoom, and scroll, as the grid has it, and the formats that rows and columns give the cells that are
// not there. It is made from the sheet and the styles alone, so a worker can make it ahead and the window take it in a moment (a sheet may name a million rows and a million merges).
struct GridGeometry {
    GridLayout layout;
    std::vector<jp::office::sheet::ColumnSpan> columnStyles;
    std::vector<jp::office::sheet::RowSpec> rowStyles;
};

// What the model reads. The sheet, styles, and strings must stay where they are while it does. With a calculation, a cell shows its value (a formula still to be worked out, the
// result the file stored); without, what the file stored.
struct GridData {
    const Styles *styles = nullptr;
    const SharedStrings *strings = nullptr;
    const Sheet *sheet = nullptr;
    const jp::office::sheet::Calculation *calc = nullptr;   // read directly (only where nothing else is changing it: a tool, a test)
    const ValueStore *values = nullptr;                      // what a calculation on another thread has worked out and told (the window's way): a formula not in it shows what the file stored
    int sheetIndex = 0;
    // The other sheets and the names of all, for the formulas of conditional formats that read cells of another sheet (none: they read only this one).
    std::vector<const Sheet *> sheets;
    QStringList sheetNames;
    jp::calc::DateSystem dateSystem = jp::calc::DateSystem::Date1900;
    std::shared_ptr<const GridGeometry> geometry;   // the sheet's geometry, made ahead by GridSheet::makeGeometry; none: setData makes it
};

class GridSheet {
public:
    // The most texts held at once; a screen shows a few thousand cells, and more asked for lets all the rest go.
    static constexpr size_t kMaxTexts = 30000;

    GridSheet();
    // Builds the geometry from the sheet and forgets the texts.
    void setData(const GridData &data);
    const GridData &data() const { return m_data; }
    // How text is measured (a fixed width of 7 for each character until one is given).
    void setMeter(const TextMeter *meter);

    GridLayout &layout() { return m_layout; }
    const GridLayout &layout() const { return m_layout; }
    const LookTable &looks() const { return m_looks; }

    // The cell, or null.
    const Cell *cellAt(quint32 row, quint32 col) const;
    // The value of a cell as it is shown.
    jp::calc::Value valueOf(quint32 row, quint32 col, const Cell &cell) const;
    // The value of a cell of this sheet or of another (as it is shown; empty for none), and a sheet by its place (null for none).
    jp::calc::Value valueIn(int sheet, quint32 row, quint32 col) const;
    const Sheet *sheetAt(int sheet) const;
    // ---- conditional formats (see condformat.h)
    bool hasConditionalFormats() const { return m_cond.any(); }
    const CondFormats &conditional() const { return m_cond; }
    CondFormats &conditional() { return m_cond; }
    // The sheet's rules are new or changed: what was worked out of them, and the looks made from it, are let go. (Use this, not conditional().reset().)
    void resetConditional() const;
    // The day the rules of dates and TODAY() read (a day number; the clock's by default).
    void setToday(double serial) const;
    // The look of a cell with the changes the rules of conditional formatting make to it (the cell's own, when there are none).
    const CellLook &lookOf(quint32 row, quint32 col, quint32 style) const;
    // ---- table styles (sheet/tablestyle.h): the cells of a table are drawn as its style says, under what the cell formats itself (a fill, borders, a text color, or bold of the cell's own stay)
    struct TableSkin {
        jp::office::sheet::Table table;
        const jp::office::sheet::TableStyle *style = nullptr;   // held by whoever set it
    };
    static constexpr size_t kMaxTableSkins = 500;
    void setTableSkins(std::vector<TableSkin> skins);
    bool hasTableSkins() const { return !m_skins.empty(); }
    bool inTableSkin(quint32 row, quint32 col) const
    {
        for (const TableSkin &skin : m_skins)
            if (skin.style && row >= skin.table.row1 && row <= skin.table.row2 && col >= skin.table.col1 && col <= skin.table.col2) return true;
        return false;
    }
    // The format of a place: its cell's, else its row's when the row has one, else its column's; 0 for none.
    quint32 styleOf(quint32 row, quint32 col, const Cell *cell) const;
    // The width of a column in the grid's units at 100% (0 when hidden), and of a block of columns.
    double columnWidth(qint64 col) const;

    // What the cell shows. The width it is shown in is its column's, or its merged range's. Cached by the cell's place.
    Shown shown(quint32 row, quint32 col, const Cell &cell) const;
    // The text of a cell for a copy: what it holds as its format shows it, not cut to its cell (a number is never "#", a text keeps its line breaks and all its letters).
    QString textOf(quint32 row, quint32 col, const Cell &cell) const;
    // Forgets the texts (values or formats changed), or the text of one cell.
    void invalidateTexts() const;
    // How a cell that holds an error is shown (a printed page can show it as it is, blank, as "--", or as #N/A).
    enum class ErrorDisplay { Shown, Blank, Dash, NA };
    void setErrorDisplay(ErrorDisplay mode);
    // Forgets the texts and what the rules of conditional formatting made of the cells, but not the figures they keep of all their cells (those are let go by `invalidateText` of a cell
    // that is among theirs): for values that changed, one by one, that a rule's formula may read.
    void invalidateLooks() const;
    void invalidateText(quint32 row, quint32 col) const
    {
        m_texts.erase((quint64(row) << 16) | col);
        m_cond.valueChanged(row, col);
    }
    // Shows the formula of a cell that has one in place of its value ("=SUM(A1:A3)", as text).
    // The sheet reads right to left: column A is at the right, and what a cell's alignment says of the left and right is the screen's. (The `align` a cell shows is in the grid's own places,
    // the way the layout puts the columns: a sheet that reads right to left has them mirrored for the screen, so a text aligned to its left has `align` right.)
    bool rightToLeft() const { return m_rtl; }
    // The marks the person's language puts in numbers (the decimal mark, the grouping of digits): what a number cell shows and a copy gives. Plain ("." and ",") until set.
    void setNumberSymbols(const jp::calc::NumberSymbols &symbols);
    const jp::calc::NumberSymbols &numberSymbols() const { return m_symbols; }
    void setShowFormulas(bool on);
    bool showFormulas() const { return m_showFormulas; }
    // Texts made (not found in the cache) so far, and held now.
    qint64 textsMade() const { return m_textsMade; }
    size_t textsHeld() const { return m_texts.size(); }

    // The rows and columns the cells reach (the most of a cell with a value, a formula, or a format of its own), for Ctrl+End.
    void usedExtent(quint32 *rows, quint32 *cols) const;

    // The sheet has rows or columns with a format of their own (which the cells that are not there have).
    bool hasImplicitStyles() const { return m_geometry && (!m_geometry->columnStyles.empty() || !m_geometry->rowStyles.empty()); }

    // The geometry a sheet's file gives (the sheet's own view: its zoom, frozen panes, and where it was scrolled to); the work setData does when it is not given one.
    static std::shared_ptr<GridGeometry> makeGeometry(const Sheet &sheet, const Styles &styles);
    // What showing a sheet again rebuilds, counted (for tests and tools): the sizes of rows and columns that makeGeometry has put in a layout, and the texts of cells that setData has let go of.
    static qint64 viewCellsRebuiltForTest();

    // The column (row) a sheet's width (height) in characters (points) is, in the grid's units; the conversions the file's numbers go through.
    static int columnPixels(double chars, int maxDigitWidth);
    static int rowPixels(double points);
    // And the other way: the width a file gives a column of this many pixels (so that columnPixels gives them back), the height in points of a row, and the width a person is shown and
    // types (the number of digits that fit: the file's width less the padding).
    static double widthForPixels(int pixels, int maxDigitWidth);
    static double pointsForPixels(int pixels);
    static double shownWidth(double fileWidth, int maxDigitWidth);
    static double widthOfShown(double shown, int maxDigitWidth);

    // What a column needs to show what is in it (the width the file would give it, with its padding), and what a row needs (points): the widest text of the cells in the column that
    // are not merged, and the tallest line or lines of the cells in the row. Zero when the column (row) holds nothing, the column's (row's) own size then standing. Texts that wrap do
    // not widen a column. For a few thousand cells: the walk stops at `limit` cells.
    double fitWidth(quint32 col, qint64 limit = 200000) const;
    double fitHeight(quint32 row, qint64 limit = 20000) const;
    // The height (in points) that the text of the row needs when its height is the program's to choose: as many lines as each cell shows (a wrapped cell as many as its width needs, a
    // cell of several lines that wraps one for each), each as tall as a line of the cell's font, where a line of the sheet's standard font is the sheet's standard row height. 0 if the
    // row holds nothing. Looks at most `limit` cells.
    // `work`, if given, is added the work done: 20 for each cell shown (its value made into the text it shows) and a unit for each character measured to wrap it.
    double autoHeight(quint32 row, qint64 limit = 2000, qint64 *work = nullptr) const;
    int wrappedLines(const QString &text, const CellLook &look, double room) const;
    // The width that shows one cell's text whole (a number typed in it), 0 for a cell that holds nothing.
    double fitWidthOfCell(quint32 row, quint32 col) const;

private:
    GridData m_data;
    GridLayout m_layout;
    LookTable m_looks;
    const TextMeter *m_meter = nullptr;
    mutable std::unordered_map<quint64, Shown> m_texts;
    mutable qint64 m_textsMade = 0;
    bool m_showFormulas = false;
    ErrorDisplay m_errors = ErrorDisplay::Shown;
    jp::calc::NumberSymbols m_symbols;
    bool m_rtl = false;
    mutable CondFormats m_cond{this};
    std::vector<TableSkin> m_skins;
    jp::calc::Value valueOfCell(int sheetIndex, quint32 row, quint32 col, const Sheet &sheet, const Cell &cell) const;
    std::shared_ptr<const GridGeometry> m_geometry;   // what the sheet's file gave (the layout above begins as a copy of its layout, which shares its places and merges), and the cell formats of the rows and columns, held where they were made
};

} // namespace jp::grid
