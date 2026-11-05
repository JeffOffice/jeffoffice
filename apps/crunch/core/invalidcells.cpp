#include "core/invalidcells.h"

#include <algorithm>

namespace jp::crunch {

using namespace jp::office::sheet;

std::vector<CellRange> invalidCells(const Calculation &calc, int sheet, bool *truncated)
{
    std::vector<CellRange> out;
    if (truncated) *truncated = false;
    if (sheet < 0 || sheet >= calc.sheetCount() || calc.sheet(sheet).validations.isEmpty()) return out;
    bool full = false;
    calc.sheet(sheet).cells.forEach([&](quint32 row, quint32 col, const Cell &c) {
        if (full || (c.type == CellType::Empty && !c.hasFormula())) return;
        const Validation *rule = calc.validationAt(sheet, row, col);
        if (!rule || rule->type == Validation::Type::Any) return;
        const Calculation::CellContent content = calc.content(sheet, row, col);
        if (!content.exists || content.value.isEmpty()) return;
        if (calc.entryAllowed(sheet, row, col, *rule, content.value, QString())) return;
        if (out.size() >= kMaxInvalidCells) {
            full = true;
            if (truncated) *truncated = true;
            return;
        }
        CellRange r;
        r.valid = true;
        r.row1 = r.row2 = row;
        r.col1 = r.col2 = col;
        out.push_back(r);
    });
    std::sort(out.begin(), out.end(), [](const CellRange &a, const CellRange &b) { return a.row1 != b.row1 ? a.row1 < b.row1 : a.col1 < b.col1; });
    return out;
}

} // namespace jp::crunch
