#include "calc/ets.h"

#include "calc/special.h"

#include <QDate>

#include <algorithm>
#include <cmath>
#include <limits>

namespace jp::calc::ets {

namespace {

constexpr double kEven = 1e-6;                  // how far from a whole number of steps a time may be and still be on the grid

bool serialMonth(double t, DateSystem system, long long *index, int *day, bool *last)
{
    if (t != std::floor(t)) return false;
    int y, m, d;
    if (!serialToDate(t, system, &y, &m, &d)) return false;
    *index = (long long)y * 12 + (m - 1);
    *day = d;
    *last = d == QDate(y, m, 1).daysInMonth();
    return true;
}

double aggregate(std::vector<double> &v, Aggregation a)
{
    switch (a) {
    case Aggregation::Count:
    case Aggregation::CountA: return double(v.size());
    case Aggregation::Max: return *std::max_element(v.begin(), v.end());
    case Aggregation::Min: return *std::min_element(v.begin(), v.end());
    case Aggregation::Sum: {
        double s = 0;
        for (double x : v) s += x;
        return s;
    }
    case Aggregation::Median: {
        std::sort(v.begin(), v.end());
        const size_t h = v.size() / 2;
        return v.size() % 2 ? v[h] : (v[h - 1] + v[h]) / 2;
    }
    case Aggregation::Average:
    default: {
        double s = 0;
        for (double x : v) s += x;
        return s / double(v.size());
    }
    }
}

} // namespace

double Timeline::position(double t) const
{
    if (!months) return (t - t0) / step;
    long long a, b;
    int d;
    bool l;
    if (!serialMonth(t, system, &a, &d, &l) || !serialMonth(t0, system, &b, &d, &l)) return (t - t0) / 30.4375 / step;
    return double(a - b) / step;
}

double Timeline::timeAt(long long k) const
{
    if (!months) return t0 + double(k) * step;
    long long m0;
    int d;
    bool l;
    if (!serialMonth(t0, system, &m0, &d, &l)) return t0 + double(k) * step * 30.4375;
    const long long m = m0 + (long long)std::llround(double(k) * step);
    const int y = int(m / 12), mo = int(m % 12) + 1;
    if (m < 0 || y > 9999) return std::numeric_limits<double>::quiet_NaN();
    const int dim = QDate(std::max(y, 1), mo, 1).daysInMonth();
    double serial;
    if (!dateToSerial(y, mo, lastDay ? dim : std::min(day, dim), system, &serial)) return std::numeric_limits<double>::quiet_NaN();
    return serial;
}

Problem prepare(std::vector<std::pair<double, double>> pairs, Aggregation aggregation, bool interpolate, DateSystem system, Prepared *out)
{
    std::stable_sort(pairs.begin(), pairs.end(), [](const auto &a, const auto &b) { return a.first < b.first; });
    std::vector<double> times, vals;
    for (size_t i = 0; i < pairs.size();) {
        size_t j = i;
        std::vector<double> group;
        while (j < pairs.size() && pairs[j].first == pairs[i].first) {
            if (!std::isnan(pairs[j].second)) group.push_back(pairs[j].second);
            ++j;
        }
        if (!group.empty()) {
            times.push_back(pairs[i].first);
            vals.push_back(aggregate(group, aggregation));
        }
        i = j;
    }
    if (times.size() < 3) return Problem::TooFew;
    Timeline tl;
    tl.system = system;
    tl.t0 = times.front();
    double step = std::numeric_limits<double>::infinity();
    for (size_t i = 1; i < times.size(); ++i) step = std::min(step, times[i] - times[i - 1]);
    bool even = step > 0;
    for (size_t i = 0; i < times.size() && even; ++i) {
        const double r = (times[i] - tl.t0) / step;
        if (std::fabs(r - std::round(r)) > kEven) even = false;
    }
    std::vector<double> index(times.size());
    if (even) {
        tl.step = step;
        for (size_t i = 0; i < times.size(); ++i) index[i] = std::round((times[i] - tl.t0) / step);
    } else {
        // whole months apart, on one day of the month (or each on the last)
        std::vector<long long> months(times.size());
        std::vector<int> days(times.size());
        std::vector<bool> lasts(times.size());
        bool ok = true;
        for (size_t i = 0; i < times.size() && ok; ++i) {
            bool l;
            ok = serialMonth(times[i], system, &months[i], &days[i], &l);
            lasts[i] = l;
        }
        if (!ok) return Problem::Uneven;
        bool sameDay = true, allLast = true;
        for (size_t i = 0; i < times.size(); ++i) {
            if (days[i] != days[0]) sameDay = false;
            if (!lasts[i]) allLast = false;
        }
        if (!sameDay && !allLast) return Problem::Uneven;
        long long ms = std::numeric_limits<long long>::max();
        for (size_t i = 1; i < months.size(); ++i) ms = std::min(ms, months[i] - months[i - 1]);
        if (ms < 1) return Problem::Uneven;
        for (size_t i = 0; i < months.size(); ++i)
            if ((months[i] - months[0]) % ms != 0) return Problem::Uneven;
        tl.months = true;
        tl.step = double(ms);
        tl.day = days[0];
        tl.lastDay = allLast && !sameDay;
        for (size_t i = 0; i < times.size(); ++i) index[i] = double((months[i] - months[0]) / ms);
    }
    const double span = index.back() + 1;
    if (span > double(kMaxPoints)) return Problem::TooMany;
    tl.n = (long long)span;
    out->timeline = tl;
    out->filled = 0;
    out->values.assign(size_t(tl.n), std::numeric_limits<double>::quiet_NaN());
    for (size_t i = 0; i < times.size(); ++i) out->values[size_t(index[i])] = vals[i];
    for (size_t i = 0; i < out->values.size(); ++i) {
        if (!std::isnan(out->values[i])) continue;
        size_t j = i;
        while (std::isnan(out->values[j])) ++j;   // (the last point has a value)
        const double lo = out->values[i - 1], hi = out->values[j];
        for (size_t k = i; k < j; ++k) {
            out->values[k] = interpolate ? lo + (hi - lo) * double(k - (i - 1)) / double(j - (i - 1)) : 0;
            ++out->filled;
        }
        i = j;
    }
    return Problem::None;
}

// ---- the model

namespace {

struct Run {
    double sse = 0;
    double level = 0, trend = 0;
    std::vector<double> season;
    long long first = 0;    // the first point whose forecast is counted
};

// The series run through the model with these constants. `fitted`, if given, gets the one-step forecasts.
Run run(const std::vector<double> &y, int m, double a, double b, double g, std::vector<double> *fitted)
{
    Run r;
    const size_t n = y.size();
    size_t t = 0;
    double l, tr;
    if (m < 2) {
        l = y[0];
        tr = n > 1 ? y[1] - y[0] : 0;
        r.season.clear();
        t = 1;
        if (fitted) {
            fitted->assign(n, 0);
            (*fitted)[0] = y[0];
        }
    } else {
        double m1 = 0, m2 = 0;
        for (int i = 0; i < m; ++i) {
            m1 += y[size_t(i)];
            m2 += y[size_t(m + i)];
        }
        m1 /= m;
        m2 /= m;
        tr = (m2 - m1) / m;
        l = m1 + tr * (m - 1) / 2.0;
        r.season.resize(size_t(m));
        for (int i = 0; i < m; ++i) r.season[size_t(i)] = y[size_t(i)] - (m1 + tr * (i - (m - 1) / 2.0));
        t = size_t(m);
        if (fitted) {
            fitted->assign(n, 0);
            for (int i = 0; i < m; ++i) (*fitted)[size_t(i)] = y[size_t(i)];
        }
    }
    r.first = (long long)t;
    size_t si = m < 2 ? 0 : t % size_t(m);
    for (; t < n; ++t) {
        double s = 0;
        if (m >= 2) s = r.season[si];
        const double pred = l + tr + s;
        const double e = y[t] - pred;
        if (fitted) (*fitted)[t] = pred;
        r.sse += e * e;
        const double l2 = a * (y[t] - s) + (1 - a) * (l + tr);
        tr = b * (l2 - l) + (1 - b) * tr;
        if (m >= 2) {
            r.season[si] = g * (y[t] - l2) + (1 - g) * s;
            if (++si == size_t(m)) si = 0;
        }
        l = l2;
    }
    r.level = l;
    r.trend = tr;
    return r;
}

// Nelder-Mead on the constants (each kept in 0 to 1), from one start, using at most `budget` runs.
struct Search {
    const std::vector<double> &y;
    int m;
    int dims;
    int evals = 0;

