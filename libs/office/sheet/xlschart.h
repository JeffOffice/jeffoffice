#pragma once
// The charts of a workbook in the old binary format, read into the chart model the XML form fills (libs/office/office/chart.h). A chart's records are those between the start and the end of
// its substream: the series (each with its name, values, and categories, as references to cells or as values the chart holds), the groups of series drawn the same way (columns, bars, lines,
// areas, pies, scatters), the axes, the titles, and the legend. What the model cannot hold (3-D looks, radar and surface and bubble charts, fonts, fills of the chart's parts) is left out and
// said to be. Every count and length is checked against the bytes of its record, nothing recurses, and a chart keeps at most the series and points the XML reader keeps.

#include "office/chart.h"
#include "sheet/xlsformula.h"

#include <QByteArray>
#include <QString>

namespace jp::office::sheet {

// The most series a chart is read with, and the most literal values (or categories) of one series.
constexpr int kMaxXlsChartSeries = 256;
constexpr int kMaxXlsChartPoints = 10000;

struct XlsChartRead {
    office::Chart chart;
    bool read = false;     // the chart has a kind of drawing the model holds and a series to draw
    bool lost = false;     // something of it was left out (a group of another kind, a 3-D look, series past the limit)
};

// Reads the records of the chart substream at [from, to) of `data` (the bytes of the workbook's stream; the range begins after the start of the substream). `cx` is what the
// references in the chart's formulas need, and `ownSheet` is the name a reference with no sheet takes.
XlsChartRead readXlsChart(const QByteArray &data, qsizetype from, qsizetype to, const XlsContext &cx, const QString &ownSheet);

// Where a drawing record of a worksheet puts the object it is for: the columns and rows (from 0), and how far into the first and last cell, as the record has it (in 1024ths of a column and
// 256ths of a row: the size of the cell is not known here). False if the record holds no anchor.
struct XlsAnchor {
    quint32 col1 = 0, row1 = 0, col2 = 0, row2 = 0;
    quint32 dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0;
    bool absolute = false;   // a chart sheet's chart: no cells, the size of a page
};
bool readXlsAnchor(const QByteArray &data, qsizetype from, qsizetype to, XlsAnchor *out);

} // namespace jp::office::sheet
