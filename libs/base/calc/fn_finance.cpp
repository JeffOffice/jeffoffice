// The financial functions that have a closed form: loans and savings, net present value, and straight-line depreciation. (The ones that are solved by trying
// numbers, IRR and RATE, give the digits of the iteration the original makes, which are not reproduced here.)
#include "calc/machine.h"

#include <cmath>

namespace jp::calc {

namespace {

// These are worked out in the widest floating-point type the platform has (80 bits on x86, as the original works them out; the same as a double elsewhere), so
// that the last digits of a loan's payment are those of the files: in double, they are off by some units of the last place.
using real = long double;

// (1 + rate) to the power n.
real growth(real rate, real n) { return std::pow(1 + rate, n); }

real futureValue(real rate, real n, real pmt, real pv, real type)
{
    if (rate == 0) return -(pv + pmt * n);
    const real g = growth(rate, n);
    return -(pv * g + pmt * (1 + rate * type) * (g - 1) / rate);
}

bool payment(real rate, real n, real pv, real fv, real type, real *out)
{
    if (n == 0) return false;
    if (rate == 0) {
        *out = -(pv + fv) / n;
        return true;
    }
    const real g = growth(rate, n);
    *out = -(fv + pv * g) * rate / ((1 + rate * type) * (g - 1));
    return true;
}

Value fnPmt(Call &c)
{
    double rate, n, pv, fv, type;
    if (!c.number(0, &rate) || !c.number(1, &n) || !c.number(2, &pv) || !c.numberOr(3, 0, &fv) || !c.numberOr(4, 0, &type)) return c.failure();
    real p;
    if (!payment(rate, n, pv, fv, type != 0 ? 1 : 0, &p)) return c.error(rate == 0 ? ErrorKind::Div0 : ErrorKind::Num);
    return numberResult(double(p));
}

Value fnFv(Call &c)
{
    double rate, n, pmt, pv, type;
    if (!c.number(0, &rate) || !c.number(1, &n) || !c.number(2, &pmt) || !c.numberOr(3, 0, &pv) || !c.numberOr(4, 0, &type)) return c.failure();
    return numberResult(double(futureValue(rate, n, pmt, pv, type != 0 ? 1 : 0)));
}

Value fnPv(Call &c)
{
    double rate, n, pmt, fv, type;
    if (!c.number(0, &rate) || !c.number(1, &n) || !c.number(2, &pmt) || !c.numberOr(3, 0, &fv) || !c.numberOr(4, 0, &type)) return c.failure();
    type = type != 0 ? 1 : 0;
    if (rate == 0) return numberResult(-(fv + pmt * n));
    const real g = growth(rate, n);
    return numberResult(double(-(fv + pmt * (1 + rate * type) * (g - 1) / rate) / g));
}

Value fnNper(Call &c)
{
    double rate, pmt, pv, fv, type;
    if (!c.number(0, &rate) || !c.number(1, &pmt) || !c.number(2, &pv) || !c.numberOr(3, 0, &fv) || !c.numberOr(4, 0, &type)) return c.failure();
    type = type != 0 ? 1 : 0;
    if (rate == 0) {
        if (pmt == 0) return c.error(ErrorKind::Num);
        return numberResult(-(pv + fv) / pmt);
    }
    const double top = pmt * (1 + rate * type) - fv * rate, bottom = pmt * (1 + rate * type) + pv * rate;
    if (bottom == 0 || top / bottom <= 0 || 1 + rate <= 0) return c.error(ErrorKind::Num);
    return numberResult(std::log(top / bottom) / std::log(1 + rate));
}

// The interest and the principal of a payment (IPMT, PPMT): the same arguments, and the number of the payment.
Value interestOrPrincipal(Call &c, bool principal)
{
    double rate, per, n, pv, fv, type;
    if (!c.number(0, &rate) || !c.number(1, &per) || !c.number(2, &n) || !c.number(3, &pv) || !c.numberOr(4, 0, &fv) || !c.numberOr(5, 0, &type)) return c.failure();
    type = type != 0 ? 1 : 0;
    per = std::trunc(per);
    if (per < 1 || per > n) return c.error(ErrorKind::Num);
    real pmt;
    if (!payment(rate, n, pv, fv, type, &pmt)) return c.error(rate == 0 ? ErrorKind::Div0 : ErrorKind::Num);
    real owed;
    if (per == 1)
        owed = type > 0 ? 0 : -pv;
    else if (type > 0)
        owed = futureValue(rate, per - 2, pmt, pv, 1) - pmt;
    else
        owed = futureValue(rate, per - 1, pmt, pv, 0);
    const real interest = owed * rate;
    return numberResult(double(principal ? pmt - interest : interest));
}
Value fnIpmt(Call &c) { return interestOrPrincipal(c, false); }
Value fnPpmt(Call &c) { return interestOrPrincipal(c, true); }

Value fnNpv(Call &c)
{
    double rate;
    if (!c.number(0, &rate)) return c.failure();
    std::vector<double> values;
    Value error;
    if (!gatherNumbers(c, 1, &values, &error)) return error;
    if (rate == -1) return c.error(ErrorKind::Div0);
    real sum = 0;
    for (size_t i = 0; i < values.size(); ++i) sum += values[i] / growth(rate, real(i + 1));
    return numberResult(double(sum));
}

Value fnSln(Call &c)
{
    double cost, salvage, life;
    if (!c.number(0, &cost) || !c.number(1, &salvage) || !c.number(2, &life)) return c.failure();
    if (life == 0) return c.error(ErrorKind::Div0);
    return numberResult((cost - salvage) / life);
}

Value fnSyd(Call &c)
{
    double cost, salvage, life, per;
    if (!c.number(0, &cost) || !c.number(1, &salvage) || !c.number(2, &life) || !c.number(3, &per)) return c.failure();
    if (life <= 0 || per <= 0 || per > life) return c.error(ErrorKind::Num);
    return numberResult((cost - salvage) * (life - per + 1) * 2 / (life * (life + 1)));
}

Value fnEffect(Call &c)
{
    double nominal, periods;
    if (!c.number(0, &nominal) || !c.number(1, &periods)) return c.failure();
    periods = std::trunc(periods);
    if (nominal <= 0 || periods < 1) return c.error(ErrorKind::Num);
    return numberResult(double(growth(nominal / periods, periods) - 1));
}

Value fnNominal(Call &c)
{
    double effective, periods;
    if (!c.number(0, &effective) || !c.number(1, &periods)) return c.failure();
    periods = std::trunc(periods);
    if (effective <= 0 || periods < 1) return c.error(ErrorKind::Num);
    return numberResult(periods * (std::exp(std::log(1 + effective) / periods) - 1));
}

// ---- depreciation by the declining balance

// The depreciation of one period by the fixed declining balance: `factor` over the life is the rate; never below what leaves the salvage value.
double decliningBalance(double cost, double salvage, double life, double period, double factor)
{
    double rate = factor / life, before, after;
    if (rate >= 1) {
        rate = 1;
        before = period == 1 ? cost : 0;
    } else {
        before = cost * std::pow(1 - rate, period - 1);
    }
    after = cost * std::pow(1 - rate, period);
    const double d = after < salvage ? before - salvage : before - after;
    return d < 0 ? 0 : d;
}

Value fnDdb(Call &c)
{
    double cost, salvage, life, period, factor;
    if (!c.number(0, &cost) || !c.number(1, &salvage) || !c.number(2, &life) || !c.number(3, &period) || !c.numberOr(4, 2, &factor)) return c.failure();
    if (cost < 0 || salvage < 0 || life <= 0 || period <= 0 || period > life || factor <= 0) return c.error(ErrorKind::Num);
    return numberResult(decliningBalance(cost, salvage, life, period, factor));
}

// The depreciation of the stretch of the asset's life from `start` to `end`, in periods: the declining balance in each period (or, once the straight line over what is left of
// the life gives more, the straight line), and of a period the stretch begins or ends in, the part the stretch covers. The amounts of the periods are those from the first
// period of the life whatever the stretch, so that stretches that make up a longer one make up its amount.
double variableBalance(double cost, double salvage, double life, double start, double end, double factor, bool noSwitch)
{
    double total = 0, left = cost - salvage, line = 0;
    bool straight = false;
    const qint64 last = qint64(std::ceil(end));
    for (qint64 i = 1; i <= last; ++i) {
        const double declining = decliningBalance(cost, salvage, life, double(i), factor);
        double term = declining;
        if (!noSwitch) {
            if (!straight) {
                line = left / (life - double(i - 1));
                if (line > declining) {
                    term = line;
                    straight = true;
                } else {
                    left -= declining;
                }
            } else {
                term = line;
            }
        }
        const double covered = std::min(end, double(i)) - std::max(start, double(i - 1));
        if (covered > 0) total += term * covered;
    }
    return total;
}

Value fnVdb(Call &c)
{
    double cost, salvage, life, start, end, factor;
    bool noSwitch;
    if (!c.number(0, &cost) || !c.number(1, &salvage) || !c.number(2, &life) || !c.number(3, &start) || !c.number(4, &end) || !c.numberOr(5, 2, &factor) || !c.booleanOr(6, false, &noSwitch))
        return c.failure();
    if (cost < 0 || salvage < 0 || life <= 0 || start < 0 || end < start || end > life || factor <= 0) return c.error(ErrorKind::Num);
    if (!c.machine().charge(qint64(std::min(std::ceil(end), 1e7)) + 1)) return c.error(ErrorKind::Value);
    return numberResult(variableBalance(cost, salvage, life, start, end, factor, noSwitch));
}

// ---- cash flows: IRR and XIRR are found by Newton's method from a guess; XNPV is a sum

namespace {

// The value of the flows at a rate: the sum of each flow over (1 + rate) to the power of its time, the slope of that, and the sum of their sizes (what the value is measured
// against: a rate is found where the value is nothing next to that, not where a step is small).
struct RateTerms {
    double value = 0, slope = 0, scale = 0;
};

RateTerms termsAt(const std::vector<double> &v, const std::vector<double> &t, double rate)
{
    RateTerms x;
    const double base = 1 + rate;
    for (size_t i = 0; i < v.size(); ++i) {
        const double term = v[i] / std::pow(base, t[i]);
        x.value += term;
        x.scale += std::fabs(term);
        x.slope -= t[i] * term / base;
    }
    return x;
}

bool isRoot(const RateTerms &x) { return std::isfinite(x.value) && std::isfinite(x.scale) && std::fabs(x.value) <= 1e-12 * x.scale; }

// The rate where the value of the flows is nothing, from a guess: Newton's method, which never goes to -1 or below (it goes half the way there), and when that does not come
// to a rate in sixty steps, the rate nearest the guess between two of a set of rates from just above -1 to a million at which the value changes sign, by halving. False if there
// is none. `c` is charged for each time the flows are worked out.
bool solveRate(Call &c, const std::vector<double> &v, const std::vector<double> &t, double guess, double *out)
{
    const qint64 n = qint64(v.size());
    double rate = guess;
    for (int iteration = 0; iteration < 60; ++iteration) {
        if (!c.machine().charge(n)) return false;
        const RateTerms x = termsAt(v, t, rate);
        if (!std::isfinite(x.value) || !std::isfinite(x.slope)) break;
        if (isRoot(x)) {
            *out = rate;
            return true;
        }
        if (x.slope == 0) break;
        double next = rate - x.value / x.slope;
        if (!std::isfinite(next)) break;
        if (next <= -1) next = (rate - 1) / 2;
        if (next == rate) break;
        rate = next;
    }
    static const double samples[] = {-0.999999999, -0.9999999, -0.99999, -0.999, -0.99, -0.9, -0.75, -0.5, -0.25, -0.1, -0.01, 0, 0.01, 0.05, 0.1, 0.25, 0.5, 1, 2, 5, 10, 100, 1e3, 1e4, 1e6};
    const int count = int(sizeof(samples) / sizeof(samples[0]));
    double at[sizeof(samples) / sizeof(samples[0])];
    for (int i = 0; i < count; ++i) {
        if (!c.machine().charge(n)) return false;
        at[i] = termsAt(v, t, samples[i]).value;
    }
    double best = 0, bestDistance = 0;
    bool found = false;
    for (int i = 0; i + 1 < count; ++i) {
        if (!std::isfinite(at[i]) || !std::isfinite(at[i + 1]) || !((at[i] <= 0) != (at[i + 1] <= 0) || at[i] == 0)) continue;
        // (nearest the guess, as the rates are spaced: by the logarithm of what is added to 1)
        const double distance = std::fabs(std::log((1 + samples[i]) / (1 + std::max(guess, -0.999999999999))));
        if (!found || distance < bestDistance) {
            found = true;
            bestDistance = distance;
            best = double(i);
        }
    }
    if (!found) return false;
    double lo = samples[int(best)], hi = samples[int(best) + 1];
    double flo = at[int(best)];
    for (int iteration = 0; iteration < 200; ++iteration) {
        if (!c.machine().charge(n)) return false;
        const double mid = (lo + hi) / 2;
        const RateTerms x = termsAt(v, t, mid);
        if (isRoot(x) || mid == lo || mid == hi) {
            *out = mid;
            return true;
        }
        if ((x.value <= 0) == (flo <= 0)) {
            lo = mid;
            flo = x.value;
        } else {
            hi = mid;
        }
    }
    *out = (lo + hi) / 2;
    return true;
}

} // namespace

Value fnIrr(Call &c)
{
    double guess = 0.1;
    if (c.argc() > 1 && !c.missing(1) && !c.number(1, &guess)) return c.failure();
    std::vector<double> v;
    Value error;
    bool bad = false;
    const bool done = c.each(0, [&](const Value &x, bool typed) {
        if (x.isError()) {
            error = x;
            bad = true;
            return false;
        }
        double n;
        ErrorKind e = ErrorKind::Value;
        if (x.isNumber()) v.push_back(x.number());
        else if (!typed) return true;   // (a logical or a text in a reference or an array is not a flow)
        else if (x.isBool()) v.push_back(x.number());
        else if (coerceNumber(x, c.context().dateSystem, &n, &e)) v.push_back(n);
        else {
            error = Value::error(e);
            bad = true;
            return false;
        }
        return true;
    });
    if (!done) return bad ? error : c.error(ErrorKind::Value);
    bool positive = false, negative = false;
    for (const double x : v) {
        positive = positive || x > 0;
        negative = negative || x < 0;
    }
    if (!positive || !negative || guess <= -1) return c.error(ErrorKind::Num);
    std::vector<double> times(v.size());
    for (size_t i = 0; i < v.size(); ++i) times[i] = double(i);
    double rate;
    if (!solveRate(c, v, times, guess, &rate)) return c.machine().stopped() ? c.error(ErrorKind::Value) : c.error(ErrorKind::Num);
    return rate > -1 ? numberResult(rate) : c.error(ErrorKind::Num);
}

// The values and the dates (whole days) of XNPV and XIRR: as many of each; false, with the error, if they are not.
bool datedFlows(Call &c, int valuesAt, int datesAt, std::vector<double> *v, std::vector<double> *d, Value *error)
{
    for (int pass = 0; pass < 2; ++pass) {
        std::vector<double> &out = pass == 0 ? *v : *d;
        bool bad = false;
        const bool done = c.each(pass == 0 ? valuesAt : datesAt, [&](const Value &x, bool typed) {
            if (x.isError()) {
                *error = x;
                bad = true;
                return false;
            }
            double n;
            ErrorKind e = ErrorKind::Value;
            if (x.isNumber() || (typed && x.isBool())) out.push_back(pass == 0 ? x.number() : std::floor(x.number()));   // (a logical in a reference or an array is left out)
            else if (!typed && x.isBool()) return true;
            else if (typed && coerceNumber(x, c.context().dateSystem, &n, &e)) out.push_back(pass == 0 ? n : std::floor(n));
            else if (typed || pass == 1) {
                *error = Value::error(typed ? e : ErrorKind::Value);
                bad = true;
                return false;
            }
            return true;
        });
        if (!done) {
            if (!bad) *error = c.error(ErrorKind::Value);
            return false;
        }
    }
    if (v->size() != d->size() || v->empty()) {
        *error = c.error(ErrorKind::Num);
        return false;
    }
    return true;
}

double datedValue(double rate, const std::vector<double> &v, const std::vector<double> &d)
{
    double sum = 0;
    for (size_t i = 0; i < v.size(); ++i) sum += v[i] / powerOf(1 + rate, (d[i] - d[0]) / 365);
    return sum;
}

Value fnXnpv(Call &c)
{
    double rate;
    if (!c.number(0, &rate)) return c.failure();
    std::vector<double> v, d;
    Value error;
    if (!datedFlows(c, 1, 2, &v, &d, &error)) return error;
    if (rate <= -1) return c.error(ErrorKind::Num);
    for (const double day : d)
        if (day < d[0]) return c.error(ErrorKind::Num);
    if (!c.machine().charge(qint64(v.size()) + 1)) return c.error(ErrorKind::Value);
    return numberResult(datedValue(rate, v, d));
}

Value fnXirr(Call &c)
{
    double guess = 0.1;
    if (c.argc() > 2 && !c.missing(2) && !c.number(2, &guess)) return c.failure();
    std::vector<double> v, d;
    Value error;
    if (!datedFlows(c, 0, 1, &v, &d, &error)) return error;
    bool positive = false, negative = false;
    for (const double x : v) {
        positive = positive || x > 0;
        negative = negative || x < 0;
    }
    if (!positive || !negative || guess <= -1) return c.error(ErrorKind::Num);
    for (const double day : d)
        if (day < d[0]) return c.error(ErrorKind::Num);
    std::vector<double> years(v.size());
    for (size_t i = 0; i < v.size(); ++i) years[i] = (d[i] - d[0]) / 365;
    double rate;
    if (!solveRate(c, v, years, guess, &rate)) return c.machine().stopped() ? c.error(ErrorKind::Value) : c.error(ErrorKind::Num);
    return rate > -1 ? numberResult(rate) : c.error(ErrorKind::Num);
}

} // namespace

void registerFinanceFunctions(std::vector<FunctionInfo> &r)
{
    r.push_back({"PMT", 3, 5, "vvvvv", 0, fnPmt});
    r.push_back({"FV", 3, 5, "vvvvv", 0, fnFv});
    r.push_back({"PV", 3, 5, "vvvvv", 0, fnPv});
    r.push_back({"NPER", 3, 5, "vvvvv", 0, fnNper});
    r.push_back({"IPMT", 4, 6, "vvvvvv", 0, fnIpmt});
    r.push_back({"PPMT", 4, 6, "vvvvvv", 0, fnPpmt});
    r.push_back({"NPV", 2, 255, "vr", 0, fnNpv});
    r.push_back({"SLN", 3, 3, "vvv", 0, fnSln});
    r.push_back({"SYD", 4, 4, "vvvv", 0, fnSyd});
    r.push_back({"EFFECT", 2, 2, "vv", 0, fnEffect});
    r.push_back({"NOMINAL", 2, 2, "vv", 0, fnNominal});
    r.push_back({"DDB", 4, 5, "vvvvv", 0, fnDdb});
    r.push_back({"VDB", 5, 7, "vvvvvvv", 0, fnVdb});
    r.push_back({"IRR", 1, 2, "rv", kFnIterative, fnIrr});
    r.push_back({"XNPV", 3, 3, "vrr", 0, fnXnpv});
    r.push_back({"XIRR", 2, 3, "rrv", kFnIterative, fnXirr});
}

} // namespace jp::calc
