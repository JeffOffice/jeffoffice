#pragma once
// Forecast Sheet: from a timeline and a series of values, the forecast with its confidence band (the smoothing of calc/ets.h, the same as FORECAST.ETS), and a new sheet that holds
// it: a table of the timeline, the values, the forecast, and the lower and upper bounds, where the forecast and the bounds are formulas of FORECAST.ETS and FORECAST.ETS.CONFINT that read
// the table's own columns (so the sheet works itself out again when a value is changed), the statistics as FORECAST.ETS.STAT formulas, and the plan of a chart of it. No GUI; the window
// puts the plan on a sheet in one step of Undo.

#include "calc/ets.h"
#include "calc/value.h"
#include "sheet/cellref.h"

#include <QString>
#include <QStringList>
#include <QVector>

#include <functional>
#include <limits>
#include <memory>
#include <vector>

namespace jp::crunch {

constexpr long long kMaxForecastInput = 1000000;    // points of the data: past it the data is refused
constexpr long long kMaxForecastPoints = 10000;     // rows of forecast

struct ForecastSettings {
    double end = 0;                         // the time the forecast ends at
    double start = std::numeric_limits<double>::quiet_NaN();    // where it starts (not a number: after the last point); earlier than the last point, the forecast is made from the points before it
    double confidence = 0.95;
    int seasonality = 1;                    // 1: found from the data, 0: none, else the length of the season
    bool statistics = false;
    bool interpolate = true;                // a missing point is a line between its neighbors (else zero)
    jp::calc::ets::Aggregation aggregation = jp::calc::ets::Aggregation::Average;
    bool columns = false;                   // the chart shows the values and the forecast as columns (else lines)
};

// The pairs of time and value (a blank value is NaN), in the order of the cells.
struct ForecastInput {
    std::vector<std::pair<double, double>> pairs;
};

using ForecastCellReader = std::function<jp::calc::Value(quint32 row, quint32 col)>;

// The data of two ranges that are each one column or one row of the same number of cells. False with the problem in plain words: a range that is not a line, ranges of different lengths,
// text or an error in them, more than kMaxForecastInput points.
bool readForecastInput(const jp::office::sheet::CellRange &timeline, const ForecastCellReader &readTimeline, const jp::office::sheet::CellRange &values, const ForecastCellReader &readValues, ForecastInput *out, QString *problem);

struct ForecastPoint {
    double time = 0;
    double value = std::numeric_limits<double>::quiet_NaN();
    double forecast = std::numeric_limits<double>::quiet_NaN();
    double lower = std::numeric_limits<double>::quiet_NaN();
    double upper = std::numeric_limits<double>::quiet_NaN();
};

struct ForecastResult {
    QString problem;                        // not empty: nothing else is set
    jp::calc::ets::Prepared prepared;       // the points the forecast is made from, on their grid
    std::shared_ptr<const jp::calc::ets::Model> model;
    int season = 0;
    std::vector<std::pair<double, double>> used;     // the pairs the forecast is made from, by time
    std::vector<std::pair<double, double>> later;    // the pairs from the forecast's start on (when it starts before the last point)
    std::vector<ForecastPoint> points;      // the forecast, one for each step of the grid from its start to its end
};

// The default end of the forecast: half as many steps again as the data has (at least one).
double defaultForecastEnd(const ForecastInput &input, jp::calc::DateSystem system);

ForecastResult computeForecast(const ForecastInput &input, const ForecastSettings &settings, jp::calc::DateSystem system);

// Everything a sheet is made of.
struct ForecastSheetPlan {
    QVector<QStringList> table;             // as typed, from A1
    int dataRows = 0;                       // the rows of the table after its header
    int usedRows = 0;                       // the first rows: the data the forecast is made from
    int forecastFirstRow = 0, forecastLastRow = 0;      // sheet rows from 0 (the first and last that have a forecast formula)
    QVector<QStringList> statistics;        // from G1 when there are some (label, formula)
};

// The table, with the forecast and bounds as formulas.
ForecastSheetPlan planForecastSheet(const ForecastResult &result, const ForecastSettings &settings);

} // namespace jp::crunch
