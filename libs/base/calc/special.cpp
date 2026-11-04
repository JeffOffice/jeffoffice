#include "calc/special.h"

#include "calc/budget.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace jp::calc {

namespace {

constexpr double kEps = 1e-16;
constexpr int kMaxIterations = 100000;

// P(a, x) by its series (x < a + 1).
double gammaSeries(double a, double x)
{
    double ap = a, sum = 1 / a, del = sum;
    for (int n = 0; n < kMaxIterations; ++n) {
        ap += 1;
        del *= x / ap;
        sum += del;
        if (std::fabs(del) < std::fabs(sum) * kEps) {
            budget::addWork(n + 1);
            break;
        }
        if (n + 1 == kMaxIterations) budget::addWork(n + 1);
    }
    return sum * std::exp(-x + a * std::log(x) - std::lgamma(a));
}

// Q(a, x) by its continued fraction (x >= a + 1), the modified Lentz method.
double gammaFraction(double a, double x)
{
    const double tiny = 1e-300;
    double b = x + 1 - a, c = 1 / tiny, d = 1 / b, h = d;
    for (int i = 1; i < kMaxIterations; ++i) {
        const double an = -double(i) * (double(i) - a);
        b += 2;
        d = an * d + b;
        if (std::fabs(d) < tiny) d = tiny;
        c = b + an / c;
        if (std::fabs(c) < tiny) c = tiny;
        d = 1 / d;
        const double del = d * c;
        h *= del;
        if (std::fabs(del - 1) < kEps || i + 1 == kMaxIterations) {
            budget::addWork(i);
            break;
        }
    }
    return std::exp(-x + a * std::log(x) - std::lgamma(a)) * h;
}

double betaFraction(double a, double b, double x)
{
    const double tiny = 1e-300;
    const double qab = a + b, qap = a + 1, qam = a - 1;
    double c = 1, d = 1 - qab * x / qap;
    if (std::fabs(d) < tiny) d = tiny;
    d = 1 / d;
    double h = d;
    for (int m = 1; m < kMaxIterations; ++m) {
        const int m2 = 2 * m;
        double aa = m * (b - m) * x / ((qam + m2) * (a + m2));
        d = 1 + aa * d;
        if (std::fabs(d) < tiny) d = tiny;
        c = 1 + aa / c;
        if (std::fabs(c) < tiny) c = tiny;
        d = 1 / d;
        h *= d * c;
        aa = -(a + m) * (qab + m) * x / ((a + m2) * (qap + m2));
        d = 1 + aa * d;
        if (std::fabs(d) < tiny) d = tiny;
        c = 1 + aa / c;
        if (std::fabs(c) < tiny) c = tiny;
        d = 1 / d;
        const double del = d * c;
        h *= del;
        if (std::fabs(del - 1) < kEps || m + 1 == kMaxIterations) {
            budget::addWork(m);
            break;
        }
    }
    return h;
}

} // namespace

double gammaP(double a, double x)
{
    if (x <= 0) return 0;
    if (std::isinf(x)) return 1;
    return x < a + 1 ? gammaSeries(a, x) : 1 - gammaFraction(a, x);
}

double gammaQ(double a, double x)
{
    if (x <= 0) return 1;
    if (std::isinf(x)) return 0;
    return x < a + 1 ? 1 - gammaSeries(a, x) : gammaFraction(a, x);
}

double betaI(double a, double b, double x)
{
    if (x <= 0) return 0;
    if (x >= 1) return 1;
    const double front = std::exp(std::lgamma(a + b) - std::lgamma(a) - std::lgamma(b) + a * std::log(x) + b * std::log1p(-x));
    if (x < (a + 1) / (a + b + 2)) return front * betaFraction(a, b, x) / a;
    return 1 - front * betaFraction(b, a, 1 - x) / b;
}

bool inverseRising(const std::function<double(double)> &f, double p, double lo, double hi, double *out)
{
    if (std::isinf(hi)) {
        double step = std::max(1.0, std::fabs(lo) + 1);
        hi = lo + step;
        for (int i = 0; i < 200 && f(hi) < p; ++i) {
            step *= 2;
            hi = lo + step;
        }
        if (f(hi) < p) return false;
    }
    if (f(lo) > p) return false;
    for (int i = 0; i < 400; ++i) {
        const double mid = lo + (hi - lo) / 2;
        if (mid == lo || mid == hi) break;
        if (f(mid) < p) lo = mid;
        else hi = mid;
    }
    *out = lo + (hi - lo) / 2;
    return true;
}

} // namespace jp::calc
