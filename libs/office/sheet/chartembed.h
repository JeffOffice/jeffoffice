#pragma once
// A chart that another program saved keeps its series as references (c:f) into a workbook kept beside the chart (c:externalData, an .xlsx part). This reads the cells those references
// name, as the grid of the chart's data (ChartData: the words under the points, the names of the series, the numbers), and writes an edit of the grid back: the changed cells in the
// workbook (its other cells, sheets, and parts stay as they are: sheet/xlsxsave.h) and the values the chart stores of them (the number and string caches of its series), so that other
// programs and our drawing agree. The grid keeps the shape the references give: no row or column is added or taken away.
//
// Followed only when each reference is plain: one block of one sheet that is there, one column (or one row) of the same length for every series, the numbers in cells that hold
// numbers (no formula, date, text, or blank), names and words in cells that hold words or plain (General) numbers, or none; no cell read for two things; nothing past the sheet's cells; else
// the chart's numbers stay read-only, with the reason (embeddedChartProblem). A workbook comes from a file, so it is bounded: at most kMostEmbeddedWorkbookBytes of file, at most
// kMostEmbeddedInflatedBytes once inflated, at most kMostEmbeddedSheetCells cells in the sheet read, and no more series and points than a chart made here has (kMostChartSeries and
// kMostChartCategories). A number past +-1e15 can't be a chart's. After an edit the saved workbook asks to be worked out again when it is opened (fullCalcOnLoad), so that formulas of
// its that read the cells show the new numbers.

#include "office/chart.h"
#include "sheet/chartops.h"

#include <QByteArray>
#include <QString>

namespace jp::office::sheet {

constexpr qint64 kMostEmbeddedWorkbookBytes = kMostChartBytesInFile;   // 16 MB, the most the chart parts of one file come to
constexpr qint64 kMostEmbeddedInflatedBytes = qint64(64) << 20;
constexpr qint64 kMostEmbeddedSheetCells = 1000000;                   // cells of the sheet that are read (Limits::cellsPerSheet: a cell with only a format counts)
// How many workbooks have been opened to say what a chart's cells hold, on this thread since the program began: a count for tests of cost (a chart is asked about again and again, and its
// workbook is opened once for each workbook and set of references).
qint64 embeddedWorkbooksOpenedForTest();

// Why the numbers of a chart that keeps a workbook can't be edited, in a sentence; empty when they can, and then `data` (when given) is the grid read from the workbook's cells.
// `workbook` is the .xlsx part's bytes (empty: none to be read).
QString embeddedChartProblem(const Chart &chart, const QByteArray &workbook, ChartData *data = nullptr);

// The edit: the chart's stored values and the workbook's cells made as `data` says (the same shape the grid had). `newWorkbook` is the workbook's bytes after. False, nothing changed, when
// the chart can't be followed (embeddedChartProblem says why), the data has another shape, or it is not usable. A chart and workbook that are changed only in what the data changed.
bool embeddedChartSetData(Chart &chart, const QByteArray &workbook, const ChartData &data, QByteArray *newWorkbook);

} // namespace jp::office::sheet
