#pragma once
// The special functions the statistical functions are made of (internal to calc/): the regularized incomplete gamma and beta functions, and the inverses of
// distributions that rise with their argument. No GUI.

#include <functional>

namespace jp::calc {

// P(a, x) and Q(a, x) = 1 - P(a, x): the regularized lower and upper incomplete gamma function, a > 0, x >= 0.
double gammaP(double a, double x);
double gammaQ(double a, double x);
// I_x(a, b): the regularized incomplete beta function, a, b > 0, 0 <= x <= 1.
double betaI(double a, double b, double x);
// The inverse of the standard normal distribution (false outside 0 < p < 1).
bool inverseStandardNormal(double p, double *out);
// The x in [lo, hi] (hi may be infinite: the bracket then grows) at which a function that rises with x reaches p; false if it does not.
bool inverseRising(const std::function<double(double)> &f, double p, double lo, double hi, double *out);

} // namespace jp::calc
