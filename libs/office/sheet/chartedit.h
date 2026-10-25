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
};
ChartElements elementsOf(const Chart &chart);

// The first plot's axis that runs across the chart (`horizontal`) or up it, or null (a pie).
ChartAxis *primaryAxis(Chart *chart, bool horizontal);
const ChartAxis *primaryAxis(const Chart &chart, bool horizontal);

enum class TitleMode : quint8 { None, Above, Overlay };
void setChartTitle(Chart *chart, TitleMode mode);
void setAxisShown(Chart *chart, bool horizontal, bool shown);
void setAxisTitle(Chart *chart, bool horizontal, bool shown);
// Gridlines across the chart (`horizontal`) or up and down it, major or minor.
void setGridlines(Chart *chart, bool horizontal, bool minor, bool shown);
// "" takes the legend away; else "r", "l", "t", or "b".
void setLegend(Chart *chart, const QString &position);
// The positions data labels may be given for this chart (empty means the usual only).
QStringList labelPositionsFor(const Chart &chart);
// Labels of the values on every series (`position` empty: the usual), or none.
void setDataLabels(Chart *chart, bool shown, const QString &position = QString());

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
