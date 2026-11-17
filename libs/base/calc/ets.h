#pragma once
// Exponential smoothing of a series with a trend and a season (the additive Holt-Winters method), and what the forecast functions and Forecast Sheet make of it: a timeline and values
// put on an even grid (duplicates combined, gaps filled), the smoothing constants found by search, a forecast and its confidence interval, the season found by looking at the
// series, and the measures of fit. No GUI. Bounded: at most kMaxPoints points on the grid, and the search does a bounded amount of work.

#include "calc/dates.h"

#include <memory>
#include <utility>
#include <vector>

namespace jp::calc::ets {

constexpr long long kMaxPoints = 1000000;       // the points of the grid, after gaps are filled
constexpr long long kMaxHorizon = 100000;       // how far past the end a forecast is made

enum class Aggregation { Average = 1, Count, CountA, Max, Median, Min, Sum };

enum class Problem { None, TooFew, Uneven, TooMany };

// The evenly spaced times of the series: every `step` days from `t0`, or every `step` months from the month of `t0` (for dates that fall on one day of each month, or on the last day).
struct Timeline {
    bool months = false;
    double t0 = 0, step = 1;
    int day = 1;
    bool lastDay = false;
    DateSystem system = DateSystem::Date1900;
    long long n = 0;                // the points

    double position(double t) const;            // the place on the grid of a time, as a fraction (0 is the first point)
    double timeAt(long long k) const;           // the time of grid point k (k may be past the end)
};

struct Prepared {
    Timeline timeline;
    std::vector<double> values;
    long long filled = 0;           // the points that had no value and were given one
};

// The time and value pairs (a value that is NaN is a blank) to an even grid. Duplicate times are combined with `aggregation`; a gap is filled by a straight line between its
// neighbors (`interpolate`) or with zeros. A timeline that is not evenly spaced is Uneven; fewer than 3 times with values, TooFew.
Problem prepare(std::vector<std::pair<double, double>> pairs, Aggregation aggregation, bool interpolate, DateSystem system, Prepared *out);

struct Model {
    int season = 0;                 // 0 for none, else the length of the season
    double alpha = 0, beta = 0, gamma = 0;
    double level = 0, trend = 0;    // after the last point
    std::vector<double> seasonal;   // the season's states, by position in the grid (index % season)
    std::vector<double> fitted;     // the one-step forecast of each point
    long long n = 0;
    double mase = 0, smape = 0, mae = 0, rmse = 0, sigma = 0;
    mutable std::vector<double> variance;   // for each horizon h (index h - 1), the variance of the forecast error in units of sigma squared

    // The forecast for the point h steps past the last one (h >= 1); for h <= 0, the fitted value of point n - 1 + h (false if there is none).
    bool forecast(long long h, double *out) const;
    // The half width of the interval for that forecast (for h <= 0, that of h = 1).
    bool interval(long long h, double confidence, double *halfWidth) const;
};

// The length of the season the series seems to have, 0 if none (a season is at least 2 points, and the series must hold two of them).
int detectSeason(const std::vector<double> &y);

// `season`: 0 for none, else the length (the series must hold two of them: otherwise null).
std::shared_ptr<const Model> fit(const std::vector<double> &y, int season);

// What the calls of the three heavy steps have been so far, all threads together (for a test to tell that a series was worked on once, not once for each cell that reads it).
struct Counts {
    long long prepares = 0, detects = 0, fits = 0;
};
Counts counts();

// The same series and season, fitted again, is not worked again: the last few fits are kept (this thread's).
std::shared_ptr<const Model> fitCached(const std::vector<double> &y, int season);

} // namespace jp::calc::ets
