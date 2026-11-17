// FORECAST.ETS and its three companions: a forecast by exponential smoothing with a trend and a season (calc/ets.h), the half width of its confidence interval, the length of the
// season the series has, and the measures of fit.
#include "calc/ets.h"
#include "calc/machine.h"

#include <algorithm>
#include <cmath>
#include <cstring>

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

// A series worked on once, and what is made from it as it is asked for (the season looked for, the fit): shared by every cell that reads the same cells in the same way, so that the cells of a
// Forecast Sheet, which read one series and differ in the time they ask about, do the work of it once.
struct Built {
    Value error;                    // not an error: the series is ready
    ets::Prepared prepared;
    int detected = -1;              // the season looked for (-1: not yet)
    int modelSeason = -2;           // the season the model was fitted for (-2: not yet)
    std::shared_ptr<const ets::Model> model;
    long long points() const { return prepared.timeline.n; }
};
using BuiltPtr = std::shared_ptr<Built>;

struct Entry {
    const CellSource *source = nullptr;
    bool byReference = false;       // found by the cells it was read from (when they are constants and nothing wrote them since); else by what the cells held
    CellArea values, times;
    LineToken valuesToken, timesToken;
    quint64 hash1 = 0, hash2 = 0;
    long long count = 0;
    int aggregation = 0;
    bool interpolate = true;
    DateSystem system = DateSystem::Date1900;
    BuiltPtr built;
};

constexpr size_t kMaxEntries = 4;
constexpr long long kMaxEntryPoints = 2000000;   // the points the entries hold together (a series of a million is a fit and a copy of it)

std::vector<Entry> &entries()
{
    thread_local std::vector<Entry> list;
    return list;
}

void remember(Entry &&e)
{
    std::vector<Entry> &list = entries();
    list.insert(list.begin(), std::move(e));
    long long total = 0;
    size_t keep = 0;
    for (; keep < list.size() && keep < kMaxEntries; ++keep) {
        total += list[keep].built->points();
        if (total > kMaxEntryPoints && keep > 0) break;
    }
    list.resize(keep);
}

bool fresh(const CellSource &source, const CellArea &a, LineToken *token)
{
    LineToken now;
    if (!source.constantsToken(a.sheet1, &now)) return false;
    if (now == *token) return true;
    if (!now.sameStructure(*token) || source.wroteSince(a.sheet1, token->e, a.row1, a.col1, a.row2, a.col2)) return false;
    *token = now;
    return true;
}

bool onlyConstants(const CellSource &source, const CellArea &a)
{
    return source.visitConstants(a.sheet1, a.row1, a.col1, a.row2, a.col2, [](quint32, quint32, const Value &) { return true; }) >= 0;
}

bool oneBlock(const Item &it)
{
    return it.isRef() && !it.more && it.area.sheet1 == it.area.sheet2;
}

void hashPairs(const std::vector<std::pair<double, double>> &pairs, quint64 *h1, quint64 *h2)
{
    quint64 a = 1469598103934665603ull, b = 0x9E3779B97F4A7C15ull;
    auto mix = [&](double d) {
        quint64 bits;
        std::memcpy(&bits, &d, sizeof bits);
        a = (a ^ bits) * 1099511628211ull;
        b = (b + bits + 0x7F4A7C15ull) * 0xBF58476D1CE4E5B9ull;
        b ^= b >> 29;
    };
    for (const auto &p : pairs) {
        mix(p.first);
        mix(p.second);
    }
    *h1 = a ^ pairs.size();
    *h2 = b;
}

// The steps the work on a series of n points is counted as (calc/machine.h charge): the grid, the search for a season, the search for the constants.
qint64 buildWork(long long n)
{
    return qint64(n) * 4 + 16;
}

qint64 detectWork(long long n)
{
    const long long cap = std::min<long long>(n, 20000);
    return qint64(cap) * std::min<long long>(n / 2, 1000) / 4 + qint64(n);
}

qint64 fitWork(long long n)
{
    const double budget = std::min<double>(900, std::max<double>(90, 2e7 / double(std::max<long long>(1, n))));
    return qint64(budget * double(n) / 4) + qint64(n);
}

