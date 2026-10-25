#pragma once
// The cells of a workbook as a chart reads them (libs/office/sheet/chartdata.h): the value a cell shows now (a formula's from the values the calculation has told the window, else the
// result the file stored; a constant as it is held), the text its number format makes of it, the rows and columns that are hidden, and the names the workbook defines. It reads where
// the grid reads, beside the worker that calculates, and holds nothing of the workbook but what it is given.

#include "grid/look.h"
#include "grid/valuestore.h"
#include "sheet/chartdata.h"
#include "sheet/recalc.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace jp::grid {

class WorkbookChartCells : public jp::office::sheet::ChartCells {
public:
    // `source` has the sheets (a Calculation is one), `values` the results worked out so far (may be null), and `styles` the number formats; all must outlive this.
    WorkbookChartCells(const jp::office::sheet::WorkbookSource *source, const ValueStore *values, const jp::office::sheet::Styles *styles, jp::calc::DateSystem dateSystem);

    int sheetCount() const override;
    int sheetIndex(const QString &name) const override;
    jp::calc::Value value(int sheet, quint32 row, quint32 col) const override;
    QString text(int sheet, quint32 row, quint32 col) const override;
    QString formatCode(int sheet, quint32 row, quint32 col) const override;
    bool rowHidden(int sheet, quint32 row) const override;
    bool columnHidden(int sheet, quint32 col) const override;
    void extent(int sheet, quint32 *rows, quint32 *cols) const override;
    QString definedName(const QString &name, int sheet) const override;

    // The sizes of rows and columns changed (a row was hidden or shown): the hidden ones are found again when asked.
    void sizesChanged();

private:
    struct Hidden {
        bool made = false;
        std::unordered_set<quint32> rows;
        std::vector<std::pair<quint32, quint32>> columns;   // runs, both ends in them
    };
    const Hidden &hidden(int sheet) const;
    const jp::office::sheet::Cell *cellAt(int sheet, quint32 row, quint32 col) const;
    jp::calc::Value valueOf(int sheet, const jp::office::sheet::Cell &cell) const;

    const jp::office::sheet::WorkbookSource *m_source;
    const ValueStore *m_values;
    const jp::office::sheet::Styles *m_styles;
    jp::calc::DateSystem m_dateSystem;
    LookTable m_looks;
    mutable std::unordered_map<int, Hidden> m_hidden;
};

} // namespace jp::grid
