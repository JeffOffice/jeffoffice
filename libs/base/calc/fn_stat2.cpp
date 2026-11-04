// More statistics: the shape of a set of numbers, ranks and percentiles of the newer kinds, the probability distributions and their inverses, the tests, and
// the fitted lines and curves.
#include "calc/machine.h"
#include "calc/special.h"

#include <algorithm>
#include <cmath>
#include <map>

namespace jp::calc {

namespace {

constexpr double kPi = 3.14159265358979323846;

// ---- reading the arguments

// The numbers of the first `n` arguments, truncated when `whole` says so; false (with failure()) for a bad one.
bool args(Call &c, double *v, int n)
{
    for (int i = 0; i < n; ++i)
        if (!c.number(i, &v[i])) return false;
    return true;
}

// The numbers of argument i: in a reference or an array the numbers (text, true and false, and empty cells are left out); an error stops it.
bool numbersAt(Call &c, int i, std::vector<double> *x, Value *error)
{
    bool ok = true;
    const bool done = c.each(i, [&](const Value &v, bool typed) {
        if (v.isNumber()) x->push_back(v.number());
        else if (v.isError()) {
            *error = v;
            ok = false;
            return false;
        } else if (typed && v.isBool()) x->push_back(v.number());
        return true;
    });
    if (!done) {
        if (ok) *error = Value::error(ErrorKind::Value);
        return false;
    }
    return true;
}

// The pairs of numbers of two blocks of one shape (a pair with something else in it is left out).
bool pairsAt(Call &c, int a, int b, std::vector<double> *xs, std::vector<double> *ys, Value *error)
{
    Value va, vb;
    if (!c.array(a, &va) || !c.array(b, &vb)) {
        *error = c.failure();
        return false;
    }
    if (va.rows() != vb.rows() || va.cols() != vb.cols()) {
        *error = Value::error(ErrorKind::NA);
        return false;
    }
    for (int i = 0; i < va.rows(); ++i)
        for (int j = 0; j < va.cols(); ++j) {
            const Value &x = va.at(i, j), &y = vb.at(i, j);
            if (x.isError()) {
                *error = x;
                return false;
            }
            if (y.isError()) {
                *error = y;
                return false;
            }
            if (x.isNumber() && y.isNumber()) {
                xs->push_back(x.number());
                ys->push_back(y.number());
            }
        }
    return true;
}

// The numbers of the arguments as the "A" functions take them: in a reference a text counts as 0 and true as 1.
bool gatherA(Call &c, std::vector<double> *x, Value *error)
{
    for (int i = 0; i < c.argc(); ++i) {
        bool ok = true;
        const bool done = c.each(i, [&](const Value &v, bool typed) {
            double d;
            switch (v.kind()) {
            case Value::Kind::Number:
            case Value::Kind::Bool: x->push_back(v.number()); break;
            case Value::Kind::Text:
                if (typed) {
                    if (!textToNumber(v.text(), c.context().dateSystem, &d)) {
                        *error = Value::error(ErrorKind::Value);
                        ok = false;
                        return false;
                    }
                    x->push_back(d);
                } else x->push_back(0);
                break;
            case Value::Kind::Error:
                *error = v;
                ok = false;
                return false;
            case Value::Kind::Empty:
                if (typed) x->push_back(0);
                break;
            default: break;
            }
            return true;
        });
        if (!done) {
            if (ok) *error = Value::error(ErrorKind::Value);
            return false;
        }
    }
    return true;
}

double meanOf(const std::vector<double> &x)
{
    double s = 0;
    for (const double v : x) s += v;
    return s / double(x.size());
}

double sumSquares(const std::vector<double> &x, double mean)
{
    double s = 0;
    for (const double v : x) s += (v - mean) * (v - mean);
    return s;
}

bool charged(Call &c, qint64 n) { return c.machine().charge(n); }

// ---- the shape of a set of numbers

Value spreadA(Call &c, bool sample, bool root)
{
    std::vector<double> x;
    Value error;
    if (!gatherA(c, &x, &error)) return error;
    const size_t n = x.size();
    if (sample ? n < 2 : n < 1) return c.error(ErrorKind::Div0);
    const double var = sumSquares(x, meanOf(x)) / double(sample ? n - 1 : n);
    return numberResult(root ? std::sqrt(var) : var);
}
Value fnStdevA(Call &c) { return spreadA(c, true, true); }
Value fnStdevPA(Call &c) { return spreadA(c, false, true); }
Value fnVarA(Call &c) { return spreadA(c, true, false); }
Value fnVarPA(Call &c) { return spreadA(c, false, false); }

Value fnSkew(Call &c)
{
    std::vector<double> x;
    Value error;
    if (!gatherNumbers(c, 0, &x, &error)) return error;
    const double n = double(x.size());
    if (n < 3) return c.error(ErrorKind::Div0);
    const double m = meanOf(x), sd = std::sqrt(sumSquares(x, m) / (n - 1));
    if (sd == 0) return c.error(ErrorKind::Div0);
    double s = 0;
    for (const double v : x) s += std::pow((v - m) / sd, 3);
    return numberResult(n / ((n - 1) * (n - 2)) * s);
}

Value fnSkewP(Call &c)
{
    std::vector<double> x;
    Value error;
    if (!gatherNumbers(c, 0, &x, &error)) return error;
    const double n = double(x.size());
    if (n < 1) return c.error(ErrorKind::Div0);
    const double m = meanOf(x), sd = std::sqrt(sumSquares(x, m) / n);
    if (sd == 0) return c.error(ErrorKind::Div0);
    double s = 0;
    for (const double v : x) s += std::pow((v - m) / sd, 3);
    return numberResult(s / n);
}

Value fnKurt(Call &c)
{
    std::vector<double> x;
    Value error;
    if (!gatherNumbers(c, 0, &x, &error)) return error;
    const double n = double(x.size());
    if (n < 4) return c.error(ErrorKind::Div0);
    const double m = meanOf(x), sd = std::sqrt(sumSquares(x, m) / (n - 1));
    if (sd == 0) return c.error(ErrorKind::Div0);
    double s = 0;
    for (const double v : x) s += std::pow((v - m) / sd, 4);
    return numberResult(n * (n + 1) / ((n - 1) * (n - 2) * (n - 3)) * s - 3 * (n - 1) * (n - 1) / ((n - 2) * (n - 3)));
}

Value fnGeomean(Call &c)
{
    std::vector<double> x;
    Value error;
    if (!gatherNumbers(c, 0, &x, &error)) return error;
    if (x.empty()) return c.error(ErrorKind::Num);
    double s = 0;
    for (const double v : x) {
        if (v <= 0) return c.error(ErrorKind::Num);
        s += std::log(v);
    }
    return numberResult(std::exp(s / double(x.size())));
}

Value fnHarmean(Call &c)
{
    std::vector<double> x;
    Value error;
    if (!gatherNumbers(c, 0, &x, &error)) return error;
    if (x.empty()) return c.error(ErrorKind::Num);
    double s = 0;
    for (const double v : x) {
        if (v <= 0) return c.error(ErrorKind::Num);
        s += 1 / v;
    }
    return numberResult(double(x.size()) / s);
}

Value fnAvedev(Call &c)
{
    std::vector<double> x;
    Value error;
    if (!gatherNumbers(c, 0, &x, &error)) return error;
    if (x.empty()) return c.error(ErrorKind::Num);
    const double m = meanOf(x);
    double s = 0;
    for (const double v : x) s += std::fabs(v - m);
    return numberResult(s / double(x.size()));
}

Value fnDevsq(Call &c)
{
    std::vector<double> x;
    Value error;
    if (!gatherNumbers(c, 0, &x, &error)) return error;
    if (x.empty()) return c.error(ErrorKind::Num);
    return numberResult(sumSquares(x, meanOf(x)));
}

Value fnTrimmean(Call &c)
{
    std::vector<double> x;
    Value error;
    double pct;
    if (!numbersAt(c, 0, &x, &error)) return error;
    if (!c.number(1, &pct)) return c.failure();
    if (pct < 0 || pct >= 1 || x.empty()) return c.error(ErrorKind::Num);
    std::sort(x.begin(), x.end());
    const size_t cut = size_t(std::floor(double(x.size()) * pct / 2));
    double s = 0;
    for (size_t i = cut; i < x.size() - cut; ++i) s += x[i];
    return numberResult(s / double(x.size() - 2 * cut));
}

Value fnFisher(Call &c)
{
    double x;
    if (!c.number(0, &x)) return c.failure();
    if (!(std::fabs(x) < 1)) return c.error(ErrorKind::Num);
    return numberResult(0.5 * std::log((1 + x) / (1 - x)));
}

Value fnFisherInv(Call &c)
{
    double y;
    if (!c.number(0, &y)) return c.failure();
    return numberResult(std::tanh(y));
}

Value fnStandardize(Call &c)
{
    double v[3];
    if (!args(c, v, 3)) return c.failure();
    if (v[2] <= 0) return c.error(ErrorKind::Num);
    return numberResult((v[0] - v[1]) / v[2]);
}

Value fnPermutationA(Call &c)
{
    double v[2];
    if (!args(c, v, 2)) return c.failure();
    v[0] = std::trunc(v[0]);
    v[1] = std::trunc(v[1]);
    if (v[0] < 0 || v[1] < 0) return c.error(ErrorKind::Num);
    return numberResult(powerOf(v[0], v[1]));
}

Value fnGammaLn(Call &c)
{
    double x;
    if (!c.number(0, &x)) return c.failure();
    if (x <= 0) return c.error(ErrorKind::Num);
    return numberResult(std::lgamma(x));
}

Value fnGamma(Call &c)
{
    double x;
    if (!c.number(0, &x)) return c.failure();
    if (x == 0 || (x < 0 && x == std::floor(x))) return c.error(ErrorKind::Num);
    return numberResult(std::tgamma(x));
}

Value fnProb(Call &c)
{
    Value xs, ps;
    double lower, upper;
    if (!c.array(0, &xs) || !c.array(1, &ps) || !c.number(2, &lower)) return c.failure();
    const bool single = c.missing(3);
    upper = lower;
    if (!single && !c.number(3, &upper)) return c.failure();
    if (xs.rows() != ps.rows() || xs.cols() != ps.cols()) return c.error(ErrorKind::NA);
    double total = 0, hit = 0;
    if (upper < lower) std::swap(lower, upper);
    for (int i = 0; i < xs.rows(); ++i)
        for (int j = 0; j < xs.cols(); ++j) {
            const Value &x = xs.at(i, j), &p = ps.at(i, j);
            if (x.isError()) return x;
            if (p.isError()) return p;
            if (!x.isNumber() || !p.isNumber()) return c.error(ErrorKind::Value);
            if (p.number() < 0 || p.number() > 1) return c.error(ErrorKind::Num);
            total += p.number();
            if (x.number() >= lower && x.number() <= upper) hit += p.number();
        }
    if (std::fabs(total - 1) > 1e-9) return c.error(ErrorKind::Num);
    return numberResult(hit);
}

// ---- ranks and percentiles

Value rankAverage(Call &c)
{
    double x;
    bool descending = true;
    if (!c.number(0, &x)) return c.failure();
    if (!c.missing(2)) {
        double order;
        if (!c.number(2, &order)) return c.failure();
        descending = order == 0;
    }
    std::vector<double> v;
    Value error;
    if (!numbersAt(c, 1, &v, &error)) return error;
    double greater = 0, equal = 0;
    for (const double d : v) {
        if (d == x) ++equal;
        else if (descending ? d > x : d < x) ++greater;
    }
    if (equal == 0) return c.error(ErrorKind::NA);
    return numberResult(greater + 1 + (equal - 1) / 2);
}

Value exclusive(Call &c, double k)
{
    std::vector<double> x;
    Value error;
    if (!numbersAt(c, 0, &x, &error)) return error;
    const double n = double(x.size());
    if (n == 0 || !(k > 0 && k < 1)) return c.error(ErrorKind::Num);
    std::sort(x.begin(), x.end());
    const double pos = k * (n + 1);
    if (pos < 1 || pos > n) return c.error(ErrorKind::Num);
    const size_t lo = size_t(std::floor(pos));
    if (lo >= x.size()) return Value::number(x.back());
    return numberResult(x[lo - 1] + (x[lo] - x[lo - 1]) * (pos - double(lo)));
}

Value fnPercentileExc(Call &c)
{
    double k;
    if (!c.number(1, &k)) return c.failure();
    return exclusive(c, k);
}

Value fnQuartileExc(Call &c)
{
    double q;
    if (!c.number(1, &q)) return c.failure();
    q = std::trunc(q);
    if (q < 1 || q > 3) return c.error(ErrorKind::Num);
    return exclusive(c, q / 4);
}

// The digits of a rank cut (not rounded) to `significance` places.
double cutDigits(double v, int significance)
{
    const double scale = std::pow(10.0, significance);
    return std::floor(v * scale + 1e-9) / scale;
}

Value percentRank(Call &c, bool exc)
{
    std::vector<double> a;
    Value error;
    double x, significance = 3;
    if (!numbersAt(c, 0, &a, &error)) return error;
    if (!c.number(1, &x) || !c.numberOr(2, 3, &significance)) return c.failure();
    significance = std::trunc(significance);
    if (significance < 1) return c.error(ErrorKind::Num);
    if (a.empty()) return c.error(ErrorKind::NA);
    std::sort(a.begin(), a.end());
    const size_t n = a.size();
    if (x < a.front() || x > a.back()) return c.error(ErrorKind::NA);
    double rank;
    // the place of x among the sorted numbers, counting from 0, by interpolation when it falls between two
    size_t i = 0;
    while (i + 1 < n && a[i + 1] <= x) ++i;
    double place = double(i);
    if (a[i] != x) {
        if (i + 1 >= n) return c.error(ErrorKind::NA);
        place += (x - a[i]) / (a[i + 1] - a[i]);
    }
    if (exc) rank = (place + 1) / double(n + 1);
    else rank = n == 1 ? 1 : place / double(n - 1);
    if (exc && !(rank > 0 && rank < 1)) return c.error(ErrorKind::NA);
    return numberResult(cutDigits(rank, int(std::min(significance, 15.0))));
}
Value fnPercentRankInc(Call &c) { return percentRank(c, false); }
Value fnPercentRankExc(Call &c) { return percentRank(c, true); }

Value fnModeMult(Call &c)
{
    std::vector<double> x;
    Value error;
    if (!gatherNumbers(c, 0, &x, &error)) return error;
    std::map<double, size_t> counts;
    for (const double v : x) ++counts[v];
    size_t best = 1;
    for (const auto &kv : counts) best = std::max(best, kv.second);
    if (best < 2) return c.error(ErrorKind::NA);
    std::vector<double> modes;
    std::vector<double> seen;
    for (const double v : x)
        if (counts[v] == best && std::find(seen.begin(), seen.end(), v) == seen.end()) {
            seen.push_back(v);
            modes.push_back(v);
        }
    Value out = c.machine().newArray(int(modes.size()), 1);
    if (!out.isArray()) return out;
    for (size_t i = 0; i < modes.size(); ++i) out.set(int(i), 0, Value::number(modes[i]));
    return out;
}

Value fnFrequency(Call &c)
{
    std::vector<double> data, bins;
    Value error;
    if (!numbersAt(c, 0, &data, &error) || !numbersAt(c, 1, &bins, &error)) return error;
    if (!charged(c, qint64(data.size()) * 2 + qint64(bins.size()))) return c.error(ErrorKind::Value);
    std::vector<double> sorted = bins;
    std::sort(sorted.begin(), sorted.end());
    std::vector<double> counts(sorted.size() + 1, 0);
    for (const double d : data) {
        const size_t k = size_t(std::lower_bound(sorted.begin(), sorted.end(), d) - sorted.begin());
        counts[k] += 1;
    }
    // (the counts follow the bins in the order they were given: each bin gets the count of its place in the sorted bins; equal bins give 0 after the first)
    Value out = c.machine().newArray(int(bins.size()) + 1, 1);
    if (!out.isArray()) return out;
    for (size_t i = 0; i < bins.size(); ++i) {
        size_t k = size_t(std::lower_bound(sorted.begin(), sorted.end(), bins[i]) - sorted.begin());
        out.set(int(i), 0, Value::number(counts[k]));
        if (k < sorted.size()) {
            // a later equal bin counts nothing
            counts[k] = 0;
        }
    }
    out.set(int(bins.size()), 0, Value::number(counts[sorted.size()]));
    return out;
}

// ---- covariance, the standard error of a fit, tests of two sets

Value fnCovarianceS(Call &c)
{
    std::vector<double> xs, ys;
    Value error;
    if (!pairsAt(c, 0, 1, &xs, &ys, &error)) return error;
    const double n = double(xs.size());
    if (n < 2) return c.error(ErrorKind::Div0);
    const double mx = meanOf(xs), my = meanOf(ys);
    double s = 0;
    for (size_t i = 0; i < xs.size(); ++i) s += (xs[i] - mx) * (ys[i] - my);
    return numberResult(s / (n - 1));
}

Value fnSteyx(Call &c)
{
    std::vector<double> ys, xs;
    Value error;
    if (!pairsAt(c, 0, 1, &ys, &xs, &error)) return error;
    const double n = double(xs.size());
    if (n < 3) return c.error(ErrorKind::Div0);
    const double mx = meanOf(xs), my = meanOf(ys);
    double sxx = 0, syy = 0, sxy = 0;
    for (size_t i = 0; i < xs.size(); ++i) {
        sxx += (xs[i] - mx) * (xs[i] - mx);
        syy += (ys[i] - my) * (ys[i] - my);
        sxy += (xs[i] - mx) * (ys[i] - my);
    }
    if (sxx == 0) return c.error(ErrorKind::Div0);
    return numberResult(std::sqrt(std::max(0.0, (syy - sxy * sxy / sxx) / (n - 2))));
}

// ---- distributions: building blocks

double normalCdf(double z) { return 0.5 * std::erfc(-z / std::sqrt(2.0)); }
double normalPdf(double z) { return std::exp(-z * z / 2) / std::sqrt(2 * kPi); }

double logChoose(double n, double k) { return std::lgamma(n + 1) - std::lgamma(k + 1) - std::lgamma(n - k + 1); }

double binomialPmf(double k, double n, double p)
{
    if (p == 0) return k == 0 ? 1 : 0;
    if (p == 1) return k == n ? 1 : 0;
    return std::exp(logChoose(n, k) + k * std::log(p) + (n - k) * std::log1p(-p));
}

double binomialCdf(double k, double n, double p)
{
    if (k >= n) return 1;
    if (p == 0) return 1;
    if (p == 1) return 0;
    return betaI(n - k, k + 1, 1 - p);
}

double studentCdf(double t, double df)
{
    const double tail = 0.5 * betaI(df / 2, 0.5, df / (df + t * t));
    return t > 0 ? 1 - tail : tail;
}

double studentTail(double t, double df)   // the upper tail: P(T > t)
{
    const double tail = 0.5 * betaI(df / 2, 0.5, df / (df + t * t));
    return t > 0 ? tail : 1 - tail;
}

double studentTwoTail(double t, double df) { return betaI(df / 2, 0.5, df / (df + t * t)); }

double fCdf(double x, double d1, double d2)
{
    if (x <= 0) return 0;
    return betaI(d1 / 2, d2 / 2, d1 * x / (d1 * x + d2));
}

double fTail(double x, double d1, double d2)
{
    if (x <= 0) return 1;
    return betaI(d2 / 2, d1 / 2, d2 / (d2 + d1 * x));
}

// ---- the normal family

Value fnNormSDist2(Call &c)
{
    double z;
    bool cumulative;
    if (!c.number(0, &z) || !c.boolean(1, &cumulative)) return c.failure();
    return numberResult(cumulative ? normalCdf(z) : normalPdf(z));
}

Value fnGauss(Call &c)
{
    double z;
    if (!c.number(0, &z)) return c.failure();
    return numberResult(normalCdf(z) - 0.5);
}

Value fnPhi(Call &c)
{
    double z;
    if (!c.number(0, &z)) return c.failure();
    return numberResult(normalPdf(z));
}

Value fnLognormDist(Call &c)
{
    double v[3];
    bool cumulative = true;
    if (!args(c, v, 3)) return c.failure();
    if (c.argc() > 3 && !c.boolean(3, &cumulative)) return c.failure();
    if (v[0] <= 0 || v[2] <= 0) return c.error(ErrorKind::Num);
    const double z = (std::log(v[0]) - v[1]) / v[2];
    return numberResult(cumulative ? normalCdf(z) : normalPdf(z) / (v[0] * v[2]));
}

Value fnLognormInv(Call &c)
{
    double v[3], z;
    if (!args(c, v, 3)) return c.failure();
    if (v[2] <= 0 || !inverseStandardNormal(v[0], &z)) return c.error(ErrorKind::Num);
    return numberResult(std::exp(v[1] + v[2] * z));
}

Value fnConfidenceNorm(Call &c)
{
    double v[3], z;
    if (!args(c, v, 3)) return c.failure();
    v[2] = std::trunc(v[2]);
    if (!(v[0] > 0 && v[0] < 1) || v[1] <= 0 || v[2] < 1 || !inverseStandardNormal(1 - v[0] / 2, &z)) return c.error(ErrorKind::Num);
    return numberResult(z * v[1] / std::sqrt(v[2]));
}

Value fnZTest(Call &c)
{
    std::vector<double> x;
    Value error;
    double v, sigma = 0;
    if (!numbersAt(c, 0, &x, &error)) return error;
    if (!c.number(1, &v)) return c.failure();
    const bool hasSigma = !c.missing(2);
    if (hasSigma && !c.number(2, &sigma)) return c.failure();
    const double n = double(x.size());
    if (n == 0) return c.error(ErrorKind::NA);
    if (!hasSigma) {
        if (n < 2) return c.error(ErrorKind::Div0);
        sigma = std::sqrt(sumSquares(x, meanOf(x)) / (n - 1));
    }
    if (sigma <= 0 && hasSigma) return c.error(ErrorKind::Num);
    if (sigma == 0) return c.error(ErrorKind::Div0);
    return numberResult(1 - normalCdf((meanOf(x) - v) / (sigma / std::sqrt(n))));
}

// ---- the other continuous distributions

Value fnExponDist(Call &c)
{
    double v[2];
    bool cumulative;
    if (!args(c, v, 2) || !c.boolean(2, &cumulative)) return c.failure();
    if (v[0] < 0 || v[1] <= 0) return c.error(ErrorKind::Num);
    return numberResult(cumulative ? -std::expm1(-v[1] * v[0]) : v[1] * std::exp(-v[1] * v[0]));
}

Value fnWeibull(Call &c)
{
    double v[3];
    bool cumulative;
    if (!args(c, v, 3) || !c.boolean(3, &cumulative)) return c.failure();
    if (v[0] < 0 || v[1] <= 0 || v[2] <= 0) return c.error(ErrorKind::Num);
    const double t = std::pow(v[0] / v[2], v[1]);
    if (cumulative) return numberResult(-std::expm1(-t));
    if (v[0] == 0) return numberResult(v[1] == 1 ? 1 / v[2] : v[1] < 1 ? INFINITY : 0);
    return numberResult(v[1] / v[2] * std::pow(v[0] / v[2], v[1] - 1) * std::exp(-t));
}

Value fnGammaDist(Call &c)
{
    double v[3];
    bool cumulative;
    if (!args(c, v, 3) || !c.boolean(3, &cumulative)) return c.failure();
    if (v[0] < 0 || v[1] <= 0 || v[2] <= 0) return c.error(ErrorKind::Num);
    if (cumulative) return numberResult(gammaP(v[1], v[0] / v[2]));
    if (v[0] == 0) return numberResult(v[1] == 1 ? 1 / v[2] : v[1] < 1 ? INFINITY : 0);
    return numberResult(std::exp((v[1] - 1) * std::log(v[0]) - v[0] / v[2] - v[1] * std::log(v[2]) - std::lgamma(v[1])));
}

Value fnGammaInv(Call &c)
{
    double v[3], x;
    if (!args(c, v, 3)) return c.failure();
    if (v[0] < 0 || v[0] > 1 || v[1] <= 0 || v[2] <= 0) return c.error(ErrorKind::Num);
    if (v[0] == 0) return Value::number(0);
    if (v[0] == 1) return c.error(ErrorKind::Num);
    if (!inverseRising([&](double t) { return gammaP(v[1], t); }, v[0], 0, INFINITY, &x)) return c.error(ErrorKind::Num);
    return numberResult(x * v[2]);
}

// Beta with a lower and upper bound (A and B, 0 and 1 if left out), arguments `first` and `first + 1`.
bool betaBounds(Call &c, int first, double *a, double *b)
{
    *a = 0;
    *b = 1;
    return c.numberOr(first, 0, a) && c.numberOr(first + 1, 1, b);
}

Value fnBetaDist(Call &c)
{
    double v[3], lo, hi;
    bool cumulative = true;
    if (!args(c, v, 3)) return c.failure();
    if (!c.missing(3) && !c.boolean(3, &cumulative)) return c.failure();
    if (!betaBounds(c, 4, &lo, &hi)) return c.failure();
    if (v[1] <= 0 || v[2] <= 0 || v[0] < lo || v[0] > hi || lo == hi) return c.error(ErrorKind::Num);
    const double x = (v[0] - lo) / (hi - lo);
    if (cumulative) return numberResult(betaI(v[1], v[2], x));
    if (x == 0 || x == 1) return numberResult((x == 0 ? v[1] : v[2]) == 1 ? v[1 + (x == 1)] / (hi - lo) : (x == 0 ? v[1] : v[2]) < 1 ? INFINITY : 0);
    return numberResult(std::exp(std::lgamma(v[1] + v[2]) - std::lgamma(v[1]) - std::lgamma(v[2]) + (v[1] - 1) * std::log(x) + (v[2] - 1) * std::log1p(-x)) / (hi - lo));
}

// BETADIST: the older form, with no cumulative argument
Value fnBetaDistOld(Call &c)
{
    double v[3], lo, hi;
    if (!args(c, v, 3) || !betaBounds(c, 3, &lo, &hi)) return c.failure();
    if (v[1] <= 0 || v[2] <= 0 || v[0] < lo || v[0] > hi || lo == hi) return c.error(ErrorKind::Num);
    return numberResult(betaI(v[1], v[2], (v[0] - lo) / (hi - lo)));
}

Value fnBetaInv(Call &c)
{
    double v[3], lo, hi, x;
    if (!args(c, v, 3) || !betaBounds(c, 3, &lo, &hi)) return c.failure();
    if (v[0] <= 0 || v[0] > 1 || v[1] <= 0 || v[2] <= 0 || lo >= hi) return c.error(ErrorKind::Num);
    if (!inverseRising([&](double t) { return betaI(v[1], v[2], t); }, v[0], 0, 1, &x)) return c.error(ErrorKind::Num);
    return numberResult(lo + x * (hi - lo));
}

double chiPdf(double x, double df)
{
    if (x == 0) return df == 2 ? 0.5 : df < 2 ? INFINITY : 0;
    return std::exp((df / 2 - 1) * std::log(x) - x / 2 - (df / 2) * std::log(2.0) - std::lgamma(df / 2));
}

bool degrees(Call &c, int i, double *df)
{
    if (!c.number(i, df)) return false;
    *df = std::trunc(*df);
    return true;
}

Value fnChiDist(Call &c)
{
    double x, df;
    bool cumulative;
    if (!c.number(0, &x) || !degrees(c, 1, &df) || !c.boolean(2, &cumulative)) return c.failure();
    if (x < 0 || df < 1 || df > 1e10) return c.error(ErrorKind::Num);
    return numberResult(cumulative ? gammaP(df / 2, x / 2) : chiPdf(x, df));
}

Value fnChiDistRt(Call &c)
{
    double x, df;
    if (!c.number(0, &x) || !degrees(c, 1, &df)) return c.failure();
    if (x < 0 || df < 1 || df > 1e10) return c.error(ErrorKind::Num);
    return numberResult(gammaQ(df / 2, x / 2));
}

Value fnChiInv(Call &c)
{
    double p, df, x;
    if (!c.number(0, &p) || !degrees(c, 1, &df)) return c.failure();
    if (p < 0 || p >= 1 || df < 1 || df > 1e10) return c.error(ErrorKind::Num);
    if (p == 0) return Value::number(0);
    if (!inverseRising([&](double t) { return gammaP(df / 2, t / 2); }, p, 0, INFINITY, &x)) return c.error(ErrorKind::Num);
    return numberResult(x);
}

Value fnChiInvRt(Call &c)
{
    double p, df, x;
    if (!c.number(0, &p) || !degrees(c, 1, &df)) return c.failure();
    if (p <= 0 || p > 1 || df < 1 || df > 1e10) return c.error(ErrorKind::Num);
    if (p == 1) return Value::number(0);
    if (!inverseRising([&](double t) { return gammaP(df / 2, t / 2); }, 1 - p, 0, INFINITY, &x)) return c.error(ErrorKind::Num);
    return numberResult(x);
}

Value fnChiTest(Call &c)
{
    Value actual, expected;
    if (!c.array(0, &actual) || !c.array(1, &expected)) return c.failure();
    if (actual.rows() != expected.rows() || actual.cols() != expected.cols()) return c.error(ErrorKind::NA);
    double stat = 0;
    for (int i = 0; i < actual.rows(); ++i)
        for (int j = 0; j < actual.cols(); ++j) {
            const Value &a = actual.at(i, j), &e = expected.at(i, j);
            if (a.isError()) return a;
            if (e.isError()) return e;
            if (!a.isNumber() || !e.isNumber()) continue;
            if (e.number() == 0) return c.error(ErrorKind::Div0);
            stat += (a.number() - e.number()) * (a.number() - e.number()) / e.number();
        }
    const double r = actual.rows(), k = actual.cols();
    const double df = r > 1 && k > 1 ? (r - 1) * (k - 1) : r > 1 ? r - 1 : k - 1;
    if (df < 1) return c.error(ErrorKind::NA);
    return numberResult(gammaQ(df / 2, stat / 2));
}

Value fnFDist(Call &c)
{
    double x, d1, d2;
    bool cumulative;
    if (!c.number(0, &x) || !degrees(c, 1, &d1) || !degrees(c, 2, &d2) || !c.boolean(3, &cumulative)) return c.failure();
    if (x < 0 || d1 < 1 || d2 < 1 || d1 > 1e10 || d2 > 1e10) return c.error(ErrorKind::Num);
    if (cumulative) return numberResult(fCdf(x, d1, d2));
    if (x == 0) return numberResult(d1 == 2 ? 1 : d1 < 2 ? INFINITY : 0);
    return numberResult(std::exp(0.5 * (d1 * std::log(d1 * x) + d2 * std::log(d2) - (d1 + d2) * std::log(d1 * x + d2)) - std::log(x) - (std::lgamma(d1 / 2) + std::lgamma(d2 / 2) - std::lgamma((d1 + d2) / 2))));
}

Value fnFDistRt(Call &c)
{
    double x, d1, d2;
    if (!c.number(0, &x) || !degrees(c, 1, &d1) || !degrees(c, 2, &d2)) return c.failure();
    if (x < 0 || d1 < 1 || d2 < 1 || d1 > 1e10 || d2 > 1e10) return c.error(ErrorKind::Num);
    return numberResult(fTail(x, d1, d2));
}

Value fInverse(Call &c, bool rightTail)
{
    double p, d1, d2, x;
    if (!c.number(0, &p) || !degrees(c, 1, &d1) || !degrees(c, 2, &d2)) return c.failure();
    if (p < 0 || p > 1 || d1 < 1 || d2 < 1 || d1 > 1e10 || d2 > 1e10) return c.error(ErrorKind::Num);
    if (rightTail) p = 1 - p;
    if (p == 0) return Value::number(0);
    if (p == 1) return c.error(ErrorKind::Num);
    if (!inverseRising([&](double t) { return fCdf(t, d1, d2); }, p, 0, INFINITY, &x)) return c.error(ErrorKind::Num);
    return numberResult(x);
}
Value fnFInv(Call &c) { return fInverse(c, false); }
Value fnFInvRt(Call &c) { return fInverse(c, true); }

Value fnFTest(Call &c)
{
    std::vector<double> a, b;
    Value error;
    if (!numbersAt(c, 0, &a, &error) || !numbersAt(c, 1, &b, &error)) return error;
    if (a.size() < 2 || b.size() < 2) return c.error(ErrorKind::Div0);
    const double va = sumSquares(a, meanOf(a)) / double(a.size() - 1), vb = sumSquares(b, meanOf(b)) / double(b.size() - 1);
    if (va == 0 || vb == 0) return c.error(ErrorKind::Div0);
    const double f = va / vb, d1 = double(a.size() - 1), d2 = double(b.size() - 1);
    const double tail = f > 1 ? fTail(f, d1, d2) : fCdf(f, d1, d2);
    return numberResult(std::min(1.0, 2 * tail));
}

// ---- Student's t

Value fnTDist(Call &c)
{
    double x, df;
    bool cumulative;
    if (!c.number(0, &x) || !degrees(c, 1, &df) || !c.boolean(2, &cumulative)) return c.failure();
    if (df < 1) return c.error(ErrorKind::Num);
    if (cumulative) return numberResult(studentCdf(x, df));
    return numberResult(std::exp(std::lgamma((df + 1) / 2) - std::lgamma(df / 2)) / std::sqrt(df * kPi) * std::pow(1 + x * x / df, -(df + 1) / 2));
}

Value fnTDistRt(Call &c)
{
    double x, df;
    if (!c.number(0, &x) || !degrees(c, 1, &df)) return c.failure();
    if (df < 1) return c.error(ErrorKind::Num);
    return numberResult(studentTail(x, df));
}

Value fnTDist2T(Call &c)
{
    double x, df;
    if (!c.number(0, &x) || !degrees(c, 1, &df)) return c.failure();
    if (x < 0 || df < 1) return c.error(ErrorKind::Num);
    return numberResult(studentTwoTail(x, df));
}

Value fnTDistOld(Call &c)
{
    double x, df, tails;
    if (!c.number(0, &x) || !degrees(c, 1, &df) || !c.number(2, &tails)) return c.failure();
    tails = std::trunc(tails);
    if (x < 0 || df < 1 || (tails != 1 && tails != 2)) return c.error(ErrorKind::Num);
    return numberResult(tails == 1 ? studentTail(x, df) : studentTwoTail(x, df));
}

// The t with P(T <= t) = p.
bool studentInverse(double p, double df, double *t)
{
    if (p == 0.5) {
        *t = 0;
        return true;
    }
    double x;
    const bool below = p < 0.5;
    const double q = below ? p : 1 - p;
    // the two-tailed probability 2q falls as t rises: find the t with studentTwoTail(t) = 2q
    if (!inverseRising([&](double u) { return 1 - studentTwoTail(u, df); }, 1 - 2 * q, 0, INFINITY, &x)) return false;
    *t = below ? -x : x;
    return true;
}

Value fnTInv(Call &c)
{
    double p, df, t;
    if (!c.number(0, &p) || !degrees(c, 1, &df)) return c.failure();
    if (!(p > 0 && p < 1) || df < 1) return c.error(ErrorKind::Num);
    if (!studentInverse(p, df, &t)) return c.error(ErrorKind::Num);
    return numberResult(t);
}

Value fnTInv2T(Call &c)
{
    double p, df, t;
    if (!c.number(0, &p) || !degrees(c, 1, &df)) return c.failure();
    if (!(p > 0 && p <= 1) || df < 1) return c.error(ErrorKind::Num);
    if (p == 1) return Value::number(0);
    if (!studentInverse(1 - p / 2, df, &t)) return c.error(ErrorKind::Num);
    return numberResult(t);
}

Value fnConfidenceT(Call &c)
{
    double v[3], t;
    if (!args(c, v, 3)) return c.failure();
    v[2] = std::trunc(v[2]);
    if (!(v[0] > 0 && v[0] < 1) || v[1] <= 0 || v[2] < 1) return c.error(ErrorKind::Num);
    if (v[2] == 1) return c.error(ErrorKind::Div0);
    if (!studentInverse(1 - v[0] / 2, v[2] - 1, &t)) return c.error(ErrorKind::Num);
    return numberResult(t * v[1] / std::sqrt(v[2]));
}

Value fnTTest(Call &c)
{
    double tails, type;
    if (!c.number(2, &tails) || !c.number(3, &type)) return c.failure();
    tails = std::trunc(tails);
    type = std::trunc(type);
    if ((tails != 1 && tails != 2) || type < 1 || type > 3) return c.error(ErrorKind::Num);
    std::vector<double> a, b;
    Value error;
    double t, df;
    if (type == 1) {
        if (!pairsAt(c, 0, 1, &a, &b, &error)) return error;
        const double n = double(a.size());
        if (n < 2) return c.error(ErrorKind::Div0);
        std::vector<double> d(a.size());
        for (size_t i = 0; i < a.size(); ++i) d[i] = a[i] - b[i];
        const double sd = std::sqrt(sumSquares(d, meanOf(d)) / (n - 1));
        if (sd == 0) return c.error(ErrorKind::Div0);
        t = meanOf(d) / (sd / std::sqrt(n));
        df = n - 1;
    } else {
        if (!numbersAt(c, 0, &a, &error) || !numbersAt(c, 1, &b, &error)) return error;
        if (a.size() < 2 || b.size() < 2) return c.error(ErrorKind::Div0);
        const double na = double(a.size()), nb = double(b.size());
        const double va = sumSquares(a, meanOf(a)) / (na - 1), vb = sumSquares(b, meanOf(b)) / (nb - 1);
        if (type == 2) {
            const double pooled = ((na - 1) * va + (nb - 1) * vb) / (na + nb - 2);
            if (pooled == 0) return c.error(ErrorKind::Div0);
            t = (meanOf(a) - meanOf(b)) / std::sqrt(pooled * (1 / na + 1 / nb));
            df = na + nb - 2;
        } else {
            const double sa = va / na, sb = vb / nb;
            if (sa + sb == 0) return c.error(ErrorKind::Div0);
            t = (meanOf(a) - meanOf(b)) / std::sqrt(sa + sb);
            df = (sa + sb) * (sa + sb) / (sa * sa / (na - 1) + sb * sb / (nb - 1));
        }
    }
    const double two = studentTwoTail(std::fabs(t), df);
    return numberResult(tails == 2 ? two : two / 2);
}

// ---- the discrete distributions

Value fnBinomDist(Call &c)
{
    double v[3];
    bool cumulative;
    if (!args(c, v, 3) || !c.boolean(3, &cumulative)) return c.failure();
    v[0] = std::trunc(v[0]);
    v[1] = std::trunc(v[1]);
    if (v[1] < 0 || v[0] < 0 || v[0] > v[1] || v[2] < 0 || v[2] > 1) return c.error(ErrorKind::Num);
    return numberResult(cumulative ? binomialCdf(v[0], v[1], v[2]) : binomialPmf(v[0], v[1], v[2]));
}

Value fnBinomRange(Call &c)
{
    double v[3], upper;
    if (!args(c, v, 3) || !c.numberOr(3, 0, &upper)) return c.failure();
    v[0] = std::trunc(v[0]);
    v[2] = std::trunc(v[2]);
    upper = c.missing(3) ? v[2] : std::trunc(upper);
    if (v[0] < 0 || v[1] < 0 || v[1] > 1 || v[2] < 0 || v[2] > v[0] || upper < v[2] || upper > v[0]) return c.error(ErrorKind::Num);
    if (!charged(c, qint64(upper - v[2]) + 1)) return c.error(ErrorKind::Value);
    double s = 0;
    for (double k = v[2]; k <= upper; ++k) s += binomialPmf(k, v[0], v[1]);
    return numberResult(std::min(1.0, s));
}

Value fnBinomInv(Call &c)
{
    double v[3];
    if (!args(c, v, 3)) return c.failure();
    v[0] = std::trunc(v[0]);
    if (v[0] < 0 || v[1] < 0 || v[1] > 1 || v[2] < 0 || v[2] > 1) return c.error(ErrorKind::Num);
    // the least k whose cumulative probability reaches alpha (a bisection on k)
    double lo = 0, hi = v[0];
    if (v[2] == 0) return Value::number(0);
    while (lo < hi) {
        const double mid = std::floor((lo + hi) / 2);
        if (binomialCdf(mid, v[0], v[1]) >= v[2] * (1 - 1e-15)) hi = mid;
        else lo = mid + 1;
    }
    return Value::number(lo);
}

Value fnNegBinom(Call &c)
{
    double v[3];
    bool cumulative = false;
    if (!args(c, v, 3)) return c.failure();
    if (c.argc() > 3 && !c.boolean(3, &cumulative)) return c.failure();
    v[0] = std::trunc(v[0]);
    v[1] = std::trunc(v[1]);
    if (v[0] < 0 || v[1] < 1 || v[2] < 0 || v[2] > 1) return c.error(ErrorKind::Num);
    if (v[2] == 0) return c.error(ErrorKind::Num);
    if (cumulative) return numberResult(betaI(v[1], v[0] + 1, v[2]));
    return numberResult(std::exp(logChoose(v[0] + v[1] - 1, v[1] - 1) + v[1] * std::log(v[2]) + (v[2] == 1 ? (v[0] == 0 ? 0 : -INFINITY) : v[0] * std::log1p(-v[2]))));
}

Value fnHypgeom(Call &c)
{
    double v[4];
    bool cumulative = false;
    if (!args(c, v, 4)) return c.failure();
    if (c.argc() > 4 && !c.boolean(4, &cumulative)) return c.failure();
    for (double &d : v) d = std::trunc(d);
    const double x = v[0], n = v[1], m = v[2], total = v[3];
    if (total <= 0 || n < 0 || n > total || m < 0 || m > total || x < 0 || x > std::min(n, m) || x < std::max(0.0, n - total + m)) return c.error(ErrorKind::Num);
    auto pmf = [&](double k) { return std::exp(logChoose(m, k) + logChoose(total - m, n - k) - logChoose(total, n)); };
    if (!cumulative) return numberResult(pmf(x));
    if (!charged(c, qint64(x) + 1)) return c.error(ErrorKind::Value);
    double s = 0;
    for (double k = std::max(0.0, n - total + m); k <= x; ++k) s += pmf(k);
    return numberResult(std::min(1.0, s));
}

Value fnPoisson(Call &c)
{
    double x, mean;
    bool cumulative = true;
    if (!c.number(0, &x) || !c.number(1, &mean)) return c.failure();
    if (c.argc() > 2 && !c.boolean(2, &cumulative)) return c.failure();
    x = std::trunc(x);
    if (x < 0 || mean < 0) return c.error(ErrorKind::Num);
    if (mean == 0) return Value::number(cumulative || x == 0 ? 1 : 0);
    if (cumulative) return numberResult(gammaQ(x + 1, mean));
    return numberResult(std::exp(-mean + x * std::log(mean) - std::lgamma(x + 1)));
}

// ---- fitted lines and curves

struct Regression {
    std::vector<double> coef;      // for the columns of x, then the constant (if any)
    std::vector<double> error;     // the standard errors, same order
    double r2 = 0, seY = 0, f = 0, df = 0, ssReg = 0, ssResid = 0;
    int columns = 0;               // the number of x columns
    bool constant = true;
};

// Gaussian elimination with partial pivoting on a symmetric system; the inverse is returned too.
bool invert(std::vector<double> m, int n, std::vector<double> *inverse)
{
    inverse->assign(size_t(n) * n, 0);
    for (int i = 0; i < n; ++i) (*inverse)[size_t(i) * n + i] = 1;
    for (int k = 0; k < n; ++k) {
        int p = k;
        for (int i = k + 1; i < n; ++i)
            if (std::fabs(m[size_t(i) * n + k]) > std::fabs(m[size_t(p) * n + k])) p = i;
        if (std::fabs(m[size_t(p) * n + k]) < 1e-300) return false;
        if (p != k)
            for (int j = 0; j < n; ++j) {
                std::swap(m[size_t(p) * n + j], m[size_t(k) * n + j]);
                std::swap((*inverse)[size_t(p) * n + j], (*inverse)[size_t(k) * n + j]);
            }
        const double d = m[size_t(k) * n + k];
        for (int j = 0; j < n; ++j) {
            m[size_t(k) * n + j] /= d;
            (*inverse)[size_t(k) * n + j] /= d;
        }
        for (int i = 0; i < n; ++i) {
            if (i == k) continue;
            const double f = m[size_t(i) * n + k];
            if (f == 0) continue;
            for (int j = 0; j < n; ++j) {
                m[size_t(i) * n + j] -= f * m[size_t(k) * n + j];
                (*inverse)[size_t(i) * n + j] -= f * (*inverse)[size_t(k) * n + j];
            }
        }
    }
    return true;
}

// y (n values) against the columns of x (n rows of `k` values, by row); with `constant` an intercept is fitted. False when there are too few points or the columns are
// not independent.
bool regress(const std::vector<double> &y, const std::vector<double> &x, int k, bool constant, Regression *r)
{
    const int n = int(y.size());
    const int p = k + (constant ? 1 : 0);
    if (n < 1 || p < 1 || n < p) return false;
    // centered when there is a constant, for the accuracy of the sums
    std::vector<double> mean(size_t(k), 0);
    double my = 0;
    if (constant) {
        for (int i = 0; i < n; ++i) {
            my += y[size_t(i)];
            for (int j = 0; j < k; ++j) mean[size_t(j)] += x[size_t(i) * k + j];
        }
        my /= n;
        for (double &m : mean) m /= n;
    }
    std::vector<double> xtx(size_t(k) * k, 0), xty(size_t(k), 0);
    for (int i = 0; i < n; ++i)
        for (int a = 0; a < k; ++a) {
            const double xa = x[size_t(i) * k + a] - mean[size_t(a)];
            xty[size_t(a)] += xa * (y[size_t(i)] - my);
            for (int b = 0; b < k; ++b) xtx[size_t(a) * k + b] += xa * (x[size_t(i) * k + b] - mean[size_t(b)]);
        }
    std::vector<double> inv;
    if (!invert(xtx, k, &inv)) return false;
    std::vector<double> beta(size_t(k), 0);
    for (int a = 0; a < k; ++a)
        for (int b = 0; b < k; ++b) beta[size_t(a)] += inv[size_t(a) * k + b] * xty[size_t(b)];
    double intercept = 0;
    if (constant) {
        intercept = my;
        for (int a = 0; a < k; ++a) intercept -= beta[size_t(a)] * mean[size_t(a)];
    }
    double ssResid = 0, ssTotal = 0;
    for (int i = 0; i < n; ++i) {
        double fit = intercept;
        for (int a = 0; a < k; ++a) fit += beta[size_t(a)] * x[size_t(i) * k + a];
        ssResid += (y[size_t(i)] - fit) * (y[size_t(i)] - fit);
        ssTotal += constant ? (y[size_t(i)] - my) * (y[size_t(i)] - my) : y[size_t(i)] * y[size_t(i)];
    }
    r->columns = k;
    r->constant = constant;
    r->df = n - p;
    r->ssResid = ssResid;
    r->ssReg = ssTotal - ssResid;
    r->r2 = ssTotal == 0 ? 1 : r->ssReg / ssTotal;
    r->seY = r->df > 0 ? std::sqrt(ssResid / r->df) : 0;
    r->f = r->df > 0 && ssResid > 0 ? (r->ssReg / k) / (ssResid / r->df) : 0;
    r->coef = beta;
    r->error.assign(size_t(k), 0);
    for (int a = 0; a < k; ++a) r->error[size_t(a)] = r->df > 0 ? std::sqrt(std::max(0.0, ssResid / r->df * inv[size_t(a) * k + a])) : 0;
    if (constant) {
        r->coef.push_back(intercept);
        // the variance of the intercept: s^2 (1/n + mean' (X'X)^-1 mean)
        double q = 1.0 / n;
        for (int a = 0; a < k; ++a)
            for (int b = 0; b < k; ++b) q += mean[size_t(a)] * inv[size_t(a) * k + b] * mean[size_t(b)];
        r->error.push_back(r->df > 0 ? std::sqrt(std::max(0.0, ssResid / r->df * q)) : 0);
    }
    return true;
}

// Known y values and x values as columns: y is a vector (a row or a column) and x has as many points, with its variables along the other side.
bool seriesOf(Call &c, int yArg, int xArg, std::vector<double> *y, std::vector<double> *x, int *k, Value *error)
{
    Value vy;
    if (!c.array(yArg, &vy)) {
        *error = c.failure();
        return false;
    }
    const int ny = vy.rows() * vy.cols();
    const bool yIsColumn = vy.cols() == 1;
    if (vy.rows() != 1 && vy.cols() != 1) {
        *error = Value::error(ErrorKind::Ref);
        return false;
    }
    for (int i = 0; i < ny; ++i) {
        const Value &v = yIsColumn ? vy.at(i, 0) : vy.at(0, i);
        if (v.isError()) {
            *error = v;
            return false;
        }
        if (!v.isNumber()) {
            *error = Value::error(ErrorKind::Value);
            return false;
        }
        y->push_back(v.number());
    }
    if (c.missing(xArg)) {
        for (int i = 0; i < ny; ++i) x->push_back(i + 1);
        *k = 1;
        return true;
    }
    Value vx;
    if (!c.array(xArg, &vx)) {
        *error = c.failure();
        return false;
    }
    // the points run along the same side as y's; the variables along the other
    int points, vars;
    bool pointsAreRows;
    if (yIsColumn && vx.rows() == ny) {
        points = vx.rows();
        vars = vx.cols();
        pointsAreRows = true;
    } else if (!yIsColumn && vx.cols() == ny) {
        points = vx.cols();
        vars = vx.rows();
        pointsAreRows = false;
    } else if (ny == 1 && vx.rows() == 1 && vx.cols() == 1) {
        points = 1;
        vars = 1;
        pointsAreRows = true;
    } else {
        *error = Value::error(ErrorKind::Ref);
        return false;
    }
    for (int i = 0; i < points; ++i)
        for (int j = 0; j < vars; ++j) {
            const Value &v = pointsAreRows ? vx.at(i, j) : vx.at(j, i);
            if (v.isError()) {
                *error = v;
                return false;
            }
            if (!v.isNumber()) {
                *error = Value::error(ErrorKind::Value);
                return false;
            }
            x->push_back(v.number());
        }
    *k = vars;
    return true;
}

Value linest(Call &c, bool logarithmic)
{
    std::vector<double> y, x;
    int k = 0;
    Value error;
    bool constant = true, stats = false;
    if (!seriesOf(c, 0, 1, &y, &x, &k, &error)) return error;
    if (!c.booleanOr(2, true, &constant) || !c.booleanOr(3, false, &stats)) return c.failure();
    if (!charged(c, qint64(y.size()) * k * k + 1)) return c.error(ErrorKind::Value);
    if (logarithmic)
        for (double &v : y) {
            if (v <= 0) return c.error(ErrorKind::Num);
            v = std::log(v);
        }
    Regression r;
    if (!regress(y, x, k, constant, &r)) return c.error(ErrorKind::Num);
    const int cols = k + 1;
    Value out = c.machine().newArray(stats ? 5 : 1, cols);
    if (!out.isArray()) return out;
    auto put = [&](int row, int col, double v) { out.set(row, col, Value::number(v)); };
    auto unlog = [&](double v, bool coefficient) { return logarithmic && coefficient ? std::exp(v) : v; };
    for (int j = 0; j < k; ++j) put(0, j, unlog(r.coef[size_t(k - 1 - j)], true));
    put(0, k, constant ? unlog(r.coef[size_t(k)], true) : (logarithmic ? 1 : 0));
    if (stats) {
        for (int j = 0; j < k; ++j) put(1, j, r.error[size_t(k - 1 - j)]);
        put(1, k, constant ? r.error[size_t(k)] : NAN);
        put(2, 0, r.r2);
        put(2, 1, r.seY);
        put(3, 0, r.f);
        put(3, 1, r.df);
        put(4, 0, r.ssReg);
        put(4, 1, r.ssResid);
        for (int row = 1; row < 5; ++row)
            for (int col = 0; col < cols; ++col) {
                const bool filled = row == 1 || (row <= 4 && col <= 1);
                if (filled) continue;
                out.set(row, col, Value::error(ErrorKind::NA));
            }
        if (!constant) out.set(1, k, Value::error(ErrorKind::NA));
        for (int row = 1; row < 5; ++row)
            for (int col = 0; col < cols; ++col) {
                const Value &v = out.at(row, col);
                if (v.isNumber() && std::isnan(v.number())) out.set(row, col, Value::error(ErrorKind::NA));
            }
    }
    return out;
}
Value fnLinest(Call &c) { return linest(c, false); }
Value fnLogest(Call &c) { return linest(c, true); }

Value trend(Call &c, bool exponential)
{
    std::vector<double> y, x;
    int k = 0;
    Value error;
    bool constant = true;
    if (!seriesOf(c, 0, 1, &y, &x, &k, &error)) return error;
    if (!c.booleanOr(3, true, &constant)) return c.failure();
    if (!charged(c, qint64(y.size()) * k * k + 1)) return c.error(ErrorKind::Value);
    if (exponential)
        for (double &v : y) {
            if (v <= 0) return c.error(ErrorKind::Num);
            v = std::log(v);
        }
    Regression r;
    if (!regress(y, x, k, constant, &r)) return c.error(ErrorKind::Num);
    // the new x values: as many variables as the known ones, the points down the same side as the known y values
    Value vy;
    if (!c.array(0, &vy)) return c.failure();
    const bool pointsAreRows = vy.cols() == 1;
    std::vector<double> nx;
    if (c.missing(2)) nx = x;
    else {
        Value vn;
        if (!c.array(2, &vn)) return c.failure();
        const int vars = pointsAreRows ? vn.cols() : vn.rows(), points = pointsAreRows ? vn.rows() : vn.cols();
        if (vars != k) return c.error(ErrorKind::Ref);
        for (int i = 0; i < points; ++i)
            for (int j = 0; j < vars; ++j) {
                const Value &v = pointsAreRows ? vn.at(i, j) : vn.at(j, i);
                if (v.isError()) return v;
                if (!v.isNumber()) return c.error(ErrorKind::Value);
                nx.push_back(v.number());
            }
    }
    const int points = int(nx.size()) / k;
    if (!charged(c, qint64(points) * k)) return c.error(ErrorKind::Value);
    Value out = c.machine().newArray(pointsAreRows ? points : 1, pointsAreRows ? 1 : points);
    if (!out.isArray()) return out;
    for (int i = 0; i < points; ++i) {
        double fit = constant ? r.coef[size_t(k)] : 0;
        for (int a = 0; a < k; ++a) fit += r.coef[size_t(a)] * nx[size_t(i) * k + a];
        const double v = exponential ? std::exp(fit) : fit;
        if (pointsAreRows) out.set(i, 0, Value::number(v));
        else out.set(0, i, Value::number(v));
    }
    return out;
}
Value fnTrend(Call &c) { return trend(c, false); }
Value fnGrowth(Call &c) { return trend(c, true); }

} // namespace

void registerMoreStatFunctions(std::vector<FunctionInfo> &r)
{
    // shape and spread
    r.push_back({"SKEW", 1, 255, "r", 0, fnSkew});
    r.push_back({"SKEW.P", 1, 255, "r", kFnFuture, fnSkewP});
    r.push_back({"KURT", 1, 255, "r", 0, fnKurt});
    r.push_back({"GEOMEAN", 1, 255, "r", 0, fnGeomean});
    r.push_back({"HARMEAN", 1, 255, "r", 0, fnHarmean});
    r.push_back({"AVEDEV", 1, 255, "r", 0, fnAvedev});
    r.push_back({"DEVSQ", 1, 255, "r", 0, fnDevsq});
    r.push_back({"TRIMMEAN", 2, 2, "rv", 0, fnTrimmean});
    r.push_back({"STDEVA", 1, 255, "r", 0, fnStdevA});
    r.push_back({"STDEVPA", 1, 255, "r", 0, fnStdevPA});
    r.push_back({"VARA", 1, 255, "r", 0, fnVarA});
    r.push_back({"VARPA", 1, 255, "r", 0, fnVarPA});
    r.push_back({"FISHER", 1, 1, "v", 0, fnFisher});
    r.push_back({"FISHERINV", 1, 1, "v", 0, fnFisherInv});
    r.push_back({"STANDARDIZE", 3, 3, "vvv", 0, fnStandardize});
    r.push_back({"PERMUTATIONA", 2, 2, "vv", kFnFuture, fnPermutationA});
    r.push_back({"GAMMALN", 1, 1, "v", 0, fnGammaLn});
    r.push_back({"GAMMALN.PRECISE", 1, 1, "v", kFnFuture, fnGammaLn});
    r.push_back({"GAMMA", 1, 1, "v", kFnFuture, fnGamma});
    r.push_back({"PROB", 3, 4, "aavv", 0, fnProb});
    // ranks and percentiles
    r.push_back({"RANK.AVG", 2, 3, "vrv", kFnFuture, rankAverage});
    r.push_back({"PERCENTILE.EXC", 2, 2, "rv", kFnFuture, fnPercentileExc});
    r.push_back({"QUARTILE.EXC", 2, 2, "rv", kFnFuture, fnQuartileExc});
    r.push_back({"PERCENTRANK", 2, 3, "rvv", 0, fnPercentRankInc});
    r.push_back({"PERCENTRANK.INC", 2, 3, "rvv", kFnFuture, fnPercentRankInc});
    r.push_back({"PERCENTRANK.EXC", 2, 3, "rvv", kFnFuture, fnPercentRankExc});
    r.push_back({"MODE.MULT", 1, 255, "r", kFnFuture, fnModeMult});
    r.push_back({"FREQUENCY", 2, 2, "rr", 0, fnFrequency});
    r.push_back({"COVARIANCE.S", 2, 2, "aa", kFnFuture, fnCovarianceS});
    r.push_back({"STEYX", 2, 2, "aa", 0, fnSteyx});
    // the normal family
    r.push_back({"NORM.S.DIST", 2, 2, "vv", kFnFuture, fnNormSDist2});
    r.push_back({"GAUSS", 1, 1, "v", kFnFuture, fnGauss});
    r.push_back({"PHI", 1, 1, "v", kFnFuture, fnPhi});
    r.push_back({"LOGNORM.DIST", 4, 4, "vvvv", kFnFuture, fnLognormDist});
    r.push_back({"LOGNORMDIST", 3, 3, "vvv", 0, fnLognormDist});
    r.push_back({"LOGNORM.INV", 3, 3, "vvv", kFnFuture, fnLognormInv});
    r.push_back({"LOGINV", 3, 3, "vvv", 0, fnLognormInv});
    r.push_back({"CONFIDENCE.NORM", 3, 3, "vvv", kFnFuture, fnConfidenceNorm});
    r.push_back({"CONFIDENCE", 3, 3, "vvv", 0, fnConfidenceNorm});
    r.push_back({"CONFIDENCE.T", 3, 3, "vvv", kFnFuture, fnConfidenceT});
    r.push_back({"Z.TEST", 2, 3, "rvv", kFnFuture, fnZTest});
    r.push_back({"ZTEST", 2, 3, "rvv", 0, fnZTest});
    // other continuous distributions
    r.push_back({"EXPON.DIST", 3, 3, "vvv", kFnFuture, fnExponDist});
    r.push_back({"EXPONDIST", 3, 3, "vvv", 0, fnExponDist});
    r.push_back({"WEIBULL.DIST", 4, 4, "vvvv", kFnFuture, fnWeibull});
    r.push_back({"WEIBULL", 4, 4, "vvvv", 0, fnWeibull});
    r.push_back({"GAMMA.DIST", 4, 4, "vvvv", kFnFuture, fnGammaDist});
    r.push_back({"GAMMADIST", 4, 4, "vvvv", 0, fnGammaDist});
    r.push_back({"GAMMA.INV", 3, 3, "vvv", kFnFuture, fnGammaInv});
    r.push_back({"GAMMAINV", 3, 3, "vvv", 0, fnGammaInv});
    r.push_back({"BETA.DIST", 4, 6, "vvvvvv", kFnFuture, fnBetaDist});
    r.push_back({"BETADIST", 3, 5, "vvvvv", 0, fnBetaDistOld});
    r.push_back({"BETA.INV", 3, 5, "vvvvv", kFnFuture, fnBetaInv});
    r.push_back({"BETAINV", 3, 5, "vvvvv", 0, fnBetaInv});
    r.push_back({"CHISQ.DIST", 3, 3, "vvv", kFnFuture, fnChiDist});
    r.push_back({"CHISQ.DIST.RT", 2, 2, "vv", kFnFuture, fnChiDistRt});
    r.push_back({"CHIDIST", 2, 2, "vv", 0, fnChiDistRt});
    r.push_back({"CHISQ.INV", 2, 2, "vv", kFnFuture, fnChiInv});
    r.push_back({"CHISQ.INV.RT", 2, 2, "vv", kFnFuture, fnChiInvRt});
    r.push_back({"CHIINV", 2, 2, "vv", 0, fnChiInvRt});
    r.push_back({"CHISQ.TEST", 2, 2, "aa", kFnFuture, fnChiTest});
    r.push_back({"CHITEST", 2, 2, "aa", 0, fnChiTest});
    r.push_back({"F.DIST", 4, 4, "vvvv", kFnFuture, fnFDist});
    r.push_back({"F.DIST.RT", 3, 3, "vvv", kFnFuture, fnFDistRt});
    r.push_back({"FDIST", 3, 3, "vvv", 0, fnFDistRt});
    r.push_back({"F.INV", 3, 3, "vvv", kFnFuture, fnFInv});
    r.push_back({"F.INV.RT", 3, 3, "vvv", kFnFuture, fnFInvRt});
    r.push_back({"FINV", 3, 3, "vvv", 0, fnFInvRt});
    r.push_back({"F.TEST", 2, 2, "rr", kFnFuture, fnFTest});
    r.push_back({"FTEST", 2, 2, "rr", 0, fnFTest});
    r.push_back({"T.DIST", 3, 3, "vvv", kFnFuture, fnTDist});
    r.push_back({"T.DIST.RT", 2, 2, "vv", kFnFuture, fnTDistRt});
    r.push_back({"T.DIST.2T", 2, 2, "vv", kFnFuture, fnTDist2T});
    r.push_back({"TDIST", 3, 3, "vvv", 0, fnTDistOld});
    r.push_back({"T.INV", 2, 2, "vv", kFnFuture, fnTInv});
    r.push_back({"T.INV.2T", 2, 2, "vv", kFnFuture, fnTInv2T});
    r.push_back({"TINV", 2, 2, "vv", 0, fnTInv2T});
    r.push_back({"T.TEST", 4, 4, "rrvv", kFnFuture, fnTTest});
    r.push_back({"TTEST", 4, 4, "rrvv", 0, fnTTest});
    // discrete distributions
    r.push_back({"BINOM.DIST", 4, 4, "vvvv", kFnFuture, fnBinomDist});
    r.push_back({"BINOMDIST", 4, 4, "vvvv", 0, fnBinomDist});
    r.push_back({"BINOM.DIST.RANGE", 3, 4, "vvvv", kFnFuture, fnBinomRange});
    r.push_back({"BINOM.INV", 3, 3, "vvv", kFnFuture, fnBinomInv});
    r.push_back({"CRITBINOM", 3, 3, "vvv", 0, fnBinomInv});
    r.push_back({"NEGBINOM.DIST", 4, 4, "vvvv", kFnFuture, fnNegBinom});
    r.push_back({"NEGBINOMDIST", 3, 3, "vvv", 0, fnNegBinom});
    r.push_back({"HYPGEOM.DIST", 5, 5, "vvvvv", kFnFuture, fnHypgeom});
    r.push_back({"HYPGEOMDIST", 4, 4, "vvvv", 0, fnHypgeom});
    r.push_back({"POISSON.DIST", 3, 3, "vvv", kFnFuture, fnPoisson});
    r.push_back({"POISSON", 3, 3, "vvv", 0, fnPoisson});
    // fitted lines and curves
    r.push_back({"LINEST", 1, 4, "aavv", 0, fnLinest});
    r.push_back({"LOGEST", 1, 4, "aavv", 0, fnLogest});
    r.push_back({"TREND", 1, 4, "aaav", 0, fnTrend});
    r.push_back({"GROWTH", 1, 4, "aaav", 0, fnGrowth});
}

} // namespace jp::calc