// The two things of the arguments as a line each of numbers, and the pairs they make.
bool readPairs(Call &c, int values, int timeline, std::vector<std::pair<double, double>> *pairs, Value *error)
{
    Value vv, tv;
    if (!c.array(values, &vv) || !c.array(timeline, &tv)) {
        *error = c.failure();
        return false;
    }
    // (a range of rows and columns is no line: it is refused, as it is where the series is taken from)
    if ((vv.rows() > 1 && vv.cols() > 1) || (tv.rows() > 1 && tv.cols() > 1)) {
        *error = Value::error(ErrorKind::Value);
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
    pairs->reserve(size_t(n));
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
        pairs->push_back({t.number(), v.isEmpty() ? std::numeric_limits<double>::quiet_NaN() : v.number()});
    }
    return true;
}

// The prepared series of the values (argument `values`) and the timeline.
bool series(Call &c, int values, int timeline, const Options &o, BuiltPtr *out, Value *error)
{
    Machine &m = c.machine();
    const CellSource &source = m.source();
    const DateSystem system = c.context().dateSystem;
    auto matches = [&](const Entry &e) { return e.source == &source && e.aggregation == int(o.aggregation) && e.interpolate == o.interpolate && e.system == system; };
    // the cells it was read from, if they are constants and none of them was written since
    const bool byReference = values < c.argc() && timeline < c.argc() && oneBlock(c.item(values)) && oneBlock(c.item(timeline));
    CellArea va, ta;
    LineToken vt, tt;
    bool tokens = false;
    if (byReference) {
        va = c.item(values).area;
        ta = c.item(timeline).area;
        tokens = source.constantsToken(va.sheet1, &vt) && source.constantsToken(ta.sheet1, &tt);
        if (tokens) {
            std::vector<Entry> &list = entries();
            for (size_t i = 0; i < list.size(); ++i) {
                Entry &e = list[i];
                if (!e.byReference || !matches(e) || !(e.values == va) || !(e.times == ta)) continue;
                if (fresh(source, e.values, &e.valuesToken) && fresh(source, e.times, &e.timesToken)) {
                    if (!m.charge(8)) {
                        *error = Value::error(ErrorKind::Value);
                        return false;
                    }
                    std::rotate(list.begin(), list.begin() + long(i), list.begin() + long(i) + 1);
                    *out = list.front().built;
                    *error = (*out)->error;
                    return (*out)->error.isEmpty();
                }
                list.erase(list.begin() + long(i));
                break;
            }
        }
    }
    std::vector<std::pair<double, double>> pairs;
    if (!readPairs(c, values, timeline, &pairs, error)) return false;
    Entry e;
    e.source = &source;
    e.aggregation = int(o.aggregation);
    e.interpolate = o.interpolate;
    e.system = system;
    e.count = qint64(pairs.size());
    hashPairs(pairs, &e.hash1, &e.hash2);
    BuiltPtr built;
    {
        std::vector<Entry> &list = entries();
        for (size_t i = 0; i < list.size() && !built; ++i)
            if (!list[i].byReference && matches(list[i]) && list[i].count == e.count && list[i].hash1 == e.hash1 && list[i].hash2 == e.hash2) built = list[i].built;
    }
    if (!built) {
        if (!m.charge(buildWork(e.count))) {
            *error = Value::error(ErrorKind::Value);
            return false;
        }
        built = std::make_shared<Built>();
        if (ets::prepare(std::move(pairs), o.aggregation, o.interpolate, system, &built->prepared) != ets::Problem::None) built->error = Value::error(ErrorKind::Num);
        Entry byContent = e;
        byContent.built = built;
        remember(std::move(byContent));
    }
    if (tokens && onlyConstants(source, va) && onlyConstants(source, ta)) {
        e.byReference = true;
        e.values = va;
        e.times = ta;
        e.valuesToken = vt;
        e.timesToken = tt;
        e.built = built;
        remember(std::move(e));
    }
    *out = built;
    *error = built->error;
    return built->error.isEmpty();
}

