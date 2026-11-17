// FORECAST.ETS and its three companions: a forecast by exponential smoothing with a trend and a season (calc/ets.h), the half width of its confidence interval, the length of the
// season the series has, and the measures of fit.
#include "calc/ets.h"
#include "calc/machine.h"

#include <cmath>

namespace jp::calc {

namespace {

struct Options {
    int season = 1;                 // 1: found from the series, 0: none, else the length
    bool interpolate = true;
    ets::Aggregation aggregation = ets::Aggregation::Average;
};

bool omitted(Call &c, int i)
{
    return i >= c.argc() || c.scalar(i).isEmpty();
}

bool wholeOption(Call &c, int i, int low, int high, int fallback, int *out)
{
    *out = fallback;
    if (omitted(c, i)) return true;
    double d;
    if (!c.number(i, &d)) return false;
    if (d < low || d > high) {
        c.error(ErrorKind::Num);
        return false;
    }
    *out = int(d);
    return true;
}

bool readOptions(Call &c, int seasonArg, int completionArg, int aggregationArg, Options *o)
{
    int v;
    if (seasonArg >= 0) {
        if (!wholeOption(c, seasonArg, 0, 100000, 1, &o->season)) return false;
    }
    if (completionArg >= 0) {
        if (!wholeOption(c, completionArg, 0, 1, 1, &v)) return false;
        o->interpolate = v == 1;
    }
    if (aggregationArg >= 0) {
        if (!wholeOption(c, aggregationArg, 1, 7, 1, &v)) return false;
        o->aggregation = ets::Aggregation(v);
    }
    return true;
}

// The prepared series of the values (argument `values`) and the timeline.
bool series(Call &c, int values, int timeline, const Options &o, ets::Prepared *p, Value *error)
{
    Value vv, tv;
    if (!c.array(values, &vv) || !c.array(timeline, &tv)) {
        *error = c.failure();
        return false;
    }
    if (vv.rows() * vv.cols() != tv.rows() * tv.cols() || (vv.rows() != tv.rows() && vv.cols() != tv.cols() && vv.rows() * vv.cols() > 1)) {
        *error = Value::error(ErrorKind::NA);
        return false;
    }
    const int n = vv.rows() * vv.cols();
    if (n > ets::kMaxPoints) {
        *error = Value::error(ErrorKind::Num);
        return false;
    }
    std::vector<std::pair<double, double>> pairs;
    pairs.reserve(size_t(n));
    for (int k = 0; k < n; ++k) {
        const Value &v = vv.at(k / vv.cols(), k % vv.cols()), &t = tv.at(k / tv.cols(), k % tv.cols());
        if (t.isError()) {
            *error = t;
            return false;
        }
        if (v.isError()) {
            *error = v;
            return false;
        }
        if (!t.isNumber() || !std::isfinite(t.number()) || (!v.isNumber() && !v.isEmpty())) {
            *error = Value::error(ErrorKind::Value);
            return false;
        }
        pairs.push_back({t.number(), v.isEmpty() ? std::numeric_limits<double>::quiet_NaN() : v.number()});
    }
    if (ets::prepare(std::move(pairs), o.aggregation, o.interpolate, c.context().dateSystem, p) != ets::Problem::None) {
        *error = Value::error(ErrorKind::Num);
        return false;
    }
    return true;
}

int seasonOf(const Options &o, const ets::Prepared &p)
{
    return o.season == 1 ? ets::detectSeason(p.values) : o.season;
}

std::shared_ptr<const ets::Model> modelOf(const Options &o, const ets::Prepared &p)
{
    return ets::fitCached(p.values, seasonOf(o, p));
}

bool horizonOf(const ets::Prepared &p, double target, long long *h)
{
    const double pos = p.timeline.position(target);
    if (!std::isfinite(pos) || std::fabs(pos) > 4e6) return false;
    *h = (long long)std::llround(pos) - (p.timeline.n - 1);
    return true;
}

Value fnForecastEts(Call &c)
{
    double target;
    Options o;
    if (!c.number(0, &target) || !readOptions(c, 3, 4, 5, &o)) return c.failure();
    ets::Prepared p;
    Value err;
    if (!series(c, 1, 2, o, &p, &err)) return err;
    const auto m = modelOf(o, p);
    long long h;
    double out;
    if (!m || !horizonOf(p, target, &h) || !m->forecast(h, &out)) return c.error(ErrorKind::Num);
    return numberResult(out);
}

Value fnForecastEtsConfint(Call &c)
{
    double target, confidence = 0.95;
    Options o;
    if (!c.number(0, &target)) return c.failure();
    if (!omitted(c, 3) && !c.number(3, &confidence)) return c.failure();
    if (!readOptions(c, 4, 5, 6, &o)) return c.failure();
    ets::Prepared p;
    Value err;
    if (!series(c, 1, 2, o, &p, &err)) return err;
    const auto m = modelOf(o, p);
    long long h;
    double out;
    if (!m || !horizonOf(p, target, &h) || !m->interval(h, confidence, &out)) return c.error(ErrorKind::Num);
    return numberResult(out);
}

Value fnForecastEtsSeasonality(Call &c)
{
    Options o;
    if (!readOptions(c, -1, 2, 3, &o)) return c.failure();
    ets::Prepared p;
    Value err;
    if (!series(c, 0, 1, o, &p, &err)) return err;
    const int s = ets::detectSeason(p.values);
    return Value::number(s < 2 ? 1 : s);
}

Value fnForecastEtsStat(Call &c)
{
    double type;
    Options o;
    if (!c.number(2, &type) || !readOptions(c, 3, 4, 5, &o)) return c.failure();
    ets::Prepared p;
    Value err;
    if (!series(c, 0, 1, o, &p, &err)) return err;
    const int k = int(type);
    if (k < 1 || k > 8) return c.error(ErrorKind::Num);
    if (k == 8) return numberResult(p.timeline.step);
    const auto m = modelOf(o, p);
    if (!m) return c.error(ErrorKind::Num);
    const double v[] = {m->alpha, m->beta, m->gamma, m->mase, m->smape, m->mae, m->rmse};
    return numberResult(v[k - 1]);
}

} // namespace

void registerEtsFunctions(std::vector<FunctionInfo> &r)
{
    r.push_back({"FORECAST.ETS", 3, 6, "vrrvvv", kFnFuture, fnForecastEts});
    r.push_back({"FORECAST.ETS.CONFINT", 3, 7, "vrrvvvv", kFnFuture, fnForecastEtsConfint});
    r.push_back({"FORECAST.ETS.SEASONALITY", 2, 4, "rrvv", kFnFuture, fnForecastEtsSeasonality});
    r.push_back({"FORECAST.ETS.STAT", 3, 6, "rrvvvv", kFnFuture, fnForecastEtsStat});
}

} // namespace jp::calc
