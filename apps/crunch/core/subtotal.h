#pragma once
// Subtotal: in a list sorted by one column, a row of SUBTOTAL formulas under each run of equal entries in that column, and a grand total under the list; the rows of each run are made a group
// of the sheet's outline. Remove All takes those rows away again. Each is one step of Undo.

#include "core/editsession.h"

#include <QCoreApplication>
#include <vector>

namespace jp::crunch {

// The functions SUBTOTAL has, by the number it is called with (the rows hidden by hand count).
struct SubtotalFunction {
    int code;
    const char *name;
};
const std::vector<SubtotalFunction> &subtotalFunctions();

struct SubtotalSpec {
    quint32 column = 0;                // the column whose changes make a new run (a column of the sheet)
    int function = 9;                  // a number from subtotalFunctions (9 is Sum)
    std::vector<quint32> columns;      // the columns to add a subtotal to (columns of the sheet)
    bool replace = true;               // subtotals already in the list are taken away first
};

// `region` is the list with its header row (the first row is not totaled). On success `groups` is the number of runs that got a subtotal row. Not ok, and nothing changed, if the list is
// empty, has no header row to be below, or has too many runs.
EditSession::Outcome applySubtotals(EditSession &session, int sheet, jp::office::sheet::CellRange region, const SubtotalSpec &spec, int *groups = nullptr);
// Takes away the rows that hold only text and SUBTOTAL formulas (one at least) inside `region`, and the outline of the rows left; `removed` is how many rows went.
EditSession::Outcome removeSubtotals(EditSession &session, int sheet, jp::office::sheet::CellRange region, int *removed = nullptr);

} // namespace jp::crunch
