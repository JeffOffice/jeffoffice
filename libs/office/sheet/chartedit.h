#pragma once
// Changing a chart's model as a person does from the chart's menus and dialogs: its title, axes, gridlines, legend, and data labels on or off and placed; its colors and look from a
// small set of our own; its kind, for the chart and for each series (columns with a line over them, a series on the second axes); and its series (added, removed, put in another order).
// Each function changes the model it is given and nothing else; saving writes the model (office/chart_write.h).

#include "sheet/chartmake.h"

#include <QColor>
#include <QString>
#include <QStringList>
#include <QVector>

namespace jp::office::sheet {

// ---- what the chart shows now (for the checks of a menu)
struct ChartElements {
    bool title = false, titleOverlay = false;
    bool axisHorizontal = false, axisVertical = false;          // shown (a chart with none of that axis: false)
    bool hasHorizontal = false, hasVertical = false;            // the chart has the axis at all (a pie has neither)
    bool axisTitleHorizontal = false, axisTitleVertical = false;
    bool gridHorizontal = false, gridVertical = false, minorGridHorizontal = false, minorGridVertical = false;   // lines across the chart, and lines up and down it
    bool legend = false;
    QString legendPosition;                                     // "r", "l", "t", "b", or empty
    bool labels = false;
    QString labelPosition;                                      // as the labels say it ("outEnd"), empty for the usual
    bool legendOverlay = false;                                 // the legend lays over the plot
    // The axes of the plots drawn against a second pair (a series on the second axes): the chart has them, they are shown, and have titles.
    bool hasSecondaryHorizontal = false, hasSecondaryVertical = false;
    bool axisSecondaryHorizontal = false, axisSecondaryVertical = false;
    bool axisTitleSecondaryHorizontal = false, axisTitleSecondaryVertical = false;
    // The trendline the series have ("linear", "exp", "log", "poly", "power", "movingAvg"), empty for none or for series that differ, and whether the first one shows its equation and R-squared.
    QString trendline;
    bool trendlineEquation = false, trendlineRSquared = false;
};
ChartElements elementsOf(const Chart &chart);

// The first plot's axis that runs across the chart (`horizontal`) or up it, or null (a pie).
ChartAxis *primaryAxis(Chart *chart, bool horizontal);
const ChartAxis *primaryAxis(const Chart &chart, bool horizontal);

// The axis of the plots drawn against the second pair of axes that runs across the chart (`horizontal`) or up it, or null.
ChartAxis *secondaryAxis(Chart *chart, bool horizontal);
const ChartAxis *secondaryAxis(const Chart &chart, bool horizontal);

enum class TitleMode : quint8 { None, Above, Overlay };
void setChartTitle(Chart *chart, TitleMode mode);
// The words of the chart title, or the cell it reads them from (`ref` such as "Sheet1!$A$1": the words follow that cell); the title is shown.
void setChartTitleText(Chart *chart, const QString &text, const QString &ref = QString());
// The same for the title of an axis.
void setAxisTitleText(Chart *chart, bool horizontal, const QString &text, const QString &ref = QString(), bool secondary = false);
void setAxisShown(Chart *chart, bool horizontal, bool shown, bool secondary = false);
void setAxisTitle(Chart *chart, bool horizontal, bool shown, bool secondary = false);
// Gridlines across the chart (`horizontal`) or up and down it, major or minor.
void setGridlines(Chart *chart, bool horizontal, bool minor, bool shown);
// A chart as a template file (.crtx): a package that holds the chart's part. Its look is what a template is for; the data it shows goes with it, as a chart's file has it.
QByteArray chartTemplateFile(const Chart &chart);
// The chart a template file holds; false, with a reason, if it is not one.
bool readChartTemplate(const QByteArray &file, Chart *out, QString *why = nullptr);
// `target` in the look of `look`: its kinds, axes, legend, title and text settings, fills, and the look of each series (a series past those of `look` takes the look of its last); the data,
// the names, and the cells it reads stay `target`'s.
Chart chartWithLook(const Chart &target, const Chart &look);
// "" takes the legend away; else "r", "l", "t", or "b".
void setLegend(Chart *chart, const QString &position, bool overlay = false);
// A trendline of this kind on every series that can have one (none, with an empty `kind`); a series that has one of another kind is given this kind and keeps how far it runs and what it shows.
// `order` is a polynomial's, `period` a moving average's (below 0: the series' own stay, and a new trendline has 2).
void setTrendlines(Chart *chart, const QString &kind, int order = -1, int period = -1);
// The equation and R-squared of the series' trendlines shown or not.
void setTrendlineDisplay(Chart *chart, bool equation, bool rSquared);
// The parts of an axis a format pane sets: its bounds and units, its order, where the other axis crosses it, the labels' number format, turn, and place, and its tick marks, taken from `from`
// (the axis as the pane made it); the rest of the axis (its title, line, gridlines, and fonts) is left as it is. Nothing for an axis the chart does not have.
void setAxisFormat(Chart *chart, bool horizontal, bool secondary, const ChartAxis &from);
// Why the axis a format pane made can't be used, in a sentence (the lowest value is not under the highest, a unit is negative); empty when it can.
QString axisFormatProblem(const ChartAxis &axis);
// The positions data labels may be given for this chart (empty means the usual only).
QStringList labelPositionsFor(const Chart &chart);
// Labels of the values on every series (`position` empty: the usual), or none.
void setDataLabels(Chart *chart, bool shown, const QString &position = QString());

// What the data labels hold: the value, the category's name, the series' name, and the percentage, with what stands between them ("" for the usual comma and space) and a number format
// ("" for the values' own). Read from the first series that has labels; set on every series: one that has none gets labels in the usual place, and none chosen takes the labels away.
struct ChartLabelParts {
    bool value = false, category = false, series = false, percent = false, legendKey = false;
    QString separator, format;
};
ChartLabelParts labelPartsOf(const Chart &chart);
// "Value from cells": every series' labels show the words of the cells of `ref` ("Sheet1!$D$2:$D$4"), one cell for each point, along with what else the labels hold; a series that has no labels is
// given these alone. An empty `ref` takes it away (the labels left are the other parts', or none). The words come from the cells when the chart next reads them (applyChartCells).
void setDataLabelsFromCells(Chart *chart, const QString &ref);
// The cells the first series' labels read the words of; empty for none.
QString dataLabelsCellsRef(const Chart &chart);
void setDataLabelParts(Chart *chart, const ChartLabelParts &parts);

// Error bars on every series of the bar, line, and scatter plots: "fixedVal" (`value` each way), "percentage" (`value` percent), "stdDev" (`value` standard deviations), or "stdErr"; an empty
// `kind` takes them away. The bars are both ways, with end caps; a custom pair of ranges a file holds is left as it is by a change of kind only if it is `kind` "cust" (no change).
void setErrorBars(Chart *chart, const QString &kind, double value = 1);
// The kind the first series' error bars have ("" for none).
QString errorBarsKind(const Chart &chart);

// Drop lines, high-low lines, and up-down bars of the chart's line plots (drop lines also of area plots). `appliesTo` says whether the chart can have them (high-low lines and up-down bars need a
// line plot of two series or more); `on` whether every plot that can has them.
enum class PlotLines : quint8 { Drop, HighLow, UpDown };
bool plotLinesApply(const Chart &chart, PlotLines kind);
bool plotLinesOn(const Chart &chart, PlotLines kind);
void setPlotLines(Chart *chart, PlotLines kind, bool on);

// ---- colors and looks of our own
int chartColorSchemeCount();
QString chartColorSchemeName(int index);
// The colors of a scheme (as many as the chart has series or slices, the scheme's own repeated).
QColor chartSchemeColor(const Chart &chart, int scheme, int n);
void applyColorScheme(Chart *chart, int scheme);
int chartStyleCount();
QString chartStyleName(int index);
void applyChartStyle(Chart *chart, int style);
// A few layouts that set several elements at once (title, legend, labels, gridlines).
int chartLayoutCount();
QString chartLayoutName(int index);
void applyChartLayout(Chart *chart, int layout);

// ---- kinds
// What each series is drawn as, in the order of the chart's series (the plots' in turn).
QVector<SeriesKind> seriesKindsOf(const Chart &chart);
// The chart drawn as the kinds say (one for each series, the same number as the chart has), keeping what can be kept of its look; the chart as a whole of one kind is all the same.
Chart withSeriesKinds(const Chart &chart, const QVector<SeriesKind> &kinds, const jp::office::Theme &theme);
Chart withKind(const Chart &chart, ChartKind kind, const jp::office::Theme &theme);

// ---- series
// The chart's series in order (plot by plot), and the chart made again from series in another order or number, each as it was drawn.
QVector<ChartSeries> seriesOf(const Chart &chart);
int seriesCount(const Chart &chart);
// Replaces the series (the kinds as they were, the same number; or `kinds` if given) keeping the look.
Chart withSeries(const Chart &chart, const QVector<ChartSeries> &series, const QVector<SeriesKind> &kinds, const jp::office::Theme &theme);
// The chart with its rows and columns switched: where the series ran down columns they run along rows, and the other way. The cells are the same block (the names and categories the
// series had stay the names and categories). Empty (`ok` false) if the series do not read one block that way.
Chart switchedRowsColumns(const Chart &chart, const jp::office::Theme &theme, bool *ok);
// The cells all the chart's series read as one reference ("Sheet1!$A$1:$D$8"): the smallest block that holds them; empty if they read none, or more than one sheet.
QString chartDataRange(const Chart &chart);
// Whether the chart's series run down columns of one block (false: along rows, or no block).
bool seriesRunDownColumns(const Chart &chart);

} // namespace jp::office::sheet
