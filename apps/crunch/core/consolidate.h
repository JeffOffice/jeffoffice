#pragma once
// Consolidate: ranges from one or several sheets combined into one table with a function (sum, count, average, and the others). By position, the cell at the same place in each range is
// combined. By labels, the first row and/or the first column of each range names what a cell is, and cells with the same labels are combined, whatever their places. With links, each result
// is a formula that names its source cells, so it follows them; without, it is the number.

#include "core/editsession.h"

#include <functional>
#include <vector>

namespace jp::crunch {

struct ConsolidateSource {
    int sheet = 0;
    jp::office::sheet::CellRange range;
};

struct ConsolidateSpec {
    std::vector<ConsolidateSource> sources;
    int function = 9;                 // the number SUBTOTAL is called with (9 sum, 1 average, 3 count, ...)
    bool topLabels = false;           // the first row of each range holds the names of its columns
    bool leftLabels = false;          // the first column of each range holds the names of its rows
    bool links = false;               // the results are formulas that read the source cells
    int sheet = 0;                    // where the results go: from this cell
    quint32 row = 0, col = 0;
};

constexpr size_t kMaxConsolidateSources = 255;

// One step of Undo. `written` is the cells written. Not ok, and nothing changed, if there are no sources, the results would not fit, or cells could not be changed.
// `beforeEnd` is called just before the step ends (see applySubtotals).
EditSession::Outcome consolidate(EditSession &session, const ConsolidateSpec &spec, jp::office::sheet::CellRange *written = nullptr, const std::function<void()> &beforeEnd = nullptr);

} // namespace jp::crunch