    double cost(const std::vector<double> &p)
    {
        ++evals;
        double pen = 0;
        double q[3] = {0, 0, 0};
        for (int i = 0; i < dims; ++i) {
            q[i] = std::min(1.0, std::max(0.0, p[size_t(i)]));
            pen += std::fabs(p[size_t(i)] - q[i]);
        }
        const double sse = run(y, m, q[0], q[1], q[2], nullptr).sse;
        return sse * (1 + pen);
    }

    std::vector<double> go(const std::vector<double> &start, int budget, double *best)
    {
        const size_t d = size_t(dims);
        std::vector<std::vector<double>> s(d + 1, start);
        for (size_t i = 0; i < d; ++i) s[i + 1][i] += start[i] > 0.5 ? -0.2 : 0.2;
        std::vector<double> f(d + 1);
        for (size_t i = 0; i <= d; ++i) f[i] = cost(s[i]);
        const int limit = evals + budget;
        while (evals < limit) {
            std::vector<size_t> o(d + 1);
            for (size_t i = 0; i <= d; ++i) o[i] = i;
            std::sort(o.begin(), o.end(), [&](size_t a, size_t b) { return f[a] < f[b]; });
            const size_t lo = o[0], hi = o[d], nx = o[d - 1];
            if (f[hi] - f[lo] <= 1e-12 * std::max(f[lo], 1e-300)) break;
            std::vector<double> c(d, 0.0);
            for (size_t i = 0; i <= d; ++i)
                if (i != hi)
                    for (size_t k = 0; k < d; ++k) c[k] += s[i][k] / double(d);
            auto at = [&](double t) {
                std::vector<double> p(d);
                for (size_t k = 0; k < d; ++k) p[k] = c[k] + t * (s[hi][k] - c[k]);
                return p;
            };
            std::vector<double> xr = at(-1);
            const double fr = cost(xr);
            if (fr < f[lo]) {
                std::vector<double> xe = at(-2);
                const double fe = cost(xe);
                if (fe < fr) {
                    s[hi] = xe;
                    f[hi] = fe;
                } else {
                    s[hi] = xr;
                    f[hi] = fr;
                }
            } else if (fr < f[nx]) {
                s[hi] = xr;
                f[hi] = fr;
            } else {
                std::vector<double> xc = at(fr < f[hi] ? -0.5 : 0.5);
                const double fc = cost(xc);
                if (fc < std::min(fr, f[hi])) {
                    s[hi] = xc;
                    f[hi] = fc;
                } else {
                    for (size_t i = 0; i <= d; ++i) {
                        if (i == lo) continue;
                        for (size_t k = 0; k < d; ++k) s[i][k] = s[lo][k] + 0.5 * (s[i][k] - s[lo][k]);
                        f[i] = cost(s[i]);
                    }
                }
            }
        }
        size_t b = 0;
        for (size_t i = 1; i <= d; ++i)
            if (f[i] < f[b]) b = i;
        *best = f[b];
        return s[b];
    }
};

} // namespace

bool Model::forecast(long long h, double *out) const
{
    if (h <= 0) {
        const long long i = n - 1 + h;
        if (i < 0 || i >= n) return false;
        *out = fitted[size_t(i)];
        return true;
    }
    if (h > kMaxHorizon) return false;
    double s = 0;
    if (season >= 2) s = seasonal[size_t((n - 1 + h) % season)];
    *out = level + double(h) * trend + s;
    return std::isfinite(*out);
}

bool Model::interval(long long h, double confidence, double *halfWidth) const
{
    if (!(confidence > 0 && confidence < 1)) return false;
    if (h < 1) h = 1;
    if (h > kMaxHorizon) return false;
    while ((long long)variance.size() < h) {
        const long long j = (long long)variance.size();   // variance of horizon j + 1: 1 plus the squares of the weights of the earlier errors
        double v = 1;
        if (j > 0) {
            double c = alpha * (1 + double(j) * beta * 1.0);
            if (season >= 2 && j % season == 0) c += gamma * (1 - alpha);
            v = variance[size_t(j - 1)] + c * c;
        }
        variance.push_back(v);
    }
    double z;
    if (!inverseStandardNormal(1 - (1 - confidence) / 2, &z)) return false;
    *halfWidth = z * sigma * std::sqrt(variance[size_t(h - 1)]);
    return std::isfinite(*halfWidth);
}

int detectSeason(const std::vector<double> &all)
{
    // a long series is looked at by its last part
    const size_t cap = 20000;
    const std::vector<double> y(all.size() > cap ? all.end() - long(cap) : all.begin(), all.end());
    const size_t n = y.size();
    if (n < 8) return 0;
    double sx = 0, sy = 0, sxx = 0, sxy = 0;
    for (size_t i = 0; i < n; ++i) {
        sx += double(i);
        sy += y[i];
        sxx += double(i) * double(i);
        sxy += double(i) * y[i];
    }
    const double dn = double(n);
    const double den = dn * sxx - sx * sx;
    const double slope = den == 0 ? 0 : (dn * sxy - sx * sy) / den;
    const double icpt = (sy - slope * sx) / dn;
    std::vector<double> r(n);
    double var = 0;
    for (size_t i = 0; i < n; ++i) {
        r[i] = y[i] - (icpt + slope * double(i));
        var += r[i] * r[i];
    }
    if (var <= 0) return 0;
    const size_t maxLag = std::min<size_t>(n / 2, 1000);
    std::vector<double> acf(maxLag + 2, 0.0);
    for (size_t k = 1; k <= maxLag; ++k) {
        double s = 0;
        for (size_t i = 0; i + k < n; ++i) s += r[i] * r[i + k];
        acf[k] = s / var;
    }
    double best = 0;
    std::vector<size_t> peaks;
    for (size_t k = 2; k <= maxLag; ++k)
        if (acf[k] > acf[k - 1] && acf[k] >= acf[k + 1]) {
            peaks.push_back(k);
            best = std::max(best, acf[k]);
        }
    if (best < 0.3) return 0;
    for (size_t k : peaks)
        if (acf[k] >= 0.85 * best) return int(k);
    return 0;
}

std::shared_ptr<const Model> fit(const std::vector<double> &y, int season)
{
    const size_t n = y.size();
    if (n < 2) return nullptr;
    if (season < 2) season = 0;
    if (season && n < size_t(2 * season)) return nullptr;
    auto model = std::make_shared<Model>();
    model->season = season;
    model->n = (long long)n;
    Search search{y, season, season ? 3 : 2};
    const int budget = int(std::min<double>(900, std::max<double>(90, 2e7 / double(n))));
    std::vector<std::vector<double>> starts;
    if (season) starts = {{0.4, 0.05, 0.3}, {0.8, 0.01, 0.1}, {0.15, 0.1, 0.6}};
    else starts = {{0.5, 0.1}, {0.9, 0.3}, {0.2, 0.02}};
    double bestCost = std::numeric_limits<double>::infinity();
    std::vector<double> bestParams = starts[0];
    for (const auto &st : starts) {
        double c;
        const std::vector<double> p = search.go(st, budget / int(starts.size()), &c);
        if (c < bestCost) {
            bestCost = c;
            bestParams = p;
        }
    }
    double q[3] = {0, 0, 0};
    for (size_t i = 0; i < bestParams.size(); ++i) q[i] = std::min(1.0, std::max(0.0, bestParams[i]));
    model->alpha = q[0];
    model->beta = q[1];
    model->gamma = season ? q[2] : 0;
    const Run r = run(y, season, model->alpha, model->beta, model->gamma, &model->fitted);
    model->level = r.level;
    model->trend = r.trend;
    // the states are kept by the place in the grid: position t % m holds the state last updated at that place
    model->seasonal = r.season;
    const double count = double(std::max<long long>(1, (long long)n - r.first));
    double ae = 0, se = 0, sm = 0;
    long long k = 0;
    for (size_t t = size_t(r.first); t < n; ++t) {
        const double e = y[t] - model->fitted[t];
        ae += std::fabs(e);
        se += e * e;
        const double d = std::fabs(y[t]) + std::fabs(model->fitted[t]);
        if (d > 0) sm += 2 * std::fabs(e) / d;
        ++k;
    }
    model->mae = ae / count;
    model->rmse = std::sqrt(se / count);
    model->smape = sm / count;
    const size_t lag = season ? size_t(season) : 1;
    double naive = 0;
    for (size_t t = lag; t < n; ++t) naive += std::fabs(y[t] - y[t - lag]);
    naive /= double(std::max<size_t>(1, n - lag));
    model->mase = naive > 0 ? model->mae / naive : 0;
    const double params = 2 + (season ? 1 + season : 0);
    model->sigma = std::sqrt(se / std::max(1.0, double(k) - params));
    return model;
}

std::shared_ptr<const Model> fitCached(const std::vector<double> &y, int season)
{
    struct Entry {
        std::vector<double> y;
        int season;
        std::shared_ptr<const Model> model;
    };
    thread_local std::vector<Entry> cache;
    for (const Entry &e : cache)
        if (e.season == season && e.y == y) return e.model;
    std::shared_ptr<const Model> m = fit(y, season);
    if (m) {
        if (cache.size() >= 4) cache.erase(cache.begin());
        cache.push_back({y, season, m});
    }
    return m;
}

} // namespace jp::calc::ets