// The season the options ask for, found (once) from the series if they say so.
bool seasonOf(Call &c, Built &b, const Options &o, int *season)
{
    if (o.season != 1) {
        *season = o.season;
        return true;
    }
    if (b.detected < 0) {
        if (!c.machine().charge(detectWork(b.points()))) return false;
        b.detected = ets::detectSeason(b.prepared.values);
    }
    *season = b.detected;
    return true;
}

// The model of the series (fitted once for a season); false if the work gave out.
bool modelOf(Call &c, Built &b, const Options &o, std::shared_ptr<const ets::Model> *model)
{
    int season;
    if (!seasonOf(c, b, o, &season)) return false;
    if (b.modelSeason != season) {
        if (!c.machine().charge(fitWork(b.points()))) return false;
        b.model = ets::fit(b.prepared.values, season);
        b.modelSeason = season;
    }
    *model = b.model;
    return true;
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
    BuiltPtr b;
    Value err;
    if (!series(c, 1, 2, o, &b, &err)) return err;
    std::shared_ptr<const ets::Model> m;
    if (!modelOf(c, *b, o, &m)) return Value::error(ErrorKind::Value);
    long long h;
    double out;
    if (!m || !horizonOf(b->prepared, target, &h) || !m->forecast(h, &out)) return c.error(ErrorKind::Num);
    return numberResult(out);
}

Value fnForecastEtsConfint(Call &c)
{
    double target, confidence = 0.95;
    Options o;
    if (!c.number(0, &target)) return c.failure();
    if (!omitted(c, 3) && !c.number(3, &confidence)) return c.failure();
    if (!readOptions(c, 4, 5, 6, &o)) return c.failure();
    BuiltPtr b;
    Value err;
    if (!series(c, 1, 2, o, &b, &err)) return err;
    std::shared_ptr<const ets::Model> m;
    if (!modelOf(c, *b, o, &m)) return Value::error(ErrorKind::Value);
    long long h;
    double out;
    if (!m || !horizonOf(b->prepared, target, &h) || !m->interval(h, confidence, &out)) return c.error(ErrorKind::Num);
    return numberResult(out);
}

Value fnForecastEtsSeasonality(Call &c)
{
    Options o;
    if (!readOptions(c, -1, 2, 3, &o)) return c.failure();
    BuiltPtr b;
    Value err;
    if (!series(c, 0, 1, o, &b, &err)) return err;
    int s;
    if (!seasonOf(c, *b, o, &s)) return Value::error(ErrorKind::Value);
    return Value::number(s < 2 ? 1 : s);
}

Value fnForecastEtsStat(Call &c)
{
    double type;
    Options o;
    if (!c.number(2, &type) || !readOptions(c, 3, 4, 5, &o)) return c.failure();
    BuiltPtr b;
    Value err;
    if (!series(c, 0, 1, o, &b, &err)) return err;
    const int k = int(type);
    if (k < 1 || k > 8) return c.error(ErrorKind::Num);
    if (k == 8) return numberResult(b->prepared.timeline.step);
    std::shared_ptr<const ets::Model> m;
    if (!modelOf(c, *b, o, &m)) return Value::error(ErrorKind::Value);
    if (!m) return c.error(ErrorKind::Num);
    const double v[] = {m->alpha, m->beta, m->gamma, m->mase, m->smape, m->mae, m->rmse};
    return numberResult(v[k - 1]);
}

} // namespace

void registerEtsFunctions(std::vector<FunctionInfo> &r)
{
    r.push_back({"FORECAST.ETS", 3, 6, "vrrvvv", kFnFuture | kFnKeepStored, fnForecastEts});
    r.push_back({"FORECAST.ETS.CONFINT", 3, 7, "vrrvvvv", kFnFuture | kFnKeepStored, fnForecastEtsConfint});
    r.push_back({"FORECAST.ETS.SEASONALITY", 2, 4, "rrvv", kFnFuture | kFnKeepStored, fnForecastEtsSeasonality});
    r.push_back({"FORECAST.ETS.STAT", 3, 6, "rrvvvv", kFnFuture | kFnKeepStored, fnForecastEtsStat});
}

} // namespace jp::calc
