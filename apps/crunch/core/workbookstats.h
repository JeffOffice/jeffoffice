#pragma once
// Workbook Statistics: how many sheets, cells with data, formulas, and pivot tables a workbook holds (the charts are counted by the window, which knows the drawings).

#include "core/editsession.h"

namespace jp::crunch {

struct WorkbookStats {
    int sheets = 0;          // sheets that are not deleted
    int hiddenSheets = 0;    // of them, those the workbook hides
    qint64 cells = 0;        // cells that hold a value or a formula
    qint64 formulas = 0;     // cells that hold a formula (an array formula once)
    int pivotTables = 0;
};

WorkbookStats workbookStatistics(const EditSession &session);

} // namespace jp::crunch
