#pragma once
// Go To: a reference a person types (a cell or a range, with or without a sheet, whole rows or columns, or the name of a defined name) found in the workbook, and Go To Special: the cells
// of a sheet that are of a kind (blank, constants, formulas, errors, the current region, the last cell, those that show, those with conditional formats or a rule of validation) as
// ranges. All of it is bounded: the work is counted in cells and rows looked at, and what a search finds is at most kMaxSpecialRanges ranges.

#include "core/editsession.h"

#include <QString>
#include <QVector>

namespace jp::crunch {

constexpr int kMaxSpecialRanges = 10000;
constexpr qint64 kMaxSpecialWork = 3000000;

struct GoToTarget {
    bool ok = false;
    int sheet = 0;
    jp::office::sheet::CellRange range;
    bool wholeRows = false, wholeColumns = false;
    QString problem;       // why not, in a sentence
};
// Finds what `text` names: "B3", "B3:D9", "$B$3", "Sheet2!A1", "'My Sheet'!A1:B2", "A:C", "3:7", or a defined name (one of the sheet, or of the workbook) that stands for a range.
// `sheet` is the sheet in view (where a reference with no sheet is).
GoToTarget resolveGoTo(const EditSession &session, const QString &text, int sheet);
// The defined names a person can go to (those that stand for a range), by name, with what they stand for.
QVector<QPair<QString, QString>> goToNames(const EditSession &session, int sheet);

enum class SpecialKind { Blanks, Constants, Formulas, Errors, CurrentRegion, LastCell, VisibleCells, ConditionalFormats, DataValidation };
struct SpecialOptions {
    SpecialKind kind = SpecialKind::Blanks;
    // Constants and Formulas: which results count
    bool numbers = true, text = true, logicals = true, errors = true;
    // DataValidation: only the cells whose rule is the one the active cell has
    bool sameRule = false;
};
struct SpecialResult {
    QVector<jp::office::sheet::CellRange> ranges;   // in order of row, then column
    qint64 cells = 0;
    bool truncated = false;     // a limit stopped the search
    QString problem;            // nothing found: why, in a sentence ("No cells were found." when there are none)
};
// The cells of the sheet that are of the kind, within `selection` (when it is more than one cell; else the whole sheet's used cells). `active` is the cell with the cursor (the current
// region grows from it, and the rule it has is the one "same" means).
SpecialResult goToSpecial(const EditSession &session, int sheet, const jp::office::sheet::CellRange &selection, quint32 activeRow, quint32 activeCol, const SpecialOptions &options);

// The block of cells around a cell that has no empty row and column between its cells (the block Ctrl+A selects, and what sorting and filtering take when one cell is selected); the cell
// itself if it and its neighbors are empty. `truncated` is set if the work limit was reached (the block found so far is given).
jp::office::sheet::CellRange currentRegion(const EditSession &session, int sheet, quint32 row, quint32 col, bool *truncated = nullptr);

} // namespace jp::crunch
