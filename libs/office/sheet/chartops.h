#pragma once
// Making and changing the charts a person puts in a document by hand (Insert > Chart): the kinds Insert offers, the sample numbers a new chart begins with, the words and numbers a chart
// plots (what the data editor shows), and one change to the model of chart.h for each thing the Chart tab sets (its kind, title, legend, labels, axes, gridlines, and style). The word
// processor, the presentation program, and the publishing program all make and change their charts with these, so a chart means the same wherever it is; each keeps the chart part
// (chart_write.h) its own way and draws it with chartSvg.

#include "core/items.h"
#include "office/chart.h"
#include "office/theme.h"
#include "sheet/chartmake.h"

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QVector>

namespace jp::office::sheet {

// The kinds of chart Insert offers (and a chart's kind, as nearly as these say it).
enum class ChartForm { Column, ColumnStacked, ColumnPercent, Bar, BarStacked, Line, LineMarkers, Pie, Doughnut, Area, AreaStacked, Scatter };
const QVector<ChartForm> &chartForms();
// What a chart plots: the words under the points (or a scatter's x numbers), the name of each series, and the numbers (a list for each series, a number for each category).
struct ChartData {
    QStringList categories, seriesNames;
    QVector<QVector<double>> values;
};
// The most categories and series a chart made here has.
constexpr int kMostChartCategories = 200, kMostChartSeries = 24;
// The most charts one file holds as charts (a chart past it is kept as the picture it is drawn as: the same as the most charts a presentation is read with), the most bytes of a
// chart part kept for one, and the most the parts of one file come to.
constexpr int kMostChartsInFile = 300;
constexpr qint64 kMostChartPartBytes = jp::kMaxChartPartBytes, kMostChartBytesInFile = 16 << 20;
// Three categories and two series, to begin with.
ChartData sampleChartData();
// Whether the data can be a chart: some of each, no more than the most, a number for each category of each series, and numbers that are numbers.
bool chartDataIsUsable(const ChartData &data);
enum class ChartAxisKind { Category, Value };
// What a chart is: its kind, its title (its words; shown or not), its legend (shown, and where: "r", "l", "t", "b"), whether its points are labeled, and its data when the chart holds the
// numbers itself (a chart that reads cells of a workbook it is kept with can be changed in all but its data).
struct ChartInfo {
    bool valid = false;
    ChartForm form = ChartForm::Column;
    bool titleShown = false;
    QString title;
    bool legendShown = false;
    QString legendPosition;
    bool dataLabels = false;
    bool dataEditable = false;
    ChartData data;
    // The axes: whether each is shown (a pie has none), whether the value axis has major and minor gridlines and the category axis major ones, the words of each axis's title (empty for
    // none), and the style number (1 to 48).
    bool hasAxes = false, categoryAxisShown = false, valueAxisShown = false;
    bool valueGridlines = false, valueMinorGridlines = false, categoryGridlines = false;
    QString categoryAxisTitle, valueAxisTitle;
    int style = 2;
};

// A chart of the kind plotting the data, in the theme's colors; the usual look of a new chart (a legend, a title only when asked). The data must be usable.
Chart chartOfData(ChartForm form, const ChartData &data, const Theme &theme);
// The chart's kind, looks, and data as ChartInfo says them; `part` is the chart part it was read from (for whether it is only what this writes). Not valid for a chart with no plot.
ChartInfo chartInfoOf(const Chart &chart, const QByteArray &part);
// Text as a chart keeps it: a character XML refuses is a space, a pair of surrogates whole and a lone one dropped, at most `limit` UTF-16 units.
QString cleanChartText(const QString &text, int limit = 500);
// The axis of the chart that is its category (or x) axis or its value axis, or null (a chart with no axes, a pie).
ChartAxis *chartAxisOf(Chart &chart, ChartAxisKind which);
// Every series of the chart, plot by plot.
QVector<ChartSeries> chartAllSeries(const Chart &chart);

// One change each; true when the chart changed (false for no difference, or a chart the change does not suit: a pie has no axes). The look the chart had is kept.
bool chartSetForm(Chart &chart, ChartForm form, const Theme &theme);
bool chartSetTitle(Chart &chart, bool shown, const QString &text);
// `legendPosition` is "r", "l", "t", or "b"; empty takes the legend off.
bool chartSetLegend(Chart &chart, const QString &legendPosition);
bool chartSetDataLabels(Chart &chart, bool shown);
bool chartSetAxis(Chart &chart, ChartAxisKind axis, bool shown);
bool chartSetGridlines(Chart &chart, ChartAxisKind axis, bool major, bool minor);
bool chartSetAxisTitle(Chart &chart, ChartAxisKind axis, const QString &text);
bool chartSetStyle(Chart &chart, int style);
// New numbers and words; false for a chart that reads cells (its data is theirs) or data that is not usable. The look of the series that were there stays with them.
bool chartSetData(Chart &chart, const ChartData &data, const Theme &theme);

} // namespace jp::office::sheet
