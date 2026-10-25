#pragma once
// Making a chart from cells: which of the cells a person selected are the series' names, the categories, and the values (the first row and column are headers when they look like
// them), how many series there are and whether they run down the columns or along the rows, and the chart model for a kind of chart with those series. The chart reads the cells by
// reference (the c:f of a chart part), so it follows them when they change; its numbers are put in by chartdata.h when it is drawn.
//
// Bounded: a selection of any size gives at most kMaxMadeSeries series of at most kMaxChartSeriesCells points, and not more than kMaxChartCells points between them.

#include "sheet/chartdata.h"

#include "office/chart_write.h"
#include "office/theme.h"

#include <QString>
#include <QVector>

namespace jp::office::sheet {

enum class ChartKind : quint8 {
    Column, ColumnStacked, ColumnPercent, Bar, BarStacked, BarPercent, Line, LineMarkers, LineStacked, Pie, Doughnut, Area, AreaStacked, Scatter, ScatterLines, Combo
};

// Where a series reads (empty: none).
struct ChartSeriesRefs {
    QString nameRef, categoryRef, valueRef, xRef;
};

struct ChartPlan {
    QVector<ChartSeriesRefs> series;
    bool seriesInRows = false;      // each series is a row (else a column)
    bool headerRow = false, headerColumn = false;   // the first row holds the names, and the first column the categories (a scatter's x values)
    CellRange data;                 // the cells the values are in (without the headers)
    bool truncated = false;         // the selection was larger than a chart reads
    int points = 0;                 // the points of each series
};

constexpr int kMaxMadeSeries = 255;

// A reference as a chart part writes it: the sheet's name (in quotes when it needs them), then the cells with their dollar signs ("'My Sheet'!$B$2:$B$9", "Sheet1!$B$1").
QString chartRef(const QString &sheetName, const CellRange &range);

// The block of cells a selection stands for: a cell is the block around it that has something in it, whole columns or rows go as far as the sheet's cells do, and the empty rows and columns at the
// edges are left off. Invalid if there is none.
CellRange chartBlockOf(const ChartCells &cells, int sheet, const CellRange &selection, bool *truncated = nullptr);

// The plan for the cells selected on `sheet` (called `sheetName`). A selection of one cell is the block of cells around it that has something in it; whole columns or rows go as far as the
// sheet's cells do. `inRows`: 0 the series are columns, 1 they are rows, -1 whichever the shape of the selection says (more columns than rows: rows). The plan has no series if the selection
// has nothing to chart.
ChartPlan planChart(const ChartCells &cells, int sheet, const QString &sheetName, const CellRange &selection, ChartKind kind, int inRows = -1);

// The same, from a block of cells given as the data (no headers are looked for: `headerRow` and `headerColumn` say whether the block's first row and column are the names and categories).
ChartPlan planChartFrom(const QString &sheetName, const CellRange &block, bool headerRow, bool headerColumn, bool inRows, ChartKind kind);

// A chart of the kind for the plan: the kind's plots and axes, the legend and title a new chart has, the series reading the plan's references, in the theme's colors. The pie's slices
// are given a color each (`slices` of them, at most 256).
jp::office::Chart makeChart(ChartKind kind, const ChartPlan &plan, const jp::office::Theme &theme, int slices = 0);

// What a series is drawn as: a kind of chart, and whether against the second pair of axes.
struct SeriesKind {
    ChartKind kind = ChartKind::Column;
    bool secondary = false;
};

// A chart for series of any kinds: the series are grouped into plots by what they are drawn as (areas, then bars, then lines, then points; those against the second axes after), with the
// axes they need. A pie or doughnut is its series alone, and bars across with other kinds are columns. `like`, if given, is a chart whose look is kept: its title, legend, page, and the
// titles, gridlines, and scale of its first axes.
jp::office::Chart chartFromSeries(const QVector<jp::office::ChartSeries> &series, const QVector<SeriesKind> &kinds, const jp::office::Theme &theme, const jp::office::Chart *like = nullptr);

// The kind of one plot, as this makes it.
ChartKind kindOfPlot(const jp::office::ChartPlot &plot);

// The kind of the chart's first plot, as this makes it: the nearest of ChartKind for what the chart is (stacked, percent, markers, horizontal); a combo when there are two kinds of plot.
ChartKind kindOfChart(const jp::office::Chart &chart);

} // namespace jp::office::sheet
