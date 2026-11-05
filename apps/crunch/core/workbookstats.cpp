#include "core/workbookstats.h"

#include "sheet/calculation.h"
#include "sheet/pivotbook.h"

namespace jp::crunch {

using namespace jp::office::sheet;

WorkbookStats workbookStatistics(const EditSession &session)
{
    WorkbookStats out;
    const Workbook *book = session.workbook();
    const Calculation *calc = session.calculation();
    if (!book) return out;
    const auto &sheets = book->info().sheets;
    for (int i = 0; i < int(sheets.size()); ++i) {
        if (sheets[size_t(i)].deleted) continue;
        ++out.sheets;
        if (sheets[size_t(i)].state != SheetState::Visible) ++out.hiddenSheets;
        if (!calc || i >= calc->sheetCount()) continue;
        calc->sheet(i).cells.forEach([&](quint32, quint32, const Cell &c) {
            if (c.flags & ArrayMember) return;
            if (c.hasFormula()) {
                ++out.formulas;
                ++out.cells;
            } else if (c.type != CellType::Empty) {
                ++out.cells;
            }
        });
    }
    out.pivotTables = book->pivots().tableCount();
    return out;
}

} // namespace jp::crunch
