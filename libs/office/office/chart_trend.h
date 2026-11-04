#pragma once
// The line a trendline draws, worked out from a series' points: the kinds a chart offers (linear, exponential, logarithmic, polynomial of order 2 to 6, power, and moving average), how far it runs
// past the data, a line through a chosen point on the vertical axis, and the words of its equation and its R-squared. No drawing is done here (chart_svg.cpp draws what this makes).

#include "office/chart.h"

#include <QPointF>
#include <QString>
#include <QVector>

namespace jp::office {

struct TrendFit {
    bool ok = false;                // the points can be fitted to the kind of line (a logarithmic one needs x above 0, an exponential one y above 0, and enough points)
    QVector<QPointF> curve;         // along the line in the data's own units, left to right
    QString equation;               // "y = 2x + 1", empty for a moving average
    double rSquared = 0;
    bool hasRSquared = false;       // not for a moving average
    QVector<double> coefficients;   // linear: a, b (y = a + bx); exp: a, b (y = a e^bx); log: a, b (y = a + b ln x); power: a, b (y = a x^b); poly: c0, c1, ... ck
};

// The line for `trend` through the points (x[i], y[i]); a point with a value that is not a number is left out. `samples` is how many points a curved line is drawn with.
TrendFit fitTrend(const ChartTrendline &trend, const QVector<double> &x, const QVector<double> &y, int samples = 64);
// The name the legend gives the line: its own, else "Linear (Series1)", "Exponential (...)", "Logarithmic (...)", "Poly. (...)", "Power (...)", "2 per. Mov. Avg. (...)".
QString trendlineName(const ChartTrendline &trend, const QString &seriesName);

} // namespace jp::office
