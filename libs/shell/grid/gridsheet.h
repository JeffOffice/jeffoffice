#pragma once
// The grid's model of one sheet: the geometry the file gives it (sizes of rows and columns, hidden ones, merged ranges, frozen panes, zoom), how each cell looks, the value each
// cell shows (from the calculation once there is one, else the result the file stored), and the text of each value in its format, kept in a cache so that a screen is drawn from
// text made once. It reads the sheet and the workbook's styles and shared strings where they are (nothing is copied), and changes nothing in them.

#include "grid/layout.h"
#include "grid/look.h"
#include "grid/text.h"
#include "sheet/calculation.h"
#include "sheet/sheetmodel.h"
#include "sheet/styles.h"

#include <unordered_map>

namespace jp::grid {

using jp::office::sheet::Cell;
using jp::office::sheet::Sheet;
using jp::office::sheet::SharedStrings;
using jp::office::sheet::Styles;

// What the model reads. The sheet, styles, and strings must stay where they are while it does. With a calculation, a cell shows its value (a formula still to be worked out, the
// result the file stored); without, what the file stored.
struct GridData {
    const Styles *styles = nullptr;
    const SharedStrings *strings = nullptr;
    const Sheet *sheet = nullptr;
    const jp::office::sheet::Calculation *calc = nullptr;
    int sheetIndex = 0;
    jp::calc::DateSystem dateSystem = jp::calc::DateSystem::Date1900;
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
    jp::calc::Value valueOf(const Cell &cell) const;
    // The format of a place: its cell's, else its row's when the row has one, else its column's; 0 for none.
    quint32 styleOf(quint32 row, quint32 col, const Cell *cell) const;
    // The width of a column in the grid's units at 100% (0 when hidden), and of a block of columns.
    double columnWidth(qint64 col) const;

    // What the cell shows. The width it is shown in is its column's, or its merged range's. Cached by the cell's place.
    Shown shown(quint32 row, quint32 col, const Cell &cell) const;
    // Forgets the texts (values or formats changed).
    void invalidateTexts() const;
    // Shows the formula of a cell that has one in place of its value ("=SUM(A1:A3)", as text).
    void setShowFormulas(bool on);
    bool showFormulas() const { return m_showFormulas; }
    // Texts made (not found in the cache) so far, and held now.
    qint64 textsMade() const { return m_textsMade; }
    size_t textsHeld() const { return m_texts.size(); }

    // The rows and columns the cells reach (the most of a cell with a value, a formula, or a format of its own), for Ctrl+End.
    void usedExtent(quint32 *rows, quint32 *cols) const;

    // The sheet has rows or columns with a format of their own (which the cells that are not there have).
    bool hasImplicitStyles() const { return !m_columnStyles.empty() || !m_rowStyles.empty(); }

    // The column (row) a sheet's width (height) in characters (points) is, in the grid's units; the conversions the file's numbers go through.
    static int columnPixels(double chars, int maxDigitWidth);
    static int rowPixels(double points);

private:
    void buildGeometry();
    GridData m_data;
    GridLayout m_layout;
    LookTable m_looks;
    const TextMeter *m_meter = nullptr;
    mutable std::unordered_map<quint64, Shown> m_texts;
    mutable qint64 m_textsMade = 0;
    bool m_showFormulas = false;
    std::vector<jp::office::sheet::ColumnSpan> m_columnStyles;   // by place of the run: the cell formats of columns, in order of first column (for places with no cell)
    std::vector<jp::office::sheet::RowSpec> m_rowStyles;         // the rows with a format of their own, in order
};

} // namespace jp::grid
