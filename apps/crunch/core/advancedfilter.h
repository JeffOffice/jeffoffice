#pragma once
// Advanced Filter: the rows of a list that meet conditions written on the sheet. The conditions are a range whose first row repeats column headings of the list and whose other rows are the
// conditions: the cells of a row must all hold (And), and a row of them is enough (Or). A condition is a value (text is matched at the start, `?` and `*` stand for characters), or a value
// after `=`, `<>`, `<`, `<=`, `>`, or `>=`; `=` alone means an empty cell. A column heading not found in the list is left out; a row with no conditions in it lets every row through.

#include "core/editsession.h"

#include <vector>

namespace jp::crunch {

struct AdvancedFilterSpec {
    jp::office::sheet::CellRange list;       // with its heading row
    jp::office::sheet::CellRange criteria;   // with its heading row
    bool unique = false;                     // a row equal to an earlier row that met the conditions is left out
};

struct AdvancedFilterResult {
    bool ok = true;
    QString problem;
    std::vector<quint32> rows;               // the rows of the list (not its heading row) that meet the conditions, in order
};

AdvancedFilterResult advancedFilterRows(const EditSession &session, int sheet, const AdvancedFilterSpec &spec);

} // namespace jp::crunch
